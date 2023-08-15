﻿#include"SceneManager.h"
#include "VulkanRenderer.h"
#include "Component/GameObject.h"
#include "Component/ModelComponent.h"
#include "FileSystem.h"
SceneManager::~SceneManager()
{
	//wait gameobject destroy
	while (_gameObjects.size() > 0 || _gameObjectNeedDestroy.size() > 0 || _gameObjectParentSettings.size() > 0 )
	{
		for (int i = 0; i < _gameObjects.size(); i++)
		{
			_gameObjects[i]->Destroy();
		}
		SceneUpdate();
	}
}

//Test 
std::weak_ptr<GameObject> testObj;

void SceneManager::SceneInit(class VulkanRenderer* renderer)
{
	_renderer = renderer;
	//Test
	auto test = new GameObject();
	auto modelComp0 = test->AddComponent<ModelComponent>();
	modelComp0->SetModel(FileSystem::GetResourceAbsPath() + "Content/FBX/TestFbx_1_Combine.FBX");

	GameObject* cube = new GameObject();
	testObj = cube->_selfWeak;
	auto modelComp = cube->AddComponent<ModelComponent>();
	cube->GetTransform()->SetLocation(glm::vec3(-0.75f, 1.5f, 0));
	modelComp->SetModel(FileSystem::GetResourceAbsPath() + "Content/FBX/TestFbx_Cube.FBX");
}
#include "HInput.h"
void SceneManager::SceneUpdate()
{
	//Test
	if (!testObj.expired() && testObj.lock()->GetTransform() && ( HInput::GetKey(KeyCode::F)))
	{
		testObj.lock()->GetTransform()->SetRotation(testObj.lock()->GetTransform()->GetRotation() + glm::vec3(0, _renderer->GetFrameRate() / 10.0f, 0));
	}

	//Setting object parent
	const auto settingObjCount = _gameObjectParentSettings.size();
	for (auto& i : _gameObjectParentSettings)
	{
		if (i->_newParent != NULL)
		{
			if (i->_parent != NULL)
			{
				//为了安全考虑，最好进行一次Parent的查找，不过感觉应该不需要...
				//auto it = std::find(_gameObjects.begin(), _gameObjects.end(), i->_parent);
				//if (it != _gameObjects.end())
				{
					//(*it)->_children.erase();
					//如果已经有父类了先清除
					auto cit = std::find(i->_parent->_children.begin(), i->_parent->_children.end(), i);
					if (cit != i->_parent->_children.end())
					{
						i->_parent->_children.erase(cit);
					}		
				}
			}
			i->_parent = i->_newParent;
			i->_newParent = NULL;
		}
		else
		{
			//如果已经有父类了先清除
			if (i->_parent != NULL)
			{
				auto cit = std::find(i->_parent->_children.begin(), i->_parent->_children.end(), i);
				if (cit != i->_parent->_children.end())
				{
					i->_parent->_children.erase(cit);
				}
			}
			i->_parent = NULL;
		}
	}
	_gameObjectParentSettings.clear();

	//Destroy Objects
	const auto destroyCount = _gameObjectNeedDestroy.size();
	for (auto& i : _gameObjectNeedDestroy)
	{
		i.reset();
	}
	_gameObjectNeedDestroy.clear();

	//Update Objecets
	const auto objCount = _gameObjects.size();
	for (int i = 0; i < objCount ; i++)
	{
		if (!_gameObjects[i]->Update())
		{
			i -= 1;
			if (_gameObjects.size() <= 0)
				break;
		}
	}

	//Update Editor if the function is not null.
	#if IS_EDITOR
		_editorSceneUpdateFunc(this, _gameObjects);
	#endif
}

void SceneManager::AddNewObject(std::shared_ptr<GameObject> newObject)
{
	_gameObjects.push_back(newObject);
	#if IS_EDITOR
		_editorGameObjectAddFunc(this, newObject);
	#endif
}

void SceneManager::RemoveObject(GameObject* object)
{
	auto it = std::find_if(_gameObjects.begin(),_gameObjects.end(), [object](std::shared_ptr<GameObject>& obj)
		{
			return obj.get() == object;
		});
	if (it != _gameObjects.end())
	{
		//延迟到下一帧再销毁
		_gameObjectNeedDestroy.push_back(*it);
		#if IS_EDITOR
			_editorGameObjectAddFunc(this, *it);
		#endif
		_gameObjects.erase(it);
	}
}
