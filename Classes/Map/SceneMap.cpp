/*************************************************************
* @file     : SceneMap.cpp
* @function ：所有地图的基类实现 - 部落冲突地图系统
* @author   : 叶芷含
* @note     ：实现地图网格系统、建筑放置、碰撞检测等核心功能
**************************************************************/

// =========================update qy===================================
const int MOUSE_LEFT_BUTTON = 0;
const int MOUSE_RIGHT_BUTTON = 1;
// =========================update qy===================================

// 静态实例初始化
SceneMap* SceneMap::sInstance = nullptr;

// 构造函数
SceneMap::SceneMap() :
    _homeVillageMap(nullptr),
    _buildingPreview(nullptr),
    _goldLabel(nullptr),
    _elixirLabel(nullptr),
    _populationLabel(nullptr),
    _isPlacingBuilding(false),
    _isMovingBuilding(false),
    _selectedBuildingType(BuildingType::TOWN_HALL),
    _selectedBuilding(nullptr)
    , _goldIcon(nullptr)
    , _elixirIcon(nullptr)
{
}

// 析构函数
SceneMap::~SceneMap() {
    CCLOG("SceneMap destructor called");

    // 移除事件监听器
    if (_eventDispatcher) {
        // 移除所有针对当前节点的事件监听器
        _eventDispatcher->removeEventListenersForTarget(this);

        // 移除自定义事件监听器
        _eventDispatcher->removeCustomEventListeners("building_placed");
        _eventDispatcher->removeCustomEventListeners("building_moved");
    }

	//// 设置地图属性
	//tileMap->setAnchorPoint(Vec2(0.0, 0.0));

	// 计算地图初始位置 
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Size mapContentSize = tileMap->getContentSize();

    // 处理SceneMap和HomeVillageMap的父子关系 - 这里只移除但不销毁HomeVillageMap单例
    // 因为HomeVillageMap自身的生命周期由其单例模式管理
    if (_homeVillageMap) {
        CCLOG("Removing HomeVillageMap from scene but keeping singleton instance alive");
        // 使用removeChild false避免清理HomeVillageMap的资源
        if (_homeVillageMap->getParent() == this)
            this->removeChild(_homeVillageMap, false);
        _homeVillageMap = nullptr; // 清空指针
    }

    // 清空建筑列表 - 实际的建筑对象由HomeVillageMap管理
    _buildings.clear();

	Vec2 initialPos;
	initialPos.x = 0;  // X轴左对齐

    CCLOG("SceneMap destructor completed");
}

// 获取单例实例
SceneMap* SceneMap::getInstance() {
    if (!sInstance) {
        sInstance = new (std::nothrow) SceneMap();
        if (sInstance && sInstance->init()) {
            sInstance->autorelease();
            sInstance->retain(); // Keep it alive
        }
        else {
            CC_SAFE_DELETE(sInstance);
        }
    }
    return sInstance;
}

//Scene* SceneMap::createScene()
//{
//    // 不使用 CREATE_FUNC 宏来创建
//    SceneMap* scene = new (std::nothrow) SceneMap();
//    if (scene && scene->init())
//    {
//        scene->autorelease();
//        return scene;
//    }
//    CC_SAFE_DELETE(scene);
//    return nullptr;
//}

bool SceneMap::init()
{
    if (!Scene::init()) {
        CCLOG("Failed to init SceneMap! ");
        return false;
    }

    // 获取可见区域大小
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto background = LayerColor::create(Color4B(50, 50, 50, 255), visibleSize.width, visibleSize.height);
    this->addChild(background, -10);

    // 创建地图 - 使用单例模式
    _homeVillageMap = HomeVillageMap::getInstance("Map/1.png");
    if (_homeVillageMap)
    {
        // 检查并移除地图可能存在的父节点
        auto parent = _homeVillageMap->getParent();
        if (parent) {
            CCLOG("HomeVillageMap already has a parent, removing from previous parent");
            parent->removeChild(_homeVillageMap, false); // false表示不执行cleanup清理
        }

        this->addChild(_homeVillageMap, 0);
        CCLOG("Map layer created successfully with singleton instance");

        // 清理旧的建筑预览
        if (_buildingPreview)
        {
            CCLOG("SceneMap::init - removing old building preview");
            if (_buildingPreview->getParent()) {
                _buildingPreview->removeFromParent();
            }
            _buildingPreview = nullptr;
        }

	// 放大按钮
	auto zoomInBtn = MenuItemImage::create(
		ResPath::ZOOMINBUTTON, ResPath::ZOOMINBUTTONPRESSED,
		CC_CALLBACK_0(SceneMap::zoomIn, this)
	);
	zoomInBtn->setPosition(Vec2(visibleSize.width * 0.97f, visibleSize.height * 0.95f));

        auto label = Label::createWithSystemFont("Map Loading Failed", "fonts/arial.ttf", 48);
        if (label)
        {
            label->setPosition(visibleSize.width / 2, visibleSize.height / 2);
            label->setColor(Color3B::WHITE);
            this->addChild(label, 1);
        }
    }

    // 设置UI界面
    setupUI();
    // =========================update qy===================================
    setupResourceUI();
    // =========================update qy===================================

    // 重新初始化事件监听器
    reinitializeEventListeners();

    // 检查GameManager中是否有等待放置的建筑
    bool hasPendingPlacement = false;
    auto gameManager = GameManager::getInstance();
    if (gameManager->hasPendingBuildingPlacement())
    {
        hasPendingPlacement = true;
        // 这里不直接处理，留给checkPendingBuildingPlacement处理
        // 确保BuildingPreview能够正确显示预览
    }

    // 重置建筑放置状态 - 如果有pending placement会在reset中被保留
    resetBuildingPlacementState();

    // 检查并处理等待放置的建筑
    checkPendingBuildingPlacement();

    // 从地图中获取已有建筑并添加到_buildings列表
    if (_homeVillageMap)
    {
        // 检查是否已有建筑
        bool hasBuildings = false;
        auto buildingsContainer = _homeVillageMap->getBuildingsContainer();

        if (buildingsContainer && buildingsContainer->getChildrenCount() > 0) {
            hasBuildings = true;
            CCLOG("Found existing buildings (%d), skipping default TownHall creation",
                (int)buildingsContainer->getChildrenCount());

            // 将地图中的建筑添加到SceneMap的列表中
            auto existingBuildings = buildingsContainer->getChildren();
            for (auto building : existingBuildings) {
                auto buildingObj = dynamic_cast<Building*>(building);
                if (buildingObj) {
                    _buildings.pushBack(buildingObj);
                }
            }
        }

// 检测位置是否合法
bool SceneMap::isPositionValid(const Vec2& pos) const {
	return isWithinMapBounds(pos) && !checkTileCollision(pos);
}

            // 创建金矿
            int goldMineGridX = centerGridX - 4;
            int goldMineGridY = centerGridY;
            auto goldMine = GoldMine::create(1);
            if (goldMine)
            {
                Size goldMineGridSize = goldMine->getGridSize();
                if (_homeVillageMap->canPlaceBuilding(goldMineGridX, goldMineGridY, goldMineGridSize))
                {
                    goldMine->setGridPosition(goldMineGridX, goldMineGridY);
                    Vec2 goldMineWorldPos = _homeVillageMap->gridToWorld(goldMineGridX, goldMineGridY);
                    goldMine->setPosition(goldMineWorldPos);
                    _homeVillageMap->addBuilding(goldMine);
                    _buildings.pushBack(goldMine);
                    _homeVillageMap->markGridsOccupied(goldMineGridX, goldMineGridY, goldMineGridSize, true);
                    CCLOG("GoldMine created at grid (%d, %d)", goldMineGridX, goldMineGridY);
                }
            }

            // 创建储金罐
            int goldStorageGridX = centerGridX + 4;
            int goldStorageGridY = centerGridY;
            auto goldStorage = GoldStorage::create(1);
            if (goldStorage)
            {
                Size goldStorageGridSize = goldStorage->getGridSize();
                if (_homeVillageMap->canPlaceBuilding(goldStorageGridX, goldStorageGridY, goldStorageGridSize))
                {
                    goldStorage->setGridPosition(goldStorageGridX, goldStorageGridY);
                    Vec2 goldStorageWorldPos = _homeVillageMap->gridToWorld(goldStorageGridX, goldStorageGridY);
                    goldStorage->setPosition(goldStorageWorldPos);
                    _homeVillageMap->addBuilding(goldStorage);
                    _buildings.pushBack(goldStorage);
                    _homeVillageMap->markGridsOccupied(goldStorageGridX, goldStorageGridY, goldStorageGridSize, true);
                    CCLOG("GoldStorage created at grid (%d, %d)", goldStorageGridX, goldStorageGridY);
                }
            }

            // ========== 新增：创建圣水收集器 ==========
            int elixirCollectorGridX = centerGridX - 4;
            int elixirCollectorGridY = centerGridY + 4;
            auto elixirCollector = ElixirCollector::create(1);
            if (elixirCollector)
            {
                Size gridSize = elixirCollector->getGridSize();
                if (_homeVillageMap->canPlaceBuilding(elixirCollectorGridX, elixirCollectorGridY, gridSize))
                {
                    elixirCollector->setGridPosition(elixirCollectorGridX, elixirCollectorGridY);
                    Vec2 worldPos = _homeVillageMap->gridToWorld(elixirCollectorGridX, elixirCollectorGridY);
                    elixirCollector->setPosition(worldPos);
                    _homeVillageMap->addBuilding(elixirCollector);
                    _buildings.pushBack(elixirCollector);
                    _homeVillageMap->markGridsOccupied(elixirCollectorGridX, elixirCollectorGridY, gridSize, true);
                    CCLOG("ElixirCollector created at grid (%d, %d)", elixirCollectorGridX, elixirCollectorGridY);
                }
            }

            // ========== 新增：创建圣水瓶 ==========
            int elixirStorageGridX = centerGridX + 4;
            int elixirStorageGridY = centerGridY + 4;
            auto elixirStorage = ElixirStorage::create(1);
            if (elixirStorage)
            {
                Size gridSize = elixirStorage->getGridSize();
                if (_homeVillageMap->canPlaceBuilding(elixirStorageGridX, elixirStorageGridY, gridSize))
                {
                    elixirStorage->setGridPosition(elixirStorageGridX, elixirStorageGridY);
                    Vec2 worldPos = _homeVillageMap->gridToWorld(elixirStorageGridX, elixirStorageGridY);
                    elixirStorage->setPosition(worldPos);
                    _homeVillageMap->addBuilding(elixirStorage);
                    _buildings.pushBack(elixirStorage);
                    _homeVillageMap->markGridsOccupied(elixirStorageGridX, elixirStorageGridY, gridSize, true);
                    CCLOG("ElixirStorage created at grid (%d, %d)", elixirStorageGridX, elixirStorageGridY);
                }
            }
            // ========== 新增：创建1级加农炮 ==========
            auto cannon = Cannon::create(1);
            if (cannon)
            {
                Size gridSize = cannon->getGridSize();
                int cannonGridX = centerGridX - 6;
                int cannonGridY = centerGridY - 4;
                if (_homeVillageMap->canPlaceBuilding(cannonGridX, cannonGridY, gridSize))
                {
                    cannon->setGridPosition(cannonGridX, cannonGridY);
                    Vec2 worldPos = _homeVillageMap->gridToWorld(cannonGridX, cannonGridY);
                    cannon->setPosition(worldPos);
                    _homeVillageMap->addBuilding(cannon);
                    _buildings.pushBack(cannon);
                    _homeVillageMap->markGridsOccupied(cannonGridX, cannonGridY, gridSize, true);
                }
            }

            // ========== 新增：创建1级箭塔 ==========
            auto archerTower = ArcherTower::create(1);
            if (archerTower)
            {
                Size gridSize = archerTower->getGridSize();
                int archerGridX = centerGridX + 6;
                int archerGridY = centerGridY - 4;
                if (_homeVillageMap->canPlaceBuilding(archerGridX, archerGridY, gridSize))
                {
                    archerTower->setGridPosition(archerGridX, archerGridY);
                    Vec2 worldPos = _homeVillageMap->gridToWorld(archerGridX, archerGridY);
                    archerTower->setPosition(worldPos);
                    _homeVillageMap->addBuilding(archerTower);
                    _buildings.pushBack(archerTower);
                    _homeVillageMap->markGridsOccupied(archerGridX, archerGridY, gridSize, true);
                }
            }
            // ========== 新增：创建1级兵营 ==========
            auto barracks = Barracks::create(1);
            if (barracks)
            {
                Size gridSize = barracks->getGridSize();
                int barracksGridX = centerGridX;
                int barracksGridY = centerGridY - 4;
                if (_homeVillageMap->canPlaceBuilding(barracksGridX, barracksGridY, gridSize))
                {
                    barracks->setGridPosition(barracksGridX, barracksGridY);
                    Vec2 worldPos = _homeVillageMap->gridToWorld(barracksGridX, barracksGridY);
                    barracks->setPosition(worldPos);
                    _homeVillageMap->addBuilding(barracks);
                    _buildings.pushBack(barracks);
                    _homeVillageMap->markGridsOccupied(barracksGridX, barracksGridY, gridSize, true);
                    CCLOG("Barracks created at grid (%d, %d)", barracksGridX, barracksGridY);
                }
            }
        }
    }
    // =========================update qy===================================

    this->scheduleUpdate();

    // 金币产速定时器
    this->schedule([this](float dt) {
        GameManager::getInstance()->updateGoldProduce(dt);
        }, 0.2f, "gold_accumulate_only");

    // ========== 新增：圣水产速定时器 ==========
    this->schedule([this](float dt) {
        GameManager::getInstance()->updateElixirProduce(dt);
        }, 0.2f, "elixir_accumulate_only");

    // 初始化计算所有产速与容量
    this->calculateTotalGoldProduceSpeed();
    this->calculateTotalGoldCapacity();
    this->calculateTotalElixirProduceSpeed();
    this->calculateTotalElixirCapacity();

	// 原有的Collision层逻辑
	Size tileSize = tileMap->getTileSize();
	Size mapSize = tileMap->getMapSize();

    //// 播放背景音乐
    //auto audioMgr = AudioManager::getInstance();
    //// audioMgr->playBackgroundMusic("sounds/background.mp3", true);

    CCLOG("SceneMap initialized successfully");
    return true;
}

	// 对于"left-up"渲染顺序，直接使用原始坐标
	unsigned int gid = collisionLayer->getTileGIDAt(Vec2(tileX, tileY));
	return gid != 0;
}

    // 检查pending的建筑放置
    checkPendingBuildingPlacement();
}

// =========================update qy===================================
// 获取单个建筑金币容量（原有）
int SceneMap::getSingleBuildingGoldCapacity(Building* building)
{
    if (building == nullptr) return 0;

    int level = building->getLevel();
    BuildingType type = building->getBuildingType();
    int singleCapacity = 0;

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
// =========================update qy===================================

void SceneMap::setupUI()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建功能按钮
    Vector<MenuItem*> menuItems;

    // 战斗按钮
    auto battleBtn = MenuItemImage::create(
        "Icon/BattleIcon.png", "Icon/BattleIconPressed.png",   //正常&按下 图片
        CC_CALLBACK_1(SceneMap::onBattleButtonClicked, this));    // 点击回调

    battleBtn->setPosition(Vec2(visibleSize.width * 0.05f, visibleSize.height * 0.08f));
    menuItems.pushBack(battleBtn);

    // yzh-商店按钮
    auto shopBtn = MenuItemImage::create(
        "Icon/shop.png", "Icon/shopPressed.png",
        CC_CALLBACK_1(SceneMap::onShopButtonClicked, this));    // 点击回调

    shopBtn->setPosition(Vec2(visibleSize.width * 0.95f, visibleSize.height * 0.08f));
    menuItems.pushBack(shopBtn);

    // =========================update yxy===================================
    // 选兵按钮
    auto troopSelectionBtn = MenuItemImage::create(
        "Icon/TroopIcon.png",                 // 正常状态图标路径
        "Icon/TroopIconPressed.png",          // 按下状态图标路径
        CC_CALLBACK_1(SceneMap::onTroopSelectionButtonClicked, this)); // 点击回调函数

    // 设置缩放比例  像素有点大
    troopSelectionBtn->setScale(0.05f);

    // 将选兵按钮放在商店按钮左侧
    troopSelectionBtn->setPosition(Vec2(visibleSize.width * 0.85f, visibleSize.height * 0.08f));
    menuItems.pushBack(troopSelectionBtn);
    // =========================update yxy===================================

// 获取碰撞层
TMXLayer* SceneMap::getCollisionLayer() const {
	return collisionLayer;
}

// 获取地形类型
TerrainType SceneMap::getTerrainType(const Vec2& pos) const {
	if (!isWithinMapBounds(pos)) {
		return TerrainType::Grass; // 默认返回草地类型
	}


// 主动操作专属即时刷新（升级扣除后调用）
void SceneMap::refreshResourceImmediately()
{
    if (!_goldLabel || !_elixirLabel) return;
    GameManager* gm = GameManager::getInstance();
    if (!gm) return;

    int gold = gm->getResource(ResourceType::GOLD);
    int elixir = gm->getResource(ResourceType::ELIXIR);
    int goldCap = gm->getGoldStorageCapacity();
    int elixirCap = gm->getElixirStorageCapacity();

    _goldLabel->setString(StringUtils::format("Gold: %d/%d", gold, goldCap));
    _elixirLabel->setString(StringUtils::format("Elixir: %d/%d", elixir, elixirCap));
    CCLOG("[即时刷新] 资源更新完成");
}

	// 坐标转换（世界坐标到瓦片坐标）
	Size tileSize = tileMap->getTileSize();
	Size mapSize = tileMap->getMapSize();

    int gold = gm->getResource(ResourceType::GOLD);
    int elixir = gm->getResource(ResourceType::ELIXIR);
    int goldCap = gm->getGoldStorageCapacity();
    int elixirCap = gm->getElixirStorageCapacity();

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
//cocos2d::Vec2 SceneMap::TMXToCocos2d(const cocos2d::Vec2& tmxPos) const {
//	if (!tileMap) {
//		return tmxPos;
//	}
//	
//	Size tileSize = tileMap->getTileSize();
//	Size mapSize = tileMap->getMapSize();
//	
//	// y坐标转换
//	Vec2 cocos2dPos;
//	cocos2dPos.x = tmxPos.x * tileSize.width;
//	cocos2dPos.y = mapSize.height * tileSize.height - tmxPos.y * tileSize.height;
//	
//	return cocos2dPos;
//}

    auto level1Btn = MenuItemLabel::create(
        Label::createWithTTF("Goblin Forest (Level 1)", "fonts/Marker Felt.ttf", 32),
        [=](Ref* sender) {
            shade->removeFromParent();
            auto scene = BattleScene::createScene(1);
            Director::getInstance()->pushScene(scene);
        });

	// 1. 获取地图参数
	Size tileSize = tileMap->getTileSize();   // 16×16
	Vec2 mapOrigin = tileMap->getPosition();  // 地图节点的世界坐标（含位移）
	float scale = tileMap->getScale();        // 地图缩放系数

	// 2. Y轴栅格轴的Staggered转换核心公式
	// 水平步长：图块宽度 × 0.75（交错列的水平偏移）
	float stepX = tileSize.width * 0.75f;
	// 垂直步长：图块高度 + 偶数列的垂直偏移（图块高度/2）
	float yOffset = (static_cast<int>(tmxPos.x) % 2) * (tileSize.height / 2);

	// 计算世界坐标
	float x = tmxPos.x * stepX;
	float y = tmxPos.y * tileSize.height + yOffset;

	// 3. 应用地图的缩放和位移
	x *= scale;
	y *= scale;
	x += mapOrigin.x;
	y += mapOrigin.y;

	// （可选）转为瓦片中心坐标
	x += (tileSize.width / 2) * scale;
	y += (tileSize.height / 2) * scale;

    // 通过 GameManager 切换到商店场景
    auto gameManager = GameManager::getInstance();
    gameManager->gotoShopScene();
}

//cocos2d::Vec2 SceneMap::Cocos2dToTMX(const cocos2d::Vec2& cocosPos) const {
//	if (!tileMap) {
//		return cocosPos;
//	}
//	
//	Size tileSize = tileMap->getTileSize();
//	Size mapSize = tileMap->getMapSize();
//	
//	// Cocos2d世界坐标转换为TMX瓦片坐标
//	Vec2 tmxPos;
//	tmxPos.x = static_cast<int>(cocosPos.x / tileSize.width);
//	tmxPos.y = static_cast<int>(mapSize.height - cocosPos.y / tileSize.height);		// y坐标转换
//	
//	return tmxPos;
//}
//
//void SceneMap::onMouseMoved(Event* event)
//{
//    if (!_buildingPreview) {
//        return;
//    }
//
//    EventMouse* mouseEvent = static_cast<EventMouse*>(event);
//    Vec2 mousePos = Vec2(mouseEvent->getCursorX(), mouseEvent->getCursorY());
//
//    // 如果正在预览并拖拽，更新预览位置
//    if (_buildingPreview->isPreviewing() && _buildingPreview->isPreviewDragging())
//    {
//        _buildingPreview->updatePreviewPosition(mousePos);
//    }
//    // 如果正在移动建筑
//    else if (_buildingPreview->isMoving() || _buildingPreview->isSimpleDragging())
//    {
//        _buildingPreview->updatePreviewPosition(mousePos);
//    }
//}

//void SceneMap::update(float dt)
//{
//    // 更新资源UI显示
//    auto resourceMgr = ResourceManager::getInstance();
//    resourceMgr->updateUI();
//}

	// 1. 获取地图参数
	Size tileSize = tileMap->getTileSize();   // 16×16
	Vec2 mapOrigin = tileMap->getPosition();  // 地图节点的世界坐标
	float scale = tileMap->getScale();        // 地图缩放系数
	Size mapSize = tileMap->getMapSize();     // 60列 × 120行

	// 2. 抵消地图的缩放和位移
	float x = (cocosPos.x - mapOrigin.x) / scale;
	float y = (cocosPos.y - mapOrigin.y) / scale;

	// 3. 抵消瓦片中心的偏移（若TMXToCocos2d中加了中心偏移）
	x -= (tileSize.width / 2);
	y -= (tileSize.height / 2);

	// 4. Y轴栅格轴的Staggered逆转换
	float stepX = tileSize.width * 0.75f;
	// 先计算列坐标（X）
	float tileX = x / stepX;
	// 修正偶数列的垂直偏移
	float yOffset = (static_cast<int>(tileX) % 2) * (tileSize.height / 2);
	// 再计算行坐标（Y）
	float tileY = (y - yOffset) / tileSize.height;

	// 5. 取整并限制边界（避免越界）
	tileX = clampf(floor(tileX), 0, mapSize.width - 1);
	tileY = clampf(floor(tileY), 0, mapSize.height - 1);

        // startBuildingPlacement 会设置 _isPlacingBuilding = true 并调用 preview->startPreview
        startBuildingPlacement(buildingType);

        // 清除pending状态
        gameManager->clearPendingBuildingPlacement();
    }
}

// 设置滚动视图
void SceneMap::setupScrollView() {
	// 启用触摸事件
	auto listener = EventListenerTouchAllAtOnce::create();
	listener->onTouchesBegan = CC_CALLBACK_2(SceneMap::onTouchesBegan, this);
	listener->onTouchesMoved = CC_CALLBACK_2(SceneMap::onTouchesMoved, this);
	listener->onTouchesEnded = CC_CALLBACK_2(SceneMap::onTouchesEnded, this);

    _isPlacingBuilding = true;
    _selectedBuildingType = buildingType;

    // 开始预览
    if (_buildingPreview)
    {
        _buildingPreview->startPreview(buildingType);
        CCLOG("Started building placement preview for type: %d", (int)buildingType);
    }
    else
    {
        CCLOG("ERROR: BuildingPreview not available for building placement");
    }
}


///////////////////////////////////////////////////////////////////
///////////////////////核心融合冲突/////////////////////////////////
///////////////////////////////////////////////////////////////////
//void SceneMap::reinitializeEventListeners()
//{
//    CCLOG("SceneMap::reinitializeEventListeners - setting up event listeners");
//
//    // 检查场景是否正在运行，否则延迟初始化
//    if (!this->isRunning()) {
//        CCLOG("Scene is not running, scheduling event listener initialization");
//        this->scheduleOnce([this](float dt) {
//            this->reinitializeEventListeners();
//            }, 0.1f, "delayed_event_init");
//        return;
//    }
//
//    // 移除旧的监听器
//    if (_eventDispatcher) {
//        _eventDispatcher->removeEventListenersForTarget(this);
//    }
//
//    CCLOG("Registering touch event listener...");
//
//    // =========================update qy===================================
//    // comment : 鼠标右键
//#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
//    auto mouseListener = EventListenerMouse::create();
//    mouseListener->onMouseDown = [this](Event* event) {
//        EventMouse* e = static_cast<EventMouse*>(event);
//        if (static_cast<int>(e->getMouseButton()) == MOUSE_RIGHT_BUTTON)
//        {
//            Vec2 mouseWorldPos = Vec2(e->getCursorX(), e->getCursorY());
//            Vec2 mapPos = _homeVillageMap->getMapPosition(); float zoom = _homeVillageMap->getZoom(); Vec2 targetPos = (mouseWorldPos - mapPos) / zoom;
//            Building* targetBuilding = _buildingPreview->checkBuildingAtPosition(targetPos, _buildings);
//            if (targetBuilding) targetBuilding->showMenuImmediately();
//        }
//        };
//    mouseListener->onMouseUp = [this](Event* event) {
//        EventMouse* e = static_cast<EventMouse*>(event);
//        if (static_cast<int>(e->getMouseButton()) == MOUSE_RIGHT_BUTTON)
//        {
//            Vec2 mouseWorldPos = Vec2(e->getCursorX(), e->getCursorY());
//            for (auto& building : _buildings)
//            {
//                if (building && building->isMenuShow() && building->getUpgradeMenu())
//                {
//                    Vec2 localPos = building->getUpgradeMenu()->convertToNodeSpace(mouseWorldPos);
//                    if (building->getUpgradeBtn() && building->getUpgradeBtn()->getBoundingBox().containsPoint(localPos)) {
//                        building->upgradeBuilding();
//                        this->calculateTotalGoldProduceSpeed();
//                        this->calculateTotalElixirProduceSpeed();
//                        CCLOG("建筑升级成功！立刻更新产速");
//                    }
//                    else if (building->getCancelBtn() && building->getCancelBtn()->getBoundingBox().containsPoint(localPos)) building->removeBuilding();
//                    else if (building->getInfoBtn() && building->getInfoBtn()->getBoundingBox().containsPoint(localPos)) building->showBuildingInfo();
//                }
//            }
//        }
//        };
//    _eventDispatcher->addEventListenerWithFixedPriority(mouseListener, -100);
//#endif
//    // =========================update qy===================================
//
//    // 创建触摸监听器 - 设置优先级-1确保比HomeVillageMap先接收
//    auto listener upd EventListenerTouchOneByOne::create();
//    listener->setSwallowTouches(true); // 吞噬触摸事件(不传递给HomeVillageMap)
//    listener->onTouchBegan = CC_CALLBACK_2(SceneMap::onMapTouched, this);
//    listener->onTouchMoved = [this](Touch* touch, Event* event) {
//        Vec2 touchPos = touch->getLocation();
//
//        // 如果正在预览建筑，更新预览位置
//        if (_buildingPreview && _buildingPreview->isPreviewing())
//        {
//            // 检查是否应该开始拖拽
//            if (!_buildingPreview->isPreviewDragging())
//            {
//                if (_buildingPreview->checkDragStart(touchPos))
//                {
//                    // 开始拖拽
//                    _buildingPreview->updatePreviewPosition(touchPos);
//                }
//            }
//            else
//            {
//                // 持续更新位置
//                _buildingPreview->updatePreviewPosition(touchPos);
//            }
//        }
//        // 如果正在移动建筑
//        else if (_buildingPreview && (_buildingPreview->isMoving() || _buildingPreview->isSimpleDragging()))
//        {
//            _buildingPreview->updatePreviewPosition(touchPos);
//        }
//        };
//    listener->onTouchEnded = [this](Touch* touch, Event* event) {
//        // 结束预览拖拽
//        if (_buildingPreview && _buildingPreview->isPreviewDragging())
//        {
//            _buildingPreview->endPreviewDrag();
//        }
//        // 结束简单拖拽
//        else if (_buildingPreview && _buildingPreview->isSimpleDragging())
//        {
//            _buildingPreview->endSimpleDrag();
//        }
//        };
//    _eventDispatcher->addEventListenerWithFixedPriority(listener, -1);
//
//    // 自定义事件 - 建筑放置完成
//    auto buildingPlacedListener = EventListenerCustom::create("building_placed",
//        [this](EventCustom* event) {
//            // 检查SceneMap是否有效
//            if (!this || !this->getScene()) {
//                CCLOG("SceneMap object or scene is invalid, ignoring building_placed event");
//                return;
//            }
//
//            Building* building = static_cast<Building*>(event->getUserData());
//            CCLOG("Building placed event received for building: %p", building);
//
//            if (building) {
//                // 将建筑添加到列表中（避免重复）
//                bool found = false;
//                for (auto existingBuilding : this->_buildings) {
//                    if (existingBuilding == building) {
//                        found = true;
//                        break;
//                    }
//                }
//
//                if (!found) {
//                    this->_buildings.pushBack(building);
//                    CCLOG("Added building to SceneMap buildings list (total: %d)", (int)this->_buildings.size());
//                }
//                else {
//                    CCLOG("Building already in SceneMap buildings list");
//                }
//
//                // 重置放置状态
//                this->_isPlacingBuilding = false;
//                CCLOG("Building placed event processed successfully, _isPlacingBuilding set to false");
//            }
//            else {
//                CCLOG("Warning: Building placed event received with null building");
//                // 安全重置状态
//                this->_isPlacingBuilding = false;
//            }
//            // ================================update qy=====================================
//            this->calculateTotalGoldCapacity();
//            this->calculateTotalElixirCapacity();
//            // ================================update qy=====================================
//        });
//    _eventDispatcher->addEventListenerWithSceneGraphPriority(buildingPlacedListener, this);
//
//    auto buildingMovedListener = EventListenerCustom::create("building_moved",
//        [this](EventCustom* event) {
//            // 检查SceneMap是否有效
//            if (this && this->getScene() && !this->getScene()->isRunning()) {
//                CCLOG("SceneMap object or scene is invalid, ignoring building_moved event");
//                return;
//            }
//
//            // 重置移动状态
//            this->_isMovingBuilding = false;
//            this->_selectedBuilding = nullptr;
//            CCLOG("Building moved event received and processed");
//        });
//    _eventDispatcher->addEventListenerWithSceneGraphPriority(buildingMovedListener, this);
//
//    CCLOG("SceneMap event listeners reinitialized successfully");
//}
void SceneMap::reinitializeEventListeners()
{
    CCLOG("SceneMap::reinitializeEventListeners");
    if (!this->isRunning()) {
        this->runAction(Sequence::create(
            DelayTime::create(0.1f),
            CallFunc::create([this]() {
                this->reinitializeEventListeners();
                }),
            nullptr
        ));
        return;
    }
    if (_eventDispatcher) _eventDispatcher->removeEventListenersForTarget(this);

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

    auto buildingMovedListener = EventListenerCustom::create("building_moved", [this](EventCustom* event) {
        _isMovingBuilding = false; _selectedBuilding = nullptr;
        });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(buildingMovedListener, this);
}

void SceneMap::resetBuildingPlacementState()
{
    CCLOG("SceneMap::resetBuildingPlacementState - resetting building states");

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
void SceneMap::onTroopSelectionButtonClicked(Ref* pSender) {

    CCLOG("TroopSelection button clicked, switching to troopselection scene");

    auto gameManager = GameManager::getInstance();
    gameManager->gotoTroopSelectionScene();
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

void SceneMap::onShopButtonClicked(Ref* sender) {
	this->scheduleOnce([this](float dt) {
		this->enterShop();
	}, 0.0f, "enter_shop");	  //后面的过渡场景有延时，所以这里的延时设置为0.0f
}

// ========== 新增：圣水总产速计算 ==========
void SceneMap::calculateTotalElixirProduceSpeed()
{
    int totalSpeed = 0;
    int elixirCollectorCount = 0;

	// 创建商店场景
	auto shopScene = ShopScene::create();

	if (!shopScene) {
		CCLOG("Warning: Failed to enter shopScene!");
		// 显示错误信息给用户
		statusLabel->setString("Failed to enter shopScene! Please try again.");
		statusLabel->setColor(Color3B::RED);
		return;
	}
	CCLOG("enter shopScene successfully");

    auto buildingsContainer = _homeVillageMap->getBuildingsContainer();
    auto allChildren = buildingsContainer->getChildren();
    for (auto node : allChildren)
    {
        Building* building = dynamic_cast<Building*>(node);
        if (building == nullptr) continue;

	if (!scene) {
		CCLOG("shopScene: Failed to create scene!");
		return;
	}
	scene->addChild(shopScene);

	// 使用过渡效果切换场景
	auto transition = TransitionFade::create(1.0f, scene);
	Director::getInstance()->replaceScene(transition);
}
// =========================update qy===================================
