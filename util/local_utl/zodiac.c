#include "bbs.h"
#include "chart.h"

int main(int argc, char **argv)
{
	struct userec user;
	struct bbsstat bst;
	memset(&bst, 0, sizeof(bst));
	FILE *fp = fopen(BBSHOME"/.PASSWDS", "rb");
	if (fp == NULL)
		return 1;
	int i, zodiac, mtotal = 0, ftotal = 0;
	while (fread(&user, sizeof(user), 1, fp) > 0) {
		if (user.birthmonth == 0)
			continue;
		// since every one born in the 20th century..
		zodiac = user.birthyear % 12;
		if (user.gender == 'M') {
			bst.value[zodiac * 2]++;
			mtotal++;
		} else {
			bst.value[zodiac * 2 + 1]++;
			ftotal++;
		}
	}
	fclose(fp);

	for (i = 0; i < 12; i++) {
		bst.color[i * 2] = ANSI_COLOR_CYAN;
		bst.color[i * 2 + 1] = ANSI_COLOR_PURPLE;
	}

	int item = draw_chart(&bst);
	char *mg = left_margin(item, NULL);
	printf("\033[0;1m%s0└─── 目前本站注册使用者生肖统计───%s──\n"
			"%s\033[1;33m     鼠    牛    虎    兔    龙    蛇    马    羊    猴    鸡    狗    猪\n\n"
			"%s                          \033[1;36m█ 男生 (%d)    \033[35m█ 女生 (%d)\033[m\n",
			mg, getdatestring(time(NULL), DATE_ZH), mg, mg, mtotal, ftotal);
	return 0;
}

