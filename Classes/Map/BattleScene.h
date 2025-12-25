#pragma once
#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include "cocos2d.h"
#include "BaseMap.h" // 从HomeVillageMap.h修改
#include "Troops/Unit.h"
#include "Building/Building.h"

USING_NS_CC;

/**
 * 战斗场景
 */
class BattleScene : public Scene
{
public:
    static Scene* createScene(int levelId = 0);
    static BattleScene* create(int levelId);
    
    virtual bool init(int levelId);

    // 部署单元
    void deployUnit(UnitType unitType, const Vec2& worldPos);

    // 检查胜负
    void checkBattleResult();

    // 开始/结束战斗
    void startBattle();
    void endBattle(bool isVictory);

private:
    BaseMap* _gameMap; // 已更改类型
    Vector<Unit*> _deployedUnits;
    Vector<Building*> _enemyBuildings;

    Building* _enemyTownHall;
    bool _battleStarted;
    bool _battleEnded;
    int _levelId;

    void setupUI();
    void update(float dt) override;
    
    // 0级（测试/玩家对战存根）的遗留设置
    void setupEnemyBuildings(); 
    
    // 关卡的新设置
    void initBattleEntities();
};

#endif // __BATTLE_SCENE_H__
