#include "libweb.h"

int bbsqry_main(void)
{
	char userid[IDLEN + 1];
	strlcpy(userid, getparm("u"), sizeof(userid));
	if (!loginok)
		return BBS_ELGNREQ;
	struct userec user;
	int uid;
	xml_header("bbs");
	uid = getuserec(userid, &user);
	if (uid != 0) {
		int level, repeat;
		level = iconexp(countexp(&user), &repeat);		
		printf("<bbsqry id='%s' login='%d' lastlogin='%s' "
				"perf='%s' post='%d' hp='%d' level='%d' repeat='%d' ",
				user.userid, user.numlogins,
				getdatestring(user.lastlogin, DATE_XML),
				cperf(countperf(&user)), user.numposts,
				compute_user_value(&user), level, repeat);
		print_session();
		if (HAS_DEFINE(user.userdefine, DEF_S_HOROSCOPE)) {
			printf(" horo='%s'", 
					horoscope(user.birthmonth, user.birthday));
			if (HAS_DEFINE(user.userdefine, DEF_COLOREDSEX))
				printf(" gender='%c'", user.gender);
		}
		printf("><ip>");
		xml_fputs(mask_host(user.lasthost), stdout);
		printf("</ip><nick>");
		xml_fputs(user.username, stdout);
		printf("</nick><ident>");
		char ident[160];
		show_position(&user, ident, sizeof(ident));
		xml_fputs(ident, stdout);
		printf("</ident><smd>");
		char file[HOMELEN];
		sethomefile(file, user.userid, "plans");
		xml_printfile(file, stdout);
		printf("</smd>");

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
	printf("<b>≤È—ØÕ¯”— °§ %s </b>\n", BBSNAME);
//	if(userid[0]==0) {
		printf("<br><form action=bbsqry>\n");
		printf("«Î ‰»Î”√ªß√˚: <input name=userid maxlength=12 size=12>\n");
		printf("<input type=submit value=≤È—Ø”√ªß>\n");
		printf("</form>\n");
	if(userid[0]==0){
		http_quit();
	}
/* added by roly */
	if(!strcmp(userid,"≥œ’˜∞Ê÷˜÷–")) {
		printf("faint£¨’‚∏ˆ∂º“™≤È£ø");
		http_quit();
	}
/* added end */
	x=getuser(userid);
	if(!loginok || x==0) {
		printf("”√ªß [%s] ≤ª¥Ê‘⁄.", userid);
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
	sprintf(buf, "%s ([33m%s[37m) π≤…œ’æ [32m%d[m ¥Œ£¨∑¢±ÌŒƒ’¬ [32m%d[m ∆™", 
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
//	hprintf("…œ¥Œ‘⁄ [[32m%s[37m] ¥” [[32m%s[37m] µΩ±æ’æ“ª”Œ°£\n", Ctime(x->lastlogin), x->lasthost);
//modified by iamfat 2002.08.01
/*
	{
		int i;
		srand(time(0));
		i=(int) (10.0*rand()/(RAND_MAX+1.0));;
		while(i--)
		{
			printf("<!--…œ¥Œ‘⁄ [[32m%s[37m] ¥” [[32m%d.%d.%d.%d[37m] µΩ±æ’æ“ª”Œ°£\n-->", cn_Ctime(time(0)), (int) (256.0*rand()/(RAND_MAX+1.0)), (int) (256.0*rand()/(RAND_MAX+1.0)), (int) (256.0*rand()/(RAND_MAX+1.0)), (int) (256.0*rand()/(RAND_MAX+1.0)));
		}
		if (!num) {
			time_t logout = user.lastlogout;
			if (logout < user.lastlogin) {
				logout = ((time(NULL) - user.lastlogin) / 120) % 47 + 1
						+ user.lastlogin;
			}
			printf("<logout>%s</logout>",
					getdatestring(logout, DATE_XML));
		}
		// TODO: mail
	} else {
		printf("<bbsqry ");
		print_session();
		printf(" id='%s'>", userid);
	}
	printf("</bbsqry>");
	return 0;
}

