/*
 $Id: postheader.c 366 2007-05-12 16:35:51Z danielfree $
 本页面主要处理发表文章，回复文章和发信件时的头处理
 */

#include "bbs.h"

extern int numofsig;
extern int noreply;
extern int mailtoauther;
extern struct boardheader *getbcache();
#ifdef ENABLE_PREFIX
char prefixbuf[MAX_PREFIX][6];
int numofprefix;
#endif
void check_title(char *title) {
	char tempfilename[50];
	trimboth(title);
	if (!strncasecmp(title, "Re:", 3) && !HAS_PERM(PERM_SYSOPS)) {
		sprintf(tempfilename, "Re：%s", &title[3]);
		strcpy(title, tempfilename);
	}
}
#ifdef ENABLE_PREFIX
void set_prefix() {
	char filename[STRLEN], buf[128];
	int i;
	FILE *fp;

	setvfile(filename, currboard, "prefix");
	if ((fp = fopen(filename, "r"))> 0) {
		for (i = 0; i < MAX_PREFIX; i++) {
			if (!fgets(buf, STRLEN, fp)) {
				break;
			}
			if (i == 0 && (buf[0] == '\n' || buf[0] == ' ') )
			buf[0] = '\0';
			strtok(buf, " \r\n\t");
			safe_strcpy(prefixbuf[i], buf);
			prefixbuf[i][4] = '\0';
		}
		numofprefix = i;
		fclose(fp);
	} else numofprefix = 0;
}

void print_prefixbuf(char *buf, int index) {
	int i;
	sprintf(buf, "前缀:");
	for (i = 0; i < numofprefix; i++ ) {
		if (i == 0 && prefixbuf[i][0] == '\0' )
		sprintf(buf, "%s 1.%s无%s", buf,
				(index == i+1)?"\033[1m":"",
				(index == i+1)?"\033[m":"");
		else
		sprintf(buf, "%s %d.%s%s%s", buf,i+1,
				(index == i+1)?"\033[1;33m":"",
				prefixbuf[i],
				(index == i+1)?"\033[m":"");
	}
	sprintf(buf, "%s[%d]:", buf, index);
}
#endif
int post_header(struct postheader *header) {
	int anonyboard = 0;
#ifdef ENABLE_PREFIX
	int index = 0;
	char pbuf[128];
#endif
	char r_prompt[20], mybuf[256], ans[5];
	char titlebuf[STRLEN];
	//在回复模式和投条时作为标题的buffer
	struct boardheader *bp;
#ifdef IP_2_NAME
	extern char fromhost[];
#endif
#ifdef RNDSIGN  //随机签名档
	/*rnd_sign=0表明非随机,=1表明随机*/
	int oldset, rnd_sign = 0;
#endif

	//对当前签名档的越界处理
	if (currentuser.signature > numofsig || currentuser.signature < 0) {
		currentuser.signature = 1;
	}
#ifdef RNDSIGN
	if (numofsig> 0) {
		if (DEFINE(DEF_RANDSIGN)) {
			oldset = currentuser.signature;
			srand((unsigned) time(0));
			currentuser.signature = (rand() % numofsig) + 1;//产生随机签名档
			rnd_sign = 1; //标记随机签名档
		} else {
			rnd_sign = 0;
		}
	}
#endif

	/*如果当前是回复模式，则把原标题copy到当前标题中，并标记当前为回复模式
	 否则当前标题为空*/
	if (header->reply_mode) {
		strcpy(titlebuf, header->title);
		header->include_mode = 'R'; //exchange 'R' and 'S' by Danielfree 07.04.05
	} else {
		titlebuf[0] = '\0';
#ifdef ENABLE_PREFIX
		set_prefix();
#endif
	}

	//bp记录当前所在版面的信息
	bp = getbcache(currboard);

	//如果是发表文章，则首先检查所在版面是否匿名（发信时不存在这个问题）
	if (header->postboard) {
		anonyboard = bp->flag & BOARD_ANONY_FLAG;
	}

#ifdef IP_2_NAME
	if (anonyboard && (fromhost[0] < '1'||fromhost[0]> '9'))
	anonyboard = 0;
#endif

	// modified by roly 02.03.07
	// modified by iamfat 02.10.30
	header->chk_anony = (anonyboard) ? 1 : 0;
	//header->chk_anony = 0;
	//modifiy end
#ifdef ENABLE_PREFIX
	if (numofprefix> 0)
	index = 1;
#endif	
	while (1) {
#ifdef ENABLE_PREFIX
		if (header->reply_mode) {
			sprintf(r_prompt, "引言模式 [[1m%c[m]", header->include_mode);
			move(t_lines - 4, 0);
		} else if (numofprefix == 0)
		move(t_lines - 4, 0);
		else
		move(t_lines - 5, 0);
#else
		if (header->reply_mode)
			sprintf(r_prompt, "引言模式 [[1m%c[m]", header->include_mode);
		move(t_lines - 4, 0);
#endif
		//清除该行内容
		clrtobot();

		//根据相应操作打印出当前信息
		prints(
				"[m%s [1m%s[m      %s    %s%s\n",
				(header->postboard) ? "发表文章於" : "收信人：",
				header->ds,
				(anonyboard) ? (header->chk_anony == 1 ? "[1m要[m使用匿名"
						: "[1m不[m使用匿名") : "",
				(header->postboard) ? ((noreply) ? "[本文[1;33m不可以[m回复"
						: "[本文[1;33m可以[m回复") : "",
				(header->postboard&&header->reply_mode) ? ((mailtoauther) ? ",且[1;33m发送[m本文至作者信箱]"
						: ",且[1;33m不发送[m本文至作者信箱]")
						: (header->postboard) ? "]" : "");
#ifdef ENABLE_PREFIX
		if (!header->reply_mode && numofprefix) {
			if (bp->flag & BOARD_PREFIX_FLAG && !header->title[0]) {
				index = 0;
				print_prefixbuf(pbuf, index);
				while (1) {
					getdata(t_lines - 4, 0, pbuf, ans, 2, DOECHO, YEA);
					if (!ans[0])
					return NA;
					index = ans[0] - '0';
					if (index> 0 && index<= numofprefix) {
						print_prefixbuf(pbuf, index);
						break;
					}
				}
			} else {
				print_prefixbuf(pbuf, index);
			}
			move(t_lines - 4, 0);
			prints(pbuf);
		}

		//对于回复和发信，title初始不为空.所以只有在发表文章时，才会出现"[正在设定主题]"
		move(t_lines-3, 0 );
#endif
		prints("使用标题: [1m%-50s[m\n",
				(header->title[0] == '\0') ? "[正在设定主题]" : header->title);
#ifdef RNDSIGN
		//在回复模式下会出现相应的引言模式信息
		prints("使用第 [1m%d[m 个签名档     %s %s", currentuser.signature
				,(header->reply_mode) ? r_prompt : "", (rnd_sign == 1) ? "[随机签名档]" : "");
#else
		prints("使用第 [1m%d[m 个签名档     %s", currentuser.signature,
				(header->reply_mode) ? r_prompt : "");
#endif
		//对于发表文章或者投条情况
		if (titlebuf[0] == '\0') {
			//move到相应的行，为输入做准备
			move(t_lines - 1, 0);
			if (header->postboard == YEA || strcmp(header->title, "没主题"))
				safe_strcpy(titlebuf, header->title);

			//从当前行获得用户输入放到titlebuf中，最多存入50-1个字节(此处会阻塞在用户输入上，只到响应enter)
			getdata(t_lines - 1, 0, "标题: ", titlebuf, 50, DOECHO, NA);
			check_title(titlebuf);

			//在用户输入为空的情况下，如果是发表文章则直接取消，如果是投条用户还可以继续，信头为没主题
			if (titlebuf[0] == '\0') {
				if (header->title[0] != '\0') {
					titlebuf[0] = ' ';
					continue;
				} else
					return NA;
			}

			//将用户设定title复制到相应结构中
			strcpy(header->title, titlebuf);
			continue;
		}

		trimboth(header->title); //add by money 2003.10.29.
		move(t_lines - 1, 0);

#ifdef ENABLE_PREFIX	
		sprintf(mybuf,
				"[1;32m0[m~[1;32m%d V[m看签名档%s [1;32mX[m随机签名档,[1;32mT[m标题%s%s%s,[1;32mQ[m放弃:",
				numofsig, (header->reply_mode) ? ",[1;32mS[m/[1;32mY[m/[1;32mN[m/[1;32mR[m/[1;32mA[m 引言模式" : " \033[1;32mF\033[m前缀",
				(anonyboard) ? "，[1;32mL[m匿名" : "",(header->postboard)?",[1;32mU[m属性":"",(header->postboard&&header->reply_mode)?",[1;32mM[m寄信":"");
#else
		sprintf(
				mybuf,
				"[1;32m0[m~[1;32m%d V[m看签名档%s [1;32mX[m随机签名档,[1;32mT[m标题%s%s%s,[1;32mQ[m放弃:",
				numofsig,
				(header->reply_mode) ? ",[1;32mS[m/[1;32mY[m/[1;32mN[m/[1;32mR[m/[1;32mA[m 引言模式"
						: "", (anonyboard) ? "，[1;32mL[m匿名" : "",
				(header->postboard) ? ",[1;32mU[m属性" : "",
				(header->postboard&&header->reply_mode) ? ",[1;32mM[m寄信"
						: "");
#endif
		//打印出提示信息，并阻塞在用户输入动作上,用户最多输入2个字节
		getdata(t_lines - 1, 0, mybuf, ans, 3, DOECHO, YEA);
		ans[0] = toupper(ans[0]);

		//用户对签名档设置，包括取消当前操作
		if ((ans[0] - '0') >= 0 && ans[0] - '0' <= 9) {
			//有效的签名档选择
			if (atoi(ans) <= numofsig)
				currentuser.signature = atoi(ans);
		} else if (ans[0] == 'Q' || ans[0] == 'q') { //取消当前动作
			return -1;
		}
		//对于回复模式
		else if (header->reply_mode && (ans[0] == 'Y' || ans[0] == 'N'
				|| ans[0] == 'A' || ans[0] == 'R'||ans[0]=='S')) {
			header->include_mode = ans[0];
		} //重新设置标题
		else if (ans[0] == 'T') {
			titlebuf[0] = '\0';
		}//对于匿名版的特别处理 
		else if (ans[0] == 'L' && anonyboard) {
			header->chk_anony = (header->chk_anony == 1) ? 0 : 1;
		}//对于文章中，可否回复的更改 
		else if (ans[0] == 'U' && header->postboard) {
			noreply = ~noreply;
		}//对于回复文章时的特殊属性设置 
		else if (ans[0] == 'M' && header->postboard && header->reply_mode) {
			mailtoauther = ~mailtoauther;
		}//对签名档的处理 
		else if (ans[0] == 'V') {
			setuserfile(mybuf, "signatures");
			if (askyn("预设显示前三个签名档, 要显示全部吗", NA, YEA) == YEA)
				ansimore(mybuf);
			else {
				clear();
				ansimore2(mybuf, NA, 0, 18);
			}
#ifdef RNDSIGN
		}//随机签名档 
		else if (ans[0] == 'X') {
			if (rnd_sign == 0 && numofsig != 0) {
				oldset = currentuser.signature;
				srand((unsigned) time(0));
				currentuser.signature = (rand() % numofsig) + 1;
				rnd_sign = 1;
			} else if (rnd_sign == 1 && numofsig != 0) {
				rnd_sign = 0;
				currentuser.signature = oldset;
			}
			ans[0] = ' ';
#endif
		}
#ifdef ENABLE_PREFIX
		//修改前缀
		else if (!header->reply_mode && numofprefix && ans[0] == 'F') {
			int i;
			getdata(t_lines - 1, 0, pbuf, ans, 3, DOECHO, YEA);
			i = ans[0] - '0';
			if (i >= 0 && i <= numofprefix &&
					!(i == 0 && (bp->flag & BOARD_PREFIX_FLAG)))
			index = i;
		}
#endif
		else {
			if (header->title[0] == '\0')
				return NA;
			else {
#ifdef ENABLE_PREFIX
				strcpy(header->prefix, index?prefixbuf[index - 1]:"");
#endif
				return YEA;
			}
		}
	}
}

//此方法未被使用，主要用于检查当前版面是否为匿名
int check_anonyboard(currboard) {
	struct boardheader *bp;
	int anonyboard = 0;
	bp = getbcache(currboard);
	anonyboard = bp->flag & BOARD_ANONY_FLAG;
	if (anonyboard == 8) {
		return YEA;
		//this is because anony_flag = 8 = 100 in binary
	} else {
		return NA;
	}

}
