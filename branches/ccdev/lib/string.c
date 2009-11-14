#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "bbs.h"

/**
 * Convert string to lower case.
 * @param dst result string.
 * @param src the string to convert.
 * @return the converted string.
 * @note 'dst' should have enough space to hold 'src'.
 */
char *strtolower(char *dst, const char *src) {
	char *ret = dst;

	if (dst == NULL || src == NULL)
		return NULL;
	while (*src != '\0')
		*dst++ = tolower(*src++);
	*dst = '\0';
	return ret;	
}

/**
 * Convert string to upper case.
 * @param dst result string.
 * @param src the string to convert.
 * @return the converted string.
 * @note 'dst' should have enough space to hold 'src'.
 */
char *strtoupper(char *dst, const char *src) {
	char *ret = dst;

	if (dst == NULL || src == NULL)
		return NULL;
	while (*src != '\0')
		*dst++ = toupper(*src++);
	*dst = '\0';
	return ret;	
}

// Compare string 's1' against 's2' and differences in case are ignored.
// No more than 'n' characters are compared.
// This function supports zh_CN.GBK.
int strncasecmp_gbk(const char *s1, const char *s2, int n) {
	register int c1, c2, l = 0;

	while (*s1 && *s2 && l < n) {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
		if (c1 != c2)
			return (c1 - c2);
		++l;
		if (c1 & 0x80) {
			if(*s1 == *s2)
				++l;
			else
				return (*s1 - *s2);
		}
	}
	if (l==n)
		return 0;
	else
		return -1;
}

// Search string 'haystack' for a substring 'needle'
// and differences in case are ignored.
// This function supports zh_CN.GBK.
char *strcasestr_gbk(const char *haystack, const char *needle) {
	int i, nlength, hlength;

	if (haystack == NULL || needle == NULL)
		return NULL;
	nlength = strlen(needle);
	hlength = strlen(haystack);
	if (nlength > hlength)
		return NULL;
	if (hlength <= 0)
		return NULL;
	if (nlength <= 0)
		return (char *)haystack;
	for (i = 0; i <= (hlength - nlength); i++) {
		if (strncasecmp_gbk(haystack + i, needle, nlength) == 0)
			return (char *)(haystack + i);
		if (haystack[i] & 0x80)
			i++;
	}
	return NULL;
}

// Eliminate ANSI escape codes from 'src' and store it in 'dst'.
// 'src' and 'dst' can be the same.
char *ansi_filter(char *dst, const char *src)
{
	char *ret = dst;
	int flag = 0;

	if (dst == NULL || src == NULL)
		return NULL;
	for (; *src != '\0'; src++) {
		if (*src == '\033')
			flag = 1;
		else if (flag == 0)
			*dst++ = *src;
		else if (isalpha(*src))
			flag = 0;
	}
	*dst = '\0';
	return ret;
}

// Truncates 'str' to 'len' chars ended with ".."  or "...".
// Do nothing if 'str' is less than or equal to 'len' chars.
int ellipsis(char *str, int len)
{
	int i = 0, inGBK = 0;
	char *ptr = str;
	if (len < 0 || str == NULL)
		return 0;
	if (len < 3) {
		str[len] = '\0';
		return 1;
	}
	i = len - 3;
	while (*ptr != '\0' && i) {
		if (inGBK) {
			inGBK = 0;
		}
		else if (*ptr & 0x80)
			inGBK = 1;
		++ptr;
		--i;
	}
	i = 3;
	while(*ptr++ != '\0' && --i)
		;
	if(*ptr != '\0' && !i){
		str[len] = '\0';
		*--ptr = '.';
		*--ptr = '.';
		if(!inGBK && *--ptr & 0x80)
			*ptr = '.';
	}
	return 1;
}

// Removes trailing chars whose ASCII code is less than 0x20.
char *rtrim(char *str){
	if (str == NULL)
		return NULL;
	size_t len = strlen(str);
	unsigned char *ustr = (unsigned char *)str;
	unsigned char *ptr = ustr + len;
	while (*ptr <= 0x20 && ptr >= ustr) {
		--ptr;
	}
	*++ptr = '\0';
	return str;
}

// Removes both leading and trailing chars
// whose ASCII code is less than 0x20.
char *trim(char *str)
{
	if (str == NULL)
		return NULL;
	size_t len = strlen(str);
	unsigned char *ustr = (unsigned char *)str;
	unsigned char *right = ustr + len;
	while (*right <= 0x20 && right >= ustr) {
		--right;
	}
	*++right = '\0';
	unsigned char *left = ustr;
	while (*left <= 0x20 && *left != '\0')
		++left;
	if (left != ustr)
		memmove(ustr, left, right - left + 1);
	return str;
}

// OpenBSD: strlcpy.c,v 1.11
// Copy 'src' to string 'dst' of size 'siz'.
// At most siz-1 characters will be copied.
// Always NUL terminates (unless siz == 0).
// Returns strlen(src); if retval >= siz, truncation occurred.
size_t strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	// Copy as many bytes as will fit
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	// Not enough room in dst, add NUL and traverse rest of src
	if (n == 0) {
		if (siz != 0)
			*d = '\0';  // NUL-terminate dst
		while (*s++)
			;
	}

	return(s - src - 1);  //count does not include NUL
}


void strtourl(char *url, const char *str)
{
	char c, h;

	while ((c = *str) != '\0') {
		if (isprint(c) && c != ' ' && c!= '%') {
			*url++ = c;
		} else {
			*url++ = '%';
			// hexadecimal representation
			h = c / 16;
			*url++ = h > 9 ? 'A' - 10 + h : '0' + h;
			h = c % 16;
			*url++ = h > 9 ? 'A' - 10 + h : '0' + h;
		}
		++str;
	}
	*url = '\0';
}

//大写转小写
unsigned char upper2lower_map[]={
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
};

//字符串HASH函数，用于用户信息的快速查找
//HASH_FA、HASH_FB、HASH_SIZE在bbs.h中定义
unsigned int strhash(char *str)
{
    unsigned int hash = 0;
    while (*str)
    {
        hash += hash * HASH_FA + upper2lower_map[(unsigned char)(*str++)];
        hash *= HASH_FB;
    }
    hash &= 0x7FFFFFFF;
    return hash;
}

