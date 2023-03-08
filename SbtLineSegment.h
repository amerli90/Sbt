#ifndef SBTLINESEGMENT_HH
#define SBTLINESEGMENT_HH

#include "SbtDef.h"

#include <TVector3.h>

class SbtSpacePoint;

//
// Description
//
// represents a line segment in 3D space. Will be used in both SbtHit
// and SbtTrack

class SbtLineSegment {
 public:
  SbtLineSegment() {;}
  SbtLineSegment(const TVector3& x1, const TVector3& x2);

  ~SbtLineSegment() {;}

  double distance(const TVector3& point) const;
  double distance(const SbtLineSegment& line) const;
  TVector3 poca(const SbtLineSegment& line) const;

  double angle(const SbtLineSegment& line) const;

  double angle_xzplane(const SbtLineSegment& line) const;
  double angle_yzplane(const SbtLineSegment& line) const;
  double angle_xyplane(const SbtLineSegment& line) const;

  void print() const;

  TVector3 GetPoint1() const { return _x1; }
  TVector3 GetPoint2() const { return _x2; }

 protected:
  TVector3 _x1;
  TVector3 _x2;

  ClassDef(SbtLineSegment, 1);
};

#endif
