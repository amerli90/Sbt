#ifndef SBT_MAKECLUSTERS
#define SBT_MAKECLUSTERS

#include <vector>
#include <string>

#include "SbtCluster.h"
#include "SbtDef.h"
#include "SbtDigi.h"

class SbtEvent;
class SbtClusteringAlg;

class SbtMakeClusters {
 public:
  SbtMakeClusters(std::string algorithm, std::string pxlClusteringOpt);
  virtual ~SbtMakeClusters();
  void setDebugLevel(int debugLevel) { _DebugLevel = debugLevel; }
  inline int getDebugLevel() { return _DebugLevel; }

  void makeClusters(SbtEvent* event);

 protected:
  int _DebugLevel;

  SbtClusteringAlg* _stripClusterAlg;
  SbtClusteringAlg* _pxlClusterAlg;

  // strip digis order by layer/side
  std::vector<SbtDigi*> _orderedStripDigis[maxNTelescopeDetector][2];
  // pixel digis order by layer
  std::vector<SbtDigi*> _orderedPxlDigis[maxNDutDetector];

  void makeStripClusters(SbtEvent* event);
  void makePxlClusters(SbtEvent* event);

  // this method reorder the digis in the event
  // the orderedDigis array is filled accordingly
  void orderStripDigis(std::vector<SbtDigi>& eventStripDigiList);
  void orderPxlDigis(std::vector<SbtDigi>& eventStripDigiList);

  // sort a list of digis
  void SortStripDigis(std::vector<SbtDigi*>& stripDigiList);
  void SortPxlDigis(std::vector<SbtDigi*>& pxlDigiList);

  // append clusters to overall list
  void AppendClusters(std::vector<SbtCluster>& clusterList);

  static bool stripLt(SbtDigi* d1, SbtDigi* d2);
  static bool pxlLt(SbtDigi* d1, SbtDigi* d2);

  ClassDef(SbtMakeClusters, 1);
};

#endif
