#include <cassert>
#include <iostream>

#include "SbtCluster.h"
#include "SbtEvent.h"
#include "SbtHit.h"
#include "SbtMakeHits.h"

ClassImp(SbtMakeHits);

SbtMakeHits::SbtMakeHits() : _DebugLevel(0) {}

void SbtMakeHits::makeHits(SbtEvent* event) {
  if (_DebugLevel) {
    std::cout << "SbtMakeHits::makeHits" << std::endl;
  }

  for (auto& cluster : event->GetStripClusterList()) {
    event->AddHit(SbtHit(&cluster));
  }
}
