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
{//��imgui�����ʽ���������������������
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

	m_parentHandle = parent;//����ȷ���ǲ�����ҳ
	SetWindowLongPtr(m_pHandle, GWLP_USERDATA, (LONG_PTR)this);
	set_borderless(m_pHandle, borderless);
	set_borderless_shadow(m_pHandle, borderless_shadow);
	::ShowWindow(m_pHandle, SW_SHOW);
	m_fullsize = GetFullscreen();

	if (parent != nullptr)//�ϳ��Ĵ��ڣ��������
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
	ImGui_ImplWin32_Init(m_pHandle);//�󶨾��
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
}
// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static LRESULT WINAPI ImGuiWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{//�ϳ����Ĵ�����Ϣ��imgui_impl_win32.cpp�д���
	switch (msg)
	{
	case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
	{
		if (WM_LBUTTONDBLCLK == msg || WM_LBUTTONDOWN == msg)//ֻ����ϲ���ʾ���Ӵ����ó�����󻯵ı������ĵ������������������ʱ�Ѿ��ѱ�������ɿͻ����ˣ��ʲ��Ǵ�WM_NCLBUTTONDBLCLK
			if (auto window_ptr = reinterpret_cast<MainWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
				auto& window = *window_ptr;
				ImVec2 cursorPos = window.CursorPos();
				ImGuiWindow* win = window.GetCursorWin(hwnd, cursorPos);//�ҳ�������ڴ���
				if (win != NULL)
				{
					ImRect rect = ImRect(win->Pos, win->Pos + win->Size);//�з���ʱ�õ������������ڵĴ�С
					rect.Max.y = rect.Min.y + TITLE_HEIGHT;
					if (rect.Contains(cursorPos))
					{//ֻ�����������ɿͻ�������¼�
						if (WM_LBUTTONDOWN == msg)
							window.SaveWinSize(win);
						//window.ClickPosition(win, cursorPos);//���������ʱҪ������λ�ã����������϶�ʱ����ڱ�������λ��
						if (WM_LBUTTONDBLCLK == msg)
						{//���������1 �Ӵ����� 2 ������󻯵ģ������Լ�����ˣ���windows�ڲ�����SW_NORMAL.��Щ��Ҫ�Լ��ó�������󻯼���ԭ��
							ImVec2 fullsize = window.GetFullscreen();
							if ((window.IsMerged(win) || win->Viewport->Size.x == fullsize.x&&win->Viewport->Size.y == fullsize.y))
								window.WinMaximized(win);//��API����WM_NCLBUTTONDOWN�е��ߴ������ϣ������ϵĹ���imgui�ǻ治�����ģ�Ҳ���ǿ��������ϵ�Ԫ��
						}
					}
				}
			}
	}
	break;
	case WM_NCLBUTTONUP://ȡ����
		break;
	case WM_NCLBUTTONDBLCLK://������˫���¼���WM_NC�������¼�. �϶��ڳ��洰��Ӧ������Ѿ���������������
	{//��������Ϣѭ���ƺ�������ִ�������ĺ�������ͬ����MergedTitleDoubleClick��WM_NCLBUTTONDBLCLK�²��У���WM_LBUTTONDBLCLK������ԡ��ĳ����������ǣ�Ȼ���ڻ���ʱ��󻯡���Ҫ�ǻ���ͬ����ԭ����API�Ŵ�ģ���С��ԭʱ����������󻯣�������覴á�
		if (auto window_ptr = reinterpret_cast<MainWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
			auto& window = *window_ptr;
			ImVec2 cursorPos = window.CursorPos();
			ImGuiWindow* win = window.GetCursorWin(hwnd, cursorPos);
			window.WinMaximized(win);//����������Ϣ��imgui���Ǽ�ⲻ��,�����ǣ���imgui����ʱ����󻯻�ԭ
		}
		return 0;//ע���������ִ�б������ڲ���������󻯣��϶�ʱimgui������С�޷����ô�С
	}
	break;
	}

	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))//������Ϣ����������
		return true;

	if (auto window_ptr = reinterpret_cast<MainWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA))) {
		auto& window = *window_ptr;
		switch (msg)
		{
		case WM_NCHITTEST:
		{//�϶����͵���С��When we have no border or title bar, we need to perform our own hit testing to allow resizing and moving.
			if (window.IsFullscreen(hwnd))
				return HTCLIENT;//ȫ�����ٰ�������������������Сʱ���Լ�ʱ��������imgui�����ͺ�У�������ȫ������Կ�����
								//return HTTOPLEFT;
								//return HTTRANSPARENT;
			if (borderless&&window.IsDragArea(hwnd))
			{//û�����ʱ����λ�þ�����Щ��������϶������С�����������������洰��������imgui��Ч����	
			 //OutputDebugStringA((to_string(GET_X_LPARAM(lparam)) + "--�϶���--" + to_string(GET_Y_LPARAM(lparam)) + "\n").c_str());
				LRESULT result = hit_test(hwnd, POINT{ GET_X_LPARAM(lparam),GET_Y_LPARAM(lparam) });
				return result;
			}
		}
		break;
		case WM_NCCALCSIZE: {//ȥ��������
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
			{//����������ر�ʱ,�����޸�״̬�رմ����µ������Ӵ�
				if (m_tabs[k].hwnd == hwnd)
					m_tabs[k].open = false;
			}
			return 0;
		}
		case WM_IME_CHAR:
		{//������������
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
			{//������˫����ʾ����
				SetForegroundWindow(hwnd);
				ShowWindow(hwnd, SW_SHOWNORMAL);
				DeleteTray(hwnd);
			}
			break;
		case WM_SIZE:
		{
			bool isminized = window.IsMinimized(hwnd);
			//if (window.g_pd3dDevice != NULL && wparam != SIZE_MINIMIZED)//wparam��ʱ����
			if (window.g_pd3dDevice != NULL && !isminized)
			{
				window.CleanupRenderTarget();
				window.g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lparam), (UINT)HIWORD(lparam), DXGI_FORMAT_UNKNOWN, 0);
				window.CreateRenderTarget();
			}
			if (isminized)
			{//����
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
	if (hovered && (held || GetAsyncKeyState(VK_LBUTTON) & 0x8000))//����󱣳ְ���
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
			if (btn.Contains(cursor) && ImGui::IsMouseHoveringRect(btn.Min, btn.Max))//�������ƽ̨��ť��hovered
				window->DrawList->AddCircleFilled(center, ImMax(2.0f, btn.GetWidth() / 2 - 1), col, num_segments);
		}

	if (pressed)
	{//������������������
		if (GetTickCount() - m_mouseTickCount < 100)
		{
			pressed = false;
			//g.IO.MouseDown[0] = false;
		}
		else
		{
			m_mouseTickCount = GetTickCount();
			//m_clicklock = true;//ֱ������ֻһ��
		}
	}
	//float cross_extent = g.FontSize * 0.5f * 0.7071f - 1.0f;//�Զ�����С
	float cross_extent = 16 * 0.5f * 0.7071f - 1.0f;
 center = ImVec2(center.x - 0.5f, center.y - 0.5f);
	float size = 6.0f;//+�ִ�С

	switch (type)
	{//�����ο�����ɫһ��
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
		//window->DrawList->AddCircle(ImVec2(center.x, center.y), size, colText, 20, 2.0f);//��Բ
		// window->DrawList->AddLine(ImVec2(center.x, center.y - size+4), ImVec2(center.x, center.y - size + 4), col, 8.0f);//��������
		window->DrawList->AddTriangleFilled(ImVec2(center.x, center.y - size - 2), ImVec2(center.x, center.y - size + 5), ImVec2(center.x + 6, center.y - size + 2), colText);
		//window->DrawList->AddTriangle(ImVec2(center.x, center.y - size - 1), ImVec2(center.x, center.y - size + 4), ImVec2(center.x + 5, center.y - size + 2), colText, 2.0f);//��ͷ
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
		int sz = 5;//С�����δ�С
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
		//��̫����window->DrawList->AddText(g.IO.FontDefault, 20.0f, ImVec2(center.x - 4, center.y - 15), ~0U, "...");
		break;
	case ImGuiSetting:
	{
	}
	break;
	case ImGuiProxy:
		window->DrawList->AddText(g.IO.FontDefault, 16.0f, ImVec2(center.x - 2, center.y - 8), ImGui::GetColorU32(ImGuiCol_Text), "P");
		//window->DrawList->AddText(g.IO.FontDefault, 16.0f, ImVec2(center.x - 2, center.y - 8), ~0U, "P");//д���μ�����ɫ
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
		int sz = 5;//С�����δ�С
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
	{//����û���Զ���ɫ�������У�������ȫ��ֱ���߾ͻ��г�״
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
{//������λ�ã�������λ�ã���ִ�����.����SaveWinSize�ϲ�
	for (int k = 0; k < m_tabs.size(); k++)
	{
		if (m_tabs[k].window != NULL&&m_tabs[k].window == win)
		{//���µ��λ�����󶥵����
			cursorPos.x = m_tabs[k].MainSize.x*(cursorPos.x*1.00 / m_fullsize.x);//Ҫ�������ı���
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
{//�������д���״̬����ԭ��
	TabItemInfo* tab = NULL;
	ImVec2 cursorPos = CursorPos();
	//bool issinglewin = IsMerged(win);//�ǲ��ǵ��������ܰ���
	for (int k = 0; k < m_tabs.size(); k++)
	{
		//if (m_tabs[k].laststatus != SW_MAXIMIZE && !(m_tabs[k].window->Size.x == m_fullsize.x&&m_tabs[k].window->Size.y == m_fullsize.y))
		if (!(m_tabs[k].window->Viewport->Size.x == m_fullsize.x&&m_tabs[k].window->Viewport->Size.y == m_fullsize.y))
		{//ȫ�����棬��Ϊһ����������������ڿ��ܱ��ϲ���ͼ
			m_tabs[k].Pos = m_tabs[k].window->Pos;
			m_tabs[k].Size = m_tabs[k].window->Size;
			m_tabs[k].MainPos = m_tabs[k].window->Viewport->Pos;
			m_tabs[k].ismerged = false;
			if (IsMerged(m_tabs[k].window))//�ϲ���ͼ��������������һ��Viewport,Ҫ�ܻ�ԭ���Լ��Ĵ�С��������������С
			{
				m_tabs[k].ismerged = true;//��ԭʱҪ��
				m_tabs[k].MainSize = m_tabs[k].window->Size;
			}
			else
				m_tabs[k].MainSize = m_tabs[k].window->Viewport->Size;
		}

		m_tabs[k].clickitem = false;
		if (m_tabs[k].window->ViewportId == win->ViewportId)
		{//ͬһ��ƽ̨����ͬһ���ڵ㣬����ImGuiWindow����tab
			ImRect rect = ImRect(m_tabs[k].window->Pos, m_tabs[k].window->Pos + m_tabs[k].window->Size);
			rect.Max.y = rect.Min.y + TITLE_HEIGHT;
			if (rect.Contains(cursorPos))//�ù���жϵ������һ��
			{
				m_tabs[k].clickitem = true;
				tab = &m_tabs[k];

				//���µ��λ�����󶥵����
				ImVec2 tmp = cursorPos;
				tmp.x = m_tabs[k].Size.x*(cursorPos.x*1.00 / m_fullsize.x) + (m_tabs[k].Pos.x - m_tabs[k].MainPos.x);//�����Ĵ����Ե����Ŵ���С������λ�ü������ƫ�Ƽ���ƫ�������߻��з�����
				m_tabs[k].clickposition = tmp;//���λ��
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
		else//�ָ�
			tab->formstatus = SW_RESTORE;
		SetStatus(tab->window, tab->formstatus, false);//���浱ǰ״̬
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
	for (int i = g.Windows.Size - 1; i >= 0; i--)//�ҵ��˴���
	{//���д��ڶ������棬����¼�˴��ڵ���ʾ˳��0����ײ㣬Size-1��ʾ����ǰ��.��ǰ������
		ImGuiWindow* win = g.Windows[i];
		if (win->Viewport != NULL&&win->Active&&hwnd == win->Viewport->PlatformHandle&&strcmp("Debug##Default", win->Name) != 0)
		{//һ���������ж����������һ���ģ��ù��λ����ȷ�����ĸ�����
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

	ImRect rect = ImRect(window->Pos, window->Pos + window->Size);//�з���ʱ�õ������������ڵĴ�С

																  //�������������,�ð�ť�����������Ӧ�϶��¼�
	ImRect bar = rect;
	bar.Max.y = rect.Min.y + TITLE_HEIGHT;

	//�Ź��߽�����ʹ����Ӧ�����ڴ�С�¼�
	ImRect leftborder = rect;
	leftborder.Max.x = rect.Min.x + 2;
	ImRect rightborder = rect;
	rightborder.Min.x = rect.Max.x - 2;
	ImRect bottomborder = rect;
	bottomborder.Min.y = rect.Max.y - 2;
	if (leftborder.Contains(cursorPos) || rightborder.Contains(cursorPos) || bottomborder.Contains(cursorPos))
		return true;//��������ҡ��±߿�

	float icowidth = 0;//��ͼ���
	bool issubwin = (window->DockNode != NULL&&window->DockNode->ParentNode != nullptr);//�Ƿ����Ӵ����Ӵ�����ͼ�ꡢ�����С��ť
	ImRect ico = rect;
	ico.Max.x = rect.Min.x + (issubwin ? 0 : TITLE_HEIGHT);
	ico.Max.y = rect.Min.y + TITLE_HEIGHT;
	if (ico.Contains(cursorPos))
		return true;//����ڱ���ͼ����

	ImGuiContext* ctx = GImGui;
	ImGuiDockContext* dc = &ctx->DockContext;
	for (int n = 0; n < dc->Nodes.Data.Size; n++)
	{
		if (ImGuiDockNode* node = (ImGuiDockNode*)dc->Nodes.Data[n].val_p)
		{
			if (node->Windows.Size > 0)
			{//����CountNodeWithWindows>0,��WindowsΪ��
				int iSize = node->Windows.Size;
				if (node->Size.x < 180)//̫С�����϶����䣬ֻ����tab
					return false;

				ImRect subbar = ImRect(node->Pos, node->Pos + node->Size);//��������
				int btnwidth = 90;
				bool issubwin = IsSubwin(node);
				if (issubwin)
				{
					if ((node->Pos.y - 10 < node->Windows[0]->Viewport->Pos.y&&node->Pos.x + node->Size.x + 10 > node->Windows[0]->Viewport->Pos.x + node->Windows[0]->Viewport->Size.x))//���Ͻǵ�Ҫ�ſ�
						issubwin = false;//Ҫ������ť
					else
						btnwidth = 38;//�Ӵ�����Ҫ��ͼ�����󻯡���С����ť
				}

				for (int i = 0; i < node->Windows.Size; i++)
				{
					float tabwidth = node->TabBar->Tabs[i].Width*iSize;//��ʵ��ռ�������󲻹̶������
					subbar.Max.y = subbar.Min.y + TITLE_HEIGHT;
					subbar.Max.x = subbar.Min.x + node->Size.x - btnwidth - 3;//�϶���������,���ұ�����ť�ͼ��
					subbar.Min.x = subbar.Min.x + (issubwin ? 0 : TITLE_HEIGHT) + tabwidth + ((node->Size.x - tabwidth > 5 * m_fBtnHight) ? 31 : 0);//�϶������

					if (subbar.Contains(cursorPos))
					{//����ڱ�����ʱ��1 tab������Ӧ�����϶���2�����пؼ��Ĳ���Ӧ
						if (!IsMerged(node->Windows[i]))//�Ѿ��ϲ���������Ӧ����Է����ϳ�,����С���ʱ��Ϊ����������
							return true;
					}
				}
			}
		}
	}
	return false;
}
int MainWindow::WinStatus(HWND hwnd)
{//��ô���״̬
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
	return (node != NULL&&node->ParentNode != nullptr && (node->ParentNode->CountNodeWithWindows > 1 || node->ParentNode->ChildNodes[0] != NULL&& node->ParentNode->ChildNodes[1] != NULL));//��ֻһ��ʱҲ������
}
bool MainWindow::IsHostWin(ImGuiWindow* win)
{
	if (win == NULL || win->Viewport == NULL)
		return false;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (win->Viewport->PlatformHandle == viewport->PlatformHandle/*&&viewport->Size.x== win->Size.x&&viewport->Size.y == win->Size.y����ʱ����*/)
		return true;
	return false;
}
bool MainWindow::IsDockIn(ImGuiWindow* win)
{//�����Ƿ�ͣ����������ͣ�����ڵ�node�Ƕ��������������������֧����û�з����ģ����ܶ��ٸ����ڶ���node->Windows������
	if (win->DockNode != NULL &&win->DockNode->ParentNode != NULL)
	{
		ImGuiDockNode* node = win->DockNode->ParentNode;
		if (node->ChildNodes[0] != NULL || node->ChildNodes[1] != NULL)
		{
			if (node->ChildNodes[0] != NULL)
			{//������
				for (int i = 0; i < node->ChildNodes[0]->Windows.Size; i++)
				{
					if (node->ChildNodes[0]->Windows[i] == win)
						return true;
				}
			}
			if (node->ChildNodes[1] != NULL)
			{//������
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
{//�жϴ����Ƿ�ϲ���ʾ�������ͼ��.�������A����ͬ�ҳߴ�С��A��ʱ���Ǻϲ��ġ����ǲ����������ģ�������Ҫ���϶�ȫ��,����windowҲ�ǹ��þ��
	ImGuiViewportP* main_viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
	ImRect mainrect=main_viewport->GetMainRect();
	if (win->Viewport == main_viewport&&mainrect.Contains(win->Rect()))//ע��,ֻ�Ტ��host�����ϳ�ȥ���໥����Ҳ����ϲ�,���Կ��Բ��ù�
	{//��������� 1 û�з���ʱ 2 ������
		bool isdock = IsDockIn(win);
		if (isdock)//��ͣ�����㣬win���ܾ����Լ�
		{
			if (win->ParentWindow != NULL && !(main_viewport->Pos.x == win->ParentWindow->Pos.x&&main_viewport->Pos.y == win->ParentWindow->Pos.y&&main_viewport->Size.x == win->ParentWindow->Size.x&&main_viewport->Size.y == win->ParentWindow->Size.y))//�ų�����
				if (mainrect.Contains(win->ParentWindow->Rect()))
					return true;//�ϳ�ȥ��Ҳ�����Ƕ���
			return false;
		}
		else
			if (!(main_viewport->Pos.x == win->Pos.x&&main_viewport->Pos.y == win->Pos.y&&main_viewport->Size.x == win->Size.x&&main_viewport->Size.y == win->Size.y))//�ų�����
				return true;
	}
	return false;
}

int MainWindow::WinCount(ImGuiDockNode* node)
{//ͳ�ƽڵ��µĴ��ڣ���������ChildNodes[1]��Ҷ��
	ImGuiDockNode*p = node;
	//�ȵ����ڵ�
	while (p != NULL &&p->ParentNode != NULL)
		p = p->ParentNode;
	//return p->CountNodeWithWindows;//���ֻ�Ƿ�����,tab��ͨ��Windows.Sizeȡ��
	int count = 0;
	while (p != NULL && (p->ChildNodes[0] || p->ChildNodes[1]))
	{
		if (p->ChildNodes[1] != NULL)
			count += p->ChildNodes[1]->Windows.Size;//��Ҷ��
		if (p->ChildNodes[0] != NULL)
		{
			if (p->ChildNodes[0]->IsLeafNode())
				count += p->ChildNodes[0]->Windows.Size;//��Ҷ��
		}
		p = p->ChildNodes[0];
	}
	return count;
}
void MainWindow::CreateThumbnail(HWND hWndDst, HWND sourceWindow)
{//���ɴ��ڽ�ͼ������ͼ��hWndDst��ʾͼƬ�Ĵ��ھ��
	HRESULT hr = S_OK;
	hr = DwmRegisterThumbnail(hWndDst, sourceWindow, &m_Imguithumbnail);//������ͼ��DwmRegisterThumbnail(hwnd, FindWindow(_T("Progman"), NULL), &thumbnail);
	RECT dest;
	::GetClientRect(hWndDst, &dest);//ȡ�ô��ڴ�С

	DWM_THUMBNAIL_PROPERTIES dskThumbProps;
	dskThumbProps.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_VISIBLE | DWM_TNP_SOURCECLIENTAREAONLY;
	dskThumbProps.fSourceClientAreaOnly = FALSE;
	dskThumbProps.fVisible = TRUE;
	dskThumbProps.opacity = (255 * 70) / 100;//��͸���ԣ�255Ϊ��͸��
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
	//�Ȱ�ָ����
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
		m_Fonts->AddFontDefault(&font_cfg);//Ĭ�ϵİ�һ�㣬��ɫ��
	}
	//������
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
{//��ť����ͼƬͳһ����ȡ��û�е��Զ�����
	ImGuiWindow* window = GImGui->CurrentWindow;
	ImGuiID id=window->GetID(name.c_str());//ͼ��Ҫ����imgui,ͳһ��ImGuiID
	void* ptexture = m_texture[id];
	if (ptexture == NULL)
	{
		int image_width = 0;
		int image_height = 0;
		if (m_Config.nameToIcoString.count(name) > 0)
			m_favorite.LoadTextureFromFile("", m_Config.nameToIcoString[name], &ptexture, &image_width, &image_height, g_pd3dDevice);
		else//���ļ�ʱֱ�Ӽ���
			//if (name.length() != 6)//ֱ���ų�CN.png֮���
			m_favorite.LoadTextureFromFile(name.c_str(), "", &ptexture, &image_width, &image_height, g_pd3dDevice);
		m_texture[id] = ptexture;
	}
	return ptexture;
}
void MainWindow::DrawImage(int &id, const ImVec2& pos, char* imageId, const ImVec2& imageSize, void * texture)
{//��ͼ�������ļ������᳤��HASH���У���ͼ��Ҫ�������ù�����ɾ��
	ImTextureID texId = texture;
	if (NULL == texture&&NULL != imageId)
		texId = GetTexture(imageId);//����ֱ�Ӵ��ļ���
	ImGuiContext& g = *GImGui;
	ImVec2 btnsz = pos;
	btnsz.x += imageSize.x;
	btnsz.y += imageSize.y + 4;
	const ImRect btn(pos, btnsz);
	ImVec2 center = btn.GetCenter();
	const ImRect image_bb(ImVec2(center.x - imageSize.x / 2, center.y - imageSize.y / 2), ImVec2(center.x + imageSize.x / 2, center.y + imageSize.y / 2));//ͼƬλ�ô�С,�Խ�����
	if (texId != NULL)
		g.CurrentWindow->DrawList->AddImage(texId, image_bb.Min, image_bb.Max, ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.00f)));
}


void MainWindow::AddTab(ImGuiWindow* window)
{
	TabItemInfo tab;
	TabItemInfo * ptab = GetTab(window);//������
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
	//m_tabs.insert(m_tabs.begin() + index, tab);//�ڵ�i+1��Ԫ��ǰ�����tab
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
{//ͬһ����һ�����޸ĺ�������ҲҪ��
	bool issinglewin = IsMerged(window);//�ǲ��ǵ��������ܰ���
	for (vector<TabItemInfo>::iterator it = m_tabs.begin(); it != m_tabs.end(); it++)
	{
		if (it->window != NULL&&window->ViewportId == it->window->ViewportId)
		{//��������ͼ��ͬ����
			if (issinglewin&&window == it->window || !issinglewin && (window == it->window || IsDockIn(it->window)))
			{//û��ͣ���������м���tab,��ͣ��������ͬ�����Ӵ�ҲҪ��
				if (islast)
				{//����ʱ֮ǰ��ͬһ��������Ӵ��Ŵ�ʱĸ������ʾ����󻯣�ʵ��û��
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
		if (ImGuiDockNode* node = (ImGuiDockNode*)dc->Nodes.Data[k].val_p)//һ��node���Կ���һ��ƽ̨,����������Window,һ��Window����һ��tab
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
	::GetCursorPos(&oCursorPos);//��ʱImGui::GetIO().MousePos��û�м�ʱ����
								//::ScreenToClient(hWnd, &oCursorPos);
	return ImVec2(oCursorPos.x, oCursorPos.y);
}
ImVec2 MainWindow::GetFullscreen()
{
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);//��ȥ�������������С
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
	int empty = 50;//����50�հ���������
	if (issubwin)
	{//�Ӵ�ֻһ���رհ�ť����Ҫ��ͼ�����󻯡���С����ť,���ٿռ�
		if ((node->Pos.y - 10 < node->Windows[0]->Viewport->Pos.y&&node->Pos.x + node->Size.x + 10 > node->Windows[0]->Viewport->Pos.x + node->Windows[0]->Viewport->Size.x))//���Ͻǵ�Ҫ�ſ�
		{//���Ͻ���󻯰�ť
			if (node->Size.x<300)
				empty = 20;
			issubwin = false;//Ҫ������ť
		}
		else
		{
			empty = 10;
			btnwidth = 30;
		}
	}
	float fMaxSize = (node->Size.x - (issubwin ? 0 : TITLE_HEIGHT) - 10 - 30 - empty - btnwidth) / size;//�����ǩƽ��������ͼ��TITLE_HEIGHT��������ť30���ұ�����90
	if (fMaxSize > 180 || fMaxSize <= 1)
		fMaxSize = 180;
	if (issubwin&&node->Size.x / 3<fMaxSize)
		fMaxSize = node->Size.x / 3;
	return fMaxSize;//TABʵ��ռ������
}
float MainWindow::GetTabWidth(ImGuiWindow* window, int &size)
{//����tab��ռ���,�еĽڵ��ǿ��.�ҳ��������ڵĽڵ㣬����ڵ����һ������
	if (window == NULL || window->Viewport == NULL)
		return 180;
	ImGuiDockNode* node = GetNode(window);
	if (node != nullptr)
	{
		size = node->Windows.Size + (m_addwindow == window ? 1 : 0);
		float twidth = CaluTabWidth(node, size);//Windows.Size���ܻ�û��
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
{//������󻯻�ԭ����һ���������ʱ��һ����̬���Ĵ��ڿ��ܱ�������������ʱ��С�ͻ�Ӱ�쵽��̬�����ڣ���ť�����ˡ�
	if (!tab->drag&&ImGui::IsMouseDragging(0))
	{//�ų��Ǳ��������϶�
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
	bool issinglewin = IsMerged(tab->window);//�ǲ��ǵ����������Ǳ���������
	if (tab->formstatus == SW_MAXIMIZE || tab->drag || !issinglewin&&IsFullscreen(tab->hwnd) || issinglewin&&fullsize.x == tab->window->Size.x&&fullsize.y == tab->window->Size.y)
	{//��󻯻������϶�
		ImVec2 pos = ImVec2(0, 0);
		bool isdockwin = IsSubwin(tab->window->DockNode);
		if (ImGui::IsMouseDragging(0))
		{//�϶���ԭ�������϶�
			if (tab->Size.x > 0 && tab->Size.y > 0)
			{//��󻯺��϶�ʱҪ�Ȼָ�windows���ڳߴ磬���򻭲��������ʱ�Ĵ�С���϶�ʱ�Ϳ������������ı�ǩ�Ͱ�ť������������Ϣ��imgui���Ǽ�ⲻ���ġ�ֱ����api�Ͽ�������ǩ�Ͱ�ť
				if (!tab->drag)
				{//�޸Ĵ�ƽ̨�µĴ��ڴ�С��ֻ��һ��
					for (int k = 0; k < m_tabs.size(); k++)
					{
						if (m_tabs[k].hwnd == tab->hwnd&&m_tabs[k].Size.x > 0)
						{
							ImGui::DockBuilderSetNodeSize(m_tabs[k].window->DockNode->ID, m_tabs[k].Size);
							ImGui::DockBuilderSetNodePos(m_tabs[k].window->DockNode->ID, m_tabs[k].Pos);//������ᶶ
						}
					}
				}
				//�޸���λ��
				ImVec2 mousepos = CursorPos();
				pos = { mousepos.x - tab->clickposition.x ,mousepos.y - tab->clickposition.y };
				tab->drag = true;//��С�ķſ���Ҫ�ܶ�����

				if (isdockwin && !IsHostWin(tab->window))
				{//�ϳ����ڶ���״̬����Сʱ��Ҫ�Ļ�����С
					::SetWindowPos(tab->hwnd, NULL, pos.x, pos.y, tab->MainSize.x, tab->MainSize.y, SWP_NOZORDER | SWP_NOREDRAW);//��Сһ����λ
					if (tab->window->Viewport->Size.x != tab->MainSize.x)
						PostMessage(tab->hwnd, WM_SIZE, 0, (LPARAM)MAKELONG(tab->MainSize.x, tab->MainSize.y));//�ϳ���������Ҫ�޸Ļ�����С
						//SetViewportSize(tab, pos, tab->MainSize, true);
					if (tab->window->Viewport->Window)
					{//Ҫ��һ�£���Ȼ������
						ImVec2 size = tab->MainSize;
						tab->window->Viewport->Window->Pos = pos;
						tab->window->Viewport->Window->Size = size;
					}
				}
				else
				{
					//if (isdockwin)
					::SetWindowPos(tab->hwnd, NULL, pos.x, pos.y, tab->MainSize.x, tab->MainSize.y, SWP_NOZORDER | SWP_NOREDRAW);//��Сһ����λ
					//else
					//  ImGui::DockBuilderSetNodePos(tab->window->DockNode->ID, pos);//ֹͣ�϶������Ҫ��һ�Σ���Ȼ�������α����������ˣ�Pos���ԣ�����ⲻ����,�������裬�϶�ʱ��ť���е㶶��
				}
				tab->Pos = pos;
				tab->MovePos = pos;
			}
		}
		else
		{//�϶��ſ����,�ð�ť�Ŵ󡢻�ԭ��˫�����
			if (tab->drag || !issinglewin&&IsFullscreen(tab->hwnd) || issinglewin&&fullsize.x == tab->window->Size.x&&fullsize.y == tab->window->Size.y)
			{//��С��ſ����ʱ���SW_NORMAL����ԭdrag���
				if (tab->drag)
				{
					if (!isdockwin)
					{
						if (tab->Size.x > 0)
						{
							ImGui::DockBuilderSetNodePos(tab->window->DockNode->ID, tab->Pos);//ֹͣ�϶������Ҫ��һ�Σ���Ȼ�������α����������ˣ�Pos���ԣ�����ⲻ����
							ImGui::DockBuilderSetNodeSize(tab->window->DockNode->ID, tab->Size);
						}
					}
				}
				else
				{
					for (int k = 0; k < m_tabs.size(); k++)
					{//�޸�ͬһ�����µ����б�
						if (m_tabs[k].hwnd == tab->hwnd&&m_tabs[k].Size.x > 0)
						{
							ImGui::DockBuilderSetNodePos(m_tabs[k].window->DockNode->ID, m_tabs[k].Pos);
							ImGui::DockBuilderSetNodeSize(m_tabs[k].window->DockNode->ID, m_tabs[k].Size);
						}
					}
					::SetWindowPos(tab->hwnd, NULL, tab->MainPos.x, tab->MainPos.y, tab->MainSize.x, tab->MainSize.y, SWP_NOZORDER | SWP_NOACTIVATE);//��Сһ����λ������Ҫ�ֲ�
					ImGui::DockBuilderSetNodePos(tab->window->DockNode->ID, tab->Pos);
					ImGui::DockBuilderSetNodeSize(tab->window->DockNode->ID, tab->Size);
					if (isdockwin && !IsHostWin(tab->window))
						SetViewportSize(tab, tab->MainPos, tab->MainSize,true);//�ϳ�ȥ�Ķ�����С��Ҫ
					Sleep(150);
				}
			}
			else
			{//�Ŵ�
				bool ishostwin = IsHostWin(tab->window);
				if (isdockwin || ishostwin)
				{//��һ��С���Ѿ��ϲ���������ͼ,����Ӵ�ʱ�����ֻ��һ�����Ӵ����ڲ������ڣ�������Viewport���Ӵ��Ŵ��Ż������Լ��ľ������ʱֻ�޸�node���ɡ�host�����������SetWindowPos,�ڲ������ϳ�ȥ�Ĵ��ڶ����Բ���.
					if (fullsize.x > tab->window->Size.x || fullsize.y > tab->window->Size.y)
					{//��������SetWindowPos����ÿ�������,�ڲ������ϳ�ȥ�Ĵ��ڶ����Բ���SetWindowPos.
					//::SetWindowPos(tab->hwnd, NULL, 0, 0, fullsize.x, fullsize.y, SWP_NOZORDER | SWP_NOACTIVATE| SWP_FRAMECHANGED| SWP_NOREDRAW);//�����������Ͻ�
						tab->NewWinSize = ImRect(ImVec2(0, 0), ImVec2(fullsize.x, fullsize.y));
					}

					if (isdockwin)
					{
						float otherpos_x = tab->Pos.x - tab->MainPos.x;//ƫ����
						float otherpos_y = tab->Pos.y - tab->MainPos.y;

						//����ֻ���Ŵ������Ǹ����������İ�ԭֵ��������󻯺�����λ��
						float othersize_x = tab->MainSize.x - tab->Size.x;//�������ǰ�Ĵ�С
						float othersize_y = tab->MainSize.y - tab->Size.y;//ȫ���߼��Լ��ĸ߼���������ռ��
						pos = ImVec2(otherpos_x, otherpos_y);
						fullsize.x = fullsize.x - othersize_x;
						fullsize.y = fullsize.y - othersize_y;

						for (int k = 0; k < m_tabs.size(); k++)
						{//�޸�ͬһ�����µ����б�
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
							PostMessage(tab->hwnd, WM_SIZE, 0, (LPARAM)MAKELONG(fullsize.x, fullsize.y));//�ϳ���������Ҫ�޸Ļ�����С
					}
					ImGui::DockBuilderSetNodePos(tab->window->DockNode->ID, pos);
					ImGui::DockBuilderSetNodeSize(tab->window->DockNode->ID, fullsize);

				}
				else
				{
					ImGui::DockBuilderSetNodePos(tab->window->DockNode->ID, pos);
					ImGui::DockBuilderSetNodeSize(tab->window->DockNode->ID, fullsize);
					Sleep(150);//�Ŵ��Ӵ����ϳ�����ͣһ�µ�����ʱ�����ˣ�0��0����
				}
			}
			tab->drag = false;
		}
	}
	else
	{//SW_RESTORE���ָ�λ��
		if (tab->Size.x > 0 && tab->window->DockNode&&tab->formstatus != SW_NORMAL)
		{
			ImGui::DockBuilderSetNodePos(tab->window->DockNode->ID, tab->Pos);
			ImGui::DockBuilderSetNodeSize(tab->window->DockNode->ID, tab->Size);
			if (ImGui::IsMouseDragging(0))
			{//����λ��
				ImVec2 mousepos = ImGui::GetIO().MousePos;
				//�޸���λ��
				tab->Pos = { mousepos.x - tab->clickposition.x ,mousepos.y - tab->clickposition.y };
				ImGui::SetNextWindowPos(tab->Pos);
			}
			else
			{
				if (tab->formstatus == SW_RESTORE)
				{
					tab->clickposition = ImVec2(0, 0);
					if (tab->hwnd == ImGui::GetMainViewport()->PlatformHandle)//����ʱҪ��������С������ԭ���ˣ��ϳ�ȥ���Ӵ�����Ҫ
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
	m_contrlId = 0;//id�ظ�����
	static ImGuiViewport itemdata;//���������������ImGui����������
	ImGuiContext& g = *GImGui;
	itemdata.RendererUserData=&m_texture;//����ͼ��,����������ڱ�ǩʡ�ü���
	g.IO.UserData = &itemdata;
	ImGuiStyle& style = ImGui::GetStyle();
	//����һ���ײ�ƽ̨�����������ϣ�������tab�������ϵ������棬����tab�ϵ����ƽ̨�ϡ������Ͳ�������ѽ���
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();//ȡ�䴰��(ImGuiWindow*)GImGui->WindowsById.GetVoidPtr(viewport->id);
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));//������϶
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	ImGui::Begin(("DockSpaceForm" + to_string(0)).c_str(), nullptr, window_flags);
	ImGui::PopStyleVar(3);
	ImGuiID dockspace_id = ImGui::GetID("DockSpace-main");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	ImGui::End();

	ImGuiWindowClass window_class;//������ClassId�涨������Ŀ򣬲���ClassId������
	window_class.DockingAllowUnclassed = true;
	window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton;
	ImGui::SetNextWindowClass(&window_class);//�����٣���Ȼ�ϳ�ȥ�����ֹرհ�ť

	if (ImGui::IsMouseDragging(0))
	{//�϶�ʱ��ǩʱ�����ҳ�Ƿ���ˣ��������϶������������ֻ�б�ǩ�ž���
		ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspace_id);
		if (node->HostWindow != NULL&&node->Windows.Size == 0 && node->CountNodeWithWindows == 0)//���ڵ������ýڵ��û��window��
			AddTab(NULL);//��һ���հ�tab
		//TABͣ��λ����ɫ

		if (g.MovingWindow&&g.MovingWindow->DockNode)
		{//�ϳ���ǩ��ԭ������ǰ�Ĵ�С
			TabItemInfo * dragtab = GetTab(g.MovingWindow);//�ҳ���Ӧtab
			if (dragtab != nullptr)
			{
				if (g.MovingWindow->DockNode->Windows.Size > 1)
				{//��û����ϳ�������ͣ��ԭ���ڣ��ϳ���ͱ�ɵ�������.
					if (dragtab->DragSize.x > 0)
						ImGui::DockBuilderSetNodeSize(dragtab->window->DockNode->ID, dragtab->DragSize);//��ԭ������ǰ��С
				}
				else
				{//�ϵ����򱣴��С�������ϳ����п��õĴ�С�ˡ��ϳ��ǻ��ظ����棬��Ӱ��ʲô
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
	{//����ǰ�棬��Ϊִ�е������Begin���ɹ����У��׳�����
		AddTab(m_addwindow);//�������,����ӽ�ȥ��Window��û�����ɣ��㴰����Ҫ��һ
		m_tabs[m_tabs.size() - 1].window = m_addwindow;
		addtabwidth = GetTabWidth(m_addwindow, addtabcount);//������TAB�Ŀ��Ҫ�����������ΪWindow��û�����ɣ�m_addwindowҲ���ܷź���
		if (m_addwindow != NULL)
			node = m_addwindow->DockNode;
		m_addwindow = NULL;//ֻ�ܷ����⣬���ܷź��棬m_addwindow��ֵʵ������ִ��Begin�Ĺ�����
	}

	//�������SetWindowPos�����
	if (m_redrawtab != NULL&&m_redrawtab->setwinstep>0)//��Ҫ�ȵ������Ѿ��������ߴ续�����
	{//�ڶ��������úò��Ƶ���ȷλ�á���һ����viewport���󣬻���һ���Ժ�window�Ż���
		m_redrawtab->setwinstep++;
		if (m_redrawtab->setwinstep == 3)
		{//���ǷŴ󴰿ڣ��ڻ��ƹ����У���ͼ���ڲ�û�зŴ󣩣��ڶ��ֻ�ͼ���ڷŴ󲢻��ƣ��������ѻ�þͿ����ƶ���
			::SetWindowPos(m_redrawtab->hwnd, NULL, m_redrawtab->NewWinSize.Min.x, m_redrawtab->NewWinSize.Min.y, m_redrawtab->NewWinSize.Max.x - m_redrawtab->NewWinSize.Min.x, m_redrawtab->NewWinSize.Max.y - m_redrawtab->NewWinSize.Min.y, SWP_NOZORDER | SWP_NOACTIVATE);
			//::MoveWindow(m_redrawtab->hwnd, m_redrawtab->NewWinSize.Min.x, m_redrawtab->NewWinSize.Min.y, m_redrawtab->window->Viewport->Size.x, m_redrawtab->window->Viewport->Size.y, TRUE);
			if (IsSubwin(m_redrawtab->window->DockNode) && !IsHostWin(m_redrawtab->window))
			{//ֻ�ϳ�ȥ�Ķ�����Ҫ�޸Ļ����С������ؼ�ֻ��ԭ��λ�òŸ�Ӧ�õ�
				SetViewportSize(m_redrawtab, m_redrawtab->NewWinSize.Min, m_redrawtab->NewWinSize.Max,true);
			}
			m_redrawtab->setwinstep = 0;
			m_redrawtab->NewWinSize.Max.x = m_redrawtab->NewWinSize.Min.x;//�������ٻ�����
			m_redrawtab = NULL;
		}
	}

	int tabcount = 0;//ȡ��TAB����
	float tabwidth = 0;//���
	TabItemInfo* closedtab = nullptr;//�رյ�tab�ȼ���,��ѭ����ɾ��
	ImGuiDockNode* calutab = nullptr;//���µ�ǰ���ĸ����ڿ��
	for (int k = 0; k < m_tabs.size(); k++)//���Ӵ���ȫ��������ʱ���Զ��ϲ���������ڶ�����һ�����ھ��
	{//���ù����ϵ��ĸ����ڣ���ȫ��Imgui������Ȼ����������.Ҫ��tab����ƽ̨��ͨ��dc->Nodes.Data.Size����,����ֻ�ܰ�tab������.��Tab��ʽ�޸�TabItemEx��BeginTabBarEx����imgui_widgets��8098�У��޸�tab_bar->BarRect
		if (m_tabs[k].window != NULL)
		{//���Ѿ����õ���ߴ��λ�ã���һ�β���
			if (m_tabs[k].window->DockNode != calutab)
			{//���㵱ǰƽ̨ƽ��tab��ȣ���ƽ̨���¼���
				if (addtabwidth > 0 && node == m_tabs[k].window->DockNode)
				{//�����ʱWindow��û������Ҫ�����
					tabcount = addtabcount;
					tabwidth = addtabwidth;
				}
				else
					tabwidth = GetTabWidth(m_tabs[k].window, tabcount);

				calutab = m_tabs[k].window->DockNode;

				if (m_tabs[k].window->DockNode&&m_tabs[k].window->DockNode->TabBar)
				{//�趨tab��ȣ�һ��ƽ̨һ�����ꡣSetNextItemWidthֻ�ܱ���һ��ֵ���ж��ƽ̨�����ˡ�
					m_tabs[k].window->DockNode->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton;//�ϳ���ҲҪ��
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
				//�����ڵ�����С��λ�á����������С��.imgui�����϶�����Ҫ���⴦��
				if (m_tabs[k].window->DockNode &&m_tabs[k].clickitem && (m_tabs[k].formstatus != SW_NORMAL || m_tabs[k].drag || ImGui::IsMouseDragging(0)))//����л��������ϣ�ֹͣ��Ҫִ��һ�ζ�λ��������ʼ��
				{//��󻯣��Ӵ�û���Լ��Ķ��������ͨ���޸Ľڵ��Сʵ�����.���Ҫ��������ɲο�ImWindow��Ҫ�л�Context ,���׳�����
					bool ismoveMergedwin = m_tabs[k].formstatus == SW_NORMAL && !m_tabs[k].drag&&IsMerged(m_tabs[k].window);//�϶��������Ĵ��ڲ���Ҫ�ı�״̬
					if (!ismoveMergedwin&&!(m_tabs[k].ismerged&&ImGui::IsMouseDragging(0)))//����󻯵ı���������Ҳ����С
					{
						ImVec2 newpos = m_tabs[k].window->Viewport->Pos;
						Adjustwindow(&m_tabs[k], viewport->WorkSize, newpos);//���ֻ��������С��λ�ã��϶�ʱ������λ��
						if (!ImGui::IsMouseDragging(0))
						{//�����ϲ���
							if (m_tabs[k].drag)//���϶���ֻ��һ��
								SetStatus(m_tabs[k].window, m_tabs[k].formstatus, false);//�����޸���ͬһ�����µ��Ӵ�ȫ���޸�,��Ȼ�еľ��ϲ���
							else
							{
								SetStatus(m_tabs[k].window, (m_tabs[k].formstatus == SW_MAXIMIZE ? SW_MAXIMIZE : SW_NORMAL), true);//�����޸ĺ��ʵ��״̬
								SetStatus(m_tabs[k].window, SW_NORMAL, false);//��ԭ��ֵ����
							}
							m_tabs[k].clickitem = false;
						}
					}
				}
			}

			if (m_redrawtab == NULL && ((m_tabs[k].NewWinSize.Max.x - m_tabs[k].NewWinSize.Min.x) > 0 && m_tabs[k].window->Size.x != (m_tabs[k].NewWinSize.Max.x - m_tabs[k].NewWinSize.Min.x)))
			{//��һ���������ڴ�С��ֻ���ڷŴ󣬻ָ�һ����λ��,����λ�ã������,���괰�ں����λ������. SetWindowPos����������Ļ�⣬������ֻ�ܻ�����Ļ�ڣ����Ի��ǻ��е�����
				m_redrawtab = &m_tabs[k];
				::SetWindowPos(m_redrawtab->hwnd, NULL, m_redrawtab->NewWinSize.Min.x, m_redrawtab->NewWinSize.Min.y, m_redrawtab->NewWinSize.Max.x - m_redrawtab->NewWinSize.Min.x, m_redrawtab->NewWinSize.Max.y - m_redrawtab->NewWinSize.Min.y, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
				//::MoveWindow(m_redrawtab->hwnd, m_redrawtab->window->Pos.x, m_redrawtab->window->Pos.y, m_redrawtab->NewWinSize.Max.x - m_redrawtab->NewWinSize.Min.x, m_redrawtab->NewWinSize.Max.y - m_redrawtab->NewWinSize.Min.y, TRUE);
				m_redrawtab->setwinstep = 1;
				if (!IsHostWin(m_redrawtab->window))
					Sleep(200);//������Ҫͣһ��,��Ȼ�Ʋ������
			}
		}

		//�Ƶ�������
		if (node != NULL && (k + 1 == m_tabs.size()))//������
		{//���������ڼӵ�ƽ̨���ڵ��飬���ĸ�ƽ̨�����Ӿͼӵ��ĸ�ƽ̨�顣ֻ��һ��.Ҫ�������Ȼ����
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

		//����
		if (((ImGuiWindow*)itemdata.PlatformUserData) == m_tabs[k].window || m_tabs[k].window&&m_tabs[k].window->DockNode&&m_tabs[k].window->DockNode->IsFocused)
		{//��ǩ��Ӧ���ʱҪ��֤������tab��ǩ��ɫһ��
			if (((ImGuiWindow*)itemdata.PlatformUserData) == m_tabs[k].window)
				ImGui::PushStyleColor(ImGuiCol_WindowBg, style.Colors[ImGuiCol_TabHovered]);
			else
				ImGui::PushStyleColor(ImGuiCol_WindowBg, style.Colors[ImGuiCol_TabActive]);
		}
		else
			ImGui::PushStyleColor(ImGuiCol_WindowBg, style.Colors[ImGuiCol_TabUnfocusedActive]);

		bool tabopen = m_tabs[k].open || !IsWindowVisible(m_tabs[k].hwnd);//���ɼ���Ҳ�ص�������û������γ�һ�����εĴ��ڣ����봰��ȴ��ʧ
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
		{//���ǰҳ,����ӵ������
			ImGui::SetWindowFocus();
			m_tabs[k].window->DockNode->TabBar->SelectedTabId = m_tabs[k].window->DockNode->TabBar->NextSelectedTabId = m_tabs[k].window->DockNode->SelectedTabId;
			Sleep(150);
		}
		//���ؼ�
		if (m_redrawtab == NULL &&m_tabs[k].window->DockNode)
		{//��ÿ�����ǵ�����ҳ������Ҫ���Լ��ı�����
			ImGuiWindow* window = g.CurrentWindow;
			const ImRect titleBarRect = window->TitleBarRect();
			ImGui::PushClipRect(titleBarRect.Min, titleBarRect.Max, false);
			//��������
			DrawTittleBar(&m_tabs[k], ImRect(window->Pos.x, window->Pos.y, window->Pos.x + window->Size.x, window->Pos.y + window->Size.y), TITLE_HEIGHT);
			//ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 20.0f);//Բ�ΰ�ť
			ImGui::PopClipRect();

			float framePadding = style.FramePadding.y;
			style.FramePadding.y = 5;//��tab�޸ĳ���10�������С����Ȼ�����̫��

			DrawToolBar(&m_tabs[k]);

			//�Ӳ���
			ImGui::Separator();

			style.FramePadding.y = framePadding;
		}
		ImGui::End();
		ImGui::PopStyleColor(1);//�������ɫ

	}

	if (closedtab != nullptr)
	{
		if (m_tabs.size() == 1)
		{//��һ���ռ�¼,�ϳ�ȥ�Ĳ����������һ����ҳֱ�ӹر�ƽ̨
			AddTab(closedtab->window);
		}
		RemoveTab(closedtab->window);
	}
	itemdata.PlatformUserData=NULL;//������
	itemdata.RendererUserData = NULL;//��ͼ����
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
	ImGuiWindow* window = g.CurrentWindow;//����õ��Ĳ���Begin���´��ڴ�С����SetNextWindowSizeӰ��.ֻ�������Ĵ�С
	float framePadding = oStyle.FramePadding.y;//����y����tab��
	ImGui::SetCursorPosX(1);
	ImVec2 pos = ImGui::GetWindowPos();//����λ��
	int windorwleft = pos.x;//���´��ڳ�ʼλ�á�pos�������޸�

	ImVec2 size(oRect.Max.x - oRect.Min.x, oRect.Min.y + ftitlehight);//���ڴ�С
	int leWidth = size.x - tab->tabwidth - 2 * oStyle.WindowPadding.x;//����ʣ�µĿռ䣬ȷ���Ƿ���Ҫ��ť

	float icowidth = 0;
	bool issubwin = tab->window->DockNode != NULL&&IsSubwin(tab->window->DockNode) && !(tab->window->Pos.y - 10 < tab->window->Viewport->Pos.y&&tab->window->Pos.x + tab->window->Size.x + 10 > tab->window->Viewport->Pos.x + tab->window->Viewport->Size.x);//�Ƿ����Ӵ����Ӵ�����ͼ�ꡢ�����С��ť.����ʱֻ����������ʾ��󻯡���С����ť

	if (tab->window != NULL&&tab->window->DockNode != NULL)
	{
		if (tab->window->DockNode->TabBar != NULL)
			icowidth = tab->window->DockNode->TabBar->BarRect.Min.x - tab->window->Pos.x;//��tab�������Ŀ��
	}

	if (!issubwin&&icowidth > 16)
	{
		int id = (intptr_t)window->ID + (m_contrlId++);
		DrawImage(id, ImVec2(pos.x + (icowidth - 16) / 2, pos.y - 2 + (ftitlehight - 16) / 2), "AppIco", ImVec2(16, 16));
	}

	if (icowidth == 0)
	{//������
		ImGui::SameLine();
		ImGui::SetCursorPosX(35);
		ImGui::Text(tab->title.c_str());
		ImGui::SameLine();//ע�⣬û����SameLine()����ʾһ���ؼ���DC.CursorPos�ͱ����һ����
	}
	//���������ǩ��ť
	if ((oRect.Max.x - oRect.Min.x) - tab->tabwidth > 5 * m_fBtnHight)
	{
		ImVec2 oPos = ImGui::GetWindowPos();
		oPos.x += tab->tabwidth + (issubwin ? 0 : TITLE_HEIGHT) + 20;//��Ҫȥ��ͼ��
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

	//�ұ߰�ť
	const float close_button_sz = g.FontSize;
	pos = ImGui::GetWindowPos();
	float btnwidth = (3.f * m_fBtnHight + 4 * framePadding);//��ť��

	//��С����ť
	pos.y += framePadding / 2;
	pos.x = windorwleft + size.x - btnwidth;//Ҫ�þ�������
	if (!issubwin&&leWidth > 4 * m_fBtnHight)
	{
		ImRect b_MinBtn(pos, pos + ImVec2(m_fBtnHight, m_fBtnHight));
		const ImGuiID close_button_idMin = window->GetID((void*)((intptr_t)window->ID + (m_contrlId++)));
		if (AddButton(tab->hwnd, close_button_idMin, ImVec2(b_MinBtn.Max.x - framePadding - close_button_sz, b_MinBtn.Min.y), ImGuiMinBtn, true))
		{//���Ӵ���ȫ��������ʱ���Զ��ϲ���������ڶ����������ھ���������Ӵ������þ������
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

	//��󻯰�ť
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
	//�رհ�ť
	if (leWidth > 2 * m_fBtnHight)
	{
		pos.x += m_fBtnHight + framePadding + 5;
		ImRect b_CloseBtn(pos, pos + ImVec2(m_fBtnHight, m_fBtnHight));
		const ImGuiID close_button_idClose = window->GetID((void*)((intptr_t)window->ID + (m_contrlId++)));
		if (AddButton(tab->hwnd, close_button_idClose, ImVec2(b_CloseBtn.Max.x - framePadding - close_button_sz, b_CloseBtn.Min.y), ImGuiCloseBtn, true))
		{
			TabItemInfo* firsttab = nullptr;//�رպ�������Ĵ���
			bool ismaintitlebar = (pos.x + 25 > tab->window->Viewport->Pos.x + tab->window->Viewport->Size.x) && (pos.y < tab->window->Viewport->Pos.y + TITLE_HEIGHT);//�ж��ǲ��ǵ�����������Ͻǰ�ť
			for (int k = 0; k < m_tabs.size(); k++)
			{//��ǩ�Ϲص�������������ȫ��
				if (ismaintitlebar&&m_tabs[k].hwnd == tab->hwnd || m_tabs[k].window->DockNode == tab->window->DockNode)//ͬһ�����ϵ�
					m_tabs[k].open = false;//���false�Ͳ�����ʾ�ˣ�tab���괰�ھ���ʧ��
				else
					if (firsttab == nullptr)
						firsttab = &m_tabs[k];
			}

			if (tab->hwnd == ImGui::GetMainViewport()->PlatformHandle)
			{//������ʱ�Ҹ��滻���ڣ��������ϵ�tab�Ƶ��滻������
				if (firsttab == nullptr)
					::PostQuitMessage(0);
				else//�������ڡ����ܹ�host���ڣ�������ʼ��dx11��
				{//û�ҵ�ֱ�ӹ�host�ķ�������������ʾ����Ķ��������滻�ķ���
					::SetWindowPos(m_pHandle, NULL, firsttab->window->Viewport->Pos.x, firsttab->window->Viewport->Pos.y, firsttab->window->Viewport->Size.x, firsttab->window->Viewport->Size.y, SWP_NOZORDER | SWP_NOACTIVATE);//��SetNextWindowPos��������������
					HWND closehnd = firsttab->hwnd;
					for (int k = 0; k < m_tabs.size(); k++)
					{//�滻����
						if (m_tabs[k].hwnd == firsttab->hwnd)
							m_tabs[k].window->Viewport->PlatformHandle = m_tabs[k].window->Viewport->PlatformHandleRaw = (void*)m_pHandle;
					}
				}
			}
			else//�رյ�ǰ���󣬰ѹ��ת�Ƶ��»���ڣ���Ȼ��Ҫ���һ��
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

	ImVec2 btnsz = pos;//posΪ���Ͻ�����
	ImVec2 label_size(0, 0);
	if (label != NULL)
		label_size = ImGui::CalcTextSize(label, NULL, true);

	int circleR = ImGui::GetFrameHeight();//��ֱ����
	if (imageSize.y > circleR)
		circleR = imageSize.y;
	btnsz.x += label_size.x + circleR;
	if (label != NULL&&imageSize.x > 0)
	{//ͼ�İ�ť�Ӽ������ �� ��
		btnsz.x += circleR;
		btnsz.y += (imageSize.x > 12 ? 2.0f : 1.0f);//Բʱ�м���Ȧ
	}
	btnsz.y += circleR;

	const ImRect btn(ImVec2(pos.x, pos.y - ((label != NULL&&imageSize.x > 0) ? (imageSize.x > 12 ? 2.0f : 1.0f) : 0)), btnsz);
	const ImGuiID button_id = window->GetID((void*)((intptr_t)window->ID + (id++)));
	ImU32 colText = ImGui::GetColorU32(ImGuiCol_Text);
	if (!abled)
	{//�Ұ�ť
		colText = ImGui::GetColorU32(ImGuiCol_TextDisabled);
	}

	bool is_clipped = !ImGui::ItemAdd(btn, button_id);
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(btn, button_id, &hovered, &held, abled ? ImGuiButtonFlags_PressedOnClick : ImGuiItemFlags_Disabled);
	if (is_clipped)
		return pressed;

	int addpress = (hovered && (held || GetAsyncKeyState(VK_LBUTTON) & 0x8000)) ? 1 : 0;//���¶���

	ImU32 col = ImGui::GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered);
	ImVec2 center = btn.GetCenter();

	ImRect image_bb(ImVec2(center.x - imageSize.x / 2 + addpress, center.y - imageSize.y / 2 + addpress), ImVec2(center.x + imageSize.x / 2 + addpress, center.y + imageSize.y / 2 + addpress));
	if (label != NULL)//ͼ�Ļ�ϣ���ʼλ�Ӱ�Բ��ʼ
		image_bb = ImRect(ImVec2(pos.x + circleR / 2 + addpress, pos.y + addpress), ImVec2(pos.x + circleR / 2 + imageSize.x + addpress, pos.y + circleR + addpress));
	//����λ��
	ImRect text_bb = ImRect(ImVec2(center.x - label_size.x / 2 + addpress, center.y - circleR / 2 + addpress), ImVec2(center.x + label_size.x / 2 + addpress, center.y + circleR / 2 + addpress));
	if (imageSize.x > 0)//ͼ�Ļ�ϣ���ʼλȥ����Բ��ͼƬ��
		text_bb = ImRect(ImVec2(pos.x + circleR / 2 + imageSize.x + 3 + addpress, pos.y + addpress), ImVec2(pos.x + circleR / 2 + imageSize.x + 3 + label_size.x + addpress, pos.y + circleR + addpress));

	if (hovered)
	{
		if (label_size.x > imageSize.x)
		{//������Բװ����ʱ��������Բ
			window->DrawList->AddRectFilled(ImVec2(btn.Min.x + addpress, btn.Min.y + addpress), ImVec2(btn.Max.x + addpress, btn.Max.y + addpress), col, ((imageSize.x > 12 ? 2.0f : 1.0f) + circleR / 2));
		}
		else
			window->DrawList->AddCircleFilled(ImVec2(center.x + addpress, center.y + addpress), ImMax(2.0f, g.FontSize * 0.5f + (imageSize.x > 12 ? 5.0f : 1.0f)), col, 12);
	}

	if (!hovered&&label != NULL)
	{//�ǻͼ�İ�ť��ʾ��ӡ��
		col = ImGui::GetColorU32(ImGuiCol_Button);
		window->DrawList->AddRectFilled(ImVec2(btn.Min.x + addpress, btn.Min.y + addpress), ImVec2(btn.Max.x + addpress, btn.Max.y + addpress), col, ((imageSize.x > 12 ? 2.0f : 1.0f) + circleR / 2));
	}
	if (texId != NULL)
		window->DrawList->AddImage(texId, image_bb.Min, image_bb.Max, ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.00f)));

	if (label != NULL)
	{//��ʾ�ֺ��ֵ���ɫ
		const ImGuiStyle& style = g.Style;
		ImVec4 co = style.Colors[ImGuiCol_Text];
		ImGui::PushStyleColor(ImGuiCol_Text, co);
		label_size.x -= 2;
		label_size.y += 2;
		//p0.x += imageSize.x+15;//ȥ��ͼ��λ��
		ImGui::RenderTextClipped(text_bb.Min, text_bb.Max, label, NULL, &label_size, style.ButtonTextAlign, &text_bb);
		ImGui::PopStyleColor(1);
	}

	return held;
}
void MainWindow::ShowTooltip(const char* pText, bool needdelay)
{//��ʾ����ʾ��ʱ������ʱ�Ͳ���ʱ����
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
					{//����������ͼ�������ԣ������ڴ�������OK
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
				m_tiptime = GetTickCount();//��ʱ��ʼ
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
	int fontpadding =8;//���������Ҫ��������
	const ImGuiStyle& oStyle = ImGui::GetStyle();
	ImGuiContext& g = *GImGui;

	ImGuiWindow* window = g.CurrentWindow;
	ImVec2 pos = window->DC.CursorPos;
	pos.x = pos.x - 22 + fontpadding;//����λ��
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
	ImGui::SetNextItemWidth(winwidth - 92 - 20);//����ǰ��Ŀ�Ⱦ��������Ŀ�
	ImGui::Spacing();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() +65 + 4 * fontpadding);//�ո�
	static char url[MAX_PATH] = "";
	if (ImGui::InputTextWithHint("##urlTxt", "", url, MAX_PATH, ImGuiInputTextFlags_EnterReturnsTrue) == true)
		winwidth = winwidth;
}