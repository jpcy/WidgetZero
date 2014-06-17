/* stb-2.23 - Sean's Tool Box -- public domain -- http://nothings.org/stb.h
          no warranty is offered or implied; use this code at your own risk

   This is a single header file with a bunch of useful utilities
   for getting stuff done in C/C++.

   Email bug reports, feature requests, etc. to 'sean' at the same site.


   Documentation: http://nothings.org/stb/stb_h.html
   Unit tests:    http://nothings.org/stb/stb.c
*/
#ifndef _WZ_ARR_H_
#define _WZ_ARR_H_

//////////////////////////////////////////////////////////////////////////////
//
//                                wz_arr
//
//  An wz_arr is directly useable as a pointer (use the actual type in your
//  definition), but when it resizes, it returns a new pointer and you can't
//  use the old one, so you have to be careful to copy-in-out as necessary.
//
//  Use a NULL pointer as a 0-length array.
//
//     float *my_array = NULL, *temp;
//
//     // add elements on the end one at a time
//     wz_arr_push(my_array, 0.0f);
//     wz_arr_push(my_array, 1.0f);
//     wz_arr_push(my_array, 2.0f);
//
//     assert(my_array[1] == 2.0f);
//
//     // add an uninitialized element at the end, then assign it
//     *wz_arr_add(my_array) = 3.0f;
//
//     // add three uninitialized elements at the end
//     temp = wz_arr_addn(my_array,3);
//     temp[0] = 4.0f;
//     temp[1] = 5.0f;
//     temp[2] = 6.0f;
//
//     assert(my_array[5] == 5.0f);
//
//     // remove the last one
//     wz_arr_pop(my_array);
//
//     assert(wz_arr_len(my_array) == 6);


#ifdef STB_MALLOC_WRAPPER
  #define STB__PARAMS    , char *file, int line
  #define STB__ARGS      ,       file,     line
#else
  #define STB__PARAMS
  #define STB__ARGS
#endif

// calling this function allocates an empty wz_arr attached to p
// (whereas NULL isn't attached to anything)
void wz_arr_malloc(void **target, void *context);

// call this function with a non-NULL value to have all successive
// wzs that are created be attached to the associated parent. Note
// that once a given wz_arr is non-empty, it stays attached to its
// current parent, even if you call this function again.
// it turns the previous value, so you can restore it
void* wz_arr_malloc_parent(void *p);

// simple functions written on top of other functions
#define wz_arr_empty(a)       (  wz_arr_len(a) == 0 )
#define wz_arr_add(a)         (  wz_arr_addn((a),1) )
#define wz_arr_push(a,v)      ( *wz_arr_add(a)=(v)  )

typedef struct
{
   int len, limit;
   unsigned int signature;
} wz__arr;

#define wz_arr_signature      0x51bada7b  // ends with 0123 in decimal

// access the header block stored before the data
#define wz_arrhead(a)         /*lint --e(826)*/ (((wz__arr *) (a)) - 1)
#define wz_arrhead2(a)        /*lint --e(826)*/ (((wz__arr *) (a)) - 1)

#ifdef STB_DEBUG
#define wz_arr_check(a)       assert(!a || wz_arrhead(a)->signature == wz_arr_signature)
#define wz_arr_check2(a)      assert(!a || wz_arrhead2(a)->signature == wz_arr_signature)
#else
#define wz_arr_check(a)       0
#define wz_arr_check2(a)      0
#endif

// ARRAY LENGTH

// get the array length; special case if pointer is NULL
#define wz_arr_len(a)         (a ? wz_arrhead(a)->len : 0)
#define wz_arr_len2(a)        ((wz__arr *) (a) ? wz_arrhead2(a)->len : 0)
#define wz_arr_lastn(a)       (wz_arr_len(a)-1)

// check whether a given index is valid -- tests 0 <= i < wz_arr_len(a) 
#define wz_arr_valid(a,i)     (a ? (int) (i) < wz_arrhead(a)->len : 0)

// change the array length so is is exactly N entries long, creating
// uninitialized entries as needed
#define wz_arr_setlen(a,n)  \
            (wz__arr_setlen((void **) &(a), sizeof(a[0]), (n)))

// change the array length so that N is a valid index (that is, so
// it is at least N entries long), creating uninitialized entries as needed
#define wz_arr_makevalid(a,n)  \
            (wz_arr_len(a) < (n)+1 ? wz_arr_setlen(a,(n)+1),(a) : (a))

// remove the last element of the array, returning it
#define wz_arr_pop(a)         ((wz_arr_check(a), (a))[--wz_arrhead(a)->len])

// access the last element in the array
#define wz_arr_last(a)        ((wz_arr_check(a), (a))[wz_arr_len(a)-1])

// is iterator at end of list?
#define wz_arr_end(a,i)       ((i) >= &(a)[wz_arr_len(a)])

// (internal) change the allocated length of the array
#define wz_arr__grow(a,n)     (wz_arr_check(a), wz_arrhead(a)->len += (n))

// add N new unitialized elements to the end of the array
#define wz_arr__addn(a,n)     /*lint --e(826)*/ \
                               ((wz_arr_len(a)+(n) > wz_arrcurmax(a))      \
                                 ? (wz__arr_addlen((void **) &(a),sizeof(*a),(n)),0) \
                                 : ((wz_arr__grow(a,n), 0)))

// add N new unitialized elements to the end of the array, and return
// a pointer to the first new one
#define wz_arr_addn(a,n)      (wz_arr__addn((a),n),(a)+wz_arr_len(a)-(n))

// add N new uninitialized elements starting at index 'i'
#define wz_arr_insertn(a,i,n) (wz__arr_insertn((void **) &(a), sizeof(*a), i, n))

// insert an element at i
#define wz_arr_insert(a,i,v)  (wz__arr_insertn((void **) &(a), sizeof(*a), i, n), ((a)[i] = v))

// delete N elements from the middle starting at index 'i'
#define wz_arr_deleten(a,i,n) (wz__arr_deleten((void **) &(a), sizeof(*a), i, n))

// delete the i'th element
#define wz_arr_delete(a,i)   wz_arr_deleten(a,i,1)

// delete the i'th element, swapping down from the end
#define wz_arr_fastdelete(a,i)  \
   (wz_swap(&a[i], &a[wz_arrhead(a)->len-1], sizeof(*a)), wz_arr_pop(a))


// ARRAY STORAGE

// get the array maximum storage; special case if NULL
#define wz_arrcurmax(a)       (a ? wz_arrhead(a)->limit : 0)
#define wz_arrcurmax2(a)      (a ? wz_arrhead2(a)->limit : 0)

// set the maxlength of the array to n in anticipation of further growth
#define wz_arr_setsize(a,n)   (wz_arr_check(a), wz__arr_setsize((void **) &(a),sizeof((a)[0]),n))

// make sure maxlength is large enough for at least N new allocations
#define wz_arr_atleast(a,n)   (wz_arr_len(a)+(n) > wz_arrcurmax(a)      \
                                 ? wz_arr_setsize((a), (n)) : 0)

// make a copy of a given array (copies contents via 'memcpy'!)
#define wz_arr_copy(a)        wz__arr_copy(a, sizeof((a)[0]))

// compute the storage needed to store all the elements of the array
#define wz_arr_storage(a)     (wz_arr_len(a) * sizeof((a)[0]))

#define wz_arr_for(v,arr)     for((v)=(arr); (v) < (arr)+wz_arr_len(arr); ++(v))

// IMPLEMENTATION

void wz_arr_free_(void **p);
void *wz__arr_copy_(void *p, int elem_size);
void wz__arr_setsize_(void **p, int size, int limit  STB__PARAMS);
void wz__arr_setlen_(void **p, int size, int newlen  STB__PARAMS);
void wz__arr_addlen_(void **p, int size, int addlen  STB__PARAMS);
void wz__arr_deleten_(void **p, int size, int loc, int n  STB__PARAMS);
void wz__arr_insertn_(void **p, int size, int loc, int n  STB__PARAMS);

#define wz_arr_free(p)            wz_arr_free_((void **) &(p))
#define wz__arr_copy              wz__arr_copy_

#ifndef STB_MALLOC_WRAPPER
  #define wz__arr_setsize         wz__arr_setsize_
  #define wz__arr_setlen          wz__arr_setlen_
  #define wz__arr_addlen          wz__arr_addlen_
  #define wz__arr_deleten         wz__arr_deleten_
  #define wz__arr_insertn         wz__arr_insertn_
#else
  #define wz__arr_addlen(p,s,n)    wz__arr_addlen_(p,s,n,__FILE__,__LINE__)
  #define wz__arr_setlen(p,s,n)    wz__arr_setlen_(p,s,n,__FILE__,__LINE__)
  #define wz__arr_setsize(p,s,n)   wz__arr_setsize_(p,s,n,__FILE__,__LINE__)
  #define wz__arr_deleten(p,s,i,n) wz__arr_deleten_(p,s,i,n,__FILE__,__LINE__)
  #define wz__arr_insertn(p,s,i,n) wz__arr_insertn_(p,s,i,n,__FILE__,__LINE__)
#endif

#endif // _WZ_ARR_H_
