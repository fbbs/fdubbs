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

    //计算title的hash謎