#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>

#include <yaml-cpp/yaml.h>

#include <TGeoPhysicalNode.h>
#include <TMath.h>

#include "SbtIO.h"
#include "SbtAlignGeom.h"
#include "SbtConfig.h"
#include "SbtDetectorElem.h"

ClassImp(SbtAlignGeom);

SbtAlignGeom::SbtAlignGeom()
    : _debugLevel(0),
      _name(),
      _alignPath(),
      _config(nullptr),
      _mode(SbtEnums::alignMode::dataAlign) {
  std::cout << "SbtAlignGeom:  DebugLevel= " << _debugLevel << std::endl;
}

void SbtAlignGeom::initialize(std::string name,
                              SbtConfig* config,
                              SbtEnums::alignMode mode,
                              std::string alignPath) {
  assert(config);
  _name = name;
  _alignPath = alignPath;
  _config = config;
  _mode = mode;
  _detElemList = _config->getDetectorElemList();
  for(auto detElem : _detElemList) {
    _nominalAlignMap[detElem->GetID()] = new TGeoCombiTrans(*(detElem->GetCombiTrans()));
  }
  resetAlignPars();
}

void SbtAlignGeom::readAlignParms(int loop) {
  std::cout << "SbtAlignGeom::readAlignParms" << std::endl;
  std::stringstream fileName;
  if (loop >= 0) {
    fileName << _alignPath << "/" << _name << "_" << loop << ".yaml";
  }
  else {
    fileName << _alignPath << "/" << _name << ".yaml";
  }
  std::cout << "Align file: '" << fileName.str() << "'" << std::endl;
  YAML::Node yamlDetElemList = YAML::LoadFile(fileName.str().c_str());
  for (auto align : yamlDetElemList) {
    auto det = _config->getDetectorElemFromID(align["id"].as<int>());
    auto eulerAngles = align["eulerAngles"].as<std::vector<double>>();
    TGeoRotation* rot = det->GetCombiTrans()->GetRotation();
    if (!rot) {
      rot = new TGeoRotation("rot", eulerAngles[0], eulerAngles[1], eulerAngles[2]);
      det->GetCombiTrans()->SetRotation(rot);
    }
    else {
      det->GetCombiTrans()->GetRotation()->SetAngles(eulerAngles[0], eulerAngles[1], eulerAngles[2]);
    }
    auto translation = align["position"].as<std::vector<double>>();
    det->GetCombiTrans()->SetTranslation(translation[0], translation[1], translation[2]);
  }
}

void SbtAlignGeom::writeAlignParms(int loop) {
  std::cout << "SbtAlignGeom::writeAlignParms" << std::endl;
  if (!SbtIO::createPath(_alignPath)) {
    std::cout << "Could not create path '" << _alignPath << "'" << std::endl;
    return;
  }
  std::stringstream fileName;
  if (loop >= 0) {
    fileName << _alignPath << "/" << _name << "_" << loop << ".yaml";
  }
  else {
    fileName << _alignPath << "/" << _name << ".yaml";
  }
  YAML::Node yamlDetElemList(YAML::NodeType::Sequence);
  for (auto det : _detElemList) {
    double phi = 0, theta = 0, psi = 0; // euler angles
    if (det->GetCombiTrans()->GetRotation()) {
      det->GetCombiTrans()->GetRotation()->GetAngles(phi, theta, psi);
    }
    YAML::Node align(YAML::NodeType::Map);
    align["eulerAngles"].push_back(phi);
    align["eulerAngles"].push_back(theta);
    align["eulerAngles"].push_back(psi);
    align["eulerAngles"].SetStyle(YAML::EmitterStyle::Flow);
    const double* position = det->GetCombiTrans()->GetTranslation();
    for (int i = 0; i < 3; i++) {
      align["position"].push_back(position[i]);
    }
    align["position"].SetStyle(YAML::EmitterStyle::Flow);
    align["orientation"] = std::vector<double>({1.0, 1.0});
    align["orientation"].SetStyle(YAML::EmitterStyle::Flow);
    align["id"] = det->GetID();
    align["typeId"] = det->GetDetectorType()->GetID();
    yamlDetElemList.push_back(align);
  }
  std::ofstream file(fileName.str());
  file << yamlDetElemList;
  file.close();
}

bool SbtAlignGeom::applyAlignment() {
  std::cout << "SbtAlignGeom::applyAlignment" << std::endl;
  if (_debugLevel) std::cout << "SbtAlignGeom::applyAlignment" << std::endl;
  for(auto detElem : _detElemList) {
    std::cout << "Aligning detector " << detElem->GetID() << std::endl;
    std::cout << "Translation: " << _alignPars[detElem->GetID()][3] << ", "<< _alignPars[detElem->GetID()][4] << ", " << _alignPars[detElem->GetID()][5] << std::endl;
    std::cout << "Rotating around X: " << _alignPars[detElem->GetID()][0] << std::endl;
    std::cout << "Rotating around Y: " << _alignPars[detElem->GetID()][1] << std::endl;
    std::cout << "Rotating around Z: " << _alignPars[detElem->GetID()][2] << std::endl;
    _applyAlignment(_alignPars[detElem->GetID()], detElem->GetCombiTrans());
    std::cout << "Corrected geometry" << std::endl;
    detElem->GetCombiTrans()->Print();
  }
  resetAlignPars();
  return true;
}

void SbtAlignGeom::resetAlignPars() {
  if (_debugLevel) std::cout << "SbtAlignGeom::resetAlignPars" << std::endl;
  for(auto detElem : _detElemList) {
    _alignPars[detElem->GetID()][0] = 0;
    _alignPars[detElem->GetID()][1] = 0;
    _alignPars[detElem->GetID()][2] = 0;
    _alignPars[detElem->GetID()][3] = 0;
    _alignPars[detElem->GetID()][4] = 0;
    _alignPars[detElem->GetID()][5] = 0;
  }
}

void SbtAlignGeom::_applyAlignment(const std::array<double,6> pars, TGeoCombiTrans* t) {
  std::cout << "SbtAlignGeom::_applyAlignment" << std::endl;
  TGeoRotation* rot = t->GetRotation();
  if (!rot) {
    rot = new TGeoRotation();
    rot->RotateX(pars[0]);
    rot->RotateY(pars[1]);
    rot->RotateZ(pars[2]);
    t->SetRotation(rot);
  }
  else {
    rot->RotateX(pars[0]);
    rot->RotateY(pars[1]);
    rot->RotateZ(pars[2]);
  }
  const double* trans = t->GetTranslation();
  t->SetTranslation(trans[0]+pars[3], trans[1]+pars[4], trans[2]+pars[5]);
}