/*************************************************************
* @file     : TroopSelcetionScene.cpp
* @function ：选兵场景实现
* @author   : 俞筱雨
* @note     ：选兵界面
**************************************************************/

#include "Troops/TroopManager.h"
#include "TroopSelectionScene.h"

// 使用cocos2d命名空间
USING_NS_CC;

// 创建选兵场景实例
TroopSelectionScene* TroopSelectionScene::create() {
    TroopSelectionScene* scene = new (std::nothrow) TroopSelectionScene();
    if (scene && scene->init()) {
        scene->autorelease();
        return scene;
    }
    CC_SAFE_DELETE(scene);
    return nullptr;
}

// 初始化选兵场景
bool TroopSelectionScene::init() {
    if (!Scene::init()) {
        return false;
    }

    // 获取屏幕可视区域大小和原点坐标
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 背景层（深色背景，区分主基地场景）
    auto background = LayerColor::create(Color4B(50, 50, 50, 255));
    this->addChild(background, 0);

    // 1. 顶部：容量显示标签
    capacityLabel = Label::createWithTTF("Capacity: 0 / 0", "fonts/Marker Felt.ttf", 24);
    capacityLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - 50));
    this->addChild(capacityLabel, 1);

    // 2. 中间：兵种选择UI区域
    // 起始X坐标（屏幕20%宽度位置）
    float startX = origin.x + visibleSize.width * 0.2f;
    // 兵种之间的X间距（屏幕20%宽度）
    float gapX = visibleSize.width * 0.2f;
    // Y坐标居中（屏幕50%高度位置）
    float centerY = origin.y + visibleSize.height * 0.5f;

    // 要显示的兵种类型列表
    std::vector<TroopType> types = { TroopType::BARBARIAN, TroopType::ARCHER, TroopType::GOBLIN, TroopType::GIANT };

    int index = 0;
    for (auto type : types) {
        // 计算当前兵种的X坐标
        float xPos = startX + (index * gapX);
        // 获取当前兵种的配置信息
        const auto& info = TROOP_DATA.at(type);

        // A. 兵种图片（图片加载失败则显示颜色块）
        // 从兵种配置中获取图片路径
        std::string imagePath = info.imagePath;
        auto troopSprite = Sprite::create(imagePath);

        if (troopSprite) {
            // 设置精灵位置（匹配原有布局）
            troopSprite->setPosition(Vec2(xPos, centerY + 40));

            // 缩放控制：限制图片最大尺寸为80x80
            float maxSide = 80.0f;
            float scale = 1.0f;
            Size spriteSize = troopSprite->getContentSize();
            if (spriteSize.width > maxSide || spriteSize.height > maxSide) {
                scale = std::min(maxSide / spriteSize.width, maxSide / spriteSize.height);
            }
            troopSprite->setScale(scale);

            this->addChild(troopSprite, 1);
        }
        else {
            // 图片加载失败，打印日志并显示颜色块
            CCLOG("加载兵种图片失败: %s", imagePath.c_str());
            Color3B color = Color3B::GRAY; // 默认灰色
            if (type == TroopType::BARBARIAN) color = Color3B::RED;    // 野蛮人-红色
            else if (type == TroopType::ARCHER) color = Color3B::GREEN;// 弓箭手-绿色
            else if (type == TroopType::GOBLIN) color = Color3B::BLUE; // 哥布林-蓝色
            else if (type == TroopType::GIANT) color = Color3B::YELLOW;// 巨人-黄色

            auto colorBlock = LayerColor::create(Color4B(color), 80, 80);
            colorBlock->setPosition(Vec2(xPos - 40, centerY));
            this->addChild(colorBlock, 1);
        }

        // B. 兵种名称按钮（图片下方）
        auto nameBtn = ui::Button::create();
        nameBtn->setTitleText(info.name);
        nameBtn->setTitleFontName("fonts/Marker Felt.ttf");
        nameBtn->setTitleFontSize(18);
        nameBtn->setPosition(Vec2(xPos, centerY - 60));
        // 点击事件：选中该兵种
        nameBtn->addClickEventListener([this, type](Ref* sender) {
            this->onTroopSelected(type);
            });
        this->addChild(nameBtn, 1);

        // C. 兵种占用空间信息
        auto spaceLabel = Label::createWithTTF("Space: " + std::to_string(info.housingSpace), "fonts/Marker Felt.ttf", 14);
        spaceLabel->setPosition(Vec2(xPos, centerY - 90));
        this->addChild(spaceLabel, 1);

        // D. 数量控制区域（图片上方）
        float controlY = centerY + 110;

        // 减号按钮
        auto minusBtn = ui::Button::create();
        minusBtn->setTitleText("-");
        minusBtn->setTitleFontName("fonts/Marker Felt.ttf");
        minusBtn->setTitleFontSize(30);
        minusBtn->setTitleColor(Color3B::WHITE);
        minusBtn->setPosition(Vec2(xPos - 40, controlY));
        // 点击事件：减少该兵种数量
        minusBtn->addClickEventListener([this, type](Ref* sender) {
            this->onTroopDeselected(type);
            });
        this->addChild(minusBtn, 1);

        // 数量显示标签
        auto countLabel = Label::createWithTTF("0", "fonts/Marker Felt.ttf", 24);
        countLabel->setPosition(Vec2(xPos, controlY));
        this->addChild(countLabel, 1);
        this->countLabels[type] = countLabel;

        // 加号按钮
        auto plusBtn = ui::Button::create();
        plusBtn->setTitleText("+");
        plusBtn->setTitleFontName("fonts/Marker Felt.ttf");
        plusBtn->setTitleFontSize(30);
        plusBtn->setTitleColor(Color3B::WHITE);
        plusBtn->setPosition(Vec2(xPos + 40, controlY));
        // 点击事件：增加该兵种数量
        plusBtn->addClickEventListener([this, type](Ref* sender) {
            this->onTroopSelected(type);
            });
        this->addChild(plusBtn, 1);

        index++;
    }

    // 3. 左下角：返回按钮
    auto backBtn = ui::Button::create();
    backBtn->setTitleText("Back");
    backBtn->setTitleFontName("fonts/Marker Felt.ttf");
    backBtn->setTitleFontSize(24);
    backBtn->setPosition(Vec2(origin.x + 80, origin.y + 50));
    backBtn->addClickEventListener(CC_CALLBACK_1(TroopSelectionScene::onBackCallback, this));
    this->addChild(backBtn, 1);

    // 4. 右下角：重置按钮
    auto resetBtn = ui::Button::create();
    resetBtn->setTitleText("Reset");
    resetBtn->setTitleFontName("fonts/Marker Felt.ttf");
    resetBtn->setTitleFontSize(24);
    resetBtn->setPosition(Vec2(origin.x + visibleSize.width - 80, origin.y + 50));
    resetBtn->addClickEventListener(CC_CALLBACK_1(TroopSelectionScene::onResetCallback, this));
    this->addChild(resetBtn, 1);

    // 更新所有标签显示
    updateLabels();

    CCLOG("选兵场景初始化成功");
    return true;
}

// 返回按钮点击回调
void TroopSelectionScene::onBackCallback(Ref* pSender) {
    CCLOG("选兵场景关闭，返回村庄场景...");

    // 使用游戏管理器切换到村庄场景
    auto gameManager = GameManager::getInstance();
    gameManager->gotoVillageScene();
}

// 选中兵种（增加数量）
void TroopSelectionScene::onTroopSelected(TroopType type) {
    auto tm = TroopManager::getInstance();
    // 检查是否可添加该兵种（容量足够）
    if (tm->canAddTroop(type)) {
        tm->setTroopCount(type, tm->getTroopCount(type) + 1);
        updateLabels();
    }
    else {
        // 可选：添加容量不足的提示动画/音效
    }
}

// 取消选中兵种（减少数量）
void TroopSelectionScene::onTroopDeselected(TroopType type) {
    auto tm = TroopManager::getInstance();
    int current = tm->getTroopCount(type);
    // 数量大于0时才减少
    if (current > 0) {
        tm->setTroopCount(type, current - 1);
        updateLabels();
    }
}

// 重置按钮点击回调
void TroopSelectionScene::onResetCallback(Ref* pSender) {
    // 清空所有已选兵种
    TroopManager::getInstance()->clearTroops();
    updateLabels();
}

// 更新所有标签显示（容量和兵种数量）
void TroopSelectionScene::updateLabels() {
    auto tm = TroopManager::getInstance();

    // 更新容量显示文本
    std::string capText = "Capacity: " + std::to_string(tm->getCurrentHousingSpace()) +
        " / " + std::to_string(tm->getMaxHousingSpace());
    this->capacityLabel->setString(capText);

    // 更新各兵种数量显示
    for (auto& pair : this->countLabels) {
        int count = tm->getTroopCount(pair.first);
        pair.second->setString(std::to_string(count));
    }
}