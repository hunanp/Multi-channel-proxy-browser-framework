// FavoriteThings.cpp : Defines the entry point for the console application.
#include "FavoriteItem.h"
#include <cstdlib>
#include <sstream>
#include "strCoding.h"
#include <windows.h>
#include <windowsx.h>
#include "WinInet.h"

#pragma comment(lib,"wininet.lib")
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif
#include "ximage.h"
#include "png.h"
#pragma comment(lib, "png.lib")
#pragma comment(lib, "cximage.lib")

namespace ImWindow
{
	static std::vector<FavoriteItem*> m_favoriteList;//保存收藏夾数据，拖动要用数组，所以这里用数组，数据量太大再另建一个Map,用来快速判断是否已存在
	static std::map<std::string, HostTexture*> hostTexture;//临时图标表，包括收藏夾和本次启动已访问过的网页标，快速访问用，不保存

	static std::string faviconFileName = "favicon.dat";
	FavoriteList::FavoriteList(bool loaddata)
	{
		fileName = "";
	}
	FavoriteList::~FavoriteList()
	{
		/*for (auto act : m_favoriteList)
			delete act;*/// m_favoriteList已经是全局变量，不能再在这删除
	}

	std::vector<FavoriteItem*>* FavoriteList::ItemList()
	{ 
		return &m_favoriteList;
	}
	bool FavoriteList::AddItem(string Url, string Title, string Hexstr, int id)
	{
		return true;
	}
	bool FavoriteList::ChangeItem(int id, string Url, string Title)
	{
		return false;
	}
	int FavoriteList::GetItemIndex(string Url, string Title)
	{
		int index = 0;
		for (vector<FavoriteItem*>::iterator it = m_favoriteList.begin(); it != m_favoriteList.end(); it++, index++)
		{
			if (((FavoriteItem*)(*it))->mUrl == Url)
				return index;
		}
		return -1;
	}
	bool FavoriteList::DeleteItem(int id, bool save)
	{
		return false;
	}
	
	void FavoriteList::DownloadIco(const char* url, bool downloadDomain)
	{
	}

	const unsigned char * loadfile(const std::string &file, int &size)
	{
		std::ifstream fs(file.c_str(), std::ios::binary);
		fs.seekg(0, std::ios::end);
		size =(int)fs.tellg();
		char * data = new char[size + 1];
		fs.seekg(0);
		fs.read(data, size);
		fs.close();
		data[size] = 0;
		return (unsigned char *)data;
	}

	void FavoriteList::DownIcoResultCallback(void *curl,bool success, const std::string& data, void* userdata)
	{
		
	}
	bool FavoriteList::LoadTextureFromFile(const char* filename, std::string hex_str, void** out_srv, int* out_width, int* out_height, ID3D11Device * DXDevice)
	{// Load from disk into a raw RGBA buffer
		int image_width = 0;
		int image_height = 0,components=0;
		unsigned char* image_data = NULL;
		if (hex_str.size()> 0)
		{
			std::string result;
			for (size_t i = 0; i < hex_str.length(); i += 2)
			{
				std::string byte = hex_str.substr(i, 2);
				char chr = (char)(int)strtol(byte.c_str(), NULL, 16);
				result.push_back(chr);
			}
			const BYTE* str1 = reinterpret_cast<const BYTE*>(result.c_str());
			image_data = stbi_load_from_memory(str1, result.length(), &image_width, &image_height, &components, 4);
		}
		
		if (image_data == NULL)
			return false;

		// Create texture
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = image_width;
		desc.Height = image_height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		ID3D11Texture2D *pTexture = NULL;
		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = image_data;
		subResource.SysMemPitch = desc.Width * 4;
		subResource.SysMemSlicePitch = 0;

		if (DXDevice != NULL)
		{
			DXDevice->CreateTexture2D(&desc, &subResource, &pTexture);

			// Create texture view
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));
			srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = desc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			//m_pDX11Device->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
			DXDevice->CreateShaderResourceView(pTexture, &srvDesc, (ID3D11ShaderResourceView**)out_srv);//out_srv占用内存，不释放会泄漏.用Release()解决
			pTexture->Release();
			*out_width = image_width;
			*out_height = image_height;

			stbi_image_free(image_data);
		}
		else
		{
			stbi_image_free(image_data);
			return false;
		}
		
		return true;
	}
	void FavoriteList::ChangeToCxImage(const void* buf,int sz)
	{
	}

	void FavoriteList::DownLoadFavoriteIco(const char* icourl, const char* url, string domain)
	{
		
	}
	void* FavoriteList::GetIcoTexture(const char* url,string domain)
	{
		return NULL;
	}
	void FavoriteList::ParserToImg(void* buf, int size, const char* url, void* curl)
	{
	}
}