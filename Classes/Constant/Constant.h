#pragma once
/*************************************************************
* @file     : Constant.h
* @function ：常量类
* @author   : 叶芷含
* @note     ：包含整个项目中的所有常量
**************************************************************/
#include "cocos2d.h"
#include <string>

static cocos2d::Size smallResolutionSize = cocos2d::Size(1280, 720);   // 小屏幕
static cocos2d::Size mediumResolutionSize = cocos2d::Size(1280, 720);  // 中等屏幕
static cocos2d::Size largeResolutionSize = cocos2d::Size(1920, 1080);  // 大屏幕

// 地图类型枚举
enum class MapType {
    HomeVillage,        // 家乡基地
    BuilderBase,       // 建筑大师基地
    BattleMap,         // 战斗地图
};

// 地形类型枚举
enum class TerrainType {
    Grass,              // 草地
    Occupied            // 已占用
    //Water,              // 水域
    //Buildable
};

// 兵种类型枚举
enum class Troops {
    Barbarian,   // 野蛮人
    Archer,    // 弓箭手
    WallBreaker,    //炸弹人
    Giant,  //巨人
    Goblin  //哥布林
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
    constexpr const char* LOGINBACKGROUND = "Scene/LoginBackground.png";

    //瓦片地图TMX
	const std::string TMX_HOMEVILLAGEMAP = "Map/HomeVillage.tmx";    // 家乡基地

    // 显示屏大小,设计比例 16:9
    const cocos2d::Size WINDOWS(1280.0f, 720.0f);




    //兵种Sprite


    //图标icons
    constexpr const char* ZOOMINBUTTON = "Icon/zoom_in_button.png";
    constexpr const char* ZOOMOUTBUTTON = "Icon/zoom_out_button.png";
    constexpr const char* ZOOMINBUTTONPRESSED = "Icon/zoom_in_button_pressed.png";
    constexpr const char* ZOOMOUTBUTTONPRESSED = "Icon/zoom_out_button_pressed.png";
}
