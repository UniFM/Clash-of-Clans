#pragma once
/*************************************************************
* @file     : TroopSelectScene.h
* @function ：选兵场景 - 部落冲突选兵系统
* @author   : 俞筱雨
* @note     ：包含选兵人数上限和显示等功能
**************************************************************/

#ifndef TROOP_SELECTION_SCENE_H
#define TROOP_SELECTION_SCENE_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Constant/Constant.h"
#include "buildings/BuildingsData.h"
#include "Control/GameManager.h"
#include "Troops/TroopDefinitions.h"
#include "Troops/Unit.h"

USING_NS_CC;
using namespace ui;

class TroopSelectionScene : public Scene {
public:

    // 创建场景
    static Scene* createScene() {
        return TroopSelectionScene::create();
    }

    static TroopSelectionScene* create();

    // 初始化
    virtual bool init() override;

    // UI Callbacks
    void onBackCallback(cocos2d::Ref* pSender);
    void onTroopSelected(TroopType type);   // Add (Plus)
    void onTroopDeselected(TroopType type); // Remove (Minus)
    void onResetCallback(cocos2d::Ref* pSender);

    // Update labels
    void updateLabels();

    //CREATE_FUNC(TroopSelectionScene);这个报错应该是已经有主体了

    // 更新兵营显示
    void updateTroopDisplay();

    cocos2d::Label* getCapacityLabel() {
        return capacityLabel;
    }

private:
    cocos2d::Label* capacityLabel;
    std::map<TroopType, cocos2d::Label*> countLabels;

    //// 事件处理
    //void onCloseButtonClicked(Ref* sender); // 关闭按钮点击
    //void onBackKeyPressed();                // 返回键处理
};

#endif