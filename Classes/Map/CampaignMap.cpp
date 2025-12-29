#include "CampaignMap.h"
#include "buildings/BuildingsData.h"

USING_NS_CC;

CampaignMap* CampaignMap::create(int levelId)
{
    CampaignMap* map = new (std::nothrow) CampaignMap();
    if (map && map->init(levelId))
    {
        map->autorelease();
        return map;
    }
    CC_SAFE_DELETE(map);
    return nullptr;
}

bool CampaignMap::init(int levelId)
{
    // 为战役地图设置地图参数（可自定义）
    _mapWidth = 3020.0f;
    _mapHeight = 2420.0f;
    _grassRectWidth = 2400.0f;
    _grassRectHeight = 1800.0f;
    _grassOffsetX = 310.0f;
    _grassOffsetY = 194.0f;
    _gridCols = 36;
    _gridRows = 36;

    // 战斗地图背景图
    if (!BaseMap::init("Map/map1.png"))
    {
        return false;
    }

    setupLevel(levelId);
    return true;
}

void CampaignMap::setupLevel(int levelId)
{
    // 如果复用地图实例，清除已有的建筑（本战斗场景下实际是创建新实例）
    _buildingsContainer->removeAllChildren();

    // 【修改】移除在CampaignMap中创建建筑的逻辑
    // 战斗场景的建筑现在完全由BattleScene::initHardcodedBuildings()管理
    // 这样避免了重复创建大本营的问题
    
    CCLOG("CampaignMap level %d setup complete - buildings managed by BattleScene", levelId);

    // 原来的建筑创建代码已注释掉，由BattleScene统一管理
    /*
    if (levelId == 0) // 测试关卡 / 多人模式占位关卡
    {
        auto townHall = TownHall::create(1);
        townHall->setPosition(gridToWorld(18, 18));
        addBuilding(townHall);
    }
    else if (levelId == 1)
    {
        // 1号关卡：简单布局
        auto townHall = TownHall::create(1);
        townHall->setPosition(gridToWorld(18, 22));
        addBuilding(townHall);
    }
    else if (levelId == 2)
    {
        // 2号关卡：更多防御建筑
        auto townHall = TownHall::create(2);
        townHall->setPosition(gridToWorld(18, 18));
        addBuilding(townHall);
    }
    */
}