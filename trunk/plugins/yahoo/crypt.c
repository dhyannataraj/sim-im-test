/* One way encryption based on MD5 sum.
   Copyright (C) 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* warmenhoven took this file and made it work with the md5.[ch] we
 * already had. isn't that lovely. people should just use linux or
 * freebsd, crypt works properly on those systems. i hate solaris */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/md5.h>

/* Define our magic string to mark salt for MD5 "encryption"
   replacement.  This is meant to be the same as for other MD5 based
   encryption implementations.  */
const char md5_salt_prefix[] = "$1$";

/* Table with characters for base64 transformation.  */
const char b64t[64] =
    "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

#define MIN(A, B)	((A < B) ? A : B)
#define MAX(A, B)	((A > B) ? A : B)

char *yahoo_crypt(const char *key, const char *salt)
{
    static char *buffer = NULL;
    static int buflen = 0;
    int needed = 3 + strlen (salt) + 1 + 26 + 1;

    unsigned char alt_result[16];
    MD5_CTX ctx;
    MD5_CTX alt_ctx;
    size_t salt_len;
    size_t key_len;
    size_t cnt;
    char *cp;

    if (buflen < needed) {
        buflen = needed;
        if ((buffer = realloc(buffer, buflen)) == NULL)
            return NULL;
    }

    /* Find beginning of salt string.  The prefix should normally always
       be present.  Just in case it is not.  */
    if (strncmp (md5_salt_prefix, salt, sizeof (md5_salt_prefix) - 1) == 0)
        /* Skip salt prefix.  */
        salt += sizeof (md5_salt_prefix) - 1;

    salt_len = MIN (strcspn (salt, "$"), 8);
    key_len = strlen (key);

    /* Prepare for the real work.  */
    MD5_Init(&ctx);

    /* Add the key string.  */
    MD5_Update(&ctx, (unsigned char*)key, key_len);

    /* Because the SALT argument need not always have the salt prefix we
       add it separately.  */
    MD5_Update(&ctx, (unsigned char*)md5_salt_prefix, sizeof (md5_salt_prefix) - 1);

    /* The last part is the salt string.  This must be at most 8
       characters and it ends at the first `$' character (for
       compatibility which existing solutions).  */
    MD5_Update(&ctx, (unsigned char*)salt, salt_len);

    /* Compute alternate MD5 sum with input KEY, SALT, and KEY.  The
       final result will be added to the first context.  */
    MD5_Init(&alt_ctx);

    /* Add key.  */
    MD5_Update(&alt_ctx, (unsigned char*)key, key_len);

    /* Add salt.  */
    MD5_Update(&alt_ctx, (unsigned char*)salt, salt_len);

    /* Add key again.  */
    MD5_Update(&alt_ctx, (unsigned char*)key, key_len);

    /* Now get result of this (16 bytes) and add it to the other
       context.  */
    MD5_Final(alt_result, &alt_ctx);

    /* Add for any character in the key one byte of the alternate sum.  */
    for (cnt = key_len; cnt > 16; cnt -= 16)
        MD5_Update(&ctx, alt_result, 16);
    MD5_Update(&ctx, alt_result, cnt);

    /* For the following code we need a NUL byte.  */
    alt_result[0] = '\0';

    /* The original implementation now does something weird: for every 1
       bit in the key the first 0 is added to the buffer, for every 0
       bit the first character of the key.  This does not seem to be
       what was intended but we have to follow this to be compatible.  */
    for (cnt = key_len; cnt > 0; cnt >>= 1)
        MD5_Update(&ctx, (cnt & 1) != 0 ? alt_result : (unsigned char *)key, 1);

    /* Create intermediate result.  */
    MD5_Final(alt_result, &ctx);

    /* Now comes another weirdness.  In fear of password crackers here
       comes a quite long loop which just processes the output of the
       previous round again.  We cannot ignore this here.  */
    for (cnt = 0; cnt < 1000; ++cnt) {
        /* New context.  */
        MD5_Init(&ctx);

        /* Add key or last result.  */
        if ((cnt & 1) != 0)
            MD5_Update(&ctx, (unsigned char*)key, key_len);
        else
            MD5_Update(&ctx, alt_result, 16);

        /* Add salt for numbers not divisible by 3.  */
        if (cnt % 3 != 0)
            MD5_Update(&ctx, (unsigned char*)salt, salt_len);

        /* Add key for numbers not divisible by 7.  */
        if (cnt % 7 != 0)
            MD5_Update(&ctx, (unsigned char*)key, key_len);

        /* Add key or last result.  */
        if ((cnt & 1) != 0)
            MD5_Update(&ctx, alt_result, 16);
        else
            MD5_Update(&ctx, (unsigned char*)key, key_len);

        /* Create intermediate result.  */
        MD5_Final(alt_result, &ctx);
    }

    /* Now we can construct the result string.  It consists of three
       parts.  */

    strncpy(buffer, md5_salt_prefix, MAX (0, buflen));
    cp = buffer + strlen(buffer);
    buflen -= sizeof (md5_salt_prefix);

    strncpy(cp, salt, MIN ((size_t) buflen, salt_len));
    cp = cp + strlen(cp);
    buflen -= MIN ((size_t) buflen, salt_len);

    if (buflen > 0) {
        *cp++ = '$';
        --buflen;
    }

#define b64_from_24bit(B2, B1, B0, N) \
	{ \
		unsigned int w = ((B2) << 16) | ((B1) << 8) | (B0); \
		int n = (N); \
		while (n-- > 0 && buflen > 0) { \
			*cp++ = b64t[w & 0x3f]; \
			--buflen; \
			w >>= 6; \
		}\
	}

    b64_from_24bit (alt_result[0], alt_result[6], alt_result[12], 4);
    b64_from_24bit (alt_result[1], alt_result[7], alt_result[13], 4);
    b64_from_24bit (alt_result[2], alt_result[8], alt_result[14], 4);
    b64_from_24bit (alt_result[3], alt_result[9], alt_result[15], 4);
    b64_from_24bit (alt_result[4], alt_result[10], alt_result[5], 4);
    b64_from_24bit (0, 0, alt_result[11], 2);
    if (buflen <= 0) {
        free(buffer);
        buffer = NULL;
    } else
        *cp = '\0';	/* Terminate the string.  */

    /* Clear the buffer for the intermediate result so that people
       attaching to processes or reading core dumps cannot get any
       information.  We do it in this way to clear correct_words[]
       inside the MD5 implementation as well.  */
    MD5_Init(&ctx);
    MD5_Final(alt_result, &ctx);
    memset (&ctx, '\0', sizeof (ctx));
    memset (&alt_ctx, '\0', sizeof (alt_ctx));

    return buffer;
}
