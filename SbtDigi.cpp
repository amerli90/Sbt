// the implementation of the SbtDigi class

#include <iostream>
#include <vector>

#include "SbtDetectorElem.h"
#include "SbtDetectorType.h"
#include "SbtDigi.h"
#include "SbtEnums.h"

ClassImp(SbtDigi);

SbtDigi::SbtDigi()
    : _side(SbtEnums::view::undefinedView),
      _chip(-1),
      _set(-1),
      _strip(-1),
      _adc(-1),
      _bco(0),
      _detectorElem(nullptr),
      _recoType(SbtEnums::recoType::data),
      _DebugLevel(0),
      _digiType(SbtEnums::digiType::undefinedDigiType),
      _IsOnTrack(false) {
  }
// this is the c'tor for strip detectors
SbtDigi::SbtDigi(SbtEnums::view side, int chip, int set, int strip, int adc,
                 unsigned long bco, const SbtDetectorElem* detElem,
                 SbtEnums::recoType RecoType)
    : _side(side),
      _chip(chip),
      _set(set),
      _strip(strip),
      _adc(adc),
      _bco(bco),
      _detectorElem(detElem),
      _recoType(RecoType),
      _DebugLevel(0) {
  // channel initialization
  _channel = _detectorElem->GetChannelNumber(_chip, _set, _strip);
  assert(_detectorElem->GetDetectorType()->GetType() == "strip" ||
         _detectorElem->GetDetectorType()->GetType() == "striplet" ||
         _detectorElem->GetDetectorType()->GetType() == "singleside");
  _digiType = SbtEnums::strip;
  _IsOnTrack = false;
}

// this is the c'tor for pixel detectors
SbtDigi::SbtDigi(int macroColumn, int row, int columnInMP, int bco,
                 const SbtDetectorElem* detElem, SbtEnums::recoType RecoType)
    : _side(SbtEnums::undefinedView),
      _macroColumn(macroColumn),
      _row(row),
      _columnInMP(columnInMP),
      _bco(bco),
      _detectorElem(detElem),
      _recoType(RecoType),
      _DebugLevel(0) {
  assert(_detectorElem->GetDetectorType()->GetType() == "pixel");
  _digiType = SbtEnums::pixel;
  _IsOnTrack = false;
}

double SbtDigi::Position() const {
  // position in local coordinates for strip detectors for each side
  assert(_digiType == SbtEnums::strip);
  SbtEnums::view side = GetSide();
  double pos = _detectorElem->Position(_channel, side);
  return pos;
}

void SbtDigi::Position(double* pos) const {
  // position in local coordinates for pixel detectors
  assert(_digiType == SbtEnums::pixel);
  pos[0] = _detectorElem->Position(GetColumn(), SbtEnums::U);
  pos[1] = _detectorElem->Position(_row, SbtEnums::V);
  return;
}

void SbtDigi::print() const {
  std::cout << "  Digi -- ";
  std::cout << "       Layer: " << GetLayer();

  if (_digiType == SbtEnums::strip) {
    std::cout << ", Side: " << GetSide() << ", Chip: " << GetChip()
         << ", Set: " << GetSet() << ", Strip: " << GetStrip()
         << ", Channel: " << GetChannelNumber() << ", ADC: " << GetADC()
         << ", BCO: " << GetBCO() << std::endl;
  } else if (_digiType == SbtEnums::pixel) {
    std::cout << ", MacroColumn: " << _macroColumn << ", ColumnInM: " << _columnInMP
         << ", Row: " << _row << ", Column: " << GetColumn()
         << ", ADC: " << GetADC() << ", BCO: " << GetBCO() << std::endl;
  }
}

  int SbtDigi::GetPhysicalLayer() const {
    int physicalLayer = GetLayer();
    if (_digiType == SbtEnums::strip) {
      physicalLayer += GetSide() * 100;
    } else if (_digiType == SbtEnums::pixel) {
    } else {
      std::cout << "FATAL: SbtDigi::GetPhysicalLayer:\n";
      std::cout << "Unknown digiType: " << _digiType << "\n";
      std::cout << "Exiting now...\n";
      // exit( 7 );
      assert(false);
    }
    return physicalLayer;
  }

  //  get-methods for strip detectors
  SbtEnums::view SbtDigi::GetSide() const {
    assert(_digiType == SbtEnums::strip);
    return _side;
  }
  int SbtDigi::GetPulseHeight() const {
    assert(_digiType == SbtEnums::strip);
    return _adc;
  }
  int SbtDigi::GetChannelNumber() const {
    assert(_digiType == SbtEnums::strip);
    return _channel;
  }
  int SbtDigi::GetStrip() const {
    assert(_digiType == SbtEnums::strip);
    return _strip;
  }
  int SbtDigi::GetSet() const {
    assert(_digiType == SbtEnums::strip);
    return _set;
  }
  int SbtDigi::GetChip() const {
    assert(_digiType == SbtEnums::strip);
    return _chip;
  }

  //  get-methods for pixel detectors
  int SbtDigi::GetMacroColumn() const {
    assert(_digiType == SbtEnums::pixel);
    return _macroColumn;
  }

  int SbtDigi::GetColumnInMP() const {
    assert(_digiType == SbtEnums::pixel);
    return _columnInMP;
  }

  int SbtDigi::GetColumn() const {
    assert(_digiType == SbtEnums::pixel);
    return (_macroColumn << 2) + _columnInMP;
  }

  int SbtDigi::GetRow() const {
    assert(_digiType == SbtEnums::pixel);
    return _row;
  }