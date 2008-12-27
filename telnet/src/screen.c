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
 $Id: screen.c 2 2005-07-14 15:06:08Z root $
 */

#include "bbs.h"
#include "screen.h"
#include "edit.h"
#include <sys/param.h>
#include <stdarg.h>

extern char clearbuf[];
extern char cleolbuf[];
extern char scrollrev[];
extern char strtstandout[];
extern char endstandout[];
extern int iscolor;
extern int clearbuflen;
extern int cleolbuflen;
extern int scrollrevlen;
extern int strtstandoutlen;
extern int endstandoutlen;
extern int editansi;

extern int automargins;
extern int dumb_term;
#define o_clear()     output(clearbuf,clearbuflen)
#define o_cleol()     output(cleolbuf,cleolbuflen)
#define o_scrollrev() output(scrollrev,scrollrevlen)
#define o_standup()   output(strtstandout,strtstandoutlen)
#define o_standdown() output(endstandout,endstandoutlen)

unsigned char scr_lns, scr_cols;
unsigned char cur_ln = 0, cur_col = 0;
int roll, scrollcnt;//roll 表示首行在big_picture的偏移量
//因为随着光标滚动,big_picture[0]可能不再保存第一行的数据
unsigned char docls;
unsigned char downfrom;
int standing = NA;
int inansi = NA;

struct screenline *big_picture = NULL;

//add by wujian
//	清除非哑终端中从当前行开始的n行
//	若超过屏幕,则从第一行接着清除(将第一行中的数据均置为0)
void clrnlines(int n) {
	register struct screenline *slp;
	register int i, k;
	if (dumb_term)
		return;
	for (i=cur_ln; i<cur_ln+n; i++) {
		slp = &big_picture[(i + roll) % scr_lns];
		slp->mode = 0;
		slp->oldlen = 255;
		slp->len = 0;
		for (k=0; k<LINELEN; k++)
			slp->data[k]=0;
	}
}

#ifdef ALLOWAUTOWRAP
//返回str中前num个字符中以ansi格式实际显示的字符数?
int seekthestr(char *str, int num)
{
	int len, i, ansi= NA;
	len = strlen(str);
	for(i=0;i<len;i++) {
		if(!(num--))
		break;
		if(str[i] == KEY_ESC) {
			ansi = YEA;
			continue;
		}
		if( ansi ) {
			if ( !strchr("[0123456789; ", str[i]))
			ansi = NA;
			continue;
			/*                      if (strchr("[0123456789; ", str[i]))
			 continue;
			 else if (isalpha(str[i])) {
			 ansi = NA;
			 continue;
			 }
			 else
			 break;
			 */
		} //if
		//		if(!(num--)) break;
	} //for
	return i;
}
#endif	

//返回字符串中属于 ansi的个数?	对后一个continue不太理解 
int num_ans_chr(char *str) {
	int len, i, ansinum, ansi;

	ansinum=0;
	ansi=NA;
	len=strlen(str);
	for (i=0; i < len; i++) {
		if (str[i] == KEY_ESC) {
			ansi = YEA;
			ansinum++;
			continue;
		}
		if (ansi) {
			if (!strchr("[0123456789; ", str[i]))
				ansi = NA;
			ansinum++;
			continue;
			/*
			 if (strchr("[0123456789; ", str[i]))
			 {
			 ansinum++;
			 continue;
			 }
			 else if (isalpha(str[i]))
			 {
			 ansinum++;
			 ansi = NA;
			 continue;
			 }
			 else
			 break;
			 */
		}
	}
	return ansinum;
}

//	初始化屏幕,将行数设成 slns ,将列数设置成LINELEN与scols的最小值,
//	其中LINELEN表示系统设置的最大列数,为256
//		将分配到的屏幕缓冲映射到big_picture, 供调用
void init_screen(int slns, int scols) {
	register struct screenline *slp;
	scr_lns = slns;
	scr_cols = Min(scols, LINELEN);
	big_picture = (struct screenline *) calloc(scr_lns,
			sizeof(struct screenline));
	for (slns = 0; slns < scr_lns; slns++) {
		slp = &big_picture[slns];
		slp->mode = 0;
		slp->len = 0;
		slp->oldlen = 0;
	}
	docls = YEA;
	downfrom = 0;
	roll = 0;
}

//对于哑终端或是big_picture中尚无内存映射,将t_columns设置成WRAPMARGIN
//	调用init_screen初始化终端
void initscr() {
	if (!dumb_term && !big_picture)
		t_columns = WRAPMARGIN;
	init_screen(t_lines, WRAPMARGIN);
}

int tc_col, tc_line; //terminal's current collumn,current line?

//	从老位置(was_col,was_ln)移动到新位置(new_col,new_ln)
void rel_move(int was_col, int was_ln, int new_col, int new_ln) {
	int ochar();
	extern char *BC;
	if (new_ln >= t_lines || new_col >= t_columns) //越界,返回
		return;
	tc_col = new_col;
	tc_line = new_ln;
	if ((new_col == 0) && (new_ln == was_ln + 1)) { //换行
		ochar('\n');
		if (was_col != 0) //到第一列位置,返回
			ochar('\r');
		return;
	}
	if ((new_col == 0) && (new_ln == was_ln)) { //不换行,到第一列位置,并返回
		if (was_col != 0)
			ochar('\r');
		return;
	}
	if (was_col == new_col && was_ln == new_ln)
		return;
	if (new_col == was_col - 1 && new_ln == was_ln) { //到前一行
		if (BC)
			tputs(BC, 1, ochar);
		else
			ochar(Ctrl('H'));
		return;
	}
	do_move(new_col, new_ln, ochar); //所有情况都不满足时,执行此函数
}

// 标准输出buf中的数据,	ds,de表示数据的区间,sso,eso也是
//		但当它们没有交集时,以ds,de为准
//		有交集时,取合集
//			但下限以ds为准,上限以de为准				跟直接取ds,de有什么区别?
///		对o_standup,o_standdown作用不太清楚
void standoutput(char * buf, int ds, int de, int sso, int eso) {
	int st_start, st_end;
	if (eso <= ds || sso >= de) {
		output(buf + ds, de - ds);
		return;
	}
	st_start = Max(sso, ds);
	st_end = Min(eso, de);
	if (sso > ds)
		output(buf + ds, sso - ds);
	o_standup();
	output(buf + st_start, st_end - st_start);
	o_standdown();
	if (de > eso)
		output(buf + eso, de - eso);
}

//	刷新屏幕
void redoscr() {
	register int i, j;
	int ochar();
	register struct screenline *bp = big_picture;
	if (dumb_term)
		return;
	o_clear();
	//清除缓冲
	tc_col = 0;
	tc_line = 0;
	for (i = 0; i < scr_lns; i++) {
		j = i + roll;
		while (j >= scr_lns)
			j -= scr_lns;
		if (bp[j].len == 0)
			continue;
		rel_move(tc_col, tc_line, 0, i);
		if (bp[j].mode & STANDOUT)
			standoutput(bp[j].data, 0, bp[j].len, bp[j].sso, bp[j].eso);
		else
			output(bp[j].data, bp[j].len);
		tc_col += bp[j].len;
		if (tc_col >= t_columns) {
			if (!automargins) {
				tc_col -= t_columns;
				tc_line++;
				if (tc_line >= t_lines)
					tc_line = t_lines - 1;
			} else
				tc_col = t_columns - 1;
		}
		bp[j].mode &= ~(MODIFIED);
		bp[j].oldlen = bp[j].len;
	}
	rel_move(tc_col, tc_line, cur_col, cur_ln);
	docls = NA;
	scrollcnt = 0;
	oflush();
}

//刷新缓冲区,重新显示屏幕?
void refresh() {
	register int i, j;
	register struct screenline *bp = big_picture;
	extern int automargins;
	extern int scrollrevlen;
#ifndef BBSD
	if (dumb_term)
		return;
#endif		
	if (num_in_buf() != 0)
		return;
	if ((docls) || (abs(scrollcnt) >= (scr_lns - 3))) {
		redoscr();
		return;
	}
	if (scrollcnt < 0) {
		if (!scrollrevlen) {
			redoscr();
			return;
		}
		rel_move(tc_col, tc_line, 0, 0);
		while (scrollcnt < 0) {
			o_scrollrev();
			scrollcnt++;
		}
	}
	if (scrollcnt > 0) {
		rel_move(tc_col, tc_line, 0, t_lines - 1);
		while (scrollcnt > 0) {
			ochar('\n');
			scrollcnt--;
		}
	}
	for (i = 0; i < scr_lns; i++) {
		j = i + roll;
		while (j >= scr_lns)
			j -= scr_lns;
		if (bp[j].mode & MODIFIED && bp[j].smod < bp[j].len) {
			bp[j].mode &= ~(MODIFIED); //若被修改,则输出
			if (bp[j].emod >= bp[j].len)
				bp[j].emod = bp[j].len - 1;
			rel_move(tc_col, tc_line, bp[j].smod, i);
			if (bp[j].mode & STANDOUT)
				standoutput(bp[j].data, bp[j].smod, bp[j].emod + 1,
						bp[j].sso, bp[j].eso);
			else
				output(&bp[j].data[bp[j].smod], bp[j].emod - bp[j].smod
						+ 1);
			tc_col = bp[j].emod + 1;
			if (tc_col >= t_columns) {
				if (automargins) {
					tc_col -= t_columns;
					tc_line++;
					if (tc_line >= t_lines)
						tc_line = t_lines - 1;
				} else
					tc_col = t_columns - 1;
			}
		}
		if (bp[j].oldlen > bp[j].len) {
			rel_move(tc_col, tc_line, bp[j].len, i);
			o_cleol();
		}
		bp[j].oldlen = bp[j].len;
	}
	rel_move(tc_col, tc_line, cur_col, cur_ln);
	oflush();
}

/*移动到第y行,第x列*/
void move(int y, int x) {
	cur_col = x /* +c_shift(y,x) */;
	cur_ln = y;
}

//	返回当前的行数到y,列数到x
void getyx(int *y, int *x) {
	*y = cur_ln;
	*x = cur_col /*-c_shift(y,x)*/;
}

//	清零	big_picture中的数据,roll,docls,downfrom
//	移动到位置(0,0)
void clear() {
	register int i;
	register struct screenline *slp;
	if (dumb_term)/*哑终端*/
		return;
	roll = 0;
	docls = YEA;
	downfrom = 0;
	for (i = 0; i < scr_lns; i++) {
		slp = &big_picture[i];
		slp->mode = 0;
		slp->len = 0;
		slp->oldlen = 0;
	}
	move(0, 0);
}

//清除big_picture中的第i行,将mode与len置0
void clear_whole_line(int i) {
	register struct screenline *slp = &big_picture[i];
	slp->mode = slp->len = 0;
	slp->oldlen = 79;
}

//	将从当前光标到行末的所有字符变成空格,达到清除的效果
void clrtoeol() {
	register struct screenline *slp;
	register int ln;

	if (dumb_term)
		return;
	standing = NA;
	ln = cur_ln + roll;
	while (ln >= scr_lns)
		//相当于ln%=scr_lns,取当前行在big_picture中的序号
		ln -= scr_lns;
	slp = &big_picture[ln];
	if (cur_col <= slp->sso)
		slp->mode &= ~STANDOUT; //将slp->mode第0位置0
	if (cur_col > slp->oldlen) {
		register int i;
		for (i = slp->len; i <= cur_col; i++)
			slp->data[i] = ' ';
	}
	slp->len = cur_col;
}

//从当前行清除到最后一行
void clrtobot() {
	register struct screenline *slp;
	register int i, j;
	if (dumb_term)
		return;
	for (i = cur_ln; i < scr_lns; i++) {
		j = i + roll;
		while (j >= scr_lns)
			//求j%scr_lns ? 因为减法比取余时间少?
			j -= scr_lns;
		slp = &big_picture[j];
		slp->mode = 0;
		slp->len = 0;
		if (slp->oldlen > 0)
			slp->oldlen = 255;
	}
}

//	将big_picture的STANDOUT位置0
void clrstandout() {
	register int i;
	if (dumb_term)
		return;
	for (i = 0; i < scr_lns; i++)
		big_picture[i].mode &= ~(STANDOUT);
}

static char nullstr[] = "(null)";

//以ANSI格式输出字符c,通常是作用在一个字符串上,以ANSI格式输出一个一个字符
void outc(register unsigned char c) {
	register struct screenline *slp;
	register unsigned char reg_col;
#ifndef BIT8
	c &= 0x7f; /*若定义了双字节,去掉最高位*/
#endif
	if (inansi == 1) { //inansi表示是否在ansi状态内
		if (c == 'm') {
			inansi = 0;
			return;
		}
		return;
	}
	if (c == KEY_ESC && iscolor == NA) {//进入ansi状态
		inansi = 1;
		return;
	}
	if (dumb_term) {
		if (!isprint2(c)) {
			if (c == '\n') { //换行
				ochar('\r');
			} else if (c != KEY_ESC || !showansi) {//不可打印字符显示为'*'
				c = '*';
			}
		}
		ochar(c);
		return;
	}
	if (1) {
		register int reg_line = cur_ln + roll;
		register int reg_scrln = scr_lns;
		while (reg_line > 0 && reg_line >= reg_scrln)
			reg_line -= reg_scrln;
		slp = &big_picture[reg_line];//获得当前行的映射
	}
	reg_col = cur_col;
	/* deal with non-printables */
	if (!isprint2(c)) {
		if (c == '\n' || c == '\r') { /* do the newline thing */
			if (standing) {
				slp->eso = Max(slp->eso, reg_col);
				standing = NA;
			}
			if (reg_col > slp->len) {//以空格扩充列
				register int i;
				for (i = slp->len; i <= reg_col; i++)
					slp->data[i] = ' ';
			}
			slp->len = reg_col;
			cur_col = 0; /* reset cur_col */
			if (cur_ln < scr_lns)
				cur_ln++;
			return;
		} else if (c != KEY_ESC || !showansi) {
			c = '*';/* else substitute a '*' for non-printable */
		}
	}
	if (reg_col >= slp->len) { //	>= 还是 > ?
		register int i;
		for (i = slp->len; i < reg_col; i++)
			slp->data[i] = ' ';
		slp->data[reg_col] = '\0';
		slp->len = reg_col + 1;
	}
	if (slp->data[reg_col] != c) {
		if ((slp->mode & MODIFIED) != MODIFIED)
			slp->smod = (slp->emod = reg_col);
		else {
			if (reg_col > slp->emod)
				slp->emod = reg_col;
			if (reg_col < slp->smod)
				slp->smod = reg_col;
		}
		slp->mode |= MODIFIED;
	}
	slp->data[reg_col] = c; //在当前行reg_col列存储字符c
	reg_col++;
	if (reg_col >= scr_cols) { //超过屏幕最大宽度
		if (standing && slp->mode & STANDOUT) {
			standing = NA;
			slp->eso = Max(slp->eso, reg_col);
		}
		reg_col = 0;
		if (cur_ln < scr_lns)
			cur_ln++;
	}
	cur_col = reg_col; /* store cur_col back */
}

//	利用outc输出字符串str
void outs(register char *str) {
	while (*str != '\0') {
		outc(*str++);
	}
}

//	cc表示是否Ansi方式输出?
//	n表示输出的字符串长度,str是相应的字符串
//	
void outns(register char * str, register int n, register int cc) {
	if (!cc) {
		for (; n > 0; n--) {
			outc(*str++);
		}
	} else {
		/*
		 * need to do find out how many color control char used. and
		 * then add to 'n'.
		 * 
		 * n = n + count_of_color_controler
		 */
		int lock = 0, i = 0, j, k;
		char *foo;
		foo = (char *) malloc(strlen(str) + 100);
		strcpy(foo, str);

		for (j = 0, k = n; k > 0; k--, j++) { //k似乎是多余的,用j就可以?
			if (foo[j] == '' && lock == 0) { //lock为真,表示进入ansi的控制标志
				lock = 1;
				i++;
				continue;
			} else if (isalpha(foo[j]) && lock > 0) {
				i++;
				lock = 0;
				continue;
			} else if (lock > 0) {
				i++;
			}
		}

		n += i; //i为求出的控制标志字符个数
		for (; n > 0; n--)
			outc(*str++);
		outs("[m");

		free(foo); //avoid memory overflow, iamfat 2004.01.12
	}
}

int dec[] = { 1000000000, 100000000, 10000000, 1000000, 100000, 10000,
		1000, 100, 10, 1 };

/*以ANSI格式输出可变参数的字符串序列*/
void prints(char *fmt, ...) {
	va_list ap;
	char *bp;
	register int i, count, hd, indx;
	va_start(ap, fmt);
	while (*fmt != '\0') {
		if (*fmt == '%') {
			int sgn = 1;
			int sgn2 = 1;
			int val = 0;
			int len, negi;
			fmt++;
			switch (*fmt) {
				case '-':
					while (*fmt == '-') {
						sgn *= -1;
						fmt++;
					}
					break;
				case '.':
					sgn2 = 0;
					fmt++;
					break;
			}
			while (isdigit(*fmt)) {
				val *= 10;
				val += *fmt - '0';
				fmt++;
			}
			switch (*fmt) {
				case 's':
					bp = va_arg(ap, char *);
					if (bp == NULL)
						bp = nullstr;
					if (val) {
						register int slen = strlen(bp);
						if (!sgn2) {
							if (val <= slen)
								outns(bp, val, 1);
							else
								outns(bp, slen, 1);
						} else if (val <= slen)
							outns(bp, val, 0);
						else if (sgn > 0) {
							for (slen = val - slen; slen > 0; slen--)
								outc(' ');
							outs(bp);
						} else {
							outs(bp);
							for (slen = val - slen; slen > 0; slen--)
								outc(' ');
						}
					} else
						outs(bp);
					break;
				case 'd':
					i = va_arg(ap, int);

					negi = NA;
					if (i < 0) {
						negi = YEA;
						i *= -1;
					}
					for (indx = 0; indx < 10; indx++)
						if (i >= dec[indx])
							break;
					if (i == 0)
						len = 1;
					else
						len = 10 - indx;
					if (negi)
						len++;
					if (val >= len && sgn > 0) {
						register int slen;
						for (slen = val - len; slen > 0; slen--)
							outc(' ');
					}
					if (negi)
						outc('-');
					hd = 1, indx = 0;
					while (indx < 10) {
						count = 0;
						while (i >= dec[indx]) {
							count++;
							i -= dec[indx];
						}
						indx++;
						if (indx == 10)
							hd = 0;
						if (hd && !count)
							continue;
						hd = 0;
						outc('0' + count);
					}
					if (val >= len && sgn < 0) {
						register int slen;
						for (slen = val - len; slen > 0; slen--)
							outc(' ');
					}
					break;
				case 'c':
					i = va_arg(ap, int);
					outc(i);
					break;
				case '\0':
					goto endprint;
				default:
					outc(*fmt);
					break;
			}
			fmt++;
			continue;
		}
		outc(*fmt);
		fmt++;
	}
	va_end(ap);
	endprint: return;
}
//	输出一个字符
void addch(int ch) {
	outc(ch);
}
// 卷动一行
void scroll() {
	if (dumb_term) {
		prints("\n");
		return;
	}
	scrollcnt++;
	roll++;
	if (roll >= scr_lns)
		roll -= scr_lns;
	move(scr_lns - 1, 0);
	clrtoeol();
}
//向上卷动一行
void rscroll() {
	if (dumb_term) {
		prints("\n\n");
		return;
	}
	scrollcnt--;
	if (roll > 0)
		roll--;
	else
		roll = scr_lns - 1;
	move(0, 0);
	clrtoeol();
}

//将big_picture输出位置1,标准输出区间为(cur_col,cur_col)
void standout() {
	register struct screenline *slp;
	register int ln;
	if (dumb_term || !strtstandoutlen)
		return;
	if (!standing) {
		ln = cur_ln + roll;
		while (ln >= scr_lns)
			ln -= scr_lns;
		slp = &big_picture[ln];
		standing = YEA;
		slp->sso = cur_col;
		slp->eso = cur_col;
		slp->mode |= STANDOUT;
	}
}
//	如果standing为真,将当前行在big_picture中的映射设成真
//		并将eso设成eso,cur_col的最大值
void standend() {
	register struct screenline *slp;
	register int ln;
	if (dumb_term || !strtstandoutlen)
		return;
	if (standing) {
		ln = cur_ln + roll;
		while (ln >= scr_lns)
			ln -= scr_lns;
		slp = &big_picture[ln];
		standing = NA;
		slp->eso = Max(slp->eso, cur_col);
	}
}
//	根据mode来决定 保存或恢复行line的内容
//		最多只能保存一行,否则会被抹去
void saveline(int line, int mode) /* 0,2 : save, 1,3 : restore */
{
	register struct screenline *bp = big_picture;
	static char tmp[2][256];
	int x, y;

	switch (mode) {
		case 0:
		case 2:
			strncpy(tmp[mode/2], bp[line].data, LINELEN);
			tmp[mode/2][bp[line].len]='\0';
			break;
		case 1:
		case 3:
			getyx(&x, &y);
			move(line, 0);
			clrtoeol();
			refresh();
			prints("%s", tmp[(mode-1)/2]);
			move(x, y);
			refresh();
	}
}

