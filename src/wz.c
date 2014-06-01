/*
The MIT License (MIT)

Copyright (c) 2014 Jonathan Young

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
/* stb-2.23 - Sean's Tool Box -- public domain -- http://nothings.org/stb.h
          no warranty is offered or implied; use this code at your own risk

   This is a single header file with a bunch of useful utilities
   for getting stuff done in C/C++.

   Email bug reports, feature requests, etc. to 'sean' at the same site.


   Documentation: http://nothings.org/stb/stb_h.html
   Unit tests:    http://nothings.org/stb/stb.c
*/

/*
wz_intersect_rects is SDL_IntersectRect

  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "wz_internal.h"

bool wz_is_rect_empty(wzRect rect)
{
	return rect.x == 0 && rect.y == 0 && rect.w == 0 && rect.h == 0;
}

bool wz_intersect_rects(wzRect A, wzRect B, wzRect *result)
{
    int Amin, Amax, Bmin, Bmax;

	assert(result);

    /* Special cases for empty rects */
    if (wz_is_rect_empty(A) || wz_is_rect_empty(B))
	{
		result->x = result->y = result->w = result->h = 0;
        return false;
    }

    /* Horizontal intersection */
    Amin = A.x;
    Amax = Amin + A.w;
    Bmin = B.x;
    Bmax = Bmin + B.w;
    if (Bmin > Amin)
        Amin = Bmin;
    result->x = Amin;
    if (Bmax < Amax)
        Amax = Bmax;
    result->w = Amax - Amin;

    /* Vertical intersection */
    Amin = A.y;
    Amax = Amin + A.h;
    Bmin = B.y;
    Bmax = Bmin + B.h;
    if (Bmin > Amin)
        Amin = Bmin;
    result->y = Amin;
    if (Bmax < Amax)
        Amax = Bmax;
    result->h = Amax - Amin;

    return !wz_is_rect_empty(*result);
}

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
