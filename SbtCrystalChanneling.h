#ifndef SBT_CRYSTALCHANNELING
#define SBT_CRYSTALCHANNELING

#include <string>
#include <map>

#include <TVector3.h>

#include "SbtGenAlg.h"

class TGeoManager;

class SbtCrystalChanneling {
 public:
  struct crystal_plane_properties {
    double max_efficiency;
    double critical_angle;
    double weight;
  };

  SbtCrystalChanneling(TGeoManager* geoMan,
                       double xPos, double yPos, double zPos,
                       double xHalfDim, double yHalfDim, double zHalfDim,
                       const char* material, double channelingAngle, double deflectionAngle);
  virtual ~SbtCrystalChanneling() {;}

  void setGenAlg(SbtGenAlg* genAlg) { _genAlg = genAlg; }

  void setDebugLevel(int debugLevel) { _debugLevel = debugLevel; }
  int getDebugLevel() const { return _debugLevel; }

  void setDeflectionAngle(double a) { _deflectionAngle = a; }
  void addCrystalPlane(const char* name, double max_eff, double crit_angle, double w=1.0);

  TVector3 getCrystalPosition() const { return _crystalPosition; }

  virtual bool CrystalChanneling(TVector3& point, TVector3& direction);

 protected:
  int _debugLevel;

  TVector3 _crystalPosition;

  std::map<std::string, crystal_plane_properties> _crystalPlanes;

  double _deflectionAngle;
  double _channelingAngle;

  SbtGenAlg* _genAlg;
  TGeoManager* _geoManager;

  TVector3& getLastPoint() { return _genAlg->_lastPoint; }
  TVector3& getLastDirection() { return _genAlg->_lastDirection; }
  TRandom* getRandomGen() { return _genAlg->_aRandomFnc; }
  double getEnergy() { return _genAlg->_energy; }

  // sigma of MS angle distribution
  static double ChannelingEfficiency(double max_efficiency, double critical_angle, double incoming_angle);

  ClassDef(SbtCrystalChanneling, 1);
};
#endif
