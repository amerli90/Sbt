#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include "SbtDetectorElem.h"
#include "SbtEvent.h"
#include "SbtHit.h"
#include "SbtLineSegment.h"
#include "SbtSimplePatRecAlg.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"

#include <TVector3.h>

ClassImp(SbtSimplePatRecAlg);

SbtSimplePatRecAlg::SbtSimplePatRecAlg(const YAML::Node& config, std::vector<int> trackDetID)
    : SbtPatRecAlg(config, trackDetID) {
  if (getDebugLevel() > 0) {
    std::cout << "SbtSimplePatRecAlg:  DebugLevel= " << getDebugLevel() << std::endl;
  }
  assert(_trackDetID.size() >= 4);
  _algName = "Simple";
}

int SbtSimplePatRecAlg::_linkHits() {
  // reset all the lists
  for (int i = 0; i < maxNTelescopeDetector; i++) {
    _detSpacePointList[i].clear();
  }
  // the SimplePatRecAlg consider space points on the 4 telescope layers
  // _trackDetID contains the telescope DetectorElemID ordered according to z
  if (getDebugLevel()) {
    std::cout << "SbtSimplePatRecAlg::linkHits " << std::endl;
  }

  // initialize the TrkCounter
  int TrkCounter = 0;
  // tracks with space points on all Telescope Det are selected
  if (_currentEvent->GetSpacePointList().size() < _trackDetID.size()) {
    if (getDebugLevel()) {
      std::cout << "WARNING: skipping ev with " << _currentEvent->GetSpacePointList().size()
           << " spacepoints" << std::endl;
    }
    return 0;
  }

  if (getDebugLevel() > 1) {
    for (unsigned int i = 0; i < _trackDetID.size(); i++) {
      std::cout << "SbtSimplePatRec: TrakDetID[" << i << "] = " << _trackDetID[i]
           << std::endl;
    }
    std::cout << "SbtSimplePatRecAlg: aSpacePointList.size()= "
         << _currentEvent->GetSpacePointList().size() << std::endl;
  }

  if (!FindTelescopeDet(_currentEvent->GetSpacePointList())) return 0;

  // create the candidate tracks with 4 SpacePoints
  std::vector<SbtSpacePoint*>::iterator SPIterator0;
  std::vector<SbtSpacePoint*>::iterator SPIterator3;
  std::vector<SbtSpacePoint*>::iterator SPIterator1;
  std::vector<SbtSpacePoint*>::iterator SPIterator2;
  // loop on outer telescope detector0  SpacePoints
  for (SPIterator0 = _detSpacePointList[_trackDetID[0]].begin();
       SPIterator0 != _detSpacePointList[_trackDetID[0]].end(); SPIterator0++) {
    // loop on outer telescope detector3  SpacePoints
    for (SPIterator3 = _detSpacePointList[_trackDetID[3]].begin();
         SPIterator3 != _detSpacePointList[_trackDetID[3]].end();
         SPIterator3++) {
      // loop on inner telescope detector1  SpacePoints
      for (SPIterator1 = _detSpacePointList[_trackDetID[1]].begin();
           SPIterator1 != _detSpacePointList[_trackDetID[1]].end();
           SPIterator1++) {
        // check if the Internal SP is within the track nominal road
        // pay attention: SP ordering matters below
        if (!isInsideTrkRoad(*SPIterator0, *SPIterator3, *SPIterator1))
          continue;
        // loop on inner telescope detector2  SpacePoints
        for (SPIterator2 = _detSpacePointList[_trackDetID[2]].begin();
             SPIterator2 != _detSpacePointList[_trackDetID[2]].end();
             SPIterator2++) {
          // check if the Internal SP is within the track nominal road
          // pay attention: SP ordering matters below
          if (!isInsideTrkRoad(*SPIterator0, *SPIterator3, *SPIterator2))
            continue;

          // Finall we will remove the checks below,
          // for the moment we keep it for debugging purposes: it is redundant

          // check if the candidate track is a good one
          // require distance of the SpacePoints from the trajectory
          // to be within the cuts
          bool isGoodTrack = isCandidateTrack(*SPIterator0, *SPIterator1,
                                              *SPIterator2, *SPIterator3);
          if (isGoodTrack) {
            //	   start to build the tracks using SpacePoints
            _currentEvent->AddTrack(SbtTrack(*SPIterator0, *SPIterator1, *SPIterator2, *SPIterator3));
            TrkCounter++;
          }
        }
      }
    }
  }
  if (getDebugLevel() > 1) {
    std::cout << "SbtSimplePatRec: TrkCounter = " << TrkCounter << std::endl;
  }
  return TrkCounter;
}

bool SbtSimplePatRecAlg::isInsideTrkRoad(SbtSpacePoint* sp0, SbtSpacePoint* sp3,
                                         SbtSpacePoint* spInternal) {
  bool passed = false;

  TVector3 x0 = sp0->point();
  TVector3 x3 = sp3->point();
  TVector3 xInternal = spInternal->point();

  SbtLineSegment line(x0, x3);
  if (line.distance(xInternal) < _roadWidth) {
    passed = true;
  } else {
    passed = false;
  }
  if (getDebugLevel() > 1) {
    std::cout << "SbtSimplePatRec: Distance xInternal = " << line.distance(xInternal)
         << std::endl;
  }

  return passed;
}

bool SbtSimplePatRecAlg::isCandidateTrack(SbtSpacePoint* sp1,
                                          SbtSpacePoint* sp2,
                                          SbtSpacePoint* sp3,
                                          SbtSpacePoint* sp4) {
  bool passed = false;

  std::vector<SbtSpacePoint*> tmpSPList;
  // assume 4 space points for tracking
  tmpSPList.push_back(sp1);
  tmpSPList.push_back(sp2);
  tmpSPList.push_back(sp3);
  tmpSPList.push_back(sp4);
  // order the space points according to the DetectorElem z position
  SortSpacePoints(tmpSPList);
  if (getDebugLevel() > 1) {
    for (int i = 0; i < 4; i++)
      std::cout << "SbtSimplePatRec: SPList[" << i << "](x,y,z)=("
           << tmpSPList[i]->GetXPosition() << ","
           << tmpSPList[i]->GetYPosition() << ","
           << tmpSPList[i]->GetZPosition() << ")" << std::endl;
  }

  // x1 has lowest z value, x4 has highest x value (outer detectors)
  TVector3 x1 = tmpSPList.at(0)->point();
  TVector3 x2 = tmpSPList.at(1)->point();
  TVector3 x3 = tmpSPList.at(2)->point();
  TVector3 x4 = tmpSPList.at(3)->point();

  SbtLineSegment line(x1, x4);
  if (getDebugLevel() > 1) {
    line.print();
    std::cout << "  Point - x2: ";
    for (int i = 0; i < 3; i++) std::cout << x2[i] << ",";
    std::cout << std::endl;
    std::cout << "  Point - x3: ";
    for (int i = 0; i < 3; i++) std::cout << x3[i] << ",";
    std::cout << std::endl;
  }

  if (line.distance(x2) < _roadWidth && line.distance(x3) < _roadWidth) {
    passed = true;
  }
  else {
    passed = false;
  }
  if (getDebugLevel() > 1) {
    std::cout << "SbtSimplePatRec: Distance x2 = " << line.distance(x2) << std::endl;
    std::cout << "SbtSimplePatRec: Distance x3 = " << line.distance(x3) << std::endl;
  }
  return passed;
}

void SbtSimplePatRecAlg::SortSpacePoints(std::vector<SbtSpacePoint*>& SPList) {
  // try this
  std::sort(SPList.begin(), SPList.end(), SbtSpacePoint::ltz_ptr);
}
