//// Ensure that GdiPlus header files work properly with MFC DEBUG_NEW and STL header files.

#define iterator _iterator

#ifdef _DEBUG

namespace Gdiplus
{
	namespace DllExports
	{
		#include <GdiplusMem.h>
	};

	#ifndef _GDIPLUSBASE_H
	#define _GDIPLUSBASE_H
	class GdiplusBase
	{
		public:
			void (operator delete)(void* in_pVoid)
			{
				DllExports::GdipFree(in_pVoid);
			}

			void* (operator new)(size_t in_size)
			{
				return DllExports::GdipAlloc(in_size);
			}

			void (operator delete[])(void* in_pVoid)
			{
				DllExports::GdipFree(in_pVoid);
			}

			void* (operator new[])(size_t in_size)
			{
				return DllExports::GdipAlloc(in_size);
			}

			void * (operator new)(size_t nSize, int, LPCSTR, int)
			{
				return DllExports::GdipAlloc(nSize);
			}

			void operator delete(void* p, int, LPCSTR, int)
			{
				DllExports::GdipFree(p);
			}

		};
	#endif // #ifndef _GDIPLUSBASE_H
}
#endif // #ifdef _DEBUG

#include <gdiplus.h>
#undef iterator