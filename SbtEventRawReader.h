#ifndef SBTEVENTRAWREADER_HH
#define SBTEVENTRAWREADER_HH

#include <fstream>
#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

#include <Rtypes.h>

#include "SbtEvent.h"

class SbtConfig;
class SbtDetectorElem;
class TGeoManager;
class SbtDigi;

class SbtEventRawReader {
 public: 
  SbtEventRawReader();
  SbtEventRawReader(std::string path, std::string fileNamePattern);
  virtual ~SbtEventRawReader() {;}

  void setFileNamePattern(std::string path, std::string fileNamePattern) { _path = path; _fileNamePattern = fileNamePattern; }

  void setDebugLevel(int debugLevel) { _debugLevel = debugLevel; }
  int getDebugLevel() const { return _debugLevel; }

  void setConfigurator(SbtConfig* configurator);

  // a method to retrieve a detector elem pointer given its ID
  SbtDetectorElem* GetDetectorElem(int ID);

  // a method to retrieve a detector elem pointer given its ID
  int GetDetectorIDfromZCoord(double zCoord);

  virtual bool nextEvent() = 0;
  const SbtEvent& getEvent() const { return _currentEvent; }

  virtual void reset() = 0;

  typedef SbtEventRawReader*(event_raw_reader_factory)(void);

  static void addInRawReaderFactory(std::string k, event_raw_reader_factory* f) { _rawReaderFactoryMap[k] = f; }
  static SbtEventRawReader* createRawReader(std::string rawReaderName);

  virtual bool noMoreEvents() const;

  virtual unsigned int generateFileList();

  virtual bool isEventSelected() { return true; }

  virtual void loadConfiguration(const YAML::Node& conf);

 protected:
  static std::map<std::string, event_raw_reader_factory*> _rawReaderFactoryMap;
  static bool match(const char *pattern, const char *candidate, int p=0, int c=0);

  virtual bool openFile(int i = 0);

  int _debugLevel;
  SbtConfig* _configurator;  // the telescope configurator
  TGeoManager* _geoManager;
  std::string _path;
  std::string _fileNamePattern;

  std::ifstream _currentFile;
  int _currentFileId;
  std::vector<std::string> _fileNameList;

  SbtEvent _currentEvent;

  ClassDef(SbtEventRawReader, 0);
};

#endif
