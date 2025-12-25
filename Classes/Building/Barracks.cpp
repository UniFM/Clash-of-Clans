#include "Barracks.h"
#include "Troops/Unit.h"
#include "ResourceManager.h"

Barracks* Barracks::create(int level)
{
    Barracks* barracks = new (std::nothrow) Barracks();
    if (barracks && barracks->init(level))
    {
        barracks->autorelease();
        return barracks;
    }
    CC_SAFE_DELETE(barracks);
    return nullptr;
}

bool Barracks::init(int level)
{
    if (!Building::init(BuildingType::BARRACKS, level))
        return false;

    _trainingSpeed = 1.0f + (level - 1) * 0.2f; // 等级越高训练越快
    this->scheduleUpdate();
    return true;
}

bool Barracks::trainUnit(UnitType unitType)
{
    if (!canTrainUnit(unitType))
        return false;

    // 检查资源
    auto resourceMgr = ResourceManager::getInstance();
    int goldCost = Unit::getGoldCost(unitType);
    int elixirCost = Unit::getElixirCost(unitType);
    int populationCost = Unit::getPopulationCost(unitType);

    if (!resourceMgr->spendGold(goldCost) ||
        !resourceMgr->spendElixir(elixirCost) ||
        !resourceMgr->addPopulation(populationCost))
    {
        return false;
    }

    // 添加到训练队列
    TrainingUnit training;
    training.type = unitType;
    training.remainingTime = Unit::getTrainingTime(unitType) / _trainingSpeed;
    _trainingQueue.push_back(training);

    return true;
}

bool Barracks::canTrainUnit(UnitType unitType) const
{
    auto resourceMgr = ResourceManager::getInstance();
    int goldCost = Unit::getGoldCost(unitType);
    int elixirCost = Unit::getElixirCost(unitType);
    int populationCost = Unit::getPopulationCost(unitType);

    return resourceMgr->getGold() >= goldCost &&
        resourceMgr->getElixir() >= elixirCost &&
        (resourceMgr->getPopulation() + populationCost) <= resourceMgr->getMaxPopulation();
}

void Barracks::update(float dt)
{
    if (isDestroyed() || _trainingQueue.empty())
        return;

    // 更新训练队列
    for (auto it = _trainingQueue.begin(); it != _trainingQueue.end();)
    {
        it->remainingTime -= dt;
        if (it->remainingTime <= 0)
        {
            // 训练完成，通知游戏管理器
            // GameManager::getInstance()->onUnitTrained(it->type);
            it = _trainingQueue.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

