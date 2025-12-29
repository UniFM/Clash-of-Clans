#pragma once
/*************************************************************
* @file     : GameManager.h
* @function ：全局游戏管理器
* @author   : 叶芷含 齐颖 俞筱雨
* @note     ：1. 单例模式建立游戏管理器
*             2. 管理游戏界面之间的切换（启动/登录/村庄/商店/战斗）
**************************************************************/
#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include "cocos2d.h"

USING_NS_CC;

// 建筑类型，实现在BuildingsData.h中
enum class BuildingType;

enum class ResourceType
{
    GOLD,    // 金币（建造/升级建筑）
    ELIXIR,  // 圣水（训练兵种）
    GEMS     // 宝石（加速/付费道具）
};

// 游戏状态
enum class GameState
{
    SPLASH,     // 启动界面
    LOGIN,      // 登录界面
    VILLAGE,    // 村庄界面
    SHOP,       // 商店界面
    BATTLE      // 战斗界面
};

/**
 * 游戏管理器 - 管理游戏全局状态
 */
class GameManager : public Ref
{
public:
    static GameManager* getInstance();
    static void destroyInstance();

    // 当前地图
    void setCurrentMap(int mapId) { _currentMapId = mapId; }    // 设置
    int getCurrentMap() const { return _currentMapId; }     // 获取

    // 设置游戏状态
    void setGameState(GameState state) { _gameState = state; }
    // 获取游戏状态
    GameState getGameState() const { return _gameState; }

    // 建筑放置状态管理
    void setPendingBuildingPlacement(BuildingType buildingType) { 
        _pendingBuildingType = buildingType; 
        _hasPendingBuilding = true;
    }
    
    // 判断是否有待放建筑
    bool hasPendingBuildingPlacement() const { return _hasPendingBuilding; }
    
    BuildingType getPendingBuildingType() const { return _pendingBuildingType; }
    
    void clearPendingBuildingPlacement() { 
        _hasPendingBuilding = false; 
    }

    // 资源管理方法
    //void addResource(ResourceType type, int amount);     // 增加指定类型资源（如金矿产出、战斗奖励）
    //bool spendResource(ResourceType type, int amount);    // 消耗指定类型资源（如训练兵种、建造建筑）   
    //int getResource(ResourceType type) const;    // 查询当前资源数量
     
     
    // 场景切换方法
    void gotoVillageScene();
    void gotoShopScene();
    void gotoBattleScene();
    void gotoLoginScene();
    //yxy-兵营update
    void gotoTroopSelectionScene();

    // 从商店返回村庄并放置建筑
    void gotoVillageSceneWithBuildingPlacement(BuildingType buildingType);

    // =======================update qy===================================
    // 资源操作
    void addResource(ResourceType type, int amount);
    bool spendResource(ResourceType type, int amount);
    int getResource(ResourceType type) const;

    // 产金核心接口（优化版）
    void setGoldProduceSpeedPerHour(int totalSpeed);
    int getGoldProduceSpeedPerHour() const;
    void updateGoldProduce(float dt);

    // 储金罐容量接口（新增）
    void setGoldStorageCapacity(int capacity);
    int getGoldStorageCapacity() const;


    // ==========  新增：圣水系统接口（与金币对称） ==========
    void setElixirProduceSpeedPerHour(int totalSpeed);
    int getElixirProduceSpeedPerHour() const;
    void updateElixirProduce(float dt);
    void setElixirStorageCapacity(int capacity);
    int getElixirStorageCapacity() const;
    // =======================update qy===================================

private:
    GameManager();
    ~GameManager();

    static GameManager* _instance;

    int _currentMapId;
    GameState _gameState;
    
    // 建筑放置状态
    bool _hasPendingBuilding;
    BuildingType _pendingBuildingType;

    // =======================update qy===================================
    int _gold;          // 当前金币（初始3000）
    int _elixir;        // 当前圣水
    int _gems;          // 当前宝石
    int _goldProduceSpeedPerHour; // 总产速(金币/小时)
    int _goldStorageCapacity;     // 储金罐容量（新增核心）
    static float _goldAccumulator;// 产金浮点累加器（精准无丢失）

    // ==========  新增：圣水系统（与金币对称） ==========
    int _elixirProduceSpeedPerHour;
    int _elixirStorageCapacity;
    static float _elixirAccumulator;
    // =======================update qy===================================
};

#endif // __GAME_MANAGER_H__

