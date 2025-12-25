#pragma once
#ifndef __UNIT_H__
#define __UNIT_H__

#include "cocos2d.h"

USING_NS_CC;

/**
 * 兵种类型枚举
 */
enum class UnitType
{
    ARCHER,     // 弓箭手
    BARBARIAN,  // 野蛮人
    BOMBER,     // 炸弹人
    GIANT       // 巨人
};

class Building;

/**
 * 兵种基类
 */
class Unit : public Sprite
{
public:
    static Unit* create(UnitType type);
    virtual bool init(UnitType type);

    // 属性
    UnitType getUnitType() const { return _type; }
    int getHP() const { return _hp; }
    int getMaxHP() const { return _maxHP; }
    int getAttackDamage() const { return _attackDamage; }
    float getAttackRange() const { return _attackRange; }
    float getMoveSpeed() const { return _moveSpeed; }
    float getAttackInterval() const { return _attackInterval; }

    // 状态
    bool isDead() const { return _hp <= 0; }
    bool isMoving() const { return _isMoving; }
    bool isAttacking() const { return _isAttacking; }

    // 战斗
    void takeDamage(int damage);
    void attackTarget(Building* target);
    bool canAttack(Building* target) const;

    // 移动
    void moveTo(const Vec2& targetPos);
    void stopMoving();

    // 更新
    void update(float dt) override;

    // 静态方法：获取兵种属性
    static int getGoldCost(UnitType type);
    static int getElixirCost(UnitType type);
    static int getPopulationCost(UnitType type);
    static float getTrainingTime(UnitType type);

protected:
    Unit();
    virtual ~Unit();

    UnitType _type;
    int _hp;
    int _maxHP;
    int _attackDamage;
    float _attackRange;
    float _moveSpeed;
    float _attackInterval;

    bool _isMoving;
    bool _isAttacking;
    Vec2 _targetPosition;
    Building* _currentTarget;
    float _lastAttackTime;

    void updateProperties();
    std::string getImagePath() const;
};

#endif // __UNIT_H__

