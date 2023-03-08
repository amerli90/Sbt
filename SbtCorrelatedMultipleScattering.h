#ifndef SBT_CORRELATEDMULTIPLESCATTERING
#define SBT_CORRELATEDMULTIPLESCATTERING

#include "SbtMultipleScattering.h"

class SbtCorrelatedMultipleScattering : public SbtMultipleScattering {
 public:
  SbtCorrelatedMultipleScattering(SbtGenAlg* genAlg);
  virtual ~SbtCorrelatedMultipleScattering() {;}

  void MultipleScattering(TVector3& point, TVector3& direction, double RadLen);

 protected:
  ClassDef(SbtCorrelatedMultipleScattering, 1);
};
#endif
