#include "GameScene.h"
#include "TroopManager.h"
#include "TroopSelectionScene.h" // Will create this next
#include "ui/CocosGUI.h"

USING_NS_CC;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. Background (Greenish for grass)
    auto background = LayerColor::create(Color4B(34, 139, 34, 255));
    this->addChild(background, 0);

    // 2. Barracks (A simple Gray box in the center)
    barracksNode = Node::create();
    barracksNode->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(barracksNode, 1);

    auto barracksVisual = LayerColor::create(Color4B(128, 128, 128, 255), 100, 100);
    barracksVisual->setIgnoreAnchorPointForPosition(false);
    barracksVisual->setAnchorPoint(Vec2(0.5, 0.5));
    barracksNode->addChild(barracksVisual);

    auto label = Label::createWithSystemFont("Barracks\nLvl " + std::to_string(TroopManager::getInstance()->getBarracksLevel()), "Arial", 16);
    label->setPosition(Vec2::ZERO);
    barracksNode->addChild(label);

    // 3. Container for troops (Next to barracks)
    troopsContainer = Node::create();
    troopsContainer->setPosition(barracksNode->getPosition() + Vec2(80, -50)); // Offset a bit
    this->addChild(troopsContainer, 2);

    // 4. "Select Troops" Button
    auto selectButton = ui::Button::create();
    selectButton->setTitleText("Select Troops");
    selectButton->setTitleFontSize(24);
    selectButton->setTitleColor(Color3B::WHITE);
    // Create a simple colored background for the button using a 1x1 white texture if needed, 
    // but for simplicity we assume standard button behavior or text only if images missing.
    // Let's make it a Label masquerading as a button for simplicity if no assets, 
    // but ui::Button is better. I'll just rely on text.
    selectButton->setPosition(Vec2(origin.x + visibleSize.width - 100, origin.y + 50));
    selectButton->addClickEventListener(CC_CALLBACK_1(GameScene::onSelectTroopsCallback, this));
    this->addChild(selectButton, 10);

    updateTroopDisplay();

    return true;
}

void GameScene::onEnterTransitionDidFinish() {
    Scene::onEnterTransitionDidFinish();
    // Refresh display when returning from selection scene
    updateTroopDisplay();
}

void GameScene::onSelectTroopsCallback(Ref* pSender) {
    auto scene = TroopSelectionScene::createScene();
    Director::getInstance()->pushScene(TransitionFade::create(0.5, scene));
}

void GameScene::updateTroopDisplay() {
    troopsContainer->removeAllChildren();

    auto tm = TroopManager::getInstance();
    
    // Simple grid layout for troops
    int col = 0;
    int row = 0;
    float padding = 20.0f;
    int maxCols = 5;

    // Iterate through all troop types
    std::vector<TroopType> types = {TroopType::BARBARIAN, TroopType::ARCHER, TroopType::BOMBER, TroopType::GIANT};

    for (auto type : types) {
        int count = tm->getTroopCount(type);
        const auto& info = TROOP_DATA.at(type);
        
        // Parse hex color (simplified)
        Color4B color;
        if (type == TroopType::BARBARIAN) color = Color4B::RED;
        else if (type == TroopType::ARCHER) color = Color4B::GREEN;
        else if (type == TroopType::BOMBER) color = Color4B::BLUE;
        else if (type == TroopType::GIANT) color = Color4B::YELLOW;
        else color = Color4B::WHITE;

        for (int i = 0; i < count; ++i) {
            // Represent each troop as a small colored dot
            auto troopDot = LayerColor::create(color, 10, 10);
            troopDot->setIgnoreAnchorPointForPosition(false);
            
            float x = col * padding;
            float y = row * -padding;

            troopDot->setPosition(Vec2(x, y));
            troopsContainer->addChild(troopDot);

            col++;
            if (col >= maxCols) {
                col = 0;
                row++;
            }
        }
    }
}
