#include <cassert>
#include <cstdlib>
#include <iostream>

#include "SbtPixelClusteringAlg.h"
#include "SbtDef.h"
#include "SbtDigi.h"

ClassImp(SbtPixelClusteringAlg);

SbtPixelClusteringAlg::SbtPixelClusteringAlg(std::string pxlClusteringOpt)
    : _pxlClusteringOpt(pxlClusteringOpt) {
  std::cout << "SbtPixelClusteringAlg:  DebugLevel= " << getDebugLevel() << std::endl;
}

int SbtPixelClusteringAlg::Clusterize(std::vector<SbtDigi*> selectedDigis, std::vector<SbtCluster>& clusterList) {
  int nDigi = selectedDigis.size();
  if (selectedDigis.size() == 0) return 0;

  std::vector<SbtDigi*> previousNeighbors;
  previousNeighbors.clear();
  std::vector<SbtDigi*> nextNeighbors;
  nextNeighbors.clear();

  int nclusters = 0;

  if (getDebugLevel() > 0) {
    std::cout << " PixelClusteringAlg::Clusterize digiList size = " << nDigi << std::endl;
    std::cout << " Start digi loop " << std::endl;
  }
  std::vector<SbtDigi*>::iterator iterDigi;

  // --- the main loop over digis -----------------------------------//
  while (selectedDigis.size() > 0) {
    // if we are here, we have just finished creating
    // a cluster and are starting a new one.
    // this is the seed
    iterDigi = selectedDigis.begin();

    std::vector<SbtDigi*> clusterDigiList;
    clusterDigiList.clear();
    previousNeighbors.clear();

    int i = 0;  // the i-th (radial distance) of neighbor digi list from seed

    previousNeighbors.push_back(*iterDigi);
    clusterDigiList.push_back(*iterDigi);  // put the seed digi in cluster list

    if (getDebugLevel()) {
      std::cout << "seedDigiList digi " << std::endl;
      (*iterDigi)->print();
    }

    // remove the seed digi from the selectedDigis list.
    // note that this acts as an iteration, since it moves
    // the rest of the list upward

    selectedDigis.erase(iterDigi);

    if (getDebugLevel() > 0) {
      std::cout << "radial distance = " << i << std::endl;
      std::cout << "digiList size after seed erased = " << selectedDigis.size()
           << std::endl;
    }

    // get all pixels adjacent to those in previousNeighbors (which is
    // just the seed at this point)

    nextNeighbors = FindNeighborDigis(previousNeighbors, selectedDigis);

    // if some adjacent digis are found, add them to the cluster list and
    // then look for digis adjacent to _those_

    while (nextNeighbors.size() > 0) {
      i++;  // increment the radial distance counter
      if (getDebugLevel()) {
        std::cout << "nextNeighors size = " << nextNeighbors.size() << std::endl;
      }

      std::vector<SbtDigi*>::iterator iterNeighbor;
      if (getDebugLevel()) {
        std::cout << "Summary nextNeighbor digis " << std::endl;
      }

      // add the found digis to the cluster digi list
      for (iterNeighbor = nextNeighbors.begin();
           iterNeighbor != nextNeighbors.end(); iterNeighbor++) {
        if (getDebugLevel()) {
          std::cout << "neighbor digi printing " << std::endl;
          (*iterNeighbor)->print();
        }
        clusterDigiList.push_back(*iterNeighbor);
      }

      // iterate
      previousNeighbors = nextNeighbors;
      nextNeighbors = FindNeighborDigis(previousNeighbors, selectedDigis);
      if (getDebugLevel())
        std::cout << "after iterate: NN size: " << nextNeighbors.size() << std::endl;
    }

    // if we are here, no more adjacent pixels found. Our list of digis
    // for this cluster is complete. Make the cluster.

    if (getDebugLevel()) {
      std::cout << "Cluster digi list size =  " << clusterDigiList.size() << std::endl;
    }

    clusterList.push_back(SbtCluster(clusterDigiList));
    ++nclusters;
    if (getDebugLevel()) {
      std::cout << "Cluster Added to CluserList=  " << std::endl;
      clusterList.back().print();
    }
    // now go back to selected digis and get a new seed.
  }

  if (getDebugLevel()) {
    std::cout << "Exiting SbtPixelClusteringAlg::Clusterize " << std::endl;
    std::cout << "cluster list size = " << clusterList.size() << std::endl;
  }
  return nclusters;
}

std::vector<SbtDigi*> SbtPixelClusteringAlg::FindNeighborDigis(std::vector<SbtDigi*> prevNeighborDigis, std::vector<SbtDigi*>& SelectedDigis) {
  if (getDebugLevel())
    std::cout << "SbtPixelClusteringAlg::FindNeighborDigis " << std::endl;

  std::vector<SbtDigi*> nextNeighborDigis;
  // SbtEnums::PxlPosWrtSeed PxlPos;

  std::vector<SbtDigi*>::iterator digiIter;

  // loop on outer cluster digis
  for (digiIter = prevNeighborDigis.begin();
       digiIter != prevNeighborDigis.end(); digiIter++) {
    int NeighborRow = (*digiIter)->GetRow();
    int NeighborColumn = (*digiIter)->GetColumn();

    if (getDebugLevel()) {
      std::cout << "FindNeighborDigis: NeighborRow = " << NeighborRow
           << " NeighborColumn = " << NeighborColumn << std::endl;
    }

    // loop on digis outside present cluster and look for someone to attach
    std::vector<SbtDigi*>::iterator SelDigiIter;

    // Need to be careful here with iterator, since we'll be removing
    // elements from this vector as we go.  We'll iterate the iterator
    // by hand at the end of this loop

    for (SelDigiIter = SelectedDigis.begin();
         SelDigiIter != SelectedDigis.end();) {
      int SelRow = (*SelDigiIter)->GetRow();
      int SelColumn = (*SelDigiIter)->GetColumn();

      if (getDebugLevel()) {
        std::cout << "FindNeighborDigis: SelRow = " << SelRow
             << " SelColumn = " << SelColumn << std::endl;
      }

      // set default value
      bool isAdjacent = false;

      if (!(NeighborRow == SelRow && NeighborColumn == SelColumn)) {
        if (_pxlClusteringOpt == "Loose") {
          //"Loose" criteria: adjacent pixel has dx=+/-1, dy=+/-1;
          isAdjacent = (abs(NeighborRow - SelRow) < 2 &&
                        abs(NeighborColumn - SelColumn) < 2);
        } else if (_pxlClusteringOpt == "Tight") {
          //"Tight" criteria: adjacent pixel has dx=+/-1,dy=0 or dx=0,dy=+/-1
          isAdjacent = (abs(NeighborRow - SelRow) < 2 &&
                        (NeighborColumn - SelColumn) == 0) ||
                       ((NeighborRow - SelRow) == 0 &&
                        abs(NeighborColumn - SelColumn) < 2);
        } else {
          std::cout << "SbtPixelClusteringAlg: pxlClusteringOpt unknown; exit "
               << std::endl;
          assert(0);
        }
        if (getDebugLevel()) std::cout << "isAdjacent = " << isAdjacent << std::endl;
      }

      if (isAdjacent) {
        nextNeighborDigis.push_back(*SelDigiIter);
        if (getDebugLevel()) {
          std::cout << "Adding a digi to nextNeighborDigis cluster" << std::endl;
          (*SelDigiIter)->print();
        }
        SelectedDigis.erase(SelDigiIter);
      } else {
        SelDigiIter++;
      }
    }
  }
  return nextNeighborDigis;
}

bool SbtPixelClusteringAlg::isAdjacentPixel(SbtDigi* neighborDigi,
                                            SbtDigi* selDigi,
                                            SbtEnums::PxlPosWrtSeed PxlPos) {
  if (getDebugLevel()) {
    std::cout << "SbtPixelClusteringAlg::isAdjacentPixel"
         << "neighborDigi" << std::endl;
    neighborDigi->print();
    std::cout << "SbtPixelClusteringAlg::isAdjacentPixel"
         << "selDigi" << std::endl;
    selDigi->print();
    std::cout << "Pixel position = " << PxlPos << std::endl;
  }

  bool isAdjacent = false;

  switch (PxlPos) {
    case SbtEnums::top:
      if (neighborDigi->GetColumn() == (selDigi->GetColumn() - 1) ||
          neighborDigi->GetColumn() == (selDigi->GetColumn()) ||
          neighborDigi->GetColumn() == (selDigi->GetColumn() + 1))
        isAdjacent = true;
      break;

    case SbtEnums::bottom:
      if (neighborDigi->GetColumn() == (selDigi->GetColumn() - 1) ||
          neighborDigi->GetColumn() == (selDigi->GetColumn()) ||
          neighborDigi->GetColumn() == (selDigi->GetColumn() + 1))
        isAdjacent = true;
      break;

    case SbtEnums::left:
      if (neighborDigi->GetRow() == (selDigi->GetRow() - 1) ||
          neighborDigi->GetRow() == (selDigi->GetRow()) ||
          neighborDigi->GetRow() == (selDigi->GetRow() + 1))
        isAdjacent = true;
      break;

    case SbtEnums::right:
      if (neighborDigi->GetRow() == (selDigi->GetRow() - 1) ||
          neighborDigi->GetRow() == (selDigi->GetRow()) ||
          neighborDigi->GetRow() == (selDigi->GetRow() + 1))
        isAdjacent = true;
      break;

    case SbtEnums::center:
      isAdjacent = false;
      break;

    default:
      isAdjacent = false;
      std::cout << "SbtPixelClusteringAlg::isAdjacentPixel : PxlPosWrtSee not "
              "defined!"
           << std::endl;
      assert(0);
  }

  return isAdjacent;
}
