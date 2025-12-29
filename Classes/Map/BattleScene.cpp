/*************************************************************
* @file     : BattleScene.cpp
* @function ：战斗场景核心实现 - 多关卡战斗逻辑+兵种部署+胜负判定
* @author   : 俞筱雨
* @note     : 1.支持多关卡差异化建筑布局，按关卡ID创建测试/简单/中等/默认敌人建筑配置；
*             2.封装兵种选择UI、长按部署、待机动画等交互逻辑，适配地图坐标系统；
*             3.实现战斗AI目标选择（兵种/防御建筑自动寻敌），帧更新驱动战斗逻辑；
*             4.定义胜利（全建筑摧毁）/失败（全兵种耗尽）判定规则，配套UI提示；
*             5.管理实体容器，区分建筑/兵种容器节点，适配地图缩放/移动同步；
*             6.复用TroopManager管理兵种数量，实时刷新UI标签与选中状态；
*             7.支持长按连续部署兵种，屏幕坐标转换为地图本地坐标适配部署逻辑
**************************************************************/


#include "BattleScene.h"
#include "HomeVillageMap.h"
#include "CampaignMap.h"
#include "Troops/TroopManager.h"
#include "ui/CocosGUI.h"

Scene* BattleScene::createScene(int levelId)
{
    return BattleScene::create(levelId);
}

BattleScene* BattleScene::create(int levelId)
{
    BattleScene* pRet = new(std::nothrow) BattleScene();
    if (pRet && pRet->init(levelId))
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

bool BattleScene::init(int levelId)
{
    if (!Scene::init())
        return false;

    _levelId = levelId;
    _battleStarted = false;
    _battleEnded = false;
    _enemyTownHall = nullptr;
    _enemyBuildings.clear();
    _selectedTroopType = TroopType::NONE;

    // 【添加】初始化胜负状态
    _victoryShown = false;
    _defeatShown = false;

    // 【添加】调试信息
    auto visibleSize = Director::getInstance()->getVisibleSize();
    CCLOG("Visible size: %.2f x %.2f", visibleSize.width, visibleSize.height);

    // 【添加】初始化实体容器
    _entities.clear();

    // 兵种管理
    auto tm = TroopManager::getInstance();
    const auto& selectedTroops = tm->getSelectedTroops();
    // 设置每个兵种的数量
    for (const auto& troop : selectedTroops) {
        tm->setTroopCount(troop.first, troop.second);
    }

    // 等级ID
    _gameMap = CampaignMap::create(levelId);
    if (!_gameMap) return false;
    this->addChild(_gameMap, 0);

    // 【修改】移除独立的_gameLayer，直接使用地图的容器
    // 这样建筑和兵种会跟随地图一起移动和缩放
    //_gameLayer = Layer::create();
    //this->addChild(_gameLayer, Z_TROOP);  

    // 【添加】初始化硬编码敌人建筑
    initHardcodedBuildings();

    setupUI();
    initBattleEntities();
    initTroopSelectionUI();
    initMapDeploymentListener();

    this->scheduleUpdate();

    return true;
}

void BattleScene::spawnBuilding(BattleBuildingType type, int x, int y, Team team) {
    auto building = BattleBuilding::create(type, team);
    
    // 【修改】将建筑添加到地图的建筑容器，这样它们会跟随地图移动缩放
    Vec2 worldPos = Vec2(x, y);
    building->setPosition(worldPos);
    _gameMap->addBuilding(building); // 使用BaseMap的建筑容器
    _entities.pushBack(building);
}

// 【修改】初始化硬编码敌人建筑 - 根据关卡ID配置不同布局
void BattleScene::initHardcodedBuildings()
{
    // 【修改】使用地图的网格坐标系统，并根据关卡ID创建不同布局
    
    // 在地图中央区域放置建筑（使用网格坐标）
    int centerX = _gameMap->getGridCols() / 2;
    int centerY = _gameMap->getGridRows() / 2;

    // 根据关卡ID创建不同的建筑布局
    switch (_levelId) {
    case 0: // 测试关卡
        createLevel0Buildings(centerX, centerY);
        break;
    case 1: // 简单关卡
        createLevel1Buildings(centerX, centerY);
        break;
    case 2: // 中等关卡
        createLevel2Buildings(centerX, centerY);
        break;
    default: // 默认布局
        createDefaultBuildings(centerX, centerY);
        break;
    }

    CCLOG("Initialized buildings for level %d on map grid coordinates", _levelId);
}

// 【添加】创建测试关卡建筑布局
void BattleScene::createLevel0Buildings(int centerX, int centerY)
{
    // 大本营放在中央
    Vec2 townHallPos = _gameMap->gridToWorld(centerX, centerY);
    auto townHall = BattleBuilding::create(BattleBuildingType::TOWN_HALL, Team::ENEMY);
    townHall->setPosition(townHallPos);
    _gameMap->addBuilding(townHall);
    _entities.pushBack(townHall);
}

// 【添加】创建简单关卡建筑布局
void BattleScene::createLevel1Buildings(int centerX, int centerY)
{
    // 大本营
    Vec2 townHallPos = _gameMap->gridToWorld(centerX, centerY + 2);
    auto townHall = BattleBuilding::create(BattleBuildingType::TOWN_HALL, Team::ENEMY);
    townHall->setPosition(townHallPos);
    _gameMap->addBuilding(townHall);
    _entities.pushBack(townHall);

    // 一门大炮
    Vec2 cannonPos = _gameMap->gridToWorld(centerX, centerY - 2);
    auto cannon = BattleBuilding::create(BattleBuildingType::CANNON, Team::ENEMY);
    cannon->setPosition(cannonPos);
    _gameMap->addBuilding(cannon);
    _entities.pushBack(cannon);
}

// 【添加】创建中等关卡建筑布局
void BattleScene::createLevel2Buildings(int centerX, int centerY)
{
    // 大本营放在中央
    Vec2 townHallPos = _gameMap->gridToWorld(centerX, centerY);
    auto townHall = BattleBuilding::create(BattleBuildingType::TOWN_HALL, Team::ENEMY);
    townHall->setPosition(townHallPos);
    _gameMap->addBuilding(townHall);
    _entities.pushBack(townHall);

    // 两门大炮
    Vec2 cannon1Pos = _gameMap->gridToWorld(centerX - 4, centerY + 2);
    auto cannon1 = BattleBuilding::create(BattleBuildingType::CANNON, Team::ENEMY);
    cannon1->setPosition(cannon1Pos);
    _gameMap->addBuilding(cannon1);
    _entities.pushBack(cannon1);

    Vec2 cannon2Pos = _gameMap->gridToWorld(centerX + 5, centerY - 1);
    auto cannon2 = BattleBuilding::create(BattleBuildingType::CANNON, Team::ENEMY);
    cannon2->setPosition(cannon2Pos);
    _gameMap->addBuilding(cannon2);
    _entities.pushBack(cannon2);

    // 箭塔
    Vec2 archerTowerPos = _gameMap->gridToWorld(centerX - 4, centerY - 2);
    auto archerTower = BattleBuilding::create(BattleBuildingType::ARCHER_TOWER, Team::ENEMY);
    archerTower->setPosition(archerTowerPos);
    _gameMap->addBuilding(archerTower);
    _entities.pushBack(archerTower);
}

// 【添加】创建默认建筑布局（原来的复杂布局）
void BattleScene::createDefaultBuildings(int centerX, int centerY)
{
    // 大本营放在中央
    Vec2 townHallPos = _gameMap->gridToWorld(centerX, centerY);
    auto townHall = BattleBuilding::create(BattleBuildingType::TOWN_HALL, Team::ENEMY);
    townHall->setPosition(townHallPos);
    _gameMap->addBuilding(townHall);
    _entities.pushBack(townHall);

    // 大炮放在左下方
    Vec2 cannonPos = _gameMap->gridToWorld(centerX - 3, centerY - 2);
    auto cannon = BattleBuilding::create(BattleBuildingType::CANNON, Team::ENEMY);
    cannon->setPosition(cannonPos);
    _gameMap->addBuilding(cannon);
    _entities.pushBack(cannon);

    // 箭塔放在右下方
    Vec2 archerTowerPos = _gameMap->gridToWorld(centerX + 3, centerY - 2);
    auto archerTower = BattleBuilding::create(BattleBuildingType::ARCHER_TOWER, Team::ENEMY);
    archerTower->setPosition(archerTowerPos);
    _gameMap->addBuilding(archerTower);
    _entities.pushBack(archerTower);

    // 金库放在左上方
    Vec2 goldStoragePos = _gameMap->gridToWorld(centerX - 4, centerY + 2);
    auto goldStorage = BattleBuilding::create(BattleBuildingType::GOLD_STORAGE, Team::ENEMY);
    goldStorage->setPosition(goldStoragePos);
    _gameMap->addBuilding(goldStorage);
    _entities.pushBack(goldStorage);

    // 圣水收集器放在右上方
    Vec2 elixirCollectorPos = _gameMap->gridToWorld(centerX + 4, centerY + 2);
    auto elixirCollector = BattleBuilding::create(BattleBuildingType::ELIXIR_COLLECTOR, Team::ENEMY);
    elixirCollector->setPosition(elixirCollectorPos);
    _gameMap->addBuilding(elixirCollector);
    _entities.pushBack(elixirCollector);

    // 金矿放在正上方
    Vec2 goldMinePos = _gameMap->gridToWorld(centerX, centerY + 4);
    auto goldMine = BattleBuilding::create(BattleBuildingType::GOLD_MINE, Team::ENEMY);
    goldMine->setPosition(goldMinePos);
    _gameMap->addBuilding(goldMine);
    _entities.pushBack(goldMine);
}

void BattleScene::initBattleEntities()
{
    // 战斗地图
    auto buildings = _gameMap->getBuildingsContainer()->getChildren();

    for (auto node : buildings)
    {
        Building* building = dynamic_cast<Building*>(node);
        if (building)
        {
            _enemyBuildings.pushBack(building);

            // 大本营
            if (dynamic_cast<TownHall*>(building))
            {
                _enemyTownHall = building;
                CCLOG("Found TownHall at position: %.2f, %.2f", building->getPositionX(), building->getPositionY());
            }
        }
    }
}

void BattleScene::setupUI()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    auto backBtn = MenuItemLabel::create(
        Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 24),
        [](Ref* sender) {
            Director::getInstance()->popScene();
        });
    backBtn->setPosition(visibleSize.width - 100, visibleSize.height - 50);

    auto menu = Menu::create(backBtn, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 100);
}

// 兵种待机动画
Animation* BattleScene::createTroopIdleAnimation(TroopType type) {
    Animation* animation = Animation::create();
    animation->setDelayPerUnit(0.5f); 

    std::string basePath = "";
    switch (type) {
    case TroopType::BARBARIAN: basePath = "Troops/Barbarian"; break;
    case TroopType::ARCHER: basePath = "Troops/Archer"; break;
    case TroopType::GOBLIN: basePath = "Troops/Goblin"; break;
    case TroopType::GIANT: basePath = "Troops/Giant"; break;
    default: basePath = "Troops/Default"; break;
    }

    bool hasAnimationFrames = false;
    for (int i = 1; i <= 3; i++) {
        std::string framePath = basePath + "_idle_" + std::to_string(i) + ".png";
        if (FileUtils::getInstance()->isFileExist(framePath)) {
            animation->addSpriteFrameWithFile(framePath);
            hasAnimationFrames = true;
        }
    }

    if (!hasAnimationFrames) {
        std::string staticPath = basePath + ".png";
        if (FileUtils::getInstance()->isFileExist(staticPath)) {
            animation->addSpriteFrameWithFile(staticPath);
        }
    }

    return animation;
}

void BattleScene::playSelectionAnimation(Sprite* sprite) {
    if (!sprite) return;

    sprite->setColor(Color3B(255, 255, 150));

    auto scale1 = ScaleTo::create(0.5f, 1.1f);
    auto scale2 = ScaleTo::create(0.5f, 1.0f);
    auto breathe = RepeatForever::create(Sequence::create(scale1, scale2, nullptr));
    breathe->setTag(999); 
    sprite->runAction(breathe);
}

void BattleScene::stopSelectionAnimation(Sprite* sprite) {
    if (!sprite) return;

    sprite->setColor(Color3B(255, 255, 255)); 
    sprite->stopActionByTag(999);
    sprite->setScale(1.0f); 
}

void BattleScene::initTroopSelectionUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    float startX = visibleSize.width * 0.2f;
    float gapX = visibleSize.width * 0.2f;
    float bottomY = origin.y + 120;
    std::vector<TroopType> types = { TroopType::BARBARIAN, TroopType::ARCHER, TroopType::GOBLIN, TroopType::GIANT };

    int index = 0;
    for (auto type : types) {
        float xPos = startX + (index * gapX);
        const auto& info = TROOP_DATA.at(type);

        auto troopSprite = Sprite::create(info.imagePath);
        if (!troopSprite) {
            CCLOG("Failed to load troop image: %s", info.imagePath.c_str());
            troopSprite = Sprite::create();
            auto drawNode = DrawNode::create();
            Color4F color = Color4F::GRAY;
            switch (type) {
            case TroopType::BARBARIAN: color = Color4F::RED; break;
            case TroopType::ARCHER: color = Color4F::GREEN; break;
            case TroopType::GOBLIN: color = Color4F::BLUE; break;
            case TroopType::GIANT: color = Color4F::YELLOW; break;
            default: break;
            }
            drawNode->drawSolidCircle(Vec2::ZERO, 40, 0, 32, color);
            troopSprite->addChild(drawNode);
            troopSprite->setContentSize(Size(80, 80));
        }

        troopSprite->setPosition(Vec2(xPos, bottomY));

        float maxSide = 80.0f;
        Size spriteSize = troopSprite->getContentSize();
        if (spriteSize.width > maxSide || spriteSize.height > maxSide) {
            float scale = std::min(maxSide / spriteSize.width, maxSide / spriteSize.height);
            troopSprite->setScale(scale);
        }

        troopSprite->setTag(static_cast<int>(type));
        this->addChild(troopSprite, 10);

        _troopSprites[type] = troopSprite;

        auto listener = EventListenerTouchOneByOne::create();
        listener->setSwallowTouches(true);
        listener->onTouchBegan = [this, type](Touch* touch, Event* event) {
            auto sprite = _troopSprites[type];
            auto bounds = sprite->getBoundingBox();
            if (bounds.containsPoint(touch->getLocation())) {
                auto tm = TroopManager::getInstance();
                int count = tm->getTroopCount(type);

                if (count <= 0) {
                    sprite->setOpacity(100);
                    auto fadeIn = FadeTo::create(0.2f, 255);
                    sprite->runAction(fadeIn);
                    return true;
                }

                this->updateTroopSelection(type);
                return true;
            }
            return false;
            };
        _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, troopSprite);

        auto nameLabel = Label::createWithTTF(info.name, "fonts/Marker Felt.ttf", 16);
        nameLabel->setPosition(Vec2(xPos, bottomY + 50));
        this->addChild(nameLabel, 10);

        auto spaceLabel = Label::createWithTTF("Space: " + std::to_string(info.housingSpace), "fonts/Marker Felt.ttf", 12);
        spaceLabel->setPosition(Vec2(xPos, bottomY + 30));
        this->addChild(spaceLabel, 10);

        auto tm = TroopManager::getInstance();
        int count = tm->getTroopCount(type);
        auto countLabel = Label::createWithTTF(std::to_string(count), "fonts/Marker Felt.ttf", 18);
        countLabel->setPosition(Vec2(xPos, bottomY - 50));
        countLabel->setColor(Color3B::YELLOW);
        this->addChild(countLabel, 10);

        _troopCountLabels[type] = countLabel;

        index++;
    }
}

void BattleScene::updateTroopSelection(TroopType selectedType) {
    if (_selectedTroopSprite != nullptr) {
        stopSelectionAnimation(_selectedTroopSprite);
    }

    _selectedTroopType = selectedType;
    _selectedTroopSprite = _troopSprites[selectedType];

    if (_selectedTroopSprite != nullptr) {
        playSelectionAnimation(_selectedTroopSprite);
        CCLOG("Selected troop type: %d", static_cast<int>(selectedType));
    }
}

void BattleScene::initMapDeploymentListener() {
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(false); // ?UI??

    listener->onTouchBegan = [this](Touch* touch, Event* event) {
        if (_selectedTroopType == TroopType::NONE) return false;

        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 touchPos = touch->getLocation();
        if (touchPos.y < 200) { // UI??200
            return false;
        }

        auto tm = TroopManager::getInstance();
        int remaining = tm->getTroopCount(_selectedTroopType);
        if (remaining <= 0) return false;

        deployTroop(touch->getLocation());
        _isLongPressing = true;

        this->schedule([this](float dt) { this->onLongPressDeploy(dt); }, 0.3f, "troop_deploy");
        return true;
        };

    listener->onTouchMoved = [this](Touch* touch, Event* event) {
        if (_isLongPressing) {
            _lastDeployPos = touch->getLocation();
        }
        };

    listener->onTouchEnded = [this](Touch* touch, Event* event) {
        _isLongPressing = false;
        this->unschedule("troop_deploy");
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void BattleScene::deployTroop(const Vec2& mapPos) {
    auto tm = TroopManager::getInstance();
    int count = tm->getTroopCount(_selectedTroopType);

    if (count <= 0) return;

    // 【修改】将屏幕坐标转换为地图坐标系
    // 使用地图的兵种容器，这样兵种会跟随地图移动缩放
    Node* troopsContainer = _gameMap->getTroopsContainer();
    Vec2 localPos = troopsContainer->convertToNodeSpace(mapPos);

    // 直接创建 BattleTroop 而不是 Unit
    BattleTroopType battleType;
    switch (_selectedTroopType) {
    case TroopType::BARBARIAN: battleType = BattleTroopType::BARBARIAN; break;
    case TroopType::ARCHER: battleType = BattleTroopType::ARCHER; break;
    case TroopType::GOBLIN: battleType = BattleTroopType::GOBLIN; break;
    case TroopType::GIANT: battleType = BattleTroopType::GIANT; break;
    default: battleType = BattleTroopType::BARBARIAN; break;
    }

    // 【修改】将兵种添加到地图的兵种容器，这样它们会跟随地图移动缩放
    auto troop = BattleTroop::create(battleType, Team::PLAYER);
    troop->setPosition(localPos);
    troopsContainer->addChild(troop, Z_TROOP);

    // 添加到实体容器中
    _entities.pushBack(troop);

    CCLOG("Deployed battle troop at map position: %.2f, %.2f", localPos.x, localPos.y);

    // 寻找目标（使用 _entities 中的所有实体）
    troop->findNewTarget(_entities);

    // 更新数量
    tm->setTroopCount(_selectedTroopType, count - 1);
    updateLabels();

    if ((count - 1) <= 0) {
        if (_selectedTroopSprite != nullptr) {
            stopSelectionAnimation(_selectedTroopSprite);
        }
        _selectedTroopType = TroopType::NONE;
        _selectedTroopSprite = nullptr;
    }
    // 【添加】当某种兵种数量为0时立即检查战斗结果
    checkBattleResult();
}

void BattleScene::onLongPressDeploy(float dt) {
    if (_isLongPressing && _lastDeployPos != Vec2::ZERO) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        if (_lastDeployPos.y > 200) { // ?UI
            deployTroop(_lastDeployPos);
        }
    }
}

void BattleScene::updateLabels() {
    auto tm = TroopManager::getInstance();

    for (auto& pair : _troopCountLabels) {
        TroopType type = pair.first;
        Label* label = pair.second;
        if (label) {
            int count = tm->getTroopCount(type);
            label->setString(std::to_string(count));

            auto sprite = _troopSprites[type];
            if (sprite) {
                if (count <= 0) {
                    sprite->setOpacity(100); 
                    label->setColor(Color3B::RED);
                }
                else {
                    sprite->setOpacity(255);
                    label->setColor(Color3B::YELLOW);
                }
            }
        }
    }
}

// 检查所有建筑是否都被摧毁
bool BattleScene::areAllBuildingsDestroyed()
{
    for (auto entity : _entities) {
        BattleBuilding* building = dynamic_cast<BattleBuilding*>(entity);
        if (building && building->getTeam() == Team::ENEMY) {
            if (!building->isDead()) {
                return false; // 还有存活的建筑
            }
        }
    }
    return true; // 所有建筑都被摧毁
}

// 检查所有兵种是否都已投放且死亡
bool BattleScene::areAllTroopsDeployedAndDead()
{
    auto tm = TroopManager::getInstance();

    // 检查是否还有未投放的兵种
    for (auto& pair : _troopCountLabels) {
        TroopType type = pair.first;
        int count = tm->getTroopCount(type);
        if (count > 0) {
            return false; // 还有未投放的兵种
        }
    }

    // 检查场上是否还有存活的兵种
    for (auto entity : _entities) {
        BattleTroop* troop = dynamic_cast<BattleTroop*>(entity);
        if (troop && troop->getTeam() == Team::PLAYER) {
            if (!troop->isDead()) {
                return false; // 还有存活的兵种
            }
        }
    }

    return true; // 所有兵种都已投放且死亡
}

// 检查战斗结果
void BattleScene::checkBattleResult()
{
    // 如果已经显示过胜负，不再重复检查
    if (_victoryShown || _defeatShown) {
        return;
    }

    // 胜利条件：所有建筑都被摧毁
    if (areAllBuildingsDestroyed()) {
        showVictoryUI();
        return;
    }

    // 失败条件：所有兵种都已投放且死亡，但还有存活的建筑
    if (areAllTroopsDeployedAndDead() && !areAllBuildingsDestroyed()) {
        showDefeatUI();
        return;
    }
}

// 显示胜利UI
void BattleScene::showVictoryUI()
{
    if (_victoryShown) return;
    _victoryShown = true;
    _battleEnded = true;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建胜利标签
    auto victoryLabel = Label::createWithTTF("Victory", "fonts/Marker Felt.ttf", 72);
    victoryLabel->setPosition(visibleSize.width / 2, visibleSize.height * 0.7f);
    victoryLabel->setColor(Color3B::GREEN);
    victoryLabel->enableOutline(Color4B::BLACK, 3);
    this->addChild(victoryLabel, 200);

    // 添加闪烁效果
    auto fadeOut = FadeTo::create(1.0f, 150);
    auto fadeIn = FadeTo::create(1.0f, 255);
    auto sequence = Sequence::create(fadeOut, fadeIn, nullptr);
    auto repeat = RepeatForever::create(sequence);
    victoryLabel->runAction(repeat);

    CCLOG("Victory! All enemy buildings destroyed!");
}

// 显示失败UI
void BattleScene::showDefeatUI()
{
    if (_defeatShown) return;
    _defeatShown = true;
    _battleEnded = true;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建失败标签
    auto defeatLabel = Label::createWithTTF("Defeat", "fonts/Marker Felt.ttf", 72);
    defeatLabel->setPosition(visibleSize.width / 2, visibleSize.height * 0.7f);
    defeatLabel->setColor(Color3B::RED);
    defeatLabel->enableOutline(Color4B::BLACK, 3);
    this->addChild(defeatLabel, 200);

    // 添加闪烁效果
    auto fadeOut = FadeTo::create(1.0f, 150);
    auto fadeIn = FadeTo::create(1.0f, 255);
    auto sequence = Sequence::create(fadeOut, fadeIn, nullptr);
    auto repeat = RepeatForever::create(sequence);
    defeatLabel->runAction(repeat);

    CCLOG("Defeat! All troops deployed and dead, but enemy buildings remain.");
}

void BattleScene::onTroopDeselected(TroopType type) {
    auto tm = TroopManager::getInstance();
    int current = tm->getTroopCount(type);
    if (current > 0) {
        tm->setTroopCount(type, current - 1);
        updateLabels();
    }
}

// 【添加】更新函数，处理战斗逻辑
void BattleScene::update(float dt)
{
    // 如果战斗已经结束，不再更新战斗逻辑
    if (_battleEnded) return;

    // 1. AI 目标选择
    for (auto entity : _entities) {
        if (entity->isDead()) continue;

        // 处理兵种目标
        BattleTroop* troop = dynamic_cast<BattleTroop*>(entity);
        if (troop) {
            troop->update(dt);
            if (!troop->getTarget() || troop->getTarget()->isDead()) {
                troop->findNewTarget(_entities);
            }
        }

        // 处理防御建筑目标
        BattleBuilding* building = dynamic_cast<BattleBuilding*>(entity);
        if (building) {
            // 检查是否是防御建筑（有攻击力）
            auto stats = BattleManager::getInstance()->getBattleBuildingStats(building->getType());
            if (stats.damage > 0) {
                if (!building->getTarget() || building->getTarget()->isDead()) {
                    // 寻找最近的敌方兵种
                    float minDist = FLT_MAX;
                    Entity* bestTarget = nullptr;
                    for (auto target : _entities) {
                        if (target->getTeam() != building->getTeam() && !target->isDead()) {
                            // 检查是否是兵种（防御建筑只攻击兵种）
                            BattleTroop* enemyTroop = dynamic_cast<BattleTroop*>(target);
                            if (enemyTroop) {
                                float dist = building->getPosition().distance(target->getPosition());
                                if (dist < minDist && dist <= stats.attackRange) {
                                    minDist = dist;
                                    bestTarget = target;
                                }
                            }
                        }
                    }
                    building->setTarget(bestTarget);
                }
            }
        }
    }

    // 2. 清理死亡实体
    //cleanupDeadEntities();

    // 3. 【添加】每帧检查战斗结果
    checkBattleResult();
}

//// 【添加】清理死亡实体
//void BattleScene::cleanupDeadEntities()
//{
//    cocos2d::Vector<Entity*> aliveEntities;
//    for (auto entity : _entities) {
//        if (!entity->isDead()) {
//            aliveEntities.pushBack(entity);
//        }
//        else {
//            // 检查是否所有敌人都死了
//            if (entity->getTeam() == Team::ENEMY) {
//                // 如果是市政厅，检查是否游戏胜利
//                BattleBuilding* building = dynamic_cast<BattleBuilding*>(entity);
//                if (building && building->getType() == BattleBuildingType::TOWN_HALL) {
//                    // 游戏胜利逻辑
//                    CCLOG("Enemy Town Hall destroyed! Victory!");
//                    _battleEnded = true;
//                    showVictoryUI();
//                }
//            }
//        }
//    }
//    _entities.swap(aliveEntities);
//}
