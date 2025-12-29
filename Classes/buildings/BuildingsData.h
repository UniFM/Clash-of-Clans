/*************************************************************
* @file     : BuildingsData.h
* @function ：建筑基类 - 部落冲突建筑系统
* @author   : 齐颖
* @note     ：定义所有建筑的通用接口和成员变量，无实现代码
**************************************************************/


#ifndef __BUILDINGSDATA_H__
#define __BUILDINGSDATA_H__

#include "cocos2d.h"
#include "Map/HomeVillageMap.h"

USING_NS_CC;

// 建筑类型枚举
enum class BuildingType {
    TOWN_HALL,          // 大本营
    BARRACKS,           // 兵营
    GOLD_MINE,          // 金矿
    ELIXIR_COLLECTOR,   // 圣水收集器
    GOLD_STORAGE,       //储金罐
    ELIXIR_STORAGE,      //圣水瓶
    CANNON,              //加农炮
    ARCHER_TOWER,        // 弓箭塔
};

class Building : public Sprite {
public:
    // 构造与析构
    Building();
    virtual ~Building();
    void showMenuImmediately(); // 强制显示菜单接口
    virtual bool init(BuildingType type, int level = 1);

    // 获取建筑属性
    BuildingType getBuildingType() const { return _type; }
    int getLevel() const { return _level; }
    Size getBuildingSize() const { return _buildingSize; }

    // 设置建筑位置（适配瓦片地图）
    void setBuildingPosition(const Vec2& worldPos);

    // 触摸事件处理（移动+点击判定）
    virtual bool onTouchBegan(Touch* touch, Event* event);
    virtual void onTouchMoved(Touch* touch, Event* event);
    virtual void onTouchEnded(Touch* touch, Event* event);

    // 建筑升级接口
    virtual bool upgradeBuilding();

    // 切换菜单显示/隐藏
    void toggleUpgradeMenu();
    void closeUpgradeMenu();
    void removeBuilding();

    // 信息菜单相关方法
    void showBuildingInfo();  // 显示建筑信息图片
    void hideBuildingInfo();  // 隐藏信息图片，返回菜单界面

    // 网格位置
    void setGridPosition(int gridX, int gridY);
    Vec2 getGridPosition() const { return Vec2(_gridX, _gridY); }

    // 获取建筑尺寸（网格单位）
    virtual Size getGridSize() const { return Size(2, 2); }

    // ==============================================
    //  新增公共访问接口（解决权限报错核心）
    // ==============================================
    bool isMenuShow() const { return _isMenuShow; }          // 获取菜单显示状态
    Node* getUpgradeMenu() const { return _upgradeMenu; }    // 获取菜单根节点
    MenuItemImage* getUpgradeBtn() const { return _upgradeBtn; } // 获取升级按钮
    MenuItemImage* getCancelBtn() const { return _cancelBtn; }   // 获取删除按钮
    MenuItemImage* getInfoBtn() const { return _infoBtn; }       // 获取信息按钮


protected:
    // 纯虚函数：子类必须实现
    virtual bool loadBuildingRes() = 0;                  // 加载建筑资源
    virtual bool loadBuildingInfoRes() = 0;              // 加载建筑信息图片

    // 通用方法：基类实现
    void initTouchListener();                            // 初始化触摸监听
    void createUpgradeMenu();                            // 创建升级菜单
    void createInfoLayer();                              // 创建信息图片层

    // 核心成员变量
    BuildingType _type;          // 建筑类型
    int _level;                  // 建筑等级
    Size _buildingSize;          // 建筑尺寸
    Vec2 _originalPos;           // 原始位置
    Vec2 _touchOffset;           // 触摸偏移量

    // 操作状态变量
    bool _isDragging;            // 是否正在拖动
    bool _isMenuShow;            // 菜单是否显示
    bool _isInfoShow;            // 信息图片是否显示
    float _dragThreshold;        // 拖动阈值（像素）
    Vec2 _touchStartPos;         // 触摸起始位置

    // 触摸监听对象
    EventListenerTouchOneByOne* _touchListener;
    EventListenerTouchOneByOne* _infoTouchListener;

    // 菜单相关节点
    Node* _upgradeMenu;          // 菜单根节点
    MenuItemImage* _upgradeBtn;  // 升级按钮
    MenuItemImage* _cancelBtn;   // 取消按钮
    MenuItemImage* _infoBtn;     // 信息按钮

    // 信息图片相关节点
    Node* _infoLayer;            // 信息界面根节点
    Sprite* _infoImage;          // 建筑信息图片

    // 子类差异化配置（新增：解决未声明报错）
    Vec2 m_menuOffset;          // 菜单偏移量
    Vec2 m_infoImageOffset;     // 信息图片偏移量

    int _gridX;
    int _gridY;
};

// 大本营类声明
class TownHall : public Building {
public:
    static TownHall* create(int level = 1);
    bool upgradeBuilding() override;

protected:
    bool loadBuildingRes() override;
    bool loadTownHallResByLevel(int level);
    bool loadBuildingInfoRes() override;
    bool loadTownHallInfoResByLevel(int level);
};


// 金矿类声明
class GoldMine : public Building {
public:
    static GoldMine* create(int level = 1);
    bool upgradeBuilding() override;

protected:
    bool loadBuildingRes() override;
    bool loadGoldMineResByLevel(int level);
    bool loadBuildingInfoRes() override;
    bool loadGoldMineInfoResByLevel(int level);
};

//圣水收集器类声明
class ElixirCollector : public Building {
public:
    static ElixirCollector* create(int level = 1);
    bool upgradeBuilding() override;

protected:
    bool loadBuildingRes() override;
    bool loadElixirCollectorResByLevel(int level);
    bool loadBuildingInfoRes() override;
    bool loadElixirCollectorInfoResByLevel(int level);
};

//储金罐类声明
class GoldStorage : public Building {
public:
    // 创建方法（与圣水收集器/金矿/大本营一致）
    static GoldStorage* create(int level = 1);
    bool upgradeBuilding() override;

protected:
    // 子类专属资源加载方法
    virtual bool loadBuildingRes() override;
    bool loadGoldStorageResByLevel(int level);
    virtual bool loadBuildingInfoRes() override;
    bool loadGoldStorageInfoResByLevel(int level);
};

//圣水瓶类声明
class ElixirStorage : public Building {
public:
    // 创建方法（与储金罐/圣水收集器/金矿/大本营一致）
    static ElixirStorage* create(int level);
    bool upgradeBuilding() override;

protected:
    // 子类专属资源加载方法
    virtual bool loadBuildingRes() override;
    bool loadElixirStorageResByLevel(int level);
    virtual bool loadBuildingInfoRes() override;
    bool loadElixirStorageInfoResByLevel(int level);
};

// 加农炮类声明
class Cannon : public Building {
public:
    static Cannon* create(int level = 1);
    bool upgradeBuilding() override;

protected:
    bool loadBuildingRes() override;
    bool loadCannonResByLevel(int level);
    bool loadBuildingInfoRes() override;
    bool loadCannonInfoResByLevel(int level);
};

// 弓箭塔类声明
class ArcherTower : public Building {
public:
    static ArcherTower* create(int level = 1);
    bool upgradeBuilding() override;

protected:
    bool loadBuildingRes() override;
    bool loadArcherTowerResByLevel(int level);
    bool loadBuildingInfoRes() override;
    bool loadArcherTowerInfoResByLevel(int level);
};

// 兵营类声明
class Barracks : public Building {
public:
    static Barracks* create(int level = 1);
    bool upgradeBuilding() override;

protected:
    bool loadBuildingRes() override;
    bool loadBarracksResByLevel(int level);
    bool loadBuildingInfoRes() override;
    bool loadBarracksInfoResByLevel(int level);
};


// 全局创建方法声明
void createLevel1TownHall();
void createLevel1GoldMine();
void createLevel1ElixirCollector();
void createLevel1GoldStorage();
void createLevel1ElixirStorage();
void createLevel1Cannon();
void createLevel1ArcherTower();
void createLevel1Barracks();

#endif // __BUILDINGSDATA_H__