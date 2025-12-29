/*************************************************************
@file : BattleBuilding.h
@function ：战斗建筑头文件 - 定义战斗建筑类结构及核心接口
@author : 俞筱雨
@note : 1. 继承 Entity 类实现战斗建筑基础属性；2. 定义多类型建筑创建 / 初始化 / 帧更新接口；3. 封装防御建筑攻击目标管理逻辑；
4. 包含建筑图片路径 / 缩放比例私有辅助函数声明；5. 存储建筑类型、属性、攻击目标、攻击计时等核心成员变量
**************************************************************/
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
