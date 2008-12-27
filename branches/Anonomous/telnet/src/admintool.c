#ifndef WITHOUT_ADMIN_TOOLS
#ifndef DLM
#include <stdio.h>
#include "bbs.h"

extern int cmpbnames();
extern int cleanmail();
extern char *chgrp();
extern int dowall();
extern int cmpuids();
extern int t_cmpuids();
extern void flush_bcache();
extern int getblankbnum();
extern void rebuild_brdshm();
int showperminfo(int, int);
char cexplain[STRLEN];
char buf2[STRLEN];
char lookgrp[30];
char bnames[3][STRLEN]; //存放用户担任版主的版名,最多为三
FILE *cleanlog;

//在userid的主目录下 打开.bmfile文件,并将里面的内容与bname相比较
//              find存放从1开始返回所任版面的序号,为0表示没找到
//函数的返回值为userid担任版主的版面数
int getbnames(char *userid, char *bname, int *find) {
	int oldbm = 0;
	FILE *bmfp;
	char bmfilename[STRLEN], tmp[20];
	*find = 0;
	sethomefile(bmfilename, userid, ".bmfile");
	bmfp = fopen(bmfilename, "r");
	if (!bmfp) {
		return 0;
	}
	for (oldbm = 0; oldbm < 3;) {
		fscanf(bmfp, "%s\n", tmp);
		if (!strcmp(bname, tmp)) {
			*find = oldbm + 1;
		}
		strcpy(bnames[oldbm++], tmp);
		if (feof(bmfp)) {
			break;
		}
	}
	fclose(bmfp);
	return oldbm;
}
//      修改使用者资料
int m_info() {
	struct userec uinfo;
	char reportbuf[30];
	int id;

	if (!(HAS_PERM(PERM_USER)))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	stand_title("修改使用者资料");
	if (!gettheuserid(1, "请输入使用者代号: ", &id))
		return -1;
	memcpy(&uinfo, &lookupuser, sizeof (uinfo));
	sprintf(reportbuf, "check info: %s", uinfo.userid);
	report(reportbuf);

	move(1, 0);
	clrtobot();
	disply_userinfo(&uinfo);
	uinfo_query(&uinfo, 1, id);
	return 0;
}

//任命版主
int m_ordainBM() {

	int id, pos, oldbm = 0, i, find, bm = 1;
	struct boardheader fh;
	FILE *bmfp;
	char bmfilename[STRLEN], bname[STRLEN];
	char buf[5][STRLEN];

	if (!(HAS_PERM(PERM_USER)))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd())
		return;

	clear();
	stand_title("任命版主\n");
	clrtoeol();
	if (!gettheuserid(2, "输入欲任命的使用者帐号: ", &id))
		return 0;
	if (!gettheboardname(3, "输入该使用者将管理的讨论区名称: ", &pos, &fh, bname, 0))
		return -1;
	if (fh.BM[0] != '\0') {
		if (!strncmp(fh.BM, "SYSOPs", 6)) {
			move(5, 0);
			prints("%s 讨论区的版主是 SYSOPs 你不能再任命版主", bname);
			pressreturn();
			clear();
			return -1;
		}
		for (i = 0, oldbm = 1; fh.BM[i] != '\0'; i++) {
			if (fh.BM[i] == ' ')
				oldbm++;
		}
		//added by infotech,防止版主列表过长
		if (i + strlen(lookupuser.userid) > BMNAMEMAXLEN) {
			move(5, 0);
			prints("%s 讨论区版主列表太长,无法加入!", bname);
			pressreturn();
			clear();
			return -1;
		}
		//add end
		if (oldbm >= 3) {
			move(5, 0);
			prints("%s 讨论区已有 %d 名版主", bname, oldbm);
			pressreturn();
			if (oldbm >= BMMAXNUM) {
				clear();
				return -1;
			}
		}

		bm = 0;
	}
	if (!strcmp(lookupuser.userid, "guest")) {
		move(5, 0);
		prints("你不能任命 guest 当版主");
		pressanykey();
		clear();
		return -1;
	}
	oldbm = getbnames(lookupuser.userid, bname, &find);
	if (find || oldbm == 3) { //同一ID不能兼任超过三个版的版主
		move(5, 0);
		prints(" %s 已经是%s版的版主了", lookupuser.userid, find ? "该" : "三个");
		pressanykey();
		clear();
		return -1;
	}
	prints("\n你将任命 %s 为 %s 版版%s.\n", lookupuser.userid, bname, bm ? "主"
			: "副");
	if (askyn("你确定要任命吗?", NA, NA) == NA) {
		prints("取消任命版主");
		pressanykey();
		clear();
		return -1;
	}
	strcpy(bnames[oldbm], bname);
	if (!oldbm) { //第一次做版主
		char secu[STRLEN];

		lookupuser.userlevel |= PERM_BOARDS;
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
		sprintf(secu, "版主任命, 给予 %s 的版主权限", lookupuser.userid);
		securityreport(secu, 0, 1);
		move(15, 0);
		prints(secu);
		pressanykey();
		clear();
	}
	if (fh.BM[0] == '\0')
		strcpy(genbuf, lookupuser.userid);
	else
		sprintf(genbuf, "%s %s", fh.BM, lookupuser.userid);
	strncpy(fh.BM, genbuf, sizeof (fh.BM));
	//added by infotech
	strcpy(buf[0], fh.BM);
#ifdef  BMNAMELISTLIMIT
	for (i = 0; i < BMNAMELISTLEN && buf[0][i]; i++);
	if (i == BMNAMELISTLEN) {
		buf[0][i++] = '.';
		buf[0][i++] = '.';
		buf[0][i++] = '.';
		buf[0][i] = '\0';
	}
#endif
	//endadd
	//sprintf(genbuf, "%-38.38s(BM: %s)", fh.title +8, fh.BM);
	//精华区的显示: 动态分配        显示10个空格 printf("%*c",10,' ');
	{
		int blanklen; //前两个空间大小
		static const char BLANK = ' ';
		blanklen = STRLEN - strlen(fh.title + 8) - strlen(buf[0]) - 7;
		blanklen /= 2;
		blanklen = (blanklen > 0) ? blanklen : 1;
		sprintf(genbuf, "%s%*c(BM: %s)", fh.title + 8, blanklen, BLANK,
				buf[0]);
	}
	buf[0][0] = '\0';
	get_grp(fh.filename);
	edit_grp(fh.filename, lookgrp, fh.title + 8, genbuf);
	substitute_record(BOARDS, &fh, sizeof (fh), pos);
	sethomefile(bmfilename, lookupuser.userid, ".bmfile");
	bmfp = fopen(bmfilename, "w+");
	for (i = 0; i < oldbm + 1; i++) {
		fprintf(bmfp, "%s\n", bnames[i]);
	}
	fclose(bmfp);
	/* Modified by Amigo 2002.07.01. Add reference to BM-Guide. */
	//sprintf (genbuf, "\n\t\t\t【 通告 】\n\n"
	//	   "\t任命 %s 为 %s 版%s！\n"
	//	   "\t欢迎 %s 前往 BM_Home 版和本区 Zone 版向大家问好。\n"
	//	   "\t开始工作前，请先通读BM_Home版精华区的版主指南目录。\n",
	//	   lookupuser.userid, bname, bm ? "版主" : "版副", lookupuser.userid);

	//the new version add by Danielfree 06.11.12
	sprintf(
			genbuf,
			"\n"
				" 		[1;31m   ╔═╗╔═╗╔═╗╔═╗										 [m\n"
				" 	 [31m╋──[1m║[33m日[31m║║[33m月[31m║║[33m光[31m║║[33m华[31m║[0;33m──[1;36m〖领会站规精神·熟悉版主操作〗[0;33m─◇◆  [m\n"
				" 	 [31m│    [1m╚═╝╚═╝╚═╝╚═╝										  [m\n"
				" 	 [31m│																	  [m\n"
				" 		 [1;33m︻	[37m任命  %s  为  %s  版%s。							   [m\n"
				" 		 [1;33m通																  [m\n"
				" 		[1m	欢迎  %s  前往 BM_Home 版和本区 Zone 版向大家问好。			 [m\n"
				" 		 [1;33m告																  [m\n"
				" 		 [1;33m︼	[37m开始工作前，请先通读BM_Home版精华区的版主指南目录。		   [m\n"
				" 																		 [33m│  [m\n"
				" 											 [1;33m╔═╗╔═╗╔═╗╔═╗   [0;33m │  [m\n"
				" 	 [31m◇◆─[1;35m〖维护版面秩序·建设和谐光华〗[0;31m──[1;33m║[31m版[33m║║[31m主[33m║║[31m委[33m║║[31m任[33m║[0;33m──╋	[m\n"
				" 											 [1;33m╚═╝╚═╝╚═╝╚═╝		  [m\n"
				" 																			 [m\n", lookupuser.userid, bname,
			bm ? "版主" : "版副", lookupuser.userid);
	//add end
	for (i = 0; i < 5; i++)
		buf[i][0] = '\0';
	move(8, 0);
	prints("请输入任命附言(最多五行，按 Enter 结束)");
	for (i = 0; i < 5; i++) {
		getdata(i + 9, 0, ": ", buf[i], STRLEN - 5, DOECHO, YEA);
		if (buf[i][0] == '\0')
			break;
	}
	for (i = 0; i < 5; i++) {
		if (buf[i][0] == '\0')
			break;
		if (i == 0)
			strcat(genbuf, "\n\n");
		strcat(genbuf, "\t");
		strcat(genbuf, buf[i]);
		strcat(genbuf, "\n");
	}
	strcpy(currboard, bname);
	sprintf(bmfilename, "任命 %s 为 %s 版%s", lookupuser.userid, fh.filename,
			bm ? "版主" : "版副");
	//autoreport(bmfilename,genbuf,YEA,NULL);
	autoreport(bmfilename, genbuf, YEA, lookupuser.userid, 1); //3x rubing and erebus:)   
#ifdef ORDAINBM_POST_BOARDNAME
	strcpy (currboard, ORDAINBM_POST_BOARDNAME);
	//autoreport(bmfilename,genbuf,YEA,NULL);
	autoreport (bmfilename, genbuf, YEA, lookupuser.userid, 1);
#endif
	/* Added by Amigo 2002.07.01. Send BM assign mail to user's mail box. */
	//{
	//  FILE *se;
	//  char fname[STRLEN];

	//  sprintf( fname, "tmp/AutoPoster.%s.%05d", currentuser.userid, uinfo.pid );
	//  if( ( se = fopen(fname,"w") ) != NULL ){
	//          fprintf( se, "%s", genbuf );
	//          fclose( se );
	//          if( lookupuser.userid != NULL )
	//          mail_file( fname, lookupuser.userid, bmfilename );
	//  }
	//}
	/* Add end. */
	securityreport(bmfilename, 0, 1);
	move(16, 0);
	prints(bmfilename);
	pressanykey();
	return 0;
}

//版主离职
int m_retireBM() {
	int id, pos, right = 0, oldbm = 0, i, j, bmnum;
	int find, bm = 1;
	struct boardheader fh;
	FILE *bmfp;
	char bmfilename[STRLEN], buf[5][STRLEN];
	char bname[STRLEN], usernames[BMMAXNUM][STRLEN];

	if (!(HAS_PERM(PERM_USER)))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd())
		return;

	clear();
	stand_title("版主离职\n");
	clrtoeol();
	if (!gettheuserid(2, "输入欲离职的版主帐号: ", &id))
		return -1;
	if (!gettheboardname(3, "请输入该版主要辞去的版名: ", &pos, &fh, bname, 0))
		return -1;
	oldbm = getbnames(lookupuser.userid, bname, &find);
	if (!oldbm || !find) {
		move(5, 0);
		prints(" %s %s版版主，如有错误，请通知程序站长。", lookupuser.userid,
				(oldbm) ? "不是该" : "没有担任任何");
		pressanykey();
		clear();
		return -1;
	}
	for (i = find - 1; i < oldbm; i++) {
		if (i != oldbm - 1)
			strcpy(bnames[i], bnames[i + 1]);
	}
	bmnum = 0;
	for (i = 0, j = 0; fh.BM[i] != '\0'; i++) {
		if (fh.BM[i] == ' ') {
			usernames[bmnum][j] = '\0';
			bmnum++;
			j = 0;
		} else {
			usernames[bmnum][j++] = fh.BM[i];
		}
	}
	usernames[bmnum++][j] = '\0';
	for (i = 0, right = 0; i < bmnum; i++) {
		if (!strcmp(usernames[i], lookupuser.userid)) {
			right = 1;
			if (i)
				bm = 0;
		}
		if (right && i != bmnum - 1) //while(right&&i<bmnum)似乎更好一些;infotech
			strcpy(usernames[i], usernames[i + 1]);
	}
	if (!right) {
		move(5, 0);
		prints("对不起， %s 版版主名单中没有 %s ，如有错误，请通知技术站长。", bname,
				lookupuser.userid);
		pressanykey();
		clear();
		return -1;
	}
	prints("\n你将取消 %s 的 %s 版版%s职务.\n", lookupuser.userid, bname, bm ? "主"
			: "副");
	if (askyn("你确定要取消他的该版版主职务吗?", NA, NA) == NA) {
		prints("\n呵呵，你改变心意了？ %s 继续留任 %s 版版主职务！", lookupuser.userid, bname);
		pressanykey();
		clear();
		return -1;
	}
	if (bmnum - 1) { //还有版主,为什么不用strcat ?
		sprintf(genbuf, "%s", usernames[0]);
		for (i = 1; i < bmnum - 1; i++)
			sprintf(genbuf, "%s %s", genbuf, usernames[i]);
	} else {
		genbuf[0] = '\0';
	}
	strncpy(fh.BM, genbuf, sizeof (fh.BM));
	if (fh.BM[0] != '\0') {
		//added by infotech
		strcpy(buf[0], fh.BM);
#ifdef BMNAMELISTLIMIT
		for (i = 0; i < BMNAMELISTLEN && buf[0][i]; i++);
		if (i == BMNAMELISTLEN) {
			buf[0][i++] = '.';
			buf[0][i++] = '.';
			buf[0][i++] = '.';
			buf[0][i] = '\0';
		}
#endif
		//endadd
		//sprintf(genbuf, "%-38.38s(BM: %s)", fh.title +8, fh.BM);
		//精华区的显示: 动态分配        显示10个空格 printf("%*c",10,' ');
		{
			int blanklen; //前两个空间大小
			static const char BLANK = ' ';
			blanklen = STRLEN - strlen(fh.title + 8) - strlen(buf[0]) - 7;
			blanklen /= 2;
			blanklen = (blanklen > 0) ? blanklen : 1;
			sprintf(genbuf, "%s%*c(BM: %s)", fh.title + 8, blanklen,
					BLANK, buf[0]);
		}
	} else {
		sprintf(genbuf, "%-38.38s", fh.title + 8);
	}
	get_grp(fh.filename);
	edit_grp(fh.filename, lookgrp, fh.title + 8, genbuf);
	substitute_record(BOARDS, &fh, sizeof (fh), pos);
	sprintf(genbuf, "取消 %s 的 %s 版版主职务", lookupuser.userid, fh.filename);
	securityreport(genbuf, 0, 1);
	move(8, 0);
	prints(genbuf);
	sethomefile(bmfilename, lookupuser.userid, ".bmfile");
	if (oldbm - 1) {
		bmfp = fopen(bmfilename, "w+");
		for (i = 0; i < oldbm - 1; i++)
			fprintf(bmfp, "%s\n", bnames[i]);
		fclose(bmfp);
	} else {
		char secu[STRLEN];

		unlink(bmfilename);
		if (!(lookupuser.userlevel & PERM_OBOARDS) //没有讨论区管理权限
				&& !(lookupuser.userlevel & PERM_SYSOPS) //没有站务权限
		) {
			lookupuser.userlevel &= ~PERM_BOARDS;
			substitut_record(PASSFILE, &lookupuser, sizeof(struct userec),
					id);
			sprintf(secu, "版主卸职, 取消 %s 的版主权限", lookupuser.userid);
			securityreport(secu, 0, 1);
			move(9, 0);
			prints(secu);
		}
	}
	prints("\n\n");
	if (askyn("需要在相关版面发送通告吗?", YEA, NA) == NA) {
		pressanykey();
		return 0;
	}
	prints("\n");
	if (askyn("正常离任请按 Enter 键确认，撤职惩罚按 N 键", YEA, NA) == YEA)
		right = 1;
	else
		right = 0;
	if (right)
		sprintf(bmfilename, "%s 版%s %s 离任通告", bname, bm ? "版主" : "版副",
				lookupuser.userid);
	else
		sprintf(bmfilename, "[通告]撤除 %s 版%s %s", bname, bm ? "版主" : "版副",
				lookupuser.userid);
	strcpy(currboard, bname);
	if (right) {
		sprintf(genbuf, "\n\t\t\t【 通告 】\n\n"
			"\t经站务委员会讨论：\n"
			"\t同意 %s 辞去 %s 版的%s职务。\n"
			"\t在此，对其曾经在 %s 版的辛苦劳作表示感谢。\n\n"
			"\t希望今后也能支持本版的工作.\n", lookupuser.userid, bname, bm ? "版主"
				: "版副", bname);
	} else {
		sprintf(genbuf, "\n\t\t\t【撤职通告】\n\n"
			"\t经站务委员会讨论决定：\n"
			"\t撤除 %s 版%s %s 的%s职务。\n", bname, bm ? "版主" : "版副",
				lookupuser.userid, bm ? "版主" : "版副");
	}
	for (i = 0; i < 5; i++)
		buf[i][0] = '\0';
	move(14, 0);
	prints("请输入%s附言(最多五行，按 Enter 结束)", right ? "版主离任" : "版主撤职");
	for (i = 0; i < 5; i++) {
		getdata(i + 15, 0, ": ", buf[i], STRLEN - 5, DOECHO, YEA);
		if (buf[i][0] == '\0')
			break;
		//      if(i == 0) strcat(genbuf,right?"\n\n离任附言：\n":"\n\n撤职说明：\n");
		if (i == 0)
			strcat(genbuf, "\n\n");
		strcat(genbuf, "\t");
		strcat(genbuf, buf[i]);
		strcat(genbuf, "\n");
	}
	//autoreport(bmfilename,genbuf,YEA,NULL);
	autoreport(bmfilename, genbuf, YEA, lookupuser.userid, 1);

	/* Added by Amigo 2002.07.01. Send BM assign mail to mail box. */
	/*	{
	 FILE *	se;
	 char 	fname[STRLEN];

	 sprintf( fname, "tmp/AutoPoster.%s.%05d", currentuser.userid, uinfo.pid );
	 if( ( se = fopen(fname,"w") ) != NULL ){
	 fprintf( se, "%s", genbuf );
	 fclose( se );
	 if( lookupuser.userid != NULL )
	 mail_file( fname, lookupuser.userid, bmfilename );
	 }
	 }*/
	/* Add end. */
	prints("\n执行完毕！");
	pressanykey();
	return 0;
}

//  开设新版
int m_newbrd() {
	struct boardheader newboard, fh;
	char ans[20];
	char vbuf[100];
	char *group;
	int bid, pos;

	if (!(HAS_PERM(PERM_BLEVELS)))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	stand_title("开启新讨论区");
	memset(&newboard, 0, sizeof (newboard));
	move(2, 0);
	ansimore2("etc/boardref", NA, 3, 7);
	while (1) {
		getdata(10, 0, "讨论区名称:   ", newboard.filename, 18, DOECHO, YEA);
		if (newboard.filename[0] != 0) {
			struct boardheader dh;
			if (search_record(BOARDS, &dh, sizeof (dh), cmpbnames,
					newboard.filename)) {
				prints("\n错误! 此讨论区已经存在!!");
				pressanykey();
				return -1;
			}
		} else {
			return -1;
		}
		if (valid_brdname(newboard.filename))
			break;
		prints("\n不合法名称!!");
	}
	newboard.flag = 0;
	while (1) {
		getdata(11, 0, "讨论区说明:   ", newboard.title, 60, DOECHO, YEA);
		if (newboard.title[0] == '\0')
			return -1;
		if (strstr(newboard.title, "●") || strstr(newboard.title, "⊙")) {
			newboard.flag |= BOARD_OUT_FLAG;
			break;
		} else if (strstr(newboard.title, "○")) {
			newboard.flag &= ~BOARD_OUT_FLAG;
			break;
		} else {
			prints("错误的格式, 无法判断是否转信!!");
		}
	}
	strcpy(vbuf, "vote/");
	strcat(vbuf, newboard.filename);
	setbpath(genbuf, newboard.filename);
	if (getbnum(newboard.filename) > 0 || mkdir(genbuf, 0755) == -1
			|| mkdir(vbuf, 0755) == -1) {
		prints("\n错误的讨论区名称!!\n");
		pressreturn();
		clear();
		return -1;
	}
	//sprintf(vbuf, "/dev/shm/bbs/boards/%s", newboard.filename);
	//mkdir(vbuf, 0755);

	move(12, 0);
	if (gettheboardname(12, "输入所属讨论区名: ", &pos, &fh, ans, 2)) {
		newboard.group = pos;
	} else {
		newboard.group = 0;
		newboard.flag |= BOARD_NOZAP_FLAG; //root dir can't zap.Danielfree 06.2.22
	}
	if (askyn("本版是目录吗?", NA, NA) == YEA) {
		newboard.flag |= BOARD_DIR_FLAG;
		//suggest by monoply.06.2.22
		newboard.flag |= BOARD_JUNK_FLAG;
		newboard.flag |= BOARD_NOREPLY_FLAG;
		newboard.flag |= BOARD_POST_FLAG;
		if (askyn("是否限制存取权力", NA, NA) == YEA) {
			getdata(14, 0, "限制 Read? [R]: ", ans, 2, DOECHO, YEA);
			move(1, 0);
			clrtobot();
			move(2, 0);
			prints("设定 %s 权力. 讨论区: '%s'\n", "READ", newboard.filename);
			newboard.level = setperms(newboard.level, "权限", NUMPERMS,
					showperminfo);
			clear();
		} else {
			newboard.level = 0;
		}
		//add  end
	} else {
		newboard.flag &= ~BOARD_DIR_FLAG;

		if (askyn("本版诚征版主吗(否则由SYSOPs管理)?", YEA, NA) == NA) {
			strcpy(newboard.BM, "SYSOPs");
		} else {
			newboard.BM[0] = '\0';
		}

		if (askyn("该版的全部文章均不可以回复", NA, NA) == YEA) {
			newboard.flag |= BOARD_NOREPLY_FLAG;
		} else {
			newboard.flag &= ~BOARD_NOREPLY_FLAG;
		}

		if (askyn("是否是俱乐部版面", NA, NA) == YEA) {
			newboard.flag |= BOARD_CLUB_FLAG;
			if (askyn("是否读限制俱乐部版面", NA, NA) == YEA) {
				newboard.flag |= BOARD_READ_FLAG;
			} else {
				newboard.flag &= ~BOARD_READ_FLAG;
			}
		} else {
			newboard.flag &= ~BOARD_CLUB_FLAG;
		}

		if (askyn("是否不计算文章数", NA, NA) == YEA) {
			newboard.flag |= BOARD_JUNK_FLAG;
		} else {
			newboard.flag &= ~BOARD_JUNK_FLAG;
		}

		if (askyn("是否加入匿名版", NA, NA) == YEA) {
			newboard.flag |= BOARD_ANONY_FLAG;
		} else {
			newboard.flag &= ~BOARD_ANONY_FLAG;
		}
#ifdef ENABLE_PREFIX
		if (askyn ("是否强制使用前缀", NA, NA) == YEA) {
			newboard.flag |= BOARD_PREFIX_FLAG;
		} else {
			newboard.flag &= ~BOARD_PREFIX_FLAG;
		}
#endif
		if (askyn("是否限制存取权力", NA, NA) == YEA) {
			getdata(14, 0, "限制 Read/Post? [R]: ", ans, 2, DOECHO, YEA);
			if (*ans == 'P' || *ans == 'p') {
				newboard.flag |= BOARD_POST_FLAG;
			} else {
				newboard.flag &= ~BOARD_POST_FLAG;
			}
			move(1, 0);
			clrtobot();
			move(2, 0);
			prints("设定 %s 权力. 讨论区: '%s'\n", (newboard.flag
					& BOARD_POST_FLAG ? "POST" : "READ"),
					newboard.filename);
			newboard.level = setperms(newboard.level, "权限", NUMPERMS,
					showperminfo);
			clear();
		} else {
			newboard.level = 0;
		}
	}
	if (askyn("是否 可以 ZAP讨论区？", (newboard.flag & BOARD_NOZAP_FLAG) ? NA
			: YEA, YEA) == NA) {
		newboard.flag |= BOARD_NOZAP_FLAG;
	} else {
		newboard.flag &= ~BOARD_NOZAP_FLAG;
	}
	if ((bid = getblankbnum()) > 0) {
		substitute_record(BOARDS, &newboard, sizeof (newboard), bid);
		flush_bcache();
	} else if (append_record(BOARDS, &newboard, sizeof (newboard)) == -1) {
		pressreturn();
		clear();
		return -1;
	}

	if (!(newboard.flag & BOARD_DIR_FLAG)) {
		group = chgrp();
		if (group != NULL) {
			if (newboard.BM[0] != '\0') {
				sprintf(vbuf, "%-38.38s(BM: %s)", newboard.title + 8,
						newboard.BM);
			} else {
				sprintf(vbuf, "%-38.38s", newboard.title + 8);
			}
			if (add_grp(group, cexplain, newboard.filename, vbuf) == -1) {
				prints("\n成立精华区失败....\n");
			} else {
				prints("已经置入精华区...\n");
			}
		}
	}

	flush_bcache();
	rebuild_brdshm(); //add by cometcaptor 2006-10-13
	prints("\n新讨论区成立\n");

	char secu[STRLEN];
	sprintf(secu, "成立新版：%s", newboard.filename);
	securityreport(secu, 0, 1);

	pressreturn();
	clear();
	return 0;
}

//      修改讨论区设定
int m_editbrd() {
	char bname[STRLEN], buf[STRLEN], oldtitle[STRLEN], vbuf[256], *group;
	char type[10];
	char oldpath[STRLEN], newpath[STRLEN], tmp_grp[30];
	int pos, tmppos, a_mv;
	struct boardheader fh, newfh, tmpfh;
	extern struct boardheader *bcache;
	a_mv = 0; // added by Danielfree 05.12.4

	//added by roly 02.03.07
	if (!(HAS_PERM(PERM_BLEVELS)))
		return;
	//add end

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	stand_title("修改讨论区资讯");
	if (!gettheboardname(2, "输入讨论区名称: ", &pos, &fh, bname, 0))
		return -1;
	if (fh.flag & BOARD_DIR_FLAG)
		sprintf(type, "目录");
	else
		sprintf(type, "版面");
	move(2, 0);
	memcpy(&newfh, &fh, sizeof (newfh));
	while (1) {
		clear();
		stand_title("修改讨论区资讯");
		move(2, 0);
		prints("1)修改%s名称:            %s\n", type, newfh.filename);
		prints("2)修改%s说明:            %s\n", type, newfh.title);
		prints("3)修改%s管理员:          %s\n", type, newfh.BM);
		prints("4)修改%s所属目录:        %s(%d)\n", type,
				bcache[fh.group - 1].filename, newfh.group);
		if (fh.flag & BOARD_DIR_FLAG) {
			prints("5)修改%s读写属性:        %s\n", type,
					(newfh.level == 0) ? "没有限制" : "r(限制阅读)");
		} else {
			prints("5)修改%s读写属性:        %s\n", type, (newfh.flag
					& BOARD_POST_FLAG) ? "p(限制发文)"
					: (newfh.level == 0) ? "没有限制" : "r(限制阅读)");
		}
		//zap dir and board. Danielfree 06.2.22
		prints("6)可以ZAP%s:             %s\n", type, (newfh.flag
				& BOARD_NOZAP_FLAG) ? "可" : "否");
		if (!(newfh.flag & BOARD_DIR_FLAG)) {
			prints("7)移动精华区位置\n");
			//prints ("7)可以ZAP版面:             %s\n",
			//    (newfh.flag & BOARD_POST_FLAG) ? "可" : "否");
			prints("8)匿名版面:                %s\n", (newfh.flag
					& BOARD_ANONY_FLAG) ? "匿名" : "不匿名");
			prints("9)可以回复:                %s\n", (newfh.flag
					& BOARD_NOREPLY_FLAG) ? "不可回复" : "可以回复");
			prints("A)是否计算文章数:          %s\n", (newfh.flag
					& BOARD_JUNK_FLAG) ? "不计算" : "计算");
			prints(
					"B)俱乐部属性:              %s\n",
					(newfh.flag & BOARD_CLUB_FLAG) ? (newfh.flag
							& BOARD_READ_FLAG) ? "\033[1;31mc\033[0m(读限制俱乐部)"
							: "\033[1;33mc\033[0m(普通俱乐部)"
							: "不是俱乐部");
#ifdef ENABLE_PREFIX
			prints ("C)是否强制使用前缀:        %s\n",
					(newfh.flag & BOARD_PREFIX_FLAG) ? "必须" : "不必");
#endif
			getdata(14, 0, "更改哪项资讯？[1-9,A,B][0]", genbuf, 2, DOECHO, YEA);
		} else {
			getdata(14, 0, "更改哪项资讯？[1-6][0]", genbuf, 2, DOECHO, YEA);
		}
		if (genbuf[0] == '0' || genbuf[0] == 0)
			break;
		move(15, 0);
		strcpy(oldtitle, fh.title);
		switch (genbuf[0]) {
			case '1':
				while (1) {
					sprintf(buf, "新讨论区名称[%s]: ", fh.filename);
					getdata(15, 0, buf, genbuf, 18, DOECHO, YEA);
					if (*genbuf != 0) {
						struct boardheader dh;
						if (search_record(BOARDS, &dh, sizeof (dh),
								cmpbnames, genbuf)) {
							move(16, 0);
							prints("错误! 此讨论区已经存在!!");
							move(0, 0);
							clrtoeol();
							continue;
						}
						if (valid_brdname(genbuf)) {
							strncpy(newfh.filename, genbuf,
									sizeof (newfh.filename));
							strcpy(bname, genbuf);
							break;
						} else {
							move(16, 0);
							prints("不合法的讨论区名称!");
							move(0, 0);
							clrtoeol();
							continue;
						}
					} else {
						break;
					}
				}
				break;
			case '2':
				ansimore2("etc/boardref", NA, 11, 7);
				snprintf(genbuf, sizeof(newfh.title), "%s", newfh.title);
				while (1) {
					getdata(22, 0, "新讨论区说明: ", genbuf, 60, DOECHO, YEA);
					if (*genbuf != 0) {
						strncpy(newfh.title, genbuf, sizeof (newfh.title));
					} else {
						break;
					}
					if (strstr(newfh.title, "●") || strstr(newfh.title,
							"⊙")) {
						newfh.flag |= BOARD_OUT_FLAG;
						break;
					} else if (strstr(newfh.title, "○")) {
						newfh.flag &= ~BOARD_OUT_FLAG;
						break;
					} else {
						prints("\n错误的格式, 无法判断是否转信!!");
					}
				}
				break;
			case '3':
				if (fh.BM[0] != '\0' && strcmp(fh.BM, "SYSOPs")) {
					if (askyn("修改讨论区管理员。注意：仅供出错修正使用，版主任免请勿改动此处！", NA, NA)
							== YEA) {
						getdata(16, 0, "讨论区管理员: ", newfh.BM,
								sizeof (newfh.BM), DOECHO, YEA);
						if (newfh.BM[0] == '\0') {
							strcpy(newfh.BM, fh.BM);
						} else if (newfh.BM[0] == ' ') {
							newfh.BM[0] = '\0';
						}
					}
				} else {
					if (askyn("本版诚征版主吗(否，则由SYSOPs管理)?", YEA, NA) == NA) {
						strncpy(newfh.BM, "SYSOPs", sizeof (newfh.BM));
					} else {
						strncpy(newfh.BM, "\0", sizeof (newfh.BM));
					}
				}
				break;
			case '4':
				if (gettheboardname(15, "输入所属讨论区名: ", &tmppos, &tmpfh,
						genbuf, 2))
					newfh.group = tmppos;
				else if (askyn("所属讨论区为根目录么？", NA, NA) == YEA)
					newfh.group = 0;
				break;
			case '5':
				if (fh.flag & BOARD_DIR_FLAG) { //modiy for dir. Danielfree 06.2.23
					sprintf(buf, "(N)不限制 (R)限制阅读 [%c]: ",
							(newfh.level) ? 'R' : 'N');
					getdata(15, 0, buf, genbuf, 2, DOECHO, YEA);
					if (genbuf[0] == 'N' || genbuf[0] == 'n') {
						newfh.flag &= ~BOARD_POST_FLAG;
						newfh.level = 0;
					} else {
						if (genbuf[0] == 'R' || genbuf[0] == 'r')
							newfh.flag &= ~BOARD_POST_FLAG;
						clear();
						move(2, 0);
						prints("设定 %s '%s' 讨论区的权限\n", "阅读", newfh.filename);
						newfh.level = setperms(newfh.level, "权限",
								NUMPERMS, showperminfo);
						clear();
					}
				} // if dir
				else { //if board
					sprintf(buf, "(N)不限制 (R)限制阅读 (P)限制张贴 文章 [%c]: ",
							(newfh.flag & BOARD_POST_FLAG) ? 'P' : (newfh.
							level) ? 'R' : 'N');
					getdata(15, 0, buf, genbuf, 2, DOECHO, YEA);
					if (genbuf[0] == 'N' || genbuf[0] == 'n') {
						newfh.flag &= ~BOARD_POST_FLAG;
						newfh.level = 0;
					} else {
						if (genbuf[0] == 'R' || genbuf[0] == 'r')
							newfh.flag &= ~BOARD_POST_FLAG;
						else if (genbuf[0] == 'P' || genbuf[0] == 'p')
							newfh.flag |= BOARD_POST_FLAG;
						clear();
						move(2, 0);
						prints("设定 %s '%s' 讨论区的权限\n", newfh.flag
								& BOARD_POST_FLAG ? "张贴" : "阅读",
								newfh.filename);
						newfh.level = setperms(newfh.level, "权限",
								NUMPERMS, showperminfo);
						clear();
					}
				}
				break;
				//both dir and board can zap. Danielfree 06.2.22
			case '6':
				if (askyn("是否 可以 ZAP讨论区？",
						(fh.flag & BOARD_NOZAP_FLAG) ? NA : YEA, YEA)
						== NA) {
					newfh.flag |= BOARD_NOZAP_FLAG;
				} else {
					newfh.flag &= ~BOARD_NOZAP_FLAG;
				}
				break;
				//modify end
			default:
				if (!(fh.flag & BOARD_DIR_FLAG)) {
					switch (genbuf[0]) {
						case '7':
							a_mv = 2;
							break; // move from out of default into default -.- Danielfree 05.12.4
						case '8':
							if (askyn("是否匿名版？", (fh.flag
									& BOARD_ANONY_FLAG) ? YEA : NA, NA)
									== YEA) {
								newfh.flag |= BOARD_ANONY_FLAG;
							} else {
								newfh.flag &= ~BOARD_ANONY_FLAG;
							}

							break;
						case '9':
							if (askyn("文章是否 可以 回复？", (fh.flag
									& BOARD_NOREPLY_FLAG) ? NA : YEA, YEA)
									== NA) {
								newfh.flag |= BOARD_NOREPLY_FLAG;
							} else {
								newfh.flag &= ~BOARD_NOREPLY_FLAG;
							}
							break;
						case 'a':
						case 'A':
							if (askyn("是否 不计算 文章数？", (fh.flag
									& BOARD_JUNK_FLAG) ? YEA : NA, NA)
									== YEA) {
								newfh.flag |= BOARD_JUNK_FLAG;
							} else {
								newfh.flag &= ~BOARD_JUNK_FLAG;
							}
							break;
						case 'b':
						case 'B':
							if (askyn("是否俱乐部版面？", (fh.flag
									& BOARD_CLUB_FLAG) ? YEA : NA, NA)
									== YEA) {
								newfh.flag |= BOARD_CLUB_FLAG;
								if (askyn("是否读限制俱乐部？", (fh.flag
										& BOARD_READ_FLAG) ? YEA : NA, NA)
										== YEA) {
									newfh.flag |= BOARD_READ_FLAG;
								} else {
									newfh.flag &= ~BOARD_READ_FLAG;
								}
							} else {
								newfh.flag &= ~BOARD_CLUB_FLAG;
								newfh.flag &= ~BOARD_READ_FLAG;
							}
							break;
#ifdef ENABLE_PREFIX
							case 'c':
							case 'C':
							if (askyn("是否强制使用前缀？", (fh.flag & BOARD_PREFIX_FLAG) ? YEA : NA, NA)
									== YEA) {
								newfh.flag |= BOARD_PREFIX_FLAG;
							} else {
								newfh.flag &= ~BOARD_PREFIX_FLAG;
							}
#endif
					}//wswitch
				}//if dir
		}//switch
	}//while
	getdata(23, 0, "确定要更改吗? (Y/N) [N]: ", genbuf, 4, DOECHO, YEA);
	if (*genbuf == 'Y' || *genbuf == 'y') {
		char secu[STRLEN];
		sprintf(secu, "修改讨论区：%s(%s)", fh.filename, newfh.filename);
		securityreport(secu, 0, 1);
		if (strcmp(fh.filename, newfh.filename)) {
			char old[256], tar[256];
			a_mv = 1;
			setbpath(old, fh.filename);
			setbpath(tar, newfh.filename);
			rename(old, tar);
			sprintf(old, "vote/%s", fh.filename);
			sprintf(tar, "vote/%s", newfh.filename);
			rename(old, tar);
		}
		if (newfh.BM[0] != '\0')
			sprintf(vbuf, "%-38.38s(BM: %s)", newfh.title + 8, newfh.BM);
		else
			sprintf(vbuf, "%-38.38s", newfh.title + 8);
		get_grp(fh.filename);
		edit_grp(fh.filename, lookgrp, oldtitle + 8, vbuf);
		if (a_mv >= 1) {
			group = chgrp();
			get_grp(fh.filename);
			strcpy(tmp_grp, lookgrp);
			if (strcmp(tmp_grp, group) || a_mv == 1) {
				char tmpbuf[160];
				sprintf(tmpbuf, "%s:", fh.filename);
				del_from_file("0Announce/.Search", tmpbuf);
				if (group != NULL) {
					if (newfh.BM[0] != '\0')
						sprintf(vbuf, "%-38.38s(BM: %s)", newfh.title + 8,
								newfh.BM);
					else
						sprintf(vbuf, "%-38.38s", newfh.title + 8);
					if (add_grp(group, cexplain, newfh.filename, vbuf)
							== -1)
						prints("\n成立精华区失败....\n");
					else
						prints("已经置入精华区...\n");
					sprintf(newpath, "0Announce/groups/%s/%s", group,
							newfh.filename);
					sprintf(oldpath, "0Announce/groups/%s/%s", tmp_grp,
							fh.filename);
					if (strcmp(oldpath, newpath) != 0 && dashd(oldpath)) {
						deltree(newpath);
						rename(oldpath, newpath);
						del_grp(tmp_grp, fh.filename, fh.title + 8);
					} // add by Danielfree,suggest by fancitron 05.12.4
				} // if group !=NULL
			} // if strcmp
		} // if a_mv >= 1
		substitute_record(BOARDS, &newfh, sizeof (newfh), pos);
		sprintf(genbuf, "更改讨论区 %s 的资料 --> %s", fh.filename, newfh.filename);
		report(genbuf);
		// numboards = -1;/* force re-caching */
		flush_bcache();
	} // if askyn
	clear();
	return 0;

}

// 批注册单时显示的标题
void regtitle() {
	prints("[1;33;44m批注册单 NEW VERSION wahahaha                                                   [m\n");
	prints(" 离开[[1;32m←[m,[1;32me[m] 选择[[1;32m↑[m,[1;32m↓[m] 阅读[[1;32m→[m,[1;32mRtn[m] 批准[[1;32my[m] 删除[[1;32md[m]\n");

	prints("[1;37;44m  编号 用户ID       姓  名       系别             住址             注册时间     [m\n");
}

//      在批注册单时显示的注册ID列表
char *regdoent(int num, REGINFO * ent) {
	static char buf[128];
	char rname[17];
	char dept[17];
	char addr[17];
	//struct tm* tm;
	//tm=gmtime(&ent->regdate);
	strncpy(rname, ent->realname, 12);
	strncpy(dept, ent->dept, 16);
	strncpy(addr, ent->addr, 16);
	ellipsis(rname, 12);
	ellipsis(dept, 16);
	ellipsis(addr, 16);
	getdatestring(ent->regdate, 2);
	sprintf(buf, "  %4d %-12s %-12s %-16s %-16s %s", num, ent->userid,
			rname, dept, addr, datestring);
	return buf;
}

//      返回userid 与ent->userid是否相等
int filecheck(REGINFO * ent, char *userid) {
	return !strcmp(ent->userid, userid);
}

// 删除注册单文件里的一个记录
int delete_register(int index, REGINFO * ent, char *direct) {
	delete_record(direct, sizeof(REGINFO), index, filecheck, ent->userid);
	return DIRCHANGED;
}

//      通过注册单
int pass_register(int index, REGINFO * ent, char *direct) {
	int unum;
	struct userec uinfo;
	char buf[80];
	FILE *fout;

	strncpy(uinfo.realname, ent->realname, NAMELEN);
	unum = getuser(ent->userid);
	if (!unum) {
		clear();
		prints("系统错误! 查无此账号!\n"); //      在回档或者某些情况下,找不到在注册单文件
		pressanykey(); // unregister中的此记录,故删除
		delete_record(direct, sizeof(REGINFO), index, filecheck,
				ent->userid);
		return DIRCHANGED;
	}

	delete_record(direct, sizeof(REGINFO), index, filecheck, ent->userid);

	memcpy(&uinfo, &lookupuser, sizeof (uinfo));
	strncpy(uinfo.address, ent->addr, NAMELEN);
	sprintf(genbuf, "%s$%s@%s", ent->dept, ent->phone, currentuser.userid);
	genbuf[STRLEN - 16] = '\0';
	strncpy(uinfo.reginfo, genbuf, STRLEN - 17);
#ifdef ALLOWGAME
	uinfo.money = 1000;
#endif
	uinfo.lastjustify = time(0);
	substitut_record(PASSFILE, &uinfo, sizeof (uinfo), unum);
	sethomefile(buf, uinfo.userid, "register");
	if ((fout = fopen(buf, "a")) != NULL) {
		getdatestring(ent->regdate, YEA);
		fprintf(fout, "注册时间     : %s\n", datestring);
		fprintf(fout, "申请帐号     : %s\n", ent->userid);
		fprintf(fout, "真实姓名     : %s\n", ent->realname);
		fprintf(fout, "学校系级     : %s\n", ent->dept);
		fprintf(fout, "目前住址     : %s\n", ent->addr);
		fprintf(fout, "联络电话     : %s\n", ent->phone);
#ifndef FDQUAN
		fprintf(fout, "电子邮件     : %s\n", ent->email);
#endif
		fprintf(fout, "校 友 会     : %s\n", ent->assoc);
		getdatestring(time(0), YEA);
		fprintf(fout, "成功日期     : %s\n", datestring);
		fprintf(fout, "批准人       : %s\n", currentuser.userid);
		fclose(fout);
	}
	mail_file("etc/s_fill", uinfo.userid, "恭禧您，您已经完成注册。");
	sethomefile(buf, uinfo.userid, "mailcheck");
	unlink(buf);
	sprintf(genbuf, "让 %s 通过身分确认.", uinfo.userid);
	securityreport(genbuf, 0, 0);

	return DIRCHANGED;
}

//      处理注册单
int do_register(int index, REGINFO * ent, char *direct) {
	int unum;
	struct userec uinfo;
	//char ps[80];
	register int ch;
	static char *reason[] = { "请确实填写真实姓名.", "请详填学校科系与年级.", "请填写完整的住址资料.",
			"请详填联络电话.", "请确实填写注册申请表.", "请用中文填写申请单.", "其他" };
	unsigned char rejectindex = 4;

	if (!ent)
		return DONOTHING;

	unum = getuser(ent->userid);
	if (!unum) {
		prints("系统错误! 查无此账号!\n"); //删除不存在的记录,如果有的话
		delete_record(direct, sizeof(REGINFO), index, filecheck,
				ent->userid);
		return DIRCHANGED;
	}

	memcpy(&uinfo, &lookupuser, sizeof (uinfo));
	clear();
	move(0, 0);
	prints("[1;33;44m 详细资料                                                                      [m\n");
	prints("[1;37;42m [.]接受 [+]拒绝 [d]删除 [0-6]不符合原因                                       [m");

	//strcpy(ps, "(无)");
	for (;;) {
		disply_userinfo(&uinfo);
		move(14, 0);
		printdash(NULL);
		getdatestring(ent->regdate, YEA);
		prints("   注册时间   : %s\n", datestring);
		prints("   申请帐号   : %s\n", ent->userid);
		prints("   真实姓名   : %s\n", ent->realname);
		prints("   学校系级   : %s\n", ent->dept);
		prints("   目前住址   : %s\n", ent->addr);
		prints("   联络电话   : %s\n", ent->phone);
#ifndef FDQUAN
		prints("   电子邮件   : %s\n", ent->email);
#endif
		prints("   校 友 会   : %s\n", ent->assoc);
		ch = egetch();
		switch (ch) {
			case '.':
				pass_register(index, ent, direct);
				return READ_AGAIN;
			case '+':
				uinfo.userlevel &= ~PERM_SPECIAL4;
				substitut_record(PASSFILE, &uinfo, sizeof (uinfo), unum);
				//mail_file("etc/f_fill", uinfo.userid, "请重新填写您的注册资料");
				mail_file("etc/f_fill", uinfo.userid, reason[rejectindex]);
			case 'd':
				uinfo.userlevel &= ~PERM_SPECIAL4;
				substitut_record(PASSFILE, &uinfo, sizeof (uinfo), unum);
				delete_register(index, ent, direct);
				return READ_AGAIN;
			case KEY_DOWN:
			case '\r':
				return READ_NEXT;
			case KEY_LEFT:
				return DIRCHANGED;
			default:
				if (ch >= '0' && ch <= '6') {
					rejectindex = ch - '0';
					//strcpy(uinfo.address, reason[ch-'0']);
				}
				break;
		}
	}
	return 0;
}

struct one_key reg_comms[] = { 'r', do_register, 'y', pass_register, 'd',
		delete_register, '\0', NULL };
void show_register() {
	FILE *fn;
	int x; //, y, wid, len;
	char uident[STRLEN];
	if (!(HAS_PERM(PERM_USER)))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	stand_title("查询使用者注册资料");
	move(1, 0);
	usercomplete("请输入要查询的代号: ", uident);
	if (uident[0] != '\0') {
		if (!getuser(uident)) {
			move(2, 0);
			prints("错误的使用者代号...");
		} else {
			sprintf(genbuf, "home/%c/%s/register",
					toupper(lookupuser.userid[0]), lookupuser.userid);
			if ((fn = fopen(genbuf, "r")) != NULL) {
				prints("\n注册资料如下:\n\n");
				for (x = 1; x <= 15; x++) {
					if (fgets(genbuf, STRLEN, fn))
						prints("%s", genbuf);
					else
						break;
				}
			} else {
				prints("\n\n找不到他/她的注册资料!!\n");
			}
		}
	}
	pressanykey();
}
//  进入 注册单察看栏,看使用者的注册资料或进注册单管理程序
int m_register() {
	FILE *fn;
	char ans[3]; //, *fname;
	int x; //, y, wid, len;
	char uident[STRLEN];

	if (!(HAS_PERM(PERM_USER)))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();

	stand_title("设定使用者注册资料");
	for (;;) {
		getdata(1, 0, "(0)离开  (1)审查新注册 (2)查询使用者注册资料 ? : ", ans, 2, DOECHO,
				YEA);
		if (ans[0] == '1' || ans[0] == '2') { // || ans[0]=='3') 现在只有0,1,2
			break;
		} else {
			return 0;
		}
	}
	switch (ans[0]) {
		/*
		 case '1':
		 fname = "new_register";
		 if ((fn = fopen(fname, "r")) == NULL) {
		 prints("\n\n目前并无新注册资料.");
		 pressreturn();
		 } else {
		 y = 3, x = wid = 0;
		 while (fgets(genbuf, STRLEN, fn) != NULL && x < 65) {
		 if (strncmp(genbuf, "userid: ", 8) == 0) {
		 move(y++, x);
		 prints("%s",genbuf + 8);
		 len = strlen(genbuf + 8);
		 if (len > wid)
		 wid = len;
		 if (y >= t_lines - 2) {
		 y = 3;
		 x += wid + 2;
		 }
		 }
		 }
		 fclose(fn);
		 if (askyn("设定资料吗", NA, YEA) == YEA) {
		 securityreport("设定使用者注册资料");
		 scan_register_form(fname);
		 }
		 }
		 break; */
		case '2':
			move(1, 0);
			usercomplete("请输入要查询的代号: ", uident);
			if (uident[0] != '\0') {
				if (!getuser(uident)) {
					move(2, 0);
					prints("错误的使用者代号...");
				} else {
					sprintf(genbuf, "home/%c/%s/register",
							toupper(lookupuser.userid[0]),
							lookupuser.userid);
					if ((fn = fopen(genbuf, "r")) != NULL) {
						prints("\n注册资料如下:\n\n");
						for (x = 1; x <= 15; x++) {
							if (fgets(genbuf, STRLEN, fn))
								prints("%s", genbuf);
							else
								break;
						}
					} else {
						prints("\n\n找不到他/她的注册资料!!\n");
					}
				}
			}
			pressanykey();
			break;
		case '1':
			i_read(ADMIN, "unregistered", regtitle, regdoent,
					&reg_comms[0], sizeof(REGINFO));
			break;
	}
	clear();
	return 0;
}

//      删除讨论区
int d_board() {
	struct boardheader binfo;
	int bid, ans;
	char bname[STRLEN];
	extern char lookgrp[];
	char genbuf_rm[STRLEN]; //added by roly 02.03.24

	if (!HAS_PERM(PERM_BLEVELS)) {
		return 0;
	}
	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	stand_title("删除讨论区");
	make_blist(0); //生成讨论区列表
	move(1, 0);
	namecomplete("请输入讨论区: ", bname);
	if (bname[0] == '\0')
		return 0;
	bid = getbnum(bname);
	if (get_record(BOARDS, &binfo, sizeof (binfo), bid) == -1) { //取得讨论区的记录
		move(2, 0);
		prints("不正确的讨论区\n");
		pressreturn();
		clear();
		return 0;
	}
	if (binfo.BM[0] != '\0' && strcmp(binfo.BM, "SYSOPs")) { //还有不是叫SYSOPs的版主
		move(5, 0);
		prints("该版还有版主，在删除本版前，请先取消版主的任命。\n");
		pressanykey();
		clear();
		return 0;
	}
	ans = askyn("你确定要删除这个讨论区", NA, NA);
	if (ans != 1) {
		move(2, 0);
		prints("取消删除行动\n");
		pressreturn();
		clear();
		return 0;
	}
	{
		char secu[STRLEN];
		sprintf(secu, "删除讨论区：%s", binfo.filename);
		securityreport(secu, 0, 1);
	}
	if (seek_in_file("0Announce/.Search", bname)) {
		move(4, 0);
		if (askyn("移除精华区", NA, NA) == YEA) {
			get_grp(binfo.filename);
			del_grp(lookgrp, binfo.filename, binfo.title + 8);
		}
	}
	if (seek_in_file("etc/junkboards", bname))
		del_from_file("etc/junkboards", bname);
	if (seek_in_file("0Announce/.Search", bname)) {
		char tmpbuf[160];
		sprintf(tmpbuf, "%s:", bname);
		del_from_file("0Announce/.Search", tmpbuf);
	}
	if (binfo.filename[0] == '\0') {
		return -1; /* rrr - precaution */
	}
	sprintf(genbuf, "boards/%s", binfo.filename);
	//f_rm(genbuf);
	/* added by roly 02.03.24 */
	sprintf(genbuf_rm, "/bin/rm -fr %s", genbuf); //added by roly 02.03.24
	system(genbuf_rm); //与f_rm(genbuf)是不是重复了?
	/* add end */
	sprintf(genbuf, "vote/%s", binfo.filename);
	//f_rm(genbuf);
	/* added by roly 02.03.24 */
	sprintf(genbuf_rm, "/bin/rm -fr %s", genbuf); //added by roly 02.03.24
	system(genbuf_rm);
	/* add end */
	sprintf(genbuf, " << '%s' 被 %s 删除 >>", binfo.filename,
			currentuser.userid);
	memset(&binfo, 0, sizeof (binfo));
	strncpy(binfo.title, genbuf, STRLEN);
	binfo.level = PERM_SYSOPS;
	substitute_record(BOARDS, &binfo, sizeof (binfo), bid);

	move(4, 0);
	prints("\n本讨论区已经删除...\n");
	pressreturn();
	flush_bcache();
	clear();
	return 0;
}

//      删除一个帐号
int d_user(char *cid) {
	int id, num, i;
	char secu[STRLEN];
	char genbuf_rm[STRLEN]; //added by roly 02.03.24
	char passbuf[PASSLEN];

	if (!(HAS_PERM(PERM_USER)))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	stand_title("删除使用者帐号");
	// Added by Ashinmarch in 2008.10.20 
	// 砍掉账号时增加密码验证
	getdata(1, 0, "[1;37m请输入密码: [m", passbuf, PASSLEN, NOECHO, YEA);
	passbuf[8] = '\0';
	if (!checkpasswd(currentuser.passwd, passbuf)) {
		prints("[1;31m密码输入错误...[m\n");
		return 0;
	}
	// Add end.
	if (!gettheuserid(1, "请输入欲删除的使用者代号: ", &id))
		return 0;
	if (!strcmp(lookupuser.userid, "SYSOP")) {
		prints("\n对不起，你不可以删除 SYSOP 帐号!!\n");
		pressreturn();
		clear();
		return 0;
	}
	if (!strcmp(lookupuser.userid, currentuser.userid)) {
		prints("\n对不起，你不可以删除自己的这个帐号!!\n");
		pressreturn();
		clear();
		return 0;
	}
	prints("\n\n以下是 [%s] 的部分资料:\n", lookupuser.userid);
	prints("    User ID:  [%s]\n", lookupuser.userid);
	prints("    昵   称:  [%s]\n", lookupuser.username);
	prints("    姓   名:  [%s]\n", lookupuser.realname);
	strcpy(secu, "ltmprbBOCAMURS#@XLEast0123456789\0");
	for (num = 0; num < strlen(secu) - 1; num++) {
		if (!(lookupuser.userlevel & (1 << num)))
			secu[num] = '-';
	}
	prints("    权   限: %s\n\n", secu);

	num = getbnames(lookupuser.userid, secu, &num);
	if (num) {
		prints("[%s] 目前尚担任了 %d 个版的版主: ", lookupuser.userid, num);
		for (i = 0; i < num; i++)
			prints("%s ", bnames[i]);
		prints("\n请先使用版主卸职功能取消其版主职务再做该操作.");
		pressanykey();
		clear();
		return 0;
	}

	sprintf(genbuf, "你确认要删除 [%s] 这个 ID 吗", lookupuser.userid);
	if (askyn(genbuf, NA, NA) == NA) {
		prints("\n取消删除使用者...\n");
		pressreturn();
		clear();
		return 0;
	}
	sprintf(secu, "删除使用者：%s", lookupuser.userid);
	securityreport(secu, 0, 0);
	sprintf(genbuf, "mail/%c/%s", toupper(lookupuser.userid[0]),
			lookupuser.userid);
	//f_rm(genbuf);
	/* added by roly 02.03.24 */
	sprintf(genbuf_rm, "/bin/rm -fr %s", genbuf); //added by roly 02.03.24
	system(genbuf_rm);
	/* add end */
	sprintf(genbuf, "home/%c/%s", toupper(lookupuser.userid[0]),
			lookupuser.userid);
	//f_rm(genbuf);
	/* added by roly 02.03.24 */
	sprintf(genbuf_rm, "/bin/rm -fr %s", genbuf); //added by roly 02.03.24
	system(genbuf_rm);
	/* add end */
	lookupuser.userlevel = 0;
	strcpy(lookupuser.address, "");
#ifdef ALLOWGAME
	lookupuser.money = 0;
	lookupuser.nummedals = 0;
	lookupuser.bet = 0;
#endif
	strcpy(lookupuser.username, "");
	strcpy(lookupuser.realname, "");
	strcpy(lookupuser.termtype, "");
	prints("\n%s 已经被灭绝了...\n", lookupuser.userid);
	lookupuser.userid[0] = '\0';
	substitut_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
	setuserid(id, lookupuser.userid);
	pressreturn();
	clear();
	return 1;
}

//      更改使用者的权限
int x_level() {
	int id;
	char reportbuf[60];
	unsigned int newlevel;

	if (!HAS_PERM(PERM_SYSOPS))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	move(0, 0);
	prints("更改使用者权限\n");
	clrtoeol();
	move(1, 0);
	usercomplete("输入欲更改的使用者帐号: ", genbuf);
	if (genbuf[0] == '\0') {
		clear();
		return 0;
	}
	if (!(id = getuser(genbuf))) {
		move(3, 0);
		prints("Invalid User Id");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	move(1, 0);
	clrtobot();
	move(2, 0);
	prints("设定使用者 '%s' 的权限 \n", genbuf);
	newlevel
			= setperms(lookupuser.userlevel, "权限", NUMPERMS, showperminfo);
	move(2, 0);
	if (newlevel == lookupuser.userlevel)
		prints("使用者 '%s' 权限没有变更\n", lookupuser.userid);
	else {
		sprintf(reportbuf, "change level: %s %.8x -> %.8x",
				lookupuser.userid, lookupuser.userlevel, newlevel);
		report(reportbuf);
		lookupuser.userlevel = newlevel;
		{
			char secu[STRLEN];
			sprintf(secu, "修改 %s 的权限", lookupuser.userid);
			securityreport(secu, 0, 0);
		}

		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
		if (!(lookupuser.userlevel & PERM_REGISTER)) {
			char src[STRLEN], dst[STRLEN];
			sethomefile(dst, lookupuser.userid, "register.old");
			if (dashf(dst))
				unlink(dst);
			sethomefile(src, lookupuser.userid, "register");
			if (dashf(src))
				rename(src, dst);
		}
		prints("使用者 '%s' 权限已经更改完毕.\n", lookupuser.userid);
	}
	pressreturn();
	clear();
	return 0;
}

//added by iamfat 2002.07.22
//rewrite by iamfat 2002.08.19
extern void list_text(char *fname, void (*title_show) (), int (*key_deal) (), int (*ifcheck) ());
extern void changereason(char *fname);
extern void setreasondefault();
extern void setreason(char *rsn, int i);
extern char *getreason();
extern char *getdetailreason();
char *denylevellist = ".DenyLevel";
int add_denylevel(char *line);

char date_name[STRLEN];

//恢复所有到期全站处罚
extern int text_find(char *fname, int no, char *str, char *line);

//显示处罚到期的ID列表标题.
void denylist_title_show() {
	move(0, 0);
	prints("[1;44;36m 处罚到期的ID列表                                                              [m\n");
	prints(" 离开[[1;32m←[m] 选择[[1;32m↑[m,[1;32m↓[m] 添加[[1;32ma[m]  修改[[1;32mc[m] 恢复[[1;32md[m] 到期[[1;32mx[m] 查找[[1;32m/[m]\n");
	prints("[1;44m 用户代号     处罚说明(A-Z;'.[])                 权限 结束日期   站务          [m\n");
}

// 处理显示封禁列表时的按键处理
int denylist_key_deal(char *fname, int ch, char *line) {
	switch (ch) {
		case 'a': //添加
			add_denylevel(0);
			break;
		case 'c': //修改
			if (!line)
				break;
			add_denylevel(line);
			break;
		case 'd': //恢复
			if (!line)
				break;
			move(1, 0);
			if (askyn("您确定吗?", NA, NA) == NA) {
				denylist_title_show();
				return 0;
			}
			del_from_denylist(fname, line);
			break;
		case 'x': //释放所有到期的
			move(1, 0);
			if (askyn("您确定吗?", NA, NA) == NA) {
				denylist_title_show();
				return 0;
			}
			del_from_denylist(fname, NULL);
			break;
		case Ctrl('A'):
		case KEY_RIGHT: //用户信息
			if (!line)
				return 0;
			t_query(line);
			break;
	}
	return 1;
}

//  进入修改封禁权限列表
int x_new_denylevel() {
	if (!HAS_PERM(PERM_OBOARDS) && !HAS_PERM(PERM_SPECIAL0))
		return;
	modify_user_mode(ADMIN);
	list_text(denylevellist, denylist_title_show, denylist_key_deal, NULL);
}

// 返回权限ch相对应的消息
char *clevel(char ch) {
	switch (ch) {
		case '1':
			return "发文";
		case '2':
			return "登陆";
		case '3':
			return "聊天";
		case '4':
			return "信件";
		case '5':
			return "消息";
	}
	return "未知";
}

// 返回st相对应的整数时间值.
time_t get_denydate(char *st) {
	time_t tt;
	struct tm tms;
	int ch1, ch2, ch3;
	if (!strncmp(st, "待定", 4))
		return 0;
	if (!strncmp(st, "终身", 4))
		return 0x7fffffff;
	if (!strncmp(st, "永久", 4))
		return 0x7fffffff;
	ch1 = st[4];
	st[4] = '\0'; //年
	ch2 = st[7];
	st[7] = '\0'; //月
	ch3 = st[10];
	st[10] = '\0'; //日
	tms.tm_year = atoi(st) - 1900; //      年
	tms.tm_mon = atoi(st + 5) - 1; //      月
	tms.tm_mday = atoi(st + 8); //      日
	st[4] = ch1;
	st[7] = ch2;
	st[10] = ch3;
	tms.tm_hour = tms.tm_min = tms.tm_sec = 0;
	tt = mktime(&tms); //生成tms相对应的时间
	return tt;
}

//      添加到封禁列表
int

add_to_denylist(char *uident, char ch, int day, char *msg) {
	char strtosave[STRLEN];
	char line[256];
	char luid[IDLEN + 1];
	char fname[STRLEN];
	char ps[40];
	FILE *fpr, *fpw;
	int added = 0;
	int change = YEA;
	struct tm *tmtime;
	time_t tt;
	time_t tt2;

	getdata(12, 0, "输入说明: ", ps, 40, DOECHO, YEA);
	move(13, 0);
	if (askyn("您确定吗?", NA, NA) == NA)
		return NA;
	sprintf(fname, "%s.%d", denylevellist, uinfo.pid);
	if (!(fpr = fopen(denylevellist, "r")))
		return NA;
	if (!(fpw = fopen(fname, "w")))
		return NA;
	if (day != 999) {
		day = (day > 999) ? 999 : day;
	}
	if (day == 999) {
		tt = 0x7fffffff;
	} else if (day > 0) {
		tt = time(0) + (day) * 24 * 60 * 60;
	} else {
		tt = 0;
	}
	//commented by iamfat 2002.10.08
	/*
	 while(fgets(line,256,fpr))
	 {
	 strncpy(luid,line,IDLEN);
	 luid[IDLEN]='\0';
	 strtok(luid," \r\n\t");
	 if(!strcmp(luid, uident) && !strncmp(line+48, clevel(ch),4))
	 {
	 if(tt<=get_denydate(line+53))
	 {
	 change=NA;
	 break;
	 }
	 }
	 } */
	if (change == YEA) {
		if (day == 999) {
			sprintf(strtosave, "%-12s %-34s %-4s 终身       %-12s\n",
					uident, getreason(), clevel(ch), currentuser.userid);
			sprintf(msg, "%s因:\n%s\n应被封禁 %s 权限终身\n\n如不服本决定,"
				" 可以联系处罚决定人或在7日内到Appeal申请复议。\n"
				"P.S.: %s\n\n执行人: %s\n", uident, getdetailreason(),
					clevel(ch), ps, currentuser.userid);
		} else if (day > 0) {
			tmtime = localtime(&tt);
			sprintf(strtosave, "%-12s %-34s %-4s %04d.%02d.%02d %-12s\n",
					uident, getreason(), clevel(ch), 1900
							+ tmtime->tm_year, tmtime->tm_mon + 1,
					tmtime->tm_mday, currentuser.userid);
			sprintf(msg,
					"%s因:\n%s\n应被封禁 %s 权限%d天\n\n请在处罚期满后(%04d.%02d.%02d),"
						" 向%s写信要求解除处罚.\n如不服本决定,"
						" 可以联系处罚决定人或在7日内到Appeal申请复议。\n"
						"P.S.: %s\n\n执行人: %s\n", uident,
					getdetailreason(), clevel(ch), day, 1900
							+ tmtime->tm_year, tmtime->tm_mon + 1,
					tmtime->tm_mday, currentuser.userid, ps,
					currentuser.userid);
		} else { //待定
			sprintf(strtosave, "%-12s %-34s %-4s 待定       %-12s\n",
					uident, getreason(), clevel(ch), currentuser.userid);
			sprintf(msg, "%s因:\n%s\n暂时被封禁 %s 权限\n\n处罚决定稍后做出\n"
				"如不服本决定, 可以联系处罚决定人或在7日内到Appeal申请复议。\n"
				"P.S.: %s\n\n执行人: %s\n", uident, getdetailreason(),
					clevel(ch), ps, currentuser.userid);
		}
		fseek(fpr, 0, SEEK_SET);
		while (fgets(line, 256, fpr)) {
			tt2 = get_denydate(line + 53);
			strncpy(luid, line, IDLEN);
			luid[IDLEN] = '\0';
			strtok(luid, " \r\n\t");
			if (!strcmp(luid, uident)
					&& !strncmp(line + 48, clevel(ch), 4))
				continue;
			if (!added && tt < tt2) {
				fputs(strtosave, fpw);
				added = 1;
			}
			fputs(line, fpw);
		} // while(fgets(line,256,fpr)
		if (!added)
			fputs(strtosave, fpw);
	} // if change == YEA
	fclose(fpw);
	fclose(fpr);

	if (change == YEA)
		rename(fname, denylevellist);
	else
		unlink(fname);

	return change;
}

// 恢复用户的权限
int release_user(char *line) {
	int id;
	FILE *se;
	char uident[IDLEN + 1];
	char fname[STRLEN];
	char secu[STRLEN];
	char rep[STRLEN];
	char msgbuf[256];
	strncpy(uident, line, IDLEN);
	uident[IDLEN] = '\0';
	strtok(uident, " \r\n\t");
	if (!(id = getuser(uident)))
		return -1;
	if (!strncmp(&line[48], clevel('1'), 4)) {
		lookupuser.userlevel |= PERM_POST;
		sprintf(secu, "[站内公告]恢复%s发表文章的权力", lookupuser.userid);
		sprintf(rep, "被%s恢复发表文章的权力", currentuser.userid);
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	} else if (!strncmp(&line[48], clevel('2'), 4)) {
		lookupuser.userlevel |= PERM_LOGIN;
		sprintf(secu, "[站内公告]恢复%s基本上站的权力", lookupuser.userid);
		sprintf(rep, "被%s恢复基本上站的权力", currentuser.userid);
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	} else if (!strncmp(&line[48], clevel('3'), 4)) {
		lookupuser.userlevel |= PERM_TALK;
		sprintf(secu, "[站内公告]恢复%s与他人聊天的权力", lookupuser.userid);
		sprintf(rep, "被%s恢复与他人聊天的权力", currentuser.userid);
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	} else if (!strncmp(&line[48], clevel('4'), 4)) {
		lookupuser.userlevel |= PERM_MAIL;
		sprintf(secu, "[站内公告]恢复%s发送信件的权力", lookupuser.userid);
		sprintf(rep, "被%s恢复发送信件的权力", currentuser.userid);
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	} else {
		lookupuser.userlevel |= PERM_POST;
		sprintf(secu, "[站内公告]恢复%s未知权力", lookupuser.userid);
		sprintf(rep, "被%s恢复未知权力", currentuser.userid);
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	}
	//发信给本人
	sprintf(fname, "tmp/AutoPoster.%s.%05d", currentuser.userid, uinfo.pid);
	sprintf(msgbuf, "执行人:%s\n", currentuser.userid);
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "%s", msgbuf);
		fclose(se);
		if (lookupuser.userid != NULL)
			mail_file(fname, lookupuser.userid, secu);
	}
	Poststring(msgbuf, "Notice", secu, 1);
	//安全记录
	securityreport(secu, 0, 0);
	log_DOTFILE(lookupuser.userid, rep);
	substitute_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	sprintf(msgbuf, "释放 %s", line);
	do_report(".Released", strtok(msgbuf, "\r\n"));
}

//      从封禁列表中释放
int del_from_denylist(char *fname, char *line) {
	FILE *fpr, *fpw;
	int deleted = NA;
	char tmpbuf[256], fnnew[STRLEN];
	char tmpdate[11];

	if (!(fpr = fopen(fname, "r")))
		return -1;
	sprintf(fnnew, "%s.%d", fname, uinfo.pid);
	if (!(fpw = fopen(fnnew, "w")))
		return -1;
	if (line) {
		while (fgets(tmpbuf, 256, fpr)) {
			if (deleted == NA && !strcmp(tmpbuf, line))
				deleted = YEA;
			else
				fputs(tmpbuf, fpw);
		}
		if (deleted == YEA)
			release_user(line);
	} else {
		time_t tt;
		time_t now = time(0);
		while (fgets(tmpbuf, 256, fpr)) {
			//pighead      A-----------------------------     发文 2002.07.24 iamfat
			//123456789012345678901234567890123456789012345678901234
			strncpy(tmpdate, tmpbuf + 53, 10);
			tt = get_denydate(tmpdate);
			if (tt != 0 && tt <= now) {
				deleted = YEA;
				release_user(tmpbuf);
			} else {
				fputs(tmpbuf, fpw);
			}
		}
	}
	fclose(fpw);
	fclose(fpr);
	if (deleted == YEA)
		return rename(fnnew, fname);
	return 0;
}

//  修改封禁权限
int add_denylevel(char *line) {
	int id;
	char ans[7];
	char buf2[5];
	char msgbuf[4096];
	char genbuf[STRLEN];
	char secu[STRLEN];
	char rep[STRLEN];
	char deny_uid[IDLEN + 1];

	int bDeny = NA;
	modify_user_mode(ADMIN);
	if (!check_systempasswd())
		return 0;
	move(1, 0);
	if (line) {
		strncpy(deny_uid, line, IDLEN);
		deny_uid[IDLEN] = '\0';
		strtok(deny_uid, " \n\r\t");
		setreason(line + IDLEN + 1, strlen(getreason()));
		prints("修改%s的封禁时间\n", deny_uid);
	} else {
		usercomplete("封禁使用者: ", deny_uid);
		if (*deny_uid == '\0')
			return 0;
		setreasondefault();
		prints("\n");
	}
	if (!(id = getuser(deny_uid))) {
		prints("非法的使用者帐号!\n");
		clrtoeol();
		egetch();
		return 0;
	}
	if (line) {
		if (!strncmp(&line[48], clevel('1'), 4))
			ans[0] = '1';
		else if (!strncmp(&line[48], clevel('2'), 4))
			ans[0] = '2';
		else if (!strncmp(&line[48], clevel('3'), 4))
			ans[0] = '3';
		else if (!strncmp(&line[48], clevel('4'), 4))
			ans[0] = '4';
		else
			ans[0] = '0';
		sprintf(secu, "[站内公告]修改%s的封禁时间", lookupuser.userid);
	} else {
		changereason("etc/denylevel");
		clear();
		move(2, 0);
		prints("设定使用者%s的基本权限或资料 \n\n", deny_uid);
		prints("(1) 封禁发表文章权力 %s\n",
				(lookupuser.userlevel & PERM_POST) ? "ON" : "OFF");
		prints("(2) 取消基本上站权力 %s\n",
				(lookupuser.userlevel & PERM_LOGIN) ? "ON" : "OFF");
		prints("(3) 禁止与他人聊天   %s\n",
				(lookupuser.userlevel & PERM_TALK) ? "ON" : "OFF");
		prints("(4) 禁止发送信件     %s\n",
				(lookupuser.userlevel & PERM_MAIL) ? "ON" : "OFF");
		while (1) {
			move(12, 0);
			clrtobot();
			getdata(10, 0, "请输入你的处理: ", ans, 3, DOECHO, YEA);
			switch (ans[0]) {
				case '1':
					if (!(lookupuser.userlevel & PERM_POST)) {
						prints("\n%s发表文章的权力已被封禁!\n", lookupuser.userid);
						egetch();
					}
					sprintf(secu, "[站内公告]封禁%s发表文章的权力", lookupuser.userid);
					sprintf(rep, "被%s封禁[站内]发表文章的权力", currentuser.userid);
					bDeny = YEA;
					lookupuser.userlevel &= ~PERM_POST;
					break;
				case '2':
					if (!(lookupuser.userlevel & PERM_LOGIN)) {
						prints("\n%s基本上站的权力已被封禁!\n", lookupuser.userid);
						egetch();
					}
					sprintf(secu, "[站内公告]封禁%s基本上站的权力", lookupuser.userid);
					sprintf(rep, "被%s封禁[站内]基本上站的权力", currentuser.userid);
					bDeny = YEA;
					lookupuser.userlevel &= ~PERM_LOGIN;
					break;
				case '3':
					if (!(lookupuser.userlevel & PERM_TALK)) {
						prints("\n%s与他人聊天的权力已被封禁!\n", lookupuser.userid);
						egetch();
					}
					sprintf(secu, "[站内公告]封禁%s与他人聊天的权力", lookupuser.userid);
					sprintf(rep, "被%s封禁[站内]与他人聊天的权力", currentuser.userid);
					bDeny = YEA;
					lookupuser.userlevel &= ~PERM_TALK;
					break;
				case '4':
					if (!(lookupuser.userlevel & PERM_MAIL)) {
						prints("\n%s发送信件的权力已被封禁!\n", lookupuser.userid);
						egetch();
					}
					sprintf(secu, "[站内公告]封禁%s发送信件的权力", lookupuser.userid);
					sprintf(rep, "被%s封禁[站内]发送信件的权力", currentuser.userid);
					bDeny = YEA;
					lookupuser.userlevel &= ~PERM_MAIL;
					break;
			} //switch ans[]
			if (bDeny == YEA)
				break;
		} // while (1)
		// switch (ans[0]) {
		// case '1':
		//lookupuser.userlevel &= ~PERM_POST;
		//break;
		// case '2':
		//lookupuser.userlevel &= ~PERM_LOGIN;
		//break;
		//  case '3':
		//lookupuser.userlevel &= ~PERM_TALK;
		//break;
		//  case '4':
		//lookupuser.userlevel &= ~PERM_MAIL;
		//break;
		//  default:
		//break;
		//  }
	}
	getdata(11, 0, "封禁天数(999-终身, 0-待定): ", buf2, 5, DOECHO, YEA);
	if (add_to_denylist(lookupuser.userid, ans[0], atoi(buf2), msgbuf)
			== NA) {
		return 0;
	}
	substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	sethomefile(genbuf, lookupuser.userid, "giveupBBS");
	if (dashf(genbuf))
		unlink(genbuf);
	{
		//发信给本人
		FILE *se;
		char fname[STRLEN];
		sprintf(fname, "tmp/AutoPoster.%s.%05d", currentuser.userid,
				uinfo.pid);
		if ((se = fopen(fname, "w")) != NULL) {
			fprintf(se, "%s", msgbuf);
			fclose(se);
			if (lookupuser.userid != NULL)
				mail_file(fname, lookupuser.userid, secu);
		}
	}
	Poststring(msgbuf, "Notice", secu, 1);
	//安全纪录
	securityreport(secu, 0, 0);
	log_DOTFILE(lookupuser.userid, rep);
	//if (!line)
	//  substitut_record (PASSFILE, &lookupuser, sizeof (struct userec), id);
	return 1;
}

void a_edits() {
	int aborted;
	char ans[7], buf[STRLEN], buf2[STRLEN];
	int ch, num, confirm;
	extern struct BCACHE *brdshm;
	static char *e_file[] = { "../Welcome", "../Welcome2", "issue",
			"logout", "../vote/notes", "hotspot", "menu.ini",
			"../.badname", "../.bad_email", "../.bad_host", "autopost",
			"junkboards", "sysops", "whatdate", "../NOLOGIN",
			"../NOREGISTER", "special.ini", "hosts", "restrictip",
			"freeip", "s_fill", "f_fill", "register", "firstlogin",
			"chatstation", "notbackupboards", "bbsnet.ini", "bbsnetip",
			"bbsnet2.ini", "bbsnetip2", NULL };
	static char *explain_file[] = { "特殊进站公布栏", "进站画面", "进站欢迎档", "离站画面",
			"公用备忘录", "系统热点", "menu.ini", "不可注册的 ID", "不可确认之E-Mail",
			"不可上站之位址", "每日自动送信档", "不算POST数的版", "管理者名单", "纪念日清单",
			"暂停登陆(NOLOGIN)", "暂停注册(NOREGISTER)", "个人ip来源设定档", "穿梭ip来源设定档",
			"只能登陆5id的ip设定档", "不受5 id限制的ip设定档", "注册成功信件", "注册失败信件",
			"新用户注册范例", "用户第一次登陆公告", "国际会议厅清单", "区段删除不需备份之清单",
			"BBSNET 转站清单", "穿梭限制ip", "BBSNET2 转站清单", "穿梭2限制IP", NULL };
	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	move(1, 0);
	prints("编修系统档案\n\n");
	for (num = 0; (HAS_PERM(PERM_ESYSFILE)) ? e_file[num] != NULL
			&& explain_file[num] != NULL : explain_file[num] != "menu.ini"; num++) {
		prints("[[1;32m%2d[m] %s", num + 1, explain_file[num]);
		if (num < 17)
			move(4 + num, 0);
		else
			move(num - 14, 50);
	}
	prints("[[1;32m%2d[m] 都不想改\n", num + 1);

	getdata(23, 0, "你要编修哪一项系统档案: ", ans, 3, DOECHO, YEA);
	ch = atoi(ans);
	if (!isdigit(ans[0]) || ch <= 0 || ch > num || ans[0] == '\n'
			|| ans[0] == '\0')
		return;
	ch -= 1;
	sprintf(buf2, "etc/%s", e_file[ch]);
	move(3, 0);
	clrtobot();
	sprintf(buf, "(E)编辑 (D)删除 %s? [E]: ", explain_file[ch]);
	getdata(3, 0, buf, ans, 2, DOECHO, YEA);
	if (ans[0] == 'D' || ans[0] == 'd') {
		sprintf(buf, "你确定要删除 %s 这个系统档", explain_file[ch]);
		confirm = askyn(buf, NA, NA);
		if (confirm != 1) {
			move(5, 0);
			prints("取消删除行动\n");
			pressreturn();
			clear();
			return;
		}
		{
			char secu[STRLEN];
			sprintf(secu, "删除系统档案：%s", explain_file[ch]);
			securityreport(secu, 0, 0);
		}
		unlink(buf2);
		move(5, 0);
		prints("%s 已删除\n", explain_file[ch]);
		pressreturn();
		clear();
		return;
	}
	modify_user_mode(EDITSFILE);
	aborted = vedit(buf2, NA, YEA); /* 不添加文件头, 允许修改头部信息 */
	clear();
	if (aborted != -1) {
		prints("%s 更新过", explain_file[ch]);
		{
			char secu[STRLEN];
			sprintf(secu, "修改系统档案：%s", explain_file[ch]);
			securityreport(secu, 0, 0);
		}

		if (!strcmp(e_file[ch], "../Welcome")) {
			unlink("Welcome.rec");
			prints("\nWelcome 记录档更新");
		} else if (!strcmp(e_file[ch], "whatdate")) {
			brdshm->fresh_date = time(0);
			prints("\n纪念日清单 更新");
		}
	}
	pressreturn();
}

// 全站广播...
int wall() {
	char passbuf[PASSLEN];

	if (!HAS_PERM(PERM_SYSOPS))
		return 0;
	// Added by Ashinmarch on 2008.10.20
	// 全站广播前增加密码验证
	clear();
	stand_title("全站广播!");
	getdata(1, 0, "[1;37m请输入密码: [m", passbuf, PASSLEN, NOECHO, YEA);
	passbuf[8] = '\0';
	if (!checkpasswd(currentuser.passwd, passbuf)) {
		prints("[1;31m密码输入错误...[m\n");
		return 0;
	}
	// Add end.

	modify_user_mode(MSG);
	move(2, 0);
	clrtobot();
	if (!get_msg("所有使用者", buf2, 1)) {
		return 0;
	}
	if (apply_ulist(dowall) == -1) {
		move(2, 0);
		prints("线上空无一人\n");
		pressanykey();
	}
	prints("\n已经广播完毕...\n");
	pressanykey();
	return 1;
}

// 设定系统密码
int setsystempasswd() {
	FILE *pass;
	char passbuf[20], prepass[20];
	modify_user_mode(ADMIN);
	if (!check_systempasswd())
		return;
	if (strcmp(currentuser.userid, "SYSOP")) {
		clear();
		move(10, 20);
		prints("对不起，系统密码只能由 SYSOP 修改！");
		pressanykey();
		return;
	}
	getdata(2, 0, "请输入新的系统密码(直接回车则取消系统密码): ", passbuf, 19, NOECHO, YEA);
	if (passbuf[0] == '\0') {
		if (askyn("你确定要取消系统密码吗?", NA, NA) == YEA) {
			unlink("etc/.syspasswd");
			securityreport("取消系统密码", 0, 0);
		}
		return;
	}
	getdata(3, 0, "确认新的系统密码: ", prepass, 19, NOECHO, YEA);
	if (strcmp(passbuf, prepass)) {
		move(4, 0);
		prints("两次密码不相同, 取消此次设定.");
		pressanykey();
		return;
	}
	if ((pass = fopen("etc/.syspasswd", "w")) == NULL) {
		move(4, 0);
		prints("系统密码无法设定....");
		pressanykey();
		return;
	}
	fprintf(pass, "%s\n", genpasswd(passbuf));
	fclose(pass);
	move(4, 0);
	prints("系统密码设定完成....");
	pressanykey();
	return;
}

//      进入c shell环境
int x_csh() {
	char buf[PASSLEN];
	int save_pager;
	int magic;

	if (!HAS_PERM(PERM_SYSOPS)) {
		return 0;
	}
	if (!check_systempasswd()) {
		return;
	}
	modify_user_mode(SYSINFO);
	clear();
	getdata(1, 0, "请输入通行暗号: ", buf, PASSLEN, NOECHO, YEA);
	if (*buf == '\0' || !checkpasswd(currentuser.passwd, buf)) {
		prints("\n\n暗号不正确, 不能执行。\n");
		pressreturn();
		clear();
		return;
	}
	randomize();
	magic = rand() % 1000;
	prints("\nMagic Key: %d", magic * 5 - 2);
	getdata(4, 0, "Your Key : ", buf, PASSLEN, NOECHO, YEA);
	if (*buf == '\0' || !(atoi(buf) == magic)) {
		securityreport("Fail to shell out", 0, 0);
		prints("\n\nKey 不正确, 不能执行。\n");
		pressreturn();
		clear();
		return;
	}
	securityreport("Shell out", 0, 0);
	modify_user_mode(SYSINFO);
	clear();
	refresh();
	reset_tty();
	save_pager = uinfo.pager;
	uinfo.pager = 0;
	update_utmp();
	do_exec("csh", NULL);
	restore_tty();
	uinfo.pager = save_pager;
	update_utmp();
	clear();
	return 0;
}
#endif
#endif

#ifdef CERTIFYMODE
char *certify_list = ".UNCERTIFIED";
//      认证文章?  似乎没有使用到过
void
certify_title_show () {
	move (0, 0);
	prints
	("[1;44;36m 待批准的文章列表                                                              [m\n");
	prints (" 离开[[1;32m←[m] 选择[[1;32m↑[m,[1;32m↓[m]"
			" 批阅[[1;32ma[m] 删除[[1;32md[m] 查找[[1;32m/[m]\n");
	prints ("[1;44m 用户代号     版面             文件名    "
			"                                      [m\n");
}

//认证时按键处理函数,似乎没有退出功能
int
certify_key_deal (char *fname, int ch, char *line) {
	char origin[128];
	char filename[STRLEN];
	char board[STRLEN];
	char direct[STRLEN];
	int ent = 0;
	int gotit = 0;
	struct fileheader fileinfo;
	char *p;
	FILE *fp;
	if (!line)
	return 0;
	strcpy (origin, line);
	p = strtok (line, " \t\r\n");
	p = strtok (NULL, " \t\r\n");
	strcpy (board, p);
	p = strtok (NULL, " \t\r\n");
	strcpy (filename, p);
	setbfile (direct, board, ".DIR");
	fp = fopen (direct, "rb");
	if (!fp)
	return 1;
	fileinfo.filename[0] = '\0';
	while (fread (&fileinfo, sizeof (struct fileheader), 1, fp)) {
		if (!strcmp (filename, fileinfo.filename)) {
			gotit = 1;
			break;
		}
		ent++;
	}
	fclose (fp);
	if (gotit) {
		brc_initial (board);
		switch (ch) {
			case KEY_RIGHT:
			//阅读文章
			fileinfo.accessed[0] |= FILE_DELETED;
			read_post (ent, &fileinfo, direct);
			break;
			case 'a': //通过
			move (1, 0);
			if ((ch == 'a' || ch == 'd') && askyn ("您确定吗?", NA, NA) == NA) {
				certify_title_show ();
				return 0;
			}
			certify_post (ent, &fileinfo, direct);
			//通过文章
			break;
			case 'd':
			if (DIRCHANGED != _del_post (ent, &fileinfo, direct, YEA, NA))
			return 1;
			break;
		} //switch (ch)
	} // if gotit

	if (!gotit || ch == 'a' || ch == 'd') {
		FILE *fpr, *fpw;
		char buf[256];
		char fnnew[STRLEN];
		if (!(fpr = fopen (fname, "r")))
		return 1;
		sprintf (fnnew, "%s.%d", fname, uinfo.pid);
		if (!(fpw = fopen (fnnew, "w")))
		return 1;
		while (fgets (buf, 256, fpr)) {
			if (strcmp (buf, origin))
			fputs (buf, fpw);
		}
		fclose (fpw);
		fclose (fpr);
		rename (fnnew, fname);
	} // if (!gotit ||ch =='a'||ch=='d')
	return 1;
}

// 进入认证管理列表
int
certify_admin () {
	modify_user_mode (ADMIN);
	list_text (certify_list, certify_title_show, certify_key_deal, NULL);
}

// 将需认证文件中的内容循环读出
int
certify_admin2 () {
	struct stat st;
	stat (certify_list, &st);
	while (st.st_size) {
		certify_admin ();
		stat (certify_list, &st);
	}
}
#endif
