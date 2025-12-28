#include "TroopManager.h"

TroopManager* TroopManager::instance = nullptr;

TroopManager* TroopManager::getInstance() {
    if (!instance) {
        instance = new TroopManager();
    }
    return instance;
}

TroopManager::TroopManager() : barracksLevel(1) {
    // Initialize counts to 0
    selectedTroops[TroopType::BARBARIAN] = 0;
    selectedTroops[TroopType::ARCHER] = 0;
    selectedTroops[TroopType::GOBLIN] = 0;
    selectedTroops[TroopType::GIANT] = 0;
    //selectedTroops[TroopType::NONE] = 0;
}

void TroopManager::setBarracksLevel(int level) {
    barracksLevel = level;
}

int TroopManager::getBarracksLevel() const {
    return barracksLevel;
}

int TroopManager::getMaxHousingSpace() const {
    // Example logic: Level 1 = 20, Level 2 = 30, etc.
    return 10 + (barracksLevel * 10);
}

void TroopManager::setTroopCount(TroopType type, int count) {
    if (count < 0) return;
    selectedTroops[type] = count;
}

int TroopManager::getTroopCount(TroopType type) const {
    auto it = selectedTroops.find(type);
    if (it != selectedTroops.end()) {
        return it->second;
    }
    return 0;
}

void TroopManager::clearTroops() {
    for (auto& pair : selectedTroops) {
        pair.second = 0;
    }
}

int TroopManager::getCurrentHousingSpace() const {
    int total = 0;
    for (const auto& pair : selectedTroops) {
        total += pair.second * TROOP_DATA.at(pair.first).housingSpace;
    }
    return total;
}

bool TroopManager::canAddTroop(TroopType type) const {
    int spaceNeeded = TROOP_DATA.at(type).housingSpace;
    return (getCurrentHousingSpace() + spaceNeeded) <= getMaxHousingSpace();
}