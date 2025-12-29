#pragma once
#ifndef __BATTLETROOP_H__
#define __BATTLETROOP_H__

#include "Entity.h"


class BattleTroop : public Entity {
public:
    static BattleTroop* create(BattleTroopType type, Team team);
    virtual bool init(BattleTroopType type, Team team);

    void update(float dt) override;

    void setTarget(Entity* target);
    Entity* getTarget() const;

    // AI
    void findNewTarget(const cocos2d::Vector<Entity*>& potentialTargets);

private:
    BattleTroopType _type;
    BattleTroopStats _stats;

    std::string getImagePathForBattleTroop(BattleTroopType type, Team team);

    Entity* _target;
    float _attackTimer;
};

#endif