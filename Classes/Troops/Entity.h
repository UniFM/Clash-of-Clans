/*************************************************************
* @file     : Entity.h
* @function ：战斗实体基类实现 - 血量/队伍/血条核心管理
* @author   : 俞筱雨
* @note     : 1. 初始化实体基础属性，适配队伍与生命值配置；
*             2. 实现伤害计算、死亡判定及死亡特效；
*             3. 封装血条创建与动态更新，适配血量百分比变色。
**************************************************************/

#pragma once
#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "cocos2d.h"
#include "Constant/Constant.h"

class Entity : public cocos2d::Node {
public:
    Entity();
    virtual ~Entity();

    virtual bool init() override;

    void setStats(int maxHealth, Team team);
    void takeDamage(int damage);
    bool isDead() const;
    Team getTeam() const;

    // 【修改】避免与cocos2d::Node::getName冲突，使用不同的方法名
    virtual std::string getEntityName() const { return _entityName; }
    virtual void setEntityName(const std::string& name) { _entityName = name; }

    // 可视化
    void createHealthBar();
    void updateHealthBar();

protected:
    cocos2d::Sprite* _sprite;
    cocos2d::DrawNode* _healthBar;

    int _currentHealth;
    int _maxHealth;
    Team _team;
    bool _isDead;
    std::string _entityName; // 【修改】重命名避免冲突
};

#endif // __ENTITY_H__
