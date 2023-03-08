#include <iostream>

#include <SbtIO.h>
#include "SbtConfig.h"
#include "SbtDetectorElem.h"
#include "SbtEventRawReader.h"

ClassImp(SbtEventRawReader);

std::map<std::string, SbtEventRawReader::event_raw_reader_factory*> SbtEventRawReader::_rawReaderFactoryMap;

SbtEventRawReader::SbtEventRawReader()
    : _debugLevel(0),
      _configurator(nullptr),
      _geoManager(nullptr),
      _path(),
      _fileNamePattern(),
      _currentFile(),
      _currentFileId(-1),
      _fileNameList(),
      _currentEvent() {
  std::cout << "SbtEventRawReader:  DebugLevel= " << _debugLevel << std::endl;
}

SbtEventRawReader::SbtEventRawReader(std::string path,
                                     std::string fileNamePattern)
    : _debugLevel(0),
      _configurator(nullptr),
      _geoManager(nullptr),
      _path(path),
      _fileNamePattern(fileNamePattern),
      _currentFile(),
      _currentFileId(-1),
      _fileNameList(),
      _currentEvent() {
  std::cout << "SbtEventRawReader:  DebugLevel= " << _debugLevel << std::endl;
}

bool SbtEventRawReader::match(const char* pattern, const char* candidate, int p,
                              int c) {
  if (pattern[p] == '\0') {
    return candidate[c] == '\0';
  } else if (pattern[p] == '*') {
    for (; candidate[c] != '\0'; c++) {
      if (match(pattern, candidate, p + 1, c)) return true;
    }
    return match(pattern, candidate, p + 1, c);
  } else if (pattern[p] != '?' && pattern[p] != candidate[c]) {
    return false;
  } else {
    return match(pattern, candidate, p + 1, c + 1);
  }
}

unsigned int SbtEventRawReader::generateFileList() {
  _fileNameList = SbtIO::generateFileList(_path, _fileNamePattern);
  return _fileNameList.size();
}

SbtEventRawReader* SbtEventRawReader::createRawReader(std::string rawReaderName) {
  auto search = _rawReaderFactoryMap.find(rawReaderName);
  if (search != _rawReaderFactoryMap.end()) {
    std::cout << "Creating raw reader of class '" << rawReaderName << "'" << std::endl;
    return ((*search).second)();
  }
  else {
    std::cout << "Could not create raw reader with name '" << rawReaderName << "'" << std::endl;
    return nullptr;
  }
}

void SbtEventRawReader::loadConfiguration(const YAML::Node& conf) {
  setFileNamePattern(conf["inputPath"].as<std::string>(), conf["inputFilePattern"].as<std::string>());
  generateFileList();
}

bool SbtEventRawReader::noMoreEvents() const {
  if (_currentFileId + 1 < _fileNameList.size()) return false;
  if (_currentFile.is_open() && !_currentFile.eof()) return false;
  return true;
}

bool SbtEventRawReader::openFile(int i) {
  if (i >= 0 && i < _fileNameList.size()) {
    if (_currentFile.is_open()) {
      _currentFile.close();
    }
    std::cout << "Opening file '" << _fileNameList[i] << "'..." << std::endl;
    _currentFile.open(_fileNameList[i]);
    if (_currentFile.good()) {
      _currentFileId = i;
    } else {
      std::cout << "ERROR: unable to open file!" << std::endl;
    }
    return _currentFile.good();
  } else {
    _currentFileId = -1;
    return false;
  }
}

void SbtEventRawReader::setConfigurator(SbtConfig* configurator) {
  _configurator = configurator;
  _geoManager = configurator->getGeoManager();
}

// a method to retrieve a detector elem pointer given its ID
SbtDetectorElem* SbtEventRawReader::GetDetectorElem(int ID) {
  SbtDetectorElem* theElem;
  theElem = nullptr;

  std::vector<SbtDetectorElem*> detEleList =
      _configurator->getDetectorElemList();

  for (unsigned int iEle = 0; iEle < detEleList.size(); iEle++) {
    if ((detEleList.at(iEle))->GetID() == ID) {
      theElem = detEleList.at(iEle);
    }
  }
  return theElem;
}

// a method to retrieve a detector elem pointer given its ID
int SbtEventRawReader::GetDetectorIDfromZCoord(double zCoord) {
  int detID(-1);
  std::vector<SbtDetectorElem*> detEleList =
      _configurator->getDetectorElemList();

  for (unsigned int iEle = 0; iEle < detEleList.size(); iEle++) {
    if (fabs(detEleList.at(iEle)->GetZPos() - zCoord) <
        detEleList.at(iEle)->GetDetectorType()->GetZ_HalfDim())
      detID = detEleList.at(iEle)->GetID();
  }
  return detID;
}
