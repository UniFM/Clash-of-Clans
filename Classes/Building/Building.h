#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"
#include "BuildingData.h"

USING_NS_CC;

/**
 * 建筑基类
 */
class Building : public Sprite
{
public:
    static Building* create(BuildingType type, int level = 1);

    virtual bool init(BuildingType type, int level);

    // 建筑属性
    BuildingType getBuildingType() const { return _type; }
    int getLevel() const { return _level; }
    int getMaxLevel() const { return _maxLevel; }
    int getHP() const { return _hp; }
    int getMaxHP() const { return _maxHP; }

    // 升级
    virtual bool upgrade();
    virtual bool canUpgrade() const;

    // 受伤和死亡
    virtual void takeDamage(int damage);
    virtual bool isDestroyed() const { return _hp <= 0; }

    // 网格位置
    void setGridPosition(int gridX, int gridY);
    Vec2 getGridPosition() const { return Vec2(_gridX, _gridY); }

    // 获取建筑尺寸（网格单位）
    virtual Size getGridSize() const { return Size(2, 2); }

    // 获取攻击优先级（用于AI）
    virtual int getAttackPriority() const { return 1; }

protected:
    Building();
    virtual ~Building();

    BuildingType _type;
    int _level;
    int _maxLevel;
    int _hp;    //Health Points血量
    int _maxHP;
    int _gridX;
    int _gridY;

    // 根据类型和等级获取属性
    virtual void updateProperties();
    virtual std::string getImagePath() const;
};

#endif // __BUILDING_H__

