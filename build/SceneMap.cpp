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

// 不使用 USING_NS_CC，避免与cocos2d::SceneMap模板类冲突

// === 创建和初始化 ===
SceneMap* SceneMap::create(MapType type, int width, int height) {
    SceneMap* map = new (std::nothrow) SceneMap();
    if (map && map->initWithSize(type, width, height)) {
        map->autorelease();
        return map;
    }
    CC_SAFE_DELETE(map);
    return nullptr;
}

bool SceneMap::init() {
    if (!cocos2d::Node::init()) {
        return false;
    }
    
    // 默认初始化为32x32的玩家基地地图
    return initWithSize(MapType::PLAYER_BASE, 32, 32);
}

bool SceneMap::initWithSize(MapType type, int width, int height) {
    if (!cocos2d::Node::init()) {
        return false;
    }
    
    _mapType = type;
    _mapWidth = width;
    _mapHeight = height;
    _tileSize = 32.0f; // 默认32像素一个格子
    _touchListener = nullptr;
    _gridDrawNode = nullptr;
    _highlightDrawNode = nullptr;
    
    // 初始化地图数据
    initializeMapData();
    
    // 初始化渲染层
    initializeRenderLayers();
    
    // 创建背景
    createBackground();
    
    // 设置触摸监听
    _touchListener = cocos2d::EventListenerTouchOneByOne::create();
    _touchListener->setSwallowTouches(true);
    _touchListener->onTouchBegan = CC_CALLBACK_2(SceneMap::onTouchBegan, this);
    _touchListener->onTouchMoved = CC_CALLBACK_2(SceneMap::onTouchMoved, this);
    _touchListener->onTouchEnded = CC_CALLBACK_2(SceneMap::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_touchListener, this);
    
    // 启用更新
    scheduleUpdate();
    
    return true;
}

SceneMap::~SceneMap() {
    // 清理资源
    if (_touchListener) {
        _eventDispatcher->removeEventListener(_touchListener);
    }
}

// === 坐标转换 ===
cocos2d::Vec2 SceneMap::worldToGrid(const cocos2d::Vec2& worldPos) const {
    cocos2d::Vec2 localPos = convertToNodeSpace(worldPos);
    return cocos2d::Vec2(floor(localPos.x / _tileSize), floor(localPos.y / _tileSize));
}

cocos2d::Vec2 SceneMap::gridToWorld(const cocos2d::Vec2& gridPos) const {
    cocos2d::Vec2 localPos = cocos2d::Vec2(gridPos.x * _tileSize, gridPos.y * _tileSize);
    return convertToWorldSpace(localPos);
}

cocos2d::Vec2 SceneMap::gridToWorldCenter(const cocos2d::Vec2& gridPos) const {
    cocos2d::Vec2 localPos = cocos2d::Vec2(gridPos.x * _tileSize + _tileSize * 0.5f, 
                        gridPos.y * _tileSize + _tileSize * 0.5f);
    return convertToWorldSpace(localPos);
}

// === 网格操作 ===
bool SceneMap::isValidGridPos(const cocos2d::Vec2& gridPos) const {
    return isValidGridPos(gridPos.x, gridPos.y);
}

bool SceneMap::isValidGridPos(int x, int y) const {
    return x >= 0 && x < _mapWidth && y >= 0 && y < _mapHeight;
}

MapTile* SceneMap::getTile(const cocos2d::Vec2& gridPos) {
    return getTile(gridPos.x, gridPos.y);
}

MapTile* SceneMap::getTile(int x, int y) {
    if (!isValidGridPos(x, y)) {
        return nullptr;
    }
    return &_mapData[y][x];
}

const MapTile* SceneMap::getTile(const cocos2d::Vec2& gridPos) const {
    return getTile(gridPos.x, gridPos.y);
}

const MapTile* SceneMap::getTile(int x, int y) const {
    if (!isValidGridPos(x, y)) {
        return nullptr;
    }
    return &_mapData[y][x];
}

void SceneMap::setTerrain(const cocos2d::Vec2& gridPos, TerrainType terrain) {
    setTerrain(gridPos.x, gridPos.y, terrain);
}

void SceneMap::setTerrain(int x, int y, TerrainType terrain) {
    MapTile* tile = getTile(x, y);
    if (tile) {
        tile->terrain = terrain;
        tile->isBuildable = (terrain == TerrainType::GRASS || terrain == TerrainType::BUILDABLE);
    }
}

// === 建筑放置系统 ===
bool SceneMap::canPlaceBuilding(const cocos2d::Vec2& gridPos, const cocos2d::Size& buildingSize) const {
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

bool SceneMap::placeBuilding(Building* building, const cocos2d::Vec2& gridPos) {
    // 暂时注释掉实现，因为Building类只是前向声明，无法确定其基类
    // 需要Building类的完整定义或确认Building继承自cocos2d::Node
    CCLOG("SceneMap::placeBuilding - Method needs Building class implementation");
    return false;
    
    /*
    if (!building) {
        return false;
    }
    
    // 这里需要根据实际的Building类来获取建筑尺寸
    // Size buildingSize = building->getBuildingSize();
    cocos2d::Size buildingSize = cocos2d::Size(2, 2); // 临时假设建筑大小为2x2
    
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

// === 碰撞检测 ===
bool SceneMap::isPointBlocked(const cocos2d::Vec2& worldPos) const {
    cocos2d::Vec2 gridPos = worldToGrid(worldPos);
    const MapTile* tile = getTile(gridPos);
    return !tile || tile->isOccupied || tile->terrain == TerrainType::WATER || 
           tile->terrain == TerrainType::ROCK;
}

bool SceneMap::isAreaBlocked(const cocos2d::Rect& area) const {
    cocos2d::Vec2 topLeft = worldToGrid(cocos2d::Vec2(area.getMinX(), area.getMaxY()));
    cocos2d::Vec2 bottomRight = worldToGrid(cocos2d::Vec2(area.getMaxX(), area.getMinY()));
    
    for (int y = topLeft.y; y <= bottomRight.y; ++y) {
        for (int x = topLeft.x; x <= bottomRight.x; ++x) {
            if (isPointBlocked(gridToWorld(cocos2d::Vec2(x, y)))) {
                return true;
            }
        }
    }
    
    return false;
}

// === 初始化辅助方法 ===
void SceneMap::initializeMapData() {
    _mapData.clear();
    _mapData.resize(_mapHeight);
    
    for (int y = 0; y < _mapHeight; ++y) {
        _mapData[y].resize(_mapWidth);
        for (int x = 0; x < _mapWidth; ++x) {
            _mapData[y][x] = MapTile();
            // 根据地图类型设置初始地形
            if (_mapType == MapType::PLAYER_BASE) {
                _mapData[y][x].terrain = TerrainType::GRASS;
                _mapData[y][x].isBuildable = true;
            }
        }
    }
}

void SceneMap::initializeRenderLayers() {
    // 创建渲染层级
    _backgroundLayer = cocos2d::Node::create();
    _buildingLayer = cocos2d::Node::create();
    _unitLayer = cocos2d::Node::create();
    _effectLayer = cocos2d::Node::create();
    _uiLayer = cocos2d::Node::create();
    
    // 添加到当前节点，注意层级顺序
    addChild(_backgroundLayer, 0);
    addChild(_buildingLayer, 1);
    addChild(_unitLayer, 2);
    addChild(_effectLayer, 3);
    addChild(_uiLayer, 4);
    
    // 创建调试绘制节点
    _gridDrawNode = cocos2d::DrawNode::create();
    _highlightDrawNode = cocos2d::DrawNode::create();
    _uiLayer->addChild(_gridDrawNode);
    _uiLayer->addChild(_highlightDrawNode);
}

void SceneMap::createBackground() {
    // 创建简单的草地背景
    auto bgSprite = cocos2d::Sprite::create(); // 这里应该使用实际的背景贴图
    if (bgSprite) {
        bgSprite->setContentSize(cocos2d::Size(_mapWidth * _tileSize, _mapHeight * _tileSize));
        bgSprite->setAnchorPoint(cocos2d::Vec2::ZERO);
        bgSprite->setColor(cocos2d::Color3B(34, 139, 34)); // 草绿色
        _backgroundLayer->addChild(bgSprite);
    }
}

void SceneMap::updateTileOccupancy(const cocos2d::Vec2& gridPos, const cocos2d::Size& size, bool occupied, cocos2d::Node* object) {
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

// === 调试功能 ===
void SceneMap::drawGrid(bool show) {
    if (!_gridDrawNode) return;
    
    _gridDrawNode->clear();
    
    if (!show) return;
    
    // 绘制网格线
    for (int x = 0; x <= _mapWidth; ++x) {
        cocos2d::Vec2 start(x * _tileSize, 0);
        cocos2d::Vec2 end(x * _tileSize, _mapHeight * _tileSize);
        _gridDrawNode->drawLine(start, end, cocos2d::Color4F::GRAY);
    }
    
    for (int y = 0; y <= _mapHeight; ++y) {
        cocos2d::Vec2 start(0, y * _tileSize);
        cocos2d::Vec2 end(_mapWidth * _tileSize, y * _tileSize);
        _gridDrawNode->drawLine(start, end, cocos2d::Color4F::GRAY);
    }
}

// === 触摸事件处理 ===
bool SceneMap::onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event) {
    cocos2d::Vec2 touchPos = touch->getLocation();
    cocos2d::Vec2 gridPos = worldToGrid(touchPos);
    
    CCLOG("Touch at grid position: (%.0f, %.0f)", gridPos.x, gridPos.y);
    
    return true;
}

void SceneMap::onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event) {
    // 处理拖拽逻辑
}

void SceneMap::onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event) {
    // 处理触摸结束逻辑
}

// === 更新方法 ===
void SceneMap::update(float delta) {
    cocos2d::Node::update(delta);
    
    // 更新所有单位
    for (auto unit : _units) {
        // unit->update(delta);
    }
    
    // 其他地图逻辑更新
}