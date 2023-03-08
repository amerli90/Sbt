#include <cassert>
#include <iostream>

#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include <TGeoVolume.h>
#include <TMath.h>

#include "SbtDetectorElem.h"
#include "SbtDetectorType.h"

#include <TVector3.h>

ClassImp(SbtDetectorElem);

SbtDetectorElem::SbtDetectorElem(TGeoManager *geom,
                                     SbtDetectorType *detType, int detElemId,
                                     int detTrackId, double xPos, double yPos,
                                     double zPos, double Phi, double Theta,
                                     double Psi, double orientation_U,
                                     double orientation_V)
    : _DebugLevel(0),
      _detType(detType),
      _detElemId(detElemId),
      _detTrackId(detTrackId),
      _geoManager(geom),
      _xPos(xPos),
      _yPos(yPos),
      _zPos(zPos),
      _orientation_U(orientation_U),
      _orientation_V(orientation_V) {
  std::cout << "SbtDetectorElem:  DebugLevel= " << _DebugLevel << std::endl;
  // Require detectorElemId in [0-10]
  if (_detElemId > 10 || _detElemId < 0) {
    std::cout << "SbtDetectorElem : ERROR detElemID not valid " << std::endl;
    assert(0);
  }

  TGeoMaterial *matSi = new TGeoMaterial("Silicon", geom->GetElementTable()->FindElement("silicon"), 2.33);
  TGeoMedium *Si = nullptr;
  if (_detType->GetTypeMaterial() == "silicon") {
    Si = new TGeoMedium("Silicon", _detElemId, matSi);
  } 
  else {
    std::cout << "Detector Material is not Silicon" << std::endl;
    assert(0);
  }

  TGeoVolume *top = geom->GetTopVolume();

  // getting top volume name
  // it will be used to build the path
  const char *topVolumeName = top->GetName();
  char detectorPath[100];

  char detVolName[50];
  sprintf(detVolName, "detVol%d", detElemId);

  if (_DebugLevel) {
    std::cout << "Building detector " << detVolName << std::endl;
    std::cout << "Half dimensions x, y, z = " << _detType->GetX_HalfDim() << ", "
         << _detType->GetY_HalfDim() << ", " << _detType->GetZ_HalfDim()
         << std::endl;
    std::cout << "Positions  x, y, z = " << xPos << ", " << yPos << ", " << zPos
         << std::endl;
    std::cout << "Rotations (phi, theta, psi) = (" << Phi << ", " << Theta << ", "
         << Psi << ") " << std::endl;
  }

  _detVolume = geom->MakeBox(detVolName, Si, _detType->GetX_HalfDim(), _detType->GetY_HalfDim(), _detType->GetZ_HalfDim());

  _detVolume->SetLineColor(detElemId + 1);
  _rot = new TGeoRotation("rot", Phi, Theta, Psi);
  _tr = new TGeoTranslation("tr", xPos, yPos, zPos);
  _rt = new TGeoCombiTrans(*_tr, *_rot);

  if (_DebugLevel) {
    std::cout << "Built detector " << detVolName << std::endl;
  }

  sprintf(detectorPath, "/%s_1/%s_%d", topVolumeName, detVolName, detElemId);
  _thePath += detectorPath;

  if (_DebugLevel) {
    std::cout << "Detector " << detVolName << " has path: " << _thePath << std::endl;
  }

  top->AddNode(_detVolume, detElemId, _rt);

  if (_DebugLevel) {
    std::cout << "Exiting SbtDetectorElem 'ctor " << detVolName << std::endl;
  }
}
// default versions: valid for strips and pixels where local = local prime
// the striplets version in implemented in sub-class
void SbtDetectorElem::LocalToMaster(double *local, double *master) const {
  LocalPrimeToMaster(local, master);
}

void SbtDetectorElem::MasterToLocal(double *master, double *local) const {
  MasterToLocalPrime(master, local);
}

// see header file for discussion of local prime coords
void SbtDetectorElem::LocalToLocalPrime(double *local, double *localprime) const {
  double c = cos(_detType->GetStripAngle());
  double s = sin(_detType->GetStripAngle());

  localprime[0] = c * local[0] - s * local[1];
  localprime[1] = s * local[0] + c * local[1];
  localprime[2] = local[2];
}

void SbtDetectorElem::LocalPrimeToMaster(double *localprime, double *master) const {
  _geoManager->GetTopVolume()->GetNode(_detElemId)->LocalToMaster(localprime, master);
}

void SbtDetectorElem::MasterToLocalPrime(double *master, double *localprime) const {
  _geoManager->GetTopVolume()->GetNode(_detElemId)->MasterToLocal(master, localprime);
}

void SbtDetectorElem::LocalPrimeToLocal(double *localprime, double *local) const {
  double c = cos(_detType->GetStripAngle());
  double s = sin(_detType->GetStripAngle());

  local[0] = c * localprime[0] + s * localprime[1];
  local[1] = -s * localprime[0] + c * localprime[1];
  local[2] = localprime[2];
}

void SbtDetectorElem::MasterToLocalVect(double *master, double *local) const {
  MasterToLocalPrimeVect(master, local);
}

void SbtDetectorElem::MasterToLocalPrimeVect(double *master, double *localprime) const {
  _geoManager->GetTopVolume()->GetNode(_detElemId)->MasterToLocalVect(master, localprime);
}

void SbtDetectorElem::LocalToMasterVect(double *local, double *master) const {
  MasterToLocalPrimeVect(local, master);
}

void SbtDetectorElem::LocalPrimeToMasterVect(double *localprime, double *master) const {
  _geoManager->GetTopVolume()->GetNode(_detElemId)->LocalToMasterVect(master, localprime);
}

// the following versions use TVector3 instead of double*
void SbtDetectorElem::LocalToMaster(TVector3 local, TVector3 &master) const {
  double xlocal[3];
  double xmaster[3];

  for (int i = 0; i < 3; i++) xlocal[i] = local[i];

  LocalToMaster(xlocal, xmaster);

  for (int i = 0; i < 3; i++) master[i] = xmaster[i];
}

void SbtDetectorElem::MasterToLocal(TVector3 master, TVector3 &local) const {
  double xlocal[3];
  double xmaster[3];

  for (int i = 0; i < 3; i++) xmaster[i] = master[i];

  MasterToLocal(xmaster, xlocal);

  for (int i = 0; i < 3; i++) local[i] = xlocal[i];
}

void SbtDetectorElem::LocalPrimeToMaster(TVector3 local, TVector3 &master) const {
  double xlocal[3];
  double xmaster[3];

  for (int i = 0; i < 3; i++) xlocal[i] = local[i];

  LocalPrimeToMaster(xlocal, xmaster);

  for (int i = 0; i < 3; i++) master[i] = xmaster[i];
}

void SbtDetectorElem::MasterToLocalPrime(TVector3 master, TVector3 &local) const {
  double xlocal[3];
  double xmaster[3];

  for (int i = 0; i < 3; i++) xmaster[i] = master[i];

  MasterToLocalPrime(xmaster, xlocal);

  for (int i = 0; i < 3; i++) local[i] = xlocal[i];
}

void SbtDetectorElem::LocalToLocalPrime(TVector3 local, TVector3 &localprime) const {
  double xlocal[3];
  double xlocalprime[3];

  for (int i = 0; i < 3; i++) xlocal[i] = local[i];

  LocalToLocalPrime(xlocal, xlocalprime);

  for (int i = 0; i < 3; i++) localprime[i] = xlocalprime[i];
}

void SbtDetectorElem::LocalPrimeToLocal(TVector3 localprime, TVector3 &local) const {
  double xlocalprime[3];
  double xlocal[3];

  for (int i = 0; i < 3; i++) xlocalprime[i] = localprime[i];

  LocalPrimeToLocal(xlocalprime, xlocal);

  for (int i = 0; i < 3; i++) local[i] = xlocal[i];
}

void SbtDetectorElem::MasterToLocalVect(TVector3 master, TVector3 &local) const {
  double xmaster[3];
  double xlocal[3];

  for (int i = 0; i < 3; i++) xmaster[i] = master[i];

  MasterToLocalVect(xmaster, xlocal);

  for (int i = 0; i < 3; i++) local[i] = xlocal[i];
}

void SbtDetectorElem::MasterToLocalPrimeVect(TVector3 master, TVector3 &localprime) const {
  double xmaster[3];
  double xlocalprime[3];

  for (int i = 0; i < 3; i++) xmaster[i] = master[i];

  MasterToLocalPrimeVect(xmaster, xlocalprime);

  for (int i = 0; i < 3; i++) localprime[i] = xlocalprime[i];
}

void SbtDetectorElem::LocalToMasterVect(TVector3 local, TVector3 &master) const {
  double xlocal[3];
  double xmaster[3];

  for (int i = 0; i < 3; i++) xlocal[i] = local[i];

  LocalToMasterVect(xlocal, xmaster);

  for (int i = 0; i < 3; i++) master[i] = xmaster[i];
}

void SbtDetectorElem::LocalPrimeToMasterVect(TVector3 localprime, TVector3 &master) const {
  double xlocalprime[3];
  double xmaster[3];

  for (int i = 0; i < 3; i++) xlocalprime[i] = localprime[i];

  LocalPrimeToMasterVect(xlocalprime, xmaster);

  for (int i = 0; i < 3; i++) master[i] = xmaster[i];
}

bool SbtDetectorElem::ltz(SbtDetectorElem *DetElem1, SbtDetectorElem *DetElem2) {
  if ((DetElem1 == 0) || (DetElem2 == 0)) {
    std::cout << "SbtDetectorElem::ltz():  passed null SbtDetectorElem pointers" << std::endl;
    assert(0);
  }
  return (DetElem1->GetZPos() < DetElem2->GetZPos());
}

int SbtDetectorElem::GetLayerSide(SbtEnums::view LayerView) const {
  if (LayerView == SbtEnums::V) {
    assert(_detType->GetType() == "strip" || _detType->GetType() == "striplet");
  }
  return _layerSides.at(LayerView);
}

double SbtDetectorElem::Position(int channel, SbtEnums::view side) const {
  double orientation = (SbtEnums::U == side) ? _orientation_U : _orientation_V;
  return orientation * _detType->Position(channel, side);
}

int SbtDetectorElem::Channel(double pos, SbtEnums::view side) const {
  double orientation = (SbtEnums::U == side) ? _orientation_U : _orientation_V;
  return _detType->Channel(orientation * pos, side);
}

bool SbtDetectorElem::InActiveArea(TVector3 point) const {
  // is the point with the detector active area. By convention, the
  // point is in local coordinates
  return ((point.X() > _detType->GetUActMin()) &
          (point.X() < _detType->GetUActMax()) &
          (point.Y() > _detType->GetVActMin()) &
          (point.Y() < _detType->GetVActMax()));
}
