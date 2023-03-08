#ifndef SBT_ALIGNGEOM
#define SBT_ALIGNGEOM

#include <map>
#include <string>
#include <vector>
#include <array>

#include <TGeoMatrix.h>

#include <SbtEnums.h>

class SbtConfig;
class SbtDetectorElem;
class TGeoPhysicalNode;
class TGeoMatrix;
class TGeoCombiTrans;

class SbtAlignGeom {
 public:
  SbtAlignGeom();
  ~SbtAlignGeom() {;}

  void initialize(std::string name, SbtConfig* config, SbtEnums::alignMode mode, std::string alignPath);  

  // I/O
  void readAlignParms(int loop = -1);
  void writeAlignParms(int loop = -1);

  bool applyAlignment();

  void resetAlignPars();

  void addAlignPars(int detId, std::array<double,6> alignPars) { _alignPars[detId] = alignPars; }
  double getAlignPar(int detId, int parId) const { return _alignPars.at(detId)[parId]; }
  std::array<double,6> getAlignPars(int detId) const { return _alignPars.at(detId); }

 protected:
  static void _applyAlignment(const std::array<double,6> pars, TGeoCombiTrans* t);

  int _debugLevel;
  std::string _name;
  std::string _alignPath;
  SbtEnums::alignMode _mode;
  SbtConfig* _config;

  std::vector<SbtDetectorElem*> _detElemList;

  std::map<int, std::array<double,6>> _alignPars;

  // nominal map <detElemID, TGeoCombiTrans>
  std::map<int, const TGeoCombiTrans*> _nominalAlignMap;

  ClassDef(SbtAlignGeom, 1);
};

#endif
