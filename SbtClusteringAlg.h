#ifndef SBT_CLUSTERINGALG
#define SBT_CLUSTERINGALG

#include <vector>
#include "SbtCluster.h"

class SbtClusteringAlg {
 public:
  SbtClusteringAlg();
  virtual ~SbtClusteringAlg() {;}
  void setDebugLevel(int debugLevel) { _DebugLevel = debugLevel; }
  int getDebugLevel() const { return _DebugLevel; }

  virtual int Clusterize(std::vector<SbtDigi*> digis, std::vector<SbtCluster>& clusterList) = 0;

 protected:
  int _DebugLevel;

  ClassDef(SbtClusteringAlg, 0);
};

#endif
