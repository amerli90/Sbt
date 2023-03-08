#ifndef SBTPATRECALG_HH
#define SBTPATRECALG_HH

#include <vector>

#include "SbtSpacePoint.h"
#include "SbtDef.h"

#include <yaml-cpp/yaml.h>

class SbtHit;
class SbtTrack;
class SbtEvent;

class SbtPatRecAlg {
 public:
  SbtPatRecAlg();
  SbtPatRecAlg(const YAML::Node& config, std::vector<int> trackDetID);
  virtual ~SbtPatRecAlg() {;}
  void setDebugLevel(int debugLevel) { _DebugLevel = debugLevel; }
  int getDebugLevel() const { return _DebugLevel; }

  int linkHits(SbtEvent* event);

  double getRoadWidth() const { return _roadWidth; }
  std::string getAlgName() const { return _algName; }

  void overrideRoadWidth(double roadWidth);

 protected:
  virtual int _linkHits() = 0;
  virtual int FindTelescopeDet(std::vector<SbtSpacePoint>& SpList) final;

  int _DebugLevel;
  double _roadWidth;  // road width for the candidate track
  std::string _algName;
  SbtEvent* _currentEvent;
  std::vector<int> _trackDetID;
  int _nTrackDet;
  std::vector<SbtSpacePoint*> _detSpacePointList[maxNTelescopeDetector];

  ClassDef(SbtPatRecAlg, 0);
};

#endif
