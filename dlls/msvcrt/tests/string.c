/*
 * Unit test suite for string functions.
 *
 * Copyright 2004 Uwe Bonnes
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "wine/test.h"
#include "winbase.h"
#include <string.h>
#include <mbstring.h>
#include <stdlib.h>
#include <stdio.h>
#include <mbctype.h>
#include <locale.h>
#include <errno.h>
#include <limits.h>

static char *buf_to_string(const unsigned char *bin, int len, int nr)
{
    static char buf[2][1024];
    char *w = buf[nr];
    int i;

    for (i = 0; i < len; i++)
    {
        sprintf(w, "%02x ", (unsigned char)bin[i]);
        w += strlen(w);
    }
    return buf[nr];
}

#define expect_eq(expr, value, type, format) { type ret = (expr); ok((value) == ret, #expr " expected " format " got " format "\n", value, ret); }
#define expect_bin(buf, value, len) { ok(memcmp((buf), value, len) == 0, "Binary buffer mismatch - expected %s, got %s\n", buf_to_string((unsigned char *)value, len, 1), buf_to_string((buf), len, 0)); }

static void* (__cdecl *pmemcpy)(void *, const void *, size_t n);
static int* (__cdecl *pmemcmp)(void *, const void *, size_t n);
static int (__cdecl *pstrcpy_s)(char *dst, size_t len, const char *src);
static int (__cdecl *pstrcat_s)(char *dst, size_t len, const char *src);
static int (__cdecl *p_mbsnbcpy_s)(unsigned char * dst, size_t size, const unsigned char * src, size_t count);
static int (__cdecl *p_wcscpy_s)(wchar_t *wcDest, size_t size, const wchar_t *wcSrc);
static int (__cdecl *p_wcsupr_s)(wchar_t *str, size_t size);
static size_t (__cdecl *p_strnlen)(const char *, size_t);
static __int64 (__cdecl *p_strtoi64)(const char *, char **, int);
static unsigned __int64 (__cdecl *p_strtoui64)(const char *, char **, int);
static int *p__mb_cur_max;
static unsigned char *p_mbctype;

#define SETNOFAIL(x,y) x = (void*)GetProcAddress(hMsvcrt,y)
#define SET(x,y) SETNOFAIL(x,y); ok(x != NULL, "Export '%s' not found\n", y)

HMODULE hMsvcrt;

static void test_swab( void ) {
    char original[]  = "BADCFEHGJILKNMPORQTSVUXWZY@#";
    char expected1[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ@#";
    char expected2[] = "ABCDEFGHIJKLMNOPQRSTUVWX$";
    char expected3[] = "$";
    
    char from[30];
    char to[30];
    
    int testsize;
    
    /* Test 1 - normal even case */                               
    memset(to,'$', sizeof(to));
    memset(from,'@', sizeof(from));
    testsize = 26;
    memcpy(from, original, testsize);
    _swab( from, to, testsize );
    ok(memcmp(to,expected1,testsize) == 0, "Testing even size %d returned '%*.*s'\n", testsize, testsize, testsize, to);

    /* Test 2 - uneven case  */                               
    memset(to,'$', sizeof(to));
    memset(from,'@', sizeof(from));
    testsize = 25;
    memcpy(from, original, testsize);
    _swab( from, to, testsize );
    ok(memcmp(to,expected2,testsize) == 0, "Testing odd size %d returned '%*.*s'\n", testsize, testsize, testsize, to);

    /* Test 3 - from = to */                               
    memset(to,'$', sizeof(to));
    memset(from,'@', sizeof(from));
    testsize = 26;
    memcpy(to, original, testsize);
    _swab( to, to, testsize );
    ok(memcmp(to,expected1,testsize) == 0, "Testing overlapped size %d returned '%*.*s'\n", testsize, testsize, testsize, to);

    /* Test 4 - 1 bytes */                               
    memset(to,'$', sizeof(to));
    memset(from,'@', sizeof(from));
    testsize = 1;
    memcpy(from, original, testsize);
    _swab( from, to, testsize );
    ok(memcmp(to,expected3,testsize) == 0, "Testing small size %d returned '%*.*s'\n", testsize, testsize, testsize, to);
}

#if 0      /* use this to generate more tests */

static void test_codepage(int cp)
{
    int i;
    int prev;
    int count = 1;

    ok(_setmbcp(cp) == 0, "Couldn't set mbcp\n");

    prev = p_mbctype[0];
    printf("static int result_cp_%d_mbctype[] = { ", cp);
    for (i = 1; i < 257; i++)
    {
        if (p_mbctype[i] != prev)
        {
            printf("0x%x,%d, ", prev, count);
            prev = p_mbctype[i];
            count = 1;
        }
        else
            count++;
    }
    printf("0x%x,%d };\n", prev, count);
}

#else

/* RLE-encoded mbctype tables for given codepages */
static int result_cp_932_mbctype[] = { 0x0,65, 0x8,1, 0x18,26, 0x8,6, 0x28,26, 0x8,4,
  0x0,1, 0x8,1, 0xc,31, 0x8,1, 0xa,5, 0x9,58, 0xc,29, 0,3 };
static int result_cp_936_mbctype[] = { 0x0,65, 0x8,1, 0x18,26, 0x8,6, 0x28,26, 0x8,6,
  0xc,126, 0,1 };
static int result_cp_949_mbctype[] = { 0x0,66, 0x18,26, 0x8,6, 0x28,26, 0x8,6, 0xc,126,
  0,1 };
static int result_cp_950_mbctype[] = { 0x0,65, 0x8,1, 0x18,26, 0x8,6, 0x28,26, 0x8,4,
  0x0,2, 0x4,32, 0xc,94, 0,1 };

static void test_cp_table(int cp, int *result)
{
    int i;
    int count = 0;
    int curr = 0;
    _setmbcp(cp);
    for (i = 0; i < 256; i++)
    {
        if (count == 0)
        {
            curr = result[0];
            count = result[1];
            result += 2;
        }
        ok(p_mbctype[i] == curr, "CP%d: Mismatch in ctype for character %d - %d instead of %d\n", cp, i-1, p_mbctype[i], curr);
        count--;
    }
}

#define test_codepage(num) test_cp_table(num, result_cp_##num##_mbctype);

#endif

static void test_mbcp(void)
{
    int mb_orig_max = *p__mb_cur_max;
    int curr_mbcp = _getmbcp();
    unsigned char *mbstring = (unsigned char *)"\xb0\xb1\xb2 \xb3\xb4 \xb5"; /* incorrect string */
    unsigned char *mbstring2 = (unsigned char *)"\xb0\xb1\xb2\xb3Q\xb4\xb5"; /* correct string */
    unsigned char *mbsonlylead = (unsigned char *)"\xb0\0\xb1\xb2 \xb3";
    unsigned char buf[16];
    int step;

    /* _mbtype tests */

    /* An SBCS codepage test. The ctype of characters on e.g. CP1252 or CP1250 differs slightly
     * between versions of Windows. Also Windows 9x seems to ignore the codepage and always uses
     * CP1252 (or the ACP?) so we test only a few ASCII characters */
    _setmbcp(1252);
    expect_eq(p_mbctype[10], 0, char, "%x");
    expect_eq(p_mbctype[50], 0, char, "%x");
    expect_eq(p_mbctype[66], _SBUP, char, "%x");
    expect_eq(p_mbctype[100], _SBLOW, char, "%x");
    expect_eq(p_mbctype[128], 0, char, "%x");
    _setmbcp(1250);
    expect_eq(p_mbctype[10], 0, char, "%x");
    expect_eq(p_mbctype[50], 0, char, "%x");
    expect_eq(p_mbctype[66], _SBUP, char, "%x");
    expect_eq(p_mbctype[100], _SBLOW, char, "%x");
    expect_eq(p_mbctype[128], 0, char, "%x");

    /* double byte code pages */
    test_codepage(932);
    test_codepage(936);
    test_codepage(949);
    test_codepage(950);

    _setmbcp(936);
    ok(*p__mb_cur_max == mb_orig_max, "__mb_cur_max shouldn't be updated (is %d != %d)\n", *p__mb_cur_max, mb_orig_max);
    ok(_ismbblead('\354'), "\354 should be a lead byte\n");
    ok(_ismbblead(' ') == FALSE, "' ' should not be a lead byte\n");
    ok(_ismbblead(0x1234b0), "0x1234b0 should not be a lead byte\n");
    ok(_ismbblead(0x123420) == FALSE, "0x123420 should not be a lead byte\n");
    ok(_ismbbtrail('\xb0'), "\xa0 should be a trail byte\n");
    ok(_ismbbtrail(' ') == FALSE, "' ' should not be a trail byte\n");

    /* _ismbslead */
    expect_eq(_ismbslead(mbstring, &mbstring[0]), -1, int, "%d");
    expect_eq(_ismbslead(mbstring, &mbstring[1]), FALSE, int, "%d");
    expect_eq(_ismbslead(mbstring, &mbstring[2]), -1, int, "%d");
    expect_eq(_ismbslead(mbstring, &mbstring[3]), FALSE, int, "%d");
    expect_eq(_ismbslead(mbstring, &mbstring[4]), -1, int, "%d");
    expect_eq(_ismbslead(mbstring, &mbstring[5]), FALSE, int, "%d");
    expect_eq(_ismbslead(mbstring, &mbstring[6]), FALSE, int, "%d");
    expect_eq(_ismbslead(mbstring, &mbstring[7]), -1, int, "%d");
    expect_eq(_ismbslead(mbstring, &mbstring[8]), FALSE, int, "%d");

    expect_eq(_ismbslead(mbsonlylead, &mbsonlylead[0]), -1, int, "%d");
    expect_eq(_ismbslead(mbsonlylead, &mbsonlylead[1]), FALSE, int, "%d");
    expect_eq(_ismbslead(mbsonlylead, &mbsonlylead[2]), FALSE, int, "%d");
    expect_eq(_ismbslead(mbsonlylead, &mbsonlylead[5]), FALSE, int, "%d");

    /* _ismbstrail */
    expect_eq(_ismbstrail(mbstring, &mbstring[0]), FALSE, int, "%d");
    expect_eq(_ismbstrail(mbstring, &mbstring[1]), -1, int, "%d");
    expect_eq(_ismbstrail(mbstring, &mbstring[2]), FALSE, int, "%d");
    expect_eq(_ismbstrail(mbstring, &mbstring[3]), -1, int, "%d");
    expect_eq(_ismbstrail(mbstring, &mbstring[4]), FALSE, int, "%d");
    expect_eq(_ismbstrail(mbstring, &mbstring[5]), -1, int, "%d");
    expect_eq(_ismbstrail(mbstring, &mbstring[6]), FALSE, int, "%d");
    expect_eq(_ismbstrail(mbstring, &mbstring[7]), FALSE, int, "%d");
    expect_eq(_ismbstrail(mbstring, &mbstring[8]), -1, int, "%d");

    expect_eq(_ismbstrail(mbsonlylead, &mbsonlylead[0]), FALSE, int, "%d");
    expect_eq(_ismbstrail(mbsonlylead, &mbsonlylead[1]), -1, int, "%d");
    expect_eq(_ismbstrail(mbsonlylead, &mbsonlylead[2]), FALSE, int, "%d");
    expect_eq(_ismbstrail(mbsonlylead, &mbsonlylead[3]), FALSE, int, "%d");
    expect_eq(_ismbstrail(mbsonlylead, &mbsonlylead[4]), FALSE, int, "%d");
    expect_eq(_ismbstrail(mbsonlylead, &mbsonlylead[5]), FALSE, int, "%d");

    /* _mbsbtype */
    expect_eq(_mbsbtype(mbstring, 0), _MBC_LEAD, int, "%d");
    expect_eq(_mbsbtype(mbstring, 1), _MBC_TRAIL, int, "%d");
    expect_eq(_mbsbtype(mbstring, 2), _MBC_LEAD, int, "%d");
    expect_eq(_mbsbtype(mbstring, 3), _MBC_ILLEGAL, int, "%d");
    expect_eq(_mbsbtype(mbstring, 4), _MBC_LEAD, int, "%d");
    expect_eq(_mbsbtype(mbstring, 5), _MBC_TRAIL, int, "%d");
    expect_eq(_mbsbtype(mbstring, 6), _MBC_SINGLE, int, "%d");
    expect_eq(_mbsbtype(mbstring, 7), _MBC_LEAD, int, "%d");
    expect_eq(_mbsbtype(mbstring, 8), _MBC_ILLEGAL, int, "%d");

    expect_eq(_mbsbtype(mbsonlylead, 0), _MBC_LEAD, int, "%d");
    expect_eq(_mbsbtype(mbsonlylead, 1), _MBC_ILLEGAL, int, "%d");
    expect_eq(_mbsbtype(mbsonlylead, 2), _MBC_ILLEGAL, int, "%d");
    expect_eq(_mbsbtype(mbsonlylead, 3), _MBC_ILLEGAL, int, "%d");
    expect_eq(_mbsbtype(mbsonlylead, 4), _MBC_ILLEGAL, int, "%d");
    expect_eq(_mbsbtype(mbsonlylead, 5), _MBC_ILLEGAL, int, "%d");

    /* _mbsnextc */
    expect_eq(_mbsnextc(mbstring), 0xb0b1, int, "%x");
    expect_eq(_mbsnextc(&mbstring[2]), 0xb220, int, "%x");  /* lead + invalid tail */
    expect_eq(_mbsnextc(&mbstring[3]), 0x20, int, "%x");    /* single char */

    /* _mbclen/_mbslen */
    expect_eq(_mbclen(mbstring), 2, int, "%d");
    expect_eq(_mbclen(&mbstring[2]), 2, int, "%d");
    expect_eq(_mbclen(&mbstring[3]), 1, int, "%d");
    expect_eq(_mbslen(mbstring2), 4, int, "%d");
    expect_eq(_mbslen(mbsonlylead), 0, int, "%d");          /* lead + NUL not counted as character */
    expect_eq(_mbslen(mbstring), 4, int, "%d");             /* lead + invalid trail counted */

    /* _mbccpy/_mbsncpy */
    memset(buf, 0xff, sizeof(buf));
    _mbccpy(buf, mbstring);
    expect_bin(buf, "\xb0\xb1\xff", 3);

    memset(buf, 0xff, sizeof(buf));
    _mbsncpy(buf, mbstring, 1);
    expect_bin(buf, "\xb0\xb1\xff", 3);
    memset(buf, 0xff, sizeof(buf));
    _mbsncpy(buf, mbstring, 2);
    expect_bin(buf, "\xb0\xb1\xb2 \xff", 5);
    memset(buf, 0xff, sizeof(buf));
    _mbsncpy(buf, mbstring, 3);
    expect_bin(buf, "\xb0\xb1\xb2 \xb3\xb4\xff", 7);
    memset(buf, 0xff, sizeof(buf));
    _mbsncpy(buf, mbstring, 4);
    expect_bin(buf, "\xb0\xb1\xb2 \xb3\xb4 \xff", 8);
    memset(buf, 0xff, sizeof(buf));
    _mbsncpy(buf, mbstring, 5);
    expect_bin(buf, "\xb0\xb1\xb2 \xb3\xb4 \0\0\xff", 10);
    memset(buf, 0xff, sizeof(buf));
    _mbsncpy(buf, mbsonlylead, 6);
    expect_bin(buf, "\0\0\0\0\0\0\0\xff", 8);

    memset(buf, 0xff, sizeof(buf));
    _mbsnbcpy(buf, mbstring2, 2);
    expect_bin(buf, "\xb0\xb1\xff", 3);
    _mbsnbcpy(buf, mbstring2, 3);
    expect_bin(buf, "\xb0\xb1\0\xff", 4);
    _mbsnbcpy(buf, mbstring2, 4);
    expect_bin(buf, "\xb0\xb1\xb2\xb3\xff", 5);
    memset(buf, 0xff, sizeof(buf));
    _mbsnbcpy(buf, mbsonlylead, 5);
    expect_bin(buf, "\0\0\0\0\0\xff", 6);

    /* _mbsinc/mbsdec */
    step = _mbsinc(mbstring) - mbstring;
    ok(step == 2, "_mbsinc adds %d (exp. 2)\n", step);
    step = _mbsinc(&mbstring[2]) - &mbstring[2];  /* lead + invalid tail */
    ok(step == 2, "_mbsinc adds %d (exp. 2)\n", step);

    step = _mbsninc(mbsonlylead, 1) - mbsonlylead;
    ok(step == 0, "_mbsninc adds %d (exp. 0)\n", step);
    step = _mbsninc(mbsonlylead, 2) - mbsonlylead;  /* lead + NUL byte + lead + char */
    ok(step == 0, "_mbsninc adds %d (exp. 0)\n", step);
    step = _mbsninc(mbstring2, 0) - mbstring2;
    ok(step == 0, "_mbsninc adds %d (exp. 2)\n", step);
    step = _mbsninc(mbstring2, 1) - mbstring2;
    ok(step == 2, "_mbsninc adds %d (exp. 2)\n", step);
    step = _mbsninc(mbstring2, 2) - mbstring2;
    ok(step == 4, "_mbsninc adds %d (exp. 4)\n", step);
    step = _mbsninc(mbstring2, 3) - mbstring2;
    ok(step == 5, "_mbsninc adds %d (exp. 5)\n", step);
    step = _mbsninc(mbstring2, 4) - mbstring2;
    ok(step == 7, "_mbsninc adds %d (exp. 7)\n", step);
    step = _mbsninc(mbstring2, 5) - mbstring2;
    ok(step == 7, "_mbsninc adds %d (exp. 7)\n", step);
    step = _mbsninc(mbstring2, 17) - mbstring2;
    ok(step == 7, "_mbsninc adds %d (exp. 7)\n", step);

    /* functions that depend on locale codepage, not mbcp.
     * we hope the current locale to be SBCS because setlocale(LC_ALL, ".1252") seems not to work yet
     * (as of Wine 0.9.43)
     */
    if (*p__mb_cur_max == 1)
    {
        expect_eq(mblen((char *)mbstring, 3), 1, int, "%x");
        expect_eq(_mbstrlen((char *)mbstring2), 7, int, "%d");
    }
    else
        skip("Current locale has double-byte charset - could leave to false positives\n");

    _setmbcp(1361);
    expect_eq(_ismbblead(0x80), 0, int, "%d");
    todo_wine {
      expect_eq(_ismbblead(0x81), 1, int, "%d");
      expect_eq(_ismbblead(0x83), 1, int, "%d");
    }
    expect_eq(_ismbblead(0x84), 1, int, "%d");
    expect_eq(_ismbblead(0xd3), 1, int, "%d");
    expect_eq(_ismbblead(0xd7), 0, int, "%d");
    todo_wine {
      expect_eq(_ismbblead(0xd8), 1, int, "%d");
    }
    expect_eq(_ismbblead(0xd9), 1, int, "%d");

    expect_eq(_ismbbtrail(0x30), 0, int, "%d");
    expect_eq(_ismbbtrail(0x31), 1, int, "%d");
    expect_eq(_ismbbtrail(0x7e), 1, int, "%d");
    expect_eq(_ismbbtrail(0x7f), 0, int, "%d");
    expect_eq(_ismbbtrail(0x80), 0, int, "%d");
    expect_eq(_ismbbtrail(0x81), 1, int, "%d");
    expect_eq(_ismbbtrail(0xfe), 1, int, "%d");
    expect_eq(_ismbbtrail(0xff), 0, int, "%d");

    _setmbcp(curr_mbcp);
}

static void test_mbsspn( void)
{
    unsigned char str1[]="cabernet";
    unsigned char str2[]="shiraz";
    unsigned char set[]="abc";
    unsigned char empty[]="";
    int ret;
    ret=_mbsspn( str1, set);
    ok( ret==3, "_mbsspn returns %d should be 3\n", ret);
    ret=_mbsspn( str2, set);
    ok( ret==0, "_mbsspn returns %d should be 0\n", ret);
    ret=_mbsspn( str1, empty);
    ok( ret==0, "_mbsspn returns %d should be 0\n", ret);
}

static void test_mbsspnp( void)
{
    unsigned char str1[]="cabernet";
    unsigned char str2[]="shiraz";
    unsigned char set[]="abc";
    unsigned char empty[]="";
    unsigned char full[]="abcenrt";
    unsigned char* ret;
    ret=_mbsspnp( str1, set);
    ok( ret[0]=='e', "_mbsspnp returns %c should be e\n", ret[0]);
    ret=_mbsspnp( str2, set);
    ok( ret[0]=='s', "_mbsspnp returns %c should be s\n", ret[0]);
    ret=_mbsspnp( str1, empty);
    ok( ret[0]=='c', "_mbsspnp returns %c should be c\n", ret[0]);
    ret=_mbsspnp( str1, full);
    ok( ret==NULL, "_mbsspnp returns %p should be NULL\n", ret);
}

static void test_strdup(void)
{
   char *str;
   str = _strdup( 0 );
   ok( str == 0, "strdup returns %s should be 0\n", str);
   free( str );
}

static void test_strcpy_s(void)
{
    char dest[8];
    const char *small = "small";
    const char *big = "atoolongstringforthislittledestination";
    int ret;

    if(!pstrcpy_s)
    {
        skip("strcpy_s not found\n");
        return;
    }

    memset(dest, 'X', sizeof(dest));
    ret = pstrcpy_s(dest, sizeof(dest), small);
    ok(ret == 0, "Copying a string into a big enough destination returned %d, expected 0\n", ret);
    ok(dest[0] == 's' && dest[1] == 'm' && dest[2] == 'a' && dest[3] == 'l' &&
       dest[4] == 'l' && dest[5] == '\0'&& dest[6] == 'X' && dest[7] == 'X',
       "Unexpected return data from strcpy_s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
       dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6], dest[7]);

    memset(dest, 'X', sizeof(dest));
    ret = pstrcpy_s(dest, 0, big);
    ok(ret == EINVAL, "Copying into a destination of size 0 returned %d, expected EINVAL\n", ret);
    ok(dest[0] == 'X' && dest[1] == 'X' && dest[2] == 'X' && dest[3] == 'X' &&
       dest[4] == 'X' && dest[5] == 'X' && dest[6] == 'X' && dest[7] == 'X',
       "Unexpected return data from strcpy_s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
       dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6], dest[7]);
    ret = pstrcpy_s(dest, 0, NULL);
    ok(ret == EINVAL, "Copying into a destination of size 0 returned %d, expected EINVAL\n", ret);
    ok(dest[0] == 'X' && dest[1] == 'X' && dest[2] == 'X' && dest[3] == 'X' &&
       dest[4] == 'X' && dest[5] == 'X' && dest[6] == 'X' && dest[7] == 'X',
       "Unexpected return data from strcpy_s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
       dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6], dest[7]);

    memset(dest, 'X', sizeof(dest));
    ret = pstrcpy_s(dest, sizeof(dest), big);
    ok(ret == ERANGE, "Copying a big string in a small location returned %d, expected ERANGE\n", ret);
    ok(dest[0] == '\0'&& dest[1] == 't' && dest[2] == 'o' && dest[3] == 'o' &&
       dest[4] == 'l' && dest[5] == 'o' && dest[6] == 'n' && dest[7] == 'g',
       "Unexpected return data from strcpy_s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
       dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6], dest[7]);

    memset(dest, 'X', sizeof(dest));
    ret = pstrcpy_s(dest, sizeof(dest), NULL);
    ok(ret == EINVAL, "Copying from a NULL source string returned %d, expected EINVAL\n", ret);
    ok(dest[0] == '\0'&& dest[1] == 'X' && dest[2] == 'X' && dest[3] == 'X' &&
       dest[4] == 'X' && dest[5] == 'X' && dest[6] == 'X' && dest[7] == 'X',
       "Unexpected return data from strcpy_s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
       dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6], dest[7]);

    ret = pstrcpy_s(NULL, sizeof(dest), small);
    ok(ret == EINVAL, "Copying a big string a NULL dest returned %d, expected EINVAL\n", ret);
}

static void test_strcat_s(void)
{
    char dest[8];
    const char *small = "sma";
    int ret;

    if(!pstrcat_s)
    {
        skip("strcat_s not found\n");
        return;
    }

    memset(dest, 'X', sizeof(dest));
    dest[0] = '\0';
    ret = pstrcat_s(dest, sizeof(dest), small);
    ok(ret == 0, "strcat_s: Copying a string into a big enough destination returned %d, expected 0\n", ret);
    ok(dest[0] == 's' && dest[1] == 'm' && dest[2] == 'a' && dest[3] == '\0'&&
       dest[4] == 'X' && dest[5] == 'X' && dest[6] == 'X' && dest[7] == 'X',
       "Unexpected return data from strcpy_s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
       dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6], dest[7]);
    ret = pstrcat_s(dest, sizeof(dest), small);
    ok(ret == 0, "strcat_s: Attaching a string to a big enough destination returned %d, expected 0\n", ret);
    ok(dest[0] == 's' && dest[1] == 'm' && dest[2] == 'a' && dest[3] == 's' &&
       dest[4] == 'm' && dest[5] == 'a' && dest[6] == '\0'&& dest[7] == 'X',
       "Unexpected return data from strcpy_s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
       dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6], dest[7]);

    ret = pstrcat_s(dest, sizeof(dest), small);
    ok(ret == ERANGE, "strcat_s: Attaching a string to a filled up destination returned %d, expected ERANGE\n", ret);
    ok(dest[0] == '\0'&& dest[1] == 'm' && dest[2] == 'a' && dest[3] == 's' &&
       dest[4] == 'm' && dest[5] == 'a' && dest[6] == 's' && dest[7] == 'm',
       "Unexpected return data from strcpy_s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
       dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6], dest[7]);

    memset(dest, 'X', sizeof(dest));
    dest[0] = 'a';
    dest[1] = '\0';

    ret = pstrcat_s(dest, 0, small);
    ok(ret == EINVAL, "strcat_s: Source len = 0 returned %d, expected EINVAL\n", ret);
    ok(dest[0] == 'a' && dest[1] == '\0'&& dest[2] == 'X' && dest[3] == 'X' &&
       dest[4] == 'X' && dest[5] == 'X' && dest[6] == 'X' && dest[7] == 'X',
       "Unexpected return data from strcpy_s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
       dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6], dest[7]);

    ret = pstrcat_s(dest, 0, NULL);
    ok(ret == EINVAL, "strcat_s: len = 0 and src = NULL returned %d, expected EINVAL\n", ret);
    ok(dest[0] == 'a' && dest[1] == '\0'&& dest[2] == 'X' && dest[3] == 'X' &&
       dest[4] == 'X' && dest[5] == 'X' && dest[6] == 'X' && dest[7] == 'X',
       "Unexpected return data from strcpy_s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
       dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6], dest[7]);

    ret = pstrcat_s(dest, sizeof(dest), NULL);
    ok(ret == EINVAL, "strcat_s:  Sourcing from NULL returned %d, expected EINVAL\n", ret);
    ok(dest[0] == '\0'&& dest[1] == '\0'&& dest[2] == 'X' && dest[3] == 'X' &&
       dest[4] == 'X' && dest[5] == 'X' && dest[6] == 'X' && dest[7] == 'X',
       "Unexpected return data from strcpy_s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
       dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6], dest[7]);

    ret = pstrcat_s(NULL, sizeof(dest), small);
    ok(ret == EINVAL, "strcat_s: Writing to a NULL string returned %d, expected EINVAL\n", ret);
}

static void test__mbsnbcpy_s(void)
{
    unsigned char dest[8];
    const unsigned char big[] = "atoolongstringforthislittledestination";
    const unsigned char small[] = "small";
    int ret;

    if(!p_mbsnbcpy_s)
    {
        skip("_mbsnbcpy_s not found\n");
        return;
    }

    memset(dest, 'X', sizeof(dest));
    ret = p_mbsnbcpy_s(dest, sizeof(dest), small, sizeof(small));
    ok(ret == 0, "_mbsnbcpy_s: Copying a string into a big enough destination returned %d, expected 0\n", ret);
    ok(dest[0] == 's' && dest[1] == 'm' && dest[2] == 'a' && dest[3] == 'l' &&
       dest[4] == 'l' && dest[5] == '\0'&& dest[6] == 'X' && dest[7] == 'X',
       "Unexpected return data from _mbsnbcpy_s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
       dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6], dest[7]);

    /* WTF? */
    memset(dest, 'X', sizeof(dest));
    ret = p_mbsnbcpy_s(dest, sizeof(dest) - 2, big, sizeof(small));
    ok(ret == ERANGE, "_mbsnbcpy_s: Copying a too long string returned %d, expected ERANGE\n", ret);
    ok(dest[0] == '\0'&& dest[1] == 't' && dest[2] == 'o' && dest[3] == 'o' &&
       dest[4] == 'l' && dest[5] == 'o' && dest[6] == 'X' && dest[7] == 'X',
       "Unexpected return data from _mbsnbcpy_s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
       dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6], dest[7]);

    memset(dest, 'X', sizeof(dest));
    ret = p_mbsnbcpy_s(dest, sizeof(dest) - 2, big, 4);
    ok(ret == 0, "_mbsnbcpy_s: Copying a too long string with a count cap returned %d, expected 0\n", ret);
    ok(dest[0] == 'a' && dest[1] == 't' && dest[2] == 'o' && dest[3] == 'o' &&
       dest[4] == '\0'&& dest[5] == 'X' && dest[6] == 'X' && dest[7] == 'X',
       "Unexpected return data from _mbsnbcpy_s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
       dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6], dest[7]);

    memset(dest, 'X', sizeof(dest));
    ret = p_mbsnbcpy_s(dest, sizeof(dest) - 2, small, sizeof(small) + 10);
    ok(ret == 0, "_mbsnbcpy_s: Copying more data than the source string len returned %d, expected 0\n", ret);
    ok(dest[0] == 's' && dest[1] == 'm' && dest[2] == 'a' && dest[3] == 'l' &&
       dest[4] == 'l' && dest[5] == '\0'&& dest[6] == 'X' && dest[7] == 'X',
       "Unexpected return data from _mbsnbcpy_s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
       dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6], dest[7]);
}

static void test_wcscpy_s(void)
{
    static const WCHAR szLongText[] = { 'T','h','i','s','A','L','o','n','g','s','t','r','i','n','g',0 };
    static WCHAR szDest[18];
    static WCHAR szDestShort[8];
    int ret;

    if(!p_wcscpy_s)
    {
        skip("wcscpy_s not found\n");
        return;
    }

    /* Test NULL Dest */
    ret = p_wcscpy_s(NULL, 18, szLongText);
    ok(ret == EINVAL, "p_wcscpy_s expect EINVAL got %d\n", ret);

    /* Test NULL Source */
    szDest[0] = 'A';
    ret = p_wcscpy_s(szDest, 18, NULL);
    ok(ret == EINVAL, "expected EINVAL got %d\n", ret);
    ok(szDest[0] == 0, "szDest[0] not 0\n");

    /* Test invalid size */
    szDest[0] = 'A';
    ret = p_wcscpy_s(szDest, 0, szLongText);
    /* Later versions changed the return value for this case to EINVAL,
     * and don't modify the result if the dest size is 0.
     */
    ok(ret == ERANGE || ret == EINVAL, "expected ERANGE/EINVAL got %d\n", ret);
    ok(szDest[0] == 0 || ret == EINVAL, "szDest[0] not 0\n");

    /* Copy same buffer size */
    ret = p_wcscpy_s(szDest, 18, szLongText);
    ok(ret == 0, "expected 0 got %d\n", ret);
    ok(lstrcmpW(szDest, szLongText) == 0, "szDest != szLongText\n");

    /* Copy smaller buffer size */
    szDest[0] = 'A';
    ret = p_wcscpy_s(szDestShort, 8, szLongText);
    ok(ret == ERANGE || ret == EINVAL, "expected ERANGE/EINVAL got %d\n", ret);
    ok(szDestShort[0] == 0, "szDestShort[0] not 0\n");
}

static void test__wcsupr_s(void)
{
    static const WCHAR mixedString[] = {'M', 'i', 'X', 'e', 'D', 'l', 'o', 'w',
                                        'e', 'r', 'U', 'P', 'P', 'E', 'R', 0};
    static const WCHAR expectedString[] = {'M', 'I', 'X', 'E', 'D', 'L', 'O',
                                           'W', 'E', 'R', 'U', 'P', 'P', 'E',
                                           'R', 0};
    WCHAR testBuffer[2*sizeof(mixedString)/sizeof(WCHAR)];
    int ret;

    if (!p_wcsupr_s)
    {
        win_skip("_wcsupr_s not found\n");
        return;
    }

    /* Test NULL input string and invalid size. */
    errno = EBADF;
    ret = p_wcsupr_s(NULL, 0);
    ok(ret == EINVAL, "Expected _wcsupr_s to fail with EINVAL, got %d\n", ret);
    ok(errno == EINVAL, "Expected errno to be EINVAL, got %d\n", errno);

    /* Test NULL input string and valid size. */
    errno = EBADF;
    ret = p_wcsupr_s(NULL, sizeof(testBuffer)/sizeof(WCHAR));
    ok(ret == EINVAL, "Expected _wcsupr_s to fail with EINVAL, got %d\n", ret);
    ok(errno == EINVAL, "Expected errno to be EINVAL, got %d\n", errno);

    /* Test empty string with zero size. */
    errno = EBADF;
    testBuffer[0] = '\0';
    ret = p_wcsupr_s(testBuffer, 0);
    ok(ret == EINVAL, "Expected _wcsupr_s to fail with EINVAL, got %d\n", ret);
    ok(errno == EINVAL, "Expected errno to be EINVAL, got %d\n", errno);
    ok(testBuffer[0] == '\0', "Expected the buffer to be unchanged\n");

    /* Test empty string with size of one. */
    testBuffer[0] = '\0';
    ret = p_wcsupr_s(testBuffer, 1);
    ok(ret == 0, "Expected _wcsupr_s to succeed, got %d\n", ret);
    ok(testBuffer[0] == '\0', "Expected the buffer to be unchanged\n");

    /* Test one-byte buffer with zero size. */
    errno = EBADF;
    testBuffer[0] = 'x';
    ret = p_wcsupr_s(testBuffer, 0);
    ok(ret == EINVAL, "Expected _wcsupr_s to fail with EINVAL, got %d\n", ret);
    ok(errno == EINVAL, "Expected errno to be EINVAL, got %d\n", errno);
    ok(testBuffer[0] == '\0', "Expected the first buffer character to be null\n");

    /* Test one-byte buffer with size of one. */
    errno = EBADF;
    testBuffer[0] = 'x';
    ret = p_wcsupr_s(testBuffer, 1);
    ok(ret == EINVAL, "Expected _wcsupr_s to fail with EINVAL, got %d\n", ret);
    ok(errno == EINVAL, "Expected errno to be EINVAL, got %d\n", errno);
    ok(testBuffer[0] == '\0', "Expected the first buffer character to be null\n");

    /* Test invalid size. */
    wcscpy(testBuffer, mixedString);
    errno = EBADF;
    ret = p_wcsupr_s(testBuffer, 0);
    ok(ret == EINVAL, "Expected _wcsupr_s to fail with EINVAL, got %d\n", ret);
    ok(errno == EINVAL, "Expected errno to be EINVAL, got %d\n", errno);
    ok(testBuffer[0] == '\0', "Expected the first buffer character to be null\n");

    /* Test normal string uppercasing. */
    wcscpy(testBuffer, mixedString);
    ret = p_wcsupr_s(testBuffer, sizeof(mixedString)/sizeof(WCHAR));
    ok(ret == 0, "Expected _wcsupr_s to succeed, got %d\n", ret);
    ok(!wcscmp(testBuffer, expectedString), "Expected the string to be fully upper-case\n");

    /* Test uppercasing with a shorter buffer size count. */
    wcscpy(testBuffer, mixedString);
    errno = EBADF;
    ret = p_wcsupr_s(testBuffer, sizeof(mixedString)/sizeof(WCHAR) - 1);
    ok(ret == EINVAL, "Expected _wcsupr_s to fail with EINVAL, got %d\n", ret);
    ok(errno == EINVAL, "Expected errno to be EINVAL, got %d\n", errno);
    ok(testBuffer[0] == '\0', "Expected the first buffer character to be null\n");

    /* Test uppercasing with a longer buffer size count. */
    wcscpy(testBuffer, mixedString);
    ret = p_wcsupr_s(testBuffer, sizeof(testBuffer)/sizeof(WCHAR));
    ok(ret == 0, "Expected _wcsupr_s to succeed, got %d\n", ret);
    ok(!wcscmp(testBuffer, expectedString), "Expected the string to be fully upper-case\n");
}

static void test_mbcjisjms(void)
{
    /* List of value-pairs to test. The test assumes the last pair to be {0, ..} */
    unsigned int jisjms[][2] = { {0x2020, 0}, {0x2021, 0}, {0x2120, 0}, {0x2121, 0x8140},
                                 {0x7f7f, 0}, {0x7f7e, 0}, {0x7e7f, 0}, {0x7e7e, 0xeffc},
                                 {0x2121FFFF, 0}, {0x2223, 0x81a1}, {0x237e, 0x829e}, {0, 0}};
    unsigned int ret, exp, i;

    i = 0;
    do
    {
        ret = _mbcjistojms(jisjms[i][0]);

        if(_getmbcp() == 932)   /* Japanese codepage? */
            exp = jisjms[i][1];
        else
            exp = jisjms[i][0]; /* If not, no conversion */

        ok(ret == exp, "Expected 0x%x, got 0x%x\n", exp, ret);
    } while(jisjms[i++][0] != 0);
}

static void test_mbctombb(void)
{
    static const unsigned int mbcmbb_932[][2] = {
        {0x829e, 0x829e}, {0x829f, 0xa7}, {0x82f1, 0xdd}, {0x82f2, 0x82f2},
        {0x833f, 0x833f}, {0x8340, 0xa7}, {0x837e, 0xd0}, {0x837f, 0x837f},
        {0x8380, 0xd1}, {0x8396, 0xb9}, {0x8397, 0x8397}, {0x813f, 0x813f},
        {0x8140, 0x20}, {0x814c, 0x814c}, {0x814f, 0x5e}, {0x8197, 0x40},
        {0x8198, 0x8198}, {0x8258, 0x39}, {0x8259, 0x8259}, {0x825f, 0x825f},
        {0x8260, 0x41}, {0x82f1, 0xdd}, {0x82f2, 0x82f2}, {0,0}};
    unsigned int exp, ret, i;
    unsigned int prev_cp = _getmbcp();

    _setmbcp(932);
    for (i = 0; mbcmbb_932[i][0] != 0; i++)
    {
        ret = _mbctombb(mbcmbb_932[i][0]);
        exp = mbcmbb_932[i][1];
        ok(ret == exp, "Expected 0x%x, got 0x%x\n", exp, ret);
    }
    _setmbcp(prev_cp);
}

static void test_ismbclegal(void) {
    unsigned int prev_cp = _getmbcp();
    int ret, exp, err;
    unsigned int i;

    _setmbcp(932); /* Japanese */
    err = 0;
    for(i = 0; i < 0x10000; i++) {
        ret = _ismbclegal(i);
        exp = ((HIBYTE(i) >= 0x81 && HIBYTE(i) <= 0x9F) ||
               (HIBYTE(i) >= 0xE0 && HIBYTE(i) <= 0xFC)) &&
              ((LOBYTE(i) >= 0x40 && LOBYTE(i) <= 0x7E) ||
               (LOBYTE(i) >= 0x80 && LOBYTE(i) <= 0xFC));
        if(ret != exp) {
            err = 1;
            break;
        }
    }
    ok(!err, "_ismbclegal (932) : Expected 0x%x, got 0x%x (0x%x)\n", exp, ret, i);
    _setmbcp(936); /* Chinese (GBK) */
    err = 0;
    for(i = 0; i < 0x10000; i++) {
        ret = _ismbclegal(i);
        exp = HIBYTE(i) >= 0x81 && HIBYTE(i) <= 0xFE &&
              LOBYTE(i) >= 0x40 && LOBYTE(i) <= 0xFE;
        if(ret != exp) {
            err = 1;
            break;
        }
    }
    ok(!err, "_ismbclegal (936) : Expected 0x%x, got 0x%x (0x%x)\n", exp, ret, i);
    _setmbcp(949); /* Korean */
    err = 0;
    for(i = 0; i < 0x10000; i++) {
        ret = _ismbclegal(i);
        exp = HIBYTE(i) >= 0x81 && HIBYTE(i) <= 0xFE &&
              LOBYTE(i) >= 0x41 && LOBYTE(i) <= 0xFE;
        if(ret != exp) {
            err = 1;
            break;
        }
    }
    ok(!err, "_ismbclegal (949) : Expected 0x%x, got 0x%x (0x%x)\n", exp, ret, i);
    _setmbcp(950); /* Chinese (Big5) */
    err = 0;
    for(i = 0; i < 0x10000; i++) {
        ret = _ismbclegal(i);
        exp = HIBYTE(i) >= 0x81 && HIBYTE(i) <= 0xFE &&
            ((LOBYTE(i) >= 0x40 && LOBYTE(i) <= 0x7E) ||
             (LOBYTE(i) >= 0xA1 && LOBYTE(i) <= 0xFE));
        if(ret != exp) {
            err = 1;
            break;
        }
    }
    ok(!err, "_ismbclegal (950) : Expected 0x%x, got 0x%x (0x%x)\n", exp, ret, i);
    _setmbcp(1361); /* Korean (Johab) */
    err = 0;
    for(i = 0; i < 0x10000; i++) {
        ret = _ismbclegal(i);
        exp = ((HIBYTE(i) >= 0x81 && HIBYTE(i) <= 0xD3) ||
               (HIBYTE(i) >= 0xD8 && HIBYTE(i) <= 0xF9)) &&
              ((LOBYTE(i) >= 0x31 && LOBYTE(i) <= 0x7E) ||
               (LOBYTE(i) >= 0x81 && LOBYTE(i) <= 0xFE)) &&
                HIBYTE(i) != 0xDF;
        if(ret != exp) {
            err = 1;
            break;
        }
    }
    todo_wine ok(!err, "_ismbclegal (1361) : Expected 0x%x, got 0x%x (0x%x)\n", exp, ret, i);

    _setmbcp(prev_cp);
}

static const struct {
    const char* string;
    const char* delimiter;
    int exp_offsetret1; /* returned offset from string after first call to strtok()
                           -1 means NULL  */
    int exp_offsetret2; /* returned offset from string after second call to strtok()
                           -1 means NULL  */
    int exp_offsetret3; /* returned offset from string after third call to strtok()
                           -1 means NULL  */
} testcases_strtok[] = {
    { "red cabernet", " ", 0, 4, -1 },
    { "sparkling white riesling", " ", 0, 10, 16 },
    { " pale cream sherry", "e ", 1, 6, 9 },
    /* end mark */
    { 0}
};

static void test_strtok(void)
{
    int i;
    char *strret;
    char teststr[100];
    for( i = 0; testcases_strtok[i].string; i++){
        strcpy( teststr, testcases_strtok[i].string);
        strret = strtok( teststr, testcases_strtok[i].delimiter);
        ok( (int)(strret - teststr) ==  testcases_strtok[i].exp_offsetret1 ||
                (!strret && testcases_strtok[i].exp_offsetret1 == -1),
                "string (%p) \'%s\' return %p\n",
                teststr, testcases_strtok[i].string, strret);
        if( !strret) continue;
        strret = strtok( NULL, testcases_strtok[i].delimiter);
        ok( (int)(strret - teststr) ==  testcases_strtok[i].exp_offsetret2 ||
                (!strret && testcases_strtok[i].exp_offsetret2 == -1),
                "second call string (%p) \'%s\' return %p\n",
                teststr, testcases_strtok[i].string, strret);
        if( !strret) continue;
        strret = strtok( NULL, testcases_strtok[i].delimiter);
        ok( (int)(strret - teststr) ==  testcases_strtok[i].exp_offsetret3 ||
                (!strret && testcases_strtok[i].exp_offsetret3 == -1),
                "third call string (%p) \'%s\' return %p\n",
                teststr, testcases_strtok[i].string, strret);
    }
}

static void test_strtol(void)
{
    char* e;
    LONG l;
    ULONG ul;

    /* errno is only set in case of error, so reset errno to EBADF to check for errno modification */
    /* errno is modified on W2K8+ */
    errno = EBADF;
    l = strtol("-1234", &e, 0);
    ok(l==-1234, "wrong value %d\n", l);
    ok(errno == EBADF || broken(errno == 0), "wrong errno %d\n", errno);
    errno = EBADF;
    ul = strtoul("1234", &e, 0);
    ok(ul==1234, "wrong value %u\n", ul);
    ok(errno == EBADF || broken(errno == 0), "wrong errno %d\n", errno);

    errno = EBADF;
    l = strtol("2147483647L", &e, 0);
    ok(l==2147483647, "wrong value %d\n", l);
    ok(errno == EBADF || broken(errno == 0), "wrong errno %d\n", errno);
    errno = EBADF;
    l = strtol("-2147483648L", &e, 0);
    ok(l==-2147483647L - 1, "wrong value %d\n", l);
    ok(errno == EBADF || broken(errno == 0), "wrong errno %d\n", errno);
    errno = EBADF;
    ul = strtoul("4294967295UL", &e, 0);
    ok(ul==4294967295ul, "wrong value %u\n", ul);
    ok(errno == EBADF || broken(errno == 0), "wrong errno %d\n", errno);

    errno = 0;
    l = strtol("9223372036854775807L", &e, 0);
    ok(l==2147483647, "wrong value %d\n", l);
    ok(errno == ERANGE, "wrong errno %d\n", errno);
    errno = 0;
    ul = strtoul("9223372036854775807L", &e, 0);
    ok(ul==4294967295ul, "wrong value %u\n", ul);
    ok(errno == ERANGE, "wrong errno %d\n", errno);
}

static void test_strnlen(void)
{
    static const char str[] = "string";
    size_t res;

    if(!p_strnlen) {
        win_skip("strnlen not found\n");
        return;
    }

    res = p_strnlen(str, 20);
    ok(res == 6, "Returned length = %d\n", (int)res);

    res = p_strnlen(str, 3);
    ok(res == 3, "Returned length = %d\n", (int)res);

    res = p_strnlen(NULL, 0);
    ok(res == 0, "Returned length = %d\n", (int)res);
}

static void test__strtoi64(void)
{
    static const char no1[] = "31923";
    static const char no2[] = "-213312";
    static const char no3[] = "12aa";
    static const char no4[] = "abc12";
    static const char overflow[] = "99999999999999999999";
    static const char neg_overflow[] = "-99999999999999999999";
    static const char hex[] = "0x123";
    static const char oct[] = "000123";
    static const char blanks[] = "        12 212.31";

    __int64 res;
    unsigned __int64 ures;
    char *endpos;

    if(!p_strtoi64 || !p_strtoui64) {
        win_skip("_strtoi64 or _strtoui64 not found\n");
        return;
    }

    errno = 0xdeadbeef;
    res = p_strtoi64(no1, NULL, 10);
    ok(res == 31923, "res != 31923\n");
    res = p_strtoi64(no2, NULL, 10);
    ok(res == -213312, "res != -213312\n");
    res = p_strtoi64(no3, NULL, 10);
    ok(res == 12, "res != 12\n");
    res = p_strtoi64(no4, &endpos, 10);
    ok(res == 0, "res != 0\n");
    ok(endpos == no4, "Scanning was not stopped on first character\n");
    res = p_strtoi64(hex, &endpos, 10);
    ok(res == 0, "res != 0\n");
    ok(endpos == hex+1, "Incorrect endpos (%p-%p)\n", hex, endpos);
    res = p_strtoi64(oct, &endpos, 10);
    ok(res == 123, "res != 123\n");
    ok(endpos == oct+strlen(oct), "Incorrect endpos (%p-%p)\n", oct, endpos);
    res = p_strtoi64(blanks, &endpos, 10);
    ok(res == 12, "res != 12");
    ok(endpos == blanks+10, "Incorrect endpos (%p-%p)\n", blanks, endpos);
    ok(errno == 0xdeadbeef, "errno = %x\n", errno);

    errno = 0xdeadbeef;
    res = p_strtoi64(overflow, &endpos, 10);
    ok(res == _I64_MAX, "res != _I64_MAX\n");
    ok(endpos == overflow+strlen(overflow), "Incorrect endpos (%p-%p)\n", overflow, endpos);
    ok(errno == ERANGE, "errno = %x\n", errno);

    errno = 0xdeadbeef;
    res = p_strtoi64(neg_overflow, &endpos, 10);
    ok(res == _I64_MIN, "res != _I64_MIN\n");
    ok(endpos == neg_overflow+strlen(neg_overflow), "Incorrect endpos (%p-%p)\n", neg_overflow, endpos);
    ok(errno == ERANGE, "errno = %x\n", errno);

    errno = 0xdeadbeef;
    res = p_strtoi64(no1, &endpos, 16);
    ok(res == 203043, "res != 203043\n");
    ok(endpos == no1+strlen(no1), "Incorrect endpos (%p-%p)\n", no1, endpos);
    res = p_strtoi64(no2, &endpos, 16);
    ok(res == -2175762, "res != -2175762\n");
    ok(endpos == no2+strlen(no2), "Incorrect endpos (%p-%p)\n", no2, endpos);
    res = p_strtoi64(no3, &endpos, 16);
    ok(res == 4778, "res != 4778\n");
    ok(endpos == no3+strlen(no3), "Incorrect endpos (%p-%p)\n", no3, endpos);
    res = p_strtoi64(no4, &endpos, 16);
    ok(res == 703506, "res != 703506\n");
    ok(endpos == no4+strlen(no4), "Incorrect endpos (%p-%p)\n", no4, endpos);
    res = p_strtoi64(hex, &endpos, 16);
    ok(res == 291, "res != 291\n");
    ok(endpos == hex+strlen(hex), "Incorrect endpos (%p-%p)\n", hex, endpos);
    res = p_strtoi64(oct, &endpos, 16);
    ok(res == 291, "res != 291\n");
    ok(endpos == oct+strlen(oct), "Incorrect endpos (%p-%p)\n", oct, endpos);
    res = p_strtoi64(blanks, &endpos, 16);
    ok(res == 18, "res != 18\n");
    ok(endpos == blanks+10, "Incorrect endpos (%p-%p)\n", blanks, endpos);
    ok(errno == 0xdeadbeef, "errno = %x\n", errno);

    errno = 0xdeadbeef;
    res = p_strtoi64(hex, &endpos, 36);
    ok(res == 1541019, "res != 1541019\n");
    ok(endpos == hex+strlen(hex), "Incorrect endpos (%p-%p)\n", hex, endpos);
    ok(errno == 0xdeadbeef, "errno = %x\n", errno);

    errno = 0xdeadbeef;
    res = p_strtoi64(no1, &endpos, 0);
    ok(res == 31923, "res != 31923\n");
    ok(endpos == no1+strlen(no1), "Incorrect endpos (%p-%p)\n", no1, endpos);
    res = p_strtoi64(no2, &endpos, 0);
    ok(res == -213312, "res != -213312\n");
    ok(endpos == no2+strlen(no2), "Incorrect endpos (%p-%p)\n", no2, endpos);
    res = p_strtoi64(no3, &endpos, 10);
    ok(res == 12, "res != 12\n");
    ok(endpos == no3+2, "Incorrect endpos (%p-%p)\n", no3, endpos);
    res = p_strtoi64(no4, &endpos, 10);
    ok(res == 0, "res != 0\n");
    ok(endpos == no4, "Incorrect endpos (%p-%p)\n", no4, endpos);
    res = p_strtoi64(hex, &endpos, 10);
    ok(res == 0, "res != 0\n");
    ok(endpos == hex+1, "Incorrect endpos (%p-%p)\n", hex, endpos);
    res = p_strtoi64(oct, &endpos, 10);
    ok(res == 123, "res != 123\n");
    ok(endpos == oct+strlen(oct), "Incorrect endpos (%p-%p)\n", oct, endpos);
    res = p_strtoi64(blanks, &endpos, 10);
    ok(res == 12, "res != 12\n");
    ok(endpos == blanks+10, "Incorrect endpos (%p-%p)\n", blanks, endpos);
    ok(errno == 0xdeadbeef, "errno = %x\n", errno);

    errno = 0xdeadbeef;
    ures = p_strtoui64(no1, &endpos, 0);
    ok(ures == 31923, "ures != 31923\n");
    ok(endpos == no1+strlen(no1), "Incorrect endpos (%p-%p)\n", no1, endpos);
    ures = p_strtoui64(no2, &endpos, 0);
    ok(ures == -213312, "ures != -213312\n");
    ok(endpos == no2+strlen(no2), "Incorrect endpos (%p-%p)\n", no2, endpos);
    ures = p_strtoui64(no3, &endpos, 10);
    ok(ures == 12, "ures != 12\n");
    ok(endpos == no3+2, "Incorrect endpos (%p-%p)\n", no3, endpos);
    ures = p_strtoui64(no4, &endpos, 10);
    ok(ures == 0, "ures != 0\n");
    ok(endpos == no4, "Incorrect endpos (%p-%p)\n", no4, endpos);
    ures = p_strtoui64(hex, &endpos, 10);
    ok(ures == 0, "ures != 0\n");
    ok(endpos == hex+1, "Incorrect endpos (%p-%p)\n", hex, endpos);
    ures = p_strtoui64(oct, &endpos, 10);
    ok(ures == 123, "ures != 123\n");
    ok(endpos == oct+strlen(oct), "Incorrect endpos (%p-%p)\n", oct, endpos);
    ures = p_strtoui64(blanks, &endpos, 10);
    ok(ures == 12, "ures != 12\n");
    ok(endpos == blanks+10, "Incorrect endpos (%p-%p)\n", blanks, endpos);
    ok(errno == 0xdeadbeef, "errno = %x\n", errno);

    errno = 0xdeadbeef;
    ures = p_strtoui64(overflow, &endpos, 10);
    ok(ures == _UI64_MAX, "ures != _UI64_MAX\n");
    ok(endpos == overflow+strlen(overflow), "Incorrect endpos (%p-%p)\n", overflow, endpos);
    ok(errno == ERANGE, "errno = %x\n", errno);

    errno = 0xdeadbeef;
    ures = p_strtoui64(neg_overflow, &endpos, 10);
    ok(ures == 1, "ures != 1\n");
    ok(endpos == neg_overflow+strlen(neg_overflow), "Incorrect endpos (%p-%p)\n", neg_overflow, endpos);
    ok(errno == ERANGE, "errno = %x\n", errno);
}

static inline BOOL almost_equal(double d1, double d2) {
    if(d1-d2>-1e-30 && d1-d2<1e-30)
        return TRUE;
    return FALSE;
}

static void test__strtod(void)
{
    const char double1[] = "12.1";
    const char double2[] = "-13.721";
    const char double3[] = "INF";
    const char double4[] = ".21e12";
    const char double5[] = "214353e-3";
    const char overflow[] = "1d9999999999999999999";

    char *end;
    double d;

    d = strtod(double1, &end);
    ok(almost_equal(d, 12.1), "d = %lf\n", d);
    ok(end == double1+4, "incorrect end (%d)\n", end-double1);

    d = strtod(double2, &end);
    ok(almost_equal(d, -13.721), "d = %lf\n", d);
    ok(end == double2+7, "incorrect end (%d)\n", end-double2);

    d = strtod(double3, &end);
    ok(almost_equal(d, 0), "d = %lf\n", d);
    ok(end == double3, "incorrect end (%d)\n", end-double3);

    d = strtod(double4, &end);
    ok(almost_equal(d, 210000000000.0), "d = %lf\n", d);
    ok(end == double4+6, "incorrect end (%d)\n", end-double4);

    d = strtod(double5, &end);
    ok(almost_equal(d, 214.353), "d = %lf\n", d);
    ok(end == double5+9, "incorrect end (%d)\n", end-double5);

    d = strtod("12.1d2", NULL);
    ok(almost_equal(d, 12.1e2), "d = %lf\n", d);

    /* Set locale with non '.' decimal point (',') */
    if(!setlocale(LC_ALL, "Polish")) {
        win_skip("system with limited locales\n");
        return;
    }

    d = strtod("12.1", NULL);
    ok(almost_equal(d, 12.0), "d = %lf\n", d);

    d = strtod("12,1", NULL);
    ok(almost_equal(d, 12.1), "d = %lf\n", d);

    setlocale(LC_ALL, "C");

    /* Precision tests */
    d = strtod("0.1", NULL);
    ok(almost_equal(d, 0.1), "d = %lf\n", d);
    d = strtod("-0.1", NULL);
    ok(almost_equal(d, -0.1), "d = %lf\n", d);
    d = strtod("0.1281832188491894198128921", NULL);
    ok(almost_equal(d, 0.1281832188491894198128921), "d = %lf\n", d);
    d = strtod("0.82181281288121", NULL);
    ok(almost_equal(d, 0.82181281288121), "d = %lf\n", d);
    d = strtod("21921922352523587651128218821", NULL);
    ok(almost_equal(d, 21921922352523587651128218821.0), "d = %lf\n", d);
    d = strtod("0.1d238", NULL);
    ok(almost_equal(d, 0.1e238L), "d = %lf\n", d);
    d = strtod("0.1D-4736", NULL);
    ok(almost_equal(d, 0.1e-4736L), "d = %lf\n", d);

    errno = 0xdeadbeef;
    d = strtod(overflow, &end);
    ok(errno == ERANGE, "errno = %x\n", errno);
    ok(end == overflow+21, "incorrect end (%d)\n", end-overflow);

    errno = 0xdeadbeef;
    strtod("-1d309", NULL);
    ok(errno == ERANGE, "errno = %x\n", errno);
}

START_TEST(string)
{
    char mem[100];
    static const char xilstring[]="c:/xilinx";
    int nLen;

    hMsvcrt = GetModuleHandleA("msvcrt.dll");
    if (!hMsvcrt)
        hMsvcrt = GetModuleHandleA("msvcrtd.dll");
    ok(hMsvcrt != 0, "GetModuleHandleA failed\n");
    SET(pmemcpy,"memcpy");
    SET(pmemcmp,"memcmp");
    SET(p_mbctype,"_mbctype");
    SET(p__mb_cur_max,"__mb_cur_max");
    pstrcpy_s = (void *)GetProcAddress( hMsvcrt,"strcpy_s" );
    pstrcat_s = (void *)GetProcAddress( hMsvcrt,"strcat_s" );
    p_mbsnbcpy_s = (void *)GetProcAddress( hMsvcrt,"_mbsnbcpy_s" );
    p_wcscpy_s = (void *)GetProcAddress( hMsvcrt,"wcscpy_s" );
    p_wcsupr_s = (void *)GetProcAddress( hMsvcrt,"_wcsupr_s" );
    p_strnlen = (void *)GetProcAddress( hMsvcrt,"strnlen" );
    p_strtoi64 = (void *) GetProcAddress(hMsvcrt, "_strtoi64");
    p_strtoui64 = (void *) GetProcAddress(hMsvcrt, "_strtoui64");

    /* MSVCRT memcpy behaves like memmove for overlapping moves,
       MFC42 CString::Insert seems to rely on that behaviour */
    strcpy(mem,xilstring);
    nLen=strlen(xilstring);
    pmemcpy(mem+5, mem,nLen+1);
    ok(pmemcmp(mem+5,xilstring, nLen) == 0,
       "Got result %s\n",mem+5);

    /* Test _swab function */
    test_swab();

    /* Test ismbblead*/
    test_mbcp();
   /* test _mbsspn */
    test_mbsspn();
    test_mbsspnp();
   /* test _strdup */
    test_strdup();
    test_strcpy_s();
    test_strcat_s();
    test__mbsnbcpy_s();
    test_mbcjisjms();
    test_mbctombb();
    test_ismbclegal();
    test_strtok();
    test_wcscpy_s();
    test__wcsupr_s();
    test_strtol();
    test_strnlen();
    test__strtoi64();
    test__strtod();
}
