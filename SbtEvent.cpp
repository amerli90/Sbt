#include "SbtDef.h"

#include <iomanip>
#include <iostream>
#include <vector>
#include "SbtEvent.h"

ClassImp(SbtEvent);

SbtEvent::SbtEvent() {
  _scintillators = false;
  _wordList.clear();
  _eventNumber = -1;
  _runNumber = -1;
  _DebugLevel = 0;
  _IsTrackable = true;
  _dataIsGood = true;

  _theStripDigis.reserve(1000);
  _thePxlDigis.reserve(1000);
  _theDigis.reserve(1000);

  _theStripClusters.reserve(1000);
  _thePxlClusters.reserve(1000);

  _theHits.reserve(1000);

  _theSpacePoints.reserve(1000);

  _theTracks.reserve(10);
  _simulatedTracks.reserve(10);
  _idealTracks.reserve(10);
}

void SbtEvent::reset() {
  _theStripDigis.clear();
  _thePxlDigis.clear();
  _theDigis.clear();

  _theStripClusters.clear();
  _thePxlClusters.clear();

  _theHits.clear();

  _theSpacePoints.clear();

  _theTracks.clear();
  _simulatedTracks.clear();
  _idealTracks.clear();

  _dataIsGood = true;

  _triggerInfo = nullptr;
  _eventNumber = -1;
  _runNumber = -1;
  _timestamp = 0;
  _trigger_type = 0;

  _isDut = false;
  _eventCounter = 0;
  _BCOCounter = 0;
  _ClkCounter = 0;
  memset(_nHits, 0, sizeof(int)*nMaxLayerSides);

  _triggerWord = 0;

  _scintillators = false;
  _scintillatorData.clear();

  _checkWord = 0;
  _wordList.clear();

  _IsTrackable = true;

  _TDCTime = 0;
}

bool SbtEvent::QEventCheck() {
  std::vector<word> wordList = GetWordList();
  word checkWord = GetCheckWord();
  std::vector<word>::iterator aWord;
  word xorWord(0);

  for (aWord = wordList.begin(); aWord != wordList.end(); aWord++) {
    if (aWord == wordList.begin()) {
      xorWord = (*aWord);
    } else {
      if (*aWord != checkWord) {
        xorWord ^= (*aWord);
      }
    }
  }
  if ((xorWord ^ checkWord) != 0) {
    std::cerr << "QEventCheck error!" << std::endl;
    std::cerr << "Check Word: " << std::hex << std::setw(8) << std::setfill('0')
              << (xorWord ^ checkWord) << std::setw(0) << std::dec << std::endl;
    return false;
  }
  std::cout << "QEventCheck is ok!\n";
  return true;
}

  // offset is 0 for layer 0 to 3
  // offset is 4 for layer 4 to 7
  void SbtEvent::SetNHitsLayer(word aWord, int offset) {
    _nHits[offset] = aWord & 0xff;
    _nHits[offset + 1] = (int)((aWord >> 8) & 0xff);
    _nHits[offset + 2] = (int)((aWord >> 16) & 0xff);
    _nHits[offset + 3] = (int)((aWord >> 24) & 0xff);
  }