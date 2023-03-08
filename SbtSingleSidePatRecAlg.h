#ifndef SBTSINGLESIDEPATRECALG_HH
#define SBTSINGLESIDEPATRECALG_HH

#include <vector>

#include "SbtDef.h"
#include "SbtPatRecAlg.h"

class SbtEvent;
class SbtTrack;
class SbtSpacePoint;
class SbtDetectorElem;

class SbtSingleSidePatRecAlg : public SbtPatRecAlg {
 public:
  SbtSingleSidePatRecAlg(const YAML::Node& config, std::vector<int> trackDetID);
  ~SbtSingleSidePatRecAlg() {;}

 protected:
  unsigned int _nTrackDet_X;
  unsigned int _nTrackDet_Y;
  int _trkCounter;
  int _trkCounter_X;
  int _trkCounter_Y;

  std::vector<SbtTrack*> _SingleSidePatRecTrackList;
  std::vector<SbtTrack*> _SingleSidePatRecTrackList_X;
  std::vector<SbtTrack*> _SingleSidePatRecTrackList_Y;
  std::vector<int> _trackDetID_X;
  std::vector<int> _trackDetID_Y;

  bool isCandidateTrack(std::vector<std::vector<SbtSpacePoint*>::iterator> SPIter, unsigned int index) const;
  bool isCandidateTrack(SbtSpacePoint* outerSpacePoint0,
                        SbtSpacePoint* outerSpacePoint1,
                        SbtSpacePoint* innerSpacePoint0,
                        SbtSpacePoint* innerSpacePoint1);
  bool isInsideTrkRoad(SbtSpacePoint* outerSpacePoint0,
                       SbtSpacePoint* outerSpacePoint1,
                       SbtSpacePoint* innerSpacePoint, unsigned int index) const;
  void SortSpacePoints(std::vector<SbtSpacePoint*>& SPList) const;
  void SortDetectorElems(std::vector<SbtDetectorElem*>& DEList) const;
  void LoopOnSpacePoints(std::vector<std::vector<SbtSpacePoint*>::iterator>& SPIter, unsigned int k, unsigned int index);
  int _linkHits();

  ClassDef(SbtSingleSidePatRecAlg, 1);
};
#endif
