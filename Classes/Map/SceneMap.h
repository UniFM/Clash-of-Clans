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

USING_NS_CC; 

class SceneMap : public Node {
public:
	// 初始化瓦片地图
	virtual bool init(const std::string& tmxFile);

protected:
	// 获取地图层
	TMXLayer* getLayer(const std::string& layerName) const;

	// 地图对象
	TMXTiledMap* tileMap;
};

#endif