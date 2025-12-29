#include "SceneMap.h"
//#include "buildings/TownHall.h"
#include "BattleScene.h"
#include "Control/GameManager.h"
#include "buildings/BuildingPreview.h"
#include "Control/BuildingsElixirCapacityConfig.h"
#include "Control/BuildingsGoldCapacityConfig.h"
#include "buildings/BuildingsData.h"

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

    // 取消更新调度
    this->unscheduleUpdate();

    // 清理BuildingPreview
    if (_buildingPreview) {
        if (_buildingPreview->getParent())
            _buildingPreview->removeFromParent();
        //_buildingPreview->cancel(); // Cancel might use invalid map
        _buildingPreview = nullptr;
    }

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

    // 重置单例指针
    if (sInstance == this) {
        sInstance = nullptr;
    }

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

        _buildingPreview = BuildingPreview::create(_homeVillageMap);
        if (_buildingPreview)
        {
            this->addChild(_buildingPreview, 150);  // 设置高优先级
            CCLOG("BuildingPreview created successfully");
        }
        else
            CCLOG("ERROR: Failed to create BuildingPreview");
    }
    else
    {
        CCLOG("ERROR: Failed to create map layer - this should not happen with placeholder support");
        // 创建备用背景
        auto fallbackBg = LayerColor::create(Color4B(20, 120, 20, 255), visibleSize.width, visibleSize.height);
        this->addChild(fallbackBg, 0);

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

        // =========================update qy===================================
        
        // 如果没有已有建筑，创建默认的市政厅
        if (!hasBuildings) {
            CCLOG("No existing buildings found, creating default TownHall/GoldMine/GoldStorage/ElixirCollector/ElixirStorage");
            int centerGridX = _homeVillageMap->getGridCols() / 2;
            int centerGridY = _homeVillageMap->getGridRows() / 2;

            // 创建大本营
            auto townHall = TownHall::create(1);
            if (townHall)
            {
                Size gridSize = townHall->getGridSize();
                centerGridX -= (int)gridSize.width / 2;
                centerGridY -= (int)gridSize.height / 2;
                if (_homeVillageMap->canPlaceBuilding(centerGridX, centerGridY, gridSize))
                {
                    townHall->setGridPosition(centerGridX, centerGridY);
                    Vec2 worldPos = _homeVillageMap->gridToWorld(centerGridX, centerGridY);
                    townHall->setPosition(worldPos);
                    _homeVillageMap->addBuilding(townHall);
                    _buildings.pushBack(townHall);
                    _homeVillageMap->markGridsOccupied(centerGridX, centerGridY, gridSize, true);
                    CCLOG("TownHall created at grid (%d, %d)", centerGridX, centerGridY);
                }
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

    // =========================update qy===================================

    //// 播放背景音乐
    //auto audioMgr = AudioManager::getInstance();
    //// audioMgr->playBackgroundMusic("sounds/background.mp3", true);

    CCLOG("SceneMap initialized successfully");
    return true;
}


void SceneMap::onEnter()
{
    Scene::onEnter();
    CCLOG("SceneMap::onEnter called");

    // 重新初始化事件监听器
    reinitializeEventListeners();

    // 重置建筑放置状态
    // 确保进入场景时清理掉无效的放置状态，但保留pending的放置
    resetBuildingPlacementState();

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

    if (type == BuildingType::TOWN_HALL)
    {
        switch (level)
        {
        case 1: singleCapacity = TOWN_HALL_LEVEL_1_GOLD_CAPACITY; break;
        case 2: singleCapacity = TOWN_HALL_LEVEL_2_GOLD_CAPACITY; break;
        case 3: singleCapacity = TOWN_HALL_LEVEL_3_GOLD_CAPACITY; break;
        default: singleCapacity = TOWN_HALL_LEVEL_1_GOLD_CAPACITY; break;
        }
    }
    else if (type == BuildingType::GOLD_MINE)
    {
        switch (level)
        {
        case 1: singleCapacity = GOLD_MINE_LEVEL_1_GOLD_CAPACITY; break;
        case 2: singleCapacity = GOLD_MINE_LEVEL_2_GOLD_CAPACITY; break;
        case 3: singleCapacity = GOLD_MINE_LEVEL_3_GOLD_CAPACITY; break;
        default: singleCapacity = GOLD_MINE_LEVEL_1_GOLD_CAPACITY; break;
        }
    }
    else if (type == BuildingType::GOLD_STORAGE)
    {
        switch (level)
        {
        case 1: singleCapacity = GOLD_STORAGE_LEVEL_1_GOLD_CAPACITY; break;
        case 2: singleCapacity = GOLD_STORAGE_LEVEL_2_GOLD_CAPACITY; break;
        case 3: singleCapacity = GOLD_STORAGE_LEVEL_3_GOLD_CAPACITY; break;
        default: singleCapacity = GOLD_STORAGE_LEVEL_1_GOLD_CAPACITY; break;
        }
    }
    return singleCapacity;
}

// ========== 新增：获取单个建筑圣水容量 ==========
int SceneMap::getSingleBuildingElixirCapacity(Building* building)
{
    if (building == nullptr) return 0;
    int level = building->getLevel();
    BuildingType type = building->getBuildingType();
    int singleCapacity = 0;

    if (type == BuildingType::TOWN_HALL)
    {
        switch (level)
        {
        case 1: singleCapacity = TOWN_HALL_LEVEL_1_ELIXIR_CAPACITY; break;
        case 2: singleCapacity = TOWN_HALL_LEVEL_2_ELIXIR_CAPACITY; break;
        case 3: singleCapacity = TOWN_HALL_LEVEL_3_ELIXIR_CAPACITY; break;
        default: singleCapacity = TOWN_HALL_LEVEL_1_ELIXIR_CAPACITY; break;
        }
    }
    else if (type == BuildingType::ELIXIR_COLLECTOR)
    {
        switch (level)
        {
        case 1: singleCapacity = ELIXIR_COLLECTOR_LEVEL_1_ELIXIR_CAPACITY; break;
        case 2: singleCapacity = ELIXIR_COLLECTOR_LEVEL_2_ELIXIR_CAPACITY; break;
        case 3: singleCapacity = ELIXIR_COLLECTOR_LEVEL_3_ELIXIR_CAPACITY; break;
        default: singleCapacity = ELIXIR_COLLECTOR_LEVEL_1_ELIXIR_CAPACITY; break;
        }
    }
    else if (type == BuildingType::ELIXIR_STORAGE)
    {
        switch (level)
        {
        case 1: singleCapacity = ELIXIR_STORAGE_LEVEL_1_ELIXIR_CAPACITY; break;
        case 2: singleCapacity = ELIXIR_STORAGE_LEVEL_2_ELIXIR_CAPACITY; break;
        case 3: singleCapacity = ELIXIR_STORAGE_LEVEL_3_ELIXIR_CAPACITY; break;
        default: singleCapacity = ELIXIR_STORAGE_LEVEL_1_ELIXIR_CAPACITY; break;
        }
    }
    return singleCapacity;
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

    auto menu = Menu::createWithArray(menuItems);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 100);
}

// =========================update qy===================================
void SceneMap::setupResourceUI()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Node* resRootNode = Node::create();
    resRootNode->setPosition(Vec2(origin.x + visibleSize.width - 60, origin.y + visibleSize.height - 100));
    resRootNode->setAnchorPoint(Vec2(1, 1));
    this->addChild(resRootNode, 9999);

    _goldIcon = Sprite::create("Icon/gold_icon.png");
    if (!_goldIcon) { _goldIcon = Sprite::create(); _goldIcon->setColor(Color3B::YELLOW); _goldIcon->setContentSize(Size(40, 40)); }
    _goldIcon->setAnchorPoint(Vec2(1, 0.5)); _goldIcon->setPosition(Vec2(-10, 25)); resRootNode->addChild(_goldIcon);
    _goldLabel = Label::createWithSystemFont("Gold: 0", "Arial", 30);
    _goldLabel->setTextColor(Color4B::YELLOW); _goldLabel->setAnchorPoint(Vec2(1, 0.5)); _goldLabel->setPosition(Vec2(-_goldIcon->getContentSize().width - 20, 25)); resRootNode->addChild(_goldLabel);

    _elixirIcon = Sprite::create("Icon/elixir_icon.png");
    if (!_elixirIcon) { _elixirIcon = Sprite::create(); _elixirIcon->setColor(Color3B::BLUE); _elixirIcon->setContentSize(Size(40, 40)); }
    _elixirIcon->setAnchorPoint(Vec2(1, 0.5)); _elixirIcon->setPosition(Vec2(-10, -25)); resRootNode->addChild(_elixirIcon);
    _elixirLabel = Label::createWithSystemFont("Elixir: 0", "Arial", 30);
    _elixirLabel->setTextColor(Color4B::BLUE); _elixirLabel->setAnchorPoint(Vec2(1, 0.5)); _elixirLabel->setPosition(Vec2(-_elixirIcon->getContentSize().width - 20, -25)); resRootNode->addChild(_elixirLabel);

    refreshResourceUI();
    CCLOG("SceneMap: 右上角金币圣水UI创建完成！");
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

// 产金/产圣水刷新方法
void SceneMap::refreshResourceUI()
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
}
// =========================update qy===================================


void SceneMap::onBattleButtonClicked(Ref* sender)
{
    // 创建简单的战斗模式选择弹窗
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 背景遮罩
    auto shade = LayerColor::create(Color4B(0, 0, 0, 200));
    this->addChild(shade, 200);

    // 菜单选项
    auto title = Label::createWithTTF("Select Battle Mode", "fonts/Marker Felt.ttf", 48);
    title->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 150);
    shade->addChild(title);

    auto multiplayerBtn = MenuItemLabel::create(
        Label::createWithTTF("Multiplayer (Test)", "fonts/Marker Felt.ttf", 32),
        [=](Ref* sender) {
            shade->removeFromParent();
            auto scene = BattleScene::createScene(0);
            Director::getInstance()->pushScene(scene);
        });

    auto level1Btn = MenuItemLabel::create(
        Label::createWithTTF("Goblin Forest (Level 1)", "fonts/Marker Felt.ttf", 32),
        [=](Ref* sender) {
            shade->removeFromParent();
            auto scene = BattleScene::createScene(1);
            Director::getInstance()->pushScene(scene);
        });

    auto level2Btn = MenuItemLabel::create(
        Label::createWithTTF("Goblin Outpost (Level 2)", "fonts/Marker Felt.ttf", 32),
        [=](Ref* sender) {
            shade->removeFromParent();
            auto scene = BattleScene::createScene(2);
            Director::getInstance()->pushScene(scene);
        });

    auto cancelBtn = MenuItemLabel::create(
        Label::createWithTTF("Cancel", "fonts/Marker Felt.ttf", 32),
        [=](Ref* sender) {
            shade->removeFromParent();
        });

    auto menu = Menu::create(multiplayerBtn, level1Btn, level2Btn, cancelBtn, nullptr);
    menu->alignItemsVerticallyWithPadding(30);
    menu->setPosition(visibleSize.width / 2, visibleSize.height / 2 - 20);
    shade->addChild(menu);

    // 捕获触摸事件防止穿透
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, shade);
}

void SceneMap::onShopButtonClicked(Ref* sender)
{
    CCLOG("Shop button clicked, switching to shop scene");

    // 通过 GameManager 切换到商店场景
    auto gameManager = GameManager::getInstance();
    gameManager->gotoShopScene();
}


// 建筑菜单设置（暂时注释）
// void SceneMap::setupBuildingMenu()
// {
//     auto visibleSize = Director::getInstance()->getVisibleSize();

//     // 建筑类型列表
//     std::vector<std::pair<std::string, BuildingType>> buildingTypes = {
//         {"Town Hall", BuildingType::TOWN_HALL},
//         {"Gold Mine", BuildingType::GOLD_MINE},
//         {"Elixir Collector", BuildingType::ELIXIR_COLLECTOR},
//         {"Barracks", BuildingType::BARRACKS},
//         {"Archer Tower", BuildingType::ARCHER_TOWER},
//         {"Cannon", BuildingType::CANNON},
//         {"Gold Storage", BuildingType::GOLD_STORAGE},
//         {"Elixir Storage", BuildingType::ELIXIR_COLLECTOR}
//     };

//     Vector<MenuItem*> menuItems;
//     float startY = visibleSize.height - 200;
//     float spacing = 40;

//     for (size_t i = 0; i < buildingTypes.size(); ++i)
//     {
//         BuildingType buildingType = buildingTypes[i].second;
//         auto label = Label::createWithTTF(buildingTypes[i].first, "fonts/Marker Felt.ttf", 20);
//         if (!label)
//         {
//             label = Label::createWithSystemFont(buildingTypes[i].first, "Arial", 20);
//         }
//         if (label)
//         {
//             auto item = MenuItemLabel::create(label,
//                 [this, buildingType](Ref* sender) {
//                     this->onBuildingSelected(sender, buildingType);
//                 });
//             item->setPosition(visibleSize.width - 150, startY - i * spacing);
//             menuItems.pushBack(item);
//         }
//     }

//     _buildingMenu = Menu::createWithArray(menuItems);
//     _buildingMenu->setPosition(Vec2::ZERO);
//     this->addChild(_buildingMenu, 100);
// }

// 建筑选择回调
// void SceneMap::onBuildingSelected(Ref* sender, BuildingType type)
// {
//     // 取消当前可能正在进行的预览或移动操作
//     if (_buildingPreview && (_buildingPreview->isPreviewing() || _buildingPreview->isMoving()))
//     {
//         _buildingPreview->cancel();
//     }

//     _isPlacingBuilding = true;
//     _selectedBuildingType = type;

//     // 开始预览
//     if (_buildingPreview)
//     {
//         _buildingPreview->startPreview(type);
//     }

//     // 更新UI提示
//     // ...
// }

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// 地图触摸逻辑：
// 
//bool SceneMap::onMapTouched(Touch* touch, Event* event)
//{
//    Vec2 touchPos = touch->getLocation();
//
//    CCLOG("SceneMap::onMapTouched called at (%.2f, %.2f)", touchPos.x, touchPos.y);
//
//    // 检查BuildingPreview是否有效
//    if (!_buildingPreview) {
//        CCLOG("BuildingPreview is null, cannot process touch");
//        return false;
//    }
//
//    // 如果正在预览建筑且未开始拖拽，开始拖拽检测
//    if (_buildingPreview->isPreviewing() && !_buildingPreview->isPreviewDragging())
//    {
//        CCLOG("Preview building detected, starting drag detection");
//        _buildingPreview->startPreviewDragDetection(touchPos);
//        return true;
//    }
//
//    // 如果正在移动建筑，更新位置
//    if (_buildingPreview->isMoving())
//    {
//        CCLOG("Building is moving, updating position");
//        _buildingPreview->updatePreviewPosition(touchPos);
//        return true;
//    }
//
//    // 如果是简单拖拽模式，更新位置
//    if (_buildingPreview->isSimpleDragging())
//    {
//        CCLOG("Building is simple dragging, updating position");
//        _buildingPreview->updatePreviewPosition(touchPos);
//        return true;
//    }
//
//    // 如果不是在移动建筑，检查是否点击了现有建筑
//    // 准备开始移动操作
//    if (!_isMovingBuilding)
//    {
//        CCLOG("Checking for building click - moving:%s",
//            _isMovingBuilding ? "true" : "false");
//
//        // 检查HomeVillageMap是否有效    
//        if (!_homeVillageMap) {
//            CCLOG("HomeVillageMap is null, cannot check buildings");
//            return false;
//        }
//
//        // 转换触摸坐标到地图坐标系
//        Vec2 mapPos = _homeVillageMap->getMapPosition();
//        float zoom = _homeVillageMap->getZoom();
//        Vec2 worldPos = (touchPos - mapPos) / zoom;
//
//        CCLOG("SceneMap::onMapTouched - checking for building at world pos (%.2f, %.2f)", worldPos.x, worldPos.y);
//        CCLOG("Current buildings count: %d", (int)_buildings.size());
//
//        // 通过BuildingPreview检查点击位置是否有建筑
//        Building* clickedBuilding = _buildingPreview->checkBuildingAtPosition(worldPos, _buildings);
//
//        if (clickedBuilding)
//        {
//            CCLOG("Building found via BuildingPreview, starting simple drag");
//            _buildingPreview->startSimpleDrag(clickedBuilding);
//            return true;
//        }
//
//        // 从HomeVillageMap的建筑容器中再次检查
//        auto buildingsContainer = _homeVillageMap->getBuildingsContainer();
//        if (buildingsContainer) {
//            Vector<Building*> containerBuildings;
//            auto existingBuildings = buildingsContainer->getChildren();
//            CCLOG("Buildings in container: %d", (int)existingBuildings.size());
//
//            for (auto building : existingBuildings) {
//                auto buildingObj = dynamic_cast<Building*>(building);
//                if (buildingObj) {
//                    containerBuildings.pushBack(buildingObj);
//                }
//            }
//
//            // 通过BuildingPreview检查容器中的建筑
//            clickedBuilding = _buildingPreview->checkBuildingAtPosition(worldPos, containerBuildings);
//
//            if (clickedBuilding)
//            {
//                CCLOG("Building found in container via BuildingPreview, starting simple drag");
//
//                // 将找到的建筑添加到SceneMap的列表中
//                bool found = false;
//                for (auto existingBuilding : _buildings) {
//                    if (existingBuilding == clickedBuilding) {
//                        found = true;
//                        break;
//                    }
//                }
//                if (!found) {
//                    _buildings.pushBack(clickedBuilding);
//                    CCLOG("Added building to SceneMap buildings list");
//                }
//
//                _buildingPreview->startSimpleDrag(clickedBuilding);
//                return true;
//            }
//        }
//
//        CCLOG("No building found at touch position");
//    }
//    else
//    {
//        CCLOG("Cannot check buildings - moving:%s preview:%s",
//            _isMovingBuilding ? "true" : "false",
//            _buildingPreview ? "valid" : "null");
//    }
//
//    // 返回false允许事件继续传递
//    // 这样UI元素可以接收到触摸事件
//    return false;
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

bool SceneMap::onMapTouched(Touch* touch, Event* event)
{
    Vec2 touchPos = touch->getLocation();
    CCLOG("SceneMap::onMapTouched at (%.2f, %.2f)", touchPos.x, touchPos.y);
    if (!_buildingPreview) return false;
    if (_buildingPreview->isPreviewing() && !_buildingPreview->isPreviewDragging()) { _buildingPreview->startPreviewDragDetection(touchPos); return true; }
    if (_buildingPreview->isMoving() || _buildingPreview->isSimpleDragging()) { _buildingPreview->updatePreviewPosition(touchPos); return true; }
    if (!_isMovingBuilding)
    {
        if (!_homeVillageMap) return false;
        Vec2 mapPos = _homeVillageMap->getMapPosition(); float zoom = _homeVillageMap->getZoom(); Vec2 worldPos = (touchPos - mapPos) / zoom;
        Building* clickedBuilding = _buildingPreview->checkBuildingAtPosition(worldPos, _buildings);
        if (clickedBuilding) { _buildingPreview->startSimpleDrag(clickedBuilding); return true; }
        auto buildingsContainer = _homeVillageMap->getBuildingsContainer();
        if (buildingsContainer)
        {
            Vector<Building*> containerBuildings;
            auto existingBuildings = buildingsContainer->getChildren();
            for (auto building : existingBuildings) {
                auto buildingObj = dynamic_cast<Building*>(building);
                if (buildingObj) containerBuildings.pushBack(buildingObj);
            }
            clickedBuilding = _buildingPreview->checkBuildingAtPosition(worldPos, containerBuildings);
            if (clickedBuilding)
            {
                bool found = false;
                for (auto b : _buildings) if (b == clickedBuilding) { found = true; break; }
                if (!found) _buildings.pushBack(clickedBuilding);
                _buildingPreview->startSimpleDrag(clickedBuilding);
                return true;
            }
        }
    }
    return false;
}

void SceneMap::onMouseMoved(Event* event)
{
    if (!_buildingPreview) return;
    EventMouse* mouseEvent = static_cast<EventMouse*>(event);
    Vec2 mousePos = Vec2(mouseEvent->getCursorX(), mouseEvent->getCursorY());
    if (_buildingPreview->isPreviewing() && _buildingPreview->isPreviewDragging()) _buildingPreview->updatePreviewPosition(mousePos);
    else if (_buildingPreview->isMoving() || _buildingPreview->isSimpleDragging()) _buildingPreview->updatePreviewPosition(mousePos);
}


void SceneMap::checkPendingBuildingPlacement()
{
    auto gameManager = GameManager::getInstance();
    if (gameManager->hasPendingBuildingPlacement())
    {
        BuildingType buildingType = gameManager->getPendingBuildingType();
        CCLOG("Found pending building placement for type: %d", (int)buildingType);

        // 开始建筑放置流程
        // 这里调用startBuildingPlacement来初始化预览状态
        // 确保resetBuildingPlacementState不会清除pending状态
        // 在init中已经调用过reset，所以这里安全

        // startBuildingPlacement 会设置 _isPlacingBuilding = true 并调用 preview->startPreview
        startBuildingPlacement(buildingType);

        // 清除pending状态
        gameManager->clearPendingBuildingPlacement();
    }
}

void SceneMap::startBuildingPlacement(BuildingType buildingType)
{
    // 取消当前可能正在进行的预览或移动操作
    if (_buildingPreview && (_buildingPreview->isPreviewing() || _buildingPreview->isMoving()))
    {
        _buildingPreview->cancel();
    }

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

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseDown = [this](Event* event) {
        EventMouse* e = static_cast<EventMouse*>(event);
        if (static_cast<int>(e->getMouseButton()) == MOUSE_RIGHT_BUTTON)
        {
            Vec2 mouseWorldPos = Vec2(e->getCursorX(), e->getCursorY());
            Vec2 mapPos = _homeVillageMap->getMapPosition(); float zoom = _homeVillageMap->getZoom(); Vec2 targetPos = (mouseWorldPos - mapPos) / zoom;
            Building* targetBuilding = _buildingPreview->checkBuildingAtPosition(targetPos, _buildings);
            if (targetBuilding) targetBuilding->showMenuImmediately();
        }
        };
    mouseListener->onMouseUp = [this](Event* event) {
        EventMouse* e = static_cast<EventMouse*>(event);
        if (static_cast<int>(e->getMouseButton()) == MOUSE_RIGHT_BUTTON)
        {
            Vec2 mouseWorldPos = Vec2(e->getCursorX(), e->getCursorY());
            for (auto& building : _buildings)
            {
                if (building && building->isMenuShow() && building->getUpgradeMenu())
                {
                    Vec2 localPos = building->getUpgradeMenu()->convertToNodeSpace(mouseWorldPos);
                    if (building->getUpgradeBtn() && building->getUpgradeBtn()->getBoundingBox().containsPoint(localPos)) {
                        building->upgradeBuilding();
                        this->calculateTotalGoldProduceSpeed();
                        this->calculateTotalElixirProduceSpeed();
                        CCLOG("建筑升级成功！立刻更新产速");
                    }
                    else if (building->getCancelBtn() && building->getCancelBtn()->getBoundingBox().containsPoint(localPos)) building->removeBuilding();
                    else if (building->getInfoBtn() && building->getInfoBtn()->getBoundingBox().containsPoint(localPos)) building->showBuildingInfo();
                }
            }
        }
        };
    _eventDispatcher->addEventListenerWithFixedPriority(mouseListener, -100);
#endif

    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    touchListener->onTouchBegan = CC_CALLBACK_2(SceneMap::onMapTouched, this);
    touchListener->onTouchMoved = [this](Touch* touch, Event* event) {
        Vec2 touchPos = touch->getLocation();
        if (_buildingPreview && _buildingPreview->isPreviewing())
        {
            if (!_buildingPreview->isPreviewDragging() && _buildingPreview->checkDragStart(touchPos)) _buildingPreview->updatePreviewPosition(touchPos);
            else if (_buildingPreview->isPreviewDragging()) _buildingPreview->updatePreviewPosition(touchPos);
        }
        else if (_buildingPreview && (_buildingPreview->isMoving() || _buildingPreview->isSimpleDragging())) _buildingPreview->updatePreviewPosition(touchPos);
        };
    touchListener->onTouchEnded = [this](Touch* touch, Event* event) {
        if (_buildingPreview && _buildingPreview->isPreviewDragging()) _buildingPreview->endPreviewDrag();
        else if (_buildingPreview && _buildingPreview->isSimpleDragging()) _buildingPreview->endSimpleDrag();
        };
    _eventDispatcher->addEventListenerWithFixedPriority(touchListener, -1);

    auto buildingPlacedListener = EventListenerCustom::create("building_placed", [this](EventCustom* event) {
        Building* building = static_cast<Building*>(event->getUserData());
        if (building && !_buildings.contains(building)) _buildings.pushBack(building);
        _isPlacingBuilding = false;
        this->calculateTotalGoldCapacity();
        this->calculateTotalElixirCapacity();
        });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(buildingPlacedListener, this);

    auto buildingMovedListener = EventListenerCustom::create("building_moved", [this](EventCustom* event) {
        _isMovingBuilding = false; _selectedBuilding = nullptr;
        });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(buildingMovedListener, this);
}

void SceneMap::resetBuildingPlacementState()
{
    CCLOG("SceneMap::resetBuildingPlacementState - resetting building states");

    // 检查是否有等待放置的建筑，如果有则保留状态
    auto gameManager = GameManager::getInstance();
    if (gameManager && gameManager->hasPendingBuildingPlacement()) {
        CCLOG("Found pending building placement, skipping state reset to preserve building preview");
        return;
    }

    _isPlacingBuilding = false;
    _isMovingBuilding = false;
    _selectedBuilding = nullptr;

    // 取消任何正在进行的预览
    if (_buildingPreview) {
        // 这里需要注意：如果有PendingPlacement，不应该调用cancel
        // 因为在SceneMap::init中会先调用reset然后调用checkPendingBuildingPlacement
        // 只有当ShopScene设置了PendingPlacement时才会为TRUE
        // 所以上面的if (gameManager->hasPendingBuildingPlacement()) 会保护这种情况

        if (_buildingPreview->isPreviewing() || _buildingPreview->isMoving()) {
            _buildingPreview->cancel();
            CCLOG("Cancelled existing building preview/move operations");
        }
    }

    CCLOG("Building placement state reset completed");
}

//yxy-update:
// 场景切换后重新初始化事件监听器
void SceneMap::onEnterTransitionDidFinish() {
    Scene::onEnterTransitionDidFinish();
    // Refresh display when returning from selection scene
    _homeVillageMap->updateTroopDisplay();
}
void SceneMap::onTroopSelectionButtonClicked(Ref* pSender) {

    CCLOG("TroopSelection button clicked, switching to troopselection scene");

    auto gameManager = GameManager::getInstance();
    gameManager->gotoTroopSelectionScene();
}


// =========================update qy===================================
// 金币总产速计算（原有）
void SceneMap::calculateTotalGoldProduceSpeed()
{
    int totalSpeed = 0;
    int goldMineCount = 0;

    if (_homeVillageMap && _homeVillageMap->getBuildingsContainer())
    {
        auto buildingsContainer = _homeVillageMap->getBuildingsContainer();
        auto allChildren = buildingsContainer->getChildren();
        for (auto node : allChildren)
        {
            Building* building = dynamic_cast<Building*>(node);
            if (building && building->getBuildingType() == BuildingType::GOLD_MINE)
            {
                goldMineCount++;
                int lv = building->getLevel();
                switch (lv) {
                case 1: totalSpeed += 200; break;
                case 2: totalSpeed += 400; break;
                case 3: totalSpeed += 600; break;
                default: totalSpeed += 200; break;
                }
            }
        }
    }

    GameManager::getInstance()->setGoldProduceSpeedPerHour(totalSpeed);
    CCLOG("[产速计算] 检测到%d座金矿 → 总产速：%d金币/小时", goldMineCount, totalSpeed);
}

// 金币总容量计算（原有）
void SceneMap::calculateTotalGoldCapacity()
{
    int totalGoldCapacity = 0;
    if (_homeVillageMap == nullptr || _homeVillageMap->getBuildingsContainer() == nullptr)
    {
        CCLOG("[金币上限] 地图容器为空，总容量设为0");
        GameManager::getInstance()->setGoldStorageCapacity(totalGoldCapacity);
        return;
    }

    auto buildingsContainer = _homeVillageMap->getBuildingsContainer();
    auto allChildren = buildingsContainer->getChildren();
    for (auto node : allChildren)
    {
        Building* building = dynamic_cast<Building*>(node);
        if (building == nullptr) continue;

        BuildingType type = building->getBuildingType();
        if (type == BuildingType::TOWN_HALL ||
            type == BuildingType::GOLD_MINE ||
            type == BuildingType::GOLD_STORAGE)
        {
            int singleCap = getSingleBuildingGoldCapacity(building);
            totalGoldCapacity += singleCap;
            CCLOG("[金币上限] 建筑类型：%d | 等级：%d | 单建筑容量：%d", (int)type, building->getLevel(), singleCap);
        }
    }

    GameManager::getInstance()->setGoldStorageCapacity(totalGoldCapacity);
    CCLOG("[金币上限] 全局总容量计算完成 → 总和 = %d", totalGoldCapacity);
    this->refreshResourceImmediately();
}

// ========== 新增：圣水总产速计算 ==========
void SceneMap::calculateTotalElixirProduceSpeed()
{
    int totalSpeed = 0;
    int elixirCollectorCount = 0;

    if (_homeVillageMap && _homeVillageMap->getBuildingsContainer())
    {
        auto buildingsContainer = _homeVillageMap->getBuildingsContainer();
        auto allChildren = buildingsContainer->getChildren();
        for (auto node : allChildren)
        {
            Building* building = dynamic_cast<Building*>(node);
            if (building && building->getBuildingType() == BuildingType::ELIXIR_COLLECTOR)
            {
                elixirCollectorCount++;
                int lv = building->getLevel();
                switch (lv) {
                case 1: totalSpeed += 200; break;
                case 2: totalSpeed += 400; break;
                case 3: totalSpeed += 600; break;
                default: totalSpeed += 200; break;
                }
            }
        }
    }
    GameManager::getInstance()->setElixirProduceSpeedPerHour(totalSpeed);
    CCLOG("[产圣水计算] 检测到%d座圣水收集器 → 总产速：%d圣水/小时", elixirCollectorCount, totalSpeed);
}

// ========== 新增：圣水总容量计算 ==========
void SceneMap::calculateTotalElixirCapacity()
{
    int totalElixirCapacity = 0;
    if (_homeVillageMap == nullptr || _homeVillageMap->getBuildingsContainer() == nullptr)
    {
        CCLOG("[圣水上限] 地图容器为空，总容量设为0");
        GameManager::getInstance()->setElixirStorageCapacity(totalElixirCapacity);
        return;
    }

    auto buildingsContainer = _homeVillageMap->getBuildingsContainer();
    auto allChildren = buildingsContainer->getChildren();
    for (auto node : allChildren)
    {
        Building* building = dynamic_cast<Building*>(node);
        if (building == nullptr) continue;

        BuildingType type = building->getBuildingType();
        if (type == BuildingType::TOWN_HALL ||
            type == BuildingType::ELIXIR_COLLECTOR ||
            type == BuildingType::ELIXIR_STORAGE)
        {
            int singleCap = getSingleBuildingElixirCapacity(building);
            totalElixirCapacity += singleCap;
            CCLOG("[圣水上限] 建筑类型：%d | 等级：%d | 单建筑容量：%d", (int)type, building->getLevel(), singleCap);
        }
    }
    GameManager::getInstance()->setElixirStorageCapacity(totalElixirCapacity);
    CCLOG("[圣水上限] 全局总容量计算完成 → 总和 = %d", totalElixirCapacity);
    this->refreshResourceImmediately();
}
// =========================update qy===================================
