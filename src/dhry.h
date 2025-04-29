/**
 * @file        dhry.h
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     3.5.0
 * @date        04-28-2025
 *
 * @details
 * This file is a C++ rewrite of dhry.h from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */

/* Compiler and system dependent definitions: */

#ifndef TIME
#define TIMES
#endif
/* Use times(2) time function unless explicitly defined otherwise */

#ifdef TIMES
#include <sys/types.h>
#include <sys/times.h> // for "times"
#endif

constexpr double Mic_secs_Per_Second = 1000000.0;
// Berkeley UNIX C returns process times in seconds/HZ

#ifdef NOSTRUCTASSIGN
#define structassign(d, s) memcpy(&(d), &(s), sizeof(d))
#else
#define structassign(d, s) d = s
#endif

#ifdef NOENUM
#define Ident_1 0
#define Ident_2 1
#define Ident_3 2
#define Ident_4 3
#define Ident_5 4
using Enumeration = int;
#else
enum Enumeration
{
  Ident_1,
  Ident_2,
  Ident_3,
  Ident_4,
  Ident_5
};
#endif
/* for boolean and enumeration types in Ada, Pascal */

/* General definitions: */

#include <cstdio> // for strcpy, strcmp

#define Null NULL
// Value of a Null pointer

using One_Thirty = int;
using One_Fifty = int;
using Capital_Letter = char;
using Boolean = bool;
using Str_30 = char[31]; // Keep the original definition, 31 bytes fixed array (including the terminator)
using Arr_1_Dim = int[50];
using Arr_2_Dim = int[50][50];

struct record
{
  struct record *Ptr_Comp;
  Enumeration Discr;
  union
  {
    struct
    {
      Enumeration Enum_Comp;
      int Int_Comp;
      char Str_Comp[31];
    } var_1;
    struct
    {
      Enumeration E_Comp_2;
      char Str_2_Comp[31];
    } var_2;
    struct
    {
      char Ch_1_Comp;
      char Ch_2_Comp;
    } var_3;
  } variant;
};

// Use using to define structure aliases and pointer types
using Rec_Type = record;
using Rec_Pointer = record *;
