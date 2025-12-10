#pragma once
/*************************************************************
* @file     : HomeVillageMap.h
* @function ：家乡基地类
* @author   : 叶芷含
* @note     ：包含地图网格系统、建筑放置、碰撞检测等核心功能
**************************************************************/

#include "cocos2d.h"
#include "SceneMap.h"

class HomeVillageMap : public SceneMap {
public:
	// 使用单例，提供全局访问
	static HomeVillageMap* getInstance();

	// 删除拷贝构造和赋值操作，防止复制实例
	HomeVillageMap(const HomeVillageMap&) = delete;
	HomeVillageMap& operator=(const HomeVillageMap&) = delete;

	// 初始化地图
	bool init(const std::string& tmxFile) override;


private:
	// 构造函数私有化
	HomeVillageMap();
	~HomeVillageMap();

	// 静态实例指针
	static HomeVillageMap* sInstance;


};