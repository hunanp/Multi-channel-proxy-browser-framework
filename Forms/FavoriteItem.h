#ifndef FavoriteItem_H
#define FavoriteItem_H
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
//#include <curl/curl.h>
#include <d3d11.h>
#include <map>
//#include "ImwWindowManagerDX11.h"  //放这就不能包括到ImwWindowManager中去
using namespace std;
namespace ImWindow {
	typedef struct{
		int ID = 0;
		string mUrl="";
		string mDomain = "";
		string mTitle="";
		//string mIco;//保存真实的Ico网址,不用了
		string mHexstr="";
		void* mTexture=NULL;//用了这个编译带来很多问题
	}FavoriteItem;
	typedef struct{
		string mHexstr="";
		string mDomain = "";
		void* mTexture = NULL;
	}HostTexture;
	class FavoriteList
	{
	public:
		FavoriteList(bool loaddata=false);
		~FavoriteList();
		bool AddItem(string Url, string Title, string Hexstr, int id = -1);
		bool DeleteItem(int id,bool save = true);
		bool ChangeItem(int id, string Url, string Title);
		int  GetItemIndex(string Url, string Title);
        bool LoadTextureFromFile(const char* filename, std::string hex_str, void** out_srv, int* out_width, int* out_height, ID3D11Device * DXDevice=NULL);
		void DownloadIco(const char* url,bool downloadDomain=false);//优先按子域名，没成功回调后再按主域名
		void DownIcoResultCallback(void *curl, bool success, const std::string& data, void* userdata=NULL);
		std::vector<FavoriteItem*>* ItemList();

		static void DownLoadFavoriteIco(const char* icourl, const char* host, string domain="");
		static void* GetIcoTexture(const char* url, string domain);//有自动清理，如果保存到RootWindow，这边清理后指针就是伪指什了，改成保存domain直接从这里取
		void ChangeToCxImage(const void* buf,int sz);
		static void ParserToImg(void* buf,int size, const char* url,void* curl=NULL);//接收从WKE得到的图片，用CxImage收就行，应该就是文件流
	private:
		//std::vector<FavoriteItem*> favoriteList;
		string fileName;
		//std::map<int, ID3D11ShaderResourceView*> HostTexture;
		void* mEmptyTexture = NULL;//没有图标的情况
		void* mSettingTexture = NULL;
		char split ='|';//分隔符，单元分隔和字段分隔，保存文件时用， (char)0即‘\0’,'\v'==(char)11垂直制表
		bool isFirst = false;
	};
}
#endif
