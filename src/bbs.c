#include "bbs.h"
#include <time.h>
#if defined(BSD44)
#include <stdlib.h>
#elif defined(LINUX)
/* include nothing :-) */
#else
#include <rpcsvc/rstat.h>
#endif
#include "mmap.h"

#ifndef DLM
#undef	ALLOWGAME
#endif

//用于保存版主对精华区操作记录的文件名,硬盘上的位置是logs/boardname
char ANN_LOG_PATH[256];
struct postheader header;

extern char buf2[STRLEN];
int hisfriend_wall_logout();
int digestmode;
int local_article;
struct userec currentuser;
struct boardheader *currbp;
int usernum = 0;
char currboard[STRLEN - BM_LEN];
char currBM[BM_LEN - 1];
int selboard = 0;
char someoneID[31];
char topic[STRLEN] = "";
int FFLL = 0;
int noreply = 0;
int mailtoauther = 0;
int totalusers;
char genbuf[1024];
char quote_title[120], quote_board[120];
char quote_file[120], quote_user[120];
#ifndef NOREPLY
char replytitle[STRLEN];
int o_id = 0;
int o_gid = 0;
#endif

int	getlist(char *, char **, int, char **, int, char **, int, char **,
				int);
char *filemargin();
void board_usage();
void canceltotrash();
void add_crossinfo();
int thesis_mode();

/*For read.c*/
int auth_search_down();
int auth_search_up();
int t_search_down();
int t_search_up();
int post_search_down();
int post_search_up();
int thread_up();
int thread_down();
int deny_user();
int club_user();
int show_author();
int SR_first_new();
int SR_last();
int SR_first();
int SR_read();
int SR_read();
int SR_author();
int SR_BMfunc();
int Q_Goodbye();
int online_users_show_override();
int s_msg();
int send_msg();
int b_notes_passwd();
int post_cross(char islocal, int mod);
int BM_range();
int lock();
extern int numboards;
extern int x_lockscreen();
extern time_t login_start_time;
extern char BoardName[];
extern int cmpbnames();
extern char fromhost[];
extern struct boardheader *getbcache();
extern struct bstat *getbstat();


int check_stuffmode() {
	//if (uinfo.mode == RMAIL || (uinfo.mode == READING && junkboard()))
	if (uinfo.mode == RMAIL) //modified by roly 02.03.27
		return YEA;
	else
		return NA;
}

//  取得用户ID为userid的用户信息,保存在currentuser中
static int getcurrentuser(char *userid)
{
	int uid = searchuser(userid);
	if (uid == 0)
		return 0;
	//get_record(PASSFILE, &lookupuser, sizeof(lookupuser), uid);
	memcpy(&currentuser, &(uidshm->passwd[uid-1]), sizeof(currentuser));
	return uid;
}

//取得用户信息,不成功返回-1
int set_safe_record() {
	if (getcurrentuser(currentuser.userid) == 0)
		return -1;
}

/*
 Commented by Erebus 2004-11-03
 input: userid
 output: path such as "home/a/abc" when userid="abc"
 */
char *sethomepath(char *buf, char *userid) {
	sprintf(buf, "home/%c/%s", toupper(userid[0]), userid);
	return buf;
}

/*
 Commented by Erebus 2004-11-03
 copy the title from stitle to topic
 ignore string "Re: "
 */
/*Add by SmallPig*/
//Modified by IAMFAT 2002-05-27
//      取得stitle字符串所表示的标题,保存在全局变量topic中返回
//              如果是回复文章,去掉前面的 "Re: "这4个字符
void setqtitle(char *stitle, int gid) {
	FFLL = 1;
	o_gid = gid;
	if (strncmp(stitle, "Re: ", 4) != 0)
		//commented by iamfat 2002.07.26
		//&& strncmp (stitle, "RE: ", 4) != 0)
		strcpy(topic, stitle);
	else
		strcpy(topic, stitle + 4);
}

/*
 Commented by Erebus 2004-11-03
 check whether the current user is BM	
 */
int chk_currBM(char BMstr[BM_LEN-1], int isclub)
//STRLEN --> BM_LEN  changed by iamfat 2002.07.26
{
	char *ptr;
	char BMstrbuf[BM_LEN - 1];

	if (isclub) {
		if (HAS_PERM(PERM_OCLUB))
			return YEA;
	} else {
		if (HAS_PERM(PERM_BLEVELS))
			return YEA;
	}

	if (!HAS_PERM(PERM_BOARDS))
		return NA;

	strcpy(BMstrbuf, BMstr);
	ptr = strtok(BMstrbuf, ",: ;|&()\0\n");
	while (ptr) {
		if (!strcmp(ptr, currentuser.userid))
			return YEA;
		ptr = strtok(NULL, ",: ;|&()\0\n");
	}
	return NA;
}

// set quotefile as filepath
void setquotefile(char filepath[]) {
	strcpy(quote_file, filepath);
}

//返回用户主目录下所在的文件filename路径
char *setuserfile(char *buf, char *filename) {
	sprintf(buf, "home/%c/%s/%s", toupper(currentuser.userid[0]),
			currentuser.userid, filename);
	return buf;
}

char *setbdir(char *buf, char *boardname) {
	char *dir;

	switch (digestmode) {
		case NA:
			dir = DOT_DIR;
			break;
		case YEA:
			dir = DIGEST_DIR;
			break;
		case 2:
			dir = THREAD_DIR;
			break;
		case 3:
			dir = MARKED_DIR;
			break;
		case 4:
			dir = AUTHOR_DIR;
			break;
			//      case 5:                 /* 同作者 */
			//      case 6:                 /* 同作者 */
			//      case 7:                 /* 标题关键字 */
		case TRASH_MODE:
			dir = TRASH_DIR;
			break;
		case JUNK_MODE:
			dir = JUNK_DIR;
			break;
#ifdef ENABLE_NOTICE
			case NOTICE_MODE:
			dir = NOTICE_DIR;
			break;
#endif
	}
	//dir[STRLEN - 1] = '\0';
	if (digestmode == 5 || digestmode == 6)
		sprintf(buf, "boards/%s/SOMEONE.%s.DIR.%d", boardname, someoneID,
				digestmode - 5);
	else if (digestmode == 7)
		sprintf(buf, "boards/%s/KEY.%s.DIR", boardname, currentuser.userid);
	else
		sprintf(buf, "boards/%s/%s", boardname, dir);
	return buf;
}

/*Add by SmallPig*/
void shownotepad() {
	modify_user_mode(NOTEPAD);
	ansimore("etc/notepad", YEA);
	return;
}

int uleveltochar(char *buf, unsigned int lvl) {
	if (!(lvl & PERM_LOGIN)) {
		strcpy(buf, "--------- ");
		return 0;
	}
	if (lvl < PERM_DEFAULT) {
		strcpy(buf, "- -------- ");
		return 1;
	}
	buf[10] = '\0';
	buf[9] = (lvl & (PERM_BOARDS)) ? 'B' : ' ';
	buf[8] = (lvl & (PERM_CLOAK)) ? '#' : ' ';
	buf[7] = (lvl & (PERM_SEECLOAK)) ? '@' : ' ';
	buf[6] = (lvl & (PERM_OBOARDS)) ? 'O' : ' ';
	buf[5] = (lvl & (PERM_OCLUB)) ? 'C' : ' ';
	buf[4] = (lvl & (PERM_ANNOUNCE)) ? 'N' : ' ';
	buf[3] = (lvl & (PERM_USER)) ? 'U' : ' ';
	buf[2] = (lvl & (PERM_OCBOARD)) ? 'M' : ' ';
	buf[1] = (lvl & (PERM_OCHAT)) ? 'R' : ' ';
	buf[0] = (lvl & (PERM_SYSOPS)) ? 'S' : ' ';
	return 1;
}

int g_board_names(struct boardheader *fhdrp) {
	if (!(fhdrp->flag & BOARD_DIR_FLAG)
			&& ((fhdrp->flag & BOARD_POST_FLAG) || HAS_PERM(fhdrp->level)
					|| (fhdrp->flag & BOARD_NOZAP_FLAG))) {
		AddNameList(fhdrp->filename);
	}
	return 0;
}

int g_dir_names(struct boardheader *fhdrp) {
	if ((fhdrp->flag & BOARD_DIR_FLAG) && ((fhdrp->flag & BOARD_POST_FLAG)
			|| HAS_PERM(fhdrp->level) || (fhdrp->flag & BOARD_NOZAP_FLAG))) {
		AddNameList(fhdrp->filename);
	}
	return 0;
}

int g_all_names(struct boardheader *fhdrp) {
	if ((fhdrp->flag & BOARD_POST_FLAG) || HAS_PERM(fhdrp->level)
			|| (fhdrp->flag & BOARD_NOZAP_FLAG)) {
		AddNameList(fhdrp->filename);
	}
	return 0;
}

// 生成讨论区列表
void make_blist(int mode) {
	CreateNameList();
	if (mode == 1)
		apply_boards(g_board_names, &currentuser);
	else if (mode == 2)
		apply_boards(g_dir_names, &currentuser);
	else
		apply_boards(g_all_names, &currentuser);
}

int board_select() {
	do_select(0, NULL, genbuf);
	return 0;
}

/* added by roly */
int Poststring(char *str, char *nboard, char *posttitle, int mode) {
	int savemode;
	FILE *se;
	char fname[STRLEN];

	savemode = uinfo.mode;
	sprintf(fname, "tmp/AutoPoster.%s.%05d", currentuser.userid, uinfo.pid);
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "%s", str);
		fclose(se);
		Postfile(fname, nboard, posttitle, mode);
		unlink(fname);
		modify_user_mode(savemode);
	}
}

/* add end */

int Postfile(char *filename, char *nboard, char *posttitle, int mode) {
	//char bname[STRLEN];
	char dbname[STRLEN];

	//struct boardheader fh;
	//added by iamfat 2003.11.21
	int old_inmail = in_mail;

	in_mail = NA;
	strcpy(quote_board, currboard);
	strcpy(dbname, currboard);
	strcpy(currboard, nboard);
	strcpy(quote_file, filename);
	strcpy(quote_title, posttitle);
	post_cross('l', mode);
	strcpy(currboard, dbname);
	in_mail = old_inmail;
	return;
}

int get_a_boardname(char *bname, char *prompt) {
	struct boardheader fh;

	make_blist(1);
	namecomplete(prompt, bname);
	if (*bname == '\0') {
		return 0;
	}
	if (search_record(BOARDS, &fh, sizeof (fh), cmpbnames, bname) <= 0) {
		move(1, 0);
		prints("错误的讨论区名称\n");
		pressreturn();
		move(1, 0);
		return 0;
	}
	return 1;
}

/* Add by SmallPig */
int do_cross(int ent, struct fileheader *fileinfo, char *direct) {
	char bname[STRLEN];
	char dbname[STRLEN];
	char ispost[10];

	set_safe_record();
	if (!HAS_PERM(PERM_POST) || digestmode == ATTACH_MODE)
		return DONOTHING;
	//Added by Ashinmarch to Forbide Cross
	if (fileinfo->filename[0] == 's') {
		prints("Type 2 Forbidden to Do cross!!\n");
		return DONOTHING;
	}
	//add end
	if (uinfo.mode != RMAIL)
		sprintf(genbuf, "boards/%s/%s", currboard, fileinfo->filename);
	else
		sprintf(genbuf, "mail/%c/%s/%s", toupper(currentuser.userid[0]),
				currentuser.userid, fileinfo->filename);
	strlcpy(quote_file, genbuf, sizeof (quote_file));
	strcpy(quote_title, fileinfo->title);

	clear();
	prints("[1;33m请珍惜本站资源，请在转载后删除不必要的文章，谢谢！ \n[1;32m并且，您不能将本文转载到本版，如果您觉得不方便的话，请与站长联系。[m\n\n");
	prints("您选择转载的文章是: [[1;33m%s[m]\n", quote_title);
	if (!get_a_boardname(bname, "请输入要转贴的讨论区名称(取消转载请按回车): ")) {
		return FULLUPDATE;
	}
	if (!strcmp(bname, currboard) && uinfo.mode != RMAIL) {
		prints("\n\n             很抱歉，您不能把文章转到同一个版上。");
		pressreturn();
		clear();
		return FULLUPDATE;
	}
	move(3, 0);
	clrtoeol();
	prints("转载 ' %s ' 到 %s 版 ", quote_title, bname);
	move(4, 0);
	getdata(4, 0, "(S)转信 (L)本站 (A)取消? [A]: ", ispost, 9, DOECHO, YEA);
	if (ispost[0] == 's' || ispost[0] == 'S' || ispost[0] == 'L'
			|| ispost[0] == 'l') {
		strcpy(quote_board, currboard);
		strcpy(dbname, currboard);
		strcpy(currboard, bname);
		if (post_cross(ispost[0], 0) == -1) {
			pressreturn();
			move(2, 0);
			strcpy(currboard, dbname);
			return FULLUPDATE;
		}
		strcpy(currboard, dbname);
		prints("\n已把文章 \'%s\' 转贴到 %s 版\n", quote_title, bname);
	} else {
		prints("取消");
	}
	move(2, 0);
	pressreturn();
	return FULLUPDATE;
}

extern int t_cmpuids(int uid, const struct user_info *up);

// Show title when entering a board.
static void readtitle(void)
{
	struct boardheader *bp;
	struct bstat *bs;
	int i, j, bnum, tuid;
	struct user_info uin;
	char tmp[STRLEN];
	char bmlists[5][IDLEN + 1]; // up to 5 BMs.
	char header[120], title[STRLEN];
	const char *readmode;

	bp = getbcache(currboard);
	bs = getbstat(currboard);

	bnum = 0;
	// Copy ID of BMs ('bp->BM') to 'bmlists'.
	for (i = 0, j = 0; bp->BM[i] != '\0' && i < BMNAMELISTLEN; i++) {
		if (bp->BM[i] == ' ') {
			bmlists[bnum][j] = '\0';
			bnum++;
			j = 0;
		} else {
			bmlists[bnum][j++] = bp->BM[i];
		}
	}
#ifdef BMNAMELISTLIMIT
	// If length of BM string exceeds BMNAMELISTLEN, use "...".
	if (i >= BMNAMELISTLEN) {
		j = 0;
		bmlists[bnum][j++] = '.';
		bmlists[bnum][j++] = '.';
		bmlists[bnum][j++] = '.';
	}
#endif
	bmlists[bnum++][j] = '\0';

	if (bp->BM[0] == '\0' || bp->BM[0] == ' ') {
		strcpy(header, "诚征版主中");
	} else {
		strcpy(header, "版主: ");

		// Online BMs are shown in green, offline yellow, cloaking cyan
		// (if currentuser have PERM_SEECLOAK, otherwise in yellow).
		for (i = 0; i < bnum; i++) {
			tuid = getuser(bmlists[i]);
			tuid = search_ulist(&uin, t_cmpuids, tuid);
			if (tuid && uin.active && uin.pid && !uin.invisible)
				sprintf(tmp, "\033[32m%s\033[33m ", bmlists[i]);
			else if (tuid && uin.active && uin.pid && uin.invisible
					&& (HAS_PERM(PERM_SEECLOAK) || usernum == uin.uid))
				sprintf(tmp, "\033[36m%s\033[33m ", bmlists[i]);
			else
				sprintf(tmp, "%s ", bmlists[i]);
			strcat(header, tmp);
		}

	}
	if (chkmail())
		strcpy(title, "[您有信件，按 M 看新信]");
	else if ((bp->flag & BOARD_VOTE_FLAG))
		sprintf(title, "※投票中,按 v 进入投票※");
	else
		strcpy(title, bp->title + 8);

	showtitle(header, title);
	prints(" 离开[\033[1;32m←\033[m,\033[1;32me\033[m] "
		"选择[\033[1;32m↑\033[m,\033[1;32m↓\033[m] "
		"阅读[\033[1;32m→\033[m,\033[1;32mRtn\033[m] "
		"发文章[\033[1;32mCtrl-P\033[m] 砍信[\033[1;32md\033[m] "
		"备忘录[\033[1;32mTAB\033[m] 求助[\033[1;32mh\033[m]\n");

	switch (digestmode) {
		case NORMAL_MODE:
			if (DEFINE(DEF_THESIS))
				readmode = "主题";
			else
				readmode = "一般";
			break;
		case DIGIST_MODE:
			readmode = "文摘";
			break;
		case THREAD_MODE:
			readmode = "主题";
			break;
		case MARK_MODE:
			readmode = "MARK";
			break;
		case ORIGIN_MODE:
			readmode = "原作";
			break;
		case AUTHOR1_MODE:
			readmode = "模糊";
			break;
		case AUTHOR2_MODE:
			readmode = "精确";
			break;
		case KEYWORD_MODE:
			readmode = "标题关键字";
			break;
		case TRASH_MODE:
			readmode = "垃圾箱";
			break;
		case JUNK_MODE:
			readmode = "站务垃圾箱";
			break;
		case ATTACH_MODE:
			readmode = "附件区";
			break;
		default:
			readmode = "未定义";
	}

	if (digestmode == AUTHOR1_MODE || digestmode == AUTHOR2_MODE)
		prints("\033[1;37;44m  编号   %-12s %6s %-9s (关键字: \033[32m%-12s"
			"\033[37m) [\033[33m%s\033[37m同作者阅读] \033[m\n",
			"刊 登 者", "日  期", " 标  题", someoneID, readmode);
	else if (digestmode == KEYWORD_MODE)
		prints("\033[1;37;44m  编号   %-12s %6s %-19s            "
			"[\033%10s式看版] \033[m\n",
			"刊 登 者", "日  期", " 标  题", readmode);
	else if (digestmode == TRASH_MODE)
		prints("\033[1;37;44m  编号   %-12s %6s %-25s            [%6s模式]"
			" \033[m\n",
			"刊 登 者", "日  期", " 标  题", readmode);
	else {
		if (bp->flag & BOARD_NOREPLY_FLAG) {
			prints("\033[1;37;44m  编号   %-12s %6s %-8s  \033[33m"
				"本版不可回复\033[37m    在线:%-4d    [%4s模式] \033[m\n",
				"刊 登 者", "日  期", " 标  题", bs->inboard, readmode);
		} else {
			prints("\033[1;37;44m  编号   %-12s %6s %-25s 在线:%-4d"
				"    [%4s模式] \033[m\n",
				"刊 登 者", "日  期", " 标  题", bs->inboard, readmode);
		}
	}
	clrtobot();
}

char *getshortdate(time_t time) {
	static char str[10];
	struct tm *tm;

	tm = localtime(&time);
	sprintf(str, "%02d.%02d.%02d", tm->tm_year - 100, tm->tm_mon + 1,
			tm->tm_mday);
	return str;
}

char *readdoent(int num, struct fileheader *ent) //Post list
{
	static char buf[128];
	time_t filetime;
	char *date, color[10];
	int type, sameflag, reflag;
	char title[STRLEN], typeprefix[6] = "", typesufix[4] = "";
#ifdef COLOR_POST_DATE
	struct tm *mytm;
#endif
#ifdef FDQUAN
	struct user_info uin;
	const char *idcolor = "";
	extern int t_cmpuids();
#endif
	type = brc_unread(ent->filename) ?
		(!DEFINE(DEF_NOT_N_MASK) ? 'N' : '+') : ' ';
	if ((ent->accessed[0] & FILE_DIGEST)) {
		if (type == ' ')
			type = 'g';
		else
			type = 'G';
	}
	if (ent->accessed[0] & FILE_MARKED) {
		switch (type) {
			case ' ':
				type = 'm';
				break;
			case 'N':
			case '+':
				type = 'M';
				break;
			case 'g':
				type = 'b';
				break;
			case 'G':
				type = 'B';
				break;
		}
	}
	if (ent->accessed[0] & FILE_DELETED) {
		if (type == ' ')
			type = 'w';
		else
			type = 'W';
	}
	if (ent->accessed[1] & FILE_IMPORTED && chkBM(currbp, &currentuser)){
		if (type == ' ')
			strcpy(typeprefix, "\033[42m");
		else
			strcpy(typeprefix, "\033[32m");
		strcpy(typesufix, "\033[m");
	}
	if (ent->accessed[0] & FILE_NOREPLY)
		noreply = 1;
	else
		noreply = 0;
	if (digestmode == ATTACH_MODE) {
		filetime = ent->timeDeleted;
	} else {
		filetime = atoi(ent->filename + 2);
	}
	if (filetime > 740000000) {
#ifdef FDQUAN
		if(time(NULL) - filetime < 24 * 60 * 60)
			date = ctime(&filetime) + 10;
		else
			date = ctime(&filetime) + 4;
#else
		date = ctime(&filetime) + 4;
#endif
	} else {
		date = "";
	}

#ifdef FDQUAN
	if (!search_ulist(&uin, t_cmpuids, getuser(ent->owner)) || !uin.active
		|| (uin.active && uin.invisible && !HAS_PERM (PERM_SEECLOAK))) {
		idcolor = "1;30";
	} else if (uin.invisible && HAS_PERM(PERM_SEECLOAK)) {
		idcolor = "1;36";
	} else if (uin.currbrdnum == getbnum (currboard, &currentuser)
		|| !strcmp (uin.userid, currentuser.userid)) {
		idcolor = "1;37";
	}
#endif

#ifdef COLOR_POST_DATE
	mytm = localtime(&filetime);
	sprintf (color, "\033[1;%dm", 30 + mytm->tm_wday + 1);
#else
	strcpy(color, "");
#endif
	if (digestmode == ATTACH_MODE) {
		sprintf(title, "◆ %s", ent->filename);
	} else if (!strncmp("Re:", ent->title, 3) || !strncmp("RE:",
			ent->title, 3)) {
		sprintf(title, "%s %s", (digestmode == 2) ? ((ent->accessed[1]
				& FILE_LASTONE) ? "└" : "├") : "Re:", ent->title + 4);
		reflag = 1;
	} else {
		sprintf(title, "◆ %s", ent->title);
		reflag = 0;
	}
	if (ent->gid == o_gid)
		sameflag = 1;
	else
		sameflag = 0;
	ent->szEraser[IDLEN] = 0;
	if (digestmode != TRASH_MODE && digestmode != JUNK_MODE)
		ellipsis(title, 49);
	else
		ellipsis(title, 38 - strlen(ent->szEraser));
	if (digestmode == TRASH_MODE || digestmode == JUNK_MODE) {
		sprintf(buf,
#ifdef FDQUAN
				" \033[%sm%5d\033[m %s%c%s \033[%sm%-12.12s %s%6.6s%c%s\033[%sm%s\033[%sm[%s.%s]\033[m",
#else
				" \033[%sm%5d\033[m %s%c%s %-12.12s %s%6.6s%c%s\033[%sm%s\033[%sm[%s.%s]\033[m",
#endif
				(FFLL & sameflag) ? (reflag ? "1;36" : "1;32") : "", num,
#ifdef FDQUAN
				typeprefix, type, typesufix, idcolor, ent->owner, color, date, 
#else
				typeprefix, type, typesufix, ent->owner, color, date, 
#endif
				(FFLL & sameflag) ? '.' : ' ', 
				noreply ? "\033[1;33mx" : " ",
				(FFLL & sameflag) ? (reflag ? "1;36" : "1;32") : "",
				title, (ent->accessed[1] & FILE_SUBDEL) ? "1;31" : "1;32",
				ent->szEraser, getshortdate(ent->timeDeleted));
	} else if (digestmode == ATTACH_MODE) {
		sprintf(buf, " %5d %c %-12.12s %s%6.6s\033[m %s", num, type,
				ent->owner, color, date, title);
	} else {
#ifdef ENABLE_NOTICE
		if (ent->accessed[1] & FILE_NOTICE) {
			sprintf (buf,
#ifdef FDQUAN
					" \033[1;31m [∞]\033[m %c \033[%sm%-12.12s %s%6.6s%c%s\033[%sm%-.49s\033[m",
					type, idcolor, ent->owner, color, date,
#else
					" \033[1;31m [∞]\033[m %c %-12.12s %s%6.6s%c%s\033[%sm%-.49s\033[m",
					type, ent->owner, color, date,
#endif
					(FFLL & sameflag) ? '.' : ' ',
					noreply ? "\033[1;33mx" : " ",
					(FFLL & sameflag) ? (reflag ? "1;36" : "1;32") : "", title);
		} else {
#endif
			sprintf(buf,
#ifdef FDQUAN
				" \033[%sm%5d\033[m %s%c%s \033[%sm%-12.12s %s%6.6s%c%s\033[%sm%-.49s\033[m",
#else
				" \033[%sm%5d\033[m %s%c%s %-12.12s %s%6.6s%c%s\033[%sm%-.49s\033[m",
#endif
				(FFLL & sameflag) ? (reflag ? "1;36" : "1;32") : "", num,
#ifdef FDQUAN
				typeprefix, type, typesufix, idcolor, ent->owner, color, date,
#else
				typeprefix, type, typesufix, ent->owner, color, date,
#endif
				(FFLL & sameflag) ? '.' : ' ',
				noreply ? "\033[1;33mx" : " ",
				(FFLL & sameflag) ? (reflag ? "1;36" : "1;32") : "", title);
#ifdef ENABLE_NOTICE
		}
#endif
	}
	noreply = 0;
	return buf;
}

int cmpfilename(void *fhdr, void *filename)
{
	if (!strncmp(((struct fileheader *)fhdr)->filename,
			(char *)filename, STRLEN))
		return 1;
	return 0;
}

static int cmpdigestfilename(void *digest_name, void *fhdr)
{
	if (!strcmp(((struct fileheader *)fhdr)->filename, (char *)digest_name))
		return 1;
	return 0;
} /* comapare file names for dele_digest function. Luzi 99.3.30 */

int read_post(int ent, struct fileheader *fileinfo, char *direct) {
	char *t;
	char buf[512];
	int ch;

	//int     cou;

	/*
	 if(brc_unread(fileinfo->filename))
	 {
	 fileinfo->visit_num++;
	 substitute_record(direct, fileinfo, sizeof (*fileinfo), ent);
	 }
	 */
	if (digestmode == ATTACH_MODE) {
		struct stat filestat;
		int i, len;
		char tmp[1024];

		sprintf(genbuf, "upload/%s/%s", currboard, fileinfo->filename);
		if (stat(genbuf, &filestat) < 0) {
			clear();
			move(10, 30);
			prints("对不起，%s 不存在！\n", genbuf);
			pressanykey();
			clear();
			return FULLUPDATE;
		}

		clear();
		prints("文件详细信息\n\n");
		prints("版    名:     %s\n", currboard);
		prints("序    号:     第 %d 篇\n", ent);
		prints("文 件 名:     %s\n", fileinfo->filename);
		prints("上 传 者:     %s\n", fileinfo->owner);
		prints("上传日期:     %s\n", getdatestring(fileinfo->timeDeleted, DATE_ZH));
		prints("文件大小:     %d 字节\n", filestat.st_size);
		prints("文件说明:     %s\n", fileinfo->title);
		prints("URL 地址:\n");
		sprintf(tmp, "http://%s/upload/%s/%s", BBSHOST, currboard,
				fileinfo->filename);
		strtourl(genbuf, tmp);
		len = strlen(genbuf);
		clrtoeol();
		for (i = 0; i < len; i += 78) {
			strlcpy(tmp, genbuf + i, 78);
			tmp[78] = '\n';
			tmp[79] = '\0';
			outs(tmp);
		}
		if (!(ch == KEY_UP || ch == KEY_PGUP))
			ch = egetch();
		switch (ch) {
			case 'N':
			case 'Q':
			case 'n':
			case 'q':
			case KEY_LEFT:
				break;
			case ' ':
			case 'j':
			case KEY_RIGHT:
				if (DEFINE(DEF_THESIS)) {
					sread(0, 0, fileinfo);
					break;
				} else
					return READ_NEXT;
			case KEY_DOWN:
			case KEY_PGDN:
				return READ_NEXT;
			case KEY_UP:
			case KEY_PGUP:
				return READ_PREV;
			default:
				break;
		}
		return FULLUPDATE;
	} else {

		brc_addlist(fileinfo->filename);
		if (fileinfo->owner[0] == '-')
			return FULLUPDATE;
		clear();
		strcpy(buf, direct);
		if ((t = strrchr(buf, '/')) != NULL)
			*t = '\0';
		sprintf(genbuf, "%s/%s", buf, fileinfo->filename);
		if (!dashf(genbuf)) {
			clear();
			move(10, 30);
			prints("对不起，本文内容丢失！");
			pressanykey();
			return FULLUPDATE; //deardragon 0729
		}
		strlcpy(quote_file, genbuf, sizeof (quote_file));
		strcpy(quote_board, currboard);
		strcpy(quote_title, fileinfo->title);
		o_gid = fileinfo->gid;
		o_id = fileinfo->id;
		strcpy(quote_user, fileinfo->owner);

#ifndef NOREPLY
		ch = ansimore4(genbuf, NA, currboard, fileinfo->filename, ent);
#else
		ch = ansimore4 (genbuf, YEA, currboard, fileinfo->filename, ent);
#endif
#ifndef NOREPLY
		move(t_lines - 1, 0);
		clrtoeol();
		if (haspostperm(&currentuser, currbp)) {
			prints("\033[0;1;44;31m[阅读文章]  \033[33m回信 R │ 结束 Q,← │上一封 ↑│下一封 <Space>,↓│主题阅读 ^s或p \033[m");
		} else {
			prints("\033[1;44;31m[阅读文章]  \033[33m结束 Q,← │上一封 ↑│下一封 <Space>,<Enter>,↓│主题阅读 p       \033[m");
		}
		/* Re-Write By Excellent */
		FFLL = 1;
		//Re-Write By IAMFAT 2002-05-27
		if (strncmp(fileinfo->title, "Re:", 3) != 0) {
			strcpy(topic, fileinfo->title);
		} else {
			strcpy(topic, fileinfo->title + 4);
		}
		//End IAMFAT

		refresh();
	}
	if (!(ch == KEY_UP || ch == KEY_PGUP))
		ch = egetch();
	switch (ch) {
		case 'N':
		case 'Q':
		case 'n':
		case 'q':
		case KEY_LEFT:
			break;
		case '*':
			show_file_info(ent, fileinfo, direct);
			break;
		case ' ':
		case 'j':
		case KEY_RIGHT:
			if (DEFINE(DEF_THESIS)) { /* youzi */

				sread(0, 0, fileinfo);
				break;
			} else
				return READ_NEXT;
		case KEY_DOWN:
		case KEY_PGDN:
			return READ_NEXT;
		case KEY_UP:
		case KEY_PGUP:
			return READ_PREV;
		case 'Y':
		case 'R':
		case 'y':
		case 'r': {
			struct boardheader *bp;

			bp = getbcache(currboard);
			noreply = fileinfo->accessed[0] & FILE_NOREPLY || bp->flag
					& BOARD_NOREPLY_FLAG;
			local_article = !(fileinfo->filename[STRLEN - 2] == 'S');
			if (!noreply || chkBM(currbp, &currentuser)) {
				do_reply(fileinfo);
			} else {
				clear();
				prints("\n\n    对不起, 该文章有不可 RE 属性, 您不能回复(RE) 这篇文章.    ");
				pressreturn();
				clear();
			}
		}
			break;
		case Ctrl('R'):
			post_reply(ent, fileinfo, direct);
			break;
		case 'g':
			digest_post(ent, fileinfo, direct);
			break;
		case Ctrl('U'):
			sread(1, 1, fileinfo);
			break;
		case Ctrl('N'):
			locate_the_post(fileinfo, fileinfo->title, 5, 0, 1);
			sread(1, 0, fileinfo);
			break;
		case Ctrl('S'):
		case 'p': /* Add by SmallPig */
			sread(0, 0, fileinfo);
			break;
		case Ctrl('A'): /* Add by SmallPig */
			clear();
			show_author(0, fileinfo, '\0');
			return READ_NEXT;
			break;
		case 'S': /* by youzi */
			if (!HAS_PERM(PERM_TALK))
				break;
			clear();
			s_msg();
			break;
		default:
			break;
	}
#endif
	return FULLUPDATE;
}

int skip_post(int ent, struct fileheader *fileinfo, char *direct) {
	brc_addlist(fileinfo->filename);
	return GOTO_NEXT;
}

int do_select(int ent, struct fileheader *fileinfo, char *direct) {
	char bname[STRLEN], bpath[STRLEN];
	struct stat st;

	modify_user_mode(SELECT);

	if (digestmode == TRASH_MODE || digestmode == JUNK_MODE || digestmode
			== ATTACH_MODE)
		return DONOTHING;

	move(0, 0);
	clrtoeol();
	prints("选择一个讨论区 (英文字母大小写皆可)\n");
	clrtoeol();

	make_blist(1);
	namecomplete("输入讨论区名 (按空白键自动搜寻): ", bname);
	if (*bname == '\0')
		return FULLUPDATE;
	setbpath(bpath, bname);
	if ( /* (*bname == '\0') || */(stat(bpath, &st) == -1)) {
		move(2, 0);
		prints("不正确的讨论区.\n");
		pressreturn();
		return FULLUPDATE;
	}
	if (!(st.st_mode & S_IFDIR)) {
		move(2, 0);
		prints("不正确的讨论区.\n");
		pressreturn();
		return FULLUPDATE;
	}

	struct boardheader *bp;

	bp = getbcache(bname);

	if ((bp->flag & BOARD_CLUB_FLAG) && (bp->flag & BOARD_READ_FLAG)
			&& !chkBM(currbp, &currentuser) && !isclubmember(currentuser.userid,
			bname)) {
		clear();
		move(5, 10);
		prints("您不是俱乐部版 %s 的成员，无权进入该版", bname);
		pressanykey();
		return FULLUPDATE;
	}

	selboard = 1;
	brc_update(currentuser.userid, currboard);
	brc_initial(currentuser.userid, bname);
	changeboard(&currbp, currboard, bname);

	move(0, 0);
	clrtoeol();
	move(1, 0);
	clrtoeol();
	setbdir(direct, currboard);
	if (uinfo.currbrdnum && brdshm->bstatus[uinfo.currbrdnum - 1].inboard> 0) {
		brdshm->bstatus[uinfo.currbrdnum - 1].inboard--;
	}
	uinfo.currbrdnum = getbnum (bname, &currentuser);
	update_ulist(&uinfo, utmpent);
	brdshm->bstatus[uinfo.currbrdnum - 1].inboard++;

	return NEWDIRECT;
}

/*

 int
 read_letter(ent, fileinfo, direct)
 int     ent;
 struct fileheader *fileinfo;
 char   *direct;
 {
 setmdir(direct,currentuser.userid);
 return NEWDIRECT;
 }
 */

void do_acction(int type) {
	char buf[STRLEN];

	move(t_lines - 1, 0);
	clrtoeol();
	prints("[1;5m系统处理标题中, 请稍候...[m\n");
	refresh();
	switch (type) {
		case 2:
			sprintf(buf, "bin/thread %s", currboard);
			system(buf);
			break;
		case 3:
		case 4:
		case 5: /* 同作者 */
		case 6: /* 同作者  精确 */
		case 7: /* 标题关键字 */
			marked_all(type - 3);
			break;
	}
}

int acction_mode(int ent, struct fileheader *fileinfo, char *direct) {
	int type;
	extern char currdirect[STRLEN];
	char ch[4];

	if (digestmode != NA) {
		if (digestmode == 5 || digestmode == 6) {
			sprintf(genbuf, "boards/%s/SOMEONE.%s.DIR.%d", currboard,
					someoneID, digestmode - 5);
			unlink(genbuf);
		} else if (digestmode == 7) {
			sprintf(genbuf, "boards/%s/KEY.%s.DIR", currboard,
					currentuser.userid);
			unlink(genbuf);
		}
		digestmode = NA;
		setbdir(currdirect, currboard);
	} else {
		saveline(t_lines - 1, 0);
		move(t_lines - 1, 0);
		clrtoeol();
		ch[0] = '\0';
		getdata(t_lines - 1, 0, "切换模式到: 1)文摘 2)同主题 3)被 m 文章 4)原作"
			" 5)同作者 6)标题关键字 [1]: ", ch, 3, DOECHO, YEA);
		if (ch[0] == '\0')
			ch[0] = '1';
		type = atoi(ch);

		if (type < 1 || type > 6){ 
		saveline (t_lines - 1, 1);
		return PARTUPDATE;
	} else if (type == 6) {
		getdata (t_lines - 1, 0, "您想查找的文章标题关键字: ",
				someoneID, 30, DOECHO, YEA);
		if (someoneID[0] == '\0') {
			saveline (t_lines - 1, 1);
			return PARTUPDATE;
		}
		type = 7;
	} else if (type == 5) {
		strlcpy (someoneID, fileinfo->owner, IDLEN);
		getdata (t_lines - 1, 0, "您想查找哪位网友的文章? ", someoneID, 13,
				DOECHO, YEA);
		if (someoneID[0] == '\0') {
			strlcpy (someoneID, fileinfo->owner, IDLEN);
			move (t_lines - 1, 24);
			prints ("%s", someoneID);
			//saveline (t_lines - 1, 1);
			//return PARTUPDATE;
		}
		getdata (t_lines - 1, 37, "精确查找按 Y， 模糊查找请回车[Enter]",
				ch, 2, DOECHO, YEA);
		if (ch[0] == 'y' || ch[0] == 'Y')
		type = 6;
	}

	digestmode = type;
	setbdir (currdirect, currboard);
	if (digestmode != 1)
	do_acction (type);
	if (!dashf (currdirect)) {
		digestmode = NA;
		setbdir (currdirect, currboard);
		return PARTUPDATE;
	}
}
return NEWDIRECT;
}

int dele_digest(char *dname, char *direc) {
	char digest_name[STRLEN];
	char new_dir[STRLEN];
	char buf[STRLEN];
	char *ptr;
	struct fileheader fh;
	int pos;

	strlcpy(digest_name, dname, STRLEN);
	strcpy(new_dir, direc);
	digest_name[0] = 'G';
	ptr = strrchr(new_dir, '/') + 1;
	strcpy(ptr, DIGEST_DIR);
	pos = search_record(new_dir, &fh, sizeof (fh), cmpdigestfilename,
			digest_name);
	if (pos <= 0) {
		return;
	}
	delete_record(new_dir, sizeof(struct fileheader), pos, cmpfilename,
			digest_name);
	*ptr = '\0';
	sprintf(buf, "%s%s", new_dir, digest_name);
	unlink(buf);
	return;
}

int digest_post(int ent, struct fileheader *fhdr, char *direct) {
	struct fileheader chkfileinfo; // add by quickmouse 01-05-30 检查一下 避免出现.DIR破坏

	if (!chkBM(currbp, &currentuser)) {
		return DONOTHING;
	}
	if (digestmode == YEA)
		return DONOTHING;
#ifdef ENABLE_NOTICE
	if (fhdr->accessed[1] & FILE_NOTICE) {
		return DONOTHING;
	}
#endif
	if (fhdr->accessed[0] & FILE_DIGEST) {
		fhdr->accessed[0] = (fhdr->accessed[0] & ~FILE_DIGEST);
		dele_digest(fhdr->filename, direct);
		bm_log(currentuser.userid, currboard, BMLOG_UNDIGIST, 1);
	} else {
		struct fileheader digest;
		char *ptr, buf[64];

		memcpy(&digest, fhdr, sizeof (digest));
		digest.filename[0] = 'G';
		strcpy(buf, direct);
		ptr = strrchr(buf, '/') + 1;
		ptr[0] = '\0';
		sprintf(genbuf, "%s%s", buf, digest.filename);
		if (dashf(genbuf)) {
			fhdr->accessed[0] = fhdr->accessed[0] | FILE_DIGEST;
			if (get_records(direct, &chkfileinfo, sizeof (chkfileinfo),
					ent, 1) != 1) // add by quickmouse 01-05-30
			{
				return DONOTHING;
			}
			if (strcmp(fhdr->filename, chkfileinfo.filename)) // add by quickmouse 01-05-30
			{
				return DONOTHING;
			}
			substitute_record(direct, fhdr, sizeof (*fhdr), ent);
			return PARTUPDATE;
		}
		digest.accessed[0] = 0;
		sprintf(&genbuf[512], "%s%s", buf, fhdr->filename);
		link(&genbuf[512], genbuf);
		strcpy(ptr, DIGEST_DIR);
		if (get_num_records(buf, sizeof (digest)) >= MAX_DIGEST) {
			move(3, 0);
			clrtobot();
			move(4, 10);
			prints("抱歉，您的文摘文章已经超过 %d 篇，无法再加入...\n", MAX_DIGEST);
			pressanykey();
			return PARTUPDATE;
		}
		append_record(buf, &digest, sizeof (digest));
		fhdr->accessed[0] = fhdr->accessed[0] | FILE_DIGEST;
		fhdr->accessed[0] &= ~FILE_DELETED;
		bm_log(currentuser.userid, currboard, BMLOG_DIGIST, 1);
	}
	if (get_records(direct, &chkfileinfo, sizeof (chkfileinfo), ent, 1)
			!= 1) // add by quickmouse 01-05-30
	{
		return DONOTHING;
	}
	if (strcmp(fhdr->filename, chkfileinfo.filename)) // add by quickmouse 01-05-30
	{
		return DONOTHING;
	}
	substitute_record(direct, fhdr, sizeof (*fhdr), ent);
	return PARTUPDATE;
}

#ifndef NOREPLY
int do_reply(struct fileheader *fh) {
	strcpy(replytitle, fh->title);
	o_id = fh->id;
	o_gid = fh->gid;
	post_article(currboard, fh->owner);
	replytitle[0] = '\0';
	return FULLUPDATE;
}
#endif

/**
 * Tell if a line is meaningless.
 * @param str The string to be checked.
 * @return True if str is quotation of a quotation or contains only white
           spaces, false otherwise.
 */
bool garbage_line(const char *str)
{
	int qlevel = 0;

	while (*str == ':' || *str == '>') {
		str++;
		if (*str == ' ')
			str++;
		if (qlevel++ >= 1)
			return true;
	}
	while (*str == ' ' || *str == '\t' || *str == '\r')
		str++;
	return (*str == '\n');
}

/**
 * Find newline in [begin, end).
 * @param begin The head pointer.
 * @param end The off-the-end pointer.
 * @return Off-the-end pointer to the first newline, 'end' if not found.
 */
static const char *get_newline(const char *begin, const char *end)
{
	while (begin < end) {
		if (*begin++ == '\n')
			return begin;
	}
	return begin;
}

enum {
	MAX_QUOTE_LINES = 9,     ///< Maximum quoted lines (for mode 'R').
	MAX_QUOTE_BYTES = 1024,  ///< Maximum quoted bytes (for mode 'R').
};

/**
 * Quote in given mode.
 * @param orig The file to be quoted.
 * @param file The output file.
 * @param mode Quote mode (R/S/Y/A/N).
 */
// TODO: do not use quote_file in callers.
void do_quote(const char *orig, const char *file, char mode)
{
	bool mail = !strncmp(orig, "mail", 4);
	FILE *fp = fopen(file, "w");
	if (!fp)
		return;
	if (mode != 'N') {
		mmap_t m;
		m.oflag = O_RDONLY;
		if (mmap_open(orig, &m) == 0) {
			const char *begin = m.ptr;
			const char *end = begin + m.size;
			const char *lend = get_newline(begin, end);

			// Parse author & nick.
			const char *quser = begin, *ptr = lend;
			while (quser < lend) {
				if (*quser++ == ' ')
					break;
			}
			while (--ptr >= begin) {
				if (*ptr == ')')
					break;
			}
			++ptr;
			fprintf(fp, "\n【 在 ");
			if (ptr > quser)
				fwrite(quser, ptr - quser, sizeof(char), fp);
			fprintf(fp, " 的%s中提到: 】\n", mail ? "来信" : "大作");

			bool header = true, tail = false;
			size_t lines = 0, bytes= 0;
			while (1) {
				ptr = lend;
				if (ptr >= end)
					break;
				lend = get_newline(ptr, end);
				if (header && *ptr == '\n') {
					header = false;
					continue;
				}
				if (lend - ptr == 3 && !memcmp(ptr, "--\n", 3)) {
					tail = true;
					if (mode == 'Y' || mode == 'R')
						break;
				}
				if (!header || mode == 'A') {
					if ((mode == 'Y' || mode == 'R')
							&& garbage_line(ptr)) {
						continue;
					}
					if (mode == 'S' && lend - ptr > 10 + sizeof("※ 来源:·")
							&& !memcmp(ptr + 10, "※ 来源:·", sizeof("※ 来源:·"))) {
						break;
					}
					if (mode == 'R') {
						bytes += lend - ptr;
						if (++lines > MAX_QUOTE_LINES
								|| bytes > MAX_QUOTE_BYTES) {
							fputs(": .................（以下省略）", fp);
							break;
						}
					}
					if (mode != 'S')
						fputs(": ", fp);
					fwrite(ptr, lend - ptr, sizeof(char), fp);
				}
			}
			mmap_close(&m);
		}
	}
	if (currentuser.signature && !header.chk_anony)
		add_signature(fp, currentuser.userid, currentuser.signature);
	fclose(fp);
}

/* Add by SmallPig */
void getcross(char *filepath, int mode) {
	FILE *inf, *of;
	char buf[256];
	char owner[STRLEN];

	//int count, owner_found = 0;
	int owner_found = 0;
	char *p = NULL;
	time_t now;

	now = time(0);
	inf = fopen(quote_file, "r");
	of = fopen(filepath, "w");
	if (inf == NULL || of == NULL) {
		report("Cross Post error", currentuser.userid);
		return;
	}
	if (mode == 0 || mode == 4) {
		if (in_mail == YEA) {
			in_mail = NA;
			write_header(of, 1 /* 不写入 .posts */);
			in_mail = YEA;
		} else
			write_header(of, 1 /* 不写入 .posts */);
		/*
		 if (in_mail && strncmp(buf, "寄信人: ", 8)) {
		 strcpy(owner, currentuser.userid);
		 owner_found = 0;
		 } else {
		 */
		/*
		 if(fgets(buf,256,inf))
		 if(strncmp (buf+2, "信人: ", 6))
		 {
		 owner_found = 0;
		 strcpy (owner, "Unkown User");
		 }
		 else
		 {
		 for (count = 8;
		 buf[count] != ' ' && buf[count] != '\n'
		 && buf[count] != '\0'; count++)
		 owner[count - 8] = buf[count];
		 owner[count - 8] = '\0';
		 owner_found = 1;
		 }
		 */
		//optimized by iamfat 2002.08.18
		if (fgets(buf, 256, inf) && (p = strstr(buf, "信人: "))) {
			p += 6;
			strtok(p, " \n\r");
			strcpy(owner, p);
			owner_found = 1;
		} else {
			owner_found = 0;
			strcpy(owner, "Unkown User");
		}
		//optimized end.
		//                      }
		if (in_mail == YEA)
			fprintf(of, "[1;37m【 以下文字转载自 [32m%s [37m的信箱 】[m\n",
					currentuser.userid);
		else {
			struct boardheader *bp;

			bp = getbcache(quote_board);

			fprintf(
					of,
					"[1;37m【 以下文字转载自 [32m%s [37m%s区 】[m\n",
					((bp->flag & BOARD_POST_FLAG) || (bp->level == 0)) ? quote_board
							: "未知", mode ? "精华" : "讨论");
			mode = 0;
		}
		if (owner_found) {
			// Clear Post header
			/*
			 if(fgets (buf, 256, inf) != NULL)
			 {
			 if (buf[0] != '\n')fgets (buf, 256, inf);
			 } */
			while (fgets(buf, 256, inf) && buf[0] != '\n')
				;
			fprintf(of, "\033[1m【 原文由[32m %s[37m 所发表 】[m\n\n", owner);
		} else
			fseek(inf, (long) 0, SEEK_SET);
	} else if (mode == 1) {
		fprintf(of,
		//           "[1;41;33m发信人: deliver (自动发信系统), 信区: %-12.12s                          [m\n",
				//modified by iamfat 2002.08.18
				"[1;33m发信人: deliver (自动发信系统), 信区: %s[m\n", currboard);
		/* modified by roly 2002.01.13 */
		//              fprintf(of, "1;41;33m发信人: deliver (自动发信系统), 信区: %-12.12s                          m\n", currboard);
		fprintf(of, "标  题: %s\n", quote_title);
		fprintf(of, "发信站: %s自动发信系统 (%s)\n\n", BoardName, getdatestring(now, DATE_ZH));
		/*		fprintf(of, "【此篇文章是由自动发信系统所张贴】\n\n"); */
		/* Added by Amigo 2002.04.17. Set BMS announce poster as 'BMS'. */
	} else if (mode == 3) {
		fprintf(of,
		//           "[1;41;33m发信人: BMS (版主管理员), 信区: %-12.12s                          [m\n",
				//modified by iamfat 2002.08.18
				"[1;33m发信人: BMS (版主管理员), 信区: %s[m\n", currboard);
		fprintf(of, "标  题: %s\n", quote_title);
		fprintf(of, "发信站: %s自动发信系统 (%s)\n\n", BoardName, getdatestring(now, DATE_ZH));
		/* Add end. */
	} else if (mode == 2) {
		write_header(of, 0 /* 写入 .posts */);
	}
	while (fgets(buf, 256, inf) != NULL) {
		//    if ((strstr (buf, "【 以下文字转载自 ") && strstr (buf, "讨论区 】"))
		//      || (strstr (buf, "【 原文由") && strstr (buf, "所发表 】")))
		//      continue;
		//    else
		fprintf(of, "%s", buf);
	}
	fclose(inf);
	fclose(of);
	/* added by roly 2002.02.26 to remove crossinfo from file which is crossposted from 0Announce 
	 if (strstr(quote_file,"0Announce/")) {
	 sprintf(buf,"cp %s %s",quote_file,filepath);
	 system(buf);
	 }
	 add end */
	*quote_file = '\0';
}

//发文动作
int do_post() {
	*quote_file = '\0';
	*quote_user = '\0';
	local_article = YEA;
	return post_article(currboard, (char *) NULL);
}

int show_file_info(int ent, struct fileheader *fileinfo, char *direct) {
	char weblink[256], tmp[80], type[20], filepath[STRLEN];
	time_t filetime;
	struct stat filestat;
	struct boardheader *bp;
	struct bstat *bs;
	int i, len, unread;

	if (digestmode == ATTACH_MODE)
		return DONOTHING;

	bp = getbcache(currboard);
	bs = getbstat(currboard);
	if (in_mail)
		setmfile(filepath, currentuser.userid, fileinfo->filename);
	else
		setbfile(filepath, currboard, fileinfo->filename);

	if (stat(filepath, &filestat) < 0) {
		clear();
		move(10, 30);
		prints("对不起，当前文章不存在！\n", filepath);
		pressanykey();
		clear();
		return FULLUPDATE;
	}

	if (in_mail) {
		snprintf(weblink, 256,
				"http://%s/bbs/mailcon?f=%s&n=%d\n",
				BBSHOST, fileinfo->filename, ent - 1);
		if (fileinfo->accessed[0] & FILE_READ)
			unread = 0;
		else
			unread = 1;
		if (fileinfo->accessed[0] & MAIL_REPLY) {
			if (fileinfo->accessed[0] & FILE_MARKED)
				strcpy(type, "保留且已回复");
			else
				strcpy(type, "已回复");
		} else if (fileinfo->accessed[0] & FILE_MARKED)
			strcpy(type, "保留");
		else
			strcpy(type, "普通");
	} else {
		snprintf(weblink, 256, "http://%s/bbs/con?bid=%d&f=%u%s\n",
				BBSHOST, currbp - bcache + 1, fileinfo->id,
				fileinfo->accessed[1] & FILE_NOTICE ? "&s=1" : "");
		unread = brc_unread(fileinfo->filename);
		if (fileinfo->accessed[0] & FILE_DIGEST) {
			if (fileinfo->accessed[0] & FILE_MARKED)
				strcpy(type, "B文");
			else
				strcpy(type, "G文");
		} else if (fileinfo->accessed[0] & FILE_MARKED)
			strcpy(type, "M文");
		else if (fileinfo->accessed[0] & FILE_DELETED)
			strcpy(type, "水文");
		else
			strcpy(type, "普通");

	}
	clear();
	move(0, 0);
	prints("%s的详细信息:\n\n", in_mail ? "邮箱信件" : "版面文章");
	if (!in_mail) {
		prints("版    名:     %s\n", bp->filename);
		prints("版    主:     %s\n", bp->BM);
		prints("在线人数:     %d 人\n", bs->inboard);
	}
	prints("序    号:     第 %d %s\n", ent, in_mail ? "封" : "篇");
	prints("标    题:     %s\n", fileinfo->title);
	prints("%s:     %s\n", in_mail ? "发 信 人" : "作    者", fileinfo->owner);
	filetime = atoi(fileinfo->filename + 2);
	prints("时    间:     %s\n", getdatestring(filetime, DATE_ZH));
	prints("阅读状态:     %s\n", unread ? "未读" : "已读");
	prints("文章类型:     %s\n", type);
	prints("大    小:     %d 字节\n", filestat.st_size);
	prints("文 章 id:     %d\n", fileinfo->id);
	prints("文 章gid:     %d\n", fileinfo->gid);
	prints("文章reid:     %d\n", fileinfo->reid);
	len = strlen(weblink);
	prints("URL 地址:\n", weblink);
	for (i = 0; i < len; i += 78) {
		strlcpy(tmp, weblink + i, 78);
		tmp[78] = '\n';
		tmp[79] = '\0';
		outs(tmp);
	}

	pressanykey();
	return FULLUPDATE;
}

/*ARGSUSED*/
int post_reply(int ent, struct fileheader *fileinfo, char *direct) {
	char uid[STRLEN];
	char title[STRLEN];

	//char *t;
	//FILE *fp;
	int savemode = uinfo.mode;

	if (!strcmp(currentuser.userid, "guest"))
		return DONOTHING;
	clear();
	// Added by Amigo 2002.06.10. To add mail right check.
	if (!HAS_PERM(PERM_MAIL)) {
		move(4, 0);
		prints("\n\n        您尚未完成注册，或者发送信件的权限被封禁。");
		pressreturn();
		return FULLUPDATE;
	}
	// Added end.
	if (check_maxmail()) {
		pressreturn();
		return FULLUPDATE;
	}
	modify_user_mode(SMAIL);

	// indicate the quote file/user
	setbfile(quote_file, currboard, fileinfo->filename);
	strcpy(quote_user, fileinfo->owner);

	// find the author
	// comment by iamfat 2002.08.18 这段对我们没有用处
	/*
	 if (!getuser (quote_user))
	 {
	 genbuf[0] = '\0';
	 fp = fopen (quote_file, "r");
	 if (fp != NULL)
	 {
	 fgets (genbuf, 255, fp);
	 fclose (fp);
	 }
	 t = strtok (genbuf, ":");
	 if (strncmp (t, "发信人", 6) == 0 ||
	 strncmp (t, "寄信人", 6) == 0 ||
	 strncmp (t, "Posted By", 9) == 0 || 
	 strncmp (t, "作  者", 6) == 0) {
	 while (t != NULL) {
	 t = (char *) strtok (NULL, " \r\t\n<>");
	 if (t == NULL)
	 break;
	 if (!invalidaddr (t))
	 break;
	 }
	 if (t != NULL)strncpy (uid, t, STRLEN);
	 }
	 else {
	 prints ("对不起，该帐号已经不存在。\n");
	 pressreturn ();
	 }
	 }
	 else
	 strcpy (uid, quote_user);
	 */
	//rewrote one.  iamfat 2002.08.18 写一个简单的
	if (!getuser(quote_user)) {
		prints("对不起，该帐号已经不存在!\n");
		pressreturn();
	} else
		strcpy(uid, quote_user);
	//rewrote end.

	//make the title
	/*
	 if(  toupper (fileinfo->title[0]) != 'R' || 
	 fileinfo->title[1] != 'e' ||
	 fileinfo->title[2] != ':')
	 strcpy (title, "Re: ");
	 else
	 title[0] = '\0';
	 strncat (title, fileinfo->title, STRLEN - 5); */
	//optimized by iamfat 2002.08.18
	sprintf(title, "%s%s", strncmp(fileinfo->title, "Re: ", 4) ? "Re: "
			: "", fileinfo->title);
	//optimized end.

	// edit, then send the mail
	switch (do_send(uid, title)) {
		case -1:
			prints("系统无法送信\n");
			break;
		case -2:
			prints("送信动作已经中止\n");
			break;
		case -3:
			prints("使用者 '%s' 无法收信\n", uid);
			break;
		case -4:
			prints("使用者 '%s' 无法收信，信箱已满\n", uid);
			break;
		default:
			prints("信件已成功地寄给原作者 %s\n", uid);
	}
	pressreturn();
	modify_user_mode(savemode);
	in_mail = NA;
	return FULLUPDATE;
}

//added by iamfat 2002.07.25
char month[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
		"Aug", "Sep", "Oct", "Nov", "Dec" };
time_t en_gettime(char *str) {
	struct tm tms;
	int i;

	tms.tm_isdst = 0;
	//Thu Jul 25 08:33:34 2002
	str = strtok(str, " :"); //Ignore the week
	str = strtok(NULL, " :"); //now is month
	i = 12;
	while (i-- && strcmp(str, month[i]))
		;
	if (i < 0)
		return time(0);
	tms.tm_mon = i;
	str = strtok(NULL, " :"); //now is day
	tms.tm_mday = atoi(str);
	str = strtok(NULL, " :"); //now is hour
	tms.tm_hour = atoi(str);
	str = strtok(NULL, " :"); //now is min
	tms.tm_min = atoi(str);
	str = strtok(NULL, " :"); //now is sec
	tms.tm_sec = atoi(str);
	str = strtok(NULL, " :"); //now is year
	tms.tm_year = atoi(str) - 1900;
	return mktime(&tms);
}

time_t cn_gettime(char *str) {
	struct tm tms;
	char *cval = str;

	tms.tm_isdst = 0;
	while (*str) {
		if (!isdigit(*str)) {
			if (!strncmp(str, "年", 2)) {
				*str = '\0';
				tms.tm_year = atoi(cval) - 1900;
				str += 2;
			} else if (!strncmp(str, "月", 2)) {
				*str = '\0';
				tms.tm_mon = atoi(cval) - 1;
				str += 2;
			} else if (!strncmp(str, "日", 2)) {
				*str = '\0';
				tms.tm_mday = atoi(cval);
				str += 2;
			} else if (*str == ':') {
				*str = '\0';
				tms.tm_hour = atoi(str - 2);
				str += 3;
				*str = '\0';
				tms.tm_min = atoi(str - 2);
				str += 3;
				*str = '\0';
				tms.tm_sec = atoi(str - 2);
				str++;
			} else
				str++;
			cval = str;
		} else
			str++;
	}
	return mktime(&tms);
}


static int undelcheck(void *fh1, void *fh2)
{
	return (atoi(((struct fileheader *)fh1)->filename + 2)
		> atoi(((struct fileheader *)fh2)->filename + 2));
}

int date_to_fname(char *postboard, time_t now, char *fname) {
	static unsigned ref = 0;

	sprintf(fname, "M.%ld.%X%X", now, uinfo.pid, ref);
	ref++;
	return 0;
}

// Add by SmallPig
// mode = 0 普通
//        1 deliver
//        2 
//        3 BMS
//        4 精华区zz
int post_cross(char islocal, int mode)
{
	struct fileheader postfile;
	struct boardheader *bp;
	char filepath[STRLEN], fname[STRLEN];
	char buf[256], buf4[STRLEN], whopost[IDLEN + 2];

	time_t now;

	bp = getbcache(currboard);
	if (!haspostperm(&currentuser, bp) && !mode) {
		prints("\n\n 您尚无权限在 %s 版发表文章.\n", currboard);
		return -1;
	}
	memset(&postfile, 0, sizeof (postfile));
	//  strncpy (save_filename, fname, 4096);
	now = time(0);
	//sprintf (fname, "M.%d.A", now);
	//optimized by iamfat 2002.08.18
	if ((mode == 0 || mode == 4) && (strncmp(quote_title, "[转载]", 6)
			&& strncmp(quote_title, "Re: [转载]", 10)))
		sprintf(buf4, "[转载]%.70s", quote_title);
	else if ((mode == 0 || mode == 4) && !strncmp(quote_title, "Re: [转载]",
			10))
		//modified by money 04.01.17 for judge Re & cross
		sprintf(buf4, "[转载]Re: %.70s", quote_title + 10);
	else
		strcpy(buf4, quote_title);
	//optimized end

	strlcpy(save_title, buf4, STRLEN);

	if (date_to_fname(currboard, now, fname) < 0)
		return -1;
	strcpy(postfile.filename, fname);
	if (mode == 1)
		/* modified by roly 2002.01.13 */
		strcpy(whopost, "deliver");
	/* Following 2 lines added by Amigo 2002.04.17. Set BMS announce poster as 'BMS'. */
	else if (mode == 3)
		strcpy(whopost, "BMS");
	else
		strcpy(whopost, currentuser.userid);

	strlcpy(postfile.owner, whopost, STRLEN);
	setbfile(filepath, currboard, postfile.filename);

	local_article = YEA;
	if ((islocal == 'S' || islocal == 's') && (bp->flag & BOARD_OUT_FLAG))
		local_article = NA;

	modify_user_mode(POSTING);

	getcross(filepath, mode);

	strlcpy(postfile.title, save_title, sizeof(postfile.title));
	if (local_article == YEA || !(bp->flag & BOARD_OUT_FLAG)) {
		postfile.filename[STRLEN - 9] = 'L';
		postfile.filename[STRLEN - 10] = 'L';
	} else {
		postfile.filename[STRLEN - 9] = 'S';
		postfile.filename[STRLEN - 10] = 'S';
		outgo_post(&postfile, currboard);
	}
	postfile.id = get_nextid(currboard);
	postfile.gid = postfile.id;
	postfile.reid = postfile.id;
	//合集默认不可re.  eefree 06.6.6
	if (!strncmp(postfile.title, "[合集]", 6) && (mode == 2)) {
		postfile.accessed[0] |= FILE_NOREPLY;
	}
	//deliver 默认不可re.  eefree 06.6.6  
	if (mode == 1) {
		postfile.accessed[0] |= FILE_NOREPLY;
	}
	setwbdir(buf, currboard);

	if (append_record(buf, &postfile, sizeof (postfile)) == -1) {
		if (mode == 0 || mode == 4) {
			sprintf(buf,
					"cross_posting '%s' on %s: append_record failed!",
					postfile.title, quote_board);
		} else {
			sprintf(buf, "Posting '%s' on %s: append_record failed!",
					postfile.title, quote_board);
		}
		report(buf, currentuser.userid);
		pressreturn();
		clear();
		return 1;
	}
	/* brc_addlist( postfile.filename ) ; */
	updatelastpost(currboard);
	if (mode == 0 || mode == 4) {
		add_crossinfo(filepath, 1);
		//commented by roly 2002.02.26 to disable add_crossinfo for compatible with crosspost form 0Announce
		sprintf(buf, "cross_posted '%s' on %s", postfile.title, currboard);
		report(buf, currentuser.userid);
	}
	/*    else
	 sprintf(buf,"自动发表系统 POST '%s' on '%s'", postfile.title, currboard) ;
	 report(buf) ;*/
	return 1;
}

void add_crossinfo(char *filepath, int mode) {
	FILE *fp;
	int color;

	color = (currentuser.numlogins % 7) + 31;
	if ((fp = fopen(filepath, "a")) == NULL)
		return;
	fprintf(fp, "--\n[m[1;%2dm※ 转%s:·%s %s·[FROM: %-.20s][m\n", color,
			(mode == 1) ? "载" : "寄", BoardName, BBSHOST, mask_host(fromhost));
	fclose(fp);
	return;
}

// 显示版面提示
int show_board_notes(char bname[30], int command) {
	char buf[256];

	sprintf(buf, "vote/%s/notes", bname);
	if (dashf(buf)) {
		if (command == 1) {
			ansimore2(buf, NA, 0, 19);
		} else {
			ansimore(buf, YEA);
		}
		return 1;
	}
	strcpy(buf, "vote/notes");
	if (dashf(buf)) {
		if (command == 1) {
			ansimore2(buf, NA, 0, 19);
		} else {
			ansimore(buf, YEA);
		}
		return 1;
	}
	return -1;
}

int show_user_notes() {
	char buf[256];

	setuserfile(buf, "notes");
	if (dashf(buf)) {
		ansimore(buf);
		return FULLUPDATE;
	}
	clear();
	move(10, 15);
	prints("您尚未在 InfoEdit->WriteFile 编辑个人备忘录。\n");
	pressanykey();
	return FULLUPDATE;
}

int outgo_post(struct fileheader *fh, char *board) {
	char buf[256];

	sprintf(buf, "%s\t%s\t%s\t%s\t%s\n", board, fh->filename,
			header.chk_anony ? "Anonymous" : currentuser.userid,
			/* fh->filename, header.chk_anony ? board : currentuser.userid, */
			/* modified by roly 2002.01.13  change author to "Anonymous" */
			header.chk_anony ? "我是匿名天使" : currentuser.username, save_title);
	file_append("innd/out.bntp", buf);

}

int post_article(char *postboard, char *mailid) {
	struct fileheader postfile;
	struct boardheader *bp;
	char filepath[STRLEN], fname[STRLEN], buf[120];
	int aborted;
	time_t now = time(0);

	static time_t lastposttime = 0;
	static int failure = 0;

	modify_user_mode(POSTING);

	/* added by roly 02.05.18 灌水机 */
	if ((abs(now - lastposttime) < 3 || failure >= 9999)) {
		clear();
		move(5, 10);
		failure++;
		if (failure > 9999) {
			if (failure >= 10020) {
				abort_bbs(0);
			}
			prints("对不起，您在被劝阻多次的情况下，仍不断试图发文。");
			move(6, 10);
			prints("您目前被系统认定为灌水机，请退出后重新登陆！[%d/20]", failure - 9999);
		} else {
			prints("您太辛苦了，先喝杯咖啡歇会儿，3 秒钟后再发表文章。\n");
			if (failure > 5) {
				move(6, 10);
				prints("您在被劝阻的情况下仍旧试图发表文章。[%d/20]", failure - 5);
				if (failure >= 25) {
					securityreport("多次试图发表文章，被系统判定为灌水机", 0, 3);
					failure = 9999;
				}
			}
			lastposttime = now;
		}
		pressreturn();
		clear();
		return FULLUPDATE;
	} // if (abs..)

	/* add end */
	bp = getbcache(postboard);
	if (bp == NULL || !haspostperm(&currentuser, bp) || digestmode == DIGIST_MODE 
			|| digestmode == TRASH_MODE || digestmode == JUNK_MODE) {
		move(3, 0);
		clrtobot();
		if (digestmode == NA) {
			prints("\n\n        此讨论区是唯读的, 或是您尚无权限在此发表文章。");
		} else {
			prints("\n\n     目前是文摘模式, 所以不能发表文章 (按左键可离开此模式)。");
		}
		pressreturn();
		clear();
		return FULLUPDATE;
	}

	memset(&postfile, 0, sizeof (postfile));
	//俱乐部
	if ((bp->flag & BOARD_CLUB_FLAG) && !chkBM(currbp, &currentuser)
			&& !isclubmember(currentuser.userid, postboard)) {
		move(3, 0);
		clrtobot();
		prints("\n\n              您不是俱乐部 %s 的成员，请向版主申请加入俱乐部", postboard);
		pressreturn();
		clear();
		return FULLUPDATE;

	}

	clear();
	show_board_notes(postboard, 1);
	if ((bp->flag & BOARD_OUT_FLAG) && replytitle[0] == '\0') {
		local_article = NA;
	}
#ifndef NOREPLY
	if (replytitle[0] != '\0') {
		if (strncasecmp(replytitle, "Re:", 3) == 0) {
			strlcpy(header.title, replytitle, sizeof(header.title));
		} else {
			snprintf(header.title, sizeof(header.title), "Re: %s", replytitle);
		}
		header.reply_mode = 1;
	} else
#endif
	{
		header.title[0] = '\0';
		header.reply_mode = 0;
	}
	strcpy(header.ds, postboard);
	header.postboard = YEA;
#ifdef ENABLE_PREFIX
	header.prefix[0] = '\0';
#endif
	if (post_header(&header) == YEA) {
#ifdef ENABLE_PREFIX
		if (!header.reply_mode && header.prefix[0]) {
#ifdef FDQUAN
			if (bp->flag & BOARD_PREFIX_FLAG)
				snprintf(postfile.title, sizeof(postfile.title),
						"\033[1;33m[%s]\033[m%s", header.prefix, header.title);
			else
				snprintf(postfile.title, sizeof(postfile.title),
						"\033[1m[%s]\033[m%s", header.prefix, header.title);
#else
			snprintf(postfile.title, sizeof(postfile.title),
					"[%s]%s",header.prefix, header.title);
#endif
		}
		else
#endif
		{
			ansi_filter(header.title, header.title);
			strlcpy(postfile.title, header.title, sizeof(postfile.title));
		}
		strlcpy(save_title, postfile.title, STRLEN);
	} else {
		return FULLUPDATE;
	}
	now = time(0);

	lastposttime = now;
	failure = 0;

	if (date_to_fname(postboard, now, fname) < 0)
		return -1;
	strcpy(postfile.filename, fname);
	in_mail = NA;

	strlcpy(postfile.owner, (header.chk_anony) ? "Anonymous"
			: currentuser.userid, STRLEN);
	setbfile(filepath, postboard, postfile.filename);
	modify_user_mode(POSTING);
	do_quote(quote_file, filepath, header.include_mode);
	aborted = vedit(filepath, YEA, YEA);
	if (aborted == -1) {
		unlink(filepath);
		clear();
		return FULLUPDATE;
	}

	strlcpy(postfile.title, save_title, sizeof(postfile.title));
	// TODO: ...
	if ((local_article == YEA) || !(bp->flag & BOARD_OUT_FLAG)) {
		postfile.filename[STRLEN - 9] = 'L';
		postfile.filename[STRLEN - 10] = 'L';
	} else {
		postfile.filename[STRLEN - 9] = 'S';
		postfile.filename[STRLEN - 10] = 'S';
		outgo_post(&postfile, postboard);
	}

	if (noreply) {
		postfile.accessed[0] |= FILE_NOREPLY;
		noreply = 0;
	}
	//added by iamfat 2003.03.19
	if (mailtoauther) {
		if (header.chk_anony) {
			prints("对不起，您不能在匿名版使用寄信给原作者功能。");
		} else {
			extern int cmpfnames();
			struct override or;

			sethomefile(buf, mailid, "rejects");
			if (!search_record
					(buf, &or, sizeof (or), cmpfnames, currentuser.userid)
					&& !mail_file (filepath, mailid, postfile.title)) {
				prints ("信件已成功地寄给原作者 %s", mailid);
			}
			else {
				prints ("信件邮寄失败，%s 无法收信。", mailid);
			}
		}
		mailtoauther = 0;
		pressanykey();
	}
	setwbdir(buf, postboard);
	postfile.id = get_nextid(currboard);
	if (header.reply_mode == 1) {
		postfile.gid = o_gid;
		postfile.reid = o_id;
	} else {
		postfile.gid = postfile.id;
		postfile.reid = postfile.id;
	}
	save_gid = postfile.gid;
	if (append_record(buf, &postfile, sizeof (postfile)) == -1) {
		sprintf(buf, "posting '%s' on %s: append_record failed!",
				postfile.title, currboard);
		report(buf, currentuser.userid);
		pressreturn();
		clear();
		return FULLUPDATE;
	}
	brc_addlist(postfile.filename);
	updatelastpost(currboard);
	sprintf(buf, "posted '%s' on %s", postfile.title, currboard);
	report(buf, currentuser.userid);

	if (!junkboard(currbp) && !header.chk_anony) {
		set_safe_record();
		currentuser.numposts++;
		substitut_record(PASSFILE, &currentuser, sizeof (currentuser),
				usernum);
	}
	bm_log(currentuser.userid, currboard, BMLOG_POST, 1);
	return FULLUPDATE;
}

int IsTheFileOwner(struct fileheader *fileinfo) {
	char buf[512];
	int posttime;

	if (fileinfo->owner[0] == '-' || strstr(fileinfo->owner, "."))
		return 0;
	if (strcmp(currentuser.userid, fileinfo->owner))
		return 0;
	strcpy(buf, &(fileinfo->filename[2]));
	buf[strlen (buf) - 2] = '\0';
	posttime = atoi(buf);
	if (posttime < currentuser.firstlogin)
		return 0;
	return 1;
}

int change_title(char *fname, char *title) {
	FILE *fp, *out;
	char buf[256], outname[STRLEN];
	int newtitle = 0;

	char systembuf[256];

	if ((fp = fopen(fname, "r")) == NULL)
		return 0;

	sprintf(outname, "tmp/editpost.%s.%05d", currentuser.userid, uinfo.pid);
	if ((out = fopen(outname, "w")) == NULL)
		return 0;

	while ((fgets(buf, 256, fp)) != NULL) {
		if (!strncmp(buf, "标  题: ", 8) && newtitle == 0) {
			fprintf(out, "标  题: %s\n", title);
			newtitle = 1;
			continue;
		}
		fputs(buf, out);
	}
	fclose(fp);
	fclose(out);
	//rename(outname, fname);
	sprintf(systembuf, "/bin/mv %s/%s %s/%s", BBSHOME, outname, BBSHOME,
			fname);
	//modified by roly 02.03.30
	system(systembuf);
	chmod(fname, 0644);

	return;
}

int edit_post(int ent, struct fileheader *fileinfo, char *direct) {
	char buf[512];
	char *t;
	extern char currmaildir[STRLEN];

	if (!strcmp(currboard, "GoneWithTheWind") || digestmode == ATTACH_MODE)
		return DONOTHING;

	if (!in_mail) {
		if (!chkBM(currbp, &currentuser)) {
			struct boardheader *bp;

			//if (strcmp(fileinfo->owner, currentuser.userid)) return DONOTHING;
			if (!IsTheFileOwner(fileinfo))
				return DONOTHING;
			bp = getbcache(currboard);
			if ((bp->flag & BOARD_ANONY_FLAG) && !strcmp(fileinfo->owner,
					currboard))
				return DONOTHING;
		}
	}
	modify_user_mode(EDIT);
	clear();
	if (in_mail)
		strcpy(buf, currmaildir);
	else
		strcpy(buf, direct);
	if ((t = strrchr(buf, '/')) != NULL)
		*t = '\0';
	//Added by Ashinmarch to support sharedmail
	if (fileinfo->filename[0] == 's') {
		prints("Type 2 Forbidden to Edit post!\n");
		return DONOTHING;
	}
	//added end
	sprintf(genbuf, "%s/%s", buf, fileinfo->filename);
	if (vedit(genbuf, NA, NA) == -1)
		return FULLUPDATE;
	if (!in_mail) {
		sprintf(genbuf, "edited post '%s' on %s", fileinfo->title,
				currboard);
		report(genbuf, currentuser.userid);
	}
	return FULLUPDATE;
}

int getnam(char *direct, int num, char *id) {
	FILE *fp;
	int size;
	struct fileheader ff;
	size = sizeof(struct fileheader);
	fp = fopen(direct, "r");
	fseek(fp, (num - 1) * size, SEEK_SET);
	strcpy(id, "none.");
	if (fread(&ff, size, 1, fp))
		strcpy(id, ff.filename);
	fclose(fp);
}

int edit_title(int ent, struct fileheader *fileinfo, char *direct) {
	long i;
	struct fileheader xfh; //added by roly,02.03.29

	char buf[STRLEN];
	char id1[20]; //added by zhch, 12.19, to fix T d T bug.

	getnam(direct, ent, id1);

	if (!strcmp(currboard, "GoneWithTheWind") || digestmode == ATTACH_MODE)
		return DONOTHING;

	if (!chkBM(currbp, &currentuser)) {
		struct boardheader *bp;

		//      if(strcmp(fileinfo->owner,currentuser.userid))return DONOTHING;
		if (!IsTheFileOwner(fileinfo))
			return DONOTHING;
		bp = getbcache(currboard);
		if ((bp->flag & BOARD_ANONY_FLAG) && !strcmp(fileinfo->owner,
				currboard))
			return DONOTHING;
	}
#ifdef ENABLE_NOTICE
	if (fileinfo->accessed[1] & FILE_NOTICE)
	return DONOTHING;
#endif

	ansi_filter(buf, fileinfo->title);
	getdata(t_lines - 1, 0, "新文章标题: ", buf, 50, DOECHO, NA);
	if (!strcmp(buf, fileinfo->title))
		return PARTUPDATE;
	check_title(buf);
	if (buf[0] != '\0') {
		/*
		 char    tmp[STRLEN * 2], *t;
		 getnam(direct, ent,id2);
		 if(strcmp(id1,id2)) return PARTUPDATE;
		 strcpy(fileinfo->title, buf);
		 strcpy(tmp, direct);
		 if ((t = strrchr(tmp, '/')) != NULL)
		 *t = '\0';
		 sprintf(genbuf, "%s/%s", tmp, fileinfo->filename);
		 change_title(genbuf, buf);
		 substitute_record(direct, fileinfo, sizeof(*fileinfo), ent);
		 *///modified by roly 02.03.29
		char tmp[STRLEN * 2], *t;

		strcpy(fileinfo->title, buf);
		strcpy(tmp, direct);
		if ((t = strrchr(tmp, '/')) != NULL)
			*t = '\0';
		sprintf(genbuf, "%s/%s", tmp, fileinfo->filename);
		change_title(genbuf, buf);
		/* Leeward 99.07.12 added below to fix a big bug */
		setbdir(buf, currboard);
		for (i = ent; i > 0; i--) {
			if (0 == get_record(buf, &xfh, sizeof (xfh), i)) {
				if (0 == strcmp(xfh.filename, fileinfo->filename)) {
					ent = i;
					break;
				}
			}
		}
		if (0 == i)
			return PARTUPDATE;
		/* Leeward 99.07.12 added above to fix a big bug */
		substitute_record(direct, fileinfo, sizeof (*fileinfo), ent);
	}
	return PARTUPDATE;
}

int underline_post(int ent, struct fileheader *fileinfo, char *direct) {
	struct fileheader chkfileinfo; // add by quickmouse 01-05-30 检查一下 避免出现.DIR破坏
	char *path = direct;

	/* Modified by Amigo 2002.06.27. Poster can't set or unset noreply tag. */
	//      if(!chk_currBM(currBM)&&!IsTheFileOwner(fileinfo)) {
	if (!chkBM(currbp, &currentuser)) {
		return DONOTHING;
	}
#ifdef ENABLE_NOTICE
	if (fileinfo->accessed[1] & FILE_NOTICE) {
		char notice[STRLEN];
		struct fileheader tmpfh;

		get_noticedirect (direct, notice);
		ent =
		search_record (notice, &tmpfh, sizeof (struct fileheader), cmpfilename,
				fileinfo->filename);
		if (ent <= 0)
		return DONOTHING;
		path = notice;
	} else {
#endif
	if (get_records(direct, &chkfileinfo, sizeof (chkfileinfo), ent, 1)
			!= 1) // add by quickmouse 01-05-30
	{
		return DONOTHING;
	}
	if (strcmp(fileinfo->filename, chkfileinfo.filename)) // add by quickmouse 01-05-30
	{
		return DONOTHING;
	}
#ifdef ENABLE_NOTICE
}
#endif
	if (fileinfo->accessed[0] & FILE_NOREPLY) {
		fileinfo->accessed[0] &= ~FILE_NOREPLY;
		bm_log(currentuser.userid, currboard, BMLOG_UNCANNOTRE, 1);
	} else {
		fileinfo->accessed[0] |= FILE_NOREPLY;
		bm_log(currentuser.userid, currboard, BMLOG_CANNOTRE, 1);
	}
	substitute_record(path, fileinfo, sizeof (*fileinfo), ent);
	return PARTUPDATE;
}

int makeDELETEDflag(int ent, struct fileheader *fileinfo, char *direct) {
	if (!(chkBM(currbp, &currentuser)) || fileinfo->accessed[0] & (FILE_MARKED
			| FILE_DIGEST)) {
		return DONOTHING;
	}
#ifdef ENABLE_NOTICE
	if (fileinfo->accessed[1] & FILE_NOTICE)
	return DONOTHING;
#endif
	if (fileinfo->accessed[0] & FILE_DELETED) {
		fileinfo->accessed[0] &= ~FILE_DELETED;
		bm_log(currentuser.userid, currboard, BMLOG_UNWATER, 1);
	} else {
		fileinfo->accessed[0] |= FILE_DELETED;
		bm_log(currentuser.userid, currboard, BMLOG_WATER, 1);
	}
	substitute_record(direct, fileinfo, sizeof (*fileinfo), ent);
	return PARTUPDATE;
}

#ifdef ENABLE_NOTICE
int make_notice (int ent, struct fileheader *fh, char *direct)
{
	char path[256];
	char file1[STRLEN], file2[STRLEN]; //added by cometcaptor 2006-05-25

	if (digestmode != NA)
	return DONOTHING;
	if (!chkBM(currbp, &currentuser))
	return DONOTHING;
	get_noticedirect (direct, path);
	if (fh->accessed[1] & FILE_NOTICE) {
		struct fileheader tmpfh;
		int pos = search_record (path, &tmpfh, sizeof (struct fileheader),
				cmpfilename, fh->filename);

		if (pos> 0) {
			fh->accessed[1] &= ~FILE_NOTICE;
			//if(0==insert_record(direct, sizeof(struct fileheader), undelcheck, fh)){
			delete_record (path, sizeof (struct fileheader), pos,
					cmpfilename, fh->filename);
			//added by cometcaptor 2006-05-25
			setbfile (file1, currboard, fh->filename);
			unlink (file1);
			//}
		}
	} else if (get_num_records (path, sizeof (struct fileheader)) < MAX_NOTICE) {
		//modified by cometcaptor 2006-05-25
		setbfile (file1, currboard, fh->filename);
		fh->filename[0] = 'T';
		setbfile (file2, currboard, fh->filename);

		fh->accessed[1] |= FILE_NOTICE;

		if (link (file1, file2) == 0) {
			if (append_record (path, fh, sizeof (struct fileheader)) == 0) {
				//if(!(fh->accessed[0] & FILE_NOREPLY))
				//    underline_post(ent, fh, direct);
			} else
			unlink (file2);
		}
	}
	updatelastpost (currboard);
	return DIRCHANGED;
}
#endif
//added by cometcaptor 2006-05-29
int move_notice(int ent, struct fileheader *fh, char *direct) {
	char path[256], tmppath[256];
	int lockfd;
	if (digestmode!=NA)
		return DONOTHING;
	if (!chkBM(currbp, &currentuser))
		return DONOTHING;
	if (fh->accessed[1]&FILE_NOTICE) {
		get_noticedirect(direct, path);
		struct fileheader tmpfh;
		int pos=search_record(path, &tmpfh, sizeof(struct fileheader),
				cmpfilename, fh->filename);
		strcat(tmppath, "tmp/");
		strcat(tmppath, currboard);
		strcat(tmppath, ".lock");
		lockfd = creat(tmppath, 0600);
		fb_flock(lockfd, LOCK_EX);
		if (pos > 0) {
			delete_record(path, sizeof(struct fileheader), pos,
					cmpfilename, fh->filename);
			append_record(path, fh, sizeof(struct fileheader));
		} else
			return DONOTHING;
		fb_flock(lockfd, LOCK_UN);
		close(lockfd);
	}
	updatelastpost(currboard);
	return DIRCHANGED;
}
//add end
int mark_post(int ent, struct fileheader *fileinfo, char *direct) {
	struct fileheader chkfileinfo; // add by quickmouse 01-05-30 检查一下 避免出现.DIR破坏

	if (!chkBM(currbp, &currentuser)) {
		return DONOTHING;
	}
#ifdef ENABLE_NOTICE
	if (fileinfo->accessed[1] & FILE_NOTICE)
	return DONOTHING;
#endif

	if (get_records(direct, &chkfileinfo, sizeof (chkfileinfo), ent, 1)
			!= 1) {
		// add by quickmouse 01-05-30

		return DONOTHING;
	}
	if (strcmp(fileinfo->filename, chkfileinfo.filename)) // add by quickmouse 01-05-30
	{
		return DONOTHING;
	}
	if (fileinfo->accessed[0] & FILE_MARKED) {
		fileinfo->accessed[0] &= ~FILE_MARKED;
		bm_log(currentuser.userid, currboard, BMLOG_UNMARK, 1);
	} else {
		fileinfo->accessed[0] |= FILE_MARKED;
		fileinfo->accessed[0] &= ~FILE_DELETED;
		bm_log(currentuser.userid, currboard, BMLOG_MARK, 1);
	}
	substitute_record(direct, fileinfo, sizeof (*fileinfo), ent);
	return PARTUPDATE;
}

// 区段删除,从id1到id2;版面文件与精华区有区别
int delete_range(char *filename, int id1, int id2)
{
	struct fileheader *ptr, *wptr, *rptr;
	char dirpath[80], *p;
	int ret;
	int deleted=0;
	int subflag;
	int lookupuid;
	mmap_t m;

	strcpy(dirpath, filename);
	p=strrchr(dirpath, '/');
	if (!p) {
		return -1;
	}
	p++;

	BBS_TRY {
		m.oflag = O_RDWR;
		if (mmap_open(filename, &m) < 0)
			BBS_RETURN(-1);
		ret = 0;
		ptr = m.ptr;
		if (id1 > id2 || id2 * sizeof(struct fileheader) > m.size) {
			ret = -2; //区段范围超过文件大小
		} else {
			wptr = rptr = ptr + id1 - 1;
			while (id1 <= id2) {
				if (rptr->accessed[0] & FILE_MARKED) {
					if (wptr != rptr)
						memcpy(wptr, rptr, sizeof(struct fileheader));
					wptr++;
					rptr++;
				} else if (in_mail == NA) {
					if (digestmode == NA) {
						subflag = rptr->accessed[0] & FILE_DELETED ? YEA : NA;
						canceltotrash(filename, currentuser.userid, rptr,
								subflag, !HAS_PERM(PERM_OBOARDS));
						if (subflag == YEA && !junkboard(currbp)) {
							lookupuid = getuser(rptr->owner);
							if (lookupuid> 0 && lookupuser.numposts > 0) {
								lookupuser.numposts--;
								substitut_record (PASSFILE, &lookupuser, 
										sizeof (struct userec), lookupuid);
							}
						}
					} else if (digestmode == YEA || digestmode == ATTACH_MODE) {
						*p = '\0';
						strcat(dirpath, rptr->filename);
						unlink(dirpath);
					}
					rptr++;
					deleted++;
				} else {
					*p = '\0';
					strcat(dirpath, rptr->filename);
					unlink(dirpath);
					rptr++;
					deleted++;
				}
				id1++;
			}
		}
		if (ret == 0) {
			memcpy(wptr, rptr, m.size - sizeof(struct fileheader) * id2);
			mmap_truncate(&m, m.size - sizeof(struct fileheader) * deleted);
		}
	}
	BBS_CATCH {
		ret = -3;
	}
	BBS_END mmap_close(&m);
	//add by danielfree to recount the attach files size.06-10-31
	if (digestmode==ATTACH_MODE) {
		char apath[256], cmd[256];
		sprintf(apath, "%s/upload/%s", BBSHOME, currboard);
		if (dashd(apath)) {
			sprintf(cmd, "du %s|cut -f1>%s/.size", apath, apath);
			system(cmd);
		}
	}//add end
	return ret;
}

int del_range(int ent, struct fileheader *fileinfo, char *direct) {
	char num[8];
	int inum1, inum2;

	if (uinfo.mode == READING) {
		if (!chkBM(currbp, &currentuser)) {
			return DONOTHING;
		}
	}
	if (digestmode > 1 && digestmode != ATTACH_MODE)
		return DONOTHING;
	getdata(t_lines - 1, 0, "首篇文章编号: ", num, 6, DOECHO, YEA);
	inum1 = atoi(num);
	if (inum1 <= 0) {
		move(t_lines - 1, 50);
		prints("错误编号...");
		egetch();
		return PARTUPDATE;
	}
	getdata(t_lines - 1, 25, "末篇文章编号: ", num, 6, DOECHO, YEA);
	inum2 = atoi(num);
	if (inum2 < inum1) {
		move(t_lines - 1, 50);
		prints("错误区间...");
		egetch();
		return PARTUPDATE;
	}
	move(t_lines - 1, 50);
	if (askyn("确定删除", NA, NA) == YEA) {
		delete_range(direct, inum1, inum2);
		fixkeep(direct, inum1, inum2);
		if (uinfo.mode == READING) {
			sprintf(genbuf, "Range delete %d-%d on %s", inum1, inum2,
					currboard);
			//securityreport (genbuf, 0, 2);
			bm_log(currentuser.userid, currboard, BMLOG_DELETE, 1);
			updatelastpost(currboard);
		} else {
			sprintf(genbuf, "Range delete %d-%d in mailbox", inum1, inum2);
			report(genbuf, currentuser.userid);
		}
		return DIRCHANGED;
	}
	move(t_lines - 1, 50);
	clrtoeol();
	prints("放弃删除...");
	egetch();
	return PARTUPDATE;
}

//added by iamfat 2002.07.24
//extern int insert_record(char *filename, char *rptr, int size, int offset, int size2);
//modified by iamfat 2002.07.24
int UndeleteArticle(int ent, struct fileheader *fileinfo, char *direct) /* undelete 一篇文章 Leeward 98.05.18 */
{
	return _UndeleteArticle(ent, fileinfo, direct, YEA);
}

//加了个response变量 方便区域恢复的时候不回显
int _UndeleteArticle(int ent, struct fileheader *fileinfo, char *direct,
		int response) /* undelete 一篇文章 Leeward 98.05.18 */
{
	char buf[1024];
	struct stat st;
	int owned;
	int subflag;

	/* Modified by Amigo 2002.06.27. Add undelete for big D board. */
	// if (strcmp(currboard, "deleted")&&strcmp(currboard,"junk")) {
	if (digestmode != TRASH_MODE && digestmode != JUNK_MODE) {
		return DONOTHING;
	}

	sprintf(buf, "boards/%s/%s", currboard, fileinfo->filename);
	stat(buf, &st);
	if (!S_ISREG(st.st_mode))
		return DONOTHING;

	//if(digestmode==TRASH_MODE)
	setwbdir(buf, currboard);
	subflag = (fileinfo->accessed[1] & FILE_SUBDEL);
	fileinfo->accessed[1] &= ~FILE_SUBDEL;
	if (insert_record(buf, sizeof(struct fileheader), undelcheck, fileinfo)
			!= 0) {
		return DONOTHING;
	}
	updatelastpost(currboard);

	sprintf(buf, "boards/%s/%s", currboard,
			digestmode == TRASH_MODE ? ".TRASH" : ".JUNK");
	delete_record(buf, sizeof(struct fileheader), ent, cmpfilename,
			fileinfo->filename);
	owned = getuser(fileinfo->owner);
	if (!junkboard(currbp) && subflag && owned != 0 && atoi(fileinfo->filename
			+ 2) > lookupuser.firstlogin) {
		lookupuser.numposts++;
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec),
				owned);
	}
	sprintf(genbuf, "UNDEL '%s' on %s", fileinfo->title, currboard);
	report(genbuf, currentuser.userid);
	bm_log(currentuser.userid, currboard, BMLOG_UNDELETE, 1);
	return DIRCHANGED;
}

int _del_post(int ent, struct fileheader *fileinfo, char *direct,
		int subflag, int hasjudge) {
	char buf[512];
	char usrid[STRLEN];
	char *t;
	int owned = 0, fail, IScurrent = 0;
	int posttime;
	extern int SR_BMDELFLAG;

	//if (fileinfo->accessed[0] & (FILE_MARKED|FILE_DIGEST)) return DONOTHING;
	//modified by roly 02.03.29

	if ((SR_BMDELFLAG) && (fileinfo->accessed[0] & (FILE_MARKED)))
		return DONOTHING;

	if (fileinfo->accessed[1] & (FILE_NOTICE))
		return DONOTHING;

	if (fileinfo->accessed[0] & (FILE_DELETED))
		subflag = YEA;

	if (digestmode > 1 && digestmode != ATTACH_MODE)
		return DONOTHING;
	if (fileinfo->owner[0] == '-')
		return DONOTHING;
	strcpy(usrid, fileinfo->owner);
	if (!strstr(usrid, "."))
		IScurrent = !strcmp(usrid, currentuser.userid);

	strcpy(buf, &(fileinfo->filename[2]));
	buf[strlen (buf) - 2] = '\0';
	posttime = atoi(buf);
	if (!IScurrent) {
		owned = getuser(usrid);
		if (owned != 0) {
			if (posttime < lookupuser.firstlogin)
				owned = 0;
		}
	} else
		owned = posttime > currentuser.firstlogin;

	if (hasjudge == YEA && !chkBM(currbp, &currentuser)) {
		struct boardheader *bp;

		if (!(owned && IScurrent))
			return DONOTHING;
		bp = getbcache(currboard);
		if ((bp->flag & BOARD_ANONY_FLAG) && !strcmp(usrid, currboard))
			return DONOTHING;
	}
	if (!SR_BMDELFLAG) {
		sprintf(genbuf, "删除文章 [%-.55s]", fileinfo->title);
		if (askyn(genbuf, NA, YEA) == NA) {
			move(t_lines - 1, 0);
			prints("放弃删除文章...");
			clrtoeol();
			egetch();
			return PARTUPDATE;
		}
	}
	fail = delete_record(direct, sizeof(struct fileheader), ent,
			cmpfilename, fileinfo->filename);
	if (!fail) {
		strcpy(buf, direct);
		if ((t = strrchr(buf, '/')) != NULL)
			*t = '\0';
		sprintf(genbuf, "Del '%s' on %s", fileinfo->title, currboard);
		report(genbuf, currentuser.userid);

		updatelastpost(currboard);
		/*if(subflag==NA)
		 cancelpost (currboard, currentuser.userid, fileinfo, 2);
		 else cancelpost (currboard, currentuser.userid, fileinfo, owned && IScurrent); */
		//added by iamfat 2003.03.01
		if (digestmode == NA)
			canceltotrash(direct, currentuser.userid, fileinfo, subflag,
					(SR_BMDELFLAG || !IScurrent)
							&& !HAS_PERM(PERM_OBOARDS));
		//add by danielfree to recount the attach files size.06-10-31
		if (digestmode==ATTACH_MODE) {
			char apath[256], cmd[256];
			sprintf(apath, "%s/upload/%s", BBSHOME, currboard);
			if (dashd(apath)) {
				sprintf(cmd, "du %s|cut -f1>%s/.size", apath, apath);
				system(cmd);
			}
		}//add end
		sprintf(genbuf, "%s/%s", buf, fileinfo->filename);
		if (digestmode > 0)
			unlink(genbuf);
		if (!junkboard(currbp) && !digestmode) {
			if (owned && IScurrent) {
				set_safe_record();
				if (currentuser.numposts > 0 && subflag == YEA)
					currentuser.numposts--;
				substitut_record(PASSFILE, &currentuser,
						sizeof (currentuser), usernum);
			} else if (owned && BMDEL_DECREASE) {
				if (lookupuser.numposts > 0 && subflag == YEA)
					lookupuser.numposts--;
				substitut_record(PASSFILE, &lookupuser,
						sizeof(struct userec), owned);
			}
		}
		return DIRCHANGED;
	} else if (SR_BMDELFLAG) {
		return -1;
	}
	move(t_lines - 1, 0);
	prints("删除失败...");
	clrtoeol();
	egetch();
	return PARTUPDATE;
}

int del_post(int ent, struct fileheader *fileinfo, char *direct) {
	if (!strcmp(currboard, "GoneWithTheWind"))
		return DONOTHING;

	return _del_post(ent, fileinfo, direct, YEA, YEA);
}

int new_flag_clearto(int ent, struct fileheader *fileinfo, char *direct) {
	if (uinfo.mode != READING)
		return DONOTHING;
	return brc_clear(ent, direct, NA);
}

int new_flag_clear(int ent, struct fileheader *fileinfo, char *direct) {
	if (uinfo.mode != READING)
		return DONOTHING;
	return brc_clear(ent, direct, YEA);
}

/* Added by netty to handle post saving into (0)Announce */
int Save_post(int ent, struct fileheader *fileinfo, char *direct) {
	if (!HAS_PERM(PERM_BOARDS) || digestmode == ATTACH_MODE)
		return DONOTHING;
	if (!in_mail && !chkBM(currbp, &currentuser))
		return DONOTHING;

	return (a_Save("0Announce", currboard, fileinfo, NA));
}

/* Added by netty to handle post saving into (0)Announce */
int Import_post(int ent, struct fileheader *fileinfo, char *direct) {
	//if (!chk_currBM(currBM, 0))
	if (!HAS_PERM(PERM_BOARDS) || digestmode == ATTACH_MODE)
		return DONOTHING;
	if (DEFINE(DEF_MULTANNPATH) && set_ann_path(NULL, NULL,
			ANNPATH_GETMODE) == 0)
		return FULLUPDATE;
	a_Import("0Announce", currboard, ent, fileinfo, direct, NA);
	if (DEFINE(DEF_MULTANNPATH))
		return FULLUPDATE;
	return DONOTHING;
}

int check_notespasswd() {
	FILE *pass;
	char passbuf[20], prepass[STRLEN];
	char buf[STRLEN];

	setvfile(buf, currboard, "notespasswd");
	if ((pass = fopen(buf, "r")) != NULL) {
		fgets(prepass, STRLEN, pass);
		fclose(pass);
		prepass[strlen (prepass) - 1] = '\0';
		getdata(2, 0, "请输入秘密备忘录密码: ", passbuf, 19, NOECHO, YEA);
		if (passbuf[0] == '\0' || passbuf[0] == '\n')
			return NA;
		if (!checkpasswd(prepass, passbuf)) {
			move(3, 0);
			prints("错误的秘密备忘录密码...");
			pressanykey();
			return NA;
		}
	}
	return YEA;
}

int show_b_secnote() {
	char buf[256];

	clear();
	setvfile(buf, currboard, "secnotes");
	if (dashf(buf)) {
		if (!check_notespasswd())
			return FULLUPDATE;
		clear();
		ansimore(buf, NA);
	} else {
		move(3, 25);
		prints("此讨论区尚无「秘密备忘录」。");
	}
	pressanykey();
	return FULLUPDATE;
}

int show_b_note() {
	clear();
	if (show_board_notes(currboard, 2) == -1) {
		move(3, 30);
		prints("此讨论区尚无「备忘录」。");
		pressanykey();
	}
	return FULLUPDATE;
}

int into_announce() {
	char found[STRLEN];

	//char ablogpath[256];
	if (a_menusearch("0Announce", currboard, found)) {
		sprintf(ANN_LOG_PATH, "logs/%s", currboard);
		//sprintf(genbuf, "%s/.log", found);
		//sprintf(ablogpath, "%s/%s", BBSHOME, ANN_LOG_PATH);
		//unlink(genbuf);
		//symlink(ablogpath, genbuf);
		a_menu("", found, (HAS_PERM(PERM_ANNOUNCE) ? PERM_BOARDS : 0), 0);
		//return FULLUPDATE;
		return MODECHANGED;
	}
	return DONOTHING;
}

int into_myAnnounce() {
	Personal("*");
	return FULLUPDATE;
}

int into_PAnnounce() {
	Personal(NULL);
	return FULLUPDATE;
}

int Personal(char *userid) {
	char found[256], lookid[IDLEN + 6];
	int id;

	if (!userid) {
		clear();
		move(2, 0);
		usercomplete("您想看谁的个人文集: ", lookid);
		if (lookid[0] == '\0') {
			clear();
			return 1;
		}
	} else
		strcpy(lookid, userid);
	if (lookid[0] == '*') {
		sprintf(lookid, "/%c/%s", toupper(currentuser.userid[0]),
				currentuser.userid);
	} else {
		if (!(id = getuser(lookid))) {
			lookid[1] = toupper(lookid[0]);
			if (lookid[1] < 'A' || lookid[1] > 'Z')
				lookid[0] = '\0';
			else {
				lookid[0] = '/';
				lookid[2] = '\0';
			}
		} else {
			sprintf(lookid, "/%c/%s", toupper(lookupuser.userid[0]),
					lookupuser.userid);
		}
	}
	sprintf(found, "0Announce/groups/GROUP_0/PersonalCorpus/%s", lookid);
	if (!dashd(found))
		sprintf(found, "0Announce/groups/GROUP_0/PersonalCorpus");
	a_menu("", found, (HAS_PERM(PERM_ANNOUNCE) ? PERM_BOARDS : 0), 0);
	return 1;
}

#ifdef INTERNET_EMAIL
int forward_post (int ent, struct fileheader *fileinfo, char *direct)
{
	if (strcmp ("guest", currentuser.userid) == 0 || digestmode == ATTACH_MODE)
	return DONOTHING;
	return (mail_forward (ent, fileinfo, direct));
}

int forward_u_post (int ent, struct fileheader *fileinfo, char *direct)
{
	if (strcmp ("guest", currentuser.userid) == 0 || digestmode == ATTACH_MODE)
	return DONOTHING;
	return (mail_u_forward (ent, fileinfo, direct));
}
#endif

int read_trash(int ent, struct fileheader *fileinfo, char *direct) {
	extern char currdirect[STRLEN];

	if (!chkBM(currbp, &currentuser)) {
		return DONOTHING;
	}
	digestmode = TRASH_MODE;
	setbdir(currdirect, currboard);
	if (!dashf(currdirect)) {
		digestmode = NA;
		setbdir(currdirect, currboard);
		presskeyfor("版面垃圾箱无内容，按任意键继续...", t_lines-1);
		update_endline();
		return DONOTHING;
	}
	return NEWDIRECT;
}

int read_attach(int ent, struct fileheader *fileinfo, char *direct) {
	extern char currdirect[STRLEN];

	digestmode = ATTACH_MODE;
	sprintf(currdirect, "upload/%s/.DIR", currboard);
	if (!dashf(currdirect)) {
		digestmode = NA;
		setbdir(currdirect, currboard);
		presskeyfor("版面附件区无内容，按任意键继续...", t_lines-1);
		update_endline();
		return DONOTHING;
	}
	return NEWDIRECT;
}

int read_junk(int ent, struct fileheader *fileinfo, char *direct) {
	extern char currdirect[STRLEN];
	if (!HAS_PERM(PERM_OBOARDS)) {
		return DONOTHING;
	}
	digestmode = JUNK_MODE;
	setbdir(currdirect, currboard);
	if (!dashf(currdirect)) {
		digestmode = NA;
		setbdir(currdirect, currboard);
		presskeyfor("站务垃圾箱无内容，按任意键继续...", t_lines-1);
		update_endline();
		return DONOTHING;
	}
	return NEWDIRECT;
}

int show_online(void)
{
	if (currbp->flag & BOARD_ANONY_FLAG) {
		// TODO: prompt at bottom.
		return DONOTHING;
	}
#ifndef FDQUAN
	if (!(currbp->flag & BOARD_CLUB_FLAG) || !(chkBM(currbp, &currentuser)
			|| isclubmember(currentuser.userid, currboard))) {
		return DONOTHING;
	}
#endif
	online_users_show_board();
	return FULLUPDATE;
}

extern int mainreadhelp();
extern int b_vote();
extern int b_results();
extern int b_vote_maintain();
extern int b_notes_edit();
int count_range(int ent, struct fileheader *fileinfo, char *direct);

struct one_key read_comms[] = {
		{'_', underline_post}, {'@', show_online},
#ifdef ENABLE_NOTICE
		{'#', make_notice}, {';', move_notice},
#endif
		{'.', read_trash}, {'J', read_junk}, {',', read_attach},
		{'w', makeDELETEDflag}, {'Y', UndeleteArticle}, {'r', read_post},
		{'K', skip_post}, {'d', del_post}, {'D', del_range},
		{'m', mark_post}, {'E', edit_post}, {Ctrl('G'), acction_mode},
		{'`', acction_mode}, {'g', digest_post}, {'T', edit_title},
		{'s', do_select}, {Ctrl('C'), do_cross}, {Ctrl('P'), do_post},
		{'c', new_flag_clearto}, {'C', count_range},
#ifdef INTERNET_EMAIL
		{'F', forward_post}, {'U', forward_u_post}, {Ctrl ('R'), post_reply},
#endif
		{'i', Save_post}, {'I', Import_post}, {'R', b_results},
		{'v', b_vote}, {'V', b_vote_maintain}, {'W', b_notes_edit},
		{Ctrl('W'), b_notes_passwd}, {'h', mainreadhelp},
		{Ctrl('J'), mainreadhelp}, {KEY_TAB, show_b_note},
		{'z', show_b_secnote}, {'x', into_announce},
		{'a', auth_search_down}, {'A', auth_search_up}, {'/', t_search_down},
		{'?', t_search_up}, {'\'', post_search_down}, {'\"', post_search_up},
		{']', thread_down}, {'[', thread_up}, {Ctrl('D'), deny_user},
		{Ctrl('K'), club_user}, {Ctrl('A'), show_author},
		{Ctrl('N'), SR_first_new}, {'n', SR_first_new}, {'\\', SR_last},
		{'=', SR_first}, {Ctrl('S'), SR_read}, {'p', SR_read},
		{Ctrl('U'), SR_author}, {'b', SR_BMfunc}, {Ctrl('T'), acction_mode},
		{'t', thesis_mode}, {'!', Q_Goodbye}, {'S', s_msg},
		{'f', new_flag_clear}, {'o', online_users_show_override}, {'L', BM_range},
		{'*', show_file_info}, {'Z', send_msg}, {'|', lock}, {'\0', NULL}
};

int board_read() {
	char buf[STRLEN];
	char notename[STRLEN];
	time_t usetime;
	struct stat st;

	if (!selboard) {
		move(2, 0);
		prints("请先选择讨论区\n");
		pressreturn();
		move(2, 0);
		clrtoeol();
		return -1;
	}

	in_mail = NA;

	//bcache_online_num(currboard,1);

	struct boardheader *bp;

	bp = getbcache(currboard);
	//dir can't post. Danielfree 06.2.23
	if (bp->flag & BOARD_DIR_FLAG)
		return FULLUPDATE;
	//add end
	if ((bp->flag & BOARD_CLUB_FLAG) && (bp->flag & BOARD_READ_FLAG)
			&& !chkBM(currbp, &currentuser) && !isclubmember(currentuser.userid,
			currboard)) {
		clear();
		move(5, 10);
		prints("您不是俱乐部版 %s 的成员，无权进入该版", currboard);
		pressanykey();
		return FULLUPDATE;
	}

	brc_initial(currentuser.userid, currboard);
	setbdir(buf, currboard);
	if (uinfo.currbrdnum && brdshm->bstatus[uinfo.currbrdnum - 1].inboard> 0) {
		brdshm->bstatus[uinfo.currbrdnum - 1].inboard--;
	}
	uinfo.currbrdnum = getbnum (currboard, &currentuser);
	update_ulist(&uinfo, utmpent);
	brdshm->bstatus[uinfo.currbrdnum - 1].inboard++;

	setvfile(notename, currboard, "notes");
	if (stat(notename, &st) != -1) {
		if (st.st_mtime < (time(NULL) - 7 * 86400)) {
			utimes(notename, NULL);
			setvfile(genbuf, currboard, "noterec");
			unlink(genbuf);
		}
	}

	if (vote_flag(currboard, '\0', 1 /* 检查读过新的备忘录没 */) == 0) {
		if (dashf(notename)) {
			ansimore(notename, YEA);
			vote_flag(currboard, 'R', 1 /* 写入读过新的备忘录 */);
		}
	}

	usetime = time(0);
	i_read(READING, buf, readtitle, readdoent, &read_comms[0],
			sizeof(struct fileheader));
	//commented by iamfat 2004.03.14
	board_usage(currboard, time(0) - usetime);
	bm_log(currentuser.userid, currboard, BMLOG_STAYTIME, time(0)
			- usetime);
	bm_log(currentuser.userid, currboard, BMLOG_INBOARD, 1);

	if (uinfo.currbrdnum && brdshm->bstatus[uinfo.currbrdnum - 1].inboard> 0) {
		brdshm->bstatus[uinfo.currbrdnum - 1].inboard--;
	}
	uinfo.currbrdnum = 0;
	update_ulist(&uinfo, utmpent);
	bonlinesync (usetime);

	brc_update(currentuser.userid, currboard);
	return 0;
}

/*Add by SmallPig*/
void notepad() {
	char tmpname[STRLEN], note1[4];
	char note[3][STRLEN - 4];
	char tmp[STRLEN];
	FILE *in;
	int i, n;
	time_t thetime = time(0);
	extern int talkrequest;

	clear();
	move(0, 0);
	prints("开始您的留言吧！大家正拭目以待....\n");
	modify_user_mode(WNOTEPAD);
	sprintf(tmpname, "tmp/notepad.%s.%05d", currentuser.userid, uinfo.pid);
	if ((in = fopen(tmpname, "w")) != NULL) {
		for (i = 0; i < 3; i++)
			memset(note[i], 0, STRLEN - 4);
		while (1) {
			for (i = 0; i < 3; i++) {
				getdata(1 + i, 0, ": ", note[i], STRLEN - 5, DOECHO, NA);
				if (note[i][0] == '\0')
					break;
			}
			if (i == 0) {
				fclose(in);
				unlink(tmpname);
				return;
			}
			getdata(5, 0, "是否把您的大作放入留言板 (Y)是的 (N)不要 (E)再编辑 [Y]: ", note1,
					3, DOECHO, YEA);
			if (note1[0] == 'e' || note1[0] == 'E')
				continue;
			else
				break;
		}
		if (note1[0] != 'N' && note1[0] != 'n') {
			sprintf(tmp, "[1;32m%s[37m（%.18s）", currentuser.userid,
					currentuser.username);

			fprintf(
					in,
					"[1;34m▕[44m▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔[36m酸[32m甜[33m苦[31m辣[37m板[34m▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔[44m◣[m\n");
			
			fprintf(
					in,
					"[1;34m▕[32;44m %-44s[32m在 [36m%23.23s[32m 离开时留下的话  [m\n",
					tmp, getdatestring(thetime, DATE_ZH) + 6);
			for (n = 0; n < i; n++) {
				if (note[n][0] == '\0')
					break;
				fprintf(in, "[1;34m▕[33;44m %-75.75s[1;34m[m \n",
						note[n]);
			}
			fprintf(in,
					"[1;34m▕[44m ───────────────────────────────────── [m \n");
			catnotepad(in, "etc/notepad");
			fclose(in);
			f_cp(tmpname, "etc/notepad", O_CREAT);
			unlink(tmpname);
		} else {
			fclose(in);
			unlink(tmpname);
		}
	}
	if (talkrequest) {
		talkreply();
	}
	clear();
	return;
}

/* youzi quick goodbye */
int Q_Goodbye() {
	extern int started;
	char fname[STRLEN];
	int logouts;

	/* added by roly 02.03.21*/
	FILE *sysops;
	int byes, i, choose;
	char buf[STRLEN], spbuf[STRLEN];
	char bye_msgs[9][STRLEN];

	/* add end */

	setuserfile(fname, "msgfile");

#ifdef LOG_MY_MESG
	unlink (fname);
	setuserfile (fname, "msgfile.me");
#endif

	/* edwardc.990423 讯息浏览器 */
	if (dashf(fname)) {
		clear();
		msg_more();
	}
	clear();
	prints("\n\n\n\n");

	/* added by roly 02.03.21 */
	if (!uinfo.invisible) {
		if ((sysops = fopen("etc/friendbye", "r")) != NULL) {
			byes = 0;
			while (byes < 8 && fgets(buf, STRLEN, sysops) != NULL) {
				if (buf[0] != '\0') {
					buf[strlen (buf) - 1] = '\0';
					strcpy(bye_msgs[byes], buf);
					byes++;
				}
			}
			fclose(sysops);
		}
		clear();
		move(0, 0);
		prints("您就要离开 %s 向好友们告个别?\n", BoardName);
		for (i = 0; i < byes; i++)
			prints(" %1d. %s\n", i, bye_msgs[i]);
		prints(" %1d. (其他)\n", byes);
		prints(" %1d. (不通知好友,悄悄离去)\n", byes + 1);
		sprintf(spbuf, "您的选择是 [%1d]:", byes + 1);
		getdata(byes + 3, 0, spbuf, genbuf, 4, DOECHO, YEA);
		choose = atoi(genbuf);
		choose = genbuf[0] - '0';

		if (choose >= 0 && choose <= byes) {
			char buftemp[STRLEN]; //added by roly 02.03.24;

			if (choose >= 0 && choose < byes)
				strlcpy(buftemp, bye_msgs[choose], STRLEN);

			if (choose == byes)
				get_msg("在线好友", buftemp, byes + 4);
			sprintf(buf2, "%s", buftemp); //added by roly 02.03.24                 
			apply_ulist(hisfriend_wall_logout);
		}
		clear();
		prints("\n\n\n\n\n\n\n");
	}

	/* added end */

	setuserfile(fname, "notes");
	if (dashf(fname))
		ansimore(fname, YEA);

	setuserfile(fname, "logout");
	if (dashf(fname)) {
		logouts = countlogouts(fname);
		if (logouts >= 1) {
			user_display(fname, (logouts == 1) ? 1
					: (currentuser.numlogins % (logouts)) + 1, YEA);
		}
	} else {
		if (fill_shmfile(2, "etc/logout", "GOODBYE_SHMKEY"))
			show_goodbyeshm();
	}
	pressreturn();
	report("exit", currentuser.userid);

	CreateNameList();

	if (started) {
		time_t stay;

		stay = time(0) - login_start_time;
		//iamfat added 2004.01.05 to avoid overflow
		currentuser.username[NAMELEN - 1] = 0;
		sprintf(genbuf, "Stay:%3ld (%s)", stay / 60, currentuser.username);
		log_usies("EXIT ", genbuf, &currentuser);
		u_exit();
	}

	if (uinfo.currbrdnum && brdshm->bstatus[uinfo.currbrdnum - 1].inboard> 0) {
		brdshm->bstatus[uinfo.currbrdnum - 1].inboard--;
	}
	uinfo.currbrdnum = 0;
	update_ulist(&uinfo, utmpent);

	sleep(1);
	exit(0);
	return -1;
}

int Goodbye() {
	char sysoplist[20][15], syswork[20][21], buf[STRLEN];
	int i, num_sysop, choose;
	FILE *sysops;
	char *ptr;

	*quote_file = '\0';
	i = 0;
	if ((sysops = fopen("etc/sysops", "r")) != NULL) {
		while (fgets(buf, STRLEN, sysops) != NULL && i <= 19) {
			if (buf[0] == '#')
				continue;
			ptr = strtok(buf, " \n\r\t");
			if (ptr) {
				strlcpy(sysoplist[i], ptr, 14);
				ptr = strtok(NULL, " \n\r\t");
				if (ptr) {
					strlcpy(syswork[i], ptr, 20);
				} else
					strcpy(syswork[i], "[职务不明]");
				i++;
			}
		}
		fclose(sysops);
	}

	num_sysop = i;
	move(1, 0);
	alarm(0);
	clear();
	move(0, 0);
	prints("您就要离开 %s ，可有什麽建议吗？\n", BoardName);
	prints("[[1;33m1[m] 寄信给站长/服务组寻求帮助\n");
	prints("[[1;33m2[m] 按错了啦，我还要玩\n");
#ifdef USE_NOTEPAD
	if (strcmp (currentuser.userid, "guest") != 0) {
		prints ("[[1;33m3[m] 写写[1;32m留[33m言[35m板[m罗\n");
	}
#endif
	//prints("[[1;33m4[m][1;32m 给朋友发个消息 ;)[m\n");
	//commented by roly 02.03.21
	prints("[[1;33m4[m] 不寄罗，要离开啦\n");
	sprintf(buf, "您的选择是 [[1;32m4[m]：");
	getdata(7, 0, buf, genbuf, 4, DOECHO, YEA);
	clear();
	choose = genbuf[0] - '0';
	if (choose == 1) {
		prints("     站长的 ID    负 责 的 职 务\n");
		prints("     ============ =====================\n");
		for (i = 1; i <= num_sysop; i++) {
			prints("[[1;33m%2d[m] %-12s %s\n", i, sysoplist[i - 1],
					syswork[i - 1]);
		}
		prints("[[1;33m%2d[m] 还是走了罗！\n", num_sysop + 1);
		sprintf(buf, "您的选择是 [[1;32m%d[m]：", num_sysop + 1);
		getdata(num_sysop + 5, 0, buf, genbuf, 4, DOECHO, YEA);
		choose = atoi(genbuf);
		if (choose >= 1 && choose <= num_sysop)
			do_send(sysoplist[choose - 1], "使用者寄来的建议信");
		choose = -1;
	}
	if (choose == 2)
		return FULLUPDATE;
	if (strcmp(currentuser.userid, "guest") != 0) {
#ifdef USE_NOTEPAD
		if (HAS_PERM (PERM_POST) && choose == 3)
		notepad ();
#endif
		/* 
		 if( choose == 4)
		 friend_wall();
		 *///commented by roly  02.03.21
	}
	return Q_Goodbye();

}

void do_report(const char *filename, const char *s) {
	char buf[512];
	sprintf(buf, "%-12.12s %16.16s %s\n", currentuser.userid,
			getdatestring(time(NULL), DATE_ZH) + 6, s);
	file_append(filename, buf);
}

void gamelog(char *s) {
	do_report("game/trace", s);
}

/* added by money to provide a method of logging by metalog daemon 2004.01.07 */
#ifdef USE_METALOG
void board_usage (char *mode, time_t usetime)
{
	syslog (LOG_LOCAL5 | LOG_INFO, "USE %-20.20s Stay: %5ld (%s)", mode,
			usetime, currentuser.userid);
}
#else
void board_usage(char *mode, time_t usetime) {
	char buf[256];

	sprintf(buf, "%.22s USE %-20.20s Stay: %5ld (%s)\n",
			getdatestring(time(NULL), DATE_ZH),
			mode, usetime, currentuser.userid);
	file_append("use_board", buf);
}
#endif

int Info() {
	modify_user_mode(XMENU);
	ansimore("Version.Info", YEA);
	clear();
	return 0;
}

int Conditions() {
	modify_user_mode(XMENU);
	ansimore("COPYING", YEA);
	clear();
	return 0;
}

int Welcome() {
	char ans[3];

	modify_user_mode(XMENU);
	if (!dashf("Welcome2"))
		ansimore("Welcome", YEA);
	else {
		clear();
		stand_title("观看进站画面");
		for (;;) {
			getdata(1, 0, "(1)特殊进站公布栏  (2)本站进站画面 ? : ", ans, 2, DOECHO,
					YEA);

			/* skyo.990427 modify  按 Enter 跳出  */
			if (ans[0] == '\0') {
				clear();
				return 0;
			}
			if (ans[0] == '1' || ans[0] == '2')
				break;
		}
		if (ans[0] == '1')
			ansimore("Welcome", YEA);
		else
			ansimore("Welcome2", YEA);
	}
	clear();
	return 0;
}

//      将bname与brec->filename进行大小为sizeof(brec->filename)的比较
int cmpbnames(char *bname, struct fileheader *brec) {
	if (!strncasecmp(bname, brec->filename, sizeof (brec->filename)))
		return 1;
	else
		return 0;
}

void canceltotrash(char *path, char *userid, struct fileheader *fh,
		int subflag, int totrash) {
	char tpath[80];
	char *ptr;

	strcpy(tpath, path);
	ptr = strrchr(tpath, '/');
	if (!ptr)
		return;
	ptr++;
	*ptr = '\0';
	strcat(tpath, fh->filename);
	if (!dashf(tpath))
		return;
	*ptr = '\0';
	strcat(tpath, totrash ? TRASH_DIR : JUNK_DIR);
	fh->timeDeleted = time(0);
	strcpy(fh->szEraser, userid);
	fh->accessed[0] = 0;
	fh->accessed[1] = 0;
	if (subflag == YEA)
		fh->accessed[1] |= FILE_SUBDEL;
	else
		fh->accessed[1] &= ~FILE_SUBDEL;
	append_record(tpath, fh, sizeof(struct fileheader));
}

int thesis_mode() {
	int id; //, i;
	unsigned int pbits;

	//i = 'W' - 'A';
	id = getuser(currentuser.userid);
	pbits = lookupuser.userdefine;
	//pbits ^= (1 << i);
	pbits ^= DEF_THESIS;
	lookupuser.userdefine = pbits;
	currentuser.userdefine = pbits;
	substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	update_ulist(&uinfo, utmpent);
	return FULLUPDATE;
}

/* Add by everlove 07/08/2001 制作合集 */
void Add_Combine(char *board, struct fileheader * fileinfo, int has_cite) //added by roly 02.05.20
{
	FILE *fp;
	char buf[STRLEN];
	char temp2[1024];

	sprintf(buf, "tmp/%s.combine", currentuser.userid);
	fp = fopen(buf, "at");
	fprintf(fp, "[1;32m☆──────────────────────────────────────☆[0;1m\n");
	{
		FILE *fp1;
		char buf[80];
		char *s_ptr, *e_ptr;
		int blankline = 0;

		//modified by iamfat 2003.03.25
		if (in_mail == NA)
			setbfile(buf, board, fileinfo->filename);
		else
			setmfile(buf, currentuser.userid, fileinfo->filename);
		fp1 = fopen(buf, "rt");
		if (fgets(temp2, 256, fp1) != NULL) {
			e_ptr = strchr(temp2, ',');
			if (e_ptr != NULL)
				*e_ptr = '\0';
			s_ptr = &temp2[7];
			fprintf(fp, "    [0;1;32m%s [0;1m于", s_ptr);
		}
		fgets(temp2, 256, fp1);
		if (fgets(temp2, 256, fp1) != NULL) {
			e_ptr = strchr(temp2, ')');
			if (e_ptr != NULL)
				*(e_ptr) = '\0';
			s_ptr = strchr(temp2, '(');
			if (s_ptr == NULL)
				s_ptr = temp2;
			else
				s_ptr++;
			fprintf(fp, " [1;36m%s[0;1m 提到：[0m\n", s_ptr);
		}
		while (!feof(fp1)) {
			fgets(temp2, 256, fp1);
			if ((unsigned) *temp2 < '\x1b') {
				if (blankline)
					continue;
				else
					blankline = 1;
			} else
				blankline = 0;
			//if ((strstr(temp2, "【"))|| + (*temp2==':')) continue;
			if (strstr(temp2, "【"))
				continue;
			if ((has_cite != YEA) && (+(*temp2 == ':')))
				continue;
			//modified by roly 02.05.20 for yinyan
			if (strncmp(temp2, "--", 2) == 0)
				break;
			fputs(temp2, fp);
		}
		fclose(fp1);
	}
	fprintf(fp, "\n");
	fclose(fp);
}

//Added Count Function by IAMFAT 2002.06.14
void combinepath(char *filename, char *tmpfile, char *fname) {
	char *ptr;

	strcpy(tmpfile, filename);
	if ((ptr = strrchr(tmpfile, '/')) != NULL) {
		strcpy(ptr + 1, fname);
	} else {
		strcpy(tmpfile, fname);
	}
}

int _count_range(char *filename, int from, int to, int sortmode,
		int sortopt, int numlevel, char *resultfile) {
	struct fileheader article;
	struct countheader *head, *count, *tail, *prev, *swap;
	int farticle, pos;
	FILE *fresult;
	int countnum;
	char option[STRLEN];

	combinepath(filename, resultfile, ".result");

	if ((farticle = open(filename, O_RDONLY, 0)) == -1)
		return -1;
	pos = 1;
	countnum = 0;
	head = NULL;
	while (read(farticle, &article, sizeof (article)) == sizeof (article)) {
		//added by iamfat 2002.08.10
		/*
		 check_anonymous(article.owner);
		 */
		//added end
		if (pos > to)
			break;
		if (pos >= from) {
			countnum++;
			count = head;
			prev = NULL;
			while (count) {
				if (!strcmp(count->id, article.owner)) {
					count->all_num++;
					if (article.accessed[0] & (FILE_MARKED | FILE_DIGEST)) {
						if (article.accessed[0] & (FILE_MARKED))
							count->m_num++;
						if (article.accessed[0] & (FILE_DIGEST))
							count->g_num++;
					} else if (article.accessed[0] & (FILE_DELETED))
						count->w_num++;
					else
						count->other_num++;
					break;
				}
				prev = count;
				count = count->next;
			}
			if (!count) {
				count = malloc(sizeof(struct countheader));
				memset(count, 0, sizeof(struct countheader));
				strcpy(count->id, article.owner);
				count->all_num = 1;
				if (article.accessed[0] & (FILE_MARKED | FILE_DIGEST)) {
					if (article.accessed[0] & (FILE_MARKED))
						count->m_num = 1;
					if (article.accessed[0] & (FILE_DIGEST))
						count->g_num = 1;
				} else if (article.accessed[0] & (FILE_DELETED))
					count->w_num = 1;
				else
					count->other_num = 1;
				if (prev)
					prev->next = count;
				else
					head = count;
			}
		}
		pos++;
	}
	close(farticle);
	//Bubble Sort
	tail = NULL;
	while (1) {
		count = head;
		prev = NULL;
		while (count && count->next != tail) {
			if ((sortopt == 0 && (sortmode ? (count->all_num
					> count->next->all_num) : (count->all_num
					< count->next->all_num))) || (sortopt == 1
					&& (sortmode ? (count->m_num > count->next->m_num)
							: (count->m_num < count->next->m_num)))
					|| (sortopt == 2 && (sortmode ? (count->g_num
							> count->next->g_num) : (count->g_num
							< count->next->g_num))) || (sortopt == 3
					&& (sortmode ? (count->w_num > count->next->w_num)
							: (count->w_num < count->next->w_num)))
					|| (sortopt == 4 && (sortmode ? (count->other_num
							> count->next->other_num) : (count->other_num
							< count->next->other_num)))) {
				if (prev)
					prev->next = count->next;
				else
					head = count->next;
				swap = count->next->next;
				count->next->next = count;
				count->next = swap;
				if (prev)
					count = prev->next;
				else
					count = head;
			}
			prev = count;
			count = count->next;
		}
		if (count == head)
			break;
		if (count)
			tail = count;
	}

	if ((fresult = fopen(resultfile, "w")) == NULL)
		return -1;
	fprintf(fresult, "版    面: [1;33m%s[m\n", currboard);
	fprintf(fresult, "有效篇数: [1;33m%d[m 篇 [[1;33m%d-%d[m]\n", countnum,
			from, to);
	switch (sortopt) {
		case 1:
			strcpy(option, "按被m的");
			break;
		case 2:
			strcpy(option, "按被g的");
			break;
		case 3:
			strcpy(option, "按被w的");
			break;
		case 4:
			strcpy(option, "按无标记的");
			break;
		default:
			strcpy(option, "按总数");
	}
	fprintf(fresult, "排序方式: [1;33m%s%s[m\n", option, sortmode ? "升序"
			: "降序");
	fprintf(fresult, "文章数下限: [1;33m%d[m\n\n", numlevel);
	//[1;44m
	//[m
	//  fprintf(fresult, "┏━━━━━━┯━━━┯━━━┯━━━┯━━━┯━━━┓\n");
	//  fprintf(fresult, "┃ 使用者代号 │ 总 数│ 被M的│ 被G的│ 被w的│无标记┃\n");
	fprintf(fresult, "[1;44;37m 使用者代号  │总  数│ 被M的│ 被G的│ 被w的│无标记 [m\n");
	//  fprintf(fresult, "┠──────┼───┼───┼───┼───┼───┨\n");
	count = head;
	while (count) {
		if ((sortopt == 0 && count->all_num >= numlevel) || (sortopt == 1
				&& count->m_num >= numlevel) || (sortopt == 2
				&& count->g_num >= numlevel) || (sortopt == 3
				&& count->w_num >= numlevel) || (sortopt == 4
				&& count->other_num >= numlevel))
			//      fprintf (fresult, "┃%-12.12s│%6d│%6d│%6d│%6d│%6d┃\n", count->id,
			fprintf(fresult, " %-12.12s  %6d  %6d  %6d  %6d  %6d \n",
					count->id, count->all_num, count->m_num, count->g_num,
					count->w_num, count->other_num);
		count = count->next;
	}
	fprintf(fresult, "───────────────────────────\n");
	//  fprintf(fresult, "┗━━━━━━┷━━━┷━━━┷━━━┷━━━┷━━━┛\n");
	//  fprintf(fresult, "┗━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n");
	fclose(fresult);

	while (head) {
		count = head->next;
		free(head);
		head = count;
	}

	return 0;
}

int lock(int ent, struct fileheader *fileinfo, char *direct) {
	x_lockscreen();
	return FULLUPDATE;
}

int count_range(int ent, struct fileheader *fileinfo, char *direct) {
	char num[8];
	char resultfile[STRLEN];
	int from, to;
	int sortmode = 0;
	int sortopt = 0;
	int numlevel = 1;
	char title[STRLEN];

	if (uinfo.mode == READING) {
		if (!chkBM(currbp, &currentuser)) {
			return DONOTHING;
		}
	}
	if (digestmode > 1 && digestmode != TRASH_MODE && digestmode
			!= JUNK_MODE)
		return DONOTHING;
	getdata(t_lines - 1, 0, "首篇文章编号: ", num, 6, DOECHO, YEA);
	from = atoi(num);
	if (from <= 0) {
		move(t_lines - 1, 50);
		prints("错误编号...");
		egetch();
		return PARTUPDATE;
	}
	getdata(t_lines - 1, 25, "末篇文章编号: ", num, 6, DOECHO, YEA);
	to = atoi(num);
	if (to < from + 1) {
		move(t_lines - 1, 50);
		prints("错误区间...");
		egetch();
		return PARTUPDATE;
	}

	getdata(t_lines - 1, 0, "排序方式 (0)降序 (1)升序 ? : ", num, 2, DOECHO, YEA);
	sortmode = atoi(num);
	if (sortmode != 0 && sortmode != 1)
		sortmode = 0;

	getdata(t_lines - 1, 0, "排序选项 (0)总数 (1)被m (2)被g (3)被w (4)未标记? : ",
			num, 2, DOECHO, YEA);
	sortopt = atoi(num);
	if (sortopt < 0 || sortopt > 4)sortopt = 0;

	getdata (t_lines - 1, 0, "文章数下限(默认0): ", num, 6, DOECHO, YEA);
	numlevel = atoi (num);
	if (numlevel < 0)
	numlevel = 0;

	move (t_lines - 1, 0);
	clrtoeol ();
	prints ("正在统计...");
	refresh ();

	//开始统计
	if (_count_range (direct, from, to, sortmode, sortopt, numlevel, resultfile)
			== -1) {
		move (t_lines - 1, 0);
		clrtoeol ();
		prints ("统计失败!");
		egetch ();
		return PARTUPDATE;
	}
	//mail to user
	sprintf (title, "[%s]统计文章数(%d-%d)", currboard, from, to);
	mail_file (resultfile, currentuser.userid, title);
	//Postfile(resultfile,currboard, title,2);
	unlink (resultfile);

	securityreport (title, 0, 2);

	return FULLUPDATE;
}

//End IAMFAT
