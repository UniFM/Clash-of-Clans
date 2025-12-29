/*************************************************************
* @file     : ResourceUILayer.h
* @function ：金币圣水UI层头文件 - 定义资源显示UI核心接口与配置
* @author   : 齐颖
* @note     : 1.单例模式实现全局独立管理村庄界面右上角资源展示；
              2.复用GameManager数据，支持实时刷新资源数值；
              3.封装金币/圣水图标、数值标签等UI节点及样式配置常量；
              4.提供全局访问接口，支持外部/定时触发数值刷新
**************************************************************/

#pragma once
#include "cocos2d.h"
#include "GameManager.h"

USING_NS_CC;

/**
 * 金币圣水UI层 - 独立管理村庄界面右上角资源显示
 * 复用GameManager的资源数据，实时刷新、全局唯一
 */
class ResourceUILayer : public Layer
{
public:
    CREATE_FUNC(ResourceUILayer);
    virtual bool init() override;

    // 实时刷新资源数值（外部调用/定时调用均可）
    void refreshResourceNum();

    // 获取单例（全局快速访问）
    static ResourceUILayer* getInstance();

private:
    ResourceUILayer();
    ~ResourceUILayer();

    // 单例对象
    static ResourceUILayer* _instance;

    // UI节点
    Label* _goldLabel;       // 金币数值标签
    Label* _elixirLabel;     // 圣水数值标签
    Sprite* _goldIcon;       // 金币图标
    Sprite* _elixirIcon;     // 圣水图标
    Node* _resourceRootNode; // UI根节点（统一管理）

    // 样式配置（一键微调位置/大小/字体）
    const float UI_OFFSET_X = 20.0f;    // 右内边距
    const float UI_OFFSET_Y = 20.0f;    // 上内边距
    const float ICON_SIZE = 40.0f;      // 图标尺寸
    const float LABEL_FONT_SIZE = 30.0f;// 文字字号
    const float ITEM_SPACING = 15.0f;   // 图标-文字间距
    const float RES_SPACING = 60.0f;    // 金币-圣水垂直间距
};