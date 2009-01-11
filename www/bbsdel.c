#include "BBSLIB.inc"

int main() {
	FILE *fp;
	struct boardheader *brd;
	struct fileheader f;
	struct userec *u;
	char buf[80], dir[80], path[80], board[80], file[80], *id;
	int num=0;
	init_all();
	printf("<b>删除文章 · %s </b><br>\n",BBSNAME);
	printpretable_lite();
	if(!loginok) http_fatal("请先登录");
	id=currentuser.userid;
	strsncpy(board, getparm("board"), 60);
	strsncpy(file, getparm("file"), 32);
	brd=getbcache(board);
	if(strncmp(file, "M.", 2) && strncmp(file, "G.", 2)) http_fatal("错误的参数");
	if(strstr(file, "..")) http_fatal("错误的参数");
	if(brd==0) http_fatal("版面错误");
	if(!has_post_perm(&currentuser, board)) http_fatal("错误的讨论区");
	#ifdef USE_SHMFS
		sprintf(dir, "%s/boards/%s/.DIR", SHM_HOMEDIR, board);
		sprintf(path, "%s/boards/%s/%s", SHM_HOMEDIR, board, file);
	#else
		sprintf(dir, "boards/%s/.DIR", board);
		sprintf(path, "boards/%s/%s", board, file);
	#endif
	fp=fopen(dir, "r");
	if(fp==0) http_fatal("错误的参数");
	while(1) {
		if(fread(&f, sizeof(struct fileheader), 1, fp)<=0) break;
		//added by iamfat 2002.08.10
		//check_anonymous(f.owner);
		//added end.
		if(!strcmp(f.filename, file)) {
        		if(strcasecmp(id, f.owner) && !has_BM_perm(&currentuser, board))
                		http_fatal("您无权删除该文");
			del_record(dir, sizeof(struct fileheader), num);
//			sprintf(buf, "\n※ %s 于 %s 删除