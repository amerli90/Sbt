#ifndef SBTTRACK_HH
#define SBTTRACK_HH

// the class for track representation
// a track is made of one or two points in the space
// a track is built on two or several hits

#include <vector>
#include <array>

#include <TMatrixD.h>
#include <TVector3.h>
#include <TF1.h>

#include "SbtDef.h"
#include "SbtEnums.h"

class SbtHit;
class SbtSpacePoint;
class SbtDigi;
class SbtDetectorElem;

class SbtTrack {
 public:
  // contructor accept a hit list
  SbtTrack();
  SbtTrack(std::vector<SbtHit*> aHitList, SbtEnums::objectType type = SbtEnums::objectType::reconstructed, SbtEnums::trackShape shape = SbtEnums::trackShape::longTrack);
  SbtTrack(std::vector<SbtSpacePoint*> aSpacePointList, SbtEnums::objectType type = SbtEnums::objectType::reconstructed, SbtEnums::trackShape shape = SbtEnums::trackShape::longTrack);
  SbtTrack(SbtSpacePoint* SP0, SbtSpacePoint* SP1, SbtEnums::objectType type = SbtEnums::objectType::reconstructed, SbtEnums::trackShape shape = SbtEnums::trackShape::longTrack);
  SbtTrack(SbtSpacePoint* SP0, SbtSpacePoint* SP1, SbtSpacePoint* SP2, SbtSpacePoint* SP3, SbtEnums::objectType type = SbtEnums::objectType::reconstructed, SbtEnums::trackShape shape = SbtEnums::trackShape::longTrack);
  SbtTrack(SbtSpacePoint* SP0, SbtSpacePoint* SP1, SbtSpacePoint* SP2, SbtSpacePoint* SP3, SbtSpacePoint* SP4, SbtSpacePoint* SP5, SbtEnums::objectType type = SbtEnums::objectType::reconstructed, SbtEnums::trackShape shape = SbtEnums::trackShape::longTrack);
  SbtTrack(std::array<SbtSpacePoint*,2> us, std::array<SbtSpacePoint*,2> ds, SbtEnums::objectType type, SbtEnums::trackShape shape = SbtEnums::trackShape::channelledTrack);

  SbtTrack(const SbtTrack& other);
  SbtTrack(const SbtTrack&& other);

  SbtTrack& operator=(const SbtTrack& other);
  SbtTrack& operator=(const SbtTrack&& other);

  ~SbtTrack();

  void reset();

  void SortSpacePoints();

  void AddHit(SbtHit* aHit);
  void AddSpacePoint(SbtSpacePoint* aSpacePoint);
  void Print() const;
  void Residual();
  void SetChi2(double chi2) { _chi2 = chi2 ;}
  void SetNdof(int ndof) { _ndof = ndof ;}

  void SetFitStatus(int fitStatus) { _fitStatus = fitStatus; }

  void SetXCovMatrix(TMatrixD CovX);
  void SetYCovMatrix(TMatrixD CovY);

  // method to retreive the hit list
  const std::vector<SbtHit*>& GetHitList() const { return _hitList; }
  // method to retreive the SpacePoint list
  const std::vector<SbtSpacePoint*>& GetSpacePointList() const { return _spacePointList; }
  double GetChi2() const { return _chi2; }
  int GetNdof() const { return _ndof; }
  int GetFitStatus() const { return _fitStatus; }

  void SetTrackFunctionX(TF1* func) { if (_trackFunctionX) delete _trackFunctionX; _trackFunctionX = func; }
  void SetTrackFunctionY(TF1* func) { if (_trackFunctionY) delete _trackFunctionY; _trackFunctionY = func; }

  void SetTrackShape(SbtEnums::trackShape s) { _trackShape = s; }
  void SetTrackType(SbtEnums::objectType s) { _trackType = s; }

  const TF1* GetTrackFunctionX() const { return _trackFunctionX; }
  const TF1* GetTrackFunctionY() const { return _trackFunctionY; }

  TF1* GetTrackFunctionX() { return _trackFunctionX; }
  TF1* GetTrackFunctionY() { return _trackFunctionY; }

  // Legacy functions
  double GetAx() const;
  double GetAy() const;
  double GetBx() const;
  double GetBy() const;
  void SetAx(double Ax);
  void SetAy(double Ay);
  void SetBx(double Bx);
  void SetBy(double By);

  double GetSlopeX() const;
  double GetInterceptX() const;
  double GetSlopeY() const;
  double GetInterceptY() const;
  double GetBendingPoint() const;
  double GetBentSlopeX() const;
  double GetDeflectionAngleX() const;
  double GetBentSlopeY() const;
  double GetDeflectionAngleY() const;

  double GetSlopeXTwoPoints() const;
  double GetSlopeYTwoPoints() const;
  double GetBentSlopeXTwoPoints() const;
  double GetBentSlopeYTwoPoints() const;
  double GetDeflectionAngleXFourPoints() const;
  double GetDeflectionAngleYFourPoints() const;

  const TMatrixD& GetXCovMatrix() const;
  const TMatrixD& GetYCovMatrix() const;
  double GetXResidual(int i) const { return _residualX[i]; }
  double GetYResidual(int i) const { return _residualY[i]; }
  double GetXReco(int i) const { return _recoX[i]; }
  double GetYReco(int i) const { return _recoY[i]; }
  double GetXFit(int i) const { return _fitX[i]; }
  double GetYFit(int i) const { return _fitY[i]; }

  SbtEnums::objectType GetType() const { return _trackType; }
  SbtEnums::trackShape GetShape() const { return _trackShape; }

  // get residuals in local coordinates w.r.t. a given SP
  bool ResidualsOnWafer(const SbtSpacePoint& SP, std::array<double,2>& resids) const;

  bool IntersectPlane(const SbtDetectorElem* detElem, TVector3& point) const;

  void SetIdealTrackParms();
  void AddTrackParmsAtNode(double slpx, double slpy, double x, double y, double z);
  double GetNumberOfSimNodes() const { return _simulationSlpX.size(); }
  double GetSlpXAtNode(int i) const  { return _simulationSlpX[i]    ; }
  double GetSlpYAtNode(int i) const  { return _simulationSlpY[i]    ; }
  double GetPointXAtNode(int i) const  { return _simulationPointX[i]    ; }
  double GetPointYAtNode(int i) const  { return _simulationPointY[i]    ; }
  double GetPointZAtNode(int i) const  { return _simulationPointZ[i]    ; }

 protected:
  int _DebugLevel;

  // type of track
  SbtEnums::objectType _trackType;

  SbtEnums::trackShape _trackShape;

  // define track fit parameters
  TF1* _trackFunctionX;
  TF1* _trackFunctionY;
  int _fitStatus;  // 0=OK
  int _ndof;
  double _chi2;
  double _residualX[maxTrkNSpacePoint];  // X residual
  double _residualY[maxTrkNSpacePoint];  // Y residual

  double _recoX[maxTrkNSpacePoint];  // X reco SP position
  double _recoY[maxTrkNSpacePoint];  // Y reco SP position

  double _fitX[maxTrkNSpacePoint];  // X fit position
  double _fitY[maxTrkNSpacePoint];  // Y fit position

  TMatrixD _CovX;  // X covariance  matrix
  TMatrixD _CovY;  // Y covariance  matrix

  // the list of hit's the track is built on
  std::vector<SbtHit*> _hitList;
  // the list of SpacePoint the track is built on
  std::vector<SbtSpacePoint*> _spacePointList;

  std::vector<double> _simulationSlpX; // slope x at each geometrical node for simulated tracks
  std::vector<double> _simulationSlpY; // slope y at each geometrical node for simulated tracks
  std::vector<double> _simulationPointX; // position x at each geometrical node for simulated tracks
  std::vector<double> _simulationPointY; // position y at each geometrical node for simulated tracks
  std::vector<double> _simulationPointZ; // position z at each geometrical node for simulated tracks

  void Residual(SbtSpacePoint* SP, int i);
  void SetIsOnTrack();
  void SetDigiIsOnTrack(std::vector<SbtDigi*> aDigiList);
  TF1* CreateLinearTrackFunction() const;
  bool IntersectPlane(TVector3 p1, TVector3 p2, const SbtDetectorElem* detElem, TVector3& point) const;

  ClassDef(SbtTrack, 1);
};
#endif
