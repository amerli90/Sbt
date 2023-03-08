
#ifndef SBTEVENT_HH
#define SBTEVENT_HH

// the headers of the Events information (hit, track, etc...)
#include "SbtCluster.h"
#include "SbtDigi.h"
#include "SbtHit.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"
#include "SbtTriggerInfo.h"

// a pure header with some constants
#include "SbtDef.h"

#include <vector>

class SbtEvent {
  /*
   * The Event will keep track of all the information of the Event itself
   * ( digi, cluster, hit, track )
   * All these information will be stored in vectors
   * Trigger mask will be persisted too
   */
 public:
  SbtEvent();
  ~SbtEvent() {;}

  void reset();

  void AddStripDigi(const SbtDigi& aStripDigi) { _theStripDigis.push_back(aStripDigi); }
  void AddPxlDigi(const SbtDigi& aPxlDigi) { _thePxlDigis.push_back(aPxlDigi); }
  void AddStripCluster(const SbtCluster& cluster) { _theStripClusters.push_back(cluster); }
  void AddPxlCluster(const SbtCluster& cluster) { _thePxlClusters.push_back(cluster); }
  void AddHit(const SbtHit& hit) { _theHits.push_back(hit); }
  void AddSpacePoint(const SbtSpacePoint& sp) { _theSpacePoints.push_back(sp); }
  SbtTrack& AddTrack(const SbtTrack& track) { _theTracks.push_back(track); return _theTracks.back(); }
  SbtTrack& AddSimulatedTrack(const SbtTrack& track) { _simulatedTracks.push_back(track); return _simulatedTracks.back(); }
  SbtTrack& AddIdealTrack(const SbtTrack& track) { _idealTracks.push_back(track); return _idealTracks.back(); }
  SbtTrack& AddTrack() { _theTracks.push_back(SbtTrack()); return _theTracks.back(); }
  SbtTrack& AddSimulatedTrack() { _simulatedTracks.push_back(SbtTrack()); return _simulatedTracks.back(); }
  SbtTrack& AddIdealTrack() { _idealTracks.push_back(SbtTrack()); return _idealTracks.back(); }

  // method to set the trigger mask
  void SetTriggerInfo(unsigned long trigMask) { _triggerInfo = new SbtTriggerInfo(trigMask); }

  // event number methods
  void SetEventNumber(int i) { _eventNumber = i; }
  int GetEventNumber() const { return _eventNumber; }

  // the set-run number method
  void SetRunNumber(int i) { _runNumber = i; }
  // the set-run number method
  int GetRunNumber() const { return _runNumber; }

  // some methods to get the various lists
  std::vector<SbtDigi>& GetStripDigiList() { return _theStripDigis; }
  std::vector<SbtDigi>& GetPxlDigiList() { return _thePxlDigis; }

  std::vector<SbtCluster>& GetStripClusterList() { return _theStripClusters; }
  std::vector<SbtCluster>& GetPxlClusterList() { return _thePxlClusters; }

  std::vector<SbtHit>& GetHitList() { return _theHits; }
  std::vector<SbtSpacePoint>& GetSpacePointList() { return _theSpacePoints; }
  std::vector<SbtTrack>& GetTrackList() { return _theTracks; }
  std::vector<SbtTrack>& GetIdealTrackList() { return _idealTracks; }
  std::vector<SbtTrack>& GetMCTrackList() { return _simulatedTracks; }

  const std::vector<SbtDigi>& GetStripDigiList() const { return _theStripDigis; }
  const std::vector<SbtDigi>& GetPxlDigiList() const { return _thePxlDigis; }

  const std::vector<SbtCluster>& GetStripClusterList() const { return _theStripClusters; }
  const std::vector<SbtCluster>& GetPxlClusterList() const { return _thePxlClusters; }

  const std::vector<SbtHit>& GetHitList() const { return _theHits; }
  const std::vector<SbtSpacePoint>& GetSpacePointList() const { return _theSpacePoints; }
  const std::vector<SbtTrack>& GetTrackList() const { return _theTracks; }
  const std::vector<SbtTrack>& GetIdealTrackList() const { return _idealTracks; }
  const std::vector<SbtTrack>& GetMCTrackList() const { return _simulatedTracks; }

  // method to get the trigger mask
  SbtTriggerInfo* GetTriggerInfo() const { return _triggerInfo; }

  /*
   * methods to copy the Edro Board info into the event
   */

  // add a word to the list
  void AddAWord(word aWord) { _wordList.push_back(aWord); }

  // add a word to the list
  void RemoveAWord() { _wordList.pop_back(); }

  // set the Dut flag: 1 <-> DUT, 0 <-> telescope
  void SetDutFlag(word aWord) { _isDut = aWord & 0x1; }

  // event counter
  void SetEventCounter(word eventCounter) { _eventCounter = eventCounter; }

  // BCO counter
  void SetBCOCounter(word BCOCounter) { _BCOCounter = BCOCounter; }

  // BCO counter
  word GetBCOCounter() const { return _BCOCounter; }

  // clock counter
  void SetClockCounter(word ClkCounter) { _ClkCounter = ClkCounter; }

  // offset is 0 for layer 0 to 3
  // offset is 4 for layer 4 to 7
  void SetNHitsLayer(word aWord, int offset);

  void SetNHitsLayer(int nLayer, int nHits) { _nHits[nLayer] = nHits; }

  // trigger word
  void SetTriggerWord(word triggerWord) { _triggerWord = triggerWord; }

  // set the Dut flag: 1 <-> DUT, 0 <-> telescope
  bool GetDutFlag() const { return _isDut; }

  // event counter
  word GetEventCounter() const { return _eventCounter; }

  // get nHits
  const int* GetNHits() const { return (_nHits); }

  // get nHits in Layer n
  int GetNHitsLayerN(int nLayer) const { return _nHits[nLayer]; }

  // the check word
  void SetCheckWord(word checkWord) { _checkWord = checkWord; }

  // get the check word
  word GetCheckWord() const { return _checkWord; }

  // get the word list
  std::vector<word>& GetWordList() { return _wordList; }

  const std::vector<word>& GetWordList() const { return _wordList; }

  // set the scintillator flag
  void SetScintillators() { _scintillators = ((_eventCounter & 0x400) == 0x400); }

  void SetScintillators(bool scint) { _scintillators = (scint); }

  // get the scintillator flag
  bool GetScintillatorsFlag() const { return _scintillators; }

  // add a scintillator word
  void AddScintillatorWord(word scintillatorWord) { _scintillatorData.push_back(scintillatorWord); }

  // get scintillator words
  std::vector<word> GetScintillatorWords() const { return _scintillatorData; }

  /*
   * QEventCheck method checks the quality of the event
   * checkword XOR every other word should be 0
   * a false it is returned if not true
   */

  bool QEventCheck();

  void SetTrackable(bool t) { _IsTrackable = t; }
  bool IsTrackable() { return _IsTrackable; }

  void DataIsGood(bool g) { _dataIsGood = g; }
  bool IsDataGood() { return _dataIsGood; }

  void Settimestamp(unsigned long timestamp) { _timestamp = timestamp; }
  unsigned long Gettimestamp() const { return _timestamp; }

  void Settrigger_type(unsigned int trigger_type) {
    _trigger_type = trigger_type;
  }
  unsigned int Gettrigger_type() const { return _trigger_type; }

  // void SetTDCTime( double TDCTime ){ _TDCTime = TDCTime; }
  void SetTDCTime(unsigned int TDCTime) { _TDCTime = TDCTime; }
  unsigned int GetTDCTime() const { return _TDCTime; }

 protected:
  int _DebugLevel;

  std::vector<SbtDigi> _theStripDigis;  // Strip digi list
  std::vector<SbtDigi> _thePxlDigis;    // Pixel digi list
  std::vector<SbtDigi> _theDigis;       // Event digi list

  std::vector<SbtCluster> _theStripClusters;  // Strip clusters
  std::vector<SbtCluster> _thePxlClusters;    // Pxl  clusters

  std::vector<SbtHit> _theHits;  // hits

  std::vector<SbtSpacePoint> _theSpacePoints;  // space point

  std::vector<SbtTrack> _theTracks;        // reconstructed tracks
  std::vector<SbtTrack> _simulatedTracks;  // MC simulated tracks
  std::vector<SbtTrack> _idealTracks;  // MC ideal (no material effects) tracks

  bool _dataIsGood;

  // the trigger information
  SbtTriggerInfo* _triggerInfo;
  // the event number
  int _eventNumber;

  // the run number
  int _runNumber;

  // this is internal timestamp
  // unsigned int _timestamp;
  unsigned long _timestamp;

  // this is internal trigger_type
  unsigned int _trigger_type;

  /*
   * an event will contain several counters
   * (written out by the Edro Board):
   * there is a start word: c1a0c1a0(1) -> last bit is 0 for tele, 1 for DUT
   * a) Event counter
   * b) BCO counter
   * c) Clk counter
   * d) nHitsLayer03: n3n2n1n0 : ni are the number of hits in the ith layer
   * e) nHitsLayer47: n7n6n540 : ni are the number of hits in the ith layer
   */

  bool _isDut;  // last bit of start word
  word _eventCounter;
  word _BCOCounter;
  word _ClkCounter;
  int _nHits[nMaxLayerSides];  // the hits per layerSide in the event

  /*
   * an event will contain also a trigger word
   * in this trigger word form bit 8 to 15
   */

  word _triggerWord;

  /*
   * scintillators data
   */
  bool _scintillators;
  std::vector<word> _scintillatorData;

  /*
   * finally there is the check word
   */
  word _checkWord;

  // the whole word list
  std::vector<word> _wordList;

  // is the event trackable?
  // the flag is set false if there are too
  // many digis on at least one side of the
  // telescope detector

  bool _IsTrackable;
  unsigned int _TDCTime;

  ClassDef(SbtEvent, 1);
};

#endif
