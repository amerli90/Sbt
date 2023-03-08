#ifndef SBT_MAKETRACKS
#define SBT_MAKETRACKS

#include <vector>
#include <string>

#include <yaml-cpp/yaml.h>

class SbtEvent;
class SbtTrack;
class SbtHit;
class SbtPatRecAlg;
class SbtFittingAlg;

class SbtMakeTracks {
 public:
  SbtMakeTracks(const YAML::Node& config, std::vector<int> trackDetID);
  virtual ~SbtMakeTracks();

  SbtPatRecAlg* getPatRecAlg() { return _patRecAlg; }
  SbtFittingAlg* getFittingAlg() { return _fittingAlg; }
  void setDebugLevel(int debugLevel) { _DebugLevel = debugLevel; }
  int getDebugLevel() const { return _DebugLevel; }

  void makeTracks(SbtEvent* event);

 protected:
  int _DebugLevel;
  SbtPatRecAlg* _patRecAlg;
  SbtFittingAlg* _fittingAlg;
  std::vector<int> _trackDetID;

  ClassDef(SbtMakeTracks, 1);
};



#endif
