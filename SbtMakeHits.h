#ifndef SBTMAKEHITS_HH
#define SBTMAKEHITS_HH

#include <vector>
#include "SbtDef.h"

class SbtEvent;
class SbtHit;

class SbtMakeHits {
 public:
  SbtMakeHits();
  virtual ~SbtMakeHits() {;}
  void setDebugLevel(int debugLevel) { _DebugLevel = debugLevel; }
  int getDebugLevel() const { return _DebugLevel; }

  void makeHits(SbtEvent* event);

 protected:
  int _DebugLevel;

  ClassDef(SbtMakeHits, 1);
};

#endif
