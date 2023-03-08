#include <cassert>
#include <iostream>
#include <sstream>

#include <TCanvas.h>
#include <TList.h>

#include "SbtIO.h"
#include "SbtAlignGeom.h"
#include "SbtAlignment.h"
#include "SbtConfig.h"
#include "SbtDetectorElem.h"
#include "SbtEvent.h"
#include "SbtMakeTracks.h"
#include "SbtNtupleDumper.h"
#include "SbtPatRecAlg.h"
#include "SbtSimpleAlignmentAlg.h"

ClassImp(SbtAlignment);

SbtAlignment::SbtAlignment(SbtConfig* configurator, std::string alignConfigFile, SbtEnums::alignMode mode)
    : _debugLevel(0),
      _alignConfigFile(alignConfigFile),
      _configurator(configurator),
      _alignmentAlg(nullptr),
      _nDetToBeAligned(0),
      _mode(mode) {
  std::cout << "SbtAlignment:  DebugLevel= " << _debugLevel << std::endl;

  loadConfiguration(alignConfigFile);

  // Instantiate the _alignGeom
  _alignmentAlg->getAlignGeom()->initialize(_name, _configurator, _mode, _alignPath);
}

SbtAlignment::~SbtAlignment() {
  delete _alignmentAlg;
}

void SbtAlignment::loadConfiguration(std::string alignConfigFile) {
  if (!SbtIO::searchConfigFile(alignConfigFile)) return;
  std::cout << "Loading alignment config file '" << alignConfigFile << "'" << std::endl;
  YAML::Node config = YAML::LoadFile(alignConfigFile);
  if (!config) return;
  _name = config["name"].as<std::string>();
  std::stringstream path;
  path << _configurator->getName() << "/" << config["plotPath"].as<std::string>();
  _plotPath = path.str();
  path.str(""); path.clear();
  path << _configurator->getName() << "/" << config["alignPath"].as<std::string>();
  _alignPath = path.str();
  if (config["detectors"]) getAlignmentDetectors(config["detectors"]);
  // create the list of detectors to be aligned
  if (_nDetToBeAligned) makeAlignmentDetList();
  getAlignmentAlgName(config);
  if (_alignmentAlgName == "Simple") {
    _alignmentAlg = new SbtSimpleAlignmentAlg(_alignDetList);
  } 
  else {
    std::cout << "Invalid alignment algorithm name: " << _alignmentAlgName << std::endl;
    assert(false);
  }
  _alignmentAlg->getAlignmentAlgParms(config["algorithmParameters"]);
  if (config["trackSlopeShifts"] && config["trackSlopeShifts"][_configurator->getName()]) {
    _alignmentAlg->setTrackSlopeShifts(config["trackSlopeShifts"][_configurator->getName()][0].as<double>(), config["trackSlopeShifts"][_configurator->getName()][1].as<double>());
  }
}

void SbtAlignment::getAlignmentDetectors(const YAML::Node& conf) {
  _nDetToBeAligned = conf.size();
  for (auto id : conf) _detToBeAlignedIDList.push_back(id.as<int>());
  std::cout << "Number of Detectors to be aligned \t = \t " << _nDetToBeAligned << std::endl;
  for (unsigned int i = 0; i < _nDetToBeAligned; i++) {
    std::cout << "Alignment Det[" << i << "] \t = \t" << _detToBeAlignedIDList.at(i) << std::endl;
  }
}

void SbtAlignment::getAlignmentAlgName(const YAML::Node& conf) {
  _alignmentAlgName = conf["algorithm"].as<std::string>();
  std::cout << "Alignment algorithm name \t = \t " << _alignmentAlgName << std::endl;
}

void SbtAlignment::makeAlignmentDetList() {
  // make the list of detector to be aligned
  for (unsigned int i = 0; i < _nDetToBeAligned; i++) {
    _alignDetList.push_back(_configurator->getDetectorElemFromID(_detToBeAlignedIDList.at(i)));
  }
}

void SbtAlignment::overrideRoadWidth() {
  if (_debugLevel) {
    std::cout << "SbtAlignment::overrideRoadWidth() _roadWidth = "
              << _alignmentAlg->getRoadWidth(_alignmentAlg->getLoopIndex()) << " (cm) " << std::endl;
  }
  _configurator->getMakeTracks()->getPatRecAlg()->overrideRoadWidth(_alignmentAlg->getRoadWidth(_alignmentAlg->getLoopIndex()));
}

void SbtAlignment::incrementLoopIndex() {
  // override the road widht in the pat rec alg
  overrideRoadWidth();
  _alignmentAlg->incrementLoopIndex();
}

void SbtAlignment::loadEvent(SbtEvent* anEvent) {
  _alignmentAlg->loadEvent(anEvent);
}

void SbtAlignment::align() { 
  _alignmentAlg->align();
}

void SbtAlignment::makePlots() {
  std::cout << "SbtAlignment::makePlots" << std::endl;
  std::stringstream path;
  path << _plotPath << "_" << _alignmentAlg->getLoopIndex();
  if (!SbtIO::createPath(path.str())) {
    std::cout << "Could not create path '" << path.str() << "'." << std::endl;
  }
  else {
    std::cout << "Plots will be saved in '" << path.str() << "'." << std::endl;
  }
  auto plots = _alignmentAlg->makePlots(_alignmentAlg->getLoopIndex());
  for (auto c : plots) {
    std::stringstream fileName;
    fileName << path.str() << "/" << c->GetName() << ".pdf";
    c->SaveAs(fileName.str().c_str());
  }
}

void SbtAlignment::exportHistograms() const {
  std::cout << "SbtAlignment::exportHistograms: start" << std::endl;
  std::stringstream fileName;
  fileName << _plotPath << "_" << _alignmentAlg->getLoopIndex() << "/" << "histograms.root";
  TFile output(fileName.str().c_str(), "recreate");
  output.cd();
  auto histograms = _alignmentAlg->getHistograms();
  for (auto h : histograms) {
    if (h) {
      h->Write(h->GetName(),TObject::kSingleKey);
    }
  }
  std::cout << "SbtAlignment::exportHistograms: closing file" << std::endl;
  output.Close();
  std::cout << "SbtAlignment::exportHistograms: done" << std::endl;
}

void SbtAlignment::writeAlignParms() {
  _alignmentAlg->getAlignGeom()->writeAlignParms(_alignmentAlg->getLoopIndex());
}

bool SbtAlignment::checkConvergence() {
  return _alignmentAlg->checkConvergence();
}
