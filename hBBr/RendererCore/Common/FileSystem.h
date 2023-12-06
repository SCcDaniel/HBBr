﻿#pragma once
#include "Common.h"
#include <vector>
#include <filesystem>
#include "HString.h"

enum class FileEntryType
{
	Dir = 0,
	File = 1,
};

struct FileEntry
{
	HString absPath = " ";
	HString relativePath = " ";
	HString fileName = " ";
	HString baseName = " ";
	HString suffix = " ";
	FileEntryType type = FileEntryType::File;
};

#if __ANDROID__
extern "C" {
#endif

	class FileSystem
	{
	public:

		HBBR_API static HString GetProgramPath();
		HBBR_API static HString GetShaderCacheAbsPath();
		HBBR_API static HString GetAssetAbsPath();
		HBBR_API static HString GetContentAbsPath();
		HBBR_API static HString GetConfigAbsPath();
		HBBR_API static HString GetWorldAbsPath();
		//Fill up asset path (Asset/...)
		HBBR_API static HString FillUpAssetPath(HString assetPath);
		/* editor only */
		HBBR_API static HString GetShaderIncludeAbsPath();
		//
		HBBR_API static HString GetRelativePath(const char* path);
		HBBR_API static uint32_t GetPathFileNum(const char* path);
		HBBR_API static bool FileExist(const char* path);
		HBBR_API static bool IsDir(const char* path);
		HBBR_API static bool CreateDir(const char* path);
		HBBR_API static bool CreateDirSymlink(const char* createPath, const char* linkTo);
		HBBR_API static bool IsNormalFile(const char* path);
		HBBR_API static void FileCopy(const char* srcFile, const char* newPath);
		HBBR_API static bool FileRemove(const char* path);
		HBBR_API static void FileRename(const char* src , const char* dst);
		HBBR_API static uint64_t GetFileSize(const char* path);
		HBBR_API static HString CorrectionPath(const char* path);
		HBBR_API static void CorrectionPath(HString& path);
		HBBR_API static void NormalizePath(HString& path);
		HBBR_API static HString GetFilePath(HString path);
		HBBR_API static HString GetFileName(HString path);
		HBBR_API static HString GetBaseName(HString path);
		HBBR_API static HString GetFileExt(HString path);
		static std::vector<FileEntry> GetFilesBySuffix(const char* path, const char* suffix);
		static std::vector<char>ReadBinaryFile(const char* filePath);
		static HString _appPath;
	};

#if __ANDROID__
}
#endif