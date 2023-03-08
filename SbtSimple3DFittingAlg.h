#ifndef SBTSIMPLE3DFITTINGALG_HH
#define SBTSIMPLE3DFITTINGALG_HH

#include <TMatrixD.h>
#include <TVectorD.h>
#include "SbtFittingAlg.h"

class SbtTrack;

class SbtSimple3DFittingAlg : public SbtFittingAlg {
 public:
  SbtSimple3DFittingAlg();
  bool fitTrack(SbtTrack&);
  ~SbtSimple3DFittingAlg();

 protected:
  int _nSpOnTrk;

  int FitStatus(TVectorD LambdaX, TVectorD LambdaY, TMatrixD CovX,
                TMatrixD CovY);

  void PrintMeasMatrix(TMatrixD aFx, TMatrixD aFy, TVectorD aMx, TVectorD aMy,
                       TMatrixD aVx, TMatrixD aVy);
  void PrintFitMatrix(TVectorD aLambdaX, TVectorD aLambdaY, TMatrixD aCovX,
                      TMatrixD aCovY);
  
  ClassDef(SbtSimple3DFittingAlg, 1);
};

#endif
