#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <signal.h>

#include "./NxCoreAPI_Wrapper_C.h"
#include "./NxCoreAPI.h"

// JTime Time Structure
typedef struct {
	uint8_t Hour;
	uint8_t Minute;
	uint8_t Second;
	uint16_t Millisecond;
	uint16_t Microsecond;
	uint32_t MsOfDay;
	uint64_t MicrosOfDay;
} JTime;

char *fileName = NULL;
int lastHour = -1;
bool headers = true;
char *bucket = NULL;

FILE *fp_info = NULL,*fp_trade = NULL,*fp_exg = NULL,*fp_mm = NULL,*fp_opt = NULL,*fp_spin = NULL;

char * fname_info = NULL;
char * fname_trade = NULL;
char * fname_opt = NULL;


char *exchanges[] = { "Composite", "NQEX", "NQAD", "NYSE", "AMEX", "CBOE",
"ISEX", "PACF", "CINC", "PHIL", "OPRA", "BOST", "NQNM", "NQSC", "NQBB",
"NQPK", "NQAG", "CHIC", "TSE", "CDNX", "CME", "NYBT", "MRCY", "COMX",
"CBOT", "NYMX", "KCBT", "MGEX", "WCE", "ONEC", "DOWJ", "GEMI", "SIMX",
"FTSE", "EURX", "ENXT", "DTN", "LMT", "LME", "IPEX", "MX", "WSE", "C2",
"MIAX", "CLRP", "BARK", "TEN4", "NQBX", "HOTS", "EUUS", "EUEU", "ENCM",
"ENID", "ENIR", "CFE", "PBOT", "HWTB", "NQNX", "BTRF", "NTRF", "BATS",
"NYLF", "PINK", "BATY", "EDGE", "EDGX", "RUSL", "CMEX", "IEX", "PERL", "LSE" };

char *conditions[] = { "Regular", "FormT", "OutOfSeq", "AvgPrc", "AvgPrc_Nasdaq", "OpenReportLate", "OpenReportOutOfSeq",
"OpenReportInSeq", "PriorReferencePrice", "NextDaySale", "Bunched", "CashSale", "Seller", "SoldLast", "Rule127", "BunchedSold",
"NonBoardLot", "POSIT", "AutoExecution", "Halt", "Delayed", "Reopen", "Acquisition", "CashMarket", "NextDayMarket", "BurstBasket",
"OpenDetail", "IntraDetail", "BasketOnClose", "Rule155", "Distribution", "Split", "Reserved", "CustomBasketCross", "AdjTerms",
"Spread", "Straddle", "BuyWrite", "Combo", "STPD", "CANC", "CANCLAST", "CANCOPEN", "CANCONLY", "CANCSTPD", "MatchCross", "FastMarket",
"Nominal", "Cabinet", "BlankPrice", "NotSpecified", "MCOfficialClose", "SpecialTerms", "ContingentOrder", "InternalCross", "StoppedRegular",
"StoppedSoldLast", "StoppedOutOfSeq", "Basis", "VWAP", "SpecialSession", "NanexAdmin", "OpenReport", "MarketOnClose", "Not Defined",
"OutOfSeqPreMkt", "MCOfficialOpen", "FuturesSpread", "OpenRange", "CloseRange", "NominalCabinet", "ChangingTrans", "ChangingTransCab",
"NominalUpdate", "PitSettlement", "BlockTrade", "ExgForPhysical", "VolumeAdjustment", "VolatilityTrade", "YellowFlag", "FloorPrice",
"OfficialPrice", "UnofficialPrice", "MidBidAskPrice", "EndSessionHigh", "EndSessionLow", "Backwardation", "Contango", "Holiday",
"PreOpening", "PostFull", "PostRestricted", "ClosingAuction", "Batch", "Trading", "IntermarketSweep", "Derivative", "Reopening",
"Closing", "CAPElection", "SpotSettlement", "BasisHigh", "BasisLow", "Yield", "PriceVariation", "ContingentTrade (formerly StockOption)",
"StoppedIM", "Benchmark", "TradeThruExempt", "Implied", "OTC", "MktSupervision", "RESERVED_77", "RESERVED_91", "ContingentUTP",
"OddLot", "RESERVED_89", "CorrectedCSLast", "OPRAExtHours", "RESERVED_78", "RESERVED_81", "RESERVED_84", "RESERVED_878", "RESERVED_90",
"QualifiedContingentTrade", "MultipleSpreadLegsNotUnbundleable", "UnbundledSpreadLeg", "NEW_TC_127_80", "NEW_TC_128_88",
"NEW_TC_129_85", "NEW_TC_130_82", "NEW_TC_131_84" };

/**
 * OnNxCoreStatus: Handler for NxCore Status messages
 *
 * Not sure if these are particularly relevent
 * but use the timestamp to decide when to close and submit the files
 */
void OnNxCoreStatus(const NxCoreSystem* coreSys,const NxCoreMessage* coreMsg) {
	char buff[MAXPATHLEN];
	char url[MAXPATHLEN];

	char *host = "http://localhost:8082";
	char *dataset = "WyJEYXRhU2V0Iiw1MTkzNTMzMDk1OTM2MDAwXQ";

	strcpy(&url[0], fileName);
	char *c = strchr(&url[0], '.');
	if (c != NULL) *c = '\0';
	snprintf(&buff[0], MAXPATHLEN, "%s.%s", dataset, &url[0]);
	char *bqtable = strdup(&buff[0]);

	// Just log the specific NxCore status message for now.
	switch (coreSys->Status) {
	case NxCORESTATUS_COMPLETE:


		fflush(fp_info);
		fflush(fp_trade);
		fflush(fp_opt);

		fclose(fp_info);
		fclose(fp_trade);
		fclose(fp_opt);

		// Upload data to Google Storage
		snprintf(&buff[0], MAXPATHLEN, "gs://%s/nanex/%s", bucket, fname_trade);
		char *target = strdup(&buff[0]);

		int pid = fork();
		if (pid < 0) {
			perror("Cannot fork");
			exit(1);
		} else if (pid == 0) {
			fprintf(stderr, "Copy %s to %s \n", fname_trade, target);
			execl("/usr/bin/gsutil", "gsutil", "cp", fname_trade, target, NULL);
			exit(0);
		}

		int status;
		waitpid(pid, &status, 0);
		fprintf(stderr, "Child process %d returns %d\n", pid, status);

		// Use bq to create a table from the data

		pid = fork();
		if (pid < 0) {
			perror("Cannot fork");
			exit(1);
		} else if (pid == 0) {
			fprintf(stderr, "Create BQ table \n");
			fprintf(stderr, "/usr/bin/bq load --source_format=CSV --schema=./schema %s %s", bqtable, target);
			execl("/usr/bin/bq","bq", "load", "--source_format=CSV", "--schema=./schema", bqtable, target, NULL);
			exit(0);
		}

		waitpid(pid, &status, 0);
		fprintf(stderr, "Child process %d returns %d\n", pid, status);

//
//		strcpy(&url[0], fileName);
//		char *c = strchr(&url[0], '.');
//		if (c != NULL) *c = '\0';
//		char *table = strdup(&url[0]);
//
//
//
//
//
//			snprintf(&url[0], MAXPATHLEN, "%s/user/dataset/resource/pushTable/%s?tableName=%s&delimiter=,&fileName=%s",
//						host, dataset, table, &buff[0]);
//			fprintf(stderr, "\n curl %s \n\n", &url[0]);
//			execl("/usr/bin/curl", "curl", &url[0], NULL);
//			exit(0);
//		}
		fprintf(fp_info,"NxCore Complete Message.\n");
		break;

	case NxCORESTATUS_INITIALIZING:
		fprintf(fp_info,"NxCore Initialize Message.\n");
		break;

	case NxCORESTATUS_SYNCHRONIZING:
		fprintf(fp_info,"NxCore Synchronizing Message.\n");
		break;

	case NxCORESTATUS_WAITFORCOREACCESS:
		fprintf(fp_info,"NxCore Wait For Access.\n");
		break;

	case NxCORESTATUS_RESTARTING_TAPE:
		fprintf(fp_info,"NxCore Restart Tape Message.\n");
		break;

	case NxCORESTATUS_ERROR:
		fprintf(fp_info,"NxCore Error.\n");
		break;

	case NxCORESTATUS_RUNNING:
		break;
	}
}

/**
 * OnNxCoreMMQuote: Handler for NxCore Market Maker Quote messages.
 *
 * Have not encountered these yet
 **/

void OnNxCoreMMQuote(const NxCoreSystem* coreSys,
		const NxCoreMessage* coreMsg) {

/**
struct NxCoreMMQuote { // 48/56 bytes
  NxCoreQuote		coreQuote;
  NxString*			pnxStringMarketMaker;
  uint8_t	        	MarketMakerType;
  uint8_t			QuoteType;
  uint8_t			alignment[2];
};
**/

	if (fp_mm == NULL) return;

	NxCoreMMQuote* Quote = (NxCoreMMQuote*) &coreMsg->coreData.MMQuote;
	double Bid = pfNxCorePriceToDouble(Quote->coreQuote.BidPrice,Quote->coreQuote.PriceType);
	double Ask = pfNxCorePriceToDouble(Quote->coreQuote.AskPrice,Quote->coreQuote.PriceType);
	char *symbol = coreMsg->coreHeader.pnxStringSymbol->String;

	//  MMQuote for Symbol,Time,Bid,Ask,Bid Size,Ask Size,Market Maker,Exchange
	fprintf(fp_mm,"%s,%d,%02d:%02d:%02d,%0.2f,%0.2f,%d,%d,%s,%s\n",
			symbol,
			coreMsg->coreHeader.nxExgTimestamp.MsOfDay,
			coreMsg->coreHeader.nxExgTimestamp.Hour,
			coreMsg->coreHeader.nxExgTimestamp.Minute,
			coreMsg->coreHeader.nxExgTimestamp.Second,
			Bid,
			Ask,
			Quote->coreQuote.BidSize,
			Quote->coreQuote.AskSize,
			Quote->pnxStringMarketMaker->String,
			exchanges[coreMsg->coreHeader.ReportingExg]);
}

/**
 * OnGetExgQuoteStates: Retrieve the current Exchange quote list for the issue.
 * We get a LOT of these. I don't think they are useful.
 **/

void OnGetExgQuoteStates(const NxCoreSystem* coreSys, const NxCoreMessage* coreMsg) {

/**
	struct NxCoreStateExgQuote {
	  int32_t         AskPrice;
	  int32_t         BidPrice;
	  int32_t         AskSize;
	  int32_t         BidSize;
	  uint16_t        ReportingExg;
	  uint8_t         QuoteCondition;         // quote condition
	  uint8_t         alignment[1];           // 2.3.198, 2007.04.16
	};
**/

	if (fp_exg == NULL) return;

	NxCoreStateExgQuotes exchangeQuotes;
	NxString *RequestString = NULL;

	// Options are prefixed with o
	// http://nxcoreapi.com/doc/struct_NxOptionHdr.html
	if ((coreMsg->coreHeader.pnxStringSymbol->String[0] == 'o') && (coreMsg->coreHeader.pnxOptionHdr))
		RequestString = coreMsg->coreHeader.pnxOptionHdr->pnxsDateAndStrike;
	else
		RequestString = coreMsg->coreHeader.pnxStringSymbol;

	if (pfNxCoreStateGetExgQuotes(&exchangeQuotes,RequestString) == 0) {
		char *symbol = coreMsg->coreHeader.pnxStringSymbol->String;

		for (int loop = 0; loop < exchangeQuotes.StateQuoteCount; loop++) {
			double Bid = pfNxCorePriceToDouble(exchangeQuotes.StateExgQuotes[loop].BidPrice,exchangeQuotes.PriceType);
			double Ask = pfNxCorePriceToDouble(exchangeQuotes.StateExgQuotes[loop].AskPrice,exchangeQuotes.PriceType);
			int BidSize = exchangeQuotes.StateExgQuotes[loop].BidSize;
			int AskSize = exchangeQuotes.StateExgQuotes[loop].AskSize;
			unsigned short ReportingExg = exchangeQuotes.StateExgQuotes[loop].ReportingExg;
			unsigned char Condition = exchangeQuotes.StateExgQuotes[loop].QuoteCondition;

			fprintf(fp_exg,
					"%s,%d,%02d:%02d:%02d,%0.2f,%0.2f,%d,%d,%d,%d,%d,%s\n",
					symbol,
					coreMsg->coreHeader.nxExgTimestamp.MsOfDay,
					coreMsg->coreHeader.nxExgTimestamp.Hour,
					coreMsg->coreHeader.nxExgTimestamp.Minute,
					coreMsg->coreHeader.nxExgTimestamp.Second,
					Bid,
					Ask,
					BidSize,
					AskSize,
					exchangeQuotes.BestBidExg,
					exchangeQuotes.BestAskExg,
					Condition,
					exchanges[ReportingExg]);
		}
	}
}

/**
 *
 * OnGetLastTradeStates: Handle the current composite trade info for the issue, populated to the current time.
 * This seems to be the good stuff.
**/

void OnTradeStates(const NxCoreSystem* coreSys, const NxCoreMessage* coreMsg) {

/**

These are the data structures:

	struct NxCoreStateTrade {
	  uint64_t   TotalVolume;
	  uint32_t   TickVolume;
	  uint8_t    PriceType;
	  uint8_t    PriceFlags;
	  uint8_t    ConditionFlags;
	  uint8_t    VolumeType;
	  int32_t    Open;
	  int32_t    High;
	  int32_t    Low;
	  int32_t    Last;
	  int32_t    NetChange;
	  int32_t    Price;
	  int32_t    Threshold;
	  int32_t    Tick;
	};

	struct NxCoreTrade {
	  int32_t        Price;                  // price of trade report. Note: There is also a "Last" field in this structure.
	  	  	  	  	  	  	  	  	  	  	// Not all Prices update the "Last". FormT (after hours) trades are one example.
	  uint8_t        PriceType;              // Applies to Price,Open,High,Low,Last,Tick,Net Change,nxAnalysis.FilterThreshold,
	  	  	  	  	  	  	  	  	  	  	// nxAnalysis.QteMatchDistanceRGN/BBO fields in this record.
	  uint8_t        PriceFlags;             // flags indicating if trade is new high/low, etc.
	  	  	  	  	  	  	  	  	  	  	// see defines beginning with NxTPF_ .
	  uint8_t        TradeCondition;         // trade condition code assigned by exchange.
	  	  	  	  	  	  	  	  	  	  	// Use NxCoreGetDefinedString(NxST_TRADECONDITION,TradeCondition) for text name
	  uint8_t        ConditionFlags;         // converts TradeCondition (and BATE when present) to set eligibility flags
	  	  	  	  	  	  	  	  	  	  	// (defines begin with NxTCF_ )
	  uint8_t        VolumeType;             // indicates how the Size field changes (or not) the TotalVolume/TickVolume fields
	  	  	  	  	  	  	  	  	  	  	// (defines begin with NxTVT_ )
	  uint8_t        BATECode;               // 'B'id,'A'sk, or 'T'rade indicator for some commodities.
	  	  	  	  	  	  	  	  	  	  	// ('E'xception not used -- historical artifact).
	  uint8_t        alignment[2];
	  uint32_t       Size;
	  uint32_t       ExgSequence;            // original exchange sequence number.
	  uint32_t       RecordsBack;            // for inserts and deletes, indicates the number of records
	  	  	  	  	  	  	  	  	  	  	// from the last record received the cancel/insert applies.
	  NxCTAnalysis
	  uint64_t       TotalVolume;
	  uint32_t       TickVolume;
	  int32_t        Open;
	  int32_t        High;
	  int32_t        Low;
	  int32_t        Last;
	  int32_t        Tick;
	  int32_t        NetChange;
	  uint8_t        ExtTradeConditions[4];  // 2015.11.11 changed from alignment[4] to ExtTradeConditions
};

**/

	NxCoreStateTrade lastTrade;
	NxString *RequestString = NULL;
	char oei = coreMsg->coreHeader.pnxStringSymbol->String[0];

	// Is it an option?
	if ((oei == 'o') && (coreMsg->coreHeader.pnxOptionHdr != NULL)) {
		// Get the strike price from header info

	;	char OptionSymbol[25];
		char UnderlyingStr[25];
		char SeriesStr[255];

		double StrikePrice = coreMsg->coreHeader.pnxOptionHdr->strikePrice / 1000.0;

		if (coreMsg->coreHeader.pnxOptionHdr->pnxsUnderlying)
			strncpy(UnderlyingStr, coreMsg->coreHeader.pnxOptionHdr->pnxsUnderlying->String, 24);
		else
			UnderlyingStr[0] = '\0';

		if (coreMsg->coreHeader.pnxOptionHdr->pnxsSeriesChain)
			strncpy(SeriesStr, coreMsg->coreHeader.pnxOptionHdr->pnxsSeriesChain->String, 254);
		else
			SeriesStr[0] = '\0';

		// If the string starts with 'o' it's in one format, if a space then it's another
		if (coreMsg->coreHeader.pnxOptionHdr->pnxsDateAndStrike->String[1] == ' ') {
			snprintf(&OptionSymbol[0], 24, "%-6s%02d%02d%02d%c%08d",
					coreMsg->coreHeader.pnxStringSymbol->String+1, // skip first character
					coreMsg->coreHeader.pnxOptionHdr->nxExpirationDate.Year-2000,
					coreMsg->coreHeader.pnxOptionHdr->nxExpirationDate.Month,
					coreMsg->coreHeader.pnxOptionHdr->nxExpirationDate.Day,
					(coreMsg->coreHeader.pnxOptionHdr->PutCall == 0) ? 'C' : 'P',
					coreMsg->coreHeader.pnxOptionHdr->strikePrice);
		} else {
			snprintf(&OptionSymbol[0], 24, "%s%c%c",
					coreMsg->coreHeader.pnxStringSymbol->String + 1, // skip first character
					coreMsg->coreHeader.pnxOptionHdr->pnxsDateAndStrike->String[0],
					coreMsg->coreHeader.pnxOptionHdr->pnxsDateAndStrike->String[1]);
		}
		fprintf(fp_opt,"%s,%s,%0.2f,%s,%s\n",
				coreMsg->coreHeader.pnxStringSymbol->String,
				OptionSymbol,
				StrikePrice,
				UnderlyingStr,
				SeriesStr);
	}

	if (pfNxCoreStateGetLastTrade(&lastTrade,RequestString) == 0) {
		uint8_t priceType = lastTrade.PriceType;
		// Is it an equity or an index?
		if ((oei == 'e') | (oei == 'i')) {
			NxCoreTrade* Trade = (NxCoreTrade*) &coreMsg->coreData.Trade;
			uint8_t priceType = Trade->PriceType;

//			if (Trade->Price != lastTrade.Price) fprintf(fp_info , "%s %d Trade->Price %0.2f != lastTrade.Price %.02f\n",
//					coreMsg->coreHeader.pnxStringSymbol->String, coreMsg->coreHeader.nxExgTimestamp.MsOfDay, Trade->Price, lastTrade.Price);
//			if (Trade->NetChange != lastTrade.NetChange) fprintf(fp_info , "%s %d Trade->NetChange %0.2f != lastTrade.NetChange %0.2f\n",
//					coreMsg->coreHeader.pnxStringSymbol->String, coreMsg->coreHeader.nxExgTimestamp.MsOfDay, Trade->NetChange, lastTrade.NetChange);

			char symb[11];
			snprintf(&symb[0], 10, "%s__________", coreMsg->coreHeader.pnxStringSymbol->String);

			fprintf(fp_trade,
					"%s%08d_%08d,%08d,%02d:%02d:%02d,%c,%s,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%d,%d,%d,%d,%lld,%d,%s,%s(%d %d %d %d)\n",
					&symb[1], coreMsg->coreHeader.nxExgTimestamp.MsOfDay, lastTrade.TotalVolume,	// %s+%d+%d		// Key
					coreMsg->coreHeader.nxExgTimestamp.MsOfDay,	// %d
					coreMsg->coreHeader.nxExgTimestamp.Hour,		// }
					coreMsg->coreHeader.nxExgTimestamp.Minute,	// } %02:%02:%02
					coreMsg->coreHeader.nxExgTimestamp.Second,	// }

					symb[0],
					coreMsg->coreHeader.pnxStringSymbol->String+1,	// %s

					pfNxCorePriceToDouble(lastTrade.Open, priceType),		// %0.2f		// Can be zero
					pfNxCorePriceToDouble(lastTrade.High, priceType),		// %0.2f		// Can be zero
					pfNxCorePriceToDouble(lastTrade.Low, priceType),		// %0.2f		// Can be zero
					pfNxCorePriceToDouble(lastTrade.Last, priceType),		// %0.2f		// Can be zero
					pfNxCorePriceToDouble(Trade->Price, priceType),			// %0.2f
					pfNxCorePriceToDouble(Trade->NetChange, priceType),		// %0.2f
					Trade->Size,											// %d
					Trade->PriceFlags,									// %d

					lastTrade.Tick,								// %d
					lastTrade.TickVolume,						// %d
					lastTrade.TotalVolume,						// %lld
					lastTrade.Threshold,			  				// %d

					exchanges[coreMsg->coreHeader.ReportingExg],	// %s
					conditions[Trade->TradeCondition],			// %s
					Trade->ExtTradeConditions[0],				// %d
					Trade->ExtTradeConditions[1],				// %d
					Trade->ExtTradeConditions[2],				// %d
					Trade->ExtTradeConditions[3]);				// %d
		} else {
			fprintf(fp_info,"Should not happen: Found trade for %s - not sure what it is",coreMsg->coreHeader.pnxStringSymbol->String);
		}
	}
}

int callback(const struct NxCoreSystem *coreSys,const struct NxCoreMessage *coreMsg) {
	// Do something based on the message type
	switch (coreMsg->MessageType) {

	case NxMSG_STATUS:
		/**
		 * Status messages are unique in that they do not have a corresponding NxCoreMessage data structure.
		 *
		 * Sent on initialization, whenever the NxCore millisecond clock changes,
		 * if any errors are detected, when a tape completes or restarts,
		 * on shutdown and when paused waiting for more data.
		 * Nanex Clock changes occur at 25 millisecond intervals when trading is active
		 * making this the most frequent reason for receiving a NxMSG_STATUS message.
		 *
		 * Since NxCore API is a callback model, these frequent callbacks give your program
		 * frequent opportunity to perform tasks.
		 */
		OnNxCoreStatus(coreSys,coreMsg);
		break;

	case NxMSG_TRADE:
		/**
		 * NxTrade messages are the most heavily processed and analyzed within the Nanex Financial Servers.
		 * Many members in NxTrade are the result of this processing and are therefore unique to NxCore.
		 * Other members are updated from exchanges but rarely found in other financial feeds.
		 *
		 * Trade messages are sent for last sale reports and corrections to last sale reports.
		 * Less than 10% of the Nanex Financial Feed contains Trade messages, mostly due to the heavy volume of quotes.
		 * When processing a tape, you can set a filter to exclude ExgQuote and MMQuotes,
		 * which will dramatically speed up the processing of trades.
		 * The relationship between the last sale and the most recent quote has been added
		 * to the Trade message members to make it easier to exclude quotes in certain analysis situations.
		 *
		 * There is an abundance of information included with each trade message you won't find,
		 * or will rarely find in other feeds, such as original Exchange Sequence numbers,
		 * trade condition processing flags (is the trade elgible to update last, high, low, open?),
		 * NxCore QuoteMatch (matches each trade to the recent regional and BBO quotes),
		 * NxCore Realtime Trade Filter analysis results, and NxCore Significant High/Low data on each trade message.
		 * A typical trading day will have 10+ million of these message types.
		 */
		OnTradeStates(coreSys,coreMsg);
		break;

	case NxMSG_EXGQUOTE:
		/**
		 * Sent for every exchange quote (regular quote) and BBO (Exchange-determined Best Bid/Offer).
		 * Each quote update includes the bid and ask prices, sizes and condition codes, plus price/size changes.
		 * Also for symbols trading on multiple exchanges, each ExgQuote also contains fields
		 * with the current values of the best bid/best ask prices, sizes and condition codes.
		 * This message type is by far the most active of all messages your callback will receive
		 * (depending of course on the exchanges you subscribe to).
		 * A typical trading day will have 700+ million option quotes
		 */
		// Maybe not so useful?
		// OnGetExgQuoteStates(coreSys,coreMsg);
		break;

	case NxMSG_MMQUOTE:
		/**
		 * Sent for every Market Maker Quote ("level 2") from either Nasdaq SuperMontage,
		 * Intermarket quotes on Nasdaq issues (e.g. Cincinnati -- ISLD, pacific -- ARCA),
		 * or Nasdaq Intermarket quotes on listed (NYSE/AMEX) securities.
		 * A typical trading day will have 40+ million or so of these messages.
		 * MMQuotes contain Market Maker identifiers and quote types in addition to bid and ask prices,
		 * sizes, condition codes and price/size changes.
		 * For depth messages, the market makers will be D1/D2/D3/D4/D5/D6/D7/D8/D9/D10
		 * to signify the 5 or 10 levels of depth
		 */

		// Not seen these yet
		OnNxCoreMMQuote(coreSys,coreMsg);
		break;

	case NxMSG_CATEGORY:
		/**
		 * Sent for all other data types, such as fundamental information, 52 week/contract/YTD high/low data,
		 * additional trade correction information, exchange code translations, trade/quote condition code translation,
		 * trading halts, trading imbalances, open trade indications, etc.
		 * A typical trading day will have 1-2 million or so of these message types,
		 * with the bulk of them occurring at the start of each tape summarizing the previous trading session.
		 */
		break;

	case NxMSG_SYMBOLCHANGE:
		/**
		 * Sent when an issue changes symbols or trading venues (switches exchanges).
		 * The most frequent user of this message type are Nasdaq equities changing between Pink Sheets,
		 * Bulletin Board, Over-The-Counter, or when the adding/removing the fifth letter 'E'
		 * because of a change in delinquency filing status.
		 * Also, when option symbols change, each option contract in the series generates one of these messages.
		 * Usually the only task you need to perform to handle this type of message
		 * is to transfer any UserData1 or UserData2 values from the old symbol to the new symbol.
		 */
		break;

	case NxMSG_SYMBOLSPIN:
		/**
		 * NxCoreSymbolSpin Messages are automatically sent once for each Symbol
		 * that has traded previous to the start of the NxCore Tape.
		 * Symbol Spin messages provide a convenient and efficient method for iterating Symbol Sets
		 * without having to create and maintain a container to hold them.
		 * The system symbol spin is useful for preallocating storage before market open
		 * to minimize allocations and reallocations during active trading.
		 *
		 * You do not need to process symbol spin messages; they are provided for convenience only.
		 */
		break;

	default:
		fprintf(fp_info,"Cannot handle message type %d yet \n",coreMsg->MessageType);

	}

	// Continue running the tape
	return NxCALLBACKRETURN_CONTINUE;
}

int main(int argc,char *argv[]) {
	loadNxCore("./libnx.so");
	if (argc < 2) {
		fprintf(stderr,"usage: %s nxfile [bucket] \n",argv[0]);
		exit(1);
	}
	bucket = (argc == 3) ? argv[2] : getenv("GCLOUD_BUCKET");
	if (bucket == NULL) {
		fprintf(stderr,"setenv GCLOUD_BUCKET or pass the bucket ID as an argument");
		fprintf(stderr,"usage: %s nxfile [bucket] \n",argv[0]);
		exit(1);
	}

	fileName = argv[1];
	char buff[MAXPATHLEN+1];

	snprintf(&buff[0],MAXPATHLEN,"%s_info.csv", fileName);
	fp_info = fopen(&buff[0],"w");
	fname_info = strdup(&buff[0]);

	snprintf(&buff[0],MAXPATHLEN,"%s_opt.csv", fileName);
	fp_opt = fopen(&buff[0],"w");
	fname_opt = strdup(&buff[0]);

	snprintf(&buff[0],MAXPATHLEN,"%s_trade.csv", fileName);
	fp_trade = fopen(&buff[0],"w");
	fname_trade = strdup(&buff[0]);


//	if (fp_trade) fprintf(fp_trade,
//			"Time(ms), Time, Trade for Symbol ,Open, High, Low, Last, Price, Net Change, Tick, Tick Volume, Total Volume, Trade Price, Trade Net Change, Trade Size, Reporting Exchange, Trade Conditions\n");
//
//	if (fp_exg) fprintf(fp_exg,
//			"EXG Quote for Symbol,Time,Bid,Ask,Bid Size,Ask Size,Best Bid Exchange,Best Ask Exchange,Condition,Exchange\n");
//
//	if (fp_opt) fprintf(fp_opt,"Symbol Spin for: NxSym,OSI Sym,Strike,Underlying,Series Chain\n");
//
//	if (fp_mm) fprintf(fp_mm,
//			"MM Quote for Symbol,Time,Bid,Ask,Bid Size,Ask Size,Market Maker,Exchange\n");

	// (NxCF_EXCLUDE_QUOTES | NxCF_EXCLUDE_QUOTES2 | NxCF_EXCLUDE_OPRA)
	pfNxCoreProcessTape(fileName, NULL, 0, 0, callback);
}
