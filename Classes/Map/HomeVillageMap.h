#pragma once
/*************************************************************
* @file     : HomeVillageMap.h
* @function ：家乡基地类
* @author   : 叶芷含
* @note     ：包含地图网格系统、建筑放置、碰撞检测等核心功能
**************************************************************/

#include "cocos2d.h"
#include "SceneMap.h"
#include "Constant/Constant.h"
#include "Building/BuildingData.h"

USING_NS_CC;

class HomeVillageMap : public SceneMap {
public:
	// 使用单例，提供全局访问
	static HomeVillageMap* getInstance();

	// 删除拷贝构造和赋值操作，防止复制实例
	HomeVillageMap(const HomeVillageMap&) = delete;
	HomeVillageMap& operator=(const HomeVillageMap&) = delete;

	// 初始化地图
	bool init(const std::string& tmxFile) override;

	// 重写基类方法，添加家乡基地特有逻辑
	bool canPlaceBuilding(const Vec2& pos, const Size& buildingSize) const override;

	// 家乡基地特有的检测方法
	bool isOnGrassland(const Vec2& pos, const Size& buildingSize) const;  // 检查是否完全在草地上

	// 建筑放置相关方法
	void startBuildingPlacement(BuildingType buildingType);  // 开始建筑放置模式
	void endBuildingPlacement();                             // 结束建筑放置模式
	bool isInBuildingPlacementMode() const { return isPlacingBuilding; }

	// 析构函数公开，允许正常释放
	virtual ~HomeVillageMap();

private:
	// 构造函数私有化 
	HomeVillageMap();

	// 静态实例指针
	static HomeVillageMap* sInstance;

	// TMX地图层
	TMXLayer* backgroundLayer;   // 背景层
	TMXLayer* grassLayer;        // 草地层

	// 建筑放置相关
	bool isPlacingBuilding;           // 是否处于建筑放置模式
	BuildingType currentBuildingType; // 当前要放置的建筑类型
	Sprite* buildingPreview;          // 建筑预览精灵
	
	// 建筑放置相关事件处理
	void onTouchBeganForBuilding(const Vec2& touchPos);
	void onTouchMovedForBuilding(const Vec2& touchPos);
	void onTouchEndedForBuilding(const Vec2& touchPos);
	
	// 更新建筑预览位置和状态
	void updateBuildingPreview(const Vec2& worldPos);
	
	// 确认放置建筑
	void placeBuildingAtPosition(const Vec2& pos);
};