#ifndef SBTRECURSIVEPATRECALG_HH
#define SBTRECURSIVEPATRECALG_HH

#include <vector>

#include "SbtDef.h"
#include "SbtPatRecAlg.h"

class SbtEvent;
class SbtTrack;
class SbtSpacePoint;
class SbtDetectorElem;

class SbtRecursivePatRecAlg : public SbtPatRecAlg {
 public:
  SbtRecursivePatRecAlg(const YAML::Node& config, std::vector<int> trackDetID);
  ~SbtRecursivePatRecAlg() {;}

 protected:
  int _trkCounter;

  bool isCandidateTrack(std::vector<std::vector<SbtSpacePoint*>::iterator> SPIter);
  bool isInsideTrkRoad(SbtSpacePoint* outerSpacePoint0, SbtSpacePoint* outerSpacePoint1, SbtSpacePoint* innerSpacePoint);
  void SortSpacePoints(std::vector<SbtSpacePoint*>& SPList);
  void SortDetectorElems(std::vector<SbtDetectorElem*>& DEList);
  void LoopOnSpacePoints(std::vector<std::vector<SbtSpacePoint*>::iterator>& SPIter, unsigned int k);
  int _linkHits();

  ClassDef(SbtRecursivePatRecAlg, 1);
};
#endif
