#pragma once
/*************************************************************
* @file     : HomeVillageMap.h
* @function ：家乡基地类
* @author   : 叶芷含
* @note     ：包含地图网格系统、建筑放置、碰撞检测等核心功能
**************************************************************/

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
