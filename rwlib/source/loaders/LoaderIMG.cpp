#include <loaders/LoaderIMG.hpp>

#include <cstring>

LoaderIMG::LoaderIMG()
: m_version(GTAIIIVC)
, m_assetCount(0)
{

}

bool LoaderIMG::load(const std::string& filename)
{
	auto baseName = filename;
	auto extpos = filename.find(".img");
	if (extpos != std::string::npos)
	{
		baseName.erase(extpos);
	}
	auto dirName = baseName + ".dir";
	auto imgName = baseName + ".img";

	FILE* fp = fopen(dirName.c_str(), "rb");
	if(fp)
	{
		fseek(fp, 0, SEEK_END);
		unsigned long fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		m_assets.resize(fileSize / 32);

		m_assetCount = fileSize / 32;

		if( fread(&m_assets[0], sizeof(LoaderIMGFile), fileSize / 32, fp) == 0 )
		{
			std::cout << "No Records in IMG archive" << std::endl;
		}

		fclose(fp);
		m_archive = imgName;
		return true;
	}
	else
	{
		return false;
	}
}

/// Get the information of a asset in the examining archive
bool LoaderIMG::findAssetInfo(const std::string& assetname, LoaderIMGFile& out)
{
	for(size_t i = 0; i < m_assets.size(); ++i)
	{
		if(strcasecmp(m_assets[i].name, assetname.c_str()) == 0)
		{
			out = m_assets[i];
			return true;
		}
	}
	return false;
}

char* LoaderIMG::loadToMemory(const std::string& assetname)
{
	LoaderIMGFile assetInfo;
	bool found = false;
	for(size_t i = 0; i < m_assets.size(); ++i)
	{
		if(strcasecmp(m_assets[i].name, assetname.c_str()) == 0)
		{
			assetInfo = m_assets[i];
			found = true;
		}
	}

	if(!found)
	{
		std::cerr << "Asset '" << assetname << "' not found!" << std::endl;
		return 0;
	}
	
	std::string dirName = m_archive;

	FILE* fp = fopen(dirName.c_str(), "rb");
	if(fp)
	{
		char* raw_data = new char[assetInfo.size * 2048];

		fseek(fp, assetInfo.offset * 2048, SEEK_SET);
		if( fread(raw_data, 2048, assetInfo.size, fp) == 0 ) {
			std::cerr << "Error reading asset " << assetInfo.name << std::endl;
		}

		fclose(fp);
		return raw_data;
	}
	else
		return 0;
}

/// Writes the contents of assetname to filename
bool LoaderIMG::saveAsset(const std::string& assetname, const std::string& filename)
{
	char* raw_data = loadToMemory(assetname);
	if(!raw_data)
		return false;

	FILE* dumpFile = fopen(filename.c_str(), "wb");
	if(dumpFile)
	{
		LoaderIMGFile asset;
		if( findAssetInfo( assetname, asset ) )
		{
			fwrite(raw_data, asset.size * 2048, 1, dumpFile);
			printf("=> IMG: Saved %s to disk with filename %s\n", assetname.c_str(), filename.c_str());
		}
		fclose(dumpFile);

		delete[] raw_data;
		return true;
	}
	else
	{
		delete[] raw_data;
		return false;
	}
}

/// Get the information of an asset by its index
const LoaderIMGFile &LoaderIMG::getAssetInfoByIndex(size_t index) const
{
	return m_assets[index];
}


uint32_t LoaderIMG::getAssetCount() const
{
	return m_assetCount;
}
