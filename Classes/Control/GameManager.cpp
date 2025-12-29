/*************************************************************
* @file     : GameManager.h
* @function ：全局游戏管理器
* @author   : 叶芷含 齐颖 俞筱雨
* @note     ：1. 单例模式建立游戏管理器
*             2. 管理游戏界面之间的切换（启动/登录/村庄/商店/战斗）
**************************************************************/
#include "GameManager.h"
#include "Map/SceneMap.h"
#include "Scene/ShopScene.h"
#include "Map/BattleScene.h"
#include "Scene/LoginScene.h"
#include "buildings/BuildingsData.h"
#include "Scene/TroopSelectionScene.h"

GameManager* GameManager::_instance = nullptr;
// =======================update qy===================================
float GameManager::_goldAccumulator = 0.0f; // 全局浮点累加器，保证产金无丢失
float GameManager::_elixirAccumulator = 0.0f; //  新增圣水累加器
// =======================update qy===================================

GameManager* GameManager::getInstance()
{
    if (!_instance)
    {
        _instance = new (std::nothrow) GameManager();
    }
    return _instance;
}

void GameManager::destroyInstance()
{
    CC_SAFE_DELETE(_instance);
}

// =======================update qy===================================
// 构造函数【核心修改：初始化3000金币+默认储金罐容量】
GameManager::GameManager()
    : _currentMapId(1)
    , _gameState(GameState::SPLASH)
    , _hasPendingBuilding(false)
    , _pendingBuildingType(BuildingType::TOWN_HALL)
    , _goldProduceSpeedPerHour(0)
    , _gold(3000)          //  固定初始金币3000
    , _elixir(3000)
    , _gems(0)
    , _goldStorageCapacity(5000) //  初始储金罐容量5000（可自定义）
    , _elixirStorageCapacity(5000)
    , _elixirProduceSpeedPerHour(0)
{
}
// =======================update qy===================================

GameManager::~GameManager()
{
}

void GameManager::gotoVillageScene()
{
    CCLOG("GameManager: Switching to Village Scene");
    setGameState(GameState::VILLAGE);

    auto scene = SceneMap::getInstance();
    if (scene) {
        Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene));
    }
}

void GameManager::gotoVillageSceneWithBuildingPlacement(BuildingType buildingType)
{
    CCLOG("GameManager: Switching to Village Scene with building placement for type: %d", (int)buildingType);

    // 设置待放置的建筑类型
    setPendingBuildingPlacement(buildingType);
    setGameState(GameState::VILLAGE);

    auto scene = SceneMap::getInstance();
    if (scene) {
        Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene));
    }
}

void GameManager::gotoShopScene()
{
    CCLOG("GameManager: Switching to Shop Scene");
    setGameState(GameState::SHOP);
    
    auto scene = ShopScene::create();
    if (scene) {
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
    }
}

void GameManager::gotoBattleScene()
{
    CCLOG("GameManager: Switching to Battle Scene");
    setGameState(GameState::BATTLE);
    
    auto scene = BattleScene::createScene();
    if (scene) {
        Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene));
    }
}

void GameManager::gotoLoginScene()
{
    CCLOG("GameManager: Switching to Login Scene");
    setGameState(GameState::LOGIN);
    
    auto scene = LoginScene::createScene();
    if (scene) {
        Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene));
    }
}

void GameManager::gotoTroopSelectionScene()
{
    CCLOG("GameManager: Pushing TroopSelection Scene");

    auto scene = TroopSelectionScene::create();
    if (scene) {
        // Use pushScene to keep the MapScene alive in the background
        //Director::getInstance()->pushScene(scene);
        Director::getInstance()->replaceScene(TransitionFade::create(0.5, scene));
    }
}

// =======================update qy===================================
//  修改addResource：圣水添加带容量校验（与金币一致）
void GameManager::addResource(ResourceType type, int amount)
{
    if (amount <= 0) return;
    switch (type)
    {
    case ResourceType::GOLD:
        _gold = std::min(_gold + amount, _goldStorageCapacity);
        break;
    case ResourceType::ELIXIR:
        _elixir = std::min(_elixir + amount, _elixirStorageCapacity); //  圣水容量校验
        break;
    case ResourceType::GEMS: _gems += amount; break;
    }
}
bool GameManager::spendResource(ResourceType type, int amount)
{
    if (amount < 0) return false;
    switch (type)
    {
    case ResourceType::GOLD:
        if (_gold >= amount) { _gold -= amount; return true; }
        break;
    case ResourceType::ELIXIR:
        if (_elixir >= amount) { _elixir -= amount; return true; }
        break;
    case ResourceType::GEMS:
        if (_gems >= amount) { _gems -= amount; return true; }
        break;
    }
    return false;
}
int GameManager::getResource(ResourceType type) const
{
    switch (type)
    {
    case ResourceType::GOLD: return _gold;
    case ResourceType::ELIXIR: return _elixir;
    case ResourceType::GEMS: return _gems;
    default: return 0;
    }
}

//  产金核心逻辑【彻底重构，与升级无任何耦合】
void GameManager::setGoldProduceSpeedPerHour(int totalSpeed)
{
    _goldProduceSpeedPerHour = std::max(0, totalSpeed);
    CCLOG("[产金系统] 总产速更新：%d 金币/小时", _goldProduceSpeedPerHour);
}
int GameManager::getGoldProduceSpeedPerHour() const
{
    return _goldProduceSpeedPerHour;
}
//  精准产金累加：仅计算、不阻塞、满1金币才刷新UI，0影响升级
void GameManager::updateGoldProduce(float dt)
{
    // 无产能/金币已满 → 直接跳过，无性能消耗
    if (_goldProduceSpeedPerHour <= 0 || _gold >= _goldStorageCapacity)
    {
        _goldAccumulator = 0.0f;
        return;
    }

    // 小时 → 秒 换算，精准计算每帧产出
    float goldPerSec = (float)_goldProduceSpeedPerHour / 3600.0f;
    _goldAccumulator += goldPerSec * dt;

    //  仅当累加≥1金币时，才更新数值+刷新UI，极致性能
    if (_goldAccumulator >= 1.0f)
    {
        int addGold = (int)_goldAccumulator;
        // 校验：不超过储金罐上限
        int realAdd = std::min(addGold, _goldStorageCapacity - _gold);
        if (realAdd > 0)
        {
            _gold += realAdd;
            _goldAccumulator -= realAdd;
            // 仅此时刷新UI，无冗余调用
            SceneMap::getInstance()->refreshResourceUI();
            CCLOG("[产金系统] 产出金币：%d，当前总金币：%d / %d", realAdd, _gold, _goldStorageCapacity);
        }
    }
}

//  储金罐容量接口（供储金罐升级调用）
void GameManager::setGoldStorageCapacity(int capacity)
{
    _goldStorageCapacity = std::max(1000, capacity);
    CCLOG("[储金罐] 容量更新：%d 金币", _goldStorageCapacity);
}
int GameManager::getGoldStorageCapacity() const
{
    return _goldStorageCapacity;
}

// ==========  新增：圣水系统接口实现（与金币1:1对称） ==========
void GameManager::setElixirProduceSpeedPerHour(int totalSpeed)
{
    _elixirProduceSpeedPerHour = std::max(0, totalSpeed);
    CCLOG("[产圣水系统] 总产速更新：%d 圣水/小时", _elixirProduceSpeedPerHour);
}
int GameManager::getElixirProduceSpeedPerHour() const
{
    return _elixirProduceSpeedPerHour;
}
void GameManager::updateElixirProduce(float dt)
{
    if (_elixirProduceSpeedPerHour <= 0 || _elixir >= _elixirStorageCapacity)
    {
        _elixirAccumulator = 0.0f;
        return;
    }
    float elixirPerSec = (float)_elixirProduceSpeedPerHour / 3600.0f;
    _elixirAccumulator += elixirPerSec * dt;
    if (_elixirAccumulator >= 1.0f)
    {
        int addElixir = (int)_elixirAccumulator;
        int realAdd = std::min(addElixir, _elixirStorageCapacity - _elixir);
        if (realAdd > 0)
        {
            _elixir += realAdd;
            _elixirAccumulator -= realAdd;
            SceneMap::getInstance()->refreshResourceUI();
            CCLOG("[产圣水系统] 产出圣水：%d，当前总圣水：%d / %d", realAdd, _elixir, _elixirStorageCapacity);
        }
    }
}
void GameManager::setElixirStorageCapacity(int capacity)
{
    _elixirStorageCapacity = std::max(1000, capacity);
    CCLOG("[圣水瓶] 容量更新：%d 圣水", _elixirStorageCapacity);
}
int GameManager::getElixirStorageCapacity() const
{
    return _elixirStorageCapacity;
}
// =======================update qy===================================
