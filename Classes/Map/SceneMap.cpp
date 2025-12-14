/*************************************************************
* @file     : SceneMap.cpp
* @function ：所有地图的基类实现 - 部落冲突地图系统
* @author   : 叶芷含
* @note     ：实现地图网格系统、建筑放置、碰撞检测等核心功能
**************************************************************/

#include "SceneMap.h"
#include <algorithm>
#include <cmath>

USING_NS_CC;

// 初始化瓦片地图
bool SceneMap::init(const std::string& tmxFile) {
	if (!Node::init()) {
		return false;
	}

	// 创建并添加TMX地图 - 注意：必须先创建，再设置属性
	tileMap = TMXTiledMap::create(tmxFile);
	if (!tileMap) {
		return false;
	}

	// 设置地图属性
	tileMap->setAnchorPoint(Vec2(0, 0));
	tileMap->setPosition(Vec2(0, 0));
	this->addChild(tileMap);

	// 获取碰撞层
	collisionLayer = getLayer("Collision");

	return true;
}

// 检测位置是否合法
bool SceneMap::isPositionValid(const Vec2& pos) const {
	return isWithinMapBounds(pos) && !checkTileCollision(pos);
}

// 检测建筑能否放置（正交地图）
bool SceneMap::canPlaceBuilding(const Vec2& pos, const Size& buildingSize) const {
	if (!tileMap) {
		return false;
	}

	// 检查建筑的每个瓦片位置
	Size tileSize = tileMap->getTileSize();
	int tilesX = static_cast<int>(std::ceil(buildingSize.width / tileSize.width));
	int tilesY = static_cast<int>(std::ceil(buildingSize.height / tileSize.height));

	// 检查建筑占用的所有瓦片
	for (int x = 0; x < tilesX; x++) {
		for (int y = 0; y < tilesY; y++) {
			Vec2 checkPos = Vec2(pos.x + x * tileSize.width, pos.y + y * tileSize.height);
			if (!isPositionValid(checkPos)) {
				return false;
			}
		}
	}

	return true;
}

// 检测单个瓦片碰撞
bool SceneMap::checkTileCollision(const Vec2& pos) const {
	if (!collisionLayer || !tileMap) {
		return false;
	}

	// 将世界坐标转换为瓦片坐标
	Size tileSize = tileMap->getTileSize();
	Size mapSize = tileMap->getMapSize();

	// 世界坐标转瓦片坐标（简化版本）
	int tileX = static_cast<int>(pos.x / tileSize.width);
	int tileY = static_cast<int>(pos.y / tileSize.height);

	// 检查边界
	if (tileX < 0 || tileX >= mapSize.width || tileY < 0 || tileY >= mapSize.height) {
		return true; // 超出边界视为碰撞
	}

	// 直接使用瓦片坐标检查
	unsigned int gid = collisionLayer->getTileGIDAt(Vec2(tileX, tileY));
	return gid != 0;
}

// 检测位置是否在地图边界内
bool SceneMap::isWithinMapBounds(const Vec2& pos) const {
	if (!tileMap) {
		return false;
	}

	Size mapSize = tileMap->getMapSize();
	Size tileSize = tileMap->getTileSize();

	// 计算地图的实际像素大小
	float mapWidth = mapSize.width * tileSize.width;
	float mapHeight = mapSize.height * tileSize.height;

	// 检查位置是否在地图范围内
	return pos.x >= 0 && pos.x < mapWidth && pos.y >= 0 && pos.y < mapHeight;
}

// 获取地图层
TMXLayer* SceneMap::getLayer(const std::string& layerName) const {
	if (tileMap) {
		return tileMap->getLayer(layerName);
	}
	return nullptr;
}

// 获取碰撞层
TMXLayer* SceneMap::getCollisionLayer() const {
	return collisionLayer;
}

// 获取地形类型
TerrainType SceneMap::getTerrainType(const Vec2& pos) const {
	if (!isWithinMapBounds(pos)) {
		return TerrainType::Grass; // 默认返回草地类型
	}

	TMXLayer* grassLayer = getLayer("Grass");
	if (!grassLayer || !tileMap) {
		return TerrainType::Grass;
	}

	// 坐标转换（世界坐标到瓦片坐标）
	Size tileSize = tileMap->getTileSize();
	Size mapSize = tileMap->getMapSize();

	int tileX = static_cast<int>(pos.x / tileSize.width);
	int tileY = static_cast<int>(mapSize.height - 1 - pos.y / tileSize.height);

	// 边界检查
	if (tileX < 0 || tileX >= mapSize.width || tileY < 0 || tileY >= mapSize.height) {
		return TerrainType::Grass;
	}

	unsigned int gid = grassLayer->getTileGIDAt(Vec2(tileX, tileY));
	return gid != 0 ? TerrainType::Grass : TerrainType::Grass;
}

// 获取地图尺寸
Size SceneMap::getMapSize() const {
	if (tileMap) {
		return tileMap->getMapSize();
	}
	return Size::ZERO;
}

// 获取瓦片尺寸
Size SceneMap::getTileSize() const {
	if (tileMap) {
		return tileMap->getTileSize();
	}
	return Size::ZERO;
}

// 设置滚动视图
void SceneMap::setupScrollView() {
	// 启用触摸事件
	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchBegan = CC_CALLBACK_2(SceneMap::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(SceneMap::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(SceneMap::onTouchEnded, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

// 触摸开始
bool SceneMap::onTouchBegan(Touch* touch, Event* event) {
	lastTouchPos = touch->getLocation();
	return true;
}

// 触摸移动
void SceneMap::onTouchMoved(Touch* touch, Event* event) {
	Vec2 currentPos = touch->getLocation();
	Vec2 delta = currentPos - lastTouchPos;

	if (tileMap) {
		Vec2 mapPos = tileMap->getPosition();
		tileMap->setPosition(mapPos + delta);
	}

	lastTouchPos = currentPos;
}

// 触摸结束
void SceneMap::onTouchEnded(Touch* touch, Event* event) {
	// 触摸结束后可以添加惯性滚动等效果
}
