#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>

void setoboard(char *bname) {
	register int i;

	if (resolve_boards() < 0)
		exit(1);
	for (i = 0; i < numboards; i++) {
		if (bcache[i].flag & BOARD_POST_FLAG || HAS_PERM(bcache[i].level)
				|| (bcache[i].flag & BOARD_NOZAP_FLAG)) {
			if (bcache[i].filename[0] != 0 && bcache[i].filename[0] != ' ') {
				strcpy(bname, bcache[i].filename);
				return;
			}
		}
	}
}

int
normal_board(bname)
char *bname;
{
	register int i;
	if (strcmp(bname, DEFAULTBOARD) == 0)
	return 1;
	if ((i = getbnum(bname, &currentuser)) == 0)//���治�ɼ�
	return 0;
	if (bcache[i - 1].flag & BOARD_NOZAP_FLAG)
	return 1;
	if ((bcache[i - 1].flag & BOARD_POST_FLAG) || bcache[i - 1].level)
	return 0;
	return 1;
}
