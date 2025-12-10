/*************************************************************
* @file     : GameCover.cpp
* @function ：游戏启动界面
* @author   : 叶芷含
* @note     ：
**************************************************************/
#include "SplashScene.h"
#include "LoginScene.h"

USING_NS_CC;
   
Scene* SplashScene::createScene()
{
    return SplashScene::create();
}

// 启动页面初始化
bool SplashScene::init()
{
    if (!Scene::init())
        return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 添加游戏封面
    auto cover = Sprite::create("Scene/Cover.png");
    cover->setPosition(Vec2(visibleSize.width / 2 + origin.x,
        visibleSize.height / 2 + origin.y));
    this->addChild(cover);

    // 添加提示文字
    auto hintLabel = Label::createWithTTF("Click anywhere to continue", "fonts/arial.ttf", 24);
    if (hintLabel) {
        hintLabel->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height * 0.2f + origin.y));
        hintLabel->setColor(Color3B::WHITE);
        this->addChild(hintLabel);

        // 添加闪烁效果
        auto fadeIn = FadeIn::create(1.0f);
        auto fadeOut = FadeOut::create(1.0f);
        auto blink = Sequence::create(fadeOut, fadeIn, nullptr);
        auto repeat = RepeatForever::create(blink);
        hintLabel->runAction(repeat);
    }

    // 添加触摸监听器
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = [this](Touch* touch, Event* event) {
        this->gotoLogin(0.0f);
        return true;
        };
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);

    return true;
}

// 前往登录界面
void SplashScene::gotoLogin(float dt)
{
    auto scene = LoginScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene));
}