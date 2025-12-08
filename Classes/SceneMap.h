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
#include <vector>
#include <unordered_map>

USING_NS_CC;

// 前向声明
class Building;
class Unit;
class Resource;

// 地图类型枚举
enum class MapType {
    HomeVillage,        // 家乡基地
    BuilderBase,       // 建筑大师基地
    BattleMap,         // 战斗地图
};

// 地形类型枚举
enum class TerrainType {
    Grass,              // 草地
    Water,              // 水域
    Rock,               // 岩石
    Buildable
};

// 地图网格结构
struct MapTile {
    TerrainType terrain;
    bool isOccupied;
    bool isBuildable;
    Node* occupyingObject;  // 占用此格子的对象

    MapTile() : terrain(TerrainType::Grass), isOccupied(false), isBuildable(false),
        occupyingObject(nullptr) {
    }
};

// SceneMap类
class SceneMap : public Node {
public:
    // 创建方法
    static SceneMap* create(MapType type, int width, int height);
    // 初始化
    virtual bool init() override;
    virtual bool initWithSize(MapType type, int width, int height);

    // 析构函数
    virtual ~SceneMap();

    //  地图基本属性 
    MapType getMapType() const { return mapType; }
    Size getMapSize() const { return Size(mapWidth, mapHeight); }
    float getTileSize() const { return tileSize; }

    //  坐标转换 
    // 世界坐标转换为网格坐标
    Vec2 worldToGrid(const Vec2& worldPos) const;
    // 网格坐标转换为世界坐标
    Vec2 gridToWorld(const Vec2& gridPos) const;
    // 网格坐标转换为世界坐标（中心点）
    Vec2 gridToWorldCenter(const Vec2& gridPos) const;

    //  网格操作 
    // 检查网格坐标是否有效
    bool isValidGridPos(const Vec2& gridPos) const;
    bool isValidGridPos(int x, int y) const;

    // 获取指定网格的信息
    MapTile* getTile(const Vec2& gridPos);
    MapTile* getTile(int x, int y);
    const MapTile* getTile(const Vec2& gridPos) const;
    const MapTile* getTile(int x, int y) const;

    // 设置地形类型
    void setTerrain(const Vec2& gridPos, TerrainType terrain);
    void setTerrain(int x, int y, TerrainType terrain);

    //  建筑放置系统 
    // 检查区域是否可以放置建筑
    bool canPlaceBuilding(const Vec2& gridPos, const Size& buildingSize) const;

    // 放置建筑
    bool placeBuilding(Building* building, const Vec2& gridPos);

    // 移除建筑
    bool removeBuilding(Building* building);
    bool removeBuilding(const Vec2& gridPos);

    // 获取指定位置的建筑
    Building* getBuildingAt(const Vec2& gridPos) const;

    //  碰撞检测 
    // 检查点碰撞
    bool isPointBlocked(const Vec2& worldPos) const;

    // 检查矩形区域碰撞
    bool isAreaBlocked(const Rect& area) const;

    // 获取从起点到终点的路径（A*寻路）
    std::vector<Vec2> findPath(const Vec2& start, const Vec2& end) const;

    //  单位管理 
    // 添加单位到地图
    void addUnit(Unit* unit);

    // 移除单位
    void removeUnit(Unit* unit);

    // 获取指定区域内的所有单位
    std::vector<Unit*> getUnitsInArea(const Rect& area) const;

    // 获取最近的敌方单位
    Unit* getNearestEnemyUnit(const Vec2& position, float maxRange) const;

    //  资源管理 
    // 添加资源
    void addResource(Resource* resource, const Vec2& gridPos);

    // 移除资源
    void removeResource(const Vec2& gridPos);

    //  地图渲染 
    // 绘制网格线（调试用）
    void drawGrid(bool show);

    //  地图事件 
    // 触摸事件处理
    virtual bool onTouchBegan(Touch* touch, Event* event);
    virtual void onTouchMoved(Touch* touch, Event* event);
    virtual void onTouchEnded(Touch* touch, Event* event);

    //  地图更新 
    virtual void update(float delta) override;

protected:
    //  内部方法 
    // 初始化地图数据
    void initializeMapData();

    // 初始化渲染层
    void initializeRenderLayers();

    // 创建背景
    void createBackground();

    // 更新网格占用状态
    void updateTileOccupancy(const Vec2& gridPos, const Size& size, bool occupied, Node* object = nullptr);

    // 寻路算法辅助方法
    float getHeuristic(const Vec2& a, const Vec2& b) const;
    std::vector<Vec2> getNeighbors(const Vec2& pos) const;
    bool isWalkable(const Vec2& gridPos) const;

protected:
    //  地图属性 
    MapType mapType;                           // 地图类型
    int mapWidth, mapHeight;                  // 地图网格尺寸
    float tileSize;                            // 单个网格的像素大小

    //  地图数据 (关于瓦点地图)
    std::vector<std::vector<MapTile>> mapData; // 地图网格数据
    std::vector<cocos2d::TMXLayer*> layers;    // 用来存储TMX地图的各个层

    //  对象管理 
    std::vector<Building*> buildings;          // 地图上的建筑
    std::vector<Unit*> units;                  // 地图上的单位
    std::unordered_map<int, Resource*> resources; // 资源对象 (key = x*1000+y)

    //  渲染层 
    Node* backgroundLayer;                     // 背景层
    Node* buildingLayer;                       // 建筑层
    Node* unitLayer;                          // 单位层
    Node* effectLayer;                        // 特效层
    Node* uiLayer;                            // UI层

    //  调试显示 
    DrawNode* gridDrawNode;                   // 网格线绘制节点
    DrawNode* highlightDrawNode;              // 高亮区域绘制节点

    //  触摸处理 
    EventListenerTouchOneByOne* touchListener; // 触摸监听器

    //  缓存数据 
    mutable std::vector<Vec2> pathCache;      // 路径缓存
};

#endif