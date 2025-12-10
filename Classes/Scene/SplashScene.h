#pragma once
/*************************************************************
* @file     : GameCover.h
* @function ：游戏欢迎界面
* @author   : 叶芷含
* @note     ：
**************************************************************/
#ifndef __GAMECOVER_H__
#define __GAMECOVER_H__
#include "cocos2d.h"

class SplashScene : public cocos2d::Scene{
public:
	static cocos2d::Scene* createScene();

	virtual bool init();

private:
	// 登录界面
	void gotoLogin(float dt);

	CREATE_FUNC(SplashScene);
};

#endif