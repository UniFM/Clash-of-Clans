#pragma once
/*************************************************************
* @file     : GameManager.h
* @function ：全局游戏管理器
* @author   : 
* @note     ：1. 单例模式建立游戏管理器
*             2. 管理游戏界面之间的切换（启动/登录/村庄/商店/战斗）
**************************************************************/
#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include "cocos2d.h"

USING_NS_CC;

// 前向声明
enum class BuildingType;

/**
 * 游戏管理器 - 管理游戏全局状态
 */
class GameManager
{
public:
    static GameManager* getInstance();
    static void destroyInstance();

    // 当前地图
    void setCurrentMap(int mapId) { _currentMapId = mapId; }    // 设置
    int getCurrentMap() const { return _currentMapId; }     // 获取

    // 游戏状态
    enum class GameState
    {
        SPLASH,     // 启动界面
        LOGIN,      // 登录界面
        VILLAGE,    // 村庄界面
        SHOP,       // 商店界面
        BATTLE      // 战斗界面
    };

    // 设置游戏状态
    void setGameState(GameState state) { _gameState = state; }
    // 获取游戏状态
    GameState getGameState() const { return _gameState; }

    // 建筑放置状态管理
    void setPendingBuildingPlacement(BuildingType buildingType) { 
        _pendingBuildingType = buildingType; 
        _hasPendingBuilding = true;
    }
    
    bool hasPendingBuildingPlacement() const { return _hasPendingBuilding; }
    
    BuildingType getPendingBuildingType() const { return _pendingBuildingType; }
    
    void clearPendingBuildingPlacement() { 
        _hasPendingBuilding = false; 
    }

    // 场景切换方法
    void gotoVillageScene();
    void gotoShopScene();
    void gotoBattleScene();
    void gotoLoginScene();

    // 从商店返回村庄并放置建筑
    void gotoVillageSceneWithBuildingPlacement(BuildingType buildingType);

private:
    GameManager();
    ~GameManager();

    int _currentMapId;
    GameState _gameState;
    
    // 建筑放置状态
    bool _hasPendingBuilding;
    BuildingType _pendingBuildingType;
    
    static GameManager* _instance;
};

#endif // __GAME_MANAGER_H__

