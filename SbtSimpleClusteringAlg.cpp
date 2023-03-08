#include <cstdlib>
#include <iostream>

#include "SbtSimpleClusteringAlg.h"
#include "SbtDetectorElem.h"
#include "SbtDetectorType.h"

#include "SbtDef.h"
#include "SbtDigi.h"

ClassImp(SbtSimpleClusteringAlg);

SbtSimpleClusteringAlg::SbtSimpleClusteringAlg() : SbtClusteringAlg() {
  std::cout << "SbtSimpleClusteringAlg:  DebugLevel= " << getDebugLevel()
            << std::endl;
}

SbtSimpleClusteringAlg::~SbtSimpleClusteringAlg() {}

int SbtSimpleClusteringAlg::Clusterize(std::vector<SbtDigi*> digis, std::vector<SbtCluster>& clusterList) {
  if (getDebugLevel() > 0) std::cout << "SbtSimpleClusteringAlg is ready to clusterize!\n";

  int iFrwd(0), iBkwd(0), iDigi(0);
  int nDigi = digis.size();
  int nclusters = 0;
  std::vector<SbtDigi*> selectedDigis;
  if (getDebugLevel() > 0) {
    std::cout << " SimpleClusteringAlg: start digi loop.\n";
  }
  for (iDigi = 0; iDigi < nDigi; iDigi++) {
    selectedDigis.clear();
    iFrwd = 0;
    iBkwd = 0;
    int maxChDist = maxChDistance;
    if (digis[iDigi]->GetDetectorElem()->GetDetectorType()->isFloatingStrip()) maxChDist *= 2;
    if (((digis[iDigi])->GetADC()) >=
        minAdcClusterSeed * ((digis[iDigi])->GetThr())) {
      if (getDebugLevel() > 0)
        std::cout << "A digi is good for a cluster seed!\n";

      selectedDigis.push_back(digis[iDigi]);
      iFrwd++;
      iBkwd++;
      // looking in the forward direction for a digi that:
      // 1) has a meaningful index
      // 2) has an over-threshold adc value
      // 3) is contiguos
      while (1 && ((iDigi + iFrwd) < nDigi)  // 1)
             && ((digis[iDigi + iFrwd])->GetADC() >=
                 minAdcClusterAddendum * ((digis[iDigi])->GetThr()))  // 2)
             && abs(digis[iDigi + iFrwd - 1]->GetChannelNumber() -
                    digis[iDigi + iFrwd]->GetChannelNumber()) <=
                    (iFrwd * maxChDist)  // 3)
      ) {
        if (getDebugLevel() > 0) {
          std::cout << "Adding a 'forward' digi...\t";
          std::cout << "Distance is: "
                    << abs(digis[iDigi]->GetChannelNumber() -
                           digis[iDigi + iFrwd]->GetChannelNumber())
                    << "\n";
        }
        selectedDigis.push_back(digis[iDigi + iFrwd]);
        iFrwd++;
      }
      iFrwd--;
      // looking in the backward direction for a digi that:
      // 1) has a meaningful index
      // 2) has an over-threshold adc value
      // 3) is contiguos
      while (1 && ((iDigi - iBkwd) >= 0)  // 1)
             && ((digis[iDigi - iBkwd])->GetADC() >=
                 minAdcClusterAddendum * ((digis[iDigi])->GetThr()))  // 2)
             && abs(digis[iDigi - iBkwd + 1]->GetChannelNumber() -
                    digis[iDigi - iBkwd]->GetChannelNumber()) <=
                    (iBkwd * maxChDist)  // 3)
      ) {
        if (getDebugLevel() > 0) {
          std::cout << "Adding a 'backward' digi...\t";
          std::cout << "Distance is: "
                    << abs(digis[iDigi]->GetChannelNumber() -
                           digis[iDigi - iBkwd]->GetChannelNumber())
                    << "\n";
        }
        selectedDigis.push_back(digis[iDigi - iBkwd]);
        iBkwd++;
      }
      iBkwd--;
    }
    iDigi += iFrwd;
    if (!selectedDigis.empty()) {
      if (getDebugLevel() > 0) {
        std::cout << " SimpleClusteringAlg: creating new cluster. size: "
                  << selectedDigis.size() << "\n";
      }
      clusterList.push_back(SbtCluster(selectedDigis));
      ++nclusters;
      if (getDebugLevel() > 0) {
        clusterList.back().print();
      }
    }
  }
  return nclusters;
}
