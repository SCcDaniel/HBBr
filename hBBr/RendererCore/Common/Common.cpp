﻿#include "Common.h"
#include "ConsoleDebug.h"
#include <assert.h>
void MessageOut(const char* msg, bool bExit, bool bMessageBox, const char* textColor)
{
	HString msgStr = msg;
	ConsoleDebug::print_endl(msgStr, textColor);
#if defined(_WIN32)
	if (bMessageBox)
	{
		//MessageBoxA(NULL, msg, "message", MB_ICONERROR);
		#if NDEBUG
		MessageBoxA(NULL, msg, "message", MB_ICONERROR);
		#else
		DE_ASSERT(0, msg);
		#endif
	}
#else
	printf(msg);
	fflush(stdout);
#endif
	if (bExit)
		exit(EXIT_FAILURE);
}

