// BubbleVariables.h

#ifndef BUBBLEVARIABLES_H
#define BUBBLEVARIABLES_H
#include <vector>
#include <map>

// http://www.chris.com/ascii/index.php?art=animals/birds%20%28land%29

#if _MSC_VER
#pragma message("        .---.        .-----------")
#pragma message("       /     \\  __  /    ------   bubble build bird bows and greets you")
#pragma message("      / /     \\(--)/    -----")
#pragma message("     //////   ' \\/ `   ---")
#pragma message("    //// / // :    : ---")
#pragma message("   // /   /  /`    '--")
#pragma message("  //          //..\\\\")
#pragma message("=============UU====UU====")
#pragma message("             '//||\\\\`")
#pragma message("               ''``")
#else
__pragma(message(""))("        .---.        .-----------")
__pragma(message(""))("       /     \\  __  /    ------   bubble build bird bows and greets you")
__pragma(message(""))("      / /     \\(--)/    -----")
__pragma(message(""))("     //////   ' \\/ `   ---")
__pragma(message(""))("    //// / // :    : ---")
__pragma(message(""))("   // /   /  /`    '--")
__pragma(message(""))("  //          //..\\\\")
__pragma(message(""))("=============UU====UU====")
__pragma(message(""))("             '//||\\\\`")
__pragma(message(""))("               ''``")
#endif

static std::vector<cBubbleEngine::PTR> engines;
static std::map<unsigned int /*engineGroupId*/, std::vector<unsigned int /*engineId*/>> engineGroups;
static std::map<unsigned int /*engineId*/, unsigned int /*engineGroupId*/> engineToGroup;
#endif
