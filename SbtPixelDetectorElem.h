#ifndef SBT_PIXELDETECTORELEM
#define SBT_PIXELDETECTORELEM

#include <cassert>

#include "SbtDetectorElem.h"
#include "SbtEnums.h"

class TGeoManager;
class SbtDetectorType;

class SbtPixelDetectorElem : public SbtDetectorElem {
 public:
  SbtPixelDetectorElem(TGeoManager* geom, SbtDetectorType* detType,
                       int detElemId, int detTrackId, double xPos, double yPos,
                       double zPos, double Phi, double Theta, double Psi,
                       double orientation_U, double orientation_V);

  ~SbtPixelDetectorElem() {;}

  // define a function that given the electronic address
  // return the local coordinate address of the pixel
  void ChannelToLocal(int chip, int channel, char side, SbtEnums::view coordSide) const;

  void GetPxlDigiData(int Row, int Column, int& macroColumn, int& columnInMP, int& row) const;

  int GetChannelNumber(int chip, int set, int strip) const { assert(0); return 0; }
  void GetStripDigiData(int channel, int& c, int& set, int& strip) const { assert(0); return; }

  ClassDef(SbtPixelDetectorElem, 1);  // Implementation of PixelDetectorElem
};

#endif
