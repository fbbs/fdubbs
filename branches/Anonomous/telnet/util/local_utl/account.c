/* account.c  -- count for no. of logins */
/* $Id: account.c 2 2005-07-14 15:06:08Z root $ */

#include <time.h>
#include <stdio.h>
#include "bbs.h"
#define MAX_LINE        (15)
char 	datestring[30];

struct {
	int     no[24];		/* 次数 */
	int     sum[24];	/* 总合 */
}       st;

/* Added by deardragon 1999.12.2 */
void getdatestring( time_t now)
{
        //char weeknum[7][3]={"天","一","二","三","四","五","六"};

		snprintf(datestring,30,"%s",ctime(&now));
		datestring[19] =0;
		sprintf(datestring,"%s",datestring+4);
	    /*
        sprintf(datestring,"%4d年%02d月%02d日%02d:%02d:%02d 星期%2s",
                tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,
                tm->tm_hour,tm->tm_min,tm->tm_sec,
                weeknum[tm->tm_wday]);
		*/
}
/* Added End. */

int
main(argc, argv)
char   *argv[];
{
	FILE   *fp;
	char    buf[256], *p;
	char    date[80];
	int     now;
	int     hour, max = 0, item, total = 0;
	int     totaltime = 0;
	int     i, j;
	char   *blk[10] =
	{
                "  ", "  ", "▁", "▂", "▃",
                "▄", "▅", "▆", "▇", "█",
	};
	
	//sprintf(buf,"%s/usies", BBSHOME);
	strcpy(buf, "/home/bbs/usies");
	if ((fp = fopen(buf, "r")) == NULL) {
		printf("can't open usies\n");
		return 1;
	}
	now = time(0);
	getdatestring(now);
	sprintf(date, "%s", datestring);
	while (fgets(buf, 256, fp)) {
		hour = atoi(buf + 7);	//时数
		if (hour < 0 || hour > 23) {
			printf("%s", buf);
			continue;
		}
		if (strncmp(buf, date, 6))
			continue;
		if (!strncmp(buf + 20, "bbsd: ENTER", 11)
			||!strncmp(buf+20, "bbslogin: ",10)
		) {
			st.no[hour]++;
			continue;
		}
		if (p = (char *) strstr(buf + 45, "Stay:")) {
			st.sum[hour] += atoi(p + 6);
			continue;
		}
	}
	fclose(fp);

	for (i = 0; i < 24; i++) {
		total += st.no[i];
		totaltime += st.sum[i];
		if (st.no[i] > max)
			max = st.no[i];
	}

	if ( max != 0 )
		item = max / MAX_LINE + 1;
	else {
/*		printf("max is 0.\n"); */
		exit(0);
	}

	sprintf(buf,"%s/0Announce/bbslist/countusr", BBSHOME);
	if ((fp = fopen(buf, "w")) == NULL) {
		printf("Cann't open countusr\n");
		return 1;
	}
	//fprintf(fp, "\n[1;36m    ┌────────────────────────────────────┐\n");
//	fprintf(fp, "\n[1;36m    ┌─────────────────────────┐\n");
	fprintf(fp, "\n[1;36m    ┌──────────超过一千将不显示个位数───────────────┐\n");
	
	for (i = MAX_LINE; i >= 0; i--) {
		fprintf(fp, "[1;37m%4.d[36m│[33m", (i + 1) * item);
		for (j = 0; j < 24; j++) {
			if ((item * (i) > st.no[j]) && (item * (i - 1) <= st.no[j]) && st.no[j]) {
				//fprintf(fp, "[35m%-3d[33m", (st.no[j]));
				/*
				if (st.no[j]<1000)
					fprintf(fp, "[35m%-3d[33m", (st.no[j]));
				else
					fprintf(fp, "[35m%-3d[33m", (st.no[j])/10);
				*/
				fprintf(fp, "   ");
				continue;
			}
			if (st.no[j] - item * i < item && item * i < st.no[j])
				fprintf(fp, "%s ", blk[((st.no[j] - item * i) * 10) / item]);
			else if (st.no[j] - item * i >= item)
				fprintf(fp, "%s ", blk[9]);
			else
				fprintf(fp, "   ");
		}
		fprintf(fp, "[1;36m│\n");
	}
	
	fprintf(fp,"   [37m0[36m└── [37m%s 每小时到访人次统计[36m ───[37m%20s[36m ──────┘\n"
		"    [;36m  00 01 02 03 04 05 06 07 08 09 10 11 [1;31m12 13 14 15 16 17 18 19 20 21 22 23\n\n"
		"                 [32m1 [33m▇ [32m= [37m%-5d [32m总共上站人次：[37m%-9d[32m平均使用时间：[37m%d[m\n"
		,BBSNAME, datestring, item, total, ( totaltime == 0 ) ? 0 : totaltime / total + 1);
	fclose(fp);
}
