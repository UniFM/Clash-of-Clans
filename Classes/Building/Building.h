#pragma once
/*************************************************************
* @file     : Building.h
* @function ：建筑基类 - 部落冲突建筑系统
* @author   : 叶芷含
* @note     ：包含建筑的基础属性、放置逻辑、与地图交互等
**************************************************************/

#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"
#include "BuildingData.h"

USING_NS_CC;

class SceneMap; // 前向声明

class Building : public Node {
public:
    // 创建建筑的静态方法
    static Building* create(BuildingType type, int level = 1);
    
    // 初始化方法
    virtual bool init(BuildingType type, int level);
    
    // 建筑放置相关方法
    bool canPlaceAt(SceneMap* map, const Vec2& position);      // 检查是否可以放置
    void setPlaceMode(bool placing);                           // 设置放置模式
    void placeOnMap(SceneMap* map, const Vec2& position);      // 确认放置到地图
    
    // 基础属性访问器
    BuildingType getBuildingType() const { return type; }
    int getLevel() const { return level; }
    Size getBuildingSize() const { return gridSize * 16.0f; } 
    Vec2 getGridPosition() const { return gridPosition; }
    
    // 状态检查
    bool checkIsPlaced() const { return isPlaced; }
    bool isInPlaceMode() const { return inPlaceMode; }

protected:
    Building();
    virtual ~Building();
    
    // 内部方法
    void updateAppearance();                    // 更新建筑外观
    void updatePlacePreview(bool canPlace);     // 更新放置预览效果
    
private:
    BuildingType type;         // 建筑类型
    int level;                // 建筑等级
    Size gridSize;            // 建筑占地格子大小
    Vec2 gridPosition;        // 在地图网格中的位置
    
    bool isPlaced;            // 是否已放置到地图
    bool inPlaceMode;         // 是否处于放置模式
    
    Sprite* buildingSprite;   // 建筑精灵
    Sprite* shadowSprite;     // 阴影精灵（放置预览时显示）
    
    SceneMap* ownerMap;       // 所属地图的引用
};

#endif // __BUILDING_H__
