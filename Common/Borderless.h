#pragma once
#ifndef __Borderless_H__
#define __Borderless_H__
#define WM_TO_TRAY (WM_USER + 2133)
#ifndef GET_X_LPARAM
#define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))
#endif
#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

static inline ImVec2 operator*(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
static inline ImVec2 operator/(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x / rhs, lhs.y / rhs); }
static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
static inline ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
static inline ImVec2 operator/(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
vector<TabItemInfo> m_tabs;
static const MARGINS Imguishadow_state[2]{ { 0,0,0,0 },{ 0,0,0,1 } };
HTHUMBNAIL m_Imguithumbnail = NULL;

bool borderless = true; // is the window currently borderless
bool borderless_resize = true; // should the window allow resizing by dragging the borders while borderless
bool borderless_drag = true; // should the window allow moving my dragging the client area
bool borderless_shadow = true; // should the window display a native aero shadow while borderless


bool maximized(HWND hwnd) {
	WINDOWPLACEMENT placement;
	if (!::GetWindowPlacement(hwnd, &placement)) {
		return false;
	}
	return placement.showCmd == SW_MAXIMIZE;
}
void adjust_maximized_client_rect(HWND window, RECT& rect) {
	if (!maximized(window)) {
		return;
	}

	auto monitor = ::MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
	if (!monitor) {
		return;
	}

	MONITORINFO monitor_info{};
	monitor_info.cbSize = sizeof(monitor_info);
	if (!::GetMonitorInfoW(monitor, &monitor_info)) {
		return;
	}

	// when maximized, make the client area fill just the monitor (without task bar) rect,
	// not the whole window rect which extends beyond the monitor.
	rect = monitor_info.rcWork;
}

std::system_error last_error(const std::string& message) {
	return std::system_error(
		std::error_code(::GetLastError(), std::system_category()),
		message
	);
}
const char* window_class(WNDPROC wndproc) {
	static const char* window_class_name = [&] {
		WNDCLASSEX wcx{};
		wcx.cbSize = sizeof(wcx);
		wcx.style = CS_HREDRAW | CS_DBLCLKS;//响应双击
		wcx.hInstance = nullptr;
		wcx.lpfnWndProc = wndproc;
		wcx.lpszClassName = "RobotWindowClass";
		//wcx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);//快速拖有白底
		wcx.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
		wcx.hIcon = LoadIcon(GetModuleHandle(NULL), (LPCTSTR)IDI_ICON1);//应用程序的图标
		wcx.hIconSm = LoadIcon(GetModuleHandle(NULL), (LPCTSTR)IDI_ICON1);
		const ATOM result = ::RegisterClassEx(&wcx);
		if (!result) {
			throw last_error("failed to register window class");
		}
		return wcx.lpszClassName;
	}();
	return window_class_name;
}

bool composition_enabled() {
	BOOL composition_enabled = FALSE;
	bool success = ::DwmIsCompositionEnabled(&composition_enabled) == S_OK;
	return composition_enabled && success;
}

WinStyle select_borderless_style() {
	return composition_enabled() ? WinStyle::aero_borderless : WinStyle::basic_borderless;
}

void set_shadow(HWND handle, bool enabled) {
	if (composition_enabled()) {
		static const MARGINS shadow_state[2]{ { 0,0,0,0 },{ 1,1,1,1 } };
		::DwmExtendFrameIntoClientArea(handle, &shadow_state[enabled]);
	}
}

void set_borderless(HWND hwnd, bool enabled) {
	WinStyle new_style = (enabled) ? select_borderless_style() : WinStyle::windowed;
	WinStyle old_style = static_cast<WinStyle>(::GetWindowLongPtrW(hwnd, GWL_STYLE));

	if (new_style != old_style) {
		borderless = enabled;

		::SetWindowLongPtrW(hwnd, GWL_STYLE, static_cast<LONG>(new_style));

		// when switching between borderless and windowed, restore appropriate shadow state
		set_shadow(hwnd, borderless_shadow && (new_style != WinStyle::windowed));

		// redraw frame
		::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
		::ShowWindow(hwnd, SW_SHOW);
	}
}

void set_borderless_shadow(HWND hwnd, bool enabled) {
	if (borderless) {
		borderless_shadow = enabled;
		set_shadow(hwnd, enabled);
	}
}
LRESULT hit_test(HWND hwnd, POINT cursor) {
	const POINT border{
		::GetSystemMetrics(SM_CXFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER),
		::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER)
	};
	RECT window;
	if (!::GetWindowRect(hwnd, &window)) {
		return HTNOWHERE;
	}
	const auto drag = borderless_drag ? HTCAPTION : HTCLIENT;
	enum region_mask {
		client = 0b0000,
		left = 0b0001,
		right = 0b0010,
		top = 0b0100,
		bottom = 0b1000,
	};
	const auto result =
		left    * (cursor.x <  (window.left + border.x)) |
		right   * (cursor.x >= (window.right - border.x)) |
		top     * (cursor.y <  (window.top + border.y)) |
		bottom  * (cursor.y >= (window.bottom - border.y));

	switch (result) {
	case left: return borderless_resize ? HTLEFT : drag;
	case right: return borderless_resize ? HTRIGHT : drag;
	case top: return borderless_resize ? HTTOP : drag;
	case bottom: return borderless_resize ? HTBOTTOM : drag;
	case top | left: return borderless_resize ? HTTOPLEFT : drag;
	case top | right: return borderless_resize ? HTTOPRIGHT : drag;
	case bottom | left: return borderless_resize ? HTBOTTOMLEFT : drag;
	case bottom | right: return borderless_resize ? HTBOTTOMRIGHT : drag;
	case client: 
	{
		ImVec2 cursorPos = ImVec2(cursor.x, cursor.y);
		if (cursorPos.x > window.right - 90 && cursorPos.x < window.right&&cursorPos.y>window.top&&cursorPos.y<window.top + TITLE_HEIGHT)
			return HTCLIENT;
		return drag;
	}
	default: return HTNOWHERE;
	}
}
static void ToTray(HWND hWnd)
{
	NOTIFYICONDATA nid;
	nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = IDI_ICON1;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_TO_TRAY;
	nid.hIcon = LoadIcon(GetModuleHandle(NULL), (LPCTSTR)IDI_ICON1);
	strcpy(nid.szTip, TRAYAPPNAME);
								 
	Shell_NotifyIcon(NIM_ADD, &nid);
}
static void DeleteTray(HWND hWnd)
{
	NOTIFYICONDATA nid;
	nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = IDI_ICON1;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_TO_TRAY;
	nid.hIcon = LoadIcon(GetModuleHandle(NULL), (LPCTSTR)IDI_ICON1);
	strcpy(nid.szTip, TRAYAPPNAME);
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

#endif
