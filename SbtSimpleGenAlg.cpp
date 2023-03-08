#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include <map>

#include <TF1.h>
#include <TGeoManager.h>
#include <TVector.h>
#include <TMath.h>

#include "SbtMakeTracks.h"
#include "SbtFittingAlg.h"
#include "SbtDetectorElem.h"
#include "SbtHit.h"
#include "SbtLineSegment.h"
#include "SbtSimpleGenAlg.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"
#include "SbtMultipleScattering.h"
#include "SbtEnums.h"
#include "SbtCrystalChanneling.h"

ClassImp(SbtSimpleGenAlg);

SbtSimpleGenAlg::SbtSimpleGenAlg(TRandom* rN) :
  SbtGenAlg(rN) {
  if (getDebugLevel() > 0) {
    std::cout << "SbtSimpleGenAlg:  DebugLevel= " << getDebugLevel() << std::endl;
  }
}

void SbtSimpleGenAlg::_generateRecoTrack(SbtTrack& track,
                                         std::vector<SbtDigi>& event_strip_digis,
                                         std::vector<SbtDigi>& event_pxl_digis,
                                         std::vector<SbtSpacePoint>& event_space_points,
                                         word BCOCounter) {
  SbtSpacePoint aPoint;
  while (Navigate(true, aPoint, track)) {
    if (getDebugLevel()) {
      std::cout << "Reco point" << std::endl;
      aPoint.print();
    }

    _lastPoint = TVector3(_theGeoManager->GetCurrentPoint());
    _lastDirection = TVector3(_theGeoManager->GetCurrentDirection());

    if (getDebugLevel()) {
      std::cout << "Current position: " << _lastPoint.X() << " "
                << _lastPoint.Y() << " " << _lastPoint.Z() << " "
                << std::endl;

      std::cout << "Current direction: " << _lastDirection.X() << " "
                << _lastDirection.Y() << " " << _lastDirection.Z() << " "
                << std::endl;
    }

    if (!aPoint.isValid()) {
      if (getDebugLevel() > 1) {
        std::cout << "point is not valid" << std::endl;
      }
      continue;
    }

    event_space_points.push_back(aPoint);
    track.AddSpacePoint(&event_space_points.back());

    // creation of the digis
    if (getDebugLevel()) {
      std::cout << aPoint.GetXPosition() << ", " << aPoint.GetYPosition() << ", " << aPoint.GetZPosition() << std::endl;
    }

    TVector3 master(aPoint.GetXPosition(), aPoint.GetYPosition(), aPoint.GetZPosition());

    auto detElem = aPoint.GetDetectorElem();
    if (detElem == nullptr) {
      std::cout << "FATAL! SbtGenerator::GenerateEvent NULL pointer" << std::endl;
      std::cout << "Exiting now..." << std::endl;
      exit(2);
    }

    TVector3 local(0, 0, 0);  // local coords u, v, w
    detElem->MasterToLocal(master, local);

    TVector3 localDirection(0, 0, 0);  // localDirection coords u, v, w
    detElem->MasterToLocalVect(_lastDirection, localDirection);

    if (getDebugLevel()) {
      std::cout << "SbtSimpleGenAlg::GenerateTrack : master = (" << master[0]
            << ", " << master[1] << ", " << master[2] << ")" << std::endl;
    }

    if (detElem->GetDetectorType()->GetType() == "strip" || detElem->GetDetectorType()->GetType() == "striplet") {
      createStripDigi(detElem, local, localDirection, SbtEnums::U, BCOCounter, event_strip_digis);
      createStripDigi(detElem, local, localDirection, SbtEnums::V, BCOCounter, event_strip_digis);
    } 
    else if (detElem->GetDetectorType()->GetType() == "pixel") {
      event_pxl_digis.push_back(createPxlDigi(detElem, local, BCOCounter));
    }
    else if (detElem->GetDetectorType()->GetType() == "singleside") {
      createStripDigi(detElem, local, localDirection, SbtEnums::U, BCOCounter, event_strip_digis);
    }
  }
  track.SetTrackType(SbtEnums::objectType::simulated);
  SetTrackRealDirection(track, true);
}

void SbtSimpleGenAlg::_generateIdealTrack(SbtTrack& track, std::vector<SbtSpacePoint>& event_space_points) {
  SbtSpacePoint aPoint;
  while (Navigate(false, aPoint, track)) {
    if (getDebugLevel()) {
      std::cout << "Ideal/simul point" << std::endl;
      aPoint.print();
    }

    _lastPoint = TVector3(_theGeoManager->GetCurrentPoint());
    _lastDirection = TVector3(_theGeoManager->GetCurrentDirection());

    if (getDebugLevel()) {
      std::cout << "Current position: " << _lastPoint.X() << " "
                << _lastPoint.Y() << " " << _lastPoint.Z() << " "
                << std::endl;

      std::cout << "Current direction: " << _lastDirection.X() << " "
                << _lastDirection.Y() << " " << _lastDirection.Z() << " "
                << std::endl;
    }

    if (!aPoint.isValid()) {
      if (getDebugLevel() > 1) {
        std::cout << "point is not valid" << std::endl;
      }
      continue;
    }

    event_space_points.push_back(aPoint);
    track.AddSpacePoint(&event_space_points.back());
  }
  track.SetTrackType(SbtEnums::objectType::ideal);
  SetTrackRealDirection(track);
}

bool SbtSimpleGenAlg::GenerateTrack(SbtEvent& event) {
  if (getDebugLevel()) {
    std::cout << std::endl << "SbtSimpleGenAlg::GenerateTrack()" << std::endl;
    if (!_theGeoManager) {
      std::cerr << "Fatal! In SbtSimpleGenAlg::"
                << "GenerateTrack() no geoManager is set." << std::endl << "Exiting now..." << std::endl;
      exit(19);
    }
    if (!_theGeoManager->GetTopNode()) {
      std::cerr << "Fatal! In SbtSimpleGenAlg::"
                << "GenerateTrack() no TopNode is found" << std::endl << "Exiting now..." << std::endl;
      exit(20);
    }

    std::cout << "Initializing starting point" << std::endl;
  }

  InitStartPoint();  // setting the starting point
  const double* startPoint = _theGeoManager->GetCurrentPoint();
  const double* startDir = _theGeoManager->GetCurrentDirection();

  if (getDebugLevel()) {
    std::cout << "Starting position: " << *(startPoint) << " "
              << *(startPoint + 1) << " " << *(startPoint + 2) << " "
              << std::endl;

    std::cout << "Starting direction: " << *(startDir) << " " << *(startDir + 1)
              << " " << *(startDir + 2) << " "
              << std::endl;
  }

  if (getDebugLevel() > 0) {
    std::cout << "Initializing ideal navigator with point: " << _beamPoint.X()
              << " " << _beamPoint.Y() << " " << _beamPoint.Z() << " "
              << ".\nInitializing ideal navigator with direction: "
              << _beamDirection.X() << " " << _beamDirection.Y() << " "
              << _beamDirection.Z() << " "
              << std::endl;
  }

  // set the first navigator for the ideal track (without multiple scattering)
  _theGeoManager->InitTrack(_beamPoint.X(), _beamPoint.Y(), _beamPoint.Z(),
                            _beamDirection.X(), _beamDirection.Y(), _beamDirection.Z());
  _generateIdealTrack(event.AddIdealTrack(), event.GetSpacePointList());

  if (getDebugLevel()) {
    std::cout << "Initializing simulated navigator with point: "
              << _beamPoint.X() << " " << _beamPoint.Y() << " "
              << _beamPoint.Z() << "." << std::endl
              << "Initializing simulated navigator with direction: "
              << _beamDirection.X() << " " << _beamDirection.Y() << " "
              << _beamDirection.Z() << " "
              << std::endl;
  }

  // set the second navigator for the simulated track (with multiple scattering)
  _theGeoManager->InitTrack(_beamPoint.X(), _beamPoint.Y(), _beamPoint.Z(),
                            _beamDirection.X(), _beamDirection.Y(), _beamDirection.Z());
  _generateRecoTrack(event.AddSimulatedTrack(), event.GetStripDigiList(), event.GetPxlDigiList(), event.GetSpacePointList(), event.GetBCOCounter());

  return true;
}

void SbtSimpleGenAlg::InitStartPoint() {
  if (getDebugLevel()) std::cout << "SbtSimpleGenAlg::InitStartPoint()" << std::endl;

  // getting the detector elems list
  std::vector<SbtDetectorElem*> detectorElems = _theConfigurator->getDetectorElemList();
  // calculating the starting point
  double firstZPos = 0.;
  std::sort(detectorElems.begin(), detectorElems.end(), SbtDetectorElem::ltz);
  firstZPos = detectorElems.at(0)->GetZPos();

  if (getDebugLevel()) std::cout << "SbtSimpleGenAlg::InitStartPoint() firstZPos = " << firstZPos << std::endl;

  double startingPoint[3] = {0., 0., -firstZPos};
  double startingDirection[3] = {0., 0., 1.};

  //default method for simple beam tracks generation
  GeneratInitialTrack(startingPoint, startingDirection);

  if (getDebugLevel()) {
    std::cout << "SbtSimpleGenAlg::InitStartPoint()" << std::endl;
    std::cout << "Starting point: " << std::endl;
    std::cout << startingPoint[0] << ", " << startingPoint[1] << ", "
              << startingPoint[2] << std::endl;
    std::cout << "Starting Direction: " << std::endl;
    std::cout << startingDirection[0] << ", " << startingDirection[1] << ", "
              << startingDirection[2] << std::endl;
  }

  if (_theGeoManager == nullptr) {
    std::cerr << "Fatal! _theGeoManager is NULL in "
                 "SbtSimpleGenAlg::InitStartPoint()." << std::endl << "Exiting now..." << std::endl;
    exit(22);
  }
  if (_theGeoManager->InitTrack(startingPoint, startingDirection) == nullptr) {
    std::cerr << "Fatal! _theGeoManager->InitTrack is NULL!" << std::endl << "Exiting now..." << std::endl;
    exit(23);
  }
  if (getDebugLevel()) {
    std::cout << "Starting Point and Direction initialized..." << std::endl;
  }

  _beamPoint = TVector3(startingPoint[0], startingPoint[1], startingPoint[2]);
  _beamDirection = TVector3(startingDirection[0], startingDirection[1], startingDirection[2]);
  _beamDirection *= 1. / (_beamDirection.Mag());

  _lastPoint = TVector3(startingPoint[0], startingPoint[1], startingPoint[2]);
  _lastDirection = TVector3(startingDirection[0], startingDirection[1], startingDirection[2]);

  if (getDebugLevel()) {
    std::cout << "Starting position: " << _lastPoint.X() << " "
              << _lastPoint.Y() << " " << _lastPoint.Z() << " "
              << std::endl;

    std::cout << "Starting direction: " << _lastDirection.X() << " "
              << _lastDirection.Y() << " " << _lastDirection.Z() << " "
              << std::endl;
  }
}

bool SbtSimpleGenAlg::Navigate(bool MS, SbtSpacePoint& sp, SbtTrack& track) {
  if (getDebugLevel()) {
    std::cout << std::endl << "SbtSimpleGenAlg::Navigate()" << std::endl;
  }

  TVector3 point, direction;

  /*
   *  navigation for the first point
   */

  // looking for next boundary
  if (NavigateAPoint(MS, track)) {
    point = TVector3(*(_theGeoManager->GetCurrentPoint()),
                      *(_theGeoManager->GetCurrentPoint() + 1),
                      *(_theGeoManager->GetCurrentPoint() + 2));
    direction = TVector3(*(_theGeoManager->GetCurrentDirection()),
                          *(_theGeoManager->GetCurrentDirection() + 1),
                          *(_theGeoManager->GetCurrentDirection() + 2));
  }
  else {
    if (getDebugLevel()) std::cout << "navigate false" << std::endl;
    return false;
  }

  SbtDetectorElem* theDetElem = nullptr;
  if (_detID >= 0) {
    theDetElem = GetDetectorElem(_detID);

    if (theDetElem == nullptr) {
      std::cout << "FATAL! NULL pointer to theDetElem for ID: " << _detID << std::endl;
      exit(4);
    }
  }

  new (&sp) SbtSpacePoint(point, theDetElem);
  if (MS) {
    sp.SetSpacePointType(SbtEnums::objectType::simulated);
  }
  else {
    sp.SetSpacePointType(SbtEnums::objectType::ideal);
  }

  if (getDebugLevel() > 0) {
    std::cout << "SbtSimpleGenAlg::Navigate : detElemID = " << _detID
         << "  SpacePoint = ( " << sp.GetXPosition() << ", "
         << sp.GetYPosition() << ", " << sp.GetZPosition() << " ) "
         << std::endl;
  }

  return true;
}

bool SbtSimpleGenAlg::NavigateAPoint(bool MS, SbtTrack& track) {
  if (getDebugLevel()) {
    std::cout << std::endl << "SbtSimpleGenAlg::NavigateAPoint()" << std::endl;
  }

  _detID = -1;
  bool navigationOK = false;
  TGeoNode* node = _theGeoManager->GetCurrentNode();
  while (_detID < 0) {
    navigationOK = true;
    if (!_theGeoManager->FindNextBoundary()) return false;
    double step = _theGeoManager->GetStep();

    if (getDebugLevel()) {
      std::cout << std::endl << std::endl << "New iteration" << std::endl;
      std::cout << "The step to be taken is: " << step << std::endl;
      std::cout << "The current node is " << std::endl;
      _theGeoManager->GetCurrentNode()->Print();
    }
    if (step < minStep) {
      if (getDebugLevel()) {
        std::cout << "Warning: step is too low!" << std::endl
                  << "Trying to force the step..." << std::endl;
      }
      TVector3 tempPoint = TVector3(_theGeoManager->GetCurrentPoint());
      TVector3 tempDir = TVector3(_theGeoManager->GetCurrentDirection());
      int iForce = 0;
      do {
        ForceStep(tempPoint, tempDir);
        iForce++;
        _theGeoManager->InitTrack(tempPoint.X(), tempPoint.Y(), tempPoint.Z(), tempDir.X(), tempDir.Y(), tempDir.Z());
        if (getDebugLevel()) {
          std::cout << iForce << " times the point has been moved forward." << std::endl;
        }
        step = _theGeoManager->GetStep();
        node = _theGeoManager->GetCurrentNode();
      } while (step < minStep && _theGeoManager->FindNextBoundary());
      if (!_theGeoManager->FindNextBoundary()) return false;
      if (getDebugLevel()) {
        std::cout << "The step to be taken is: " << step << std::endl;
        std::cout << "The current node is " << std::endl;
        _theGeoManager->GetCurrentNode()->Print();
      }
    }

    if (_theGeoManager->GetCurrentNode() != _theGeoManager->GetTopNode()) {
      double masterCoord[3] = {.0, .0, .0};
      double localCoord[3] = {.0, .0, .0};
      _theGeoManager->GetCurrentNode()->LocalToMaster(localCoord, masterCoord);
      if (getDebugLevel() > 0) {
        std::cout << "SbtSimpleGenAlg::NavigateAPoint : masterCoord = ("
             << masterCoord[0] << ", " << masterCoord[1] << ", "
             << masterCoord[2] << ") " << std::endl;
        std::cout << "DetID = " << GetDetectorIDfromZCoord(masterCoord[2]) << std::endl;
      }
      // get the detector z position in the master coordinates
      _detID = GetDetectorIDfromZCoord(masterCoord[2]);

      if (getDebugLevel()) {
        std::cout << "_detID = " << _detID << std::endl;
        _theGeoManager->GetCurrentNode()->GetMedium()->Print();
        _theGeoManager->GetCurrentNode()->GetMedium()->GetMaterial()->Print();
      }
    }
    else {
      _detID = -1;
      if (getDebugLevel()) {
        std::cout << "Not in a detector" << std::endl;
        _theGeoManager->GetCurrentNode()->GetMedium()->Print();
        _theGeoManager->GetCurrentNode()->GetMedium()->GetMaterial()->Print();
      }
    }

    _lastPoint = TVector3(_theGeoManager->GetCurrentPoint());
    _lastDirection = TVector3(_theGeoManager->GetCurrentDirection());

    if (step > 1e-4) {
      SetTrackRealDirection(track, false);
    }

    _lastNode = node;

    if (getDebugLevel()) {
      std::cout << "Last node: ";
      _lastNode->Print();

      std::cout << "Last point: " << _lastPoint.X() << " " << _lastPoint.Y()
                << " " << _lastPoint.Z() << " "
                << std::endl;

      std::cout << "Last direction: " << _lastDirection.X() << " "
                << _lastDirection.Y() << " " << _lastDirection.Z() << " "
                << std::endl;
    }

    // move current point forward
    _theGeoManager->FindNextBoundaryAndStep();

    TVector3 point = TVector3(_theGeoManager->GetCurrentPoint());
    TVector3 direction = TVector3(_theGeoManager->GetCurrentDirection());
    node = _theGeoManager->GetCurrentNode();
    if (getDebugLevel()) {
      std::cout << "Projected node: ";
      node->Print();
      std::cout << "Projected point: " << point.X() << " " << point.Y() << " "
                << point.Z() << " "
                << std::endl;

      std::cout << "Projected direction: " << direction.X() << " "
                << direction.Y() << " " << direction.Z() << " "
                << std::endl;
    }

    bool crystalChanneledParticle = false;
    if (TString(_lastNode->GetName()).BeginsWith("crystal") && _lastNode != node) {
      assert(_crystalChanneling);
      crystalChanneledParticle = _crystalChanneling->CrystalChanneling(point, direction);

      if (crystalChanneledParticle) {
        _theGeoManager->InitTrack(point.X(), point.Y(), point.Z(), 
                                  direction.X(), direction.Y(), direction.Z());

        if (getDebugLevel()) {
          std::cout << "Channeled point: " << point.X() << " " << point.Y() << " "
                    << point.Z() << " "
                    << std::endl;

          std::cout << "Channeled direction: " << direction.X() << " "
                    << direction.Y() << " " << direction.Z() << " "
                    << std::endl;
        }
      }
      else {
        if (getDebugLevel()) {
          std::cout << "No channeling occured!" << std::endl;
        }
      }
    }

    // multiple scattering
    if (!crystalChanneledParticle && MS && _multipleScattering) {
      double RadLen = _lastNode->GetMedium()->GetMaterial()->GetRadLen();
      _multipleScattering->MultipleScattering(point, direction, RadLen);

      _theGeoManager->InitTrack(point.X(), point.Y(), point.Z(), 
                                direction.X(), direction.Y(), direction.Z());

      if (getDebugLevel()) {
        std::cout << "Corrected point: " << point.X() << " " << point.Y() << " "
                  << point.Z() << " "
                  << std::endl;

        std::cout << "Corrected direction: " << direction.X() << " "
                  << direction.Y() << " " << direction.Z() << " "
                  << std::endl;
      }
    }
  }
  return navigationOK;
}

void SbtSimpleGenAlg::GenerateStripNoise(std::vector<SbtDigi>& digis) {
  if (getDebugLevel()) {
    std::cout << "SbtSimpleGenAlg::GenerateStripNoise() method is starting..." << std::endl;
  }
  /*
   * Strategy: a loop over the detectors to create Poiss(occ. X #channels)
   * random events. They will be randomly distributed over the faces
   * of the detectors.
   */

  if (getDebugLevel() > 0)
    std::cout << "Getting  the detector element list..." << std::endl;

  if (nullptr == _theConfigurator) {
    std::cout << "FATAL: the configurator has not been initialized!" << std::endl;
    exit(5);
  }

  if (getDebugLevel() > 0) {
    std::cout << "Detector elem list has " << _theConfigurator->getDetectorElemList().size()<< " detectors" << std::endl;
  }

  // loop on detectors
  for (auto aDetector : _theConfigurator->getDetectorElemList()) {
    if (aDetector->GetDetectorType()->GetType() == "pixel") continue;

    int layer = aDetector->GetID();  // the layer

    if (getDebugLevel() > 0)
      std::cout << "Looping over detector with ID: " << layer << std::endl;

    if (getDebugLevel() > 0)
      std::cout << "It has " << aDetector->GetDetectorType()->GetUnStrips()
                << " strips on u side" << std::endl;

    int uNstrips = aDetector->GetDetectorType()->GetUnStrips();  // the number of channels on u side
    int meanUnoise = int(_stripOccupancy * uNstrips);  // the average # of noise hits
    int nUnoise = _aRandomFnc->Poisson(meanUnoise);  // a sampled # of noise hits

    if (getDebugLevel() > 0) std::cout << "Generating " << nUnoise << " noise-digi on u-side:" << std::endl;

    // for each noise hit we 'simulate' adc, bco and channel number
    for (int i = 0; i < nUnoise; i++) {
      int uChannel = int(_aRandomFnc->Rndm() * uNstrips);     // channel #
      int ubco = int(_aRandomFnc->Rndm() * 0xff);             // bco count
      int uAdc = _aRandomFnc->Poisson(_avgNoiseAdc + 1) - 1;  // adc count
      uAdc = (uAdc >= 0) ? uAdc : 0;  // adc < 0 -> adc = 0

      int uchip, uset, ustrip;  // chip, set, strip
      aDetector->GetStripDigiData(uChannel, uchip, uset, ustrip);

      // the noise digi to push back
      digis.push_back(SbtDigi(SbtEnums::U, uchip, uset, ustrip, uAdc, ubco, aDetector, SbtEnums::simulatedNoise));
      digis.back().SetThr(2);

      if (getDebugLevel()) {
        std::cout << "A strip digi has been created with: " << std::endl;
        digis.back().print();
      }
    }

    if (aDetector->GetDetectorType()->GetType() == "singleside") continue;

    // now we redo exactly the same for the v side
    if (getDebugLevel() > 0) {
      std::cout << "It has " << aDetector->GetDetectorType()->GetVnStrips() << " strips on v side" << std::endl;
    }

    int vNstrips = aDetector->GetDetectorType()->GetVnStrips();
    int meanVnoise = int(_stripOccupancy * vNstrips);
    int nVnoise = _aRandomFnc->Poisson(meanVnoise);

    if (getDebugLevel() > 0) std::cout << "Generating " << nVnoise << " noise-digi on v-side:" << std::endl;

    for (int i = 0; i < nVnoise; i++) {
      int vChannel = int(_aRandomFnc->Rndm() * vNstrips);
      int vbco = int(_aRandomFnc->Rndm() * 0xff);
      int vAdc = _aRandomFnc->Poisson(_avgNoiseAdc + 1) - 1;
      vAdc = (vAdc >= 0) ? vAdc : 0;
      int vchip, vset, vstrip;
      aDetector->GetStripDigiData(vChannel, vchip, vset, vstrip);
      digis.push_back(SbtDigi(SbtEnums::V, vchip, vset, vstrip, vAdc, vbco, aDetector, SbtEnums::simulatedNoise));
      digis.back().SetThr(2);

      if (getDebugLevel() > 0) {
        std::cout << "A strip digi has been created with: " << std::endl;
        digis.back().print();
      }
    }
  }  // loop on detectors
}

void SbtSimpleGenAlg::GeneratePxlNoise(std::vector<SbtDigi>& digis) {
  if (getDebugLevel()) {
    std::cout << "SbtSimpleGenAlg::GeneratePxlNoise() method is starting..." << std::endl;
  }
  /*
   * Strategy: a loop over the detectors to create Poiss(occ. X #channels)
   * random events. They will be randomly distributed over the faces
   * of the detectors.
   */

  if (getDebugLevel() > 0) std::cout << "Getting  the detector element list..." << std::endl;

  if (nullptr == _theConfigurator) {
    std::cout << "FATAL: the configurator has not been initialized!" << std::endl;
    exit(5);
  }
  if (getDebugLevel() > 0) std::cout << "Detector elem list has " << _theConfigurator->getDetectorElemList().size() << " detectors" << std::endl;

  // loop on detectors
  for (auto aDetector : _theConfigurator->getDetectorElemList()) {
    if (aDetector->GetDetectorType()->GetType() != "pixel") continue;

    int layer = aDetector->GetID();  // the layer

    if (getDebugLevel() > 0)
      std::cout << "Looping over detector with ID: " << layer << std::endl;

    if (getDebugLevel() > 0)
      std::cout << "It has " << aDetector->GetDetectorType()->GetNPxl()
                << " pixels " << std::endl;

    // the number of channels on u side
    int NPxl = aDetector->GetDetectorType()->GetNPxl();
    int NColumn = aDetector->GetDetectorType()->GetNColumn();
    int NRow = aDetector->GetDetectorType()->GetNRow();

    // the average # of noise hits
    int meanNoise = int(_pxlOccupancy * NPxl);
    // a sampled # of noise hits
    int NNoise = _aRandomFnc->Poisson(meanNoise);

    if (getDebugLevel() > 0)
      std::cout << "Generating " << NNoise << " pixel noise-digi:" << std::endl;

    // for each noise hit we 'simulate' bco and channel number
    for (int i = 0; i < NNoise; i++) {
      int Column = int(_aRandomFnc->Rndm() * NColumn);       // pixel column #
      int Row = int(_aRandomFnc->Rndm() * NRow);             // pixel row #
      int adc = _aRandomFnc->Poisson(_avgNoiseAdc + 1) - 1;  // adc count
      int bco = int(_aRandomFnc->Rndm() * 0xff);             // bco count
      int macroColumn, columnInMP, row;                      // chip, set, strip
      aDetector->GetPxlDigiData(Row, Column, macroColumn, columnInMP, row);

      // the noise digi to push back

      digis.push_back(SbtDigi(macroColumn, row, columnInMP, bco, aDetector, SbtEnums::simulatedNoise));
      digis.back().SetADC(adc);
      digis.back().SetThr(2);

      if (getDebugLevel() > 0) {
        std::cout << "A pxl digi has been created with: " << std::endl;
        digis.back().print();
      }
    }
  }
}

void SbtSimpleGenAlg::ForceStep(TVector3& point, TVector3& direction) {
  /*
   * This is a hack to resolve too low step
   */
  double step = _theGeoManager->GetStep();
  if (step < minStep) {
    point += minStep * (direction);
  }
}

void SbtSimpleGenAlg::GeneratInitialTrack(double* startingPoint, double* startingDirection) {
  // starting position
  startingPoint[0] = _aRandomFnc->Uniform(-_beam_sigma_x, _beam_sigma_x);
  startingPoint[1] = _aRandomFnc->Uniform(-_beam_sigma_y, _beam_sigma_y);
  startingPoint[2] -= _d_beam_tele;

  //  starting direction
  double x_dir = 0., y_dir = 0., z_dir = 1.;
  if (_angular_div > 0) {
    switch (_angular_div_distr) {
      case SbtEnums::pdf::gaus:
      x_dir = _aRandomFnc->Gaus(_angular_mean_x, _angular_div);
      y_dir = _aRandomFnc->Gaus(_angular_mean_y, _angular_div);
      break;

      case SbtEnums::pdf::uniform:
      x_dir = _aRandomFnc->Uniform(-_angular_div/2 + _angular_mean_x, _angular_div/2 + _angular_mean_x);
      y_dir = _aRandomFnc->Uniform(-_angular_div/2 + _angular_mean_y, _angular_div/2 + _angular_mean_y);
      break;

      case SbtEnums::pdf::landau:
      default:
      std::cout << "Cannot use pdf " << _angular_div_distr << " for the beam angular spread!" << std::endl;
      break;
    }
  }
  else {
      x_dir = _angular_mean_x;
      y_dir = _angular_mean_y;
  }

  double mag = TMath::Sqrt(x_dir*x_dir + y_dir*y_dir + z_dir*z_dir);
  startingDirection[0] = x_dir / mag;
  startingDirection[1] = y_dir / mag;
  startingDirection[2] = z_dir / mag;
}

int SbtSimpleGenAlg::createStripDigi(const SbtDetectorElem* detElem,
                                     TVector3 local, TVector3 direction,
                                     SbtEnums::view digiView, word BCOCounter,
                                     std::vector<SbtDigi>& event_digi_list) {
  double z_HalfDim = detElem->GetDetectorType()->GetZ_HalfDim();

  if (detElem->GetDetectorType()->getArtificialSpRes() > 0) {
    local.SetX(_aRandomFnc->Gaus(local.X(), detElem->GetDetectorType()->getArtificialSpRes()));
    local.SetY(_aRandomFnc->Gaus(local.Y(), detElem->GetDetectorType()->getArtificialSpRes()));
  }

  TVector3 local_in = local - direction * (z_HalfDim / direction.z());
  TVector3 local_out = local + direction * (z_HalfDim / direction.z());

  double pitch = 0;
  double local_in_coord = 0;
  double local_out_coord = 0;
  double local_coord = 0;
  int thr = 0;
  
  if (digiView == SbtEnums::U) {
    pitch = detElem->GetDetectorType()->GetUpitch();
    local_in_coord = local_in.x();
    local_out_coord = local_out.x();
    local_coord = local.x();
    thr = detElem->GetDetectorType()->getSignalThresholdU();
  }
  else if (digiView == SbtEnums::V) {
    pitch = detElem->GetDetectorType()->GetVpitch();
    local_in_coord = local_in.y();
    local_out_coord = local_out.y();
    local_coord = local.y();
    thr = detElem->GetDetectorType()->getSignalThresholdV();
  }

  double sigma = detElem->GetDetectorType()->getChargeSpreadSigma();

  if (getDebugLevel() > 2) {
    std::cout << "Pitch/sigma = " << pitch / sigma << std::endl;
  }

  // This is actually not used properly later... see below.
  if (local_in_coord < local_out_coord) {
    local_in_coord -= 4 * sigma;
    local_out_coord += 4 * sigma;
  }
  else {
    local_in_coord += 4 * sigma;
    local_out_coord -= 4 * sigma;
  }

  int Channel_in = detElem->Channel(local_in_coord, digiView);
  int Channel_out = detElem->Channel(local_out_coord, digiView);

  int min_channel = 0;
  int max_channel = 0;
  if (Channel_in < Channel_out) {
    min_channel = Channel_in;
    max_channel = Channel_out;
  }
  else {
    min_channel = Channel_out;
    max_channel = Channel_in;
  }

  int nDigis = abs(Channel_out - Channel_in) + 1;


  double adc = getRandomSignal(detElem->GetDetectorType());

  int nAddedDigi = 0;

  if (getDebugLevel() > 2) {
    std::cout << "Total signal: " << adc << std::endl;
  }

  if (nDigis > 1) {
    int Channel[nDigis], Chip[nDigis], Set[nDigis], Strip[nDigis];
    int signal[nDigis];

    for (int i = 0; i < nDigis; i++) {
      int ch =  min_channel + i;
      signal[i] = 0;
      Channel[i] = ch;
      // In fact one should do something different
      // First calculate the number of strips traversed by the particle (local_coord_in and local_coord_out)
      // Then one should somehow convolute this distance with Gaussian distributions of charges
      double x1 = (TMath::Abs(detElem->Position(ch, digiView) - local_coord) - pitch / 2) / sigma / 2 * TMath::Sqrt2();
      double x2 = (TMath::Abs(detElem->Position(ch, digiView) - local_coord) + pitch / 2) / sigma / 2 * TMath::Sqrt2();
      double signal_fraction = (TMath::Erf(x2) - TMath::Erf(x1)) / 2;
      if (getDebugLevel() > 2) {
        std::cout << "Fraction: " << signal_fraction << std::endl;
      }

      detElem->GetStripDigiData(Channel[i], Chip[i], Set[i], Strip[i]);
      double sig = adc * signal_fraction;
      if (detElem->GetDetectorType()->isFloatingStrip() && Channel[i] % 2 == 0) {
        if (i - 1 >= 0) {
          signal[i - 1] += sig / 2;
        }
        if (i + 1 < nDigis) {
          signal[i + 1] += sig / 2;
        }
      }
      else {
        signal[i] = sig;
      }

      if (getDebugLevel() > 2) {
        std::cout << "Digit signal: " << signal[i] << std::endl;
      }
      if (signal[i] < thr) continue;
      if (signal[i] > detElem->GetDetectorType()->getadcSaturation()) signal[i] = detElem->GetDetectorType()->getadcSaturation();

      event_digi_list.push_back(SbtDigi(digiView, Chip[i], Set[i], Strip[i], signal[i], BCOCounter, detElem, SbtEnums::simulatedSignal));
      event_digi_list.back().SetThr(2.);
      ++nAddedDigi;
      if (getDebugLevel() > 2) {
        std::cout << "Digit added" << std::endl;
      }
    }
  }
  else {
    int Chip = -1, Set = -1, Strip = -1;
    detElem->GetStripDigiData(Channel_in, Chip, Set, Strip);
    if (adc >= thr) {
      if (adc > detElem->GetDetectorType()->getadcSaturation()) adc = detElem->GetDetectorType()->getadcSaturation();
      event_digi_list.push_back(SbtDigi(digiView, Chip, Set, Strip, adc, BCOCounter, detElem, SbtEnums::simulatedSignal));
      event_digi_list.back().SetThr(2.);
      ++nAddedDigi;
    }
  }
  return nAddedDigi;
}

SbtDigi SbtSimpleGenAlg::createPxlDigi(const SbtDetectorElem* detElem,
                                       TVector3 local, word BCOCounter) {
  int Column(-1);
  int Row(-1);
  // digi info for pixel

  double adc = getRandomSignal(detElem->GetDetectorType());
  int thr = detElem->GetDetectorType()->getSignalThresholdU();
  if (adc < thr) adc = 0;

  Column = detElem->GetDetectorType()->Channel(local.x(), SbtEnums::U);
  Row = detElem->GetDetectorType()->Channel(local.y(), SbtEnums::V);

  int macroColumn, columnInMP, row;
  detElem->GetPxlDigiData(Row, Column, macroColumn, columnInMP, row);

  SbtDigi aSimPxlDigi(macroColumn, row, columnInMP, BCOCounter, detElem, SbtEnums::simulatedSignal);
  aSimPxlDigi.SetADC(adc);
  aSimPxlDigi.SetThr(2.);
  return aSimPxlDigi;
}
