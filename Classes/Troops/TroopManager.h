
/*************************************************************
* @file     : TroopManager.cpp
* @function ：兵种管理器实现 - 单例管控兵种数量与人口容量
* @author   : 俞筱雨
* @note     : 1. 单例模式管理兵种选中数量、兵营等级配置；
*             2. 计算兵营人口容量，判断兵种添加容量是否充足；
*             3. 封装兵种数量增删、清空及容量占用计算逻辑。
**************************************************************/


#pragma once
#ifndef TROOP_MANAGER_H
#define TROOP_MANAGER_H

#include "TroopDefinitions.h"
#include "buildings/BuildingsData.h"
#include <map>

// 兵种管理器类（单例模式）
class TroopManager {
public:
    // 获取兵种管理器单例实例
    static TroopManager* getInstance();

    // 游戏状态相关接口
    void setBarracksLevel(int level);    // 设置兵营等级
    int getBarracksLevel() const;        // 获取兵营等级

    int getMaxHousingSpace() const;      // 获取最大人口容量

    // 兵种选择相关接口
    void setTroopCount(TroopType type, int count); // 设置指定兵种的数量
    int getTroopCount(TroopType type) const;       // 获取指定兵种的数量
    void clearTroops();                            // 清空所有已选兵种数量

    int getCurrentHousingSpace() const;  // 计算当前已占用的人口容量
    bool canAddTroop(TroopType type) const;        // 判断是否可添加指定兵种（容量是否充足）

    // 已选兵种列表（存储各兵种对应的数量）
    std::map<TroopType, int> selectedTroops;
    // 获取已选兵种列表（只读）
    const std::map<TroopType, int>& getSelectedTroops() const { return selectedTroops; }

private:
    TroopManager();                      // 私有构造函数（确保单例）
    static TroopManager* instance;       // 单例对象指针

    int barracksLevel;                   // 兵营等级（影响最大人口容量）
};

#endif