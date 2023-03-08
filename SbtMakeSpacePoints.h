#ifndef SBTMAKESPACEPOINTS_HH
#define SBTMAKESPACEPOINTS_HH

#include <vector>

class SbtEvent;
class SbtSpacePoint;

class SbtMakeSpacePoints {
 public:
  SbtMakeSpacePoints(std::string spErrMethod, double trackDetErr);
  virtual ~SbtMakeSpacePoints() {;}
  void setDebugLevel(int debugLevel) { _DebugLevel = debugLevel; }
  int getDebugLevel() const { return _DebugLevel; }

  void makeSpacePoints(SbtEvent* event);

 protected:
  int _DebugLevel;
  std::string _errorMethod;
  double _trackDetErr;  // nominal tracking error for spacePoints (used in the
                        // trk chi2 evalutation)

  ClassDef(SbtMakeSpacePoints, 1);
};

#endif
