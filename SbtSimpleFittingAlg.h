#ifndef SBTSIMPLEFITTINGALG_HH
#define SBTSIMPLEFITTINGALG_HH

#include <utility>
#include <TMatrixD.h>
#include <TVectorD.h>
#include "SbtFittingAlg.h"

class SbtSimpleFittingAlg : public SbtFittingAlg {
 public:
  SbtSimpleFittingAlg();
  bool fitTrack(SbtTrack&);
  ~SbtSimpleFittingAlg() {;}

 protected:
  int _nSpOnTrk;

  ClassDef(SbtSimpleFittingAlg, 1);
};

#endif
