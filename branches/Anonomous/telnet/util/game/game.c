#include "bbs.h"
/* 黑杰克游戏 */
int 
BlackJack()
{
	int             num[52] = {11, 11, 11, 11, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6,
		7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10};
	int             cardlist[52] = {0};
	int             i, j, m, tmp = 0, tmp2, ch;
	int             win = 2, win_jack = 5;	/* win 为赢时的倍率, win_jack
						 * 1 点倍率 */
	int             six = 10, seven = 20, aj = 10, super_jack = 20;	/* 777, A+J, spade A+J 的倍率 */
	int             host_count = 2, guest_count = 1, card_count = 3,
	                A_count = 0, AA_count = 0;
	int             host_point = 0, guest_point = 0, mov_y = 4;
	int             host_card[12] = {0}, guest_card[12] = {0};
	long int        money;

	int             CHEAT = 0;	/* 做弊参数, 1 就作弊, 0 就不作 */

	modify_user_mode(M_BLACKJACK);
	money = get_money(0,"game/blackjack.welcome");
	if(!money) return 0;
	move(1, 0);
	prints("【黑杰克】游戏  [按 y 续牌, n 不续牌, d double, q 认输退出]");
	move(0, 0);
	clrtoeol();
        srandom(time(0));
	for (i = 1; i <= 52; i++) {
		m = 0;
		do {
			j = random() % 52;
			if (cardlist[j] == 0) {
				cardlist[j] = i;
				m = 1;
			}
		} while (m == 0);
	};
	for (i = 0; i < 52; i++)
		cardlist[i]--;	/* 洗牌 */

	if (money >= 20000)
		CHEAT = 1;
	if (CHEAT == 1) {
		if (cardlist[1] <= 3) {
			tmp2 = cardlist[50];
			cardlist[50] = cardlist[1];
			cardlist[1] = tmp2;
		}
	}			/* 作弊码 */
	host_card[0] = cardlist[0];
	if (host_card[0] < 4)
		AA_count++;
	guest_card[0] = cardlist[1];

	if (guest_card[0] < 4)
		A_count++;
	host_card[1] = cardlist[2];
	if (host_card[1] < 4)
		AA_count++;	/* 发前三张牌 */

	move(5, 0);
	prints("╭───╮");
	move(6, 0);
	prints("│      │");
	move(7, 0);
	prints("│      │");
	move(8, 0);
	prints("│      │");
	move(9, 0);
	prints("│      │");
	move(10, 0);
	prints("│      │");
	move(11, 0);
	prints("╰───╯");
	print_card(host_card[1], 5, 4);
	print_card(guest_card[0], 15, 0);	/* 印出前三张牌 */

	host_point = num[host_card[1]];
	guest_point = num[guest_card[0]];

	do {
		m = 1;
		guest_card[guest_count] = cardlist[card_count];
		if (guest_card[guest_count] < 4)
			A_count++;
		print_card(guest_card[guest_count], 15, mov_y);
		guest_point += num[guest_card[guest_count]];

		if ((guest_card[0] >= 24 && guest_card[0] <= 27) && (guest_card[1] >= 24 && guest_card[1] <= 27) && (guest_card[2] >= 24 && guest_card[2] <= 27)) {
			move(18, 3);
			prints("[1;41;33m     ７７７     [m");
			move(3, 0);
			sprintf(genbuf,"[1;41;33m７７７ !!! 得奖金 %d 银两[m", money * seven);
			prints(genbuf);
			inmoney(money * seven);
			gamelog(genbuf);
			pressanykey();
			return 0;
		}
		if ((guest_card[0] == 40 && guest_card[1] == 0) || (guest_card[0] == 0 && guest_card[1] == 40)) {
			move(18, 3);
			prints("[1;41;33m 超级正统 BLACK JACK  [m");
			move(3, 0);
			sprintf(genbuf,"[1;41;33m超级正统 BLACK JACK !!! 得奖金 %d 银两[m", money * super_jack);
			prints(genbuf);
			inmoney(money * super_jack);
			gamelog(genbuf);
			pressanykey();
			return 0;
		}
		if ((guest_card[0] <= 3 && guest_card[0] >= 0) && (guest_card[1] <= 43 && guest_card[1] >= 40))
			tmp = 1;

		if ((tmp == 1) || ((guest_card[1] <= 3 && guest_card[1] >= 0) && (guest_card[0] <= 43 && guest_card[0] >= 40))) {
			move(18, 3);
			prints("[1;41;33m SUPER BLACK JACK  [m");
			move(3, 0);
			sprintf(genbuf,"[1;41;33mSUPER BLACK JACK !!! 得奖金 %d 银两[m", money * aj);
			prints(genbuf);
			inmoney(money * aj);
			gamelog(genbuf);
			pressanykey();
			return 0;
		}
		if (guest_point == 21 && guest_count == 1) {
			move(18, 3);
			prints("[1;41;33m  BLACK JACK  [m");
			move(3, 0);
			sprintf(genbuf,"[1;41;33mBLACK JACK !!![44m 得奖金 %d 银两[m", money * win_jack);
			prints(genbuf);
			inmoney(money * win_jack);
			gamelog(genbuf);
			pressanykey();
			return 0;
		}		/* 前两张就 21 点 */
		if (guest_point > 21) {
			if (A_count > 0) {
				guest_point -= 10;
				A_count--;
			};
		}
		move(19, 0);
		//clrtoeol();
		prints("[1;32m点数: [33m%d[m", host_point);
		move(20, 0);
		//clrtoeol();
		prints("[1;32m点数: [33m%d[m", guest_point);
		if (guest_point > 21) {
			move(20, 0);
			//clrtoeol();
			prints("  爆掉啦~~~  ");
			pressanykey();
			return 0;
		}
		if (guest_count == 5) {
			move(18, 3);
			prints("[1;41;33m            过六关            [m");
			move(3, 0);
			sprintf(genbuf,"[1;41;33m过六关 !!! 得奖金 %d 银两[m", money * six);
			prints(genbuf);
			inmoney(money * six);
			gamelog(genbuf);
			pressanykey();
			return 0;
		}
		guest_count++;
		card_count++;
		mov_y += 4;

		do {
			if (ch == 'd')
				m = 0;
			if (m != 0)
				ch = egetch();
		} while (ch != 'y' && ch != 'Y' && ch != 'n' && ch != 'N' 
                      && ch != 'd' && ch != 'D' && ch != 'q' && ch != 'Q'
                      && m != 0 );	/* 抓 key */

		if (ch == 'd' && m != 0 && guest_count == 2) {
			if (currentuser.money >= money) {
				demoney(money);
				money *= 2;
			} else
				ch = 'n';
		}		/* double */
		if (ch == 'd' && guest_count > 2)
			ch = 'n';
		if (ch == 'q' || ch == 'Q') return ;
		if (guest_point == 21)
			ch = 'n';
	} while (ch != 'n' && m != 0);

	mov_y = 8;

	print_card(host_card[0], 5, 0);
	print_card(host_card[1], 5, 4);
	host_point += num[host_card[0]];

	do {

		if (host_point < guest_point) {
			host_card[host_count] = cardlist[card_count];
			print_card(host_card[host_count], 5, mov_y);
			if (host_card[host_count] < 4)
				AA_count++;
			host_point += num[host_card[host_count]];
		}
		if (host_point > 21) {
			if (AA_count > 0) {
				host_point -= 10;
				AA_count--;
			};
		}
		move(19, 0);
		//clrtoeol();
		prints("[1;32m点数: [33m%d[m", host_point);
		move(20, 0);
		//clrtoeol();
		prints("[1;32m点数: [33m%d[m", guest_point);
		if (host_point > 21) {
			move(20, 0);
			//clrtoeol();
			prints("[1;32m点数: [33m%d [1;41;33m WINNER [m", guest_point);

			move(3, 0);
			sprintf(genbuf,"赢了~~~~ 得奖金 %d 银两", money * win);
			prints(genbuf);
			gamelog(genbuf);
			inmoney(money * win);
			pressanykey();
			return 0;
		}
		host_count++;
		card_count++;
		mov_y += 4;
	} while (host_point < guest_point);

	sprintf(genbuf,"输了~~~~ 没收 %d 银两!", money);
	prints(genbuf);
	gamelog(genbuf);
        pressanykey();
	return 0;
}


int 
print_card(int card, int x, int y)
{
	char           *flower[4] = {"Ｓ", "Ｈ", "Ｄ", "Ｃ"};
	char           *poker[52] = {"Ａ", "Ａ", "Ａ", "Ａ", "２", "２", "２", "２", "３", "３", "３", "３",
		"４", "４", "４", "４", "５", "５", "５", "５", "６", "６", "６", "６",
		"７", "７", "７", "７", "８", "８", "８", "８", "９", "９", "９", "９",
		"10", "10", "10", "10", "Ｊ", "Ｊ", "Ｊ", "Ｊ", "Ｑ", "Ｑ", "Ｑ", "Ｑ",
	"Ｋ", "Ｋ", "Ｋ", "Ｋ"};

	move(x, y);
	prints("╭───╮");
	move(x + 1, y);
	prints("│%s    │", poker[card]);
	move(x + 2, y);
	prints("│%s    │", flower[card % 4]);
	move(x + 3, y);
	prints("│      │");
	move(x + 4, y);
	prints("│      │");
	move(x + 5, y);
	prints("│      │");
	move(x + 6, y);
	prints("╰───╯");
	return 0;
}




int
gagb()
{
	int             money;
	char            genbuf[200], buf[80];
	char            ans[5] = "";
	/* 倍率        0  1   2   3   4   5   6   7   8   9   10 */
	float           bet[11] = {0, 100, 50, 10, 3, 1.5, 1.2, 0.9, 0.8, 0.5, 0.1};
	int             a, b, c, count;

	modify_user_mode(M_XAXB);
	srandom(time(0));
	money = get_money(0,"game/gagb.welcome");
	if(!money) return 0;
	move(6, 0);
	prints("[36m%s[m", MSG_SEPERATOR);
	move(17, 0);
	prints("[36m%s[m", MSG_SEPERATOR);

	do {
		itoa(random() % 10000, ans);
		for (a = 0; a < 3; a++)
			for (b = a + 1; b < 4; b++)
				if (ans[a] == ans[b])
					ans[0] = 0;
	} while (!ans[0]);

	for (count = 1; count < 11; count++) {
		do {
			getdata(5, 0, "请猜[q - 退出] → ", genbuf, 5, DOECHO, YEA);
			if (!strcmp(genbuf, "Good")) {
				prints("[%s]", ans);
				sprintf(genbuf,"猜数字作弊, 下注 %d 元", money);
				gamelog(genbuf);
				igetch();
			}
			if ( genbuf[0] == 'q' || genbuf[0] == 'Q' ) {
				sprintf(buf,"放弃猜测, 扣除压注金额 %d 元.", money);
				gamelog(buf);
				return;
			}
			c = atoi(genbuf);
			itoa(c, genbuf);
			for (a = 0; a < 3; a++)
				for (b = a + 1; b < 4; b++)
					if (genbuf[a] == genbuf[b])
						genbuf[0] = 0;
			if (!genbuf[0]) {
				move ( 18,3 );
				prints("输入数字有问题!!");
				pressanykey();
				move ( 18,3 );
				prints("                ");
			}
		} while (!genbuf[0]);
		move(count + 6, 0);
		prints("  [1;31m第 [37m%2d [31m次： [37m%s  ->  [33m%dA [36m%dB [m", count, genbuf, an(genbuf, ans), bn(genbuf, ans));
		if (an(genbuf, ans) == 4)
			break;
	}

	if (count > 10) {
		sprintf(buf, "你输了呦！正确答案是 %s，下次再加油吧!!", ans);
		sprintf(genbuf,"[1;31m可怜没猜到，输了 %d 元！[m", money);
		gamelog(genbuf);
	} else {
		int             oldmoney = money;
		money *= bet[count];
		inmoney(money);
		if (money - oldmoney > 0)
			sprintf(buf, "恭喜！总共猜了 %d 次，净赚奖金 %d 元", count, money - oldmoney);
		else if (money - oldmoney == 0)
			sprintf(buf, "唉～～总共猜了 %d 次，没输没赢！", count);
		else
			sprintf(buf, "啊～～总共猜了 %d 次，赔钱 %d 元！", count, oldmoney - money);
	}
	gamelog(buf);
	move(22, 0);
        clrtobot();
	prints(buf);
	pressanykey();
	return 0;
}

itoa(i, a)
	int             i;
	char           *a;
{
	int             j, k, l = 1000;
	//prints("itoa: i=%d ", i);

	for (j = 3; j > 0; j--) {
		k = i - (i % l);
		i -= k;
		k = k / l + 48;
		a[3 - j] = k;
		l /= 10;
	}
	a[3] = i + 48;
	a[4] = 0;

	//prints(" a=%s\n", a);
	//igetch();
}

int
an(a, b)
	char           *a, *b;
{
	int             i, k = 0;
	for (i = 0; i < 4; i++)
		if (*(a + i) == *(b + i))
			k++;
	return k;
}

int
bn(a, b)
	char           *a, *b;
{
	int             i, j, k = 0;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			if (*(a + i) == *(b + j))
				k++;
	return (k - an(a, b));
}


/*
 * 程式设计：wsyfish 自己评语：写得很烂，乱写一通，没啥深度:)
 * 相容程度：Ptt板本应该都行吧，就用inmoney和demoney，其他如Sob就要改一下罗
 * */
char           *dice[6][3] = {"        ",
	"   ●   ",
	"        ",
	"   ●   ",
	"        ",
	"   ●   ",
	"   ●   ",
	"   ●   ",
	"   ●   ",
	"●    ●",
	"        ",
	"●    ●",
	"●    ●",
	"   ●   ",
	"●    ●",
	"●    ●",
	"●    ●",
	"●    ●"
};

int
x_dice()
{
	char            choice[11], buf[60];
	int             i, money;
	char            tmpchar;/* 纪录选项 */
	char            tmpdice[3];	/* 三个骰子的值 */
	char            totaldice;
	time_t          now = time(0);

	srandom(time(0));
	time(&now);

	modify_user_mode(M_DICE);
	while (1) {
		money = get_money(0,"game/xdice.welcome");
		if(!money) return 0;
		move(2,0);
		outs("\n┌────────────────────────────────────┐\n"
		     "│ ２倍  1. 大      2. 小                                                 │\n"
		     "│ ５倍  3. 三点    4. 四点     5. 五点    6. 六点    7. 七点    8. 八点  │\n"
		     "│       9. 九点   10. 十点    11. 十一点 12. 十二点 13. 十三点 14. 十四点│\n"
		     "│      15. 十五点 16. 十六点  17. 十七点 18. 十八点                      │\n"
		     "│ ９倍 19. 一一一 20. 二二二  21. 三三三 22. 四四四 23. 五五五 24. 六六六│\n"
		     "└────────────────────────────────────┘\n");
		getdata(11, 0, "要押哪一项呢？(请输入号码) ", choice, 3, DOECHO, YEA);
		tmpchar = atoi(choice);
		if (tmpchar <= 0 || tmpchar > 24) {
			prints("要押的项目输入有误！离开赌场");
			pressanykey();
			break;
		}
		outs("\n按任一键掷出骰子....\n");
		egetch();

		do {
			totaldice = 0;
			for (i = 0; i < 3; i++) {
				tmpdice[i] = random() % 6 + 1;
				totaldice += tmpdice[i];
			}

			if (((tmpchar == 1) && totaldice > 10) ||
			    ((tmpchar == 2) && totaldice <= 10)) {
				if ((random() % 10) < 7)	/* 作弊用，中奖率为原来之70% */
					break;
			} else
				break;

		} while (tmpchar <= 2);

		if ((tmpchar <= 18 && totaldice == tmpchar) && (currentuser.money > 10000)) {
			if (tmpdice[0] > 1)
				tmpdice[0]--;
			else if (tmpdice[1] > 1)
				tmpdice[1]--;
			else if (tmpdice[2] < 5)
				tmpdice[2]++;
		}
		outs("\n╭────╮╭────╮╭────╮\n");

		for (i = 0; i < 3; i++)
			prints("│%s││%s││%s│\n",
			       dice[tmpdice[0] - 1][i],
			       dice[tmpdice[1] - 1][i],
			       dice[tmpdice[2] - 1][i]);

		outs("╰────╯╰────╯╰────╯\n\n");

		if ((tmpchar == 1 && totaldice > 10)
		    || (tmpchar == 2 && totaldice <= 10))	/* 处理大小 */
			sprintf(buf, "中了！得到２倍奖金 %d 元，总共有 %d 元",
				money * 2, inmoney(money * 2));
		else if (tmpchar <= 18 && totaldice == tmpchar)	/* 处理总和 */
			sprintf(buf, "中了！得到５倍奖金 %d 元，总共有 %d 元",
				money * 5, inmoney(money * 5));
		else if ((tmpchar - 18) == tmpdice[0] && (tmpdice[0] == tmpdice[1])
			 && (tmpdice[1] == tmpdice[2]))	/* 处理三个一样总和 */
			sprintf(buf, "中了！得到９倍奖金 %d 元，总共有 %d 元",
				money * 9, inmoney(money * 9));

		else		/* 处理没中 */
			sprintf(buf, "很可惜没有押中！输了 %d 元",money);
		gamelog(buf);
		prints(buf);
		pressanykey();
	}
	return 0;
}



/* write by dsyan               */
/* 87/10/24                     */
/* 天长地久 Forever.twbbs.org   */

char            card[52], mycard[5], cpucard[5], sty[100], now;

static int
forq(a, b)
	char           *a, *b;
{
	char            c = (*a) % 13;
	char            d = (*b) % 13;
	if (!c)
		c = 13;
	if (!d)
		d = 13;
	if (c == 1)
		c = 14;
	if (d == 1)
		d = 14;
	if (c == d)
		return *a - *b;
	return c - d;
}

int
p_gp()
{
	char            genbuf[200], hold[5];
	char            ans[5];
	int             bet, i, j, k, tmp, x, xx, doub, gw = 0, cont = 0;

	srandom(time(0));
	modify_user_mode(M_GP);
	bet = 0;
	while (1) {
		clear();
		ans[0] = 0;
		if (cont == 0) 
			if(!(bet = get_money(bet,"game/gp.welcome"))) return 0;
		move(21, 0);
		clrtoeol();
		if (cont > 0)
			prints("[33;1m (←)(→)改变选牌  (SPCAE)改变换牌  (Enter)确定[m");
		else
			prints("[33;1m (←)(→)改变选牌  (d)Double  (SPCAE)改变换牌  (Enter)确定[m");
		show_money(bet,NULL,NA);

		for (i = 0; i < 52; i++)
			card[i] = i;	/* 0~51 ..黑杰克是 1~52 */

		for (i = 0; i < 1000; i++) {
			j = random() % 52;
			k = random() % 52;
			tmp = card[j];
			card[j] = card[k];
			card[k] = tmp;
		}

		now = doub = 0;
		for (i = 0; i < 5; i++) {
			mycard[i] = card[now++];
			hold[i] = 1;
		}
		qsort(mycard, 5, sizeof(char), forq);

		for (i = 0; i < 5; i++)
			show_card(0, mycard[i], i);

		x = xx = tmp = 0;
		while (tmp != 10) {
			for (i = 0; i < 5; i++) {
				move(16, i * 4 + 2);
				outs(hold[i] < 0 ? "保" : "  ");
				move(17, i * 4 + 2);
				outs(hold[i] < 0 ? "留" : "  ");
			}
			move(8, xx * 4 + 2);
			outs("  ");
			move(8, x * 4 + 2);
			outs("↓");
			move(t_lines - 1, 0);
			refresh();
			xx = x;

			tmp = egetch();
			switch (tmp) {
#ifdef GP_DEBUG
			case KEY_UP:
				getdata(21, 0, "把牌换成> ", genbuf, 3, DOECHO, YEA);
				mycard[x] = atoi(genbuf);
				qsort(mycard, 5, sizeof(char), forq);
				for (i = 0; i < 5; i++)
					show_card(0, mycard[i], i);
				break;
#endif
			case KEY_LEFT:
			case 'l':
				x = x ? x - 1 : 4;
				break;
			case KEY_RIGHT:
			case 'r':
				x = (x == 4) ? 0 : x + 1;
				break;
			case ' ':
				hold[x] *= -1;
				break;
			case 'd':
				if (!cont && !doub && currentuser.money >= bet) {
					doub++;
					move(21, 0);
					clrtoeol();
					prints("[33;1m (←)(→)改变选牌  (SPCAE)改变换牌  (Enter)确定[m");
					demoney(bet);
					bet *= 2;
					show_money(bet,NULL,NA);
				}
				break;
			}
		}

		for (i = 0; i < 5; i++)
			if (hold[i] == 1)
				mycard[i] = card[now++];
		qsort(mycard, 5, sizeof(char), forq);
		for (i = 0; i < 5; i++)
			show_card(0, mycard[i], i);
		move(11, x * 4 + 2);
		outs("  ");

		cpu();
#ifdef GP_DEBUG
		for (x = 0; x < 5; x++) {
			getdata(21, 0, "把牌换成> ", genbuf, 3, DOECHO, YEA);
			cpucard[x] = atoi(genbuf);
		}
		qsort(cpucard, 5, sizeof(char), forq);
		for (i = 0; i < 5; i++)
			show_card(1, cpucard[i], i);
#endif
		i = gp_win();

		if (i < 0) {
			inmoney(bet * 2);
			sprintf(genbuf, "哇!!好棒喔!!净赚 %d 元咧.. :DDD", bet);
			prints(genbuf);
			if (cont > 0)
				sprintf(genbuf, "连胜 %d 次, 赢了 %d 元",
					cont + 1, bet);
			else
				sprintf(genbuf, "赢了 %d 元", bet);
			gamelog(genbuf);
			bet = (bet > 50000 ? 100000 : bet * 2);
			gw = 1;
		} else if (i > 1000) {
			switch (i) {
			case 1001:
				doub = 15;
				break;
			case 1002:
				doub = 10;
				break;
			case 1003:
				doub = 5;
				break;
			}
			inmoney(bet * 2 * doub);
			sprintf(genbuf, "哇!!好棒喔!!净赚 %d 元咧.. :DDD", bet * 2 * doub - bet);
			prints(genbuf);
			if (cont > 0)
				sprintf(genbuf, "连胜 %d 次, 赢了 %d 元",
				   cont + 1, bet * doub);
			else
				sprintf(genbuf, "赢了 %d 元", bet * doub);
			gamelog(genbuf);
			bet = (bet > 50000 ? 100000 : bet * 2 * doub);
			gw = 1;
			bet = (bet >= 100000 ? 100000 : bet);
		} else {
			prints("输了..:~~~");
			if (cont > 1)
				sprintf(genbuf, "中止 %d 连胜, 输了 %d 元", cont, bet);
			else
				sprintf(genbuf, "输了 %d 元", bet);
			gamelog(genbuf);
			cont = 0;
			bet = 0;
			pressanykey();
		}

		if (gw == 1) {
			gw = 0;
			getdata(21, 0, "您要把奖金继续压注吗 (y/n)?", ans, 2, DOECHO, YEA);
			if (ans[0] == 'y' || ans[0] == 'Y') {
				demoney (bet);/* added by soff */
				cont++;
			} else {
				cont = 0;
				bet = 0;
			}
		}
	}
}

show_card(isDealer, c, x)
	int             isDealer, c, x;
{
	int             beginL;
	char           *suit[4] = {"Ｃ", "Ｄ", "Ｈ", "Ｓ"};
	char           *num[13] = {"Ｋ", "Ａ", "２", "３", "４", "５", "６",
	"７", "８", "９", "10", "Ｊ", "Ｑ"};

	beginL = (isDealer) ? 2 : 12;
	move(beginL, x * 4);
	outs("╭───╮");
	move(beginL + 1, x * 4);
	prints("│%2s    │", num[c % 13]);
	move(beginL + 2, x * 4);
	prints("│%2s    │", suit[c / 13]);	/* ←这里跟黑杰克 */
#ifdef GP_DEBUG
	move(beginL + 3, x * 4);
	prints("│%2d    │", c);	/* 有点不同喔!! */
#else
	move(beginL + 3, x * 4);
	outs("│      │");	/* 有点不同喔!! */
#endif
	move(beginL + 4, x * 4);
	outs("│      │");
	move(beginL + 5, x * 4);
	outs("│      │");
	move(beginL + 6, x * 4);
	outs("╰───╯");
}

cpu()
{
	char            i, j, hold[5];
	char            p[13], q[5], r[4];
	char            a[5], b[5];

	for (i = 0; i < 5; i++) {
		cpucard[i] = card[now++];
		hold[i] = 0;
	}
	qsort(cpucard, 5, sizeof(char), forq);
	for (i = 0; i < 5; i++)
		show_card(1, cpucard[i], i);

	tran(a, b, cpucard);
	check(p, q, r, cpucard);

	for (i = 0; i < 13; i++)
		if (p[i] > 1)
			for (j = 0; j < 5; j++)
				if (i == cpucard[j] % 13)
					hold[j] = -1;

	for (i = 0; i < 5; i++) {
		if (a[i] == 13 || a[i] == 1)
			hold[i] = -1;
		move(6, i * 4 + 2);
		outs(hold[i] < 0 ? "保" : "  ");
		move(7, i * 4 + 2);
		outs(hold[i] < 0 ? "留" : "  ");
	}
	move(10,25);
	prints("[44;37m电脑换牌前...[40m");
	pressanykey();
	move(10,0); clrtoeol();

	for (i = 0; i < 5; i++)
		if (!hold[i])
			cpucard[i] = card[now++];
	qsort(cpucard, 5, sizeof(char), forq);
	for (i = 0; i < 5; i++)
		show_card(1, cpucard[i], i);
}

int
gp_win()
{
	int             my, cpu, ret;
	char            myx, myy, cpux, cpuy;

	my = complex(mycard, &myx, &myy);
	cpu = complex(cpucard, &cpux, &cpuy);
	show_style(my, cpu);

	if (my != cpu)
		ret = my - cpu;
	else if (myx == 1 && cpux != 1)
		ret = -1;
	else if (myx != 1 && cpux == 1)
		ret = 1;
	else if (myx != cpux)
		ret = cpux - myx;
	else if (myy != cpuy)
		ret = cpuy - myy;

	if (ret < 0)
		switch (my) {
		case 1:
			ret = 1001;
			break;
		case 2:
			ret = 1002;
			break;
		case 3:
			ret = 1003;
			break;
		}

	return ret;
}

//同花顺、铁枝、葫、同花、顺、三条、兔胚、胚、一只
int
complex(cc, x, y)
	char           *cc, *x, *y;
{
	char            p[13], q[5], r[4];
	char            a[5], b[5], c[5], d[5];
	int             i, j, k;

	tran(a, b, cc);
	check(p, q, r, cc);

	/* 同花顺 */
	if ((a[0] == a[1] - 1 && a[1] == a[2] - 1 && a[2] == a[3] - 1 && a[3] == a[4] - 1) &&
	    (b[0] == b[1] && b[1] == b[2] && b[2] == b[3] && b[3] == b[4])) {
		*x = a[4];
		*y = b[4];
		return 1;
	}
	if (a[4] == 1 && a[0] == 2 && a[1] == 3 && a[2] == 4 && a[3] == 5 &&
	    (b[0] == b[1] && b[1] == b[2] && b[2] == b[3] && b[3] == b[4])) {
		*x = a[3];
		*y = b[4];
		return 1;
	}
	if (a[4] == 1 && a[0] == 10 && a[1] == 11 && a[2] == 12 && a[3] == 13 &&
	    (b[0] == b[1] && b[1] == b[2] && b[2] == b[3] && b[3] == b[4])) {
		*x = 1;
		*y = b[4];
		return 1;
	}
	/* 四张 */
	if (q[4] == 1) {
		for (i = 0; i < 13; i++)
			if (p[i] == 4)
				*x = i ? i : 13;
		return 2;
	}
	/* 葫芦 */
	if (q[3] == 1 && q[2] == 1) {
		for (i = 0; i < 13; i++)
			if (p[i] == 3)
				*x = i ? i : 13;
		return 3;
	}
	/* 同花 */
	for (i = 0; i < 4; i++)
		if (r[i] == 5) {
			*x = i;
			return 4;
		}
	/* 顺子 */
	memcpy(c, a, 5);
	memcpy(d, b, 5);
	for (i = 0; i < 4; i++)
		for (j = i; j < 5; j++)
			if (c[i] > c[j]) {
				k = c[i];
				c[i] = c[j];
				c[j] = k;
				k = d[i];
				d[i] = d[j];
				d[j] = k;
			}
	if (10 == c[1] && c[1] == c[2] - 1 && c[2] == c[3] - 1 && c[3] == c[4] - 1 && c[0] == 1) {
		*x = 1;
		*y = d[0];
		return 5;
	}
	if (c[0] == c[1] - 1 && c[1] == c[2] - 1 && c[2] == c[3] - 1 && c[3] == c[4] - 1) {
		*x = c[4];
		*y = d[4];
		return 5;
	}
	/* 三条 */
	if (q[3] == 1)
		for (i = 0; i < 13; i++)
			if (p[i] == 3) {
				*x = i ? i : 13;
				return 6;
			}
	/* 兔胚 */
	if (q[2] == 2) {
		for (*x = 0, i = 0; i < 13; i++)
			if (p[i] == 2) {
				if ((i > 1 ? i : i + 13) > (*x == 1 ? 14 : *x)) {
					*x = i ? i : 13;
					*y = 0;
					for (j = 0; j < 5; j++)
						if (a[j] == i && b[j] > *y)
							*y = b[j];
				}
			}
		return 7;
	}
	/* 胚 */
	if (q[2] == 1)
		for (i = 0; i < 13; i++)
			if (p[i] == 2) {
				*x = i ? i : 13;
				*y = 0;
				for (j = 0; j < 5; j++)
					if (a[j] == i && b[j] > *y)
						*y = b[j];
				return 8;
			}
	/* 一张 */
	*x = 0;
	*y = 0;
	for (i = 0; i < 5; i++)
		if ((a[i] = a[i] ? a[i] : 13 > *x || a[i] == 1) && *x != 1) {
			*x = a[i];
			*y = b[i];
		}
	return 9;
}

/* a 是点数 .. b 是花色 */
tran(a, b, c)
	char           *a, *b, *c;
{
	int             i;
	for (i = 0; i < 5; i++) {
		a[i] = c[i] % 13;
		if (!a[i])
			a[i] = 13;
	}

	for (i = 0; i < 5; i++)
		b[i] = c[i] / 13;
}

check(p, q, r, cc)
	char           *p, *q, *r, *cc;
{
	char            i;

	for (i = 0; i < 13; i++)
		p[i] = 0;
	for (i = 0; i < 5; i++)
		q[i] = 0;
	for (i = 0; i < 4; i++)
		r[i] = 0;

	for (i = 0; i < 5; i++)
		p[cc[i] % 13]++;

	for (i = 0; i < 13; i++)
		q[p[i]]++;

	for (i = 0; i < 5; i++)
		r[cc[i] / 13]++;
}

//同花顺、铁枝、葫、同花、顺、三条、兔胚、胚、一只
show_style(my, cpu)
	char            my, cpu;
{
	char           *style[9] = {"同花顺", "四张", "葫芦", "同花", "顺子",
	"三条", "兔胚", "单胚", "一张"};
	move(5, 26);
	prints("[41;37;1m%s[m", style[cpu - 1]);
	move(15, 26);
	prints("[41;37;1m%s[m", style[my - 1]);
	sprintf(sty, "我的牌[44;1m%s[m..电脑的牌[44;1m%s[m",
		style[my - 1], style[cpu - 1]);
}

/********************************/
/* BBS 站内游戏–天地九九       */
/* 11/26/98 */
/* dsyan.bbs@Forever.twbbs.org  */
/********************************/

#undef  NINE_DEBUG

//0 1 2 3 4 5 6 7 8 9 10 11 12	/* 电脑 AI 所在 */
// char         cp[13] = {9, 8, 7, 6, 3, 2, 1, 0, 11, 5, 4, 10, 12};
char            tb[13] = {7, 6, 5, 4, 10, 9, 3, 2, 1, 0, 11, 8, 12};
char           *tu[4] = {"↓", "→", "↑", "←"};
char            card[52], ahand[4][5], now, dir, turn, live;
static char            buf[255];
int             sum;

static int
forqp(a, b)
	char           *a, *b;
{
	return tb[(*a) % 13] - tb[(*b) % 13];
}

p_nine()
{
	int             bet, i, j, k, tmp, x, xx;
	srandom(time(0));
	while (1) {
		modify_user_mode(M_NINE);
		bet = get_money(bet=0,"game/99.welcome");
		if (!bet)
			return 0;
//		move(1, 0);
                ansimore("game/99", NA);
/*
		fs = fopen("r");
		while (fgets(genbuf, 255, fs))
			prints(genbuf);
*/
		move(21, 0);
		clrtoeol();
		prints("[33;1m (←)(→)改变选牌  (↓)查询该牌作用 (SPCAE)(Enter)打牌 (Q)退出 [m");
		show_money(bet,NULL,NA);

		for (i = 0; i < 52; i++)
			card[i] = 1;

		for (i = 0; i < 4; i++) {
			for (j = 0; j < 5; j++) {
				while (-1) {
					k = random() % 52;
					if (card[k]) {
						ahand[i][j] = k;
						card[k] = 0;
						break;
					}
				}
			}
		}

		qsort(ahand[0], 5, sizeof(char), forqp);
		x = xx = now = turn = sum = tmp = 0;
		dir = 1;
		live = 3;
		show_mycard(100);

		while (1) {
			move(8, 52);
			prints(tu[turn]);
			refresh();
			sum++;
			if (turn)
				//电脑
			{
				qsort(ahand[turn], 5, sizeof(char), forqp);
				for (i = 0; i < 5; i++) {
					tmp = ahand[turn][i] % 13;
					if (tmp == 0 || tmp == 4 || tmp == 5 || tmp > 9)
						break;
					if (now + tmp <= 99 && now + tmp >= 0)
						break;
				}
				if (i < 2)
					if (tmp == 0 || tmp == 4 || tmp == 5 || tmp > 9)
						i += random() % (5 - i);
				if (i == 5)
					cpu_die();
				else
					add(&(ahand[turn][i]));
				if (random() % 5 == 0)
					mmsg();
				continue;
			}
			if (!live) {
				//gamelog(NINE, "[32;1m在 %d 张牌赢了.. :)) [m %d", sum, bet);
				if (sum < 25)
					live = 20;
				else if (sum < 50)
					live = 15;
				else if (sum < 100)
					live = 10;
				else if (sum < 150)
					live = 7;
				else if (sum < 200)
					live = 5;
				else
					live = 3;
				inmoney(bet * (live + 1));
				sprintf(buf, "赢了 %d ... :D", bet * live);
				prints(buf);
				break;
			}
			tmp = ahand[0][4] % 13;
			if (tmp != 0 && tmp != 4 && tmp != 5 && tmp < 10 && now + tmp > 99) {
				prints("呜呜呜..被电爆了!!.. :~");
				//game_log(NINE, "[31;1m在 %d 张牌被电脑电爆掉了.. :~ %d[m %d", sum, live, bet);
				break;
			}
			while (tmp != 13 && tmp != 32)
				//人类
			{
				move(18, xx * 4 + 30);
				outs("  ");
				move(18, (xx = x) * 4 + 30);

				if (tb[ahand[0][x] % 13] < 7) {
					if (ahand[0][x] % 13 + now > 99)
						outs("！");
					else
						outs("○");
				} else
					outs("★");

				move(18, x * 4 + 31);
				refresh();

				switch (tmp = egetch()) {
#ifdef NINE_DEBUG
				case KEY_UP:
					getdata(22, 0, "把牌换成> ", genbuf, 3, DOECHO, YEA);
					card[ahand[0][x]] = 3;
					ahand[0][x] = atoi(genbuf);
					card[ahand[0][x]] = 0;
					qsort(ahand[0], 5, sizeof(char), forqp);
					show_mycard(100);
					break;
#endif
				case KEY_DOWN:
					nhelp(ahand[0][x]);
					break;
				case KEY_LEFT:
				case 'l':
                                move(18, 30);
                                outs("                                            ");
					x = x ? x - 1 : 4;
					break;
				case KEY_RIGHT:
				case 'r':
                                move(18, 30);
                                outs("                                            ");
					x = (x == 4) ? 0 : x + 1;
					break;
				case 'q':
					break;
				}
				if (tmp == 'q')
					return;
			}

			move(18, xx * 4 + 30);
			outs("  ");
			if (add(&(ahand[0][x]))) {
				prints("呜呜呜..白烂爆了!!.. :~");
				//game_log(NINE, "[31;1m在 %d 张牌白烂爆了.. :~ %d[m %d", sum, live, bet);
				break;
			}
			qsort(ahand[0], 5, sizeof(char), forqp);
			show_mycard(100);
		}
	}
}

show_mycard(t)
	char            t;
{
	char            i;
#ifdef NINE_DEBUG
	char            j;
#endif
	char           *suit[4] = {"Ｃ", "Ｄ", "Ｈ", "Ｓ"};
	char           *num[13] = {"Ｋ", "Ａ", "２", "３", "４", "５", "６",
	"７", "８", "９", "10", "Ｊ", "Ｑ"};
	char            coorx[4] = {30, 38, 30, 22};
	char            coory[4] = {8, 6, 4, 6};

#ifdef NINE_DEBUG
	move(22, 0);
	for (i = 3; i > 0; i--) {
		if (ahand[i][0] == -1)
			continue;
		qsort(ahand[i], 5, sizeof(char), forqp);
		for (j = 0; j < 5; j++)
			prints(num[ahand[i][j] % 13]);
		prints("  ");
	}
#endif

	if (t == 100) {
		for (i = 0; i < 5; i++) {
			move(16, 30 + i * 4);
			prints(num[ahand[0][i] % 13]);
			move(17, 30 + i * 4);
			prints(suit[ahand[0][i] / 13]);
		}
		return;
	}
	move(coory[turn], coorx[turn]);
	prints("╭───╮");
	move(coory[turn] + 1, coorx[turn]);
	prints("│%s    │", num[t % 13]);
	move(coory[turn] + 2, coorx[turn]);
	prints("│%s    │", suit[t / 13]);
	move(coory[turn] + 3, coorx[turn]);
	prints("│      │");
	move(coory[turn] + 4, coorx[turn]);
	prints("│      │");
	//prints("│    %s│", num[t % 13]);
	move(coory[turn] + 5, coorx[turn]);
	prints("│      │");
	//prints("│    %s│", suit[t / 13]);
	move(coory[turn] + 6, coorx[turn]);
	prints("╰───╯");

	move(7, 50);
	prints("%s  %s", dir == 1 ? "↙" : "↗", dir == 1 ? "↖" : "↘");
	move(9, 50);
	prints("%s  %s", dir == 1 ? "↘" : "↖", dir == 1 ? "↗" : "↙");

	move(19, 52);
	prints("点数：%c%c%c%c", (now / 10) ? 162 : 32,
	       (now / 10) ? (now / 10 + 175) : 32, 162, now % 10 + 175);
	move(20, 52);
	prints("张数：%d", sum);
	refresh();
	sleep(1);
	move(21, 0);
	clrtoeol();
	refresh();
	prints("[33;1m (←)(→)改变选牌  (↓)查询该牌作用 (SPCAE)(Enter)打牌 [m");
}

int 
add(t)
	char           *t;
{
	int             k = 0, change = 0;

	switch (*t % 13) {
	case 4:
		//4 