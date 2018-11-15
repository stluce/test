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
 * All rights reserved Copyright (C)2002-2017 Ivan Mairakov
 * 
 * BriFT - Brick text format
 *
 * I/O stack
 * 
 */


#ifndef _BriFT_STACK_H__
#define _BriFT_STACK_H__

#include "BriFT_Error.h"
#include "BriFT_Brick.h"
#include "FileIO.h"

#define BRIFT_STACK_SIZE 1024

// Input objects stack
typedef struct _BRIFT_STACK_ITEM
{
DWORD Type;		// Type of object
LPSTR sName;		// Name of object
LPVOID Object;		// Pointer to object
} BRIFT_STACK_ITEM, *PBRIFT_STACK_ITEM, *LPBRIFT_STACK_ITEM;

// Stack objects items
#define BRIFT_STACK_STR   0   // ASCIIZ      - line brick
#define BRIFT_STACK_FILE  1   // ASCII File  - file brick
#define BRIFT_STACK_CHAR  2   // Char - char brick
#define BRIFT_STACK_BRICK 3   // Line brick - named string brick object (obsolete)
#define BRIFT_STACK_STRSAFE 4 // Allocated ASCIIZ string
#define BRIFT_STACK_MASK 5 // 32-bit mask

#ifndef _BRIFT_STACK_
#define _BRIFT_STACK_ 
typedef struct _BRIFT_STACK
{
DWORD SP; // Stack pointer
HANDLE Heap; 	   // Heap
DWORD dwAllocated; //  Allocated memory
LPSTR sLastFileName; // Last FileName
DWORD dwLastLine;    // Last Line
BRIFT_STACK_ITEM *pItems; // Stack memory
BOOL isPopLock;
} BRIFT_STACK, *PBRIFT_STACK, *LPBRIFT_STACK;
#endif

typedef struct _FILE_ITEM
{
//BOOL isEOF;
DWORD dwPopCount;
BYTEFILE bf;
} FILE_ITEM,*PFILE_ITEM,*LPFILE_ITEM;

typedef struct _STRSAFE_ITEM
{
LPBYTE sStr;
DWORD dwPopCount;
//BYTE arr[];
} STRSAFE_ITEM,*PSTRSAFE_ITEM,*LPSTRSAFE_ITEM;

// Stack Functions
BRIFT_RETURN BRIFT_STACK_Create(BRIFT_STACK *Stack);   // Constuctor
BRIFT_RETURN BRIFT_STACK_Destroy(BRIFT_STACK *Stack); // Destructor
BRIFT_RETURN BRIFT_STACKITEM_Destroy(BRIFT_STACK_ITEM *Item); // Destructor of item

#define IS_STACKEMPTY(Stack) ((Stack)->SP==0)

// Push input flows
BRIFT_RETURN BRIFT_STACK_CheckRecursion(BRIFT_STACK *Stack); // check recursion
BRIFT_RETURN BRIFT_STACK_Push(BRIFT_STACK *Stack,BRIFT_STACK_ITEM *Item); // Push item
BRIFT_RETURN BRIFT_STACK_PushFile(BRIFT_STACK *Stack,LPSTR File);// Push File stream
#define BRIFT_STACK_PushStr(Stack,Str) BRIFT_STACK_PushNamedStr(Stack,NULL,Str) // Push string stream
BRIFT_RETURN BRIFT_STACK_PushNamedStr(BRIFT_STACK *Stack,LPSTR sName,LPSTR Str); // Push named string stream
BRIFT_RETURN BRIFT_STACK_PushBrick(BRIFT_STACK *Stack,LPSTR sName,LPSTR sData); // Push brick stream
BRIFT_RETURN BRIFT_STACK_PushChar(BRIFT_STACK *Stack,BYTE ch);   // Push char to stream
BRIFT_RETURN BRIFT_STACK_PushNamedStrSafe(BRIFT_STACK *Stack,LPSTR sName,LPSTR Str); // Allocate space and push named string to stream
#define BRIFT_STACK_PushStrSafe(Stack,Str) BRIFT_STACK_PushNamedStrSafe(Stack,NULL,Str) // Allocate space and push string to stream

// Work with masks
BRIFT_RETURN BRIFT_STACK_PushMask(BRIFT_STACK *Stack,LPSTR Str,DWORD dwMask); // Push named 32bit mask
DWORD BRIFT_STACK_GetCurrentMask(BRIFT_STACK *Stack); // Get current mask
BRIFT_RETURN BRIFT_STACK_SetCurrentMask(BRIFT_STACK *Stack,DWORD dwMask); // Set current mask
DWORD BRIFT_STACK_GetMaskSummery(BRIFT_STACK *Stack); // Get mask summery

// pop up
BRIFT_RETURN BRIFT_STACK_Pop(BRIFT_STACK *Stack);
BRIFT_RETURN BRIFT_STACK_RemoveAt(BRIFT_STACK *Stack,DWORD dwPosition); // remove at position

// Get information
DWORD BRIFT_STACK_GetLine(BRIFT_STACK *Stack); // Get current line
LPSTR BRIFT_STACK_GetFileName(BRIFT_STACK *Stack); // Get current file name
BRIFT_RETURN BRIFT_STACK_RememberPosition(BRIFT_STACK *Stack); // remember parsing line and file
DWORD BRIFT_STACK_GetActualLine(BRIFT_STACK *Stack); // Get parsing line

// Stack Char Operations
int BRIFT_STACK_GetChar(BRIFT_STACK *Stack); // return char from stack

BRIFT_RETURN BRIFT_STACK_DropEndLine(BRIFT_STACK *Stack); // Remove \r\n

// debug
BRIFT_RETURN BRIFT_STACK_Print(BRIFT_STACK *Stack);


/*
typedef struct _REQ_NAME
{
LPSTR sName;
DWORD Type;
} REQ_NAME,*PREQ_NAME,*LPREQ_NAME;

typedef struct _REQ_CHECK
{
LPSTR sName;
DWORD Type;
} REQ_CHECK,*PREQ_CHECK,*LPREQ_CHECK;
*/


#endif /* _BriFT_STACK_H__ */