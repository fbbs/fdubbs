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

#include "bbs.h"

//显示帮助文件fname的内容
void show_help(char *fname)
{
    ansimore(fname, YEA);
    clear();
}

//显示多功能阅读选单说明
int mainreadhelp(void)
{
    show_help("help/mainreadhelp");
    return FULLUPDATE;
}

//显示信件的帮助
int mailreadhelp(void)
{
    show_help("help/mailreadhelp");
    return FULLUPDATE;
}

