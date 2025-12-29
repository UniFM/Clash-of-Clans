//#pragma once
//#ifndef __UNIT_H__
//#define __UNIT_H__
//
//#include "cocos2d.h"
//
///**
// * @brief 兵种类（战斗单位）
// * @details 封装所有兵种的核心战斗逻辑，包括移动、攻击、目标选择等
// *          支持不同兵种类型（野蛮人、弓箭手、巨人等）的差异化行为
// */
//
//USING_NS_CC;
//
//// 兵种类型枚举
//enum class UnitType {
//    BARBARIAN,    ///< 野蛮人（基础近战兵种，高血量低伤害）
//    ARCHER,       ///< 弓箭手（基础远程兵种，低血量高射程）
//    GIANT,        ///< 巨人（肉盾    /兵种，优先攻击防御建筑）
//    GOBLIN,  ///< 哥布林 (优先选择资源建筑（金矿、圣水收集器）
//    NONE        
//};
//
//// 兵种属性配置（生命值、攻击力、移动速度等，从GameManager获取）
//struct UnitStats {
//    std::string name;        ///< 兵种名称（用于UI显示：如"野蛮人"）
//    int maxHealth;           ///< 最大生命值（为0则兵种死亡）
//    int damage;              ///< 单次攻击伤害
//    float moveSpeed;         ///< 移动速度（像素/秒）
//    float attackRange;       ///< 攻击范围（像素，远程兵种>0，近战=0）
//    float attackSpeed;       ///< 攻击速度（秒/次，越小攻击越快）
//    int trainingTime;        ///< 训练时间（秒）
//    int housingSpace;        ///< 占用兵营空间（决定单场战斗可派出的数量）
//    int trainingCost;        ///< 训练成本（圣水，0表示免费）
//};
//
//class Building;
//
///**
// * 兵种基类
// */
//class Unit : public Sprite
//{
//public:
//    // Unit* 初始化完成的兵种实例（自动加入内存管理池）
//    static Unit* create(UnitType type);
//    // 内部会从GameManager获取对应兵种的属性配置（UnitStats），初始化生命值、移动速度、攻击力等核心属性，并创建血条等可视化组件
//    virtual bool init(UnitType type);
//
//    /**
//     * @brief 每帧更新方法（重写Node的update）
//     * @param dt 帧间隔时间（秒）
//     * @details 核心战斗逻辑入口：
//     *          1. 若兵种死亡则跳过所有逻辑
//     *          2. 有有效目标时：判断距离，近则攻击，远则移动
//     *          3. 无目标/目标死亡时：等待外部调用findNewTarget重新寻敌
//     */
//    //void update(float dt) override;
//
//    /**
//     * @brief 设置当前攻击目标
//     * @param target 目标建筑
//     * @note 外部负责调用该方法分配目标，内部仅维护目标引用
//     */
//    //void setTarget(Building* target);
//
//    /**
//     * @brief 获取当前攻击目标
//     * @return Unit* 当前目标（无目标返回nullptr）
//     */
//    //Unit* getTarget() const;
//
//    //// 属性
//    //UnitType getUnitType() const { return _type; }
//    //int getHP() const { return _hp; }
//    //int getMaxHP() const { return _maxHP; }
//    //int getAttackDamage() const { return _attackDamage; }
//    //float getAttackRange() const { return _attackRange; }
//    //float getMoveSpeed() const { return _moveSpeed; }
//    //float getAttackInterval() const { return _attackInterval; }
//
//    /**
//     * @brief AI寻敌逻辑：从候选目标列表中筛选最优攻击目标
//     * @param potentialTargets 场景中所有敌方实体列表（由GameScene提供）
//     * @details 不同兵种有差异化寻敌规则：
//     *          - 普通兵种（野蛮人/弓箭手）：选择最近的敌方实体
//     *          - 巨人：优先选择防御建筑（加农炮、箭塔），无则选最近目标
//     *          - 哥布林：优先选择资源建筑（金矿、圣水收集器）
//     * @note 该方法仅筛选目标，不执行攻击/移动，筛选结果通过setTarget生效
//     */
//    //void findNewTarget(const cocos2d::Vector<Unit*>& potentialTargets);
//
//    /**
//     * @brief 设置核心属性
//     * @param maxHealth 最大生命值
//     * @details 初始化当前生命值（_currentHealth = maxHealth），
//     *          在init中调用该方法
//     */
//    //void setStats(int maxHealth);
//
//    /**
//     * @brief 受击扣血逻辑
//     * @param damage 受到的伤害值（需≥0）
//     * @details 核心逻辑：
//     *          1. 若已死亡，直接返回
//     *          2. 扣除对应生命值（_currentHealth -= damage）
//     *          3. 生命值≤0时标记为死亡（_isDead = true）
//     *          4. 更新血条显示
//     * @note 伤害值为负时不处理，避免出现生命值增加的异常
//     */
//    //void takeDamage(int damage);
//
//    /**
//     * @brief 判断实体是否死亡
//     * @return bool 死亡返回true，存活返回false
//     * @details 外部逻辑（如战斗场景清理、目标选择）通过该方法判断实体状态，
//     *          避免直接访问私有变量_isDead
//     */
//    //bool isDead() const;
//
//    // ===================== 可视化相关方法 =====================
//    /**
//     * @brief 创建血条（可视化生命值）
//     * @details 基于DrawNode绘制血条：
//     *          1. 血条位置固定在实体上方（偏移量可自定义）
//     *          2. 初始为绿色，随生命值降低变色（黄/红）
//     *          3. 血条尺寸与实体尺寸适配
//     * @note 建议在实体精灵加载完成后调用该方法
//     */
//    //void createHealthBar();
//
//    /**
//     * @brief 更新血条显示
//     * @details 核心逻辑：
//     *          1. 计算生命值比例（_currentHealth / _maxHealth）
//     *          2. 根据比例调整血条长度和颜色：
//     *             - >50%：绿色
//     *             - 20%~50%：黄色
//     *             - <20%：红色
//     *          3. 实体死亡时隐藏血条
//     */
//    //void updateHealthBar();
//
//    // 游荡行为
//    void wanderAround(const Vec2& center, float radius);
//
//    //// 静态方法：获取兵种属性
//    //static int getGoldCost(UnitType type);
//    //static int getElixirCost(UnitType type);
//    //static int getPopulationCost(UnitType type);
//    //static float getTrainingTime(UnitType type);
//
//    //// 状态
//    //bool isDead() const { return _hp <= 0; }
//    //bool isMoving() const { return _isMoving; }
//    //bool isAttacking() const { return _isAttacking; }
//
//    //// 战斗
//    //void takeDamage(int damage);
//    ////void attackTarget(Building* target);
//    ////bool canAttack(Building* target) const;
//
//    //// 移动
//    //void moveTo(const Vec2& targetPos);
//    //void stopMoving();
//
//protected:
//
//    /**
//     * @brief 构造函数私有化（强制使用create方法创建实例）
//     */
//    //Unit() = default;
//
//    /**
//     * @brief 析构函数私有化
//     * @note 由Cocos2d的内存管理池自动释放，外部无需手动调用
//     */
//    virtual ~Unit() = default;
//
//    cocos2d::Sprite* _sprite;       ///< 实体精灵（可视化模型，子类需加载对应纹理）
//
//    UnitType _type;          ///< 兵种类型标识（用于区分不同兵种的行为/属性）
//    UnitStats _stats;        ///< 兵种属性配置（生命值、攻击力、移动速度等，从GameManager获取）
//    Unit* _target;          ///< 当前攻击目标（弱引用，不负责内存释放）
//    float _attackTimer;       ///< 攻击计时器（累计帧间隔时间，达到攻击间隔则触发攻击）
//
//
//    cocos2d::DrawNode* _healthBar;  ///< 血条绘制节点（可视化生命值，由createHealthBar创建）
//
//    int _currentHealth;             ///< 当前生命值（随受击减少，≤0则死亡）
//    int _maxHealth;                 ///< 最大生命值
//    bool _isDead;                   ///< 死亡状态标识（避免重复处理死亡逻辑）
//
//    //UnitType _type;
//    //int _hp;
//    //int _maxHP;
//    //int _attackDamage;
//    //float _attackRange;
//    float _moveSpeed;
//    //float _attackInterval;
//
//    bool _isMoving;
//    //bool _isAttacking;
//    //Vec2 _targetPosition;
//    //Building* _currentTarget;
//    //float _lastAttackTime;
//
//    //void updateProperties();
//    //std::string getImagePath() const;
//};
//
//#endif // __UNIT_H__
//

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