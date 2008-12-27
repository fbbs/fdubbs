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
 $Id: vote.c 317 2006-10-27 13:20:07Z danielfree $
 */

#include "bbs.h"
#include "vote.h"

extern cmpbnames();
extern int page, range;
extern struct boardheader *bcache;
extern struct BCACHE *brdshm;
static char *vote_type[] = { "是非", "单选", "复选", "数字", "问答" };
struct votebal currvote; //当前投票
char controlfile[STRLEN];
unsigned int result[33]; //投票结果数组
int vnum;
int voted_flag;
FILE *sug; //投票结果的文件指针
int makevote(struct votebal *ball, char *bname); //设置投票箱

//Added by IAMFAT 2002.06.13
extern void ellipsis(char *str, int len); //加省略号

//Added End
//commented by jacobson

//本文件主要处理投票功能

//比较字符串userid和投票者uv 
//userid:用户名 uv:投票者 
//返回值:0不等， 1相等
int cmpvuid(char *userid, struct ballot *uv) {
	return !strcmp(userid, uv->uid);
}

//设置版面投票的标志,           
//bname:版面名,flag版面标志
//1:开启投票,0:关闭投票 返回值:无..
int setvoteflag(char *bname, int flag) {
	int pos;
	struct boardheader fh;

	pos = search_record(BOARDS, &fh, sizeof(fh), cmpbnames, bname);
	if (flag == 0)
		fh.flag = fh.flag & ~BOARD_VOTE_FLAG;
	else
		fh.flag = fh.flag | BOARD_VOTE_FLAG;
	if (substitute_record(BOARDS, &fh, sizeof(fh), pos) == -1)
		prints("Error updating BOARDS file...\n");
}

//显示bug报告(目前好像没有实现)
//str:错误信息字符串
void b_report(char *str) {
	char buf[STRLEN];

	sprintf(buf, "%s %s", currboard, str);
	report(buf);
}

//建立目录,目录为 vote/版名,权限为755
//bname:版面名字
void makevdir(char *bname) {
	struct stat st;
	char buf[STRLEN];

	sprintf(buf, "vote/%s", bname);
	if (stat(buf, &st) != 0)
		mkdir(buf, 0755);
}

//设置文件名
//bname：版面名
//filename:文件名
//buf:返回的文件名
void setvfile(char *buf, char *bname, char *filename) {
	sprintf(buf, "vote/%s/%s", bname, filename);
}

//设置控制controlfile文件名为 vote\版面名\control
void setcontrolfile() {
	setvfile(controlfile, currboard, "control");
}

//编辑或删除版面备忘录
//返回值:FULLUPDATE
#ifdef ENABLE_PREFIX
int b_notes_edit()
{
	char buf[STRLEN], buf2[STRLEN];
	char ans[4];
	int aborted;
	int notetype;

	if (!chk_currBM(currBM, 0)) { //检查是否版主
		return 0;
	}
	clear();
	move(0, 0);
	prints("设定：\n\n  (1)一般备忘录\n  (2)秘密备忘录\n");
	prints("  (3)版面前缀表\n  (4)是否强制使用前缀\n");
	while (1) {
		getdata(7, 0,"当前选择[1](0~4): ", ans, 2, DOECHO, YEA);
		if (ans[0] == '0')
		return FULLUPDATE;
		if (ans[0] == '\0')
		strcpy(ans, "1");
		if (ans[0] >= '1' && ans[0] <= '4' )
		break;
	}
	makevdir(currboard); //建立备忘录目录
	notetype = ans[0] - '0';
	if (notetype == 2) {
		setvfile(buf, currboard, "secnotes");
	} else if (notetype == 3) {
		setvfile(buf, currboard, "prefix");
	} else if (notetype == 1) {
		setvfile(buf, currboard, "notes");
	} else if (notetype == 4 ) {
		int pos;
		struct boardheader fh;
		pos = search_record(BOARDS, &fh, sizeof(fh), cmpbnames, currboard);

		if (askyn("是否强制使用前缀？", (fh.flag & BOARD_PREFIX_FLAG)?YEA:NA,NA)) {
			fh.flag |= BOARD_PREFIX_FLAG;
		} else {
			fh.flag &= ~BOARD_PREFIX_FLAG;
		}
		substitute_record(BOARDS, &fh, sizeof(fh), pos);
		return FULLUPDATE;
	}
	sprintf(buf2, "(E)编辑 (D)删除 %4s? [E]: ",
			(notetype == 3)?"版面前缀表":(notetype == 1) ? "一般备忘录" : "秘密备忘录");
	getdata(8, 0, buf2, ans, 2, DOECHO, YEA); //询问编辑或者删除
	if (ans[0] == 'D' || ans[0] == 'd') { //删除备忘录
		move(9, 0);
		sprintf(buf2, "真的要删除么？");
		if (askyn(buf2, NA, NA)) {
			move(10, 0);
			prints("已经删除...\n");
			pressanykey();
			unlink(buf);
			aborted = 1;
		} else
		aborted = -1;
	} else
	aborted = vedit(buf, NA, YEA); //编辑备忘录
	if (aborted == -1) {
		pressreturn();
	} else {
		if (notetype == 1)
		setvfile(buf, currboard, "noterec");
		else
		setvfile(buf, currboard, "notespasswd");
		unlink(buf);
	}

	return FULLUPDATE;
}
#else
int b_notes_edit() {
	char buf[STRLEN], buf2[STRLEN];
	char ans[4];
	int aborted;
	int notetype;
	if (!chk_currBM(currBM, 0)) { //检查是否版主
		return 0;
	}
	clear();
	move(1, 0);
	prints("编辑/删除备忘录"); //询问编辑哪种备忘录
	while (1) {
		getdata(3, 0, "编辑或删除本讨论区的 (0) 离开  (1) 一般备忘录  (2) 秘密备忘录? [1] ",
				ans, 2, DOECHO, YEA);
		if (ans[0] == '0')
			return FULLUPDATE;
		if (ans[0] == '\0')
			strcpy(ans, "1");
		if (ans[0] == '1' || ans[0] == '2')
			break;
	}
	makevdir(currboard); //建立备忘录目录
	if (ans[0] == '2') {
		setvfile(buf, currboard, "secnotes");
		notetype = 2;
	} else {
		setvfile(buf, currboard, "notes");
		notetype = 1;
	}
	sprintf(buf2, "(E)编辑 (D)删除 %4s备忘录? [E]: ", (notetype == 1) ? "一般"
			: "秘密");
	getdata(5, 0, buf2, ans, 2, DOECHO, YEA); //询问编辑或者删除
	if (ans[0] == 'D' || ans[0] == 'd') { //删除备忘录
		move(6, 0);
		sprintf(buf2, "真的要删除%4s备忘录", (notetype == 1) ? "一般" : "秘密");
		if (askyn(buf2, NA, NA)) {
			move(7, 0);
			prints("备忘录已经删除...\n");
			pressanykey();
			unlink(buf);
			aborted = 1;
		} else
			aborted = -1;
	} else
		aborted = vedit(buf, NA, YEA); //编辑备忘录
	if (aborted == -1) {
		pressreturn();
	} else {
		if (notetype == 1)
			setvfile(buf, currboard, "noterec");
		else
			setvfile(buf, currboard, "notespasswd");
		unlink(buf);
	}

	return FULLUPDATE;
}
#endif 
//设置秘密备忘录密码
int b_notes_passwd() {
	FILE *pass;
	char passbuf[20], prepass[20];
	char buf[STRLEN];

	if (!chk_currBM(currBM, 0)) { //检查是否版主
		return 0;
	}
	clear();
	move(1, 0);
	prints("设定/更改/取消「秘密备忘录」密码...");
	setvfile(buf, currboard, "secnotes");
	if (!dashf(buf)) {
		move(3, 0);
		prints("本讨论区尚无「秘密备忘录」。\n\n");
		prints("请先用 W 编好「秘密备忘录」再来设定密码...");
		pressanykey();
		return FULLUPDATE;
	}
	if (!check_notespasswd())
		return FULLUPDATE;
	getdata(3, 0, "请输入新的秘密备忘录密码(Enter 取消密码): ", passbuf, 19, NOECHO, YEA);
	if (passbuf[0] == '\0') {
		setvfile(buf, currboard, "notespasswd");
		unlink(buf);
		prints("已经取消备忘录密码。");
		pressanykey();
		return FULLUPDATE;
	}
	getdata(4, 0, "确认新的秘密备忘录密码: ", prepass, 19, NOECHO, YEA);
	if (strcmp(passbuf, prepass)) {
		prints("\n密码不相符, 无法设定或更改....");
		pressanykey();
		return FULLUPDATE;
	}
	setvfile(buf, currboard, "notespasswd");
	if ((pass = fopen(buf, "w")) == NULL) {
		move(5, 0);
		prints("备忘录密码无法设定....");
		pressanykey();
		return FULLUPDATE;
	}
	fprintf(pass, "%s\n", genpasswd(passbuf));
	fclose(pass);
	move(5, 0);
	prints("秘密备忘录密码设定完成...");
	pressanykey();
	return FULLUPDATE;
}

//将一个文件全部内容写入已经打开的另一个文件
//fp: 已经打开的文件指针,（被写入文件）
//fname: 需要写入的文件的路径
int b_suckinfile(FILE * fp, char *fname) {
	char inbuf[256];
	FILE *sfp;

	if ((sfp = fopen(fname, "r")) == NULL)
		return -1;
	while (fgets(inbuf, sizeof(inbuf), sfp) != NULL)
		fputs(inbuf, fp);
	fclose(sfp);
	return 0;
}

//将一个文件全部内容写入已经打开的另一个文件,(用于读留言板)
//如果不能打开写入一条横线
//fp: 已经打开的文件指针,（被写入文件）
//fname: 需要写入的文件的路径
/*Add by SmallPig*/
int catnotepad(fp, fname)
FILE *fp;
char *fname;
{
	char inbuf[256];
	FILE *sfp;
	int count;

	count = 0;
	if ((sfp = fopen(fname, "r")) == NULL) {
		fprintf(fp,
				"[1;34m  □[44m__________________________________________________________________________[m \n\n");
		return -1;
	}
	while (fgets(inbuf, sizeof(inbuf), sfp) != NULL) {
		if (count != 0)
		fputs(inbuf, fp);
		else
		count++;
	}
	fclose(sfp);
	return 0;
}

//关闭投票
//返回值:固定为0
int b_closepolls() {
	char buf[80];
	time_t now, nextpoll;
	int i, end;

	now = time(0);
	resolve_boards();

	if (now < brdshm->pollvote) { //现在时间小于下次可投票时间则返回？
		return;
	}
	//关闭显示 函数调用移到miscd
	/*
	 move(t_lines - 1, 0);
	 prints("对不起，系统关闭投票中，请稍候...");
	 refresh();
	 */

	nextpoll = now + 7 * 3600;

	strcpy(buf, currboard);
	for (i = 0; i < brdshm->number; i++) {
		strcpy(currboard, (&bcache[i])->filename);
		setcontrolfile();
		end = get_num_records(controlfile, sizeof(currvote));
		for (vnum = end; vnum >= 1; vnum--) {
			time_t closetime;

			get_record(controlfile, &currvote, sizeof(currvote), vnum);
			closetime = currvote.opendate + currvote.maxdays * 86400;
			if (now > closetime)
				mk_result(vnum); //若投票期限已过写入投票结果
			else if (nextpoll > closetime)
				nextpoll = closetime + 300;
		}
	}
	strcpy(currboard, buf);

	brdshm->pollvote = nextpoll; //下次可投票时间？
	return 0;
}

//计算一次的投票结果,并放入result数组中,用于mk_result中的apply_record函数中的回调函数 -.-!
//result[32]记录调用次数
//参数ptr:一次的投票结果
//返回值:固定为0
int count_result(struct ballot *ptr) {
	int i;

	/*	if (ptr == NULL) {
	 if (sug != NULL) {
	 fclose(sug);
	 sug == NULL;
	 }
	 return 0;
	 }
	 */if (ptr->msg[0][0] != '\0') {
		if (currvote.type == VOTE_ASKING) {
			fprintf(sug, "[1m%s [m的作答如下：\n", ptr->uid);
		} else
			fprintf(sug, "[1m%s [m的建议如下：\n", ptr->uid);
		for (i = 0; i < 3; i++)
			fprintf(sug, "%s\n", ptr->msg[i]);
	}
	result[32]++;
	if (currvote.type == VOTE_ASKING) {
		return 0;
	}
	if (currvote.type != VOTE_VALUE) {
		for (i = 0; i < 32; i++) {
			if ((ptr->voted >> i) & 1)
				(result[i])++;
		}

	} else {
		result[31] += ptr->voted;
		result[(ptr->voted * 10) / (currvote.maxtkt + 1)]++;
	}
	return 0;
}

//将投票的抬头写入sug投票结果文件
get_result_title() {
	char buf[STRLEN];

	getdatestring(currvote.opendate, NA);
	fprintf(sug, "⊙ 投票开启於：[1m%s[m  类别：[1m%s[m\n", datestring,
			vote_type[currvote.type - 1]);
	fprintf(sug, "⊙ 主题：[1m%s[m\n", currvote.title);
	if (currvote.type == VOTE_VALUE)
		fprintf(sug, "⊙ 此次投票的值不可超过：[1m%d[m\n\n", currvote.maxtkt);
	fprintf(sug, "⊙ 票选题目描述：\n\n");
	sprintf(buf, "vote/%s/desc.%d", currboard, currvote.opendate);
	b_suckinfile(sug, buf);
}

//结束投票,计算投票结果
//num:投票control文件中第几个记录
int mk_result(int num) {
	char fname[STRLEN], nname[STRLEN];
	char sugname[STRLEN];
	char title[STRLEN];
	int i, j;
	unsigned int total = 0;
    unsigned int sorted_index[33];

	setcontrolfile();
	sprintf(fname, "vote/%s/flag.%d", currboard, currvote.opendate); //投票记录文件路径为 vote/版名/flag.开启投票日
	/*	count_result(NULL); */
	sug = NULL;
	sprintf(sugname, "vote/%s/tmp.%d", currboard, uinfo.pid); //投票临时文件路径为 vote/版名/tmp.用户id
	if ((sug = fopen(sugname, "w")) == NULL) {
		report("open vote tmp file error");
		//prints("Error: 结束投票错误...\n");
		pressanykey();
	}
	(void) memset(result, 0, sizeof(result));
    for (i = 0; i < 33; i++) sorted_index[i] = i;
	if (apply_record(fname, count_result, sizeof(struct ballot), 0, 0, 0)
			== -1) {
		report("Vote apply flag error");
	}
	fprintf(sug, "[1;44;36m——————————————┤使用者%s├——————————————[m\n\n\n",
			(currvote.type != VOTE_ASKING) ? "建议或意见" : "此次的作答");
	fclose(sug);
	sprintf(nname, "vote/%s/results", currboard); //投票结果文件路径为 vote/版名/results
	if ((sug = fopen(nname, "w")) == NULL) {
		report("open vote newresult file error");
		//prints("Error: 结束投票错误...\n");
	}
	get_result_title(sug);
	//计算投票结果
	fprintf(sug, "** 投票结果:\n\n");
	if (currvote.type == VOTE_VALUE) {
        /* Sorting vote result. Bubble sort. */
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 10 - i - 1; j++) {
                if (result[sorted_index[j]] < result[sorted_index[j+1]]) {
                    total = sorted_index[j];
                    sorted_index[j] = sorted_index[j+1];
                    sorted_index[j+1] = total;
                }
            }
        }
        /* Output vote results.*/
		total = result[32];
		for (i = 0; i < 10; i++) {
			fprintf(
					sug,
					"[1m  %4d[m 到 [1m%4d[m 之间有 [1m%4d[m 票  约占 [1m%d%%[m\n",
					(sorted_index[i] * currvote.maxtkt) / 10 + ((sorted_index[i] == 0) ? 0 : 1), ((sorted_index[i]
							+ 1) * currvote.maxtkt) / 10, result[sorted_index[i]],
					(result[sorted_index[i]] * 100) / ((total <= 0) ? 1 : total));
		}
		fprintf(sug, "此次投票结果平均值是: [1m%d[m\n", result[31]
				/ ((total <= 0) ? 1 : total));
	} else if (currvote.type == VOTE_ASKING) {
		total = result[32];
	} else {
        /* Sorting vote result. Bubble sort. */
        for (i = 0; i < currvote.totalitems; i++) {
            for (j = 0; j < currvote.totalitems - i - 1; j++) {
                if (result[sorted_index[j]] < result[sorted_index[j+1]]) {
                    total = sorted_index[j];
                    sorted_index[j] = sorted_index[j+1];
                    sorted_index[j+1] = total;
                }
            }
        }
        total = 0;
		for (i = 0; i < currvote.totalitems; i++) {
			total += result[i];
		}
		for (i = 0; i < currvote.totalitems; i++) {
			fprintf(sug, "(%c) %-40s  %4d 票  约占 [1m%d%%[m\n", 'A' + i,
					currvote.items[sorted_index[i]], result[sorted_index[i]], (result[sorted_index[i]] * 100)
							/ ((total <= 0) ? 1 : total));
		}
	}
	fprintf(sug, "\n投票总人数 = [1m%d[m 人\n", result[32]);
	fprintf(sug, "投票总票数 =[1m %d[m 票\n\n", total);
	fprintf(sug, "[1;44;36m——————————————┤使用者%s├——————————————[m\n\n\n",
			(currvote.type != VOTE_ASKING) ? "建议或意见" : "此次的作答");
	b_suckinfile(sug, sugname);
	unlink(sugname); //删除投票临时文件,并将投票文件写入sug投票结果文件
	fclose(sug);

	sprintf(title, "[公告] %s 版的投票结果", currboard);
	Postfile(nname, "vote", title, 1); //投票结果贴入vote版
	if (currboard != "vote")
		Postfile(nname, currboard, title, 1); //投票结果贴入当前版
	dele_vote(num); //关闭投票,删除临时文件
	return;
}

//取得选择题可选项目,放入bal中
//返回值 num：可选项目数
int get_vitems(struct votebal *bal) {
	int num;
	char buf[STRLEN];

	move(3, 0);
	prints("请依序输入可选择项, 按 ENTER 完成设定.\n");
	num = 0;
	for (num = 0; num < 32; num++) {
		sprintf(buf, "%c) ", num + 'A');
		getdata((num % 16) + 4, (num / 16) * 40, buf, bal->items[num], 36,
				DOECHO, YEA);
		if (strlen(bal->items[num]) == 0) {
			if (num != 0)
				break;
			num = -1;
		}
	}
	bal->totalitems = num;
	return num;
}

//开启投票箱并设置投票箱
//bname:版名
//返回值:固定为 FULLUPDATE
int vote_maintain(char *bname) {
	char buf[STRLEN];
	struct votebal *ball = &currvote;

	setcontrolfile();
	if (!chk_currBM(currBM, 0)) {
		return 0;
	}
	stand_title("开启投票箱");
	makevdir(bname);
	for (;;) {
		getdata(2, 0, "(1)是非, (2)单选, (3)复选, (4)数值 (5)问答 (6)取消 ? : ",
				genbuf, 2, DOECHO, YEA);
		genbuf[0] -= '0';
		if (genbuf[0] < 1 || genbuf[0] > 5) {
			prints("取消此次投票\n");
			return FULLUPDATE;
		}
		ball->type = (int) genbuf[0];
		break;
	}
	ball->opendate = time(NULL);
	if (makevote(ball, bname))
		return FULLUPDATE; //设置投票箱
	setvoteflag(currboard, 1);
	clear();
	strcpy(ball->userid, currentuser.userid);
	if (append_record(controlfile, ball, sizeof(*ball)) == -1) {
		prints("发生严重的错误，无法开启投票，请通告站长");
		b_report("Append Control file Error!!");
	} else {
		char votename[STRLEN];
		int i;

		b_report("OPEN");
		prints("投票箱开启了！\n");
		range++;;
		sprintf(votename, "tmp/votetmp.%s.%05d", currentuser.userid,
				uinfo.pid);
		if ((sug = fopen(votename, "w")) != NULL) {
			//Modified by IAMFAT 2002.06.13
			//sprintf(buf, "[通知] %s 举办投票：%s", currboard, ball->title);
			strcpy(genbuf, ball->title);
			ellipsis(genbuf, 31 - strlen(currboard));
			sprintf(buf, "[通知] %s 举办投票: %s", currboard, ball->title);
			get_result_title(sug);
			if (ball->type != VOTE_ASKING && ball->type != VOTE_VALUE) {
				fprintf(sug, "\n【[1m选项如下[m】\n");
				for (i = 0; i < ball->totalitems; i++) {
					fprintf(sug, "([1m%c[m) %-40s\n", 'A' + i,
							ball->items[i]);
				}
			}
			fclose(sug);
			Postfile(votename, "vote", buf, 1);
			unlink(votename);
		}
	}
	pressreturn();
	return FULLUPDATE;
}

//设置投票箱
//ball: 投票箱
//bname：版名
//返回值0： 正常退出 1：用户取消
int makevote(struct votebal *ball, char *bname) {
	char buf[STRLEN];
	int aborted;

	prints("请按任何键开始编辑此次 [投票的描述]: \n");
	igetkey();
	setvfile(genbuf, bname, "desc");
	sprintf(buf, "%s.%d", genbuf, ball->opendate);
	aborted = vedit(buf, NA, YEA);
	if (aborted) {
		clear();
		prints("取消此次投票设定\n");
		pressreturn();
		return 1;
	}
	clear();
	getdata(0, 0, "此次投票所须天数 (不可０天): ", buf, 3, DOECHO, YEA);

	if (*buf == '\n' || atoi(buf) == 0 || *buf == '\0')
		strcpy(buf, "1");

	ball->maxdays = atoi(buf);
	for (;;) {
		//Modified by IAMFAT 2002.06.13
		//getdata(1, 0, "投票箱的标题: ", ball->title, 61, DOECHO, YEA);
		getdata(1, 0, "投票箱的标题: ", ball->title, 50, DOECHO, YEA);
		if (strlen(ball->title) > 0)
			break;
		bell();
	}
	switch (ball->type) {
		case VOTE_YN:
			ball->maxtkt = 0;
			strcpy(ball->items[0], "赞成  （是的）");
			strcpy(ball->items[1], "不赞成（不是）");
			strcpy(ball->items[2], "没意见（不清楚）");
			ball->maxtkt = 1;
			ball->totalitems = 3;
			break;
		case VOTE_SINGLE:
			get_vitems(ball);
			ball->maxtkt = 1;
			break;
		case VOTE_MULTI:
			get_vitems(ball);
			for (;;) {
				getdata(21, 0, "一个人最多几票? [1]: ", buf, 5, DOECHO, YEA);
				ball->maxtkt = atoi(buf);
				if (ball->maxtkt <= 0)
					ball->maxtkt = 1;
				if (ball->maxtkt > ball->totalitems)
					continue;
				break;
			}
			break;
		case VOTE_VALUE:
			for (;;) {
				getdata(3, 0, "输入数值最大不得超过 [100] : ", buf, 4, DOECHO, YEA);
				ball->maxtkt = atoi(buf);
				if (ball->maxtkt <= 0)
					ball->maxtkt = 100;
				break;
			}
			break;
		case VOTE_ASKING:
			/*                    getdata(3,0,"此问答题作答行数之限制 :",buf,3,DOECHO,YEA) ;
			 ball->maxtkt = atof(buf) ;
			 if(ball->maxtkt <= 0) ball->maxtkt = 10;*/
			ball->maxtkt = 0;
			currvote.totalitems = 0;
			break;
		default:
			ball->maxtkt = 1;
			break;
	}
	return 0;
}

// 检查是否读过新的备忘录或者进站welcome 或者写入
// bname:版名, mode =2时设为NULL
// val:  0：检查模式    不等于0:写入模式
// mode: 1:检查备忘录   2:检查进站Welcome
// 返回值 0:未读 1:已读
int vote_flag(char *bname, char val, int mode) {
	char buf[STRLEN], flag;
	int fd, num, size;

	num = usernum - 1;

	switch (mode) {
		case 2:
			sprintf(buf, "Welcome.rec"); /* 进站的 Welcome 画面 */
			break;
		case 1:
			setvfile(buf, bname, "noterec"); /* 讨论区备忘录的旗标 */
			break;
		default:
			return -1;
	}

	if (num >= MAXUSERS) {
		report("Vote Flag, Out of User Numbers");
		return -1;
	}

	if ((fd = open(buf, O_RDWR | O_CREAT, 0600)) == -1) {
		return -1;
	}

	FLOCK(fd, LOCK_EX);
	size = (int) lseek(fd, 0, SEEK_END);
	memset(buf, 0, sizeof(buf));
	while (size <= num) {
		write(fd, buf, sizeof(buf));
		size += sizeof(buf);
	}
	lseek(fd, (off_t) num, SEEK_SET);
	read(fd, &flag, 1); //读是否已经读过的标志flag
	if ((flag == 0 && val != 0)) {
		lseek(fd, (off_t) num, SEEK_SET);
		write(fd, &val, 1);
	}
	FLOCK(fd, LOCK_UN);
	close(fd);

	return flag;
}

//检查投了几票
//bits: 32位的值
//返回值 二进制32位bits中 等于1的位数的数量
int vote_check(int bits) {
	int i, count;

	for (i = count = 0; i < 32; i++) {
		if ((bits >> i) & 1)
			count++;
	}
	return count;
}

//显示用户投过的票，以及可选项
//pbits:票数字段 i:显示位置 flag:是否显示你已经投了几票 YEA:显示 NO:不显示
//返回值:固定为YEA
int showvoteitems(unsigned int pbits, int i, int flag) {
	char buf[STRLEN];
	int count;

	if (flag == YEA) {
		count = vote_check(pbits);
		if (count > currvote.maxtkt)
			return NA;
		move(2, 0);
		clrtoeol();
		prints("您已经投了 [1m%d[m 票", count);
	}
	sprintf(buf, "%c.%2.2s%-36.36s", 'A' + i, ((pbits >> i) & 1 ? "√"
			: "  "), currvote.items[i]);
	move(i + 6 - ((i > 15) ? 16 : 0), 0 + ((i > 15) ? 40 : 0));
	prints(buf);
	refresh();
	return YEA;
}

//显示投票内容
void show_voteing_title() {
	time_t closedate;
	char buf[STRLEN];

	if (currvote.type != VOTE_VALUE && currvote.type != VOTE_ASKING)
		sprintf(buf, "可投票数: [1m%d[m 票", currvote.maxtkt);
	else
		buf[0] = '\0';
	closedate = currvote.opendate + currvote.maxdays * 86400;
	getdatestring(closedate, NA);
	prints("投票将结束於: [1m%s[m  %s  %s\n", datestring, buf,
			(voted_flag) ? "([5;1m修改前次投票[m)" : "");
	prints("投票主题是: [1m%-50s[m类型: [1m%s[m \n", currvote.title,
			vote_type[currvote.type - 1]);
}

//取得提问型投票答案
//uv:用户投票的数据,返回后用户输入的答案放在 uv->msg里,最多3行
//返回值: 用户输入的答案行数
int getsug(struct ballot *uv) {
	int i, line;

	move(0, 0);
	clrtobot();
	if (currvote.type == VOTE_ASKING) {
		show_voteing_title();
		line = 3;
		prints("请填入您的作答(三行):\n");
	} else {
		line = 1;
		prints("请填入您宝贵的意见(三行):\n");
	}
	move(line, 0);
	for (i = 0; i < 3; i++) {
		prints(": %s\n", uv->msg[i]);
	}
	for (i = 0; i < 3; i++) {
		getdata(line + i, 0, ": ", uv->msg[i], STRLEN - 2, DOECHO, NA);
		if (uv->msg[i][0] == '\0')
			break;
	}
	return i;
}

//输入多选/单选/是非的答案
//uv:用户投票的数据,返回后用户输入的答案放在 uv->msg里
//返回值: 成功1 用户取消-1
int multivote(struct ballot *uv) {
	unsigned int i;

	i = uv->voted;
	move(0, 0);
	show_voteing_title();
	uv->voted = setperms(uv->voted, "选票", currvote.totalitems,
			showvoteitems);
	if (uv->voted == i)
		return -1;
	return 1;
}

//输入值型选项的答案
//uv:用户投票的数据,返回后用户输入的答案放在 uv->msg里
//返回值: 成功1 用户取消-1
int valuevote(struct ballot *uv) {
	unsigned int chs;
	char buf[10];

	chs = uv->voted;
	move(0, 0);
	show_voteing_title();
	prints("此次作答的值不能超过 [1m%d[m", currvote.maxtkt);
	if (uv->voted != 0)
		sprintf(buf, "%d", uv->voted);
	else
		memset(buf, 0, sizeof(buf));
	do {
		getdata(3, 0, "请输入一个值? [0]: ", buf, 5, DOECHO, NA);
		uv->voted = abs(atoi(buf));
	} while (uv->voted > currvote.maxtkt && buf[0] != '\n' && buf[0]
			!= '\0');
	if (buf[0] == '\n' || buf[0] == '\0' || uv->voted == chs)
		return -1;
	return 1;
}

//用户进行投票,由vote_key,b_vote函数调用
//num:投票controlfile中第几个记录
//返回值:无
int user_vote(int num) {
	char fname[STRLEN], bname[STRLEN];
	char buf[STRLEN];
	struct ballot uservote, tmpbal;
	int votevalue;
	int aborted = NA, pos;

	move(t_lines - 2, 0);
	get_record(controlfile, &currvote, sizeof(struct votebal), num);
	if (currentuser.firstlogin > currvote.opendate) { //注册日在投票开启日前不能投票
		prints("对不起, 投票名册上找不到您的大名\n");
		pressanykey();
		return;
	}
	sprintf(fname, "vote/%s/flag.%d", currboard, currvote.opendate);
	if ((pos = search_record(fname, &uservote, sizeof(uservote), cmpvuid,
			currentuser.userid)) <= 0) {
		(void) memset(&uservote, 0, sizeof(uservote));
		voted_flag = NA;
	} else {
		voted_flag = YEA;
	}
	strcpy(uservote.uid, currentuser.userid);
	sprintf(bname, "desc.%d", currvote.opendate);
	setvfile(buf, currboard, bname);
	ansimore(buf, YEA);
	move(0, 0);
	clrtobot();
	switch (currvote.type) {
		case VOTE_SINGLE:
		case VOTE_MULTI:
		case VOTE_YN:
			votevalue = multivote(&uservote);
			if (votevalue == -1)
				aborted = YEA;
			break;
		case VOTE_VALUE:
			votevalue = valuevote(&uservote);
			if (votevalue == -1)
				aborted = YEA;
			break;
		case VOTE_ASKING:
			uservote.voted = 0;
			aborted = !getsug(&uservote);
			break;
	}
	clear();
	if (aborted == YEA) {
		prints("保留 【[1m%s[m】原来的的投票。\n", currvote.title);
	} else {
		if (currvote.type != VOTE_ASKING)
			getsug(&uservote);
		pos = search_record(fname, &tmpbal, sizeof(tmpbal), cmpvuid,
				currentuser.userid);
		if (pos) {
			substitute_record(fname, &uservote, sizeof(uservote), pos);
		} else if (append_record(fname, &uservote, sizeof(uservote)) == -1) {
			move(2, 0);
			clrtoeol();
			prints("投票失败! 请通知站长参加那一个选项投票\n");
			pressreturn();
		}
		prints("\n已经帮您投入票箱中...\n");
	}
	pressanykey();
	return;
}

//显示投票箱信息的头部
void voteexp() {
	clrtoeol();
	prints("[1;44m编号 开启投票箱者 开启日 %-39s 类别 天数 人数[m\n", "投票主题");
}

//显示投票箱信息
//ent 投票信息
int printvote(struct votebal *ent) {
	static int i;
	struct ballot uservote;
	char buf[STRLEN + 10];
	char flagname[STRLEN];
	int num_voted;

	//Added by IAMFAT 2002.06.13
	char title[STRLEN];

	//Added End

	if (ent == NULL) {
		move(2, 0);
		voteexp();
		i = 0;
		return 0;
	}
	i++;
	if (i > page + 19 || i > range)
		return QUIT;
	else if (i <= page)
		return 0;
	sprintf(buf, "flag.%d", ent->opendate);
	setvfile(flagname, currboard, buf);
	if (search_record(flagname, &uservote, sizeof(uservote), cmpvuid,
			currentuser.userid) <= 0) {
		voted_flag = NA;
	} else
		voted_flag = YEA;
	num_voted = get_num_records(flagname, sizeof(struct ballot));
	getdatestring(ent->opendate, NA);
	//Modified by IAMFAT 2002.06.13
	/*
	 sprintf(buf, " %s%3d %-12.12s %6.6s %-40.40s%-4.4s %3d  %4d[m\n", (voted_flag == NA) ? "[1m" : "", i, ent->userid,
	 datestring+6, ent->title, vote_type[ent->type - 1], ent->maxdays, num_voted);
	 */
	strcpy(title, ent->title);
	ellipsis(title, 39);
	sprintf(buf, " %s%3d %-12.12s %6.6s %-39.39s %-4.4s %3d  %4d[m\n",
			(voted_flag == NA) ? "[1m" : "", i, ent->userid, datestring
					+ 6, title, vote_type[ent->type - 1], ent->maxdays,
			num_voted);
	//Ended IAMFAT
	prints("%s", buf);
}

//删除投票文件
//num 投票controlfile中第几个记录
//返回值 无
int dele_vote(num)
int num;
{
	char buf[STRLEN];

	sprintf(buf, "vote/%s/flag.%d", currboard, currvote.opendate);
	unlink(buf);
	sprintf(buf, "vote/%s/desc.%d", currboard, currvote.opendate);
	unlink(buf);
	if (delete_record(controlfile, sizeof(currvote), num, NULL, NULL) == -1) {
		prints("发生错误，请通知站长....");
		pressanykey();
	}
	range--;
	if (get_num_records(controlfile, sizeof(currvote)) == 0) {
		setvoteflag(currboard, 0);
	}
}

//显示投票结果
//bname:版名
//返回值:固定为FULLUPDATE
int vote_results(char *bname) {
	char buf[STRLEN];

	setvfile(buf, bname, "results");
	if (ansimore(buf, YEA) == -1) {
		move(3, 0);
		prints("目前没有任何投票的结果。\n");
		clrtobot();
		pressreturn();
	} else
		clear();
	return FULLUPDATE;
}

//开启投票箱并设置投票箱
int b_vote_maintain() {
	return vote_maintain(currboard);
}

//显示投票箱选项
void vote_title() {

	docmdtitle(
			"[投票箱列表]",
			"[[1;32m←[m,[1;32me[m] 离开 [[1;32mh[m] 求助 [[1;32m→[m,[1;32mr <cr>[m] 进行投票 [[1;32m↑[m,[1;32m↓[m] 上,下选择 [1m高亮度[m表示尚未投票");
	update_endline();
}

//根据用户的按键对投票箱进行操作,可以结束/修改/强制关闭/显示投票结果
//ch: 用户的按键
//allnum:投票controlfile的第几个记录
//pagenum:未使用
//返回值 0:失败 1:成功
int vote_key(int ch, int allnum, int pagenum) {
	int deal = 0, ans;
	char buf[STRLEN];

	switch (ch) {
		case 'v':
		case 'V':
		case '\n':
		case '\r':
		case 'r':
		case KEY_RIGHT:
			user_vote(allnum + 1);
			deal = 1;
			break;
		case 'R':
			vote_results(currboard);
			deal = 1;
			break;
		case 'H':
		case 'h':
			show_help("help/votehelp");
			deal = 1;
			break;
		case 'A':
		case 'a':
			if (!chk_currBM(currBM, 0))
				return YEA;
			vote_maintain(currboard);
			deal = 1;
			break;
		case 'O':
		case 'o':
			if (!chk_currBM(currBM, 0))
				return YEA;
			clear();
			deal = 1;
			get_record(controlfile, &currvote, sizeof(struct votebal),
					allnum + 1);
			prints("[5;1;31m警告!![m\n");
			prints("投票箱标题：[1m%s[m\n", currvote.title);
			ans = askyn("您确定要提早结束这个投票吗", NA, NA);

			if (ans != 1) {
				move(2, 0);
				prints("取消删除行动\n");
				pressreturn();
				clear();
				break;
			}
			mk_result(allnum + 1);
			sprintf(buf, "[结束] 提早结束投票 %s", currvote.title);
			securityreport(buf, 0, 4);
			break;
		case 'M':
		case 'm':
			if (!chk_currBM(currBM, 0))
				return YEA;
			clear();
			deal = 1;
			get_record(controlfile, &currvote, sizeof(struct votebal),
					allnum + 1);
			prints("[5;1;31m警告!![m\n");
			prints("投票箱标题：[1m%s[m\n", currvote.title);
			ans = askyn("您确定要修改这个投票的设定吗", NA, NA);

			if (ans != 1) {
				move(2, 0);
				prints("取消修改行动\n");
				pressreturn();
				clear();
				break;
			}
			makevote(&currvote, currboard);
			substitute_record(controlfile, &currvote,
					sizeof(struct votebal), allnum + 1);
			sprintf(buf, "[修改] 修改投票设定 %s", currvote.title);
			securityreport(buf, 0, 4);
			break;
		case 'D':
		case 'd':
			if (!chk_currBM(currBM, 0)) {
				return 1;
			}
			deal = 1;
			get_record(controlfile, &currvote, sizeof(struct votebal),
					allnum + 1);
			clear();
			prints("[5;1;31m警告!![m\n");
			prints("投票箱标题：[1m%s[m\n", currvote.title);
			ans = askyn("您确定要强制关闭这个投票吗", NA, NA);

			if (ans != 1) {
				move(2, 0);
				prints("取消删除行动\n");
				pressreturn();
				clear();
				break;
			}
			sprintf(buf, "[关闭] 强制关闭投票 %s", currvote.title);
			securityreport(buf, 0, 4);
			dele_vote(allnum + 1);
			break;
		default:
			return 0;
	}
	if (deal) {
		Show_Votes();
		vote_title();
	}
	return 1;
}

//显示投票箱信息
int Show_Votes() {

	move(3, 0);
	clrtobot();
	printvote(NULL);
	setcontrolfile();
	if (apply_record(controlfile, printvote, sizeof(struct votebal), 0, 0,
			0) == -1) {
		prints("错误，没有投票箱开启....");
		pressreturn();
		return 0;
	}
	clrtobot();
	return 0;
}

//用户对本版进行投票，bbs.c调用
//返回值:固定为FULLUPDATE
int b_vote() {
	int num_of_vote;
	int voting;

	if (!HAS_PERM(PERM_VOTE) || (currentuser.stay < 1800)) {
		return;
	}
	setcontrolfile();
	num_of_vote = get_num_records(controlfile, sizeof(struct votebal));
	if (num_of_vote == 0) {
		move(2, 0);
		clrtobot();
		prints("\n抱歉, 目前并没有任何投票举行。\n");
		pressreturn();
		setvoteflag(currboard, 0);
		return FULLUPDATE;
	}
	setlistrange(num_of_vote);
	clear();
	voting = choose(NA, 0, vote_title, vote_key, Show_Votes, user_vote); //?
	clear();
	return /* user_vote( currboard ) */FULLUPDATE;
}

//显示投票结果  bbs.c调用
int b_results() {
	return vote_results(currboard);
}

//SYSOP版开启投票箱
int m_vote() {
	char buf[STRLEN];

	strcpy(buf, currboard);
	strcpy(currboard, DEFAULTBOARD);
	modify_user_mode(ADMIN);
	vote_maintain(DEFAULTBOARD);
	strcpy(currboard, buf);
	return;
}

//对SYSOP版进行投票
int x_vote() {
	char buf[STRLEN];

	modify_user_mode(XMENU);
	strcpy(buf, currboard);
	strcpy(currboard, DEFAULTBOARD);
	b_vote();
	strcpy(currboard, buf);
	return;
}

//显示sysop版投票结果
int x_results() {
	modify_user_mode(XMENU); //更改用户 模式状态至??
	return vote_results(DEFAULTBOARD); //显示sysop版投票结果
}
