#ifndef SBTDIGI_HH
#define SBTDIGI_HH

#include <assert.h>
#include <iostream>
#include <vector>

#include "SbtDef.h"
#include "SbtDetectorElem.h"
#include "SbtEnums.h"

// class for digi representation
// digi is the low level information for detector response
// it deals with: address, detector ID, pulse height, timestamp, side, ...

class SbtDigi {
 public:
  SbtDigi();
  // constructor for strip detector
  SbtDigi(SbtEnums::view side, int chip, int set, int strip, int adc,
          unsigned long bco, const SbtDetectorElem* detElem, SbtEnums::recoType RecoType);

  // constructor for pixel detector
  SbtDigi(int macroColumn, int row, int columnInMP, int bco,
          const SbtDetectorElem* detElem, SbtEnums::recoType RecoType);

  // general get methods

  const SbtDetectorElem* GetDetectorElem() const { return _detectorElem; }
  unsigned long GetTimeStamp() const { return _bco; }
  unsigned long GetBCO() const { return _bco; }
  int GetLayer() const { return _detectorElem->GetID(); }
  SbtEnums::digiType GetType() const { return _digiType; }
  SbtEnums::recoType GetRecoType() const { return _recoType; }

  int GetPhysicalLayer() const;

  //  get-methods for strip detectors
  SbtEnums::view GetSide() const;
  int GetPulseHeight() const;
  int GetChannelNumber() const;
  int GetADC() const { return _adc; }         // strip && pixel
  void SetADC(int adc) { _adc = adc; }  // strip && pixel
  int GetStrip() const;
  int GetSet() const;
  int GetChip() const;
  double GetThr() const { return _thr; }         // strip && pixel
  void SetThr(double thr) { _thr = thr; }  // strip && pixel

  //  get-methods for pixel detectors
  int GetMacroColumn() const;
  int GetColumnInMP() const;
  int GetColumn() const;
  int GetRow() const;
  int GetDaqLayerSide() const { return (_detectorElem->GetLayerSide(_side)); }

  double Position() const;
  void Position(double* pos) const;
  bool IsOnTrack() const { return _IsOnTrack; };
  void SetIsOnTrack(bool isOnTrk) { _IsOnTrack = isOnTrk; };

  void print() const;

  void SetRaw(word raw) { _raw = raw; }
  word GetRaw() const { return _raw; }

  /*
   * All the information of the Digi: address, timestamp, side...
  */

 protected:
  int _DebugLevel;
  bool _IsOnTrack;
  word _raw;  // the word coming from edro
  // add here general digi info
  SbtEnums::view _side;
  unsigned long _bco;
  SbtEnums::digiType _digiType;
  SbtEnums::recoType _recoType;

  // add here strip digi info
  int _chip;
  int _set;
  int _strip;
  int _adc;
  int _channel;
  double _thr;

  // add here pixel digi info
  int _macroColumn;  // 0-31 macro column
  int _row;          // 0-31 row
  int _columnInMP;   // 0-3 column in Macro Pixel

  // detector related information

  const SbtDetectorElem* _detectorElem;  // a pointer to the detector

  ClassDef(SbtDigi, 1);
};

#endif
