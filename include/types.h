#ifndef TYPES_H
#define TYPES_H

/*
 *  types.h --
 */

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

typedef unsigned int    uint32;
typedef unsigned long   uint64;

#if X64
typedef unsigned long   uintp;
#else
typedef unsigned int    uintp;
#endif

typedef unsigned char   uint8;

//  Create the new type definitions 
//  (later we remove the old ones)

typedef unsigned short  ushort_t;
typedef unsigned char   uchar_t;

typedef unsigned int    uint_t;
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
//typedef unsigned long   uint64_t;

typedef uintp pde_t;

#endif
