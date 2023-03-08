#ifndef SBTSIMPLEPATRECALG_HH
#define SBTSIMPLEPATRECALG_HH

#include <vector>

#include "SbtDef.h"
#include "SbtPatRecAlg.h"

class SbtEvent;
class SbtTrack;
class SbtSpacePoint;
class SbtDetectorElem;

class SbtSimplePatRecAlg : public SbtPatRecAlg {
 public:
  SbtSimplePatRecAlg(const YAML::Node& config, std::vector<int> trackDetID);
  ~SbtSimplePatRecAlg() {;}

 protected:
  bool isCandidateTrack(SbtSpacePoint* outerSpacePoint0,
                        SbtSpacePoint* outerSpacePoint1,
                        SbtSpacePoint* innerSpacePoint0,
                        SbtSpacePoint* innerSpacePoint1);
  bool isInsideTrkRoad(SbtSpacePoint* outerSpacePoint0,
                       SbtSpacePoint* outerSpacePoint1,
                       SbtSpacePoint* innerSpacePoint);
  void SortSpacePoints(std::vector<SbtSpacePoint*>& SPList);
  int _linkHits();

  ClassDef(SbtSimplePatRecAlg, 1);
};

#endif
