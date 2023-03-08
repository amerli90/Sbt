#include <cassert>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>

#include <TVector3.h>

#include "SbtDetectorElem.h"
#include "SbtEvent.h"
#include "SbtHit.h"
#include "SbtLineSegment.h"
#include "SbtRecursivePatRecAlg.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"

ClassImp(SbtRecursivePatRecAlg);

SbtRecursivePatRecAlg::SbtRecursivePatRecAlg(const YAML::Node& config, std::vector<int> trackDetID)
    : SbtPatRecAlg(config, trackDetID) {
  if (getDebugLevel() > 0) {
    std::cout << "SbtRecursivePatRecAlg: DebugLevel= " << getDebugLevel() << std::endl;
  }
  assert(_trackDetID.size() >= 3);
  _algName = "Recursive";
}

int SbtRecursivePatRecAlg::_linkHits() {
  // reset all the lists
  for (int i = 0; i < maxNTelescopeDetector; i++) {
    _detSpacePointList[i].clear();
  }
  // the RecursivePatRecAlg consider space points on the telescope layers
  // _trackDetID contains the telescope DetectorElemID ordered according to z
  if (getDebugLevel()) {
    std::cout << "SbtRecursivePatRecAlg::linkHits " << std::endl;
  }

  // initialize the TrkCounter
  _trkCounter = 0;
  // tracks with space points on all Telescope Det are selected

  if (_currentEvent->GetSpacePointList().size() < _nTrackDet) {
    if (getDebugLevel()) {
      std::cout << "WARNING: #" << _currentEvent->GetSpacePointList().size() << " Spacepoints found."
           << "Skip the event" << std::endl;
    }
    return 0;
  }

  if (getDebugLevel() > 1) {
    for (unsigned int i = 0; i < _nTrackDet; i++) {
      std::cout << "SbtRecursivePatRec: TrackDetID[" << i << "] = " << _trackDetID[i] << std::endl;
    }
    std::cout << "SbtRecursivePatRecAlg: aSpacePointList.size() = "
         << _currentEvent->GetSpacePointList().size() << std::endl;
  }

  if (!FindTelescopeDet(_currentEvent->GetSpacePointList())) return 0;

  // create the candidate tracks with n SpacePoints
  // n = _nTrackDet, the number of tracking detectors

  // define here the vector of SpacePoint iterators
  std::vector<std::vector<SbtSpacePoint *>::iterator> SPIterator;
  std::vector<SbtSpacePoint *>::iterator tmpIter;

  for (unsigned int i = 0; i < _nTrackDet; i++) {
    SPIterator.push_back(tmpIter);
  }

  // loop on outer telescope (first detector)  SpacePoints
  for (SPIterator.front() = _detSpacePointList[_trackDetID[0]].begin();
       SPIterator.front() != _detSpacePointList[_trackDetID[0]].end();
       SPIterator.front()++) {
    // loop on outer telescope (last detector)  SpacePoints
    for (SPIterator.back() =
             _detSpacePointList[_trackDetID[_nTrackDet - 1]].begin();
         SPIterator.back() !=
         _detSpacePointList[_trackDetID[_nTrackDet - 1]].end();
         SPIterator.back()++) {
      // loop on inner telescope detector SpacePoints
      unsigned int k = 1;

      LoopOnSpacePoints(SPIterator, k);

    }  // close loop on outer detector, last one
  }    // close loop on outer detector, first one

  SPIterator.clear();

  if (getDebugLevel() > 1) {
    std::cout << "SbtRecursivePatRec: _trkCounter = " << _trkCounter << std::endl;
  }
  return _trkCounter;
}

bool SbtRecursivePatRecAlg::isInsideTrkRoad(SbtSpacePoint *sp0,
                                            SbtSpacePoint *sp3,
                                            SbtSpacePoint *spInternal) {
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
    std::cout << "SbtRecursivePatRec: Distance xInternal = "
         << line.distance(xInternal) << std::endl;
    std::cout << "x0   = ";
    x0.Print();
    std::cout << "x3   = ";
    x3.Print();
    std::cout << "xInt = ";
    xInternal.Print();
    std::cout << "passed = " << passed << std::endl;
  }

  return passed;
}

bool SbtRecursivePatRecAlg::isCandidateTrack(std::vector<std::vector<SbtSpacePoint *>::iterator> SPIter) {
  bool passed = false;

  if (getDebugLevel() > 1) {
    std::cout << "SbtRecursivePatRecAlg::isCandidateTrack " << std::endl;
    std::cout << "Track definition based on SpacePoint List" << std::endl;
  }

  std::vector<SbtSpacePoint *> tmpSPList;
  for (unsigned int i = 0; i < _nTrackDet; i++) {
    tmpSPList.push_back(*SPIter.at(i));
    if (getDebugLevel() > 1) {
      std::cout << "Space Point List = " << std::endl;
      TVector3 x = (*SPIter.at(i))->point();
      x.Print();
    }
  }

  // order the space points according to the DetectorElem z position
  SortSpacePoints(tmpSPList);
  if (getDebugLevel() > 1) {
    for (unsigned int i = 0; i < _nTrackDet; i++)
      std::cout << "SbtRecursivePatRec: SPList[" << i << "](x,y,z)=("
           << tmpSPList.at(i)->GetXPosition() << ","
           << tmpSPList.at(i)->GetYPosition() << ","
           << tmpSPList.at(i)->GetZPosition() << ")" << std::endl;
  }

  // define the list of ordered space points, according to z position

  std::vector<TVector3> x;
  for (unsigned int i = 0; i < _nTrackDet; i++) {
    x.push_back(tmpSPList.at(i)->point());
  }

  SbtLineSegment line(x.front(), x.back());
  if (getDebugLevel() > 1) {
    line.print();
    for (unsigned int i = 1; i < (_nTrackDet - 1); i++) {
      std::cout << "  Point - x[" << i << "]: ";
      for (int j = 0; j < 3; j++) std::cout << x.at(i)[j] << ",";
      std::cout << std::endl;
    }
  }

  passed = true;

  for (unsigned int i = 1; i < (_nTrackDet - 1); i++) {
    if (line.distance(x.at(i)) > _roadWidth) passed = false;
  }

  if (getDebugLevel() > 1) {
    for (unsigned int i = 1; i < (_nTrackDet - 1); i++) {
      std::cout << "SbtRecursivePatRec: Distance x[" << i
           << "] = " << line.distance(x.at(i)) << std::endl;
    }
  }

  return passed;
}

void SbtRecursivePatRecAlg::LoopOnSpacePoints(std::vector<std::vector<SbtSpacePoint*>::iterator> &SPIter, unsigned int k) {
  if (getDebugLevel() > 1) {
    std::cout << "SbtRecursivePatRecAlg::LoopOnSpacePoints nested loop n. " << k
         << std::endl;
  }

  // loop on inner telescope detector SpacePoints

  for (SPIter[k] = _detSpacePointList[_trackDetID[k]].begin();
       SPIter[k] != _detSpacePointList[_trackDetID[k]].end(); SPIter[k]++) {
    // check if the Internal SP is within the track nominal road
    // pay attention: SP ordering matters below

    if (!isInsideTrkRoad(*SPIter.front(), *SPIter.back(), *SPIter[k])) continue;

    if ((_nTrackDet - 2) == k) {
      bool isGoodTrack = isCandidateTrack(SPIter);

      if (isGoodTrack) {
        //  start to build the tracks using SpacePoints
        std::vector<SbtSpacePoint*> SPList;
        for (unsigned int i = 0; i < _nTrackDet; i++) {
          SPList.push_back(*SPIter.at(i));
        }
        _currentEvent->AddTrack(SbtTrack(SPList));
        _trkCounter++;
      }
    }

    if (k < (_nTrackDet - 2)) {
      LoopOnSpacePoints(SPIter, k + 1);
    }
  }
}

void SbtRecursivePatRecAlg::SortSpacePoints(std::vector<SbtSpacePoint*> &SPList) {
  // try this
  sort(SPList.begin(), SPList.end(), SbtSpacePoint::ltz_ptr);
}

void SbtRecursivePatRecAlg::SortDetectorElems(std::vector<SbtDetectorElem *> &DEList) {
  // try this
  if (DEList.size() == 0) {
    std::cout << "SbtRecursivePatRec::SortDetectorElems - Detector List Empty"
         << std::endl;
    assert(0);
  }
  sort(DEList.begin(), DEList.end(), SbtDetectorElem::ltz);
}
