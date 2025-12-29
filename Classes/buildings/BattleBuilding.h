#pragma once
#ifndef __BATTLEBUILDING_H__
#define __BATTLEBUILDING_H__

#include "Troops/Entity.h"

class BattleBuilding : public Entity {
public:
    static BattleBuilding* create(BattleBuildingType type, Team team);
    virtual bool init(BattleBuildingType type, Team team);

    BattleBuildingType getType() const;
    void update(float dt) override;

    // Attack logic for defensive buildings
    void setTarget(Entity* target);
    Entity* getTarget() const;

private:
    BattleBuildingType _type;
    BattleBuildingStats _stats;

    // 添加私有辅助函数
    std::string getImagePathForBattleBuilding(BattleBuildingType type, Team team);
    float getScaleForBattleBuilding(BattleBuildingType type);
    //void setupHealthBar(float height);
    //void updateHealthBar();

    //cocos2d::DrawNode* _healthBar = nullptr; // 添加血条成员

    Entity* _target;
    float _attackTimer;
};

#endif // __BATTLEBUILDING_H__
