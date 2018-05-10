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

FILE *fp_info = NULL;
char *bucket;
char *fileName;

void OnNxCoreStatus(const NxCoreSystem* coreSys,const NxCoreMessage* coreMsg) {
	switch (coreSys->Status) {
	case NxCORESTATUS_COMPLETE:
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
		fwrite(coreSys, sizeof(struct NxCoreSystem), 1, fp_info);
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
		fwrite(coreSys, sizeof(struct NxCoreSystem), 1, fp_info);
		fwrite(coreMsg, sizeof(struct NxCoreMessage), 1, fp_info);
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
		fwrite(coreSys, sizeof(struct NxCoreSystem), 1, fp_info);
		fwrite(coreMsg, sizeof(struct NxCoreMessage), 1, fp_info);
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

		fwrite(coreSys, sizeof(struct NxCoreSystem), 1, fp_info);
		fwrite(coreMsg, sizeof(struct NxCoreMessage), 1, fp_info);
		break;

	case NxMSG_CATEGORY:
		/**
		 * Sent for all other data types, such as fundamental information, 52 week/contract/YTD high/low data,
		 * additional trade correction information, exchange code translations, trade/quote condition code translation,
		 * trading halts, trading imbalances, open trade indications, etc.
		 * A typical trading day will have 1-2 million or so of these message types,
		 * with the bulk of them occurring at the start of each tape summarizing the previous trading session.
		 */
		fwrite(coreSys, sizeof(struct NxCoreSystem), 1, fp_info);
		fwrite(coreMsg, sizeof(struct NxCoreMessage), 1, fp_info);
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
		fwrite(coreSys, sizeof(struct NxCoreSystem), 1, fp_info);
		fwrite(coreMsg, sizeof(struct NxCoreMessage), 1, fp_info);
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
		fwrite(coreSys, sizeof(struct NxCoreSystem), 1, fp_info);
		fwrite(coreMsg, sizeof(struct NxCoreMessage), 1, fp_info);
		break;

	default:
		fwrite(coreSys, sizeof(struct NxCoreSystem), 1, fp_info);
		fwrite(coreMsg, sizeof(struct NxCoreMessage), 1, fp_info);
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

	snprintf(&buff[0],MAXPATHLEN,"%s_info", fileName);
	fp_info = fopen(&buff[0],"w");

	pfNxCoreProcessTape(fileName, NULL, 0, 0, callback);

	fclose(fp_info);
}
