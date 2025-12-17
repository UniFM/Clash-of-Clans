#pragma once
/*************************************************************
* @file     : Control.h
* @function ：控制功能类
* @author   : 叶芷含
* @note     ：包含: 
*					1. 画面缩放功能（地图缩放）
*					2. 窗口缩放功能
*					3. 滚轮和触摸控制
**************************************************************/

#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "cocos2d.h"

USING_NS_CC;

class SceneMap;

class Control : public Node {
public:
    // 单例模式
    static Control* getInstance();
    static void destroyInstance();
    
    // 创建控制器
    static Control* create();
    
    // 初始化
    virtual bool init() override;
    
    // 设置目标地图（用于缩放控制）
    void setTargetMap(SceneMap* map);
    
    // 画面缩放功能
    void enableMapZoom(bool enable = true);
    void setZoomRange(float minZoom, float maxZoom);
    void setMapZoom(float zoom);
    float getMapZoom() const;
    void zoomMapIn();   // 放大
    void zoomMapOut();  // 缩小
    
    // 窗口缩放功能
    void enableWindowResize(bool enable = true);
    void setWindowSize(const Size& size);
    Size getWindowSize() const;
    void setAutoFitMap(bool autoFit = true);  // 地图自动横向填满
    
    // 滚轮控制
    void enableScrollWheel(bool enable = true);
    
    // 触摸手势控制（双指缩放）
    void enablePinchGesture(bool enable = true);
    
    // 重置所有设置
    void reset();

protected:
    // 构造函数和析构函数
    Control();
    virtual ~Control();
    
    // 事件处理
    void setupEventListeners();
    void removeEventListeners();
    
    // 鼠标滚轮事件
    void onMouseScroll(Event* event);
    
    // 键盘事件（用于快捷键）
    void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
    void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);
    
    // 窗口大小改变事件（需要平台特定实现）
    void onWindowResize(void* window, int width, int height);
    
    // 触摸事件（多点触摸）
    void onTouchesBegan(const std::vector<Touch*>& touches, Event* event);
    void onTouchesMoved(const std::vector<Touch*>& touches, Event* event);
    void onTouchesEnded(const std::vector<Touch*>& touches, Event* event);
    
    // 内部功能函数
    void updateMapZoom();
    void updateWindowSize();
    void autoFitMapToWindow();
    float calculateDistance(const Vec2& point1, const Vec2& point2);

private:
    static Control* sInstance;
    
    // 目标地图
    SceneMap* targetMap;
    
    // 缩放相关
    bool zoomEnabled;
    float currentZoom;
    float minZoom;
    float maxZoom;
    float zoomStep;
    
    // 窗口相关
    bool windowResizeEnabled;
    bool autoFitMap;
    Size originalWindowSize;
    Size currentWindowSize;
    
    // 事件监听器
    EventListenerMouse* mouseListener;
    EventListenerKeyboard* keyboardListener;
    EventListenerTouchAllAtOnce* touchListener;
    
    // 多点触摸缩放
    bool pinchEnabled;
    bool isPinching;
    float lastPinchDistance;
    Vec2 pinchCenter;
    
    // 窗口指针
    void* glfwWindow;
};

#endif
