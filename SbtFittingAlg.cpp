#include "SbtFittingAlg.h"

#include "SbtSpacePoint.h"
#include "SbtDetectorElem.h"

#include <TF1.h>
#include <TVectorD.h>
#include <TGraphErrors.h>
#include <TFitResultPtr.h>
#include <TFitResult.h>

#include <iostream>
#include <cfloat>

ClassImp(SbtFittingAlg);

SbtFittingAlg::SbtFittingAlg() : _DebugLevel(0) {
  std::cout << "SbtFittingAlg:  DebugLevel= " << _DebugLevel << "\n";
}

std::pair<int,std::vector<double>> SbtFittingAlg::linearFit(std::vector<double> x, std::vector<double> y, 
                             std::vector<double> xerr,  std::vector<double> yerr,
                             TMatrixD& Cov) {
  std::vector<double> parameters;
  const int npoints = x.size();
  if (y.size() != npoints || xerr.size() != npoints || yerr.size() != npoints) {
    std::cout << "ERROR in SbtFittingAlg::linearFit: size of the input arrays are not consistent!" << std::endl;
    return {-1, parameters};
  }
  // Matrix of derivatives of line functional form wrt parameters
  TMatrixD F(npoints, 2);
  // Matrix of derivatives of line functional form wrt parameters
  TVectorD M(npoints);  // vector of x SpacePoints measurements
  TMatrixD V(npoints, npoints);  // vector of x SpacePoints measurements Errors

  for (int i = 0; i < npoints; i++) {
    // z measurements of Trk SpacePoints
    F[i][0] = 1;
    F[i][1] = x[i];

    // x measurements of Trk SpacePoints
    M[i] = y[i];

    // x measurement Errors of Trk SpacePoints
    V[i][i] = yerr[i]*yerr[i];
  }

  // Transpose matrices
  TMatrixD FT = F;
  FT.T();  // define FT Transpose Matrix

  // Invert error matrices
  TMatrixD VInv = V;
  VInv.Invert();  // define inverse x Error Matrix

  // Matrix product
  TMatrixD Product = FT * VInv * F;

  // Calculate covariance matrices
  Cov = Product.Invert();  // Evaluate Covariance Matrix

  auto Lambda = Cov * FT * VInv * M;

  parameters.push_back(Lambda[0]);
  parameters.push_back(Lambda[1]);

  return {0, parameters};
}

std::pair<int,TF1*> SbtFittingAlg::linearFitv2(std::vector<double> x, std::vector<double> y, 
                               std::vector<double> xerr,  std::vector<double> yerr,
                               TMatrixD& Cov) {
  TF1* linearTrack = new TF1("linearFit","[0]+x*[1]", 0, 100);
  linearTrack->SetParNames("Intercept", "Slope");
  return generalFit(linearTrack, x, y, xerr, yerr, Cov);
}

std::pair<int,TF1*> SbtFittingAlg::generalFit(TF1* func,
                              std::vector<double> x, std::vector<double> y, 
                              std::vector<double> xerr,  std::vector<double> yerr,
                              TMatrixD& Cov) {
  const int npoints = x.size();
  if (y.size() != npoints || xerr.size() != npoints || yerr.size() != npoints) {
    std::cout << "ERROR in SbtFittingAlg::linearFit: size of the input arrays are not consistent!" << std::endl;
    return {-1, nullptr};
  }
  double minx = DBL_MAX;
  double maxx = -DBL_MAX;
  for (auto xp : x) {
    if (xp < minx) minx = xp;
    if (xp > maxx) maxx = xp;
  }
  double rangex = maxx - minx;
  func->SetRange(minx - 0.5 * rangex, maxx + 0.5 * rangex);
  TGraphErrors graph(npoints, x.data(), y.data(), xerr.data(), yerr.data());

  TFitResultPtr fitResults = graph.Fit(func, "NSQ");
  if (int(fitResults) == 0) {
    Cov = fitResults->GetCovarianceMatrix();
  }
  else {
    return {-1, nullptr};
  }
  return {int(fitResults), func};
}

// Here is a modified version of SbtTrack::Chi2(), to take in account rotations
// of planes and/or strips
double SbtFittingAlg::Chi2(const SbtTrack& track) {
  double chi2 = 0.0;
  for (auto Sp : track.GetSpacePointList()) {
    double master[3] = {Sp->GetXPosition(), Sp->GetYPosition(),
                        Sp->GetZPosition()};

    double masterfit[3] = {track.GetTrackFunctionX()->Eval(Sp->GetZPosition()),
                           track.GetTrackFunctionY()->Eval(Sp->GetZPosition()),
                           Sp->GetZPosition()};
    double local[3] = {0, 0, 0};
    double localfit[3] = {0, 0, 0};

    Sp->GetDetectorElem()->MasterToLocal(master, local);
    Sp->GetDetectorElem()->MasterToLocal(masterfit, localfit);

    chi2 += (local[0] - localfit[0]) * (local[0] - localfit[0]) /
            (Sp->GetXPositionErr() * Sp->GetXPositionErr());
    chi2 += (local[1] - localfit[1]) * (local[1] - localfit[1]) /
            (Sp->GetYPositionErr() * Sp->GetYPositionErr());
  }
  return chi2;
}