#ifndef SBT_MULTIPLESCATTERING
#define SBT_MULTIPLESCATTERING

#include <TVector3.h>

#include "SbtGenAlg.h"

class SbtMultipleScattering {
 public:
  SbtMultipleScattering(SbtGenAlg* genAlg);
  virtual ~SbtMultipleScattering() {;}

  void setDebugLevel(int debugLevel) { _debugLevel = debugLevel; }
  int getDebugLevel() const { return _debugLevel; }

  virtual void MultipleScattering(TVector3& point, TVector3& direction, double RadLen) = 0;

 protected:
  int _debugLevel;

  SbtGenAlg* _genAlg;

  TGeoManager* getGeoManager() { return _genAlg->_theGeoManager; }
  TVector3& getLastPoint() { return _genAlg->_lastPoint; }
  TVector3& getLastDirection() { return _genAlg->_lastDirection; }
  TRandom* getRandomGen() { return _genAlg->_aRandomFnc; }
  double getEnergy() { return _genAlg->_energy; }

  // sigma of MS angle distribution
  static double MultipleScatteringAngleSigma(double energy, double thickness, double radLen);

  ClassDef(SbtMultipleScattering, 0);
};
#endif
