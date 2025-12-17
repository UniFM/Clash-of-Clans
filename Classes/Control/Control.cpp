/*************************************************************
* @file     : Control.cpp
* @function ：控制功能
* @author   : 叶芷含
* @note     ：包含: 
*					1. 画面缩放功能
*					2. 窗口缩放功能
**************************************************************/

#include "Control.h"
#include "Map/SceneMap.h"

USING_NS_CC;

// 静态实例指针
Control* Control::sInstance = nullptr;

// 单例模式获取实例
Control* Control::getInstance() {
    if (sInstance == nullptr) {
        sInstance = Control::create();
        if (sInstance) {
            sInstance->retain(); // 保持引用
        }
    }
    return sInstance;
}

// 销毁单例实例
void Control::destroyInstance() {
    if (sInstance) {
        sInstance->release();
        sInstance = nullptr;
    }
}

// 创建控制器
Control* Control::create() {
    Control* control = new (std::nothrow) Control();
    if (control && control->init()) {
        control->autorelease();
        return control;
    }
    CC_SAFE_DELETE(control);
    return nullptr;
}

// 构造函数
Control::Control()
    : targetMap(nullptr)
    , zoomEnabled(true)
    , currentZoom(1.0f)
    , minZoom(0.5f)
    , maxZoom(3.0f)
    , zoomStep(0.1f)
    , windowResizeEnabled(true)
    , autoFitMap(true)
    , mouseListener(nullptr)
    , keyboardListener(nullptr)
    , touchListener(nullptr)
    , pinchEnabled(true)
    , isPinching(false)
    , lastPinchDistance(0.0f) {
    
    // 获取原始窗口大小
    auto director = Director::getInstance();
    originalWindowSize = director->getWinSize();
    currentWindowSize = originalWindowSize;
}

// 析构函数
Control::~Control() {
    removeEventListeners();
}

// 初始化
bool Control::init() {
    if (!Node::init()) {
        return false;
    }
    
    setupEventListeners();
    return true;
}

// 设置目标地图
void Control::setTargetMap(SceneMap* map) {
    targetMap = map;
}

// 启用画面缩放
void Control::enableMapZoom(bool enable) {
    zoomEnabled = enable;
}

// 设置缩放范围
void Control::setZoomRange(float minZoom, float maxZoom) {
    minZoom = std::max(0.1f, minZoom);
    maxZoom = std::max(minZoom, maxZoom);
    
    // 确保当前缩放值在范围内
    currentZoom = std::max(minZoom, std::min(maxZoom, currentZoom));
    updateMapZoom();
}

// 设置地图缩放
void Control::setMapZoom(float zoom) {
    if (!zoomEnabled) return;
    
    float newZoom = std::max(minZoom, std::min(maxZoom, zoom));
    if (newZoom != currentZoom) {
        currentZoom = newZoom;
        updateMapZoom();
    }
}

// 获取当前缩放
float Control::getMapZoom() const {
    return currentZoom;
}

// 放大地图
void Control::zoomMapIn() {
    setMapZoom(currentZoom + zoomStep);
}

// 缩小地图
void Control::zoomMapOut() {
    setMapZoom(currentZoom - zoomStep);
}

// 更新地图缩放
void Control::updateMapZoom() {
    if (targetMap && zoomEnabled) {
        targetMap->setScale(currentZoom);
        
        // 如果启用了自动适应，重新调整位置
        if (autoFitMap) {
            autoFitMapToWindow();
        }
        
        CCLOG("Map zoom updated to: %.2f", currentZoom);
    }
}

// 启用窗口缩放
void Control::enableWindowResize(bool enable) {
    windowResizeEnabled = enable;
}

// 设置窗口大小
void Control::setWindowSize(const Size& size) {
    if (!windowResizeEnabled) return;
    
    auto director = Director::getInstance();
    auto glView = director->getOpenGLView();
    
    if (glView) {
        glView->setFrameSize(size.width, size.height);
        currentWindowSize = size;
        
        // 更新设计分辨率
        director->getOpenGLView()->setDesignResolutionSize(
            size.width, size.height, ResolutionPolicy::SHOW_ALL);
        
        if (autoFitMap) {
            autoFitMapToWindow();
        }
        
        CCLOG("Window size set to: %.0fx%.0f", size.width, size.height);
    }
}

// 获取当前窗口大小
Size Control::getWindowSize() const {
    return currentWindowSize;
}

// 设置自动适应
void Control::setAutoFitMap(bool autoFit) {
    autoFitMap = autoFit;
    if (autoFitMap) {
        autoFitMapToWindow();
    }
}

// 自动适应地图到窗口
void Control::autoFitMapToWindow() {
    if (!targetMap || !autoFitMap) return;
    
    auto director = Director::getInstance();
    Size visibleSize = director->getVisibleSize();
    Vec2 origin = director->getVisibleOrigin();
    
    // 获取地图大小
    Size mapSize = targetMap->getContentSize();
    if (mapSize.width <= 0 || mapSize.height <= 0) return;
    
    // 计算横向填满所需的缩放比例
    float scaleX = visibleSize.width / mapSize.width;
    
    // 如果没有人为缩放，使用自动计算的缩放
    if (currentZoom == 1.0f) {
        currentZoom = scaleX;
        targetMap->setScale(currentZoom);
    }
    
    // 居中显示
    Vec2 mapPos;
    mapPos.x = origin.x + (visibleSize.width - mapSize.width * currentZoom) / 2;
    mapPos.y = origin.y + (visibleSize.height - mapSize.height * currentZoom) / 2;
    
    targetMap->setPosition(mapPos);
    
    CCLOG("Auto fit map - Scale: %.2f, Position: (%.0f, %.0f)", 
          currentZoom, mapPos.x, mapPos.y);
}

// 启用滚轮控制
void Control::enableScrollWheel(bool enable) {
    // 鼠标监听器会在setupEventListeners中设置
    if (mouseListener) {
        mouseListener->setEnabled(enable);
    }
}

// 启用双指缩放
void Control::enablePinchGesture(bool enable) {
    pinchEnabled = enable;
    if (touchListener) {
        touchListener->setEnabled(enable);
    }
}

// 重置设置
void Control::reset() {
    currentZoom = 1.0f;
    currentWindowSize = originalWindowSize;
    updateMapZoom();
    setWindowSize(originalWindowSize);
}

// 设置事件监听器
void Control::setupEventListeners() {
    auto director = Director::getInstance();
    auto dispatcher = director->getEventDispatcher();
    
    // 鼠标滚轮事件
    mouseListener = EventListenerMouse::create();
    mouseListener->onMouseScroll = CC_CALLBACK_1(Control::onMouseScroll, this);
    dispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
    
    // 键盘事件
    keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = CC_CALLBACK_2(Control::onKeyPressed, this);
    keyboardListener->onKeyReleased = CC_CALLBACK_2(Control::onKeyReleased, this);
    dispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);
    
    // 多点触摸事件
    touchListener = EventListenerTouchAllAtOnce::create();
    touchListener->onTouchesBegan = CC_CALLBACK_2(Control::onTouchesBegan, this);
    touchListener->onTouchesMoved = CC_CALLBACK_2(Control::onTouchesMoved, this);
    touchListener->onTouchesEnded = CC_CALLBACK_2(Control::onTouchesEnded, this);
    dispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
}

// 移除事件监听器
void Control::removeEventListeners() {
    auto dispatcher = Director::getInstance()->getEventDispatcher();
    
    if (mouseListener) {
        dispatcher->removeEventListener(mouseListener);
        mouseListener = nullptr;
    }
    
    if (keyboardListener) {
        dispatcher->removeEventListener(keyboardListener);
        keyboardListener = nullptr;
    }
    
    if (touchListener) {
        dispatcher->removeEventListener(touchListener);
        touchListener = nullptr;
    }
}

// 鼠标滚轮事件处理
void Control::onMouseScroll(Event* event) {
    if (!zoomEnabled) return;
    
    EventMouse* mouseEvent = static_cast<EventMouse*>(event);
    float scrollY = mouseEvent->getScrollY();
    
    // 根据滚轮方向进行缩放
    if (scrollY > 0) {
        zoomMapIn();
    } else if (scrollY < 0) {
        zoomMapOut();
    }
}

// 键盘按下事件
void Control::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event) {
    switch (keyCode) {
        case EventKeyboard::KeyCode::KEY_EQUAL:  // + 键放大
            zoomMapIn();
            break;
            
        case EventKeyboard::KeyCode::KEY_MINUS:  // - 键缩小
            zoomMapOut();
            break;
            
        case EventKeyboard::KeyCode::KEY_0:      // 0 键重置
            setMapZoom(1.0f);
            break;
            
        case EventKeyboard::KeyCode::KEY_F11:    // F11 切换全屏
            // 这里可以添加全屏切换逻辑
            break;
            
        default:
            break;
    }
}

// 键盘释放事件
void Control::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event) {
    // 预留接口
}

// 窗口大小改变事件
void Control::onWindowResize(void* window, int width, int height) {
    if (!windowResizeEnabled) return;
    
    currentWindowSize = Size(width, height);
    
    // 更新OpenGL视图
    auto director = Director::getInstance();
    auto glView = director->getOpenGLView();
    if (glView) {
        glView->setFrameSize(width, height);
    }
    
    if (autoFitMap) {
        // 延迟调用，确保窗口大小更新完成
        this->scheduleOnce([this](float dt) {
            autoFitMapToWindow();
        }, 0.1f, "autoFitDelay");
    }
    
    CCLOG("Window resized to: %dx%d", width, height);
}

// 多点触摸开始
void Control::onTouchesBegan(const std::vector<Touch*>& touches, Event* event) {
    if (!pinchEnabled || touches.size() != 2) {
        isPinching = false;
        return;
    }
    
    isPinching = true;
    
    Vec2 point1 = touches[0]->getLocation();
    Vec2 point2 = touches[1]->getLocation();
    
    lastPinchDistance = calculateDistance(point1, point2);
    pinchCenter = Vec2((point1.x + point2.x) / 2, (point1.y + point2.y) / 2);
}

// 多点触摸移动
void Control::onTouchesMoved(const std::vector<Touch*>& touches, Event* event) {
    if (!isPinching || !pinchEnabled || touches.size() != 2) return;
    
    Vec2 point1 = touches[0]->getLocation();
    Vec2 point2 = touches[1]->getLocation();
    
    float currentDistance = calculateDistance(point1, point2);
    
    if (lastPinchDistance > 0) {
        float scale = currentDistance / lastPinchDistance;
        float newZoom = currentZoom * scale;
        setMapZoom(newZoom);
    }
    
    lastPinchDistance = currentDistance;
}

// 多点触摸结束
void Control::onTouchesEnded(const std::vector<Touch*>& touches, Event* event) {
    isPinching = false;
    lastPinchDistance = 0.0f;
}

// 计算两点距离
float Control::calculateDistance(const Vec2& point1, const Vec2& point2) {
    return point1.distance(point2);
}

