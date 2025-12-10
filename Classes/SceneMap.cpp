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

// 创建方法，与cocos引擎的代码风格保持一致
SceneMap* SceneMap::create(MapType type, int width, int height) {
    SceneMap* map = new (std::nothrow) SceneMap();
    if (map && map->initWithSize(type, width, height)) {
        map->autorelease();
        return map;
    }
    CC_SAFE_DELETE(map);
    return nullptr;
}

// 初始化
bool SceneMap::init() {
    if (!Node::init()) {
        return false;
    }

    // 默认初始化为32x32的家乡基地Map
    return initWithSize(MapType::HomeVillage, 32, 32);
}

bool SceneMap::initWithSize(MapType type, int width, int height) {
    if (!Node::init()) {
        return false;
    }

    mapType = type;
    mapWidth = width;
    mapHeight = height;
    tileSize = 32.0f; // 默认32像素一个格子
    touchListener = nullptr;
    gridDrawNode = nullptr;
    highlightDrawNode = nullptr;

    // 初始化地图数据
    initializeMapData();

    // 初始化渲染层
    initializeRenderLayers();

    // 创建背景
    createBackground();

    // 设置触摸监听
    touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    touchListener->onTouchBegan = CC_CALLBACK_2(SceneMap::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(SceneMap::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(SceneMap::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    // 启用更新
    scheduleUpdate();

    return true;
}

SceneMap::~SceneMap() {
    // 清理资源
    if (touchListener) {
        _eventDispatcher->removeEventListener(touchListener);
    }
}

// 坐标转换 
Vec2 SceneMap::worldToGrid(const Vec2& worldPos) const {
    Vec2 localPos = convertToNodeSpace(worldPos);
    return Vec2(floor(localPos.x / tileSize), floor(localPos.y / tileSize));
}

Vec2 SceneMap::gridToWorld(const Vec2& gridPos) const {
    Vec2 localPos = Vec2(gridPos.x * tileSize, gridPos.y * tileSize);
    return convertToWorldSpace(localPos);
}

Vec2 SceneMap::gridToWorldCenter(const Vec2& gridPos) const {
    Vec2 localPos = Vec2(gridPos.x * tileSize + tileSize * 0.5f,
        gridPos.y * tileSize + tileSize * 0.5f);
    return convertToWorldSpace(localPos);
}

// 网格操作 
bool SceneMap::isValidGridPos(const Vec2& gridPos) const {
    return isValidGridPos(gridPos.x, gridPos.y);
}

bool SceneMap::isValidGridPos(int x, int y) const {
    return x >= 0 && x < mapWidth && y >= 0 && y < mapHeight;
}

MapTile* SceneMap::getTile(const Vec2& gridPos) {
    return getTile(gridPos.x, gridPos.y);
}

MapTile* SceneMap::getTile(int x, int y) {
    if (!isValidGridPos(x, y)) {
        return nullptr;
    }
    return &mapData[y][x];
}

const MapTile* SceneMap::getTile(const Vec2& gridPos) const {
    return getTile(gridPos.x, gridPos.y);
}

const MapTile* SceneMap::getTile(int x, int y) const {
    if (!isValidGridPos(x, y)) {
        return nullptr;
    }
    return &mapData[y][x];
}

void SceneMap::setTerrain(const Vec2& gridPos, TerrainType terrain) {
    setTerrain(gridPos.x, gridPos.y, terrain);
}

void SceneMap::setTerrain(int x, int y, TerrainType terrain) {
    MapTile* tile = getTile(x, y);
    if (tile) {
        tile->terrain = terrain;
        tile->isBuildable = (terrain == TerrainType::Grass || terrain == TerrainType::Buildable);
    }
}

// 建筑放置系统 
bool SceneMap::canPlaceBuilding(const Vec2& gridPos, const Size& buildingSize) const {
    int startX = gridPos.x;
    int startY = gridPos.y;
    int endX = startX + buildingSize.width;
    int endY = startY + buildingSize.height;

    // 检查所有网格是否可用
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            if (!isValidGridPos(x, y)) {
                return false;
            }

            const MapTile* tile = getTile(x, y);
            if (!tile || !tile->isBuildable || tile->isOccupied) {
                return false;
            }
        }
    }

    return true;
}

bool SceneMap::placeBuilding(Building* building, const Vec2& gridPos) {
    // 暂时注释掉实现，因为Building类只是前向声明，无法确定其基类
    // 需要Building类的完整定义或确认Building继承自Node
    CCLOG("SceneMap::placeBuilding - Method needs Building class implementation");
    return false;

    /*
    if (!building) {
        return false;
    }

    // 这里需要根据实际的Building类来获取建筑尺寸
    // Size buildingSize = building->getBuildingSize();
    Size buildingSize = Size(2, 2); // 临时假设建筑大小为2x2

    if (!canPlaceBuilding(gridPos, buildingSize)) {
        return false;
    }

    // 更新网格占用状态
    updateTileOccupancy(gridPos, buildingSize, true, building);

    // 添加建筑到建筑层
    _buildingLayer->addChild(building);
    _buildings.push_back(building);

    // 设置建筑位置
    building->setPosition(gridToWorldCenter(gridPos));

    return true;
    */
}

// 碰撞检测 
bool SceneMap::isPointBlocked(const Vec2& worldPos) const {
    Vec2 gridPos = worldToGrid(worldPos);
    const MapTile* tile = getTile(gridPos);
    return !tile || tile->isOccupied || tile->terrain == TerrainType::Water ||
        tile->terrain == TerrainType::Rock;
}

bool SceneMap::isAreaBlocked(const Rect& area) const {
    Vec2 topLeft = worldToGrid(Vec2(area.getMinX(), area.getMaxY()));
    Vec2 bottomRight = worldToGrid(Vec2(area.getMaxX(), area.getMinY()));

    for (int y = topLeft.y; y <= bottomRight.y; ++y) {
        for (int x = topLeft.x; x <= bottomRight.x; ++x) {
            if (isPointBlocked(gridToWorld(Vec2(x, y)))) {
                return true;
            }
        }
    }

    return false;
}

// 初始化地图数据
void SceneMap::initializeMapData() {
    mapData.clear();
    mapData.resize(mapHeight);

    for (int y = 0; y < mapHeight; ++y) {
        mapData[y].resize(mapWidth);
        for (int x = 0; x < mapWidth; ++x) {
            mapData[y][x] = MapTile();
            // 根据地图类型设置初始地形
            if (mapType == MapType::HomeVillage) {
                mapData[y][x].terrain = TerrainType::Grass;
                mapData[y][x].isBuildable = true;
            }
        }
    }
}

void SceneMap::initializeRenderLayers() {
    // 创建渲染层级
    backgroundLayer = Node::create();
    buildingLayer = Node::create();
    unitLayer = Node::create();
    effectLayer = Node::create();
    uiLayer = Node::create();

    // 添加到当前节点，注意层级顺序
    addChild(backgroundLayer, 0);
    addChild(buildingLayer, 1);
    addChild(unitLayer, 2);
    addChild(effectLayer, 3);
    addChild(uiLayer, 4);

    // 创建调试绘制节点
    gridDrawNode = DrawNode::create();
    highlightDrawNode = DrawNode::create();
    uiLayer->addChild(gridDrawNode);
    uiLayer->addChild(highlightDrawNode);
}

void SceneMap::createBackground() {
    // 创建简单的草地背景
    auto bgSprite = Sprite::create(); // 这里应该使用实际的背景贴图
    if (bgSprite) {
        bgSprite->setContentSize(Size(mapWidth * tileSize, mapHeight * tileSize));
        bgSprite->setAnchorPoint(Vec2::ZERO);
        bgSprite->setColor(Color3B(34, 139, 34)); // 草绿色
        backgroundLayer->addChild(bgSprite);
    }
}

// 更新网格占用状态
void SceneMap::updateTileOccupancy(const Vec2& gridPos, const Size& size, bool occupied, Node* object) {
    int startX = gridPos.x;
    int startY = gridPos.y;
    int endX = startX + size.width;
    int endY = startY + size.height;

    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            MapTile* tile = getTile(x, y);
            if (tile) {
                tile->isOccupied = occupied;
                tile->occupyingObject = occupied ? object : nullptr;
            }
        }
    }
}

// 调试功能 
void SceneMap::drawGrid(bool show) {
    if (!gridDrawNode) return;

    gridDrawNode->clear();

    if (!show) return;

    // 绘制网格线
    for (int x = 0; x <= mapWidth; ++x) {
        Vec2 start(x * tileSize, 0);
        Vec2 end(x * tileSize, mapHeight * tileSize);
        gridDrawNode->drawLine(start, end, Color4F::GRAY);
    }

    for (int y = 0; y <= mapHeight; ++y) {
        Vec2 start(0, y * tileSize);
        Vec2 end(mapWidth * tileSize, y * tileSize);
        gridDrawNode->drawLine(start, end, Color4F::GRAY);
    }
}

// 触摸事件处理 
bool SceneMap::onTouchBegan(Touch* touch, Event* event) {
    Vec2 touchPos = touch->getLocation();
    Vec2 gridPos = worldToGrid(touchPos);

    CCLOG("Touch at grid position: (%.0f, %.0f)", gridPos.x, gridPos.y);

    return true;
}

void SceneMap::onTouchMoved(Touch* touch, Event* event) {
    // 处理拖拽逻辑
}

void SceneMap::onTouchEnded(Touch* touch, Event* event) {
    // 处理触摸结束逻辑
}

// 更新方法 
void SceneMap::update(float delta) {
    Node::update(delta);

    // 更新所有单位
    for (auto unit : units) {
        // unit->update(delta);
    }

    // 其他地图逻辑更新
}