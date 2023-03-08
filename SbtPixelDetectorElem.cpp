#include "SbtPixelDetectorElem.h"
#include <iostream>

ClassImp(SbtPixelDetectorElem);

SbtPixelDetectorElem::SbtPixelDetectorElem(
    TGeoManager *geom, SbtDetectorType *detType, int detElemId, int detTrackId,
    double xPos, double yPos, double zPos, double Phi, double Theta, double Psi,
    double orientation_U, double orientation_V) : 
    SbtDetectorElem(geom, detType, detElemId, detTrackId, xPos, yPos, zPos,
                    Phi, Theta, Psi, orientation_U, orientation_V)
{
  std::cout << "SbtPixelDetectorElem:  DebugLevel= " << _DebugLevel << std::endl;
}

void SbtPixelDetectorElem::ChannelToLocal(int chip, int channel, char side, SbtEnums::view coordSide) const {
  // in the case of the pixel the local coordinates x,y (local volume
  // coordinate)
  // correspond to the center of the pixel.
}

void SbtPixelDetectorElem::GetPxlDigiData(int Row, int Column, int &macroColumn, int &columnInMP, int &row) const {
  macroColumn = Column / 4;
  columnInMP = Column % 4;
  row = Row;
}
