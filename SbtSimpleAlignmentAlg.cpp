#include <iostream>
#include <algorithm>

#include <TCanvas.h>
#include <TF1.h>
#include <TGeoMatrix.h>
#include <TH2D.h>
#include <TProfile.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TList.h>
#include <TPaveText.h>
#include <TRandom.h>

#include "SbtDetectorElem.h"
#include "SbtDetectorType.h"
#include "SbtEvent.h"
#include "SbtSimpleAlignmentAlg.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"
#include "SbtAlignGeom.h"

ClassImp(SbtSimpleAlignmentAlg);

SbtSimpleAlignmentAlg::SbtSimpleAlignmentAlg(std::vector<SbtDetectorElem*> alignDetList)
    : SbtAlignmentAlg(alignDetList), _trackSlopeDistribution(nullptr), _trackSlopeXVsSumResiduals(nullptr), _trackSlopeYVsSumResiduals(nullptr), _trackSlopeXVsSumResidualProfile(nullptr), _trackSlopeYVsSumResidualProfile(nullptr) {
  std::cout << "SbtSimpleAlignmentAlg:  DebugLevel= " << _debugLevel << std::endl;
  if (_debugLevel) {
    std::cout << "_alignDetList size = " << _alignDetList.size() << std::endl;
  }
}

void SbtSimpleAlignmentAlg::loadEvent(SbtEvent* event) {
  if (_debugLevel > 1) std::cout << "SbtSimpleAlignmentAlg::loadEvent " << std::endl;

  _currentEvent = event;

  fillHisto();
}

void SbtSimpleAlignmentAlg::align() {
  alignPositionXYRotateZ("All");
}

TProfile* SbtSimpleAlignmentAlg::_makeProfile(TH2* hist) const {
  std::stringstream profName;
  profName << hist->GetName() << "_Prof";
  TProfile* prof = nullptr;
  if (hist->GetXaxis()->GetXbins() && hist->GetXaxis()->GetXbins()->GetSize() > 1) {
    prof = new TProfile(profName.str().c_str(), profName.str().c_str(), hist->GetXaxis()->GetXbins()->GetSize() - 1, hist->GetXaxis()->GetXbins()->GetArray(), "i");
  }
  else {
    prof = new TProfile(profName.str().c_str(), profName.str().c_str(), hist->GetNbinsX(), hist->GetXaxis()->GetXmin(), hist->GetXaxis()->GetXmax(), "i");
  }
  prof->GetXaxis()->SetTitle(hist->GetXaxis()->GetTitle());
  prof->GetYaxis()->SetTitle(convertToMean(hist->GetYaxis()->GetTitle()).c_str());
  prof->SetMarkerStyle(kFullCircle);
  prof->SetMarkerSize(0.7);
  prof->SetMarkerColor(kBlue+2);
  prof->SetLineColor(kBlue+2);
  return prof;
}

void SbtSimpleAlignmentAlg::makeProfiles() {
  TProfile* prof = nullptr;
  for (auto level1 : _trackResiduals) {
    for (auto level2 : level1.second) {
      for (auto level3 : level2.second) {
        prof = _makeProfile(static_cast<TH2*>(level3.second));
        _trackResidualProfiles[level1.first][level2.first][level3.first] = prof;
      }
    }
  }
  for (auto level1 : _trackSlopes) {
    for (auto level2 : level1.second) {
      for (auto level3 : level2.second) {
        prof = _makeProfile(static_cast<TH2*>(level3.second));
        _trackSlopeProfiles[level1.first][level2.first][level3.first] = prof;
      }
    }
  }
  _trackSlopeXVsSumResidualProfile = _makeProfile(static_cast<TH2*>(_trackSlopeXVsSumResiduals));
  _trackSlopeYVsSumResidualProfile = _makeProfile(static_cast<TH2*>(_trackSlopeYVsSumResiduals));
}

void SbtSimpleAlignmentAlg::generateStdDevPlots() {
  TH1* stdDev = nullptr;
  for (auto level1 : _trackResiduals) {
    for (auto level2 : level1.second) {
      for (auto level3 : level2.second) {
        std::stringstream hName;
        hName << level3.second->GetName() << "_StdDev";
        stdDev = new TH1D(hName.str().c_str(), hName.str().c_str(), level3.second->GetNbinsX(), level3.second->GetXaxis()->GetXmin(), level3.second->GetXaxis()->GetXmax());
        stdDev->GetXaxis()->SetTitle(level3.second->GetXaxis()->GetTitle());
        stdDev->GetYaxis()->SetTitle(convertToStdDev(level3.second->GetYaxis()->GetTitle()).c_str());
        stdDev->SetMarkerStyle(kFullSquare);
        stdDev->SetMarkerSize(0.7);
        stdDev->SetMarkerColor(kRed+2);
        stdDev->SetLineColor(kRed+2);
        _trackResidualStdDev[level1.first][level2.first][level3.first] = stdDev;
        level3.second->GetYaxis()->SetRange(1, level3.second->GetNbinsY());
        for (int ibin = 1; ibin <= level3.second->GetNbinsX(); ibin++) {
          level3.second->GetXaxis()->SetRange(ibin, ibin);
          double std_dev = level3.second->GetStdDev(2);
          double std_dev_err = level3.second->GetStdDevError(2);
          stdDev->SetBinContent(ibin, std_dev);
          stdDev->SetBinError(ibin, std_dev_err);
        }
        level3.second->GetXaxis()->SetRange(1, level3.second->GetNbinsX());
      }
    }
  }
  for (auto level1 : _trackSlopes) {
    for (auto level2 : level1.second) {
      for (auto level3 : level2.second) {
        std::stringstream hName;
        hName << level3.second->GetName() << "_StdDev";
        stdDev = new TH1D(hName.str().c_str(), hName.str().c_str(), level3.second->GetNbinsX(), level3.second->GetXaxis()->GetXmin(), level3.second->GetXaxis()->GetXmax());
        stdDev->GetXaxis()->SetTitle(level3.second->GetXaxis()->GetTitle());
        stdDev->GetYaxis()->SetTitle(convertToStdDev(level3.second->GetYaxis()->GetTitle()).c_str());
        stdDev->SetMarkerStyle(kFullSquare);
        stdDev->SetMarkerSize(0.7);
        stdDev->SetMarkerColor(kRed+2);
        stdDev->SetLineColor(kRed+2);
        _trackSlopeStdDev[level1.first][level2.first][level3.first] = stdDev;
        level3.second->GetYaxis()->SetRange(1, level3.second->GetNbinsY());
        for (int ibin = 1; ibin <= level3.second->GetNbinsX(); ibin++) {
          level3.second->GetXaxis()->SetRange(ibin, ibin);
          double std_dev = level3.second->GetStdDev(2);
          double std_dev_err = level3.second->GetStdDevError(2);
          stdDev->SetBinContent(ibin, std_dev);
          stdDev->SetBinError(ibin, std_dev_err);
        }
        level3.second->GetXaxis()->SetRange(1, level3.second->GetNbinsX());
      }
    }
  }
}

void SbtSimpleAlignmentAlg::alignPositionXYRotateZ(std::string criteria) {
for (auto itDet = _alignDetList.begin(); itDet != _alignDetList.end(); itDet++) {
    auto det = *itDet;
    std::cout << std::endl;
    std::cout << "Calculating alignment for detector " << det->GetID() << std::endl;
    TProfile* prof_x = nullptr;
    TProfile* prof_y = nullptr;
    for (auto level2 : _trackResidualProfiles[det]) {
      if (std::get<0>(level2.first) == criteria) {
        prof_x = static_cast<TProfile*>(level2.second["dx_y"]);
        prof_y = static_cast<TProfile*>(level2.second["dy_x"]);
        break;
      }
    }
    assert(prof_x && prof_y);
    TF1* fitx = new TF1("fitx", "[0] + [1] * x", prof_x->GetXaxis()->GetXmin(), prof_x->GetXaxis()->GetXmax());
    fitx->SetParNames("dx / cm", "sin(#theta_{z})");
    TF1* fity = new TF1("fity", "[0] + [1] * x", prof_y->GetXaxis()->GetXmin(), prof_y->GetXaxis()->GetXmax());
    fity->SetParNames("dy / cm", "-sin(#theta_{z})");
    std::cout << "Fitting dx vs y" << std::endl;
    prof_x->Fit(fitx,"0");
    std::cout << "Fitting dy vs x" << std::endl;
    prof_y->Fit(fity,"0");
    std::array<double, 6> pars = _alignGeom->getAlignPars(det->GetID());
    double cf = _corrFraction;
    if (TMath::Abs(fitx->GetParameter(0)) > fitx->GetParError(0)) pars[3] = -fitx->GetParameter(0) * cf;
    if (TMath::Abs(fity->GetParameter(0)) > fity->GetParError(0)) pars[4] = -fity->GetParameter(0) * cf;
    std::cout << "Angle dx vs y: " << fitx->GetParameter(1) << " +/- " << fitx->GetParError(1) << std::endl;
    std::cout << "Angle dy vs x: " << fity->GetParameter(1) << " +/- " << fity->GetParError(1) << std::endl;
    if (fitx->GetParameter(1) * fity->GetParameter(1) < 0) {
      double avgSin = (TMath::Abs(fity->GetParameter(1)) + TMath::Abs(fitx->GetParameter(1))) / 2;
      double avgSinErr = TMath::Sqrt(fity->GetParError(1) * fity->GetParError(1) + 
                                     fitx->GetParError(1) * fitx->GetParError(1)) / 2;
      if (TMath::Abs(avgSin) > avgSinErr) {
        pars[2] = TMath::ASin(avgSin) * TMath::RadToDeg() * cf;
        if (fitx->GetParameter(1) < 0) pars[2] *= -1;
      }
      else {
        pars[2] = 0;
      }
    }
    else {
      pars[2] = 0;
    }
    std::cout << "Rotation about Z: " << pars[2] << std::endl;
    if (itDet != _alignDetList.begin()) {
      _alignGeom->addAlignPars(det->GetID(), pars);
    }
  }
}

void SbtSimpleAlignmentAlg::buildHisto() {
  if (_debugLevel) std::cout << "SimpleAlignmentAlg::BuildHisto " << std::endl;

  int nglobald = 200;
  double maxglobald = getRoadWidth(getLoopIndex() - 1) * getRoadWidth(getLoopIndex() - 1) * TMath::Sqrt(_alignDetList.size());

  auto slpx_bins = _getSlopeBins(_slopeShiftX);
  auto slpy_bins = _getSlopeBins(_slopeShiftY);

  _trackSlopeDistribution = new TH2D("slpx_slpy_trackSlopes", "slpx_slpy_trackSlopes; slope x; slope y; counts", slpx_bins.size() - 1, slpx_bins.data(), slpy_bins.size() - 1, slpy_bins.data());
  _trackSlopeXVsSumResiduals = new TH2D("slpx_globaldx_trackResiduals", "slpx_globaldx_trackResiduals; slope x; sum of squared x residuals; counts", slpx_bins.size() - 1, slpx_bins.data(), nglobald, 0, maxglobald);
  _trackSlopeYVsSumResiduals = new TH2D("slpy_globaldy_trackResiduals", "slpy_globaldy_trackResiduals; slope y; sum of squared y residuals; counts", slpy_bins.size() - 1, slpy_bins.data(), nglobald, 0, maxglobald);

  std::stringstream hname;
  std::stringstream htitle;
  for (auto det : _alignDetList) {
    double xmin = det->GetDetectorType()->GetUActMin();
    double xmax = det->GetDetectorType()->GetUActMax();
    double ymin = det->GetDetectorType()->GetVActMin();
    double ymax = det->GetDetectorType()->GetVActMax();
    int nxbins = TMath::FloorNint((xmax - xmin) / det->GetDetectorType()->GetUpitch() / 4);
    int nybins = TMath::FloorNint((ymax - ymin) / det->GetDetectorType()->GetVpitch() / 4);
    double dmin = -getRoadWidth(getLoopIndex() - 1) * 1.5;
    double dmax = -dmin;
    int ndxbins = TMath::FloorNint(5. / det->GetDetectorType()->GetUpitch());
    int ndybins = TMath::FloorNint(5. / det->GetDetectorType()->GetVpitch());

    hname.str(""); hname.clear();
    htitle.str(""); htitle.clear();
    hname << "spacePointsXY_Det" << det->GetID();
    htitle << hname.str() << ";x_{local} (cm);y_{local} (cm); counts";
    TH2D* h_spacePointsXY = new TH2D(hname.str().c_str(), htitle.str().c_str(), nxbins, xmin, xmax, nybins, ymin, ymax);
    _spacePointsXY[det] = h_spacePointsXY;

    std::array<std::pair<std::string,bool(*)(const trackResidual& tr)>,1> criteria = {{
      std::make_pair("All",nullptr)
      }};

    for (auto criterion : criteria) {
      std::map<std::string, TH1*> h_trackSlopes;

      hname.str(""); hname.clear();
      htitle.str(""); htitle.clear();
      hname << "slpx_x_trackSlopes" << "_" << criterion.first << "_Det" << det->GetID();
      htitle << hname.str() << ";x_{local} (cm); slope x; counts";
      TH1* h_slpx_x_trackSlopes = new TH2D(hname.str().c_str(), htitle.str().c_str(), nxbins, xmin, xmax, slpx_bins.size() - 1, slpx_bins.data());
      h_trackSlopes["slpx_x"] = h_slpx_x_trackSlopes;

      hname.str(""); hname.clear();
      htitle.str(""); htitle.clear();
      hname << "slpx_y_trackSlopes" << "_" << criterion.first << "_Det" << det->GetID();
      htitle << hname.str() << ";y_{local} (cm); slope x; counts";
      TH1* h_slpx_y_trackSlopes = new TH2D(hname.str().c_str(), htitle.str().c_str(), nybins, ymin, ymax, slpx_bins.size() - 1, slpx_bins.data());
      h_trackSlopes["slpx_y"] = h_slpx_y_trackSlopes;

      hname.str(""); hname.clear();
      htitle.str(""); htitle.clear();
      hname << "slpy_x_trackSlopes" << "_" << criterion.first << "_Det" << det->GetID();
      htitle << hname.str() << ";x_{local} (cm); slope y; counts";
      TH1* h_slpy_x_trackSlopes = new TH2D(hname.str().c_str(), htitle.str().c_str(), nxbins, xmin, xmax, slpy_bins.size() - 1, slpy_bins.data());
      h_trackSlopes["slpy_x"] = h_slpy_x_trackSlopes;

      hname.str(""); hname.clear();
      htitle.str(""); htitle.clear();
      hname << "slpy_y_trackSlopes" << "_" << criterion.first << "_Det" << det->GetID();
      htitle << hname.str() << ";y_{local} (cm); slope y; counts";
      TH1* h_slpy_y_trackSlopes = new TH2D(hname.str().c_str(), htitle.str().c_str(), nybins, ymin, ymax, slpy_bins.size() - 1, slpy_bins.data());
      h_trackSlopes["slpy_y"] = h_slpy_y_trackSlopes;

      _trackSlopes[det][criterion] = h_trackSlopes;

      std::map<std::string, TH1*> h_trackResiduals;

      for (auto det2 : _alignDetList) {
        if (det->GetID() == det2->GetID()) continue;
        std::stringstream tag;

        tag << "dx" << det->GetID() << "_dx" << det2->GetID();
        hname.str(""); hname.clear();
        htitle.str(""); htitle.clear();
        hname << tag.str() << "_trackResiduals" << "_" << criterion.first << "_Det" << det->GetID();
        htitle << hname.str() << ";x_{" << det2->GetID() << "} - x_{" << det2->GetID() << ",track} (cm); x_{" << det->GetID() << "} - x_{" << det->GetID() << ",track} (cm);counts";
        TH2D* h_dx_dx_trackResiduals = new TH2D(hname.str().c_str(), htitle.str().c_str(), ndxbins / 2, dmin, dmax, ndxbins / 2, dmin, dmax);
        h_trackResiduals[tag.str()] = h_dx_dx_trackResiduals;

        tag.str("");
        tag.clear();
        tag << "dy" << det->GetID() << "_dy" << det2->GetID();
        hname.str(""); hname.clear();
        htitle.str(""); htitle.clear();
        hname << tag.str() << "_trackResiduals" << "_" << criterion.first << "_Det" << det->GetID();
        htitle << hname.str() << ";y_{" << det->GetID() << "} - y_{" << det->GetID() << ",track} (cm); y_{" << det2->GetID() << "} - y_{" << det2->GetID() << ",track} (cm);counts";
        TH2D* h_dy_dy_trackResiduals = new TH2D(hname.str().c_str(), htitle.str().c_str(), ndybins / 2, dmin, dmax, ndybins / 2, dmin, dmax);
        h_trackResiduals[tag.str()] = h_dy_dy_trackResiduals;
      }

      hname.str(""); hname.clear();
      htitle.str(""); htitle.clear();
      hname << "dx_x_trackResiduals" << "_" << criterion.first << "_Det" << det->GetID();
      htitle << hname.str() << ";x_{local} (cm); x - x_{track} (cm);counts";
      TH2D* h_dx_x_trackResiduals = new TH2D(hname.str().c_str(), htitle.str().c_str(), nxbins, xmin, xmax, ndxbins, dmin, dmax);
      h_trackResiduals["dx_x"] = h_dx_x_trackResiduals;

      hname.str(""); hname.clear();
      htitle.str(""); htitle.clear();
      hname << "dx_y_trackResiduals" << "_" << criterion.first << "_Det" << det->GetID();
      htitle << hname.str() << ";y_{local} (cm); x - x_{track} (cm);counts";
      TH2* h_dx_y_trackResiduals = new TH2D(hname.str().c_str(), htitle.str().c_str(), nybins, ymin, ymax, ndxbins, dmin, dmax);
      h_trackResiduals["dx_y"] = h_dx_y_trackResiduals;

      hname.str(""); hname.clear();
      htitle.str(""); htitle.clear();
      hname << "dy_x_trackResiduals" << "_" << criterion.first << "_Det" << det->GetID();
      htitle << hname.str() << ";x_{local} (cm); y - y_{track} (cm);counts";
      TH2* h_dy_x_trackResiduals = new TH2D(hname.str().c_str(), htitle.str().c_str(), nxbins, xmin, xmax, ndybins, dmin, dmax);
      h_trackResiduals["dy_x"] = h_dy_x_trackResiduals;

      hname.str(""); hname.clear();
      htitle.str(""); htitle.clear();
      hname << "dy_y_trackResiduals" << "_" << criterion.first << "_Det" << det->GetID();
      htitle << hname.str() << ";y_{local} (cm); y - y_{track} (cm);counts";
      TH2* h_dy_y_trackResiduals = new TH2D(hname.str().c_str(), htitle.str().c_str(), nybins, ymin, ymax, ndybins, dmin, dmax);
      h_trackResiduals["dy_y"] = h_dy_y_trackResiduals;

      hname.str(""); hname.clear();
      htitle.str(""); htitle.clear();
      hname << "dx_slpx_trackResiduals" << "_" << criterion.first << "_Det" << det->GetID();
      htitle << hname.str() << ";slope x; x - x_{track} (cm);counts";
      TH2* h_dx_slpx_trackResiduals = new TH2D(hname.str().c_str(), htitle.str().c_str(), slpx_bins.size() - 1, slpx_bins.data(), ndxbins, dmin, dmax);
      h_trackResiduals["dx_slpx"] = h_dx_slpx_trackResiduals;

      hname.str(""); hname.clear();
      htitle.str(""); htitle.clear();
      hname << "dx_slpy_trackResiduals" << "_" << criterion.first << "_Det" << det->GetID();
      htitle << hname.str() << ";slope y; x - x_{track} (cm);counts";
      TH2* h_dx_slpy_trackResiduals = new TH2D(hname.str().c_str(), htitle.str().c_str(), slpy_bins.size() - 1, slpy_bins.data(), ndxbins, dmin, dmax);
      h_trackResiduals["dx_slpy"] = h_dx_slpy_trackResiduals;

      hname.str(""); hname.clear();
      htitle.str(""); htitle.clear();
      hname << "dy_slpx_trackResiduals" << "_" << criterion.first << "_Det" << det->GetID();
      htitle << hname.str() << ";slope x; y - y_{track} (cm);counts";
      TH2* h_dy_slpx_trackResiduals = new TH2D(hname.str().c_str(), htitle.str().c_str(), slpx_bins.size() - 1, slpx_bins.data(), ndybins, dmin, dmax);
      h_trackResiduals["dy_slpx"] = h_dy_slpx_trackResiduals;

      hname.str(""); hname.clear();
      htitle.str(""); htitle.clear();
      hname << "dy_slpy_trackResiduals" << "_" << criterion.first << "_Det" << det->GetID();
      htitle << hname.str() << ";slope y; y - y_{track} (cm);counts";
      TH2* h_dy_slpy_trackResiduals = new TH2D(hname.str().c_str(), htitle.str().c_str(), slpy_bins.size() - 1, slpy_bins.data(), ndybins, dmin, dmax);
      h_trackResiduals["dy_slpy"] = h_dy_slpy_trackResiduals;

      _trackResiduals[det][criterion] = h_trackResiduals;
    }
  }
  makeProfiles();
}

void SbtSimpleAlignmentAlg::_fillHistoSlopes(histogram_map& histograms, const trackResidual& tr, const SbtTrack& t, const SbtSpacePoint& sp) {
  for (auto criteria_histos : histograms[sp.GetDetectorElem()]) {
    if (!criteria_histos.first.second || criteria_histos.first.second(tr)) {
      int detId = sp.GetDetectorElem()->GetID();
      std::map<std::string,TH1*>* histos = &criteria_histos.second;
      (*histos).at("slpx_x")->Fill(tr._x.at(detId), tr._slpx);
      (*histos).at("slpx_y")->Fill(tr._y.at(detId), tr._slpx);
      (*histos).at("slpy_x")->Fill(tr._x.at(detId), tr._slpy);
      (*histos).at("slpy_y")->Fill(tr._y.at(detId), tr._slpy);
    }
  }
}

void SbtSimpleAlignmentAlg::_fillHistoResiduals(histogram_map& histograms, const trackResidual& tr, const SbtTrack& t, const SbtSpacePoint& sp) {
  for (auto criteria_histos : histograms[sp.GetDetectorElem()]) {
    if (!criteria_histos.first.second || criteria_histos.first.second(tr)) {
      int detId = sp.GetDetectorElem()->GetID();
      std::map<std::string,TH1*>* histos = &criteria_histos.second;
      try {
        (*histos).at("dx_y")->Fill(tr._y.at(detId), tr._dx.at(detId));
        (*histos).at("dy_y")->Fill(tr._y.at(detId), tr._dy.at(detId));
        (*histos).at("dx_slpx")->Fill(tr._slpx, tr._dx.at(detId));
        (*histos).at("dy_slpx")->Fill(tr._slpx, tr._dy.at(detId));
        (*histos).at("dx_x")->Fill(tr._x.at(detId), tr._dx.at(detId));
        (*histos).at("dy_x")->Fill(tr._x.at(detId), tr._dy.at(detId));
        (*histos).at("dx_slpy")->Fill(tr._slpy, tr._dx.at(detId));
        (*histos).at("dy_slpy")->Fill(tr._slpy, tr._dy.at(detId));
        for (auto sp2 : t.GetSpacePointList()) {
          if (sp2->GetDetectorElem() == sp.GetDetectorElem()) continue;
          int detId2 = sp2->GetDetectorElem()->GetID();
          std::stringstream tag;
          tag << "dx" << detId << "_dx" << detId2;
          (*histos).at(tag.str())->Fill(tr._dx.at(detId2), tr._dx.at(detId));
          tag.str("");
          tag.clear();
          tag << "dy" << detId << "_dy" << detId2;
          (*histos).at(tag.str())->Fill(tr._dy.at(detId2), tr._dy.at(detId));
        }
      }
      catch (std::out_of_range ex) {
        std::cout << "Error in SbtSimpleAlignmentAlg::_fillHistoResiduals" << std::endl << ex.what() << std::endl;
        for (auto p : *histos) {
          std::cout << p.first << std::endl;
        }
      }
    }
  }
}

void SbtSimpleAlignmentAlg::fillHisto() {
  // consider only events with 1 track for alignment
  if (_currentEvent->GetTrackList().size() != 1) return;

  // loop on space points
  for (const auto& sp : _currentEvent->GetSpacePointList()) {
    _spacePointsXY[sp.GetDetectorElem()]->Fill(sp.GetXPosition(), sp.GetYPosition());
  }

  // loop on tracks
  for (const auto& t : _currentEvent->GetTrackList()) {
    double slpx = t.GetSlopeX();
    double slpy = t.GetSlopeY();
    _trackSlopeDistribution->Fill(slpx, slpy);
    trackResidual tr;
    tr._slpx = slpx;
    tr._slpy = slpy;
    for (const auto sp : t.GetSpacePointList()) {
      double x = sp->GetXPosition();
      double y = sp->GetYPosition();
      double z = sp->GetZPosition();
      double master[3] = {x, y, z};
      double local[3] = {0};
      sp->GetDetectorElem()->MasterToLocal(master, local);
      double& x_local = local[0];
      double& y_local = local[1];
      double& z_local = local[2];
      int detId = sp->GetDetectorElem()->GetID();
      tr._x[detId] = x_local;
      tr._y[detId] = y_local;
      std::array<double,2> d;
      t.ResidualsOnWafer(*sp, d);
      tr._dx[detId] = d[0];
      tr._dy[detId] = d[1];
    }
    double tot_x_res =  tr.get_total_x_residuals();
    double tot_y_res =  tr.get_total_y_residuals();
    _trackSlopeXVsSumResiduals->Fill(slpx, tot_x_res);
    _trackSlopeYVsSumResiduals->Fill(slpy, tot_y_res);
    _trackSlopeXVsSumResidualProfile->Fill(slpx, tot_x_res);
    _trackSlopeYVsSumResidualProfile->Fill(slpy, tot_y_res);
    for (auto sp : t.GetSpacePointList()) {
      _fillHistoResiduals(_trackResiduals, tr, t, *sp);
      _fillHistoResiduals(_trackResidualProfiles, tr, t, *sp);
      _fillHistoSlopes(_trackSlopes, tr, t, *sp);
      _fillHistoSlopes(_trackSlopeProfiles, tr, t, *sp);
    }
  }
}

std::vector<TList*> SbtSimpleAlignmentAlg::getHistograms() const {
  std::cout << "SbtSimpleAlignmentAlg::getHistograms: start." << std::endl;

  std::map<int, TList*> result_map;

  for (auto det : _alignDetList) {
    std::stringstream name;
    name << "Det_" << det->GetID();
    TList* list = new TList();
    list->SetName(name.str().c_str());
    result_map[det->GetID()] = list;
  }
  
  for (auto h : _spacePointsXY) {
    result_map[h.first->GetID()]->Add(h.second);
  }

  for (auto h1 : _trackResiduals) {
    for (auto h2 : h1.second) {
      for (auto h3 : h2.second) {
        result_map[h1.first->GetID()]->Add(h3.second);
      }
    }
  }

  for (auto h1 : _trackResidualProfiles) {
    for (auto h2 : h1.second) {
      for (auto h3 : h2.second) {
        result_map[h1.first->GetID()]->Add(h3.second);
      }
    }
  }

  for (auto h1 : _trackResidualStdDev) {
    for (auto h2 : h1.second) {
      for (auto h3 : h2.second) {
        result_map[h1.first->GetID()]->Add(h3.second);
      }
    }
  }

  for (auto h1 : _trackSlopes) {
    for (auto h2 : h1.second) {
      for (auto h3 : h2.second) {
        result_map[h1.first->GetID()]->Add(h3.second);
      }
    }
  }

  for (auto h1 : _trackSlopeProfiles) {
    for (auto h2 : h1.second) {
      for (auto h3 : h2.second) {
        result_map[h1.first->GetID()]->Add(h3.second);
      }
    }
  }

  for (auto h1 : _trackSlopeStdDev) {
    for (auto h2 : h1.second) {
      for (auto h3 : h2.second) {
        result_map[h1.first->GetID()]->Add(h3.second);
      }
    }
  }

  TList* global_plots = new TList();
  global_plots->SetName("Global");
  global_plots->Add(_trackSlopeDistribution);
  global_plots->Add(_trackSlopeXVsSumResiduals);
  global_plots->Add(_trackSlopeXVsSumResidualProfile);
  global_plots->Add(_trackSlopeYVsSumResiduals);
  global_plots->Add(_trackSlopeYVsSumResidualProfile);

  std::vector<TList*> result;
  for (auto list : result_map) result.push_back(list.second);
  result.push_back(global_plots);

  std::cout << "SbtSimpleAlignmentAlg::getHistograms: done." << std::endl;

  return result;
}

std::vector<TCanvas*> SbtSimpleAlignmentAlg::makePlots(int iteration) {
  generateStdDevPlots();

  std::vector<TCanvas*> result;
  std::stringstream cname;
  TProfile* prof = nullptr;

  cname << "TrackSlopes" << "_Iter" << iteration;
  TCanvas* cSlopes = new TCanvas(cname.str().c_str(), cname.str().c_str());
  result.push_back(cSlopes);
  cSlopes->cd();
  _trackSlopeDistribution->Draw("colz");

  double leftmargin = 0.2;
  double topmargin = 0.15;
  double bottommargin = 0.1;
  double rightmargin = 0.05;
  TVirtualPad* pad = nullptr;

  cname.str(""); cname.clear();
  cname << "TrackTotResidualsVsSlope" << "_Iter" << iteration;
  TCanvas* cResSlp = new TCanvas(cname.str().c_str(), cname.str().c_str(), 800, 400);
  result.push_back(cResSlp);
  cResSlp->Divide(2, 1);
  pad = cResSlp->cd(1);
  pad->SetGridx();
  pad->SetGridy();
  pad->SetMargin(leftmargin, rightmargin, bottommargin, topmargin);
  _trackSlopeXVsSumResiduals->Draw("colz");
  pad = cResSlp->cd(2);
  pad->SetGridx();
  pad->SetGridy();
  pad->SetMargin(leftmargin, rightmargin, bottommargin, topmargin);
  _trackSlopeYVsSumResiduals->Draw("colz");

  cname.str(""); cname.clear();
  cname << "TrackTotResidualsVsSlopeProfile" << "_Iter" << iteration;
  TCanvas* cResSlpProf = new TCanvas(cname.str().c_str(), cname.str().c_str(), 800, 400);
  result.push_back(cResSlpProf);
  cResSlpProf->Divide(2, 1);
  pad = cResSlpProf->cd(1);
  pad->SetGridx();
  pad->SetGridy();
  pad->SetMargin(leftmargin, rightmargin, bottommargin, topmargin);
  _trackSlopeXVsSumResidualProfile->Draw();
  adjustAxis(_trackSlopeXVsSumResidualProfile, false);
  pad = cResSlpProf->cd(2);
  pad->SetGridx();
  pad->SetGridy();
  pad->SetMargin(leftmargin, rightmargin, bottommargin, topmargin);
  _trackSlopeYVsSumResidualProfile->Draw();
  adjustAxis(_trackSlopeYVsSumResidualProfile, false);

  for (auto entry : _spacePointsXY) {
    cname.str(""); cname.clear();
    cname << "SpacePoints" << "_Det" << entry.first->GetID() << "_Iter" << iteration;
    TCanvas* cSP = new TCanvas(cname.str().c_str(), cname.str().c_str());
    result.push_back(cSP);
    cSP->cd();
    entry.second->Draw("colz");
  }

  for (auto level1 : _trackSlopes) {
    for (auto level2 : level1.second) {
      cname.str(""); cname.clear();
      cname << "TrackSlopesVsXY" << "_" << level2.first.first << "_Det" << level1.first->GetID() << "_Iter" << iteration;
      TCanvas* cSlopesXY = plot4(cname.str(), level2.second, {"slpx_x", "slpy_y", "slpy_x", "slpx_y"}, "colz");
      result.push_back(cSlopesXY);

      cname.str(""); cname.clear();
      cname << "TrackSlopesProfileVsXY" << "_" << level2.first.first << "_Det" << level1.first->GetID() << "_Iter" << iteration;
      TCanvas* cSlopesProfXY = plot4(cname.str(), _trackSlopeProfiles[level1.first][level2.first], {"slpx_x", "slpy_y", "slpy_x", "slpx_y"}, "");
      result.push_back(cSlopesProfXY);

      cname.str(""); cname.clear();
      cname << "TrackSlopesStdDevVsXY" << "_" << level2.first.first << "_Det" << level1.first->GetID() << "_Iter" << iteration;
      TCanvas* cSlopesStdDevXY = plot4(cname.str(), _trackSlopeStdDev[level1.first][level2.first], {"slpx_x", "slpy_y", "slpy_x", "slpx_y"}, "");
      result.push_back(cSlopesStdDevXY);
    }
  }

  for (auto level1 : _trackResiduals) {
    for (auto level2 : level1.second) {
      cname.str(""); cname.clear();
      cname << "TrackResidualsVsXY" << "_" << level2.first.first << "_Det" << level1.first->GetID() << "_Iter" << iteration;
      TCanvas* cTRXY = plot4(cname.str(), level2.second, {"dx_x", "dy_y", "dy_x", "dx_y"}, "colz");
      result.push_back(cTRXY);

      cname.str(""); cname.clear();
      cname << "TrackResidualsProfileVsXY" << "_" << level2.first.first << "_Det" << level1.first->GetID() << "_Iter" << iteration;
      TCanvas* cProfTRXY = plot4(cname.str(), _trackResidualProfiles[level1.first][level2.first], {"dx_x", "dy_y", "dy_x", "dx_y"}, "", &drawFitResults);
      result.push_back(cProfTRXY);

      cname.str(""); cname.clear();
      cname << "TrackResidualsStdDevVsXY" << "_" << level2.first.first << "_Det" << level1.first->GetID() << "_Iter" << iteration;
      TCanvas* cStdDevTRXY = plot4(cname.str(), _trackResidualStdDev[level1.first][level2.first], {"dx_x", "dy_y", "dy_x", "dx_y"}, "");
      result.push_back(cStdDevTRXY);

      cname.str(""); cname.clear();
      cname << "TrackResidualsVsSlopesXY" << "_" << level2.first.first << "_Det" << level1.first->GetID() << "_Iter" << iteration;
      TCanvas* cTRSlpXY = plot4(cname.str(), level2.second, {"dx_slpx", "dy_slpy", "dy_slpx", "dx_slpy"}, "colz");
      result.push_back(cTRSlpXY);

      cname.str(""); cname.clear();
      cname << "TrackResidualsProfileVsSlopesXY" << "_" << level2.first.first << "_Det" << level1.first->GetID() << "_Iter" << iteration;
      TCanvas* cProfTRSlpXY = plot4(cname.str(), _trackResidualProfiles[level1.first][level2.first], {"dx_slpx", "dy_slpy", "dy_slpx", "dx_slpy"}, "", &drawFitResults);
      result.push_back(cProfTRSlpXY);

      cname.str(""); cname.clear();
      cname << "TrackResidualsStdDevVsSlopesXY" << "_" << level2.first.first << "_Det" << level1.first->GetID() << "_Iter" << iteration;
      TCanvas* cStdDevTRSlpXY = plot4(cname.str(), _trackResidualStdDev[level1.first][level2.first], {"dx_slpx", "dy_slpy", "dy_slpx", "dx_slpy"}, "");
      result.push_back(cStdDevTRSlpXY);

      std::set<std::string> tags;
      for (auto det : _alignDetList) {
        if (det->GetID() == level1.first->GetID()) continue;
        std::stringstream tag;
        tag << "dx" << level1.first->GetID() << "_dx" << det->GetID();
        tags.insert(tag.str());
        tag.str("");
        tag.clear();
        tag << "dy" << level1.first->GetID() << "_dy" << det->GetID();
        tags.insert(tag.str());
      }
      cname.str(""); cname.clear();
      cname << "TrackResidualsVsResiduals" << "_" << level2.first.first << "_Det" << level1.first->GetID() << "_Iter" << iteration;
      TCanvas* cTR2 = plot4(cname.str(), level2.second, tags, "colz", nullptr, 2, 2);
      result.push_back(cTR2);

      cname.str(""); cname.clear();
      cname << "TrackResidualsProfileVsResiduals" << "_" << level2.first.first << "_Det" << level1.first->GetID() << "_Iter" << iteration;
      TCanvas* cProfTR2 = plot4(cname.str(), _trackResidualProfiles[level1.first][level2.first], tags, "", nullptr, 2, 2);
      result.push_back(cProfTR2);
    }
  }
  return result;
}

std::string SbtSimpleAlignmentAlg::convertToMean(std::string obs) {
  std::stringstream result;
  size_t ipos = obs.find('(');
  if (ipos != std::string::npos) {
    result << "<" << obs.substr(0, ipos - 1) << ">" << obs.substr(ipos - 1);
  }
  else {
    result << "<" << obs << ">";
  }
  return result.str();
}

std::string SbtSimpleAlignmentAlg::convertToStdDev(std::string obs) {
  std::stringstream result;
  size_t ipos = obs.find('(');
  if (ipos != std::string::npos) {
    result << "#sigma(" << obs.substr(0, ipos - 1) << ")" << obs.substr(ipos - 1);
  }
  else {
    result << "#sigma(" << obs << ")";
  }
  return result.str();
}

void SbtSimpleAlignmentAlg::drawFitResults(TH1* h) {
  double x = 0.10;
  double y = 0.98;
  if (h->GetListOfFunctions()->GetSize() > 0) {
    TF1* f = dynamic_cast<TF1*>(h->GetListOfFunctions()->At(0));
    if (f) {
      f->SetLineStyle(2);
      f->SetLineColor(kRed);
      f->SetLineWidth(2);
      f->Draw("same");
      double h = f->GetNpar() * 0.05;
      TPaveText* text = new TPaveText(x, y, x + 0.4, y - h, "brNDC");
      text->SetTextAlign(11);
      text->SetBorderSize(0);
      text->SetFillStyle(0);
      for (int i = 0; i < f->GetNpar(); i++) {
        std::stringstream line;
        line << f->GetParName(i) << " = " << f->GetParameter(i) << " +/- " << f->GetParError(i);
        text->AddText(line.str().c_str());
      }
      text->Draw();
    }
  }
}

bool SbtSimpleAlignmentAlg::isTrackStraight(const trackResidual& tr)
{
  double limit = 0.0002;
  if (tr._slpx > -limit && tr._slpx < limit && tr._slpy > -limit && tr._slpy < limit) return true;
  else return false;
}

bool SbtSimpleAlignmentAlg::isTrackNonStraightX(const trackResidual& tr) {
  double limit = 0.0004;
  if (tr._slpx > limit || tr._slpx < -limit) return true;
  else return false;
}

bool SbtSimpleAlignmentAlg::isTrackNonStraightY(const trackResidual& tr) {
  double limit = 0.00004;
  if (tr._slpy > limit || tr._slpy < -limit) return true;
  else return false;
}

bool SbtSimpleAlignmentAlg::isOutlier_DXSlpX_Det0(const trackResidual& tr) {
  double min_slp = -0.0009, max_slp = 0.0001;
  if (tr._slpx > min_slp && tr._slpx < max_slp) return false;
  double min_d = -1.0 / 200.0 - 25.0 / 4.0 * tr._slpx;
  double max_d = -25.0 / 4.0 * tr._slpx;
  if (tr._dx.at(0) > min_d && tr._dx.at(0) < max_d) return true;
  else return false;
}

template<typename KEYTYPE>
TCanvas* SbtSimpleAlignmentAlg::plot4(std::string name, std::map<KEYTYPE, TH1*>& histograms, std::set<KEYTYPE> keys, std::string drawOpt, void(*func)(TH1*), int fx, int fy) {
  double leftmargin = 0.2;
  double topmargin = 0.15;
  double bottommargin = 0.1;
  double rightmargin = 0.05;
  TVirtualPad* pad = nullptr;
  TProfile* prof = nullptr;
  
  int ncols = TMath::CeilNint(TMath::Sqrt(keys.size()));
  int nrows = TMath::FloorNint(1.0 * keys.size() / ncols);

  TCanvas* canvas = new TCanvas(name.c_str(),name.c_str(), 400 * ncols, 400 * nrows);
  canvas->Divide(ncols, nrows);
  int ic = 1;
  for (auto k : keys) {
    pad = canvas->cd(ic);
    ++ic;
    pad->SetMargin(leftmargin, rightmargin, bottommargin, topmargin);
    pad->SetGridx();
    pad->SetGridy();
    TH1* hist = nullptr;
    try {
      hist = histograms.at(k);
    }
    catch (std::out_of_range ex) {
      std::cout << "Error: could not find key '" << k << "' while drawing '" << name << "'" << std::endl;
    }
    if (!hist) continue;
    hist->Draw(drawOpt.c_str());
    if (hist->GetDimension() == 1) {
      adjustAxis(hist);
    }
    else {
      adjustAxis2D(hist, fx, fy);
    }
    if (func) func(hist);
  }
  return canvas;
}

void SbtSimpleAlignmentAlg::adjustAxis2D(TH1* hist, int fx, int fy) {
  double xmax = 0, ymax = 0;
  if (fx > 0) {
    hist->GetXaxis()->SetRange(1, hist->GetNbinsX());
    for (int i = 1; i <= hist->GetNbinsY(); i++) {
      hist->GetYaxis()->SetRange(i, i);
      double mean = hist->GetMean(1);
      double stddev = hist->GetStdDev(1);
      if (mean < 0) {
        mean = -mean + stddev;
      }
      else {
        mean += stddev;
      }
      if (mean > xmax) xmax = mean;
    }
    xmax *= fx;
  }
  if (fy > 0) {
    hist->GetYaxis()->SetRange(1, hist->GetNbinsY());
    for (int i = 1; i <= hist->GetNbinsX(); i++) {
      hist->GetXaxis()->SetRange(i, i);
      double mean = hist->GetMean(2);
      double stddev = hist->GetStdDev(2);
      if (mean < 0) {
        mean = -mean + stddev;
      }
      else {
        mean += stddev;
      }
      if (mean > ymax) ymax = mean;
    }
    ymax *= fy;
  }
  if (xmax > 0) {
    hist->GetXaxis()->SetRangeUser(-xmax, xmax);
  }
  else {
    hist->GetXaxis()->SetRange(1, hist->GetNbinsX());
  }
  if (ymax > 0) {
    hist->GetYaxis()->SetRangeUser(-ymax, ymax);
  }
  else {
    hist->GetYaxis()->SetRange(1, hist->GetNbinsY());
  }
}

void SbtSimpleAlignmentAlg::adjustAxis(TH1* hist, bool sym) {
  double min = FLT_MAX;
  double max = -FLT_MAX;
  int imin = -1;
  int imax = -1;
  double avgErr = 0;
  int nbins = 0;
  for (int i = 1; i <= hist->GetNbinsX(); i++) {
    if (hist->GetBinContent(i) == 0) continue;
    if (hist->GetBinError(i) / hist->GetBinContent(i) > 2) continue;
    avgErr += hist->GetBinError(i);
    nbins++;
  }
  avgErr /= nbins;
  for (int i = 1; i <= hist->GetNbinsX(); i++) {
    if (hist->GetBinContent(i) == 0) continue;
    if (hist->GetBinError(i) > 10 * avgErr) continue;
    double vlow = hist->GetBinContent(i);
    double vhigh = hist->GetBinContent(i);
    if (hist->GetBinError(i) < 5 * avgErr) {
      vlow -= hist->GetBinError(i);
      vhigh += hist->GetBinError(i);
    }
    if (vlow < min) min = vlow;
    if (vhigh > max) max = vhigh;
  }

  double d = max - min;
  max += d * 0.2;
  min -= d * 0.2;

  if (sym) {
    if (min > -max) {
      min = -max;
    }
    else {
      max = -min;
    }
  }
  else {
    min = 0;
  }

  hist->SetMinimum(min);
  hist->SetMaximum(max);
}

const std::vector<double>& SbtSimpleAlignmentAlg::_getSlopeBins() {
  static std::vector<double> bins;
  if (bins.size() == 0) {
    const double binW1 = 4e-6;
    const int nbins1 = 25;
    bins.push_back(0);
    for (int i = 1; i < nbins1; i++) {
      double v = binW1 * i;
      bins.push_back(v);
      bins.push_back(-v);
    }
    const double binW2 = 20e-6;
    const int nbins2 = 5;
    for (int i = 0; i < nbins2; i++) {
      double v = binW2 * i + binW1 * nbins1;
      bins.push_back(v);
      bins.push_back(-v);
    }
    const double binW3 = 160e-6;
    const int nbins3 = 0;
    for (int i = 0; i <= nbins3; i++) {
      double v = binW3 * i + binW2 * nbins2 + binW1 * nbins1;
      bins.push_back(v);
      bins.push_back(-v);
    }
    std::sort(bins.begin(), bins.end());
  }
  return bins;
}

std::vector<double> SbtSimpleAlignmentAlg::_getSlopeBins(double shift) {
  std::vector<double> result = _getSlopeBins();
  for (auto& v : result) v += shift;
  return result;
}