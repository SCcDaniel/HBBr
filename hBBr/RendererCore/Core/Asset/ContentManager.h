﻿#pragma once
#include "Common.h"
#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include "RendererType.h"
#include "pugixml/pugixml.hpp"
#include "Asset/HGuid.h"
#include "HString.h"
#include "Asset/HGuid.h"

//资产类型
enum class AssetType
{
	Unknow = 0,
	Model = 1,			//.fbx
	Material = 2,		//.mat
	Texture2D = 3,		//.tex2D

	MaxNum = 32,
};


inline static AssetType GetAssetTypeBySuffix(HString suffix)
{
	if (suffix.IsSame("fbx", false))
	{
		return AssetType::Model;
	}
	else if (suffix.IsSame("mat", false))
	{
		return AssetType::Material;
	}
	else if(suffix.IsSame("dds", false))
	{
		return AssetType::Texture2D;
	}
	return AssetType::Unknow;
}

inline static HString GetAssetTypeString(AssetType type)
{
	switch (type)
	{
	case AssetType::Model:return "Model";
	case AssetType::Material:return "Material";
	case AssetType::Texture2D:return "Texture2D";
	case AssetType::Unknow:
	case AssetType::MaxNum:	return "Unknow";
	}
	return "Unknow";
}

struct AssetInfoRefTemp
{
	HGUID guid;
	AssetType type;
};

class AssetInfoBase
{
public:
	AssetInfoBase() {}
	virtual ~AssetInfoBase() {}
	HGUID guid;
	AssetType type;
	HString name;
	HString suffix;
	//相对路径(Asset/....)
	HString assetPath;
	//真实绝对路径，带完整文件名字和后缀
	HString absPath;
	//Info文件的绝对路径
	HString metaFileAbsPath;
	uint64_t byteSize;
	std::vector<std::weak_ptr<AssetInfoBase>> refs;
	//用来暂时储存引用的guid和type,没有太多实际意义,通常是空的
	std::vector<AssetInfoRefTemp> refTemps;
	//
	bool bAssetLoad = false;
	virtual std::weak_ptr<class AssetObject> GetAssetData()const { return std::weak_ptr<class AssetObject>(); }
	virtual void ReleaseData() {}
	inline const bool IsAssetLoad()const
	{
		return bAssetLoad;
	}
};

template<class T>
class AssetInfo : public AssetInfoBase
{
	std::shared_ptr<T> data = nullptr;
public:
	AssetInfo():AssetInfoBase(){}
	virtual ~AssetInfo() { ReleaseData(); }
	inline std::weak_ptr<T> GetData()const{
		if (data)
		{
			return data;
		}
		return T::LoadAsset(this->guid);
	}
	inline std::weak_ptr<class AssetObject> GetAssetData()const override {
		if (data)
		{
			return data;
		}
		return T::LoadAsset(this->guid);
	}
	inline void ReleaseData() override{
		data.reset();
		data = nullptr;
		bAssetLoad = false;
	}
	inline void SetData(std::shared_ptr<T> newData){
		data = std::move(newData);
		bAssetLoad = true;
	}
};

struct AssetSaveType
{
	HString metaAssetPath;
	HGUID guid;
	AssetType type;
	size_t byteSize;
	std::vector<std::weak_ptr<AssetInfoBase>> refs;
};

class ContentManager
{
	friend class VulkanApp;
public:
	~ContentManager();

	HBBR_API inline static ContentManager* Get() {
		if (!_ptr)
			_ptr.reset(new ContentManager());
		return _ptr.get(); 
	}
	
	/* 重载所有资产信息(只是加载引用信息,非资产本身) */
	HBBR_API void ReloadAllAssetInfos();

	/* 更新所有资产引用关系 */
	HBBR_API void UpdateAllAssetReference();

	/* 根据AssetType更新资产的引用关系(Type) */
	HBBR_API void UpdateAssetReferenceByType(AssetType type);

	/* 更新单个资产的引用关系(GUID)&(Type) */
	HBBR_API void UpdateAssetReference(AssetType type , HGUID obj);

	/* 更新单个资产的引用关系(GUID),不指定Type,会全局检索,可能会比较慢 */
	HBBR_API void UpdateAssetReference(HGUID obj);

	/* 创建资产信息 */
	HBBR_API std::weak_ptr<AssetInfoBase> CreateAssetInfo(HString AssetPath);

	HBBR_API void SaveAssetInfo(AssetInfoBase* info);

	HBBR_API void SaveAssetInfo(AssetSaveType save);

	/* 删除资产 */
	HBBR_API void  DeleteAsset(HString filePath);

	/* 删除资产信息 */
	HBBR_API void RemoveAssetInfo(HGUID obj, AssetType type = AssetType::Unknow);

	HBBR_API inline const std::unordered_map<HGUID, std::shared_ptr<AssetInfoBase>>& GetAssets(AssetType type)const { return _assets[(uint32_t)type]; }

	HBBR_API std::weak_ptr<AssetInfoBase> GetAssetInfo(HGUID guid, AssetType type = AssetType::Unknow)const;

	/* 根据路径实际文件获取 */
	HBBR_API std::weak_ptr<AssetInfoBase> GetAssetInfo(HString assetPath)const;

	/* 根据内容浏览器显示的文件名称(虚拟路径)查找(非实际GUID的名称)GUID */
	HBBR_API HGUID GetAssetGUID(HString assetPath)const;

	template<class T>
	HBBR_INLINE std::weak_ptr<T> GetAsset(HGUID guid , AssetType type = AssetType::Unknow)
	{
		auto assetInfo = GetAssetInfo(guid , type);
		if (assetInfo.expired())
		{
			return std::weak_ptr<T>();
		}
		auto asset = std::static_pointer_cast<AssetInfo<T>>(assetInfo.lock());
		return asset->GetData();
	}

	//通过已知类型和guid加载资产
	template<class T>
	HBBR_INLINE std::weak_ptr<class AssetObject> LoadAsset(HGUID guid)
	{
		return T::LoadAsset(guid);
	}

private:

	/* 更新单个资产的引用关系(info) */
	void UpdateAssetReference(std::weak_ptr<AssetInfoBase> info);

	/* 重载单个资产的信息(只是加载引用信息,非资产本身),meta绝对路径 */
	std::weak_ptr<AssetInfoBase> ReloadAssetInfoByMetaFile(HString AbsPath);

	void Release();

	void ReleaseAssetsByType(AssetType type, bool bDestroy);

	void ReleaseAsset(AssetType type, HGUID obj);

	ContentManager();

	static std::unique_ptr<ContentManager> _ptr;

	std::vector<std::unordered_map<HGUID, std::shared_ptr<AssetInfoBase>>>_assets;

};
