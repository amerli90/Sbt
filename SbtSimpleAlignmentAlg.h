#ifndef SBT_SIMPLEALIGNMENTALG
#define SBT_SIMPLEALIGNMENTALG

#include <string>
#include <vector>
#include <array>
#include <set>
#include <map>

#include "SbtAlignmentAlg.h"

class TCanvas;
class SbtEvent;
class SbtTrack;
class TH1;
class TH2;
class TList;
class TProfile;

class SbtSimpleAlignmentAlg : public SbtAlignmentAlg {
 public:
  SbtSimpleAlignmentAlg(std::vector<SbtDetectorElem*> alignDetList);
  ~SbtSimpleAlignmentAlg() {;}

  void loadEvent(SbtEvent* event);
  void align();
  std::vector<TCanvas*> makePlots(int iteration);

  std::vector<TList*> getHistograms() const;

 protected:

  struct trackResidual {
    std::map<int,double> _dx, _dy;
    std::map<int,double> _x, _y;
    double _slpx, _slpy;
    double get_total_x_residuals() const {
      double t = 0;
      for (auto it : _dx) {
        t += it.second * it.second;
      }
      return t;
    }
    double get_total_y_residuals() const {
      double t = 0;
      for (auto it : _dy) {
        t += it.second * it.second;
      }
      return t;
    }
  };

  typedef std::map<const SbtDetectorElem*, std::map<std::pair<std::string,bool(*)(const trackResidual&)>,std::map<std::string,TH1*>>> histogram_map;

  static const std::vector<double>& _getSlopeBins();
  static std::vector<double> _getSlopeBins(double shift);

  void buildHisto();

  void alignPositionXYRotateZ(std::string criteria);

  void makeProfiles();

  void fillHisto();

  void _fillHistoResiduals(histogram_map& histograms, const trackResidual& residuals, const SbtTrack& t, const SbtSpacePoint& sp);
  void _fillHistoSlopes(histogram_map& histograms, const trackResidual& residuals, const SbtTrack& t, const SbtSpacePoint& sp);

  void generateStdDevPlots();

  static std::string convertToMean(std::string obs);
  static std::string convertToStdDev(std::string obs);
  static void drawFitResults(TH1* h);

  static bool isTrackStraight(const trackResidual& tr);
  static bool isTrackNonStraightX(const trackResidual& tr);
  static bool isTrackNonStraightY(const trackResidual& tr);
  static bool isLargeDXLargeSlpX(const trackResidual& tr);
  static bool isLargeDYLargeSlpY(const trackResidual& tr);
  static bool isOutlier_DXSlpX_Det0(const trackResidual& tr);

  template<typename KEYTYPE>
  static TCanvas* plot4(std::string name, std::map<KEYTYPE, TH1*>& histograms, std::set<KEYTYPE> keys, std::string drawOpt="", void(*func)(TH1*)=nullptr, int fx=0, int fy=2);
  static void adjustAxis(TH1* hist, bool sym = true);
  static void adjustAxis2D(TH1* hist, int fx, int fy);

  TProfile* _makeProfile(TH2* hist) const;

  std::map<const SbtDetectorElem*, TH1*> _spacePointsXY;
  TH1* _trackSlopeDistribution;
  TH1* _trackSlopeXVsSumResiduals;
  TH1* _trackSlopeYVsSumResiduals;
  TH1* _trackSlopeXVsSumResidualProfile;
  TH1* _trackSlopeYVsSumResidualProfile;

#if !defined(__CLING__) && !defined(__ROOTCLING__) && !defined(_MAKECINT__) && !defined(_CINT__)
  histogram_map _trackSlopes;
  histogram_map _trackSlopeProfiles;
  histogram_map _trackSlopeStdDev;
  histogram_map _trackResiduals;
  histogram_map _trackResidualProfiles;
  histogram_map _trackResidualStdDev;
#endif
  ClassDef(SbtSimpleAlignmentAlg, 1);
};

#endif
