#include "BBSLIB.inc"
#include "math.h"

time_t get_last_logout(struct userec * );

int count_life_value(struct userec *urec) {
    int value, value2;
    value = (time(0) - urec->lastlogin);
    value2 = (time(0) - urec->firstlogin); //....
    // new user should register in 30 mins
    if (strcmp(urec->userid, "new") == 0) {
        return 30 * 60 - value;
    }
#ifdef FDQUAN
    if ((urec->userlevel & PERM_XEMPT)
            || strcmp(urec->userid, "SYSOP") == 0
            || strcmp(urec->userid, "guest") == 0)
    return 999;
    if (!(urec->userlevel & PERM_REGISTER))
        return 14 - value / (24 * 60 * 60);
    if (value2 >= 5 * 365 * 24 * 60 * 60)
        return 666 - value / (24 * 60 * 60);
    if (value2 >= 2 * 365 * 24 * 60 * 60)
        return 365 - value / (24 * 60 * 60);
    return 150 - value / (24 * 60 * 60);
#else
    if (((urec->userlevel & PERM_XEMPT) && (urec->userlevel
            & PERM_LONGLIFE)) || strcmp(urec->userid, "SYSOP") == 0
            || strcmp(urec->userid, "guest") == 0)
        return 999;
    if ((urec->userlevel & PERM_XEMPT) && !(urec->userlevel
            & PERM_LONGLIFE))
        return 666;
    if (!(urec->userlevel & PERM_REGISTER))
        return 14 - value / (24 * 60 * 60);
    if (!(urec->userlevel & PERM_XEMPT) && (urec->userlevel
            & PERM_LONGLIFE))
        return 365 - value / (24 * 60 * 60);
    if (value2 >= 3 * 365 * 24 * 60 * 60)
        return 180 - value / (24 * 60 * 60);
    return 120 - value / (24 * 60 * 60);
#endif
}


int main() {
	FILE *fp;
	char userid[14], filename[80], buf[512];
	struct userec *x;
	struct user_info *u;
	int i, tmp1, tmp2, num, clr;
	init_all();
	strsncpy(userid, getparm("userid"), 13);
	//printf("%s",userid);
	printf("<b>查询网友 · %s </b>\n", BBSNAME);
//	if(userid[0]==0) {
		printf("<br><form action=bbsqry>\n");
		printf("请输入用户名: <input name=userid maxlength=12 size=12>\n");
		printf("<input type=submit value=查询用户>\n");
		printf("</form>\n");
	if(userid[0]==0){
		http_quit();
	}
/* added by roly */
	if(!strcmp(userid,"诚征版主中")) {
		printf("faint，这个都要查？");
		http_quit();
	}
/* added end */
	x=getuser(userid);
	if(!loginok || x==0) {
		printf("用户 [%s] 不存在.", userid);
		http_quit();
	}
	/*
	{
	char query_string[256];
	sprintf(query_string,"%s from %s", x->userid, fromhost);
	do_report("QUERY_LOG", query_string);
	}*/
printpretable();
printf("<table bgcolor=#ffffff>\n");

	printf("<pre class=ansi>\n");
	sprintf(buf, "%s ([33m%s[37m) 共上站 [32m%d[m 次，发表文章 [32m%d[m 篇", 
		x->userid, x->username, x->numlogins, x->numposts);
	hprintf("%s", buf);
	show_special(x->userid);
	printf("\n");
	if(x->userdefine & DEF_COLOREDSEX) {
                clr=(x->gender == 'F') ? 35 : 36;
        } else {
                clr=32;
	}
	if(x->userdefine & DEF_S_HOROSCOPE) hprintf("[[1;%dm%s[m]", clr, horoscope(x->birthmonth, x->birthday));
//	hprintf("上次在 [[32m%s[37m] 从 [[32m%s[37m] 到本站一游。\n", Ctime(x->lastlogin), x->lasthost);
//modified by iamfat 2002.08.01
/*
	{
		int i;
		srand(time(0));
		i=(int) (10.0*rand()/(RAND_MAX+1.0));;
		while(i--)
		{
			printf("<!--上次在 [[32m%s[37m] 从 [[32m%d.%d.%d.%d[37m] 到本站一游。\n-->", cn_Ctime(time(0)), (int) (256.0*rand()/(RAND_MAX+1.0)), (int) (256.0*rand()/(RAND_MAX+1.0)), (int) (256.0*rand()/(RAND_MAX+1.0)), (int) (256.0*rand()/(RAND_MAX+1.0)));
		}
	}
	hprintf("上次在 [[32m%s[37m] 从 [[32m%s[37m] 到本站一游。\n", cn_Ctime(x->lastlogin), x->lasthost);*/
	count_mails(userid, &tmp1, &tmp2);
	hprintf("信箱：[[32m%s[37m]，", tmp2 ? "⊙":"  ");
/*
#ifdef SHOWEXP
	hprintf("经验值：[[32m%d[37m]([33m%s[37m) ", countexp(x), cexp(countexp(x)));
#else
	hprintf("经验值：[[33m%s[37m] ", cexp(countexp(x)));
#endif
*/
	printf("经验值: [");
	iconexp(countexp(x));
	printf("]");
#ifdef SHOWPERF
	hprintf("表现值：[[32m%d[37m]([33m%s[37m) ", countperf(x), cperf(countperf(x)));
#else
	hprintf("表现值：[[33m%s[37m] ", cperf(countperf(x)));
#endif
	hprintf("生命力：[[32m%d[37m]。\n", count_life_value(x));
	num=0;
	for(i=0; i<MAXACTIVE; i++) {
		u=&(shm_utmp->uinfo[i]);
		if(!strcmp(u->userid, x->userid)) {
			if(u->active==0 || u->pid==0 || u->invisible && !HAS_PERM(PERM_SEECLOAK)) continue;
			num++;
			if(num==1) hprintf("目前在站上, 状态如下:\n");
			if(u->invisible) hprintf("[36mC[37m");
			hprintf("[32m%s[m ", ModeType(u->mode));
			if(num%5==0) printf("\n");
		}
	}
	if(num==0) {
		sprintf(filename, "home/%c/%s", toupper(x->userid[0]), x->userid);
//		hprintf("目前不在站上, 上次离站时间 [[1;32m%s[m]\n\n", Ctime(file_time(filename)));
//modified by iamfat 2002.08.01
/* 2002.09.22 modified by stephen to fix the web query last login time error and make it fit the telnet query. */
/*		hprintf("目前不在站上, 上次离站时间 [[1;32m%s[m]\n\n", cn_Ctime(file_time(filename)));
*/
	hprintf("目前不在站上, 上次离站时间 [[1;32m%s[m]\n\n", cn_Ctime( get_last_logout(x) ));
/* 2002.09.22 modified end*/
	}
	printf("\n");
	sprintf(filename, "home/%c/%s/plans", toupper(x->userid[0]), x->userid);
	fp=fopen(filename, "r");
	if(fp) {
		while(1) {
			if(fgets(buf, 256, fp)==0) break;
			hhprintf(buf);
		}
		fclose(fp);
	} else {
		hprintf("[36m没有个人说明档[37m\n");
	}
	printf("</pre>");
//	printf("<br><br>\n");
printf("</table>");

printposttable();

	printf("<p align=center><a href=bbspstmail?userid=%s&title=没主题>[写信问候]</a>  ", x->userid);
	printf("<a href=bbssendmsg?destid=%s>[发送讯息]</a>  ", x->userid);
	printf("<a href=bbsfadd?userid=%s>[加入好友]</a>  ", x->userid);
	printf("<a href=bbsfdel?userid=%s>[删除好友]</a>  ", x->userid);
	printf("</p>\n");
	http_quit();
}

int show_special(char *id2) {
        FILE *fp;
        char  id1[80], name[80];
        fp=fopen("etc/sysops", "r");
        if(fp==0) return;
        while(1) {
                id1[0]=0;
                name[0]=0;
                if(fscanf(fp, "%s %s", id1, name)<=0) break;
                if(!strcmp(id1, id2))
			hprintf(" [1;31m★[0;36m%s[1;31m★[m", name);
        }
        fclose(fp);
}

/*2002.09.25 add by stephen to keep the web query the same as the telnet query use the same arithmetic,
*/
time_t get_last_logout(struct userec *x)
{
	time_t	now;

	if(x->lastlogout < x->lastlogin){
         now = ((time(0)-x->lastlogin)/120)%47+1+x->lastlogin;
       
      	}
         else if(x->lastlogout > x->lastlogin + 3600*24){
	  now = ((time(0)-x->lastlogin)/120)%47+1+x->lastlogin;         	
        } 
         else  {
         now = x->lastlogout;
		}
	return(now);
}
/*2002.09.25 add end*/
