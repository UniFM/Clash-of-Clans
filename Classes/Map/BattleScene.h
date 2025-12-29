/*************************************************************
* @file     : BattleScene.h
* @function ：战斗场景头文件 - 定义战斗场景核心接口与成员变量
* @author   : 俞筱雨
* @note     : 1.定义多关卡战斗场景创建、初始化、帧更新核心接口；
*             2.封装兵种选择、部署、动画及UI交互相关方法；
*             3.区分不同关卡建筑布局创建函数，适配差异化关卡配置；
*             4.包含胜负判定、UI提示、实体管理等战斗核心逻辑接口；
*             5.存储地图实例、敌方建筑列表、选中兵种状态等核心成员；
*             6.定义战斗状态标记（开始/结束/胜负显示），适配战斗流程管控；
*             7.管理兵种精灵/标签映射，支持选中状态与数量实时更新
**************************************************************/

#pragma once
#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include "cocos2d.h"
#include "BaseMap.h"
#include "buildings/BuildingsData.h"
#include "Troops/Unit.h"
#include "Troops/TroopDefinitions.h" // 需包含TroopType定义的头文件
#include "Troops/TroopManager.h"
#include "Troops/Entity.h"
#include "buildings/BattleBuilding.h"
#include "Troops/BattleTroop.h"
#include "Control/BattleManager.h"
#include "Constant/Constant.h"

USING_NS_CC;

// 战斗场景类（管理战斗流程、兵种部署、UI交互、战斗逻辑）
class BattleScene : public Scene
{
public:
    // 创建战斗场景实例（指定关卡ID，默认0）
    static Scene* createScene(int levelId = 0);
    // 创建BattleScene实例（内部调用，配合createScene使用）
    static BattleScene* create(int levelId);

    // 初始化战斗场景（加载关卡数据、初始化地图/UI/战斗实体）
    virtual bool init(int levelId);

    void spawnBuilding(BattleBuildingType type, int x, int y, Team team);

    //virtual void update(float dt) override; // 新增：重写update方法

// 初始化兵种选择UI（创建兵种选择按钮、数量显示等）
    void initTroopSelectionUI();
    // 初始化地图部署监听器（监听触摸事件处理兵种部署）
    void initMapDeploymentListener();

    // 更新兵种选择状态（切换选中的兵种类型）
    void updateTroopSelection(TroopType selectedType);

    // 获取当前选中的部队类型（返回选中的兵种枚举值）
    TroopType getSelectedTroopType() { return _selectedTroopType; }

    // 部署兵种到指定地图位置（创建Unit实例并添加到场景）
    void deployTroop(const Vec2& mapPos);

    // 长按部署回调（处理长按连续部署兵种逻辑）
    void onLongPressDeploy(float dt);

    // 更新UI标签（刷新兵种数量、战斗状态等显示）
    void updateLabels();

    // 取消选中兵种回调（重置选中状态，停止选中动画）
    void onTroopDeselected(TroopType type);

    // 【添加】战斗相关函数
//void spawnBuilding(BattleBuildingType type, float x, float y, Team team);
    void initHardcodedBuildings();
    
    // 【添加】分关卡建筑布局创建方法
    void createLevel0Buildings(int centerX, int centerY);
    void createLevel1Buildings(int centerX, int centerY);
    void createLevel2Buildings(int centerX, int centerY);
    void createDefaultBuildings(int centerX, int centerY);
    
    void update(float dt) override;
    //void cleanupDeadEntities();

        // 【添加】胜负判断相关函数
    void checkBattleResult();      // 检查战斗结果
    void showVictoryUI();          // 显示胜利UI
    void showDefeatUI();           // 显示失败UI
    bool areAllBuildingsDestroyed(); // 检查所有建筑是否都被摧毁
    bool areAllTroopsDeployedAndDead(); // 检查所有兵种是否都已投放且死亡

protected:
    std::map<TroopType, int> selectedTroops; // 选中的兵种数量映射（类型-数量）

private:
    BaseMap* _gameMap; // 游戏地图实例（承载地形、建筑等）
    Vector<Building*> _enemyBuildings; // 敌方建筑列表（存储所有可攻击的建筑）

    Building* _enemyTownHall; // 敌方主城（核心目标，摧毁则战斗胜利）
    bool _battleStarted; // 战斗是否已开始标记
    bool _battleEnded; // 战斗是否已结束标记
    int _levelId; // 当前关卡ID（用于加载对应关卡数据）

    bool _victoryShown;           // 是否已显示胜利
    bool _defeatShown;            // 是否已显示失败

    // 【修改】战斗实体管理 - 移除_gameLayer，直接使用地图容器
    cocos2d::Vector<Entity*> _entities;



    // 兵种选择相关
    TroopType _selectedTroopType; // 当前选中的兵种类型
    bool _isLongPressing = false; // 长按状态标记（用于连续部署）
    Vec2 _lastDeployPos; // 上一次部署位置（用于校准长按部署坐标）
    Sprite* _selectedTroopSprite = nullptr; // 当前选中兵种的预览精灵

    // 兵种动画相关
    std::map<TroopType, Sprite*> _troopSprites; // 兵种预览精灵映射（类型-精灵）
    std::map<TroopType, Label*> _troopCountLabels; // 兵种数量标签映射（类型-标签）

    // 初始化基础UI（仅创建返回按钮等核心UI元素）
    void setupUI();

    // 初始化战斗实体（加载敌方建筑、地形等战斗相关元素）
    void initBattleEntities();

    // 创建兵种闲置动画（生成指定兵种的循环闲置动画）
    Animation* createTroopIdleAnimation(TroopType type);

    // 播放选中动画（对指定精灵执行缩放/高亮等选中特效）
    void playSelectionAnimation(Sprite* sprite);
    // 停止选中动画（移除精灵的选中特效，恢复初始状态）
    void stopSelectionAnimation(Sprite* sprite);
};

#endif // __BATTLE_SCENE_H__
