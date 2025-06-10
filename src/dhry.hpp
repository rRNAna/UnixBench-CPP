/**
 * @file        dhry.hpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     3.6.0
 * @date        06-10-2025
 *
 * @details
 * This file is a C++ rewrite of dhry.h from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */

#pragma once

// Compiler and system dependent definitions

// Choose time function if not defined
#ifndef TIME
#define TIMES
#endif

#ifdef TIMES
#include <sys/types.h>
#include <sys/times.h>
#endif

#include <cstdio>
#include <cstring>
#include <cstdlib>

// Constants
constexpr double Mic_secs_Per_Second = 1000000.0;  // Berkeley UNIX returns times in seconds/HZ

// Struct assignment macro
#ifdef NOSTRUCTASSIGN
#define structassign(d, s) memcpy(&(d), &(s), sizeof(d))
#else
#define structassign(d, s) d = s
#endif

// Enumeration definitions
#ifdef NOENUM
#define Ident_1 0
#define Ident_2 1
#define Ident_3 2
#define Ident_4 3
#define Ident_5 4
using Enumeration = int;
#else
enum Enumeration {
    Ident_1,
    Ident_2,
    Ident_3,
    Ident_4,
    Ident_5
  };
#endif

// Typedefs
using One_Thirty = int;
using One_Fifty = int;
using Capital_Letter = char;
using Boolean = bool;
using Str_30 = char[31];            // fixed-length char array
using Arr_1_Dim = int[50];
using Arr_2_Dim = int[50][50];

// Forward declare struct for pointer
struct record;

// Alias types
using Rec_Type = record;
using Rec_Pointer = record*;

// Record structure
struct record {
    Rec_Pointer Ptr_Comp;
    Enumeration Discr;
    union {
        struct {
            Enumeration Enum_Comp;
            int Int_Comp;
            char Str_Comp[31];
        } var_1;
        struct {
            Enumeration E_Comp_2;
            char Str_2_Comp[31];
        } var_2;
        struct {
            char Ch_1_Comp;
            char Ch_2_Comp;
        } var_3;
    } variant;
};