#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

#include "SbtConstrainedPatRecAlg.h"
#include "SbtDetectorElem.h"
#include "SbtEvent.h"
#include "SbtHit.h"
#include "SbtLineSegment.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"

#include <TVector3.h>

ClassImp(SbtConstrainedPatRecAlg);

SbtConstrainedPatRecAlg::SbtConstrainedPatRecAlg(const YAML::Node& config,std::vector<int> trackDetID)
    : SbtPatRecAlg(config, trackDetID) {
  if (getDebugLevel() > 0) {
    std::cout << "SbtConstrainedPatRecAlg:  DebugLevel= " << getDebugLevel() << std::endl;
  }
  if (config["origin"]) {
    _origin.SetXYZ(
      config["origin"][0].as<double>(),
      config["origin"][1].as<double>(),
      config["origin"][2].as<double>());
  }
  assert(_trackDetID.size() >= 2);
  _algName = "Constrained";
}

int SbtConstrainedPatRecAlg::_linkHits() {
  // reset all the lists
  for (int i = 0; i < maxNTelescopeDetector; i++) {
    _detSpacePointList[i].clear();
  }
  // the ConstrainedPatRecAlg considers space points on the 2 telescope layers
  // _trackDetID contains the telescope DetectorElemID ordered according to z
  if (getDebugLevel()) {
    std::cout << "SbtConstrainedPatRecAlg::linkHits " << std::endl;
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
      std::cout << "SbtConstrainedPatRec: TrakDetID[" << i << "] = " << _trackDetID[i] << std::endl;
    }
    std::cout << "SbtConstrainedPatRecAlg: aSpacePointList.size()= " << _currentEvent->GetSpacePointList().size() << std::endl;
  }

  if (!FindTelescopeDet(_currentEvent->GetSpacePointList())) return 0;

  // create the candidate tracks with 2 SpacePoints
  for (auto SP1 : _detSpacePointList[1]) {
    for (auto SP0 : _detSpacePointList[0]) {
      if (getDebugLevel() > 2) {
        std::cout << "Comparing point" << std::endl;
        SP0->point().Print();
        std::cout << "with point" << std::endl;
        SP1->point().Print();
      }
      // check if the Internal SP is within the track nominal road
      // pay attention: SP ordering matters below
      if (!isInsideTrkRoad(_origin, SP1->point(), SP0->point())) {
        if (getDebugLevel() > 2) std::cout << "Not a track" << std::endl;
        continue;
      }
      else {
        if (getDebugLevel() > 2) std::cout << "Good track" << std::endl;
      }

      // start to build the tracks using SpacePoints
      _currentEvent->AddTrack(SbtTrack(std::vector<SbtSpacePoint*>({SP0, SP1})));
      TrkCounter++;
    }
  }
  if (getDebugLevel() > 1) {
    std::cout << "SbtConstrainedPatRec: TrkCounter = " << TrkCounter << std::endl;
  }
  return TrkCounter;
}

bool SbtConstrainedPatRecAlg::isInsideTrkRoad(const TVector3& x0,
                                              const TVector3& x3,
                                              const TVector3& xInternal) {
  bool passed = false;

  SbtLineSegment line(x0, x3);
  if (line.distance(xInternal) < _roadWidth) {
    passed = true;
  } else {
    passed = false;
  }
  if (getDebugLevel() > 1) {
    std::cout << "SbtConstrainedPatRec: Distance xInternal = "
         << line.distance(xInternal) << std::endl;
  }

  return passed;
}
