#include <sys/stat.h>

#include <cassert>
#include <iostream>

#include <TCanvas.h>
#include <TGeoPainter.h>
#include <TPad.h>
#include <TView.h>
#include <TView3D.h>

#include "SbtAlignGeom.h"
#include "SbtIO.h"
#include "SbtConfig.h"
#include "SbtMakeClusters.h"
#include "SbtMakeHits.h"
#include "SbtMakeSpacePoints.h"
#include "SbtMakeTracks.h"
#include "SbtNtupleDumper.h"
#include "SbtPixelDetectorElem.h"
#include "SbtSingleSideDetectorElem.h"
#include "SbtStripDetectorElem.h"
#include "SbtStripletsDetectorElem.h"
#include "SbtEventReader.h"
#include "SbtCrystalChanneling.h"

ClassImp(SbtConfig);

SbtConfig::SbtConfig() :
  _debugLevel(0),
  _topVolName("TOP"),
  _geoManager(nullptr),
  _crystalChanneling(nullptr),
  _ignoreAlignment(false) {
  // initialize detectorMapArray
  for (int i = 0; i < nMaxLayerSides; i++) {
    _detectorMapArray[i] = -1;
  }
}

SbtConfig::SbtConfig(std::string configFileName, std::string datafile, bool ignoreAlign) :
  _debugLevel(0),
  _topVolName("TOP"),
  _geoManager(nullptr),
  _crystalChanneling(nullptr),
  _ignoreAlignment(ignoreAlign) {
  // initialize detectorMapArray
  for (int i = 0; i < nMaxLayerSides; i++) {
    _detectorMapArray[i] = -1;
  }

  configureFromYaml(configFileName, datafile);
}

void SbtConfig::initializeGeoManager(const YAML::Node& conf) {
  _geoManager = new TGeoManager("SbtGeoManager", "SbtGeoManager");

  _geoManager->BuildDefaultMaterials();  // Build the default materials.

  std::vector<double> topVolumeSize = conf["dimensions"].as<std::vector<double>>();

  std::string material_name = conf["material"].as<std::string>();

  TGeoMaterial* material = nullptr;
  TGeoMedium* medium = nullptr;

  if (material_name == "air") {
    material = new TGeoMaterial("Air", 14.61, 7.3, 0.001205);
    medium = new TGeoMedium("air", -1, material);
  }
  else if (material_name == "vacuum") {
    material = new TGeoMaterial("Vacuum");
    medium = new TGeoMedium("vacuum", -1, material);
  }
  else {
    std::cout << "Material '" << material_name << "' unknown!" << std::endl;
    assert(0);
  }
  TGeoVolume* top = _geoManager->MakeBox(_topVolName, medium, topVolumeSize[0], topVolumeSize[1], topVolumeSize[2]);
  _geoManager->SetTopVolume(top);
}

void SbtConfig::configureFromYaml(std::string configFileName, std::string datafile) {
  if (!SbtIO::searchConfigFile(configFileName)) return;
  std::cout << "Loading config file '" << configFileName << "'" << std::endl;
  YAML::Node config = YAML::LoadFile(configFileName);

  if (!datafile.empty() && SbtIO::searchConfigFile(datafile)) {
    std::cout << "Loading data config file '" << datafile << "'" << std::endl;
    YAML::Node data_config = YAML::LoadFile(datafile);

    std::stringstream fullname;
    fullname << config["name"].as<std::string>() << "_" << data_config["name"].as<std::string>();
    _name = fullname.str();

    configureEventReader(data_config);
  }
  else {
    _name = config["name"].as<std::string>();
  }

  initializeGeoManager(config["topVolume"]);
  
  if (config["detectorTypes"]) configureDetectorTypeList(config["detectorTypes"]);
  if (config["tracking"]) configureTrackingDetectors(config["tracking"]);
  if (config["detectorElements"]) configureDetectorElemList(config["detectorElements"]);
  if (config["DAQmap"]) configureDetectorMap(config["DAQmap"]);
  if (config["crystal"]) configureCrystal(config["crystal"]);

  configureOutput();

  configureAlgorithms(config);

  if (config["align"] && !_ignoreAlignment) {
    SbtAlignGeom alignGeom;
    alignGeom.initialize(config["align"]["name"].as<std::string>(), this, SbtEnums::alignMode::dataAlign, SbtIO::expandPath(config["align"]["path"].as<std::string>()));
    alignGeom.readAlignParms(config["align"]["iterations"].as<int>());
  }

    // close the geometry
  _geoManager->CloseGeometry();
}

void SbtConfig::configureEventReader(const YAML::Node& conf) {
  _eventReader = new SbtEventReader(conf);
  _eventReader->setConfigurator(this);
}

void SbtConfig::configureCrystal(const YAML::Node& conf) {
  auto x_HalfDim = conf["halfDimensions"][0].as<double>();
  auto y_HalfDim = conf["halfDimensions"][1].as<double>();
  auto z_HalfDim = conf["halfDimensions"][2].as<double>();
  auto x_pos = conf["position"][0].as<double>();
  auto y_pos = conf["position"][1].as<double>();
  auto z_pos = conf["position"][2].as<double>();
  auto material = conf["material"].as<std::string>();
  auto deflAngle = conf["deflectionAngle"].as<double>();
  double chanAngle = 0.;
  if (conf["channelingAngle"]) {
    chanAngle = conf["channelingAngle"].as<double>();
  }
  _crystalChanneling = new SbtCrystalChanneling(_geoManager, x_pos, y_pos, z_pos, x_HalfDim, y_HalfDim, z_HalfDim, material.c_str(), chanAngle, deflAngle);
  for (auto plane : conf["channelingPlanes"]) {
    auto name = plane["plane"].as<std::string>().c_str();
    auto max_eff = plane["maxChannelingEff"].as<double>();
    auto crit_angle = plane["crticalAngle"].as<double>();
    auto w = plane["weight"].as<double>();
    _crystalChanneling->addCrystalPlane(name, max_eff, crit_angle, w);
  }
}

void SbtConfig::configureDetectorTypeList(const YAML::Node& config) {
  // Define here the relevant variables to read
  int type_id(0);   // type id
  std::string type_det;  // type of detector
  double x_HalfDim(0.0), y_HalfDim(0.0), z_HalfDim(0.0);  // half-dimensions of detector x, y, z

  double x_ActMin(0.0), x_ActMax(0.0);  // range of active area x
  double y_ActMin(0.0), y_ActMax(0.0);  // range of active area y

  double xPrime_ActMin(0.0), xPrime_ActMax(0.0);  // range of active area x
  double yPrime_ActMin(0.0), yPrime_ActMax(0.0);  // range of active area y

  std::string type_material;                 // material
  double u_pitch(0.0), v_pitch(0.0);    // pitch u, pitch v
  double u_angle(0.0), v_angle(0.0);    // angle u, angle v
  double u_offset(0.0), v_offset(0.0);  // offset u, offset v

  for (auto detectorType : config) {
    type_id = detectorType["id"].as<int>();
    type_det = detectorType["type"].as<std::string>();
    type_material = detectorType["material"].as<std::string>();
    x_HalfDim = detectorType["halfDimensions"][0].as<double>();
    y_HalfDim = detectorType["halfDimensions"][1].as<double>();
    z_HalfDim = detectorType["halfDimensions"][2].as<double>();
    x_ActMin = detectorType["activeAreaX"][0].as<double>();
    x_ActMax = detectorType["activeAreaX"][1].as<double>();
    y_ActMin = detectorType["activeAreaY"][0].as<double>();
    y_ActMax = detectorType["activeAreaY"][1].as<double>();

    u_pitch = detectorType["pitch"][0].as<double>();
    u_angle = detectorType["pitchAngle"][0].as<double>();
    u_offset = detectorType["pitchOffset"][0].as<double>();

    if (type_det != "singleside") {
      v_pitch = detectorType["pitch"][1].as<double>();
      v_angle = detectorType["pitchAngle"][1].as<double>();
      v_offset = detectorType["pitchOffset"][1].as<double>();
    }

    // striplet detectors have 4 extra params
    if (type_det == "striplet") {
      xPrime_ActMin = detectorType["primeActiveAreaX"][0].as<double>();
      xPrime_ActMax = detectorType["primeActiveAreaX"][1].as<double>();
      yPrime_ActMin = detectorType["primeActiveAreaY"][0].as<double>();
      yPrime_ActMax = detectorType["primeActiveAreaY"][1].as<double>();
    }

    //  Build here the DetectorType Object using the above informations
    //  and add to the DetectorType List
    SbtDetectorType* DetectorType = new SbtDetectorType(
        type_id, type_det, x_HalfDim, y_HalfDim, z_HalfDim, x_ActMin,
        x_ActMax, y_ActMin, y_ActMax, type_material, u_pitch, v_pitch,
        u_angle, v_angle, u_offset, v_offset, xPrime_ActMin, xPrime_ActMax,
        yPrime_ActMin, yPrime_ActMax);

    _detectorTypes.push_back(DetectorType);

    if (_debugLevel > 0) {
      std::cout << "SbtConfig: DetectorType configuration parameters " << std::endl;
      std::cout << "type_id \t = \t " << type_id << std::endl;
      std::cout << "type_det \t = \t " << type_det << std::endl;
      std::cout << "x_HalfDim \t = \t " << x_HalfDim << std::endl;
      std::cout << "y_HalfDim \t = \t " << y_HalfDim << std::endl;
      std::cout << "z_HalfDim \t = \t " << z_HalfDim << std::endl;
      std::cout << "x_ActMin \t = \t " << x_ActMin << std::endl;
      std::cout << "x_ActMax \t = \t " << x_ActMax << std::endl;
      std::cout << "y_ActMin \t = \t " << y_ActMin << std::endl;
      std::cout << "y_ActMax \t = \t " << y_ActMax << std::endl;
      std::cout << "xPrime_ActMin \t = \t " << xPrime_ActMin << std::endl;
      std::cout << "xPrime_ActMax \t = \t " << xPrime_ActMax << std::endl;
      std::cout << "yPrime_ActMin \t = \t " << yPrime_ActMin << std::endl;
      std::cout << "yPrime_ActMax \t = \t " << yPrime_ActMax << std::endl;
      std::cout << "type_material \t = \t " << type_material << std::endl;
      std::cout << "u_pitch \t = \t " << u_pitch << std::endl;
      std::cout << "v_pitch \t = \t " << v_pitch << std::endl;
      std::cout << "u_angle \t = \t " << u_angle << std::endl;
      std::cout << "v_angle \t = \t " << v_angle << std::endl;
      std::cout << "u_offset \t = \t " << u_offset << std::endl;
      std::cout << "v_offset \t = \t " << v_offset << std::endl;
    }
  }
}

void SbtConfig::configureDetectorElemList(const YAML::Node& config) {
  // Define here the relevant variables to read
  int DetElem_Id(-1);                         // detector elem id
  int DetType_Id(-1);                         // detector type id
  int DetTrack_Id(-1);                        // detector tracking id
  double x_Pos(0.0), y_Pos(0.0), z_Pos(0.0);  // half-dim of detector x, y, z

  // Use Eulero rotation angles in degree (phi, theta, psi)
  double det_phi(0.0);
  double det_theta(0.0);
  double det_psi(0.0);

  double orientation_U(0);
  double orientation_V(0);

  for (auto detectorElement : config) {
    DetElem_Id = detectorElement["id"].as<int>();
    DetType_Id = detectorElement["typeId"].as<int>();
    x_Pos = detectorElement["position"][0].as<double>();
    y_Pos = detectorElement["position"][1].as<double>();
    z_Pos = detectorElement["position"][2].as<double>();
    det_phi = detectorElement["eulerAngles"][0].as<double>();
    det_theta = detectorElement["eulerAngles"][1].as<double>();
    det_psi = detectorElement["eulerAngles"][2].as<double>();
    orientation_U = detectorElement["orientation"][0].as<double>();
    if (detectorElement["orientation"].size() > 1)
      orientation_V = detectorElement["orientation"][1].as<double>();

    // set the _trackingID for tracking detector
    for (unsigned int i = 0; i < _nTrackDet; i++) {
      DetTrack_Id = -1;
      if (_debugLevel > 0) {
        std::cout << "DetElem_Id = " << DetElem_Id << std::endl;
        std::cout << "_trackDetID[" << i << "] = " << _trackDetID[i]
                  << std::endl;
      }
      if (DetElem_Id == _trackDetID[i]) {
        DetTrack_Id = i;
        break;
      }
    }

    //  Build here the DetectorType Object using the above informations
    SbtDetectorElem* DetectorElem;
    SbtDetectorType* DetectorType = getDetectorType(DetType_Id);
    std::string detType = DetectorType->GetType();
    if (detType != "strip" && detType != "pixel" && detType != "striplet" &&
        detType != "singleside") {
      std::cout << "SbtConfig::MakeDetectorElemList() not valid DetectorType"
                << std::endl;
      assert(0);
    }

    if (detType == "strip") {
      DetectorElem = new SbtStripDetectorElem(
          _geoManager, DetectorType, DetElem_Id, DetTrack_Id, x_Pos, y_Pos,
          z_Pos, det_phi, det_theta, det_psi, orientation_U, orientation_V);
    } else if (detType == "pixel") {
      DetectorElem = new SbtPixelDetectorElem(
          _geoManager, DetectorType, DetElem_Id, DetTrack_Id, x_Pos, y_Pos,
          z_Pos, det_phi, det_theta, det_psi, orientation_U, orientation_V);

    } else if (detType == "striplet") {
      DetectorElem = new SbtStripletsDetectorElem(
          _geoManager, DetectorType, DetElem_Id, DetTrack_Id, x_Pos, y_Pos,
          z_Pos, det_phi, det_theta, det_psi, orientation_U, orientation_V);
    } else if (detType == "singleside") {
      DetectorElem = new SbtSingleSideDetectorElem(
          _geoManager, DetectorType, DetElem_Id, DetTrack_Id, x_Pos, y_Pos,
          z_Pos, det_phi, det_theta, det_psi, orientation_U, orientation_V);
    }
    _detectorElems.push_back(DetectorElem);

    if (_debugLevel > 0) {
      std::cout << "SbtConfig: DetectorElem configuration parameters "
                << std::endl;
      std::cout << "DetElem_id \t = \t " << DetElem_Id << std::endl;
      std::cout << "DetType_id \t = \t " << DetType_Id << std::endl;
      std::cout << "DetTrack_id \t = \t " << DetTrack_Id << std::endl;
      std::cout << "DetType \t = \t " << detType << std::endl;
      std::cout << "x_Pos   \t = \t " << x_Pos << std::endl;
      std::cout << "y_Pos   \t = \t " << y_Pos << std::endl;
      std::cout << "z_Pos   \t = \t " << z_Pos << std::endl;
      std::cout << "det_phi \t = \t " << det_phi << std::endl;
      std::cout << "det_theta \t = \t " << det_theta << std::endl;
      std::cout << "det_psi \t = \t " << det_psi << std::endl;

      std::cout << "orientation_U \t = \t" << orientation_U << std::endl;
      std::cout << "orientation_V \t = \t" << orientation_V << std::endl;
    }
  }
}

void SbtConfig::configureTrackingDetectors(const YAML::Node& config) {
  // read number of tracking detectors
  _nTrackDet = config["detectorIds"].size();
  assert(_nTrackDet > 1);
  // read  tracking detectors ID
  for (auto detId : config["detectorIds"]) {
    _trackDetID.push_back(detId.as<int>());
  }

  if (config["spacePointError"]) {
    _trackDetErr = config["spacePointError"].as<double>();
  }
  else {
    _trackDetErr = 0.;
  }

  // read hit error evaluation method
  _spErrMethod = config["spacePointErrorMethod"].as<std::string>();
  assert(_spErrMethod == "Simple" || _spErrMethod == "FirstPrinciple" ||
      _spErrMethod == "ErrorPropagation");

  if (_debugLevel > 0) {
    std::cout << "SbtConfig: Tracking Det configuration parameters "
              << std::endl;
    std::cout << "Number of tracking Det \t = \t " << _nTrackDet << std::endl;
    std::cout << "SpacePoint Error Method \t = \t " << _spErrMethod
              << std::endl;
    std::cout << "SpacePoint Default Error \t = \t " << _trackDetErr
              << std::endl;
    for (unsigned int i = 0; i < _nTrackDet; i++) {
      std::cout << "Tracking Det[" << i << "] \t = \t" << _trackDetID[i]
                << std::endl;
    }
  }
}

void SbtConfig::configureDetectorMap(const YAML::Node& config) {
  _nLayerSides = config.size();

  // read the map values : layerSide for DAQ and detectorElemID
  for (auto layer : config) {
    int DaqLayerSide = layer.first.as<int>();
    int PhysLayer = layer.second.as<int>();

    for (unsigned int i = 0; i < _detectorElems.size(); i++) {
      if (PhysLayer % 100 == _detectorElems.at(i)->GetID()) {
        _detectorMapArray[DaqLayerSide] = PhysLayer;
        _detectorElems.at(i)->AddLayerSide(DaqLayerSide);
      }
    }

    if (_debugLevel > 0) {
      std::cout << "SbtConfig: Detector Map  parameters " << std::endl;
      std::cout << "LayerSide: " << DaqLayerSide << "  Layer: " << PhysLayer
            << std::endl;
    }
  }

  if (_debugLevel > 0) {
    for (int i = 0; i < nMaxLayerSides; i++) {
      std::cout << "_detectorMapArray[" << i << "] = " << _detectorMapArray[i]
            << std::endl;
    }
  }
}

void SbtConfig::configureOutput() {
  // read Root output filename
  std::stringstream outfilename;
  outfilename << _name << "/rootuple/" << _name;
  _rootOutFilename = outfilename.str();

  if (_debugLevel > 0) {
    std::cout << "SbtConfig: Output configuration parameters " << std::endl;
    std::cout << "Root Output Filename \t = \t " << _rootOutFilename << ".root " << std::endl;
  }

  if (_rootOutFilename != "") {
    _ntupleDumper = new SbtNtupleDumper(_rootOutFilename, this);
  }
}

void SbtConfig::configureAlgorithms(const YAML::Node& config) {
  if (config["clustering"]) {
    _clusteringAlg = config["clustering"]["algorithm"].as<std::string>();
    if (config["clustering"]["pixelOption"]) {
      _pixelClusteringOpt = config["clustering"]["pixelOption"].as<std::string>();
    }
    else {
      _pixelClusteringOpt = "Loose";
    }
    if (_pixelClusteringOpt != "Tight" && _pixelClusteringOpt != "Loose") {
      std::cout << std::endl;
      std::cout << "Configuration Error" << std::endl;
      std::cout << "SbtConfig: pixelClusteringOpt is not correctly defined"
                << std::endl;
      std::cout << _pixelClusteringOpt << " not known" << std::endl;
      std::cout << "Please check the configuration file. Correct values are: "
                   "Loose, Tight"
                << std::endl;
      std::cout << std::endl;
      assert(0);
    }
  }

  // instantiate the algorithm objects
  _makeClusters = new SbtMakeClusters(_clusteringAlg, _pixelClusteringOpt);
  _makeHits = new SbtMakeHits();
  _makeSpacePoints = new SbtMakeSpacePoints(_spErrMethod, _trackDetErr);
  _makeTracks = new SbtMakeTracks(config["tracking"], _trackDetID);
}

SbtDetectorType* SbtConfig::getDetectorTypeFromID(int t) {
  for (auto detType : _detectorTypes) {
    if (detType->GetID() == t) return detType;
  }
  return nullptr;
}

SbtDetectorElem* SbtConfig::getDetectorElemFromID(int t) {
  for (auto detElem : _detectorElems) {
    if (detElem->GetID() == t) return detElem;
  }
  return nullptr;
}

void SbtConfig::drawGeom() {
  _geoManager->SetVisLevel(4);
  TCanvas* can = new TCanvas("can", "Test Beam Geometry", 800, 600);
  TGeoPainter Painter(_geoManager);
  Painter.Draw();
  static_cast<TView3D*>(gPad->GetView())->SetPsi(-75);
  gPad->GetView()->SetParallel();  // a nicer view
  std::stringstream path;
  path << _name << "/plots";
  SbtIO::createPath(path.str());
  path << "/GeoTest.pdf";
  can->SaveAs(path.str().c_str());
}

SbtDetectorElem* SbtConfig::getPhysicalDetector(int daqLayerSide) {
  int detID = _detectorMapArray[daqLayerSide] % 100;

  if (_debugLevel) {
    std::cout << "SbtConfig::GetPhysicalDetector() " << std::endl;
    std::cout << "_detectorMapArray[" << daqLayerSide
         << "] = " << _detectorMapArray[daqLayerSide] << std::endl;
    std::cout << "_detectorMapArray[" << daqLayerSide
         << "]%%100 = " << _detectorMapArray[daqLayerSide] % 100 << std::endl;
  }

  if (detID < 0 || detID > getNumberOfDetectors()) {
    if (_debugLevel) {
      std::cout << "SbtConfig::GetPhysicalDetector " << std::endl;
      std::cout << "daqLayerSide " << daqLayerSide
           << " not associated with any physical detector. Return NULL pointer"
           << std::endl;
    }

    return nullptr;
  }
  else {
    return getDetectorElemFromID(detID);
  }
}

SbtEnums::view SbtConfig::getSide(int daqLayerSide) {
  int intSide = _detectorMapArray[daqLayerSide] / 100;

  SbtEnums::view side = SbtEnums::U;
  if (intSide > 0) side = SbtEnums::V;

  return side;
}

std::vector<SbtDetectorElem*> SbtConfig::getTrackingDetectorElemList() {
  std::vector<SbtDetectorElem*> tracking_detectors;
  std::vector<SbtDetectorElem*>::iterator detector;

  for (detector = _detectorElems.begin(); detector != _detectorElems.end();
       detector++) {
    if ((*detector)->GetID() != -1) tracking_detectors.push_back(*detector);
  }

  return tracking_detectors;
}

SbtDetectorType* SbtConfig::getDetectorType(int t) {
  if (!(UInt_t(t) <= _detectorTypes.size())) {
    std::cout << "SbtConfig::getDetectorType "
         << " trying to acces a detector which is not in the list" << std::endl;
    assert(0);
  }
  SbtDetectorType* DetType = _detectorTypes.at(t - 1);
  return DetType;
}
