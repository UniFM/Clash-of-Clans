/*************************************************************
* @file     : TroopDefinitions.h
* @function ：兵种定义头文件 - 统一兵种类型与属性配置
* @author   : 俞筱雨
* @note     : 1. 重命名UnitType为TroopType，统一命名规范；
*             2. 定义兵种信息结构体，包含名称、人口、图片路径；
*             3. 配置各兵种基础属性映射表（野蛮人/弓箭手等）。
**************************************************************/

#pragma once
#ifndef TROOP_DEFINITIONS_H
#define TROOP_DEFINITIONS_H

#include <string>
#include <map>
#include "Troops/Unit.h"

// 将UnitType重命名为TroopType以保持命名一致性
using TroopType = UnitType;

// 兵种信息结构体
struct TroopInfo {
    std::string name;        // 兵种名称
    int housingSpace;        // 占用人口/空间
    std::string imagePath;   // 图片路径（替代原有的颜色码）
};

// 兵种数据映射表
static const std::map<TroopType, TroopInfo> TROOP_DATA = {
    {TroopType::BARBARIAN, {"Barbarian", 1, "Troops/Barbarian.png"}}, // 野蛮人
    {TroopType::ARCHER,    {"Archer",    3, "Troops/Archer.png"}},    // 弓箭手
    {TroopType::GOBLIN,    {"Goblin",    3, "Troops/Goblin.png"}},    // 哥布林（占位）
    {TroopType::GIANT,     {"Giant",     5, "Troops/Giant.png"}}      // 巨人
};

#endif // TROOP_DEFINITIONS_H