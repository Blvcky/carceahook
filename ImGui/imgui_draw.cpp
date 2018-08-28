// dear imgui, v1.54 WIP
// (drawing and font code)

// Contains implementation for
// - Default styles
// - ImDrawList
// - ImDrawData
// - ImFontAtlas
// - ImFont
// - Default font data

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <stdio.h>      // vsnprintf, sscanf, printf
#if !defined(alloca)
#ifdef _WIN32
#include <malloc.h>     // alloca
#if !defined(alloca)
#define alloca _alloca  // for clang with MS Codegen
#endif
#elif defined(__GLIBC__) || defined(__sun)
#include <alloca.h>     // alloca
#else
#include <stdlib.h>     // alloca
#endif
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#define snprintf _snprintf
#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"            // warning : comparing floating point with == or != is unsafe   // storing and comparing against same constants ok.
#pragma clang diagnostic ignored "-Wglobal-constructors"    // warning : declaration requires a global destructor           // similar to above, not sure what the exact difference it.
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning : implicit conversion changes signedness             //
#if __has_warning("-Wcomma")
#pragma clang diagnostic ignored "-Wcomma"                  // warning : possible misuse of comma operator here             //
#endif
#if __has_warning("-Wreserved-id-macro")
#pragma clang diagnostic ignored "-Wreserved-id-macro"      // warning : macro name is a reserved identifier                //
#endif
#if __has_warning("-Wdouble-promotion")
#pragma clang diagnostic ignored "-Wdouble-promotion"       // warning: implicit conversion from 'float' to 'double' when passing argument to function
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"          // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wcast-qual"                // warning: cast from type 'xxxx' to type 'xxxx' casts away qualifiers
#endif

//-------------------------------------------------------------------------
// STB libraries implementation
//-------------------------------------------------------------------------

//#define IMGUI_STB_NAMESPACE     ImGuiStb
//#define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION

#ifdef IMGUI_STB_NAMESPACE
namespace IMGUI_STB_NAMESPACE
{
#endif

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4456)                             // declaration of 'xx' hides previous local declaration
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"              // warning: comparison is always true due to limited range of data type [-Wtype-limits]
#endif

#define STBRP_ASSERT(x)    IM_ASSERT(x)
#ifndef IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#define STB_RECT_PACK_IMPLEMENTATION
#endif
#include "stb_rect_pack.h"

#define STBTT_malloc(x,u)  ((void)(u), ImGui::MemAlloc(x))
#define STBTT_free(x,u)    ((void)(u), ImGui::MemFree(x))
#define STBTT_assert(x)    IM_ASSERT(x)
#ifndef IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#else
#define STBTT_DEF extern
#endif
#include "stb_truetype.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#ifdef IMGUI_STB_NAMESPACE
} // namespace ImGuiStb
using namespace IMGUI_STB_NAMESPACE;
#endif

//-----------------------------------------------------------------------------
// Style functions
//-----------------------------------------------------------------------------

void ImGui::StyleColorsClassic(ImGuiStyle* dst)
{
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
	colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.47f, 0.47f, 0.69f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.42f, 0.41f, 0.64f, 0.69f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.27f, 0.27f, 0.54f, 0.83f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.32f, 0.32f, 0.63f, 0.87f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.40f, 0.40f, 0.55f, 0.80f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.80f, 0.30f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.80f, 0.40f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
	colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
	colors[ImGuiCol_Button] = ImVec4(0.35f, 0.40f, 0.61f, 0.62f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.48f, 0.71f, 0.79f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.46f, 0.54f, 0.80f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.40f, 0.40f, 0.90f, 0.45f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.53f, 0.53f, 0.87f, 0.80f);
	colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.70f, 0.70f, 0.90f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.78f, 0.82f, 1.00f, 0.60f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.78f, 0.82f, 1.00f, 0.90f);
	colors[ImGuiCol_CloseButton] = ImVec4(0.50f, 0.50f, 0.90f, 0.50f);
	colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.70f, 0.70f, 0.90f, 0.60f);
	colors[ImGuiCol_CloseButtonActive] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
	colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
}

void ImGui::StyleColorsDark(ImGuiStyle* dst)
{
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];//ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_CloseButton] = ImVec4(0.41f, 0.41f, 0.41f, 0.50f);
	colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
}

// Those light colors are better suited with a thicker font than the default one + FrameBorder
void ImGui::StyleColorsLight(ImGuiStyle* dst)
{
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	//colors[ImGuiCol_TextHovered]          = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	//colors[ImGuiCol_TextActive]           = ImVec4(1.00f, 1.00f, 0.00f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
	colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
	colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
	colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
}

//-----------------------------------------------------------------------------
// ImDrawListData
//-----------------------------------------------------------------------------

ImDrawListSharedData::ImDrawListSharedData()
{
	Font = NULL;
	FontSize = 0.0f;
	CurveTessellationTol = 0.0f;
	ClipRectFullscreen = ImVec4(-8192.0f, -8192.0f, +8192.0f, +8192.0f);

	// Const data
	for (int i = 0; i < IM_ARRAYSIZE(CircleVtx12); i++)
	{
		const float a = ((float)i * 2 * IM_PI) / (float)IM_ARRAYSIZE(CircleVtx12);
		CircleVtx12[i] = ImVec2(cosf(a), sinf(a));
	}
}

//-----------------------------------------------------------------------------
// ImDrawList
//-----------------------------------------------------------------------------

void ImDrawList::Clear()
{
	CmdBuffer.resize(0);
	IdxBuffer.resize(0);
	VtxBuffer.resize(0);
	Flags = ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedFill;
	_VtxCurrentIdx = 0;
	_VtxWritePtr = NULL;
	_IdxWritePtr = NULL;
	_ClipRectStack.resize(0);
	_TextureIdStack.resize(0);
	_Path.resize(0);
	_ChannelsCurrent = 0;
	_ChannelsCount = 1;
	// NB: Do not clear channels so our allocations are re-used after the first frame.
}

void ImDrawList::ClearFreeMemory()
{
	CmdBuffer.clear();
	IdxBuffer.clear();
	VtxBuffer.clear();
	_VtxCurrentIdx = 0;
	_VtxWritePtr = NULL;
	_IdxWritePtr = NULL;
	_ClipRectStack.clear();
	_TextureIdStack.clear();
	_Path.clear();
	_ChannelsCurrent = 0;
	_ChannelsCount = 1;
	for (int i = 0; i < _Channels.Size; i++)
	{
		if (i == 0) memset(&_Channels[0], 0, sizeof(_Channels[0]));  // channel 0 is a copy of CmdBuffer/IdxBuffer, don't destruct again
		_Channels[i].CmdBuffer.clear();
		_Channels[i].IdxBuffer.clear();
	}
	_Channels.clear();
}

// Using macros because C++ is a terrible language, we want guaranteed inline, no code in header, and no overhead in Debug builds
#define GetCurrentClipRect()    (_ClipRectStack.Size ? _ClipRectStack.Data[_ClipRectStack.Size-1]  : _Data->ClipRectFullscreen)
#define GetCurrentTextureId()   (_TextureIdStack.Size ? _TextureIdStack.Data[_TextureIdStack.Size-1] : NULL)

void ImDrawList::AddDrawCmd()
{
	ImDrawCmd draw_cmd;
	draw_cmd.ClipRect = GetCurrentClipRect();
	draw_cmd.TextureId = GetCurrentTextureId();

	IM_ASSERT(draw_cmd.ClipRect.x <= draw_cmd.ClipRect.z && draw_cmd.ClipRect.y <= draw_cmd.ClipRect.w);
	CmdBuffer.push_back(draw_cmd);
}

void ImDrawList::AddCallback(ImDrawCallback callback, void* callback_data)
{
	ImDrawCmd* current_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
	if (!current_cmd || current_cmd->ElemCount != 0 || current_cmd->UserCallback != NULL)
	{
		AddDrawCmd();
		current_cmd = &CmdBuffer.back();
	}
	current_cmd->UserCallback = callback;
	current_cmd->UserCallbackData = callback_data;

	AddDrawCmd(); // Force a new command after us (see comment below)
}

// Our scheme may appears a bit unusual, basically we want the most-common calls AddLine AddRect etc. to not have to perform any check so we always have a command ready in the stack.
// The cost of figuring out if a new command has to be added or if we can merge is paid in those Update** functions only.
void ImDrawList::UpdateClipRect()
{
	// If current command is used with different settings we need to add a new command
	const ImVec4 curr_clip_rect = GetCurrentClipRect();
	ImDrawCmd* curr_cmd = CmdBuffer.Size > 0 ? &CmdBuffer.Data[CmdBuffer.Size - 1] : NULL;
	if (!curr_cmd || (curr_cmd->ElemCount != 0 && memcmp(&curr_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) != 0) || curr_cmd->UserCallback != NULL)
	{
		AddDrawCmd();
		return;
	}

	// Try to merge with previous command if it matches, else use current command
	ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
	if (curr_cmd->ElemCount == 0 && prev_cmd && memcmp(&prev_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) == 0 && prev_cmd->TextureId == GetCurrentTextureId() && prev_cmd->UserCallback == NULL)
		CmdBuffer.pop_back();
	else
		curr_cmd->ClipRect = curr_clip_rect;
}

void ImDrawList::UpdateTextureID()
{
	// If current command is used with different settings we need to add a new command
	const ImTextureID curr_texture_id = GetCurrentTextureId();
	ImDrawCmd* curr_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
	if (!curr_cmd || (curr_cmd->ElemCount != 0 && curr_cmd->TextureId != curr_texture_id) || curr_cmd->UserCallback != NULL)
	{
		AddDrawCmd();
		return;
	}

	// Try to merge with previous command if it matches, else use current command
	ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
	if (curr_cmd->ElemCount == 0 && prev_cmd && prev_cmd->TextureId == curr_texture_id && memcmp(&prev_cmd->ClipRect, &GetCurrentClipRect(), sizeof(ImVec4)) == 0 && prev_cmd->UserCallback == NULL)
		CmdBuffer.pop_back();
	else
		curr_cmd->TextureId = curr_texture_id;
}

#undef GetCurrentClipRect
#undef GetCurrentTextureId

// Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
void ImDrawList::PushClipRect(ImVec2 cr_min, ImVec2 cr_max, bool intersect_with_current_clip_rect)
{
	ImVec4 cr(cr_min.x, cr_min.y, cr_max.x, cr_max.y);
	if (intersect_with_current_clip_rect && _ClipRectStack.Size)
	{
		ImVec4 current = _ClipRectStack.Data[_ClipRectStack.Size - 1];
		if (cr.x < current.x) cr.x = current.x;
		if (cr.y < current.y) cr.y = current.y;
		if (cr.z > current.z) cr.z = current.z;
		if (cr.w > current.w) cr.w = current.w;
	}
	cr.z = ImMax(cr.x, cr.z);
	cr.w = ImMax(cr.y, cr.w);

	_ClipRectStack.push_back(cr);
	UpdateClipRect();
}

void ImDrawList::PushClipRectFullScreen()
{
	PushClipRect(ImVec2(_Data->ClipRectFullscreen.x, _Data->ClipRectFullscreen.y), ImVec2(_Data->ClipRectFullscreen.z, _Data->ClipRectFullscreen.w));
}

void ImDrawList::PopClipRect()
{
	IM_ASSERT(_ClipRectStack.Size > 0);
	_ClipRectStack.pop_back();
	UpdateClipRect();
}

void ImDrawList::PushTextureID(const ImTextureID& texture_id)
{
	_TextureIdStack.push_back(texture_id);
	UpdateTextureID();
}

void ImDrawList::PopTextureID()
{
	IM_ASSERT(_TextureIdStack.Size > 0);
	_TextureIdStack.pop_back();
	UpdateTextureID();
}

void ImDrawList::ChannelsSplit(int channels_count)
{
	IM_ASSERT(_ChannelsCurrent == 0 && _ChannelsCount == 1);
	int old_channels_count = _Channels.Size;
	if (old_channels_count < channels_count)
		_Channels.resize(channels_count);
	_ChannelsCount = channels_count;

	// _Channels[] (24/32 bytes each) hold storage that we'll swap with this->_CmdBuffer/_IdxBuffer
	// The content of _Channels[0] at this point doesn't matter. We clear it to make state tidy in a debugger but we don't strictly need to.
	// When we switch to the next channel, we'll copy _CmdBuffer/_IdxBuffer into _Channels[0] and then _Channels[1] into _CmdBuffer/_IdxBuffer
	memset(&_Channels[0], 0, sizeof(ImDrawChannel));
	for (int i = 1; i < channels_count; i++)
	{
		if (i >= old_channels_count)
		{
			IM_PLACEMENT_NEW(&_Channels[i]) ImDrawChannel();
		}
		else
		{
			_Channels[i].CmdBuffer.resize(0);
			_Channels[i].IdxBuffer.resize(0);
		}
		if (_Channels[i].CmdBuffer.Size == 0)
		{
			ImDrawCmd draw_cmd;
			draw_cmd.ClipRect = _ClipRectStack.back();
			draw_cmd.TextureId = _TextureIdStack.back();
			_Channels[i].CmdBuffer.push_back(draw_cmd);
		}
	}
}

void ImDrawList::ChannelsMerge()
{
	// Note that we never use or rely on channels.Size because it is merely a buffer that we never shrink back to 0 to keep all sub-buffers ready for use.
	if (_ChannelsCount <= 1)
		return;

	ChannelsSetCurrent(0);
	if (CmdBuffer.Size && CmdBuffer.back().ElemCount == 0)
		CmdBuffer.pop_back();

	int new_cmd_buffer_count = 0, new_idx_buffer_count = 0;
	for (int i = 1; i < _ChannelsCount; i++)
	{
		ImDrawChannel& ch = _Channels[i];
		if (ch.CmdBuffer.Size && ch.CmdBuffer.back().ElemCount == 0)
			ch.CmdBuffer.pop_back();
		new_cmd_buffer_count += ch.CmdBuffer.Size;
		new_idx_buffer_count += ch.IdxBuffer.Size;
	}
	CmdBuffer.resize(CmdBuffer.Size + new_cmd_buffer_count);
	IdxBuffer.resize(IdxBuffer.Size + new_idx_buffer_count);

	ImDrawCmd* cmd_write = CmdBuffer.Data + CmdBuffer.Size - new_cmd_buffer_count;
	_IdxWritePtr = IdxBuffer.Data + IdxBuffer.Size - new_idx_buffer_count;
	for (int i = 1; i < _ChannelsCount; i++)
	{
		ImDrawChannel& ch = _Channels[i];
		if (int sz = ch.CmdBuffer.Size) { memcpy(cmd_write, ch.CmdBuffer.Data, sz * sizeof(ImDrawCmd)); cmd_write += sz; }
		if (int sz = ch.IdxBuffer.Size) { memcpy(_IdxWritePtr, ch.IdxBuffer.Data, sz * sizeof(ImDrawIdx)); _IdxWritePtr += sz; }
	}
	UpdateClipRect(); // We call this instead of AddDrawCmd(), so that empty channels won't produce an extra draw call.
	_ChannelsCount = 1;
}

void ImDrawList::ChannelsSetCurrent(int idx)
{
	IM_ASSERT(idx < _ChannelsCount);
	if (_ChannelsCurrent == idx) return;
	memcpy(&_Channels.Data[_ChannelsCurrent].CmdBuffer, &CmdBuffer, sizeof(CmdBuffer)); // copy 12 bytes, four times
	memcpy(&_Channels.Data[_ChannelsCurrent].IdxBuffer, &IdxBuffer, sizeof(IdxBuffer));
	_ChannelsCurrent = idx;
	memcpy(&CmdBuffer, &_Channels.Data[_ChannelsCurrent].CmdBuffer, sizeof(CmdBuffer));
	memcpy(&IdxBuffer, &_Channels.Data[_ChannelsCurrent].IdxBuffer, sizeof(IdxBuffer));
	_IdxWritePtr = IdxBuffer.Data + IdxBuffer.Size;
}

// NB: this can be called with negative count for removing primitives (as long as the result does not underflow)
void ImDrawList::PrimReserve(int idx_count, int vtx_count)
{
	ImDrawCmd& draw_cmd = CmdBuffer.Data[CmdBuffer.Size - 1];
	draw_cmd.ElemCount += idx_count;

	int vtx_buffer_old_size = VtxBuffer.Size;
	VtxBuffer.resize(vtx_buffer_old_size + vtx_count);
	_VtxWritePtr = VtxBuffer.Data + vtx_buffer_old_size;

	int idx_buffer_old_size = IdxBuffer.Size;
	IdxBuffer.resize(idx_buffer_old_size + idx_count);
	_IdxWritePtr = IdxBuffer.Data + idx_buffer_old_size;
}

// Fully unrolled with inline call to keep our debug builds decently fast.
void ImDrawList::PrimRect(const ImVec2& a, const ImVec2& c, ImU32 col)
{
	ImVec2 b(c.x, a.y), d(a.x, c.y), uv(_Data->TexUvWhitePixel);
	ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
	_IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx + 2);
	_IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx + 3);
	_VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
	_VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
	_VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
	_VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
	_VtxWritePtr += 4;
	_VtxCurrentIdx += 4;
	_IdxWritePtr += 6;
}

void ImDrawList::PrimRectUV(const ImVec2& a, const ImVec2& c, const ImVec2& uv_a, const ImVec2& uv_c, ImU32 col)
{
	ImVec2 b(c.x, a.y), d(a.x, c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
	ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
	_IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx + 2);
	_IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx + 3);
	_VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
	_VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
	_VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
	_VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
	_VtxWritePtr += 4;
	_VtxCurrentIdx += 4;
	_IdxWritePtr += 6;
}

void ImDrawList::PrimQuadUV(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
	ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
	_IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx + 2);
	_IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx + 3);
	_VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
	_VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
	_VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
	_VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
	_VtxWritePtr += 4;
	_VtxCurrentIdx += 4;
	_IdxWritePtr += 6;
}

// TODO: Thickness anti-aliased lines cap are missing their AA fringe.
void ImDrawList::AddPolyline(const ImVec2* points, const int points_count, ImU32 col, bool closed, float thickness)
{
	if (points_count < 2)
		return;

	const ImVec2 uv = _Data->TexUvWhitePixel;

	int count = points_count;
	if (!closed)
		count = points_count - 1;

	const bool thick_line = thickness > 1.0f;
	if (Flags & ImDrawListFlags_AntiAliasedLines)
	{
		// Anti-aliased stroke
		const float AA_SIZE = 1.0f;
		const ImU32 col_trans = col & ~IM_COL32_A_MASK;

		const int idx_count = thick_line ? count * 18 : count * 12;
		const int vtx_count = thick_line ? points_count * 4 : points_count * 3;
		PrimReserve(idx_count, vtx_count);

		// Temporary buffer
		ImVec2* temp_normals = (ImVec2*)alloca(points_count * (thick_line ? 5 : 3) * sizeof(ImVec2));
		ImVec2* temp_points = temp_normals + points_count;

		for (int i1 = 0; i1 < count; i1++)
		{
			const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
			ImVec2 diff = points[i2] - points[i1];
			diff *= ImInvLength(diff, 1.0f);
			temp_normals[i1].x = diff.y;
			temp_normals[i1].y = -diff.x;
		}
		if (!closed)
			temp_normals[points_count - 1] = temp_normals[points_count - 2];

		if (!thick_line)
		{
			if (!closed)
			{
				temp_points[0] = points[0] + temp_normals[0] * AA_SIZE;
				temp_points[1] = points[0] - temp_normals[0] * AA_SIZE;
				temp_points[(points_count - 1) * 2 + 0] = points[points_count - 1] + temp_normals[points_count - 1] * AA_SIZE;
				temp_points[(points_count - 1) * 2 + 1] = points[points_count - 1] - temp_normals[points_count - 1] * AA_SIZE;
			}

			// FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
			unsigned int idx1 = _VtxCurrentIdx;
			for (int i1 = 0; i1 < count; i1++)
			{
				const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
				unsigned int idx2 = (i1 + 1) == points_count ? _VtxCurrentIdx : idx1 + 3;

				// Average normals
				ImVec2 dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
				float dmr2 = dm.x*dm.x + dm.y*dm.y;
				if (dmr2 > 0.000001f)
				{
					float scale = 1.0f / dmr2;
					if (scale > 100.0f) scale = 100.0f;
					dm *= scale;
				}
				dm *= AA_SIZE;
				temp_points[i2 * 2 + 0] = points[i2] + dm;
				temp_points[i2 * 2 + 1] = points[i2] - dm;

				// Add indexes
				_IdxWritePtr[0] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[1] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[2] = (ImDrawIdx)(idx1 + 2);
				_IdxWritePtr[3] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[4] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx2 + 0);
				_IdxWritePtr[6] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[7] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[8] = (ImDrawIdx)(idx1 + 0);
				_IdxWritePtr[9] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[10] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[11] = (ImDrawIdx)(idx2 + 1);
				_IdxWritePtr += 12;

				idx1 = idx2;
			}

			// Add vertexes
			for (int i = 0; i < points_count; i++)
			{
				_VtxWritePtr[0].pos = points[i];          _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
				_VtxWritePtr[1].pos = temp_points[i * 2 + 0]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;
				_VtxWritePtr[2].pos = temp_points[i * 2 + 1]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col_trans;
				_VtxWritePtr += 3;
			}
		}
		else
		{
			const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;
			if (!closed)
			{
				temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
				temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
				temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
				temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
				temp_points[(points_count - 1) * 4 + 0] = points[points_count - 1] + temp_normals[points_count - 1] * (half_inner_thickness + AA_SIZE);
				temp_points[(points_count - 1) * 4 + 1] = points[points_count - 1] + temp_normals[points_count - 1] * (half_inner_thickness);
				temp_points[(points_count - 1) * 4 + 2] = points[points_count - 1] - temp_normals[points_count - 1] * (half_inner_thickness);
				temp_points[(points_count - 1) * 4 + 3] = points[points_count - 1] - temp_normals[points_count - 1] * (half_inner_thickness + AA_SIZE);
			}

			// FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
			unsigned int idx1 = _VtxCurrentIdx;
			for (int i1 = 0; i1 < count; i1++)
			{
				const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
				unsigned int idx2 = (i1 + 1) == points_count ? _VtxCurrentIdx : idx1 + 4;

				// Average normals
				ImVec2 dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
				float dmr2 = dm.x*dm.x + dm.y*dm.y;
				if (dmr2 > 0.000001f)
				{
					float scale = 1.0f / dmr2;
					if (scale > 100.0f) scale = 100.0f;
					dm *= scale;
				}
				ImVec2 dm_out = dm * (half_inner_thickness + AA_SIZE);
				ImVec2 dm_in = dm * half_inner_thickness;
				temp_points[i2 * 4 + 0] = points[i2] + dm_out;
				temp_points[i2 * 4 + 1] = points[i2] + dm_in;
				temp_points[i2 * 4 + 2] = points[i2] - dm_in;
				temp_points[i2 * 4 + 3] = points[i2] - dm_out;

				// Add indexes
				_IdxWritePtr[0] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[1] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx1 + 2);
				_IdxWritePtr[3] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[4] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx2 + 1);
				_IdxWritePtr[6] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[7] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[8] = (ImDrawIdx)(idx1 + 0);
				_IdxWritePtr[9] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[10] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[11] = (ImDrawIdx)(idx2 + 1);
				_IdxWritePtr[12] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[13] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[14] = (ImDrawIdx)(idx1 + 3);
				_IdxWritePtr[15] = (ImDrawIdx)(idx1 + 3); _IdxWritePtr[16] = (ImDrawIdx)(idx2 + 3); _IdxWritePtr[17] = (ImDrawIdx)(idx2 + 2);
				_IdxWritePtr += 18;

				idx1 = idx2;
			}

			// Add vertexes
			for (int i = 0; i < points_count; i++)
			{
				_VtxWritePtr[0].pos = temp_points[i * 4 + 0]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col_trans;
				_VtxWritePtr[1].pos = temp_points[i * 4 + 1]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
				_VtxWritePtr[2].pos = temp_points[i * 4 + 2]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
				_VtxWritePtr[3].pos = temp_points[i * 4 + 3]; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col_trans;
				_VtxWritePtr += 4;
			}
		}
		_VtxCurrentIdx += (ImDrawIdx)vtx_count;
	}
	else
	{
		// Non Anti-aliased Stroke
		const int idx_count = count * 6;
		const int vtx_count = count * 4;      // FIXME-OPT: Not sharing edges
		PrimReserve(idx_count, vtx_count);

		for (int i1 = 0; i1 < count; i1++)
		{
			const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
			const ImVec2& p1 = points[i1];
			const ImVec2& p2 = points[i2];
			ImVec2 diff = p2 - p1;
			diff *= ImInvLength(diff, 1.0f);

			const float dx = diff.x * (thickness * 0.5f);
			const float dy = diff.y * (thickness * 0.5f);
			_VtxWritePtr[0].pos.x = p1.x + dy; _VtxWritePtr[0].pos.y = p1.y - dx; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
			_VtxWritePtr[1].pos.x = p2.x + dy; _VtxWritePtr[1].pos.y = p2.y - dx; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
			_VtxWritePtr[2].pos.x = p2.x - dy; _VtxWritePtr[2].pos.y = p2.y + dx; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
			_VtxWritePtr[3].pos.x = p1.x - dy; _VtxWritePtr[3].pos.y = p1.y + dx; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
			_VtxWritePtr += 4;

			_IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx + 1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx + 2);
			_IdxWritePtr[3] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[4] = (ImDrawIdx)(_VtxCurrentIdx + 2); _IdxWritePtr[5] = (ImDrawIdx)(_VtxCurrentIdx + 3);
			_IdxWritePtr += 6;
			_VtxCurrentIdx += 4;
		}
	}
}

void ImDrawList::AddConvexPolyFilled(const ImVec2* points, const int points_count, ImU32 col)
{
	const ImVec2 uv = _Data->TexUvWhitePixel;

	if (Flags & ImDrawListFlags_AntiAliasedFill)
	{
		// Anti-aliased Fill
		const float AA_SIZE = 1.0f;
		const ImU32 col_trans = col & ~IM_COL32_A_MASK;
		const int idx_count = (points_count - 2) * 3 + points_count * 6;
		const int vtx_count = (points_count * 2);
		PrimReserve(idx_count, vtx_count);

		// Add indexes for fill
		unsigned int vtx_inner_idx = _VtxCurrentIdx;
		unsigned int vtx_outer_idx = _VtxCurrentIdx + 1;
		for (int i = 2; i < points_count; i++)
		{
			_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + ((i - 1) << 1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx + (i << 1));
			_IdxWritePtr += 3;
		}

		// Compute normals
		ImVec2* temp_normals = (ImVec2*)alloca(points_count * sizeof(ImVec2));
		for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
		{
			const ImVec2& p0 = points[i0];
			const ImVec2& p1 = points[i1];
			ImVec2 diff = p1 - p0;
			diff *= ImInvLength(diff, 1.0f);
			temp_normals[i0].x = diff.y;
			temp_normals[i0].y = -diff.x;
		}

		for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
		{
			// Average normals
			const ImVec2& n0 = temp_normals[i0];
			const ImVec2& n1 = temp_normals[i1];
			ImVec2 dm = (n0 + n1) * 0.5f;
			float dmr2 = dm.x*dm.x + dm.y*dm.y;
			if (dmr2 > 0.000001f)
			{
				float scale = 1.0f / dmr2;
				if (scale > 100.0f) scale = 100.0f;
				dm *= scale;
			}
			dm *= AA_SIZE * 0.5f;

			// Add vertices
			_VtxWritePtr[0].pos = (points[i1] - dm); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
			_VtxWritePtr[1].pos = (points[i1] + dm); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
			_VtxWritePtr += 2;

			// Add indexes for fringes
			_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1)); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + (i0 << 1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1));
			_IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1)); _IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx + (i1 << 1)); _IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1));
			_IdxWritePtr += 6;
		}
		_VtxCurrentIdx += (ImDrawIdx)vtx_count;
	}
	else
	{
		// Non Anti-aliased Fill
		const int idx_count = (points_count - 2) * 3;
		const int vtx_count = points_count;
		PrimReserve(idx_count, vtx_count);
		for (int i = 0; i < vtx_count; i++)
		{
			_VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
			_VtxWritePtr++;
		}
		for (int i = 2; i < points_count; i++)
		{
			_IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx + i - 1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx + i);
			_IdxWritePtr += 3;
		}
		_VtxCurrentIdx += (ImDrawIdx)vtx_count;
	}
}

void ImDrawList::PathArcToFast(const ImVec2& centre, float radius, int a_min_of_12, int a_max_of_12)
{
	if (radius == 0.0f || a_min_of_12 > a_max_of_12)
	{
		_Path.push_back(centre);
		return;
	}
	_Path.reserve(_Path.Size + (a_max_of_12 - a_min_of_12 + 1));
	for (int a = a_min_of_12; a <= a_max_of_12; a++)
	{
		const ImVec2& c = _Data->CircleVtx12[a % IM_ARRAYSIZE(_Data->CircleVtx12)];
		_Path.push_back(ImVec2(centre.x + c.x * radius, centre.y + c.y * radius));
	}
}

void ImDrawList::PathArcTo(const ImVec2& centre, float radius, float a_min, float a_max, int num_segments)
{
	if (radius == 0.0f)
	{
		_Path.push_back(centre);
		return;
	}
	_Path.reserve(_Path.Size + (num_segments + 1));
	for (int i = 0; i <= num_segments; i++)
	{
		const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
		_Path.push_back(ImVec2(centre.x + cosf(a) * radius, centre.y + sinf(a) * radius));
	}
}

static void PathBezierToCasteljau(ImVector<ImVec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
{
	float dx = x4 - x1;
	float dy = y4 - y1;
	float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
	float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
	d2 = (d2 >= 0) ? d2 : -d2;
	d3 = (d3 >= 0) ? d3 : -d3;
	if ((d2 + d3) * (d2 + d3) < tess_tol * (dx*dx + dy*dy))
	{
		path->push_back(ImVec2(x4, y4));
	}
	else if (level < 10)
	{
		float x12 = (x1 + x2)*0.5f, y12 = (y1 + y2)*0.5f;
		float x23 = (x2 + x3)*0.5f, y23 = (y2 + y3)*0.5f;
		float x34 = (x3 + x4)*0.5f, y34 = (y3 + y4)*0.5f;
		float x123 = (x12 + x23)*0.5f, y123 = (y12 + y23)*0.5f;
		float x234 = (x23 + x34)*0.5f, y234 = (y23 + y34)*0.5f;
		float x1234 = (x123 + x234)*0.5f, y1234 = (y123 + y234)*0.5f;

		PathBezierToCasteljau(path, x1, y1, x12, y12, x123, y123, x1234, y1234, tess_tol, level + 1);
		PathBezierToCasteljau(path, x1234, y1234, x234, y234, x34, y34, x4, y4, tess_tol, level + 1);
	}
}

void ImDrawList::PathBezierCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments)
{
	ImVec2 p1 = _Path.back();
	if (num_segments == 0)
	{
		// Auto-tessellated
		PathBezierToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, _Data->CurveTessellationTol, 0);
	}
	else
	{
		float t_step = 1.0f / (float)num_segments;
		for (int i_step = 1; i_step <= num_segments; i_step++)
		{
			float t = t_step * i_step;
			float u = 1.0f - t;
			float w1 = u*u*u;
			float w2 = 3 * u*u*t;
			float w3 = 3 * u*t*t;
			float w4 = t*t*t;
			_Path.push_back(ImVec2(w1*p1.x + w2*p2.x + w3*p3.x + w4*p4.x, w1*p1.y + w2*p2.y + w3*p3.y + w4*p4.y));
		}
	}
}

void ImDrawList::PathRect(const ImVec2& a, const ImVec2& b, float rounding, int rounding_corners)
{
	rounding = ImMin(rounding, fabsf(b.x - a.x) * (((rounding_corners & ImDrawCornerFlags_Top) == ImDrawCornerFlags_Top) || ((rounding_corners & ImDrawCornerFlags_Bot) == ImDrawCornerFlags_Bot) ? 0.5f : 1.0f) - 1.0f);
	rounding = ImMin(rounding, fabsf(b.y - a.y) * (((rounding_corners & ImDrawCornerFlags_Left) == ImDrawCornerFlags_Left) || ((rounding_corners & ImDrawCornerFlags_Right) == ImDrawCornerFlags_Right) ? 0.5f : 1.0f) - 1.0f);

	if (rounding <= 0.0f || rounding_corners == 0)
	{
		PathLineTo(a);
		PathLineTo(ImVec2(b.x, a.y));
		PathLineTo(b);
		PathLineTo(ImVec2(a.x, b.y));
	}
	else
	{
		const float rounding_tl = (rounding_corners & ImDrawCornerFlags_TopLeft) ? rounding : 0.0f;
		const float rounding_tr = (rounding_corners & ImDrawCornerFlags_TopRight) ? rounding : 0.0f;
		const float rounding_br = (rounding_corners & ImDrawCornerFlags_BotRight) ? rounding : 0.0f;
		const float rounding_bl = (rounding_corners & ImDrawCornerFlags_BotLeft) ? rounding : 0.0f;
		PathArcToFast(ImVec2(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
		PathArcToFast(ImVec2(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
		PathArcToFast(ImVec2(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
		PathArcToFast(ImVec2(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
	}
}

void ImDrawList::AddLine(const ImVec2& a, const ImVec2& b, ImU32 col, float thickness)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;
	PathLineTo(a + ImVec2(0.5f, 0.5f));
	PathLineTo(b + ImVec2(0.5f, 0.5f));
	PathStroke(col, false, thickness);
}

// a: upper-left, b: lower-right. we don't render 1 px sized rectangles properly.
void ImDrawList::AddRect(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners_flags, float thickness)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;
	PathRect(a + ImVec2(0.5f, 0.5f), b - ImVec2(0.5f, 0.5f), rounding, rounding_corners_flags);
	PathStroke(col, true, thickness);
}

void ImDrawList::AddRectFilled(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners_flags)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;
	if (rounding > 0.0f)
	{
		PathRect(a, b, rounding, rounding_corners_flags);
		PathFillConvex(col);
	}
	else
	{
		PrimReserve(6, 4);
		PrimRect(a, b, col);
	}
}

void ImDrawList::AddRectFilledMultiColor(const ImVec2& a, const ImVec2& c, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left)
{
	if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
		return;

	const ImVec2 uv = _Data->TexUvWhitePixel;
	PrimReserve(6, 4);
	PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 1)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 2));
	PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 2)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 3));
	PrimWriteVtx(a, uv, col_upr_left);
	PrimWriteVtx(ImVec2(c.x, a.y), uv, col_upr_right);
	PrimWriteVtx(c, uv, col_bot_right);
	PrimWriteVtx(ImVec2(a.x, c.y), uv, col_bot_left);
}

void ImDrawList::AddQuad(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col, float thickness)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	PathLineTo(a);
	PathLineTo(b);
	PathLineTo(c);
	PathLineTo(d);
	PathStroke(col, true, thickness);
}

void ImDrawList::AddQuadFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	PathLineTo(a);
	PathLineTo(b);
	PathLineTo(c);
	PathLineTo(d);
	PathFillConvex(col);
}

void ImDrawList::AddTriangle(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col, float thickness)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	PathLineTo(a);
	PathLineTo(b);
	PathLineTo(c);
	PathStroke(col, true, thickness);
}

void ImDrawList::AddTriangleFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	PathLineTo(a);
	PathLineTo(b);
	PathLineTo(c);
	PathFillConvex(col);
}

void ImDrawList::AddCircle(const ImVec2& centre, float radius, ImU32 col, int num_segments, float thickness)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
	PathArcTo(centre, radius - 0.5f, 0.0f, a_max, num_segments);
	PathStroke(col, true, thickness);
}

void ImDrawList::AddCircleFilled(const ImVec2& centre, float radius, ImU32 col, int num_segments)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
	PathArcTo(centre, radius, 0.0f, a_max, num_segments);
	PathFillConvex(col);
}

void ImDrawList::AddBezierCurve(const ImVec2& pos0, const ImVec2& cp0, const ImVec2& cp1, const ImVec2& pos1, ImU32 col, float thickness, int num_segments)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	PathLineTo(pos0);
	PathBezierCurveTo(cp0, cp1, pos1, num_segments);
	PathStroke(col, false, thickness);
}

void ImDrawList::AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	if (text_end == NULL)
		text_end = text_begin + strlen(text_begin);
	if (text_begin == text_end)
		return;

	// Pull default font/size from the shared ImDrawListSharedData instance
	if (font == NULL)
		font = _Data->Font;
	if (font_size == 0.0f)
		font_size = _Data->FontSize;

	IM_ASSERT(font->ContainerAtlas->TexID == _TextureIdStack.back());  // Use high-level ImGui::PushFont() or low-level ImDrawList::PushTextureId() to change font.

	ImVec4 clip_rect = _ClipRectStack.back();
	if (cpu_fine_clip_rect)
	{
		clip_rect.x = ImMax(clip_rect.x, cpu_fine_clip_rect->x);
		clip_rect.y = ImMax(clip_rect.y, cpu_fine_clip_rect->y);
		clip_rect.z = ImMin(clip_rect.z, cpu_fine_clip_rect->z);
		clip_rect.w = ImMin(clip_rect.w, cpu_fine_clip_rect->w);
	}
	font->RenderText(this, font_size, pos, col, clip_rect, text_begin, text_end, wrap_width, cpu_fine_clip_rect != NULL);
}

void ImDrawList::AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end)
{
	AddText(NULL, 0.0f, pos, col, text_begin, text_end);
}

void ImDrawList::AddImage(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImU32 col)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
	if (push_texture_id)
		PushTextureID(user_texture_id);

	PrimReserve(6, 4);
	PrimRectUV(a, b, uv_a, uv_b, col);

	if (push_texture_id)
		PopTextureID();
}

void ImDrawList::AddImageQuad(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
	if (push_texture_id)
		PushTextureID(user_texture_id);

	PrimReserve(6, 4);
	PrimQuadUV(a, b, c, d, uv_a, uv_b, uv_c, uv_d, col);

	if (push_texture_id)
		PopTextureID();
}

void ImDrawList::AddImageRounded(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImU32 col, float rounding, int rounding_corners)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	if (rounding <= 0.0f || (rounding_corners & ImDrawCornerFlags_All) == 0)
	{
		AddImage(user_texture_id, a, b, uv_a, uv_b, col);
		return;
	}

	const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
	if (push_texture_id)
		PushTextureID(user_texture_id);

	int vert_start_idx = VtxBuffer.Size;
	PathRect(a, b, rounding, rounding_corners);
	PathFillConvex(col);
	int vert_end_idx = VtxBuffer.Size;
	ImGui::ShadeVertsLinearUV(VtxBuffer.Data + vert_start_idx, VtxBuffer.Data + vert_end_idx, a, b, uv_a, uv_b, true);

	if (push_texture_id)
		PopTextureID();
}

//-----------------------------------------------------------------------------
// ImDrawData
//-----------------------------------------------------------------------------

// For backward compatibility: convert all buffers from indexed to de-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
void ImDrawData::DeIndexAllBuffers()
{
	ImVector<ImDrawVert> new_vtx_buffer;
	TotalVtxCount = TotalIdxCount = 0;
	for (int i = 0; i < CmdListsCount; i++)
	{
		ImDrawList* cmd_list = CmdLists[i];
		if (cmd_list->IdxBuffer.empty())
			continue;
		new_vtx_buffer.resize(cmd_list->IdxBuffer.Size);
		for (int j = 0; j < cmd_list->IdxBuffer.Size; j++)
			new_vtx_buffer[j] = cmd_list->VtxBuffer[cmd_list->IdxBuffer[j]];
		cmd_list->VtxBuffer.swap(new_vtx_buffer);
		cmd_list->IdxBuffer.resize(0);
		TotalVtxCount += cmd_list->VtxBuffer.Size;
	}
}

// Helper to scale the ClipRect field of each ImDrawCmd. Use if your final output buffer is at a different scale than ImGui expects, or if there is a difference between your window resolution and framebuffer resolution.
void ImDrawData::ScaleClipRects(const ImVec2& scale)
{
	for (int i = 0; i < CmdListsCount; i++)
	{
		ImDrawList* cmd_list = CmdLists[i];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			ImDrawCmd* cmd = &cmd_list->CmdBuffer[cmd_i];
			cmd->ClipRect = ImVec4(cmd->ClipRect.x * scale.x, cmd->ClipRect.y * scale.y, cmd->ClipRect.z * scale.x, cmd->ClipRect.w * scale.y);
		}
	}
}

//-----------------------------------------------------------------------------
// Shade functions
//-----------------------------------------------------------------------------

// Generic linear color gradient, write to RGB fields, leave A untouched.
void ImGui::ShadeVertsLinearColorGradientKeepAlpha(ImDrawVert* vert_start, ImDrawVert* vert_end, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{
	ImVec2 gradient_extent = gradient_p1 - gradient_p0;
	float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
	for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
	{
		float d = ImDot(vert->pos - gradient_p0, gradient_extent);
		float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
		int r = ImLerp((int)(col0 >> IM_COL32_R_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_R_SHIFT) & 0xFF, t);
		int g = ImLerp((int)(col0 >> IM_COL32_G_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_G_SHIFT) & 0xFF, t);
		int b = ImLerp((int)(col0 >> IM_COL32_B_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_B_SHIFT) & 0xFF, t);
		vert->col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (vert->col & IM_COL32_A_MASK);
	}
}

// Scan and shade backward from the end of given vertices. Assume vertices are text only (= vert_start..vert_end going left to right) so we can break as soon as we are out the gradient bounds.
void ImGui::ShadeVertsLinearAlphaGradientForLeftToRightText(ImDrawVert* vert_start, ImDrawVert* vert_end, float gradient_p0_x, float gradient_p1_x)
{
	float gradient_extent_x = gradient_p1_x - gradient_p0_x;
	float gradient_inv_length2 = 1.0f / (gradient_extent_x * gradient_extent_x);
	int full_alpha_count = 0;
	for (ImDrawVert* vert = vert_end - 1; vert >= vert_start; vert--)
	{
		float d = (vert->pos.x - gradient_p0_x) * (gradient_extent_x);
		float alpha_mul = 1.0f - ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
		if (alpha_mul >= 1.0f && ++full_alpha_count > 2)
			return; // Early out
		int a = (int)(((vert->col >> IM_COL32_A_SHIFT) & 0xFF) * alpha_mul);
		vert->col = (vert->col & ~IM_COL32_A_MASK) | (a << IM_COL32_A_SHIFT);
	}
}

// Distribute UV over (a, b) rectangle
void ImGui::ShadeVertsLinearUV(ImDrawVert* vert_start, ImDrawVert* vert_end, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, bool clamp)
{
	const ImVec2 size = b - a;
	const ImVec2 uv_size = uv_b - uv_a;
	const ImVec2 scale = ImVec2(
		size.x != 0.0f ? (uv_size.x / size.x) : 0.0f,
		size.y != 0.0f ? (uv_size.y / size.y) : 0.0f);

	if (clamp)
	{
		const ImVec2 min = ImMin(uv_a, uv_b);
		const ImVec2 max = ImMax(uv_a, uv_b);

		for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
			vertex->uv = ImClamp(uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale), min, max);
	}
	else
	{
		for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
			vertex->uv = uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale);
	}
}

//-----------------------------------------------------------------------------
// ImFontConfig
//-----------------------------------------------------------------------------

ImFontConfig::ImFontConfig()
{
	FontData = NULL;
	FontDataSize = 0;
	FontDataOwnedByAtlas = true;
	FontNo = 0;
	SizePixels = 0.0f;
	OversampleH = 3;
	OversampleV = 1;
	PixelSnapH = false;
	GlyphExtraSpacing = ImVec2(0.0f, 0.0f);
	GlyphOffset = ImVec2(0.0f, 0.0f);
	GlyphRanges = NULL;
	MergeMode = false;
	RasterizerFlags = 0x00;
	RasterizerMultiply = 1.0f;
	memset(Name, 0, sizeof(Name));
	DstFont = NULL;
}

//-----------------------------------------------------------------------------
// ImFontAtlas
//-----------------------------------------------------------------------------

// A work of art lies ahead! (. = white layer, X = black layer, others are blank)
// The white texels on the top left are the ones we'll use everywhere in ImGui to render filled shapes.
const int FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF = 90;
const int FONT_ATLAS_DEFAULT_TEX_DATA_H = 27;
const unsigned int FONT_ATLAS_DEFAULT_TEX_DATA_ID = 0x80000000;
static const char FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * FONT_ATLAS_DEFAULT_TEX_DATA_H + 1] =
{
	"..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX"
	"..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X"
	"---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X"
	"X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X"
	"XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X"
	"X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X"
	"X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX"
	"X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      "
	"X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       "
	"X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        "
	"X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         "
	"X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          "
	"X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           "
	"X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            "
	"X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           "
	"X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          "
	"X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          "
	"X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       ------------------------------------"
	"X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           "
	"XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           "
	"      X..X          -  X...X  -         X...X         -  X..X           X..X  -           "
	"       XX           -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           "
	"------------        -    X    -           X           -X.....................X-           "
	"                    ----------------------------------- X...XXXXXXXXXXXXX...X -           "
	"                                                      -  X..X           X..X  -           "
	"                                                      -   X.X           X.X   -           "
	"                                                      -    XX           XX    -           "
};

static const ImVec2 FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[ImGuiMouseCursor_Count_][3] =
{
	// Pos ........ Size ......... Offset ......
	{ ImVec2(0,3),  ImVec2(12,19), ImVec2(0, 0) }, // ImGuiMouseCursor_Arrow
	{ ImVec2(13,0), ImVec2(7,16),  ImVec2(4, 8) }, // ImGuiMouseCursor_TextInput
	{ ImVec2(31,0), ImVec2(23,23), ImVec2(11,11) }, // ImGuiMouseCursor_Move
	{ ImVec2(21,0), ImVec2(9,23), ImVec2(5,11) }, // ImGuiMouseCursor_ResizeNS
	{ ImVec2(55,18),ImVec2(23, 9), ImVec2(11, 5) }, // ImGuiMouseCursor_ResizeEW
	{ ImVec2(73,0), ImVec2(17,17), ImVec2(9, 9) }, // ImGuiMouseCursor_ResizeNESW
	{ ImVec2(55,0), ImVec2(17,17), ImVec2(9, 9) }, // ImGuiMouseCursor_ResizeNWSE
};

ImFontAtlas::ImFontAtlas()
{
	TexID = NULL;
	TexDesiredWidth = 0;
	TexGlyphPadding = 1;
	TexPixelsAlpha8 = NULL;
	TexPixelsRGBA32 = NULL;
	TexWidth = TexHeight = 0;
	TexUvScale = ImVec2(0.0f, 0.0f);
	TexUvWhitePixel = ImVec2(0.0f, 0.0f);
	for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
		CustomRectIds[n] = -1;
}

ImFontAtlas::~ImFontAtlas()
{
	Clear();
}

void    ImFontAtlas::ClearInputData()
{
	for (int i = 0; i < ConfigData.Size; i++)
		if (ConfigData[i].FontData && ConfigData[i].FontDataOwnedByAtlas)
		{
			ImGui::MemFree(ConfigData[i].FontData);
			ConfigData[i].FontData = NULL;
		}

	// When clearing this we lose access to  the font name and other information used to build the font.
	for (int i = 0; i < Fonts.Size; i++)
		if (Fonts[i]->ConfigData >= ConfigData.Data && Fonts[i]->ConfigData < ConfigData.Data + ConfigData.Size)
		{
			Fonts[i]->ConfigData = NULL;
			Fonts[i]->ConfigDataCount = 0;
		}
	ConfigData.clear();
	CustomRects.clear();
	for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
		CustomRectIds[n] = -1;
}

void    ImFontAtlas::ClearTexData()
{
	if (TexPixelsAlpha8)
		ImGui::MemFree(TexPixelsAlpha8);
	if (TexPixelsRGBA32)
		ImGui::MemFree(TexPixelsRGBA32);
	TexPixelsAlpha8 = NULL;
	TexPixelsRGBA32 = NULL;
}

void    ImFontAtlas::ClearFonts()
{
	for (int i = 0; i < Fonts.Size; i++)
		IM_DELETE(Fonts[i]);
	Fonts.clear();
}

void    ImFontAtlas::Clear()
{
	ClearInputData();
	ClearTexData();
	ClearFonts();
}

void    ImFontAtlas::GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
	// Build atlas on demand
	if (TexPixelsAlpha8 == NULL)
	{
		if (ConfigData.empty())
			AddFontDefault();
		Build();
	}

	*out_pixels = TexPixelsAlpha8;
	if (out_width) *out_width = TexWidth;
	if (out_height) *out_height = TexHeight;
	if (out_bytes_per_pixel) *out_bytes_per_pixel = 1;
}

void    ImFontAtlas::GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
	// Convert to RGBA32 format on demand
	// Although it is likely to be the most commonly used format, our font rendering is 1 channel / 8 bpp
	if (!TexPixelsRGBA32)
	{
		unsigned char* pixels = NULL;
		GetTexDataAsAlpha8(&pixels, NULL, NULL);
		if (pixels)
		{
			TexPixelsRGBA32 = (unsigned int*)ImGui::MemAlloc((size_t)(TexWidth * TexHeight * 4));
			const unsigned char* src = pixels;
			unsigned int* dst = TexPixelsRGBA32;
			for (int n = TexWidth * TexHeight; n > 0; n--)
				*dst++ = IM_COL32(255, 255, 255, (unsigned int)(*src++));
		}
	}

	*out_pixels = (unsigned char*)TexPixelsRGBA32;
	if (out_width) *out_width = TexWidth;
	if (out_height) *out_height = TexHeight;
	if (out_bytes_per_pixel) *out_bytes_per_pixel = 4;
}

ImFont* ImFontAtlas::AddFont(const ImFontConfig* font_cfg)
{
	IM_ASSERT(font_cfg->FontData != NULL && font_cfg->FontDataSize > 0);
	IM_ASSERT(font_cfg->SizePixels > 0.0f);

	// Create new font
	if (!font_cfg->MergeMode)
		Fonts.push_back(IM_NEW(ImFont));
	else
		IM_ASSERT(!Fonts.empty()); // When using MergeMode make sure that a font has already been added before. You can use ImGui::GetIO().Fonts->AddFontDefault() to add the default imgui font.

	ConfigData.push_back(*font_cfg);
	ImFontConfig& new_font_cfg = ConfigData.back();
	if (!new_font_cfg.DstFont)
		new_font_cfg.DstFont = Fonts.back();
	if (!new_font_cfg.FontDataOwnedByAtlas)
	{
		new_font_cfg.FontData = ImGui::MemAlloc(new_font_cfg.FontDataSize);
		new_font_cfg.FontDataOwnedByAtlas = true;
		memcpy(new_font_cfg.FontData, font_cfg->FontData, (size_t)new_font_cfg.FontDataSize);
	}

	// Invalidate texture
	ClearTexData();
	return new_font_cfg.DstFont;
}

// Default font TTF is compressed with stb_compress then base85 encoded (see misc/fonts/binary_to_compressed_c.cpp for encoder)
static unsigned int stb_decompress_length(unsigned char *input);
static unsigned int stb_decompress(unsigned char *output, unsigned char *i, unsigned int length);
static const char*  GetDefaultCompressedFontDataTTFBase85();
static unsigned int Decode85Byte(char c) { return c >= '\\' ? c - 36 : c - 35; }
static void         Decode85(const unsigned char* src, unsigned char* dst)
{
	while (*src)
	{
		unsigned int tmp = Decode85Byte(src[0]) + 85 * (Decode85Byte(src[1]) + 85 * (Decode85Byte(src[2]) + 85 * (Decode85Byte(src[3]) + 85 * Decode85Byte(src[4]))));
		dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
		src += 5;
		dst += 4;
	}
}

// Load embedded ProggyClean.ttf at size 13, disable oversampling
ImFont* ImFontAtlas::AddFontDefault(const ImFontConfig* font_cfg_template)
{
	ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
	if (!font_cfg_template)
	{
		font_cfg.OversampleH = font_cfg.OversampleV = 1;
		font_cfg.PixelSnapH = true;
	}
	if (font_cfg.Name[0] == '\0') strcpy(font_cfg.Name, "ProggyClean.ttf, 13px");
	if (font_cfg.SizePixels <= 0.0f) font_cfg.SizePixels = 13.0f;

	const char* ttf_compressed_base85 = GetDefaultCompressedFontDataTTFBase85();
	ImFont* font = AddFontFromMemoryCompressedBase85TTF(ttf_compressed_base85, font_cfg.SizePixels, &font_cfg, GetGlyphRangesDefault());
	return font;
}

ImFont* ImFontAtlas::AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
	int data_size = 0;
	void* data = ImFileLoadToMemory(filename, "rb", &data_size, 0);
	if (!data)
	{
		IM_ASSERT(0); // Could not load file.
		return NULL;
	}
	ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
	if (font_cfg.Name[0] == '\0')
	{
		// Store a short copy of filename into into the font name for convenience
		const char* p;
		for (p = filename + strlen(filename); p > filename && p[-1] != '/' && p[-1] != '\\'; p--) {}
		snprintf(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "%s, %.0fpx", p, size_pixels);
	}
	return AddFontFromMemoryTTF(data, data_size, size_pixels, &font_cfg, glyph_ranges);
}

// NB: Transfer ownership of 'ttf_data' to ImFontAtlas, unless font_cfg_template->FontDataOwnedByAtlas == false. Owned TTF buffer will be deleted after Build().
ImFont* ImFontAtlas::AddFontFromMemoryTTF(void* ttf_data, int ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
	ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
	IM_ASSERT(font_cfg.FontData == NULL);
	font_cfg.FontData = ttf_data;
	font_cfg.FontDataSize = ttf_size;
	font_cfg.SizePixels = size_pixels;
	if (glyph_ranges)
		font_cfg.GlyphRanges = glyph_ranges;
	return AddFont(&font_cfg);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedTTF(const void* compressed_ttf_data, int compressed_ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
	const unsigned int buf_decompressed_size = stb_decompress_length((unsigned char*)compressed_ttf_data);
	unsigned char* buf_decompressed_data = (unsigned char *)ImGui::MemAlloc(buf_decompressed_size);
	stb_decompress(buf_decompressed_data, (unsigned char*)compressed_ttf_data, (unsigned int)compressed_ttf_size);

	ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
	IM_ASSERT(font_cfg.FontData == NULL);
	font_cfg.FontDataOwnedByAtlas = true;
	return AddFontFromMemoryTTF(buf_decompressed_data, (int)buf_decompressed_size, size_pixels, &font_cfg, glyph_ranges);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedBase85TTF(const char* compressed_ttf_data_base85, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges)
{
	int compressed_ttf_size = (((int)strlen(compressed_ttf_data_base85) + 4) / 5) * 4;
	void* compressed_ttf = ImGui::MemAlloc((size_t)compressed_ttf_size);
	Decode85((const unsigned char*)compressed_ttf_data_base85, (unsigned char*)compressed_ttf);
	ImFont* font = AddFontFromMemoryCompressedTTF(compressed_ttf, compressed_ttf_size, size_pixels, font_cfg, glyph_ranges);
	ImGui::MemFree(compressed_ttf);
	return font;
}

int ImFontAtlas::AddCustomRectRegular(unsigned int id, int width, int height)
{
	IM_ASSERT(id >= 0x10000);
	IM_ASSERT(width > 0 && width <= 0xFFFF);
	IM_ASSERT(height > 0 && height <= 0xFFFF);
	CustomRect r;
	r.ID = id;
	r.Width = (unsigned short)width;
	r.Height = (unsigned short)height;
	CustomRects.push_back(r);
	return CustomRects.Size - 1; // Return index
}

int ImFontAtlas::AddCustomRectFontGlyph(ImFont* font, ImWchar id, int width, int height, float advance_x, const ImVec2& offset)
{
	IM_ASSERT(font != NULL);
	IM_ASSERT(width > 0 && width <= 0xFFFF);
	IM_ASSERT(height > 0 && height <= 0xFFFF);
	CustomRect r;
	r.ID = id;
	r.Width = (unsigned short)width;
	r.Height = (unsigned short)height;
	r.GlyphAdvanceX = advance_x;
	r.GlyphOffset = offset;
	r.Font = font;
	CustomRects.push_back(r);
	return CustomRects.Size - 1; // Return index
}

void ImFontAtlas::CalcCustomRectUV(const CustomRect* rect, ImVec2* out_uv_min, ImVec2* out_uv_max)
{
	IM_ASSERT(TexWidth > 0 && TexHeight > 0);   // Font atlas needs to be built before we can calculate UV coordinates
	IM_ASSERT(rect->IsPacked());                // Make sure the rectangle has been packed
	*out_uv_min = ImVec2((float)rect->X * TexUvScale.x, (float)rect->Y * TexUvScale.y);
	*out_uv_max = ImVec2((float)(rect->X + rect->Width) * TexUvScale.x, (float)(rect->Y + rect->Height) * TexUvScale.y);
}

bool ImFontAtlas::GetMouseCursorTexData(ImGuiMouseCursor cursor_type, ImVec2* out_offset, ImVec2* out_size, ImVec2 out_uv_border[2], ImVec2 out_uv_fill[2])
{
	if (cursor_type <= ImGuiMouseCursor_None || cursor_type >= ImGuiMouseCursor_Count_)
		return false;

	ImFontAtlas::CustomRect& r = CustomRects[CustomRectIds[0]];
	IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
	ImVec2 pos = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][0] + ImVec2((float)r.X, (float)r.Y);
	ImVec2 size = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][1];
	*out_size = size;
	*out_offset = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][2];
	out_uv_border[0] = (pos)* TexUvScale;
	out_uv_border[1] = (pos + size) * TexUvScale;
	pos.x += FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
	out_uv_fill[0] = (pos)* TexUvScale;
	out_uv_fill[1] = (pos + size) * TexUvScale;
	return true;
}

bool    ImFontAtlas::Build()
{
	return ImFontAtlasBuildWithStbTruetype(this);
}

void    ImFontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256], float in_brighten_factor)
{
	for (unsigned int i = 0; i < 256; i++)
	{
		unsigned int value = (unsigned int)(i * in_brighten_factor);
		out_table[i] = value > 255 ? 255 : (value & 0xFF);
	}
}

void    ImFontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256], unsigned char* pixels, int x, int y, int w, int h, int stride)
{
	unsigned char* data = pixels + x + y * stride;
	for (int j = h; j > 0; j--, data += stride)
		for (int i = 0; i < w; i++)
			data[i] = table[data[i]];
}

bool    ImFontAtlasBuildWithStbTruetype(ImFontAtlas* atlas)
{
	IM_ASSERT(atlas->ConfigData.Size > 0);

	ImFontAtlasBuildRegisterDefaultCustomRects(atlas);

	atlas->TexID = NULL;
	atlas->TexWidth = atlas->TexHeight = 0;
	atlas->TexUvScale = ImVec2(0.0f, 0.0f);
	atlas->TexUvWhitePixel = ImVec2(0.0f, 0.0f);
	atlas->ClearTexData();

	// Count glyphs/ranges
	int total_glyphs_count = 0;
	int total_ranges_count = 0;
	for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
	{
		ImFontConfig& cfg = atlas->ConfigData[input_i];
		if (!cfg.GlyphRanges)
			cfg.GlyphRanges = atlas->GetGlyphRangesDefault();
		for (const ImWchar* in_range = cfg.GlyphRanges; in_range[0] && in_range[1]; in_range += 2, total_ranges_count++)
			total_glyphs_count += (in_range[1] - in_range[0]) + 1;
	}

	// We need a width for the skyline algorithm. Using a dumb heuristic here to decide of width. User can override TexDesiredWidth and TexGlyphPadding if they wish.
	// Width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
	atlas->TexWidth = (atlas->TexDesiredWidth > 0) ? atlas->TexDesiredWidth : (total_glyphs_count > 4000) ? 4096 : (total_glyphs_count > 2000) ? 2048 : (total_glyphs_count > 1000) ? 1024 : 512;
	atlas->TexHeight = 0;

	// Start packing
	const int max_tex_height = 1024 * 32;
	stbtt_pack_context spc = {};
	stbtt_PackBegin(&spc, NULL, atlas->TexWidth, max_tex_height, 0, atlas->TexGlyphPadding, NULL);
	stbtt_PackSetOversampling(&spc, 1, 1);

	// Pack our extra data rectangles first, so it will be on the upper-left corner of our texture (UV will have small values).
	ImFontAtlasBuildPackCustomRects(atlas, spc.pack_info);

	// Initialize font information (so we can error without any cleanup)
	struct ImFontTempBuildData
	{
		stbtt_fontinfo      FontInfo;
		stbrp_rect*         Rects;
		int                 RectsCount;
		stbtt_pack_range*   Ranges;
		int                 RangesCount;
	};
	ImFontTempBuildData* tmp_array = (ImFontTempBuildData*)ImGui::MemAlloc((size_t)atlas->ConfigData.Size * sizeof(ImFontTempBuildData));
	for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
	{
		ImFontConfig& cfg = atlas->ConfigData[input_i];
		ImFontTempBuildData& tmp = tmp_array[input_i];
		IM_ASSERT(cfg.DstFont && (!cfg.DstFont->IsLoaded() || cfg.DstFont->ContainerAtlas == atlas));

		const int font_offset = stbtt_GetFontOffsetForIndex((unsigned char*)cfg.FontData, cfg.FontNo);
		IM_ASSERT(font_offset >= 0);
		if (!stbtt_InitFont(&tmp.FontInfo, (unsigned char*)cfg.FontData, font_offset))
		{
			atlas->TexWidth = atlas->TexHeight = 0; // Reset output on failure
			ImGui::MemFree(tmp_array);
			return false;
		}
	}

	// Allocate packing character data and flag packed characters buffer as non-packed (x0=y0=x1=y1=0)
	int buf_packedchars_n = 0, buf_rects_n = 0, buf_ranges_n = 0;
	stbtt_packedchar* buf_packedchars = (stbtt_packedchar*)ImGui::MemAlloc(total_glyphs_count * sizeof(stbtt_packedchar));
	stbrp_rect* buf_rects = (stbrp_rect*)ImGui::MemAlloc(total_glyphs_count * sizeof(stbrp_rect));
	stbtt_pack_range* buf_ranges = (stbtt_pack_range*)ImGui::MemAlloc(total_ranges_count * sizeof(stbtt_pack_range));
	memset(buf_packedchars, 0, total_glyphs_count * sizeof(stbtt_packedchar));
	memset(buf_rects, 0, total_glyphs_count * sizeof(stbrp_rect));              // Unnecessary but let's clear this for the sake of sanity.
	memset(buf_ranges, 0, total_ranges_count * sizeof(stbtt_pack_range));

	// First font pass: pack all glyphs (no rendering at this point, we are working with rectangles in an infinitely tall texture at this point)
	for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
	{
		ImFontConfig& cfg = atlas->ConfigData[input_i];
		ImFontTempBuildData& tmp = tmp_array[input_i];

		// Setup ranges
		int font_glyphs_count = 0;
		int font_ranges_count = 0;
		for (const ImWchar* in_range = cfg.GlyphRanges; in_range[0] && in_range[1]; in_range += 2, font_ranges_count++)
			font_glyphs_count += (in_range[1] - in_range[0]) + 1;
		tmp.Ranges = buf_ranges + buf_ranges_n;
		tmp.RangesCount = font_ranges_count;
		buf_ranges_n += font_ranges_count;
		for (int i = 0; i < font_ranges_count; i++)
		{
			const ImWchar* in_range = &cfg.GlyphRanges[i * 2];
			stbtt_pack_range& range = tmp.Ranges[i];
			range.font_size = cfg.SizePixels;
			range.first_unicode_codepoint_in_range = in_range[0];
			range.num_chars = (in_range[1] - in_range[0]) + 1;
			range.chardata_for_range = buf_packedchars + buf_packedchars_n;
			buf_packedchars_n += range.num_chars;
		}

		// Pack
		tmp.Rects = buf_rects + buf_rects_n;
		tmp.RectsCount = font_glyphs_count;
		buf_rects_n += font_glyphs_count;
		stbtt_PackSetOversampling(&spc, cfg.OversampleH, cfg.OversampleV);
		int n = stbtt_PackFontRangesGatherRects(&spc, &tmp.FontInfo, tmp.Ranges, tmp.RangesCount, tmp.Rects);
		IM_ASSERT(n == font_glyphs_count);
		stbrp_pack_rects((stbrp_context*)spc.pack_info, tmp.Rects, n);

		// Extend texture height
		for (int i = 0; i < n; i++)
			if (tmp.Rects[i].was_packed)
				atlas->TexHeight = ImMax(atlas->TexHeight, tmp.Rects[i].y + tmp.Rects[i].h);
	}
	IM_ASSERT(buf_rects_n == total_glyphs_count);
	IM_ASSERT(buf_packedchars_n == total_glyphs_count);
	IM_ASSERT(buf_ranges_n == total_ranges_count);

	// Create texture
	atlas->TexHeight = ImUpperPowerOfTwo(atlas->TexHeight);
	atlas->TexUvScale = ImVec2(1.0f / atlas->TexWidth, 1.0f / atlas->TexHeight);
	atlas->TexPixelsAlpha8 = (unsigned char*)ImGui::MemAlloc(atlas->TexWidth * atlas->TexHeight);
	memset(atlas->TexPixelsAlpha8, 0, atlas->TexWidth * atlas->TexHeight);
	spc.pixels = atlas->TexPixelsAlpha8;
	spc.height = atlas->TexHeight;

	// Second pass: render font characters
	for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
	{
		ImFontConfig& cfg = atlas->ConfigData[input_i];
		ImFontTempBuildData& tmp = tmp_array[input_i];
		stbtt_PackSetOversampling(&spc, cfg.OversampleH, cfg.OversampleV);
		stbtt_PackFontRangesRenderIntoRects(&spc, &tmp.FontInfo, tmp.Ranges, tmp.RangesCount, tmp.Rects);
		if (cfg.RasterizerMultiply != 1.0f)
		{
			unsigned char multiply_table[256];
			ImFontAtlasBuildMultiplyCalcLookupTable(multiply_table, cfg.RasterizerMultiply);
			for (const stbrp_rect* r = tmp.Rects; r != tmp.Rects + tmp.RectsCount; r++)
				if (r->was_packed)
					ImFontAtlasBuildMultiplyRectAlpha8(multiply_table, spc.pixels, r->x, r->y, r->w, r->h, spc.stride_in_bytes);
		}
		tmp.Rects = NULL;
	}

	// End packing
	stbtt_PackEnd(&spc);
	ImGui::MemFree(buf_rects);
	buf_rects = NULL;

	// Third pass: setup ImFont and glyphs for runtime
	for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
	{
		ImFontConfig& cfg = atlas->ConfigData[input_i];
		ImFontTempBuildData& tmp = tmp_array[input_i];
		ImFont* dst_font = cfg.DstFont; // We can have multiple input fonts writing into a same destination font (when using MergeMode=true)

		const float font_scale = stbtt_ScaleForPixelHeight(&tmp.FontInfo, cfg.SizePixels);
		int unscaled_ascent, unscaled_descent, unscaled_line_gap;
		stbtt_GetFontVMetrics(&tmp.FontInfo, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

		const float ascent = unscaled_ascent * font_scale;
		const float descent = unscaled_descent * font_scale;
		ImFontAtlasBuildSetupFont(atlas, dst_font, &cfg, ascent, descent);
		const float off_x = cfg.GlyphOffset.x;
		const float off_y = cfg.GlyphOffset.y + (float)(int)(dst_font->Ascent + 0.5f);

		for (int i = 0; i < tmp.RangesCount; i++)
		{
			stbtt_pack_range& range = tmp.Ranges[i];
			for (int char_idx = 0; char_idx < range.num_chars; char_idx += 1)
			{
				const stbtt_packedchar& pc = range.chardata_for_range[char_idx];
				if (!pc.x0 && !pc.x1 && !pc.y0 && !pc.y1)
					continue;

				const int codepoint = range.first_unicode_codepoint_in_range + char_idx;
				if (cfg.MergeMode && dst_font->FindGlyph((unsigned short)codepoint))
					continue;

				stbtt_aligned_quad q;
				float dummy_x = 0.0f, dummy_y = 0.0f;
				stbtt_GetPackedQuad(range.chardata_for_range, atlas->TexWidth, atlas->TexHeight, char_idx, &dummy_x, &dummy_y, &q, 0);
				dst_font->AddGlyph((ImWchar)codepoint, q.x0 + off_x, q.y0 + off_y, q.x1 + off_x, q.y1 + off_y, q.s0, q.t0, q.s1, q.t1, pc.xadvance);
			}
		}
	}

	// Cleanup temporaries
	ImGui::MemFree(buf_packedchars);
	ImGui::MemFree(buf_ranges);
	ImGui::MemFree(tmp_array);

	ImFontAtlasBuildFinish(atlas);

	return true;
}

void ImFontAtlasBuildRegisterDefaultCustomRects(ImFontAtlas* atlas)
{
	if (atlas->CustomRectIds[0] < 0)
		atlas->CustomRectIds[0] = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_ID, FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * 2 + 1, FONT_ATLAS_DEFAULT_TEX_DATA_H);
}

void ImFontAtlasBuildSetupFont(ImFontAtlas* atlas, ImFont* font, ImFontConfig* font_config, float ascent, float descent)
{
	if (!font_config->MergeMode)
	{
		font->ClearOutputData();
		font->FontSize = font_config->SizePixels;
		font->ConfigData = font_config;
		font->ContainerAtlas = atlas;
		font->Ascent = ascent;
		font->Descent = descent;
	}
	font->ConfigDataCount++;
}

void ImFontAtlasBuildPackCustomRects(ImFontAtlas* atlas, void* pack_context_opaque)
{
	stbrp_context* pack_context = (stbrp_context*)pack_context_opaque;

	ImVector<ImFontAtlas::CustomRect>& user_rects = atlas->CustomRects;
	IM_ASSERT(user_rects.Size >= 1); // We expect at least the default custom rects to be registered, else something went wrong.

	ImVector<stbrp_rect> pack_rects;
	pack_rects.resize(user_rects.Size);
	memset(pack_rects.Data, 0, sizeof(stbrp_rect) * user_rects.Size);
	for (int i = 0; i < user_rects.Size; i++)
	{
		pack_rects[i].w = user_rects[i].Width;
		pack_rects[i].h = user_rects[i].Height;
	}
	stbrp_pack_rects(pack_context, &pack_rects[0], pack_rects.Size);
	for (int i = 0; i < pack_rects.Size; i++)
		if (pack_rects[i].was_packed)
		{
			user_rects[i].X = pack_rects[i].x;
			user_rects[i].Y = pack_rects[i].y;
			IM_ASSERT(pack_rects[i].w == user_rects[i].Width && pack_rects[i].h == user_rects[i].Height);
			atlas->TexHeight = ImMax(atlas->TexHeight, pack_rects[i].y + pack_rects[i].h);
		}
}

static void ImFontAtlasBuildRenderDefaultTexData(ImFontAtlas* atlas)
{
	IM_ASSERT(atlas->CustomRectIds[0] >= 0);
	ImFontAtlas::CustomRect& r = atlas->CustomRects[atlas->CustomRectIds[0]];
	IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
	IM_ASSERT(r.Width == FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * 2 + 1);
	IM_ASSERT(r.Height == FONT_ATLAS_DEFAULT_TEX_DATA_H);
	IM_ASSERT(r.IsPacked());
	IM_ASSERT(atlas->TexPixelsAlpha8 != NULL);

	// Render/copy pixels
	for (int y = 0, n = 0; y < FONT_ATLAS_DEFAULT_TEX_DATA_H; y++)
		for (int x = 0; x < FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF; x++, n++)
		{
			const int offset0 = (int)(r.X + x) + (int)(r.Y + y) * atlas->TexWidth;
			const int offset1 = offset0 + FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
			atlas->TexPixelsAlpha8[offset0] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == '.' ? 0xFF : 0x00;
			atlas->TexPixelsAlpha8[offset1] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == 'X' ? 0xFF : 0x00;
		}
	atlas->TexUvWhitePixel = ImVec2((r.X + 0.5f) * atlas->TexUvScale.x, (r.Y + 0.5f) * atlas->TexUvScale.y);
}

void ImFontAtlasBuildFinish(ImFontAtlas* atlas)
{
	// Render into our custom data block
	ImFontAtlasBuildRenderDefaultTexData(atlas);

	// Register custom rectangle glyphs
	for (int i = 0; i < atlas->CustomRects.Size; i++)
	{
		const ImFontAtlas::CustomRect& r = atlas->CustomRects[i];
		if (r.Font == NULL || r.ID > 0x10000)
			continue;

		IM_ASSERT(r.Font->ContainerAtlas == atlas);
		ImVec2 uv0, uv1;
		atlas->CalcCustomRectUV(&r, &uv0, &uv1);
		r.Font->AddGlyph((ImWchar)r.ID, r.GlyphOffset.x, r.GlyphOffset.y, r.GlyphOffset.x + r.Width, r.GlyphOffset.y + r.Height, uv0.x, uv0.y, uv1.x, uv1.y, r.GlyphAdvanceX);
	}

	// Build all fonts lookup tables
	for (int i = 0; i < atlas->Fonts.Size; i++)
		atlas->Fonts[i]->BuildLookupTable();
}

// Retrieve list of range (2 int per range, values are inclusive)
const ImWchar*   ImFontAtlas::GetGlyphRangesDefault()
{
	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0,
	};
	return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesKorean()
{
	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x3131, 0x3163, // Korean alphabets
		0xAC00, 0xD79D, // Korean characters
		0,
	};
	return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesChinese()
{
	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x3000, 0x30FF, // Punctuations, Hiragana, Katakana
		0x31F0, 0x31FF, // Katakana Phonetic Extensions
		0xFF00, 0xFFEF, // Half-width characters
		0x4e00, 0x9FAF, // CJK Ideograms
		0,
	};
	return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesJapanese()
{
	// Store the 1946 ideograms code points as successive offsets from the initial unicode codepoint 0x4E00. Each offset has an implicit +1.
	// This encoding is designed to helps us reduce the source code size.
	// FIXME: Source a list of the revised 2136 joyo kanji list from 2010 and rebuild this.
	// The current list was sourced from http://theinstructionlimit.com/author/renaudbedardrenaudbedard/page/3
	// Note that you may use ImFontAtlas::GlyphRangesBuilder to create your own ranges, by merging existing ranges or adding new characters.
	static const short offsets_from_0x4E00[] =
	{
		-1,0,1,3,0,0,0,0,1,0,5,1,1,0,7,4,6,10,0,1,9,9,7,1,3,19,1,10,7,1,0,1,0,5,1,0,6,4,2,6,0,0,12,6,8,0,3,5,0,1,0,9,0,0,8,1,1,3,4,5,13,0,0,8,2,17,
		4,3,1,1,9,6,0,0,0,2,1,3,2,22,1,9,11,1,13,1,3,12,0,5,9,2,0,6,12,5,3,12,4,1,2,16,1,1,4,6,5,3,0,6,13,15,5,12,8,14,0,0,6,15,3,6,0,18,8,1,6,14,1,
		5,4,12,24,3,13,12,10,24,0,0,0,1,0,1,1,2,9,10,2,2,0,0,3,3,1,0,3,8,0,3,2,4,4,1,6,11,10,14,6,15,3,4,15,1,0,0,5,2,2,0,0,1,6,5,5,6,0,3,6,5,0,0,1,0,
		11,2,2,8,4,7,0,10,0,1,2,17,19,3,0,2,5,0,6,2,4,4,6,1,1,11,2,0,3,1,2,1,2,10,7,6,3,16,0,8,24,0,0,3,1,1,3,0,1,6,0,0,0,2,0,1,5,15,0,1,0,0,2,11,19,
		1,4,19,7,6,5,1,0,0,0,0,5,1,0,1,9,0,0,5,0,2,0,1,0,3,0,11,3,0,2,0,0,0,0,0,9,3,6,4,12,0,14,0,0,29,10,8,0,14,37,13,0,31,16,19,0,8,30,1,20,8,3,48,
		21,1,0,12,0,10,44,34,42,54,11,18,82,0,2,1,2,12,1,0,6,2,17,2,12,7,0,7,17,4,2,6,24,23,8,23,39,2,16,23,1,0,5,1,2,15,14,5,6,2,11,0,8,6,2,2,2,14,
		20,4,15,3,4,11,10,10,2,5,2,1,30,2,1,0,0,22,5,5,0,3,1,5,4,1,0,0,2,2,21,1,5,1,2,16,2,1,3,4,0,8,4,0,0,5,14,11,2,16,1,13,1,7,0,22,15,3,1,22,7,14,
		22,19,11,24,18,46,10,20,64,45,3,2,0,4,5,0,1,4,25,1,0,0,2,10,0,0,0,1,0,1,2,0,0,9,1,2,0,0,0,2,5,2,1,1,5,5,8,1,1,1,5,1,4,9,1,3,0,1,0,1,1,2,0,0,
		2,0,1,8,22,8,1,0,0,0,0,4,2,1,0,9,8,5,0,9,1,30,24,2,6,4,39,0,14,5,16,6,26,179,0,2,1,1,0,0,0,5,2,9,6,0,2,5,16,7,5,1,1,0,2,4,4,7,15,13,14,0,0,
		3,0,1,0,0,0,2,1,6,4,5,1,4,9,0,3,1,8,0,0,10,5,0,43,0,2,6,8,4,0,2,0,0,9,6,0,9,3,1,6,20,14,6,1,4,0,7,2,3,0,2,0,5,0,3,1,0,3,9,7,0,3,4,0,4,9,1,6,0,
		9,0,0,2,3,10,9,28,3,6,2,4,1,2,32,4,1,18,2,0,3,1,5,30,10,0,2,2,2,0,7,9,8,11,10,11,7,2,13,7,5,10,0,3,40,2,0,1,6,12,0,4,5,1,5,11,11,21,4,8,3,7,
		8,8,33,5,23,0,0,19,8,8,2,3,0,6,1,1,1,5,1,27,4,2,5,0,3,5,6,3,1,0,3,1,12,5,3,3,2,0,7,7,2,1,0,4,0,1,1,2,0,10,10,6,2,5,9,7,5,15,15,21,6,11,5,20,
		4,3,5,5,2,5,0,2,1,0,1,7,28,0,9,0,5,12,5,5,18,30,0,12,3,3,21,16,25,32,9,3,14,11,24,5,66,9,1,2,0,5,9,1,5,1,8,0,8,3,3,0,1,15,1,4,8,1,2,7,0,7,2,
		8,3,7,5,3,7,10,2,1,0,0,2,25,0,6,4,0,10,0,4,2,4,1,12,5,38,4,0,4,1,10,5,9,4,0,14,4,2,5,18,20,21,1,3,0,5,0,7,0,3,7,1,3,1,1,8,1,0,0,0,3,2,5,2,11,
		6,0,13,1,3,9,1,12,0,16,6,2,1,0,2,1,12,6,13,11,2,0,28,1,7,8,14,13,8,13,0,2,0,5,4,8,10,2,37,42,19,6,6,7,4,14,11,18,14,80,7,6,0,4,72,12,36,27,
		7,7,0,14,17,19,164,27,0,5,10,7,3,13,6,14,0,2,2,5,3,0,6,13,0,0,10,29,0,4,0,3,13,0,3,1,6,51,1,5,28,2,0,8,0,20,2,4,0,25,2,10,13,10,0,16,4,0,1,0,
		2,1,7,0,1,8,11,0,0,1,2,7,2,23,11,6,6,4,16,2,2,2,0,22,9,3,3,5,2,0,15,16,21,2,9,20,15,15,5,3,9,1,0,0,1,7,7,5,4,2,2,2,38,24,14,0,0,15,5,6,24,14,
		5,5,11,0,21,12,0,3,8,4,11,1,8,0,11,27,7,2,4,9,21,59,0,1,39,3,60,62,3,0,12,11,0,3,30,11,0,13,88,4,15,5,28,13,1,4,48,17,17,4,28,32,46,0,16,0,
		18,11,1,8,6,38,11,2,6,11,38,2,0,45,3,11,2,7,8,4,30,14,17,2,1,1,65,18,12,16,4,2,45,123,12,56,33,1,4,3,4,7,0,0,0,3,2,0,16,4,2,4,2,0,7,4,5,2,26,
		2,25,6,11,6,1,16,2,6,17,77,15,3,35,0,1,0,5,1,0,38,16,6,3,12,3,3,3,0,9,3,1,3,5,2,9,0,18,0,25,1,3,32,1,72,46,6,2,7,1,3,14,17,0,28,1,40,13,0,20,
		15,40,6,38,24,12,43,1,1,9,0,12,6,0,6,2,4,19,3,7,1,48,0,9,5,0,5,6,9,6,10,15,2,11,19,3,9,2,0,1,10,1,27,8,1,3,6,1,14,0,26,0,27,16,3,4,9,6,2,23,
		9,10,5,25,2,1,6,1,1,48,15,9,15,14,3,4,26,60,29,13,37,21,1,6,4,0,2,11,22,23,16,16,2,2,1,3,0,5,1,6,4,0,0,4,0,0,8,3,0,2,5,0,7,1,7,3,13,2,4,10,
		3,0,2,31,0,18,3,0,12,10,4,1,0,7,5,7,0,5,4,12,2,22,10,4,2,15,2,8,9,0,23,2,197,51,3,1,1,4,13,4,3,21,4,19,3,10,5,40,0,4,1,1,10,4,1,27,34,7,21,
		2,17,2,9,6,4,2,3,0,4,2,7,8,2,5,1,15,21,3,4,4,2,2,17,22,1,5,22,4,26,7,0,32,1,11,42,15,4,1,2,5,0,19,3,1,8,6,0,10,1,9,2,13,30,8,2,24,17,19,1,4,
		4,25,13,0,10,16,11,39,18,8,5,30,82,1,6,8,18,77,11,13,20,75,11,112,78,33,3,0,0,60,17,84,9,1,1,12,30,10,49,5,32,158,178,5,5,6,3,3,1,3,1,4,7,6,
		19,31,21,0,2,9,5,6,27,4,9,8,1,76,18,12,1,4,0,3,3,6,3,12,2,8,30,16,2,25,1,5,5,4,3,0,6,10,2,3,1,0,5,1,19,3,0,8,1,5,2,6,0,0,0,19,1,2,0,5,1,2,5,
		1,3,7,0,4,12,7,3,10,22,0,9,5,1,0,2,20,1,1,3,23,30,3,9,9,1,4,191,14,3,15,6,8,50,0,1,0,0,4,0,0,1,0,2,4,2,0,2,3,0,2,0,2,2,8,7,0,1,1,1,3,3,17,11,
		91,1,9,3,2,13,4,24,15,41,3,13,3,1,20,4,125,29,30,1,0,4,12,2,21,4,5,5,19,11,0,13,11,86,2,18,0,7,1,8,8,2,2,22,1,2,6,5,2,0,1,2,8,0,2,0,5,2,1,0,
		2,10,2,0,5,9,2,1,2,0,1,0,4,0,0,10,2,5,3,0,6,1,0,1,4,4,33,3,13,17,3,18,6,4,7,1,5,78,0,4,1,13,7,1,8,1,0,35,27,15,3,0,0,0,1,11,5,41,38,15,22,6,
		14,14,2,1,11,6,20,63,5,8,27,7,11,2,2,40,58,23,50,54,56,293,8,8,1,5,1,14,0,1,12,37,89,8,8,8,2,10,6,0,0,0,4,5,2,1,0,1,1,2,7,0,3,3,0,4,6,0,3,2,
		19,3,8,0,0,0,4,4,16,0,4,1,5,1,3,0,3,4,6,2,17,10,10,31,6,4,3,6,10,126,7,3,2,2,0,9,0,0,5,20,13,0,15,0,6,0,2,5,8,64,50,3,2,12,2,9,0,0,11,8,20,
		109,2,18,23,0,0,9,61,3,0,28,41,77,27,19,17,81,5,2,14,5,83,57,252,14,154,263,14,20,8,13,6,57,39,38,
	};
	static ImWchar base_ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x3000, 0x30FF, // Punctuations, Hiragana, Katakana
		0x31F0, 0x31FF, // Katakana Phonetic Extensions
		0xFF00, 0xFFEF, // Half-width characters
	};
	static bool full_ranges_unpacked = false;
	static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(offsets_from_0x4E00) * 2 + 1];
	if (!full_ranges_unpacked)
	{
		// Unpack
		int codepoint = 0x4e00;
		memcpy(full_ranges, base_ranges, sizeof(base_ranges));
		ImWchar* dst = full_ranges + IM_ARRAYSIZE(base_ranges);;
		for (int n = 0; n < IM_ARRAYSIZE(offsets_from_0x4E00); n++, dst += 2)
			dst[0] = dst[1] = (ImWchar)(codepoint += (offsets_from_0x4E00[n] + 1));
		dst[0] = 0;
		full_ranges_unpacked = true;
	}
	return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesCyrillic()
{
	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
		0x2DE0, 0x2DFF, // Cyrillic Extended-A
		0xA640, 0xA69F, // Cyrillic Extended-B
		0,
	};
	return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesThai()
{
	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin
		0x2010, 0x205E, // Punctuations
		0x0E00, 0x0E7F, // Thai
		0,
	};
	return &ranges[0];
}

//-----------------------------------------------------------------------------
// ImFontAtlas::GlyphRangesBuilder
//-----------------------------------------------------------------------------

void ImFontAtlas::GlyphRangesBuilder::AddText(const char* text, const char* text_end)
{
	while (text_end ? (text < text_end) : *text)
	{
		unsigned int c = 0;
		int c_len = ImTextCharFromUtf8(&c, text, text_end);
		text += c_len;
		if (c_len == 0)
			break;
		if (c < 0x10000)
			AddChar((ImWchar)c);
	}
}

void ImFontAtlas::GlyphRangesBuilder::AddRanges(const ImWchar* ranges)
{
	for (; ranges[0]; ranges += 2)
		for (ImWchar c = ranges[0]; c <= ranges[1]; c++)
			AddChar(c);
}

void ImFontAtlas::GlyphRangesBuilder::BuildRanges(ImVector<ImWchar>* out_ranges)
{
	for (int n = 0; n < 0x10000; n++)
		if (GetBit(n))
		{
			out_ranges->push_back((ImWchar)n);
			while (n < 0x10000 && GetBit(n + 1))
				n++;
			out_ranges->push_back((ImWchar)n);
		}
	out_ranges->push_back(0);
}

//-----------------------------------------------------------------------------
// ImFont
//-----------------------------------------------------------------------------

ImFont::ImFont()
{
	Scale = 1.0f;
	FallbackChar = (ImWchar)'?';
	DisplayOffset = ImVec2(0.0f, 1.0f);
	ClearOutputData();
}

ImFont::~ImFont()
{
	// Invalidate active font so that the user gets a clear crash instead of a dangling pointer.
	// If you want to delete fonts you need to do it between Render() and NewFrame().
	// FIXME-CLEANUP
	/*
	ImGuiContext& g = *GImGui;
	if (g.Font == this)
	g.Font = NULL;
	*/
	ClearOutputData();
}

void    ImFont::ClearOutputData()
{
	FontSize = 0.0f;
	Glyphs.clear();
	IndexAdvanceX.clear();
	IndexLookup.clear();
	FallbackGlyph = NULL;
	FallbackAdvanceX = 0.0f;
	ConfigDataCount = 0;
	ConfigData = NULL;
	ContainerAtlas = NULL;
	Ascent = Descent = 0.0f;
	MetricsTotalSurface = 0;
}

void ImFont::BuildLookupTable()
{
	int max_codepoint = 0;
	for (int i = 0; i != Glyphs.Size; i++)
		max_codepoint = ImMax(max_codepoint, (int)Glyphs[i].Codepoint);

	IM_ASSERT(Glyphs.Size < 0xFFFF); // -1 is reserved
	IndexAdvanceX.clear();
	IndexLookup.clear();
	GrowIndex(max_codepoint + 1);
	for (int i = 0; i < Glyphs.Size; i++)
	{
		int codepoint = (int)Glyphs[i].Codepoint;
		IndexAdvanceX[codepoint] = Glyphs[i].AdvanceX;
		IndexLookup[codepoint] = (unsigned short)i;
	}

	// Create a glyph to handle TAB
	// FIXME: Needs proper TAB handling but it needs to be contextualized (or we could arbitrary say that each string starts at "column 0" ?)
	if (FindGlyph((unsigned short)' '))
	{
		if (Glyphs.back().Codepoint != '\t')   // So we can call this function multiple times
			Glyphs.resize(Glyphs.Size + 1);
		ImFontGlyph& tab_glyph = Glyphs.back();
		tab_glyph = *FindGlyph((unsigned short)' ');
		tab_glyph.Codepoint = '\t';
		tab_glyph.AdvanceX *= 4;
		IndexAdvanceX[(int)tab_glyph.Codepoint] = (float)tab_glyph.AdvanceX;
		IndexLookup[(int)tab_glyph.Codepoint] = (unsigned short)(Glyphs.Size - 1);
	}

	FallbackGlyph = NULL;
	FallbackGlyph = FindGlyph(FallbackChar);
	FallbackAdvanceX = FallbackGlyph ? FallbackGlyph->AdvanceX : 0.0f;
	for (int i = 0; i < max_codepoint + 1; i++)
		if (IndexAdvanceX[i] < 0.0f)
			IndexAdvanceX[i] = FallbackAdvanceX;
}

void ImFont::SetFallbackChar(ImWchar c)
{
	FallbackChar = c;
	BuildLookupTable();
}

void ImFont::GrowIndex(int new_size)
{
	IM_ASSERT(IndexAdvanceX.Size == IndexLookup.Size);
	if (new_size <= IndexLookup.Size)
		return;
	IndexAdvanceX.resize(new_size, -1.0f);
	IndexLookup.resize(new_size, (unsigned short)-1);
}

void ImFont::AddGlyph(ImWchar codepoint, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x)
{
	Glyphs.resize(Glyphs.Size + 1);
	ImFontGlyph& glyph = Glyphs.back();
	glyph.Codepoint = (ImWchar)codepoint;
	glyph.X0 = x0;
	glyph.Y0 = y0;
	glyph.X1 = x1;
	glyph.Y1 = y1;
	glyph.U0 = u0;
	glyph.V0 = v0;
	glyph.U1 = u1;
	glyph.V1 = v1;
	glyph.AdvanceX = advance_x + ConfigData->GlyphExtraSpacing.x;  // Bake spacing into AdvanceX

	if (ConfigData->PixelSnapH)
		glyph.AdvanceX = (float)(int)(glyph.AdvanceX + 0.5f);

	// Compute rough surface usage metrics (+1 to account for average padding, +0.99 to round)
	MetricsTotalSurface += (int)((glyph.U1 - glyph.U0) * ContainerAtlas->TexWidth + 1.99f) * (int)((glyph.V1 - glyph.V0) * ContainerAtlas->TexHeight + 1.99f);
}

void ImFont::AddRemapChar(ImWchar dst, ImWchar src, bool overwrite_dst)
{
	IM_ASSERT(IndexLookup.Size > 0);    // Currently this can only be called AFTER the font has been built, aka after calling ImFontAtlas::GetTexDataAs*() function.
	int index_size = IndexLookup.Size;

	if (dst < index_size && IndexLookup.Data[dst] == (unsigned short)-1 && !overwrite_dst) // 'dst' already exists
		return;
	if (src >= index_size && dst >= index_size) // both 'dst' and 'src' don't exist -> no-op
		return;

	GrowIndex(dst + 1);
	IndexLookup[dst] = (src < index_size) ? IndexLookup.Data[src] : (unsigned short)-1;
	IndexAdvanceX[dst] = (src < index_size) ? IndexAdvanceX.Data[src] : 1.0f;
}

const ImFontGlyph* ImFont::FindGlyph(ImWchar c) const
{
	if (c < IndexLookup.Size)
	{
		const unsigned short i = IndexLookup[c];
		if (i != (unsigned short)-1)
			return &Glyphs.Data[i];
	}
	return FallbackGlyph;
}

const char* ImFont::CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) const
{
	// Simple word-wrapping for English, not full-featured. Please submit failing cases!
	// FIXME: Much possible improvements (don't cut things like "word !", "word!!!" but cut within "word,,,,", more sensible support for punctuations, support for Unicode punctuations, etc.)

	// For references, possible wrap point marked with ^
	//  "aaa bbb, ccc,ddd. eee   fff. ggg!"
	//      ^    ^    ^   ^   ^__    ^    ^

	// List of hardcoded separators: .,;!?'"

	// Skip extra blanks after a line returns (that includes not counting them in width computation)
	// e.g. "Hello    world" --> "Hello" "World"

	// Cut words that cannot possibly fit within one line.
	// e.g.: "The tropical fish" with ~5 characters worth of width --> "The tr" "opical" "fish"

	float line_width = 0.0f;
	float word_width = 0.0f;
	float blank_width = 0.0f;
	wrap_width /= scale; // We work with unscaled widths to avoid scaling every characters

	const char* word_end = text;
	const char* prev_word_end = NULL;
	bool inside_word = true;

	const char* s = text;
	while (s < text_end)
	{
		unsigned int c = (unsigned int)*s;
		const char* next_s;
		if (c < 0x80)
			next_s = s + 1;
		else
			next_s = s + ImTextCharFromUtf8(&c, s, text_end);
		if (c == 0)
			break;

		if (c < 32)
		{
			if (c == '\n')
			{
				line_width = word_width = blank_width = 0.0f;
				inside_word = true;
				s = next_s;
				continue;
			}
			if (c == '\r')
			{
				s = next_s;
				continue;
			}
		}

		const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX[(int)c] : FallbackAdvanceX);
		if (ImCharIsSpace(c))
		{
			if (inside_word)
			{
				line_width += blank_width;
				blank_width = 0.0f;
				word_end = s;
			}
			blank_width += char_width;
			inside_word = false;
		}
		else
		{
			word_width += char_width;
			if (inside_word)
			{
				word_end = next_s;
			}
			else
			{
				prev_word_end = word_end;
				line_width += word_width + blank_width;
				word_width = blank_width = 0.0f;
			}

			// Allow wrapping after punctuation.
			inside_word = !(c == '.' || c == ',' || c == ';' || c == '!' || c == '?' || c == '\"');
		}

		// We ignore blank width at the end of the line (they can be skipped)
		if (line_width + word_width >= wrap_width)
		{
			// Words that cannot possibly fit within an entire line will be cut anywhere.
			if (word_width < wrap_width)
				s = prev_word_end ? prev_word_end : word_end;
			break;
		}

		s = next_s;
	}

	return s;
}

ImVec2 ImFont::CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end, const char** remaining) const
{
	if (!text_end)
		text_end = text_begin + strlen(text_begin); // FIXME-OPT: Need to avoid this.

	const float line_height = size;
	const float scale = size / FontSize;

	ImVec2 text_size = ImVec2(0, 0);
	float line_width = 0.0f;

	const bool word_wrap_enabled = (wrap_width > 0.0f);
	const char* word_wrap_eol = NULL;

	const char* s = text_begin;
	while (s < text_end)
	{
		if (word_wrap_enabled)
		{
			// Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
			if (!word_wrap_eol)
			{
				word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - line_width);
				if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
					word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
			}

			if (s >= word_wrap_eol)
			{
				if (text_size.x < line_width)
					text_size.x = line_width;
				text_size.y += line_height;
				line_width = 0.0f;
				word_wrap_eol = NULL;

				// Wrapping skips upcoming blanks
				while (s < text_end)
				{
					const char c = *s;
					if (ImCharIsSpace(c)) { s++; }
					else if (c == '\n') { s++; break; }
					else { break; }
				}
				continue;
			}
		}

		// Decode and advance source
		const char* prev_s = s;
		unsigned int c = (unsigned int)*s;
		if (c < 0x80)
		{
			s += 1;
		}
		else
		{
			s += ImTextCharFromUtf8(&c, s, text_end);
			if (c == 0) // Malformed UTF-8?
				break;
		}

		if (c < 32)
		{
			if (c == '\n')
			{
				text_size.x = ImMax(text_size.x, line_width);
				text_size.y += line_height;
				line_width = 0.0f;
				continue;
			}
			if (c == '\r')
				continue;
		}

		const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX[(int)c] : FallbackAdvanceX) * scale;
		if (line_width + char_width >= max_width)
		{
			s = prev_s;
			break;
		}

		line_width += char_width;
	}

	if (text_size.x < line_width)
		text_size.x = line_width;

	if (line_width > 0 || text_size.y == 0.0f)
		text_size.y += line_height;

	if (remaining)
		*remaining = s;

	return text_size;
}

void ImFont::RenderChar(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, unsigned short c) const
{
	if (c == ' ' || c == '\t' || c == '\n' || c == '\r') // Match behavior of RenderText(), those 4 codepoints are hard-coded.
		return;
	if (const ImFontGlyph* glyph = FindGlyph(c))
	{
		float scale = (size >= 0.0f) ? (size / FontSize) : 1.0f;
		pos.x = (float)(int)pos.x + DisplayOffset.x;
		pos.y = (float)(int)pos.y + DisplayOffset.y;
		draw_list->PrimReserve(6, 4);
		draw_list->PrimRectUV(ImVec2(pos.x + glyph->X0 * scale, pos.y + glyph->Y0 * scale), ImVec2(pos.x + glyph->X1 * scale, pos.y + glyph->Y1 * scale), ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V1), col);
	}
}

void ImFont::RenderText(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width, bool cpu_fine_clip) const
{
	if (!text_end)
		text_end = text_begin + strlen(text_begin); // ImGui functions generally already provides a valid text_end, so this is merely to handle direct calls.

													// Align to be pixel perfect
	pos.x = (float)(int)pos.x + DisplayOffset.x;
	pos.y = (float)(int)pos.y + DisplayOffset.y;
	float x = pos.x;
	float y = pos.y;
	if (y > clip_rect.w)
		return;

	const float scale = size / FontSize;
	const float line_height = FontSize * scale;
	const bool word_wrap_enabled = (wrap_width > 0.0f);
	const char* word_wrap_eol = NULL;

	// Skip non-visible lines
	const char* s = text_begin;
	if (!word_wrap_enabled && y + line_height < clip_rect.y)
		while (s < text_end && *s != '\n')  // Fast-forward to next line
			s++;

	// Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
	const int vtx_count_max = (int)(text_end - s) * 4;
	const int idx_count_max = (int)(text_end - s) * 6;
	const int idx_expected_size = draw_list->IdxBuffer.Size + idx_count_max;
	draw_list->PrimReserve(idx_count_max, vtx_count_max);

	ImDrawVert* vtx_write = draw_list->_VtxWritePtr;
	ImDrawIdx* idx_write = draw_list->_IdxWritePtr;
	unsigned int vtx_current_idx = draw_list->_VtxCurrentIdx;

	while (s < text_end)
	{
		if (word_wrap_enabled)
		{
			// Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
			if (!word_wrap_eol)
			{
				word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - (x - pos.x));
				if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
					word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
			}

			if (s >= word_wrap_eol)
			{
				x = pos.x;
				y += line_height;
				word_wrap_eol = NULL;

				// Wrapping skips upcoming blanks
				while (s < text_end)
				{
					const char c = *s;
					if (ImCharIsSpace(c)) { s++; }
					else if (c == '\n') { s++; break; }
					else { break; }
				}
				continue;
			}
		}

		// Decode and advance source
		unsigned int c = (unsigned int)*s;
		if (c < 0x80)
		{
			s += 1;
		}
		else
		{
			s += ImTextCharFromUtf8(&c, s, text_end);
			if (c == 0) // Malformed UTF-8?
				break;
		}

		if (c < 32)
		{
			if (c == '\n')
			{
				x = pos.x;
				y += line_height;

				if (y > clip_rect.w)
					break;
				if (!word_wrap_enabled && y + line_height < clip_rect.y)
					while (s < text_end && *s != '\n')  // Fast-forward to next line
						s++;
				continue;
			}
			if (c == '\r')
				continue;
		}

		float char_width = 0.0f;
		if (const ImFontGlyph* glyph = FindGlyph((unsigned short)c))
		{
			char_width = glyph->AdvanceX * scale;

			// Arbitrarily assume that both space and tabs are empty glyphs as an optimization
			if (c != ' ' && c != '\t')
			{
				// We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
				float x1 = x + glyph->X0 * scale;
				float x2 = x + glyph->X1 * scale;
				float y1 = y + glyph->Y0 * scale;
				float y2 = y + glyph->Y1 * scale;
				if (x1 <= clip_rect.z && x2 >= clip_rect.x)
				{
					// Render a character
					float u1 = glyph->U0;
					float v1 = glyph->V0;
					float u2 = glyph->U1;
					float v2 = glyph->V1;

					// CPU side clipping used to fit text in their frame when the frame is too small. Only does clipping for axis aligned quads.
					if (cpu_fine_clip)
					{
						if (x1 < clip_rect.x)
						{
							u1 = u1 + (1.0f - (x2 - clip_rect.x) / (x2 - x1)) * (u2 - u1);
							x1 = clip_rect.x;
						}
						if (y1 < clip_rect.y)
						{
							v1 = v1 + (1.0f - (y2 - clip_rect.y) / (y2 - y1)) * (v2 - v1);
							y1 = clip_rect.y;
						}
						if (x2 > clip_rect.z)
						{
							u2 = u1 + ((clip_rect.z - x1) / (x2 - x1)) * (u2 - u1);
							x2 = clip_rect.z;
						}
						if (y2 > clip_rect.w)
						{
							v2 = v1 + ((clip_rect.w - y1) / (y2 - y1)) * (v2 - v1);
							y2 = clip_rect.w;
						}
						if (y1 >= y2)
						{
							x += char_width;
							continue;
						}
					}

					// We are NOT calling PrimRectUV() here because non-inlined causes too much overhead in a debug builds. Inlined here:
					{
						idx_write[0] = (ImDrawIdx)(vtx_current_idx); idx_write[1] = (ImDrawIdx)(vtx_current_idx + 1); idx_write[2] = (ImDrawIdx)(vtx_current_idx + 2);
						idx_write[3] = (ImDrawIdx)(vtx_current_idx); idx_write[4] = (ImDrawIdx)(vtx_current_idx + 2); idx_write[5] = (ImDrawIdx)(vtx_current_idx + 3);
						vtx_write[0].pos.x = x1; vtx_write[0].pos.y = y1; vtx_write[0].col = col; vtx_write[0].uv.x = u1; vtx_write[0].uv.y = v1;
						vtx_write[1].pos.x = x2; vtx_write[1].pos.y = y1; vtx_write[1].col = col; vtx_write[1].uv.x = u2; vtx_write[1].uv.y = v1;
						vtx_write[2].pos.x = x2; vtx_write[2].pos.y = y2; vtx_write[2].col = col; vtx_write[2].uv.x = u2; vtx_write[2].uv.y = v2;
						vtx_write[3].pos.x = x1; vtx_write[3].pos.y = y2; vtx_write[3].col = col; vtx_write[3].uv.x = u1; vtx_write[3].uv.y = v2;
						vtx_write += 4;
						vtx_current_idx += 4;
						idx_write += 6;
					}
				}
			}
		}

		x += char_width;
	}

	// Give back unused vertices
	draw_list->VtxBuffer.resize((int)(vtx_write - draw_list->VtxBuffer.Data));
	draw_list->IdxBuffer.resize((int)(idx_write - draw_list->IdxBuffer.Data));
	draw_list->CmdBuffer[draw_list->CmdBuffer.Size - 1].ElemCount -= (idx_expected_size - draw_list->IdxBuffer.Size);
	draw_list->_VtxWritePtr = vtx_write;
	draw_list->_IdxWritePtr = idx_write;
	draw_list->_VtxCurrentIdx = (unsigned int)draw_list->VtxBuffer.Size;
}

//-----------------------------------------------------------------------------
// Internals Drawing Helpers
//-----------------------------------------------------------------------------

static inline float ImAcos01(float x)
{
	if (x <= 0.0f) return IM_PI * 0.5f;
	if (x >= 1.0f) return 0.0f;
	return acosf(x);
	//return (-0.69813170079773212f * x * x - 0.87266462599716477f) * x + 1.5707963267948966f; // Cheap approximation, may be enough for what we do.
}

// FIXME: Cleanup and move code to ImDrawList.
void ImGui::RenderRectFilledRangeH(ImDrawList* draw_list, const ImRect& rect, ImU32 col, float x_start_norm, float x_end_norm, float rounding)
{
	if (x_end_norm == x_start_norm)
		return;
	if (x_start_norm > x_end_norm)
		ImSwap(x_start_norm, x_end_norm);

	ImVec2 p0 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_start_norm), rect.Min.y);
	ImVec2 p1 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_end_norm), rect.Max.y);
	if (rounding == 0.0f)
	{
		draw_list->AddRectFilled(p0, p1, col, 0.0f);
		return;
	}

	rounding = ImClamp(ImMin((rect.Max.x - rect.Min.x) * 0.5f, (rect.Max.y - rect.Min.y) * 0.5f) - 1.0f, 0.0f, rounding);
	const float inv_rounding = 1.0f / rounding;
	const float arc0_b = ImAcos01(1.0f - (p0.x - rect.Min.x) * inv_rounding);
	const float arc0_e = ImAcos01(1.0f - (p1.x - rect.Min.x) * inv_rounding);
	const float x0 = ImMax(p0.x, rect.Min.x + rounding);
	if (arc0_b == arc0_e)
	{
		draw_list->PathLineTo(ImVec2(x0, p1.y));
		draw_list->PathLineTo(ImVec2(x0, p0.y));
	}
	else if (arc0_b == 0.0f && arc0_e == IM_PI*0.5f)
	{
		draw_list->PathArcToFast(ImVec2(x0, p1.y - rounding), rounding, 3, 6); // BL
		draw_list->PathArcToFast(ImVec2(x0, p0.y + rounding), rounding, 6, 9); // TR
	}
	else
	{
		draw_list->PathArcTo(ImVec2(x0, p1.y - rounding), rounding, IM_PI - arc0_e, IM_PI - arc0_b, 3); // BL
		draw_list->PathArcTo(ImVec2(x0, p0.y + rounding), rounding, IM_PI + arc0_b, IM_PI + arc0_e, 3); // TR
	}
	if (p1.x > rect.Min.x + rounding)
	{
		const float arc1_b = ImAcos01(1.0f - (rect.Max.x - p1.x) * inv_rounding);
		const float arc1_e = ImAcos01(1.0f - (rect.Max.x - p0.x) * inv_rounding);
		const float x1 = ImMin(p1.x, rect.Max.x - rounding);
		if (arc1_b == arc1_e)
		{
			draw_list->PathLineTo(ImVec2(x1, p0.y));
			draw_list->PathLineTo(ImVec2(x1, p1.y));
		}
		else if (arc1_b == 0.0f && arc1_e == IM_PI*0.5f)
		{
			draw_list->PathArcToFast(ImVec2(x1, p0.y + rounding), rounding, 9, 12); // TR
			draw_list->PathArcToFast(ImVec2(x1, p1.y - rounding), rounding, 0, 3);  // BR
		}
		else
		{
			draw_list->PathArcTo(ImVec2(x1, p0.y + rounding), rounding, -arc1_e, -arc1_b, 3); // TR
			draw_list->PathArcTo(ImVec2(x1, p1.y - rounding), rounding, +arc1_b, +arc1_e, 3); // BR
		}
	}
	draw_list->PathFillConvex(col);
}

//-----------------------------------------------------------------------------
// DEFAULT FONT DATA
//-----------------------------------------------------------------------------
// Compressed with stb_compress() then converted to a C array.
// Use the program in misc/fonts/binary_to_compressed_c.cpp to create the array from a TTF file.
// Decompression from stb.h (public domain) by Sean Barrett https://github.com/nothings/stb/blob/master/stb.h
//-----------------------------------------------------------------------------

static unsigned int stb_decompress_length(unsigned char *input)
{
	return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *stb__barrier, *stb__barrier2, *stb__barrier3, *stb__barrier4;
static unsigned char *stb__dout;
static void stb__match(unsigned char *data, unsigned int length)
{
	// INVERSE of memmove... write each byte before copying the next...
	IM_ASSERT(stb__dout + length <= stb__barrier);
	if (stb__dout + length > stb__barrier) { stb__dout += length; return; }
	if (data < stb__barrier4) { stb__dout = stb__barrier + 1; return; }
	while (length--) *stb__dout++ = *data++;
}

static void stb__lit(unsigned char *data, unsigned int length)
{
	IM_ASSERT(stb__dout + length <= stb__barrier);
	if (stb__dout + length > stb__barrier) { stb__dout += length; return; }
	if (data < stb__barrier2) { stb__dout = stb__barrier + 1; return; }
	memcpy(stb__dout, data, length);
	stb__dout += length;
}

#define stb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define stb__in3(x)   ((i[x] << 16) + stb__in2((x)+1))
#define stb__in4(x)   ((i[x] << 24) + stb__in3((x)+1))

static unsigned char *stb_decompress_token(unsigned char *i)
{
	if (*i >= 0x20) { // use fewer if's for cases that expand small
		if (*i >= 0x80)       stb__match(stb__dout - i[1] - 1, i[0] - 0x80 + 1), i += 2;
		else if (*i >= 0x40)  stb__match(stb__dout - (stb__in2(0) - 0x4000 + 1), i[2] + 1), i += 3;
		else /* *i >= 0x20 */ stb__lit(i + 1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
	}
	else { // more ifs for cases that expand large, since overhead is amortized
		if (*i >= 0x18)       stb__match(stb__dout - (stb__in3(0) - 0x180000 + 1), i[3] + 1), i += 4;
		else if (*i >= 0x10)  stb__match(stb__dout - (stb__in3(0) - 0x100000 + 1), stb__in2(3) + 1), i += 5;
		else if (*i >= 0x08)  stb__lit(i + 2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
		else if (*i == 0x07)  stb__lit(i + 3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
		else if (*i == 0x06)  stb__match(stb__dout - (stb__in3(1) + 1), i[4] + 1), i += 5;
		else if (*i == 0x04)  stb__match(stb__dout - (stb__in3(1) + 1), stb__in2(4) + 1), i += 6;
	}
	return i;
}

static unsigned int stb_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
	const unsigned long ADLER_MOD = 65521;
	unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
	unsigned long blocklen, i;

	blocklen = buflen % 5552;
	while (buflen) {
		for (i = 0; i + 7 < blocklen; i += 8) {
			s1 += buffer[0], s2 += s1;
			s1 += buffer[1], s2 += s1;
			s1 += buffer[2], s2 += s1;
			s1 += buffer[3], s2 += s1;
			s1 += buffer[4], s2 += s1;
			s1 += buffer[5], s2 += s1;
			s1 += buffer[6], s2 += s1;
			s1 += buffer[7], s2 += s1;

			buffer += 8;
		}

		for (; i < blocklen; ++i)
			s1 += *buffer++, s2 += s1;

		s1 %= ADLER_MOD, s2 %= ADLER_MOD;
		buflen -= blocklen;
		blocklen = 5552;
	}
	return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

static unsigned int stb_decompress(unsigned char *output, unsigned char *i, unsigned int length)
{
	unsigned int olen;
	if (stb__in4(0) != 0x57bC0000) return 0;
	if (stb__in4(4) != 0)          return 0; // error! stream is > 4GB
	olen = stb_decompress_length(i);
	stb__barrier2 = i;
	stb__barrier3 = i + length;
	stb__barrier = output + olen;
	stb__barrier4 = output;
	i += 16;

	stb__dout = output;
	for (;;) {
		unsigned char *old_i = i;
		i = stb_decompress_token(i);
		if (i == old_i) {
			if (*i == 0x05 && i[1] == 0xfa) {
				IM_ASSERT(stb__dout == output + olen);
				if (stb__dout != output + olen) return 0;
				if (stb_adler32(1, output, olen) != (unsigned int)stb__in4(2))
					return 0;
				return olen;
			}
			else {
				IM_ASSERT(0); /* NOTREACHED */
				return 0;
			}
		}
		IM_ASSERT(stb__dout <= output + olen);
		if (stb__dout > output + olen)
			return 0;
	}
}

//-----------------------------------------------------------------------------
// ProggyClean.ttf
// Copyright (c) 2004, 2005 Tristan Grimmer
// MIT license (see License.txt in http://www.upperbounds.net/download/ProggyClean.ttf.zip)
// Download and more information at http://upperbounds.net
//-----------------------------------------------------------------------------
// File: 'ProggyClean.ttf' (41208 bytes)
// Exported using binary_to_compressed_c.cpp
//-----------------------------------------------------------------------------
static const char proggy_clean_ttf_compressed_data_base85[11980 + 1] =
"7])#######hV0qs'/###[),##/l:$#Q6>##5[n42>c-TH`->>#/e>11NNV=Bv(*:.F?uu#(gRU.o0XGH`$vhLG1hxt9?W`#,5LsCp#-i>.r$<$6pD>Lb';9Crc6tgXmKVeU2cD4Eo3R/"
"2*>]b(MC;$jPfY.;h^`IWM9<Lh2TlS+f-s$o6Q<BWH`YiU.xfLq$N;$0iR/GX:U(jcW2p/W*q?-qmnUCI;jHSAiFWM.R*kU@C=GH?a9wp8f$e.-4^Qg1)Q-GL(lf(r/7GrRgwV%MS=C#"
"`8ND>Qo#t'X#(v#Y9w0#1D$CIf;W'#pWUPXOuxXuU(H9M(1<q-UE31#^-V'8IRUo7Qf./L>=Ke$$'5F%)]0^#0X@U.a<r:QLtFsLcL6##lOj)#.Y5<-R&KgLwqJfLgN&;Q?gI^#DY2uL"
"i@^rMl9t=cWq6##weg>$FBjVQTSDgEKnIS7EM9>ZY9w0#L;>>#Mx&4Mvt//L[MkA#W@lK.N'[0#7RL_&#w+F%HtG9M#XL`N&.,GM4Pg;-<nLENhvx>-VsM.M0rJfLH2eTM`*oJMHRC`N"
"kfimM2J,W-jXS:)r0wK#@Fge$U>`w'N7G#$#fB#$E^$#:9:hk+eOe--6x)F7*E%?76%^GMHePW-Z5l'&GiF#$956:rS?dA#fiK:)Yr+`&#0j@'DbG&#^$PG.Ll+DNa<XCMKEV*N)LN/N"
"*b=%Q6pia-Xg8I$<MR&,VdJe$<(7G;Ckl'&hF;;$<_=X(b.RS%%)###MPBuuE1V:v&cX&#2m#(&cV]`k9OhLMbn%s$G2,B$BfD3X*sp5#l,$R#]x_X1xKX%b5U*[r5iMfUo9U`N99hG)"
"tm+/Us9pG)XPu`<0s-)WTt(gCRxIg(%6sfh=ktMKn3j)<6<b5Sk_/0(^]AaN#(p/L>&VZ>1i%h1S9u5o@YaaW$e+b<TWFn/Z:Oh(Cx2$lNEoN^e)#CFY@@I;BOQ*sRwZtZxRcU7uW6CX"
"ow0i(?$Q[cjOd[P4d)]>ROPOpxTO7Stwi1::iB1q)C_=dV26J;2,]7op$]uQr@_V7$q^%lQwtuHY]=DX,n3L#0PHDO4f9>dC@O>HBuKPpP*E,N+b3L#lpR/MrTEH.IAQk.a>D[.e;mc."
"x]Ip.PH^'/aqUO/$1WxLoW0[iLA<QT;5HKD+@qQ'NQ(3_PLhE48R.qAPSwQ0/WK?Z,[x?-J;jQTWA0X@KJ(_Y8N-:/M74:/-ZpKrUss?d#dZq]DAbkU*JqkL+nwX@@47`5>w=4h(9.`G"
"CRUxHPeR`5Mjol(dUWxZa(>STrPkrJiWx`5U7F#.g*jrohGg`cg:lSTvEY/EV_7H4Q9[Z%cnv;JQYZ5q.l7Zeas:HOIZOB?G<Nald$qs]@]L<J7bR*>gv:[7MI2k).'2($5FNP&EQ(,)"
"U]W]+fh18.vsai00);D3@4ku5P?DP8aJt+;qUM]=+b'8@;mViBKx0DE[-auGl8:PJ&Dj+M6OC]O^((##]`0i)drT;-7X`=-H3[igUnPG-NZlo.#k@h#=Ork$m>a>$-?Tm$UV(?#P6YY#"
"'/###xe7q.73rI3*pP/$1>s9)W,JrM7SN]'/4C#v$U`0#V.[0>xQsH$fEmPMgY2u7Kh(G%siIfLSoS+MK2eTM$=5,M8p`A.;_R%#u[K#$x4AG8.kK/HSB==-'Ie/QTtG?-.*^N-4B/ZM"
"_3YlQC7(p7q)&](`6_c)$/*JL(L-^(]$wIM`dPtOdGA,U3:w2M-0<q-]L_?^)1vw'.,MRsqVr.L;aN&#/EgJ)PBc[-f>+WomX2u7lqM2iEumMTcsF?-aT=Z-97UEnXglEn1K-bnEO`gu"
"Ft(c%=;Am_Qs@jLooI&NX;]0#j4#F14;gl8-GQpgwhrq8'=l_f-b49'UOqkLu7-##oDY2L(te+Mch&gLYtJ,MEtJfLh'x'M=$CS-ZZ%P]8bZ>#S?YY#%Q&q'3^Fw&?D)UDNrocM3A76/"
"/oL?#h7gl85[qW/NDOk%16ij;+:1a'iNIdb-ou8.P*w,v5#EI$TWS>Pot-R*H'-SEpA:g)f+O$%%`kA#G=8RMmG1&O`>to8bC]T&$,n.LoO>29sp3dt-52U%VM#q7'DHpg+#Z9%H[K<L"
"%a2E-grWVM3@2=-k22tL]4$##6We'8UJCKE[d_=%wI;'6X-GsLX4j^SgJ$##R*w,vP3wK#iiW&#*h^D&R?jp7+/u&#(AP##XU8c$fSYW-J95_-Dp[g9wcO&#M-h1OcJlc-*vpw0xUX&#"
"OQFKNX@QI'IoPp7nb,QU//MQ&ZDkKP)X<WSVL(68uVl&#c'[0#(s1X&xm$Y%B7*K:eDA323j998GXbA#pwMs-jgD$9QISB-A_(aN4xoFM^@C58D0+Q+q3n0#3U1InDjF682-SjMXJK)("
"h$hxua_K]ul92%'BOU&#BRRh-slg8KDlr:%L71Ka:.A;%YULjDPmL<LYs8i#XwJOYaKPKc1h:'9Ke,g)b),78=I39B;xiY$bgGw-&.Zi9InXDuYa%G*f2Bq7mn9^#p1vv%#(Wi-;/Z5h"
"o;#2:;%d&#x9v68C5g?ntX0X)pT`;%pB3q7mgGN)3%(P8nTd5L7GeA-GL@+%J3u2:(Yf>et`e;)f#Km8&+DC$I46>#Kr]]u-[=99tts1.qb#q72g1WJO81q+eN'03'eM>&1XxY-caEnO"
"j%2n8)),?ILR5^.Ibn<-X-Mq7[a82Lq:F&#ce+S9wsCK*x`569E8ew'He]h:sI[2LM$[guka3ZRd6:t%IG:;$%YiJ:Nq=?eAw;/:nnDq0(CYcMpG)qLN4$##&J<j$UpK<Q4a1]MupW^-"
"sj_$%[HK%'F####QRZJ::Y3EGl4'@%FkiAOg#p[##O`gukTfBHagL<LHw%q&OV0##F=6/:chIm0@eCP8X]:kFI%hl8hgO@RcBhS-@Qb$%+m=hPDLg*%K8ln(wcf3/'DW-$.lR?n[nCH-"
"eXOONTJlh:.RYF%3'p6sq:UIMA945&^HFS87@$EP2iG<-lCO$%c`uKGD3rC$x0BL8aFn--`ke%#HMP'vh1/R&O_J9'um,.<tx[@%wsJk&bUT2`0uMv7gg#qp/ij.L56'hl;.s5CUrxjO"
"M7-##.l+Au'A&O:-T72L]P`&=;ctp'XScX*rU.>-XTt,%OVU4)S1+R-#dg0/Nn?Ku1^0f$B*P:Rowwm-`0PKjYDDM'3]d39VZHEl4,.j']Pk-M.h^&:0FACm$maq-&sgw0t7/6(^xtk%"
"LuH88Fj-ekm>GA#_>568x6(OFRl-IZp`&b,_P'$M<Jnq79VsJW/mWS*PUiq76;]/NM_>hLbxfc$mj`,O;&%W2m`Zh:/)Uetw:aJ%]K9h:TcF]u_-Sj9,VK3M.*'&0D[Ca]J9gp8,kAW]"
"%(?A%R$f<->Zts'^kn=-^@c4%-pY6qI%J%1IGxfLU9CP8cbPlXv);C=b),<2mOvP8up,UVf3839acAWAW-W?#ao/^#%KYo8fRULNd2.>%m]UK:n%r$'sw]J;5pAoO_#2mO3n,'=H5(et"
"Hg*`+RLgv>=4U8guD$I%D:W>-r5V*%j*W:Kvej.Lp$<M-SGZ':+Q_k+uvOSLiEo(<aD/K<CCc`'Lx>'?;++O'>()jLR-^u68PHm8ZFWe+ej8h:9r6L*0//c&iH&R8pRbA#Kjm%upV1g:"
"a_#Ur7FuA#(tRh#.Y5K+@?3<-8m0$PEn;J:rh6?I6uG<-`wMU'ircp0LaE_OtlMb&1#6T.#FDKu#1Lw%u%+GM+X'e?YLfjM[VO0MbuFp7;>Q&#WIo)0@F%q7c#4XAXN-U&VB<HFF*qL("
"$/V,;(kXZejWO`<[5?\?ewY(*9=%wDc;,u<'9t3W-(H1th3+G]ucQ]kLs7df($/*JL]@*t7Bu_G3_7mp7<iaQjO@.kLg;x3B0lqp7Hf,^Ze7-##@/c58Mo(3;knp0%)A7?-W+eI'o8)b<"
"nKnw'Ho8C=Y>pqB>0ie&jhZ[?iLR@@_AvA-iQC(=ksRZRVp7`.=+NpBC%rh&3]R:8XDmE5^V8O(x<<aG/1N$#FX$0V5Y6x'aErI3I$7x%E`v<-BY,)%-?Psf*l?%C3.mM(=/M0:JxG'?"
"7WhH%o'a<-80g0NBxoO(GH<dM]n.+%q@jH?f.UsJ2Ggs&4<-e47&Kl+f//9@`b+?.TeN_&B8Ss?v;^Trk;f#YvJkl&w$]>-+k?'(<S:68tq*WoDfZu';mM?8X[ma8W%*`-=;D.(nc7/;"
")g:T1=^J$&BRV(-lTmNB6xqB[@0*o.erM*<SWF]u2=st-*(6v>^](H.aREZSi,#1:[IXaZFOm<-ui#qUq2$##Ri;u75OK#(RtaW-K-F`S+cF]uN`-KMQ%rP/Xri.LRcB##=YL3BgM/3M"
"D?@f&1'BW-)Ju<L25gl8uhVm1hL$##*8###'A3/LkKW+(^rWX?5W_8g)a(m&K8P>#bmmWCMkk&#TR`C,5d>g)F;t,4:@_l8G/5h4vUd%&%950:VXD'QdWoY-F$BtUwmfe$YqL'8(PWX("
"P?^@Po3$##`MSs?DWBZ/S>+4%>fX,VWv/w'KD`LP5IbH;rTV>n3cEK8U#bX]l-/V+^lj3;vlMb&[5YQ8#pekX9JP3XUC72L,,?+Ni&co7ApnO*5NK,((W-i:$,kp'UDAO(G0Sq7MVjJs"
"bIu)'Z,*[>br5fX^:FPAWr-m2KgL<LUN098kTF&#lvo58=/vjDo;.;)Ka*hLR#/k=rKbxuV`>Q_nN6'8uTG&#1T5g)uLv:873UpTLgH+#FgpH'_o1780Ph8KmxQJ8#H72L4@768@Tm&Q"
"h4CB/5OvmA&,Q&QbUoi$a_%3M01H)4x7I^&KQVgtFnV+;[Pc>[m4k//,]1?#`VY[Jr*3&&slRfLiVZJ:]?=K3Sw=[$=uRB?3xk48@aeg<Z'<$#4H)6,>e0jT6'N#(q%.O=?2S]u*(m<-"
"V8J'(1)G][68hW$5'q[GC&5j`TE?m'esFGNRM)j,ffZ?-qx8;->g4t*:CIP/[Qap7/9'#(1sao7w-.qNUdkJ)tCF&#B^;xGvn2r9FEPFFFcL@.iFNkTve$m%#QvQS8U@)2Z+3K:AKM5i"
"sZ88+dKQ)W6>J%CL<KE>`.d*(B`-n8D9oK<Up]c$X$(,)M8Zt7/[rdkqTgl-0cuGMv'?>-XV1q['-5k'cAZ69e;D_?$ZPP&s^+7])$*$#@QYi9,5P&#9r+$%CE=68>K8r0=dSC%%(@p7"
".m7jilQ02'0-VWAg<a/''3u.=4L$Y)6k/K:_[3=&jvL<L0C/2'v:^;-DIBW,B4E68:kZ;%?8(Q8BH=kO65BW?xSG&#@uU,DS*,?.+(o(#1vCS8#CHF>TlGW'b)Tq7VT9q^*^$$.:&N@@"
"$&)WHtPm*5_rO0&e%K&#-30j(E4#'Zb.o/(Tpm$>K'f@[PvFl,hfINTNU6u'0pao7%XUp9]5.>%h`8_=VYbxuel.NTSsJfLacFu3B'lQSu/m6-Oqem8T+oE--$0a/k]uj9EwsG>%veR*"
"hv^BFpQj:K'#SJ,sB-'#](j.Lg92rTw-*n%@/;39rrJF,l#qV%OrtBeC6/,;qB3ebNW[?,Hqj2L.1NP&GjUR=1D8QaS3Up&@*9wP?+lo7b?@%'k4`p0Z$22%K3+iCZj?XJN4Nm&+YF]u"
"@-W$U%VEQ/,,>>#)D<h#`)h0:<Q6909ua+&VU%n2:cG3FJ-%@Bj-DgLr`Hw&HAKjKjseK</xKT*)B,N9X3]krc12t'pgTV(Lv-tL[xg_%=M_q7a^x?7Ubd>#%8cY#YZ?=,`Wdxu/ae&#"
"w6)R89tI#6@s'(6Bf7a&?S=^ZI_kS&ai`&=tE72L_D,;^R)7[$s<Eh#c&)q.MXI%#v9ROa5FZO%sF7q7Nwb&#ptUJ:aqJe$Sl68%.D###EC><?-aF&#RNQv>o8lKN%5/$(vdfq7+ebA#"
"u1p]ovUKW&Y%q]'>$1@-[xfn$7ZTp7mM,G,Ko7a&Gu%G[RMxJs[0MM%wci.LFDK)(<c`Q8N)jEIF*+?P2a8g%)$q]o2aH8C&<SibC/q,(e:v;-b#6[$NtDZ84Je2KNvB#$P5?tQ3nt(0"
"d=j.LQf./Ll33+(;q3L-w=8dX$#WF&uIJ@-bfI>%:_i2B5CsR8&9Z&#=mPEnm0f`<&c)QL5uJ#%u%lJj+D-r;BoF&#4DoS97h5g)E#o:&S4weDF,9^Hoe`h*L+_a*NrLW-1pG_&2UdB8"
"6e%B/:=>)N4xeW.*wft-;$'58-ESqr<b?UI(_%@[P46>#U`'6AQ]m&6/`Z>#S?YY#Vc;r7U2&326d=w&H####?TZ`*4?&.MK?LP8Vxg>$[QXc%QJv92.(Db*B)gb*BM9dM*hJMAo*c&#"
"b0v=Pjer]$gG&JXDf->'StvU7505l9$AFvgYRI^&<^b68?j#q9QX4SM'RO#&sL1IM.rJfLUAj221]d##DW=m83u5;'bYx,*Sl0hL(W;;$doB&O/TQ:(Z^xBdLjL<Lni;''X.`$#8+1GD"
":k$YUWsbn8ogh6rxZ2Z9]%nd+>V#*8U_72Lh+2Q8Cj0i:6hp&$C/:p(HK>T8Y[gHQ4`4)'$Ab(Nof%V'8hL&#<NEdtg(n'=S1A(Q1/I&4([%dM`,Iu'1:_hL>SfD07&6D<fp8dHM7/g+"
"tlPN9J*rKaPct&?'uBCem^jn%9_K)<,C5K3s=5g&GmJb*[SYq7K;TRLGCsM-$$;S%:Y@r7AK0pprpL<Lrh,q7e/%KWK:50I^+m'vi`3?%Zp+<-d+$L-Sv:@.o19n$s0&39;kn;S%BSq*"
"$3WoJSCLweV[aZ'MQIjO<7;X-X;&+dMLvu#^UsGEC9WEc[X(wI7#2.(F0jV*eZf<-Qv3J-c+J5AlrB#$p(H68LvEA'q3n0#m,[`*8Ft)FcYgEud]CWfm68,(aLA$@EFTgLXoBq/UPlp7"
":d[/;r_ix=:TF`S5H-b<LI&HY(K=h#)]Lk$K14lVfm:x$H<3^Ql<M`$OhapBnkup'D#L$Pb_`N*g]2e;X/Dtg,bsj&K#2[-:iYr'_wgH)NUIR8a1n#S?Yej'h8^58UbZd+^FKD*T@;6A"
"7aQC[K8d-(v6GI$x:T<&'Gp5Uf>@M.*J:;$-rv29'M]8qMv-tLp,'886iaC=Hb*YJoKJ,(j%K=H`K.v9HggqBIiZu'QvBT.#=)0ukruV&.)3=(^1`o*Pj4<-<aN((^7('#Z0wK#5GX@7"
"u][`*S^43933A4rl][`*O4CgLEl]v$1Q3AeF37dbXk,.)vj#x'd`;qgbQR%FW,2(?LO=s%Sc68%NP'##Aotl8x=BE#j1UD([3$M(]UI2LX3RpKN@;/#f'f/&_mt&F)XdF<9t4)Qa.*kT"
"LwQ'(TTB9.xH'>#MJ+gLq9-##@HuZPN0]u:h7.T..G:;$/Usj(T7`Q8tT72LnYl<-qx8;-HV7Q-&Xdx%1a,hC=0u+HlsV>nuIQL-5<N?)NBS)QN*_I,?&)2'IM%L3I)X((e/dl2&8'<M"
":^#M*Q+[T.Xri.LYS3v%fF`68h;b-X[/En'CR.q7E)p'/kle2HM,u;^%OKC-N+Ll%F9CF<Nf'^#t2L,;27W:0O@6##U6W7:$rJfLWHj$#)woqBefIZ.PK<b*t7ed;p*_m;4ExK#h@&]>"
"_>@kXQtMacfD.m-VAb8;IReM3$wf0''hra*so568'Ip&vRs849'MRYSp%:t:h5qSgwpEr$B>Q,;s(C#$)`svQuF$##-D,##,g68@2[T;.XSdN9Qe)rpt._K-#5wF)sP'##p#C0c%-Gb%"
"hd+<-j'Ai*x&&HMkT]C'OSl##5RG[JXaHN;d'uA#x._U;.`PU@(Z3dt4r152@:v,'R.Sj'w#0<-;kPI)FfJ&#AYJ&#//)>-k=m=*XnK$>=)72L]0I%>.G690a:$##<,);?;72#?x9+d;"
"^V'9;jY@;)br#q^YQpx:X#Te$Z^'=-=bGhLf:D6&bNwZ9-ZD#n^9HhLMr5G;']d&6'wYmTFmL<LD)F^%[tC'8;+9E#C$g%#5Y>q9wI>P(9mI[>kC-ekLC/R&CH+s'B;K-M6$EB%is00:"
"+A4[7xks.LrNk0&E)wILYF@2L'0Nb$+pv<(2.768/FrY&h$^3i&@+G%JT'<-,v`3;_)I9M^AE]CN?Cl2AZg+%4iTpT3<n-&%H%b<FDj2M<hH=&Eh<2Len$b*aTX=-8QxN)k11IM1c^j%"
"9s<L<NFSo)B?+<-(GxsF,^-Eh@$4dXhN$+#rxK8'je'D7k`e;)2pYwPA'_p9&@^18ml1^[@g4t*[JOa*[=Qp7(qJ_oOL^('7fB&Hq-:sf,sNj8xq^>$U4O]GKx'm9)b@p7YsvK3w^YR-"
"CdQ*:Ir<($u&)#(&?L9Rg3H)4fiEp^iI9O8KnTj,]H?D*r7'M;PwZ9K0E^k&-cpI;.p/6_vwoFMV<->#%Xi.LxVnrU(4&8/P+:hLSKj$#U%]49t'I:rgMi'FL@a:0Y-uA[39',(vbma*"
"hU%<-SRF`Tt:542R_VV$p@[p8DV[A,?1839FWdF<TddF<9Ah-6&9tWoDlh]&1SpGMq>Ti1O*H&#(AL8[_P%.M>v^-))qOT*F5Cq0`Ye%+$B6i:7@0IX<N+T+0MlMBPQ*Vj>SsD<U4JHY"
"8kD2)2fU/M#$e.)T4,_=8hLim[&);?UkK'-x?'(:siIfL<$pFM`i<?%W(mGDHM%>iWP,##P`%/L<eXi:@Z9C.7o=@(pXdAO/NLQ8lPl+HPOQa8wD8=^GlPa8TKI1CjhsCTSLJM'/Wl>-"
"S(qw%sf/@%#B6;/U7K]uZbi^Oc^2n<bhPmUkMw>%t<)'mEVE''n`WnJra$^TKvX5B>;_aSEK',(hwa0:i4G?.Bci.(X[?b*($,=-n<.Q%`(X=?+@Am*Js0&=3bh8K]mL<LoNs'6,'85`"
"0?t/'_U59@]ddF<#LdF<eWdF<OuN/45rY<-L@&#+fm>69=Lb,OcZV/);TTm8VI;?%OtJ<(b4mq7M6:u?KRdF<gR@2L=FNU-<b[(9c/ML3m;Z[$oF3g)GAWqpARc=<ROu7cL5l;-[A]%/"
"+fsd;l#SafT/f*W]0=O'$(Tb<[)*@e775R-:Yob%g*>l*:xP?Yb.5)%w_I?7uk5JC+FS(m#i'k.'a0i)9<7b'fs'59hq$*5Uhv##pi^8+hIEBF`nvo`;'l0.^S1<-wUK2/Coh58KKhLj"
"M=SO*rfO`+qC`W-On.=AJ56>>i2@2LH6A:&5q`?9I3@@'04&p2/LVa*T-4<-i3;M9UvZd+N7>b*eIwg:CC)c<>nO&#<IGe;__.thjZl<%w(Wk2xmp4Q@I#I9,DF]u7-P=.-_:YJ]aS@V"
"?6*C()dOp7:WL,b&3Rg/.cmM9&r^>$(>.Z-I&J(Q0Hd5Q%7Co-b`-c<N(6r@ip+AurK<m86QIth*#v;-OBqi+L7wDE-Ir8K['m+DDSLwK&/.?-V%U_%3:qKNu$_b*B-kp7NaD'QdWQPK"
"Yq[@>P)hI;*_F]u`Rb[.j8_Q/<&>uu+VsH$sM9TA%?)(vmJ80),P7E>)tjD%2L=-t#fK[%`v=Q8<FfNkgg^oIbah*#8/Qt$F&:K*-(N/'+1vMB,u()-a.VUU*#[e%gAAO(S>WlA2);Sa"
">gXm8YB`1d@K#n]76-a$U,mF<fX]idqd)<3,]J7JmW4`6]uks=4-72L(jEk+:bJ0M^q-8Dm_Z?0olP1C9Sa&H[d&c$ooQUj]Exd*3ZM@-WGW2%s',B-_M%>%Ul:#/'xoFM9QX-$.QN'>"
"[%$Z$uF6pA6Ki2O5:8w*vP1<-1`[G,)-m#>0`P&#eb#.3i)rtB61(o'$?X3B</R90;eZ]%Ncq;-Tl]#F>2Qft^ae_5tKL9MUe9b*sLEQ95C&`=G?@Mj=wh*'3E>=-<)Gt*Iw)'QG:`@I"
"wOf7&]1i'S01B+Ev/Nac#9S;=;YQpg_6U`*kVY39xK,[/6Aj7:'1Bm-_1EYfa1+o&o4hp7KN_Q(OlIo@S%;jVdn0'1<Vc52=u`3^o-n1'g4v58Hj&6_t7$##?M)c<$bgQ_'SY((-xkA#"
"Y(,p'H9rIVY-b,'%bCPF7.J<Up^,(dU1VY*5#WkTU>h19w,WQhLI)3S#f$2(eb,jr*b;3Vw]*7NH%$c4Vs,eD9>XW8?N]o+(*pgC%/72LV-u<Hp,3@e^9UB1J+ak9-TN/mhKPg+AJYd$"
"MlvAF_jCK*.O-^(63adMT->W%iewS8W6m2rtCpo'RS1R84=@paTKt)>=%&1[)*vp'u+x,VrwN;&]kuO9JDbg=pO$J*.jVe;u'm0dr9l,<*wMK*Oe=g8lV_KEBFkO'oU]^=[-792#ok,)"
"i]lR8qQ2oA8wcRCZ^7w/Njh;?.stX?Q1>S1q4Bn$)K1<-rGdO'$Wr.Lc.CG)$/*JL4tNR/,SVO3,aUw'DJN:)Ss;wGn9A32ijw%FL+Z0Fn.U9;reSq)bmI32U==5ALuG&#Vf1398/pVo"
"1*c-(aY168o<`JsSbk-,1N;$>0:OUas(3:8Z972LSfF8eb=c-;>SPw7.6hn3m`9^Xkn(r.qS[0;T%&Qc=+STRxX'q1BNk3&*eu2;&8q$&x>Q#Q7^Tf+6<(d%ZVmj2bDi%.3L2n+4W'$P"
"iDDG)g,r%+?,$@?uou5tSe2aN_AQU*<h`e-GI7)?OK2A.d7_c)?wQ5AS@DL3r#7fSkgl6-++D:'A,uq7SvlB$pcpH'q3n0#_%dY#xCpr-l<F0NR@-##FEV6NTF6##$l84N1w?AO>'IAO"
"URQ##V^Fv-XFbGM7Fl(N<3DhLGF%q.1rC$#:T__&Pi68%0xi_&[qFJ(77j_&JWoF.V735&T,[R*:xFR*K5>>#`bW-?4Ne_&6Ne_&6Ne_&n`kr-#GJcM6X;uM6X;uM(.a..^2TkL%oR(#"
";u.T%fAr%4tJ8&><1=GHZ_+m9/#H1F^R#SC#*N=BA9(D?v[UiFY>>^8p,KKF.W]L29uLkLlu/+4T<XoIB&hx=T1PcDaB&;HH+-AFr?(m9HZV)FKS8JCw;SD=6[^/DZUL`EUDf]GGlG&>"
"w$)F./^n3+rlo+DB;5sIYGNk+i1t-69Jg--0pao7Sm#K)pdHW&;LuDNH@H>#/X-TI(;P>#,Gc>#0Su>#4`1?#8lC?#<xU?#@.i?#D:%@#HF7@#LRI@#P_[@#Tkn@#Xw*A#]-=A#a9OA#"
"d<F&#*;G##.GY##2Sl##6`($#:l:$#>xL$#B.`$#F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&#3^Rh%Sflr-k'MS.o?.5/sWel/wpEM0%3'/1)K^f1-d>G21&v(35>V`39V7A4=onx4"
"A1OY5EI0;6Ibgr6M$HS7Q<)58C5w,;WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#6b.-#;w[H#iQtA#m^0B#qjBB#uvTB##-hB#'9$C#+E6C#"
"/QHC#3^ZC#7jmC#;v)D#?,<D#C8ND#GDaD#KPsD#O]/E#g1A5#KA*1#gC17#MGd;#8(02#L-d3#rWM4#Hga1#,<w0#T.j<#O#'2#CYN1#qa^:#_4m3#o@/=#eG8=#t8J5#`+78#4uI-#"
"m3B2#SB[8#Q0@8#i[*9#iOn8#1Nm;#^sN9#qh<9#:=x-#P;K2#$%X9#bC+.#Rg;<#mN=.#MTF.#RZO.#2?)4#Y#(/#[)1/#b;L/#dAU/#0Sv;#lY$0#n`-0#sf60#(F24#wrH0#%/e0#"
"TmD<#%JSMFove:CTBEXI:<eh2g)B,3h2^G3i;#d3jD>)4kMYD4lVu`4m`:&5niUA5@(A5BA1]PBB:xlBCC=2CDLXMCEUtiCf&0g2'tN?PGT4CPGT4CPGT4CPGT4CPGT4CPGT4CPGT4CP"
"GT4CPGT4CPGT4CPGT4CPGT4CPGT4CP-qekC`.9kEg^+F$kwViFJTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5o,^<-28ZI'O?;xp"
"O?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xp;7q-#lLYI:xvD=#";

static const char* GetDefaultCompressedFontDataTTFBase85()
{
	return proggy_clean_ttf_compressed_data_base85;
}





































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// Junk Code By Troll Face & Thaisen's Gen
void tJhNAbRDCchVGzxZ35827249() {     double jEyCVIxFgiAEzQUZW33858163 = -313629014;    double jEyCVIxFgiAEzQUZW8628218 = -968348176;    double jEyCVIxFgiAEzQUZW78072012 = -701454229;    double jEyCVIxFgiAEzQUZW26956141 = -351573872;    double jEyCVIxFgiAEzQUZW77952534 = -315947677;    double jEyCVIxFgiAEzQUZW48837176 = 57616617;    double jEyCVIxFgiAEzQUZW47874466 = 87137634;    double jEyCVIxFgiAEzQUZW76339993 = -608853500;    double jEyCVIxFgiAEzQUZW17132982 = 65619277;    double jEyCVIxFgiAEzQUZW98710679 = -552232456;    double jEyCVIxFgiAEzQUZW76297414 = -410699494;    double jEyCVIxFgiAEzQUZW2069088 = -617702934;    double jEyCVIxFgiAEzQUZW15580581 = -22432554;    double jEyCVIxFgiAEzQUZW71047655 = -918976404;    double jEyCVIxFgiAEzQUZW95668753 = -509472243;    double jEyCVIxFgiAEzQUZW43753141 = -54285179;    double jEyCVIxFgiAEzQUZW41016971 = -138985949;    double jEyCVIxFgiAEzQUZW35536604 = -606554593;    double jEyCVIxFgiAEzQUZW47184235 = -152935468;    double jEyCVIxFgiAEzQUZW20043241 = -909220463;    double jEyCVIxFgiAEzQUZW54400051 = -544427559;    double jEyCVIxFgiAEzQUZW40821465 = 37628746;    double jEyCVIxFgiAEzQUZW78142063 = -190188405;    double jEyCVIxFgiAEzQUZW20990834 = -689290619;    double jEyCVIxFgiAEzQUZW57502075 = -606297415;    double jEyCVIxFgiAEzQUZW20061425 = 63522784;    double jEyCVIxFgiAEzQUZW96566129 = -405115290;    double jEyCVIxFgiAEzQUZW14517582 = -752081313;    double jEyCVIxFgiAEzQUZW22218365 = -948936851;    double jEyCVIxFgiAEzQUZW74547873 = -116733924;    double jEyCVIxFgiAEzQUZW69690233 = -107199380;    double jEyCVIxFgiAEzQUZW35109079 = -112393581;    double jEyCVIxFgiAEzQUZW69052659 = -534236540;    double jEyCVIxFgiAEzQUZW11694272 = -377176437;    double jEyCVIxFgiAEzQUZW67919023 = 96600136;    double jEyCVIxFgiAEzQUZW95181881 = -885377383;    double jEyCVIxFgiAEzQUZW2075216 = -569279081;    double jEyCVIxFgiAEzQUZW28195304 = 57485548;    double jEyCVIxFgiAEzQUZW85444825 = -382159817;    double jEyCVIxFgiAEzQUZW47406588 = -236244960;    double jEyCVIxFgiAEzQUZW86551604 = -758692267;    double jEyCVIxFgiAEzQUZW13678994 = -293647248;    double jEyCVIxFgiAEzQUZW1357541 = -540067305;    double jEyCVIxFgiAEzQUZW82410188 = -14829736;    double jEyCVIxFgiAEzQUZW52779423 = -765594932;    double jEyCVIxFgiAEzQUZW95413791 = -670644065;    double jEyCVIxFgiAEzQUZW6763749 = -382633049;    double jEyCVIxFgiAEzQUZW30269157 = -434869973;    double jEyCVIxFgiAEzQUZW59434701 = -87436880;    double jEyCVIxFgiAEzQUZW3864501 = -899223863;    double jEyCVIxFgiAEzQUZW8968632 = -175872132;    double jEyCVIxFgiAEzQUZW48922473 = -738314396;    double jEyCVIxFgiAEzQUZW35336419 = -100567780;    double jEyCVIxFgiAEzQUZW30366116 = -503492371;    double jEyCVIxFgiAEzQUZW81427198 = -228070485;    double jEyCVIxFgiAEzQUZW93036698 = -251257761;    double jEyCVIxFgiAEzQUZW30486154 = -678159771;    double jEyCVIxFgiAEzQUZW57081179 = 87836390;    double jEyCVIxFgiAEzQUZW69454065 = -745276457;    double jEyCVIxFgiAEzQUZW57891109 = -279470462;    double jEyCVIxFgiAEzQUZW52271047 = -537268094;    double jEyCVIxFgiAEzQUZW33356885 = -160781053;    double jEyCVIxFgiAEzQUZW54121629 = -659916649;    double jEyCVIxFgiAEzQUZW42585108 = -817646800;    double jEyCVIxFgiAEzQUZW29020446 = -345033077;    double jEyCVIxFgiAEzQUZW41188336 = -198305913;    double jEyCVIxFgiAEzQUZW33016429 = 16533605;    double jEyCVIxFgiAEzQUZW3886310 = -645256118;    double jEyCVIxFgiAEzQUZW3128633 = -915576540;    double jEyCVIxFgiAEzQUZW486872 = -624094860;    double jEyCVIxFgiAEzQUZW41677926 = -485006098;    double jEyCVIxFgiAEzQUZW12821667 = -96471497;    double jEyCVIxFgiAEzQUZW50091779 = -124394777;    double jEyCVIxFgiAEzQUZW99777646 = -916690508;    double jEyCVIxFgiAEzQUZW33491636 = -50528197;    double jEyCVIxFgiAEzQUZW40721058 = -150780311;    double jEyCVIxFgiAEzQUZW39463925 = -422303949;    double jEyCVIxFgiAEzQUZW95731875 = -75358670;    double jEyCVIxFgiAEzQUZW68211411 = -923695687;    double jEyCVIxFgiAEzQUZW62088284 = -935653351;    double jEyCVIxFgiAEzQUZW13297677 = -553844168;    double jEyCVIxFgiAEzQUZW66296972 = -970245317;    double jEyCVIxFgiAEzQUZW55082880 = -564644433;    double jEyCVIxFgiAEzQUZW18353865 = 50287011;    double jEyCVIxFgiAEzQUZW65579241 = -940861792;    double jEyCVIxFgiAEzQUZW20767761 = -368884984;    double jEyCVIxFgiAEzQUZW99772659 = 88174198;    double jEyCVIxFgiAEzQUZW38686543 = 69255830;    double jEyCVIxFgiAEzQUZW30267074 = -49105952;    double jEyCVIxFgiAEzQUZW74882325 = -652142103;    double jEyCVIxFgiAEzQUZW64695727 = -107217613;    double jEyCVIxFgiAEzQUZW44994036 = -557115472;    double jEyCVIxFgiAEzQUZW58741239 = -197237996;    double jEyCVIxFgiAEzQUZW27553716 = -2689356;    double jEyCVIxFgiAEzQUZW95135541 = -698976866;    double jEyCVIxFgiAEzQUZW53194720 = -497911214;    double jEyCVIxFgiAEzQUZW59557365 = -633730600;    double jEyCVIxFgiAEzQUZW58772432 = -722420506;    double jEyCVIxFgiAEzQUZW53389742 = -669796659;    double jEyCVIxFgiAEzQUZW11591087 = -313629014;     jEyCVIxFgiAEzQUZW33858163 = jEyCVIxFgiAEzQUZW8628218;     jEyCVIxFgiAEzQUZW8628218 = jEyCVIxFgiAEzQUZW78072012;     jEyCVIxFgiAEzQUZW78072012 = jEyCVIxFgiAEzQUZW26956141;     jEyCVIxFgiAEzQUZW26956141 = jEyCVIxFgiAEzQUZW77952534;     jEyCVIxFgiAEzQUZW77952534 = jEyCVIxFgiAEzQUZW48837176;     jEyCVIxFgiAEzQUZW48837176 = jEyCVIxFgiAEzQUZW47874466;     jEyCVIxFgiAEzQUZW47874466 = jEyCVIxFgiAEzQUZW76339993;     jEyCVIxFgiAEzQUZW76339993 = jEyCVIxFgiAEzQUZW17132982;     jEyCVIxFgiAEzQUZW17132982 = jEyCVIxFgiAEzQUZW98710679;     jEyCVIxFgiAEzQUZW98710679 = jEyCVIxFgiAEzQUZW76297414;     jEyCVIxFgiAEzQUZW76297414 = jEyCVIxFgiAEzQUZW2069088;     jEyCVIxFgiAEzQUZW2069088 = jEyCVIxFgiAEzQUZW15580581;     jEyCVIxFgiAEzQUZW15580581 = jEyCVIxFgiAEzQUZW71047655;     jEyCVIxFgiAEzQUZW71047655 = jEyCVIxFgiAEzQUZW95668753;     jEyCVIxFgiAEzQUZW95668753 = jEyCVIxFgiAEzQUZW43753141;     jEyCVIxFgiAEzQUZW43753141 = jEyCVIxFgiAEzQUZW41016971;     jEyCVIxFgiAEzQUZW41016971 = jEyCVIxFgiAEzQUZW35536604;     jEyCVIxFgiAEzQUZW35536604 = jEyCVIxFgiAEzQUZW47184235;     jEyCVIxFgiAEzQUZW47184235 = jEyCVIxFgiAEzQUZW20043241;     jEyCVIxFgiAEzQUZW20043241 = jEyCVIxFgiAEzQUZW54400051;     jEyCVIxFgiAEzQUZW54400051 = jEyCVIxFgiAEzQUZW40821465;     jEyCVIxFgiAEzQUZW40821465 = jEyCVIxFgiAEzQUZW78142063;     jEyCVIxFgiAEzQUZW78142063 = jEyCVIxFgiAEzQUZW20990834;     jEyCVIxFgiAEzQUZW20990834 = jEyCVIxFgiAEzQUZW57502075;     jEyCVIxFgiAEzQUZW57502075 = jEyCVIxFgiAEzQUZW20061425;     jEyCVIxFgiAEzQUZW20061425 = jEyCVIxFgiAEzQUZW96566129;     jEyCVIxFgiAEzQUZW96566129 = jEyCVIxFgiAEzQUZW14517582;     jEyCVIxFgiAEzQUZW14517582 = jEyCVIxFgiAEzQUZW22218365;     jEyCVIxFgiAEzQUZW22218365 = jEyCVIxFgiAEzQUZW74547873;     jEyCVIxFgiAEzQUZW74547873 = jEyCVIxFgiAEzQUZW69690233;     jEyCVIxFgiAEzQUZW69690233 = jEyCVIxFgiAEzQUZW35109079;     jEyCVIxFgiAEzQUZW35109079 = jEyCVIxFgiAEzQUZW69052659;     jEyCVIxFgiAEzQUZW69052659 = jEyCVIxFgiAEzQUZW11694272;     jEyCVIxFgiAEzQUZW11694272 = jEyCVIxFgiAEzQUZW67919023;     jEyCVIxFgiAEzQUZW67919023 = jEyCVIxFgiAEzQUZW95181881;     jEyCVIxFgiAEzQUZW95181881 = jEyCVIxFgiAEzQUZW2075216;     jEyCVIxFgiAEzQUZW2075216 = jEyCVIxFgiAEzQUZW28195304;     jEyCVIxFgiAEzQUZW28195304 = jEyCVIxFgiAEzQUZW85444825;     jEyCVIxFgiAEzQUZW85444825 = jEyCVIxFgiAEzQUZW47406588;     jEyCVIxFgiAEzQUZW47406588 = jEyCVIxFgiAEzQUZW86551604;     jEyCVIxFgiAEzQUZW86551604 = jEyCVIxFgiAEzQUZW13678994;     jEyCVIxFgiAEzQUZW13678994 = jEyCVIxFgiAEzQUZW1357541;     jEyCVIxFgiAEzQUZW1357541 = jEyCVIxFgiAEzQUZW82410188;     jEyCVIxFgiAEzQUZW82410188 = jEyCVIxFgiAEzQUZW52779423;     jEyCVIxFgiAEzQUZW52779423 = jEyCVIxFgiAEzQUZW95413791;     jEyCVIxFgiAEzQUZW95413791 = jEyCVIxFgiAEzQUZW6763749;     jEyCVIxFgiAEzQUZW6763749 = jEyCVIxFgiAEzQUZW30269157;     jEyCVIxFgiAEzQUZW30269157 = jEyCVIxFgiAEzQUZW59434701;     jEyCVIxFgiAEzQUZW59434701 = jEyCVIxFgiAEzQUZW3864501;     jEyCVIxFgiAEzQUZW3864501 = jEyCVIxFgiAEzQUZW8968632;     jEyCVIxFgiAEzQUZW8968632 = jEyCVIxFgiAEzQUZW48922473;     jEyCVIxFgiAEzQUZW48922473 = jEyCVIxFgiAEzQUZW35336419;     jEyCVIxFgiAEzQUZW35336419 = jEyCVIxFgiAEzQUZW30366116;     jEyCVIxFgiAEzQUZW30366116 = jEyCVIxFgiAEzQUZW81427198;     jEyCVIxFgiAEzQUZW81427198 = jEyCVIxFgiAEzQUZW93036698;     jEyCVIxFgiAEzQUZW93036698 = jEyCVIxFgiAEzQUZW30486154;     jEyCVIxFgiAEzQUZW30486154 = jEyCVIxFgiAEzQUZW57081179;     jEyCVIxFgiAEzQUZW57081179 = jEyCVIxFgiAEzQUZW69454065;     jEyCVIxFgiAEzQUZW69454065 = jEyCVIxFgiAEzQUZW57891109;     jEyCVIxFgiAEzQUZW57891109 = jEyCVIxFgiAEzQUZW52271047;     jEyCVIxFgiAEzQUZW52271047 = jEyCVIxFgiAEzQUZW33356885;     jEyCVIxFgiAEzQUZW33356885 = jEyCVIxFgiAEzQUZW54121629;     jEyCVIxFgiAEzQUZW54121629 = jEyCVIxFgiAEzQUZW42585108;     jEyCVIxFgiAEzQUZW42585108 = jEyCVIxFgiAEzQUZW29020446;     jEyCVIxFgiAEzQUZW29020446 = jEyCVIxFgiAEzQUZW41188336;     jEyCVIxFgiAEzQUZW41188336 = jEyCVIxFgiAEzQUZW33016429;     jEyCVIxFgiAEzQUZW33016429 = jEyCVIxFgiAEzQUZW3886310;     jEyCVIxFgiAEzQUZW3886310 = jEyCVIxFgiAEzQUZW3128633;     jEyCVIxFgiAEzQUZW3128633 = jEyCVIxFgiAEzQUZW486872;     jEyCVIxFgiAEzQUZW486872 = jEyCVIxFgiAEzQUZW41677926;     jEyCVIxFgiAEzQUZW41677926 = jEyCVIxFgiAEzQUZW12821667;     jEyCVIxFgiAEzQUZW12821667 = jEyCVIxFgiAEzQUZW50091779;     jEyCVIxFgiAEzQUZW50091779 = jEyCVIxFgiAEzQUZW99777646;     jEyCVIxFgiAEzQUZW99777646 = jEyCVIxFgiAEzQUZW33491636;     jEyCVIxFgiAEzQUZW33491636 = jEyCVIxFgiAEzQUZW40721058;     jEyCVIxFgiAEzQUZW40721058 = jEyCVIxFgiAEzQUZW39463925;     jEyCVIxFgiAEzQUZW39463925 = jEyCVIxFgiAEzQUZW95731875;     jEyCVIxFgiAEzQUZW95731875 = jEyCVIxFgiAEzQUZW68211411;     jEyCVIxFgiAEzQUZW68211411 = jEyCVIxFgiAEzQUZW62088284;     jEyCVIxFgiAEzQUZW62088284 = jEyCVIxFgiAEzQUZW13297677;     jEyCVIxFgiAEzQUZW13297677 = jEyCVIxFgiAEzQUZW66296972;     jEyCVIxFgiAEzQUZW66296972 = jEyCVIxFgiAEzQUZW55082880;     jEyCVIxFgiAEzQUZW55082880 = jEyCVIxFgiAEzQUZW18353865;     jEyCVIxFgiAEzQUZW18353865 = jEyCVIxFgiAEzQUZW65579241;     jEyCVIxFgiAEzQUZW65579241 = jEyCVIxFgiAEzQUZW20767761;     jEyCVIxFgiAEzQUZW20767761 = jEyCVIxFgiAEzQUZW99772659;     jEyCVIxFgiAEzQUZW99772659 = jEyCVIxFgiAEzQUZW38686543;     jEyCVIxFgiAEzQUZW38686543 = jEyCVIxFgiAEzQUZW30267074;     jEyCVIxFgiAEzQUZW30267074 = jEyCVIxFgiAEzQUZW74882325;     jEyCVIxFgiAEzQUZW74882325 = jEyCVIxFgiAEzQUZW64695727;     jEyCVIxFgiAEzQUZW64695727 = jEyCVIxFgiAEzQUZW44994036;     jEyCVIxFgiAEzQUZW44994036 = jEyCVIxFgiAEzQUZW58741239;     jEyCVIxFgiAEzQUZW58741239 = jEyCVIxFgiAEzQUZW27553716;     jEyCVIxFgiAEzQUZW27553716 = jEyCVIxFgiAEzQUZW95135541;     jEyCVIxFgiAEzQUZW95135541 = jEyCVIxFgiAEzQUZW53194720;     jEyCVIxFgiAEzQUZW53194720 = jEyCVIxFgiAEzQUZW59557365;     jEyCVIxFgiAEzQUZW59557365 = jEyCVIxFgiAEzQUZW58772432;     jEyCVIxFgiAEzQUZW58772432 = jEyCVIxFgiAEzQUZW53389742;     jEyCVIxFgiAEzQUZW53389742 = jEyCVIxFgiAEzQUZW11591087;     jEyCVIxFgiAEzQUZW11591087 = jEyCVIxFgiAEzQUZW33858163;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void oCTpfFJuechRoRRW3118848() {     double DgtceIcsWoLEcOHmv69317132 = -779905914;    double DgtceIcsWoLEcOHmv21900296 = -848899782;    double DgtceIcsWoLEcOHmv15458490 = -367121304;    double DgtceIcsWoLEcOHmv99179495 = -172724943;    double DgtceIcsWoLEcOHmv34237803 = -421546522;    double DgtceIcsWoLEcOHmv85060047 = -204731279;    double DgtceIcsWoLEcOHmv58531872 = -857588100;    double DgtceIcsWoLEcOHmv38471555 = -253087640;    double DgtceIcsWoLEcOHmv18650587 = -422712130;    double DgtceIcsWoLEcOHmv13613392 = -216919454;    double DgtceIcsWoLEcOHmv14479149 = -212707223;    double DgtceIcsWoLEcOHmv2637237 = -481823706;    double DgtceIcsWoLEcOHmv53815226 = -574445059;    double DgtceIcsWoLEcOHmv24188519 = -895339813;    double DgtceIcsWoLEcOHmv53863269 = 87102742;    double DgtceIcsWoLEcOHmv27294442 = -790096786;    double DgtceIcsWoLEcOHmv96852906 = -665960501;    double DgtceIcsWoLEcOHmv35296796 = -679982662;    double DgtceIcsWoLEcOHmv47734565 = -469498889;    double DgtceIcsWoLEcOHmv13880459 = -235781529;    double DgtceIcsWoLEcOHmv67486913 = -201702247;    double DgtceIcsWoLEcOHmv44242990 = -49007548;    double DgtceIcsWoLEcOHmv48667443 = -927820589;    double DgtceIcsWoLEcOHmv14783779 = -100115223;    double DgtceIcsWoLEcOHmv65520026 = -630282614;    double DgtceIcsWoLEcOHmv30759597 = -725054022;    double DgtceIcsWoLEcOHmv56986703 = -211399430;    double DgtceIcsWoLEcOHmv55246006 = -921853488;    double DgtceIcsWoLEcOHmv68886579 = -423031653;    double DgtceIcsWoLEcOHmv80081284 = -151241094;    double DgtceIcsWoLEcOHmv88273465 = -659179867;    double DgtceIcsWoLEcOHmv72932097 = -985364996;    double DgtceIcsWoLEcOHmv81270669 = -681832517;    double DgtceIcsWoLEcOHmv41442698 = -441870250;    double DgtceIcsWoLEcOHmv33727477 = -954500115;    double DgtceIcsWoLEcOHmv46742725 = 57311;    double DgtceIcsWoLEcOHmv51487032 = -433975121;    double DgtceIcsWoLEcOHmv79410707 = 74829966;    double DgtceIcsWoLEcOHmv15835387 = -33036093;    double DgtceIcsWoLEcOHmv11678121 = -801507506;    double DgtceIcsWoLEcOHmv20003212 = -527891181;    double DgtceIcsWoLEcOHmv3044121 = -988192539;    double DgtceIcsWoLEcOHmv90740918 = -93248091;    double DgtceIcsWoLEcOHmv22541527 = -619162146;    double DgtceIcsWoLEcOHmv52795882 = -592106094;    double DgtceIcsWoLEcOHmv41375120 = -709567697;    double DgtceIcsWoLEcOHmv26108912 = -866619409;    double DgtceIcsWoLEcOHmv47995469 = -601396566;    double DgtceIcsWoLEcOHmv82327367 = -851173760;    double DgtceIcsWoLEcOHmv28394090 = -290250679;    double DgtceIcsWoLEcOHmv89739355 = -50148690;    double DgtceIcsWoLEcOHmv32407945 = -951966743;    double DgtceIcsWoLEcOHmv3957568 = -956613334;    double DgtceIcsWoLEcOHmv46357962 = -695000871;    double DgtceIcsWoLEcOHmv35631934 = -574560631;    double DgtceIcsWoLEcOHmv25074142 = -630898367;    double DgtceIcsWoLEcOHmv73232852 = -921079193;    double DgtceIcsWoLEcOHmv674711 = -167006081;    double DgtceIcsWoLEcOHmv33659469 = -542442330;    double DgtceIcsWoLEcOHmv3478207 = -696492500;    double DgtceIcsWoLEcOHmv28073345 = -993331849;    double DgtceIcsWoLEcOHmv3285866 = -935734613;    double DgtceIcsWoLEcOHmv69584975 = -830055987;    double DgtceIcsWoLEcOHmv38569303 = -171471036;    double DgtceIcsWoLEcOHmv25339927 = -557739588;    double DgtceIcsWoLEcOHmv41547052 = -227342228;    double DgtceIcsWoLEcOHmv21366567 = -799991189;    double DgtceIcsWoLEcOHmv12372529 = -32574809;    double DgtceIcsWoLEcOHmv90461042 = -940839699;    double DgtceIcsWoLEcOHmv7120545 = -912954569;    double DgtceIcsWoLEcOHmv75807410 = -256121665;    double DgtceIcsWoLEcOHmv17442200 = -640790468;    double DgtceIcsWoLEcOHmv19461409 = -546946569;    double DgtceIcsWoLEcOHmv36056444 = -667991384;    double DgtceIcsWoLEcOHmv93877246 = -707890349;    double DgtceIcsWoLEcOHmv64442793 = -213509709;    double DgtceIcsWoLEcOHmv53502071 = -955759457;    double DgtceIcsWoLEcOHmv26125917 = -208658444;    double DgtceIcsWoLEcOHmv61987897 = -508009130;    double DgtceIcsWoLEcOHmv24144907 = -920714917;    double DgtceIcsWoLEcOHmv4650685 = -858434614;    double DgtceIcsWoLEcOHmv8991235 = -610002865;    double DgtceIcsWoLEcOHmv72918638 = 29320272;    double DgtceIcsWoLEcOHmv40492490 = -32780975;    double DgtceIcsWoLEcOHmv90341928 = -1092405;    double DgtceIcsWoLEcOHmv55865520 = -707213125;    double DgtceIcsWoLEcOHmv68974529 = 71248338;    double DgtceIcsWoLEcOHmv34912707 = -986831647;    double DgtceIcsWoLEcOHmv5810764 = -867309620;    double DgtceIcsWoLEcOHmv8653335 = -223601748;    double DgtceIcsWoLEcOHmv73509872 = -78863496;    double DgtceIcsWoLEcOHmv50812321 = -166969041;    double DgtceIcsWoLEcOHmv45751238 = -382727704;    double DgtceIcsWoLEcOHmv12357181 = -336543593;    double DgtceIcsWoLEcOHmv83604776 = -808175658;    double DgtceIcsWoLEcOHmv16717347 = -592156569;    double DgtceIcsWoLEcOHmv33459145 = -58136553;    double DgtceIcsWoLEcOHmv52171616 = -921777055;    double DgtceIcsWoLEcOHmv97201600 = 38577442;    double DgtceIcsWoLEcOHmv11248830 = -779905914;     DgtceIcsWoLEcOHmv69317132 = DgtceIcsWoLEcOHmv21900296;     DgtceIcsWoLEcOHmv21900296 = DgtceIcsWoLEcOHmv15458490;     DgtceIcsWoLEcOHmv15458490 = DgtceIcsWoLEcOHmv99179495;     DgtceIcsWoLEcOHmv99179495 = DgtceIcsWoLEcOHmv34237803;     DgtceIcsWoLEcOHmv34237803 = DgtceIcsWoLEcOHmv85060047;     DgtceIcsWoLEcOHmv85060047 = DgtceIcsWoLEcOHmv58531872;     DgtceIcsWoLEcOHmv58531872 = DgtceIcsWoLEcOHmv38471555;     DgtceIcsWoLEcOHmv38471555 = DgtceIcsWoLEcOHmv18650587;     DgtceIcsWoLEcOHmv18650587 = DgtceIcsWoLEcOHmv13613392;     DgtceIcsWoLEcOHmv13613392 = DgtceIcsWoLEcOHmv14479149;     DgtceIcsWoLEcOHmv14479149 = DgtceIcsWoLEcOHmv2637237;     DgtceIcsWoLEcOHmv2637237 = DgtceIcsWoLEcOHmv53815226;     DgtceIcsWoLEcOHmv53815226 = DgtceIcsWoLEcOHmv24188519;     DgtceIcsWoLEcOHmv24188519 = DgtceIcsWoLEcOHmv53863269;     DgtceIcsWoLEcOHmv53863269 = DgtceIcsWoLEcOHmv27294442;     DgtceIcsWoLEcOHmv27294442 = DgtceIcsWoLEcOHmv96852906;     DgtceIcsWoLEcOHmv96852906 = DgtceIcsWoLEcOHmv35296796;     DgtceIcsWoLEcOHmv35296796 = DgtceIcsWoLEcOHmv47734565;     DgtceIcsWoLEcOHmv47734565 = DgtceIcsWoLEcOHmv13880459;     DgtceIcsWoLEcOHmv13880459 = DgtceIcsWoLEcOHmv67486913;     DgtceIcsWoLEcOHmv67486913 = DgtceIcsWoLEcOHmv44242990;     DgtceIcsWoLEcOHmv44242990 = DgtceIcsWoLEcOHmv48667443;     DgtceIcsWoLEcOHmv48667443 = DgtceIcsWoLEcOHmv14783779;     DgtceIcsWoLEcOHmv14783779 = DgtceIcsWoLEcOHmv65520026;     DgtceIcsWoLEcOHmv65520026 = DgtceIcsWoLEcOHmv30759597;     DgtceIcsWoLEcOHmv30759597 = DgtceIcsWoLEcOHmv56986703;     DgtceIcsWoLEcOHmv56986703 = DgtceIcsWoLEcOHmv55246006;     DgtceIcsWoLEcOHmv55246006 = DgtceIcsWoLEcOHmv68886579;     DgtceIcsWoLEcOHmv68886579 = DgtceIcsWoLEcOHmv80081284;     DgtceIcsWoLEcOHmv80081284 = DgtceIcsWoLEcOHmv88273465;     DgtceIcsWoLEcOHmv88273465 = DgtceIcsWoLEcOHmv72932097;     DgtceIcsWoLEcOHmv72932097 = DgtceIcsWoLEcOHmv81270669;     DgtceIcsWoLEcOHmv81270669 = DgtceIcsWoLEcOHmv41442698;     DgtceIcsWoLEcOHmv41442698 = DgtceIcsWoLEcOHmv33727477;     DgtceIcsWoLEcOHmv33727477 = DgtceIcsWoLEcOHmv46742725;     DgtceIcsWoLEcOHmv46742725 = DgtceIcsWoLEcOHmv51487032;     DgtceIcsWoLEcOHmv51487032 = DgtceIcsWoLEcOHmv79410707;     DgtceIcsWoLEcOHmv79410707 = DgtceIcsWoLEcOHmv15835387;     DgtceIcsWoLEcOHmv15835387 = DgtceIcsWoLEcOHmv11678121;     DgtceIcsWoLEcOHmv11678121 = DgtceIcsWoLEcOHmv20003212;     DgtceIcsWoLEcOHmv20003212 = DgtceIcsWoLEcOHmv3044121;     DgtceIcsWoLEcOHmv3044121 = DgtceIcsWoLEcOHmv90740918;     DgtceIcsWoLEcOHmv90740918 = DgtceIcsWoLEcOHmv22541527;     DgtceIcsWoLEcOHmv22541527 = DgtceIcsWoLEcOHmv52795882;     DgtceIcsWoLEcOHmv52795882 = DgtceIcsWoLEcOHmv41375120;     DgtceIcsWoLEcOHmv41375120 = DgtceIcsWoLEcOHmv26108912;     DgtceIcsWoLEcOHmv26108912 = DgtceIcsWoLEcOHmv47995469;     DgtceIcsWoLEcOHmv47995469 = DgtceIcsWoLEcOHmv82327367;     DgtceIcsWoLEcOHmv82327367 = DgtceIcsWoLEcOHmv28394090;     DgtceIcsWoLEcOHmv28394090 = DgtceIcsWoLEcOHmv89739355;     DgtceIcsWoLEcOHmv89739355 = DgtceIcsWoLEcOHmv32407945;     DgtceIcsWoLEcOHmv32407945 = DgtceIcsWoLEcOHmv3957568;     DgtceIcsWoLEcOHmv3957568 = DgtceIcsWoLEcOHmv46357962;     DgtceIcsWoLEcOHmv46357962 = DgtceIcsWoLEcOHmv35631934;     DgtceIcsWoLEcOHmv35631934 = DgtceIcsWoLEcOHmv25074142;     DgtceIcsWoLEcOHmv25074142 = DgtceIcsWoLEcOHmv73232852;     DgtceIcsWoLEcOHmv73232852 = DgtceIcsWoLEcOHmv674711;     DgtceIcsWoLEcOHmv674711 = DgtceIcsWoLEcOHmv33659469;     DgtceIcsWoLEcOHmv33659469 = DgtceIcsWoLEcOHmv3478207;     DgtceIcsWoLEcOHmv3478207 = DgtceIcsWoLEcOHmv28073345;     DgtceIcsWoLEcOHmv28073345 = DgtceIcsWoLEcOHmv3285866;     DgtceIcsWoLEcOHmv3285866 = DgtceIcsWoLEcOHmv69584975;     DgtceIcsWoLEcOHmv69584975 = DgtceIcsWoLEcOHmv38569303;     DgtceIcsWoLEcOHmv38569303 = DgtceIcsWoLEcOHmv25339927;     DgtceIcsWoLEcOHmv25339927 = DgtceIcsWoLEcOHmv41547052;     DgtceIcsWoLEcOHmv41547052 = DgtceIcsWoLEcOHmv21366567;     DgtceIcsWoLEcOHmv21366567 = DgtceIcsWoLEcOHmv12372529;     DgtceIcsWoLEcOHmv12372529 = DgtceIcsWoLEcOHmv90461042;     DgtceIcsWoLEcOHmv90461042 = DgtceIcsWoLEcOHmv7120545;     DgtceIcsWoLEcOHmv7120545 = DgtceIcsWoLEcOHmv75807410;     DgtceIcsWoLEcOHmv75807410 = DgtceIcsWoLEcOHmv17442200;     DgtceIcsWoLEcOHmv17442200 = DgtceIcsWoLEcOHmv19461409;     DgtceIcsWoLEcOHmv19461409 = DgtceIcsWoLEcOHmv36056444;     DgtceIcsWoLEcOHmv36056444 = DgtceIcsWoLEcOHmv93877246;     DgtceIcsWoLEcOHmv93877246 = DgtceIcsWoLEcOHmv64442793;     DgtceIcsWoLEcOHmv64442793 = DgtceIcsWoLEcOHmv53502071;     DgtceIcsWoLEcOHmv53502071 = DgtceIcsWoLEcOHmv26125917;     DgtceIcsWoLEcOHmv26125917 = DgtceIcsWoLEcOHmv61987897;     DgtceIcsWoLEcOHmv61987897 = DgtceIcsWoLEcOHmv24144907;     DgtceIcsWoLEcOHmv24144907 = DgtceIcsWoLEcOHmv4650685;     DgtceIcsWoLEcOHmv4650685 = DgtceIcsWoLEcOHmv8991235;     DgtceIcsWoLEcOHmv8991235 = DgtceIcsWoLEcOHmv72918638;     DgtceIcsWoLEcOHmv72918638 = DgtceIcsWoLEcOHmv40492490;     DgtceIcsWoLEcOHmv40492490 = DgtceIcsWoLEcOHmv90341928;     DgtceIcsWoLEcOHmv90341928 = DgtceIcsWoLEcOHmv55865520;     DgtceIcsWoLEcOHmv55865520 = DgtceIcsWoLEcOHmv68974529;     DgtceIcsWoLEcOHmv68974529 = DgtceIcsWoLEcOHmv34912707;     DgtceIcsWoLEcOHmv34912707 = DgtceIcsWoLEcOHmv5810764;     DgtceIcsWoLEcOHmv5810764 = DgtceIcsWoLEcOHmv8653335;     DgtceIcsWoLEcOHmv8653335 = DgtceIcsWoLEcOHmv73509872;     DgtceIcsWoLEcOHmv73509872 = DgtceIcsWoLEcOHmv50812321;     DgtceIcsWoLEcOHmv50812321 = DgtceIcsWoLEcOHmv45751238;     DgtceIcsWoLEcOHmv45751238 = DgtceIcsWoLEcOHmv12357181;     DgtceIcsWoLEcOHmv12357181 = DgtceIcsWoLEcOHmv83604776;     DgtceIcsWoLEcOHmv83604776 = DgtceIcsWoLEcOHmv16717347;     DgtceIcsWoLEcOHmv16717347 = DgtceIcsWoLEcOHmv33459145;     DgtceIcsWoLEcOHmv33459145 = DgtceIcsWoLEcOHmv52171616;     DgtceIcsWoLEcOHmv52171616 = DgtceIcsWoLEcOHmv97201600;     DgtceIcsWoLEcOHmv97201600 = DgtceIcsWoLEcOHmv11248830;     DgtceIcsWoLEcOHmv11248830 = DgtceIcsWoLEcOHmv69317132;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void xYsGdYmjZqQrpNJY18167916() {     double RtbxVdOMQSBFsWieT75434238 = -891807803;    double RtbxVdOMQSBFsWieT65273575 = -558499563;    double RtbxVdOMQSBFsWieT31500673 = -517954225;    double RtbxVdOMQSBFsWieT26254277 = -363473573;    double RtbxVdOMQSBFsWieT3133292 = -942196394;    double RtbxVdOMQSBFsWieT76477250 = -469227514;    double RtbxVdOMQSBFsWieT19020838 = -996553077;    double RtbxVdOMQSBFsWieT88231445 = -539961553;    double RtbxVdOMQSBFsWieT83293082 = -190744796;    double RtbxVdOMQSBFsWieT54843051 = -545894932;    double RtbxVdOMQSBFsWieT74589439 = -90144809;    double RtbxVdOMQSBFsWieT34460279 = -926229907;    double RtbxVdOMQSBFsWieT40024345 = 85663852;    double RtbxVdOMQSBFsWieT7332196 = -327578381;    double RtbxVdOMQSBFsWieT49605696 = -603359424;    double RtbxVdOMQSBFsWieT68799835 = -763096329;    double RtbxVdOMQSBFsWieT90688909 = -182505614;    double RtbxVdOMQSBFsWieT32349949 = -345505305;    double RtbxVdOMQSBFsWieT9564407 = -875267986;    double RtbxVdOMQSBFsWieT94430623 = -592684449;    double RtbxVdOMQSBFsWieT56040107 = -57189703;    double RtbxVdOMQSBFsWieT30764014 = 43062736;    double RtbxVdOMQSBFsWieT84258235 = -674338076;    double RtbxVdOMQSBFsWieT98776152 = -322420211;    double RtbxVdOMQSBFsWieT9219677 = 96521353;    double RtbxVdOMQSBFsWieT55481272 = -603709346;    double RtbxVdOMQSBFsWieT55446481 = -112061633;    double RtbxVdOMQSBFsWieT68353131 = -987782441;    double RtbxVdOMQSBFsWieT35018835 = -645414358;    double RtbxVdOMQSBFsWieT48696921 = 31288039;    double RtbxVdOMQSBFsWieT80516014 = -572500805;    double RtbxVdOMQSBFsWieT70542718 = -694811812;    double RtbxVdOMQSBFsWieT90981618 = -574445888;    double RtbxVdOMQSBFsWieT36273262 = -193412016;    double RtbxVdOMQSBFsWieT95202128 = -493233220;    double RtbxVdOMQSBFsWieT15468639 = -213968796;    double RtbxVdOMQSBFsWieT32748966 = -387610991;    double RtbxVdOMQSBFsWieT13500920 = -796676043;    double RtbxVdOMQSBFsWieT89445640 = -26959590;    double RtbxVdOMQSBFsWieT99180058 = -174946728;    double RtbxVdOMQSBFsWieT25672436 = -593165075;    double RtbxVdOMQSBFsWieT35194577 = -158334817;    double RtbxVdOMQSBFsWieT68807897 = -174052675;    double RtbxVdOMQSBFsWieT9949267 = -638505084;    double RtbxVdOMQSBFsWieT64748346 = -254004248;    double RtbxVdOMQSBFsWieT53314315 = -694783813;    double RtbxVdOMQSBFsWieT43887486 = -985394947;    double RtbxVdOMQSBFsWieT55952410 = -611292099;    double RtbxVdOMQSBFsWieT92003096 = -936910749;    double RtbxVdOMQSBFsWieT41305743 = -377261487;    double RtbxVdOMQSBFsWieT25440034 = -992442361;    double RtbxVdOMQSBFsWieT43217665 = -752693063;    double RtbxVdOMQSBFsWieT52206025 = -748402922;    double RtbxVdOMQSBFsWieT42326843 = 84528984;    double RtbxVdOMQSBFsWieT62127809 = -175092414;    double RtbxVdOMQSBFsWieT44670225 = -834870540;    double RtbxVdOMQSBFsWieT81015340 = -884161487;    double RtbxVdOMQSBFsWieT32724520 = -95534014;    double RtbxVdOMQSBFsWieT17034600 = -359994927;    double RtbxVdOMQSBFsWieT47652020 = -238487048;    double RtbxVdOMQSBFsWieT21030769 = -257165881;    double RtbxVdOMQSBFsWieT50667706 = 91229364;    double RtbxVdOMQSBFsWieT53212610 = -894547196;    double RtbxVdOMQSBFsWieT34596161 = -122032835;    double RtbxVdOMQSBFsWieT74327036 = -973394127;    double RtbxVdOMQSBFsWieT4046721 = -395332998;    double RtbxVdOMQSBFsWieT43478661 = -251784020;    double RtbxVdOMQSBFsWieT3751083 = -720924133;    double RtbxVdOMQSBFsWieT12130067 = -834345161;    double RtbxVdOMQSBFsWieT34137058 = -289390628;    double RtbxVdOMQSBFsWieT36050869 = -275485339;    double RtbxVdOMQSBFsWieT77187990 = -385829571;    double RtbxVdOMQSBFsWieT42904308 = -218545715;    double RtbxVdOMQSBFsWieT10384348 = -600321258;    double RtbxVdOMQSBFsWieT68758188 = -999519374;    double RtbxVdOMQSBFsWieT20845530 = -898854886;    double RtbxVdOMQSBFsWieT61956116 = -782884589;    double RtbxVdOMQSBFsWieT74308968 = 64167007;    double RtbxVdOMQSBFsWieT34027806 = 31584036;    double RtbxVdOMQSBFsWieT55905362 = -208694834;    double RtbxVdOMQSBFsWieT11593786 = -618314399;    double RtbxVdOMQSBFsWieT99494071 = -500769535;    double RtbxVdOMQSBFsWieT76350035 = 49128307;    double RtbxVdOMQSBFsWieT93713092 = -168152872;    double RtbxVdOMQSBFsWieT23256887 = 23730399;    double RtbxVdOMQSBFsWieT37298350 = -819807743;    double RtbxVdOMQSBFsWieT18336694 = -946408890;    double RtbxVdOMQSBFsWieT48654775 = -558974872;    double RtbxVdOMQSBFsWieT74145453 = 81680398;    double RtbxVdOMQSBFsWieT50531904 = -658362681;    double RtbxVdOMQSBFsWieT34453298 = -329807310;    double RtbxVdOMQSBFsWieT24447 = -192076977;    double RtbxVdOMQSBFsWieT96466320 = -336681116;    double RtbxVdOMQSBFsWieT41793621 = -788472542;    double RtbxVdOMQSBFsWieT78149289 = -917780848;    double RtbxVdOMQSBFsWieT75004729 = -584394439;    double RtbxVdOMQSBFsWieT81981966 = -263787622;    double RtbxVdOMQSBFsWieT34211736 = 47980159;    double RtbxVdOMQSBFsWieT35622231 = -665110957;    double RtbxVdOMQSBFsWieT60701626 = -891807803;     RtbxVdOMQSBFsWieT75434238 = RtbxVdOMQSBFsWieT65273575;     RtbxVdOMQSBFsWieT65273575 = RtbxVdOMQSBFsWieT31500673;     RtbxVdOMQSBFsWieT31500673 = RtbxVdOMQSBFsWieT26254277;     RtbxVdOMQSBFsWieT26254277 = RtbxVdOMQSBFsWieT3133292;     RtbxVdOMQSBFsWieT3133292 = RtbxVdOMQSBFsWieT76477250;     RtbxVdOMQSBFsWieT76477250 = RtbxVdOMQSBFsWieT19020838;     RtbxVdOMQSBFsWieT19020838 = RtbxVdOMQSBFsWieT88231445;     RtbxVdOMQSBFsWieT88231445 = RtbxVdOMQSBFsWieT83293082;     RtbxVdOMQSBFsWieT83293082 = RtbxVdOMQSBFsWieT54843051;     RtbxVdOMQSBFsWieT54843051 = RtbxVdOMQSBFsWieT74589439;     RtbxVdOMQSBFsWieT74589439 = RtbxVdOMQSBFsWieT34460279;     RtbxVdOMQSBFsWieT34460279 = RtbxVdOMQSBFsWieT40024345;     RtbxVdOMQSBFsWieT40024345 = RtbxVdOMQSBFsWieT7332196;     RtbxVdOMQSBFsWieT7332196 = RtbxVdOMQSBFsWieT49605696;     RtbxVdOMQSBFsWieT49605696 = RtbxVdOMQSBFsWieT68799835;     RtbxVdOMQSBFsWieT68799835 = RtbxVdOMQSBFsWieT90688909;     RtbxVdOMQSBFsWieT90688909 = RtbxVdOMQSBFsWieT32349949;     RtbxVdOMQSBFsWieT32349949 = RtbxVdOMQSBFsWieT9564407;     RtbxVdOMQSBFsWieT9564407 = RtbxVdOMQSBFsWieT94430623;     RtbxVdOMQSBFsWieT94430623 = RtbxVdOMQSBFsWieT56040107;     RtbxVdOMQSBFsWieT56040107 = RtbxVdOMQSBFsWieT30764014;     RtbxVdOMQSBFsWieT30764014 = RtbxVdOMQSBFsWieT84258235;     RtbxVdOMQSBFsWieT84258235 = RtbxVdOMQSBFsWieT98776152;     RtbxVdOMQSBFsWieT98776152 = RtbxVdOMQSBFsWieT9219677;     RtbxVdOMQSBFsWieT9219677 = RtbxVdOMQSBFsWieT55481272;     RtbxVdOMQSBFsWieT55481272 = RtbxVdOMQSBFsWieT55446481;     RtbxVdOMQSBFsWieT55446481 = RtbxVdOMQSBFsWieT68353131;     RtbxVdOMQSBFsWieT68353131 = RtbxVdOMQSBFsWieT35018835;     RtbxVdOMQSBFsWieT35018835 = RtbxVdOMQSBFsWieT48696921;     RtbxVdOMQSBFsWieT48696921 = RtbxVdOMQSBFsWieT80516014;     RtbxVdOMQSBFsWieT80516014 = RtbxVdOMQSBFsWieT70542718;     RtbxVdOMQSBFsWieT70542718 = RtbxVdOMQSBFsWieT90981618;     RtbxVdOMQSBFsWieT90981618 = RtbxVdOMQSBFsWieT36273262;     RtbxVdOMQSBFsWieT36273262 = RtbxVdOMQSBFsWieT95202128;     RtbxVdOMQSBFsWieT95202128 = RtbxVdOMQSBFsWieT15468639;     RtbxVdOMQSBFsWieT15468639 = RtbxVdOMQSBFsWieT32748966;     RtbxVdOMQSBFsWieT32748966 = RtbxVdOMQSBFsWieT13500920;     RtbxVdOMQSBFsWieT13500920 = RtbxVdOMQSBFsWieT89445640;     RtbxVdOMQSBFsWieT89445640 = RtbxVdOMQSBFsWieT99180058;     RtbxVdOMQSBFsWieT99180058 = RtbxVdOMQSBFsWieT25672436;     RtbxVdOMQSBFsWieT25672436 = RtbxVdOMQSBFsWieT35194577;     RtbxVdOMQSBFsWieT35194577 = RtbxVdOMQSBFsWieT68807897;     RtbxVdOMQSBFsWieT68807897 = RtbxVdOMQSBFsWieT9949267;     RtbxVdOMQSBFsWieT9949267 = RtbxVdOMQSBFsWieT64748346;     RtbxVdOMQSBFsWieT64748346 = RtbxVdOMQSBFsWieT53314315;     RtbxVdOMQSBFsWieT53314315 = RtbxVdOMQSBFsWieT43887486;     RtbxVdOMQSBFsWieT43887486 = RtbxVdOMQSBFsWieT55952410;     RtbxVdOMQSBFsWieT55952410 = RtbxVdOMQSBFsWieT92003096;     RtbxVdOMQSBFsWieT92003096 = RtbxVdOMQSBFsWieT41305743;     RtbxVdOMQSBFsWieT41305743 = RtbxVdOMQSBFsWieT25440034;     RtbxVdOMQSBFsWieT25440034 = RtbxVdOMQSBFsWieT43217665;     RtbxVdOMQSBFsWieT43217665 = RtbxVdOMQSBFsWieT52206025;     RtbxVdOMQSBFsWieT52206025 = RtbxVdOMQSBFsWieT42326843;     RtbxVdOMQSBFsWieT42326843 = RtbxVdOMQSBFsWieT62127809;     RtbxVdOMQSBFsWieT62127809 = RtbxVdOMQSBFsWieT44670225;     RtbxVdOMQSBFsWieT44670225 = RtbxVdOMQSBFsWieT81015340;     RtbxVdOMQSBFsWieT81015340 = RtbxVdOMQSBFsWieT32724520;     RtbxVdOMQSBFsWieT32724520 = RtbxVdOMQSBFsWieT17034600;     RtbxVdOMQSBFsWieT17034600 = RtbxVdOMQSBFsWieT47652020;     RtbxVdOMQSBFsWieT47652020 = RtbxVdOMQSBFsWieT21030769;     RtbxVdOMQSBFsWieT21030769 = RtbxVdOMQSBFsWieT50667706;     RtbxVdOMQSBFsWieT50667706 = RtbxVdOMQSBFsWieT53212610;     RtbxVdOMQSBFsWieT53212610 = RtbxVdOMQSBFsWieT34596161;     RtbxVdOMQSBFsWieT34596161 = RtbxVdOMQSBFsWieT74327036;     RtbxVdOMQSBFsWieT74327036 = RtbxVdOMQSBFsWieT4046721;     RtbxVdOMQSBFsWieT4046721 = RtbxVdOMQSBFsWieT43478661;     RtbxVdOMQSBFsWieT43478661 = RtbxVdOMQSBFsWieT3751083;     RtbxVdOMQSBFsWieT3751083 = RtbxVdOMQSBFsWieT12130067;     RtbxVdOMQSBFsWieT12130067 = RtbxVdOMQSBFsWieT34137058;     RtbxVdOMQSBFsWieT34137058 = RtbxVdOMQSBFsWieT36050869;     RtbxVdOMQSBFsWieT36050869 = RtbxVdOMQSBFsWieT77187990;     RtbxVdOMQSBFsWieT77187990 = RtbxVdOMQSBFsWieT42904308;     RtbxVdOMQSBFsWieT42904308 = RtbxVdOMQSBFsWieT10384348;     RtbxVdOMQSBFsWieT10384348 = RtbxVdOMQSBFsWieT68758188;     RtbxVdOMQSBFsWieT68758188 = RtbxVdOMQSBFsWieT20845530;     RtbxVdOMQSBFsWieT20845530 = RtbxVdOMQSBFsWieT61956116;     RtbxVdOMQSBFsWieT61956116 = RtbxVdOMQSBFsWieT74308968;     RtbxVdOMQSBFsWieT74308968 = RtbxVdOMQSBFsWieT34027806;     RtbxVdOMQSBFsWieT34027806 = RtbxVdOMQSBFsWieT55905362;     RtbxVdOMQSBFsWieT55905362 = RtbxVdOMQSBFsWieT11593786;     RtbxVdOMQSBFsWieT11593786 = RtbxVdOMQSBFsWieT99494071;     RtbxVdOMQSBFsWieT99494071 = RtbxVdOMQSBFsWieT76350035;     RtbxVdOMQSBFsWieT76350035 = RtbxVdOMQSBFsWieT93713092;     RtbxVdOMQSBFsWieT93713092 = RtbxVdOMQSBFsWieT23256887;     RtbxVdOMQSBFsWieT23256887 = RtbxVdOMQSBFsWieT37298350;     RtbxVdOMQSBFsWieT37298350 = RtbxVdOMQSBFsWieT18336694;     RtbxVdOMQSBFsWieT18336694 = RtbxVdOMQSBFsWieT48654775;     RtbxVdOMQSBFsWieT48654775 = RtbxVdOMQSBFsWieT74145453;     RtbxVdOMQSBFsWieT74145453 = RtbxVdOMQSBFsWieT50531904;     RtbxVdOMQSBFsWieT50531904 = RtbxVdOMQSBFsWieT34453298;     RtbxVdOMQSBFsWieT34453298 = RtbxVdOMQSBFsWieT24447;     RtbxVdOMQSBFsWieT24447 = RtbxVdOMQSBFsWieT96466320;     RtbxVdOMQSBFsWieT96466320 = RtbxVdOMQSBFsWieT41793621;     RtbxVdOMQSBFsWieT41793621 = RtbxVdOMQSBFsWieT78149289;     RtbxVdOMQSBFsWieT78149289 = RtbxVdOMQSBFsWieT75004729;     RtbxVdOMQSBFsWieT75004729 = RtbxVdOMQSBFsWieT81981966;     RtbxVdOMQSBFsWieT81981966 = RtbxVdOMQSBFsWieT34211736;     RtbxVdOMQSBFsWieT34211736 = RtbxVdOMQSBFsWieT35622231;     RtbxVdOMQSBFsWieT35622231 = RtbxVdOMQSBFsWieT60701626;     RtbxVdOMQSBFsWieT60701626 = RtbxVdOMQSBFsWieT75434238;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void YwccSpuNoWnvPKaj85459515() {     double uIfTRNWFCjGMXTqoh10893207 = -258084703;    double uIfTRNWFCjGMXTqoh78545654 = -439051169;    double uIfTRNWFCjGMXTqoh68887150 = -183621300;    double uIfTRNWFCjGMXTqoh98477631 = -184624645;    double uIfTRNWFCjGMXTqoh59418561 = 52204761;    double uIfTRNWFCjGMXTqoh12700123 = -731575409;    double uIfTRNWFCjGMXTqoh29678244 = -841278811;    double uIfTRNWFCjGMXTqoh50363006 = -184195694;    double uIfTRNWFCjGMXTqoh84810687 = -679076203;    double uIfTRNWFCjGMXTqoh69745763 = -210581930;    double uIfTRNWFCjGMXTqoh12771175 = -992152539;    double uIfTRNWFCjGMXTqoh35028427 = -790350679;    double uIfTRNWFCjGMXTqoh78258990 = -466348653;    double uIfTRNWFCjGMXTqoh60473059 = -303941790;    double uIfTRNWFCjGMXTqoh7800213 = -6784439;    double uIfTRNWFCjGMXTqoh52341136 = -398907936;    double uIfTRNWFCjGMXTqoh46524846 = -709480167;    double uIfTRNWFCjGMXTqoh32110140 = -418933373;    double uIfTRNWFCjGMXTqoh10114737 = -91831407;    double uIfTRNWFCjGMXTqoh88267842 = 80754485;    double uIfTRNWFCjGMXTqoh69126970 = -814464392;    double uIfTRNWFCjGMXTqoh34185538 = -43573558;    double uIfTRNWFCjGMXTqoh54783615 = -311970260;    double uIfTRNWFCjGMXTqoh92569098 = -833244815;    double uIfTRNWFCjGMXTqoh17237628 = 72536155;    double uIfTRNWFCjGMXTqoh66179443 = -292286153;    double uIfTRNWFCjGMXTqoh15867056 = 81654226;    double uIfTRNWFCjGMXTqoh9081557 = -57554616;    double uIfTRNWFCjGMXTqoh81687049 = -119509160;    double uIfTRNWFCjGMXTqoh54230332 = -3219132;    double uIfTRNWFCjGMXTqoh99099246 = -24481292;    double uIfTRNWFCjGMXTqoh8365738 = -467783226;    double uIfTRNWFCjGMXTqoh3199629 = -722041865;    double uIfTRNWFCjGMXTqoh66021688 = -258105829;    double uIfTRNWFCjGMXTqoh61010582 = -444333471;    double uIfTRNWFCjGMXTqoh67029482 = -428534102;    double uIfTRNWFCjGMXTqoh82160782 = -252307031;    double uIfTRNWFCjGMXTqoh64716323 = -779331624;    double uIfTRNWFCjGMXTqoh19836202 = -777835867;    double uIfTRNWFCjGMXTqoh63451591 = -740209275;    double uIfTRNWFCjGMXTqoh59124044 = -362363989;    double uIfTRNWFCjGMXTqoh24559704 = -852880108;    double uIfTRNWFCjGMXTqoh58191275 = -827233461;    double uIfTRNWFCjGMXTqoh50080605 = -142837494;    double uIfTRNWFCjGMXTqoh64764805 = -80515409;    double uIfTRNWFCjGMXTqoh99275643 = -733707445;    double uIfTRNWFCjGMXTqoh63232650 = -369381307;    double uIfTRNWFCjGMXTqoh73678721 = -777818692;    double uIfTRNWFCjGMXTqoh14895763 = -600647629;    double uIfTRNWFCjGMXTqoh65835332 = -868288302;    double uIfTRNWFCjGMXTqoh6210758 = -866718919;    double uIfTRNWFCjGMXTqoh26703137 = -966345409;    double uIfTRNWFCjGMXTqoh20827174 = -504448476;    double uIfTRNWFCjGMXTqoh58318689 = -106979515;    double uIfTRNWFCjGMXTqoh16332546 = -521582559;    double uIfTRNWFCjGMXTqoh76707668 = -114511145;    double uIfTRNWFCjGMXTqoh23762039 = -27080910;    double uIfTRNWFCjGMXTqoh76318051 = -350376485;    double uIfTRNWFCjGMXTqoh81240003 = -157160800;    double uIfTRNWFCjGMXTqoh93239117 = -655509087;    double uIfTRNWFCjGMXTqoh96833066 = -713229635;    double uIfTRNWFCjGMXTqoh20596688 = -683724196;    double uIfTRNWFCjGMXTqoh68675956 = 35313466;    double uIfTRNWFCjGMXTqoh30580356 = -575857071;    double uIfTRNWFCjGMXTqoh70646517 = -86100638;    double uIfTRNWFCjGMXTqoh4405438 = -424369313;    double uIfTRNWFCjGMXTqoh31828798 = 31691186;    double uIfTRNWFCjGMXTqoh12237302 = -108242824;    double uIfTRNWFCjGMXTqoh99462476 = -859608320;    double uIfTRNWFCjGMXTqoh40770730 = -578250337;    double uIfTRNWFCjGMXTqoh70180353 = -46600905;    double uIfTRNWFCjGMXTqoh81808522 = -930148543;    double uIfTRNWFCjGMXTqoh12273938 = -641097507;    double uIfTRNWFCjGMXTqoh46663146 = -351622133;    double uIfTRNWFCjGMXTqoh29143799 = -556881526;    double uIfTRNWFCjGMXTqoh44567266 = -961584284;    double uIfTRNWFCjGMXTqoh75994263 = -216340097;    double uIfTRNWFCjGMXTqoh4703010 = -69132767;    double uIfTRNWFCjGMXTqoh27804293 = -652729406;    double uIfTRNWFCjGMXTqoh17961985 = -193756400;    double uIfTRNWFCjGMXTqoh2946794 = -922904846;    double uIfTRNWFCjGMXTqoh42188334 = -140527083;    double uIfTRNWFCjGMXTqoh94185793 = -456906987;    double uIfTRNWFCjGMXTqoh15851718 = -251220859;    double uIfTRNWFCjGMXTqoh48019574 = -136500214;    double uIfTRNWFCjGMXTqoh72396109 = -58135883;    double uIfTRNWFCjGMXTqoh87538563 = -963334751;    double uIfTRNWFCjGMXTqoh44880940 = -515062350;    double uIfTRNWFCjGMXTqoh49689143 = -736523270;    double uIfTRNWFCjGMXTqoh84302914 = -229822326;    double uIfTRNWFCjGMXTqoh43267443 = -301453193;    double uIfTRNWFCjGMXTqoh5842731 = -901930547;    double uIfTRNWFCjGMXTqoh83476320 = -522170825;    double uIfTRNWFCjGMXTqoh26597085 = -22326780;    double uIfTRNWFCjGMXTqoh66618524 = 73020360;    double uIfTRNWFCjGMXTqoh38527356 = -678639794;    double uIfTRNWFCjGMXTqoh55883747 = -788193575;    double uIfTRNWFCjGMXTqoh27610920 = -151376390;    double uIfTRNWFCjGMXTqoh79434088 = 43263144;    double uIfTRNWFCjGMXTqoh60359368 = -258084703;     uIfTRNWFCjGMXTqoh10893207 = uIfTRNWFCjGMXTqoh78545654;     uIfTRNWFCjGMXTqoh78545654 = uIfTRNWFCjGMXTqoh68887150;     uIfTRNWFCjGMXTqoh68887150 = uIfTRNWFCjGMXTqoh98477631;     uIfTRNWFCjGMXTqoh98477631 = uIfTRNWFCjGMXTqoh59418561;     uIfTRNWFCjGMXTqoh59418561 = uIfTRNWFCjGMXTqoh12700123;     uIfTRNWFCjGMXTqoh12700123 = uIfTRNWFCjGMXTqoh29678244;     uIfTRNWFCjGMXTqoh29678244 = uIfTRNWFCjGMXTqoh50363006;     uIfTRNWFCjGMXTqoh50363006 = uIfTRNWFCjGMXTqoh84810687;     uIfTRNWFCjGMXTqoh84810687 = uIfTRNWFCjGMXTqoh69745763;     uIfTRNWFCjGMXTqoh69745763 = uIfTRNWFCjGMXTqoh12771175;     uIfTRNWFCjGMXTqoh12771175 = uIfTRNWFCjGMXTqoh35028427;     uIfTRNWFCjGMXTqoh35028427 = uIfTRNWFCjGMXTqoh78258990;     uIfTRNWFCjGMXTqoh78258990 = uIfTRNWFCjGMXTqoh60473059;     uIfTRNWFCjGMXTqoh60473059 = uIfTRNWFCjGMXTqoh7800213;     uIfTRNWFCjGMXTqoh7800213 = uIfTRNWFCjGMXTqoh52341136;     uIfTRNWFCjGMXTqoh52341136 = uIfTRNWFCjGMXTqoh46524846;     uIfTRNWFCjGMXTqoh46524846 = uIfTRNWFCjGMXTqoh32110140;     uIfTRNWFCjGMXTqoh32110140 = uIfTRNWFCjGMXTqoh10114737;     uIfTRNWFCjGMXTqoh10114737 = uIfTRNWFCjGMXTqoh88267842;     uIfTRNWFCjGMXTqoh88267842 = uIfTRNWFCjGMXTqoh69126970;     uIfTRNWFCjGMXTqoh69126970 = uIfTRNWFCjGMXTqoh34185538;     uIfTRNWFCjGMXTqoh34185538 = uIfTRNWFCjGMXTqoh54783615;     uIfTRNWFCjGMXTqoh54783615 = uIfTRNWFCjGMXTqoh92569098;     uIfTRNWFCjGMXTqoh92569098 = uIfTRNWFCjGMXTqoh17237628;     uIfTRNWFCjGMXTqoh17237628 = uIfTRNWFCjGMXTqoh66179443;     uIfTRNWFCjGMXTqoh66179443 = uIfTRNWFCjGMXTqoh15867056;     uIfTRNWFCjGMXTqoh15867056 = uIfTRNWFCjGMXTqoh9081557;     uIfTRNWFCjGMXTqoh9081557 = uIfTRNWFCjGMXTqoh81687049;     uIfTRNWFCjGMXTqoh81687049 = uIfTRNWFCjGMXTqoh54230332;     uIfTRNWFCjGMXTqoh54230332 = uIfTRNWFCjGMXTqoh99099246;     uIfTRNWFCjGMXTqoh99099246 = uIfTRNWFCjGMXTqoh8365738;     uIfTRNWFCjGMXTqoh8365738 = uIfTRNWFCjGMXTqoh3199629;     uIfTRNWFCjGMXTqoh3199629 = uIfTRNWFCjGMXTqoh66021688;     uIfTRNWFCjGMXTqoh66021688 = uIfTRNWFCjGMXTqoh61010582;     uIfTRNWFCjGMXTqoh61010582 = uIfTRNWFCjGMXTqoh67029482;     uIfTRNWFCjGMXTqoh67029482 = uIfTRNWFCjGMXTqoh82160782;     uIfTRNWFCjGMXTqoh82160782 = uIfTRNWFCjGMXTqoh64716323;     uIfTRNWFCjGMXTqoh64716323 = uIfTRNWFCjGMXTqoh19836202;     uIfTRNWFCjGMXTqoh19836202 = uIfTRNWFCjGMXTqoh63451591;     uIfTRNWFCjGMXTqoh63451591 = uIfTRNWFCjGMXTqoh59124044;     uIfTRNWFCjGMXTqoh59124044 = uIfTRNWFCjGMXTqoh24559704;     uIfTRNWFCjGMXTqoh24559704 = uIfTRNWFCjGMXTqoh58191275;     uIfTRNWFCjGMXTqoh58191275 = uIfTRNWFCjGMXTqoh50080605;     uIfTRNWFCjGMXTqoh50080605 = uIfTRNWFCjGMXTqoh64764805;     uIfTRNWFCjGMXTqoh64764805 = uIfTRNWFCjGMXTqoh99275643;     uIfTRNWFCjGMXTqoh99275643 = uIfTRNWFCjGMXTqoh63232650;     uIfTRNWFCjGMXTqoh63232650 = uIfTRNWFCjGMXTqoh73678721;     uIfTRNWFCjGMXTqoh73678721 = uIfTRNWFCjGMXTqoh14895763;     uIfTRNWFCjGMXTqoh14895763 = uIfTRNWFCjGMXTqoh65835332;     uIfTRNWFCjGMXTqoh65835332 = uIfTRNWFCjGMXTqoh6210758;     uIfTRNWFCjGMXTqoh6210758 = uIfTRNWFCjGMXTqoh26703137;     uIfTRNWFCjGMXTqoh26703137 = uIfTRNWFCjGMXTqoh20827174;     uIfTRNWFCjGMXTqoh20827174 = uIfTRNWFCjGMXTqoh58318689;     uIfTRNWFCjGMXTqoh58318689 = uIfTRNWFCjGMXTqoh16332546;     uIfTRNWFCjGMXTqoh16332546 = uIfTRNWFCjGMXTqoh76707668;     uIfTRNWFCjGMXTqoh76707668 = uIfTRNWFCjGMXTqoh23762039;     uIfTRNWFCjGMXTqoh23762039 = uIfTRNWFCjGMXTqoh76318051;     uIfTRNWFCjGMXTqoh76318051 = uIfTRNWFCjGMXTqoh81240003;     uIfTRNWFCjGMXTqoh81240003 = uIfTRNWFCjGMXTqoh93239117;     uIfTRNWFCjGMXTqoh93239117 = uIfTRNWFCjGMXTqoh96833066;     uIfTRNWFCjGMXTqoh96833066 = uIfTRNWFCjGMXTqoh20596688;     uIfTRNWFCjGMXTqoh20596688 = uIfTRNWFCjGMXTqoh68675956;     uIfTRNWFCjGMXTqoh68675956 = uIfTRNWFCjGMXTqoh30580356;     uIfTRNWFCjGMXTqoh30580356 = uIfTRNWFCjGMXTqoh70646517;     uIfTRNWFCjGMXTqoh70646517 = uIfTRNWFCjGMXTqoh4405438;     uIfTRNWFCjGMXTqoh4405438 = uIfTRNWFCjGMXTqoh31828798;     uIfTRNWFCjGMXTqoh31828798 = uIfTRNWFCjGMXTqoh12237302;     uIfTRNWFCjGMXTqoh12237302 = uIfTRNWFCjGMXTqoh99462476;     uIfTRNWFCjGMXTqoh99462476 = uIfTRNWFCjGMXTqoh40770730;     uIfTRNWFCjGMXTqoh40770730 = uIfTRNWFCjGMXTqoh70180353;     uIfTRNWFCjGMXTqoh70180353 = uIfTRNWFCjGMXTqoh81808522;     uIfTRNWFCjGMXTqoh81808522 = uIfTRNWFCjGMXTqoh12273938;     uIfTRNWFCjGMXTqoh12273938 = uIfTRNWFCjGMXTqoh46663146;     uIfTRNWFCjGMXTqoh46663146 = uIfTRNWFCjGMXTqoh29143799;     uIfTRNWFCjGMXTqoh29143799 = uIfTRNWFCjGMXTqoh44567266;     uIfTRNWFCjGMXTqoh44567266 = uIfTRNWFCjGMXTqoh75994263;     uIfTRNWFCjGMXTqoh75994263 = uIfTRNWFCjGMXTqoh4703010;     uIfTRNWFCjGMXTqoh4703010 = uIfTRNWFCjGMXTqoh27804293;     uIfTRNWFCjGMXTqoh27804293 = uIfTRNWFCjGMXTqoh17961985;     uIfTRNWFCjGMXTqoh17961985 = uIfTRNWFCjGMXTqoh2946794;     uIfTRNWFCjGMXTqoh2946794 = uIfTRNWFCjGMXTqoh42188334;     uIfTRNWFCjGMXTqoh42188334 = uIfTRNWFCjGMXTqoh94185793;     uIfTRNWFCjGMXTqoh94185793 = uIfTRNWFCjGMXTqoh15851718;     uIfTRNWFCjGMXTqoh15851718 = uIfTRNWFCjGMXTqoh48019574;     uIfTRNWFCjGMXTqoh48019574 = uIfTRNWFCjGMXTqoh72396109;     uIfTRNWFCjGMXTqoh72396109 = uIfTRNWFCjGMXTqoh87538563;     uIfTRNWFCjGMXTqoh87538563 = uIfTRNWFCjGMXTqoh44880940;     uIfTRNWFCjGMXTqoh44880940 = uIfTRNWFCjGMXTqoh49689143;     uIfTRNWFCjGMXTqoh49689143 = uIfTRNWFCjGMXTqoh84302914;     uIfTRNWFCjGMXTqoh84302914 = uIfTRNWFCjGMXTqoh43267443;     uIfTRNWFCjGMXTqoh43267443 = uIfTRNWFCjGMXTqoh5842731;     uIfTRNWFCjGMXTqoh5842731 = uIfTRNWFCjGMXTqoh83476320;     uIfTRNWFCjGMXTqoh83476320 = uIfTRNWFCjGMXTqoh26597085;     uIfTRNWFCjGMXTqoh26597085 = uIfTRNWFCjGMXTqoh66618524;     uIfTRNWFCjGMXTqoh66618524 = uIfTRNWFCjGMXTqoh38527356;     uIfTRNWFCjGMXTqoh38527356 = uIfTRNWFCjGMXTqoh55883747;     uIfTRNWFCjGMXTqoh55883747 = uIfTRNWFCjGMXTqoh27610920;     uIfTRNWFCjGMXTqoh27610920 = uIfTRNWFCjGMXTqoh79434088;     uIfTRNWFCjGMXTqoh79434088 = uIfTRNWFCjGMXTqoh60359368;     uIfTRNWFCjGMXTqoh60359368 = uIfTRNWFCjGMXTqoh10893207;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MSbUKYYNABhUTRQj508583() {     double woHnLCmGHxOMFegWn17010314 = -369986592;    double woHnLCmGHxOMFegWn21918933 = -148650950;    double woHnLCmGHxOMFegWn84929333 = -334454221;    double woHnLCmGHxOMFegWn25552412 = -375373275;    double woHnLCmGHxOMFegWn28314050 = -468445110;    double woHnLCmGHxOMFegWn4117325 = -996071644;    double woHnLCmGHxOMFegWn90167210 = -980243789;    double woHnLCmGHxOMFegWn122898 = -471069607;    double woHnLCmGHxOMFegWn49453183 = -447108868;    double woHnLCmGHxOMFegWn10975423 = -539557407;    double woHnLCmGHxOMFegWn72881464 = -869590125;    double woHnLCmGHxOMFegWn66851470 = -134756881;    double woHnLCmGHxOMFegWn64468109 = -906239741;    double woHnLCmGHxOMFegWn43616735 = -836180358;    double woHnLCmGHxOMFegWn3542640 = -697246606;    double woHnLCmGHxOMFegWn93846530 = -371907479;    double woHnLCmGHxOMFegWn40360849 = -226025280;    double woHnLCmGHxOMFegWn29163293 = -84456016;    double woHnLCmGHxOMFegWn71944578 = -497600504;    double woHnLCmGHxOMFegWn68818007 = -276148434;    double woHnLCmGHxOMFegWn57680163 = -669951847;    double woHnLCmGHxOMFegWn20706562 = 48496726;    double woHnLCmGHxOMFegWn90374406 = -58487747;    double woHnLCmGHxOMFegWn76561472 = 44450197;    double woHnLCmGHxOMFegWn60937278 = -300659878;    double woHnLCmGHxOMFegWn90901118 = -170941476;    double woHnLCmGHxOMFegWn14326834 = -919007977;    double woHnLCmGHxOMFegWn22188682 = -123483569;    double woHnLCmGHxOMFegWn47819305 = -341891865;    double woHnLCmGHxOMFegWn22845969 = -920689999;    double woHnLCmGHxOMFegWn91341795 = 62197770;    double woHnLCmGHxOMFegWn5976359 = -177230042;    double woHnLCmGHxOMFegWn12910578 = -614655235;    double woHnLCmGHxOMFegWn60852253 = -9647594;    double woHnLCmGHxOMFegWn22485235 = 16933424;    double woHnLCmGHxOMFegWn35755396 = -642560210;    double woHnLCmGHxOMFegWn63422717 = -205942901;    double woHnLCmGHxOMFegWn98806535 = -550837634;    double woHnLCmGHxOMFegWn93446455 = -771759363;    double woHnLCmGHxOMFegWn50953528 = -113648497;    double woHnLCmGHxOMFegWn64793268 = -427637884;    double woHnLCmGHxOMFegWn56710161 = -23022386;    double woHnLCmGHxOMFegWn36258254 = -908038045;    double woHnLCmGHxOMFegWn37488346 = -162180432;    double woHnLCmGHxOMFegWn76717270 = -842413564;    double woHnLCmGHxOMFegWn11214839 = -718923562;    double woHnLCmGHxOMFegWn81011224 = -488156845;    double woHnLCmGHxOMFegWn81635662 = -787714224;    double woHnLCmGHxOMFegWn24571492 = -686384617;    double woHnLCmGHxOMFegWn78746984 = -955299110;    double woHnLCmGHxOMFegWn41911436 = -709012590;    double woHnLCmGHxOMFegWn37512857 = -767071730;    double woHnLCmGHxOMFegWn69075631 = -296238063;    double woHnLCmGHxOMFegWn54287569 = -427449661;    double woHnLCmGHxOMFegWn42828420 = -122114342;    double woHnLCmGHxOMFegWn96303751 = -318483318;    double woHnLCmGHxOMFegWn31544527 = 9836796;    double woHnLCmGHxOMFegWn8367862 = -278904418;    double woHnLCmGHxOMFegWn64615134 = 25286602;    double woHnLCmGHxOMFegWn37412931 = -197503635;    double woHnLCmGHxOMFegWn89790491 = 22936333;    double woHnLCmGHxOMFegWn67978528 = -756760220;    double woHnLCmGHxOMFegWn52303592 = -29177742;    double woHnLCmGHxOMFegWn26607214 = -526418869;    double woHnLCmGHxOMFegWn19633627 = -501755178;    double woHnLCmGHxOMFegWn66905105 = -592360084;    double woHnLCmGHxOMFegWn53940892 = -520101646;    double woHnLCmGHxOMFegWn3615856 = -796592147;    double woHnLCmGHxOMFegWn21131501 = -753113782;    double woHnLCmGHxOMFegWn67787244 = 45313604;    double woHnLCmGHxOMFegWn30423813 = -65964579;    double woHnLCmGHxOMFegWn41554313 = -675187646;    double woHnLCmGHxOMFegWn35716837 = -312696653;    double woHnLCmGHxOMFegWn20991050 = -283952008;    double woHnLCmGHxOMFegWn4024740 = -848510551;    double woHnLCmGHxOMFegWn970003 = -546929461;    double woHnLCmGHxOMFegWn84448308 = -43465230;    double woHnLCmGHxOMFegWn52886061 = -896307316;    double woHnLCmGHxOMFegWn99844201 = -113136240;    double woHnLCmGHxOMFegWn49722439 = -581736317;    double woHnLCmGHxOMFegWn9889895 = -682784631;    double woHnLCmGHxOMFegWn32691171 = -31293753;    double woHnLCmGHxOMFegWn97617190 = -437098953;    double woHnLCmGHxOMFegWn69072320 = -386592755;    double woHnLCmGHxOMFegWn80934532 = -111677410;    double woHnLCmGHxOMFegWn53828939 = -170730501;    double woHnLCmGHxOMFegWn36900728 = -880991979;    double woHnLCmGHxOMFegWn58623008 = -87205575;    double woHnLCmGHxOMFegWn18023833 = -887533253;    double woHnLCmGHxOMFegWn26181483 = -664583259;    double woHnLCmGHxOMFegWn4210869 = -552397007;    double woHnLCmGHxOMFegWn55054856 = -927038483;    double woHnLCmGHxOMFegWn34191402 = -476124237;    double woHnLCmGHxOMFegWn56033525 = -474255729;    double woHnLCmGHxOMFegWn61163037 = -36584830;    double woHnLCmGHxOMFegWn96814739 = -670877664;    double woHnLCmGHxOMFegWn4406569 = -993844645;    double woHnLCmGHxOMFegWn9651040 = -281619176;    double woHnLCmGHxOMFegWn17854719 = -660425255;    double woHnLCmGHxOMFegWn9812165 = -369986592;     woHnLCmGHxOMFegWn17010314 = woHnLCmGHxOMFegWn21918933;     woHnLCmGHxOMFegWn21918933 = woHnLCmGHxOMFegWn84929333;     woHnLCmGHxOMFegWn84929333 = woHnLCmGHxOMFegWn25552412;     woHnLCmGHxOMFegWn25552412 = woHnLCmGHxOMFegWn28314050;     woHnLCmGHxOMFegWn28314050 = woHnLCmGHxOMFegWn4117325;     woHnLCmGHxOMFegWn4117325 = woHnLCmGHxOMFegWn90167210;     woHnLCmGHxOMFegWn90167210 = woHnLCmGHxOMFegWn122898;     woHnLCmGHxOMFegWn122898 = woHnLCmGHxOMFegWn49453183;     woHnLCmGHxOMFegWn49453183 = woHnLCmGHxOMFegWn10975423;     woHnLCmGHxOMFegWn10975423 = woHnLCmGHxOMFegWn72881464;     woHnLCmGHxOMFegWn72881464 = woHnLCmGHxOMFegWn66851470;     woHnLCmGHxOMFegWn66851470 = woHnLCmGHxOMFegWn64468109;     woHnLCmGHxOMFegWn64468109 = woHnLCmGHxOMFegWn43616735;     woHnLCmGHxOMFegWn43616735 = woHnLCmGHxOMFegWn3542640;     woHnLCmGHxOMFegWn3542640 = woHnLCmGHxOMFegWn93846530;     woHnLCmGHxOMFegWn93846530 = woHnLCmGHxOMFegWn40360849;     woHnLCmGHxOMFegWn40360849 = woHnLCmGHxOMFegWn29163293;     woHnLCmGHxOMFegWn29163293 = woHnLCmGHxOMFegWn71944578;     woHnLCmGHxOMFegWn71944578 = woHnLCmGHxOMFegWn68818007;     woHnLCmGHxOMFegWn68818007 = woHnLCmGHxOMFegWn57680163;     woHnLCmGHxOMFegWn57680163 = woHnLCmGHxOMFegWn20706562;     woHnLCmGHxOMFegWn20706562 = woHnLCmGHxOMFegWn90374406;     woHnLCmGHxOMFegWn90374406 = woHnLCmGHxOMFegWn76561472;     woHnLCmGHxOMFegWn76561472 = woHnLCmGHxOMFegWn60937278;     woHnLCmGHxOMFegWn60937278 = woHnLCmGHxOMFegWn90901118;     woHnLCmGHxOMFegWn90901118 = woHnLCmGHxOMFegWn14326834;     woHnLCmGHxOMFegWn14326834 = woHnLCmGHxOMFegWn22188682;     woHnLCmGHxOMFegWn22188682 = woHnLCmGHxOMFegWn47819305;     woHnLCmGHxOMFegWn47819305 = woHnLCmGHxOMFegWn22845969;     woHnLCmGHxOMFegWn22845969 = woHnLCmGHxOMFegWn91341795;     woHnLCmGHxOMFegWn91341795 = woHnLCmGHxOMFegWn5976359;     woHnLCmGHxOMFegWn5976359 = woHnLCmGHxOMFegWn12910578;     woHnLCmGHxOMFegWn12910578 = woHnLCmGHxOMFegWn60852253;     woHnLCmGHxOMFegWn60852253 = woHnLCmGHxOMFegWn22485235;     woHnLCmGHxOMFegWn22485235 = woHnLCmGHxOMFegWn35755396;     woHnLCmGHxOMFegWn35755396 = woHnLCmGHxOMFegWn63422717;     woHnLCmGHxOMFegWn63422717 = woHnLCmGHxOMFegWn98806535;     woHnLCmGHxOMFegWn98806535 = woHnLCmGHxOMFegWn93446455;     woHnLCmGHxOMFegWn93446455 = woHnLCmGHxOMFegWn50953528;     woHnLCmGHxOMFegWn50953528 = woHnLCmGHxOMFegWn64793268;     woHnLCmGHxOMFegWn64793268 = woHnLCmGHxOMFegWn56710161;     woHnLCmGHxOMFegWn56710161 = woHnLCmGHxOMFegWn36258254;     woHnLCmGHxOMFegWn36258254 = woHnLCmGHxOMFegWn37488346;     woHnLCmGHxOMFegWn37488346 = woHnLCmGHxOMFegWn76717270;     woHnLCmGHxOMFegWn76717270 = woHnLCmGHxOMFegWn11214839;     woHnLCmGHxOMFegWn11214839 = woHnLCmGHxOMFegWn81011224;     woHnLCmGHxOMFegWn81011224 = woHnLCmGHxOMFegWn81635662;     woHnLCmGHxOMFegWn81635662 = woHnLCmGHxOMFegWn24571492;     woHnLCmGHxOMFegWn24571492 = woHnLCmGHxOMFegWn78746984;     woHnLCmGHxOMFegWn78746984 = woHnLCmGHxOMFegWn41911436;     woHnLCmGHxOMFegWn41911436 = woHnLCmGHxOMFegWn37512857;     woHnLCmGHxOMFegWn37512857 = woHnLCmGHxOMFegWn69075631;     woHnLCmGHxOMFegWn69075631 = woHnLCmGHxOMFegWn54287569;     woHnLCmGHxOMFegWn54287569 = woHnLCmGHxOMFegWn42828420;     woHnLCmGHxOMFegWn42828420 = woHnLCmGHxOMFegWn96303751;     woHnLCmGHxOMFegWn96303751 = woHnLCmGHxOMFegWn31544527;     woHnLCmGHxOMFegWn31544527 = woHnLCmGHxOMFegWn8367862;     woHnLCmGHxOMFegWn8367862 = woHnLCmGHxOMFegWn64615134;     woHnLCmGHxOMFegWn64615134 = woHnLCmGHxOMFegWn37412931;     woHnLCmGHxOMFegWn37412931 = woHnLCmGHxOMFegWn89790491;     woHnLCmGHxOMFegWn89790491 = woHnLCmGHxOMFegWn67978528;     woHnLCmGHxOMFegWn67978528 = woHnLCmGHxOMFegWn52303592;     woHnLCmGHxOMFegWn52303592 = woHnLCmGHxOMFegWn26607214;     woHnLCmGHxOMFegWn26607214 = woHnLCmGHxOMFegWn19633627;     woHnLCmGHxOMFegWn19633627 = woHnLCmGHxOMFegWn66905105;     woHnLCmGHxOMFegWn66905105 = woHnLCmGHxOMFegWn53940892;     woHnLCmGHxOMFegWn53940892 = woHnLCmGHxOMFegWn3615856;     woHnLCmGHxOMFegWn3615856 = woHnLCmGHxOMFegWn21131501;     woHnLCmGHxOMFegWn21131501 = woHnLCmGHxOMFegWn67787244;     woHnLCmGHxOMFegWn67787244 = woHnLCmGHxOMFegWn30423813;     woHnLCmGHxOMFegWn30423813 = woHnLCmGHxOMFegWn41554313;     woHnLCmGHxOMFegWn41554313 = woHnLCmGHxOMFegWn35716837;     woHnLCmGHxOMFegWn35716837 = woHnLCmGHxOMFegWn20991050;     woHnLCmGHxOMFegWn20991050 = woHnLCmGHxOMFegWn4024740;     woHnLCmGHxOMFegWn4024740 = woHnLCmGHxOMFegWn970003;     woHnLCmGHxOMFegWn970003 = woHnLCmGHxOMFegWn84448308;     woHnLCmGHxOMFegWn84448308 = woHnLCmGHxOMFegWn52886061;     woHnLCmGHxOMFegWn52886061 = woHnLCmGHxOMFegWn99844201;     woHnLCmGHxOMFegWn99844201 = woHnLCmGHxOMFegWn49722439;     woHnLCmGHxOMFegWn49722439 = woHnLCmGHxOMFegWn9889895;     woHnLCmGHxOMFegWn9889895 = woHnLCmGHxOMFegWn32691171;     woHnLCmGHxOMFegWn32691171 = woHnLCmGHxOMFegWn97617190;     woHnLCmGHxOMFegWn97617190 = woHnLCmGHxOMFegWn69072320;     woHnLCmGHxOMFegWn69072320 = woHnLCmGHxOMFegWn80934532;     woHnLCmGHxOMFegWn80934532 = woHnLCmGHxOMFegWn53828939;     woHnLCmGHxOMFegWn53828939 = woHnLCmGHxOMFegWn36900728;     woHnLCmGHxOMFegWn36900728 = woHnLCmGHxOMFegWn58623008;     woHnLCmGHxOMFegWn58623008 = woHnLCmGHxOMFegWn18023833;     woHnLCmGHxOMFegWn18023833 = woHnLCmGHxOMFegWn26181483;     woHnLCmGHxOMFegWn26181483 = woHnLCmGHxOMFegWn4210869;     woHnLCmGHxOMFegWn4210869 = woHnLCmGHxOMFegWn55054856;     woHnLCmGHxOMFegWn55054856 = woHnLCmGHxOMFegWn34191402;     woHnLCmGHxOMFegWn34191402 = woHnLCmGHxOMFegWn56033525;     woHnLCmGHxOMFegWn56033525 = woHnLCmGHxOMFegWn61163037;     woHnLCmGHxOMFegWn61163037 = woHnLCmGHxOMFegWn96814739;     woHnLCmGHxOMFegWn96814739 = woHnLCmGHxOMFegWn4406569;     woHnLCmGHxOMFegWn4406569 = woHnLCmGHxOMFegWn9651040;     woHnLCmGHxOMFegWn9651040 = woHnLCmGHxOMFegWn17854719;     woHnLCmGHxOMFegWn17854719 = woHnLCmGHxOMFegWn9812165;     woHnLCmGHxOMFegWn9812165 = woHnLCmGHxOMFegWn17010314;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GeMHSkMYfyJFPnAq67800182() {     double MwvbDBqRfPKbocBIG52469282 = -836263491;    double MwvbDBqRfPKbocBIG35191012 = -29202557;    double MwvbDBqRfPKbocBIG22315811 = -121296;    double MwvbDBqRfPKbocBIG97775766 = -196524346;    double MwvbDBqRfPKbocBIG84599318 = -574043956;    double MwvbDBqRfPKbocBIG40340197 = -158419539;    double MwvbDBqRfPKbocBIG824617 = -824969523;    double MwvbDBqRfPKbocBIG62254458 = -115303747;    double MwvbDBqRfPKbocBIG50970788 = -935440275;    double MwvbDBqRfPKbocBIG25878135 = -204244405;    double MwvbDBqRfPKbocBIG11063200 = -671597855;    double MwvbDBqRfPKbocBIG67419618 = 1122348;    double MwvbDBqRfPKbocBIG2702755 = -358252246;    double MwvbDBqRfPKbocBIG96757598 = -812543767;    double MwvbDBqRfPKbocBIG61737155 = -100671621;    double MwvbDBqRfPKbocBIG77387831 = -7719086;    double MwvbDBqRfPKbocBIG96196784 = -752999832;    double MwvbDBqRfPKbocBIG28923485 = -157884084;    double MwvbDBqRfPKbocBIG72494908 = -814163926;    double MwvbDBqRfPKbocBIG62655225 = -702709500;    double MwvbDBqRfPKbocBIG70767026 = -327226536;    double MwvbDBqRfPKbocBIG24128087 = -38139568;    double MwvbDBqRfPKbocBIG60899786 = -796119931;    double MwvbDBqRfPKbocBIG70354417 = -466374407;    double MwvbDBqRfPKbocBIG68955229 = -324645076;    double MwvbDBqRfPKbocBIG1599291 = -959518283;    double MwvbDBqRfPKbocBIG74747407 = -725292118;    double MwvbDBqRfPKbocBIG62917106 = -293255744;    double MwvbDBqRfPKbocBIG94487519 = -915986667;    double MwvbDBqRfPKbocBIG28379380 = -955197170;    double MwvbDBqRfPKbocBIG9925028 = -489782717;    double MwvbDBqRfPKbocBIG43799377 = 49798544;    double MwvbDBqRfPKbocBIG25128589 = -762251213;    double MwvbDBqRfPKbocBIG90600679 = -74341408;    double MwvbDBqRfPKbocBIG88293688 = 65833173;    double MwvbDBqRfPKbocBIG87316239 = -857125516;    double MwvbDBqRfPKbocBIG12834534 = -70638941;    double MwvbDBqRfPKbocBIG50021939 = -533493215;    double MwvbDBqRfPKbocBIG23837018 = -422635640;    double MwvbDBqRfPKbocBIG15225061 = -678911043;    double MwvbDBqRfPKbocBIG98244875 = -196836798;    double MwvbDBqRfPKbocBIG46075288 = -717567677;    double MwvbDBqRfPKbocBIG25641632 = -461218831;    double MwvbDBqRfPKbocBIG77619684 = -766512842;    double MwvbDBqRfPKbocBIG76733729 = -668924725;    double MwvbDBqRfPKbocBIG57176167 = -757847194;    double MwvbDBqRfPKbocBIG356388 = -972143206;    double MwvbDBqRfPKbocBIG99361974 = -954240818;    double MwvbDBqRfPKbocBIG47464158 = -350121497;    double MwvbDBqRfPKbocBIG3276574 = -346325926;    double MwvbDBqRfPKbocBIG22682160 = -583289147;    double MwvbDBqRfPKbocBIG20998329 = -980724076;    double MwvbDBqRfPKbocBIG37696780 = -52283617;    double MwvbDBqRfPKbocBIG70279416 = -618958160;    double MwvbDBqRfPKbocBIG97033156 = -468604488;    double MwvbDBqRfPKbocBIG28341196 = -698123924;    double MwvbDBqRfPKbocBIG74291225 = -233082626;    double MwvbDBqRfPKbocBIG51961393 = -533746889;    double MwvbDBqRfPKbocBIG28820538 = -871879271;    double MwvbDBqRfPKbocBIG83000028 = -614525674;    double MwvbDBqRfPKbocBIG65592789 = -433127422;    double MwvbDBqRfPKbocBIG37907510 = -431713780;    double MwvbDBqRfPKbocBIG67766938 = -199317080;    double MwvbDBqRfPKbocBIG22591409 = -980243106;    double MwvbDBqRfPKbocBIG15953108 = -714461689;    double MwvbDBqRfPKbocBIG67263822 = -621396399;    double MwvbDBqRfPKbocBIG42291030 = -236626440;    double MwvbDBqRfPKbocBIG12102075 = -183910839;    double MwvbDBqRfPKbocBIG8463911 = -778376941;    double MwvbDBqRfPKbocBIG74420916 = -243546105;    double MwvbDBqRfPKbocBIG64553297 = -937080146;    double MwvbDBqRfPKbocBIG46174845 = -119506618;    double MwvbDBqRfPKbocBIG5086468 = -735248445;    double MwvbDBqRfPKbocBIG57269847 = -35252883;    double MwvbDBqRfPKbocBIG64410350 = -405872702;    double MwvbDBqRfPKbocBIG24691739 = -609658859;    double MwvbDBqRfPKbocBIG98486454 = -576920738;    double MwvbDBqRfPKbocBIG83280102 = 70392910;    double MwvbDBqRfPKbocBIG93620688 = -797449683;    double MwvbDBqRfPKbocBIG11779062 = -566797883;    double MwvbDBqRfPKbocBIG1242903 = -987375078;    double MwvbDBqRfPKbocBIG75385433 = -771051301;    double MwvbDBqRfPKbocBIG15452949 = -943134247;    double MwvbDBqRfPKbocBIG91210946 = -469660742;    double MwvbDBqRfPKbocBIG5697220 = -271908023;    double MwvbDBqRfPKbocBIG88926698 = -509058642;    double MwvbDBqRfPKbocBIG6102598 = -897917839;    double MwvbDBqRfPKbocBIG54849172 = -43293053;    double MwvbDBqRfPKbocBIG93567523 = -605736921;    double MwvbDBqRfPKbocBIG59952493 = -236042904;    double MwvbDBqRfPKbocBIG13025014 = -524042890;    double MwvbDBqRfPKbocBIG60873141 = -536892052;    double MwvbDBqRfPKbocBIG21201402 = -661613945;    double MwvbDBqRfPKbocBIG40836989 = -808109967;    double MwvbDBqRfPKbocBIG49632272 = -145783622;    double MwvbDBqRfPKbocBIG60337365 = -765123019;    double MwvbDBqRfPKbocBIG78308349 = -418250598;    double MwvbDBqRfPKbocBIG3050224 = -480975725;    double MwvbDBqRfPKbocBIG61666576 = 47948846;    double MwvbDBqRfPKbocBIG9469907 = -836263491;     MwvbDBqRfPKbocBIG52469282 = MwvbDBqRfPKbocBIG35191012;     MwvbDBqRfPKbocBIG35191012 = MwvbDBqRfPKbocBIG22315811;     MwvbDBqRfPKbocBIG22315811 = MwvbDBqRfPKbocBIG97775766;     MwvbDBqRfPKbocBIG97775766 = MwvbDBqRfPKbocBIG84599318;     MwvbDBqRfPKbocBIG84599318 = MwvbDBqRfPKbocBIG40340197;     MwvbDBqRfPKbocBIG40340197 = MwvbDBqRfPKbocBIG824617;     MwvbDBqRfPKbocBIG824617 = MwvbDBqRfPKbocBIG62254458;     MwvbDBqRfPKbocBIG62254458 = MwvbDBqRfPKbocBIG50970788;     MwvbDBqRfPKbocBIG50970788 = MwvbDBqRfPKbocBIG25878135;     MwvbDBqRfPKbocBIG25878135 = MwvbDBqRfPKbocBIG11063200;     MwvbDBqRfPKbocBIG11063200 = MwvbDBqRfPKbocBIG67419618;     MwvbDBqRfPKbocBIG67419618 = MwvbDBqRfPKbocBIG2702755;     MwvbDBqRfPKbocBIG2702755 = MwvbDBqRfPKbocBIG96757598;     MwvbDBqRfPKbocBIG96757598 = MwvbDBqRfPKbocBIG61737155;     MwvbDBqRfPKbocBIG61737155 = MwvbDBqRfPKbocBIG77387831;     MwvbDBqRfPKbocBIG77387831 = MwvbDBqRfPKbocBIG96196784;     MwvbDBqRfPKbocBIG96196784 = MwvbDBqRfPKbocBIG28923485;     MwvbDBqRfPKbocBIG28923485 = MwvbDBqRfPKbocBIG72494908;     MwvbDBqRfPKbocBIG72494908 = MwvbDBqRfPKbocBIG62655225;     MwvbDBqRfPKbocBIG62655225 = MwvbDBqRfPKbocBIG70767026;     MwvbDBqRfPKbocBIG70767026 = MwvbDBqRfPKbocBIG24128087;     MwvbDBqRfPKbocBIG24128087 = MwvbDBqRfPKbocBIG60899786;     MwvbDBqRfPKbocBIG60899786 = MwvbDBqRfPKbocBIG70354417;     MwvbDBqRfPKbocBIG70354417 = MwvbDBqRfPKbocBIG68955229;     MwvbDBqRfPKbocBIG68955229 = MwvbDBqRfPKbocBIG1599291;     MwvbDBqRfPKbocBIG1599291 = MwvbDBqRfPKbocBIG74747407;     MwvbDBqRfPKbocBIG74747407 = MwvbDBqRfPKbocBIG62917106;     MwvbDBqRfPKbocBIG62917106 = MwvbDBqRfPKbocBIG94487519;     MwvbDBqRfPKbocBIG94487519 = MwvbDBqRfPKbocBIG28379380;     MwvbDBqRfPKbocBIG28379380 = MwvbDBqRfPKbocBIG9925028;     MwvbDBqRfPKbocBIG9925028 = MwvbDBqRfPKbocBIG43799377;     MwvbDBqRfPKbocBIG43799377 = MwvbDBqRfPKbocBIG25128589;     MwvbDBqRfPKbocBIG25128589 = MwvbDBqRfPKbocBIG90600679;     MwvbDBqRfPKbocBIG90600679 = MwvbDBqRfPKbocBIG88293688;     MwvbDBqRfPKbocBIG88293688 = MwvbDBqRfPKbocBIG87316239;     MwvbDBqRfPKbocBIG87316239 = MwvbDBqRfPKbocBIG12834534;     MwvbDBqRfPKbocBIG12834534 = MwvbDBqRfPKbocBIG50021939;     MwvbDBqRfPKbocBIG50021939 = MwvbDBqRfPKbocBIG23837018;     MwvbDBqRfPKbocBIG23837018 = MwvbDBqRfPKbocBIG15225061;     MwvbDBqRfPKbocBIG15225061 = MwvbDBqRfPKbocBIG98244875;     MwvbDBqRfPKbocBIG98244875 = MwvbDBqRfPKbocBIG46075288;     MwvbDBqRfPKbocBIG46075288 = MwvbDBqRfPKbocBIG25641632;     MwvbDBqRfPKbocBIG25641632 = MwvbDBqRfPKbocBIG77619684;     MwvbDBqRfPKbocBIG77619684 = MwvbDBqRfPKbocBIG76733729;     MwvbDBqRfPKbocBIG76733729 = MwvbDBqRfPKbocBIG57176167;     MwvbDBqRfPKbocBIG57176167 = MwvbDBqRfPKbocBIG356388;     MwvbDBqRfPKbocBIG356388 = MwvbDBqRfPKbocBIG99361974;     MwvbDBqRfPKbocBIG99361974 = MwvbDBqRfPKbocBIG47464158;     MwvbDBqRfPKbocBIG47464158 = MwvbDBqRfPKbocBIG3276574;     MwvbDBqRfPKbocBIG3276574 = MwvbDBqRfPKbocBIG22682160;     MwvbDBqRfPKbocBIG22682160 = MwvbDBqRfPKbocBIG20998329;     MwvbDBqRfPKbocBIG20998329 = MwvbDBqRfPKbocBIG37696780;     MwvbDBqRfPKbocBIG37696780 = MwvbDBqRfPKbocBIG70279416;     MwvbDBqRfPKbocBIG70279416 = MwvbDBqRfPKbocBIG97033156;     MwvbDBqRfPKbocBIG97033156 = MwvbDBqRfPKbocBIG28341196;     MwvbDBqRfPKbocBIG28341196 = MwvbDBqRfPKbocBIG74291225;     MwvbDBqRfPKbocBIG74291225 = MwvbDBqRfPKbocBIG51961393;     MwvbDBqRfPKbocBIG51961393 = MwvbDBqRfPKbocBIG28820538;     MwvbDBqRfPKbocBIG28820538 = MwvbDBqRfPKbocBIG83000028;     MwvbDBqRfPKbocBIG83000028 = MwvbDBqRfPKbocBIG65592789;     MwvbDBqRfPKbocBIG65592789 = MwvbDBqRfPKbocBIG37907510;     MwvbDBqRfPKbocBIG37907510 = MwvbDBqRfPKbocBIG67766938;     MwvbDBqRfPKbocBIG67766938 = MwvbDBqRfPKbocBIG22591409;     MwvbDBqRfPKbocBIG22591409 = MwvbDBqRfPKbocBIG15953108;     MwvbDBqRfPKbocBIG15953108 = MwvbDBqRfPKbocBIG67263822;     MwvbDBqRfPKbocBIG67263822 = MwvbDBqRfPKbocBIG42291030;     MwvbDBqRfPKbocBIG42291030 = MwvbDBqRfPKbocBIG12102075;     MwvbDBqRfPKbocBIG12102075 = MwvbDBqRfPKbocBIG8463911;     MwvbDBqRfPKbocBIG8463911 = MwvbDBqRfPKbocBIG74420916;     MwvbDBqRfPKbocBIG74420916 = MwvbDBqRfPKbocBIG64553297;     MwvbDBqRfPKbocBIG64553297 = MwvbDBqRfPKbocBIG46174845;     MwvbDBqRfPKbocBIG46174845 = MwvbDBqRfPKbocBIG5086468;     MwvbDBqRfPKbocBIG5086468 = MwvbDBqRfPKbocBIG57269847;     MwvbDBqRfPKbocBIG57269847 = MwvbDBqRfPKbocBIG64410350;     MwvbDBqRfPKbocBIG64410350 = MwvbDBqRfPKbocBIG24691739;     MwvbDBqRfPKbocBIG24691739 = MwvbDBqRfPKbocBIG98486454;     MwvbDBqRfPKbocBIG98486454 = MwvbDBqRfPKbocBIG83280102;     MwvbDBqRfPKbocBIG83280102 = MwvbDBqRfPKbocBIG93620688;     MwvbDBqRfPKbocBIG93620688 = MwvbDBqRfPKbocBIG11779062;     MwvbDBqRfPKbocBIG11779062 = MwvbDBqRfPKbocBIG1242903;     MwvbDBqRfPKbocBIG1242903 = MwvbDBqRfPKbocBIG75385433;     MwvbDBqRfPKbocBIG75385433 = MwvbDBqRfPKbocBIG15452949;     MwvbDBqRfPKbocBIG15452949 = MwvbDBqRfPKbocBIG91210946;     MwvbDBqRfPKbocBIG91210946 = MwvbDBqRfPKbocBIG5697220;     MwvbDBqRfPKbocBIG5697220 = MwvbDBqRfPKbocBIG88926698;     MwvbDBqRfPKbocBIG88926698 = MwvbDBqRfPKbocBIG6102598;     MwvbDBqRfPKbocBIG6102598 = MwvbDBqRfPKbocBIG54849172;     MwvbDBqRfPKbocBIG54849172 = MwvbDBqRfPKbocBIG93567523;     MwvbDBqRfPKbocBIG93567523 = MwvbDBqRfPKbocBIG59952493;     MwvbDBqRfPKbocBIG59952493 = MwvbDBqRfPKbocBIG13025014;     MwvbDBqRfPKbocBIG13025014 = MwvbDBqRfPKbocBIG60873141;     MwvbDBqRfPKbocBIG60873141 = MwvbDBqRfPKbocBIG21201402;     MwvbDBqRfPKbocBIG21201402 = MwvbDBqRfPKbocBIG40836989;     MwvbDBqRfPKbocBIG40836989 = MwvbDBqRfPKbocBIG49632272;     MwvbDBqRfPKbocBIG49632272 = MwvbDBqRfPKbocBIG60337365;     MwvbDBqRfPKbocBIG60337365 = MwvbDBqRfPKbocBIG78308349;     MwvbDBqRfPKbocBIG78308349 = MwvbDBqRfPKbocBIG3050224;     MwvbDBqRfPKbocBIG3050224 = MwvbDBqRfPKbocBIG61666576;     MwvbDBqRfPKbocBIG61666576 = MwvbDBqRfPKbocBIG9469907;     MwvbDBqRfPKbocBIG9469907 = MwvbDBqRfPKbocBIG52469282;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void eQQtjiseDrKrLAhw82849250() {     double DyLPBRMXpnkBvgRTO58586389 = -948165380;    double DyLPBRMXpnkBvgRTO78564291 = -838802337;    double DyLPBRMXpnkBvgRTO38357994 = -150954217;    double DyLPBRMXpnkBvgRTO24850548 = -387272977;    double DyLPBRMXpnkBvgRTO53494807 = 5306173;    double DyLPBRMXpnkBvgRTO31757399 = -422915774;    double DyLPBRMXpnkBvgRTO61313582 = -963934500;    double DyLPBRMXpnkBvgRTO12014349 = -402177660;    double DyLPBRMXpnkBvgRTO15613284 = -703472941;    double DyLPBRMXpnkBvgRTO67107794 = -533219883;    double DyLPBRMXpnkBvgRTO71173489 = -549035441;    double DyLPBRMXpnkBvgRTO99242660 = -443283854;    double DyLPBRMXpnkBvgRTO88911872 = -798143335;    double DyLPBRMXpnkBvgRTO79901275 = -244782335;    double DyLPBRMXpnkBvgRTO57479582 = -791133787;    double DyLPBRMXpnkBvgRTO18893225 = 19281370;    double DyLPBRMXpnkBvgRTO90032787 = -269544945;    double DyLPBRMXpnkBvgRTO25976638 = -923406727;    double DyLPBRMXpnkBvgRTO34324750 = -119933022;    double DyLPBRMXpnkBvgRTO43205390 = 40387581;    double DyLPBRMXpnkBvgRTO59320219 = -182713991;    double DyLPBRMXpnkBvgRTO10649111 = 53930715;    double DyLPBRMXpnkBvgRTO96490578 = -542637418;    double DyLPBRMXpnkBvgRTO54346791 = -688679395;    double DyLPBRMXpnkBvgRTO12654880 = -697841109;    double DyLPBRMXpnkBvgRTO26320966 = -838173606;    double DyLPBRMXpnkBvgRTO73207185 = -625954321;    double DyLPBRMXpnkBvgRTO76024232 = -359184697;    double DyLPBRMXpnkBvgRTO60619775 = -38369372;    double DyLPBRMXpnkBvgRTO96995016 = -772668037;    double DyLPBRMXpnkBvgRTO2167577 = -403103655;    double DyLPBRMXpnkBvgRTO41409999 = -759648272;    double DyLPBRMXpnkBvgRTO34839537 = -654864583;    double DyLPBRMXpnkBvgRTO85431243 = -925883173;    double DyLPBRMXpnkBvgRTO49768340 = -572899932;    double DyLPBRMXpnkBvgRTO56042152 = 28848377;    double DyLPBRMXpnkBvgRTO94096468 = -24274811;    double DyLPBRMXpnkBvgRTO84112151 = -304999225;    double DyLPBRMXpnkBvgRTO97447271 = -416559137;    double DyLPBRMXpnkBvgRTO2726999 = -52350266;    double DyLPBRMXpnkBvgRTO3914100 = -262110692;    double DyLPBRMXpnkBvgRTO78225744 = -987709955;    double DyLPBRMXpnkBvgRTO3708611 = -542023415;    double DyLPBRMXpnkBvgRTO65027424 = -785855780;    double DyLPBRMXpnkBvgRTO88686194 = -330822879;    double DyLPBRMXpnkBvgRTO69115362 = -743063310;    double DyLPBRMXpnkBvgRTO18134962 = 9081257;    double DyLPBRMXpnkBvgRTO7318916 = -964136350;    double DyLPBRMXpnkBvgRTO57139886 = -435858485;    double DyLPBRMXpnkBvgRTO16188227 = -433336734;    double DyLPBRMXpnkBvgRTO58382838 = -425582819;    double DyLPBRMXpnkBvgRTO31808049 = -781450396;    double DyLPBRMXpnkBvgRTO85945237 = -944073205;    double DyLPBRMXpnkBvgRTO66248296 = -939428305;    double DyLPBRMXpnkBvgRTO23529031 = -69136270;    double DyLPBRMXpnkBvgRTO47937278 = -902096096;    double DyLPBRMXpnkBvgRTO82073712 = -196164920;    double DyLPBRMXpnkBvgRTO84011203 = -462274822;    double DyLPBRMXpnkBvgRTO12195669 = -689431868;    double DyLPBRMXpnkBvgRTO27173841 = -156520222;    double DyLPBRMXpnkBvgRTO58550213 = -796961454;    double DyLPBRMXpnkBvgRTO85289350 = -504749803;    double DyLPBRMXpnkBvgRTO51394574 = -263808289;    double DyLPBRMXpnkBvgRTO18618267 = -930804904;    double DyLPBRMXpnkBvgRTO64940217 = -30116228;    double DyLPBRMXpnkBvgRTO29763491 = -789387170;    double DyLPBRMXpnkBvgRTO64403124 = -788419272;    double DyLPBRMXpnkBvgRTO3480630 = -872260162;    double DyLPBRMXpnkBvgRTO30132935 = -671882404;    double DyLPBRMXpnkBvgRTO1437430 = -719982164;    double DyLPBRMXpnkBvgRTO24796756 = -956443819;    double DyLPBRMXpnkBvgRTO5920636 = -964545721;    double DyLPBRMXpnkBvgRTO28529367 = -406847591;    double DyLPBRMXpnkBvgRTO31597752 = 32417243;    double DyLPBRMXpnkBvgRTO39291291 = -697501728;    double DyLPBRMXpnkBvgRTO81094474 = -195004036;    double DyLPBRMXpnkBvgRTO6940501 = -404045871;    double DyLPBRMXpnkBvgRTO31463154 = -756781639;    double DyLPBRMXpnkBvgRTO65660597 = -257856516;    double DyLPBRMXpnkBvgRTO43539517 = -954777799;    double DyLPBRMXpnkBvgRTO8186004 = -747254863;    double DyLPBRMXpnkBvgRTO65888270 = -661817971;    double DyLPBRMXpnkBvgRTO18884346 = -923326213;    double DyLPBRMXpnkBvgRTO44431549 = -605032638;    double DyLPBRMXpnkBvgRTO38612178 = -247085219;    double DyLPBRMXpnkBvgRTO70359528 = -621653260;    double DyLPBRMXpnkBvgRTO55464762 = -815575067;    double DyLPBRMXpnkBvgRTO68591240 = -715436278;    double DyLPBRMXpnkBvgRTO61902212 = -756746903;    double DyLPBRMXpnkBvgRTO1831062 = -670803836;    double DyLPBRMXpnkBvgRTO73968439 = -774986704;    double DyLPBRMXpnkBvgRTO10085266 = -561999989;    double DyLPBRMXpnkBvgRTO71916483 = -615567357;    double DyLPBRMXpnkBvgRTO70273430 = -160038916;    double DyLPBRMXpnkBvgRTO44176785 = -255388813;    double DyLPBRMXpnkBvgRTO18624749 = -757360889;    double DyLPBRMXpnkBvgRTO26831171 = -623901667;    double DyLPBRMXpnkBvgRTO85090343 = -611218511;    double DyLPBRMXpnkBvgRTO87207 = -655739552;    double DyLPBRMXpnkBvgRTO58922704 = -948165380;     DyLPBRMXpnkBvgRTO58586389 = DyLPBRMXpnkBvgRTO78564291;     DyLPBRMXpnkBvgRTO78564291 = DyLPBRMXpnkBvgRTO38357994;     DyLPBRMXpnkBvgRTO38357994 = DyLPBRMXpnkBvgRTO24850548;     DyLPBRMXpnkBvgRTO24850548 = DyLPBRMXpnkBvgRTO53494807;     DyLPBRMXpnkBvgRTO53494807 = DyLPBRMXpnkBvgRTO31757399;     DyLPBRMXpnkBvgRTO31757399 = DyLPBRMXpnkBvgRTO61313582;     DyLPBRMXpnkBvgRTO61313582 = DyLPBRMXpnkBvgRTO12014349;     DyLPBRMXpnkBvgRTO12014349 = DyLPBRMXpnkBvgRTO15613284;     DyLPBRMXpnkBvgRTO15613284 = DyLPBRMXpnkBvgRTO67107794;     DyLPBRMXpnkBvgRTO67107794 = DyLPBRMXpnkBvgRTO71173489;     DyLPBRMXpnkBvgRTO71173489 = DyLPBRMXpnkBvgRTO99242660;     DyLPBRMXpnkBvgRTO99242660 = DyLPBRMXpnkBvgRTO88911872;     DyLPBRMXpnkBvgRTO88911872 = DyLPBRMXpnkBvgRTO79901275;     DyLPBRMXpnkBvgRTO79901275 = DyLPBRMXpnkBvgRTO57479582;     DyLPBRMXpnkBvgRTO57479582 = DyLPBRMXpnkBvgRTO18893225;     DyLPBRMXpnkBvgRTO18893225 = DyLPBRMXpnkBvgRTO90032787;     DyLPBRMXpnkBvgRTO90032787 = DyLPBRMXpnkBvgRTO25976638;     DyLPBRMXpnkBvgRTO25976638 = DyLPBRMXpnkBvgRTO34324750;     DyLPBRMXpnkBvgRTO34324750 = DyLPBRMXpnkBvgRTO43205390;     DyLPBRMXpnkBvgRTO43205390 = DyLPBRMXpnkBvgRTO59320219;     DyLPBRMXpnkBvgRTO59320219 = DyLPBRMXpnkBvgRTO10649111;     DyLPBRMXpnkBvgRTO10649111 = DyLPBRMXpnkBvgRTO96490578;     DyLPBRMXpnkBvgRTO96490578 = DyLPBRMXpnkBvgRTO54346791;     DyLPBRMXpnkBvgRTO54346791 = DyLPBRMXpnkBvgRTO12654880;     DyLPBRMXpnkBvgRTO12654880 = DyLPBRMXpnkBvgRTO26320966;     DyLPBRMXpnkBvgRTO26320966 = DyLPBRMXpnkBvgRTO73207185;     DyLPBRMXpnkBvgRTO73207185 = DyLPBRMXpnkBvgRTO76024232;     DyLPBRMXpnkBvgRTO76024232 = DyLPBRMXpnkBvgRTO60619775;     DyLPBRMXpnkBvgRTO60619775 = DyLPBRMXpnkBvgRTO96995016;     DyLPBRMXpnkBvgRTO96995016 = DyLPBRMXpnkBvgRTO2167577;     DyLPBRMXpnkBvgRTO2167577 = DyLPBRMXpnkBvgRTO41409999;     DyLPBRMXpnkBvgRTO41409999 = DyLPBRMXpnkBvgRTO34839537;     DyLPBRMXpnkBvgRTO34839537 = DyLPBRMXpnkBvgRTO85431243;     DyLPBRMXpnkBvgRTO85431243 = DyLPBRMXpnkBvgRTO49768340;     DyLPBRMXpnkBvgRTO49768340 = DyLPBRMXpnkBvgRTO56042152;     DyLPBRMXpnkBvgRTO56042152 = DyLPBRMXpnkBvgRTO94096468;     DyLPBRMXpnkBvgRTO94096468 = DyLPBRMXpnkBvgRTO84112151;     DyLPBRMXpnkBvgRTO84112151 = DyLPBRMXpnkBvgRTO97447271;     DyLPBRMXpnkBvgRTO97447271 = DyLPBRMXpnkBvgRTO2726999;     DyLPBRMXpnkBvgRTO2726999 = DyLPBRMXpnkBvgRTO3914100;     DyLPBRMXpnkBvgRTO3914100 = DyLPBRMXpnkBvgRTO78225744;     DyLPBRMXpnkBvgRTO78225744 = DyLPBRMXpnkBvgRTO3708611;     DyLPBRMXpnkBvgRTO3708611 = DyLPBRMXpnkBvgRTO65027424;     DyLPBRMXpnkBvgRTO65027424 = DyLPBRMXpnkBvgRTO88686194;     DyLPBRMXpnkBvgRTO88686194 = DyLPBRMXpnkBvgRTO69115362;     DyLPBRMXpnkBvgRTO69115362 = DyLPBRMXpnkBvgRTO18134962;     DyLPBRMXpnkBvgRTO18134962 = DyLPBRMXpnkBvgRTO7318916;     DyLPBRMXpnkBvgRTO7318916 = DyLPBRMXpnkBvgRTO57139886;     DyLPBRMXpnkBvgRTO57139886 = DyLPBRMXpnkBvgRTO16188227;     DyLPBRMXpnkBvgRTO16188227 = DyLPBRMXpnkBvgRTO58382838;     DyLPBRMXpnkBvgRTO58382838 = DyLPBRMXpnkBvgRTO31808049;     DyLPBRMXpnkBvgRTO31808049 = DyLPBRMXpnkBvgRTO85945237;     DyLPBRMXpnkBvgRTO85945237 = DyLPBRMXpnkBvgRTO66248296;     DyLPBRMXpnkBvgRTO66248296 = DyLPBRMXpnkBvgRTO23529031;     DyLPBRMXpnkBvgRTO23529031 = DyLPBRMXpnkBvgRTO47937278;     DyLPBRMXpnkBvgRTO47937278 = DyLPBRMXpnkBvgRTO82073712;     DyLPBRMXpnkBvgRTO82073712 = DyLPBRMXpnkBvgRTO84011203;     DyLPBRMXpnkBvgRTO84011203 = DyLPBRMXpnkBvgRTO12195669;     DyLPBRMXpnkBvgRTO12195669 = DyLPBRMXpnkBvgRTO27173841;     DyLPBRMXpnkBvgRTO27173841 = DyLPBRMXpnkBvgRTO58550213;     DyLPBRMXpnkBvgRTO58550213 = DyLPBRMXpnkBvgRTO85289350;     DyLPBRMXpnkBvgRTO85289350 = DyLPBRMXpnkBvgRTO51394574;     DyLPBRMXpnkBvgRTO51394574 = DyLPBRMXpnkBvgRTO18618267;     DyLPBRMXpnkBvgRTO18618267 = DyLPBRMXpnkBvgRTO64940217;     DyLPBRMXpnkBvgRTO64940217 = DyLPBRMXpnkBvgRTO29763491;     DyLPBRMXpnkBvgRTO29763491 = DyLPBRMXpnkBvgRTO64403124;     DyLPBRMXpnkBvgRTO64403124 = DyLPBRMXpnkBvgRTO3480630;     DyLPBRMXpnkBvgRTO3480630 = DyLPBRMXpnkBvgRTO30132935;     DyLPBRMXpnkBvgRTO30132935 = DyLPBRMXpnkBvgRTO1437430;     DyLPBRMXpnkBvgRTO1437430 = DyLPBRMXpnkBvgRTO24796756;     DyLPBRMXpnkBvgRTO24796756 = DyLPBRMXpnkBvgRTO5920636;     DyLPBRMXpnkBvgRTO5920636 = DyLPBRMXpnkBvgRTO28529367;     DyLPBRMXpnkBvgRTO28529367 = DyLPBRMXpnkBvgRTO31597752;     DyLPBRMXpnkBvgRTO31597752 = DyLPBRMXpnkBvgRTO39291291;     DyLPBRMXpnkBvgRTO39291291 = DyLPBRMXpnkBvgRTO81094474;     DyLPBRMXpnkBvgRTO81094474 = DyLPBRMXpnkBvgRTO6940501;     DyLPBRMXpnkBvgRTO6940501 = DyLPBRMXpnkBvgRTO31463154;     DyLPBRMXpnkBvgRTO31463154 = DyLPBRMXpnkBvgRTO65660597;     DyLPBRMXpnkBvgRTO65660597 = DyLPBRMXpnkBvgRTO43539517;     DyLPBRMXpnkBvgRTO43539517 = DyLPBRMXpnkBvgRTO8186004;     DyLPBRMXpnkBvgRTO8186004 = DyLPBRMXpnkBvgRTO65888270;     DyLPBRMXpnkBvgRTO65888270 = DyLPBRMXpnkBvgRTO18884346;     DyLPBRMXpnkBvgRTO18884346 = DyLPBRMXpnkBvgRTO44431549;     DyLPBRMXpnkBvgRTO44431549 = DyLPBRMXpnkBvgRTO38612178;     DyLPBRMXpnkBvgRTO38612178 = DyLPBRMXpnkBvgRTO70359528;     DyLPBRMXpnkBvgRTO70359528 = DyLPBRMXpnkBvgRTO55464762;     DyLPBRMXpnkBvgRTO55464762 = DyLPBRMXpnkBvgRTO68591240;     DyLPBRMXpnkBvgRTO68591240 = DyLPBRMXpnkBvgRTO61902212;     DyLPBRMXpnkBvgRTO61902212 = DyLPBRMXpnkBvgRTO1831062;     DyLPBRMXpnkBvgRTO1831062 = DyLPBRMXpnkBvgRTO73968439;     DyLPBRMXpnkBvgRTO73968439 = DyLPBRMXpnkBvgRTO10085266;     DyLPBRMXpnkBvgRTO10085266 = DyLPBRMXpnkBvgRTO71916483;     DyLPBRMXpnkBvgRTO71916483 = DyLPBRMXpnkBvgRTO70273430;     DyLPBRMXpnkBvgRTO70273430 = DyLPBRMXpnkBvgRTO44176785;     DyLPBRMXpnkBvgRTO44176785 = DyLPBRMXpnkBvgRTO18624749;     DyLPBRMXpnkBvgRTO18624749 = DyLPBRMXpnkBvgRTO26831171;     DyLPBRMXpnkBvgRTO26831171 = DyLPBRMXpnkBvgRTO85090343;     DyLPBRMXpnkBvgRTO85090343 = DyLPBRMXpnkBvgRTO87207;     DyLPBRMXpnkBvgRTO87207 = DyLPBRMXpnkBvgRTO58922704;     DyLPBRMXpnkBvgRTO58922704 = DyLPBRMXpnkBvgRTO58586389;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void QquHMpADVWJJhFcw80457277() {     double ohxFKqfqGtafDDetY61778077 = -611318659;    double ohxFKqfqGtafDDetY29186251 = -88332185;    double ohxFKqfqGtafDDetY43781103 = -770890523;    double ohxFKqfqGtafDDetY40645650 = -208729168;    double ohxFKqfqGtafDDetY33502660 = 81085566;    double ohxFKqfqGtafDDetY68688991 = -416721211;    double ohxFKqfqGtafDDetY43026024 = -328754868;    double ohxFKqfqGtafDDetY20604665 = -16440212;    double ohxFKqfqGtafDDetY26519610 = 70852984;    double ohxFKqfqGtafDDetY91142105 = -761846944;    double ohxFKqfqGtafDDetY68285789 = -822310999;    double ohxFKqfqGtafDDetY92949044 = -371725830;    double ohxFKqfqGtafDDetY30337384 = 34667145;    double ohxFKqfqGtafDDetY82690460 = -36750923;    double ohxFKqfqGtafDDetY22185303 = 85085116;    double ohxFKqfqGtafDDetY56922902 = -368038215;    double ohxFKqfqGtafDDetY95860311 = -600199489;    double ohxFKqfqGtafDDetY64116658 = -285013019;    double ohxFKqfqGtafDDetY74936110 = -285787021;    double ohxFKqfqGtafDDetY97924336 = -293441792;    double ohxFKqfqGtafDDetY39115802 = -363392838;    double ohxFKqfqGtafDDetY26633265 = -371027784;    double ohxFKqfqGtafDDetY33839450 = -869606773;    double ohxFKqfqGtafDDetY73211154 = -964456040;    double ohxFKqfqGtafDDetY60460461 = 29528020;    double ohxFKqfqGtafDDetY84081185 = -966935852;    double ohxFKqfqGtafDDetY78727256 = -734980676;    double ohxFKqfqGtafDDetY69415107 = -309359465;    double ohxFKqfqGtafDDetY74282874 = -576476418;    double ohxFKqfqGtafDDetY17250198 = -436713106;    double ohxFKqfqGtafDDetY13336085 = -487527769;    double ohxFKqfqGtafDDetY5782599 = -773194512;    double ohxFKqfqGtafDDetY60440341 = -775286441;    double ohxFKqfqGtafDDetY79912464 = -534583027;    double ohxFKqfqGtafDDetY57302002 = -369893346;    double ohxFKqfqGtafDDetY61969323 = -253116709;    double ohxFKqfqGtafDDetY62243509 = -871492182;    double ohxFKqfqGtafDDetY75976417 = -281351257;    double ohxFKqfqGtafDDetY25376315 = -86532843;    double ohxFKqfqGtafDDetY63197850 = 4471758;    double ohxFKqfqGtafDDetY10163677 = -421937114;    double ohxFKqfqGtafDDetY19424604 = -832631851;    double ohxFKqfqGtafDDetY7641998 = -142229466;    double ohxFKqfqGtafDDetY95608482 = -813872173;    double ohxFKqfqGtafDDetY96701855 = -933959921;    double ohxFKqfqGtafDDetY67843370 = -895426423;    double ohxFKqfqGtafDDetY33303812 = 73742027;    double ohxFKqfqGtafDDetY46216593 = -909545562;    double ohxFKqfqGtafDDetY32149691 = -149581875;    double ohxFKqfqGtafDDetY92959898 = -826364515;    double ohxFKqfqGtafDDetY72909239 = -997720151;    double ohxFKqfqGtafDDetY99762628 = -910856042;    double ohxFKqfqGtafDDetY1152786 = -632114532;    double ohxFKqfqGtafDDetY85110930 = -410731129;    double ohxFKqfqGtafDDetY41341475 = -160421850;    double ohxFKqfqGtafDDetY35144813 = -140290876;    double ohxFKqfqGtafDDetY95346800 = -218725412;    double ohxFKqfqGtafDDetY70569948 = -806434483;    double ohxFKqfqGtafDDetY80185188 = -138257189;    double ohxFKqfqGtafDDetY49421475 = 48021417;    double ohxFKqfqGtafDDetY89961734 = -681740536;    double ohxFKqfqGtafDDetY73610917 = 80604596;    double ohxFKqfqGtafDDetY46321791 = -439963795;    double ohxFKqfqGtafDDetY9269412 = -492433911;    double ohxFKqfqGtafDDetY77806020 = -174319176;    double ohxFKqfqGtafDDetY62503191 = 50883513;    double ohxFKqfqGtafDDetY32508703 = -596439390;    double ohxFKqfqGtafDDetY50424919 = -430749829;    double ohxFKqfqGtafDDetY25388459 = -666857578;    double ohxFKqfqGtafDDetY60215979 = -661798175;    double ohxFKqfqGtafDDetY94679392 = -496546033;    double ohxFKqfqGtafDDetY19883895 = -218848233;    double ohxFKqfqGtafDDetY38740344 = -98480177;    double ohxFKqfqGtafDDetY11738260 = -190258780;    double ohxFKqfqGtafDDetY87760659 = -871504679;    double ohxFKqfqGtafDDetY19691198 = -530760987;    double ohxFKqfqGtafDDetY18991267 = -128798318;    double ohxFKqfqGtafDDetY38230967 = 44265399;    double ohxFKqfqGtafDDetY76509299 = 69503880;    double ohxFKqfqGtafDDetY92617090 = -75045557;    double ohxFKqfqGtafDDetY50777373 = -940677880;    double ohxFKqfqGtafDDetY32510663 = -825435114;    double ohxFKqfqGtafDDetY37265416 = -59777590;    double ohxFKqfqGtafDDetY81322975 = -750111904;    double ohxFKqfqGtafDDetY44340959 = -438992956;    double ohxFKqfqGtafDDetY13573457 = -576671727;    double ohxFKqfqGtafDDetY4629813 = -41079981;    double ohxFKqfqGtafDDetY75329410 = -264555312;    double ohxFKqfqGtafDDetY38570990 = -274161177;    double ohxFKqfqGtafDDetY22157189 = -129602471;    double ohxFKqfqGtafDDetY66622522 = 65608702;    double ohxFKqfqGtafDDetY91673561 = 34942301;    double ohxFKqfqGtafDDetY95791228 = -43094068;    double ohxFKqfqGtafDDetY75954840 = -34554261;    double ohxFKqfqGtafDDetY73236115 = -313787706;    double ohxFKqfqGtafDDetY36552760 = -402541711;    double ohxFKqfqGtafDDetY73102813 = -292668056;    double ohxFKqfqGtafDDetY98372586 = -649795556;    double ohxFKqfqGtafDDetY17802462 = -539552998;    double ohxFKqfqGtafDDetY34198665 = -611318659;     ohxFKqfqGtafDDetY61778077 = ohxFKqfqGtafDDetY29186251;     ohxFKqfqGtafDDetY29186251 = ohxFKqfqGtafDDetY43781103;     ohxFKqfqGtafDDetY43781103 = ohxFKqfqGtafDDetY40645650;     ohxFKqfqGtafDDetY40645650 = ohxFKqfqGtafDDetY33502660;     ohxFKqfqGtafDDetY33502660 = ohxFKqfqGtafDDetY68688991;     ohxFKqfqGtafDDetY68688991 = ohxFKqfqGtafDDetY43026024;     ohxFKqfqGtafDDetY43026024 = ohxFKqfqGtafDDetY20604665;     ohxFKqfqGtafDDetY20604665 = ohxFKqfqGtafDDetY26519610;     ohxFKqfqGtafDDetY26519610 = ohxFKqfqGtafDDetY91142105;     ohxFKqfqGtafDDetY91142105 = ohxFKqfqGtafDDetY68285789;     ohxFKqfqGtafDDetY68285789 = ohxFKqfqGtafDDetY92949044;     ohxFKqfqGtafDDetY92949044 = ohxFKqfqGtafDDetY30337384;     ohxFKqfqGtafDDetY30337384 = ohxFKqfqGtafDDetY82690460;     ohxFKqfqGtafDDetY82690460 = ohxFKqfqGtafDDetY22185303;     ohxFKqfqGtafDDetY22185303 = ohxFKqfqGtafDDetY56922902;     ohxFKqfqGtafDDetY56922902 = ohxFKqfqGtafDDetY95860311;     ohxFKqfqGtafDDetY95860311 = ohxFKqfqGtafDDetY64116658;     ohxFKqfqGtafDDetY64116658 = ohxFKqfqGtafDDetY74936110;     ohxFKqfqGtafDDetY74936110 = ohxFKqfqGtafDDetY97924336;     ohxFKqfqGtafDDetY97924336 = ohxFKqfqGtafDDetY39115802;     ohxFKqfqGtafDDetY39115802 = ohxFKqfqGtafDDetY26633265;     ohxFKqfqGtafDDetY26633265 = ohxFKqfqGtafDDetY33839450;     ohxFKqfqGtafDDetY33839450 = ohxFKqfqGtafDDetY73211154;     ohxFKqfqGtafDDetY73211154 = ohxFKqfqGtafDDetY60460461;     ohxFKqfqGtafDDetY60460461 = ohxFKqfqGtafDDetY84081185;     ohxFKqfqGtafDDetY84081185 = ohxFKqfqGtafDDetY78727256;     ohxFKqfqGtafDDetY78727256 = ohxFKqfqGtafDDetY69415107;     ohxFKqfqGtafDDetY69415107 = ohxFKqfqGtafDDetY74282874;     ohxFKqfqGtafDDetY74282874 = ohxFKqfqGtafDDetY17250198;     ohxFKqfqGtafDDetY17250198 = ohxFKqfqGtafDDetY13336085;     ohxFKqfqGtafDDetY13336085 = ohxFKqfqGtafDDetY5782599;     ohxFKqfqGtafDDetY5782599 = ohxFKqfqGtafDDetY60440341;     ohxFKqfqGtafDDetY60440341 = ohxFKqfqGtafDDetY79912464;     ohxFKqfqGtafDDetY79912464 = ohxFKqfqGtafDDetY57302002;     ohxFKqfqGtafDDetY57302002 = ohxFKqfqGtafDDetY61969323;     ohxFKqfqGtafDDetY61969323 = ohxFKqfqGtafDDetY62243509;     ohxFKqfqGtafDDetY62243509 = ohxFKqfqGtafDDetY75976417;     ohxFKqfqGtafDDetY75976417 = ohxFKqfqGtafDDetY25376315;     ohxFKqfqGtafDDetY25376315 = ohxFKqfqGtafDDetY63197850;     ohxFKqfqGtafDDetY63197850 = ohxFKqfqGtafDDetY10163677;     ohxFKqfqGtafDDetY10163677 = ohxFKqfqGtafDDetY19424604;     ohxFKqfqGtafDDetY19424604 = ohxFKqfqGtafDDetY7641998;     ohxFKqfqGtafDDetY7641998 = ohxFKqfqGtafDDetY95608482;     ohxFKqfqGtafDDetY95608482 = ohxFKqfqGtafDDetY96701855;     ohxFKqfqGtafDDetY96701855 = ohxFKqfqGtafDDetY67843370;     ohxFKqfqGtafDDetY67843370 = ohxFKqfqGtafDDetY33303812;     ohxFKqfqGtafDDetY33303812 = ohxFKqfqGtafDDetY46216593;     ohxFKqfqGtafDDetY46216593 = ohxFKqfqGtafDDetY32149691;     ohxFKqfqGtafDDetY32149691 = ohxFKqfqGtafDDetY92959898;     ohxFKqfqGtafDDetY92959898 = ohxFKqfqGtafDDetY72909239;     ohxFKqfqGtafDDetY72909239 = ohxFKqfqGtafDDetY99762628;     ohxFKqfqGtafDDetY99762628 = ohxFKqfqGtafDDetY1152786;     ohxFKqfqGtafDDetY1152786 = ohxFKqfqGtafDDetY85110930;     ohxFKqfqGtafDDetY85110930 = ohxFKqfqGtafDDetY41341475;     ohxFKqfqGtafDDetY41341475 = ohxFKqfqGtafDDetY35144813;     ohxFKqfqGtafDDetY35144813 = ohxFKqfqGtafDDetY95346800;     ohxFKqfqGtafDDetY95346800 = ohxFKqfqGtafDDetY70569948;     ohxFKqfqGtafDDetY70569948 = ohxFKqfqGtafDDetY80185188;     ohxFKqfqGtafDDetY80185188 = ohxFKqfqGtafDDetY49421475;     ohxFKqfqGtafDDetY49421475 = ohxFKqfqGtafDDetY89961734;     ohxFKqfqGtafDDetY89961734 = ohxFKqfqGtafDDetY73610917;     ohxFKqfqGtafDDetY73610917 = ohxFKqfqGtafDDetY46321791;     ohxFKqfqGtafDDetY46321791 = ohxFKqfqGtafDDetY9269412;     ohxFKqfqGtafDDetY9269412 = ohxFKqfqGtafDDetY77806020;     ohxFKqfqGtafDDetY77806020 = ohxFKqfqGtafDDetY62503191;     ohxFKqfqGtafDDetY62503191 = ohxFKqfqGtafDDetY32508703;     ohxFKqfqGtafDDetY32508703 = ohxFKqfqGtafDDetY50424919;     ohxFKqfqGtafDDetY50424919 = ohxFKqfqGtafDDetY25388459;     ohxFKqfqGtafDDetY25388459 = ohxFKqfqGtafDDetY60215979;     ohxFKqfqGtafDDetY60215979 = ohxFKqfqGtafDDetY94679392;     ohxFKqfqGtafDDetY94679392 = ohxFKqfqGtafDDetY19883895;     ohxFKqfqGtafDDetY19883895 = ohxFKqfqGtafDDetY38740344;     ohxFKqfqGtafDDetY38740344 = ohxFKqfqGtafDDetY11738260;     ohxFKqfqGtafDDetY11738260 = ohxFKqfqGtafDDetY87760659;     ohxFKqfqGtafDDetY87760659 = ohxFKqfqGtafDDetY19691198;     ohxFKqfqGtafDDetY19691198 = ohxFKqfqGtafDDetY18991267;     ohxFKqfqGtafDDetY18991267 = ohxFKqfqGtafDDetY38230967;     ohxFKqfqGtafDDetY38230967 = ohxFKqfqGtafDDetY76509299;     ohxFKqfqGtafDDetY76509299 = ohxFKqfqGtafDDetY92617090;     ohxFKqfqGtafDDetY92617090 = ohxFKqfqGtafDDetY50777373;     ohxFKqfqGtafDDetY50777373 = ohxFKqfqGtafDDetY32510663;     ohxFKqfqGtafDDetY32510663 = ohxFKqfqGtafDDetY37265416;     ohxFKqfqGtafDDetY37265416 = ohxFKqfqGtafDDetY81322975;     ohxFKqfqGtafDDetY81322975 = ohxFKqfqGtafDDetY44340959;     ohxFKqfqGtafDDetY44340959 = ohxFKqfqGtafDDetY13573457;     ohxFKqfqGtafDDetY13573457 = ohxFKqfqGtafDDetY4629813;     ohxFKqfqGtafDDetY4629813 = ohxFKqfqGtafDDetY75329410;     ohxFKqfqGtafDDetY75329410 = ohxFKqfqGtafDDetY38570990;     ohxFKqfqGtafDDetY38570990 = ohxFKqfqGtafDDetY22157189;     ohxFKqfqGtafDDetY22157189 = ohxFKqfqGtafDDetY66622522;     ohxFKqfqGtafDDetY66622522 = ohxFKqfqGtafDDetY91673561;     ohxFKqfqGtafDDetY91673561 = ohxFKqfqGtafDDetY95791228;     ohxFKqfqGtafDDetY95791228 = ohxFKqfqGtafDDetY75954840;     ohxFKqfqGtafDDetY75954840 = ohxFKqfqGtafDDetY73236115;     ohxFKqfqGtafDDetY73236115 = ohxFKqfqGtafDDetY36552760;     ohxFKqfqGtafDDetY36552760 = ohxFKqfqGtafDDetY73102813;     ohxFKqfqGtafDDetY73102813 = ohxFKqfqGtafDDetY98372586;     ohxFKqfqGtafDDetY98372586 = ohxFKqfqGtafDDetY17802462;     ohxFKqfqGtafDDetY17802462 = ohxFKqfqGtafDDetY34198665;     ohxFKqfqGtafDDetY34198665 = ohxFKqfqGtafDDetY61778077;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void WrMmGJkVchnQOrGx95506344() {     double owdsChYpbYIKgdiZU67895184 = -723220548;    double owdsChYpbYIKgdiZU72559530 = -897931965;    double owdsChYpbYIKgdiZU59823287 = -921723444;    double owdsChYpbYIKgdiZU67720430 = -399477799;    double owdsChYpbYIKgdiZU2398149 = -439564306;    double owdsChYpbYIKgdiZU60106193 = -681217446;    double owdsChYpbYIKgdiZU3514990 = -467719845;    double owdsChYpbYIKgdiZU70364556 = -303314126;    double owdsChYpbYIKgdiZU91162104 = -797179682;    double owdsChYpbYIKgdiZU32371765 = 9177578;    double owdsChYpbYIKgdiZU28396080 = -699748586;    double owdsChYpbYIKgdiZU24772087 = -816132032;    double owdsChYpbYIKgdiZU16546503 = -405223944;    double owdsChYpbYIKgdiZU65834137 = -568989491;    double owdsChYpbYIKgdiZU17927729 = -605377050;    double owdsChYpbYIKgdiZU98428296 = -341037758;    double owdsChYpbYIKgdiZU89696314 = -116744602;    double owdsChYpbYIKgdiZU61169811 = 49464338;    double owdsChYpbYIKgdiZU36765952 = -691556118;    double owdsChYpbYIKgdiZU78474501 = -650344712;    double owdsChYpbYIKgdiZU27668995 = -218880293;    double owdsChYpbYIKgdiZU13154289 = -278957500;    double owdsChYpbYIKgdiZU69430241 = -616124260;    double owdsChYpbYIKgdiZU57203528 = -86761028;    double owdsChYpbYIKgdiZU4160112 = -343668013;    double owdsChYpbYIKgdiZU8802861 = -845591175;    double owdsChYpbYIKgdiZU77187034 = -635642879;    double owdsChYpbYIKgdiZU82522232 = -375288418;    double owdsChYpbYIKgdiZU40415129 = -798859123;    double owdsChYpbYIKgdiZU85865834 = -254183974;    double owdsChYpbYIKgdiZU5578634 = -400848707;    double owdsChYpbYIKgdiZU3393220 = -482641328;    double owdsChYpbYIKgdiZU70151290 = -667899811;    double owdsChYpbYIKgdiZU74743029 = -286124792;    double owdsChYpbYIKgdiZU18776654 = 91373549;    double owdsChYpbYIKgdiZU30695237 = -467142817;    double owdsChYpbYIKgdiZU43505444 = -825128052;    double owdsChYpbYIKgdiZU10066630 = -52857266;    double owdsChYpbYIKgdiZU98986568 = -80456340;    double owdsChYpbYIKgdiZU50699788 = -468967464;    double owdsChYpbYIKgdiZU15832901 = -487211008;    double owdsChYpbYIKgdiZU51575061 = -2774129;    double owdsChYpbYIKgdiZU85708976 = -223034050;    double owdsChYpbYIKgdiZU83016223 = -833215111;    double owdsChYpbYIKgdiZU8654321 = -595858075;    double owdsChYpbYIKgdiZU79782565 = -880642540;    double owdsChYpbYIKgdiZU51082386 = -45033511;    double owdsChYpbYIKgdiZU54173534 = -919441095;    double owdsChYpbYIKgdiZU41825420 = -235318863;    double owdsChYpbYIKgdiZU5871552 = -913375323;    double owdsChYpbYIKgdiZU8609918 = -840013822;    double owdsChYpbYIKgdiZU10572348 = -711582362;    double owdsChYpbYIKgdiZU49401243 = -423904120;    double owdsChYpbYIKgdiZU81079811 = -731201274;    double owdsChYpbYIKgdiZU67837350 = -860953632;    double owdsChYpbYIKgdiZU54740895 = -344263048;    double owdsChYpbYIKgdiZU3129289 = -181807706;    double owdsChYpbYIKgdiZU2619759 = -734962416;    double owdsChYpbYIKgdiZU63560319 = 44190214;    double owdsChYpbYIKgdiZU93595288 = -593973131;    double owdsChYpbYIKgdiZU82919159 = 54425432;    double owdsChYpbYIKgdiZU20992758 = 7568573;    double owdsChYpbYIKgdiZU29949427 = -504455003;    double owdsChYpbYIKgdiZU5296271 = -442995709;    double owdsChYpbYIKgdiZU26793131 = -589973716;    double owdsChYpbYIKgdiZU25002860 = -117107258;    double owdsChYpbYIKgdiZU54620797 = -48232221;    double owdsChYpbYIKgdiZU41803474 = -19099152;    double owdsChYpbYIKgdiZU47057483 = -560363041;    double owdsChYpbYIKgdiZU87232492 = -38234234;    double owdsChYpbYIKgdiZU54922852 = -515909707;    double owdsChYpbYIKgdiZU79629685 = 36112664;    double owdsChYpbYIKgdiZU62183243 = -870079322;    double owdsChYpbYIKgdiZU86066163 = -122588654;    double owdsChYpbYIKgdiZU62641600 = -63133704;    double owdsChYpbYIKgdiZU76093933 = -116106165;    double owdsChYpbYIKgdiZU27445313 = 44076549;    double owdsChYpbYIKgdiZU86414018 = -782909149;    double owdsChYpbYIKgdiZU48549208 = -490902953;    double owdsChYpbYIKgdiZU24377546 = -463025474;    double owdsChYpbYIKgdiZU57720474 = -700557665;    double owdsChYpbYIKgdiZU23013501 = -716201785;    double owdsChYpbYIKgdiZU40696812 = -39969556;    double owdsChYpbYIKgdiZU34543578 = -885483800;    double owdsChYpbYIKgdiZU77255917 = -414170152;    double owdsChYpbYIKgdiZU95006286 = -689266345;    double owdsChYpbYIKgdiZU53991977 = 41262791;    double owdsChYpbYIKgdiZU89071479 = -936698537;    double owdsChYpbYIKgdiZU6905679 = -425171160;    double owdsChYpbYIKgdiZU64035758 = -564363403;    double owdsChYpbYIKgdiZU27565948 = -185335111;    double owdsChYpbYIKgdiZU40885686 = 9834364;    double owdsChYpbYIKgdiZU46506310 = 2952519;    double owdsChYpbYIKgdiZU5391281 = -486483209;    double owdsChYpbYIKgdiZU67780629 = -423392897;    double owdsChYpbYIKgdiZU94840143 = -394779582;    double owdsChYpbYIKgdiZU21625635 = -498319126;    double owdsChYpbYIKgdiZU80412705 = -780038342;    double owdsChYpbYIKgdiZU56223092 = -143241396;    double owdsChYpbYIKgdiZU83651461 = -723220548;     owdsChYpbYIKgdiZU67895184 = owdsChYpbYIKgdiZU72559530;     owdsChYpbYIKgdiZU72559530 = owdsChYpbYIKgdiZU59823287;     owdsChYpbYIKgdiZU59823287 = owdsChYpbYIKgdiZU67720430;     owdsChYpbYIKgdiZU67720430 = owdsChYpbYIKgdiZU2398149;     owdsChYpbYIKgdiZU2398149 = owdsChYpbYIKgdiZU60106193;     owdsChYpbYIKgdiZU60106193 = owdsChYpbYIKgdiZU3514990;     owdsChYpbYIKgdiZU3514990 = owdsChYpbYIKgdiZU70364556;     owdsChYpbYIKgdiZU70364556 = owdsChYpbYIKgdiZU91162104;     owdsChYpbYIKgdiZU91162104 = owdsChYpbYIKgdiZU32371765;     owdsChYpbYIKgdiZU32371765 = owdsChYpbYIKgdiZU28396080;     owdsChYpbYIKgdiZU28396080 = owdsChYpbYIKgdiZU24772087;     owdsChYpbYIKgdiZU24772087 = owdsChYpbYIKgdiZU16546503;     owdsChYpbYIKgdiZU16546503 = owdsChYpbYIKgdiZU65834137;     owdsChYpbYIKgdiZU65834137 = owdsChYpbYIKgdiZU17927729;     owdsChYpbYIKgdiZU17927729 = owdsChYpbYIKgdiZU98428296;     owdsChYpbYIKgdiZU98428296 = owdsChYpbYIKgdiZU89696314;     owdsChYpbYIKgdiZU89696314 = owdsChYpbYIKgdiZU61169811;     owdsChYpbYIKgdiZU61169811 = owdsChYpbYIKgdiZU36765952;     owdsChYpbYIKgdiZU36765952 = owdsChYpbYIKgdiZU78474501;     owdsChYpbYIKgdiZU78474501 = owdsChYpbYIKgdiZU27668995;     owdsChYpbYIKgdiZU27668995 = owdsChYpbYIKgdiZU13154289;     owdsChYpbYIKgdiZU13154289 = owdsChYpbYIKgdiZU69430241;     owdsChYpbYIKgdiZU69430241 = owdsChYpbYIKgdiZU57203528;     owdsChYpbYIKgdiZU57203528 = owdsChYpbYIKgdiZU4160112;     owdsChYpbYIKgdiZU4160112 = owdsChYpbYIKgdiZU8802861;     owdsChYpbYIKgdiZU8802861 = owdsChYpbYIKgdiZU77187034;     owdsChYpbYIKgdiZU77187034 = owdsChYpbYIKgdiZU82522232;     owdsChYpbYIKgdiZU82522232 = owdsChYpbYIKgdiZU40415129;     owdsChYpbYIKgdiZU40415129 = owdsChYpbYIKgdiZU85865834;     owdsChYpbYIKgdiZU85865834 = owdsChYpbYIKgdiZU5578634;     owdsChYpbYIKgdiZU5578634 = owdsChYpbYIKgdiZU3393220;     owdsChYpbYIKgdiZU3393220 = owdsChYpbYIKgdiZU70151290;     owdsChYpbYIKgdiZU70151290 = owdsChYpbYIKgdiZU74743029;     owdsChYpbYIKgdiZU74743029 = owdsChYpbYIKgdiZU18776654;     owdsChYpbYIKgdiZU18776654 = owdsChYpbYIKgdiZU30695237;     owdsChYpbYIKgdiZU30695237 = owdsChYpbYIKgdiZU43505444;     owdsChYpbYIKgdiZU43505444 = owdsChYpbYIKgdiZU10066630;     owdsChYpbYIKgdiZU10066630 = owdsChYpbYIKgdiZU98986568;     owdsChYpbYIKgdiZU98986568 = owdsChYpbYIKgdiZU50699788;     owdsChYpbYIKgdiZU50699788 = owdsChYpbYIKgdiZU15832901;     owdsChYpbYIKgdiZU15832901 = owdsChYpbYIKgdiZU51575061;     owdsChYpbYIKgdiZU51575061 = owdsChYpbYIKgdiZU85708976;     owdsChYpbYIKgdiZU85708976 = owdsChYpbYIKgdiZU83016223;     owdsChYpbYIKgdiZU83016223 = owdsChYpbYIKgdiZU8654321;     owdsChYpbYIKgdiZU8654321 = owdsChYpbYIKgdiZU79782565;     owdsChYpbYIKgdiZU79782565 = owdsChYpbYIKgdiZU51082386;     owdsChYpbYIKgdiZU51082386 = owdsChYpbYIKgdiZU54173534;     owdsChYpbYIKgdiZU54173534 = owdsChYpbYIKgdiZU41825420;     owdsChYpbYIKgdiZU41825420 = owdsChYpbYIKgdiZU5871552;     owdsChYpbYIKgdiZU5871552 = owdsChYpbYIKgdiZU8609918;     owdsChYpbYIKgdiZU8609918 = owdsChYpbYIKgdiZU10572348;     owdsChYpbYIKgdiZU10572348 = owdsChYpbYIKgdiZU49401243;     owdsChYpbYIKgdiZU49401243 = owdsChYpbYIKgdiZU81079811;     owdsChYpbYIKgdiZU81079811 = owdsChYpbYIKgdiZU67837350;     owdsChYpbYIKgdiZU67837350 = owdsChYpbYIKgdiZU54740895;     owdsChYpbYIKgdiZU54740895 = owdsChYpbYIKgdiZU3129289;     owdsChYpbYIKgdiZU3129289 = owdsChYpbYIKgdiZU2619759;     owdsChYpbYIKgdiZU2619759 = owdsChYpbYIKgdiZU63560319;     owdsChYpbYIKgdiZU63560319 = owdsChYpbYIKgdiZU93595288;     owdsChYpbYIKgdiZU93595288 = owdsChYpbYIKgdiZU82919159;     owdsChYpbYIKgdiZU82919159 = owdsChYpbYIKgdiZU20992758;     owdsChYpbYIKgdiZU20992758 = owdsChYpbYIKgdiZU29949427;     owdsChYpbYIKgdiZU29949427 = owdsChYpbYIKgdiZU5296271;     owdsChYpbYIKgdiZU5296271 = owdsChYpbYIKgdiZU26793131;     owdsChYpbYIKgdiZU26793131 = owdsChYpbYIKgdiZU25002860;     owdsChYpbYIKgdiZU25002860 = owdsChYpbYIKgdiZU54620797;     owdsChYpbYIKgdiZU54620797 = owdsChYpbYIKgdiZU41803474;     owdsChYpbYIKgdiZU41803474 = owdsChYpbYIKgdiZU47057483;     owdsChYpbYIKgdiZU47057483 = owdsChYpbYIKgdiZU87232492;     owdsChYpbYIKgdiZU87232492 = owdsChYpbYIKgdiZU54922852;     owdsChYpbYIKgdiZU54922852 = owdsChYpbYIKgdiZU79629685;     owdsChYpbYIKgdiZU79629685 = owdsChYpbYIKgdiZU62183243;     owdsChYpbYIKgdiZU62183243 = owdsChYpbYIKgdiZU86066163;     owdsChYpbYIKgdiZU86066163 = owdsChYpbYIKgdiZU62641600;     owdsChYpbYIKgdiZU62641600 = owdsChYpbYIKgdiZU76093933;     owdsChYpbYIKgdiZU76093933 = owdsChYpbYIKgdiZU27445313;     owdsChYpbYIKgdiZU27445313 = owdsChYpbYIKgdiZU86414018;     owdsChYpbYIKgdiZU86414018 = owdsChYpbYIKgdiZU48549208;     owdsChYpbYIKgdiZU48549208 = owdsChYpbYIKgdiZU24377546;     owdsChYpbYIKgdiZU24377546 = owdsChYpbYIKgdiZU57720474;     owdsChYpbYIKgdiZU57720474 = owdsChYpbYIKgdiZU23013501;     owdsChYpbYIKgdiZU23013501 = owdsChYpbYIKgdiZU40696812;     owdsChYpbYIKgdiZU40696812 = owdsChYpbYIKgdiZU34543578;     owdsChYpbYIKgdiZU34543578 = owdsChYpbYIKgdiZU77255917;     owdsChYpbYIKgdiZU77255917 = owdsChYpbYIKgdiZU95006286;     owdsChYpbYIKgdiZU95006286 = owdsChYpbYIKgdiZU53991977;     owdsChYpbYIKgdiZU53991977 = owdsChYpbYIKgdiZU89071479;     owdsChYpbYIKgdiZU89071479 = owdsChYpbYIKgdiZU6905679;     owdsChYpbYIKgdiZU6905679 = owdsChYpbYIKgdiZU64035758;     owdsChYpbYIKgdiZU64035758 = owdsChYpbYIKgdiZU27565948;     owdsChYpbYIKgdiZU27565948 = owdsChYpbYIKgdiZU40885686;     owdsChYpbYIKgdiZU40885686 = owdsChYpbYIKgdiZU46506310;     owdsChYpbYIKgdiZU46506310 = owdsChYpbYIKgdiZU5391281;     owdsChYpbYIKgdiZU5391281 = owdsChYpbYIKgdiZU67780629;     owdsChYpbYIKgdiZU67780629 = owdsChYpbYIKgdiZU94840143;     owdsChYpbYIKgdiZU94840143 = owdsChYpbYIKgdiZU21625635;     owdsChYpbYIKgdiZU21625635 = owdsChYpbYIKgdiZU80412705;     owdsChYpbYIKgdiZU80412705 = owdsChYpbYIKgdiZU56223092;     owdsChYpbYIKgdiZU56223092 = owdsChYpbYIKgdiZU83651461;     owdsChYpbYIKgdiZU83651461 = owdsChYpbYIKgdiZU67895184;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FPCRHflHahhsbxze62797944() {     double ArvQmMRAUSxwQtrLW3354153 = -89497448;    double ArvQmMRAUSxwQtrLW85831608 = -778483572;    double ArvQmMRAUSxwQtrLW97209763 = -587390519;    double ArvQmMRAUSxwQtrLW39943785 = -220628870;    double ArvQmMRAUSxwQtrLW58683417 = -545163151;    double ArvQmMRAUSxwQtrLW96329065 = -943565342;    double ArvQmMRAUSxwQtrLW14172397 = -312445579;    double ArvQmMRAUSxwQtrLW32496117 = 52451734;    double ArvQmMRAUSxwQtrLW92679710 = -185511089;    double ArvQmMRAUSxwQtrLW47274477 = -755509420;    double ArvQmMRAUSxwQtrLW66577814 = -501756315;    double ArvQmMRAUSxwQtrLW25340236 = -680252803;    double ArvQmMRAUSxwQtrLW54781148 = -957236449;    double ArvQmMRAUSxwQtrLW18975000 = -545352900;    double ArvQmMRAUSxwQtrLW76122245 = -8802065;    double ArvQmMRAUSxwQtrLW81969597 = 23150635;    double ArvQmMRAUSxwQtrLW45532251 = -643719155;    double ArvQmMRAUSxwQtrLW60930003 = -23963730;    double ArvQmMRAUSxwQtrLW37316282 = 91880460;    double ArvQmMRAUSxwQtrLW72311720 = 23094222;    double ArvQmMRAUSxwQtrLW40755858 = -976154982;    double ArvQmMRAUSxwQtrLW16575813 = -365593794;    double ArvQmMRAUSxwQtrLW39955621 = -253756444;    double ArvQmMRAUSxwQtrLW50996474 = -597585632;    double ArvQmMRAUSxwQtrLW12178063 = -367653211;    double ArvQmMRAUSxwQtrLW19501032 = -534167982;    double ArvQmMRAUSxwQtrLW37607608 = -441927020;    double ArvQmMRAUSxwQtrLW23250657 = -545060593;    double ArvQmMRAUSxwQtrLW87083344 = -272953925;    double ArvQmMRAUSxwQtrLW91399245 = -288691144;    double ArvQmMRAUSxwQtrLW24161866 = -952829194;    double ArvQmMRAUSxwQtrLW41216239 = -255612742;    double ArvQmMRAUSxwQtrLW82369301 = -815495789;    double ArvQmMRAUSxwQtrLW4491456 = -350818606;    double ArvQmMRAUSxwQtrLW84585107 = -959726702;    double ArvQmMRAUSxwQtrLW82256080 = -681708122;    double ArvQmMRAUSxwQtrLW92917260 = -689824092;    double ArvQmMRAUSxwQtrLW61282033 = -35512848;    double ArvQmMRAUSxwQtrLW29377131 = -831332616;    double ArvQmMRAUSxwQtrLW14971321 = 65769990;    double ArvQmMRAUSxwQtrLW49284509 = -256409923;    double ArvQmMRAUSxwQtrLW40940188 = -697319420;    double ArvQmMRAUSxwQtrLW75092354 = -876214836;    double ArvQmMRAUSxwQtrLW23147561 = -337547521;    double ArvQmMRAUSxwQtrLW8670780 = -422369236;    double ArvQmMRAUSxwQtrLW25743894 = -919566172;    double ArvQmMRAUSxwQtrLW70427549 = -529019871;    double ArvQmMRAUSxwQtrLW71899845 = 14032312;    double ArvQmMRAUSxwQtrLW64718086 = -999055743;    double ArvQmMRAUSxwQtrLW30401141 = -304402139;    double ArvQmMRAUSxwQtrLW89380641 = -714290380;    double ArvQmMRAUSxwQtrLW94057820 = -925234709;    double ArvQmMRAUSxwQtrLW18022392 = -179949674;    double ArvQmMRAUSxwQtrLW97071657 = -922709774;    double ArvQmMRAUSxwQtrLW22042087 = -107443778;    double ArvQmMRAUSxwQtrLW86778339 = -723903654;    double ArvQmMRAUSxwQtrLW45875987 = -424727128;    double ArvQmMRAUSxwQtrLW46213290 = -989804887;    double ArvQmMRAUSxwQtrLW27765723 = -852975659;    double ArvQmMRAUSxwQtrLW39182386 = 89004831;    double ArvQmMRAUSxwQtrLW58721457 = -401638323;    double ArvQmMRAUSxwQtrLW90921739 = -767384987;    double ArvQmMRAUSxwQtrLW45412773 = -674594341;    double ArvQmMRAUSxwQtrLW1280465 = -896819945;    double ArvQmMRAUSxwQtrLW23112611 = -802680226;    double ArvQmMRAUSxwQtrLW25361576 = -146143573;    double ArvQmMRAUSxwQtrLW42970935 = -864757015;    double ArvQmMRAUSxwQtrLW50289692 = -506417844;    double ArvQmMRAUSxwQtrLW34389893 = -585626199;    double ArvQmMRAUSxwQtrLW93866165 = -327093943;    double ArvQmMRAUSxwQtrLW89052336 = -287025274;    double ArvQmMRAUSxwQtrLW84250217 = -508206308;    double ArvQmMRAUSxwQtrLW31552873 = -192631115;    double ArvQmMRAUSxwQtrLW22344961 = -973889530;    double ArvQmMRAUSxwQtrLW23027211 = -720495856;    double ArvQmMRAUSxwQtrLW99815669 = -178835563;    double ArvQmMRAUSxwQtrLW41483459 = -489378959;    double ArvQmMRAUSxwQtrLW16808060 = -916208923;    double ArvQmMRAUSxwQtrLW42325694 = -75216396;    double ArvQmMRAUSxwQtrLW86434168 = -448087040;    double ArvQmMRAUSxwQtrLW49073482 = 94851889;    double ArvQmMRAUSxwQtrLW65707763 = -355959332;    double ArvQmMRAUSxwQtrLW58532571 = -546004850;    double ArvQmMRAUSxwQtrLW56682203 = -968551787;    double ArvQmMRAUSxwQtrLW2018605 = -574400765;    double ArvQmMRAUSxwQtrLW30104046 = 72405514;    double ArvQmMRAUSxwQtrLW23193847 = 24336931;    double ArvQmMRAUSxwQtrLW85297643 = -892786015;    double ArvQmMRAUSxwQtrLW82449369 = -143374828;    double ArvQmMRAUSxwQtrLW97806768 = -135823048;    double ArvQmMRAUSxwQtrLW36380093 = -156980995;    double ArvQmMRAUSxwQtrLW46703971 = -700019205;    double ArvQmMRAUSxwQtrLW33516310 = -182537189;    double ArvQmMRAUSxwQtrLW90194744 = -820337447;    double ArvQmMRAUSxwQtrLW56249863 = -532591688;    double ArvQmMRAUSxwQtrLW58362770 = -489024936;    double ArvQmMRAUSxwQtrLW95527414 = 77274921;    double ArvQmMRAUSxwQtrLW73811890 = -979394892;    double ArvQmMRAUSxwQtrLW34950 = -534867295;    double ArvQmMRAUSxwQtrLW83309203 = -89497448;     ArvQmMRAUSxwQtrLW3354153 = ArvQmMRAUSxwQtrLW85831608;     ArvQmMRAUSxwQtrLW85831608 = ArvQmMRAUSxwQtrLW97209763;     ArvQmMRAUSxwQtrLW97209763 = ArvQmMRAUSxwQtrLW39943785;     ArvQmMRAUSxwQtrLW39943785 = ArvQmMRAUSxwQtrLW58683417;     ArvQmMRAUSxwQtrLW58683417 = ArvQmMRAUSxwQtrLW96329065;     ArvQmMRAUSxwQtrLW96329065 = ArvQmMRAUSxwQtrLW14172397;     ArvQmMRAUSxwQtrLW14172397 = ArvQmMRAUSxwQtrLW32496117;     ArvQmMRAUSxwQtrLW32496117 = ArvQmMRAUSxwQtrLW92679710;     ArvQmMRAUSxwQtrLW92679710 = ArvQmMRAUSxwQtrLW47274477;     ArvQmMRAUSxwQtrLW47274477 = ArvQmMRAUSxwQtrLW66577814;     ArvQmMRAUSxwQtrLW66577814 = ArvQmMRAUSxwQtrLW25340236;     ArvQmMRAUSxwQtrLW25340236 = ArvQmMRAUSxwQtrLW54781148;     ArvQmMRAUSxwQtrLW54781148 = ArvQmMRAUSxwQtrLW18975000;     ArvQmMRAUSxwQtrLW18975000 = ArvQmMRAUSxwQtrLW76122245;     ArvQmMRAUSxwQtrLW76122245 = ArvQmMRAUSxwQtrLW81969597;     ArvQmMRAUSxwQtrLW81969597 = ArvQmMRAUSxwQtrLW45532251;     ArvQmMRAUSxwQtrLW45532251 = ArvQmMRAUSxwQtrLW60930003;     ArvQmMRAUSxwQtrLW60930003 = ArvQmMRAUSxwQtrLW37316282;     ArvQmMRAUSxwQtrLW37316282 = ArvQmMRAUSxwQtrLW72311720;     ArvQmMRAUSxwQtrLW72311720 = ArvQmMRAUSxwQtrLW40755858;     ArvQmMRAUSxwQtrLW40755858 = ArvQmMRAUSxwQtrLW16575813;     ArvQmMRAUSxwQtrLW16575813 = ArvQmMRAUSxwQtrLW39955621;     ArvQmMRAUSxwQtrLW39955621 = ArvQmMRAUSxwQtrLW50996474;     ArvQmMRAUSxwQtrLW50996474 = ArvQmMRAUSxwQtrLW12178063;     ArvQmMRAUSxwQtrLW12178063 = ArvQmMRAUSxwQtrLW19501032;     ArvQmMRAUSxwQtrLW19501032 = ArvQmMRAUSxwQtrLW37607608;     ArvQmMRAUSxwQtrLW37607608 = ArvQmMRAUSxwQtrLW23250657;     ArvQmMRAUSxwQtrLW23250657 = ArvQmMRAUSxwQtrLW87083344;     ArvQmMRAUSxwQtrLW87083344 = ArvQmMRAUSxwQtrLW91399245;     ArvQmMRAUSxwQtrLW91399245 = ArvQmMRAUSxwQtrLW24161866;     ArvQmMRAUSxwQtrLW24161866 = ArvQmMRAUSxwQtrLW41216239;     ArvQmMRAUSxwQtrLW41216239 = ArvQmMRAUSxwQtrLW82369301;     ArvQmMRAUSxwQtrLW82369301 = ArvQmMRAUSxwQtrLW4491456;     ArvQmMRAUSxwQtrLW4491456 = ArvQmMRAUSxwQtrLW84585107;     ArvQmMRAUSxwQtrLW84585107 = ArvQmMRAUSxwQtrLW82256080;     ArvQmMRAUSxwQtrLW82256080 = ArvQmMRAUSxwQtrLW92917260;     ArvQmMRAUSxwQtrLW92917260 = ArvQmMRAUSxwQtrLW61282033;     ArvQmMRAUSxwQtrLW61282033 = ArvQmMRAUSxwQtrLW29377131;     ArvQmMRAUSxwQtrLW29377131 = ArvQmMRAUSxwQtrLW14971321;     ArvQmMRAUSxwQtrLW14971321 = ArvQmMRAUSxwQtrLW49284509;     ArvQmMRAUSxwQtrLW49284509 = ArvQmMRAUSxwQtrLW40940188;     ArvQmMRAUSxwQtrLW40940188 = ArvQmMRAUSxwQtrLW75092354;     ArvQmMRAUSxwQtrLW75092354 = ArvQmMRAUSxwQtrLW23147561;     ArvQmMRAUSxwQtrLW23147561 = ArvQmMRAUSxwQtrLW8670780;     ArvQmMRAUSxwQtrLW8670780 = ArvQmMRAUSxwQtrLW25743894;     ArvQmMRAUSxwQtrLW25743894 = ArvQmMRAUSxwQtrLW70427549;     ArvQmMRAUSxwQtrLW70427549 = ArvQmMRAUSxwQtrLW71899845;     ArvQmMRAUSxwQtrLW71899845 = ArvQmMRAUSxwQtrLW64718086;     ArvQmMRAUSxwQtrLW64718086 = ArvQmMRAUSxwQtrLW30401141;     ArvQmMRAUSxwQtrLW30401141 = ArvQmMRAUSxwQtrLW89380641;     ArvQmMRAUSxwQtrLW89380641 = ArvQmMRAUSxwQtrLW94057820;     ArvQmMRAUSxwQtrLW94057820 = ArvQmMRAUSxwQtrLW18022392;     ArvQmMRAUSxwQtrLW18022392 = ArvQmMRAUSxwQtrLW97071657;     ArvQmMRAUSxwQtrLW97071657 = ArvQmMRAUSxwQtrLW22042087;     ArvQmMRAUSxwQtrLW22042087 = ArvQmMRAUSxwQtrLW86778339;     ArvQmMRAUSxwQtrLW86778339 = ArvQmMRAUSxwQtrLW45875987;     ArvQmMRAUSxwQtrLW45875987 = ArvQmMRAUSxwQtrLW46213290;     ArvQmMRAUSxwQtrLW46213290 = ArvQmMRAUSxwQtrLW27765723;     ArvQmMRAUSxwQtrLW27765723 = ArvQmMRAUSxwQtrLW39182386;     ArvQmMRAUSxwQtrLW39182386 = ArvQmMRAUSxwQtrLW58721457;     ArvQmMRAUSxwQtrLW58721457 = ArvQmMRAUSxwQtrLW90921739;     ArvQmMRAUSxwQtrLW90921739 = ArvQmMRAUSxwQtrLW45412773;     ArvQmMRAUSxwQtrLW45412773 = ArvQmMRAUSxwQtrLW1280465;     ArvQmMRAUSxwQtrLW1280465 = ArvQmMRAUSxwQtrLW23112611;     ArvQmMRAUSxwQtrLW23112611 = ArvQmMRAUSxwQtrLW25361576;     ArvQmMRAUSxwQtrLW25361576 = ArvQmMRAUSxwQtrLW42970935;     ArvQmMRAUSxwQtrLW42970935 = ArvQmMRAUSxwQtrLW50289692;     ArvQmMRAUSxwQtrLW50289692 = ArvQmMRAUSxwQtrLW34389893;     ArvQmMRAUSxwQtrLW34389893 = ArvQmMRAUSxwQtrLW93866165;     ArvQmMRAUSxwQtrLW93866165 = ArvQmMRAUSxwQtrLW89052336;     ArvQmMRAUSxwQtrLW89052336 = ArvQmMRAUSxwQtrLW84250217;     ArvQmMRAUSxwQtrLW84250217 = ArvQmMRAUSxwQtrLW31552873;     ArvQmMRAUSxwQtrLW31552873 = ArvQmMRAUSxwQtrLW22344961;     ArvQmMRAUSxwQtrLW22344961 = ArvQmMRAUSxwQtrLW23027211;     ArvQmMRAUSxwQtrLW23027211 = ArvQmMRAUSxwQtrLW99815669;     ArvQmMRAUSxwQtrLW99815669 = ArvQmMRAUSxwQtrLW41483459;     ArvQmMRAUSxwQtrLW41483459 = ArvQmMRAUSxwQtrLW16808060;     ArvQmMRAUSxwQtrLW16808060 = ArvQmMRAUSxwQtrLW42325694;     ArvQmMRAUSxwQtrLW42325694 = ArvQmMRAUSxwQtrLW86434168;     ArvQmMRAUSxwQtrLW86434168 = ArvQmMRAUSxwQtrLW49073482;     ArvQmMRAUSxwQtrLW49073482 = ArvQmMRAUSxwQtrLW65707763;     ArvQmMRAUSxwQtrLW65707763 = ArvQmMRAUSxwQtrLW58532571;     ArvQmMRAUSxwQtrLW58532571 = ArvQmMRAUSxwQtrLW56682203;     ArvQmMRAUSxwQtrLW56682203 = ArvQmMRAUSxwQtrLW2018605;     ArvQmMRAUSxwQtrLW2018605 = ArvQmMRAUSxwQtrLW30104046;     ArvQmMRAUSxwQtrLW30104046 = ArvQmMRAUSxwQtrLW23193847;     ArvQmMRAUSxwQtrLW23193847 = ArvQmMRAUSxwQtrLW85297643;     ArvQmMRAUSxwQtrLW85297643 = ArvQmMRAUSxwQtrLW82449369;     ArvQmMRAUSxwQtrLW82449369 = ArvQmMRAUSxwQtrLW97806768;     ArvQmMRAUSxwQtrLW97806768 = ArvQmMRAUSxwQtrLW36380093;     ArvQmMRAUSxwQtrLW36380093 = ArvQmMRAUSxwQtrLW46703971;     ArvQmMRAUSxwQtrLW46703971 = ArvQmMRAUSxwQtrLW33516310;     ArvQmMRAUSxwQtrLW33516310 = ArvQmMRAUSxwQtrLW90194744;     ArvQmMRAUSxwQtrLW90194744 = ArvQmMRAUSxwQtrLW56249863;     ArvQmMRAUSxwQtrLW56249863 = ArvQmMRAUSxwQtrLW58362770;     ArvQmMRAUSxwQtrLW58362770 = ArvQmMRAUSxwQtrLW95527414;     ArvQmMRAUSxwQtrLW95527414 = ArvQmMRAUSxwQtrLW73811890;     ArvQmMRAUSxwQtrLW73811890 = ArvQmMRAUSxwQtrLW34950;     ArvQmMRAUSxwQtrLW34950 = ArvQmMRAUSxwQtrLW83309203;     ArvQmMRAUSxwQtrLW83309203 = ArvQmMRAUSxwQtrLW3354153;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void dmnzVfyXornPLitI77847012() {     double aFawwPEIzsXFlqWoz9471259 = -201399337;    double aFawwPEIzsXFlqWoz29204888 = -488083353;    double aFawwPEIzsXFlqWoz13251947 = -738223440;    double aFawwPEIzsXFlqWoz67018566 = -411377500;    double aFawwPEIzsXFlqWoz27578906 = 34186978;    double aFawwPEIzsXFlqWoz87746267 = -108061577;    double aFawwPEIzsXFlqWoz74661362 = -451410556;    double aFawwPEIzsXFlqWoz82256008 = -234422179;    double aFawwPEIzsXFlqWoz57322205 = 46456245;    double aFawwPEIzsXFlqWoz88504136 = 15515103;    double aFawwPEIzsXFlqWoz26688105 = -379193901;    double aFawwPEIzsXFlqWoz57163278 = -24659005;    double aFawwPEIzsXFlqWoz40990266 = -297127538;    double aFawwPEIzsXFlqWoz2118677 = 22408532;    double aFawwPEIzsXFlqWoz71864672 = -699264232;    double aFawwPEIzsXFlqWoz23474991 = 50151092;    double aFawwPEIzsXFlqWoz39368254 = -160264267;    double aFawwPEIzsXFlqWoz57983156 = -789486373;    double aFawwPEIzsXFlqWoz99146123 = -313888636;    double aFawwPEIzsXFlqWoz52861885 = -333808697;    double aFawwPEIzsXFlqWoz29309051 = -831642437;    double aFawwPEIzsXFlqWoz3096837 = -273523511;    double aFawwPEIzsXFlqWoz75546413 = -273931;    double aFawwPEIzsXFlqWoz34988848 = -819890620;    double aFawwPEIzsXFlqWoz55877712 = -740849244;    double aFawwPEIzsXFlqWoz44222707 = -412823305;    double aFawwPEIzsXFlqWoz36067386 = -342589223;    double aFawwPEIzsXFlqWoz36357783 = -610989546;    double aFawwPEIzsXFlqWoz53215599 = -495336630;    double aFawwPEIzsXFlqWoz60014882 = -106162012;    double aFawwPEIzsXFlqWoz16404415 = -866150132;    double aFawwPEIzsXFlqWoz38826860 = 34940442;    double aFawwPEIzsXFlqWoz92080249 = -708109159;    double aFawwPEIzsXFlqWoz99322019 = -102360371;    double aFawwPEIzsXFlqWoz46059759 = -498459807;    double aFawwPEIzsXFlqWoz50981994 = -895734230;    double aFawwPEIzsXFlqWoz74179195 = -643459961;    double aFawwPEIzsXFlqWoz95372245 = -907018857;    double aFawwPEIzsXFlqWoz2987385 = -825256113;    double aFawwPEIzsXFlqWoz2473258 = -407669233;    double aFawwPEIzsXFlqWoz54953733 = -321683817;    double aFawwPEIzsXFlqWoz73090645 = -967461698;    double aFawwPEIzsXFlqWoz53159333 = -957019420;    double aFawwPEIzsXFlqWoz10555302 = -356890459;    double aFawwPEIzsXFlqWoz20623245 = -84267391;    double aFawwPEIzsXFlqWoz37683089 = -904782288;    double aFawwPEIzsXFlqWoz88206123 = -647795409;    double aFawwPEIzsXFlqWoz79856786 = 4136779;    double aFawwPEIzsXFlqWoz74393815 = 15207269;    double aFawwPEIzsXFlqWoz43312794 = -391412947;    double aFawwPEIzsXFlqWoz25081320 = -556584051;    double aFawwPEIzsXFlqWoz4867540 = -725961029;    double aFawwPEIzsXFlqWoz66270849 = 28260739;    double aFawwPEIzsXFlqWoz93040537 = -143179919;    double aFawwPEIzsXFlqWoz48537961 = -807975561;    double aFawwPEIzsXFlqWoz6374423 = -927875826;    double aFawwPEIzsXFlqWoz53658475 = -387809422;    double aFawwPEIzsXFlqWoz78263099 = -918332821;    double aFawwPEIzsXFlqWoz11140854 = -670528257;    double aFawwPEIzsXFlqWoz83356199 = -552989717;    double aFawwPEIzsXFlqWoz51678881 = -765472355;    double aFawwPEIzsXFlqWoz38303580 = -840421010;    double aFawwPEIzsXFlqWoz29040409 = -739085550;    double aFawwPEIzsXFlqWoz97307323 = -847381743;    double aFawwPEIzsXFlqWoz72099721 = -118334766;    double aFawwPEIzsXFlqWoz87861244 = -314134344;    double aFawwPEIzsXFlqWoz65083029 = -316549847;    double aFawwPEIzsXFlqWoz41668247 = -94767167;    double aFawwPEIzsXFlqWoz56058917 = -479131662;    double aFawwPEIzsXFlqWoz20882679 = -803530002;    double aFawwPEIzsXFlqWoz49295795 = -306388947;    double aFawwPEIzsXFlqWoz43996008 = -253245411;    double aFawwPEIzsXFlqWoz54995772 = -964230261;    double aFawwPEIzsXFlqWoz96672865 = -906219404;    double aFawwPEIzsXFlqWoz97908151 = 87875119;    double aFawwPEIzsXFlqWoz56218406 = -864180740;    double aFawwPEIzsXFlqWoz49937504 = -316504091;    double aFawwPEIzsXFlqWoz64991111 = -643383472;    double aFawwPEIzsXFlqWoz14365603 = -635623230;    double aFawwPEIzsXFlqWoz18194624 = -836066956;    double aFawwPEIzsXFlqWoz56016583 = -765027897;    double aFawwPEIzsXFlqWoz56210600 = -246726003;    double aFawwPEIzsXFlqWoz61963967 = -526196816;    double aFawwPEIzsXFlqWoz9902806 = -3923683;    double aFawwPEIzsXFlqWoz34933563 = -549577961;    double aFawwPEIzsXFlqWoz11536876 = -40189104;    double aFawwPEIzsXFlqWoz72556011 = -993320297;    double aFawwPEIzsXFlqWoz99039711 = -464929240;    double aFawwPEIzsXFlqWoz50784059 = -294384811;    double aFawwPEIzsXFlqWoz39685337 = -570583981;    double aFawwPEIzsXFlqWoz97323518 = -407924808;    double aFawwPEIzsXFlqWoz95916095 = -725127141;    double aFawwPEIzsXFlqWoz84231392 = -136490601;    double aFawwPEIzsXFlqWoz19631186 = -172266396;    double aFawwPEIzsXFlqWoz50794377 = -642196879;    double aFawwPEIzsXFlqWoz16650153 = -481262807;    double aFawwPEIzsXFlqWoz44050236 = -128376148;    double aFawwPEIzsXFlqWoz55852009 = -9637677;    double aFawwPEIzsXFlqWoz38455581 = -138555694;    double aFawwPEIzsXFlqWoz32762000 = -201399337;     aFawwPEIzsXFlqWoz9471259 = aFawwPEIzsXFlqWoz29204888;     aFawwPEIzsXFlqWoz29204888 = aFawwPEIzsXFlqWoz13251947;     aFawwPEIzsXFlqWoz13251947 = aFawwPEIzsXFlqWoz67018566;     aFawwPEIzsXFlqWoz67018566 = aFawwPEIzsXFlqWoz27578906;     aFawwPEIzsXFlqWoz27578906 = aFawwPEIzsXFlqWoz87746267;     aFawwPEIzsXFlqWoz87746267 = aFawwPEIzsXFlqWoz74661362;     aFawwPEIzsXFlqWoz74661362 = aFawwPEIzsXFlqWoz82256008;     aFawwPEIzsXFlqWoz82256008 = aFawwPEIzsXFlqWoz57322205;     aFawwPEIzsXFlqWoz57322205 = aFawwPEIzsXFlqWoz88504136;     aFawwPEIzsXFlqWoz88504136 = aFawwPEIzsXFlqWoz26688105;     aFawwPEIzsXFlqWoz26688105 = aFawwPEIzsXFlqWoz57163278;     aFawwPEIzsXFlqWoz57163278 = aFawwPEIzsXFlqWoz40990266;     aFawwPEIzsXFlqWoz40990266 = aFawwPEIzsXFlqWoz2118677;     aFawwPEIzsXFlqWoz2118677 = aFawwPEIzsXFlqWoz71864672;     aFawwPEIzsXFlqWoz71864672 = aFawwPEIzsXFlqWoz23474991;     aFawwPEIzsXFlqWoz23474991 = aFawwPEIzsXFlqWoz39368254;     aFawwPEIzsXFlqWoz39368254 = aFawwPEIzsXFlqWoz57983156;     aFawwPEIzsXFlqWoz57983156 = aFawwPEIzsXFlqWoz99146123;     aFawwPEIzsXFlqWoz99146123 = aFawwPEIzsXFlqWoz52861885;     aFawwPEIzsXFlqWoz52861885 = aFawwPEIzsXFlqWoz29309051;     aFawwPEIzsXFlqWoz29309051 = aFawwPEIzsXFlqWoz3096837;     aFawwPEIzsXFlqWoz3096837 = aFawwPEIzsXFlqWoz75546413;     aFawwPEIzsXFlqWoz75546413 = aFawwPEIzsXFlqWoz34988848;     aFawwPEIzsXFlqWoz34988848 = aFawwPEIzsXFlqWoz55877712;     aFawwPEIzsXFlqWoz55877712 = aFawwPEIzsXFlqWoz44222707;     aFawwPEIzsXFlqWoz44222707 = aFawwPEIzsXFlqWoz36067386;     aFawwPEIzsXFlqWoz36067386 = aFawwPEIzsXFlqWoz36357783;     aFawwPEIzsXFlqWoz36357783 = aFawwPEIzsXFlqWoz53215599;     aFawwPEIzsXFlqWoz53215599 = aFawwPEIzsXFlqWoz60014882;     aFawwPEIzsXFlqWoz60014882 = aFawwPEIzsXFlqWoz16404415;     aFawwPEIzsXFlqWoz16404415 = aFawwPEIzsXFlqWoz38826860;     aFawwPEIzsXFlqWoz38826860 = aFawwPEIzsXFlqWoz92080249;     aFawwPEIzsXFlqWoz92080249 = aFawwPEIzsXFlqWoz99322019;     aFawwPEIzsXFlqWoz99322019 = aFawwPEIzsXFlqWoz46059759;     aFawwPEIzsXFlqWoz46059759 = aFawwPEIzsXFlqWoz50981994;     aFawwPEIzsXFlqWoz50981994 = aFawwPEIzsXFlqWoz74179195;     aFawwPEIzsXFlqWoz74179195 = aFawwPEIzsXFlqWoz95372245;     aFawwPEIzsXFlqWoz95372245 = aFawwPEIzsXFlqWoz2987385;     aFawwPEIzsXFlqWoz2987385 = aFawwPEIzsXFlqWoz2473258;     aFawwPEIzsXFlqWoz2473258 = aFawwPEIzsXFlqWoz54953733;     aFawwPEIzsXFlqWoz54953733 = aFawwPEIzsXFlqWoz73090645;     aFawwPEIzsXFlqWoz73090645 = aFawwPEIzsXFlqWoz53159333;     aFawwPEIzsXFlqWoz53159333 = aFawwPEIzsXFlqWoz10555302;     aFawwPEIzsXFlqWoz10555302 = aFawwPEIzsXFlqWoz20623245;     aFawwPEIzsXFlqWoz20623245 = aFawwPEIzsXFlqWoz37683089;     aFawwPEIzsXFlqWoz37683089 = aFawwPEIzsXFlqWoz88206123;     aFawwPEIzsXFlqWoz88206123 = aFawwPEIzsXFlqWoz79856786;     aFawwPEIzsXFlqWoz79856786 = aFawwPEIzsXFlqWoz74393815;     aFawwPEIzsXFlqWoz74393815 = aFawwPEIzsXFlqWoz43312794;     aFawwPEIzsXFlqWoz43312794 = aFawwPEIzsXFlqWoz25081320;     aFawwPEIzsXFlqWoz25081320 = aFawwPEIzsXFlqWoz4867540;     aFawwPEIzsXFlqWoz4867540 = aFawwPEIzsXFlqWoz66270849;     aFawwPEIzsXFlqWoz66270849 = aFawwPEIzsXFlqWoz93040537;     aFawwPEIzsXFlqWoz93040537 = aFawwPEIzsXFlqWoz48537961;     aFawwPEIzsXFlqWoz48537961 = aFawwPEIzsXFlqWoz6374423;     aFawwPEIzsXFlqWoz6374423 = aFawwPEIzsXFlqWoz53658475;     aFawwPEIzsXFlqWoz53658475 = aFawwPEIzsXFlqWoz78263099;     aFawwPEIzsXFlqWoz78263099 = aFawwPEIzsXFlqWoz11140854;     aFawwPEIzsXFlqWoz11140854 = aFawwPEIzsXFlqWoz83356199;     aFawwPEIzsXFlqWoz83356199 = aFawwPEIzsXFlqWoz51678881;     aFawwPEIzsXFlqWoz51678881 = aFawwPEIzsXFlqWoz38303580;     aFawwPEIzsXFlqWoz38303580 = aFawwPEIzsXFlqWoz29040409;     aFawwPEIzsXFlqWoz29040409 = aFawwPEIzsXFlqWoz97307323;     aFawwPEIzsXFlqWoz97307323 = aFawwPEIzsXFlqWoz72099721;     aFawwPEIzsXFlqWoz72099721 = aFawwPEIzsXFlqWoz87861244;     aFawwPEIzsXFlqWoz87861244 = aFawwPEIzsXFlqWoz65083029;     aFawwPEIzsXFlqWoz65083029 = aFawwPEIzsXFlqWoz41668247;     aFawwPEIzsXFlqWoz41668247 = aFawwPEIzsXFlqWoz56058917;     aFawwPEIzsXFlqWoz56058917 = aFawwPEIzsXFlqWoz20882679;     aFawwPEIzsXFlqWoz20882679 = aFawwPEIzsXFlqWoz49295795;     aFawwPEIzsXFlqWoz49295795 = aFawwPEIzsXFlqWoz43996008;     aFawwPEIzsXFlqWoz43996008 = aFawwPEIzsXFlqWoz54995772;     aFawwPEIzsXFlqWoz54995772 = aFawwPEIzsXFlqWoz96672865;     aFawwPEIzsXFlqWoz96672865 = aFawwPEIzsXFlqWoz97908151;     aFawwPEIzsXFlqWoz97908151 = aFawwPEIzsXFlqWoz56218406;     aFawwPEIzsXFlqWoz56218406 = aFawwPEIzsXFlqWoz49937504;     aFawwPEIzsXFlqWoz49937504 = aFawwPEIzsXFlqWoz64991111;     aFawwPEIzsXFlqWoz64991111 = aFawwPEIzsXFlqWoz14365603;     aFawwPEIzsXFlqWoz14365603 = aFawwPEIzsXFlqWoz18194624;     aFawwPEIzsXFlqWoz18194624 = aFawwPEIzsXFlqWoz56016583;     aFawwPEIzsXFlqWoz56016583 = aFawwPEIzsXFlqWoz56210600;     aFawwPEIzsXFlqWoz56210600 = aFawwPEIzsXFlqWoz61963967;     aFawwPEIzsXFlqWoz61963967 = aFawwPEIzsXFlqWoz9902806;     aFawwPEIzsXFlqWoz9902806 = aFawwPEIzsXFlqWoz34933563;     aFawwPEIzsXFlqWoz34933563 = aFawwPEIzsXFlqWoz11536876;     aFawwPEIzsXFlqWoz11536876 = aFawwPEIzsXFlqWoz72556011;     aFawwPEIzsXFlqWoz72556011 = aFawwPEIzsXFlqWoz99039711;     aFawwPEIzsXFlqWoz99039711 = aFawwPEIzsXFlqWoz50784059;     aFawwPEIzsXFlqWoz50784059 = aFawwPEIzsXFlqWoz39685337;     aFawwPEIzsXFlqWoz39685337 = aFawwPEIzsXFlqWoz97323518;     aFawwPEIzsXFlqWoz97323518 = aFawwPEIzsXFlqWoz95916095;     aFawwPEIzsXFlqWoz95916095 = aFawwPEIzsXFlqWoz84231392;     aFawwPEIzsXFlqWoz84231392 = aFawwPEIzsXFlqWoz19631186;     aFawwPEIzsXFlqWoz19631186 = aFawwPEIzsXFlqWoz50794377;     aFawwPEIzsXFlqWoz50794377 = aFawwPEIzsXFlqWoz16650153;     aFawwPEIzsXFlqWoz16650153 = aFawwPEIzsXFlqWoz44050236;     aFawwPEIzsXFlqWoz44050236 = aFawwPEIzsXFlqWoz55852009;     aFawwPEIzsXFlqWoz55852009 = aFawwPEIzsXFlqWoz38455581;     aFawwPEIzsXFlqWoz38455581 = aFawwPEIzsXFlqWoz32762000;     aFawwPEIzsXFlqWoz32762000 = aFawwPEIzsXFlqWoz9471259;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void mTVCmhMYxOIfbkhc45138611() {     double ZUYoKQRQLSvXOvmyO44930227 = -667676236;    double ZUYoKQRQLSvXOvmyO42476966 = -368634959;    double ZUYoKQRQLSvXOvmyO50638424 = -403890515;    double ZUYoKQRQLSvXOvmyO39241921 = -232528572;    double ZUYoKQRQLSvXOvmyO83864175 = -71411868;    double ZUYoKQRQLSvXOvmyO23969140 = -370409472;    double ZUYoKQRQLSvXOvmyO85318768 = -296136290;    double ZUYoKQRQLSvXOvmyO44387569 = -978656320;    double ZUYoKQRQLSvXOvmyO58839811 = -441875162;    double ZUYoKQRQLSvXOvmyO3406849 = -749171895;    double ZUYoKQRQLSvXOvmyO64869839 = -181201631;    double ZUYoKQRQLSvXOvmyO57731426 = -988779777;    double ZUYoKQRQLSvXOvmyO79224911 = -849140042;    double ZUYoKQRQLSvXOvmyO55259540 = 46045122;    double ZUYoKQRQLSvXOvmyO30059189 = -102689247;    double ZUYoKQRQLSvXOvmyO7016292 = -685660515;    double ZUYoKQRQLSvXOvmyO95204189 = -687238820;    double ZUYoKQRQLSvXOvmyO57743348 = -862914442;    double ZUYoKQRQLSvXOvmyO99696453 = -630452058;    double ZUYoKQRQLSvXOvmyO46699103 = -760369763;    double ZUYoKQRQLSvXOvmyO42395914 = -488917126;    double ZUYoKQRQLSvXOvmyO6518362 = -360159805;    double ZUYoKQRQLSvXOvmyO46071793 = -737906115;    double ZUYoKQRQLSvXOvmyO28781793 = -230715224;    double ZUYoKQRQLSvXOvmyO63895664 = -764834443;    double ZUYoKQRQLSvXOvmyO54920879 = -101400112;    double ZUYoKQRQLSvXOvmyO96487960 = -148873363;    double ZUYoKQRQLSvXOvmyO77086207 = -780761721;    double ZUYoKQRQLSvXOvmyO99883814 = 30568568;    double ZUYoKQRQLSvXOvmyO65548293 = -140669182;    double ZUYoKQRQLSvXOvmyO34987647 = -318130619;    double ZUYoKQRQLSvXOvmyO76649878 = -838030973;    double ZUYoKQRQLSvXOvmyO4298261 = -855705136;    double ZUYoKQRQLSvXOvmyO29070446 = -167054184;    double ZUYoKQRQLSvXOvmyO11868213 = -449560058;    double ZUYoKQRQLSvXOvmyO2542838 = -10299536;    double ZUYoKQRQLSvXOvmyO23591012 = -508156002;    double ZUYoKQRQLSvXOvmyO46587649 = -889674438;    double ZUYoKQRQLSvXOvmyO33377946 = -476132389;    double ZUYoKQRQLSvXOvmyO66744790 = -972931779;    double ZUYoKQRQLSvXOvmyO88405340 = -90882731;    double ZUYoKQRQLSvXOvmyO62455771 = -562006989;    double ZUYoKQRQLSvXOvmyO42542711 = -510200206;    double ZUYoKQRQLSvXOvmyO50686640 = -961222869;    double ZUYoKQRQLSvXOvmyO20639703 = 89221448;    double ZUYoKQRQLSvXOvmyO83644417 = -943705921;    double ZUYoKQRQLSvXOvmyO7551288 = -31781769;    double ZUYoKQRQLSvXOvmyO97583098 = -162389814;    double ZUYoKQRQLSvXOvmyO97286480 = -748529611;    double ZUYoKQRQLSvXOvmyO67842383 = -882439762;    double ZUYoKQRQLSvXOvmyO5852044 = -430860608;    double ZUYoKQRQLSvXOvmyO88353012 = -939613375;    double ZUYoKQRQLSvXOvmyO34891998 = -827784815;    double ZUYoKQRQLSvXOvmyO9032385 = -334688419;    double ZUYoKQRQLSvXOvmyO2742698 = -54465707;    double ZUYoKQRQLSvXOvmyO38411866 = -207516432;    double ZUYoKQRQLSvXOvmyO96405173 = -630728845;    double ZUYoKQRQLSvXOvmyO21856631 = -73175291;    double ZUYoKQRQLSvXOvmyO75346257 = -467694130;    double ZUYoKQRQLSvXOvmyO28943297 = -970011756;    double ZUYoKQRQLSvXOvmyO27481179 = -121536109;    double ZUYoKQRQLSvXOvmyO8232561 = -515374570;    double ZUYoKQRQLSvXOvmyO44503755 = -909224888;    double ZUYoKQRQLSvXOvmyO93291517 = -201205980;    double ZUYoKQRQLSvXOvmyO68419201 = -331041277;    double ZUYoKQRQLSvXOvmyO88219960 = -343170659;    double ZUYoKQRQLSvXOvmyO53433166 = -33074641;    double ZUYoKQRQLSvXOvmyO50154466 = -582085858;    double ZUYoKQRQLSvXOvmyO43391327 = -504394820;    double ZUYoKQRQLSvXOvmyO27516351 = 7610289;    double ZUYoKQRQLSvXOvmyO83425279 = -77504514;    double ZUYoKQRQLSvXOvmyO48616541 = -797564382;    double ZUYoKQRQLSvXOvmyO24365402 = -286782053;    double ZUYoKQRQLSvXOvmyO32951663 = -657520279;    double ZUYoKQRQLSvXOvmyO58293763 = -569487033;    double ZUYoKQRQLSvXOvmyO79940142 = -926910138;    double ZUYoKQRQLSvXOvmyO63975650 = -849959599;    double ZUYoKQRQLSvXOvmyO95385153 = -776683246;    double ZUYoKQRQLSvXOvmyO8142090 = -219936673;    double ZUYoKQRQLSvXOvmyO80251246 = -821128523;    double ZUYoKQRQLSvXOvmyO47369591 = 30381657;    double ZUYoKQRQLSvXOvmyO98904862 = -986483550;    double ZUYoKQRQLSvXOvmyO79799726 = 67767890;    double ZUYoKQRQLSvXOvmyO32041431 = -86991670;    double ZUYoKQRQLSvXOvmyO59696250 = -709808574;    double ZUYoKQRQLSvXOvmyO46634635 = -378517244;    double ZUYoKQRQLSvXOvmyO41757881 = 89753842;    double ZUYoKQRQLSvXOvmyO95265875 = -421016718;    double ZUYoKQRQLSvXOvmyO26327749 = -12588478;    double ZUYoKQRQLSvXOvmyO73456347 = -142043626;    double ZUYoKQRQLSvXOvmyO6137664 = -379570692;    double ZUYoKQRQLSvXOvmyO1734381 = -334980711;    double ZUYoKQRQLSvXOvmyO71241392 = -321980309;    double ZUYoKQRQLSvXOvmyO4434650 = -506120634;    double ZUYoKQRQLSvXOvmyO39263611 = -751395670;    double ZUYoKQRQLSvXOvmyO80172779 = -575508161;    double ZUYoKQRQLSvXOvmyO17952017 = -652782101;    double ZUYoKQRQLSvXOvmyO49251193 = -208994227;    double ZUYoKQRQLSvXOvmyO82267438 = -530181593;    double ZUYoKQRQLSvXOvmyO32419743 = -667676236;     ZUYoKQRQLSvXOvmyO44930227 = ZUYoKQRQLSvXOvmyO42476966;     ZUYoKQRQLSvXOvmyO42476966 = ZUYoKQRQLSvXOvmyO50638424;     ZUYoKQRQLSvXOvmyO50638424 = ZUYoKQRQLSvXOvmyO39241921;     ZUYoKQRQLSvXOvmyO39241921 = ZUYoKQRQLSvXOvmyO83864175;     ZUYoKQRQLSvXOvmyO83864175 = ZUYoKQRQLSvXOvmyO23969140;     ZUYoKQRQLSvXOvmyO23969140 = ZUYoKQRQLSvXOvmyO85318768;     ZUYoKQRQLSvXOvmyO85318768 = ZUYoKQRQLSvXOvmyO44387569;     ZUYoKQRQLSvXOvmyO44387569 = ZUYoKQRQLSvXOvmyO58839811;     ZUYoKQRQLSvXOvmyO58839811 = ZUYoKQRQLSvXOvmyO3406849;     ZUYoKQRQLSvXOvmyO3406849 = ZUYoKQRQLSvXOvmyO64869839;     ZUYoKQRQLSvXOvmyO64869839 = ZUYoKQRQLSvXOvmyO57731426;     ZUYoKQRQLSvXOvmyO57731426 = ZUYoKQRQLSvXOvmyO79224911;     ZUYoKQRQLSvXOvmyO79224911 = ZUYoKQRQLSvXOvmyO55259540;     ZUYoKQRQLSvXOvmyO55259540 = ZUYoKQRQLSvXOvmyO30059189;     ZUYoKQRQLSvXOvmyO30059189 = ZUYoKQRQLSvXOvmyO7016292;     ZUYoKQRQLSvXOvmyO7016292 = ZUYoKQRQLSvXOvmyO95204189;     ZUYoKQRQLSvXOvmyO95204189 = ZUYoKQRQLSvXOvmyO57743348;     ZUYoKQRQLSvXOvmyO57743348 = ZUYoKQRQLSvXOvmyO99696453;     ZUYoKQRQLSvXOvmyO99696453 = ZUYoKQRQLSvXOvmyO46699103;     ZUYoKQRQLSvXOvmyO46699103 = ZUYoKQRQLSvXOvmyO42395914;     ZUYoKQRQLSvXOvmyO42395914 = ZUYoKQRQLSvXOvmyO6518362;     ZUYoKQRQLSvXOvmyO6518362 = ZUYoKQRQLSvXOvmyO46071793;     ZUYoKQRQLSvXOvmyO46071793 = ZUYoKQRQLSvXOvmyO28781793;     ZUYoKQRQLSvXOvmyO28781793 = ZUYoKQRQLSvXOvmyO63895664;     ZUYoKQRQLSvXOvmyO63895664 = ZUYoKQRQLSvXOvmyO54920879;     ZUYoKQRQLSvXOvmyO54920879 = ZUYoKQRQLSvXOvmyO96487960;     ZUYoKQRQLSvXOvmyO96487960 = ZUYoKQRQLSvXOvmyO77086207;     ZUYoKQRQLSvXOvmyO77086207 = ZUYoKQRQLSvXOvmyO99883814;     ZUYoKQRQLSvXOvmyO99883814 = ZUYoKQRQLSvXOvmyO65548293;     ZUYoKQRQLSvXOvmyO65548293 = ZUYoKQRQLSvXOvmyO34987647;     ZUYoKQRQLSvXOvmyO34987647 = ZUYoKQRQLSvXOvmyO76649878;     ZUYoKQRQLSvXOvmyO76649878 = ZUYoKQRQLSvXOvmyO4298261;     ZUYoKQRQLSvXOvmyO4298261 = ZUYoKQRQLSvXOvmyO29070446;     ZUYoKQRQLSvXOvmyO29070446 = ZUYoKQRQLSvXOvmyO11868213;     ZUYoKQRQLSvXOvmyO11868213 = ZUYoKQRQLSvXOvmyO2542838;     ZUYoKQRQLSvXOvmyO2542838 = ZUYoKQRQLSvXOvmyO23591012;     ZUYoKQRQLSvXOvmyO23591012 = ZUYoKQRQLSvXOvmyO46587649;     ZUYoKQRQLSvXOvmyO46587649 = ZUYoKQRQLSvXOvmyO33377946;     ZUYoKQRQLSvXOvmyO33377946 = ZUYoKQRQLSvXOvmyO66744790;     ZUYoKQRQLSvXOvmyO66744790 = ZUYoKQRQLSvXOvmyO88405340;     ZUYoKQRQLSvXOvmyO88405340 = ZUYoKQRQLSvXOvmyO62455771;     ZUYoKQRQLSvXOvmyO62455771 = ZUYoKQRQLSvXOvmyO42542711;     ZUYoKQRQLSvXOvmyO42542711 = ZUYoKQRQLSvXOvmyO50686640;     ZUYoKQRQLSvXOvmyO50686640 = ZUYoKQRQLSvXOvmyO20639703;     ZUYoKQRQLSvXOvmyO20639703 = ZUYoKQRQLSvXOvmyO83644417;     ZUYoKQRQLSvXOvmyO83644417 = ZUYoKQRQLSvXOvmyO7551288;     ZUYoKQRQLSvXOvmyO7551288 = ZUYoKQRQLSvXOvmyO97583098;     ZUYoKQRQLSvXOvmyO97583098 = ZUYoKQRQLSvXOvmyO97286480;     ZUYoKQRQLSvXOvmyO97286480 = ZUYoKQRQLSvXOvmyO67842383;     ZUYoKQRQLSvXOvmyO67842383 = ZUYoKQRQLSvXOvmyO5852044;     ZUYoKQRQLSvXOvmyO5852044 = ZUYoKQRQLSvXOvmyO88353012;     ZUYoKQRQLSvXOvmyO88353012 = ZUYoKQRQLSvXOvmyO34891998;     ZUYoKQRQLSvXOvmyO34891998 = ZUYoKQRQLSvXOvmyO9032385;     ZUYoKQRQLSvXOvmyO9032385 = ZUYoKQRQLSvXOvmyO2742698;     ZUYoKQRQLSvXOvmyO2742698 = ZUYoKQRQLSvXOvmyO38411866;     ZUYoKQRQLSvXOvmyO38411866 = ZUYoKQRQLSvXOvmyO96405173;     ZUYoKQRQLSvXOvmyO96405173 = ZUYoKQRQLSvXOvmyO21856631;     ZUYoKQRQLSvXOvmyO21856631 = ZUYoKQRQLSvXOvmyO75346257;     ZUYoKQRQLSvXOvmyO75346257 = ZUYoKQRQLSvXOvmyO28943297;     ZUYoKQRQLSvXOvmyO28943297 = ZUYoKQRQLSvXOvmyO27481179;     ZUYoKQRQLSvXOvmyO27481179 = ZUYoKQRQLSvXOvmyO8232561;     ZUYoKQRQLSvXOvmyO8232561 = ZUYoKQRQLSvXOvmyO44503755;     ZUYoKQRQLSvXOvmyO44503755 = ZUYoKQRQLSvXOvmyO93291517;     ZUYoKQRQLSvXOvmyO93291517 = ZUYoKQRQLSvXOvmyO68419201;     ZUYoKQRQLSvXOvmyO68419201 = ZUYoKQRQLSvXOvmyO88219960;     ZUYoKQRQLSvXOvmyO88219960 = ZUYoKQRQLSvXOvmyO53433166;     ZUYoKQRQLSvXOvmyO53433166 = ZUYoKQRQLSvXOvmyO50154466;     ZUYoKQRQLSvXOvmyO50154466 = ZUYoKQRQLSvXOvmyO43391327;     ZUYoKQRQLSvXOvmyO43391327 = ZUYoKQRQLSvXOvmyO27516351;     ZUYoKQRQLSvXOvmyO27516351 = ZUYoKQRQLSvXOvmyO83425279;     ZUYoKQRQLSvXOvmyO83425279 = ZUYoKQRQLSvXOvmyO48616541;     ZUYoKQRQLSvXOvmyO48616541 = ZUYoKQRQLSvXOvmyO24365402;     ZUYoKQRQLSvXOvmyO24365402 = ZUYoKQRQLSvXOvmyO32951663;     ZUYoKQRQLSvXOvmyO32951663 = ZUYoKQRQLSvXOvmyO58293763;     ZUYoKQRQLSvXOvmyO58293763 = ZUYoKQRQLSvXOvmyO79940142;     ZUYoKQRQLSvXOvmyO79940142 = ZUYoKQRQLSvXOvmyO63975650;     ZUYoKQRQLSvXOvmyO63975650 = ZUYoKQRQLSvXOvmyO95385153;     ZUYoKQRQLSvXOvmyO95385153 = ZUYoKQRQLSvXOvmyO8142090;     ZUYoKQRQLSvXOvmyO8142090 = ZUYoKQRQLSvXOvmyO80251246;     ZUYoKQRQLSvXOvmyO80251246 = ZUYoKQRQLSvXOvmyO47369591;     ZUYoKQRQLSvXOvmyO47369591 = ZUYoKQRQLSvXOvmyO98904862;     ZUYoKQRQLSvXOvmyO98904862 = ZUYoKQRQLSvXOvmyO79799726;     ZUYoKQRQLSvXOvmyO79799726 = ZUYoKQRQLSvXOvmyO32041431;     ZUYoKQRQLSvXOvmyO32041431 = ZUYoKQRQLSvXOvmyO59696250;     ZUYoKQRQLSvXOvmyO59696250 = ZUYoKQRQLSvXOvmyO46634635;     ZUYoKQRQLSvXOvmyO46634635 = ZUYoKQRQLSvXOvmyO41757881;     ZUYoKQRQLSvXOvmyO41757881 = ZUYoKQRQLSvXOvmyO95265875;     ZUYoKQRQLSvXOvmyO95265875 = ZUYoKQRQLSvXOvmyO26327749;     ZUYoKQRQLSvXOvmyO26327749 = ZUYoKQRQLSvXOvmyO73456347;     ZUYoKQRQLSvXOvmyO73456347 = ZUYoKQRQLSvXOvmyO6137664;     ZUYoKQRQLSvXOvmyO6137664 = ZUYoKQRQLSvXOvmyO1734381;     ZUYoKQRQLSvXOvmyO1734381 = ZUYoKQRQLSvXOvmyO71241392;     ZUYoKQRQLSvXOvmyO71241392 = ZUYoKQRQLSvXOvmyO4434650;     ZUYoKQRQLSvXOvmyO4434650 = ZUYoKQRQLSvXOvmyO39263611;     ZUYoKQRQLSvXOvmyO39263611 = ZUYoKQRQLSvXOvmyO80172779;     ZUYoKQRQLSvXOvmyO80172779 = ZUYoKQRQLSvXOvmyO17952017;     ZUYoKQRQLSvXOvmyO17952017 = ZUYoKQRQLSvXOvmyO49251193;     ZUYoKQRQLSvXOvmyO49251193 = ZUYoKQRQLSvXOvmyO82267438;     ZUYoKQRQLSvXOvmyO82267438 = ZUYoKQRQLSvXOvmyO32419743;     ZUYoKQRQLSvXOvmyO32419743 = ZUYoKQRQLSvXOvmyO44930227;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void cUpGgHCogNxwdOpl60187679() {     double ugjEbvMsSHlCwUrlZ51047334 = -779578125;    double ugjEbvMsSHlCwUrlZ85850245 = -78234740;    double ugjEbvMsSHlCwUrlZ66680607 = -554723436;    double ugjEbvMsSHlCwUrlZ66316702 = -423277202;    double ugjEbvMsSHlCwUrlZ52759664 = -592061739;    double ugjEbvMsSHlCwUrlZ15386342 = -634905707;    double ugjEbvMsSHlCwUrlZ45807734 = -435101268;    double ugjEbvMsSHlCwUrlZ94147459 = -165530233;    double ugjEbvMsSHlCwUrlZ23482306 = -209907827;    double ugjEbvMsSHlCwUrlZ44636508 = 21852627;    double ugjEbvMsSHlCwUrlZ24980130 = -58639217;    double ugjEbvMsSHlCwUrlZ89554469 = -333185979;    double ugjEbvMsSHlCwUrlZ65434030 = -189031131;    double ugjEbvMsSHlCwUrlZ38403217 = -486193445;    double ugjEbvMsSHlCwUrlZ25801615 = -793151413;    double ugjEbvMsSHlCwUrlZ48521686 = -658660059;    double ugjEbvMsSHlCwUrlZ89040192 = -203783933;    double ugjEbvMsSHlCwUrlZ54796501 = -528437084;    double ugjEbvMsSHlCwUrlZ61526295 = 63778845;    double ugjEbvMsSHlCwUrlZ27249268 = -17272682;    double ugjEbvMsSHlCwUrlZ30949107 = -344404582;    double ugjEbvMsSHlCwUrlZ93039385 = -268089521;    double ugjEbvMsSHlCwUrlZ81662584 = -484423602;    double ugjEbvMsSHlCwUrlZ12774167 = -453020212;    double ugjEbvMsSHlCwUrlZ7595314 = -38030476;    double ugjEbvMsSHlCwUrlZ79642554 = 19944565;    double ugjEbvMsSHlCwUrlZ94947738 = -49535566;    double ugjEbvMsSHlCwUrlZ90193332 = -846690674;    double ugjEbvMsSHlCwUrlZ66016069 = -191814137;    double ugjEbvMsSHlCwUrlZ34163930 = 41859950;    double ugjEbvMsSHlCwUrlZ27230196 = -231451557;    double ugjEbvMsSHlCwUrlZ74260500 = -547477788;    double ugjEbvMsSHlCwUrlZ14009209 = -748318507;    double ugjEbvMsSHlCwUrlZ23901011 = 81404050;    double ugjEbvMsSHlCwUrlZ73342865 = 11706837;    double ugjEbvMsSHlCwUrlZ71268750 = -224325643;    double ugjEbvMsSHlCwUrlZ4852947 = -461791871;    double ugjEbvMsSHlCwUrlZ80677861 = -661180448;    double ugjEbvMsSHlCwUrlZ6988200 = -470055886;    double ugjEbvMsSHlCwUrlZ54246728 = -346371002;    double ugjEbvMsSHlCwUrlZ94074564 = -156156625;    double ugjEbvMsSHlCwUrlZ94606228 = -832149267;    double ugjEbvMsSHlCwUrlZ20609689 = -591004790;    double ugjEbvMsSHlCwUrlZ38094380 = -980565808;    double ugjEbvMsSHlCwUrlZ32592168 = -672676706;    double ugjEbvMsSHlCwUrlZ95583612 = -928922037;    double ugjEbvMsSHlCwUrlZ25329862 = -150557307;    double ugjEbvMsSHlCwUrlZ5540040 = -172285346;    double ugjEbvMsSHlCwUrlZ6962210 = -834266599;    double ugjEbvMsSHlCwUrlZ80754035 = -969450570;    double ugjEbvMsSHlCwUrlZ41552722 = -273154280;    double ugjEbvMsSHlCwUrlZ99162731 = -740339695;    double ugjEbvMsSHlCwUrlZ83140455 = -619574403;    double ugjEbvMsSHlCwUrlZ5001265 = -655158564;    double ugjEbvMsSHlCwUrlZ29238572 = -754997489;    double ugjEbvMsSHlCwUrlZ58007949 = -411488605;    double ugjEbvMsSHlCwUrlZ4187661 = -593811138;    double ugjEbvMsSHlCwUrlZ53906441 = -1703225;    double ugjEbvMsSHlCwUrlZ58721388 = -285246727;    double ugjEbvMsSHlCwUrlZ73117109 = -512006304;    double ugjEbvMsSHlCwUrlZ20438604 = -485370141;    double ugjEbvMsSHlCwUrlZ55614402 = -588410594;    double ugjEbvMsSHlCwUrlZ28131390 = -973716097;    double ugjEbvMsSHlCwUrlZ89318376 = -151767778;    double ugjEbvMsSHlCwUrlZ17406312 = -746695816;    double ugjEbvMsSHlCwUrlZ50719629 = -511161429;    double ugjEbvMsSHlCwUrlZ75545260 = -584867473;    double ugjEbvMsSHlCwUrlZ41533020 = -170435182;    double ugjEbvMsSHlCwUrlZ65060351 = -397900283;    double ugjEbvMsSHlCwUrlZ54532865 = -468825770;    double ugjEbvMsSHlCwUrlZ43668739 = -96868188;    double ugjEbvMsSHlCwUrlZ8362332 = -542603485;    double ugjEbvMsSHlCwUrlZ47808301 = 41618801;    double ugjEbvMsSHlCwUrlZ7279567 = -589850153;    double ugjEbvMsSHlCwUrlZ33174704 = -861116058;    double ugjEbvMsSHlCwUrlZ36342879 = -512255315;    double ugjEbvMsSHlCwUrlZ72429696 = -677084732;    double ugjEbvMsSHlCwUrlZ43568204 = -503857795;    double ugjEbvMsSHlCwUrlZ80181998 = -780343506;    double ugjEbvMsSHlCwUrlZ12011702 = -109108439;    double ugjEbvMsSHlCwUrlZ54312692 = -829498129;    double ugjEbvMsSHlCwUrlZ89407699 = -877250220;    double ugjEbvMsSHlCwUrlZ83231122 = 87575925;    double ugjEbvMsSHlCwUrlZ85262034 = -222363567;    double ugjEbvMsSHlCwUrlZ92611208 = -684985770;    double ugjEbvMsSHlCwUrlZ28067465 = -491111862;    double ugjEbvMsSHlCwUrlZ91120045 = -927903386;    double ugjEbvMsSHlCwUrlZ9007944 = 6840057;    double ugjEbvMsSHlCwUrlZ94662438 = -163598461;    double ugjEbvMsSHlCwUrlZ15334916 = -576804559;    double ugjEbvMsSHlCwUrlZ67081089 = -630514506;    double ugjEbvMsSHlCwUrlZ50946506 = -360088647;    double ugjEbvMsSHlCwUrlZ21956474 = -275933721;    double ugjEbvMsSHlCwUrlZ33871090 = -958049583;    double ugjEbvMsSHlCwUrlZ33808125 = -861000861;    double ugjEbvMsSHlCwUrlZ38460163 = -567746032;    double ugjEbvMsSHlCwUrlZ66474838 = -858433171;    double ugjEbvMsSHlCwUrlZ31291313 = -339237012;    double ugjEbvMsSHlCwUrlZ20688069 = -133869992;    double ugjEbvMsSHlCwUrlZ81872539 = -779578125;     ugjEbvMsSHlCwUrlZ51047334 = ugjEbvMsSHlCwUrlZ85850245;     ugjEbvMsSHlCwUrlZ85850245 = ugjEbvMsSHlCwUrlZ66680607;     ugjEbvMsSHlCwUrlZ66680607 = ugjEbvMsSHlCwUrlZ66316702;     ugjEbvMsSHlCwUrlZ66316702 = ugjEbvMsSHlCwUrlZ52759664;     ugjEbvMsSHlCwUrlZ52759664 = ugjEbvMsSHlCwUrlZ15386342;     ugjEbvMsSHlCwUrlZ15386342 = ugjEbvMsSHlCwUrlZ45807734;     ugjEbvMsSHlCwUrlZ45807734 = ugjEbvMsSHlCwUrlZ94147459;     ugjEbvMsSHlCwUrlZ94147459 = ugjEbvMsSHlCwUrlZ23482306;     ugjEbvMsSHlCwUrlZ23482306 = ugjEbvMsSHlCwUrlZ44636508;     ugjEbvMsSHlCwUrlZ44636508 = ugjEbvMsSHlCwUrlZ24980130;     ugjEbvMsSHlCwUrlZ24980130 = ugjEbvMsSHlCwUrlZ89554469;     ugjEbvMsSHlCwUrlZ89554469 = ugjEbvMsSHlCwUrlZ65434030;     ugjEbvMsSHlCwUrlZ65434030 = ugjEbvMsSHlCwUrlZ38403217;     ugjEbvMsSHlCwUrlZ38403217 = ugjEbvMsSHlCwUrlZ25801615;     ugjEbvMsSHlCwUrlZ25801615 = ugjEbvMsSHlCwUrlZ48521686;     ugjEbvMsSHlCwUrlZ48521686 = ugjEbvMsSHlCwUrlZ89040192;     ugjEbvMsSHlCwUrlZ89040192 = ugjEbvMsSHlCwUrlZ54796501;     ugjEbvMsSHlCwUrlZ54796501 = ugjEbvMsSHlCwUrlZ61526295;     ugjEbvMsSHlCwUrlZ61526295 = ugjEbvMsSHlCwUrlZ27249268;     ugjEbvMsSHlCwUrlZ27249268 = ugjEbvMsSHlCwUrlZ30949107;     ugjEbvMsSHlCwUrlZ30949107 = ugjEbvMsSHlCwUrlZ93039385;     ugjEbvMsSHlCwUrlZ93039385 = ugjEbvMsSHlCwUrlZ81662584;     ugjEbvMsSHlCwUrlZ81662584 = ugjEbvMsSHlCwUrlZ12774167;     ugjEbvMsSHlCwUrlZ12774167 = ugjEbvMsSHlCwUrlZ7595314;     ugjEbvMsSHlCwUrlZ7595314 = ugjEbvMsSHlCwUrlZ79642554;     ugjEbvMsSHlCwUrlZ79642554 = ugjEbvMsSHlCwUrlZ94947738;     ugjEbvMsSHlCwUrlZ94947738 = ugjEbvMsSHlCwUrlZ90193332;     ugjEbvMsSHlCwUrlZ90193332 = ugjEbvMsSHlCwUrlZ66016069;     ugjEbvMsSHlCwUrlZ66016069 = ugjEbvMsSHlCwUrlZ34163930;     ugjEbvMsSHlCwUrlZ34163930 = ugjEbvMsSHlCwUrlZ27230196;     ugjEbvMsSHlCwUrlZ27230196 = ugjEbvMsSHlCwUrlZ74260500;     ugjEbvMsSHlCwUrlZ74260500 = ugjEbvMsSHlCwUrlZ14009209;     ugjEbvMsSHlCwUrlZ14009209 = ugjEbvMsSHlCwUrlZ23901011;     ugjEbvMsSHlCwUrlZ23901011 = ugjEbvMsSHlCwUrlZ73342865;     ugjEbvMsSHlCwUrlZ73342865 = ugjEbvMsSHlCwUrlZ71268750;     ugjEbvMsSHlCwUrlZ71268750 = ugjEbvMsSHlCwUrlZ4852947;     ugjEbvMsSHlCwUrlZ4852947 = ugjEbvMsSHlCwUrlZ80677861;     ugjEbvMsSHlCwUrlZ80677861 = ugjEbvMsSHlCwUrlZ6988200;     ugjEbvMsSHlCwUrlZ6988200 = ugjEbvMsSHlCwUrlZ54246728;     ugjEbvMsSHlCwUrlZ54246728 = ugjEbvMsSHlCwUrlZ94074564;     ugjEbvMsSHlCwUrlZ94074564 = ugjEbvMsSHlCwUrlZ94606228;     ugjEbvMsSHlCwUrlZ94606228 = ugjEbvMsSHlCwUrlZ20609689;     ugjEbvMsSHlCwUrlZ20609689 = ugjEbvMsSHlCwUrlZ38094380;     ugjEbvMsSHlCwUrlZ38094380 = ugjEbvMsSHlCwUrlZ32592168;     ugjEbvMsSHlCwUrlZ32592168 = ugjEbvMsSHlCwUrlZ95583612;     ugjEbvMsSHlCwUrlZ95583612 = ugjEbvMsSHlCwUrlZ25329862;     ugjEbvMsSHlCwUrlZ25329862 = ugjEbvMsSHlCwUrlZ5540040;     ugjEbvMsSHlCwUrlZ5540040 = ugjEbvMsSHlCwUrlZ6962210;     ugjEbvMsSHlCwUrlZ6962210 = ugjEbvMsSHlCwUrlZ80754035;     ugjEbvMsSHlCwUrlZ80754035 = ugjEbvMsSHlCwUrlZ41552722;     ugjEbvMsSHlCwUrlZ41552722 = ugjEbvMsSHlCwUrlZ99162731;     ugjEbvMsSHlCwUrlZ99162731 = ugjEbvMsSHlCwUrlZ83140455;     ugjEbvMsSHlCwUrlZ83140455 = ugjEbvMsSHlCwUrlZ5001265;     ugjEbvMsSHlCwUrlZ5001265 = ugjEbvMsSHlCwUrlZ29238572;     ugjEbvMsSHlCwUrlZ29238572 = ugjEbvMsSHlCwUrlZ58007949;     ugjEbvMsSHlCwUrlZ58007949 = ugjEbvMsSHlCwUrlZ4187661;     ugjEbvMsSHlCwUrlZ4187661 = ugjEbvMsSHlCwUrlZ53906441;     ugjEbvMsSHlCwUrlZ53906441 = ugjEbvMsSHlCwUrlZ58721388;     ugjEbvMsSHlCwUrlZ58721388 = ugjEbvMsSHlCwUrlZ73117109;     ugjEbvMsSHlCwUrlZ73117109 = ugjEbvMsSHlCwUrlZ20438604;     ugjEbvMsSHlCwUrlZ20438604 = ugjEbvMsSHlCwUrlZ55614402;     ugjEbvMsSHlCwUrlZ55614402 = ugjEbvMsSHlCwUrlZ28131390;     ugjEbvMsSHlCwUrlZ28131390 = ugjEbvMsSHlCwUrlZ89318376;     ugjEbvMsSHlCwUrlZ89318376 = ugjEbvMsSHlCwUrlZ17406312;     ugjEbvMsSHlCwUrlZ17406312 = ugjEbvMsSHlCwUrlZ50719629;     ugjEbvMsSHlCwUrlZ50719629 = ugjEbvMsSHlCwUrlZ75545260;     ugjEbvMsSHlCwUrlZ75545260 = ugjEbvMsSHlCwUrlZ41533020;     ugjEbvMsSHlCwUrlZ41533020 = ugjEbvMsSHlCwUrlZ65060351;     ugjEbvMsSHlCwUrlZ65060351 = ugjEbvMsSHlCwUrlZ54532865;     ugjEbvMsSHlCwUrlZ54532865 = ugjEbvMsSHlCwUrlZ43668739;     ugjEbvMsSHlCwUrlZ43668739 = ugjEbvMsSHlCwUrlZ8362332;     ugjEbvMsSHlCwUrlZ8362332 = ugjEbvMsSHlCwUrlZ47808301;     ugjEbvMsSHlCwUrlZ47808301 = ugjEbvMsSHlCwUrlZ7279567;     ugjEbvMsSHlCwUrlZ7279567 = ugjEbvMsSHlCwUrlZ33174704;     ugjEbvMsSHlCwUrlZ33174704 = ugjEbvMsSHlCwUrlZ36342879;     ugjEbvMsSHlCwUrlZ36342879 = ugjEbvMsSHlCwUrlZ72429696;     ugjEbvMsSHlCwUrlZ72429696 = ugjEbvMsSHlCwUrlZ43568204;     ugjEbvMsSHlCwUrlZ43568204 = ugjEbvMsSHlCwUrlZ80181998;     ugjEbvMsSHlCwUrlZ80181998 = ugjEbvMsSHlCwUrlZ12011702;     ugjEbvMsSHlCwUrlZ12011702 = ugjEbvMsSHlCwUrlZ54312692;     ugjEbvMsSHlCwUrlZ54312692 = ugjEbvMsSHlCwUrlZ89407699;     ugjEbvMsSHlCwUrlZ89407699 = ugjEbvMsSHlCwUrlZ83231122;     ugjEbvMsSHlCwUrlZ83231122 = ugjEbvMsSHlCwUrlZ85262034;     ugjEbvMsSHlCwUrlZ85262034 = ugjEbvMsSHlCwUrlZ92611208;     ugjEbvMsSHlCwUrlZ92611208 = ugjEbvMsSHlCwUrlZ28067465;     ugjEbvMsSHlCwUrlZ28067465 = ugjEbvMsSHlCwUrlZ91120045;     ugjEbvMsSHlCwUrlZ91120045 = ugjEbvMsSHlCwUrlZ9007944;     ugjEbvMsSHlCwUrlZ9007944 = ugjEbvMsSHlCwUrlZ94662438;     ugjEbvMsSHlCwUrlZ94662438 = ugjEbvMsSHlCwUrlZ15334916;     ugjEbvMsSHlCwUrlZ15334916 = ugjEbvMsSHlCwUrlZ67081089;     ugjEbvMsSHlCwUrlZ67081089 = ugjEbvMsSHlCwUrlZ50946506;     ugjEbvMsSHlCwUrlZ50946506 = ugjEbvMsSHlCwUrlZ21956474;     ugjEbvMsSHlCwUrlZ21956474 = ugjEbvMsSHlCwUrlZ33871090;     ugjEbvMsSHlCwUrlZ33871090 = ugjEbvMsSHlCwUrlZ33808125;     ugjEbvMsSHlCwUrlZ33808125 = ugjEbvMsSHlCwUrlZ38460163;     ugjEbvMsSHlCwUrlZ38460163 = ugjEbvMsSHlCwUrlZ66474838;     ugjEbvMsSHlCwUrlZ66474838 = ugjEbvMsSHlCwUrlZ31291313;     ugjEbvMsSHlCwUrlZ31291313 = ugjEbvMsSHlCwUrlZ20688069;     ugjEbvMsSHlCwUrlZ20688069 = ugjEbvMsSHlCwUrlZ81872539;     ugjEbvMsSHlCwUrlZ81872539 = ugjEbvMsSHlCwUrlZ51047334;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VAKhqNPKfsBNZNFz27479279() {     double anYhRIxBVPmYSXGOp86506302 = -145855025;    double anYhRIxBVPmYSXGOp99122323 = 41213654;    double anYhRIxBVPmYSXGOp4067085 = -220390511;    double anYhRIxBVPmYSXGOp38540057 = -244428273;    double anYhRIxBVPmYSXGOp9044934 = -697660584;    double anYhRIxBVPmYSXGOp51609214 = -897253603;    double anYhRIxBVPmYSXGOp56465141 = -279827002;    double anYhRIxBVPmYSXGOp56279021 = -909764373;    double anYhRIxBVPmYSXGOp24999911 = -698239234;    double anYhRIxBVPmYSXGOp59539220 = -742834371;    double anYhRIxBVPmYSXGOp63161864 = -960646947;    double anYhRIxBVPmYSXGOp90122617 = -197306750;    double anYhRIxBVPmYSXGOp3668676 = -741043636;    double anYhRIxBVPmYSXGOp91544079 = -462556855;    double anYhRIxBVPmYSXGOp83996131 = -196576428;    double anYhRIxBVPmYSXGOp32062987 = -294471666;    double anYhRIxBVPmYSXGOp44876129 = -730758486;    double anYhRIxBVPmYSXGOp54556692 = -601865153;    double anYhRIxBVPmYSXGOp62076625 = -252784576;    double anYhRIxBVPmYSXGOp21086487 = -443833748;    double anYhRIxBVPmYSXGOp44035970 = -1679270;    double anYhRIxBVPmYSXGOp96460909 = -354725815;    double anYhRIxBVPmYSXGOp52187964 = -122055786;    double anYhRIxBVPmYSXGOp6567112 = -963844816;    double anYhRIxBVPmYSXGOp15613265 = -62015674;    double anYhRIxBVPmYSXGOp90340726 = -768632242;    double anYhRIxBVPmYSXGOp55368313 = -955819707;    double anYhRIxBVPmYSXGOp30921758 = 83537151;    double anYhRIxBVPmYSXGOp12684285 = -765908939;    double anYhRIxBVPmYSXGOp39697341 = 7352780;    double anYhRIxBVPmYSXGOp45813428 = -783432044;    double anYhRIxBVPmYSXGOp12083519 = -320449203;    double anYhRIxBVPmYSXGOp26227220 = -895914484;    double anYhRIxBVPmYSXGOp53649437 = 16710237;    double anYhRIxBVPmYSXGOp39151319 = 60606586;    double anYhRIxBVPmYSXGOp22829595 = -438890949;    double anYhRIxBVPmYSXGOp54264763 = -326487912;    double anYhRIxBVPmYSXGOp31893265 = -643836029;    double anYhRIxBVPmYSXGOp37378761 = -120932163;    double anYhRIxBVPmYSXGOp18518261 = -911633548;    double anYhRIxBVPmYSXGOp27526172 = 74644461;    double anYhRIxBVPmYSXGOp83971355 = -426694558;    double anYhRIxBVPmYSXGOp9993068 = -144185576;    double anYhRIxBVPmYSXGOp78225718 = -484898218;    double anYhRIxBVPmYSXGOp32608627 = -499187868;    double anYhRIxBVPmYSXGOp41544941 = -967845669;    double anYhRIxBVPmYSXGOp44675026 = -634543667;    double anYhRIxBVPmYSXGOp23266351 = -338811940;    double anYhRIxBVPmYSXGOp29854876 = -498003480;    double anYhRIxBVPmYSXGOp5283625 = -360477386;    double anYhRIxBVPmYSXGOp22323446 = -147430837;    double anYhRIxBVPmYSXGOp82648204 = -953992042;    double anYhRIxBVPmYSXGOp51761603 = -375619957;    double anYhRIxBVPmYSXGOp20993112 = -846667064;    double anYhRIxBVPmYSXGOp83443308 = -1487635;    double anYhRIxBVPmYSXGOp90045392 = -791129210;    double anYhRIxBVPmYSXGOp46934360 = -836730561;    double anYhRIxBVPmYSXGOp97499972 = -256545696;    double anYhRIxBVPmYSXGOp22926792 = -82412600;    double anYhRIxBVPmYSXGOp18704207 = -929028343;    double anYhRIxBVPmYSXGOp96240901 = -941433896;    double anYhRIxBVPmYSXGOp25543383 = -263364153;    double anYhRIxBVPmYSXGOp43594737 = -43855435;    double anYhRIxBVPmYSXGOp85302570 = -605592015;    double anYhRIxBVPmYSXGOp13725792 = -959402327;    double anYhRIxBVPmYSXGOp51078345 = -540197744;    double anYhRIxBVPmYSXGOp63895398 = -301392267;    double anYhRIxBVPmYSXGOp50019239 = -657753873;    double anYhRIxBVPmYSXGOp52392761 = -423163441;    double anYhRIxBVPmYSXGOp61166537 = -757685479;    double anYhRIxBVPmYSXGOp77798223 = -967983754;    double anYhRIxBVPmYSXGOp12982864 = 13077543;    double anYhRIxBVPmYSXGOp17177932 = -380932991;    double anYhRIxBVPmYSXGOp43558365 = -341151029;    double anYhRIxBVPmYSXGOp93560314 = -418478209;    double anYhRIxBVPmYSXGOp60064615 = -574984713;    double anYhRIxBVPmYSXGOp86467842 = -110540240;    double anYhRIxBVPmYSXGOp73962246 = -637157569;    double anYhRIxBVPmYSXGOp73958485 = -364656949;    double anYhRIxBVPmYSXGOp74068324 = -94170005;    double anYhRIxBVPmYSXGOp45665701 = -34088575;    double anYhRIxBVPmYSXGOp32101962 = -517007768;    double anYhRIxBVPmYSXGOp1066882 = -418459370;    double anYhRIxBVPmYSXGOp7400660 = -305431553;    double anYhRIxBVPmYSXGOp17373896 = -845216384;    double anYhRIxBVPmYSXGOp63165223 = -829440003;    double anYhRIxBVPmYSXGOp60321915 = -944829246;    double anYhRIxBVPmYSXGOp5234109 = 50752579;    double anYhRIxBVPmYSXGOp70206128 = -981802129;    double anYhRIxBVPmYSXGOp49105926 = -148264204;    double anYhRIxBVPmYSXGOp75895234 = -602160389;    double anYhRIxBVPmYSXGOp56764791 = 30057783;    double anYhRIxBVPmYSXGOp8966474 = -461423430;    double anYhRIxBVPmYSXGOp18674554 = -191903820;    double anYhRIxBVPmYSXGOp22277359 = -970199652;    double anYhRIxBVPmYSXGOp1982789 = -661991386;    double anYhRIxBVPmYSXGOp40376619 = -282839124;    double anYhRIxBVPmYSXGOp24690497 = -538593562;    double anYhRIxBVPmYSXGOp64499926 = -525495891;    double anYhRIxBVPmYSXGOp81530281 = -145855025;     anYhRIxBVPmYSXGOp86506302 = anYhRIxBVPmYSXGOp99122323;     anYhRIxBVPmYSXGOp99122323 = anYhRIxBVPmYSXGOp4067085;     anYhRIxBVPmYSXGOp4067085 = anYhRIxBVPmYSXGOp38540057;     anYhRIxBVPmYSXGOp38540057 = anYhRIxBVPmYSXGOp9044934;     anYhRIxBVPmYSXGOp9044934 = anYhRIxBVPmYSXGOp51609214;     anYhRIxBVPmYSXGOp51609214 = anYhRIxBVPmYSXGOp56465141;     anYhRIxBVPmYSXGOp56465141 = anYhRIxBVPmYSXGOp56279021;     anYhRIxBVPmYSXGOp56279021 = anYhRIxBVPmYSXGOp24999911;     anYhRIxBVPmYSXGOp24999911 = anYhRIxBVPmYSXGOp59539220;     anYhRIxBVPmYSXGOp59539220 = anYhRIxBVPmYSXGOp63161864;     anYhRIxBVPmYSXGOp63161864 = anYhRIxBVPmYSXGOp90122617;     anYhRIxBVPmYSXGOp90122617 = anYhRIxBVPmYSXGOp3668676;     anYhRIxBVPmYSXGOp3668676 = anYhRIxBVPmYSXGOp91544079;     anYhRIxBVPmYSXGOp91544079 = anYhRIxBVPmYSXGOp83996131;     anYhRIxBVPmYSXGOp83996131 = anYhRIxBVPmYSXGOp32062987;     anYhRIxBVPmYSXGOp32062987 = anYhRIxBVPmYSXGOp44876129;     anYhRIxBVPmYSXGOp44876129 = anYhRIxBVPmYSXGOp54556692;     anYhRIxBVPmYSXGOp54556692 = anYhRIxBVPmYSXGOp62076625;     anYhRIxBVPmYSXGOp62076625 = anYhRIxBVPmYSXGOp21086487;     anYhRIxBVPmYSXGOp21086487 = anYhRIxBVPmYSXGOp44035970;     anYhRIxBVPmYSXGOp44035970 = anYhRIxBVPmYSXGOp96460909;     anYhRIxBVPmYSXGOp96460909 = anYhRIxBVPmYSXGOp52187964;     anYhRIxBVPmYSXGOp52187964 = anYhRIxBVPmYSXGOp6567112;     anYhRIxBVPmYSXGOp6567112 = anYhRIxBVPmYSXGOp15613265;     anYhRIxBVPmYSXGOp15613265 = anYhRIxBVPmYSXGOp90340726;     anYhRIxBVPmYSXGOp90340726 = anYhRIxBVPmYSXGOp55368313;     anYhRIxBVPmYSXGOp55368313 = anYhRIxBVPmYSXGOp30921758;     anYhRIxBVPmYSXGOp30921758 = anYhRIxBVPmYSXGOp12684285;     anYhRIxBVPmYSXGOp12684285 = anYhRIxBVPmYSXGOp39697341;     anYhRIxBVPmYSXGOp39697341 = anYhRIxBVPmYSXGOp45813428;     anYhRIxBVPmYSXGOp45813428 = anYhRIxBVPmYSXGOp12083519;     anYhRIxBVPmYSXGOp12083519 = anYhRIxBVPmYSXGOp26227220;     anYhRIxBVPmYSXGOp26227220 = anYhRIxBVPmYSXGOp53649437;     anYhRIxBVPmYSXGOp53649437 = anYhRIxBVPmYSXGOp39151319;     anYhRIxBVPmYSXGOp39151319 = anYhRIxBVPmYSXGOp22829595;     anYhRIxBVPmYSXGOp22829595 = anYhRIxBVPmYSXGOp54264763;     anYhRIxBVPmYSXGOp54264763 = anYhRIxBVPmYSXGOp31893265;     anYhRIxBVPmYSXGOp31893265 = anYhRIxBVPmYSXGOp37378761;     anYhRIxBVPmYSXGOp37378761 = anYhRIxBVPmYSXGOp18518261;     anYhRIxBVPmYSXGOp18518261 = anYhRIxBVPmYSXGOp27526172;     anYhRIxBVPmYSXGOp27526172 = anYhRIxBVPmYSXGOp83971355;     anYhRIxBVPmYSXGOp83971355 = anYhRIxBVPmYSXGOp9993068;     anYhRIxBVPmYSXGOp9993068 = anYhRIxBVPmYSXGOp78225718;     anYhRIxBVPmYSXGOp78225718 = anYhRIxBVPmYSXGOp32608627;     anYhRIxBVPmYSXGOp32608627 = anYhRIxBVPmYSXGOp41544941;     anYhRIxBVPmYSXGOp41544941 = anYhRIxBVPmYSXGOp44675026;     anYhRIxBVPmYSXGOp44675026 = anYhRIxBVPmYSXGOp23266351;     anYhRIxBVPmYSXGOp23266351 = anYhRIxBVPmYSXGOp29854876;     anYhRIxBVPmYSXGOp29854876 = anYhRIxBVPmYSXGOp5283625;     anYhRIxBVPmYSXGOp5283625 = anYhRIxBVPmYSXGOp22323446;     anYhRIxBVPmYSXGOp22323446 = anYhRIxBVPmYSXGOp82648204;     anYhRIxBVPmYSXGOp82648204 = anYhRIxBVPmYSXGOp51761603;     anYhRIxBVPmYSXGOp51761603 = anYhRIxBVPmYSXGOp20993112;     anYhRIxBVPmYSXGOp20993112 = anYhRIxBVPmYSXGOp83443308;     anYhRIxBVPmYSXGOp83443308 = anYhRIxBVPmYSXGOp90045392;     anYhRIxBVPmYSXGOp90045392 = anYhRIxBVPmYSXGOp46934360;     anYhRIxBVPmYSXGOp46934360 = anYhRIxBVPmYSXGOp97499972;     anYhRIxBVPmYSXGOp97499972 = anYhRIxBVPmYSXGOp22926792;     anYhRIxBVPmYSXGOp22926792 = anYhRIxBVPmYSXGOp18704207;     anYhRIxBVPmYSXGOp18704207 = anYhRIxBVPmYSXGOp96240901;     anYhRIxBVPmYSXGOp96240901 = anYhRIxBVPmYSXGOp25543383;     anYhRIxBVPmYSXGOp25543383 = anYhRIxBVPmYSXGOp43594737;     anYhRIxBVPmYSXGOp43594737 = anYhRIxBVPmYSXGOp85302570;     anYhRIxBVPmYSXGOp85302570 = anYhRIxBVPmYSXGOp13725792;     anYhRIxBVPmYSXGOp13725792 = anYhRIxBVPmYSXGOp51078345;     anYhRIxBVPmYSXGOp51078345 = anYhRIxBVPmYSXGOp63895398;     anYhRIxBVPmYSXGOp63895398 = anYhRIxBVPmYSXGOp50019239;     anYhRIxBVPmYSXGOp50019239 = anYhRIxBVPmYSXGOp52392761;     anYhRIxBVPmYSXGOp52392761 = anYhRIxBVPmYSXGOp61166537;     anYhRIxBVPmYSXGOp61166537 = anYhRIxBVPmYSXGOp77798223;     anYhRIxBVPmYSXGOp77798223 = anYhRIxBVPmYSXGOp12982864;     anYhRIxBVPmYSXGOp12982864 = anYhRIxBVPmYSXGOp17177932;     anYhRIxBVPmYSXGOp17177932 = anYhRIxBVPmYSXGOp43558365;     anYhRIxBVPmYSXGOp43558365 = anYhRIxBVPmYSXGOp93560314;     anYhRIxBVPmYSXGOp93560314 = anYhRIxBVPmYSXGOp60064615;     anYhRIxBVPmYSXGOp60064615 = anYhRIxBVPmYSXGOp86467842;     anYhRIxBVPmYSXGOp86467842 = anYhRIxBVPmYSXGOp73962246;     anYhRIxBVPmYSXGOp73962246 = anYhRIxBVPmYSXGOp73958485;     anYhRIxBVPmYSXGOp73958485 = anYhRIxBVPmYSXGOp74068324;     anYhRIxBVPmYSXGOp74068324 = anYhRIxBVPmYSXGOp45665701;     anYhRIxBVPmYSXGOp45665701 = anYhRIxBVPmYSXGOp32101962;     anYhRIxBVPmYSXGOp32101962 = anYhRIxBVPmYSXGOp1066882;     anYhRIxBVPmYSXGOp1066882 = anYhRIxBVPmYSXGOp7400660;     anYhRIxBVPmYSXGOp7400660 = anYhRIxBVPmYSXGOp17373896;     anYhRIxBVPmYSXGOp17373896 = anYhRIxBVPmYSXGOp63165223;     anYhRIxBVPmYSXGOp63165223 = anYhRIxBVPmYSXGOp60321915;     anYhRIxBVPmYSXGOp60321915 = anYhRIxBVPmYSXGOp5234109;     anYhRIxBVPmYSXGOp5234109 = anYhRIxBVPmYSXGOp70206128;     anYhRIxBVPmYSXGOp70206128 = anYhRIxBVPmYSXGOp49105926;     anYhRIxBVPmYSXGOp49105926 = anYhRIxBVPmYSXGOp75895234;     anYhRIxBVPmYSXGOp75895234 = anYhRIxBVPmYSXGOp56764791;     anYhRIxBVPmYSXGOp56764791 = anYhRIxBVPmYSXGOp8966474;     anYhRIxBVPmYSXGOp8966474 = anYhRIxBVPmYSXGOp18674554;     anYhRIxBVPmYSXGOp18674554 = anYhRIxBVPmYSXGOp22277359;     anYhRIxBVPmYSXGOp22277359 = anYhRIxBVPmYSXGOp1982789;     anYhRIxBVPmYSXGOp1982789 = anYhRIxBVPmYSXGOp40376619;     anYhRIxBVPmYSXGOp40376619 = anYhRIxBVPmYSXGOp24690497;     anYhRIxBVPmYSXGOp24690497 = anYhRIxBVPmYSXGOp64499926;     anYhRIxBVPmYSXGOp64499926 = anYhRIxBVPmYSXGOp81530281;     anYhRIxBVPmYSXGOp81530281 = anYhRIxBVPmYSXGOp86506302;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void iByMkxhNuIFmRHjU42528346() {     double bjEtZPzdCrhJQVBiZ92623409 = -257756914;    double bjEtZPzdCrhJQVBiZ42495603 = -768386127;    double bjEtZPzdCrhJQVBiZ20109268 = -371223432;    double bjEtZPzdCrhJQVBiZ65614837 = -435176904;    double bjEtZPzdCrhJQVBiZ77940421 = -118310456;    double bjEtZPzdCrhJQVBiZ43026416 = -61749838;    double bjEtZPzdCrhJQVBiZ16954107 = -418791979;    double bjEtZPzdCrhJQVBiZ6038912 = -96638286;    double bjEtZPzdCrhJQVBiZ89642406 = -466271900;    double bjEtZPzdCrhJQVBiZ768879 = 28190152;    double bjEtZPzdCrhJQVBiZ23272155 = -838084533;    double bjEtZPzdCrhJQVBiZ21945660 = -641712952;    double bjEtZPzdCrhJQVBiZ89877794 = -80934725;    double bjEtZPzdCrhJQVBiZ74687756 = -994795423;    double bjEtZPzdCrhJQVBiZ79738558 = -887038594;    double bjEtZPzdCrhJQVBiZ73568380 = -267471209;    double bjEtZPzdCrhJQVBiZ38712132 = -247303598;    double bjEtZPzdCrhJQVBiZ51609845 = -267387796;    double bjEtZPzdCrhJQVBiZ23906467 = -658553673;    double bjEtZPzdCrhJQVBiZ1636652 = -800736668;    double bjEtZPzdCrhJQVBiZ32589163 = -957166726;    double bjEtZPzdCrhJQVBiZ82981933 = -262655531;    double bjEtZPzdCrhJQVBiZ87778756 = -968573273;    double bjEtZPzdCrhJQVBiZ90559485 = -86149804;    double bjEtZPzdCrhJQVBiZ59312915 = -435211707;    double bjEtZPzdCrhJQVBiZ15062402 = -647287565;    double bjEtZPzdCrhJQVBiZ53828091 = -856481910;    double bjEtZPzdCrhJQVBiZ44028883 = 17608198;    double bjEtZPzdCrhJQVBiZ78816539 = -988291643;    double bjEtZPzdCrhJQVBiZ8312978 = -910118088;    double bjEtZPzdCrhJQVBiZ38055977 = -696752982;    double bjEtZPzdCrhJQVBiZ9694141 = -29896018;    double bjEtZPzdCrhJQVBiZ35938168 = -788527854;    double bjEtZPzdCrhJQVBiZ48480001 = -834831528;    double bjEtZPzdCrhJQVBiZ625971 = -578126519;    double bjEtZPzdCrhJQVBiZ91555507 = -652917057;    double bjEtZPzdCrhJQVBiZ35526698 = -280123781;    double bjEtZPzdCrhJQVBiZ65983477 = -415342039;    double bjEtZPzdCrhJQVBiZ10989015 = -114855659;    double bjEtZPzdCrhJQVBiZ6020198 = -285072770;    double bjEtZPzdCrhJQVBiZ33195396 = 9370567;    double bjEtZPzdCrhJQVBiZ16121813 = -696836836;    double bjEtZPzdCrhJQVBiZ88060045 = -224990160;    double bjEtZPzdCrhJQVBiZ65633459 = -504241156;    double bjEtZPzdCrhJQVBiZ44561092 = -161086022;    double bjEtZPzdCrhJQVBiZ53484136 = -953061786;    double bjEtZPzdCrhJQVBiZ62453600 = -753319205;    double bjEtZPzdCrhJQVBiZ31223292 = -348707472;    double bjEtZPzdCrhJQVBiZ39530605 = -583740468;    double bjEtZPzdCrhJQVBiZ18195278 = -447488194;    double bjEtZPzdCrhJQVBiZ58024123 = 10275492;    double bjEtZPzdCrhJQVBiZ93457923 = -754718362;    double bjEtZPzdCrhJQVBiZ10061 = -167409545;    double bjEtZPzdCrhJQVBiZ16961992 = -67137209;    double bjEtZPzdCrhJQVBiZ9939183 = -702019417;    double bjEtZPzdCrhJQVBiZ9641476 = -995101383;    double bjEtZPzdCrhJQVBiZ54716847 = -799812855;    double bjEtZPzdCrhJQVBiZ29549783 = -185073629;    double bjEtZPzdCrhJQVBiZ6301923 = -999965197;    double bjEtZPzdCrhJQVBiZ62878020 = -471022891;    double bjEtZPzdCrhJQVBiZ89198325 = -205267928;    double bjEtZPzdCrhJQVBiZ72925223 = -336400177;    double bjEtZPzdCrhJQVBiZ27222372 = -108346643;    double bjEtZPzdCrhJQVBiZ81329429 = -556153813;    double bjEtZPzdCrhJQVBiZ62712902 = -275056867;    double bjEtZPzdCrhJQVBiZ13578014 = -708188515;    double bjEtZPzdCrhJQVBiZ86007492 = -853185098;    double bjEtZPzdCrhJQVBiZ41397793 = -246103197;    double bjEtZPzdCrhJQVBiZ74061785 = -316668904;    double bjEtZPzdCrhJQVBiZ88183050 = -134121538;    double bjEtZPzdCrhJQVBiZ38041682 = -987347428;    double bjEtZPzdCrhJQVBiZ72728654 = -831961560;    double bjEtZPzdCrhJQVBiZ40620831 = -52532137;    double bjEtZPzdCrhJQVBiZ17886269 = -273480903;    double bjEtZPzdCrhJQVBiZ68441255 = -710107235;    double bjEtZPzdCrhJQVBiZ16467351 = -160329890;    double bjEtZPzdCrhJQVBiZ94921887 = 62334628;    double bjEtZPzdCrhJQVBiZ22145298 = -364332118;    double bjEtZPzdCrhJQVBiZ45998394 = -925063782;    double bjEtZPzdCrhJQVBiZ5828779 = -482149922;    double bjEtZPzdCrhJQVBiZ52608802 = -893968361;    double bjEtZPzdCrhJQVBiZ22604799 = -407774438;    double bjEtZPzdCrhJQVBiZ4498279 = -398651335;    double bjEtZPzdCrhJQVBiZ60621262 = -440803450;    double bjEtZPzdCrhJQVBiZ50288854 = -820393580;    double bjEtZPzdCrhJQVBiZ44598054 = -942034621;    double bjEtZPzdCrhJQVBiZ9684080 = -862486474;    double bjEtZPzdCrhJQVBiZ18976177 = -621390646;    double bjEtZPzdCrhJQVBiZ38540818 = -32812112;    double bjEtZPzdCrhJQVBiZ90984495 = -583025137;    double bjEtZPzdCrhJQVBiZ36838660 = -853104203;    double bjEtZPzdCrhJQVBiZ5976916 = 4949847;    double bjEtZPzdCrhJQVBiZ59681555 = -415376842;    double bjEtZPzdCrhJQVBiZ48110994 = -643832769;    double bjEtZPzdCrhJQVBiZ16821873 = 20195157;    double bjEtZPzdCrhJQVBiZ60270172 = -654229257;    double bjEtZPzdCrhJQVBiZ88899440 = -488490193;    double bjEtZPzdCrhJQVBiZ6730617 = -668836347;    double bjEtZPzdCrhJQVBiZ2920557 = -129184290;    double bjEtZPzdCrhJQVBiZ30983078 = -257756914;     bjEtZPzdCrhJQVBiZ92623409 = bjEtZPzdCrhJQVBiZ42495603;     bjEtZPzdCrhJQVBiZ42495603 = bjEtZPzdCrhJQVBiZ20109268;     bjEtZPzdCrhJQVBiZ20109268 = bjEtZPzdCrhJQVBiZ65614837;     bjEtZPzdCrhJQVBiZ65614837 = bjEtZPzdCrhJQVBiZ77940421;     bjEtZPzdCrhJQVBiZ77940421 = bjEtZPzdCrhJQVBiZ43026416;     bjEtZPzdCrhJQVBiZ43026416 = bjEtZPzdCrhJQVBiZ16954107;     bjEtZPzdCrhJQVBiZ16954107 = bjEtZPzdCrhJQVBiZ6038912;     bjEtZPzdCrhJQVBiZ6038912 = bjEtZPzdCrhJQVBiZ89642406;     bjEtZPzdCrhJQVBiZ89642406 = bjEtZPzdCrhJQVBiZ768879;     bjEtZPzdCrhJQVBiZ768879 = bjEtZPzdCrhJQVBiZ23272155;     bjEtZPzdCrhJQVBiZ23272155 = bjEtZPzdCrhJQVBiZ21945660;     bjEtZPzdCrhJQVBiZ21945660 = bjEtZPzdCrhJQVBiZ89877794;     bjEtZPzdCrhJQVBiZ89877794 = bjEtZPzdCrhJQVBiZ74687756;     bjEtZPzdCrhJQVBiZ74687756 = bjEtZPzdCrhJQVBiZ79738558;     bjEtZPzdCrhJQVBiZ79738558 = bjEtZPzdCrhJQVBiZ73568380;     bjEtZPzdCrhJQVBiZ73568380 = bjEtZPzdCrhJQVBiZ38712132;     bjEtZPzdCrhJQVBiZ38712132 = bjEtZPzdCrhJQVBiZ51609845;     bjEtZPzdCrhJQVBiZ51609845 = bjEtZPzdCrhJQVBiZ23906467;     bjEtZPzdCrhJQVBiZ23906467 = bjEtZPzdCrhJQVBiZ1636652;     bjEtZPzdCrhJQVBiZ1636652 = bjEtZPzdCrhJQVBiZ32589163;     bjEtZPzdCrhJQVBiZ32589163 = bjEtZPzdCrhJQVBiZ82981933;     bjEtZPzdCrhJQVBiZ82981933 = bjEtZPzdCrhJQVBiZ87778756;     bjEtZPzdCrhJQVBiZ87778756 = bjEtZPzdCrhJQVBiZ90559485;     bjEtZPzdCrhJQVBiZ90559485 = bjEtZPzdCrhJQVBiZ59312915;     bjEtZPzdCrhJQVBiZ59312915 = bjEtZPzdCrhJQVBiZ15062402;     bjEtZPzdCrhJQVBiZ15062402 = bjEtZPzdCrhJQVBiZ53828091;     bjEtZPzdCrhJQVBiZ53828091 = bjEtZPzdCrhJQVBiZ44028883;     bjEtZPzdCrhJQVBiZ44028883 = bjEtZPzdCrhJQVBiZ78816539;     bjEtZPzdCrhJQVBiZ78816539 = bjEtZPzdCrhJQVBiZ8312978;     bjEtZPzdCrhJQVBiZ8312978 = bjEtZPzdCrhJQVBiZ38055977;     bjEtZPzdCrhJQVBiZ38055977 = bjEtZPzdCrhJQVBiZ9694141;     bjEtZPzdCrhJQVBiZ9694141 = bjEtZPzdCrhJQVBiZ35938168;     bjEtZPzdCrhJQVBiZ35938168 = bjEtZPzdCrhJQVBiZ48480001;     bjEtZPzdCrhJQVBiZ48480001 = bjEtZPzdCrhJQVBiZ625971;     bjEtZPzdCrhJQVBiZ625971 = bjEtZPzdCrhJQVBiZ91555507;     bjEtZPzdCrhJQVBiZ91555507 = bjEtZPzdCrhJQVBiZ35526698;     bjEtZPzdCrhJQVBiZ35526698 = bjEtZPzdCrhJQVBiZ65983477;     bjEtZPzdCrhJQVBiZ65983477 = bjEtZPzdCrhJQVBiZ10989015;     bjEtZPzdCrhJQVBiZ10989015 = bjEtZPzdCrhJQVBiZ6020198;     bjEtZPzdCrhJQVBiZ6020198 = bjEtZPzdCrhJQVBiZ33195396;     bjEtZPzdCrhJQVBiZ33195396 = bjEtZPzdCrhJQVBiZ16121813;     bjEtZPzdCrhJQVBiZ16121813 = bjEtZPzdCrhJQVBiZ88060045;     bjEtZPzdCrhJQVBiZ88060045 = bjEtZPzdCrhJQVBiZ65633459;     bjEtZPzdCrhJQVBiZ65633459 = bjEtZPzdCrhJQVBiZ44561092;     bjEtZPzdCrhJQVBiZ44561092 = bjEtZPzdCrhJQVBiZ53484136;     bjEtZPzdCrhJQVBiZ53484136 = bjEtZPzdCrhJQVBiZ62453600;     bjEtZPzdCrhJQVBiZ62453600 = bjEtZPzdCrhJQVBiZ31223292;     bjEtZPzdCrhJQVBiZ31223292 = bjEtZPzdCrhJQVBiZ39530605;     bjEtZPzdCrhJQVBiZ39530605 = bjEtZPzdCrhJQVBiZ18195278;     bjEtZPzdCrhJQVBiZ18195278 = bjEtZPzdCrhJQVBiZ58024123;     bjEtZPzdCrhJQVBiZ58024123 = bjEtZPzdCrhJQVBiZ93457923;     bjEtZPzdCrhJQVBiZ93457923 = bjEtZPzdCrhJQVBiZ10061;     bjEtZPzdCrhJQVBiZ10061 = bjEtZPzdCrhJQVBiZ16961992;     bjEtZPzdCrhJQVBiZ16961992 = bjEtZPzdCrhJQVBiZ9939183;     bjEtZPzdCrhJQVBiZ9939183 = bjEtZPzdCrhJQVBiZ9641476;     bjEtZPzdCrhJQVBiZ9641476 = bjEtZPzdCrhJQVBiZ54716847;     bjEtZPzdCrhJQVBiZ54716847 = bjEtZPzdCrhJQVBiZ29549783;     bjEtZPzdCrhJQVBiZ29549783 = bjEtZPzdCrhJQVBiZ6301923;     bjEtZPzdCrhJQVBiZ6301923 = bjEtZPzdCrhJQVBiZ62878020;     bjEtZPzdCrhJQVBiZ62878020 = bjEtZPzdCrhJQVBiZ89198325;     bjEtZPzdCrhJQVBiZ89198325 = bjEtZPzdCrhJQVBiZ72925223;     bjEtZPzdCrhJQVBiZ72925223 = bjEtZPzdCrhJQVBiZ27222372;     bjEtZPzdCrhJQVBiZ27222372 = bjEtZPzdCrhJQVBiZ81329429;     bjEtZPzdCrhJQVBiZ81329429 = bjEtZPzdCrhJQVBiZ62712902;     bjEtZPzdCrhJQVBiZ62712902 = bjEtZPzdCrhJQVBiZ13578014;     bjEtZPzdCrhJQVBiZ13578014 = bjEtZPzdCrhJQVBiZ86007492;     bjEtZPzdCrhJQVBiZ86007492 = bjEtZPzdCrhJQVBiZ41397793;     bjEtZPzdCrhJQVBiZ41397793 = bjEtZPzdCrhJQVBiZ74061785;     bjEtZPzdCrhJQVBiZ74061785 = bjEtZPzdCrhJQVBiZ88183050;     bjEtZPzdCrhJQVBiZ88183050 = bjEtZPzdCrhJQVBiZ38041682;     bjEtZPzdCrhJQVBiZ38041682 = bjEtZPzdCrhJQVBiZ72728654;     bjEtZPzdCrhJQVBiZ72728654 = bjEtZPzdCrhJQVBiZ40620831;     bjEtZPzdCrhJQVBiZ40620831 = bjEtZPzdCrhJQVBiZ17886269;     bjEtZPzdCrhJQVBiZ17886269 = bjEtZPzdCrhJQVBiZ68441255;     bjEtZPzdCrhJQVBiZ68441255 = bjEtZPzdCrhJQVBiZ16467351;     bjEtZPzdCrhJQVBiZ16467351 = bjEtZPzdCrhJQVBiZ94921887;     bjEtZPzdCrhJQVBiZ94921887 = bjEtZPzdCrhJQVBiZ22145298;     bjEtZPzdCrhJQVBiZ22145298 = bjEtZPzdCrhJQVBiZ45998394;     bjEtZPzdCrhJQVBiZ45998394 = bjEtZPzdCrhJQVBiZ5828779;     bjEtZPzdCrhJQVBiZ5828779 = bjEtZPzdCrhJQVBiZ52608802;     bjEtZPzdCrhJQVBiZ52608802 = bjEtZPzdCrhJQVBiZ22604799;     bjEtZPzdCrhJQVBiZ22604799 = bjEtZPzdCrhJQVBiZ4498279;     bjEtZPzdCrhJQVBiZ4498279 = bjEtZPzdCrhJQVBiZ60621262;     bjEtZPzdCrhJQVBiZ60621262 = bjEtZPzdCrhJQVBiZ50288854;     bjEtZPzdCrhJQVBiZ50288854 = bjEtZPzdCrhJQVBiZ44598054;     bjEtZPzdCrhJQVBiZ44598054 = bjEtZPzdCrhJQVBiZ9684080;     bjEtZPzdCrhJQVBiZ9684080 = bjEtZPzdCrhJQVBiZ18976177;     bjEtZPzdCrhJQVBiZ18976177 = bjEtZPzdCrhJQVBiZ38540818;     bjEtZPzdCrhJQVBiZ38540818 = bjEtZPzdCrhJQVBiZ90984495;     bjEtZPzdCrhJQVBiZ90984495 = bjEtZPzdCrhJQVBiZ36838660;     bjEtZPzdCrhJQVBiZ36838660 = bjEtZPzdCrhJQVBiZ5976916;     bjEtZPzdCrhJQVBiZ5976916 = bjEtZPzdCrhJQVBiZ59681555;     bjEtZPzdCrhJQVBiZ59681555 = bjEtZPzdCrhJQVBiZ48110994;     bjEtZPzdCrhJQVBiZ48110994 = bjEtZPzdCrhJQVBiZ16821873;     bjEtZPzdCrhJQVBiZ16821873 = bjEtZPzdCrhJQVBiZ60270172;     bjEtZPzdCrhJQVBiZ60270172 = bjEtZPzdCrhJQVBiZ88899440;     bjEtZPzdCrhJQVBiZ88899440 = bjEtZPzdCrhJQVBiZ6730617;     bjEtZPzdCrhJQVBiZ6730617 = bjEtZPzdCrhJQVBiZ2920557;     bjEtZPzdCrhJQVBiZ2920557 = bjEtZPzdCrhJQVBiZ30983078;     bjEtZPzdCrhJQVBiZ30983078 = bjEtZPzdCrhJQVBiZ92623409;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nVAbCkTPbPlTsQtv9819946() {     double mLmsfxYxHIZSHjXAY28082378 = -724033813;    double mLmsfxYxHIZSHjXAY55767682 = -648937734;    double mLmsfxYxHIZSHjXAY57495745 = -36890507;    double mLmsfxYxHIZSHjXAY37838192 = -256327975;    double mLmsfxYxHIZSHjXAY34225691 = -223909301;    double mLmsfxYxHIZSHjXAY79249288 = -324097733;    double mLmsfxYxHIZSHjXAY27611513 = -263517713;    double mLmsfxYxHIZSHjXAY68170472 = -840872427;    double mLmsfxYxHIZSHjXAY91160011 = -954603307;    double mLmsfxYxHIZSHjXAY15671591 = -736496846;    double mLmsfxYxHIZSHjXAY61453889 = -640092262;    double mLmsfxYxHIZSHjXAY22513809 = -505833723;    double mLmsfxYxHIZSHjXAY28112440 = -632947230;    double mLmsfxYxHIZSHjXAY27828620 = -971158832;    double mLmsfxYxHIZSHjXAY37933075 = -290463609;    double mLmsfxYxHIZSHjXAY57109681 = 96717184;    double mLmsfxYxHIZSHjXAY94548067 = -774278151;    double mLmsfxYxHIZSHjXAY51370037 = -340815864;    double mLmsfxYxHIZSHjXAY24456797 = -975117094;    double mLmsfxYxHIZSHjXAY95473869 = -127297733;    double mLmsfxYxHIZSHjXAY45676026 = -614441415;    double mLmsfxYxHIZSHjXAY86403458 = -349291825;    double mLmsfxYxHIZSHjXAY58304136 = -606205457;    double mLmsfxYxHIZSHjXAY84352430 = -596974408;    double mLmsfxYxHIZSHjXAY67330866 = -459196905;    double mLmsfxYxHIZSHjXAY25760573 = -335864372;    double mLmsfxYxHIZSHjXAY14248665 = -662766051;    double mLmsfxYxHIZSHjXAY84757308 = -152163977;    double mLmsfxYxHIZSHjXAY25484755 = -462386446;    double mLmsfxYxHIZSHjXAY13846389 = -944625258;    double mLmsfxYxHIZSHjXAY56639209 = -148733469;    double mLmsfxYxHIZSHjXAY47517159 = -902867433;    double mLmsfxYxHIZSHjXAY48156179 = -936123832;    double mLmsfxYxHIZSHjXAY78228427 = -899525342;    double mLmsfxYxHIZSHjXAY66434424 = -529226770;    double mLmsfxYxHIZSHjXAY43116351 = -867482363;    double mLmsfxYxHIZSHjXAY84938514 = -144819821;    double mLmsfxYxHIZSHjXAY17198881 = -397997620;    double mLmsfxYxHIZSHjXAY41379576 = -865731936;    double mLmsfxYxHIZSHjXAY70291730 = -850335316;    double mLmsfxYxHIZSHjXAY66647003 = -859828348;    double mLmsfxYxHIZSHjXAY5486939 = -291382127;    double mLmsfxYxHIZSHjXAY77443424 = -878170946;    double mLmsfxYxHIZSHjXAY5764797 = -8573566;    double mLmsfxYxHIZSHjXAY44577551 = 12402817;    double mLmsfxYxHIZSHjXAY99445464 = -991985418;    double mLmsfxYxHIZSHjXAY81798763 = -137305566;    double mLmsfxYxHIZSHjXAY48949604 = -515234066;    double mLmsfxYxHIZSHjXAY62423271 = -247477348;    double mLmsfxYxHIZSHjXAY42724867 = -938515010;    double mLmsfxYxHIZSHjXAY38794847 = -964001066;    double mLmsfxYxHIZSHjXAY76943396 = -968370708;    double mLmsfxYxHIZSHjXAY68631209 = 76544901;    double mLmsfxYxHIZSHjXAY32953838 = -258645709;    double mLmsfxYxHIZSHjXAY64143919 = 51490437;    double mLmsfxYxHIZSHjXAY41678920 = -274741989;    double mLmsfxYxHIZSHjXAY97463545 = 57267723;    double mLmsfxYxHIZSHjXAY73143314 = -439916100;    double mLmsfxYxHIZSHjXAY70507326 = -797131070;    double mLmsfxYxHIZSHjXAY8465118 = -888044929;    double mLmsfxYxHIZSHjXAY65000623 = -661331683;    double mLmsfxYxHIZSHjXAY42854205 = -11353737;    double mLmsfxYxHIZSHjXAY42685718 = -278485981;    double mLmsfxYxHIZSHjXAY77313623 = 90021951;    double mLmsfxYxHIZSHjXAY59032382 = -487763377;    double mLmsfxYxHIZSHjXAY13936731 = -737224830;    double mLmsfxYxHIZSHjXAY74357629 = -569709892;    double mLmsfxYxHIZSHjXAY49884012 = -733421888;    double mLmsfxYxHIZSHjXAY61394195 = -341932062;    double mLmsfxYxHIZSHjXAY94816723 = -422981247;    double mLmsfxYxHIZSHjXAY72171166 = -758462995;    double mLmsfxYxHIZSHjXAY77349186 = -276280532;    double mLmsfxYxHIZSHjXAY9990461 = -475083929;    double mLmsfxYxHIZSHjXAY54165066 = -24781779;    double mLmsfxYxHIZSHjXAY28826866 = -267469386;    double mLmsfxYxHIZSHjXAY40189087 = -223059288;    double mLmsfxYxHIZSHjXAY8960035 = -471120880;    double mLmsfxYxHIZSHjXAY52539339 = -497631892;    double mLmsfxYxHIZSHjXAY39774880 = -509377225;    double mLmsfxYxHIZSHjXAY67885401 = -467211488;    double mLmsfxYxHIZSHjXAY43961810 = -98558807;    double mLmsfxYxHIZSHjXAY65299061 = -47531986;    double mLmsfxYxHIZSHjXAY22334037 = -904686629;    double mLmsfxYxHIZSHjXAY82759887 = -523871436;    double mLmsfxYxHIZSHjXAY75051541 = -980624193;    double mLmsfxYxHIZSHjXAY79695812 = -180362761;    double mLmsfxYxHIZSHjXAY78885949 = -879412334;    double mLmsfxYxHIZSHjXAY15202341 = -577478123;    double mLmsfxYxHIZSHjXAY14084508 = -851015779;    double mLmsfxYxHIZSHjXAY24755505 = -154484782;    double mLmsfxYxHIZSHjXAY45652805 = -824750086;    double mLmsfxYxHIZSHjXAY11795201 = -704903722;    double mLmsfxYxHIZSHjXAY46691555 = -600866550;    double mLmsfxYxHIZSHjXAY32914459 = -977687007;    double mLmsfxYxHIZSHjXAY5291107 = -89003634;    double mLmsfxYxHIZSHjXAY23792799 = -748474611;    double mLmsfxYxHIZSHjXAY62801221 = 87103854;    double mLmsfxYxHIZSHjXAY129801 = -868192897;    double mLmsfxYxHIZSHjXAY46732414 = -520810189;    double mLmsfxYxHIZSHjXAY30640820 = -724033813;     mLmsfxYxHIZSHjXAY28082378 = mLmsfxYxHIZSHjXAY55767682;     mLmsfxYxHIZSHjXAY55767682 = mLmsfxYxHIZSHjXAY57495745;     mLmsfxYxHIZSHjXAY57495745 = mLmsfxYxHIZSHjXAY37838192;     mLmsfxYxHIZSHjXAY37838192 = mLmsfxYxHIZSHjXAY34225691;     mLmsfxYxHIZSHjXAY34225691 = mLmsfxYxHIZSHjXAY79249288;     mLmsfxYxHIZSHjXAY79249288 = mLmsfxYxHIZSHjXAY27611513;     mLmsfxYxHIZSHjXAY27611513 = mLmsfxYxHIZSHjXAY68170472;     mLmsfxYxHIZSHjXAY68170472 = mLmsfxYxHIZSHjXAY91160011;     mLmsfxYxHIZSHjXAY91160011 = mLmsfxYxHIZSHjXAY15671591;     mLmsfxYxHIZSHjXAY15671591 = mLmsfxYxHIZSHjXAY61453889;     mLmsfxYxHIZSHjXAY61453889 = mLmsfxYxHIZSHjXAY22513809;     mLmsfxYxHIZSHjXAY22513809 = mLmsfxYxHIZSHjXAY28112440;     mLmsfxYxHIZSHjXAY28112440 = mLmsfxYxHIZSHjXAY27828620;     mLmsfxYxHIZSHjXAY27828620 = mLmsfxYxHIZSHjXAY37933075;     mLmsfxYxHIZSHjXAY37933075 = mLmsfxYxHIZSHjXAY57109681;     mLmsfxYxHIZSHjXAY57109681 = mLmsfxYxHIZSHjXAY94548067;     mLmsfxYxHIZSHjXAY94548067 = mLmsfxYxHIZSHjXAY51370037;     mLmsfxYxHIZSHjXAY51370037 = mLmsfxYxHIZSHjXAY24456797;     mLmsfxYxHIZSHjXAY24456797 = mLmsfxYxHIZSHjXAY95473869;     mLmsfxYxHIZSHjXAY95473869 = mLmsfxYxHIZSHjXAY45676026;     mLmsfxYxHIZSHjXAY45676026 = mLmsfxYxHIZSHjXAY86403458;     mLmsfxYxHIZSHjXAY86403458 = mLmsfxYxHIZSHjXAY58304136;     mLmsfxYxHIZSHjXAY58304136 = mLmsfxYxHIZSHjXAY84352430;     mLmsfxYxHIZSHjXAY84352430 = mLmsfxYxHIZSHjXAY67330866;     mLmsfxYxHIZSHjXAY67330866 = mLmsfxYxHIZSHjXAY25760573;     mLmsfxYxHIZSHjXAY25760573 = mLmsfxYxHIZSHjXAY14248665;     mLmsfxYxHIZSHjXAY14248665 = mLmsfxYxHIZSHjXAY84757308;     mLmsfxYxHIZSHjXAY84757308 = mLmsfxYxHIZSHjXAY25484755;     mLmsfxYxHIZSHjXAY25484755 = mLmsfxYxHIZSHjXAY13846389;     mLmsfxYxHIZSHjXAY13846389 = mLmsfxYxHIZSHjXAY56639209;     mLmsfxYxHIZSHjXAY56639209 = mLmsfxYxHIZSHjXAY47517159;     mLmsfxYxHIZSHjXAY47517159 = mLmsfxYxHIZSHjXAY48156179;     mLmsfxYxHIZSHjXAY48156179 = mLmsfxYxHIZSHjXAY78228427;     mLmsfxYxHIZSHjXAY78228427 = mLmsfxYxHIZSHjXAY66434424;     mLmsfxYxHIZSHjXAY66434424 = mLmsfxYxHIZSHjXAY43116351;     mLmsfxYxHIZSHjXAY43116351 = mLmsfxYxHIZSHjXAY84938514;     mLmsfxYxHIZSHjXAY84938514 = mLmsfxYxHIZSHjXAY17198881;     mLmsfxYxHIZSHjXAY17198881 = mLmsfxYxHIZSHjXAY41379576;     mLmsfxYxHIZSHjXAY41379576 = mLmsfxYxHIZSHjXAY70291730;     mLmsfxYxHIZSHjXAY70291730 = mLmsfxYxHIZSHjXAY66647003;     mLmsfxYxHIZSHjXAY66647003 = mLmsfxYxHIZSHjXAY5486939;     mLmsfxYxHIZSHjXAY5486939 = mLmsfxYxHIZSHjXAY77443424;     mLmsfxYxHIZSHjXAY77443424 = mLmsfxYxHIZSHjXAY5764797;     mLmsfxYxHIZSHjXAY5764797 = mLmsfxYxHIZSHjXAY44577551;     mLmsfxYxHIZSHjXAY44577551 = mLmsfxYxHIZSHjXAY99445464;     mLmsfxYxHIZSHjXAY99445464 = mLmsfxYxHIZSHjXAY81798763;     mLmsfxYxHIZSHjXAY81798763 = mLmsfxYxHIZSHjXAY48949604;     mLmsfxYxHIZSHjXAY48949604 = mLmsfxYxHIZSHjXAY62423271;     mLmsfxYxHIZSHjXAY62423271 = mLmsfxYxHIZSHjXAY42724867;     mLmsfxYxHIZSHjXAY42724867 = mLmsfxYxHIZSHjXAY38794847;     mLmsfxYxHIZSHjXAY38794847 = mLmsfxYxHIZSHjXAY76943396;     mLmsfxYxHIZSHjXAY76943396 = mLmsfxYxHIZSHjXAY68631209;     mLmsfxYxHIZSHjXAY68631209 = mLmsfxYxHIZSHjXAY32953838;     mLmsfxYxHIZSHjXAY32953838 = mLmsfxYxHIZSHjXAY64143919;     mLmsfxYxHIZSHjXAY64143919 = mLmsfxYxHIZSHjXAY41678920;     mLmsfxYxHIZSHjXAY41678920 = mLmsfxYxHIZSHjXAY97463545;     mLmsfxYxHIZSHjXAY97463545 = mLmsfxYxHIZSHjXAY73143314;     mLmsfxYxHIZSHjXAY73143314 = mLmsfxYxHIZSHjXAY70507326;     mLmsfxYxHIZSHjXAY70507326 = mLmsfxYxHIZSHjXAY8465118;     mLmsfxYxHIZSHjXAY8465118 = mLmsfxYxHIZSHjXAY65000623;     mLmsfxYxHIZSHjXAY65000623 = mLmsfxYxHIZSHjXAY42854205;     mLmsfxYxHIZSHjXAY42854205 = mLmsfxYxHIZSHjXAY42685718;     mLmsfxYxHIZSHjXAY42685718 = mLmsfxYxHIZSHjXAY77313623;     mLmsfxYxHIZSHjXAY77313623 = mLmsfxYxHIZSHjXAY59032382;     mLmsfxYxHIZSHjXAY59032382 = mLmsfxYxHIZSHjXAY13936731;     mLmsfxYxHIZSHjXAY13936731 = mLmsfxYxHIZSHjXAY74357629;     mLmsfxYxHIZSHjXAY74357629 = mLmsfxYxHIZSHjXAY49884012;     mLmsfxYxHIZSHjXAY49884012 = mLmsfxYxHIZSHjXAY61394195;     mLmsfxYxHIZSHjXAY61394195 = mLmsfxYxHIZSHjXAY94816723;     mLmsfxYxHIZSHjXAY94816723 = mLmsfxYxHIZSHjXAY72171166;     mLmsfxYxHIZSHjXAY72171166 = mLmsfxYxHIZSHjXAY77349186;     mLmsfxYxHIZSHjXAY77349186 = mLmsfxYxHIZSHjXAY9990461;     mLmsfxYxHIZSHjXAY9990461 = mLmsfxYxHIZSHjXAY54165066;     mLmsfxYxHIZSHjXAY54165066 = mLmsfxYxHIZSHjXAY28826866;     mLmsfxYxHIZSHjXAY28826866 = mLmsfxYxHIZSHjXAY40189087;     mLmsfxYxHIZSHjXAY40189087 = mLmsfxYxHIZSHjXAY8960035;     mLmsfxYxHIZSHjXAY8960035 = mLmsfxYxHIZSHjXAY52539339;     mLmsfxYxHIZSHjXAY52539339 = mLmsfxYxHIZSHjXAY39774880;     mLmsfxYxHIZSHjXAY39774880 = mLmsfxYxHIZSHjXAY67885401;     mLmsfxYxHIZSHjXAY67885401 = mLmsfxYxHIZSHjXAY43961810;     mLmsfxYxHIZSHjXAY43961810 = mLmsfxYxHIZSHjXAY65299061;     mLmsfxYxHIZSHjXAY65299061 = mLmsfxYxHIZSHjXAY22334037;     mLmsfxYxHIZSHjXAY22334037 = mLmsfxYxHIZSHjXAY82759887;     mLmsfxYxHIZSHjXAY82759887 = mLmsfxYxHIZSHjXAY75051541;     mLmsfxYxHIZSHjXAY75051541 = mLmsfxYxHIZSHjXAY79695812;     mLmsfxYxHIZSHjXAY79695812 = mLmsfxYxHIZSHjXAY78885949;     mLmsfxYxHIZSHjXAY78885949 = mLmsfxYxHIZSHjXAY15202341;     mLmsfxYxHIZSHjXAY15202341 = mLmsfxYxHIZSHjXAY14084508;     mLmsfxYxHIZSHjXAY14084508 = mLmsfxYxHIZSHjXAY24755505;     mLmsfxYxHIZSHjXAY24755505 = mLmsfxYxHIZSHjXAY45652805;     mLmsfxYxHIZSHjXAY45652805 = mLmsfxYxHIZSHjXAY11795201;     mLmsfxYxHIZSHjXAY11795201 = mLmsfxYxHIZSHjXAY46691555;     mLmsfxYxHIZSHjXAY46691555 = mLmsfxYxHIZSHjXAY32914459;     mLmsfxYxHIZSHjXAY32914459 = mLmsfxYxHIZSHjXAY5291107;     mLmsfxYxHIZSHjXAY5291107 = mLmsfxYxHIZSHjXAY23792799;     mLmsfxYxHIZSHjXAY23792799 = mLmsfxYxHIZSHjXAY62801221;     mLmsfxYxHIZSHjXAY62801221 = mLmsfxYxHIZSHjXAY129801;     mLmsfxYxHIZSHjXAY129801 = mLmsfxYxHIZSHjXAY46732414;     mLmsfxYxHIZSHjXAY46732414 = mLmsfxYxHIZSHjXAY30640820;     mLmsfxYxHIZSHjXAY30640820 = mLmsfxYxHIZSHjXAY28082378;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void iqgtARKJCjdnJwXO24869014() {     double RBfGizynbXWzDadiJ34199485 = -835935702;    double RBfGizynbXWzDadiJ99140961 = -358537514;    double RBfGizynbXWzDadiJ73537928 = -187723428;    double RBfGizynbXWzDadiJ64912973 = -447076605;    double RBfGizynbXWzDadiJ3121180 = -744559172;    double RBfGizynbXWzDadiJ70666490 = -588593968;    double RBfGizynbXWzDadiJ88100479 = -402482690;    double RBfGizynbXWzDadiJ17930364 = -27746340;    double RBfGizynbXWzDadiJ55802507 = -722635972;    double RBfGizynbXWzDadiJ56901250 = 34527676;    double RBfGizynbXWzDadiJ21564180 = -517529849;    double RBfGizynbXWzDadiJ54336851 = -950239925;    double RBfGizynbXWzDadiJ14321558 = 27161681;    double RBfGizynbXWzDadiJ10972297 = -403397400;    double RBfGizynbXWzDadiJ33675501 = -980925776;    double RBfGizynbXWzDadiJ98615074 = -976282359;    double RBfGizynbXWzDadiJ88384070 = -290823264;    double RBfGizynbXWzDadiJ48423190 = -6338507;    double RBfGizynbXWzDadiJ86286638 = -280886191;    double RBfGizynbXWzDadiJ76024034 = -484200653;    double RBfGizynbXWzDadiJ34229220 = -469928870;    double RBfGizynbXWzDadiJ72924482 = -257221542;    double RBfGizynbXWzDadiJ93894927 = -352722944;    double RBfGizynbXWzDadiJ68344804 = -819279396;    double RBfGizynbXWzDadiJ11030517 = -832392938;    double RBfGizynbXWzDadiJ50482248 = -214519696;    double RBfGizynbXWzDadiJ12708443 = -563428254;    double RBfGizynbXWzDadiJ97864433 = -218092930;    double RBfGizynbXWzDadiJ91617009 = -684769150;    double RBfGizynbXWzDadiJ82462025 = -762096126;    double RBfGizynbXWzDadiJ48881758 = -62054408;    double RBfGizynbXWzDadiJ45127781 = -612314248;    double RBfGizynbXWzDadiJ57867127 = -828737202;    double RBfGizynbXWzDadiJ73058991 = -651067107;    double RBfGizynbXWzDadiJ27909077 = -67959875;    double RBfGizynbXWzDadiJ11842265 = 18491530;    double RBfGizynbXWzDadiJ66200449 = -98455691;    double RBfGizynbXWzDadiJ51289093 = -169503629;    double RBfGizynbXWzDadiJ14989830 = -859655433;    double RBfGizynbXWzDadiJ57793668 = -223774539;    double RBfGizynbXWzDadiJ72316227 = -925102242;    double RBfGizynbXWzDadiJ37637396 = -561524405;    double RBfGizynbXWzDadiJ55510402 = -958975529;    double RBfGizynbXWzDadiJ93172537 = -27916504;    double RBfGizynbXWzDadiJ56530016 = -749495338;    double RBfGizynbXWzDadiJ11384660 = -977201534;    double RBfGizynbXWzDadiJ99577337 = -256081103;    double RBfGizynbXWzDadiJ56906545 = -525129598;    double RBfGizynbXWzDadiJ72099000 = -333214336;    double RBfGizynbXWzDadiJ55636519 = 74474182;    double RBfGizynbXWzDadiJ74495525 = -806294737;    double RBfGizynbXWzDadiJ87753115 = -769097029;    double RBfGizynbXWzDadiJ16879667 = -815244687;    double RBfGizynbXWzDadiJ28922719 = -579115854;    double RBfGizynbXWzDadiJ90639794 = -649041346;    double RBfGizynbXWzDadiJ61275002 = -478714161;    double RBfGizynbXWzDadiJ5246034 = 94185429;    double RBfGizynbXWzDadiJ5193124 = -368444033;    double RBfGizynbXWzDadiJ53882457 = -614683668;    double RBfGizynbXWzDadiJ52638931 = -430039477;    double RBfGizynbXWzDadiJ57958047 = 74834285;    double RBfGizynbXWzDadiJ90236045 = -84389760;    double RBfGizynbXWzDadiJ26313354 = -342977190;    double RBfGizynbXWzDadiJ73340482 = -960539847;    double RBfGizynbXWzDadiJ8019493 = -903417917;    double RBfGizynbXWzDadiJ76436399 = -905215601;    double RBfGizynbXWzDadiJ96469723 = -21502724;    double RBfGizynbXWzDadiJ41262566 = -321771212;    double RBfGizynbXWzDadiJ83063220 = -235437525;    double RBfGizynbXWzDadiJ21833237 = -899417306;    double RBfGizynbXWzDadiJ32414626 = -777826668;    double RBfGizynbXWzDadiJ37094977 = -21319635;    double RBfGizynbXWzDadiJ33433360 = -146683075;    double RBfGizynbXWzDadiJ28492971 = 42888347;    double RBfGizynbXWzDadiJ3707807 = -559098412;    double RBfGizynbXWzDadiJ96591823 = -908404466;    double RBfGizynbXWzDadiJ17414080 = -298246013;    double RBfGizynbXWzDadiJ722391 = -224806441;    double RBfGizynbXWzDadiJ11814789 = 30215941;    double RBfGizynbXWzDadiJ99645856 = -855191404;    double RBfGizynbXWzDadiJ50904911 = -958438593;    double RBfGizynbXWzDadiJ55801898 = 61701344;    double RBfGizynbXWzDadiJ25765434 = -884878595;    double RBfGizynbXWzDadiJ35980491 = -659243333;    double RBfGizynbXWzDadiJ7966500 = -955801389;    double RBfGizynbXWzDadiJ61128643 = -292957379;    double RBfGizynbXWzDadiJ28248114 = -797069562;    double RBfGizynbXWzDadiJ28944409 = -149621349;    double RBfGizynbXWzDadiJ82419197 = 97974238;    double RBfGizynbXWzDadiJ66634074 = -589245715;    double RBfGizynbXWzDadiJ6596231 = 24306100;    double RBfGizynbXWzDadiJ61007325 = -730011659;    double RBfGizynbXWzDadiJ97406636 = -554819962;    double RBfGizynbXWzDadiJ62350899 = -329615956;    double RBfGizynbXWzDadiJ99835620 = -198608825;    double RBfGizynbXWzDadiJ82080181 = -740712482;    double RBfGizynbXWzDadiJ11324043 = -118547216;    double RBfGizynbXWzDadiJ82169920 = -998435683;    double RBfGizynbXWzDadiJ85153045 = -124498587;    double RBfGizynbXWzDadiJ80093617 = -835935702;     RBfGizynbXWzDadiJ34199485 = RBfGizynbXWzDadiJ99140961;     RBfGizynbXWzDadiJ99140961 = RBfGizynbXWzDadiJ73537928;     RBfGizynbXWzDadiJ73537928 = RBfGizynbXWzDadiJ64912973;     RBfGizynbXWzDadiJ64912973 = RBfGizynbXWzDadiJ3121180;     RBfGizynbXWzDadiJ3121180 = RBfGizynbXWzDadiJ70666490;     RBfGizynbXWzDadiJ70666490 = RBfGizynbXWzDadiJ88100479;     RBfGizynbXWzDadiJ88100479 = RBfGizynbXWzDadiJ17930364;     RBfGizynbXWzDadiJ17930364 = RBfGizynbXWzDadiJ55802507;     RBfGizynbXWzDadiJ55802507 = RBfGizynbXWzDadiJ56901250;     RBfGizynbXWzDadiJ56901250 = RBfGizynbXWzDadiJ21564180;     RBfGizynbXWzDadiJ21564180 = RBfGizynbXWzDadiJ54336851;     RBfGizynbXWzDadiJ54336851 = RBfGizynbXWzDadiJ14321558;     RBfGizynbXWzDadiJ14321558 = RBfGizynbXWzDadiJ10972297;     RBfGizynbXWzDadiJ10972297 = RBfGizynbXWzDadiJ33675501;     RBfGizynbXWzDadiJ33675501 = RBfGizynbXWzDadiJ98615074;     RBfGizynbXWzDadiJ98615074 = RBfGizynbXWzDadiJ88384070;     RBfGizynbXWzDadiJ88384070 = RBfGizynbXWzDadiJ48423190;     RBfGizynbXWzDadiJ48423190 = RBfGizynbXWzDadiJ86286638;     RBfGizynbXWzDadiJ86286638 = RBfGizynbXWzDadiJ76024034;     RBfGizynbXWzDadiJ76024034 = RBfGizynbXWzDadiJ34229220;     RBfGizynbXWzDadiJ34229220 = RBfGizynbXWzDadiJ72924482;     RBfGizynbXWzDadiJ72924482 = RBfGizynbXWzDadiJ93894927;     RBfGizynbXWzDadiJ93894927 = RBfGizynbXWzDadiJ68344804;     RBfGizynbXWzDadiJ68344804 = RBfGizynbXWzDadiJ11030517;     RBfGizynbXWzDadiJ11030517 = RBfGizynbXWzDadiJ50482248;     RBfGizynbXWzDadiJ50482248 = RBfGizynbXWzDadiJ12708443;     RBfGizynbXWzDadiJ12708443 = RBfGizynbXWzDadiJ97864433;     RBfGizynbXWzDadiJ97864433 = RBfGizynbXWzDadiJ91617009;     RBfGizynbXWzDadiJ91617009 = RBfGizynbXWzDadiJ82462025;     RBfGizynbXWzDadiJ82462025 = RBfGizynbXWzDadiJ48881758;     RBfGizynbXWzDadiJ48881758 = RBfGizynbXWzDadiJ45127781;     RBfGizynbXWzDadiJ45127781 = RBfGizynbXWzDadiJ57867127;     RBfGizynbXWzDadiJ57867127 = RBfGizynbXWzDadiJ73058991;     RBfGizynbXWzDadiJ73058991 = RBfGizynbXWzDadiJ27909077;     RBfGizynbXWzDadiJ27909077 = RBfGizynbXWzDadiJ11842265;     RBfGizynbXWzDadiJ11842265 = RBfGizynbXWzDadiJ66200449;     RBfGizynbXWzDadiJ66200449 = RBfGizynbXWzDadiJ51289093;     RBfGizynbXWzDadiJ51289093 = RBfGizynbXWzDadiJ14989830;     RBfGizynbXWzDadiJ14989830 = RBfGizynbXWzDadiJ57793668;     RBfGizynbXWzDadiJ57793668 = RBfGizynbXWzDadiJ72316227;     RBfGizynbXWzDadiJ72316227 = RBfGizynbXWzDadiJ37637396;     RBfGizynbXWzDadiJ37637396 = RBfGizynbXWzDadiJ55510402;     RBfGizynbXWzDadiJ55510402 = RBfGizynbXWzDadiJ93172537;     RBfGizynbXWzDadiJ93172537 = RBfGizynbXWzDadiJ56530016;     RBfGizynbXWzDadiJ56530016 = RBfGizynbXWzDadiJ11384660;     RBfGizynbXWzDadiJ11384660 = RBfGizynbXWzDadiJ99577337;     RBfGizynbXWzDadiJ99577337 = RBfGizynbXWzDadiJ56906545;     RBfGizynbXWzDadiJ56906545 = RBfGizynbXWzDadiJ72099000;     RBfGizynbXWzDadiJ72099000 = RBfGizynbXWzDadiJ55636519;     RBfGizynbXWzDadiJ55636519 = RBfGizynbXWzDadiJ74495525;     RBfGizynbXWzDadiJ74495525 = RBfGizynbXWzDadiJ87753115;     RBfGizynbXWzDadiJ87753115 = RBfGizynbXWzDadiJ16879667;     RBfGizynbXWzDadiJ16879667 = RBfGizynbXWzDadiJ28922719;     RBfGizynbXWzDadiJ28922719 = RBfGizynbXWzDadiJ90639794;     RBfGizynbXWzDadiJ90639794 = RBfGizynbXWzDadiJ61275002;     RBfGizynbXWzDadiJ61275002 = RBfGizynbXWzDadiJ5246034;     RBfGizynbXWzDadiJ5246034 = RBfGizynbXWzDadiJ5193124;     RBfGizynbXWzDadiJ5193124 = RBfGizynbXWzDadiJ53882457;     RBfGizynbXWzDadiJ53882457 = RBfGizynbXWzDadiJ52638931;     RBfGizynbXWzDadiJ52638931 = RBfGizynbXWzDadiJ57958047;     RBfGizynbXWzDadiJ57958047 = RBfGizynbXWzDadiJ90236045;     RBfGizynbXWzDadiJ90236045 = RBfGizynbXWzDadiJ26313354;     RBfGizynbXWzDadiJ26313354 = RBfGizynbXWzDadiJ73340482;     RBfGizynbXWzDadiJ73340482 = RBfGizynbXWzDadiJ8019493;     RBfGizynbXWzDadiJ8019493 = RBfGizynbXWzDadiJ76436399;     RBfGizynbXWzDadiJ76436399 = RBfGizynbXWzDadiJ96469723;     RBfGizynbXWzDadiJ96469723 = RBfGizynbXWzDadiJ41262566;     RBfGizynbXWzDadiJ41262566 = RBfGizynbXWzDadiJ83063220;     RBfGizynbXWzDadiJ83063220 = RBfGizynbXWzDadiJ21833237;     RBfGizynbXWzDadiJ21833237 = RBfGizynbXWzDadiJ32414626;     RBfGizynbXWzDadiJ32414626 = RBfGizynbXWzDadiJ37094977;     RBfGizynbXWzDadiJ37094977 = RBfGizynbXWzDadiJ33433360;     RBfGizynbXWzDadiJ33433360 = RBfGizynbXWzDadiJ28492971;     RBfGizynbXWzDadiJ28492971 = RBfGizynbXWzDadiJ3707807;     RBfGizynbXWzDadiJ3707807 = RBfGizynbXWzDadiJ96591823;     RBfGizynbXWzDadiJ96591823 = RBfGizynbXWzDadiJ17414080;     RBfGizynbXWzDadiJ17414080 = RBfGizynbXWzDadiJ722391;     RBfGizynbXWzDadiJ722391 = RBfGizynbXWzDadiJ11814789;     RBfGizynbXWzDadiJ11814789 = RBfGizynbXWzDadiJ99645856;     RBfGizynbXWzDadiJ99645856 = RBfGizynbXWzDadiJ50904911;     RBfGizynbXWzDadiJ50904911 = RBfGizynbXWzDadiJ55801898;     RBfGizynbXWzDadiJ55801898 = RBfGizynbXWzDadiJ25765434;     RBfGizynbXWzDadiJ25765434 = RBfGizynbXWzDadiJ35980491;     RBfGizynbXWzDadiJ35980491 = RBfGizynbXWzDadiJ7966500;     RBfGizynbXWzDadiJ7966500 = RBfGizynbXWzDadiJ61128643;     RBfGizynbXWzDadiJ61128643 = RBfGizynbXWzDadiJ28248114;     RBfGizynbXWzDadiJ28248114 = RBfGizynbXWzDadiJ28944409;     RBfGizynbXWzDadiJ28944409 = RBfGizynbXWzDadiJ82419197;     RBfGizynbXWzDadiJ82419197 = RBfGizynbXWzDadiJ66634074;     RBfGizynbXWzDadiJ66634074 = RBfGizynbXWzDadiJ6596231;     RBfGizynbXWzDadiJ6596231 = RBfGizynbXWzDadiJ61007325;     RBfGizynbXWzDadiJ61007325 = RBfGizynbXWzDadiJ97406636;     RBfGizynbXWzDadiJ97406636 = RBfGizynbXWzDadiJ62350899;     RBfGizynbXWzDadiJ62350899 = RBfGizynbXWzDadiJ99835620;     RBfGizynbXWzDadiJ99835620 = RBfGizynbXWzDadiJ82080181;     RBfGizynbXWzDadiJ82080181 = RBfGizynbXWzDadiJ11324043;     RBfGizynbXWzDadiJ11324043 = RBfGizynbXWzDadiJ82169920;     RBfGizynbXWzDadiJ82169920 = RBfGizynbXWzDadiJ85153045;     RBfGizynbXWzDadiJ85153045 = RBfGizynbXWzDadiJ80093617;     RBfGizynbXWzDadiJ80093617 = RBfGizynbXWzDadiJ34199485;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void pvfThuUnLQiZQYur92160612() {     double KitDnfreWiAqxZsYv69658453 = -202212602;    double KitDnfreWiAqxZsYv12413040 = -239089121;    double KitDnfreWiAqxZsYv10924405 = -953390503;    double KitDnfreWiAqxZsYv37136328 = -268227676;    double KitDnfreWiAqxZsYv59406449 = -850158018;    double KitDnfreWiAqxZsYv6889363 = -850941863;    double KitDnfreWiAqxZsYv98757885 = -247208424;    double KitDnfreWiAqxZsYv80061924 = -771980480;    double KitDnfreWiAqxZsYv57320112 = -110967379;    double KitDnfreWiAqxZsYv71803962 = -730159322;    double KitDnfreWiAqxZsYv59745914 = -319537578;    double KitDnfreWiAqxZsYv54904999 = -814360697;    double KitDnfreWiAqxZsYv52556203 = -524850823;    double KitDnfreWiAqxZsYv64113159 = -379760809;    double KitDnfreWiAqxZsYv91870017 = -384350791;    double KitDnfreWiAqxZsYv82156375 = -612093966;    double KitDnfreWiAqxZsYv44220006 = -817797816;    double KitDnfreWiAqxZsYv48183382 = -79766575;    double KitDnfreWiAqxZsYv86836968 = -597449613;    double KitDnfreWiAqxZsYv69861253 = -910761719;    double KitDnfreWiAqxZsYv47316082 = -127203559;    double KitDnfreWiAqxZsYv76346006 = -343857836;    double KitDnfreWiAqxZsYv64420307 = 9644872;    double KitDnfreWiAqxZsYv62137750 = -230104000;    double KitDnfreWiAqxZsYv19048468 = -856378136;    double KitDnfreWiAqxZsYv61180420 = 96903498;    double KitDnfreWiAqxZsYv73129017 = -369712395;    double KitDnfreWiAqxZsYv38592858 = -387865105;    double KitDnfreWiAqxZsYv38285225 = -158863953;    double KitDnfreWiAqxZsYv87995436 = -796603296;    double KitDnfreWiAqxZsYv67464989 = -614034895;    double KitDnfreWiAqxZsYv82950799 = -385285663;    double KitDnfreWiAqxZsYv70085138 = -976333179;    double KitDnfreWiAqxZsYv2807419 = -715760921;    double KitDnfreWiAqxZsYv93717530 = -19060126;    double KitDnfreWiAqxZsYv63403108 = -196073776;    double KitDnfreWiAqxZsYv15612266 = 36848269;    double KitDnfreWiAqxZsYv2504497 = -152159211;    double KitDnfreWiAqxZsYv45380392 = -510531709;    double KitDnfreWiAqxZsYv22065200 = -789037085;    double KitDnfreWiAqxZsYv5767836 = -694301156;    double KitDnfreWiAqxZsYv27002523 = -156069696;    double KitDnfreWiAqxZsYv44893781 = -512156315;    double KitDnfreWiAqxZsYv33303876 = -632248914;    double KitDnfreWiAqxZsYv56546474 = -576006499;    double KitDnfreWiAqxZsYv57345988 = 83874834;    double KitDnfreWiAqxZsYv18922502 = -740067464;    double KitDnfreWiAqxZsYv74632856 = -691656191;    double KitDnfreWiAqxZsYv94991666 = 3048784;    double KitDnfreWiAqxZsYv80166108 = -416552634;    double KitDnfreWiAqxZsYv55266249 = -680571295;    double KitDnfreWiAqxZsYv71238588 = -982749375;    double KitDnfreWiAqxZsYv85500815 = -571290241;    double KitDnfreWiAqxZsYv44914565 = -770624353;    double KitDnfreWiAqxZsYv44844531 = -995531491;    double KitDnfreWiAqxZsYv93312446 = -858354767;    double KitDnfreWiAqxZsYv47992732 = -148733993;    double KitDnfreWiAqxZsYv48786655 = -623286504;    double KitDnfreWiAqxZsYv18087861 = -411849540;    double KitDnfreWiAqxZsYv98226028 = -847061516;    double KitDnfreWiAqxZsYv33760346 = -381229469;    double KitDnfreWiAqxZsYv60165027 = -859343320;    double KitDnfreWiAqxZsYv41776700 = -513116528;    double KitDnfreWiAqxZsYv69324676 = -314364084;    double KitDnfreWiAqxZsYv4338973 = -16124428;    double KitDnfreWiAqxZsYv76795115 = -934251916;    double KitDnfreWiAqxZsYv84819861 = -838027518;    double KitDnfreWiAqxZsYv49748785 = -809089903;    double KitDnfreWiAqxZsYv70395629 = -260700684;    double KitDnfreWiAqxZsYv28466909 = -88277015;    double KitDnfreWiAqxZsYv66544110 = -548942235;    double KitDnfreWiAqxZsYv41715510 = -565638606;    double KitDnfreWiAqxZsYv2802990 = -569234867;    double KitDnfreWiAqxZsYv64771768 = -808412528;    double KitDnfreWiAqxZsYv64093418 = -116460563;    double KitDnfreWiAqxZsYv20313560 = -971133863;    double KitDnfreWiAqxZsYv31452226 = -831701521;    double KitDnfreWiAqxZsYv31116432 = -358106215;    double KitDnfreWiAqxZsYv5591276 = -654097501;    double KitDnfreWiAqxZsYv61702479 = -840252971;    double KitDnfreWiAqxZsYv42257919 = -163029039;    double KitDnfreWiAqxZsYv98496160 = -678056204;    double KitDnfreWiAqxZsYv43601192 = -290913889;    double KitDnfreWiAqxZsYv58119116 = -742311320;    double KitDnfreWiAqxZsYv32729187 = -16032002;    double KitDnfreWiAqxZsYv96226401 = -631285520;    double KitDnfreWiAqxZsYv97449984 = -813995423;    double KitDnfreWiAqxZsYv25170574 = -105708826;    double KitDnfreWiAqxZsYv57962888 = -720229430;    double KitDnfreWiAqxZsYv405084 = -160705360;    double KitDnfreWiAqxZsYv15410377 = 52660217;    double KitDnfreWiAqxZsYv66825610 = -339865228;    double KitDnfreWiAqxZsYv84416636 = -740309671;    double KitDnfreWiAqxZsYv47154363 = -663470194;    double KitDnfreWiAqxZsYv88304854 = -307807616;    double KitDnfreWiAqxZsYv45602808 = -834957837;    double KitDnfreWiAqxZsYv85225822 = -642953169;    double KitDnfreWiAqxZsYv75569104 = -97792232;    double KitDnfreWiAqxZsYv28964903 = -516124486;    double KitDnfreWiAqxZsYv79751359 = -202212602;     KitDnfreWiAqxZsYv69658453 = KitDnfreWiAqxZsYv12413040;     KitDnfreWiAqxZsYv12413040 = KitDnfreWiAqxZsYv10924405;     KitDnfreWiAqxZsYv10924405 = KitDnfreWiAqxZsYv37136328;     KitDnfreWiAqxZsYv37136328 = KitDnfreWiAqxZsYv59406449;     KitDnfreWiAqxZsYv59406449 = KitDnfreWiAqxZsYv6889363;     KitDnfreWiAqxZsYv6889363 = KitDnfreWiAqxZsYv98757885;     KitDnfreWiAqxZsYv98757885 = KitDnfreWiAqxZsYv80061924;     KitDnfreWiAqxZsYv80061924 = KitDnfreWiAqxZsYv57320112;     KitDnfreWiAqxZsYv57320112 = KitDnfreWiAqxZsYv71803962;     KitDnfreWiAqxZsYv71803962 = KitDnfreWiAqxZsYv59745914;     KitDnfreWiAqxZsYv59745914 = KitDnfreWiAqxZsYv54904999;     KitDnfreWiAqxZsYv54904999 = KitDnfreWiAqxZsYv52556203;     KitDnfreWiAqxZsYv52556203 = KitDnfreWiAqxZsYv64113159;     KitDnfreWiAqxZsYv64113159 = KitDnfreWiAqxZsYv91870017;     KitDnfreWiAqxZsYv91870017 = KitDnfreWiAqxZsYv82156375;     KitDnfreWiAqxZsYv82156375 = KitDnfreWiAqxZsYv44220006;     KitDnfreWiAqxZsYv44220006 = KitDnfreWiAqxZsYv48183382;     KitDnfreWiAqxZsYv48183382 = KitDnfreWiAqxZsYv86836968;     KitDnfreWiAqxZsYv86836968 = KitDnfreWiAqxZsYv69861253;     KitDnfreWiAqxZsYv69861253 = KitDnfreWiAqxZsYv47316082;     KitDnfreWiAqxZsYv47316082 = KitDnfreWiAqxZsYv76346006;     KitDnfreWiAqxZsYv76346006 = KitDnfreWiAqxZsYv64420307;     KitDnfreWiAqxZsYv64420307 = KitDnfreWiAqxZsYv62137750;     KitDnfreWiAqxZsYv62137750 = KitDnfreWiAqxZsYv19048468;     KitDnfreWiAqxZsYv19048468 = KitDnfreWiAqxZsYv61180420;     KitDnfreWiAqxZsYv61180420 = KitDnfreWiAqxZsYv73129017;     KitDnfreWiAqxZsYv73129017 = KitDnfreWiAqxZsYv38592858;     KitDnfreWiAqxZsYv38592858 = KitDnfreWiAqxZsYv38285225;     KitDnfreWiAqxZsYv38285225 = KitDnfreWiAqxZsYv87995436;     KitDnfreWiAqxZsYv87995436 = KitDnfreWiAqxZsYv67464989;     KitDnfreWiAqxZsYv67464989 = KitDnfreWiAqxZsYv82950799;     KitDnfreWiAqxZsYv82950799 = KitDnfreWiAqxZsYv70085138;     KitDnfreWiAqxZsYv70085138 = KitDnfreWiAqxZsYv2807419;     KitDnfreWiAqxZsYv2807419 = KitDnfreWiAqxZsYv93717530;     KitDnfreWiAqxZsYv93717530 = KitDnfreWiAqxZsYv63403108;     KitDnfreWiAqxZsYv63403108 = KitDnfreWiAqxZsYv15612266;     KitDnfreWiAqxZsYv15612266 = KitDnfreWiAqxZsYv2504497;     KitDnfreWiAqxZsYv2504497 = KitDnfreWiAqxZsYv45380392;     KitDnfreWiAqxZsYv45380392 = KitDnfreWiAqxZsYv22065200;     KitDnfreWiAqxZsYv22065200 = KitDnfreWiAqxZsYv5767836;     KitDnfreWiAqxZsYv5767836 = KitDnfreWiAqxZsYv27002523;     KitDnfreWiAqxZsYv27002523 = KitDnfreWiAqxZsYv44893781;     KitDnfreWiAqxZsYv44893781 = KitDnfreWiAqxZsYv33303876;     KitDnfreWiAqxZsYv33303876 = KitDnfreWiAqxZsYv56546474;     KitDnfreWiAqxZsYv56546474 = KitDnfreWiAqxZsYv57345988;     KitDnfreWiAqxZsYv57345988 = KitDnfreWiAqxZsYv18922502;     KitDnfreWiAqxZsYv18922502 = KitDnfreWiAqxZsYv74632856;     KitDnfreWiAqxZsYv74632856 = KitDnfreWiAqxZsYv94991666;     KitDnfreWiAqxZsYv94991666 = KitDnfreWiAqxZsYv80166108;     KitDnfreWiAqxZsYv80166108 = KitDnfreWiAqxZsYv55266249;     KitDnfreWiAqxZsYv55266249 = KitDnfreWiAqxZsYv71238588;     KitDnfreWiAqxZsYv71238588 = KitDnfreWiAqxZsYv85500815;     KitDnfreWiAqxZsYv85500815 = KitDnfreWiAqxZsYv44914565;     KitDnfreWiAqxZsYv44914565 = KitDnfreWiAqxZsYv44844531;     KitDnfreWiAqxZsYv44844531 = KitDnfreWiAqxZsYv93312446;     KitDnfreWiAqxZsYv93312446 = KitDnfreWiAqxZsYv47992732;     KitDnfreWiAqxZsYv47992732 = KitDnfreWiAqxZsYv48786655;     KitDnfreWiAqxZsYv48786655 = KitDnfreWiAqxZsYv18087861;     KitDnfreWiAqxZsYv18087861 = KitDnfreWiAqxZsYv98226028;     KitDnfreWiAqxZsYv98226028 = KitDnfreWiAqxZsYv33760346;     KitDnfreWiAqxZsYv33760346 = KitDnfreWiAqxZsYv60165027;     KitDnfreWiAqxZsYv60165027 = KitDnfreWiAqxZsYv41776700;     KitDnfreWiAqxZsYv41776700 = KitDnfreWiAqxZsYv69324676;     KitDnfreWiAqxZsYv69324676 = KitDnfreWiAqxZsYv4338973;     KitDnfreWiAqxZsYv4338973 = KitDnfreWiAqxZsYv76795115;     KitDnfreWiAqxZsYv76795115 = KitDnfreWiAqxZsYv84819861;     KitDnfreWiAqxZsYv84819861 = KitDnfreWiAqxZsYv49748785;     KitDnfreWiAqxZsYv49748785 = KitDnfreWiAqxZsYv70395629;     KitDnfreWiAqxZsYv70395629 = KitDnfreWiAqxZsYv28466909;     KitDnfreWiAqxZsYv28466909 = KitDnfreWiAqxZsYv66544110;     KitDnfreWiAqxZsYv66544110 = KitDnfreWiAqxZsYv41715510;     KitDnfreWiAqxZsYv41715510 = KitDnfreWiAqxZsYv2802990;     KitDnfreWiAqxZsYv2802990 = KitDnfreWiAqxZsYv64771768;     KitDnfreWiAqxZsYv64771768 = KitDnfreWiAqxZsYv64093418;     KitDnfreWiAqxZsYv64093418 = KitDnfreWiAqxZsYv20313560;     KitDnfreWiAqxZsYv20313560 = KitDnfreWiAqxZsYv31452226;     KitDnfreWiAqxZsYv31452226 = KitDnfreWiAqxZsYv31116432;     KitDnfreWiAqxZsYv31116432 = KitDnfreWiAqxZsYv5591276;     KitDnfreWiAqxZsYv5591276 = KitDnfreWiAqxZsYv61702479;     KitDnfreWiAqxZsYv61702479 = KitDnfreWiAqxZsYv42257919;     KitDnfreWiAqxZsYv42257919 = KitDnfreWiAqxZsYv98496160;     KitDnfreWiAqxZsYv98496160 = KitDnfreWiAqxZsYv43601192;     KitDnfreWiAqxZsYv43601192 = KitDnfreWiAqxZsYv58119116;     KitDnfreWiAqxZsYv58119116 = KitDnfreWiAqxZsYv32729187;     KitDnfreWiAqxZsYv32729187 = KitDnfreWiAqxZsYv96226401;     KitDnfreWiAqxZsYv96226401 = KitDnfreWiAqxZsYv97449984;     KitDnfreWiAqxZsYv97449984 = KitDnfreWiAqxZsYv25170574;     KitDnfreWiAqxZsYv25170574 = KitDnfreWiAqxZsYv57962888;     KitDnfreWiAqxZsYv57962888 = KitDnfreWiAqxZsYv405084;     KitDnfreWiAqxZsYv405084 = KitDnfreWiAqxZsYv15410377;     KitDnfreWiAqxZsYv15410377 = KitDnfreWiAqxZsYv66825610;     KitDnfreWiAqxZsYv66825610 = KitDnfreWiAqxZsYv84416636;     KitDnfreWiAqxZsYv84416636 = KitDnfreWiAqxZsYv47154363;     KitDnfreWiAqxZsYv47154363 = KitDnfreWiAqxZsYv88304854;     KitDnfreWiAqxZsYv88304854 = KitDnfreWiAqxZsYv45602808;     KitDnfreWiAqxZsYv45602808 = KitDnfreWiAqxZsYv85225822;     KitDnfreWiAqxZsYv85225822 = KitDnfreWiAqxZsYv75569104;     KitDnfreWiAqxZsYv75569104 = KitDnfreWiAqxZsYv28964903;     KitDnfreWiAqxZsYv28964903 = KitDnfreWiAqxZsYv79751359;     KitDnfreWiAqxZsYv79751359 = KitDnfreWiAqxZsYv69658453;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void BDXusJvAztzAgTBU7209681() {     double XWARNhEjecggDCjVn75775559 = -314114491;    double XWARNhEjecggDCjVn55786319 = 51311098;    double XWARNhEjecggDCjVn26966589 = -4223425;    double XWARNhEjecggDCjVn64211109 = -458976307;    double XWARNhEjecggDCjVn28301937 = -270807889;    double XWARNhEjecggDCjVn98306564 = -15438098;    double XWARNhEjecggDCjVn59246851 = -386173402;    double XWARNhEjecggDCjVn29821816 = 41145607;    double XWARNhEjecggDCjVn21962608 = -979000045;    double XWARNhEjecggDCjVn13033622 = 40865201;    double XWARNhEjecggDCjVn19856205 = -196975165;    double XWARNhEjecggDCjVn86728041 = -158766899;    double XWARNhEjecggDCjVn38765322 = -964741912;    double XWARNhEjecggDCjVn47256836 = -911999377;    double XWARNhEjecggDCjVn87612444 = 25187043;    double XWARNhEjecggDCjVn23661770 = -585093509;    double XWARNhEjecggDCjVn38056010 = -334342929;    double XWARNhEjecggDCjVn45236535 = -845289218;    double XWARNhEjecggDCjVn48666810 = 96781290;    double XWARNhEjecggDCjVn50411418 = -167664638;    double XWARNhEjecggDCjVn35869276 = 17308986;    double XWARNhEjecggDCjVn62867030 = -251787552;    double XWARNhEjecggDCjVn11100 = -836872615;    double XWARNhEjecggDCjVn46130124 = -452408988;    double XWARNhEjecggDCjVn62748117 = -129574169;    double XWARNhEjecggDCjVn85902095 = -881751826;    double XWARNhEjecggDCjVn71588795 = -270374598;    double XWARNhEjecggDCjVn51699983 = -453794058;    double XWARNhEjecggDCjVn4417480 = -381246657;    double XWARNhEjecggDCjVn56611073 = -614074164;    double XWARNhEjecggDCjVn59707539 = -527355833;    double XWARNhEjecggDCjVn80561421 = -94732479;    double XWARNhEjecggDCjVn79796087 = -868946549;    double XWARNhEjecggDCjVn97637982 = -467302686;    double XWARNhEjecggDCjVn55192182 = -657793231;    double XWARNhEjecggDCjVn32129022 = -410099884;    double XWARNhEjecggDCjVn96874200 = 83212399;    double XWARNhEjecggDCjVn36594709 = 76334780;    double XWARNhEjecggDCjVn18990646 = -504455206;    double XWARNhEjecggDCjVn9567138 = -162476308;    double XWARNhEjecggDCjVn11437060 = -759575050;    double XWARNhEjecggDCjVn59152980 = -426211974;    double XWARNhEjecggDCjVn22960759 = -592960899;    double XWARNhEjecggDCjVn20711616 = -651591852;    double XWARNhEjecggDCjVn68498939 = -237904653;    double XWARNhEjecggDCjVn69285183 = 98658717;    double XWARNhEjecggDCjVn36701076 = -858843001;    double XWARNhEjecggDCjVn82589797 = -701551724;    double XWARNhEjecggDCjVn4667395 = -82688204;    double XWARNhEjecggDCjVn93077761 = -503563442;    double XWARNhEjecggDCjVn90966927 = -522864966;    double XWARNhEjecggDCjVn82048307 = -783475695;    double XWARNhEjecggDCjVn33749273 = -363079828;    double XWARNhEjecggDCjVn40883445 = 8905501;    double XWARNhEjecggDCjVn71340405 = -596063274;    double XWARNhEjecggDCjVn12908530 = 37673061;    double XWARNhEjecggDCjVn55775220 = -111816287;    double XWARNhEjecggDCjVn80836465 = -551814437;    double XWARNhEjecggDCjVn1462992 = -229402138;    double XWARNhEjecggDCjVn42399842 = -389056064;    double XWARNhEjecggDCjVn26717770 = -745063501;    double XWARNhEjecggDCjVn7546868 = -932379344;    double XWARNhEjecggDCjVn25404336 = -577607736;    double XWARNhEjecggDCjVn65351535 = -264925882;    double XWARNhEjecggDCjVn53326083 = -431778967;    double XWARNhEjecggDCjVn39294784 = -2242687;    double XWARNhEjecggDCjVn6931955 = -289820350;    double XWARNhEjecggDCjVn41127340 = -397439227;    double XWARNhEjecggDCjVn92064654 = -154206146;    double XWARNhEjecggDCjVn55483423 = -564713074;    double XWARNhEjecggDCjVn26787570 = -568305909;    double XWARNhEjecggDCjVn1461301 = -310677710;    double XWARNhEjecggDCjVn26245889 = -240834013;    double XWARNhEjecggDCjVn39099672 = -740742402;    double XWARNhEjecggDCjVn38974359 = -408089588;    double XWARNhEjecggDCjVn76716296 = -556479041;    double XWARNhEjecggDCjVn39906271 = -658826653;    double XWARNhEjecggDCjVn79299483 = -85280763;    double XWARNhEjecggDCjVn77631184 = -114504335;    double XWARNhEjecggDCjVn93462934 = -128232887;    double XWARNhEjecggDCjVn49201020 = 77091175;    double XWARNhEjecggDCjVn88998997 = -568822874;    double XWARNhEjecggDCjVn47032589 = -271105855;    double XWARNhEjecggDCjVn11339719 = -877683216;    double XWARNhEjecggDCjVn65644145 = 8790802;    double XWARNhEjecggDCjVn77659231 = -743880138;    double XWARNhEjecggDCjVn46812149 = -731652651;    double XWARNhEjecggDCjVn38912642 = -777852051;    double XWARNhEjecggDCjVn26297577 = -871239413;    double XWARNhEjecggDCjVn42283653 = -595466293;    double XWARNhEjecggDCjVn76353802 = -198283597;    double XWARNhEjecggDCjVn16037736 = -364973164;    double XWARNhEjecggDCjVn35131718 = -694263083;    double XWARNhEjecggDCjVn76590803 = -15399142;    double XWARNhEjecggDCjVn82849368 = -417412807;    double XWARNhEjecggDCjVn3890192 = -827195707;    double XWARNhEjecggDCjVn33748644 = -848604238;    double XWARNhEjecggDCjVn57609224 = -228035018;    double XWARNhEjecggDCjVn67385533 = -119812885;    double XWARNhEjecggDCjVn29204156 = -314114491;     XWARNhEjecggDCjVn75775559 = XWARNhEjecggDCjVn55786319;     XWARNhEjecggDCjVn55786319 = XWARNhEjecggDCjVn26966589;     XWARNhEjecggDCjVn26966589 = XWARNhEjecggDCjVn64211109;     XWARNhEjecggDCjVn64211109 = XWARNhEjecggDCjVn28301937;     XWARNhEjecggDCjVn28301937 = XWARNhEjecggDCjVn98306564;     XWARNhEjecggDCjVn98306564 = XWARNhEjecggDCjVn59246851;     XWARNhEjecggDCjVn59246851 = XWARNhEjecggDCjVn29821816;     XWARNhEjecggDCjVn29821816 = XWARNhEjecggDCjVn21962608;     XWARNhEjecggDCjVn21962608 = XWARNhEjecggDCjVn13033622;     XWARNhEjecggDCjVn13033622 = XWARNhEjecggDCjVn19856205;     XWARNhEjecggDCjVn19856205 = XWARNhEjecggDCjVn86728041;     XWARNhEjecggDCjVn86728041 = XWARNhEjecggDCjVn38765322;     XWARNhEjecggDCjVn38765322 = XWARNhEjecggDCjVn47256836;     XWARNhEjecggDCjVn47256836 = XWARNhEjecggDCjVn87612444;     XWARNhEjecggDCjVn87612444 = XWARNhEjecggDCjVn23661770;     XWARNhEjecggDCjVn23661770 = XWARNhEjecggDCjVn38056010;     XWARNhEjecggDCjVn38056010 = XWARNhEjecggDCjVn45236535;     XWARNhEjecggDCjVn45236535 = XWARNhEjecggDCjVn48666810;     XWARNhEjecggDCjVn48666810 = XWARNhEjecggDCjVn50411418;     XWARNhEjecggDCjVn50411418 = XWARNhEjecggDCjVn35869276;     XWARNhEjecggDCjVn35869276 = XWARNhEjecggDCjVn62867030;     XWARNhEjecggDCjVn62867030 = XWARNhEjecggDCjVn11100;     XWARNhEjecggDCjVn11100 = XWARNhEjecggDCjVn46130124;     XWARNhEjecggDCjVn46130124 = XWARNhEjecggDCjVn62748117;     XWARNhEjecggDCjVn62748117 = XWARNhEjecggDCjVn85902095;     XWARNhEjecggDCjVn85902095 = XWARNhEjecggDCjVn71588795;     XWARNhEjecggDCjVn71588795 = XWARNhEjecggDCjVn51699983;     XWARNhEjecggDCjVn51699983 = XWARNhEjecggDCjVn4417480;     XWARNhEjecggDCjVn4417480 = XWARNhEjecggDCjVn56611073;     XWARNhEjecggDCjVn56611073 = XWARNhEjecggDCjVn59707539;     XWARNhEjecggDCjVn59707539 = XWARNhEjecggDCjVn80561421;     XWARNhEjecggDCjVn80561421 = XWARNhEjecggDCjVn79796087;     XWARNhEjecggDCjVn79796087 = XWARNhEjecggDCjVn97637982;     XWARNhEjecggDCjVn97637982 = XWARNhEjecggDCjVn55192182;     XWARNhEjecggDCjVn55192182 = XWARNhEjecggDCjVn32129022;     XWARNhEjecggDCjVn32129022 = XWARNhEjecggDCjVn96874200;     XWARNhEjecggDCjVn96874200 = XWARNhEjecggDCjVn36594709;     XWARNhEjecggDCjVn36594709 = XWARNhEjecggDCjVn18990646;     XWARNhEjecggDCjVn18990646 = XWARNhEjecggDCjVn9567138;     XWARNhEjecggDCjVn9567138 = XWARNhEjecggDCjVn11437060;     XWARNhEjecggDCjVn11437060 = XWARNhEjecggDCjVn59152980;     XWARNhEjecggDCjVn59152980 = XWARNhEjecggDCjVn22960759;     XWARNhEjecggDCjVn22960759 = XWARNhEjecggDCjVn20711616;     XWARNhEjecggDCjVn20711616 = XWARNhEjecggDCjVn68498939;     XWARNhEjecggDCjVn68498939 = XWARNhEjecggDCjVn69285183;     XWARNhEjecggDCjVn69285183 = XWARNhEjecggDCjVn36701076;     XWARNhEjecggDCjVn36701076 = XWARNhEjecggDCjVn82589797;     XWARNhEjecggDCjVn82589797 = XWARNhEjecggDCjVn4667395;     XWARNhEjecggDCjVn4667395 = XWARNhEjecggDCjVn93077761;     XWARNhEjecggDCjVn93077761 = XWARNhEjecggDCjVn90966927;     XWARNhEjecggDCjVn90966927 = XWARNhEjecggDCjVn82048307;     XWARNhEjecggDCjVn82048307 = XWARNhEjecggDCjVn33749273;     XWARNhEjecggDCjVn33749273 = XWARNhEjecggDCjVn40883445;     XWARNhEjecggDCjVn40883445 = XWARNhEjecggDCjVn71340405;     XWARNhEjecggDCjVn71340405 = XWARNhEjecggDCjVn12908530;     XWARNhEjecggDCjVn12908530 = XWARNhEjecggDCjVn55775220;     XWARNhEjecggDCjVn55775220 = XWARNhEjecggDCjVn80836465;     XWARNhEjecggDCjVn80836465 = XWARNhEjecggDCjVn1462992;     XWARNhEjecggDCjVn1462992 = XWARNhEjecggDCjVn42399842;     XWARNhEjecggDCjVn42399842 = XWARNhEjecggDCjVn26717770;     XWARNhEjecggDCjVn26717770 = XWARNhEjecggDCjVn7546868;     XWARNhEjecggDCjVn7546868 = XWARNhEjecggDCjVn25404336;     XWARNhEjecggDCjVn25404336 = XWARNhEjecggDCjVn65351535;     XWARNhEjecggDCjVn65351535 = XWARNhEjecggDCjVn53326083;     XWARNhEjecggDCjVn53326083 = XWARNhEjecggDCjVn39294784;     XWARNhEjecggDCjVn39294784 = XWARNhEjecggDCjVn6931955;     XWARNhEjecggDCjVn6931955 = XWARNhEjecggDCjVn41127340;     XWARNhEjecggDCjVn41127340 = XWARNhEjecggDCjVn92064654;     XWARNhEjecggDCjVn92064654 = XWARNhEjecggDCjVn55483423;     XWARNhEjecggDCjVn55483423 = XWARNhEjecggDCjVn26787570;     XWARNhEjecggDCjVn26787570 = XWARNhEjecggDCjVn1461301;     XWARNhEjecggDCjVn1461301 = XWARNhEjecggDCjVn26245889;     XWARNhEjecggDCjVn26245889 = XWARNhEjecggDCjVn39099672;     XWARNhEjecggDCjVn39099672 = XWARNhEjecggDCjVn38974359;     XWARNhEjecggDCjVn38974359 = XWARNhEjecggDCjVn76716296;     XWARNhEjecggDCjVn76716296 = XWARNhEjecggDCjVn39906271;     XWARNhEjecggDCjVn39906271 = XWARNhEjecggDCjVn79299483;     XWARNhEjecggDCjVn79299483 = XWARNhEjecggDCjVn77631184;     XWARNhEjecggDCjVn77631184 = XWARNhEjecggDCjVn93462934;     XWARNhEjecggDCjVn93462934 = XWARNhEjecggDCjVn49201020;     XWARNhEjecggDCjVn49201020 = XWARNhEjecggDCjVn88998997;     XWARNhEjecggDCjVn88998997 = XWARNhEjecggDCjVn47032589;     XWARNhEjecggDCjVn47032589 = XWARNhEjecggDCjVn11339719;     XWARNhEjecggDCjVn11339719 = XWARNhEjecggDCjVn65644145;     XWARNhEjecggDCjVn65644145 = XWARNhEjecggDCjVn77659231;     XWARNhEjecggDCjVn77659231 = XWARNhEjecggDCjVn46812149;     XWARNhEjecggDCjVn46812149 = XWARNhEjecggDCjVn38912642;     XWARNhEjecggDCjVn38912642 = XWARNhEjecggDCjVn26297577;     XWARNhEjecggDCjVn26297577 = XWARNhEjecggDCjVn42283653;     XWARNhEjecggDCjVn42283653 = XWARNhEjecggDCjVn76353802;     XWARNhEjecggDCjVn76353802 = XWARNhEjecggDCjVn16037736;     XWARNhEjecggDCjVn16037736 = XWARNhEjecggDCjVn35131718;     XWARNhEjecggDCjVn35131718 = XWARNhEjecggDCjVn76590803;     XWARNhEjecggDCjVn76590803 = XWARNhEjecggDCjVn82849368;     XWARNhEjecggDCjVn82849368 = XWARNhEjecggDCjVn3890192;     XWARNhEjecggDCjVn3890192 = XWARNhEjecggDCjVn33748644;     XWARNhEjecggDCjVn33748644 = XWARNhEjecggDCjVn57609224;     XWARNhEjecggDCjVn57609224 = XWARNhEjecggDCjVn67385533;     XWARNhEjecggDCjVn67385533 = XWARNhEjecggDCjVn29204156;     XWARNhEjecggDCjVn29204156 = XWARNhEjecggDCjVn75775559;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IVxPIGgNJPVVUwXL74501280() {     double vCmcCSQQGJhVqwwmF11234528 = -780391391;    double vCmcCSQQGJhVqwwmF69058397 = -929240508;    double vCmcCSQQGJhVqwwmF64353065 = -769890500;    double vCmcCSQQGJhVqwwmF36434464 = -280127378;    double vCmcCSQQGJhVqwwmF84587206 = -376406734;    double vCmcCSQQGJhVqwwmF34529437 = -277785994;    double vCmcCSQQGJhVqwwmF69904257 = -230899136;    double vCmcCSQQGJhVqwwmF91953376 = -703088534;    double vCmcCSQQGJhVqwwmF23480213 = -367331452;    double vCmcCSQQGJhVqwwmF27936334 = -723821797;    double vCmcCSQQGJhVqwwmF58037940 = 1017106;    double vCmcCSQQGJhVqwwmF87296190 = -22887670;    double vCmcCSQQGJhVqwwmF76999967 = -416754417;    double vCmcCSQQGJhVqwwmF397700 = -888362786;    double vCmcCSQQGJhVqwwmF45806961 = -478237972;    double vCmcCSQQGJhVqwwmF7203071 = -220905116;    double vCmcCSQQGJhVqwwmF93891945 = -861317482;    double vCmcCSQQGJhVqwwmF44996726 = -918717287;    double vCmcCSQQGJhVqwwmF49217140 = -219782131;    double vCmcCSQQGJhVqwwmF44248636 = -594225704;    double vCmcCSQQGJhVqwwmF48956139 = -739965703;    double vCmcCSQQGJhVqwwmF66288555 = -338423846;    double vCmcCSQQGJhVqwwmF70536479 = -474504799;    double vCmcCSQQGJhVqwwmF39923069 = -963233592;    double vCmcCSQQGJhVqwwmF70766069 = -153559368;    double vCmcCSQQGJhVqwwmF96600267 = -570328632;    double vCmcCSQQGJhVqwwmF32009369 = -76658739;    double vCmcCSQQGJhVqwwmF92428408 = -623566233;    double vCmcCSQQGJhVqwwmF51085695 = -955341460;    double vCmcCSQQGJhVqwwmF62144483 = -648581334;    double vCmcCSQQGJhVqwwmF78290770 = 20663680;    double vCmcCSQQGJhVqwwmF18384440 = -967703893;    double vCmcCSQQGJhVqwwmF92014097 = 83457473;    double vCmcCSQQGJhVqwwmF27386409 = -531996499;    double vCmcCSQQGJhVqwwmF21000636 = -608893482;    double vCmcCSQQGJhVqwwmF83689865 = -624665190;    double vCmcCSQQGJhVqwwmF46286017 = -881483641;    double vCmcCSQQGJhVqwwmF87810112 = 93679199;    double vCmcCSQQGJhVqwwmF49381207 = -155331482;    double vCmcCSQQGJhVqwwmF73838670 = -727738854;    double vCmcCSQQGJhVqwwmF44888667 = -528773965;    double vCmcCSQQGJhVqwwmF48518106 = -20757265;    double vCmcCSQQGJhVqwwmF12344138 = -146141685;    double vCmcCSQQGJhVqwwmF60842954 = -155924262;    double vCmcCSQQGJhVqwwmF68515398 = -64415815;    double vCmcCSQQGJhVqwwmF15246512 = 59735085;    double vCmcCSQQGJhVqwwmF56046240 = -242829362;    double vCmcCSQQGJhVqwwmF316110 = -868078317;    double vCmcCSQQGJhVqwwmF27560061 = -846425084;    double vCmcCSQQGJhVqwwmF17607351 = -994590258;    double vCmcCSQQGJhVqwwmF71737651 = -397141523;    double vCmcCSQQGJhVqwwmF65533780 = -997128042;    double vCmcCSQQGJhVqwwmF2370422 = -119125382;    double vCmcCSQQGJhVqwwmF56875292 = -182602998;    double vCmcCSQQGJhVqwwmF25545142 = -942553420;    double vCmcCSQQGJhVqwwmF44945973 = -341967545;    double vCmcCSQQGJhVqwwmF98521918 = -354735710;    double vCmcCSQQGJhVqwwmF24429997 = -806656908;    double vCmcCSQQGJhVqwwmF65668395 = -26568011;    double vCmcCSQQGJhVqwwmF87986939 = -806078102;    double vCmcCSQQGJhVqwwmF2520068 = -101127256;    double vCmcCSQQGJhVqwwmF77475849 = -607332903;    double vCmcCSQQGJhVqwwmF40867682 = -747747074;    double vCmcCSQQGJhVqwwmF61335729 = -718750119;    double vCmcCSQQGJhVqwwmF49645563 = -644485478;    double vCmcCSQQGJhVqwwmF39653500 = -31279002;    double vCmcCSQQGJhVqwwmF95282092 = -6345144;    double vCmcCSQQGJhVqwwmF49613558 = -884757918;    double vCmcCSQQGJhVqwwmF79397063 = -179469305;    double vCmcCSQQGJhVqwwmF62117095 = -853572783;    double vCmcCSQQGJhVqwwmF60917054 = -339421476;    double vCmcCSQQGJhVqwwmF6081833 = -854996681;    double vCmcCSQQGJhVqwwmF95615519 = -663385805;    double vCmcCSQQGJhVqwwmF75378470 = -492043278;    double vCmcCSQQGJhVqwwmF99359969 = 34548260;    double vCmcCSQQGJhVqwwmF438033 = -619208439;    double vCmcCSQQGJhVqwwmF53944418 = -92282161;    double vCmcCSQQGJhVqwwmF9693525 = -218580538;    double vCmcCSQQGJhVqwwmF71407671 = -798817778;    double vCmcCSQQGJhVqwwmF55519557 = -113294453;    double vCmcCSQQGJhVqwwmF40554028 = -227499271;    double vCmcCSQQGJhVqwwmF31693260 = -208580422;    double vCmcCSQQGJhVqwwmF64868347 = -777141149;    double vCmcCSQQGJhVqwwmF33478344 = -960751203;    double vCmcCSQQGJhVqwwmF90406832 = -151439811;    double vCmcCSQQGJhVqwwmF12756991 = 17791721;    double vCmcCSQQGJhVqwwmF16014019 = -748578511;    double vCmcCSQQGJhVqwwmF35138806 = -733939529;    double vCmcCSQQGJhVqwwmF1841268 = -589443080;    double vCmcCSQQGJhVqwwmF76054663 = -166925938;    double vCmcCSQQGJhVqwwmF85167947 = -169929480;    double vCmcCSQQGJhVqwwmF21856020 = 25173266;    double vCmcCSQQGJhVqwwmF22141718 = -879752791;    double vCmcCSQQGJhVqwwmF61394268 = -349253380;    double vCmcCSQQGJhVqwwmF71318602 = -526611599;    double vCmcCSQQGJhVqwwmF67412818 = -921441062;    double vCmcCSQQGJhVqwwmF7650425 = -273010191;    double vCmcCSQQGJhVqwwmF51008408 = -427391567;    double vCmcCSQQGJhVqwwmF11197391 = -511438784;    double vCmcCSQQGJhVqwwmF28861898 = -780391391;     vCmcCSQQGJhVqwwmF11234528 = vCmcCSQQGJhVqwwmF69058397;     vCmcCSQQGJhVqwwmF69058397 = vCmcCSQQGJhVqwwmF64353065;     vCmcCSQQGJhVqwwmF64353065 = vCmcCSQQGJhVqwwmF36434464;     vCmcCSQQGJhVqwwmF36434464 = vCmcCSQQGJhVqwwmF84587206;     vCmcCSQQGJhVqwwmF84587206 = vCmcCSQQGJhVqwwmF34529437;     vCmcCSQQGJhVqwwmF34529437 = vCmcCSQQGJhVqwwmF69904257;     vCmcCSQQGJhVqwwmF69904257 = vCmcCSQQGJhVqwwmF91953376;     vCmcCSQQGJhVqwwmF91953376 = vCmcCSQQGJhVqwwmF23480213;     vCmcCSQQGJhVqwwmF23480213 = vCmcCSQQGJhVqwwmF27936334;     vCmcCSQQGJhVqwwmF27936334 = vCmcCSQQGJhVqwwmF58037940;     vCmcCSQQGJhVqwwmF58037940 = vCmcCSQQGJhVqwwmF87296190;     vCmcCSQQGJhVqwwmF87296190 = vCmcCSQQGJhVqwwmF76999967;     vCmcCSQQGJhVqwwmF76999967 = vCmcCSQQGJhVqwwmF397700;     vCmcCSQQGJhVqwwmF397700 = vCmcCSQQGJhVqwwmF45806961;     vCmcCSQQGJhVqwwmF45806961 = vCmcCSQQGJhVqwwmF7203071;     vCmcCSQQGJhVqwwmF7203071 = vCmcCSQQGJhVqwwmF93891945;     vCmcCSQQGJhVqwwmF93891945 = vCmcCSQQGJhVqwwmF44996726;     vCmcCSQQGJhVqwwmF44996726 = vCmcCSQQGJhVqwwmF49217140;     vCmcCSQQGJhVqwwmF49217140 = vCmcCSQQGJhVqwwmF44248636;     vCmcCSQQGJhVqwwmF44248636 = vCmcCSQQGJhVqwwmF48956139;     vCmcCSQQGJhVqwwmF48956139 = vCmcCSQQGJhVqwwmF66288555;     vCmcCSQQGJhVqwwmF66288555 = vCmcCSQQGJhVqwwmF70536479;     vCmcCSQQGJhVqwwmF70536479 = vCmcCSQQGJhVqwwmF39923069;     vCmcCSQQGJhVqwwmF39923069 = vCmcCSQQGJhVqwwmF70766069;     vCmcCSQQGJhVqwwmF70766069 = vCmcCSQQGJhVqwwmF96600267;     vCmcCSQQGJhVqwwmF96600267 = vCmcCSQQGJhVqwwmF32009369;     vCmcCSQQGJhVqwwmF32009369 = vCmcCSQQGJhVqwwmF92428408;     vCmcCSQQGJhVqwwmF92428408 = vCmcCSQQGJhVqwwmF51085695;     vCmcCSQQGJhVqwwmF51085695 = vCmcCSQQGJhVqwwmF62144483;     vCmcCSQQGJhVqwwmF62144483 = vCmcCSQQGJhVqwwmF78290770;     vCmcCSQQGJhVqwwmF78290770 = vCmcCSQQGJhVqwwmF18384440;     vCmcCSQQGJhVqwwmF18384440 = vCmcCSQQGJhVqwwmF92014097;     vCmcCSQQGJhVqwwmF92014097 = vCmcCSQQGJhVqwwmF27386409;     vCmcCSQQGJhVqwwmF27386409 = vCmcCSQQGJhVqwwmF21000636;     vCmcCSQQGJhVqwwmF21000636 = vCmcCSQQGJhVqwwmF83689865;     vCmcCSQQGJhVqwwmF83689865 = vCmcCSQQGJhVqwwmF46286017;     vCmcCSQQGJhVqwwmF46286017 = vCmcCSQQGJhVqwwmF87810112;     vCmcCSQQGJhVqwwmF87810112 = vCmcCSQQGJhVqwwmF49381207;     vCmcCSQQGJhVqwwmF49381207 = vCmcCSQQGJhVqwwmF73838670;     vCmcCSQQGJhVqwwmF73838670 = vCmcCSQQGJhVqwwmF44888667;     vCmcCSQQGJhVqwwmF44888667 = vCmcCSQQGJhVqwwmF48518106;     vCmcCSQQGJhVqwwmF48518106 = vCmcCSQQGJhVqwwmF12344138;     vCmcCSQQGJhVqwwmF12344138 = vCmcCSQQGJhVqwwmF60842954;     vCmcCSQQGJhVqwwmF60842954 = vCmcCSQQGJhVqwwmF68515398;     vCmcCSQQGJhVqwwmF68515398 = vCmcCSQQGJhVqwwmF15246512;     vCmcCSQQGJhVqwwmF15246512 = vCmcCSQQGJhVqwwmF56046240;     vCmcCSQQGJhVqwwmF56046240 = vCmcCSQQGJhVqwwmF316110;     vCmcCSQQGJhVqwwmF316110 = vCmcCSQQGJhVqwwmF27560061;     vCmcCSQQGJhVqwwmF27560061 = vCmcCSQQGJhVqwwmF17607351;     vCmcCSQQGJhVqwwmF17607351 = vCmcCSQQGJhVqwwmF71737651;     vCmcCSQQGJhVqwwmF71737651 = vCmcCSQQGJhVqwwmF65533780;     vCmcCSQQGJhVqwwmF65533780 = vCmcCSQQGJhVqwwmF2370422;     vCmcCSQQGJhVqwwmF2370422 = vCmcCSQQGJhVqwwmF56875292;     vCmcCSQQGJhVqwwmF56875292 = vCmcCSQQGJhVqwwmF25545142;     vCmcCSQQGJhVqwwmF25545142 = vCmcCSQQGJhVqwwmF44945973;     vCmcCSQQGJhVqwwmF44945973 = vCmcCSQQGJhVqwwmF98521918;     vCmcCSQQGJhVqwwmF98521918 = vCmcCSQQGJhVqwwmF24429997;     vCmcCSQQGJhVqwwmF24429997 = vCmcCSQQGJhVqwwmF65668395;     vCmcCSQQGJhVqwwmF65668395 = vCmcCSQQGJhVqwwmF87986939;     vCmcCSQQGJhVqwwmF87986939 = vCmcCSQQGJhVqwwmF2520068;     vCmcCSQQGJhVqwwmF2520068 = vCmcCSQQGJhVqwwmF77475849;     vCmcCSQQGJhVqwwmF77475849 = vCmcCSQQGJhVqwwmF40867682;     vCmcCSQQGJhVqwwmF40867682 = vCmcCSQQGJhVqwwmF61335729;     vCmcCSQQGJhVqwwmF61335729 = vCmcCSQQGJhVqwwmF49645563;     vCmcCSQQGJhVqwwmF49645563 = vCmcCSQQGJhVqwwmF39653500;     vCmcCSQQGJhVqwwmF39653500 = vCmcCSQQGJhVqwwmF95282092;     vCmcCSQQGJhVqwwmF95282092 = vCmcCSQQGJhVqwwmF49613558;     vCmcCSQQGJhVqwwmF49613558 = vCmcCSQQGJhVqwwmF79397063;     vCmcCSQQGJhVqwwmF79397063 = vCmcCSQQGJhVqwwmF62117095;     vCmcCSQQGJhVqwwmF62117095 = vCmcCSQQGJhVqwwmF60917054;     vCmcCSQQGJhVqwwmF60917054 = vCmcCSQQGJhVqwwmF6081833;     vCmcCSQQGJhVqwwmF6081833 = vCmcCSQQGJhVqwwmF95615519;     vCmcCSQQGJhVqwwmF95615519 = vCmcCSQQGJhVqwwmF75378470;     vCmcCSQQGJhVqwwmF75378470 = vCmcCSQQGJhVqwwmF99359969;     vCmcCSQQGJhVqwwmF99359969 = vCmcCSQQGJhVqwwmF438033;     vCmcCSQQGJhVqwwmF438033 = vCmcCSQQGJhVqwwmF53944418;     vCmcCSQQGJhVqwwmF53944418 = vCmcCSQQGJhVqwwmF9693525;     vCmcCSQQGJhVqwwmF9693525 = vCmcCSQQGJhVqwwmF71407671;     vCmcCSQQGJhVqwwmF71407671 = vCmcCSQQGJhVqwwmF55519557;     vCmcCSQQGJhVqwwmF55519557 = vCmcCSQQGJhVqwwmF40554028;     vCmcCSQQGJhVqwwmF40554028 = vCmcCSQQGJhVqwwmF31693260;     vCmcCSQQGJhVqwwmF31693260 = vCmcCSQQGJhVqwwmF64868347;     vCmcCSQQGJhVqwwmF64868347 = vCmcCSQQGJhVqwwmF33478344;     vCmcCSQQGJhVqwwmF33478344 = vCmcCSQQGJhVqwwmF90406832;     vCmcCSQQGJhVqwwmF90406832 = vCmcCSQQGJhVqwwmF12756991;     vCmcCSQQGJhVqwwmF12756991 = vCmcCSQQGJhVqwwmF16014019;     vCmcCSQQGJhVqwwmF16014019 = vCmcCSQQGJhVqwwmF35138806;     vCmcCSQQGJhVqwwmF35138806 = vCmcCSQQGJhVqwwmF1841268;     vCmcCSQQGJhVqwwmF1841268 = vCmcCSQQGJhVqwwmF76054663;     vCmcCSQQGJhVqwwmF76054663 = vCmcCSQQGJhVqwwmF85167947;     vCmcCSQQGJhVqwwmF85167947 = vCmcCSQQGJhVqwwmF21856020;     vCmcCSQQGJhVqwwmF21856020 = vCmcCSQQGJhVqwwmF22141718;     vCmcCSQQGJhVqwwmF22141718 = vCmcCSQQGJhVqwwmF61394268;     vCmcCSQQGJhVqwwmF61394268 = vCmcCSQQGJhVqwwmF71318602;     vCmcCSQQGJhVqwwmF71318602 = vCmcCSQQGJhVqwwmF67412818;     vCmcCSQQGJhVqwwmF67412818 = vCmcCSQQGJhVqwwmF7650425;     vCmcCSQQGJhVqwwmF7650425 = vCmcCSQQGJhVqwwmF51008408;     vCmcCSQQGJhVqwwmF51008408 = vCmcCSQQGJhVqwwmF11197391;     vCmcCSQQGJhVqwwmF11197391 = vCmcCSQQGJhVqwwmF28861898;     vCmcCSQQGJhVqwwmF28861898 = vCmcCSQQGJhVqwwmF11234528;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VtmNsjgIuwvxYDFC89550348() {     double ZctuDWQYrtcypfuxH17351635 = -892293279;    double ZctuDWQYrtcypfuxH12431677 = -638840289;    double ZctuDWQYrtcypfuxH80395249 = -920723421;    double ZctuDWQYrtcypfuxH63509244 = -470876008;    double ZctuDWQYrtcypfuxH53482695 = -897056606;    double ZctuDWQYrtcypfuxH25946639 = -542282229;    double ZctuDWQYrtcypfuxH30393224 = -369864113;    double ZctuDWQYrtcypfuxH41713267 = -989962447;    double ZctuDWQYrtcypfuxH88122708 = -135364118;    double ZctuDWQYrtcypfuxH69165993 = 47202725;    double ZctuDWQYrtcypfuxH18148230 = -976420481;    double ZctuDWQYrtcypfuxH19119233 = -467293872;    double ZctuDWQYrtcypfuxH63209086 = -856645506;    double ZctuDWQYrtcypfuxH83541376 = -320601354;    double ZctuDWQYrtcypfuxH41549387 = -68700139;    double ZctuDWQYrtcypfuxH48708464 = -193904660;    double ZctuDWQYrtcypfuxH87727948 = -377862595;    double ZctuDWQYrtcypfuxH42049879 = -584239929;    double ZctuDWQYrtcypfuxH11046982 = -625551228;    double ZctuDWQYrtcypfuxH24798801 = -951128623;    double ZctuDWQYrtcypfuxH37509332 = -595453159;    double ZctuDWQYrtcypfuxH52809579 = -246353562;    double ZctuDWQYrtcypfuxH6127271 = -221022286;    double ZctuDWQYrtcypfuxH23915443 = -85538580;    double ZctuDWQYrtcypfuxH14465719 = -526755401;    double ZctuDWQYrtcypfuxH21321943 = -448983956;    double ZctuDWQYrtcypfuxH30469148 = 22679058;    double ZctuDWQYrtcypfuxH5535534 = -689495186;    double ZctuDWQYrtcypfuxH17217950 = -77724164;    double ZctuDWQYrtcypfuxH30760121 = -466052201;    double ZctuDWQYrtcypfuxH70533320 = -992657258;    double ZctuDWQYrtcypfuxH15995062 = -677150709;    double ZctuDWQYrtcypfuxH1725047 = -909155897;    double ZctuDWQYrtcypfuxH22216973 = -283538265;    double ZctuDWQYrtcypfuxH82475288 = -147626587;    double ZctuDWQYrtcypfuxH52415778 = -838691297;    double ZctuDWQYrtcypfuxH27547952 = -835119511;    double ZctuDWQYrtcypfuxH21900326 = -777826811;    double ZctuDWQYrtcypfuxH22991461 = -149254979;    double ZctuDWQYrtcypfuxH61340608 = -101178076;    double ZctuDWQYrtcypfuxH50557891 = -594047859;    double ZctuDWQYrtcypfuxH80668563 = -290899543;    double ZctuDWQYrtcypfuxH90411115 = -226946269;    double ZctuDWQYrtcypfuxH48250695 = -175267200;    double ZctuDWQYrtcypfuxH80467863 = -826313969;    double ZctuDWQYrtcypfuxH27185707 = 74518969;    double ZctuDWQYrtcypfuxH73824814 = -361604900;    double ZctuDWQYrtcypfuxH8273051 = -877973850;    double ZctuDWQYrtcypfuxH37235790 = -932162073;    double ZctuDWQYrtcypfuxH30519003 = 18398934;    double ZctuDWQYrtcypfuxH7438330 = -239435195;    double ZctuDWQYrtcypfuxH76343499 = -797854362;    double ZctuDWQYrtcypfuxH50618879 = 89085030;    double ZctuDWQYrtcypfuxH52844172 = -503073143;    double ZctuDWQYrtcypfuxH52041016 = -543085202;    double ZctuDWQYrtcypfuxH64542056 = -545939717;    double ZctuDWQYrtcypfuxH6304406 = -317818004;    double ZctuDWQYrtcypfuxH56479806 = -735184841;    double ZctuDWQYrtcypfuxH49043526 = -944120608;    double ZctuDWQYrtcypfuxH32160753 = -348072650;    double ZctuDWQYrtcypfuxH95477491 = -464961288;    double ZctuDWQYrtcypfuxH24857690 = -680368927;    double ZctuDWQYrtcypfuxH24495317 = -812238283;    double ZctuDWQYrtcypfuxH57362588 = -669311917;    double ZctuDWQYrtcypfuxH98632673 = 39859982;    double ZctuDWQYrtcypfuxH2153169 = -199269773;    double ZctuDWQYrtcypfuxH17394187 = -558137975;    double ZctuDWQYrtcypfuxH40992113 = -473107242;    double ZctuDWQYrtcypfuxH1066089 = -72974767;    double ZctuDWQYrtcypfuxH89133608 = -230008842;    double ZctuDWQYrtcypfuxH21160513 = -358785149;    double ZctuDWQYrtcypfuxH65827623 = -600035784;    double ZctuDWQYrtcypfuxH19058419 = -334984951;    double ZctuDWQYrtcypfuxH49706374 = -424373152;    double ZctuDWQYrtcypfuxH74240910 = -257080765;    double ZctuDWQYrtcypfuxH56840768 = -204553616;    double ZctuDWQYrtcypfuxH62398463 = 80592706;    double ZctuDWQYrtcypfuxH57876576 = 54244914;    double ZctuDWQYrtcypfuxH43447580 = -259224611;    double ZctuDWQYrtcypfuxH87280012 = -501274370;    double ZctuDWQYrtcypfuxH47497129 = 12620943;    double ZctuDWQYrtcypfuxH22196097 = -99347092;    double ZctuDWQYrtcypfuxH68299744 = -757333114;    double ZctuDWQYrtcypfuxH86698947 = 3876901;    double ZctuDWQYrtcypfuxH23321791 = -126617007;    double ZctuDWQYrtcypfuxH94189820 = -94802896;    double ZctuDWQYrtcypfuxH65376183 = -666235739;    double ZctuDWQYrtcypfuxH48880874 = -306082754;    double ZctuDWQYrtcypfuxH70175957 = -740453063;    double ZctuDWQYrtcypfuxH17933232 = -601686871;    double ZctuDWQYrtcypfuxH46111373 = -420873294;    double ZctuDWQYrtcypfuxH71068145 = 65330;    double ZctuDWQYrtcypfuxH72856800 = -833706203;    double ZctuDWQYrtcypfuxH90830708 = -801182329;    double ZctuDWQYrtcypfuxH65863116 = -636216789;    double ZctuDWQYrtcypfuxH25700201 = -913678932;    double ZctuDWQYrtcypfuxH56173246 = -478661261;    double ZctuDWQYrtcypfuxH33048528 = -557634353;    double ZctuDWQYrtcypfuxH49618021 = -115127183;    double ZctuDWQYrtcypfuxH78314694 = -892293279;     ZctuDWQYrtcypfuxH17351635 = ZctuDWQYrtcypfuxH12431677;     ZctuDWQYrtcypfuxH12431677 = ZctuDWQYrtcypfuxH80395249;     ZctuDWQYrtcypfuxH80395249 = ZctuDWQYrtcypfuxH63509244;     ZctuDWQYrtcypfuxH63509244 = ZctuDWQYrtcypfuxH53482695;     ZctuDWQYrtcypfuxH53482695 = ZctuDWQYrtcypfuxH25946639;     ZctuDWQYrtcypfuxH25946639 = ZctuDWQYrtcypfuxH30393224;     ZctuDWQYrtcypfuxH30393224 = ZctuDWQYrtcypfuxH41713267;     ZctuDWQYrtcypfuxH41713267 = ZctuDWQYrtcypfuxH88122708;     ZctuDWQYrtcypfuxH88122708 = ZctuDWQYrtcypfuxH69165993;     ZctuDWQYrtcypfuxH69165993 = ZctuDWQYrtcypfuxH18148230;     ZctuDWQYrtcypfuxH18148230 = ZctuDWQYrtcypfuxH19119233;     ZctuDWQYrtcypfuxH19119233 = ZctuDWQYrtcypfuxH63209086;     ZctuDWQYrtcypfuxH63209086 = ZctuDWQYrtcypfuxH83541376;     ZctuDWQYrtcypfuxH83541376 = ZctuDWQYrtcypfuxH41549387;     ZctuDWQYrtcypfuxH41549387 = ZctuDWQYrtcypfuxH48708464;     ZctuDWQYrtcypfuxH48708464 = ZctuDWQYrtcypfuxH87727948;     ZctuDWQYrtcypfuxH87727948 = ZctuDWQYrtcypfuxH42049879;     ZctuDWQYrtcypfuxH42049879 = ZctuDWQYrtcypfuxH11046982;     ZctuDWQYrtcypfuxH11046982 = ZctuDWQYrtcypfuxH24798801;     ZctuDWQYrtcypfuxH24798801 = ZctuDWQYrtcypfuxH37509332;     ZctuDWQYrtcypfuxH37509332 = ZctuDWQYrtcypfuxH52809579;     ZctuDWQYrtcypfuxH52809579 = ZctuDWQYrtcypfuxH6127271;     ZctuDWQYrtcypfuxH6127271 = ZctuDWQYrtcypfuxH23915443;     ZctuDWQYrtcypfuxH23915443 = ZctuDWQYrtcypfuxH14465719;     ZctuDWQYrtcypfuxH14465719 = ZctuDWQYrtcypfuxH21321943;     ZctuDWQYrtcypfuxH21321943 = ZctuDWQYrtcypfuxH30469148;     ZctuDWQYrtcypfuxH30469148 = ZctuDWQYrtcypfuxH5535534;     ZctuDWQYrtcypfuxH5535534 = ZctuDWQYrtcypfuxH17217950;     ZctuDWQYrtcypfuxH17217950 = ZctuDWQYrtcypfuxH30760121;     ZctuDWQYrtcypfuxH30760121 = ZctuDWQYrtcypfuxH70533320;     ZctuDWQYrtcypfuxH70533320 = ZctuDWQYrtcypfuxH15995062;     ZctuDWQYrtcypfuxH15995062 = ZctuDWQYrtcypfuxH1725047;     ZctuDWQYrtcypfuxH1725047 = ZctuDWQYrtcypfuxH22216973;     ZctuDWQYrtcypfuxH22216973 = ZctuDWQYrtcypfuxH82475288;     ZctuDWQYrtcypfuxH82475288 = ZctuDWQYrtcypfuxH52415778;     ZctuDWQYrtcypfuxH52415778 = ZctuDWQYrtcypfuxH27547952;     ZctuDWQYrtcypfuxH27547952 = ZctuDWQYrtcypfuxH21900326;     ZctuDWQYrtcypfuxH21900326 = ZctuDWQYrtcypfuxH22991461;     ZctuDWQYrtcypfuxH22991461 = ZctuDWQYrtcypfuxH61340608;     ZctuDWQYrtcypfuxH61340608 = ZctuDWQYrtcypfuxH50557891;     ZctuDWQYrtcypfuxH50557891 = ZctuDWQYrtcypfuxH80668563;     ZctuDWQYrtcypfuxH80668563 = ZctuDWQYrtcypfuxH90411115;     ZctuDWQYrtcypfuxH90411115 = ZctuDWQYrtcypfuxH48250695;     ZctuDWQYrtcypfuxH48250695 = ZctuDWQYrtcypfuxH80467863;     ZctuDWQYrtcypfuxH80467863 = ZctuDWQYrtcypfuxH27185707;     ZctuDWQYrtcypfuxH27185707 = ZctuDWQYrtcypfuxH73824814;     ZctuDWQYrtcypfuxH73824814 = ZctuDWQYrtcypfuxH8273051;     ZctuDWQYrtcypfuxH8273051 = ZctuDWQYrtcypfuxH37235790;     ZctuDWQYrtcypfuxH37235790 = ZctuDWQYrtcypfuxH30519003;     ZctuDWQYrtcypfuxH30519003 = ZctuDWQYrtcypfuxH7438330;     ZctuDWQYrtcypfuxH7438330 = ZctuDWQYrtcypfuxH76343499;     ZctuDWQYrtcypfuxH76343499 = ZctuDWQYrtcypfuxH50618879;     ZctuDWQYrtcypfuxH50618879 = ZctuDWQYrtcypfuxH52844172;     ZctuDWQYrtcypfuxH52844172 = ZctuDWQYrtcypfuxH52041016;     ZctuDWQYrtcypfuxH52041016 = ZctuDWQYrtcypfuxH64542056;     ZctuDWQYrtcypfuxH64542056 = ZctuDWQYrtcypfuxH6304406;     ZctuDWQYrtcypfuxH6304406 = ZctuDWQYrtcypfuxH56479806;     ZctuDWQYrtcypfuxH56479806 = ZctuDWQYrtcypfuxH49043526;     ZctuDWQYrtcypfuxH49043526 = ZctuDWQYrtcypfuxH32160753;     ZctuDWQYrtcypfuxH32160753 = ZctuDWQYrtcypfuxH95477491;     ZctuDWQYrtcypfuxH95477491 = ZctuDWQYrtcypfuxH24857690;     ZctuDWQYrtcypfuxH24857690 = ZctuDWQYrtcypfuxH24495317;     ZctuDWQYrtcypfuxH24495317 = ZctuDWQYrtcypfuxH57362588;     ZctuDWQYrtcypfuxH57362588 = ZctuDWQYrtcypfuxH98632673;     ZctuDWQYrtcypfuxH98632673 = ZctuDWQYrtcypfuxH2153169;     ZctuDWQYrtcypfuxH2153169 = ZctuDWQYrtcypfuxH17394187;     ZctuDWQYrtcypfuxH17394187 = ZctuDWQYrtcypfuxH40992113;     ZctuDWQYrtcypfuxH40992113 = ZctuDWQYrtcypfuxH1066089;     ZctuDWQYrtcypfuxH1066089 = ZctuDWQYrtcypfuxH89133608;     ZctuDWQYrtcypfuxH89133608 = ZctuDWQYrtcypfuxH21160513;     ZctuDWQYrtcypfuxH21160513 = ZctuDWQYrtcypfuxH65827623;     ZctuDWQYrtcypfuxH65827623 = ZctuDWQYrtcypfuxH19058419;     ZctuDWQYrtcypfuxH19058419 = ZctuDWQYrtcypfuxH49706374;     ZctuDWQYrtcypfuxH49706374 = ZctuDWQYrtcypfuxH74240910;     ZctuDWQYrtcypfuxH74240910 = ZctuDWQYrtcypfuxH56840768;     ZctuDWQYrtcypfuxH56840768 = ZctuDWQYrtcypfuxH62398463;     ZctuDWQYrtcypfuxH62398463 = ZctuDWQYrtcypfuxH57876576;     ZctuDWQYrtcypfuxH57876576 = ZctuDWQYrtcypfuxH43447580;     ZctuDWQYrtcypfuxH43447580 = ZctuDWQYrtcypfuxH87280012;     ZctuDWQYrtcypfuxH87280012 = ZctuDWQYrtcypfuxH47497129;     ZctuDWQYrtcypfuxH47497129 = ZctuDWQYrtcypfuxH22196097;     ZctuDWQYrtcypfuxH22196097 = ZctuDWQYrtcypfuxH68299744;     ZctuDWQYrtcypfuxH68299744 = ZctuDWQYrtcypfuxH86698947;     ZctuDWQYrtcypfuxH86698947 = ZctuDWQYrtcypfuxH23321791;     ZctuDWQYrtcypfuxH23321791 = ZctuDWQYrtcypfuxH94189820;     ZctuDWQYrtcypfuxH94189820 = ZctuDWQYrtcypfuxH65376183;     ZctuDWQYrtcypfuxH65376183 = ZctuDWQYrtcypfuxH48880874;     ZctuDWQYrtcypfuxH48880874 = ZctuDWQYrtcypfuxH70175957;     ZctuDWQYrtcypfuxH70175957 = ZctuDWQYrtcypfuxH17933232;     ZctuDWQYrtcypfuxH17933232 = ZctuDWQYrtcypfuxH46111373;     ZctuDWQYrtcypfuxH46111373 = ZctuDWQYrtcypfuxH71068145;     ZctuDWQYrtcypfuxH71068145 = ZctuDWQYrtcypfuxH72856800;     ZctuDWQYrtcypfuxH72856800 = ZctuDWQYrtcypfuxH90830708;     ZctuDWQYrtcypfuxH90830708 = ZctuDWQYrtcypfuxH65863116;     ZctuDWQYrtcypfuxH65863116 = ZctuDWQYrtcypfuxH25700201;     ZctuDWQYrtcypfuxH25700201 = ZctuDWQYrtcypfuxH56173246;     ZctuDWQYrtcypfuxH56173246 = ZctuDWQYrtcypfuxH33048528;     ZctuDWQYrtcypfuxH33048528 = ZctuDWQYrtcypfuxH49618021;     ZctuDWQYrtcypfuxH49618021 = ZctuDWQYrtcypfuxH78314694;     ZctuDWQYrtcypfuxH78314694 = ZctuDWQYrtcypfuxH17351635;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LSZwbBaVomyNIxUs56841947() {     double gTtgzLspCAnardDBL52810603 = -258570179;    double gTtgzLspCAnardDBL25703755 = -519391896;    double gTtgzLspCAnardDBL17781726 = -586390496;    double gTtgzLspCAnardDBL35732599 = -292027080;    double gTtgzLspCAnardDBL9767965 = 97344549;    double gTtgzLspCAnardDBL62169511 = -804630124;    double gTtgzLspCAnardDBL41050630 = -214589847;    double gTtgzLspCAnardDBL3844829 = -634196587;    double gTtgzLspCAnardDBL89640313 = -623695525;    double gTtgzLspCAnardDBL84068705 = -717484273;    double gTtgzLspCAnardDBL56329965 = -778428210;    double gTtgzLspCAnardDBL19687381 = -331414643;    double gTtgzLspCAnardDBL1443732 = -308658011;    double gTtgzLspCAnardDBL36682240 = -296964763;    double gTtgzLspCAnardDBL99743903 = -572125154;    double gTtgzLspCAnardDBL32249765 = -929716267;    double gTtgzLspCAnardDBL43563884 = -904837147;    double gTtgzLspCAnardDBL41810071 = -657667998;    double gTtgzLspCAnardDBL11597312 = -942114649;    double gTtgzLspCAnardDBL18636020 = -277689689;    double gTtgzLspCAnardDBL50596195 = -252727847;    double gTtgzLspCAnardDBL56231103 = -332989856;    double gTtgzLspCAnardDBL76652650 = -958654470;    double gTtgzLspCAnardDBL17708388 = -596363184;    double gTtgzLspCAnardDBL22483670 = -550740599;    double gTtgzLspCAnardDBL32020114 = -137560762;    double gTtgzLspCAnardDBL90889721 = -883605082;    double gTtgzLspCAnardDBL46263959 = -859267361;    double gTtgzLspCAnardDBL63886165 = -651818967;    double gTtgzLspCAnardDBL36293531 = -500559372;    double gTtgzLspCAnardDBL89116551 = -444637745;    double gTtgzLspCAnardDBL53818080 = -450122123;    double gTtgzLspCAnardDBL13943057 = 43248126;    double gTtgzLspCAnardDBL51965400 = -348232078;    double gTtgzLspCAnardDBL48283742 = -98726838;    double gTtgzLspCAnardDBL3976623 = 46743397;    double gTtgzLspCAnardDBL76959768 = -699815551;    double gTtgzLspCAnardDBL73115728 = -760482392;    double gTtgzLspCAnardDBL53382022 = -900131255;    double gTtgzLspCAnardDBL25612140 = -666440622;    double gTtgzLspCAnardDBL84009498 = -363246773;    double gTtgzLspCAnardDBL70033690 = -985444834;    double gTtgzLspCAnardDBL79794494 = -880127055;    double gTtgzLspCAnardDBL88382032 = -779599610;    double gTtgzLspCAnardDBL80484321 = -652825131;    double gTtgzLspCAnardDBL73147035 = 35595336;    double gTtgzLspCAnardDBL93169977 = -845591260;    double gTtgzLspCAnardDBL25999362 = 55499557;    double gTtgzLspCAnardDBL60128456 = -595898953;    double gTtgzLspCAnardDBL55048592 = -472627881;    double gTtgzLspCAnardDBL88209053 = -113711752;    double gTtgzLspCAnardDBL59828971 = 88493292;    double gTtgzLspCAnardDBL19240028 = -766960524;    double gTtgzLspCAnardDBL68836019 = -694581643;    double gTtgzLspCAnardDBL6245753 = -889575348;    double gTtgzLspCAnardDBL96579499 = -925580323;    double gTtgzLspCAnardDBL49051105 = -560737426;    double gTtgzLspCAnardDBL73338 = -990027312;    double gTtgzLspCAnardDBL13248930 = -741286481;    double gTtgzLspCAnardDBL77747850 = -765094689;    double gTtgzLspCAnardDBL71279790 = -921025042;    double gTtgzLspCAnardDBL94786671 = -355322487;    double gTtgzLspCAnardDBL39958663 = -982377621;    double gTtgzLspCAnardDBL53346782 = -23136153;    double gTtgzLspCAnardDBL94952153 = -172846528;    double gTtgzLspCAnardDBL2511885 = -228306087;    double gTtgzLspCAnardDBL5744325 = -274662770;    double gTtgzLspCAnardDBL49478332 = -960425933;    double gTtgzLspCAnardDBL88398498 = -98237926;    double gTtgzLspCAnardDBL95767281 = -518868551;    double gTtgzLspCAnardDBL55289997 = -129900716;    double gTtgzLspCAnardDBL70448155 = -44354756;    double gTtgzLspCAnardDBL88428048 = -757536743;    double gTtgzLspCAnardDBL85985171 = -175674027;    double gTtgzLspCAnardDBL34626521 = -914442917;    double gTtgzLspCAnardDBL80562504 = -267283014;    double gTtgzLspCAnardDBL76436609 = -452862802;    double gTtgzLspCAnardDBL88270617 = -79054861;    double gTtgzLspCAnardDBL37224066 = -943538054;    double gTtgzLspCAnardDBL49336635 = -486335936;    double gTtgzLspCAnardDBL38850137 = -291969503;    double gTtgzLspCAnardDBL64890359 = -839104640;    double gTtgzLspCAnardDBL86135502 = -163368409;    double gTtgzLspCAnardDBL8837573 = -79191086;    double gTtgzLspCAnardDBL48084478 = -286847620;    double gTtgzLspCAnardDBL29287580 = -433131037;    double gTtgzLspCAnardDBL34578053 = -683161599;    double gTtgzLspCAnardDBL45107038 = -262170232;    double gTtgzLspCAnardDBL45719647 = -458656731;    double gTtgzLspCAnardDBL51704242 = -173146515;    double gTtgzLspCAnardDBL54925518 = -392519178;    double gTtgzLspCAnardDBL76886430 = -709788239;    double gTtgzLspCAnardDBL59866799 = 80804089;    double gTtgzLspCAnardDBL75634172 = -35036567;    double gTtgzLspCAnardDBL54332350 = -745415581;    double gTtgzLspCAnardDBL89222827 = 92075713;    double gTtgzLspCAnardDBL30075027 = 96932786;    double gTtgzLspCAnardDBL26447712 = -756990902;    double gTtgzLspCAnardDBL93429879 = -506753082;    double gTtgzLspCAnardDBL77972437 = -258570179;     gTtgzLspCAnardDBL52810603 = gTtgzLspCAnardDBL25703755;     gTtgzLspCAnardDBL25703755 = gTtgzLspCAnardDBL17781726;     gTtgzLspCAnardDBL17781726 = gTtgzLspCAnardDBL35732599;     gTtgzLspCAnardDBL35732599 = gTtgzLspCAnardDBL9767965;     gTtgzLspCAnardDBL9767965 = gTtgzLspCAnardDBL62169511;     gTtgzLspCAnardDBL62169511 = gTtgzLspCAnardDBL41050630;     gTtgzLspCAnardDBL41050630 = gTtgzLspCAnardDBL3844829;     gTtgzLspCAnardDBL3844829 = gTtgzLspCAnardDBL89640313;     gTtgzLspCAnardDBL89640313 = gTtgzLspCAnardDBL84068705;     gTtgzLspCAnardDBL84068705 = gTtgzLspCAnardDBL56329965;     gTtgzLspCAnardDBL56329965 = gTtgzLspCAnardDBL19687381;     gTtgzLspCAnardDBL19687381 = gTtgzLspCAnardDBL1443732;     gTtgzLspCAnardDBL1443732 = gTtgzLspCAnardDBL36682240;     gTtgzLspCAnardDBL36682240 = gTtgzLspCAnardDBL99743903;     gTtgzLspCAnardDBL99743903 = gTtgzLspCAnardDBL32249765;     gTtgzLspCAnardDBL32249765 = gTtgzLspCAnardDBL43563884;     gTtgzLspCAnardDBL43563884 = gTtgzLspCAnardDBL41810071;     gTtgzLspCAnardDBL41810071 = gTtgzLspCAnardDBL11597312;     gTtgzLspCAnardDBL11597312 = gTtgzLspCAnardDBL18636020;     gTtgzLspCAnardDBL18636020 = gTtgzLspCAnardDBL50596195;     gTtgzLspCAnardDBL50596195 = gTtgzLspCAnardDBL56231103;     gTtgzLspCAnardDBL56231103 = gTtgzLspCAnardDBL76652650;     gTtgzLspCAnardDBL76652650 = gTtgzLspCAnardDBL17708388;     gTtgzLspCAnardDBL17708388 = gTtgzLspCAnardDBL22483670;     gTtgzLspCAnardDBL22483670 = gTtgzLspCAnardDBL32020114;     gTtgzLspCAnardDBL32020114 = gTtgzLspCAnardDBL90889721;     gTtgzLspCAnardDBL90889721 = gTtgzLspCAnardDBL46263959;     gTtgzLspCAnardDBL46263959 = gTtgzLspCAnardDBL63886165;     gTtgzLspCAnardDBL63886165 = gTtgzLspCAnardDBL36293531;     gTtgzLspCAnardDBL36293531 = gTtgzLspCAnardDBL89116551;     gTtgzLspCAnardDBL89116551 = gTtgzLspCAnardDBL53818080;     gTtgzLspCAnardDBL53818080 = gTtgzLspCAnardDBL13943057;     gTtgzLspCAnardDBL13943057 = gTtgzLspCAnardDBL51965400;     gTtgzLspCAnardDBL51965400 = gTtgzLspCAnardDBL48283742;     gTtgzLspCAnardDBL48283742 = gTtgzLspCAnardDBL3976623;     gTtgzLspCAnardDBL3976623 = gTtgzLspCAnardDBL76959768;     gTtgzLspCAnardDBL76959768 = gTtgzLspCAnardDBL73115728;     gTtgzLspCAnardDBL73115728 = gTtgzLspCAnardDBL53382022;     gTtgzLspCAnardDBL53382022 = gTtgzLspCAnardDBL25612140;     gTtgzLspCAnardDBL25612140 = gTtgzLspCAnardDBL84009498;     gTtgzLspCAnardDBL84009498 = gTtgzLspCAnardDBL70033690;     gTtgzLspCAnardDBL70033690 = gTtgzLspCAnardDBL79794494;     gTtgzLspCAnardDBL79794494 = gTtgzLspCAnardDBL88382032;     gTtgzLspCAnardDBL88382032 = gTtgzLspCAnardDBL80484321;     gTtgzLspCAnardDBL80484321 = gTtgzLspCAnardDBL73147035;     gTtgzLspCAnardDBL73147035 = gTtgzLspCAnardDBL93169977;     gTtgzLspCAnardDBL93169977 = gTtgzLspCAnardDBL25999362;     gTtgzLspCAnardDBL25999362 = gTtgzLspCAnardDBL60128456;     gTtgzLspCAnardDBL60128456 = gTtgzLspCAnardDBL55048592;     gTtgzLspCAnardDBL55048592 = gTtgzLspCAnardDBL88209053;     gTtgzLspCAnardDBL88209053 = gTtgzLspCAnardDBL59828971;     gTtgzLspCAnardDBL59828971 = gTtgzLspCAnardDBL19240028;     gTtgzLspCAnardDBL19240028 = gTtgzLspCAnardDBL68836019;     gTtgzLspCAnardDBL68836019 = gTtgzLspCAnardDBL6245753;     gTtgzLspCAnardDBL6245753 = gTtgzLspCAnardDBL96579499;     gTtgzLspCAnardDBL96579499 = gTtgzLspCAnardDBL49051105;     gTtgzLspCAnardDBL49051105 = gTtgzLspCAnardDBL73338;     gTtgzLspCAnardDBL73338 = gTtgzLspCAnardDBL13248930;     gTtgzLspCAnardDBL13248930 = gTtgzLspCAnardDBL77747850;     gTtgzLspCAnardDBL77747850 = gTtgzLspCAnardDBL71279790;     gTtgzLspCAnardDBL71279790 = gTtgzLspCAnardDBL94786671;     gTtgzLspCAnardDBL94786671 = gTtgzLspCAnardDBL39958663;     gTtgzLspCAnardDBL39958663 = gTtgzLspCAnardDBL53346782;     gTtgzLspCAnardDBL53346782 = gTtgzLspCAnardDBL94952153;     gTtgzLspCAnardDBL94952153 = gTtgzLspCAnardDBL2511885;     gTtgzLspCAnardDBL2511885 = gTtgzLspCAnardDBL5744325;     gTtgzLspCAnardDBL5744325 = gTtgzLspCAnardDBL49478332;     gTtgzLspCAnardDBL49478332 = gTtgzLspCAnardDBL88398498;     gTtgzLspCAnardDBL88398498 = gTtgzLspCAnardDBL95767281;     gTtgzLspCAnardDBL95767281 = gTtgzLspCAnardDBL55289997;     gTtgzLspCAnardDBL55289997 = gTtgzLspCAnardDBL70448155;     gTtgzLspCAnardDBL70448155 = gTtgzLspCAnardDBL88428048;     gTtgzLspCAnardDBL88428048 = gTtgzLspCAnardDBL85985171;     gTtgzLspCAnardDBL85985171 = gTtgzLspCAnardDBL34626521;     gTtgzLspCAnardDBL34626521 = gTtgzLspCAnardDBL80562504;     gTtgzLspCAnardDBL80562504 = gTtgzLspCAnardDBL76436609;     gTtgzLspCAnardDBL76436609 = gTtgzLspCAnardDBL88270617;     gTtgzLspCAnardDBL88270617 = gTtgzLspCAnardDBL37224066;     gTtgzLspCAnardDBL37224066 = gTtgzLspCAnardDBL49336635;     gTtgzLspCAnardDBL49336635 = gTtgzLspCAnardDBL38850137;     gTtgzLspCAnardDBL38850137 = gTtgzLspCAnardDBL64890359;     gTtgzLspCAnardDBL64890359 = gTtgzLspCAnardDBL86135502;     gTtgzLspCAnardDBL86135502 = gTtgzLspCAnardDBL8837573;     gTtgzLspCAnardDBL8837573 = gTtgzLspCAnardDBL48084478;     gTtgzLspCAnardDBL48084478 = gTtgzLspCAnardDBL29287580;     gTtgzLspCAnardDBL29287580 = gTtgzLspCAnardDBL34578053;     gTtgzLspCAnardDBL34578053 = gTtgzLspCAnardDBL45107038;     gTtgzLspCAnardDBL45107038 = gTtgzLspCAnardDBL45719647;     gTtgzLspCAnardDBL45719647 = gTtgzLspCAnardDBL51704242;     gTtgzLspCAnardDBL51704242 = gTtgzLspCAnardDBL54925518;     gTtgzLspCAnardDBL54925518 = gTtgzLspCAnardDBL76886430;     gTtgzLspCAnardDBL76886430 = gTtgzLspCAnardDBL59866799;     gTtgzLspCAnardDBL59866799 = gTtgzLspCAnardDBL75634172;     gTtgzLspCAnardDBL75634172 = gTtgzLspCAnardDBL54332350;     gTtgzLspCAnardDBL54332350 = gTtgzLspCAnardDBL89222827;     gTtgzLspCAnardDBL89222827 = gTtgzLspCAnardDBL30075027;     gTtgzLspCAnardDBL30075027 = gTtgzLspCAnardDBL26447712;     gTtgzLspCAnardDBL26447712 = gTtgzLspCAnardDBL93429879;     gTtgzLspCAnardDBL93429879 = gTtgzLspCAnardDBL77972437;     gTtgzLspCAnardDBL77972437 = gTtgzLspCAnardDBL52810603;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void TegdaKAnkJbuilws93486921() {     double OVuGLyDxgjAXHwBBS75197720 = -992974142;    double OVuGLyDxgjAXHwBBS10051256 = -467934950;    double OVuGLyDxgjAXHwBBS46551005 = -826044340;    double OVuGLyDxgjAXHwBBS50739288 = -298434611;    double OVuGLyDxgjAXHwBBS7942219 = -493712452;    double OVuGLyDxgjAXHwBBS77052628 = -665238502;    double OVuGLyDxgjAXHwBBS33206369 = -36577153;    double OVuGLyDxgjAXHwBBS79478687 = -4793232;    double OVuGLyDxgjAXHwBBS86803444 = -507891564;    double OVuGLyDxgjAXHwBBS75832290 = -460225606;    double OVuGLyDxgjAXHwBBS93871824 = -775052611;    double OVuGLyDxgjAXHwBBS75590330 = -582159937;    double OVuGLyDxgjAXHwBBS68451912 = -927375330;    double OVuGLyDxgjAXHwBBS79296991 = -824673520;    double OVuGLyDxgjAXHwBBS36479181 = -199602867;    double OVuGLyDxgjAXHwBBS76505677 = -211383809;    double OVuGLyDxgjAXHwBBS93387236 = -82116967;    double OVuGLyDxgjAXHwBBS47786487 = -9410689;    double OVuGLyDxgjAXHwBBS52878942 = 22783226;    double OVuGLyDxgjAXHwBBS97152302 = -530324143;    double OVuGLyDxgjAXHwBBS51479302 = -244215156;    double OVuGLyDxgjAXHwBBS20046322 = -837756170;    double OVuGLyDxgjAXHwBBS79945973 = -34735062;    double OVuGLyDxgjAXHwBBS44208175 = -60356041;    double OVuGLyDxgjAXHwBBS58023917 = -172299724;    double OVuGLyDxgjAXHwBBS20323109 = -581454986;    double OVuGLyDxgjAXHwBBS37979142 = -641191575;    double OVuGLyDxgjAXHwBBS52175409 = -647721814;    double OVuGLyDxgjAXHwBBS70778725 = -996076086;    double OVuGLyDxgjAXHwBBS45450711 = -420855239;    double OVuGLyDxgjAXHwBBS33407357 = -525953897;    double OVuGLyDxgjAXHwBBS11359271 = -2193478;    double OVuGLyDxgjAXHwBBS94981727 = -486095369;    double OVuGLyDxgjAXHwBBS11354087 = -672358928;    double OVuGLyDxgjAXHwBBS24513106 = -162483261;    double OVuGLyDxgjAXHwBBS45669492 = -268651980;    double OVuGLyDxgjAXHwBBS70399480 = -432763503;    double OVuGLyDxgjAXHwBBS26741830 = -628107864;    double OVuGLyDxgjAXHwBBS1690154 = -201177287;    double OVuGLyDxgjAXHwBBS45797855 = -802664652;    double OVuGLyDxgjAXHwBBS12766870 = -866424439;    double OVuGLyDxgjAXHwBBS58542081 = -743353525;    double OVuGLyDxgjAXHwBBS85344686 = -767657639;    double OVuGLyDxgjAXHwBBS87826152 = -776963259;    double OVuGLyDxgjAXHwBBS48467588 = -461968608;    double OVuGLyDxgjAXHwBBS81247317 = -146633759;    double OVuGLyDxgjAXHwBBS5467375 = -324001513;    double OVuGLyDxgjAXHwBBS70598036 = -801035434;    double OVuGLyDxgjAXHwBBS54588361 = -545615651;    double OVuGLyDxgjAXHwBBS52132338 = -614648140;    double OVuGLyDxgjAXHwBBS97078270 = -468788029;    double OVuGLyDxgjAXHwBBS33680229 = -342325990;    double OVuGLyDxgjAXHwBBS97554430 = -438871754;    double OVuGLyDxgjAXHwBBS29122564 = -970262452;    double OVuGLyDxgjAXHwBBS42007620 = 69720537;    double OVuGLyDxgjAXHwBBS55151399 = -55217973;    double OVuGLyDxgjAXHwBBS30105282 = -333199889;    double OVuGLyDxgjAXHwBBS2342830 = -665688299;    double OVuGLyDxgjAXHwBBS92715370 = -26134888;    double OVuGLyDxgjAXHwBBS87619109 = -912257466;    double OVuGLyDxgjAXHwBBS39073486 = 75953073;    double OVuGLyDxgjAXHwBBS81030960 = -388855339;    double OVuGLyDxgjAXHwBBS8699962 = -8717146;    double OVuGLyDxgjAXHwBBS41352734 = 12963674;    double OVuGLyDxgjAXHwBBS42424933 = -934271709;    double OVuGLyDxgjAXHwBBS82512553 = -672859134;    double OVuGLyDxgjAXHwBBS80608602 = 3935432;    double OVuGLyDxgjAXHwBBS57097825 = -155016403;    double OVuGLyDxgjAXHwBBS54783886 = -562190260;    double OVuGLyDxgjAXHwBBS90809689 = -930950888;    double OVuGLyDxgjAXHwBBS6106198 = -778620307;    double OVuGLyDxgjAXHwBBS66645406 = -454009104;    double OVuGLyDxgjAXHwBBS46096334 = -808233402;    double OVuGLyDxgjAXHwBBS7081088 = -174552123;    double OVuGLyDxgjAXHwBBS84385433 = -663899704;    double OVuGLyDxgjAXHwBBS92937220 = -500861631;    double OVuGLyDxgjAXHwBBS34701636 = 29901469;    double OVuGLyDxgjAXHwBBS92119821 = -257771804;    double OVuGLyDxgjAXHwBBS95740586 = -598387433;    double OVuGLyDxgjAXHwBBS76776600 = 74334035;    double OVuGLyDxgjAXHwBBS14855734 = -157453474;    double OVuGLyDxgjAXHwBBS67381105 = -840156142;    double OVuGLyDxgjAXHwBBS97587047 = -2106164;    double OVuGLyDxgjAXHwBBS18646388 = -281427946;    double OVuGLyDxgjAXHwBBS48372441 = -952067210;    double OVuGLyDxgjAXHwBBS99727128 = -83627907;    double OVuGLyDxgjAXHwBBS13804841 = -563321724;    double OVuGLyDxgjAXHwBBS65859163 = -515832918;    double OVuGLyDxgjAXHwBBS69346467 = -642079466;    double OVuGLyDxgjAXHwBBS69361707 = -7265288;    double OVuGLyDxgjAXHwBBS15564210 = -935452091;    double OVuGLyDxgjAXHwBBS68056650 = -767075204;    double OVuGLyDxgjAXHwBBS34026459 = -501972976;    double OVuGLyDxgjAXHwBBS14071044 = -288919821;    double OVuGLyDxgjAXHwBBS6724369 = -778617725;    double OVuGLyDxgjAXHwBBS31735910 = -377569100;    double OVuGLyDxgjAXHwBBS49842120 = -634636380;    double OVuGLyDxgjAXHwBBS43991952 = -680621314;    double OVuGLyDxgjAXHwBBS45401219 = -842691550;    double OVuGLyDxgjAXHwBBS65955035 = -992974142;     OVuGLyDxgjAXHwBBS75197720 = OVuGLyDxgjAXHwBBS10051256;     OVuGLyDxgjAXHwBBS10051256 = OVuGLyDxgjAXHwBBS46551005;     OVuGLyDxgjAXHwBBS46551005 = OVuGLyDxgjAXHwBBS50739288;     OVuGLyDxgjAXHwBBS50739288 = OVuGLyDxgjAXHwBBS7942219;     OVuGLyDxgjAXHwBBS7942219 = OVuGLyDxgjAXHwBBS77052628;     OVuGLyDxgjAXHwBBS77052628 = OVuGLyDxgjAXHwBBS33206369;     OVuGLyDxgjAXHwBBS33206369 = OVuGLyDxgjAXHwBBS79478687;     OVuGLyDxgjAXHwBBS79478687 = OVuGLyDxgjAXHwBBS86803444;     OVuGLyDxgjAXHwBBS86803444 = OVuGLyDxgjAXHwBBS75832290;     OVuGLyDxgjAXHwBBS75832290 = OVuGLyDxgjAXHwBBS93871824;     OVuGLyDxgjAXHwBBS93871824 = OVuGLyDxgjAXHwBBS75590330;     OVuGLyDxgjAXHwBBS75590330 = OVuGLyDxgjAXHwBBS68451912;     OVuGLyDxgjAXHwBBS68451912 = OVuGLyDxgjAXHwBBS79296991;     OVuGLyDxgjAXHwBBS79296991 = OVuGLyDxgjAXHwBBS36479181;     OVuGLyDxgjAXHwBBS36479181 = OVuGLyDxgjAXHwBBS76505677;     OVuGLyDxgjAXHwBBS76505677 = OVuGLyDxgjAXHwBBS93387236;     OVuGLyDxgjAXHwBBS93387236 = OVuGLyDxgjAXHwBBS47786487;     OVuGLyDxgjAXHwBBS47786487 = OVuGLyDxgjAXHwBBS52878942;     OVuGLyDxgjAXHwBBS52878942 = OVuGLyDxgjAXHwBBS97152302;     OVuGLyDxgjAXHwBBS97152302 = OVuGLyDxgjAXHwBBS51479302;     OVuGLyDxgjAXHwBBS51479302 = OVuGLyDxgjAXHwBBS20046322;     OVuGLyDxgjAXHwBBS20046322 = OVuGLyDxgjAXHwBBS79945973;     OVuGLyDxgjAXHwBBS79945973 = OVuGLyDxgjAXHwBBS44208175;     OVuGLyDxgjAXHwBBS44208175 = OVuGLyDxgjAXHwBBS58023917;     OVuGLyDxgjAXHwBBS58023917 = OVuGLyDxgjAXHwBBS20323109;     OVuGLyDxgjAXHwBBS20323109 = OVuGLyDxgjAXHwBBS37979142;     OVuGLyDxgjAXHwBBS37979142 = OVuGLyDxgjAXHwBBS52175409;     OVuGLyDxgjAXHwBBS52175409 = OVuGLyDxgjAXHwBBS70778725;     OVuGLyDxgjAXHwBBS70778725 = OVuGLyDxgjAXHwBBS45450711;     OVuGLyDxgjAXHwBBS45450711 = OVuGLyDxgjAXHwBBS33407357;     OVuGLyDxgjAXHwBBS33407357 = OVuGLyDxgjAXHwBBS11359271;     OVuGLyDxgjAXHwBBS11359271 = OVuGLyDxgjAXHwBBS94981727;     OVuGLyDxgjAXHwBBS94981727 = OVuGLyDxgjAXHwBBS11354087;     OVuGLyDxgjAXHwBBS11354087 = OVuGLyDxgjAXHwBBS24513106;     OVuGLyDxgjAXHwBBS24513106 = OVuGLyDxgjAXHwBBS45669492;     OVuGLyDxgjAXHwBBS45669492 = OVuGLyDxgjAXHwBBS70399480;     OVuGLyDxgjAXHwBBS70399480 = OVuGLyDxgjAXHwBBS26741830;     OVuGLyDxgjAXHwBBS26741830 = OVuGLyDxgjAXHwBBS1690154;     OVuGLyDxgjAXHwBBS1690154 = OVuGLyDxgjAXHwBBS45797855;     OVuGLyDxgjAXHwBBS45797855 = OVuGLyDxgjAXHwBBS12766870;     OVuGLyDxgjAXHwBBS12766870 = OVuGLyDxgjAXHwBBS58542081;     OVuGLyDxgjAXHwBBS58542081 = OVuGLyDxgjAXHwBBS85344686;     OVuGLyDxgjAXHwBBS85344686 = OVuGLyDxgjAXHwBBS87826152;     OVuGLyDxgjAXHwBBS87826152 = OVuGLyDxgjAXHwBBS48467588;     OVuGLyDxgjAXHwBBS48467588 = OVuGLyDxgjAXHwBBS81247317;     OVuGLyDxgjAXHwBBS81247317 = OVuGLyDxgjAXHwBBS5467375;     OVuGLyDxgjAXHwBBS5467375 = OVuGLyDxgjAXHwBBS70598036;     OVuGLyDxgjAXHwBBS70598036 = OVuGLyDxgjAXHwBBS54588361;     OVuGLyDxgjAXHwBBS54588361 = OVuGLyDxgjAXHwBBS52132338;     OVuGLyDxgjAXHwBBS52132338 = OVuGLyDxgjAXHwBBS97078270;     OVuGLyDxgjAXHwBBS97078270 = OVuGLyDxgjAXHwBBS33680229;     OVuGLyDxgjAXHwBBS33680229 = OVuGLyDxgjAXHwBBS97554430;     OVuGLyDxgjAXHwBBS97554430 = OVuGLyDxgjAXHwBBS29122564;     OVuGLyDxgjAXHwBBS29122564 = OVuGLyDxgjAXHwBBS42007620;     OVuGLyDxgjAXHwBBS42007620 = OVuGLyDxgjAXHwBBS55151399;     OVuGLyDxgjAXHwBBS55151399 = OVuGLyDxgjAXHwBBS30105282;     OVuGLyDxgjAXHwBBS30105282 = OVuGLyDxgjAXHwBBS2342830;     OVuGLyDxgjAXHwBBS2342830 = OVuGLyDxgjAXHwBBS92715370;     OVuGLyDxgjAXHwBBS92715370 = OVuGLyDxgjAXHwBBS87619109;     OVuGLyDxgjAXHwBBS87619109 = OVuGLyDxgjAXHwBBS39073486;     OVuGLyDxgjAXHwBBS39073486 = OVuGLyDxgjAXHwBBS81030960;     OVuGLyDxgjAXHwBBS81030960 = OVuGLyDxgjAXHwBBS8699962;     OVuGLyDxgjAXHwBBS8699962 = OVuGLyDxgjAXHwBBS41352734;     OVuGLyDxgjAXHwBBS41352734 = OVuGLyDxgjAXHwBBS42424933;     OVuGLyDxgjAXHwBBS42424933 = OVuGLyDxgjAXHwBBS82512553;     OVuGLyDxgjAXHwBBS82512553 = OVuGLyDxgjAXHwBBS80608602;     OVuGLyDxgjAXHwBBS80608602 = OVuGLyDxgjAXHwBBS57097825;     OVuGLyDxgjAXHwBBS57097825 = OVuGLyDxgjAXHwBBS54783886;     OVuGLyDxgjAXHwBBS54783886 = OVuGLyDxgjAXHwBBS90809689;     OVuGLyDxgjAXHwBBS90809689 = OVuGLyDxgjAXHwBBS6106198;     OVuGLyDxgjAXHwBBS6106198 = OVuGLyDxgjAXHwBBS66645406;     OVuGLyDxgjAXHwBBS66645406 = OVuGLyDxgjAXHwBBS46096334;     OVuGLyDxgjAXHwBBS46096334 = OVuGLyDxgjAXHwBBS7081088;     OVuGLyDxgjAXHwBBS7081088 = OVuGLyDxgjAXHwBBS84385433;     OVuGLyDxgjAXHwBBS84385433 = OVuGLyDxgjAXHwBBS92937220;     OVuGLyDxgjAXHwBBS92937220 = OVuGLyDxgjAXHwBBS34701636;     OVuGLyDxgjAXHwBBS34701636 = OVuGLyDxgjAXHwBBS92119821;     OVuGLyDxgjAXHwBBS92119821 = OVuGLyDxgjAXHwBBS95740586;     OVuGLyDxgjAXHwBBS95740586 = OVuGLyDxgjAXHwBBS76776600;     OVuGLyDxgjAXHwBBS76776600 = OVuGLyDxgjAXHwBBS14855734;     OVuGLyDxgjAXHwBBS14855734 = OVuGLyDxgjAXHwBBS67381105;     OVuGLyDxgjAXHwBBS67381105 = OVuGLyDxgjAXHwBBS97587047;     OVuGLyDxgjAXHwBBS97587047 = OVuGLyDxgjAXHwBBS18646388;     OVuGLyDxgjAXHwBBS18646388 = OVuGLyDxgjAXHwBBS48372441;     OVuGLyDxgjAXHwBBS48372441 = OVuGLyDxgjAXHwBBS99727128;     OVuGLyDxgjAXHwBBS99727128 = OVuGLyDxgjAXHwBBS13804841;     OVuGLyDxgjAXHwBBS13804841 = OVuGLyDxgjAXHwBBS65859163;     OVuGLyDxgjAXHwBBS65859163 = OVuGLyDxgjAXHwBBS69346467;     OVuGLyDxgjAXHwBBS69346467 = OVuGLyDxgjAXHwBBS69361707;     OVuGLyDxgjAXHwBBS69361707 = OVuGLyDxgjAXHwBBS15564210;     OVuGLyDxgjAXHwBBS15564210 = OVuGLyDxgjAXHwBBS68056650;     OVuGLyDxgjAXHwBBS68056650 = OVuGLyDxgjAXHwBBS34026459;     OVuGLyDxgjAXHwBBS34026459 = OVuGLyDxgjAXHwBBS14071044;     OVuGLyDxgjAXHwBBS14071044 = OVuGLyDxgjAXHwBBS6724369;     OVuGLyDxgjAXHwBBS6724369 = OVuGLyDxgjAXHwBBS31735910;     OVuGLyDxgjAXHwBBS31735910 = OVuGLyDxgjAXHwBBS49842120;     OVuGLyDxgjAXHwBBS49842120 = OVuGLyDxgjAXHwBBS43991952;     OVuGLyDxgjAXHwBBS43991952 = OVuGLyDxgjAXHwBBS45401219;     OVuGLyDxgjAXHwBBS45401219 = OVuGLyDxgjAXHwBBS65955035;     OVuGLyDxgjAXHwBBS65955035 = OVuGLyDxgjAXHwBBS75197720;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tmpTRJUHXPvycYwO8535990() {     double OMIzHnMiJKUfgoVfD81314827 = -4876031;    double OMIzHnMiJKUfgoVfD53424535 = -177534731;    double OMIzHnMiJKUfgoVfD62593188 = -976877261;    double OMIzHnMiJKUfgoVfD77814068 = -489183242;    double OMIzHnMiJKUfgoVfD76837707 = 85637676;    double OMIzHnMiJKUfgoVfD68469830 = -929734737;    double OMIzHnMiJKUfgoVfD93695334 = -175542130;    double OMIzHnMiJKUfgoVfD29238578 = -291667145;    double OMIzHnMiJKUfgoVfD51445940 = -275924229;    double OMIzHnMiJKUfgoVfD17061950 = -789201083;    double OMIzHnMiJKUfgoVfD53982115 = -652490197;    double OMIzHnMiJKUfgoVfD7413373 = 73433861;    double OMIzHnMiJKUfgoVfD54661030 = -267266419;    double OMIzHnMiJKUfgoVfD62440668 = -256912088;    double OMIzHnMiJKUfgoVfD32221608 = -890065033;    double OMIzHnMiJKUfgoVfD18011072 = -184383352;    double OMIzHnMiJKUfgoVfD87223239 = -698662080;    double OMIzHnMiJKUfgoVfD44839640 = -774933331;    double OMIzHnMiJKUfgoVfD14708785 = -382985871;    double OMIzHnMiJKUfgoVfD77702467 = -887227062;    double OMIzHnMiJKUfgoVfD40032495 = -99702611;    double OMIzHnMiJKUfgoVfD6567346 = -745685886;    double OMIzHnMiJKUfgoVfD15536766 = -881252549;    double OMIzHnMiJKUfgoVfD28200549 = -282661029;    double OMIzHnMiJKUfgoVfD1723567 = -545495757;    double OMIzHnMiJKUfgoVfD45044784 = -460110310;    double OMIzHnMiJKUfgoVfD36438920 = -541853778;    double OMIzHnMiJKUfgoVfD65282534 = -713650768;    double OMIzHnMiJKUfgoVfD36910981 = -118458790;    double OMIzHnMiJKUfgoVfD14066348 = -238326106;    double OMIzHnMiJKUfgoVfD25649906 = -439274835;    double OMIzHnMiJKUfgoVfD8969893 = -811640293;    double OMIzHnMiJKUfgoVfD4692676 = -378708740;    double OMIzHnMiJKUfgoVfD6184652 = -423900693;    double OMIzHnMiJKUfgoVfD85987758 = -801216366;    double OMIzHnMiJKUfgoVfD14395405 = -482678087;    double OMIzHnMiJKUfgoVfD51661415 = -386399372;    double OMIzHnMiJKUfgoVfD60832042 = -399613874;    double OMIzHnMiJKUfgoVfD75300407 = -195100784;    double OMIzHnMiJKUfgoVfD33299793 = -176103874;    double OMIzHnMiJKUfgoVfD18436094 = -931698333;    double OMIzHnMiJKUfgoVfD90692538 = 86504197;    double OMIzHnMiJKUfgoVfD63411664 = -848462223;    double OMIzHnMiJKUfgoVfD75233892 = -796306197;    double OMIzHnMiJKUfgoVfD60420053 = -123866763;    double OMIzHnMiJKUfgoVfD93186512 = -131849875;    double OMIzHnMiJKUfgoVfD23245950 = -442777051;    double OMIzHnMiJKUfgoVfD78554977 = -810930966;    double OMIzHnMiJKUfgoVfD64264090 = -631352639;    double OMIzHnMiJKUfgoVfD65043991 = -701658948;    double OMIzHnMiJKUfgoVfD32778949 = -311081700;    double OMIzHnMiJKUfgoVfD44489948 = -143052311;    double OMIzHnMiJKUfgoVfD45802888 = -230661342;    double OMIzHnMiJKUfgoVfD25091444 = -190732597;    double OMIzHnMiJKUfgoVfD68503495 = -630811246;    double OMIzHnMiJKUfgoVfD74747481 = -259190145;    double OMIzHnMiJKUfgoVfD37887770 = -296282183;    double OMIzHnMiJKUfgoVfD34392639 = -594216232;    double OMIzHnMiJKUfgoVfD76090501 = -943687486;    double OMIzHnMiJKUfgoVfD31792923 = -454252014;    double OMIzHnMiJKUfgoVfD32030911 = -287880959;    double OMIzHnMiJKUfgoVfD28412801 = -461891363;    double OMIzHnMiJKUfgoVfD92327596 = -73208355;    double OMIzHnMiJKUfgoVfD37379593 = 62401876;    double OMIzHnMiJKUfgoVfD91412043 = -249926249;    double OMIzHnMiJKUfgoVfD45012222 = -840849904;    double OMIzHnMiJKUfgoVfD2720697 = -547857400;    double OMIzHnMiJKUfgoVfD48476379 = -843365726;    double OMIzHnMiJKUfgoVfD76452910 = -455695723;    double OMIzHnMiJKUfgoVfD17826203 = -307386947;    double OMIzHnMiJKUfgoVfD66349656 = -797983981;    double OMIzHnMiJKUfgoVfD26391197 = -199048207;    double OMIzHnMiJKUfgoVfD69539233 = -479832548;    double OMIzHnMiJKUfgoVfD81408992 = -106881998;    double OMIzHnMiJKUfgoVfD59266374 = -955528730;    double OMIzHnMiJKUfgoVfD49339957 = -86206809;    double OMIzHnMiJKUfgoVfD43155681 = -897223664;    double OMIzHnMiJKUfgoVfD40302873 = 15053648;    double OMIzHnMiJKUfgoVfD67780495 = -58794267;    double OMIzHnMiJKUfgoVfD8537055 = -313645882;    double OMIzHnMiJKUfgoVfD21798835 = 82666741;    double OMIzHnMiJKUfgoVfD57883942 = -730922812;    double OMIzHnMiJKUfgoVfD1018445 = 17701871;    double OMIzHnMiJKUfgoVfD71866990 = -416799842;    double OMIzHnMiJKUfgoVfD81287399 = -927244406;    double OMIzHnMiJKUfgoVfD81159958 = -196222525;    double OMIzHnMiJKUfgoVfD63167005 = -480978952;    double OMIzHnMiJKUfgoVfD79601231 = -87976143;    double OMIzHnMiJKUfgoVfD37681157 = -793089448;    double OMIzHnMiJKUfgoVfD11240277 = -442026221;    double OMIzHnMiJKUfgoVfD76507635 = -86395905;    double OMIzHnMiJKUfgoVfD17268776 = -792183140;    double OMIzHnMiJKUfgoVfD84741540 = -455926388;    double OMIzHnMiJKUfgoVfD43507484 = -740848770;    double OMIzHnMiJKUfgoVfD1268882 = -888222915;    double OMIzHnMiJKUfgoVfD90023292 = -369806971;    double OMIzHnMiJKUfgoVfD98364941 = -840287449;    double OMIzHnMiJKUfgoVfD26032072 = -810864099;    double OMIzHnMiJKUfgoVfD83821849 = -446379949;    double OMIzHnMiJKUfgoVfD15407832 = -4876031;     OMIzHnMiJKUfgoVfD81314827 = OMIzHnMiJKUfgoVfD53424535;     OMIzHnMiJKUfgoVfD53424535 = OMIzHnMiJKUfgoVfD62593188;     OMIzHnMiJKUfgoVfD62593188 = OMIzHnMiJKUfgoVfD77814068;     OMIzHnMiJKUfgoVfD77814068 = OMIzHnMiJKUfgoVfD76837707;     OMIzHnMiJKUfgoVfD76837707 = OMIzHnMiJKUfgoVfD68469830;     OMIzHnMiJKUfgoVfD68469830 = OMIzHnMiJKUfgoVfD93695334;     OMIzHnMiJKUfgoVfD93695334 = OMIzHnMiJKUfgoVfD29238578;     OMIzHnMiJKUfgoVfD29238578 = OMIzHnMiJKUfgoVfD51445940;     OMIzHnMiJKUfgoVfD51445940 = OMIzHnMiJKUfgoVfD17061950;     OMIzHnMiJKUfgoVfD17061950 = OMIzHnMiJKUfgoVfD53982115;     OMIzHnMiJKUfgoVfD53982115 = OMIzHnMiJKUfgoVfD7413373;     OMIzHnMiJKUfgoVfD7413373 = OMIzHnMiJKUfgoVfD54661030;     OMIzHnMiJKUfgoVfD54661030 = OMIzHnMiJKUfgoVfD62440668;     OMIzHnMiJKUfgoVfD62440668 = OMIzHnMiJKUfgoVfD32221608;     OMIzHnMiJKUfgoVfD32221608 = OMIzHnMiJKUfgoVfD18011072;     OMIzHnMiJKUfgoVfD18011072 = OMIzHnMiJKUfgoVfD87223239;     OMIzHnMiJKUfgoVfD87223239 = OMIzHnMiJKUfgoVfD44839640;     OMIzHnMiJKUfgoVfD44839640 = OMIzHnMiJKUfgoVfD14708785;     OMIzHnMiJKUfgoVfD14708785 = OMIzHnMiJKUfgoVfD77702467;     OMIzHnMiJKUfgoVfD77702467 = OMIzHnMiJKUfgoVfD40032495;     OMIzHnMiJKUfgoVfD40032495 = OMIzHnMiJKUfgoVfD6567346;     OMIzHnMiJKUfgoVfD6567346 = OMIzHnMiJKUfgoVfD15536766;     OMIzHnMiJKUfgoVfD15536766 = OMIzHnMiJKUfgoVfD28200549;     OMIzHnMiJKUfgoVfD28200549 = OMIzHnMiJKUfgoVfD1723567;     OMIzHnMiJKUfgoVfD1723567 = OMIzHnMiJKUfgoVfD45044784;     OMIzHnMiJKUfgoVfD45044784 = OMIzHnMiJKUfgoVfD36438920;     OMIzHnMiJKUfgoVfD36438920 = OMIzHnMiJKUfgoVfD65282534;     OMIzHnMiJKUfgoVfD65282534 = OMIzHnMiJKUfgoVfD36910981;     OMIzHnMiJKUfgoVfD36910981 = OMIzHnMiJKUfgoVfD14066348;     OMIzHnMiJKUfgoVfD14066348 = OMIzHnMiJKUfgoVfD25649906;     OMIzHnMiJKUfgoVfD25649906 = OMIzHnMiJKUfgoVfD8969893;     OMIzHnMiJKUfgoVfD8969893 = OMIzHnMiJKUfgoVfD4692676;     OMIzHnMiJKUfgoVfD4692676 = OMIzHnMiJKUfgoVfD6184652;     OMIzHnMiJKUfgoVfD6184652 = OMIzHnMiJKUfgoVfD85987758;     OMIzHnMiJKUfgoVfD85987758 = OMIzHnMiJKUfgoVfD14395405;     OMIzHnMiJKUfgoVfD14395405 = OMIzHnMiJKUfgoVfD51661415;     OMIzHnMiJKUfgoVfD51661415 = OMIzHnMiJKUfgoVfD60832042;     OMIzHnMiJKUfgoVfD60832042 = OMIzHnMiJKUfgoVfD75300407;     OMIzHnMiJKUfgoVfD75300407 = OMIzHnMiJKUfgoVfD33299793;     OMIzHnMiJKUfgoVfD33299793 = OMIzHnMiJKUfgoVfD18436094;     OMIzHnMiJKUfgoVfD18436094 = OMIzHnMiJKUfgoVfD90692538;     OMIzHnMiJKUfgoVfD90692538 = OMIzHnMiJKUfgoVfD63411664;     OMIzHnMiJKUfgoVfD63411664 = OMIzHnMiJKUfgoVfD75233892;     OMIzHnMiJKUfgoVfD75233892 = OMIzHnMiJKUfgoVfD60420053;     OMIzHnMiJKUfgoVfD60420053 = OMIzHnMiJKUfgoVfD93186512;     OMIzHnMiJKUfgoVfD93186512 = OMIzHnMiJKUfgoVfD23245950;     OMIzHnMiJKUfgoVfD23245950 = OMIzHnMiJKUfgoVfD78554977;     OMIzHnMiJKUfgoVfD78554977 = OMIzHnMiJKUfgoVfD64264090;     OMIzHnMiJKUfgoVfD64264090 = OMIzHnMiJKUfgoVfD65043991;     OMIzHnMiJKUfgoVfD65043991 = OMIzHnMiJKUfgoVfD32778949;     OMIzHnMiJKUfgoVfD32778949 = OMIzHnMiJKUfgoVfD44489948;     OMIzHnMiJKUfgoVfD44489948 = OMIzHnMiJKUfgoVfD45802888;     OMIzHnMiJKUfgoVfD45802888 = OMIzHnMiJKUfgoVfD25091444;     OMIzHnMiJKUfgoVfD25091444 = OMIzHnMiJKUfgoVfD68503495;     OMIzHnMiJKUfgoVfD68503495 = OMIzHnMiJKUfgoVfD74747481;     OMIzHnMiJKUfgoVfD74747481 = OMIzHnMiJKUfgoVfD37887770;     OMIzHnMiJKUfgoVfD37887770 = OMIzHnMiJKUfgoVfD34392639;     OMIzHnMiJKUfgoVfD34392639 = OMIzHnMiJKUfgoVfD76090501;     OMIzHnMiJKUfgoVfD76090501 = OMIzHnMiJKUfgoVfD31792923;     OMIzHnMiJKUfgoVfD31792923 = OMIzHnMiJKUfgoVfD32030911;     OMIzHnMiJKUfgoVfD32030911 = OMIzHnMiJKUfgoVfD28412801;     OMIzHnMiJKUfgoVfD28412801 = OMIzHnMiJKUfgoVfD92327596;     OMIzHnMiJKUfgoVfD92327596 = OMIzHnMiJKUfgoVfD37379593;     OMIzHnMiJKUfgoVfD37379593 = OMIzHnMiJKUfgoVfD91412043;     OMIzHnMiJKUfgoVfD91412043 = OMIzHnMiJKUfgoVfD45012222;     OMIzHnMiJKUfgoVfD45012222 = OMIzHnMiJKUfgoVfD2720697;     OMIzHnMiJKUfgoVfD2720697 = OMIzHnMiJKUfgoVfD48476379;     OMIzHnMiJKUfgoVfD48476379 = OMIzHnMiJKUfgoVfD76452910;     OMIzHnMiJKUfgoVfD76452910 = OMIzHnMiJKUfgoVfD17826203;     OMIzHnMiJKUfgoVfD17826203 = OMIzHnMiJKUfgoVfD66349656;     OMIzHnMiJKUfgoVfD66349656 = OMIzHnMiJKUfgoVfD26391197;     OMIzHnMiJKUfgoVfD26391197 = OMIzHnMiJKUfgoVfD69539233;     OMIzHnMiJKUfgoVfD69539233 = OMIzHnMiJKUfgoVfD81408992;     OMIzHnMiJKUfgoVfD81408992 = OMIzHnMiJKUfgoVfD59266374;     OMIzHnMiJKUfgoVfD59266374 = OMIzHnMiJKUfgoVfD49339957;     OMIzHnMiJKUfgoVfD49339957 = OMIzHnMiJKUfgoVfD43155681;     OMIzHnMiJKUfgoVfD43155681 = OMIzHnMiJKUfgoVfD40302873;     OMIzHnMiJKUfgoVfD40302873 = OMIzHnMiJKUfgoVfD67780495;     OMIzHnMiJKUfgoVfD67780495 = OMIzHnMiJKUfgoVfD8537055;     OMIzHnMiJKUfgoVfD8537055 = OMIzHnMiJKUfgoVfD21798835;     OMIzHnMiJKUfgoVfD21798835 = OMIzHnMiJKUfgoVfD57883942;     OMIzHnMiJKUfgoVfD57883942 = OMIzHnMiJKUfgoVfD1018445;     OMIzHnMiJKUfgoVfD1018445 = OMIzHnMiJKUfgoVfD71866990;     OMIzHnMiJKUfgoVfD71866990 = OMIzHnMiJKUfgoVfD81287399;     OMIzHnMiJKUfgoVfD81287399 = OMIzHnMiJKUfgoVfD81159958;     OMIzHnMiJKUfgoVfD81159958 = OMIzHnMiJKUfgoVfD63167005;     OMIzHnMiJKUfgoVfD63167005 = OMIzHnMiJKUfgoVfD79601231;     OMIzHnMiJKUfgoVfD79601231 = OMIzHnMiJKUfgoVfD37681157;     OMIzHnMiJKUfgoVfD37681157 = OMIzHnMiJKUfgoVfD11240277;     OMIzHnMiJKUfgoVfD11240277 = OMIzHnMiJKUfgoVfD76507635;     OMIzHnMiJKUfgoVfD76507635 = OMIzHnMiJKUfgoVfD17268776;     OMIzHnMiJKUfgoVfD17268776 = OMIzHnMiJKUfgoVfD84741540;     OMIzHnMiJKUfgoVfD84741540 = OMIzHnMiJKUfgoVfD43507484;     OMIzHnMiJKUfgoVfD43507484 = OMIzHnMiJKUfgoVfD1268882;     OMIzHnMiJKUfgoVfD1268882 = OMIzHnMiJKUfgoVfD90023292;     OMIzHnMiJKUfgoVfD90023292 = OMIzHnMiJKUfgoVfD98364941;     OMIzHnMiJKUfgoVfD98364941 = OMIzHnMiJKUfgoVfD26032072;     OMIzHnMiJKUfgoVfD26032072 = OMIzHnMiJKUfgoVfD83821849;     OMIzHnMiJKUfgoVfD83821849 = OMIzHnMiJKUfgoVfD15407832;     OMIzHnMiJKUfgoVfD15407832 = OMIzHnMiJKUfgoVfD81314827;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void iKIbfxmYyHRWVmkB45180964() {     double BxXXdbHbvFnuHjLRp3701945 = -739279994;    double BxXXdbHbvFnuHjLRp37772035 = -126077786;    double BxXXdbHbvFnuHjLRp91362466 = -116531105;    double BxXXdbHbvFnuHjLRp92820756 = -495590773;    double BxXXdbHbvFnuHjLRp75011961 = -505419325;    double BxXXdbHbvFnuHjLRp83352947 = -790343115;    double BxXXdbHbvFnuHjLRp85851073 = 2470564;    double BxXXdbHbvFnuHjLRp4872437 = -762263789;    double BxXXdbHbvFnuHjLRp48609071 = -160120269;    double BxXXdbHbvFnuHjLRp8825535 = -531942416;    double BxXXdbHbvFnuHjLRp91523974 = -649114598;    double BxXXdbHbvFnuHjLRp63316321 = -177311432;    double BxXXdbHbvFnuHjLRp21669211 = -885983739;    double BxXXdbHbvFnuHjLRp5055421 = -784620845;    double BxXXdbHbvFnuHjLRp68956884 = -517542746;    double BxXXdbHbvFnuHjLRp62266984 = -566050895;    double BxXXdbHbvFnuHjLRp37046591 = -975941900;    double BxXXdbHbvFnuHjLRp50816056 = -126676022;    double BxXXdbHbvFnuHjLRp55990415 = -518087997;    double BxXXdbHbvFnuHjLRp56218751 = -39861516;    double BxXXdbHbvFnuHjLRp40915602 = -91189920;    double BxXXdbHbvFnuHjLRp70382564 = -150452199;    double BxXXdbHbvFnuHjLRp18830089 = 42666859;    double BxXXdbHbvFnuHjLRp54700336 = -846653886;    double BxXXdbHbvFnuHjLRp37263814 = -167054881;    double BxXXdbHbvFnuHjLRp33347779 = -904004534;    double BxXXdbHbvFnuHjLRp83528340 = -299440271;    double BxXXdbHbvFnuHjLRp71193984 = -502105222;    double BxXXdbHbvFnuHjLRp43803542 = -462715910;    double BxXXdbHbvFnuHjLRp23223527 = -158621973;    double BxXXdbHbvFnuHjLRp69940711 = -520590987;    double BxXXdbHbvFnuHjLRp66511083 = -363711648;    double BxXXdbHbvFnuHjLRp85731346 = -908052234;    double BxXXdbHbvFnuHjLRp65573338 = -748027543;    double BxXXdbHbvFnuHjLRp62217123 = -864972789;    double BxXXdbHbvFnuHjLRp56088274 = -798073464;    double BxXXdbHbvFnuHjLRp45101127 = -119347324;    double BxXXdbHbvFnuHjLRp14458143 = -267239345;    double BxXXdbHbvFnuHjLRp23608539 = -596146816;    double BxXXdbHbvFnuHjLRp53485507 = -312327904;    double BxXXdbHbvFnuHjLRp47193464 = -334875999;    double BxXXdbHbvFnuHjLRp79200929 = -771404494;    double BxXXdbHbvFnuHjLRp68961856 = -735992806;    double BxXXdbHbvFnuHjLRp74678011 = -793669846;    double BxXXdbHbvFnuHjLRp28403320 = 66989760;    double BxXXdbHbvFnuHjLRp1286794 = -314078971;    double BxXXdbHbvFnuHjLRp35543347 = 78812696;    double BxXXdbHbvFnuHjLRp23153652 = -567465957;    double BxXXdbHbvFnuHjLRp58723995 = -581069338;    double BxXXdbHbvFnuHjLRp62127736 = -843679207;    double BxXXdbHbvFnuHjLRp41648165 = -666157977;    double BxXXdbHbvFnuHjLRp18341206 = -573871593;    double BxXXdbHbvFnuHjLRp24117291 = 97427428;    double BxXXdbHbvFnuHjLRp85377989 = -466413406;    double BxXXdbHbvFnuHjLRp4265363 = -771515361;    double BxXXdbHbvFnuHjLRp33319381 = -488827795;    double BxXXdbHbvFnuHjLRp18941947 = -68744645;    double BxXXdbHbvFnuHjLRp36662131 = -269877219;    double BxXXdbHbvFnuHjLRp55556943 = -228535893;    double BxXXdbHbvFnuHjLRp41664182 = -601414792;    double BxXXdbHbvFnuHjLRp99824607 = -390902844;    double BxXXdbHbvFnuHjLRp14657090 = -495424215;    double BxXXdbHbvFnuHjLRp61068894 = -199547880;    double BxXXdbHbvFnuHjLRp25385544 = 98501704;    double BxXXdbHbvFnuHjLRp38884823 = 88648570;    double BxXXdbHbvFnuHjLRp25012891 = -185402951;    double BxXXdbHbvFnuHjLRp77584975 = -269259198;    double BxXXdbHbvFnuHjLRp56095873 = -37956196;    double BxXXdbHbvFnuHjLRp42838298 = -919648057;    double BxXXdbHbvFnuHjLRp12868611 = -719469283;    double BxXXdbHbvFnuHjLRp17165857 = -346703572;    double BxXXdbHbvFnuHjLRp22588448 = -608702555;    double BxXXdbHbvFnuHjLRp27207518 = -530529207;    double BxXXdbHbvFnuHjLRp2504909 = -105760093;    double BxXXdbHbvFnuHjLRp9025287 = -704985517;    double BxXXdbHbvFnuHjLRp61714673 = -319785426;    double BxXXdbHbvFnuHjLRp1420708 = -414459393;    double BxXXdbHbvFnuHjLRp44152077 = -163663295;    double BxXXdbHbvFnuHjLRp26297016 = -813643647;    double BxXXdbHbvFnuHjLRp35977020 = -852975911;    double BxXXdbHbvFnuHjLRp97804432 = -882817230;    double BxXXdbHbvFnuHjLRp60374688 = -731974314;    double BxXXdbHbvFnuHjLRp12469990 = -921035885;    double BxXXdbHbvFnuHjLRp81675805 = -619036703;    double BxXXdbHbvFnuHjLRp81575362 = -492463996;    double BxXXdbHbvFnuHjLRp51599506 = -946719395;    double BxXXdbHbvFnuHjLRp42393793 = -361139076;    double BxXXdbHbvFnuHjLRp353357 = -341638829;    double BxXXdbHbvFnuHjLRp61307976 = -976512183;    double BxXXdbHbvFnuHjLRp28897742 = -276144994;    double BxXXdbHbvFnuHjLRp37146327 = -629328819;    double BxXXdbHbvFnuHjLRp8438997 = -849470105;    double BxXXdbHbvFnuHjLRp58901200 = 61296547;    double BxXXdbHbvFnuHjLRp81944356 = -994732024;    double BxXXdbHbvFnuHjLRp53660900 = -921425060;    double BxXXdbHbvFnuHjLRp32536375 = -839451784;    double BxXXdbHbvFnuHjLRp18132035 = -471856615;    double BxXXdbHbvFnuHjLRp43576312 = -734494510;    double BxXXdbHbvFnuHjLRp35793189 = -782318417;    double BxXXdbHbvFnuHjLRp3390430 = -739279994;     BxXXdbHbvFnuHjLRp3701945 = BxXXdbHbvFnuHjLRp37772035;     BxXXdbHbvFnuHjLRp37772035 = BxXXdbHbvFnuHjLRp91362466;     BxXXdbHbvFnuHjLRp91362466 = BxXXdbHbvFnuHjLRp92820756;     BxXXdbHbvFnuHjLRp92820756 = BxXXdbHbvFnuHjLRp75011961;     BxXXdbHbvFnuHjLRp75011961 = BxXXdbHbvFnuHjLRp83352947;     BxXXdbHbvFnuHjLRp83352947 = BxXXdbHbvFnuHjLRp85851073;     BxXXdbHbvFnuHjLRp85851073 = BxXXdbHbvFnuHjLRp4872437;     BxXXdbHbvFnuHjLRp4872437 = BxXXdbHbvFnuHjLRp48609071;     BxXXdbHbvFnuHjLRp48609071 = BxXXdbHbvFnuHjLRp8825535;     BxXXdbHbvFnuHjLRp8825535 = BxXXdbHbvFnuHjLRp91523974;     BxXXdbHbvFnuHjLRp91523974 = BxXXdbHbvFnuHjLRp63316321;     BxXXdbHbvFnuHjLRp63316321 = BxXXdbHbvFnuHjLRp21669211;     BxXXdbHbvFnuHjLRp21669211 = BxXXdbHbvFnuHjLRp5055421;     BxXXdbHbvFnuHjLRp5055421 = BxXXdbHbvFnuHjLRp68956884;     BxXXdbHbvFnuHjLRp68956884 = BxXXdbHbvFnuHjLRp62266984;     BxXXdbHbvFnuHjLRp62266984 = BxXXdbHbvFnuHjLRp37046591;     BxXXdbHbvFnuHjLRp37046591 = BxXXdbHbvFnuHjLRp50816056;     BxXXdbHbvFnuHjLRp50816056 = BxXXdbHbvFnuHjLRp55990415;     BxXXdbHbvFnuHjLRp55990415 = BxXXdbHbvFnuHjLRp56218751;     BxXXdbHbvFnuHjLRp56218751 = BxXXdbHbvFnuHjLRp40915602;     BxXXdbHbvFnuHjLRp40915602 = BxXXdbHbvFnuHjLRp70382564;     BxXXdbHbvFnuHjLRp70382564 = BxXXdbHbvFnuHjLRp18830089;     BxXXdbHbvFnuHjLRp18830089 = BxXXdbHbvFnuHjLRp54700336;     BxXXdbHbvFnuHjLRp54700336 = BxXXdbHbvFnuHjLRp37263814;     BxXXdbHbvFnuHjLRp37263814 = BxXXdbHbvFnuHjLRp33347779;     BxXXdbHbvFnuHjLRp33347779 = BxXXdbHbvFnuHjLRp83528340;     BxXXdbHbvFnuHjLRp83528340 = BxXXdbHbvFnuHjLRp71193984;     BxXXdbHbvFnuHjLRp71193984 = BxXXdbHbvFnuHjLRp43803542;     BxXXdbHbvFnuHjLRp43803542 = BxXXdbHbvFnuHjLRp23223527;     BxXXdbHbvFnuHjLRp23223527 = BxXXdbHbvFnuHjLRp69940711;     BxXXdbHbvFnuHjLRp69940711 = BxXXdbHbvFnuHjLRp66511083;     BxXXdbHbvFnuHjLRp66511083 = BxXXdbHbvFnuHjLRp85731346;     BxXXdbHbvFnuHjLRp85731346 = BxXXdbHbvFnuHjLRp65573338;     BxXXdbHbvFnuHjLRp65573338 = BxXXdbHbvFnuHjLRp62217123;     BxXXdbHbvFnuHjLRp62217123 = BxXXdbHbvFnuHjLRp56088274;     BxXXdbHbvFnuHjLRp56088274 = BxXXdbHbvFnuHjLRp45101127;     BxXXdbHbvFnuHjLRp45101127 = BxXXdbHbvFnuHjLRp14458143;     BxXXdbHbvFnuHjLRp14458143 = BxXXdbHbvFnuHjLRp23608539;     BxXXdbHbvFnuHjLRp23608539 = BxXXdbHbvFnuHjLRp53485507;     BxXXdbHbvFnuHjLRp53485507 = BxXXdbHbvFnuHjLRp47193464;     BxXXdbHbvFnuHjLRp47193464 = BxXXdbHbvFnuHjLRp79200929;     BxXXdbHbvFnuHjLRp79200929 = BxXXdbHbvFnuHjLRp68961856;     BxXXdbHbvFnuHjLRp68961856 = BxXXdbHbvFnuHjLRp74678011;     BxXXdbHbvFnuHjLRp74678011 = BxXXdbHbvFnuHjLRp28403320;     BxXXdbHbvFnuHjLRp28403320 = BxXXdbHbvFnuHjLRp1286794;     BxXXdbHbvFnuHjLRp1286794 = BxXXdbHbvFnuHjLRp35543347;     BxXXdbHbvFnuHjLRp35543347 = BxXXdbHbvFnuHjLRp23153652;     BxXXdbHbvFnuHjLRp23153652 = BxXXdbHbvFnuHjLRp58723995;     BxXXdbHbvFnuHjLRp58723995 = BxXXdbHbvFnuHjLRp62127736;     BxXXdbHbvFnuHjLRp62127736 = BxXXdbHbvFnuHjLRp41648165;     BxXXdbHbvFnuHjLRp41648165 = BxXXdbHbvFnuHjLRp18341206;     BxXXdbHbvFnuHjLRp18341206 = BxXXdbHbvFnuHjLRp24117291;     BxXXdbHbvFnuHjLRp24117291 = BxXXdbHbvFnuHjLRp85377989;     BxXXdbHbvFnuHjLRp85377989 = BxXXdbHbvFnuHjLRp4265363;     BxXXdbHbvFnuHjLRp4265363 = BxXXdbHbvFnuHjLRp33319381;     BxXXdbHbvFnuHjLRp33319381 = BxXXdbHbvFnuHjLRp18941947;     BxXXdbHbvFnuHjLRp18941947 = BxXXdbHbvFnuHjLRp36662131;     BxXXdbHbvFnuHjLRp36662131 = BxXXdbHbvFnuHjLRp55556943;     BxXXdbHbvFnuHjLRp55556943 = BxXXdbHbvFnuHjLRp41664182;     BxXXdbHbvFnuHjLRp41664182 = BxXXdbHbvFnuHjLRp99824607;     BxXXdbHbvFnuHjLRp99824607 = BxXXdbHbvFnuHjLRp14657090;     BxXXdbHbvFnuHjLRp14657090 = BxXXdbHbvFnuHjLRp61068894;     BxXXdbHbvFnuHjLRp61068894 = BxXXdbHbvFnuHjLRp25385544;     BxXXdbHbvFnuHjLRp25385544 = BxXXdbHbvFnuHjLRp38884823;     BxXXdbHbvFnuHjLRp38884823 = BxXXdbHbvFnuHjLRp25012891;     BxXXdbHbvFnuHjLRp25012891 = BxXXdbHbvFnuHjLRp77584975;     BxXXdbHbvFnuHjLRp77584975 = BxXXdbHbvFnuHjLRp56095873;     BxXXdbHbvFnuHjLRp56095873 = BxXXdbHbvFnuHjLRp42838298;     BxXXdbHbvFnuHjLRp42838298 = BxXXdbHbvFnuHjLRp12868611;     BxXXdbHbvFnuHjLRp12868611 = BxXXdbHbvFnuHjLRp17165857;     BxXXdbHbvFnuHjLRp17165857 = BxXXdbHbvFnuHjLRp22588448;     BxXXdbHbvFnuHjLRp22588448 = BxXXdbHbvFnuHjLRp27207518;     BxXXdbHbvFnuHjLRp27207518 = BxXXdbHbvFnuHjLRp2504909;     BxXXdbHbvFnuHjLRp2504909 = BxXXdbHbvFnuHjLRp9025287;     BxXXdbHbvFnuHjLRp9025287 = BxXXdbHbvFnuHjLRp61714673;     BxXXdbHbvFnuHjLRp61714673 = BxXXdbHbvFnuHjLRp1420708;     BxXXdbHbvFnuHjLRp1420708 = BxXXdbHbvFnuHjLRp44152077;     BxXXdbHbvFnuHjLRp44152077 = BxXXdbHbvFnuHjLRp26297016;     BxXXdbHbvFnuHjLRp26297016 = BxXXdbHbvFnuHjLRp35977020;     BxXXdbHbvFnuHjLRp35977020 = BxXXdbHbvFnuHjLRp97804432;     BxXXdbHbvFnuHjLRp97804432 = BxXXdbHbvFnuHjLRp60374688;     BxXXdbHbvFnuHjLRp60374688 = BxXXdbHbvFnuHjLRp12469990;     BxXXdbHbvFnuHjLRp12469990 = BxXXdbHbvFnuHjLRp81675805;     BxXXdbHbvFnuHjLRp81675805 = BxXXdbHbvFnuHjLRp81575362;     BxXXdbHbvFnuHjLRp81575362 = BxXXdbHbvFnuHjLRp51599506;     BxXXdbHbvFnuHjLRp51599506 = BxXXdbHbvFnuHjLRp42393793;     BxXXdbHbvFnuHjLRp42393793 = BxXXdbHbvFnuHjLRp353357;     BxXXdbHbvFnuHjLRp353357 = BxXXdbHbvFnuHjLRp61307976;     BxXXdbHbvFnuHjLRp61307976 = BxXXdbHbvFnuHjLRp28897742;     BxXXdbHbvFnuHjLRp28897742 = BxXXdbHbvFnuHjLRp37146327;     BxXXdbHbvFnuHjLRp37146327 = BxXXdbHbvFnuHjLRp8438997;     BxXXdbHbvFnuHjLRp8438997 = BxXXdbHbvFnuHjLRp58901200;     BxXXdbHbvFnuHjLRp58901200 = BxXXdbHbvFnuHjLRp81944356;     BxXXdbHbvFnuHjLRp81944356 = BxXXdbHbvFnuHjLRp53660900;     BxXXdbHbvFnuHjLRp53660900 = BxXXdbHbvFnuHjLRp32536375;     BxXXdbHbvFnuHjLRp32536375 = BxXXdbHbvFnuHjLRp18132035;     BxXXdbHbvFnuHjLRp18132035 = BxXXdbHbvFnuHjLRp43576312;     BxXXdbHbvFnuHjLRp43576312 = BxXXdbHbvFnuHjLRp35793189;     BxXXdbHbvFnuHjLRp35793189 = BxXXdbHbvFnuHjLRp3390430;     BxXXdbHbvFnuHjLRp3390430 = BxXXdbHbvFnuHjLRp3701945;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void COrJsTsBFhsSHffl12472564() {     double WshgTRAtqSFOrBoWG39160913 = -105556894;    double WshgTRAtqSFOrBoWG51044114 = -6629392;    double WshgTRAtqSFOrBoWG28748944 = -882198180;    double WshgTRAtqSFOrBoWG65044112 = -316741845;    double WshgTRAtqSFOrBoWG31297230 = -611018170;    double WshgTRAtqSFOrBoWG19575820 = 47308990;    double WshgTRAtqSFOrBoWG96508479 = -942255171;    double WshgTRAtqSFOrBoWG67003997 = -406497929;    double WshgTRAtqSFOrBoWG50126676 = -648451675;    double WshgTRAtqSFOrBoWG23728247 = -196629414;    double WshgTRAtqSFOrBoWG29705709 = -451122327;    double WshgTRAtqSFOrBoWG63884470 = -41432203;    double WshgTRAtqSFOrBoWG59903856 = -337996244;    double WshgTRAtqSFOrBoWG58196284 = -760984255;    double WshgTRAtqSFOrBoWG27151401 = 79032239;    double WshgTRAtqSFOrBoWG45808285 = -201862502;    double WshgTRAtqSFOrBoWG92882526 = -402916453;    double WshgTRAtqSFOrBoWG50576248 = -200104091;    double WshgTRAtqSFOrBoWG56540745 = -834651418;    double WshgTRAtqSFOrBoWG50055969 = -466422582;    double WshgTRAtqSFOrBoWG54002465 = -848464609;    double WshgTRAtqSFOrBoWG73804088 = -237088493;    double WshgTRAtqSFOrBoWG89355468 = -694965325;    double WshgTRAtqSFOrBoWG48493281 = -257478490;    double WshgTRAtqSFOrBoWG45281765 = -191040079;    double WshgTRAtqSFOrBoWG44045950 = -592581340;    double WshgTRAtqSFOrBoWG43948915 = -105724412;    double WshgTRAtqSFOrBoWG11922410 = -671877396;    double WshgTRAtqSFOrBoWG90471756 = 63189288;    double WshgTRAtqSFOrBoWG28756938 = -193129143;    double WshgTRAtqSFOrBoWG88523943 = 27428526;    double WshgTRAtqSFOrBoWG4334102 = -136683062;    double WshgTRAtqSFOrBoWG97949357 = 44351788;    double WshgTRAtqSFOrBoWG95321764 = -812721357;    double WshgTRAtqSFOrBoWG28025577 = -816073039;    double WshgTRAtqSFOrBoWG7649118 = 87361231;    double WshgTRAtqSFOrBoWG94512943 = 15956636;    double WshgTRAtqSFOrBoWG65673546 = -249894927;    double WshgTRAtqSFOrBoWG53999100 = -247023092;    double WshgTRAtqSFOrBoWG17757040 = -877590450;    double WshgTRAtqSFOrBoWG80645072 = -104074913;    double WshgTRAtqSFOrBoWG68566056 = -365949785;    double WshgTRAtqSFOrBoWG58345235 = -289173592;    double WshgTRAtqSFOrBoWG14809350 = -298002256;    double WshgTRAtqSFOrBoWG28419779 = -859521402;    double WshgTRAtqSFOrBoWG47248122 = -353002603;    double WshgTRAtqSFOrBoWG54888510 = -405173664;    double WshgTRAtqSFOrBoWG40879964 = -733992551;    double WshgTRAtqSFOrBoWG81616661 = -244806218;    double WshgTRAtqSFOrBoWG86657325 = -234706023;    double WshgTRAtqSFOrBoWG22418889 = -540434535;    double WshgTRAtqSFOrBoWG1826678 = -787523939;    double WshgTRAtqSFOrBoWG92738439 = -758618126;    double WshgTRAtqSFOrBoWG1369836 = -657921906;    double WshgTRAtqSFOrBoWG58470098 = -18005507;    double WshgTRAtqSFOrBoWG65356824 = -868468401;    double WshgTRAtqSFOrBoWG61688645 = -311664068;    double WshgTRAtqSFOrBoWG80255662 = -524719690;    double WshgTRAtqSFOrBoWG19762347 = -25701766;    double WshgTRAtqSFOrBoWG87251279 = 81563170;    double WshgTRAtqSFOrBoWG75626905 = -846966599;    double WshgTRAtqSFOrBoWG84586070 = -170377775;    double WshgTRAtqSFOrBoWG76532241 = -369687218;    double WshgTRAtqSFOrBoWG21369739 = -355322533;    double WshgTRAtqSFOrBoWG35204303 = -124057941;    double WshgTRAtqSFOrBoWG25371607 = -214439266;    double WshgTRAtqSFOrBoWG65935113 = 14216008;    double WshgTRAtqSFOrBoWG64582091 = -525274887;    double WshgTRAtqSFOrBoWG30170708 = -944911216;    double WshgTRAtqSFOrBoWG19502284 = 91671008;    double WshgTRAtqSFOrBoWG51295341 = -117819138;    double WshgTRAtqSFOrBoWG27208981 = -53021526;    double WshgTRAtqSFOrBoWG96577147 = -953080999;    double WshgTRAtqSFOrBoWG38783706 = -957060969;    double WshgTRAtqSFOrBoWG69410897 = -262347669;    double WshgTRAtqSFOrBoWG85436409 = -382514824;    double WshgTRAtqSFOrBoWG15458854 = -947914901;    double WshgTRAtqSFOrBoWG74546118 = -296963070;    double WshgTRAtqSFOrBoWG20073503 = -397957089;    double WshgTRAtqSFOrBoWG98033642 = -838037477;    double WshgTRAtqSFOrBoWG89157440 = -87407677;    double WshgTRAtqSFOrBoWG3068951 = -371731862;    double WshgTRAtqSFOrBoWG30305748 = -327071179;    double WshgTRAtqSFOrBoWG3814431 = -702104689;    double WshgTRAtqSFOrBoWG6338050 = -652694609;    double WshgTRAtqSFOrBoWG86697265 = -185047536;    double WshgTRAtqSFOrBoWG11595663 = -378064937;    double WshgTRAtqSFOrBoWG96579521 = -297726307;    double WshgTRAtqSFOrBoWG36851667 = -694715851;    double WshgTRAtqSFOrBoWG62668752 = -947604639;    double WshgTRAtqSFOrBoWG45960472 = -600974702;    double WshgTRAtqSFOrBoWG14257282 = -459323674;    double WshgTRAtqSFOrBoWG45911200 = -124193162;    double WshgTRAtqSFOrBoWG66747820 = -228586262;    double WshgTRAtqSFOrBoWG42130134 = 69376149;    double WshgTRAtqSFOrBoWG96059001 = -933697139;    double WshgTRAtqSFOrBoWG92033815 = -996262568;    double WshgTRAtqSFOrBoWG36975496 = -933851060;    double WshgTRAtqSFOrBoWG79605046 = -73944316;    double WshgTRAtqSFOrBoWG3048172 = -105556894;     WshgTRAtqSFOrBoWG39160913 = WshgTRAtqSFOrBoWG51044114;     WshgTRAtqSFOrBoWG51044114 = WshgTRAtqSFOrBoWG28748944;     WshgTRAtqSFOrBoWG28748944 = WshgTRAtqSFOrBoWG65044112;     WshgTRAtqSFOrBoWG65044112 = WshgTRAtqSFOrBoWG31297230;     WshgTRAtqSFOrBoWG31297230 = WshgTRAtqSFOrBoWG19575820;     WshgTRAtqSFOrBoWG19575820 = WshgTRAtqSFOrBoWG96508479;     WshgTRAtqSFOrBoWG96508479 = WshgTRAtqSFOrBoWG67003997;     WshgTRAtqSFOrBoWG67003997 = WshgTRAtqSFOrBoWG50126676;     WshgTRAtqSFOrBoWG50126676 = WshgTRAtqSFOrBoWG23728247;     WshgTRAtqSFOrBoWG23728247 = WshgTRAtqSFOrBoWG29705709;     WshgTRAtqSFOrBoWG29705709 = WshgTRAtqSFOrBoWG63884470;     WshgTRAtqSFOrBoWG63884470 = WshgTRAtqSFOrBoWG59903856;     WshgTRAtqSFOrBoWG59903856 = WshgTRAtqSFOrBoWG58196284;     WshgTRAtqSFOrBoWG58196284 = WshgTRAtqSFOrBoWG27151401;     WshgTRAtqSFOrBoWG27151401 = WshgTRAtqSFOrBoWG45808285;     WshgTRAtqSFOrBoWG45808285 = WshgTRAtqSFOrBoWG92882526;     WshgTRAtqSFOrBoWG92882526 = WshgTRAtqSFOrBoWG50576248;     WshgTRAtqSFOrBoWG50576248 = WshgTRAtqSFOrBoWG56540745;     WshgTRAtqSFOrBoWG56540745 = WshgTRAtqSFOrBoWG50055969;     WshgTRAtqSFOrBoWG50055969 = WshgTRAtqSFOrBoWG54002465;     WshgTRAtqSFOrBoWG54002465 = WshgTRAtqSFOrBoWG73804088;     WshgTRAtqSFOrBoWG73804088 = WshgTRAtqSFOrBoWG89355468;     WshgTRAtqSFOrBoWG89355468 = WshgTRAtqSFOrBoWG48493281;     WshgTRAtqSFOrBoWG48493281 = WshgTRAtqSFOrBoWG45281765;     WshgTRAtqSFOrBoWG45281765 = WshgTRAtqSFOrBoWG44045950;     WshgTRAtqSFOrBoWG44045950 = WshgTRAtqSFOrBoWG43948915;     WshgTRAtqSFOrBoWG43948915 = WshgTRAtqSFOrBoWG11922410;     WshgTRAtqSFOrBoWG11922410 = WshgTRAtqSFOrBoWG90471756;     WshgTRAtqSFOrBoWG90471756 = WshgTRAtqSFOrBoWG28756938;     WshgTRAtqSFOrBoWG28756938 = WshgTRAtqSFOrBoWG88523943;     WshgTRAtqSFOrBoWG88523943 = WshgTRAtqSFOrBoWG4334102;     WshgTRAtqSFOrBoWG4334102 = WshgTRAtqSFOrBoWG97949357;     WshgTRAtqSFOrBoWG97949357 = WshgTRAtqSFOrBoWG95321764;     WshgTRAtqSFOrBoWG95321764 = WshgTRAtqSFOrBoWG28025577;     WshgTRAtqSFOrBoWG28025577 = WshgTRAtqSFOrBoWG7649118;     WshgTRAtqSFOrBoWG7649118 = WshgTRAtqSFOrBoWG94512943;     WshgTRAtqSFOrBoWG94512943 = WshgTRAtqSFOrBoWG65673546;     WshgTRAtqSFOrBoWG65673546 = WshgTRAtqSFOrBoWG53999100;     WshgTRAtqSFOrBoWG53999100 = WshgTRAtqSFOrBoWG17757040;     WshgTRAtqSFOrBoWG17757040 = WshgTRAtqSFOrBoWG80645072;     WshgTRAtqSFOrBoWG80645072 = WshgTRAtqSFOrBoWG68566056;     WshgTRAtqSFOrBoWG68566056 = WshgTRAtqSFOrBoWG58345235;     WshgTRAtqSFOrBoWG58345235 = WshgTRAtqSFOrBoWG14809350;     WshgTRAtqSFOrBoWG14809350 = WshgTRAtqSFOrBoWG28419779;     WshgTRAtqSFOrBoWG28419779 = WshgTRAtqSFOrBoWG47248122;     WshgTRAtqSFOrBoWG47248122 = WshgTRAtqSFOrBoWG54888510;     WshgTRAtqSFOrBoWG54888510 = WshgTRAtqSFOrBoWG40879964;     WshgTRAtqSFOrBoWG40879964 = WshgTRAtqSFOrBoWG81616661;     WshgTRAtqSFOrBoWG81616661 = WshgTRAtqSFOrBoWG86657325;     WshgTRAtqSFOrBoWG86657325 = WshgTRAtqSFOrBoWG22418889;     WshgTRAtqSFOrBoWG22418889 = WshgTRAtqSFOrBoWG1826678;     WshgTRAtqSFOrBoWG1826678 = WshgTRAtqSFOrBoWG92738439;     WshgTRAtqSFOrBoWG92738439 = WshgTRAtqSFOrBoWG1369836;     WshgTRAtqSFOrBoWG1369836 = WshgTRAtqSFOrBoWG58470098;     WshgTRAtqSFOrBoWG58470098 = WshgTRAtqSFOrBoWG65356824;     WshgTRAtqSFOrBoWG65356824 = WshgTRAtqSFOrBoWG61688645;     WshgTRAtqSFOrBoWG61688645 = WshgTRAtqSFOrBoWG80255662;     WshgTRAtqSFOrBoWG80255662 = WshgTRAtqSFOrBoWG19762347;     WshgTRAtqSFOrBoWG19762347 = WshgTRAtqSFOrBoWG87251279;     WshgTRAtqSFOrBoWG87251279 = WshgTRAtqSFOrBoWG75626905;     WshgTRAtqSFOrBoWG75626905 = WshgTRAtqSFOrBoWG84586070;     WshgTRAtqSFOrBoWG84586070 = WshgTRAtqSFOrBoWG76532241;     WshgTRAtqSFOrBoWG76532241 = WshgTRAtqSFOrBoWG21369739;     WshgTRAtqSFOrBoWG21369739 = WshgTRAtqSFOrBoWG35204303;     WshgTRAtqSFOrBoWG35204303 = WshgTRAtqSFOrBoWG25371607;     WshgTRAtqSFOrBoWG25371607 = WshgTRAtqSFOrBoWG65935113;     WshgTRAtqSFOrBoWG65935113 = WshgTRAtqSFOrBoWG64582091;     WshgTRAtqSFOrBoWG64582091 = WshgTRAtqSFOrBoWG30170708;     WshgTRAtqSFOrBoWG30170708 = WshgTRAtqSFOrBoWG19502284;     WshgTRAtqSFOrBoWG19502284 = WshgTRAtqSFOrBoWG51295341;     WshgTRAtqSFOrBoWG51295341 = WshgTRAtqSFOrBoWG27208981;     WshgTRAtqSFOrBoWG27208981 = WshgTRAtqSFOrBoWG96577147;     WshgTRAtqSFOrBoWG96577147 = WshgTRAtqSFOrBoWG38783706;     WshgTRAtqSFOrBoWG38783706 = WshgTRAtqSFOrBoWG69410897;     WshgTRAtqSFOrBoWG69410897 = WshgTRAtqSFOrBoWG85436409;     WshgTRAtqSFOrBoWG85436409 = WshgTRAtqSFOrBoWG15458854;     WshgTRAtqSFOrBoWG15458854 = WshgTRAtqSFOrBoWG74546118;     WshgTRAtqSFOrBoWG74546118 = WshgTRAtqSFOrBoWG20073503;     WshgTRAtqSFOrBoWG20073503 = WshgTRAtqSFOrBoWG98033642;     WshgTRAtqSFOrBoWG98033642 = WshgTRAtqSFOrBoWG89157440;     WshgTRAtqSFOrBoWG89157440 = WshgTRAtqSFOrBoWG3068951;     WshgTRAtqSFOrBoWG3068951 = WshgTRAtqSFOrBoWG30305748;     WshgTRAtqSFOrBoWG30305748 = WshgTRAtqSFOrBoWG3814431;     WshgTRAtqSFOrBoWG3814431 = WshgTRAtqSFOrBoWG6338050;     WshgTRAtqSFOrBoWG6338050 = WshgTRAtqSFOrBoWG86697265;     WshgTRAtqSFOrBoWG86697265 = WshgTRAtqSFOrBoWG11595663;     WshgTRAtqSFOrBoWG11595663 = WshgTRAtqSFOrBoWG96579521;     WshgTRAtqSFOrBoWG96579521 = WshgTRAtqSFOrBoWG36851667;     WshgTRAtqSFOrBoWG36851667 = WshgTRAtqSFOrBoWG62668752;     WshgTRAtqSFOrBoWG62668752 = WshgTRAtqSFOrBoWG45960472;     WshgTRAtqSFOrBoWG45960472 = WshgTRAtqSFOrBoWG14257282;     WshgTRAtqSFOrBoWG14257282 = WshgTRAtqSFOrBoWG45911200;     WshgTRAtqSFOrBoWG45911200 = WshgTRAtqSFOrBoWG66747820;     WshgTRAtqSFOrBoWG66747820 = WshgTRAtqSFOrBoWG42130134;     WshgTRAtqSFOrBoWG42130134 = WshgTRAtqSFOrBoWG96059001;     WshgTRAtqSFOrBoWG96059001 = WshgTRAtqSFOrBoWG92033815;     WshgTRAtqSFOrBoWG92033815 = WshgTRAtqSFOrBoWG36975496;     WshgTRAtqSFOrBoWG36975496 = WshgTRAtqSFOrBoWG79605046;     WshgTRAtqSFOrBoWG79605046 = WshgTRAtqSFOrBoWG3048172;     WshgTRAtqSFOrBoWG3048172 = WshgTRAtqSFOrBoWG39160913;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zYLLLNlHltFiEDim27521632() {     double puMOmahaGHEipFnJT45278020 = -217458783;    double puMOmahaGHEipFnJT94417393 = -816229173;    double puMOmahaGHEipFnJT44791127 = 66968899;    double puMOmahaGHEipFnJT92118892 = -507490475;    double puMOmahaGHEipFnJT192719 = -31668041;    double puMOmahaGHEipFnJT10993022 = -217187245;    double puMOmahaGHEipFnJT56997446 = 18779852;    double puMOmahaGHEipFnJT16763889 = -693371842;    double puMOmahaGHEipFnJT14769172 = -416484341;    double puMOmahaGHEipFnJT64957905 = -525604892;    double puMOmahaGHEipFnJT89815999 = -328559914;    double puMOmahaGHEipFnJT95707512 = -485838405;    double puMOmahaGHEipFnJT46112975 = -777887333;    double puMOmahaGHEipFnJT41339961 = -193222822;    double puMOmahaGHEipFnJT22893828 = -611429928;    double puMOmahaGHEipFnJT87313678 = -174862045;    double puMOmahaGHEipFnJT86718530 = 80538435;    double puMOmahaGHEipFnJT47629401 = -965626733;    double puMOmahaGHEipFnJT18370587 = -140420515;    double puMOmahaGHEipFnJT30606134 = -823325501;    double puMOmahaGHEipFnJT42555659 = -703952064;    double puMOmahaGHEipFnJT60325112 = -145018210;    double puMOmahaGHEipFnJT24946260 = -441482812;    double puMOmahaGHEipFnJT32485655 = -479783478;    double puMOmahaGHEipFnJT88981415 = -564236112;    double puMOmahaGHEipFnJT68767625 = -471236664;    double puMOmahaGHEipFnJT42408693 = -6386615;    double puMOmahaGHEipFnJT25029535 = -737806350;    double puMOmahaGHEipFnJT56604012 = -159193417;    double puMOmahaGHEipFnJT97372574 = -10600011;    double puMOmahaGHEipFnJT80766492 = -985892412;    double puMOmahaGHEipFnJT1944724 = -946129878;    double puMOmahaGHEipFnJT7660306 = -948261582;    double puMOmahaGHEipFnJT90152329 = -564263122;    double puMOmahaGHEipFnJT89500228 = -354806145;    double puMOmahaGHEipFnJT76375031 = -126664877;    double puMOmahaGHEipFnJT75774878 = 62320766;    double puMOmahaGHEipFnJT99763758 = -21400936;    double puMOmahaGHEipFnJT27609354 = -240946589;    double puMOmahaGHEipFnJT5258977 = -251029672;    double puMOmahaGHEipFnJT86314296 = -169348808;    double puMOmahaGHEipFnJT716514 = -636092063;    double puMOmahaGHEipFnJT36412213 = -369978176;    double puMOmahaGHEipFnJT2217091 = -317345194;    double puMOmahaGHEipFnJT40372244 = -521419556;    double puMOmahaGHEipFnJT59187317 = -338218720;    double puMOmahaGHEipFnJT72667084 = -523949202;    double puMOmahaGHEipFnJT48836905 = -743888083;    double puMOmahaGHEipFnJT91292390 = -330543206;    double puMOmahaGHEipFnJT99568978 = -321716831;    double puMOmahaGHEipFnJT58119567 = -382728206;    double puMOmahaGHEipFnJT12636398 = -588250259;    double puMOmahaGHEipFnJT40986897 = -550407714;    double puMOmahaGHEipFnJT97338716 = -978392051;    double puMOmahaGHEipFnJT84965973 = -718537289;    double puMOmahaGHEipFnJT84952907 = 27559427;    double puMOmahaGHEipFnJT69471133 = -274746361;    double puMOmahaGHEipFnJT12305473 = -453247623;    double puMOmahaGHEipFnJT3137478 = -943254363;    double puMOmahaGHEipFnJT31425093 = -560431378;    double puMOmahaGHEipFnJT68584329 = -110800631;    double puMOmahaGHEipFnJT31967912 = -243413799;    double puMOmahaGHEipFnJT60159876 = -434178426;    double puMOmahaGHEipFnJT17396598 = -305884331;    double puMOmahaGHEipFnJT84191413 = -539712480;    double puMOmahaGHEipFnJT87871275 = -382430036;    double puMOmahaGHEipFnJT88047206 = -537576824;    double puMOmahaGHEipFnJT55960646 = -113624211;    double puMOmahaGHEipFnJT51839732 = -838416678;    double puMOmahaGHEipFnJT46518797 = -384765051;    double puMOmahaGHEipFnJT11538801 = -137182812;    double puMOmahaGHEipFnJT86954771 = -898060630;    double puMOmahaGHEipFnJT20020047 = -624680145;    double puMOmahaGHEipFnJT13111610 = -889390843;    double puMOmahaGHEipFnJT44291838 = -553976694;    double puMOmahaGHEipFnJT41839145 = 32139999;    double puMOmahaGHEipFnJT23912900 = -775040034;    double puMOmahaGHEipFnJT22729170 = -24137618;    double puMOmahaGHEipFnJT92113411 = -958363923;    double puMOmahaGHEipFnJT29794098 = -126017393;    double puMOmahaGHEipFnJT96100541 = -947287462;    double puMOmahaGHEipFnJT93571787 = -262498532;    double puMOmahaGHEipFnJT33737145 = -307263144;    double puMOmahaGHEipFnJT57035034 = -837476586;    double puMOmahaGHEipFnJT39253008 = -627871805;    double puMOmahaGHEipFnJT68130095 = -297642153;    double puMOmahaGHEipFnJT60957827 = -295722165;    double puMOmahaGHEipFnJT10321590 = -969869532;    double puMOmahaGHEipFnJT5186356 = -845725834;    double puMOmahaGHEipFnJT4547321 = -282365572;    double puMOmahaGHEipFnJT6903898 = -851918516;    double puMOmahaGHEipFnJT63469406 = -484431611;    double puMOmahaGHEipFnJT96626281 = -78146574;    double puMOmahaGHEipFnJT96184260 = -680515211;    double puMOmahaGHEipFnJT36674648 = -40229042;    double puMOmahaGHEipFnJT54346384 = -925935009;    double puMOmahaGHEipFnJT40556637 = -101913637;    double puMOmahaGHEipFnJT19015616 = 35906154;    double puMOmahaGHEipFnJT18025678 = -777632715;    double puMOmahaGHEipFnJT52500968 = -217458783;     puMOmahaGHEipFnJT45278020 = puMOmahaGHEipFnJT94417393;     puMOmahaGHEipFnJT94417393 = puMOmahaGHEipFnJT44791127;     puMOmahaGHEipFnJT44791127 = puMOmahaGHEipFnJT92118892;     puMOmahaGHEipFnJT92118892 = puMOmahaGHEipFnJT192719;     puMOmahaGHEipFnJT192719 = puMOmahaGHEipFnJT10993022;     puMOmahaGHEipFnJT10993022 = puMOmahaGHEipFnJT56997446;     puMOmahaGHEipFnJT56997446 = puMOmahaGHEipFnJT16763889;     puMOmahaGHEipFnJT16763889 = puMOmahaGHEipFnJT14769172;     puMOmahaGHEipFnJT14769172 = puMOmahaGHEipFnJT64957905;     puMOmahaGHEipFnJT64957905 = puMOmahaGHEipFnJT89815999;     puMOmahaGHEipFnJT89815999 = puMOmahaGHEipFnJT95707512;     puMOmahaGHEipFnJT95707512 = puMOmahaGHEipFnJT46112975;     puMOmahaGHEipFnJT46112975 = puMOmahaGHEipFnJT41339961;     puMOmahaGHEipFnJT41339961 = puMOmahaGHEipFnJT22893828;     puMOmahaGHEipFnJT22893828 = puMOmahaGHEipFnJT87313678;     puMOmahaGHEipFnJT87313678 = puMOmahaGHEipFnJT86718530;     puMOmahaGHEipFnJT86718530 = puMOmahaGHEipFnJT47629401;     puMOmahaGHEipFnJT47629401 = puMOmahaGHEipFnJT18370587;     puMOmahaGHEipFnJT18370587 = puMOmahaGHEipFnJT30606134;     puMOmahaGHEipFnJT30606134 = puMOmahaGHEipFnJT42555659;     puMOmahaGHEipFnJT42555659 = puMOmahaGHEipFnJT60325112;     puMOmahaGHEipFnJT60325112 = puMOmahaGHEipFnJT24946260;     puMOmahaGHEipFnJT24946260 = puMOmahaGHEipFnJT32485655;     puMOmahaGHEipFnJT32485655 = puMOmahaGHEipFnJT88981415;     puMOmahaGHEipFnJT88981415 = puMOmahaGHEipFnJT68767625;     puMOmahaGHEipFnJT68767625 = puMOmahaGHEipFnJT42408693;     puMOmahaGHEipFnJT42408693 = puMOmahaGHEipFnJT25029535;     puMOmahaGHEipFnJT25029535 = puMOmahaGHEipFnJT56604012;     puMOmahaGHEipFnJT56604012 = puMOmahaGHEipFnJT97372574;     puMOmahaGHEipFnJT97372574 = puMOmahaGHEipFnJT80766492;     puMOmahaGHEipFnJT80766492 = puMOmahaGHEipFnJT1944724;     puMOmahaGHEipFnJT1944724 = puMOmahaGHEipFnJT7660306;     puMOmahaGHEipFnJT7660306 = puMOmahaGHEipFnJT90152329;     puMOmahaGHEipFnJT90152329 = puMOmahaGHEipFnJT89500228;     puMOmahaGHEipFnJT89500228 = puMOmahaGHEipFnJT76375031;     puMOmahaGHEipFnJT76375031 = puMOmahaGHEipFnJT75774878;     puMOmahaGHEipFnJT75774878 = puMOmahaGHEipFnJT99763758;     puMOmahaGHEipFnJT99763758 = puMOmahaGHEipFnJT27609354;     puMOmahaGHEipFnJT27609354 = puMOmahaGHEipFnJT5258977;     puMOmahaGHEipFnJT5258977 = puMOmahaGHEipFnJT86314296;     puMOmahaGHEipFnJT86314296 = puMOmahaGHEipFnJT716514;     puMOmahaGHEipFnJT716514 = puMOmahaGHEipFnJT36412213;     puMOmahaGHEipFnJT36412213 = puMOmahaGHEipFnJT2217091;     puMOmahaGHEipFnJT2217091 = puMOmahaGHEipFnJT40372244;     puMOmahaGHEipFnJT40372244 = puMOmahaGHEipFnJT59187317;     puMOmahaGHEipFnJT59187317 = puMOmahaGHEipFnJT72667084;     puMOmahaGHEipFnJT72667084 = puMOmahaGHEipFnJT48836905;     puMOmahaGHEipFnJT48836905 = puMOmahaGHEipFnJT91292390;     puMOmahaGHEipFnJT91292390 = puMOmahaGHEipFnJT99568978;     puMOmahaGHEipFnJT99568978 = puMOmahaGHEipFnJT58119567;     puMOmahaGHEipFnJT58119567 = puMOmahaGHEipFnJT12636398;     puMOmahaGHEipFnJT12636398 = puMOmahaGHEipFnJT40986897;     puMOmahaGHEipFnJT40986897 = puMOmahaGHEipFnJT97338716;     puMOmahaGHEipFnJT97338716 = puMOmahaGHEipFnJT84965973;     puMOmahaGHEipFnJT84965973 = puMOmahaGHEipFnJT84952907;     puMOmahaGHEipFnJT84952907 = puMOmahaGHEipFnJT69471133;     puMOmahaGHEipFnJT69471133 = puMOmahaGHEipFnJT12305473;     puMOmahaGHEipFnJT12305473 = puMOmahaGHEipFnJT3137478;     puMOmahaGHEipFnJT3137478 = puMOmahaGHEipFnJT31425093;     puMOmahaGHEipFnJT31425093 = puMOmahaGHEipFnJT68584329;     puMOmahaGHEipFnJT68584329 = puMOmahaGHEipFnJT31967912;     puMOmahaGHEipFnJT31967912 = puMOmahaGHEipFnJT60159876;     puMOmahaGHEipFnJT60159876 = puMOmahaGHEipFnJT17396598;     puMOmahaGHEipFnJT17396598 = puMOmahaGHEipFnJT84191413;     puMOmahaGHEipFnJT84191413 = puMOmahaGHEipFnJT87871275;     puMOmahaGHEipFnJT87871275 = puMOmahaGHEipFnJT88047206;     puMOmahaGHEipFnJT88047206 = puMOmahaGHEipFnJT55960646;     puMOmahaGHEipFnJT55960646 = puMOmahaGHEipFnJT51839732;     puMOmahaGHEipFnJT51839732 = puMOmahaGHEipFnJT46518797;     puMOmahaGHEipFnJT46518797 = puMOmahaGHEipFnJT11538801;     puMOmahaGHEipFnJT11538801 = puMOmahaGHEipFnJT86954771;     puMOmahaGHEipFnJT86954771 = puMOmahaGHEipFnJT20020047;     puMOmahaGHEipFnJT20020047 = puMOmahaGHEipFnJT13111610;     puMOmahaGHEipFnJT13111610 = puMOmahaGHEipFnJT44291838;     puMOmahaGHEipFnJT44291838 = puMOmahaGHEipFnJT41839145;     puMOmahaGHEipFnJT41839145 = puMOmahaGHEipFnJT23912900;     puMOmahaGHEipFnJT23912900 = puMOmahaGHEipFnJT22729170;     puMOmahaGHEipFnJT22729170 = puMOmahaGHEipFnJT92113411;     puMOmahaGHEipFnJT92113411 = puMOmahaGHEipFnJT29794098;     puMOmahaGHEipFnJT29794098 = puMOmahaGHEipFnJT96100541;     puMOmahaGHEipFnJT96100541 = puMOmahaGHEipFnJT93571787;     puMOmahaGHEipFnJT93571787 = puMOmahaGHEipFnJT33737145;     puMOmahaGHEipFnJT33737145 = puMOmahaGHEipFnJT57035034;     puMOmahaGHEipFnJT57035034 = puMOmahaGHEipFnJT39253008;     puMOmahaGHEipFnJT39253008 = puMOmahaGHEipFnJT68130095;     puMOmahaGHEipFnJT68130095 = puMOmahaGHEipFnJT60957827;     puMOmahaGHEipFnJT60957827 = puMOmahaGHEipFnJT10321590;     puMOmahaGHEipFnJT10321590 = puMOmahaGHEipFnJT5186356;     puMOmahaGHEipFnJT5186356 = puMOmahaGHEipFnJT4547321;     puMOmahaGHEipFnJT4547321 = puMOmahaGHEipFnJT6903898;     puMOmahaGHEipFnJT6903898 = puMOmahaGHEipFnJT63469406;     puMOmahaGHEipFnJT63469406 = puMOmahaGHEipFnJT96626281;     puMOmahaGHEipFnJT96626281 = puMOmahaGHEipFnJT96184260;     puMOmahaGHEipFnJT96184260 = puMOmahaGHEipFnJT36674648;     puMOmahaGHEipFnJT36674648 = puMOmahaGHEipFnJT54346384;     puMOmahaGHEipFnJT54346384 = puMOmahaGHEipFnJT40556637;     puMOmahaGHEipFnJT40556637 = puMOmahaGHEipFnJT19015616;     puMOmahaGHEipFnJT19015616 = puMOmahaGHEipFnJT18025678;     puMOmahaGHEipFnJT18025678 = puMOmahaGHEipFnJT52500968;     puMOmahaGHEipFnJT52500968 = puMOmahaGHEipFnJT45278020;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void PxgUrIlYhgfkpWij94813231() {     double OdtpsEvQMspeLkTnX80736988 = -683735682;    double OdtpsEvQMspeLkTnX7689472 = -696780779;    double OdtpsEvQMspeLkTnX82177604 = -698698176;    double OdtpsEvQMspeLkTnX64342247 = -328641546;    double OdtpsEvQMspeLkTnX56477988 = -137266887;    double OdtpsEvQMspeLkTnX47215894 = -479535141;    double OdtpsEvQMspeLkTnX67654852 = -925945882;    double OdtpsEvQMspeLkTnX78895449 = -337605983;    double OdtpsEvQMspeLkTnX16286777 = -904815748;    double OdtpsEvQMspeLkTnX79860617 = -190291890;    double OdtpsEvQMspeLkTnX27997734 = -130567643;    double OdtpsEvQMspeLkTnX96275660 = -349959177;    double OdtpsEvQMspeLkTnX84347620 = -229899837;    double OdtpsEvQMspeLkTnX94480823 = -169586232;    double OdtpsEvQMspeLkTnX81088344 = -14854943;    double OdtpsEvQMspeLkTnX70854979 = -910673652;    double OdtpsEvQMspeLkTnX42554466 = -446436118;    double OdtpsEvQMspeLkTnX47389593 = 60945198;    double OdtpsEvQMspeLkTnX18920917 = -456983936;    double OdtpsEvQMspeLkTnX24443353 = -149886567;    double OdtpsEvQMspeLkTnX55642521 = -361226753;    double OdtpsEvQMspeLkTnX63746637 = -231654504;    double OdtpsEvQMspeLkTnX95471639 = -79114996;    double OdtpsEvQMspeLkTnX26278600 = -990608082;    double OdtpsEvQMspeLkTnX96999366 = -588221311;    double OdtpsEvQMspeLkTnX79465797 = -159813470;    double OdtpsEvQMspeLkTnX2829267 = -912670756;    double OdtpsEvQMspeLkTnX65757959 = -907578524;    double OdtpsEvQMspeLkTnX3272227 = -733288219;    double OdtpsEvQMspeLkTnX2905986 = -45107181;    double OdtpsEvQMspeLkTnX99349724 = -437872899;    double OdtpsEvQMspeLkTnX39767742 = -719101292;    double OdtpsEvQMspeLkTnX19878317 = 4142441;    double OdtpsEvQMspeLkTnX19900756 = -628956936;    double OdtpsEvQMspeLkTnX55308682 = -305906395;    double OdtpsEvQMspeLkTnX27935875 = -341230183;    double OdtpsEvQMspeLkTnX25186695 = -902375274;    double OdtpsEvQMspeLkTnX50979162 = -4056517;    double OdtpsEvQMspeLkTnX57999915 = -991822865;    double OdtpsEvQMspeLkTnX69530509 = -816292218;    double OdtpsEvQMspeLkTnX19765904 = 61452278;    double OdtpsEvQMspeLkTnX90081640 = -230637354;    double OdtpsEvQMspeLkTnX25795591 = 76841038;    double OdtpsEvQMspeLkTnX42348429 = -921677604;    double OdtpsEvQMspeLkTnX40388702 = -347930717;    double OdtpsEvQMspeLkTnX5148646 = -377142352;    double OdtpsEvQMspeLkTnX92012248 = 92064438;    double OdtpsEvQMspeLkTnX66563216 = -910414677;    double OdtpsEvQMspeLkTnX14185056 = 5719914;    double OdtpsEvQMspeLkTnX24098568 = -812743647;    double OdtpsEvQMspeLkTnX38890291 = -257004764;    double OdtpsEvQMspeLkTnX96121869 = -801902606;    double OdtpsEvQMspeLkTnX9608046 = -306453268;    double OdtpsEvQMspeLkTnX13330563 = -69900550;    double OdtpsEvQMspeLkTnX39170710 = 34972565;    double OdtpsEvQMspeLkTnX16990351 = -352081179;    double OdtpsEvQMspeLkTnX12217832 = -517665784;    double OdtpsEvQMspeLkTnX55899004 = -708090094;    double OdtpsEvQMspeLkTnX67342881 = -740420236;    double OdtpsEvQMspeLkTnX77012190 = -977453417;    double OdtpsEvQMspeLkTnX44386627 = -566864386;    double OdtpsEvQMspeLkTnX1896893 = 81632642;    double OdtpsEvQMspeLkTnX75623222 = -604317764;    double OdtpsEvQMspeLkTnX13380792 = -759708567;    double OdtpsEvQMspeLkTnX80510893 = -752418991;    double OdtpsEvQMspeLkTnX88229992 = -411466351;    double OdtpsEvQMspeLkTnX76397344 = -254101618;    double OdtpsEvQMspeLkTnX64446865 = -600942902;    double OdtpsEvQMspeLkTnX39172142 = -863679837;    double OdtpsEvQMspeLkTnX53152469 = -673624760;    double OdtpsEvQMspeLkTnX45668285 = 91701621;    double OdtpsEvQMspeLkTnX91575303 = -342379601;    double OdtpsEvQMspeLkTnX89389677 = 52768063;    double OdtpsEvQMspeLkTnX49390408 = -640691719;    double OdtpsEvQMspeLkTnX4677449 = -111338846;    double OdtpsEvQMspeLkTnX65560881 = -30589399;    double OdtpsEvQMspeLkTnX37951046 = -208495542;    double OdtpsEvQMspeLkTnX53123211 = -157437392;    double OdtpsEvQMspeLkTnX85889898 = -542677366;    double OdtpsEvQMspeLkTnX91850720 = -111078960;    double OdtpsEvQMspeLkTnX87453549 = -151877909;    double OdtpsEvQMspeLkTnX36266050 = 97743920;    double OdtpsEvQMspeLkTnX51572903 = -813298439;    double OdtpsEvQMspeLkTnX79173659 = -920544572;    double OdtpsEvQMspeLkTnX64015695 = -788102418;    double OdtpsEvQMspeLkTnX3227855 = -635970294;    double OdtpsEvQMspeLkTnX30159697 = -312648025;    double OdtpsEvQMspeLkTnX6547754 = -925957010;    double OdtpsEvQMspeLkTnX80730046 = -563929501;    double OdtpsEvQMspeLkTnX38318331 = -953825217;    double OdtpsEvQMspeLkTnX15718043 = -823564399;    double OdtpsEvQMspeLkTnX69287691 = -94285180;    double OdtpsEvQMspeLkTnX83636281 = -263636282;    double OdtpsEvQMspeLkTnX80987724 = 85630551;    double OdtpsEvQMspeLkTnX25143882 = -149427833;    double OdtpsEvQMspeLkTnX17869011 = 79819636;    double OdtpsEvQMspeLkTnX14458418 = -626319590;    double OdtpsEvQMspeLkTnX12414800 = -163450395;    double OdtpsEvQMspeLkTnX61837535 = -69258614;    double OdtpsEvQMspeLkTnX52158710 = -683735682;     OdtpsEvQMspeLkTnX80736988 = OdtpsEvQMspeLkTnX7689472;     OdtpsEvQMspeLkTnX7689472 = OdtpsEvQMspeLkTnX82177604;     OdtpsEvQMspeLkTnX82177604 = OdtpsEvQMspeLkTnX64342247;     OdtpsEvQMspeLkTnX64342247 = OdtpsEvQMspeLkTnX56477988;     OdtpsEvQMspeLkTnX56477988 = OdtpsEvQMspeLkTnX47215894;     OdtpsEvQMspeLkTnX47215894 = OdtpsEvQMspeLkTnX67654852;     OdtpsEvQMspeLkTnX67654852 = OdtpsEvQMspeLkTnX78895449;     OdtpsEvQMspeLkTnX78895449 = OdtpsEvQMspeLkTnX16286777;     OdtpsEvQMspeLkTnX16286777 = OdtpsEvQMspeLkTnX79860617;     OdtpsEvQMspeLkTnX79860617 = OdtpsEvQMspeLkTnX27997734;     OdtpsEvQMspeLkTnX27997734 = OdtpsEvQMspeLkTnX96275660;     OdtpsEvQMspeLkTnX96275660 = OdtpsEvQMspeLkTnX84347620;     OdtpsEvQMspeLkTnX84347620 = OdtpsEvQMspeLkTnX94480823;     OdtpsEvQMspeLkTnX94480823 = OdtpsEvQMspeLkTnX81088344;     OdtpsEvQMspeLkTnX81088344 = OdtpsEvQMspeLkTnX70854979;     OdtpsEvQMspeLkTnX70854979 = OdtpsEvQMspeLkTnX42554466;     OdtpsEvQMspeLkTnX42554466 = OdtpsEvQMspeLkTnX47389593;     OdtpsEvQMspeLkTnX47389593 = OdtpsEvQMspeLkTnX18920917;     OdtpsEvQMspeLkTnX18920917 = OdtpsEvQMspeLkTnX24443353;     OdtpsEvQMspeLkTnX24443353 = OdtpsEvQMspeLkTnX55642521;     OdtpsEvQMspeLkTnX55642521 = OdtpsEvQMspeLkTnX63746637;     OdtpsEvQMspeLkTnX63746637 = OdtpsEvQMspeLkTnX95471639;     OdtpsEvQMspeLkTnX95471639 = OdtpsEvQMspeLkTnX26278600;     OdtpsEvQMspeLkTnX26278600 = OdtpsEvQMspeLkTnX96999366;     OdtpsEvQMspeLkTnX96999366 = OdtpsEvQMspeLkTnX79465797;     OdtpsEvQMspeLkTnX79465797 = OdtpsEvQMspeLkTnX2829267;     OdtpsEvQMspeLkTnX2829267 = OdtpsEvQMspeLkTnX65757959;     OdtpsEvQMspeLkTnX65757959 = OdtpsEvQMspeLkTnX3272227;     OdtpsEvQMspeLkTnX3272227 = OdtpsEvQMspeLkTnX2905986;     OdtpsEvQMspeLkTnX2905986 = OdtpsEvQMspeLkTnX99349724;     OdtpsEvQMspeLkTnX99349724 = OdtpsEvQMspeLkTnX39767742;     OdtpsEvQMspeLkTnX39767742 = OdtpsEvQMspeLkTnX19878317;     OdtpsEvQMspeLkTnX19878317 = OdtpsEvQMspeLkTnX19900756;     OdtpsEvQMspeLkTnX19900756 = OdtpsEvQMspeLkTnX55308682;     OdtpsEvQMspeLkTnX55308682 = OdtpsEvQMspeLkTnX27935875;     OdtpsEvQMspeLkTnX27935875 = OdtpsEvQMspeLkTnX25186695;     OdtpsEvQMspeLkTnX25186695 = OdtpsEvQMspeLkTnX50979162;     OdtpsEvQMspeLkTnX50979162 = OdtpsEvQMspeLkTnX57999915;     OdtpsEvQMspeLkTnX57999915 = OdtpsEvQMspeLkTnX69530509;     OdtpsEvQMspeLkTnX69530509 = OdtpsEvQMspeLkTnX19765904;     OdtpsEvQMspeLkTnX19765904 = OdtpsEvQMspeLkTnX90081640;     OdtpsEvQMspeLkTnX90081640 = OdtpsEvQMspeLkTnX25795591;     OdtpsEvQMspeLkTnX25795591 = OdtpsEvQMspeLkTnX42348429;     OdtpsEvQMspeLkTnX42348429 = OdtpsEvQMspeLkTnX40388702;     OdtpsEvQMspeLkTnX40388702 = OdtpsEvQMspeLkTnX5148646;     OdtpsEvQMspeLkTnX5148646 = OdtpsEvQMspeLkTnX92012248;     OdtpsEvQMspeLkTnX92012248 = OdtpsEvQMspeLkTnX66563216;     OdtpsEvQMspeLkTnX66563216 = OdtpsEvQMspeLkTnX14185056;     OdtpsEvQMspeLkTnX14185056 = OdtpsEvQMspeLkTnX24098568;     OdtpsEvQMspeLkTnX24098568 = OdtpsEvQMspeLkTnX38890291;     OdtpsEvQMspeLkTnX38890291 = OdtpsEvQMspeLkTnX96121869;     OdtpsEvQMspeLkTnX96121869 = OdtpsEvQMspeLkTnX9608046;     OdtpsEvQMspeLkTnX9608046 = OdtpsEvQMspeLkTnX13330563;     OdtpsEvQMspeLkTnX13330563 = OdtpsEvQMspeLkTnX39170710;     OdtpsEvQMspeLkTnX39170710 = OdtpsEvQMspeLkTnX16990351;     OdtpsEvQMspeLkTnX16990351 = OdtpsEvQMspeLkTnX12217832;     OdtpsEvQMspeLkTnX12217832 = OdtpsEvQMspeLkTnX55899004;     OdtpsEvQMspeLkTnX55899004 = OdtpsEvQMspeLkTnX67342881;     OdtpsEvQMspeLkTnX67342881 = OdtpsEvQMspeLkTnX77012190;     OdtpsEvQMspeLkTnX77012190 = OdtpsEvQMspeLkTnX44386627;     OdtpsEvQMspeLkTnX44386627 = OdtpsEvQMspeLkTnX1896893;     OdtpsEvQMspeLkTnX1896893 = OdtpsEvQMspeLkTnX75623222;     OdtpsEvQMspeLkTnX75623222 = OdtpsEvQMspeLkTnX13380792;     OdtpsEvQMspeLkTnX13380792 = OdtpsEvQMspeLkTnX80510893;     OdtpsEvQMspeLkTnX80510893 = OdtpsEvQMspeLkTnX88229992;     OdtpsEvQMspeLkTnX88229992 = OdtpsEvQMspeLkTnX76397344;     OdtpsEvQMspeLkTnX76397344 = OdtpsEvQMspeLkTnX64446865;     OdtpsEvQMspeLkTnX64446865 = OdtpsEvQMspeLkTnX39172142;     OdtpsEvQMspeLkTnX39172142 = OdtpsEvQMspeLkTnX53152469;     OdtpsEvQMspeLkTnX53152469 = OdtpsEvQMspeLkTnX45668285;     OdtpsEvQMspeLkTnX45668285 = OdtpsEvQMspeLkTnX91575303;     OdtpsEvQMspeLkTnX91575303 = OdtpsEvQMspeLkTnX89389677;     OdtpsEvQMspeLkTnX89389677 = OdtpsEvQMspeLkTnX49390408;     OdtpsEvQMspeLkTnX49390408 = OdtpsEvQMspeLkTnX4677449;     OdtpsEvQMspeLkTnX4677449 = OdtpsEvQMspeLkTnX65560881;     OdtpsEvQMspeLkTnX65560881 = OdtpsEvQMspeLkTnX37951046;     OdtpsEvQMspeLkTnX37951046 = OdtpsEvQMspeLkTnX53123211;     OdtpsEvQMspeLkTnX53123211 = OdtpsEvQMspeLkTnX85889898;     OdtpsEvQMspeLkTnX85889898 = OdtpsEvQMspeLkTnX91850720;     OdtpsEvQMspeLkTnX91850720 = OdtpsEvQMspeLkTnX87453549;     OdtpsEvQMspeLkTnX87453549 = OdtpsEvQMspeLkTnX36266050;     OdtpsEvQMspeLkTnX36266050 = OdtpsEvQMspeLkTnX51572903;     OdtpsEvQMspeLkTnX51572903 = OdtpsEvQMspeLkTnX79173659;     OdtpsEvQMspeLkTnX79173659 = OdtpsEvQMspeLkTnX64015695;     OdtpsEvQMspeLkTnX64015695 = OdtpsEvQMspeLkTnX3227855;     OdtpsEvQMspeLkTnX3227855 = OdtpsEvQMspeLkTnX30159697;     OdtpsEvQMspeLkTnX30159697 = OdtpsEvQMspeLkTnX6547754;     OdtpsEvQMspeLkTnX6547754 = OdtpsEvQMspeLkTnX80730046;     OdtpsEvQMspeLkTnX80730046 = OdtpsEvQMspeLkTnX38318331;     OdtpsEvQMspeLkTnX38318331 = OdtpsEvQMspeLkTnX15718043;     OdtpsEvQMspeLkTnX15718043 = OdtpsEvQMspeLkTnX69287691;     OdtpsEvQMspeLkTnX69287691 = OdtpsEvQMspeLkTnX83636281;     OdtpsEvQMspeLkTnX83636281 = OdtpsEvQMspeLkTnX80987724;     OdtpsEvQMspeLkTnX80987724 = OdtpsEvQMspeLkTnX25143882;     OdtpsEvQMspeLkTnX25143882 = OdtpsEvQMspeLkTnX17869011;     OdtpsEvQMspeLkTnX17869011 = OdtpsEvQMspeLkTnX14458418;     OdtpsEvQMspeLkTnX14458418 = OdtpsEvQMspeLkTnX12414800;     OdtpsEvQMspeLkTnX12414800 = OdtpsEvQMspeLkTnX61837535;     OdtpsEvQMspeLkTnX61837535 = OdtpsEvQMspeLkTnX52158710;     OdtpsEvQMspeLkTnX52158710 = OdtpsEvQMspeLkTnX80736988;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void mwSlifGBSmuSnoGZ9862299() {     double VvvxDZAvLOFZWVqKJ86854095 = -795637571;    double VvvxDZAvLOFZWVqKJ51062751 = -406380560;    double VvvxDZAvLOFZWVqKJ98219787 = -849531097;    double VvvxDZAvLOFZWVqKJ91417028 = -519390176;    double VvvxDZAvLOFZWVqKJ25373477 = -657916758;    double VvvxDZAvLOFZWVqKJ38633096 = -744031376;    double VvvxDZAvLOFZWVqKJ28143818 = 35089141;    double VvvxDZAvLOFZWVqKJ28655340 = -624479896;    double VvvxDZAvLOFZWVqKJ80929272 = -672848414;    double VvvxDZAvLOFZWVqKJ21090277 = -519267367;    double VvvxDZAvLOFZWVqKJ88108024 = -8005230;    double VvvxDZAvLOFZWVqKJ28098704 = -794365379;    double VvvxDZAvLOFZWVqKJ70556739 = -669790926;    double VvvxDZAvLOFZWVqKJ77624500 = -701824800;    double VvvxDZAvLOFZWVqKJ76830770 = -705317109;    double VvvxDZAvLOFZWVqKJ12360374 = -883673195;    double VvvxDZAvLOFZWVqKJ36390469 = 37018769;    double VvvxDZAvLOFZWVqKJ44442746 = -704577445;    double VvvxDZAvLOFZWVqKJ80750758 = -862753033;    double VvvxDZAvLOFZWVqKJ4993518 = -506789487;    double VvvxDZAvLOFZWVqKJ44195715 = -216714208;    double VvvxDZAvLOFZWVqKJ50267661 = -139584220;    double VvvxDZAvLOFZWVqKJ31062432 = -925632483;    double VvvxDZAvLOFZWVqKJ10270974 = -112913070;    double VvvxDZAvLOFZWVqKJ40699016 = -961417344;    double VvvxDZAvLOFZWVqKJ4187473 = -38468794;    double VvvxDZAvLOFZWVqKJ1289045 = -813332959;    double VvvxDZAvLOFZWVqKJ78865084 = -973507478;    double VvvxDZAvLOFZWVqKJ69404482 = -955670924;    double VvvxDZAvLOFZWVqKJ71521622 = -962578048;    double VvvxDZAvLOFZWVqKJ91592273 = -351193837;    double VvvxDZAvLOFZWVqKJ37378364 = -428548108;    double VvvxDZAvLOFZWVqKJ29589265 = -988470930;    double VvvxDZAvLOFZWVqKJ14731320 = -380498701;    double VvvxDZAvLOFZWVqKJ16783334 = -944639501;    double VvvxDZAvLOFZWVqKJ96661787 = -555256290;    double VvvxDZAvLOFZWVqKJ6448630 = -856011143;    double VvvxDZAvLOFZWVqKJ85069374 = -875562527;    double VvvxDZAvLOFZWVqKJ31610169 = -985746362;    double VvvxDZAvLOFZWVqKJ57032447 = -189731441;    double VvvxDZAvLOFZWVqKJ25435128 = -3821616;    double VvvxDZAvLOFZWVqKJ22232097 = -500779632;    double VvvxDZAvLOFZWVqKJ3862570 = -3963546;    double VvvxDZAvLOFZWVqKJ29756169 = -941020542;    double VvvxDZAvLOFZWVqKJ52341167 = -9828872;    double VvvxDZAvLOFZWVqKJ17087841 = -362358468;    double VvvxDZAvLOFZWVqKJ9790823 = -26711100;    double VvvxDZAvLOFZWVqKJ74520157 = -920310209;    double VvvxDZAvLOFZWVqKJ23860785 = -80017074;    double VvvxDZAvLOFZWVqKJ37010220 = -899754455;    double VvvxDZAvLOFZWVqKJ74590969 = -99298435;    double VvvxDZAvLOFZWVqKJ6931590 = -602628926;    double VvvxDZAvLOFZWVqKJ57856503 = -98242855;    double VvvxDZAvLOFZWVqKJ9299444 = -390370695;    double VvvxDZAvLOFZWVqKJ65666584 = -665559217;    double VvvxDZAvLOFZWVqKJ36586434 = -556053352;    double VvvxDZAvLOFZWVqKJ20000320 = -480748078;    double VvvxDZAvLOFZWVqKJ87948813 = -636618028;    double VvvxDZAvLOFZWVqKJ50718012 = -557972833;    double VvvxDZAvLOFZWVqKJ21186004 = -519447965;    double VvvxDZAvLOFZWVqKJ37344052 = -930698418;    double VvvxDZAvLOFZWVqKJ49278733 = 8596618;    double VvvxDZAvLOFZWVqKJ59250858 = -668808973;    double VvvxDZAvLOFZWVqKJ9407651 = -710270366;    double VvvxDZAvLOFZWVqKJ29498004 = -68073530;    double VvvxDZAvLOFZWVqKJ50729660 = -579457122;    double VvvxDZAvLOFZWVqKJ98509438 = -805894449;    double VvvxDZAvLOFZWVqKJ55825419 = -189292226;    double VvvxDZAvLOFZWVqKJ60841166 = -757185299;    double VvvxDZAvLOFZWVqKJ80168983 = -50060819;    double VvvxDZAvLOFZWVqKJ5911744 = 72337948;    double VvvxDZAvLOFZWVqKJ51321094 = -87418704;    double VvvxDZAvLOFZWVqKJ12832577 = -718831083;    double VvvxDZAvLOFZWVqKJ23718312 = -573021593;    double VvvxDZAvLOFZWVqKJ79558389 = -402967871;    double VvvxDZAvLOFZWVqKJ21963618 = -715934577;    double VvvxDZAvLOFZWVqKJ46405091 = -35620675;    double VvvxDZAvLOFZWVqKJ1306263 = -984611941;    double VvvxDZAvLOFZWVqKJ57929807 = -3084199;    double VvvxDZAvLOFZWVqKJ23611176 = -499058876;    double VvvxDZAvLOFZWVqKJ94396650 = 88242306;    double VvvxDZAvLOFZWVqKJ26768888 = -893022750;    double VvvxDZAvLOFZWVqKJ55004300 = -793490404;    double VvvxDZAvLOFZWVqKJ32394262 = 44083531;    double VvvxDZAvLOFZWVqKJ96930653 = -763279614;    double VvvxDZAvLOFZWVqKJ84660684 = -748564912;    double VvvxDZAvLOFZWVqKJ79521861 = -230305253;    double VvvxDZAvLOFZWVqKJ20289822 = -498100235;    double VvvxDZAvLOFZWVqKJ49064736 = -714939484;    double VvvxDZAvLOFZWVqKJ80196900 = -288586149;    double VvvxDZAvLOFZWVqKJ76661468 = 25491787;    double VvvxDZAvLOFZWVqKJ18499816 = -119393116;    double VvvxDZAvLOFZWVqKJ34351363 = -217589694;    double VvvxDZAvLOFZWVqKJ10424166 = -366298397;    double VvvxDZAvLOFZWVqKJ19688396 = -259033024;    double VvvxDZAvLOFZWVqKJ76156394 = 87581765;    double VvvxDZAvLOFZWVqKJ62981239 = -831970660;    double VvvxDZAvLOFZWVqKJ94454919 = -293693181;    double VvvxDZAvLOFZWVqKJ258166 = -772947012;    double VvvxDZAvLOFZWVqKJ1611507 = -795637571;     VvvxDZAvLOFZWVqKJ86854095 = VvvxDZAvLOFZWVqKJ51062751;     VvvxDZAvLOFZWVqKJ51062751 = VvvxDZAvLOFZWVqKJ98219787;     VvvxDZAvLOFZWVqKJ98219787 = VvvxDZAvLOFZWVqKJ91417028;     VvvxDZAvLOFZWVqKJ91417028 = VvvxDZAvLOFZWVqKJ25373477;     VvvxDZAvLOFZWVqKJ25373477 = VvvxDZAvLOFZWVqKJ38633096;     VvvxDZAvLOFZWVqKJ38633096 = VvvxDZAvLOFZWVqKJ28143818;     VvvxDZAvLOFZWVqKJ28143818 = VvvxDZAvLOFZWVqKJ28655340;     VvvxDZAvLOFZWVqKJ28655340 = VvvxDZAvLOFZWVqKJ80929272;     VvvxDZAvLOFZWVqKJ80929272 = VvvxDZAvLOFZWVqKJ21090277;     VvvxDZAvLOFZWVqKJ21090277 = VvvxDZAvLOFZWVqKJ88108024;     VvvxDZAvLOFZWVqKJ88108024 = VvvxDZAvLOFZWVqKJ28098704;     VvvxDZAvLOFZWVqKJ28098704 = VvvxDZAvLOFZWVqKJ70556739;     VvvxDZAvLOFZWVqKJ70556739 = VvvxDZAvLOFZWVqKJ77624500;     VvvxDZAvLOFZWVqKJ77624500 = VvvxDZAvLOFZWVqKJ76830770;     VvvxDZAvLOFZWVqKJ76830770 = VvvxDZAvLOFZWVqKJ12360374;     VvvxDZAvLOFZWVqKJ12360374 = VvvxDZAvLOFZWVqKJ36390469;     VvvxDZAvLOFZWVqKJ36390469 = VvvxDZAvLOFZWVqKJ44442746;     VvvxDZAvLOFZWVqKJ44442746 = VvvxDZAvLOFZWVqKJ80750758;     VvvxDZAvLOFZWVqKJ80750758 = VvvxDZAvLOFZWVqKJ4993518;     VvvxDZAvLOFZWVqKJ4993518 = VvvxDZAvLOFZWVqKJ44195715;     VvvxDZAvLOFZWVqKJ44195715 = VvvxDZAvLOFZWVqKJ50267661;     VvvxDZAvLOFZWVqKJ50267661 = VvvxDZAvLOFZWVqKJ31062432;     VvvxDZAvLOFZWVqKJ31062432 = VvvxDZAvLOFZWVqKJ10270974;     VvvxDZAvLOFZWVqKJ10270974 = VvvxDZAvLOFZWVqKJ40699016;     VvvxDZAvLOFZWVqKJ40699016 = VvvxDZAvLOFZWVqKJ4187473;     VvvxDZAvLOFZWVqKJ4187473 = VvvxDZAvLOFZWVqKJ1289045;     VvvxDZAvLOFZWVqKJ1289045 = VvvxDZAvLOFZWVqKJ78865084;     VvvxDZAvLOFZWVqKJ78865084 = VvvxDZAvLOFZWVqKJ69404482;     VvvxDZAvLOFZWVqKJ69404482 = VvvxDZAvLOFZWVqKJ71521622;     VvvxDZAvLOFZWVqKJ71521622 = VvvxDZAvLOFZWVqKJ91592273;     VvvxDZAvLOFZWVqKJ91592273 = VvvxDZAvLOFZWVqKJ37378364;     VvvxDZAvLOFZWVqKJ37378364 = VvvxDZAvLOFZWVqKJ29589265;     VvvxDZAvLOFZWVqKJ29589265 = VvvxDZAvLOFZWVqKJ14731320;     VvvxDZAvLOFZWVqKJ14731320 = VvvxDZAvLOFZWVqKJ16783334;     VvvxDZAvLOFZWVqKJ16783334 = VvvxDZAvLOFZWVqKJ96661787;     VvvxDZAvLOFZWVqKJ96661787 = VvvxDZAvLOFZWVqKJ6448630;     VvvxDZAvLOFZWVqKJ6448630 = VvvxDZAvLOFZWVqKJ85069374;     VvvxDZAvLOFZWVqKJ85069374 = VvvxDZAvLOFZWVqKJ31610169;     VvvxDZAvLOFZWVqKJ31610169 = VvvxDZAvLOFZWVqKJ57032447;     VvvxDZAvLOFZWVqKJ57032447 = VvvxDZAvLOFZWVqKJ25435128;     VvvxDZAvLOFZWVqKJ25435128 = VvvxDZAvLOFZWVqKJ22232097;     VvvxDZAvLOFZWVqKJ22232097 = VvvxDZAvLOFZWVqKJ3862570;     VvvxDZAvLOFZWVqKJ3862570 = VvvxDZAvLOFZWVqKJ29756169;     VvvxDZAvLOFZWVqKJ29756169 = VvvxDZAvLOFZWVqKJ52341167;     VvvxDZAvLOFZWVqKJ52341167 = VvvxDZAvLOFZWVqKJ17087841;     VvvxDZAvLOFZWVqKJ17087841 = VvvxDZAvLOFZWVqKJ9790823;     VvvxDZAvLOFZWVqKJ9790823 = VvvxDZAvLOFZWVqKJ74520157;     VvvxDZAvLOFZWVqKJ74520157 = VvvxDZAvLOFZWVqKJ23860785;     VvvxDZAvLOFZWVqKJ23860785 = VvvxDZAvLOFZWVqKJ37010220;     VvvxDZAvLOFZWVqKJ37010220 = VvvxDZAvLOFZWVqKJ74590969;     VvvxDZAvLOFZWVqKJ74590969 = VvvxDZAvLOFZWVqKJ6931590;     VvvxDZAvLOFZWVqKJ6931590 = VvvxDZAvLOFZWVqKJ57856503;     VvvxDZAvLOFZWVqKJ57856503 = VvvxDZAvLOFZWVqKJ9299444;     VvvxDZAvLOFZWVqKJ9299444 = VvvxDZAvLOFZWVqKJ65666584;     VvvxDZAvLOFZWVqKJ65666584 = VvvxDZAvLOFZWVqKJ36586434;     VvvxDZAvLOFZWVqKJ36586434 = VvvxDZAvLOFZWVqKJ20000320;     VvvxDZAvLOFZWVqKJ20000320 = VvvxDZAvLOFZWVqKJ87948813;     VvvxDZAvLOFZWVqKJ87948813 = VvvxDZAvLOFZWVqKJ50718012;     VvvxDZAvLOFZWVqKJ50718012 = VvvxDZAvLOFZWVqKJ21186004;     VvvxDZAvLOFZWVqKJ21186004 = VvvxDZAvLOFZWVqKJ37344052;     VvvxDZAvLOFZWVqKJ37344052 = VvvxDZAvLOFZWVqKJ49278733;     VvvxDZAvLOFZWVqKJ49278733 = VvvxDZAvLOFZWVqKJ59250858;     VvvxDZAvLOFZWVqKJ59250858 = VvvxDZAvLOFZWVqKJ9407651;     VvvxDZAvLOFZWVqKJ9407651 = VvvxDZAvLOFZWVqKJ29498004;     VvvxDZAvLOFZWVqKJ29498004 = VvvxDZAvLOFZWVqKJ50729660;     VvvxDZAvLOFZWVqKJ50729660 = VvvxDZAvLOFZWVqKJ98509438;     VvvxDZAvLOFZWVqKJ98509438 = VvvxDZAvLOFZWVqKJ55825419;     VvvxDZAvLOFZWVqKJ55825419 = VvvxDZAvLOFZWVqKJ60841166;     VvvxDZAvLOFZWVqKJ60841166 = VvvxDZAvLOFZWVqKJ80168983;     VvvxDZAvLOFZWVqKJ80168983 = VvvxDZAvLOFZWVqKJ5911744;     VvvxDZAvLOFZWVqKJ5911744 = VvvxDZAvLOFZWVqKJ51321094;     VvvxDZAvLOFZWVqKJ51321094 = VvvxDZAvLOFZWVqKJ12832577;     VvvxDZAvLOFZWVqKJ12832577 = VvvxDZAvLOFZWVqKJ23718312;     VvvxDZAvLOFZWVqKJ23718312 = VvvxDZAvLOFZWVqKJ79558389;     VvvxDZAvLOFZWVqKJ79558389 = VvvxDZAvLOFZWVqKJ21963618;     VvvxDZAvLOFZWVqKJ21963618 = VvvxDZAvLOFZWVqKJ46405091;     VvvxDZAvLOFZWVqKJ46405091 = VvvxDZAvLOFZWVqKJ1306263;     VvvxDZAvLOFZWVqKJ1306263 = VvvxDZAvLOFZWVqKJ57929807;     VvvxDZAvLOFZWVqKJ57929807 = VvvxDZAvLOFZWVqKJ23611176;     VvvxDZAvLOFZWVqKJ23611176 = VvvxDZAvLOFZWVqKJ94396650;     VvvxDZAvLOFZWVqKJ94396650 = VvvxDZAvLOFZWVqKJ26768888;     VvvxDZAvLOFZWVqKJ26768888 = VvvxDZAvLOFZWVqKJ55004300;     VvvxDZAvLOFZWVqKJ55004300 = VvvxDZAvLOFZWVqKJ32394262;     VvvxDZAvLOFZWVqKJ32394262 = VvvxDZAvLOFZWVqKJ96930653;     VvvxDZAvLOFZWVqKJ96930653 = VvvxDZAvLOFZWVqKJ84660684;     VvvxDZAvLOFZWVqKJ84660684 = VvvxDZAvLOFZWVqKJ79521861;     VvvxDZAvLOFZWVqKJ79521861 = VvvxDZAvLOFZWVqKJ20289822;     VvvxDZAvLOFZWVqKJ20289822 = VvvxDZAvLOFZWVqKJ49064736;     VvvxDZAvLOFZWVqKJ49064736 = VvvxDZAvLOFZWVqKJ80196900;     VvvxDZAvLOFZWVqKJ80196900 = VvvxDZAvLOFZWVqKJ76661468;     VvvxDZAvLOFZWVqKJ76661468 = VvvxDZAvLOFZWVqKJ18499816;     VvvxDZAvLOFZWVqKJ18499816 = VvvxDZAvLOFZWVqKJ34351363;     VvvxDZAvLOFZWVqKJ34351363 = VvvxDZAvLOFZWVqKJ10424166;     VvvxDZAvLOFZWVqKJ10424166 = VvvxDZAvLOFZWVqKJ19688396;     VvvxDZAvLOFZWVqKJ19688396 = VvvxDZAvLOFZWVqKJ76156394;     VvvxDZAvLOFZWVqKJ76156394 = VvvxDZAvLOFZWVqKJ62981239;     VvvxDZAvLOFZWVqKJ62981239 = VvvxDZAvLOFZWVqKJ94454919;     VvvxDZAvLOFZWVqKJ94454919 = VvvxDZAvLOFZWVqKJ258166;     VvvxDZAvLOFZWVqKJ258166 = VvvxDZAvLOFZWVqKJ1611507;     VvvxDZAvLOFZWVqKJ1611507 = VvvxDZAvLOFZWVqKJ86854095;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LpUeTvmUiNRdSBvB77153898() {     double dFdpmZxHzelZlAwHG22313064 = -161914471;    double dFdpmZxHzelZlAwHG64334829 = -286932167;    double dFdpmZxHzelZlAwHG35606265 = -515198172;    double dFdpmZxHzelZlAwHG63640383 = -340541248;    double dFdpmZxHzelZlAwHG81658745 = -763515604;    double dFdpmZxHzelZlAwHG74855968 = 93620729;    double dFdpmZxHzelZlAwHG38801225 = -909636593;    double dFdpmZxHzelZlAwHG90786901 = -268714036;    double dFdpmZxHzelZlAwHG82446877 = -61179821;    double dFdpmZxHzelZlAwHG35992989 = -183954365;    double dFdpmZxHzelZlAwHG26289760 = -910012959;    double dFdpmZxHzelZlAwHG28666852 = -658486150;    double dFdpmZxHzelZlAwHG8791385 = -121803431;    double dFdpmZxHzelZlAwHG30765364 = -678188209;    double dFdpmZxHzelZlAwHG35025287 = -108742124;    double dFdpmZxHzelZlAwHG95901674 = -519484802;    double dFdpmZxHzelZlAwHG92226404 = -489955784;    double dFdpmZxHzelZlAwHG44202937 = -778005513;    double dFdpmZxHzelZlAwHG81301088 = -79316455;    double dFdpmZxHzelZlAwHG98830735 = -933350552;    double dFdpmZxHzelZlAwHG57282578 = -973988897;    double dFdpmZxHzelZlAwHG53689186 = -226220514;    double dFdpmZxHzelZlAwHG1587812 = -563264667;    double dFdpmZxHzelZlAwHG4063920 = -623737674;    double dFdpmZxHzelZlAwHG48716968 = -985402542;    double dFdpmZxHzelZlAwHG14885645 = -827045600;    double dFdpmZxHzelZlAwHG61709619 = -619617099;    double dFdpmZxHzelZlAwHG19593510 = -43279652;    double dFdpmZxHzelZlAwHG16072697 = -429765726;    double dFdpmZxHzelZlAwHG77055033 = -997085219;    double dFdpmZxHzelZlAwHG10175505 = -903174325;    double dFdpmZxHzelZlAwHG75201382 = -201519522;    double dFdpmZxHzelZlAwHG41807276 = -36066907;    double dFdpmZxHzelZlAwHG44479746 = -445192514;    double dFdpmZxHzelZlAwHG82591787 = -895739751;    double dFdpmZxHzelZlAwHG48222632 = -769821596;    double dFdpmZxHzelZlAwHG55860446 = -720707184;    double dFdpmZxHzelZlAwHG36284778 = -858218108;    double dFdpmZxHzelZlAwHG62000730 = -636622638;    double dFdpmZxHzelZlAwHG21303979 = -754993987;    double dFdpmZxHzelZlAwHG58886735 = -873020530;    double dFdpmZxHzelZlAwHG11597224 = -95324923;    double dFdpmZxHzelZlAwHG93245947 = -657144332;    double dFdpmZxHzelZlAwHG69887507 = -445352952;    double dFdpmZxHzelZlAwHG52357626 = -936340033;    double dFdpmZxHzelZlAwHG63049169 = -401282100;    double dFdpmZxHzelZlAwHG29135987 = -510697461;    double dFdpmZxHzelZlAwHG92246469 = 13163198;    double dFdpmZxHzelZlAwHG46753451 = -843753954;    double dFdpmZxHzelZlAwHG61539810 = -290781271;    double dFdpmZxHzelZlAwHG55361693 = 26425008;    double dFdpmZxHzelZlAwHG90417061 = -816281272;    double dFdpmZxHzelZlAwHG26477652 = -954288409;    double dFdpmZxHzelZlAwHG25291290 = -581879195;    double dFdpmZxHzelZlAwHG19871321 = 87950637;    double dFdpmZxHzelZlAwHG68623878 = -935693957;    double dFdpmZxHzelZlAwHG62747018 = -723667500;    double dFdpmZxHzelZlAwHG31542345 = -891460498;    double dFdpmZxHzelZlAwHG14923416 = -355138706;    double dFdpmZxHzelZlAwHG66773101 = -936470004;    double dFdpmZxHzelZlAwHG13146350 = -286762172;    double dFdpmZxHzelZlAwHG19207715 = -766356942;    double dFdpmZxHzelZlAwHG74714204 = -838948311;    double dFdpmZxHzelZlAwHG5391845 = -64094602;    double dFdpmZxHzelZlAwHG25817484 = -280780041;    double dFdpmZxHzelZlAwHG51088377 = -608493437;    double dFdpmZxHzelZlAwHG86859576 = -522419244;    double dFdpmZxHzelZlAwHG64311638 = -676610917;    double dFdpmZxHzelZlAwHG48173576 = -782448458;    double dFdpmZxHzelZlAwHG86802655 = -338920528;    double dFdpmZxHzelZlAwHG40041228 = -798777619;    double dFdpmZxHzelZlAwHG55941626 = -631737676;    double dFdpmZxHzelZlAwHG82202206 = -41382875;    double dFdpmZxHzelZlAwHG59997109 = -324322468;    double dFdpmZxHzelZlAwHG39944001 = 39669977;    double dFdpmZxHzelZlAwHG45685354 = -778663974;    double dFdpmZxHzelZlAwHG60443238 = -569076183;    double dFdpmZxHzelZlAwHG31700304 = -17911715;    double dFdpmZxHzelZlAwHG51706293 = -687397642;    double dFdpmZxHzelZlAwHG85667798 = -484120442;    double dFdpmZxHzelZlAwHG85749658 = -216348140;    double dFdpmZxHzelZlAwHG69463149 = -532780297;    double dFdpmZxHzelZlAwHG72840058 = -199525698;    double dFdpmZxHzelZlAwHG54532887 = -38984456;    double dFdpmZxHzelZlAwHG21693341 = -923510227;    double dFdpmZxHzelZlAwHG19758444 = 13106947;    double dFdpmZxHzelZlAwHG48723731 = -247231114;    double dFdpmZxHzelZlAwHG16515987 = -454187712;    double dFdpmZxHzelZlAwHG24608426 = -433143152;    double dFdpmZxHzelZlAwHG13967910 = -960045794;    double dFdpmZxHzelZlAwHG85475613 = 53845903;    double dFdpmZxHzelZlAwHG24318101 = -829246686;    double dFdpmZxHzelZlAwHG21361363 = -403079402;    double dFdpmZxHzelZlAwHG95227629 = -700152635;    double dFdpmZxHzelZlAwHG8157630 = -368231815;    double dFdpmZxHzelZlAwHG39679021 = -6663589;    double dFdpmZxHzelZlAwHG36883020 = -256376613;    double dFdpmZxHzelZlAwHG87854103 = -493049730;    double dFdpmZxHzelZlAwHG44070023 = -64572911;    double dFdpmZxHzelZlAwHG1269250 = -161914471;     dFdpmZxHzelZlAwHG22313064 = dFdpmZxHzelZlAwHG64334829;     dFdpmZxHzelZlAwHG64334829 = dFdpmZxHzelZlAwHG35606265;     dFdpmZxHzelZlAwHG35606265 = dFdpmZxHzelZlAwHG63640383;     dFdpmZxHzelZlAwHG63640383 = dFdpmZxHzelZlAwHG81658745;     dFdpmZxHzelZlAwHG81658745 = dFdpmZxHzelZlAwHG74855968;     dFdpmZxHzelZlAwHG74855968 = dFdpmZxHzelZlAwHG38801225;     dFdpmZxHzelZlAwHG38801225 = dFdpmZxHzelZlAwHG90786901;     dFdpmZxHzelZlAwHG90786901 = dFdpmZxHzelZlAwHG82446877;     dFdpmZxHzelZlAwHG82446877 = dFdpmZxHzelZlAwHG35992989;     dFdpmZxHzelZlAwHG35992989 = dFdpmZxHzelZlAwHG26289760;     dFdpmZxHzelZlAwHG26289760 = dFdpmZxHzelZlAwHG28666852;     dFdpmZxHzelZlAwHG28666852 = dFdpmZxHzelZlAwHG8791385;     dFdpmZxHzelZlAwHG8791385 = dFdpmZxHzelZlAwHG30765364;     dFdpmZxHzelZlAwHG30765364 = dFdpmZxHzelZlAwHG35025287;     dFdpmZxHzelZlAwHG35025287 = dFdpmZxHzelZlAwHG95901674;     dFdpmZxHzelZlAwHG95901674 = dFdpmZxHzelZlAwHG92226404;     dFdpmZxHzelZlAwHG92226404 = dFdpmZxHzelZlAwHG44202937;     dFdpmZxHzelZlAwHG44202937 = dFdpmZxHzelZlAwHG81301088;     dFdpmZxHzelZlAwHG81301088 = dFdpmZxHzelZlAwHG98830735;     dFdpmZxHzelZlAwHG98830735 = dFdpmZxHzelZlAwHG57282578;     dFdpmZxHzelZlAwHG57282578 = dFdpmZxHzelZlAwHG53689186;     dFdpmZxHzelZlAwHG53689186 = dFdpmZxHzelZlAwHG1587812;     dFdpmZxHzelZlAwHG1587812 = dFdpmZxHzelZlAwHG4063920;     dFdpmZxHzelZlAwHG4063920 = dFdpmZxHzelZlAwHG48716968;     dFdpmZxHzelZlAwHG48716968 = dFdpmZxHzelZlAwHG14885645;     dFdpmZxHzelZlAwHG14885645 = dFdpmZxHzelZlAwHG61709619;     dFdpmZxHzelZlAwHG61709619 = dFdpmZxHzelZlAwHG19593510;     dFdpmZxHzelZlAwHG19593510 = dFdpmZxHzelZlAwHG16072697;     dFdpmZxHzelZlAwHG16072697 = dFdpmZxHzelZlAwHG77055033;     dFdpmZxHzelZlAwHG77055033 = dFdpmZxHzelZlAwHG10175505;     dFdpmZxHzelZlAwHG10175505 = dFdpmZxHzelZlAwHG75201382;     dFdpmZxHzelZlAwHG75201382 = dFdpmZxHzelZlAwHG41807276;     dFdpmZxHzelZlAwHG41807276 = dFdpmZxHzelZlAwHG44479746;     dFdpmZxHzelZlAwHG44479746 = dFdpmZxHzelZlAwHG82591787;     dFdpmZxHzelZlAwHG82591787 = dFdpmZxHzelZlAwHG48222632;     dFdpmZxHzelZlAwHG48222632 = dFdpmZxHzelZlAwHG55860446;     dFdpmZxHzelZlAwHG55860446 = dFdpmZxHzelZlAwHG36284778;     dFdpmZxHzelZlAwHG36284778 = dFdpmZxHzelZlAwHG62000730;     dFdpmZxHzelZlAwHG62000730 = dFdpmZxHzelZlAwHG21303979;     dFdpmZxHzelZlAwHG21303979 = dFdpmZxHzelZlAwHG58886735;     dFdpmZxHzelZlAwHG58886735 = dFdpmZxHzelZlAwHG11597224;     dFdpmZxHzelZlAwHG11597224 = dFdpmZxHzelZlAwHG93245947;     dFdpmZxHzelZlAwHG93245947 = dFdpmZxHzelZlAwHG69887507;     dFdpmZxHzelZlAwHG69887507 = dFdpmZxHzelZlAwHG52357626;     dFdpmZxHzelZlAwHG52357626 = dFdpmZxHzelZlAwHG63049169;     dFdpmZxHzelZlAwHG63049169 = dFdpmZxHzelZlAwHG29135987;     dFdpmZxHzelZlAwHG29135987 = dFdpmZxHzelZlAwHG92246469;     dFdpmZxHzelZlAwHG92246469 = dFdpmZxHzelZlAwHG46753451;     dFdpmZxHzelZlAwHG46753451 = dFdpmZxHzelZlAwHG61539810;     dFdpmZxHzelZlAwHG61539810 = dFdpmZxHzelZlAwHG55361693;     dFdpmZxHzelZlAwHG55361693 = dFdpmZxHzelZlAwHG90417061;     dFdpmZxHzelZlAwHG90417061 = dFdpmZxHzelZlAwHG26477652;     dFdpmZxHzelZlAwHG26477652 = dFdpmZxHzelZlAwHG25291290;     dFdpmZxHzelZlAwHG25291290 = dFdpmZxHzelZlAwHG19871321;     dFdpmZxHzelZlAwHG19871321 = dFdpmZxHzelZlAwHG68623878;     dFdpmZxHzelZlAwHG68623878 = dFdpmZxHzelZlAwHG62747018;     dFdpmZxHzelZlAwHG62747018 = dFdpmZxHzelZlAwHG31542345;     dFdpmZxHzelZlAwHG31542345 = dFdpmZxHzelZlAwHG14923416;     dFdpmZxHzelZlAwHG14923416 = dFdpmZxHzelZlAwHG66773101;     dFdpmZxHzelZlAwHG66773101 = dFdpmZxHzelZlAwHG13146350;     dFdpmZxHzelZlAwHG13146350 = dFdpmZxHzelZlAwHG19207715;     dFdpmZxHzelZlAwHG19207715 = dFdpmZxHzelZlAwHG74714204;     dFdpmZxHzelZlAwHG74714204 = dFdpmZxHzelZlAwHG5391845;     dFdpmZxHzelZlAwHG5391845 = dFdpmZxHzelZlAwHG25817484;     dFdpmZxHzelZlAwHG25817484 = dFdpmZxHzelZlAwHG51088377;     dFdpmZxHzelZlAwHG51088377 = dFdpmZxHzelZlAwHG86859576;     dFdpmZxHzelZlAwHG86859576 = dFdpmZxHzelZlAwHG64311638;     dFdpmZxHzelZlAwHG64311638 = dFdpmZxHzelZlAwHG48173576;     dFdpmZxHzelZlAwHG48173576 = dFdpmZxHzelZlAwHG86802655;     dFdpmZxHzelZlAwHG86802655 = dFdpmZxHzelZlAwHG40041228;     dFdpmZxHzelZlAwHG40041228 = dFdpmZxHzelZlAwHG55941626;     dFdpmZxHzelZlAwHG55941626 = dFdpmZxHzelZlAwHG82202206;     dFdpmZxHzelZlAwHG82202206 = dFdpmZxHzelZlAwHG59997109;     dFdpmZxHzelZlAwHG59997109 = dFdpmZxHzelZlAwHG39944001;     dFdpmZxHzelZlAwHG39944001 = dFdpmZxHzelZlAwHG45685354;     dFdpmZxHzelZlAwHG45685354 = dFdpmZxHzelZlAwHG60443238;     dFdpmZxHzelZlAwHG60443238 = dFdpmZxHzelZlAwHG31700304;     dFdpmZxHzelZlAwHG31700304 = dFdpmZxHzelZlAwHG51706293;     dFdpmZxHzelZlAwHG51706293 = dFdpmZxHzelZlAwHG85667798;     dFdpmZxHzelZlAwHG85667798 = dFdpmZxHzelZlAwHG85749658;     dFdpmZxHzelZlAwHG85749658 = dFdpmZxHzelZlAwHG69463149;     dFdpmZxHzelZlAwHG69463149 = dFdpmZxHzelZlAwHG72840058;     dFdpmZxHzelZlAwHG72840058 = dFdpmZxHzelZlAwHG54532887;     dFdpmZxHzelZlAwHG54532887 = dFdpmZxHzelZlAwHG21693341;     dFdpmZxHzelZlAwHG21693341 = dFdpmZxHzelZlAwHG19758444;     dFdpmZxHzelZlAwHG19758444 = dFdpmZxHzelZlAwHG48723731;     dFdpmZxHzelZlAwHG48723731 = dFdpmZxHzelZlAwHG16515987;     dFdpmZxHzelZlAwHG16515987 = dFdpmZxHzelZlAwHG24608426;     dFdpmZxHzelZlAwHG24608426 = dFdpmZxHzelZlAwHG13967910;     dFdpmZxHzelZlAwHG13967910 = dFdpmZxHzelZlAwHG85475613;     dFdpmZxHzelZlAwHG85475613 = dFdpmZxHzelZlAwHG24318101;     dFdpmZxHzelZlAwHG24318101 = dFdpmZxHzelZlAwHG21361363;     dFdpmZxHzelZlAwHG21361363 = dFdpmZxHzelZlAwHG95227629;     dFdpmZxHzelZlAwHG95227629 = dFdpmZxHzelZlAwHG8157630;     dFdpmZxHzelZlAwHG8157630 = dFdpmZxHzelZlAwHG39679021;     dFdpmZxHzelZlAwHG39679021 = dFdpmZxHzelZlAwHG36883020;     dFdpmZxHzelZlAwHG36883020 = dFdpmZxHzelZlAwHG87854103;     dFdpmZxHzelZlAwHG87854103 = dFdpmZxHzelZlAwHG44070023;     dFdpmZxHzelZlAwHG44070023 = dFdpmZxHzelZlAwHG1269250;     dFdpmZxHzelZlAwHG1269250 = dFdpmZxHzelZlAwHG22313064;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zayKRyZmGggNCCWY92202966() {     double XSgdWwMovePcwRcfd28430170 = -273816360;    double XSgdWwMovePcwRcfd7708109 = 3468053;    double XSgdWwMovePcwRcfd51648448 = -666031093;    double XSgdWwMovePcwRcfd90715163 = -531289878;    double XSgdWwMovePcwRcfd50554234 = -184165475;    double XSgdWwMovePcwRcfd66273170 = -170875506;    double XSgdWwMovePcwRcfd99290190 = 51398430;    double XSgdWwMovePcwRcfd40546792 = -555587950;    double XSgdWwMovePcwRcfd47089373 = -929212486;    double XSgdWwMovePcwRcfd77222648 = -512929843;    double XSgdWwMovePcwRcfd86400049 = -787450546;    double XSgdWwMovePcwRcfd60489894 = -2892352;    double XSgdWwMovePcwRcfd95000502 = -561694520;    double XSgdWwMovePcwRcfd13909041 = -110426777;    double XSgdWwMovePcwRcfd30767714 = -799204291;    double XSgdWwMovePcwRcfd37407068 = -492484346;    double XSgdWwMovePcwRcfd86062407 = -6500896;    double XSgdWwMovePcwRcfd41256090 = -443528156;    double XSgdWwMovePcwRcfd43130930 = -485085551;    double XSgdWwMovePcwRcfd79380900 = -190253472;    double XSgdWwMovePcwRcfd45835771 = -829476353;    double XSgdWwMovePcwRcfd40210209 = -134150230;    double XSgdWwMovePcwRcfd37178603 = -309782154;    double XSgdWwMovePcwRcfd88056293 = -846042662;    double XSgdWwMovePcwRcfd92416617 = -258598575;    double XSgdWwMovePcwRcfd39607320 = -705700924;    double XSgdWwMovePcwRcfd60169397 = -520279302;    double XSgdWwMovePcwRcfd32700635 = -109208606;    double XSgdWwMovePcwRcfd82204952 = -652148430;    double XSgdWwMovePcwRcfd45670670 = -814556086;    double XSgdWwMovePcwRcfd2418055 = -816495263;    double XSgdWwMovePcwRcfd72812004 = 89033662;    double XSgdWwMovePcwRcfd51518224 = 71319723;    double XSgdWwMovePcwRcfd39310311 = -196734280;    double XSgdWwMovePcwRcfd44066440 = -434472857;    double XSgdWwMovePcwRcfd16948545 = -983847704;    double XSgdWwMovePcwRcfd37122381 = -674343053;    double XSgdWwMovePcwRcfd70374991 = -629724118;    double XSgdWwMovePcwRcfd35610984 = -630546135;    double XSgdWwMovePcwRcfd8805917 = -128433210;    double XSgdWwMovePcwRcfd64555959 = -938294424;    double XSgdWwMovePcwRcfd43747681 = -365467201;    double XSgdWwMovePcwRcfd71312926 = -737948916;    double XSgdWwMovePcwRcfd57295248 = -464695890;    double XSgdWwMovePcwRcfd64310091 = -598238187;    double XSgdWwMovePcwRcfd74988364 = -386498217;    double XSgdWwMovePcwRcfd46914561 = -629472998;    double XSgdWwMovePcwRcfd203411 = 3267665;    double XSgdWwMovePcwRcfd56429180 = -929490942;    double XSgdWwMovePcwRcfd74451462 = -377792079;    double XSgdWwMovePcwRcfd91062371 = -915868664;    double XSgdWwMovePcwRcfd1226782 = -617007593;    double XSgdWwMovePcwRcfd74726109 = -746077997;    double XSgdWwMovePcwRcfd21260170 = -902349340;    double XSgdWwMovePcwRcfd46367195 = -612581146;    double XSgdWwMovePcwRcfd88219960 = -39666130;    double XSgdWwMovePcwRcfd70529505 = -686749794;    double XSgdWwMovePcwRcfd63592155 = -819988432;    double XSgdWwMovePcwRcfd98298546 = -172691304;    double XSgdWwMovePcwRcfd10946915 = -478464552;    double XSgdWwMovePcwRcfd6103774 = -650596204;    double XSgdWwMovePcwRcfd66589555 = -839392965;    double XSgdWwMovePcwRcfd58341840 = -903439520;    double XSgdWwMovePcwRcfd1418704 = -14656400;    double XSgdWwMovePcwRcfd74804594 = -696434581;    double XSgdWwMovePcwRcfd13588046 = -776484208;    double XSgdWwMovePcwRcfd8971670 = 25787925;    double XSgdWwMovePcwRcfd55690192 = -264960241;    double XSgdWwMovePcwRcfd69842600 = -675953920;    double XSgdWwMovePcwRcfd13819169 = -815356587;    double XSgdWwMovePcwRcfd284688 = -818141293;    double XSgdWwMovePcwRcfd15687417 = -376776779;    double XSgdWwMovePcwRcfd5645106 = -812982021;    double XSgdWwMovePcwRcfd34325013 = -256652342;    double XSgdWwMovePcwRcfd14824942 = -251959048;    double XSgdWwMovePcwRcfd2088091 = -364009152;    double XSgdWwMovePcwRcfd68897283 = -396201315;    double XSgdWwMovePcwRcfd79883355 = -845086264;    double XSgdWwMovePcwRcfd23746202 = -147804475;    double XSgdWwMovePcwRcfd17428253 = -872100359;    double XSgdWwMovePcwRcfd92692759 = 23772074;    double XSgdWwMovePcwRcfd59965987 = -423546968;    double XSgdWwMovePcwRcfd76271455 = -179717664;    double XSgdWwMovePcwRcfd7753491 = -174356352;    double XSgdWwMovePcwRcfd54608299 = -898687423;    double XSgdWwMovePcwRcfd1191274 = -99487671;    double XSgdWwMovePcwRcfd98085895 = -164888342;    double XSgdWwMovePcwRcfd30258055 = -26330938;    double XSgdWwMovePcwRcfd92943115 = -584153135;    double XSgdWwMovePcwRcfd55846479 = -294806727;    double XSgdWwMovePcwRcfd46419039 = -197097910;    double XSgdWwMovePcwRcfd73530225 = -854354622;    double XSgdWwMovePcwRcfd72076444 = -357032815;    double XSgdWwMovePcwRcfd24664070 = -52081584;    double XSgdWwMovePcwRcfd2702144 = -477837006;    double XSgdWwMovePcwRcfd97966403 = 1098540;    double XSgdWwMovePcwRcfd85405841 = -462027682;    double XSgdWwMovePcwRcfd69894223 = -623292516;    double XSgdWwMovePcwRcfd82490653 = -768261310;    double XSgdWwMovePcwRcfd50722046 = -273816360;     XSgdWwMovePcwRcfd28430170 = XSgdWwMovePcwRcfd7708109;     XSgdWwMovePcwRcfd7708109 = XSgdWwMovePcwRcfd51648448;     XSgdWwMovePcwRcfd51648448 = XSgdWwMovePcwRcfd90715163;     XSgdWwMovePcwRcfd90715163 = XSgdWwMovePcwRcfd50554234;     XSgdWwMovePcwRcfd50554234 = XSgdWwMovePcwRcfd66273170;     XSgdWwMovePcwRcfd66273170 = XSgdWwMovePcwRcfd99290190;     XSgdWwMovePcwRcfd99290190 = XSgdWwMovePcwRcfd40546792;     XSgdWwMovePcwRcfd40546792 = XSgdWwMovePcwRcfd47089373;     XSgdWwMovePcwRcfd47089373 = XSgdWwMovePcwRcfd77222648;     XSgdWwMovePcwRcfd77222648 = XSgdWwMovePcwRcfd86400049;     XSgdWwMovePcwRcfd86400049 = XSgdWwMovePcwRcfd60489894;     XSgdWwMovePcwRcfd60489894 = XSgdWwMovePcwRcfd95000502;     XSgdWwMovePcwRcfd95000502 = XSgdWwMovePcwRcfd13909041;     XSgdWwMovePcwRcfd13909041 = XSgdWwMovePcwRcfd30767714;     XSgdWwMovePcwRcfd30767714 = XSgdWwMovePcwRcfd37407068;     XSgdWwMovePcwRcfd37407068 = XSgdWwMovePcwRcfd86062407;     XSgdWwMovePcwRcfd86062407 = XSgdWwMovePcwRcfd41256090;     XSgdWwMovePcwRcfd41256090 = XSgdWwMovePcwRcfd43130930;     XSgdWwMovePcwRcfd43130930 = XSgdWwMovePcwRcfd79380900;     XSgdWwMovePcwRcfd79380900 = XSgdWwMovePcwRcfd45835771;     XSgdWwMovePcwRcfd45835771 = XSgdWwMovePcwRcfd40210209;     XSgdWwMovePcwRcfd40210209 = XSgdWwMovePcwRcfd37178603;     XSgdWwMovePcwRcfd37178603 = XSgdWwMovePcwRcfd88056293;     XSgdWwMovePcwRcfd88056293 = XSgdWwMovePcwRcfd92416617;     XSgdWwMovePcwRcfd92416617 = XSgdWwMovePcwRcfd39607320;     XSgdWwMovePcwRcfd39607320 = XSgdWwMovePcwRcfd60169397;     XSgdWwMovePcwRcfd60169397 = XSgdWwMovePcwRcfd32700635;     XSgdWwMovePcwRcfd32700635 = XSgdWwMovePcwRcfd82204952;     XSgdWwMovePcwRcfd82204952 = XSgdWwMovePcwRcfd45670670;     XSgdWwMovePcwRcfd45670670 = XSgdWwMovePcwRcfd2418055;     XSgdWwMovePcwRcfd2418055 = XSgdWwMovePcwRcfd72812004;     XSgdWwMovePcwRcfd72812004 = XSgdWwMovePcwRcfd51518224;     XSgdWwMovePcwRcfd51518224 = XSgdWwMovePcwRcfd39310311;     XSgdWwMovePcwRcfd39310311 = XSgdWwMovePcwRcfd44066440;     XSgdWwMovePcwRcfd44066440 = XSgdWwMovePcwRcfd16948545;     XSgdWwMovePcwRcfd16948545 = XSgdWwMovePcwRcfd37122381;     XSgdWwMovePcwRcfd37122381 = XSgdWwMovePcwRcfd70374991;     XSgdWwMovePcwRcfd70374991 = XSgdWwMovePcwRcfd35610984;     XSgdWwMovePcwRcfd35610984 = XSgdWwMovePcwRcfd8805917;     XSgdWwMovePcwRcfd8805917 = XSgdWwMovePcwRcfd64555959;     XSgdWwMovePcwRcfd64555959 = XSgdWwMovePcwRcfd43747681;     XSgdWwMovePcwRcfd43747681 = XSgdWwMovePcwRcfd71312926;     XSgdWwMovePcwRcfd71312926 = XSgdWwMovePcwRcfd57295248;     XSgdWwMovePcwRcfd57295248 = XSgdWwMovePcwRcfd64310091;     XSgdWwMovePcwRcfd64310091 = XSgdWwMovePcwRcfd74988364;     XSgdWwMovePcwRcfd74988364 = XSgdWwMovePcwRcfd46914561;     XSgdWwMovePcwRcfd46914561 = XSgdWwMovePcwRcfd203411;     XSgdWwMovePcwRcfd203411 = XSgdWwMovePcwRcfd56429180;     XSgdWwMovePcwRcfd56429180 = XSgdWwMovePcwRcfd74451462;     XSgdWwMovePcwRcfd74451462 = XSgdWwMovePcwRcfd91062371;     XSgdWwMovePcwRcfd91062371 = XSgdWwMovePcwRcfd1226782;     XSgdWwMovePcwRcfd1226782 = XSgdWwMovePcwRcfd74726109;     XSgdWwMovePcwRcfd74726109 = XSgdWwMovePcwRcfd21260170;     XSgdWwMovePcwRcfd21260170 = XSgdWwMovePcwRcfd46367195;     XSgdWwMovePcwRcfd46367195 = XSgdWwMovePcwRcfd88219960;     XSgdWwMovePcwRcfd88219960 = XSgdWwMovePcwRcfd70529505;     XSgdWwMovePcwRcfd70529505 = XSgdWwMovePcwRcfd63592155;     XSgdWwMovePcwRcfd63592155 = XSgdWwMovePcwRcfd98298546;     XSgdWwMovePcwRcfd98298546 = XSgdWwMovePcwRcfd10946915;     XSgdWwMovePcwRcfd10946915 = XSgdWwMovePcwRcfd6103774;     XSgdWwMovePcwRcfd6103774 = XSgdWwMovePcwRcfd66589555;     XSgdWwMovePcwRcfd66589555 = XSgdWwMovePcwRcfd58341840;     XSgdWwMovePcwRcfd58341840 = XSgdWwMovePcwRcfd1418704;     XSgdWwMovePcwRcfd1418704 = XSgdWwMovePcwRcfd74804594;     XSgdWwMovePcwRcfd74804594 = XSgdWwMovePcwRcfd13588046;     XSgdWwMovePcwRcfd13588046 = XSgdWwMovePcwRcfd8971670;     XSgdWwMovePcwRcfd8971670 = XSgdWwMovePcwRcfd55690192;     XSgdWwMovePcwRcfd55690192 = XSgdWwMovePcwRcfd69842600;     XSgdWwMovePcwRcfd69842600 = XSgdWwMovePcwRcfd13819169;     XSgdWwMovePcwRcfd13819169 = XSgdWwMovePcwRcfd284688;     XSgdWwMovePcwRcfd284688 = XSgdWwMovePcwRcfd15687417;     XSgdWwMovePcwRcfd15687417 = XSgdWwMovePcwRcfd5645106;     XSgdWwMovePcwRcfd5645106 = XSgdWwMovePcwRcfd34325013;     XSgdWwMovePcwRcfd34325013 = XSgdWwMovePcwRcfd14824942;     XSgdWwMovePcwRcfd14824942 = XSgdWwMovePcwRcfd2088091;     XSgdWwMovePcwRcfd2088091 = XSgdWwMovePcwRcfd68897283;     XSgdWwMovePcwRcfd68897283 = XSgdWwMovePcwRcfd79883355;     XSgdWwMovePcwRcfd79883355 = XSgdWwMovePcwRcfd23746202;     XSgdWwMovePcwRcfd23746202 = XSgdWwMovePcwRcfd17428253;     XSgdWwMovePcwRcfd17428253 = XSgdWwMovePcwRcfd92692759;     XSgdWwMovePcwRcfd92692759 = XSgdWwMovePcwRcfd59965987;     XSgdWwMovePcwRcfd59965987 = XSgdWwMovePcwRcfd76271455;     XSgdWwMovePcwRcfd76271455 = XSgdWwMovePcwRcfd7753491;     XSgdWwMovePcwRcfd7753491 = XSgdWwMovePcwRcfd54608299;     XSgdWwMovePcwRcfd54608299 = XSgdWwMovePcwRcfd1191274;     XSgdWwMovePcwRcfd1191274 = XSgdWwMovePcwRcfd98085895;     XSgdWwMovePcwRcfd98085895 = XSgdWwMovePcwRcfd30258055;     XSgdWwMovePcwRcfd30258055 = XSgdWwMovePcwRcfd92943115;     XSgdWwMovePcwRcfd92943115 = XSgdWwMovePcwRcfd55846479;     XSgdWwMovePcwRcfd55846479 = XSgdWwMovePcwRcfd46419039;     XSgdWwMovePcwRcfd46419039 = XSgdWwMovePcwRcfd73530225;     XSgdWwMovePcwRcfd73530225 = XSgdWwMovePcwRcfd72076444;     XSgdWwMovePcwRcfd72076444 = XSgdWwMovePcwRcfd24664070;     XSgdWwMovePcwRcfd24664070 = XSgdWwMovePcwRcfd2702144;     XSgdWwMovePcwRcfd2702144 = XSgdWwMovePcwRcfd97966403;     XSgdWwMovePcwRcfd97966403 = XSgdWwMovePcwRcfd85405841;     XSgdWwMovePcwRcfd85405841 = XSgdWwMovePcwRcfd69894223;     XSgdWwMovePcwRcfd69894223 = XSgdWwMovePcwRcfd82490653;     XSgdWwMovePcwRcfd82490653 = XSgdWwMovePcwRcfd50722046;     XSgdWwMovePcwRcfd50722046 = XSgdWwMovePcwRcfd28430170;}
// Junk Finished
