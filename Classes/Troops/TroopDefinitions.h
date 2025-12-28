#ifndef TROOP_DEFINITIONS_H
#define TROOP_DEFINITIONS_H

#include <string>
#include <map>
#include "Troops/Unit.h"

// Use UnitType as TroopType to maintain consistency
using TroopType = UnitType;

struct TroopInfo {
    std::string name;
    int housingSpace;
    std::string imagePath; // Changed from color code to image path
};

// Troop data mapping
static const std::map<TroopType, TroopInfo> TROOP_DATA = {
    {TroopType::BARBARIAN, {"Barbarian", 1, "Troops/Barbarian.png"}},
    {TroopType::ARCHER,    {"Archer",    3, "Troops/Archer.png"}},
    {TroopType::GOBLIN,    {"Goblin",    3, "Troops/Goblin.png"}}, // Using Goblin as placeholder for Goblin
    {TroopType::GIANT,     {"Giant",     5, "Troops/Giant.png"}}
};

#endif // TROOP_DEFINITIONS_H