/*************************************************************
* @file     : SceneMap.cpp
* @function ：所有地图的基类实现 - 部落冲突地图系统
* @author   : 叶芷含
* @note     ：实现地图网格系统、建筑放置、碰撞检测等核心功能
**************************************************************/

#include "SceneMap.h"
#include <algorithm>
#include <cmath>
#include "Constant/Constant.h"

USING_NS_CC;

// 初始化瓦片地图
bool SceneMap::init(const std::string& tmxFile) {
	if (!Node::init()) {
		return false;
	}

	// 创建并添加TMX地图
	tileMap = TMXTiledMap::create(tmxFile);
	if (!tileMap) {
		return false;
	}

	//// 设置地图属性
	//tileMap->setAnchorPoint(Vec2(0.0, 0.0));

	// 计算地图初始位置 - 让地图左下角对齐窗口左下角
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Size mapContentSize = tileMap->getContentSize();

	Vec2 initialPos;
	initialPos.x = 0;  // X轴左对齐

	if (mapContentSize.height > visibleSize.height) {
		initialPos.y = -(mapContentSize.height - visibleSize.height)/2;
	}
	else {
		initialPos.y = (mapContentSize.height - visibleSize.height) / 2;
	}

	tileMap->setPosition(initialPos);

	CCLOG("=== 地图位置调整 ===");
	CCLOG("地图尺寸: %.0fx%.0f, 窗口尺寸: %.0fx%.0f",
		mapContentSize.width, mapContentSize.height,
		visibleSize.width, visibleSize.height);
	CCLOG("地图初始位置: (%.0f, %.0f)", initialPos.x, initialPos.y);
	CCLOG("偏移量: Y轴向上偏移 %.0f 像素", -(initialPos.y));

	this->addChild(tileMap);

	// 获取碰撞层
	collisionLayer = getLayer("Collision");

	// 放大按钮
	auto zoomInBtn = MenuItemImage::create(
		ResPath::ZOOMINBUTTON, ResPath::ZOOMINBUTTONPRESSED,
		CC_CALLBACK_0(SceneMap::zoomIn, this)
	);
	zoomInBtn->setPosition(visibleSize.width - 50, visibleSize.height - 30);

	// 缩小按钮
	auto zoomOutBtn = MenuItemImage::create(
		ResPath::ZOOMOUTBUTTON, ResPath::ZOOMOUTBUTTONPRESSED,
		CC_CALLBACK_0(SceneMap::zoomOut, this)
	);
	zoomOutBtn->setPosition(visibleSize.width - 50, visibleSize.height - 80);

	// 创建菜单并添加到场景
	auto menu = Menu::create(zoomInBtn, zoomOutBtn, nullptr);
	menu->setPosition(Vec2::ZERO); // 菜单锚点设为原点，方便按钮定位
	this->addChild(menu, 10); // 层级设为10，确保按钮在最上层

	// 多点触摸监听器
	auto touchListener = EventListenerTouchAllAtOnce::create();
	// 绑定多点触摸的三个回调
	touchListener->onTouchesBegan = CC_CALLBACK_2(SceneMap::onTouchesBegan, this);
	touchListener->onTouchesMoved = CC_CALLBACK_2(SceneMap::onTouchesMoved, this);
	touchListener->onTouchesEnded = CC_CALLBACK_2(SceneMap::onTouchesEnded, this);

	// 添加监听器到事件分发器
	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

	// 添加鼠标滚轮监听器
	auto mouseListener = EventListenerMouse::create();
	// 绑定滚轮事件
	mouseListener->onMouseScroll = CC_CALLBACK_1(SceneMap::onMouseScroll, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);

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
		// 如果没有Collision层，使用Grass层作为碰撞检测
		TMXLayer* grassLayer = getLayer("Grass");
		if (!grassLayer) {
			return false; // 没有任何碰撞层，允许通行
		}
		
		// 将世界坐标转换为瓦片坐标
		Size tileSize = tileMap->getTileSize();
		Size mapSize = tileMap->getMapSize();

		// 世界坐标转瓦片坐标
		int tileX = static_cast<int>(pos.x / tileSize.width);
		int tileY = static_cast<int>(pos.y / tileSize.height);

		// 检查边界
		if (tileX < 0 || tileX >= mapSize.width || tileY < 0 || tileY >= mapSize.height) {
			return true; // 超出边界视为碰撞
		}

		// 关键修正：对于"left-up"渲染顺序的TMX地图，Y坐标无需翻转
		// 因为cocos2d-x的坐标系(左下角为原点)与left-up渲染顺序是一致的
		// 直接使用原始坐标进行查询
		unsigned int gid = grassLayer->getTileGIDAt(Vec2(tileX, tileY));
		// 如果有草地瓦片(gid != 0)，说明可以通行，返回false表示无碰撞
		// 如果没有草地瓦片(gid == 0)，说明不能通行，返回true表示有碰撞
		return gid == 0;
	}

	// 原有的Collision层逻辑
	Size tileSize = tileMap->getTileSize();
	Size mapSize = tileMap->getMapSize();

	int tileX = static_cast<int>(pos.x / tileSize.width);
	int tileY = static_cast<int>(pos.y / tileSize.height);

	if (tileX < 0 || tileX >= mapSize.width || tileY < 0 || tileY >= mapSize.height) {
		return true;
	}

	// 对于"left-up"渲染顺序，直接使用原始坐标
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

	int tileX = static_cast<int> (pos.x / tileSize.width);
	int tileY = static_cast<int> (pos.y / tileSize.height);

	// 边界检查
	if (tileX < 0 || tileX >= mapSize.width || tileY < 0 || tileY >= mapSize.height) {
		return TerrainType::Grass;
	}

	// 对于"left-up"渲染顺序，直接使用原始坐标
	unsigned int gid = grassLayer->getTileGIDAt(Vec2(tileX, tileY));
	return gid != 0 ? TerrainType::Grass : TerrainType::Grass; // 简化逻辑，都返回Grass
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

// 添加坐标转换方法
cocos2d::Vec2 SceneMap::TMXToCocos2d(const cocos2d::Vec2& tmxPos) const {
	if (!tileMap) {
		return tmxPos;
	}
	
	Size tileSize = tileMap->getTileSize();
	
	// 对于"left-up"渲染顺序的TMX地图，坐标系与cocos2d-x一致
	// TMX瓦片坐标转换为Cocos2d世界坐标 - 无需Y轴翻转
	Vec2 cocos2dPos;
	cocos2dPos.x = tmxPos.x * tileSize.width;
	cocos2dPos.y = tmxPos.y * tileSize.height;
	
	return cocos2dPos;
}

cocos2d::Vec2 SceneMap::Cocos2dToTMX(const cocos2d::Vec2& cocosPos) const {
	if (!tileMap) {
		return cocosPos;
	}
	
	Size tileSize = tileMap->getTileSize();
	
	// Cocos2d世界坐标转换为TMX瓦片坐标 - 无需Y轴翻转
	Vec2 tmxPos;
	tmxPos.x = static_cast<int>(cocosPos.x / tileSize.width);
	tmxPos.y = static_cast<int>(cocosPos.y / tileSize.height);
	
	return tmxPos;
}

// 设置滚动视图
void SceneMap::setupScrollView() {
	// 启用触摸事件
	auto listener = EventListenerTouchAllAtOnce::create();
	listener->onTouchesBegan = CC_CALLBACK_2(SceneMap::onTouchesBegan, this);
	listener->onTouchesMoved = CC_CALLBACK_2(SceneMap::onTouchesMoved, this);
	listener->onTouchesEnded = CC_CALLBACK_2(SceneMap::onTouchesEnded, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

// 多点触摸开始
void SceneMap::onTouchesBegan(const std::vector<Touch*>& touches, Event* event) {
	if (touches.size() >= 2) {
		isTwoTouch = true;
		auto touch1 = touches[0];
		auto touch2 = touches[1];
		initTwoTouchDistance = touch1->getLocation().distance(touch2->getLocation());
		initTwoTouchCenter = (touch1->getLocation() + touch2->getLocation()) / 2.0f;
	}
	else if (touches.size() == 1) {
		isTwoTouch = false;
		lastTouchPos = touches[0]->getLocation();
	}
}

// 多点触摸移动（缩放+移动逻辑）
void SceneMap::onTouchesMoved(const std::vector<Touch*>& touches, Event* event) {
	if (!tileMap) return;

	if (touches.size() >= 2 && isTwoTouch) {
		// 双指缩放逻辑
		auto touch1 = touches[0];
		auto touch2 = touches[1];
		float currentDistance = touch1->getLocation().distance(touch2->getLocation());
		float scaleRatio = currentDistance / initTwoTouchDistance;
		float newScale = clampf(currentScale * scaleRatio, minScale, maxScale);
		float scaleDelta = newScale / currentScale;

		Vec2 currentCenter = (touch1->getLocation() + touch2->getLocation()) / 2.0f;
		Vec2 offset = currentCenter - initTwoTouchCenter;
		Vec2 newMapPos = tileMap->getPosition() - offset * scaleDelta;

		tileMap->setScale(newScale);
		tileMap->setPosition(newMapPos);

		currentScale = newScale;
		initTwoTouchDistance = currentDistance;
		initTwoTouchCenter = currentCenter;
	}
	else if (touches.size() == 1 && !isTwoTouch) {
		// 单指移动逻辑
		Vec2 currentPos = touches[0]->getLocation();
		Vec2 delta = currentPos - lastTouchPos;
		tileMap->setPosition(tileMap->getPosition() + delta);
		lastTouchPos = currentPos;
	}
}

// 多点触摸结束
void SceneMap::onTouchesEnded(const std::vector<Touch*>& touches, Event* event) {
	if (touches.size() < 2) {
		isTwoTouch = false;
	}
}
//缩放功能的实现
void SceneMap::zoomIn() {
	currentScale += scaleStep;
	currentScale = clampf(currentScale, minScale, maxScale);
	tileMap->setScale(currentScale);
}

void SceneMap::zoomOut() {
	currentScale -= scaleStep;
	currentScale = clampf(currentScale, minScale, maxScale);
	tileMap->setScale(currentScale);
}

// 实现鼠标滚轮缩放逻辑
void SceneMap::onMouseScroll(EventMouse* event) {
	if (!tileMap) return;

	// 获取滚轮方向（向上为正，向下为负）
	float scrollY = event->getScrollY();
	if (scrollY == 0) return;

	// 计算新的缩放系数
	float newScale = currentScale + (scrollY > 0 ? scrollStep : -scrollStep);
	newScale = clampf(newScale, minScale, maxScale); // 限制范围
	if (newScale == currentScale) return; // 无变化则返回

	// 以鼠标当前位置为中心缩放
	Vec2 mouseWorldPos = event->getLocation(); // 鼠标屏幕坐标
	Vec2 mapLocalPos = tileMap->convertToNodeSpace(mouseWorldPos); // 鼠标在地图节点的本地坐标

	// 计算缩放后的地图位置偏移
	float scaleRatio = newScale / currentScale;
	Vec2 newMapPos = tileMap->getPosition() - (mapLocalPos * (scaleRatio - 1)) * tileMap->getScale();

	// 应用缩放和位置
	tileMap->setScale(newScale);
	tileMap->setPosition(newMapPos);

	// 更新当前缩放系数
	currentScale = newScale;
}