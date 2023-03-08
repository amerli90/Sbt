#ifndef SBT_ALIGNMENT
#define SBT_ALIGNMENT

#include <map>
#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

#include <TGeoPhysicalNode.h>

#include "SbtDef.h"
#include "SbtEnums.h"
#include "SbtAlignmentAlg.h"

class SbtConfig;
class SbtAlignmentAlg;
class SbtDetectorElem;
class SbtEvent;
class SbtAlignGeom;

class SbtAlignment {
 public:
  SbtAlignment(SbtConfig* configurator, std::string alignConfigFile, SbtEnums::alignMode mode);
  ~SbtAlignment();

  SbtAlignmentAlg* getAlignAlg() { return _alignmentAlg; }
  std::vector<SbtDetectorElem*> getAlignDetList() { return _alignDetList; }
  SbtAlignGeom* getAlignGeom() { return getAlignAlg()->getAlignGeom(); }
  SbtEnums::alignMode getMode() { return _mode; }

  void incrementLoopIndex();
  void loadEvent(SbtEvent* anEvent);
  void align();
  void makePlots();
  void exportHistograms() const;
  bool checkConvergence();
  void writeAlignParms();

 protected:
  // loading configuration
  void loadConfiguration(std::string alignConfigFile);
  void getAlignmentDetectors(const YAML::Node& conf);
  void makeAlignmentDetList();
  void getAlignmentAlgName(const YAML::Node& conf);
  void overrideRoadWidth();

  int _debugLevel;
  std::string _alignConfigFile;

  SbtConfig* _configurator;
  SbtAlignmentAlg* _alignmentAlg;

  std::string _alignmentAlgName;
  SbtEnums::alignMode _mode;
  std::vector<SbtDetectorElem*> _alignDetList;
  unsigned int _nDetToBeAligned;
  std::vector<int> _detToBeAlignedIDList;

  std::string _name;
  std::string _plotPath;
  std::string _alignPath;

  ClassDef(SbtAlignment, 1);
};
#endif
