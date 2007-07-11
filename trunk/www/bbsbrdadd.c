#include "BBSLIB.inc"

char mybrd[22][80];
int mybrdnum=0;
struct boardheader x;

int ismybrd(char *board) {
	int n;
	for(n=0; n<mybrdnum; n++) 
	{
		if(!strcasecmp(mybrd[n], board)) 
			return n;
	}
	return -1;
}

int main() {
	FILE *fp;
	char file[200], board[200];
	int i=0;
	init_all();
	printf("<b>收藏夹 · %s </b><br>\n",BBSNAME);
	printpretable_lite();
	strsncpy(board, getparm("board"), 32);
	if(!loginok) http_fatal("超时或未登录，请重新login");
	sprintf(file, "home/%c/%s/.goodbrd", toupper(currentuser.userid[0]), currentuser.userid);
	fp=fopen(file, "r");
        mybrdnum=0;
        if(fp!=NULL)
        {
                while(fscanf(fp, "%s\n", mybrd[mybrdnum])!= EOF)
                {
                        mybrdnum++;
                        if(mybrdnum>19)
                                break;
                }
                fclose(fp);
        }
	if(mybrdnum>=40) http_fatal("您预定讨论区数目已达上限，不能增加预定");
	if(ismybrd(board)>=0) http_fatal("您已经预定了这个讨论区");
	if(!has_read_perm(&currentuser, board)) http_fatal("此讨论区不存在");
	strcpy(mybrd[mybrdnum], board);
	mybrdnum++;
	fp=fopen(file, "w");
        if(fp!=NULL)
        {
        	for(i=0;i<mybrdnum;i++)
                {
			fprintf(fp,"%s\n",mybrd[i]);
		}
                fclose(fp);
        }
	printf("<script>top.f2.location='bbsleft'</script>\n");
	printf("预定讨论区成功<br><a href='javascript:history.go(-1)'>快速返回</a>");
	http_quit();
}
