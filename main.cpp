// Dear ImGui: standalone example application for DirectX 11
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

#include "Imwin.h"
#include "Loadfun.h"

static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
// Main code
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	int iWidth =1280, iHeight = 720;
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);//³ýÈ¥ÈÎÎñÀ¸
	int scrWidth = rect.right - rect.left;
	int scrHeight = rect.bottom - rect.top;
	RECT wrect;
	wrect.left = (scrWidth - iWidth) / 2;
	wrect.top = (scrHeight - iHeight) / 2;
	wrect.right = wrect.left + iWidth;
	wrect.bottom = wrect.top + iHeight;

	MainWindow m_window("Robot", wrect, WinStyle::aero_borderless);
	HWND hwnd =(HWND)m_window.GetHandle();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;// Enable Multi-Viewport
	ImFontConfig font_cfg = ImFontConfig();
	font_cfg.SizePixels = 2 * 8;
	ImFont* font = io.Fonts->AddFontDefault(&font_cfg);
	io.Fonts->Build();

	ConfigInfo* m_config = m_window.Config();
	LoadSetting(m_config);
	LoadColor(2);
	//loadthread(&m_window,m_config);

	MONITORINFO MonitorInfo;
	MonitorInfo.cbSize = sizeof(MonitorInfo);
	::GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &MonitorInfo);

	io.IniFilename = NULL;
	io.LogFilename = NULL;
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.ConfigDockingTransparentPayload = true;
	io.ConfigDockingAlwaysTabBar = true;

	// Our state
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	bool show_demo_window = true;

#ifdef _DEBUG
	bool show_app_debug_log = true;
#else
	bool show_app_debug_log = false;
#endif

	// Main loop
	bool done = false;
	ImGuiContext& g = *GImGui;
	while (!done)
	{
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		float framePadding = (TITLE_HEIGHT - g.IO.Fonts[0].Fonts[0]->FontSize)/2;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0f, framePadding));
		ImGui::NewFrame();
	
		m_window.DrawPage(hwnd, wrect,u8"");
		ImGui::PopStyleVar(2);

		// Rendering
		ImGui::Render();
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		m_window.g_pd3dDeviceContext->OMSetRenderTargets(1, &m_window.g_mainRenderTargetView, NULL);
		m_window.g_pd3dDeviceContext->ClearRenderTargetView(m_window.g_mainRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		m_window.g_pSwapChain->Present(1, 0); // Present with vsync
	}
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	//CleanupDeviceD3D();
	m_window.Cleanup();

	return 0;
}