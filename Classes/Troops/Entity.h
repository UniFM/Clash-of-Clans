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

    // Visuals
    void createHealthBar();
    void updateHealthBar();

protected:
    cocos2d::Sprite* _sprite;
    cocos2d::DrawNode* _healthBar;

    int _currentHealth;
    int _maxHealth;
    Team _team;
    bool _isDead;
};

#endif // __ENTITY_H__
