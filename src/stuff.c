#include "bbs.h"
#include <sys/param.h>
#include <sys/sem.h>

extern char fromhost[];

/**
 * Prompt and wait user to press any key.
 * @param[in] msg The prompt message.
 * @param[in] x Line number.
 */
void presskeyfor(const char *msg, int x)
{
	extern int showansi;
	showansi = 1;
	move(x, 0);
	clrtoeol();
	outs(msg);
	egetch();
	move(x, 0);
	clrtoeol();
}

/**
 * Prompt and wait user to press any key.
 */
void pressanykey(void)
{
	presskeyfor("\033[m                                "
			"\033[5;1;33m按任何键继续...[m", t_lines - 1);
}

int pressreturn() {
	extern int showansi;
	char buf[3];
	showansi = 1;
	move(t_lines - 1, 0);
	clrtoeol();
	getdata(
			t_lines - 1,
			0,
			"                              [1;33m请按 ◆[5;36mEnter[m[1;33m◆ 继续\033[m",
			buf, 2, NOECHO, YEA);
	move(t_lines - 1, 0);
	clrtoeol();
	refresh();
	return 0;
}

/**
 * Ask for confirmation.
 * @param str The prompt string.
 * @param defa Default answer.
 * @param gobottom True if prompt at the bottom of the screen.
 * @return True if user answers "y", false if "n", default answer otherwise.
 */
bool askyn(const char *str, bool defa, bool gobottom)
{
	int x, y;
	char buf[100];
	char ans[3];
	snprintf(buf, sizeof(buf), "%s %s", str, (defa) ? "(YES/no)? [Y]"
			: "(yes/NO)? [N]");
	if (gobottom)
		move(t_lines - 1, 0);
	getyx(&x, &y);
	clrtoeol();
	getdata(x, y, buf, ans, 2, DOECHO, YEA);
	switch (ans[0]) {
		case 'Y':
		case 'y':
			return true;
		case 'N':
		case 'n':
			return false;
		default:
			return defa;
	}
}

int msgmorebar(char *filename) {
	extern int showansi;
	char title[256];
	int ch;
	showansi = 1;
	move(t_lines - 1, 0);
	clrtoeol();

	prints("[0m[1;44;32m讯息浏览器   保留 <[1;37mr[32m>    清除 <[1;37mc[1;32m>   寄回信箱<[1;37mm[1;32m>                                [m");
	move(t_lines - 1, 0);

	ch = morekey();
	if (ch == 'C') {
		if (askyn("确定要清除吗？", NA, YEA) == YEA) {
			unlink(filename);
		}
		return ch;
	} else if (ch == 'M') {
		if (askyn("确定要寄回吗？", NA, YEA) == YEA) {
			sprintf(title, "[%s] 所有讯息备份", getdatestring(time(NULL), DATE_ZH));
			mail_file(filename, currentuser.userid, title);
			unlink(filename);
		}
		return ch;
	} else if (ch == 'H') {
		show_help("help/msghelp");
	}
	clrtoeol();
	refresh();
	return ch;
}

void printdash(const char *mesg)
{
	char buf[80], *ptr;
	int len;
	memset(buf, '=', sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';
	if (mesg != NULL) {
		len = strlen(mesg);
		if (len > sizeof(buf) - 4)
		len = sizeof(buf) - 4;
		ptr = buf + (sizeof(buf) - 1 - len) / 2 - 1;
		*ptr++ = ' ';
		memcpy(ptr, mesg, len);
		ptr[len] = ' ';
	}
	prints("%s\n", buf);
}

/* 990807.edwardc fix beep sound in bbsd .. */

void bell(void)
{
	ochar(Ctrl('G'));
}

#define LOOKFIRST  (0)
#define LOOKLAST   (1)
#define QUOTEMODE  (2)
#define MAXCOMSZ (1024)
#define MAXARGS (40)
#define MAXENVS (20)
#define BINDIR "/bin/"

//若dst为目录,且并非.,..,最后一个字符不为/,
//			将其删除,成功返回	1
//					 否则返回	0
int deltree(char *dst) {
	if (strstr(dst, "//") || strstr(dst, "..") || strchr(dst, ' '))
		return 0; /* precaution */
	if (dst[strlen(dst) - 1] == '/')
		return 0;
	if (dashd(dst)) {
		sprintf(genbuf, "/bin/rm -rf %s", dst);
		system(genbuf);
		return 1;
	} else
		return 0;
}
