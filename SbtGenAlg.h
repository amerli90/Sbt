#ifndef SBTGENALG_HH
#define SBTGENALG_HH

#include <vector>

#include <TRandom.h>

#include "SbtConfig.h"
#include "SbtEnums.h"
#include "SbtSpacePoint.h"
#include "SbtEvent.h"
#include "SbtTrack.h"

class SbtMultipleScattering;
class SbtCrystalChanneling;
class SbtHit;
class SbtDigi;
class SbtTrack;
class SbtSpacePoint;

class SbtGenAlg {
 public:
  SbtGenAlg();
  SbtGenAlg(TRandom* rnd);
  virtual ~SbtGenAlg() {;}

  void setDebugLevel(int debugLevel) { _debugLevel = debugLevel; }
  int getDebugLevel() const { return _debugLevel; }

  void SetConfigurator(SbtConfig* aConfigurator);

  void SetAngularDivPdf(SbtEnums::pdf distr) { _angular_div_distr = distr; }
  void SetAngularDiv(double angDiv) { _angular_div = angDiv; }
  void SetMeanAngularDirection(double xdir, double ydir) { _angular_mean_x = xdir; _angular_mean_y = ydir; }
  void SetBeamSX(double bSX) { _beam_sigma_x = bSX; }
  void SetBeamSY(double bSY) { _beam_sigma_y = bSY; }
  void SetPId(int PId) { _particle_id = PId; }
  void SetEnergy(double e) { _energy = e; }
  void SetDB2T(double dbt) { _d_beam_tele = dbt; }

  void SetStripOcc(double stripOcc) { _stripOccupancy = stripOcc; }
  void SetPxlOcc(double pxlOcc) { _pxlOccupancy = pxlOcc; }
  void SetAvgNoiseAdcVal(int ANAV) { _avgNoiseAdc = ANAV; }

  int GetPId() const { return _particle_id; }

  virtual bool GenerateTrack(SbtEvent& event) = 0;
  virtual void GenerateStripNoise(std::vector<SbtDigi>& digi) = 0;
  virtual void GeneratePxlNoise(std::vector<SbtDigi>& digi) = 0;

  // a method to retrieve a detector elem pointer given its ID
  SbtDetectorElem* GetDetectorElem(int ID);

  // a method to retrieve a detector elem pointer given its ID
  int GetDetectorIDfromZCoord(double zCoord);

  void SetMultipleScattering(SbtMultipleScattering* ms) { _multipleScattering = ms; }
  void SetCrystalChanneling(SbtCrystalChanneling* cc) { _crystalChanneling = cc; }

 protected:
  int _debugLevel;

  TVector3 _lastPoint;
  TVector3 _lastDirection;
  TGeoNode* _lastNode;
  int _detID;

  SbtConfig* _theConfigurator;  // the telescope configurator
  TGeoManager* _theGeoManager;

  SbtMultipleScattering* _multipleScattering;

  SbtCrystalChanneling* _crystalChanneling;

  SbtEnums::pdf _angular_div_distr; // PDF of the angular divergence distribution
  double _angular_div;         // angular divergence (rad)
  double _angular_mean_x;      // mean beam angular direction in xz plane (rad)
  double _angular_mean_y;      // mean beam angular direction in xz plane (rad)
  double _beam_sigma_x;        // beam x_axis dispersion (cm)
  double _beam_sigma_y;        // beam y_axis dispersion (cm)
  int _particle_id;            // particle id (see pdt.table)
  double _energy;              // beam energy (MeV)
  double _d_beam_tele;         // distance beam-first module
  std::string _topVolumePath;  // the top volume path

  double _stripOccupancy;  // strip occupancy (%)
  double _pxlOccupancy;    // pxl occupancy (%)
  int _avgNoiseAdc;        // average adc oise level

  TRandom* _aRandomFnc;  // the random-numbers generator

  void SetTrackRealDirection(SbtTrack& track, bool last=true) const;
  double getRandomSignal(const SbtDetectorType* detType) const;

 private:
  friend class SbtMultipleScattering;
  friend class SbtCrystalChanneling;

  ClassDef(SbtGenAlg, 0);
};

#endif
