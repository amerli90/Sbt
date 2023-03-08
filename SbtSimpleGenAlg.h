#ifndef SBT_SIMPLEGENALG
#define SBT_SIMPLEGENALG

#include <TRandom.h>

#include "SbtDigi.h"
#include "SbtEnums.h"
#include "SbtGenAlg.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"

class SbtSimpleGenAlg : public SbtGenAlg {
 public:
  SbtSimpleGenAlg(TRandom* rN);
  virtual ~SbtSimpleGenAlg() {;}

  bool GenerateTrack(SbtEvent& event);

 protected:
  TVector3 _beamPoint;
  TVector3 _beamDirection;

  void _generateIdealTrack(SbtTrack& track, std::vector<SbtSpacePoint>& event_space_points);
  void _generateRecoTrack(SbtTrack& track,
                          std::vector<SbtDigi>& event_strip_digis,
                          std::vector<SbtDigi>& event_pxl_digis,
                          std::vector<SbtSpacePoint>& event_space_points,
                          word BCOCounter);

  int createStripDigi(const SbtDetectorElem* detElem,
                      TVector3 local, TVector3 direction,
                      SbtEnums::view digiView,
                      word BCOCounter,
                      std::vector<SbtDigi>& event_digi_list);
  SbtDigi createPxlDigi(const SbtDetectorElem* detElem, TVector3 local, word BCOCounter);

  void GenerateStripNoise(std::vector<SbtDigi>& digis);
  void GeneratePxlNoise(std::vector<SbtDigi>& digis);
  void InitStartPoint();
  bool Navigate(bool MS, SbtSpacePoint& point, SbtTrack& track);
  bool NavigateAPoint(bool MS, SbtTrack& track);
  void ForceStep(TVector3& point, TVector3& direction);

  virtual void GeneratInitialTrack(double* startingPoint, double* startingDirection);

 ClassDef(SbtSimpleGenAlg, 1);
};
#endif
