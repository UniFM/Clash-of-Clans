/*************************************************************
* @file     : SceneMap.cpp
* @function ：所有地图的基类实现 - 部落冲突地图系统
* @author   : 叶芷含
* @note     ：实现地图网格系统、建筑放置、碰撞检测等核心功能
**************************************************************/

#include "SceneMap.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>

USING_NS_CC;

// 初始化瓦片地图
bool SceneMap::init(const std::string& tmxFile) {
	if (!Node::init()) {
	   return false;
	}

	// 创建并添加TMX地图
	tileMap = TMXTiledMap::create(tmxFile);
	if (!tileMap) {
		return false;
	}
	this->addChild(tileMap);

	return true;
}

// 获取地图层
TMXLayer* SceneMap::getLayer(const std::string& layerName) const {
	if (tileMap) {
		return tileMap->getLayer(layerName);
	}
	return nullptr;
}

