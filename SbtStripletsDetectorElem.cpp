#include <cmath>
#include <iostream>

#include "SbtDetectorType.h"
#include "SbtStripletsDetectorElem.h"

ClassImp(SbtStripletsDetectorElem);

SbtStripletsDetectorElem::SbtStripletsDetectorElem(
  TGeoManager *geom, SbtDetectorType *detType, int detElemId,
  int detTrackId, double xPos, double yPos, double zPos, double Phi,
  double Theta, double Psi, double orientation_U, double orientation_V) :
  SbtDetectorElem(geom, detType, detElemId, detTrackId, xPos, yPos, zPos,
                  Phi, Theta, Psi, orientation_U, orientation_V)
{
  std::cout << "SbtStripletsDetectorElem:  DebugLevel= " << _DebugLevel << std::endl;
}

int SbtStripletsDetectorElem::GetChannelNumber(int chip, int set, int strip) const {
  return chip * 128 + set * 8 + strip;
}

void SbtStripletsDetectorElem::GetStripDigiData(int channel, int &chip, int &set, int &strip) const {
  chip = channel / 128;

  int rest = channel % 128;

  set = rest / 8;
  strip = rest % 8;
}

void SbtStripletsDetectorElem::ChannelToLocal(int chip, int channel, char side, SbtEnums::view coordSide) const {
  // in the case of the striplet the local coordinates x,y (local volume
  // coordinate)
  // are not collinear with u,v coordinates (coordinates collinear with the
  // strips)
}

void SbtStripletsDetectorElem::LocalToMaster(double *local, double *master) const {
  double localprime[3];
  LocalToLocalPrime(local, localprime);
  LocalPrimeToMaster(localprime, master);
}

void SbtStripletsDetectorElem::MasterToLocal(double *master, double *local) const {
  double localprime[3];
  MasterToLocalPrime(master, localprime);
  LocalPrimeToLocal(localprime, local);
}

bool SbtStripletsDetectorElem::InActiveArea(TVector3 point) const {
  // This is in local coordinates
  bool ok = SbtDetectorElem::InActiveArea(point);

  // now check if on the physical silicon
  if (ok) {
    TVector3 localPrime;
    LocalToLocalPrime(point, localPrime);

    bool ok2 = (localPrime.X() > _detType->GetUPrimeActMin()) &
               (localPrime.X() < _detType->GetUPrimeActMax()) &
               (localPrime.Y() > _detType->GetVPrimeActMin()) &
               (localPrime.Y() < _detType->GetVPrimeActMax());

    return ok2;
  }
  else {
    return false;
  }
}
