#ifndef SBTNTUPLEDUMPER_HH
#define SBTNTUPLEDUMPER_HH

#include <string>
#include <vector>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

#include "SbtDef.h"
#include "SbtEnums.h"

class SbtConfig;
class TFile;
class SbtEvent;
class SbtTrack;
class SbtDigi;
class SbtCluster;
class SbtSpacePoint;
class SbtHit;

//
// Description
//
// class used to store analysis variables into a TTree
//

struct digiSum {
  SbtEnums::digiType type;
  int nstrips;
  int count;
  int ADC;
  void reset() {
    count = 0;
    ADC = 0;
  }
};

class SbtNtupleDumper {
 public:
  SbtNtupleDumper(std::string filename, SbtConfig* config);
  virtual ~SbtNtupleDumper();

  void setDebugLevel(int debugLevel) { _debugLevel = debugLevel; }

  void setNtupleFormat(std::string f) { _ntupleFormat = f; }

  void newRootFile(bool loopFile = false, std::string name = "");
  void loadEvent(SbtEvent* event);
  void write();

  void printEvent(const SbtEvent* event, int printLevel = 1);

 private:
  void initRootFile(std::string filename);
  void buildTree();
  void buildMCTree();
  
  void loadRecoEvent(const SbtEvent* evt);
  void loadMCevent(const SbtEvent* evt);
  void getTrackInfo(const SbtEvent* evt);
  UShort_t getSpacePointInfo(const SbtEvent* evt);
  void getIntersectionInfo(const SbtEvent* evt);
  void getClusterInfo(const std::vector<SbtCluster>& clusterList);
  void getDigiInfo(const std::vector<SbtDigi>& digiList);

  void fill();

  static bool digiLt(SbtDigi* d1, SbtDigi* d2);

  int _debugLevel;

  digiSum _digiSums[maxNDetector][2];
  SbtConfig* _config;

  std::string _rootFilename;
  std::string _ntupleFormat;

  TFile* _file;
  TTree* _tree;
  int _writeCounter;

  // define here the variables to store in the rootuple

  UShort_t _nDet;
  Int_t _nrun;                   // run number
  Int_t _nevent;                 // event number
  UInt_t _bco;           // bco counter
  UInt_t _trigger_type;  // trigger_type
  ULong64_t _timestamp;        // event timestamp
  UInt_t _TDCTime;

  // track related variables
  UInt_t _ntrk;       // number of tracks per event
  UInt_t _nIdealTrk;  // number of Ideal tracks per event
  UInt_t _nMsTrk;     // number of Ms tracks per event

  Short_t _trackType[maxNTrk];
  Short_t _trackShape[maxNTrk];
  Float_t _deflectionAngleX[maxNTrk];
  Float_t _deflectionAngleY[maxNTrk];
  Float_t _deflectionAngleXFourPoints[maxNTrk];
  Float_t _deflectionAngleYFourPoints[maxNTrk];
  UShort_t _nSPinTrk[maxNTrk];
  Float_t _chi2[maxNTrk];    // chi2 of track fit
  UShort_t _ndof[maxNTrk];    // number of degree of freedom of the track fit
  Short_t _fitStatus[maxNTrk];  // fit status
  Float_t _ItpX[maxNTrk];    // Track fit parameters
  Float_t _ItpY[maxNTrk];
  Float_t _SlpX[maxNTrk];
  Float_t _SlpY[maxNTrk];
  Float_t _SlpXTwoPoints[maxNTrk];
  Float_t _SlpYTwoPoints[maxNTrk];

  Float_t _XCov00[maxNTrk];  // XCov matrix of track fit
  Float_t _XCov01[maxNTrk];
  Float_t _XCov10[maxNTrk];
  Float_t _XCov11[maxNTrk];
  Float_t _YCov00[maxNTrk];  // YCov matrix of track fit
  Float_t _YCov01[maxNTrk];
  Float_t _YCov10[maxNTrk];
  Float_t _YCov11[maxNTrk];

  Float_t _Xreco[maxEvtNSpacePoint][maxNTrk];  // X reco
  Float_t _Yreco[maxEvtNSpacePoint][maxNTrk];  // Y reco

  Float_t _Xfit[maxEvtNSpacePoint][maxNTrk];
  Float_t _Yfit[maxEvtNSpacePoint][maxNTrk];

  Short_t _nUstrips[maxEvtNSpacePoint][maxNTrk];
  Short_t _nVstrips[maxEvtNSpacePoint][maxNTrk];

  // track-plane intersections
  UShort_t _nIntersect;
  Short_t _intTrkID[maxIntersects];
  Short_t _intLayer[maxIntersects];
  Short_t _intLayerType[maxIntersects];
  Short_t _intInside[maxIntersects];
  Float_t _intXPos[maxIntersects];
  Float_t _intYPos[maxIntersects];
  Float_t _intZPos[maxIntersects];
  Float_t _intUPos[maxIntersects];
  Float_t _intVPos[maxIntersects];

  // SpacePoints related variables
  UInt_t _nSP;  // number of space point per event
  Short_t _spType[maxEvtNSpacePoint];
  Short_t _spLayer[maxEvtNSpacePoint];
  Short_t _spLayerType[maxEvtNSpacePoint];
  Float_t _spXPos[maxEvtNSpacePoint];
  Float_t _spYPos[maxEvtNSpacePoint];
  Float_t _spZPos[maxEvtNSpacePoint];
  Float_t _spXErr[maxEvtNSpacePoint];
  Float_t _spYErr[maxEvtNSpacePoint];
  Float_t _spZErr[maxEvtNSpacePoint];
  Float_t _spUPos[maxEvtNSpacePoint];
  Float_t _spVPos[maxEvtNSpacePoint];
  Bool_t _spIsOnTrk[maxEvtNSpacePoint];

  // cluster related variables
  UInt_t _ncluster;                       // number of clusters per event
  Float_t _ClustPos[maxNClusters];      // cluster strip local position
  Float_t _ClustPxlUPos[maxNClusters];  // cluster pxl local position
  Float_t _ClustPxlVPos[maxNClusters];  // cluster pxl local position
  Float_t _ClustPH[maxNClusters];       // cluster pulse height
  Short_t _ClustSide[maxNClusters];        // cluster side
  Short_t _ClustSize[maxNClusters];        // cluster length
  Short_t _ClustTrk[maxNClusters];         // cluster associated track number
  Short_t _ClustLayer[maxNClusters];       // cluster Layer Side
  Short_t _ClustLayerType[maxNClusters];   // cluster Layer Side
  Bool_t _ClustIsOnTrk[maxNClusters];

  Short_t _ClustChannelMin[maxNClusters];
  Short_t _ClustChannelMax[maxNClusters];
  Short_t _ClustRowMin[maxNClusters];
  Short_t _ClustRowMax[maxNClusters];
  Short_t _ClustColumnMin[maxNClusters];
  Short_t _ClustColumnMax[maxNClusters];

  // digi related variables
  UInt_t _ndigi;  // number of digis per event
  Float_t _DigiPos[maxNDigis];
  Float_t _DigiPxlUPos[maxNDigis];
  Float_t _DigiPxlVPos[maxNDigis];
  UInt_t _DigiBCO[maxNDigis];
  Short_t _DigiLayer[maxNDigis];
  Short_t _DigiLayerType[maxNDigis];
  Short_t _DigiSide[maxNDigis];
  Float_t _DigiAdc[maxNDigis];
  Float_t _DigiThr[maxNDigis];
  Short_t _DigiStrip[maxNDigis];
  Short_t _DigiSet[maxNDigis];
  Short_t _DigiChip[maxNDigis];
  Short_t _DigiChan[maxNDigis];
  Short_t _DigiColumn[maxNDigis];
  Short_t _DigiRow[maxNDigis];
  Bool_t _DigiIsOnTrk[maxNDigis];

  // define here relevant variables for ideal and MC tracks

  Float_t _IdealItpX[maxNIdealTrk];  // ideal track parameters
  Float_t _IdealSlpX[maxNIdealTrk];
  Float_t _IdealItpY[maxNIdealTrk];
  Float_t _IdealSlpY[maxNIdealTrk];

  Float_t _IdealXminus[maxNIdealTrk];  // ideal track parameters
  Float_t _IdealXplus[maxNIdealTrk];
  Float_t _IdealYminus[maxNIdealTrk];
  Float_t _IdealYplus[maxNIdealTrk];

  UInt_t _nIdealSP;  // ideal track space point coordinates
  Short_t _IdealTrkID[maxNIdealSP];
  Short_t _IdealSPLayer[maxNIdealSP];
  Short_t _IdealSPLayerType[maxNIdealSP];
  Float_t _IdealXPos[maxNIdealSP];
  Float_t _IdealYPos[maxNIdealSP];
  Float_t _IdealZPos[maxNIdealSP];

  Float_t _MsItpX[maxNMsTrk];  // Ms track parameters
  Float_t _MsSlpX[maxNMsTrk];
  Float_t _MsItpY[maxNMsTrk];
  Float_t _MsSlpY[maxNMsTrk];
  Float_t _MsSimulatedSlpX[12][maxNMsTrk];  // slope x at each simulated material boundary
  Float_t _MsSimulatedSlpY[12][maxNMsTrk];  // slope y at each simulated material boundary
  Float_t _MsSimulatedPointX[12][maxNMsTrk];  // position x at each simulated material boundary
  Float_t _MsSimulatedPointY[12][maxNMsTrk];  // position y at each simulated material boundary
  Float_t _MsSimulatedPointZ[12][maxNMsTrk];  // position z at each simulated material boundary

  UInt_t _nMsSP;  // track space point coordinates with MS
  Short_t _MsTrkID[maxNMsSP];
  Short_t _MsSPLayer[maxNMsSP];
  Short_t _MsSPLayerType[maxNMsSP];
  Float_t _MsXPos[maxNMsSP];
  Float_t _MsYPos[maxNMsSP];
  Float_t _MsZPos[maxNMsSP];
  Float_t _MsXFitPos[maxNMsSP];
  Float_t _MsYFitPos[maxNMsSP];

  ClassDef(SbtNtupleDumper, 1)  // Implementation of NtupleDumper
};

#endif
