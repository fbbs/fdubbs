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
 $Id: main.c 367 2007-05-12 17:08:28Z danielfree $
 */
#include "bbs.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif
//modified by money 2002.11.15

#define BADLOGINFILE    "logins.bad"
#define VISITLOG	BBSHOME"/.visitlog"
int ERROR_READ_SYSTEM_FILE = NA;
int RMSG = YEA;
int msg_num = 0;
int count_friends = 0, count_users = 0;
int iscolor = 1;
int mailXX = 0;
char *getenv();
int friend_login_wall();
char *sysconf_str();
struct user_info *t_search();
void r_msg();
void count_msg();
void c_recover();
void tlog_recover();
void refreshdate();
int listmode;
int numofsig = 0;
jmp_buf byebye;
extern struct BCACHE *brdshm;
FILE *ufp;
int talkrequest = NA;

/* int ntalkrequest = NA ; */
int enter_uflags;
time_t lastnote;

struct user_info uinfo;

#ifndef BBSD
char tty_name[20];
#endif
char fromhost[60];

char BoardName[STRLEN];

//char    ULIST[STRLEN];
int utmpent = -1;
time_t login_start_time;
int showansi = 1;
int started = 0;

char GoodWish[20][STRLEN - 3];
int WishNum = 0;
int orderWish = 0;
extern int enabledbchar;

#ifdef ALLOWSWITCHCODE
int convcode = 0; //是否在GB与BIG5间转换?
extern void resolve_GbBig5Files();
#endif

extern struct UCACHE *uidshm;

//added by iamfat 2002.08.10
//int technician=0;

/* added by money to provide a method of logging by metalog daemon 2004.01.07 */
#ifdef USE_METALOG
void log_usies(mode, mesg)
char *mode, *mesg;
{
	char *fmt;

	fmt = currentuser.userid[0] ? "%s %-12s %s" : "%s %s%s";
	syslog(LOG_LOCAL4 | LOG_INFO, fmt, mode, currentuser.userid, mesg);
	return;
}
#else
/* added end */
void log_usies(char *mode, char *mesg) {
	time_t now;
	char buf[256], *fmt;

	now = time(0);
	fmt = currentuser.userid[0] ? "%s %s %-12s %s\n" : "%s %s %s%s\n";
	getdatestring(now, NA);
	sprintf(buf, fmt, datestring, mode, currentuser.userid, mesg);
	file_append("usies", buf);
}
#endif

void u_enter() {
	struct userec tmpuserec;
	FILE *fn;
	int i, j, tmpcount, tmpid, sflag[10][2];
	int lcount = 0;

	enter_uflags = currentuser.flags[0];
	memset(&uinfo, 0, sizeof(uinfo));
	uinfo.active = YEA;
	uinfo.pid = getpid();
	uinfo.currbrdnum = 0;
	/* 使非 SYSOP 权限 ID 登陆时自动恢复到非隐身状态 */
	if (!HAS_PERM(PERM_CLOAK))
		currentuser.flags[0] &= ~CLOAK_FLAG;
	if (HAS_PERM(PERM_LOGINCLOAK) && (currentuser.flags[0] & CLOAK_FLAG))
		uinfo.invisible = YEA;
	uinfo.mode = LOGIN;
	uinfo.pager = 0;
	/*2003.05.18 added by stephen to modify the giveupBBS login user's rights */
	sethomefile(genbuf, currentuser.userid, "giveupBBS");
	fn = fopen(genbuf, "rt");
	if (fn) {
		/*resolve user's data,save in temp struct tmpuserec */
		for (tmpcount = 0; tmpcount < IDLEN + 2; tmpcount++) {
			tmpuserec.userid[tmpcount] = currentuser.userid[tmpcount];
		}
		//tmpid = searchuser(tmpuserec.userid);
		//get_record(PASSFILE,&tmpuserec,sizeof(struct userec),tmpid);
		tmpid = getuserec(tmpuserec.userid, &tmpuserec);
		/*end resolve */

		while (!feof(fn)) {
			if (fscanf(fn, "%d %d", &i, &j) <= 0)
				break;

			sflag[lcount][0] = i;
			sflag[lcount][1] = j;
			lcount++;
		}

		tmpcount = lcount;
		fclose(fn);

		for (i = 0; i < lcount; i++) {
			if (sflag[i][1] <= time(0) / 3600 / 24) {
				tmpcount--;
				switch (sflag[i][0]) {
					case 1:
						tmpuserec.userlevel |= PERM_LOGIN;
						break;
					case 2:
						tmpuserec.userlevel |= PERM_POST;
						break;
					case 3:
						tmpuserec.userlevel |= PERM_TALK;
						break;
					case 4:
						tmpuserec.userlevel |= PERM_MAIL;
						break;
				}
				sflag[i][1] = 0;
			}
		}

		if (tmpuserec.flags[0] & GIVEUPBBS_FLAG && tmpcount == 0)
			tmpuserec.flags[0] &= ~GIVEUPBBS_FLAG;

		substitut_record(PASSFILE, &tmpuserec, sizeof(struct userec),
				tmpid);

		if (tmpcount == 0)
			unlink(genbuf);
		else {
			fn = fopen(genbuf, "wt");
			for (i = 0; i < lcount; i++)
				if (sflag[i][1] > 0)
					fprintf(fn, "%d %d\n", sflag[i][0], sflag[i][1]);
			fclose(fn);
		}

	}
	/*2003.05.18 stephen add end */
#ifdef BBSD
	uinfo.idle_time = time(0);
#endif
	if (DEFINE(DEF_DELDBLCHAR))
		enabledbchar = 1;
	else
		enabledbchar = 0;
	if (DEFINE(DEF_FRIENDCALL)) {
		uinfo.pager |= FRIEND_PAGER;
	}
	if (currentuser.flags[0] & PAGER_FLAG) {
		uinfo.pager |= ALL_PAGER;
		uinfo.pager |= FRIEND_PAGER;
	}
	if (DEFINE(DEF_FRIENDMSG)) {
		uinfo.pager |= FRIENDMSG_PAGER;
	}
	if (DEFINE(DEF_ALLMSG)) {
		uinfo.pager |= ALLMSG_PAGER;
		uinfo.pager |= FRIENDMSG_PAGER;
	}
	/* Following 3 lines added by Amigo 2002.04.03. For close logoff msg. */
	if (DEFINE(DEF_LOGOFFMSG)) {
		uinfo.pager |= LOGOFFMSG_PAGER;
	}
	uinfo.uid = usernum;
	strncpy(uinfo.from, fromhost, 60);
	if (!DEFINE(DEF_NOTHIDEIP)) {
		uinfo.from[22] = 'H';
	}
#if !defined(BBSD) && defined(SHOW_IDLE_TIME)
	strncpy(uinfo.tty, tty_name, 20);
#endif
	iscolor = (DEFINE(DEF_COLOR)) ? 1 : 0;
	strncpy(uinfo.userid, currentuser.userid, 20);
	strncpy(uinfo.realname, currentuser.realname, 20);
	strncpy(uinfo.username, currentuser.username, NAMELEN);
	getfriendstr();
	getrejectstr();

	listmode = 0; /* 借用一下, 用来纪录到底 utmpent 卡位失败几次 */
	while (1) {
		utmpent = getnewutmpent(&uinfo);
		if (utmpent >= 0 || utmpent == -1)
			break;
		if (utmpent == -2 && listmode <= 100) {
			listmode++;
			DEBUG(usleep(250)); /* 休息四分之一秒再接在励 */
			continue;
		}
		if (listmode > 100) { /* 放弃吧 */
			sprintf(genbuf, "getnewutmpent(): too much times, give up.");
			report(genbuf);
			prints("getnewutmpent(): 失败太多次, 放弃. 请回报站长.\n");
			sleep(3);
			exit(0);
		}
	}
	if (utmpent < 0) {
		sprintf(genbuf, "Fault: No utmpent slot for %s\n", uinfo.userid);
		report(genbuf);
	}
	listmode = 0;
	digestmode = NA;
}

void setflags(mask, value)
int mask, value;
{
	if (((currentuser.flags[0] & mask) && 1) != value) {
		if (value)
		currentuser.flags[0] |= mask;
		else
		currentuser.flags[0] &= ~mask;
	}
}

void u_exit() { /*这些信号的处理要关掉, 否则在离线时等候回车时出现  (ylsdd)
 信号会导致重写名单, 这个导致的名单混乱比kick user更多 */
	signal(SIGHUP, SIG_DFL);
	signal(SIGALRM, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);

	setflags(PAGER_FLAG, (uinfo.pager & ALL_PAGER));
	if (HAS_PERM(PERM_LOGINCLOAK))
		setflags(CLOAK_FLAG, uinfo.invisible);

	if (!ERROR_READ_SYSTEM_FILE) {
		time_t recent;
		time_t stay;
		time_t now;

		//stay=(stay>7200)?7200/*(14400-stay)*/:stay; //added by iamfat 2002.08.20 超过两个小时 上站时间递减
		set_safe_record();
		now = time(0);
		recent = login_start_time;
		if (currentuser.lastlogout > recent)
			recent = currentuser.lastlogout;
		if (currentuser.lastlogin > recent)
			recent = currentuser.lastlogin;
		//stay=now - ((currentuser.lastlogout>login_start_time)?currentuser.lastlogout:login_start_time);
		stay = now - recent;
		if (stay < 0)
			stay = 0;
		currentuser.lastlogout = now;
		currentuser.stay += stay;
		substitut_record(PASSFILE, &currentuser, sizeof(currentuser),
				usernum);
		uidshm->status[usernum - 1]--;
	}

	uinfo.invisible = YEA;
	uinfo.sockactive = NA;
	uinfo.sockaddr = 0;
	uinfo.destuid = 0;
#if !defined(BBSD) && defined(SHOW_IDLE_TIME)
	strcpy(uinfo.tty, "NoTTY");
#endif
	uinfo.pid = 0;
	uinfo.active = NA;
	update_utmp();
}

int cmpuids(uid, up)
char *uid;
struct userec *up;
{
	return !strncasecmp(uid, up->userid, sizeof(up->userid));
}

int dosearchuser(userid)
char *userid;
{
	int id;

	if ((id = getuser(userid)) != 0) {
		if (cmpuids(userid, &lookupuser)) {
			memcpy(&currentuser, &lookupuser, sizeof(currentuser));
			return usernum = id;
		}
	}
	memset(&currentuser, 0, sizeof(currentuser));
	return usernum = 0;
}

void talk_request() {
	signal(SIGUSR1, talk_request);
	talkrequest = YEA;
	bell();
	bell();
	bell();
	sleep(1);
	bell();
	bell();
	bell();
	bell();
	bell();
	return;
}

void abort_bbs() {
	extern int child_pid;

	if (child_pid) {
		kill(child_pid, 9);
	}
	if (uinfo.mode == POSTING || uinfo.mode == SMAIL || uinfo.mode == EDIT
			|| uinfo.mode == EDITUFILE || uinfo.mode == EDITSFILE
			|| uinfo.mode == EDITANN)
		keep_fail_post();
	if (started) {
		time_t stay;

		stay = time(0) - login_start_time;
		currentuser.username[NAMELEN - 1] = 0; //added by iamfat 2004.01.05 to avoid overflow
		sprintf(genbuf, "Stay: %3ld (%s)", stay / 60, currentuser.username);
		log_usies("AXXED", genbuf);
		u_exit();
	}
	/*
	 #ifdef DMALLOC
	 dmalloc_shutdown();
	 #endif*/
	exit(0);
}

int cmpuids2(unum, urec)
int unum;
struct user_info *urec;
{
	return (unum == urec->uid);
}

int count_multi(uentp)
struct user_info *uentp;
{
	static int count;

	if (uentp == NULL) {
		int num = count;

		count = 0;
		return num;
	}
	if (!uentp->active || !uentp->pid)
	return 0;
	if (uentp->uid == usernum)
	count++;
	return 1;
}

int count_user() {
	count_multi(NULL);
	apply_ulist(count_multi);
	return count_multi(NULL);
}

#ifdef IPMAXLOGINS
int _cnt_ip(uentp)
struct user_info *uentp;
{
	static int count;

	if (uentp == NULL) {
		int num = count;

		count = 0;
		return num;
	}
	if (!uentp->active || !uentp->pid)
	return 0;
	if (!strcmp(uentp->userid, "guest"))
	return 0;
	if (!strcmp(uentp->from, fromhost))
	count++;
	return 1;
}

int count_ip()
{
	_cnt_ip(NULL);
	apply_ulist(_cnt_ip);
	return _cnt_ip(NULL);
}

void iplogins_check()
{
	int sameip;

	if (currentuser.userid && !strcmp(currentuser.userid, "guest"))
	return;
	if (!IsSpecial(fromhost, "etc/restrictip")
			|| IsSpecial(fromhost, "etc/freeip")) {
		sameip = 0;
	} else {
		sameip = count_ip();
	}
	if (sameip >= IPMAXLOGINS) {
		prints("[1;32m为确保他人上站权益, 本站仅允许此IP同时登陆 %d 个。\n[0m",
				IPMAXLOGINS);
		prints("[1;36m您目前已经使用该IP登陆了 %d 个！\n[0m", sameip);
		oflush();
		sleep(3);
		exit(1);
	}

}

#endif

void multi_user_check() {
	struct user_info uin;
	int logins, mustkick = 0;

	if (HAS_PERM(PERM_MULTILOG))
		return; /* don't check sysops */

	/* allow multiple guest user */
	logins = count_user();

	if (heavyload() && logins) {
		prints("[1;33m抱歉, 目前系统负荷过重, 请勿重复 Login。[m\n");
		oflush();
		sleep(3);
		exit(1);
	}

	if (!strcasecmp("guest", currentuser.userid)) {
		if (logins > MAXGUEST) {
			prints("[1;33m抱歉, 目前已有太多 [1;36mguest[33m, 请稍后再试。[m\n");
			oflush();
			sleep(3);
			exit(1);
		}
		return;
	} else if ((!HAS_PERM(PERM_SPECIAL0) && logins >= MULTI_LOGINS)
			|| logins > 5) {
		prints("[1;32m为确保他人上站权益, 本站仅允许您用该帐号登陆 %d 个。\n[0m", MULTI_LOGINS);
		prints("[1;36m您目前已经使用该帐号登陆了 %d 个，您必须断开其他的连接方能进入本站！\n[0m", logins);
		mustkick = 1;
	}
	//commented by iamfat 2002.10.27
	//if(logins /*&& num_active_users() >= (MAXACTIVE*2/3)*/ && !HAS_PERM(PERM_BOARDS))
	/*
	 if(logins && !HAS_PERM(PERM_BOARDS))
	 {
	 prints("[1;33m抱歉, 目前系统负荷过重, 请勿重复 Login。[m\n");
	 mustkick = 1;
	 } */

	if (search_ulist(&uin, cmpuids2, usernum) && (uin.active || (uin.pid
			&& kill(uin.pid, 0) == -1))) {
		getdata(0, 0, "[1;37m您想删除重复的 login 吗 (Y/N)? [N][m", genbuf, 4,
				DOECHO, YEA);

		if (genbuf[0] == 'N' || genbuf[0] == 'n' || genbuf[0] == '\0') {
			if (mustkick) {
				prints("[33m很抱歉，您已经用该帐号登陆 %d 个，所以，此连线将被取消。[m\n", logins);
				oflush();
				sleep(3);
				exit(1);
			}
		} else {
			if (!uin.pid)
				return;
			kill(uin.pid, SIGHUP);
			//以前不是SIGHUP，会导致编辑作业丢失 by sunner
			report("kicked (multi-login)");
			currentuser.username[NAMELEN - 1] = 0; //added by iamfat 2004.01.05 to avoid overflow
			log_usies("KICK ", currentuser.username);
		}
	}
#ifdef IPMAXLOGINS
	iplogins_check();
#endif

}

#ifndef BBSD
void system_init(argc, argv)
int argc;
char **argv;
#else
void system_init()
#endif
{
#ifndef BBSD
	char *rhost;
#endif
	struct sigaction act;

	//gethostname(genbuf, 256);
	//sprintf(ULIST, "%s.%s", ULIST_BASE, genbuf);

#ifndef BBSD
	if (argc >= 3) {
		strncpy(fromhost, argv[2], 60);
	} else {
		fromhost[0] = '\0';
	}
	if ((rhost = getenv("REMOTEHOST")) != NULL)
	strncpy(fromhost, rhost, 60);
	fromhost[59] = '\0';
#if defined(SHOW_IDLE_TIME)
	if (argc >= 4) {
		strncpy(tty_name, argv[3], 20);
	} else {
		tty_name[0] = '\0';
	}
#endif
#endif

#ifndef lint
	signal(SIGHUP, abort_bbs);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
#ifdef DOTIMEOUT
	init_alarm();
	uinfo.mode = LOGIN;
	alarm(LOGIN_TIMEOUT);
#else
	signal(SIGALRM, SIG_SIG);
#endif
	signal(SIGTERM, SIG_IGN);
	signal(SIGURG, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
#endif
	signal(SIGUSR1, talk_request);

	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NODEFER;
	act.sa_handler = r_msg;
	sigaction(SIGUSR2, &act, NULL);
	{
		struct itimerval itv;
		memset(&itv,0, sizeof(struct itimerval));
		itv.it_value.tv_sec = 2 * 60;
		setitimer(ITIMER_PROF, &itv, NULL);
		signal(SIGPROF, exit);
	}

}

void system_abort() {
	if (started) {
		currentuser.username[NAMELEN - 1] = 0; //added by iamfat 2004.01.05 to avoid overflow
		log_usies("ABORT", currentuser.username);
		u_exit();
	}
	clear();
	refresh();
	prints("谢谢光临, 记得常来喔 !\n");
	exit(0);
}

void logattempt(uid, frm)
char *uid, *frm;
{
	char fname[STRLEN];

	getdatestring(time(0), NA);
	sprintf(genbuf, "%-12.12s  %-30s %s\n", uid, datestring, frm);
	file_append(BADLOGINFILE, genbuf);
	sethomefile(fname, uid, BADLOGINFILE);
	file_append(fname, genbuf);

}

int check_tty_lines() { /* dii.nju.edu.cn  zhch  2000.4.11 */
	static unsigned char buf1[] = { 255, 253, 31 };
	unsigned char buf2[100];
	int n;

	if (ttyname(0))
		return;
	write(0, buf1, 3);
	n = read(0, buf2, 80);
	if (n == 12) {
		if (buf2[0] != 255 || buf2[1] != 251 || buf2[2] != 31)
			return;
		if (buf2[3] != 255 || buf2[4] != 250 || buf2[5] != 31 || buf2[10]
				!= 255 || buf2[11] != 240)
			return;
		t_lines = buf2[9];
	}
	if (n == 9) {
		if (buf2[0] != 255 || buf2[1] != 250 || buf2[2] != 31 || buf2[7]
				!= 255 || buf2[8] != 240)
			return;
		t_lines = buf2[6];
	}
	if (t_lines < 24 || t_lines > 100)t_lines = 24;
}

struct max_log_record {
	int year;
	int month;
	int day;
	int logins;
	unsigned long visit;
} max_log;

void visitlog(void) {
	time_t now;
	struct tm *tm;
	extern struct UTMPFILE *utmpshm;

	//modified by iamfat to avoid deadlock
	FILE *fp;

	fp = fopen(VISITLOG, "r+b");
	if (fp) {
		if (!fread(&max_log, sizeof(max_log), 1, fp)
				|| (max_log.year < 1990 || max_log.year> 2020)){
		now = time(0);
		tm = localtime(&now);
		max_log.year = tm->tm_year + 1900;
		max_log.month = tm->tm_mon + 1;
		max_log.day = tm->tm_mday;
		max_log.visit = 0;
		max_log.logins = 0;
	} else {
		max_log.visit++;
		if (max_log.logins> utmpshm->max_login_num) {
			utmpshm->max_login_num = max_log.logins;
		} else {
			max_log.logins = utmpshm->max_login_num;
		}
	}
	fseek(fp, 0, SEEK_SET);
	fwrite(&max_log, sizeof(max_log), 1, fp);
	fclose(fp);
}
/*
 int vfp;
 vfp = open(VISITLOG,O_RDWR|O_CREAT,0644);
 if(vfp == -1) {
 report("Can NOT write visit Log to .visitlog");
 return ;
 }
 flock(vfp, LOCK_EX);
 lseek(vfp,(off_t)0,SEEK_SET);
 read(vfp, &max_log,(size_t)sizeof(max_log)); 
 if(max_log.year < 1990 || max_log.year > 2020) {
 now = time(0);
 tm = localtime(&now); 
 max_log.year = tm->tm_year+1900;
 max_log.month = tm->tm_mon+1;
 max_log.day = tm->tm_mday;
 max_log.visit = 0;
 max_log.logins = 0;
 }
 max_log.visit ++ ;
 if( max_log.logins > utmpshm->max_login_num )
 utmpshm->max_login_num = max_log.logins;
 else
 max_log.logins =  utmpshm->max_login_num;
 lseek(vfp,(off_t)0,SEEK_SET);
 write(vfp,&max_log,(size_t)sizeof(max_log));
 flock(vfp, LOCK_UN);
 close(vfp); 
 */
sprintf(genbuf,
		"[1;32m从 [[36m%4d年%2d月%2d日[32m] 起, 最高人数记录: [[36m%d[32m] 累计访问人次: [[36m%u[32m][m\n",
		max_log.year, max_log.month, max_log.day, max_log.logins,
		max_log.visit);
prints(genbuf);
}

void login_query() {
	char uid[IDLEN + 2];
	char passbuf[PASSLEN];
	int curr_login_num;
	int attempts;
	char genbuf[STRLEN];
	FILE *fn;
	char *ptr;
	extern struct UTMPFILE *utmpshm;
	int i, j, tmpid, tmpcount, sflag[10][2]; /*2003.04.22 added by stephen */
	struct userec tmpuserec; /*2003.05.02 added by stephen */
	int lcount = 0, basici = 0, basicj = 0;

	curr_login_num = num_active_users();
	if (curr_login_num >= MAXACTIVE) {
		ansimore("etc/loginfull", NA);
		oflush();
		sleep(1);
		exit(1);
	}
#ifdef BBSNAME
	strcpy(BoardName, BBSNAME);
#else
	ptr = sysconf_str("BBSNAME");
	if (ptr == NULL)
		ptr = "尚未命名测试站";
	strcpy(BoardName, ptr);
#endif
	if (fill_shmfile(1, "etc/issue", "ISSUE_SHMKEY")) {
		show_issue(); /* is anibanner ready, remark this and put * \n\n */
	}
	prints(
			"[1;35m欢迎光临[1;40;33m【 %s 】 [m[[0;1;33;41m Add '.' after YourID to login for BIG5 [m]\n",
			BoardName);
	resolve_utmp();
	if (utmpshm->usersum == 0)
		utmpshm->usersum = allusers();
	utmpshm->total_num = curr_login_num;//+ get_anon();
	if (utmpshm->max_login_num < utmpshm->total_num)
		utmpshm->max_login_num = utmpshm->total_num;
	prints(
			"[1;32m目前已有帐号数: [[1;36m%d[32m/[36m%d[32m] [32m目前上站人数: [[36m%d[32m/[36m%d[1;32m]\n", // 其中WEB匿名: [[36m%d[32m]\n",
			utmpshm->usersum, MAXUSERS, utmpshm->total_num, 10000); // get_anon());
	//    utmpshm->usersum, curr_login_num-CountCloakMan(), 10000);
	visitlog();

#ifdef MUDCHECK_BEFORELOGIN
	prints("[1;33m为防止使用程式上站，请按 [1;36mCTRL + C[m : ");
	genbuf[0] = igetkey();
	if (genbuf[0] != Ctrl('C')) {
		prints("\n对不起，您并没有按下 CTRL+C 键！\n");
		oflush();
		exit(1);
	} else {
		prints("[CTRL] + [C]\n");
	}
#endif

	attempts = 0;
	while (1) {
		if (attempts++ >= LOGINATTEMPTS) {
			ansimore("etc/goodbye", NA);
			oflush();
			sleep(1);
			exit(1);
		}
#ifndef LOADTEST
		getdata(
				0,
				0,
				"[1;33m请输入帐号[m(试用请输入'[1;36mguest[m', 注册请输入'[1;31mnew[m'): ",
				uid, IDLEN + 1, DOECHO, YEA);
#else //LOADTEST
		strcpy(uid, "guest");
#endif //LOADTEST
#ifdef ALLOWSWITCHCODE
		ptr = strchr(uid, '.');
		if (ptr) {
			convcode = 1;
			*ptr = '\0';
		}
#endif
		if ((strcasecmp(uid, "guest") == 0) && (MAXACTIVE - curr_login_num
				< 10)) {
			ansimore("etc/loginfull", NA);
			oflush();
			sleep(1);
			exit(1);
		}
		if (strcasecmp(uid, "new") == 0) {
#ifdef LOGINASNEW
			memset(&currentuser, 0, sizeof(currentuser));
			new_register();
			ansimore3("etc/firstlogin", YEA);
			break;
#else
			prints("[1;37m本系统目前无法以 [36mnew[37m 注册, 请用[36m guest[37m 进入...[m\n");
#endif
		} else if (*uid == '\0')
			;
		else if (!dosearchuser(uid)) {
			prints("[1;31m经查证，无此 ID。[m\n");
		} else if (strcasecmp(uid, "guest") == 0) {
			currentuser.userlevel = 0;
			break;
#ifdef SYSOPLOGINPROTECT
		} else if (!strcasecmp(uid, "SYSOP") && strcmp(fromhost, "localhost")
				&& strcmp(fromhost, "127.0.0.1")) {
			prints("[1;32m 警告: 从 %s 登录是非法的! 请勿再试![m\n", fromhost);
			prints
			("[注意] 为安全起见，本站已经设定 SYSOP 只能从主机登陆。\n       如果您确实是本站的 SYSOP ，请登陆到本 BBS 服务器，然后: \n              telnet localhost port.\n");
			oflush();
			sleep(1);
			exit(1);
#endif
		} else {
#ifdef ALLOWSWITCHCODE
			if (!convcode)
			convcode = !(currentuser.userdefine & DEF_USEGB);
#endif
			getdata(0, 0, "[1;37m请输入密码: [m", passbuf, PASSLEN, NOECHO,
					YEA);
			passbuf[8] = '\0';
			if (!checkpasswd(currentuser.passwd, passbuf)) {
				logattempt(currentuser.userid, fromhost);
				prints("[1;31m密码输入错误...[m\n");
			} else {
				/*2003.04.22 added by stephen to  add giveup bbs user  login info */
				if (strcasecmp(currentuser.userid, "guest")
						&& !HAS_PERM(PERM_LOGIN)) {
					sethomefile(genbuf, currentuser.userid, "giveupBBS");
					fn = fopen(genbuf, "rt");
					if (fn) {
						/*2003.05.02 added by stphen to resolve user's data,save in temp struct tmpuserec */
						for (tmpcount = 0; tmpcount < IDLEN + 2; tmpcount++) {
							tmpuserec.userid[tmpcount]
									= currentuser.userid[tmpcount];
						}
						//tmpid = searchuser(tmpuserec.userid);
						//get_record(PASSFILE,&tmpuserec,sizeof(struct userec),tmpid);
						tmpid = getuserec(tmpuserec.userid, &tmpuserec);
						/*2003.05.02 add end */

						while (!feof(fn)) {
							if (fscanf(fn, "%d %d", &i, &j) <= 0)
								break;

							sflag[lcount][0] = i;
							sflag[lcount][1] = j;
							lcount++;
						}

						tmpcount = lcount;
						fclose(fn);
						/*2003.05.02 added by stephen start check giveupBBS user's giveup-datafile */
						/*and modify the user's perm ,use tmpuserec as a temp userec struct */
						for (i = 0; i < lcount; i++) {
							if (sflag[i][1] <= time(0) / 3600 / 24) {
								tmpcount--;
								switch (sflag[i][0]) {
									case 1:
										tmpuserec.userlevel |= PERM_LOGIN;
										sflag[i][0] = 0;
										break;
									case 2:
										tmpuserec.userlevel |= PERM_POST;
										break;
									case 3:
										tmpuserec.userlevel |= PERM_TALK;
										break;
									case 4:
										tmpuserec.userlevel |= PERM_MAIL;
										break;
								}
								sflag[i][1] = 0;
							}

							if (sflag[i][0] == 1) {
								basici = i;
								basicj = j;
							}
						}

						/*2003.05.02 add end */
						if (tmpuserec.flags[0] & GIVEUPBBS_FLAG
								&& tmpcount == 0)
							tmpuserec.flags[0] &= ~GIVEUPBBS_FLAG;
						substitut_record(PASSFILE, &tmpuserec,
								sizeof(struct userec), tmpid);

						if (tmpcount == 0)
							unlink(genbuf);
						else {
							fn = fopen(genbuf, "wt");
							for (i = 0; i < lcount; i++)
								if (sflag[i][1] > 0)
									fprintf(fn, "%d %d\n", sflag[i][0],
											sflag[i][1]);
							fclose(fn);
						}
						if (sflag[basici][0] == 1) {
							sprintf(genbuf, "[33m您正在戒网，离戒网结束还有%d天[m\n",
									basicj - time(0) / 3600 / 24);
							prints(genbuf);
							oflush();
							pressanykey();
							sleep(1);

							exit(1);
						}

					}

					//Don't allow revival, Added by Ashinmarch Sep.04,2008
					if (currentuser.userlevel == 0) {
						/*     && askyn("您知道您已经自杀了吗？", NA, NA) == YEA
						 && askyn("您想起死回生吗？", NA, NA) == YEA
						 && askyn("您按错键了吗？", YEA, NA) == NA
						 && askyn("您同意您的上站次数、文章数及上站总时数清零吗？", NA,
						 NA) == YEA && askyn("我们很烦吗？", YEA, NA) == YEA) {
						 currentuser.userlevel = PERM_DEFAULT;

						 currentuser.numposts = 0;
						 currentuser.numlogins = 1;
						 #ifdef FDQUAN
						 #else
						 currentuser.stay = 0;
						 #endif
						 substitut_record(PASSFILE, &currentuser, sizeof(currentuser),
						 usernum);
						 */
						prints("[32m您已经自杀[m\n");
						pressanykey();
						oflush();
						sleep(1);
						exit(1);
					} else {
						prints("[32m本帐号已停机。请向 [36msysops[32m 查询原因[m\n");
						pressanykey();
						oflush();
						sleep(1);
						exit(1);
					}
				}

				/*2003.04.22 stephen add end*/
#ifdef CHECK_FREQUENTLOGIN
				if (!HAS_PERM(PERM_SYSOPS)
                        && strcasecmp(currentuser.userid, "guest") != 0
                        && abs(time(0) - currentuser.lastlogin) < 10) {
                    prints("登录过于频繁，请稍候再来\n");
                    report("Too Frequent");
                    oflush();
                    sleep(3);
                    exit(1);
				}
#endif

#ifdef CHECK_SYSTEM_PASS
				if (HAS_PERM(PERM_SYSOPS)) {
					if (!check_systempasswd()) {
						prints("\n口令错误, 不得进入 ! !\n");
						oflush();
						sleep(2);
						exit(1);
					}
				}
#endif
				memset(passbuf, 0, PASSLEN - 1);
				break;
			}
		}
	}
	multi_user_check();

	if (!term_init(currentuser.termtype)) {
		prints("Bad terminal type.  Defaulting to 'vt100'\n");
		strcpy(currentuser.termtype, "vt100");
		term_init(currentuser.termtype);
	}

	check_tty_lines(); /* 2000.03.14 */
	sethomepath(genbuf, currentuser.userid);
	mkdir(genbuf, 0755);
	login_start_time = time(0);
}

int valid_ident(ident)
char *ident;
{
	static char *invalid[] = {"unknown@", "root@", "gopher@", "bbs@",
		"guest@", "nobody@", "www@", NULL
	};
	int i;

	if (ident[0] == '@')
	return 0;
	for (i = 0; invalid[i] != NULL; i++)
	if (strstr(ident, invalid[i]) != NULL)
	return 0;
	return 1;
}

void write_defnotepad() {
	currentuser.notedate = time(0);
	set_safe_record();
	substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	return;
}

void notepad_init() {
	FILE *check;
	char notetitle[STRLEN];
	char tmp[STRLEN * 2];
	char *fname, *bname, *ntitle;
	long int maxsec;
	time_t now;

	maxsec = 86400;
	lastnote = 0;
	if ((check = fopen("etc/checknotepad", "r")) != NULL) {
		fgets(tmp, sizeof(tmp), check);
		lastnote = atol(tmp);
		fclose(check);
	}
	now = time(0);
	if ((now - lastnote) >= maxsec) {
		move(t_lines - 1, 0);
		prints("对不起，系统自动发信，请稍候.....");
		refresh();
		check = fopen("etc/checknotepad", "w");
		lastnote = now - (now % maxsec);
		fprintf(check, "%d", lastnote);
		fclose(check);
		if ((check = fopen("etc/autopost", "r")) != NULL) {
			while (fgets(tmp, STRLEN, check) != NULL) {
				fname = strtok(tmp, " \n\t:@");
				bname = strtok(NULL, " \n\t:@");
				ntitle = strtok(NULL, " \n\t:@");
				if (fname == NULL || bname == NULL || ntitle == NULL)
					continue;
				else {
					getdatestring(now, NA);
					sprintf(notetitle, "[%8.8s %6.6s] %s", datestring + 6,
							datestring + 23, ntitle);
					if (dashf(fname)) {
						Postfile(fname, bname, notetitle, 1);
						sprintf(tmp, "%s 自动张贴", ntitle);
						report(tmp);
					}
				}
			}
			fclose(check);
		}
		getdatestring(now, NA);
		sprintf(notetitle, "[%s] 留言板记录", datestring);
		if (dashf("etc/notepad")) {
			Postfile("etc/notepad", "Notepad", notetitle, 1);
			unlink("etc/notepad");
		}
		report("自动发信时间更改");
	}
	return;
}

//函数原型: int IsSpecial(const char* str, const char* filename)
//函数功能: 根据id通过对etc/technicians里面的匹配 给出该id是否是技术站长
//added by iamfat 2003.08.10
int IsSpecial(const char *str, const char *filename) {
	FILE *fp;
	char line[STRLEN];
	char *ptr;
	int i = 0;

	if (fp = fopen(filename, "r")) {
		while (fgets(line, STRLEN, fp)) {
			ptr = strtok(line, " \r\n\t");
			if (!ptr[0] || ptr[0] == '#')
				continue;
			else if (!strncmp(str, ptr, strlen(ptr))) {
				i = 1;
				break;
			}
		}
		fclose(fp);
	}
	return i;
}

//函数原型: void SpecialID(const char* uid, char* host)
//函数功能: 根据id通过对etc/special.ini里面的匹配 给出该id相应的特殊登陆host
//added by iamfat 2002.07.30
//参数介绍:
//(IN)uid:用户的id
//(IN/OUT)host:传递过来用于修改的用户的host指针
void SpecialID(const char *uid, char *host) {
	FILE *fp;
	char line[STRLEN];
	char *special;

	//char *lasts;
	if (fp = fopen("etc/special.ini", "r")) {
		while (fgets(line, STRLEN, fp)) {
			//special=strtok_r(line," \r\n\t", &lasts);
			special = strtok(line, " \r\n\t");
			if (special && !strcasecmp(uid, special)) {
				//special=strtok_r(lasts," \r\n\t", &lasts);
				special = strtok(NULL, " \r\n\t");
				if (special)
					strcpy(host, special);
				break;
			}
		}
		fclose(fp);
	}
}

/*  02.10.05  add by stephen to convert string "a.b.c.d" to string "a.b.*.*" 
 function prototype: void MaskLoginIP(const char * uid, char * host)
 (*host) may be changed                                             */
/*void
 MaskLoginIP(const char * uid, char * host)
 {
 int counter,i;
 char  tempHostString[16];

 if( *host == NULL )
 {
 return ;
 //do nothing
 }

 for(i=0,counter=0;i <12 && counter <2;i++)
 {
 if( *(host + i) == NULL )
 {
 break;
 }
 else if( *(host + i) == '.')
 {
 counter ++;
 tempHostString[i] = * (host + i);
 }
 else
 {
 tempHostString[i] = * (host + i);
 }

 }

 if(i > 0)
 {
 tempHostString[i++] = '*';
 tempHostString[i++] = '.';
 tempHostString[i++] = '*';
 tempHostString[i] = NULL;
 }
 
 strcpy(host,tempHostString);
 
 }*/
/*02.10.05  add end*/

void user_login() {
	char fname[STRLEN];
	char *ruser;
	int logins;

	if (strcmp(currentuser.userid, "SYSOP") == 0) {
		currentuser.userlevel = ~0; /* SYSOP gets all permission bits */
		substitut_record(PASSFILE, &currentuser, sizeof(currentuser),
				usernum);
	}
	ruser = getenv("REMOTEUSER");
	fromhost[59] = 0; //added by iamfat 2004.01.05 to avoid overflow
	sprintf(genbuf, "%s@%s", ruser ? ruser : "?", fromhost);
	log_usies("ENTER", genbuf);

	/*02.10.05  add by stephen to mask the real ip of user,convert "a.b.c.d" to "a.b.*.*"  */
	//02.10.09 Don't add this line now.
	// And if you really want to try this, please add #ifdef FDQUAN befor it to make sure this function will only be compiled in FDQUAN
	//      #ifdef  FDQUAN
	//      MaskLoginIP(currentuser.userid,fromhost);
	//      #endif
	/*02.10.05  add end*/

	//这是个给特殊ID设置隐藏HOST的函数
	SpecialID(currentuser.userid, fromhost);
	//technician=IsTechnician(currentuser.userid);

	u_enter();
	if (ruser != NULL) {
		sprintf(genbuf, "%s@%s", ruser, fromhost);
		if (valid_ident(genbuf)) {
			strncpy(currentuser.ident, genbuf, NAMELEN);
			currentuser.ident[NAMELEN - 1] = '\0';
		}
	}
	report("Enter");
	started = 1;
	logins = count_user();

	if (! (HAS_PERM(PERM_MULTILOG) || (HAS_PERM(PERM_SPECIAL0) && logins
			< 5) || (logins <= MULTI_LOGINS)) && strcmp(
			currentuser.userid, "guest")) {
		report("kicked (multi-login)[漏网之鱼]");
		abort_bbs();
	}
	initscr();
#ifdef USE_NOTEPAD
	notepad_init();
	if (strcmp(currentuser.userid, "guest") != 0) {
		if (DEFINE(DEF_NOTEPAD)) {
			int noteln;

			if (lastnote> currentuser.notedate)
			currentuser.noteline = 0;
			noteln = countln("etc/notepad");
			if (currentuser.noteline == 0) {
				shownotepad();
			} else if ((noteln - currentuser.noteline)> 0) {
				move(0, 0);
				ansimore2("etc/notepad", NA, 0, noteln - currentuser.noteline + 1);
				igetkey();
				clear();
			}
			currentuser.noteline = noteln;
			write_defnotepad();
		}
	}
#endif
	/*
	 if(show_statshm("0Announce/bbslist/countusr", 0) && DEFINE(DEF_GRAPH)) {
	 refresh();
	 pressanykey();
	 }
	 */
	if (show_statshm("etc/hotspot", 0)) {
		refresh();
		pressanykey();
	}

	if ((vote_flag(NULL, '\0', 2 /* 检查读过新的Welcome 没 */) == 0)) {
		if (dashf("Welcome")) {
			ansimore("Welcome", YEA);
			vote_flag(NULL, 'R', 2 /* 写入读过新的Welcome */);
		}
	} else {
		if (fill_shmfile(3, "Welcome2", "WELCOME_SHMKEY"))
			show_welcomeshm();
	}
	show_statshm("etc/posts/day", 1);
	refresh();
	move(t_lines - 2, 0);
	clrtoeol();
	if (currentuser.numlogins < 1) {
		currentuser.numlogins = 0;
		getdatestring(time(0), NA);
		prints("[1;36m☆ 这是您第 [33m1[36m 次拜访本站，请记住今天吧。\n");
		prints("☆ 您第一次连入本站的时间为 [33m%s[m ", datestring);
	} else {
		getdatestring(currentuser.lastlogin, NA);
		prints(
				"[1;36m☆ 这是您第 [33m%d[36m 次拜访本站，上次您是从 [33m%s[36m 连往本站。\n",
				currentuser.numlogins + 1, currentuser.lasthost);
		prints("☆ 上次连线时间为 [33m%s[m ", datestring);
	}
	igetkey();
	WishNum = 9999;
	setuserfile(fname, BADLOGINFILE);
	if (ansimore(fname, NA) != -1) {
		//if (askyn("您要删除以上密码输入错误的记录吗", YEA, YEA) == YEA)
		if (askyn("您要删除以上密码输入错误的记录吗", NA, NA) == YEA)
			unlink(fname);
	}

	set_safe_record();
	check_uinfo(&currentuser, 0);
	strncpy(currentuser.lasthost, fromhost, 16);
	currentuser.lasthost[15] = '\0'; /* dumb mistake on my part */
	{
		time_t stay, recent;

		if (count_user() > 1) {
			recent = currentuser.lastlogout;
			if (currentuser.lastlogin > recent)
				recent = currentuser.lastlogin;
			stay = login_start_time - recent;
			if (stay < 0)
				stay = 0;
		} else
			stay = 0;

		if (login_start_time - currentuser.lastlogin >= 20 * 60
				|| !strcmp(currentuser.userid, "guest")
				|| currentuser.numlogins < 100){
			currentuser.numlogins++;
		}
		currentuser.lastlogin = login_start_time;
		currentuser.stay += stay;
	}

	if (HAS_PERM(PERM_SYSOPS) || !strcmp(currentuser.userid, "guest"))
		currentuser.lastjustify = time(0);
	if (HAS_PERM(PERM_REGISTER) && (abs(time(0) - currentuser.lastjustify)
			>= REG_EXPIRED * 86400)) {
#ifdef MAILCHECK
		currentuser.email[0] = '\0';
#endif
		currentuser.address[0] = '\0';
		/* Following line modified by Amigo 2002.06.08. To omit default perm_page right. */
		currentuser.userlevel &= ~(PERM_REGISTER | PERM_TALK);
		mail_file("etc/expired", currentuser.userid, "更新个人资料说明。");
	}
	/* Anonomous 2007.12.7
	 * Move the following line above
	 */
	/*currentuser.numlogins++;*/
#ifdef ALLOWGAME
	if (currentuser.money> 1000000) {
		currentuser.nummedals += currentuser.money / 10000;
		currentuser.money %= 1000000;
	}
	if ((signed int) (currentuser.money - currentuser.bet) < -4990
			&& currentuser.numlogins < 10 && currentuser.numposts < 10
			&& currentuser.nummedals == 0)
	currentuser.money += 1000;
#endif
	if (currentuser.firstlogin == 0) {
		currentuser.firstlogin = time(0) - 7 * 86400;
	}
	substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	/* Following line added by Amigo 2002.04.24. For check_register_info use. */
	m_init();
	check_register_info();
#ifdef CERTIFYMODE
	if (!strcmp(currentuser.userid, "dxwxb")) {
		certify_admin2();
	}
#endif
}

void set_numofsig() {
	int sigln;
	char signame[STRLEN];

	setuserfile(signame, "signatures");
	sigln = countln(signame);
	numofsig = sigln / MAXSIGLINES;
	if ((sigln % MAXSIGLINES) != 0)
		numofsig += 1;
}

#ifdef CHK_FRIEND_BOOK
int chk_friend_book()
{
	FILE *fp;
	int idnum, n = 0;
	char buf[STRLEN], *ptr;

	if ((fp = fopen("friendbook", "r")) == NULL)
	return 0;

	move(10, 0);
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		char uid[14];
		char msg[STRLEN];
		struct user_info *uin;

		ptr = strstr(buf, "@");
		if (ptr == NULL)
		continue;
		ptr++;
		strcpy(uid, ptr);
		ptr = strstr(uid, "\n");
		*ptr = '\0';
		idnum = atoi(buf);
		if (idnum != usernum || idnum <= 0)
		continue;
		uin = t_search(uid, NA);
		sprintf(msg, "%s 已经上站。", currentuser.userid);
		if (!uinfo.invisible && uin != NULL && !DEFINE(DEF_NOLOGINSEND)
				&& do_sendmsg(uin, msg, 2, uin->pid) == 1) {
			prints("[1m%s[m 找您，系统已经告诉他您上站的消息。\n", uid);
		} else
		prints("[1m%s[m 找您，系统无法联络到他，请您跟他联络。\n", uid);
		n++;
		del_from_file("friendbook", buf);
		if (n> 15) {
			pressanykey();
			move(10, 0);
			clrtobot();
		}
	}
	fclose(fp);
	if (n) {
		move(8, 0);
		prints("[1m系统寻人名册列表:[m");
	}
	return n;
}
#endif

int check_maxmail() {
	extern char currmaildir[STRLEN];
	int maxmail, maxsize, mailsize;

	maxmail = getmailboxhold(currentuser.userlevel);

	set_safe_record();
	currentuser.nummails = get_num_records(currmaildir,
			sizeof(struct fileheader));
	substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	maxsize = getmailboxsize(currentuser.userlevel);
	mailsize = getmailsize(currentuser.userid);
	if (currentuser.nummails > maxmail || mailsize > maxsize) {
		mailXX = 1;
		clear();
		move(4, 0);
		if (currentuser.nummails > maxmail)
			prints("您的私人信件高达 %d 封, 您的信件上限: %d 封\n", currentuser.nummails,
					maxmail);
		if (mailsize > maxsize)
			prints("您的信件容量高达 %d K，您的容量上限: %d K\n", mailsize, maxsize);
		prints("您的私人信件已经超限, 请整理信箱，否则无法使用本站的送信功能。\n");
		if (currentuser.nummails > maxmail + 100) {
			sprintf(genbuf, "私人信件过量: %d 封", currentuser.nummails);
		}
		if (mailsize > maxsize + 1000) {
			sprintf(genbuf, "私人信件过量: %d K", mailsize);
		}
	} else
		mailXX = 0;

	return mailXX;
}

#ifndef BBSD
int main(int argc, char **argv)
#else
void start_client()
#endif
{
	load_sysconf();

#ifdef ALLOWSWITCHCODE
	resolve_GbBig5Files();
#endif

#ifndef BBSD
	if (argc < 2 || *argv[1] != 'h') {
		printf("You cannot execute this program directly.\n");
		exit(-1);
	}
	system_init(argc, argv);
#else
	system_init();
#endif

	if (setjmp(byebye)) {
		system_abort();
	}
#ifndef BBSD
	get_tty();
	init_tty();
#endif

	login_query();
	user_login();
	m_init();
	RMSG = NA;
	clear();
	c_recover();
#ifdef TALK_LOG
	tlog_recover(); /* 990713.edwardc for talk_log recover */
#endif

	if (strcmp(currentuser.userid, "guest")) {
		if (HAS_PERM(PERM_USER)) {
			struct stat tpstat; //modified by money 2003.12.03 for judge file size

			stat("unregistered", &tpstat);
			if (tpstat.st_size > 0) {
				prints("[0;1m\n\n");
				prints("[33m目前有新使用者等候您的审批。[37m\n\n");
#ifdef SHOW_THANKYOU
				prints
				("感谢您使用 FB2000！非常希望您能经常光顾 [32m快意灌水站 [telnet fb2000.dhs.org][37m ！\n");
				prints
				("如果可能的话，[45;33m请回报您的使用情况![0;1;37m，包括： BUG、疑问、建议！非常感谢！\n\n");
				prints
				("如果您希望及时了解 FB2000 的最新情况，请订阅【FB2000 开发通讯】邮件列表！\n");
				prints
				("订阅方法：写信给 fb2000-request@list.cn99.com，正文填写：subscribe");
#endif
				pressanykey();
			}
		}
		if (check_maxmail())
			pressanykey();
#ifdef CHK_FRIEND_BOOK
		if (chk_friend_book())
		pressanykey();
#endif
		move(9, 0);
		clrtobot();
		if (!DEFINE(DEF_NOLOGINSEND))
			if (!uinfo.invisible)
				apply_ulist(friend_login_wall);
		clear();
		set_numofsig();
	}

	ActiveBoard_Init();

	fill_date(); /* 检查纪念日 */
	//b_closepolls();    /* 关闭投票 */

	num_alcounter();
	if (count_friends > 0 && DEFINE(DEF_LOGFRIEND))
		t_friends();
	while (1) {
		if (DEFINE(DEF_NORMALSCR))
			domenu("TOPMENU");
		else
			domenu("TOPMENU2");
		Goodbye();
	}
}

int refscreen = NA;

int egetch() {
	int rval;

	check_calltime();
	if (talkrequest) {
		talkreply();
		refscreen = YEA;
		return -1;
	}
	while (1) {
		rval = igetkey();
		if (talkrequest) {
			talkreply();
			refscreen = YEA;
			return -1;
		}
		if (rval != Ctrl('L'))
			break;
		redoscr();
	}
	refscreen = NA;
	return rval;
}

char *boardmargin() {
	static char buf[STRLEN];

	//Modified by IAMFAT 2002-05-26 Add ' '
	//Modified by IAMFAT 2002-05-28
	//Roll Back 2002-05-29
	if (selboard)
		sprintf(buf, "讨论区 [%s]", currboard);
	else {
		brc_initial(DEFAULTBOARD);
		if (!getbnum(currboard))
			setoboard(currboard);
		selboard = 1;
		sprintf(buf, "讨论区 [%s]", currboard);
	}
	return buf;
}

void update_endline() {
	char buf[255], fname[STRLEN], *ptr;
	time_t now;
	FILE *fp;
	int i, allstay, foo, foo2;

	move(t_lines - 1, 0);
	clrtoeol();

	if (!DEFINE(DEF_ENDLINE))
		return;

	now = time(0);
	allstay = getdatestring(now, NA); // allstay 为当前秒数
	if (allstay == 0) {
		nowishfile: resolve_boards();
		strcpy(datestring, brdshm->date);
		allstay = 1;
	}
	if (allstay < 5) {
		allstay = (now - login_start_time) / 60;
		sprintf(buf, "[[36m%.12s[33m]", currentuser.userid);
		num_alcounter();
		//Modified by IAMFAT 2002-05-26
		//Roll Back 2002-05-29
		prints(
				"[1;44;33m[[36m%29s[33m][[36m%4d[33m人/[1;36m%3d[33m友][[36m%1s%1s%1s%1s%1s%1s[33m]帐号%-24s[[36m%3d[33m:[36m%2d[33m][m",
				datestring, count_users, count_friends, (uinfo.pager
						& ALL_PAGER) ? "P" : "p", (uinfo.pager
						& FRIEND_PAGER) ? "O" : "o", (uinfo.pager
						& ALLMSG_PAGER) ? "M" : "m", (uinfo.pager
						& FRIENDMSG_PAGER) ? "F" : "f",
				(DEFINE(DEF_MSGGETKEY)) ? "X" : "x",
				(uinfo.invisible == 1) ? "C" : "c", buf, (allstay / 60)
						% 1000, allstay % 60);
		return;
	}
	setuserfile(fname, "HaveNewWish");
	if (WishNum == 9999 || dashf(fname)) {
		if (WishNum != 9999)
			unlink(fname);
		WishNum = 0;
		orderWish = 0;

		if (is_birth(currentuser)) {
			strcpy(GoodWish[WishNum],
			//Roll Back 2002-05-29
					"                     啦啦～～，生日快乐!   记得要请客哟 :P                   ");
			WishNum++;
		}

		setuserfile(fname, "GoodWish");
		if ((fp = fopen(fname, "r")) != NULL) {
			for (; WishNum < 20;) {
				if (fgets(buf, 255, fp) == NULL)
					break;
				buf[STRLEN - 4] = '\0';
				ptr = strtok(buf, "\n\r");
				if (ptr == NULL || ptr[0] == '#')
					continue;
				strcpy(buf, ptr);
				for (ptr = buf; *ptr == ' ' && *ptr != 0; ptr++)
					;
				if (*ptr == 0 || ptr[0] == '#')
					continue;
				for (i = strlen(ptr) - 1; i < 0; i--)
					if (ptr[i] != ' ')
						break;
				if (i < 0)
					continue;
				foo = strlen(ptr);
				foo2 = (STRLEN - 3 - foo) / 2;
				strcpy(GoodWish[WishNum], "");
				for (i = 0; i < foo2; i++)
					strcat(GoodWish[WishNum], " ");
				strcat(GoodWish[WishNum], ptr);
				for (i = 0; i < STRLEN - 3 - (foo + foo2); i++)
					strcat(GoodWish[WishNum], " ");
				GoodWish[WishNum][STRLEN - 4] = '\0';
				WishNum++;
			}
			fclose(fp);
		}
	}
	if (WishNum == 0)
		goto nowishfile;
	if (orderWish >= WishNum * 2)
		orderWish = 0;
	//Modified by IAMFAT 2002-05-26 insert space
	//Roll Back 2002-05-29
	prints("[0;1;44;33m[[36m%77s[33m][m", GoodWish[orderWish / 2]);
	orderWish++;
}

/*ReWrite by SmallPig*/
void showtitle(char *title, char *mid) {
	char buf[STRLEN], *note;
	int spc1;
	int spc2;

	note = boardmargin();
	spc1 = 39 + num_ans_chr(title) - strlen(title) - strlen(mid) / 2;
	//if(spc1 < 2) 
	//      spc1 = 2;
	//Modified by IAMFAT 2002-05-28
	//Roll Back 2002-05-29
	spc2 = 79 - (strlen(title) - num_ans_chr(title) + spc1 + strlen(note)
			+ strlen(mid));
	//if (spc2 < 1) 
	//      spc2 = 1;
	spc1 += spc2;
	spc1 = (spc1 > 2) ? spc1 : 2; //防止过小
	spc2 = spc1 / 2;
	spc1 -= spc2;
	move(0, 0);
	clrtoeol();
	sprintf(buf, "%*s", spc1, "");
	if (!strcmp(mid, BoardName))
		prints("[1;44;33m%s%s[37m%s[1;44m", title, buf, mid);
	else if (mid[0] == '[')
		prints("[1;44;33m%s%s[5;36m%s[m[1;44m", title, buf, mid);
	else
		prints("[1;44;33m%s%s[36m%s", title, buf, mid);
	sprintf(buf, "%*s", spc2, "");
	prints("%s[33m%s[m\n", buf, note);
	update_endline();
	move(1, 0);
}
void firsttitle(char *title) {
	char middoc[30];

	if (chkmail())
		strcpy(middoc, strstr(title, "讨论区列表") ? "[您有信件，按 M 看新信]"
				: "[您有信件]");
	else if (mailXX == 1)
		strcpy(middoc, "[信件过量，请整理信件!]");
	else
		strcpy(middoc, BoardName);

	showtitle(title, middoc);
}
void docmdtitle(char *title, char *prompt) {
	firsttitle(title);
	move(1, 0);
	clrtoeol();
	prints("%s", prompt);
	clrtoeol();
}

void c_recover() {
	char fname[STRLEN], buf[STRLEN];
	int a;

	sprintf(fname, "home/%c/%s/%s.deadve", toupper(currentuser.userid[0]),
			currentuser.userid, currentuser.userid);
	if (!dashf(fname) || strcmp(currentuser.userid, "guest") == 0)
		return;
	clear();
	strcpy(genbuf, "");
	getdata(0, 0,
			"[1;32m您有一个编辑作业不正常中断，(S) 写入暂存档 (M) 寄回信箱 (Q) 算了？[M]：[m",
			genbuf, 2, DOECHO, YEA);
	switch (genbuf[0]) {
		case 'Q':
		case 'q':
			unlink(fname);
			break;
		case 'S':
		case 's':
			while (1) {
				strcpy(genbuf, "");
				getdata(2, 0, "[1;33m请选择暂存档 [0-7] [0]：[m", genbuf, 2,
						DOECHO, YEA);
				if (genbuf[0] == '\0')
					a = 0;
				else
					a = atoi(genbuf);
				if (a >= 0 && a <= 7) {
					sprintf(buf, "home/%c/%s/clip_%d",
							toupper(currentuser.userid[0]),
							currentuser.userid, a);
					if (dashf(buf)) {
						getdata(
								3,
								0,
								"[1;31m暂存档已存在，覆盖或附加? (O)覆盖 (A)附加 [O]：[m",
								genbuf, 2, DOECHO, YEA);
						switch (genbuf[0]) {
							case 'A':
							case 'a':
								f_cp(fname, buf, O_APPEND);
								unlink(fname);
								break;
							default:
								unlink(buf);
								rename(fname, buf);
								break;
						}
					} else
						rename(fname, buf);
					break;
				}
			}
			break;
		default:
			mail_file(fname, currentuser.userid, "不正常断线所保留的部份...");
			unlink(fname);
			break;
	}
}

#ifdef TALK_LOG
void tlog_recover()
{
	char buf[256];

	sprintf(buf, "home/%c/%s/talk_log",
			toupper(currentuser.userid[0]), currentuser.userid);

	if (strcasecmp(currentuser.userid, "guest") == 0 || !dashf(buf))
	return;

	clear();
	strcpy(genbuf, "");
	getdata(0, 0,
			"[1;32m您有一个不正常断线所留下来的聊天记录, 您要 .. (M) 寄回信箱 (Q) 算了？[Q]：[m",
			genbuf, 2, DOECHO, YEA);

	if (genbuf[0] == 'M' || genbuf[0] == 'm') {
		mail_file(buf, currentuser.userid, "聊天记录");
	}
	unlink(buf);
	return;
}
#endif
