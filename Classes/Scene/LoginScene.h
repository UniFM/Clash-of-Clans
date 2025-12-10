#pragma once
/*************************************************************
* @file     : LoginScene.h
* @function ：登录界面
* @author   : 叶芷含
* @note     ：用户登录验证界面
**************************************************************/
#ifndef __LOGINSCENE_H__
#define __LOGINSCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class LoginScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init();

    // 登录按钮回调
    void onLoginButtonClicked(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);

private:
    cocos2d::ui::TextField* usernameField;
    cocos2d::ui::TextField* passwordField;
    cocos2d::ui::Button* loginButton;
    cocos2d::Label* statusLabel;

    // 执行登录验证
    bool performLogin(const std::string& username, const std::string& password);

    // 进入村庄地图
    void enterVillageMap();

    CREATE_FUNC(LoginScene);
};

#endif