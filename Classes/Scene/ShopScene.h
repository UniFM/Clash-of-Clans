#pragma once
/*************************************************************
* @file     : ShopScene.h  
* @function ：商店场景 - 部落冲突商店系统
* @author   : 叶芷含
* @note     ：包含建筑购买、分类浏览、滑动展示等功能
**************************************************************/

#ifndef __SHOP_SCENE_H__
#define __SHOP_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Constant/Constant.h"
#include "Map/SceneMap.h"
#include "Building/BuildingData.h"
#include "LoginScene.h"

USING_NS_CC;
using namespace ui;

// 商店分类枚举
enum class ShopCategory {
    ARMY,       // 军队 (兵营等)
    RESOURCES,  // 资源 (金矿、圣水收集器等)  
    DEFENSE,    // 防御 (弓箭塔、加农炮等)
    TRAPS       // 陷阱
};

class ShopScene : public Scene {
public:
    // 创建场景
    static ShopScene* create();
    
    // 初始化
    virtual bool init() override;
    
    // 设置回调 - 当购买建筑时通知外部
    void setPurchaseCallback(const std::function<void(BuildingType)>& callback) {
        purchaseCallback = callback;
    }

private:
    // UI组件
    Node*backgroundNode;           // 背景容器
    Node* categoryTabsNode;         // 分类标签容器  
    ScrollView* buildingScrollView; // 建筑滑动视图
    Node* resourceBarNode;          // 资源显示条
    Button* closeButton;            // 关闭按钮
    
    // 分类标签
    std::vector<Button*> categoryButtons;  // 分类按钮组
    ShopCategory currentCategory;           // 当前选中的分类

    // 错误标签
    cocos2d::Label* statusLabel;
    
    // 建筑展示
    std::vector<Node*> buildingItems;      // 当前显示的建筑项目
    
    // 回调函数
    std::function<void(BuildingType)> purchaseCallback;  // 购买回调
    
    // 初始化方法
    void initBackground();           // 初始化背景
    void initCategoryTabs();         // 初始化分类标签
    void initBuildingScrollView();   // 初始化建筑滑动视图
    void initResourceBar();          // 初始化资源显示条
    void initCloseButton();          // 初始化关闭按钮
    
    // 分类切换
    void onCategorySelected(ShopCategory category);  // 分类选择回调
    void updateCategoryButtons();                     // 更新分类按钮状态
    void refreshBuildingList();                       // 刷新建筑列表
    
    // 建筑展示
    Node* createBuildingItem(BuildingType buildingType);  // 创建单个建筑项目
    void onBuildingSelected(BuildingType buildingType);   // 建筑选择回调
    
    // 数据获取
    std::vector<BuildingType> getBuildingsByCategory(ShopCategory category); // 根据分类获取建筑
    
    // 资源相关
    void updateResourceDisplay();     // 更新资源显示
    bool canAffordBuilding(BuildingType buildingType, int level = 1); // 检查是否买得起
    
    // 事件处理
    void onCloseButtonClicked(Ref* sender); // 关闭按钮点击
    void onBackKeyPressed();                // 返回键处理
    void enterVillageWithBuildingPlacement(BuildingType buildingType); // 进入村庄并开始建筑放置
};

#endif // __SHOP_SCENE_H__