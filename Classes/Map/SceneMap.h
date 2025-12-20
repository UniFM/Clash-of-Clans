#pragma once
/*************************************************************
* @file     : SceneMap.h
* @function ：所有地图的基类 - 部落冲突地图系统
* @author   : 叶芷含
* @note     ：包含地图网格系统、建筑放置、碰撞检测等核心功能
**************************************************************/

#ifndef __SCENEMAP_H__
#define __SCENEMAP_H__
#include "cocos2d.h"
#include "Constant/Constant.h"
#include "Control/Control.h"
#include "Scene/ShopScene.h"

USING_NS_CC;

class SceneMap : public Node {
public:
	// 初始化瓦片地图
	virtual bool init(const std::string& tmxFile);

	// ͨ����ײ���ӿ�
	virtual bool isPositionValid(const Vec2& pos) const;    // ���λ���Ƿ�Ϸ�
	virtual bool canPlaceBuilding(const Vec2& pos, const Size& buildingSize) const;    // ��⽨���ܷ����
	virtual TerrainType getTerrainType(const Vec2& pos) const;    // ��ȡ��������
	Size getMapSize() const;    // ��ȡ��ͼ�ߴ�
	Size getTileSize() const;    // ��ȡ��Ƭ�ߴ�
	TMXTiledMap* getTiledMap() const { return tileMap; }  // ��ȡTMX��Ƭ��ͼ����
	cocos2d::Vec2 TMXToCocos2d(const cocos2d::Vec2& tmxPos) const;
	cocos2d::Vec2 Cocos2dToTMX(const cocos2d::Vec2& cocosPos) const;

	// �滻Ϊ��㴥���ĺ���
	void onTouchesBegan(const std::vector<Touch*>& touches, Event* event);
	void onTouchesMoved(const std::vector<Touch*>& touches, Event* event);
	void onTouchesEnded(const std::vector<Touch*>& touches, Event* event);

	//// �����¼�����  �Ƴ���ָ
	//bool onTouchBegan(Touch* touch, Event* event);
	//void onTouchMoved(Touch* touch, Event* event);
	//void onTouchEnded(Touch* touch, Event* event);

	//���ŵ�ͼ
	void zoomIn();
	void zoomOut();

	// ���������ŷ���
	void onMouseScroll(EventMouse* event);

	// �̵갴ť�ص�
	void onShopButtonClicked(Ref* sender);

	// �����̵곡��
	void enterShop();

	// ��ȡ��ͼ��
	TMXLayer* getLayer(const std::string& layerName) const;

protected:

	// ������ײ��ⷽ��
	bool checkTileCollision(const Vec2& pos) const;    // ��ⵥ����Ƭ��ײ
	bool isWithinMapBounds(const Vec2& pos) const;    // ���λ���Ƿ��ڵ�ͼ�߽���
	TMXLayer* getCollisionLayer() const;    // ��ȡ��ײ��

	// ��ͼ����
	TMXTiledMap* tileMap;
	TMXLayer* collisionLayer;  // ��ײ����

	// �������
	Vec2 lastTouchPos;  // �ϴδ���λ��

	// ���ù�����ͼ
	void setupScrollView();

	float currentScale = 1.0f;	// ��ǰ����ϵ��

	const float scaleStep = 0.1f; // ÿ�����Ų���
	const float minScale = 0.5f;  // ��С��������
	const float maxScale = 3.0f;  // �����������

	//��������
	bool isTwoTouch = false;       // �Ƿ�˫ָ����
	float initTwoTouchDistance;    // ˫ָ��ʼ����
	Vec2 initTwoTouchCenter;       // ˫ָ��ʼ���ĵ㣨��Ļ���꣩

	// ���
	const float scrollStep = 0.1f; // ����ÿ�����Ų���

	// �̵��ȡ�ɹ���־
	cocos2d::Label* statusLabel;
};

#endif