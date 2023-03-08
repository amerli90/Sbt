#include <iostream>

#include <TMath.h>

#include "SbtCluster.h"
#include "SbtConfig.h"
#include "SbtDetectorElem.h"
#include "SbtGenAlg.h"
#include "SbtHit.h"
#include "SbtSpacePoint.h"

ClassImp(SbtGenAlg);

SbtGenAlg::SbtGenAlg() :
  _debugLevel(0),
  _angular_div(-1),
  _angular_mean_x(0.),
  _angular_mean_y(0.),
  _beam_sigma_x(-1),
  _beam_sigma_y(-1),
  _particle_id(-1),
  _energy(-1),
  _detID(-1),
  _d_beam_tele(0),
  _stripOccupancy(0),
  _pxlOccupancy(0),
  _avgNoiseAdc(0),
  _theConfigurator(nullptr),
  _theGeoManager(nullptr),
  _aRandomFnc(nullptr),
  _lastNode(nullptr),
  _topVolumePath("/TOP_1"),
  _multipleScattering(nullptr) {
  std::cout << "SbtGenAlg:  DebugLevel= " << _debugLevel << std::endl;
}

SbtGenAlg::SbtGenAlg(TRandom* rnd) :
  _debugLevel(0),
  _angular_div(-1),
  _angular_mean_x(0.),
  _angular_mean_y(0.),
  _beam_sigma_x(-1),
  _beam_sigma_y(-1),
  _particle_id(-1),
  _energy(-1),
  _d_beam_tele(0),
  _stripOccupancy(0),
  _pxlOccupancy(0),
  _avgNoiseAdc(0),
  _theConfigurator(nullptr),
  _theGeoManager(nullptr),
  _aRandomFnc(rnd),
  _topVolumePath("/TOP_1"),
  _multipleScattering(nullptr) {
  std::cout << "SbtGenAlg:  DebugLevel= " << _debugLevel << std::endl;
}

// a method to retrieve a detector elem pointer given its ID
SbtDetectorElem* SbtGenAlg::GetDetectorElem(int ID) {
  return _theConfigurator->getDetectorElemFromID(ID);
}

void SbtGenAlg::SetConfigurator(SbtConfig* aConfigurator) {
  _theConfigurator = aConfigurator;
  _theGeoManager = aConfigurator->getGeoManager();
}

int SbtGenAlg::GetDetectorIDfromZCoord(double zCoord) {
  for (auto det : _theConfigurator->getDetectorElemList()) {
    if (fabs(det->GetZPos() - zCoord) < det->GetDetectorType()->GetZ_HalfDim()) {
      return det->GetID();
    }
  }
  return -1;
}

void SbtGenAlg::SetTrackRealDirection(SbtTrack& track, bool last) const {
  double Bx = _lastDirection.X() / _lastDirection.Z();
  double By = _lastDirection.Y() / _lastDirection.Z();

  if (last) {
    double Ax = _lastPoint.X() - Bx * _lastPoint.Z();
    double Ay = _lastPoint.Y() - By * _lastPoint.Z();

    track.SetAx(Ax);
    track.SetAy(Ay);
    track.SetBx(Bx);
    track.SetBy(By);
  }
  else {
    if (getDebugLevel() > 2) {
      std::cout << "Adding sim node " << track.GetNumberOfSimNodes() << " with slopes " << Bx << ", " << By << std::endl;
    }
    track.AddTrackParmsAtNode(Bx, By, _lastPoint.X(), _lastPoint.Y(), _lastPoint.Z());
  }
}

double SbtGenAlg::getRandomSignal(const SbtDetectorType* detType) const {
  if (detType->getElossSigma() == 0) {
    return detType->getElossMPV() * detType->getadcGain();
  }
  double eloss = 0;
  switch (detType->getElossDistr()) {
    case SbtEnums::pdf::uniform:
    eloss = _aRandomFnc->Uniform(detType->getElossMPV() - detType->getElossSigma() / 2, detType->getElossMPV() + detType->getElossSigma() / 2);
    break;

    case SbtEnums::pdf::gaus:
    eloss = _aRandomFnc->Gaus(detType->getElossMPV(), detType->getElossSigma());
    break;

    case SbtEnums::pdf::landau:
    eloss = _aRandomFnc->Landau(detType->getElossMPV(), detType->getElossSigma());
    break;
  }
  return eloss * detType->getadcGain();
}
