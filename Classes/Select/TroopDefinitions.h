#ifndef TROOP_DEFINITIONS_H
#define TROOP_DEFINITIONS_H

#include <string>
#include <map>

enum class TroopType {
    BARBARIAN,
    ARCHER,
    BOMBER,
    GIANT
};

struct TroopInfo {
    std::string name;
    int housingSpace;
    std::string colorHex; // For the color block placeholder
};

static const std::map<TroopType, TroopInfo> TROOP_DATA = {
    {TroopType::BARBARIAN, {"Barbarian", 1, "#FF0000"}}, // Red
    {TroopType::ARCHER,    {"Archer",    3, "#00FF00"}}, // Green
    {TroopType::BOMBER,    {"Bomber",    3, "#0000FF"}}, // Blue
    {TroopType::GIANT,     {"Giant",     5, "#FFFF00"}}  // Yellow
};

#endif // TROOP_DEFINITIONS_H
