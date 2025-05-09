
#ifndef __IM_CONFIG_H__
#define __IM_CONFIG_H__

#include <stdlib.h>
#include <vector>
#ifndef NULL
#define NULL 0
#endif // NULL

#include <string>

#define ImwVerify(bCondition) { if ((bCondition) == false) { IM_ASSERT(false); } }

#define ImwSafeDelete(pObj) { if (NULL != pObj) { delete pObj; pObj = NULL; } }
#define ImwSafeRelease(pObj) { if (NULL != pObj) { pObj->Release(); pObj = NULL; } }
#define ImwIsSafe(pObj) if (NULL != pObj) pObj

#define ImwMalloc(iSize) malloc(iSize)
#define ImwFree(pObj) free(pObj)
#define ImwSafeFree(pObj) { if (pObj != NULL) { free(pObj); pObj = NULL; } }

// Define IMW_INHERITED_BY_IMWWINDOW when you want ImwWindow inherit from one of your class
//#define IMW_INHERITED_BY_IMWWINDOW MyInheritedClass

// Define IMW_CUSTOM_DECLARE_IMWWINDOW and/or IMW_CUSTOM_IMPLEMENT_IMWWINDOW for custom declaration and implementation for ImwWindow
//#define IMW_CUSTOM_BEFORE_DECLARE_IMWWINDOW 
//#define IMW_CUSTOM_DECLARE_IMWWINDOW
//#define IMW_CUSTOM_IMPLEMENT_IMWWINDOW

// Define IMW_BEFORE_WINDOW_PAINT for calling function of instancing object before painting window
//#define IMW_BEFORE_WINDOW_PAINT(pName) printf("Painting %s\n", pName);

//////////////////////////////////////////////////////////////////////////
// Include here imgui.h
//////////////////////////////////////////////////////////////////////////
#include "imgui.h"
#include "imgui_internal.h"
#include <unordered_map>
#include "config.h"

enum ButtonType
{
	ImGuiAddBtn = 0,
	ImGuiMinBtn = 1,
	ImGuiMaxBtn = 2,
	ImGuiCloseBtn = 3,
	ImGuiGoBackBtn = 4,
	ImGuiForwardBtn = 5,
	ImGuiRefreshBtn = 6,
	ImGuiStopLoadingBtn = 7,
	ImGuiFavoritesBtn = 8,
	ImGuiTabCloseBtn = 9,
	ImGuiMultiScreen = 10,
	ImGuiSetting = 11,
	ImGuiProxy = 12,
	ImGuiOther = 13,
	ImGuiPlay = 14,
	ImGuiPause = 15,
	ImGuiStop = 16,
	ImGuiPrePage = 17,
	ImGuiNextPage = 18,
	ImGuiFirst = 19,
	ImGuiLast = 20,
	ImGuiExport = 21,
	ImGuiIsMaxBtn = 22
};
enum ETabColorMode
{
	E_TABCOLORMODE_TITLE,
	E_TABCOLORMODE_BACKGROUND,
	E_TABCOLORMODE_CUSTOM
};

static char* searchitems[] = { "",

};
#endif // __IM_CONFIG_H__