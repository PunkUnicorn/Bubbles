// BubbleSTDCALL.h

#ifndef BUBBLESTD_H
#define BUBBLESTD_H

#ifdef __GNUC__
#   define STDCALL 
#endif
#ifdef _MSC_VER 
#   define STDCALL __stdcall
#endif

#endif