#include <cassert>
#include <iostream>

#include "SbtDetectorElem.h"
#include "SbtDetectorType.h"
#include "SbtHit.h"
#include "SbtSimpleFittingAlg.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"

ClassImp(SbtSimpleFittingAlg);

SbtSimpleFittingAlg::SbtSimpleFittingAlg() : 
  _nSpOnTrk(0) {
  if (getDebugLevel() > 0) {
    std::cout << "SbtSimpleFittingAlg:  DebugLevel= " << getDebugLevel() << std::endl;
  }
  _algName = "Simple";
}

bool SbtSimpleFittingAlg::fitTrack(SbtTrack& candidateTrack) {
  _nSpOnTrk = 0;
  std::vector<SbtSpacePoint*> SPCandList = candidateTrack.GetSpacePointList();

  // define here the number of space points from tracking detectors
  for (auto SP : SPCandList) {
    if (SP->GetDetectorElem()->GetTrackingID() >= 0) {
      _nSpOnTrk++;
    }
  }

  if (getDebugLevel()) {
    std::cout << "SbtSimpleFittingAlg: SP candidate List size = "
         << SPCandList.size() << std::endl;
    std::cout << "SbtSimpleFittingAlg: _nSpOnTrk = " << _nSpOnTrk << std::endl;
  }

  // implement simple Least Squares Method for 3D track
  // fit x, y coordinates independently

  // define std::vector and matrix of double
  // assume we fit tracks with at least 4 SpacePoints
  // requirement reduced to 2 space points: to be checked whether it makes sense

  std::vector<SbtSpacePoint*> SPList;
  if (candidateTrack.GetType() == SbtEnums::objectType::reconstructed) {
    SPList = SPCandList;
  }
  else if (candidateTrack.GetType() == SbtEnums::objectType::simulated) {
    for (auto SP : SPCandList) {
      if (SP->GetDetectorElem()->GetTrackingID() >= 0) {
        SPList.push_back(SP);
      }
    }
  }
  if (SPList.size() < 2) {
    std::cout << "SbtSimpleFittingAlg: ";
    std::cout << "Warning: simulated track has SP size = " << SPList.size()
          << " no track fit performed" << std::endl;
    candidateTrack.SetFitStatus(-1);
    return false;
  }

  if (getDebugLevel()) {
    std::cout << "Candidate Track Type = " << candidateTrack.GetType() << std::endl;
    std::cout << "SpacePoint size = " << SPList.size() << std::endl;
  }

  std::vector<double> x, y, z, xerr, yerr, zerr;

  for (auto aSpacePoint : SPList) {
    x.push_back(aSpacePoint->GetXPosition());
    y.push_back(aSpacePoint->GetYPosition());
    z.push_back(aSpacePoint->GetZPosition());
    xerr.push_back(aSpacePoint->GetXPositionErr());
    yerr.push_back(aSpacePoint->GetYPositionErr());
    zerr.push_back(aSpacePoint->GetZPositionErr());
  }

  TMatrixD CovX(2, 2);  // Covariance Matrix of LambdaX parameters
  TMatrixD CovY(2, 2);  // Covariance Matrix of LambdaY parameters

  auto fitResultsX = linearFit(z, x, zerr, xerr, CovX);
  auto fitResultsY = linearFit(z, y, zerr, yerr, CovY);

  if (fitResultsX.first == 0 && fitResultsY.first == 0) {
    // line equation x = Ax + Bx z
    // line equation y = Ay + By z

    candidateTrack.SetAx(fitResultsX.second[0]);
    candidateTrack.SetAy(fitResultsY.second[0]);
    candidateTrack.SetBx(fitResultsX.second[1]);
    candidateTrack.SetBy(fitResultsY.second[1]);
    candidateTrack.SetFitStatus(0);
    candidateTrack.SetXCovMatrix(CovX);
    candidateTrack.SetYCovMatrix(CovY);
    double chi2 = Chi2(candidateTrack);
    int ndof = 2 * (SPList.size() - 2);
    candidateTrack.SetChi2(chi2);
    candidateTrack.SetNdof(ndof);
    candidateTrack.Residual();

    return true;
  }
  return false;
}
