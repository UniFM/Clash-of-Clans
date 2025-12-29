/*************************************************************
* @file     : BattleManager.cpp
* @function ：战斗管理器核心实现 - 资源管理+游戏状态控制+属性配置获取
* @author   : 俞筱雨
* @note     : 1.单例模式实现，初始化默认金币/圣水/宝石资源数量；
              2.封装资源增减、消耗校验核心逻辑；
              3.管理游戏村庄/战斗/编辑三种状态切换；
              4.提供建筑/兵种基础属性配置获取接口；
              5.不同类型建筑/兵种返回差异化属性值，适配战斗系统数值逻辑
**************************************************************/


#pragma once
#ifndef __BATTLE_MANAGER_H__
#define __BATTTLE_MANAGER_H__

#include "cocos2d.h"
#include "Constant/Constant.h"

// 战斗资源类型
enum class BattleBattleResourceType {
    GOLD,    // 金币
    ELIXIR,  // 圣水
    GEMS     // 宝石
};

// 游戏状态
enum class BattleGameState {
    HOME,       // 村庄管理状态
    BATTLE,     // 战斗攻击状态
    EDIT_MODE   // 建筑移动编辑状态
};

// 战斗管理器类（单例模式）
class BattleManager {
public:
    // 获取战斗管理器单例实例
    static BattleManager* getInstance();

    // 游戏状态相关
    void setBattleGameState(BattleGameState state);       // 设置当前游戏状态
    BattleGameState getBattleGameState() const;           // 获取当前游戏状态

    // 辅助方法：获取属性配置
    static BattleBuildingStats getBattleBuildingStats(BattleBuildingType type, int level = 1); // 获取建筑属性
    static BattleTroopStats getBattleTroopStats(BattleTroopType type, int level = 1);         // 获取兵种属性

private:
    BattleManager(); // 私有构造函数（单例）

    // 玩家战斗资源相关
    void addBattleResource(BattleResourceType type, int amount);   // 增加指定类型资源
    bool spendBattleResource(BattleResourceType type, int amount); // 消耗指定类型资源（返回是否消耗成功）
    int getBattleResource(BattleResourceType type);                // 获取指定类型资源数量

    static BattleManager* instance; // 单例对象指针

    // 资源数量
    int _gold;    // 金币数量
    int _elixir;  // 圣水数量
    int _gems;    // 宝石数量

    BattleGameState _currentState; // 当前游戏状态
};

#endif