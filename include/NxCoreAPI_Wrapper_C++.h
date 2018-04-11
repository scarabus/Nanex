#ifndef NXCORE_API_CLASS_H
#define NXCORE_API_CLASS_H

// It is not necessary to use this file to write to the NxCore API. It contains a wrapper class that makes it easier
// for some C++ developers. The only include file necessary to interface to NxCore API is NxCoreAPI.h
// We have also included headers needed to load/unload and provide simple status when the NxCoreAPI's processTape() has completed.

#include <iostream>    // cout, endl
#include <cstring> 	   // memset
#include <dlfcn.h>     // dlsym, dlopen, dlclose
#include "NxCoreAPI.h" // main NxCore API header

using namespace std;

////////////////////////////////////////////////////
////// NxCoreProcAddr<T> utility function   ////////
////////////////////////////////////////////////////
template <class T> T NxCoreProcAddr(void* NxLib,char *pfName)
{
  return (T) ::dlsym(NxLib,pfName);
}

////////////////////////////////////////////////////
////// NxCoreClass: Wrapper for NxCoreAPI.dll //////
////////////////////////////////////////////////////
struct NxCoreClass	{
private:
  void*						  NxLib;
  NxCoreAPIVersion				  pfAPIVersion;
  NxCoreProcessTape				  pfProcessTape;
  NxCoreListTapes				  pfListTapes;
  NxCoreListAPIDLLs				  pfListAPIDLLs;
  NxCoreSpinSymbols				  pfSpinSymbols;
  NxCoreSaveState				  pfSaveState;
  NxCoreGetStateData			  pfGetStateData;
  NxCoreStateGetExgQuotes		  pfStateGetExgQuotes;
  NxCoreStateGetMMQuotes		  pfStateGetMMQuotes;
  NxCoreStateGetLastTrade		  pfStateGetLastTrade;
  NxCorePriceConvert			  pfPriceConvert;
  NxCorePriceFormat				  pfPriceFormat;
  NxCorePriceToDouble			  pfPriceToDouble;
  NxCoreDateFromNDays			  pfDateFromNDays;
  NxCoreDateFromYMD				  pfDateFromYMD;
  NxCoreGetDefinedString		  pfGetDefinedString;
  NxCoreGetTapeName				  pfGetTapeName;
  NxCoreSignalWaitEvent			  pfSignalWaitEvent;
  NxCoreSetCallback				  pfSetCallback;
  NxCoreGetSymbolAtom32			  pfGetSymbolAtom32;
public:
  NxCoreClass()
  {
	memset(this,0,sizeof(*this));
  }

 ~NxCoreClass()
  {
	UnloadNxCore();
  }

 // must call LoadNxCore before calling NxCoreAPI functions.
 // LoadNxCore version of NxCoreAPI if successful, 0 on failure
  unsigned int LoadNxCore(const char* cszNxCoreLib)
  {
	bool fret = false;
	UnloadNxCore();
	NxLib = ::dlopen(cszNxCoreLib, RTLD_LAZY);
	if( NxLib ) {
		fret    |=	(pfAPIVersion  = (NxCoreAPIVersion)		dlsym(NxLib, cszNxCoreAPIVersion)) == 0;
		fret    |=	(pfProcessTape = (NxCoreProcessTape) 	dlsym(NxLib, cszNxCoreProcessTape)) == 0;
		fret    |=	(pfListTapes = (NxCoreListTapes) dlsym(NxLib,            cszNxCoreListTapes)) == 0;
		fret    |=	(pfListAPIDLLs = (NxCoreListAPIDLLs) dlsym(NxLib,              cszNxCoreListAPIDLLs)) == 0;
		fret    |=	(pfSpinSymbols = (NxCoreSpinSymbols) dlsym(NxLib,              cszNxCoreSpinSymbols)) == 0;
		fret    |=	(pfSaveState = (NxCoreSaveState) dlsym(NxLib,                cszNxCoreSaveState)) == 0;
		fret    |=	(pfStateGetExgQuotes =  (NxCoreStateGetExgQuotes) dlsym(NxLib,        cszNxCoreStateGetExgQuotes)) == 0;
		fret    |=	(pfStateGetMMQuotes = (NxCoreStateGetMMQuotes) dlsym(NxLib,         cszNxCoreStateGetMMQuotes)) == 0;
		fret    |=	(pfStateGetLastTrade = (NxCoreStateGetLastTrade) dlsym(NxLib,        cszNxCoreStateGetLastTrade)) == 0;
		fret    |=	(pfPriceConvert = (NxCorePriceConvert) dlsym(NxLib,             cszNxCorePriceConvert)) == 0;
		fret    |=	(pfPriceFormat = (NxCorePriceFormat) dlsym(NxLib,              cszNxCorePriceFormat)) == 0;
		fret    |=	(pfPriceToDouble = (NxCorePriceToDouble) dlsym(NxLib,            cszNxCorePriceToDouble)) == 0;
		fret    |=	(pfDateFromNDays = (NxCoreDateFromNDays) dlsym(NxLib,            cszNxCoreDateFromNDays)) == 0;
		fret    |=	(pfDateFromYMD = (NxCoreDateFromYMD) dlsym(NxLib,              cszNxCoreDateFromYMD)) == 0;
		fret    |=	(pfGetDefinedString = (NxCoreGetDefinedString) dlsym(NxLib,         cszNxCoreGetDefinedString)) == 0;
		fret    |=	(pfGetTapeName = (NxCoreGetTapeName) dlsym(NxLib,              cszNxCoreGetTapeName)) == 0;
		fret    |=	(pfGetStateData = (NxCoreGetStateData) dlsym(NxLib,             cszNxCoreGetStateData)) == 0;
		fret    |=	(pfSetCallback = (NxCoreSetCallback) dlsym(NxLib,              cszNxCoreSetCallback)) == 0;
		if (!fret){
			return pfAPIVersion();
		}
		else{
			return 0;
		}
	}

  }


  void UnloadNxCore()
  {
	if( NxLib )
	  ::dlclose(NxLib);
	memset(this,0,sizeof(*this));
  }

  ///// NxCoreAPI Functions available after successful call to LoadNxCore	/////////
  unsigned int APIVersion()
  {
	return pfAPIVersion();
  }

  int ProcessTape(const char* pszFilename,const char* pMemoryAddr,unsigned int controlFlags,int UserData,NxCoreCallback callbackFunction)
  {
    return pfProcessTape(pszFilename,pMemoryAddr,controlFlags,UserData,callbackFunction);
  }

  int ListTapes(unsigned int controlFlags,NxCoreCallbackTapeList stdcallbacklist,void* pYourParam)
  {
    return pfListTapes(controlFlags,stdcallbacklist,pYourParam);
  }

  int ListAPIDLLs(unsigned int controlFlags,NxCoreCallbackAPIList stdcallbacklist,void* pYourParam)
  {
    return pfListAPIDLLs(controlFlags,stdcallbacklist,pYourParam);
  }

  int SpinSymbols(unsigned short ListedExg,unsigned int controlFlags,unsigned int spinID,NxCoreCallback tempCallback=0,int tempUserData=0)
  {
    return pfSpinSymbols(ListedExg,controlFlags,spinID,tempCallback,tempUserData);
  }

  int SaveState(const char* szStateFilename,unsigned int controlFlags)
  {
    return pfSaveState(szStateFilename,controlFlags);
  }

  int GetStateData(char* pBuffer,int bufferSize,int stateType,int param1,int param2,NxString* pnxsSymOrCtc)
  {
    return pfGetStateData(pBuffer,bufferSize,stateType,param1,param2,pnxsSymOrCtc);
  }

  int StateGetExgQuotes(NxCoreStateExgQuotes* pStateExgQuotes,NxString* pnxsSymOrDateStrike=0)
  {
    return pfStateGetExgQuotes(pStateExgQuotes,pnxsSymOrDateStrike);
  }

  int StateGetMMQuotes(unsigned short ReportingExg,NxCoreStateMMQuotes* pStateMMQuotes,NxString* pnxsSymOrDateStrike=0)
  {
    return pfStateGetMMQuotes(ReportingExg,pStateMMQuotes,pnxsSymOrDateStrike);
  }

  int StateGetLastTrade(NxCoreStateTrade* pStateTrade,NxString* pnxsSymOrDateStrike=0)
  {
    return pfStateGetLastTrade(pStateTrade,pnxsSymOrDateStrike);
  }

  int PriceConvert(int lPrice,unsigned char PriceType,unsigned char PriceTypeNew)
  {
    return pfPriceConvert(lPrice,PriceType,PriceTypeNew);
  }

  int PriceFormat(char* szBuff,int lPrice,unsigned char PriceType,int expandWidth = 0,bool bCommas = false)
  {
    return pfPriceFormat(szBuff,lPrice,PriceType,expandWidth,bCommas);
  }

  double PriceToDouble(int lPrice,unsigned char PriceType)
  {
    return pfPriceToDouble(lPrice,PriceType);
  }

  void DateFromNDays(NxDate* pnxDate)
  {
    pfDateFromNDays(pnxDate);
  }

  void DateFromYMD(NxDate* pnxDate)
  {
    pfDateFromYMD(pnxDate);
  }

  const char* GetDefinedString(int ixTable,int ixString)
  {
    return pfGetDefinedString(ixTable,ixString);
  }

  int GetTapeName(char* szBuffTapename,int nBufferBytes)
  {
    return pfGetTapeName(szBuffTapename,nBufferBytes);
  }

  int SignalWaitEvent(const NxCoreSystem* pNxCoreSystem,bool bAlwaysSignal)
  {
	return pfSignalWaitEvent(pNxCoreSystem,bAlwaysSignal);
  }

  NxCoreCallback SetCallback(NxCoreCallback callbackFunction)
  {
    return pfSetCallback ? pfSetCallback(callbackFunction): 0;
  }

  int GetSymbolAtom32()
  {
	return pfGetSymbolAtom32 ? pfGetSymbolAtom32(): -2;
  }

  /**
   * Helper functions for getting the major/minor/build versions of the libnx.so
   */
  unsigned int inline GetMajorVersion(int soVersion) {
	return (soVersion) >> 24;
  }

  unsigned int inline GetMinorVersion(int soVersion) {
	return (soVersion >> 16) & 0x000000ff;
  }

  unsigned int inline GetBuildVersion(int soVersion) {
	return soVersion & 0x0000ffff;
  }

  /**
   * Helper function that outputs human-readable values based on the return from processTape()
   */
  static void ProcessReturnValue(int returnValue) {
      switch(returnValue) {
          case NxAPIERR_NO_ERROR: // 0
              cout << "Tape completed normally." << endl;
              break;
          case NxAPIERR_USER_STOPPED: // 1
              cout << "NxCALLBACKRETURN_STOP was returned from callback function." << endl;
              break;
          case NxAPIERR_NOT_CALLBACK_THREAD: // -1
              cout << "Function was called from thread other than callback thread." << endl;
              break;
          case NxAPIERR_BAD_PARAMETERS: // -2
              cout << "Function was called from thread other than callback thread." << endl;
              break;
          case NxAPIERR_EXCEPTION: // -3
              cout << "An exception occurred." << endl;
              break;
          case NxAPIERR_OPEN_TAPE_FILE_FAILED: // -4
              cout << "Could not open the file specified as parameter 1." << endl;
              break;
          case NxAPIERR_INITIALIZE_MEMORY_FAILED: // -5
              cout << "Could not initialize memory. This can occur when calling processTape with a saved state file and the memory address used to save the state has been used by another dll or within your process." << endl;
              break;
          case NxAPIERR_NOLISTED_EXG: // -6
              cout << "Symbol spin -- listed exchange does not exist in current tape" << endl;
              break;
           case NxAPIERR_NOSYMBOLS_FOR_LSTEXG: // -7
              cout << "Symbol spin -- no symbols of the type (options/not options) for the exchange specified" << endl;
              break;
           case NxAPIERR_NOSESSION_FOR_SYMBOL: // -8
              cout << "Symbol spin -- no session or session does not have data type" << endl;
              break;
           case NxAPIERR_NODATATYPE_FOR_SESSION: // -9
              cout << "Symbol spin -- There's a session, but no trade/quote/mmquote data for session" << endl;
              break;
           case NxAPIERR_NODATA_FOR_REPEXG: // -10
              cout << "Symbol spin -- MMQuotes. There is session data, but no entry for the specified rep. exg" << endl;
              break;
           case NxAPIERR_ZEROED_DATA_FOR_SESSION: // -11
              cout << "Symbol spin -- there is a session, but data is all zero" << endl;
              break;
           case NxAPIERR_SAVE_STATE_IN_PROGRESS: // -12
              cout << "Save state -- a state save is currently in progress" << endl;
              break;
           case NxAPIERR_NOT_SUPPORTED: // -13
              cout << "General error" << endl;
              break;
           case NxAPIERR_INITALLOC_ERROR: // -14
              cout << "Startup initial allocation error" << endl;
              break;
           case NxAPIERR_NSODALLOC_ERROR: // -15
              cout << "Startup pNxSOD allocation error" << endl;
              break;
           default:
              cout << "Unknown return value: (" << returnValue<< ")" << endl;
              break;
      }
  }
};

#endif // NXCORE_API_CLASS_H
