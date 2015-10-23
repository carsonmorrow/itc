#pragma once

#ifdef _DEBUG
	#include <crtdbg.h>
    #define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
    #define new DEBUG_NEW

	#define ENABLE_LEAK_CHECK _CrtSetDbgFlag( _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG ) | \
		( _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF ) );
#else
	#define ENABLE_LEAK_CHECK
#endif