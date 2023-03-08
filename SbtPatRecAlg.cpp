#include <iostream>

#include "SbtPatRecAlg.h"
#include "SbtDetectorElem.h"

ClassImp(SbtPatRecAlg)

SbtPatRecAlg::SbtPatRecAlg(const YAML::Node& config, std::vector<int> trackDetID) : 
  _DebugLevel(0),
  _roadWidth(0),
  _currentEvent(nullptr),
  _trackDetID(trackDetID),
  _nTrackDet(trackDetID.size()) {
  std::cout << "SbtPatRecAlg:  DebugLevel= " << _DebugLevel << std::endl;
  if (config["roadWidth"]) {
    _roadWidth = config["roadWidth"].as<double>();
  }
}

SbtPatRecAlg::SbtPatRecAlg() : _DebugLevel(0), _roadWidth(0),_currentEvent(nullptr) {
  std::cout << "SbtPatRecAlg:  DebugLevel= " << _DebugLevel << std::endl;
}

void SbtPatRecAlg::overrideRoadWidth(double roadWidth) {
  std::cout << "Overriding pat recognition algorithm road width. Old value = " << _roadWidth << ". New value = " << roadWidth << std::endl;
  _roadWidth = roadWidth;
}

int SbtPatRecAlg::linkHits(SbtEvent* event) {
  _currentEvent = event;
  return _linkHits();
}

int SbtPatRecAlg::FindTelescopeDet(std::vector<SbtSpacePoint>& SpList) {
  int AllTelescopeDetFound = 1;
  // find the detector Elem ID for telescope detectors
  for (auto& sp : SpList) {
    if (sp.GetSpacePointType() != SbtEnums::objectType::reconstructed) continue;
    _detSpacePointList[sp.GetDetectorElem()->GetID()].push_back(&sp);
  }

  for (unsigned int i = 0; i < _trackDetID.size(); i++) {
    if (_detSpacePointList[_trackDetID[i]].size() == 0)
      AllTelescopeDetFound = 0;
  }

  if (getDebugLevel() > 1) {
    for (unsigned int i = 0; i < _trackDetID.size(); i++) {
      std::cout << "SbtSimplePatRecAlg::FindTeleScopeDet:"
           << " Size _detSpacePointList[" << _trackDetID[i]
           << "] = " << _detSpacePointList[_trackDetID[i]].size() << std::endl;
      if (_detSpacePointList[_trackDetID[i]].size())
        std::cout << "DetectorElem _detSpacePointList[" << _trackDetID[i] << "] = "
             << _detSpacePointList[_trackDetID[i]].at(0)->GetDetectorElem()->GetID()
             << std::endl;
    }
  }

  return AllTelescopeDetFound;
}