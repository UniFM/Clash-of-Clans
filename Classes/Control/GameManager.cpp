/*************************************************************
* @file     : GameManager.h
* @function ：全局游戏管理器
* @author   :
* @note     ：1. 单例模式建立游戏管理器
*             2. 管理游戏界面之间的切换（启动/登录/村庄/商店/战斗）
**************************************************************/
#include "GameManager.h"
#include "Map/SceneMap.h"
#include "Scene/ShopScene.h"
#include "Map/BattleScene.h"
#include "Scene/LoginScene.h"
#include "Building/Building.h"

GameManager* GameManager::_instance = nullptr;

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

GameManager::GameManager()
    : _currentMapId(1)
    , _gameState(GameState::SPLASH)
    , _hasPendingBuilding(false)
    , _pendingBuildingType(BuildingType::TOWN_HALL)
{
}

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

