#ifndef SBTTRACKVIEWER_HH
#define SBTTRACKVIEWER_HH

#include <vector>

#include <TCanvas.h>
#include <TGeoManager.h>
#include <TGeoTrack.h>

#include "SbtConfig.h"
#include "SbtEvent.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"

class SbtTrackViewer {
 public:
  // c'tor needs a working configuration
  SbtTrackViewer(SbtConfig* aConfiguration, int debuglevel = 0);
  ~SbtTrackViewer();

  // drawing tracks
  void DrawListOfEvents(std::vector<SbtEvent*> aListOfEvents, int pid);
  void DrawEvent(SbtEvent* anEvent, int pid);
  void DrawMCEvent(SbtEvent* anEvent, int pid);
  void DrawListOfMCEvents(std::vector<SbtEvent*> aListOfEvents, int pid);

  TCanvas* DrawNtpTracks(double* ItpX, double* SlpX, double* ItpY,
                         double* SlpY, int ntrk, double* spXPos,
                         double* spYPos, double* spZPos, int nSP,
                         int pid, int jentry);

  TCanvas* DrawTracks(const std::vector<SbtTrack>& listOfTracks, int trackId, int pid, Color_t color = kRed);

  // clearing tracks
  inline void ClearTracks() { _theGeoManager->ClearTracks(); }

 protected:
  SbtConfig* _theConfiguration;  // the configuration
  TGeoManager* _theGeoManager;   // the geometry
  int _DebugLevel;
  std::vector<TCanvas*> _theCanvases;  // the canvases to be plotted
  int _nCanvases;                      // total number of canvases
  TCanvas* DrawCanvas(std::vector<TGeoTrack*> listOfTracks);  // method that draws the tracks

  ClassDef(SbtTrackViewer, 1);
};

#endif
