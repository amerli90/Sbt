#include <iostream>

#include "SbtSimpleMultipleScattering.h"

ClassImp(SbtSimpleMultipleScattering);

SbtSimpleMultipleScattering::SbtSimpleMultipleScattering(SbtGenAlg* genAlg) :
  SbtMultipleScattering(genAlg) {
  if (getDebugLevel() > 0) {
    std::cout << "SbtSimpleMultipleScattering:  DebugLevel= " << getDebugLevel() << std::endl;
  }
}

void SbtSimpleMultipleScattering::MultipleScattering(TVector3& point, TVector3& direction, double RadLen) {
  // this method modifies point and direction using multiple scattering

  if (getDebugLevel()) {
    std::cout << "MS on!" << std::endl;
  }
  double* norm2Surf = getGeoManager()->FindNormal();
  TVector3 norm2SurfVect(norm2Surf);
  TVector3 alongSurfX = norm2SurfVect.Orthogonal();
  alongSurfX.SetMag(1.);
  TVector3 alongSurfY = alongSurfX.Cross(norm2SurfVect);
  double dist = (point - getLastPoint()).Mag();
  if (getDebugLevel()) {
    std::cout << "Thickness: " << dist << std::endl;
    std::cout << "Energy: " << getEnergy() << std::endl;
    std::cout << "RadLen: " << RadLen << std::endl;
  }
  if (RadLen == 0) {
    if (getDebugLevel() > 3) std::cout << "Radiation lenght is 0! No MS effect will be simualted" << std::endl;
  }
  else {
    double Sigma = MultipleScatteringAngleSigma(getEnergy(), dist, RadLen);

    if (getDebugLevel() > 0) std::cout << "Sigma: " << Sigma << std::endl;

    if (Sigma == 0.) {
      if (getDebugLevel() > 1) std::cout << "Sigma is zero! No MS effect will be simulated!" << std::endl;
    } 
    else {
      if (getDebugLevel() > 10) {
        getLastPoint().Print();
        std::cout << "initial angle " << (point-getLastPoint()).Angle(direction) << std::endl;
      }

      static const double norm = 1.0 / TMath::Sqrt(3.);
      double thetaX = getRandomGen()->Gaus(0., 1) * Sigma;
      double thetaY = getRandomGen()->Gaus(0., 1) * Sigma;

      TVector3 CorrX = alongSurfX * TMath::Tan(thetaX) * dist * norm;
      TVector3 CorrY = alongSurfY * TMath::Tan(thetaY) * dist * norm;
      point += CorrX;
      point += CorrY;

      direction.Rotate(-thetaX, alongSurfY);
      direction.Rotate(-thetaY, alongSurfX);

      if (getDebugLevel() > 10) {
        std::cout << "thetaX " << thetaX << std::endl;
        std::cout << "thetaY " << thetaY << std::endl;
        std::cout << "final angle " << (point-getLastPoint()).Angle(direction) << std::endl;
      }
    }
  }
}