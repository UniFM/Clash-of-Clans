#pragma once
/*************************************************************
* @file     : Constant.h
* @function ：常量类
* @author   : 叶芷含 俞筱雨
* @note     ：常量路径
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

// Team ID
enum class Team {
    PLAYER,
    ENEMY
};

// 兵种类型枚举
enum class Troops {
    Barbarian,   // 野蛮人
    Archer,    // 弓箭手
    //WallBreaker,    //炸弹人
    Giant,  //巨人
    Goblin  //哥布林
};

// Z-Orders
enum ZOrder {
    Z_BACKGROUND = 0,
    Z_BUILDING = 10,
    Z_TROOP = 20,
    Z_PROJECTILE = 30,
    Z_UI = 100
};

// Tags
enum Tags {
    TAG_BUILDING = 1,
    TAG_TROOP = 2,
    TAG_PROJECTILE = 3
};

// 地图参数
//constexpr int MAP_SIZE_WIDTH = 968;
//constexpr int MAP_SIZE_HEIGHT = 968;    // 地图大小
//constexpr int Tile_SIZE_WIDTH = 16;
//constexpr int Tile_SIZE_HEIGHT = 16;    // 瓦片大小

// 建筑类型
enum class BattleBuildingType {
    TOWN_HALL,
    GOLD_MINE,
    ELIXIR_COLLECTOR,
    GOLD_STORAGE,
    ELIXIR_STORAGE,
    ARMY_CAMP,
    BARRACKS,
    CANNON,
    ARCHER_TOWER,
    WALL
};

enum class BattleResourceType
{
    GOLD,    // 金币（建造/升级建筑）
    ELIXIR,  // 圣水（训练兵种）
    GEMS     // 宝石（加速/付费道具）
};

struct BattleBuildingStats {
    std::string name;
    int maxHealth;
    int buildTime; // seconds
    int costGold;
    int costElixir;
    float attackRange;
    float attackSpeed;
    int damage;
    int capacity; // For storage or camp
};

struct BattleTroopStats {
    std::string name;
    int maxHealth;
    int damage;
    float moveSpeed;
    float attackRange;
    float attackSpeed;
    int trainingTime;
    int housingSpace;
    int trainingCost; // Elixir
};

// 战斗兵种枚举
enum class BattleTroopType {
    BARBARIAN,
    ARCHER,
    GIANT,
    GOBLIN
};

constexpr float MAP_WIDTH = 3460.0f;
constexpr float MAP_HEIGHT = 2480.0f;
constexpr float GRASS_RECT_WIDTH = 2400.0f;
constexpr float GRASS_RECT_HEIGHT = 1800.0f;
constexpr float GRASS_OFFSET_X = 530.0f;
constexpr float GRASS_OFFSET_Y = 252.0f;
constexpr  int GRID_COLS = 44;
constexpr int GRID_ROWS = 44;

// 资源路径,避免全局冲突
namespace ResPath {
	//根路径
	constexpr const char* ROOT = "";

    //Scene路径
    constexpr const char* SPLASHSCENE = "Scene/Cover.png";    //SplashScene游戏开屏
    constexpr const char* LOGINBACKGROUND = "Scene/LoginBackground.png";

    //PNG地图
	const std::string HOMEVILLAGEMAP = "Map/HomeVillage.png";    // 家乡基地

    // 显示屏大小,设计比例 16:9
    const cocos2d::Size WINDOWS(1280.0f, 720.0f);




    //兵种Sprite


    //图标icons
    constexpr const char* ZOOMINBUTTON = "Icon/zoom_in_button.png";
    constexpr const char* ZOOMOUTBUTTON = "Icon/zoom_out_button.png";
    constexpr const char* ZOOMINBUTTONPRESSED = "Icon/zoom_in_button_pressed.png";
    constexpr const char* ZOOMOUTBUTTONPRESSED = "Icon/zoom_out_button_pressed.png";
    constexpr const char* SHOP = "Icon/shop.png";
    constexpr const char* CLOSEBUTTON = "Icon/close_button.png";
    constexpr const char* CLOSEBUTTONPRESSED = "Icon/close_button_pressed.png";
    constexpr const char* SHOPPRESSED = "Icon/shop_pressed.png";
    constexpr const char* UNBUILDABLEBUTTON = "Icon/unbuildable_button.png";
    constexpr const char* CONFIRMCONSTRUCTIONBUTTON = "Icon/confirm_construction_button.png";
    constexpr const char* CANCELCONSTRUCTIONBUTTON = "Icon/cancel_construction_button.png";

    //商店shop
    constexpr const char* SHOPBACKGROUND = "Shop/ShopBackground.png";
    constexpr const char* SHOPCATEGORYGREENBUTTON = "Shop/green.png";
    constexpr const char* SHOPCATEGORYGREYGREENBUTTON = "Shop/greygreen.png";
    constexpr const char* AVAILABLEBUILDINGCARDBACKGROUND = "Shop/availableBuildingCardBackground.png";
    constexpr const char* UNAVAILABLEBUILDINGCARDBACKGROUND = "Shop/unavailableBuildingCardBackground.png";


    //建筑building
    // 军队建筑
    constexpr const char* BARRACKSLEVEL1 = "buildings/Barracks1.png";

    //防御建筑
    constexpr const char* CANNONLEVEL1 = "buildings/Cannon1.png";
    constexpr const char* ARCHERTOWERLEVEL1 = "buildings/Archer_Tower1.png";

    //资源建筑
    constexpr const char* ELIXIRCOLLECTORLEVEL1 = "buildings/Elixir_Collector1.png";
    constexpr const char* GOLDMINELEVEL1 = "buildings/Gold_Mine1.png";
    constexpr const char* ELIXIRSTORAGELEVEL1 = "buildings/Elixir_Storage1.png";
    constexpr const char* GOLDSTORAGELEVEL1 = "buildings/Gold_Storage1.png";

    constexpr const char* TOWNHALLLEVEL1 = "buildings/Town_Hall1.png";

    // 音效
    constexpr const char* SOUND_BUTTON = "sounds/button.mp3";
    constexpr const char* SOUND_BG_MUSIC = "music/backgroundMusic_1.mp3";


}
