// BubbleVariables.h

#ifndef BUBBLEVARIABLES_H
#define BUBBLEVARIABLES_H
#include <vector>
#include <map>
#include "BubbleEngine.h"

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
#endif

// move art to makefile

#if __GNUC__
#pragma warning "        .---.        .-----------"))
#pragma warning "       /     \\  __  /    ------   bubble build bird bows and greets you")
#pragma warning "      / /     \\(--)/    -----")
#pragma warning "     //////   ' \\/ `   ---")
#pragma warning "    //// / // :    : ---")
#pragma warning "    // /   /  /`    '--")
#pragma warning "  //          //..\\\\")
#pragma warning "=============UU====UU====")
#pragma warning "             '//||\\\\`")
#pragma warning "               ''``")
#endif

static std::vector<cBubbleEngine::PTR> engines;
static std::map<unsigned int /*engineGroupId*/, std::vector<unsigned int /*engineId*/> > engineGroups;
static std::map<unsigned int /*engineId*/, unsigned int /*engineGroupId*/> engineToGroup;


#endif
