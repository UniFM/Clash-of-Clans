/*************************************************************
* @file     : HomeVillageMap.cpp
* @function ：家乡村庄地图核心实现 - 单例管理+兵种可视化+地图交互
* @author   : 叶芷含
* @note     : 1.单例模式实现家乡村庄地图全局唯一实例，支持创建/销毁/获取接口；
*             2.自定义家乡村庄地图尺寸、草地边界、网格数量等核心参数；
*             3.移植兵种显示逻辑，从TroopManager获取兵种数量并在地图固定位置生成可视化兵种；
*             4.兵种支持随机游荡效果，基于兵营位置生成指定半径内的游荡路径；
*             5.重写触摸事件回调，继承BaseMap基础交互逻辑；
*             6.提供建筑放置启动接口，预留建筑部署扩展能力；
*             7.初始化时自动更新兵种显示，清理旧兵种节点避免重复渲染
**************************************************************/

#ifndef __MAP_LAYER_H__
#define __MAP_LAYER_H__

#include "cocos2d.h"
#include "BaseMap.h"
#include "buildings/BuildingsData.h" 

enum class BuildingType;

class HomeVillageMap : public BaseMap
{
public:
    static HomeVillageMap* getInstance(const std::string& mapImagePath);
    static HomeVillageMap* create(const std::string& mapImagePath);
    static void destroyInstance();

    HomeVillageMap(const HomeVillageMap&) = delete;
    HomeVillageMap& operator=(const HomeVillageMap&) = delete;

    virtual bool init(const std::string& mapImagePath) override;
    virtual ~HomeVillageMap();

    // 家乡特有方法：放置建筑物
    void setPendingBuildingPlacement(BuildingType buildingType) {
        _pendingBuildingType = buildingType;
        _hasPendingBuilding = true;
    }

    void startBuildingPlacement(BuildingType buildingType);

    // [新增] 移植自 SceneMap 的功能：更新部队显示
    void updateTroopDisplay();

protected:
    virtual bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event) override;
    virtual void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event) override;
    virtual void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event) override;

private:
    HomeVillageMap();
    static HomeVillageMap* sInstance;

    bool _hasPendingBuilding;
    BuildingType _pendingBuildingType;
};

#endif // __MAP_LAYER_H__
