#ifndef __PATH_FINDER_H__
#define __PATH_FINDER_H__

#include "cocos2d.h"

USING_NS_CC;

/**
 * 路径寻找器 - 使用A*算法寻找路径
 */
class PathFinder
{
public:
    static PathFinder* getInstance();
    static void destroyInstance();
    
    // 寻找路径
    std::vector<Vec2> findPath(const Vec2& start, const Vec2& end);
    
    // 检查位置是否可通行
    bool isWalkable(const Vec2& position) const;
    
    // 设置障碍物
    void setObstacle(const Vec2& position, bool isObstacle);
    
private:
    PathFinder();
    ~PathFinder();
    
    struct Node
    {
        int x, y;
        float g, h, f;
        Node* parent;
        
        Node(int x, int y) : x(x), y(y), g(0), h(0), f(0), parent(nullptr) {}
    };
    
    float calculateHeuristic(const Node* a, const Node* b) const;
    std::vector<Node*> getNeighbors(Node* node) const;
    
    static PathFinder* _instance;
};

#endif // __PATH_FINDER_H__

