#ifndef SBT_STRIPLETSDETECTORELEM
#define SBT_STRIPLETSDETECTORELEM

#include <cassert>

#include "SbtDetectorElem.h"
#include "SbtEnums.h"

class TGeoManager;
class SbtDetectorType;

class SbtStripletsDetectorElem : public SbtDetectorElem {
 public:
  SbtStripletsDetectorElem(TGeoManager *geom, SbtDetectorType *detType,
                           int detElemId, int detTrackId, double xPos,
                           double yPos, double zPos, double Phi, double Theta,
                           double Psi, double orientation_U,
                           double orientation_V);

  ~SbtStripletsDetectorElem() {;}

  // these are overloaded for Striplets, since the rotation to pseudo-local
  // coordinates must be made
  void LocalToMaster(double *local, double *master) const;
  void MasterToLocal(double *master, double *local) const;

  // the active area for striplets is more complex, hence it
  // needs its own function

  bool InActiveArea(TVector3 point) const;

  // define a function that given the electronic address
  // return the local coordinate address of the striplet
  void ChannelToLocal(int chip, int channel, char side, SbtEnums::view coordSide) const;

  int GetChannelNumber(int chip, int set, int strip) const;
  void GetStripDigiData(int channel, int &chip, int &set, int &strip) const;
  void GetPxlDigiData(int Row, int Rolumn, int &macroColumn, int &columnInMP, int &row) const { assert(0); }

  ClassDef(SbtStripletsDetectorElem, 1);  // Implementation of StripletsDetectorElem
};

#endif
