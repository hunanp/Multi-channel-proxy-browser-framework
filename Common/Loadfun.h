#ifndef __HTTP_loadfun_H__  
#define __HTTP_loadfun_H__ 
#include <windows.h>
#include <string.h>
#include <unordered_map>
#include <atlstr.h>
#include "Namestr.h"
#include "Imgstring.h"
#include "strCoding.h"
using namespace std;
static void LoadColor(int colormode)
{
	ImGuiStyle& style = ImGui::GetStyle();
	//ImwWindowManager::Config& oConfig = ImwWindowManager::GetInstance()->GetConfig();//具体值在Properties初始化是赋给m_oConfig
	style.WindowPadding = ImVec2(1, 1);//放在这统一设，不要改，已作判断用
	switch (colormode)
	{
	case 0://White
	{//White
	 //	oConfig.m_eTabColorMode = ImwWindowManager::E_TABCOLORMODE_BACKGROUND;
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.9f, 0.9f, 0.9f, 1.00f);
		style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);

		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
		//style.Colors[ImGuiCol_PopupBg] = ImVec4(0.89f, 0.98f, 1.00f, 0.99f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		//style.Colors[ImGuiCol_FrameBg] = ImVec4(0.62f, 0.70f, 0.72f, 0.56f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.30f);//输入框颜色
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.95f, 0.33f, 0.14f, 0.47f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.97f, 0.31f, 0.13f, 0.81f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.42f, 0.75f, 1.00f, 0.53f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.65f, 0.80f, 0.20f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.32f, 0.32f, 0.63f, 0.87f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.74f, 0.74f, 0.94f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.40f, 0.62f, 0.80f, 0.15f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.39f, 0.64f, 0.80f, 0.30f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.67f, 0.80f, 0.59f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.48f, 0.53f, 0.67f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.48f, 0.47f, 0.47f, 0.71f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.31f, 0.47f, 0.99f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.65f, 0.78f, 0.84f, 0.80f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.75f, 0.88f, 0.94f, 0.80f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.55f, 0.68f, 0.74f, 0.80f);
		style.Colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.60f, 0.60f, 0.80f, 0.30f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.99f, 0.54f, 0.43f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}
	break;
	case 1://Default
	{
		ImGui::StyleColorsDark();
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.095f, 0.095f, 0.095f, 1.f);//ImGuiCol_WindowBg窗口底色
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.204f, 0.204f, 0.204f, 1.f);//RobotPage，BlankImwWindow，tabs等控件颜色
		style.Colors[ImGuiCol_MenuBarBg] = style.Colors[ImGuiCol_WindowBg];
		style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_Tab] = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 1.10f);
		style.Colors[ImGuiCol_TabHovered] = style.Colors[ImGuiCol_HeaderHovered];
		style.Colors[ImGuiCol_TabActive] = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.40f);
		style.Colors[ImGuiCol_TabUnfocused] = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.80f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.30f);
	}
	break;
	case 2://Dark
	{
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);

		style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(0.204f, 0.204f, 0.204f, 1.f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
		style.Colors[ImGuiCol_TabUnfocused] = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.80f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.40f);//不是活动窗时

		style.Colors[ImGuiCol_WindowBg] = style.Colors[ImGuiCol_TabActive];//窗口底色与Tab相同，当不是主活动框时需要即时调整一下，不然标签颜色与本体不一至
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.204f, 0.204f, 0.204f, 1.f);//控件窗颜色

		style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
		style.Colors[ImGuiCol_Border] = ImVec4(1.00f, 1.00f, 1.00f, 0.19f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);

		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	}
	break;
	case 3://Dark2
	{
		//ImGui::StyleColorsLight();
		//oConfig.m_eTabColorMode = ImwWindowManager::E_TABCOLORMODE_CUSTOM;
		style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.07f, 0.10f, 1.00f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.18f, 1.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.85f);
		//style.Colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.99f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.83f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.87f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.01f, 0.01f, 0.02f, 0.80f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.55f, 0.53f, 0.55f, 0.51f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.91f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.30f, 0.69f, 1.00f, 0.53f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.61f, 0.86f, 1.00f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.38f, 0.62f, 0.83f, 1.00f);
		style.Colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.50f, 0.50f, 0.90f, 0.50f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.70f, 0.70f, 0.90f, 0.60f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}
	break;
	case 4://Blue
	{
		//m_Config.m_eTabColorMode = ImwWindowManager::E_TABCOLORMODE_BACKGROUND;
		style.Colors[ImGuiCol_Text] = ImVec4(0.93f, 0.94f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.93f, 0.94f, 0.95f, 0.58f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.17f, 0.24f, 0.31f, 0.95f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.22f, 0.31f, 0.41f, 1.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.18f, 0.24f, 0.92f);
		//style.Colors[ImGuiCol_PopupBg] = ImVec4(0.22f, 0.31f, 0.41f, 1.00f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.17f, 0.24f, 0.31f, 0.00f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.17f, 0.24f, 0.31f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.27f, 0.37f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.50f, 0.73f, 0.78f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.50f, 0.73f, 1.00f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.22f, 0.31f, 0.41f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.22f, 0.31f, 0.41f, 0.75f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.50f, 0.73f, 1.00f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.22f, 0.31f, 0.41f, 0.47f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.22f, 0.31f, 0.41f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.16f, 0.50f, 0.73f, 0.21f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.16f, 0.50f, 0.73f, 0.78f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.16f, 0.50f, 0.73f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.16f, 0.50f, 0.73f, 0.80f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.16f, 0.50f, 0.73f, 0.50f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.16f, 0.50f, 0.73f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.16f, 0.50f, 0.73f, 0.76f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.16f, 0.50f, 0.73f, 0.86f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.16f, 0.50f, 0.73f, 1.00f);
		style.Colors[ImGuiCol_Separator] = ImVec4(0.16f, 0.50f, 0.73f, 0.32f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.16f, 0.50f, 0.73f, 0.78f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.16f, 0.50f, 0.73f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.16f, 0.50f, 0.73f, 0.15f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.16f, 0.50f, 0.73f, 0.78f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.16f, 0.50f, 0.73f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.93f, 0.94f, 0.95f, 0.16f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.93f, 0.94f, 0.95f, 0.39f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.93f, 0.94f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.93f, 0.94f, 0.95f, 0.63f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.16f, 0.50f, 0.73f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.93f, 0.94f, 0.95f, 0.63f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.16f, 0.50f, 0.73f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.16f, 0.50f, 0.73f, 0.43f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.22f, 0.31f, 0.41f, 0.73f);
	}
	break;
	}
}
void LoadImg(ConfigInfo* config)
{
	Imgstring::ImageString(config->nameToIcoString);
	std::ofstream os;
	std::vector<char> bufferimg = config->nameToIcoString.serialize();
	os.open(strCoding::Absolutepath("imgstr.bin"), std::ofstream::binary);
	if (os.is_open()) {
		os << std::string(bufferimg.begin(), bufferimg.end());
		os.close();
	}
}
void GetStrings(const char* p, wordslist* words)
{//折分由逗号分号分隔的字符串
	if (strlen(p) > 0)
	{
		char buf[2 * MAX_PATH] = { 0 };
		int m = 0;
		for (int k = 0; k < strlen(p); k++)
		{
			if (p[k] == ';' || p[k] == ',' || p[k] == ' ' || p[k] == '\n' || p[k] == '\r' || k > 2 && p[k - 2] == -17 && p[k - 1] == -68 && (p[k] == -116 || p[k] == -101))//；和，全角折成了3个字节
			{
				buf[m] = '\0';
				if (m > 0)
					words->insert(std::unordered_map<string, bool>::value_type(string(buf), false));
				m = 0;
			}
			else
				buf[m++] = p[k];
		}
		if (m > 0)
		{
			buf[m] = '\0';
			words->insert(std::unordered_map<string, bool>::value_type(string(buf), false));//最后一个
		}
	}
}

void LoadNamestr(ConfigInfo* config)
{
	DWORD tickCount = GetTickCount();
	StringPair::LoadLocalString(zh_CN, config->localstr);
}

void loadNameLoop(MainWindow* form, ConfigInfo* config)
{
	LoadImg(config);
	LoadNamestr(config);
}

void loadthread(MainWindow* form, ConfigInfo* config)
{
	std::thread loadname(loadNameLoop, form, config);
	loadname.detach();
}
void LoadSetting(ConfigInfo* config)
{
	
}
#endif  