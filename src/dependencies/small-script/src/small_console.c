/* Since some of these routines go further than those of standard C,
 * they cannot always be implemented with portable C functions. In
 * other words, these routines must be ported to other environments.
 *
 *  Copyright (c) ITB CompuPhase, 1997-2002
 *  This file may be freely used. No warranties of any kind.
 *
 *  Version: $Id: Amxcons.c 2382 2003-10-19 14:12:30Z thiadmer $
 */
/* Van Buren console formatter, adapted from amxcons.c.
 * F3 adaptations: game buffer callbacks; only %,c,d,f,s format cases.
 * The consumer provides F3_DebugPutChar and F3_DebugPrintf.
 * See README.F3.md for provenance and integration.
 */
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include "small_console.h"

static int printstring(AMX *amx,cell *cstr,cell *params,int num);

static int dochar(AMX *amx,char ch,cell param)
{
  cell *cptr;

  switch (ch) {
  case '%':
    F3_DebugPutChar(amx,ch);
    return 0;
  case 'c':
    amx_GetAddr(amx,param,&cptr);
    F3_DebugPutChar(amx,(int)*cptr);
    return 1;
  case 'd':
    amx_GetAddr(amx,param,&cptr);
    F3_DebugPrintf(amx,"%ld",(long)*cptr);
    return 1;
  case 'f': /* Original F3 exposes f, without the upstream r alias. */
    amx_GetAddr(amx,param,&cptr);
    F3_DebugPrintf(amx,"%f",*(float*)cptr);
    return 1;
  case 's':
    amx_GetAddr(amx,param,&cptr);
    printstring(amx,cptr,NULL,0);
    return 1;
  } /* switch */
  /* error in the string format, try to repair */
  F3_DebugPutChar(amx,ch);
  return 0;
}

static int printstring(AMX *amx,cell *cstr,cell *params,int num)
{
  int i;
  int informat=0,paramidx=0;

  /* check whether this is a packed string */
  if ((ucell)*cstr>UCHAR_MAX) {
    int j=sizeof(cell)-sizeof(char);
    char c;
    /* the string is packed */
    i=0;
    for ( ; ; ) {
      c=(char)((ucell)cstr[i] >> 8*j);
      if (c==0)
        break;
      if (informat) {
        assert(params!=NULL);
        paramidx+=dochar(amx,c,params[paramidx]);
        informat=0;
      } else if (params!=NULL && c=='%') {
        informat=1;
      } else {
        F3_DebugPutChar(amx,c);
      } /* if */
      if (j==0)
        i++;
      j=(j+sizeof(cell)-sizeof(char)) % sizeof(cell);
    } /* for */
  } else {
    /* the string is unpacked */
    for (i=0; cstr[i]!=0; i++) {
      if (informat) {
        assert(params!=NULL);
        paramidx+=dochar(amx,(char)cstr[i],params[paramidx]);
        informat=0;
      } else if (params!=NULL && (int)cstr[i]=='%') {
        if (paramidx<num)
          informat=1;
        else
          amx_RaiseError(amx, AMX_ERR_NATIVE);
      } else {
        F3_DebugPutChar(amx,(int)cstr[i]);
      } /* if */
    } /* for */
  } /* if */
  return paramidx;
}


int F3_SmallPrintString(AMX *amx,cell *cstr,cell *params,int num)
{
  return printstring(amx,cstr,params,num);
}
