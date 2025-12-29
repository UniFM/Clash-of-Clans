#ifndef __BATTLE_MANAGER_H__
#define __BATTTLE_MANAGER_H__

#include "cocos2d.h"
#include "Constant/Constant.h"

// BattleResource Types
enum class BattleBattleResourceType {
    GOLD,
    ELIXIR,
    GEMS
};
// Game State
enum class BattleGameState {
    HOME,       // Managing village
    BATTLE,     // Attacking
    EDIT_MODE   // Moving buildings
};

class BattleManager {
public:
    static BattleManager* getInstance();


    // Game State
    void setBattleGameState(BattleGameState state);
    BattleGameState getBattleGameState() const;

    // Helper to get stats
    static BattleBuildingStats getBattleBuildingStats(BattleBuildingType type, int level = 1);
    static BattleTroopStats getBattleTroopStats(BattleTroopType type, int level = 1);

private:
    BattleManager();

    // Player BattleResources
    void addBattleResource(BattleResourceType type, int amount);
    bool spendBattleResource(BattleResourceType type, int amount);
    int getBattleResource(BattleResourceType type);

    static BattleManager* instance;

    int _gold;
    int _elixir;
    int _gems;

    BattleGameState _currentState;
};

#endif
