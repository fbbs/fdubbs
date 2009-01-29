#include "libweb.h"

static int check_multi(struct userec *user) {
	int i, total=0;
	for(i=0; i<MAXACTIVE; i++) {
		if(shm_utmp->uinfo[i].active==0) continue;
		if(!strcasecmp(shm_utmp->uinfo[i].userid, user->userid)) total++;
	}
	//add for NR autopost id:US.   eefree 06.9.8 
	if (strcasecmp(user->userid,"US") ) {
		if(!(user->userlevel&PERM_SYSOPS) && total>=2) http_fatal("您已经登录了2个窗口。为了保证他人利益，此次连线将被取消。");
	}//add end
	return total;
}

static void add_msg(int notused)
{
	int i;
	FILE *fp;
	char buf[129], file[256], *id=currentuser.userid;
	sprintf(file, "touch home/%c/%s/wwwmsg.flush", toupper(id[0]), id);
	system(file);
	sprintf(file, "home/%c/%s/msgfile", toupper(id[0]), id);
	i=file_size(file)/129;
	if (get_record(file, buf, 129, i) <= 0)
		return;
	sprintf(file, "home/%c/%s/wwwmsg", toupper(id[0]), id);
	fp=fopen(file, "a");
	fwrite(buf, 129, 1, fp);
	fclose(fp);
}

static void abort_program(int notused)
{
	int stay=0; 
	struct userec *x; 
	int loginstart;
	if(!strcmp(u_info->userid, currentuser.userid)) { 
#ifdef SPARC
		loginstart=*(int*)(u_info->from+30); 
#else
		loginstart=*(int*)(u_info->from+32); 
#endif
		shm_ucache->status[u_info->uid-1]--;
		bzero(u_info, sizeof(struct user_info)); 
	} 
	
	if(getuser(currentuser.userid)) {
		x = &lookupuser;
		time_t now=time(0);
		time_t recent;
		recent=loginstart;
		if(x->lastlogout>recent)recent=x->lastlogout;
		if(x->lastlogin>recent)recent=x->lastlogin;
		stay=now-recent;
		if(stay<0)stay=0;
		x->stay+=stay; 
		x->lastlogout=now; 
		save_user_data(x); 
	}
	exit(0); 
}

static int wwwagent(void)
{
	int i;
	for(i=0; i<1024; i++) close(i);
	for(i=0; i<NSIG; i++) signal(i, SIG_IGN);
	signal(SIGUSR2, add_msg);
	signal(SIGHUP, abort_program);
	signal(SIGABRT, abort_program);
	while(1) {
		sleep(60);
		if(abs(time(0) - u_info->idle_time)>600) {
			abort_program(0);
		}
	}
	exit(0);
}

static int wwwlogin(struct userec *user) {
	FILE *fp;
	char buf[STRLEN];
	int n, tmp;
	struct user_info *u;

	if(!(currentuser.userlevel & PERM_REGISTER)) { 
		char file[HOMELEN]; 
		sethomefile(file, currentuser.userid, "register");
		if(dashf(file)) { 
			currentuser.userlevel |=PERM_DEFAULT; 
			save_user_data(&currentuser);
		}
	}

	fp=fopen("tmp/.UTMP.lock", "a");
	FLOCK(fileno(fp), LOCK_EX);
	for(n = 0; n < MAXACTIVE; n++) {
		if(shm_utmp->uinfo[n].active == 0) {
			u = &(shm_utmp->uinfo[n]);
			u_info = u;
			bzero(u, sizeof(struct user_info));
			u->active = 1;
			u->uid = searchuser(user->userid);
			u->pid = getpid();
			u->mode = WWW;
			if (HAS_PERM(PERM_LOGINCLOAK)
					&& (currentuser.flags[0] & CLOAK_FLAG))
				u->invisible = YEA;
			u->pager = 0;
			if (DEFINE(DEF_FRIENDCALL))
				u->pager |= FRIEND_PAGER;
			if (DEFINE(PAGER_FLAG)) {
				u->pager |= ALL_PAGER;
				u->pager |= FRIEND_PAGER;
			}
			if (DEFINE(DEF_FRIENDMSG))
				u->pager |= FRIENDMSG_PAGER;
			if (DEFINE(DEF_ALLMSG)) {
				u->pager |= ALLMSG_PAGER;
				u->pager |= FRIENDMSG_PAGER;
			}

			SpecialID(u->userid, fromhost);
			strlcpy(u->from, fromhost, 24);//???
#ifdef SPARC
			*(int*)(u->from + 30)=time(NULL);
#else
			*(int*)(u->from + 32)=time(NULL);
#endif
			u->from[22] = DEFINE(DEF_NOTHIDEIP) ? 'S' : 'H';

			u->idle_time = time(NULL);
			strlcpy(u->username, user->username, sizeof(u->username));
			strlcpy(u->userid, user->userid, sizeof(u->userid));

			FLOCK(fileno(fp), LOCK_UN);
			fclose(fp);
			shm_ucache->status[u->uid - 1]++;
			return 0;
		}
	}
	FLOCK(fileno(fp), LOCK_UN);
	fclose(fp);
	http_fatal("抱歉，目前在线用户数已达上限，无法登录。请稍后再来。");
	return 0;
}

int bbslogin_main(void)
{
	char fname[STRLEN];
	char buf[256], id[IDLEN + 1], pw[PASSLEN];
	struct userec user;

	strlcpy(id, getparm("id"), sizeof(id));
	strlcpy(pw, getparm("pw"), sizeof(pw));
	if(loginok && strcasecmp(id, currentuser.userid)) {
		http_fatal("系统检测到目前您的计算机上已经登录有一个帐号 %s，"
				"请先退出.(选择注销登录, 或者关闭所有浏览器窗口)", 
				currentuser.userid);
	}

	if (getuserec(id, &user) == 0)
		http_fatal("经查证，无此 ID。");

	user.numlogins++;
	if (strcasecmp(id, "guest")) {
		int total;
		time_t stay, recent, now, t;
		if (!checkpasswd(user.passwd, pw)) {
			sprintf(buf, "%-12.12s %s @%s\n",
					user.userid, cn_Ctime(time(NULL)), fromhost);
			sethomefile(fname, user.userid, "logins.bad"); 
			f_append(fname, buf);
			f_append("logins.bad", buf);
			http_fatal("密码输入错误...");
		}
		total = check_multi(&user);
		if (!HAS_PERM2(PERM_LOGIN, &user))
			http_fatal("本帐号已停机。请到Notice版查询原因");

		now = time(NULL);
		// Do not count frequent logins.
		if (now - user.lastlogin < 20 * 60
				&& user.numlogins >= 100)
				user.numlogins--;
		if (total > 1) {
			recent = user.lastlogout;
			if (user.lastlogin > recent)
				recent = user.lastlogin;
			stay = now - recent;
			if (stay < 0)
				stay = 0;
		} else {
			stay = 0;
		}
		t = user.lastlogin;
		user.lastlogin = now;
		user.stay += stay;
#ifdef CHECK_FREQUENTLOGIN
		if (!HAS_PERM(PERM_SYSOPS)
				&& abs(t - time(NULL)) < 10) {
			report("Too Frequent", user.userid);
			http_fatal("登录过于频繁，请稍候再来。");
		}
#endif
		strlcpy(user.lasthost, fromhost, sizeof(user.lasthost));
		save_user_data(&user);
		currentuser = user;
	}

	log_usies("ENTER", fromhost, &user);
	if(!loginok && strcasecmp(id, "guest"))
		wwwlogin(&user);
	loginok = 1;
	redirect(FIRST_PAGE);
	return 0;
}
