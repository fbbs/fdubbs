#include "bbs.h"
//extern  cmpbnames();
//rewritten by iamfat 2002.07.20
//原始文件从以前的备份0720里面找:P

//需要用到的外部函数
extern struct boardheader *getbcache();

//定义页面大小
#define BBS_PAGESIZE (t_lines-4)

//从第no行开始,在文件fname中不分大小写查找str,
//	若找到,返回其所在的行数
//		否则返回-1
int text_find(char* fname, int no, char* str, char* line) {
	int fd;
	int gotit=0;
	if ((fd=open(fname, O_RDONLY))==-1)
		return -1;
	seek_nth_line(fd, no);
	while (readln(fd, genbuf)) {
		//strtok(genbuf,"\n");
		if (strcasestr_gbk(genbuf, str)) {
			gotit=1;
			if (line)
				strcpy(line, genbuf);
			break;
		}
		no++;
	}
	close(fd);
	if (gotit)
		return no;
	return -1;
}
//函数: void list_text(char *fname, void (*title_show)(), int (*key_deal)(), int (*ifcheck)())
//功能: 全屏按行列出文本文件的内容, 并实现游标操作, 以及选项操作
//(IN)fname: 文本文件的文件名
//(IN)title_show(): 显示标题行(3行)
//(IN)key_deal(fname,ch,line): 非常规键的处理函数, fname文件名, ch按键, line游标停留的行
//(IN)ifcheck(): 判断是否在该项前面显示√, 如果为NULL, 则忽略选择操作
void list_text(char *fname, void (*title_show)(), int (*key_deal)(), int (*ifcheck)()) {
	int fd;
	int no = 0, from = 0;
	int to = 0;
	int ch;
	int empty = 0;
	int redrawflag = 1;
	int y = 3;
	char textfile[STRLEN];
	char line[STRLEN];
	strcpy(textfile, fname);
	while (1) {
		if (redrawflag) {
			clear();
			title_show();
			if ((fd=open(textfile, O_RDONLY))==-1) {
				empty=1;
			} else {
				empty=0;
				to=from;
				seek_nth_line(fd, from);
				y=3;
				move(3, 0);
				while (readln(fd, genbuf)) {
					strtok(genbuf, "\n");
					if (ifcheck)
						prints(" %-2s%-76s\n", (*ifcheck)(genbuf) ? "√"
								: "  ", genbuf);
					else
						prints(" %-78s\n", genbuf);
					to++;
					y++;
					if (y>t_lines-2)
						break;
				}
				if (from==to) {
					if (from==0)
						empty=1;
					else {
						from-=(BBS_PAGESIZE-1);
						if (from<0)
							from=0;
						no=from;
						continue;
					}
				}
				if (!empty && no>to-1)
					no=to-1;
				close(fd);
			}
			if (empty)
				prints("(无内容)\n");
			update_endline();
		}
		if (!empty) {
			move(3+no-from, 0);
			prints(">");
		}
		ch = egetch();
		redrawflag=0;
		if (!empty) {
			move(3+no-from, 0);
			prints(" ");
		}
		switch (ch) {
			case KEY_UP:
				if (empty)
					break;
				no--;
				if (no<from) {
					from-=(BBS_PAGESIZE-1);
					if (from<0)
						from=0;
					if (no<from)
						no=from;
					redrawflag=1;
				}
				break;
			case KEY_DOWN:
				if (empty)
					break;
				if (no<to-1) {
					no++;
					if (no==(from+BBS_PAGESIZE-1)) {
						from+=(BBS_PAGESIZE-1);
						no=from;
						redrawflag=1;
					}
				}
				break;
			case Ctrl('B'):
			case KEY_PGUP:
				if (empty)
					break;
				from-=(BBS_PAGESIZE-1);
				no-=(BBS_PAGESIZE-1);
				if (from<0)
					from=0;
				if (no<from)
					no=from;
				redrawflag=1;
				break;
			case Ctrl('F'):
			case KEY_PGDN:
				if (empty)
					break;
				if (to-from>=BBS_PAGESIZE) {
					from+=(BBS_PAGESIZE-1);
					no+=(BBS_PAGESIZE-1);
					redrawflag=1;
				} else
					no=to-1;
				break;
			case '/': //查找字符串
			{
				int oldno;
				if (empty)
					break;
				getdata(1, 0, "查找:", line, 50, DOECHO, YEA);
				redrawflag=1;
				oldno=no;
				no++;
				no=text_find(textfile, no, line, line);
				if (no<0)
					no=oldno;
				else
					from=no-no%(BBS_PAGESIZE-1);
			}
				break;
			case KEY_LEFT:
			case KEY_ESC:
			case '\n':
				return;
			default:
				if (key_deal) {
					if (empty||(fd=open(textfile, O_RDONLY))==-1) {
						redrawflag=(*key_deal)(textfile, ch, NULL);
					} else {
						seek_nth_line(fd, no);
						readln(fd, genbuf);
						//strtok(genbuf,"\n");
						strcpy(line, genbuf);
						redrawflag=(*key_deal)(textfile, ch, line);
						if (fd!=-1)
							close(fd);
					}
				}
		}// switch
	}//while (1)
	return;
}

//modified by iamfat 2002.07.20
//以下是reason函数
char* def_reasonlist="-------------------------------";
char* reasonlist ="ABCDEFGHIJKLMNOPQRSTUVWXYZ;'.[]";
int reasoncount=30;
char reason[50]; //封禁原因 全局变量
char detailreason[4096];
char reason_detail[STRLEN]; //原因文件
char reason_suggestion[STRLEN]; //原因文件
char deny_uid[IDLEN+1];
char club_uid[IDLEN+1];

//	设置默认封禁原因
void setreasondefault() {
	strcpy(reason, def_reasonlist);
}
//设置原因,将参数传过来的字符串拷贝到reason字符串中去
void setreason(char* rsn, int i) {
	strncpy(reason, rsn, i);
	reason[i]='\0';
}
//	设置封禁时的标题
void reason_title_show() {
	move(0, 0);
	prints("[1;44;36m                                    封禁原因列表                               [m\n");
	prints(" 封禁%s的原因:%s\n", deny_uid, reason);
	prints("[1;44m 封禁的原因及处罚建议                                                          [m\n");
}
//
int reason_key_deal(char* fname, int ch, char* line) {
	char flag;
	int no=0;
	if (!line)
		return 0;
	ch=toupper(ch);
	if (strchr(reasonlist, ch)) {
		while (no<reasoncount && reasonlist[no]) {
			if (reasonlist[no]==ch) {
				reason[no]=(reason[no]=='-') ? reasonlist[no] : '-';
				break;
			}
			no++;
		}
	} else if (ch==' ') {
		flag=line[0];
		while (reasonlist[no]!=flag)
			no++;
		reason[no]=(reason[no]=='-') ? reasonlist[no] : '-';
	}
	return 1;
}
//检查line字符串首字母是否在reason中,是返回1,否则0
int reason_check(char* line) {
	if (strchr(reason, line[0]))
		return 1;
	return 0;
}
int getreasoncount() {
	FILE *fp;
	char line[256];
	if (fp=fopen(reason_suggestion, "r")) {
		reasoncount=0;
		while (fgets(line, 256, fp) && strchr(reasonlist, line[0]))
			reasoncount++;
		fclose(fp);
	}
	return reasoncount;
}
void changereason(char* fname) {
	sprintf(reason_suggestion, "%s.suggestion", fname);
	sprintf(reason_detail, "%s.detail", fname);
	reason[getreasoncount()]='\0';
	while (1) {
		list_text(reason_suggestion, reason_title_show, reason_key_deal,
				reason_check);
		if (strncmp(reason, def_reasonlist, reasoncount))
			break;
	}
}
char* getreason() {
	return reason;
}
char* getdetailreason() {
	FILE* fp;
	char line[256];
	char* dtlrsn=detailreason;
	if (fp=fopen(reason_detail, "r")) {
		while (fgets(line, 256, fp)) {
			if (strchr(reason, line[0])) {
				strcpy(dtlrsn, line+1);
				dtlrsn+=strlen(line+1);
				//*(dtlrsn++)='\n';
			}
		}
		fclose(fp);
	}
	*dtlrsn='\0';
	return detailreason;
}

//以下是版面封禁相关函数
int seekname(char *deny_uid) {
	FILE* fp;
	char uident[IDLEN+1];
	char fname[STRLEN];
	char line[256];
	setbfile(fname, currboard, "deny_users");
	if (!(fp=fopen(fname, "r")))
		return 0;
	while (fgets(line, 256, fp)) {
		strncpy(uident, line, IDLEN);
		uident[IDLEN] = '\0';
		strtok(uident, " \r\n\t");
		if (strcasecmp(deny_uid, uident)==0) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

int addtodeny(char *uident, char *msg, int ischange) {
	char strtosave[512], buf[50], buf2[50];
	int day, seek;
	time_t nowtime;
	if (!strcmp(uident, "guest")) {
		move(t_lines-1, 0);
		prints("你在搞笑吗?封guest?");
		egetch();
		return -1;
	}
	seek = seekname(uident);
	if (seek && !ischange) {
		move(1, 0);
		prints(" %s 已经在封禁名单中。", uident);
		egetch();
		return -1;
	}
	if (ischange && !seek) {
		move(1, 0);
		prints(" %s 不在封禁名单中。", uident);
		egetch();
		return -1;
	}
	if (!ischange)
		setreasondefault();
	changereason("etc/denyboard");
	getdata(1, 0, "输入补充说明:", buf, 50, DOECHO, YEA);
	getdata(1, 0, "输入天数(默认1天):", buf2, 4, DOECHO, YEA);
	day=atoi(buf2);
	day=(day>90) ? 90 : day;
	day=(day<1) ? 1 : day;
	move(1, 0);
	if (ischange) {
		sprintf(strtosave, "改变对%s的封禁吗?", uident);
	} else {
		sprintf(strtosave, "真的封禁%s吗?", uident);
	}
	if (askyn(strtosave, NA, NA)==NA)
		return -1;
	nowtime=time(0);
	struct tm* tmtime;
	time_t daytime=nowtime+(day)*24*60*60;
	tmtime=localtime(&daytime);
	sprintf(strtosave, "%-12s %-31s %2d天 %02d年%02d月%02d日解 %-12s", uident,
			reason, day, tmtime->tm_year%100, tmtime->tm_mon+1,
			tmtime->tm_mday, currentuser.userid);
	sprintf(
			msg,
			"%s因:\n%s\n应被封禁%s版发文权限%d天\n请在处罚期满后(%04d.%02d.%02d), 向%s写信要求解除处罚.\n如不服本决定, 可以联系处罚决定人或在7日内到Appeal申请复议。\nP.S.: %s\n执行人: %s\n",
			uident, getdetailreason(), currboard, day, 1900
					+tmtime->tm_year, tmtime->tm_mon+1, tmtime->tm_mday,
			currentuser.userid, buf, currentuser.userid);
	if (ischange)
		deldeny(uident);
	setbfile(genbuf, currboard, "deny_users");
	bm_log(currentuser.userid, currboard, BMLOG_DENYPOST, 1);
	return add_to_file(genbuf, strtosave);
}

int deldeny(uident)
char *uident;
{
	char fn[STRLEN];
	setbfile(fn, currboard, "deny_users");
	bm_log(currentuser.userid, currboard, BMLOG_UNDENY, 1);
	return del_from_file(fn, uident);
}

void deny_title_show() {
	move(0, 0);
	prints("[1;44;36m 设定无法发文的名单                                                            [m\n");
	prints(" 离开[[1;32m←[m] 选择[[1;32m↑[m,[1;32m↓[m] 添加[[1;32ma[m] 修改[[1;32mc[m] 解封[[1;32md[m] 查找[[1;32m/[m]\n");
	prints("[1;44m 用户代号     封禁原因(A-Z,;'[])              天数    解封日期       版主      [m\n");
}

int deny_key_deal(char* fname, int ch, char* line) {
	char msgbuf[4096];
	char repbuf[500];
	int tmp;
	if (line) {
		strncpy(deny_uid, line, IDLEN);
		deny_uid[IDLEN] = '\0';
		strtok(deny_uid, " \n\r\t");
		tmp=strlen(reasonlist);
		strncpy(reason, line+IDLEN+1, tmp);
		reason[tmp]='\0';
		while (tmp--) {
			if (reason[tmp]!=reasonlist[tmp])
				reason[tmp]='-';
		}
	}
	switch (ch) {
		case 'a': //封人
			move(1, 0);
			usercomplete("封禁使用者: ", deny_uid);
			if (*deny_uid!='\0' && getuser(deny_uid)) {
				if (!strcmp(deny_uid, currentuser.userid)) {
					move(1, 0);
					prints("ft! 封自己玩!!!??? NO WAY! :P");
					egetch();
					break;
				}
				if (addtodeny(deny_uid, msgbuf, 0)==1) {
					sprintf(repbuf, "封禁%s在%s版的发文权限", deny_uid, currboard);
					//autoreport(repbuf,msgbuf,YEA,deny_uid); //infotech
					autoreport(repbuf, msgbuf, YEA, deny_uid, 0);
					Poststring(msgbuf, "Notice", repbuf, 1);
					sprintf(repbuf, "被%s封禁在%s版的发文权限", currentuser.userid,
							currboard);
					log_DOTFILE(deny_uid, repbuf);
				}
			}
			break;
		case 'd': //解封
			if (!line)
				return 0;
			move(1, 0);
			sprintf(msgbuf, "解除%s的封禁吗?", deny_uid);
			if (askyn(msgbuf, NA, NA)==NA)
				return 1;
			if (deldeny(deny_uid)) {
				sprintf(repbuf, "恢复%s在%s版的发文权限", deny_uid, currboard);
				sprintf(msgbuf, "%s恢复%s在%s版发文权限.\n", currentuser.userid,
						deny_uid, currboard);
				autoreport(repbuf, msgbuf, YEA, deny_uid, 0);
				//autoreport(repbuf,msgbuf,YEA,deny_uid);
				Poststring(msgbuf, "Notice", repbuf, 1);
				sprintf(repbuf, "被%s恢复在%s版的发文权限", currentuser.userid,
						currboard);
				log_DOTFILE(deny_uid, repbuf);
			}
			break;
		case 'c': //修改日期
			if (!line)
				return 0;
			if (addtodeny(deny_uid, msgbuf, 1)==1) {
				sprintf(repbuf, "修改%s在%s版的封禁时间或说明", deny_uid, currboard);
				autoreport(repbuf, msgbuf, YEA, deny_uid, 0);
				//autoreport(repbuf,msgbuf,YEA,deny_uid);
				Poststring(msgbuf, "Notice", repbuf, 1);
			}
			break;
		case Ctrl('A'):
		case KEY_RIGHT: //用户信息
			if (!line)
				return 0;
			t_query(deny_uid);
			break;
	}
	return 1;
}

int deny_user() {
	if (!chk_currBM(currBM, 0))
		return DONOTHING;
	setbfile(genbuf, currboard, "deny_users");
	list_text(genbuf, deny_title_show, deny_key_deal, NULL);
	return FULLUPDATE;
}

int isclubmember(char *member, char *board) {
	FILE* fp;
	char uident[IDLEN+1];
	char fname[STRLEN];
	char line[256];
	setbfile(fname, board, "club_users");
	if (!(fp=fopen(fname, "r")))
		return 0;
	while (fgets(line, 256, fp)) {
		strncpy(uident, line, IDLEN);
		uident[IDLEN] = '\0';
		strtok(uident, " \r\n\t");
		if (strcasecmp(member, uident)==0)
			return 1;
	}
	fclose(fp);
	return 0;
}

int addtoclub(char *uident, char *msg) {
	char strtosave[512], buf[50];
	int seek;
	if (!strcmp(uident, "guest")) {
		move(t_lines-1, 0);
		prints("不能邀请guest加入俱乐部");
		egetch();
		return -1;
	}
	seek = isclubmember(uident, currboard);
	if (seek) {
		move(1, 0);
		prints(" %s 已经在俱乐部名单中。", uident);
		egetch();
		return -1;
	}
	getdata(1, 0, "输入补充说明:", buf, 50, DOECHO, YEA);
	move(1, 0);
	sprintf(strtosave, "邀请%s加入俱乐部吗?", uident);
	if (askyn(strtosave, YEA, NA)==NA)
		return -1;
	time_t daytime= time(0);
	struct tm* tmtime=localtime(&daytime);
	sprintf(strtosave, "%-12s %-40s %04d.%02d.%02d %-12s", uident, buf,
			1900+tmtime->tm_year, tmtime->tm_mon+1, tmtime->tm_mday,
			currentuser.userid);

	sprintf(msg, "%s:\n\n    您被邀请加入俱乐部版 %s\n\n补充说明：%s\n\n邀请人: %s\n",
			uident, currboard, buf, currentuser.userid);
	setbfile(genbuf, currboard, "club_users");
	bm_log(currentuser.userid, currboard, BMLOG_ADDCLUB, 1);
	return add_to_file(genbuf, strtosave);
}

int delclub(char *uident) {
	char fn[STRLEN];
	setbfile(fn, currboard, "club_users");
	bm_log(currentuser.userid, currboard, BMLOG_DELCLUB, 1);
	return del_from_file(fn, uident);
}

void club_title_show() {
	move(0, 0);
	prints("[1;44;36m 设定俱乐部的名单                                                               [m\n");
	prints("离开[[1;32m←[m] 选择[[1;32m↑[m,[1;32m↓[m] 添加[[1;32ma[m] 删除[[1;32md[m] 查找[[1;32m/[m]\n");
	prints("[1;44m用户代号               附加说明                         邀请日期     邀请人     [m\n");
}

int club_key_deal(char* fname, int ch, char* line) {
	char msgbuf[4096];
	char repbuf[500];
	if (line) {
		strncpy(club_uid, line, IDLEN);
		club_uid[IDLEN] = '\0';
		strtok(club_uid, " \n\r\t");
	}

	switch (ch) {
		case 'a': //增加
			move(1, 0);
			usercomplete("增加俱乐部成员: ", club_uid);
			if (*club_uid!='\0' && getuser(club_uid)) {
				if (addtoclub(club_uid, msgbuf)==1) {
					sprintf(repbuf, "%s邀请%s加入俱乐部版%s", currentuser.userid,
							club_uid, currboard);
					autoreport(repbuf, msgbuf, YEA, club_uid, 2);
					Poststring(msgbuf, "club", repbuf, 2);
					log_DOTFILE(club_uid, repbuf);
				}
			}
			break;
		case 'd': //删除成员
			if (!line)
				return 0;
			move(1, 0);
			sprintf(msgbuf, "删除俱乐部成员%s吗?", club_uid);
			if (askyn(msgbuf, NA, NA)==NA)
				return 1;
			if (delclub(club_uid)) {
				sprintf(repbuf, "%s取消%s在俱乐部版%s的权利", currentuser.userid,
						club_uid, currboard);
				sprintf(msgbuf, "");
				autoreport(repbuf, msgbuf, YEA, club_uid, 2);
				Poststring(msgbuf, "club", repbuf, 2);
				log_DOTFILE(club_uid, repbuf);
			}
			break;

		case Ctrl('A'):
		case KEY_RIGHT: //用户信息
			if (!line)
				return 0;
			t_query(club_uid);
			break;
			/*		case 'c':    //清空成员，没法给所有成员发信
			 move(1,0);
			 sprintf(msgbuf,"[1;31m清空[m俱乐部成员吗?");
			 if(askyn(msgbuf,NA,NA)==NA)return 1;
			 setbfile( genbuf, currboard,"club_users" );
			 sprintf(msgbuf,"%s 清空俱乐部 %s 的成员",  currentuser.userid, currboard);
			 Poststring(msgbuf,club",repbuf,2);
			 unlink(genbuf);
			 break;
			 */

	}
	return 1;
}

int club_user() {
	struct boardheader *bp;
	extern struct boardheader *getbcache();
	bp = getbcache(currboard);

	if ((bp->flag & BOARD_CLUB_FLAG) && chk_currBM(currBM, 1)) {
		setbfile(genbuf, currboard, "club_users");
		list_text(genbuf, club_title_show, club_key_deal, NULL);
		return FULLUPDATE;
	} else
		return DONOTHING;
}

int bm_log(char *id, char *boardname, int type, int value) {
	int fd, data[BMLOGLEN];
	struct flock ldata;
	struct stat buf;
	struct boardheader *btemp;
	char direct[STRLEN], BM[BM_LEN];
	char *ptr;

	btemp = getbcache(boardname);
	if (btemp == NULL)
		return 0;
	strncpy(BM, btemp->BM, sizeof(BM) - 1);
	BM[sizeof(BM) - 1] = '\0';
	ptr = strtok(BM, ",: ;|&()\0\n");
	while (ptr) {
		if (!strcmp(ptr, currentuser.userid))
			break;
		ptr = strtok(NULL, ",: ;|&()\0\n");
	}
	if (!ptr)
		return 0;
	sprintf(direct, "boards/%s/.bm.%s", boardname, id);
	if ((fd = open(direct, O_RDWR | O_CREAT, 0644)) == -1)
		return 0;
	ldata.l_type = F_RDLCK;
	ldata.l_whence = 0;
	ldata.l_len = 0;
	ldata.l_start = 0;
	if (fcntl(fd, F_SETLKW, &ldata) == -1) {
		close(fd);
		return 0;
	}
	fstat(fd, &buf);
	if (buf.st_size < BMLOGLEN * sizeof(int)) {
		memset(data, 0, sizeof(int) * BMLOGLEN);
	} else {
		read(fd, data, sizeof(int) * BMLOGLEN);
	}
	if (type >= 0 && type < BMLOGLEN)
		data[type] += value;
	lseek(fd, 0, SEEK_SET);
	write(fd, data, sizeof(int) * BMLOGLEN);
	ldata.l_type = F_UNLCK;
	fcntl(fd, F_SETLKW, &ldata);
	close(fd);
	return 0;
}

