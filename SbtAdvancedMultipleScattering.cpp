#include <iostream>

#include "SbtAdvancedMultipleScattering.h"

ClassImp(SbtAdvancedMultipleScattering)

SbtAdvancedMultipleScattering::SbtAdvancedMultipleScattering(SbtGenAlg* genAlg) :
  SbtMultipleScattering(genAlg) {
  if (getDebugLevel() > 0) {
    std::cout << "SbtAdvancedMultipleScattering:  DebugLevel= " << getDebugLevel() << std::endl;
  }
}

void SbtAdvancedMultipleScattering::MultipleScattering(TVector3& point, TVector3& direction, double RadLen) {
  // this method modifies point and direction using multiple scattering
  double dist = (point - getLastPoint()).Mag();
  if (getDebugLevel()) {
    std::cout << "MS on!" << std::endl;
    std::cout << "Just checking that direction and last point are consistent: " << std::endl;
    (dist * direction + getLastPoint()).Print();
    point.Print();
    if (RadLen < 10 && dist > 10) {
      std::cout << "MS: warning!! Unusually large multiple scattering!" << std::endl;
    }
  }

  TVector3 direction_p = -direction.Orthogonal();
  direction_p.SetMag(1);
  TVector3 direction_q = direction.Cross(direction_p);
  direction_q.SetMag(1);
  
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
      if (getDebugLevel() > 0) std::cout << "Sigma is zero! No MS effect will be simulated!" << std::endl;
    } 
    else {
      if (getDebugLevel()) {
        std::cout << "The original position is " << std::endl;
        point.Print();
        std::cout << "The original direction is " << std::endl;
        direction.Print();
        std::cout << "The 'p' direction is: " << std::endl;
        direction_p.Print();
        std::cout << "The 'q' direction is: " << std::endl;
        direction_q.Print();
      }
      
      double thetaP1 = getRandomGen()->Gaus(0., 1.) * Sigma;
      double thetaP2 = getRandomGen()->Gaus(0., 1.) * Sigma;
      double thetaQ1 = getRandomGen()->Gaus(0., 1.) * Sigma;
      double thetaQ2 = getRandomGen()->Gaus(0., 1.) * Sigma;
      static const double norm = 1. / TMath::Sqrt(3.);

      TVector3 CorrP = direction_p * dist * 0.5 * (thetaP2 + norm * thetaP1);
      TVector3 CorrQ = direction_q * dist * 0.5 * (thetaQ2 + norm * thetaQ1);

      point += CorrQ;
      point += CorrP;

      direction.Rotate(-thetaQ2, direction_p);
      if (getDebugLevel()) {
        std::cout << "After rotation around p" << std::endl;
        direction.Print();
      }
      direction.Rotate(thetaP2, direction_q);

      if (getDebugLevel()) {
        auto check_new_point = getLastPoint() + direction * dist;
        std::cout << "Checking correlation between angular and spatial shift due to MS." << std::endl;
        std::cout << "Recalculate new point using direction and distance" << std::endl;
        check_new_point.Print();
        std::cout << "Difference compared to new point" << std::endl;
        (check_new_point - point).Print();

        std::cout << "The new p position is " << std::scientific << point.Dot(direction_p) << ", after a correction of " << CorrP.Mag() << std::defaultfloat << std::endl;
        std::cout << "The new q position is " << std::scientific << point.Dot(direction_q) << ", after a correction of " << CorrQ.Mag() << std::defaultfloat << std::endl;
        std::cout << "The new position is " << std::endl;
        point.Print();
        std::cout << "The new direction is" << std::endl;
        direction.Print();

        std::cout << "Spatial correction is" << std::endl;
        (CorrQ + CorrP).Print();
      }

      // See: M. Tanabashi et al. (Particle Data Group), Phys. Rev. D 98, 030001 (2018), Eqs. 33.22 and 33.23
      // https://journals.aps.org/prd/abstract/10.1103/PhysRevD.98.030001
    }
  }
}