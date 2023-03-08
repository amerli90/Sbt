#ifndef SBT_DETECTORELEM
#define SBT_DETECTORELEM

#include <TGeoManager.h>
#include <TVector3.h>
#include <vector>
#include "SbtEnums.h"

class TGeoVolume;
class SbtDetectorType;
class TGeoRotation;
class TGeoTranslation;
class TGeoCombiTrans;

class SbtDetectorElem {
 public:
  
  SbtDetectorElem(TGeoManager* geom, SbtDetectorType* detType, int detElemId,
                  int detTrackId, double xPos, double yPos, double zPos,
                  double Phi, double Theta, double Psi, double orientation_U,
                  double orientation_V);
  virtual ~SbtDetectorElem() {;}

  virtual void LocalToMaster(double* local, double* master) const;  // 1 : double
  virtual void LocalToMasterVect(double* master, double* local) const;  // 9 : double
  virtual void MasterToLocal(double* master, double* local) const;  // 2 : double
  virtual void MasterToLocalVect(double* master, double* local) const;  // 7 : double

  // Local frame coincide with wafer axis (useful for striplet detector
  // alignment)
  // the local prime coordinate system is like local, but with u' and v'
  // nominally aligned along the x, y directions. local and local prime are
  // different only for striplet detectors.
  virtual void LocalToLocalPrime(double* local, double* localprime) const;  // 5 : double

  virtual void LocalPrimeToMaster(double* localprime, double* master) const;  // 3 : double
  virtual void LocalPrimeToLocal(double* localprime, double* local) const;  // 6 : double
  virtual void LocalPrimeToMasterVect(double* local, double* localprime) const;  // 10: double
  virtual void MasterToLocalPrime(double* master, double* localprime) const;  // 4 : double
  virtual void MasterToLocalPrimeVect(double* master, double* localprime) const;  // 8 : double

  // TVector3 versions
  void LocalToMaster(TVector3 local, TVector3& master) const;  // 1 : TVector3
  void LocalToMasterVect(TVector3 local, TVector3& master) const;  // 9 : TVector3
  void MasterToLocal(TVector3 master, TVector3& local) const;  // 2 : TVector3
  void MasterToLocalVect(TVector3 master, TVector3& local) const;  // 7 : TVector3

  void LocalToLocalPrime(TVector3 local, TVector3& localprime) const;  // 5 : TVector3
  void LocalPrimeToMaster(TVector3 localprime, TVector3& master) const;  // 3 : TVector3
  void LocalPrimeToLocal(TVector3 localprime, TVector3& local) const;  // 6 : TVector3
  void LocalPrimeToMasterVect(TVector3 localprime, TVector3& master) const;  // 10: TVector3
  void MasterToLocalPrime(TVector3 master, TVector3& localprime) const;  // 4 : TVector3
  void MasterToLocalPrimeVect(TVector3 master, TVector3& localprime) const;  // 8 : TVector3

  SbtDetectorType* GetDetectorType() const { return _detType; }
  int GetID() const { return _detElemId; }
  int GetTrackingID() const { return _detTrackId; }
  double GetXPos() const { return _xPos; }
  double GetYPos() const { return _yPos; };
  double GetZPos() const { return _zPos; };
  void AddLayerSide(int i) { _layerSides.push_back(i); }
  int GetLayerSide(SbtEnums::view LayerView) const;
  const TGeoManager* GetGeoManager() const { return _geoManager; };
  const TGeoRotation* GetRotation() const { return _rot; };
  const TGeoTranslation* GetTranslation() const { return _tr; };
  const TGeoCombiTrans* GetCombiTrans() const { return _rt; };

  TGeoRotation* GetRotation() { return _rot; };
  TGeoTranslation* GetTranslation() { return _tr; };
  TGeoCombiTrans* GetCombiTrans() { return _rt; };

  virtual bool InActiveArea(TVector3 point) const;

  // pure virtual method to transform the set/strip information
  // in a single channel information

  // only for strip detectors
  virtual int GetChannelNumber(int chip, int set, int strip) const = 0;
  virtual void GetStripDigiData(int channel, int& chip, int& set, int& strip) const = 0;

  virtual void GetPxlDigiData(int Row, int Rolumn, int& macroColumn, int& columnInMP, int& row) const = 0;

  std::string GetDetectorPath() const { return _thePath; }

  double Position(int channel, SbtEnums::view side) const;
  int Channel(double pos, SbtEnums::view side) const;

  // relational operators
  static bool ltz(SbtDetectorElem* DetElem1, SbtDetectorElem* DetElem2);

 protected:
  int _DebugLevel;
  TGeoManager* _geoManager;
  TGeoVolume* _detVolume;
  TGeoRotation* _rot;
  TGeoTranslation* _tr;
  TGeoCombiTrans* _rt;
  int _detElemId;
  int _detTrackId;
  double _xPos;
  double _yPos;
  double _zPos;
  std::vector<int> _layerSides;
  std::string _thePath;  // the detector path in our geometry

  double _orientation_U;
  double _orientation_V;

  SbtDetectorType* _detType;  // make the data available to the inheriting
                              // classes Strip, Striplets, Pixel

  ClassDef(SbtDetectorElem, 0);  // Implementation of DetectorElem
};

#endif
