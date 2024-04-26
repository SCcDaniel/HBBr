﻿#include"Component.h"
#include "GameObject.h"
#include "Asset/World.h"
#include "Asset/Level.h"
#include "VulkanRenderer.h"
#include "Model.h"
#include "Texture2D.h"
#include "Material.h"
Component::Component(GameObject* parent)
{
	_bActive = true;
	_gameObject = parent;
	_world = _gameObject->_level->GetWorld();
	_renderer = _world->GetRenderer();
}

Component::~Component()
{

}

void Component::SetActive(bool newActive)
{
	_bActive = newActive;
	GameObjectActiveChanged(_gameObject->_bActive);
	if (!newActive)
	{

	}
}

void Component::GameObjectActiveChanged(bool gameObjectActive)
{
}

void Component::Init()
{
	_bInit = true;
}

void Component::Update()
{
	if (_bActive)
	{
		if (!_bInit)//Init
		{
			Init();
		}
		else
		{


		}
	}
}

void Component::ExecuteDestroy()
{

}

void Component::Destroy()
{
	SetActive(false);
	this->ExecuteDestroy();
	auto it = std::remove_if(this->GetGameObject()->_comps.begin(), this->GetGameObject()->_comps.end(), [this](Component*& comp) {
		return comp == this;
		});
	if (it != this->GetGameObject()->_comps.end())
	{
		this->GetGameObject()->_comps.erase(it);
		delete this;
	}
}
