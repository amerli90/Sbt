#ifndef SBTHIT_HH
#define SBTHIT_HH

// the class for the hit representation
// an hit is made from one cluster

#include <vector>
#include "SbtEnums.h"
#include "SbtLineSegment.h"

class SbtCluster;
class SbtDetectorElem;
class TVector3;

class SbtHit {
 public:
  SbtHit() {;}
  SbtHit(SbtCluster* aCluster);
  ~SbtHit() {;}

  void print() const;

  // this method is useful for creating space points
  bool Intersection(SbtHit hit, TVector3& point) const;
  bool isOnSingleSide(TVector3& point) const;

  // method to retreive the cluster
  SbtCluster* GetCluster() const { return _theCluster; }
  const SbtDetectorElem* GetDetectorElem() const { return _detectorElem; }
  SbtEnums::view GetSide() const { return _side; }
  bool IsOnTrack() const { return _IsOnTrack; };
  void SetIsOnTrack(bool isOnTrk) { _IsOnTrack = isOnTrk; };

  SbtLineSegment GetLineSegment() const { return _lineSegment; }

 protected:
  bool _IsOnTrack;
  SbtLineSegment _lineSegment;

  int _DebugLevel;

  // the cluster the Hit is built on
  SbtCluster* _theCluster;

  // a pointer to the the dector element
  const SbtDetectorElem* _detectorElem;

  SbtEnums::view _side;  // side of the detector that has fired

  ClassDef(SbtHit, 1);
};
#endif
