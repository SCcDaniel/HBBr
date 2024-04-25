﻿#include"World.h"
#include "VulkanRenderer.h"
#include "Component/GameObject.h"
#include "Component/ModelComponent.h"
#include "Component/CameraComponent.h"
#include "FileSystem.h"
#include "XMLStream.h"
#include "HInput.h"
#include "ConsoleDebug.h"
World::World(class VulkanRenderer* renderer)
{
	PreLoad(renderer);
}

World::~World()
{
	ReleaseWorld();
}

void World::AddLevel(HString levelNameOrAssetPath)
{
	HString path = levelNameOrAssetPath; 
	//Asset完整路径
	if (FileSystem::FileExist(path))
	{

	}
	else
	{
		//Asset相对路径
		path = FileSystem::FillUpAssetPath(levelNameOrAssetPath);
		if (FileSystem::FileExist(path))
		{

		}
		//Name
		else
		{
			path = FileSystem::Append(_worldAssetPath, levelNameOrAssetPath) + ".level";
			if (FileSystem::FileExist(path))
			{

			}
		}
	}
#if IS_EDITOR
	_editorLevelChanged();
#endif
}

void World::AddNewLevel(HString name)
{
	std::shared_ptr<Level> newLevel = nullptr;
	newLevel.reset(new Level(this, name));
	newLevel->Load();
	_levels.push_back(newLevel);
#if IS_EDITOR
	_editorLevelChanged();
#endif
}

void World::SaveWorld(HString newWorldName)
{
	newWorldName.ClearSpace();
	HString worldSettingPath;
	if (newWorldName.Length() > 1)
	{
		HString assetPath = FileSystem::GetWorldAbsPath();
		worldSettingPath = assetPath;
		newWorldName = assetPath + "/" + newWorldName + ".world";
		worldSettingPath ="World/" + newWorldName + ".world/WorldSetting.xml";
		FileSystem::FixUpPath(worldSettingPath);
		FileSystem::FixUpPath(newWorldName);
		_worldAssetPath = newWorldName;
	}
	//
	WorldSetting.InitArchive(worldSettingPath);
	WorldSetting.SaveArchive();
	//
	//创建World目录
	FileSystem::CreateDir(_worldAssetPath.c_str());
	for (auto& i : _levels)
	{
		i->SaveLevel();
	}
}

GameObject* World::SpawnGameObject(HString name, class Level* level)
{
	if (this->_levels.size() <= 0)
	{
		MsgBox("World.cpp/SpawnGameObject","Spawn game object failed.This world is not having any levels.");
		return nullptr;
	}
	if (!level)
	{
		level = this->_levels[0].get();
	}
	if (!level->bLoad)
	{
		MsgBox("World.cpp/SpawnGameObject", "Spawn game object failed.The level is not loading.");
		return nullptr;
	}
	GameObject* newObject = GameObject::CreateGameObject(name, level);
	return newObject;
}

void World::PreLoad(class VulkanRenderer* renderer)
{
	_renderer = renderer;
}

void World::Load(HString worldName)
{
	if (worldName.GetSuffix() == "world" && FileSystem::IsDir(worldName))
	{
		HString fullPath = FileSystem::Append(FileSystem::GetWorldAbsPath(), worldName);
		_worldAssetPath = FileSystem::GetRelativePath(fullPath);
		FileSystem::FixUpPath(_worldAssetPath);

		//Find all levels
		auto levelFiles = FileSystem::GetFilesBySuffix(_worldAssetPath.c_str(), "level");
		for (auto f : levelFiles)
		{
			AddLevel(f.relativePath);
		}
	}
	else
	{
		HString assetPath = FileSystem::GetWorldAbsPath();
		if (!FileSystem::FileExist(assetPath))
		{
			FileSystem::CreateDir(assetPath.c_str());
		}
		HString dirPath = assetPath + "/" + _worldName + ".world";
		FileSystem::FixUpPath(dirPath);
		_worldAssetPath = dirPath;

		//This is a new world ,create a empty level .
		AddNewLevel("Empty Level");
	}

#if IS_EDITOR

	//Create editor only level.
	_editorLevel.reset(new Level(this, "EditorLevel"));
	_editorLevel->Load();
	_editorLevel->_isEditorLevel = true;

	//create editor camera
	auto backCamera = GameObject::CreateGameObject("EditorCamera", _editorLevel.get());
	backCamera->_sceneEditorHide = true;
	backCamera->GetTransform()->SetWorldLocation(glm::vec3(0, 2, -3.0));
	auto cameraComp = backCamera->AddComponent<CameraComponent>();
	cameraComp->OverrideMainCamera();
	_editorCamera = cameraComp;
	_editorCamera->_bIsEditorCamera = true;

#else

	//	//Test game camera
	//	auto backCamera = new GameObject("Camera");
	//	backCamera->GetTransform()->SetWorldLocation(glm::vec3(0, 2, -3.0));
	//	auto cameraComp = backCamera->AddComponent<CameraComponent>();
	//	cameraComp->OverrideMainCamera();

#endif

	bLoad = true;


#if IS_GAME
	//-----model--camera
	auto backCamera = GameObject::CreateGameObject("TestGameCamera", _levels[0].get());
	backCamera->GetTransform()->SetWorldLocation(glm::vec3(0, 2, -3.0));
	auto cameraComp = backCamera->AddComponent<CameraComponent>();
	cameraComp->OverrideMainCamera();
#endif

	//-----model--test
	auto testModel = GameObject::CreateGameObject("Test", _levels[0].get());
	auto modelComp = testModel->AddComponent<ModelComponent>();
	modelComp->SetModel(HGUID("c51a01e8-9349-660a-d2df-353a310db461"));
	ConsoleDebug::printf_endl("Test Model Spawn......");

}

bool World::ReleaseWorld()
{
	for (auto& i : _levels)
	{
		i.reset();
	}
#if IS_EDITOR
	_editorLevel.reset();
#endif
	_levels.clear();
	return true;
}

void World::WorldUpdate()
{
	std::vector < std::weak_ptr<Level> >_levelPtrs;
	_levelPtrs.resize(_levels.size() + 1);
	if (!bLoad)
	{
		return;
	}
	for (int i = 0; i < _levels.size(); i++)
	{
		_levelPtrs[i] = _levels[i];
		_levels[i]->LevelUpdate();
	}


	//Update Editor if the function is not null.
#if IS_EDITOR
	if (_editorLevel)
	{
		_editorLevel->LevelUpdate();
		_levelPtrs[_levelPtrs.size() - 1] = _editorLevel;
	}
	_editorWorldUpdate(_levelPtrs);
#endif

}

void World::UpdateObject(std::shared_ptr<GameObject> newObject)
{
#if IS_EDITOR
	if (!newObject->_sceneEditorHide)
		_editorGameObjectUpdateFunc(newObject);
#endif
}

void World::AddNewObject(std::shared_ptr<GameObject> newObject)
{
#if IS_EDITOR
	if (!newObject->_sceneEditorHide)
	{
		_editorGameObjectAddFunc(newObject);
	}
#endif
}

void World::RemoveObject(std::shared_ptr<GameObject> object)
{
#if IS_EDITOR
	if (!(object->_sceneEditorHide))
		_editorGameObjectRemoveFunc(object);
#endif
}

#if IS_EDITOR

void World::SetCurrentSelectionLevel(std::weak_ptr<Level> level)
{
	_currentSelectionLevel = level;
}

#endif

