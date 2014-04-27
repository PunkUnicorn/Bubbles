// BubbleSTDCALL.h

#ifndef BUBBLESTD_H
#define BUBBLESTD_H

#ifdef __GNUC__
#	define STDCALL 
#endif
#ifdef _MSC_VER /*visual studio version number*/
#	define STDCALL __stdcall
#endif

#endif