#include "bundle.h"
#include "base64.h"
#include "common.h"
#include "macho.h"
#include "sys/stat.h"
#include "sys/types.h"

ZBundle::ZBundle()
{
	m_pSignAsset = NULL;
	m_bForceSign = false;
	m_bWeakInject = false;
	m_bIconsChanged = false;  // NEW: Initialize icon change flag
}

bool ZBundle::FindAppFolder(const string& strFolder, string& strAppFolder)
{
	if (ZFile::IsPathSuffix(strFolder, ".app") || ZFile::IsPathSuffix(strFolder, ".appex")) {
		strAppFolder = strFolder;
		return true;
	}

	ZFile::EnumFolder(strFolder.c_str(), true, [&](bool bFolder, const string& strPath) {
		string strName = ZUtil::GetBaseName(strPath.c_str());
		if ("__MACOSX" == strName) {
			return true;
		}
		return false;
	}, [&](bool bFolder, const string& strPath) {
		if (bFolder) {
			if (ZFile::IsPathSuffix(strPath, ".app") || ZFile::IsPathSuffix(strPath, ".appex")) {
				strAppFolder = strPath;
				return true;
			}
		}
		return false;
	});

	return (!strAppFolder.empty());
}

bool ZBundle::GetSignFolderInfo(const string& strFolder, jvalue& jvNode, bool bGetName)
{
	string strInfoPlistData;
	string strInfoPlistPath = strFolder + "/Info.plist";
	ZFile::ReadFile(strInfoPlistPath.c_str(), strInfoPlistData);

	jvalue jvInfo;
	jvInfo.read_plist(strInfoPlistData);
	string strBundleId = jvInfo["CFBundleIdentifier"];
	string strBundleExe = jvInfo["CFBundleExecutable"];
	string strBundleVersion = jvInfo["CFBundleVersion"];
	if (strBundleId.empty() || strBundleExe.empty()) {
		return false;
	}

	string strInfoSHA1;
	string strInfoSHA256;
	ZSHA::SHABase64(strInfoPlistData, strInfoSHA1, strInfoSHA256);

	jvNode["bundle_id"] = strBundleId;
	jvNode["bundle_version"] = strBundleVersion;
	jvNode["bundle_executable"] = strBundleExe;
	jvNode["sha1"] = strInfoSHA1;
	jvNode["sha256"] = strInfoSHA256;
	if (!jvNode.has("path")) {
		jvNode["path"] = strFolder.substr(m_strAppFolder.size() + 1);
	}

	if (bGetName) {
		string strBundleName = jvInfo["CFBundleDisplayName"];
		if (strBundleName.empty()) {
			strBundleName = jvInfo["CFBundleName"].as_cstr();
		}
		jvNode["name"] = strBundleName;
	}

	return true;
}

bool ZBundle::GetObjectsToSign(const string& strFolder, jvalue& jvInfo)
{
	vector<string> allBundles;
	
	std::function<void(const string&)> findAllBundles = [&](const string& currentPath) {
		ZFile::EnumFolder(currentPath.c_str(), false, NULL, [&](bool bFolder, const string& strPath) {
			if (bFolder) {
				if (ZFile::IsPathSuffix(strPath, ".app") ||
					ZFile::IsPathSuffix(strPath, ".appex") ||
					ZFile::IsPathSuffix(strPath, ".framework") ||
					ZFile::IsPathSuffix(strPath, ".xctest")) {
					allBundles.push_back(strPath);
					findAllBundles(strPath);
				} else {
					findAllBundles(strPath);
				}
			}
			return false;
		});
	};
	
	findAllBundles(strFolder);
	
	sort(allBundles.begin(), allBundles.end(), [](const string& a, const string& b) {
		size_t depthA = count(a.begin(), a.end(), '/');
		size_t depthB = count(b.begin(), b.end(), '/');
		// deeper paths first
		return depthA > depthB;
	});
	
	for (const string& bundlePath : allBundles) {
		jvalue jvNode;
		if (GetSignFolderInfo(bundlePath, jvNode)) {
			jvInfo["folders"].push_back(jvNode);
		}
	}
	
	ZFile::EnumFolder(strFolder.c_str(), true, NULL, [&](bool bFolder, const string& strPath) {
		if (!bFolder && ZFile::IsPathSuffix(strPath, ".dylib")) {
			jvInfo["files"].push_back(strPath.substr(m_strAppFolder.size() + 1));
		}
		return false;
	});
	
	return true;
}

// NEW METHOD: Check if icon files or Assets.car have changed in main app
bool ZBundle::HasIconsChanged(const string& strFolder, const jvalue& jvCachedInfo)
{
	bool bIconsChanged = false;

	// Read current Info.plist to get icon information
	jvalue jvInfo;
	if (jvInfo.read_plist_from_file("%s/Info.plist", strFolder.c_str())) {
		// Get icon files from Info.plist
		vector<string> iconFiles;
		GetIconFilesFromPlist(jvInfo, iconFiles);

		// Check if any icon files have changed
		for (const string& iconFile : iconFiles) {
			string strFullPath = strFolder + "/" + iconFile;
			if (ZFile::IsFileExists(strFullPath.c_str())) {
				string strCurrentSHA1;
				string strCurrentSHA256;
				ZSHA::SHABase64File(strFullPath.c_str(), strCurrentSHA1, strCurrentSHA256);
				
				// Check against cached info
				if (jvCachedInfo.has("files") && jvCachedInfo["files"].has(iconFile)) {
					string strCachedHash = jvCachedInfo["files"][iconFile].as_string();
					if (strCachedHash.find("data:") == 0) {
						strCachedHash = strCachedHash.substr(5); // Remove "data:" prefix
					}
					if (strCachedHash != strCurrentSHA1) {
						ZLog::PrintV(">>> Icon changed: %s\n", iconFile.c_str());
						bIconsChanged = true;
					}
				} else {
					// Icon file exists but not in cache, so it's new
					ZLog::PrintV(">>> New icon found: %s\n", iconFile.c_str());
					bIconsChanged = true;
				}
			}
		}
	}

	// CRITICAL: Check Assets.car file which contains compiled icons
	string strAssetsCarPath = strFolder + "/Assets.car";
	if (ZFile::IsFileExists(strAssetsCarPath.c_str())) {
		string strCurrentSHA1;
		string strCurrentSHA256;
		ZSHA::SHABase64File(strAssetsCarPath.c_str(), strCurrentSHA1, strCurrentSHA256);
		
		// Check against cached info
		if (jvCachedInfo.has("files") && jvCachedInfo["files"].has("Assets.car")) {
			string strCachedHash = jvCachedInfo["files"]["Assets.car"].as_string();
			if (strCachedHash.find("data:") == 0) {
				strCachedHash = strCachedHash.substr(5); // Remove "data:" prefix
			}
			if (strCachedHash != strCurrentSHA1) {
				ZLog::PrintV(">>> Assets.car changed - compiled icons updated\n");
				bIconsChanged = true;
			}
		} else {
			// Assets.car exists but not in cache
			ZLog::PrintV(">>> New Assets.car found\n");
			bIconsChanged = true;
		}
	}

	// Check other common asset files that might contain icons
	vector<string> assetFiles = {
		"Assets.car",
		"Base.lproj/LaunchScreen.storyboard",
		"Base.lproj/Main.storyboard"
	};

	for (const string& assetFile : assetFiles) {
		string strAssetPath = strFolder + "/" + assetFile;
		if (ZFile::IsFileExists(strAssetPath.c_str())) {
			string strCurrentSHA1;
			string strCurrentSHA256;
			ZSHA::SHABase64File(strAssetPath.c_str(), strCurrentSHA1, strCurrentSHA256);
			
			// Check against cached info
			if (jvCachedInfo.has("files") && jvCachedInfo["files"].has(assetFile)) {
				string strCachedHash = jvCachedInfo["files"][assetFile].as_string();
				if (strCachedHash.find("data:") == 0) {
					strCachedHash = strCachedHash.substr(5); // Remove "data:" prefix
				}
				if (strCachedHash != strCurrentSHA1) {
					ZLog::PrintV(">>> Asset file changed: %s\n", assetFile.c_str());
					bIconsChanged = true;
				}
			}
		}
	}
	
	return bIconsChanged;
}

// NEW METHOD: Extract icon file names from Info.plist
void ZBundle::GetIconFilesFromPlist(const jvalue& jvInfo, vector<string>& iconFiles)
{
	// Check CFBundleIconFile (single icon file)
	if (jvInfo.has("CFBundleIconFile")) {
		string iconFile = jvInfo["CFBundleIconFile"].as_string();
		if (!iconFile.empty()) {
			// Add .png extension if not present
			if (!ZFile::IsPathSuffix(iconFile, ".png")) {
				iconFile += ".png";
			}
			iconFiles.push_back(iconFile);
		}
	}

	// Check CFBundleIconFiles (array of icon files)
	if (jvInfo.has("CFBundleIconFiles")) {
		const jvalue& jvIconFiles = jvInfo["CFBundleIconFiles"];
		if (jvIconFiles.is_array()) {
			for (size_t i = 0; i < jvIconFiles.size(); i++) {
				string iconFile = jvIconFiles[i].as_string();
				if (!iconFile.empty()) {
					// Add .png extension if not present
					if (!ZFile::IsPathSuffix(iconFile, ".png")) {
						iconFile += ".png";
					}
					iconFiles.push_back(iconFile);
				}
			}
		}
	}

	// Check CFBundleIcons (iOS style icons)
	if (jvInfo.has("CFBundleIcons")) {
		const jvalue& jvIcons = jvInfo["CFBundleIcons"];
		if (jvIcons.has("CFBundlePrimaryIcon")) {
			const jvalue& jvPrimaryIcon = jvIcons["CFBundlePrimaryIcon"];
			if (jvPrimaryIcon.has("CFBundleIconFiles")) {
				const jvalue& jvIconFiles = jvPrimaryIcon["CFBundleIconFiles"];
				if (jvIconFiles.is_array()) {
					for (size_t i = 0; i < jvIconFiles.size(); i++) {
						string iconFile = jvIconFiles[i].as_string();
						if (!iconFile.empty()) {
							// Add .png extension if not present
							if (!ZFile::IsPathSuffix(iconFile, ".png")) {
								iconFile += ".png";
							}
							iconFiles.push_back(iconFile);
						}
					}
				}
			}
		}
	}

	// Check CFBundleIcons~ipad (iPad specific icons)
	if (jvInfo.has("CFBundleIcons~ipad")) {
		const jvalue& jvIcons = jvInfo["CFBundleIcons~ipad"];
		if (jvIcons.has("CFBundlePrimaryIcon")) {
			const jvalue& jvPrimaryIcon = jvIcons["CFBundlePrimaryIcon"];
			if (jvPrimaryIcon.has("CFBundleIconFiles")) {
				const jvalue& jvIconFiles = jvPrimaryIcon["CFBundleIconFiles"];
				if (jvIconFiles.is_array()) {
					for (size_t i = 0; i < jvIconFiles.size(); i++) {
						string iconFile = jvIconFiles[i].as_string();
						if (!iconFile.empty()) {
							// Add .png extension if not present
							if (!ZFile::IsPathSuffix(iconFile, ".png")) {
								iconFile += ".png";
							}
							iconFiles.push_back(iconFile);
						}
					}
				}
			}
		}
	}

	// Add common icon files that might not be in Info.plist
	vector<string> commonIcons = {
		"Icon.png", "Icon@2x.png", "Icon@3x.png",
		"Icon-60.png", "Icon-60@2x.png", "Icon-60@3x.png",
		"Icon-76.png", "Icon-76@2x.png",
		"Icon-Small.png", "Icon-Small@2x.png", "Icon-Small@3x.png",
		"Icon-Small-40.png", "Icon-Small-40@2x.png", "Icon-Small-40@3x.png",
		"Icon-83.5@2x.png", "Icon-1024.png",
		"AppIcon20x20.png", "AppIcon20x20@2x.png", "AppIcon20x20@3x.png",
		"AppIcon29x29.png", "AppIcon29x29@2x.png", "AppIcon29x29@3x.png",
		"AppIcon40x40.png", "AppIcon40x40@2x.png", "AppIcon40x40@3x.png",
		"AppIcon60x60@2x.png", "AppIcon60x60@3x.png",
		"AppIcon76x76.png", "AppIcon76x76@2x.png",
		"AppIcon83.5x83.5@2x.png", "AppIcon1024x1024.png"
	};

	for (const string& commonIcon : commonIcons) {
		string strFullPath = m_strAppFolder + "/" + commonIcon;
		if (ZFile::IsFileExists(strFullPath.c_str())) {
			// Check if not already in the list
			bool found = false;
			for (const string& existing : iconFiles) {
				if (existing == commonIcon) {
					found = true;
					break;
				}
			}
			if (!found) {
				iconFiles.push_back(commonIcon);
			}
		}
	}
}

// NEW METHOD: Force Assets.car regeneration by touching related files
bool ZBundle::ForceAssetsCarRegeneration(const string& strFolder)
{
	string strAssetsCarPath = strFolder + "/Assets.car";
	if (!ZFile::IsFileExists(strAssetsCarPath.c_str())) {
		ZLog::PrintV(">>> No Assets.car found - using individual icon files\n");
		return true; // No Assets.car to worry about
	}

	ZLog::PrintV(">>> Found Assets.car - removing to force icon regeneration...\n");
	
	// Always try to remove Assets.car to ensure new icons are used
	// This forces the system to use the new icon files instead of cached compiled icons
	if (ZFile::RemoveFile(strAssetsCarPath.c_str())) {
		ZLog::PrintV(">>> Removed old Assets.car - new icons will be used\n");
		return true;
	} else {
		ZLog::WarnV(">>> Could not remove Assets.car - continuing with existing compiled icons\n");
		return false;
	}
}

bool ZBundle::GenerateCodeResources(const string& strFolder, jvalue& jvCodeRes)
{
	set<string> setFiles;
	ZFile::EnumFolder(strFolder.c_str(), true, NULL, [&](bool bFolder, const string& strPath) {
		if (!bFolder) {
			string strNode = strPath.substr(strFolder.size() + 1);
			ZUtil::StringReplace(strNode, "\\", "/");
			setFiles.insert(strNode);
		}
		return false;
	});

	jvalue jvInfo;
	jvInfo.read_plist_from_file("%s/Info.plist", strFolder.c_str());
	string strBundleExe = jvInfo["CFBundleExecutable"];

#ifdef _WIN32
	iconv ic;
	strBundleExe = ic.U82A(strBundleExe);
#endif

	setFiles.erase("_CodeSignature/CodeResources");
	setFiles.erase(strBundleExe);
	
	jvCodeRes.clear();
	jvCodeRes["files"] = jvalue(jvalue::E_OBJECT);
	jvCodeRes["files2"] = jvalue(jvalue::E_OBJECT);

	for (string strKey : setFiles) {
		string strFile = strFolder + "/" + strKey;
		string strSHA1Base64;
		string strSHA256Base64;
		ZSHA::SHABase64File(strFile.c_str(), strSHA1Base64, strSHA256Base64);

#ifdef _WIN32
		strKey = ic.A2U8(strKey);
#endif

		bool bomit1 = false;
		bool bomit2 = false;

		if (ZFile::IsPathSuffix(strKey, ".lproj/locversion.plist")) {
			bomit1 = true;
			bomit2 = true;
		}

		if (ZFile::IsPathSuffix(strKey, ".DS_Store") || "Info.plist" == strKey || "PkgInfo" == strKey) {
			bomit2 = true;
		}

		if (!bomit1) {
			if (string::npos != strKey.rfind(".lproj/")) {
				jvCodeRes["files"][strKey]["hash"] = "data:" + strSHA1Base64;
				jvCodeRes["files"][strKey]["optional"] = true;
			} else {
				jvCodeRes["files"][strKey] = "data:" + strSHA1Base64;
			}
		}

		if (!bomit2) {
			jvCodeRes["files2"][strKey]["hash"] = "data:" + strSHA1Base64;
			jvCodeRes["files2"][strKey]["hash2"] = "data:" + strSHA256Base64;
			if (string::npos != strKey.rfind(".lproj/")) {
				jvCodeRes["files2"][strKey]["optional"] = true;
			}
		}
	}

	jvCodeRes["rules"]["^.*"] = true;
	jvCodeRes["rules"]["^.*\\.lproj/"]["optional"] = true;
	jvCodeRes["rules"]["^.*\\.lproj/"]["weight"] = 1000.0;
	jvCodeRes["rules"]["^.*\\.lproj/locversion.plist$"]["omit"] = true;
	jvCodeRes["rules"]["^.*\\.lproj/locversion.plist$"]["weight"] = 1100.0;
	jvCodeRes["rules"]["^Base\\.lproj/"]["weight"] = 1010.0;
	jvCodeRes["rules"]["^version.plist$"] = true;

	jvCodeRes["rules2"]["^.*"] = true;
	jvCodeRes["rules2"][".*\\.dSYM($|/)"]["weight"] = 11.0;
	jvCodeRes["rules2"]["^(.*/)?\\.DS_Store$"]["omit"] = true;
	jvCodeRes["rules2"]["^(.*/)?\\.DS_Store$"]["weight"] = 2000.0;
	jvCodeRes["rules2"]["^.*\\.lproj/"]["optional"] = true;
	jvCodeRes["rules2"]["^.*\\.lproj/"]["weight"] = 1000.0;
	jvCodeRes["rules2"]["^.*\\.lproj/locversion.plist$"]["omit"] = true;
	jvCodeRes["rules2"]["^.*\\.lproj/locversion.plist$"]["weight"] = 1100.0;
	jvCodeRes["rules2"]["^Base\\.lproj/"]["weight"] = 1010.0;
	jvCodeRes["rules2"]["^Info\\.plist$"]["omit"] = true;
	jvCodeRes["rules2"]["^Info\\.plist$"]["weight"] = 20.0;
	jvCodeRes["rules2"]["^PkgInfo$"]["omit"] = true;
	jvCodeRes["rules2"]["^PkgInfo$"]["weight"] = 20.0;
	jvCodeRes["rules2"]["^embedded\\.provisionprofile$"]["weight"] = 20.0;
	jvCodeRes["rules2"]["^version\\.plist$"]["weight"] = 20.0;

	return true;
}

void ZBundle::GetChangedFiles(jvalue& jvNode, vector<string>& arrChangedFiles)
{
	if (jvNode.has("files")) {
		for (size_t i = 0; i < jvNode["files"].size(); i++) {
			arrChangedFiles.push_back(jvNode["files"][i]);
		}
	}

	if (jvNode.has("folders")) {
		for (size_t i = 0; i < jvNode["folders"].size(); i++) {
			jvalue& jvSubNode = jvNode["folders"][i];
			GetChangedFiles(jvSubNode, arrChangedFiles);
			string strPath = jvSubNode["path"];
			arrChangedFiles.push_back(strPath + "/_CodeSignature/CodeResources");
			arrChangedFiles.push_back(strPath + "/" + jvSubNode["bundle_executable"].as_string());
		}
	}
}

void ZBundle::GetNodeChangedFiles(jvalue& jvNode)
{
	if (jvNode.has("folders")) {
		for (size_t i = 0; i < jvNode["folders"].size(); i++) {
			GetNodeChangedFiles(jvNode["folders"][i]);
		}
	}

	vector<string> arrChangedFiles;
	GetChangedFiles(jvNode, arrChangedFiles);
	for (size_t i = 0; i < arrChangedFiles.size(); i++) {
		jvNode["changed"].push_back(arrChangedFiles[i]);
	}

	if ("/" == jvNode["path"]) { // root
		jvNode["changed"].push_back("embedded.mobileprovision");
	}
}

bool ZBundle::SignNode(jvalue& jvNode)
{
	if (jvNode.has("files")) {
		for (size_t i = 0; i < jvNode["files"].size(); i++) {
			string strFile = jvNode["files"][i];
			ZLog::PrintV(">>> SignFile: \t%s\n", strFile.c_str());
			ZMachO macho;
			if (macho.InitV("%s/%s", m_strAppFolder.c_str(), strFile.c_str())) {
				if (!macho.Sign(m_pSignAsset, m_bForceSign, "", "", "", "")) {
					return false;
				}
			} else {
				return false;
			}
		}
	}
	
	if (jvNode.has("folders")) {
		for (size_t i = 0; i < jvNode["folders"].size(); i++) {
			if (!SignNode(jvNode["folders"][i])) {
				return false;
			}
		}
	}

	jbase64 b64;
	string strInfoSHA1;
	string strInfoSHA256;
	string strFolder = jvNode["path"];
	string strBundleId = jvNode["bundle_id"];
	string strBundleExe = jvNode["bundle_executable"];
	b64.decode(jvNode["sha1"].as_cstr(), strInfoSHA1);
	b64.decode(jvNode["sha256"].as_cstr(), strInfoSHA256);
	if (strBundleId.empty() || strBundleExe.empty() || strInfoSHA1.empty() ||
		strInfoSHA256.empty()) {
		ZLog::ErrorV(">>> Can't get BundleID or BundleExecute or Info.plist SHASum in Info.plist! %s\n", strFolder.c_str());
		return false;
	}

#ifdef _WIN32
	iconv ic;
	strBundleExe = ic.U82A(strBundleExe);
#endif

	string strBaseFolder = m_strAppFolder;
	if ("/" != strFolder) {
		strBaseFolder += "/";
		strBaseFolder += strFolder;
	}

	string strExePath = strBaseFolder + "/" + strBundleExe;
	ZLog::PrintV(">>> SignFolder: %s, (%s)\n", ("/" == strFolder) ? ZUtil::GetBaseName(m_strAppFolder.c_str()) : strFolder.c_str(), strBundleExe.c_str());

	ZMachO macho;
	if (!macho.Init(strExePath.c_str())) {
		ZLog::ErrorV(">>> Can't parse BundleExecute file! %s\n", strExePath.c_str());
		return false;
	}

	ZFile::CreateFolderV("%s/_CodeSignature", strBaseFolder.c_str());
	string strCodeResFile = strBaseFolder + "/_CodeSignature/CodeResources";

	jvalue jvCodeRes;
	bool bForceRegenerate = m_bForceSign || m_bIconsChanged;  // Force regenerate if icons changed globally
	
	if (!bForceRegenerate) {
		jvCodeRes.read_plist_from_file(strCodeResFile.c_str());
	}

	if (bForceRegenerate || jvCodeRes.is_null()) { // create/regenerate
		if (!GenerateCodeResources(strBaseFolder, jvCodeRes)) {
			ZLog::ErrorV(">>> Create CodeResources failed! %s\n", strBaseFolder.c_str());
			return false;
		}
		ZLog::PrintV(">>> CodeResources regenerated with current icons\n");
	} else if (jvNode.has("changed")) { // use existing but update changed files
		for (size_t i = 0; i < jvNode["changed"].size(); i++) {
			string strFile = jvNode["changed"][i].as_cstr();
			string strRealFile = m_strAppFolder + "/" + strFile;

			string strFileSHA1;
			string strFileSHA256;
			if (!ZSHA::SHABase64File(strRealFile.c_str(), strFileSHA1, strFileSHA256)) {
				ZLog::ErrorV(">>> Can't get changed file SHASum! %s", strFile.c_str());
				return false;
			}

			string strKey = strFile;
			if ("/" != strFolder) {
				strKey = strFile.substr(strFolder.size() + 1);
			}

			jvCodeRes["files"][strKey] = "data:" + strFileSHA1;
			jvCodeRes["files2"][strKey]["hash"] = "data:" + strFileSHA1;
			jvCodeRes["files2"][strKey]["hash2"] = "data:" + strFileSHA256;

			ZLog::DebugV("\t\tChanged file: %s, %s\n", strFileSHA1.c_str(), strKey.c_str());
		}
	}

	string strCodeResData;
	jvCodeRes.style_write_plist(strCodeResData);
	if (!ZFile::WriteFile(strCodeResFile.c_str(), strCodeResData)) {
		ZLog::ErrorV("\tWriting CodeResources failed! %s\n", strCodeResFile.c_str());
		return false;
	}

	bool bForceSign = m_bForceSign || bForceRegenerate;
	if ("/" == strFolder) { // inject dylib
		for (const string& strDylibFile : m_arrInjectDylibs) {
			if (macho.InjectDylib(m_bWeakInject, strDylibFile.c_str())) {
				bForceSign = true;
			}
		}
	}

	if (!macho.Sign(m_pSignAsset, bForceSign, strBundleId, strInfoSHA1, strInfoSHA256, strCodeResData)) {
		return false;
	}

	return true;
}

bool ZBundle::ModifyPluginsBundleId(const string& strOldBundleId, const string& strNewBundleId)
{
	vector<string> arrFolders;
	ZFile::EnumFolder(m_strAppFolder.c_str(), true, NULL, [&](bool bFolder, const string& strPath) {
		if (bFolder) {
			if (ZFile::IsPathSuffix(strPath, ".app") || ZFile::IsPathSuffix(strPath, ".appex")) {
				arrFolders.push_back(strPath);
			}
		}
		return false;
	});

	for (const string& strFolder: arrFolders) {
		jvalue jvInfo;
		if (!jvInfo.read_plist_from_file("%s/Info.plist", strFolder.c_str())) {
			ZLog::WarnV(">>> Can't find Plugin's Info.plist! %s\n", strFolder.c_str());
			continue;
		}

		string strOldPIBundleID = jvInfo["CFBundleIdentifier"];
		string strNewPIBundleID = strOldPIBundleID;
		ZUtil::StringReplace(strNewPIBundleID, strOldBundleId, strNewBundleId);
		jvInfo["CFBundleIdentifier"] = strNewPIBundleID;
		ZLog::PrintV(">>> BundleId: \t%s -> %s, Plugin\n", strOldPIBundleID.c_str(), strNewPIBundleID.c_str());

		if (jvInfo.has("WKCompanionAppBundleIdentifier")) {
			string strOldWKCBundleID = jvInfo["WKCompanionAppBundleIdentifier"];
			string strNewWKCBundleID = strOldWKCBundleID;
			ZUtil::StringReplace(strNewWKCBundleID, strOldBundleId, strNewBundleId);
			jvInfo["WKCompanionAppBundleIdentifier"] = strNewWKCBundleID;
			ZLog::PrintV(">>> BundleId: \t%s -> %s, Plugin-WKCompanionAppBundleIdentifier\n", strOldWKCBundleID.c_str(), strNewWKCBundleID.c_str());
		}

		if (jvInfo.has("NSExtension")) {
			if (jvInfo["NSExtension"].has("NSExtensionAttributes")) {
				if (jvInfo["NSExtension"]["NSExtensionAttributes"].has("WKAppBundleIdentifier")) {
					string strOldWKBundleID = jvInfo["NSExtension"]["NSExtensionAttributes"]["WKAppBundleIdentifier"];
					string strNewWKBundleID = strOldWKBundleID;
					ZUtil::StringReplace(strNewWKBundleID, strOldBundleId, strNewBundleId);
					jvInfo["NSExtension"]["NSExtensionAttributes"]["WKAppBundleIdentifier"] = strNewWKBundleID;
					ZLog::PrintV(">>> BundleId: \t%s -> %s, NSExtension-NSExtensionAttributes-WKAppBundleIdentifier\n", strOldWKBundleID.c_str(), strNewWKBundleID.c_str());
				}
			}
		}

		jvInfo.style_write_plist_to_file("%s/Info.plist", strFolder.c_str());
	}

	return true;
}

bool ZBundle::ModifyBundleInfo(const string& strBundleId, const string& strBundleVersion, const string& strDisplayName)
{
	jvalue jvInfo;
	if (!jvInfo.read_plist_from_file("%s/Info.plist", m_strAppFolder.c_str())) {
		ZLog::ErrorV(">>> Can't find app's Info.plist! %s\n", m_strAppFolder.c_str());
		return false;
	}

	if (!strBundleId.empty()) {
		string strOldBundleId = jvInfo["CFBundleIdentifier"];
		jvInfo["CFBundleIdentifier"] = strBundleId;
		ZLog::PrintV(">>> BundleId: \t%s -> %s\n", strOldBundleId.c_str(), strBundleId.c_str());
		ModifyPluginsBundleId(strOldBundleId, strBundleId);
	}

	if (!strDisplayName.empty()) {

		string strNewDisplayName = strDisplayName;

#ifdef _WIN32
		iconv ic;
		strNewDisplayName = ic.A2U8(strDisplayName);
#endif

		string strOldDisplayName = jvInfo["CFBundleDisplayName"];
		if (strOldDisplayName.empty()) {
			strOldDisplayName = jvInfo["CFBundleName"].as_cstr();
		}

		jvInfo["CFBundleName"] = strNewDisplayName;
		jvInfo["CFBundleDisplayName"] = strNewDisplayName;

		jvalue jvInfoStrings;
		if (jvInfoStrings.read_plist_from_file("%s/zh_CN.lproj/InfoPlist.strings", m_strAppFolder.c_str())) {
			jvInfoStrings["CFBundleName"] = strNewDisplayName;
			jvInfoStrings["CFBundleDisplayName"] = strNewDisplayName;
			jvInfoStrings.style_write_plist_to_file("%s/zh_CN.lproj/InfoPlist.strings", m_strAppFolder.c_str());
		}

		jvInfoStrings.clear();
		if (jvInfoStrings.read_plist_from_file("%s/zh-Hans.lproj/InfoPlist.strings", m_strAppFolder.c_str())) {
			jvInfoStrings["CFBundleName"] = strNewDisplayName;
			jvInfoStrings["CFBundleDisplayName"] = strNewDisplayName;
			jvInfoStrings.style_write_plist_to_file("%s/zh-Hans.lproj/InfoPlist.strings", m_strAppFolder.c_str());
		}

#ifdef _WIN32
		strOldDisplayName = ic.U82A(strOldDisplayName);
		strNewDisplayName = ic.U82A(strNewDisplayName);
#endif

		ZLog::PrintV(">>> BundleName: %s -> %s\n", strOldDisplayName.c_str(), strNewDisplayName.c_str());
	}

	if (!strBundleVersion.empty()) {
		string strOldBundleVersion = jvInfo["CFBundleVersion"];
		jvInfo["CFBundleVersion"] = strBundleVersion;
		jvInfo["CFBundleShortVersionString"] = strBundleVersion;
		ZLog::PrintV(">>> BundleVersion: %s -> %s\n", strOldBundleVersion.c_str(), strBundleVersion.c_str());
	}

	jvInfo.style_write_plist_to_file("%s/Info.plist", m_strAppFolder.c_str());
	return true;
}

bool ZBundle::SignFolder(ZSignAsset* pSignAsset,
							const string& strFolder,
							const string& strBundleId,
							const string& strBundleVersion,
							const string& strDisplayName,
							const vector<string>& arrInjectDylibs,
							bool bForce,
							bool bWeakInject,
							bool bEnableCache)
{
	m_bForceSign = bForce;
	m_pSignAsset = pSignAsset;
	m_bWeakInject = bWeakInject;
	if (NULL == m_pSignAsset) {
		return false;
	}

	if (!FindAppFolder(strFolder, m_strAppFolder)) {
		ZLog::ErrorV(">>> Can't find app folder! %s\n", strFolder.c_str());
		return false;
	}

	if (!strBundleId.empty() || !strDisplayName.empty() || !strBundleVersion.empty()) {
		m_bForceSign = true;
		if (!ModifyBundleInfo(strBundleId, strBundleVersion, strDisplayName)) {
			return false;
		}
	}

	ZFile::RemoveFileV("%s/embedded.mobileprovision", m_strAppFolder.c_str());
	if (!pSignAsset->m_strProvData.empty()) {
		if (!ZFile::WriteFileV(pSignAsset->m_strProvData, "%s/embedded.mobileprovision", m_strAppFolder.c_str())) { // embedded.mobileprovision
			ZLog::ErrorV(">>> Can't write embedded.mobileprovision!\n");
			return false;
		}
	}

	if (!arrInjectDylibs.empty()) {
		m_bForceSign = true;
		for (const string& strDylibFile : arrInjectDylibs) {
			string strFileName = ZUtil::GetBaseName(strDylibFile.c_str());
			if (ZFile::CopyFileV(strDylibFile.c_str(), "%s/%s", m_strAppFolder.c_str(), strFileName.c_str())) {
				m_arrInjectDylibs.push_back("@executable_path/" + strFileName);
			}
		}
	}

	string strCacheName;
	ZSHA::SHA1Text(m_strAppFolder, strCacheName);
	if (!ZFile::IsFileExistsV("./.zsign_cache/%s.json", strCacheName.c_str())) {
		m_bForceSign = true;
	}

	jvalue jvRoot;
	bool bIconsChanged = false;
	
	// ALWAYS check and handle Assets.car, regardless of force sign status
	ForceAssetsCarRegeneration(m_strAppFolder);
	
	// MODIFICATION: Check if icons have changed in main app (sets global flag for all bundles)
	if (!m_bForceSign) {
		jvalue jvCachedRoot;
		if (jvCachedRoot.read_from_file("./.zsign_cache/%s.json", strCacheName.c_str())) {
			if (HasIconsChanged(m_strAppFolder, jvCachedRoot)) {
				ZLog::PrintV(">>> App icons or Assets.car changed, forcing regeneration for all bundles...\n");
				bIconsChanged = true;
				m_bIconsChanged = true;  // Set global flag to force regeneration for ALL bundles
				m_bForceSign = true;
			}
		}
	} else {
		// When force signing, also set the icons changed flag to ensure all bundles regenerate
		m_bIconsChanged = true;
		bIconsChanged = true;
	}

	if (m_bForceSign) {
		jvRoot["path"] = "/";
		jvRoot["root"] = m_strAppFolder;
		if (!GetSignFolderInfo(m_strAppFolder, jvRoot, true)) {
			ZLog::ErrorV(">>> Can't get BundleID, BundleVersion, or BundleExecute in Info.plist! %s\n", m_strAppFolder.c_str());
			return false;
		}
		if (!GetObjectsToSign(m_strAppFolder, jvRoot)) {
			return false;
		}
		GetNodeChangedFiles(jvRoot);
	} else {
		jvRoot.read_from_file("./.zsign_cache/%s.json", strCacheName.c_str());
	}

	string strAppName = jvRoot["name"];

#ifdef _WIN32
	iconv ic;
	strAppName = ic.U82A(strAppName);
#endif

	ZLog::PrintV(">>> Signing: \t%s ...\n", m_strAppFolder.c_str());
	ZLog::PrintV(">>> AppName: \t%s\n", strAppName.c_str());
	ZLog::PrintV(">>> BundleId: \t%s\n", jvRoot["bundle_id"].as_cstr());
	ZLog::PrintV(">>> Version: \t%s\n", jvRoot["bundle_version"].as_cstr());
	ZLog::PrintV(">>> TeamId: \t%s\n", m_pSignAsset->m_strTeamId.c_str());
	ZLog::PrintV(">>> SubjectCN: \t%s\n", m_pSignAsset->m_strSubjectCN.c_str());
	ZLog::PrintV(">>> ReadCache: \t%s\n", m_bForceSign ? "NO" : "YES");
	ZLog::PrintV(">>> AssetsCarHandled: \tYES\n");
	if (bIconsChanged) {
		ZLog::PrintV(">>> IconsChanged: \tYES\n");
	}
	// Remove embedded.mobileprovision after signing is finished, with command log and check if found
	string provPath = m_strAppFolder + "/embedded.mobileprovision";
	if (ZFile::IsFileExists(provPath.c_str())) {
		if (ZFile::RemoveFileV("%s/embedded.mobileprovision", m_strAppFolder.c_str())) {
			ZLog::PrintV(">>> Removed embedded.mobileprovision: %s\n", provPath.c_str());
		} else {
			ZLog::WarnV(">>> Failed to remove embedded.mobileprovision: %s\n", provPath.c_str());
		}
	} else {
		ZLog::PrintV(">>> embedded.mobileprovision not found, nothing to remove: %s\n", provPath.c_str());
	}

	if (SignNode(jvRoot)) {
		if (bEnableCache) {
			ZFile::CreateFolder("./.zsign_cache");
			jvRoot.style_write_to_file("./.zsign_cache/%s.json", strCacheName.c_str());
		}
		return true;
	}

	return false;
}