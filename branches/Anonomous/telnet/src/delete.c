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
 $Id: delete.c 366 2007-05-12 16:35:51Z danielfree $
 */

#include "bbs.h"

#ifdef WITHOUT_ADMIN_TOOLS
#define kick_user
#endif
void mail_info(char *lastword);

int cmpuids3(int unum, struct user_info *urec) {
	return ((unum == urec->uid) && (uinfo.pid != urec->pid));
}

//自杀,详情后叙
int offline() {
	int i;
	char buf[STRLEN], lastword[640];

	modify_user_mode(OFFLINE);
	clear();
	/*2003.04.22 modified by stephen to deny the user who is under punishing to suicide*/
	if (!HAS_PERM(PERM_POST)|| !HAS_PERM(PERM_MAIL)
			|| !HAS_PERM(PERM_TALK)) {
		move(3, 0);
		prints("您被封禁权限, 不能随便自杀!!!\n");
		pressreturn();
		clear();
		return;

	}
	if (HAS_PERM(PERM_SYSOPS) || HAS_PERM(PERM_BOARDS)
			|| HAS_PERM(PERM_ADMINMENU) || HAS_PERM(PERM_SEEULEVELS)) {
		move(3, 0);
		prints("您有重任在身, 不能随便自杀啦!!\n");
		pressreturn();
		clear();
		return;
	}
	/*2003.04.22 stephen modify end*/
	if (currentuser.stay < 86400) {
		move(1, 0);
		prints("\n\n对不起, 您还未够资格执行此命令!!\n");
		prints("请 mail 给 SYSOP 说明自杀原因, 谢谢。\n");
		pressreturn();
		clear();
		return;
	}
	getdata(1, 0, "请输入您的密码: ", buf, PASSLEN, NOECHO, YEA);
	if (*buf == '\0' || !checkpasswd(currentuser.passwd, buf)) {
		prints("\n\n很抱歉, 您输入的密码不正确。\n");
		pressreturn();
		clear();
		return;
	}
	getdata(3, 0, "请问您叫什麽名字? ", buf, NAMELEN, DOECHO, YEA);
	if (*buf == '\0' || strcmp(buf, currentuser.realname)) {
		prints("\n\n很抱歉, 我并不认识您。\n");
		pressreturn();
		clear();
		return;
	}
	clear();
	//   move(1, 0);
	//   prints("[1;5;31m警告[0;1;31m： 自杀後, 您将无法再用此帐号进入本站！！");
	move(3, 0);
	//   prints("[1;32m本站站务没有义务为您恢复帐号。好难过喔 :( .....[m");
	//   move(5,0);
	i = 0;
	if (askyn("真是舍不得你，你走之前有什么话想说么", NA, NA)==YEA) {
		strcpy(lastword, ">\n> ");
		buf[0] = '\0';
		for (i = 0; i< 8; i++) {
			getdata(i+6, 0, ": ", buf, 77, DOECHO, YEA);
			if (buf[0] == '\0')
				break;
			strcat(lastword, buf);
			strcat(lastword, "\n> ");
		}
		if (i == 0)
			lastword[0] = '\0';
		else
			strcat(lastword, "\n\n");
		move(i + 8, 0);
		if (i == 0)
			prints("哎，你还是什么都不愿意说，是不是还有心思未了？");
		else if (i <= 4)
			prints("看着你憔悴的脸，我心都碎了 ... ");
		else
			prints("我会记得你的，朋友，我也知道你的离开也是没有办法的事，好走了");
	} else {
		strcpy(lastword, "> ......\n\n");
	}
	move(i + 10, 0);
	if (askyn("你确定要离开这个大家庭", NA, NA) == 1) {
		clear();
		{
			struct user_info uin;
			if (search_ulist(&uin, cmpuids3, usernum)) {
				if (!uin.active || (uin.pid && kill(uin.pid, 0) == -1))
					;
				else if (uin.pid)
					kill(uin.pid, SIGHUP);
			}
		}
		currentuser.userlevel = 0;
		substitut_record(PASSFILE, &currentuser, sizeof(struct userec),
				usernum);
		mail_info(lastword);
		modify_user_mode(OFFLINE);
		kick_user(&uinfo);
		exit(0);
	}
}

//保存用户近期信息
int getuinfo(FILE *fn) {
	int num;
	char buf[40];
	fprintf(fn, "\n\n他的代号     : %s\n", currentuser.userid);
	fprintf(fn, "他的昵称     : %s\n", currentuser.username);
	fprintf(fn, "真实姓名     : %s\n", currentuser.realname);
	fprintf(fn, "居住住址     : %s\n", currentuser.address);
	fprintf(fn, "电子邮件信箱 : %s\n", currentuser.email);
	fprintf(fn, "真实 E-mail  : %s\n", currentuser.reginfo);
	fprintf(fn, "Ident 资料   : %s\n", currentuser.ident);
	getdatestring(currentuser.firstlogin, NA);
	fprintf(fn, "帐号建立日期 : %s\n", datestring);
	getdatestring(currentuser.lastlogin, NA);
	fprintf(fn, "最近光临日期 : %s\n", datestring);
	fprintf(fn, "最近光临机器 : %s\n", currentuser.lasthost);
	fprintf(fn, "上站次数     : %d 次\n", currentuser.numlogins);
	fprintf(fn, "文章数目     : %d\n", currentuser.numposts);
	fprintf(fn, "上站总时数   : %d 小时 %d 分钟\n", currentuser.stay / 3600,
			(currentuser.stay / 60) % 60);
	strcpy(buf, "ltmprbBOCAMURS#@XLEast0123456789");
	for (num = 0; num < 30; num++)
		if (!(currentuser.userlevel & (1 << num)))
			buf[num] = '-';
	buf[num] = '\0';
	fprintf(fn, "使用者权限   : %s\n\n", buf);
	return 0;
}

void mail_info(char *lastword) {
	FILE *fn;
	time_t now;
	char filename[STRLEN];

	now = time(0);
	getdatestring(now, NA);
	sprintf(filename, "%s 于 %s 登记自杀", currentuser.userid, datestring);
	securityreport(filename, 1, 3);
	sprintf(filename, "tmp/suicide.%s", currentuser.userid);
	if ((fn = fopen(filename, "w")) != NULL) {
		fprintf(fn, "大家好,\n\n");
		fprintf(fn, "我是 %s (%s)。我己经决定在 15 天后离开这里了。\n\n",
				currentuser.userid, currentuser.username);
		getdatestring(currentuser.firstlogin, NA);
		fprintf(fn, "自 %14.14s 至今，我已经来此 %d 次了，在这总计 %d 分钟的网络生命中，\n",
				datestring, currentuser.numlogins, currentuser.stay/60);
		fprintf(fn, "我又如何会轻易舍弃呢？但是我得走了...  点点滴滴－－尽在我心中！\n\n");
		fprintf(fn, "%s", lastword);
		fprintf(fn, "朋友们，请把 %s 从你们的好友名单中拿掉吧。因为我己经决定离开这里了!\n\n",
				currentuser.userid);
		fprintf(fn, "或许有朝一日我会回来的。 珍重!! 再见!!\n\n\n");
		getdatestring(now, NA);
		fprintf(fn, "%s 於 %s 留.\n\n", currentuser.userid, datestring);
		fclose(fn);
		{
			char sc_title[128];
			sprintf(sc_title, "%s的自杀留言...", currentuser.userid);
			Postfile(filename, "GoneWithTheWind", sc_title, 2);
			unlink(filename);
		}
	}
}

/*2003.04.22 added by stephen to add retire function
 **can give up these permisions: 1.login 2.chat 3.mail 4.post
 **use lookupuser as temp userec struct 
 */
//	戒网
int giveUpBBS() {
	char buf[STRLEN], genbuf[STRLEN];
	FILE *fn;
	char ans[3], day[10];
	int i, j, k, lcount, tcount;
	int id;
	time_t now;

	lookupuser = currentuser;

	id = getuser(currentuser.userid);

	modify_user_mode(GIVEUPBBS);
	if (!HAS_PERM(PERM_REGISTER)) {
		clear();
		move(11, 28);
		prints("[1m[33m你有还没有注册通过，不能戒网！[m");
		pressanykey();
		return;
	}

	if (HAS_PERM(PERM_SYSOPS) || HAS_PERM(PERM_BOARDS)
			|| HAS_PERM(PERM_OBOARDS) || HAS_PERM(PERM_ANNOUNCE)) {
		clear();
		move(11, 28);
		prints("[1m[33m你有重任在身，不能戒网！[m");
		pressanykey();
		return;
	}

	lcount = 0;
	tcount = 0;

	memset(buf, 0, STRLEN);
	memset(ans, 0, 3);
	memset(day, 0, 10);

	sethomefile(genbuf, lookupuser.userid, "giveupBBS");
	fn = fopen(genbuf, "rt");
	if (fn) {
		clear();
		move(1, 0);
		prints("你现在的戒网情况：\n\n");
		while (!feof(fn)) {
			if (fscanf(fn, "%d %d", &i, &j) <= 0)
				break;
			switch (i) {
				case 1:
					prints("上站权限");
					break;
				case 2:
					prints("发表权限");
					break;
				case 3:
					prints("聊天权限");
					break;
				case 4:
					prints("发信权限");
					break;
			}
			sprintf(buf, "        还有%d天\n", j - time(0) / 3600 / 24);
			prints(buf);
			lcount++;
		}
		fclose(fn);
		memset(buf, 0, STRLEN);
		pressanykey();
	}

	clear();
	move(1, 0);
	prints("请选择戒网种类:");
	move(3, 0);
	prints("(0) - 结束");
	move(4, 0);
	prints("(1) - 上站权限");
	move(5, 0);
	prints("(2) - 发表权限");
	move(6, 0);
	prints("(3) - 聊天权限");
	move(7, 0);
	prints("(4) - 发信权限");

	getdata(10, 0, "请选择 [0]", ans, 2, DOECHO, NULL);
	if (ans[0] < '1' || ans[0] > '4') {
		return;
	}
	k = 1;
	switch (ans[0]) {
		case '1':
			k = k && (lookupuser.userlevel & PERM_LOGIN);
			break;
		case '2':
			k = k && (lookupuser.userlevel & PERM_POST);
			break;
		case '3':
			k = k && (lookupuser.userlevel & PERM_TALK);
			break;
		case '4':
			k = k && (lookupuser.userlevel & PERM_MAIL);
			break;
	}

	if (!k) {
		prints("\n\n你已经没有了该权限");
		pressanykey();
		return;
	}

	getdata(11, 0, "请输入戒网天数 [0]", day, 4, DOECHO, NULL);
	i = 0;
	while (day[i]) {
		if (!isdigit(day[i]))
			return;
		i++;
	}
	j = atoi(day);
	if (j <= 0)
		return;

	if (compute_user_value(&lookupuser) <= j) {
		prints("\n\n对不起，天数不可以大于生命力...");
		pressanykey();
		return;
	}

	j = time(0) / 3600 / 24 + j;

	move(13, 0);

	if (askyn("你确定要戒网吗？", 0) == 1) {
		getdata(15, 0, "请输入密码: ", buf, 39, NOECHO, NULL);
		if (*buf == '\0' || !checkpasswd(lookupuser.passwd, buf)) {
			prints("\n\n很抱歉, 您输入的密码不正确。\n");
			pressanykey();
			return;
		}

		sethomefile(genbuf, lookupuser.userid, "giveupBBS");
		fn = fopen(genbuf, "at");
		if (!fn) {
			prints("\n\n由于系统问题，现在你不能戒网");
			pressanykey();
			return;
		}
		fprintf(fn, "%d %d\n", ans[0] - 48, j);
		fclose(fn);

		now = time(0);
		getdatestring(now, NA);

		switch (ans[0]) {
			case '1':
				lookupuser.userlevel &= ~PERM_LOGIN;
				sprintf(buf, "%s 于 %14.14s 戒 %s权限 %d 天。",
						lookupuser.userid, datestring, "上站", atoi(day));
				break;
			case '2':
				lookupuser.userlevel &= ~PERM_POST;
				sprintf(buf, "%s 于 %14.14s 戒 %s权限 %d 天。",
						lookupuser.userid, datestring, "发文", atoi(day));
				break;
			case '3':
				lookupuser.userlevel &= ~PERM_TALK;
				sprintf(buf, "%s 于 %14.14s 戒 %s权限 %d 天。",
						lookupuser.userid, datestring, "聊天", atoi(day));
				break;
			case '4':
				lookupuser.userlevel &= ~PERM_MAIL;
				sprintf(buf, "%s 于 %14.14s 戒 %s权限 %d 天。",
						lookupuser.userid, datestring, "信件", atoi(day));
				break;
		}
		lcount++;
		securityreport(buf, 1, 3);

		if (lookupuser.userlevel & PERM_LOGIN)
			tcount++;
		if (lookupuser.userlevel & PERM_POST)
			tcount++;
		if (lookupuser.userlevel & PERM_TALK)
			tcount++;
		if (lookupuser.userlevel & PERM_MAIL)
			tcount++;

		if (lcount + tcount == 4)
			lookupuser.flags[0] |= GIVEUPBBS_FLAG;
		else
			lookupuser.flags[0] &= ~GIVEUPBBS_FLAG;

		prints("\n\n你已经开始戒网了");

		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);

		memset(buf, 0, STRLEN);
		memset(day, 0, 10);

		pressanykey();
		if (ans[0] == '1')
			abort_bbs(0);

		memset(ans, 0, 3);
	}
}
/*2003.04.22 stephen add end*/
