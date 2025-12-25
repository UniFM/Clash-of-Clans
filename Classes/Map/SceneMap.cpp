#include "SceneMap.h"
#include "Building/TownHall.h"
#include "Building/ResourceBuilding.h"
#include "Building/Barracks.h"
#include "Building/DefenseTower.h"
#include "Building/StorageBuilding.h"
#include "BattleScene.h"
#include "Control/AudioManager.h"
#include "Control/GameManager.h"
#include "Building/BuildingPreview.h"

// ??????????????
SceneMap* SceneMap::sInstance = nullptr;

// ??????
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
{
}

// ????????
SceneMap::~SceneMap() {
    CCLOG("SceneMap destructor called");
    
    // ????????????
    if (_eventDispatcher) {
        // ?????????????§Þ?????
        _eventDispatcher->removeEventListenersForTarget(this);
        
        // ?????????????
        _eventDispatcher->removeCustomEventListeners("building_placed");
        _eventDispatcher->removeCustomEventListeners("building_moved");
    }
    
    // ?????§Ö?????
    this->unscheduleUpdate();
    
    // ????BuildingPreview
    if (_buildingPreview) {
        if (_buildingPreview->getParent())
            _buildingPreview->removeFromParent();
        //_buildingPreview->cancel(); // Cancel might use invalid map
        _buildingPreview = nullptr;
    }
    
    // ???SceneMap???????HomeVillageMap????????????????????????????
    // ???????????HomeVillageMap???????????????????????????
    if (_homeVillageMap) {
        CCLOG("Removing HomeVillageMap from scene but keeping singleton instance alive");
        // ?????removeChild false????????????????HomeVillageMap?????
        if (_homeVillageMap->getParent() == this)
            this->removeChild(_homeVillageMap, false);
        _homeVillageMap = nullptr; // ????????
    }
    
    // ?????????§Ò????????????????????????????HomeVillageMap??
    _buildings.clear();
    
    // ?????????????????????????
    if (sInstance == this) {
        sInstance = nullptr;
    }
    
    CCLOG("SceneMap destructor completed");
}

// ??????????
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
//    // ??? CREATE_FUNC ????????
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

    // ??????????????
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto background = LayerColor::create(Color4B(50, 50, 50, 255), visibleSize.width, visibleSize.height);
    this->addChild(background, -10);

    // ?????? - ?????????
    _homeVillageMap = HomeVillageMap::getInstance("Map/1.png");
    if (_homeVillageMap)
    {
        // ????????§Ú??????????????????
        auto parent = _homeVillageMap->getParent();
        if (parent) {
            CCLOG("HomeVillageMap already has a parent, removing from previous parent");
            parent->removeChild(_homeVillageMap, false); // false?????cleanup????????
        }
        
        this->addChild(_homeVillageMap, 0);
        CCLOG("Map layer created successfully with singleton instance");

        // ??????????????
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
            this->addChild(_buildingPreview, 150);  // ???????
            CCLOG("BuildingPreview created successfully");
        }
        else
            CCLOG("ERROR: Failed to create BuildingPreview");
    }
    else
    {
        CCLOG("ERROR: Failed to create map layer - this should not happen with placeholder support");
        // ???????????
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

    // ?????UI???
    setupUI();

    // ??????????????
    reinitializeEventListeners();

    // ????????????GameManager???????????????????
    bool hasPendingPlacement = false;
    auto gameManager = GameManager::getInstance();
    if (gameManager->hasPendingBuildingPlacement())
    {
        hasPendingPlacement = true;
        // ???????????????????? checkPendingBuildingPlacement
        // ??? BuildingPreview ????????????????
    }

    // ????????????????????????????????????
    // ?????pending placement??resetBuildingPlacementState ???????????????
    resetBuildingPlacementState();

    // ???????????????????????????
    checkPendingBuildingPlacement();

    // ???????§ß?????_buildings?§Ò????????›Ï?????
    if (_homeVillageMap)
    {
        // ??????????????????§Ò?
        bool hasBuildings = false;
        auto buildingsContainer = _homeVillageMap->getBuildingsContainer();
        
        if (buildingsContainer && buildingsContainer->getChildrenCount() > 0) {
            hasBuildings = true;
            CCLOG("Found existing buildings (%d), skipping default TownHall creation", 
                  (int)buildingsContainer->getChildrenCount());
            
            // ?????§ß????????SceneMap???§Ò???
            auto existingBuildings = buildingsContainer->getChildren();
            for (auto building : existingBuildings) {
                auto buildingObj = dynamic_cast<Building*>(building);
                if (buildingObj) {
                    _buildings.pushBack(buildingObj);
                }
            }
        }
        
        // ?????§ß??????????????????????????????
        if (!hasBuildings) {
            CCLOG("No existing buildings found, creating default TownHall");
            
            // ????????¦Ë??
            int centerGridX = _homeVillageMap->getGridCols() / 2;
            int centerGridY = _homeVillageMap->getGridRows() / 2;

            auto townHall = TownHall::create(1);
            if (townHall)
            {
                Size gridSize = townHall->getGridSize();
                // ??????????????
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
        }
    }

    this->scheduleUpdate();

    // ???????????
    auto audioMgr = AudioManager::getInstance();
    // audioMgr->playBackgroundMusic("sounds/background.mp3", true);

    CCLOG("SceneMap initialized successfully");
    CCLOG("Visible size: %.0f x %.0f", visibleSize.width, visibleSize.height);

    return true;
}

void SceneMap::onEnter()
{
    Scene::onEnter();
    CCLOG("SceneMap::onEnter called");

    // ?????????????????
    reinitializeEventListeners();
    
    // ??öã??????????????
    // ????????????????????????? pending ????????????????
    resetBuildingPlacementState();
    
    // ??? pending ???????
    checkPendingBuildingPlacement();
}

void SceneMap::setupUI()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // ??????
    // ???
    _goldLabel = Label::createWithTTF("Gold: 0", "fonts/Marker Felt.ttf", 24);
    if (!_goldLabel){
        CCLOG("goldLabel initialized failed! ");
        return;
    }

    if (_goldLabel)
    {
        _goldLabel->setPosition(100, visibleSize.height - 50);
        _goldLabel->setColor(Color3B::YELLOW);
        this->addChild(_goldLabel, 100);
    }

    // ??
    _elixirLabel = Label::createWithTTF("Elixir: 0", "fonts/Marker Felt.ttf", 24);
    if (!_elixirLabel)
    {
        CCLOG("elixirLabel initialized failed! ");
        return;
    }
    if (_elixirLabel)
    {
        _elixirLabel->setPosition(100, visibleSize.height - 100);
        _elixirLabel->setColor(Color3B::MAGENTA);
        this->addChild(_elixirLabel, 100);
    }

    //// ??? (?????????????????????????Barracks?§Õ???)
    //_populationLabel = Label::createWithTTF("Population: 0/50", "fonts/Marker Felt.ttf", 24);
    //if (!_populationLabel)
    //{
    //    CCLOG("populationLabel initialized failed! ");
    //    return;
    //}
    //if (_populationLabel)
    //{
    //    _populationLabel->setPosition(100, visibleSize.height - 150);
    //    _populationLabel->setColor(Color3B::GREEN);
    //    this->addChild(_populationLabel, 100);
    //}

    // ?????????????UI????
    auto resourceMgr = ResourceManager::getInstance();
    resourceMgr->setUI(_goldLabel, _elixirLabel, _populationLabel);
    resourceMgr->reset();

    // ?????????
    Vector<MenuItem*> menuItems;

    // ??????
    auto battleBtn = MenuItemImage::create(
        "Icon/BattleIcon.png", "Icon/BattleIconPressed.png",   //????&???? ??
        CC_CALLBACK_1(SceneMap::onBattleButtonClicked, this));    // ???????

    battleBtn->setPosition(Vec2(visibleSize.width * 0.05f, visibleSize.height * 0.08f));
    menuItems.pushBack(battleBtn);

    // ????
    auto shopBtn = MenuItemImage::create(
        "Icon/shop.png", "Icon/shopPressed.png",
        CC_CALLBACK_1(SceneMap::onShopButtonClicked, this));    // ???????

    shopBtn->setPosition(Vec2(visibleSize.width * 0.95f, visibleSize.height * 0.08f));
    menuItems.pushBack(shopBtn);

    auto menu = Menu::createWithArray(menuItems);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 100);
}

void SceneMap::onBattleButtonClicked(Ref* sender)
{
    // Create a simple popup for mode selection
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    // Background shade
    auto shade = LayerColor::create(Color4B(0, 0, 0, 200));
    this->addChild(shade, 200);
    
    // Menu Items
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
    
    // Catch touches to prevent clicking through
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* t, Event* e){ return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, shade);
}

void SceneMap::onShopButtonClicked(Ref* sender)
{
    CCLOG("Shop button clicked, switching to shop scene");

    // ??? GameManager ?§Ý????????
    auto gameManager = GameManager::getInstance();
    gameManager->gotoShopScene();
}

// ???????????????shop????
// void SceneMap::setupBuildingMenu()
// {
//     auto visibleSize = Director::getInstance()->getVisibleSize();

//     // ????????
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

// void SceneMap::onBuildingSelected(Ref* sender, BuildingType type)
// {
//     // ???????????????????????????
//     if (_buildingPreview && (_buildingPreview->isPreviewing() || _buildingPreview->isMoving()))
//     {
//         _buildingPreview->cancel();
//     }

//     _isPlacingBuilding = true;
//     _selectedBuildingType = type;

//     // ??????
//     if (_buildingPreview)
//     {
//         _buildingPreview->startPreview(type);
//     }

//     // ????UI??
//     // ...
// }

bool SceneMap::onMapTouched(Touch* touch, Event* event)
{
    Vec2 touchPos = touch->getLocation();
    
    CCLOG("SceneMap::onMapTouched called at (%.2f, %.2f)", touchPos.x, touchPos.y);

    // ???BuildingPreview?????§¹
    if (!_buildingPreview) {
        CCLOG("BuildingPreview is null, cannot process touch");
        return false;
    }

    // ?????????????????????????????
    if (_buildingPreview->isPreviewing() && !_buildingPreview->isPreviewDragging())
    {
        CCLOG("Preview building detected, starting drag detection");
        _buildingPreview->startPreviewDragDetection(touchPos);
        return true;
    }

    // ??????????????????????
    if (_buildingPreview->isMoving())
    {
        CCLOG("Building is moving, updating position");
        _buildingPreview->updatePreviewPosition(touchPos);
        return true;
    }

    // ????????????????????§ß?????
    if (_buildingPreview->isSimpleDragging())
    {
        CCLOG("Building is simple dragging, updating position");
        _buildingPreview->updatePreviewPosition(touchPos);
        return true;
    }

    // ???????§ß???????????????????????????
    // ??????????????????
    if (!_isMovingBuilding)
    {
        CCLOG("Checking for building click - moving:%s", 
              _isMovingBuilding ? "true" : "false");
        
        // ???HomeVillageMap???      
        if (!_homeVillageMap) {
            CCLOG("HomeVillageMap is null, cannot check buildings");
            return false;
        }
        
        // ????????????????????
        Vec2 mapPos = _homeVillageMap->getMapPosition();
        float zoom = _homeVillageMap->getZoom();
        Vec2 worldPos = (touchPos - mapPos) / zoom;

        CCLOG("SceneMap::onMapTouched - checking for building at world pos (%.2f, %.2f)", worldPos.x, worldPos.y);
        CCLOG("Current buildings count: %d", (int)_buildings.size());

        // ???BuildingPreview?????
        Building* clickedBuilding = _buildingPreview->checkBuildingAtPosition(worldPos, _buildings);
        
        if (clickedBuilding)
        {
            CCLOG("Building found via BuildingPreview, starting simple drag");
            _buildingPreview->startSimpleDrag(clickedBuilding);
            return true;
        }
        
        // ????????HomeVillageMap???????§Þ?÷Ï???????
        auto buildingsContainer = _homeVillageMap->getBuildingsContainer();
        if (buildingsContainer) {
            Vector<Building*> containerBuildings;
            auto existingBuildings = buildingsContainer->getChildren();
            CCLOG("Buildings in container: %d", (int)existingBuildings.size());
            
            for (auto building : existingBuildings) {
                auto buildingObj = dynamic_cast<Building*>(building);
                if (buildingObj) {
                    containerBuildings.pushBack(buildingObj);
                }
            }
            
            // ???BuildingPreview?????????????????§Ö????
            clickedBuilding = _buildingPreview->checkBuildingAtPosition(worldPos, containerBuildings);
            
            if (clickedBuilding)
            {
                CCLOG("Building found in container via BuildingPreview, starting simple drag");
                
                // ?????????????SceneMap???§Ò???
                bool found = false;
                for (auto existingBuilding : _buildings) {
                    if (existingBuilding == clickedBuilding) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    _buildings.pushBack(clickedBuilding);
                    CCLOG("Added building to SceneMap buildings list");
                }
                
                _buildingPreview->startSimpleDrag(clickedBuilding);
                return true;
            }
        }
        
        CCLOG("No building found at touch position");
    }
    else
    {
        CCLOG("Cannot check buildings - moving:%s preview:%s", 
              _isMovingBuilding ? "true" : "false",
              _buildingPreview ? "valid" : "null");
    }

    // ??????????¦Ê¦Í?????????false????????????
    // ???????UI?????????????UI??????
    return false;
}

void SceneMap::onMouseMoved(Event* event)
{
    if (!_buildingPreview) {
        return;
    }
    
    EventMouse* mouseEvent = static_cast<EventMouse*>(event);
    Vec2 mousePos = Vec2(mouseEvent->getCursorX(), mouseEvent->getCursorY());

    // ?????????????????????¦Ë??
    if (_buildingPreview->isPreviewing() && _buildingPreview->isPreviewDragging())
    {
        _buildingPreview->updatePreviewPosition(mousePos);
    }
    // ????????????
    else if (_buildingPreview->isMoving() || _buildingPreview->isSimpleDragging())
    {
        _buildingPreview->updatePreviewPosition(mousePos);
    }
}

void SceneMap::update(float dt)
{
    // ???????UI
    auto resourceMgr = ResourceManager::getInstance();
    resourceMgr->updateUI();
}

void SceneMap::checkPendingBuildingPlacement()
{
    auto gameManager = GameManager::getInstance();
    if (gameManager->hasPendingBuildingPlacement())
    {
        BuildingType buildingType = gameManager->getPendingBuildingType();
        CCLOG("Found pending building placement for type: %d", (int)buildingType);
        
        // ????????????????????????
        // ????????????????BuildingPreview????????????????????????????????????
        // ?????????startBuildingPlacement?§Ù????????????????????????
        // ????????????? resetBuildingPlacementState ??????????????????????????????
        // ????????? init ?§Ö??????????????????????????????????????????
        // ?????????? startBuildingPlacement ????????????????
        
        // startBuildingPlacement ?????? _isPlacingBuilding = true ?????? preview->startPreview
        startBuildingPlacement(buildingType);
        
        // ?????????????????????????????
        gameManager->clearPendingBuildingPlacement();
    }
}

void SceneMap::startBuildingPlacement(BuildingType buildingType)
{
    // ???????????????????????
    if (_buildingPreview && (_buildingPreview->isPreviewing() || _buildingPreview->isMoving()))
    {
        _buildingPreview->cancel();
    }

    _isPlacingBuilding = true;
    _selectedBuildingType = buildingType;

    // ??????
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

void SceneMap::reinitializeEventListeners()
{
    CCLOG("SceneMap::reinitializeEventListeners - setting up event listeners");
    
    // ????????????????§µ????????
    if (!this->isRunning()) {
        CCLOG("Scene is not running, scheduling event listener initialization");
        this->scheduleOnce([this](float dt) {
            this->reinitializeEventListeners();
        }, 0.1f, "delayed_event_init");
        return;
    }
    
    // ???????????
    if (_eventDispatcher) {
        _eventDispatcher->removeEventListenersForTarget(this);
    }
    
    CCLOG("Registering touch event listener...");
    
    // ????????????? - ??????????-1???????HomeVillageMap??????
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true); // ????????????????????(HomeVillageMap)
    listener->onTouchBegan = CC_CALLBACK_2(SceneMap::onMapTouched, this);
    listener->onTouchMoved = [this](Touch* touch, Event* event) {
        Vec2 touchPos = touch->getLocation();
        
        // ???????????????????????
        if (_buildingPreview && _buildingPreview->isPreviewing())
        {
            // ????????????§µ?????¦Ë??
            if (!_buildingPreview->isPreviewDragging())
            {
                if (_buildingPreview->checkDragStart(touchPos))
                {
                    // ??????
                    _buildingPreview->updatePreviewPosition(touchPos);
                }
            }
            else
            {
                // ??????????
                _buildingPreview->updatePreviewPosition(touchPos);
            }
        }
        // ??????????????????§ß?????
        else if (_buildingPreview && (_buildingPreview->isMoving() || _buildingPreview->isSimpleDragging()))
        {
            _buildingPreview->updatePreviewPosition(touchPos);
        }
        };
    listener->onTouchEnded = [this](Touch* touch, Event* event) {
        // ??????????
        if (_buildingPreview && _buildingPreview->isPreviewDragging())
        {
            _buildingPreview->endPreviewDrag();
        }
        // ?????????
        else if (_buildingPreview && _buildingPreview->isSimpleDragging())
        {
            _buildingPreview->endSimpleDrag();
        }
        };
    _eventDispatcher->addEventListenerWithFixedPriority(listener, -1);

    // ????????????????????????
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseMove = CC_CALLBACK_1(SceneMap::onMouseMoved, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
#endif

    // ???????? - ???????????
    auto buildingPlacedListener = EventListenerCustom::create("building_placed",
        [this](EventCustom* event) {
            // ???SceneMap?????§¹
            if (!this || !this->getScene()) {
                CCLOG("SceneMap object or scene is invalid, ignoring building_placed event");
                return;
            }
            
            Building* building = static_cast<Building*>(event->getUserData());
            CCLOG("Building placed event received for building: %p", building);
            
            if (building) {
                // ??????§Ò???????????§Ò??§µ?
                bool found = false;
                for (auto existingBuilding : this->_buildings) {
                    if (existingBuilding == building) {
                        found = true;
                        break;
                    }
                }
                
                if (!found) {
                    this->_buildings.pushBack(building);
                    CCLOG("Added building to SceneMap buildings list (total: %d)", (int)this->_buildings.size());
                } else {
                    CCLOG("Building already in SceneMap buildings list");
                }
                
                // ???¡Â?????
                this->_isPlacingBuilding = false;
                CCLOG("Building placed event processed successfully, _isPlacingBuilding set to false");
            } else {
                CCLOG("Warning: Building placed event received with null building");
                // ?????????????
                this->_isPlacingBuilding = false;
            }
        });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(buildingPlacedListener, this);

    auto buildingMovedListener = EventListenerCustom::create("building_moved",
        [this](EventCustom* event) {
            // ???SceneMap?????§¹
            if (this && this->getScene() && !this->getScene()->isRunning()) {
                CCLOG("SceneMap object or scene is invalid, ignoring building_moved event");
                return;
            }
            
            // ???????????????
            this->_isMovingBuilding = false;
            this->_selectedBuilding = nullptr;
            CCLOG("Building moved event received and processed");
        });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(buildingMovedListener, this);
    
    CCLOG("SceneMap event listeners reinitialized successfully");
}

void SceneMap::resetBuildingPlacementState()
{
    CCLOG("SceneMap::resetBuildingPlacementState - resetting building states");
    
    // ???????§Õ???????????????§à?????????
    auto gameManager = GameManager::getInstance();
    if (gameManager && gameManager->hasPendingBuildingPlacement()) {
        CCLOG("Found pending building placement, skipping state reset to preserve building preview");
        return;
    }
    
    _isPlacingBuilding = false;
    _isMovingBuilding = false;
    _selectedBuilding = nullptr;
    
    // ????¦Ê???????§Ö????
    if (_buildingPreview) {
        // ??????????????????????????????????????????????????????????????????????
        // ??????????PendingPlacement??????????cancel??
        // ??????SceneMap::init ??????????????????
        // ??????ShopScene??????PendingPlacement ????? TRUE??
        // ????????? if (gameManager->hasPendingBuildingPlacement()) ????????????
        
        if (_buildingPreview->isPreviewing() || _buildingPreview->isMoving()) {
            _buildingPreview->cancel();
            CCLOG("Cancelled existing building preview/move operations");
        }
    }
    
    CCLOG("Building placement state reset completed");
}
