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
