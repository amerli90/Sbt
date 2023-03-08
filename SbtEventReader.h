#ifndef SBTEVENTREADER_HH
#define SBTEVENTREADER_HH

#include <map>
#include <string>

#include "SbtEventRawReader.h"

class SbtConfig;
class SbtEvent;

class SbtEventReader {
 public:
  SbtEventReader();
  SbtEventReader(std::string fileName);
  SbtEventReader(const YAML::Node& conf);
  ~SbtEventReader() {;}

  void loadConfiguration(std::string fileName);
  void loadConfiguration(const YAML::Node& conf);

  void setDebugLevel(int debugLevel) { _debugLevel = debugLevel; }
  int getDebugLevel() const { return _debugLevel; }

  void setConfigurator(SbtConfig* configurator);

  void setEventRawReader(SbtEventRawReader* rawReader){ _eventRawReader = rawReader; }

  SbtEventRawReader* getEventRawReader() const { return _eventRawReader; }

  virtual SbtEvent* readEvent();
  virtual bool readEvent(SbtEvent& evt);

  const std::string& getName() const { return _name; }

  void reset();

 protected:
  int _debugLevel;
  SbtConfig* _configurator;  // the telescope configurator
  SbtEventRawReader* _eventRawReader;
  std::string _name;

  ClassDef(SbtEventReader, 1);
};

#endif
