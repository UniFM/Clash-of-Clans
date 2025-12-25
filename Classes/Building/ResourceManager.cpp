#include "ResourceManager.h"

ResourceManager* ResourceManager::_instance = nullptr;

ResourceManager* ResourceManager::getInstance()
{
    if (!_instance)
    {
        _instance = new (std::nothrow) ResourceManager();
    }
    return _instance;
}

void ResourceManager::destroyInstance()
{
    CC_SAFE_DELETE(_instance);
}

ResourceManager::ResourceManager()
    : _gold(10000)
    , _elixir(10000)
    , _population(0)
    , _maxPopulation(50)
    , _goldLabel(nullptr)
    , _elixirLabel(nullptr)
    , _populationLabel(nullptr)
{
}

ResourceManager::~ResourceManager()
{
}

bool ResourceManager::addGold(int amount)
{
    if (amount < 0)
        return false;
    _gold += amount;
    updateUI();
    return true;
}

bool ResourceManager::spendGold(int amount)
{
    if (amount < 0 || _gold < amount)
        return false;
    _gold -= amount;
    updateUI();
    return true;
}

bool ResourceManager::addElixir(int amount)
{
    if (amount < 0)
        return false;
    _elixir += amount;
    updateUI();
    return true;
}

bool ResourceManager::spendElixir(int amount)
{
    if (amount < 0 || _elixir < amount)
        return false;
    _elixir -= amount;
    updateUI();
    return true;
}

bool ResourceManager::addPopulation(int amount)
{
    if (amount < 0 || _population + amount > _maxPopulation)
        return false;
    _population += amount;
    updateUI();
    return true;
}

bool ResourceManager::removePopulation(int amount)
{
    if (amount < 0 || _population < amount)
        return false;
    _population -= amount;
    updateUI();
    return true;
}

void ResourceManager::setUI(Node* goldLabel, Node* elixirLabel, Node* populationLabel)
{
    _goldLabel = dynamic_cast<Label*>(goldLabel);
    _elixirLabel = dynamic_cast<Label*>(elixirLabel);
    _populationLabel = dynamic_cast<Label*>(populationLabel);
    updateUI();
}

void ResourceManager::updateUI()
{
    if (_goldLabel)
    {
        _goldLabel->setString(StringUtils::format("金币: %d", _gold));
    }
    if (_elixirLabel)
    {
        _elixirLabel->setString(StringUtils::format("圣水: %d", _elixir));
    }
    if (_populationLabel)
    {
        _populationLabel->setString(StringUtils::format("人口: %d/%d", _population, _maxPopulation));
    }
}

void ResourceManager::reset()
{
    _gold = 10000;
    _elixir = 10000;
    _population = 0;
    _maxPopulation = 50;
    updateUI();
}

