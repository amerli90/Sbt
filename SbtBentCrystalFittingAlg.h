#ifndef SBTBENTCRYSTALFITTINGALG_HH
#define SBTBENTCRYSTALFITTINGALG_HH

#include <TMatrixD.h>
#include <TVectorD.h>
#include "SbtFittingAlg.h"

#include <yaml-cpp/yaml.h>

class TF1;

class SbtBentCrystalFittingAlg : public SbtFittingAlg {
 public:
  SbtBentCrystalFittingAlg();
  SbtBentCrystalFittingAlg(const YAML::Node& config);
  bool fitTrack(SbtTrack&);
  ~SbtBentCrystalFittingAlg() {;}

  std::pair<int,TF1*> bentCrystalFit(std::vector<double> x, std::vector<double> y, 
                     std::vector<double> xerr,  std::vector<double> yerr,
                     TMatrixD& Cov);

 protected:
  int _nSpOnTrk;
  double _crystalPosition;

  ClassDef(SbtBentCrystalFittingAlg, 1);
};

#endif
