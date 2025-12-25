#ifndef __MAP_LAYER_H__
#define __MAP_LAYER_H__

#include "cocos2d.h"
#include "BaseMap.h"
#include "Building/BuildingData.h" 

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

    // Specific to HomeVillage: placing buildings
    void setPendingBuildingPlacement(BuildingType buildingType) {
        _pendingBuildingType = buildingType;
        _hasPendingBuilding = true;
    }

    void startBuildingPlacement(BuildingType buildingType);

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
