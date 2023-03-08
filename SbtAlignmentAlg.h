#ifndef SBT_ALIGNMENTALG
#define SBT_ALIGNMENTALG

#include <map>
#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

#include <Rtypes.h>

class TH2D;
class TProfile;
class TCanvas;
class TList;
class SbtEvent;
class SbtDetectorElem;
class SbtTrack;
class SbtSpacePoint;
class SbtAlignGeom;

class SbtAlignmentAlg {
 public:
  SbtAlignmentAlg(std::vector<SbtDetectorElem*> alignDetList);
  virtual ~SbtAlignmentAlg() {;}

  virtual void getAlignmentAlgParms(const YAML::Node& conf);

  virtual void loadEvent(SbtEvent* event) = 0;
  virtual void align() = 0;
  virtual std::vector<TCanvas*> makePlots(int iteration) = 0;
  virtual bool checkConvergence() const;

  virtual int getLoopIndex() const { return _nLoops; }
  virtual void incrementLoopIndex() { ++_nLoops; buildHisto(); }

  SbtAlignGeom* getAlignGeom() { return _alignGeom; }
  std::vector<SbtDetectorElem*> getDetList() const { return _alignDetList; }

  double getRoadWidth(int i = 0) const;

  void setRoadWidth(double r, int i = 0) { _roadWidth[i] = r; }
  void addRoadWidth(double r) { _roadWidth.push_back(r); }

  virtual std::vector<TList*> getHistograms() const = 0;

  void setTrackSlopeShifts(double x, double y) { _slopeShiftX = x; _slopeShiftY = y; }

 protected:
  virtual void buildHisto() = 0;

  // configuration
  int _debugLevel;
  std::vector<SbtDetectorElem*> _alignDetList;
  double _rotTolerance;
  double _transTolerance;
  std::vector<double> _roadWidth;
  double _corrFraction;

  // current event info
  SbtEvent* _currentEvent;

  int _nLoops;  // an index to count alignment iterations

  // alignment storage
  SbtAlignGeom* _alignGeom;

  double _slopeShiftX;
  double _slopeShiftY;

  ClassDef(SbtAlignmentAlg, 0);
};

#endif
