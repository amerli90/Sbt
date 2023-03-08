#ifndef SBT_SIMPLECLUSTERINGALG
#define SBT_SIMPLECLUSTERINGALG

#include "SbtClusteringAlg.h"

class SbtSimpleClusteringAlg : public SbtClusteringAlg {
 public:
  SbtSimpleClusteringAlg();
  ~SbtSimpleClusteringAlg();

  int Clusterize(std::vector<SbtDigi*> digis, std::vector<SbtCluster>& clusters);

 protected:
  ClassDef(SbtSimpleClusteringAlg, 1);
};
#endif
