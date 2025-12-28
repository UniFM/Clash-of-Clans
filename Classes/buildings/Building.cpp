/*************************************************************
* @file     : Building.cpp
* @function ：建筑基类实现 - 部落冲突建筑系统
* @author   : 齐颖
* @note     ：基类通用方法实现，仅在此文件存在，无子类专属配置
**************************************************************/

#include "Map/HomeVillageMap.h"
#include "BuildingsData.h"

USING_NS_CC;

// 菜单按钮通用配置（基类共用，统一定义）
#define UPGRADE_BTN_NORMAL "buildings/upgrademenu.png"
#define UPGRADE_BTN_PRESSED "upgrade_btn_pressed.png"
#define CANCEL_BTN_NORMAL "buildings/cancelbtn.png"
#define CANCEL_BTN_PRESSED "buildings/cancelbtn_pressed.png"
#define INFO_BTN_NORMAL "buildings/informationmenu.png"
#define INFO_BTN_PRESSED "buildings/infocaidan_pressed.png"
#define BTN_SPACING 60
#define DRAG_THRESHOLD 5.0f

void Building::showMenuImmediately() {
    if (_isInfoShow) return; // 若建筑信息面板已显示，则不再弹出菜单
    _isMenuShow = true;
    if (_upgradeMenu) {
        _upgradeMenu->setVisible(true);
        _upgradeMenu->setLocalZOrder(2000); // 确保菜单显示在最顶层
        // _upgradeMenu->setPosition(m_menuOffset); // 可选：确保菜单位置准确
    }
}
// ------------------------ 建筑基类构造与析构 ------------------------
Building::Building()
    : _type(BuildingType::TOWN_HALL)
    , _level(1)
    , _buildingSize(Size::ZERO)
    , _isDragging(false)
    , _isMenuShow(false)
    , _isInfoShow(false)
    , _dragThreshold(DRAG_THRESHOLD)
    , _touchListener(nullptr)
    , _infoTouchListener(nullptr)
    , _upgradeMenu(nullptr)
    , _upgradeBtn(nullptr)
    , _cancelBtn(nullptr)
    , _infoBtn(nullptr)
    , _infoLayer(nullptr)
    , _infoImage(nullptr)
    , m_menuOffset(Vec2::ZERO)
    , m_infoImageOffset(Vec2::ZERO)
{
}

Building::~Building() {
    // 安全移除触摸监听，避免野指针
    if (_touchListener) {
        _eventDispatcher->removeEventListener(_touchListener);
        _touchListener = nullptr;
    }
    if (_infoTouchListener) {
        _eventDispatcher->removeEventListener(_infoTouchListener);
        _infoTouchListener = nullptr;
    }
}

// ------------------------ 建筑基类通用方法实现 ------------------------
bool Building::upgradeBuilding() {
    CCLOG("【建筑基类】该建筑未实现自定义升级逻辑，等级：%d，类型：%d", _level, (int)_type);
    return false;
}

void Building::removeBuilding() {
    CCLOG("【建筑】删除建筑，类型：%d，等级：%d", (int)_type, _level);
    this->closeUpgradeMenu();
    this->hideBuildingInfo();
    this->removeFromParentAndCleanup(true);
}

void Building::showBuildingInfo() {
    if (!_infoLayer || !_upgradeMenu || _isInfoShow) {
        CCLOG("【信息菜单】信息层未初始化或已显示，跳过显示");
        return;
    }

    // 额外校验：确保信息图片已挂载到信息层（升级后可能丢失关联）
    if (_infoImage && _infoImage->getParent() != _infoLayer) {
        _infoLayer->addChild(_infoImage);
        CCLOG("【信息菜单】信息图片重新挂载到信息层");
    }

    // 互斥状态切换：隐藏菜单，显示信息层
    _isMenuShow = false;
    _upgradeMenu->setVisible(false);

    _isInfoShow = true;
    _infoLayer->setVisible(true);
    _infoLayer->setLocalZOrder(1000); // 强制置顶，避免被任何节点遮挡
    if (_infoImage) {
        _infoImage->setLocalZOrder(1001);
    }

    CCLOG("【信息菜单】信息图片已叠加显示，点击图片可返回菜单");
}

void Building::hideBuildingInfo() {
    if (!_infoLayer || !_upgradeMenu || !_isInfoShow) {
        CCLOG("【信息菜单】信息层未初始化或未显示，跳过隐藏");
        return;
    }

    // 互斥状态切换：隐藏信息层，显示菜单
    _isInfoShow = false;
    _infoLayer->setVisible(false);

    _isMenuShow = true;
    _upgradeMenu->setVisible(true);

    CCLOG("【信息菜单】信息图片已隐藏，返回菜单界面");
}

bool Building::init(BuildingType type, int level) {
    if (!Sprite::init()) {
        return false;
    }

    _type = type;
    _level = level;

    // 按顺序初始化：建筑资源→触摸监听→菜单→信息层
    if (!loadBuildingRes()) {
        return false;
    }
    initTouchListener();
    createUpgradeMenu();
    createInfoLayer();

    return true;
}

void Building::setBuildingPosition(const Vec2& worldPos) {
    this->setPosition(worldPos);
    _originalPos = worldPos;

    // 同步菜单和信息层位置，跟随建筑移动（使用成员变量）
    if (_upgradeMenu) {
        _upgradeMenu->setPosition(m_menuOffset);
    }
    if (_infoLayer) {
        _infoLayer->setPosition(m_infoImageOffset);
    }
}

bool Building::onTouchBegan(Touch* touch, Event* event) {
    Vec2 touchWorldPos = touch->getLocation();
    // 信息层显示时，不响应建筑点击/拖动
    if (_isInfoShow) {
        return false;
    }

    if (this->getBoundingBox().containsPoint(touchWorldPos)) {
        _touchOffset = this->getPosition() - touchWorldPos;
        _touchStartPos = touchWorldPos;
        _isDragging = false;
        this->setLocalZOrder(200); // 拖动时建筑置顶
        return true;
    }
    return false;
}

void Building::onTouchMoved(Touch* touch, Event* event) {
    Vec2 touchWorldPos = touch->getLocation();
    float distance = touchWorldPos.distance(_touchStartPos);

    if (distance > _dragThreshold) {
        _isDragging = true;
        // 拖动时自动关闭菜单和信息层
        this->closeUpgradeMenu();
        this->hideBuildingInfo();

        // 更新建筑及附属节点位置（使用成员变量）
        Vec2 newPos = touchWorldPos + _touchOffset;
        this->setPosition(newPos);
        if (_upgradeMenu) {
            _upgradeMenu->setPosition(m_menuOffset);
        }
        if (_infoLayer) {
            _infoLayer->setPosition(m_infoImageOffset);
        }
    }
}

void Building::onTouchEnded(Touch* touch, Event* event) {
    this->setLocalZOrder(100); // 恢复默认层级

    // 非拖动操作且信息层未显示时，切换菜单
    if (!_isDragging && !_isInfoShow) {
        this->toggleUpgradeMenu();
    }
}

void Building::toggleUpgradeMenu() {
    if (!_upgradeMenu || _isInfoShow) {
        CCLOG("【菜单】信息图片显示中，禁止切换菜单");
        return;
    }

    _isMenuShow = !_isMenuShow;
    _upgradeMenu->setVisible(_isMenuShow);
    CCLOG("【菜单】%s菜单", _isMenuShow ? "显示" : "隐藏");
}

void Building::closeUpgradeMenu() {
    if (_upgradeMenu && _isMenuShow) {
        _isMenuShow = false;
        _upgradeMenu->setVisible(false);
        CCLOG("【菜单】强制关闭升级菜单");
    }
}

void Building::initTouchListener() {
    _touchListener = EventListenerTouchOneByOne::create();
    if (!_touchListener) {
        CCLOG("【触摸】创建建筑触摸监听失败");
        return;
    }

    _touchListener->setSwallowTouches(true);
    _touchListener->onTouchBegan = CC_CALLBACK_2(Building::onTouchBegan, this);
    _touchListener->onTouchMoved = CC_CALLBACK_2(Building::onTouchMoved, this);
    _touchListener->onTouchEnded = CC_CALLBACK_2(Building::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_touchListener, this);
}

void Building::createUpgradeMenu() {
    _upgradeMenu = Node::create();
    if (!_upgradeMenu) {
        CCLOG("【菜单】创建菜单根节点失败");
        return;
    }
    _upgradeMenu->setPosition(m_menuOffset); // 使用成员变量
    _upgradeMenu->setVisible(false);
    _upgradeMenu->setScale(0.4f);
    _upgradeMenu->setLocalZOrder(300); // 菜单层级低于信息层
    this->addChild(_upgradeMenu);

    // 升级按钮
    _upgradeBtn = MenuItemImage::create(
        UPGRADE_BTN_NORMAL,
        UPGRADE_BTN_PRESSED,
        CC_CALLBACK_0(Building::upgradeBuilding, this)
    );
    if (!_upgradeBtn) {
        CCLOG("【菜单】创建升级按钮失败");
        return;
    }

    // 取消按钮
    _cancelBtn = MenuItemImage::create(
        CANCEL_BTN_NORMAL,
        CANCEL_BTN_PRESSED,
        CC_CALLBACK_0(Building::removeBuilding, this)
    );
    if (!_cancelBtn) {
        CCLOG("【菜单】创建取消按钮失败");
        return;
    }

    // 信息按钮：绑定显示信息层方法
    _infoBtn = MenuItemImage::create(
        INFO_BTN_NORMAL,
        INFO_BTN_PRESSED,
        CC_CALLBACK_0(Building::showBuildingInfo, this)
    );
    if (!_infoBtn) {
        CCLOG("【菜单】创建信息按钮失败");
        return;
    }

    // 横向排列按钮，自动对齐
    Menu* menu = Menu::create(_infoBtn, _upgradeBtn, _cancelBtn, nullptr);
    if (!menu) {
        CCLOG("【菜单】创建菜单容器失败");
        return;
    }
    menu->setPosition(Vec2::ZERO);
    menu->alignItemsHorizontallyWithPadding(BTN_SPACING);
    _upgradeMenu->addChild(menu);

    CCLOG("【菜单】三按钮菜单创建成功");
}

void Building::createInfoLayer() {
    // 1. 创建信息层根节点
    _infoLayer = Node::create();
    if (!_infoLayer) {
        CCLOG("【信息层】创建根节点失败");
        return;
    }
    _infoLayer->setPosition(m_infoImageOffset); // 使用成员变量
    _infoLayer->setVisible(false);
    _infoLayer->setLocalZOrder(1000); // 置顶层级，确保叠加显示
    this->addChild(_infoLayer);

    // 2. 初始化信息图片（由子类加载资源）
    if (!_infoImage) {
        loadBuildingInfoRes(); // 确保图片被创建
    }
    if (_infoImage && _infoImage->getParent() == nullptr) {
        _infoLayer->addChild(_infoImage);
    }

    // 3. 创建触摸监听：监听整个信息层，升级后仍有效（绑定到信息层而非图片）
    _infoTouchListener = EventListenerTouchOneByOne::create();
    if (!_infoTouchListener) {
        CCLOG("【信息层】创建触摸监听失败");
        return;
    }

    _infoTouchListener->setSwallowTouches(true); // 吞噬触摸，不传递给建筑

    // 触摸开始：只要信息层显示，就响应点击
    _infoTouchListener->onTouchBegan = [this](Touch* touch, Event* event) {
        if (_isInfoShow && _infoLayer->isVisible()) {
            CCLOG("【信息层】检测到点击");
            return true;
        }
        return false;
        };

    // 触摸结束：隐藏信息层，返回菜单
    _infoTouchListener->onTouchEnded = [this](Touch* touch, Event* event) {
        this->hideBuildingInfo();
        };

    // 关键：监听注册到信息层（固定节点）
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_infoTouchListener, _infoLayer);

    CCLOG("【信息层】信息界面创建成功，支持升级后点击返回菜单");
}

void Building::setGridPosition(int gridX, int gridY)
{
    _gridX = gridX;
    _gridY = gridY;

    // 注意：实际的世界坐标位置需要由MapLayer来设置
    // 这里只保存网格坐标
}