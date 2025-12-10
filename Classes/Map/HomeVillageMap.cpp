/*************************************************************
* @file     : HomeVillageMap.cpp
* @function ：家乡基地类
* @author   : 叶芷含
* @note     ：
**************************************************************/
#include "HomeVillageMap.h"
#include "Constant/Constant.h"

// 初始化静态实例指针
HomeVillageMap* HomeVillageMap::sInstance = nullptr;

// 构造函数
HomeVillageMap::HomeVillageMap() {
	// 构造函数实现
}

// 析构函数
HomeVillageMap::~HomeVillageMap() {
}

// 获取单例实例
HomeVillageMap* HomeVillageMap::getInstance() {
	if (!sInstance) {
		sInstance = new (std::nothrow) HomeVillageMap();
		if (sInstance && sInstance->init(ResPath::TMX_HOMEVILLAGEMAP)) {
			sInstance->autorelease();
		}
		else {
			CC_SAFE_DELETE(sInstance);
		}
	}
	return sInstance;
}

// 初始化地图
bool HomeVillageMap::init(const std::string& tmxFile) {
	if (!SceneMap::init(tmxFile)) {
		return false;
	}
	// 加载地图
	// 



	return true;
}