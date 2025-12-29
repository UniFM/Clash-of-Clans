/*************************************************************
* @file     : Unit.cpp
* @function ：游戏单位基类头文件 - 定义兵种核心属性与接口
* @author   : 俞筱雨
* @note     : 1. 定义兵种类型枚举及基础属性结构体；
*             2. 封装单位创建、移动、攻击、寻路核心接口；
*             3. 管理单位状态（移动/攻击/死亡）及目标选择逻辑。
**************************************************************/

#include "Unit.h"
#include "TroopDefinitions.h"
#include "Control/GameManager.h"
#include <cmath>

// 创建单位实例（内存分配+初始化+自动释放管理）
Unit* Unit::create(UnitType type)
{
    Unit* unit = new (std::nothrow) Unit();
    if (unit && unit->init(type))
    {
        unit->autorelease(); // 交给cocos内存管理
        return unit;
    }
    CC_SAFE_DELETE(unit); // 初始化失败则释放内存
    return nullptr;
}

// 初始化单位（类型赋值+属性加载+资源加载+状态初始化）
bool Unit::init(UnitType type)
{
    _type = type;
    updateProperties(); // 加载单位基础属性

    // 加载单位纹理图片（优先从配置表获取，失败则用默认图）
    std::string imagePath = "";
    auto it = TROOP_DATA.find(type);
    if (it != TROOP_DATA.end()) {
        imagePath = it->second.imagePath;
    }
    else {
        imagePath = "Troops/Barbarian.png"; // 兜底默认图
    }

    // 尝试加载纹理，失败则绘制占位图形
    if (!Sprite::initWithFile(imagePath))
    {
        if (!Sprite::init()) return false;

        // 绘制不同颜色的圆形占位符（区分不同兵种）
        auto drawNode = DrawNode::create();
        Color4F color;
        switch (type)
        {
        case UnitType::ARCHER: color = Color4F::GREEN; break;
        case UnitType::BARBARIAN: color = Color4F::RED; break;
        case UnitType::GOBLIN: color = Color4F::YELLOW; break;
        case UnitType::GIANT: color = Color4F::BLUE; break;
        default: color = Color4F::WHITE; break;
        }
        drawNode->drawSolidCircle(Vec2::ZERO, 15, 0, 20, color);
        this->addChild(drawNode);
        this->setContentSize(Size(30, 30));
    }

    // 初始化单位核心状态
    _target = nullptr;       // 初始无攻击目标
    _isMoving = false;       // 初始静止
    _attackTimer = 0.0f;     // 攻击冷却计时器清零
    _isDead = false;         // 初始存活状态

    runIdleAnimation();      // 播放闲置动画
    this->scheduleUpdate();  // 开启帧更新

    return true;
}

// 更新单位属性（从配置表/硬编码加载对应兵种的基础属性）
void Unit::updateProperties()
{
    // 从配置表加载名称和人口占用，其他属性暂时硬编码
    _stats.name = TROOP_DATA.at(_type).name;
    _stats.housingSpace = TROOP_DATA.at(_type).housingSpace;

    // 按兵种类型设置战斗属性（生命值/伤害/移速等）
    switch (_type)
    {
    case UnitType::ARCHER:
        _maxHealth = 50;
        _stats.damage = 15;
        _stats.attackRange = 100.0f;
        _stats.moveSpeed = 60.0f;
        _stats.attackSpeed = 1.0f;
        break;
    case UnitType::BARBARIAN:
        _maxHealth = 100;
        _stats.damage = 20;
        _stats.attackRange = 30.0f;
        _stats.moveSpeed = 50.0f;
        _stats.attackSpeed = 1.2f;
        break;
    case UnitType::GOBLIN:
        _maxHealth = 30;
        _stats.damage = 100; // 哥布林对资源建筑高伤害
        _stats.attackRange = 20.0f;
        _stats.moveSpeed = 80.0f; // 移速快
        _stats.attackSpeed = 1.0f;
        break;
    case UnitType::GIANT:
        _maxHealth = 500;    // 巨人血量厚
        _stats.damage = 30;
        _stats.attackRange = 40.0f;
        _stats.moveSpeed = 20.0f; // 移速慢
        _stats.attackSpeed = 1.5f;
        break;
    default:
        // 默认属性（未知兵种）
        _maxHealth = 100;
        _stats.damage = 10;
        _stats.attackRange = 20.0f;
        _stats.moveSpeed = 50.0f;
        _stats.attackSpeed = 1.0f;
        break;
    }
    _currentHealth = _maxHealth; // 初始化当前生命值为最大值
}

// 播放闲置动画（加载3帧闲置帧动画并循环播放）
void Unit::runIdleAnimation()
{
    // 从配置表获取纹理路径，截取基础路径（去掉后缀）
    std::string fullPath = TROOP_DATA.at(_type).imagePath;
    size_t lastDot = fullPath.find_last_of(".");
    if (lastDot == std::string::npos) return;

    std::string basePath = fullPath.substr(0, lastDot);

    // 加载3帧闲置动画帧
    Vector<SpriteFrame*> animFrames;
    bool framesExist = false;

    for (int i = 1; i <= 3; i++) {
        std::string framePath = basePath + "_idle_" + std::to_string(i) + ".png";
        if (FileUtils::getInstance()->isFileExist(framePath)) {
            auto sprite = Sprite::create(framePath);
            if (sprite) {
                animFrames.pushBack(sprite->getSpriteFrame());
                framesExist = true;
            }
        }
    }

    // 动画帧存在则创建并播放循环动画
    if (framesExist && animFrames.size() > 0) {
        auto animation = Animation::createWithSpriteFrames(animFrames, 0.2f);
        auto animate = Animate::create(animation);
        this->runAction(RepeatForever::create(animate));
    }
}

// 帧更新逻辑（处理攻击冷却、目标校验、移动/攻击行为）
void Unit::update(float dt)
{
    if (_isDead) return; // 死亡单位不处理任何逻辑

    // 更新攻击冷却计时器
    if (_attackTimer > 0) {
        _attackTimer -= dt;
    }

    // 校验目标有效性（目标已销毁则清空）
    if (_target && (_target->getReferenceCount() == 0 || _target->getParent() == nullptr)) {
        _target = nullptr;
    }

    // 有攻击目标时的逻辑
    if (_target) {
        float distance = this->getPosition().distance(_target->getPosition());

        if (distance <= _stats.attackRange) {
            stopMoving(); // 进入攻击范围则停止移动
            // 攻击冷却结束则发起攻击
            if (_attackTimer <= 0) {
                attackTarget(_target);
                _attackTimer = _stats.attackSpeed; // 重置攻击冷却
            }
        }
        else {
            moveTo(_target->getPosition()); // 未进入攻击范围则向目标移动
        }
    }
    // 无攻击目标但处于移动状态（漫游）
    else if (_isMoving) {
        Vec2 currentPos = this->getPosition();
        Vec2 direction = _targetPosition - currentPos;
        float distance = direction.length();

        // 到达目标点则停止移动
        if (distance < 5.0f) {
            stopMoving();
        }
        else {
            // 按移速更新位置
            direction.normalize();
            Vec2 newPos = currentPos + direction * _stats.moveSpeed * dt;
            this->setPosition(newPos);

            // 根据移动方向翻转精灵（左右朝向）
            if (direction.x < 0) this->setFlippedX(true);
            else if (direction.x > 0) this->setFlippedX(false);
        }
    }
}

// 设置攻击目标（赋值目标建筑指针）
void Unit::setTarget(Building* target)
{
    _target = target;
}

// 获取当前攻击目标（返回目标建筑指针）
Building* Unit::getTarget() const
{
    return _target;
}

// 寻找新攻击目标（按兵种偏好筛选候选建筑，优先最近的偏好目标）
void Unit::findNewTarget(const cocos2d::Vector<Building*>& potentialTargets)
{
    Building* bestTarget = nullptr;
    float minDistance = FLT_MAX;
    Vec2 myPos = this->getPosition();

    // 遍历所有候选建筑，筛选最优目标
    for (auto building : potentialTargets) {
        if (!building) continue;

        // 兵种攻击偏好逻辑
        bool isPreferred = false;
        BuildingType bType = building->getBuildingType();

        if (_type == UnitType::GOBLIN) {
            // 哥布林优先攻击资源建筑
            if (bType == BuildingType::GOLD_MINE ||
                bType == BuildingType::ELIXIR_COLLECTOR ||
                bType == BuildingType::GOLD_STORAGE ||
                bType == BuildingType::ELIXIR_STORAGE) {
                isPreferred = true;
            }
        }
        else if (_type == UnitType::GIANT) {
            // 巨人优先攻击防御建筑（暂时只处理主城）
            if (bType == BuildingType::TOWN_HALL) {
                isPreferred = true;
            }
        }
        else {
            // 野蛮人/弓箭手无偏好，攻击所有建筑
            isPreferred = true;
        }

        // 计算有效距离（偏好目标加距离权重）
        float dist = myPos.distance(building->getPosition());
        if (isPreferred) {
            dist -= 10000.0f; // 偏好目标强制优先（距离减大数）
        }

        // 选取距离最近的目标
        if (dist < minDistance) {
            minDistance = dist;
            bestTarget = building;
        }
    }

    setTarget(bestTarget); // 设置最优目标
}

// 移动到指定位置（设置移动目标并标记移动状态）
void Unit::moveTo(const Vec2& targetPos)
{
    _targetPosition = targetPos;
    _isMoving = true;
}

// 停止移动（重置移动状态）
void Unit::stopMoving()
{
    _isMoving = false;
}

// 判断是否可攻击目标（检查目标是否在攻击范围内）
bool Unit::canAttack(Building* target) const
{
    if (!target) return false;
    float dist = this->getPosition().distance(target->getPosition());
    return dist <= _stats.attackRange;
}

// 攻击目标（播放攻击特效，打印日志，实际需补充扣血逻辑）
void Unit::attackTarget(Building* target)
{
    if (!target) return;

    // 攻击视觉反馈（缩放动画）
    auto scale1 = ScaleTo::create(0.1f, 1.2f);
    auto scale2 = ScaleTo::create(0.1f, 1.0f);
    this->runAction(Sequence::create(scale1, scale2, nullptr));

    CCLOG("%s attacked building!", _stats.name.c_str()); // 打印攻击日志
}

// 随机漫游（生成随机目标点并移动，无目标时调用）
void Unit::wanderAround(const Vec2& center, float radius)
{
    stopAllActions();
    _isMoving = false;

    // 随机生成漫游目标点（圆形区域内）
    float angle = CCRANDOM_0_1() * M_PI * 2;
    float dist = CCRANDOM_0_1() * radius;
    Vec2 targetPos = center + Vec2(cos(angle) * dist, sin(angle) * dist);

    moveTo(targetPos); // 移动到随机点
}