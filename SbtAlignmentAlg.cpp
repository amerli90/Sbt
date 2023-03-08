#include <cassert>
#include <iostream>

#include "SbtAlignmentAlg.h"
#include "SbtAlignGeom.h"

ClassImp(SbtAlignmentAlg);

SbtAlignmentAlg::SbtAlignmentAlg(std::vector<SbtDetectorElem*> alignDetList) {
  _debugLevel = 0;
  _nLoops = 0;  // an index to count alignment iterations
  _alignDetList = alignDetList;
  _rotTolerance = 0.;
  _transTolerance = 0.;
  _corrFraction = 1.0;
  _alignGeom = new SbtAlignGeom();
  _slopeShiftX = 0;
  _slopeShiftY = 0;
  _currentEvent = nullptr;
}

void SbtAlignmentAlg::getAlignmentAlgParms(const YAML::Node& conf) {
  std::cout << conf << std::endl;
  _roadWidth = conf["roadWidth"].as<std::vector<double>>();
  _rotTolerance = conf["rotTolerance"].as<double>();
  _transTolerance = conf["transTolerance"].as<double>();
  _corrFraction = conf["corrFraction"].as<double>();

  assert(_rotTolerance > 0);
  assert(_transTolerance > 0);

  std::cout << "Road Width for alignment tracks \t = \t " 
            << _roadWidth[0] << " (cm) " << std::endl;
  std::cout << "Rotation tolerance \t = \t "
            << _rotTolerance << " (degree) " << std::endl;
  std::cout << "Translation tolerance \t = \t "
            << _transTolerance << " (cm) " << std::endl;
}

double SbtAlignmentAlg::getRoadWidth(int i) const {
  if (i < 0) i = 0;
  if (i >= _roadWidth.size()) i = _roadWidth.size() - 1;
  return _roadWidth[i];
}

bool SbtAlignmentAlg::checkConvergence() const {
  for (int idet = 0; idet < _alignDetList.size(); idet++) {
    for (int i = 0; i < 3; i++) {
      if (_alignGeom->getAlignPar(idet, i) > _rotTolerance) return false;
    }
    for (int i = 3; i < 6; i++) {
      if (_alignGeom->getAlignPar(idet, i) > _transTolerance) return false;
    }
  }
  return true;
}