#ifndef SBT_CONFIG
#define SBT_CONFIG

#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "SbtDef.h"
#include "SbtDetectorElem.h"
#include "SbtDetectorType.h"
#include "SbtEnums.h"

class SbtCrystalChanneling;
class SbtEventReader;
class TGeoManager;
class SbtMakeClusters;
class SbtMakeHits;
class SbtMakeSpacePoints;
class SbtMakeTracks;
class SbtNtupleDumper;

class SbtConfig {
 public:
  SbtConfig();
  SbtConfig(std::string configfile, std::string datafile="", bool ignoreAlign=false);
  virtual ~SbtConfig() {;}

  void configureFromYaml(std::string configfile, std::string datafile="");

  SbtDetectorType* getDetectorType(int i);
  SbtDetectorType* getDetectorTypeFromID(int i);
  SbtDetectorElem* getDetectorElemFromID(int i);
  std::vector<SbtDetectorElem*> getDetectorElemList() { return _detectorElems; }
  std::vector<SbtDetectorElem*> getTrackingDetectorElemList();
  int getNumberOfDetectors() { return _detectorElems.size(); }
  SbtDetectorElem* getPhysicalDetector(int daqLayerSide);
  SbtEnums::view getSide(int daqLayerSide);
  int getNLayerSides() { return _nLayerSides; };

  const char* getTopVolumeName() const { return _topVolName; }

  void drawGeom();
  TGeoManager* getGeoManager() { return _geoManager; }

  SbtMakeClusters* getMakeClusters() { return _makeClusters; }
  SbtMakeHits* getMakeHits() { return _makeHits; }
  SbtMakeSpacePoints* getMakeSpacePoints() { return _makeSpacePoints; }
  SbtMakeTracks* getMakeTracks() { return _makeTracks; }
  std::vector<int> getTrackingDetID() { return _trackDetID; }
  SbtNtupleDumper* getNtupleDumper() { return _ntupleDumper; }
  SbtEventReader* getEventReader() { return _eventReader; }

  std::string getDataType() const { return _ntupleFormat; }

  const std::string& getName() const { return _name; }

  SbtCrystalChanneling* getCrystalChanneling() const { return _crystalChanneling; }

 protected:
  // define private methods to instantiate detector lists
  void initializeGeoManager(const YAML::Node& conf);
  void configureDetectorTypeList(const YAML::Node& conf);
  void configureDetectorElemList(const YAML::Node& conf);
  void configureAlgorithms(const YAML::Node& conf);
  void configureTrackingDetectors(const YAML::Node& conf);
  void configureOutput();
  void configureDetectorMap(const YAML::Node& conf);
  void configureEventReader(const YAML::Node& conf);
  void configureCrystal(const YAML::Node& conf);

  int _debugLevel;
  std::vector<SbtDetectorType*> _detectorTypes;
  std::vector<SbtDetectorElem*> _detectorElems;
  int _detectorMapArray[nMaxLayerSides];

  std::string _name;

  TGeoManager* _geoManager;
  const char* _topVolName;  // the top volume name

  std::string _clusteringAlg;
  std::string _pixelClusteringOpt;  // Options  "Loose" (dx=+/-1, dy=+/-1);
                                    //          "Tight" (dx=+/-1,dy=0; dx=0,dy=+/-1)
  std::string _patRecAlg;
  std::string _fittingAlg;
  std::string _spErrMethod;

  double _patRecRoadWidth;  // Road Width (cm) for PatRecAlg
  TVector3 _trackOrigin;
  unsigned int _nTrackDet;             // number of tracking detectors
  std::vector<int> _trackDetID;  // vector of ID of tracking detectors
  double _trackDetErr;  // spacePoint errors for x, y for tracking detectors
  int _nLayerSides;     // number of DAQ layer sides
  std::string _rootOutFilename;
  std::string _ntupleFormat;
  bool _ignoreAlignment;

  SbtCrystalChanneling* _crystalChanneling;

  SbtMakeClusters* _makeClusters;
  SbtMakeHits* _makeHits;
  SbtMakeSpacePoints* _makeSpacePoints;
  SbtMakeTracks* _makeTracks;
  SbtNtupleDumper* _ntupleDumper;
  SbtEventReader* _eventReader;

  ClassDef(SbtConfig, 1);  // Implementation of TB Configuration
};

#endif
