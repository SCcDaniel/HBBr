﻿#pragma once
#include "Common.h"
#include <vector>
#include <map>
#include "HString.h"
#include "Transform.h"
#include "HGuid.h"

class GameObject
{
	friend class World;
	friend class Level;
	friend class VulkanRenderer;
	friend class Component;
	friend class Inspector;
	GameObject(HString objectName = "NewGameObject", class Level* level = nullptr, bool SceneEditorHide = false);
	GameObject(class Level* level = nullptr, bool SceneEditorHide = false);
public:
	~GameObject();

	void ObjectInit(HString objectName = "NewGameObject", class Level* level = nullptr, bool SceneEditorHide = false);

	HBBR_API static GameObject* CreateGameObject(HString objectName = "NewGameObject", class Level* level = nullptr);

	HBBR_API static GameObject* CreateModelGameObject(HString virtualPath, class Level* level = nullptr);

	HBBR_API HBBR_INLINE void Destroy() {
		SetActive(false);
		_bWantDestroy = true;
	}

	HBBR_API HBBR_INLINE bool IsActive() const {
		return _bActive;
	}

	HBBR_API void SetActive(bool newActive = true);

	HBBR_API HBBR_INLINE HString GetObjectName() const {
		return _name;
	}

	HBBR_API void SetObjectName(HString newName);

	HBBR_API HBBR_INLINE std::vector<GameObject*> GetChildren() {
		return _children;
	}

	HBBR_API HBBR_INLINE uint64_t GetChildrenNum() {
		return _children.size();
	}

	HBBR_API HBBR_INLINE GameObject* GetParent() {
		return _parent;
	}

	HBBR_API HBBR_INLINE Transform* GetTransform() {
		return _transform;
	}

	HBBR_API HBBR_INLINE class Level* GetLevel() {
		return _level;
	}

	HBBR_API HBBR_INLINE class World* GetWorld() {
		return _world;
	}

	HBBR_API HBBR_INLINE std::weak_ptr<GameObject> GetSelfWeekPtr() {
		return _selfWeak;
	}

	HBBR_API HBBR_INLINE HGUID GetGUID() {
		return _guid;
	}

	HBBR_API void SetParent(GameObject* newParent);

	HBBR_API static bool IsValid(std::weak_ptr<GameObject> obj){
		return !obj.expired() && !obj.lock()->_bWantDestroy && obj.lock()->_transform != nullptr;
	}

	HBBR_INLINE static std::map<HString, std::function<class Component* (class GameObject*)>> GetCompSpawnMap()	{
		return _componentSpawnFunctions;
	}

	template<typename T, typename ...Args>
	T* AddComponent(Args... args)
	{
		T* result = new T(this, args...);
		_comps.push_back(result);
		return result;
	}

	HBBR_API class Component* AddComponent(HString className)
	{
		auto it = _componentSpawnFunctions.find(className);
		if (it != _componentSpawnFunctions.end())
		{
			auto newComp = it->second(this);
			_comps.push_back(newComp);
			return newComp;
		}
		return nullptr;
	}

#if IS_EDITOR
	void* _editorObject = nullptr;
	bool _bEditorNeedUpdate = false;
	bool _sceneEditorHide = false;
	bool _IsEditorObject = false;
#endif

private:

	void Init();

	/* If object need to destroy(_bWantDesctroy = true),the function will return false. */
	bool Update();

	/* Auto run destroy execute.Do not call this function initiatively. */
	bool ExecuteDestroy();

	class World* _world = nullptr;

	class Level* _level = nullptr;

	bool _bActive = true;

	bool _bWantDestroy = false;

	bool _bInit = false;

	std::vector<class Component*> _comps;

	HString _name = "None";

	GameObject* _parent = nullptr;

	std::vector<GameObject*> _children;

	Transform* _transform;

	std::weak_ptr<GameObject>_selfWeak;

	HGUID _guid;

	//Component spawn by name <class name , spawn function>
	static std::map<HString, std::function<class Component*(class GameObject*)>> _componentSpawnFunctions;
};
