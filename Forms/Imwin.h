#pragma once
#ifndef __Imwin_H__
#define __Imwin_H__
#include <memory>
#include <string>
#include <stdexcept>
#include <system_error>
#include "config.h"
#include "ImwConfig.h"

#include "imgui.h"
#include <imgui_internal.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include "FavoriteItem.h"
#include "resource.h"
#include <sstream>
#include <Uxtheme.h>
#include <vssym32.h>
#pragma comment(lib, "uxtheme.lib")

#define WM_TO_TRAY (WM_USER + 2133)
using namespace ImWindow;

enum class WinStyle : DWORD {
	windowed = WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
	aero_borderless = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
	basic_borderless = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX
};

typedef struct {
	string title = "";
	string url = "";
	bool open = true;//�رձ��
	bool drag = false;//�����ָ���ǰһ����С��ͨ���϶���ɵ�
	bool ismerged = false;//�ϲ���ͼ�ı��һ�£���ԭʱ����������ͼ����
	int tabcount = 0;
	float tabwidth = 0;
	long idindex = 0;//ID��ţ����Ʋ����ظ������ֻ������ѭ��ɾ��tab����ܳ����ظ�����
	ImVec2 Pos;//���ڱ��������С��ǰ��״̬
	ImVec2 Size;
	ImVec2 MainPos;//���ڱ������������С��ǰ��״̬.�Ӵ�ʱ��Ҫ���Ᵽ��������С����ԭʱ��Ҫ
	ImVec2 MainSize;
	ImVec2 MovePos;//�ϳ�ȥ�Ķ�����Ҫ
	ImVec2 DragSize;//����ǰ��С,�ϳ���������ԭ
	bool clickitem = false;//�����ǰ�����ʶ���Ŵ�ʱ��Ҫ����ü���
	int setwinstep = 0;//��¼���ô��ڲ���
	ImVec2 clickposition;//�����϶���ʼ�ı������Ա�����ͬһλ��
	int formstatus = SW_NORMAL;
	//int laststatus = SW_NORMAL;//���һ�����õ�״̬������ֻ��һ������
	HWND hwnd = NULL;//������ʱ��Ҫһ��tabһ�����ھ����viewport�Զ��ϲ���ʾʱ��������תΧ�ڲ��ܶ��ٸ��Ӵ���ֻһ�����ھ��.���ϳ��󸡶��������ж��������������ʱ�䣨���Ƶ�ĳ���ھ���ᱻע����,�޷��̶�������ConfigViewportsNoAutoMerge����᲻ͣ�������ͷŴ��ڡ��������������imgui�Զ�ɾ��
	ImGuiWindow* window = NULL;
	ImRect NewWinSize;//Ϊ�˼�������������������޸ĺ��λ�ò��ڻ��ƹ������޸Ĵ�С
} TabItemInfo;
enum TabColorMode
{
	TABCOLORMODE_TITLE,
	TABCOLORMODE_BACKGROUND,
	TABCOLORMODE_CUSTOM
};
enum localeinfo
{
	en_US,
	zh_CN,
	zh_TW,
	fr_FR,
	ja_JP,
	de_DE,
	es_ES,
	pt_PT,
	it_IT,
};

template<class Key, class Value>
class serializable_map : public std::unordered_map<Key, Value>
{//�����л�MAP�������ֱ�Ӷ���
private:
	int intsize = 4;//���洮����λ�̶�����λ����
	char intchar[4];
	size_t offset;
	template<class T>
	void write(std::stringstream &ss, T &t) {
		ss.write((char*)(&t), sizeof(t));
	}

	void write(std::stringstream &ss, std::string &str) {
		size_t size = str.size();
		//char intchar[4];//����char���ֵ��127������ֱ�Ӱ���ֵת����������λ
		intchar[0] = size & 0xff;
		intchar[1] = (size >> 8) & 0xff;
		intchar[2] = (size >> 16) & 0xff;
		intchar[3] = (size >> 24) & 0xff;
		//size = *((int*)intchar);//��ԭ
		ss.write(intchar, intsize);
		//ss.write((char*)(&size), sizeof(size));
		ss.write((char*)(str.data()), str.length());
	}

	template<class T>
	void read(std::vector<char> &buffer, T &t) {
		t = (T)(*(buffer.data() + offset));
		offset += sizeof(T);
	}

	void read(std::vector<char> &buffer, std::string &str) {
		char* p = buffer.data() + offset;
		intchar[0] = p[0];
		intchar[1] = p[1];
		intchar[2] = p[2];
		intchar[3] = p[3];
		size_t size = *((int*)intchar);//��ԭ������
		offset += intsize;
		std::string str2(buffer.data() + offset, buffer.data() + offset + size);
		str = str2;
		offset += size;
	}
public:
	std::vector<char> serialize() {
		std::vector<char> buffer;
		std::stringstream ss;
		for (auto &i : (*this)) {
			Key str = i.first;
			Value value = i.second;
			write(ss, str);
			write(ss, value);
		}
		size_t size = ss.str().size();
		buffer.resize(size);
		ss.read(buffer.data(), size);
		return buffer;
	}
	void deserialize(std::vector<char> &buffer) {
		offset = 0;
		while (offset < buffer.size()) {
			Key key;
			Value value;
			read(buffer, key);
			read(buffer, value);
			(*this)[key] = value;//���ǰ���ֵ�Բ���
		}
	}
	void show(void) {
		for (auto &i : (*this)) {
			std::cout << i.first << ":" << i.second << std::endl;
		}
		std::cout << std::endl;
	}
	void GetKeyValue(std::string key, std::string &value)
	{
		//if (Count(key) > 0)
		value = (*this)[key];
	}
	bool Delete(std::string key)
	{
		erase(key);
		return false;
	}
	void Add(std::string key, std::string val)
	{
		insert(std::unordered_map<std::string, std::string>::value_type(key, val));
	}
	void Add(std::string key)
	{//ֻ����ʱ��
		insert(std::unordered_map<string, DWORD>::value_type(key, GetTickCount()));
	}
};
typedef std::unordered_map<std::string, bool> wordslist;
typedef struct
{
	bool m_bExtractEmail = false;
	bool m_bExtractNumber = false;
	bool m_Exten = false;
	int  m_nThreadCount = 100;//�߳���
	int  m_nTimeOut = 20;//���س�ʱ
	int  m_nConnectTimeOut = 10;
	int  m_nMaxTotal = 100000;//�����ȡ��ҳ����
	int  m_nTotalPerSite = 100000;//ÿ����վ�������
	int  m_nMaxPerPage = 1000;//ÿҳ�����ȡ��������,������ٶ�Ӱ��ܴ�30�߳�5���ӣ���100�ɴ�����21000��200��ַ14500��500��ַ13000��1000��ַ7000��2000��ַ6700
	int  m_follow_relative_links = 1000;//
	int  m_nLandscapeDepth = 1; //�������
	int  m_nDepth = 10;          //����
	int  m_nBak = 1;           //����ʱ��������
	int  m_nPageItem = 1000;       //ÿҳ��ʾ����
	char  m_nBakPath[MAX_PATH] = { 0 };  //����·��
										 //ImVector<string> m_keywords; //����������ַʱ�Զ���ӵĹؼ���
	char  m_keywords[2 * MAX_PATH] = { 0 };//���ӹؼ���
	char  m_urlInclude[MAX_PATH] = { 0 };  //��ַ����
	char  m_ContentInclude[MAX_PATH] = { 0 };  //���ݰ���
	char  m_EmailOrNumInclude[MAX_PATH] = { 0 };  //�������
	char  m_urlExclude[MAX_PATH] = { 0 };  //��ַ������
	char  m_ContentExclude[MAX_PATH] = { 0 };  //���ݰ���
	char  m_EmailOrNumExclude[MAX_PATH] = { 0 };  //���䲻����
	char  m_NumberFormat[10 * MAX_PATH] = { 0 };//�����ʽ
	char  m_Prefix[MAX_PATH] = { 0 };//ǰ׺

	wordslist tmp_keywords;//����ת������
	wordslist tmp_urlInclude;
	wordslist tmp_ContentInclude;
	wordslist tmp_EmailOrNumInclude;
	wordslist tmp_urlExclude;
	wordslist tmp_ContentExclude;
	wordslist tmp_EmailOrNumExclude;
	wordslist tmp_NumberFormat;
	serializable_map<std::string, std::string> hd;
}SearchParam;

typedef struct{
	//ConfigInfo();
	float					m_fDragMarginRatio;
	float					m_fDragMarginSizeRatio;
	ImColor					m_oHightlightAreaColor;
	TabColorMode			m_eTabColorMode;
	bool					m_bVisibleDragger;

	ImColor					m_oTabColorNormal;
	ImColor					m_oTabColorActive;
	ImColor					m_oTabColorBorder;
	float					m_fTabOverlap;
	float					m_fTabSlopWidth;
	float					m_fTabSlopP1Ratio;
	float					m_fTabSlopP2Ratio;
	float					m_fTabSlopHRatio;
	float					m_fTabShadowDropSize;
	float					m_fTabShadowSlopRatio;
	float					m_fTabShadowAlpha;
	ImVec2					m_oStatusBarFramePadding;
	bool					m_bShowTabShadows;
	string                  m_DefaultLang = "";
	string                  m_temp;//���Ǳ�������ֵ��
	int                     m_counttmp = 0;//���Ǳ�������ֵ��,���߳�ͳ�Ƽ�����
	bool                    m_breginfo = false;//����ע����Ϣ
											   //��Ҫ����Ĳ�����
	bool					m_bShowTabBorder;
	int                     m_iTabType;
	int                     m_iColorMode;
	int                     m_iRow = 1;
	int                     m_iCol = 1;
	int                     m_iMultiScreenZoom = 100;
	int                     m_iSingleScreenZoom = 100;
	char                    m_cDefaultURL[MAX_PATH] = "https://www.baidu.com";
	char                    m_cProxyTestURL[MAX_PATH] = "";
	int                     m_iFontSize = 8;
	int                     m_iFontSizePadding = 0;//������ı���Ҫ���������Ĭ�ϰ�8����
	int                     m_iPopOutSizeX = 0;//��������
	int                     m_iPopOutSizeY = 0;
	int                     m_iWidth = 0;//�½�ƽ̨�����
	int                     m_iHeight = 0;
	int                     m_iChangeIPTime = 0;//�Զ���IPʱ����������

	vector<char*>         m_testhttp;
	vector<char*>         m_testhttps;


	int                     m_nSearchIndex = 1;     //��������˳�������IP��Χ
	int                     m_nLocalCodeIndex = zh_CN; //�û�����������ţ������ȡ�ö�Ӧ�����ַ�
	serializable_map<std::string, char*> localstr; //���԰汾
	serializable_map<std::string, char> TopDomain;//������
	serializable_map<std::string, std::string> nameToIcoString;//������16λͼƬ�ַ�����Ӧ��ϵ

	vector<std::pair<unsigned long, string>> m_ipdata;//IP��Ӧ���Ҵ���
	char  m_MachineCode[30] = { 0 };//������
	char  key[30] = { 0 };//ע����
	char  code[3] = { "" };//��IP���Ҵ���
	SearchParam            m_sParam;           //��������

	int finishedcount = 0;
	bool AutoUpdateVPN = true;
	bool AutoChangeProxy = false;
	char ProxyTestType = 0;
	bool VPNListMode = 0;//0ȫ����1���õģ�2�����õ�
	bool m_fontloaded = true;//�����Ӻ���أ����سɹ���ǡ�����������ʾ���֣���Ȼ�ȿ������롣������Ҳ��Ҫ�ϴμ���
	int m_localport = 10801;
	string m_localip = "127.0.0.1";
	string exepath = "";
	ImFontAtlas* m_Fonts = NULL;
	}ConfigInfo;
#if 0
extern IMGUI_IMPL_API LRESULT WndProcHandlerfromImGui(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
class MainWindow
{
	static const char* const c_pClassName;

public:
	MainWindow() {};
	MainWindow(const char* pTitle, RECT rect, WinStyle style, HWND parent = nullptr);
	MainWindow(const char* pTitle, RECT rect, DWORD exstyle, DWORD style);
	virtual ~MainWindow();
	HWND                       GetHandle();
	ImGuiWindow*               GetCursorWin(HWND hwnd, ImVec2 cursorPos);
	TabItemInfo*               GetTab(HWND hwnd);
	bool                       IsDragArea(HWND hwnd);//�����϶���
	ImVec2					   GetSize() const;
	void                       SetMaximized(HWND hwnd);
	void                       SetMinimized(HWND hwnd);
	bool                       IsMaximized(HWND hwnd);
	bool                       IsMinimized(HWND hwnd);
	
	void                        CreateThumbnail(HWND renderedWindow, HWND sourceWindow);
	void                        UnregisterThumbnail();

	//bool                      LoadTextureFromFile(const char* filename, std::string hex_str, void** out_srv, int* out_width, int* out_height);
	void*                       GetTexture(std::string name);
	void                        DrawImage(int &id, const ImVec2& pos, char* imageId, const ImVec2& imageSize, void * texture = NULL);
	void                        DrawTittleBar(TabItemInfo* tab, ImRect oRect, float ftitlehight = TITLE_HEIGHT);
	void                        DrawToolBar(TabItemInfo* tab, bool isTab = true);
	void						ShowTooltip(const char* pText, bool needdelay = true);
	char*                       Local(const char* name);
	bool                        DrawCircleButton(int &id, const ImVec2& pos, char* icoId, const ImVec2& imageSize,bool abled = true, const char* label = NULL, void * texture = NULL);
	bool                        AddButton(HWND hwnd, ImGuiID id, const ImVec2& pos, ButtonType type, bool abled = true);
	ImFontAtlas*                SetFontSize(int sz);
	void                        DrawPage(HWND hwnd, RECT oRect, string title, float alpha = 1.00);

	void                        ClickPosition(ImGuiWindow* win, ImVec2 cursorPos);
	//void                      WinMaximized(TabItemInfo* tab);
	void                        WinMaximized(ImGuiWindow* win);
	TabItemInfo*                SaveWinSize(ImGuiWindow* win);
	bool                        IsMerged(ImGuiWindow* win);
	bool                        IsDockIn(ImGuiWindow* win);
	bool                        IsHostWin(ImGuiWindow* win);
	int                         WinStatus(HWND hwnd);
	ImVec2                      GetFullscreen();
	ImVec2                      CursorPos();
	bool                        IsFullscreen(HWND hwnd);
	TabItemInfo *               GetTab(ImGuiWindow* window);
	void                        Cleanup();
	ConfigInfo*                 Config();

	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();
	ID3D11Device* GetDXDevice();
	// Data
	ID3D11Device*            g_pd3dDevice = NULL;
	ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
	IDXGISwapChain*          g_pSwapChain = NULL;
	ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;
protected:
	int                         WinCount(ImGuiDockNode* node);
	bool                        IsSubwin(ImGuiDockNode* node);
	float                       CaluTabWidth(ImGuiDockNode* node, int size);
	float                       GetTabWidth(ImGuiWindow* window, int &size);//��ø�ƽ̨��tab��ռ���
	ImGuiDockNode*              GetNode(ImGuiWindow* window);
	bool                        ViewportIsMinimized(ImGuiViewportP* viewport);
	
private:
	void                                RemoveTab(ImGuiWindow* window);
	void                                AddTab(ImGuiWindow* window);//����λ��
	void                                SetStatus(ImGuiWindow* window, int statu,bool islast=true);
	void                                Adjustwindow(TabItemInfo* tab, ImVec2 viewsize, ImVec2 &newpos);
	void                                SetViewportSize(TabItemInfo* tab, ImVec2 pos, ImVec2 size, bool setviewport = true);
protected:
	bool							m_bManualSizing;
	bool							m_bSizing;
	LONG_PTR						m_iSizingMode;
	bool							m_bCatchAlt;
	bool							m_bCaptureMouseOnClick;
	static bool						s_bClassInitialized;

private:
	int                             m_tabindex = 1;
	int                             m_contrlId=0;
	FavoriteList                    m_favorite;
	std::unordered_map<ImGuiID, void*> m_texture;//���水ťͼ�꣬��վͼ��

	ConfigInfo					    m_config;
	ImFontAtlas*                    m_Fonts = NULL;
	float                           m_fBtnHight = 15.f;
	int                             m_rightpadding = 7;
	HWND							m_parentHandle = NULL;
	DWORD                           m_mouseTickCount;//�������ʱ
	DWORD                           m_tiptime;//tip��ʱ����ʼ
	ImGuiID                         m_tipid;
	ImGuiWindow*                    m_addwindow = NULL;

	bool							m_clicklock;
	ImGuiID                         m_dockspace_id = 0;
	HWND                            m_pHandle = NULL;
	ImVec2                          m_fullsize;
	TabItemInfo*                    m_redrawtab = NULL;//���µ�ǰ���ĸ�������Ҫ�ƶ����ػ�
	TabItemInfo*                    m_testtab = NULL;
	bool							m_isfirstrun=true;//�����ж��ϳ���ǩ
};
#endif