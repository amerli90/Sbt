#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "SbtCluster.h"
#include "SbtDetectorElem.h"
#include "SbtDetectorType.h"
#include "SbtDigi.h"
#include "SbtHit.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"

#include <TGeoMatrix.h>

ClassImp(SbtTrack);

SbtTrack::SbtTrack()
    : _DebugLevel(0),
      _trackType(SbtEnums::objectType::reconstructed),
      _trackShape(SbtEnums::trackShape::longTrack),
      _trackFunctionX(nullptr),
      _trackFunctionY(nullptr) {
  reset();
}

SbtTrack::SbtTrack(std::vector<SbtHit*> aHitList, SbtEnums::objectType type, SbtEnums::trackShape shape)
    : _DebugLevel(0),
      _trackType(type),
      _trackShape(shape),
      _trackFunctionX(nullptr),
      _trackFunctionY(nullptr) {
  // initiliaze the covariance matrix
  reset();
  _hitList = aHitList;
}

SbtTrack::SbtTrack(std::vector<SbtSpacePoint*> aSpacePointList, SbtEnums::objectType type, SbtEnums::trackShape shape)
    : _DebugLevel(0),
      _trackType(type),
      _trackShape(shape),
      _trackFunctionX(nullptr),
      _trackFunctionY(nullptr) {
  
  reset();
  _spacePointList = aSpacePointList;

  // sort the space points in ascending z position
  std::sort(_spacePointList.begin(), _spacePointList.end(), SbtSpacePoint::ltz_ptr);

  if (type == SbtEnums::objectType::ideal) {
    SetIdealTrackParms();
  }
  //
  // if reconstructed track, set flat is-on-track for all spacepoints,
  // strip/pixel hits and clusters, digis if they exist
  //
  if (_trackType == SbtEnums::objectType::reconstructed) SetIsOnTrack();
}

SbtTrack::SbtTrack(SbtSpacePoint* SP0, SbtSpacePoint* SP1, SbtEnums::objectType type, SbtEnums::trackShape shape)
    : _DebugLevel(0),
      _trackType(type),
      _trackShape(shape),
      _trackFunctionX(nullptr),
      _trackFunctionY(nullptr) {
  reset();
  _spacePointList.push_back(SP0);
  _spacePointList.push_back(SP1);

  if (_DebugLevel > 0) std::cout << "SbtTrack:  DebugLevel= " << _DebugLevel << std::endl;

  // sort the space points in ascending z position
  std::sort(_spacePointList.begin(), _spacePointList.end(), SbtSpacePoint::ltz_ptr);

  if (_trackType == SbtEnums::objectType::reconstructed) SetIsOnTrack();
}

SbtTrack::SbtTrack(SbtSpacePoint* SP0, SbtSpacePoint* SP1, SbtSpacePoint* SP2,
                   SbtSpacePoint* SP3, SbtEnums::objectType type, SbtEnums::trackShape shape)
    : _DebugLevel(0),
      _trackType(type),
      _trackShape(shape),
      _trackFunctionX(nullptr),
      _trackFunctionY(nullptr) {
  reset();
  _spacePointList.push_back(SP0);
  _spacePointList.push_back(SP1);
  _spacePointList.push_back(SP2);
  _spacePointList.push_back(SP3);

  if (_DebugLevel > 0) std::cout << "SbtTrack:  DebugLevel= " << _DebugLevel << std::endl;

  // sort the space points in ascending z position
  std::sort(_spacePointList.begin(), _spacePointList.end(), SbtSpacePoint::ltz_ptr);

  if (type == SbtEnums::objectType::ideal) {
    SetIdealTrackParms();
  }

  // digi type are not produced at the moment for simulated tracks
  // track are produced directly from SpacePoints
  if (_trackType == SbtEnums::objectType::reconstructed) SetIsOnTrack();
}

SbtTrack::SbtTrack(SbtSpacePoint* SP0, SbtSpacePoint* SP1, SbtSpacePoint* SP2,
                   SbtSpacePoint* SP3, SbtSpacePoint* SP4, SbtSpacePoint* SP5,
                   SbtEnums::objectType type, SbtEnums::trackShape shape)
    : _DebugLevel(0),
      _trackType(type),
      _trackShape(shape),
      _trackFunctionX(nullptr),
      _trackFunctionY(nullptr) {

  reset();

  _spacePointList.push_back(SP0);
  _spacePointList.push_back(SP1);
  _spacePointList.push_back(SP2);
  _spacePointList.push_back(SP3);
  _spacePointList.push_back(SP4);
  _spacePointList.push_back(SP5);

  if (_DebugLevel > 0) std::cout << "SbtTrack:  DebugLevel= " << _DebugLevel << std::endl;

  // sort the space points in ascending z position
  std::sort(_spacePointList.begin(), _spacePointList.end(), SbtSpacePoint::ltz_ptr);

  if (type == SbtEnums::objectType::ideal) {
    SetIdealTrackParms();
  }

  // digi type are not produced at the moment for simulated tracks
  // track are produced directly from SpacePoints
  if (_trackType == SbtEnums::objectType::reconstructed) SetIsOnTrack();
}

SbtTrack::SbtTrack(std::array<SbtSpacePoint*,2> us, std::array<SbtSpacePoint*,2> ds, SbtEnums::objectType type, SbtEnums::trackShape shape) :
  _DebugLevel(0),
  _trackType(type),
  _trackShape(shape),
  _trackFunctionX(nullptr),
  _trackFunctionY(nullptr) {
  reset();
  _spacePointList.push_back(us[0]);
  _spacePointList.push_back(us[1]);
  _spacePointList.push_back(ds[0]);
  _spacePointList.push_back(ds[1]);
  std::sort(_spacePointList.begin(), _spacePointList.end(), SbtSpacePoint::ltz_ptr);
  if (_trackType == SbtEnums::objectType::reconstructed) SetIsOnTrack();
}

SbtTrack::~SbtTrack() {
  delete _trackFunctionX;
  delete _trackFunctionY;
}

SbtTrack::SbtTrack(const SbtTrack& other) :
  _trackFunctionX(nullptr),
  _trackFunctionY(nullptr) { 
  *this = other; 
}

SbtTrack::SbtTrack(const SbtTrack&& other) :
  _trackFunctionX(nullptr),
  _trackFunctionY(nullptr) {
  *this = std::move(other);
}

SbtTrack& SbtTrack::operator=(const SbtTrack& other) {
  if (this != &other) {
    _DebugLevel = other._DebugLevel;
    _trackType = other._trackType;
    _trackShape = other._trackShape;
    if (_trackFunctionX != other._trackFunctionX) {
      if (_trackFunctionX) delete _trackFunctionX;
      if (other._trackFunctionX) _trackFunctionX = new TF1(*other._trackFunctionX);
    }
    if (_trackFunctionY != other._trackFunctionY) {
      if (_trackFunctionY) delete _trackFunctionY;
      if (other._trackFunctionY) _trackFunctionY = new TF1(*other._trackFunctionY);
    }
    _fitStatus = other._fitStatus;
    _ndof = other._ndof;
    _chi2 = other._chi2;
    memcpy(_residualX, other._residualX, sizeof(double) * maxTrkNSpacePoint);
    memcpy(_residualY, other._residualY, sizeof(double) * maxTrkNSpacePoint);
    memcpy(_recoX, other._recoX, sizeof(double) * maxTrkNSpacePoint);
    memcpy(_recoY, other._recoY, sizeof(double) * maxTrkNSpacePoint);
    memcpy(_fitX, other._fitX, sizeof(double) * maxTrkNSpacePoint);
    memcpy(_fitY, other._fitY, sizeof(double) * maxTrkNSpacePoint);
    _hitList = other._hitList;
    _spacePointList = other._spacePointList;
    _simulationSlpX = other._simulationSlpX;
    _simulationSlpY = other._simulationSlpY;
    _simulationPointX = other._simulationPointX;
    _simulationPointY = other._simulationPointY;
    _simulationPointZ = other._simulationPointZ;

    if (_CovX.GetNcols() != other._CovX.GetNcols() || _CovX.GetNrows() != other._CovX.GetNrows()) {
      _CovX.ResizeTo(other._CovX.GetNrows(), other._CovX.GetNcols());
    }

    if (_CovY.GetNcols() != other._CovY.GetNcols() || _CovY.GetNrows() != other._CovY.GetNrows()) {
      _CovY.ResizeTo(other._CovY.GetNrows(), other._CovY.GetNcols());
    }

    _CovX = other._CovX;
    _CovY = other._CovY;
  }
  return *this;
}

SbtTrack& SbtTrack::operator=(const SbtTrack&& other) {
  if (this != &other) {
    _DebugLevel = other._DebugLevel;
    _trackType = other._trackType;
    _trackShape = other._trackShape;
    _trackFunctionX = other._trackFunctionX;
    _trackFunctionY = other._trackFunctionY;
    _fitStatus = other._fitStatus;
    _ndof = other._ndof;
    _chi2 = other._chi2;
    memcpy(_residualX, other._residualX, sizeof(double) * maxTrkNSpacePoint);
    memcpy(_residualY, other._residualY, sizeof(double) * maxTrkNSpacePoint);
    memcpy(_recoX, other._recoX, sizeof(double) * maxTrkNSpacePoint);
    memcpy(_recoY, other._recoY, sizeof(double) * maxTrkNSpacePoint);
    memcpy(_fitX, other._fitX, sizeof(double) * maxTrkNSpacePoint);
    memcpy(_fitY, other._fitY, sizeof(double) * maxTrkNSpacePoint);
    _hitList = std::move(other._hitList);
    _spacePointList = std::move(other._spacePointList);
    _simulationSlpX = std::move(other._simulationSlpX);
    _simulationSlpY = std::move(other._simulationSlpY);
    _simulationPointX = std::move(other._simulationPointX);
    _simulationPointY = std::move(other._simulationPointY);
    _simulationPointZ = std::move(other._simulationPointZ);
    _CovX = std::move(other._CovX);
    _CovY = std::move(other._CovY);
  }
  return *this;
}

void SbtTrack::reset() {
  _CovX.Zero();
  _CovY.Zero();

  if (_DebugLevel > 0) std::cout << "SbtTrack::reset  DebugLevel= " << _DebugLevel << std::endl;
  // set default value for residaul
  for (int i = 0; i < maxTrkNSpacePoint; i++) {
    _residualX[i] = -999.0;
    _residualY[i] = -999.0;
    _recoX[i] = -999.0;
    _recoY[i] = -999.0;
    _fitX[i] = -999.0;
    _fitY[i] = -999.0;
  }
  delete _trackFunctionX;
  _trackFunctionX = nullptr;
  delete _trackFunctionY;
  _trackFunctionY = nullptr;
  _fitStatus = -1;
  _ndof = -1;
  _chi2 = -1;

  _hitList.clear();
  _spacePointList.clear();
  _simulationSlpX.clear();
  _simulationSlpY.clear();
  _simulationPointX.clear();
  _simulationPointY.clear();
  _simulationPointZ.clear();
}

void SbtTrack::SetIsOnTrack() {
  for (auto Sp : _spacePointList) {
    Sp->SetIsOnTrack(true);
    if (Sp->GetDigitType() == SbtEnums::digiType::strip) {
      if (Sp->GetDetectorElem()->GetDetectorType()->GetType() == "singleside") {
        if (Sp->GetHitU()) {
          Sp->GetHitU()->SetIsOnTrack(true);
          Sp->GetHitU()->GetCluster()->SetIsOnTrack(true);
          SetDigiIsOnTrack(Sp->GetHitU()->GetCluster()->GetDigiList());
        }
      }
      else {
        if (Sp->GetHitU() && Sp->GetHitV()) {
          Sp->GetHitU()->SetIsOnTrack(true);
          Sp->GetHitV()->SetIsOnTrack(true);
          Sp->GetHitU()->GetCluster()->SetIsOnTrack(true);
          SetDigiIsOnTrack(Sp->GetHitU()->GetCluster()->GetDigiList());
          Sp->GetHitV()->GetCluster()->SetIsOnTrack(true);
          SetDigiIsOnTrack(Sp->GetHitV()->GetCluster()->GetDigiList());
        }
      }
    } 
    else if (Sp->GetDigitType() == SbtEnums::digiType::pixel) {
      Sp->GetPxlCluster()->SetIsOnTrack(true);
      SetDigiIsOnTrack(Sp->GetPxlCluster()->GetDigiList());
    } 
  }
}

void SbtTrack::SetDigiIsOnTrack(std::vector<SbtDigi*> aDigiList) {
  for (auto digi : aDigiList) {
    digi->SetIsOnTrack(true);
  }
}

void SbtTrack::AddHit(SbtHit* aHit) {
  if (aHit == 0) {
    std::cout << "SbtTrack::AddHit tried to add null SbtHit pointer" << std::endl;
    assert(0);
  } else {
    _hitList.push_back(aHit);
  }
}

void SbtTrack::AddSpacePoint(SbtSpacePoint* aSpacePoint) {
  if (aSpacePoint == 0) {
    std::cout << "SbtTrack::AddSpacePoint tried to add null SbtSpacePoint pointer" << std::endl;
    assert(0);
  }
  else {
    _spacePointList.push_back(aSpacePoint);
  }
}

void SbtTrack::SortSpacePoints() {
  std::sort(_spacePointList.begin(), _spacePointList.end(), SbtSpacePoint::ltz_ptr);
}

void SbtTrack::AddTrackParmsAtNode(double slpx, double slpy, double x, double y, double z) {
  _simulationSlpX.push_back(slpx);
  _simulationSlpY.push_back(slpy);
  _simulationPointX.push_back(x);
  _simulationPointY.push_back(y);
  _simulationPointZ.push_back(z);
}

void SbtTrack::SetIdealTrackParms() {
  if (_spacePointList.size() >= 2) {
    SbtSpacePoint* sp0 = _spacePointList.at(0);
    SbtSpacePoint* sp1 = _spacePointList.at(1);
    double Bx = (sp0->GetXPosition() - sp1->GetXPosition()) /
                (sp0->GetZPosition() - sp1->GetZPosition());
    double Ax = sp0->GetXPosition() - Bx * sp0->GetZPosition();

    double By = (sp0->GetYPosition() - sp1->GetYPosition()) /
                (sp0->GetZPosition() - sp1->GetZPosition());
    double Ay = sp0->GetYPosition() - By * sp0->GetZPosition();

    SetAx(Ax);
    SetAy(Ay);
    SetBx(Bx);
    SetBy(By);
  }
  else {
    std::cout << "Warning: NULL SbtSpacePoint pointer." << std::endl;
    std::cout << "I will set your ideal track parms to dummy values" << std::endl;
    if (_trackFunctionX) {
      delete _trackFunctionX;
      _trackFunctionX = nullptr;
    }
    if (_trackFunctionY) {
      delete _trackFunctionY;
      _trackFunctionY = nullptr;
    }
  }
}

void SbtTrack::Print() const {
  std::cout << "SbtTrack::Print()" << std::endl;
  std::cout << "Fit status = " << _fitStatus << std::endl;
  std::cout << "Track parameters values: " << std::endl;
  std::cout << "Ax = " << GetAx() << "\t "
       << "Ay = " << GetAy() << "\t "
       << "Bx = " << GetBx() << "\t "
       << "By = " << GetBy() << std::endl;
  if (_trackFunctionX && _trackFunctionY) {
    for (int i = 0; i < _spacePointList.size(); i++) { 
      std::cout << "Residual SP[" << i << "] (x,y) = (" << _residualX[i] << ","
           << _residualY[i] << ")" << std::endl;
    }
    std::cout << "Chi2 = " << GetChi2() << "\t ndof = " << GetNdof() << std::endl;
  }
}

bool SbtTrack::IntersectPlane(TVector3 p1, TVector3 p2, const SbtDetectorElem* detElem, TVector3& point) const {
  // intersect line defined by p1 and p2 (in global coordinates) with detector
  // element. Returned point is in global coordinates.

  // active area check not yet implemented...
  // returns true if resulting
  // point is in the active area of the detector

  // construct a point on the plane and its normal
  TVector3 localP3(0, 0, 0);
  TVector3 p3;

  detElem->LocalToMaster(localP3, p3);

  // endpoint of normal in local, global coords
  TVector3 localNend(0, 0, 1);
  TVector3 Nend;

  detElem->LocalToMaster(localNend, Nend);

  // normal in global coords is endpoint minus startpoint (=p3)
  TVector3 N = Nend - p3;

  assert(N.Dot(p2 - p1) != 0);

  point = p1 + N.Dot(p3 - p1) / N.Dot(p2 - p1) * (p2 - p1);

  // check if in active area
  // put point in local coords
  TVector3 pointLocal;
  detElem->MasterToLocal(point, pointLocal);

  return detElem->InActiveArea(pointLocal);
}

void SbtTrack::Residual() {
  // Attention: this is a residual evaluation for debugging purposes.
  if (_trackFunctionX && _trackFunctionY) {
    int i = 0;
    for (auto Sp : _spacePointList) {
      Residual(Sp, i);
      i++;
    }
  }
}

void SbtTrack::Residual(SbtSpacePoint* SP, int i) {
  // Attention: this is a residual evaluation for debugging purposes.
  if (_trackFunctionX && _trackFunctionY) {
    _recoX[i] = SP->GetXPosition();
    _recoY[i] = SP->GetYPosition();
    _fitX[i] = _trackFunctionX->Eval(SP->GetZPosition());
    _fitY[i] = _trackFunctionY->Eval(SP->GetZPosition());

    _residualX[i] = _recoX[i] - _fitX[i];
    _residualY[i] = _recoY[i] - _fitY[i];
  }
}

bool SbtTrack::ResidualsOnWafer(const SbtSpacePoint& SP, std::array<double,2>& resids) const {
  // returns the residual (point - track) for SP with respect to this track
  // the residual is measured in local coordinates u, v

  if (!_trackFunctionX || !_trackFunctionY){
    return false;
  }

  const SbtDetectorElem* detElem = SP.GetDetectorElem();

  // Choose any two points on the fitted line
  TVector3 p1(_trackFunctionX->Eval(-10), _trackFunctionY->Eval(-10), -10.);

  TVector3 p2(_trackFunctionX->Eval(0), _trackFunctionY->Eval(0), 0.);

  TVector3 exPoint;  // in global coords
  bool ok = IntersectPlane(p1, p2, detElem, exPoint);

  // go to local
  TVector3 localExPoint;
  detElem->MasterToLocal(exPoint, localExPoint);

  // now convert space point to local coordinates

  TVector3 localSP;
  detElem->MasterToLocal(SP.point(), localSP);

  resids[0] = localSP.x() - localExPoint.x();
  resids[1] = localSP.y() - localExPoint.y();

  return ok;
}

bool SbtTrack::IntersectPlane(const SbtDetectorElem* detElem, TVector3& point) const {
  if (!_trackFunctionX || !_trackFunctionY){
    return false;
  }

  // Choose any two points on the fitted track (global coordinates)

  TVector3 p1(_trackFunctionX->Eval(-10), _trackFunctionY->Eval(-10), -10.);

  TVector3 p2(_trackFunctionX->Eval(0), _trackFunctionY->Eval(0), 0.);

  bool ok = IntersectPlane(p1, p2, detElem, point);
  return ok;
}

void SbtTrack::SetXCovMatrix(TMatrixD CovX) {
  if (_CovX.GetNcols() != CovX.GetNcols() || _CovX.GetNrows() != CovX.GetNrows()) {
    _CovX.ResizeTo(CovX);
  }
  _CovX = CovX;
}

void SbtTrack::SetYCovMatrix(TMatrixD CovY) {
  if (_CovY.GetNcols() != CovY.GetNcols() || _CovY.GetNrows() != CovY.GetNrows()) {
    _CovY.ResizeTo(CovY);
  }
  _CovY = CovY;
}

const TMatrixD& SbtTrack::GetXCovMatrix() const {
  return _CovX;
}

const TMatrixD& SbtTrack::GetYCovMatrix() const {
  return _CovY;
}

double SbtTrack::GetAx() const { 
  if (!_trackFunctionX) return -999.;
  return _trackFunctionX->GetParameter(0);
}

double SbtTrack::GetAy() const { 
  if (!_trackFunctionY) return -999.;
  return _trackFunctionY->GetParameter(0);
}

double SbtTrack::GetBx() const { 
  if (!_trackFunctionX) return -999.;
  return _trackFunctionX->GetParameter(1);
}

double SbtTrack::GetBy() const { 
  if (!_trackFunctionY) return -999.;
  return _trackFunctionY->GetParameter(1);
}

void SbtTrack::SetAx(double Ax) { 
  if (!_trackFunctionX) _trackFunctionX = CreateLinearTrackFunction();
  _trackFunctionX->SetParameter(0, Ax);
}

void SbtTrack::SetAy(double Ay) { 
  if (!_trackFunctionY) _trackFunctionY = CreateLinearTrackFunction();
  _trackFunctionY->SetParameter(0, Ay);
}

void SbtTrack::SetBx(double Bx) { 
  if (!_trackFunctionX) _trackFunctionX = CreateLinearTrackFunction();
  _trackFunctionX->SetParameter(1, Bx);
}

void SbtTrack::SetBy(double By) { 
  if (!_trackFunctionY) _trackFunctionY = CreateLinearTrackFunction();
  _trackFunctionY->SetParameter(1, By);
}

TF1* SbtTrack::CreateLinearTrackFunction() const {
  double zmin = 0, zmax = 100;
  if (_spacePointList.size() > 2) {
    zmin = _spacePointList.front()->GetZPosition();
    zmax = _spacePointList.back()->GetZPosition();
    double range = zmax - zmin;
    zmin -= 0.5 * range;
    zmax += 0.5 * range;
  }
  TF1* linearTrack = new TF1("_trackFunctionX", "[0] + [1] * x", zmin, zmax);
  linearTrack->SetParNames("Intercept", "Slope");
  return linearTrack;
}

double SbtTrack::GetSlopeX() const {
  if (!_trackFunctionX) return -999.;
  return _trackFunctionX->GetParameter("Slope");
}

double SbtTrack::GetInterceptX() const {
  if (!_trackFunctionX) return -999.;
  return _trackFunctionX->GetParameter("Intercept");
}

double SbtTrack::GetSlopeY() const {
  if (!_trackFunctionY) return -999.;
  return _trackFunctionY->GetParameter("Slope");
}

double SbtTrack::GetInterceptY() const {
  if (!_trackFunctionY) return -999.;
  return _trackFunctionY->GetParameter("Intercept");
}

double SbtTrack::GetBendingPoint() const {
  TF1* trackFunc = nullptr;
  if (_trackShape == SbtEnums::trackShape::channelledTrackX) {
    trackFunc = _trackFunctionX;
  }
  else if (_trackShape == SbtEnums::trackShape::channelledTrackY) {
    trackFunc = _trackFunctionY;
  }
  if (trackFunc) {
    return trackFunc->GetParameter("BendingPoint");
  }
  else {
    return -999.;
  }
}

double SbtTrack::GetBentSlopeX() const {
  if (_trackFunctionX && (_trackShape == SbtEnums::trackShape::channelledTrackX || _trackShape == SbtEnums::trackShape::channelledTrack)) {
    return _trackFunctionX->GetParameter("BentSlope");
  }
  else {
    return 0.;
  }
}

double SbtTrack::GetDeflectionAngleX() const {
  if (_trackFunctionX && (_trackShape == SbtEnums::trackShape::channelledTrackX || _trackShape == SbtEnums::trackShape::channelledTrack)) {
    return TMath::ATan(_trackFunctionX->GetParameter("BentSlope")) - TMath::ATan(_trackFunctionX->GetParameter("Slope"));
  }
  else {
    return 0.;
  }
}

double SbtTrack::GetBentSlopeY() const {
  if (_trackFunctionY && (_trackShape == SbtEnums::trackShape::channelledTrackY || _trackShape == SbtEnums::trackShape::channelledTrack)) {
    return _trackFunctionY->GetParameter("BentSlope");
  }
  else {
    return 0.;
  }
}

double SbtTrack::GetDeflectionAngleY() const {
  if (_trackFunctionY && (_trackShape == SbtEnums::trackShape::channelledTrackY || _trackShape == SbtEnums::trackShape::channelledTrack)) {
    return TMath::ATan(_trackFunctionY->GetParameter("BentSlope")) - TMath::ATan(_trackFunctionY->GetParameter("Slope"));
  }
  else {
    return 0.;
  }
}

double SbtTrack::GetSlopeXTwoPoints() const {
  if (_spacePointList.size() < 2) return 999.;
  return (_spacePointList[0]->GetXPosition() - _spacePointList[1]->GetXPosition()) / (_spacePointList[0]->GetZPosition() - _spacePointList[1]->GetZPosition());
}

double SbtTrack::GetSlopeYTwoPoints() const {
  if (_spacePointList.size() < 2) return 999.;
  return (_spacePointList[0]->GetYPosition() - _spacePointList[1]->GetYPosition()) / (_spacePointList[0]->GetZPosition() - _spacePointList[1]->GetZPosition());
}

double SbtTrack::GetBentSlopeXTwoPoints() const {
  if (_spacePointList.size() < 4) return 999.;
  return (_spacePointList[2]->GetXPosition() - _spacePointList[3]->GetXPosition()) / (_spacePointList[2]->GetZPosition() - _spacePointList[3]->GetZPosition());
}

double SbtTrack::GetBentSlopeYTwoPoints() const {
  if (_spacePointList.size() < 4) return 999.;
  return (_spacePointList[2]->GetYPosition() - _spacePointList[3]->GetYPosition()) / (_spacePointList[2]->GetZPosition() - _spacePointList[3]->GetZPosition());
}

double SbtTrack::GetDeflectionAngleXFourPoints() const {
  if (_spacePointList.size() < 4) return 0.;
  return TMath::ATan(GetBentSlopeXTwoPoints()) - TMath::ATan(GetSlopeXTwoPoints());
}

double SbtTrack::GetDeflectionAngleYFourPoints() const {
  if (_spacePointList.size() < 4) return 0.;
  return TMath::ATan(GetBentSlopeYTwoPoints()) - TMath::ATan(GetSlopeYTwoPoints());
}
