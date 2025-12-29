#include "Entity.h"

USING_NS_CC;

Entity::Entity() : _currentHealth(0), _maxHealth(0), _team(Team::PLAYER), _isDead(false), _sprite(nullptr), _healthBar(nullptr) {}

Entity::~Entity() {}

bool Entity::init() {
    if (!Node::init()) return false;
    return true;
}

void Entity::setStats(int maxHealth, Team team) {
    _maxHealth = maxHealth;
    _currentHealth = maxHealth;
    _team = team;
    createHealthBar();
}

void Entity::takeDamage(int damage) {
    if (_isDead) return;

    _currentHealth -= damage;
    if (_currentHealth <= 0) {
        _currentHealth = 0;
        _isDead = true;
        // Simple death effect: fade out
        this->runAction(Sequence::create(FadeOut::create(0.5f), RemoveSelf::create(), nullptr));
    }
    updateHealthBar();
}

bool Entity::isDead() const {
    return _isDead;
}

Team Entity::getTeam() const {
    return _team;
}

void Entity::createHealthBar() {
    if (_healthBar) {
        _healthBar->removeFromParent();
    }
    _healthBar = DrawNode::create();
    this->addChild(_healthBar, 100);
    updateHealthBar();
}

void Entity::updateHealthBar() {
    if (!_healthBar) return;
    _healthBar->clear();

    if (_isDead) return;

    float width = 40.0f;
    float height = 5.0f;
    float yOffset = 60.0f; // Above the sprite

    // Background
    _healthBar->drawSolidRect(Vec2(-width / 2, yOffset), Vec2(width / 2, yOffset + height), Color4F::GRAY);

    // Foreground
    float healthPct = (float)_currentHealth / _maxHealth;
    Color4F color = Color4F::GREEN;
    if (healthPct < 0.5f) color = Color4F::YELLOW;
    if (healthPct < 0.2f) color = Color4F::RED;

    _healthBar->drawSolidRect(Vec2(-width / 2, yOffset), Vec2(-width / 2 + width * healthPct, yOffset + height), color);
}
