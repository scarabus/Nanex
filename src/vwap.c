#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>

#include "./NxCoreAPI_Wrapper_C.h"
#include "./NxCoreAPI.h"

char *nxfile;
char *bucket;
char *dataset;
char *date;
char *fileName;
FILE *fp;
char buff[MAXPATHLEN+1];

struct vwap {
	char prefix[12];
	struct vwap *next[27];
	long double sum_pv;
	uint64_t volume;
	uint32_t trades;
};

struct vwap trie[27];

struct vwap *find(char *symbol, int level, struct vwap *t) {
	char s = symbol[level];
	if (s == '\0') {
		return t;
	} else {
		int i = toupper(s) - 'A';
		if ((i < 0) || (i > 26)) i = 26;
		if (t == NULL) t = &trie[0];
		struct vwap *v = t->next[i];
		if (v == NULL) {
			struct vwap *vv = (struct vwap *) calloc(1, sizeof(struct vwap));
			t->next[i] = vv;
			strncpy(&vv->prefix[0], symbol, level+1);
		}
		return find(symbol, level+1, t->next[i]);
	}
}

void each(struct vwap *v) {
	if (v) {
		if (v->volume != 0) {
			long double vwap = v->sum_pv/v->volume;
			long double pq = v->sum_pv;
			uint64_t volume = v->volume;
			uint32_t trades = v->trades;
//			fprintf(fp, "%s|%0.5Lf|%0.2Lf|%llu|%d %s", &v->prefix[0], (vwap + 0.0000001), (pq + 0.0000001), volume, trades, "\n");
			fprintf(fp, "%s|%0.5Lf\n", &v->prefix[0], (vwap + 0.0000001), (pq + 0.0000001), volume, trades, "\n");
		}
		for (int i = 0; i < 27; i++) {
			each(v->next[i]);
		}
	}
}

int callback(const struct NxCoreSystem *coreSys,const struct NxCoreMessage *coreMsg) {
	// Do something based on the message type
	if ((coreMsg != NULL) && (coreMsg->MessageType == NxMSG_TRADE)) {
		NxCoreTrade* Trade = (NxCoreTrade*) &coreMsg->coreData.Trade;
		char *symbol = coreMsg->coreHeader.pnxStringSymbol->String+1;
		double price = pfNxCorePriceToDouble(Trade->Price, Trade->PriceType);
		uint64_t volume = Trade->Size;
		struct vwap *vw = find(symbol, 0, NULL);
		vw->sum_pv += (price * volume);
		vw->volume += volume;
		vw->trades++;
	}

	 if (coreSys->Status == NxCORESTATUS_COMPLETE) {
		each(&trie[0]);
		fflush(fp);
		fclose(fp);

		// Upload data to Google Storage
		snprintf(&buff[0], MAXPATHLEN, "gs://%s/nanex/%s", bucket, fileName);
		char *target = strdup(&buff[0]);

		int pid = fork();
		if (pid < 0) {
			perror("Cannot fork");
			exit(1);
		} else if (pid == 0) {
			fprintf(stderr, "Copy %s to %s \n", fileName, target);
			execl("/usr/bin/gsutil", "gsutil", "cp", fileName, target, NULL);
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

// bq load  --skip_leading_rows 1 --field_delimiter '|' DATASETdotTABLE 20180202.GS.nx2_vwap.csv symbol:string,vwap:float

			snprintf(&buff[0], MAXPATHLEN, "%s.%s", dataset, date);
			fprintf(stderr, "/usr/bin/bq load --skip_leading_rows=1 --field_delimiter='|' %s %s %s \n", &buff[0], fileName, "symbol:string,vwap:float");
			execl("/usr/bin/bq","bq", "load", "--skip_leading_rows=1", "--field_delimiter=|", &buff[0], fileName, "symbol:string,vwap:float", NULL);
			exit(0);
		}

		waitpid(pid, &status, 0);
		fprintf(stderr, "Child process %d returns %d\n", pid, status);
	}

	// Continue running the tape
	return NxCALLBACKRETURN_CONTINUE;
}

int main(int argc,char *argv[]) {
	loadNxCore("./libnx.so");
	if (argc < 2) {
		fprintf(stderr,"usage: %s nxfile \n",argv[0]);
		exit(1);
	}
	date = strdup(argv[1]);
	char *c = strchr(date, '.');
	if (c) *c = '\0';

	nxfile = argv[1];

	bucket = (argc > 2) ? argv[2] : getenv("GCLOUD_BUCKET");
	dataset = (argc > 3) ? argv[3] : getenv("GCLOUD_DATASET");
	if ((dataset == NULL) || (bucket == NULL)) {

		fprintf(stderr,"setenv GCLOUD_BUCKET and GCLOUD_DATASET or pass the bucket and dataset IDs as arguments");
		fprintf(stderr,"usage: %s nxfile bucket dataset\n",argv[0]);
		exit(1);
	}

	memset(&trie[0], 0, sizeof(struct vwap));

	snprintf(&buff[0],MAXPATHLEN,"%s_vwap.csv", nxfile);
	fp = fopen(&buff[0],"w");
	fileName = strdup(&buff[0]);

//	fprintf(fp, "#Ticker|VWAP0|TotalTradeValue|TotalQuantity|TotalTradeCount\n");
	fprintf(fp, "#Ticker|VWAP0\n");

	pfNxCoreProcessTape(nxfile, NULL, 0, 0, callback);
}
