#pragma once
#include "common.h"
#include "json.h"
#include "openssl.h"
#include <vector>

class ZBundle
{
public:
	ZBundle();
	
public:
	bool SignFolder(ZSignAsset* pSignAsset,
					const string& strFolder,
					const string& strBundleId,
					const string& strBundleVersion,
					const string& strDisplayName,
					const vector<string>& arrDylibFiles,
					bool bForce,
					bool bWeakInject,
					bool bEnableCache);

private:
	bool SignNode(jvalue& jvNode);
	void GetNodeChangedFiles(jvalue& jvNode);
	void GetChangedFiles(jvalue& jvNode, vector<string>& arrChangedFiles);
	bool ModifyPluginsBundleId(const string& strOldBundleId, const string& strNewBundleId);
	bool ModifyBundleInfo(const string& strBundleId, const string& strBundleVersion, const string& strDisplayName);

private:
	bool FindAppFolder(const string& strFolder, string& strAppFolder);
	bool GetObjectsToSign(const string& strFolder, jvalue& jvInfo);
	bool GetSignFolderInfo(const string& strFolder, jvalue& jvNode, bool bGetName = false);

private:
	bool GenerateCodeResources(const string& strFolder, jvalue& jvCodeRes);
	
	// NEW METHODS: Icon and Assets.car handling functions
	bool HasIconsChanged(const string& strFolder, const jvalue& jvCachedInfo);
	void GetIconFilesFromPlist(const jvalue& jvInfo, vector<string>& iconFiles);
	bool ForceAssetsCarRegeneration(const string& strFolder);

private:
	bool			m_bForceSign;
	bool			m_bWeakInject;
	bool			m_bIconsChanged;  // NEW: Global flag to force regeneration when icons change
	ZSignAsset*		m_pSignAsset;
	vector<string>	m_arrInjectDylibs;

public:
	string			m_strAppFolder;
};