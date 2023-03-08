// class for trigger info -- it will hold the information for the trigger mask

#ifndef SBTTRIGGERINFO_HH
#define SBTTRIGGERINFO_HH

#include <bitset>
#include <vector>

class SbtTriggerInfo {
 public:
  SbtTriggerInfo(unsigned long aTrigMask) : _triggerMask(aTrigMask) {}

 protected:
  std::bitset<24> _triggerMask;

  ClassDef(SbtTriggerInfo, 1);
};

#endif
