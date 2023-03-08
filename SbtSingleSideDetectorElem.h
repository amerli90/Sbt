#ifndef SBTSINGLESIDEDETECTORELEM_HH
#define SBTSINGLESIDEDETECTORELEM_HH

#include <cassert>

#include "SbtDetectorElem.h"
#include "SbtEnums.h"

class TGeoManager;
class SbtDetectorType;

class SbtSingleSideDetectorElem : public SbtDetectorElem {
 public:
  SbtSingleSideDetectorElem(TGeoManager* geom, SbtDetectorType* detType,
                            int detElemId, int detTrackId, double xPos,
                            double yPos, double zPos, double Phi, double Theta,
                            double Psi, double orientation_U,
                            double orientation_V);

  ~SbtSingleSideDetectorElem() {;}

  // define a function that given the electronic address
  // return the local coordinate address of the strip
  void ChannelToLocal(int chip, int channel, char side, SbtEnums::view coordSide) const;
  int GetChannelNumber(int chip, int set, int strip) const;
  void GetStripDigiData(int channel, int& chip, int& set, int& strip) const;
  void GetPxlDigiData(int Row, int Rolumn, int& macroColumn, int& columnInMP, int& row) const { assert(0); }

  ClassDef(SbtSingleSideDetectorElem, 1);  // Implementation of SingleSideDetectorElem
};

#endif
