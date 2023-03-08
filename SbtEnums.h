#ifndef SBTENUMS_HH
#define SBTENUMS_HH

class SbtEnums {
 public:
  // define enum types for local coordinates and side type for strip/striplet
  enum view { U, V, undefinedView };
  enum siliconType { N, P };

  enum objectType { 
    ideal = 0,
    simulated = 1,
    reconstructed = 2
  };

  enum trackShape {
    undefinedTrackShape = 0,
    longTrack = 1,
    downStreamTrack = 2,
    upStreamTrack = 3,
    channelledTrack = 4,
    channelledTrackX = 5,
    channelledTrackY = 6
  };

  enum PxlPosWrtSeed {
    top = 0,
    bottom = 1,
    left = 2,
    right = 3,
    center = 4
  };

  enum eventType {
    normal = 0,
    onlyNoise = 1
  };

  enum digiType {
    undefinedDigiType = 0,
    strip = 1,
    pixel = 2
  };

  enum recoType {
    data = 0,
    simulatedNoise = 1,
    simulatedSignal = 2
  };

  enum alignMode {
    dataAlign = 0,
    simAlign = 1
  };

  enum pdf {
    uniform = 0,
    landau = 1,
    gaus = 2
  };

 protected:
  ClassDef(SbtEnums, 1);
};
#endif
