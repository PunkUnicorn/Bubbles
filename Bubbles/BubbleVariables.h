// BubbleVariables.h

#ifndef BUBBLEVARIABLES_H
#define BUBBLEVARIABLES_H
#include <vector>
#include <map>
#include "BubbleEngine.h"

namespace Bubbles 
{

#if _MSC_VER
#pragma message("        .---.         .-----------")
#pragma message("       /     \\  __  /    ------   bubble build bird bows and greets you")
#pragma message("      / /     \\(--)/    -----")
#pragma message("     //////   ' \\/ `   ---")
#pragma message("    //// / // :    : ---")
#pragma message("   // /   /  /`    '--")
#pragma message("  //          //..\\\\    ASCII Art by Daron Brewood (http://www.chris.com/ascii/index.php)")
#pragma message("=============UU====UU====")
#pragma message("             '//||\\\\`")
#pragma message("               ''``")
#endif

static std::vector<cBubbleEngine::PTR> engines;
static std::map<unsigned int /*engineGroupId*/, std::vector<unsigned int /*engineId*/> > engineGroups;
static std::map<unsigned int /*engineId*/, unsigned int /*engineGroupId*/> engineToGroup;

}

#endif
