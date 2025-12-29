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

#include "BaseMap.h"

USING_NS_CC;

BaseMap::BaseMap()
    : _mapSprite(nullptr)
    , _zoom(1.0f)
    , _mapPosition(Vec2::ZERO)
    , _lastTouchPos(Vec2::ZERO)
    , _isDragging(false)
    , _gridCellWidth(0.0f)
    , _gridCellHeight(0.0f)
    , _buildingsContainer(nullptr)
    , _troopsContainer(nullptr)  // 初始化部队容器
    , _mapWidth(0.0f)
    , _mapHeight(0.0f)
    , _grassRectWidth(0.0f)
    , _grassRectHeight(0.0f)
    , _grassOffsetX(0.0f)
    , _grassOffsetY(0.0f)
    , _gridCols(0)
    , _gridRows(0)
{
}

BaseMap::~BaseMap()
{
}

bool BaseMap::init(const std::string& mapImagePath)
{
    if (!Layer::init())
    {
        return false;
    }

    // 加载地图图像
    if (!mapImagePath.empty())
    {
        _mapSprite = Sprite::create(mapImagePath);
    }

    // 图片加载失败时的占位符
    if (!_mapSprite)
    {
        auto placeholder = DrawNode::create();
        Vec2 vertices[4] = {
            Vec2(0, 0),
            Vec2(_mapWidth, 0),
            Vec2(_mapWidth, _mapHeight),
            Vec2(0, _mapHeight)
        };
        placeholder->drawPoly(vertices, 4, true, Color4F(0.2f, 0.6f, 0.2f, 1.0f));

        // 草地
        Vec2 grassVertices[4] = {
            Vec2(_grassOffsetX, _grassOffsetY),
            Vec2(_grassOffsetX + _grassRectWidth, _grassOffsetY),
            Vec2(_grassOffsetX + _grassRectWidth, _grassOffsetY + _grassRectHeight),
            Vec2(_grassOffsetX, _grassOffsetY + _grassRectHeight)
        };
        placeholder->drawPoly(grassVertices, 4, true, Color4F(0.4f, 0.8f, 0.4f, 1.0f));

        // 菱形网格
        float rectLeft = _grassOffsetX;
        float rectRight = _grassOffsetX + _grassRectWidth;
        float rectTop = _grassOffsetY;
        float rectBottom = _grassOffsetY + _grassRectHeight;
        float centerX = _grassOffsetX + _grassRectWidth / 2;
        float centerY = _grassOffsetY + _grassRectHeight / 2;

        Vec2 diamondVertices[4] = {
            Vec2(centerX, rectTop),
            Vec2(rectRight, centerY),
            Vec2(centerX, rectBottom),
            Vec2(rectLeft, centerY)
        };
        placeholder->drawPoly(diamondVertices, 4, false, Color4F(1.0f, 1.0f, 0.0f, 1.0f));

        this->addChild(placeholder);
    }
    else
    {
        _mapSprite->setAnchorPoint(Vec2(0, 0));
        _mapSprite->setPosition(Vec2::ZERO);
        this->addChild(_mapSprite);
    }

    _mapBounds = Rect(0, 0, _mapWidth, _mapHeight);
    _grassBounds = Rect(_grassOffsetX, _grassOffsetY, _grassRectWidth, _grassRectHeight);

    if (_gridCols > 0 && _gridRows > 0) {
        _gridCellWidth = _grassRectWidth / _gridCols;
        _gridCellHeight = _grassRectHeight / _gridRows;
    } else {
        _gridCellWidth = 0;
        _gridCellHeight = 0;
    }

    _buildingsContainer = Node::create();
    _buildingsContainer->setAnchorPoint(Vec2(0, 0));
    _buildingsContainer->setPosition(Vec2::ZERO);
    this->addChild(_buildingsContainer, 10);

    // {update} 创建部队容器
    _troopsContainer = Node::create();
    _troopsContainer->setAnchorPoint(Vec2(0, 0));
    _troopsContainer->setPosition(Vec2::ZERO);
    this->addChild(_troopsContainer, 100); // 部队在建筑之上

    initOccupiedGrids();

    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    // 对地图尺寸进行安全检查，以避免除以零的情况
    if (_mapWidth > 0 && _mapHeight > 0) {
        float scaleX = visibleSize.width / _mapWidth;
        float scaleY = visibleSize.height / _mapHeight;
        _zoom = std::min(scaleX, scaleY) * 0.8f;

        _mapPosition = Vec2((visibleSize.width - _mapWidth * _zoom) / 2,
            (visibleSize.height - _mapHeight * _zoom) / 2);
    } else {
        _zoom = 1.0f;
        _mapPosition = Vec2::ZERO;
    }

    updateMapTransform();

    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(BaseMap::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(BaseMap::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(BaseMap::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseScroll = [this](Event* event) {
        EventMouse* mouseEvent = static_cast<EventMouse*>(event);
        float scrollY = mouseEvent->getScrollY();
        float newZoom = _zoom * (1.0f + scrollY * 0.1f);
        setZoom(newZoom);
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
#endif

    return true;
}

bool BaseMap::onTouchBegan(Touch* touch, Event* event)
{
    _lastTouchPos = touch->getLocation();
    _isDragging = false;
    return true; // 基础地图默认设为 true
}

void BaseMap::onTouchMoved(Touch* touch, Event* event)
{
    // 基本实现允许始终进行拖动，子类可以重写
    _isDragging = true; 

    Vec2 currentPos = touch->getLocation();
    Vec2 delta = currentPos - _lastTouchPos;

    _mapPosition += delta;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    float minX = visibleSize.width - _mapWidth * _zoom;
    float minY = visibleSize.height - _mapHeight * _zoom;
    
    _mapPosition.x = clampf(_mapPosition.x, minX, 0);
    _mapPosition.y = clampf(_mapPosition.y, minY, 0);

    updateMapTransform();
    _lastTouchPos = currentPos;
}

void BaseMap::onTouchEnded(Touch* touch, Event* event)
{
    _isDragging = false;
}

void BaseMap::setZoom(float zoom)
{
    zoom = clampf(zoom, 0.3f, 3.0f);
    if (_zoom == zoom) return;

    float oldZoom = _zoom;
    _zoom = zoom;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 screenCenter = Vec2(visibleSize.width / 2, visibleSize.height / 2);

    Vec2 worldPosBefore = (_mapPosition - screenCenter) / oldZoom + screenCenter;
    Vec2 worldPosAfter = (_mapPosition - screenCenter) / _zoom + screenCenter;

    _mapPosition += (worldPosBefore - worldPosAfter);

    float minX = visibleSize.width - _mapWidth * _zoom;
    float minY = visibleSize.height - _mapHeight * _zoom;

    _mapPosition.x = clampf(_mapPosition.x, minX, 0);
    _mapPosition.y = clampf(_mapPosition.y, minY, 0);

    updateMapTransform();
}

void BaseMap::moveTo(const Vec2& position)
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    float minX = visibleSize.width - _mapWidth * _zoom;
    float minY = visibleSize.height - _mapHeight * _zoom;

    _mapPosition.x = clampf(position.x, minX, 0);
    _mapPosition.y = clampf(position.y, minY, 0);

    updateMapTransform();
}

void BaseMap::updateMapTransform()
{
    if (_mapSprite)
    {
        _mapSprite->setPosition(_mapPosition);
        _mapSprite->setScale(_zoom);
    }
    
    if (_buildingsContainer)
    {
        _buildingsContainer->setPosition(_mapPosition);
        _buildingsContainer->setScale(_zoom);
    }
    
    // 让部队容器也跟随地图缩放和移动
    if (_troopsContainer)
    {
        _troopsContainer->setPosition(_mapPosition);
        _troopsContainer->setScale(_zoom);
    }
}

Vec2 BaseMap::worldToGrid(const Vec2& worldPos) const
{
    if (_gridCellWidth <= 0 || _gridCellHeight <= 0) return Vec2(-1, -1);

    Vec2 relativePos = worldPos - Vec2(_grassOffsetX, _grassOffsetY);
    int gridX = (int)(relativePos.x / _gridCellWidth);
    int gridY = (int)(relativePos.y / _gridCellHeight);

    float centerX = _grassRectWidth / 2.0f;
    float centerY = _grassRectHeight / 2.0f;
    float dx = relativePos.x - centerX;
    float dy = relativePos.y - centerY;

    float halfDiagonalX = _grassRectWidth / 2.0f;
    float halfDiagonalY = _grassRectHeight / 2.0f;
    bool inDiamond = (std::abs(dx) / halfDiagonalX + std::abs(dy) / halfDiagonalY <= 1.0f);

    if (!inDiamond) return Vec2(-1, -1);
    return Vec2(gridX, gridY);
}

Vec2 BaseMap::gridToWorld(int gridX, int gridY) const
{
    if (!isValidGrid(gridX, gridY)) return Vec2::ZERO;

    float worldX = _grassOffsetX + (gridX + 0.5f) * _gridCellWidth;
    float worldY = _grassOffsetY + (gridY + 0.5f) * _gridCellHeight;

    return Vec2(worldX, worldY);
}

bool BaseMap::isValidGrid(int gridX, int gridY) const
{
    if (gridX < 0 || gridX >= _gridCols || gridY < 0 || gridY >= _gridRows) return false;

    float centerX = _gridCols / 2.0f;
    float centerY = _gridRows / 2.0f;
    float dx = gridX - centerX;
    float dy = gridY - centerY;

    float halfDiagonalX = _gridCols / 2.0f;
    float halfDiagonalY = _gridRows / 2.0f * (3.0f / 4.0f);
    return (std::abs(dx) / halfDiagonalX + std::abs(dy) / halfDiagonalY <= 1.0f);
}

void BaseMap::initOccupiedGrids()
{
    _occupiedGrids.clear();
    if (_gridRows > 0 && _gridCols > 0) {
        _occupiedGrids.resize(_gridRows);
        for (int i = 0; i < _gridRows; ++i)
        {
            _occupiedGrids[i].resize(_gridCols, false);
        }
    }
}

void BaseMap::addBuilding(Node* building)
{
    if (building && _buildingsContainer)
    {
        _buildingsContainer->addChild(building);
    }
}

void BaseMap::removeBuilding(Node* building)
{
    if (building && _buildingsContainer)
    {
        _buildingsContainer->removeChild(building);
    }
}

bool BaseMap::canPlaceBuilding(int gridX, int gridY, Size gridSize) const
{
    std::vector<Vec2> occupied = getOccupiedGrids(gridX, gridY, gridSize);
    for (const auto& grid : occupied)
    {
        int x = (int)grid.x;
        int y = (int)grid.y;

        if (!isValidGrid(x, y)) return false;
        if (y >= 0 && y < (int)_occupiedGrids.size() &&
            x >= 0 && x < (int)_occupiedGrids[y].size() &&
            _occupiedGrids[y][x])
        {
            return false;
        }
    }
    return true;
}

std::vector<Vec2> BaseMap::getOccupiedGrids(int gridX, int gridY, Size gridSize) const
{
    std::vector<Vec2> grids;
    for (int y = 0; y < (int)gridSize.height; ++y)
    {
        for (int x = 0; x < (int)gridSize.width; ++x)
        {
            grids.push_back(Vec2(gridX + x, gridY + y));
        }
    }
    return grids;
}

void BaseMap::markGridsOccupied(int gridX, int gridY, Size gridSize, bool occupied)
{
    std::vector<Vec2> grids = getOccupiedGrids(gridX, gridY, gridSize);
    for (const auto& grid : grids)
    {
        int x = (int)grid.x;
        int y = (int)grid.y;

        if (y >= 0 && y < (int)_occupiedGrids.size() &&
            x >= 0 && x < (int)_occupiedGrids[y].size())
        {
            _occupiedGrids[y][x] = occupied;
        }
    }
}
