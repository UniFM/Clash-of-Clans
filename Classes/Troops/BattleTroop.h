/*************************************************************
* @file     : BattleTroop.h
* @function ：战斗兵种核心实现 - 兵种AI+动画系统+攻击逻辑
* @author   : 俞筱雨 叶芷含
* @note     : 1. 实现兵种创建、初始化及帧更新，适配不同兵种属性；
*             2. 支持闲置/移动/攻击动画切换，集成攻击特效可视化；
*             3. 差异化AI目标选择，适配兵种特性（巨人/哥布林等）；
*             4. 适配Entity基类，完成血量、伤害、移动寻路等核心逻辑。
**************************************************************/


#pragma once
#ifndef __BATTLETROOP_H__
#define __BATTLETROOP_H__

#include "Entity.h"
#include "Constant/Constant.h"

// 【新增】动画状态枚举
enum class AnimationState {
    IDLE,       // 闲置状态
    MOVING,     // 移动状态
    ATTACKING   // 攻击状态
};

class BattleTroop : public Entity {
public:
    static BattleTroop* create(BattleTroopType type, Team team);
    virtual bool init(BattleTroopType type, Team team);

    BattleTroopType getType() const { return _type; } // 【修正】返回正确的类型
    void update(float dt) override;

    // Attack logic for defensive buildings
    void setTarget(Entity* target);
    Entity* getTarget() const;
    
    // AI
    void findNewTarget(const cocos2d::Vector<Entity*>& potentialTargets);

private:
    BattleTroopType _type;
    BattleTroopStats _stats;

    // 添加私有辅助函数
    std::string getImagePathForBattleTroop(BattleTroopType type, Team team);

    Entity* _target;
    float _attackTimer;
    
    // 【新增】动画相关成员变量和方法
    AnimationState _currentAnimationState; // 当前动画状态
    
    // 动画播放方法
    void runIdleAnimation();        // 播放闲置动画
    void runMoveAnimation();        // 播放移动动画
    void runAttackAnimation();      // 播放攻击动画
    
    // 辅助方法
    cocos2d::Vector<cocos2d::SpriteFrame*> getAnimationFrames(const std::string& animationType);
    std::string getBaseImagePath();  // 获取基础图片路径（去掉扩展名）
    void showAttackEffect();         // 显示攻击特效
};

#endif // __BATTLETROOP_H__