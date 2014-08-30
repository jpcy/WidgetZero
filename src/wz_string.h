/* SDS (Simple Dynamic Strings), A C dynamic strings library.
 *
 * Copyright (c) 2006-2014, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WZ_STRING_H
#define WZ_STRING_H

#define SDS_MAX_PREALLOC (1024*1024)

#include <sys/types.h>
#include <stdarg.h>

typedef char *wzString;

struct wzStringHeader {
    int len;
    int free;
    char buf[];
};

static size_t wz_string_length(const wzString s) {
    struct wzStringHeader *sh = (void*)(s-sizeof *sh);
    return sh->len;
}

static size_t wz_string_available(const wzString s) {
    struct wzStringHeader *sh = (void*)(s-sizeof *sh);
    return sh->free;
}

wzString wz_string_new_length(const void *init, size_t initlen);
wzString wz_string_new(const char *init);
wzString wz_string_empty(void);
size_t wz_string_length(const wzString s);
wzString wz_string_upper(const wzString s);
void wz_string_free(wzString s);
size_t wz_string_available(const wzString s);
wzString wz_string_grow_zero(wzString s, size_t len);
wzString wz_string_concat_length(wzString s, const void *t, size_t len);
wzString wz_string_concat(wzString s, const char *t);
wzString wz_string_concat_string(wzString s, const wzString t);
wzString wz_string_copy_length(wzString s, const char *t, size_t len);
wzString wz_string_copy(wzString s, const char *t);

wzString wz_string_concat_vprintf(wzString s, const char *fmt, va_list ap);
#ifdef __GNUC__
wzString wz_string_concat_printf(wzString s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
wzString wz_string_concat_printf(wzString s, const char *fmt, ...);
#endif

void wz_string_trim(wzString s, const char *cset);
void wz_string_range(wzString s, int start, int end);
void wz_string_update_length(wzString s);
void wz_string_clear(wzString s);
int wz_string_compare(const wzString s1, const wzString s2);
wzString *wz_string_split_length(const char *s, int len, const char *sep, int seplen, int *count);
void wz_string_free_split_result(wzString *tokens, int count);
void wz_string_to_lower(wzString s);
void wz_string_to_upper(wzString s);
wzString wz_string_from_longlong(long long value);
wzString wz_string_concat_representation(wzString s, const char *p, size_t len);
wzString *wz_string_split_args(const char *line, int *argc);
wzString wz_string_map_chars(wzString s, const char *from, const char *to, size_t setlen);
wzString wz_string_join(char **argv, int argc, char *sep, size_t seplen);
wzString wz_string_join_string(wzString *argv, int argc, const char *sep, size_t seplen);

/* Low level functions exposed to the user API */
wzString wzStringMakeRoomFor(wzString s, size_t addlen);
void wzStringIncrLen(wzString s, int incr);
wzString wzStringRemoveFreeSpace(wzString s);
size_t wzStringAllocSize(wzString s);

#endif
