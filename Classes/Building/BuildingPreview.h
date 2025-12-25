#ifndef __BUILDING_PREVIEW_H__
#define __BUILDING_PREVIEW_H__

#include "cocos2d.h"
#include "Building.h"

USING_NS_CC;

// 前置声明
class HomeVillageMap;

/**
 * 建筑预览系统
 */
class BuildingPreview : public Node
{
public:
    // 创建建筑预览实例
    static BuildingPreview* create(HomeVillageMap* mapLayer);

    // 初始化建筑预览
    bool init(HomeVillageMap* mapLayer);

    // 开始建筑预览（放置前）
    void startPreview(BuildingType buildingType);

    // 开始移动建筑（带确认/取消按钮）
    void startMoving(Building* building);

    // 开始简易拖拽建筑（拖拽结束自动确认）
    void startSimpleDrag(Building* building);

    // 结束简易拖拽
    void endSimpleDrag();

    // 取消预览/移动操作
    void cancel();

    // 确认预览/移动操作
    bool confirm();

    // 更新预览位置（根据屏幕坐标）
    void updatePreviewPosition(const Vec2& screenPos);

    // 开始预览拖拽检测
    void startPreviewDragDetection(const Vec2& startPos);

    // 检查是否触发拖拽
    bool checkDragStart(const Vec2& currentPos);

    // 结束预览拖拽
    void endPreviewDrag();

    // 是否正在预览中
    bool isPreviewing() const { return _isPreviewing; }

    // 是否正在移动中
    bool isMoving() const { return _isMoving; }

    // 是否正在简易拖拽中
    bool isSimpleDragging() const { return _isSimpleDragging; }

    // 是否正在预览拖拽中
    bool isPreviewDragging() const { return _isPreviewDragging; }

    // 获取预览的建筑
    Building* getPreviewBuilding() const { return _previewBuilding; }

    // 获取正在移动的建筑
    Building* getMovingBuilding() const { return _movingBuilding; }

    // 检查指定位置是否有可交互的建筑
    Building* checkBuildingAtPosition(const Vec2& worldPos, const cocos2d::Vector<Building*>& buildings);

private:
    // 构造函数
    BuildingPreview();
    // 析构函数
    ~BuildingPreview();

    // 主村庄地图层引用
    HomeVillageMap* _mapLayer;
    // 预览的建筑
    Building* _previewBuilding;
    // 正在移动的建筑
    Building* _movingBuilding;
    // 预览的建筑类型
    BuildingType _previewType;
    // 是否正在预览
    bool _isPreviewing;
    // 是否正在移动
    bool _isMoving;
    // 是否正在简易拖拽
    bool _isSimpleDragging;
    // 是否正在预览拖拽
    bool _isPreviewDragging;

    // 拖拽起始坐标
    Vec2 _dragStartPos;
    // 拖拽触发阈值（像素）
    float _dragThreshold;

    // 原始网格坐标（移动前）
    int _originalGridX;
    int _originalGridY;

    // 网格高亮根节点
    Node* _gridHighlightNode;
    // 高亮瓦片列表
    std::vector<Node*> _highlightTiles;

    // 确认按钮
    Sprite* _confirmBtn;
    // 取消按钮
    Sprite* _cancelBtn;
    // UI触摸监听器
    EventListenerTouchOneByOne* _uiListener;

    // 更新网格高亮（显示可放置/不可放置区域）
    void updateGridHighlight(int gridX, int gridY, Size gridSize);
    // 清除网格高亮
    void clearGridHighlight();

    // 创建高亮瓦片（区分可放置/不可放置）
    Node* createHighlightTile(bool canPlace);

    // 设置建筑透明度
    void setBuildingOpacity(Building* building, GLubyte opacity);

    // 创建操作按钮（确认/取消）
    void createActionButtons();

    // 更新按钮位置（跟随建筑）
    void updateButtonPosition(const Vec2& worldBuildingPos);

    // 验证并确认当前位置（不可放置则回退）
    bool validateAndConfirmPosition();
};

#endif // __BUILDING_PREVIEW_H__