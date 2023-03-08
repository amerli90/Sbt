#ifndef SBTERROR_MANAGEMENT_HH
#define SBTERROR_MANAGEMENT_HH

#include <string>
#include <vector>

#include "SbtBit_operations.h"

typedef enum {
  kGENERIC_ERROR = 0,
  kEVENT_INDEX,
  kEVENT_NOT_STARTED,
  kEVENT_ALREADY_STARTED,
  kROB_NOT_STARTED,
  kROD_NOT_STARTED,
  kIMPOSSIBLE_NEW_EVENT,
  kROD_HEADER_NOT_FOUND,
  kWRONG_ORDINAL,
  kWRONG_FLAGS,
  kBCO,
  kBX,
  kLVL1ID,
  kTIMESTAMP,
  kTIMESTAMPS_NON_CONSISTENT,
  kTRIGGER_LESS,
  kTRIGGER_MORE,
  kINPUT_LINE,
  kLAYER_LAST_WORD,
  kHITBLOCK,
  kEVENT_LENGTH,
  kCHECKSUM,
  kWRONGNROBS,

  kNUMBER_OF_KINDS
} KIND_OF_ERROR;

const std::string errorStrings[kNUMBER_OF_KINDS] = {
    "Generic error",
    "Invalid event index",
    "Event not yet started",
    "Event already started",
    "ROB not yet started",
    "ROD not yet started",
    "Impossible to create new event",
    "ROD header not found",
    "Event ordinal number doesn\'t match number of events found",
    "Event flags are different from ones of the previous event",
    "BCO counter doesn\'t follow the BCO counter of the previous event",
    "BX counter doesn\'t follow the BX counter of the previous event",
    "Lvl1ID counter doesn\'t follow the LVL1ID counter of the previous event",
    "Wrong Timestamp",
    "Non consistent timestamps",
    "Less trigger informations than expected",
    "More trigger informations than expected",
    "Selected input line doesn\'t exist",
    "Layer last word reached but bit 23 is not set",
    "Hit block for selected line has fewer words than expected",
    "Event length is different from calculated length",
    "Wrong checksum",
    "Wrong number of ROBs"};

struct _ERROR {
  KIND_OF_ERROR id;
  const char* description;
  _BLOCK position;
  _CHUNK raw_block;
  unsigned long counts;
  _ERROR& operator++() {
    counts++;
    return *this;
  }

  _ERROR operator++(int) {
    _ERROR err = *this;
    counts++;
    return err;
  };  // postfix
  void print() { printf("@ %s\n", description); }
};

typedef std::vector<_ERROR> errorDB;

#endif /* _ERROR_MANAGEMENT_ */
