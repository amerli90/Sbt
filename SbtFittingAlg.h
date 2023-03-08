#ifndef SBTFITTINGALG_HH
#define SBTFITTINGALG_HH

#include <string>
#include <vector>
#include <utility>

#include <TMatrixD.h>

#include "SbtTrack.h"

class TF1;

class SbtFittingAlg {
public:
  SbtFittingAlg();
  virtual bool fitTrack(SbtTrack&) = 0;
  virtual ~SbtFittingAlg() {;}
  void setDebugLevel(int debugLevel) { _DebugLevel = debugLevel; }
  int getDebugLevel() const { return _DebugLevel; }
  std::string getAlgName() const { return _algName; };

  static std::pair<int,std::vector<double>> linearFit(std::vector<double> x, std::vector<double> y, 
                       std::vector<double> xerr,  std::vector<double> yerr,
                       TMatrixD& Cov);
  static std::pair<int,TF1*> linearFitv2(std::vector<double> x, std::vector<double> y, 
                         std::vector<double> xerr,  std::vector<double> yerr,
                         TMatrixD& Cov);
  static std::pair<int,TF1*> generalFit(TF1* func,
                        std::vector<double> x, std::vector<double> y, 
                        std::vector<double> xerr,  std::vector<double> yerr,
                        TMatrixD& Cov);

 protected:
  int _DebugLevel;
  std::string _algName;

  virtual double Chi2(const SbtTrack& track);

  ClassDef(SbtFittingAlg, 0);
};

#endif
