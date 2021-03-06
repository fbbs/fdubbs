// deardragon 2000.09.26 over
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
 $Id: userinfo.c 366 2007-05-12 16:35:51Z danielfree $
 */

#include "bbs.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif
//modified by money 2002.11.15

extern time_t login_start_time;
extern char fromhost[60];
extern char *cexpstr();

//将ptr指向的字符串中字符值为0xFF的转换成空格
void filter_ff(char *ptr) {
	while (*ptr) {
		if (*(unsigned char *)ptr == 0xff)
			*ptr = ' ';
		ptr++;
	}
	return;
}

//	用于	设定个人资料  选单时显示的信息,即显示个人资料
void disply_userinfo(struct userec *u) {
	int num, exp;
	time_t now;

	move(2, 0);
	clrtobot();
	now = time(0);
	set_safe_record();
	prints("您的代号     : %-14s", u->userid);
	prints("昵称 : %-20s", u->username);
	prints("     性别 : %s\n", (u->gender == 'M' ? "男" : "女"));
	prints("出生日期     : %d年%d月%d日", u->birthyear + 1900, u->birthmonth,
			u->birthday);
	prints(" (累计生活天数 : %d)\n", days_elapsed(u->birthyear + 1900, 
			u->birthmonth, u->birthday, now));
	prints("电子邮件信箱 : %s\n", u->email);
	prints("最近光临机器 : %-22s\n", u->lasthost);
	prints("帐号建立日期 : %s[距今 %d 天]\n",
			getdatestring(u->firstlogin, DATE_ZH),
			(now - (u->firstlogin)) / 86400);
	getdatestring(u->lastlogin, NA);
	prints("最近光临日期 : %s[距今 %d 天]\n",
			getdatestring(u->lastlogin, DATE_ZH),
			(now-(u->lastlogin)) / 86400);
#ifdef ALLOWGAME
	prints("文章数目     : %-20d 奖章数目 : %d\n",u->numposts,u->nummedals);
	prints("私人信箱     : %d 封\n", u->nummails);
	prints("您的银行存款 : %d元  贷款 : %d元 (%s)\n",
			u->money,u->bet,cmoney(u->money-u->bet));
#else
	prints("文章数目     : %-20d \n", u->numposts);
	prints("私人信箱     : %d 封 \n", u->nummails);
#endif
	prints("上站次数     : %d 次      ", u->numlogins);
	prints("上站总时数   : %d 小时 %d 分钟\n", u->stay/3600, (u->stay/60)%60);
	exp = countexp(u);
	//modified by iamfat 2002.07.25
#ifdef SHOWEXP
	prints("经验值       : %d  (%-10s)    ", exp, cexpstr(exp));
#else
	prints("经验值       : [%-10s]     ", cexpstr(exp));
#endif
	exp = countperf(u);
#ifdef SHOWPERF
	prints("表现值 : %d  (%s)\n", exp, cperf(exp));
#else
	prints("表现值  : [%s]\n", cperf(exp));
#endif
	strcpy(genbuf, "ltmprbBOCAMURS#@XLEast0123456789\0");
	for (num = 0; num < strlen(genbuf) ; num++)
		if (!(u->userlevel & (1 << num))) //相应权限为空,则置'-'
			genbuf[num] = '-';
	prints("使用者权限   : %s\n", genbuf);
	prints("\n");
	if (u->userlevel & PERM_SYSOPS) {
		prints("  您是本站的站长, 感谢您的辛勤劳动.\n");
	} else if (u->userlevel & PERM_BOARDS) {
		prints("  您是本站的版主, 感谢您的付出.\n");
	} else if (u->userlevel & PERM_REGISTER) {
		prints("  您的注册程序已经完成, 欢迎加入本站.\n");
	} else if (u->lastlogin - u->firstlogin < 3 * 86400) {
		prints("  新手上路, 请阅读 Announce 讨论区.\n");
	} else {
		prints("  注册尚未成功, 请参考本站进站画面说明.\n");
	}
}

//	改变用户记录,u为以前的记录,newinfo为新记录,后两个参数均为指针
//		i为所显示的行
void uinfo_change1(int i, struct userec *u, struct userec *newinfo) {
	char buf[STRLEN], genbuf[128];

	if (currentuser.userlevel & PERM_SYSOPS) {
		char temp[30];
		temp[0] = 0;
		FILE *fp;
		sethomefile(genbuf, u->userid, ".volunteer");
		if ((fp = fopen(genbuf, "r")) != NULL) {
			fgets(temp, 30, fp);
			fclose(fp);
			sprintf(genbuf, "输入身份(输空格取消身份)：[%s]", temp);
		} else
			sprintf(genbuf, "输入身份：");
		getdata(i++, 0, genbuf, buf, 30, DOECHO, YEA);
		if (buf[0]) {
			sethomefile(genbuf, u->userid, ".volunteer");
			if ((fp = fopen(genbuf, "w")) != NULL) {
				if (buf[0] != ' ') {
					fputs(buf, fp);
					fclose(fp);
				} else
					unlink(genbuf);
			}
		}
	}

	sprintf(genbuf, "电子信箱 [%s]: ", u->email);
	getdata(i++, 0, genbuf, buf, STRLEN - 1, DOECHO, YEA);
	if (buf[0]) {
#ifdef MAILCHECK
#ifdef MAILCHANGED
		if(u->uid == usernum)
		netty_check = 1;
#endif
#endif
		strlcpy(newinfo->email, buf, STRLEN-12);
	}

	sprintf(genbuf, "上线次数 [%d]: ", u->numlogins);
	getdata(i++, 0, genbuf, buf, 10, DOECHO, YEA);
	if (atoi(buf) > 0)
		newinfo->numlogins = atoi(buf);

	sprintf(genbuf, "发表文章数 [%d]: ", u->numposts);
	getdata(i++, 0, genbuf, buf, 10, DOECHO, YEA);
	if (atoi(buf) >0)
		newinfo->numposts = atoi(buf);

	sprintf(genbuf, "登陆总时间 [%d]: ", u->stay);
	getdata(i++, 0, genbuf, buf, 10, DOECHO, YEA);
	if (atoi(buf) > 0)
		newinfo->stay = atoi(buf);
	sprintf(genbuf, "firstlogin [%d]: ", u->firstlogin);
	getdata(i++, 0, genbuf, buf, 15, DOECHO, YEA);
	if (atoi(buf) >0)
		newinfo->firstlogin = atoi(buf);
	//add end          				      	      	
#ifdef ALLOWGAME
	sprintf(genbuf, "银行存款 [%d]: ", u->money);
	getdata(i++, 0, genbuf, buf, 8, DOECHO, YEA);
	if (atoi(buf)> 0)
	newinfo->money = atoi(buf);

	sprintf(genbuf, "银行贷款 [%d]: ", u->bet);
	getdata(i++, 0, genbuf, buf, 8, DOECHO, YEA);
	if (atoi(buf)> 0)
	newinfo->bet = atoi(buf);

	sprintf(genbuf, "奖章数 [%d]: ", u->nummedals);
	getdata(i++, 0, genbuf, buf, 10, DOECHO, YEA);
	if (atoi(buf)> 0)
	newinfo->nummedals = atoi(buf);
#endif
}

// 检查用户的资料,
void check_uinfo(struct userec *u, int MUST) {
	int changeIT = 0, changed = 0, pos = 2;
	int r = 0; // added by money 2003.10.24. for test 闰年
	char *ptr;// added by money 2003.10.29. for filter '0xff'
	char ans[5];

	while (1) { // 检查昵称
		changeIT = MUST || (strlen(u->username) < 2) ||(strstr(
				u->username, "  "))||(strstr(u->username, "　"));
		if (!changeIT) { //不需要再改变
			if (changed) {
				pos ++;
				changed = 0;
			}
			break;
		} else {
			MUST = 0;
			changed = 1;
		}
		getdata(pos, 0, "请输入您的昵称 (Enter nickname): ", u->username,
				NAMELEN, DOECHO, YEA);
		strcpy(uinfo.username, u->username);
		ptr = uinfo.username;
		filter_ff(ptr);
		update_ulist(&uinfo, utmpent);
	}
	{ // 检查性别
		changeIT = MUST||(strchr("MF", u->gender) == NULL);
		if (changeIT) {
			getdata(pos, 0, "请输入您的性别: M.男 F.女 [M]: ", ans, 2, DOECHO, YEA);
			if (ans[0]!='F'&& ans[0]!='f'||ans[0]=='m') //后一个判断可省...
				u->gender = 'M';
			else
				u->gender = 'F';
			pos ++;
		}
	}
	while (1) { // 检查出生年
		changeIT = MUST||(u->birthyear <20) ||(u->birthyear>98);
		if (u->birthyear % 4 == 0) {
			if (u->birthyear % 100 != 0)
				r = 1;
			else if (u->birthyear % 400 == 0)
				r = 1;
		}
		if (!changeIT) {
			if (changed) {
				pos ++;
				changed = 0;
			}
			break;
		} else {
			MUST = 0;
			changed = 1;
		}
		getdata(pos, 0, "请输入您的生日年份(四位数): ", ans, 5, DOECHO, YEA);
		if (atoi(ans)<1920 || atoi(ans) > 1998) {
			MUST = 1;
			continue;
		}
		u->birthyear = atoi(ans) -1900;
		/* add by money 2003.10.24. for test 闰年 */
		if ((atoi(ans) % 4) == 0) {
			if ((atoi(ans) % 100) != 0)
				r = 1;
			else if ((atoi(ans) % 400) == 0)
				r = 1;
		}
		/* add end */
	}
	while (1) { // 检查出生月
		changeIT = MUST||(u->birthmonth <1) ||(u->birthmonth>12);
		if (!changeIT) {
			if (changed) {
				pos ++;
				changed = 0;
			}
			break;
		} else {
			MUST = 0;
			changed = 1;
		}
		getdata(pos, 0, "请输入您的生日月份: ", ans, 3, DOECHO, YEA);
		u->birthmonth = atoi(ans);
	}
	while (1) { // 检查出生日
		changeIT = MUST||(u->birthday <1) ||(u->birthday>31)
				||(u->birthmonth<8&&!(u->birthmonth%2)&&(u->birthday>30)
				||u->birthmonth>7&&(u->birthmonth%2))&&u->birthday>30
		|| u->birthmonth==2&&u->birthday>29;
		/* add by money 2003.10.24. for check 2.28/29 */
		if (u->birthmonth == 2) {
			if (!r)
			if ((u->birthday>28)&&(!changeIT))
			changeIT = !changeIT;
		}
		/* add end */

		if(!changeIT) {
			if(changed) {
				pos ++;
				changed = 0;
			}
			break;
		} else {
			MUST = 0;
			changed = 1;
		}
		getdata(pos, 0, "请输入您的出生日: ", ans, 3, DOECHO, YEA);
		u->birthday = atoi(ans);
	}
}

//	查询u所指向的用户的资料信息
int uinfo_query(struct userec *u, int real, int unum) {
	struct userec newinfo;
	char ans[3], buf[STRLEN], genbuf[128];
	char src[STRLEN], dst[STRLEN];
	int i, fail = 0;
	unsigned char *ptr; //add by money 2003.10.29 for filter '0xff' in nick
	int r = 0; //add by money 2003.10.14 for test 闰年
#ifdef MAILCHANGED
	int netty_check = 0;
#endif
	time_t now;
	struct tm *tmnow;
	memcpy(&newinfo, u, sizeof(currentuser));
	getdata(t_lines - 1, 0, real ? "请选择 (0)结束 (1)修改资料 (2)设定密码 ==> [0]"
			: "请选择 (0)结束 (1)修改资料 (2)设定密码 (3) 选签名档 ==> [0]", ans, 2,
			DOECHO, YEA);
	clear();

	//added by roly 02.03.07
	if (real && !HAS_PERM(PERM_SPECIAL0))
		return;
	//add end

	refresh();
	now = time(0);
	tmnow = localtime(&now);

	i = 3;
	move(i++, 0);
	if (ans[0] != '3' || real)
		prints("使用者代号: %s\n", u->userid);
	switch (ans[0]) {
		case '1':
			move(1, 0);
			prints("请逐项修改,直接按 <ENTER> 代表使用 [] 内的资料。\n");
			sprintf(genbuf, "昵称 [%s]: ", u->username);
			getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, YEA);
			if (buf[0]) {
				strlcpy(newinfo.username, buf, NAMELEN);
				/* added by money 2003.10.29 for filter 0xff in nick */
				ptr = newinfo.username;
				filter_ff(ptr);
				/* added end */
			}
			sprintf(genbuf, "出生年 [%d]: ", u->birthyear + 1900);
			getdata(i++, 0, genbuf, buf, 5, DOECHO, YEA);
			if (buf[0] && atoi(buf) > 1920 && atoi(buf) < 1998)
				newinfo.birthyear = atoi(buf) - 1900;

			sprintf(genbuf, "出生月 [%d]: ", u->birthmonth);
			getdata(i++, 0, genbuf, buf, 3, DOECHO, YEA);
			if (buf[0] && atoi(buf) >= 1 && atoi(buf) <= 12)
				newinfo.birthmonth = atoi(buf);

			sprintf(genbuf, "出生日 [%d]: ", u->birthday);
			getdata(i++, 0, genbuf, buf, 3, DOECHO, YEA);
			if (buf[0] && atoi(buf) >= 1 && atoi(buf) <= 31)
				newinfo.birthday = atoi(buf);

			/* add by money 2003.10.24 for 2.28/29 test */
			if (newinfo.birthmonth == 2) {
				if (((newinfo.birthyear+1900) % 4) == 0) {
					if (((newinfo.birthyear+1900) % 100) != 0)
						r = 1;
					else if (((newinfo.birthyear+1900) % 400) == 0)
						r = 1;
				}
				if (r) {
					if (newinfo.birthday > 29)
						newinfo.birthday = 29;
				} else {
					if (newinfo.birthday > 28)
						newinfo.birthday = 28;
				}
			}

			if ((newinfo.birthmonth<7)&&(!(newinfo.birthmonth%2))
					&&(newinfo.birthday>30))
				newinfo.birthday = 30;
			if ((newinfo.birthmonth>8)&&(newinfo.birthmonth%2)
					&&(newinfo.birthday>30))
				newinfo.birthday = 30;
			/* add end */

			sprintf(genbuf, "性别(M.男)(F.女) [%c]: ", u->gender);
			getdata(i++, 0, genbuf, buf, 2, DOECHO, YEA);
			if (buf[0]) {
				if (strchr("MmFf", buf[0]))
					newinfo.gender = toupper(buf[0]);
			}

			if (real)
				uinfo_change1(i, u, &newinfo);
			break;
		case '2':
			if (!real) {
				getdata(i++, 0, "请输入原密码: ", buf, PASSLEN, NOECHO, YEA);
				if (*buf == '\0' || !checkpasswd(u->passwd, buf)) {
					prints("\n\n很抱歉, 您输入的密码不正确。\n");
					fail++;
					break;
				}
			}
			/*Modified by IAMFAT 2002-05-25*/
			/*
			 getdata(i++, 0, "请设定新密码: ", buf, PASSLEN, NOECHO, YEA);
			 if (buf[0] == '\0') {
			 prints("\n\n密码设定取消, 继续使用旧密码\n");
			 fail++;
			 break;
			 }
			 strncpy(genbuf, buf, PASSLEN); 
			 getdata(i++, 0, "请重新输入新密码: ", buf, PASSLEN, NOECHO, YEA);
			 if (strncmp(buf, genbuf, PASSLEN)) {
			 prints("\n\n新密码确认失败, 无法设定新密码。\n");
			 fail++;
			 break;
			 }
			 buf[8] = '\0';
			 strncpy(newinfo.passwd, genpasswd(buf), ENCPASSLEN);
			 */
			while (1) {
				getdata(i++, 0, "请设定新密码: ", buf, PASSLEN, NOECHO, YEA);
				if (buf[0] == '\0') {
					prints("\n\n密码设定取消, 继续使用旧密码\n");
					fail++;
					break;
				}
				if (strlen(buf) < 4 || !strcmp(buf, u->userid)) {
					prints("\n\n密码太短或与使用者代号相同, 密码设定取消, 继续使用旧密码\n");
					fail++;
					break;
				}
				strlcpy(genbuf, buf, PASSLEN);
				getdata(i++, 0, "请重新输入新密码: ", buf, PASSLEN, NOECHO, YEA);
				if (strncmp(buf, genbuf, PASSLEN)) {
					prints("\n\n新密码确认失败, 无法设定新密码。\n");
					fail++;
					break;
				}
				buf[8] = '\0';
				strlcpy(newinfo.passwd, genpasswd(buf), ENCPASSLEN);
				break;
			}
			/* Modify End */
			break;
		case '3':
			if (!real) {
				sprintf(genbuf, "目前使用签名档 [%d]: ", u->signature);
				getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
				if (atoi(buf) >= 0)
					newinfo.signature = atoi(buf);
			} else {
				/* Remmed by Amigo 2002.04.24. Userid unchangable. 
				 struct user_info uin;
				 extern int t_cmpuids();
				 if(t_search_ulist(&uin, t_cmpuids, unum, NA, NA)!=0){
				 prints("\n对不起，该用户目前正在线上。");
				 fail++;
				 } else if(!strcmp(lookupuser.userid,"SYSOP")) {
				 prints("\n对不起，你不可以修改 SYSOP 的 ID。");
				 fail++;
				 } else {   
				 getdata(i++,0,"新的使用者代号: ",genbuf,IDLEN+1,DOECHO, YEA);
				 if (*genbuf != '\0') {
				 if (getuser(genbuf)) {
				 prints("\n对不起! 已经有同样 ID 的使用者\n");
				 fail++;
				 } else {
				 strncpy(newinfo.userid, genbuf, IDLEN + 2);
				 }
				 } else fail ++;
				 }*/
			}
			break;
		default:
			clear();
			return 0;
	}
	if (fail != 0) {
		pressreturn();
		clear();
		return 0;
	}
	if (askyn("确定要改变吗", NA, YEA) == YEA) {
		if (real) {
			char secu[STRLEN];
			sprintf(secu, "修改 %s 的基本资料或密码。", u->userid);
			securityreport(secu, 0, 0);
		}
		if (strcmp(u->userid, newinfo.userid)) {
			sprintf(src, "mail/%c/%s", toupper(u->userid[0]), u->userid);
			sprintf(dst, "mail/%c/%s", toupper(newinfo.userid[0]),
					newinfo.userid);
			rename(src, dst);
			sethomepath(src, u->userid);
			sethomepath(dst, newinfo.userid);
			rename(src, dst);
			sethomefile(src, u->userid, "register");
			unlink(src);
			sethomefile(src, u->userid, "register.old");
			unlink(src);
			setuserid(unum, newinfo.userid);
		}
		if (!strcmp(u->userid, currentuser.userid)) {
			extern int WishNum;
			strlcpy(uinfo.username, newinfo.username, NAMELEN);
			WishNum = 9999;
		}
#ifdef MAILCHECK	 
#ifdef MAILCHANGED
		if ((netty_check == 1)&&!HAS_PERM(PERM_SYSOPS)) {
			sprintf(genbuf, "%s", BBSHOST);
			if ( (!strstr(newinfo.email, genbuf)) &&
					(!invalidaddr(newinfo.email)) &&
					(!invalid_email(newinfo.email))) {
				strcpy(u->email, newinfo.email);
				send_regmail(u);
			} else {
				move(t_lines - 5, 0);
				prints("\n您所填的电子邮件地址 【[1;33m%s[m】\n",
						newinfo.email);
				prints("恕不受本站承认，系统不会投递注册信，请把它修正好...\n");
				pressanykey();
				return 0;
			}
		}
#endif
#endif
		memcpy(u, &newinfo, (size_t)sizeof(currentuser));
#ifdef MAILCHECK
#ifdef MAILCHANGED
		if ((netty_check == 1)&&!HAS_PERM(PERM_SYSOPS)) {
			/* Following line modified by Amigo 2002.06.08. To omit default perm_page right. */
			newinfo.userlevel &= ~(PERM_REGISTER | PERM_TALK);
			sethomefile(src, newinfo.userid, "register");
			sethomefile(dst, newinfo.userid, "register.old");
			rename(src, dst);
		}
#endif
#endif
		substitut_record(PASSFILE, &newinfo, sizeof(newinfo), unum);
	}
	clear();
	return 0;
}

//与Information相关联.在comm_list.c里,用于显示和设定个人资料
void x_info() {
	if (!strcmp("guest", currentuser.userid))
		return;
	modify_user_mode(GMENU);
	disply_userinfo(&currentuser);
	uinfo_query(&currentuser, 0, usernum);
}

//	更改用户资料中某域所对应设定
void getfield(int line, char *info, char *desc, char *buf, int len) {
	char prompt[STRLEN];
	sprintf(genbuf, "  原先设定: %-40.40s [1;32m(%s)[m",
			(buf[0] == '\0') ? "(未设定)" : buf, info);
	move(line, 0);
	prints("%s", genbuf);
	sprintf(prompt, "  %s: ", desc);
	getdata(line + 1, 0, prompt, genbuf, len, DOECHO, YEA);
	if (genbuf[0] != '\0')
		strlcpy(buf, genbuf, len);
	move(line, 0);
	clrtoeol();
	prints("  %s: %s\n", desc, buf);
	clrtoeol();
}

//	填写用户资料
void x_fillform() {
	char ans[5], *mesg, *ptr;
	REGINFO ri;
	FILE *fn;

	if (!strcmp("guest", currentuser.userid))
		return;
	modify_user_mode(NEW);
	clear();
	move(2, 0);
	clrtobot();
	if (currentuser.userlevel & PERM_REGISTER) {
		prints("您已经完成本站的使用者注册手续, 欢迎加入本站的行列.");
		pressreturn();
		return;
	}
#ifdef PASSAFTERTHREEDAYS
	if (currentuser.lastlogin - currentuser.firstlogin < 3 * 86400) {
		prints("您首次登入本站未满三天(72个小时)...\n");
		prints("请先四处熟悉一下，在满三天以后再填写注册单。");
		pressreturn();
		return;
	}
#endif      
	if ((fn = fopen("unregistered", "rb")) != NULL) {
		while (fread(&ri, sizeof(ri), 1, fn)) {
			if (!strcasecmp(ri.userid, currentuser.userid)) {
				fclose(fn);
				prints("站长尚未处理您的注册申请单, 您先到处看看吧.");
				pressreturn();
				return;
			}
		}
		fclose(fn);
	}

	memset(&ri, 0, sizeof(ri));
	strlcpy(ri.userid, currentuser.userid, IDLEN+1);
	strlcpy(ri.email, currentuser.email, STRLEN-12);
	while (1) {
		move(3, 0);
		clrtoeol();
		prints("%s 您好, 请据实填写以下的资料:\n", currentuser.userid);
		do {
			getfield(6, "请用中文", "真实姓名", ri.realname, NAMELEN);
		} while (strlen(ri.realname)<4);

		do {
			getfield(8, "学校系级或所在单位", "学校系级", ri.dept, STRLEN);
		} while (strlen(ri.dept)< 6);

		do {
			getfield(10, "包括寝室或门牌号码", "目前住址", ri.addr, STRLEN);
		} while (strlen(ri.addr)<10);

		do {
			getfield(12, "包括可联络时间", "联络电话", ri.phone, STRLEN);
		} while (strlen(ri.phone)<8);

		getfield(14, "校友会或毕业学校", "校 友 会", ri.assoc, STRLEN);
		mesg = "以上资料是否正确, 按 Q 放弃注册 (Y/N/Quit)? [Y]: ";
		getdata(t_lines - 1, 0, mesg, ans, 3, DOECHO, YEA);
		if (ans[0] == 'Q' || ans[0] == 'q')
			return;
		if (ans[0] != 'N' && ans[0] != 'n')
			break;
	}
	ptr = ri.realname;
	filter_ff(ptr);
	ptr = ri.addr;
	filter_ff(ptr);
	ptr = ri.dept;
	filter_ff(ptr);
#ifndef FDQUAN
	strlcpy(currentuser.email, ri.email, STRLEN-12);
#endif	
	if ((fn = fopen("unregistered", "ab")) != NULL) {
		ri.regdate= time(NULL);
		fwrite(&ri, sizeof(ri), 1, fn);
		fclose(fn);
	}
	setuserfile(genbuf, "mailcheck");
	if ((fn = fopen(genbuf, "w")) == NULL) {
		fclose(fn);
		return;
	}
	fprintf(fn, "usernum: %d\n", usernum);
	fclose(fn);
}
// deardragon 2000.09.26 over

