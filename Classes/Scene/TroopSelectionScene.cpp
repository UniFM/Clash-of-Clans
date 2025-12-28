/*************************************************************
* @file     : TroopSelcetionScene.cpp
* @function ：选兵场景实现
* @author   : 俞筱雨
* @note     ：选兵界面
**************************************************************/

#include "Troops/TroopManager.h"
#include "TroopSelectionScene.h"

USING_NS_CC;

TroopSelectionScene* TroopSelectionScene::create() {
    TroopSelectionScene* scene = new (std::nothrow) TroopSelectionScene();
    if (scene && scene->init()) {
        scene->autorelease();
        return scene;
    }
    CC_SAFE_DELETE(scene);
    return nullptr;
}

bool TroopSelectionScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // Background (Darker background to distinguish from main base)
    auto background = LayerColor::create(Color4B(50, 50, 50, 255));
    this->addChild(background, 0);

    // 1. Top: Capacity Label
    //capacityLabel = Label::createWithSystemFont("Capacity: 0 / 0", "Arial", 24);
    //capacityLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - 50));
    //this->addChild(capacityLabel, 1);
    capacityLabel = Label::createWithTTF("Capacity: 0 / 0", "fonts/Marker Felt.ttf", 24);
    capacityLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - 50));
    this->addChild(capacityLabel, 1);

    // 2. Middle: Troop Selection UI
    float startX = origin.x + visibleSize.width * 0.2f;
    float gapX = visibleSize.width * 0.2f;
    float centerY = origin.y + visibleSize.height * 0.5f;

    std::vector<TroopType> types = { TroopType::BARBARIAN, TroopType::ARCHER, TroopType::GOBLIN, TroopType::GIANT };

    int index = 0;
    for (auto type : types) {
        float xPos = startX + (index * gapX);
        const auto& info = TROOP_DATA.at(type);

        // A. Troop Image (Color Block)
        // [新增] 使用 Sprite 显示人物图片
        std::string imagePath = info.imagePath; // 从 TROOP_DATA 获取路径
        auto troopSprite = Sprite::create(imagePath);

        if (troopSprite) {
            // 设置位置
            troopSprite->setPosition(Vec2(xPos, centerY + 40)); // 调整位置以匹配之前的布局

            // 缩放控制：如果图片太大，限制在 80x80 以内
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
            // 如果图片加载失败，回退到原来的颜色块逻辑
            CCLOG("Failed to load troop image: %s", imagePath.c_str());
            Color3B color = Color3B::GRAY; // 默认灰色
            if (type == TroopType::BARBARIAN) color = Color3B::RED;
            else if (type == TroopType::ARCHER) color = Color3B::GREEN;
            else if (type == TroopType::GOBLIN) color = Color3B::BLUE;
            else if (type == TroopType::GIANT) color = Color3B::YELLOW;

            auto colorBlock = LayerColor::create(Color4B(color), 80, 80);
            colorBlock->setPosition(Vec2(xPos - 40, centerY));
            this->addChild(colorBlock, 1);
        }

        // B. Troop Name (Below Image)
        auto nameBtn = ui::Button::create();
        nameBtn->setTitleText(info.name);
        nameBtn->setTitleFontName("fonts/Marker Felt.ttf");
        nameBtn->setTitleFontSize(18);
        nameBtn->setPosition(Vec2(xPos, centerY - 60));
        nameBtn->addClickEventListener([this, type](Ref* sender) {
            this->onTroopSelected(type);
            });
        this->addChild(nameBtn, 1);

        // C. Cost/Space Info
        auto spaceLabel = Label::createWithTTF("Space: " + std::to_string(info.housingSpace), "fonts/Marker Felt.ttf", 14);
        spaceLabel->setPosition(Vec2(xPos, centerY - 90));
        this->addChild(spaceLabel, 1);

        // D. Count Control Area (Top of Image)
        float controlY = centerY + 110;

        // Minus Button
        auto minusBtn = ui::Button::create();
        minusBtn->setTitleText("-");
        minusBtn->setTitleFontName("fonts/Marker Felt.ttf");
        minusBtn->setTitleFontSize(30);
        minusBtn->setTitleColor(Color3B::WHITE);
        minusBtn->setPosition(Vec2(xPos - 40, controlY));
        minusBtn->addClickEventListener([this, type](Ref* sender) {
            this->onTroopDeselected(type);
            });
        this->addChild(minusBtn, 1);

        // Count Label
        auto countLabel = Label::createWithTTF("0", "fonts/Marker Felt.ttf", 24);
        countLabel->setPosition(Vec2(xPos, controlY));
        this->addChild(countLabel, 1);
        this->countLabels[type] = countLabel;

        // Plus Button
        auto plusBtn = ui::Button::create();
        plusBtn->setTitleText("+");
        plusBtn->setTitleFontName("fonts/Marker Felt.ttf");
        plusBtn->setTitleFontSize(30);
        plusBtn->setTitleColor(Color3B::WHITE);
        plusBtn->setPosition(Vec2(xPos + 40, controlY));
        plusBtn->addClickEventListener([this, type](Ref* sender) {
            this->onTroopSelected(type);
            });
        this->addChild(plusBtn, 1);

        index++;
    }

    // 3. Bottom Left: Back Button - 修改按钮字体
    auto backBtn = ui::Button::create();
    backBtn->setTitleText("Back");
    backBtn->setTitleFontName("fonts/Marker Felt.ttf");
    backBtn->setTitleFontSize(24);
    backBtn->setPosition(Vec2(origin.x + 80, origin.y + 50));
    backBtn->addClickEventListener(CC_CALLBACK_1(TroopSelectionScene::onBackCallback, this));
    this->addChild(backBtn, 1);

    // 4. Reset Button - 修改按钮字体
    auto resetBtn = ui::Button::create();
    resetBtn->setTitleText("Reset");
    resetBtn->setTitleFontName("fonts/Marker Felt.ttf");
    resetBtn->setTitleFontSize(24);
    resetBtn->setPosition(Vec2(origin.x + visibleSize.width - 80, origin.y + 50));
    resetBtn->addClickEventListener(CC_CALLBACK_1(TroopSelectionScene::onResetCallback, this));
    this->addChild(resetBtn, 1);

    updateLabels();

    CCLOG("TroopSelectionScene initialized successfully");
    return true;
}

void TroopSelectionScene::onBackCallback(Ref* pSender) {
    //Director::getInstance()->popScene();
    CCLOG("TroopSelection scene closing, returning to village...");

    // 使用GameManager返回村庄场景
    auto gameManager = GameManager::getInstance();
    gameManager->gotoVillageScene();
}

void TroopSelectionScene::onTroopSelected(TroopType type) {
    auto tm = TroopManager::getInstance();
    if (tm->canAddTroop(type)) {
        tm->setTroopCount(type, tm->getTroopCount(type) + 1);
        updateLabels();
    }
    else {
        // Optional: Animation or sound for "Full"
    }
}

void TroopSelectionScene::onTroopDeselected(TroopType type) {
    auto tm = TroopManager::getInstance();
    int current = tm->getTroopCount(type);
    if (current > 0) {
        tm->setTroopCount(type, current - 1);
        updateLabels();
    }
}

void TroopSelectionScene::onResetCallback(Ref* pSender) {
    TroopManager::getInstance()->clearTroops();
    updateLabels();
}

void TroopSelectionScene::updateLabels() {
    auto tm = TroopManager::getInstance();

    // Update Capacity
    std::string capText = "Capacity: " + std::to_string(tm->getCurrentHousingSpace()) +
        " / " + std::to_string(tm->getMaxHousingSpace());
    this->capacityLabel->setString(capText);

    // Update Counts
    for (auto& pair : this->countLabels) {
        int count = tm->getTroopCount(pair.first);
        pair.second->setString(std::to_string(count));
    }
}