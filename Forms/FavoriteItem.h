#ifndef FavoriteItem_H
#define FavoriteItem_H
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
//#include <curl/curl.h>
#include <d3d11.h>
#include <map>
//#include "ImwWindowManagerDX11.h"  //����Ͳ��ܰ�����ImwWindowManager��ȥ
using namespace std;
namespace ImWindow {
	typedef struct{
		int ID = 0;
		string mUrl="";
		string mDomain = "";
		string mTitle="";
		//string mIco;//������ʵ��Ico��ַ,������
		string mHexstr="";
		void* mTexture=NULL;//���������������ܶ�����
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
		void DownloadIco(const char* url,bool downloadDomain=false);//���Ȱ���������û�ɹ��ص����ٰ�������
		void DownIcoResultCallback(void *curl, bool success, const std::string& data, void* userdata=NULL);
		std::vector<FavoriteItem*>* ItemList();

		static void DownLoadFavoriteIco(const char* icourl, const char* host, string domain="");
		static void* GetIcoTexture(const char* url, string domain);//���Զ�����������浽RootWindow����������ָ�����αָʲ�ˣ��ĳɱ���domainֱ�Ӵ�����ȡ
		void ChangeToCxImage(const void* buf,int sz);
		static void ParserToImg(void* buf,int size, const char* url,void* curl=NULL);//���մ�WKE�õ���ͼƬ����CxImage�վ��У�Ӧ�þ����ļ���
	private:
		//std::vector<FavoriteItem*> favoriteList;
		string fileName;
		//std::map<int, ID3D11ShaderResourceView*> HostTexture;
		void* mEmptyTexture = NULL;//û��ͼ������
		void* mSettingTexture = NULL;
		char split ='|';//�ָ�������Ԫ�ָ����ֶηָ��������ļ�ʱ�ã� (char)0����\0��,'\v'==(char)11��ֱ�Ʊ�
		bool isFirst = false;
	};
}
#endif
