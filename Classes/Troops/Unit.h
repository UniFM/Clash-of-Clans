/*************************************************************
* @file     : Unit.h
* @function ：游戏单位基类头文件 - 定义兵种核心属性与接口
* @author   : 俞筱雨
* @note     : 1. 定义兵种类型枚举及基础属性结构体；
*             2. 封装单位创建、移动、攻击、寻路核心接口；
*             3. 管理单位状态（移动/攻击/死亡）及目标选择逻辑。
**************************************************************/

#pragma once
#ifndef __UNIT_H__
#define __UNIT_H__

#include "cocos2d.h"
#include "buildings/BuildingsData.h"

USING_NS_CC;

// 单位类型枚举（包含所有可训练兵种及空值类型）
enum class UnitType {
    BARBARIAN,  // 野蛮人
    ARCHER,     // 弓箭手
    GIANT,      // 巨人
    GOBLIN,     // 哥布林
    NONE        // 无类型（默认/无效）
};

// 单位属性结构体（存储单个单位的所有基础战斗/养成属性）
struct UnitStats {
    std::string name;        // 单位名称
    int maxHealth;           // 最大生命值
    int damage;              // 单次攻击伤害
    float moveSpeed;         // 移动速度（像素/秒）
    float attackRange;       // 攻击范围（像素）
    float attackSpeed;       // 攻击速度（攻击/秒）
    int trainingTime;        // 训练时间（秒）
    int housingSpace;        // 占用人口空间
    int trainingCost;        // 训练成本（金币/圣水）
};

// 游戏单位基类（所有作战单位的通用逻辑实现）
class Unit : public Sprite
{
public:
    // 创建单位实例（传入类型返回对应Unit对象）
    static Unit* create(UnitType type);

    // 初始化单位（根据类型加载属性和资源）
    virtual bool init(UnitType type);

    // 帧更新函数（处理移动、攻击、状态等每帧逻辑）
    void update(float dt) override;

    // 设置攻击目标（指定要攻击的建筑）
    void setTarget(Building* target);

    // 获取当前攻击目标（返回目标建筑指针，无则返回nullptr）
    Building* getTarget() const;

    // 获取单位类型（返回当前单位的类型枚举值）
    UnitType getUnitType() const { return _type; }

    // 寻找新攻击目标（从候选建筑列表中筛选优先级最高的目标）
    void findNewTarget(const cocos2d::Vector<Building*>& potentialTargets);

    // 随机漫游（无目标时在指定区域内随机移动）
    void wanderAround(const Vec2& center, float radius);

    // 判断是否可攻击目标（检查目标是否在攻击范围且有效）
    bool canAttack(Building* target) const;

    // 移动到指定位置（设置移动目标并启动移动逻辑）
    void moveTo(const Vec2& targetPos);

    // 停止移动（重置移动状态，终止当前移动行为）
    void stopMoving();

protected:
    Unit() = default;          // 构造函数（保护级，禁止直接实例化）
    virtual ~Unit() = default; // 析构函数（保护级，通过内存管理机制释放）

    UnitType _type;               // 单位类型
    UnitStats _stats;             // 单位基础属性
    Building* _target;            // 当前攻击目标
    float _attackTimer;           // 攻击冷却计时器

    cocos2d::DrawNode* _healthBar; // 生命值条绘制节点
    bool _isMoving;               // 移动状态标记
    Vec2 _targetPosition;         // 移动目标位置
    int _currentHealth;           // 当前生命值
    int _maxHealth;               // 最大生命值
    bool _isDead;                 // 死亡状态标记

    // 更新单位属性（根据类型加载对应配置）
    void updateProperties();

    // 获取图片路径（返回对应类型单位的纹理路径）
    std::string getImagePath() const;

    // 播放闲置动画（无操作时的默认动画）
    void runIdleAnimation();

    // 攻击目标（对指定建筑造成伤害并重置攻击冷却）
    void attackTarget(Building* target);
};

#endif // __UNIT_H__