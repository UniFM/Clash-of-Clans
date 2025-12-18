#pragma once
/*************************************************************
* @file     : SceneMap.h
* @function ：所有地图的基类 - 部落冲突地图系统
* @author   : 叶芷含
* @note     ：包含地图网格系统、建筑放置、碰撞检测等核心功能
**************************************************************/

#ifndef __SCENEMAP_H__
#define __SCENEMAP_H__
#include "cocos2d.h"
#include "Constant/Constant.h"
#include "Control/Control.h"

USING_NS_CC;

class SceneMap : public Node {
public:
	// 初始化瓦片地图
	virtual bool init(const std::string& tmxFile);

	// 通用碰撞检测接口
	virtual bool isPositionValid(const Vec2& pos) const;    // 检测位置是否合法
	virtual bool canPlaceBuilding(const Vec2& pos, const Size& buildingSize) const;    // 检测建筑能否放置
	virtual TerrainType getTerrainType(const Vec2& pos) const;    // 获取地形类型
	Size getMapSize() const;    // 获取地图尺寸
	Size getTileSize() const;    // 获取瓦片尺寸
	TMXTiledMap* getTiledMap() const { return tileMap; }  // 获取TMX瓦片地图对象
	cocos2d::Vec2 TMXToCocos2d(const cocos2d::Vec2& tmxPos) const;
	cocos2d::Vec2 Cocos2dToTMX(const cocos2d::Vec2& cocosPos) const;

	// 触摸事件处理
	bool onTouchBegan(Touch* touch, Event* event);
	void onTouchMoved(Touch* touch, Event* event);
	void onTouchEnded(Touch* touch, Event* event);

protected:
	// 获取地图层
	TMXLayer* getLayer(const std::string& layerName) const;

	// 基础碰撞检测方法
	bool checkTileCollision(const Vec2& pos) const;    // 检测单个瓦片碰撞
	bool isWithinMapBounds(const Vec2& pos) const;    // 检测位置是否在地图边界内
	TMXLayer* getCollisionLayer() const;    // 获取碰撞层

	// 地图对象
	TMXTiledMap* tileMap;
	TMXLayer* collisionLayer;  // 碰撞检测层

	// 滚动相关
	Vec2 lastTouchPos;  // 上次触摸位置

	// 设置滚动视图
	void setupScrollView();

};

#endif