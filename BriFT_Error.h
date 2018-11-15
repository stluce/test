/*
 * This source code is developed by Ivan Mairakov and exclusively owned 
 * by Ivan Mairakov.
 *
 * You should have written permissions from author to use this code.
 * This source code is protected by copyright laws and international 
 * treaty provisions. 
 * All illegal use is prohibited.
 *
 * All rights not expressly granted here are reserved by Ivan Mairakov.
 * All rights reserved Copyright (C)2002 Ivan Mairakov
 * 
 * BriFT - Brick text format
 *
 * Error codes 
 * 
 */

#ifndef _BriFT_ERROR_H__
#define _BriFT_ERROR_H__

/// Error codes 
typedef DWORD BRIFT_RETURN; // type
#define BRIFT_ERROR_OK		 0	// Succefull
#define BRIFT_ERROR_BRICK_EXIST	 1	// Line Brick already exist
#define BRIFT_ERROR_LOWMEMORY	 2	// Can't allocate memory
#define BRIFT_ERROR_FILE	 3	// File I/O
#define BRIFT_ERROR_WRONGFILE	 4	// Wrong file format
#define BRIFT_ERROR_STACK_MEM    5	// BriFT stack overflow
#define BRIFT_ERROR_MAX_LEN	 6	// Name of brick too long
#define BRIFT_ERROR_RECURSE	 7	// Recursive call of brick - fatal
#define BRIFT_ERROR_NOTFUNC	 8	// Can't find function name - not error
#define BRIFT_ERROR_PARAMTOOLONG 9	// function parameter too long - warning
#define BRIFT_ERROR_NOPARAM	10	// No parameters in function - warning
#define BRIFT_ERROR_BRICKS_FILE 11	// $BRICKS can't load brick's file
#define BRIFT_ERROR_FILE_CREATE 12	// Can't create output file
#define BRIFT_ERROR_FILE_DUB	13	// Can't dublicate file
#define BRIFT_ERROR_FILE_OPEN   14	// Can't open input file
#define BRIFT_ERROR_FREEMEM     15	// Can't free memory
#define BRIFT_ERROR_STACKZERO   16	// Nothing to popup: stack is empty
#define BRIFT_ERROR_VITALPARAM  17	// Parameter is vitally necessary - error
#define BRIFT_ERROR_STACKMASK   18	// Internal: Can't get/set stack masks - error
#define BRIFT_ERROR_ELSE	19	// misplaced $else error
#define BRIFT_ERROR_ENDIF	20	// misplaced $endif error
#define BRIFT_ERROR_ELSEIF	21	// misplaced $elseif error
#define BRIFT_ERROR_ELSEIFN	22	// misplaced $elseifn error
#define BRIFT_ERROR_PARSEBRICK	23	// can't parse brick - warning
#define BRIFT_ERROR_COMMAND	24	// possible wrong command in brick file - warning
#define BRIFT_ERROR_MLINECLOSE  25	// multi line is not closed by ']'

#ifdef _BRIFT_STACK_
BOOL BRIFT_PrintError(PBRIFT_STACK Stack, BRIFT_RETURN error); // Print errors - return  FALSE if it's FATAL error
BOOL BRIFT_StackRecursionPrint(PBRIFT_STACK Stack); // Print stack recursion
#endif

// error checking macro

// check error and return
#define BRICK_CHECK_ERROR(stack,error) if (error!=BRIFT_ERROR_OK) if (!BRIFT_PrintError(stack,error)) return error;
// check error and jump to label_out to cleanup
#define BRICK_CHECK_ERROR_CLEAN(stack,error,label_out) if (error!=BRIFT_ERROR_OK) if (!BRIFT_PrintError(stack,error)) goto label_out;
// save error to variable var and jump to label_out for cleanup
#define BRICK_RAISE_ERROR_CLEAN(stack,error,var,label_out) if (!BRIFT_PrintError(stack,error)) {var=error; goto label_out;};
#define BRICK_RETURN_IF_ERROR(r) if (r!=BRIFT_ERROR_OK) return r;

#endif /* _BriFT_ERROR_H__ */
