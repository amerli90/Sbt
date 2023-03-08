#ifndef SBTBIT_OPERATIONS_HH
#define SBTBIT_OPERATIONS_HH

#include <stdint.h>

#define MASTER_ID 0xeda03001

#define MARKER_EVT_HEADER 0xaa1234aa  // testbeam 2011
//#define MARKER_EVT_HEADER       0xcc1234cc // testbeam 2008
#define EVT_HEADER_SIZE 12  // testbeam 2011
//#define EVT_HEADER_SIZE         8          // testbeam 2008

#define MARKER_ROB_HEADER 0xdd1234dd
#define ROB_PER_EVT 2
#define ROB_HEADER_SIZE 10

#define MARKER_ROD_HEADER 0xee1234ee
#define ROD_HEADER_SIZE 9

#define EVT_NEXT 0x1234cccc

#define MARKER_ROD_ENDDATA 0xb1eb1e0f
#define RUN_END 0x1234dddd

#define ALLBITSUP 0xffffffff

#define MASK_EVTCOUNTER 0xfffff000  // 1111 1111 1111 1111 1111 0000 0000 0000
#define MASK_EVTCOUNTER_FLAGS \
  0x00000f00                         // 0000 0000 0000 0000 0000 1111 0000 0000
#define MASK_MASTER_EDRO 0x20000000  // 0000 0000 0000 0000 0000 1000 0000 0000
#define MASK_SCINT_FLAG 0x10000000   // 0000 0000 0000 0000 0000 0100 0000 0000

// Triggers
#define TRIG_INT 0x00020000  // 0000 0000 0000 0000 0000 0000 0100 0000
                                       // 0000
#define TRIG_EXT 0x00010000  // 0000 0000 0000 0000 0000 0000 0010 0000
#define TRIGGERDATA 0x0a200000      // 0000 1010 0010 0000 0000 0000 0000 0000
#define MASK_TRACKWORDS 0x0000ff00  // 0000 0000 0000 0000 1111 1111 0000 0000
#define MASK_TIMESTAMP 0x0000001f   // 0000 0000 0000 0000 0000 0000 0001 1111
#define MASK_HITS_TIMESTAMP \
  0x001f0000  // 0000 0000 0001 1111 0000 0000 0000 0000
#define MASK_SCINT_TIMESTAMP \
  0x1f000000  // 0001 1111 0000 0000 0000 0000 0000 0000
#define MASK_HITSZ 0xff000000
#define MASK_HITSW 0x00ff0000
#define MASK_HITSY 0x0000ff00
#define MASK_HITSX 0x000000ff
//#define LASTWORD                0x40000000              //0100 0000 0000 0000
//0000 0000 0000 0000
#define LASTWORD 0x80000000          // 1000 0000 0000 0000 0000 0000 0000
#define HITBLOCK 0xd8000000          // 1101 1000 0000 0000 0000 0000 0000 0000
#define STARTTRIGGERDATA 0xa0a00000  // 1010 0000 1010 0000 0000 0000 0000 0000
#define ENDTRIGGERDATA 0x80000000    // 1000 0000 0000 0000 0000 0000 0000

// input lines
#define MASK_INPUTLINE 0x07000000     // 0000 0111 0000 0000 0000 0000 0000 0000
#define MASK_RECEIVEDHITS 0x000fffff  // 0000 0000 0001 1111 1111 1111 1111 1111
#define MASK_HITBLOCK_WORDS \
  0x00003f00                          // 0000 0000 0000 0000 0011 1111 0000 0000
#define HITBLOCK_LASTWORD 0x00000001  // 0000 0000 0000 0000 0000 0000 0000 0001

////////////////// Some macros ////////////////////
// general
#define RAISEALLBITS(x) (x = ALLBITSUP)
#define RESET_MASK(x) (x = ~ALLBITSUP)

#define START_EVT(x) (x == MARKER_EVT_HEADER)
#define END_EVT(x) ((x & MARKER_ROD_ENDDATA) == MARKER_ROD_ENDDATA)

#define ROB_HEADER(x) (x == MARKER_ROB_HEADER)

#define ROD_HEADER(x) ((x & MARKER_ROD_HEADER) == MARKER_ROD_HEADER)
#define IS_MARKER_ROD_ENDDATA(x) \
  ((x & MARKER_ROD_ENDDATA) == MARKER_ROD_ENDDATA)

// triggers
#define TRIG_IS_INT(x) ((x & TRIG_INT) == TRIG_INT)
#define TRIG_IS_EXT(x) ((x & TRIG_EXT) == TRIG_EXT)
#define IS_STARTTRIGGERDATA(x) ((x & STARTTRIGGERDATA) == STARTTRIGGERDATA)
#define IS_TRIGGERDATA(x) ((x & TRIGGERDATA) == TRIGGERDATA)
#define IS_TRIGGERLASTWORD(x) ((x & LASTWORD) == LASTWORD)
#define IS_HITBLOCK(x) ((x & HITBLOCK) == HITBLOCK)
#define IS_HITBLOC_LASTWORD(x) ((x & HITBLOCK_LASTWORD) == HITBLOCK_LASTWORD)

// masks
#define GET_EVT_COUNTER(x) ((x & MASK_EVTCOUNTER) >> 12)
#define GET_EVT_FLAGS(x) ((x & MASK_EVTCOUNTER_FLAGS) >> 8)
#define GET_MASTER_EDRO(x) ((x & MASK_MASTER_EDRO) >> 11)
#define GET_SCINT_FLAG(x) ((x & MASK_SCINT_FLAG) >> 10)

// Watch out!
#define GET_TIMESTAMP(x) (x & MASK_TIMESTAMP)
#define GET_HITS_TIMESTAMP(x) ((x & MASK_HITS_TIMESTAMP) >> 16)
#define GET_SCINT_TIMESTAMP(x) ((x & MASK_SCINT_TIMESTAMP) >> 24)
#define GET_TRACKWORDS(x) ((x & MASK_TRACKWORDS) >> 8)
#define GET_INPUTLINE(x) ((x & MASK_INPUTLINE) >> 24)
#define GET_RECEIVEDHITS(x) (x & MASK_RECEIVEDHITS)
#define GET_HITBLOCK_WORDS(x) ((x & MASK_HITBLOCK_WORDS) >> 8)
#define GET_HITS_Z(x) ((x & MASK_HITSZ) >> 24)
#define GET_HITS_W(x) ((x & MASK_HITSW) >> 20)
#define GET_HITS_X(x) ((x & MASK_HITSX) >> 16)
#define GET_HITS_Y(x) ((x & MASK_HITSY) >> 12)

///////////////////////////////////////////////////

typedef uint32_t _CHUNK;
typedef uint32_t _BLOCK;

///////////////////////////////////////////////////
// MAPS & FSSR Decoding
///////////////////////////////////////////////////
//#define A_COL_MASK    0x0000003E        // testbeam 2008
//#define R_COL_MASK    0x000000C0        // testbeam 2008
#define A_COL_MASK 0x000000f8
#define R_COL_MASK 0x00000006
#define ROW_MASK 0x00001F00
#define HIT_CN_MASK 0x00006000  // CHIP NUMBER RELATIVE TO THE EPMC
#define HIT_TS_MASK 0x00FF0000  // TIME STAMP OF THE HIT

//#define GET_PX_ACOL(x)      ( (x & A_COL_MASK)  >>1) //testbeam 2008
//#define GET_PX_RCOL(x)      ( (x & R_COL_MASK)  >>6) //testbeam 2008
#define GET_PX_ACOL(x) ((x & A_COL_MASK) >> 3)
#define GET_PX_RCOL(x) ((x & R_COL_MASK) >> 1)
#define GET_PX_ROW(x) ((x & ROW_MASK) >> 8)                   // y
#define GET_PX_COL(x) ((x & (R_COL_MASK | A_COL_MASK)) >> 1)  // x
#define GET_HIT_TS(x) ((x & HIT_TS_MASK) >> 16)

// FSSR2
#define ADC_MASK 0x0000000e
#define CHIP_MASK 0x00000070
#define SET_MASK 0x00000f80
#define STRIP_MASK 0x0000f000

#define GET_ADC(x) ((x & ADC_MASK) >> 1)
#define GET_CHIP(x) ((x & CHIP_MASK) >> 4)
#define GET_SET(x) ((x & SET_MASK) >> 7)
#define GET_STRIP(x) ((x & STRIP_MASK) >> 12)

#endif /* _BIT_OPERATIONS_H */
