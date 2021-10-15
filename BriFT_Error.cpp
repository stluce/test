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
 * Error codes divided to 'fatal' and 'warnings'
 * 
 */

#include <stdio.h>
#include "BriFT.h"

//#define _OUT_STDOUT_

#ifdef _OUT_STDOUT_
#define OUTSTREAM stdout
#else
#define OUTSTREAM stderr
#endif


typedef struct _BRIFT_ERROR_TYPE
{
BRIFT_RETURN r;
LPSTR Description;
BOOL IsContinue;
} BRIFT_ERROR_TYPE,*PBRIFT_ERROR_TYPE,*LPBRIFT_ERROR_TYPE;


BRIFT_ERROR_TYPE BRIFT_ErrorTable[]=
{
{
BRIFT_ERROR_OK,"No error",TRUE
},
{
BRIFT_ERROR_BRICK_EXIST,"Brick already exist",TRUE
},
{
BRIFT_ERROR_LOWMEMORY,"Can't allocate memory",FALSE
},
{
BRIFT_ERROR_FILE,"File I/O",FALSE
},
{
BRIFT_ERROR_WRONGFILE,"Wrong file format",TRUE
},
{
BRIFT_ERROR_STACK_MEM,"BriFT out stack overflow",FALSE
},
{
BRIFT_ERROR_MAX_LEN,"Name of brick too long, brick skipped",TRUE
},
{
BRIFT_ERROR_RECURSE,"Found recurse",FALSE
},
{
BRIFT_ERROR_NOTFUNC,"Can't find function name",TRUE
},
{
BRIFT_ERROR_PARAMTOOLONG,"Function parameter too long",TRUE
},
{
BRIFT_ERROR_NOPARAM,"Function need parameter",TRUE
},
{
BRIFT_ERROR_BRICKS_FILE,"$BRICKS can't load bricks file",TRUE
},
{
BRIFT_ERROR_FILE_CREATE,"Can't create output file",FALSE
},
{
BRIFT_ERROR_FILE_OPEN,"Can't open input file",FALSE
},
{
BRIFT_ERROR_FREEMEM,"Can't free memory",TRUE
},
{
BRIFT_ERROR_STACKZERO,"Internal: nothing to popup - stack is empty",FALSE
},
{
BRIFT_ERROR_VITALPARAM,"Parameter is really necessary",FALSE
},
{
BRIFT_ERROR_STACKMASK,"Internal: Can't get or set masks to stack",FALSE
},
{
BRIFT_ERROR_ELSE,"misplaced $ELSE",FALSE
},
{
BRIFT_ERROR_ENDIF,"misplaced $ENDIF",FALSE
},
{
BRIFT_ERROR_ELSEIF,"misplaced $ELSEIF",FALSE
},
{
BRIFT_ERROR_ELSEIFN,"misplaced $ENDIFN",FALSE
},
{
BRIFT_ERROR_PARSEBRICK,"can't parse brick",TRUE
},
{
BRIFT_ERROR_COMMAND,"a wrong command in brick file",TRUE
},
{
BRIFT_ERROR_MLINECLOSE,"multi line is not closed by ']'",FALSE
},
{
0,NULL,0
}
};

BOOL BRIFT_StackRecursionPrint(PBRIFT_STACK Stack)
{
int Nb;
int  i;
DWORD SP;
DWORD SP_B;
LPSTR stype;
BRIFT_STACK_ITEM *pItems;
SP=1;SP_B=Stack->SP;
pItems=Stack->pItems;
Nb=0;
fprintf(OUTSTREAM,"Recursion is:\n");
while (SP<=SP_B) 
{
	switch(pItems[SP].Type)
	{
	case BRIFT_STACK_STR:
             stype="brick $";break;
	case BRIFT_STACK_FILE:
             stype="file: ";break;
	case BRIFT_STACK_CHAR:
             stype=NULL;break;
	case BRIFT_STACK_BRICK:
             stype=NULL;break;
	case BRIFT_STACK_STRSAFE:
             stype="brick $";break;
	case BRIFT_STACK_MASK:
             stype=NULL;break;
	default: stype=NULL;
	}	

if (stype!=NULL && pItems[SP].sName!=NULL)
	{
	fprintf(OUTSTREAM,"|");
	
	for (i=0;i<Nb;i++) fprintf(OUTSTREAM,"-");Nb++;
	fprintf(OUTSTREAM,"-> %s%s",stype,pItems[SP].sName);
	if (pItems[SP].Type==pItems[SP_B].Type &&  lstrcmpi(pItems[SP].sName,pItems[SP_B].sName)==0) fprintf(OUTSTREAM," <-");
	fprintf(OUTSTREAM,"\n");
	}
SP++;
}
return TRUE;
}

/// Print errors - return  FALSE if it's FATAL error
BOOL BRIFT_PrintError(PBRIFT_STACK Stack, BRIFT_RETURN error) 
{
PBRIFT_ERROR_TYPE err;
LPSTR FileName;

if (error==BRIFT_ERROR_OK) return TRUE;

err=(PBRIFT_ERROR_TYPE) &BRIFT_ErrorTable;

while (err->r!=error && (err->Description!=NULL)) err++;

if (err->r!=error) return FALSE;

//printf(
fprintf(OUTSTREAM,(err->IsContinue)? "Warning ": "Error ");

if (Stack!=NULL)
{
FileName=BRIFT_STACK_GetFileName(Stack);
if (FileName==NULL) fprintf(OUTSTREAM,"Unknow file:");
	else 
	{
	fprintf(OUTSTREAM,"file '%s' line %i: ",FileName,BRIFT_STACK_GetActualLine(Stack)+1);
	}
} else fprintf(OUTSTREAM,": ");

fprintf(OUTSTREAM,"%s\n",err->Description);
if (error==BRIFT_ERROR_RECURSE)
	BRIFT_StackRecursionPrint(Stack);

return err->IsContinue;
}
