#pragma once
/*************************************************************
* @file     : SceneMap.h
* @function ：主游戏场景 - 村庄管理
* @author   :
* @note     ：管理村庄场景的UI和建筑系统
**************************************************************/
#ifndef __SCENE_MAP_H__
#define __SCENE_MAP_H__

#include "cocos2d.h"
#include "HomeVillageMap.h"
#include "buildings/BuildingsData.h"
#include "buildings/BuildingPreview.h"

USING_NS_CC;

/**
 * 主游戏场景 - 村庄管理
 */
class SceneMap : public Scene
{
public:
    // 使用单例，提供全局访问
    static SceneMap* getInstance();

    // 删除拷贝构造和赋值操作，防止复制实例
    SceneMap(const SceneMap&) = delete;
    SceneMap& operator=(const SceneMap&) = delete;

    // 重新初始化事件监听器（用于场景切换后）
    void reinitializeEventListeners();

    virtual bool init() override;
    virtual void onEnter() override;
    
    // 析构函数 - 确保正确清理资源
    virtual ~SceneMap();

    // =======================update qy===================================
    //  对外开放核心方法
    void calculateTotalGoldProduceSpeed();  // 计算金矿总产速
    void refreshResourceUI();               // 产出金币后刷新UI（原有）
    void refreshResourceImmediately();      // 新增：主动操作（升级等）后即时刷新UI
    // 新增：金币容量计算方法声明
    int getSingleBuildingGoldCapacity(Building* building); // 获取单个建筑容量
    void calculateTotalGoldCapacity();                      // 计算全局总容量
    //  圣水系统（新增）
    void calculateTotalElixirProduceSpeed();
    int getSingleBuildingElixirCapacity(Building* building);
    void calculateTotalElixirCapacity();
    // =========================update qy===================================

private:
    // 构造函数私有化 
    SceneMap();

    // 静态实例指针
    static SceneMap* sInstance;

    HomeVillageMap* _homeVillageMap;
    Vector<Building*> _buildings;
    BuildingPreview* _buildingPreview;

    // UI
    Label* _goldLabel;
    Label* _elixirLabel;
    Label* _populationLabel;
    // =========================update qy===================================
    Sprite* _goldIcon;
    Sprite* _elixirIcon;
    // =========================update qy===================================

    // 建筑放置状态
    bool _isPlacingBuilding;
    bool _isMovingBuilding;
    BuildingType _selectedBuildingType;
    Building* _selectedBuilding;  // 选中的建筑（用于移动）

    void setupUI(); 
    void onBattleButtonClicked(Ref* sender);
    void onShopButtonClicked(Ref* sender);  // 商店按钮回调
    bool onMapTouched(Touch* touch, Event* event);
    void onMouseMoved(Event* event);  // 鼠标移动事件（用于预览）

    // 建筑放置相关方法
    void checkPendingBuildingPlacement();  // 检查来自商店的建筑放置请求
    void startBuildingPlacement(BuildingType buildingType);  // 开始建筑放置
    void resetBuildingPlacementState(); // 重置建筑放置状态

    //void update(float dt) override;

    void onEnterTransitionDidFinish();
    void onTroopSelectionButtonClicked(Ref* pSender);

    // =========================update qy===================================
    void setupResourceUI();

    // =========================update qy===================================

};

#endif // __SCENE_MAP_H__

