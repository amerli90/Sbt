#include <iostream>
#include <sstream>

#include <TMath.h>
#include <TGeoManager.h>
#include "SbtCrystalChanneling.h"

ClassImp(SbtCrystalChanneling);

SbtCrystalChanneling::SbtCrystalChanneling(TGeoManager* geoMan,
                                           double xPos, double yPos, double zPos,
                                           double xHalfDim, double yHalfDim, double zHalfDim,
                                           const char* material_str, double channelingAngle, double deflectionAngle) :
  _debugLevel(0),
  _channelingAngle(channelingAngle),
  _deflectionAngle(deflectionAngle),
  _genAlg(nullptr) {
  std::cout << "SbtCrystalChanneling:  DebugLevel= " << _debugLevel << std::endl;

  _crystalPosition.SetXYZ(xPos, yPos, zPos);

  std::map<std::string, double> mat_density = { {"silicon" , 2.33}, {"germanium" , 5.32}};
  auto density = mat_density.find(material_str);

  if (density == mat_density.end()) {
    std::cout << "Crystal material '" << material_str << "' unknown!" << std::endl;
    assert(0);
  }

  int crystalId = 1;

  TGeoMaterial *mat = new TGeoMaterial(material_str, geoMan->GetElementTable()->FindElement(material_str), density->second);
  TGeoMedium *crystal = new TGeoMedium("Silicon", crystalId, mat);

  TGeoVolume *top = geoMan->GetTopVolume();

  std::string crystalName = "crystal";

  if (_debugLevel) {
    std::cout << "Building crystal " << crystalName << std::endl;
    std::cout << "Half dimensions x, y, z = " << xHalfDim << ", "
         << yHalfDim << ", " << zHalfDim
         << std::endl;
    std::cout << "Positions  x, y, z = " << xPos << ", " << yPos << ", " << zPos
         << std::endl;
  }

  auto crystalVolume = geoMan->MakeBox(crystalName.c_str(), crystal, xHalfDim, yHalfDim, zHalfDim);

  crystalVolume->SetLineColor(9);

  auto pos = new TGeoTranslation("pos", xPos, yPos, zPos);

  top->AddNode(crystalVolume, crystalId, pos);
}

bool SbtCrystalChanneling::CrystalChanneling(TVector3& point, TVector3& direction) {
  if (!_genAlg) {
    std::cout << "SbtCrystalChanneling::CrystalChanneling: ERROR: gen alg not defined!!" << std::endl;
    return false;
  }
  double tot_eff = 0;
  double tot_w = 0;
  double incoming_angle = TMath::ATan(getLastDirection().X() / getLastDirection().Z());
  if (_debugLevel > 2) {
    std::cout << "The incoming angle is " << incoming_angle << std::endl;
    std::cout << "The incoming angle with respect to the crystal plane is " << incoming_angle - _channelingAngle << std::endl;
  }
  for (auto crystal_plane : _crystalPlanes) {
    double plane_eff = ChannelingEfficiency(crystal_plane.second.max_efficiency, crystal_plane.second.critical_angle, incoming_angle - _channelingAngle);
    if (_debugLevel> 2) std::cout << "The efficiency for plane " << crystal_plane.first << " is " << plane_eff << std::endl;
    tot_w += crystal_plane.second.weight;
    tot_eff += crystal_plane.second.weight * plane_eff;
  }
  tot_eff /= tot_w;
  if (_debugLevel > 2) std::cout << "The total channeling efficiency is: " << tot_eff << std::endl;
  if (tot_eff > 0 && tot_eff > getRandomGen()->Uniform()) {
    direction.RotateY(_deflectionAngle);
    double dist = (point - getLastPoint()).Mag();
    point.SetX(point.X() + dist / 2.0 * TMath::Tan(TMath::Pi() + _deflectionAngle));
    return true;
  }
  return false;
}

double SbtCrystalChanneling::ChannelingEfficiency(double max_efficiency, double critical_angle, double incoming_angle) {
  if (TMath::Abs(incoming_angle) >= critical_angle) return 0.;
  return max_efficiency * TMath::Sqrt(1 - (incoming_angle * incoming_angle) / (critical_angle * critical_angle));
  // Biryukov et al, Crystal Channeling and Its Application at High-Energy Accelerators, Springer 1997, page 16
}

void SbtCrystalChanneling::addCrystalPlane(const char* name, double max_eff, double crit_angle, double w) {
  _crystalPlanes[name] = { max_eff, crit_angle, w };
}