// C++ Headers
#include <cassert>
#include <iostream>

// including package classes
#include "SbtIO.h"
#include "SbtEnums.h"
#include "SbtEvent.h"
#include "SbtGenAlg.h"
#include "SbtGenerator.h"
#include "SbtSimpleGenAlg.h"
#include "SbtTrack.h"
#include "SbtCosmicGenAlg.h"
#include "SbtSimpleMultipleScattering.h"
#include "SbtAdvancedMultipleScattering.h"
#include "SbtCorrelatedMultipleScattering.h"
#include "SbtCrystalChanneling.h"

ClassImp(SbtGenerator);

word SbtGenerator::_GenEventCounter = (word)0x0;

SbtGenerator::SbtGenerator() :
  _DebugLevel(0),
  _theConfigurator(nullptr),
  _theGenAlg(nullptr) {
  _rN = new TRandom3();
  _runNumber = (word)0x0;
}

SbtGenerator::SbtGenerator(SbtConfig* configurator, std::string config, unsigned int seed) :
    _DebugLevel(0),
    _theConfigurator(configurator),
    _theGenAlg(nullptr) {
  if (_DebugLevel > 1) {
    std::cout << "SbtGenerator::SbtGenerator(" << config << ")" << std::endl;
    std::cout << config << std::endl;
  }

  if (_DebugLevel > 1) std::cout << "Generator seed is: " << seed << std::endl;
  _rN = new TRandom3(seed);

  _runNumber = (word)0x0;

  ReadConfiguration(config);

  if (_theConfigurator->getCrystalChanneling()) {
    _theGenAlg->SetCrystalChanneling(_theConfigurator->getCrystalChanneling());
    _theConfigurator->getCrystalChanneling()->setGenAlg(_theGenAlg);
  }
}

void SbtGenerator::ReadConfiguration(std::string configFileName) {
  assert(_theConfigurator);
  if (!SbtIO::searchConfigFile(configFileName)) return;
  std::cout << "Loading generator config file '" << configFileName << "'" << std::endl;
  YAML::Node config = YAML::LoadFile(configFileName);
  ReadGenAlg(config);
  ReadTrackParameters(config);
  _name = config["name"].as<std::string>();
  if (config["beam"]) ReadBeamParameters(config["beam"]);
  if (config["noise"]) ReadNoiseParameters(config["noise"]);
  if (config["multipleScattering"]) ReadMultipleScatteringParameters(config["multipleScattering"]);
  if (config["detectorResponse"]) ReadDetectorParameters(config["detectorResponse"]);
}

void SbtGenerator::ReadDetectorParameters(const YAML::Node& config) {
  for (const auto& detConf : config) {
    auto idDet = detConf["id"].as<int>();
    auto detType = _theConfigurator->getDetectorTypeFromID(idDet);
    if (!detType) {
      std::cout << "Could not find detector type id " << idDet << std::endl;
      continue;
    }
    auto elossDistr_str = detConf["distribution"].as<std::string>();
    SbtEnums::pdf elossDistr = SbtEnums::pdf::uniform;
    if (elossDistr_str == "uniform") {
      elossDistr = SbtEnums::pdf::uniform;
    }
    else if (elossDistr_str == "landau") {
      elossDistr = SbtEnums::pdf::landau;
    }
    else if (elossDistr_str == "gaus") {
      elossDistr = SbtEnums::pdf::gaus;
    }
    else {
      std::cout << "ERROR in SbtGenerator::ReadDetectorParameters: distribution type '" <<
      elossDistr_str << "' unknonwn" << std::endl;
    }
    double artificialRes = 0.;
    if (detConf["artificialSpatialResolution"]) {
      artificialRes = detConf["artificialSpatialResolution"].as<double>();
      std::cout << "Artifical spatial resolution: " << artificialRes << std::endl;
    }
    detType->setSimulationParameters(elossDistr, detConf["mpv"].as<double>(), detConf["sigma"].as<double>(), detConf["adcGain"].as<double>(), detConf["adcSaturation"].as<int>(), detConf["thresholdU"].as<int>(), detConf["thresholdV"].as<int>(), detConf["floatingStrips"].as<bool>(), detConf["chargeSpreadSigma"].as<double>(), artificialRes);
  }
}

void SbtGenerator::ReadMultipleScatteringParameters(const YAML::Node& config) {
  auto msName = config.as<std::string>();
  std::cout << "Multiple scattering option: " << msName << std::endl;
  if (msName == "Simple") {
    _theGenAlg->SetMultipleScattering(new SbtSimpleMultipleScattering(_theGenAlg));
  }
  else if (msName == "Advanced") {
    _theGenAlg->SetMultipleScattering(new SbtAdvancedMultipleScattering(_theGenAlg));
  }
  else if (msName == "Correlated") {
    _theGenAlg->SetMultipleScattering(new SbtCorrelatedMultipleScattering(_theGenAlg));
  }
  else if (msName != "None") {
    std::cout << "Multiple scattering '" << msName << "' unknown!" << std::endl;
    exit(2);
  }
}

void SbtGenerator::ReadBeamParameters(const YAML::Node& config) {
  /*
   * Define here the relevant variables to read
   */

  std::cout << "SbtGenerator: DebugLevel =  " << _DebugLevel << std::endl;
  auto angular_div = config["angularDiv"].as<double>(); // angular divergence (rad)
  double angular_mean_x = 0.; 
  if (config["angularMeanX"]) angular_mean_x = config["angularMeanX"].as<double>(); // mean angular direction of the beam in the xz plane (rad)
  double angular_mean_y = 0.;
  if (config["angularMeanY"]) angular_mean_y = config["angularMeanY"].as<double>(); // mean angular direction of the beam in the yz plane (rad)
  auto beam_sigma_x = config["sigma_x"].as<double>(); // beam x_axis dispersion (cm)
  auto beam_sigma_y = config["sigma_y"].as<double>(); // beam y_axis dispersion (cm)
  auto energy = config["energy"].as<double>(); // beam energy (MeV)
  auto particle_id = config["particle"].as<int>(); // particle id (see pdt.table)
  auto d_beam_tele = config["distanceToDetector"].as<double>(); // distance beam-first module
  auto angularDivDistr_str = config["angularDivDistr"].as<std::string>();

  std::cout << "angular_div: " << angular_div << std::endl;
  std::cout << "mean beam angular directions: " << angular_mean_x << ", " << angular_mean_y << std::endl;
  std::cout << "angular_div_distr: " << angularDivDistr_str << std::endl;
  std::cout << "beam_sigma_x: " << beam_sigma_x << std::endl;
  std::cout << "beam_sigma_y: " << beam_sigma_y << std::endl;
  std::cout << "particle_id: " << particle_id << std::endl;
  std::cout << "energy: " << energy << std::endl;
  std::cout << "d_beam_tele: " << d_beam_tele << std::endl;

  if (_theGenAlg != nullptr) {
      SbtEnums::pdf angularDivDistr = SbtEnums::pdf::gaus;
      if (angularDivDistr_str == "uniform") {
        angularDivDistr = SbtEnums::pdf::uniform;
      }
      else if (angularDivDistr_str == "gaus") {
        angularDivDistr = SbtEnums::pdf::gaus;
      }
      else {
        std::cout << "ERROR in SbtGenerator::ReadBeamParameters: distribution type '" <<
        angularDivDistr_str << "' unknonwn" << std::endl;
      }


    _theGenAlg->SetAngularDiv(angular_div);
    _theGenAlg->SetAngularDivPdf(angularDivDistr);
    _theGenAlg->SetMeanAngularDirection(angular_mean_x, angular_mean_y);
    _theGenAlg->SetBeamSX(beam_sigma_x);
    _theGenAlg->SetBeamSY(beam_sigma_y);
    _theGenAlg->SetPId(particle_id);
    _theGenAlg->SetEnergy(energy);
    _theGenAlg->SetDB2T(d_beam_tele);
  }
}

void SbtGenerator::ReadNoiseParameters(const YAML::Node& config) {
  std::cout << "SbtGenerator::ReadNoiseParameters()" << std::endl;
  /*
   * noise param
   */
  double StripOccupancy(0.);  // the strip noise occupancy
  double PxlOccupancy(0.);    // the pxl noise occupancy
  int AvgNoiseAdcValue(0);    // the average noise value

  StripOccupancy = config["strip"].as<double>();
  PxlOccupancy = config["pixel"].as<double>();
  AvgNoiseAdcValue = config["avgADC"].as<int>();

  std::cout << "StripOccupancy: " << StripOccupancy
            << "\t PxlOccupancy: " << PxlOccupancy << std::endl;

  SetStripOcc(StripOccupancy);
  SetPxlOcc(PxlOccupancy);
  SetAvgNoiseAdcVal(AvgNoiseAdcValue);

  if (_theGenAlg) {
    _theGenAlg->SetStripOcc(StripOccupancy);
    _theGenAlg->SetPxlOcc(PxlOccupancy);
    _theGenAlg->SetAvgNoiseAdcVal(AvgNoiseAdcValue);
  }
}

void SbtGenerator::ReadTrackParameters(const YAML::Node& config) {
  std::cout << "SbtGenerator::ReadTrackParameters()"
            << std::endl;
  //
  // track parameters
  //
  int track_per_event(1);  // number (Poissonian) of tracks per event

  track_per_event = config["tracksPerEvent"].as<int>();
  std::cout << "track_per_event: " << track_per_event << std::endl;
  SetNTracks(track_per_event);
}

void SbtGenerator::ReadGenAlg(const YAML::Node& config) {
  std::cout << "SbtGenerator::ReadGenAlg()" << std::endl;

  _genAlgName = config["algorithm"].as<std::string>();
  std::cout << "genAlg: " << _genAlgName << std::endl;

  if (_genAlgName == "Simple") {
    _theGenAlg = new SbtSimpleGenAlg(_rN);
    std::cout << "A SbtSimpleGenAlg is born!" << std::endl;
  } 
  else if (_genAlgName == "Cosmic") {
    _theGenAlg = new SbtCosmicGenAlg(_rN);
    std::cout << "A SbtCosmicGenAlg is born!" << std::endl;
  }
  else {
    std::cout << "Unable to find the Generator alg you are looking for: "
              << _genAlgName << std::endl;
    assert(0);
  }
}

void SbtGenerator::GenerateTracks(SbtEvent& event) {
  if (_DebugLevel > 1) std::cout << "SbtGenerator::GenerateTracks" << std::endl;

  if (_theGenAlg != nullptr) {
    _theGenAlg->SetConfigurator(_theConfigurator);
  }

  if (_DebugLevel > 1) {
    std::cout << "nTracks: " << _nTracks << std::endl;
  }

  for (int iTrack = 0; iTrack < _nTracks; iTrack++) {
    _theGenAlg->GenerateTrack(event);
  }

  if (_DebugLevel > 0) {
    std::cout << "A total of " << event.GetMCTrackList().size() << " simulated and "
              << event.GetIdealTrackList().size() << " ideal tracks has been produced" << std::endl;
  }
}

void SbtGenerator::GenerateStripNoise(std::vector<SbtDigi>& digi) {
  if (_theGenAlg) {
    _theGenAlg->SetConfigurator(_theConfigurator);
  }

  _theGenAlg->GenerateStripNoise(digi);
}

void SbtGenerator::GeneratePxlNoise(std::vector<SbtDigi>& digi) {
  if (_theGenAlg) {
    _theGenAlg->SetConfigurator(_theConfigurator);
  }

  _theGenAlg->GeneratePxlNoise(digi);
}

bool SbtGenerator::GenerateEvent(SbtEvent& event, SbtEnums::eventType eT) {
  //
  // analogous to SbtEventReader::ReadEvent(). Creates a new SbtEvent.
  //
  if (_DebugLevel > 20) std::cout << "Generating an event..." << std::endl;
  event.reset();

  // this will be used later when creating digis and will also be inserted
  // into event
  word BCOCounter = GenerateBCOCounter();

  // generate noise hits if requested
  if (_stripOccupancy > 0) {
    GenerateStripNoise(event.GetStripDigiList());
  }
  if (_pxlOccupancy > 0) {
    GeneratePxlNoise(event.GetPxlDigiList());
  }

  if (eT != SbtEnums::onlyNoise) {
    GenerateTracks(event);
  }

  event.SetEventNumber(_GenEventCounter);
  event.SetRunNumber(_runNumber);

  if (_DebugLevel > 1) {
    std::cout << "SbtGenerator::GenerateEvent " << std::endl;
    std::cout << "number of MsTracks = " << event.GetMCTrackList().size() << std::endl;
    std::cout << "number of idealTracks = " << event.GetIdealTrackList().size() << std::endl;
  }

  // event header
  bool isDut = 0;
  word EventHeader = GenerateEventHeader(isDut);
  event.AddAWord(EventHeader);

  // event counter
  word EventCounter = GenerateEventCounter();
  event.AddAWord(EventCounter);
  event.SetEventCounter(EventCounter);
  event.SetScintillators();  // scintillators flag

  // BCO counter
  // jw moved generation of BCO above creation of track digis, so the value
  // is available when contructing the digis
  // word BCOCounter = GenerateBCOCounter(  );
  event.AddAWord(BCOCounter);

  // Clock counter
  word ClockCounter = GenerateClockCounter();
  event.AddAWord(ClockCounter);

  double nDAQSides = _theConfigurator->getNLayerSides();
  // digis: need to add headers
  for (int i = 0; i < nDAQSides; i++) {
    word aLayerHeader = GetLayerHeader(i);
    event.AddAWord(aLayerHeader);
  }

  // scintillators data
  std::vector<word> scintWords;
  if (event.GetScintillatorsFlag()) {
    GetScintillatorsWords(scintWords);
    for (unsigned int i = 0; i < scintWords.size(); i++) {
      event.AddAWord(scintWords.at(i));
    }
  }

  // end word
  word EndWord = GenerateEndWord();
  event.AddAWord(EndWord);

  // check word
  word CheckWord = GenerateCheckWord(&event);
  event.AddAWord(CheckWord);
  event.SetCheckWord(CheckWord);

  _GenEventCounter = _GenEventCounter + (word)(0x1);

  if (_DebugLevel > 5) std::cout << "SbtGenerator::GenerateEvent: done with this event" << std::endl;
  return true;
}

word SbtGenerator::GenerateCheckWord(SbtEvent* anEvent) {
  std::vector<word> wordList = anEvent->GetWordList();
  std::vector<word>::iterator aWord;
  word xorWord(0);

  for (aWord = wordList.begin(); aWord != wordList.end(); aWord++) {
    if (aWord == wordList.begin()) {
      xorWord = (*aWord);
    } else {
      xorWord ^= (*aWord);
    }
  }
  return xorWord;
}

void SbtGenerator::SetConfigurator(SbtConfig* aConfigurator) {
  _theConfigurator = aConfigurator;
  if (_theGenAlg) {
    _theGenAlg->SetConfigurator(aConfigurator);
  }
}

word SbtGenerator::GenerateBCOCounter() {
  TTimeStamp ts;
  timespec_t t;
  t = ts.GetTimeSpec();
  word time = (word)(t.tv_nsec / nsPerBCO);  // ticks of BCO clock
  return time;
}

word SbtGenerator::GenerateClockCounter() {
  TTimeStamp ts;
  timespec_t t;
  t = ts.GetTimeSpec();
  word time = (word)(t.tv_sec);
  return time;
}

word SbtGenerator::GetLayerHeader(int layerNumber) {
  assert(layerNumber > -1 && layerNumber < _theConfigurator->getNLayerSides());
  return (word)((0xd8 + layerNumber) << 24);
}

void SbtGenerator::GetScintillatorsWords(std::vector<word>& scintWords) {
  scintWords.clear();
  for (int i = 0; i < nScintillatorsWords; i++) {
    scintWords.push_back(0x8b1c001c);
  }
}

word SbtGenerator::GenerateRunNumber() {
  TTimeStamp ts;
  timespec_t t;
  t = ts.GetTimeSpec();
  word now = (word)(t.tv_sec);
  return now;
}
