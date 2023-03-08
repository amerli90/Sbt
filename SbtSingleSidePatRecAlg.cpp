#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

#include <TVector3.h>

#include "SbtDetectorElem.h"
#include "SbtEvent.h"
#include "SbtHit.h"
#include "SbtLineSegment.h"
#include "SbtSingleSidePatRecAlg.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"

#include "SbtDetectorType.h"

ClassImp(SbtSingleSidePatRecAlg);

SbtSingleSidePatRecAlg::SbtSingleSidePatRecAlg(const YAML::Node& config, std::vector<int> trackDetID)
    : SbtPatRecAlg(config, trackDetID) {
  std::cout << "Costruttore SbtSingleSidePatRecAlg, debugLevel: " << getDebugLevel() << std::endl;
  _algName = "SingleSide";
  std::cout << "trackDetID" << std::endl;

  for (auto ID : trackDetID) {
    std::cout << ID << std::endl;
  }
}

int SbtSingleSidePatRecAlg::_linkHits() {
  // reset all the lists
  _SingleSidePatRecTrackList.clear();
  _SingleSidePatRecTrackList_X.clear();
  _SingleSidePatRecTrackList_Y.clear();

  for (int i = 0; i < maxNTelescopeDetector; i++) {
    _detSpacePointList[i].clear();
  }
  // the SingleSidePatRecAlg consider space points on the telescope layers
  // _trackDetID contains the telescope DetectorElemID ordered according to z
  if (getDebugLevel()) {
    std::cout << "SbtSingleSidePatRecAlg::linkHits " << std::endl;
  }

  // initialize the TrkCounter
  _trkCounter = 0;
  _trkCounter_X = 0;
  _trkCounter_Y = 0;

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
      std::cout << "SbtSingleSidePatRec: TrackDetID[" << i
           << "] = " << _trackDetID[i] << std::endl;
    }
    std::cout << "SbtSingleSidePatRecAlg: aSpacePointList.size()= "
         << _currentEvent->GetSpacePointList().size() << std::endl;
  }

  if (FindTelescopeDet(_currentEvent->GetSpacePointList()) == 0) return 0;

  // create the candidate tracks with n SpacePoints
  // n = _nTrackDet, the number of tracking detectors

  // define here the std::vector of SpacePoint iterators
  std::vector<std::vector<SbtSpacePoint*>::iterator> SPIterator;
  std::vector<std::vector<SbtSpacePoint*>::iterator> SPIterator_X;
  std::vector<std::vector<SbtSpacePoint*>::iterator> SPIterator_Y;
  std::vector<SbtSpacePoint*>::iterator tmpIter;

  // HERE BUILD THE LIST OF TRACKING DETECTOR FOR X AND Y

  const SbtDetectorElem* detElem = nullptr;
  const TGeoRotation* rot = nullptr;
  double phi, theta, psi;
  _trackDetID_X.clear();
  _trackDetID_Y.clear();
  for (unsigned int i = 0; i < _nTrackDet; i++) {
    if (_detSpacePointList[_trackDetID[i]].size() == 0) continue;
    detElem = _detSpacePointList[_trackDetID[i]].at(0)->GetDetectorElem();  // mi basta guardare un punto

    assert(detElem->GetDetectorType()->GetType() == "singleside");  // temporaneamente
    if (detElem->GetDetectorType()->GetType() == "singleside") {  // se è singleside controllo se è U oppure V e aggiungo alla lista corrispondente
      rot = detElem->GetRotation();
      rot->GetAngles(phi, theta, psi);

      if (fabs(theta) < 0.01) {
        if (fabs(fmod(phi + psi + 0, 180)) < 5) {
          _trackDetID_X.push_back(detElem->GetID());
        }
        else if (fabs(fmod(phi + psi + 90, 180)) < 5) {
          _trackDetID_Y.push_back(detElem->GetID());
        }
      }
      else {
        std::cout << "ERROR:\tThe detector is not ortoghonal to the z-axis" << std::endl;
        assert(0);
      }

    } else {
      _trackDetID_X.push_back(detElem->GetID());
      _trackDetID_Y.push_back(detElem->GetID());
    }
  }
  _nTrackDet_X = _trackDetID_X.size();
  _nTrackDet_Y = _trackDetID_Y.size();

  for (unsigned int i = 0; i < _nTrackDet; i++) {
    SPIterator.push_back(tmpIter);
  }
  for (unsigned int i = 0; i < _nTrackDet_X; i++) {
    SPIterator_X.push_back(tmpIter);
  }
  for (unsigned int i = 0; i < _nTrackDet_Y; i++) {
    SPIterator_Y.push_back(tmpIter);
  }

  if (_nTrackDet_X != 0) {
    // loop on outer telescope (first detector)  SpacePoints
    for (SPIterator_X.front() = _detSpacePointList[_trackDetID_X[0]].begin();
         SPIterator_X.front() != _detSpacePointList[_trackDetID_X[0]].end();
         SPIterator_X.front()++) {
      // loop on outer telescope (last detector)  SpacePoints
      for (SPIterator_X.back() =
               _detSpacePointList[_trackDetID_X[_nTrackDet_X - 1]].begin();
           SPIterator_X.back() !=
           _detSpacePointList[_trackDetID_X[_nTrackDet_X - 1]].end();
           SPIterator_X.back()++) {
        // loop on inner telescope detector SpacePoints
        unsigned int k = 1;

        LoopOnSpacePoints(SPIterator_X, k, 0);  // 0 stands for X
      }  // close loop on outer detector, last one
    }    // close loop on outer detector, first one
    SPIterator_X.clear();
  }

  if (_nTrackDet_Y != 0) {
    // loop on outer telescope (first detector)  SpacePoints
    for (SPIterator_Y.front() = _detSpacePointList[_trackDetID_Y[0]].begin();
         SPIterator_Y.front() != _detSpacePointList[_trackDetID_Y[0]].end();
         SPIterator_Y.front()++) {
      // loop on outer telescope (last detector)  SpacePoints
      for (SPIterator_Y.back() =
               _detSpacePointList[_trackDetID_Y[_nTrackDet_Y - 1]].begin();
           SPIterator_Y.back() !=
           _detSpacePointList[_trackDetID_Y[_nTrackDet_Y - 1]].end();
           SPIterator_Y.back()++) {
        // loop on inner telescope detector SpacePoints
        unsigned int k = 1;

        LoopOnSpacePoints(SPIterator_Y, k, 1);  // 1 stands for Y
      }  // close loop on outer detector, last one
    }    // close loop on outer detector, first one
    SPIterator_Y.clear();
  }

  // HERE MERGE THE TRACKs
  std::vector<SbtTrack*>::iterator TrackListIter_X;
  std::vector<SbtTrack*>::iterator TrackListIter_Y;
  std::vector<SbtSpacePoint*> aSpacePointList_X;
  std::vector<SbtSpacePoint*> aSpacePointList_Y;

  std::vector<SbtSpacePoint*> spacePointList_merged;

  if (_SingleSidePatRecTrackList_X.size() == 0) {
    for (TrackListIter_Y = _SingleSidePatRecTrackList_Y.begin();
         TrackListIter_Y != _SingleSidePatRecTrackList_Y.end();
         TrackListIter_Y++) {
      spacePointList_merged = (*TrackListIter_Y)->GetSpacePointList();
      SortSpacePoints(spacePointList_merged);
      _currentEvent->AddTrack(SbtTrack(spacePointList_merged));
      ++_trkCounter;
    }
  } else if (_SingleSidePatRecTrackList_Y.size() == 0) {
    for (TrackListIter_X = _SingleSidePatRecTrackList_X.begin();
         TrackListIter_X != _SingleSidePatRecTrackList_X.end();
         TrackListIter_X++) {
      spacePointList_merged = (*TrackListIter_X)->GetSpacePointList();
      SortSpacePoints(spacePointList_merged);
      _currentEvent->AddTrack(SbtTrack(spacePointList_merged));
      ++_trkCounter;
    }
  } else {
    for (TrackListIter_X = _SingleSidePatRecTrackList_X.begin();
         TrackListIter_X != _SingleSidePatRecTrackList_X.end();
         TrackListIter_X++) {
      for (TrackListIter_Y = _SingleSidePatRecTrackList_Y.begin();
           TrackListIter_Y != _SingleSidePatRecTrackList_Y.end();
           TrackListIter_Y++) {
        spacePointList_merged.clear();
        aSpacePointList_X = (*TrackListIter_X)->GetSpacePointList();
        aSpacePointList_Y = (*TrackListIter_Y)->GetSpacePointList();

        spacePointList_merged = aSpacePointList_Y;
        spacePointList_merged.insert(spacePointList_merged.end(),
                                     aSpacePointList_X.begin(),
                                     aSpacePointList_X.end());

        SortSpacePoints(spacePointList_merged);
        _currentEvent->AddTrack(SbtTrack(spacePointList_merged));
        ++_trkCounter;
      }
    }
  }

  if (getDebugLevel() > 1) {
    std::cout << "SbtSingleSidePatRec: _trkCounter = " << _trkCounter << std::endl;
  }

  return _trkCounter;  // PROVA
}

bool SbtSingleSidePatRecAlg::isInsideTrkRoad(SbtSpacePoint* sp0,
                                             SbtSpacePoint* sp3,
                                             SbtSpacePoint* spInternal,
                                             unsigned int index) const {
  bool passed = false;

  TVector3 x0, x3, xInternal;

  if (index == 0) {
    x0 = TVector3(sp0->point().X(), 0, sp0->point().Z());
    x3 = TVector3(sp3->point().X(), 0, sp3->point().Z());
    xInternal = TVector3(spInternal->point().X(), 0, spInternal->point().Z());
  } else if (index == 1) {
    x0 = TVector3(0, sp0->point().Y(), sp0->point().Z());
    x3 = TVector3(0, sp3->point().Y(), sp3->point().Z());
    xInternal = TVector3(0, spInternal->point().Y(), spInternal->point().Z());
  }

  SbtLineSegment line(x0, x3);
  if (line.distance(xInternal) < _roadWidth) {
    passed = true;
  } else {
    passed = false;
  }
  if (getDebugLevel() > 1) {
    std::cout << "SbtSingleSidePatRec: Distance xInternal = "
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

bool SbtSingleSidePatRecAlg::isCandidateTrack(
    std::vector<std::vector<SbtSpacePoint*>::iterator> SPIter, unsigned int index) const {
  bool passed = false;

  if (getDebugLevel() > 1) {
    std::cout << "SbtSingleSidePatRecAlg::isCandidateTrack " << std::endl;
    std::cout << "Track definition based on SpacePoint List" << std::endl;
  }

  unsigned int tempnTrackDet;
  assert(index == 0 || index == 1);

  if (index == 0) {
    tempnTrackDet = _nTrackDet_X;
  } else if (index == 1) {
    tempnTrackDet = _nTrackDet_Y;
  }

  std::vector<SbtSpacePoint*> tmpSPList;
  for (unsigned int i = 0; i < tempnTrackDet; i++) {
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
    for (unsigned int i = 0; i < tempnTrackDet; i++)
      std::cout << "SbtSingleSidePatRec: SPList[" << i << "](x,y,z)=("
           << tmpSPList.at(i)->GetXPosition() << ","
           << tmpSPList.at(i)->GetYPosition() << ","
           << tmpSPList.at(i)->GetZPosition() << ")" << std::endl;
  }

  // define the list of ordered space points, according to z position

  std::vector<TVector3> x;
  for (unsigned int i = 0; i < tempnTrackDet; i++) {
    x.push_back(tmpSPList.at(i)->point());
  }

  SbtLineSegment line(x.front(), x.back());
  if (getDebugLevel() > 1) {
    line.print();
    for (unsigned int i = 1; i < (tempnTrackDet - 1); i++) {
      std::cout << "  Point - x[" << i << "]: ";
      for (int j = 0; j < 3; j++) std::cout << x.at(i)[j] << ",";
      std::cout << std::endl;
    }
  }

  passed = true;

  for (unsigned int i = 1; i < (tempnTrackDet - 1); i++) {
    if (line.distance(x.at(i)) > _roadWidth) passed = false;
  }

  if (getDebugLevel() > 1) {
    for (unsigned int i = 1; i < (tempnTrackDet - 1); i++) {
      std::cout << "SbtSingleSidePatRec: Distance x[" << i
           << "] = " << line.distance(x.at(i)) << std::endl;
    }
  }

  return passed;
}

bool SbtSingleSidePatRecAlg::isCandidateTrack(SbtSpacePoint* sp1,
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
      std::cout << "SbtSingleSidePatRec: SPList[" << i << "](x,y,z)=("
           << tmpSPList.at(i)->GetXPosition() << ","
           << tmpSPList.at(i)->GetYPosition() << ","
           << tmpSPList.at(i)->GetZPosition() << ")" << std::endl;
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
  } else {
    passed = false;
  }

  if (getDebugLevel() > 1) {
    std::cout << "SbtSingleSidePatRec: Distance x2 = " << line.distance(x2) << std::endl;
    std::cout << "SbtSingleSidePatRec: Distance x3 = " << line.distance(x3) << std::endl;
  }
  return passed;
}

void SbtSingleSidePatRecAlg::LoopOnSpacePoints(std::vector<std::vector<SbtSpacePoint*>::iterator>& SPIter, unsigned int k, unsigned int index) {
  if (getDebugLevel() > 1) {
    std::cout << "SbtSingleSidePatRecAlg::LoopOnSpacePoints nested loop n. " << k
         << std::endl;
  }

  // loop on inner telescope detector SpacePoints

  std::vector<int> temptrackDetID;
  unsigned int tempnTrackDet;

  assert(index == 0 || index == 1);

  if (index == 0) {
    temptrackDetID = _trackDetID_X;
    tempnTrackDet = _nTrackDet_X;
  } else if (index == 1) {
    temptrackDetID = _trackDetID_Y;
    tempnTrackDet = _nTrackDet_Y;
  }

  for (SPIter[k] = _detSpacePointList[temptrackDetID[k]].begin();
       SPIter[k] != _detSpacePointList[temptrackDetID[k]].end(); SPIter[k]++) {
    // check if the Internal SP is within the track nominal road
    // pay attention: SP ordering matters below

    if (!isInsideTrkRoad(*SPIter.front(), *SPIter.back(), *SPIter[k], index))
      continue;

    if ((tempnTrackDet - 2) == k) {
      bool isGoodTrack = isCandidateTrack(SPIter, index);

      if (isGoodTrack) {
        //  start to build the tracks using SpacePoints
        std::vector<SbtSpacePoint*> SPList;
        for (unsigned int i = 0; i < tempnTrackDet; i++) {
          SPList.push_back(*SPIter.at(i));
        }

        SbtTrack* candidateTrack = new SbtTrack(SPList);
        if (index == 0) {
          _SingleSidePatRecTrackList_X.push_back(candidateTrack);
          _trkCounter_X++;
        } else if (index == 1) {
          _SingleSidePatRecTrackList_Y.push_back(candidateTrack);
          _trkCounter_Y++;
        }
      }
    }

    if (k < (tempnTrackDet - 2)) {
      LoopOnSpacePoints(SPIter, k + 1, index);
    }
  }
}

void SbtSingleSidePatRecAlg::SortSpacePoints(std::vector<SbtSpacePoint*>& SPList) const {
  // try this
  sort(SPList.begin(), SPList.end(), SbtSpacePoint::ltz_ptr);
}

void SbtSingleSidePatRecAlg::SortDetectorElems(std::vector<SbtDetectorElem*>& DEList) const {
  // try this
  if (DEList.size() == 0) {
    std::cout << "SbtSingleSidePatRec::SortDetectorElems - Detector List Empty"
         << std::endl;
    assert(0);
  }
  sort(DEList.begin(), DEList.end(), SbtDetectorElem::ltz);
}

