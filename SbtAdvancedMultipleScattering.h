#ifndef SBT_ADVANCEDMULTIPLESCATTERING
#define SBT_ADVANCEDMULTIPLESCATTERING

#include "SbtMultipleScattering.h"

class SbtAdvancedMultipleScattering : public SbtMultipleScattering {
 public:
  SbtAdvancedMultipleScattering(SbtGenAlg* genAlg);
  virtual ~SbtAdvancedMultipleScattering() {;}

  void MultipleScattering(TVector3& point, TVector3& direction, double RadLen);

 protected:
  ClassDef(SbtAdvancedMultipleScattering, 1);
};
#endif
