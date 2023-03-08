#include <cassert>
#include <iostream>

#include "SbtCluster.h"
#include "SbtDetectorElem.h"
#include "SbtDetectorType.h"
#include "SbtEnums.h"
#include "SbtHit.h"
#include "SbtLineSegment.h"

#include <TVector3.h>

ClassImp(SbtHit);

SbtHit::SbtHit(SbtCluster* aCluster)
    : _DebugLevel(0),
      _theCluster(aCluster),
      _detectorElem(aCluster->GetDetectorElem()),
      _side(aCluster->GetSide()) {
  // cannot make an SbtHit from a pixel cluster
  SbtEnums::digiType type = aCluster->GetClusterType();
  assert(type != SbtEnums::pixel);
  _IsOnTrack = false;
  // create line segment that represents hit
  // local coords of hit endpoints

  TVector3 x1;
  TVector3 x2;
  SbtDetectorType* detType = _detectorElem->GetDetectorType();

  detType->GetEndPoints(_theCluster->GetSide(), _theCluster->GetPosition(), x1, x2);

  // now convert local to global coordinates
  TVector3 x1global;
  TVector3 x2global;

  _detectorElem->LocalToMaster(x1, x1global);
  _detectorElem->LocalToMaster(x2, x2global);

  _lineSegment = SbtLineSegment(x1global, x2global);

  if (_DebugLevel > 0) {
    if (detType->GetStripAngle() > 0) {
      std::cout << "SbtHit: side, pos: " << _theCluster->GetSide() << ", "
                << _theCluster->GetPosition() << "\n";
      x1.Print();
      x2.Print();
      x1global.Print();
      x2global.Print();
    }
  }
}

void SbtHit::print() const {
  std::cout << "*** SbtHit *** " << std::endl;
  _lineSegment.print();
}

bool SbtHit::Intersection(SbtHit hit, TVector3& point) const {
  // finds the intersection of two SbtHits, with the result being
  // used in the SP c'tor.  Returns true if resulting point is
  // within the active area of the wafer

  // note: there is no check here that SP are in the active area.
  if (_detectorElem->GetID() == hit.GetDetectorElem()->GetID() &&
      _detectorElem->GetID() < maxNTelescopeDetector &&
      _detectorElem->GetID() >= 0 &&
      this->GetCluster()->GetSide() != hit.GetCluster()->GetSide()) {
    TVector3 local;

    if (0 == GetCluster()->GetSide()) {
      local[0] = GetCluster()->GetPosition();
      local[1] = hit.GetCluster()->GetPosition();
    } else {
      local[0] = hit.GetCluster()->GetPosition();
      local[1] = GetCluster()->GetPosition();
    }
    local[2] = 0;

    _detectorElem->LocalToMaster(local, point);

    // check if in active area.
    bool ok = _detectorElem->InActiveArea(local);

    // temporary print...
    if (_DebugLevel > 1 && !ok) {
      TVector3 localPrime;
      _detectorElem->LocalToLocalPrime(local, localPrime);

      std::cout << "Intersection not in active area: (" << localPrime[0] << ","
           << localPrime[1] << ")" << std::endl;
      std::cout << "  DetID: " << _detectorElem->GetID()
           << ", type: " << _detectorElem->GetDetectorType()->GetType() << std::endl;
      std::cout << "  U, V : " << local[0] << ", " << local[1] << std::endl;

      // get endpoints of the two hits in local prime coords

      TVector3 this1 = GetLineSegment().GetPoint1();
      TVector3 this2 = GetLineSegment().GetPoint2();
      TVector3 this1local;
      _detectorElem->MasterToLocalPrime(this1, this1local);
      TVector3 this2local;
      _detectorElem->MasterToLocalPrime(this2, this2local);

      std::cout << " This -> Side: " << GetCluster()->GetSide() << std::endl;
      std::cout << "           p1: " << this1local[0] << ", " << this1local[1] << std::endl;
      std::cout << "           p2: " << this2local[0] << ", " << this2local[1] << std::endl;

      TVector3 hit1 = hit.GetLineSegment().GetPoint1();
      TVector3 hit2 = hit.GetLineSegment().GetPoint2();
      TVector3 hit1local;
      _detectorElem->MasterToLocalPrime(hit1, hit1local);
      TVector3 hit2local;
      _detectorElem->MasterToLocalPrime(hit2, hit2local);

      std::cout << " Hit -> Side: " << hit.GetCluster()->GetSide() << std::endl;
      std::cout << "          p1: " << hit1local[0] << ", " << hit1local[1] << std::endl;
      std::cout << "          p2: " << hit2local[0] << ", " << hit2local[1] << std::endl;
    }
    return ok;
  } 
  else {
    return false;
  }
}

bool SbtHit::isOnSingleSide(TVector3& point) const {
  // Returns true if resulting point is
  // within the active area of the wafer

  // note: there is no check here that SP are in the active area.
  if (_detectorElem->GetID() < maxNTelescopeDetector &&
      _detectorElem->GetID() >= 0) {
    TVector3 local;

    local[0] = GetCluster()->GetPosition();
    local[1] = 0;  // local coord V in not measured by singleside strip
                   // detectors
    local[2] = 0;

    // check if in active area.
    bool ok = _detectorElem->InActiveArea(local);

    _detectorElem->LocalToMaster(local, point);

    // temporary print...
    if (_DebugLevel > 1 && !ok) {
      TVector3 localPrime;
      _detectorElem->LocalToLocalPrime(local, localPrime);

      std::cout << "Intersection not in active area: (" << localPrime[0] << ","
           << localPrime[1] << ")" << std::endl;
      std::cout << "  DetID: " << _detectorElem->GetID()
           << ", type: " << _detectorElem->GetDetectorType()->GetType() << std::endl;
      std::cout << "  U, V : " << local[0] << ", " << local[1] << std::endl;

      // get endpoints of the two hits in local prime coords

      TVector3 this1 = GetLineSegment().GetPoint1();
      TVector3 this2 = GetLineSegment().GetPoint2();
      TVector3 this1local;
      _detectorElem->MasterToLocalPrime(this1, this1local);
      TVector3 this2local;
      _detectorElem->MasterToLocalPrime(this2, this2local);

      std::cout << " This -> Side: " << GetCluster()->GetSide() << std::endl;
      std::cout << "           p1: " << this1local[0] << ", " << this1local[1] << std::endl;
      std::cout << "           p2: " << this2local[0] << ", " << this2local[1] << std::endl;
    }
    return ok;
  }
  else {
    return false;
  }
}
