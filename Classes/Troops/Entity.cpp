/*************************************************************
* @file     : Entity.cpp
* @function ：战斗实体基类实现 - 血量/队伍/血条核心管理
* @author   : 俞筱雨
* @note     : 1. 初始化实体基础属性，适配队伍与生命值配置；
*             2. 实现伤害计算、死亡判定及死亡特效；
*             3. 封装血条创建与动态更新，适配血量百分比变色。
**************************************************************/

#include "Entity.h"

USING_NS_CC;

// 构造函数：初始化实体属性
Entity::Entity() : _currentHealth(0), _maxHealth(0), _team(Team::PLAYER), _isDead(false), _sprite(nullptr), _healthBar(nullptr), _entityName("Entity") {}

// 析构函数
Entity::~Entity() {}

// 初始化函数
bool Entity::init() {
    // 调用父类初始化
    if (!Node::init()) return false;
    return true;
}

// 设置实体属性（最大生命值、所属队伍）
void Entity::setStats(int maxHealth, Team team) {
    _maxHealth = maxHealth;
    _currentHealth = maxHealth;
    _team = team;
    // 创建血条
    createHealthBar();
}

// 受到伤害处理
void Entity::takeDamage(int damage) {
    // 已死亡则不处理
    if (_isDead) return;

    // 扣除生命值
    _currentHealth -= damage;
    // 生命值小于等于0则判定死亡
    if (_currentHealth <= 0) {
        _currentHealth = 0;
        _isDead = true;
        // 死亡效果：渐隐后移除自身
        this->runAction(Sequence::create(FadeOut::create(0.5f), RemoveSelf::create(), nullptr));
    }
    // 更新血条显示
    updateHealthBar();
}

// 判断是否死亡
bool Entity::isDead() const {
    return _isDead;
}

// 获取所属队伍
Team Entity::getTeam() const {
    return _team;
}

// 创建血条
void Entity::createHealthBar() {
    // 如果已有血条则先移除
    if (_healthBar) {
        _healthBar->removeFromParent();
    }
    // 创建绘制节点作为血条
    _healthBar = DrawNode::create();
    this->addChild(_healthBar, 100); // 设置高优先级确保显示在顶层
    // 更新血条显示
    updateHealthBar();
}

// 更新血条显示
void Entity::updateHealthBar() {
    // 无血条或已死亡则不更新
    if (!_healthBar || _isDead) return;

    // 清空原有绘制内容
    _healthBar->clear();

    // 血条尺寸和偏移（在精灵上方）
    float width = 40.0f;
    float height = 5.0f;
    float yOffset = 60.0f; // 血条在精灵上方的偏移量

    // 绘制血条背景（灰色）
    _healthBar->drawSolidRect(Vec2(-width / 2, yOffset), Vec2(width / 2, yOffset + height), Color4F::GRAY);

    // 计算生命值百分比
    float healthPct = (float)_currentHealth / _maxHealth;
    // 根据生命值百分比设置血条颜色
    Color4F color = Color4F::GREEN; // 健康-绿色
    if (healthPct < 0.5f) color = Color4F::YELLOW; // 半血-黄色
    if (healthPct < 0.2f) color = Color4F::RED; // 残血-红色

    // 绘制当前生命值进度条
    _healthBar->drawSolidRect(Vec2(-width / 2, yOffset), Vec2(-width / 2 + width * healthPct, yOffset + height), color);
}