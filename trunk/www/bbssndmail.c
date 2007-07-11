#include "BBSLIB.inc"

int main() {
   	FILE *fp;
	char userid[80], filename[80], dir[80], title[80], title2[80], buf[80], *content;
	int t, i, sig, backup;
	struct fileheader x;
   	struct userec *u;
	init_all();
	printf("<b>发送信件 · %s </b><br>\n",BBSNAME);
	printpretable_lite();
	if(!loginok) http_fatal("匆匆过客不能写信，请先登录");
	/* Added by Amigo 2002.06.19. For mail right check. */
	if (!HAS_PERM(PERM_MAIL)) 
		http_fatal("您尚未完成注册，或者发送信件的权限被封禁");
	/* Add end. */
	if(!mailnum_under_limit(currentuser.userid) || !mailsize_under_limit(currentuser.userid))
		http_fatal("您的信件容量超标，无法发信");
   	strsncpy(userid, getparm("userid"), 40);
	sprintf(filename, "home/%c/%s/rejects", toupper(userid[0]), userid);
	if(file_has_word(filename, currentuser.userid)) 
   		http_fatal("对方不想收到您的信件");
	strsncpy(title, noansi(getparm("title")), 50);
	backup=strlen(getparm("backup"));
	if(!strstr(userid, "@")) {
	 	u=getuser(userid);
		if(u==0) http_fatal("错误的收信人帐号");
	         //add by Danielfree 06.2.5
		 else if (!(u -> userlevel & PERM_READMAIL))
		      http_fatal("对方无法收信");
	   	//add  end 	
		strcpy(userid, u->userid);
		
		//added by roly to test mail size
		//modified by Danielfree to test mail num
		if (!mailsize_under_limit(userid)||!mailnum_under_limit(userid)) http_fatal("收件人信件容量超标，无法收信");
		//add end
	} 
	/* added by roly to deny internet mail */
	else {
		http_fatal("本站不支持对外发信");
	}
	
	/* add end */
  	for(i=0; i<strlen(title); i++)
		if(title[i]<27 && title[i]>=-1) title[i]=' ';
   	sig=atoi(getparm("signature"));
   	content=getparm("text");
   	if(title[0]==0)
      		strcpy(title, "没主题");
	sprintf(filename, "tmp/%d.tmp", getpid());
	f_append(filename, content);
	sprintf(title2, "{%s} %s", userid, title);
	title2[70]=0;
	post_mail(userid, title, filename, currentuser.userid, currentuser.username, fromhost, sig-1);
	if(backup)
		post_mail(currentuser.userid, title2, filename, currentuser.userid, currentuser.username, fromhost, sig-1);
	unlink(filename);
	printf("信件已寄给%s.<br>\n", userid);
	if(backup) printf("信件已经备份.<br>\n");
	//added by iamfat 2002.10.20
	sprintf(title2, "mailed %s: %s", userid, title);
	trace(title2);
	//added end
	printf("<a href='javascript:history.go(-2)'>返回</a>");
	http_quit();
}
