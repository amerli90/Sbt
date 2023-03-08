#include <iomanip>
#include <iostream>

#include "SbtLineSegment.h"

#include <TVector3.h>

ClassImp(SbtLineSegment);

SbtLineSegment::SbtLineSegment(const TVector3& x1, const TVector3& x2) : _x1(x1), _x2(x2){};

double SbtLineSegment::distance(const TVector3& x0) const {
  TVector3 d1 = _x2 - _x1;
  TVector3 d2 = _x1 - x0;

  if (0 == d1.Mag()) return 0;

  double d = d1.Cross(d2).Mag() / d1.Mag();
  return d;
}

double SbtLineSegment::distance(const SbtLineSegment& line) const {
  auto u = _x2 - _x1;
  auto v = line._x2 - line._x1;
  auto a = u.Mag2();
  auto b = u.Dot(v);
  auto c = v.Mag2();
  auto w = _x1 - line._x1;
  auto d = u.Dot(w);
  auto e = v.Dot(w);
  auto sum_mag = u.Mag2() + v.Mag2();
  if (a < 1e-12 * sum_mag || c < 1e-12 * sum_mag) return 0.;
  TVector3 distance;
  if (a*c - b*b < 1e-12 * sum_mag) {
    distance = w - e / c * v;
  }
  else {
    distance = w + ((b*e - c*d) * u - (a*e - b*d)* v) * (1.0 / (a*c - b*b));
  }
  return distance.Mag();
}

void SbtLineSegment::print() const {
  std::cout << "  LineSegment - x1: ";
  std::cout << _x1[0] << ", " << _x1[1] << ", " << _x1[2];

  std::cout << std::endl << "              - x2: ";
  std::cout << _x2[0] << ", " << _x2[1] << ", " << _x2[2];
  std::cout << std::endl;
}

double SbtLineSegment::angle(const SbtLineSegment& line) const {
  return (_x2 - _x1).Angle(line._x2 - line._x1);
}

double SbtLineSegment::angle_xyplane(const SbtLineSegment& line) const {
  auto v1 = _x2 - _x1;
  v1.SetZ(0.);
  auto v2 = line._x2 - line._x1;
  v2.SetZ(0.);
  return v1.Angle(v2);
}

double SbtLineSegment::angle_xzplane(const SbtLineSegment& line) const {
  auto v1 = _x2 - _x1;
  v1.SetY(0.);
  auto v2 = line._x2 - line._x1;
  v2.SetY(0.);
  return v1.Angle(v2);
}

double SbtLineSegment::angle_yzplane(const SbtLineSegment& line) const {
  auto v1 = _x2 - _x1;
  v1.SetX(0.);
  auto v2 = line._x2 - line._x1;
  v2.SetX(0.);
  return v1.Angle(v2);
}

TVector3 SbtLineSegment::poca(const SbtLineSegment& line) const {
  auto u = _x2 - _x1;
  auto v = line._x2 - line._x1;
  auto diff = u - v;
  auto sum_mag = u.Mag2() + v.Mag2();
  if (diff.Mag2() < 1e-12 * sum_mag) {
    return _x1;
  }
  auto w = _x1 - line._x1;
  auto t = -w.Dot(diff) / diff.Mag2();
  auto poca = _x1 + u * t;
  return poca;
}