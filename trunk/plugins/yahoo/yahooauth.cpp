/***************************************************************************
                          yahooauth.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 * Based on libyahoo2
 *
 * Some code copyright (C) 2002-2004, Philip S Tellis <philip.tellis AT gmx.net>
 *
 * Yahoo Search copyright (C) 2003, Konstantin Klyagin <konst AT konst.org.ua>
 *
 * Much of this code was taken and adapted from the yahoo module for
 * gaim released under the GNU GPL.  This code is also released under the 
 * GNU GPL.
 *
 * This code is derivitive of Gaim <http://gaim.sourceforge.net>
 * copyright (C) 1998-1999, Mark Spencer <markster@marko.net>
 *             1998-1999, Adam Fritzler <afritz@marko.net>
 *             1998-2002, Rob Flynn <rob@marko.net>
 *             2000-2002, Eric Warmenhoven <eric@warmenhoven.org>
 *             2001-2002, Brian Macke <macke@strangelove.net>
 *                  2001, Anand Biligiri S <abiligiri@users.sf.net>
 *                  2001, Valdis Kletnieks
 *                  2002, Sean Egan <bj91704@binghamton.edu>
 *                  2002, Toby Gray <toby.gray@ntlworld.com>
 *
 * This library also uses code from other libraries, namely:
 *     Portions from libfaim copyright 1998, 1999 Adam Fritzler
 *     <afritz@auk.cx>
 *     Portions of Sylpheed copyright 2000-2002 Hiroyuki Yamamoto
 *     <hiro-y@kcn.ne.jp>
 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "yahooclient.h"

#include <openssl/md5.h>
#include <openssl/sha.h>

extern "C"
{
    char *yahoo_crypt(const char *key, const char *salt);
    int yahoo_Dispatch(int Salt, int Parameter);
};

/* This is the y64 alphabet... it's like base64, but has a . and a _ */
char base64digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789._";

/* This is taken from Sylpheed by Hiroyuki Yamamoto.  We have our own tobase64 function
 * in util.c, but it has a bug I don't feel like finding right now ;) */
void to_y64(unsigned char *out, const unsigned char *in, int inlen)
/* raw bytes in quasi-big-endian order to base 64 string (NUL-terminated) */
{
    for (; inlen >= 3; inlen -= 3)
    {
        *out++ = base64digits[in[0] >> 2];
        *out++ = base64digits[((in[0] << 4) & 0x30) | (in[1] >> 4)];
        *out++ = base64digits[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
        *out++ = base64digits[in[2] & 0x3f];
        in += 3;
    }
    if (inlen > 0)
    {
        unsigned char fragment;

        *out++ = base64digits[in[0] >> 2];
        fragment = (in[0] << 4) & 0x30;
        if (inlen > 1)
            fragment |= in[1] >> 4;
        *out++ = base64digits[fragment];
        *out++ = (inlen < 2) ? '-' : base64digits[(in[1] << 2) & 0x3c];
        *out++ = '-';
    }
    *out = '\0';
}

void YahooClient::process_auth(const char *method, const char *seed, const char *sn)
{
    if (!method || !seed || !sn){
        m_socket->error_state("Bad auth packet");
        return;
    }
    if (atol(method) != 1){
        m_socket->error_state("Unknown auth method");
        return;
    }
    string password = getPassword().ascii();

    unsigned char      result[16];
    MD5_CTX	           ctx;

    SHA_CTX            ctx1;
    SHA_CTX            ctx2;

    const char *alphabet1 = "FBZDWAGHrJTLMNOPpRSKUVEXYChImkwQ";
    const char *alphabet2 = "F0E1D2C3B4A59687abcdefghijklmnop";

    const char *challenge_lookup = "qzec2tb3um1olpar8whx4dfgijknsvy5";
    const char *operand_lookup = "+|&%/*^-";
    const char *delimit_lookup = ",;";

    char *password_hash = (char*)malloc(25);
    char *crypt_hash = (char*)malloc(25);
    char *crypt_result = NULL;
    char pass_hash_xor1[64];
    char pass_hash_xor2[64];
    char crypt_hash_xor1[64];
    char crypt_hash_xor2[64];
    char resp_6[100];
    char resp_96[100];

    unsigned char digest1[20];
    unsigned char digest2[20];
    unsigned char magic_key_char[4];
    const unsigned char *magic_ptr;

    unsigned int  magic[64];
    unsigned int  magic_work = 0;

    char comparison_src[20];
    int x;
    int i, j;
    int cnt = 0;
    int magic_cnt = 0;
    int magic_len;
    int magic_iterations = -1;
    int magic_method = -1;
    int magic_from_tables;

    unsigned char md5test[7];

    FILE *dump = fopen("yahoo.log", "a");
    fprintf(dump, "Yahoo auth\n");

    memset(&pass_hash_xor1, 0, 64);
    memset(&pass_hash_xor2, 0, 64);
    memset(&crypt_hash_xor1, 0, 64);
    memset(&crypt_hash_xor2, 0, 64);
    memset(&digest1, 0, 20);
    memset(&digest2, 0, 20);
    memset(&magic, 0, 64);
    memset(&resp_6, 0, 100);
    memset(&resp_96, 0, 100);
    memset(&magic_key_char, 0, 4);

    /*
     * Magic: Phase 1.  Generate what seems to be a 30 
     * byte value (could change if base64
     * ends up differently?  I don't remember and I'm 
     * tired, so use a 64 byte buffer.
     */

    magic_ptr = (unsigned char*)seed;

    while (*magic_ptr != '\0') {
        char *loc;

        /* Ignore parentheses.  */

        if (*magic_ptr == '(' || *magic_ptr == ')') {
            magic_ptr++;
            continue;
        }

        /* Characters and digits verify against
           the challenge lookup.
        */

        if (isalpha(*magic_ptr) || isdigit(*magic_ptr)) {
            loc = strchr(challenge_lookup, *magic_ptr);
            if (!loc) {
                /* This isn't good */
            }

            /* Get offset into lookup table and lsh 3. */

            magic_work = loc - challenge_lookup;
            magic_work <<= 3;

            magic_ptr++;
            continue;
        } else {
            unsigned int local_store;

            loc = strchr(operand_lookup, *magic_ptr);
            if (!loc) {
                /* Also not good. */
            }

            local_store = loc - operand_lookup;

            /* Oops; how did this happen? */
            if (magic_cnt >= 64)
                break;

            magic[magic_cnt++] = magic_work | local_store;
            magic_ptr++;
            continue;
        }
    }

    magic_len = magic_cnt;
    magic_cnt = 0;

    /* Magic: Phase 2.  Take generated magic value and
     * sprinkle fairy dust on the values. */

    for (magic_cnt = magic_len-2; magic_cnt >= 0; magic_cnt--) {
        unsigned char byte1;
        unsigned char byte2;

        /* Bad.  Abort.
         */
        if ((magic_cnt + 1 > magic_len) ||
                (magic_cnt > magic_len))
            break;

        byte1 = magic[magic_cnt];
        byte2 = magic[magic_cnt+1];

        byte1 *= 0xcd;
        byte1 ^= byte2;

        magic[magic_cnt+1] = byte1;
    }

    /* Magic: Phase 3.  This computes 20 bytes.  The first 4 bytes are used as our magic
     * key (and may be changed later); the next 16 bytes are an MD5 sum of the magic key 
     * plus 3 bytes.  The 3 bytes are found by looping, and they represent the offsets 
     * into particular functions we'll later call to potentially alter the magic key. 
     * 
     * %-) 
     */ 

    magic_cnt = 1;
    x = 0;

    do {
        unsigned int     bl = 0;
        unsigned int     cl = magic[magic_cnt++];

        if (magic_cnt >= magic_len)
            break;

        if (cl > 0x7F) {
            if (cl < 0xe0)
                bl = cl = (cl & 0x1f) << 6;
            else {
                bl = magic[magic_cnt++];
                cl = (cl & 0x0f) << 6;
                bl = ((bl & 0x3f) + cl) << 6;
            }

            cl = magic[magic_cnt++];
            bl = (cl & 0x3f) + bl;
        } else
            bl = cl;

        comparison_src[x++] = (bl & 0xff00) >> 8;
        comparison_src[x++] = bl & 0xff;
    } while (x < 20);

    /* First four bytes are the initial magic key. */
    for (x = 0; x < 4; x++)
        magic_key_char[x] = comparison_src[x];

    /* We need to determine the final magic key. When in mode 0, it's what we just
     * calculated otherwise we'll need to use the big ugly lookup tables to calculate
     * it. Many thanks to Jeffrey Lim for figuring this bit out.
     */

    memcpy(md5test, magic_key_char, 4);

    for(i = 0; i < 0xFFFF; i++)
    {
        md5test[4] = i;
        md5test[5] = i >> 8;

        for(j = 0; j <= 5; j++)
        {
            md5test[6] = j;

            MD5_Init(&ctx);
            MD5_Update(&ctx, md5test, 7);
            MD5_Final(result, &ctx);

            if(!memcmp(&result, comparison_src+4, 16))
            {
                magic_iterations = i;
                magic_method = j;
            }

        }
    }

    if (magic_method == 1) {
        unsigned int Salt = magic_key_char[0] | (magic_key_char[1]<<8) | (magic_key_char[2]<<16) | (magic_key_char[3]<<24);
        magic_from_tables = yahoo_Dispatch(yahoo_Dispatch(Salt, magic_iterations), magic_iterations);

        magic_key_char[0] = magic_from_tables & 0xff;
        magic_key_char[1] = (magic_from_tables >> 8) & 0xff;
        magic_key_char[2] = (magic_from_tables >> 16) & 0xff;
        magic_key_char[3] = (magic_from_tables >> 24) & 0xff;
    }

    /* Get password and crypt hashes as per usual. */
    MD5_Init(&ctx);
    MD5_Update(&ctx, (unsigned char*)password.c_str(), password.length());
    MD5_Final(result, &ctx);
    to_y64((unsigned char*)password_hash, result, 16);

    MD5_Init(&ctx);
    crypt_result = yahoo_crypt(password.c_str(), "$1$_2S43d5f$");
    MD5_Update(&ctx, (unsigned char*)crypt_result, strlen(crypt_result));
    MD5_Final(result, &ctx);
    to_y64((unsigned char*)crypt_hash, result, 16);

    /* Our first authentication response is based off
     * of the password hash. */

    for (x = 0; x < (int)strlen(password_hash); x++)
        pass_hash_xor1[cnt++] = password_hash[x] ^ 0x36;

    if (cnt < 64)
        memset(&(pass_hash_xor1[cnt]), 0x36, 64-cnt);

    cnt = 0;

    for (x = 0; x < (int)strlen(password_hash); x++)
        pass_hash_xor2[cnt++] = password_hash[x] ^ 0x5c;

    if (cnt < 64)
        memset(&(pass_hash_xor2[cnt]), 0x5c, 64-cnt);

    SHA1_Init(&ctx1);
    SHA1_Init(&ctx2);

    /* The first context gets the password hash XORed
     * with 0x36 plus a magic value
     * which we previously extrapolated from our 
     * challenge. */

    SHA1_Update(&ctx1, (unsigned char*)pass_hash_xor1, 64);
    SHA1_Update(&ctx1, magic_key_char, 4);
    SHA1_Final(digest1, &ctx1);

    /* The second context gets the password hash XORed
     * with 0x5c plus the SHA-1 digest
     * of the first context. */

    SHA1_Update(&ctx2, (unsigned char*)pass_hash_xor2, 64);
    SHA1_Update(&ctx2, digest1, 20);
    SHA1_Final(digest2, &ctx2);

    /* Now that we have digest2, use it to fetch
     * characters from an alphabet to construct
     * our first authentication response. */

    for (x = 0; x < 20; x += 2) {
        unsigned int    val = 0;
        unsigned int    lookup = 0;
        char            byte[6];

        memset(&byte, 0, 6);

        /* First two bytes of digest stuffed
         *  together.
         */

        val = digest2[x];
        val <<= 8;
        val += digest2[x+1];

        lookup = (val >> 0x0b);
        lookup &= 0x1f;
        if (lookup >= strlen(alphabet1))
            break;
        sprintf(byte, "%c", alphabet1[lookup]);
        strcat(resp_6, byte);
        strcat(resp_6, "=");

        lookup = (val >> 0x06);
        lookup &= 0x1f;
        if (lookup >= strlen(alphabet2))
            break;
        sprintf(byte, "%c", alphabet2[lookup]);
        strcat(resp_6, byte);

        lookup = (val >> 0x01);
        lookup &= 0x1f;
        if (lookup >= strlen(alphabet2))
            break;
        sprintf(byte, "%c", alphabet2[lookup]);
        strcat(resp_6, byte);

        lookup = (val & 0x01);
        if (lookup >= strlen(delimit_lookup))
            break;
        sprintf(byte, "%c", delimit_lookup[lookup]);
        strcat(resp_6, byte);
    }

    /* Our second authentication response is based off
     * of the crypto hash. */

    cnt = 0;
    memset(&digest1, 0, 20);
    memset(&digest2, 0, 20);

    for (x = 0; x < (int)strlen(crypt_hash); x++)
        crypt_hash_xor1[cnt++] = crypt_hash[x] ^ 0x36;

    if (cnt < 64)
        memset(&(crypt_hash_xor1[cnt]), 0x36, 64-cnt);

    cnt = 0;

    for (x = 0; x < (int)strlen(crypt_hash); x++)
        crypt_hash_xor2[cnt++] = crypt_hash[x] ^ 0x5c;

    if (cnt < 64)
        memset(&(crypt_hash_xor2[cnt]), 0x5c, 64-cnt);

    SHA1_Init(&ctx1);
    SHA1_Init(&ctx2);

    /* The first context gets the password hash XORed
     * with 0x36 plus a magic value
     * which we previously extrapolated from our 
     * challenge. */

    SHA1_Update(&ctx1, (unsigned char*)crypt_hash_xor1, 64);
    SHA1_Update(&ctx1, magic_key_char, 4);
    SHA1_Final(digest1, &ctx1);

    /* The second context gets the password hash XORed
     * with 0x5c plus the SHA-1 digest
     * of the first context. */

    SHA1_Update(&ctx2, (unsigned char*)crypt_hash_xor2, 64);
    SHA1_Update(&ctx2, digest1, 20);
    SHA1_Final(digest2, &ctx2);

    /* Now that we have digest2, use it to fetch
     * characters from an alphabet to construct
     * our first authentication response.  */

    for (x = 0; x < 20; x += 2) {
        unsigned int val = 0;
        unsigned int lookup = 0;

        char byte[6];

        memset(&byte, 0, 6);

        /* First two bytes of digest stuffed
         *  together. */

        val = digest2[x];
        val <<= 8;
        val += digest2[x+1];

        lookup = (val >> 0x0b);
        lookup &= 0x1f;
        if (lookup >= strlen(alphabet1))
            break;
        sprintf(byte, "%c", alphabet1[lookup]);
        strcat(resp_96, byte);
        strcat(resp_96, "=");

        lookup = (val >> 0x06);
        lookup &= 0x1f;
        if (lookup >= strlen(alphabet2))
            break;
        sprintf(byte, "%c", alphabet2[lookup]);
        strcat(resp_96, byte);

        lookup = (val >> 0x01);
        lookup &= 0x1f;
        if (lookup >= strlen(alphabet2))
            break;
        sprintf(byte, "%c", alphabet2[lookup]);
        strcat(resp_96, byte);

        lookup = (val & 0x01);
        if (lookup >= strlen(delimit_lookup))
            break;
        sprintf(byte, "%c", delimit_lookup[lookup]);
        strcat(resp_96, byte);
    }
    addParam(0, sn);
    addParam(6, resp_6);
    addParam(96, resp_96);
    addParam(1, sn);
    sendPacket(YAHOO_SERVICE_AUTHRESP);
}




