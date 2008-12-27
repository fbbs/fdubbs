/*
 Pirate Bulletin Board System
 Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
 Eagles Bulletin Board System
 Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
 Guy Vega, gtvega@seabass.st.usm.edu
 Dominic Tynes, dbtynes@seabass.st.usm.edu
 Firebird Bulletin Board System
 Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
 Peng Piaw Foong, ppfoong@csie.ncu.edu.tw

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 1, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 */
/*
 $Id: boards.c 374 2007-06-06 18:27:02Z danielfree $
 */

#include "bbs.h"

#define BBS_PAGESIZE    (t_lines - 4)

#define BRC_MAXSIZE     50000
#define BRC_MAXNUM      60
#define BRC_STRLEN      20
//modified by roly form 15 to 20 02.03.25
#define BRC_ITEMSIZE    (BRC_STRLEN + 1 + BRC_MAXNUM * sizeof( int ))
//#define GOOD_BRC_NUM	40	//modfied by money 2003.10.17 //移到bbs.h里 cometcaptor 2007-04-23

char brc_buf[BRC_MAXSIZE];
int brc_size, brc_changed = 0;
char brc_name[BRC_STRLEN];
int brc_list[BRC_MAXNUM], brc_num;

char *sysconf_str();
extern time_t login_start_time;
extern int numboards;
extern int getbnum();
extern struct boardheader *bcache;
extern struct BCACHE *brdshm;
extern struct boardheader *getbcache();
extern struct bstat *getbstat();
struct newpostdata {
	char *name, *title, *BM;
	unsigned int flag;
	int pos, total;
	char unread, zap;
	char status;
}*nbrd;

struct goodboard {
	//char            ID[GOOD_BRC_NUM][BRC_STRLEN];
	//changed by cometcaptor 2007-04-17 为增加自定义目录功能
	struct goodbrdheader boards[GOOD_BRC_NUM];
	int nowpid;
	int num;
} GoodBrd;

int *zapbuf = NULL;
int brdnum, yank_flag = 0;
int boardparent;
char *boardprefix;
int choosemode;

void EGroup (cmd)
char *cmd;
{
	char buf[STRLEN];
	sprintf (buf, "EGROUP%c", *cmd);
	GoodBrd.num = 0;
	GoodBrd.nowpid = -1;
	choosemode = 1;
	boardprefix = sysconf_str(buf);
	choose_board (DEFINE (DEF_NEWPOST) ? 1 : 0);
}

void BoardGroup() {
	GoodBrd.num = 0;
	boardparent = 0;
	choosemode = 0;
	boardprefix = NULL;
	GoodBrd.nowpid = -1;
	choose_board(DEFINE(DEF_NEWPOST) ? 1 : 0);
}

void Boards() {
	boardprefix = NULL;
	boardparent = -1;
	GoodBrd.num = 0;
	GoodBrd.nowpid = -1;
	choose_board(0);
}

void GoodBrds() {
	if (!strcmp(currentuser.userid, "guest"))
		return;
	//GoodBrd.num = 9999;
	boardprefix = NULL;
	boardparent = -2;
	choosemode = 0;
	GoodBrd.nowpid = 0; //added by cometcaptor 2007-04-21
	choose_board(1);
	GoodBrd.nowpid = -1;
	GoodBrd.num = 0;
}

void New() {
	if (heavyload()) {
		clear();
		prints("抱歉，目前系统负荷过重，请改用 Boards 指令阅览讨论区...");
		pressanykey();
		return;
	}
	boardprefix = NULL;
	boardparent = -1;
	GoodBrd.num = 0;
	GoodBrd.nowpid = -1;
	choose_board(1);
}

//int inGoodBrds (char *bname)
int inGoodBrds(int pos) //modified by cometcaptor 2007-04-21 简化查找流程
{
	int i;
	for (i = 0; i < GoodBrd.num && i < GOOD_BRC_NUM; i++)
		//if (!strcmp (bname, GoodBrd.ID[i]))
		if ((GoodBrd.boards[i].pid == GoodBrd.nowpid)
				&&(!(GoodBrd.boards[i].flag & BOARD_CUSTOM_FLAG))&&(pos
				== GoodBrd.boards[i].pos)) //modified by cometcaptor
			return i + 1;
	return 0;
}

void load_zapbuf() {
	char fname[STRLEN];
	int fd, size, n;

	size = MAXBOARD * sizeof(int);
	zapbuf = (int *) malloc(size);
	for (n = 0; n < MAXBOARD; n++)
		zapbuf[n] = 1;
	setuserfile(fname, ".lastread");
	if ((fd = open(fname, O_RDONLY, 0600)) != -1) {
		size = numboards * sizeof(int);
		read(fd, zapbuf, size);
		close(fd);
	}
}

void load_GoodBrd() {
	int i;
	char fname[STRLEN];
	FILE *fp;

	GoodBrd.num = 0;
	setuserfile(fname, ".goodbrd");
	//modified by cometcaptor 2007-04-23 收藏夹自定义目录
	if ((fp = fopen(fname, "rb"))) {
		while (fread(&GoodBrd.boards[GoodBrd.num],
				sizeof(struct goodbrdheader), 1, fp)) {
			if (GoodBrd.boards[GoodBrd.num].flag & BOARD_CUSTOM_FLAG)
				GoodBrd.num++;
			else {
				i = GoodBrd.boards[GoodBrd.num].pos;
				if ((bcache[i].filename[0]) && (bcache[i].flag
						& BOARD_POST_FLAG //p限制版面
						|| HAS_PERM(bcache[i].level) //权限足够
				||(bcache[i].flag & BOARD_NOZAP_FLAG))) //不可zap
					GoodBrd.num++;
			}
			if (GoodBrd.num == GOOD_BRC_NUM)
				break;
		}
		fclose(fp);
	}

	if (GoodBrd.num == 0) {
		GoodBrd.num = 1;
		i = getbnum(DEFAULTBOARD);
		if (i == 0)
			i = getbnum(currboard);
		GoodBrd.boards[0].id = 1;
		GoodBrd.boards[0].pid = 0;
		GoodBrd.boards[0].pos = i-1;
		GoodBrd.boards[0].flag = bcache[i-1].flag;
		strcpy(GoodBrd.boards[0].filename, bcache[i-1].filename);
		strcpy(GoodBrd.boards[0].title, bcache[i-1].title);
	}
	/*
	 if ((fp = fopen (fname, "r"))) {

	 for ( i = 0; i< GOOD_BRC_NUM ; i++)
	 if(fscanf(fp, "%s\n", GoodBrd.ID[i]) != EOF){
	 if( getbnum(GoodBrd.ID[i]) )GoodBrd.num ++;
	 } else break;
	 //modified by roly 02.05.30

	 for (i = 0; i < GOOD_BRC_NUM; i++)
	 if (fscanf (fp, "%s\n", fname) != EOF) {
	 if (getbnum (fname))
	 strcpy (GoodBrd.ID[GoodBrd.num++], fname);
	 } else
	 break;

	 fclose (fp);
	 }
	 if (GoodBrd.num == 0) {
	 GoodBrd.num++;
	 if (getbcache (DEFAULTBOARD) != NULL)
	 strcpy (GoodBrd.ID[0], DEFAULTBOARD);
	 else
	 strcpy (GoodBrd.ID[0], currboard);
	 }
	 */
}

void save_GoodBrd() {
	int i;
	FILE *fp;
	char fname[STRLEN];

	//modified by cometcaptor 2007-04-21
	if (GoodBrd.num == 0) {
		GoodBrd.num = 1;
		i = getbnum(DEFAULTBOARD);
		if (i == 0)
			i = getbnum(currboard);
		GoodBrd.boards[0].id = 1;
		GoodBrd.boards[0].pid = 0;
		GoodBrd.boards[0].pos = i-1;
		GoodBrd.boards[0].flag = bcache[i-1].flag;
		strcpy(GoodBrd.boards[0].filename, bcache[i-1].filename);
		strcpy(GoodBrd.boards[0].title, bcache[i-1].title);
	}
	setuserfile(fname, ".goodbrd");
	if ((fp = fopen(fname, "wb")) != NULL) {
		for (i = 0; i < GoodBrd.num; i++) {
			fwrite(&GoodBrd.boards[i], sizeof(struct goodbrdheader), 1, fp);
			report(GoodBrd.boards[i].filename);
		}
		fclose(fp);
	}

	/*
	 if (GoodBrd.num <= 0) {
	 GoodBrd.num = 1;
	 if (getbcache (DEFAULTBOARD) != NULL)
	 strcpy (GoodBrd.ID[0], DEFAULTBOARD);
	 else
	 strcpy (GoodBrd.ID[0], currboard);
	 }
	 setuserfile (fname, ".goodbrd");
	 if ((fp = fopen (fname, "wb+")) != NULL) {
	 for (i = 0; i < GoodBrd.num; i++) {
	 fprintf (fp, "%s\n", GoodBrd.ID[i]);
	 report (GoodBrd.ID[i]);
	 }
	 fclose (fp);
	 }
	 */
}

void add_GoodBrd(char *bname, int pid) //cometcaptor 2007-04-21
{
	int i = getbnum(bname);
	if ((i > 0)&&(GoodBrd.num < GOOD_BRC_NUM)) {
		i--;
		GoodBrd.boards[GoodBrd.num].pid = pid;
		GoodBrd.boards[GoodBrd.num].pos = i;
		strcpy(GoodBrd.boards[GoodBrd.num].filename, bcache[i].filename);
		strcpy(GoodBrd.boards[GoodBrd.num].title, bcache[i].title);
		GoodBrd.boards[GoodBrd.num].flag = bcache[i].flag;
		//还是不要用那么复杂的循环找空了，没必要
		if (GoodBrd.num)
			GoodBrd.boards[GoodBrd.num].id
					= GoodBrd.boards[GoodBrd.num-1].id + 1;
		else
			GoodBrd.boards[GoodBrd.num].id = 1;
		GoodBrd.num++;
		save_GoodBrd();
	}
}

void mkdir_GoodBrd(char *dirname, char *dirtitle, int pid)//cometcaptor 2007-04-21
{
	//pid暂时是个不使用的参数，因为不打算建二级目录（删除目录的代码目录仍不完善）
	if (GoodBrd.num < GOOD_BRC_NUM) {
		GoodBrd.boards[GoodBrd.num].pid = 0;
		strcpy(GoodBrd.boards[GoodBrd.num].filename, dirname);
		strcpy(GoodBrd.boards[GoodBrd.num].title, "~[收藏] ○ ");
		if (dirtitle[0] != '\0')
			strcpy(GoodBrd.boards[GoodBrd.num].title+11, dirtitle);
		else
			strcpy(GoodBrd.boards[GoodBrd.num].title+11, "自定义目录");
		GoodBrd.boards[GoodBrd.num].flag = BOARD_DIR_FLAG
				| BOARD_CUSTOM_FLAG;
		GoodBrd.boards[GoodBrd.num].pos = -1;
		if (GoodBrd.num)
			GoodBrd.boards[GoodBrd.num].id
					= GoodBrd.boards[GoodBrd.num-1].id + 1;
		else
			GoodBrd.boards[GoodBrd.num].id = 1;
		GoodBrd.num++;
		save_GoodBrd();
	}
}

void rmdir_GoodBrd(int id)//cometcaptor 2007-04-21
{
	//目录没有对二级目录嵌套删除的功能，也因为这个限制，收藏夹目录不允许建立二级目录
	int i, n = 0;
	for (i = 0; i < GoodBrd.num; i++) {
		if (((GoodBrd.boards[i].flag & BOARD_CUSTOM_FLAG)
				&&(GoodBrd.boards[i].id == id)) ||(GoodBrd.boards[i].pid
				== id))
			continue;
		else {
			if (i != n)
				memcpy(&GoodBrd.boards[n], &GoodBrd.boards[i],
						sizeof(struct goodbrdheader));
			n++;
		}
	}
	GoodBrd.num = n;
	save_GoodBrd();
}

void save_zapbuf() {
	char fname[STRLEN];
	int fd, size;
	setuserfile(fname, ".lastread");
	if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) != -1) {
		size = numboards * sizeof(int);
		write(fd, zapbuf, size);
		close(fd);
	}
}

int load_boards() {
	struct boardheader *bptr;
	struct newpostdata *ptr;
	int n, addto = 0, goodbrd = 0;
	// resolve_boards ();
	if (zapbuf == NULL) {
		load_zapbuf();
	}
	brdnum = 0;
	/*
	 if (choosemode == 0 && (boardparent == -2 || GoodBrd.num == 9999)) {
	 load_GoodBrd ();
	 goodbrd = 1;
	 }
	 */
	if (GoodBrd.nowpid >= 0) {
		load_GoodBrd();
		goodbrd = 1;
	}
	for (n = 0; n < numboards; n++) {
		bptr = &bcache[n];
		if (!(bptr->filename[0]))
			continue; /* 隐藏被删除的版面 */
		if (goodbrd == 0) {
			if (!(bptr->flag & BOARD_POST_FLAG) && !HAS_PERM(bptr->level)
					&& !(bptr->flag & BOARD_NOZAP_FLAG))
				continue;
			if ((bptr->flag & BOARD_CLUB_FLAG)&& (bptr->flag
					& BOARD_READ_FLAG )&& !chk_currBM(bptr->BM, 1)
					&& !isclubmember(currentuser.userid, bptr->filename))
				continue;
			if (choosemode == 0) {
				if (boardparent > 0 && boardparent != bptr->group - 1)
					continue;
				if (boardparent == 0 && bptr->group != 0)
					continue;
				if (boardparent > 0 && bptr->title[0] == '*')
					continue;
			} else {
				if (boardprefix != NULL && strchr(boardprefix,
						bptr->title[0]) == NULL && boardprefix[0] != '*')
					continue;
				if (boardprefix != NULL && boardprefix[0] == '*') {
					if (!strstr(bptr->title, "●") && !strstr(bptr->title,
							"⊙") && bptr->title[0] != '*')
						continue;
				}
				if (boardprefix == NULL && bptr->title[0] == '*')
					continue;
			}
			addto = yank_flag || zapbuf[n] != 0 || (bptr->flag
					& BOARD_NOZAP_FLAG);
		} else
			//addto = inGoodBrds (bptr->filename);
			addto = inGoodBrds(n); //modified by cometcaptor 2007-04-17
		if (addto) {
			ptr = &nbrd[brdnum++];
			ptr->name = bptr->filename;
			ptr->title = bptr->title;
			ptr->BM = bptr->BM;
			ptr->flag = bptr->flag;
			ptr->pos = n;
			ptr->total = -1;
			ptr->zap = (zapbuf[n] == 0);
			if (bptr ->flag & BOARD_DIR_FLAG) {
				if (bptr->level != 0)
					ptr->status = 'r';
				else
					ptr->status = ' ';
			} else {
				if (bptr->flag & BOARD_NOZAP_FLAG)
					ptr->status = 'z';
				else if (bptr->flag & BOARD_POST_FLAG)
					ptr->status = 'p';
				else if (bptr->flag & BOARD_NOREPLY_FLAG)
					ptr->status = 'x';
				else if (bptr->level != 0)
					ptr->status = 'r';
				else
					ptr->status = ' ';
			}
		}
	}
	//added by cometcaptor 2007-04-21 读取自定义目录
	if (goodbrd) {
		for (n = 0; n<GoodBrd.num && n<GOOD_BRC_NUM; n++) {
			if ((GoodBrd.boards[n].flag & BOARD_CUSTOM_FLAG)
					&&(GoodBrd.boards[n].pid == GoodBrd.nowpid)) {
				ptr = &nbrd[brdnum++];
				ptr->name = GoodBrd.boards[n].filename;
				ptr->title = GoodBrd.boards[n].title;
				ptr->BM = NULL;
				ptr->flag = GoodBrd.boards[n].flag;
				ptr->pos = GoodBrd.boards[n].id;
				ptr->zap = 0;
				ptr->total = 0;
				ptr->status = ' ';
			}
		}
	}
	//add end
	if (brdnum == 0 && !yank_flag && boardparent == -1) {
		/*
		 if (goodbrd) {
		 GoodBrd.num = 0;
		 save_GoodBrd ();
		 GoodBrd.num = 9999;
		 }
		 *///modified by cometcaptor 2007-04-23
		brdnum = -1;
		yank_flag = 1;
		return -1;
	}
	return 0;
}

int search_board(int *num) {
	static int i = 0, find = YEA;
	static char bname[STRLEN];
	int n, ch, tmpn = NA;
	if (find == YEA) {
		bzero(bname, sizeof (bname));
		find = NA;
		i = 0;
	}
	while (1) {
		move(t_lines - 1, 0);
		clrtoeol();
		prints("请输入要找寻的 board 名称：%s", bname);
		ch = egetch();

		if (isprint2(ch)) {
			bname[i++] = ch;
			for (n = 0; n < brdnum; n++) {
				if (!strncasecmp(nbrd[n].name, bname, i)) {
					tmpn = YEA;
					*num = n;
					if (!strcmp(nbrd[n].name, bname))
						return 1 /* 找到类似的版，画面重画
						 */;
				}
			}
			if (tmpn)
				return 1;
			if (find == NA) {
				bname[--i] = '\0';
			}
			continue;
		} else if (ch == Ctrl('H') || ch == KEY_LEFT || ch == KEY_DEL
				|| ch == '\177') {
			i--;
			if (i < 0) {
				find = YEA;
				break;
			} else {
				bname[i] = '\0';
				continue;
			}
		} else if (ch == '\t') {
			find = YEA;
			break;
		} else if (ch == '\n' || ch == '\r' || ch == KEY_RIGHT) {
			find = YEA;
			break;
		}
		bell(1);
	}
	if (find) {
		move(t_lines - 1, 0);
		clrtoeol();
		return 2 /* 结束了 */;
	}
	return 1;
}

int brc_unread1(int ftime) {
	int n;
	if (brc_num <= 0)
		return 1;
	for (n = 0; n < brc_num; n++) {
		if (ftime > brc_list[n]) {
			return 1;
		} else if (ftime == brc_list[n]) {
			return 0;
		}
	}
	return 0;
}

int check_newpost (ptr)
struct newpostdata *ptr;
{
	ptr->unread = 0;
	ptr->total = (brdshm->bstatus[ptr->pos]).total;
	if (!brc_initial (ptr->name)) {
		ptr->unread = 1;
	} else {
		if (brc_unread1 ((brdshm->bstatus[ptr->pos]).lastpost)) {
			ptr->unread = 1;
		}
	}
	return 1;
}

int unread_position (dirfile, ptr)
char *dirfile;
struct newpostdata *ptr;
{
	struct fileheader fh;
	char filename[STRLEN];
	int fd, offset, step, num;
	num = ptr->total + 1;
	if (ptr->unread && (fd = open (dirfile, O_RDWR))> 0) {
		if (!brc_initial (ptr->name)) {
			num = 1;
		} else {
			offset = (int) ((char *) &(fh.filename[0]) - (char *) &(fh));
			num = ptr->total - 1;
			step = 4;
			while (num> 0) {
				lseek (fd, (off_t) (offset + num * sizeof (fh)), SEEK_SET);
				if (read (fd, filename, STRLEN) <= 0 || !brc_unread (filename))
				break;
				num -= step;
				if (step < 32)
				step += step / 2;
			}
			if (num < 0)
			num = 0;
			while (num < ptr->total) {
				lseek (fd, (off_t) (offset + num * sizeof (fh)), SEEK_SET);
				if (read (fd, filename, STRLEN) <= 0 || brc_unread (filename))
				break;
				num++;
			}
		}
		close (fd);
	}
	if (num < 0)
	num = 0;
	return num;
}

extern void ellipsis(char *, int);

#ifndef NEWONLINECOUNT
int *online_num;
int _cntbrd(struct user_info *ui) {
	if (ui->active && ui->pid) {
		if (ui->currbrdnum > 0 && ui->currbrdnum <= numboards)
			online_num[ui->currbrdnum - 1]++;
	}
}

void countbrdonline() {
	register int i;
	static time_t lasttime = 0;
	static time_t now = 0;
	int semid;

	now = time(0);
	if (now - lasttime < 5)
		return;
	semid = sem(SEM_COUNTONLINE);
	if (0 == p_nowait(semid)) {
		lasttime = now;
		online_num = calloc(numboards, sizeof(int));
		bzero(online_num, sizeof(int) * numboards);
		apply_ulist(_cntbrd);
		resolve_boards();
		for (i = 0; i < numboards; i++) {
			bcache[i].online_num = online_num[i];
		}
		free(online_num);
		v(semid);
	}
}
#endif
/*
 char *num2str(int num)
 {
 static char str[5];
 if(num>9999)
 strcpy(str, "GOD!");
 else if(!num)
 strcpy(str, "    ");
 else 
 sprintf(str, "%4d", num);
 return str;
 }
 */
void show_brdlist (page, clsflag, newflag)
int page, clsflag, newflag;
{
	struct newpostdata *ptr;
	int n;
	char tmpBM[BM_LEN - 1];
	char cate[7];
	char title[80];
	cate[6] = '\0';
	char buf[20];

	if (currentuser.flags[0] & BRDSORT_FLAG) {
		strcpy (buf, "[讨论区列表] [字母]");
	} else if (currentuser.flags[0] & BRDSORT_ONLINE) {
		strcpy (buf, "[讨论区列表] [在线]");
	} else {
		strcpy (buf, "[讨论区列表] [分类]");
	}

	//countbrdonline();
	//resolve_bcache();
	if (clsflag) {
		clear ();
		docmdtitle (buf,
				" [m主选单[[1;32m←[m,[1;32me[m] 阅读[[1;32m→[m,[1;32mRtn[m] 选择[[1;32m↑[m,[1;32m↓[m] 列出[[1;32my[m] 排序[[1;32ms[m] 搜寻[[1;32m/[m] 切换[[1;32mc[m] 求助[[1;32mh[m]\n");
		/*
		 prints("[1;44;37m %s 讨论区名称       V  类别  转 %-25s S 版  主   %s   [m\n",
		 newflag ? "全部  未" : "编号  ", "中  文  叙  述", newflag ? "" : "   "); */
		//Modified by IAMFAT 2002-05-26
		//Modified by IAMFAT 2002-05-29
		//Modified by IAMFAT 2002-06-11
		/*
		 prints("[1;44;37m %s 讨论区名称       V  类别  转 %-25s S 版  主   %s  [m\n",
		 newflag ? "全 部  未" : "编 号  ", "中  文  叙  述", newflag ? "" : "   "); */
		prints
		("[1;44;37m %s 讨论区名称        V  类别  %-20s S 版  主        在线 [m\n",
				newflag ? "全 部  未" : "编 号  未", "中  文  叙  述");
		//              prints("[1;44;37m %s 讨论区名称       V  类别  %-25s S 版  主   %s     [m\n",
		//                      newflag ? "全 部  未" : "编 号  ", "中  文  叙  述", newflag ? "" : "  ");
	}
	move (3, 0);
	for (n = page; n < page + BBS_PAGESIZE; n++) {
		if (n >= brdnum) {
			prints ("\n");
			continue;
		}
		ptr = &nbrd[n];
		if (ptr->total == -1) {
			refresh ();
			check_newpost (ptr);
		}
		//Modified by IAMFAT 2002-05-26
		if (!newflag)
		prints (" %5d", (n + 1));
		else if (ptr->flag & BOARD_DIR_FLAG)
		prints ("  目录");
		else
		prints (" %5d", ptr->total);

		if (ptr->flag & BOARD_DIR_FLAG)
		prints ("  ＋");
		else
		prints ("  %s", ptr->unread ? "◆" : "◇");
		if (!(ptr->flag & BOARD_CUSTOM_FLAG)) //added by cometcaptor 2007-04-23   
		strcpy (tmpBM, ptr->BM);
		strncpy (cate, ptr->title + 1, 6);
		strcpy (title, ptr->title + 11);
		ellipsis (title, 20);
		prints ("%c%-17s %s%s%6s %-20s %c ",
				(ptr->zap && !(ptr->flag & BOARD_NOZAP_FLAG)) ? '*' : ' ',
				ptr->name,
				(ptr->flag & BOARD_VOTE_FLAG) ? "[1;31mV[m" : " ",
				(ptr->flag & BOARD_CLUB_FLAG) ? (ptr->flag & BOARD_READ_FLAG)
				? "[1;31mc[m" : "[1;33mc[m" : " ",
				cate, title, HAS_PERM (PERM_POST) ? ptr->status : ' ');
		if (ptr->flag & BOARD_DIR_FLAG)
		prints ("[目录]\n");
		else
		prints ("%-12s %4d\n",
				ptr->BM[0] <= ' ' ? "诚征版主中" : strtok (tmpBM, " ")
#ifdef NEWONLINECOUNT
				, brdshm->bstatus[ptr->pos].inboard
#else
				, brdshm->bstatus[ptr->pos].online_num
#endif
		);
	}
	refresh ();
}

int cmpboard (brd, tmp)
struct newpostdata *brd, *tmp;
{
	register int type = 0;
	if (currentuser.flags[0] & BRDSORT_FLAG) {
		return strcasecmp (brd->name, tmp->name);
	} else if (currentuser.flags[0] & BRDSORT_ONLINE) {
		return brdshm->bstatus[tmp->pos].inboard - brdshm->bstatus[brd->pos].inboard;
	}

	type = brd->title[0] - tmp->title[0];
	if (type == 0)
	type = strncasecmp (brd->title + 1, tmp->title + 1, 6);
	if (type == 0)
	type = strcasecmp (brd->name, tmp->name);
	return type;
}

int show_board_info(char *board) {
	int i;
	struct boardheader *bp;
	struct bstat *bs;
	char secu[40];
	bp = getbcache(board);
	bs = getbstat(board);
	clear();
	prints("版面详细信息:\n\n");
	prints("number  :     %d\n", getbnum(bp->filename));
	prints("英文名称:     %s\n", bp->filename);
	prints("中文名称:     %s\n", (HAS_PERM(PERM_SPECIAL0)) ? bp->title
			: (bp->title + 11));
	prints("版    主:     %s\n", bp->BM);
	prints("所属讨论区:   %s\n", bp->group ? bcache[bp->group - 1].filename
			: "无");
	prints("是否目录:     %s\n", (bp->flag & BOARD_DIR_FLAG) ? "目录" : "版面");
	prints("可以 ZAP:     %s\n", (bp->flag & BOARD_NOZAP_FLAG) ? "不可以"
			: "可以");

	if (!(bp->flag & BOARD_DIR_FLAG)) {
		prints("在线人数:     %d 人\n", bs->inboard);
		prints("文 章 数:     %s\n", (bp->flag & BOARD_JUNK_FLAG) ? "不计算"
				: "计算");
		prints("可以回复:     %s\n", (bp->flag & BOARD_NOREPLY_FLAG) ? "不可以"
				: "可以");
		//prints ("可以 ZAP:     %s\n",
		//	    (bp->flag & BOARD_NOZAP_FLAG) ? "不可以" : "可以");
		prints("匿 名 版:     %s\n", (bp->flag & BOARD_ANONY_FLAG) ? "是"
				: "否");
#ifdef ENABLE_PREFIX
		prints ("强制前缀:     %s\n",
				(bp->flag & BOARD_PREFIX_FLAG) ? "必须" : "不必");
#endif
		prints("俱 乐 部:     %s\n", (bp->flag & BOARD_CLUB_FLAG) ? (bp-> flag
				& BOARD_READ_FLAG) ? "读限制俱乐部" : "普通俱乐部" : "非俱乐部");
		prints("now id  :     %d\n", bs->nowid);
		prints("读写限制:     %s\n", (bp->flag & BOARD_POST_FLAG) ? "限制发文"
				: (bp->level ==0) ? "没有限制" : "限制阅读");
	}
	if (HAS_PERM(PERM_SPECIAL0) && bp->level != 0) {
		prints("权    限:     ");
		strcpy(secu, "ltmprbBOCAMURS#@XLEast0123456789");
		for (i = 0; i < 32; i++) {
			if (!(bp->level & (1 << i)))
				secu[i] = '-';
			else {
				prints("%s\n              ", permstrings[i]);
			}

		}
		prints("\n权 限 位:     %s\n", secu);
	}

	prints(
			"URL 地址:     http://bbs.fudan.edu.cn/cgi-bin/bbs/bbsdoc?board=%s\n",
			bp->filename);
	pressanykey();
	return FULLUPDATE;

}

int read_board(struct newpostdata *ptr, int newflag) {
	char buf[STRLEN];
	if (ptr->flag & BOARD_DIR_FLAG) {
		int tmpgrp, tmpmode;
		int oldpid; //added by cometcaptor 2007-04-21
		tmpgrp = boardparent;
		tmpmode = choosemode;
		choosemode = 0;
		boardparent = getbnum(ptr->name) - 1;
		oldpid = GoodBrd.nowpid; //cometcaptor 保存GoodBrd.nowpid
		if (ptr->flag & BOARD_CUSTOM_FLAG)
			GoodBrd.nowpid = ptr->pos;
		else
			GoodBrd.nowpid = -1;
		choose_board(newflag);
		GoodBrd.nowpid = oldpid;
		boardparent = tmpgrp;
		choosemode = tmpmode;
		brdnum = -1;
	} else {
		brc_initial(ptr->name);
		memcpy(currBM, ptr->BM, BM_LEN - 1);
		if (DEFINE(DEF_FIRSTNEW)) {
			setbdir(buf, currboard);
			int tmp = unread_position(buf, ptr);
			int page = tmp - t_lines / 2;
			getkeep(buf, page > 1 ? page : 1, tmp + 1);
		}
		Read();
		if (zapbuf[ptr->pos] > 0 && brc_num > 0) {
			zapbuf[ptr->pos] = brc_list[0];
		}
		ptr->total = -1;
		currBM[0] = '\0'; //added by cometcaptor 2007-06-30
	}
}

int choose_board(int newflag) {
	static int num;
	struct newpostdata newpost_buffer[MAXBOARD];
	struct newpostdata *ptr;
	struct newpostdata *oldptr = nbrd; //added by cometcaptor 2006-10-20
	int page, ch, tmp, number, tmpnum;
	int loop_mode = 0;
	char ans[2];
	static char addname[STRLEN-8];
	if (!strcmp(currentuser.userid, "guest"))
		yank_flag = 1;
	nbrd = newpost_buffer;
	modify_user_mode(newflag ? READNEW : READBRD);
	brdnum = number = 0;
	clear();
	//show_brdlist(0, 1, newflag);
	while (1) {
		digestmode = NA;
		if (brdnum <= 0) {
			if (load_boards() == -1)
				continue;
			qsort(nbrd, brdnum, sizeof (nbrd[0]), cmpboard);
			page = -1;
			if (brdnum < 0)
				// prints ("brdnum <=0 ");
				break;
		}
		if (num < 0)
			num = 0;
		if (num >= brdnum)
			num = brdnum - 1;
		if (page < 0) {
			if (newflag) {
				tmp = num;
				while (num < brdnum) {
					ptr = &nbrd[num];
					if (!(ptr->flag & BOARD_DIR_FLAG)) {
						if (ptr->total == -1)
							check_newpost(ptr);
						if (ptr->unread)
							break;
					}
					num++;
				}
				if (num >= brdnum)
					num = tmp;
			}
			page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
			show_brdlist(page, 1, newflag);
			update_endline();
		}
		if (num < page || num >= page + BBS_PAGESIZE) {
			page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
			show_brdlist(page, 0, newflag);
			update_endline();
		}
		move(3 + num - page, 0);
		prints(">", number);
		if (loop_mode == 0) {
			ch = egetch();
		}
		move(3 + num - page, 0);
		prints(" ");
		if (ch == 'q' || ch == 'e' || ch == KEY_LEFT || ch == EOF)
			break;
		switch (ch) {
			case '*':
				if (nbrd[num].flag & BOARD_CUSTOM_FLAG)
					break; //added by cometcaptor 2007-04-22
				ptr = &nbrd[num];
				show_board_info(ptr->name);
				page = -1;
				break;
			case 'b':
			case Ctrl('B'):
			case KEY_PGUP:
				if (num == 0)
					num = brdnum - 1;
				else
					num -= BBS_PAGESIZE;
				break;
			case 'C':
				if (!HAS_PERM(PERM_LOGIN))
					break;
				if ((GoodBrd.num==0)&&(GoodBrd.nowpid == -1))
					break;
				if (nbrd[num].flag & BOARD_CUSTOM_FLAG)
					break;
				strncpy(addname, nbrd[num].name, STRLEN-8);
				presskeyfor("版名已复制 请按P粘贴", t_lines - 1);
				brdnum=-1;
				break;
			case 'c':
				if (newflag == 1)
					newflag = 0;
				else
					newflag = 1;
				show_brdlist(page, 1, newflag);
				break;
			case 'L':
				m_read();
				page = -1;
				break;
			case 'M':
				m_new();
				page = -1;
				break;
			case 'u':
				modify_user_mode(QUERY);
				t_query();
				page = -1;
				break;
			case 'H':
				getdata(t_lines - 1, 0, "您选择? (1) 本日十大  (2) 系统热点 [1]",
						ans, 2, DOECHO, YEA);
				if (ans[0] == '2')
					show_help("etc/hotspot");
				else
					show_help("0Announce/bbslist/day");
				page = -1;
				break;
				/*		case 'R': {
				 *			char buf[200],path[80],ans[4],*t;
				 *			sprintf(buf, "[1;5;31m立即断线[m: [1;33m以便恢复上次正常离开本站时的未读标记 (Y/N) ? [[1;32mN[m[1;33m]：[m");
				 *			getdata(22, 0, buf, ans, 3, DOECHO, YEA);
				 *			if (ans[0] == 'Y' || ans[0] == 'y' ) {
				 *			setuserfile(path, ".lastread");
				 t = strstr(path,".");
				 *t = '\0';
				 sprintf(buf,"cp %s/.lastread.backup %s/.lastread",path,path);
				 system(buf);
				 sprintf(buf,"cp %s/.boardrc.backup %s/.boardrc",path,path);
				 system(buf);
				 move(23,0);clrtoeol();
				 move(22,0);clrtoeol();
				 prints("[1;33m已经恢复上次正常离开本站时的未读标记[m\n[1;32m请按 Enter 键[1;31m立即断线[m[1;32m, 然后重新登陆本站 [m");
				 egetch();
				 exit(0);
				 }
				 page = -1;
				 break;	
				 }
				 */
			case 'l':
				show_allmsgs();
				page = -1;
				break;
			case 'N':
			case ' ':
			case Ctrl('F'):
			case KEY_PGDN:
				if (num == brdnum - 1)
					num = 0;
				else
					num += BBS_PAGESIZE;
				break;
			case 'P':
				if (!HAS_PERM(PERM_LOGIN))
					break;
				if ((GoodBrd.num==0)&&(GoodBrd.nowpid == -1))
					break;
				add_GoodBrd(addname, GoodBrd.nowpid);
				addname[0]='\0';
				brdnum = -1;
				break;
			case 'p':
			case 'k':
			case KEY_UP:
				if (num-- <= 0)
					num = brdnum - 1;
				break;
			case 'n':
			case 'j':
			case KEY_DOWN:
				if (++num >= brdnum)
					num = 0;
				break;
			case '$':
				num = brdnum - 1;
				break;
			case '!': /* youzi leave */
				return Goodbye();
				break;
			case 'h':
				show_help("help/boardreadhelp");
				page = -1;
				break;
			case '/':
				move(3 + num - page, 0);
				prints(">", number);
				tmpnum = num;
				tmp = search_board(&num);
				move(3 + tmpnum - page, 0);
				prints(" ", number);
				if (tmp == 1)
					loop_mode = 1;
				else {
					loop_mode = 0;
					update_endline();
				}
				break;
			case 'i': //sort by online num
				currentuser.flags[0] ^= BRDSORT_ONLINE;
				qsort(nbrd, brdnum, sizeof (nbrd[0]), cmpboard);
				page = -1;
				substitut_record(PASSFILE, &currentuser,
						sizeof(currentuser), usernum);
				break;
			case 's': /* sort/unsort -mfchen */
				if (currentuser.flags[0] & BRDSORT_FLAG) {
					currentuser.flags[0] ^= BRDSORT_FLAG;
					currentuser.flags[0] |= BRDSORT_ONLINE;
				} else if (currentuser.flags[0] & BRDSORT_ONLINE) {
					currentuser.flags[0] ^= BRDSORT_ONLINE;
				} else {
					currentuser.flags[0] ^= BRDSORT_FLAG;
				}
				qsort(nbrd, brdnum, sizeof (nbrd[0]), cmpboard);
				substitut_record(PASSFILE, &currentuser,
						sizeof(currentuser), usernum);
				page = -1;
				break;
			case 'y':
				if (GoodBrd.num)
					break;
				yank_flag = !yank_flag;
				brdnum = -1;
				break;
			case 'z':
				if (GoodBrd.num)
					break;
				if (HAS_PERM(PERM_LOGIN) && !(nbrd[num].flag
						& BOARD_NOZAP_FLAG)) {
					ptr = &nbrd[num];
					ptr->zap = !ptr->zap;
					ptr->total = -1;
					zapbuf[ptr->pos] = (ptr->zap ? 0 : login_start_time);
					page = 999;
				}
				break;
			case 'a':
				if (!HAS_PERM(PERM_LOGIN))
					break;
				if ((GoodBrd.num)&&(GoodBrd.nowpid == -1))
					break; //added by cometcaptor 2007-04-24 防止在非自定义目录加版面
				if (GoodBrd.num >= GOOD_BRC_NUM) {
					presskeyfor("个人热门版数已经达上限", t_lines - 1);
					//收藏夹
				} else if (GoodBrd.num) {
					int pos;
					char bname[STRLEN];
					struct boardheader fh;
					if (gettheboardname(1, "输入讨论区名 (按空白键自动搜寻): ", &pos,
							&fh, bname, 1)) {
						if (!inGoodBrds(getbnum(bname)-1)) {
							//strcpy (GoodBrd.ID[GoodBrd.num++], bname);
							add_GoodBrd(bname, GoodBrd.nowpid); //modified by cometcaptor 2007-04-21 
							//save_GoodBrd ();
							//GoodBrd.num = 9999;
							brdnum = -1;
							break;
						}
					}
					page = -1;
					//其他情况
				} else { //added by iamfat 2003.11.20 add goodbrd directly
					load_GoodBrd();
					//added by iamfat 2003.12.28 to avoid flow bug
					//struct boardheader *bp1 =NULL;
					//bp1 = getbcache(nbrd[num].name);
					//if (!((nbrd[num].flag & BOARD_DIR_FLAG)&& (bp1->group == 0))){ //根目录不可加入收藏夹Danielfree 06.3.5
					if (GoodBrd.num >= GOOD_BRC_NUM) {
						presskeyfor("个人热门版数已经达上限", t_lines - 1);
					} else if (!inGoodBrds(getbnum(nbrd[num].name)-1)) {
						sprintf(genbuf, "您确定要添加%s到收藏夹吗?", nbrd[num].name);
						if (askyn(genbuf, NA, YEA) == YEA) {
							//strcpy (GoodBrd.ID[GoodBrd.num++], nbrd[num].name);
							add_GoodBrd(nbrd[num].name, 0); //modified by cometcaptor 2007-04-21
							//save_GoodBrd ();
						}
						brdnum = -1;
					}
					//}
					GoodBrd.num = 0;
				}
				break;
			case 'A':
				//added by cometcaptor 2007-04-22 这里写入的是创建自定义目录的代码
				if (!HAS_PERM(PERM_LOGIN))
					break;
				if (GoodBrd.nowpid == 0) {
					if (GoodBrd.num >= GOOD_BRC_NUM)
						presskeyfor("个人热门版数已经达上限", t_lines - 1);
					else {
						//要求输入目录名
						char dirname[STRLEN];
						char dirtitle[STRLEN];
						//.....这里
						dirname[0] = '\0'; //清除上一次创建的目录名
						getdata(t_lines - 1, 0, "创建自定义目录: ", dirname, 17,
								DOECHO, NA); //设成17是因为只显示16个字符，为了防止创建的目录名和显示的不同而限定
						if (dirname[0] != '\0') {
							strcpy(dirtitle, "自定义目录");
							getdata(t_lines - 1, 0, "自定义目录描述: ", dirtitle,
									21, DOECHO, NA);
							mkdir_GoodBrd(dirname, dirtitle, 0);
						}
						brdnum = -1;
					}
				}
				break;
			case 'T':
				//added by cometcaptor 2007-04-25 修改自定义目录名
				if (!HAS_PERM(PERM_LOGIN))
					break;
				if ((GoodBrd.nowpid == 0)&&(brdnum)&&(nbrd[num].flag
						& BOARD_CUSTOM_FLAG)) {
					char dirname[STRLEN];
					char dirtitle[STRLEN];
					int gbid = 0;
					for (gbid = 0; gbid < GoodBrd.num; gbid++)
						if (GoodBrd.boards[gbid].id == nbrd[num].pos)
							break;
					if (gbid == GoodBrd.num)
						break;
					strcpy(dirname, GoodBrd.boards[gbid].filename);
					getdata(t_lines - 1, 0, "修改自定义目录名: ", dirname, 17,
							DOECHO, NA);
					if (dirname[0] != '\0') {
						strcpy(dirtitle, GoodBrd.boards[gbid].title+11);
						getdata(t_lines - 1, 0, "自定义目录描述: ", dirtitle, 21,
								DOECHO, NA);
						if (dirtitle[0] == '\0')
							strcpy(dirtitle, GoodBrd.boards[gbid].title+11);
						strcpy(GoodBrd.boards[gbid].filename, dirname);
						strcpy(GoodBrd.boards[gbid].title+11, dirtitle);
						save_GoodBrd();
					}
					brdnum = -1;
				}
				break;
			case 'd':
				if ((GoodBrd.num)&&(brdnum > 0)) { //modified by cometcaptor 2007-04-24
					int i, pos;
					char ans[5];
					sprintf(genbuf, "要把 %s 从收藏夹中去掉？[y/N]", nbrd[num].name);
					getdata(t_lines - 1, 0, genbuf, ans, 2, DOECHO, YEA);
					//if (ans[0] == 'n' || ans[0] == 'N') {
					//  page = -1;
					//  break;
					//}
					if (ans[0] == 'y' || ans[0] == 'Y') {
						/*pos = inGoodBrds (nbrd[num].name);
						 for (i = pos - 1; i < GoodBrd.num - 1; i++)
						 strcpy (GoodBrd.ID[i], GoodBrd.ID[i + 1]);
						 GoodBrd.num--;
						 */
						//modified by cometcaptor 2007-04-21
						if (nbrd[num].flag & BOARD_CUSTOM_FLAG)
							rmdir_GoodBrd(nbrd[num].pos);
						else {
							pos = inGoodBrds(nbrd[num].pos);
							if (pos) {
								for (i = pos-1; i < GoodBrd.num-1; i++)
									memcpy(&GoodBrd.boards[i],
											&GoodBrd.boards[i+1],
											sizeof(struct goodbrdheader));
								GoodBrd.num--;
							}
							save_GoodBrd();
						}
						//GoodBrd.num = 9999;
						brdnum = -1;
					} else {
						page = -1;
						break;
					}
				}
				break;
			case KEY_HOME:
				num = 0;
				break;
			case KEY_END:
				num = brdnum - 1;
				break;
			case '\n':
			case '\r':
				if (number > 0) {
					num = number - 1;
					break;
				}
				/* fall through */
			case KEY_RIGHT:
				tmp = num;
				num = 0;
				if (brdnum > 0)
					read_board(&nbrd[tmp], newflag); //modified by cometcaptor 2007-04-23
				//read_board (&nbrd[tmp], newflag);
				num = tmp;
				page = -1;
				break;
			case 'S': /* sendmsg ... youzi */
				if (!HAS_PERM(PERM_TALK))
					break;
				s_msg();
				page = -1;
				break;
			case 'o': /* show friends ... youzi */
				if (!HAS_PERM(PERM_LOGIN))
					break;
				t_friends();
				page = -1;
				break;

			default:
				;
		}
		modify_user_mode(newflag ? READNEW : READBRD);
		if (ch >= '0' && ch <= '9') {
			number = number * 10 + (ch - '0');
			ch = '\0';
		} else {
			number = 0;
		}
	}
	clear();
	save_zapbuf();
	nbrd = oldptr; //added by cometcaptor 2006-10-20
	return -1;
}

char *brc_getrecord (ptr, name, pnum, list)
char *ptr, *name;
int *pnum, *list;
{
	int num;
	char *tmp;
	strncpy (name, ptr, BRC_STRLEN);
	ptr += BRC_STRLEN;
	num = (*ptr++) & 0xff;
	tmp = ptr + num * sizeof (int);
	if (num> BRC_MAXNUM) {
		num = BRC_MAXNUM;
	}
	*pnum = num;
	memcpy (list, ptr, num * sizeof (int));
	return tmp;
}

char *brc_putrecord (ptr, name, num, list)
char *ptr, *name;
int num, *list;
{
	if (num> 0) {
		if (num> BRC_MAXNUM) {
			num = BRC_MAXNUM;
		}
		strncpy (ptr, name, BRC_STRLEN);
		ptr += BRC_STRLEN;
		*ptr++ = num;
		memcpy (ptr, list, num * sizeof (int));
		ptr += num * sizeof (int);
	}
	return ptr;
}

void brc_update() {
	char dirfile[STRLEN], *ptr;
	//modify by cometcaptor to fix 'f'function bug 2006-10-15
	char tmp_buf[BRC_MAXSIZE], *tmp;
	char tmp_name[BRC_STRLEN];
	int tmp_list[BRC_MAXNUM], tmp_num;
	int fd, tmp_size;
	if (brc_changed == 0) {
		return;
	}
	ptr = brc_buf;
	if (brc_num > 0) {
		ptr = brc_putrecord(ptr, brc_name, brc_num, brc_list);
	}
	if (1) {
		setuserfile(dirfile, ".boardrc");
		if ((fd = open(dirfile, O_RDONLY)) != -1) {
			tmp_size = read(fd, tmp_buf, sizeof (tmp_buf));
			//add by cometcaptor to fix 'f' function bug 2006-10-15
			if (tmp_size > sizeof(tmp_buf) - BRC_ITEMSIZE)
				tmp_size = sizeof(tmp_buf) - BRC_ITEMSIZE*2 + 1;
			//add end
			close(fd);
		} else {
			tmp_size = 0;
		}
	}
	tmp = tmp_buf;
	while (tmp < &tmp_buf[tmp_size] && (*tmp >= ' ' && *tmp <= 'z')) {
		tmp = brc_getrecord(tmp, tmp_name, &tmp_num, tmp_list);
		if (strncmp(tmp_name, currboard, BRC_STRLEN) != 0) {
			ptr = brc_putrecord(ptr, tmp_name, tmp_num, tmp_list);
		}
	}
	brc_size = (int) (ptr - brc_buf);

	if ((fd = open(dirfile, O_WRONLY | O_CREAT, 0644)) != -1) {
		ftruncate(fd, 0);
		write(fd, brc_buf, brc_size);
		close(fd);
	}
	brc_changed = 0;
}

int brc_initial (boardname) //deardragon0912
char *boardname;
{
	char dirfile[STRLEN], *ptr;
	int fd;
	if (strcmp (currboard, boardname) == 0) {
		return brc_num;
	}
	brc_update ();
	strcpy (currboard, boardname);
	brc_changed = 0;
	if (brc_buf[0] == '\0') {
		setuserfile (dirfile, ".boardrc");
		if ((fd = open (dirfile, O_RDONLY)) != -1) {
			brc_size = read (fd, brc_buf, sizeof (brc_buf));
			close (fd);
		} else {
			brc_size = 0;
		}
	}
	ptr = brc_buf;
	while (ptr < &brc_buf[brc_size] && (*ptr >= ' ' && *ptr <= 'z')) {
		ptr = brc_getrecord (ptr, brc_name, &brc_num, brc_list);
		if (strncmp (brc_name, currboard, BRC_STRLEN) == 0) {
			return brc_num;
		}
	}
	strncpy (brc_name, boardname, BRC_STRLEN);
	brc_list[0] = 1;
	brc_num = 1;
	return 0;
}

void brc_addlist(char *filename) {
	int ftime, n, i;
	if (!strcmp(currentuser.userid, "guest"))
		return;
	ftime = atoi(&filename[2]);
	if ((filename[0] != 'M' && filename[0] != 'G') || filename[1] != '.') {
		return;
	}
	if (brc_num <= 0) {
		brc_list[brc_num++] = ftime;
		brc_changed = 1;
		return;
	}
	for (n = 0; n < brc_num; n++) {
		if (ftime == brc_list[n]) {
			return;
		} else if (ftime > brc_list[n]) {
			if (brc_num < BRC_MAXNUM)
				brc_num++;
			for (i = brc_num - 1; i > n; i--) {
				brc_list[i] = brc_list[i - 1];
			}
			brc_list[n] = ftime;
			brc_changed = 1;
			return;
		}
	}
	if (brc_num < BRC_MAXNUM) {
		brc_list[brc_num++] = ftime;
		brc_changed = 1;
	}
}

int brc_unread (filename)
char *filename;
{
	int ftime, n;
	ftime = atoi (&filename[2]);
	if ((filename[0] != 'M' && filename[0] != 'G') || filename[1] != '.') {
		return 0;
	}
	if (brc_num <= 0)
	return 1;
	for (n = 0; n < brc_num; n++) {
		if (ftime> brc_list[n]) {
			return 1;
		} else if (ftime == brc_list[n]) {
			return 0;
		}
	}
	return 0;
}

/*
 int brc_repair_load_last_read(void)  //quickmouse 02-08-16
 {
 char    dirfile[STRLEN], *ptr, *wptr;
 int     fd, i,  flag, j;
 unsigned int lastbrc;

 resolve_boards();
 brc_update();
 setuserfile(dirfile, ".boardrc");
 // memset(last_read, 0, MAXBOARD * sizeof(unsigned int));
 if ((fd = open(dirfile, O_RDONLY)) != -1)
 {
 brc_size = read(fd, brc_buf, sizeof(brc_buf));
 close(fd);
 }
 else
 {
 return 0;

 }
 fd = 0;
 ptr = brc_buf;
 wptr = ptr;
 while (ptr < &brc_buf[brc_size] && (*ptr >= ' ' && *ptr <= 'z'))
 {
 ptr = brc_getrecord(ptr, brc_name, &brc_num, brc_list);
 flag = 1;
 for(j = 0; j < numboards; j++)
 {
 if(!strcmp(brc_name, bcache[j].filename))
 {
 break;
 }
 }
 if(j == numboards)
 {
 report("Erase boardrc err board name");
 fd = 1;
 flag = 0;
 }
 if(brc_num == 0 || !flag )
 continue;
 lastbrc = time(0);
 for(i = 0; i < brc_num; i++)
 {
 if(brc_list[i] > lastbrc)
 {
 brc_list[i] = lastbrc--;
 flag = 0;
 }
 else
 lastbrc = brc_list[i];
 }
 wptr = brc_putrecord(wptr, brc_name, brc_num, brc_list);
 if(!flag)
 {
 fd = 1;
 sprintf(genbuf, "Repair boardrc of BOARD %s", brc_name);
 report(genbuf);
 }
 //  last_read[j] = brc_list[0];
 brc_size = wptr - brc_buf;
 if(!fd)
 return 0;
 if ((fd = open(dirfile, O_WRONLY | O_CREAT, 0644)) != -1)
 {
 ftruncate(fd, 0);
 write(fd, brc_buf, brc_size);
 close(fd);
 }
 return 0;
 }
 */
