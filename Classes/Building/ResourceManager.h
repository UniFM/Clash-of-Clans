#pragma once
#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "cocos2d.h"

USING_NS_CC;

/**
 * 资源管理器 - 管理金币、圣水、人口
 */
class ResourceManager
{
public:
    static ResourceManager* getInstance();
    static void destroyInstance();

    // 金币
    int getGold() const { return _gold; }
    void setGold(int gold) { _gold = gold; updateUI(); }
    bool addGold(int amount);
    bool spendGold(int amount);

    // 圣水
    int getElixir() const { return _elixir; }
    void setElixir(int elixir) { _elixir = elixir; updateUI(); }
    bool addElixir(int amount);
    bool spendElixir(int amount);

    // 人口/容量
    int getPopulation() const { return _population; }
    int getMaxPopulation() const { return _maxPopulation; }
    void setPopulation(int population) { _population = population; updateUI(); }
    void setMaxPopulation(int maxPopulation) { _maxPopulation = maxPopulation; updateUI(); }
    bool addPopulation(int amount);
    bool removePopulation(int amount);

    // 更新UI显示
    void updateUI();
    void setUI(Node* goldLabel, Node* elixirLabel, Node* populationLabel);

    // 重置资源
    void reset();

private:
    ResourceManager();
    ~ResourceManager();

    int _gold;
    int _elixir;
    int _population;
    int _maxPopulation;

    // UI引用
    Label* _goldLabel;
    Label* _elixirLabel;
    Label* _populationLabel;

    static ResourceManager* _instance;
};

#endif // __RESOURCE_MANAGER_H__

