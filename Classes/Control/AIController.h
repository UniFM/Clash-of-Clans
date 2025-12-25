#pragma once
#ifndef __AI_CONTROLLER_H__
#define __AI_CONTROLLER_H__

#include "cocos2d.h"
#include "Building/Building.h"
#include "Troops/Unit.h"

USING_NS_CC;

/**
 * AI控制器 - 处理战斗AI逻辑
 */
class AIController
{
public:
    static AIController* getInstance();
    static void destroyInstance();

    // 寻找最近的敌人建筑（用于兵种）
    Building* findNearestBuilding(Unit* unit, float maxRange = 1000.0f);

    // 寻找最近的敌人单位（用于防御塔）
    Unit* findNearestEnemy(Building* building, float maxRange = 1000.0f);

    // 根据优先级寻找目标
    Building* findTargetByPriority(Unit* unit, float maxRange = 1000.0f);

    // 注册/注销单位
    void registerUnit(Unit* unit);
    void unregisterUnit(Unit* unit);

    // 注册/注销建筑
    void registerBuilding(Building* building);
    void unregisterBuilding(Building* building);

    // 清空所有注册
    void clearAll();

private:
    AIController();
    ~AIController();

    Vector<Unit*> _units;
    Vector<Building*> _buildings;

    static AIController* _instance;
};

#endif // __AI_CONTROLLER_H__

