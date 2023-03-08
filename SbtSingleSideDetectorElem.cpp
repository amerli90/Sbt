#include <iostream>

#include "SbtSingleSideDetectorElem.h"

ClassImp(SbtSingleSideDetectorElem);

SbtSingleSideDetectorElem::SbtSingleSideDetectorElem(
    TGeoManager *geom, SbtDetectorType *detType, int detElemId,
    int detTrackId, double xPos, double yPos, double zPos, double Phi,
    double Theta, double Psi, double orientation_U, double orientation_V) :
  SbtDetectorElem(geom, detType, detElemId, detTrackId, xPos, yPos, zPos,
                  Phi, Theta, Psi, orientation_U, orientation_V)
{
std::cout << "SbtSingleSideDetectorElem:  DebugLevel= " << _DebugLevel << std::endl;
}

void SbtSingleSideDetectorElem::ChannelToLocal(int chip, int channel, char side, SbtEnums::view coordSide) const {
  // in the case of the strip the local coordinates x,y (local volume
  // coordinate)
  // are collinear with u,v coordinates (coordinates collinear with the strips)
}

int SbtSingleSideDetectorElem::GetChannelNumber(int chip, int set, int strip) const {
  return chip * 128 + set * 8 + strip;
}

void SbtSingleSideDetectorElem::GetStripDigiData(int channel, int &chip, int &set, int &strip) const {
  chip = channel / 128;

  int rest = channel % 128;

  set = rest / 8;
  strip = rest % 8;
}
