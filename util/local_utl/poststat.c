/***************************************************************************
                          poststat.c  -  description
                             -------------------
    begin                : 四  3月 10 2005
    copyright            : (C) 2005 by SpiritRain
    email                : SpiritRain@citiz.net
	last modified        : 二  4月 26 2005
 ***************************************************************************/

/*
*   参数: poststat [dir] [type]
*   type: 
*   0 表示统计日十大
*   1 表示统计周五十大
*   2 表示统计月一百大
*   3 表示统计年一百五十大
*/

//#define DEBUG			1

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define HASHSIZE		2048
#define AUTHORSIZE		13
#define BOARDSIZE		18
#define TITLESIZE		62

char *myfile[4] = {"day", "week", "month", "year"};
int mytop[4] = {10, 50, 100, 200};
char *mytitle[4] = {"日十", "周五十", "月一百", "年度二百"};


time_t now;

/* 105 bytes */
struct postrec {
    char author[AUTHORSIZE];              /* author name */
    char board[BOARDSIZE];               /* board name */
    char title[TITLESIZE];               /* title name */
    int gid;
    time_t date;                  /* last post's date */
    int number;                   /* post number */
};

struct postlist {
    char author[AUTHORSIZE];              /* author name */
    char board[BOARDSIZE];               /* board name */
    char title[TITLESIZE];               /* title name */
    int gid;
    time_t date;                  /* last post's date */
    int number;                   /* post number */
	struct postlist  *bnext, *prev, *next;
}*bucket[HASHSIZE],*toppost;


int	hash(char *key) {
    int i, value = 0;

    for (i = 0; key[i] && i < TITLESIZE; i++)
        value += key[i] < 0 ? -key[i] : key[i];

    value = value % HASHSIZE;
    return value;
}


/* ---------------------------------- */
/* hash structure : array + link list */
/* ---------------------------------- */
static __inline__ void add_rec(struct postlist * rec, struct postlist * prev,struct postlist * next)
{
	next->prev = rec;
	rec->next = next;
	rec->prev = prev;
	prev->next = rec;
}
static __inline__ void del_rec(struct postlist * prev,
				  struct postlist * next)
{
	next->prev = prev;
	prev->next = next;
}


void swap(struct postlist *i, struct postlist * j){
	del_rec(i->prev, i->next);
	del_rec(j->prev, j->next);
	add_rec(i, j->prev, j->next);
	add_rec(j, i->prev, i->next);
}

void sort(struct postlist *rec)
{
	struct postlist *tmppost = rec->prev;

	while (rec->number > tmppost->number && tmppost != toppost)
	{
		tmppost = tmppost->prev;
	}
	del_rec(rec->prev, rec->next);
	add_rec(rec, tmppost, tmppost->next);
}
/*
//链表插入什么方法比较高效啊？头晕了-_-b
//1.0版先只实现一个顺序搜索
//效率太差，不要了
void sort_all(){
	struct postlist *prev = toppost->next, *post = prev->next, *next;
	int count = 0;
	while (post != toppost)
	{
		count++;
		next = post->next;
		prev = post->prev;
		while (prev->number < post->number && prev != toppost)
		{
			prev = prev->prev;
		}
		if (prev != post->prev)
		{
			del_rec(post->prev, post->next);
			add_rec(post, prev, prev->next);
		}
		post = next;
	}
}
*/


void search(struct postlist *rec) {
    struct postlist *tmppost;
	//struct postlist *s;
    int 	h;
	//int		found = 0;

    //计算title的hash
	//h = hash(rec->title);
	h = rec->gid % HASHSIZE;
	//如果bucket为空
	rec->bnext = NULL;
	if (bucket[h] == NULL)
	{
		//bucket的第一项为record
		bucket[h] = rec;
		//record的下一个为空
		//rec加入toppost的双连表中
		add_rec(rec,toppost->prev, toppost );
		sort(rec);
	}else{
		//临时变量为bucket的第一个
	    tmppost = bucket[h];
		//如果tmppost非空，并且tmppost和rec的版名不同或title不同
		while (1) 
		{
			//找到了
			if (tmppost->gid == rec->gid && !strcmp(tmppost->board, rec->board))
			{
				//增加post number
				tmppost->number += rec->number;
				//取较晚的date
				if (tmppost->date < rec->date){
					tmppost->date = rec->date;
				}
				else{
					tmppost->date = rec->date;
				}
				sort(tmppost);
				return;
			}
			//没找到
			if (tmppost->bnext == NULL)
			{
				//搜索完列表，rec加入列表底部
				tmppost->bnext = rec;
				add_rec(rec,toppost->prev, toppost );
				sort(rec);
				return;
			}
			else
			{
				//搜索下一条记录
				tmppost = tmppost->bnext;
			}
		}
	}
}

//从fname中读取记录，并为每个记录在list中找到适当的位置
void load_stat(char *fname, size_t size) {
    FILE *fp;
	struct postlist *post;
	int listsize = sizeof(struct postlist); 
	long count = 0;
    //打开fname
	if( (fp = fopen(fname, "r")) != NULL) {
		//分配一个内存块
		post = (struct postlist *)malloc(listsize);
		post->prev = post;
		post->next = post;
		post->bnext = NULL;
		//从文件中读取记录
		while (fread(post, sizeof(struct postrec), 1, fp) && count < size)
		{
			//搜索列表，将记录加入
			search(post);
			//重新分配一个内存块
			count++;
			post = (struct postlist *)malloc(listsize);
			post->prev = post;
			post->next = post;
			post->bnext = NULL;
		};
		//释放多余的内存块
		free(post);
		//关闭fp
        fclose(fp);
    }
}


void poststat(int mytype) {
	FILE *fp;
	char srcfile[64], dstfile[64], *p;
    struct postlist *tmppost;
    int i, top, count;


	//十大统计篇数
	top = mytop[mytype];
	//初始化toppost数据结构
	//toppost是一个空的list
	toppost = malloc(sizeof(struct postlist));
	toppost->prev = toppost;
	toppost->next = toppost;
	toppost->bnext = NULL;
	toppost->number = 0;


	//统计日十大
	if (mytype == 0)
	{
#ifdef DEBUG
		sprintf(srcfile,  "tmp/post");
		sprintf(dstfile,  "tmp/post.old");
#else
		sprintf(srcfile,  "tmp/.post");
		sprintf(dstfile,  "tmp/.post.old");
#endif
		//移除 tmp/.post.old
		remove(dstfile);
		
		//将 tmp/.post 重命名为 tmp/.post.old
		rename(srcfile, dstfile);
		
		//先读取etc/posts/day.0
		sprintf(srcfile, "etc/posts/day.0");
		load_stat(srcfile, -1);
		
		//再读取 tmp/.post.old
		load_stat(dstfile, -1);
	}
	//统计周五十大
	else if (mytype > 0 && mytype < 4)
	{
		//先读取本周top记录etc/posts/week.0
		sprintf(srcfile,  "etc/posts/%s.0", myfile[mytype]);
		load_stat(srcfile, -1);

		//再读取本日记录etc/posts/day.0
		sprintf(dstfile, "etc/posts/day.0");
		load_stat(dstfile, -1);
	}
	//统计月一百大
	else
	{
		return;
	}

	//把十大写回
	if( (fp = fopen(srcfile, "w"))!=NULL) {
		count = 0;
		tmppost = toppost->next;
		while (tmppost != toppost)
		{
			fwrite(tmppost, sizeof(struct postrec), 1, fp);
			tmppost = tmppost->next;
		}
		fclose(fp);
	}

    sprintf(srcfile, "etc/posts/%s", myfile[mytype]);
    if( (fp = fopen(srcfile, "w"))!=NULL ) {
        fprintf(fp, "                [1;34m-----[37m=====[41m 本%s大热门话题 [40m=====[34m-----[0m\n\n", mytitle[mytype]);

		tmppost = toppost->next;
        for (i = 0; i < top && tmppost != toppost; i++) {
            strcpy(dstfile, ctime(&tmppost->date));
            dstfile[20] = '\0';
            p = dstfile + 4;
            fprintf(fp,
                    "[1;37m第[1;31m%3d[37m 名 [37m信区 : [33m%-18s[37m"
                    "〖 [32m%s[37m〗[36m%4d [37m篇[35m%13.13s\n"
                    "     [37m标题 : [1;44;37m%-60.60s[40m\n"
                    ,(int)(i+1),tmppost->board,p,tmppost->number, tmppost->author, tmppost->title);
    		tmppost = tmppost->next;
		}
        fclose(fp);
    }

    /* free statistics */
	do
	{
		tmppost = toppost->next;
		del_rec(toppost,tmppost->next);
		free(tmppost);
	}
	while  (tmppost != toppost);
	for (i = 0; i < HASHSIZE ; i++ )
	{
		bucket[i] = NULL;
	}
}

//删除原先的 etc/posts/????.0
//将 etc/posts/day.0 重命名为 etc/posts/day.x
//如此则开始重新计算十大
void reset(int type)
{
	char srcfile[64];
	sprintf(srcfile, "etc/posts/%s.0",myfile[type]);
	remove(srcfile);
}


int	main(int argc,char ** argv) {
#ifdef DEBUG
	int i, j;
	char buf[64];
	time_t t;
#endif
    struct tm *today;

#ifndef DEBUG
	if (argc < 2) {
		printf("Usage:\t%s bbshome [day]\n", argv[0]);
		return (-1);
	}
#endif
	chdir(argv[1]);
	if (argc == 3) {
		poststat(atoi(argv[2]));
		return (0);
	}
	time(&now);
    today = localtime(&now);
#ifdef DEBUG
	printf("mon:%x\tday:%d\thour:%d\tmin:%d\n", today->tm_mon , today->tm_mday , today->tm_hour,today->tm_min);
#endif
/*
	for (i = 0 ; i < 24 ; i++ )
	{
		for (j = 0; j < 60 ; j+=15 )
		{
			sprintf(buf, "tmp/.post.71%d%d", i, j);
			rename(buf,"tmp/post");
			poststat(0);
	printf("mon:%x\tday:%d\thour:%d\tmin:%d\n", today->tm_mon , today->tm_mday , today->tm_hour,today->tm_min);
		}
	}
	*/
#ifdef DEBUG
	printf("mon:%x\tday:%d\thour:%d\tmin:%d\n", today->tm_mon , today->tm_mday , today->tm_hour,today->tm_min);
#endif


// 如果hour是0点,则在0点的前10分钟调用到main
    //
	//生成当日十大
    poststat(0);
    today = localtime(&now);
//    today = localtime(&now);
	//凌晨0点的时候
	if (today->tm_hour == 0 && today->tm_min <10) {
		//生成本周五十大
		poststat(1);
		//生成本月一百大
		poststat(2);
		//生成本年一百五十大
		poststat(3);

		today = localtime(&now);
#ifdef DEBUG
		printf("mon:%x\tday:%d\thour:%d\tmin:%d\n", today->tm_mon , today->tm_mday , today->tm_hour,today->tm_min);
#endif
		//删除旧的day.0日志文件，从新开始统计每日十大
		reset(0);

        //如果week day是周日,删除week.0日志文件，从新开始统计每周五十大
       if (today->tm_wday == 0){
			reset(1);
		}

        //如果month day是1号,删除month.0日志文件，从新开始统计每月一百大
        if (today->tm_mday == 1){
            reset(2);

		}
		
		//如果是1月1号，删除year.0日志文件，从新开始统计每年一百五十大
	//	today = localtime(&now);
		if (today->tm_yday == 0)
		{
			reset(3);
		}
    }
#ifdef DEBUG
   time(&t);
   printf("time: %d\n", t - now);
#endif
   return 0;
	
}
