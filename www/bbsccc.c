#include "BBSLIB.inc"

int main() {
	struct fileheader *x;
	char board[80], file[80], target[80];
	init_all();
	strsncpy(board, getparm("board"), 30);
	strsncpy(file, getparm("file"), 32);
	strsncpy(target, getparm("target"), 30);
	if(!loginok) 
	{
		printf("<b>转载文章 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("匆匆过客不能进行本项操作");
	}
	if(!has_read_perm(&currentuser, board)) 
	{
		printf("<b>转载文章 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("错误的讨论区");
	}
	x=get_file_ent(board, file);
	if(x==0) 
	{
		printf("<b>转载文章 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("错误的文件名");
	}
	printf("<b>转载文章 · %s [使用者: %s]</b><br>\n", BBSNAME, currentuser.userid);
	printpretable_lite();
	if(target[0]) {
		if(!has_post_perm(&currentuser, target)) http_fatal("错误的讨论区名称或您没有在该版发文的权限");
		return do_ccc(x, board, target);
	}
	printf("<table><tr><td>\n");
	printf("<font color=red>转贴发文注意事项:<br>\n");
	printf("本站规定同样内容的文章严禁在 4 个或 4 个以上讨论区内重复发表。");
	printf("违者将被封禁在本站发文的权利<br><br></font>\n");
	printf("文章标题: %s<br>\n", nohtml(x->title));
	printf("文章作者: %s<br>\n", x->owner);
	printf("原讨论区: %s<br>\n", board);
	printf("<form action=bbsccc method=post>\n");
	printf("<input type=hidden name=board value=%s>", board);
	printf("<input type=hidden name=file value=%s>", file);
	printf("转载到 <input name=target size=30 maxlength=30> 讨论区. ");
	printf("<input type=submit value=确定></form>");
}

int do_ccc(struct fileheader *x, char *board, char *board2) {
	FILE *fp, *fp2;
	struct boardheader *brc = NULL;
	brc = getbcache(board2);
	if (brc -> flag & BOARD_DIR_FLAG) {  //不可转载到目录 Danielfree 06.3.5
	        http_fatal("你选择了一个目录");
        }
	if ((brc->flag & BOARD_CLUB_FLAG)&& (brc->flag & BOARD_READ_FLAG )&& !has_BM_perm(&currentuser, brc->filename)&& !isclubmember(currentuser.userid, brc->filename)) {
			http_fatal ("错误的讨论区名称或您没有在该版发文的权限");
	}
	char title[512], buf[512], path[200], path2[200], i;
	sprintf(path, "boards/%s/%s", board, x->filename);
	fp=fopen(path, "r");
	if(fp==0) http_fatal("文件内容已丢失, 无法转载");
	sprintf(path2, "tmp/%d.tmp", getpid());
	fp2=fopen(path2, "w");
	for(i=0; i<3; i++)
		if(fgets(buf, 256, fp)==0) break;
	fprintf(fp2, "[37;1m【 以下文字转载自 [32m%s [37m讨论区 】\n", board);
	fprintf(fp2, "[37;1m【 原文由 [32m%s [37m所发表 】[m\n\n", x->owner);
	while(1) {
		if(fgets(buf, 256, fp)==0) break;
		fprintf(fp2, "%s", buf);
	}
	fclose(fp);
	fclose(fp2);
	if((!strncmp(x->title, "[转载]", 6))||
		(!strncmp(x->title, "Re: [转载]", 10))){
		//modified by money 04.01.17 for judge Re & cross
		sprintf(title, x->title);
	} else {
		sprintf(title, "[转载]%.55s", x->title);
	}
	post_article(board2, title, path2, currentuser.userid, currentuser.username, fromhost, -1, -1, -1);
	unlink(path2);
	printf("'%s' 已转贴到 %s 版.<br>\n", nohtml(title), board2);
	printf("[<a href='javascript:history.go(-2)'>返回</a>]");
}
