/* stb-2.23 - Sean's Tool Box -- public domain -- http://nothings.org/stb.h
          no warranty is offered or implied; use this code at your own risk

   This is a single header file with a bunch of useful utilities
   for getting stuff done in C/C++.

   Email bug reports, feature requests, etc. to 'sean' at the same site.


   Documentation: http://nothings.org/stb/stb_h.html
   Unit tests:    http://nothings.org/stb/stb.c
*/
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <wz.h>
#include "wz_arr.h"

static void * wz__arr_malloc(int size)
{
   return malloc(size);
}

void * wz__arr_copy_(void *p, int elem_size)
{
   wz__arr *q;
   if (p == NULL) return p;
   q = (wz__arr *) wz__arr_malloc(sizeof(*q) + elem_size * wz_arrhead2(p)->limit);
   wz_arr_check2(p);
   memcpy(q, wz_arrhead2(p), sizeof(*q) + elem_size * wz_arrhead2(p)->len);
   return q+1;
}

void wz_arr_free_(void **pp)
{
   void *p = *pp;
   wz_arr_check2(p);
   if (p) {
      wz__arr *q = wz_arrhead2(p);
      free(q);
   }
   *pp = NULL;
}

static void wz__arrsize_(void **pp, int size, int limit, int len  STB__PARAMS)
{
   void *p = *pp;
   wz__arr *a;
   wz_arr_check2(p);
   if (p == NULL) {
      if (len == 0 && size == 0) return;
      a = (wz__arr *) wz__arr_malloc(sizeof(*a) + size*limit);
      a->limit = limit;
      a->len   = len;
      a->signature = wz_arr_signature;
   } else {
      a = wz_arrhead2(p);
      a->len = len;
      if (a->limit < limit) {
         void *p;
         if (a->limit >= 4 && limit < a->limit * 2)
            limit = a->limit * 2;
         #ifdef STB_MALLOC_WRAPPER
         p = wz__realloc(a, sizeof(*a) + limit*size, file, line);
         #else
         p = realloc(a, sizeof(*a) + limit*size);
         #endif
         if (p) {
            a = (wz__arr *) p;
            a->limit = limit;
         } else {
            // throw an error!
         }
      }
   }
   a->len   = WZ_MIN(a->len, a->limit);
   *pp = a+1;
}

void wz__arr_setsize_(void **pp, int size, int limit  STB__PARAMS)
{
   void *p = *pp;
   wz_arr_check2(p);
   wz__arrsize_(pp, size, limit, wz_arr_len2(p)  STB__ARGS);
}

void wz__arr_setlen_(void **pp, int size, int newlen  STB__PARAMS)
{
   void *p = *pp;
   wz_arr_check2(p);
   if (wz_arrcurmax2(p) < newlen || p == NULL) {
      wz__arrsize_(pp, size, newlen, newlen  STB__ARGS);
   } else {
      wz_arrhead2(p)->len = newlen;
   }
}

void wz__arr_addlen_(void **p, int size, int addlen  STB__PARAMS)
{
   wz__arr_setlen_(p, size, wz_arr_len2(*p) + addlen  STB__ARGS);
}

void wz__arr_insertn_(void **pp, int size, int i, int n  STB__PARAMS)
{
   void *p = *pp;
   if (n) {
      int z;

      if (p == NULL) {
         wz__arr_addlen_(pp, size, n  STB__ARGS);
         return;
      }

      z = wz_arr_len2(p);
      wz__arr_addlen_(&p, size, i  STB__ARGS);
      memmove((char *) p + (i+n)*size, (char *) p + i*size, size * (z-i));
   }
   *pp = p;
}

void wz__arr_deleten_(void **pp, int size, int i, int n  STB__PARAMS)
{
   void *p = *pp;
   if (n) {
      memmove((char *) p + i*size, (char *) p + (i+n)*size, size * (wz_arr_len2(p)-i));
      wz_arrhead2(p)->len -= n;
   }
   *pp = p;
}