#include <cassert>
#include <iostream>

#include "SbtEvent.h"
#include "SbtFittingAlg.h"
#include "SbtMakeTracks.h"
#include "SbtPatRecAlg.h"
#include "SbtRecursivePatRecAlg.h"
#include "SbtSimple3DFittingAlg.h"
#include "SbtSimpleFittingAlg.h"
#include "SbtSimplePatRecAlg.h"
#include "SbtConstrainedPatRecAlg.h"
#include "SbtSingleSidePatRecAlg.h"
#include "SbtBentCrystalPatRecAlg.h"
#include "SbtBentCrystalFittingAlg.h"
#include "SbtTrack.h"

ClassImp(SbtMakeTracks);

SbtMakeTracks::SbtMakeTracks(const YAML::Node& config, std::vector<int> trackDetID)
    : _DebugLevel(0),
      _trackDetID(trackDetID) {
  std::cout << "SbtMakeTracks:  DebugLevel= " << _DebugLevel << std::endl;

  std::string patRecAlg = config["patternRecognition"].as<std::string>();
  std::string fittingAlg = config["fitting"].as<std::string>();

  // instantiate the correct algorithms
  if (patRecAlg == "Simple") {
    _patRecAlg = new SbtSimplePatRecAlg(config, _trackDetID);
    if (_DebugLevel) {
      std::cout << "SbtMakeTracks c'tor : _patRecAlg = " << _patRecAlg->getAlgName()
           << std::endl;
      std::cout << "SbtMakeTracks c'tor : roadWidth = " << _patRecAlg->getRoadWidth()
           << std::endl;
    }
  }
  else if (patRecAlg == "Recursive") {
    _patRecAlg = new SbtRecursivePatRecAlg(config, _trackDetID);
    if (_DebugLevel) {
      std::cout << "SbtMakeTracks c'tor : _patRecAlg = " << _patRecAlg->getAlgName()
           << std::endl;
      std::cout << "SbtMakeTracks c'tor : roadWidth = " << _patRecAlg->getRoadWidth()
           << std::endl;
    }
  }
  else if (patRecAlg == "SingleSide") {
    _patRecAlg = new SbtSingleSidePatRecAlg(config, _trackDetID);
    if (_DebugLevel) {
      std::cout << "SbtMakeTracks c'tor : _patRecAlg = " << _patRecAlg->getAlgName()
           << std::endl;
      std::cout << "SbtMakeTracks c'tor : roadWidth = " << _patRecAlg->getRoadWidth()
           << std::endl;
    }
  }
  else if (patRecAlg == "Constrained") {
    _patRecAlg = new SbtConstrainedPatRecAlg(config, _trackDetID);
    if (_DebugLevel) {
      std::cout << "SbtMakeTracks c'tor : _patRecAlg = " << _patRecAlg->getAlgName()
           << std::endl;
      std::cout << "SbtMakeTracks c'tor : roadWidth = " << _patRecAlg->getRoadWidth()
           << std::endl;
    }
  }
  else if (patRecAlg == "BentCrystal") {
    _patRecAlg = new SbtBentCrystalPatRecAlg(config, _trackDetID);
    if (_DebugLevel) {
      std::cout << "SbtMakeTracks c'tor : _patRecAlg = " << _patRecAlg->getAlgName()
           << std::endl;
      std::cout << "SbtMakeTracks c'tor : roadWidth = " << _patRecAlg->getRoadWidth()
           << std::endl;
    }
  }
  else {
    std::cout << "Invalid pattern recognition algorithm specified: " << patRecAlg << std::endl;
    assert(false);
  }

  if (fittingAlg == "Simple") {
    _fittingAlg = new SbtSimpleFittingAlg();

    if (_DebugLevel) {
      std::cout << "SbtMakeTracks c'tor : _fittingAlg Simple = " << _fittingAlg->getAlgName() << std::endl;
    }
  } 
  else if (fittingAlg == "Simple3D") {
    _fittingAlg = new SbtSimple3DFittingAlg();

    if (_DebugLevel) {
      std::cout << "SbtMakeTracks c'tor : _fittingAlg Simple3D = " << _fittingAlg->getAlgName() << std::endl;
    }
  }
  else if (fittingAlg == "BentCrystal") {
    _fittingAlg = new SbtBentCrystalFittingAlg(config);

    if (_DebugLevel) {
      std::cout << "SbtMakeTracks c'tor : _fittingAlg BentCrystal = " << _fittingAlg->getAlgName() << std::endl;
    }
  }
  else {
    std::cout << "Invalid fitting algorithm specified: " << fittingAlg << std::endl;
    assert(false);
  }
}

void SbtMakeTracks::makeTracks(SbtEvent* event) {
  // Debugging stuff
  if (_DebugLevel) {
    std::cout << "SbtMakeTracks::makeTracks " << std::endl;
    std::cout << "makeTracks: HitList size = " << event->GetHitList().size() << std::endl;
    if (_patRecAlg != 0) {
      std::cout << "makeTracks:_fittingAlg = " << _fittingAlg->getAlgName() << std::endl;
      std::cout << "makeTracks: _roadWidth = " << _patRecAlg->getRoadWidth() << std::endl;
    }
    else {
      std::cout << "_patRecAlg = null pointer" << std::endl;
      assert(false);
    }
  }

  int ntracks = _patRecAlg->linkHits(event);

  if (_DebugLevel) {
    std::cout << "patRecList size =  " << ntracks << std::endl;
  }
  for (auto& track : event->GetTrackList()) {
    // Add the fitted track to the final Track list of the event
    if (_DebugLevel > 2) {
      std::cout << "SbtMakeTracks:_fittingAlg->getAlgName() = " << _fittingAlg->getAlgName() << std::endl;
    }
    track.SortSpacePoints();
    _fittingAlg->fitTrack(track);
    if (_DebugLevel > 1) track.Print();
  }

  if (_DebugLevel > 2) {
    std::cout << "Done with making tracks. Total tracks: " << event->GetTrackList().size() << std::endl;
  }
}

SbtMakeTracks::~SbtMakeTracks() {
  delete _patRecAlg;
  delete _fittingAlg;
}
