#include <cmath>

#include "SbtSpacePoint.h"

#include "SbtCluster.h"
#include "SbtDef.h"
#include "SbtDetectorElem.h"
#include "SbtDetectorType.h"
#include "SbtHit.h"

ClassImp(SbtSpacePoint);

SbtSpacePoint::SbtSpacePoint() :
  _DebugLevel(0),
  _detectorElem(nullptr),
  _point(),
  _pointErr(),
  _errorMethod(),
  _trackDetErr(0.),
  _digiType(SbtEnums::digiType::undefinedDigiType),
  _spacePointType(SbtEnums::objectType::reconstructed),
  _pxlCluster(nullptr),
  _IsOnTrack(false) {
  }

SbtSpacePoint::SbtSpacePoint(TVector3 point, const SbtDetectorElem* detElem,
                             SbtHit* HitA, SbtHit* HitB, std::string ErrorMethod,
                             double trackDetErr) :
  _DebugLevel(0),
  _detectorElem(detElem),
  _point(point),
  _pointErr(DefaultPointErr),
  _errorMethod(ErrorMethod),
  _trackDetErr(trackDetErr),
  _digiType(SbtEnums::digiType::strip),
  _spacePointType(SbtEnums::objectType::reconstructed) {
  if (_DebugLevel > 0)
    std::cout << "SbtSpacePoint:  DebugLevel= " << _DebugLevel << std::endl;
  // assing the cluster pointer for U and V side
  if (HitA->GetSide() == SbtEnums::U && HitB->GetSide() == SbtEnums::V) {
    _hitU = HitA;
    _hitV = HitB;
  }
  else if (HitA->GetSide() == SbtEnums::V && HitB->GetSide() == SbtEnums::U) {
    _hitU = HitB;
    _hitV = HitA;
  }
  else {
    std::cout << "SbtSpacePoint::c'tor : Wrong Cluster Side " << std::endl;
    assert(0);
  }
  _pxlCluster = nullptr;
  _IsOnTrack = false;

  InitError();

  if (_DebugLevel > 1) {
    std::cout << "SbtSpacePoint: (x,y,z) = (" << GetXPosition() << ","
         << GetYPosition() << "," << GetZPosition() << ")" << std::endl;
    std::cout << "SbtSpacePoint: (xErr,yErr,zErr) = (" << GetXPositionErr() << ","
         << GetYPositionErr() << "," << GetZPositionErr() << ")" << std::endl;
  }
}

SbtSpacePoint::SbtSpacePoint(TVector3 point, const SbtDetectorElem* detElem,
                             SbtHit* HitA, std::string ErrorMethod,
                             double trackDetErr) :
  _DebugLevel(0),
  _detectorElem(detElem),
  _point(point),
  _pointErr(DefaultPointErr),
  _errorMethod(ErrorMethod),
  _trackDetErr(trackDetErr),
  _digiType(SbtEnums::digiType::strip),
  _spacePointType(SbtEnums::objectType::reconstructed) {
  if (_DebugLevel > 0) {
    std::cout << "SbtSpacePoint:  DebugLevel= " << _DebugLevel << std::endl;
  }
  // assing the cluster pointer for U and V side
  _hitU = HitA;
  _hitV = nullptr;

  _pxlCluster = nullptr;
  _IsOnTrack = false;

  InitError();

  if (_DebugLevel > 1) {
    std::cout << "SbtSpacePoint: (x,y,z) = (" << GetXPosition() << ","
         << GetYPosition() << "," << GetZPosition() << ")" << std::endl;
    std::cout << "SbtSpacePoint: (xErr,yErr,zErr) = (" << GetXPositionErr() << ","
         << GetYPositionErr() << "," << GetZPositionErr() << ")" << std::endl;
  }
}

SbtSpacePoint::SbtSpacePoint(TVector3 point, TVector3 pointErr,
                             const SbtDetectorElem* detElem, SbtHit* HitA,
                             SbtHit* HitB) :
  _DebugLevel(0),
  _detectorElem(detElem),
  _point(point),
  _pointErr(pointErr),
  _digiType(SbtEnums::digiType::strip),
  _spacePointType(SbtEnums::objectType::reconstructed) {
  if (_DebugLevel) std::cout << "SbtSpacePoint:  DebugLevel= " << _DebugLevel << std::endl;
  // assing the cluster pointer for U and V side
  if (HitA->GetSide() == SbtEnums::U && HitB->GetSide() == SbtEnums::V) {
    _hitU = HitA;
    _hitV = HitB;
  }
  else if (HitA->GetSide() == SbtEnums::V && HitB->GetSide() == SbtEnums::U) {
    _hitU = HitB;
    _hitV = HitA;
  }
  else {
    std::cout << "SbtSpacePoint::c'tor : Wrong Cluster Side " << std::endl;
    assert(0);
  }
  _pxlCluster = nullptr;
  _IsOnTrack = false;

  // spacePointError is already defined
}

// to be used only for simulation generation
SbtSpacePoint::SbtSpacePoint(TVector3 point, const SbtDetectorElem* detElem) :
  _DebugLevel(0),
  _detectorElem(detElem),
  _point(point),
  _digiType(SbtEnums::digiType::undefinedDigiType),
  _spacePointType(SbtEnums::objectType::reconstructed) {
  if (_DebugLevel > 0) {
    std::cout << "SbtSpacePoint:  DebugLevel= " << _DebugLevel << std::endl;
  }

  _pxlCluster = nullptr;
  _hitU = nullptr;
  _hitV = nullptr;
  _IsOnTrack = false;
  _pointErr.SetXYZ(DefaultPointErr[0], DefaultPointErr[1], DefaultPointErr[2]);

  if (_DebugLevel > 1) {
    std::cout << "SbtSpacePoint: (x,y,z) = (" << GetXPosition() << ","
         << GetYPosition() << "," << GetZPosition() << ")" << std::endl;
    std::cout << "SbtSpacePoint: (xErr,yErr,zErr) = (" << GetXPositionErr() << ","
         << GetYPositionErr() << "," << GetZPositionErr() << ")" << std::endl;
  }
}

SbtSpacePoint::SbtSpacePoint(SbtCluster* pixelCluster, std::string ErrorMethod, double trackDetErr) :
  _DebugLevel(0),
  _detectorElem(pixelCluster->GetDetectorElem()),
  _pointErr(DefaultPointErr),
  _errorMethod(ErrorMethod),
  _trackDetErr(trackDetErr),
  _digiType(SbtEnums::digiType::pixel),
  _spacePointType(SbtEnums::objectType::reconstructed) {
  _pxlCluster = pixelCluster;
  _hitU = nullptr;
  _hitV = nullptr;
  _IsOnTrack = false;

  // define point in master coordinates
  TVector3 masterPoint;
  // define point in local coordinates
  TVector3 localPoint(pixelCluster->GetPxlUPosition(),
                      pixelCluster->GetPxlVPosition(), 0.0);

  // get the point in master coordinates
  _detectorElem->LocalToMaster(localPoint, masterPoint);
  _point = masterPoint;

  if (_DebugLevel > 0) {
    std::cout << "SbtSpacePoint:  DebugLevel= " << _DebugLevel << std::endl;
  }

  InitError();

  if (_DebugLevel > 1) {
    std::cout << "SbtSpacePoint Pixel: (x,y,z) = (" << GetXPosition() << ","
         << GetYPosition() << "," << GetZPosition() << ")" << std::endl;
    std::cout << "SbtSpacePoint Pixel: (xErr,yErr,zErr) = (" << GetXPositionErr()
         << "," << GetYPositionErr() << "," << GetZPositionErr() << ")" << std::endl;
  }
}

SbtSpacePoint::SbtSpacePoint(const SbtSpacePoint& other) :
  _DebugLevel(0) {
  *this = other;
}

SbtSpacePoint& SbtSpacePoint::operator=(const SbtSpacePoint& other) {
  _point = other._point;
  _pointErr = other._pointErr;
  _detectorElem = other._detectorElem;
  _IsOnTrack = other._IsOnTrack;
  _DebugLevel = other._DebugLevel;
  _hitU = other._hitU;
  _hitV = other._hitV;
  _pxlCluster = other._pxlCluster;
  _digiType = other._digiType;
  _errorMethod = other._errorMethod;
  _trackDetErr = other._trackDetErr;
  _spacePointType = other._spacePointType;
  return *this;
}

bool SbtSpacePoint::ltz_ptr(const SbtSpacePoint* aSpacePoint1, const SbtSpacePoint* aSpacePoint2) {
  return ltz(*aSpacePoint1, *aSpacePoint2);
}

bool SbtSpacePoint::ltz(const SbtSpacePoint& aSpacePoint1, const SbtSpacePoint& aSpacePoint2) {
    return (aSpacePoint1.GetDetectorElem()->GetZPos() < aSpacePoint2.GetDetectorElem()->GetZPos());
}

bool SbtSpacePoint::isValid() const {
  // checks to see if space point is actually on active detector
  if (!_detectorElem) return false;

  TVector3 local;
  _detectorElem->MasterToLocal(_point, local);

  bool ok = _detectorElem->InActiveArea(local);

  return ok;
}

void SbtSpacePoint::InitError() {
  const SbtDetectorElem* detElem = GetDetectorElem();
  assert(detElem);
  SbtDetectorType* detType = detElem->GetDetectorType();
  assert(detType);
  std::string detTypeName = detType->GetType();

  if (_errorMethod == "Simple") {
    // define error for simple method
    _pointErr[0] = _trackDetErr;
    _pointErr[1] = _trackDetErr;
    _pointErr[2] = 0.1;  // default 1 mm error on z coordinate
  }

  else if (_errorMethod == "ErrorPropagation") {
    if (detTypeName == "strip") {  // tele detector
      SbtHit* uHit = GetHitU();
      SbtHit* vHit = GetHitV();
      if (!uHit || !vHit) return;
      SbtCluster* uCluster = uHit->GetCluster();
      SbtCluster* vCluster = vHit->GetCluster();
      int uLength = uCluster->GetLength();
      int vLength = vCluster->GetLength();
      double uPH = uCluster->GetPulseHeight();
      double vPH = vCluster->GetPulseHeight();

      double uPitch = detType->GetUpitch();
      double vPitch = detType->GetVpitch();

      auto uDigiList = uCluster->GetDigiList();
      auto vDigiList = vCluster->GetDigiList();

      if (uLength <= 1 || uDigiList.size() != uLength) {
        _pointErr[0] = uPitch / sqrt(12.);
      }
      else {
        double w2 = 0;
        for (auto digi : uDigiList) {
          w2 += digi->GetADC() * digi->GetADC();
        }
        _pointErr[0] = sqrt(w2) / uPH * uPitch / sqrt(12.);
      }
      if (vLength <= 1 || vDigiList.size() != vLength) {
        _pointErr[1] = vPitch / sqrt(12.);
      }
      else {
        double w2 = 0;
        for (auto digi : vDigiList) {
          w2 += digi->GetADC() * digi->GetADC();
        }
        _pointErr[1] = sqrt(w2) / vPH * vPitch / sqrt(12.);
      }
      _pointErr[2] = detType->GetZ_HalfDim();
    }
  }

  else if (_errorMethod == "FirstPrinciple") {
    // this method is intended for strip dets only
    if (detTypeName == "strip") {  // tele detector

      SbtHit* uHit = GetHitU();
      SbtHit* vHit = GetHitV();
      SbtCluster* uCluster = uHit->GetCluster();
      SbtCluster* vCluster = vHit->GetCluster();
      int uLength = uCluster->GetLength();
      int vLength = vCluster->GetLength();
      double uPH = uCluster->GetPulseHeight();
      // double vPH = vCluster->GetPulseHeight();

      double uPitch = detType->GetUpitch();
      double vPitch = detType->GetVpitch();

      double cff = 2.12;  // cluster form factor

      if (uLength == 1)
        _pointErr[0] = uPitch / sqrt(12.);
      else
        _pointErr[0] =
            cff * sqrt(double(uLength)) * thrTeleDac[0] / uPH * uPitch;

      if (vLength == 1)
        _pointErr[1] = vPitch / sqrt(12.);
      else
        _pointErr[1] =
            cff * sqrt(double(vLength)) * thrTeleDac[1] / uPH * vPitch;

      _pointErr[2] = 0.1;  // default 1 mm error on z coordinate
    }

    else if (detTypeName == "striplet") {  // striplet detector

      SbtHit* uHit = GetHitU();
      SbtHit* vHit = GetHitV();
      SbtCluster* uCluster = uHit->GetCluster();
      SbtCluster* vCluster = vHit->GetCluster();
      int uLength = uCluster->GetLength();
      int vLength = vCluster->GetLength();
      double uPH = uCluster->GetPulseHeight();
      // double vPH = vCluster->GetPulseHeight();

      double uPitch = detType->GetUpitch();
      double vPitch = detType->GetVpitch();

      double cff = 2.12;  // cluster form factor

      if (uLength == 1)
        _pointErr[0] = uPitch / sqrt(12.);
      else
        _pointErr[0] =
            cff * sqrt(double(uLength)) * thrStripletDac[0] / uPH * uPitch;

      if (vLength == 1)
        _pointErr[1] = vPitch / sqrt(12.);
      else
        _pointErr[1] =
            cff * sqrt(double(vLength)) * thrStripletDac[1] / uPH * vPitch;

      _pointErr[2] = 0.1;  // default 1 mm error on z coordinate
    }
  }

  if (GetDetectorElem()->GetDetectorType()->GetType() == "singleside")
    _pointErr[1] = (GetDetectorElem()->GetDetectorType()->GetVActMax() -
                    GetDetectorElem()->GetDetectorType()->GetVActMin()) /
                   2;
}

SbtHit* SbtSpacePoint::GetHitU() const {
  assert(_digiType == SbtEnums::digiType::strip);
  return _hitU;
}

SbtHit* SbtSpacePoint::GetHitV() const {
  assert(_digiType == SbtEnums::digiType::strip);
  return _hitV;
}

SbtCluster* SbtSpacePoint::GetPxlCluster() const {
  assert(_digiType == SbtEnums::digiType::pixel);
  assert(_pxlCluster != 0);
  return _pxlCluster;
}

void SbtSpacePoint::print() const {
  std::cout << "SbtSpacePoint" << std::endl;
  _point.Print();
  if (_detectorElem) {
    std::cout << "Detector element " << _detectorElem->GetID() << std::endl;
  }
  else {
    std::cout << "No Detector element assigned" << std::endl;
  }
  std::cout << "Is on track " << _IsOnTrack << std::endl;
  std::cout << "Digit type " << _digiType << std::endl;
  std::cout << "Space point type " << _spacePointType << std::endl;
}
