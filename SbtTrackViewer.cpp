// class to visualize tracks
// you can feed a single track, an event or a list of events

#include <algorithm>
#include <cstdlib>

#include <TPad.h>
#include <TPaveText.h>
#include <TPolyMarker3D.h>
#include <TString.h>
#include <TView.h>
#include <TView3D.h>

#include "SbtDef.h"
#include "SbtDetectorElem.h"
#include "SbtSpacePoint.h"
#include "SbtTrackViewer.h"

ClassImp(SbtTrackViewer);

SbtTrackViewer::SbtTrackViewer(SbtConfig* aConfiguration, int debuglevel)
    : _DebugLevel(debuglevel) {
  std::cout << "SbtTrackViewer is starting... (debuglevel = " << _DebugLevel
            << " )\n";
  _theConfiguration = aConfiguration;
  if (_theConfiguration != nullptr) {
    _theGeoManager = _theConfiguration->getGeoManager();
  } else {
    std::cerr << "FATAL: NULL Configurator in SbtTrackViewer c'tor.\n";
    std::cerr << "Exiting now... \n";
    exit(2);
  }

  // clearing up canvases
  for (unsigned int i = 0; i < _theCanvases.size(); i++) {
    TCanvas* c = _theCanvases[i];
    delete c;
    c = nullptr;
  }
  _theCanvases.clear();
  _nCanvases = 0;
}

SbtTrackViewer::~SbtTrackViewer() {
  // clearing up canvases
  for (unsigned int i = 0; i < _theCanvases.size(); i++) {
    TCanvas* c = _theCanvases[i];
    delete c;
    c = nullptr;
  }
  _theCanvases.clear();
  ClearTracks();
}

TCanvas* SbtTrackViewer::DrawTracks(const std::vector<SbtTrack>& aListOfTracks, int trackId, int pid, Color_t color) {
  std::vector<TGeoTrack*> listOfGeoTracks;
  if (_DebugLevel)
    std::cout << "\tTotal # of tracks " << aListOfTracks.size() << "\n";
  for (auto& track : aListOfTracks) {
    TGeoTrack* aGeoTrack = new TGeoTrack(trackId, pid);
    if (_DebugLevel > 0) {
      std::cout << "\t\tTotal points # " << track.GetSpacePointList().size() << "\n";
    }
    int iPoint = 0;
    for (auto aPoint : track.GetSpacePointList()) {
      double x = aPoint->GetXPosition();
      double y = aPoint->GetYPosition();
      double z = aPoint->GetZPosition();
      double t = iPoint * eps;
      aGeoTrack->AddPoint(x, y, z, t);
      ++iPoint;
    }
    aGeoTrack->SetLineColor(color);
    listOfGeoTracks.push_back(aGeoTrack);
  }
  TCanvas* c = DrawCanvas(listOfGeoTracks);
  return c;
}

void SbtTrackViewer::DrawEvent(SbtEvent* anEvent, int pid) {
  TCanvas* c = DrawTracks( anEvent->GetTrackList(), 0, pid, kBlue);  // let's use blue for data
  if (c) c->cd();
  TPaveText* pT = new TPaveText(0.5, 0.5, 0.9, 0.9);
  TString details("");
  details.Form("DATA: Run # %d    Event # %d", anEvent->GetRunNumber(), anEvent->GetEventNumber());
  pT->AddText(details.Data());
  pT->Draw();
  ((gPad->GetView()))->SetParallel();
  TString save("");
  save.Form("plots/EvtDsply_Run_%d_Event_%d.pdf", anEvent->GetRunNumber(), anEvent->GetEventNumber());
  c->Print(save.Data());
}

void SbtTrackViewer::DrawListOfEvents(std::vector<SbtEvent*> aListOfEvents,
                                      int pid) {
  for (UInt_t iEvt = 0; iEvt < aListOfEvents.size(); iEvt++) {
    if (_DebugLevel) std::cout << "Event # " << iEvt << "\n";
    SbtEvent* anEvent = aListOfEvents[iEvt];
    DrawEvent(anEvent, pid);
  }
}

TCanvas* SbtTrackViewer::DrawCanvas(std::vector<TGeoTrack*> listOfGeoTracks) {
  char cName[100];
  sprintf(cName, "Viewer_%d", _nCanvases);
  TCanvas* c = new TCanvas(cName, cName, 800, 600);
  c->cd();
  gGeoManager->GetTopVolume()->Draw();
  std::cout << "\t\tDrawing " << listOfGeoTracks.size() << " tracks...\n";
  for (UInt_t i = 0; i < listOfGeoTracks.size(); i++) {
    listOfGeoTracks[i]->Draw();
  }
  _theCanvases.push_back(c);
  _nCanvases++;
  return _theCanvases[_nCanvases - 1];
}

void SbtTrackViewer::DrawMCEvent(SbtEvent* anEvent, int pid) {
  TCanvas* c = DrawTracks(anEvent->GetMCTrackList(), 0, pid, kRed);  // let's use red for MC
  if (c) c->cd();
  TPaveText* pT = new TPaveText(0.5, 0.5, 0.9, 0.9);
  TString details("");
  details.Form("MC  : Run # %d    Event # %d", anEvent->GetRunNumber(), anEvent->GetEventNumber());
  pT->AddText(details.Data());
  pT->Draw();
  ((gPad->GetView()))->SetParallel();
  TString save("");
  save.Form("plots/EvtDsply_MC_Run_%d_Event_%d.pdf", anEvent->GetRunNumber(), anEvent->GetEventNumber());
  c->Print(save.Data());
}

void SbtTrackViewer::DrawListOfMCEvents(std::vector<SbtEvent*> aListOfEvents, int pid) {
  for (unsigned int iEvt = 0; iEvt < aListOfEvents.size(); iEvt++) {
    if (_DebugLevel) std::cout << "Event # " << iEvt << "\n";
    SbtEvent* anEvent = aListOfEvents[iEvt];
    DrawMCEvent(anEvent, pid);
  }
}

TCanvas* SbtTrackViewer::DrawNtpTracks(double* ItpX, double* SlpX,
                                       double* ItpY, double* SlpY,
                                       int ntrk, double* spXPos,
                                       double* spYPos, double* spZPos,
                                       int nSP, int pid, int jentry) {
  std::vector<SbtDetectorElem*> detectorElems = _theConfiguration->getDetectorElemList();
  std::sort(detectorElems.begin(), detectorElems.end(), SbtDetectorElem::ltz);
  std::vector<TGeoTrack*> listOfGeoTracks;
  int nDets = detectorElems.size();

  if (ntrk == 0) return nullptr;

  TGeoTrack* aGeoTrack = nullptr;

  for (int iTrk = 0; iTrk < ntrk; iTrk++) {
    aGeoTrack = new TGeoTrack(iTrk, pid);
    for (int iDet = 0; iDet < nDets; iDet++) {
      SbtDetectorElem* aDet = detectorElems[iDet];
      if (aDet) {
        double z = aDet->GetZPos();
        double x = ItpX[iTrk] + SlpX[iTrk] * z;
        double y = ItpY[iTrk] + SlpY[iTrk] * z;
        double t = iDet * eps;
        aGeoTrack->AddPoint(x, y, z, t);
      }
    }
    listOfGeoTracks.push_back(aGeoTrack);
  }
  TCanvas* c = DrawCanvas(listOfGeoTracks);
  if (c) c->cd();
  ((gPad->GetView()))->SetParallel();
  TPaveText* pT = new TPaveText(0.6, 0.6, 0.9, 0.9);
  pT->SetName("info");
  TString details("");
  details.Form("DATA: Event # %d", jentry);
  pT->AddText(details.Data());
  pT->Draw();
  ((gPad->GetView()))->SetParallel();

  TPolyMarker3D* sp3d = new TPolyMarker3D(nSP);

  for (int iSP = 0; iSP < nSP; iSP++) {
    sp3d->SetPoint(iSP, spXPos[iSP], spYPos[iSP], spZPos[iSP]);
    sp3d->SetMarkerSize(2);
    sp3d->SetMarkerColor(4);
    sp3d->SetMarkerStyle(5);
  }

  // draw
  sp3d->Draw();

  return c;
}
