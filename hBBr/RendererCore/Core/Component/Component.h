﻿#pragma once
//基础Compoennt组件类,组件都是附加在GameObject上的,结构体参数必须至少带有class GameObject* parent
#include <vector>


class Component
{
	friend class GameObject;
public:

	Component(class GameObject* parent);

	~Component();

	__forceinline void Destroy() {
		SetActive(false);
		_bWantDestroy = true;
	}

	__forceinline bool IsActive() const {
		return _bActive;
	}

	__forceinline GameObject* GetGameObject() const {
		return _gameObject;
	}

	virtual void SetActive(bool newActive = true);

protected:

	virtual void Init();

	virtual bool Update();

	virtual void ExecuteDestroy();

	bool _bActive;

	bool _bInit;

	bool _bWantDestroy;

	class GameObject* _gameObject = NULL;
	

};