#include "bbs.h"
#include "screen.h"
#ifdef lint
#include <sys/uio.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

extern int RMSG;
extern int msg_num;
struct user_info *t_search();
void r_msg();

char buf2[MAX_MSG_SIZE+2];

//在阅读信息期间，又有其他信息发入，那么以消息的形式通知msg_num++
void count_msg(int notused)
{
	signal(SIGUSR2, count_msg);  
	msg_num++;
}

//获取msg记录数目
int get_num_msgs(const char *filename)
{
	int i = 0;
	struct stat st;
	if(stat(filename, &st) == -1)
		return 0;
	char head[256], msg[MAX_MSG_SIZE+2];
	FILE *fp;
	if((fp = fopen(filename, "r")) == NULL)
		return 0;
	while (true) {
		if(fgets(head, sizeof(head), fp) != NULL && fgets(msg, MAX_MSG_SIZE + 2, fp) != NULL)
			i++;
		else
			break;
	}
	fclose(fp);
	return i;
}

int get_msg(const char *uid, char *msg, int line)
{
	char buf[3];
	int gdata;
	int msg_line;
	move(line, 0);
	clrtoeol();
	prints("送音信给：%s  按Ctrl+Q重写当前消息.     音信:", uid);
	msg[0] = 0;
	while (true) {
		msg_line = multi_getdata(line + 1, 0, LINE_LEN - 1, NULL, msg, MAX_MSG_SIZE + 1, MAX_MSG_LINE, 0, 0);
		if (msg[0] == '\0')
			return NA;
		gdata = getdata(line + 4, 0, "确定要送出吗(Y)是的 (N)不要 (E)再编辑? [Y]: ",
				buf, 2, DOECHO, YEA);
		if (gdata == -1)
			return NA;
		if (buf[0] == 'e' || buf[0] == 'E')
			continue;
		if (buf[0] == 'n' || buf[0] == 'N')
			return NA;
		else
			return YEA;
	} //while
}

char msgchar(struct user_info *uin)
{
	if (uin->mode == FIVE|| uin->mode == BBSNET || uin->mode == LOCKSCREEN) 
		return '@';
	if (isreject(uin))
		return '*';
	if ((uin->pager & ALLMSG_PAGER))
		return ' ';
	if (hisfriend(uin)) {
		if ((uin->pager & FRIENDMSG_PAGER))
			return 'O';
		else
			return '#';
	}
	return '*';
}

int canmsg(const struct user_info *uin)
{
	if (isreject(uin))
		return NA;
	if ((uin->pager & ALLMSG_PAGER) || HAS_PERM(PERM_OCHAT))
		return YEA;
	if ((uin->pager & FRIENDMSG_PAGER) && hisfriend(uin))
		return YEA;
	return NA;
}

void s_msg(void)
{
	do_sendmsg(NULL, NULL, 0, 0);
}

int send_msg(int ent, const struct fileheader *fileinfo, char *direct)
{
	struct user_info* uin;
	if (!strcmp(currentuser.userid,"guest"))
		return DONOTHING;
	uin = t_search(fileinfo->owner, NA);
	if (uin == NULL || (uin->invisible && !HAS_PERM(PERM_SEECLOAK))) {
		move(2, 0);
		prints("对方目前不在线上...\n");
		pressreturn();
	} else {
		do_sendmsg(uin, NULL, 0, uin->pid);
	}
	return FULLUPDATE;
}

int show_allmsgs(void)
{
	char fname[STRLEN];
	if (!strcmp(currentuser.userid, "guest"))
		return;
#ifdef LOG_MY_MESG
	setuserfile(fname, "msgfile.me");
#else
	setuserfile(fname, "msgfile");
#endif
	clear();
	modify_user_mode(LOOKMSGS);
	if (dashf(fname)) {
		mesgmore(fname, YEA, 0, 9999);
		clear();
	} else {
		move(5, 30);
		prints("没有任何的讯息存在！！");
		pressanykey();
		clear();
	}//if dashf(fname)
}

int do_sendmsg(const struct user_info *uentp, const char *msgstr, int mode, int userpid)
{
	char uident[STRLEN], ret_str[20];
	time_t now;
	const struct user_info *uin;
	char buf[MAX_MSG_SIZE+2], *msgbuf, *timestr;
#ifdef LOG_MY_MESG
	char *mymsg, buf2[MAX_MSG_SIZE+2];
	int ishelo = 0; /* 是不是好友上站通知讯息 */
	mymsg = (char *) malloc(MAX_MSG_SIZE + 256);
#endif
	msgbuf = (char *) malloc(MAX_MSG_SIZE + 256);

	char wholebuf[MAX_MSG_SIZE+2];
	if (mode == 0) {
		move(2, 0);
		clrtobot();
		modify_user_mode(MSG);
	}
	if (uentp == NULL) {
		prints("<输入使用者代号>\n");
		move(1, 0);
		clrtoeol();
		prints("送讯息给: ");
		creat_list();
		namecomplete(NULL, uident);
		if (uident[0] == '\0') {
			clear();
			return 0;
		}
		if(!strcasecmp(uident, "guest")) {
			clear();
			return 0;
		}
		uin = t_search(uident, NA);
		if (uin == NULL) {
			move(2, 0);
			prints("对方目前不在线上...\n");
			pressreturn();
			move(2, 0);
			clrtoeol();
			return -1;
		}
		if (is_web_user(uin->mode) || uin->mode == BBSNET
				||uin->mode == LOCKSCREEN || uin->mode == PAGE
				|| uin->mode == FIVE || !canmsg(uin)) {
			move(2, 0);
			prints("目前无法传送讯息给对方.\n");
			pressreturn();
			move(2, 0);
			clrtoeol();
			return -1;
		}
	} else {
		if (uentp->uid == usernum)
			return 0;
		uin = uentp;
		if (is_web_user(uin->mode) || uin->mode == BBSNET
				|| uin->mode == PAGE || uin->mode == LOCKSCREEN 
				|| uin->mode == FIVE || (mode != 2 && !canmsg(uin))) //add mode!=2 by quickmouse
			return 0;
		strcpy(uident, uin->userid);
	}
	if (msgstr == NULL) {
		if (!get_msg(uident, buf, 1)) {
			int i;
			for(i = 1; i <= MAX_MSG_LINE+ 1; i++) {
				move(i, 0);
				clrtoeol();
			}
			return 0;
		}
	}
	now = time(NULL);
	timestr = ctime(&now);
	strcpy(ret_str, "^Z回");
	if (msgstr == NULL || mode == 2) {
		sprintf(msgbuf, "\033[0;1;44;36m%-12.12s\033[33m(\033[36m%-24.24s\033[33m):\033[37m%-34.34s\033[31m(%s)\033[m\033[%05dm\n", currentuser.userid, timestr , " ", ret_str, uinfo.pid);
		sprintf(wholebuf, "%s\n", msgstr == NULL ? buf : msgstr);
		strcat(msgbuf, wholebuf);
#ifdef LOG_MY_MESG
		sprintf(mymsg, "\033[1;32;40mTo \033[1;33;40m%-12.12s\033[m(%-24.24s):%-38.38s\n", uin->userid, timestr, " ");
		sprintf(wholebuf, "%s\n", msgstr == NULL ? buf : msgstr);
		strcat(mymsg, wholebuf);
		sprintf(buf2, "你的好朋友 %s 已经上站罗！", currentuser.userid);
		if (msgstr != NULL)
			if (strcmp(msgstr, buf2) == 0)
				ishelo = 1;
			else if (strcmp(buf, buf2) == 0)
				ishelo = 1;
#endif
	} else if (mode == 0) {
		sprintf(msgbuf, "\033[0;1;5;44;33m站长 于\033[36m %24.24s \033[33m广播：\033[m\033[1;37;44m%-39.39s\033[m\033[%05dm\n", timestr," ",  uinfo.pid); 
		sprintf(wholebuf, "%s\n", msgstr);
		strcat(msgbuf, wholebuf);        

	} else if (mode == 1) {
		sprintf(msgbuf, "\033[0;1;44;36m%-12.12s\033[37m(\033[36m%-24.24s\033[37m) 邀请你\033[37m%-34.34s\033[31m(%s)\033[m\033[%05dm\n", currentuser.userid, timestr, " ", ret_str, uinfo.pid); 
		sprintf(wholebuf, "%s\n", msgstr); 
		strcat(msgbuf, wholebuf);
	} else if (mode == 3) {
		sprintf(msgbuf, "\033[0;1;45;36m%-12.12s\033[33m(\033[36m%-24.24s\033[33m):\033[37m%-34.34s\033[31m(%s)\033[m\033[%05dm\n", currentuser.userid, timestr, " ", ret_str, uinfo.pid); 
		sprintf(wholebuf, "%s\n", msgstr == NULL ? buf : msgstr);
		strcat(msgbuf, wholebuf);
	}
	else if (mode == 4) {
		sprintf(msgbuf, "\033[0;1;45;36m%-12.12s\033[36m向您告别(\033[1;36;45m%24.24s\033[36m)：\033[m\033[1;36;45m%-38.38s\033[m\033[%05dm\n", currentuser.userid, timestr, " ", 0); 
		sprintf(wholebuf, "%s\n", msgstr); 
		strcat(msgbuf, wholebuf);
	}
	if (userpid) {
		if (userpid != uin->pid) {
			saveline(0, 0);	/* Save line */
			move(0, 0);
			clrtoeol();
			prints("[1m对方已经离线...[m\n");
			sleep(1);
			saveline(0, 1);	/* restore line */
			return -1;
		}
	}
	if (!uin->active || bbskill(uin, 0) == -1) {
		if (msgstr == NULL) {
			prints("\n对方已经离线...\n");
			pressreturn();
			clear();
		}
		return -1;
	}
	sethomefile(buf, uident, "msgfile");
	file_append(buf, msgbuf);
#ifdef LOG_MY_MESG
	if (mode == 2 || (mode == 0 && msgstr == NULL)) {
		if (ishelo == 0) {
			sethomefile(buf, currentuser.userid, "msgfile.me");
			file_append(buf, mymsg);
		}
	}
	sethomefile(buf, uident, "msgfile.me");
	file_append(buf, msgbuf);
	free(mymsg);
#endif
	free(msgbuf);
	if(uin->pid) {
		bbskill(uin, SIGUSR2);
	}
	if (msgstr == NULL) {
		prints("\n已送出讯息...\n");
		pressreturn();
		clear();
	}
	return 1;
}

int dowall(const struct user_info *uin)
{
	if (!uin->active || !uin->pid)
		return -1;
	move(1, 0);
	clrtoeol();
	prints("\033[1;32m正对 %s 广播.... Ctrl-D 停止对此位 User 广播。\033[m", uin->userid);
	refresh();
	do_sendmsg(uin, buf2, 0, uin->pid);
}

int myfriend_wall(const struct user_info *uin)
{
	if ((uin->pid - uinfo.pid == 0) || !uin->active || !uin->pid || isreject(uin))
		return -1;
	if (myfriend(uin->uid)) {
		move(1, 0);
		clrtoeol();
		prints("\033[1;32m正在送讯息给 %s...  \033[m", uin->userid);
		refresh();
		do_sendmsg(uin, buf2, 3, uin->pid);
	}
}

int hisfriend_wall_logout(const struct user_info *uin)
{
	if ((uin->pid - uinfo.pid == 0) || !uin->active || !uin->pid 
			|| isreject(uin) || !(uin->pager & LOGOFFMSG_PAGER))
		return -1;
	if (hisfriend(uin)) {
		refresh();
		do_sendmsg(uin, buf2, 4, uin->pid);
	}
}

int hisfriend_wall(const struct user_info *uin)
{
	if ((uin->pid - uinfo.pid == 0) || !uin->active || !uin->pid || isreject(uin))
		return -1;
	if (hisfriend(uin)) {
		refresh();
		do_sendmsg(uin, buf2, 3, uin->pid);
	}
}

int friend_wall(void)
{
	char    buf[3];
	char    msgbuf[MAX_MSG_SIZE + 256], filename[80];
	time_t  now;
	char   *timestr;
	now = time(0);
	timestr = ctime(&now) + 11;
	*(timestr + 8) = '\0';

	char wholebuf[MAX_MSG_SIZE+2];

	modify_user_mode(MSG);
	move(2, 0);
	clrtobot();
	getdata(1, 0, "送讯息给 [1] 我的好朋友，[2] 与我为友者: ",
			buf, 2, DOECHO, YEA);
	switch (buf[0]) {
		case '1':
			if (!get_msg("我的好朋友", buf2, 1))
				return 0;
			if (apply_ulist(myfriend_wall) == -1) {
				move(2, 0);
				prints("线上空无一人\n");
				pressanykey();
			} else {
				sprintf(msgbuf, "\033[0;1;45;36m送讯息给好友\033[33m(\033[36m%-24.24s\033[33m):\033[37m%-39.39s\033[31m(^Z回)\033[m\033[%05dm\n", timestr," ",  uinfo.pid); 
				sprintf(wholebuf, "%s\n", buf2);
				strcat(msgbuf, wholebuf);
				setuserfile(filename, "msgfile.me");
				file_append(filename, msgbuf);
			}
			break;
		case '2':
			if (!get_msg("与我为友者", buf2, 1))
				return 0;
			if (apply_ulist(hisfriend_wall) == -1) {
				move(2, 0);
				prints("线上空无一人\n");
				pressanykey();
			} else {
				sprintf(msgbuf, "[0;1;45;36m送给与我为友[33m([36m%-24.24s[33m):[37m%-39.39s[31m(^Z回)[m[%05dm\n", timestr," ",  uinfo.pid); 
				sprintf(wholebuf, "%s\n", buf2);
				strcat(msgbuf, wholebuf);
				setuserfile(filename, "msgfile.me");
				file_append(filename, msgbuf);

			}
			break;
		default:
			return 0;
	}
	move(7, 0);
	prints("讯息传送完毕...");
	pressanykey();
	return 1;
}

void r_msg2(void)
{
	FILE   *fp;
	char    bufhead[256];
	char    msghead[256];
	char    buf[MAX_MSG_SIZE+2];
	char    msg[MAX_MSG_SIZE+2];
	char    fname[STRLEN];
	int     line, tmpansi;
	int     y, x, ch, i, j;
	int     MsgNum;
	struct sigaction act;
	int k;
	int msg_line = 0;    

	if (!strcmp(currentuser.userid,"guest"))
		return;
	getyx(&y, &x);
	if (uinfo.mode == TALK)
		line = t_lines / 2 - 1;
	else
		line = 0;
	setuserfile(fname, "msgfile");
	i = get_num_msgs(fname);
	if (i == 0)
		return;
	signal(SIGUSR2, count_msg);
	tmpansi = showansi;
	showansi = 1;
	oflush();
	if (RMSG == NA) {
		for(k = 0; k < MAX_MSG_LINE*2 + 2; k++) {   
			saveline_buf(k, 0);
		}
	}
	MsgNum = 0;
	RMSG = YEA;
	while (true) {
		MsgNum = (MsgNum % i);
		if ((fp = fopen(fname, "r")) == NULL) {
			RMSG = NA;
			if (msg_num)
				r_msg();
			else {
				sigemptyset(&act.sa_mask);
				act.sa_flags = SA_NODEFER;
				act.sa_handler = r_msg;
				sigaction(SIGUSR2, &act, NULL);
			}
			return;
		}
		for (j = 0; j < (i - MsgNum); j++) {
			if (fgets(bufhead, 256, fp) == NULL || fgets(buf, MAX_MSG_SIZE + 2, fp) == NULL)
				break;
			else
			{
				strcpy(msghead, bufhead);
				strcpy(msg,buf);
			}
		}
		fclose(fp);
		move(line, 0);
		clrtoeol();
		prints("%s", msghead);
		move(line+1, 0);
		clrtoeol();
		msg_line = show_data(msg, LINE_LEN-1, line + 1, 0); 
		refresh();
		{
			struct user_info *uin;
			char    msgbuf[STRLEN];
			int     good_id, send_pid;
			char   *ptr, usid[STRLEN];
			ptr = strrchr(msghead, '[');
			send_pid = atoi(ptr + 1);
			ptr = strtok(msghead + 12, " \033[");

			if (ptr == NULL)
				good_id = NA;
			else if (!strcmp(ptr, currentuser.userid))
				good_id = NA;
			else {
				strcpy(usid, ptr);
				uin = t_search(usid, send_pid);
				if (send_pid == 0)
					send_pid = -1;
				good_id = NA;
				if (uin != NULL && ( uin->pid == send_pid || canmsg(uin) ))
					good_id = YEA;
			}
			if (good_id == YEA) {
				int userpid;
				userpid = uin->pid;
				move(msg_line, 0);
				clrtoeol();
				sprintf(msgbuf, "回讯息给 %s: ", usid);
				prints("%s", msgbuf);

				move(msg_line+1, 0);
				clrtoeol();
				refresh();
				multi_getdata(msg_line + 1, 0, LINE_LEN - 1, NULL, buf, MAX_MSG_SIZE + 1, MAX_MSG_LINE, 1, 0);
				if (buf[0] == Ctrl('Z')) {
					MsgNum++;
					continue;
				} else if (buf[0] == Ctrl('A')) {
					MsgNum--;
					if (MsgNum < 0)
						MsgNum = i - 1;
					continue;
				}
				if (buf[0] != '\0') {
					if (do_sendmsg(uin, buf, 2, userpid) == 1)
						sprintf(msgbuf, "\033[1;32m帮您送出讯息给 %s 了!\033[m", usid);
					else
						sprintf(msgbuf, "\033[1;32m讯息无法送出.\033[m");
				} else
					sprintf(msgbuf, "\033[1;33m空讯息, 所以不送出.\033[m");
				move(msg_line, 0);
				clrtoeol();
				refresh();
				prints("%s", msgbuf);
				refresh();
				if (!strstr(msgbuf, "帮您"))
					sleep(1);
			} else {
				sprintf(msgbuf, "\033[1;32m找不到发讯息的 %s! 请按上:[^Z ↑] 或下:[^A ↓] 或其他键离开.\033[m", usid);
				move(msg_line, 0);
				clrtoeol();
				refresh();
				prints("%s", msgbuf);
				refresh();
				if ((ch = igetkey()) == Ctrl('Z')) {
					MsgNum++;
					continue;
				}
				if (ch == Ctrl('A')) {
					MsgNum--;
					if (MsgNum < 0)
						MsgNum = i - 1;
					continue;
				}
			}
		}
		break;
	}
	for(k = 0; k < MAX_MSG_LINE * 2 + 2; k++) {
		saveline_buf(k, 1);
	}
	showansi = tmpansi;
	move(y, x);
	refresh();
	RMSG = NA;
	if (msg_num)
		r_msg();
	else {
		sigemptyset(&act.sa_mask);
		act.sa_flags = SA_NODEFER;
		act.sa_handler = r_msg;
		sigaction(SIGUSR2, &act, NULL);
	}
	return;
}

void r_msg(int notused)
{
	FILE   *fp;
	char    bufhead[256];
	char    msghead[256];
	char    buf[MAX_MSG_SIZE+2];
	char    mustbak_title[80];
	char    msg[MAX_MSG_SIZE+2];
	char    fname[STRLEN];
	int     line, tmpansi;
	int     y, x, i, j, premsg;
	char    ch;
	struct sigaction act;

	int k;
	int msg_line = 0;

	signal(SIGUSR2, count_msg);
	msg_num++;
	getyx(&y, &x);
	tmpansi = showansi;
	showansi = 1;
	if (uinfo.mode == TALK)
		line = t_lines / 2 - 1;
	else
		line = 0;
	if (DEFINE(DEF_MSGGETKEY)) {
		oflush();
		//saveline(line, 0);
		for(k = 0; k < MAX_MSG_LINE + 1; k++)
		{
			saveline_buf(k, 0);
		}
		premsg = RMSG;
	}
	while (msg_num) {
		if (DEFINE(DEF_SOUNDMSG)) {
			bell();
		}
		setuserfile(fname, "msgfile");
		//i = get_num_records(fname, 129);
		i = get_num_msgs(fname);
		if ((fp = fopen(fname, "r")) == NULL){
			sigemptyset(&act.sa_mask);
			act.sa_flags = SA_NODEFER;
			act.sa_handler = r_msg;
			sigaction(SIGUSR2, &act, NULL);
			return;
		}
		for (j = 0; j <= (i - msg_num); j++) {
			if (fgets(bufhead, 256, fp) == NULL || fgets(buf, MAX_MSG_SIZE + 2, fp) == NULL)
				break;
			else
			{
				strcpy(msghead,bufhead);
				strcpy(msg,buf);
			}
		}
		fclose(fp);
		move(line, 0);
		clrtoeol();
		//prints("%s", msg);
		prints("%s", msghead);

		move(line+1, 0);
		clrtoeol();
		msg_line = show_data(msg, LINE_LEN-1, line + 1, 0); //返回的是msg_line行,也就是回信息的第一行

		refresh();
		msg_num--;
		if (DEFINE(DEF_MSGGETKEY)) {
			RMSG = YEA;
			ch = 0;
#ifdef MSG_CANCLE_BY_CTRL_C
			while (ch != Ctrl('C'))
#else
				while (ch != '\r' && ch != '\n')
#endif
				{
					ch = igetkey();
#ifdef MSG_CANCLE_BY_CTRL_C
					if (ch == Ctrl('C'))
						break;
#else
					if (ch == '\r' || ch == '\n')
						break;
#endif
					else if (ch == Ctrl('R') || ch == 'R' || ch == 'r' || ch == Ctrl('Z')) {
						struct user_info *uin;
						char    msgbuf[STRLEN];
						int     good_id, send_pid;
						char   *ptr, usid[STRLEN];
						ptr = strrchr(msghead, '[');
						send_pid = atoi(ptr + 1);
						/* added by roly 02.06.02 for disable reply logout msg*/
						if (send_pid==0) send_pid=-1;
						/* add end */ 
						ptr = strtok(msghead + 12, " [");
						if (ptr == NULL)
							good_id = NA;
						else if (!strcmp(ptr, currentuser.userid))
							good_id = NA;
						else {
							strcpy(usid, ptr);
							uin = t_search(usid, send_pid);
							if (uin == NULL) 
								good_id = NA;
							else
								good_id = YEA;
						}
						oflush();
						//saveline(line + 1, 2);
						for(k = MAX_MSG_LINE + 1; k < MAX_MSG_LINE*2 + 2; k++)
						{
							saveline_buf(k, 0);
						}

						if (good_id == YEA) {
							int     userpid;
							userpid = uin->pid;
							move(msg_line, 0);
							clrtoeol();
							sprintf(msgbuf, "立即回讯息给 %s: ", usid);
							prints("%s", msgbuf);

							move(msg_line + 1, 0);
							clrtoeol();
							refresh();
							multi_getdata(msg_line + 1, 0, LINE_LEN-1, NULL, buf, MAX_MSG_SIZE+1, MAX_MSG_LINE, 1, 0);
							if (buf[0] != '\0' && buf[0] != Ctrl('Z') && buf[0] != Ctrl('A')) {
								if (do_sendmsg(uin, buf, 2, userpid))
									sprintf(msgbuf, "[1;32m帮您送出讯息给 %s 了![m", usid);
								else
									sprintf(msgbuf, "[1;32m讯息无法送出.[m");
							} else
								sprintf(msgbuf, "[1;33m空讯息, 所以不送出. [m");
						} else {
							sprintf(msgbuf, "[1;32m找不到发讯息的 %s.[m", usid);
						}
						move(msg_line, 0);
						clrtoeol();
						refresh();
						prints("%s", msgbuf);
						refresh();
						if (!strstr(msgbuf, "帮您"))
							sleep(1);
						//saveline(line + 1, 3);

						for(k = msg_line; k < MAX_MSG_LINE*2+2; k++)
						{
							saveline_buf(k, 1);
						}
						refresh();
						break;
					}	/* if */
				}	/* while */
		}		/* if */
	}			/* while */

	setuserfile(fname, "msgfile.me");
	//i = get_num_records(fname, 129);
	i = get_num_msgs(fname);
	if (i > 500) {
		char bak_title[STRLEN];
		sprintf(mustbak_title, "[%s] 强制讯息备份%d条", getdatestring(time(NULL), NA), i);
		strncpy(bak_title, save_title, STRLEN-1);
		bak_title[STRLEN-1]=0;
		mail_file(fname, currentuser.userid, mustbak_title);
		strcpy(save_title, bak_title);
		unlink(fname);
	}

	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NODEFER;
	act.sa_handler = r_msg;
	sigaction(SIGUSR2, &act, NULL);

	if (DEFINE(DEF_MSGGETKEY)) {
		RMSG = premsg;
		//saveline(line, 1);
		for(k = 0; k < msg_line; k++)
		{
			saveline_buf(k,1);
		}
	}
	showansi = tmpansi;
	move(y, x);
	refresh();
	return;
}

int friend_login_wall(const struct user_info *pageinfo)
{
	char    msg[MAX_MSG_SIZE+2];
	int     x, y;
	if (!pageinfo->active || !pageinfo->pid || isreject(pageinfo))
		return 0;
	if (hisfriend(pageinfo)) {
		if (getuser(pageinfo->userid) <= 0)
			return 0;
		if (!(lookupuser.userdefine & DEF_LOGINFROM))
			return 0;
		if (pageinfo->uid ==usernum)
			return 0;
		/* edwardc.990427 好友隐身就不显示送出上站通知 */
		if (pageinfo->invisible)
			return 0;
		getyx(&y, &x);
		if (y > 22) {
			pressanykey();
			move(7, 0);
			clrtobot();
		}
		prints("送出好友上站通知给 %s\n", pageinfo->userid);
		sprintf(msg, "你的好朋友 %s 已经上站罗！", currentuser.userid);
		do_sendmsg(pageinfo, msg, 2, pageinfo->pid);
	}
	return 0;
}
