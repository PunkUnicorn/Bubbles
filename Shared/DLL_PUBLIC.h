// BubbleDLL_PUBLIC.h

#ifndef BUBBLEDLL_PUBLIC_H
#define BUBBLEDLL_PUBLIC_H

#if defined _WIN32 || defined __CYGWIN__
#   ifdef BUILDING_DLL
#      ifdef __GNUC__
#         define DLL_PUBLIC __attribute__ ((dllexport))
#      endif
#      ifdef _MSC_VER /*visual studio version number*/
#         define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
#      endif
#   else
#      ifdef __GNUC__
#         define DLL_PUBLIC __attribute__ ((dllimport))
#      endif
#      ifdef _MSC_VER 
#         define DLL_PUBLIC __declspec(dllimport)
#      endif
#   endif
#   define DLL_LOCAL
#else
#   if __GNUC__ >= 4
#      define DLL_PUBLIC __attribute__ ((visibility ("default")))
#      define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#   endif
#   ifdef _MSC_VER
#      define DLL_PUBLIC
#      define DLL_LOCAL
#   endif
#endif

#endif