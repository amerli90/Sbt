#ifndef SBTGENERATOR_HH
#define SBTGENERATOR_HH

#include <cassert>
#include <cmath>
#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

#include <TDatime.h>
#include <TTimeStamp.h>
#include <TRandom3.h>

#include "SbtConfig.h"
#include "SbtDef.h"
#include "SbtDigi.h"
#include "SbtEnums.h"
#include "SbtGenAlg.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"

class SbtEvent;

class SbtGenerator {
 public:
  SbtGenerator();
  SbtGenerator(SbtConfig* configurator, std::string configfile, unsigned int seed = 0);
  ~SbtGenerator() {;}

  void ReadConfiguration(std::string configFileName);

  void setDebugLevel(int debugLevel) { _DebugLevel = debugLevel; }
  int getDebugLevel() { return _DebugLevel; }

  void SetConfigurator(SbtConfig* aConfigurator);

  void SetStripOcc(double stripOcc) { _stripOccupancy = stripOcc; }
  void SetPxlOcc(double pxlOcc) { _pxlOccupancy = pxlOcc; }
  void SetAvgNoiseAdcVal(int ANAV) { _avgNoiseAdc = ANAV; }
  void SetNTracks(int nT) { _nTracks = nT; }
  SbtGenAlg* GetGenAlg() { return _theGenAlg; }

  void GenerateTracks(SbtEvent& event);

  void GenerateStripNoise(std::vector<SbtDigi>& digis);
  void GeneratePxlNoise(std::vector<SbtDigi>& digis);

  word GenerateEventHeader(bool isDut) { return (isDut) ? aStartWord1 : aStartWord0; }

  word GenerateEventCounter() { return _GenEventCounter; }  // temporary

  static word GenerateBCOCounter();

  static word GenerateClockCounter();

  word GetLayerHeader(int layerNumber);

  void GetScintillatorsWords(std::vector<word>& scintWords);

  word GenerateEndWord() { return (word)0xb1eb1e0f; }

  word GenerateCheckWord(SbtEvent* anEvent);

  bool GenerateEvent(SbtEvent& event, SbtEnums::eventType = SbtEnums::normal);

  word GenerateRunNumber();

  void SetRunNumber(word i) { _runNumber = i; }

  const std::string& getName() { return _name; }

 protected:
  void ReadBeamParameters(const YAML::Node& config);   // read beam parameters
  void ReadNoiseParameters(const YAML::Node& config);  // read noise parameters
  void ReadTrackParameters(const YAML::Node& config);  // read track parameters
  void ReadDetectorParameters(const YAML::Node& config);  // read detector parameters
  void ReadGenAlg(const YAML::Node& config);  // read the name of the generator algo
  void ReadMultipleScatteringParameters(const YAML::Node& config);

  int _DebugLevel;
  SbtConfig* _theConfigurator;  // the telescope configurator
  std::string _genAlgName;      // the generator algo name
  std::string _name;

  double _stripOccupancy;  // strip occupancy (%)
  double _pxlOccupancy;    // pxl occupancy (%)
  int _avgNoiseAdc;        // average adc noise level

  int _nTracks;  // average # of track per event

  static word _GenEventCounter;  // the event counter

  SbtGenAlg* _theGenAlg;  // the generator algo

  TRandom* _rN;  // the random generator

  word _runNumber;  // run number

  ClassDef(SbtGenerator, 1);
};

#endif
