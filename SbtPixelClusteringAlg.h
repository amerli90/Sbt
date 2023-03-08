#ifndef SBT_PIXELCLUSTERINGALG
#define SBT_PIXELCLUSTERINGALG

#include <vector>
#include <string>

#include "SbtClusteringAlg.h"
#include "SbtEnums.h"

class SbtPixelClusteringAlg : public SbtClusteringAlg {
 public:
  SbtPixelClusteringAlg(std::string pxlClusteringOpt);
  ~SbtPixelClusteringAlg() {;}

  int Clusterize(std::vector<SbtDigi*> digis, std::vector<SbtCluster>& clusters);

 protected:
  std::vector<SbtDigi*> FindNeighborDigis(std::vector<SbtDigi*> prevNeighborDigi, std::vector<SbtDigi*>& SelectedDigis);
  bool isAdjacentPixel(SbtDigi* NeighborDigi, SbtDigi* SelDigi, SbtEnums::PxlPosWrtSeed PxlPos);

  std::string _pxlClusteringOpt;

  ClassDef(SbtPixelClusteringAlg, 1);
};

#endif
