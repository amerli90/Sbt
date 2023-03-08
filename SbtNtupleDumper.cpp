#include <cassert>
#include <iostream>
#include <stdexcept>

#include <TMath.h>

#include <SbtIO.h>
#include "SbtCluster.h"
#include "SbtConfig.h"
#include "SbtDetectorType.h"
#include "SbtDigi.h"
#include "SbtEvent.h"
#include "SbtFittingAlg.h"
#include "SbtHit.h"
#include "SbtMakeTracks.h"
#include "SbtNtupleDumper.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"

ClassImp(SbtNtupleDumper);

SbtNtupleDumper::SbtNtupleDumper(std::string filename, SbtConfig* config)
    : _debugLevel(0),
      _rootFilename(filename),
      _ntupleFormat(),
      _config(config) {
  std::cout << "SbtNtupleDumper: DebugLevel = " << _debugLevel << std::endl;
  if (_debugLevel) std::cout << "Ntuple Format = " << _ntupleFormat << std::endl;
  // define here the root output file where to store the TTree

  _writeCounter = 0;

  // initialize sums
  _nDet = _config->getNumberOfDetectors();
  for (unsigned int i = 0; i < _nDet; i++) {
    std::string type = _config->getDetectorElemFromID(i)->GetDetectorType()->GetType();

    if (type == "strip" || type == "striplet") {
      (_digiSums[i][0]).reset();
      (_digiSums[i][1]).reset();
      (_digiSums[i][0]).type = SbtEnums::strip;
      (_digiSums[i][1]).type = SbtEnums::strip;
      // ugh! inconsistent labeling of layer number
      (_digiSums[i][0]).nstrips = _config->getDetectorElemFromID(i)->GetDetectorType()->GetUnStrips();
      (_digiSums[i][1]).nstrips = _config->getDetectorElemFromID(i)->GetDetectorType()->GetVnStrips();
    }
    else if (type == "pixel") {
      (_digiSums[i][0]).reset();
      (_digiSums[i][0]).type = SbtEnums::pixel;
      (_digiSums[i][0]).nstrips = _config->getDetectorElemFromID(i)->GetDetectorType()->GetNPxl();
    }
    else if (type == "singleside") {
      (_digiSums[i][0]).reset();
      (_digiSums[i][0]).type = SbtEnums::strip;
      // ugh! inconsistent labeling of layer number
      (_digiSums[i][0]).nstrips = _config->getDetectorElemFromID(i)->GetDetectorType()->GetUnStrips();
    }
  }
}

SbtNtupleDumper::~SbtNtupleDumper() {
  if (_debugLevel) std::cout << "SbtNtupleDumper::~SbtNtupleDumper" << std::endl;
  delete _file;
  delete _tree;
}

void SbtNtupleDumper::initRootFile(std::string filename) {
  if (_debugLevel) std::cout << "SbtNtupleDumper::InitRootFile" << std::endl;
  std::string path = filename.substr(0, filename.find_last_of('/'));
  if (_debugLevel) std::cout << "Directory is " << path << std::endl;
  if (!SbtIO::createPath(path)) {
    std::cout << "Could not create path '" << path << "'." << std::endl;
    assert(0);
  }
  filename.append(".root");

  _file = new TFile(filename.c_str(), "recreate");
  _file->cd();
  _tree = new TTree("tree", "Test Beam Reconst ntuple");
  if (_ntupleFormat == "data") {
    buildTree();
  }
  else if (_ntupleFormat == "MC") {
    buildMCTree();
  }
  else {
    std::cout << "SbtNtupleDumper:  Ntuple format not valid" << std::endl;
    assert(0);
  }
}

void SbtNtupleDumper::write() {
  if (_debugLevel) std::cout << "SbtNtupleDumper::Write" << std::endl;
  _file->Write();
}

void SbtNtupleDumper::newRootFile(bool loopFile, std::string name) {
  if (_debugLevel) std::cout << "SbtNtupleDumper::NewRootFile" << std::endl;
  _writeCounter++;
  std::string filename = _rootFilename;
  if (loopFile) {
    char appendLabel[10];
    sprintf(appendLabel, "_L%d", _writeCounter);
    filename.append(appendLabel);
  }
  if (_ntupleFormat == "MC") {
    filename.append("_MC");
  }
  if (name.size() > 0) {
    filename.append("_");
    filename.append(name);
  }
  initRootFile(filename);
}

void SbtNtupleDumper::buildTree() {
  if (_debugLevel) std::cout << "SbtNtupleDumper::BuildTree" << std::endl;
  _tree->Branch("nrun", &_nrun, "nrun/I");
  _tree->Branch("nevent", &_nevent, "nevent/I");
  //_tree->Branch("bco", &_bco, "bco/i");
  //_tree->Branch("trigger_type", &_trigger_type, "trigger_type/i");
  //_tree->Branch("timestamp", &_timestamp, "timestamp/l");
  //_tree->Branch("TDCTime", &_TDCTime, "TDCTime/i");
  //_tree->Branch("ndet", &_nDet, "ndet/s");

  _tree->Branch("ntrk", &_ntrk, "ntrk/i");
  _tree->Branch("chi2", _chi2, "chi2[ntrk]/F");
  _tree->Branch("ndof", _ndof, "ndof[ntrk]/s");
  _tree->Branch("nSPinTrk", _nSPinTrk, "nSPinTrk[ntrk]/s");
  _tree->Branch("fitStatus", _fitStatus, "fitStatus[ntrk]/S");

  _tree->Branch("TrackType", _trackType, "TrackType[ntrk]/S");
  _tree->Branch("TrackShape", _trackShape, "TrackShape[ntrk]/S");

  _tree->Branch("DeflAngleX", _deflectionAngleX, "DeflAngleX[ntrk]/F");
  _tree->Branch("DeflAngleY", _deflectionAngleY, "DeflAngleY[ntrk]/F");
  _tree->Branch("DeflAngleXFourPoints", _deflectionAngleXFourPoints, "DeflAngleXFourPoints[ntrk]/F");
  _tree->Branch("DeflAngleYFourPoints", _deflectionAngleYFourPoints, "DeflAngleYFourPoints[ntrk]/F");
  _tree->Branch("ItpX", _ItpX, "ItpX[ntrk]/F");
  _tree->Branch("ItpY", _ItpY, "ItpY[ntrk]/F");
  _tree->Branch("SlpX", _SlpX, "SlpX[ntrk]/F");
  _tree->Branch("SlpY", _SlpY, "SlpY[ntrk]/F");
  _tree->Branch("SlpXTwoPoints", _SlpXTwoPoints, "SlpXTwoPoints[ntrk]/F");
  _tree->Branch("SlpYTwoPoints", _SlpYTwoPoints, "SlpYTwoPoints[ntrk]/F");

  //_tree->Branch("XCov00", _XCov00, "XCov00[ntrk]/F");
  //_tree->Branch("XCov01", _XCov01, "XCov01[ntrk]/F");
  //_tree->Branch("XCov10", _XCov10, "XCov10[ntrk]/F");
  //_tree->Branch("XCov11", _XCov11, "XCov11[ntrk]/F");

  //_tree->Branch("YCov00", _YCov00, "YCov00[ntrk]/F");
  //_tree->Branch("YCov01", _YCov01, "YCov01[ntrk]/F");
  //_tree->Branch("YCov10", _YCov10, "YCov10[ntrk]/F");
  //_tree->Branch("YCov11", _YCov11, "YCov11[ntrk]/F");

  for (int i = 0; i < _nDet; i++) {
    std::stringstream leaf;
    std::stringstream name;

    leaf.str(""); name.str("");
    leaf << "Xreco" << i << "[ntrk]/F";
    name << "Xreco" << i;
    _tree->Branch(name.str().c_str(), &_Xreco[i], leaf.str().c_str());

    leaf.str(""); name.str("");
    leaf << "Yreco" << i << "[ntrk]/F";
    name << "Yreco" << i;
    _tree->Branch(name.str().c_str(), &_Yreco[i], leaf.str().c_str());

    leaf.str(""); name.str("");
    leaf << "Xfit" << i << "[ntrk]/F";
    name << "Xfit" << i;
    _tree->Branch(name.str().c_str(), &_Xfit[i], leaf.str().c_str());

    leaf.str(""); name.str("");
    leaf << "Yfit" << i << "[ntrk]/F";
    name << "Yfit" << i;
    _tree->Branch(name.str().c_str(), &_Yfit[i], leaf.str().c_str());

    leaf.str(""); name.str("");
    leaf << "nUstrips" << i << "[ntrk]/S";
    name << "nUstrips" << i;
    _tree->Branch(name.str().c_str(), &_nUstrips[i], leaf.str().c_str());

    leaf.str(""); name.str("");
    leaf << "nVstrips" << i << "[ntrk]/S";
    name << "nVstrips" << i;
    _tree->Branch(name.str().c_str(), &_nVstrips[i], leaf.str().c_str());
  }

  // track-plane intersections
  _tree->Branch("nIntersect", &_nIntersect, "nIntersect/s");
  _tree->Branch("intTrkID", _intTrkID, "intTrkID[nIntersect]/S");
  _tree->Branch("intLayer", _intLayer, "intLayer[nIntersect]/S");
  _tree->Branch("intInside", _intInside, "intInside[nIntersect]/S");
  _tree->Branch("intLayerType", _intLayerType, "intLayerType[nIntersect]/S");
  _tree->Branch("intXPos", _intXPos, "intXPos[nIntersect]/F");
  _tree->Branch("intYPos", _intYPos, "intYPos[nIntersect]/F");
  _tree->Branch("intZPos", _intZPos, "intZPos[nIntersect]/F");
  _tree->Branch("intUPos", _intUPos, "intUPos[nIntersect]/F");
  _tree->Branch("intVPos", _intVPos, "intVPos[nIntersect]/F");

  // space points
  _tree->Branch("nSP", &_nSP, "nSP/i");
  _tree->Branch("spType", _spType, "spType[nSP]/S");
  _tree->Branch("spLayer", _spLayer, "spLayer[nSP]/S");
  _tree->Branch("spLayerType", _spLayerType, "spLayerType[nSP]/S");
  _tree->Branch("spXPos", _spXPos, "spXPos[nSP]/F");

  _tree->Branch("spYPos", _spYPos, "spYPos[nSP]/F");
  _tree->Branch("spZPos", _spZPos, "spZPos[nSP]/F");
  _tree->Branch("spXErr", _spXErr, "spXErr[nSP]/F");

  _tree->Branch("spYErr", _spYErr, "spYErr[nSP]/F");
  _tree->Branch("spZErr", _spZErr, "spZErr[nSP]/F");
  _tree->Branch("spUPos", _spUPos, "spUPos[nSP]/F");
  _tree->Branch("spVPos", _spVPos, "spVPos[nSP]/F");
  _tree->Branch("spIsOnTrk", _spIsOnTrk, "spIsOnTrk[nSP]/O");

  _tree->Branch("ncluster", &_ncluster, "ncluster/i");
  _tree->Branch("ClustPos", _ClustPos, "ClustPos[ncluster]/F");
  _tree->Branch("ClustPxlUPos", _ClustPxlUPos, "ClustPxlUPos[ncluster]/F");
  _tree->Branch("ClustPxlVPos", _ClustPxlVPos, "ClustPxlVPos[ncluster]/F");
  _tree->Branch("ClustPH", _ClustPH, "ClustPH[ncluster]/F");
  _tree->Branch("ClustSide", _ClustSide, "ClustSide[ncluster]/S");
  _tree->Branch("ClustSize", _ClustSize, "ClustSize[ncluster]/S");
  _tree->Branch("ClustLayer", _ClustLayer, "ClustLayer[ncluster]/S");
  _tree->Branch("ClustLayerType", _ClustLayerType, "ClustLayerType[ncluster]/S");
  _tree->Branch("ClustIsOnTrk", _ClustIsOnTrk, "ClustIsOnTrk[ncluster]/O");

  _tree->Branch("ClustChannelMin", _ClustChannelMin, "ClustChannelMin[ncluster]/S");
  _tree->Branch("ClustChannelMax", _ClustChannelMax, "ClustChannelMax[ncluster]/S");
  _tree->Branch("ClustRowMin", _ClustRowMin, "ClustRowMin[ncluster]/S");
  _tree->Branch("ClustRowMax", _ClustRowMax, "ClustRowMax[ncluster]/S");
  _tree->Branch("ClustColumnMin", _ClustColumnMin, "ClustColumnMin[ncluster]/S");
  _tree->Branch("ClustColumnMax", _ClustColumnMax, "ClustColumnMax[ncluster]/S");

  _tree->Branch("ndigi", &_ndigi, "ndigi/i");
  _tree->Branch("DigiPos", _DigiPos, "DigiPos[ndigi]/F");
  _tree->Branch("DigiPxlUPos", _DigiPxlUPos, "DigiPxlUPos[ndigi]/F");
  _tree->Branch("DigiPxlVPos", _DigiPxlVPos, "DigiPxlVPos[ndigi]/F");
  _tree->Branch("DigiBCO", _DigiBCO, "DigiBCO[ndigi]/l");
  _tree->Branch("DigiLayer", _DigiLayer, "DigiLayer[ndigi]/S");
  _tree->Branch("DigiLayerType", _DigiLayerType, "DigiLayerType[ndigi]/S");
  _tree->Branch("DigiSide", _DigiSide, "DigiSide[ndigi]/S");
  _tree->Branch("DigiAdc", _DigiAdc, "DigiAdc[ndigi]/F");
  _tree->Branch("DigiThr", _DigiThr, "DigiThr[ndigi]/F");
  _tree->Branch("DigiStrip", _DigiStrip, "DigiStrip[ndigi]/S");
  _tree->Branch("DigiSet", _DigiSet, "DigiSet[ndigi]/S");
  _tree->Branch("DigiChip", _DigiChip, "DigiChip[ndigi]/S");
  _tree->Branch("DigiChan", _DigiChan, "DigiChan[ndigi]/S");
  _tree->Branch("DigiColumn", _DigiColumn, "DigiColumn[ndigi]/S");
  _tree->Branch("DigiRow", _DigiRow, "DigiRow[ndigi]/S");
  _tree->Branch("DigiIsOnTrk", _DigiIsOnTrk, "DigiIsOnTrk[ndigi]/O");
}

void SbtNtupleDumper::buildMCTree() {
  if (_debugLevel) std::cout << "SbtNtupleDumper::BuildMCTree" << std::endl;

  buildTree();

  // add here MC information

  _tree->Branch("nIdealTrk", &_nIdealTrk, "nIdealTrk/i");
  _tree->Branch("IdealItpX", _IdealItpX, "IdealItpX[nIdealTrk]/F");
  _tree->Branch("IdealSlpX", _IdealSlpX, "IdealSlpX[nIdealTrk]/F");
  _tree->Branch("IdealItpY", _IdealItpY, "IdealItpY[nIdealTrk]/F");
  _tree->Branch("IdealSlpY", _IdealSlpY, "IdealSlpY[nIdealTrk]/F");

  _tree->Branch("nIdealSP", &_nIdealSP, "nIdealSP/i");
  _tree->Branch("IdealTrkID", _IdealTrkID, "IdealTrkID[nIdealSP]/S");
  _tree->Branch("IdealSPLayer", _IdealSPLayer, "IdealSPLayer[nIdealSP]/S");
  _tree->Branch("IdealSPLayerType", _IdealSPLayerType, "IdealSPLayerType[nIdealSP]/S");
  _tree->Branch("IdealXPos", _IdealXPos, "IdealXPos[nIdealSP]/F");
  _tree->Branch("IdealYPos", _IdealYPos, "IdealYPos[nIdealSP]/F");
  _tree->Branch("IdealZPos", _IdealZPos, "IdealZPos[nIdealSP]/F");

  _tree->Branch("nMsTrk", &_nMsTrk, "nMsTrk/I");
  _tree->Branch("MsItpX", _MsItpX, "MsItpX[nMsTrk]/F");
  _tree->Branch("MsSlpX", _MsSlpX, "MsSlpX[nMsTrk]/F");
  _tree->Branch("MsItpY", _MsItpY, "MsItpY[nMsTrk]/F");
  _tree->Branch("MsSlpY", _MsSlpY, "MsSlpY[nMsTrk]/F");

  std::stringstream leaf;
  std::stringstream name;
  for (int i = 0; i < 12; i++) {
    leaf.str(""); name.str("");
    leaf << "MsSimulatedSlpX" << i << "[nMsTrk]/F";
    name << "MsSimulatedSlpX" << i;
    _tree->Branch(name.str().c_str(), &_MsSimulatedSlpX[i], leaf.str().c_str());

    leaf.str(""); name.str("");
    leaf << "MsSimulatedSlpY" << i << "[nMsTrk]/F";
    name << "MsSimulatedSlpY" << i;
    _tree->Branch(name.str().c_str(), &_MsSimulatedSlpY[i], leaf.str().c_str());

    leaf.str(""); name.str("");
    leaf << "MsSimulatedPointX" << i << "[nMsTrk]/F";
    name << "MsSimulatedPointX" << i;
    _tree->Branch(name.str().c_str(), &_MsSimulatedPointX[i], leaf.str().c_str());

    leaf.str(""); name.str("");
    leaf << "MsSimulatedPointY" << i << "[nMsTrk]/F";
    name << "MsSimulatedPointY" << i;
    _tree->Branch(name.str().c_str(), &_MsSimulatedPointY[i], leaf.str().c_str());

    leaf.str(""); name.str("");
    leaf << "MsSimulatedPointZ" << i << "[nMsTrk]/F";
    name << "MsSimulatedPointZ" << i;
    _tree->Branch(name.str().c_str(), &_MsSimulatedPointZ[i], leaf.str().c_str());
  }

  _tree->Branch("nMsSP", &_nMsSP, "nMsSP/I");
  _tree->Branch("MsTrkID", _MsTrkID, "MsTrkID[nMsSP]/S");
  _tree->Branch("MsSPLayer", _MsSPLayer, "MsSPLayer[nMsSP]/S");
  _tree->Branch("MsSPLayerType", _MsSPLayerType, "MsSPLayerType[nMsSP]/S");
  _tree->Branch("MsXPos", _MsXPos, "MsXPos[nMsSP]/F");
  _tree->Branch("MsYPos", _MsYPos, "MsYPos[nMsSP]/F");
  _tree->Branch("MsZPos", _MsZPos, "MsZPos[nMsSP]/F");
  _tree->Branch("MsXFitPos", _MsXFitPos, "MsXFitPos[nMsSP]/F");
  _tree->Branch("MsYFitPos", _MsYFitPos, "MsYFitPos[nMsSP]/F");
}

void SbtNtupleDumper::loadEvent(SbtEvent* event) {
  if (_debugLevel) std::cout << "SbtNtupleDumper::loadEvent" << std::endl;
  _nrun = event->GetRunNumber();
  _nevent = event->GetEventNumber();
  _TDCTime = event->GetTDCTime();

  loadRecoEvent(event);

  if (_ntupleFormat == "MC") {
    loadMCevent(event);
  }

  fill();
}

void SbtNtupleDumper::loadRecoEvent(const SbtEvent* event) {
  if (_debugLevel) std::cout << "SbtNtupleDumper::loadRecoEvent" << std::endl;

  _timestamp = event->Gettimestamp();
  _bco = event->GetBCOCounter();
  _trigger_type = event->Gettrigger_type();

  _ncluster = event->GetPxlClusterList().size() + event->GetStripClusterList().size();
  _ndigi = event->GetPxlDigiList().size() + event->GetStripDigiList().size();
  _ntrk = event->GetTrackList().size();

  getTrackInfo(event);
  getIntersectionInfo(event);
  _nSP = getSpacePointInfo(event);
  getClusterInfo(event->GetPxlClusterList());
  getClusterInfo(event->GetStripClusterList());
  getDigiInfo(event->GetPxlDigiList());
  getDigiInfo(event->GetStripDigiList());
}

void SbtNtupleDumper::getTrackInfo(const SbtEvent* event) {
  if (_debugLevel) std::cout << "SbtNtupleDumper::GetTrackInfo" << std::endl;

  // loop on the track objects

  int IdxTrk = 0;
  if (event->GetTrackList().size() > maxNTrk) {
    std::cout << "SbtNtupleDumper::GetTrackInfo: too many tracks: "
              << event->GetTrackList().size() << std::endl;
  }
  // assert number of tracks in the event < maxNTrk
  assert(event->GetTrackList().size() < maxNTrk);

  for (const auto& track : event->GetTrackList()) {
    _nSPinTrk[IdxTrk] = track.GetSpacePointList().size();
    _chi2[IdxTrk] = track.GetChi2();
    _ndof[IdxTrk] = track.GetNdof();
    _fitStatus[IdxTrk] = track.GetFitStatus();
    _ItpX[IdxTrk] = track.GetInterceptX();
    _SlpX[IdxTrk] = track.GetSlopeX();
    _ItpY[IdxTrk] = track.GetInterceptY();
    _SlpY[IdxTrk] = track.GetSlopeY();
    _SlpXTwoPoints[IdxTrk] = track.GetSlopeXTwoPoints();
    _SlpYTwoPoints[IdxTrk] = track.GetSlopeYTwoPoints();
    _trackType[IdxTrk] = track.GetType();
    _trackShape[IdxTrk] = track.GetShape();
    _deflectionAngleX[IdxTrk] = track.GetDeflectionAngleX();
    _deflectionAngleY[IdxTrk] = track.GetDeflectionAngleY();
    _deflectionAngleXFourPoints[IdxTrk] = track.GetDeflectionAngleXFourPoints();
    _deflectionAngleYFourPoints[IdxTrk] = track.GetDeflectionAngleYFourPoints();

    if (track.GetXCovMatrix().GetNcols() >= 2 && track.GetXCovMatrix().GetNrows() >= 2) {
      _XCov00[IdxTrk] = track.GetXCovMatrix()[0][0];
      _XCov01[IdxTrk] = track.GetXCovMatrix()[0][1];
      _XCov10[IdxTrk] = track.GetXCovMatrix()[1][0];
      _XCov11[IdxTrk] = track.GetXCovMatrix()[1][1];
    }
    if (track.GetYCovMatrix().GetNcols() >= 2 && track.GetYCovMatrix().GetNrows() >= 2) {
      _YCov00[IdxTrk] = track.GetYCovMatrix()[0][0];
      _YCov01[IdxTrk] = track.GetYCovMatrix()[0][1];
      _YCov10[IdxTrk] = track.GetYCovMatrix()[1][0];
      _YCov11[IdxTrk] = track.GetYCovMatrix()[1][1];
    }

    for (int i = 0; i < _nSPinTrk[IdxTrk]; i++) {
      _Xreco[i][IdxTrk] = track.GetXReco(i);
      _Yreco[i][IdxTrk] = track.GetYReco(i);
      _Xfit[i][IdxTrk] = track.GetXFit(i);
      _Yfit[i][IdxTrk] = track.GetYFit(i);
      _nUstrips[i][IdxTrk] = track.GetSpacePointList()[i]->GetHitU()->GetCluster()->GetLength();
      _nVstrips[i][IdxTrk] = track.GetSpacePointList()[i]->GetHitV()->GetCluster()->GetLength();
    }

    if (_debugLevel > 1) {
      std::cout << "SbtNtupleDumper::GetTrakInfo  Debugging " << std::endl;
      std::cout << "Chi2 =  " << track.GetChi2() << std::endl;
    }
    IdxTrk++;
  }
  if (_debugLevel > 2) {
    std::cout << "Done with get tracking info" << std::endl;
  }
}

void SbtNtupleDumper::getClusterInfo(const std::vector<SbtCluster>& clusterList) {
  if (_debugLevel) std::cout << "SbtNtupleDumper::GetClusterInfo" << std::endl;

  // loop on the strip cluster objects

  int IdxCluster = 0;

  for (const auto& cluster : clusterList) {
    // assert number of tracks in the event < maxNTrk
    assert(IdxCluster < maxNClusters);

    // access the digis to get min/max digi positions
    std::vector<SbtDigi*> digiList = cluster.GetDigiList();

    int digiMin, digiMax, digiUMin, digiUMax, digiVMin, digiVMax;
    digiMin = digiMax = digiUMin = digiUMax = digiVMin = digiVMax = -1;

    int digiCount = 0;
    for (auto digi : digiList) {
      if (digi->GetType() == SbtEnums::pixel) {  // pixels
        int digiU = digi->GetColumn();
        int digiV = digi->GetRow();

        if (digiCount == 0) {
          digiUMin = digiU;
          digiUMax = digiU;
          digiVMin = digiV;
          digiVMax = digiV;
        } else {
          if (digiU < digiUMin) digiUMin = digiU;
          if (digiU > digiUMax) digiUMax = digiU;
          if (digiV < digiVMin) digiVMin = digiV;
          if (digiV > digiVMax) digiVMax = digiV;
        }
      } else {  // strips
        int channel = digi->GetChannelNumber();
        if (digiCount == 0) {
          digiMin = channel;
          digiMax = channel;
        } else {
          if (channel < digiMin) digiMin = channel;
          if (channel > digiMax) digiMax = channel;
        }
      }
      digiCount++;
    }

    _ClustChannelMin[IdxCluster] = digiMin;
    _ClustChannelMax[IdxCluster] = digiMax;
    _ClustRowMin[IdxCluster] = digiVMin;
    _ClustRowMax[IdxCluster] = digiVMax;
    _ClustColumnMin[IdxCluster] = digiUMin;
    _ClustColumnMax[IdxCluster] = digiUMax;

    if (cluster.GetDetectorElem()->GetDetectorType()->GetType() == "pixel") {
      _ClustPH[IdxCluster] = -1;
      _ClustSide[IdxCluster] = -1;
      _ClustPos[IdxCluster] = -999.0;
      _ClustPxlUPos[IdxCluster] = cluster.GetPxlUPosition();
      _ClustPxlVPos[IdxCluster] = cluster.GetPxlVPosition();
    } else {
      _ClustPH[IdxCluster] = cluster.GetPulseHeight();
      if (_debugLevel) std::cout << "_ClustPH: " << _ClustPH[IdxCluster] << std::endl;
      _ClustSide[IdxCluster] = cluster.GetSide();
      _ClustPos[IdxCluster] = cluster.GetPosition();
      _ClustPxlUPos[IdxCluster] = -999.0;
      _ClustPxlVPos[IdxCluster] = -999.0;
    }
    _ClustSize[IdxCluster] = cluster.GetLength();
    _ClustLayer[IdxCluster] = cluster.GetDetectorElem()->GetID();
    _ClustLayerType[IdxCluster] =
        cluster.GetDetectorElem()->GetDetectorType()->GetIntType();
    _ClustIsOnTrk[IdxCluster] = cluster.IsOnTrack();

    if (_debugLevel > 1) {
      std::cout << "SbtNtupleDumper::GetClusterInfo  Debugging " << std::endl;
      std::cout << "ClusterSize =  " << cluster.GetLength() <<std:: endl;
    }
    IdxCluster++;
  }
}

void SbtNtupleDumper::getDigiInfo(const std::vector<SbtDigi>& digiList) {
  if (_debugLevel) std::cout << "SbtNtupleDumper::GetDigiInfo" << std::endl;

  int IdxDigi = 0;

  for (const auto& digi : digiList) {
    // assert number of tracks in the event < maxNTrk
    assert(IdxDigi < maxNDigis);

    if (digi.GetDetectorElem()->GetDetectorType()->GetType() ==
        "pixel") {
      _DigiAdc[IdxDigi] = -1;
      _DigiThr[IdxDigi] = -1;
      _DigiSide[IdxDigi] = -1;
      _DigiPos[IdxDigi] = -999.0;
      double pos[2];
      digi.Position(pos);
      _DigiPxlUPos[IdxDigi] = pos[0];
      _DigiPxlVPos[IdxDigi] = pos[1];
      _DigiSet[IdxDigi] = -1;
      _DigiStrip[IdxDigi] = -1;
      _DigiChip[IdxDigi] = -1;
      _DigiChan[IdxDigi] = -1;
      _DigiColumn[IdxDigi] = digi.GetColumn();
      _DigiRow[IdxDigi] = digi.GetRow();

    } else {
      _DigiAdc[IdxDigi] = digi.GetPulseHeight();
      _DigiThr[IdxDigi] = digi.GetThr();
      _DigiSide[IdxDigi] = digi.GetSide();
      _DigiPos[IdxDigi] = digi.Position();
      _DigiPxlUPos[IdxDigi] = -999.0;
      _DigiPxlVPos[IdxDigi] = -999.0;
      _DigiSet[IdxDigi] = digi.GetSet();
      _DigiStrip[IdxDigi] = digi.GetStrip();
      _DigiChip[IdxDigi] = digi.GetChip();
      _DigiChan[IdxDigi] = digi.GetChannelNumber();
      _DigiColumn[IdxDigi] = -1;
      _DigiRow[IdxDigi] = -1;
    }
    _DigiLayer[IdxDigi] = digi.GetDetectorElem()->GetID();
    _DigiLayerType[IdxDigi] =
        digi.GetDetectorElem()->GetDetectorType()->GetIntType();
    _DigiBCO[IdxDigi] = digi.GetBCO();
    _DigiIsOnTrk[IdxDigi] = digi.IsOnTrack();

    if (_debugLevel > 1) {
      std::cout << "SbtNtupleDumper::GetDigiInfo  Debugging " << std::endl;
      std::cout << "number of digi  = " << _ndigi << std::endl;
    }

    IdxDigi++;
  }
}

UShort_t SbtNtupleDumper::getSpacePointInfo(const SbtEvent* event) {
  if (_debugLevel) std::cout << "SbtNtupleDumper::GetSpacePointInfo" << std::endl;
  // loop on the space point objects
  UShort_t IdxSP = 0;
  for (const auto& sp : event->GetSpacePointList()) {
    assert(IdxSP < maxEvtNSpacePoint);

    if (!sp.GetDetectorElem()) continue;
    if (sp.GetSpacePointType() != SbtEnums::objectType::reconstructed) continue;
    _spLayer[IdxSP] = sp.GetDetectorElem()->GetID();
    _spLayerType[IdxSP] = sp.GetDetectorElem()->GetDetectorType()->GetIntType();
    _spXPos[IdxSP] = sp.GetXPosition();
    _spYPos[IdxSP] = sp.GetYPosition();
    _spZPos[IdxSP] = sp.GetZPosition();
    _spXErr[IdxSP] = sp.GetXPositionErr();
    _spYErr[IdxSP] = sp.GetYPositionErr();
    _spZErr[IdxSP] = sp.GetZPositionErr();
    _spIsOnTrk[IdxSP] = sp.IsOnTrack();
  
    // get local coordinates
    double U = -999.;
    double V = -999.;

    if (SbtEnums::digiType::strip == sp.GetDigitType()) {
      if (sp.GetDetectorElem()->GetDetectorType()->GetType() == "singleside") {
        if (sp.GetHitU()) U = sp.GetHitU()->GetCluster()->GetPosition();
      }
      else {
        if (sp.GetHitU()) U = sp.GetHitU()->GetCluster()->GetPosition();
        if (sp.GetHitV()) V = sp.GetHitV()->GetCluster()->GetPosition();
      }
    }
    else if (SbtEnums::digiType::pixel == sp.GetDigitType()) {
      U = sp.GetPxlCluster()->GetPxlUPosition();
      V = sp.GetPxlCluster()->GetPxlVPosition();
    }

    _spUPos[IdxSP] = U;
    _spVPos[IdxSP] = V;

    if (_debugLevel > 1) {
      std::cout << "SbtNtupleDumper::GetSpacePoint  Debugging " << std::endl;
      std::cout << "number of SpacePoint  = " << IdxSP << std::endl;
    }

    IdxSP++;
  }
  return IdxSP;
}

void SbtNtupleDumper::getIntersectionInfo(const SbtEvent* event) {
  if (_debugLevel) std::cout << "SbtNtupleDumper::GetIntersectionInfo" << std::endl;

  // loop over tracks

  int intIdx = 0;

  for (unsigned int itk = 0; itk < event->GetTrackList().size(); itk++) {
    if (event->GetTrackList().at(itk).GetFitStatus() <= 0) continue;
    // loop over layers
    for (unsigned int idet = 0; idet < _nDet; idet++) {
      SbtDetectorElem* detElem = _config->getDetectorElemFromID(idet);

      if (_debugLevel > 0) {
        std::cout << "SbtNtupleDumper::GetIntersectionInfo" << std::endl;
        std::cout << "DetElemID = " << detElem->GetID() << std::endl;
        std::cout << "X position =  " << detElem->GetXPos() << std::endl;
        std::cout << "Y position =  " << detElem->GetYPos() << std::endl;
        std::cout << "Z position =  " << detElem->GetZPos() << std::endl;
      }

      // get intersection point (in global) coordinates
      TVector3 intPoint;
      bool ok = event->GetTrackList().at(itk).IntersectPlane(detElem, intPoint);

      // get local version
      TVector3 intPointLocal;

      detElem->MasterToLocal(intPoint, intPointLocal);

      _intTrkID[intIdx] = itk;
      _intLayer[intIdx] = detElem->GetID();
      _intInside[intIdx] = (ok) ? 1 : 0;
      _intLayerType[intIdx] = detElem->GetDetectorType()->GetIntType();

      _intXPos[intIdx] = intPoint[0];
      _intYPos[intIdx] = intPoint[1];
      _intZPos[intIdx] = intPoint[2];

      _intUPos[intIdx] = intPointLocal[0];
      _intVPos[intIdx] = intPointLocal[1];
      intIdx++;

      if (intIdx >= maxIntersects) {
        std::cout << "SbtNtupleDumper: maxIntersects exceeded: " << intIdx << std::endl;
        assert(intIdx < maxIntersects);
      }
    }
  }
  _nIntersect = intIdx;
}

void SbtNtupleDumper::loadMCevent(const SbtEvent* event) {
  if (_debugLevel) {
    std::cout << "SbtNtupleDumper::loadMCevent " << std::endl;
  }

  _nIdealTrk = event->GetIdealTrackList().size();
  _nMsTrk = event->GetMCTrackList().size();

  int IdxIdealTrk = 0;
  int IdxIdealSP = 0;
  for (const auto& IdealTrk : event->GetIdealTrackList()) {
    if (_debugLevel) {
      std::cout << "IdxIdealTrk : " << IdxIdealTrk << std::endl;
      std::cout << "maxNIdealTrk: " << maxNIdealTrk << std::endl;
    }
    // assert number of tracks in the event < maxNIdealTrk
    assert(IdxIdealTrk < maxNIdealTrk);

    _IdealItpX[IdxIdealTrk] = IdealTrk.GetAx();
    _IdealSlpX[IdxIdealTrk] = IdealTrk.GetBx();
    _IdealItpY[IdxIdealTrk] = IdealTrk.GetAy();
    _IdealSlpY[IdxIdealTrk] = IdealTrk.GetBy();

    if (_debugLevel > 0) {
      std::cout << "SbtNtupleDumper::loadMCevent :" << std::endl;
      std::cout << "\tIdealTrkSP.size(): " << IdealTrk.GetSpacePointList().size() << std::endl;
    }

    for (auto IdealSP : IdealTrk.GetSpacePointList()) {
      if (_debugLevel > 5) IdealSP->print();
      if (IdealSP->GetDetectorElem()) {
        _IdealSPLayer[IdxIdealSP] = IdealSP->GetDetectorElem()->GetID();
        _IdealSPLayerType[IdxIdealSP] = IdealSP->GetDetectorElem()->GetDetectorType()->GetIntType();
      }
      else {
        _IdealSPLayer[IdxIdealSP] = -1;
        _IdealSPLayerType[IdxIdealSP] = -1;
      }
      _IdealXPos[IdxIdealSP] = IdealSP->GetXPosition();
      _IdealYPos[IdxIdealSP] = IdealSP->GetYPosition();
      _IdealZPos[IdxIdealSP] = IdealSP->GetZPosition();

      IdxIdealSP++;
    }
    IdxIdealTrk++;
  }
  _nIdealSP = IdxIdealSP;

  int IdxMsTrk = 0;
  int IdxMsSP = 0;
  for (const auto& MsTrk : event->GetMCTrackList()) {
    if (_debugLevel) {
      std::cout << "IdxMsTrk : " << IdxMsTrk << std::endl;
      std::cout << "maxNMsTrk: " << maxNMsTrk << std::endl;
      std::cout << "Expected boundaires: " << MsTrk.GetSpacePointList().size()+2 << std::endl;
      std::cout << "Found boundaires: " << MsTrk.GetNumberOfSimNodes() << std::endl;
    }
    // retrieve the Ms track SpacePoint List
    // the SpacePoint are already ordered by z

    // assert number of tracks in the event < maxNMsTrk
    assert(IdxMsTrk < maxNMsTrk);

    _MsItpX[IdxMsTrk] = MsTrk.GetAx();
    _MsSlpX[IdxMsTrk] = MsTrk.GetBx();
    _MsItpY[IdxMsTrk] = MsTrk.GetAy();
    _MsSlpY[IdxMsTrk] = MsTrk.GetBy();

    for (int i = 0; i < 12; i++) {
      if (i < MsTrk.GetNumberOfSimNodes()) {
        _MsSimulatedSlpX[i][IdxMsTrk] = MsTrk.GetSlpXAtNode(i);
        _MsSimulatedSlpY[i][IdxMsTrk] = MsTrk.GetSlpYAtNode(i);
        _MsSimulatedPointX[i][IdxMsTrk] = MsTrk.GetPointXAtNode(i);
        _MsSimulatedPointY[i][IdxMsTrk] = MsTrk.GetPointYAtNode(i);
        _MsSimulatedPointZ[i][IdxMsTrk] = MsTrk.GetPointZAtNode(i);
      }
      else {
        _MsSimulatedSlpX[i][IdxMsTrk] = -999;
        _MsSimulatedSlpY[i][IdxMsTrk] = -999;
        _MsSimulatedPointX[i][IdxMsTrk] = -999;
        _MsSimulatedPointY[i][IdxMsTrk] = -999;
        _MsSimulatedPointZ[i][IdxMsTrk] = -999;
      }
    }

    for (auto Mssp : MsTrk.GetSpacePointList()) {
      if (Mssp->GetDetectorElem()) {
        _MsSPLayer[IdxMsSP] = Mssp->GetDetectorElem()->GetID();
        _MsSPLayerType[IdxMsSP] = Mssp->GetDetectorElem()->GetDetectorType()->GetIntType();
      }
      else {
        _MsSPLayer[IdxMsSP] = -1;
        _MsSPLayerType[IdxMsSP] = -1;
      }
      _MsXPos[IdxMsSP] = Mssp->GetXPosition();
      _MsYPos[IdxMsSP] = Mssp->GetYPosition();
      _MsZPos[IdxMsSP] = Mssp->GetZPosition();
      _MsXFitPos[IdxMsSP] = _MsItpX[IdxMsTrk] + _MsSlpX[IdxMsTrk] * Mssp->GetZPosition();
      _MsYFitPos[IdxMsSP] = _MsItpY[IdxMsTrk] + _MsSlpY[IdxMsTrk] * Mssp->GetZPosition();
      IdxMsSP++;
    }
    IdxMsTrk++;
  }
  _nMsSP = IdxMsSP;

  if (_debugLevel) std::cout  << "MC event processed." << std::endl;
}

void SbtNtupleDumper::fill() {
  if (_debugLevel) std::cout << "SbtNtupleDumper::Fill" << std::endl;
  _tree->Fill();
};

//
// print event
//
void SbtNtupleDumper::printEvent(const SbtEvent* event, int printLevel) {
  if (_debugLevel) std::cout << "SbtNtupleDumper::PrintEvent" << std::endl;

  int runNumber = event->GetRunNumber();
  int eventNumber = event->GetEventNumber();

  std::cout << "\n\n---------- Run: " << runNumber
            << " --------- Event: " << eventNumber << std::endl;

  printf("\n%30s %zu \n%30s %zu \n%30s %zu\n%30s %zu\n%30s %zu\n",
         "Event totals: strip digis:", event->GetStripDigiList().size(),
         "              pixel digis:", event->GetPxlDigiList().size(),
         "           strip clusters:", event->GetStripClusterList().size(),
         "           pixel clusters:", event->GetPxlClusterList().size(),
         "                 tracks:", event->GetTrackList().size());

  // -------------- prinout quantities per layer ----------------------
  // first reset counters
  for (unsigned int i = 0; i < _nDet; i++) {
    for (unsigned int j = 0; j < 2; j++) {
      _digiSums[i][j].reset();
    }
  }

  for (auto digi : event->GetPxlDigiList()) {
    int ilay = digi.GetLayer();

    if (SbtEnums::strip == digi.GetType()) {
      int iside = (digi.GetSide() == SbtEnums::U) ? 0 : 1;
      _digiSums[ilay][iside].count++;
      _digiSums[ilay][iside].ADC += digi.GetADC();
    } else {
      _digiSums[ilay][0].count++;
    }
  }

  for (auto digi : event->GetStripDigiList()) {
    int ilay = digi.GetLayer();

    if (SbtEnums::strip == digi.GetType()) {
      int iside = (digi.GetSide() == SbtEnums::U) ? 0 : 1;
      _digiSums[ilay][iside].count++;
      _digiSums[ilay][iside].ADC += digi.GetADC();
    } else {
      _digiSums[ilay][0].count++;
    }
  }

  // print header
  std::string labels[maxNDetector];
  for (int i = 0; i < maxNDetector; i++) {
    labels[i] = Form("Det%d", i);
  }
  std::cout << std::endl;

  for (int i = 0; i < 20; i++) std::cout << " ";

  for (unsigned int i = 0; i < _nDet; i++) {
    std::cout << labels[i] << "            ";  // 12 spaces
  }
  std::cout << std::endl;

  std::cout << "                   ";  // 19 spaces
  for (unsigned int i = 0; i < _nDet; i++) {
    if (_digiSums[i][0].type == SbtEnums::strip) {
      std::cout << "U      V         ";
    } else {
      std::cout << "                 ";
    }
  }
  std::cout << std::endl;

  // -- number of digis ---
  printf("%14s", "NDigis   ");
  for (unsigned int i = 0; i < _nDet; i++) {
    if (_digiSums[i][0].type == SbtEnums::strip) {
      printf("%6d %6d    ", _digiSums[i][0].count, _digiSums[i][1].count);
    } else {
      printf("    %6d", _digiSums[i][0].count);
    }
  }
  std::cout << std::endl;

  // -- occupancy ---
  printf("%14s", "Occupancy   ");
  for (unsigned int i = 0; i < _nDet; i++) {
    if (_digiSums[i][0].type == SbtEnums::strip) {
      printf("%6.3f %6.3f    ",
             (double)_digiSums[i][0].count / (double)_digiSums[i][0].nstrips,
             (double)_digiSums[i][1].count / (double)_digiSums[i][1].nstrips);
    } else {
      printf("    %6.3f",
             (double)_digiSums[i][0].count / (double)_digiSums[i][0].nstrips);
    }
  }
  std::cout << std::endl;

  // -- avg adc ---
  printf("%14s", "Avg. ADC   ");
  for (unsigned int i = 0; i < _nDet; i++) {
    if (_digiSums[i][0].type == SbtEnums::strip) {
      double adc0 =
          _digiSums[i][0].count > 0
              ? (double)_digiSums[i][0].ADC / (double)_digiSums[i][0].count
              : 0.;
      double adc1 =
          _digiSums[i][1].count > 0
              ? (double)_digiSums[i][1].ADC / (double)_digiSums[i][1].count
              : 0.;
      printf("%6.3f %6.3f    ", adc0, adc1);
    } else {
      printf("    %6s", "NA");
    }
  }
  std::cout << std::endl << std::endl;
}

bool SbtNtupleDumper::digiLt(SbtDigi* d1, SbtDigi* d2) {
  if (d1->GetDetectorElem()->GetZPos() < d2->GetDetectorElem()->GetZPos()) {
    return true;
  }
  return false;
}
