﻿#pragma once
#include "Common.h"
#include "Texture.h"
#include "Pass/PassType.h"
#include "Primitive.h"
#include "Resource/HGuid.h"
#include "XMLStream.h"

#include <unordered_map>
#include <vector>

#define DefaultMaterialGuid "61A147FF-32BD-48EC-B523-57BC75EB16BA"

class Material
{
public:
	Material();
	
	~Material();

	HBBR_API static Material* LoadMaterial(HGUID guid);

	HBBR_API static Material* CreateMaterial(HString newMatFilePath);

	HBBR_API HBBR_INLINE MaterialPrimitive* GetPrimitive()const { return _primitive.get(); }

	HBBR_API HBBR_INLINE HGUID GetGUID()const { return _guid; }

	HBBR_INLINE std::vector<class Texture*> GetTextures() {
		return _primitive->GetTextures();
	}

private:

	HString _materialName = "Unknow material" ;

	std::unique_ptr<MaterialPrimitive> _primitive;

	HGUID _guid;

};