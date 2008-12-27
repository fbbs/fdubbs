/* 快意灌水站 交易市场代码 1999.12.19 */
#include <string.h>
#include <stdlib.h>

#include "bbs.h"
#define MAXBET 5000
typedef struct exchanges {
	char	title[STRLEN];
	int	value;
	char	information[STRLEN];
} EXCHANGES;
int gotomarket(char *title)
{
        if (!strcmp("guest", currentuser.userid)) return 1;
        modify_user_mode(MARKET);
        clear();
        set_safe_record();
        move(2,0);
        prints("欢迎进入 [[32m%s[37m]....\n\n",title);
	return 0;
}

int lending()
{
        int     id, canlending=0,maxnum = 0, num = 0;
	char	ans[STRLEN];
	time_t 	now=time(0);
	extern int gettheuserid();
	if(gotomarket("交易市场")) return 0;
	maxnum = currentuser.money - currentuser.bet - 1000;
	prints("欢迎使用[0;1;33m"BBSNAME"[37m交易市场[32m友情转帐[37m功能[m");
	prints("\n\n您目前的情况是：\n注册天数([32m%d[37m 天) 上站总时数([32m%d[37m 小时) [44;37m可转帐资金([32m%d[37m 元)[m[37m",
		(now - currentuser.firstlogin)/86400,currentuser.stay/3600,currentuser.money-currentuser.bet-1000);
	if ( currentuser.stay <= 36000 || now - currentuser.firstlogin  <= 2592000 || maxnum <= 0 ) {
		 prints("\n\n目前市场规定： 参与[32m友情转帐[m的 ID 必须具备以下全部条件：\n    1. 本帐号注册天数超过 30 天;\n    2. 总上站时数超过 10 小时;\n    3. 最终拥有存款超过 1000 元.\n      (注：指存款减去贷款后的差值.)");
		prints("\n\n根据市场规定，您目前尚没有[32m友情转帐[m的资格。 :P \n");
		pressanykey();
		return 0;
	}
        if (!gettheuserid(9,"您想转帐到谁的帐户上？请输入他的帐号: ",&id))
                return 0;
	if(!strcmp(currentuser.userid,lookupuser.userid)) {
		prints("\n呵呵，转帐给自己啊？ 嗯，也行。不过本站不提供这个服务。");
		pressanykey();
		return 0;
	}
        if( lookupuser.money+lookupuser.nummedals*1000 > 90000 ) {
                prints("\n对不起，对方目前经济能力尚不需要您的转帐！");
                pressanykey();
                return 0;
        }
	move(10,0);
	canlending = maxnum > 90000 ? 90000 : maxnum;
	prints("您将转帐到 [1;32m%s[m 的帐号，您可以最多可以转帐 [1;33m%d[m 元。",lookupuser.userid, canlending);
        getdata(12, 0, "确认要转帐，请输入转帐数目，否则，请直接回车取消转帐: ",ans, 6, DOECHO, YEA);
        num = atoi(ans);
        if ( num <= 0 || num > canlending ) {
                prints("\n输入有错误哦。 还是算了吧。。。");
                pressanykey();
                return 0;
        }
	set_safe_record();
	if(currentuser.money - currentuser.bet - 1000 != maxnum) {
		prints("\n对不起，您的可转帐资金有所变化，取消此次交易！");
		prints("\n请重新执行本交易。");
		pressanykey();
		return 0;
	}
 	currentuser.money -= num;
	substitut_record(PASSFILE, &currentuser, sizeof(currentuser),usernum);
	lookupuser.money += num;
	substitut_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
	prints("\n谢谢您，您已经给 [1;32m%s[m 转帐 [1;33m%d[m 元。",lookupuser.userid,num);
	prints("\n为表示对你的转帐行为的感谢，本站已经用信件通知了他。");
	sprintf(genbuf,"给 %s 转帐 %d 元",lookupuser.userid,num);
	gamelog(genbuf);
	sprintf(ans,"%s 给您寄来了 %d 元友情转帐",currentuser.userid,num);
	sprintf(genbuf,"\n %s :\n\t您好！\n\t您的朋友 %s 给您寄来了 %d 元友情转帐资金。\n\t请您在 Market 板对他的无私行为发文表示感谢，\n\t这样，您就可以获得这笔友情转帐资金。\n\n\t当然，您也可以退出一次后，再进入本站，\n\t一样可以获得这笔友情转帐资金。\n  ",lookupuser.userid,currentuser.userid,num);
	autoreport(ans,genbuf,NA,lookupuser.userid);
	pressanykey();
	return 1;
}

int popshop(void)
{
	int no,num,maxnum,templog;
	char ans[10];
	EXCHANGES exchanges[10] = {
		{"上站次数",2},
		{"文章数",5},
		{"奖章数",10000},
		{"隐身术",16000},
		{"看穿隐身术",4500},
		{"帐号永久保留",45000},
		{"强制呼叫",54000}, //expired function 06.1.5
		{"延长发呆时间",9000},//expired function 06.1.5
		{"大信箱",45000}
		};
	if(gotomarket("市场典当行")) return 1;
	prints("今日典当报价:");
	for(no = 0; no < 9; no ++) {
		move(5+no, 2);
		prints("(%2d) %s",no+1,exchanges[no].title);
		move(5+no, 20);
		prints("==> %6d 元", exchanges[no].value);
	}
	move(16,0);
	prints("您目前的情况是: (1)上站次数([32m%d[37m)  (2)文章数([32m%d[37m)  (3)奖章数.([32m%d[37m)\n",
	    currentuser.numlogins,currentuser.numposts,currentuser.nummedals);
	getdata(18, 0, "您想典当哪一项？", ans, 10, DOECHO, YEA);
	no = atoi(ans);
	if ( no < 1 || no > 9 ) {
		prints("\n呵呵，不典当了？ 那，好走。。欢迎再来 ;)");
		pressanykey();
		return 0;
	}
	move(18, 30);
	prints("你选择典当“[32m%s[m”。",exchanges[no-1].title);
if(no>3){
	set_safe_record();
	maxnum = exchanges[no-1].value;
	switch(no) {
		case 4:
			if(!HAS_PERM(PERM_CLOAK)) {
				num = 0;
				break;
			}
			break;
		case 5:
                        if(!HAS_PERM(PERM_SEECLOAK)) {
                                num = 0;
                                break;
                        }
			break;
		case 6:
                        if(!HAS_PERM(PERM_XEMPT)) {
                                num = 0;
                                break;
                        }
			break;
		case 7:
                        //if(!HAS_PERM(PERM_FORCEPAGE)) {
                        //        num = 0;
                        //        break;
                        //} 
                        num = 0;
			break;
		case 8:
                        //if(!HAS_PERM(PERM_EXT_IDLE)) {
                        //        num = 0;
                        //        break;
                        //}
                        num = 0;
			break;
		case 9:
                        if(!HAS_PERM(PERM_LARGEMAIL)) {
                                num = 0;
                                break;
                        }
			break;
	}
	prints("\n\n");
	if(!num) {
		prints("对不起, 你还没有这种权限. ");
		pressanykey();
		return 0;
	}
        if(askyn("您确定要典当吗？",NA,NA) == NA ) {
                move(23,0);clrtoeol();
                prints("现在不典当了？ 那你下次再来。 要记得哦。");
                pressanykey();
                return 0;
        }
	set_safe_record();
        switch(no) {
                case 4:
                        num = HAS_PERM(PERM_CLOAK);
			currentuser.userlevel &= ~PERM_CLOAK ;
                        break;
                case 5:
                        num = HAS_PERM(PERM_SEECLOAK);
                        currentuser.userlevel &= ~PERM_SEECLOAK ;
                        break;
                case 6:
                        num = HAS_PERM(PERM_XEMPT);
                        currentuser.userlevel &= ~PERM_XEMPT ;
                        break;
                case 7:
                        //num = HAS_PERM(PERM_FORCEPAGE);
                        //currentuser.userlevel &= ~PERM_FORCEPAGE ;
                        break;
                case 8:
                        //num = HAS_PERM(PERM_EXT_IDLE);
                        //currentuser.userlevel &= ~PERM_EXT_IDLE ;
                        break;
                case 9:
                        num = HAS_PERM(PERM_LARGEMAIL);
                        currentuser.userlevel &= ~PERM_LARGEMAIL ;
                        break;
	}
        if(!num) {
                prints("对不起, 你的数据发生了变化, 你目前没有这种权限. ");
                pressanykey();
                return 0;
        }
} else {
	if( no == 1 )maxnum = currentuser.numlogins;
	else if ( no == 2) maxnum = currentuser.numposts;  
	else	maxnum = currentuser.nummedals;
	templog = maxnum;
	sprintf(genbuf,"您想典当多少呢(最多%d)？",maxnum);
	getdata(19, 0, genbuf,ans, 10, DOECHO, YEA);
	num = atoi(ans);
	if ( num <= 0 || num > maxnum ) {
		prints("输入有错误哦。 还是算了吧。。。");
		pressanykey();
		return 0;
	}
        maxnum = num*exchanges[no-1].value;
	move(19,0);
	prints("您共计典当%s[32m%d[m 份， %s [33m%d[m 元。\n",exchanges[no-1].title,num,"可以获得",maxnum);
        if(askyn("您确定要典当吗？",NA,NA) == NA ) {
                move(21,0);clrtoeol();
		prints("现在不典当了？ 那你下次再来。 要记得哦。");
                pressanykey();
                return 0;
        }
	set_safe_record();
	if (no == 1) {
		if(templog==currentuser.numlogins)
			currentuser.numlogins -= num;
		else templog = -1;
	} else if (no == 2)  {
		if(templog == currentuser.numposts)
			currentuser.numposts -= num;
		else templog = -1;
	} else {
		if(templog == currentuser.nummedals)
			 currentuser.nummedals -= num;
		else templog = -1;
	}
	if( templog == -1) {
		move(21,0); clrtoeol();
		prints("对不起, 在交易过程中您的数据发生了变化.\n为保证交易的正常进行, 此次交易取消.\n您可以重新进行本交易.");
		pressanykey();
		return 0;
	}
}
	currentuser.money += maxnum;
	if( currentuser.money > 400000000 ){
		move(21,0); clrtoeol();
		prints("对不起，交易数据过大，产生溢出，请重新交易！");
		pressanykey();
		return 0;
	}
        substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	move(21,0); clrtoeol();
        prints("顺利完成交易，欢迎您的再次惠顾。;P");
	sprintf(genbuf,"典当%s %d 份，花费 %d 元.",exchanges[no-1].title,num,maxnum);
	gamelog(genbuf);
        pressanykey();
        return 1;
}
int doshopping()
{
        int no,hasperm=1,maxnum;
        char ans[10];
        EXCHANGES exchanges[10] = {
                {"隐身术",40000},
                {"看穿隐身术",10000},
                {"帐号永久保留",100000},
                {"强制呼叫",120000},//expired 06.1.5
                {"延长发呆时间",20000},//expired 06.1.5
                {"大信箱",100000}
                };
        if(gotomarket("市场购物中心")) return 1;
        prints("今日商品报价:");
        for(no = 0; no < 6; no ++) {
                move(5+no, 2);
                prints("(%2d) %s",no+1,exchanges[no].title);
                move(5+no, 20);
                prints("==> %6d 元", exchanges[no].value);
        }
        move(16,0);
        prints("您目前尚有 %d 元 (奖章 %d 个)\n",
            currentuser.money,currentuser.nummedals);
        getdata(18, 0, "您想购买哪一件商品？", ans, 10, DOECHO, YEA);
        no = atoi(ans);
        if ( no < 1 || no > 6 ) {
                prints("\n呵呵，不想买了？ 那，好走。。欢迎再来 ;)");
                pressanykey();
                return 0;
        }
        if ( no == 4 || no == 5 ) {
                prints("\n小店不提供该商品了哦 :)");
                pressanykey();
                return 0;
       }
        set_safe_record();
        maxnum = exchanges[no-1].value;
        switch(no) {
                case 1:
                        hasperm = HAS_PERM(PERM_CLOAK);
                        break;
                case 2:
                        hasperm = HAS_PERM(PERM_SEECLOAK);
                        break;
                case 3:
                        hasperm = HAS_PERM(PERM_XEMPT);
                        break;
                case 4:
                        //hasperm = HAS_PERM(PERM_FORCEPAGE);
                        break;
                case 5:
                        //hasperm = HAS_PERM(PERM_EXT_IDLE);
                        break;
                case 6:
                        hasperm = HAS_PERM(PERM_LARGEMAIL);
                        break;
        }
        prints("\n\n");
        if(hasperm) {
                prints("您已经有这种权限, 不需要再购买. ");
                pressanykey();
                return 0;
        }
        if(currentuser.money < maxnum) {
                prints("\n对不起, 你没有足够的钱购买这种权限.");
                pressanykey();
                return 0;
        }
        if(askyn("您确定要购买吗？",NA,NA) == NA ) {
                move(23,0);clrtoeol();
                prints("现在不买了？ 那你下次再来。 要记得哦。");
                pressanykey();
                return 0;
        }
        set_safe_record();
        switch(no) {
                case 1:
                        hasperm = HAS_PERM(PERM_CLOAK);
                        currentuser.userlevel |= PERM_CLOAK ;
                        break;
                case 2:
                        hasperm = HAS_PERM(PERM_SEECLOAK);
                        currentuser.userlevel |= PERM_SEECLOAK ;
                        break;
                case 3:
                        hasperm = HAS_PERM(PERM_XEMPT);
                        currentuser.userlevel |= PERM_XEMPT ;
                        break;
                case 4://expired 06.1.5
                        //hasperm = HAS_PERM(PERM_FORCEPAGE);
                        //currentuser.userlevel |= PERM_FORCEPAGE ;
                        break;
                case 5://expired 06.1.5
                        //hasperm = HAS_PERM(PERM_EXT_IDLE);
                        //currentuser.userlevel |= PERM_EXT_IDLE ;
                        break;
                case 6:
                        hasperm = HAS_PERM(PERM_LARGEMAIL);
                        currentuser.userlevel |= PERM_LARGEMAIL ;
                        break;
        }
        if(hasperm) {
                prints("在交易进行前您已经有了这种权限, 所以取消此次交易. ");
                pressanykey();
                return 0;
        }
        if(currentuser.money < maxnum) {
                prints("\n对不起, 你没有足够的钱购买这种权限.");
                pressanykey();
                return 0;
        }
	currentuser.money -= maxnum;
        substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);        move(23,0); clrtoeol();
        prints("顺利完成交易，欢迎您的再次惠顾。;P");
        sprintf(genbuf,"购买[%s]，花费 %d 元.",exchanges[no-1].title,maxnum);
        gamelog(genbuf);
        pressanykey();
        return 1;
}	

int
payoff()
{
	if(gotomarket("水站还贷处")) return 0;
        prints("本处规定: 偿还贷款必须一次还清. \n\n");
	if(currentuser.bet == 0 ) {
		prints("你并没有在本市场借钱，所以无需还钱，呵呵");
		pressanykey();
		return 0;
	}
	if(currentuser.money < currentuser.bet) {
		prints("你的钱不够还贷款，请下次再来还罗。");
		pressanykey();
		return 0;
	}
	prints("您在本处共贷款 %d 元.\n\n", currentuser.bet);
	 if(askyn("您现在就想还清贷款吗？",NA,NA) == NA ) {
		prints("现在不还了？ 那你下次再来。 要记得哦。");
		pressanykey();
		return 0;
	}
        currentuser.money -= currentuser.bet;
        sprintf(genbuf,"还清贷款 %d 元.",currentuser.bet);
        gamelog(genbuf);
        currentuser.bet = 0;
        currentuser.dateforbet = 0;
        substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
        prints("您已经还清了在本市场所借的钱。欢迎您的再次惠顾。;P");
        pressanykey();
        return 1;
}
	
int
borrow()
{
	time_t now = time(0);
	int tempbet,maxbet;
	char 	buf[STRLEN];
	if(gotomarket("水站借贷处"))return 0;
	prints("本处规定: 目前每人最多可以贷款 %d 元.\n\n", MAXBET);
	if(!currentuser.bet)
		prints("您目前还没有在本处贷款.\n\n");
	else {
		prints("您已经在本处贷款 %d 元.\n",currentuser.bet);
		getdatestring(currentuser.dateforbet,NA);
		sprintf(genbuf,"您偿还贷款的最后期限是:  %14.14s(%s) \n\n",datestring,datestring+23);
		prints(genbuf);
		if( currentuser.bet>=MAXBET) {
               		prints("对不起, 您的贷款已经达到规定数目, 不能再享受贷款服务.");
                        pressanykey();
                        return 0;
                }

	}
	if(askyn("您现在想向本站贷款吗？",NA,NA) == NA ) return 0;
	maxbet = MAXBET-currentuser.bet;
	if( maxbet > 1000 ) {
		sprintf(genbuf,  "您可以贷款: 至少 1000 元, 最多 %d 元。您想借多少呢？",maxbet);
		getdata(10, 0, genbuf, buf, 10, DOECHO, YEA);
		tempbet = atoi(buf);
	} else {
		sprintf(genbuf,"您可以贷款 %d 元，您确定要贷款吗？",maxbet);
		if( askyn(genbuf,YEA,NA) == NA) {
			prints("\n嗯，不借了？ 那好，下次再来。 ;p");
			pressanykey();
			return 0;
		}
		tempbet = maxbet;
	}
	if ( (maxbet > 1000 && tempbet >= 1000 && tempbet <= maxbet)
		||  maxbet <= 1000 ) {
		currentuser.money += tempbet;
		currentuser.bet += tempbet;
		currentuser.dateforbet = now + 10*24*60*60;
		substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
		getdatestring(currentuser.dateforbet,NA);
		sprintf(genbuf, "\n你向本站总共借款 %d 元，您需要在 %14.14s(%s) 还清贷款。",currentuser.bet,datestring,datestring+23);
		prints(genbuf);
		sprintf(genbuf,"%s 借款 %d 元.",currentuser.userid,tempbet);
		gamelog(genbuf);
		pressanykey();
		return 1;
        }
	prints("\n您输入的数目不正确，取消此次交易。");
	pressanykey();
	return 0;
}

int inmoney(unsigned int money)
{
	set_safe_record();
        if(currentuser.money + money < 400000000)currentuser.money += money ;
	else currentuser.money = 400000000;
        substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
        return currentuser.money;
}

void demoney(unsigned int money)
{
	set_safe_record();
	if(currentuser.money > money ) currentuser.money -= money;
	else currentuser.money = 0;
        substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
}

check_money(unsigned int money)
{
  set_safe_record();
  if(currentuser.money < money)
    {
        move(22, 0);
        clrtobot();
        prints("抱歉！您不可以下注 %d 元, 因为您现在身上只有 %d 元！",
		money,currentuser.money);
	pressanykey();
        return 1;
    }
    return 0;
}
void
show_money(int m, char *welcome,int Clear)
{
	if(Clear) clear();
	if(welcome) {
                ansimore(welcome, NA);
        }
        move(22, 0);
        clrtobot();
        set_safe_record();
        prints("[0;1;37;44m                  你现有现金: [36m%-18d[37m押注金额: [36m%-20d[m  ", currentuser.money, m);
}

int get_money(int m, char *welcome)
{
   unsigned int	money;
   char	buf[5];
   do {
      show_money(m,welcome,YEA);
      getdata(23,16,"☆要押注多少钱(1 - 2000)? ", buf, 5, DOECHO, YEA);
      if(buf[0] == '\0') return 0;
      money = abs(atoi(buf));
      if ( money <= 0) return 0;
      if(check_money(money))return 0;
   } while ((money < 1) || (money > 2000));
   demoney(money);
   show_money(money,NULL,YEA);
   return money;
}
