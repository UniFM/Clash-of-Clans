#include "ResourceUILayer.h"
#include "GameManager.h"

// 单例初始化
ResourceUILayer* ResourceUILayer::_instance = nullptr;

ResourceUILayer::ResourceUILayer()
    : _goldLabel(nullptr)
    , _elixirLabel(nullptr)
    , _goldIcon(nullptr)
    , _elixirIcon(nullptr)
    , _resourceRootNode(nullptr)
{
}

ResourceUILayer::~ResourceUILayer()
{
    _instance = nullptr;
}

ResourceUILayer* ResourceUILayer::getInstance()
{
    if (!_instance)
    {
        _instance = ResourceUILayer::create();
        _instance->retain(); // 单例常驻内存
    }
    return _instance;
}

bool ResourceUILayer::init()
{
    if (!Layer::init())
    {
        return false;
    }

    // 创建UI根节点（统一管理所有资源UI）
    _resourceRootNode = Node::create();
    this->addChild(_resourceRootNode, 9999); // 层级置顶，永不遮挡

    // ===================== 1. 初始化UI位置（右上角） =====================
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 uiRootPos = Vec2(visibleSize.width - UI_OFFSET_X, visibleSize.height - UI_OFFSET_Y);
    _resourceRootNode->setPosition(uiRootPos);
    _resourceRootNode->setAnchorPoint(Vec2(1, 1)); // 锚点设为右上，适配屏幕更精准

    // ===================== 2. 创建金币UI（图标+文字） =====================
    Node* goldNode = Node::create();
    goldNode->setAnchorPoint(Vec2(1, 1));
    goldNode->setPosition(0, 0);
    _resourceRootNode->addChild(goldNode);

    // 金币图标（资源缺失时用黄色色块兜底）
    _goldIcon = Sprite::create("Icon/gold_icon.png");
    if (!_goldIcon)
    {
        _goldIcon = Sprite::create();
        _goldIcon->setColor(Color3B::YELLOW);
        _goldIcon->setContentSize(Size(ICON_SIZE, ICON_SIZE));
    }
    else
    {
        _goldIcon->setScale(ICON_SIZE / _goldIcon->getContentSize().width);
    }
    _goldIcon->setAnchorPoint(Vec2(1, 0.5));
    _goldIcon->setPosition(-ITEM_SPACING, 0);
    goldNode->addChild(_goldIcon);

    // 金币数值标签（黄色文字）
    _goldLabel = Label::createWithSystemFont("0", "Arial", LABEL_FONT_SIZE);
    _goldLabel->setTextColor(Color4B(255, 215, 0, 255)); // 金色
    _goldLabel->setAnchorPoint(Vec2(1, 0.5));
    _goldLabel->setPosition(-_goldIcon->getContentSize().width - ITEM_SPACING * 2, 0);
    goldNode->addChild(_goldLabel);

    // ===================== 3. 创建圣水UI（图标+文字） =====================
    Node* elixirNode = Node::create();
    elixirNode->setAnchorPoint(Vec2(1, 1));
    elixirNode->setPosition(0, -RES_SPACING); // 金币下方
    _resourceRootNode->addChild(elixirNode);

    // 圣水图标（资源缺失时用蓝色色块兜底）
    _elixirIcon = Sprite::create("Icon/elixir_icon.png");
    if (!_elixirIcon)
    {
        _elixirIcon = Sprite::create();
        _elixirIcon->setColor(Color3B::BLUE);
        _elixirIcon->setContentSize(Size(ICON_SIZE, ICON_SIZE));
    }
    else
    {
        _elixirIcon->setScale(ICON_SIZE / _elixirIcon->getContentSize().width);
    }
    _elixirIcon->setAnchorPoint(Vec2(1, 0.5));
    _elixirIcon->setPosition(-ITEM_SPACING, 0);
    elixirNode->addChild(_elixirIcon);

    // 圣水数值标签（蓝色文字）
    _elixirLabel = Label::createWithSystemFont("0", "Arial", LABEL_FONT_SIZE);
    _elixirLabel->setTextColor(Color4B(135, 206, 235, 255)); // 蓝色
    _elixirLabel->setAnchorPoint(Vec2(1, 0.5));
    _elixirLabel->setPosition(-_elixirIcon->getContentSize().width - ITEM_SPACING * 2, 0);
    elixirNode->addChild(_elixirLabel);

    // ===================== 4. 初始化数值+开启定时刷新 =====================
    refreshResourceNum();
    //this->schedule(schedule_selector(ResourceUILayer::refreshResourceNum), 0.1f); // 100ms刷新一次，实时同步

    CCLOG("ResourceUILayer: 金币圣水UI初始化完成，位置-右上角");
    return true;
}

//  核心方法：从GameManager读取数值并刷新UI
void ResourceUILayer::refreshResourceNum()
{
    if (!_goldLabel || !_elixirLabel) return;

    GameManager* gm = GameManager::getInstance();
    if (gm)
    {
        // 读取金币、圣水数值并格式化显示
        int gold = gm->getResource(ResourceType::GOLD);
        int elixir = gm->getResource(ResourceType::ELIXIR);
        _goldLabel->setString(StringUtils::format("%d", gold));
        _elixirLabel->setString(StringUtils::format("%d", elixir));
    }
}