/**
 * @file        dummy.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     3.4.0
 * @date        04-28-2025
 *
 * @details
 * This file is a C++ rewrite of dummy.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */
#include <cstring>   // for strcmp, if needed
#include "dhry.hpp"

#ifndef REG
#define REG // REG becomes defined as empty (i.e. no register variables)
#endif

// External variables (variables defined elsewhere in C++)
extern int Int_Glob;
extern char Ch_1_Glob;

// Function declaration
Enumeration Func_1(Capital_Letter Ch_1_Par_Val, Capital_Letter Ch_2_Par_Val);
bool Func_2(Str_30 Str_1_Par_Ref, Str_30 Str_2_Par_Ref);
bool Func_3(Enumeration Enum_Par_Val);
void Proc_6(Enumeration Enum_Val_Par, Enumeration* Enum_Ref_Par);
void Proc_7(One_Fifty Int_1_Par_Val, One_Fifty Int_2_Par_Val, One_Fifty* Int_Par_Ref);
void Proc_8(Arr_1_Dim Arr_1_Par_Ref, Arr_2_Dim Arr_2_Par_Ref, int Int_1_Par_Val, int Int_2_Par_Val);



// -----------------------------------------------------------------
// Func_1: Execute three
//   First:  Ch_1_Par_Val == 'H', Ch_2_Par_Val == 'R'
//   Second:  Ch_1_Par_Val == 'A', Ch_2_Par_Val == 'C'
//   Third:  Ch_1_Par_Val == 'B', Ch_2_Par_Val == 'C'
// -----------------------------------------------------------------
Enumeration Func_1(Capital_Letter Ch_1_Par_Val, Capital_Letter Ch_2_Par_Val)
{
    Capital_Letter Ch_1_Loc = Ch_1_Par_Val;
    Capital_Letter Ch_2_Loc = Ch_1_Loc;

    if (Ch_2_Loc != Ch_2_Par_Val)
    {
        // then, executed
        return Ident_1;
    }
    else
    {
        // not executed
        Ch_1_Glob = Ch_1_Loc;
        return Ident_2;
    }
}

// -----------------------------------------------------------------
// Func_2: Execute once
//   Input： Str_1_Par_Ref == "DHRYSTONE PROGRAM, 1'ST STRING"
//         Str_2_Par_Ref == "DHRYSTONE PROGRAM, 2'ND STRING"
// -----------------------------------------------------------------
bool Func_2(Str_30 Str_1_Par_Ref, Str_30 Str_2_Par_Ref)
{
    REG One_Thirty Int_Loc = 2;
    Capital_Letter Ch_Loc = 'A';

    // loop body executed once
    while (Int_Loc <= 2)
    {
        if (Func_1(Str_1_Par_Ref[Int_Loc], Str_2_Par_Ref[Int_Loc + 1]) == Ident_1)
        {
            Ch_Loc = 'A';
            Int_Loc += 1;
        }
        else
        {
            break; // Add break to prevent infinite loop, if necessary
        }
    }

    if (Ch_Loc >= 'W' && Ch_Loc < 'Z')
    {
        Int_Loc = 7;
    }

    if (Ch_Loc == 'R')
    {
        return true;
    }
    else
    {
        // executed
        // Compare using std::string's compare (assuming Str_30 is of type std::string)
        if (Str_1_Par_Ref > Str_2_Par_Ref)
        {
            Int_Loc += 7;
            Int_Glob = Int_Loc;
            return true;
        }
        else
        {
            return false;
        }
    }
}

// -----------------------------------------------------------------
// Func_3: Execute once
//   Input： Enum_Par_Val == Ident_3
// -----------------------------------------------------------------
bool Func_3(Enumeration Enum_Par_Val)
{
    // Return the comparison result directly
    return (Enum_Par_Val == Ident_3);
}

// -----------------------------------------------------------------
// Proc_6: Execute once
//   input： Enum_Val_Par == Ident_3
//   output： Enum_Ref_Par becomes Ident_2
// -----------------------------------------------------------------
void Proc_6(Enumeration Enum_Val_Par, Enumeration* Enum_Ref_Par)
{
    *Enum_Ref_Par = Enum_Val_Par;
    if (!Func_3(Enum_Val_Par))  // If the condition is not met, it will not be executed.
        *Enum_Ref_Par = Ident_4;

    switch (Enum_Val_Par)
    {
        case Ident_1:
            *Enum_Ref_Par = Ident_1;
            break;
        case Ident_2:
            if (Int_Glob > 100)
                *Enum_Ref_Par = Ident_1;
            else
                *Enum_Ref_Par = Ident_4;
            break;
        case Ident_3:
            *Enum_Ref_Par = Ident_2;
            break;
        case Ident_4:
            break;
        case Ident_5:
            *Enum_Ref_Par = Ident_3;
            break;
    }
}

// -----------------------------------------------------------------
// Proc_7: Execute three
//   First：  Int_1_Par_Val == 2,  Int_2_Par_Val == 3,  output Int_Par_Ref == 7
//   Second：  Int_1_Par_Val == 10, Int_2_Par_Val == 5,  output Int_Par_Ref == 17
//   Third：  Int_1_Par_Val == 6,  Int_2_Par_Val == 10, output Int_Par_Ref == 18
// -----------------------------------------------------------------
void Proc_7(One_Fifty Int_1_Par_Val, One_Fifty Int_2_Par_Val, One_Fifty* Int_Par_Ref)
{
    One_Fifty Int_Loc = Int_1_Par_Val + 2;
    *Int_Par_Ref = Int_2_Par_Val + Int_Loc;
}

// -----------------------------------------------------------------
// Proc_8: Execute once
//   Input： Int_1_Par_Val == 3, Int_2_Par_Val == 7
// -----------------------------------------------------------------
void Proc_8(Arr_1_Dim Arr_1_Par_Ref, Arr_2_Dim Arr_2_Par_Ref, int Int_1_Par_Val, int Int_2_Par_Val)
{
    REG One_Fifty Int_Index;
    REG One_Fifty Int_Loc;

    Int_Loc = Int_1_Par_Val + 5;
    Arr_1_Par_Ref[Int_Loc] = Int_2_Par_Val;
    Arr_1_Par_Ref[Int_Loc + 1] = Arr_1_Par_Ref[Int_Loc];
    Arr_1_Par_Ref[Int_Loc + 30] = Int_Loc;

    for (Int_Index = Int_Loc; Int_Index <= Int_Loc + 1; ++Int_Index)
        Arr_2_Par_Ref[Int_Loc][Int_Index] = Int_Loc;

    Arr_2_Par_Ref[Int_Loc][Int_Loc - 1] += 1;
    Arr_2_Par_Ref[Int_Loc + 20][Int_Loc] = Arr_1_Par_Ref[Int_Loc];

    Int_Glob = 5;
}