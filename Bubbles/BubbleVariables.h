// BubbleVariables.h

#ifndef BUBBLEVARIABLES_H
#define BUBBLEVARIABLES_H
#include <vector>
#include <map>


#if _MSC_VER
#   define COMPILE_MESSAGE(exp) #pragma message(exp)
#else//__GNUC__ - may need other defines for different compilers
#   define COMPILE_MESSAGE(exp) #pragma message exp
#endif


// http://www.chris.com/ascii/index.php?art=animals/birds%20%28land%29
COMPILE_MESSAGE("        .---.        .-----------")
COMPILE_MESSAGE("       /     \\  __  /    ------   bubble build bird bows and greets you")
COMPILE_MESSAGE("      / /     \\(--)/    -----")
COMPILE_MESSAGE("     //////   ' \\/ `   ---")
COMPILE_MESSAGE("    //// / // :    : ---")
COMPILE_MESSAGE("   // /   /  /`    '--")
COMPILE_MESSAGE("  //          //..\\\\")
COMPILE_MESSAGE("=============UU====UU====")
COMPILE_MESSAGE("             '//||\\\\`")
COMPILE_MESSAGE("               ''``")

static std::vector<cBubbleEngine::PTR> engines;
static std::map<unsigned int /*engineGroupId*/, std::vector<unsigned int /*engineId*/>> engineGroups;
static std::map<unsigned int /*engineId*/, unsigned int /*engineGroupId*/> engineToGroup;
#endif
