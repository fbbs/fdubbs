#include "BBSLIB.inc"
#include "netinet/in.h"
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#define  IPADDRESS	"61.129.42.9"


int chat_fd;
int test=0;
char genbuf[1024];

int main() {
	char t[80];
	int pid;
	init_all();
	strsncpy(t, getparm("t"), 80);
	pid=atoi(getparm("pid"));
	if(pid==0) reg();
	if(!loginok) {
		http_fatal("错误，请先登录");
		exit(0);
	}
	if(pid>0)
		if(kill(pid, 0)!=0) {
			printf("<script>top.location='about:您已经断线或已有1个窗口进入聊天室了.'</script>");
			exit(0);
		}
        if(!strcmp(t, "frm")) show_frm(pid);
	if(!strcmp(t, "chatsnd")) chatsnd(pid);
	if(!strcmp(t, "frame_input")) frame_input(pid);
	if(!strcmp(t, "chatrefresh")) {test=1;chatrefresh(pid);}
	printf("err cmd");
}

int reg() {
	int n, pid;
	if(pid=fork()) {
		sprintf(genbuf, "bbschat?pid=%d&t=frm", pid);
		redirect(genbuf);
		http_quit();
	}
	for(n=0; n<1024; n++) close(n);
	pid=getpid();
	agent(pid);
	exit(0);
}

int show_frm(int pid) {
	printf(
"	<frameset rows=0,0,*,48,16 frameborder=0>\n"
"	<frame name=hide src=''>\n"
"	<frame name=hide2 src=''>\n"
"	<frame name=main src=''>\n"
"	<frame scrolling=auto marginheight=1 framespacing=1 name=input src=bbschat?t=frame_input&pid=%d>\n"
"	<frame scrolling=no marginwidth=4 marginheight=1 framespacing=1 name=foot src=bbsfoot>\n"
"	</frameset>\n"
"	</html>\n", pid);
	http_quit();
}

int frame_input(int pid) {
	printf(
"	<script>\n"
"		function r1() {\n"
"			top.hide2.location='bbschat?t=chatrefresh&pid=%d';\n"
"			setTimeout('r1()', 10000);\n"
"		}\n"
"		setTimeout('r1()', 500);\n"
"	</script>\n"
"	<body onload='document.form1.in1.focus()'>\n"
"	<nobr>\n"
"	<form onsubmit='add_cc()' name=form1 action=bbschat?pid=%d&t=chatsnd method=post target=hide>\n"
"	Input: <input name=in1 maxlength=60 size=56>\n"
"	<input type=submit value=发送>\n"
"	<script>\n"
"		var cc, cc2;\n"
"		cc='';\n"
"		function add_cc0(x1, x) {\n"
"			cc2=x1;\n"
"			cc=x;\n"
"		}\n"
"		function do_quit() {\n"
"			if(confirm('您真的要退出了吗？')) {\n"
"				form1.in1.value='/b';\n"
"				form1.submit();\n"
"			}\n"
"		}\n"
"		function do_help() {\n"
"			open('/chathelp.html', '_blank', \n"
"			'toolbar=yes,location=no,status=no,menubar=no,scrollbar=auto,resizable=yes,width=620,height=400');\n"
"		}\n"
"		function do_alias(x) {\n"
"			form1.in1.value=x;\n"
"			form1.submit();\n"
"		}\n"
"		function do_room() {\n"
"			xx=prompt('请输入包厢名称','');\n"
"			if(xx=='' || xx==null) return;\n"
"			form1.in1.value='/j '+ xx;\n"
"			form1.submit();\n"
"		}\n"
"		function do_user() {\n"
"			form1.in1.value='/l';\n"
"			form1.submit();\n"
"		}\n"
"		function do_r() {\n"
"			form1.in1.value='/r';\n"
"			form1.submit();\n"
"		}\n"
"		function do_w() {\n"
"			form1.in1.value='/w';\n"
"			form1.submit();\n"
"		}\n"
"		function do_msg() {\n"
"			xx=prompt('给谁丢小纸条','');\n"
"			if(xx=='' || xx==null) return;\n"
"			yy=prompt('什么内容','');\n"
"			if(yy=='' || xx==null) return;\n"
"			form1.in1.value='/m '+xx+' '+yy;\n"
"			form1.submit();\n"
"		}\n"
"		function do_n() {\n"
"			xx=prompt('您想改成什么名字?','');\n"
"			if(xx=='' || xx==null) return;\n"
"			form1.in1.value='/n '+xx;\n"
"			form1.submit();\n"
"		}\n"
"		function do_pic() {\n"
"			xx=prompt('请输入图片的URL地址:','http://');\n"
"			if(xx=='http://' || xx=='' || xx==null) return;\n"
"			form1.in1.value='<img src='+xx+'>';\n"
"			form1.submit();\n"
"		}\n"
"	</script>\n"
"	<select onChange='do_alias(this.options[this.selectedIndex].value);this.selectedIndex=0;'>\n"
"        <option value=' ' selected>聊天动作</option>\n"
"        <option value='//hehe'>呵呵的傻笑</option>\n"
"	<option value='//faint'>要晕倒了</option>\n"
"	<option value='//sleep'>要睡着了</option>\n"
"	<option value='//:D'>乐滋滋的</option>\n"
"	<option value='//so'>就这样</option>\n"
"	<option value='//shake'>摇摇头</option>\n"
"	<option value='//luck'>真幸运啊</option>\n"
"	<option value='//tongue'>吐吐舌头</option>\n"
"	<option value='//blush'>脸红了</option>\n"
"	<option value='//applaud'>热烈鼓掌</option>\n"
"	<option value='//cough'>咳嗽一下</option>\n"
"	<option value='//happy'>好高兴啊</option>\n"
"	<option value='//hungry'>肚子饿了</option>\n"
"	<option value='//strut>大摇大摆</option>\n"
"	<option value='//think'>想一想</option>\n"
"	<option value='//?'>疑惑不已</option>\n"
"	<option value='//bearbug'>热情拥抱</option>\n"
"	<option value='//bless'>祝福</option>\n"
"	<option value='//bow'>鞠躬</option>\n"
"        <option value='//caress'>抚摸</option>\n"
"        <option value='//cringe'>企求宽恕</option>\n"
"        <option value='//cry'>放声大哭</option>\n"
"        <option value='//comfort'>安慰一下</option>\n"
"	<option value='//clap'>热烈鼓掌</option>\n"
"        <option value='//dance'>翩翩起舞</option>\n"
"    	<option value='//drivel'>流口水</option>\n"
"    	<option value='//farewell'>再见</option>\n"
"  	<option value='//giggle'>呆笑</option>\n"
"    	<option value='//grin'>咧嘴笑</option>\n"
"      	<option value='//growl'>大声咆哮</option>\n"
/*
hand      heng      hug       haha      heihei    joycup    kick
kiss      koko      laugh     mm        nod       nudge     oh        pad
pat       papaya    pinch     punch     pure      puke      report    shrug
sigh      slap      smooch    snicker   sniff     spank     squeeze   thank
tickle    wave      welcome   wink      xixi      zap

【 Verb + Message：动词 + 要说的话 】   例：//sing 天天天蓝
ask       chant     cheer     chuckle   curse     demand    frown     groan
grumble   hum       moan      notice    order     ponder    pout      pray
request   shout     sing      smile     smirk     swear     tease     whimper
yawn      yell
*/
"        </select>\n"
"	<select name=ccc onChange='add_cc0(this, this.options[this.selectedIndex].value)'>\n"
"	<option value='' selected>白色</option>\n"
"	<option value='%s'><font color=green>红色</font></option>\n"
"	<option value='%s'><font color=red>绿色</font></option>\n"
"        <option value='%s'><font color=blue>蓝色</font></option>\n"
"        <option value='%s'><font color=blue>天蓝</font></option>\n"
"        <option value='%s'><font color=yellow>黄色</font></option>\n"
"        <option value='%s'><font color=red>品红</font></option>\n"
"	<option value='%s'>大字</option>\n"
"	</select>\n"
"	<select onChange='do_func(this.selectedIndex);this.selectedIndex=0;'>\n"
" 	<option selected>聊天室功能</option>\n"
"	<option>进入包厢</option>\n"
"	<option>查看包厢名</option>\n"
"	<option>本包厢有谁</option>\n"
"	<option>看有谁在线</option>\n"
"	<option>丢小纸条</option>\n"
"	<option>改聊天代号</option>\n"
"	<option>贴图片</option>\n"
"	<option>清除屏幕</option>\n"
"	<option>背景反色</option>\n"
"	<option>离开聊天室</option>\n"
"        </select>\n"
"	<br>\n"
"	<a href='javascript:do_quit()'>[离开bbs茶馆] </a>\n"
"	<a href='/chathelp.html' target=_blank>[聊天室帮助] </a>\n"
"	<script>\n"
"	function do_func(n) {\n"
"		if(n==0) return;\n"
"		if(n==1) return do_room();\n"
"		if(n==2) return do_r();\n"
"		if(n==3) return do_w();\n"
"		if(n==4) return do_user();\n"
"		if(n==5) return do_msg();\n"
"		if(n==6) return do_n();\n"
"		if(n==7) return do_pic();\n"
"		if(n==8) return do_c();\n"
"		if(n==9) return do_css2();\n"
"		if(n==10) return do_quit();\n"
"	}\n"
"	var css1;\n\n"
	
"	css1='bbschat.css';\n"
"	function do_c() {\n"
"		top.main.document.close();\n"
"                top.main.document.writeln('<link rel=stylesheet type=text/css href='+css1+'><body><pre><font class=c37>');\n"
"	}\n"
"	function do_css2() {\n"
"		if(css1=='bbschat.css')\n"
"			css1='bbschat2.css';\n"
"		else\n"
"			css1='bbschat.css';\n"
"		top.main.document.writeln('<link rel=stylesheet type=text/css href='+css1+'><body><pre><font class=c37>');\n"
"	}\n"
"	function add_cc() {\n"
"	 	xxx=form1.in1.value;\n"
"		if(xxx=='/h') {\n"
"			do_help();\n"
"			form1.in1.value='';\n"
"			return; \n"
"		}\n"
"		if(xxx=='/c') {\n"
"			do_c();\n"
"			form1.in1.value='';\n"
"			return;\n"
"		}\n"
"		if(xxx=='') return;\n"
" 		if(xxx.indexOf('/')<0) {\n"
" 			form1.in1.value=cc+xxx;\n"
" 		}\n"
" 		if(cc=='%I') {\n"
" 			cc='';\n"
" 			cc2.selectedIndex=0;\n"
" 		}\n"
" 	}\n"
"	</script>\n"
"	</form></body>\n", pid, pid, "%R", "%G", "%B", "%C", "%Y", "%M", "%I");
	http_quit();
}

int chatsnd(int pid) {
	char in1[255], filename[256];
	FILE *fp;
	strsncpy(in1, getparm("in1"), 60);
	sprintf(filename, "tmp/%d.in", pid);
	fp=fopen(filename, "a");
	fprintf(fp, "%s\n\n", in1);
	fclose(fp);
	chatrefresh(pid);
}

char *cco(char *s) {
        static char buf[512];
        char *p=buf, co[20];
        bzero(buf, 512);
        while(s[0]) {
                if(s[0]!='%') {
                        p[0]=s[0];
                        p++;
                        s++;
                        continue;
                }
                bzero(co, 20);
                if(!strncmp(s, "%R", 2)) strcpy(co, "[31m");
                if(!strncmp(s, "%G", 2)) strcpy(co, "[32m");
                if(!strncmp(s, "%B", 2)) strcpy(co, "[34m");
                if(!strncmp(s, "%C", 2)) strcpy(co, "[36m");
                if(!strncmp(s, "%Y", 2)) strcpy(co, "[33m");
                if(!strncmp(s, "%M", 2)) strcpy(co, "[35m");
                if(!strncmp(s, "%N", 2)) strcpy(co, "[0m");
                if(!strncmp(s, "%W", 2)) strcpy(co, "[37m");
                if(!strncmp(s, "%I", 2)) strcpy(co, "[99m");
                if(co[0]) {
                        strncpy(p, co, strlen(co));
                        p+=strlen(co);
                        s+=2;
                        continue;
                }
                p[0]=s[0];
                p++;
                s++;
        }
        return buf;
}

int chatrefresh(int pid) {
	char filename[256], tmp[256];
	int t1;
	FILE *fp;
        kill(pid, SIGINT);
        usleep(150000);
        if(kill(pid, 0)!=0) {
                printf("<script>top.location='javascript:close()';</script>");
                exit(0);
        }
	sprintf(filename, "tmp/%d.out", pid);
	t1=time(0);
	while(abs(t1-time(0))<8 && !file_exist(filename)) {
		sleep(1);
		continue;
	}
	fp=fopen(filename, "r");
	if(fp) 
		while(1) {
			int i;
			char buf2[512];
			if(fgets(buf2, 255, fp)<=0) break;
			sprintf(genbuf, "%s", cco(buf2));
			for(i=0; genbuf[i]; i++) if(genbuf[i]==10 || genbuf[i]==13) genbuf[i]=0;
			if(!strncmp(genbuf, "/init", 5)) {
				printf("<script>\n");
				printf("top.main.document.write(\"");
				printf("<link rel=stylesheet type=text/css href='http://%s/bbschat.css'><body id=body1 bgColor=black><pre>",IPADDRESS);
				printf("\");");
				printf("\n</script>\n");
				continue;
			}
			if(!strncmp(genbuf, "/t", 2)) {
				int i;
				printf("<script>top.document.title='bbs茶馆--话题: ");
				hprintf(genbuf+2);
				printf("'</script>");
				sprintf(buf2, "本包厢的话题是: [[1;33m%s[37m]", genbuf+2);
				strcpy(genbuf, buf2);
			}
			if(!strncmp(genbuf, "/r", 2)) {
				sprintf(buf2, "本包厢的名称是: [[1;33m%s[37m]", genbuf+2);
				strcpy(genbuf, buf2);
			}
			if(!strncmp(genbuf, "/", 1)) {
				genbuf[0]='>';
				genbuf[1]='>';
			}
			for(i=0; i<strlen(genbuf); i++) {
				if(genbuf[i]==10 || genbuf[i]==13) genbuf[i]=0;
				if(genbuf[i]==34) genbuf[i]=39;
			}
			printf("<script>\n");	
			printf("top.main.document.writeln(\"");
			hhprintf(genbuf);
			printf(" <font class=c37>");
			printf("\");");
			printf("top.main.scrollBy(0, 99999);\n");
			if(test==0) printf("top.input.form1.in1.value='';\n");
			printf("</script>\n");
		}
	unlink(filename);
	printf("<br>");
	http_quit();
}

void foo() {
	FILE *fp;
	char filename[80], buf[256];
	sprintf(filename, "tmp/%d.in", getpid());
	fp=fopen(filename, "r");
	if(fp) {
		while(1) {
			if(fgets(buf, 250, fp)<=0) break;
			write(chat_fd, buf, strlen(buf));
		}
		fclose(fp);
	}
	unlink(filename);
	alarm(60);
}

void abort_chat() {
	int pid=getpid();
        char filename[200];
	sprintf(filename, "tmp/%d.out", pid);
        unlink(filename);
        sprintf(filename, "tmp/%d.in", pid);
        unlink(filename);
        exit(0);
}

int agent(int pid) {
        int i, num;
 	FILE *fp;
	char filename[80];
	struct sockaddr_in blah;
	sprintf(filename, "tmp/%d.out", pid);
        bzero((char *)&blah, sizeof(blah));
        blah.sin_family=AF_INET;
        blah.sin_addr.s_addr=inet_addr("127.0.0.1");
        blah.sin_port=htons(6702);
        chat_fd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(connect(chat_fd, (struct sockaddr *)&blah, 16)<0) return;
        signal(SIGINT, foo);
	signal(SIGALRM, abort_chat);
	alarm(60);
 	sprintf(genbuf, "/! %d %d %s %s %d\n", 
		u_info->uid, currentuser.userlevel, currentuser.userid, currentuser.userid, 0);
 	write(chat_fd, genbuf, strlen(genbuf));
	read(chat_fd, genbuf, 2);
	if(!strcasecmp(genbuf, "OK")) exit(0);
	fp=fopen(filename, "w");
	fprintf(fp, "/init\n");
	fclose(fp);
 	while(1) {
 			num=read(chat_fd, genbuf, 2048);
                        if (num<=0) break;
                        for(i=0; i<num; i++)
                                if(genbuf[i]==0) genbuf[i]=10;
			genbuf[num]=0;
 			fp=fopen(filename, "a");
			fprintf(fp, "%s", genbuf);
			fclose(fp);
        }
	abort_chat();
}

