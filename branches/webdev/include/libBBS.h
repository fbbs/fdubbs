#ifndef _LIBBBS_H_

#define _LIBBBS_H_

//fileio.c
void file_append(const char *fpath, const char *msg);
int dashf(char *fname);
int dashd(char *fname);
int part_cp(char *src, char *dst, char *mode);
int f_cp(char *src, char *dst, int mode);
int f_ln(char *src, char *dst);
int valid_fname(char *str);
int touchfile(char *filename);
int f_rm(char *fpath);

//mmdecode.c
void _mmdecode(unsigned char *str);

//modetype.c
char *ModeType(int mode);

//string.c
char *strtolower(char *dst, const char *src);
char *strtoupper(char *dst, const char *src);
char *strcasestr_gbk(const char *haystack, const char *needle);
char *ansi_filter(char *dst, const char *src);
int getdatestring(time_t time, int mode);
int ellipsis(char *str, int len);
char *rtrim(char *str);
char *trim(char *str);
size_t strlcpy(char *dst, const char *src, size_t siz);

//boardrc.c
void brc_update(const char *userid, const char *board);
int brc_initial(const char* userid, const char *board);
void brc_addlist(const char *filename);
int brc_unread(const char *filename);
int brc_unread1(int ftime);
int brc_clear(int ent, const char *direct, int clearall);
void brc_zapbuf(int *zbuf);

//record.c
long get_num_records(const char *filename, const int size);
int append_record(const char *filename, const void *record, int size);
int get_record(char *filename, void *rptr, int size, int id);
int get_records(const char *filename, void *rptr, int size, int id,
		int number);
int apply_record(char *filename, APPLY_FUNC_ARG fptr, int size, void *arg,
		int applycopy, int reverse);
int search_record(char *filename, void *rptr, int size,
		RECORD_FUNC_ARG fptr, void *farg);
int substitute_record(char *filename, void *rptr, int size, int id);
int delete_record(char *filename, int size, int id,
		RECORD_FUNC_ARG filecheck, void *arg);
int delete_range(char *filename, int id1, int id2);
int insert_record(char *filename, int size, RECORD_FUNC_ARG filecheck,
		void *arg);

//pass.c
char *genpasswd(const char *pw);
int checkpasswd(char *pw_crypted, char *pw_try);

//shm.c
void *attach_shm(const char *shmstr, int defaultkey, int shmsize);
void *attach_shm2(const char *shmstr, int defaultkey, int shmsize, int *iscreate);
int remove_shm(const char *shmstr, int defaultkey, int shmsize);
int safe_mmapfile(const char *filename, int openflag, int prot, int flag,
		void **ret_ptr, size_t *size, int *ret_fd);
int safe_mmapfile_handle(int fd, int openflag, int prot, int flag,
		void **ret_ptr, size_t *size);
void end_mmapfile(void *ptr, int size, int fd);
#endif
