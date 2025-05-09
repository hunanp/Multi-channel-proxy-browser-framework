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
	bool open = true;//关闭标记
	bool drag = false;//从最大恢复到前一个大小是通过拖动完成的
	bool ismerged = false;//合并视图的标记一下，还原时不能用主视图窗口
	int tabcount = 0;
	float tabwidth = 0;
	long idindex = 0;//ID序号，名称不能重复。如果只按数量循环删除tab后可能出现重复名称
	ImVec2 Pos;//用于保存最大、最小化前的状态
	ImVec2 Size;
	ImVec2 MainPos;//用于保存主窗最大、最小化前的状态.子窗时需要另外保存主窗大小，还原时需要
	ImVec2 MainSize;
	ImVec2 MovePos;//拖出去的多屏需要
	ImVec2 DragSize;//拖入前大小,拖出后用来还原
	bool clickitem = false;//设个当前点击标识，放大时需要，免得计算
	int setwinstep = 0;//记录设置窗口步骤
	ImVec2 clickposition;//保存拖动开始的比例，以便光标在同一位置
	int formstatus = SW_NORMAL;
	//int laststatus = SW_NORMAL;//最后一次设置的状态，多屏只设一个即可
	HWND hwnd = NULL;//作多屏时需要一个tab一个窗口句柄，viewport自动合并显示时在主屏范转围内不管多少个子窗都只一个窗口句柄.而拖出后浮动窗可能有独立句柄，但是随时变（如移到某大窗内句柄会被注销）,无法固定。若用ConfigViewportsNoAutoMerge，则会不停建建、释放窗口。单独设个窗不让imgui自动删除
	ImGuiWindow* window = NULL;
	ImRect NewWinSize;//为了减少闪动设个变量保存修改后的位置不在绘制过程中修改大小
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
{//可序列化MAP，保存后直接读入
private:
	int intsize = 4;//保存串长度位固定按四位处理
	char intchar[4];
	size_t offset;
	template<class T>
	void write(std::stringstream &ss, T &t) {
		ss.write((char*)(&t), sizeof(t));
	}

	void write(std::stringstream &ss, std::string &str) {
		size_t size = str.size();
		//char intchar[4];//单个char最大值是127，这里直接把数值转成整数的四位
		intchar[0] = size & 0xff;
		intchar[1] = (size >> 8) & 0xff;
		intchar[2] = (size >> 16) & 0xff;
		intchar[3] = (size >> 24) & 0xff;
		//size = *((int*)intchar);//还原
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
		size_t size = *((int*)intchar);//还原成整数
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
			(*this)[key] = value;//还是按键值对插入
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
	{//只保存时间
		insert(std::unordered_map<string, DWORD>::value_type(key, GetTickCount()));
	}
};
typedef std::unordered_map<std::string, bool> wordslist;
typedef struct
{
	bool m_bExtractEmail = false;
	bool m_bExtractNumber = false;
	bool m_Exten = false;
	int  m_nThreadCount = 100;//线程数
	int  m_nTimeOut = 20;//下载超时
	int  m_nConnectTimeOut = 10;
	int  m_nMaxTotal = 100000;//最多搜取网页数量
	int  m_nTotalPerSite = 100000;//每个网站最大数量
	int  m_nMaxPerPage = 1000;//每页最多提取链接数量,这个对速度影响很大，30线程5分钟，设100可处理完21000，200网址14500，500网址13000，1000网址7000，2000网址6700
	int  m_follow_relative_links = 1000;//
	int  m_nLandscapeDepth = 1; //横向深度
	int  m_nDepth = 10;          //纵向
	int  m_nBak = 1;           //备份时间间隔分钟
	int  m_nPageItem = 1000;       //每页显示行数
	char  m_nBakPath[MAX_PATH] = { 0 };  //备份路径
										 //ImVector<string> m_keywords; //生成搜索网址时自动添加的关键词
	char  m_keywords[2 * MAX_PATH] = { 0 };//附加关键字
	char  m_urlInclude[MAX_PATH] = { 0 };  //网址包括
	char  m_ContentInclude[MAX_PATH] = { 0 };  //内容包括
	char  m_EmailOrNumInclude[MAX_PATH] = { 0 };  //邮箱包括
	char  m_urlExclude[MAX_PATH] = { 0 };  //网址不包括
	char  m_ContentExclude[MAX_PATH] = { 0 };  //内容包括
	char  m_EmailOrNumExclude[MAX_PATH] = { 0 };  //邮箱不包括
	char  m_NumberFormat[10 * MAX_PATH] = { 0 };//号码格式
	char  m_Prefix[MAX_PATH] = { 0 };//前缀

	wordslist tmp_keywords;//减少转换次数
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
	string                  m_temp;//临是变量，传值用
	int                     m_counttmp = 0;//临是变量，传值用,多线程统计记数用
	bool                    m_breginfo = false;//保存注册信息
											   //需要保存的参数：
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
	int                     m_iFontSizePadding = 0;//因字体改变需要调整间隔，默认按8设置
	int                     m_iPopOutSizeX = 0;//弹出窗宽
	int                     m_iPopOutSizeY = 0;
	int                     m_iWidth = 0;//新建平台界面宽
	int                     m_iHeight = 0;
	int                     m_iChangeIPTime = 0;//自动换IP时间间隔，毫秒

	vector<char*>         m_testhttp;
	vector<char*>         m_testhttps;


	int                     m_nSearchIndex = 1;     //搜索引擎顺序或搜索IP范围
	int                     m_nLocalCodeIndex = zh_CN; //用户设置语言序号，由序号取得对应语言字符
	serializable_map<std::string, char*> localstr; //语言版本
	serializable_map<std::string, char> TopDomain;//域名库
	serializable_map<std::string, std::string> nameToIcoString;//名称与16位图片字符串对应关系

	vector<std::pair<unsigned long, string>> m_ipdata;//IP对应国家代码
	char  m_MachineCode[30] = { 0 };//机器码
	char  key[30] = { 0 };//注册码
	char  code[3] = { "" };//搜IP国家代码
	SearchParam            m_sParam;           //搜索参数

	int finishedcount = 0;
	bool AutoUpdateVPN = true;
	bool AutoChangeProxy = false;
	char ProxyTestType = 0;
	bool VPNListMode = 0;//0全部，1可用的，2不可用的
	bool m_fontloaded = true;//中文延后加载，加载成功标记。加载完后才显示文字，不然先看到乱码。调字体也需要上次加完
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
	bool                       IsDragArea(HWND hwnd);//连框拖动区
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
	float                       GetTabWidth(ImGuiWindow* window, int &size);//获得该平台的tab所占宽度
	ImGuiDockNode*              GetNode(ImGuiWindow* window);
	bool                        ViewportIsMinimized(ImGuiViewportP* viewport);
	
private:
	void                                RemoveTab(ImGuiWindow* window);
	void                                AddTab(ImGuiWindow* window);//插入位置
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
	std::unordered_map<ImGuiID, void*> m_texture;//保存按钮图标，网站图标

	ConfigInfo					    m_config;
	ImFontAtlas*                    m_Fonts = NULL;
	float                           m_fBtnHight = 15.f;
	int                             m_rightpadding = 7;
	HWND							m_parentHandle = NULL;
	DWORD                           m_mouseTickCount;//鼠标间隔计时
	DWORD                           m_tiptime;//tip计时器开始
	ImGuiID                         m_tipid;
	ImGuiWindow*                    m_addwindow = NULL;

	bool							m_clicklock;
	ImGuiID                         m_dockspace_id = 0;
	HWND                            m_pHandle = NULL;
	ImVec2                          m_fullsize;
	TabItemInfo*                    m_redrawtab = NULL;//记下当前是哪个窗口需要移动或重画
	TabItemInfo*                    m_testtab = NULL;
	bool							m_isfirstrun=true;//用来判断拖出标签
};
#endif