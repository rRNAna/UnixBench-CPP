/**
 * @file        dhry.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     3.5.0
 * @date        04-28-2025
 *
 * @details
 * This file is a C++ rewrite of dhry_1.c and dhry_2.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <array>
#include "dhry.h"
#include "timeit.cpp"

using namespace std;

unsigned long Run_Index;

void report(int sig)
{
    cerr << "COUNT|" << Run_Index << "|1|lps" << endl;
    exit(0);
}

/* Global Variables: */

Rec_Pointer Ptr_Glob,
            Next_Ptr_Glob;
int Int_Glob;
bool Bool_Glob;
char Ch_1_Glob,
     Ch_2_Glob;
array<int, 50> Arr_1_Glob;
array<array<int, 50>, 50> Arr_2_Glob;

Enumeration Func_1(Capital_Letter Ch_1_Par_Val, Capital_Letter Ch_2_Par_Val);
  /* forward declaration necessary since Enumeration may not simply be int */


/* variables for time measurement: */

#ifdef TIMES
struct tms time_info;
extern int times ();
                /* see library function "times" */
#define Too_Small_Time 120
                /* Measurements should last at least about 2 seconds */
#endif
#ifdef TIME
extern long time();
                /* see library function "time"  */
#define Too_Small_Time 2
                /* Measurements should last at least 2 seconds */
#endif

long Begin_Time,
     End_Time,
     User_Time;
float Microseconds,
      Dhrystones_Per_Second;

/* end of variables for time measurement */

void Proc_1 (Rec_Pointer Ptr_Val_Par);
void Proc_2 (One_Fifty *Int_Par_Ref);
void Proc_3 (Rec_Pointer *Ptr_Ref_Par);
void Proc_4 (void);
void Proc_5 (void);

extern bool Func_2(const std::string& Str_1_Par_Ref, const std::string& Str_2_Par_Ref);
extern void Proc_6(Enumeration, Enumeration *);
extern void Proc_7(One_Fifty, One_Fifty, One_Fifty *);
//extern void Proc_8(Arr_1_Dim, Arr_2_Dim, int, int);
extern void Proc_8(std::array<int, 50>& Arr_1_Par_Ref,
            std::array<std::array<int, 50>, 50>& Arr_2_Par_Ref,
            int Int_1_Par_Val, int Int_2_Par_Val);

int main (int argc, char *argv[])
  /* main program, corresponds to procedures        */
  /* Main and Proc_0 in the Ada version             */
{
    int duration;
    One_Fifty Int_1_Loc;
    One_Fifty Int_2_Loc;
    One_Fifty Int_3_Loc;
    char Ch_Index;
    Enumeration Enum_Loc;
    string Str_1_Loc;
    string Str_2_Loc;

    /* Initializations */

    Next_Ptr_Glob = new Rec_Type;
    Ptr_Glob = new Rec_Type;

    Ptr_Glob->Ptr_Comp = Next_Ptr_Glob;
    Ptr_Glob->Discr = Ident_1;
    Ptr_Glob->variant.var_1.Enum_Comp = Ident_3;
    Ptr_Glob->variant.var_1.Int_Comp = 40;
    strcpy(Ptr_Glob->variant.var_1.Str_Comp, "DHRYSTONE PROGRAM, SOME STRING");
    Str_1_Loc = "DHRYSTONE PROGRAM, 1'ST STRING";

    Arr_2_Glob[8][7] = 10;
        /* Was missing in published program. Without this statement,    */
        /* Arr_2_Glob [8][7] would have an undefined value.             */
        /* Warning: With 16-Bit processors and Number_Of_Runs > 32000,  */
        /* overflow may occur for this array element.                   */



    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " duration" << endl;
        exit(1);
    }

    duration = atoi(argv[1]);
    Run_Index = 0;
    wake_me(duration, report);

    /***************/
    /* Start timer */
    /***************/

#ifdef SELF_TIMED
#ifdef TIMES
    times (&time_info);
    Begin_Time = (long) time_info.tms_utime;
#endif
#ifdef TIME
    Begin_Time = time ((long *) 0);
#endif
#endif /* SELF_TIMED */

    for (Run_Index = 1; ; ++Run_Index)
    {

        Proc_5();
        Proc_4();
        /* Ch_1_Glob == 'A', Ch_2_Glob == 'B', Bool_Glob == true */
        Int_1_Loc = 2;
        Int_2_Loc = 3;
        Str_2_Loc = "DHRYSTONE PROGRAM, 2'ND STRING";
        Enum_Loc = Ident_2;
        Bool_Glob = ! Func_2 (Str_1_Loc, Str_2_Loc);
        /* Bool_Glob == 1 */
        while (Int_1_Loc < Int_2_Loc)  /* loop body executed once */
        {
            Int_3_Loc = 5 * Int_1_Loc - Int_2_Loc;
            /* Int_3_Loc == 7 */
            Proc_7 (Int_1_Loc, Int_2_Loc, &Int_3_Loc);
            /* Int_3_Loc == 7 */
            Int_1_Loc += 1;
        } /* while */
        /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
        Proc_8 (Arr_1_Glob, Arr_2_Glob, Int_1_Loc, Int_3_Loc);
//        Proc_8 (std::array<int, 50>& Arr_1_Par_Ref,
//            std::array<std::array<int, 50>, 50>& Arr_2_Par_Ref,
//            int Int_1_Par_Val, int Int_2_Par_Val);
        /* Int_Glob == 5 */
        Proc_1 (Ptr_Glob);
        for (Ch_Index = 'A'; Ch_Index <= Ch_2_Glob; ++Ch_Index)
                             /* loop body executed twice */
        {
            if (Enum_Loc == Func_1 (Ch_Index, 'C'))
            /* then, not executed */
            {
                Proc_6 (Ident_1, &Enum_Loc);
                Str_2_Loc = "DHRYSTONE PROGRAM, 3'RD STRING";
                Int_2_Loc = Run_Index;
                Int_Glob = Run_Index;
            }
        }
        /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
        Int_2_Loc = Int_2_Loc * Int_1_Loc;
        Int_1_Loc = Int_2_Loc / Int_3_Loc;
        Int_2_Loc = 7 * (Int_2_Loc - Int_3_Loc) - Int_1_Loc;
        /* Int_1_Loc == 1, Int_2_Loc == 13, Int_3_Loc == 7 */
        Proc_2 (&Int_1_Loc);
        /* Int_1_Loc == 5 */

    } /* loop "for Run_Index" */

    /**************/
    /* Stop timer */
    /**************/
#ifdef SELF_TIMED
#ifdef TIMES
    times (&time_info);
    End_Time = (long) time_info.tms_utime;
#endif
#ifdef TIME
    End_Time = time ((long *) 0);
#endif
#endif /* SELF_TIMED */

    /* BYTE version never executes this stuff */
#ifdef SELF_TIMED
    cout << "Execution ends\n";
    cout << "\n";
    cout << "Final values of the variables used in the benchmark:\n";
    cout << "\n";
    cout << "Int_Glob:            " << Int_Glob << endl;
    cout << "        should be:   " << 5 << endl;
    cout << "Bool_Glob:           " << Bool_Glob << endl;
    cout << "        should be:   " << 1 << endl;
    cout << "Ch_1_Glob:           " << Ch_1_Glob << endl;
    cout << "        should be:   " << 'A' << endl;
    cout << "Ch_2_Glob:           " << Ch_2_Glob << endl;
    cout << "        should be:   " << 'B' << endl;
    cout << "Arr_1_Glob[8]:       " << Arr_1_Glob[8] << endl;
    cout << "        should be:   " << 7 << endl;
    cout << "Arr_2_Glob[8][7]:    " << Arr_2_Glob[8][7] << endl;
    cout << "        should be:   Number_Of_Runs + 10" << endl;
    cout << "Ptr_Glob->\n";
    cout << "  Ptr_Comp:          " << (int) Ptr_Glob->Ptr_Comp << endl;
    cout << "        should be:   (implementation-dependent)\n";
    cout << "  Discr:             " << Ptr_Glob->Discr << endl;
    cout << "        should be:   " << 0 << endl;
    cout << "  Enum_Comp:         " << Ptr_Glob->variant.var_1.Enum_Comp << endl;
    cout << "        should be:   " << 2 << endl;
    cout << "  Int_Comp:          " << Ptr_Glob->variant.var_1.Int_Comp << endl;
    cout << "        should be:   " << 17 << endl;
    cout << "  Str_Comp:          " << Ptr_Glob->variant.var_1.Str_Comp << endl;
    cout << "        should be:   DHRYSTONE PROGRAM, SOME STRING" << endl;
    cout << "Next_Ptr_Glob->\n";
    cout << "  Ptr_Comp:          " << (int) Next_Ptr_Glob->Ptr_Comp << endl;
    cout << "        should be:   (implementation-dependent), same as above" << endl;
    cout << "  Discr:             " << Next_Ptr_Glob->Discr << endl;
    cout << "        should be:   " << 0 << endl;
    cout << "  Enum_Comp:         " << Next_Ptr_Glob->variant.var_1.Enum_Comp << endl;
    cout << "        should be:   " << 1 << endl;
    cout << "  Int_Comp:          " << Next_Ptr_Glob->variant.var_1.Int_Comp << endl;
    cout << "        should be:   " << 18 << endl;
    cout << "  Str_Comp:          " << Next_Ptr_Glob->variant.var_1.Str_Comp << endl;
    cout << "        should be:   DHRYSTONE PROGRAM, SOME STRING" << endl;
    cout << "Int_1_Loc:           " << Int_1_Loc << endl;
    cout << "        should be:   " << 5 << endl;
    cout << "Int_2_Loc:           " << Int_2_Loc << endl;
    cout << "        should be:   " << 13 << endl;
    cout << "Int_3_Loc:           " << Int_3_Loc << endl;
    cout << "        should be:   " << 7 << endl;
    cout << "Enum_Loc:            " << Enum_Loc << endl;
    cout << "        should be:   " << 1 << endl;
    cout << "Str_1_Loc:           " << Str_1_Loc << endl;
    cout << "        should be:   DHRYSTONE PROGRAM, 1'ST STRING" << endl;
    cout << "Str_2_Loc:           " << Str_2_Loc << endl;
    cout << "        should be:   DHRYSTONE PROGRAM, 2'ND STRING" << endl;
    cout << "\n";

    User_Time = End_Time - Begin_Time;

    if (User_Time < Too_Small_Time)
    {
        cout << "Measured time too small to obtain meaningful results" << endl;
        cout << "Please increase number of runs" << endl;
        cout << "\n";
    }
    else
    {
#ifdef TIME
        Microseconds = (float) User_Time * Mic_secs_Per_Second
                        / (float) Number_Of_Runs;
        Dhrystones_Per_Second = (float) Number_Of_Runs / (float) User_Time;
#else
        Microseconds = (float) User_Time * Mic_secs_Per_Second
                        / ((float) HZ * ((float) Number_Of_Runs));
        Dhrystones_Per_Second = ((float) HZ * (float) Number_Of_Runs)
                        / (float) User_Time;
#endif
        cout << "Microseconds for one run through Dhrystone: ";
        cout << Microseconds << endl;
        cout << "Dhrystones per Second:                      ";
        cout << Dhrystones_Per_Second << endl;
        cout << "\n";
    }
#endif /* SELF_TIMED */
}


void Proc_1 (Rec_Pointer Ptr_Val_Par)
    /* executed once */
{
    Rec_Pointer Next_Record = Ptr_Val_Par->Ptr_Comp;
                                        /* == Ptr_Glob_Next */
    /* Local variable, initialized with Ptr_Val_Par->Ptr_Comp,    */
    /* corresponds to "rename" in Ada, "with" in Pascal           */

    structassign (*Ptr_Val_Par->Ptr_Comp, *Ptr_Glob);
    Ptr_Val_Par->variant.var_1.Int_Comp = 5;
    Next_Record->variant.var_1.Int_Comp
        = Ptr_Val_Par->variant.var_1.Int_Comp;
    Next_Record->Ptr_Comp = Ptr_Val_Par->Ptr_Comp;
    Proc_3 (&Next_Record->Ptr_Comp);
    /* Ptr_Val_Par->Ptr_Comp->Ptr_Comp
                        == Ptr_Glob->Ptr_Comp */
    if (Next_Record->Discr == Ident_1)
    /* then, executed */
    {
        Next_Record->variant.var_1.Int_Comp = 6;
        Proc_6 (Ptr_Val_Par->variant.var_1.Enum_Comp,
           &Next_Record->variant.var_1.Enum_Comp);
        Next_Record->Ptr_Comp = Ptr_Glob->Ptr_Comp;
        Proc_7 (Next_Record->variant.var_1.Int_Comp, 10,
           &Next_Record->variant.var_1.Int_Comp);
    }
    else /* not executed */
        structassign (*Ptr_Val_Par, *Ptr_Val_Par->Ptr_Comp);
} /* Proc_1 */


void Proc_2 (One_Fifty *Int_Par_Ref)
    /* executed once */
    /* *Int_Par_Ref == 1, becomes 4 */
{
    One_Fifty Int_Loc;
    Enumeration Enum_Loc;

    Enum_Loc = Ident_1;

    Int_Loc = *Int_Par_Ref + 10;
    do /* executed once */
        if (Ch_1_Glob == 'A')
        /* then, executed */
        {
            Int_Loc -= 1;
            *Int_Par_Ref = Int_Loc - Int_Glob;
            Enum_Loc = Ident_1;
        } /* if */
    while (Enum_Loc != Ident_1); /* true */
} /* Proc_2 */


void Proc_3 (Rec_Pointer *Ptr_Ref_Par)
    /* executed once */
    /* Ptr_Ref_Par becomes Ptr_Glob */
{
    if (Ptr_Glob != Null)
    /* then, executed */
        *Ptr_Ref_Par = Ptr_Glob->Ptr_Comp;
    Proc_7 (10, Int_Glob, &Ptr_Glob->variant.var_1.Int_Comp);
} /* Proc_3 */


void Proc_4 () {

    bool Bool_Loc;

    Bool_Loc = Ch_1_Glob == 'A';
    Bool_Glob = Bool_Loc | Bool_Glob;
    Ch_2_Glob = 'B';
} 

void Proc_5 (void) {
    Ch_1_Glob = 'A';
    Bool_Glob = false;
}


extern int Int_Glob;
extern char Ch_1_Glob;

void Proc_6(Enumeration Enum_Val_Par, Enumeration* Enum_Ref_Par);
void Proc_7(One_Fifty Int_1_Par_Val, One_Fifty Int_2_Par_Val, One_Fifty* Int_Par_Ref);
// void Proc_8(Arr_1_Dim Arr_1_Par_Ref, Arr_2_Dim Arr_2_Par_Ref, int Int_1_Par_Val, int Int_2_Par_Val);
Enumeration Func_1(Capital_Letter Ch_1_Par_Val, Capital_Letter Ch_2_Par_Val);
bool Func_2(const std::string& Str_1_Par_Ref, const std::string& Str_2_Par_Ref);
bool Func_3(Enumeration Enum_Par_Val);

void Proc_6(Enumeration Enum_Val_Par, Enumeration* Enum_Ref_Par)
{
    // executed once
    // Enum_Val_Par == Ident_3, Enum_Ref_Par becomes Ident_2
    *Enum_Ref_Par = Enum_Val_Par;
    if (!Func_3(Enum_Val_Par))
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

void Proc_7(One_Fifty Int_1_Par_Val, One_Fifty Int_2_Par_Val, One_Fifty* Int_Par_Ref)
{
    // executed three times
    // first call:      Int_1_Par_Val == 2,  Int_2_Par_Val == 3,  Int_Par_Ref becomes 7
    // second call:     Int_1_Par_Val == 10, Int_2_Par_Val == 5,  Int_Par_Ref becomes 17
    // third call:      Int_1_Par_Val == 6,  Int_2_Par_Val == 10, Int_Par_Ref becomes 18
    One_Fifty Int_Loc = Int_1_Par_Val + 2;
    *Int_Par_Ref = Int_2_Par_Val + Int_Loc;
}

// -----------------------------------------------------------------
// Proc_8: 执行一次
//   输入： Int_1_Par_Val == 3, Int_2_Par_Val == 7
// -----------------------------------------------------------------
void Proc_8(std::array<int, 50>& Arr_1_Par_Ref,
    std::array<std::array<int, 50>, 50>& Arr_2_Par_Ref,
    int Int_1_Par_Val, int Int_2_Par_Val) {

        int Int_Index;
        int Int_Loc;

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

Enumeration Func_1(Capital_Letter Ch_1_Par_Val, Capital_Letter Ch_2_Par_Val)
{
    // executed three times
    // first call:  Ch_1_Par_Val == 'H', Ch_2_Par_Val == 'R'
    // second call: Ch_1_Par_Val == 'A', Ch_2_Par_Val == 'C'
    // third call:  Ch_1_Par_Val == 'B', Ch_2_Par_Val == 'C'
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

bool Func_2(const std::string& Str_1_Par_Ref, const std::string& Str_2_Par_Ref)
{
    // executed once
    // Str_1_Par_Ref == "DHRYSTONE PROGRAM, 1'ST STRING"
    // Str_2_Par_Ref == "DHRYSTONE PROGRAM, 2'ND STRING"
    One_Thirty Int_Loc = 2;
    Capital_Letter Ch_Loc = 'A';
    while (Int_Loc <= 2) // loop body executed once
    {
        if (Func_1(Str_1_Par_Ref[Int_Loc], Str_2_Par_Ref[Int_Loc + 1]) == Ident_1)
        {
            // then, executed
            Ch_Loc = 'A';
            Int_Loc += 1;
        }
    }
    if (Ch_Loc >= 'W' && Ch_Loc < 'Z')
        // then, not executed
        Int_Loc = 7;
    if (Ch_Loc == 'R')
        // then, not executed
        return true;
    else
    {
        // executed
        if (Str_1_Par_Ref > Str_2_Par_Ref) // Modern C++ string comparison
        {
            Int_Loc += 7;
            Int_Glob = Int_Loc;
            return true;
        }
        else
            return false;
    }
}

bool Func_3(Enumeration Enum_Par_Val) 
{
    // executed once
    // Enum_Par_Val == Ident_3
    Enumeration Enum_Loc = Enum_Par_Val;

    if (Enum_Loc == Ident_3) 
    {
        // then, executed
        return true;
    } 
    else 
    {
        // not executed
        return false;
    }
}