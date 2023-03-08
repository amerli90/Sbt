#include <yaml-cpp/yaml.h>

// including package classes
#include "SbtEvent.h"

#include "SbtEventReader.h"

ClassImp(SbtEventReader);

SbtEventReader::SbtEventReader()
    : _debugLevel(0), _configurator(nullptr), _eventRawReader(nullptr), _name() {
}

SbtEventReader::SbtEventReader(std::string fileName)
    : _debugLevel(0), _configurator(nullptr), _eventRawReader(nullptr), _name() {
  loadConfiguration(fileName);
}

SbtEventReader::SbtEventReader(const YAML::Node& conf)
    : _debugLevel(0), _configurator(nullptr), _eventRawReader(nullptr), _name() {
  loadConfiguration(conf);
}

void SbtEventReader::loadConfiguration(std::string fileName) {
  YAML::Node conf = YAML::LoadFile(fileName);
  loadConfiguration(conf);
}

void SbtEventReader::loadConfiguration(const YAML::Node& conf) {
  _name = conf["name"].as<std::string>();
  std::string eventRawReaderName = conf["rawReader"].as<std::string>();
  _eventRawReader = SbtEventRawReader::createRawReader(eventRawReaderName);
  if (_configurator) _eventRawReader->setConfigurator(_configurator);
  if (_eventRawReader) _eventRawReader->loadConfiguration(conf);
}

SbtEvent* SbtEventReader::readEvent() {
  if (!_eventRawReader) {
    return nullptr;
  }
  if (!_eventRawReader->nextEvent()) {
    return nullptr;
  }
  SbtEvent* event = new SbtEvent(_eventRawReader->getEvent());
  return event;
}

bool SbtEventReader::readEvent(SbtEvent& evt) {
  if (!_eventRawReader) {
    return false;
  }
  if (!_eventRawReader->nextEvent()) {
    return false;
  }
  evt = _eventRawReader->getEvent();
  return true;
}


void SbtEventReader::reset() {
  _eventRawReader->reset();
}

void SbtEventReader::setConfigurator(SbtConfig* configurator) {
  _configurator = configurator;
  if (_eventRawReader) _eventRawReader->setConfigurator(_configurator);
}