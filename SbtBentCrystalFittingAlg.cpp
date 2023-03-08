
#include <cassert>
#include <iostream>

#include <TF1.h>

#include "SbtDetectorElem.h"
#include "SbtDetectorType.h"
#include "SbtHit.h"
#include "SbtBentCrystalFittingAlg.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"

ClassImp(SbtBentCrystalFittingAlg);

SbtBentCrystalFittingAlg::SbtBentCrystalFittingAlg() : 
  _nSpOnTrk(0),
  _crystalPosition(0) {
  if (getDebugLevel() > 0) {
    std::cout << "SbtBentCrystalFittingAlg:  DebugLevel= " << getDebugLevel() << std::endl;
  }
  _algName = "BentCrystal";
}

SbtBentCrystalFittingAlg::SbtBentCrystalFittingAlg(const YAML::Node& config) : 
  _nSpOnTrk(0),
  _crystalPosition(0) {
  if (getDebugLevel() > 0) {
    std::cout << "SbtBentCrystalFittingAlg:  DebugLevel= " << getDebugLevel() << std::endl;
  }
  _algName = "BentCrystal";
  if (config["crystalPosition"]) _crystalPosition = config["crystalPosition"].as<double>();
}

bool SbtBentCrystalFittingAlg::fitTrack(SbtTrack& candidateTrack) {
  _nSpOnTrk = 0;
  std::vector<SbtSpacePoint*> SPCandList = candidateTrack.GetSpacePointList();

  // define here the number of space points from tracking detectors
  for (auto SP : SPCandList) {
    if (SP->GetDetectorElem()->GetTrackingID() >= 0) {
      _nSpOnTrk++;
    }
  }

  if (getDebugLevel()) {
    std::cout << "SbtBentCrystalFittingAlg: SP candidate List size = " << SPCandList.size() << std::endl;
    std::cout << "SbtBentCrystalFittingAlg: _nSpOnTrk = " << _nSpOnTrk << std::endl;
  }

  // implement simple Least Squares Method for 3D track
  // fit x, y coordinates independently

  // define std::vector and matrix of double
  // assume we fit tracks with at least 4 SpacePoints
  // requirement reduced to 2 space points: to be checked whether it makes sense

  std::vector<SbtSpacePoint*> SPList;
  SPList.clear();
  if (candidateTrack.GetType() == SbtEnums::objectType::reconstructed) {
    SPList = SPCandList;
  }
  else if (candidateTrack.GetType() == SbtEnums::objectType::simulated) {
    std::vector<SbtSpacePoint*>::iterator SPIter;
    for (SPIter = SPCandList.begin(); SPIter != SPCandList.end(); SPIter++) {
      if ((*SPIter)->GetDetectorElem()->GetTrackingID() >= 0) {
        SPList.push_back(*SPIter);

        if (getDebugLevel()) {
          std::cout << "SP (x, y, z) =  "
               << "(" << (*SPIter)->GetXPosition() << ", "
               << (*SPIter)->GetYPosition() << ", " << (*SPIter)->GetZPosition()
               << ")" << std::endl;
          std::cout << "SP (Dx, Dy, Dz) =  "
               << "(" << (*SPIter)->GetXPositionErr() << ", "
               << (*SPIter)->GetYPositionErr() << ", "
               << (*SPIter)->GetZPositionErr() << ")" << std::endl;
        }
      }
    }
  }
  if (SPList.size() < 2) {
    std::cout << "SbtBentCrystalFittingAlg: ";
    std::cout << "Warning: simulated track has SP size = " << SPList.size() << " no track fit performed" << std::endl;
    candidateTrack.SetFitStatus(0);
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

  TMatrixD CovX;  // Covariance Matrix of LambdaX parameters
  TMatrixD CovY;  // Covariance Matrix of LambdaY parameters
  int ndof = 0;

  if (candidateTrack.GetShape() == SbtEnums::trackShape::channelledTrack) {
    CovX.ResizeTo(4, 4);
    CovY.ResizeTo(4, 4);
    auto bentFitResultX = bentCrystalFit(z, x, zerr, xerr, CovX);
    auto bentFitResultY = bentCrystalFit(z, y, zerr, yerr, CovY);
    if (bentFitResultX.first == 0 && bentFitResultY.first == 0) {
      candidateTrack.SetTrackFunctionX(bentFitResultX.second);
      candidateTrack.SetTrackFunctionY(bentFitResultY.second);
      ndof = 2 * SPList.size() - 8;
    }
    else {
      return false;
    }
  }
  else if (candidateTrack.GetShape() == SbtEnums::trackShape::channelledTrackX ) {
    CovX.ResizeTo(4, 4);
    CovY.ResizeTo(2, 2);
    auto bentFitResult = bentCrystalFit(z, x, zerr, xerr, CovX);
    auto linearFitResult = linearFit(z, y, zerr, yerr, CovY);
    if (linearFitResult.first == 0 && bentFitResult.first == 0) {
      candidateTrack.SetTrackFunctionX(bentFitResult.second);
      candidateTrack.SetAy(linearFitResult.second[0]);
      candidateTrack.SetBy(linearFitResult.second[1]);
      ndof = 2 * SPList.size() - 6;
    }
    else {
      return false;
    }
  }
  else if (candidateTrack.GetShape() == SbtEnums::trackShape::channelledTrackY) {
    CovX.ResizeTo(2, 2);
    CovY.ResizeTo(4, 4);
    auto linearFitResult = linearFit(z, x, zerr, xerr, CovX);
    auto bentFitResult = bentCrystalFit(z, y, zerr, yerr, CovY);
    if (linearFitResult.first == 0 && bentFitResult.first == 0) {
      candidateTrack.SetTrackFunctionY(bentFitResult.second);
      candidateTrack.SetAx(linearFitResult.second[0]);
      candidateTrack.SetBx(linearFitResult.second[1]);
      candidateTrack.SetTrackShape(SbtEnums::trackShape::channelledTrackY);
      ndof = 2 * SPList.size() - 6;
    }
    else {
      return false;
    }
  }
  else if (candidateTrack.GetShape() == SbtEnums::trackShape::longTrack) {
    CovX.ResizeTo(2, 2);
    CovY.ResizeTo(2, 2);
    auto linearFitResultY = linearFit(z, y, zerr, yerr, CovY);
    if (linearFitResultY.first == 0) {
      candidateTrack.SetAy(linearFitResultY.second[0]);
      candidateTrack.SetBy(linearFitResultY.second[1]);
    }
    else {
      return false;
    }
    auto linearFitResultX = linearFit(z, x, zerr, xerr, CovX);
    if (linearFitResultX.first == 0) {
      candidateTrack.SetAx(linearFitResultX.second[0]);
      candidateTrack.SetBx(linearFitResultX.second[1]);
    }
    else {
      return false;
    }
    ndof = 2 * SPList.size() - 4;
  }
  else if (candidateTrack.GetShape() == SbtEnums::trackShape::downStreamTrack ||
           candidateTrack.GetShape() == SbtEnums::trackShape::upStreamTrack) {
    double Bx = (x[0] - x[1]) / (z[0] - z[1]);
    double Ax = x[0] - Bx * z[0];
    double By = (y[0] - y[1]) / (z[0] - z[1]);
    double Ay = y[0] - By * z[0];

    candidateTrack.SetAx(Ax);
    candidateTrack.SetAy(Ay);
    candidateTrack.SetBx(Bx);
    candidateTrack.SetBy(By);
    ndof = 0;
  }
  else {
    std::cout << "Track type " << candidateTrack.GetShape() << " not recognized! No fit performed." << std::endl;
    return false;
  }
  if (ndof < 0) ndof = 0;
  candidateTrack.SetFitStatus(0);
  candidateTrack.SetXCovMatrix(CovX);
  candidateTrack.SetYCovMatrix(CovY);
  double chi2 = Chi2(candidateTrack);
  candidateTrack.SetChi2(chi2);
  candidateTrack.SetNdof(ndof);
  candidateTrack.Residual();

  return true;
}

std::pair<int,TF1*> SbtBentCrystalFittingAlg::bentCrystalFit(std::vector<double> x, std::vector<double> y, 
                                             std::vector<double> xerr,  std::vector<double> yerr,
                                             TMatrixD& Cov) {
  TF1* bentTrack = new TF1("bentTrack",
  "[&](double *x, double *p){ if (x[0] < p[3]) {return p[0]+p[1]*x[0];}"
  "else { double a = p[0]+p[1]*p[3]; return a+p[2]*(x[0]-p[3]); } }", 1, 100, 4);
  bentTrack->SetParNames("Intercept", "Slope", "BentSlope", "BendingPoint");
  bentTrack->SetParLimits(3, _crystalPosition * 0.90, _crystalPosition * 1.1);
  bentTrack->SetParameter(3,_crystalPosition);
  return generalFit(bentTrack, x, y, xerr, yerr, Cov);
}
