#ifndef NX_CSUPPORT_C 
#define NX_CSUPPORT_C 
 
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <stdbool.h>

/***************************************************************************** 
 * Makes NX compatible with standard C and includes Nanex-provided headers.  * 
 *****************************************************************************/ 
 
typedef struct NxDate NxDate; 
typedef struct NxTime NxTime; 
typedef struct NxString NxString; 
typedef struct NxAccessStatus NxAccessStatus; 
typedef struct NxCoreSystem NxCoreSystem; 
typedef struct NxCoreQuote NxCoreQuote; 
typedef struct NxCoreExgQuote NxCoreExgQuote; 
typedef struct NxCoreMMQuote NxCoreMMQuote; 
typedef struct NxCTAnalysis NxCTAnalysis; 
typedef struct NxCoreTrade NxCoreTrade; 
typedef struct NxPrice NxPrice; 
typedef struct StringTableItem StringTableItem; 
typedef struct NxCategoryField NxCategoryField; 
typedef struct NxCoreCategory NxCoreCategory; 
typedef struct NxOptionHdr NxOptionHdr; 
typedef struct NxCoreSymbolChange NxCoreSymbolChange; 
typedef struct NxCoreSymbolSpin NxCoreSymbolSpin; 
typedef union NxCoreData NxCoreData; 
typedef struct NxCoreHeader NxCoreHeader; 
typedef struct NxCoreMessage NxCoreMessage; 
typedef struct NxCoreStateExgQuote NxCoreStateExgQuote; 
typedef struct NxCoreStateMMQuote NxCoreStateMMQuote; 
typedef struct NxCoreStateExgQuotes NxCoreStateExgQuotes; 
typedef struct NxCoreStateMMQuotes NxCoreStateMMQuotes; 
typedef struct NxCoreStateTrade NxCoreStateTrade; 
typedef struct NxCoreStateOHLCVTrade NxCoreStateOHLCVTrade; 
typedef struct NxCoreTapeFile NxCoreTapeFile; 
typedef struct NxCoreAPIDLLFile NxCoreAPIDLLFile; 
typedef struct NxCore3Ext NxCore3Ext;

#include "NxCoreAPI.h"

NxCoreAPIVersion pfNxCoreAPIVersion; 
NxCoreProcessTape pfNxCoreProcessTape; 
NxCoreListTapes pfNxCoreListTapes; 
NxCoreListAPIDLLs pfNxCoreListAPIDLLs; 
NxCoreSpinSymbols pfNxCoreSpinSymbols; 
NxCoreSaveState pfNxCoreSaveState; 
NxCoreGetStateData pfNxCoreGetStateData; 
NxCoreStateGetExgQuotes pfNxCoreStateGetExgQuotes; 
NxCoreStateGetMMQuotes pfNxCoreStateGetMMQuotes; 
NxCoreStateGetLastTrade pfNxCoreStateGetLastTrade; 
NxCorePriceConvert pfNxCorePriceConvert; 
NxCorePriceFormat pfNxCorePriceFormat; 
NxCorePriceToDouble pfNxCorePriceToDouble; 
NxCoreDateFromNDays pfNxCoreDateFromNDays; 
NxCoreDateFromYMD pfNxCoreDateFromYMD; 
NxCoreGetDefinedString pfNxCoreGetDefinedString; 
NxCoreGetTapeName pfNxCoreGetTapeName; 
NxCoreSignalWaitEvent pfNxCoreSignalWaitEvent; 
NxCoreSetCallback pfNxCoreSetCallback; 
 
// Standard dynamic loading support 
 
#include <dlfcn.h> 
void *NxLib; 
 
/**
 * Unloads the libnx.so if it was loaded
 */
static void unloadNxCore()
{ 
  if (NxLib) 
    dlclose(NxLib); 
} 
 
/**
 * Loads the libnx.so specified
 */
static unsigned char loadNxCore(char const *FileName)
{ 
  bool fret = false; 
 
  unloadNxCore();
  if ((NxLib = dlopen(FileName, RTLD_LAZY)) != 0) { 
    fret |= (pfNxCoreAPIVersion = (NxCoreAPIVersion) dlsym(NxLib, cszNxCoreAPIVersion)) == 0; 
    fret |= (pfNxCoreProcessTape = (NxCoreProcessTape) dlsym(NxLib, cszNxCoreProcessTape)) == 0; 
    fret |= (pfNxCoreListTapes = (NxCoreListTapes) dlsym(NxLib, cszNxCoreListTapes)) == 0; 
    fret |= (pfNxCoreListAPIDLLs = (NxCoreListAPIDLLs) dlsym(NxLib, cszNxCoreListAPIDLLs)) == 0; 
    fret |= (pfNxCoreSpinSymbols = (NxCoreSpinSymbols) dlsym(NxLib, cszNxCoreSpinSymbols)) == 0;
    fret |=	(pfNxCoreSaveState = (NxCoreSaveState) dlsym(NxLib, cszNxCoreSaveState)) == 0;
    fret |= (pfNxCoreStateGetExgQuotes = (NxCoreStateGetExgQuotes) dlsym(NxLib, cszNxCoreStateGetExgQuotes)) == 0; 
    fret |= (pfNxCoreStateGetMMQuotes = (NxCoreStateGetMMQuotes) dlsym(NxLib, cszNxCoreStateGetMMQuotes)) == 0; 
    fret |= (pfNxCoreStateGetLastTrade = (NxCoreStateGetLastTrade) dlsym(NxLib, cszNxCoreStateGetLastTrade)) == 0; 
    fret |= (pfNxCoreGetDefinedString = (NxCoreGetDefinedString) dlsym(NxLib, cszNxCoreGetDefinedString)) == 0; 
    fret |= (pfNxCoreGetTapeName = (NxCoreGetTapeName) dlsym(NxLib, cszNxCoreGetTapeName)) == 0; 
    fret |= (pfNxCoreDateFromNDays = (NxCoreDateFromNDays) dlsym(NxLib, cszNxCoreDateFromNDays)) == 0; 
    fret |= (pfNxCoreDateFromYMD = (NxCoreDateFromYMD) dlsym(NxLib, cszNxCoreDateFromYMD)) == 0; 
    fret |= (pfNxCorePriceConvert = (NxCorePriceConvert) dlsym(NxLib, cszNxCorePriceConvert)) == 0; 
    fret |= (pfNxCorePriceFormat = (NxCorePriceFormat) dlsym(NxLib, cszNxCorePriceFormat)) == 0; 
    fret |= (pfNxCorePriceToDouble = (NxCorePriceToDouble) dlsym(NxLib, cszNxCorePriceToDouble)) == 0; 

    if (!fret) { 
      fret |= (pfNxCoreSetCallback = (NxCoreSetCallback) dlsym(NxLib, cszNxCoreSetCallback)) == 0; 
      fret |= (pfNxCoreGetStateData = (NxCoreGetStateData) dlsym(NxLib, cszNxCoreGetStateData)) == 0; 
    } else {
    	unloadNxCore();
    }

  } 
  return (NxLib ? 1 : 0); 
} 

/**
 * Helper function that outputs human-readable values based on the return from processTape()
 */
static void processReturnValue(int returnValue) {
    switch(returnValue) {
        case NxAPIERR_NO_ERROR: // 0
            printf("Tape completed normally.\n");
            break;
        case NxAPIERR_USER_STOPPED: // 1
            printf("NxCALLBACKRETURN_STOP was returned from callback function.\n");
            break;
        case NxAPIERR_NOT_CALLBACK_THREAD: // -1
            printf("Function was called from thread other than callback thread.\n");
            break;
        case NxAPIERR_BAD_PARAMETERS: // -2
            printf("Function was called from thread other than callback thread.\n");
            break;
        case NxAPIERR_EXCEPTION: // -3
            printf("An exception occurred.\n");
            break;
        case NxAPIERR_OPEN_TAPE_FILE_FAILED: // -4
            printf("Could not open the file specified as parameter 1.\n");
            break;
        case NxAPIERR_INITIALIZE_MEMORY_FAILED: // -5
            printf("Could not initialize memory. This can occur when calling processTape with a saved state file and the memory address used to save the state has been used by another dll or within your process.\n");
            break;
        case NxAPIERR_NOLISTED_EXG: // -6
            printf("Symbol spin -- listed exchange does not exist in current tape\n");
            break;
         case NxAPIERR_NOSYMBOLS_FOR_LSTEXG: // -7
            printf("Symbol spin -- no symbols of the type (options/not options) for the exchange specified\n");
            break;
         case NxAPIERR_NOSESSION_FOR_SYMBOL: // -8
            printf("Symbol spin -- no session or session does not have data type\n");
            break;
         case NxAPIERR_NODATATYPE_FOR_SESSION: // -9
            printf("Symbol spin -- There's a session, but no trade/quote/mmquote data for session\n");
            break;
         case NxAPIERR_NODATA_FOR_REPEXG: // -10
            printf("Symbol spin -- MMQuotes. There is session data, but no entry for the specified rep. exg\n");
            break;
         case NxAPIERR_ZEROED_DATA_FOR_SESSION: // -11
            printf("Symbol spin -- there is a session, but data is all zero\n");
            break;
         case NxAPIERR_SAVE_STATE_IN_PROGRESS: // -12
            printf("Save state -- a state save is currently in progress\n");
            break;
         case NxAPIERR_NOT_SUPPORTED: // -13
            printf("General error\n");
            break;
         case NxAPIERR_INITALLOC_ERROR: // -14
            printf("Startup initial allocation error\n");
            break;
         case NxAPIERR_NSODALLOC_ERROR: // -15
            printf("Startup pNxSOD allocation error\n");
            break;
         default:
            printf("Unknown return value: (%d)",
                   returnValue);
            break;
    }
}


#endif // NX_CSUPPORT_C 
