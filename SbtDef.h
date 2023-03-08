#ifndef SBTDEF_HH
#define SBTDEF_HH

#include <math.h>
#include <stdint.h>

#include <Rtypes.h>

//
// this pure header will contain
// data type definitions and constants
//

//
// DAQ word, should be defined in commonheader with DAQ
// presently duplicates _CHUNK and _BLOCK in SbtBit_operations.rdl
//
typedef uint32_t word;

// start and end word for Edro-words stream
const word aStartWord0 = 0xc1a0f0f0;
const word aStartWord1 = 0xc1a0f0f1;
const word aEndWord = 0xb1eb1e0f;

// number of layerSides
const int nLayerSides = 8;
const int nLayers = nLayerSides / 2;
const int nMaxLayerSides = 16;

//
// +++ must examine the code to improve the description
// max id or layer number for telescope planes and DUTs
//
const int maxNTelescopeDetector(10);
const int maxNDutDetector(10);

//--- used in SbtNtupleDumper.cpp, max detector number for printing event
const int maxNDetector(10);

// edro-hits headers
const UInt_t layer0Header = 0xd8;
const UInt_t layer1Header = 0xd9;
const UInt_t layer2Header = 0xda;
const UInt_t layer3Header = 0xdb;
const UInt_t layer4Header = 0xdc;
const UInt_t layer5Header = 0xdd;
const UInt_t layer6Header = 0xde;
const UInt_t layer7Header = 0xdf;

// number of scintillator words (if any)
const int nScintillatorsWords = 4;

// minimum adc value for cluster seed
// no cut on ADC until calibrated (JW, aug 29, 2008)
const double minAdcClusterSeed = 3.0;
// const int minAdcClusterSeed = 150;

// minimum adc value for cluster addendum
// no cut on ADC until calibrated (JW, aug 29, 2008)
const double minAdcClusterAddendum = 2.5;
// const int minAdcClusterAddendum = 80;

// maximum distance for cluster addendum
const int maxChDistance = 1;

// maximum digis per side on a tele det
const UInt_t maxNDigisPerTeleSide = 5;

// maximum digis per side on a generic det
const UInt_t maxNDigisPerSide = 1000;

// define default error for SpacePoint
const double DefaultPointErr[3] = {0.0010, 0.0010, 0.1000};

// define the maximum number of tracks, clusters
// per event to store in the rootuple

const UInt_t maxNTrk = 100;
const int maxNClusters = 100000;
const int maxNDigis = 10000;
const int maxNIdealTrk = 10;  // accepted simulated trk per event
const int maxNMsTrk = maxNIdealTrk;
const int maxTrkNSpacePoint = 10;
const int maxEvtNSpacePoint = 10000;
const int maxIntersects = 10000000;

// define max number of Ideal and MS SP

const int maxNIdealSP = 100;
const int maxNMsSP = 100;

// low limit for step in navigation
const double minStep = 1.e-8;

// BCO frequency
const word BCOFreq = 60000000;                          // 60 MHz
const double ns = 1.e-9;                                // 1 ns expressed in s
const int nsPerBCO = (int)floor(1. / BCOFreq / 1.e-9);  // ns per BCO tick

// define the max values for strip data :adc, chip, set, strip
const int maxAdc = 4096;
const int maxStrip = 7;
const int maxSet = 15;
const int maxChip = 2;

// define the max values for pixel data: macroColumn, row, columnInPxl;
const int maxMacroColumn = 31;
const int maxRow = 31;
const int maxColumnInMP = 3;

const double eps = 1.e-9;

// runs till 2896 have first 32 events corrupted
const UInt_t firstGoodRun = 2896;
const UInt_t firstBadEvents = 32;

// max number of different detectors
const int maxDetTypes = 4;

// number of thresholds fssr2-read detectors
const int nThr = 9;

// max threshold value (in DAC) for fssr2-read detectors
const int maxThrVal = 255;

// threshold dac-to-mv
const double thresholdCalib = 3.4;

// pulser dac-to-mv
const double pulserCalib = 1.2;

// fssr2 gain (0 --> pos. signals; 1 --> neg. signals)
const double gain[2] = {97., 67.};  // [mV/fC]

// noise in enc for tele modules
const double encTeleDac[2] = {510., 810.};  // (pos,neg) signals
// noise in enc for striplet modules
const double encStripletDac[2] = {630., 1020.};  // (pos,neg) signals

const double enc_in_fc = 1.6e-4;  // 1 e- = 1.6e-4 fc

// noise in threshold dac for tele modules
const double thrTeleDac[2] = {
    encTeleDac[0] * enc_in_fc * gain[0] / thresholdCalib,
    encTeleDac[1] * enc_in_fc* gain[1] / thresholdCalib};

// noise in threshold dac for striplet modules
const double thrStripletDac[2] = {
    encStripletDac[0] * enc_in_fc * gain[0] / thresholdCalib,
    encStripletDac[1] * enc_in_fc* gain[1] / thresholdCalib};

#endif
