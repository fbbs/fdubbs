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

//½«srcÖÐµÄ×Ö·û´®×ª»»³ÉÐ¡Ð´²¢´æ·ÅÔÚdestÖÐ
char *strtolower(char *dest, char *src)
{
    char *ret = dest;
    if (dest == NULL || src == NULL)
        return NULL;
    while (*src)
        *dest++ = tolower(*src++);
    *dest = '\0';
    return ret;
}

//½«srcÖÐµÄ×Ö·û×ª»»³É´óÐ´²¢´æ·ÅÔÚdestÖÐ
char *strtoupper(char *dest, char *src)
{
    char *ret = dest;
    if (dest == NULL || src == NULL)
        return NULL;
    while (*src)
        *dest++ = toupper(*src++);
    *dest = '\0';
    return ret;
}

//Ö§³ÖÖÐÎÄµÄstrcasestr£¬²ÎÊýÃûÆðµÃºÜ´´Òâ°¡
char *strcasestr_zh(char *haystack, char *needle)
{
    int i;
    int nlength;
    int hlength;

    if (haystack == NULL || needle == NULL)
        return NULL;

    nlength = strlen(needle);
    hlength = strlen(haystack);

    if (nlength > hlength)
        return NULL;
    if (hlength <= 0)
        return NULL;
    if (nlength <= 0)
        return haystack;

    for (i = 0; i <= (hlength - nlength); i++)
    {
        if (strncasecmp(haystack+i, needle, nlength) == 0)
            return haystack+i;
        if (haystack[i] & 0x80)
            i++;
    }
    return NULL;
}

//½«srcÖÐµÄÄÚÈÝ¹ýÂËANSI¸´ÖÆµ½destÖÐ£¬srcºÍdest¿ÉÒÔÎªÍ¬Ò»ÇøÓò
char *ansi_filter(char *dest, char *src)
{
    char *ret = dest;
    int flag = 0;
    if (dest == NULL || src == NULL)
        return NULL;
    for (; *src != '\0'; src++)
    {
        if (*src == '')
            flag = 1;
        else if (flag == 0)
            *dest++ = *src;
        else if (isalpha(*src))
            flag = 0;
    }
    *dest = '\0';
    return ret;
}

char datestring[30]; //Îª¼æÈÝÒÔÇ°µÄ´úÂë£¬ÏÈÓÃ×ÅÕâ¸ö£¬ÒÔºóÔÙÖð²½È¥µô

//Ê±¼ä×ª»» mode: 0-ÖÐÎÄ 1-Ó¢ÎÄ 2-Ó¢ÎÄ¶Ì¸ñÊ½
int getdatestring(time_t time, int mode)
{
    struct tm tm;
    char weeknum[7][3] = {"Ìì", "Ò»", "¶þ", "Èý", "ËÄ", "Îå", "Áù"};
    char engweek[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    localtime_r(&time, &tm);
    switch (mode)
    {
        case 0:
            sprintf(datestring, "%4dÄê%02dÔÂ%02dÈÕ%02d:%02d:%02d ÐÇÆÚ%2s",
                    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                    tm.tm_hour, tm.tm_min, tm.tm_sec, weeknum[tm.tm_wday]);
            break;
        case 1:
            sprintf(datestring, "%02d/%02d/%02d %02d:%02d:%02d",
                    tm.tm_mon + 1, tm.tm_mday, tm.tm_year - 100,
                    tm.tm_hour, tm.tm_min, tm.tm_sec);
            break;
        case 2:
            sprintf(datestring, "%02d.%02d %02d:%02d",
                    tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
            break;
        default:
            sprintf(datestring, "%02d/%02d/%02d %02d:%02d:%02d %3s",
                    tm.tm_mon + 1, tm.tm_mday, tm.tm_year - 100,
                    tm.tm_hour, tm.tm_min, tm.tm_sec, engweek[tm.tm_wday]);
            break;
    }
    return (tm.tm_sec % 10);
}

