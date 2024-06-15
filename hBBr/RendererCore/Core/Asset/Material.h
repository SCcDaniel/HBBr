﻿#pragma once
#include "Common.h"
#include "Texture2D.h"
#include "Pass/PassType.h"
#include "Primitive.h"
#include "Asset/HGuid.h"
#include "XMLStream.h"
#include "AssetObject.h"
#include <unordered_map>
#include <vector>

#define DefaultMaterialGuid "b51e2e9a-0985-75e8-6138-fa95efcbab57"

class Material :public AssetObject
{
public:
	Material();
	
	~Material();

	HBBR_API static std::weak_ptr<Material> LoadAsset(HGUID guid);

	HBBR_API void SaveAsset(HString path);

	HBBR_API static  std::weak_ptr<AssetInfoBase> CreateMaterial(HString repository,HString virtualPath);

	HBBR_API HBBR_INLINE MaterialPrimitive* GetPrimitive()const { return _primitive.get(); }

	HBBR_INLINE std::vector<class Texture2D*> GetTextures() {
		return _primitive->GetTextures();
	}

private:

	HString _materialName = "Unknow material" ;

	std::unique_ptr<MaterialPrimitive> _primitive;

};