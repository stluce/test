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
 * BriFT command line interface
 *
 */

#include <windows.h>
#include <stdio.h>
#include "BriFT.h"
#include "Console_BuildNumber.H"

#define CONSOLE_VERSION "2.0."_BUILD_NUMBER_SYM_1

LPSTR strHelp=	"Usage:\n" \
		"BriFT [-bBrickFile] [-D\"Brick=Value\"] [-BD] [-c[+-CJH]] InputFile [OutFile]\n\n" \
		"\t-bFILENAME - Load FILENAME as brick file\n"\
		"\t-BD - always load Default.brk\n"\
		"\t-D\"BRICK=TEXT\" - define a single brick\n"\
		"\t-h - show this text\n" \
		"  Comments options:\n"\
		"\t-c+ - keep all comments\n"\
		"\t-c- - remove any kind of comments(default)\n"\
		"\t-cC - /* allow css-like comments */\n"\
		"\t-cJ - // allow js-like comments\n"\
		"\t-cH - <!-- allow html-like comments -->\n";



LPSTR strWelcomeHeader=
	"BriFT - Brick Formated Text console "CONSOLE_VERSION". BriFT core version "BRIFT_CORE_VERSION"\n" \
	"Copyright (C)&(P) 2002-2017 by Ivan Mayrakov. All rights reserved.\n\n";


LPSTR strDefaultInputFile="Default.btf";
LPSTR strDefaultBrickFile="Default.brk";

extern BOOL isCcomment;
extern BOOL isCPPcomment;
extern BOOL isHTMLcomment;


int main(int argc, char **argv)
{
LPSTR InFile;
LPSTR OutFile;
LPSTR BrickFile;
int i;
int r;
int stage=0;
BOOL isDefBrk;
isDefBrk=FALSE;

BRICKLIST lb;

//printf(strWelcomeHeader);

InFile=strDefaultInputFile;
OutFile=NULL;
BrickFile=strDefaultBrickFile;
BRICKLIST_Init(&lb);

//for(i=0;i<argc;i++) printf("%s\n",argv[i]);

/*
for(i=1;i<argc;i++) 
	{
	if (argv[i][0]=='-' || argv[i][0]=='/')
		{
		switch (argv[i][1])
			{
		case 'b': // Set FILENAME as brick file
			BrickFile=&argv[i][2];
			BRICKLIST_LoadFile(&lb,&argv[i][2]);
			break;
		case 'B': // always load Default.brk
			isDefBrk=argv[i][2]=='D';
			break;
		case 'D':
	                BRICKLIST_AddParsed(&lb,&argv[i][2]);
			break;
		case 'h':
			 printf(strWelcomeHeader);
			 printf(strHelp); return 0;
			break;
			}
		} else 
		{
		if (stage==0)
			{
			InFile=argv[i];stage++;
			} else
			{OutFile=argv[i];
			}
		}
	}
*/


if (argc==1)
	{
	 printf(strWelcomeHeader);
	 printf(strHelp); return 0;
	}

// Check Help
for(i=1;i<argc;i++) 
	{
	if (argv[i][0]=='-' || argv[i][0]=='/')
		if (argv[i][1]=='h' || argv[i][1]=='?')
			{
			 printf(strWelcomeHeader);
			 printf(strHelp); 
			 return 0;
			} else 
		if (argv[i][1]=='c')
			{
			switch (argv[i][2])
				{
				case 'C':isCcomment=FALSE; break;
				case 'J':isCPPcomment=FALSE; break;
				case 'H':isHTMLcomment=FALSE; break;
				case '+':isCcomment=FALSE;
					isCPPcomment=FALSE;
					isHTMLcomment=FALSE; 
					break;
				case '-':isCcomment=TRUE;
					isCPPcomment=TRUE;
					isHTMLcomment=TRUE;   break;
				}
			}
	}

for(i=1;i<argc;i++) // Load default brick file
	{
	if (argv[i][0]=='-' || argv[i][0]=='/')
		{
		if (argv[i][1]=='B')
			if (argv[i][2]=='D')
				{
				if (!isDefBrk)
					{
					BRICKLIST_LoadFile(&lb,strDefaultBrickFile);
					isDefBrk=TRUE;
					}
				}
		} else
		{
		if (stage==0)
			{
			InFile=argv[i];stage++;
			} else
			{OutFile=argv[i];
			}
		}

	}

for(i=1;i<argc;i++) // Load custom brick file
	{
	if (argv[i][0]=='-' || argv[i][0]=='/')
		{
		if (argv[i][1]=='b')
			{
			BrickFile=&argv[i][2];
			BRICKLIST_LoadFile(&lb,&argv[i][2]);
			}
		}
	}

if (BrickFile==strDefaultBrickFile)
	{
	if (!isDefBrk) BRICKLIST_LoadFile(&lb,strDefaultBrickFile);
	};

for(i=1;i<argc;i++) // parse command line bricks defs
	{
	if (argv[i][0]=='-' || argv[i][0]=='/')
		if (argv[i][1]=='D')
	                BRICKLIST_AddParsed(&lb,&argv[i][2]);
	}

r=BriFT_ProcessFile(OutFile,InFile,&lb);
BRICKLIST_Destroy(&lb);
return r;
}



