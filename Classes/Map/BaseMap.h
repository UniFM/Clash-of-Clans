/*************************************************************
* @file     : BaseMap.h
* @function ：地图基类 - 定义地图核心功能接口与核心成员
* @author   : 叶芷含
* @note     : 1.封装地图坐标转换、缩放、移动等核心操作；
*             2.提供建筑放置校验、占位标记、网格管理接口；
*             3.包含触摸拖拽、地图边界限制逻辑；
*             4.区分建筑/部队容器节点，管理地图可视化与交互基础；
*             5.定义地图尺寸、网格数量、草地边界等常量参数，适配坐标计算逻辑
**************************************************************/


#ifndef __BASE_MAP_H__
#define __BASE_MAP_H__

#include "cocos2d.h"

// 前向声明
namespace cocos2d {
    class Layer;
    class Sprite;
    class Vec2;
    class Size;
    class Rect;
    class Touch;
    class Event;
    class Node;
}

class BaseMap : public cocos2d::Layer
{
public:
    BaseMap();
    virtual ~BaseMap();

    virtual bool init(const std::string& mapImagePath);

    // 坐标转换
    cocos2d::Vec2 worldToGrid(const cocos2d::Vec2& worldPos) const;
    cocos2d::Vec2 gridToWorld(int gridX, int gridY) const;
    bool isValidGrid(int gridX, int gridY) const;

    // 获取方法
    cocos2d::Rect getMapBounds() const { return _mapBounds; }
    cocos2d::Rect getGrassBounds() const { return _grassBounds; }
    float getZoom() const { return _zoom; }
    cocos2d::Vec2 getMapPosition() const { return _mapPosition; }
    cocos2d::Node* getBuildingsContainer() const { return _buildingsContainer; }
    cocos2d::Node* getTroopsContainer() const { return _troopsContainer; }

    // 设置 / 动作处理
    void setZoom(float zoom);
    void moveTo(const cocos2d::Vec2& position);
    void updateMapTransform();

    // 建筑管理
    virtual void addBuilding(cocos2d::Node* building);
    virtual void removeBuilding(cocos2d::Node* building);
    bool canPlaceBuilding(int gridX, int gridY, cocos2d::Size gridSize) const;
    void markGridsOccupied(int gridX, int gridY, cocos2d::Size gridSize, bool occupied);
    std::vector<cocos2d::Vec2> getOccupiedGrids(int gridX, int gridY, cocos2d::Size gridSize) const;

    // 常量 （后续移入Constant.h）
    float getMapWidth() const { return _mapWidth; }
    float getMapHeight() const { return _mapHeight; }
    float getGrassRectWidth() const { return _grassRectWidth; }
    float getGrassRectHeight() const { return _grassRectHeight; }
    float getGrassOffsetX() const { return _grassOffsetX; }
    float getGrassOffsetY() const { return _grassOffsetY; }
    int getGridCols() const { return _gridCols; }
    int getGridRows() const { return _gridRows; }

protected:
    // 触摸事件
    virtual bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    virtual void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    virtual void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

    void initOccupiedGrids();

    cocos2d::Sprite* _mapSprite;
    float _zoom;
    cocos2d::Vec2 _mapPosition;
    cocos2d::Vec2 _lastTouchPos;
    bool _isDragging;

    cocos2d::Rect _mapBounds;
    cocos2d::Rect _grassBounds;

    float _gridCellWidth;
    float _gridCellHeight;

    // 地图参数
    float _mapWidth;
    float _mapHeight;
    float _grassRectWidth;
    float _grassRectHeight;
    float _grassOffsetX;
    float _grassOffsetY;
    int _gridCols;
    int _gridRows;

    cocos2d::Node* _buildingsContainer;
    cocos2d::Node* _troopsContainer;  // 新增：部队容器
    std::vector<std::vector<bool>> _occupiedGrids;
};

#endif // __BASE_MAP_H__
