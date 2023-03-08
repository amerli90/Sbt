#ifndef SBT_DETECTORTYPE
#define SBT_DETECTORTYPE

#include <cassert>
#include <string>

#include <Rtypes.h>

#include "SbtEnums.h"

class TVector3;

class SbtDetectorType {
  friend class SbtDetectorElem;
 public:
  SbtDetectorType(int typeId, std::string typeDet, double xHalfDim,
                  double yHalfDim, double zHalfDim, double uActMin,
                  double uActMax, double vActMin, double vActMax,
                  std::string typeMaterial, double uPitch, double vPitch,
                  double uAngle, double vAngle, double uOffset, double vOffset,
                  double uPrimeActMin = 0, double uPrimeActMax = 0,
                  double vPrimeActMin = 0, double vPrimeActMax = 0);

  ~SbtDetectorType() {;}
  void Print();
  std::string GetType();
  int GetIntType();
  int GetID() { return _typeId; }

  std::string GetTypeMaterial() { return _typeMaterial; };
  std::string GetTypeDet() { return _typeDet; };

  double GetX_HalfDim() { return _xHalfDim; };
  double GetY_HalfDim() { return _yHalfDim; };
  double GetZ_HalfDim() { return _zHalfDim; };

  double GetUActMin() { return _uActMin; }
  double GetUActMax() { return _uActMax; }
  double GetVActMin() { return _vActMin; }
  double GetVActMax() { return _vActMax; }
  double GetUPrimeActMin() { return _uPrimeActMin; }
  double GetUPrimeActMax() { return _uPrimeActMax; }
  double GetVPrimeActMin() { return _vPrimeActMin; }
  double GetVPrimeActMax() { return _vPrimeActMax; }

  int GetUnStrips() {
    assert(_typeDet == "strip" || _typeDet == "striplet" ||
           _typeDet == "singleside");
    return _uNstrips;
  }
  int GetVnStrips() {
    assert(_typeDet == "strip" || _typeDet == "striplet");
    return _vNstrips;
  }
  int GetNPxl() {
    assert(_typeDet == "pixel");
    return _uNstrips * _vNstrips;
  }
  int GetNColumn() {
    assert(_typeDet == "pixel");
    return _uNstrips;
  }
  int GetNRow() {
    assert(_typeDet == "pixel");
    return _vNstrips;
  }

  double Position(int channel, SbtEnums::view side);
  double uPosition(int channel);
  double vPosition(int channel);

  double GetStripAngle() {
    assert(_typeDet == "strip" || _typeDet == "striplet" ||
           _typeDet == "singleside");
    return _uAngle;
  }  //

  int Channel(double position, SbtEnums::view side);

  void GetEndPoints(SbtEnums::view side, double pos, TVector3& x1,
                    TVector3& x2);

  double GetUpitch() { return _uPitch; }
  double GetVpitch() { return _vPitch; }

  void setSimulationParameters(SbtEnums::pdf elossDistr, double elossMPV, double elossSigma, double adcGain, int adcSaturation, int thU, int thV, bool floatStrips, double chargeSpread, double artificialRes=0.);

  SbtEnums::pdf getElossDistr() const { return _elossDistr; }
  double getElossMPV() const { return _elossMPV; }
  double getElossSigma() const { return _elossSigma; }
  double getadcGain() const { return _adcGain; }
  int getadcSaturation() const { return _adcSaturation; }
  int getSignalThresholdU() const { return _signalThrU; }
  int getSignalThresholdV() const { return _signalThrV; }
  bool isFloatingStrip() const { return _floatingStrips; }
  double getChargeSpreadSigma() const { return _chargeSpreadSigma; }
  double getArtificialSpRes() const { return _artificalSpRes; }

 protected:
  int _DebugLevel;
  int _typeId;
  std::string _typeDet;
  double _xHalfDim, _yHalfDim, _zHalfDim;

  // define the active area

  double _uActMin, _uActMax;
  double _vActMin, _vActMax;
  double _uPrimeActMin, _uPrimeActMax;
  double _vPrimeActMin, _vPrimeActMax;

  std::string _typeMaterial;
  double _uPitch, _vPitch;
  double _uAngle, _vAngle;
  double _uOffset, _vOffset;
  int _uNstrips, _vNstrips;

  // For simulation
  SbtEnums::pdf _elossDistr;
  double _elossMPV;
  double _elossSigma;
  double _adcGain;
  int _adcSaturation;
  int _signalThrU;
  int _signalThrV;
  bool _floatingStrips;
  double _chargeSpreadSigma;
  double _artificalSpRes;

  ClassDef(SbtDetectorType, 1);
};


#endif
