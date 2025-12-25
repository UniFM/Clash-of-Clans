#include "DefenseTower.h"
#include "Control/AIController.h"

DefenseTower* DefenseTower::create(BuildingType type, int level)
{
    DefenseTower* tower = new (std::nothrow) DefenseTower();
    if (tower && tower->init(type, level))
    {
        tower->autorelease();
        return tower;
    }
    CC_SAFE_DELETE(tower);
    return nullptr;
}

bool DefenseTower::init(BuildingType type, int level)
{
    if (!Building::init(type, level))
        return false;

    _lastAttackTime = 0.0f;
    _currentTarget = nullptr;
    updateAttackProperties();

    this->scheduleUpdate();
    return true;
}

void DefenseTower::update(float dt)
{
    if (isDestroyed())
        return;

    _lastAttackTime += dt;

    // 寻找目标
    if (!_currentTarget || _currentTarget->isDead() || !canAttack(_currentTarget))
    {
        _currentTarget = AIController::getInstance()->findNearestEnemy(this, _attackRange);
    }

    // 攻击目标
    if (_currentTarget && _lastAttackTime >= _attackInterval)
    {
        attackTarget(_currentTarget);
        _lastAttackTime = 0.0f;
    }
}

void DefenseTower::attackTarget(Unit* target)
{
    if (!target || target->isDead())
        return;

    target->takeDamage(_attackDamage);

    // 播放攻击动画/音效
    // ...
}

bool DefenseTower::canAttack(Unit* target) const
{
    if (!target || target->isDead())
        return false;

    float distance = this->getPosition().distance(target->getPosition());
    return distance <= _attackRange;
}

void DefenseTower::updateAttackProperties()
{
    // 根据类型和等级设置攻击属性
    switch (_type)
    {
    case BuildingType::ARCHER_TOWER:
        _attackRange = 200.0f + (_level - 1) * 50.0f;
        _attackDamage = 20 + (_level - 1) * 10;
        _attackInterval = 1.0f - (_level - 1) * 0.1f;
        break;
    case BuildingType::CANNON:
        _attackRange = 150.0f + (_level - 1) * 40.0f;
        _attackDamage = 50 + (_level - 1) * 25;
        _attackInterval = 1.5f - (_level - 1) * 0.15f;
        break;
    default:
        _attackRange = 100.0f;
        _attackDamage = 10;
        _attackInterval = 1.0f;
        break;
    }
}

