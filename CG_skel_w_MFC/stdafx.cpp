// stdafx.cpp : source file that includes just the standard includes
// CG_skel_w_MFC.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#pragma comment(lib, "legacy_stdio_definitions.lib")
#ifdef __cplusplus
FILE iob[] = { *stdin, *stdout, *stderr };
extern "C" {
	FILE* __cdecl _iob(void) { return iob; }
}
#endif
// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
