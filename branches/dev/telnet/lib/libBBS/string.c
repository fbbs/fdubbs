/*
 * string.c -- there's some useful function about string
 *
 * of SEEDNetBBS generation 1 (libtool implement)
 *
 * Copyright (c) 1998, 1999, Edward Ping-Da Chuang <edwardc@edwardc.dhs.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * CVS: $Id: string.c 2 2005-07-14 15:06:08Z root $
 */

#ifdef BBS
#include "bbs.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h> /* for time_t prototype */
#endif

char string_c[] = "$Id: string.c 2 2005-07-14 15:06:08Z root $";

// ½«srcÖÐµÄ×Ö·û´®×ª»»³ÉÐ¡Ð´²¢´æ·ÅÔÚdstÖÐ
// ****   ÓÉµ÷ÓÃÕßÈ·±£dstËùÄÜ´æ´¢µÄÈÝÁ¿²»ÉÙÓÚsrcµÄ×Ö·û¸öÊý
char *strtolower(char *dst, char *src)
{
    char *ret = dst;
    if (dst == NULL || src == NULL)
        return NULL;
    while (*src)
        *dst++ = tolower(*src++);
    *dst = '\0';
    return ret;
}

char *strtoupper(char *dst, char *src)
{
    char *ret = dst;
    if (dst == NULL || src == NULL)
        return NULL;
    while (*src)
        *dst++ = toupper(*src++);
    *dst = '\0';
    return ret;
}

void my_ansi_filter(char *source)
{
    char result[500];
    int  i, flag = 0, loc=0;

    for ( i = 0 ; i < strlen(source) ; i++ ) {
        if ( source[i] == '' ) {
            flag = 1;
            continue;
        } else if ( flag == 1 && isalpha(source[i]) ) {
            flag = 0;
            continue;
        } else if ( flag == 1 ) {
            continue;
        } else {
            result[loc++]=source[i];
        }
    }
    result[loc]='\0';
    strncpy(source, result,loc+1);
}

char *ansi_filter(char *source)
{
    char *result, ch[3];
    int  i, flag = 0, slen = strlen(source);

    result = (char *)malloc((slen+10)*sizeof(char));

    for ( i = 0 ; i < slen ; i++ ) {
        if ( source[i] == '' ) {
            flag = 1;
            continue;
        } else if ( flag == 1 && isalpha(source[i]) ) {
            flag = 0;
            continue;
        } else if ( flag == 1 ) {
            continue;
        } else {
            sprintf(ch,"%c", source[i]);
            strcat(result, ch);
        }
    }

    return (char *)result;
}

// ½«Ò»¸öÕûÊýÊ±¼äÖµÂÖ»»³É ÄêÔÂÈÕÊ±·ÖÃëÖÜÈÕ¸ñÊ½,²¢·µ»Ø
char *Cdate(time_t *clock)
{
    static char foo[23];
    struct tm *mytm = localtime(clock);

    strftime(foo, 23, "%D %T %a", mytm);
    return (foo);
}

