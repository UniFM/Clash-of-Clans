#pragma once
#ifndef __DEFENSE_TOWER_H__
#define __DEFENSE_TOWER_H__

#include "Building.h"
#include "Troops/Unit.h"

class DefenseTower : public Building
{
public:
    static DefenseTower* create(BuildingType type, int level = 1);
    virtual bool init(BuildingType type, int level);

    // 攻击
    void update(float dt) override;
    void attackTarget(Unit* target);
    bool canAttack(Unit* target) const;

    // 属性
    float getAttackRange() const { return _attackRange; }
    int getAttackDamage() const { return _attackDamage; }
    float getAttackInterval() const { return _attackInterval; }

    virtual int getAttackPriority() const override { return 5; } // 防御塔优先级较高

protected:
    float _attackRange;
    int _attackDamage;
    float _attackInterval;
    float _lastAttackTime;
    Unit* _currentTarget;

    void updateAttackProperties();
};

#endif // __DEFENSE_TOWER_H__

