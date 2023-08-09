﻿#include "Material.h"
#include "Shader.h"
#include "VertexFactory.h"
#include "VulkanManager.h"
Material* Material::_defaultMaterial;

Material* Material::_errorMaterial;

std::unordered_map<HUUID, std::unique_ptr<Material>> Material::_allMaterials;

Material::Material(bool bDefault)
{
	if (bDefault)
	{
		_primitive.reset(new MaterialPrimitive());
		_primitive->graphicsName = _materialName;
		_primitive->vsShader = "BasePassTemplate";
		_primitive->psShader = "BasePassTemplate";
		_primitive->pass = Pass::OpaquePass;
		_primitive->inputLayout = VertexFactory::VertexInput::BuildLayout(Shader::_vsShader[_primitive->vsShader].header.vertexInput);
		PrimitiveProxy::GetNewMaterialPrimitiveIndex(_primitive.get());
		PrimitiveProxy::AddMaterialPrimitive(_primitive.get());
	}
}

Material::~Material()
{
	PrimitiveProxy::RemoveMaterialPrimitive(_primitive->pass, _primitive.get());
}

Material* Material::LoadMaterial(HString materialFilePath)
{
	pugi::xml_document materialDoc;
	if (XMLStream::LoadXML(materialFilePath.c_wstr(), materialDoc))
	{
		std::unique_ptr<Material> mat (new Material) ;
		auto root = materialDoc.child(TEXT("root"));
		HUUID uuid;
		HString uuidStr;
		XMLStream::LoadXMLAttributeString(root, TEXT("UUID"), uuidStr);
		if (!StringToUUID(uuidStr.c_str(), &uuid))
		{
			uuid = CreateUUID();
			uuidStr = UUIDToString(uuid).c_str();
		}
		auto materialPrim = root.child(TEXT("MaterialPrimitive"));
		//MaterialPrimitive
		mat->_primitive.reset(new MaterialPrimitive());
		mat->_primitive->graphicsName = materialFilePath.GetBaseName();
		XMLStream::LoadXMLAttributeString(materialPrim, TEXT("vsShader"), mat->_primitive->vsShader);
		XMLStream::LoadXMLAttributeString(materialPrim, TEXT("psShader"), mat->_primitive->psShader);
		int pass;
		XMLStream::LoadXMLAttributeInt(materialPrim, TEXT("pass"), pass);
		mat->_primitive->pass = (Pass)pass;
		mat->_primitive->inputLayout = VertexFactory::VertexInput::BuildLayout(Shader::_vsShader[mat->_primitive->vsShader].header.vertexInput);
		//Parameters
		auto parameters = root.child(TEXT("Parameters"));
		int paramCount = 0;
		for (auto i = parameters.first_child(); i != NULL; i = i.next_sibling())
		{
			paramCount++;
		}
		mat->_paramterInfos.reserve(paramCount);
		mat->_primitive->uniformBuffer.reserve( paramCount * 4 );
		int alignmentFloat4 = 0; // float4 对齐
		for (auto i = parameters.first_child(); i != NULL; i = i.next_sibling())
		{
			MaterialParameterInfo info;
			HString value;
			XMLStream::LoadXMLAttributeString(i, TEXT("name"), info.name);
			XMLStream::LoadXMLAttributeString(i, TEXT("type"), info.type);
			XMLStream::LoadXMLAttributeString(i, TEXT("value"), value);
			XMLStream::LoadXMLAttributeString(i, TEXT("ui"), info.ui);

			//
			auto splitValue = value.Split(",");
			if (alignmentFloat4 + splitValue.size() > 4)
			{
				for (int i = 0; i < 4 - alignmentFloat4; i++)
				{
					mat->_primitive->uniformBuffer.push_back(0);
				}
				alignmentFloat4 = 0;
			}
			for (int i = 0; i < splitValue.size(); i++)
			{
				info.beginPos = mat->_primitive->uniformBuffer.size();
				mat->_primitive->uniformBuffer.push_back((float)HString::ToDouble(splitValue[i]));
				alignmentFloat4++;
				if (alignmentFloat4 >= 4)
					alignmentFloat4 = 0;
			}
			mat->_paramterInfos.push_back(info);
		}
		auto anlignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(sizeof(float) * mat->_primitive->uniformBuffer.size());
		mat->_primitive->uniformBufferSize = anlignmentSize;
		//
		PrimitiveProxy::GetNewMaterialPrimitiveIndex(mat->_primitive.get());
		PrimitiveProxy::AddMaterialPrimitive( mat->_primitive.get());
		_allMaterials.emplace(std::make_pair(uuid, std::move(mat)));
		return _allMaterials[uuid].get();
	}
	return NULL;
}

