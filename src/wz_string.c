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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "wz_string.h"

/* Create a new wzString string with the content specified by the 'init' pointer
 * and 'initlen'.
 * If NULL is used for 'init' the string is initialized with zero bytes.
 *
 * The string is always null-termined (all the wzString strings are, always) so
 * even if you create an wzString string with:
 *
 * mystring = wz_string_new_length("abc",3");
 *
 * You can print the string with printf() as there is an implicit \0 at the
 * end of the string. However the string is binary safe and can contain
 * \0 characters in the middle, as the length is stored in the wzString header. */
wzString wz_string_new_length(const void *init, size_t initlen) {
    struct wzStringHeader *sh;

    if (init) {
        sh = malloc(sizeof *sh+initlen+1);
    } else {
        sh = calloc(sizeof *sh+initlen+1,1);
    }
    if (sh == NULL) return NULL;
    sh->len = initlen;
    sh->free = 0;
    if (initlen && init)
        memcpy(sh->buf, init, initlen);
    sh->buf[initlen] = '\0';
    return (char*)sh->buf;
}

/* Create an empty (zero length) wzString string. Even in this case the string
 * always has an implicit null term. */
wzString wz_string_empty(void) {
    return wz_string_new_length("",0);
}

/* Create a new wzString string starting from a null termined C string. */
wzString wz_string_new(const char *init) {
    size_t initlen = (init == NULL) ? 0 : strlen(init);
    return wz_string_new_length(init, initlen);
}

/* Duplicate an wzString string. */
wzString wz_string_upper(const wzString s) {
    return wz_string_new_length(s, wz_string_length(s));
}

/* Free an wzString string. No operation is performed if 's' is NULL. */
void wz_string_free(wzString s) {
    if (s == NULL) return;
    free(s-sizeof(struct wzStringHeader));
}

/* Set the wzString string length to the length as obtained with strlen(), so
 * considering as content only up to the first null term character.
 *
 * This function is useful when the wzString string is hacked manually in some
 * way, like in the following example:
 *
 * s = wz_string_new("foobar");
 * s[2] = '\0';
 * wz_string_update_length(s);
 * printf("%d\n", wz_string_length(s));
 *
 * The output will be "2", but if we comment out the call to wz_string_update_length()
 * the output will be "6" as the string was modified but the logical length
 * remains 6 bytes. */
void wz_string_update_length(wzString s) {
    struct wzStringHeader *sh = (void*) (s-sizeof *sh);
    int reallen = strlen(s);
    sh->free += (sh->len-reallen);
    sh->len = reallen;
}

/* Modify an wzString string on-place to make it empty (zero length).
 * However all the existing buffer is not discarded but set as free space
 * so that next append operations will not require allocations up to the
 * number of bytes previously available. */
void wz_string_clear(wzString s) {
    struct wzStringHeader *sh = (void*) (s-sizeof *sh);;
    sh->free += sh->len;
    sh->len = 0;
    sh->buf[0] = '\0';
}

/* Enlarge the free space at the end of the wzString string so that the caller
 * is sure that after calling this function can overwrite up to addlen
 * bytes after the end of the string, plus one more byte for nul term.
 * 
 * Note: this does not change the *length* of the wzString string as returned
 * by wz_string_length(), but only the free buffer space we have. */
wzString wzStringMakeRoomFor(wzString s, size_t addlen) {
    struct wzStringHeader *sh, *newsh;
    size_t free = wz_string_available(s);
    size_t len, newlen;

    if (free >= addlen) return s;
    len = wz_string_length(s);
    sh = (void*) (s-sizeof *sh);;
    newlen = (len+addlen);
    if (newlen < SDS_MAX_PREALLOC)
        newlen *= 2;
    else
        newlen += SDS_MAX_PREALLOC;
    newsh = realloc(sh, sizeof *newsh+newlen+1);
    if (newsh == NULL) return NULL;

    newsh->free = newlen - len;
    return newsh->buf;
}

/* Reallocate the wzString string so that it has no free space at the end. The
 * contained string remains not altered, but next concatenation operations
 * will require a reallocation.
 *
 * After the call, the passed wzString string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
wzString wzStringRemoveFreeSpace(wzString s) {
    struct wzStringHeader *sh;

    sh = (void*) (s-sizeof *sh);;
    sh = realloc(sh, sizeof *sh+sh->len+1);
    sh->free = 0;
    return sh->buf;
}

/* Return the total size of the allocation of the specifed wzString string,
 * including:
 * 1) The wzString header before the pointer.
 * 2) The string.
 * 3) The free buffer at the end if any.
 * 4) The implicit null term.
 */
size_t wzStringAllocSize(wzString s) {
    struct wzStringHeader *sh = (void*) (s-sizeof *sh);;

    return sizeof(*sh)+sh->len+sh->free+1;
}

/* Increment the wzString length and decrements the left free space at the
 * end of the string according to 'incr'. Also set the null term
 * in the new end of the string.
 *
 * This function is used in order to fix the string length after the
 * user calls wzStringMakeRoomFor(), writes something after the end of
 * the current string, and finally needs to set the new length.
 *
 * Note: it is possible to use a negative increment in order to
 * right-trim the string.
 *
 * Usage example:
 *
 * Using wzStringIncrLen() and wzStringMakeRoomFor() it is possible to mount the
 * following schema, to cat bytes coming from the kernel to the end of an
 * wzString string without copying into an intermediate buffer:
 *
 * oldlen = wz_string_length(s);
 * s = wzStringMakeRoomFor(s, BUFFER_SIZE);
 * nread = read(fd, s+oldlen, BUFFER_SIZE);
 * ... check for nread <= 0 and handle it ...
 * wzStringIncrLen(s, nread);
 */
void wzStringIncrLen(wzString s, int incr) {
    struct wzStringHeader *sh = (void*) (s-sizeof *sh);;

    assert(sh->free >= incr);
    sh->len += incr;
    sh->free -= incr;
    assert(sh->free >= 0);
    s[sh->len] = '\0';
}

/* Grow the wzString to have the specified length. Bytes that were not part of
 * the original length of the wzString will be set to zero.
 *
 * if the specified length is smaller than the current length, no operation
 * is performed. */
wzString wz_string_grow_zero(wzString s, size_t len) {
    struct wzStringHeader *sh = (void*) (s-sizeof *sh);
    size_t totlen, curlen = sh->len;

    if (len <= curlen) return s;
    s = wzStringMakeRoomFor(s,len-curlen);
    if (s == NULL) return NULL;

    /* Make sure added region doesn't contain garbage */
    sh = (void*)(s-sizeof *sh);
    memset(s+curlen,0,(len-curlen+1)); /* also set trailing \0 byte */
    totlen = sh->len+sh->free;
    sh->len = len;
    sh->free = totlen-sh->len;
    return s;
}

/* Append the specified binary-safe string pointed by 't' of 'len' bytes to the
 * end of the specified wzString string 's'.
 *
 * After the call, the passed wzString string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
wzString wz_string_concat_length(wzString s, const void *t, size_t len) {
    struct wzStringHeader *sh;
    size_t curlen = wz_string_length(s);

    s = wzStringMakeRoomFor(s,len);
    if (s == NULL) return NULL;
    sh = (void*) (s-sizeof *sh);;
    memcpy(s+curlen, t, len);
    sh->len = curlen+len;
    sh->free = sh->free-len;
    s[curlen+len] = '\0';
    return s;
}

/* Append the specified null termianted C string to the wzString string 's'.
 *
 * After the call, the passed wzString string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
wzString wz_string_concat(wzString s, const char *t) {
    return wz_string_concat_length(s, t, strlen(t));
}

/* Append the specified wzString 't' to the existing wzString 's'.
 *
 * After the call, the modified wzString string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
wzString wz_string_concat_string(wzString s, const wzString t) {
    return wz_string_concat_length(s, t, wz_string_length(t));
}

/* Destructively modify the wzString string 's' to hold the specified binary
 * safe string pointed by 't' of length 'len' bytes. */
wzString wz_string_copy_length(wzString s, const char *t, size_t len) {
    struct wzStringHeader *sh = (void*) (s-sizeof *sh);
    size_t totlen = sh->free+sh->len;

    if (totlen < len) {
        s = wzStringMakeRoomFor(s,len-sh->len);
        if (s == NULL) return NULL;
        sh = (void*) (s-sizeof *sh);;
        totlen = sh->free+sh->len;
    }
    memcpy(s, t, len);
    s[len] = '\0';
    sh->len = len;
    sh->free = totlen-len;
    return s;
}

/* Like wz_string_copy_length() but 't' must be a null-termined string so that the length
 * of the string is obtained with strlen(). */
wzString wz_string_copy(wzString s, const char *t) {
    return wz_string_copy_length(s, t, strlen(t));
}

/* Like wz_string_concatpritf() but gets va_list instead of being variadic. */
wzString wz_string_concat_vprintf(wzString s, const char *fmt, va_list ap) {
    va_list cpy;
    char *buf, *t;
    size_t buflen = 16;

    while(1) {
        buf = malloc(buflen);
        if (buf == NULL) return NULL;
        buf[buflen-2] = '\0';

#ifdef _MSC_VER
		cpy = ap;
#else
        va_copy(cpy,ap);
#endif
        vsnprintf(buf, buflen, fmt, cpy);
        if (buf[buflen-2] != '\0') {
            free(buf);
            buflen *= 2;
            continue;
        }
        break;
    }
    t = wz_string_concat(s, buf);
    free(buf);
    return t;
}

/* Append to the wzString string 's' a string obtained using printf-alike format
 * specifier.
 *
 * After the call, the modified wzString string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call.
 *
 * Example:
 *
 * s = wz_string_empty("Sum is: ");
 * s = wz_string_concat_printf(s,"%d+%d = %d",a,b,a+b).
 *
 * Often you need to create a string from scratch with the printf-alike
 * format. When this is the need, just use wz_string_empty() as the target string:
 *
 * s = wz_string_concat_printf(wz_string_empty(), "... your format ...", args);
 */
wzString wz_string_concat_printf(wzString s, const char *fmt, ...) {
    va_list ap;
    char *t;
    va_start(ap, fmt);
    t = wz_string_concat_vprintf(s,fmt,ap);
    va_end(ap);
    return t;
}

/* Remove the part of the string from left and from right composed just of
 * contiguous characters found in 'cset', that is a null terminted C string.
 *
 * After the call, the modified wzString string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call.
 *
 * Example:
 *
 * s = wz_string_new("AA...AA.a.aa.aHelloWorld     :::");
 * s = wz_string_trim(s,"A. :");
 * printf("%s\n", s);
 *
 * Output will be just "Hello World".
 */
void wz_string_trim(wzString s, const char *cset) {
    char *start, *end, *sp, *ep;
    size_t len;
    struct wzStringHeader *sh = (void*) (s-sizeof *sh);

    sp = start = s;
    ep = end = s+wz_string_length(s)-1;
    while(sp <= end && strchr(cset, *sp)) sp++;
    while(ep > start && strchr(cset, *ep)) ep--;
    len = (sp > ep) ? 0 : ((ep-sp)+1);
    if (sh->buf != sp) memmove(sh->buf, sp, len);
    sh->buf[len] = '\0';
    sh->free = sh->free+(sh->len-len);
    sh->len = len;
}

/* Turn the string into a smaller (or equal) string containing only the
 * substring specified by the 'start' and 'end' indexes.
 *
 * start and end can be negative, where -1 means the last character of the
 * string, -2 the penultimate character, and so forth.
 *
 * The interval is inclusive, so the start and end characters will be part
 * of the resulting string.
 *
 * The string is modified in-place.
 *
 * Example:
 *
 * s = wz_string_new("Hello World");
 * wz_string_range(s,1,-1); => "ello World"
 */
void wz_string_range(wzString s, int start, int end) {
	size_t newlen;
    struct wzStringHeader *sh = (void*) (s-sizeof *sh);
    size_t len = wz_string_length(s);

    if (len == 0) return;
    if (start < 0) {
        start = len+start;
        if (start < 0) start = 0;
    }
    if (end < 0) {
        end = len+end;
        if (end < 0) end = 0;
    }
    newlen = (start > end) ? 0 : (end-start)+1;
    if (newlen != 0) {
        if (start >= (signed)len) {
            newlen = 0;
        } else if (end >= (signed)len) {
            end = len-1;
            newlen = (start > end) ? 0 : (end-start)+1;
        }
    } else {
        start = 0;
    }
    if (start && newlen) memmove(sh->buf, sh->buf+start, newlen);
    sh->buf[newlen] = 0;
    sh->free = sh->free+(sh->len-newlen);
    sh->len = newlen;
}

/* Apply tolower() to every character of the wzString string 's'. */
void wz_string_to_lower(wzString s) {
    int len = wz_string_length(s), j;

    for (j = 0; j < len; j++) s[j] = tolower(s[j]);
}

/* Apply toupper() to every character of the wzString string 's'. */
void wz_string_to_upper(wzString s) {
    int len = wz_string_length(s), j;

    for (j = 0; j < len; j++) s[j] = toupper(s[j]);
}

/* Compare two wzString strings s1 and s2 with memcmp().
 *
 * Return value:
 *
 *     1 if s1 > s2.
 *    -1 if s1 < s2.
 *     0 if s1 and s2 are exactly the same binary string.
 *
 * If two strings share exactly the same prefix, but one of the two has
 * additional characters, the longer string is considered to be greater than
 * the smaller one. */
int wz_string_compare(const wzString s1, const wzString s2) {
    size_t l1, l2, minlen;
    int cmp;

    l1 = wz_string_length(s1);
    l2 = wz_string_length(s2);
    minlen = (l1 < l2) ? l1 : l2;
    cmp = memcmp(s1,s2,minlen);
    if (cmp == 0) return l1-l2;
    return cmp;
}

/* Split 's' with separator in 'sep'. An array
 * of wzString strings is returned. *count will be set
 * by reference to the number of tokens returned.
 *
 * On out of memory, zero length string, zero length
 * separator, NULL is returned.
 *
 * Note that 'sep' is able to split a string using
 * a multi-character separator. For example
 * wzStringsplit("foo_-_bar","_-_"); will return two
 * elements "foo" and "bar".
 *
 * This version of the function is binary-safe but
 * requires length arguments. wzStringsplit() is just the
 * same function but for zero-terminated strings.
 */
wzString *wz_string_split_length(const char *s, int len, const char *sep, int seplen, int *count) {
    int elements = 0, slots = 5, start = 0, j;
    wzString *tokens;

    if (seplen < 1 || len < 0) return NULL;

    tokens = malloc(sizeof(wzString)*slots);
    if (tokens == NULL) return NULL;

    if (len == 0) {
        *count = 0;
        return tokens;
    }
    for (j = 0; j < (len-(seplen-1)); j++) {
        /* make sure there is room for the next element and the final one */
        if (slots < elements+2) {
            wzString *newtokens;

            slots *= 2;
            newtokens = realloc(tokens,sizeof(wzString)*slots);
            if (newtokens == NULL) goto cleanup;
            tokens = newtokens;
        }
        /* search the separator */
        if ((seplen == 1 && *(s+j) == sep[0]) || (memcmp(s+j,sep,seplen) == 0)) {
            tokens[elements] = wz_string_new_length(s+start,j-start);
            if (tokens[elements] == NULL) goto cleanup;
            elements++;
            start = j+seplen;
            j = j+seplen-1; /* skip the separator */
        }
    }
    /* Add the final element. We are sure there is room in the tokens array. */
    tokens[elements] = wz_string_new_length(s+start,len-start);
    if (tokens[elements] == NULL) goto cleanup;
    elements++;
    *count = elements;
    return tokens;

cleanup:
    {
        int i;
        for (i = 0; i < elements; i++) wz_string_free(tokens[i]);
        free(tokens);
        *count = 0;
        return NULL;
    }
}

/* Free the result returned by wz_string_split_length(), or do nothing if 'tokens' is NULL. */
void wz_string_free_split_result(wzString *tokens, int count) {
    if (!tokens) return;
    while(count--)
        wz_string_free(tokens[count]);
    free(tokens);
}

/* Create an wzString string from a long long value. It is much faster than:
 *
 * wz_string_concat_printf(wz_string_empty(),"%lld\n", value);
 */
wzString wz_string_from_longlong(long long value) {
    char buf[32], *p;
    unsigned long long v;

    v = (value < 0) ? -value : value;
    p = buf+31; /* point to the last character */
    do {
        *p-- = '0'+(v%10);
        v /= 10;
    } while(v);
    if (value < 0) *p-- = '-';
    p++;
    return wz_string_new_length(p,32-(p-buf));
}

/* Append to the wzString string "s" an escaped string representation where
 * all the non-printable characters (tested with isprint()) are turned into
 * escapes in the form "\n\r\a...." or "\x<hex-number>".
 *
 * After the call, the modified wzString string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
wzString wz_string_concat_representation(wzString s, const char *p, size_t len) {
    s = wz_string_concat_length(s,"\"",1);
    while(len--) {
        switch(*p) {
        case '\\':
        case '"':
            s = wz_string_concat_printf(s,"\\%c",*p);
            break;
        case '\n': s = wz_string_concat_length(s,"\\n",2); break;
        case '\r': s = wz_string_concat_length(s,"\\r",2); break;
        case '\t': s = wz_string_concat_length(s,"\\t",2); break;
        case '\a': s = wz_string_concat_length(s,"\\a",2); break;
        case '\b': s = wz_string_concat_length(s,"\\b",2); break;
        default:
            if (isprint(*p))
                s = wz_string_concat_printf(s,"%c",*p);
            else
                s = wz_string_concat_printf(s,"\\x%02x",(unsigned char)*p);
            break;
        }
        p++;
    }
    return wz_string_concat_length(s,"\"",1);
}

/* Helper function for wz_string_split_args() that returns non zero if 'c'
 * is a valid hex digit. */
int is_hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

/* Helper function for wz_string_split_args() that converts a hex digit into an
 * integer from 0 to 15 */
int hex_digit_to_int(char c) {
    switch(c) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': case 'A': return 10;
    case 'b': case 'B': return 11;
    case 'c': case 'C': return 12;
    case 'd': case 'D': return 13;
    case 'e': case 'E': return 14;
    case 'f': case 'F': return 15;
    default: return 0;
    }
}

/* Split a line into arguments, where every argument can be in the
 * following programming-language REPL-alike form:
 *
 * foo bar "newline are supported\n" and "\xff\x00otherstuff"
 *
 * The number of arguments is stored into *argc, and an array
 * of wzString is returned.
 *
 * The caller should free the resulting array of wzString strings with
 * wz_string_free_split_result().
 *
 * Note that wz_string_concat_representation() is able to convert back a string into
 * a quoted string in the same format wz_string_split_args() is able to parse.
 *
 * The function returns the allocated tokens on success, even when the
 * input string is empty, or NULL if the input contains unbalanced
 * quotes or closed quotes followed by non space characters
 * as in: "foo"bar or "foo'
 */
wzString *wz_string_split_args(const char *line, int *argc) {
    const char *p = line;
    char *current = NULL;
    char **vector = NULL;

    *argc = 0;
    while(1) {
        /* skip blanks */
        while(*p && isspace(*p)) p++;
        if (*p) {
            /* get a token */
            int inq=0;  /* set to 1 if we are in "quotes" */
            int insq=0; /* set to 1 if we are in 'single quotes' */
            int done=0;

            if (current == NULL) current = wz_string_empty();
            while(!done) {
                if (inq) {
                    if (*p == '\\' && *(p+1) == 'x' &&
                                             is_hex_digit(*(p+2)) &&
                                             is_hex_digit(*(p+3)))
                    {
                        unsigned char byte;

                        byte = (hex_digit_to_int(*(p+2))*16)+
                                hex_digit_to_int(*(p+3));
                        current = wz_string_concat_length(current,(char*)&byte,1);
                        p += 3;
                    } else if (*p == '\\' && *(p+1)) {
                        char c;

                        p++;
                        switch(*p) {
                        case 'n': c = '\n'; break;
                        case 'r': c = '\r'; break;
                        case 't': c = '\t'; break;
                        case 'b': c = '\b'; break;
                        case 'a': c = '\a'; break;
                        default: c = *p; break;
                        }
                        current = wz_string_concat_length(current,&c,1);
                    } else if (*p == '"') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        current = wz_string_concat_length(current,p,1);
                    }
                } else if (insq) {
                    if (*p == '\\' && *(p+1) == '\'') {
                        p++;
                        current = wz_string_concat_length(current,"'",1);
                    } else if (*p == '\'') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        current = wz_string_concat_length(current,p,1);
                    }
                } else {
                    switch(*p) {
                    case ' ':
                    case '\n':
                    case '\r':
                    case '\t':
                    case '\0':
                        done=1;
                        break;
                    case '"':
                        inq=1;
                        break;
                    case '\'':
                        insq=1;
                        break;
                    default:
                        current = wz_string_concat_length(current,p,1);
                        break;
                    }
                }
                if (*p) p++;
            }
            /* add the token to the vector */
            vector = realloc(vector,((*argc)+1)*sizeof(char*));
            vector[*argc] = current;
            (*argc)++;
            current = NULL;
        } else {
            /* Even on empty input string return something not NULL. */
            if (vector == NULL) vector = malloc(sizeof(void*));
            return vector;
        }
    }

err:
    while((*argc)--)
        wz_string_free(vector[*argc]);
    free(vector);
    if (current) wz_string_free(current);
    *argc = 0;
    return NULL;
}

/* Modify the string substituting all the occurrences of the set of
 * characters specified in the 'from' string to the corresponding character
 * in the 'to' array.
 *
 * For instance: wz_string_map_chars(mystring, "ho", "01", 2)
 * will have the effect of turning the string "hello" into "0ell1".
 *
 * The function returns the wzString string pointer, that is always the same
 * as the input pointer since no resize is needed. */
wzString wz_string_map_chars(wzString s, const char *from, const char *to, size_t setlen) {
    size_t j, i, l = wz_string_length(s);

    for (j = 0; j < l; j++) {
        for (i = 0; i < setlen; i++) {
            if (s[j] == from[i]) {
                s[j] = to[i];
                break;
            }
        }
    }
    return s;
}

/* Join an array of C strings using the specified separator (also a C string).
 * Returns the result as an wzString string. */
wzString wz_string_join(char **argv, int argc, char *sep, size_t seplen) {
    wzString join = wz_string_empty();
    int j;

    for (j = 0; j < argc; j++) {
        join = wz_string_concat(join, argv[j]);
        if (j != argc-1) join = wz_string_concat_length(join,sep,seplen);
    }
    return join;
}

/* Like wz_string_join, but joins an array of SDS strings. */
wzString wz_string_join_string(wzString *argv, int argc, const char *sep, size_t seplen) {
    wzString join = wz_string_empty();
    int j;

    for (j = 0; j < argc; j++) {
        join = wz_string_concat_string(join, argv[j]);
        if (j != argc-1) join = wz_string_concat_length(join,sep,seplen);
    }
    return join;
}
