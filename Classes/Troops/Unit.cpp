#include "Unit.h"
#include "Building/Building.h"
#include "AIController.h"

Unit::Unit()
    : _type(UnitType::ARCHER)
    , _hp(100)
    , _maxHP(100)
    , _attackDamage(10)
    , _attackRange(50.0f)
    , _moveSpeed(50.0f)
    , _attackInterval(1.0f)
    , _isMoving(false)
    , _isAttacking(false)
    , _currentTarget(nullptr)
    , _lastAttackTime(0.0f)
{
}

Unit::~Unit()
{
}

Unit* Unit::create(UnitType type)
{
    Unit* unit = new (std::nothrow) Unit();
    if (unit && unit->init(type))
    {
        unit->autorelease();
        return unit;
    }
    CC_SAFE_DELETE(unit);
    return nullptr;
}

bool Unit::init(UnitType type)
{
    _type = type;
    updateProperties();

    std::string imagePath = getImagePath();
    if (!Sprite::initWithFile(imagePath))
    {
        // 创建占位符
        if (!Sprite::init())
            return false;

        auto drawNode = DrawNode::create();
        Color4F color;
        switch (type)
        {
        case UnitType::ARCHER: color = Color4F::GREEN; break;
        case UnitType::BARBARIAN: color = Color4F::RED; break;
        case UnitType::BOMBER: color = Color4F::YELLOW; break;
        case UnitType::GIANT: color = Color4F::BLUE; break;
        }
        drawNode->drawSolidCircle(Vec2::ZERO, 15, 0, 20, color);
        this->addChild(drawNode);
        this->setContentSize(Size(30, 30));
    }

    this->scheduleUpdate();
    return true;
}

void Unit::updateProperties()
{
    switch (_type)
    {
    case UnitType::ARCHER:
        _maxHP = 50;
        _attackDamage = 15;
        _attackRange = 100.0f;
        _moveSpeed = 60.0f;
        _attackInterval = 1.0f;
        break;
    case UnitType::BARBARIAN:
        _maxHP = 100;
        _attackDamage = 20;
        _attackRange = 30.0f;
        _moveSpeed = 50.0f;
        _attackInterval = 1.2f;
        break;
    case UnitType::BOMBER:
        _maxHP = 30;
        _attackDamage = 100; // 对建筑高伤害
        _attackRange = 20.0f;
        _moveSpeed = 40.0f;
        _attackInterval = 2.0f;
        break;
    case UnitType::GIANT:
        _maxHP = 500;
        _attackDamage = 30;
        _attackRange = 40.0f;
        _moveSpeed = 30.0f;
        _attackInterval = 1.5f;
        break;
    }
    _hp = _maxHP;
}

std::string Unit::getImagePath() const
{
    std::string typeStr;
    switch (_type)
    {
    case UnitType::ARCHER: typeStr = "archer"; break;
    case UnitType::BARBARIAN: typeStr = "barbarian"; break;
    case UnitType::BOMBER: typeStr = "bomber"; break;
    case UnitType::GIANT: typeStr = "giant"; break;
    }
    return StringUtils::format("units/%s.png", typeStr.c_str());
}

void Unit::takeDamage(int damage)
{
    _hp -= damage;
    if (_hp < 0)
        _hp = 0;

    if (_hp > 0)
    {
        auto blink = Blink::create(0.2f, 2);
        this->runAction(blink);
    }
}

void Unit::moveTo(const Vec2& targetPos)
{
    _targetPosition = targetPos;
    _isMoving = true;
    _currentTarget = nullptr;
}

void Unit::stopMoving()
{
    _isMoving = false;
}

void Unit::update(float dt)
{
    if (isDead())
    {
        // 播放死亡动画
        this->removeFromParent();
        return;
    }

    _lastAttackTime += dt;

    // 寻找目标
    if (!_currentTarget || _currentTarget->isDestroyed())
    {
        _currentTarget = AIController::getInstance()->findNearestBuilding(this, _attackRange);
    }

    // 如果有目标且可以攻击
    if (_currentTarget && canAttack(_currentTarget))
    {
        _isMoving = false;
        _isAttacking = true;

        if (_lastAttackTime >= _attackInterval)
        {
            attackTarget(_currentTarget);
            _lastAttackTime = 0.0f;
        }
    }
    // 否则继续移动
    else if (_isMoving)
    {
        Vec2 currentPos = this->getPosition();
        Vec2 direction = _targetPosition - currentPos;
        float distance = direction.length();

        if (distance < 5.0f)
        {
            _isMoving = false;
        }
        else
        {
            direction.normalize();
            Vec2 newPos = currentPos + direction * _moveSpeed * dt;
            this->setPosition(newPos);
        }
    }
}

void Unit::attackTarget(Building* target)
{
    if (!target || target->isDestroyed())
        return;

    target->takeDamage(_attackDamage);

    // 播放攻击音效
    // AudioManager::getInstance()->playSoundEffect("sounds/attack.wav");
}

bool Unit::canAttack(Building* target) const
{
    if (!target || target->isDestroyed())
        return false;

    float distance = this->getPosition().distance(target->getPosition());
    return distance <= _attackRange;
}

// 静态方法实现
int Unit::getGoldCost(UnitType type)
{
    switch (type)
    {
    case UnitType::ARCHER: return 50;
    case UnitType::BARBARIAN: return 25;
    case UnitType::BOMBER: return 100;
    case UnitType::GIANT: return 200;
    }
    return 0;
}

int Unit::getElixirCost(UnitType type)
{
    switch (type)
    {
    case UnitType::ARCHER: return 50;
    case UnitType::BARBARIAN: return 25;
    case UnitType::BOMBER: return 100;
    case UnitType::GIANT: return 200;
    }
    return 0;
}

int Unit::getPopulationCost(UnitType type)
{
    switch (type)
    {
    case UnitType::ARCHER: return 1;
    case UnitType::BARBARIAN: return 1;
    case UnitType::BOMBER: return 2;
    case UnitType::GIANT: return 5;
    }
    return 0;
}

float Unit::getTrainingTime(UnitType type)
{
    switch (type)
    {
    case UnitType::ARCHER: return 10.0f;
    case UnitType::BARBARIAN: return 8.0f;
    case UnitType::BOMBER: return 15.0f;
    case UnitType::GIANT: return 30.0f;
    }
    return 0.0f;
}

