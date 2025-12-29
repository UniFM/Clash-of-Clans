#include "BattleManager.h"

BattleManager* BattleManager::instance = nullptr;

BattleManager* BattleManager::getInstance() {
    if (!instance) {
        instance = new BattleManager();
    }
    return instance;
}

BattleManager::BattleManager()
    : _gold(1000), _elixir(1000), _gems(500), _currentState(BattleGameState::HOME) {
}

int BattleManager::getBattleResource(BattleResourceType type) {
    switch (type) {
    case BattleResourceType::GOLD: return _gold;
    case BattleResourceType::ELIXIR: return _elixir;
    case BattleResourceType::GEMS: return _gems;
    }
    return 0;
}

void BattleManager::addBattleResource(BattleResourceType type, int amount) {
    switch (type) {
    case BattleResourceType::GOLD: _gold += amount; break;
    case BattleResourceType::ELIXIR: _elixir += amount; break;
    case BattleResourceType::GEMS: _gems += amount; break;
    }
}

bool BattleManager::spendBattleResource(BattleResourceType type, int amount) {
    switch (type) {
    case BattleResourceType::GOLD:
        if (_gold >= amount) { _gold -= amount; return true; }
        break;
    case BattleResourceType::ELIXIR:
        if (_elixir >= amount) { _elixir -= amount; return true; }
        break;
    case BattleResourceType::GEMS:
        if (_gems >= amount) { _gems -= amount; return true; }
        break;
    }
    return false;
}


void BattleManager::setBattleGameState(BattleGameState state) {
    _currentState = state;
}

BattleGameState BattleManager::getBattleGameState() const {
    return _currentState;
}

BattleBuildingStats BattleManager::getBattleBuildingStats(BattleBuildingType type, int level) {
    BattleBuildingStats stats;
    // Simple placeholder logic for stats
    switch (type) {
    case BattleBuildingType::TOWN_HALL:
        stats = { "Town Hall", 1000, 0, 1000, 0, 0, 0, 0, 1000 };
        break;
    case BattleBuildingType::GOLD_MINE:
        stats = { "Gold Mine", 100, 10, 0, 150, 0, 0, 0, 0 };
        break;
    case BattleBuildingType::ELIXIR_COLLECTOR:
        stats = { "Elixir Collector", 100, 10, 150, 0, 0, 0, 0, 0 };
        break;
    case BattleBuildingType::CANNON:
        stats = { "Cannon", 200, 60, 250, 0, 150.0f, 1.0f, 20, 0 };
        break;
    case BattleBuildingType::ARCHER_TOWER:
        stats = { "Archer Tower", 150, 60, 300, 0, 200.0f, 0.8f, 15, 0 };
        break;
    case BattleBuildingType::WALL:
        stats = { "Wall", 200, 5, 50, 0, 0, 0, 0, 0 };
        break;
    case BattleBuildingType::GOLD_STORAGE:
        stats = { "Gold Storage", 100, 30, 500, 0, 0, 0, 0, 2000 };
        break;
    case BattleBuildingType::ELIXIR_STORAGE:
        stats = { "Elixir Storage", 100, 30, 0, 500, 0, 0, 0, 2000 };
        break;
    case BattleBuildingType::ARMY_CAMP:
        stats = { "Army Camp", 600, 20, 300, 0, 0, 0, 0, 50 };
        break;
    case BattleBuildingType::BARRACKS:
        stats = { "Barracks", 700, 40, 400, 0, 0, 0, 0, 0 };
        break;
    default:
        stats = { "BattleBuilding", 100, 0, 0, 0, 0, 0, 0, 0 };
        break;
    }
    return stats;
}

BattleTroopStats BattleManager::getBattleTroopStats(BattleTroopType type, int level) {
    BattleTroopStats stats;
    switch (type) {
    case BattleTroopType::BARBARIAN:
        stats = { "Barbarian", 100, 20, 50.0f, 30.0f, 1.0f, 10, 1, 25 };
        break;
    case BattleTroopType::ARCHER:
        stats = { "Archer", 50, 20, 50.0f, 100.0f, 1.0f, 10, 1, 50 };
        break;
    case BattleTroopType::GIANT:
        stats = { "Giant", 400, 30, 30.0f, 30.0f, 1.0f, 30, 5, 250 };
    case BattleTroopType::GOBLIN:
        stats = { "Goblin", 50, 30, 80.0f, 30.0f, 2.0f, 30, 3, 50 };
        break;
    default:
        stats = { "BattleTroop", 50, 5, 50.0f, 30.0f, 1.0f, 25, 1, 25 };
        break;
    }
    return stats;
}
