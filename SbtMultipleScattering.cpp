#include "SbtMultipleScattering.h"

ClassImp(SbtMultipleScattering);

SbtMultipleScattering::SbtMultipleScattering(SbtGenAlg* _genAlg) :
  _debugLevel(0),
  _genAlg(_genAlg) {
}

double SbtMultipleScattering::MultipleScatteringAngleSigma(double energy, double thickness, double radLen) {
  return 13.6 / energy * TMath::Sqrt(thickness / radLen) * (1. + 0.038 * TMath::Log(thickness / radLen));
  // See: M. Tanabashi et al. (Particle Data Group), Phys. Rev. D 98, 030001 (2018), Eq. 33.15 (page 451)
  // https://journals.aps.org/prd/abstract/10.1103/PhysRevD.98.030001
}