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
    if (username == "1" && password == "1") {
        return true;
    }
    if (username == "player" && password == "password") {
        return true;
    }
    return false;
}

void LoginScene::enterVillageMap()
{
    CCLOG("LoginScene: Entering village map...");

    // 获取村庄地图单例并添加到新场景
    auto villageMap = HomeVillageMap::getInstance();

    if (!villageMap) {
        CCLOG("LoginScene: Failed to get HomeVillageMap instance!");
        // 显示错误信息给用户
        statusLabel->setString("Failed to load map! Please try again.");
        statusLabel->setColor(Color3B::RED);
        return;
    }

    CCLOG("LoginScene: HomeVillageMap instance created successfully");

    auto scene = Scene::create();

    if (!scene) {
        CCLOG("LoginScene: Failed to create scene!");
        return;
    }

    scene->addChild(villageMap);
    CCLOG("LoginScene: Scene created and map added, switching scene...");

    // 使用过渡效果切换场景
    auto transition = TransitionFade::create(1.0f, scene);
    Director::getInstance()->replaceScene(transition);
}

//// 用于场景切换的简单测试场景--测试成功，切换有效，问题定位在村庄TMX地图的加载上
//void LoginScene::enterVillageMap()
//{
//    CCLOG("LoginScene: Entering village map...");
//
//    // 创建一个简单的测试场景
//    auto scene = Scene::create();
//    if (!scene) {
//        CCLOG("LoginScene: Failed to create scene!");
//        statusLabel->setString("Failed to create scene!");
//        statusLabel->setColor(Color3B::RED);
//        return;
//    }
//
//    // 添加一个简单的绿色背景
//    auto visibleSize = Director::getInstance()->getVisibleSize();
//    auto greenBackground = LayerColor::create(Color4B(0, 128, 0, 255)); // 绿色背景
//    scene->addChild(greenBackground);
//
//    // 添加一个标签表示进入了村庄
//    auto villageLabel = Label::createWithTTF("Welcome to Home Village!", "fonts/arial.ttf", 36);
//    if (villageLabel) {
//        villageLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
//        villageLabel->setColor(Color3B::WHITE);
//        scene->addChild(villageLabel);
//    }
//
//    // 添加一个返回按钮
//    auto backButton = ui::Button::create();
//    if (backButton) {
//        backButton->setTitleText("Back to Login");
//        backButton->setTitleFontSize(20);
//        backButton->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 100));
//        backButton->setColor(Color3B::BLUE);
//        backButton->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type) {
//            if (type == ui::Widget::TouchEventType::ENDED) {
//                // 返回登录界面
//                auto loginScene = LoginScene::createScene();
//                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, loginScene));
//            }
//            });
//        scene->addChild(backButton);
//    }
//
//    CCLOG("LoginScene: Simple test scene created, switching...");
//
//    // 切换到测试场景
//    auto transition = TransitionFade::create(1.0f, scene);
//    Director::getInstance()->replaceScene(transition);
//}