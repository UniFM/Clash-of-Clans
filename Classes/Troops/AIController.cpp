#include "AIController.h"

AIController* AIController::_instance = nullptr;

AIController* AIController::getInstance()
{
    if (!_instance)
    {
        _instance = new (std::nothrow) AIController();
    }
    return _instance;
}

void AIController::destroyInstance()
{
    CC_SAFE_DELETE(_instance);
}

AIController::AIController()
{
}

AIController::~AIController()
{
}

Building* AIController::findNearestBuilding(Unit* unit, float maxRange)
{
    if (!unit)
        return nullptr;

    Building* nearest = nullptr;
    float minDistance = maxRange;
    Vec2 unitPos = unit->getPosition();

    for (auto building : _buildings)
    {
        if (!building || building->isDestroyed())
            continue;

        float distance = unitPos.distance(building->getPosition());
        if (distance < minDistance)
        {
            minDistance = distance;
            nearest = building;
        }
    }

    return nearest;
}

Unit* AIController::findNearestEnemy(Building* building, float maxRange)
{
    if (!building)
        return nullptr;

    Unit* nearest = nullptr;
    float minDistance = maxRange;
    Vec2 buildingPos = building->getPosition();

    for (auto unit : _units)
    {
        if (!unit || unit->isDead())
            continue;

        float distance = buildingPos.distance(unit->getPosition());
        if (distance < minDistance)
        {
            minDistance = distance;
            nearest = unit;
        }
    }

    return nearest;
}

Building* AIController::findTargetByPriority(Unit* unit, float maxRange)
{
    if (!unit)
        return nullptr;

    Building* bestTarget = nullptr;
    float minDistance = maxRange;
    int highestPriority = 0;
    Vec2 unitPos = unit->getPosition();

    for (auto building : _buildings)
    {
        if (!building || building->isDestroyed())
            continue;

        float distance = unitPos.distance(building->getPosition());
        if (distance > maxRange)
            continue;

        int priority = building->getAttackPriority();

        // 优先选择优先级高的，如果优先级相同则选择近的
        if (priority > highestPriority ||
            (priority == highestPriority && distance < minDistance))
        {
            highestPriority = priority;
            minDistance = distance;
            bestTarget = building;
        }
    }

    return bestTarget;
}

void AIController::registerUnit(Unit* unit)
{
    if (unit && _units.find(unit) == _units.end())
    {
        _units.pushBack(unit);
    }
}

void AIController::unregisterUnit(Unit* unit)
{
    _units.eraseObject(unit);
}

void AIController::registerBuilding(Building* building)
{
    if (building && _buildings.find(building) == _buildings.end())
    {
        _buildings.pushBack(building);
    }
}

void AIController::unregisterBuilding(Building* building)
{
    _buildings.eraseObject(building);
}

void AIController::clearAll()
{
    _units.clear();
    _buildings.clear();
}

