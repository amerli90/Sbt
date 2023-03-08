#ifndef SBT_COSMICGENALG
#define SBT_COSMICGENALG

#include "SbtSimpleGenAlg.h"

class SbtCosmicGenAlg : public SbtSimpleGenAlg {
 public:
  SbtCosmicGenAlg(TRandom* rN);
  virtual ~SbtCosmicGenAlg() {;}

 protected:
  void GeneratInitialTrack(double* startingPoint, double* startingDirection);

  ClassDef(SbtCosmicGenAlg, 1);
};
#endif
