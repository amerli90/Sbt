#include <algorithm>

#include "SbtCluster.h"
#include "SbtDigi.h"

ClassImp(SbtCluster);

SbtCluster::SbtCluster()
    : _debugLevel(0),
      _IsOnTrack(false),
      _pulseHeight(0.),
      _side(SbtEnums::view::undefinedView),
      _detectorElem(nullptr),
      _length(0),
      _stripPosition(-999) {}

SbtCluster::SbtCluster(std::vector<SbtDigi*> aDigiList)
    : _debugLevel(0), _IsOnTrack(false) {
  assert(aDigiList.size() > 0);
  assert(aDigiList.at(0) != 0);
  _digiList = aDigiList;
  // get digiType from the first element of the digi list
  _clusterType = aDigiList.at(0)->GetType();
  if (_clusterType == SbtEnums::strip) {
    InitStrip();
  } else if (_clusterType == SbtEnums::pixel) {
    InitPixel();
  } else {
    std::cout << "SbtCluster c'tor: clusterType not defined " << std::endl;
    assert(0);
  }
}

void SbtCluster::InitStrip() {
  // introduce here the view enum type for side (check if this is correct)
  _side = _digiList.at(0)->GetSide();
  _detectorElem = _digiList.at(0)->GetDetectorElem();
  _length = _digiList.size();

  _pulseHeight = 0;
  for (int iDigi = 0; iDigi < _length; iDigi++) {
    _pulseHeight += _digiList.at(iDigi)->GetADC();
  }

  calcStripPosition();
}

void SbtCluster::InitPixel() {
  _side = SbtEnums::undefinedView;  // set default value here
  _detectorElem = _digiList.at(0)->GetDetectorElem();
  _length = _digiList.size();

  _pulseHeight = 0;
  for (int iDigi = 0; iDigi < _length; iDigi++) {
    _pulseHeight += _digiList.at(iDigi)->GetADC();
  }

  calcPixelPosition();
}

void SbtCluster::AddDigi(SbtDigi* aDigi) {
  _digiList.push_back(aDigi);
  _length++;
  if (_clusterType == SbtEnums::strip) _pulseHeight += aDigi->GetADC();
}

void SbtCluster::calcStripPosition() {
  // simply barycenter algorithm for now

  double pos = 0.;

  std::sort(_digiList.begin(), _digiList.end(), lt);

  for (auto digi : _digiList) {
    pos += digi->Position() * digi->GetADC();

    if (_debugLevel > 1) {
      std::cout << "digi    ADC: " << digi->GetADC() << "\t";
    }
  }

  _stripPosition = pos / _pulseHeight;

  if (_debugLevel > 1) {
    std::cout << "cluster pos: " << _stripPosition << std::endl;
  }
}

void SbtCluster::calcPixelPosition() {
  // simply barycenter algorithm for now

  double Upos = 0.;
  double Vpos = 0.;

  for (auto digi : _digiList) {
    double pos[2] = {0., 0.};
    digi->Position(pos);
    Upos += pos[0] * digi->GetADC();
    Vpos += pos[1] * digi->GetADC();
  }

  _pxlUPosition = Upos / _pulseHeight;
  _pxlVPosition = Vpos / _pulseHeight;
}

void SbtCluster::print(bool digis) const {
  if (_clusterType == SbtEnums::strip) {
    std::cout << "Strip Cluster - side: " << _side << ", len: " << _length
         << ",  PH: " << _pulseHeight << ", Pos: " << _stripPosition << std::endl;
  } else if (_clusterType == SbtEnums::pixel) {
    std::cout << "Pixel Cluster - len: " << _length << ", UPos: " << _pxlUPosition
         << ", VPos: " << _pxlVPosition << std::endl;
  }

  if (digis) {
    for (auto digi : _digiList) {
      digi->print();
    }
  }
}

bool SbtCluster::lt(SbtDigi* d1, SbtDigi* d2) {
  assert(d1->GetType() == SbtEnums::strip);
  return (d1->GetChannelNumber() < d2->GetChannelNumber());
}
