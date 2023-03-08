#ifndef SBTSPACEPOINT_HH
#define SBTSPACEPOINT_HH

#include <cassert>
#include <string>
#include <iostream>

#include <TVector3.h>

#include "SbtEnums.h"

class SbtHit;
class SbtDetectorElem;
class SbtCluster;

class SbtSpacePoint {
 public:
  SbtSpacePoint();
  SbtSpacePoint(TVector3 point, const SbtDetectorElem* detElem, SbtHit* HitA,
                SbtHit* HitB, std::string errorMethod, double trackDetErr);
  SbtSpacePoint(TVector3 point, const SbtDetectorElem* detElem, SbtHit* HitA,
                std::string errorMethod, double trackDetErr);
  SbtSpacePoint(TVector3 point, TVector3 pointErr, const SbtDetectorElem* detElem,
                SbtHit* HitA, SbtHit* HitB);
  SbtSpacePoint(SbtCluster* pixelCluster, std::string errorMethod,
                double trackDetErr);
  SbtSpacePoint(TVector3 point, const SbtDetectorElem* detElem);  // for generation
  SbtSpacePoint(const SbtSpacePoint& other);                      // copy constructor
  SbtSpacePoint& operator=(const SbtSpacePoint& other);           // assignment operator
  ~SbtSpacePoint() {;}

  const SbtDetectorElem* GetDetectorElem() const { return _detectorElem; }

  TVector3 point() const { return _point; }

  double GetXPosition() const { return _point[0]; };
  double GetYPosition() const { return _point[1]; };
  double GetZPosition() const { return _point[2]; };
  double GetXPositionErr() const { return _pointErr[0]; };
  double GetYPositionErr() const { return _pointErr[1]; };
  double GetZPositionErr() const { return _pointErr[2]; };
  bool IsOnTrack() const { return _IsOnTrack; };
  void SetIsOnTrack(bool isOnTrk) { _IsOnTrack = isOnTrk; };
  SbtEnums::digiType GetDigitType() const { return _digiType; };

  SbtEnums::objectType GetSpacePointType() const { return _spacePointType; }
  void SetSpacePointType(SbtEnums::objectType type) { _spacePointType = type; }

  SbtHit* GetHitU() const;
  SbtHit* GetHitV() const;
  SbtCluster* GetPxlCluster() const;

  bool isValid() const;

  // relational operators
  static bool ltz_ptr(const SbtSpacePoint* aSpacePoint1, const SbtSpacePoint* aSpacePoint2);

  // relational operators
  static bool ltz(const SbtSpacePoint& aSpacePoint1, const SbtSpacePoint& aSpacePoint2);

  void print() const;

 protected:
  void InitError();  // method to be called by each c'tor to properly set
                     // space point errors

  int _DebugLevel;
  TVector3 _point;
  TVector3 _pointErr;
  SbtHit* _hitU;
  SbtHit* _hitV;
  SbtCluster* _pxlCluster;
  SbtEnums::digiType _digiType;
  SbtEnums::objectType _spacePointType;
  bool _IsOnTrack;
  std::string _errorMethod;
  double _trackDetErr;
  const SbtDetectorElem* _detectorElem;

  ClassDef(SbtSpacePoint, 1);
};

#endif
