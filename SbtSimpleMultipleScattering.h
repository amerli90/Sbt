#ifndef SBT_SIMPLEMULTIPLESCATTERING
#define SBT_SIMPLEMULTIPLESCATTERING

#include "SbtMultipleScattering.h"

class SbtSimpleMultipleScattering : public SbtMultipleScattering {
 public:
  SbtSimpleMultipleScattering(SbtGenAlg* genAlg);
  virtual ~SbtSimpleMultipleScattering() {;}

  void MultipleScattering(TVector3& point, TVector3& direction, double RadLen);

 protected:
  ClassDef(SbtSimpleMultipleScattering, 1);
};
#endif
