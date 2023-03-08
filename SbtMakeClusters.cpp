#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>

#include "SbtClusteringAlg.h"
#include "SbtEvent.h"
#include "SbtMakeClusters.h"
#include "SbtPixelClusteringAlg.h"
#include "SbtSimpleClusteringAlg.h"

ClassImp(SbtMakeClusters);

SbtMakeClusters::SbtMakeClusters(std::string algorithm, std::string pxlClusteringOpt)
    : _DebugLevel(0) {
  std::cout << "SbtMakeClusters:  DebugLevel= " << _DebugLevel << std::endl;

  // instantiate the correct algorithm
  if (algorithm == "Simple") {
    _stripClusterAlg = new SbtSimpleClusteringAlg();
    _pxlClusterAlg = new SbtPixelClusteringAlg(pxlClusteringOpt);
  } 
  else {
    std::cout << "Invalid clustering algorithm specified: " << algorithm << std::endl;
    assert(0);
  }
}

SbtMakeClusters::~SbtMakeClusters() {
  delete _stripClusterAlg;
  delete _pxlClusterAlg;
}

void SbtMakeClusters::makeClusters(SbtEvent* event) {
  makeStripClusters(event);
  makePxlClusters(event);
}

void SbtMakeClusters::makeStripClusters(SbtEvent* event) {
  // divide digis up into layer/side sub-samples, which
  // will be clustered independently
  orderStripDigis(event->GetStripDigiList());

  // loop over layers and sides
  for (int iLayers = 0; iLayers < maxNTelescopeDetector; iLayers++) {
    for (int side = 0; side < 2; side++) {
      if (_orderedStripDigis[iLayers][side].size() == 0) continue;

      std::vector<SbtDigi*>::iterator aDigi;
      if (_DebugLevel > 1) {
        std::cout << "Ordered digis of layer/side " << iLayers << "/" << side
             << "\n";
      }
      std::vector<SbtDigi*> Digis = _orderedStripDigis[iLayers][side];
      if (_DebugLevel > 1)
        std::cout << "  Sorting sub-list... "
             << "\n";

      // sort the digis in the sublist
      SortStripDigis(Digis);
      if (_DebugLevel > 1)
        std::cout << "(MakeClusters)  sub-list Digi size is: " << Digis.size()
             << "\n";

      // do the clustering on the sub-list
      int nclusters = _stripClusterAlg->Clusterize(Digis, event->GetStripClusterList());

      if (_DebugLevel > 0) {
        std::cout << " ----> " << nclusters
             << " clusters found for layer/side: " << iLayers << "/" << side
             << "\n";
      }
    }  // loop on sides
  }    // loop on layers
}

void SbtMakeClusters::makePxlClusters(SbtEvent* event) {
  // divide digis up into layer sub-samples, which
  // will be clustered independently
  orderPxlDigis(event->GetPxlDigiList());

  // loop over layers
  for (int iLayers = 0; iLayers < maxNDutDetector; iLayers++) {
    if (_orderedPxlDigis[iLayers].size() == 0) continue;

    std::vector<SbtDigi*>::iterator aPxlDigi;
    if (_DebugLevel > 0) {
      std::cout << "Ordered digis of layer " << iLayers << "\n";
    }
    std::vector<SbtDigi*> Digis = _orderedPxlDigis[iLayers];
    if (_DebugLevel > 1)
      std::cout << "  Sorting sub-list... "
           << "\n";

    // sort the digis in the sublist
    SortPxlDigis(Digis);

    if (_DebugLevel > 0)
      std::cout << "(MakeClusters)  sub-list Digi size is: " << Digis.size() << "\n";

    // do the clustering on the sub-list
    int nclusters = _pxlClusterAlg->Clusterize(Digis, event->GetPxlClusterList());

    if (_DebugLevel > 0) {
      std::cout << " ----> " << nclusters
           << " clusters found for layer: " << iLayers << "\n";
    }
  }  // loop on layers
}

void SbtMakeClusters::orderStripDigis(std::vector<SbtDigi>& eventStripDigiList) {
  for (int iLayer = 0; iLayer < maxNTelescopeDetector; iLayer++) {
    _orderedStripDigis[iLayer][0].clear();
    _orderedStripDigis[iLayer][1].clear();
  }
  for (auto& aStripDigi : eventStripDigiList) {
    if (_DebugLevel > 0) {
      std::cout << "layer is: " << aStripDigi.GetLayer()
           << "; side is: " << aStripDigi.GetSide() << "\n";
    }
    _orderedStripDigis[aStripDigi.GetLayer()][aStripDigi.GetSide()].push_back(&aStripDigi);
  }

  if (_DebugLevel > 0) {
    for (int iLayer = 0; iLayer < maxNTelescopeDetector; iLayer++) {
      std::cout << "Layer: " << iLayer
           << "; side: 0; size: " << _orderedStripDigis[iLayer][0].size()
           << "\n";
      std::cout << "Layer: " << iLayer
           << "; side: 1; size: " << _orderedStripDigis[iLayer][1].size()
           << "\n";
    }
  }
}

void SbtMakeClusters::orderPxlDigis(std::vector<SbtDigi>& eventPxlDigiList) {

  for (int iLayer = 0; iLayer < maxNDutDetector; iLayer++) {
    _orderedPxlDigis[iLayer].clear();
    _orderedPxlDigis[iLayer].clear();
  }
  for (auto& aPxlDigi : eventPxlDigiList) {
    if (_DebugLevel > 0) {
      std::cout << "Ordered Pxl Digi layer : " << aPxlDigi.GetLayer()
           << "; row : " << aPxlDigi.GetRow()
           << "; column : " << aPxlDigi.GetColumn() << "\n";
    }
    _orderedPxlDigis[aPxlDigi.GetLayer()].push_back(&aPxlDigi);
  }
  if (_DebugLevel > 0) {
    for (int iLayer = 0; iLayer < maxNDutDetector; iLayer++) {
      std::cout << "Pxl Layer: " << iLayer
           << ";  size: " << _orderedPxlDigis[iLayer].size() << "\n";
    }
  }
}

void SbtMakeClusters::SortStripDigis(std::vector<SbtDigi*>& digiList) {
  assert(digiList.at(0)->GetType() == SbtEnums::strip);
  std::sort(digiList.begin(), digiList.end(), SbtMakeClusters::stripLt);
}

void SbtMakeClusters::SortPxlDigis(std::vector<SbtDigi*>& digiList) {
  assert(digiList.at(0)->GetType() == SbtEnums::pixel);
  std::sort(digiList.begin(), digiList.end(), SbtMakeClusters::pxlLt);
}

bool SbtMakeClusters::stripLt(SbtDigi* d1, SbtDigi* d2) {
  assert(d1->GetType() == SbtEnums::strip);
  return (d1->GetChannelNumber() < d2->GetChannelNumber());
}

bool SbtMakeClusters::pxlLt(SbtDigi* d1, SbtDigi* d2) {
  assert(d1->GetType() == SbtEnums::pixel);
  if (d1->GetRow() != d2->GetRow())
    return (d1->GetRow() < d2->GetRow());
  else
    return (d1->GetColumn() < d2->GetColumn());
}
