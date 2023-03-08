#ifndef SBTCLUSTER_HH
#define SBTCLUSTER_HH

#include <cassert>
#include <iostream>
#include <vector>

#include "SbtDigi.h"
#include "SbtEnums.h"

class SbtDetectorElem;

// the class for cluster representation
// a cluster is made from a certain number of digi's
// a cluster is an object in the space, with a barycenter, a width
// if strip detector cluster
// it is localized on one face of a detector
// and it has an associated pulseHeight (total of all digi's pulseheights)
// if pixel maps detector cluster it is localized on both faces
// no pulseHight information

class SbtCluster {
 public:
  SbtCluster();

  // a cluster can be created by a digi's list
  SbtCluster(std::vector<SbtDigi*> aDigiList);

  // get the digi list
  const std::vector<SbtDigi*>& GetDigiList() const { return _digiList; }

  // add a digi
  void AddDigi(SbtDigi* aDigi);

  // get-methods for generic detector
  const SbtDetectorElem* GetDetectorElem() const { return _detectorElem; }

  void SetDetectorElem(const SbtDetectorElem* det) { _detectorElem = det; }

  int GetLength() const { return _length; }
  void SetLength(int length) { _length = length; }

  SbtEnums::digiType GetClusterType() const { return _clusterType; }
  void SetClusterType(SbtEnums::digiType type) { _clusterType = type; }

  // set-methods for strip detector
  void SetSide(SbtEnums::view side) {
    _side = side;
  }

  void SetPulseHeight(double pH) {
    assert(_clusterType == SbtEnums::strip);
    _pulseHeight = pH;
  }

  void AddPulseHeight(double pH) {
    assert(_clusterType == SbtEnums::strip);
    _pulseHeight += pH;
  }

  void SetPosition(double pos) {
    _stripPosition = pos;
  }

  // get-methods for strip detector
  SbtEnums::view GetSide() const {
    assert(_clusterType == SbtEnums::strip);
    return _side;
  }

  double GetPosition() const {
    assert(_clusterType == SbtEnums::strip);
    return _stripPosition;
  }

  double GetPulseHeight() const {
    assert(_clusterType == SbtEnums::strip);
    return _pulseHeight;
  }

  // get-methods for pixel detector
  double GetPxlUPosition() const {
    assert(_clusterType == SbtEnums::pixel);
    return _pxlUPosition;
  }
  double GetPxlVPosition() const {
    assert(_clusterType == SbtEnums::pixel);
    return _pxlVPosition;
  }

  bool IsOnTrack() const { return _IsOnTrack; };
  void SetIsOnTrack(bool isOnTrk) { _IsOnTrack = isOnTrk; };

  void InitStrip();
  void InitPixel();
  void print(bool digis = false) const;

 protected:
  int _debugLevel;

  // introduce cluster type
  SbtEnums::digiType _clusterType;
  bool _IsOnTrack;

  std::vector<SbtDigi*> _digiList;

  const SbtDetectorElem* _detectorElem;  // a pointer to the detector

  // signal related information for generic detector
  int _length;  // total length of the cluster

  // signal related information for strip detectors
  SbtEnums::view _side;   // side of the detector that has fired
  double _pulseHeight;    // total pulse height of the cluster
  double _stripPosition;  // position of the strip cluster in local coordinates

  // signal related information for pixel detectors
  double _pxlUPosition;  // position of the pixel cluster in U local coordinates
  double _pxlVPosition;  // position of the pixel cluster in V local coordinates

  // calculate position for strip detectors
  void calcStripPosition();
  // calculate position for pixel detectors
  void calcPixelPosition();

  static bool lt(SbtDigi* aDigi1, SbtDigi* aDigi2);

  ClassDef(SbtCluster, 1);
};
#endif
