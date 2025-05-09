#include "Imwin.h"
#include "Borderless.h"
#include "Control.h"
#include "strCoding.h"
#include <Shlwapi.h>

ConfigInfo m_Config;
static LRESULT WINAPI ImGuiWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
bool MainWindow::CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void MainWindow::CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void MainWindow::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void MainWindow::CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}
ID3D11Device* MainWindow::GetDXDevice() { return g_pd3dDevice; }

MainWindow::MainWindow(const char* pTitle, RECT rect, DWORD exstyle, DWORD style)
	: m_bSizing(false)
{//用imgui设的样式解决主窗浮不上来的问题
	m_pHandle = CreateWindowEx(
		0,
		window_class(ImGuiWndProc),
		pTitle,
		static_cast<DWORD>(style),
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top,
		nullptr, nullptr, ::GetModuleHandle(nullptr), NULL
	);
	if (!m_pHandle) {
		throw last_error("failed to create window");
	}

	SetWindowLongPtr(m_pHandle, GWLP_USERDATA, (LONG_PTR)this);
	set_borderless(m_pHandle, borderless);
	set_borderless_shadow(m_pHandle, borderless_shadow);
	::ShowWindow(m_pHandle, SW_SHOW);
	m_fullsize= GetFullscreen();
}
MainWindow::MainWindow(const char* pTitle, RECT rect, WinStyle style, HWND parent)
	: m_bSizing(false)
	//, m_eCursor(EasyWindow::E_CURSOR_ARROW)
{
	m_pHandle = CreateWindowEx(
		0,
		window_class(ImGuiWndProc),
		pTitle,
		static_cast<DWORD>(style),
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top,
		parent, nullptr, ::GetModuleHandle(nullptr), NULL
	);
	if (!m_pHandle) {
		throw last_error("failed to create window");
	}

	m_parentHandle = parent;//用来确定是不是主页
	SetWindowLongPtr(m_pHandle, GWLP_USERDATA, (LONG_PTR)this);
	set_borderless(m_pHandle, borderless);
	set_borderless_shadow(m_pHandle, borderless_shadow);
	::ShowWindow(m_pHandle, SW_SHOW);
	m_fullsize = GetFullscreen();

	if (parent != nullptr)//拖出的窗口，独立句柄
		return;

	// Initialize Direct3D
	if (!CreateDeviceD3D(m_pHandle))
	{
		CleanupDeviceD3D();
		return;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(m_pHandle);//绑定句柄
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
}
// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static LRESULT WINAPI ImGuiWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{//拖出来的窗口消息在imgui_impl_win32.cpp中处理
	switch (msg)
	{
	case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
	{
		if (WM_LBUTTONDBLCLK == msg || WM_LBUTTONDOWN == msg)//只处理合并显示的子窗和用程序最大化的标题栏的点击，被包含或程序最大化时已经把标题栏设成客户区了，故不是传WM_NCLBUTTONDBLCLK
			if (auto window_ptr = reinterpret_cast<MainWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
				auto& window = *window_ptr;
				ImVec2 cursorPos = window.CursorPos();
				ImGuiWindow* win = window.GetCursorWin(hwnd, cursorPos);//找出光标所在窗口
				if (win != NULL)
				{
					ImRect rect = ImRect(win->Pos, win->Pos + win->Size);//有分区时得到的是容器窗口的大小
					rect.Max.y = rect.Min.y + TITLE_HEIGHT;
					if (rect.Contains(cursorPos))
					{//只处理标题栏设成客户区后的事件
						if (WM_LBUTTONDOWN == msg)
							window.SaveWinSize(win);
						//window.ClickPosition(win, cursorPos);//点击标题栏时要保存点击位置，用来计算拖动时光标在标题栏的位置
						if (WM_LBUTTONDBLCLK == msg)
						{//两种情况，1 子窗被包 2 程序最大化的，会有自己句柄了，还windows内部还是SW_NORMAL.这些需要自己用程序处理最大化及还原。
							ImVec2 fullsize = window.GetFullscreen();
							if ((window.IsMerged(win) || win->Viewport->Size.x == fullsize.x&&win->Viewport->Size.y == fullsize.y))
								window.WinMaximized(win);//用API拖在WM_NCLBUTTONDOWN中调尺寸后可以拖，但是拖的过程imgui是绘不出来的，也就是看不到表单上的元素
						}
					}
				}
			}
	}
	break;
	case WM_NCLBUTTONUP://取不到
		break;
	case WM_NCLBUTTONDBLCLK://标题栏双击事件，WM_NC标题栏事件. 拖动在常规窗响应，最大化已经不把它当标题栏
	{//标题栏消息循环似乎不允许执行其它的函数，如同样的MergedTitleDoubleClick在WM_NCLBUTTONDBLCLK下不行，而WM_LBUTTONDBLCLK下则可以。改成这里设个标记，然后在画表单时最大化。主要是基于同样的原因用API放大的，拖小还原时画布还是最大化，界面有瑕疵。
		if (auto window_ptr = reinterpret_cast<MainWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
			auto& window = *window_ptr;
			ImVec2 cursorPos = window.CursorPos();
			ImGuiWindow* win = window.GetCursorWin(hwnd, cursorPos);
			window.WinMaximized(win);//标题栏的消息在imgui中是检测不到,设个标记，绘imgui窗口时再最大化或还原
		}
		return 0;//注销这里则会执行标题栏内部程序来最大化，拖动时imgui主窗大小无法设置大小
	}
	break;
	}

	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))//鼠标等消息在这拦掉了
		return true;

	if (auto window_ptr = reinterpret_cast<MainWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
		auto& window = *window_ptr;
		switch (msg)
		{
		case WM_NCHITTEST:
		{//拖动表单和调大小。When we have no border or title bar, we need to perform our own hit testing to allow resizing and moving.
			if (window.IsFullscreen(hwnd))
				return HTCLIENT;//全屏后不再把它当标题栏，这样拖小时可以即时画（快拖imgui表单有滞后感），不是全屏则可以快速拖
								//return HTTOPLEFT;
								//return HTTRANSPARENT;
			if (borderless&&window.IsDragArea(hwnd))
			{//没标题框时根据位置决定哪些区域可以拖动或调大小。保留标题栏，常规窗口它比拖imgui表单效果好	
			 //OutputDebugStringA((to_string(GET_X_LPARAM(lparam)) + "--拖动区--" + to_string(GET_Y_LPARAM(lparam)) + "\n").c_str());
				LRESULT result = hit_test(hwnd, POINT{ GET_X_LPARAM(lparam),GET_Y_LPARAM(lparam) });
				return result;
			}
		}
		break;
		case WM_NCCALCSIZE: {//去掉标题栏
			if (wparam == TRUE && borderless) {
				auto& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);
				adjust_maximized_client_rect(hwnd, params.rgrc[0]);
				return 0;
			}
			break;
		}
		case WM_NCCREATE:
		{
			auto userdata = reinterpret_cast<CREATESTRUCTW*>(lparam)->lpCreateParams;
			// store window instance pointer in window user data
			::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(userdata));
		}
		break;
		case WM_NCACTIVATE: {
			if (!composition_enabled()) {
				// Prevents window frame reappearing on window activation
				// in "basic" theme, where no aero shadow is present.
				return 1;
			}
		}
							break;
		case WM_CLOSE: {
			::DestroyWindow(hwnd);
			for (int k = 0; k < m_tabs.size(); k++)
			{//从任务栏点关闭时,在这修改状态关闭窗口下的所有子窗
				if (m_tabs[k].hwnd == hwnd)
					m_tabs[k].open = false;
			}
			return 0;
		}
		case WM_IME_CHAR:
		{//处理中文输入
			auto& io = ImGui::GetIO();
			DWORD wChar = wparam;
			if (wChar <= 127)
			{
				io.AddInputCharacter(wChar);
			}
			else
			{
				// swap lower and upper part.
				BYTE low = (BYTE)(wChar & 0x00FF);
				BYTE high = (BYTE)((wChar & 0xFF00) >> 8);
				wChar = MAKEWORD(high, low);
				wchar_t ch[6];
				MultiByteToWideChar(CP_OEMCP, 0, (LPCSTR)&wChar, 4, ch, 3);
				io.AddInputCharacter(ch[0]);
			}
			return 0;
		}
		case WM_MOUSEMOVE:
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {
			switch (wparam) {
			case VK_F8: { borderless_drag = !borderless_drag;        return 0; }
			case VK_F9: { borderless_resize = !borderless_resize;    return 0; }
			case VK_F10: { set_borderless(hwnd, !borderless);               return 0; }
			case VK_F11: { set_borderless_shadow(hwnd, !borderless_shadow); return 0; }
			}
			break;
		}
		case WM_TO_TRAY:
			if (lparam == WM_LBUTTONDBLCLK || lparam == WM_LBUTTONDOWN)
			{//单击或双击显示主窗
				SetForegroundWindow(hwnd);
				ShowWindow(hwnd, SW_SHOWNORMAL);
				DeleteTray(hwnd);
			}
			break;
		case WM_SIZE:
		{
			bool isminized = window.IsMinimized(hwnd);
			//if (window.g_pd3dDevice != NULL && wparam != SIZE_MINIMIZED)//wparam有时有误
			if (window.g_pd3dDevice != NULL && !isminized)
			{
				window.CleanupRenderTarget();
				window.g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lparam), (UINT)HIWORD(lparam), DXGI_FORMAT_UNKNOWN, 0);
				window.CreateRenderTarget();
			}
			if (isminized)
			{//托盘
				ToTray(hwnd);
				ShowWindow(hwnd, SW_HIDE);
				break;
			}
		}
		return 0;
		case WM_SYSCOMMAND:
			if ((wparam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return 0;
			break;
		case WM_DESTROY:
			//::PostQuitMessage(0);
			return 0;
		case WM_DPICHANGED:
			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
			{
				//const int dpi = HIWORD(wParam);
				//printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
				const RECT* suggested_rect = (RECT*)lparam;
				::SetWindowPos(hwnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			break;
		}
	}
	return ::DefWindowProc(hwnd, msg, wparam, lparam);
}

MainWindow::~MainWindow()
{
	if (m_pHandle != NULL)
	{
		DestroyWindow(m_pHandle);
		m_pHandle = NULL;
	}
}
void MainWindow::Cleanup()
{
	UnregisterThumbnail();
	CleanupDeviceD3D();
}
ConfigInfo* MainWindow::Config()
{
	return &m_Config;
}
HWND MainWindow::GetHandle()
{
	return m_pHandle;
}
void MainWindow::SetMaximized(HWND hwnd)
{
	if (IsZoomed(hwnd) != TRUE)
		ShowWindow(hwnd, SW_MAXIMIZE);
	else
		ShowWindow(hwnd, SW_RESTORE);
}
void MainWindow::SetMinimized(HWND hwnd)
{
	if (!IsIconic(hwnd))
		ShowWindow(hwnd, SW_MINIMIZE);
	else
		ShowWindow(hwnd, SW_RESTORE);
}
bool MainWindow::IsMaximized(HWND hwnd)
{
	return IsZoomed(hwnd) == TRUE;
}
bool MainWindow::IsMinimized(HWND hwnd)
{
	return IsIconic(hwnd) == TRUE;
}
bool MainWindow::AddButton(HWND hwnd, ImGuiID id, const ImVec2& pos, ButtonType type, bool abled)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	const ImRect btn(pos, ImVec2(pos.x + 16 + 10, pos.y + 16 + 10));//FramePadding.x=0
	ImU32 colText = ImGui::GetColorU32(ImGuiCol_Text);

	if (!abled)
	{
		colText = ImGui::GetColorU32(ImGuiCol_TextDisabled);
	}
	ImGui::KeepAliveID(id);
	bool hovered, held, pressed = false;
	if (ImGui::ButtonBehavior(btn, id, &hovered, &held, abled ? ImGuiButtonFlags_PressedOnClick : ImGuiItemFlags_Disabled))
		pressed = true;

	ImU32 col = ImGui::GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered);
	ImVec2 center = btn.GetCenter();
	if (hovered && (held || GetAsyncKeyState(VK_LBUTTON) & 0x8000))//点击后保持按下
	{
		center.x += 1;
		center.y += 1;
	}

	int num_segments = 10 + btn.GetWidth() / 2;
	if (ImGui::IsMouseDragging(0))
		pressed = false;
	else
		if (hovered || ImGui::IsMouseHoveringRect(btn.Min, btn.Max))
		{
			if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
			{
				pressed = true;
				hovered = true;
				center.x += 1;
				center.y += 1;
			}

			ImVec2 cursor = CursorPos();
			if (btn.Contains(cursor) && ImGui::IsMouseHoveringRect(btn.Min, btn.Max))//解决光标出平台后按钮还hovered
				window->DrawList->AddCircleFilled(center, ImMax(2.0f, btn.GetWidth() / 2 - 1), col, num_segments);
		}

	if (pressed)
	{//防误计数产生连续点击
		if (GetTickCount() - m_mouseTickCount < 100)
		{
			pressed = false;
			//g.IO.MouseDown[0] = false;
		}
		else
		{
			m_mouseTickCount = GetTickCount();
			//m_clicklock = true;//直到弹起只一次
		}
	}
	//float cross_extent = g.FontSize * 0.5f * 0.7071f - 1.0f;//自动调大小
	float cross_extent = 16 * 0.5f * 0.7071f - 1.0f;
 center = ImVec2(center.x - 0.5f, center.y - 0.5f);
	float size = 6.0f;//+字大小

	switch (type)
	{//画两次可让颜色一致
	case ImGuiAddBtn:
		window->DrawList->AddLine(ImVec2(center.x - size, center.y - 1), ImVec2(center.x + size, center.y - 1), colText, 1.0f);
		window->DrawList->AddLine(ImVec2(center.x, center.y - size), ImVec2(center.x, center.y + size), colText, 1.0f);
		break;
	case ImGuiMinBtn:
		window->DrawList->AddLine(ImVec2(center.x - size + 1, center.y), ImVec2(center.x + size - 1, center.y), colText, 1.0f);
		window->DrawList->AddLine(ImVec2(center.x - size + 1, center.y), ImVec2(center.x + size - 1, center.y), colText, 1.0f);
		break;
	case ImGuiMaxBtn:
		window->DrawList->AddRect(ImVec2(center.x - size + 2, center.y - size + 2), ImVec2(center.x + size - 1, center.y + size - 1), colText, 0.0f, colText, 1.0f);
		window->DrawList->AddRect(ImVec2(center.x - size + 2, center.y - size + 2), ImVec2(center.x + size - 1, center.y + size - 1), colText, 0.0f, colText, 1.0f);
		break;
	case ImGuiIsMaxBtn:
		window->DrawList->AddRect(ImVec2(center.x - size + 3, center.y - size + 1), ImVec2(center.x + size, center.y + size - 3), colText, 0.0f, colText, 1.0f);
		window->DrawList->AddRect(ImVec2(center.x - size, center.y - size + 4), ImVec2(center.x + size - 3, center.y + size), colText, 0.0f, colText, 1.0f);
		window->DrawList->AddRect(ImVec2(center.x - size + 3, center.y - size + 1), ImVec2(center.x + size, center.y + size - 3), colText, 0.0f, colText, 1.0f);
		window->DrawList->AddRect(ImVec2(center.x - size, center.y - size + 4), ImVec2(center.x + size - 3, center.y + size), colText, 0.0f, colText, 1.0f);
		break;
	case ImGuiCloseBtn:
		window->DrawList->AddLine(ImVec2(center.x + cross_extent, center.y + cross_extent), ImVec2(center.x - cross_extent, center.y - cross_extent), colText, 1.0f);
		window->DrawList->AddLine(ImVec2(center.x + cross_extent, center.y - cross_extent), ImVec2(center.x - cross_extent, center.y + cross_extent), colText, 1.0f);
		window->DrawList->AddLine(ImVec2(center.x + cross_extent, center.y + cross_extent), ImVec2(center.x - cross_extent, center.y - cross_extent), colText, 1.0f);
		window->DrawList->AddLine(ImVec2(center.x + cross_extent, center.y - cross_extent), ImVec2(center.x - cross_extent, center.y + cross_extent), colText, 1.0f);
		break;
	case ImGuiTabCloseBtn:
		window->DrawList->AddLine(ImVec2(center.x + cross_extent - 1, center.y + cross_extent - 1), ImVec2(center.x - cross_extent + 1, center.y - cross_extent + 1), colText, 1.0f);
		window->DrawList->AddLine(ImVec2(center.x + cross_extent - 1, center.y - cross_extent + 1), ImVec2(center.x - cross_extent + 1, center.y + cross_extent - 1), colText, 1.0f);
		break;
	case ImGuiGoBackBtn:
		window->DrawList->AddLine(ImVec2(center.x - size, center.y), ImVec2(center.x, center.y - 6), colText, 2.0f);
		window->DrawList->AddLine(ImVec2(center.x - size - 1, center.y), ImVec2(center.x + size, center.y), colText, 2.0f);
		window->DrawList->AddLine(ImVec2(center.x - size, center.y), ImVec2(center.x, center.y + 6), colText, 2.0f);
		break;
	case ImGuiForwardBtn:
		window->DrawList->AddLine(ImVec2(center.x, center.y - 6), ImVec2(center.x + size, center.y), colText, 2.0f);
		window->DrawList->AddLine(ImVec2(center.x - size, center.y), ImVec2(center.x + size + 1, center.y), colText, 2.0f);
		window->DrawList->AddLine(ImVec2(center.x, center.y + 6), ImVec2(center.x + size, center.y), colText, 2.0f);
		break;
	case ImGuiRefreshBtn:
		//window->DrawList->AddCircle(ImVec2(center.x, center.y), size, colText, 20, 2.0f);//画圆
		// window->DrawList->AddLine(ImVec2(center.x, center.y - size+4), ImVec2(center.x, center.y - size + 4), col, 8.0f);//擦掉部分
		window->DrawList->AddTriangleFilled(ImVec2(center.x, center.y - size - 2), ImVec2(center.x, center.y - size + 5), ImVec2(center.x + 6, center.y - size + 2), colText);
		//window->DrawList->AddTriangle(ImVec2(center.x, center.y - size - 1), ImVec2(center.x, center.y - size + 4), ImVec2(center.x + 5, center.y - size + 2), colText, 2.0f);//箭头
		window->DrawList->AddBezierCurve(ImVec2(center.x, center.y - size + 1), ImVec2(center.x - size - 5, center.y), ImVec2(center.x, center.y + size + 7), ImVec2(center.x + size + 1, center.y + 1), colText, 2.0f);
		break;
	case ImGuiStopLoadingBtn:
		window->DrawList->AddLine(ImVec2(center.x + cross_extent + 1, center.y + cross_extent + 1), ImVec2(center.x - cross_extent - 1, center.y - cross_extent - 1), colText, 1.0f);
		window->DrawList->AddLine(ImVec2(center.x + cross_extent + 1, center.y - cross_extent - 1), ImVec2(center.x - cross_extent - 1, center.y + cross_extent + 1), colText, 1.0f);
		window->DrawList->AddLine(ImVec2(center.x + cross_extent + 1, center.y + cross_extent + 1), ImVec2(center.x - cross_extent - 1, center.y - cross_extent - 1), colText, 1.0f);
		window->DrawList->AddLine(ImVec2(center.x + cross_extent + 1, center.y - cross_extent - 1), ImVec2(center.x - cross_extent - 1, center.y + cross_extent + 1), colText, 1.0f);
		break;
	case ImGuiMultiScreen:
	{
		int sz = 5;//小正方形大小
		center.x -= 4;
		center.y -= 5;
		window->DrawList->AddRectFilled(ImVec2(center.x, center.y), ImVec2(center.x + sz, center.y + sz), colText);
		center.x += 6;
		window->DrawList->AddRectFilled(ImVec2(center.x, center.y), ImVec2(center.x + sz, center.y + sz), colText);
		center.y += 6;
		window->DrawList->AddRectFilled(ImVec2(center.x, center.y), ImVec2(center.x + sz, center.y + sz), colText);
		center.x -= 6;
		window->DrawList->AddRectFilled(ImVec2(center.x, center.y), ImVec2(center.x + sz, center.y + sz), colText);
	}
	break;
	case ImGuiOther:
		window->DrawList->AddLine(ImVec2(center.x - 5, center.y), ImVec2(center.x - 3, center.y), colText, 0.5f);
		window->DrawList->AddLine(ImVec2(center.x - 1, center.y), ImVec2(center.x + 1, center.y), colText, 0.5f);
		window->DrawList->AddLine(ImVec2(center.x + 3, center.y), ImVec2(center.x + 5, center.y), colText, 0.5f);
		//不太清晰window->DrawList->AddText(g.IO.FontDefault, 20.0f, ImVec2(center.x - 4, center.y - 15), ~0U, "...");
		break;
	case ImGuiSetting:
	{
	}
	break;
	case ImGuiProxy:
		window->DrawList->AddText(g.IO.FontDefault, 16.0f, ImVec2(center.x - 2, center.y - 8), ImGui::GetColorU32(ImGuiCol_Text), "P");
		//window->DrawList->AddText(g.IO.FontDefault, 16.0f, ImVec2(center.x - 2, center.y - 8), ~0U, "P");//写两次加深颜色
		break;
	case ImGuiFavoritesBtn:
		break;
	case ImGuiPlay:
	{
		int sz = 6;
		window->DrawList->AddTriangleFilled(ImVec2(center.x + sz / 2 + 4, center.y), ImVec2(center.x - sz / 2 + 1, center.y + sz), ImVec2(center.x - sz / 2 + 1, center.y - sz), colText);
		window->DrawList->AddTriangleFilled(ImVec2(center.x + sz / 2 + 4, center.y), ImVec2(center.x - sz / 2 + 1, center.y + sz), ImVec2(center.x - sz / 2 + 1, center.y - sz), colText);
	}
	break;
	case ImGuiPause:
	{
		int sz = 8;
		window->DrawList->AddRectFilled(ImVec2(center.x - 3, center.y - 5), ImVec2(center.x, center.y + 5), colText);
		window->DrawList->AddRectFilled(ImVec2(center.x + 2, center.y - 5), ImVec2(center.x + 5, center.y + 5), colText);
	}
	break;
	case ImGuiStop:
	{
		int sz = 5;//小正方形大小
		window->DrawList->AddRectFilled(ImVec2(center.x - sz + 1, center.y - sz + 1), ImVec2(center.x + sz + 1, center.y + sz), colText);
	}
	break;
	case ImGuiFirst:
	{
		int sz = 6;
		window->DrawList->AddRectFilled(ImVec2(center.x - 4, center.y - 7), ImVec2(center.x - 1, center.y + 8), colText);
		window->DrawList->AddTriangleFilled(ImVec2(center.x - sz + 4, center.y), ImVec2(center.x + sz / 2 + 2, center.y + sz + 1), ImVec2(center.x + sz / 2 + 2, center.y - sz - 1), colText);
	}
	break;
	case ImGuiPrePage:
	{//反向没有自动润色，正向有，不能完全按直线走就会有齿状
		int sz = 6;
		window->DrawList->AddTriangleFilled(ImVec2(center.x - sz - 1, center.y), ImVec2(center.x + sz / 2 - 3, center.y + sz + 1), ImVec2(center.x + sz / 2 - 3, center.y - sz - 1), colText);
		window->DrawList->AddTriangleFilled(ImVec2(center.x - sz + 4, center.y), ImVec2(center.x + sz / 2 + 2, center.y + sz + 1), ImVec2(center.x + sz / 2 + 2, center.y - sz - 1), colText);
	}
	break;
	case ImGuiNextPage:
	{
		int sz = 6;
		window->DrawList->AddTriangleFilled(ImVec2(center.x + sz / 2 + 6, center.y), ImVec2(center.x - sz / 2 + 4, center.y + sz + 1), ImVec2(center.x - sz / 2 + 4, center.y - sz - 1), colText);
		window->DrawList->AddTriangleFilled(ImVec2(center.x + sz / 2 + 1, center.y), ImVec2(center.x - sz / 2 - 1, center.y + sz + 1), ImVec2(center.x - sz / 2 - 1, center.y - sz - 1), colText);
		window->DrawList->AddTriangleFilled(ImVec2(center.x + sz / 2 + 6, center.y), ImVec2(center.x - sz / 2 + 4, center.y + sz + 1), ImVec2(center.x - sz / 2 + 4, center.y - sz - 1), colText);
		window->DrawList->AddTriangleFilled(ImVec2(center.x + sz / 2 + 1, center.y), ImVec2(center.x - sz / 2 - 1, center.y + sz + 1), ImVec2(center.x - sz / 2 - 1, center.y - sz - 1), colText);
	}
	break;

	case ImGuiLast:
	{
		int sz = 6;
		window->DrawList->AddRectFilled(ImVec2(center.x + 6, center.y - 7), ImVec2(center.x + 3, center.y + 8), colText);
		window->DrawList->AddTriangleFilled(ImVec2(center.x + sz / 2 + 1, center.y), ImVec2(center.x - sz / 2 - 1, center.y + sz + 1), ImVec2(center.x - sz / 2 - 1, center.y - sz - 1), colText);
		window->DrawList->AddTriangleFilled(ImVec2(center.x + sz / 2 + 1, center.y), ImVec2(center.x - sz / 2 - 1, center.y + sz + 1), ImVec2(center.x - sz / 2 - 1, center.y - sz - 1), colText);
	}
	break;
	case ImGuiExport:
		break;
	}

	return pressed;
}
void MainWindow::ClickPosition(ImGuiWindow* win, ImVec2 cursorPos)
{//保存点击位置，计算光标位置，不执行最大化.不与SaveWinSize合并
	for (int k = 0; k < m_tabs.size(); k++)
	{
		if (m_tabs[k].window != NULL&&m_tabs[k].window == win)
		{//记下点击位置与左顶点距离
			cursorPos.x = m_tabs[k].MainSize.x*(cursorPos.x*1.00 / m_fullsize.x);//要用主窗的比例
			m_tabs[k].clickposition = cursorPos;
			for (int i = 0; i < m_tabs.size(); i++)
			{
				m_tabs[i].clickposition = cursorPos;
			}
			break;
		}
	}
}
TabItemInfo* MainWindow::SaveWinSize(ImGuiWindow* win)
{//保存所有窗口状态，还原用
	TabItemInfo* tab = NULL;
	ImVec2 cursorPos = CursorPos();
	//bool issinglewin = IsMerged(win);//是不是单屏，可能包含
	for (int k = 0; k < m_tabs.size(); k++)
	{
		//if (m_tabs[k].laststatus != SW_MAXIMIZE && !(m_tabs[k].window->Size.x == m_fullsize.x&&m_tabs[k].window->Size.y == m_fullsize.y))
		if (!(m_tabs[k].window->Viewport->Size.x == m_fullsize.x&&m_tabs[k].window->Viewport->Size.y == m_fullsize.y))
		{//全部保存，因为一个窗口最大化其它窗口可能被合并视图
			m_tabs[k].Pos = m_tabs[k].window->Pos;
			m_tabs[k].Size = m_tabs[k].window->Size;
			m_tabs[k].MainPos = m_tabs[k].window->Viewport->Pos;
			m_tabs[k].ismerged = false;
			if (IsMerged(m_tabs[k].window))//合并视图则是与主窗共用一个Viewport,要能还原到自己的大小，而不是主窗大小
			{
				m_tabs[k].ismerged = true;//还原时要用
				m_tabs[k].MainSize = m_tabs[k].window->Size;
			}
			else
				m_tabs[k].MainSize = m_tabs[k].window->Viewport->Size;
		}

		m_tabs[k].clickitem = false;
		if (m_tabs[k].window->ViewportId == win->ViewportId)
		{//同一个平台必有同一根节点，返回ImGuiWindow所在tab
			ImRect rect = ImRect(m_tabs[k].window->Pos, m_tabs[k].window->Pos + m_tabs[k].window->Size);
			rect.Max.y = rect.Min.y + TITLE_HEIGHT;
			if (rect.Contains(cursorPos))//用光标判断点击了哪一个
			{
				m_tabs[k].clickitem = true;
				tab = &m_tabs[k];

				//记下点击位置与左顶点距离
				ImVec2 tmp = cursorPos;
				tmp.x = m_tabs[k].Size.x*(cursorPos.x*1.00 / m_fullsize.x) + (m_tabs[k].Pos.x - m_tabs[k].MainPos.x);//包含的窗可以单独放大、拖小。本区位置加上左边偏移即总偏差（可能左边还有分区）
				m_tabs[k].clickposition = tmp;//点击位置
			}
		}
	}
	return tab;
}
void MainWindow::WinMaximized(ImGuiWindow* win)
{
	TabItemInfo* tab = SaveWinSize(win);
	if (tab != NULL)
	{
		if (!(tab->window->Viewport->Size.x == m_fullsize.x&&tab->window->Viewport->Size.y == m_fullsize.y))
			tab->formstatus = SW_MAXIMIZE;
		else//恢复
			tab->formstatus = SW_RESTORE;
		SetStatus(tab->window, tab->formstatus, false);//保存当前状态
	}
}

TabItemInfo* MainWindow::GetTab(HWND hwnd)
{
	for (int k = 0; k < m_tabs.size(); k++)
	{
		if (m_tabs[k].hwnd == hwnd)
			return &m_tabs[k];
	}
	return NULL;
}
ImGuiWindow* MainWindow::GetCursorWin(HWND hwnd, ImVec2 cursorPos)
{
	ImGuiContext& g = *GImGui;
	if (GImGui == NULL)
		return NULL;
	ImGuiWindow* window = NULL;
	for (int i = g.Windows.Size - 1; i >= 0; i--)//找到此窗口
	{//所有窗口都在里面，它记录了窗口的显示顺序，0在最底层，Size-1显示在最前面.从前往后找
		ImGuiWindow* win = g.Windows[i];
		if (win->Viewport != NULL&&win->Active&&hwnd == win->Viewport->PlatformHandle&&strcmp("Debug##Default", win->Name) != 0)
		{//一个窗口下有多屏，句柄是一样的，用光标位置来确定是哪个窗口
			ImRect rect = ImRect(win->Pos, win->Pos + win->Size);
			if (rect.Contains(cursorPos))
			{
				window = win;
				break;
			}
		}
	}
	return window;
}

bool MainWindow::IsDragArea(HWND hwnd)
{
	ImVec2 cursorPos = CursorPos();
	ImGuiWindow* window = GetCursorWin(hwnd, cursorPos);
	if (window == NULL)
		return false;

	ImRect rect = ImRect(window->Pos, window->Pos + window->Size);//有分区时得到的是容器窗口的大小

																  //构造标题栏区域,让按钮以外区域可响应拖动事件
	ImRect bar = rect;
	bar.Max.y = rect.Min.y + TITLE_HEIGHT;

	//放过边界区域，使它响应调窗口大小事件
	ImRect leftborder = rect;
	leftborder.Max.x = rect.Min.x + 2;
	ImRect rightborder = rect;
	rightborder.Min.x = rect.Max.x - 2;
	ImRect bottomborder = rect;
	bottomborder.Min.y = rect.Max.y - 2;
	if (leftborder.Contains(cursorPos) || rightborder.Contains(cursorPos) || bottomborder.Contains(cursorPos))
		return true;//光标在左、右、下边框

	float icowidth = 0;//左图标宽
	bool issubwin = (window->DockNode != NULL&&window->DockNode->ParentNode != nullptr);//是否是子窗，子窗不画图标、最大最小按钮
	ImRect ico = rect;
	ico.Max.x = rect.Min.x + (issubwin ? 0 : TITLE_HEIGHT);
	ico.Max.y = rect.Min.y + TITLE_HEIGHT;
	if (ico.Contains(cursorPos))
		return true;//光标在标题图标区

	ImGuiContext* ctx = GImGui;
	ImGuiDockContext* dc = &ctx->DockContext;
	for (int n = 0; n < dc->Nodes.Data.Size; n++)
	{
		if (ImGuiDockNode* node = (ImGuiDockNode*)dc->Nodes.Data[n].val_p)
		{
			if (node->Windows.Size > 0)
			{//容器CountNodeWithWindows>0,但Windows为空
				int iSize = node->Windows.Size;
				if (node->Size.x < 180)//太小不设拖动区间，只供拖tab
					return false;

				ImRect subbar = ImRect(node->Pos, node->Pos + node->Size);//标题栏框
				int btnwidth = 90;
				bool issubwin = IsSubwin(node);
				if (issubwin)
				{
					if ((node->Pos.y - 10 < node->Windows[0]->Viewport->Pos.y&&node->Pos.x + node->Size.x + 10 > node->Windows[0]->Viewport->Pos.x + node->Windows[0]->Viewport->Size.x))//右上角的要放开
						issubwin = false;//要新增按钮
					else
						btnwidth = 38;//子窗不需要画图标和最大化、最小化按钮
				}

				for (int i = 0; i < node->Windows.Size; i++)
				{
					float tabwidth = node->TabBar->Tabs[i].Width*iSize;//用实际占宽，分屏后不固定宽度了
					subbar.Max.y = subbar.Min.y + TITLE_HEIGHT;
					subbar.Max.x = subbar.Min.x + node->Size.x - btnwidth - 3;//拖动区结束点,减右边三按钮和间隔
					subbar.Min.x = subbar.Min.x + (issubwin ? 0 : TITLE_HEIGHT) + tabwidth + ((node->Size.x - tabwidth > 5 * m_fBtnHight) ? 31 : 0);//拖动区起点

					if (subbar.Contains(cursorPos))
					{//光标在标题栏时，1 tab区不响应主窗拖动。2上面有控件的不响应
						if (!IsMerged(node->Windows[i]))//已经合并的让它响应鼠标以方便拖出,当大小相等时认为是在主窗口
							return true;
					}
				}
			}
		}
	}
	return false;
}
int MainWindow::WinStatus(HWND hwnd)
{//获得窗口状态
	for (vector<TabItemInfo>::iterator it = m_tabs.begin(); it != m_tabs.end(); it++)
	{
		if (it->window != NULL&&hwnd == it->hwnd)
		{
			if (!(it->window->Viewport->Size.x == m_fullsize.x&&it->window->Viewport->Size.y == m_fullsize.y))
			return SW_MAXIMIZE;
			break;
		}
	}
	return SW_NORMAL;
}
bool MainWindow::IsSubwin(ImGuiDockNode* node)
{
	return (node != NULL&&node->ParentNode != nullptr && (node->ParentNode->CountNodeWithWindows > 1 || node->ParentNode->ChildNodes[0] != NULL&& node->ParentNode->ChildNodes[1] != NULL));//当只一个时也当主屏
}
bool MainWindow::IsHostWin(ImGuiWindow* win)
{
	if (win == NULL || win->Viewport == NULL)
		return false;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (win->Viewport->PlatformHandle == viewport->PlatformHandle/*&&viewport->Size.x== win->Size.x&&viewport->Size.y == win->Size.y多屏时不对*/)
		return true;
	return false;
}
bool MainWindow::IsDockIn(ImGuiWindow* win)
{//窗口是否停靠在主窗。停靠窗口的node是二叉树，其必在左右两分支，而没有分屏的，不管多少个窗口都在node->Windows数组里
	if (win->DockNode != NULL &&win->DockNode->ParentNode != NULL)
	{
		ImGuiDockNode* node = win->DockNode->ParentNode;
		if (node->ChildNodes[0] != NULL || node->ChildNodes[1] != NULL)
		{
			if (node->ChildNodes[0] != NULL)
			{//查左树
				for (int i = 0; i < node->ChildNodes[0]->Windows.Size; i++)
				{
					if (node->ChildNodes[0]->Windows[i] == win)
						return true;
				}
			}
			if (node->ChildNodes[1] != NULL)
			{//查右树
				for (int i = 0; i < node->ChildNodes[1]->Windows.Size; i++)
				{
					if (node->ChildNodes[1]->Windows[i] == win)
						return true;
				}
			}
		}
	}
	return false;
}
bool MainWindow::IsMerged(ImGuiWindow* win)
{//判断窗口是否合并显示到别的视图中.当句柄与A窗相同且尺寸小于A窗时就是合并的。但是不包括分屏的，分屏的要能拖动全屏,分屏window也是共用句柄
	ImGuiViewportP* main_viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
	ImRect mainrect=main_viewport->GetMainRect();
	if (win->Viewport == main_viewport&&mainrect.Contains(win->Rect()))//注意,只会并到host窗，拖出去的相互包含也不会合并,所以可以不用管
	{//分两种情况 1 没有分屏时 2 分屏的
		bool isdock = IsDockIn(win);
		if (isdock)//已停靠不算，win可能就是自己
		{
			if (win->ParentWindow != NULL && !(main_viewport->Pos.x == win->ParentWindow->Pos.x&&main_viewport->Pos.y == win->ParentWindow->Pos.y&&main_viewport->Size.x == win->ParentWindow->Size.x&&main_viewport->Size.y == win->ParentWindow->Size.y))//排除自已
				if (mainrect.Contains(win->ParentWindow->Rect()))
					return true;//拖出去的也可能是多屏
			return false;
		}
		else
			if (!(main_viewport->Pos.x == win->Pos.x&&main_viewport->Pos.y == win->Pos.y&&main_viewport->Size.x == win->Size.x&&main_viewport->Size.y == win->Size.y))//排除自已
				return true;
	}
	return false;
}

int MainWindow::WinCount(ImGuiDockNode* node)
{//统计节点下的窗口，二叉树，ChildNodes[1]是叶子
	ImGuiDockNode*p = node;
	//先到根节点
	while (p != NULL &&p->ParentNode != NULL)
		p = p->ParentNode;
	//return p->CountNodeWithWindows;//这个只是分区数,tab数通过Windows.Size取得
	int count = 0;
	while (p != NULL && (p->ChildNodes[0] || p->ChildNodes[1]))
	{
		if (p->ChildNodes[1] != NULL)
			count += p->ChildNodes[1]->Windows.Size;//是叶子
		if (p->ChildNodes[0] != NULL)
		{
			if (p->ChildNodes[0]->IsLeafNode())
				count += p->ChildNodes[0]->Windows.Size;//是叶子
		}
		p = p->ChildNodes[0];
	}
	return count;
}
void MainWindow::CreateThumbnail(HWND hWndDst, HWND sourceWindow)
{//生成窗口截图或略缩图，hWndDst显示图片的窗口句柄
	HRESULT hr = S_OK;
	hr = DwmRegisterThumbnail(hWndDst, sourceWindow, &m_Imguithumbnail);//截桌面图：DwmRegisterThumbnail(hwnd, FindWindow(_T("Progman"), NULL), &thumbnail);
	RECT dest;
	::GetClientRect(hWndDst, &dest);//取得窗口大小

	DWM_THUMBNAIL_PROPERTIES dskThumbProps;
	dskThumbProps.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_VISIBLE | DWM_TNP_SOURCECLIENTAREAONLY;
	dskThumbProps.fSourceClientAreaOnly = FALSE;
	dskThumbProps.fVisible = TRUE;
	dskThumbProps.opacity = (255 * 70) / 100;//不透明性，255为不透明
	dskThumbProps.rcDestination = dest;
	// Display the thumbnail
	hr = DwmUpdateThumbnailProperties(m_Imguithumbnail, &dskThumbProps);
	if (SUCCEEDED(hr))
	{
	}
}

void MainWindow::UnregisterThumbnail()
{
	if (m_Imguithumbnail != NULL)
		DwmUnregisterThumbnail(m_Imguithumbnail);
	m_Imguithumbnail = NULL;
}
ImVec2 MainWindow::GetSize() const
{
	return ImGui::GetIO().DisplaySize;
}

ImFontAtlas* MainWindow::SetFontSize(int sz)
{
	m_Fonts = new ImFontAtlas();

	char filename[MAX_PATH] = "c:/Windows/Fonts/msyh.ttc";
	float size_pixels = sz;
	size_t data_size = 0;
	ImFontConfig font_cfg = ImFontConfig();
	static ImVector<ImWchar> gr;
	gr.clear();
	static ImFontGlyphRangesBuilder range;
	range.Clear();
	range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
	range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
	range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesKorean());
	range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesDefault());
	range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesThai());
	range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesVietnamese());
	range.BuildRanges(&gr);
	//先按指定加
	if (strlen(filename) > 0 && PathFileExists(filename))
	{
		void* data = ImFileLoadToMemory(filename, "rb", &data_size, 0);
		if (!data)
		{
			IM_ASSERT_USER_ERROR(0, "Could not load font file!");
			return m_Fonts;
		}

		if (font_cfg.Name[0] == '\0')
		{
			// Store a short copy of filename into into the font name for convenience
			const char* p;
			for (p = filename + strlen(filename); p > filename && p[-1] != '/' && p[-1] != '\\'; p--) {}
			ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "%s, %.0fpx", p, size_pixels);
		}
		font_cfg.FontData = data;
		font_cfg.FontDataSize = data_size;
		font_cfg.SizePixels = 2 * size_pixels;
		//font_cfg.MergeMode = true;
		font_cfg.PixelSnapH = true;
		font_cfg.GlyphRanges = gr.Data;
		m_Fonts->ClearFonts();
		m_Fonts->AddFont(&font_cfg);
	}
	else
	{
		font_cfg.SizePixels = 2 * size_pixels;
		m_Fonts->AddFontDefault(&font_cfg);//默认的白一点，有色差
	}
	//补中文
	if (strlen(filename) > 0 && strstr(filename, "msyh.ttc") == NULL)
	{
		const char* defaultfont = "c:\\windows\\Fonts\\msyh.ttc";
		if (PathFileExists(defaultfont))
		{
			font_cfg.MergeMode = true;
			m_Fonts->AddFontFromFileTTF(defaultfont, size_pixels*2.00, &font_cfg, gr.Data);
		}
	}
	m_Fonts->Build();
	return m_Fonts;
}
void* MainWindow::GetTexture(std::string name)
{//按钮所需图片统一在这取，没有的自动加载
	ImGuiWindow* window = GImGui->CurrentWindow;
	ImGuiID id=window->GetID(name.c_str());//图标要传到imgui,统一用ImGuiID
	void* ptexture = m_texture[id];
	if (ptexture == NULL)
	{
		int image_width = 0;
		int image_height = 0;
		if (m_Config.nameToIcoString.count(name) > 0)
			m_favorite.LoadTextureFromFile("", m_Config.nameToIcoString[name], &ptexture, &image_width, &image_height, g_pd3dDevice);
		else//传文件时直接加载
			//if (name.length() != 6)//直接排除CN.png之类的
			m_favorite.LoadTextureFromFile(name.c_str(), "", &ptexture, &image_width, &image_height, g_pd3dDevice);
		m_texture[id] = ptexture;
	}
	return ptexture;
}
void MainWindow::DrawImage(int &id, const ImVec2& pos, char* imageId, const ImVec2& imageSize, void * texture)
{//传图标名或文件名，会长留HASH表中，大图不要用它或用过后需删除
	ImTextureID texId = texture;
	if (NULL == texture&&NULL != imageId)
		texId = GetTexture(imageId);//可以直接传文件名
	ImGuiContext& g = *GImGui;
	ImVec2 btnsz = pos;
	btnsz.x += imageSize.x;
	btnsz.y += imageSize.y + 4;
	const ImRect btn(pos, btnsz);
	ImVec2 center = btn.GetCenter();
	const ImRect image_bb(ImVec2(center.x - imageSize.x / 2, center.y - imageSize.y / 2), ImVec2(center.x + imageSize.x / 2, center.y + imageSize.y / 2));//图片位置大小,对角坐标
	if (texId != NULL)
		g.CurrentWindow->DrawList->AddImage(texId, image_bb.Min, image_bb.Max, ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.00f)));
}


void MainWindow::AddTab(ImGuiWindow* window)
{
	TabItemInfo tab;
	TabItemInfo * ptab = GetTab(window);//所在组
										//tab.title = u8"New Tab##robot" + to_string(m_tabindex++);
	tab.title = "A" + to_string(m_tabindex) + string(emptytabname) + to_string(m_tabindex++);
	if (ptab)
		tab.formstatus = ptab->formstatus;
	else
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		if (m_fullsize.x == viewport->Size.x)
			tab.formstatus = SW_MAXIMIZE;
	}
	m_tabs.push_back(tab);
	//m_tabs.insert(m_tabs.begin() + index, tab);//在第i+1个元素前面插入tab
}
TabItemInfo * MainWindow::GetTab(ImGuiWindow* window)
{
	for (int i = 0; i<m_tabs.size(); i++)
	{
		if (window == m_tabs[i].window)
		{
			return &m_tabs[i];
		}
	}
	return nullptr;
}
void MainWindow::RemoveTab(ImGuiWindow* window)
{
	for (vector<TabItemInfo>::iterator it = m_tabs.begin(); it != m_tabs.end();)
	{
		if (window == it->window)
		{
			it = m_tabs.erase(it);
			break;
		}
		else
			it++;
	}
}
void MainWindow::SetStatus(ImGuiWindow* window, int statu, bool islast)
{//同一个组一个表单修改后其它的也要改
	bool issinglewin = IsMerged(window);//是不是单屏，可能包含
	for (vector<TabItemInfo>::iterator it = m_tabs.begin(); it != m_tabs.end(); it++)
	{
		if (it->window != NULL&&window->ViewportId == it->window->ViewportId)
		{//首先用视图相同过滤
			if (issinglewin&&window == it->window || !issinglewin && (window == it->window || IsDockIn(it->window)))
			{//没有停靠，可能有几个tab,有停靠则所有同窗的子窗也要设
				if (islast)
				{//包含时之前是同一个句柄，子窗放大时母窗会显示已最大化，实际没有
				}
				else
					it->formstatus = statu;
			}
		}
	}
}
ImGuiDockNode* MainWindow::GetNode(ImGuiWindow* window)
{
	ImGuiContext* ctx = GImGui;
	ImGuiDockContext* dc = &ctx->DockContext;
	for (int k = 0; k < dc->Nodes.Data.Size; k++)
	{
		if (ImGuiDockNode* node = (ImGuiDockNode*)dc->Nodes.Data[k].val_p)//一个node可以看作一个平台,里面包括多个Window,一个Window就是一个tab
		{
			if (node->Windows.Size > 0)
			{
				for (int w = 0; w < node->Windows.Size; w++)
				{
					if (node->Windows[w] == window)
						return node;
				}
			}
		}
	}
	return nullptr;
}
ImVec2 MainWindow::CursorPos()
{
	POINT oCursorPos = { 0,0 };
	::GetCursorPos(&oCursorPos);//有时ImGui::GetIO().MousePos并没有即时更新
								//::ScreenToClient(hWnd, &oCursorPos);
	return ImVec2(oCursorPos.x, oCursorPos.y);
}
ImVec2 MainWindow::GetFullscreen()
{
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);//除去任务栏有区域大小
	return ImVec2(rect.right - rect.left, rect.bottom - rect.top);
}
bool MainWindow::IsFullscreen(HWND hwnd)
{
	if (IsZoomed(hwnd))
		return true;
	RECT rect;
	::GetWindowRect(hwnd, &rect);
	return m_fullsize.x == (rect.right - rect.left) && m_fullsize.y == (rect.bottom - rect.top);
}
float MainWindow::CaluTabWidth(ImGuiDockNode* node, int size)
{
	if (size == 0)
		return 0.0;
	bool issubwin = IsSubwin(node);
	int btnwidth = 90;
	int empty = 50;//保留50空白区用来拖
	if (issubwin)
	{//子窗只一个关闭按钮不需要画图标和最大化、最小化按钮,减少空间
		if ((node->Pos.y - 10 < node->Windows[0]->Viewport->Pos.y&&node->Pos.x + node->Size.x + 10 > node->Windows[0]->Viewport->Pos.x + node->Windows[0]->Viewport->Size.x))//右上角的要放开
		{//右上角最大化按钮
			if (node->Size.x<300)
				empty = 20;
			issubwin = false;//要新增按钮
		}
		else
		{
			empty = 10;
			btnwidth = 30;
		}
	}
	float fMaxSize = (node->Size.x - (issubwin ? 0 : TITLE_HEIGHT) - 10 - 30 - empty - btnwidth) / size;//计算标签平均宽度左边图标TITLE_HEIGHT，新增按钮30，右边三个90
	if (fMaxSize > 180 || fMaxSize <= 1)
		fMaxSize = 180;
	if (issubwin&&node->Size.x / 3<fMaxSize)
		fMaxSize = node->Size.x / 3;
	return fMaxSize;//TAB实际占用区载
}
float MainWindow::GetTabWidth(ImGuiWindow* window, int &size)
{//计算tab所占宽度,有的节点是框架.找出窗口所在的节点，这个节点就是一个窗口
	if (window == NULL || window->Viewport == NULL)
		return 180;
	ImGuiDockNode* node = GetNode(window);
	if (node != nullptr)
	{
		size = node->Windows.Size + (m_addwindow == window ? 1 : 0);
		float twidth = CaluTabWidth(node, size);//Windows.Size可能还没加
		return twidth;
	}
	return 0;
}
void MainWindow::SetViewportSize(TabItemInfo* tab, ImVec2 pos, ImVec2 size, bool setviewport)
{
	if (setviewport)
	{
		if (tab->window->Viewport->Window)
		{
			tab->window->Pos = pos;
			tab->window->Viewport->Pos = pos;
			tab->window->Viewport->Size = size;
			tab->window->Viewport->WorkPos = pos;
			tab->window->Viewport->WorkSize = size;
			tab->window->Viewport->Window->Pos = pos;
			tab->window->Viewport->Window->Size = size;
			tab->window->Viewport->Window->SizeFull = size;
		}
	}
	else
	{
		tab->window->Pos = pos;
		tab->window->Size = size;
		tab->window->SizeFull = size;
		tab->window->DockNode->HostWindow->Pos = pos;
		ImGui::DockBuilderSetNodePos(tab->window->DockNode->ID, pos);
		ImGui::DockBuilderSetNodeSize(tab->window->DockNode->ID, size);
	}
}
void MainWindow::Adjustwindow(TabItemInfo* tab, ImVec2 viewsize, ImVec2 &newpos)
{//处理最大化还原。当一个窗口最大化时另一个常态化的窗口可能被包含进来，这时拖小就会影响到常态化窗口，按钮不见了。
	if (!tab->drag&&ImGui::IsMouseDragging(0))
	{//排除非标题栏的拖动
		ImVec2 cursorPos = CursorPos();
		ImGuiWindow* window = GetCursorWin(tab->hwnd, cursorPos);
		if (window != NULL)
		{
			ImRect rect = ImRect(window->Pos, window->Pos + window->Size);
			rect.Max.y = rect.Min.y + TITLE_HEIGHT;
			if (!rect.Contains(cursorPos) || !IsDragArea(tab->hwnd))
				return;
		}
	}
	tab->MovePos.x = -98595;
	ImVec2 fullsize = m_fullsize;
	bool issinglewin = IsMerged(tab->window);//是不是单屏，可能是被包含浮窗
	if (tab->formstatus == SW_MAXIMIZE || tab->drag || !issinglewin&&IsFullscreen(tab->hwnd) || issinglewin&&fullsize.x == tab->window->Size.x&&fullsize.y == tab->window->Size.y)
	{//最大化或正在拖动
		ImVec2 pos = ImVec2(0, 0);
		bool isdockwin = IsSubwin(tab->window->DockNode);
		if (ImGui::IsMouseDragging(0))
		{//拖动还原并继续拖动
			if (tab->Size.x > 0 && tab->Size.y > 0)
			{//最大化后拖动时要先恢复windows窗口尺寸，否则画布还是最大化时的大小，拖动时就看不到标题栏的标签和按钮。标题栏的消息在imgui中是检测不到的。直接用api拖看不到标签和按钮
				if (!tab->drag)
				{//修改此平台下的窗口大小，只设一次
					for (int k = 0; k < m_tabs.size(); k++)
					{
						if (m_tabs[k].hwnd == tab->hwnd&&m_tabs[k].Size.x > 0)
						{
							ImGui::DockBuilderSetNodeSize(m_tabs[k].window->DockNode->ID, m_tabs[k].Size);
							ImGui::DockBuilderSetNodePos(m_tabs[k].window->DockNode->ID, m_tabs[k].Pos);//反复设会抖
						}
					}
				}
				//修改左顶位置
				ImVec2 mousepos = CursorPos();
				pos = { mousepos.x - tab->clickposition.x ,mousepos.y - tab->clickposition.y };
				tab->drag = true;//拖小的放开左建要能定下来

				if (isdockwin && !IsHostWin(tab->window))
				{//拖出窗口多屏状态下拖小时需要改画屏大小
					::SetWindowPos(tab->hwnd, NULL, pos.x, pos.y, tab->MainSize.x, tab->MainSize.y, SWP_NOZORDER | SWP_NOREDRAW);//拖小一步到位
					if (tab->window->Viewport->Size.x != tab->MainSize.x)
						PostMessage(tab->hwnd, WM_SIZE, 0, (LPARAM)MAKELONG(tab->MainSize.x, tab->MainSize.y));//拖出的主窗需要修改画布大小
						//SetViewportSize(tab, pos, tab->MainSize, true);
					if (tab->window->Viewport->Window)
					{//要设一下，不然成死表单
						ImVec2 size = tab->MainSize;
						tab->window->Viewport->Window->Pos = pos;
						tab->window->Viewport->Window->Size = size;
					}
				}
				else
				{
					//if (isdockwin)
					::SetWindowPos(tab->hwnd, NULL, pos.x, pos.y, tab->MainSize.x, tab->MainSize.y, SWP_NOZORDER | SWP_NOREDRAW);//拖小一步到位
					//else
					//  ImGui::DockBuilderSetNodePos(tab->window->DockNode->ID, pos);//停止拖动后必须要设一次，不然反复几次表单就死在那了（Pos不对，光标检测不到）,在这里设，拖动时按钮会有点抖。
				}
				tab->Pos = pos;
				tab->MovePos = pos;
			}
		}
		else
		{//拖动放开左键,用按钮放大、还原或双击最大化
			if (tab->drag || !issinglewin&&IsFullscreen(tab->hwnd) || issinglewin&&fullsize.x == tab->window->Size.x&&fullsize.y == tab->window->Size.y)
			{//拖小后放开左键时设成SW_NORMAL并还原drag标记
				if (tab->drag)
				{
					if (!isdockwin)
					{
						if (tab->Size.x > 0)
						{
							ImGui::DockBuilderSetNodePos(tab->window->DockNode->ID, tab->Pos);//停止拖动后必须要设一次，不然反复几次表单就死在那了（Pos不对，光标检测不到）
							ImGui::DockBuilderSetNodeSize(tab->window->DockNode->ID, tab->Size);
						}
					}
				}
				else
				{
					for (int k = 0; k < m_tabs.size(); k++)
					{//修改同一窗口下的所有表单
						if (m_tabs[k].hwnd == tab->hwnd&&m_tabs[k].Size.x > 0)
						{
							ImGui::DockBuilderSetNodePos(m_tabs[k].window->DockNode->ID, m_tabs[k].Pos);
							ImGui::DockBuilderSetNodeSize(m_tabs[k].window->DockNode->ID, m_tabs[k].Size);
						}
					}
					::SetWindowPos(tab->hwnd, NULL, tab->MainPos.x, tab->MainPos.y, tab->MainSize.x, tab->MainSize.y, SWP_NOZORDER | SWP_NOACTIVATE);//调小一步到位，不需要分步
					ImGui::DockBuilderSetNodePos(tab->window->DockNode->ID, tab->Pos);
					ImGui::DockBuilderSetNodeSize(tab->window->DockNode->ID, tab->Size);
					if (isdockwin && !IsHostWin(tab->window))
						SetViewportSize(tab, tab->MainPos, tab->MainSize,true);//拖出去的多屏缩小需要
					Sleep(150);
				}
			}
			else
			{//放大
				bool ishostwin = IsHostWin(tab->window);
				if (isdockwin || ishostwin)
				{//当一个小窗已经合并到其它视图,最大化子窗时句柄还只有一个，子窗以内部窗存在，不能用Viewport，子窗放大后才会生成自己的句柄，这时只修改node即可。host窗口则必须用SetWindowPos,内部窗和拖出去的窗口都可以不用.
					if (fullsize.x > tab->window->Size.x || fullsize.y > tab->window->Size.y)
					{//尽量减少SetWindowPos，免得看见闪窗,内部窗和拖出去的窗口都可以不用SetWindowPos.
					//::SetWindowPos(tab->hwnd, NULL, 0, 0, fullsize.x, fullsize.y, SWP_NOZORDER | SWP_NOACTIVATE| SWP_FRAMECHANGED| SWP_NOREDRAW);//会先闪到左上角
						tab->NewWinSize = ImRect(ImVec2(0, 0), ImVec2(fullsize.x, fullsize.y));
					}

					if (isdockwin)
					{
						float otherpos_x = tab->Pos.x - tab->MainPos.x;//偏移量
						float otherpos_y = tab->Pos.y - tab->MainPos.y;

						//多屏只调放大点击的那个区，其它的按原值。计算最大化后的相对位置
						float othersize_x = tab->MainSize.x - tab->Size.x;//都是最大化前的大小
						float othersize_y = tab->MainSize.y - tab->Size.y;//全窗高减自己的高即其它分区占高
						pos = ImVec2(otherpos_x, otherpos_y);
						fullsize.x = fullsize.x - othersize_x;
						fullsize.y = fullsize.y - othersize_y;

						for (int k = 0; k < m_tabs.size(); k++)
						{//修改同一窗口下的所有表单
							if (m_tabs[k].hwnd == tab->hwnd&&m_tabs[k].Size.x > 0)
							{
								m_tabs[k].setwinstep = -1;
								ImGui::DockBuilderSetNodePos(m_tabs[k].window->DockNode->ID, m_tabs[k].Pos);
								ImGui::DockBuilderSetNodeSize(m_tabs[k].window->DockNode->ID, m_tabs[k].Size);
							}
						}
						if (IsHostWin(tab->window))
							::SetWindowPos(tab->hwnd, NULL, 0, 0, fullsize.x, fullsize.y, SWP_NOACTIVATE | SWP_FRAMECHANGED);
						else
							PostMessage(tab->hwnd, WM_SIZE, 0, (LPARAM)MAKELONG(fullsize.x, fullsize.y));//拖出的主窗需要修改画布大小
					}
					ImGui::DockBuilderSetNodePos(tab->window->DockNode->ID, pos);
					ImGui::DockBuilderSetNodeSize(tab->window->DockNode->ID, fullsize);

				}
				else
				{
					ImGui::DockBuilderSetNodePos(tab->window->DockNode->ID, pos);
					ImGui::DockBuilderSetNodeSize(tab->window->DockNode->ID, fullsize);
					Sleep(150);//放大子窗或拖出窗不停一下单窗有时到不了（0，0）。
				}
			}
			tab->drag = false;
		}
	}
	else
	{//SW_RESTORE，恢复位置
		if (tab->Size.x > 0 && tab->window->DockNode&&tab->formstatus != SW_NORMAL)
		{
			ImGui::DockBuilderSetNodePos(tab->window->DockNode->ID, tab->Pos);
			ImGui::DockBuilderSetNodeSize(tab->window->DockNode->ID, tab->Size);
			if (ImGui::IsMouseDragging(0))
			{//计算位置
				ImVec2 mousepos = ImGui::GetIO().MousePos;
				//修改左顶位置
				tab->Pos = { mousepos.x - tab->clickposition.x ,mousepos.y - tab->clickposition.y };
				ImGui::SetNextWindowPos(tab->Pos);
			}
			else
			{
				if (tab->formstatus == SW_RESTORE)
				{
					tab->clickposition = ImVec2(0, 0);
					if (tab->hwnd == ImGui::GetMainViewport()->PlatformHandle)//主窗时要改主窗大小，否则还原不了，拖出去的子窗不需要
						::SetWindowPos(tab->hwnd, NULL, tab->MainPos.x, tab->MainPos.y, tab->MainSize.x, tab->MainSize.y, SWP_NOZORDER | SWP_NOACTIVATE);
					else
						Sleep(150);
				}
			}
		}
	}
}
void MainWindow::DrawPage(HWND hwnd, RECT oRect, string title, float alpha)
{
	m_contrlId = 0;//id重复会闪
	static ImGuiViewport itemdata;//借用它用来保存从ImGui传出的数据
	ImGuiContext& g = *GImGui;
	itemdata.RendererUserData=&m_texture;//传入图标,传出光标所在标签省得计算
	g.IO.UserData = &itemdata;
	ImGuiStyle& style = ImGui::GetStyle();
	//创建一个底部平台，让它不能拖，其它的tab窗口则拖到它下面，其它tab拖到这个平台上。这样就不会出现脱节了
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();//取其窗口(ImGuiWindow*)GImGui->WindowsById.GetVoidPtr(viewport->id);
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));//不留空隙
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	ImGui::Begin(("DockSpaceForm" + to_string(0)).c_str(), nullptr, window_flags);
	ImGui::PopStyleVar(3);
	ImGuiID dockspace_id = ImGui::GetID("DockSpace-main");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	ImGui::End();

	ImGuiWindowClass window_class;//可以用ClassId规定可拖入的框，不用ClassId不能拖
	window_class.DockingAllowUnclassed = true;
	window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton;
	ImGui::SetNextWindowClass(&window_class);//不能少，不然拖出去后会出现关闭按钮

	if (ImGui::IsMouseDragging(0))
	{//拖动时标签时检查主页是否空了，标题栏拖动区不经过这里，只有标签才经过
		ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspace_id);
		if (node->HostWindow != NULL&&node->Windows.Size == 0 && node->CountNodeWithWindows == 0)//当节点分区后该节点就没有window了
			AddTab(NULL);//补一个空白tab
		//TAB停靠位置上色

		if (g.MovingWindow&&g.MovingWindow->DockNode)
		{//拖出标签还原成拖入前的大小
			TabItemInfo * dragtab = GetTab(g.MovingWindow);//找出对应tab
			if (dragtab != nullptr)
			{
				if (g.MovingWindow->DockNode->Windows.Size > 1)
				{//还没完成拖出，表单则还停在原窗口，拖出后就变成单窗口了.
					if (dragtab->DragSize.x > 0)
						ImGui::DockBuilderSetNodeSize(dragtab->window->DockNode->ID, dragtab->DragSize);//还原成拖入前大小
				}
				else
				{//拖单屏则保存大小，这样拖出就有可用的大小了。拖出是会重复保存，不影响什么
					dragtab->DragSize = dragtab->window->Size;
				}
			}
		}
	}

	ImVec2  oCursorPos=CursorPos();
	ImGuiDockNode* node = NULL;
	float addtabwidth = 0;
	int addtabcount = 0;
	if (m_addwindow != NULL || m_tabs.size() == 0)
	{//放在前面，因为执行点击是在Begin生成过程中，易出乱子
		AddTab(m_addwindow);//加在最后,这里加进去后Window还没有生成，算窗口数要加一
		m_tabs[m_tabs.size() - 1].window = m_addwindow;
		addtabwidth = GetTabWidth(m_addwindow, addtabcount);//有新增TAB的宽度要先算出来，因为Window还没有生成，m_addwindow也不能放后面
		if (m_addwindow != NULL)
			node = m_addwindow->DockNode;
		m_addwindow = NULL;//只能放在这，不能放后面，m_addwindow赋值实际是在执行Begin的过程中
	}

	//绘完后再SetWindowPos免得闪
	if (m_redrawtab != NULL&&m_redrawtab->setwinstep>0)//需要等到窗口已经按调整尺寸画后才移
	{//第二步，画好好才移到正确位置。第一步是viewport调大，绘制一次以后window才会变大
		m_redrawtab->setwinstep++;
		if (m_redrawtab->setwinstep == 3)
		{//先是放大窗口（在绘制过程中，绘图窗口并没有放大），第二轮绘图窗口放大并绘制，第三轮已绘好就可以移动了
			::SetWindowPos(m_redrawtab->hwnd, NULL, m_redrawtab->NewWinSize.Min.x, m_redrawtab->NewWinSize.Min.y, m_redrawtab->NewWinSize.Max.x - m_redrawtab->NewWinSize.Min.x, m_redrawtab->NewWinSize.Max.y - m_redrawtab->NewWinSize.Min.y, SWP_NOZORDER | SWP_NOACTIVATE);
			//::MoveWindow(m_redrawtab->hwnd, m_redrawtab->NewWinSize.Min.x, m_redrawtab->NewWinSize.Min.y, m_redrawtab->window->Viewport->Size.x, m_redrawtab->window->Viewport->Size.y, TRUE);
			if (IsSubwin(m_redrawtab->window->DockNode) && !IsHostWin(m_redrawtab->window))
			{//只拖出去的多屏需要修改画板大小，否则控件只在原来位置才感应得到
				SetViewportSize(m_redrawtab, m_redrawtab->NewWinSize.Min, m_redrawtab->NewWinSize.Max,true);
			}
			m_redrawtab->setwinstep = 0;
			m_redrawtab->NewWinSize.Max.x = m_redrawtab->NewWinSize.Min.x;//不让它再画窗口
			m_redrawtab = NULL;
		}
	}

	int tabcount = 0;//取出TAB数量
	float tabwidth = 0;//宽度
	TabItemInfo* closedtab = nullptr;//关闭的tab先记下,在循环外删除
	ImGuiDockNode* calutab = nullptr;//记下当前是哪个窗口宽度
	for (int k = 0; k < m_tabs.size(); k++)//当子窗完全在主窗内时会自动合并，多个窗口都共用一个窗口句柄
	{//不用管它拖到哪个窗口，完全让Imgui管理，不然很容易乱套.要找tab所在平台则通过dc->Nodes.Data.Size查找,这里只管按tab数理画表单.改Tab样式修改TabItemEx，BeginTabBarEx，在imgui_widgets中8098行，修改tab_bar->BarRect
		if (m_tabs[k].window != NULL)
		{//表单已经建好的设尺寸和位置，画一次才有
			if (m_tabs[k].window->DockNode != calutab)
			{//计算当前平台平的tab宽度，换平台重新计算
				if (addtabwidth > 0 && node == m_tabs[k].window->DockNode)
				{//有添加时Window还没有生成要先算出
					tabcount = addtabcount;
					tabwidth = addtabwidth;
				}
				else
					tabwidth = GetTabWidth(m_tabs[k].window, tabcount);

				calutab = m_tabs[k].window->DockNode;

				if (m_tabs[k].window->DockNode&&m_tabs[k].window->DockNode->TabBar)
				{//设定tab宽度，一个平台一次设完。SetNextItemWidth只能保存一个值，有多个平台就乱了。
					m_tabs[k].window->DockNode->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton;//拖出后也要设
					for (int t = 0; t < m_tabs[k].window->DockNode->TabBar->Tabs.size(); t++)
					{
						m_tabs[k].window->DockNode->TabBar->Tabs[t].Width = tabwidth;
						m_tabs[k].window->DockNode->TabBar->Tabs[t].ContentWidth = tabwidth;
						m_tabs[k].window->DockNode->TabBar->Tabs[t].RequestedWidth = tabwidth;
					}
				}
			}

			if (m_tabs[k].formstatus == SW_MAXIMIZE || m_tabs[k].drag ||m_fullsize.x == m_tabs[k].window->Viewport->Size.x&&m_fullsize.y == m_tabs[k].window->Viewport->Size.y)
			{
				//主窗口调整大小和位置、主窗最大化拖小等.imgui窗口拖动不需要另外处理
				if (m_tabs[k].window->DockNode &&m_tabs[k].clickitem && (m_tabs[k].formstatus != SW_NORMAL || m_tabs[k].drag || ImGui::IsMouseDragging(0)))//点击切换、正在拖（停止后还要执行一次定位操作）或开始拖
				{//最大化，子窗没有自己的独立句柄，通过修改节点大小实现最大化.如果要单独句柄可参考ImWindow，要切换Context ,容易出问题
					bool ismoveMergedwin = m_tabs[k].formstatus == SW_NORMAL && !m_tabs[k].drag&&IsMerged(m_tabs[k].window);//拖动被包含的窗口不需要改变状态
					if (!ismoveMergedwin&&!(m_tabs[k].ismerged&&ImGui::IsMouseDragging(0)))//拖最大化的被包含窗口也不缩小
					{
						ImVec2 newpos = m_tabs[k].window->Viewport->Pos;
						Adjustwindow(&m_tabs[k], viewport->WorkSize, newpos);//这个只设主窗大小和位置，拖动时传出新位置
						if (!ImGui::IsMouseDragging(0))
						{//正在拖不改
							if (m_tabs[k].drag)//除拖动外只做一次
								SetStatus(m_tabs[k].window, m_tabs[k].formstatus, false);//若有修改则同一窗口下的子窗全改修改,不然有的就拖不了
							else
							{
								SetStatus(m_tabs[k].window, (m_tabs[k].formstatus == SW_MAXIMIZE ? SW_MAXIMIZE : SW_NORMAL), true);//保存修改后的实际状态
								SetStatus(m_tabs[k].window, SW_NORMAL, false);//还原传值变量
							}
							m_tabs[k].clickitem = false;
						}
					}
				}
			}

			if (m_redrawtab == NULL && ((m_tabs[k].NewWinSize.Max.x - m_tabs[k].NewWinSize.Min.x) > 0 && m_tabs[k].window->Size.x != (m_tabs[k].NewWinSize.Max.x - m_tabs[k].NewWinSize.Min.x)))
			{//第一步调整窗口大小（只用在放大，恢复一步到位）,不移位置，免得闪,绘完窗口后才移位以免闪. SetWindowPos可以设在屏幕外，但画板只能画到屏幕内，所以还是会有点闪。
				m_redrawtab = &m_tabs[k];
				::SetWindowPos(m_redrawtab->hwnd, NULL, m_redrawtab->NewWinSize.Min.x, m_redrawtab->NewWinSize.Min.y, m_redrawtab->NewWinSize.Max.x - m_redrawtab->NewWinSize.Min.x, m_redrawtab->NewWinSize.Max.y - m_redrawtab->NewWinSize.Min.y, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
				//::MoveWindow(m_redrawtab->hwnd, m_redrawtab->window->Pos.x, m_redrawtab->window->Pos.y, m_redrawtab->NewWinSize.Max.x - m_redrawtab->NewWinSize.Min.x, m_redrawtab->NewWinSize.Max.y - m_redrawtab->NewWinSize.Min.y, TRUE);
				m_redrawtab->setwinstep = 1;
				if (!IsHostWin(m_redrawtab->window))
					Sleep(200);//多屏需要停一下,不然移不到项点
			}
		}

		//移到浮动框
		if (node != NULL && (k + 1 == m_tabs.size()))//新增的
		{//把新增窗口加到平台所在的组，在哪个平台点的添加就加到哪个平台组。只用一次.要放在最后不然乱了
			ImGui::SetNextWindowDockID(node->ID, ImGuiCond_Once);
			node = NULL;
		}
		else
		{
			if (m_tabs[k].window != NULL&&m_tabs[k].window->DockNode != NULL)
				ImGui::SetNextWindowDockID(m_tabs[k].window->DockNode->ID, ImGuiCond_Once);
			else
				ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Once);
		}

		//画表单
		if (((ImGuiWindow*)itemdata.PlatformUserData) == m_tabs[k].window || m_tabs[k].window&&m_tabs[k].window->DockNode&&m_tabs[k].window->DockNode->IsFocused)
		{//标签感应光标时要保证本体与tab标签颜色一致
			if (((ImGuiWindow*)itemdata.PlatformUserData) == m_tabs[k].window)
				ImGui::PushStyleColor(ImGuiCol_WindowBg, style.Colors[ImGuiCol_TabHovered]);
			else
				ImGui::PushStyleColor(ImGuiCol_WindowBg, style.Colors[ImGuiCol_TabActive]);
		}
		else
			ImGui::PushStyleColor(ImGuiCol_WindowBg, style.Colors[ImGuiCol_TabUnfocusedActive]);

		bool tabopen = m_tabs[k].open || !IsWindowVisible(m_tabs[k].hwnd);//不可见的也关掉，防有没有清空形成一个隐形的窗口，拖入窗口却消失
		char namebuf[512];
		ImFormatString(namebuf, 512, "%s_%d", m_tabs[k].title.c_str(), k);
		ImGui::Begin(namebuf, &tabopen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiDockNodeFlags_NoResize);
		m_tabs[k].tabcount = tabcount;
		m_tabs[k].tabwidth = tabcount*tabwidth;
		m_tabs[k].window = ImGui::GetCurrentWindow();
		m_tabs[k].hwnd = (HWND)m_tabs[k].window->Viewport->PlatformHandle;

		if (!tabopen)
			closedtab = &m_tabs[k];

		if (addtabcount != 0 && k + 1 == m_tabs.size())
		{//激活当前页,新添加的在最后
			ImGui::SetWindowFocus();
			m_tabs[k].window->DockNode->TabBar->SelectedTabId = m_tabs[k].window->DockNode->TabBar->NextSelectedTabId = m_tabs[k].window->DockNode->SelectedTabId;
			Sleep(150);
		}
		//画控件
		if (m_redrawtab == NULL &&m_tabs[k].window->DockNode)
		{//因每个都是单独的页，都需要画自己的标题栏
			ImGuiWindow* window = g.CurrentWindow;
			const ImRect titleBarRect = window->TitleBarRect();
			ImGui::PushClipRect(titleBarRect.Min, titleBarRect.Max, false);
			//画标题栏
			DrawTittleBar(&m_tabs[k], ImRect(window->Pos.x, window->Pos.y, window->Pos.x + window->Size.x, window->Pos.y + window->Size.y), TITLE_HEIGHT);
			//ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 20.0f);//圆形按钮
			ImGui::PopClipRect();

			float framePadding = style.FramePadding.y;
			style.FramePadding.y = 5;//因tab修改成了10，这里改小，不然输入框太高

			DrawToolBar(&m_tabs[k]);

			//加藏条
			ImGui::Separator();

			style.FramePadding.y = framePadding;
		}
		ImGui::End();
		ImGui::PopStyleColor(1);//表单本体底色

	}

	if (closedtab != nullptr)
	{
		if (m_tabs.size() == 1)
		{//加一条空记录,拖出去的不管它，最后一个子页直接关闭平台
			AddTab(closedtab->window);
		}
		RemoveTab(closedtab->window);
	}
	itemdata.PlatformUserData=NULL;//传窗口
	itemdata.RendererUserData = NULL;//传图标组
}
bool MainWindow::ViewportIsMinimized(ImGuiViewportP* viewport)
{
	if ((viewport->Flags & ImGuiViewportFlags_CanHostOtherWindows) == 0)
		return false;
	if ((viewport->Flags & ImGuiViewportFlags_Minimized) != 0)
		return false;
}
void MainWindow::DrawTittleBar(TabItemInfo* tab, ImRect oRect, float ftitlehight)
{
	const ImGuiStyle& oStyle = ImGui::GetStyle();
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;//这里得到的才是Begin后新窗口大小，受SetNextWindowSize影响.只标题栏的大小
	float framePadding = oStyle.FramePadding.y;//因用y来调tab高
	ImGui::SetCursorPosX(1);
	ImVec2 pos = ImGui::GetWindowPos();//画笔位置
	int windorwleft = pos.x;//记下窗口初始位置。pos后面有修改

	ImVec2 size(oRect.Max.x - oRect.Min.x, oRect.Min.y + ftitlehight);//窗口大小
	int leWidth = size.x - tab->tabwidth - 2 * oStyle.WindowPadding.x;//计算剩下的空间，确定是否需要按钮

	float icowidth = 0;
	bool issubwin = tab->window->DockNode != NULL&&IsSubwin(tab->window->DockNode) && !(tab->window->Pos.y - 10 < tab->window->Viewport->Pos.y&&tab->window->Pos.x + tab->window->Size.x + 10 > tab->window->Viewport->Pos.x + tab->window->Viewport->Size.x);//是否是子窗，子窗不画图标、最大最小按钮.多屏时只在右上屏显示最大化、最小化按钮

	if (tab->window != NULL&&tab->window->DockNode != NULL)
	{
		if (tab->window->DockNode->TabBar != NULL)
			icowidth = tab->window->DockNode->TabBar->BarRect.Min.x - tab->window->Pos.x;//画tab留出来的宽度
	}

	if (!issubwin&&icowidth > 16)
	{
		int id = (intptr_t)window->ID + (m_contrlId++);
		DrawImage(id, ImVec2(pos.x + (icowidth - 16) / 2, pos.y - 2 + (ftitlehight - 16) / 2), "AppIco", ImVec2(16, 16));
	}

	if (icowidth == 0)
	{//标题名
		ImGui::SameLine();
		ImGui::SetCursorPosX(35);
		ImGui::Text(tab->title.c_str());
		ImGui::SameLine();//注意，没有用SameLine()，显示一个控件后DC.CursorPos就变成下一行了
	}
	//添加新增标签按钮
	if ((oRect.Max.x - oRect.Min.x) - tab->tabwidth > 5 * m_fBtnHight)
	{
		ImVec2 oPos = ImGui::GetWindowPos();
		oPos.x += tab->tabwidth + (issubwin ? 0 : TITLE_HEIGHT) + 20;//还要去掉图标
		oPos.y += framePadding / 2 + 1;
		const float close_button_sz = g.FontSize;
		ImRect btn(oPos, oPos + ImVec2(m_fBtnHight, m_fBtnHight));
		ImVec2 newpos = ImVec2(btn.Max.x - m_rightpadding * 2.0f - close_button_sz, btn.Min.y);
		const ImGuiID add_button = window->GetID((void*)((intptr_t)window->ID + (m_contrlId++)));

		if (AddButton(tab->hwnd, add_button, newpos, ImGuiAddBtn, true))
		{
			if (!ImGui::IsMouseDragging(0))
				m_addwindow = tab->window;
		}
		if (ImGui::IsItemHovered())
			ShowTooltip(Local("Newtab"));
	}

	//右边按钮
	const float close_button_sz = g.FontSize;
	pos = ImGui::GetWindowPos();
	float btnwidth = (3.f * m_fBtnHight + 4 * framePadding);//按钮区

	//最小化按钮
	pos.y += framePadding / 2;
	pos.x = windorwleft + size.x - btnwidth;//要用绝对坐标
	if (!issubwin&&leWidth > 4 * m_fBtnHight)
	{
		ImRect b_MinBtn(pos, pos + ImVec2(m_fBtnHight, m_fBtnHight));
		const ImGuiID close_button_idMin = window->GetID((void*)((intptr_t)window->ID + (m_contrlId++)));
		if (AddButton(tab->hwnd, close_button_idMin, ImVec2(b_MinBtn.Max.x - framePadding - close_button_sz, b_MinBtn.Min.y), ImGuiMinBtn, true))
		{//当子窗完全在主窗内时会自动合并，多个窗口都共用主窗口句柄。所以子窗不能用句柄处理
			if (!ImGui::IsMouseDragging(0))
				if (!IsMerged(tab->window))
					SetMinimized(tab->hwnd);
				else
				{
					ImGuiViewportP* viewport = tab->window->Viewport;
					const bool platform_funcs_available = viewport->PlatformWindowCreated;
					if (g.PlatformIO.Platform_GetWindowMinimized && platform_funcs_available)
					{
						bool minimized = g.PlatformIO.Platform_GetWindowMinimized(viewport);
						if (minimized)
							viewport->Flags |= ImGuiViewportFlags_Minimized;
						else
							viewport->Flags &= ~ImGuiViewportFlags_Minimized;
					}
				}
		}
		if (ImGui::IsItemHovered((ImGuiHoveredFlags_DelayNormal)))
			ShowTooltip(Local("Min"), true);
	}

	//最大化按钮
	pos.x += m_fBtnHight + framePadding + 5;
	if (!issubwin&&leWidth > 3 * m_fBtnHight)
	{
		ImRect b_MaxBtn(pos, pos + ImVec2(m_fBtnHight, m_fBtnHight));
		const ImGuiID close_button_idMax = window->GetID((void*)((intptr_t)window->ID + (m_contrlId++)));
		bool ismax = !(tab->window->Viewport->Size.x == m_fullsize.x&&tab->window->Viewport->Size.y == m_fullsize.y);
		if (AddButton(tab->hwnd, close_button_idMax, ImVec2(b_MaxBtn.Max.x - framePadding - close_button_sz, b_MaxBtn.Min.y), ismax ? ImGuiMaxBtn : ImGuiIsMaxBtn, true))
		{
			if (!ImGui::IsMouseDragging(0))
				WinMaximized(tab->window);
		}
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			ImGui::SetTooltip(Local("Max"), true);
	}
	//关闭按钮
	if (leWidth > 2 * m_fBtnHight)
	{
		pos.x += m_fBtnHight + framePadding + 5;
		ImRect b_CloseBtn(pos, pos + ImVec2(m_fBtnHight, m_fBtnHight));
		const ImGuiID close_button_idClose = window->GetID((void*)((intptr_t)window->ID + (m_contrlId++)));
		if (AddButton(tab->hwnd, close_button_idClose, ImVec2(b_CloseBtn.Max.x - framePadding - close_button_sz, b_CloseBtn.Min.y), ImGuiCloseBtn, true))
		{
			TabItemInfo* firsttab = nullptr;//关闭后最上面的窗口
			bool ismaintitlebar = (pos.x + 25 > tab->window->Viewport->Pos.x + tab->window->Viewport->Size.x) && (pos.y < tab->window->Viewport->Pos.y + TITLE_HEIGHT);//判断是不是点击标题栏右上角按钮
			for (int k = 0; k < m_tabs.size(); k++)
			{//标签上关单个，标题栏关全部
				if (ismaintitlebar&&m_tabs[k].hwnd == tab->hwnd || m_tabs[k].window->DockNode == tab->window->DockNode)//同一窗口上的
					m_tabs[k].open = false;//设成false就不会显示了，tab关完窗口就消失了
				else
					if (firsttab == nullptr)
						firsttab = &m_tabs[k];
			}

			if (tab->hwnd == ImGui::GetMainViewport()->PlatformHandle)
			{//关主窗时找个替换窗口，把主窗上的tab移到替换窗口上
				if (firsttab == nullptr)
					::PostQuitMessage(0);
				else//交换窗口。不能关host窗口，用它初始化dx11的
				{//没找到直接关host的方法，它绑定了显示方面的东西，用替换的方法
					::SetWindowPos(m_pHandle, NULL, firsttab->window->Viewport->Pos.x, firsttab->window->Viewport->Pos.y, firsttab->window->Viewport->Size.x, firsttab->window->Viewport->Size.y, SWP_NOZORDER | SWP_NOACTIVATE);//用SetNextWindowPos移主窗不起作用
					HWND closehnd = firsttab->hwnd;
					for (int k = 0; k < m_tabs.size(); k++)
					{//替换主窗
						if (m_tabs[k].hwnd == firsttab->hwnd)
							m_tabs[k].window->Viewport->PlatformHandle = m_tabs[k].window->Viewport->PlatformHandleRaw = (void*)m_pHandle;
					}
				}
			}
			else//关闭当前窗后，把光标转移到新活动窗口，不然就要多点一次
				if (ismaintitlebar&&firsttab != nullptr)
				{
					ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
					platform_io.Platform_SetWindowFocus(firsttab->window->Viewport);
					Sleep(50);
				}
		}
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			ImGui::SetTooltip(Local("Close"), true);
	}
}

bool MainWindow::DrawCircleButton(int &id, const ImVec2& pos, char* icoId, const ImVec2& imageSize, bool abled, const char* label, void * texture)
{
	ImTextureID texId = texture;
	if (NULL == texture&&NULL != icoId)
		texId = GetTexture(icoId);
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	ImVec2 btnsz = pos;//pos为左上角坐标
	ImVec2 label_size(0, 0);
	if (label != NULL)
		label_size = ImGui::CalcTextSize(label, NULL, true);

	int circleR = ImGui::GetFrameHeight();//作直径用
	if (imageSize.y > circleR)
		circleR = imageSize.y;
	btnsz.x += label_size.x + circleR;
	if (label != NULL&&imageSize.x > 0)
	{//图文按钮加间隔。左 中 右
		btnsz.x += circleR;
		btnsz.y += (imageSize.x > 12 ? 2.0f : 1.0f);//圆时有加外圈
	}
	btnsz.y += circleR;

	const ImRect btn(ImVec2(pos.x, pos.y - ((label != NULL&&imageSize.x > 0) ? (imageSize.x > 12 ? 2.0f : 1.0f) : 0)), btnsz);
	const ImGuiID button_id = window->GetID((void*)((intptr_t)window->ID + (id++)));
	ImU32 colText = ImGui::GetColorU32(ImGuiCol_Text);
	if (!abled)
	{//灰按钮
		colText = ImGui::GetColorU32(ImGuiCol_TextDisabled);
	}

	bool is_clipped = !ImGui::ItemAdd(btn, button_id);
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(btn, button_id, &hovered, &held, abled ? ImGuiButtonFlags_PressedOnClick : ImGuiItemFlags_Disabled);
	if (is_clipped)
		return pressed;

	int addpress = (hovered && (held || GetAsyncKeyState(VK_LBUTTON) & 0x8000)) ? 1 : 0;//按下动画

	ImU32 col = ImGui::GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered);
	ImVec2 center = btn.GetCenter();

	ImRect image_bb(ImVec2(center.x - imageSize.x / 2 + addpress, center.y - imageSize.y / 2 + addpress), ImVec2(center.x + imageSize.x / 2 + addpress, center.y + imageSize.y / 2 + addpress));
	if (label != NULL)//图文混合，起始位从半圆开始
		image_bb = ImRect(ImVec2(pos.x + circleR / 2 + addpress, pos.y + addpress), ImVec2(pos.x + circleR / 2 + imageSize.x + addpress, pos.y + circleR + addpress));
	//文字位置
	ImRect text_bb = ImRect(ImVec2(center.x - label_size.x / 2 + addpress, center.y - circleR / 2 + addpress), ImVec2(center.x + label_size.x / 2 + addpress, center.y + circleR / 2 + addpress));
	if (imageSize.x > 0)//图文混合，起始位去掉半圆，图片宽
		text_bb = ImRect(ImVec2(pos.x + circleR / 2 + imageSize.x + 3 + addpress, pos.y + addpress), ImVec2(pos.x + circleR / 2 + imageSize.x + 3 + label_size.x + addpress, pos.y + circleR + addpress));

	if (hovered)
	{
		if (label_size.x > imageSize.x)
		{//文字在圆装不下时画长条椭圆
			window->DrawList->AddRectFilled(ImVec2(btn.Min.x + addpress, btn.Min.y + addpress), ImVec2(btn.Max.x + addpress, btn.Max.y + addpress), col, ((imageSize.x > 12 ? 2.0f : 1.0f) + circleR / 2));
		}
		else
			window->DrawList->AddCircleFilled(ImVec2(center.x + addpress, center.y + addpress), ImMax(2.0f, g.FontSize * 0.5f + (imageSize.x > 12 ? 5.0f : 1.0f)), col, 12);
	}

	if (!hovered&&label != NULL)
	{//非活动图文按钮显示个印子
		col = ImGui::GetColorU32(ImGuiCol_Button);
		window->DrawList->AddRectFilled(ImVec2(btn.Min.x + addpress, btn.Min.y + addpress), ImVec2(btn.Max.x + addpress, btn.Max.y + addpress), col, ((imageSize.x > 12 ? 2.0f : 1.0f) + circleR / 2));
	}
	if (texId != NULL)
		window->DrawList->AddImage(texId, image_bb.Min, image_bb.Max, ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.00f)));

	if (label != NULL)
	{//显示字和字的颜色
		const ImGuiStyle& style = g.Style;
		ImVec4 co = style.Colors[ImGuiCol_Text];
		ImGui::PushStyleColor(ImGuiCol_Text, co);
		label_size.x -= 2;
		label_size.y += 2;
		//p0.x += imageSize.x+15;//去掉图标位置
		ImGui::RenderTextClipped(text_bb.Min, text_bb.Max, label, NULL, &label_size, style.ButtonTextAlign, &text_bb);
		ImGui::PopStyleColor(1);
	}

	return held;
}
void MainWindow::ShowTooltip(const char* pText, bool needdelay)
{//提示框显示计时，分延时和不延时两种
	ImGuiID id = ImGui::GetHoveredID();
	//ImGui::SetTooltip(pText);
	if (id > 0)
	{
		if (pText != NULL&&strlen(pText) > 0)
		{
			if (id == m_tipid)
			{
				if (!needdelay&&GetTickCount() - m_tiptime > 100 || GetTickCount() - m_tiptime > 1000)
				{
					ImGuiContext& g = *GImGui;
					char namebuf[512];
					ImFormatString(namebuf, 512, "%s_%d", "tip", 2000);
					bool p_open = true;
					ImVec2 txt_size = ImGui::CalcTextSize(pText, NULL, true);
					ImVec2 oCursorPos = ImGui::GetIO().MousePos;
					oCursorPos.x += 10;
					oCursorPos.y += 20;
					txt_size.x += 20;
					txt_size.y -= 20;
					const ImGuiViewport* viewport = ImGui::GetMainViewport();
					if (oCursorPos.x + txt_size.x+5>viewport->Pos.x+ viewport->Size.x)
					{//出界会出现视图比例不对，控制在窗口内则OK
						oCursorPos.x = oCursorPos.x - txt_size.x-5;
					}
					if (oCursorPos.y + txt_size.y + 5>viewport->Pos.y + viewport->Size.y)
					{
						oCursorPos.x = oCursorPos.x - txt_size.x - 15;
						oCursorPos.y = oCursorPos.y - txt_size.y - 15;
					}
					ImGui::SetNextWindowPos(oCursorPos);
					ImGui::SetNextWindowSize(txt_size);
					ImGui::SetNextWindowBgAlpha(0.35f);
					ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
					ImGui::Begin(namebuf, &p_open, window_flags);
					ImGuiWindow* window = ImGui::GetCurrentWindow();
					ImGui::SetCursorPosX(10);
					ImGui::SetCursorPosY(5);
					ImGui::Text(pText);
					ImGui::End();
				}
			}
			else
			{
				m_tiptime = GetTickCount();//记时开始
				m_tipid = id;
			}
		}
	}
}
char* MainWindow::Local(const char* name)
{
	if(m_Config.localstr.count(name))
		return m_Config.localstr[name];
	return "";
}
void MainWindow::DrawToolBar(TabItemInfo* tab, bool isTab)
{
	int fontpadding =8;//字体调整需要调整的量
	const ImGuiStyle& oStyle = ImGui::GetStyle();
	ImGuiContext& g = *GImGui;

	ImGuiWindow* window = g.CurrentWindow;
	ImVec2 pos = window->DC.CursorPos;
	pos.x = pos.x - 22 + fontpadding;//调整位置
	pos.y += 7;
	bool able = false;
	ImRect b_Btn(pos, pos + ImVec2(m_fBtnHight, m_fBtnHight));
	if (AddButton(tab->hwnd, window->GetID((void*)((intptr_t)window->ID + (m_contrlId++))), ImVec2(b_Btn.Max.x, b_Btn.Min.y - 3), ImGuiGoBackBtn, false /*pWindow->CanGoBack()*/))
	{
		//pWindow->GoBack();
	}
	if (ImGui::IsItemHovered((ImGuiHoveredFlags_DelayNormal)))
		ShowTooltip(Local("Back"), true);

	pos.x += m_fBtnHight + 5 + fontpadding;
	ImRect b_BtnF(pos, pos + ImVec2(m_fBtnHight, m_fBtnHight));
	if (AddButton(tab->hwnd, window->GetID((void*)((intptr_t)window->ID + (m_contrlId++))), ImVec2(b_BtnF.Max.x, b_BtnF.Min.y - 3), ImGuiForwardBtn, false /*pWindow->CanGoForward()*/))
	{
		//pWindow->GoForward();
	}
	if (ImGui::IsItemHovered((ImGuiHoveredFlags_DelayNormal)))
	    ShowTooltip(Local("Forward"), true);

	pos.x += m_fBtnHight + 8 + fontpadding;
	ImRect b_BtnR(pos, pos + ImVec2(m_fBtnHight, m_fBtnHight));
	if (1/*pWindow->IsLoading()*/)
	{
		if (AddButton(tab->hwnd, window->GetID((void*)((intptr_t)window->ID + (m_contrlId++))), ImVec2(b_BtnR.Max.x, b_BtnR.Min.y - 3), ImGuiStopLoadingBtn, false))
		{
			//pWindow->StopLoading(0);
		}
		if (ImGui::IsItemHovered((ImGuiHoveredFlags_DelayNormal)))
			ShowTooltip(Local("Stop"), true);
	}
	else
	{
		if (AddButton(tab->hwnd, window->GetID((void*)((intptr_t)window->ID + (m_contrlId++))), ImVec2(b_BtnR.Max.x, b_BtnR.Min.y - 3), ImGuiRefreshBtn, false /*pWindow->CanRefresh()*/))
		{
			//pWindow->Refresh();
		}
		if (ImGui::IsItemHovered())
			ShowTooltip(Local("Refresh"), true);
	}

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4 tempFrameBg = style.Colors[ImGuiCol_FrameBg];

	float winwidth = ImGui::GetWindowWidth();
	ImGui::SetNextItemWidth(winwidth - 92 - 20);//减掉前后的宽度就是输入框的宽
	ImGui::Spacing();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() +65 + 4 * fontpadding);//空格
	static char url[MAX_PATH] = "";
	if (ImGui::InputTextWithHint("##urlTxt", "", url, MAX_PATH, ImGuiInputTextFlags_EnterReturnsTrue) == true)
		winwidth = winwidth;
}