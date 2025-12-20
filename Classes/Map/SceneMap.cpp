/*************************************************************
* @file     : SceneMap.cpp
* @function ：所有地图的基类实现 - 部落冲突地图系统
* @author   : 叶芷含
* @note     ：实现地图网格系统、建筑放置、碰撞检测等核心功能
**************************************************************/

#include "SceneMap.h"-
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

	//// ���õ�ͼ����
	//tileMap->setAnchorPoint(Vec2(0.0, 0.0));

	// �����ͼ��ʼλ�� 
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Size mapContentSize = tileMap->getContentSize();

	Size mapSize = tileMap->getMapSize();
	Size tileSize = tileMap->getTileSize();

	CCLOG("mapSize = %.0f x %.0f", mapSize.width, mapSize.height);
	CCLOG("tileSize = %.0f x %.0f", tileSize.width, tileSize.height);

	Vec2 initialPos;
	initialPos.x = 0;  // X�������

	if (mapContentSize.height > visibleSize.height) {
		initialPos.y = -(mapContentSize.height - visibleSize.height)/2;
	}
	else {
		initialPos.y = (mapContentSize.height - visibleSize.height) / 2;
	}

	tileMap->setPosition(initialPos);

	//CCLOG("=== ��ͼλ�õ��� ===");
	//CCLOG("��ͼ�ߴ�: %.0fx%.0f, ���ڳߴ�: %.0fx%.0f",
	//	mapContentSize.width, mapContentSize.height,
	//	visibleSize.width, visibleSize.height);
	//CCLOG("��ͼ��ʼλ��: (%.0f, %.0f)", initialPos.x, initialPos.y);
	//CCLOG("ƫ����: Y������ƫ�� %.0f ����", -(initialPos.y));

	this->addChild(tileMap);

	//// ʵ�ֵ�ͼ��ʼ��ʱ���������Ӧ����
	//Size winSize = Director::getInstance()->getVisibleSize();	//�鿴���ڿɼ�����
	//float mapOriginalWidth = tileMap->getMapSize().width * tileMap->getTileSize().width;	//��ͼԭʼ�������ش�С
	////float mapOriginalHeight = tileMap->getMapSize().height * tileMap->getTileSize().height;	//��ͼԭʼ�߶����ش�С
	//float adaptScale = winSize.width / mapOriginalWidth;

	//tileMap->setScale(adaptScale);
	//currentScale = adaptScale;

	// ��ȡ��ײ��
	collisionLayer = getLayer("Collision");

	// �Ŵ�ť
	auto zoomInBtn = MenuItemImage::create(
		ResPath::ZOOMINBUTTON, ResPath::ZOOMINBUTTONPRESSED,
		CC_CALLBACK_0(SceneMap::zoomIn, this)
	);
	zoomInBtn->setPosition(Vec2(visibleSize.width * 0.97f, visibleSize.height * 0.95f));

	// ��С��ť
	auto zoomOutBtn = MenuItemImage::create(
		ResPath::ZOOMOUTBUTTON, ResPath::ZOOMOUTBUTTONPRESSED,
		CC_CALLBACK_0(SceneMap::zoomOut, this)
	);
	zoomOutBtn->setPosition(Vec2(visibleSize.width * 0.97f, visibleSize.height * 0.88f));

	//�����̵갴ť
	auto shopBtn = MenuItemImage::create(
		ResPath::SHOP, ResPath::SHOPPRESSED,
		CC_CALLBACK_1(SceneMap::onShopButtonClicked, this));    // ����ص�

	shopBtn->setPosition(Vec2(visibleSize.width * 0.95f, visibleSize.height * 0.08f));

	// �����˵������ӵ�����
	auto menu = Menu::create(zoomInBtn, zoomOutBtn, shopBtn ,nullptr);
	menu->setPosition(Vec2::ZERO); // �˵�ê����Ϊԭ�㣬���㰴ť��λ
	this->addChild(menu, 10); // �㼶��Ϊ10��ȷ����ť�����ϲ�

	// ��㴥��������
	auto touchListener = EventListenerTouchAllAtOnce::create();
	// �󶨶�㴥���������ص�
	touchListener->onTouchesBegan = CC_CALLBACK_2(SceneMap::onTouchesBegan, this);
	touchListener->onTouchesMoved = CC_CALLBACK_2(SceneMap::onTouchesMoved, this);
	touchListener->onTouchesEnded = CC_CALLBACK_2(SceneMap::onTouchesEnded, this);

	// ���Ӽ��������¼��ַ���
	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

	// ���������ּ�����
	auto mouseListener = EventListenerMouse::create();
	// �󶨹����¼�
	mouseListener->onMouseScroll = CC_CALLBACK_1(SceneMap::onMouseScroll, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);

	return true;
}

// ���λ���Ƿ�Ϸ�
bool SceneMap::isPositionValid(const Vec2& pos) const {
	return isWithinMapBounds(pos) && !checkTileCollision(pos);
}

// ��⽨���ܷ���ã�������ͼ��
bool SceneMap::canPlaceBuilding(const Vec2& pos, const Size& buildingSize) const {
	if (!tileMap) {
		return false;
	}

	// ��齨����ÿ����Ƭλ��
	Size tileSize = tileMap->getTileSize();
	int tilesX = static_cast<int>(std::ceil(buildingSize.width / tileSize.width));
	int tilesY = static_cast<int>(std::ceil(buildingSize.height / tileSize.height));

	// ��齨��ռ�õ�������Ƭ
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

// ��ⵥ����Ƭ��ײ
bool SceneMap::checkTileCollision(const Vec2& pos) const {
	if (!collisionLayer || !tileMap) {
		// ���û��Collision�㣬ʹ��Grass����Ϊ��ײ���
		TMXLayer* grassLayer = getLayer("Grass");
		if (!grassLayer) {
			return false; // û���κ���ײ�㣬����ͨ��
		}
		
		// ����������ת��Ϊ��Ƭ����
		Size tileSize = tileMap->getTileSize();
		Size mapSize = tileMap->getMapSize();

		// ��������ת��Ƭ����
		int tileX = static_cast<int>(pos.x / tileSize.width);
		int tileY = static_cast<int>(pos.y / tileSize.height);

		// ���߽�
		if (tileX < 0 || tileX >= mapSize.width || tileY < 0 || tileY >= mapSize.height) {
			return true; // �����߽���Ϊ��ײ
		}

		// �ؼ�����������"left-up"��Ⱦ˳���TMX��ͼ��Y�������跭ת
		// ��Ϊcocos2d-x������ϵ(���½�Ϊԭ��)��left-up��Ⱦ˳����һ�µ�
		// ֱ��ʹ��ԭʼ������в�ѯ
		unsigned int gid = grassLayer->getTileGIDAt(Vec2(tileX, tileY));
		// ����вݵ���Ƭ(gid != 0)��˵������ͨ�У�����false��ʾ����ײ
		// ���û�вݵ���Ƭ(gid == 0)��˵������ͨ�У�����true��ʾ����ײ
		return gid == 0;
	}

	// ԭ�е�Collision���߼�
	Size tileSize = tileMap->getTileSize();
	Size mapSize = tileMap->getMapSize();

	int tileX = static_cast<int>(pos.x / tileSize.width);
	int tileY = static_cast<int>(pos.y / tileSize.height);

	if (tileX < 0 || tileX >= mapSize.width || tileY < 0 || tileY >= mapSize.height) {
		return true;
	}

	// ����"left-up"��Ⱦ˳��ֱ��ʹ��ԭʼ����
	unsigned int gid = collisionLayer->getTileGIDAt(Vec2(tileX, tileY));
	return gid != 0;
}

// ���λ���Ƿ��ڵ�ͼ�߽���
bool SceneMap::isWithinMapBounds(const Vec2& pos) const {
	if (!tileMap) {
		return false;
	}

	Size mapSize = tileMap->getMapSize();
	Size tileSize = tileMap->getTileSize();

	// �����ͼ��ʵ�����ش�С
	float mapWidth = mapSize.width * tileSize.width;
	float mapHeight = mapSize.height * tileSize.height;

	// ���λ���Ƿ��ڵ�ͼ��Χ��
	return pos.x >= 0 && pos.x < mapWidth && pos.y >= 0 && pos.y < mapHeight;
}

// ��ȡ��ͼ��
TMXLayer* SceneMap::getLayer(const std::string& layerName) const {
	if (tileMap) {
		return tileMap->getLayer(layerName);
	}
	return nullptr;
}

// ��ȡ��ײ��
TMXLayer* SceneMap::getCollisionLayer() const {
	return collisionLayer;
}

// ��ȡ��������
TerrainType SceneMap::getTerrainType(const Vec2& pos) const {
	if (!isWithinMapBounds(pos)) {
		return TerrainType::Grass; // Ĭ�Ϸ��زݵ�����
	}

	TMXLayer* grassLayer = getLayer("Grass");
	if (!grassLayer || !tileMap) {
		return TerrainType::Grass;
	}

	// ����ת�����������굽��Ƭ���꣩
	Size tileSize = tileMap->getTileSize();
	Size mapSize = tileMap->getMapSize();

	int tileX = static_cast<int> (pos.x / tileSize.width);
	int tileY = static_cast<int> (pos.y / tileSize.height);

	// �߽���
	if (tileX < 0 || tileX >= mapSize.width || tileY < 0 || tileY >= mapSize.height) {
		return TerrainType::Grass;
	}

	// ����"left-up"��Ⱦ˳��ֱ��ʹ��ԭʼ����
	unsigned int gid = grassLayer->getTileGIDAt(Vec2(tileX, tileY));
	return gid != 0 ? TerrainType::Grass : TerrainType::Grass; // ���߼���������Grass
}

// ��ȡ��ͼ�ߴ�
Size SceneMap::getMapSize() const {
	if (tileMap) {
		return tileMap->getMapSize();
	}
	return Size::ZERO;
}

// ��ȡ��Ƭ�ߴ�
Size SceneMap::getTileSize() const {
	if (tileMap) {
		return tileMap->getTileSize();
	}
	return Size::ZERO;
}

// ��������ת������
//cocos2d::Vec2 SceneMap::TMXToCocos2d(const cocos2d::Vec2& tmxPos) const {
//	if (!tileMap) {
//		return tmxPos;
//	}
//	
//	Size tileSize = tileMap->getTileSize();
//	Size mapSize = tileMap->getMapSize();
//	
//	// y����ת��
//	Vec2 cocos2dPos;
//	cocos2dPos.x = tmxPos.x * tileSize.width;
//	cocos2dPos.y = mapSize.height * tileSize.height - tmxPos.y * tileSize.height;
//	
//	return cocos2dPos;
//}

cocos2d::Vec2 SceneMap::TMXToCocos2d(const cocos2d::Vec2& tmxPos) const {
	if (!tileMap) return tmxPos;

	// 1. ��ȡ��ͼ����
	Size tileSize = tileMap->getTileSize();   // 16��16
	Vec2 mapOrigin = tileMap->getPosition();  // ��ͼ�ڵ���������꣨��λ�ƣ�
	float scale = tileMap->getScale();        // ��ͼ����ϵ��

	// 2. Y��դ�����Staggeredת�����Ĺ�ʽ
	// ˮƽ������ͼ����� �� 0.75�������е�ˮƽƫ�ƣ�
	float stepX = tileSize.width * 0.75f;
	// ��ֱ������ͼ��߶� + ż���еĴ�ֱƫ�ƣ�ͼ��߶�/2��
	float yOffset = (static_cast<int>(tmxPos.x) % 2) * (tileSize.height / 2);

	// ������������
	float x = tmxPos.x * stepX;
	float y = tmxPos.y * tileSize.height + yOffset;

	// 3. Ӧ�õ�ͼ�����ź�λ��
	x *= scale;
	y *= scale;
	x += mapOrigin.x;
	y += mapOrigin.y;

	// ����ѡ��תΪ��Ƭ��������
	x += (tileSize.width / 2) * scale;
	y += (tileSize.height / 2) * scale;

	return Vec2(x, y);
}

//cocos2d::Vec2 SceneMap::Cocos2dToTMX(const cocos2d::Vec2& cocosPos) const {
//	if (!tileMap) {
//		return cocosPos;
//	}
//	
//	Size tileSize = tileMap->getTileSize();
//	Size mapSize = tileMap->getMapSize();
//	
//	// Cocos2d��������ת��ΪTMX��Ƭ����
//	Vec2 tmxPos;
//	tmxPos.x = static_cast<int>(cocosPos.x / tileSize.width);
//	tmxPos.y = static_cast<int>(mapSize.height - cocosPos.y / tileSize.height);		// y����ת��
//	
//	return tmxPos;
//}

cocos2d::Vec2 SceneMap::Cocos2dToTMX(const cocos2d::Vec2& cocosPos) const {
	if (!tileMap) return cocosPos;

	// 1. ��ȡ��ͼ����
	Size tileSize = tileMap->getTileSize();   // 16��16
	Vec2 mapOrigin = tileMap->getPosition();  // ��ͼ�ڵ����������
	float scale = tileMap->getScale();        // ��ͼ����ϵ��
	Size mapSize = tileMap->getMapSize();     // 60�� �� 120��

	// 2. ������ͼ�����ź�λ��
	float x = (cocosPos.x - mapOrigin.x) / scale;
	float y = (cocosPos.y - mapOrigin.y) / scale;

	// 3. ������Ƭ���ĵ�ƫ�ƣ���TMXToCocos2d�м�������ƫ�ƣ�
	x -= (tileSize.width / 2);
	y -= (tileSize.height / 2);

	// 4. Y��դ�����Staggered��ת��
	float stepX = tileSize.width * 0.75f;
	// �ȼ��������꣨X��
	float tileX = x / stepX;
	// ����ż���еĴ�ֱƫ��
	float yOffset = (static_cast<int>(tileX) % 2) * (tileSize.height / 2);
	// �ټ��������꣨Y��
	float tileY = (y - yOffset) / tileSize.height;

	// 5. ȡ�������Ʊ߽磨����Խ�磩
	tileX = clampf(floor(tileX), 0, mapSize.width - 1);
	tileY = clampf(floor(tileY), 0, mapSize.height - 1);

	return Vec2(tileX, tileY);
}

// ���ù�����ͼ
void SceneMap::setupScrollView() {
	// ���ô����¼�
	auto listener = EventListenerTouchAllAtOnce::create();
	listener->onTouchesBegan = CC_CALLBACK_2(SceneMap::onTouchesBegan, this);
	listener->onTouchesMoved = CC_CALLBACK_2(SceneMap::onTouchesMoved, this);
	listener->onTouchesEnded = CC_CALLBACK_2(SceneMap::onTouchesEnded, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

// ��㴥����ʼ
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

// ��㴥���ƶ�������+�ƶ��߼���
void SceneMap::onTouchesMoved(const std::vector<Touch*>& touches, Event* event) {
	if (!tileMap) return;

	if (touches.size() >= 2 && isTwoTouch) {
		// ˫ָ�����߼�
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
		// ��ָ�ƶ��߼�
		Vec2 currentPos = touches[0]->getLocation();
		Vec2 delta = currentPos - lastTouchPos;
		tileMap->setPosition(tileMap->getPosition() + delta);
		lastTouchPos = currentPos;
	}
}

// ��㴥������
void SceneMap::onTouchesEnded(const std::vector<Touch*>& touches, Event* event) {
	if (touches.size() < 2) {
		isTwoTouch = false;
	}
}
//���Ź��ܵ�ʵ��
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

// ʵ�������������߼�
void SceneMap::onMouseScroll(EventMouse* event) {
	if (!tileMap) return;

	// ��ȡ���ַ�������Ϊ��������Ϊ����
	float scrollY = event->getScrollY();
	if (scrollY == 0) return;

	// �����µ�����ϵ��
	float newScale = currentScale + (scrollY > 0 ? scrollStep : -scrollStep);
	newScale = clampf(newScale, minScale, maxScale); // ���Ʒ�Χ
	if (newScale == currentScale) return; // �ޱ仯�򷵻�

	// ����굱ǰλ��Ϊ��������
	Vec2 mouseWorldPos = event->getLocation(); // �����Ļ����
	Vec2 mapLocalPos = tileMap->convertToNodeSpace(mouseWorldPos); // ����ڵ�ͼ�ڵ�ı�������

	// �������ź�ĵ�ͼλ��ƫ��
	float scaleRatio = newScale / currentScale;
	Vec2 newMapPos = tileMap->getPosition() - (mapLocalPos * (scaleRatio - 1)) * tileMap->getScale();

	// Ӧ�����ź�λ��
	tileMap->setScale(newScale);
	tileMap->setPosition(newMapPos);

	// ���µ�ǰ����ϵ��
	currentScale = newScale;
}

void SceneMap::onShopButtonClicked(Ref* sender) {
	this->scheduleOnce([this](float dt) {
		this->enterShop();
	}, 0.0f, "enter_shop");	  //����Ĺ��ɳ�������ʱ�������������ʱ����Ϊ0.0f
}

void SceneMap::enterShop() {
	CCLOG("Click shop button, jump to shop scene!");

	// �����̵곡��
	auto shopScene = ShopScene::create();

	if (!shopScene) {
		CCLOG("Warning: Failed to enter shopScene!");
		// ��ʾ������Ϣ���û�
		statusLabel->setString("Failed to enter shopScene! Please try again.");
		statusLabel->setColor(Color3B::RED);
		return;
	}
	CCLOG("enter shopScene successfully");

	auto scene = Scene::create();

	if (!scene) {
		CCLOG("shopScene: Failed to create scene!");
		return;
	}
	scene->addChild(shopScene);

	// ʹ�ù���Ч���л�����
	auto transition = TransitionFade::create(1.0f, scene);
	Director::getInstance()->replaceScene(transition);
}
