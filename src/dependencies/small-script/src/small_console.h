#ifndef F3_SMALL_CONSOLE_H
#define F3_SMALL_CONSOLE_H
#include "amx.h"
#ifdef __cplusplus
extern "C" {
#endif
int F3_SmallPrintString(AMX *amx,cell *cstr,cell *params,int num);
void F3_DebugPutChar(AMX *amx,int ch);
void F3_DebugPrintf(AMX *amx,const char *format,...);
#ifdef __cplusplus
}
#endif
#endif
