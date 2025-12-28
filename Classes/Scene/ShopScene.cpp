/*************************************************************
* @file     : ShopScene.cpp
* @function ：商店场景实现
* @author   : 叶芷含 
* @note     ：实现部落冲突风格的商店界面
**************************************************************/

#include "ShopScene.h"
#include "Map/HomeVillageMap.h"
#include "Control/GameManager.h"
#include "Map/SceneMap.h"

USING_NS_CC;

ShopScene* ShopScene::create(){
    ShopScene* scene = new (std::nothrow) ShopScene();
    if (scene && scene->init()) {
        scene->autorelease();
        return scene;
    }
    CC_SAFE_DELETE(scene);
    return nullptr;
}

bool ShopScene::init()
{
    if (!Scene::init()) {
        return false;
    }
    
    // 设置默认分类
    currentCategory = ShopCategory::ARMY;
    
    // 初始化各个UI组件
    initBackground();
    initCategoryTabs(); 
    initBuildingScrollView();
    initResourceBar();
    initCloseButton();
    
    // 刷新建筑列表
    refreshBuildingList();
    
    // 更新资源显示
    updateResourceDisplay();
    
    CCLOG("ShopScene initialized successfully");
    return true;
}

void ShopScene::initBackground()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    // 创建背景容器
    backgroundNode = Node::create();
    this->addChild(backgroundNode);
    
    auto shopBackground = Sprite::create(ResPath::SHOPBACKGROUND);
    if (!shopBackground) {
        // 如果没有资源，用颜色块代替
        shopBackground = Sprite::create();
        auto colorLayer = LayerColor::create(Color4B(50, 50, 50, 255), visibleSize.width, visibleSize.height);
        this->addChild(colorLayer, -1);
    } else {
        shopBackground->setPosition(Vec2(visibleSize.width / 2 + origin.x,
            visibleSize.height / 2 + origin.y));
        backgroundNode->addChild(shopBackground);
    }

    CCLOG("Shop background initialized");
}

void ShopScene::initCategoryTabs()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    // 创建分类标签容器
    categoryTabsNode = Node::create();
    this->addChild(categoryTabsNode, 2);
    
    // 分类信息
    struct CategoryInfo {
        ShopCategory category;
        std::string name;
        Vec2 position;
    };
    
    std::vector<CategoryInfo> categories = {
        {ShopCategory::ARMY, "ARMY", Vec2(visibleSize.width * 0.15f, visibleSize.height * 0.78f)},
        {ShopCategory::RESOURCES, "RESOURCES", Vec2(visibleSize.width * 0.39f, visibleSize.height * 0.78f)}, 
        {ShopCategory::DEFENSE, "DEFENSE", Vec2(visibleSize.width * 0.63f, visibleSize.height * 0.78f)},
        {ShopCategory::TRAPS, "TRAPS", Vec2(visibleSize.width * 0.87f, visibleSize.height * 0.78f)}
    };
    
    // 创建分类按钮
    for (size_t i = 0; i < categories.size(); ++i) {
        const auto& categoryInfo = categories[i];
        
        // 创建按钮背景精灵
        auto buttonSprite = Sprite::create();
        
        // 根据是否是默认选中的分类设置初始背景
        if (categoryInfo.category == currentCategory) {
            buttonSprite = Sprite::create(ResPath::SHOPCATEGORYGREENBUTTON); // 选中状态
        } else {
            buttonSprite = Sprite::create(ResPath::SHOPCATEGORYGREYGREENBUTTON); // 未选中状态
        }
        
        if (!buttonSprite) {
            CCLOG("Warning: Failed to create button sprite for category %s", categoryInfo.name.c_str());
            continue;
        }
        
        buttonSprite->setPosition(categoryInfo.position);
        categoryTabsNode->addChild(buttonSprite);
        
        // 创建按钮文本标签
        auto titleLabel = Label::createWithTTF(categoryInfo.name, "fonts/Marker Felt.ttf", 24);
        titleLabel->setPosition(categoryInfo.position);
        titleLabel->setColor(Color3B::WHITE);
        categoryTabsNode->addChild(titleLabel);
        
        // 创建触摸监听器
        auto touchListener = EventListenerTouchOneByOne::create();
        touchListener->setSwallowTouches(true);
        
        touchListener->onTouchBegan = [buttonSprite, categoryInfo](Touch* touch, Event* event) -> bool {
            Vec2 touchPos = touch->getLocation();
            Rect buttonRect = buttonSprite->getBoundingBox();
            
            // 转换到父节点坐标系
            Vec2 nodePos = buttonSprite->getParent()->convertToNodeSpace(touchPos);
            
            return buttonRect.containsPoint(nodePos);
        };
        
        touchListener->onTouchEnded = [this, categoryInfo](Touch* touch, Event* event) {
            this->onCategorySelected(categoryInfo.category);
        };
        
        // 将监听器添加到事件分发器
        _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, buttonSprite);
        
        // 存储按钮精灵和标签，用于后续更新状态
        buttonSprite->setTag(static_cast<int>(categoryInfo.category));
        titleLabel->setTag(static_cast<int>(categoryInfo.category) + 1000); // 偏移1000避免冲突
    }
    
    CCLOG("Category tabs initialized with %zu categories, default selected: ARMY", categories.size());
}

void ShopScene::initBuildingScrollView()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    // 创建滚动视图
    buildingScrollView = ScrollView::create();  
    buildingScrollView->setContentSize(Size(visibleSize.width * 0.85f, 300));
    buildingScrollView->setPosition(Vec2(visibleSize.width * 0.075f, visibleSize.height * 0.2f));
    buildingScrollView->setDirection(ScrollView::Direction::HORIZONTAL);
    buildingScrollView->setBounceEnabled(true);
    
    backgroundNode->addChild(buildingScrollView, 2);
    
    CCLOG("Building scroll view initialized");
}

void ShopScene::initResourceBar()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    // 创建资源显示容器
    resourceBarNode = Node::create();
    this->addChild(resourceBarNode, 3); // 确保在背景之上
    
    // 从GameManager获取实际资源数据并显示
    auto gameManager = GameManager::getInstance();
    
    // 金币数量文本 (对应背景中金币图标位置)
    int goldAmount = gameManager->getResource(ResourceType::GOLD);
    auto goldLabel = Label::createWithTTF(std::to_string(goldAmount), "fonts/Marker Felt.ttf", 30);
    goldLabel->setPosition(Vec2(visibleSize.width * 0.15f, visibleSize.height * 0.065f));
    goldLabel->setColor(Color3B::WHITE);
    goldLabel->setTag(1001); // 用于后续更新
    resourceBarNode->addChild(goldLabel);
    
    // 圣水数量文本 (对应后台中圣水图标位置)
    int elixirAmount = gameManager->getResource(ResourceType::ELIXIR);
    auto elixirLabel = Label::createWithTTF(std::to_string(elixirAmount), "fonts/Marker Felt.ttf", 30);
    elixirLabel->setPosition(Vec2(visibleSize.width * 0.45f, visibleSize.height * 0.065f));
    elixirLabel->setColor(Color3B::WHITE);
    elixirLabel->setTag(1002); // 用于后续更新
    resourceBarNode->addChild(elixirLabel);
    
    // 宝石数量文本 (对应后台中宝石图标位置)
    int gemAmount = gameManager->getResource(ResourceType::GEMS);
    auto gemLabel = Label::createWithTTF(std::to_string(gemAmount), "fonts/Marker Felt.ttf", 30);
    gemLabel->setPosition(Vec2(visibleSize.width * 0.76f, visibleSize.height * 0.065f));
    gemLabel->setColor(Color3B::WHITE);
    gemLabel->setTag(1003); // 用于后续更新
    resourceBarNode->addChild(gemLabel);

    CCLOG("Resource bar initialized with actual resource data from GameManager");
}

void ShopScene::initCloseButton()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    // 创建关闭按钮 - 使用MenuItemImage
    auto closeBtn = MenuItemImage::create(
        ResPath::CLOSEBUTTON, 
        ResPath::CLOSEBUTTONPRESSED,
        CC_CALLBACK_1(ShopScene::onCloseButtonClicked, this)
    );
    
    closeBtn->setPosition(Vec2(visibleSize.width * 0.05f, visibleSize.height * 0.95f));
    
    // 创建菜单
    auto menu = Menu::create(closeBtn, nullptr);
    menu->setPosition(Vec2::ZERO);
    
    backgroundNode->addChild(menu, 10);
    
    CCLOG("Close button initialized");
}

void ShopScene::onCategorySelected(ShopCategory category)
{
    if (currentCategory == category) return;
    
    currentCategory = category;
    updateCategoryButtons();
    refreshBuildingList();
    
    CCLOG("Category selected: %d", static_cast<int>(category));
}

void ShopScene::updateCategoryButtons()
{
    if (!categoryTabsNode) return;
    
    // 更新所有分类按钮的状态
    for (int i = 0; i < 4; ++i) {
        ShopCategory category = static_cast<ShopCategory>(i);
        
        // 找到对应的按钮精灵
        auto buttonSprite = dynamic_cast<Sprite*>(categoryTabsNode->getChildByTag(static_cast<int>(category)));
        if (!buttonSprite) continue;
        
        // 根据当前选中的分类更新按钮背景
        if (category == currentCategory) {
            // 选中状态 - 绿色按钮
            auto newTexture = Director::getInstance()->getTextureCache()->addImage(ResPath::SHOPCATEGORYGREENBUTTON);
            if (newTexture) {
                buttonSprite->setTexture(newTexture);
            }
        } else {
            // 未选中状态 - 灰绿色按钮
            auto newTexture = Director::getInstance()->getTextureCache()->addImage(ResPath::SHOPCATEGORYGREYGREENBUTTON);
            if (newTexture) {
                buttonSprite->setTexture(newTexture);
            }
        }
    }
    
    CCLOG("Category buttons updated, current category: %d", static_cast<int>(currentCategory));
}

void ShopScene::refreshBuildingList()
{
    // 清除旧的建筑项目
    for (auto item : buildingItems) {
        item->removeFromParent();
    }
    buildingItems.clear();
    
    // 获取当前分类的建筑
    auto buildingTypes = getBuildingsByCategory(currentCategory);
    
    if (buildingTypes.empty()) {
        CCLOG("No buildings found for category %d", static_cast<int>(currentCategory));
        return;
    }
    
    // 计算滚动区域大小
    float itemWidth = 226.0f;
    float itemSpacing = 20.0f;
    float totalWidth = buildingTypes.size() * (itemWidth + itemSpacing);
    
    buildingScrollView->setInnerContainerSize(Size(totalWidth, buildingScrollView->getContentSize().height));
    
    // 创建建筑项目
    for (size_t i = 0; i < buildingTypes.size(); ++i) {
        auto buildingItem = createBuildingItem(buildingTypes[i]);
        buildingItem->setPosition(Vec2(i * (itemWidth + itemSpacing) + itemWidth/2, 
                                     buildingScrollView->getContentSize().height / 2));
        
        buildingScrollView->addChild(buildingItem);
        buildingItems.push_back(buildingItem);
    }
    
    CCLOG("Building list refreshed with %zu buildings", buildingTypes.size());
}

Node* ShopScene::createBuildingItem(BuildingType buildingType)
{
    auto container = Node::create();
            
    // 检查是否可以建造 (这里简化为总是可以建造，实际应该检查大本营等级)
    bool canBuild = true; // 实际实现时应该检查大本营等级等条件
    
    // 选择卡片背景
    std::string cardBackgroundPath = canBuild ? 
        ResPath::AVAILABLEBUILDINGCARDBACKGROUND : 
        ResPath::UNAVAILABLEBUILDINGCARDBACKGROUND;
    
    // 创建卡片背景
    auto cardBg = Sprite::create(cardBackgroundPath);
    if (!cardBg) {
        // 如果图片加载失败，创建默认背景
        cardBg = Sprite::create();
        auto colorLayer = LayerColor::create(
            canBuild ? Color4B(100, 200, 100, 255) : Color4B(150, 150, 150, 255), 
            140, 180
        );
        cardBg->addChild(colorLayer);
        cardBg->setContentSize(Size(140, 180));
    }
    container->addChild(cardBg);
    
    // 根据建筑类型添加建筑图标
    std::string buildingSpritePath;
    switch (buildingType) {
        case BuildingType::CANNON:
            buildingSpritePath = ResPath::CANNONLEVEL1;
            break;
        case BuildingType::GOLD_MINE:
            buildingSpritePath = ResPath::GOLDMINELEVEL1;
            break;
        case BuildingType::BARRACKS:
            buildingSpritePath = ResPath::BARRACKSLEVEL1;
            break;
        case BuildingType::ELIXIR_COLLECTOR:
            buildingSpritePath = ResPath::ELIXIRCOLLECTORLEVEL1;
            break;
        case BuildingType::ARCHER_TOWER:
            buildingSpritePath = ResPath::ARCHERTOWERLEVEL1;
            break;
        case BuildingType::GOLD_STORAGE:
            buildingSpritePath = ResPath::GOLDSTORAGELEVEL1; // 暂时使用金矿图标
            break;
        case BuildingType::ELIXIR_STORAGE:
            buildingSpritePath = ResPath::ELIXIRSTORAGELEVEL1; // 暂时使用圣水收集器图标
            break;
        // 其他建筑类型可以在这里添加
        default:
            buildingSpritePath = ResPath::CANNONLEVEL1; // 使用配置中的路径
            break;
    }
    
    // 添加建筑图标
    auto buildingSprite = Sprite::create(buildingSpritePath);
    if (!buildingSprite) {
        // 如果建筑图标加载失败，创建占位符
        buildingSprite = Sprite::create();
        auto placeholder = LayerColor::create(Color4B(100, 150, 200, 255), 80, 80);
        buildingSprite->addChild(placeholder);
        buildingSprite->setContentSize(Size(80, 80));
        CCLOG("Warning: Failed to load building sprite: %s", buildingSpritePath.c_str());
    }
    
    // 设置建筑图标位置和大小
    buildingSprite->setPosition(Vec2(0, 30)); // 在卡片上方
    buildingSprite->setScale(0.8f); // 适当缩放
    container->addChild(buildingSprite);
    
    // 建筑名称标签 - 根据建筑类型显示实际名称
    std::string buildingName;
    switch (buildingType) {
        case BuildingType::CANNON:
            buildingName = "加农炮";
            break;
        case BuildingType::GOLD_MINE:
            buildingName = "金矿";
            break;
        case BuildingType::BARRACKS:
            buildingName = "兵营";
            break;
        case BuildingType::ELIXIR_COLLECTOR:
            buildingName = "圣水收集器";
            break;
        case BuildingType::ARCHER_TOWER:
            buildingName = "弓箭塔";
            break;
        case BuildingType::TOWN_HALL:
            buildingName = "大本营";
            break;
        case BuildingType::GOLD_STORAGE:
            buildingName = "储金罐";
            break;
        case BuildingType::ELIXIR_STORAGE:
            buildingName = "圣水瓶";
            break;
        default:
            buildingName = "未知建筑";
            break;
    }
    
    auto nameLabel = Label::createWithTTF(buildingName, "fonts/Marker Felt.ttf", 14);
    nameLabel->setPosition(Vec2(0, -10));
    nameLabel->setColor(Color3B::WHITE);
    container->addChild(nameLabel);
    
    // 价格标签 - 根据建筑类型显示实际价格
    int buildingCost = 0;
    switch (buildingType) {
        case BuildingType::CANNON:
            buildingCost = 1000;
            break;
        case BuildingType::GOLD_MINE:
            buildingCost = 150;
            break;
        case BuildingType::BARRACKS:
            buildingCost = 100;
            break;
        case BuildingType::ELIXIR_COLLECTOR:
            buildingCost = 150;
            break;
        case BuildingType::ARCHER_TOWER:
            buildingCost = 500;
            break;
        case BuildingType::TOWN_HALL:
            buildingCost = 0; // 通常不在商店购买
            break;
        case BuildingType::GOLD_STORAGE:
            buildingCost = 300;
            break;
        case BuildingType::ELIXIR_STORAGE:
            buildingCost = 300;
            break;
        default:
            buildingCost = 100;
            break;
    }
    
    std::string priceText = std::to_string(buildingCost);
    auto priceLabel = Label::createWithTTF(priceText, "fonts/Marker Felt.ttf", 12);
    priceLabel->setPosition(Vec2(0, -35));
    priceLabel->setColor(canBuild ? Color3B::YELLOW : Color3B::RED);
    container->addChild(priceLabel);
    
    // 添加触摸监听器到整个卡片
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    
    touchListener->onTouchBegan = [cardBg](Touch* touch, Event* event) -> bool {
        Vec2 touchPos = touch->getLocation();
        Vec2 localPos = cardBg->getParent()->convertToNodeSpace(touchPos);
        Rect cardRect = cardBg->getBoundingBox();
        return cardRect.containsPoint(localPos);
    };
    
    touchListener->onTouchEnded = [this, buildingType, canBuild](Touch* touch, Event* event) {
        if (canBuild) {
            this->onBuildingSelected(buildingType);
        } else {
            CCLOG("Cannot build this building at current town hall level");
        }
    };
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, container);
    
    return container;
}

void ShopScene::onBuildingSelected(BuildingType buildingType)
{
    // 检查是否买得起
    if (!canAffordBuilding(buildingType)) {
        CCLOG("Cannot afford building type %d", static_cast<int>(buildingType));
        
        // 显示资源不足提示
        showInsufficientResourcesMessage();
        return;
    }
    
    // 计算建筑成本并扣除资源
    int buildingCost = 0;
    switch (buildingType) {
        case BuildingType::CANNON:
            buildingCost = 1000;
            break;
        case BuildingType::GOLD_MINE:
            buildingCost = 150;
            break;
        case BuildingType::BARRACKS:
            buildingCost = 100;
            break;
        case BuildingType::ELIXIR_COLLECTOR:
            buildingCost = 150;
            break;
        case BuildingType::ARCHER_TOWER:
            buildingCost = 500;
            break;
        case BuildingType::GOLD_STORAGE:
            buildingCost = 300;
            break;
        case BuildingType::ELIXIR_STORAGE:
            buildingCost = 300;
            break;
        default:
            buildingCost = 100;
            break;
    }
    
    // 扣除资源
    auto gameManager = GameManager::getInstance();
    if (gameManager->spendResource(ResourceType::GOLD, buildingCost)) {
        CCLOG("Building purchased! Type: %d, Cost: %d gold", static_cast<int>(buildingType), buildingCost);
        
        // 更新资源显示
        updateResourceDisplay();
        
        // 使用正确的方法切换到村庄场景并开始建筑放置
        gameManager->gotoVillageSceneWithBuildingPlacement(buildingType);
    } else {
        // 理论上不应该到这里，因为前面已经检查过了
        CCLOG("Failed to spend resources!");
        showInsufficientResourcesMessage();
    }
}

std::vector<BuildingType> ShopScene::getBuildingsByCategory(ShopCategory category)
{
    std::vector<BuildingType> buildings;
    
    switch (category) {
        case ShopCategory::ARMY:
            // 军队类建筑：兵营
            buildings = {BuildingType::BARRACKS};
            break;
            
        case ShopCategory::RESOURCES:
            // 资源类建筑：金矿、圣水收集器、储金罐、圣水瓶
            buildings = {BuildingType::GOLD_MINE, BuildingType::ELIXIR_COLLECTOR, 
                        BuildingType::GOLD_STORAGE, BuildingType::ELIXIR_STORAGE};
            break;
            
        case ShopCategory::DEFENSE:
            // 防御类建筑：弓箭塔、加农炮
            buildings = {BuildingType::ARCHER_TOWER, BuildingType::CANNON};
            break;
            
        case ShopCategory::TRAPS:
            // 陷阱类建筑暂时为空，可以后续添加
            break;
    }
    
    return buildings;
}

void ShopScene::updateResourceDisplay()
{
    // 从GameManager获取实际资源数据
    auto gameManager = GameManager::getInstance();
    
    auto goldLabel = dynamic_cast<Label*>(resourceBarNode->getChildByTag(1001));
    if (goldLabel) {
        int goldAmount = gameManager->getResource(ResourceType::GOLD);
        goldLabel->setString(std::to_string(goldAmount));
    }
    
    auto elixirLabel = dynamic_cast<Label*>(resourceBarNode->getChildByTag(1002));
    if (elixirLabel) {
        int elixirAmount = gameManager->getResource(ResourceType::ELIXIR);
        elixirLabel->setString(std::to_string(elixirAmount));
    }
    
    auto gemLabel = dynamic_cast<Label*>(resourceBarNode->getChildByTag(1003));
    if (gemLabel) {
        int gemAmount = gameManager->getResource(ResourceType::GEMS);
        gemLabel->setString(std::to_string(gemAmount));
    }
}

bool ShopScene::canAffordBuilding(BuildingType buildingType, int level)
{
    // 获取建筑成本
    int buildingCost = 0;
    switch (buildingType) {
        case BuildingType::CANNON:
            buildingCost = 1000;
            break;
        case BuildingType::GOLD_MINE:
            buildingCost = 150;
            break;
        case BuildingType::BARRACKS:
            buildingCost = 100;
            break;
        case BuildingType::ELIXIR_COLLECTOR:
            buildingCost = 150;
            break;
        case BuildingType::ARCHER_TOWER:
            buildingCost = 500;
            break;
        case BuildingType::GOLD_STORAGE:
            buildingCost = 300;
            break;
        case BuildingType::ELIXIR_STORAGE:
            buildingCost = 300;
            break;
        default:
            buildingCost = 100;
            break;
    }
    
    // 从GameManager获取当前金币数量进行实际检查
    auto gameManager = GameManager::getInstance();
    int currentGold = gameManager->getResource(ResourceType::GOLD);
    return currentGold >= buildingCost;
}

void ShopScene::onCloseButtonClicked(Ref* sender)
{
    CCLOG("Shop scene closing, returning to village...");

    // 使用GameManager返回村庄场景
    auto gameManager = GameManager::getInstance();
    gameManager->gotoVillageScene();
}

void ShopScene::onBackKeyPressed()
{
    onCloseButtonClicked(nullptr);
}

void ShopScene::showInsufficientResourcesMessage()
{
    // 创建提示标签，显示在屏幕中央
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    auto messageLabel = Label::createWithTTF("Insufficient resources! Cannot proceed!", "fonts/Marker Felt.ttf", 36);
    messageLabel->setPosition(Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.5f));
    messageLabel->setColor(Color3B::RED);
    messageLabel->setLocalZOrder(1000); // 确保显示在最上层
    
    this->addChild(messageLabel);
    
    // 创建红色闪烁效果
    auto fadeOut = FadeOut::create(0.5f);
    auto fadeIn = FadeIn::create(0.5f);
    auto blink = Sequence::create(fadeOut, fadeIn, nullptr);
    auto repeat = Repeat::create(blink, 3); // 闪烁3次
    
    // 最后移除消息
    auto delay = DelayTime::create(0.5f);
    auto remove = RemoveSelf::create();
    
    auto sequence = Sequence::create(repeat, delay, remove, nullptr);
    messageLabel->runAction(sequence);
    
    CCLOG("显示资源不足提示: Insufficient resources! Cannot proceed!");
}
