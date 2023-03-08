// C++ Headers
#include <cmath>
#include <iostream>

// ROOT Headers
#include <TMath.h>
#include <TVector3.h>

// including package classes
#include "SbtDetectorType.h"
#include "SbtEnums.h"

ClassImp(SbtDetectorType);

SbtDetectorType::SbtDetectorType(int typeId, std::string typeDet, double xHalfDim,
                                 double yHalfDim, double zHalfDim,
                                 double uActMin, double uActMax, double vActMin,
                                 double vActMax, std::string typeMaterial,
                                 double uPitch, double vPitch, double uAngle,
                                 double vAngle, double uOffset, double vOffset,
                                 double uPrimeActMin, double uPrimeActMax,
                                 double vPrimeActMin, double vPrimeActMax)
    :

      _DebugLevel(0),
      _typeId(typeId),
      _typeDet(typeDet),
      _xHalfDim(xHalfDim),
      _yHalfDim(yHalfDim),
      _zHalfDim(zHalfDim),
      _uActMin(uActMin),
      _uActMax(uActMax),
      _vActMin(vActMin),
      _vActMax(vActMax),
      _typeMaterial(typeMaterial),
      _uPitch(uPitch),
      _vPitch(vPitch),
      _uOffset(uOffset),
      _vOffset(vOffset),
      _elossDistr(SbtEnums::pdf::uniform),
      _elossMPV(1),
      _elossSigma(0),
      _adcGain(512),
      _adcSaturation(1024),
      _signalThrU(0),
      _signalThrV(0),
      _floatingStrips(false),
      _chargeSpreadSigma(0),
      _artificalSpRes(0) {
  _uNstrips = (int)floor((_uActMax - _uActMin) / _uPitch + 0.5);  // MOD
  _vNstrips = (int)floor((_vActMax - _vActMin) / _vPitch + 0.5);  // MOD

  assert(vAngle - uAngle == 90.);

  _uAngle = TMath::Pi() * uAngle / 180.;  // radians
  _vAngle = TMath::Pi() * vAngle / 180.;

  if ("striplet" == _typeDet) {
    _uPrimeActMin = uPrimeActMin;
    _uPrimeActMax = uPrimeActMax;
    _vPrimeActMin = vPrimeActMin;
    _vPrimeActMax = vPrimeActMax;
  } else {
    _uPrimeActMin = _uActMin;
    _uPrimeActMax = _uActMax;
    _vPrimeActMin = _vActMin;
    _vPrimeActMax = _vActMax;
  }
  // just to be safe
  assert(_uPrimeActMin != 0);
  assert(_uPrimeActMax != 0);
  assert(_vPrimeActMin != 0);
  assert(_vPrimeActMax != 0);
}

void SbtDetectorType::setSimulationParameters(SbtEnums::pdf elossDistr, double elossMPV, double elossSigma, double adcGain, int adcSaturation, int thU, int thV, bool floatStrips, double chargeSpread, double artificialRes) {
  _elossDistr = elossDistr;
  _elossMPV = elossMPV;
  _elossSigma = elossSigma;
  _adcGain = adcGain;
  _adcSaturation = adcSaturation;
  _signalThrU = thU;
  _signalThrV = thV;
  _floatingStrips = floatStrips;
  _chargeSpreadSigma = chargeSpread;
  _artificalSpRes = artificialRes;
}

void SbtDetectorType::Print() {
  std::cout << "SbtDetectorType::Print() configuration parameters " << std::endl;
  std::cout << "typeId   \t = \t " << _typeId << std::endl;
  std::cout << "typeDet \t = \t " << _typeDet << std::endl;
  std::cout << "xHalfDim \t = \t " << _xHalfDim << std::endl;
  std::cout << "yHalfDim \t = \t " << _yHalfDim << std::endl;
  std::cout << "zHalfDim \t = \t " << _zHalfDim << std::endl;
  std::cout << "uActMin \t = \t " << _uActMin << std::endl;
  std::cout << "uActMax \t = \t " << _uActMax << std::endl;
  std::cout << "vActMin \t = \t " << _vActMin << std::endl;
  std::cout << "vActMax \t = \t " << _vActMax << std::endl;
  std::cout << "type_Material \t = \t " << _typeMaterial << std::endl;
  std::cout << "uPitch   \t = \t " << _uPitch << std::endl;
  std::cout << "vPitch   \t = \t " << _vPitch << std::endl;
  std::cout << "uAngle   \t = \t " << _uAngle << std::endl;
  std::cout << "vAngle   \t = \t " << _vAngle << std::endl;
  std::cout << "uOffset \t = \t " << _uOffset << std::endl;
  std::cout << "vOffset \t = \t " << _vOffset << std::endl;
}

std::string SbtDetectorType::GetType() {
  std::string detType = _typeDet;
  return detType;
}

int SbtDetectorType::GetIntType() {
  int intType(-1);
  if (_typeDet == "strip")
    intType = 0;
  else if (_typeDet == "striplet")
    intType = 1;
  else if (_typeDet == "pixel")
    intType = 2;
  else if (_typeDet == "singleside")
    intType = 3;
  else {
    std::cout << "SbtDetectorType::GetIntType(): _typeDet not valid " << std::endl;
    assert(0);
  }
  return intType;
}

// This function gives the local coordinate given an electronics channel
// number. It works for all types of detectors.
double SbtDetectorType::Position(int channel, SbtEnums::view side) {
  double pos;
  if (SbtEnums::U == side) {
    pos = uPosition(channel);
  } else {
    pos = vPosition(channel);
  }
  return pos;
}

double SbtDetectorType::uPosition(int channel) {
  // channel goes from 0 to (nstrip-1)

  double pos = _uPitch * (channel - double(_uNstrips - 1) / 2.0);

  return pos;
}

double SbtDetectorType::vPosition(int channel) {
  // channel goes from 0 to (nstrip-1)
  double pos = _vPitch * (channel - double(_vNstrips - 1) / 2.0);

  return pos;
}

int SbtDetectorType::Channel(double pos, SbtEnums::view side) {
  double pitch = _uPitch;
  if (SbtEnums::V == side) pitch = _vPitch;

  int nStrips = _uNstrips;
  if (SbtEnums::V == side) nStrips = _vNstrips;

  double c = pos / pitch + double(nStrips - 1) / 2.0;

  return int(c + .5);
}

void SbtDetectorType::GetEndPoints(SbtEnums::view side, double pos,
                                   TVector3& p1, TVector3& p2) {
  p1[2] = p2[2] = 0;  // w=0 always (in wafer plane)

  if ("strip" == _typeDet) {
    if (SbtEnums::U == side) {  // u strips on normal strip detector

      p1[0] = pos;
      p2[0] = p1[0];

      p1[1] = GetVActMin();
      p2[1] = GetVActMax();

    } else {  // v strips
      p1[1] = pos;
      p2[1] = p1[1];

      p1[0] = GetUActMin();
      p2[0] = GetUActMax();
    }
  } else if ("striplet" == _typeDet) {
    // with striplet calculating the end points is more complicated
    // assume strip strip terminates either at vmax or v' max
    // or vmin or v' min
    //

    double c = cos(GetStripAngle());
    double s = sin(GetStripAngle());

    if (SbtEnums::U == side) {
      //
      // p1 - maximum v
      //

      double u = pos;
      double vmax = GetVActMax();

      // value of v, when vprime = vprime max
      double vcalc = (GetVPrimeActMax() - s * u) / c;
      double v = TMath::Min(vmax, vcalc);

      p1[0] = u;
      p1[1] = v;

      //
      // p2 - small v
      //

      double vmin = GetVActMin();
      vcalc = (GetVPrimeActMin() - s * u) / c;

      v = TMath::Max(vmin, vcalc);

      p2[0] = u;
      p2[1] = v;

    }

    else {  // V side

      // p1 - large u
      //

      double v = pos;
      double umax = GetUActMax();
      double ucalc = (GetVPrimeActMax() - c * v) / s;

      double u = TMath::Min(umax, ucalc);

      p1[0] = u;
      p1[1] = v;

      //
      // p2 - small u
      //

      double umin = GetUActMin();
      ucalc = (GetVPrimeActMin() - c * v) / s;

      u = TMath::Max(umin, ucalc);

      p2[0] = u;
      p2[1] = v;
    }
  } else if ("singleside" == _typeDet) {
    p1[0] = pos;
    p2[0] = p1[0];

    p1[1] = GetVActMin();
    p2[1] = GetVActMax();

  } else {
    assert("pixel" != _typeDet);
  }
}
