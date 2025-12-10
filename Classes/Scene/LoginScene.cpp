/*************************************************************
* @file     : LoginScene.cpp
* @function ：登录界面
* @author   : 叶芷含
* @note     ：用户登录验证界面
**************************************************************/
#include "LoginScene.h"
#include "Map/HomeVillageMap.h"

USING_NS_CC;

Scene* LoginScene::createScene()
{
    return LoginScene::create();
}

bool LoginScene::init()
{
    if (!Scene::init())
        return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 添加背景
    auto background = Sprite::create("Scene/LoginBackground.png");
    background->setPosition(Vec2(visibleSize.width / 2 + origin.x,
        visibleSize.height / 2 + origin.y));
    this->addChild(background);

    // 游戏标题
    auto titleLabel = Label::createWithTTF("Clash of Clans", "fonts/arial.ttf", 48);
    if (titleLabel) {
        titleLabel->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height * 0.8f + origin.y));
        titleLabel->setColor(Color3B::YELLOW);
        this->addChild(titleLabel);
    }

    // 用户名输入框的白色背景
    auto usernameBackground = LayerColor::create(Color4B::WHITE, 320, 60);
    usernameBackground->setPosition(Vec2(visibleSize.width / 2 - 160 + origin.x, visibleSize.height * 0.6f - 30 + origin.y));
    this->addChild(usernameBackground);

    // 用户名输入框边框
    auto usernameBorder = DrawNode::create();
    usernameBorder->drawRect(Vec2(visibleSize.width / 2 - 160 + origin.x, visibleSize.height * 0.6f - 30 + origin.y),
        Vec2(visibleSize.width / 2 + 160 + origin.x, visibleSize.height * 0.6f + 30 + origin.y),
        Color4F::BLACK);
    this->addChild(usernameBorder);

    // 用户名输入框
    usernameField = ui::TextField::create("please input your username: ", "fonts/arial.ttf", 24);
    usernameField->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height * 0.6f + origin.y));
    usernameField->setContentSize(Size(300, 50));
    usernameField->setPlaceHolder("USERNAME");
    usernameField->setPlaceHolderColor(Color3B::GRAY);
    usernameField->setTextColor(Color4B::BLACK);
    this->addChild(usernameField);

    // 密码输入框的白色背景
    auto passwordBackground = LayerColor::create(Color4B::WHITE, 320, 60);
    passwordBackground->setPosition(Vec2(visibleSize.width / 2 - 160 + origin.x, visibleSize.height * 0.5f - 30 + origin.y));
    this->addChild(passwordBackground);

    // 密码输入框边框
    auto passwordBorder = DrawNode::create();
    passwordBorder->drawRect(Vec2(visibleSize.width / 2 - 160 + origin.x, visibleSize.height * 0.5f - 30 + origin.y),
        Vec2(visibleSize.width / 2 + 160 + origin.x, visibleSize.height * 0.5f + 30 + origin.y),
        Color4F::BLACK);
    this->addChild(passwordBorder);

    // 密码输入框
    passwordField = ui::TextField::create("please input your password", "fonts/arial.ttf", 24);
    passwordField->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height * 0.5f + origin.y));
    passwordField->setContentSize(Size(300, 50));
    passwordField->setPlaceHolder("PASSWORD");
    passwordField->setPlaceHolderColor(Color3B::GRAY);
    passwordField->setTextColor(Color4B::BLACK);
    passwordField->setPasswordEnabled(true); // 密码模式
    this->addChild(passwordField);


    // 登录按钮的白色背景
    auto buttonBackground = LayerColor::create(Color4B::WHITE, 160, 60);
    buttonBackground->setPosition(Vec2(visibleSize.width / 2 - 80 + origin.x, visibleSize.height * 0.35f - 30 + origin.y));
    this->addChild(buttonBackground);

    // 登录按钮边框
    auto buttonBorder = DrawNode::create();
    buttonBorder->drawRect(Vec2(visibleSize.width / 2 - 80 + origin.x, visibleSize.height * 0.35f - 30 + origin.y),
        Vec2(visibleSize.width / 2 + 80 + origin.x, visibleSize.height * 0.35f + 30 + origin.y),
        Color4F::BLACK);
    this->addChild(buttonBorder);

    // 登录按钮
    loginButton = ui::Button::create();
    loginButton->setTitleText("LOGIN");
    loginButton->setTitleFontSize(24);
    loginButton->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height * 0.35f + origin.y));
    loginButton->addTouchEventListener(CC_CALLBACK_2(LoginScene::onLoginButtonClicked, this));
    loginButton->setColor(Color3B::GREEN);
    this->addChild(loginButton);

    // 状态标签
    statusLabel = Label::createWithTTF("", "fonts/arial.ttf", 20);
    statusLabel->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height * 0.25f + origin.y));
    statusLabel->setColor(Color3B::RED);
    this->addChild(statusLabel);

    return true;
}

void LoginScene::onLoginButtonClicked(Ref* sender, ui::Widget::TouchEventType type)
{
    if (type == ui::Widget::TouchEventType::ENDED) {
        std::string username = usernameField->getString();
        std::string password = passwordField->getString();

        if (username.empty() || password.empty()) {
            statusLabel->setString("please input username and password : ");
            return;
        }

        if (performLogin(username, password)) {
            statusLabel->setString("Login Successful! Redirecting...");
            statusLabel->setColor(Color3B::GREEN);

            // 延迟1秒后进入村庄
            this->scheduleOnce([this](float dt) {
                this->enterVillageMap();
                }, 1.0f, "enter_village");
        }
        else {
            statusLabel->setString("Login Failed! Invalid username or password.");
            statusLabel->setColor(Color3B::RED);
        }
    }
}

bool LoginScene::performLogin(const std::string& username, const std::string& password)
{
    // 这里可以实现真实的登录验证逻辑
    // 目前只是简单的示例验证
    if (username == "admin" && password == "123456") {
        return true;
    }
    if (username == "player" && password == "password") {
        return true;
    }
    return false;
}

void LoginScene::enterVillageMap()
{
    // 获取村庄地图单例并添加到新场景
    auto villageMap = HomeVillageMap::getInstance();
    auto scene = Scene::create();
    scene->addChild(villageMap);

    // 使用过渡效果切换场景
    Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene));
}