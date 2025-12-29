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
    
    // 从背景图片中的资源条区域获取资源数据并显示文本
    // 由于资源条已经在背景中，我们只需要在对应位置添加数字文本
    
    // 金币数量文本 (对应背景中金币图标位置)
    auto goldLabel = Label::createWithTTF("1000", "fonts/Marker Felt.ttf", 30);
    goldLabel->setPosition(Vec2(visibleSize.width * 0.15f, visibleSize.height * 0.065f));
    goldLabel->setColor(Color3B::WHITE);
    goldLabel->setTag(1001); // 用于后续更新
    resourceBarNode->addChild(goldLabel);
    
    // 圣水数量文本 (对应背景中圣水图标位置)
    auto elixirLabel = Label::createWithTTF("6789", "fonts/Marker Felt.ttf", 30);
    elixirLabel->setPosition(Vec2(visibleSize.width * 0.45f, visibleSize.height * 0.065f));
    elixirLabel->setColor(Color3B::WHITE);
    elixirLabel->setTag(1002); // 用于后续更新
    resourceBarNode->addChild(elixirLabel);
    
    // 宝石数量文本 (对应背景中宝石图标位置)
    auto gemLabel = Label::createWithTTF("50", "fonts/Marker Felt.ttf", 30);
    gemLabel->setPosition(Vec2(visibleSize.width * 0.76f, visibleSize.height * 0.065f));
    gemLabel->setColor(Color3B::WHITE);
    gemLabel->setTag(1003); // 用于后续更新
    resourceBarNode->addChild(gemLabel);
    
    CCLOG("Resource bar initialized with text overlays on background");
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
        //case BuildingType::CANNON:
        //    buildingSpritePath = ResPath::CANNONLEVEL1;
        //    break;
        case BuildingType::TOWN_HALL:
            buildingSpritePath = ResPath::TOWNHALLLEVEL1;
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
    
    // 建筑名称标签
    auto nameLabel = Label::createWithTTF("buildingData->name", "fonts/Marker Felt.ttf", 14);
    nameLabel->setPosition(Vec2(0, -10));
    nameLabel->setColor(Color3B::WHITE);
    container->addChild(nameLabel);
    
    // 价格标签
    std::string priceText = "std::to_string(stats->goldCost) + ";
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
        // 可以在这里显示提示信息
        return;
    }
    
    CCLOG("Building selected for purchase: %d", static_cast<int>(buildingType));
    
    // 使用正确的方法切换到村庄场景并开始建筑放置
    auto gameManager = GameManager::getInstance();
    gameManager->gotoVillageSceneWithBuildingPlacement(buildingType);
}

std::vector<BuildingType> ShopScene::getBuildingsByCategory(ShopCategory category)
{
    std::vector<BuildingType> buildings;
    
    switch (category) {
        case ShopCategory::ARMY:
            buildings = {/*BuildingType::ARMY_CAMP*/BuildingType::GOLD_MINE, BuildingType::ELIXIR_COLLECTOR };
            break;
            
        case ShopCategory::RESOURCES:
            buildings = {BuildingType::GOLD_MINE, BuildingType::ELIXIR_COLLECTOR};
            break;
            
        case ShopCategory::DEFENSE:
            // 添加加农炮作为测试用例
            buildings = {/*BuildingType::ARCHER_TOWER, BuildingType::CANNON*/BuildingType::GOLD_MINE, BuildingType::ELIXIR_COLLECTOR };
            break;
            
        case ShopCategory::TRAPS:
            // 暂时为空，可以后续添加陷阱建筑
            break;
    }
    
    return buildings;
}

void ShopScene::updateResourceDisplay()
{
    // 更新资源显示 (这里使用模拟数据，实际应该从游戏数据获取)
    auto goldLabel = dynamic_cast<Label*>(resourceBarNode->getChildByTag(1001));
    if (goldLabel) {
        goldLabel->setString("12345"); // 只显示数字，因为图标在背景中
    }
    
    auto elixirLabel = dynamic_cast<Label*>(resourceBarNode->getChildByTag(1002));
    if (elixirLabel) {
        elixirLabel->setString("6789"); // 只显示数字，因为图标在背景中
    }
    
    auto gemLabel = dynamic_cast<Label*>(resourceBarNode->getChildByTag(1003));
    if (gemLabel) {
        gemLabel->setString("89"); // 只显示数字，因为图标在背景中
    }
}

bool ShopScene::canAffordBuilding(BuildingType buildingType, int level)
{
    //const BuildingLevelStats* stats = BuildingConfig::getStats(buildingType, level);
    //if (!stats) return false;
    
    // 这里应该检查实际的资源数量，现在假设都买得起
    // 实际实现应该从游戏数据管理器获取当前资源
    return true; // 简化实现
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

//if (!shopBackground) {
//    // 如果没有资源，用颜色块代替
//    shopBackground = Scale9Sprite::create();
//    shopBackground->setColor(Color3B(50, 50, 50));
//}

//shopBackground->setContentSize(Size(visibleSize.width * 0.9f, visibleSize.height * 0.8f));
//shopBackground->setPosition(visibleSize.width / 2, visibleSize.height / 2);

//// 添加提示文字
//auto hintLabel = Label::createWithTTF("Click anywhere to continue", "fonts/arial.ttf", 24);
//if (hintLabel) {
//    hintLabel->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height * 0.2f + origin.y));
//    hintLabel->setColor(Color3B::WHITE);
//    this->addChild(hintLabel);

//    // 添加闪烁效果
//    auto fadeIn = FadeIn::create(1.0f);
//    auto fadeOut = FadeOut::create(1.0f);
//    auto blink = Sequence::create(fadeOut, fadeIn, nullptr);
//    auto repeat = RepeatForever::create(blink);
//    hintLabel->runAction(repeat);
//}

//// 添加触摸监听器
//auto touchListener = EventListenerTouchOneByOne::create();
//touchListener->onTouchBegan = [this](Touch* touch, Event* event) {
//    this->gotoLogin(0.0f);
//    return true;
//    };
//this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);

//return true;
