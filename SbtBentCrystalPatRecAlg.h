#ifndef SBTBENTCRYSTALPATRECALG_HH
#define SBTBENTCRYSTALPATRECALG_HH

#include <vector>
#include <TVector3.h>

#include "SbtDef.h"
#include "SbtPatRecAlg.h"

class SbtEvent;
class SbtTrack;
class SbtSpacePoint;
class SbtDetectorElem;
class TF1;

class SbtBentCrystalPatRecAlg : public SbtPatRecAlg {
 public:
  SbtBentCrystalPatRecAlg(const YAML::Node& config, std::vector<int> trackDetID);
  ~SbtBentCrystalPatRecAlg() {;}

 protected:
  bool isCandidateTrack(std::vector<std::vector<SbtSpacePoint*>::const_iterator> SPIter) const;
  bool isInsideTrkRoad(SbtSpacePoint* outerSpacePoint0, SbtSpacePoint* outerSpacePoint1, SbtSpacePoint* innerSpacePoint) const;
  void SortSpacePoints(std::vector<SbtSpacePoint*>& SPList) const;
  void SortDetectorElems(std::vector<SbtDetectorElem*>& DEList);
  int LoopOnSpacePoints(std::vector<std::vector<SbtSpacePoint*>::const_iterator>& SPIter, unsigned int k) const;
  int _linkHits();
  int _findLongTracks();
  int _findChanneledTracks();
  int _findDownStreamTracks();
  int _findUpStreamTracks();
  std::vector<std::array<SbtSpacePoint*,2>> _findDownstreamCandidates();
  std::vector<std::array<SbtSpacePoint*,2>> _findUpstreamCandidates();

  int _trkCounter;
  double _deflectionAngleThreshold;

  ClassDef(SbtBentCrystalPatRecAlg, 1);
};
#endif
