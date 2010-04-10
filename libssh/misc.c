/*
 * misc.c - useful client functions
 *
 * This file is part of the SSH Library
 *
 * Copyright (c) 2003-2009 by Aris Adamantiadis
 * Copyright (c) 2008-2009 by Andreas Schneider <mail@cynapses.org>
 *
 * The SSH Library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * The SSH Library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the SSH Library; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "config.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>


#ifdef _WIN32
#define _WIN32_IE 0x0501 //SHGetSpecialFolderPath
#include <winsock2.h> // Must be the first to include
#include <shlobj.h>
#include <direct.h>
#else
#include <pwd.h>
#include <arpa/inet.h>
#endif

#include "libssh/priv.h"
#include "libssh/misc.h"

#ifdef HAVE_LIBGCRYPT
#define GCRYPT_STRING "/gnutls"
#else
#define GCRYPT_STRING ""
#endif

#ifdef HAVE_LIBCRYPTO
#define CRYPTO_STRING "/openssl"
#else
#define CRYPTO_STRING ""
#endif

#if defined(HAVE_LIBZ) && defined(WITH_LIBZ)
#define LIBZ_STRING "/zlib"
#else
#define LIBZ_STRING ""
#endif

/** \defgroup ssh_misc SSH Misc
 * \brief Misc functions
 */
/** \addtogroup ssh_misc
 * @{ */

#ifdef _WIN32
char *ssh_get_user_home_dir(void) {
  char tmp[MAX_PATH] = {0};
  char *szPath = NULL;

  if (SHGetSpecialFolderPathA(NULL, tmp, CSIDL_PROFILE, TRUE)) {
    szPath = malloc(strlen(tmp) + 1);
    if (szPath == NULL) {
      return NULL;
    }

    strcpy(szPath, tmp);
    return szPath;
  }

  return NULL;
}

/* we have read access on file */
int ssh_file_readaccess_ok(const char *file) {
  if (_access(file, 4) < 0) {
    return 0;
  }

  return 1;
}

#define SSH_USEC_IN_SEC         1000000LL
#define SSH_SECONDS_SINCE_1601  11644473600LL

int gettimeofday(struct timeval *__p, void *__t) {
  union {
    unsigned long long ns100; /* time since 1 Jan 1601 in 100ns units */
    FILETIME ft;
  } now;

  GetSystemTimeAsFileTime (&now.ft);
  __p->tv_usec = (long) ((now.ns100 / 10LL) % SSH_USEC_IN_SEC);
  __p->tv_sec  = (long)(((now.ns100 / 10LL ) / SSH_USEC_IN_SEC) - SSH_SECONDS_SINCE_1601);

  return (0);
}
#else /* _WIN32 */
char *ssh_get_user_home_dir(void) {
  char *szPath = NULL;
  struct passwd *pwd = NULL;

  pwd = getpwuid(getuid());
  if (pwd == NULL) {
    return NULL;
  }

  szPath = strdup(pwd->pw_dir);

  return szPath;
}

/* we have read access on file */
int ssh_file_readaccess_ok(const char *file) {
  if (access(file, R_OK) < 0) {
    return 0;
  }

  return 1;
}
#endif

uint64_t ntohll(uint64_t a) {
#ifdef WORDS_BIGENDIAN
  return a;
#else
  uint32_t low = (uint32_t)(a & 0xffffffff);
  uint32_t high = (uint32_t)(a >> 32);
  low = ntohl(low);
  high = ntohl(high);

  return ((((uint64_t) low) << 32) | ( high));
#endif
}

/**
 * @brief Check if libssh is the required version or get the version
 * string.
 *
 * @param req_version   The version required.
 *
 * @return              If the version of libssh is newer than the version
 *                      required it will return a version string.
 *                      NULL if the version is older.
 *
 * Example:
 *
 * @code
 *  if (ssh_version(SSH_VERSION_INT(0,2,1)) == NULL) {
 *    fprintf(stderr, "libssh version is too old!\n");
 *    exit(1);
 *  }
 *
 *  if (debug) {
 *    printf("libssh %s\n", ssh_version(0));
 *  }
 * @endcode
 */
const char *ssh_version(int req_version) {
  if (req_version <= LIBSSH_VERSION_INT) {
    return SSH_STRINGIFY(LIBSSH_VERSION) GCRYPT_STRING CRYPTO_STRING
      LIBZ_STRING;
  }

  return NULL;
}

struct ssh_list *ssh_list_new(){
  struct ssh_list *ret=malloc(sizeof(struct ssh_list));
  if(!ret)
    return NULL;
  ret->root=ret->end=NULL;
  return ret;
}

void ssh_list_free(struct ssh_list *list){
  struct ssh_iterator *ptr,*next;
  ptr=list->root;
  while(ptr){
    next=ptr->next;
    SAFE_FREE(ptr);
    ptr=next;
  }
  SAFE_FREE(list);
}

struct ssh_iterator *ssh_list_get_iterator(const struct ssh_list *list){
  return list->root;
}

static struct ssh_iterator *ssh_iterator_new(const void *data){
  struct ssh_iterator *iterator=malloc(sizeof(struct ssh_iterator));
  if(!iterator)
    return NULL;
  iterator->next=NULL;
  iterator->data=data;
  return iterator;
}

int ssh_list_add(struct ssh_list *list,const void *data){
  struct ssh_iterator *iterator=ssh_iterator_new(data);
  if(!iterator)
    return SSH_ERROR;
  if(!list->end){
    /* list is empty */
    list->root=list->end=iterator;
  } else {
    /* put it on end of list */
    list->end->next=iterator;
    list->end=iterator;
  }
  return SSH_OK;
}

void ssh_list_remove(struct ssh_list *list, struct ssh_iterator *iterator){
  struct ssh_iterator *ptr,*prev;
  prev=NULL;
  ptr=list->root;
  while(ptr && ptr != iterator){
    prev=ptr;
    ptr=ptr->next;
  }
  if(!ptr){
    /* we did not find the element */
    return;
  }
  /* unlink it */
  if(prev)
    prev->next=ptr->next;
  /* if iterator was the head */
  if(list->root == iterator)
    list->root=iterator->next;
  /* if iterator was the tail */
  if(list->end == iterator)
    list->end = prev;
  SAFE_FREE(iterator);
}

const void *_ssh_list_get_head(struct ssh_list *list){
  struct ssh_iterator *iterator=list->root;
  const void *data;
  if(!list->root)
    return NULL;
  data=iterator->data;
  list->root=iterator->next;
  if(list->end==iterator)
    list->end=NULL;
  SAFE_FREE(iterator);
  return data;
}

/**
 * @brief Parse directory component.
 *
 * dirname breaks a null-terminated pathname string into a directory component.
 * In the usual case, ssh_dirname() returns the string up to, but not including,
 * the final '/'. Trailing '/' characters are  not  counted as part of the
 * pathname. The caller must free the memory.
 *
 * @param path  The path to parse.
 *
 * @return  The dirname of path or NULL if we can't allocate memory. If path
 *          does not contain a slash, c_dirname() returns the string ".".  If
 *          path is the string "/", it returns the string "/". If path is
 *          NULL or an empty string, "." is returned.
 */
char *ssh_dirname (const char *path) {
  char *new = NULL;
  unsigned int len;

  if (path == NULL || *path == '\0') {
    return strdup(".");
  }

  len = strlen(path);

  /* Remove trailing slashes */
  while(len > 0 && path[len - 1] == '/') --len;

  /* We have only slashes */
  if (len == 0) {
    return strdup("/");
  }

  /* goto next slash */
  while(len > 0 && path[len - 1] != '/') --len;

  if (len == 0) {
    return strdup(".");
  } else if (len == 1) {
    return strdup("/");
  }

  /* Remove slashes again */
  while(len > 0 && path[len - 1] == '/') --len;

  new = malloc(len + 1);
  if (new == NULL) {
    return NULL;
  }

  strncpy(new, path, len);
  new[len] = '\0';

  return new;
}

/**
 * @brief basename - parse filename component.
 *
 * basename breaks a null-terminated pathname string into a filename component.
 * ssh_basename() returns the component following the final '/'.  Trailing '/'
 * characters are not counted as part of the pathname.
 *
 * @param path The path to parse.
 *
 * @return  The filename of path or NULL if we can't allocate memory. If path
 *          is a the string "/", basename returns the string "/". If path is
 *          NULL or an empty string, "." is returned.
 */
char *ssh_basename (const char *path) {
  char *new = NULL;
  const char *s;
  unsigned int len;

  if (path == NULL || *path == '\0') {
    return strdup(".");
  }

  len = strlen(path);
  /* Remove trailing slashes */
  while(len > 0 && path[len - 1] == '/') --len;

  /* We have only slashes */
  if (len == 0) {
    return strdup("/");
  }

  while(len > 0 && path[len - 1] != '/') --len;

  if (len > 0) {
    s = path + len;
    len = strlen(s);

    while(len > 0 && s[len - 1] == '/') --len;
  } else {
    return strdup(path);
  }

  new = malloc(len + 1);
  if (new == NULL) {
    return NULL;
  }

  strncpy(new, s, len);
  new[len] = '\0';

  return new;
}

/**
 * @brief Attempts to create a directory with the given pathname.
 *
 * This is the portable version of mkdir, mode is ignored on Windows systems.
 *
 * @param  pathname     The path name to create the directory.
 *
 * @param  mode         The permissions to use.
 *
 * @return 0 on success, < 0 on error with errno set.
 */
int ssh_mkdir(const char *pathname, mode_t mode) {
  int r;

#ifdef _WIN32
  r = _mkdir(pathname);
#else
  r = mkdir(pathname, mode);
#endif

  return r;
}

/** @} */
/* vim: set ts=2 sw=2 et cindent: */