#include "SbtCosmicGenAlg.h"

ClassImp(SbtCosmicGenAlg);

SbtCosmicGenAlg::SbtCosmicGenAlg(TRandom* rN) :
  SbtSimpleGenAlg(rN) {
  if (getDebugLevel() > 0) {
    std::cout << "SbtCosmicGenAlg:  DebugLevel= " << getDebugLevel() << std::endl;
  }
}

void SbtCosmicGenAlg::GeneratInitialTrack(double* startingPoint, double* startingDirection) {
  // calculating the starting direction
  TVector3 xAxis(1., 0., 0.);
  //  TVector3 yAxis( 0., 1., 0. );
  TVector3 zAxis(0., 0., 1.);

  TVector3 dir = zAxis;
  TF1 cosmic_distr("Cosmic Ray Distribution", "3/2 *pow(cos(x),2) * sin(x)", 0, _angular_div);  // check
  dir.Rotate(cosmic_distr.GetRandom(), xAxis);
  dir.Rotate((_aRandomFnc->Rndm()) * 2 * TMath::Pi(), zAxis);
  dir.SetMag(1.);

  startingDirection[0] = dir[0];
  startingDirection[1] = dir[1];
  startingDirection[2] = dir[2];

  startingPoint[2] -= _d_beam_tele;
  // uniform beam profile
  startingPoint[0] = -_d_beam_tele * (startingDirection[0] / startingDirection[2]) + (_aRandomFnc->Rndm() - 0.5) * 2 * _beam_sigma_x;
  startingPoint[1] = -_d_beam_tele * (startingDirection[1] / startingDirection[2]) + (_aRandomFnc->Rndm() - 0.5) * 2 * _beam_sigma_y;
}
