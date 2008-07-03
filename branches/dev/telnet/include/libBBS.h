#ifdef __LIBBBS_H

#define __LIBBBS_H

//fileio.c
void file_append(char *fpath,char *msg);
int dashf(char *fname);
int dashd(char *fname);
int part_cp(char *src, char *dst, char *mode);
int f_cp(char *src, char *dst, int mode);
int f_ln(char *src, char *dst);
int valid_fname(char *str);
int touchfile(char *filename);
int f_rm(char *fpath);

//mmdecode.c
void _mmdecode(str);

//string.c
char *strtolower(char *dest, char *src);
char *strtoupper(char *dest, char *src);
char *strcasestr_zh(char *haystack, char *needle);
char *ansi_filter(char *dest, char *src);
int getdatestring(time_t time, int mode);

#endif
