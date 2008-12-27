#include "../../include/bbs.h"

struct userec u;
int perm_special = PERM_OBOARDS
		  +PERM_OCLUB
		  +PERM_ANNOUNCE
		  +PERM_OCBOARD
		  +PERM_USER
		  +PERM_OCHAT
		  +PERM_SYSOPS
		  +PERM_CLOAK
		  +PERM_SEECLOAK
		  +PERM_ARBI
		  +PERM_SERV
		  +PERM_TECH
		  +PERM_SPECIAL0
		  +PERM_SPECIAL9;


int perm_admin = PERM_OBOARDS
		+PERM_OCLUB
		+PERM_ANNOUNCE
		+PERM_OCBOARD
		+PERM_USER
		+PERM_OCHAT
		+PERM_SYSOPS;

				
int perm_inner = PERM_ARBI
		+PERM_SERV
		+PERM_TECH
		+PERM_SPECIAL0
		+PERM_SPECIAL9;




int perm_long = PERM_XEMPT + PERM_LONGLIFE;


int perm_bm = PERM_BOARDS;



int main(int argc,char ** argv) {
	FILE	*fp=fopen("/home/bbs/.PASSWDS","r");
	bool print=false;
	int num=0;
	char genbuf[64];
	int perm;
				   
	if (argc < 2) {
		printf("Usage:\t%s h\n", argv[0]);
		return (-1);
	}
	if (argv[1][0] == 's')
	{
		printf("所有特殊权限列表\n");
		printf("本列表涉及权限如下：凡具有其中权限之一，即被列入。\n");
		printf("讨论区总管  俱乐部总管  精华区总管  活动看版管理员  账号管理员\n");
		printf("聊天室管理员  系统维护管理员  隐身  看隐身  仲裁组  服务组  技术组  内部版0/9\n");
		printf("注：特殊生命力、大信箱、版主权限未被列入检查范围\n");
		
		perm = perm_special;
	}
	else if (argv[1][0] == 'l')
	{
		printf("长生命力列表\n");
		perm = perm_long;
	}
    else if (argv[1][0] == 'b')
	{
		printf("版主权限列表\n");
		perm = perm_bm;
	}							
	else if (argv[1][0] == 'a')
	{
		printf("本列表涉及权限如下：凡具有其中权限之一，即被列入。\n");
		printf("讨论区总管  俱乐部总管  精华区总管  活动看版管理员\n");
		printf("账号管理员  聊天室管理员  系统维护管理员\n");	
		perm = perm_admin;
	}
	else if (argv[1][0] == 'i')
	{
	    printf("本列表涉及权限如下：凡具有其中之一，即被列入。\n");
	    printf("隐身术  看隐身术  仲裁组  服务组  技术组  内部版0/9\n");
	    printf("注：生命力权限、大信箱未被列入检查范围\n");
		
		perm = perm_inner;
	}
	else if (argv[1][0] == 'h')
	{
		printf("check_perm N (0<N<31)    检查权限2^N