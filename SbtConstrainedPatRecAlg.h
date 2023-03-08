#ifndef SBTCONSTRAINEDPATRECALG_HH
#define SBTCONSTRAINEDPATRECALG_HH

#include <vector>

#include <TVector3.h>

#include "SbtDef.h"
#include "SbtPatRecAlg.h"

class SbtEvent;
class SbtTrack;
class SbtSpacePoint;
class SbtDetectorElem;

class SbtConstrainedPatRecAlg : public SbtPatRecAlg {
 public:
  SbtConstrainedPatRecAlg(const YAML::Node& config, std::vector<int> trackDetID);
  ~SbtConstrainedPatRecAlg() {;}

 protected:
  TVector3 _origin;
  bool isInsideTrkRoad(const TVector3& x0, const TVector3& x3, const TVector3& xInternal);
  int _linkHits();

  ClassDef(SbtConstrainedPatRecAlg, 1);
};

#endif
