#include "BBSLIB.inc"

char origin[512];	//added by iamfat 2002.08.17

int main() {
   	FILE *fp;
   	int type=0, i, num;
	char userid[80], buf[512], path[512], file[512], board[512], title[80]="";
   	struct boardheader *brd;
   	struct fileheader  *x;
	
	//added by iamfat 2002.08.17
    sprintf(origin, ":·%s %s·", BBSNAME, BBSHOST);
	//added end

	init_all();
	if(!loginok) 
	{
		printf("<b>修改文章 · %s </b><br>\n", BBSNAME);
		printpretable_lite();
		http_fatal("匆匆过客不能修改文章，请先登录");
	}
	strsncpy(board, getparm("board"), 20);
	type=atoi(getparm("type"));
	brd=getbcache(board);
	if(brd==0) 
	{
		printf("<b>修改文章 · %s </b><br>\n", BBSNAME);
		printpretable_lite();
		http_fatal("错误的讨论区");
	}
	strcpy(board, brd->filename);
	strsncpy(file, getparm("file"), 32);
	if(!has_post_perm(&currentuser, board))
	{
		printf("<b>修改文章 · %s </b><br>\n", BBSNAME);
		printpretable_lite();
		http_fatal("错误的讨论区或者您无权在此讨论区发表文章");
	}
   	x=get_file_ent(board, file);
	if(strstr(file, "..") || strstr(file, "/")) 
	{
		printf("<b>修改文章 · %s </b><br>\n", BBSNAME);
		printpretable_lite();
		http_fatal("错误的参数");
	}
	if(x==0) 
	{
		printf("<b>修改文章 · %s </b><br>\n", BBSNAME);
		printpretable_lite();
		http_fatal("错误的参数");
	}
	if(strcmp(x->owner, currentuser.userid)&& !has_BM_perm(&currentuser, board)) 
	{
		printf("<b>修改文章 · %s </b><br>\n", BBSNAME);
		printpretable_lite();
		http_fatal("您无权修改此文章");
	}
	printf("<b>修改文章 · %s [使用者: %s]</b><br>\n", BBSNAME, currentuser.userid);
	printf("<center>\n");
	printpretable_lite();
	if(type!=0)
	{
		//added by iamfat 2002.08.17
		sprintf(buf, "edited post '%s' on %s", x->title, board);
		trace(buf);
		//added end;
		return update_form(board, file);
	}
   	printf("<table border=1>\n");
	printf("<tr><td>");
	printf("<tr><td><form method=post action=bbsedit>\n");
   	printf("使用标题: %s 讨论区: %s<br>\n", nohtml(x->title), board);
   	printf("本文作者：%s<br>\n", currentuser.userid);
   	printf("<textarea name=text rows=20 cols=80 wrap=physicle>");
	sprintf(path, "boards/%s/%s", board, file);
	fp=fopen(path, "r");
	if(fp==0) http_fatal("文件丢失");
	//added by iamfat 2002.08.17 跳过Header
	while(fgets(buf, 500, fp) && buf[0]!='\n');
	while(fgets(buf, 500, fp))
	{
		if(strstr(buf,origin) && buf[0]!=':')break;
		if(!strcasestr(buf, "</textarea>")) printf("%s", buf);
	}
	fclose(fp);
   	printf("</textarea>\n");
   	printf("<tr><td class=post align=center>\n");
	printf("<input type=hidden name=type value=1>\n");
	printf("<input type=hidden name=board value=%s>\n", board);
	printf("<input type=hidden name=file value=%s>\n", file);
	printf("<input type=submit value=存盘> \n");
   	printf("<input type=reset value=重置></form>\n");
	printf("</table>");
	http_quit();
}

/*
int update_form(char *board, char *file) {
	FILE *fp;
	char *buf=getparm("text"), path[80];
	sprintf(path, "boards/%s/%s", board, file);
	fp=fopen(path, "w");
	if(fp==0) http_fatal("无法存盘");
	fprintf(fp, "%s", buf);
//	fprintf(fp, "\n※ 修改:．%s 於 %s 修改本文．[FROM: %s] ", currentuser.userid, Ctime(time(0))+4, fromhost);
//modified by iamfat 2002.08.01
	fprintf(fp, "[m[1;36m※ 修改:·%s 於 %16.16s 修改本文·HTTP [FROM: %-.20s][m\n", currentuser.userid, cn_Ctime(time(0))+6, fromhost);
	fclose(fp);
	printf("修改文章成功.<br><a href=bbsdoc?board=%s>返回本讨论区</a>", board);
}
*/
//rewrite by iamfat 2002.08.17
int update_form(char *board, char *file) {
	FILE *fpr, *fpw;
	char *buf=getparm("text"), path[80], pathnew[80];
	char buf2[512];
	int flag=0;
	sprintf(path, "boards/%s/%s", board, file);
	sprintf(pathnew, "boards/%s/%s.%d", board, file, u_info->pid);
	fpr=fopen(path, "r");
	fpw=fopen(pathnew, "w");
	if(fpw==0||fpr==0) http_fatal("无法存盘");
	while(fgets(buf2, 500, fpr) && buf2[0]!='\n')fprintf(fpw,"%s",buf2);
	fprintf(fpw, "\n%s\n", buf);
	while(fgets(buf2,500,fpr)!=0)
	{
		if(strstr(buf2,origin) && buf2[0]!=':')flag=1;
		if(flag)
		{
			if(strncmp(buf2,"[m[1;36m※ 修改:·",17))fprintf(fpw,"%s",buf2);
		}
	}
	fprintf(fpw, "[m[1;36m※ 修改:·%s 於 %16.16s 修改本文·HTTP [FROM: %-.20s][m\n", currentuser.userid, cn_Ctime(time(0))+6, fromhost);
	fclose(fpw);
	fclose(fpr);
	rename(pathnew, path);
	printf("修改文章成功.<br><a href=bbsdoc?board=%s>返回本讨论区</a>", board);
}
