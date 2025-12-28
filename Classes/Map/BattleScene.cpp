//#include "BattleScene.h"
//#include "HomeVillageMap.h"
//#include "CampaignMap.h"
//#include "Troops/TroopManager.h"
//#include "ui/CocosGUI.h"
//
//Scene* BattleScene::createScene(int levelId)
//{
//    return BattleScene::create(levelId);
//}
//
//BattleScene* BattleScene::create(int levelId)
//{
//    BattleScene *pRet = new(std::nothrow) BattleScene();
//    if (pRet && pRet->init(levelId))
//    {
//        pRet->autorelease();
//        return pRet;
//    }
//    else
//    {
//        delete pRet;
//        pRet = nullptr;
//        return nullptr;
//    }
//}
//
//bool BattleScene::init(int levelId)
//{
//    if (!Scene::init())
//        return false;
//
//    _levelId = levelId;
//    _battleStarted = false;
//    _battleEnded = false;
//    _enemyTownHall = nullptr;
//    _enemyBuildings.clear();
//    _selectedTroopType = TroopType::NONE;
//
//    // 初始化TroopManager中的兵种数量(测试用)
//    auto tm = TroopManager::getInstance();
//    tm->setTroopCount(TroopType::BARBARIAN, 10);
//    tm->setTroopCount(TroopType::ARCHER, 8);
//    tm->setTroopCount(TroopType::GOBLIN, 6);
//    tm->setTroopCount(TroopType::GIANT, 3);
//
//    // 根据levelId初始化地图
//    _gameMap = CampaignMap::create(levelId);
//    if (!_gameMap) return false;
//    this->addChild(_gameMap, 0);
//    
//    setupUI();
//    initBattleEntities();
//    initTroopSelectionUI();
//    initMapDeploymentListener();
//
//    this->scheduleUpdate();
//
//    return true;
//}
//
//void BattleScene::initBattleEntities()
//{
//    // 从地图（由CampaignMap填充）中获取建筑物
//    auto buildings = _gameMap->getBuildingsContainer()->getChildren();
//    
//    for (auto node : buildings)
//    {
//        Building* building = dynamic_cast<Building*>(node);
//        if (building)
//        {
//            _enemyBuildings.pushBack(building);
//            
//            // 检查它是否是TownHall
//            if (dynamic_cast<TownHall*>(building))
//            {
//                _enemyTownHall = building;
//                CCLOG("Found TownHall at position: %.2f, %.2f", building->getPositionX(), building->getPositionY());
//            }
//        }
//    }
//}
//
//void BattleScene::setupUI()
//{
//    auto visibleSize = Director::getInstance()->getVisibleSize();
//
//    auto backBtn = MenuItemLabel::create(
//        Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 24),
//        [](Ref* sender) {
//            Director::getInstance()->popScene();
//        });
//    backBtn->setPosition(visibleSize.width - 100, visibleSize.height - 50);
//
//    auto menu = Menu::create(backBtn, nullptr);
//    menu->setPosition(Vec2::ZERO);
//    this->addChild(menu, 100);
//}
//
//// 创建兵种帧动画
//Animation* BattleScene::createTroopIdleAnimation(TroopType type) {
//    Animation* animation = Animation::create();
//    animation->setDelayPerUnit(0.5f); // 每帧0.5秒
//    
//    // 尝试加载真实的兵种图片序列(如果有的话)
//    std::string basePath = "";
//    switch(type) {
//        case TroopType::BARBARIAN: basePath = "Troops/Barbarian"; break;
//        case TroopType::ARCHER: basePath = "Troops/Archer"; break;
//        case TroopType::GOBLIN: basePath = "Troops/Goblin"; break;
//        case TroopType::GIANT: basePath = "Troops/Giant"; break;
//        default: basePath = "Troops/Default"; break;
//    }
//    
//    // 尝试加载动画帧(假设有idle_1.png, idle_2.png等)
//    bool hasAnimationFrames = false;
//    for(int i = 1; i <= 3; i++) {
//        std::string framePath = basePath + "_idle_" + std::to_string(i) + ".png";
//        if(FileUtils::getInstance()->isFileExist(framePath)) {
//            animation->addSpriteFrameWithFile(framePath);
//            hasAnimationFrames = true;
//        }
//    }
//    
//    // 如果没有动画帧，就使用静态图片
//    if(!hasAnimationFrames) {
//        std::string staticPath = basePath + ".png";
//        if(FileUtils::getInstance()->isFileExist(staticPath)) {
//            animation->addSpriteFrameWithFile(staticPath);
//        }
//    }
//    
//    return animation;
//}
//
//// 播放选中动画
//void BattleScene::playSelectionAnimation(Sprite* sprite) {
//    if(!sprite) return;
//    
//    // 高亮效果
//    sprite->setColor(Color3B(255, 255, 150)); // 浅黄色高亮
//    
//    // 缩放呼吸动画
//    auto scale1 = ScaleTo::create(0.5f, 1.1f);
//    auto scale2 = ScaleTo::create(0.5f, 1.0f);
//    auto breathe = RepeatForever::create(Sequence::create(scale1, scale2, nullptr));
//    breathe->setTag(999); // 设置标签以便停止
//    sprite->runAction(breathe);
//}
//
//void BattleScene::stopSelectionAnimation(Sprite* sprite) {
//    if(!sprite) return;
//    
//    sprite->setColor(Color3B(255, 255, 255)); // 恢复原色
//    sprite->stopActionByTag(999); // 停止呼吸动画
//    sprite->setScale(1.0f); // 恢复原始大小
//}
//
//void BattleScene::initTroopSelectionUI() {
//    auto visibleSize = Director::getInstance()->getVisibleSize();
//    Vec2 origin = Director::getInstance()->getVisibleOrigin();
//
//    float startX = visibleSize.width * 0.2f;
//    float gapX = visibleSize.width * 0.2f;
//    float bottomY = origin.y + 120; // 从底部往上120像素
//    std::vector<TroopType> types = { TroopType::BARBARIAN, TroopType::ARCHER, TroopType::GOBLIN, TroopType::GIANT };
//
//    int index = 0;
//    for (auto type : types) {
//        float xPos = startX + (index * gapX);
//        const auto& info = TROOP_DATA.at(type);
//
//        // 创建兵种精灵
//        auto troopSprite = Sprite::create(info.imagePath);
//        if (!troopSprite) {
//            // 如果图片加载失败，创建占位符
//            CCLOG("Failed to load troop image: %s", info.imagePath.c_str());
//            troopSprite = Sprite::create();
//            auto drawNode = DrawNode::create();
//            Color4F color = Color4F::GRAY;
//            switch(type) {
//                case TroopType::BARBARIAN: color = Color4F::RED; break;
//                case TroopType::ARCHER: color = Color4F::GREEN; break;
//                case TroopType::GOBLIN: color = Color4F::BLUE; break;
//                case TroopType::GIANT: color = Color4F::YELLOW; break;
//                default: break;
//            }
//            drawNode->drawSolidCircle(Vec2::ZERO, 40, 0, 32, color);
//            troopSprite->addChild(drawNode);
//            troopSprite->setContentSize(Size(80, 80));
//        }
//
//        troopSprite->setPosition(Vec2(xPos, bottomY));
//        
//        // 缩放控制
//        float maxSide = 80.0f;
//        Size spriteSize = troopSprite->getContentSize();
//        if (spriteSize.width > maxSide || spriteSize.height > maxSide) {
//            float scale = std::min(maxSide / spriteSize.width, maxSide / spriteSize.height);
//            troopSprite->setScale(scale);
//        }
//        
//        troopSprite->setTag(static_cast<int>(type));
//        this->addChild(troopSprite, 10);
//        
//        // 存储到映射中
//        _troopSprites[type] = troopSprite;
//
//        // 添加点击监听
//        auto listener = EventListenerTouchOneByOne::create();
//        listener->setSwallowTouches(true);
//        listener->onTouchBegan = [this, type](Touch* touch, Event* event) {
//            auto sprite = _troopSprites[type];
//            auto bounds = sprite->getBoundingBox();
//            if (bounds.containsPoint(touch->getLocation())) {
//                // 检查是否有可用数量
//                auto tm = TroopManager::getInstance();
//                int count = tm->getTroopCount(type);
//                
//                if (count <= 0) {
//                    // 数量为0时提示
//                    sprite->setOpacity(100);
//                    auto fadeIn = FadeTo::create(0.2f, 255);
//                    sprite->runAction(fadeIn);
//                    return true;
//                }
//
//                // 更新选中状态
//                this->updateTroopSelection(type);
//                return true;
//            }
//            return false;
//        };
//        _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, troopSprite);
//
//        // 兵种名称
//        auto nameLabel = Label::createWithTTF(info.name, "fonts/Marker Felt.ttf", 16);
//        nameLabel->setPosition(Vec2(xPos, bottomY + 50));
//        this->addChild(nameLabel, 10);
//
//        // 占用空间信息
//        auto spaceLabel = Label::createWithTTF("Space: " + std::to_string(info.housingSpace), "fonts/Marker Felt.ttf", 12);
//        spaceLabel->setPosition(Vec2(xPos, bottomY + 30));
//        this->addChild(spaceLabel, 10);
//
//        // 数量显示
//        auto tm = TroopManager::getInstance();
//        int count = tm->getTroopCount(type);
//        auto countLabel = Label::createWithTTF(std::to_string(count), "fonts/Marker Felt.ttf", 18);
//        countLabel->setPosition(Vec2(xPos, bottomY - 50));
//        countLabel->setColor(Color3B::YELLOW);
//        this->addChild(countLabel, 10);
//        
//        // 存储到映射中
//        _troopCountLabels[type] = countLabel;
//
//        index++;
//    }
//}
//
//// 更新选中状态
//void BattleScene::updateTroopSelection(TroopType selectedType) {
//    // 清除之前选中状态
//    if (_selectedTroopSprite != nullptr) {
//        stopSelectionAnimation(_selectedTroopSprite);
//    }
//
//    // 设置新选中状态
//    _selectedTroopType = selectedType;
//    _selectedTroopSprite = _troopSprites[selectedType];
//
//    if (_selectedTroopSprite != nullptr) {
//        playSelectionAnimation(_selectedTroopSprite);
//        CCLOG("Selected troop type: %d", static_cast<int>(selectedType));
//    }
//}
//
//// 初始化地图放置监听
//void BattleScene::initMapDeploymentListener() {
//    auto listener = EventListenerTouchOneByOne::create();
//    listener->setSwallowTouches(false); // 不吞噬触摸，允许UI元素响应
//    
//    listener->onTouchBegan = [this](Touch* touch, Event* event) {
//        // 未选中兵种时不处理
//        if (_selectedTroopType == TroopType::NONE) return false;
//
//        // 检查触摸位置是否在UI区域(避免误触)
//        auto visibleSize = Director::getInstance()->getVisibleSize();
//        Vec2 touchPos = touch->getLocation();
//        if(touchPos.y < 200) { // UI区域在底部200像素内
//            return false;
//        }
//
//        auto tm = TroopManager::getInstance();
//        int remaining = tm->getTroopCount(_selectedTroopType);
//        if (remaining <= 0) return false;
//
//        // 处理单击放置
//        deployTroop(touch->getLocation());
//        _isLongPressing = true;
//
//        // 启动长按连续放置定时器
//        this->schedule([this](float dt) { this->onLongPressDeploy(dt); }, 0.3f, "troop_deploy");
//        return true;
//    };
//
//    listener->onTouchMoved = [this](Touch* touch, Event* event) {
//        if (_isLongPressing) {
//            _lastDeployPos = touch->getLocation();
//        }
//    };
//
//    listener->onTouchEnded = [this](Touch* touch, Event* event) {
//        _isLongPressing = false;
//        this->unschedule("troop_deploy");
//    };
//
//    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
//}
//
//// 放置兵种实现
//void BattleScene::deployTroop(const Vec2& mapPos) {
//    auto tm = TroopManager::getInstance();
//    int count = tm->getTroopCount(_selectedTroopType);
//    
//    if (count <= 0) return;
//
//    // 创建兵种单位
//    auto troop = Unit::create(_selectedTroopType);
//    troop->setPosition(mapPos);
//    this->addChild(troop, 5);
//
//    CCLOG("Deployed troop at position: %.2f, %.2f", mapPos.x, mapPos.y);
//
//    // 向大本营移动(攻击测试)
//    if (_enemyTownHall) {
//        Vec2 targetPos = _enemyTownHall->getPosition();
//        CCLOG("Moving troop to TownHall at: %.2f, %.2f", targetPos.x, targetPos.y);
//        
//        // 计算移动时间
//        float distance = mapPos.distance(targetPos);
//        float moveSpeed = 100.0f; // 像素/秒
//        float duration = distance / moveSpeed;
//        
//        // 移动动作
//        auto moveTo = MoveTo::create(duration, targetPos);
//        auto callback = CallFunc::create([=]() {
//            CCLOG("Troop reached TownHall!");
//            // 这里可以添加攻击逻辑
//            troop->wanderAround(targetPos, 50.0f); // 到达后在大本营周围游荡
//        });
//        
//        troop->runAction(Sequence::create(moveTo, callback, nullptr));
//    } else {
//        // 如果没有大本营，随机游荡
//        troop->wanderAround(mapPos, 200.0f);
//    }
//
//    // 更新剩余数量
//    tm->setTroopCount(_selectedTroopType, count - 1);
//    updateLabels();
//
//    // 数量为0时取消选中
//    if ((count - 1) <= 0) {
//        if (_selectedTroopSprite != nullptr) {
//            stopSelectionAnimation(_selectedTroopSprite);
//        }
//        _selectedTroopType = TroopType::NONE;
//        _selectedTroopSprite = nullptr;
//    }
//}
//
//// 长按连续放置回调
//void BattleScene::onLongPressDeploy(float dt) {
//    if (_isLongPressing && _lastDeployPos != Vec2::ZERO) {
//        auto visibleSize = Director::getInstance()->getVisibleSize();
//        if(_lastDeployPos.y > 200) { // 确保不在UI区域
//            deployTroop(_lastDeployPos);
//        }
//    }
//}
//
//// 更新数量显示
//void BattleScene::updateLabels() {
//    auto tm = TroopManager::getInstance();
//
//    for (auto& pair : _troopCountLabels) {
//        TroopType type = pair.first;
//        Label* label = pair.second;
//        if (label) {
//            int count = tm->getTroopCount(type);
//            label->setString(std::to_string(count));
//            
//            // 数量为0时的视觉反馈
//            auto sprite = _troopSprites[type];
//            if (sprite) {
//                if (count <= 0) {
//                    sprite->setOpacity(100); // 置灰
//                    label->setColor(Color3B::RED);
//                } else {
//                    sprite->setOpacity(255);
//                    label->setColor(Color3B::YELLOW);
//                }
//            }
//        }
//    }
//}
//
//void BattleScene::onTroopDeselected(TroopType type) {
//    auto tm = TroopManager::getInstance();
//    int current = tm->getTroopCount(type);
//    if (current > 0) {
//        tm->setTroopCount(type, current - 1);
//        updateLabels();
//    }
//}
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

    // ?TroopManagerе?()
    auto tm = TroopManager::getInstance();
    tm->setTroopCount(TroopType::BARBARIAN, 10);
    tm->setTroopCount(TroopType::ARCHER, 8);
    tm->setTroopCount(TroopType::GOBLIN, 6);
    tm->setTroopCount(TroopType::GIANT, 3);

    // levelId??
    _gameMap = CampaignMap::create(levelId);
    if (!_gameMap) return false;
    this->addChild(_gameMap, 0);

    setupUI();
    initBattleEntities();
    initTroopSelectionUI();
    initMapDeploymentListener();

    this->scheduleUpdate();

    return true;
}

void BattleScene::initBattleEntities()
{
    // ??CampaignMap?л?
    auto buildings = _gameMap->getBuildingsContainer()->getChildren();

    for (auto node : buildings)
    {
        Building* building = dynamic_cast<Building*>(node);
        if (building)
        {
            _enemyBuildings.pushBack(building);

            // ?TownHall
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

// ?
Animation* BattleScene::createTroopIdleAnimation(TroopType type) {
    Animation* animation = Animation::create();
    animation->setDelayPerUnit(0.5f); // ??0.5

    // ?????(е?)
    std::string basePath = "";
    switch (type) {
    case TroopType::BARBARIAN: basePath = "Troops/Barbarian"; break;
    case TroopType::ARCHER: basePath = "Troops/Archer"; break;
    case TroopType::GOBLIN: basePath = "Troops/Goblin"; break;
    case TroopType::GIANT: basePath = "Troops/Giant"; break;
    default: basePath = "Troops/Default"; break;
    }

    // ???(idle_1.png, idle_2.png)
    bool hasAnimationFrames = false;
    for (int i = 1; i <= 3; i++) {
        std::string framePath = basePath + "_idle_" + std::to_string(i) + ".png";
        if (FileUtils::getInstance()->isFileExist(framePath)) {
            animation->addSpriteFrameWithFile(framePath);
            hasAnimationFrames = true;
        }
    }

    // ?ж??????
    if (!hasAnimationFrames) {
        std::string staticPath = basePath + ".png";
        if (FileUtils::getInstance()->isFileExist(staticPath)) {
            animation->addSpriteFrameWithFile(staticPath);
        }
    }

    return animation;
}

// ?ж
void BattleScene::playSelectionAnimation(Sprite* sprite) {
    if (!sprite) return;

    // Ч
    sprite->setColor(Color3B(255, 255, 150)); // ??

    // ?
    auto scale1 = ScaleTo::create(0.5f, 1.1f);
    auto scale2 = ScaleTo::create(0.5f, 1.0f);
    auto breathe = RepeatForever::create(Sequence::create(scale1, scale2, nullptr));
    breathe->setTag(999); // ?????
    sprite->runAction(breathe);
}

void BattleScene::stopSelectionAnimation(Sprite* sprite) {
    if (!sprite) return;

    sprite->setColor(Color3B(255, 255, 255)); // ???
    sprite->stopActionByTag(999); // ??
    sprite->setScale(1.0f); // ???С
}

void BattleScene::initTroopSelectionUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    float startX = visibleSize.width * 0.2f;
    float gapX = visibleSize.width * 0.2f;
    float bottomY = origin.y + 120; // ??120
    std::vector<TroopType> types = { TroopType::BARBARIAN, TroopType::ARCHER, TroopType::GOBLIN, TroopType::GIANT };

    int index = 0;
    for (auto type : types) {
        float xPos = startX + (index * gapX);
        const auto& info = TROOP_DATA.at(type);

        // ?
        auto troopSprite = Sprite::create(info.imagePath);
        if (!troopSprite) {
            // ?????λ
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

        // ?
        float maxSide = 80.0f;
        Size spriteSize = troopSprite->getContentSize();
        if (spriteSize.width > maxSide || spriteSize.height > maxSide) {
            float scale = std::min(maxSide / spriteSize.width, maxSide / spriteSize.height);
            troopSprite->setScale(scale);
        }

        troopSprite->setTag(static_cast<int>(type));
        this->addChild(troopSprite, 10);

        // ??
        _troopSprites[type] = troopSprite;

        // ?
        auto listener = EventListenerTouchOneByOne::create();
        listener->setSwallowTouches(true);
        listener->onTouchBegan = [this, type](Touch* touch, Event* event) {
            auto sprite = _troopSprites[type];
            auto bounds = sprite->getBoundingBox();
            if (bounds.containsPoint(touch->getLocation())) {
                // ?п
                auto tm = TroopManager::getInstance();
                int count = tm->getTroopCount(type);

                if (count <= 0) {
                    // ?0??
                    sprite->setOpacity(100);
                    auto fadeIn = FadeTo::create(0.2f, 255);
                    sprite->runAction(fadeIn);
                    return true;
                }

                // ???
                this->updateTroopSelection(type);
                return true;
            }
            return false;
            };
        _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, troopSprite);

        // 
        auto nameLabel = Label::createWithTTF(info.name, "fonts/Marker Felt.ttf", 16);
        nameLabel->setPosition(Vec2(xPos, bottomY + 50));
        this->addChild(nameLabel, 10);

        // ????
        auto spaceLabel = Label::createWithTTF("Space: " + std::to_string(info.housingSpace), "fonts/Marker Felt.ttf", 12);
        spaceLabel->setPosition(Vec2(xPos, bottomY + 30));
        this->addChild(spaceLabel, 10);

        // ?
        auto tm = TroopManager::getInstance();
        int count = tm->getTroopCount(type);
        auto countLabel = Label::createWithTTF(std::to_string(count), "fonts/Marker Felt.ttf", 18);
        countLabel->setPosition(Vec2(xPos, bottomY - 50));
        countLabel->setColor(Color3B::YELLOW);
        this->addChild(countLabel, 10);

        // ??
        _troopCountLabels[type] = countLabel;

        index++;
    }
}

// ???
void BattleScene::updateTroopSelection(TroopType selectedType) {
    // ?????
    if (_selectedTroopSprite != nullptr) {
        stopSelectionAnimation(_selectedTroopSprite);
    }

    // ???
    _selectedTroopType = selectedType;
    _selectedTroopSprite = _troopSprites[selectedType];

    if (_selectedTroopSprite != nullptr) {
        playSelectionAnimation(_selectedTroopSprite);
        CCLOG("Selected troop type: %d", static_cast<int>(selectedType));
    }
}

// ??ü
void BattleScene::initMapDeploymentListener() {
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(false); // ?UI??

    listener->onTouchBegan = [this](Touch* touch, Event* event) {
        // δ?б?
        if (_selectedTroopType == TroopType::NONE) return false;

        // ?λ?UI()
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 touchPos = touch->getLocation();
        if (touchPos.y < 200) { // UI??200
            return false;
        }

        auto tm = TroopManager::getInstance();
        int remaining = tm->getTroopCount(_selectedTroopType);
        if (remaining <= 0) return false;

        // 
        deployTroop(touch->getLocation());
        _isLongPressing = true;

        // ??
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

// ??
void BattleScene::deployTroop(const Vec2& mapPos) {
    auto tm = TroopManager::getInstance();
    int count = tm->getTroopCount(_selectedTroopType);

    if (count <= 0) return;

    // Convert screen position to map node space
    Node* troopsLayer = _gameMap->getTroopsContainer();
    Vec2 localPos = troopsLayer->convertToNodeSpace(mapPos);

    // Create Troop
    auto troop = Unit::create(_selectedTroopType);
    troop->setPosition(localPos);
    troopsLayer->addChild(troop);

    CCLOG("Deployed troop at position: %.2f, %.2f", localPos.x, localPos.y);

    // Set Target with preferences
    troop->findNewTarget(_enemyBuildings);

    // ?
    tm->setTroopCount(_selectedTroopType, count - 1);
    updateLabels();

    // ?0???
    if ((count - 1) <= 0) {
        if (_selectedTroopSprite != nullptr) {
            stopSelectionAnimation(_selectedTroopSprite);
        }
        _selectedTroopType = TroopType::NONE;
        _selectedTroopSprite = nullptr;
    }
}

// ??
void BattleScene::onLongPressDeploy(float dt) {
    if (_isLongPressing && _lastDeployPos != Vec2::ZERO) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        if (_lastDeployPos.y > 200) { // ?UI
            deployTroop(_lastDeployPos);
        }
    }
}

// ?
void BattleScene::updateLabels() {
    auto tm = TroopManager::getInstance();

    for (auto& pair : _troopCountLabels) {
        TroopType type = pair.first;
        Label* label = pair.second;
        if (label) {
            int count = tm->getTroopCount(type);
            label->setString(std::to_string(count));

            // ?0??
            auto sprite = _troopSprites[type];
            if (sprite) {
                if (count <= 0) {
                    sprite->setOpacity(100); // ?
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

void BattleScene::onTroopDeselected(TroopType type) {
    auto tm = TroopManager::getInstance();
    int current = tm->getTroopCount(type);
    if (current > 0) {
        tm->setTroopCount(type, current - 1);
        updateLabels();
    }
}
