#ifndef TROOP_MANAGER_H
#define TROOP_MANAGER_H

#include "TroopDefinitions.h"
#include "buildings/BuildingsData.h"
#include <map>

class TroopManager {
public:
    static TroopManager* getInstance();

    // Game State
    void setBarracksLevel(int level);
    int getBarracksLevel() const;

    int getMaxHousingSpace() const;

    // Troop Selection
    void setTroopCount(TroopType type, int count);
    int getTroopCount(TroopType type) const;
    void clearTroops();

    int getCurrentHousingSpace() const;
    bool canAddTroop(TroopType type) const;

    //const std::map<TroopType, int>& getSelectedTroops() const;
    std::map<TroopType, int> selectedTroops;
    const std::map<TroopType, int>& getSelectedTroops() const { return selectedTroops; }

private:
    TroopManager();
    static TroopManager* instance;

    int barracksLevel;
};

#endif