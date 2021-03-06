// Funtiions to attach or remove shared memory segements.

#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>

// Key-value pairs of shared memory.
struct _shmkey {
	char key[16];
	int value;
};

// TODO: Use enum.
// Shared memory keys.
const static struct _shmkey shmkeys[] = {
	{ "BCACHE_SHMKEY", 30000 }, { "UCACHE_SHMKEY", 30010 },
	{ "UTMP_SHMKEY", 30020 }, { "ACBOARD_SHMKEY", 30030 },
	{ "ISSUE_SHMKEY", 30040 }, { "GOODBYE_SHMKEY", 30050 },
	{ "WELCOME_SHMKEY", 30060 }, { "STAT_SHMKEY", 30070 },
#ifdef ALLOWSWITCHCODE
	{ "CONV_SHMKEY", 30080 },
#endif
	{ "ACACHE_SHMKEY", 30005 }, { "", 0 }
};

// Prints error message.
static int attach_err(int shmkey, const char *name, int err)
{
	char buf[STRLEN];
	snprintf(buf, sizeof(buf), "Error! %s error #%d! key = %x.\n",
			name, err, shmkey);
	write(STDOUT_FILENO, buf, strlen(buf));
	return 0;
}

// Searches 'keyname' in array 'shmkey'
// Returns key value if found, 0 otherwise.
static int search_shmkey(const char *keyname)
{
	int i = 0, found = 0;
	// Pointer check.
	if (keyname == NULL)
		return 0;
	// Search 'keyname' in 'shmkey'.
	while (shmkeys[i].key[0] != '\0') {
		if (strcmp(shmkeys[i].key, keyname) == 0) {
			found = shmkeys[i].value;
			break;
		}
		i++;
	}
	// Error logging.
	if (found == 0) {
		char buf[STRLEN];
		snprintf(buf, sizeof(buf), "%s(): cannot found %s SHMKEY entry!",
				__func__, keyname);
		report(buf, "");
	}
	return found;
}

// Finds shared memory key corresponding to 'shmstr'.
// If not found, uses defaultkey instead.
// Tries to get shared memory segment according to key.
// If the shared memory segment does not exist, creates one.
// Finally attaches the shared memory segment to the process.
// Returns the address of the attached shared memory segment,
// NULL pointer on error.
void *attach_shm(const char *shmstr, int defaultkey, int shmsize)
{
	void *shmptr = NULL;
	int shmkey, shmid;
	// Search for shared memory(shm) key. If not found, use defaultkey.
	shmkey = search_shmkey(shmstr);
	if (shmkey < 1024)
		shmkey = defaultkey;
	// Get existing shm.
	shmid = shmget(shmkey, shmsize, 0);
	if (shmid < 0) {
		// If shm does not exist, try to create one.
		shmid = shmget(shmkey, shmsize, IPC_CREAT | 0640);
		if (shmid < 0) {
			attach_err(shmkey, "shmget", errno);
			return NULL;
		}
		// Attach shm to the process.
		shmptr = (void *) shmat(shmid, NULL, 0);
		if (shmptr == (void *) -1) {
			attach_err(shmkey, "shmat", errno);
			return NULL;
		}
		// Initialization.
		memset(shmptr, 0, shmsize);
	} else {
		// shm already exists.
		// Attach shm to the process. No init needed.
		shmptr = (void *) shmat(shmid, NULL, 0);
		if (shmptr == (void *) -1) {
			attach_err(shmkey, "shmat", errno);
			return NULL;
		}
	}
	return shmptr;
}

// Does the same to attach_shm().
// If shared memory segment is newly created, 'iscreate' is set to 1.
// If shared memory segment already exists, 'iscreate' is set to 0.
void *attach_shm2(const char *shmstr, int defaultkey, int shmsize, int *iscreate)
{
	void *shmptr = NULL;
	int shmkey, shmid;

	shmkey = search_shmkey(shmstr);
	if (shmkey < 1024)
		shmkey = defaultkey;
	shmid = shmget(shmkey, shmsize, 0);
	if (shmid < 0) {
		shmid = shmget(shmkey, shmsize, IPC_CREAT | 0644);
		*iscreate = 1;
		if (shmid < 0) {
			attach_err(shmkey, "shmget", errno);
			return NULL;
		}
		shmptr = (void *) shmat(shmid, NULL, 0);
		if (shmptr == (void *) - 1) {
			attach_err(shmkey, "shmat", errno);
			return NULL;
		}
		memset(shmptr, 0, shmsize);
	} else {
		*iscreate = 0;
		shmptr = (void *) shmat(shmid, NULL, 0);
		if (shmptr == (void *) -1) {
			attach_err(shmkey, "shmat", errno);
			return NULL;
		}
	}
	return shmptr;
}

// Finds shared memory key corresponding to 'shmstr'.
// If not found, uses defaultkey instead.
// Then mark the segment to be destroyed.
// Returns result of 'shmctl' (-1 on error).
int remove_shm(const char *shmstr, int defaultkey, int shmsize)
{
	int shmkey, shmid;

	shmkey = search_shmkey(shmstr);
	if (shmkey < 1024)
		shmkey = defaultkey;
	shmid = shmget(shmkey, shmsize, 0);
	return shmctl(shmid, IPC_RMID, NULL);
}

