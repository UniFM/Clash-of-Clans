#pragma once
#ifndef __BARRACKS_H__
#define __BARRACKS_H__

#include "Building.h"
#include "Troops/Unit.h"

class Barracks : public Building
{
public:
    static Barracks* create(int level = 1);
    virtual bool init(int level);

    // 训练兵种
    bool trainUnit(UnitType unitType);
    bool canTrainUnit(UnitType unitType) const;

    // 获取训练队列
    int getTrainingQueueSize() const { return _trainingQueue.size(); }

    // 更新训练
    void update(float dt) override;

private:
    struct TrainingUnit
    {
        UnitType type;
        float remainingTime;
    };

    std::vector<TrainingUnit> _trainingQueue;
    float _trainingSpeed; // 训练速度倍数
};

#endif // __BARRACKS_H__

