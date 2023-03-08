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
#include "SbtBentCrystalPatRecAlg.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"

ClassImp(SbtBentCrystalPatRecAlg);

SbtBentCrystalPatRecAlg::SbtBentCrystalPatRecAlg(const YAML::Node& config, std::vector<int> trackDetID) : 
  SbtPatRecAlg(config, trackDetID),
  _deflectionAngleThreshold(1e-4) {
  if (getDebugLevel() > 0) {
    std::cout << "SbtBentCrystalPatRecAlg: DebugLevel= " << getDebugLevel() << std::endl;
  }
  assert(_trackDetID.size() >= 4);
  _algName = "BentCrystal";
  if (config["deflectionAngleThreshold"]) _deflectionAngleThreshold = config["deflectionAngleThreshold"].as<double>();
}

int SbtBentCrystalPatRecAlg::_findLongTracks() {
  int ntracks = 0;
  // create the candidate tracks with n SpacePoints
  // n = _nTrackDet, the number of tracking detectors

  // define here the vector of SpacePoint iterators
  std::vector<std::vector<SbtSpacePoint*>::const_iterator> SPIterator;
  for (unsigned int i = 0; i < _nTrackDet; i++) {
    SPIterator.push_back(std::vector<SbtSpacePoint *>::const_iterator());
  }

  // loop on outer telescope (first detector)  SpacePoints
  for (SPIterator.front() = _detSpacePointList[_trackDetID[0]].begin();
       SPIterator.front() != _detSpacePointList[_trackDetID[0]].end();
       SPIterator.front()++) {
    // loop on outer telescope (last detector)  SpacePoints
    for (SPIterator.back() = _detSpacePointList[_trackDetID[_nTrackDet - 1]].begin();
         SPIterator.back() != _detSpacePointList[_trackDetID[_nTrackDet - 1]].end();
         SPIterator.back()++) {
      // loop on inner telescope detector SpacePoints
      ntracks = LoopOnSpacePoints(SPIterator, 1);
    }  // close loop on outer detector, last one
  }    // close loop on outer detector, first one

  if (getDebugLevel() > 1) {
    std::cout << "SbtBentCrystalPatRecAlg: ntracks = " << ntracks << std::endl;
  }
  return ntracks;
}

int SbtBentCrystalPatRecAlg::_findChanneledTracks() {
  int ntracks = 0;
  auto dsTracks = _findDownstreamCandidates();
  auto usTracks = _findUpstreamCandidates();
  for (auto& us: usTracks) {
    SbtLineSegment line1(us[0]->point(), us[1]->point());
    for (auto& ds : dsTracks) {
      SbtLineSegment line2(ds[0]->point(), ds[1]->point());
      if (line1.distance(line2) < _roadWidth) {
        double angle_yzplane = TMath::Abs(line1.angle_yzplane(line2));
        double angle_xzplane = TMath::Abs(line1.angle_xzplane(line2));
        auto trackShape = SbtEnums::trackShape::longTrack;
        if (angle_yzplane > _deflectionAngleThreshold && angle_xzplane > _deflectionAngleThreshold) {
          trackShape = SbtEnums::trackShape::channelledTrack;
        }
        else if (angle_yzplane > _deflectionAngleThreshold) {
          trackShape = SbtEnums::trackShape::channelledTrackY;
        }
        else if (angle_xzplane > _deflectionAngleThreshold) {
          trackShape = SbtEnums::trackShape::channelledTrackX;
        }
        _currentEvent->AddTrack(SbtTrack(us, ds, SbtEnums::objectType::reconstructed, trackShape));
        ++ntracks;
      }
    }
  }
  return ntracks;
}

int SbtBentCrystalPatRecAlg::_findDownStreamTracks() {
  int ntracks = 0;
  auto dsTracks = _findDownstreamCandidates();
  for (auto& ds : dsTracks) {
    _currentEvent->AddTrack(SbtTrack(ds[0], ds[1], SbtEnums::objectType::reconstructed, SbtEnums::trackShape::downStreamTrack));
    ++ntracks;
  }
  return ntracks;
}

int SbtBentCrystalPatRecAlg::_findUpStreamTracks() {
  int ntracks = 0;
  auto usTracks = _findUpstreamCandidates();
  for (auto& us : usTracks) {
    _currentEvent->AddTrack(SbtTrack(us[0], us[1], SbtEnums::objectType::reconstructed, SbtEnums::trackShape::upStreamTrack));
    ++ntracks;
  }
  return ntracks;
}

int SbtBentCrystalPatRecAlg::_linkHits() {
  // reset all the lists
  for (int i = 0; i < maxNTelescopeDetector; i++) {
    _detSpacePointList[i].clear();
  }
  // the BentCrystalPatRecAlg consider space points on the telescope layers
  // _trackDetID contains the telescope DetectorElemID ordered according to z
  if (getDebugLevel()) {
    std::cout << "SbtBentCrystalPatRecAlg::linkHits " << std::endl;
  }

  // initialize the TrkCounter
  _trkCounter = 0;
  // tracks with space points on all Telescope Det are selected

  if (_currentEvent->GetSpacePointList().size() < 2) {
    if (getDebugLevel()) {
      std::cout << "WARNING: #" << _currentEvent->GetSpacePointList().size() << " Spacepoints found."
           << "Skip the event" << std::endl;
    }
    return 0;
  }

  if (getDebugLevel() > 1) {
    for (unsigned int i = 0; i < _nTrackDet; i++) {
      std::cout << "SbtBentCrystalPatRecAlg: TrackDetID[" << i << "] = " << _trackDetID[i] << std::endl;
    }
    std::cout << "SbtBentCrystalPatRecAlg: aSpacePointList.size() = " << _currentEvent->GetSpacePointList().size() << std::endl;
  }

  if (!FindTelescopeDet(_currentEvent->GetSpacePointList())) return 0;

  if (_currentEvent->GetSpacePointList().size() >= 4) {
    // Long and channelled track types require at least 4 space points
    _trkCounter += _findLongTracks();
    _trkCounter += _findChanneledTracks();
  }
  _trkCounter += _findDownStreamTracks();
  _trkCounter += _findUpStreamTracks();

  return _trkCounter;
}

bool SbtBentCrystalPatRecAlg::isInsideTrkRoad(SbtSpacePoint *sp0, SbtSpacePoint *sp3, SbtSpacePoint *spInternal) const {
  TVector3 x0 = sp0->point();
  TVector3 x3 = sp3->point();
  TVector3 xInternal = spInternal->point();

  bool passed = false;

  SbtLineSegment line(x0, x3);
  passed = line.distance(xInternal) < _roadWidth;
  if (getDebugLevel() > 1) {
    std::cout << "SbtBentCrystalPatRecAlg: Distance xInternal = "<< line.distance(xInternal) << std::endl;
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

bool SbtBentCrystalPatRecAlg::isCandidateTrack(std::vector<std::vector<SbtSpacePoint *>::const_iterator> SPIter) const {
  bool passed = false;

  if (getDebugLevel() > 1) {
    std::cout << "SbtBentCrystalPatRecAlg::isCandidateTrack " << std::endl;
    std::cout << "Track definition based on SpacePoint List" << std::endl;
  }

  std::vector<SbtSpacePoint*> tmpSPList;
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
    if (line.distance(x.at(i)) > _roadWidth) {
      passed = false;
      break;
    }
  }

  if (getDebugLevel() > 1) {
    for (unsigned int i = 1; i < (_nTrackDet - 1); i++) {
      std::cout << "SbtRecursivePatRec: Distance x[" << i
          << "] = " << line.distance(x.at(i)) << std::endl;
    }
  }

  return passed;
}

int SbtBentCrystalPatRecAlg::LoopOnSpacePoints(std::vector<std::vector<SbtSpacePoint*>::const_iterator> &SPIter, unsigned int k) const {
  int ntracks = 0;
  if (getDebugLevel() > 1) {
    std::cout << "SbtBentCrystalPatRecAlg::LoopOnSpacePoints nested loop n. " << k << std::endl;
  }

  // loop on inner telescope detector SpacePoints

  for (SPIter[k] = _detSpacePointList[_trackDetID[k]].begin();
       SPIter[k] != _detSpacePointList[_trackDetID[k]].end();
       SPIter[k]++) {
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
        _currentEvent->AddTrack(SbtTrack(SPList, SbtEnums::objectType::reconstructed, SbtEnums::trackShape::longTrack));
        ntracks++;
      }
    }

    if (k < (_nTrackDet - 2)) {
      ntracks += LoopOnSpacePoints(SPIter, k + 1);
    }
  }
  return ntracks;
}

void SbtBentCrystalPatRecAlg::SortSpacePoints(std::vector<SbtSpacePoint*> &SPList) const {
  // try this
  sort(SPList.begin(), SPList.end(), SbtSpacePoint::ltz_ptr);
}

void SbtBentCrystalPatRecAlg::SortDetectorElems(std::vector<SbtDetectorElem *> &DEList) {
  // try this
  if (DEList.size() == 0) {
    std::cout << "SbtRecursivePatRec::SortDetectorElems - Detector List Empty"
         << std::endl;
    assert(0);
  }
  sort(DEList.begin(), DEList.end(), SbtDetectorElem::ltz);
}

std::vector<std::array<SbtSpacePoint*,2>> SbtBentCrystalPatRecAlg::_findDownstreamCandidates() {
  std::vector<std::array<SbtSpacePoint*,2>> result;
  for (auto sp1 : _detSpacePointList[2]) {
    if (sp1->IsOnTrack()) continue;
    for (auto sp2 : _detSpacePointList[3]) {
      if (sp2->IsOnTrack()) continue;
      result.push_back({sp1, sp2});
    }
  }
  return result;
}

std::vector<std::array<SbtSpacePoint*,2>> SbtBentCrystalPatRecAlg::_findUpstreamCandidates() {
  std::vector<std::array<SbtSpacePoint*,2>> result;
  for (auto sp1 : _detSpacePointList[0]) {
    if (sp1->IsOnTrack()) continue;
    for (auto sp2 : _detSpacePointList[1]) {
      if (sp2->IsOnTrack()) continue;
      result.push_back({sp1, sp2});
    }
  }
  return result;
}
