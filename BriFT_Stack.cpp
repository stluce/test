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
 * TODO: Filename and line tracking have bad design
 *  delayied remove item - need to add counter to all item ???
 *
 */

#define _DEBUG_
//#define _TEST_

#include <windows.h>
#include "BriFT.h"
#include "FileIO.h"
#include "BriFT_Error.h"
#include "BriFT_Stack.h"
#include "BriFT_Extern.h"

#if defined (_TEST_) || defined (_DEBUG_)
#include <stdio.h>
#endif

/// Constuctor
BRIFT_RETURN BRIFT_STACK_Create(BRIFT_STACK *Stack)
{ 
Stack->dwAllocated=BRIFT_STACK_SIZE*sizeof(BRIFT_STACK_ITEM);
Stack->Heap=HeapCreate(0,Stack->dwAllocated,0);
if (Stack->Heap==NULL) return BRIFT_ERROR_LOWMEMORY; // error 
Stack->pItems=(BRIFT_STACK_ITEM *)HeapAlloc(Stack->Heap,HEAP_ZERO_MEMORY,Stack->dwAllocated);
if (Stack->pItems==NULL) return BRIFT_ERROR_LOWMEMORY; // error
Stack->SP=0;
Stack->sLastFileName=NULL; // last names
Stack->dwLastLine=(DWORD)(-1);
Stack->isPopLock=FALSE;
return BRIFT_ERROR_OK;
};

/// Destructor
BRIFT_RETURN BRIFT_STACK_Destroy(BRIFT_STACK *Stack)
{
do {BRIFT_STACK_Pop(Stack);} while (Stack->SP>0);
if (Stack->pItems!=NULL) HeapFree(Stack->Heap,0,Stack->pItems);
if (Stack->Heap!=NULL) HeapDestroy(Stack->Heap);
if (Stack->sLastFileName!=NULL) LocalFree(Stack->sLastFileName); // last names
return BRIFT_ERROR_OK;
}

/// Destructor of item
BRIFT_RETURN BRIFT_STACKITEM_Destroy(BRIFT_STACK_ITEM *Item)
{

//if (!LocalFree(Item->sName)) return BRIFT_ERROR_FREEMEM; // LocalFree should check NULL names
if (Item->sName!=NULL) LocalFree(Item->sName);
Item->sName=NULL;
switch (Item->Type)
	{
	case BRIFT_STACK_STR:
	case BRIFT_STACK_CHAR:
	case BRIFT_STACK_BRICK:
	case BRIFT_STACK_MASK:
		break;
	case BRIFT_STACK_STRSAFE:
		{
		LocalFree(Item->Object);
		//if (!LocalFree(Item->Object)) return BRIFT_ERROR_FREEMEM;
		}
		break;
	case BRIFT_STACK_FILE:
		{
		FILE_ITEM *pFI;
		pFI=(FILE_ITEM *)Item->Object;
		if (!BYTEFILE_Close((BYTEFILE*)&pFI->bf)) return BRIFT_ERROR_FREEMEM;
		//if (!BYTEFILE_Close((BYTEFILE*)Item->Object)) return BRIFT_ERROR_FREEMEM;
		LocalFree(Item->Object);
		//if (!LocalFree(Item->Object)) return BRIFT_ERROR_FREEMEM;
		}
		break;
	}
return BRIFT_ERROR_OK;
}


/// Alloc and add name of item
BRIFT_RETURN BRIFT_STACKITEM_SetName(BRIFT_STACK_ITEM *Item,LPSTR sName)
{
INT l;
if (sName==NULL)
	{
	Item->sName=NULL;
	return BRIFT_ERROR_OK;
	}
l=lstrlen(sName);
Item->sName=(LPSTR)LocalAlloc(LPTR,l+1);
if (Item->sName==NULL) return BRIFT_ERROR_LOWMEMORY;
lstrcpyn(Item->sName,sName,l+1);
return BRIFT_ERROR_OK;
}

/// check and realloc memory for items
BRIFT_RETURN BRIFT_STACK_CheckMemory(BRIFT_STACK *Stack)
{
BRIFT_RETURN r;
if ((Stack->SP+1)*sizeof(BRIFT_STACK_ITEM)<Stack->dwAllocated) return BRIFT_ERROR_OK;
Stack->dwAllocated+=BRIFT_STACK_SIZE*sizeof(BRIFT_STACK_ITEM);
Stack->pItems=(BRIFT_STACK_ITEM*)HeapReAlloc(Stack->Heap,HEAP_ZERO_MEMORY,Stack->pItems,Stack->dwAllocated);
return (Stack->pItems!=NULL)?BRIFT_ERROR_OK:BRIFT_ERROR_LOWMEMORY;
}

#ifdef _DEBUG_
/// Show Stack
BRIFT_RETURN BRIFT_STACK_Print(BRIFT_STACK *Stack)
{
DWORD SP;
LPSTR stype;
BRIFT_STACK_ITEM *pItems;
SP=Stack->SP;
pItems=Stack->pItems;
printf("-\n");
while (SP>0) 
	{
switch(pItems[SP].Type)
	{
	case BRIFT_STACK_STR:
             stype="str";break;
	case BRIFT_STACK_FILE:
             stype="file";break;
	case BRIFT_STACK_CHAR:
             stype="char";break;
	case BRIFT_STACK_BRICK:
             stype="brick";break;
	case BRIFT_STACK_STRSAFE:
             stype="strsafe";break;
	case BRIFT_STACK_MASK:
             stype="mask";break;
	default: stype="???";
	};
	printf("(%i) [%s]:%s\n",SP,stype,pItems[SP].sName);
	SP--;
	}
}
#endif // _DEBUG_

/// Check recursion
BRIFT_RETURN BRIFT_STACK_CheckRecursion(BRIFT_STACK *Stack)
{
DWORD SP;
DWORD Last;
DWORD LastType;

//BRIFT_STACK_Print(Stack);

BRIFT_STACK_ITEM *pItems;
Last=Stack->SP;
pItems=Stack->pItems;
if (pItems[Last].Type==BRIFT_STACK_CHAR || pItems[Last].Type==BRIFT_STACK_MASK)
	return BRIFT_ERROR_OK;
if (pItems[Last].sName==NULL) return BRIFT_ERROR_OK;
LastType=pItems[Last].Type;
/*
if (LastType==BRIFT_STACK_FILE)
	{
	if (lstrcmpi(pItems[Last].sName,Stack->sLastFileName)==0)
		{ 
		  return BRIFT_ERROR_RECURSE;
		}
	}*/
SP=Last-1;
while (SP>0) 
	{
	if (((LastType==BRIFT_STACK_FILE) && (pItems[SP].Type==LastType))
		&& pItems[SP].sName!=NULL)
		{
		if (lstrcmpi(pItems[Last].sName,pItems[SP].sName)==0)
			{ // recursion detected
			  return BRIFT_ERROR_RECURSE;
			}
		}
	if ( (LastType==BRIFT_STACK_STRSAFE || LastType==BRIFT_STACK_STR) &&
        	(pItems[SP].Type==BRIFT_STACK_STRSAFE || pItems[SP].Type==BRIFT_STACK_STR) )
		{
		if (lstrcmpi(pItems[Last].sName,pItems[SP].sName)==0)
			{ // recursion detected
			  return BRIFT_ERROR_RECURSE;
			}
		}
	SP--;
	}
return BRIFT_ERROR_OK;
}

/// Push item
BRIFT_RETURN BRIFT_STACK_Push(BRIFT_STACK *Stack,BRIFT_STACK_ITEM *Item)
{
BRIFT_RETURN r;
r=BRIFT_STACK_CheckMemory(Stack);
if (r!=BRIFT_ERROR_OK) return r;
Stack->SP++;
CopyMemory(&Stack->pItems[Stack->SP],Item,sizeof(BRIFT_STACK_ITEM));
return BRIFT_STACK_CheckRecursion(Stack);
}

/// Pop item
BRIFT_RETURN BRIFT_STACK_Pop(BRIFT_STACK *Stack)
{
BRIFT_RETURN r;
if (Stack->SP==0) return BRIFT_ERROR_STACKZERO;
/*
if (Stack->pItems[Stack->SP].Type==BRIFT_STACK_FILE)
	{ // save last filename and line if BRIFT_STACK_FILE is last
	DWORD SP;
	BOOL isFile;
	SP=Stack->SP-1;
	isFile=TRUE;
	while (isFile && SP>0) {if (Stack->pItems[SP].Type==BRIFT_STACK_FILE) isFile=FALSE; SP--; }
	if (isFile)
		{
		if (Stack->sLastFileName!=NULL) LocalFree(Stack->sLastFileName);
		int l;
		l=lstrlen(Stack->pItems[Stack->SP].sName);
		Stack->sLastFileName=LocalAlloc(LPTR,l+10);
		lstrcpyn(Stack->sLastFileName,Stack->pItems[Stack->SP].sName,l+1);
		Stack->dwLastLine=((BYTEFILE*)Stack->pItems[Stack->SP].Object)->Line;
		}
	}
*/
r=BRIFT_STACKITEM_Destroy(Stack->pItems+Stack->SP);
Stack->SP--;
return r;
}

/// remove at position
BRIFT_RETURN BRIFT_STACK_RemoveAt(BRIFT_STACK *Stack,DWORD dwPosition)
{
DWORD SP;
SP=Stack->SP;
if (dwPosition>SP) return BRIFT_ERROR_OK;
BRIFT_STACKITEM_Destroy(Stack->pItems+dwPosition);
if (dwPosition!=SP)
	CopyMemory(&Stack->pItems[dwPosition],&Stack->pItems[dwPosition+1],sizeof(BRIFT_STACK_ITEM)*(Stack->SP-dwPosition));
Stack->SP--;
return BRIFT_ERROR_OK;
}

/// Push char to stream
BRIFT_RETURN BRIFT_STACK_PushChar(BRIFT_STACK *Stack,BYTE ch)
{
BRIFT_RETURN r;
r=BRIFT_STACK_CheckMemory(Stack);
if (r!=BRIFT_ERROR_OK) return r;
Stack->SP++;
Stack->pItems[Stack->SP].Type=BRIFT_STACK_CHAR;
Stack->pItems[Stack->SP].sName=NULL;
Stack->pItems[Stack->SP].Object=(LPVOID)ch;
return BRIFT_ERROR_OK;
}

/// Push named asciiZ string stream pushed string should not modified
BRIFT_RETURN BRIFT_STACK_PushNamedStr(BRIFT_STACK *Stack,LPSTR sName,LPSTR Str)
{
BRIFT_RETURN r;
r=BRIFT_STACK_CheckMemory(Stack);
if (r!=BRIFT_ERROR_OK) return r;
Stack->SP++;
Stack->pItems[Stack->SP].Type=BRIFT_STACK_STR;
Stack->pItems[Stack->SP].Object=Str;
r=BRIFT_STACKITEM_SetName(Stack->pItems+Stack->SP,sName);
if (r!=BRIFT_ERROR_OK) return r;
return BRIFT_STACK_CheckRecursion(Stack);
}  

/// Allocate space and push string to stream
//  Alloc dinamic struct like
//  typedef StrSafe
//    {
//	LPSTR *ppStrOutPoint=&BufferForString[]; // for out
//	BYTE BufferForString[lstrlen(str)]
//    }
//
BRIFT_RETURN BRIFT_STACK_PushNamedStrSafe(BRIFT_STACK *Stack,LPSTR sName,LPSTR Str)
{
BRIFT_RETURN r;
LPSTR stmp;
STRSAFE_ITEM *pStrSafe;
int l;
r=BRIFT_STACK_CheckMemory(Stack);
if (r!=BRIFT_ERROR_OK) return r;
Stack->SP++;
l=lstrlen(Str);
Stack->pItems[Stack->SP].Type=BRIFT_STACK_STRSAFE;
Stack->pItems[Stack->SP].Object=LocalAlloc(LPTR,l+1+sizeof(STRSAFE_ITEM));
if (Stack->pItems[Stack->SP].Object==NULL) return BRIFT_ERROR_LOWMEMORY;
pStrSafe=(STRSAFE_ITEM *)Stack->pItems[Stack->SP].Object;
pStrSafe->sStr=(LPBYTE)pStrSafe+sizeof(STRSAFE_ITEM);
//(*((LPSTR*)Stack->pItems[Stack->SP].Object))=(LPSTR)Stack->pItems[Stack->SP].Object+sizeof(LPSTR*);
//lstrcpyn(*((LPSTR*)Stack->pItems[Stack->SP].Object),Str,l+1);
lstrcpyn(pStrSafe->sStr,Str,l+1);
//if (sName!=NULL)
r=BRIFT_STACKITEM_SetName(Stack->pItems+Stack->SP,sName); 
BRICK_RETURN_IF_ERROR(r);
//	else r=BRIFT_ERROR_OK;
return BRIFT_STACK_CheckRecursion(Stack);
}

/// Push File stream
BRIFT_RETURN BRIFT_STACK_PushFile(BRIFT_STACK *Stack,LPSTR File)
{
BRIFT_RETURN r;
FILE_ITEM *pFI;

r=BRIFT_STACK_CheckMemory(Stack);
if (r!=BRIFT_ERROR_OK) return r;
Stack->SP++;
r=BRIFT_STACKITEM_SetName(Stack->pItems+Stack->SP,File);
if (r!=BRIFT_ERROR_OK) return r;
Stack->pItems[Stack->SP].Type=BRIFT_STACK_FILE;

//Stack->pItems[Stack->SP].Object=(BYTEFILE *)LocalAlloc(LMEM_FIXED,sizeof(BYTEFILE)+1);
Stack->pItems[Stack->SP].Object=(FILE_ITEM *)LocalAlloc(LMEM_FIXED,sizeof(FILE_ITEM)+1);
if (Stack->pItems[Stack->SP].Object==NULL) return BRIFT_ERROR_LOWMEMORY;

pFI=(FILE_ITEM *)Stack->pItems[Stack->SP].Object;
//pFI->isEOF=FALSE;
pFI->dwPopCount=0;

if (!BYTEFILE_Init(&pFI->bf,File,READONLY))
	return BRIFT_ERROR_FILE;
return BRIFT_STACK_CheckRecursion(Stack);
}

/// return char from stack
int BRIFT_STACK_GetChar(BRIFT_STACK *Stack)
{
INT r;
FILE_ITEM *pFI;
DWORD SP;
SP=Stack->SP;
TryNextLevel:
if (SP==0) return EOF;
//printf("x");
switch(Stack->pItems[SP].Type)
	{
	case BRIFT_STACK_FILE:
		pFI=(FILE_ITEM *)Stack->pItems[SP].Object;
		if (pFI->dwPopCount!=0)
			{
			if (pFI->dwPopCount==1)
				{
				BRIFT_STACK_RemoveAt(Stack,SP);
				return BRIFT_STACK_GetChar(Stack);
				//BRIFT_STACK_Pop(Stack);
				//return BRIFT_STACK_GetChar(Stack);
				} else
				{
                                pFI->dwPopCount--;
				SP--; goto TryNextLevel;
				}
			}
		//r=BYTEFILE_GetChar((BYTEFILE*)Stack->pItems[Stack->SP].Object);
		r=BYTEFILE_GetChar(&pFI->bf);
		if (r==EOF) {
		pFI->dwPopCount=4;
		SP--; goto TryNextLevel;
		//return EOF; 
		//BRIFT_STACK_Pop(Stack);
		//return BRIFT_STACK_GetChar(Stack);
		}; //else return (BYTE)r;
break;
	case BRIFT_STACK_STRSAFE:
		STRSAFE_ITEM *pStrSafe;
		pStrSafe=(STRSAFE_ITEM *)Stack->pItems[SP].Object;
		if (pStrSafe->dwPopCount!=0)
			{
			if (pStrSafe->dwPopCount==1)
				{
				BRIFT_STACK_RemoveAt(Stack,SP);
				return BRIFT_STACK_GetChar(Stack);
				} else
				{
                                pStrSafe->dwPopCount--;
				SP--; goto TryNextLevel;
				}
			}
		r=*pStrSafe->sStr;
		if (r==0)
			{
			//BRIFT_STACK_RemoveAt(Stack,SP);
			pStrSafe->dwPopCount=3;
			SP--;
			goto TryNextLevel;
			}
		pStrSafe->sStr++;
/*
		r=(BYTE)*( *((LPSTR*)Stack->pItems[SP].Object) );
		  //(BYTE)*((*((LPSTR*)Stack->pItems[SP].Object)));
		if (r==0)
		{
		//BRIFT_STACK_Pop(Stack);
		BRIFT_STACK_RemoveAt(Stack,SP);
		//if (SP==Stack->SP) 
		//if (SP==1) SP--;
		SP--;
		goto TryNextLevel;
		//return BRIFT_STACK_GetChar(Stack);
		}
		(*((LPSTR*)Stack->pItems[SP].Object))++;
*/
		//return (BYTE)r;
break;
	case BRIFT_STACK_STR:
		r=(BYTE)*((LPSTR)Stack->pItems[SP].Object);
		if (r==0) 
		{
		//BRIFT_STACK_Pop(Stack);
		BRIFT_STACK_RemoveAt(Stack,SP);
		//if (SP==1) SP--;
		SP--;
		goto TryNextLevel;
		//return BRIFT_STACK_GetChar(Stack);
		};// else return (BYTE)
		((LPSTR)Stack->pItems[SP].Object)++;
break;
	case BRIFT_STACK_CHAR:
		r=(BYTE)Stack->pItems[SP].Object;
		BRIFT_STACK_RemoveAt(Stack,SP);
//	        BRIFT_STACK_Pop(Stack);
//		return (BYTE)r;
break;
	}
return r;
}

/// Get current line
DWORD BRIFT_STACK_GetLine(BRIFT_STACK *Stack)
{
DWORD SP;
SP=Stack->SP;
while (Stack->pItems[SP].Type!=BRIFT_STACK_FILE && SP>0) SP--;
if (SP<=0) return Stack->dwLastLine; //(DWORD)(-1);
return ((FILE_ITEM*)Stack->pItems[SP].Object)->bf.Line;
}

/// remember parsing line and file
BRIFT_RETURN BRIFT_STACK_RememberPosition(BRIFT_STACK *Stack)
{
Stack->dwLastLine=BRIFT_STACK_GetLine(Stack);
DWORD SP;
SP=Stack->SP;
while (Stack->pItems[SP].Type!=BRIFT_STACK_FILE && SP>0) SP--;
if (SP>0)
	{
	if (lstrcmpi(Stack->sLastFileName,Stack->pItems[SP].sName)!=0)
		{
		if (Stack->sLastFileName!=NULL) LocalFree(Stack->sLastFileName);
		int l;
		l=lstrlen(Stack->pItems[SP].sName);
		Stack->sLastFileName=(LPSTR)LocalAlloc(LPTR,l+10);
		if (Stack->sLastFileName==NULL) return BRIFT_ERROR_LOWMEMORY;
		lstrcpyn(Stack->sLastFileName,Stack->pItems[SP].sName,l+1);
		}
	}
return BRIFT_ERROR_OK;
}

/// Get parsing line
DWORD BRIFT_STACK_GetActualLine(BRIFT_STACK *Stack)
{
if (Stack->dwLastLine!=(DWORD)(-1)) return Stack->dwLastLine;
return BRIFT_STACK_GetLine(Stack);
}

/// Get current file name
LPSTR BRIFT_STACK_GetFileName(BRIFT_STACK *Stack)
{
DWORD SP;
SP=Stack->SP;
if (Stack->sLastFileName!=NULL) return Stack->sLastFileName;
while (Stack->pItems[SP].Type!=BRIFT_STACK_FILE && SP>0) SP--;
if (SP<=0) return NULL; //(Stack->sLastFileName==NULL)?(LPSTR)(-1):Stack->sLastFileName;
return Stack->pItems[SP].sName;
}

/*
/// Get current file name
LPSTR BRIFT_STACK_GetFileName(BRIFT_STACK *Stack)
{
DWORD SP;
SP=Stack->SP;
while (Stack->pItems[SP].Type!=BRIFT_STACK_FILE && SP>0) SP--;
if (SP<=0) return Stack->sLastFileName;
return Stack->pItems[SP].sName;
}
*/

/// Push named 32bit mask
BRIFT_RETURN BRIFT_STACK_PushMask(BRIFT_STACK *Stack,LPSTR Str,DWORD dwMask)
{
BRIFT_RETURN r;
r=BRIFT_STACK_CheckMemory(Stack);
if (r!=BRIFT_ERROR_OK) return r;
Stack->SP++;
r=BRIFT_STACKITEM_SetName(Stack->pItems+Stack->SP,Str); 
if (r!=BRIFT_ERROR_OK) return r;
Stack->pItems[Stack->SP].Type=BRIFT_STACK_MASK;
Stack->pItems[Stack->SP].Object=(LPVOID)dwMask;
return BRIFT_ERROR_OK;
} 

/// Get current mask
DWORD BRIFT_STACK_GetCurrentMask(BRIFT_STACK *Stack)
{
if (Stack->SP==0) return (DWORD) (-1);
if (Stack->pItems[Stack->SP].Type!=BRIFT_STACK_MASK) return (DWORD) (-1);
return (DWORD)Stack->pItems[Stack->SP].Object;
}

/// Set current mask
BRIFT_RETURN BRIFT_STACK_SetCurrentMask(BRIFT_STACK *Stack,DWORD dwMask)
{
if (Stack->pItems[Stack->SP].Type!=BRIFT_STACK_MASK) return BRIFT_ERROR_STACKMASK;
Stack->pItems[Stack->SP].Object=(LPVOID)dwMask;
return BRIFT_ERROR_OK;
}

/// Get mask summery
DWORD BRIFT_STACK_GetMaskSummery(BRIFT_STACK *Stack)
{
DWORD SP;
DWORD dwSum;
SP=Stack->SP;
dwSum=0;
if (SP==0) return 0;
while (SP!=0)
	{
	if (Stack->pItems[SP].Type!=BRIFT_STACK_MASK) return (DWORD)(-1);
	dwSum|=(DWORD)Stack->pItems[SP].Object;
	SP--;
	}
return dwSum;
}

/// Remove \r\n
BRIFT_RETURN BRIFT_STACK_DropEndLine(BRIFT_STACK *Stack)
{
int c1,c2;
BRIFT_RETURN err;

c2=BRIFT_STACK_GetChar(Stack);
switch (c2)
{
case '\n': break;
case '\r':
	c1=BRIFT_STACK_GetChar(Stack);
	if (c1!='\n')
		{
		err=BRIFT_STACK_PushChar(Stack,(BYTE)c1);
		BRICK_CHECK_ERROR(Stack,err);
		} else break;
default:
return BRIFT_STACK_PushChar(Stack,c2);
}
return BRIFT_ERROR_OK;
}



#ifdef _TEST_
main()
{
int i;
BYTE c;
BRIFT_STACK s;
BRIFT_STACK_ITEM Item;
BRIFT_STACK_Create(&s);
/*
for (i=0,c=0;i<10000;i++) 
	{
	if (c==0x7) c++;
	if (c==0x0a) c++;
	BRIFT_STACK_PushChar(&s,c++);
	//Item.Type=BRIFT_STACK_CHAR;
	//Item.sName=NULL;
	//Item.Object=(LPVOID)c;
	//BRIFT_STACK_Push(&s,&Item);
	}
*/
//BRIFT_STACK_PushStr(&s,"Abracadabra");
//BRIFT_STACK_PushNamedStrSafe(&s,NULL,"Abracadabra");
BRIFT_STACK_PushFile(&s,"ByteFileAppend.test");
//printf("%s:%d\n\n",BRIFT_STACK_GetFileName(&s),BRIFT_STACK_GetLine(&s));
//BRIFT_STACK_PushChar(&s,'_');
while ((i=BRIFT_STACK_GetChar(&s))!=EOF)
	{
	putchar(i);
	}
//printf("%s:%d\n\n",BRIFT_STACK_GetFileName(&s),BRIFT_STACK_GetLine(&s));

}
#endif // _TEST_