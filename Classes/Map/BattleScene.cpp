#include "BattleScene.h"
#include "HomeVillageMap.h"
#include "CampaignMap.h"
#include "Control/AIController.h"
#include "Building/ResourceManager.h"
#include "Building/TownHall.h"
#include "Building/DefenseTower.h"
#include "Troops/Archer.h"
#include "Troops/Barbarian.h"
#include "Troops/Bomber.h"
#include "Troops/Giant.h"
#include "Control/AudioManager.h"

Scene* BattleScene::createScene(int levelId)
{
    return BattleScene::create(levelId);
}

BattleScene* BattleScene::create(int levelId)
{
    BattleScene *pRet = new(std::nothrow) BattleScene();
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
    _deployedUnits.clear();

    // 根据levelId初始化地图
    // 目前，即使是0级（测试模式）也使用CampaignMap，以与HomeVillage实现清晰的分离
    _gameMap = CampaignMap::create(levelId);
    if (!_gameMap) return false;
    this->addChild(_gameMap, 0);
    
    // 从地图初始化实体
    initBattleEntities();

    setupUI();

    auto audioMgr = AudioManager::getInstance();
    audioMgr->playBackgroundMusic("sounds/battle.mp3", true);

    this->scheduleUpdate();

    return true;
}

void BattleScene::initBattleEntities()
{
    // 从地图（由CampaignMap填充）中获取建筑物
    auto buildings = _gameMap->getBuildingsContainer()->getChildren();
    
    for (auto node : buildings)
    {
        Building* building = dynamic_cast<Building*>(node);
        if (building)
        {
            AIController::getInstance()->registerBuilding(building);
            _enemyBuildings.pushBack(building);
            
            // 检查它是否是TownHall
            if (dynamic_cast<TownHall*>(building))
            {
                _enemyTownHall = building;
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

void BattleScene::deployUnit(UnitType unitType, const Vec2& worldPos)
{
    if (_battleEnded)
        return;

    Unit* unit = nullptr;
    switch (unitType)
    {
    case UnitType::ARCHER:
        unit = Archer::create();
        break;
    case UnitType::BARBARIAN:
        unit = Barbarian::create();
        break;
    case UnitType::BOMBER:
        unit = Bomber::create();
        break;
    case UnitType::GIANT:
        unit = Giant::create();
        break;
    }

    if (unit)
    {
        // update!!! ：将单位添加到MAP，而非SCENE，这样它们就能随地图移动
        // 如果添加到_gameMap，只要_gameMap只是一个节点，它们就会受地图缩放/位置的影响。
        
        unit->setPosition(worldPos); 
        
        _gameMap->getBuildingsContainer()->addChild(unit, 100); // z=100 ,在建筑物的顶部
        
        AIController::getInstance()->registerUnit(unit);
        _deployedUnits.pushBack(unit);
    }
}

void BattleScene::startBattle()
{
    _battleStarted = true;
}

void BattleScene::endBattle(bool isVictory)
{
    _battleEnded = true;
    _battleStarted = false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    std::string resultText = isVictory ? "Victory!" : "Defeat!";
    auto label = Label::createWithTTF(resultText, "fonts/Marker Felt.ttf", 48);
    label->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    this->addChild(label, 200);
}

void BattleScene::checkBattleResult()
{
    if (_enemyTownHall && _enemyTownHall->isDestroyed())
    {
        endBattle(true);
        return;
    }

    bool allUnitsDead = true;
    for (auto unit : _deployedUnits)
    {
        if (unit && !unit->isDead())
        {
            allUnitsDead = false;
            break;
        }
    }

    if (allUnitsDead && _deployedUnits.size() > 0)
    {
        endBattle(false);
    }
}

void BattleScene::update(float dt)
{
    if (!_battleStarted || _battleEnded)
        return;

    for (auto it = _deployedUnits.begin(); it != _deployedUnits.end();)
    {
        if ((*it)->isDead())
        {
            AIController::getInstance()->unregisterUnit(*it);
            (*it)->removeFromParent();
            it = _deployedUnits.erase(it);
        }
        else
        {
            ++it;
        }
    }

    checkBattleResult();
}
