#pragma once
/*************************************************************
* @file     : Constant.h
* @function ：常量类
* @author   : 叶芷含
* @note     ：包含整个项目中的所有常量
**************************************************************/
#include "cocos2d.h"
#include <string>

// 地图类型枚举
enum class MapType {
    HomeVillage,        // 家乡基地
    BuilderBase,       // 建筑大师基地
    BattleMap,         // 战斗地图
};

// 地形类型枚举
enum class TerrainType {
    Grass,              // 草地
    //Water,              // 水域
    //Buildable
};

// 地图参数
constexpr int MAP_SIZE_WIDTH = 968;
constexpr int MAP_SIZE_HEIGHT = 968;    // 地图大小
constexpr int Tile_SIZE_WIDTH = 16;
constexpr int Tile_SIZE_HEIGHT = 16;    // 瓦片大小

// 资源路径,避免全局冲突
namespace ResPath {
	//根路径
	constexpr const char* ROOT = "";

    //Scene路径
    constexpr const char* SPLASHSCENE = "Scene/Cover.png";    //SplashScene游戏开屏


    //瓦片地图TMX
	const std::string TMX_HOMEVILLAGEMAP = "Map/HomeVillage.tmx";    // 家乡基地


    //兵种Sprite

}
