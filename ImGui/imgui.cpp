// dear imgui, v1.54 WIP
// (main code and documentation)

// Call and read ImGui::ShowDemoWindow() in imgui_demo.cpp for demo code.
// Newcomers, read 'Programmer guide' below for notes on how to setup Dear ImGui in your codebase.
// Get latest version at https://github.com/ocornut/imgui
// Releases change-log at https://github.com/ocornut/imgui/releases
// Gallery (please post your screenshots/video there!): https://github.com/ocornut/imgui/issues/1269
// Developed by Omar Cornut and every direct or indirect contributors to the GitHub.
// This library is free but I need your support to sustain development and maintenance.
// If you work for a company, please consider financial support, see Readme. For individuals: https://www.patreon.com/imgui

/*

Index
- MISSION STATEMENT
- END-USER GUIDE
- PROGRAMMER GUIDE (read me!)
- Read first
- How to update to a newer version of Dear ImGui
- Getting started with integrating Dear ImGui in your code/engine
- API BREAKING CHANGES (read me when you update!)
- ISSUES & TODO LIST
- FREQUENTLY ASKED QUESTIONS (FAQ), TIPS
- How can I help?
- How can I dipslay an image? What is ImTextureID, how does it works?
- How can I have multiple widgets with the same label? Can I have widget without a label? (Yes). A primer on labels and the ID stack.
- How can I tell when Dear ImGui wants my mouse/keyboard inputs VS when I can pass them to my application?
- How can I load a different font than the default?
- How can I easily use icons in my application?
- How can I load multiple fonts?
- How can I display and input non-latin characters such as Chinese, Japanese, Korean, Cyrillic?
- How can I preserve my Dear ImGui context across reloading a DLL? (loss of the global/static variables)
- How can I use the drawing facilities without an ImGui window? (using ImDrawList API)
- I integrated Dear ImGui in my engine and the text or lines are blurry..
- I integrated Dear ImGui in my engine and some elements are clipping or disappearing when I move windows around..
- ISSUES & TODO-LIST
- CODE


MISSION STATEMENT
=================

- Easy to use to create code-driven and data-driven tools
- Easy to use to create ad hoc short-lived tools and long-lived, more elaborate tools
- Easy to hack and improve
- Minimize screen real-estate usage
- Minimize setup and maintenance
- Minimize state storage on user side
- Portable, minimize dependencies, run on target (consoles, phones, etc.)
- Efficient runtime and memory consumption (NB- we do allocate when "growing" content e.g. creating a window, opening a tree node
for the first time, etc. but a typical frame won't allocate anything)

Designed for developers and content-creators, not the typical end-user! Some of the weaknesses includes:
- Doesn't look fancy, doesn't animate
- Limited layout features, intricate layouts are typically crafted in code


END-USER GUIDE
==============

- Double-click on title bar to collapse window.
- Click upper right corner to close a window, available when 'bool* p_open' is passed to ImGui::Begin().
- Click and drag on lower right corner to resize window (double-click to auto fit window to its contents).
- Click and drag on any empty space to move window.
- TAB/SHIFT+TAB to cycle through keyboard editable fields.
- CTRL+Click on a slider or drag box to input value as text.
- Use mouse wheel to scroll.
- Text editor:
- Hold SHIFT or use mouse to select text.
- CTRL+Left/Right to word jump.
- CTRL+Shift+Left/Right to select words.
- CTRL+A our Double-Click to select all.
- CTRL+X,CTRL+C,CTRL+V to use OS clipboard/
- CTRL+Z,CTRL+Y to undo/redo.
- ESCAPE to revert text to its original value.
- You can apply arithmetic operators +,*,/ on numerical values. Use +- to subtract (because - would set a negative value!)
- Controls are automatically adjusted for OSX to match standard OSX text editing operations.


PROGRAMMER GUIDE
================

READ FIRST

- Read the FAQ below this section!
- Your code creates the UI, if your code doesn't run the UI is gone! == very dynamic UI, no construction/destructions steps, less data retention
on your side, no state duplication, less sync, less bugs.
- Call and read ImGui::ShowDemoWindow() for demo code demonstrating most features.
- You can learn about immediate-mode gui principles at http://www.johno.se/book/imgui.html or watch http://mollyrocket.com/861

HOW TO UPDATE TO A NEWER VERSION OF DEAR IMGUI

- Overwrite all the sources files except for imconfig.h (if you have made modification to your copy of imconfig.h)
- Read the "API BREAKING CHANGES" section (below). This is where we list occasional API breaking changes.
If a function/type has been renamed / or marked obsolete, try to fix the name in your code before it is permanently removed from the public API.
If you have a problem with a missing function/symbols, search for its name in the code, there will likely be a comment about it.
Please report any issue to the GitHub page!
- Try to keep your copy of dear imgui reasonably up to date.

GETTING STARTED WITH INTEGRATING DEAR IMGUI IN YOUR CODE/ENGINE

- Add the Dear ImGui source files to your projects, using your preferred build system.
It is recommended you build the .cpp files as part of your project and not as a library.
- You can later customize the imconfig.h file to tweak some compilation time behavior, such as integrating imgui types with your own maths types.
- See examples/ folder for standalone sample applications.
- You may be able to grab and copy a ready made imgui_impl_*** file from the examples/.
- When using Dear ImGui, your programming IDE is your friend: follow the declaration of variables, functions and types to find comments about them.

- Init: retrieve the ImGuiIO structure with ImGui::GetIO() and fill the fields marked 'Settings': at minimum you need to set io.DisplaySize
(application resolution). Later on you will fill your keyboard mapping, clipboard handlers, and other advanced features but for a basic
integration you don't need to worry about it all.
- Init: call io.Fonts->GetTexDataAsRGBA32(...), it will build the font atlas texture, then load the texture pixels into graphics memory.
- Every frame:
- In your main loop as early a possible, fill the IO fields marked 'Input' (e.g. mouse position, buttons, keyboard info, etc.)
- Call ImGui::NewFrame() to begin the frame
- You can use any ImGui function you want between NewFrame() and Render()
- Call ImGui::Render() as late as you can to end the frame and finalize render data. it will call your io.RenderDrawListFn handler.
(Even if you don't render, call Render() and ignore the callback, or call EndFrame() instead. Otherwhise some features will break)
- All rendering information are stored into command-lists until ImGui::Render() is called.
- Dear ImGui never touches or knows about your GPU state. the only function that knows about GPU is the RenderDrawListFn handler that you provide.
- Effectively it means you can create widgets at any time in your code, regardless of considerations of being in "update" vs "render" phases
of your own application.
- Refer to the examples applications in the examples/ folder for instruction on how to setup your code.
- A minimal application skeleton may be:

// Application init
ImGuiIO& io = ImGui::GetIO();
io.DisplaySize.x = 1920.0f;
io.DisplaySize.y = 1280.0f;
io.RenderDrawListsFn = MyRenderFunction;  // Setup a render function, or set to NULL and call GetDrawData() after Render() to access render data.
// TODO: Fill others settings of the io structure later.

// Load texture atlas (there is a default font so you don't need to care about choosing a font yet)
unsigned char* pixels;
int width, height;
io.Fonts->GetTexDataAsRGBA32(pixels, &width, &height);
// TODO: At this points you've got the texture data and you need to upload that your your graphic system:
MyTexture* texture = MyEngine::CreateTextureFromMemoryPixels(pixels, width, height, TEXTURE_TYPE_RGBA)
// TODO: Store your texture pointer/identifier (whatever your engine uses) in 'io.Fonts->TexID'. This will be passed back to your via the renderer.
io.Fonts->TexID = (void*)texture;

// Application main loop
while (true)
{
// Setup low-level inputs (e.g. on Win32, GetKeyboardState(), or write to those fields from your Windows message loop handlers, etc.)
ImGuiIO& io = ImGui::GetIO();
io.DeltaTime = 1.0f/60.0f;
io.MousePos = mouse_pos;
io.MouseDown[0] = mouse_button_0;
io.MouseDown[1] = mouse_button_1;

// Call NewFrame(), after this point you can use ImGui::* functions anytime
ImGui::NewFrame();

// Most of your application code here
MyGameUpdate(); // may use any ImGui functions, e.g. ImGui::Begin("My window"); ImGui::Text("Hello, world!"); ImGui::End();
MyGameRender(); // may use any ImGui functions as well!

// Render & swap video buffers
ImGui::Render();
SwapBuffers();
}

- A minimal render function skeleton may be:

void void MyRenderFunction(ImDrawData* draw_data)
{
// TODO: Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
// TODO: Setup viewport, orthographic projection matrix
// TODO: Setup shader: vertex { float2 pos, float2 uv, u32 color }, fragment shader sample color from 1 texture, multiply by vertex color.
for (int n = 0; n < draw_data->CmdListsCount; n++)
{
const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;  // vertex buffer generated by ImGui
const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;   // index buffer generated by ImGui
for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
{
const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
if (pcmd->UserCallback)
{
pcmd->UserCallback(cmd_list, pcmd);
}
else
{
// The texture for the draw call is specified by pcmd->TextureId.
// The vast majority of draw calls with use the imgui texture atlas, which value you have set yourself during initialization.
MyEngineBindTexture(pcmd->TextureId);

// We are using scissoring to clip some objects. All low-level graphics API supports it.
// If your engine doesn't support scissoring yet, you will get some small glitches (some elements outside their bounds) which you can fix later.
MyEngineScissor((int)pcmd->ClipRect.x, (int)pcmd->ClipRect.y, (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));

// Render 'pcmd->ElemCount/3' indexed triangles.
// By default the indices ImDrawIdx are 16-bits, you can change them to 32-bits if your engine doesn't support 16-bits indices.
MyEngineDrawIndexedTriangles(pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer, vtx_buffer);
}
idx_buffer += pcmd->ElemCount;
}
}
}

- The examples/ folders contains many functional implementation of the pseudo-code above.
- When calling NewFrame(), the 'io.WantCaptureMouse'/'io.WantCaptureKeyboard'/'io.WantTextInput' flags are updated.
They tell you if ImGui intends to use your inputs. So for example, if 'io.WantCaptureMouse' is set you would typically want to hide
mouse inputs from the rest of your application. Read the FAQ below for more information about those flags.



API BREAKING CHANGES
====================

Occasionally introducing changes that are breaking the API. The breakage are generally minor and easy to fix.
Here is a change-log of API breaking changes, if you are using one of the functions listed, expect to have to fix some code.
Also read releases logs https://github.com/ocornut/imgui/releases for more details.

- 2018/01/11 (1.54) - obsoleted IsAnyWindowHovered() in favor of IsWindowHovered(ImGuiHoveredFlags_AnyWindow). Kept redirection function (will obsolete).
- 2018/01/11 (1.54) - obsoleted IsAnyWindowFocused() in favor of IsWindowFocused(ImGuiFocusedFlags_AnyWindow). Kept redirection function (will obsolete).
- 2018/01/03 (1.54) - renamed ImGuiSizeConstraintCallback to ImGuiSizeCallback, ImGuiSizeConstraintCallbackData to ImGuiSizeCallbackData.
- 2017/12/29 (1.54) - removed CalcItemRectClosestPoint() which was weird and not really used by anyone except demo code. If you need it it's easy to replicate on your side.
- 2017/12/24 (1.53) - renamed the emblematic ShowTestWindow() function to ShowDemoWindow(). Kept redirection function (will obsolete).
- 2017/12/21 (1.53) - ImDrawList: renamed style.AntiAliasedShapes to style.AntiAliasedFill for consistency and as a way to explicitly break code that manipulate those flag at runtime. You can now manipulate ImDrawList::Flags
- 2017/12/21 (1.53) - ImDrawList: removed 'bool anti_aliased = true' final parameter of ImDrawList::AddPolyline() and ImDrawList::AddConvexPolyFilled(). Prefer manipulating ImDrawList::Flags if you need to toggle them during the frame.
- 2017/12/14 (1.53) - using the ImGuiWindowFlags_NoScrollWithMouse flag on a child window forwards the mouse wheel event to the parent window, unless either ImGuiWindowFlags_NoInputs or ImGuiWindowFlags_NoScrollbar are also set.
- 2017/12/13 (1.53) - renamed GetItemsLineHeightWithSpacing() to GetFrameHeightWithSpacing(). Kept redirection function (will obsolete).
- 2017/12/13 (1.53) - obsoleted IsRootWindowFocused() in favor of using IsWindowFocused(ImGuiFocusedFlags_RootWindow). Kept redirection function (will obsolete).
- obsoleted IsRootWindowOrAnyChildFocused() in favor of using IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows). Kept redirection function (will obsolete).
- 2017/12/12 (1.53) - renamed ImGuiTreeNodeFlags_AllowOverlapMode to ImGuiTreeNodeFlags_AllowItemOverlap. Kept redirection enum (will obsolete).
- 2017/12/10 (1.53) - removed SetNextWindowContentWidth(), prefer using SetNextWindowContentSize(). Kept redirection function (will obsolete).
- 2017/11/27 (1.53) - renamed ImGuiTextBuffer::append() helper to appendf(), appendv() to appendfv(). If you copied the 'Log' demo in your code, it uses appendv() so that needs to be renamed.
- 2017/11/18 (1.53) - Style, Begin: removed ImGuiWindowFlags_ShowBorders window flag. Borders are now fully set up in the ImGuiStyle structure (see e.g. style.FrameBorderSize, style.WindowBorderSize). Use ImGui::ShowStyleEditor() to look them up.
Please note that the style system will keep evolving (hopefully stabilizing in Q1 2018), and so custom styles will probably subtly break over time. It is recommended you use the StyleColorsClassic(), StyleColorsDark(), StyleColorsLight() functions.
- 2017/11/18 (1.53) - Style: removed ImGuiCol_ComboBg in favor of combo boxes using ImGuiCol_PopupBg for consistency.
- 2017/11/18 (1.53) - Style: renamed ImGuiCol_ChildWindowBg to ImGuiCol_ChildBg.
- 2017/11/18 (1.53) - Style: renamed style.ChildWindowRounding to style.ChildRounding, ImGuiStyleVar_ChildWindowRounding to ImGuiStyleVar_ChildRounding.
- 2017/11/02 (1.53) - obsoleted IsRootWindowOrAnyChildHovered() in favor of using IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
- 2017/10/24 (1.52) - renamed IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCS/IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCS to IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS/IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS for consistency.
- 2017/10/20 (1.52) - changed IsWindowHovered() default parameters behavior to return false if an item is active in another window (e.g. click-dragging item from another window to this window). You can use the newly introduced IsWindowHovered() flags to requests this specific behavior if you need it.
- 2017/10/20 (1.52) - marked IsItemHoveredRect()/IsMouseHoveringWindow() as obsolete, in favor of using the newly introduced flags for IsItemHovered() and IsWindowHovered(). See https://github.com/ocornut/imgui/issues/1382 for details.
removed the IsItemRectHovered()/IsWindowRectHovered() names introduced in 1.51 since they were merely more consistent names for the two functions we are now obsoleting.
- 2017/10/17 (1.52) - marked the old 5-parameters version of Begin() as obsolete (still available). Use SetNextWindowSize()+Begin() instead!
- 2017/10/11 (1.52) - renamed AlignFirstTextHeightToWidgets() to AlignTextToFramePadding(). Kept inline redirection function (will obsolete).
- 2017/09/25 (1.52) - removed SetNextWindowPosCenter() because SetNextWindowPos() now has the optional pivot information to do the same and more. Kept redirection function (will obsolete).
- 2017/08/25 (1.52) - io.MousePos needs to be set to ImVec2(-FLT_MAX,-FLT_MAX) when mouse is unavailable/missing. Previously ImVec2(-1,-1) was enough but we now accept negative mouse coordinates. In your binding if you need to support unavailable mouse, make sure to replace "io.MousePos = ImVec2(-1,-1)" with "io.MousePos = ImVec2(-FLT_MAX,-FLT_MAX)".
- 2017/08/22 (1.51) - renamed IsItemHoveredRect() to IsItemRectHovered(). Kept inline redirection function (will obsolete). -> (1.52) use IsItemHovered(ImGuiHoveredFlags_RectOnly)!
- renamed IsMouseHoveringAnyWindow() to IsAnyWindowHovered() for consistency. Kept inline redirection function (will obsolete).
- renamed IsMouseHoveringWindow() to IsWindowRectHovered() for consistency. Kept inline redirection function (will obsolete).
- 2017/08/20 (1.51) - renamed GetStyleColName() to GetStyleColorName() for consistency.
- 2017/08/20 (1.51) - added PushStyleColor(ImGuiCol idx, ImU32 col) overload, which _might_ cause an "ambiguous call" compilation error if you are using ImColor() with implicit cast. Cast to ImU32 or ImVec4 explicily to fix.
- 2017/08/15 (1.51) - marked the weird IMGUI_ONCE_UPON_A_FRAME helper macro as obsolete. prefer using the more explicit ImGuiOnceUponAFrame.
- 2017/08/15 (1.51) - changed parameter order for BeginPopupContextWindow() from (const char*,int buttons,bool also_over_items) to (const char*,int buttons,bool also_over_items). Note that most calls relied on default parameters completely.
- 2017/08/13 (1.51) - renamed ImGuiCol_Columns*** to ImGuiCol_Separator***. Kept redirection enums (will obsolete).
- 2017/08/11 (1.51) - renamed ImGuiSetCond_*** types and flags to ImGuiCond_***. Kept redirection enums (will obsolete).
- 2017/08/09 (1.51) - removed ValueColor() helpers, they are equivalent to calling Text(label) + SameLine() + ColorButton().
- 2017/08/08 (1.51) - removed ColorEditMode() and ImGuiColorEditMode in favor of ImGuiColorEditFlags and parameters to the various Color*() functions. The SetColorEditOptions() allows to initialize default but the user can still change them with right-click context menu.
- changed prototype of 'ColorEdit4(const char* label, float col[4], bool show_alpha = true)' to 'ColorEdit4(const char* label, float col[4], ImGuiColorEditFlags flags = 0)', where passing flags = 0x01 is a safe no-op (hello dodgy backward compatibility!). - check and run the demo window, under "Color/Picker Widgets", to understand the various new options.
- changed prototype of rarely used 'ColorButton(ImVec4 col, bool small_height = false, bool outline_border = true)' to 'ColorButton(const char* desc_id, ImVec4 col, ImGuiColorEditFlags flags = 0, ImVec2 size = ImVec2(0,0))'
- 2017/07/20 (1.51) - removed IsPosHoveringAnyWindow(ImVec2), which was partly broken and misleading. ASSERT + redirect user to io.WantCaptureMouse
- 2017/05/26 (1.50) - removed ImFontConfig::MergeGlyphCenterV in favor of a more multipurpose ImFontConfig::GlyphOffset.
- 2017/05/01 (1.50) - renamed ImDrawList::PathFill() (rarely used directly) to ImDrawList::PathFillConvex() for clarity.
- 2016/11/06 (1.50) - BeginChild(const char*) now applies the stack id to the provided label, consistently with other functions as it should always have been. It shouldn't affect you unless (extremely unlikely) you were appending multiple times to a same child from different locations of the stack id. If that's the case, generate an id with GetId() and use it instead of passing string to BeginChild().
- 2016/10/15 (1.50) - avoid 'void* user_data' parameter to io.SetClipboardTextFn/io.GetClipboardTextFn pointers. We pass io.ClipboardUserData to it.
- 2016/09/25 (1.50) - style.WindowTitleAlign is now a ImVec2 (ImGuiAlign enum was removed). set to (0.5f,0.5f) for horizontal+vertical centering, (0.0f,0.0f) for upper-left, etc.
- 2016/07/30 (1.50) - SameLine(x) with x>0.0f is now relative to left of column/group if any, and not always to left of window. This was sort of always the intent and hopefully breakage should be minimal.
- 2016/05/12 (1.49) - title bar (using ImGuiCol_TitleBg/ImGuiCol_TitleBgActive colors) isn't rendered over a window background (ImGuiCol_WindowBg color) anymore.
If your TitleBg/TitleBgActive alpha was 1.0f or you are using the default theme it will not affect you.
However if your TitleBg/TitleBgActive alpha was <1.0f you need to tweak your custom theme to readjust for the fact that we don't draw a WindowBg background behind the title bar.
This helper function will convert an old TitleBg/TitleBgActive color into a new one with the same visual output, given the OLD color and the OLD WindowBg color.
ImVec4 ConvertTitleBgCol(const ImVec4& win_bg_col, const ImVec4& title_bg_col)
{
float new_a = 1.0f - ((1.0f - win_bg_col.w) * (1.0f - title_bg_col.w)), k = title_bg_col.w / new_a;
return ImVec4((win_bg_col.x * win_bg_col.w + title_bg_col.x) * k, (win_bg_col.y * win_bg_col.w + title_bg_col.y) * k, (win_bg_col.z * win_bg_col.w + title_bg_col.z) * k, new_a);
}
If this is confusing, pick the RGB value from title bar from an old screenshot and apply this as TitleBg/TitleBgActive. Or you may just create TitleBgActive from a tweaked TitleBg color.
- 2016/05/07 (1.49) - removed confusing set of GetInternalState(), GetInternalStateSize(), SetInternalState() functions. Now using CreateContext(), DestroyContext(), GetCurrentContext(), SetCurrentContext().
- 2016/05/02 (1.49) - renamed SetNextTreeNodeOpened() to SetNextTreeNodeOpen(), no redirection.
- 2016/05/01 (1.49) - obsoleted old signature of CollapsingHeader(const char* label, const char* str_id = NULL, bool display_frame = true, bool default_open = false) as extra parameters were badly designed and rarely used. You can replace the "default_open = true" flag in new API with CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen).
- 2016/04/26 (1.49) - changed ImDrawList::PushClipRect(ImVec4 rect) to ImDraw::PushClipRect(Imvec2 min,ImVec2 max,bool intersect_with_current_clip_rect=false). Note that higher-level ImGui::PushClipRect() is preferable because it will clip at logic/widget level, whereas ImDrawList::PushClipRect() only affect your renderer.
- 2016/04/03 (1.48) - removed style.WindowFillAlphaDefault setting which was redundant. Bake default BG alpha inside style.Colors[ImGuiCol_WindowBg] and all other Bg color values. (ref github issue #337).
- 2016/04/03 (1.48) - renamed ImGuiCol_TooltipBg to ImGuiCol_PopupBg, used by popups/menus and tooltips. popups/menus were previously using ImGuiCol_WindowBg. (ref github issue #337)
- 2016/03/21 (1.48) - renamed GetWindowFont() to GetFont(), GetWindowFontSize() to GetFontSize(). Kept inline redirection function (will obsolete).
- 2016/03/02 (1.48) - InputText() completion/history/always callbacks: if you modify the text buffer manually (without using DeleteChars()/InsertChars() helper) you need to maintain the BufTextLen field. added an assert.
- 2016/01/23 (1.48) - fixed not honoring exact width passed to PushItemWidth(), previously it would add extra FramePadding.x*2 over that width. if you had manual pixel-perfect alignment in place it might affect you.
- 2015/12/27 (1.48) - fixed ImDrawList::AddRect() which used to render a rectangle 1 px too large on each axis.
- 2015/12/04 (1.47) - renamed Color() helpers to ValueColor() - dangerously named, rarely used and probably to be made obsolete.
- 2015/08/29 (1.45) - with the addition of horizontal scrollbar we made various fixes to inconsistencies with dealing with cursor position.
GetCursorPos()/SetCursorPos() functions now include the scrolled amount. It shouldn't affect the majority of users, but take note that SetCursorPosX(100.0f) puts you at +100 from the starting x position which may include scrolling, not at +100 from the window left side.
GetContentRegionMax()/GetWindowContentRegionMin()/GetWindowContentRegionMax() functions allow include the scrolled amount. Typically those were used in cases where no scrolling would happen so it may not be a problem, but watch out!
- 2015/08/29 (1.45) - renamed style.ScrollbarWidth to style.ScrollbarSize
- 2015/08/05 (1.44) - split imgui.cpp into extra files: imgui_demo.cpp imgui_draw.cpp imgui_internal.h that you need to add to your project.
- 2015/07/18 (1.44) - fixed angles in ImDrawList::PathArcTo(), PathArcToFast() (introduced in 1.43) being off by an extra PI for no justifiable reason
- 2015/07/14 (1.43) - add new ImFontAtlas::AddFont() API. For the old AddFont***, moved the 'font_no' parameter of ImFontAtlas::AddFont** functions to the ImFontConfig structure.
you need to render your textured triangles with bilinear filtering to benefit from sub-pixel positioning of text.
- 2015/07/08 (1.43) - switched rendering data to use indexed rendering. this is saving a fair amount of CPU/GPU and enables us to get anti-aliasing for a marginal cost.
this necessary change will break your rendering function! the fix should be very easy. sorry for that :(
- if you are using a vanilla copy of one of the imgui_impl_XXXX.cpp provided in the example, you just need to update your copy and you can ignore the rest.
- the signature of the io.RenderDrawListsFn handler has changed!
ImGui_XXXX_RenderDrawLists(ImDrawList** const cmd_lists, int cmd_lists_count)
became:
ImGui_XXXX_RenderDrawLists(ImDrawData* draw_data).
argument   'cmd_lists'        -> 'draw_data->CmdLists'
argument   'cmd_lists_count'  -> 'draw_data->CmdListsCount'
ImDrawList 'commands'         -> 'CmdBuffer'
ImDrawList 'vtx_buffer'       -> 'VtxBuffer'
ImDrawList  n/a               -> 'IdxBuffer' (new)
ImDrawCmd  'vtx_count'        -> 'ElemCount'
ImDrawCmd  'clip_rect'        -> 'ClipRect'
ImDrawCmd  'user_callback'    -> 'UserCallback'
ImDrawCmd  'texture_id'       -> 'TextureId'
- each ImDrawList now contains both a vertex buffer and an index buffer. For each command, render ElemCount/3 triangles using indices from the index buffer.
- if you REALLY cannot render indexed primitives, you can call the draw_data->DeIndexAllBuffers() method to de-index the buffers. This is slow and a waste of CPU/GPU. Prefer using indexed rendering!
- refer to code in the examples/ folder or ask on the GitHub if you are unsure of how to upgrade. please upgrade!
- 2015/07/10 (1.43) - changed SameLine() parameters from int to float.
- 2015/07/02 (1.42) - renamed SetScrollPosHere() to SetScrollFromCursorPos(). Kept inline redirection function (will obsolete).
- 2015/07/02 (1.42) - renamed GetScrollPosY() to GetScrollY(). Necessary to reduce confusion along with other scrolling functions, because positions (e.g. cursor position) are not equivalent to scrolling amount.
- 2015/06/14 (1.41) - changed ImageButton() default bg_col parameter from (0,0,0,1) (black) to (0,0,0,0) (transparent) - makes a difference when texture have transparence
- 2015/06/14 (1.41) - changed Selectable() API from (label, selected, size) to (label, selected, flags, size). Size override should have been rarely be used. Sorry!
- 2015/05/31 (1.40) - renamed GetWindowCollapsed() to IsWindowCollapsed() for consistency. Kept inline redirection function (will obsolete).
- 2015/05/31 (1.40) - renamed IsRectClipped() to IsRectVisible() for consistency. Note that return value is opposite! Kept inline redirection function (will obsolete).
- 2015/05/27 (1.40) - removed the third 'repeat_if_held' parameter from Button() - sorry! it was rarely used and inconsistent. Use PushButtonRepeat(true) / PopButtonRepeat() to enable repeat on desired buttons.
- 2015/05/11 (1.40) - changed BeginPopup() API, takes a string identifier instead of a bool. ImGui needs to manage the open/closed state of popups. Call OpenPopup() to actually set the "open" state of a popup. BeginPopup() returns true if the popup is opened.
- 2015/05/03 (1.40) - removed style.AutoFitPadding, using style.WindowPadding makes more sense (the default values were already the same).
- 2015/04/13 (1.38) - renamed IsClipped() to IsRectClipped(). Kept inline redirection function until 1.50.
- 2015/04/09 (1.38) - renamed ImDrawList::AddArc() to ImDrawList::AddArcFast() for compatibility with future API
- 2015/04/03 (1.38) - removed ImGuiCol_CheckHovered, ImGuiCol_CheckActive, replaced with the more general ImGuiCol_FrameBgHovered, ImGuiCol_FrameBgActive.
- 2014/04/03 (1.38) - removed support for passing -FLT_MAX..+FLT_MAX as the range for a SliderFloat(). Use DragFloat() or Inputfloat() instead.
- 2015/03/17 (1.36) - renamed GetItemBoxMin()/GetItemBoxMax()/IsMouseHoveringBox() to GetItemRectMin()/GetItemRectMax()/IsMouseHoveringRect(). Kept inline redirection function until 1.50.
- 2015/03/15 (1.36) - renamed style.TreeNodeSpacing to style.IndentSpacing, ImGuiStyleVar_TreeNodeSpacing to ImGuiStyleVar_IndentSpacing
- 2015/03/13 (1.36) - renamed GetWindowIsFocused() to IsWindowFocused(). Kept inline redirection function until 1.50.
- 2015/03/08 (1.35) - renamed style.ScrollBarWidth to style.ScrollbarWidth (casing)
- 2015/02/27 (1.34) - renamed OpenNextNode(bool) to SetNextTreeNodeOpened(bool, ImGuiSetCond). Kept inline redirection function until 1.50.
- 2015/02/27 (1.34) - renamed ImGuiSetCondition_*** to ImGuiSetCond_***, and _FirstUseThisSession becomes _Once.
- 2015/02/11 (1.32) - changed text input callback ImGuiTextEditCallback return type from void-->int. reserved for future use, return 0 for now.
- 2015/02/10 (1.32) - renamed GetItemWidth() to CalcItemWidth() to clarify its evolving behavior
- 2015/02/08 (1.31) - renamed GetTextLineSpacing() to GetTextLineHeightWithSpacing()
- 2015/02/01 (1.31) - removed IO.MemReallocFn (unused)
- 2015/01/19 (1.30) - renamed ImGuiStorage::GetIntPtr()/GetFloatPtr() to GetIntRef()/GetIntRef() because Ptr was conflicting with actual pointer storage functions.
- 2015/01/11 (1.30) - big font/image API change! now loads TTF file. allow for multiple fonts. no need for a PNG loader.
(1.30) - removed GetDefaultFontData(). uses io.Fonts->GetTextureData*() API to retrieve uncompressed pixels.
this sequence:
const void* png_data;
unsigned int png_size;
ImGui::GetDefaultFontData(NULL, NULL, &png_data, &png_size);
// <Copy to GPU>
became:
unsigned char* pixels;
int width, height;
io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
// <Copy to GPU>
io.Fonts->TexID = (your_texture_identifier);
you now have much more flexibility to load multiple TTF fonts and manage the texture buffer for internal needs.
it is now recommended that you sample the font texture with bilinear interpolation.
(1.30) - added texture identifier in ImDrawCmd passed to your render function (we can now render images). make sure to set io.Fonts->TexID.
(1.30) - removed IO.PixelCenterOffset (unnecessary, can be handled in user projection matrix)
(1.30) - removed ImGui::IsItemFocused() in favor of ImGui::IsItemActive() which handles all widgets
- 2014/12/10 (1.18) - removed SetNewWindowDefaultPos() in favor of new generic API SetNextWindowPos(pos, ImGuiSetCondition_FirstUseEver)
- 2014/11/28 (1.17) - moved IO.Font*** options to inside the IO.Font-> structure (FontYOffset, FontTexUvForWhite, FontBaseScale, FontFallbackGlyph)
- 2014/11/26 (1.17) - reworked syntax of IMGUI_ONCE_UPON_A_FRAME helper macro to increase compiler compatibility
- 2014/11/07 (1.15) - renamed IsHovered() to IsItemHovered()
- 2014/10/02 (1.14) - renamed IMGUI_INCLUDE_IMGUI_USER_CPP to IMGUI_INCLUDE_IMGUI_USER_INL and imgui_user.cpp to imgui_user.inl (more IDE friendly)
- 2014/09/25 (1.13) - removed 'text_end' parameter from IO.SetClipboardTextFn (the string is now always zero-terminated for simplicity)
- 2014/09/24 (1.12) - renamed SetFontScale() to SetWindowFontScale()
- 2014/09/24 (1.12) - moved IM_MALLOC/IM_REALLOC/IM_FREE preprocessor defines to IO.MemAllocFn/IO.MemReallocFn/IO.MemFreeFn
- 2014/08/30 (1.09) - removed IO.FontHeight (now computed automatically)
- 2014/08/30 (1.09) - moved IMGUI_FONT_TEX_UV_FOR_WHITE preprocessor define to IO.FontTexUvForWhite
- 2014/08/28 (1.09) - changed the behavior of IO.PixelCenterOffset following various rendering fixes


ISSUES & TODO-LIST
==================
See TODO.txt


FREQUENTLY ASKED QUESTIONS (FAQ), TIPS
======================================

Q: How can I help?
A: - If you are experienced with Dear ImGui and C++, look at the github issues, or TODO.txt and see how you want/can help!
- Convince your company to fund development time! Individual users: you can also become a Patron (patreon.com/imgui) or donate on PayPal! See README.
- Disclose your usage of dear imgui via a dev blog post, a tweet, a screenshot, a mention somewhere etc.
You may post screenshot or links in the gallery threads (github.com/ocornut/imgui/issues/1269). Visuals are ideal as they inspire other programmers.
But even without visuals, disclosing your use of dear imgui help the library grow credibility, and help other teams and programmers with taking decisions.
- If you have issues or if you need to hack into the library, even if you don't expect any support it is useful that you share your issues (on github or privately).

Q: How can I display an image? What is ImTextureID, how does it works?
A: ImTextureID is a void* used to pass renderer-agnostic texture references around until it hits your render function.
Dear ImGui knows nothing about what those bits represent, it just passes them around. It is up to you to decide what you want the void* to carry!
It could be an identifier to your OpenGL texture (cast GLuint to void*), a pointer to your custom engine material (cast MyMaterial* to void*), etc.
At the end of the chain, your renderer takes this void* to cast it back into whatever it needs to select a current texture to render.
Refer to examples applications, where each renderer (in a imgui_impl_xxxx.cpp file) is treating ImTextureID as a different thing.
(c++ tip: OpenGL uses integers to identify textures. You can safely store an integer into a void*, just cast it to void*, don't take it's address!)
To display a custom image/texture within an ImGui window, you may use ImGui::Image(), ImGui::ImageButton(), ImDrawList::AddImage() functions.
Dear ImGui will generate the geometry and draw calls using the ImTextureID that you passed and which your renderer can use.
You may call ImGui::ShowMetricsWindow() to explore active draw lists and visualize/understand how the draw data is generated.
It is your responsibility to get textures uploaded to your GPU.

Q: Can I have multiple widgets with the same label? Can I have widget without a label?
A: Yes. A primer on labels and the ID stack...

- Elements that are typically not clickable, such as Text() items don't need an ID.

- Interactive widgets require state to be carried over multiple frames (most typically Dear ImGui often needs to remember what is
the "active" widget). to do so they need a unique ID. unique ID are typically derived from a string label, an integer index or a pointer.

Button("OK");          // Label = "OK",     ID = hash of "OK"
Button("Cancel");      // Label = "Cancel", ID = hash of "Cancel"

- ID are uniquely scoped within windows, tree nodes, etc. so no conflict can happen if you have two buttons called "OK"
in two different windows or in two different locations of a tree.

- If you have a same ID twice in the same location, you'll have a conflict:

Button("OK");
Button("OK");          // ID collision! Both buttons will be treated as the same.

Fear not! this is easy to solve and there are many ways to solve it!

- When passing a label you can optionally specify extra unique ID information within string itself.
Use "##" to pass a complement to the ID that won't be visible to the end-user.
This helps solving the simple collision cases when you know which items are going to be created.

Button("Play");        // Label = "Play",   ID = hash of "Play"
Button("Play##foo1");  // Label = "Play",   ID = hash of "Play##foo1" (different from above)
Button("Play##foo2");  // Label = "Play",   ID = hash of "Play##foo2" (different from above)

- If you want to completely hide the label, but still need an ID:

Checkbox("##On", &b);  // Label = "",       ID = hash of "##On" (no label!)

- Occasionally/rarely you might want change a label while preserving a constant ID. This allows you to animate labels.
For example you may want to include varying information in a window title bar, but windows are uniquely identified by their ID..
Use "###" to pass a label that isn't part of ID:

Button("Hello###ID";   // Label = "Hello",  ID = hash of "ID"
Button("World###ID";   // Label = "World",  ID = hash of "ID" (same as above)

sprintf(buf, "My game (%f FPS)###MyGame", fps);
Begin(buf);            // Variable label,   ID = hash of "MyGame"

- Use PushID() / PopID() to create scopes and avoid ID conflicts within the same Window.
This is the most convenient way of distinguishing ID if you are iterating and creating many UI elements.
You can push a pointer, a string or an integer value. Remember that ID are formed from the concatenation of _everything_ in the ID stack!

for (int i = 0; i < 100; i++)
{
PushID(i);
Button("Click");   // Label = "Click",  ID = hash of integer + "label" (unique)
PopID();
}

for (int i = 0; i < 100; i++)
{
MyObject* obj = Objects[i];
PushID(obj);
Button("Click");   // Label = "Click",  ID = hash of pointer + "label" (unique)
PopID();
}

for (int i = 0; i < 100; i++)
{
MyObject* obj = Objects[i];
PushID(obj->Name);
Button("Click");   // Label = "Click",  ID = hash of string + "label" (unique)
PopID();
}

- More example showing that you can stack multiple prefixes into the ID stack:

Button("Click");     // Label = "Click",  ID = hash of "Click"
PushID("node");
Button("Click");     // Label = "Click",  ID = hash of "node" + "Click"
PushID(my_ptr);
Button("Click"); // Label = "Click",  ID = hash of "node" + ptr + "Click"
PopID();
PopID();

- Tree nodes implicitly creates a scope for you by calling PushID().

Button("Click");     // Label = "Click",  ID = hash of "Click"
if (TreeNode("node"))
{
Button("Click");   // Label = "Click",  ID = hash of "node" + "Click"
TreePop();
}

- When working with trees, ID are used to preserve the open/close state of each tree node.
Depending on your use cases you may want to use strings, indices or pointers as ID.
e.g. when displaying a single object that may change over time (dynamic 1-1 relationship), using a static string as ID will preserve your
node open/closed state when the targeted object change.
e.g. when displaying a list of objects, using indices or pointers as ID will preserve the node open/closed state differently.
experiment and see what makes more sense!

Q: How can I tell when Dear ImGui wants my mouse/keyboard inputs VS when I can pass them to my application?
A: You can read the 'io.WantCaptureMouse'/'io.WantCaptureKeyboard'/'ioWantTextInput' flags from the ImGuiIO structure.
- When 'io.WantCaptureMouse' or 'io.WantCaptureKeyboard' flags are set you may want to discard/hide the inputs from the rest of your application.
- When 'io.WantTextInput' is set to may want to notify your OS to popup an on-screen keyboard, if available (e.g. on a mobile phone, or console OS).
Preferably read the flags after calling ImGui::NewFrame() to avoid them lagging by one frame. But reading those flags before calling NewFrame() is
also generally ok, as the bool toggles fairly rarely and you don't generally expect to interact with either Dear ImGui or your application during
the same frame when that transition occurs. Dear ImGui is tracking dragging and widget activity that may occur outside the boundary of a window,
so 'io.WantCaptureMouse' is more accurate and correct than checking if a window is hovered.
(Advanced note: text input releases focus on Return 'KeyDown', so the following Return 'KeyUp' event that your application receive will typically
have 'io.WantCaptureKeyboard=false'. Depending on your application logic it may or not be inconvenient. You might want to track which key-downs
were for Dear ImGui, e.g. with an array of bool, and filter out the corresponding key-ups.)

Q: How can I load a different font than the default? (default is an embedded version of ProggyClean.ttf, rendered at size 13)
A: Use the font atlas to load the TTF/OTF file you want:

ImGuiIO& io = ImGui::GetIO();
io.Fonts->AddFontFromFileTTF("myfontfile.ttf", size_in_pixels);
io.Fonts->GetTexDataAsRGBA32() or GetTexDataAsAlpha8()

Q: How can I easily use icons in my application?
A: The most convenient and practical way is to merge an icon font such as FontAwesome inside you main font. Then you can refer to icons within your
strings. Read 'How can I load multiple fonts?' and the file 'misc/fonts/README.txt' for instructions and useful header files.

Q: How can I load multiple fonts?
A: Use the font atlas to pack them into a single texture:
(Read misc/fonts/README.txt and the code in ImFontAtlas for more details.)

ImGuiIO& io = ImGui::GetIO();
ImFont* font0 = io.Fonts->AddFontDefault();
ImFont* font1 = io.Fonts->AddFontFromFileTTF("myfontfile.ttf", size_in_pixels);
ImFont* font2 = io.Fonts->AddFontFromFileTTF("myfontfile2.ttf", size_in_pixels);
io.Fonts->GetTexDataAsRGBA32() or GetTexDataAsAlpha8()
// the first loaded font gets used by default
// use ImGui::PushFont()/ImGui::PopFont() to change the font at runtime

// Options
ImFontConfig config;
config.OversampleH = 3;
config.OversampleV = 1;
config.GlyphOffset.y -= 2.0f;      // Move everything by 2 pixels up
config.GlyphExtraSpacing.x = 1.0f; // Increase spacing between characters
io.Fonts->LoadFromFileTTF("myfontfile.ttf", size_pixels, &config);

// Combine multiple fonts into one (e.g. for icon fonts)
ImWchar ranges[] = { 0xf000, 0xf3ff, 0 };
ImFontConfig config;
config.MergeMode = true;
io.Fonts->AddFontDefault();
io.Fonts->LoadFromFileTTF("fontawesome-webfont.ttf", 16.0f, &config, ranges); // Merge icon font
io.Fonts->LoadFromFileTTF("myfontfile.ttf", size_pixels, NULL, &config, io.Fonts->GetGlyphRangesJapanese()); // Merge japanese glyphs

Q: How can I display and input non-Latin characters such as Chinese, Japanese, Korean, Cyrillic?
A: When loading a font, pass custom Unicode ranges to specify the glyphs to load.

// Add default Japanese ranges
io.Fonts->AddFontFromFileTTF("myfontfile.ttf", size_in_pixels, NULL, io.Fonts->GetGlyphRangesJapanese());

// Or create your own custom ranges (e.g. for a game you can feed your entire game script and only build the characters the game need)
ImVector<ImWchar> ranges;
ImFontAtlas::GlyphRangesBuilder builder;
builder.AddText("Hello world");                        // Add a string (here "Hello world" contains 7 unique characters)
builder.AddChar(0x7262);                               // Add a specific character
builder.AddRanges(io.Fonts->GetGlyphRangesJapanese()); // Add one of the default ranges
builder.BuildRanges(&ranges);                          // Build the final result (ordered ranges with all the unique characters submitted)
io.Fonts->AddFontFromFileTTF("myfontfile.ttf", size_in_pixels, NULL, ranges.Data);

All your strings needs to use UTF-8 encoding. In C++11 you can encode a string literal in UTF-8 by using the u8"hello" syntax.
Specifying literal in your source code using a local code page (such as CP-923 for Japanese or CP-1251 for Cyrillic) will NOT work!
Otherwise you can convert yourself to UTF-8 or load text data from file already saved as UTF-8.

Text input: it is up to your application to pass the right character code to io.AddInputCharacter(). The applications in examples/ are doing that.
For languages using IME, on Windows you can copy the Hwnd of your application to io.ImeWindowHandle.
The default implementation of io.ImeSetInputScreenPosFn() on Windows will set your IME position correctly.

Q: How can I preserve my Dear ImGui context across reloading a DLL? (loss of the global/static variables)
A: Create your own context 'ctx = CreateContext()' + 'SetCurrentContext(ctx)' and your own font atlas 'ctx->GetIO().Fonts = new ImFontAtlas()'
so you don't rely on the default globals.

Q: How can I use the drawing facilities without an ImGui window? (using ImDrawList API)
A: - You can create a dummy window. Call Begin() with NoTitleBar|NoResize|NoMove|NoScrollbar|NoSavedSettings|NoInputs flag,
push a ImGuiCol_WindowBg with zero alpha, then retrieve the ImDrawList* via GetWindowDrawList() and draw to it in any way you like.
- You can call ImGui::GetOverlayDrawList() and use this draw list to display contents over every other imgui windows.
- You can create your own ImDrawList instance. You'll need to initialize them ImGui::GetDrawListSharedData(), or create your own ImDrawListSharedData.

Q: I integrated Dear ImGui in my engine and the text or lines are blurry..
A: In your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f).
Also make sure your orthographic projection matrix and io.DisplaySize matches your actual framebuffer dimension.

Q: I integrated Dear ImGui in my engine and some elements are clipping or disappearing when I move windows around..
A: You are probably mishandling the clipping rectangles in your render function.
Rectangles provided by ImGui are defined as (x1=left,y1=top,x2=right,y2=bottom) and NOT as (x1,y1,width,height).


- tip: you can call Begin() multiple times with the same name during the same frame, it will keep appending to the same window.
this is also useful to set yourself in the context of another window (to get/set other settings)
- tip: you can create widgets without a Begin()/End() block, they will go in an implicit window called "Debug".
- tip: the ImGuiOnceUponAFrame helper will allow run the block of code only once a frame. You can use it to quickly add custom UI in the middle
of a deep nested inner loop in your code.
- tip: you can call Render() multiple times (e.g for VR renders).
- tip: call and read the ShowDemoWindow() code in imgui_demo.cpp for more example of how to use ImGui!

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "..\Global.h"
#include <ctype.h>      // toupper, isprint
#include <stdlib.h>     // NULL, malloc, free, qsort, atoi
#include <stdio.h>      // vsnprintf, sscanf, printf
#include <limits.h>     // INT_MIN, INT_MAX
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>     // intptr_t
#else
#include <stdint.h>     // intptr_t
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4127) // condition expression is constant
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

// Clang warnings with -Weverything
#ifdef __clang__
#pragma clang diagnostic ignored "-Wunknown-pragmas"        // warning : unknown warning group '-Wformat-pedantic *'        // not all warnings are known by all clang versions.. so ignoring warnings triggers new warnings on some configuration. great!
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"            // warning : comparing floating point with == or != is unsafe   // storing and comparing against same constants (typically 0.0f) is ok.
#pragma clang diagnostic ignored "-Wformat-nonliteral"      // warning : format string is not a string literal              // passing non-literal to vsnformat(). yes, user passing incorrect format strings can crash the code.
#pragma clang diagnostic ignored "-Wexit-time-destructors"  // warning : declaration requires an exit-time destructor       // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. ImGui coding style welcomes static/globals.
#pragma clang diagnostic ignored "-Wglobal-constructors"    // warning : declaration requires a global destructor           // similar to above, not sure what the exact difference it.
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning : implicit conversion changes signedness             //
#pragma clang diagnostic ignored "-Wformat-pedantic"        // warning : format specifies type 'void *' but the argument has type 'xxxx *' // unreasonable, would lead to casting every %p arg to void*. probably enabled by -pedantic. 
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast" // warning : cast to 'void *' from smaller integer type 'int' //
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"          // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"      // warning: cast to pointer from integer of different size
#pragma GCC diagnostic ignored "-Wformat"                   // warning: format '%p' expects argument of type 'void*', but argument 6 has type 'ImGuiWindow*'
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wcast-qual"                // warning: cast from type 'xxxx' to type 'xxxx' casts away qualifiers
#pragma GCC diagnostic ignored "-Wformat-nonliteral"        // warning: format not a string literal, format string not checked
#endif

//-------------------------------------------------------------------------
// Forward Declarations
//-------------------------------------------------------------------------

static bool             IsKeyPressedMap(ImGuiKey key, bool repeat = true);

static ImFont*          GetDefaultFont();
static void             SetCurrentWindow(ImGuiWindow* window);
static void             SetWindowScrollX(ImGuiWindow* window, float new_scroll_x);
static void             SetWindowScrollY(ImGuiWindow* window, float new_scroll_y);
static void             SetWindowPos(ImGuiWindow* window, const ImVec2& pos, ImGuiCond cond);
static void             SetWindowSize(ImGuiWindow* window, const ImVec2& size, ImGuiCond cond);
static void             SetWindowCollapsed(ImGuiWindow* window, bool collapsed, ImGuiCond cond);
static ImGuiWindow*     FindHoveredWindow(ImVec2 pos);
static ImGuiWindow*     CreateNewWindow(const char* name, ImVec2 size, ImGuiWindowFlags flags);
static void             CheckStacksSize(ImGuiWindow* window, bool write);
static ImVec2           CalcNextScrollFromScrollTargetAndClamp(ImGuiWindow* window);

static void             AddDrawListToDrawData(ImVector<ImDrawList*>* out_render_list, ImDrawList* draw_list);
static void             AddWindowToDrawData(ImVector<ImDrawList*>* out_render_list, ImGuiWindow* window);
static void             AddWindowToSortedBuffer(ImVector<ImGuiWindow*>* out_sorted_windows, ImGuiWindow* window);

static ImGuiWindowSettings* AddWindowSettings(const char* name);

static void             LoadIniSettingsFromDisk(const char* ini_filename);
static void             LoadIniSettingsFromMemory(const char* buf);
static void             SaveIniSettingsToDisk(const char* ini_filename);
static void             SaveIniSettingsToMemory(ImVector<char>& out_buf);
static void             MarkIniSettingsDirty(ImGuiWindow* window);

static ImRect           GetViewportRect();

static void             ClosePopupToLevel(int remaining);
static ImGuiWindow*     GetFrontMostModalRootWindow();

static bool             InputTextFilterCharacter(unsigned int* p_char, ImGuiInputTextFlags flags, ImGuiTextEditCallback callback, void* user_data);
static int              InputTextCalcTextLenAndLineCount(const char* text_begin, const char** out_text_end);
static ImVec2           InputTextCalcTextSizeW(const ImWchar* text_begin, const ImWchar* text_end, const ImWchar** remaining = NULL, ImVec2* out_offset = NULL, bool stop_on_new_line = false);

static inline void      DataTypeFormatString(ImGuiDataType data_type, void* data_ptr, const char* display_format, char* buf, int buf_size);
static inline void      DataTypeFormatString(ImGuiDataType data_type, void* data_ptr, int decimal_precision, char* buf, int buf_size);
static void             DataTypeApplyOp(ImGuiDataType data_type, int op, void* value1, const void* value2);
static bool             DataTypeApplyOpFromText(const char* buf, const char* initial_value_buf, ImGuiDataType data_type, void* data_ptr, const char* scalar_format);

namespace ImGui
{
	static void             UpdateManualResize(ImGuiWindow* window, const ImVec2& size_auto_fit, int* border_held, int resize_grip_count, ImU32 resize_grip_col[4]);
	static void             FocusFrontMostActiveWindow(ImGuiWindow* ignore_window);
}

//-----------------------------------------------------------------------------
// Platform dependent default implementations
//-----------------------------------------------------------------------------

static const char*      GetClipboardTextFn_DefaultImpl(void* user_data);
static void             SetClipboardTextFn_DefaultImpl(void* user_data, const char* text);
static void             ImeSetInputScreenPosFn_DefaultImpl(int x, int y);

//-----------------------------------------------------------------------------
// Context
//-----------------------------------------------------------------------------

// Default font atlas storage.
// New contexts always point by default to this font atlas. It can be changed by reassigning the GetIO().Fonts variable.
static ImFontAtlas      GImDefaultFontAtlas;

// Default context storage + current context pointer.
// Implicitely used by all ImGui functions. Always assumed to be != NULL. Change to a different context by calling ImGui::SetCurrentContext()
// If you are hot-reloading this code in a DLL you will lose the static/global variables. Create your own context+font atlas instead of relying on those default (see FAQ entry "How can I preserve my ImGui context across reloading a DLL?").
// ImGui is currently not thread-safe because of this variable. If you want thread-safety to allow N threads to access N different contexts, you might work around it by:
// - Having multiple instances of the ImGui code compiled inside different namespace (easiest/safest, if you have a finite number of contexts)
// - or: Changing this variable to be TLS. You may #define GImGui in imconfig.h for further custom hackery. Future development aim to make this context pointer explicit to all calls. Also read https://github.com/ocornut/imgui/issues/586
#ifndef GImGui
static ImGuiContext     GImDefaultContext;
ImGuiContext*           GImGui = &GImDefaultContext;
#endif

//-----------------------------------------------------------------------------
// User facing structures
//-----------------------------------------------------------------------------

ImGuiStyle::ImGuiStyle()
{
	Alpha = 1.0f;             // Global alpha applies to everything in ImGui
	WindowPadding = ImVec2(8, 8);      // Padding within a window
	WindowRounding = 7.0f;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows
	WindowBorderSize = 1.0f;             // Thickness of border around windows. Generally set to 0.0f or 1.0f. Other values not well tested.
	WindowMinSize = ImVec2(32, 32);    // Minimum window size
	WindowTitleAlign = ImVec2(0.0f, 0.5f);// Alignment for title bar text
	ChildRounding = 0.0f;             // Radius of child window corners rounding. Set to 0.0f to have rectangular child windows
	ChildBorderSize = 1.0f;             // Thickness of border around child windows. Generally set to 0.0f or 1.0f. Other values not well tested.
	PopupRounding = 0.0f;             // Radius of popup window corners rounding. Set to 0.0f to have rectangular child windows
	PopupBorderSize = 1.0f;             // Thickness of border around popup or tooltip windows. Generally set to 0.0f or 1.0f. Other values not well tested.
	FramePadding = ImVec2(4, 3);      // Padding within a framed rectangle (used by most widgets)
	FrameRounding = 0.0f;             // Radius of frame corners rounding. Set to 0.0f to have rectangular frames (used by most widgets).
	FrameBorderSize = 0.0f;             // Thickness of border around frames. Generally set to 0.0f or 1.0f. Other values not well tested.
	ItemSpacing = ImVec2(8, 4);      // Horizontal and vertical spacing between widgets/lines
	ItemInnerSpacing = ImVec2(4, 4);      // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label)
	TouchExtraPadding = ImVec2(0, 0);      // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
	IndentSpacing = 21.0f;            // Horizontal spacing when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
	ColumnsMinSpacing = 6.0f;             // Minimum horizontal spacing between two columns
	ScrollbarSize = 16.0f;            // Width of the vertical scrollbar, Height of the horizontal scrollbar
	ScrollbarRounding = 9.0f;             // Radius of grab corners rounding for scrollbar
	GrabMinSize = 10.0f;            // Minimum width/height of a grab box for slider/scrollbar
	GrabRounding = 0.0f;             // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
	ButtonTextAlign = ImVec2(0.5f, 0.5f);// Alignment of button text when button is larger than text.
	DisplayWindowPadding = ImVec2(22, 22);    // Window positions are clamped to be visible within the display area by at least this amount. Only covers regular windows.
	DisplaySafeAreaPadding = ImVec2(4, 4);      // If you cannot see the edge of your screen (e.g. on a TV) increase the safe area padding. Covers popups/tooltips as well regular windows.
	AntiAliasedLines = true;             // Enable anti-aliasing on lines/borders. Disable if you are really short on CPU/GPU.
	AntiAliasedFill = true;             // Enable anti-aliasing on filled shapes (rounded rectangles, circles, etc.)
	CurveTessellationTol = 1.25f;            // Tessellation tolerance when using PathBezierCurveTo() without a specific number of segments. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.

	ImGui::StyleColorsClassic(this);
}

// To scale your entire UI (e.g. if you want your app to use High DPI or generally be DPI aware) you may use this helper function. Scaling the fonts is done separately and is up to you.
// Important: This operation is lossy because we round all sizes to integer. If you need to change your scale multiples, call this over a freshly initialized ImGuiStyle structure rather than scaling multiple times.
void ImGuiStyle::ScaleAllSizes(float scale_factor)
{
	WindowPadding = ImFloor(WindowPadding * scale_factor);
	WindowRounding = ImFloor(WindowRounding * scale_factor);
	WindowMinSize = ImFloor(WindowMinSize * scale_factor);
	ChildRounding = ImFloor(ChildRounding * scale_factor);
	PopupRounding = ImFloor(PopupRounding * scale_factor);
	FramePadding = ImFloor(FramePadding * scale_factor);
	FrameRounding = ImFloor(FrameRounding * scale_factor);
	ItemSpacing = ImFloor(ItemSpacing * scale_factor);
	ItemInnerSpacing = ImFloor(ItemInnerSpacing * scale_factor);
	TouchExtraPadding = ImFloor(TouchExtraPadding * scale_factor);
	IndentSpacing = ImFloor(IndentSpacing * scale_factor);
	ColumnsMinSpacing = ImFloor(ColumnsMinSpacing * scale_factor);
	ScrollbarSize = ImFloor(ScrollbarSize * scale_factor);
	ScrollbarRounding = ImFloor(ScrollbarRounding * scale_factor);
	GrabMinSize = ImFloor(GrabMinSize * scale_factor);
	GrabRounding = ImFloor(GrabRounding * scale_factor);
	DisplayWindowPadding = ImFloor(DisplayWindowPadding * scale_factor);
	DisplaySafeAreaPadding = ImFloor(DisplaySafeAreaPadding * scale_factor);
}

ImGuiIO::ImGuiIO()
{
	// Most fields are initialized with zero
	memset(this, 0, sizeof(*this));

	// Settings
	DisplaySize = ImVec2(-1.0f, -1.0f);
	DeltaTime = 1.0f / 60.0f;
	IniSavingRate = 5.0f;
	IniFilename = "imgui.ini";
	LogFilename = "imgui_log.txt";
	MouseDoubleClickTime = 0.30f;
	MouseDoubleClickMaxDist = 6.0f;
	for (int i = 0; i < ImGuiKey_COUNT; i++)
		KeyMap[i] = -1;
	KeyRepeatDelay = 0.250f;
	KeyRepeatRate = 0.050f;
	UserData = NULL;

	Fonts = &GImDefaultFontAtlas;
	FontGlobalScale = 1.0f;
	FontDefault = NULL;
	FontAllowUserScaling = false;
	DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	DisplayVisibleMin = DisplayVisibleMax = ImVec2(0.0f, 0.0f);

	// Advanced/subtle behaviors
#ifdef __APPLE__
	OptMacOSXBehaviors = true;  // Set Mac OS X style defaults based on __APPLE__ compile time flag
#else
	OptMacOSXBehaviors = false;
#endif
	OptCursorBlink = true;

	// Settings (User Functions)
	RenderDrawListsFn = NULL;
	MemAllocFn = malloc;
	MemFreeFn = free;
	GetClipboardTextFn = GetClipboardTextFn_DefaultImpl;   // Platform dependent default implementations
	SetClipboardTextFn = SetClipboardTextFn_DefaultImpl;
	ClipboardUserData = NULL;
	ImeSetInputScreenPosFn = ImeSetInputScreenPosFn_DefaultImpl;
	ImeWindowHandle = NULL;

	// Input (NB: we already have memset zero the entire structure)
	MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
	MousePosPrev = ImVec2(-FLT_MAX, -FLT_MAX);
	MouseDragThreshold = 6.0f;
	for (int i = 0; i < IM_ARRAYSIZE(MouseDownDuration); i++) MouseDownDuration[i] = MouseDownDurationPrev[i] = -1.0f;
	for (int i = 0; i < IM_ARRAYSIZE(KeysDownDuration); i++) KeysDownDuration[i] = KeysDownDurationPrev[i] = -1.0f;
}

// Pass in translated ASCII characters for text input.
// - with glfw you can get those from the callback set in glfwSetCharCallback()
// - on Windows you can get those using ToAscii+keyboard state, or via the WM_CHAR message
void ImGuiIO::AddInputCharacter(ImWchar c)
{
	const int n = ImStrlenW(InputCharacters);
	if (n + 1 < IM_ARRAYSIZE(InputCharacters))
	{
		InputCharacters[n] = c;
		InputCharacters[n + 1] = '\0';
	}
}

void ImGuiIO::AddInputCharactersUTF8(const char* utf8_chars)
{
	// We can't pass more wchars than ImGuiIO::InputCharacters[] can hold so don't convert more
	const int wchars_buf_len = sizeof(ImGuiIO::InputCharacters) / sizeof(ImWchar);
	ImWchar wchars[wchars_buf_len];
	ImTextStrFromUtf8(wchars, wchars_buf_len, utf8_chars, NULL);
	for (int i = 0; i < wchars_buf_len && wchars[i] != 0; i++)
		AddInputCharacter(wchars[i]);
}

//-----------------------------------------------------------------------------
// HELPERS
//-----------------------------------------------------------------------------

#define IM_F32_TO_INT8_UNBOUND(_VAL)    ((int)((_VAL) * 255.0f + ((_VAL)>=0 ? 0.5f : -0.5f)))   // Unsaturated, for display purpose 
#define IM_F32_TO_INT8_SAT(_VAL)        ((int)(ImSaturate(_VAL) * 255.0f + 0.5f))               // Saturated, always output 0..255

// Play it nice with Windows users. Notepad in 2015 still doesn't display text data with Unix-style \n.
#ifdef _WIN32
#define IM_NEWLINE "\r\n"
#else
#define IM_NEWLINE "\n"
#endif

ImVec2 ImLineClosestPoint(const ImVec2& a, const ImVec2& b, const ImVec2& p)
{
	ImVec2 ap = p - a;
	ImVec2 ab_dir = b - a;
	float ab_len = sqrtf(ab_dir.x * ab_dir.x + ab_dir.y * ab_dir.y);
	ab_dir *= 1.0f / ab_len;
	float dot = ap.x * ab_dir.x + ap.y * ab_dir.y;
	if (dot < 0.0f)
		return a;
	if (dot > ab_len)
		return b;
	return a + ab_dir * dot;
}

bool ImTriangleContainsPoint(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p)
{
	bool b1 = ((p.x - b.x) * (a.y - b.y) - (p.y - b.y) * (a.x - b.x)) < 0.0f;
	bool b2 = ((p.x - c.x) * (b.y - c.y) - (p.y - c.y) * (b.x - c.x)) < 0.0f;
	bool b3 = ((p.x - a.x) * (c.y - a.y) - (p.y - a.y) * (c.x - a.x)) < 0.0f;
	return ((b1 == b2) && (b2 == b3));
}

void ImTriangleBarycentricCoords(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p, float& out_u, float& out_v, float& out_w)
{
	ImVec2 v0 = b - a;
	ImVec2 v1 = c - a;
	ImVec2 v2 = p - a;
	const float denom = v0.x * v1.y - v1.x * v0.y;
	out_v = (v2.x * v1.y - v1.x * v2.y) / denom;
	out_w = (v0.x * v2.y - v2.x * v0.y) / denom;
	out_u = 1.0f - out_v - out_w;
}

ImVec2 ImTriangleClosestPoint(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p)
{
	ImVec2 proj_ab = ImLineClosestPoint(a, b, p);
	ImVec2 proj_bc = ImLineClosestPoint(b, c, p);
	ImVec2 proj_ca = ImLineClosestPoint(c, a, p);
	float dist2_ab = ImLengthSqr(p - proj_ab);
	float dist2_bc = ImLengthSqr(p - proj_bc);
	float dist2_ca = ImLengthSqr(p - proj_ca);
	float m = ImMin(dist2_ab, ImMin(dist2_bc, dist2_ca));
	if (m == dist2_ab)
		return proj_ab;
	if (m == dist2_bc)
		return proj_bc;
	return proj_ca;
}

int ImStricmp(const char* str1, const char* str2)
{
	int d;
	while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; }
	return d;
}

int ImStrnicmp(const char* str1, const char* str2, size_t count)
{
	int d = 0;
	while (count > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; count--; }
	return d;
}

void ImStrncpy(char* dst, const char* src, size_t count)
{
	if (count < 1) return;
	strncpy(dst, src, count);
	dst[count - 1] = 0;
}

char* ImStrdup(const char *str)
{
	size_t len = strlen(str) + 1;
	void* buf = ImGui::MemAlloc(len);
	return (char*)memcpy(buf, (const void*)str, len);
}

char* ImStrchrRange(const char* str, const char* str_end, char c)
{
	for (; str < str_end; str++)
		if (*str == c)
			return (char*)str;
	return NULL;
}

int ImStrlenW(const ImWchar* str)
{
	int n = 0;
	while (*str++) n++;
	return n;
}

const ImWchar* ImStrbolW(const ImWchar* buf_mid_line, const ImWchar* buf_begin) // find beginning-of-line
{
	while (buf_mid_line > buf_begin && buf_mid_line[-1] != '\n')
		buf_mid_line--;
	return buf_mid_line;
}

const char* ImStristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end)
{
	if (!needle_end)
		needle_end = needle + strlen(needle);

	const char un0 = (char)toupper(*needle);
	while ((!haystack_end && *haystack) || (haystack_end && haystack < haystack_end))
	{
		if (toupper(*haystack) == un0)
		{
			const char* b = needle + 1;
			for (const char* a = haystack + 1; b < needle_end; a++, b++)
				if (toupper(*a) != toupper(*b))
					break;
			if (b == needle_end)
				return haystack;
		}
		haystack++;
	}
	return NULL;
}

static const char* ImAtoi(const char* src, int* output)
{
	int negative = 0;
	if (*src == '-') { negative = 1; src++; }
	if (*src == '+') { src++; }
	int v = 0;
	while (*src >= '0' && *src <= '9')
		v = (v * 10) + (*src++ - '0');
	*output = negative ? -v : v;
	return src;
}

// A) MSVC version appears to return -1 on overflow, whereas glibc appears to return total count (which may be >= buf_size). 
// Ideally we would test for only one of those limits at runtime depending on the behavior the vsnprintf(), but trying to deduct it at compile time sounds like a pandora can of worm.
// B) When buf==NULL vsnprintf() will return the output size.
#ifndef IMGUI_DISABLE_FORMAT_STRING_FUNCTIONS
int ImFormatString(char* buf, size_t buf_size, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int w = vsnprintf(buf, buf_size, fmt, args);
	va_end(args);
	if (buf == NULL)
		return w;
	if (w == -1 || w >= (int)buf_size)
		w = (int)buf_size - 1;
	buf[w] = 0;
	return w;
}

int ImFormatStringV(char* buf, size_t buf_size, const char* fmt, va_list args)
{
	int w = vsnprintf(buf, buf_size, fmt, args);
	if (buf == NULL)
		return w;
	if (w == -1 || w >= (int)buf_size)
		w = (int)buf_size - 1;
	buf[w] = 0;
	return w;
}
#endif // #ifdef IMGUI_DISABLE_FORMAT_STRING_FUNCTIONS

// Pass data_size==0 for zero-terminated strings
// FIXME-OPT: Replace with e.g. FNV1a hash? CRC32 pretty much randomly access 1KB. Need to do proper measurements.
ImU32 ImHash(const void* data, int data_size, ImU32 seed)
{
	static ImU32 crc32_lut[256] = { 0 };
	if (!crc32_lut[1])
	{
		const ImU32 polynomial = 0xEDB88320;
		for (ImU32 i = 0; i < 256; i++)
		{
			ImU32 crc = i;
			for (ImU32 j = 0; j < 8; j++)
				crc = (crc >> 1) ^ (ImU32(-int(crc & 1)) & polynomial);
			crc32_lut[i] = crc;
		}
	}

	seed = ~seed;
	ImU32 crc = seed;
	const unsigned char* current = (const unsigned char*)data;

	if (data_size > 0)
	{
		// Known size
		while (data_size--)
			crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ *current++];
	}
	else
	{
		// Zero-terminated string
		while (unsigned char c = *current++)
		{
			// We support a syntax of "label###id" where only "###id" is included in the hash, and only "label" gets displayed.
			// Because this syntax is rarely used we are optimizing for the common case.
			// - If we reach ### in the string we discard the hash so far and reset to the seed.
			// - We don't do 'current += 2; continue;' after handling ### to keep the code smaller.
			if (c == '#' && current[0] == '#' && current[1] == '#')
				crc = seed;
			crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ c];
		}
	}
	return ~crc;
}

//-----------------------------------------------------------------------------
// ImText* helpers
//-----------------------------------------------------------------------------

// Convert UTF-8 to 32-bits character, process single character input.
// Based on stb_from_utf8() from github.com/nothings/stb/
// We handle UTF-8 decoding error by skipping forward.
int ImTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end)
{
	unsigned int c = (unsigned int)-1;
	const unsigned char* str = (const unsigned char*)in_text;
	if (!(*str & 0x80))
	{
		c = (unsigned int)(*str++);
		*out_char = c;
		return 1;
	}
	if ((*str & 0xe0) == 0xc0)
	{
		*out_char = 0xFFFD; // will be invalid but not end of string
		if (in_text_end && in_text_end - (const char*)str < 2) return 1;
		if (*str < 0xc2) return 2;
		c = (unsigned int)((*str++ & 0x1f) << 6);
		if ((*str & 0xc0) != 0x80) return 2;
		c += (*str++ & 0x3f);
		*out_char = c;
		return 2;
	}
	if ((*str & 0xf0) == 0xe0)
	{
		*out_char = 0xFFFD; // will be invalid but not end of string
		if (in_text_end && in_text_end - (const char*)str < 3) return 1;
		if (*str == 0xe0 && (str[1] < 0xa0 || str[1] > 0xbf)) return 3;
		if (*str == 0xed && str[1] > 0x9f) return 3; // str[1] < 0x80 is checked below
		c = (unsigned int)((*str++ & 0x0f) << 12);
		if ((*str & 0xc0) != 0x80) return 3;
		c += (unsigned int)((*str++ & 0x3f) << 6);
		if ((*str & 0xc0) != 0x80) return 3;
		c += (*str++ & 0x3f);
		*out_char = c;
		return 3;
	}
	if ((*str & 0xf8) == 0xf0)
	{
		*out_char = 0xFFFD; // will be invalid but not end of string
		if (in_text_end && in_text_end - (const char*)str < 4) return 1;
		if (*str > 0xf4) return 4;
		if (*str == 0xf0 && (str[1] < 0x90 || str[1] > 0xbf)) return 4;
		if (*str == 0xf4 && str[1] > 0x8f) return 4; // str[1] < 0x80 is checked below
		c = (unsigned int)((*str++ & 0x07) << 18);
		if ((*str & 0xc0) != 0x80) return 4;
		c += (unsigned int)((*str++ & 0x3f) << 12);
		if ((*str & 0xc0) != 0x80) return 4;
		c += (unsigned int)((*str++ & 0x3f) << 6);
		if ((*str & 0xc0) != 0x80) return 4;
		c += (*str++ & 0x3f);
		// utf-8 encodings of values used in surrogate pairs are invalid
		if ((c & 0xFFFFF800) == 0xD800) return 4;
		*out_char = c;
		return 4;
	}
	*out_char = 0;
	return 0;
}

int ImTextStrFromUtf8(ImWchar* buf, int buf_size, const char* in_text, const char* in_text_end, const char** in_text_remaining)
{
	ImWchar* buf_out = buf;
	ImWchar* buf_end = buf + buf_size;
	while (buf_out < buf_end - 1 && (!in_text_end || in_text < in_text_end) && *in_text)
	{
		unsigned int c;
		in_text += ImTextCharFromUtf8(&c, in_text, in_text_end);
		if (c == 0)
			break;
		if (c < 0x10000)    // FIXME: Losing characters that don't fit in 2 bytes
			*buf_out++ = (ImWchar)c;
	}
	*buf_out = 0;
	if (in_text_remaining)
		*in_text_remaining = in_text;
	return (int)(buf_out - buf);
}

int ImTextCountCharsFromUtf8(const char* in_text, const char* in_text_end)
{
	int char_count = 0;
	while ((!in_text_end || in_text < in_text_end) && *in_text)
	{
		unsigned int c;
		in_text += ImTextCharFromUtf8(&c, in_text, in_text_end);
		if (c == 0)
			break;
		if (c < 0x10000)
			char_count++;
	}
	return char_count;
}

// Based on stb_to_utf8() from github.com/nothings/stb/
static inline int ImTextCharToUtf8(char* buf, int buf_size, unsigned int c)
{
	if (c < 0x80)
	{
		buf[0] = (char)c;
		return 1;
	}
	if (c < 0x800)
	{
		if (buf_size < 2) return 0;
		buf[0] = (char)(0xc0 + (c >> 6));
		buf[1] = (char)(0x80 + (c & 0x3f));
		return 2;
	}
	if (c >= 0xdc00 && c < 0xe000)
	{
		return 0;
	}
	if (c >= 0xd800 && c < 0xdc00)
	{
		if (buf_size < 4) return 0;
		buf[0] = (char)(0xf0 + (c >> 18));
		buf[1] = (char)(0x80 + ((c >> 12) & 0x3f));
		buf[2] = (char)(0x80 + ((c >> 6) & 0x3f));
		buf[3] = (char)(0x80 + ((c) & 0x3f));
		return 4;
	}
	//else if (c < 0x10000)
	{
		if (buf_size < 3) return 0;
		buf[0] = (char)(0xe0 + (c >> 12));
		buf[1] = (char)(0x80 + ((c >> 6) & 0x3f));
		buf[2] = (char)(0x80 + ((c) & 0x3f));
		return 3;
	}
}

static inline int ImTextCountUtf8BytesFromChar(unsigned int c)
{
	if (c < 0x80) return 1;
	if (c < 0x800) return 2;
	if (c >= 0xdc00 && c < 0xe000) return 0;
	if (c >= 0xd800 && c < 0xdc00) return 4;
	return 3;
}

int ImTextStrToUtf8(char* buf, int buf_size, const ImWchar* in_text, const ImWchar* in_text_end)
{
	char* buf_out = buf;
	const char* buf_end = buf + buf_size;
	while (buf_out < buf_end - 1 && (!in_text_end || in_text < in_text_end) && *in_text)
	{
		unsigned int c = (unsigned int)(*in_text++);
		if (c < 0x80)
			*buf_out++ = (char)c;
		else
			buf_out += ImTextCharToUtf8(buf_out, (int)(buf_end - buf_out - 1), c);
	}
	*buf_out = 0;
	return (int)(buf_out - buf);
}

int ImTextCountUtf8BytesFromStr(const ImWchar* in_text, const ImWchar* in_text_end)
{
	int bytes_count = 0;
	while ((!in_text_end || in_text < in_text_end) && *in_text)
	{
		unsigned int c = (unsigned int)(*in_text++);
		if (c < 0x80)
			bytes_count++;
		else
			bytes_count += ImTextCountUtf8BytesFromChar(c);
	}
	return bytes_count;
}

ImVec4 ImGui::ColorConvertU32ToFloat4(ImU32 in)
{
	float s = 1.0f / 255.0f;
	return ImVec4(
		((in >> IM_COL32_R_SHIFT) & 0xFF) * s,
		((in >> IM_COL32_G_SHIFT) & 0xFF) * s,
		((in >> IM_COL32_B_SHIFT) & 0xFF) * s,
		((in >> IM_COL32_A_SHIFT) & 0xFF) * s);
}

ImU32 ImGui::ColorConvertFloat4ToU32(const ImVec4& in)
{
	ImU32 out;
	out = ((ImU32)IM_F32_TO_INT8_SAT(in.x)) << IM_COL32_R_SHIFT;
	out |= ((ImU32)IM_F32_TO_INT8_SAT(in.y)) << IM_COL32_G_SHIFT;
	out |= ((ImU32)IM_F32_TO_INT8_SAT(in.z)) << IM_COL32_B_SHIFT;
	out |= ((ImU32)IM_F32_TO_INT8_SAT(in.w)) << IM_COL32_A_SHIFT;
	return out;
}

ImU32 ImGui::GetColorU32(ImGuiCol idx, float alpha_mul)
{
	ImGuiStyle& style = GImGui->Style;
	ImVec4 c = style.Colors[idx];
	c.w *= style.Alpha * alpha_mul;
	return ColorConvertFloat4ToU32(c);
}

ImU32 ImGui::GetColorU32(const ImVec4& col)
{
	ImGuiStyle& style = GImGui->Style;
	ImVec4 c = col;
	c.w *= style.Alpha;
	return ColorConvertFloat4ToU32(c);
}

const ImVec4& ImGui::GetStyleColorVec4(ImGuiCol idx)
{
	ImGuiStyle& style = GImGui->Style;
	return style.Colors[idx];
}

ImU32 ImGui::GetColorU32(ImU32 col)
{
	float style_alpha = GImGui->Style.Alpha;
	if (style_alpha >= 1.0f)
		return col;
	int a = (col & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT;
	a = (int)(a * style_alpha); // We don't need to clamp 0..255 because Style.Alpha is in 0..1 range.
	return (col & ~IM_COL32_A_MASK) | (a << IM_COL32_A_SHIFT);
}

// Convert rgb floats ([0-1],[0-1],[0-1]) to hsv floats ([0-1],[0-1],[0-1]), from Foley & van Dam p592
// Optimized http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
void ImGui::ColorConvertRGBtoHSV(float r, float g, float b, float& out_h, float& out_s, float& out_v)
{
	float K = 0.f;
	if (g < b)
	{
		ImSwap(g, b);
		K = -1.f;
	}
	if (r < g)
	{
		ImSwap(r, g);
		K = -2.f / 6.f - K;
	}

	const float chroma = r - (g < b ? g : b);
	out_h = fabsf(K + (g - b) / (6.f * chroma + 1e-20f));
	out_s = chroma / (r + 1e-20f);
	out_v = r;
}

// Convert hsv floats ([0-1],[0-1],[0-1]) to rgb floats ([0-1],[0-1],[0-1]), from Foley & van Dam p593
// also http://en.wikipedia.org/wiki/HSL_and_HSV
void ImGui::ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b)
{
	if (s == 0.0f)
	{
		// gray
		out_r = out_g = out_b = v;
		return;
	}

	h = fmodf(h, 1.0f) / (60.0f / 360.0f);
	int   i = (int)h;
	float f = h - (float)i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - s * f);
	float t = v * (1.0f - s * (1.0f - f));

	switch (i)
	{
	case 0: out_r = v; out_g = t; out_b = p; break;
	case 1: out_r = q; out_g = v; out_b = p; break;
	case 2: out_r = p; out_g = v; out_b = t; break;
	case 3: out_r = p; out_g = q; out_b = v; break;
	case 4: out_r = t; out_g = p; out_b = v; break;
	case 5: default: out_r = v; out_g = p; out_b = q; break;
	}
}

FILE* ImFileOpen(const char* filename, const char* mode)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
	// We need a fopen() wrapper because MSVC/Windows fopen doesn't handle UTF-8 filenames. Converting both strings from UTF-8 to wchar format (using a single allocation, because we can)
	const int filename_wsize = ImTextCountCharsFromUtf8(filename, NULL) + 1;
	const int mode_wsize = ImTextCountCharsFromUtf8(mode, NULL) + 1;
	ImVector<ImWchar> buf;
	buf.resize(filename_wsize + mode_wsize);
	ImTextStrFromUtf8(&buf[0], filename_wsize, filename, NULL);
	ImTextStrFromUtf8(&buf[filename_wsize], mode_wsize, mode, NULL);
	return _wfopen((wchar_t*)&buf[0], (wchar_t*)&buf[filename_wsize]);
#else
	return fopen(filename, mode);
#endif
}

// Load file content into memory
// Memory allocated with ImGui::MemAlloc(), must be freed by user using ImGui::MemFree()
void* ImFileLoadToMemory(const char* filename, const char* file_open_mode, int* out_file_size, int padding_bytes)
{
	IM_ASSERT(filename && file_open_mode);
	if (out_file_size)
		*out_file_size = 0;

	FILE* f;
	if ((f = ImFileOpen(filename, file_open_mode)) == NULL)
		return NULL;

	long file_size_signed;
	if (fseek(f, 0, SEEK_END) || (file_size_signed = ftell(f)) == -1 || fseek(f, 0, SEEK_SET))
	{
		fclose(f);
		return NULL;
	}

	int file_size = (int)file_size_signed;
	void* file_data = ImGui::MemAlloc(file_size + padding_bytes);
	if (file_data == NULL)
	{
		fclose(f);
		return NULL;
	}
	if (fread(file_data, 1, (size_t)file_size, f) != (size_t)file_size)
	{
		fclose(f);
		ImGui::MemFree(file_data);
		return NULL;
	}
	if (padding_bytes > 0)
		memset((void *)(((char*)file_data) + file_size), 0, padding_bytes);

	fclose(f);
	if (out_file_size)
		*out_file_size = file_size;

	return file_data;
}

//-----------------------------------------------------------------------------
// ImGuiStorage
// Helper: Key->value storage
//-----------------------------------------------------------------------------

// std::lower_bound but without the bullshit
static ImVector<ImGuiStorage::Pair>::iterator LowerBound(ImVector<ImGuiStorage::Pair>& data, ImGuiID key)
{
	ImVector<ImGuiStorage::Pair>::iterator first = data.begin();
	ImVector<ImGuiStorage::Pair>::iterator last = data.end();
	size_t count = (size_t)(last - first);
	while (count > 0)
	{
		size_t count2 = count >> 1;
		ImVector<ImGuiStorage::Pair>::iterator mid = first + count2;
		if (mid->key < key)
		{
			first = ++mid;
			count -= count2 + 1;
		}
		else
		{
			count = count2;
		}
	}
	return first;
}

// For quicker full rebuild of a storage (instead of an incremental one), you may add all your contents and then sort once.
void ImGuiStorage::BuildSortByKey()
{
	struct StaticFunc
	{
		static int PairCompareByID(const void* lhs, const void* rhs)
		{
			// We can't just do a subtraction because qsort uses signed integers and subtracting our ID doesn't play well with that.
			if (((const Pair*)lhs)->key > ((const Pair*)rhs)->key) return +1;
			if (((const Pair*)lhs)->key < ((const Pair*)rhs)->key) return -1;
			return 0;
		}
	};
	if (Data.Size > 1)
		qsort(Data.Data, (size_t)Data.Size, sizeof(Pair), StaticFunc::PairCompareByID);
}

int ImGuiStorage::GetInt(ImGuiID key, int default_val) const
{
	ImVector<Pair>::iterator it = LowerBound(const_cast<ImVector<ImGuiStorage::Pair>&>(Data), key);
	if (it == Data.end() || it->key != key)
		return default_val;
	return it->val_i;
}

bool ImGuiStorage::GetBool(ImGuiID key, bool default_val) const
{
	return GetInt(key, default_val ? 1 : 0) != 0;
}

float ImGuiStorage::GetFloat(ImGuiID key, float default_val) const
{
	ImVector<Pair>::iterator it = LowerBound(const_cast<ImVector<ImGuiStorage::Pair>&>(Data), key);
	if (it == Data.end() || it->key != key)
		return default_val;
	return it->val_f;
}

void* ImGuiStorage::GetVoidPtr(ImGuiID key) const
{
	ImVector<Pair>::iterator it = LowerBound(const_cast<ImVector<ImGuiStorage::Pair>&>(Data), key);
	if (it == Data.end() || it->key != key)
		return NULL;
	return it->val_p;
}

// References are only valid until a new value is added to the storage. Calling a Set***() function or a Get***Ref() function invalidates the pointer.
int* ImGuiStorage::GetIntRef(ImGuiID key, int default_val)
{
	ImVector<Pair>::iterator it = LowerBound(Data, key);
	if (it == Data.end() || it->key != key)
		it = Data.insert(it, Pair(key, default_val));
	return &it->val_i;
}

bool* ImGuiStorage::GetBoolRef(ImGuiID key, bool default_val)
{
	return (bool*)GetIntRef(key, default_val ? 1 : 0);
}

float* ImGuiStorage::GetFloatRef(ImGuiID key, float default_val)
{
	ImVector<Pair>::iterator it = LowerBound(Data, key);
	if (it == Data.end() || it->key != key)
		it = Data.insert(it, Pair(key, default_val));
	return &it->val_f;
}

void** ImGuiStorage::GetVoidPtrRef(ImGuiID key, void* default_val)
{
	ImVector<Pair>::iterator it = LowerBound(Data, key);
	if (it == Data.end() || it->key != key)
		it = Data.insert(it, Pair(key, default_val));
	return &it->val_p;
}

// FIXME-OPT: Need a way to reuse the result of lower_bound when doing GetInt()/SetInt() - not too bad because it only happens on explicit interaction (maximum one a frame)
void ImGuiStorage::SetInt(ImGuiID key, int val)
{
	ImVector<Pair>::iterator it = LowerBound(Data, key);
	if (it == Data.end() || it->key != key)
	{
		Data.insert(it, Pair(key, val));
		return;
	}
	it->val_i = val;
}

void ImGuiStorage::SetBool(ImGuiID key, bool val)
{
	SetInt(key, val ? 1 : 0);
}

void ImGuiStorage::SetFloat(ImGuiID key, float val)
{
	ImVector<Pair>::iterator it = LowerBound(Data, key);
	if (it == Data.end() || it->key != key)
	{
		Data.insert(it, Pair(key, val));
		return;
	}
	it->val_f = val;
}

void ImGuiStorage::SetVoidPtr(ImGuiID key, void* val)
{
	ImVector<Pair>::iterator it = LowerBound(Data, key);
	if (it == Data.end() || it->key != key)
	{
		Data.insert(it, Pair(key, val));
		return;
	}
	it->val_p = val;
}

void ImGuiStorage::SetAllInt(int v)
{
	for (int i = 0; i < Data.Size; i++)
		Data[i].val_i = v;
}

//-----------------------------------------------------------------------------
// ImGuiTextFilter
//-----------------------------------------------------------------------------

// Helper: Parse and apply text filters. In format "aaaaa[,bbbb][,ccccc]"
ImGuiTextFilter::ImGuiTextFilter(const char* default_filter)
{
	if (default_filter)
	{
		ImStrncpy(InputBuf, default_filter, IM_ARRAYSIZE(InputBuf));
		Build();
	}
	else
	{
		InputBuf[0] = 0;
		CountGrep = 0;
	}
}

bool ImGuiTextFilter::Draw(const char* label, float width)
{
	if (width != 0.0f)
		ImGui::PushItemWidth(width);
	bool value_changed = ImGui::InputText(label, InputBuf, IM_ARRAYSIZE(InputBuf));
	if (width != 0.0f)
		ImGui::PopItemWidth();
	if (value_changed)
		Build();
	return value_changed;
}

void ImGuiTextFilter::TextRange::split(char separator, ImVector<TextRange>& out)
{
	out.resize(0);
	const char* wb = b;
	const char* we = wb;
	while (we < e)
	{
		if (*we == separator)
		{
			out.push_back(TextRange(wb, we));
			wb = we + 1;
		}
		we++;
	}
	if (wb != we)
		out.push_back(TextRange(wb, we));
}

void ImGuiTextFilter::Build()
{
	Filters.resize(0);
	TextRange input_range(InputBuf, InputBuf + strlen(InputBuf));
	input_range.split(',', Filters);

	CountGrep = 0;
	for (int i = 0; i != Filters.Size; i++)
	{
		Filters[i].trim_blanks();
		if (Filters[i].empty())
			continue;
		if (Filters[i].front() != '-')
			CountGrep += 1;
	}
}

bool ImGuiTextFilter::PassFilter(const char* text, const char* text_end) const
{
	if (Filters.empty())
		return true;

	if (text == NULL)
		text = "";

	for (int i = 0; i != Filters.Size; i++)
	{
		const TextRange& f = Filters[i];
		if (f.empty())
			continue;
		if (f.front() == '-')
		{
			// Subtract
			if (ImStristr(text, text_end, f.begin() + 1, f.end()) != NULL)
				return false;
		}
		else
		{
			// Grep
			if (ImStristr(text, text_end, f.begin(), f.end()) != NULL)
				return true;
		}
	}

	// Implicit * grep
	if (CountGrep == 0)
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// ImGuiTextBuffer
//-----------------------------------------------------------------------------

// On some platform vsnprintf() takes va_list by reference and modifies it.
// va_copy is the 'correct' way to copy a va_list but Visual Studio prior to 2013 doesn't have it.
#ifndef va_copy
#define va_copy(dest, src) (dest = src)
#endif

// Helper: Text buffer for logging/accumulating text
void ImGuiTextBuffer::appendfv(const char* fmt, va_list args)
{
	va_list args_copy;
	va_copy(args_copy, args);

	int len = ImFormatStringV(NULL, 0, fmt, args);         // FIXME-OPT: could do a first pass write attempt, likely successful on first pass.
	if (len <= 0)
		return;

	const int write_off = Buf.Size;
	const int needed_sz = write_off + len;
	if (write_off + len >= Buf.Capacity)
	{
		int double_capacity = Buf.Capacity * 2;
		Buf.reserve(needed_sz > double_capacity ? needed_sz : double_capacity);
	}

	Buf.resize(needed_sz);
	ImFormatStringV(&Buf[write_off - 1], len + 1, fmt, args_copy);
}

void ImGuiTextBuffer::appendf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	appendfv(fmt, args);
	va_end(args);
}

//-----------------------------------------------------------------------------
// ImGuiSimpleColumns (internal use only)
//-----------------------------------------------------------------------------

ImGuiMenuColumns::ImGuiMenuColumns()
{
	Count = 0;
	Spacing = Width = NextWidth = 0.0f;
	memset(Pos, 0, sizeof(Pos));
	memset(NextWidths, 0, sizeof(NextWidths));
}

void ImGuiMenuColumns::Update(int count, float spacing, bool clear)
{
	IM_ASSERT(Count <= IM_ARRAYSIZE(Pos));
	Count = count;
	Width = NextWidth = 0.0f;
	Spacing = spacing;
	if (clear) memset(NextWidths, 0, sizeof(NextWidths));
	for (int i = 0; i < Count; i++)
	{
		if (i > 0 && NextWidths[i] > 0.0f)
			Width += Spacing;
		Pos[i] = (float)(int)Width;
		Width += NextWidths[i];
		NextWidths[i] = 0.0f;
	}
}

float ImGuiMenuColumns::DeclColumns(float w0, float w1, float w2) // not using va_arg because they promote float to double
{
	NextWidth = 0.0f;
	NextWidths[0] = ImMax(NextWidths[0], w0);
	NextWidths[1] = ImMax(NextWidths[1], w1);
	NextWidths[2] = ImMax(NextWidths[2], w2);
	for (int i = 0; i < 3; i++)
		NextWidth += NextWidths[i] + ((i > 0 && NextWidths[i] > 0.0f) ? Spacing : 0.0f);
	return ImMax(Width, NextWidth);
}

float ImGuiMenuColumns::CalcExtraSpace(float avail_w)
{
	return ImMax(0.0f, avail_w - Width);
}

//-----------------------------------------------------------------------------
// ImGuiListClipper
//-----------------------------------------------------------------------------

static void SetCursorPosYAndSetupDummyPrevLine(float pos_y, float line_height)
{
	// Set cursor position and a few other things so that SetScrollHere() and Columns() can work when seeking cursor. 
	// FIXME: It is problematic that we have to do that here, because custom/equivalent end-user code would stumble on the same issue. Consider moving within SetCursorXXX functions?
	ImGui::SetCursorPosY(pos_y);
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DC.CursorPosPrevLine.y = window->DC.CursorPos.y - line_height;      // Setting those fields so that SetScrollHere() can properly function after the end of our clipper usage.
	window->DC.PrevLineHeight = (line_height - GImGui->Style.ItemSpacing.y);    // If we end up needing more accurate data (to e.g. use SameLine) we may as well make the clipper have a fourth step to let user process and display the last item in their list.
	if (window->DC.ColumnsSet)
		window->DC.ColumnsSet->CellMinY = window->DC.CursorPos.y;           // Setting this so that cell Y position are set properly
}

// Use case A: Begin() called from constructor with items_height<0, then called again from Sync() in StepNo 1
// Use case B: Begin() called from constructor with items_height>0
// FIXME-LEGACY: Ideally we should remove the Begin/End functions but they are part of the legacy API we still support. This is why some of the code in Step() calling Begin() and reassign some fields, spaghetti style.
void ImGuiListClipper::Begin(int count, float items_height)
{
	StartPosY = ImGui::GetCursorPosY();
	ItemsHeight = items_height;
	ItemsCount = count;
	StepNo = 0;
	DisplayEnd = DisplayStart = -1;
	if (ItemsHeight > 0.0f)
	{
		ImGui::CalcListClipping(ItemsCount, ItemsHeight, &DisplayStart, &DisplayEnd); // calculate how many to clip/display
		if (DisplayStart > 0)
			SetCursorPosYAndSetupDummyPrevLine(StartPosY + DisplayStart * ItemsHeight, ItemsHeight); // advance cursor
		StepNo = 2;
	}
}

void ImGuiListClipper::End()
{
	if (ItemsCount < 0)
		return;
	// In theory here we should assert that ImGui::GetCursorPosY() == StartPosY + DisplayEnd * ItemsHeight, but it feels saner to just seek at the end and not assert/crash the user.
	if (ItemsCount < INT_MAX)
		SetCursorPosYAndSetupDummyPrevLine(StartPosY + ItemsCount * ItemsHeight, ItemsHeight); // advance cursor
	ItemsCount = -1;
	StepNo = 3;
}

bool ImGuiListClipper::Step()
{
	if (ItemsCount == 0 || ImGui::GetCurrentWindowRead()->SkipItems)
	{
		ItemsCount = -1;
		return false;
	}
	if (StepNo == 0) // Step 0: the clipper let you process the first element, regardless of it being visible or not, so we can measure the element height.
	{
		DisplayStart = 0;
		DisplayEnd = 1;
		StartPosY = ImGui::GetCursorPosY();
		StepNo = 1;
		return true;
	}
	if (StepNo == 1) // Step 1: the clipper infer height from first element, calculate the actual range of elements to display, and position the cursor before the first element.
	{
		if (ItemsCount == 1) { ItemsCount = -1; return false; }
		float items_height = ImGui::GetCursorPosY() - StartPosY;
		IM_ASSERT(items_height > 0.0f);   // If this triggers, it means Item 0 hasn't moved the cursor vertically
		Begin(ItemsCount - 1, items_height);
		DisplayStart++;
		DisplayEnd++;
		StepNo = 3;
		return true;
	}
	if (StepNo == 2) // Step 2: dummy step only required if an explicit items_height was passed to constructor or Begin() and user still call Step(). Does nothing and switch to Step 3.
	{
		IM_ASSERT(DisplayStart >= 0 && DisplayEnd >= 0);
		StepNo = 3;
		return true;
	}
	if (StepNo == 3) // Step 3: the clipper validate that we have reached the expected Y position (corresponding to element DisplayEnd), advance the cursor to the end of the list and then returns 'false' to end the loop.
		End();
	return false;
}

//-----------------------------------------------------------------------------
// ImGuiWindow
//-----------------------------------------------------------------------------

ImGuiWindow::ImGuiWindow(ImGuiContext* context, const char* name)
{
	Name = ImStrdup(name);
	ID = ImHash(name, 0);
	IDStack.push_back(ID);
	Flags = 0;
	PosFloat = Pos = ImVec2(0.0f, 0.0f);
	Size = SizeFull = ImVec2(0.0f, 0.0f);
	SizeContents = SizeContentsExplicit = ImVec2(0.0f, 0.0f);
	WindowPadding = ImVec2(0.0f, 0.0f);
	WindowRounding = 0.0f;
	WindowBorderSize = 0.0f;
	MoveId = GetID("#MOVE");
	Scroll = ImVec2(0.0f, 0.0f);
	ScrollTarget = ImVec2(FLT_MAX, FLT_MAX);
	ScrollTargetCenterRatio = ImVec2(0.5f, 0.5f);
	ScrollbarX = ScrollbarY = false;
	ScrollbarSizes = ImVec2(0.0f, 0.0f);
	Active = WasActive = false;
	WriteAccessed = false;
	Collapsed = false;
	SkipItems = false;
	Appearing = false;
	CloseButton = false;
	BeginOrderWithinParent = -1;
	BeginOrderWithinContext = -1;
	BeginCount = 0;
	PopupId = 0;
	AutoFitFramesX = AutoFitFramesY = -1;
	AutoFitOnlyGrows = false;
	AutoFitChildAxises = 0x00;
	AutoPosLastDirection = ImGuiDir_None;
	HiddenFrames = 0;
	SetWindowPosAllowFlags = SetWindowSizeAllowFlags = SetWindowCollapsedAllowFlags = ImGuiCond_Always | ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing;
	SetWindowPosVal = SetWindowPosPivot = ImVec2(FLT_MAX, FLT_MAX);

	LastFrameActive = -1;
	ItemWidthDefault = 0.0f;
	FontWindowScale = 1.0f;

	DrawList = IM_NEW(ImDrawList)(&context->DrawListSharedData);
	DrawList->_OwnerName = Name;
	ParentWindow = NULL;
	RootWindow = NULL;
	RootNonPopupWindow = NULL;

	FocusIdxAllCounter = FocusIdxTabCounter = -1;
	FocusIdxAllRequestCurrent = FocusIdxTabRequestCurrent = INT_MAX;
	FocusIdxAllRequestNext = FocusIdxTabRequestNext = INT_MAX;
}

ImGuiWindow::~ImGuiWindow()
{
	IM_DELETE(DrawList);
	IM_DELETE(Name);
	for (int i = 0; i != ColumnsStorage.Size; i++)
		ColumnsStorage[i].~ImGuiColumnsSet();
}

ImGuiID ImGuiWindow::GetID(const char* str, const char* str_end)
{
	ImGuiID seed = IDStack.back();
	ImGuiID id = ImHash(str, str_end ? (int)(str_end - str) : 0, seed);
	ImGui::KeepAliveID(id);
	return id;
}

ImGuiID ImGuiWindow::GetID(const void* ptr)
{
	ImGuiID seed = IDStack.back();
	ImGuiID id = ImHash(&ptr, sizeof(void*), seed);
	ImGui::KeepAliveID(id);
	return id;
}

ImGuiID ImGuiWindow::GetIDNoKeepAlive(const char* str, const char* str_end)
{
	ImGuiID seed = IDStack.back();
	return ImHash(str, str_end ? (int)(str_end - str) : 0, seed);
}

// This is only used in rare/specific situations to manufacture an ID out of nowhere.
ImGuiID ImGuiWindow::GetIDFromRectangle(const ImRect& r_abs)
{
	ImGuiID seed = IDStack.back();
	const int r_rel[4] = { (int)(r_abs.Min.x - Pos.x), (int)(r_abs.Min.y - Pos.y), (int)(r_abs.Max.x - Pos.x), (int)(r_abs.Max.y - Pos.y) };
	ImGuiID id = ImHash(&r_rel, sizeof(r_rel), seed);
	ImGui::KeepAliveID(id);
	return id;
}

//-----------------------------------------------------------------------------
// Internal API exposed in imgui_internal.h
//-----------------------------------------------------------------------------

static void SetCurrentWindow(ImGuiWindow* window)
{
	ImGuiContext& g = *GImGui;
	g.CurrentWindow = window;
	if (window)
		g.FontSize = g.DrawListSharedData.FontSize = window->CalcFontSize();
}

void ImGui::SetActiveID(ImGuiID id, ImGuiWindow* window)
{
	ImGuiContext& g = *GImGui;
	g.ActiveIdIsJustActivated = (g.ActiveId != id);
	if (g.ActiveIdIsJustActivated)
		g.ActiveIdTimer = 0.0f;
	g.ActiveId = id;
	g.ActiveIdAllowOverlap = false;
	g.ActiveIdIsAlive |= (id != 0);
	g.ActiveIdWindow = window;
}

void ImGui::ClearActiveID()
{
	SetActiveID(0, NULL);
}

void ImGui::SetHoveredID(ImGuiID id)
{
	ImGuiContext& g = *GImGui;
	g.HoveredId = id;
	g.HoveredIdAllowOverlap = false;
	g.HoveredIdTimer = (id != 0 && g.HoveredIdPreviousFrame == id) ? (g.HoveredIdTimer + g.IO.DeltaTime) : 0.0f;
}

ImGuiID ImGui::GetHoveredID()
{
	ImGuiContext& g = *GImGui;
	return g.HoveredId ? g.HoveredId : g.HoveredIdPreviousFrame;
}

void ImGui::KeepAliveID(ImGuiID id)
{
	ImGuiContext& g = *GImGui;
	if (g.ActiveId == id)
		g.ActiveIdIsAlive = true;
}

static inline bool IsWindowContentHoverable(ImGuiWindow* window, ImGuiHoveredFlags flags)
{
	// An active popup disable hovering on other windows (apart from its own children)
	// FIXME-OPT: This could be cached/stored within the window.
	ImGuiContext& g = *GImGui;
	if (g.NavWindow)
		if (ImGuiWindow* focused_root_window = g.NavWindow->RootWindow)
			if (focused_root_window->WasActive && focused_root_window != window->RootWindow)
			{
				// For the purpose of those flags we differentiate "standard popup" from "modal popup"
				// NB: The order of those two tests is important because Modal windows are also Popups.
				if (focused_root_window->Flags & ImGuiWindowFlags_Modal)
					return false;
				if ((focused_root_window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiHoveredFlags_AllowWhenBlockedByPopup))
					return false;
			}

	return true;
}

// Advance cursor given item size for layout.
void ImGui::ItemSize(const ImVec2& size, float text_offset_y)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return;

	// Always align ourselves on pixel boundaries
	const float line_height = ImMax(window->DC.CurrentLineHeight, size.y);
	const float text_base_offset = ImMax(window->DC.CurrentLineTextBaseOffset, text_offset_y);
	//if (g.IO.KeyAlt) window->DrawList->AddRect(window->DC.CursorPos, window->DC.CursorPos + ImVec2(size.x, line_height), IM_COL32(255,0,0,200)); // [DEBUG]
	window->DC.CursorPosPrevLine = ImVec2(window->DC.CursorPos.x + size.x, window->DC.CursorPos.y);
	window->DC.CursorPos = ImVec2((float)(int)(window->Pos.x + window->DC.IndentX + window->DC.ColumnsOffsetX), (float)(int)(window->DC.CursorPos.y + line_height + g.Style.ItemSpacing.y));
	window->DC.CursorMaxPos.x = ImMax(window->DC.CursorMaxPos.x, window->DC.CursorPosPrevLine.x);
	window->DC.CursorMaxPos.y = ImMax(window->DC.CursorMaxPos.y, window->DC.CursorPos.y - g.Style.ItemSpacing.y);
	//if (g.IO.KeyAlt) window->DrawList->AddCircle(window->DC.CursorMaxPos, 3.0f, IM_COL32(255,0,0,255), 4); // [DEBUG]

	window->DC.PrevLineHeight = line_height;
	window->DC.PrevLineTextBaseOffset = text_base_offset;
	window->DC.CurrentLineHeight = window->DC.CurrentLineTextBaseOffset = 0.0f;

	// Horizontal layout mode
	if (window->DC.LayoutType == ImGuiLayoutType_Horizontal)
		SameLine();
}

void ImGui::ItemSize(const ImRect& bb, float text_offset_y)
{
	ItemSize(bb.GetSize(), text_offset_y);
}

// Declare item bounding box for clipping and interaction.
// Note that the size can be different than the one provided to ItemSize(). Typically, widgets that spread over available surface
// declares their minimum size requirement to ItemSize() and then use a larger region for drawing/interaction, which is passed to ItemAdd().
bool ImGui::ItemAdd(const ImRect& bb, ImGuiID id)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	const bool is_clipped = IsClippedEx(bb, id, false);
	window->DC.LastItemId = id;
	window->DC.LastItemRect = bb;
	window->DC.LastItemRectHoveredRect = false;
	if (is_clipped)
		return false;
	//if (g.IO.KeyAlt) window->DrawList->AddRect(bb.Min, bb.Max, IM_COL32(255,255,0,120)); // [DEBUG]

	// We need to calculate this now to take account of the current clipping rectangle (as items like Selectable may change them)
	window->DC.LastItemRectHoveredRect = IsMouseHoveringRect(bb.Min, bb.Max);
	return true;
}

// This is roughly matching the behavior of internal-facing ItemHoverable()
// - we allow hovering to be true when ActiveId==window->MoveID, so that clicking on non-interactive items such as a Text() item still returns true with IsItemHovered()
// - this should work even for non-interactive items that have no ID, so we cannot use LastItemId
bool ImGui::IsItemHovered(ImGuiHoveredFlags flags)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	// Test for bounding box overlap, as updated as ItemAdd()
	if (!window->DC.LastItemRectHoveredRect)
		return false;
	IM_ASSERT((flags & (ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_ChildWindows)) == 0);   // Flags not supported by this function

																								 // Test if we are hovering the right window (our window could be behind another window)
																								 // [2017/10/16] Reverted commit 344d48be3 and testing RootWindow instead. I believe it is correct to NOT test for RootWindow but this leaves us unable to use IsItemHovered() after EndChild() itself.
																								 // Until a solution is found I believe reverting to the test from 2017/09/27 is safe since this was the test that has been running for a long while.
																								 //if (g.HoveredWindow != window)
																								 //    return false;
	if (g.HoveredRootWindow != window->RootWindow && !(flags & ImGuiHoveredFlags_AllowWhenOverlapped))
		return false;

	// Test if another item is active (e.g. being dragged)
	if (!(flags & ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		if (g.ActiveId != 0 && g.ActiveId != window->DC.LastItemId && !g.ActiveIdAllowOverlap && g.ActiveId != window->MoveId)
			return false;

	// Test if interactions on this window are blocked by an active popup or modal 
	if (!IsWindowContentHoverable(window, flags))
		return false;

	// Test if the item is disabled
	if (window->DC.ItemFlags & ImGuiItemFlags_Disabled)
		return false;

	// Special handling for the 1st item after Begin() which represent the title bar. When the window is collapsed (SkipItems==true) that last item will never be overwritten so we need to detect tht case.
	if (window->DC.LastItemId == window->MoveId && window->WriteAccessed)
		return false;
	return true;
}

// Internal facing ItemHoverable() used when submitting widgets. Differs slightly from IsItemHovered().
bool ImGui::ItemHoverable(const ImRect& bb, ImGuiID id)
{
	ImGuiContext& g = *GImGui;
	if (g.HoveredId != 0 && g.HoveredId != id && !g.HoveredIdAllowOverlap)
		return false;

	ImGuiWindow* window = g.CurrentWindow;
	if (g.HoveredWindow != window)
		return false;
	if (g.ActiveId != 0 && g.ActiveId != id && !g.ActiveIdAllowOverlap)
		return false;
	if (!IsMouseHoveringRect(bb.Min, bb.Max))
		return false;
	if (!IsWindowContentHoverable(window, ImGuiHoveredFlags_Default))
		return false;
	if (window->DC.ItemFlags & ImGuiItemFlags_Disabled)
		return false;

	SetHoveredID(id);
	return true;
}

bool ImGui::IsClippedEx(const ImRect& bb, ImGuiID id, bool clip_even_when_logged)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	if (!bb.Overlaps(window->ClipRect))
		if (id == 0 || id != g.ActiveId)
			if (clip_even_when_logged || !g.LogEnabled)
				return true;
	return false;
}


bool ImGui::IsClippedEx2(const ImRect& bb, const ImGuiID* id, bool clip_even_when_logged)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindowRead();

	if (!bb.Overlaps(window->ClipRect))
		if (!id || *id != GImGui->ActiveId)
			if (clip_even_when_logged || !g.LogEnabled)
				return true;
	return false;
}

static inline bool IsWindowContentHoverable2(ImGuiWindow* window)
{
	// An active popup disable hovering on other windows (apart from its own children)
	ImGuiContext& g = *GImGui;
	if (ImGuiWindow* focused_window = g.NavWindow->RootWindow)
		if (ImGuiWindow* focused_root_window = focused_window->RootWindow)
			if ((focused_root_window->Flags & ImGuiWindowFlags_Popup) != 0 && focused_root_window->WasActive && focused_root_window != window->RootWindow)
				return false;

	return true;
}


bool ImGui::IsHovered(const ImRect& bb, ImGuiID id, bool flatten_childs)
{
	ImGuiContext& g = *GImGui;
	if (g.HoveredId == 0 || g.HoveredId == id || g.HoveredIdAllowOverlap)
	{
		ImGuiWindow* window = GetCurrentWindowRead();
		if (g.HoveredWindow == window || (flatten_childs && g.HoveredRootWindow == window->RootWindow))
			if ((g.ActiveId == 0 || g.ActiveId == id || g.ActiveIdAllowOverlap) && IsMouseHoveringRect(bb.Min, bb.Max))
				if (IsWindowContentHoverable2(g.HoveredRootWindow))
					return true;
	}
	return false;
}


bool ImGui::ItemAdd2(const ImRect& bb, const ImGuiID* id)
{
	ImGuiWindow* window = GetCurrentWindow();
	window->DC.LastItemId = id ? *id : 0;
	window->DC.LastItemRect = bb;
	window->DC.LastItemHoveredAndUsable = window->DC.LastItemHoveredRect = false;
	if (IsClippedEx2(bb, id, false))
		return false;

	// This is a sensible default, but widgets are free to override it after calling ItemAdd()
	ImGuiContext& g = *GImGui;
	if (IsMouseHoveringRect(bb.Min, bb.Max))
	{
		// Matching the behavior of IsHovered() but allow if ActiveId==window->MoveID (we clicked on the window background)
		// So that clicking on items with no active id such as Text() still returns true with IsItemHovered()
		window->DC.LastItemHoveredRect = true;
		if (g.HoveredRootWindow == window->RootWindow)
			if (g.ActiveId == 0 || (id && g.ActiveId == *id) || g.ActiveIdAllowOverlap || (g.ActiveId == window->MoveId))
				if (IsWindowContentHoverable2(window))
					window->DC.LastItemHoveredAndUsable = true;
	}

	return true;
}



bool ImGui::FocusableItemRegister(ImGuiWindow* window, ImGuiID id, bool tab_stop)
{
	ImGuiContext& g = *GImGui;

	const bool allow_keyboard_focus = (window->DC.ItemFlags & (ImGuiItemFlags_AllowKeyboardFocus | ImGuiItemFlags_Disabled)) == ImGuiItemFlags_AllowKeyboardFocus;
	window->FocusIdxAllCounter++;
	if (allow_keyboard_focus)
		window->FocusIdxTabCounter++;

	// Process keyboard input at this point: TAB/Shift-TAB to tab out of the currently focused item.
	// Note that we can always TAB out of a widget that doesn't allow tabbing in.
	if (tab_stop && (g.ActiveId == id) && window->FocusIdxAllRequestNext == INT_MAX && window->FocusIdxTabRequestNext == INT_MAX && !g.IO.KeyCtrl && IsKeyPressedMap(ImGuiKey_Tab))
		window->FocusIdxTabRequestNext = window->FocusIdxTabCounter + (g.IO.KeyShift ? (allow_keyboard_focus ? -1 : 0) : +1); // Modulo on index will be applied at the end of frame once we've got the total counter of items.

	if (window->FocusIdxAllCounter == window->FocusIdxAllRequestCurrent)
		return true;

	if (allow_keyboard_focus)
		if (window->FocusIdxTabCounter == window->FocusIdxTabRequestCurrent)
			return true;

	return false;
}

void ImGui::FocusableItemUnregister(ImGuiWindow* window)
{
	window->FocusIdxAllCounter--;
	window->FocusIdxTabCounter--;
}

ImVec2 ImGui::CalcItemSize(ImVec2 size, float default_x, float default_y)
{
	ImGuiContext& g = *GImGui;
	ImVec2 content_max;
	if (size.x < 0.0f || size.y < 0.0f)
		content_max = g.CurrentWindow->Pos + GetContentRegionMax();
	if (size.x <= 0.0f)
		size.x = (size.x == 0.0f) ? default_x : ImMax(content_max.x - g.CurrentWindow->DC.CursorPos.x, 4.0f) + size.x;
	if (size.y <= 0.0f)
		size.y = (size.y == 0.0f) ? default_y : ImMax(content_max.y - g.CurrentWindow->DC.CursorPos.y, 4.0f) + size.y;
	return size;
}

float ImGui::CalcWrapWidthForPos(const ImVec2& pos, float wrap_pos_x)
{
	if (wrap_pos_x < 0.0f)
		return 0.0f;

	ImGuiWindow* window = GetCurrentWindowRead();
	if (wrap_pos_x == 0.0f)
		wrap_pos_x = GetContentRegionMax().x + window->Pos.x;
	else if (wrap_pos_x > 0.0f)
		wrap_pos_x += window->Pos.x - window->Scroll.x; // wrap_pos_x is provided is window local space

	return ImMax(wrap_pos_x - pos.x, 1.0f);
}

//-----------------------------------------------------------------------------

void* ImGui::MemAlloc(size_t sz)
{
	GImGui->IO.MetricsAllocs++;
	return GImGui->IO.MemAllocFn(sz);
}

void ImGui::MemFree(void* ptr)
{
	if (ptr) GImGui->IO.MetricsAllocs--;
	return GImGui->IO.MemFreeFn(ptr);
}

const char* ImGui::GetClipboardText()
{
	return GImGui->IO.GetClipboardTextFn ? GImGui->IO.GetClipboardTextFn(GImGui->IO.ClipboardUserData) : "";
}

void ImGui::SetClipboardText(const char* text)
{
	if (GImGui->IO.SetClipboardTextFn)
		GImGui->IO.SetClipboardTextFn(GImGui->IO.ClipboardUserData, text);
}

const char* ImGui::GetVersion()
{
	return IMGUI_VERSION;
}

// Internal state access - if you want to share ImGui state between modules (e.g. DLL) or allocate it yourself
// Note that we still point to some static data and members (such as GFontAtlas), so the state instance you end up using will point to the static data within its module
ImGuiContext* ImGui::GetCurrentContext()
{
	return GImGui;
}

void ImGui::SetCurrentContext(ImGuiContext* ctx)
{
#ifdef IMGUI_SET_CURRENT_CONTEXT_FUNC
	IMGUI_SET_CURRENT_CONTEXT_FUNC(ctx); // For custom thread-based hackery you may want to have control over this.
#else
	GImGui = ctx;
#endif
}

ImGuiContext* ImGui::CreateContext(void* (*malloc_fn)(size_t), void(*free_fn)(void*))
{
	if (!malloc_fn) malloc_fn = malloc;
	ImGuiContext* ctx = (ImGuiContext*)malloc_fn(sizeof(ImGuiContext));
	IM_PLACEMENT_NEW(ctx) ImGuiContext();
	ctx->IO.MemAllocFn = malloc_fn;
	ctx->IO.MemFreeFn = free_fn ? free_fn : free;
	return ctx;
}

void ImGui::DestroyContext(ImGuiContext* ctx)
{
	void(*free_fn)(void*) = ctx->IO.MemFreeFn;
	ctx->~ImGuiContext();
	free_fn(ctx);
	if (GImGui == ctx)
		SetCurrentContext(NULL);
}

ImGuiIO& ImGui::GetIO()
{
	return GImGui->IO;
}

ImGuiStyle& ImGui::GetStyle()
{
	return GImGui->Style;
}

// Same value as passed to your RenderDrawListsFn() function. valid after Render() and until the next call to NewFrame()
ImDrawData* ImGui::GetDrawData()
{
	return GImGui->DrawData.Valid ? &GImGui->DrawData : NULL;
}

float ImGui::GetTime()
{
	return GImGui->Time;
}

int ImGui::GetFrameCount()
{
	return GImGui->FrameCount;
}

ImDrawList* ImGui::GetOverlayDrawList()
{
	return &GImGui->OverlayDrawList;
}

ImDrawListSharedData* ImGui::GetDrawListSharedData()
{
	return &GImGui->DrawListSharedData;
}

void ImGui::NewFrame()
{
	ImGuiContext& g = *GImGui;

	// Check user data
	// (We pass an error message in the assert expression as a trick to get it visible to programmers who are not using a debugger, as most assert handlers display their argument)
	IM_ASSERT(g.IO.DeltaTime >= 0.0f                                    && "Need a positive DeltaTime (zero is tolerated but will cause some timing issues)");
	IM_ASSERT(g.IO.DisplaySize.x >= 0.0f && g.IO.DisplaySize.y >= 0.0f  && "Invalid DisplaySize value");
	IM_ASSERT(g.IO.Fonts->Fonts.Size > 0 && "Font Atlas not built. Did you call io.Fonts->GetTexDataAsRGBA32() / GetTexDataAsAlpha8() ?");
	IM_ASSERT(g.IO.Fonts->Fonts[0]->IsLoaded() && "Font Atlas not built. Did you call io.Fonts->GetTexDataAsRGBA32() / GetTexDataAsAlpha8() ?");
	IM_ASSERT(g.Style.CurveTessellationTol > 0.0f                       && "Invalid style setting");
	IM_ASSERT(g.Style.Alpha >= 0.0f && g.Style.Alpha <= 1.0f            && "Invalid style setting. Alpha cannot be negative (allows us to avoid a few clamps in color computations)");
	IM_ASSERT((g.FrameCount == 0 || g.FrameCountEnded == g.FrameCount) && "Forgot to call Render() or EndFrame() at the end of the previous frame?");
	for (int n = 0; n < ImGuiKey_COUNT; n++)
		IM_ASSERT(g.IO.KeyMap[n] >= -1 && g.IO.KeyMap[n] < IM_ARRAYSIZE(g.IO.KeysDown) && "io.KeyMap[] contains an out of bound value (need to be 0..512, or -1 for unmapped key)");

	// Initialize on first frame
	if (!g.Initialized)
		Initialize();

	g.Time += g.IO.DeltaTime;
	g.FrameCount += 1;
	g.TooltipOverrideCount = 0;
	g.WindowsActiveCount = 0;

	SetCurrentFont(GetDefaultFont());
	IM_ASSERT(g.Font->IsLoaded());
	g.DrawListSharedData.ClipRectFullscreen = ImVec4(0.0f, 0.0f, g.IO.DisplaySize.x, g.IO.DisplaySize.y);
	g.DrawListSharedData.CurveTessellationTol = g.Style.CurveTessellationTol;

	g.OverlayDrawList.Clear();
	g.OverlayDrawList.PushTextureID(g.IO.Fonts->TexID);
	g.OverlayDrawList.PushClipRectFullScreen();
	g.OverlayDrawList.Flags = (g.Style.AntiAliasedLines ? ImDrawListFlags_AntiAliasedLines : 0) | (g.Style.AntiAliasedFill ? ImDrawListFlags_AntiAliasedFill : 0);

	// Mark rendering data as invalid to prevent user who may have a handle on it to use it
	g.DrawData.Clear();

	// Clear reference to active widget if the widget isn't alive anymore
	if (!g.HoveredIdPreviousFrame)
		g.HoveredIdTimer = 0.0f;
	g.HoveredIdPreviousFrame = g.HoveredId;
	g.HoveredId = 0;
	g.HoveredIdAllowOverlap = false;
	if (!g.ActiveIdIsAlive && g.ActiveIdPreviousFrame == g.ActiveId && g.ActiveId != 0)
		ClearActiveID();
	if (g.ActiveId)
		g.ActiveIdTimer += g.IO.DeltaTime;
	g.ActiveIdPreviousFrame = g.ActiveId;
	g.ActiveIdIsAlive = false;
	g.ActiveIdIsJustActivated = false;
	if (g.ScalarAsInputTextId && g.ActiveId != g.ScalarAsInputTextId)
		g.ScalarAsInputTextId = 0;

	// Elapse drag & drop payload
	if (g.DragDropActive && g.DragDropPayload.DataFrameCount + 1 < g.FrameCount)
	{
		ClearDragDrop();
		g.DragDropPayloadBufHeap.clear();
		memset(&g.DragDropPayloadBufLocal, 0, sizeof(g.DragDropPayloadBufLocal));
	}
	g.DragDropAcceptIdPrev = g.DragDropAcceptIdCurr;
	g.DragDropAcceptIdCurr = 0;
	g.DragDropAcceptIdCurrRectSurface = FLT_MAX;

	// Update keyboard input state
	memcpy(g.IO.KeysDownDurationPrev, g.IO.KeysDownDuration, sizeof(g.IO.KeysDownDuration));
	for (int i = 0; i < IM_ARRAYSIZE(g.IO.KeysDown); i++)
		g.IO.KeysDownDuration[i] = g.IO.KeysDown[i] ? (g.IO.KeysDownDuration[i] < 0.0f ? 0.0f : g.IO.KeysDownDuration[i] + g.IO.DeltaTime) : -1.0f;

	// Update mouse input state
	// If mouse just appeared or disappeared (usually denoted by -FLT_MAX component, but in reality we test for -256000.0f) we cancel out movement in MouseDelta
	if (IsMousePosValid(&g.IO.MousePos) && IsMousePosValid(&g.IO.MousePosPrev))
		g.IO.MouseDelta = g.IO.MousePos - g.IO.MousePosPrev;
	else
		g.IO.MouseDelta = ImVec2(0.0f, 0.0f);
	g.IO.MousePosPrev = g.IO.MousePos;
	for (int i = 0; i < IM_ARRAYSIZE(g.IO.MouseDown); i++)
	{
		g.IO.MouseClicked[i] = g.IO.MouseDown[i] && g.IO.MouseDownDuration[i] < 0.0f;
		g.IO.MouseReleased[i] = !g.IO.MouseDown[i] && g.IO.MouseDownDuration[i] >= 0.0f;
		g.IO.MouseDownDurationPrev[i] = g.IO.MouseDownDuration[i];
		g.IO.MouseDownDuration[i] = g.IO.MouseDown[i] ? (g.IO.MouseDownDuration[i] < 0.0f ? 0.0f : g.IO.MouseDownDuration[i] + g.IO.DeltaTime) : -1.0f;
		g.IO.MouseDoubleClicked[i] = false;
		if (g.IO.MouseClicked[i])
		{
			if (g.Time - g.IO.MouseClickedTime[i] < g.IO.MouseDoubleClickTime)
			{
				if (ImLengthSqr(g.IO.MousePos - g.IO.MouseClickedPos[i]) < g.IO.MouseDoubleClickMaxDist * g.IO.MouseDoubleClickMaxDist)
					g.IO.MouseDoubleClicked[i] = true;
				g.IO.MouseClickedTime[i] = -FLT_MAX;    // so the third click isn't turned into a double-click
			}
			else
			{
				g.IO.MouseClickedTime[i] = g.Time;
			}
			g.IO.MouseClickedPos[i] = g.IO.MousePos;
			g.IO.MouseDragMaxDistanceAbs[i] = ImVec2(0.0f, 0.0f);
			g.IO.MouseDragMaxDistanceSqr[i] = 0.0f;
		}
		else if (g.IO.MouseDown[i])
		{
			ImVec2 mouse_delta = g.IO.MousePos - g.IO.MouseClickedPos[i];
			g.IO.MouseDragMaxDistanceAbs[i].x = ImMax(g.IO.MouseDragMaxDistanceAbs[i].x, mouse_delta.x < 0.0f ? -mouse_delta.x : mouse_delta.x);
			g.IO.MouseDragMaxDistanceAbs[i].y = ImMax(g.IO.MouseDragMaxDistanceAbs[i].y, mouse_delta.y < 0.0f ? -mouse_delta.y : mouse_delta.y);
			g.IO.MouseDragMaxDistanceSqr[i] = ImMax(g.IO.MouseDragMaxDistanceSqr[i], ImLengthSqr(mouse_delta));
		}
	}

	// Calculate frame-rate for the user, as a purely luxurious feature
	g.FramerateSecPerFrameAccum += g.IO.DeltaTime - g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx];
	g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx] = g.IO.DeltaTime;
	g.FramerateSecPerFrameIdx = (g.FramerateSecPerFrameIdx + 1) % IM_ARRAYSIZE(g.FramerateSecPerFrame);
	g.IO.Framerate = 1.0f / (g.FramerateSecPerFrameAccum / (float)IM_ARRAYSIZE(g.FramerateSecPerFrame));

	// Handle user moving window with mouse (at the beginning of the frame to avoid input lag or sheering).
	if (g.MovingWindow && g.MovingWindow->MoveId == g.ActiveId)
	{
		KeepAliveID(g.ActiveId);
		IM_ASSERT(g.MovingWindow && g.MovingWindow->RootWindow);
		if (g.IO.MouseDown[0])
		{
			// MovingWindow = window we clicked on, could be a child window. We track it to preserve Focus and so that ActiveIdWindow == MovingWindow and ActiveId == MovingWindow->MoveId for consistency.
			ImGuiWindow* actually_moving_window = g.MovingWindow->RootWindow;
			ImVec2 pos = g.IO.MousePos - g.ActiveIdClickOffset;
			if (actually_moving_window->PosFloat.x != pos.x || actually_moving_window->PosFloat.y != pos.y)
			{
				MarkIniSettingsDirty(actually_moving_window);
				actually_moving_window->PosFloat = pos;
			}
			FocusWindow(g.MovingWindow);
		}
		else
		{
			ClearActiveID();
			g.MovingWindow = NULL;
		}
	}
	else
	{
		// When clicking/dragging from a window that has the _NoMove flag, we still set the ActiveId in order to prevent hovering others.
		if (g.ActiveIdWindow && g.ActiveIdWindow->MoveId == g.ActiveId)
		{
			KeepAliveID(g.ActiveId);
			if (!g.IO.MouseDown[0])
				ClearActiveID();
		}
		g.MovingWindow = NULL;
	}

	// Delay saving settings so we don't spam disk too much
	if (g.SettingsDirtyTimer > 0.0f)
	{
		g.SettingsDirtyTimer -= g.IO.DeltaTime;
		if (g.SettingsDirtyTimer <= 0.0f)
			SaveIniSettingsToDisk(g.IO.IniFilename);
	}

	// Find the window we are hovering
	// - Child windows can extend beyond the limit of their parent so we need to derive HoveredRootWindow from HoveredWindow.
	// - When moving a window we can skip the search, which also conveniently bypasses the fact that window->WindowRectClipped is lagging as this point.
	// - We also support the moved window toggling the NoInputs flag after moving has started in order to be able to detect windows below it, which is useful for e.g. docking mechanisms.
	g.HoveredWindow = (g.MovingWindow && !(g.MovingWindow->Flags & ImGuiWindowFlags_NoInputs)) ? g.MovingWindow : FindHoveredWindow(g.IO.MousePos);
	g.HoveredRootWindow = g.HoveredWindow ? g.HoveredWindow->RootWindow : NULL;

	ImGuiWindow* modal_window = GetFrontMostModalRootWindow();
	if (modal_window != NULL)
	{
		g.ModalWindowDarkeningRatio = ImMin(g.ModalWindowDarkeningRatio + g.IO.DeltaTime * 6.0f, 1.0f);
		if (g.HoveredRootWindow && !IsWindowChildOf(g.HoveredRootWindow, modal_window))
			g.HoveredRootWindow = g.HoveredWindow = NULL;
	}
	else
	{
		g.ModalWindowDarkeningRatio = 0.0f;
	}

	// Update the WantCaptureMouse/WantCAptureKeyboard flags, so user can capture/discard the inputs away from the rest of their application.
	// When clicking outside of a window we assume the click is owned by the application and won't request capture. We need to track click ownership.
	int mouse_earliest_button_down = -1;
	bool mouse_any_down = false;
	for (int i = 0; i < IM_ARRAYSIZE(g.IO.MouseDown); i++)
	{
		if (g.IO.MouseClicked[i])
			g.IO.MouseDownOwned[i] = (g.HoveredWindow != NULL) || (!g.OpenPopupStack.empty());
		mouse_any_down |= g.IO.MouseDown[i];
		if (g.IO.MouseDown[i])
			if (mouse_earliest_button_down == -1 || g.IO.MouseClickedTime[i] < g.IO.MouseClickedTime[mouse_earliest_button_down])
				mouse_earliest_button_down = i;
	}
	bool mouse_avail_to_imgui = (mouse_earliest_button_down == -1) || g.IO.MouseDownOwned[mouse_earliest_button_down];
	if (g.WantCaptureMouseNextFrame != -1)
		g.IO.WantCaptureMouse = (g.WantCaptureMouseNextFrame != 0);
	else
		g.IO.WantCaptureMouse = (mouse_avail_to_imgui && (g.HoveredWindow != NULL || mouse_any_down)) || (!g.OpenPopupStack.empty());
	if (g.WantCaptureKeyboardNextFrame != -1)
		g.IO.WantCaptureKeyboard = (g.WantCaptureKeyboardNextFrame != 0);
	else
		g.IO.WantCaptureKeyboard = (g.ActiveId != 0) || (modal_window != NULL);
	g.IO.WantTextInput = (g.WantTextInputNextFrame != -1) ? (g.WantTextInputNextFrame != 0) : 0;
	g.MouseCursor = ImGuiMouseCursor_Arrow;
	g.WantCaptureMouseNextFrame = g.WantCaptureKeyboardNextFrame = g.WantTextInputNextFrame = -1;
	g.OsImePosRequest = ImVec2(1.0f, 1.0f); // OS Input Method Editor showing on top-left of our window by default

											// If mouse was first clicked outside of ImGui bounds we also cancel out hovering.
											// FIXME: For patterns of drag and drop across OS windows, we may need to rework/remove this test (first committed 311c0ca9 on 2015/02)
	bool mouse_dragging_extern_payload = g.DragDropActive && (g.DragDropSourceFlags & ImGuiDragDropFlags_SourceExtern) != 0;
	if (!mouse_avail_to_imgui && !mouse_dragging_extern_payload)
		g.HoveredWindow = g.HoveredRootWindow = NULL;

	// Mouse wheel scrolling, scale
	if (g.HoveredWindow && !g.HoveredWindow->Collapsed && (g.IO.MouseWheel != 0.0f || g.IO.MouseWheelH != 0.0f))
	{
		// If a child window has the ImGuiWindowFlags_NoScrollWithMouse flag, we give a chance to scroll its parent (unless either ImGuiWindowFlags_NoInputs or ImGuiWindowFlags_NoScrollbar are also set).
		ImGuiWindow* window = g.HoveredWindow;
		ImGuiWindow* scroll_window = window;
		while ((scroll_window->Flags & ImGuiWindowFlags_ChildWindow) && (scroll_window->Flags & ImGuiWindowFlags_NoScrollWithMouse) && !(scroll_window->Flags & ImGuiWindowFlags_NoScrollbar) && !(scroll_window->Flags & ImGuiWindowFlags_NoInputs) && scroll_window->ParentWindow)
			scroll_window = scroll_window->ParentWindow;
		const bool scroll_allowed = !(scroll_window->Flags & ImGuiWindowFlags_NoScrollWithMouse) && !(scroll_window->Flags & ImGuiWindowFlags_NoInputs);

		if (g.IO.MouseWheel != 0.0f)
		{
			if (g.IO.KeyCtrl && g.IO.FontAllowUserScaling)
			{
				// Zoom / Scale window
				const float new_font_scale = ImClamp(window->FontWindowScale + g.IO.MouseWheel * 0.10f, 0.50f, 2.50f);
				const float scale = new_font_scale / window->FontWindowScale;
				window->FontWindowScale = new_font_scale;

				const ImVec2 offset = window->Size * (1.0f - scale) * (g.IO.MousePos - window->Pos) / window->Size;
				window->Pos += offset;
				window->PosFloat += offset;
				window->Size *= scale;
				window->SizeFull *= scale;
			}
			else if (!g.IO.KeyCtrl && scroll_allowed)
			{
				// Mouse wheel vertical scrolling
				float scroll_amount = 5 * scroll_window->CalcFontSize();
				scroll_amount = (float)(int)ImMin(scroll_amount, (scroll_window->ContentsRegionRect.GetHeight() + scroll_window->WindowPadding.y * 2.0f) * 0.67f);
				SetWindowScrollY(scroll_window, scroll_window->Scroll.y - g.IO.MouseWheel * scroll_amount);
			}
		}
		if (g.IO.MouseWheelH != 0.0f && scroll_allowed)
		{
			// Mouse wheel horizontal scrolling (for hardware that supports it)
			float scroll_amount = scroll_window->CalcFontSize();
			if (!g.IO.KeyCtrl && !(window->Flags & ImGuiWindowFlags_NoScrollWithMouse))
				SetWindowScrollX(window, window->Scroll.x - g.IO.MouseWheelH * scroll_amount);
		}
	}

	// Pressing TAB activate widget focus
	if (g.ActiveId == 0 && g.NavWindow != NULL && g.NavWindow->Active && IsKeyPressedMap(ImGuiKey_Tab, false))
		g.NavWindow->FocusIdxTabRequestNext = 0;

	// Mark all windows as not visible
	for (int i = 0; i != g.Windows.Size; i++)
	{
		ImGuiWindow* window = g.Windows[i];
		window->WasActive = window->Active;
		window->Active = false;
		window->WriteAccessed = false;
	}

	// Closing the focused window restore focus to the first active root window in descending z-order
	if (g.NavWindow && !g.NavWindow->WasActive)
		FocusFrontMostActiveWindow(NULL);

	// No window should be open at the beginning of the frame.
	// But in order to allow the user to call NewFrame() multiple times without calling Render(), we are doing an explicit clear.
	g.CurrentWindowStack.resize(0);
	g.CurrentPopupStack.resize(0);
	ClosePopupsOverWindow(g.NavWindow);

	// Create implicit window - we will only render it if the user has added something to it.
	// We don't use "Debug" to avoid colliding with user trying to create a "Debug" window with custom flags.
	SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	Begin("Debug##Default");
}

static void* SettingsHandlerWindow_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name)
{
	ImGuiWindowSettings* settings = ImGui::FindWindowSettings(ImHash(name, 0));
	if (!settings)
		settings = AddWindowSettings(name);
	return (void*)settings;
}

static void SettingsHandlerWindow_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line)
{
	ImGuiWindowSettings* settings = (ImGuiWindowSettings*)entry;
	float x, y;
	int i;
	if (sscanf(line, "Pos=%f,%f", &x, &y) == 2)         settings->Pos = ImVec2(x, y);
	else if (sscanf(line, "Size=%f,%f", &x, &y) == 2)   settings->Size = ImMax(ImVec2(x, y), GImGui->Style.WindowMinSize);
	else if (sscanf(line, "Collapsed=%d", &i) == 1)     settings->Collapsed = (i != 0);
}

static void SettingsHandlerWindow_WriteAll(ImGuiContext* imgui_ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
{
	// Gather data from windows that were active during this session
	ImGuiContext& g = *imgui_ctx;
	for (int i = 0; i != g.Windows.Size; i++)
	{
		ImGuiWindow* window = g.Windows[i];
		if (window->Flags & ImGuiWindowFlags_NoSavedSettings)
			continue;
		ImGuiWindowSettings* settings = ImGui::FindWindowSettings(window->ID);
		if (!settings)
			settings = AddWindowSettings(window->Name);
		settings->Pos = window->Pos;
		settings->Size = window->SizeFull;
		settings->Collapsed = window->Collapsed;
	}

	// Write a buffer
	// If a window wasn't opened in this session we preserve its settings
	buf->reserve(buf->size() + g.SettingsWindows.Size * 96); // ballpark reserve
	for (int i = 0; i != g.SettingsWindows.Size; i++)
	{
		const ImGuiWindowSettings* settings = &g.SettingsWindows[i];
		if (settings->Pos.x == FLT_MAX)
			continue;
		const char* name = settings->Name;
		if (const char* p = strstr(name, "###"))  // Skip to the "###" marker if any. We don't skip past to match the behavior of GetID()
			name = p;
		buf->appendf("[%s][%s]\n", handler->TypeName, name);
		buf->appendf("Pos=%d,%d\n", (int)settings->Pos.x, (int)settings->Pos.y);
		buf->appendf("Size=%d,%d\n", (int)settings->Size.x, (int)settings->Size.y);
		buf->appendf("Collapsed=%d\n", settings->Collapsed);
		buf->appendf("\n");
	}
}

void ImGui::Initialize()
{
	ImGuiContext& g = *GImGui;
	g.LogClipboard = IM_NEW(ImGuiTextBuffer)();

	// Add .ini handle for ImGuiWindow type
	ImGuiSettingsHandler ini_handler;
	ini_handler.TypeName = "Window";
	ini_handler.TypeHash = ImHash("Window", 0, 0);
	ini_handler.ReadOpenFn = SettingsHandlerWindow_ReadOpen;
	ini_handler.ReadLineFn = SettingsHandlerWindow_ReadLine;
	ini_handler.WriteAllFn = SettingsHandlerWindow_WriteAll;
	g.SettingsHandlers.push_front(ini_handler);

	// Load .ini file
	IM_ASSERT(g.SettingsWindows.empty());
	LoadIniSettingsFromDisk(g.IO.IniFilename);
	g.Initialized = true;
}

// This function is merely here to free heap allocations.
void ImGui::Shutdown()
{
	ImGuiContext& g = *GImGui;

	// The fonts atlas can be used prior to calling NewFrame(), so we clear it even if g.Initialized is FALSE (which would happen if we never called NewFrame)
	if (g.IO.Fonts) // Testing for NULL to allow user to NULLify in case of running Shutdown() on multiple contexts. Bit hacky.
		g.IO.Fonts->Clear();

	// Cleanup of other data are conditional on actually having initialize ImGui.
	if (!g.Initialized)
		return;

	SaveIniSettingsToDisk(g.IO.IniFilename);

	for (int i = 0; i < g.Windows.Size; i++)
		IM_DELETE(g.Windows[i]);
	g.Windows.clear();
	g.WindowsSortBuffer.clear();
	g.CurrentWindow = NULL;
	g.CurrentWindowStack.clear();
	g.WindowsById.Clear();
	g.NavWindow = NULL;
	g.HoveredWindow = NULL;
	g.HoveredRootWindow = NULL;
	g.ActiveIdWindow = NULL;
	g.MovingWindow = NULL;
	for (int i = 0; i < g.SettingsWindows.Size; i++)
		IM_DELETE(g.SettingsWindows[i].Name);
	g.ColorModifiers.clear();
	g.StyleModifiers.clear();
	g.FontStack.clear();
	g.OpenPopupStack.clear();
	g.CurrentPopupStack.clear();
	g.DrawDataBuilder.ClearFreeMemory();
	g.OverlayDrawList.ClearFreeMemory();
	g.PrivateClipboard.clear();
	g.InputTextState.Text.clear();
	g.InputTextState.InitialText.clear();
	g.InputTextState.TempTextBuffer.clear();

	g.SettingsWindows.clear();
	g.SettingsHandlers.clear();

	if (g.LogFile && g.LogFile != stdout)
	{
		fclose(g.LogFile);
		g.LogFile = NULL;
	}
	if (g.LogClipboard)
		IM_DELETE(g.LogClipboard);

	g.Initialized = false;
}

ImGuiWindowSettings* ImGui::FindWindowSettings(ImGuiID id)
{
	ImGuiContext& g = *GImGui;
	for (int i = 0; i != g.SettingsWindows.Size; i++)
		if (g.SettingsWindows[i].Id == id)
			return &g.SettingsWindows[i];
	return NULL;
}

static ImGuiWindowSettings* AddWindowSettings(const char* name)
{
	ImGuiContext& g = *GImGui;
	g.SettingsWindows.push_back(ImGuiWindowSettings());
	ImGuiWindowSettings* settings = &g.SettingsWindows.back();
	settings->Name = ImStrdup(name);
	settings->Id = ImHash(name, 0);
	return settings;
}

static void LoadIniSettingsFromDisk(const char* ini_filename)
{
	if (!ini_filename)
		return;
	char* file_data = (char*)ImFileLoadToMemory(ini_filename, "rb", NULL, +1);
	if (!file_data)
		return;
	LoadIniSettingsFromMemory(file_data);
	ImGui::MemFree(file_data);
}

ImGuiSettingsHandler* ImGui::FindSettingsHandler(const char* type_name)
{
	ImGuiContext& g = *GImGui;
	const ImGuiID type_hash = ImHash(type_name, 0, 0);
	for (int handler_n = 0; handler_n < g.SettingsHandlers.Size; handler_n++)
		if (g.SettingsHandlers[handler_n].TypeHash == type_hash)
			return &g.SettingsHandlers[handler_n];
	return NULL;
}

// Zero-tolerance, no error reporting, cheap .ini parsing
static void LoadIniSettingsFromMemory(const char* buf_readonly)
{
	// For convenience and to make the code simpler, we'll write zero terminators inside the buffer. So let's create a writable copy.
	char* buf = ImStrdup(buf_readonly);
	char* buf_end = buf + strlen(buf);

	ImGuiContext& g = *GImGui;
	void* entry_data = NULL;
	ImGuiSettingsHandler* entry_handler = NULL;

	char* line_end = NULL;
	for (char* line = buf; line < buf_end; line = line_end + 1)
	{
		// Skip new lines markers, then find end of the line
		while (*line == '\n' || *line == '\r')
			line++;
		line_end = line;
		while (line_end < buf_end && *line_end != '\n' && *line_end != '\r')
			line_end++;
		line_end[0] = 0;

		if (line[0] == '[' && line_end > line && line_end[-1] == ']')
		{
			// Parse "[Type][Name]". Note that 'Name' can itself contains [] characters, which is acceptable with the current format and parsing code.
			line_end[-1] = 0;
			const char* name_end = line_end - 1;
			const char* type_start = line + 1;
			char* type_end = ImStrchrRange(type_start, name_end, ']');
			const char* name_start = type_end ? ImStrchrRange(type_end + 1, name_end, '[') : NULL;
			if (!type_end || !name_start)
			{
				name_start = type_start; // Import legacy entries that have no type
				type_start = "Window";
			}
			else
			{
				*type_end = 0; // Overwrite first ']' 
				name_start++;  // Skip second '['
			}
			entry_handler = ImGui::FindSettingsHandler(type_start);
			entry_data = entry_handler ? entry_handler->ReadOpenFn(&g, entry_handler, name_start) : NULL;
		}
		else if (entry_handler != NULL && entry_data != NULL)
		{
			// Let type handler parse the line
			entry_handler->ReadLineFn(&g, entry_handler, entry_data, line);
		}
	}
	ImGui::MemFree(buf);
}

static void SaveIniSettingsToDisk(const char* ini_filename)
{
	ImGuiContext& g = *GImGui;
	g.SettingsDirtyTimer = 0.0f;
	if (!ini_filename)
		return;

	ImVector<char> buf;
	SaveIniSettingsToMemory(buf);

	FILE* f = ImFileOpen(ini_filename, "wt");
	if (!f)
		return;
	fwrite(buf.Data, sizeof(char), (size_t)buf.Size, f);
	fclose(f);
}

static void SaveIniSettingsToMemory(ImVector<char>& out_buf)
{
	ImGuiContext& g = *GImGui;
	g.SettingsDirtyTimer = 0.0f;

	ImGuiTextBuffer buf;
	for (int handler_n = 0; handler_n < g.SettingsHandlers.Size; handler_n++)
	{
		ImGuiSettingsHandler* handler = &g.SettingsHandlers[handler_n];
		handler->WriteAllFn(&g, handler, &buf);
	}

	buf.Buf.pop_back(); // Remove extra zero-terminator used by ImGuiTextBuffer
	out_buf.swap(buf.Buf);
}

void ImGui::MarkIniSettingsDirty()
{
	ImGuiContext& g = *GImGui;
	if (g.SettingsDirtyTimer <= 0.0f)
		g.SettingsDirtyTimer = g.IO.IniSavingRate;
}

static void MarkIniSettingsDirty(ImGuiWindow* window)
{
	ImGuiContext& g = *GImGui;
	if (!(window->Flags & ImGuiWindowFlags_NoSavedSettings))
		if (g.SettingsDirtyTimer <= 0.0f)
			g.SettingsDirtyTimer = g.IO.IniSavingRate;
}

// FIXME: Add a more explicit sort order in the window structure.
static int ChildWindowComparer(const void* lhs, const void* rhs)
{
	const ImGuiWindow* a = *(const ImGuiWindow**)lhs;
	const ImGuiWindow* b = *(const ImGuiWindow**)rhs;
	if (int d = (a->Flags & ImGuiWindowFlags_Popup) - (b->Flags & ImGuiWindowFlags_Popup))
		return d;
	if (int d = (a->Flags & ImGuiWindowFlags_Tooltip) - (b->Flags & ImGuiWindowFlags_Tooltip))
		return d;
	return (a->BeginOrderWithinParent - b->BeginOrderWithinParent);
}

static void AddWindowToSortedBuffer(ImVector<ImGuiWindow*>* out_sorted_windows, ImGuiWindow* window)
{
	out_sorted_windows->push_back(window);
	if (window->Active)
	{
		int count = window->DC.ChildWindows.Size;
		if (count > 1)
			qsort(window->DC.ChildWindows.begin(), (size_t)count, sizeof(ImGuiWindow*), ChildWindowComparer);
		for (int i = 0; i < count; i++)
		{
			ImGuiWindow* child = window->DC.ChildWindows[i];
			if (child->Active)
				AddWindowToSortedBuffer(out_sorted_windows, child);
		}
	}
}

static void AddDrawListToDrawData(ImVector<ImDrawList*>* out_render_list, ImDrawList* draw_list)
{
	if (draw_list->CmdBuffer.empty())
		return;

	// Remove trailing command if unused
	ImDrawCmd& last_cmd = draw_list->CmdBuffer.back();
	if (last_cmd.ElemCount == 0 && last_cmd.UserCallback == NULL)
	{
		draw_list->CmdBuffer.pop_back();
		if (draw_list->CmdBuffer.empty())
			return;
	}

	// Draw list sanity check. Detect mismatch between PrimReserve() calls and incrementing _VtxCurrentIdx, _VtxWritePtr etc. May trigger for you if you are using PrimXXX functions incorrectly.
	IM_ASSERT(draw_list->VtxBuffer.Size == 0 || draw_list->_VtxWritePtr == draw_list->VtxBuffer.Data + draw_list->VtxBuffer.Size);
	IM_ASSERT(draw_list->IdxBuffer.Size == 0 || draw_list->_IdxWritePtr == draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size);
	IM_ASSERT((int)draw_list->_VtxCurrentIdx == draw_list->VtxBuffer.Size);

	// Check that draw_list doesn't use more vertices than indexable (default ImDrawIdx = unsigned short = 2 bytes = 64K vertices per ImDrawList = per window)
	// If this assert triggers because you are drawing lots of stuff manually:
	// A) Make sure you are coarse clipping, because ImDrawList let all your vertices pass. You can use the Metrics window to inspect draw list contents.
	// B) If you need/want meshes with more than 64K vertices, uncomment the '#define ImDrawIdx unsigned int' line in imconfig.h to set the index size to 4 bytes. 
	//    You'll need to handle the 4-bytes indices to your renderer. For example, the OpenGL example code detect index size at compile-time by doing:
	//      glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
	//    Your own engine or render API may use different parameters or function calls to specify index sizes. 2 and 4 bytes indices are generally supported by most API.
	// C) If for some reason you cannot use 4 bytes indices or don't want to, a workaround is to call BeginChild()/EndChild() before reaching the 64K limit to split your draw commands in multiple draw lists.
	if (sizeof(ImDrawIdx) == 2)
		IM_ASSERT(draw_list->_VtxCurrentIdx < (1 << 16) && "Too many vertices in ImDrawList using 16-bit indices. Read comment above");

	out_render_list->push_back(draw_list);
}

static void AddWindowToDrawData(ImVector<ImDrawList*>* out_render_list, ImGuiWindow* window)
{
	AddDrawListToDrawData(out_render_list, window->DrawList);
	for (int i = 0; i < window->DC.ChildWindows.Size; i++)
	{
		ImGuiWindow* child = window->DC.ChildWindows[i];
		if (child->Active && child->HiddenFrames <= 0) // clipped children may have been marked not active
			AddWindowToDrawData(out_render_list, child);
	}
}

static void AddWindowToDrawDataSelectLayer(ImGuiWindow* window)
{
	ImGuiContext& g = *GImGui;
	g.IO.MetricsActiveWindows++;
	if (window->Flags & ImGuiWindowFlags_Tooltip)
		AddWindowToDrawData(&g.DrawDataBuilder.Layers[1], window);
	else
		AddWindowToDrawData(&g.DrawDataBuilder.Layers[0], window);
}

void ImDrawDataBuilder::FlattenIntoSingleLayer()
{
	int n = Layers[0].Size;
	int size = n;
	for (int i = 1; i < IM_ARRAYSIZE(Layers); i++)
		size += Layers[i].Size;
	Layers[0].resize(size);
	for (int layer_n = 1; layer_n < IM_ARRAYSIZE(Layers); layer_n++)
	{
		ImVector<ImDrawList*>& layer = Layers[layer_n];
		if (layer.empty())
			continue;
		memcpy(&Layers[0][n], &layer[0], layer.Size * sizeof(ImDrawList*));
		n += layer.Size;
		layer.resize(0);
	}
}

static void SetupDrawData(ImVector<ImDrawList*>* draw_lists, ImDrawData* out_draw_data)
{
	out_draw_data->Valid = true;
	out_draw_data->CmdLists = (draw_lists->Size > 0) ? draw_lists->Data : NULL;
	out_draw_data->CmdListsCount = draw_lists->Size;
	out_draw_data->TotalVtxCount = out_draw_data->TotalIdxCount = 0;
	for (int n = 0; n < draw_lists->Size; n++)
	{
		out_draw_data->TotalVtxCount += draw_lists->Data[n]->VtxBuffer.Size;
		out_draw_data->TotalIdxCount += draw_lists->Data[n]->IdxBuffer.Size;
	}
}

// When using this function it is sane to ensure that float are perfectly rounded to integer values, to that e.g. (int)(max.x-min.x) in user's render produce correct result.
void ImGui::PushClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect)
{
	ImGuiWindow* window = GetCurrentWindow();
	window->DrawList->PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
	window->ClipRect = window->DrawList->_ClipRectStack.back();
}

void ImGui::PopClipRect()
{
	ImGuiWindow* window = GetCurrentWindow();
	window->DrawList->PopClipRect();
	window->ClipRect = window->DrawList->_ClipRectStack.back();
}

// This is normally called by Render(). You may want to call it directly if you want to avoid calling Render() but the gain will be very minimal.
void ImGui::EndFrame()
{
	ImGuiContext& g = *GImGui;
	IM_ASSERT(g.Initialized);                       // Forgot to call ImGui::NewFrame()
	if (g.FrameCountEnded == g.FrameCount)          // Don't process EndFrame() multiple times.
		return;

	// Notify OS when our Input Method Editor cursor has moved (e.g. CJK inputs using Microsoft IME)
	if (g.IO.ImeSetInputScreenPosFn && ImLengthSqr(g.OsImePosRequest - g.OsImePosSet) > 0.0001f)
	{
		g.IO.ImeSetInputScreenPosFn((int)g.OsImePosRequest.x, (int)g.OsImePosRequest.y);
		g.OsImePosSet = g.OsImePosRequest;
	}

	// Hide implicit "Debug" window if it hasn't been used
	IM_ASSERT(g.CurrentWindowStack.Size == 1);    // Mismatched Begin()/End() calls
	if (g.CurrentWindow && !g.CurrentWindow->WriteAccessed)
		g.CurrentWindow->Active = false;
	End();

	if (g.ActiveId == 0 && g.HoveredId == 0)
	{
		if (!g.NavWindow || !g.NavWindow->Appearing) // Unless we just made a window/popup appear
		{
			// Click to focus window and start moving (after we're done with all our widgets)
			if (g.IO.MouseClicked[0])
			{
				if (g.HoveredRootWindow != NULL)
				{
					// Set ActiveId even if the _NoMove flag is set, without it dragging away from a window with _NoMove would activate hover on other windows.
					FocusWindow(g.HoveredWindow);
					SetActiveID(g.HoveredWindow->MoveId, g.HoveredWindow);
					g.ActiveIdClickOffset = g.IO.MousePos - g.HoveredRootWindow->Pos;
					if (!(g.HoveredWindow->Flags & ImGuiWindowFlags_NoMove) && !(g.HoveredRootWindow->Flags & ImGuiWindowFlags_NoMove))
						g.MovingWindow = g.HoveredWindow;
				}
				else if (g.NavWindow != NULL && GetFrontMostModalRootWindow() == NULL)
				{
					// Clicking on void disable focus
					FocusWindow(NULL);
				}
			}

			// With right mouse button we close popups without changing focus
			// (The left mouse button path calls FocusWindow which will lead NewFrame->ClosePopupsOverWindow to trigger)
			if (g.IO.MouseClicked[1])
			{
				// Find the top-most window between HoveredWindow and the front most Modal Window.
				// This is where we can trim the popup stack.
				ImGuiWindow* modal = GetFrontMostModalRootWindow();
				bool hovered_window_above_modal = false;
				if (modal == NULL)
					hovered_window_above_modal = true;
				for (int i = g.Windows.Size - 1; i >= 0 && hovered_window_above_modal == false; i--)
				{
					ImGuiWindow* window = g.Windows[i];
					if (window == modal)
						break;
					if (window == g.HoveredWindow)
						hovered_window_above_modal = true;
				}
				ClosePopupsOverWindow(hovered_window_above_modal ? g.HoveredWindow : modal);
			}
		}
	}

	// Sort the window list so that all child windows are after their parent
	// We cannot do that on FocusWindow() because childs may not exist yet
	g.WindowsSortBuffer.resize(0);
	g.WindowsSortBuffer.reserve(g.Windows.Size);
	for (int i = 0; i != g.Windows.Size; i++)
	{
		ImGuiWindow* window = g.Windows[i];
		if (window->Active && (window->Flags & ImGuiWindowFlags_ChildWindow))       // if a child is active its parent will add it
			continue;
		AddWindowToSortedBuffer(&g.WindowsSortBuffer, window);
	}

	IM_ASSERT(g.Windows.Size == g.WindowsSortBuffer.Size);  // we done something wrong
	g.Windows.swap(g.WindowsSortBuffer);

	// Clear Input data for next frame
	g.IO.MouseWheel = g.IO.MouseWheelH = 0.0f;
	memset(g.IO.InputCharacters, 0, sizeof(g.IO.InputCharacters));

	g.FrameCountEnded = g.FrameCount;
}

void ImGui::Render()
{
	ImGuiContext& g = *GImGui;
	IM_ASSERT(g.Initialized);   // Forgot to call ImGui::NewFrame()

	if (g.FrameCountEnded != g.FrameCount)
		ImGui::EndFrame();
	g.FrameCountRendered = g.FrameCount;

	// Skip render altogether if alpha is 0.0
	// Note that vertex buffers have been created and are wasted, so it is best practice that you don't create windows in the first place, or consistently respond to Begin() returning false.
	if (g.Style.Alpha > 0.0f)
	{
		// Gather windows to render
		g.IO.MetricsRenderVertices = g.IO.MetricsRenderIndices = g.IO.MetricsActiveWindows = 0;
		g.DrawDataBuilder.Clear();
		for (int n = 0; n != g.Windows.Size; n++)
		{
			ImGuiWindow* window = g.Windows[n];
			if (window->Active && window->HiddenFrames <= 0 && (window->Flags & (ImGuiWindowFlags_ChildWindow)) == 0)
				AddWindowToDrawDataSelectLayer(window);
		}
		g.DrawDataBuilder.FlattenIntoSingleLayer();

		// Draw software mouse cursor if requested
		ImVec2 offset, size, uv[4];
		if (g.IO.MouseDrawCursor && g.IO.Fonts->GetMouseCursorTexData(g.MouseCursor, &offset, &size, &uv[0], &uv[2]))
		{
			const ImVec2 pos = g.IO.MousePos - offset;
			const ImTextureID tex_id = g.IO.Fonts->TexID;
			g.OverlayDrawList.PushTextureID(tex_id);
			g.OverlayDrawList.AddImage(tex_id, pos + ImVec2(1, 0), pos + ImVec2(1, 0) + size, uv[2], uv[3], IM_COL32(0, 0, 0, 48));        // Shadow
			g.OverlayDrawList.AddImage(tex_id, pos + ImVec2(2, 0), pos + ImVec2(2, 0) + size, uv[2], uv[3], IM_COL32(0, 0, 0, 48));        // Shadow
			g.OverlayDrawList.AddImage(tex_id, pos, pos + size, uv[2], uv[3], IM_COL32(0, 0, 0, 255));       // Black border
			g.OverlayDrawList.AddImage(tex_id, pos, pos + size, uv[0], uv[1], IM_COL32(255, 255, 255, 255)); // White fill
			g.OverlayDrawList.PopTextureID();
		}
		if (!g.OverlayDrawList.VtxBuffer.empty())
			AddDrawListToDrawData(&g.DrawDataBuilder.Layers[0], &g.OverlayDrawList);

		// Setup ImDrawData structure for end-user
		SetupDrawData(&g.DrawDataBuilder.Layers[0], &g.DrawData);
		g.IO.MetricsRenderVertices = g.DrawData.TotalVtxCount;
		g.IO.MetricsRenderIndices = g.DrawData.TotalIdxCount;

		// Render. If user hasn't set a callback then they may retrieve the draw data via GetDrawData()
		if (g.DrawData.CmdListsCount > 0 && g.IO.RenderDrawListsFn != NULL)
			g.IO.RenderDrawListsFn(&g.DrawData);
	}
}

const char* ImGui::FindRenderedTextEnd(const char* text, const char* text_end)
{
	const char* text_display_end = text;
	if (!text_end)
		text_end = (const char*)-1;

	while (text_display_end < text_end && *text_display_end != '\0' && (text_display_end[0] != '#' || text_display_end[1] != '#'))
		text_display_end++;
	return text_display_end;
}

// Pass text data straight to log (without being displayed)
void ImGui::LogText(const char* fmt, ...)
{
	ImGuiContext& g = *GImGui;
	if (!g.LogEnabled)
		return;

	va_list args;
	va_start(args, fmt);
	if (g.LogFile)
	{
		vfprintf(g.LogFile, fmt, args);
	}
	else
	{
		g.LogClipboard->appendfv(fmt, args);
	}
	va_end(args);
}

// Internal version that takes a position to decide on newline placement and pad items according to their depth.
// We split text into individual lines to add current tree level padding
static void LogRenderedText(const ImVec2* ref_pos, const char* text, const char* text_end = NULL)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	if (!text_end)
		text_end = ImGui::FindRenderedTextEnd(text, text_end);

	const bool log_new_line = ref_pos && (ref_pos->y > window->DC.LogLinePosY + 1);
	if (ref_pos)
		window->DC.LogLinePosY = ref_pos->y;

	const char* text_remaining = text;
	if (g.LogStartDepth > window->DC.TreeDepth)  // Re-adjust padding if we have popped out of our starting depth
		g.LogStartDepth = window->DC.TreeDepth;
	const int tree_depth = (window->DC.TreeDepth - g.LogStartDepth);
	for (;;)
	{
		// Split the string. Each new line (after a '\n') is followed by spacing corresponding to the current depth of our log entry.
		const char* line_end = text_remaining;
		while (line_end < text_end)
			if (*line_end == '\n')
				break;
			else
				line_end++;
		if (line_end >= text_end)
			line_end = NULL;

		const bool is_first_line = (text == text_remaining);
		bool is_last_line = false;
		if (line_end == NULL)
		{
			is_last_line = true;
			line_end = text_end;
		}
		if (line_end != NULL && !(is_last_line && (line_end - text_remaining) == 0))
		{
			const int char_count = (int)(line_end - text_remaining);
			if (log_new_line || !is_first_line)
				ImGui::LogText(IM_NEWLINE "%*s%.*s", tree_depth * 4, "", char_count, text_remaining);
			else
				ImGui::LogText(" %.*s", char_count, text_remaining);
		}
		
		if (is_last_line)
			break;
		text_remaining = line_end + 1;
	}
}

// Internal ImGui functions to render text
// RenderText***() functions calls ImDrawList::AddText() calls ImBitmapFont::RenderText()
void ImGui::RenderText(ImVec2 pos, const char* text, const char* text_end, bool hide_text_after_hash)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	// Hide anything after a '##' string
	const char* text_display_end;
	if (hide_text_after_hash)
	{
		text_display_end = FindRenderedTextEnd(text, text_end);
	}
	else
	{
		if (!text_end)
			text_end = text + strlen(text); // FIXME-OPT
		text_display_end = text_end;
	}

	const int text_len = (int)(text_display_end - text);
	if (text_len > 0)
	{
		window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_display_end);
		if (g.LogEnabled)
			LogRenderedText(&pos, text, text_display_end);
	}
}

void ImGui::RenderTextWrapped(ImVec2 pos, const char* text, const char* text_end, float wrap_width)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	if (!text_end)
		text_end = text + strlen(text); // FIXME-OPT

	const int text_len = (int)(text_end - text);
	if (text_len > 0)
	{
		window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_end, wrap_width);
		if (g.LogEnabled)
			LogRenderedText(&pos, text, text_end);
	}
}

// Default clip_rect uses (pos_min,pos_max)
// Handle clipping on CPU immediately (vs typically let the GPU clip the triangles that are overlapping the clipping rectangle edges)
void ImGui::RenderTextClipped(const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect)
{
	// Hide anything after a '##' string
	const char* text_display_end = FindRenderedTextEnd(text, text_end);
	const int text_len = (int)(text_display_end - text);
	if (text_len == 0)
		return;

	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	// Perform CPU side clipping for single clipped element to avoid using scissor state
	ImVec2 pos = pos_min;
	const ImVec2 text_size = text_size_if_known ? *text_size_if_known : CalcTextSize(text, text_display_end, false, 0.0f);

	const ImVec2* clip_min = clip_rect ? &clip_rect->Min : &pos_min;
	const ImVec2* clip_max = clip_rect ? &clip_rect->Max : &pos_max;
	bool need_clipping = (pos.x + text_size.x >= clip_max->x) || (pos.y + text_size.y >= clip_max->y);
	if (clip_rect) // If we had no explicit clipping rectangle then pos==clip_min
		need_clipping |= (pos.x < clip_min->x) || (pos.y < clip_min->y);

	// Align whole block. We should defer that to the better rendering function when we'll have support for individual line alignment.
	if (align.x > 0.0f) pos.x = ImMax(pos.x, pos.x + (pos_max.x - pos.x - text_size.x) * align.x);
	if (align.y > 0.0f) pos.y = ImMax(pos.y, pos.y + (pos_max.y - pos.y - text_size.y) * align.y);

	// Render
	if (need_clipping)
	{
		ImVec4 fine_clip_rect(clip_min->x, clip_min->y, clip_max->x, clip_max->y);
		window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_display_end, 0.0f, &fine_clip_rect);
	}
	else
	{
		window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_display_end, 0.0f, NULL);
	}
	if (g.LogEnabled)
		LogRenderedText(&pos, text, text_display_end);
}

// Render a rectangle shaped with optional rounding and borders
void ImGui::RenderFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	window->DrawList->AddRectFilled(p_min, p_max, fill_col, rounding);
	const float border_size = g.Style.FrameBorderSize;
	if (border && border_size > 0.0f)
	{
		window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1), GetColorU32(ImGuiCol_BorderShadow), rounding, ImDrawCornerFlags_All, border_size);
		window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding, ImDrawCornerFlags_All, border_size);
	}
}

void ImGui::RenderFrameBorder(ImVec2 p_min, ImVec2 p_max, float rounding)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	const float border_size = g.Style.FrameBorderSize;
	if (border_size > 0.0f)
	{
		window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1), GetColorU32(ImGuiCol_BorderShadow), rounding, ImDrawCornerFlags_All, border_size);
		window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding, ImDrawCornerFlags_All, border_size);
	}
}

// Render a triangle to denote expanded/collapsed state
void ImGui::RenderTriangle(ImVec2 p_min, ImGuiDir dir, float scale)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	const float h = g.FontSize * 1.00f;
	float r = h * 0.40f * scale;
	ImVec2 center = p_min + ImVec2(h * 0.50f, h * 0.50f * scale);

	ImVec2 a, b, c;
	switch (dir)
	{
	case ImGuiDir_Up:
	case ImGuiDir_Down:
		if (dir == ImGuiDir_Up) r = -r;
		center.y -= r * 0.25f;
		a = ImVec2(0, 1) * r;
		b = ImVec2(-0.866f, -0.5f) * r;
		c = ImVec2(+0.866f, -0.5f) * r;
		break;
	case ImGuiDir_Left:
	case ImGuiDir_Right:
		if (dir == ImGuiDir_Left) r = -r;
		center.x -= r * 0.25f;
		a = ImVec2(1, 0) * r;
		b = ImVec2(-0.500f, +0.866f) * r;
		c = ImVec2(-0.500f, -0.866f) * r;
		break;
	case ImGuiDir_None:
	case ImGuiDir_Count_:
		IM_ASSERT(0);
		break;
	}

	window->DrawList->AddTriangleFilled(center + a, center + b, center + c, GetColorU32(ImGuiCol_Text));
}

void ImGui::RenderBullet(ImVec2 pos)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	window->DrawList->AddCircleFilled(pos, GImGui->FontSize*0.20f, GetColorU32(ImGuiCol_Text), 8);
}

void ImGui::RenderCheckMark(ImVec2 pos, ImU32 col, float sz)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	float thickness = ImMax(sz / 5.0f, 1.0f);
	sz -= thickness*0.5f;
	pos += ImVec2(thickness*0.25f, thickness*0.25f);

	float third = sz / 3.0f;
	float bx = pos.x + third;
	float by = pos.y + sz - third*0.5f;
	window->DrawList->PathLineTo(ImVec2(bx - third, by - third));
	window->DrawList->PathLineTo(ImVec2(bx, by));
	window->DrawList->PathLineTo(ImVec2(bx + third * 2, by - third * 2));
	window->DrawList->PathStroke(col, false, thickness);
}

// Calculate text size. Text can be multi-line. Optionally ignore text after a ## marker.
// CalcTextSize("") should return ImVec2(0.0f, GImGui->FontSize)
ImVec2 ImGui::CalcTextSize(const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width)
{
	ImGuiContext& g = *GImGui;

	const char* text_display_end;
	if (hide_text_after_double_hash)
		text_display_end = FindRenderedTextEnd(text, text_end);      // Hide anything after a '##' string
	else
		text_display_end = text_end;

	ImFont* font = g.Font;
	const float font_size = g.FontSize;
	if (text == text_display_end)
		return ImVec2(0.0f, font_size);
	ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, wrap_width, text, text_display_end, NULL);

	// Cancel out character spacing for the last character of a line (it is baked into glyph->AdvanceX field)
	const float font_scale = font_size / font->FontSize;
	const float character_spacing_x = 1.0f * font_scale;
	if (text_size.x > 0.0f)
		text_size.x -= character_spacing_x;
	text_size.x = (float)(int)(text_size.x + 0.95f);

	return text_size;
}

// Helper to calculate coarse clipping of large list of evenly sized items.
// NB: Prefer using the ImGuiListClipper higher-level helper if you can! Read comments and instructions there on how those use this sort of pattern.
// NB: 'items_count' is only used to clamp the result, if you don't know your count you can use INT_MAX
void ImGui::CalcListClipping(int items_count, float items_height, int* out_items_display_start, int* out_items_display_end)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	if (g.LogEnabled)
	{
		// If logging is active, do not perform any clipping
		*out_items_display_start = 0;
		*out_items_display_end = items_count;
		return;
	}
	if (window->SkipItems)
	{
		*out_items_display_start = *out_items_display_end = 0;
		return;
	}

	const ImVec2 pos = window->DC.CursorPos;
	int start = (int)((window->ClipRect.Min.y - pos.y) / items_height);
	int end = (int)((window->ClipRect.Max.y - pos.y) / items_height);
	start = ImClamp(start, 0, items_count);
	end = ImClamp(end + 1, start, items_count);
	*out_items_display_start = start;
	*out_items_display_end = end;
}

// Find window given position, search front-to-back
// FIXME: Note that we have a lag here because WindowRectClipped is updated in Begin() so windows moved by user via SetWindowPos() and not SetNextWindowPos() will have that rectangle lagging by a frame at the time FindHoveredWindow() is called, aka before the next Begin(). Moving window thankfully isn't affected.
static ImGuiWindow* FindHoveredWindow(ImVec2 pos)
{
	ImGuiContext& g = *GImGui;
	for (int i = g.Windows.Size - 1; i >= 0; i--)
	{
		ImGuiWindow* window = g.Windows[i];
		if (!window->Active)
			continue;
		if (window->Flags & ImGuiWindowFlags_NoInputs)
			continue;

		// Using the clipped AABB, a child window will typically be clipped by its parent (not always)
		ImRect bb(window->WindowRectClipped.Min - g.Style.TouchExtraPadding, window->WindowRectClipped.Max + g.Style.TouchExtraPadding);
		if (bb.Contains(pos))
			return window;
	}
	return NULL;
}

// Test if mouse cursor is hovering given rectangle
// NB- Rectangle is clipped by our current clip setting
// NB- Expand the rectangle to be generous on imprecise inputs systems (g.Style.TouchExtraPadding)
bool ImGui::IsMouseHoveringRect(const ImVec2& r_min, const ImVec2& r_max, bool clip)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	// Clip
	ImRect rect_clipped(r_min, r_max);
	if (clip)
		rect_clipped.ClipWith(window->ClipRect);

	// Expand for touch input
	const ImRect rect_for_touch(rect_clipped.Min - g.Style.TouchExtraPadding, rect_clipped.Max + g.Style.TouchExtraPadding);
	return rect_for_touch.Contains(g.IO.MousePos);
}

static bool IsKeyPressedMap(ImGuiKey key, bool repeat)
{
	const int key_index = GImGui->IO.KeyMap[key];
	return (key_index >= 0) ? ImGui::IsKeyPressed(key_index, repeat) : false;
}

int ImGui::GetKeyIndex(ImGuiKey imgui_key)
{
	IM_ASSERT(imgui_key >= 0 && imgui_key < ImGuiKey_COUNT);
	return GImGui->IO.KeyMap[imgui_key];
}

// Note that imgui doesn't know the semantic of each entry of io.KeyDown[]. Use your own indices/enums according to how your backend/engine stored them into KeyDown[]!
bool ImGui::IsKeyDown(int user_key_index)
{
	if (user_key_index < 0) return false;
	IM_ASSERT(user_key_index >= 0 && user_key_index < IM_ARRAYSIZE(GImGui->IO.KeysDown));
	return GImGui->IO.KeysDown[user_key_index];
}

int ImGui::CalcTypematicPressedRepeatAmount(float t, float t_prev, float repeat_delay, float repeat_rate)
{
	if (t == 0.0f)
		return 1;
	if (t <= repeat_delay || repeat_rate <= 0.0f)
		return 0;
	const int count = (int)((t - repeat_delay) / repeat_rate) - (int)((t_prev - repeat_delay) / repeat_rate);
	return (count > 0) ? count : 0;
}

int ImGui::GetKeyPressedAmount(int key_index, float repeat_delay, float repeat_rate)
{
	ImGuiContext& g = *GImGui;
	if (key_index < 0) return false;
	IM_ASSERT(key_index >= 0 && key_index < IM_ARRAYSIZE(g.IO.KeysDown));
	const float t = g.IO.KeysDownDuration[key_index];
	return CalcTypematicPressedRepeatAmount(t, t - g.IO.DeltaTime, repeat_delay, repeat_rate);
}

bool ImGui::IsKeyPressed(int user_key_index, bool repeat)
{
	ImGuiContext& g = *GImGui;
	if (user_key_index < 0) return false;
	IM_ASSERT(user_key_index >= 0 && user_key_index < IM_ARRAYSIZE(g.IO.KeysDown));
	const float t = g.IO.KeysDownDuration[user_key_index];
	if (t == 0.0f)
		return true;
	if (repeat && t > g.IO.KeyRepeatDelay)
		return GetKeyPressedAmount(user_key_index, g.IO.KeyRepeatDelay, g.IO.KeyRepeatRate) > 0;
	return false;
}

bool ImGui::IsKeyReleased(int user_key_index)
{
	ImGuiContext& g = *GImGui;
	if (user_key_index < 0) return false;
	IM_ASSERT(user_key_index >= 0 && user_key_index < IM_ARRAYSIZE(g.IO.KeysDown));
	return g.IO.KeysDownDurationPrev[user_key_index] >= 0.0f && !g.IO.KeysDown[user_key_index];
}

bool ImGui::IsMouseDown(int button)
{
	ImGuiContext& g = *GImGui;
	IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
	return g.IO.MouseDown[button];
}

bool ImGui::IsMouseClicked(int button, bool repeat)
{
	ImGuiContext& g = *GImGui;
	IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
	const float t = g.IO.MouseDownDuration[button];
	if (t == 0.0f)
		return true;

	if (repeat && t > g.IO.KeyRepeatDelay)
	{
		float delay = g.IO.KeyRepeatDelay, rate = g.IO.KeyRepeatRate;
		if ((fmodf(t - delay, rate) > rate*0.5f) != (fmodf(t - delay - g.IO.DeltaTime, rate) > rate*0.5f))
			return true;
	}

	return false;
}

bool ImGui::IsMouseReleased(int button)
{
	ImGuiContext& g = *GImGui;
	IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
	return g.IO.MouseReleased[button];
}

bool ImGui::IsMouseDoubleClicked(int button)
{
	ImGuiContext& g = *GImGui;
	IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
	return g.IO.MouseDoubleClicked[button];
}

bool ImGui::IsMouseDragging(int button, float lock_threshold)
{
	ImGuiContext& g = *GImGui;
	IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
	if (!g.IO.MouseDown[button])
		return false;
	if (lock_threshold < 0.0f)
		lock_threshold = g.IO.MouseDragThreshold;
	return g.IO.MouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold;
}

ImVec2 ImGui::GetMousePos()
{
	return GImGui->IO.MousePos;
}

// NB: prefer to call right after BeginPopup(). At the time Selectable/MenuItem is activated, the popup is already closed!
ImVec2 ImGui::GetMousePosOnOpeningCurrentPopup()
{
	ImGuiContext& g = *GImGui;
	if (g.CurrentPopupStack.Size > 0)
		return g.OpenPopupStack[g.CurrentPopupStack.Size - 1].OpenMousePos;
	return g.IO.MousePos;
}

// We typically use ImVec2(-FLT_MAX,-FLT_MAX) to denote an invalid mouse position
bool ImGui::IsMousePosValid(const ImVec2* mouse_pos)
{
	if (mouse_pos == NULL)
		mouse_pos = &GImGui->IO.MousePos;
	const float MOUSE_INVALID = -256000.0f;
	return mouse_pos->x >= MOUSE_INVALID && mouse_pos->y >= MOUSE_INVALID;
}

// NB: This is only valid if IsMousePosValid(). Backends in theory should always keep mouse position valid when dragging even outside the client window.
ImVec2 ImGui::GetMouseDragDelta(int button, float lock_threshold)
{
	ImGuiContext& g = *GImGui;
	IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
	if (lock_threshold < 0.0f)
		lock_threshold = g.IO.MouseDragThreshold;
	if (g.IO.MouseDown[button])
		if (g.IO.MouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold)
			return g.IO.MousePos - g.IO.MouseClickedPos[button];     // Assume we can only get active with left-mouse button (at the moment).
	return ImVec2(0.0f, 0.0f);
}

void ImGui::ResetMouseDragDelta(int button)
{
	ImGuiContext& g = *GImGui;
	IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
	// NB: We don't need to reset g.IO.MouseDragMaxDistanceSqr
	g.IO.MouseClickedPos[button] = g.IO.MousePos;
}

ImGuiMouseCursor ImGui::GetMouseCursor()
{
	return GImGui->MouseCursor;
}

void ImGui::SetMouseCursor(ImGuiMouseCursor cursor_type)
{
	GImGui->MouseCursor = cursor_type;
}

void ImGui::CaptureKeyboardFromApp(bool capture)
{
	GImGui->WantCaptureKeyboardNextFrame = capture ? 1 : 0;
}

void ImGui::CaptureMouseFromApp(bool capture)
{
	GImGui->WantCaptureMouseNextFrame = capture ? 1 : 0;
}

bool ImGui::IsItemActive()
{
	ImGuiContext& g = *GImGui;
	if (g.ActiveId)
	{
		ImGuiWindow* window = g.CurrentWindow;
		return g.ActiveId == window->DC.LastItemId;
	}
	return false;
}

bool ImGui::IsItemClicked(int mouse_button)
{
	return IsMouseClicked(mouse_button) && IsItemHovered(ImGuiHoveredFlags_Default);
}

bool ImGui::IsAnyItemHovered()
{
	ImGuiContext& g = *GImGui;
	return g.HoveredId != 0 || g.HoveredIdPreviousFrame != 0;
}

bool ImGui::IsAnyItemActive()
{
	return GImGui->ActiveId != 0;
}

bool ImGui::IsItemVisible()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->ClipRect.Overlaps(window->DC.LastItemRect);
}

// Allow last item to be overlapped by a subsequent item. Both may be activated during the same frame before the later one takes priority.
void ImGui::SetItemAllowOverlap()
{
	ImGuiContext& g = *GImGui;
	if (g.HoveredId == g.CurrentWindow->DC.LastItemId)
		g.HoveredIdAllowOverlap = true;
	if (g.ActiveId == g.CurrentWindow->DC.LastItemId)
		g.ActiveIdAllowOverlap = true;
}

ImVec2 ImGui::GetItemRectMin()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->DC.LastItemRect.Min;
}

ImVec2 ImGui::GetItemRectMax()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->DC.LastItemRect.Max;
}

ImVec2 ImGui::GetItemRectSize()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->DC.LastItemRect.GetSize();
}

static ImRect GetViewportRect()
{
	ImGuiContext& g = *GImGui;
	if (g.IO.DisplayVisibleMin.x != g.IO.DisplayVisibleMax.x && g.IO.DisplayVisibleMin.y != g.IO.DisplayVisibleMax.y)
		return ImRect(g.IO.DisplayVisibleMin, g.IO.DisplayVisibleMax);
	return ImRect(0.0f, 0.0f, g.IO.DisplaySize.x, g.IO.DisplaySize.y);
}

// Not exposed publicly as BeginTooltip() because bool parameters are evil. Let's see if other needs arise first.
void ImGui::BeginTooltipEx(ImGuiWindowFlags extra_flags, bool override_previous_tooltip)
{
	ImGuiContext& g = *GImGui;
	char window_name[16];
	ImFormatString(window_name, IM_ARRAYSIZE(window_name), "##Tooltip_%02d", g.TooltipOverrideCount);
	if (override_previous_tooltip)
		if (ImGuiWindow* window = FindWindowByName(window_name))
			if (window->Active)
			{
				// Hide previous tooltips. We can't easily "reset" the content of a window so we create a new one.
				window->HiddenFrames = 1;
				ImFormatString(window_name, IM_ARRAYSIZE(window_name), "##Tooltip_%02d", ++g.TooltipOverrideCount);
			}
	ImGuiWindowFlags flags = ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
	Begin(window_name, NULL, flags | extra_flags);
}

void ImGui::SetTooltipV(const char* fmt, va_list args)
{
	BeginTooltipEx(0, true);
	TextV(fmt, args);
	EndTooltip();
}

void ImGui::SetTooltip(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	SetTooltipV(fmt, args);
	va_end(args);
}

void ImGui::BeginTooltip()
{
	BeginTooltipEx(0, false);
}

void ImGui::EndTooltip()
{
	IM_ASSERT(GetCurrentWindowRead()->Flags & ImGuiWindowFlags_Tooltip);   // Mismatched BeginTooltip()/EndTooltip() calls
	End();
}

// Mark popup as open (toggle toward open state).
// Popups are closed when user click outside, or activate a pressable item, or CloseCurrentPopup() is called within a BeginPopup()/EndPopup() block.
// Popup identifiers are relative to the current ID-stack (so OpenPopup and BeginPopup needs to be at the same level).
// One open popup per level of the popup hierarchy (NB: when assigning we reset the Window member of ImGuiPopupRef to NULL)
void ImGui::OpenPopupEx(ImGuiID id)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* parent_window = g.CurrentWindow;
	int current_stack_size = g.CurrentPopupStack.Size;
	ImGuiPopupRef popup_ref; // Tagged as new ref as Window will be set back to NULL if we write this into OpenPopupStack.
	popup_ref.PopupId = id;
	popup_ref.Window = NULL;
	popup_ref.ParentWindow = parent_window;
	popup_ref.OpenFrameCount = g.FrameCount;
	popup_ref.OpenParentId = parent_window->IDStack.back();
	popup_ref.OpenMousePos = g.IO.MousePos;
	popup_ref.OpenPopupPos = g.IO.MousePos; // NB: In the Navigation branch OpenPopupPos doesn't use the mouse position, hence the separation here.

	if (g.OpenPopupStack.Size < current_stack_size + 1)
	{
		g.OpenPopupStack.push_back(popup_ref);
	}
	else
	{
		// Close child popups if any
		g.OpenPopupStack.resize(current_stack_size + 1);

		// Gently handle the user mistakenly calling OpenPopup() every frame. It is a programming mistake! However, if we were to run the regular code path, the ui
		// would become completely unusable because the popup will always be in hidden-while-calculating-size state _while_ claiming focus. Which would be a very confusing
		// situation for the programmer. Instead, we silently allow the popup to proceed, it will keep reappearing and the programming error will be more obvious to understand. 
		if (g.OpenPopupStack[current_stack_size].PopupId == id && g.OpenPopupStack[current_stack_size].OpenFrameCount == g.FrameCount - 1)
			g.OpenPopupStack[current_stack_size].OpenFrameCount = popup_ref.OpenFrameCount;
		else
			g.OpenPopupStack[current_stack_size] = popup_ref;

		// When reopening a popup we first refocus its parent, otherwise if its parent is itself a popup it would get closed by ClosePopupsOverWindow().
		// This is equivalent to what ClosePopupToLevel() does.
		//if (g.OpenPopupStack[current_stack_size].PopupId == id)
		//    FocusWindow(parent_window);
	}
}

void ImGui::OpenPopup(const char* str_id)
{
	ImGuiContext& g = *GImGui;
	OpenPopupEx(g.CurrentWindow->GetID(str_id));
}

void ImGui::ClosePopupsOverWindow(ImGuiWindow* ref_window)
{
	ImGuiContext& g = *GImGui;
	if (g.OpenPopupStack.empty())
		return;

	// When popups are stacked, clicking on a lower level popups puts focus back to it and close popups above it.
	// Don't close our own child popup windows.
	int n = 0;
	if (ref_window)
	{
		for (n = 0; n < g.OpenPopupStack.Size; n++)
		{
			ImGuiPopupRef& popup = g.OpenPopupStack[n];
			if (!popup.Window)
				continue;
			IM_ASSERT((popup.Window->Flags & ImGuiWindowFlags_Popup) != 0);
			if (popup.Window->Flags & ImGuiWindowFlags_ChildWindow)
				continue;

			// Trim the stack if popups are not direct descendant of the reference window (which is often the NavWindow)
			bool has_focus = false;
			for (int m = n; m < g.OpenPopupStack.Size && !has_focus; m++)
				has_focus = (g.OpenPopupStack[m].Window && g.OpenPopupStack[m].Window->RootWindow == ref_window->RootWindow);
			if (!has_focus)
				break;
		}
	}
	if (n < g.OpenPopupStack.Size) // This test is not required but it allows to set a convenient breakpoint on the block below
		ClosePopupToLevel(n);
}

static ImGuiWindow* GetFrontMostModalRootWindow()
{
	ImGuiContext& g = *GImGui;
	for (int n = g.OpenPopupStack.Size - 1; n >= 0; n--)
		if (ImGuiWindow* popup = g.OpenPopupStack.Data[n].Window)
			if (popup->Flags & ImGuiWindowFlags_Modal)
				return popup;
	return NULL;
}

static void ClosePopupToLevel(int remaining)
{
	ImGuiContext& g = *GImGui;
	if (remaining > 0)
		ImGui::FocusWindow(g.OpenPopupStack[remaining - 1].Window);
	else
		ImGui::FocusWindow(g.OpenPopupStack[0].ParentWindow);
	g.OpenPopupStack.resize(remaining);
}

void ImGui::ClosePopup(ImGuiID id)
{
	if (!IsPopupOpen(id))
		return;
	ImGuiContext& g = *GImGui;
	ClosePopupToLevel(g.OpenPopupStack.Size - 1);
}

// Close the popup we have begin-ed into.
void ImGui::CloseCurrentPopup()
{
	ImGuiContext& g = *GImGui;
	int popup_idx = g.CurrentPopupStack.Size - 1;
	if (popup_idx < 0 || popup_idx >= g.OpenPopupStack.Size || g.CurrentPopupStack[popup_idx].PopupId != g.OpenPopupStack[popup_idx].PopupId)
		return;
	while (popup_idx > 0 && g.OpenPopupStack[popup_idx].Window && (g.OpenPopupStack[popup_idx].Window->Flags & ImGuiWindowFlags_ChildMenu))
		popup_idx--;
	ClosePopupToLevel(popup_idx);
}

bool ImGui::BeginPopupEx(ImGuiID id, ImGuiWindowFlags extra_flags)
{
	ImGuiContext& g = *GImGui;
	if (!IsPopupOpen(id))
	{
		g.NextWindowData.Clear(); // We behave like Begin() and need to consume those values
		return false;
	}

	char name[20];
	if (extra_flags & ImGuiWindowFlags_ChildMenu)
		ImFormatString(name, IM_ARRAYSIZE(name), "##Menu_%02d", g.CurrentPopupStack.Size);    // Recycle windows based on depth
	else
		ImFormatString(name, IM_ARRAYSIZE(name), "##Popup_%08x", id); // Not recycling, so we can close/open during the same frame

	bool is_open = Begin(name, NULL, extra_flags | ImGuiWindowFlags_Popup);
	if (!is_open) // NB: Begin can return false when the popup is completely clipped (e.g. zero size display)
		EndPopup();

	return is_open;
}

bool ImGui::BeginPopup(const char* str_id, ImGuiWindowFlags flags)
{
	ImGuiContext& g = *GImGui;
	if (g.OpenPopupStack.Size <= g.CurrentPopupStack.Size) // Early out for performance
	{
		g.NextWindowData.Clear(); // We behave like Begin() and need to consume those values
		return false;
	}
	return BeginPopupEx(g.CurrentWindow->GetID(str_id), flags | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

bool ImGui::IsPopupOpen(ImGuiID id)
{
	ImGuiContext& g = *GImGui;
	return g.OpenPopupStack.Size > g.CurrentPopupStack.Size && g.OpenPopupStack[g.CurrentPopupStack.Size].PopupId == id;
}

bool ImGui::IsPopupOpen(const char* str_id)
{
	ImGuiContext& g = *GImGui;
	return g.OpenPopupStack.Size > g.CurrentPopupStack.Size && g.OpenPopupStack[g.CurrentPopupStack.Size].PopupId == g.CurrentWindow->GetID(str_id);
}

bool ImGui::BeginPopupModal(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	const ImGuiID id = window->GetID(name);
	if (!IsPopupOpen(id))
	{
		g.NextWindowData.Clear(); // We behave like Begin() and need to consume those values
		return false;
	}

	// Center modal windows by default
	// FIXME: Should test for (PosCond & window->SetWindowPosAllowFlags) with the upcoming window.
	if (g.NextWindowData.PosCond == 0)
		SetNextWindowPos(g.IO.DisplaySize * 0.5f, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	bool is_open = Begin(name, p_open, flags | ImGuiWindowFlags_Popup | ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
	if (!is_open || (p_open && !*p_open)) // NB: is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
	{
		EndPopup();
		if (is_open)
			ClosePopup(id);
		return false;
	}

	return is_open;
}

void ImGui::EndPopup()
{
	ImGuiContext& g = *GImGui; (void)g;
	IM_ASSERT(g.CurrentWindow->Flags & ImGuiWindowFlags_Popup);  // Mismatched BeginPopup()/EndPopup() calls
	IM_ASSERT(g.CurrentPopupStack.Size > 0);
	End();
}

bool ImGui::OpenPopupOnItemClick(const char* str_id, int mouse_button)
{
	ImGuiWindow* window = GImGui->CurrentWindow;
	if (IsMouseReleased(mouse_button) && IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
	{
		ImGuiID id = str_id ? window->GetID(str_id) : window->DC.LastItemId; // If user hasn't passed an ID, we can use the LastItemID. Using LastItemID as a Popup ID won't conflict!
		IM_ASSERT(id != 0);                                                  // However, you cannot pass a NULL str_id if the last item has no identifier (e.g. a Text() item)
		OpenPopupEx(id);
		return true;
	}
	return false;
}

// This is a helper to handle the simplest case of associating one named popup to one given widget.
// You may want to handle this on user side if you have specific needs (e.g. tweaking IsItemHovered() parameters).
// You can pass a NULL str_id to use the identifier of the last item.
bool ImGui::BeginPopupContextItem(const char* str_id, int mouse_button)
{
	ImGuiWindow* window = GImGui->CurrentWindow;
	ImGuiID id = str_id ? window->GetID(str_id) : window->DC.LastItemId; // If user hasn't passed an ID, we can use the LastItemID. Using LastItemID as a Popup ID won't conflict!
	IM_ASSERT(id != 0);                                                  // However, you cannot pass a NULL str_id if the last item has no identifier (e.g. a Text() item)
	if (IsMouseReleased(mouse_button) && IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
		OpenPopupEx(id);
	return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

bool ImGui::BeginPopupContextWindow(const char* str_id, int mouse_button, bool also_over_items)
{
	if (!str_id)
		str_id = "window_context";
	ImGuiID id = GImGui->CurrentWindow->GetID(str_id);
	if (IsMouseReleased(mouse_button) && IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
		if (also_over_items || !IsAnyItemHovered())
			OpenPopupEx(id);
	return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

bool ImGui::BeginPopupContextVoid(const char* str_id, int mouse_button)
{
	if (!str_id)
		str_id = "void_context";
	ImGuiID id = GImGui->CurrentWindow->GetID(str_id);
	if (IsMouseReleased(mouse_button) && !IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
		OpenPopupEx(id);
	return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

static bool BeginChildEx(const char* name, ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* parent_window = ImGui::GetCurrentWindow();
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ChildWindow;
	flags |= (parent_window->Flags & ImGuiWindowFlags_NoMove);  // Inherit the NoMove flag

	const ImVec2 content_avail = ImGui::GetContentRegionAvail();
	ImVec2 size = ImFloor(size_arg);
	const int auto_fit_axises = ((size.x == 0.0f) ? (1 << ImGuiAxis_X) : 0x00) | ((size.y == 0.0f) ? (1 << ImGuiAxis_Y) : 0x00);
	if (size.x <= 0.0f)
		size.x = ImMax(content_avail.x + size.x, 4.0f); // Arbitrary minimum child size (0.0f causing too much issues)
	if (size.y <= 0.0f)
		size.y = ImMax(content_avail.y + size.y, 4.0f);

	const float backup_border_size = g.Style.ChildBorderSize;
	if (!border)
		g.Style.ChildBorderSize = 0.0f;
	flags |= extra_flags;

	char title[256];
	if (name)
		ImFormatString(title, IM_ARRAYSIZE(title), "%s/%s_%08X", parent_window->Name, name, id);
	else
		ImFormatString(title, IM_ARRAYSIZE(title), "%s/%08X", parent_window->Name, id);

	ImGui::SetNextWindowSize(size);
	bool ret = ImGui::Begin(title, NULL, flags);
	ImGuiWindow* child_window = ImGui::GetCurrentWindow();
	child_window->AutoFitChildAxises = auto_fit_axises;
	g.Style.ChildBorderSize = backup_border_size;

	return ret;
}

bool ImGui::BeginChild(const char* str_id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	return BeginChildEx(str_id, window->GetID(str_id), size_arg, border, extra_flags);
}

bool ImGui::BeginChild(ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
	return BeginChildEx(NULL, id, size_arg, border, extra_flags);
}

void ImGui::EndChild()
{
	ImGuiWindow* window = GetCurrentWindow();

	IM_ASSERT(window->Flags & ImGuiWindowFlags_ChildWindow);   // Mismatched BeginChild()/EndChild() callss
	if (window->BeginCount > 1)
	{
		End();
	}
	else
	{
		// When using auto-filling child window, we don't provide full width/height to ItemSize so that it doesn't feed back into automatic size-fitting.
		ImVec2 sz = GetWindowSize();
		if (window->AutoFitChildAxises & (1 << ImGuiAxis_X)) // Arbitrary minimum zero-ish child size of 4.0f causes less trouble than a 0.0f
			sz.x = ImMax(4.0f, sz.x);
		if (window->AutoFitChildAxises & (1 << ImGuiAxis_Y))
			sz.y = ImMax(4.0f, sz.y);
		End();

		ImGuiWindow* parent_window = GetCurrentWindow();
		ImRect bb(parent_window->DC.CursorPos, parent_window->DC.CursorPos + sz);
		ItemSize(sz);
		ItemAdd(bb, 0);
	}
}

// Helper to create a child window / scrolling region that looks like a normal widget frame.
bool ImGui::BeginChildFrame(ImGuiID id, const ImVec2& size, ImGuiWindowFlags extra_flags)
{
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	PushStyleColor(ImGuiCol_ChildBg, style.Colors[ImGuiCol_FrameBg]);
	PushStyleVar(ImGuiStyleVar_ChildRounding, style.FrameRounding);
	PushStyleVar(ImGuiStyleVar_ChildBorderSize, style.FrameBorderSize);
	PushStyleVar(ImGuiStyleVar_WindowPadding, style.FramePadding);
	return BeginChild(id, size, true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysUseWindowPadding | extra_flags);
}

void ImGui::EndChildFrame()
{
	EndChild();
	PopStyleVar(3);
	PopStyleColor();
}

// Save and compare stack sizes on Begin()/End() to detect usage errors
static void CheckStacksSize(ImGuiWindow* window, bool write)
{
	// NOT checking: DC.ItemWidth, DC.AllowKeyboardFocus, DC.ButtonRepeat, DC.TextWrapPos (per window) to allow user to conveniently push once and not pop (they are cleared on Begin)
	ImGuiContext& g = *GImGui;
	int* p_backup = &window->DC.StackSizesBackup[0];
	{ int current = window->IDStack.Size;       if (write) *p_backup = current; else IM_ASSERT(*p_backup == current && "PushID/PopID or TreeNode/TreePop Mismatch!");   p_backup++; }    // Too few or too many PopID()/TreePop()
	{ int current = window->DC.GroupStack.Size; if (write) *p_backup = current; else IM_ASSERT(*p_backup == current && "BeginGroup/EndGroup Mismatch!");                p_backup++; }    // Too few or too many EndGroup()
	{ int current = g.CurrentPopupStack.Size;   if (write) *p_backup = current; else IM_ASSERT(*p_backup == current && "BeginMenu/EndMenu or BeginPopup/EndPopup Mismatch"); p_backup++;}// Too few or too many EndMenu()/EndPopup()
	{ int current = g.ColorModifiers.Size;      if (write) *p_backup = current; else IM_ASSERT(*p_backup == current && "PushStyleColor/PopStyleColor Mismatch!");       p_backup++; }    // Too few or too many PopStyleColor()
	{ int current = g.StyleModifiers.Size;      if (write) *p_backup = current; else IM_ASSERT(*p_backup == current && "PushStyleVar/PopStyleVar Mismatch!");           p_backup++; }    // Too few or too many PopStyleVar()
	{ int current = g.FontStack.Size;           if (write) *p_backup = current; else IM_ASSERT(*p_backup == current && "PushFont/PopFont Mismatch!");                   p_backup++; }    // Too few or too many PopFont()
	IM_ASSERT(p_backup == window->DC.StackSizesBackup + IM_ARRAYSIZE(window->DC.StackSizesBackup));
}

enum ImGuiPopupPositionPolicy
{
	ImGuiPopupPositionPolicy_Default,
	ImGuiPopupPositionPolicy_ComboBox
};

static ImVec2 FindBestWindowPosForPopup(const ImVec2& ref_pos, const ImVec2& size, ImGuiDir* last_dir, const ImRect& r_avoid, ImGuiPopupPositionPolicy policy = ImGuiPopupPositionPolicy_Default)
{
	const ImGuiStyle& style = GImGui->Style;

	// r_avoid = the rectangle to avoid (e.g. for tooltip it is a rectangle around the mouse cursor which we want to avoid. for popups it's a small point around the cursor.)
	// r_outer = the visible area rectangle, minus safe area padding. If our popup size won't fit because of safe area padding we ignore it.
	ImVec2 safe_padding = style.DisplaySafeAreaPadding;
	ImRect r_outer(GetViewportRect());
	r_outer.Expand(ImVec2((size.x - r_outer.GetWidth() > safe_padding.x * 2) ? -safe_padding.x : 0.0f, (size.y - r_outer.GetHeight() > safe_padding.y * 2) ? -safe_padding.y : 0.0f));
	ImVec2 base_pos_clamped = ImClamp(ref_pos, r_outer.Min, r_outer.Max - size);
	//GImGui->OverlayDrawList.AddRect(r_avoid.Min, r_avoid.Max, IM_COL32(255,0,0,255));
	//GImGui->OverlayDrawList.AddRect(r_outer.Min, r_outer.Max, IM_COL32(0,255,0,255));

	// Combo Box policy (we want a connecting edge)
	if (policy == ImGuiPopupPositionPolicy_ComboBox)
	{
		const ImGuiDir dir_prefered_order[ImGuiDir_Count_] = { ImGuiDir_Down, ImGuiDir_Right, ImGuiDir_Left, ImGuiDir_Up };
		for (int n = (*last_dir != ImGuiDir_None) ? -1 : 0; n < ImGuiDir_Count_; n++)
		{
			const ImGuiDir dir = (n == -1) ? *last_dir : dir_prefered_order[n];
			if (n != -1 && dir == *last_dir) // Already tried this direction?
				continue;
			ImVec2 pos;
			if (dir == ImGuiDir_Down)  pos = ImVec2(r_avoid.Min.x, r_avoid.Max.y);          // Below, Toward Right (default)
			if (dir == ImGuiDir_Right) pos = ImVec2(r_avoid.Min.x, r_avoid.Min.y - size.y); // Above, Toward Right
			if (dir == ImGuiDir_Left)  pos = ImVec2(r_avoid.Max.x - size.x, r_avoid.Max.y); // Below, Toward Left
			if (dir == ImGuiDir_Up)    pos = ImVec2(r_avoid.Max.x - size.x, r_avoid.Min.y - size.y); // Above, Toward Left
			if (!r_outer.Contains(ImRect(pos, pos + size)))
				continue;
			*last_dir = dir;
			return pos;
		}
	}

	// Default popup policy
	const ImGuiDir dir_prefered_order[ImGuiDir_Count_] = { ImGuiDir_Right, ImGuiDir_Down, ImGuiDir_Up, ImGuiDir_Left };
	for (int n = (*last_dir != ImGuiDir_None) ? -1 : 0; n < ImGuiDir_Count_; n++)
	{
		const ImGuiDir dir = (n == -1) ? *last_dir : dir_prefered_order[n];
		if (n != -1 && dir == *last_dir) // Already tried this direction?
			continue;
		float avail_w = (dir == ImGuiDir_Left ? r_avoid.Min.x : r_outer.Max.x) - (dir == ImGuiDir_Right ? r_avoid.Max.x : r_outer.Min.x);
		float avail_h = (dir == ImGuiDir_Up ? r_avoid.Min.y : r_outer.Max.y) - (dir == ImGuiDir_Down ? r_avoid.Max.y : r_outer.Min.y);
		if (avail_w < size.x || avail_h < size.y)
			continue;
		ImVec2 pos;
		pos.x = (dir == ImGuiDir_Left) ? r_avoid.Min.x - size.x : (dir == ImGuiDir_Right) ? r_avoid.Max.x : base_pos_clamped.x;
		pos.y = (dir == ImGuiDir_Up) ? r_avoid.Min.y - size.y : (dir == ImGuiDir_Down) ? r_avoid.Max.y : base_pos_clamped.y;
		*last_dir = dir;
		return pos;
	}

	// Fallback, try to keep within display
	*last_dir = ImGuiDir_None;
	ImVec2 pos = ref_pos;
	pos.x = ImMax(ImMin(pos.x + size.x, r_outer.Max.x) - size.x, r_outer.Min.x);
	pos.y = ImMax(ImMin(pos.y + size.y, r_outer.Max.y) - size.y, r_outer.Min.y);
	return pos;
}

static void SetWindowConditionAllowFlags(ImGuiWindow* window, ImGuiCond flags, bool enabled)
{
	window->SetWindowPosAllowFlags = enabled ? (window->SetWindowPosAllowFlags | flags) : (window->SetWindowPosAllowFlags       & ~flags);
	window->SetWindowSizeAllowFlags = enabled ? (window->SetWindowSizeAllowFlags | flags) : (window->SetWindowSizeAllowFlags      & ~flags);
	window->SetWindowCollapsedAllowFlags = enabled ? (window->SetWindowCollapsedAllowFlags | flags) : (window->SetWindowCollapsedAllowFlags & ~flags);
}

ImGuiWindow* ImGui::FindWindowByName(const char* name)
{
	ImGuiContext& g = *GImGui;
	ImGuiID id = ImHash(name, 0);
	return (ImGuiWindow*)g.WindowsById.GetVoidPtr(id);
}

static ImGuiWindow* CreateNewWindow(const char* name, ImVec2 size, ImGuiWindowFlags flags)
{
	ImGuiContext& g = *GImGui;

	// Create window the first time
	ImGuiWindow* window = IM_NEW(ImGuiWindow)(&g, name);
	window->Flags = flags;
	g.WindowsById.SetVoidPtr(window->ID, window);

	// User can disable loading and saving of settings. Tooltip and child windows also don't store settings.
	if (!(flags & ImGuiWindowFlags_NoSavedSettings))
	{
		// Retrieve settings from .ini file
		// Use SetWindowPos() or SetNextWindowPos() with the appropriate condition flag to change the initial position of a window.
		window->Pos = window->PosFloat = ImVec2(60, 60);

		if (ImGuiWindowSettings* settings = ImGui::FindWindowSettings(window->ID))
		{
			SetWindowConditionAllowFlags(window, ImGuiCond_FirstUseEver, false);
			window->PosFloat = settings->Pos;
			window->Pos = ImFloor(window->PosFloat);
			window->Collapsed = settings->Collapsed;
			if (ImLengthSqr(settings->Size) > 0.00001f)
				size = settings->Size;
		}
	}
	window->Size = window->SizeFull = window->SizeFullAtLastBegin = size;

	if ((flags & ImGuiWindowFlags_AlwaysAutoResize) != 0)
	{
		window->AutoFitFramesX = window->AutoFitFramesY = 2;
		window->AutoFitOnlyGrows = false;
	}
	else
	{
		if (window->Size.x <= 0.0f)
			window->AutoFitFramesX = 2;
		if (window->Size.y <= 0.0f)
			window->AutoFitFramesY = 2;
		window->AutoFitOnlyGrows = (window->AutoFitFramesX > 0) || (window->AutoFitFramesY > 0);
	}

	if (flags & ImGuiWindowFlags_NoBringToFrontOnFocus)
		g.Windows.insert(g.Windows.begin(), window); // Quite slow but rare and only once
	else
		g.Windows.push_back(window);
	return window;
}

static ImVec2 CalcSizeAfterConstraint(ImGuiWindow* window, ImVec2 new_size)
{
	ImGuiContext& g = *GImGui;
	if (g.NextWindowData.SizeConstraintCond != 0)
	{
		// Using -1,-1 on either X/Y axis to preserve the current size.
		ImRect cr = g.NextWindowData.SizeConstraintRect;
		new_size.x = (cr.Min.x >= 0 && cr.Max.x >= 0) ? ImClamp(new_size.x, cr.Min.x, cr.Max.x) : window->SizeFull.x;
		new_size.y = (cr.Min.y >= 0 && cr.Max.y >= 0) ? ImClamp(new_size.y, cr.Min.y, cr.Max.y) : window->SizeFull.y;
		if (g.NextWindowData.SizeCallback)
		{
			ImGuiSizeCallbackData data;
			data.UserData = g.NextWindowData.SizeCallbackUserData;
			data.Pos = window->Pos;
			data.CurrentSize = window->SizeFull;
			data.DesiredSize = new_size;
			g.NextWindowData.SizeCallback(&data);
			new_size = data.DesiredSize;
		}
	}

	// Minimum size
	if (!(window->Flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_AlwaysAutoResize)))
	{
		new_size = ImMax(new_size, g.Style.WindowMinSize);
		new_size.y = ImMax(new_size.y, window->TitleBarHeight() + window->MenuBarHeight() + ImMax(0.0f, g.Style.WindowRounding - 1.0f)); // Reduce artifacts with very small windows
	}
	return new_size;
}

static ImVec2 CalcSizeContents(ImGuiWindow* window)
{
	ImVec2 sz;
	sz.x = (float)(int)((window->SizeContentsExplicit.x != 0.0f) ? window->SizeContentsExplicit.x : (window->DC.CursorMaxPos.x - window->Pos.x + window->Scroll.x));
	sz.y = (float)(int)((window->SizeContentsExplicit.y != 0.0f) ? window->SizeContentsExplicit.y : (window->DC.CursorMaxPos.y - window->Pos.y + window->Scroll.y));
	return sz + window->WindowPadding;
}

static ImVec2 CalcSizeAutoFit(ImGuiWindow* window, const ImVec2& size_contents)
{
	ImGuiContext& g = *GImGui;
	ImGuiStyle& style = g.Style;
	ImGuiWindowFlags flags = window->Flags;
	ImVec2 size_auto_fit;
	if ((flags & ImGuiWindowFlags_Tooltip) != 0)
	{
		// Tooltip always resize. We keep the spacing symmetric on both axises for aesthetic purpose.
		size_auto_fit = size_contents;
	}
	else
	{
		// When the window cannot fit all contents (either because of constraints, either because screen is too small): we are growing the size on the other axis to compensate for expected scrollbar. FIXME: Might turn bigger than DisplaySize-WindowPadding.
		size_auto_fit = ImClamp(size_contents, style.WindowMinSize, ImMax(style.WindowMinSize, g.IO.DisplaySize - g.Style.DisplaySafeAreaPadding));
		ImVec2 size_auto_fit_after_constraint = CalcSizeAfterConstraint(window, size_auto_fit);
		if (size_auto_fit_after_constraint.x < size_contents.x && !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar))
			size_auto_fit.y += style.ScrollbarSize;
		if (size_auto_fit_after_constraint.y < size_contents.y && !(flags & ImGuiWindowFlags_NoScrollbar))
			size_auto_fit.x += style.ScrollbarSize;
	}
	return size_auto_fit;
}

static float GetScrollMaxX(ImGuiWindow* window)
{
	return ImMax(0.0f, window->SizeContents.x - (window->SizeFull.x - window->ScrollbarSizes.x));
}

static float GetScrollMaxY(ImGuiWindow* window)
{
	return ImMax(0.0f, window->SizeContents.y - (window->SizeFull.y - window->ScrollbarSizes.y));
}

static ImVec2 CalcNextScrollFromScrollTargetAndClamp(ImGuiWindow* window)
{
	ImVec2 scroll = window->Scroll;
	float cr_x = window->ScrollTargetCenterRatio.x;
	float cr_y = window->ScrollTargetCenterRatio.y;
	if (window->ScrollTarget.x < FLT_MAX)
		scroll.x = window->ScrollTarget.x - cr_x * (window->SizeFull.x - window->ScrollbarSizes.x);
	if (window->ScrollTarget.y < FLT_MAX)
		scroll.y = window->ScrollTarget.y - (1.0f - cr_y) * (window->TitleBarHeight() + window->MenuBarHeight()) - cr_y * (window->SizeFull.y - window->ScrollbarSizes.y);
	scroll = ImMax(scroll, ImVec2(0.0f, 0.0f));
	if (!window->Collapsed && !window->SkipItems)
	{
		scroll.x = ImMin(scroll.x, GetScrollMaxX(window));
		scroll.y = ImMin(scroll.y, GetScrollMaxY(window));
	}
	return scroll;
}

static ImGuiCol GetWindowBgColorIdxFromFlags(ImGuiWindowFlags flags)
{
	if (flags & (ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_Popup))
		return ImGuiCol_PopupBg;
	if (flags & ImGuiWindowFlags_ChildWindow)
		return ImGuiCol_ChildBg;
	return ImGuiCol_WindowBg;
}

static void CalcResizePosSizeFromAnyCorner(ImGuiWindow* window, const ImVec2& corner_target, const ImVec2& corner_norm, ImVec2* out_pos, ImVec2* out_size)
{
	ImVec2 pos_min = ImLerp(corner_target, window->Pos, corner_norm);                // Expected window upper-left
	ImVec2 pos_max = ImLerp(window->Pos + window->Size, corner_target, corner_norm); // Expected window lower-right
	ImVec2 size_expected = pos_max - pos_min;
	ImVec2 size_constrained = CalcSizeAfterConstraint(window, size_expected);
	*out_pos = pos_min;
	if (corner_norm.x == 0.0f)
		out_pos->x -= (size_constrained.x - size_expected.x);
	if (corner_norm.y == 0.0f)
		out_pos->y -= (size_constrained.y - size_expected.y);
	*out_size = size_constrained;
}

struct ImGuiResizeGripDef
{
	ImVec2           CornerPos;
	ImVec2           InnerDir;
	int              AngleMin12, AngleMax12;
};

const ImGuiResizeGripDef resize_grip_def[4] =
{
	{ ImVec2(1,1), ImVec2(-1,-1), 0, 3 }, // Lower right
	{ ImVec2(0,1), ImVec2(+1,-1), 3, 6 }, // Lower left
	{ ImVec2(0,0), ImVec2(+1,+1), 6, 9 }, // Upper left
	{ ImVec2(1,0), ImVec2(-1,+1), 9,12 }, // Upper right
};

static ImRect GetBorderRect(ImGuiWindow* window, int border_n, float perp_padding, float thickness)
{
	ImRect rect = window->Rect();
	if (thickness == 0.0f) rect.Max -= ImVec2(1, 1);
	if (border_n == 0) return ImRect(rect.Min.x + perp_padding, rect.Min.y, rect.Max.x - perp_padding, rect.Min.y + thickness);
	if (border_n == 1) return ImRect(rect.Max.x - thickness, rect.Min.y + perp_padding, rect.Max.x, rect.Max.y - perp_padding);
	if (border_n == 2) return ImRect(rect.Min.x + perp_padding, rect.Max.y - thickness, rect.Max.x - perp_padding, rect.Max.y);
	if (border_n == 3) return ImRect(rect.Min.x, rect.Min.y + perp_padding, rect.Min.x + thickness, rect.Max.y - perp_padding);
	IM_ASSERT(0);
	return ImRect();
}

// Handle resize for: Resize Grips, Borders, Gamepad
static void ImGui::UpdateManualResize(ImGuiWindow* window, const ImVec2& size_auto_fit, int* border_held, int resize_grip_count, ImU32 resize_grip_col[4])
{
	ImGuiContext& g = *GImGui;
	ImGuiWindowFlags flags = window->Flags;
	if ((flags & ImGuiWindowFlags_NoResize) || (flags & ImGuiWindowFlags_AlwaysAutoResize) || window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
		return;

	const int resize_border_count = (flags & ImGuiWindowFlags_ResizeFromAnySide) ? 4 : 0;
	const float grip_draw_size = (float)(int)ImMax(g.FontSize * 1.35f, window->WindowRounding + 1.0f + g.FontSize * 0.2f);
	const float grip_hover_size = (float)(int)(grip_draw_size * 0.75f);

	ImVec2 pos_target(FLT_MAX, FLT_MAX);
	ImVec2 size_target(FLT_MAX, FLT_MAX);

	// Manual resize grips
	PushID("#RESIZE");
	for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
	{
		const ImGuiResizeGripDef& grip = resize_grip_def[resize_grip_n];
		const ImVec2 corner = ImLerp(window->Pos, window->Pos + window->Size, grip.CornerPos);

		// Using the FlattenChilds button flag we make the resize button accessible even if we are hovering over a child window
		ImRect resize_rect(corner, corner + grip.InnerDir * grip_hover_size);
		resize_rect.FixInverted();
		bool hovered, held;
		ButtonBehavior(resize_rect, window->GetID((void*)(intptr_t)resize_grip_n), &hovered, &held, ImGuiButtonFlags_FlattenChildren);
		if (hovered || held)
			g.MouseCursor = (resize_grip_n & 1) ? ImGuiMouseCursor_ResizeNESW : ImGuiMouseCursor_ResizeNWSE;

		if (g.HoveredWindow == window && held && g.IO.MouseDoubleClicked[0] && resize_grip_n == 0)
		{
			// Manual auto-fit when double-clicking
			size_target = CalcSizeAfterConstraint(window, size_auto_fit);
			ClearActiveID();
		}
		else if (held)
		{
			// Resize from any of the four corners
			// We don't use an incremental MouseDelta but rather compute an absolute target size based on mouse position
			ImVec2 corner_target = g.IO.MousePos - g.ActiveIdClickOffset + resize_rect.GetSize() * grip.CornerPos; // Corner of the window corresponding to our corner grip
			CalcResizePosSizeFromAnyCorner(window, corner_target, grip.CornerPos, &pos_target, &size_target);
		}
		if (resize_grip_n == 0 || held || hovered)
			resize_grip_col[resize_grip_n] = GetColorU32(held ? ImGuiCol_ResizeGripActive : hovered ? ImGuiCol_ResizeGripHovered : ImGuiCol_ResizeGrip);
	}
	for (int border_n = 0; border_n < resize_border_count; border_n++)
	{
		const float BORDER_SIZE = 5.0f;          // FIXME: Only works _inside_ window because of HoveredWindow check.
		const float BORDER_APPEAR_TIMER = 0.05f; // Reduce visual noise
		bool hovered, held;
		ImRect border_rect = GetBorderRect(window, border_n, grip_hover_size, BORDER_SIZE);
		ButtonBehavior(border_rect, window->GetID((void*)(intptr_t)(border_n + 4)), &hovered, &held, ImGuiButtonFlags_FlattenChildren);
		if ((hovered && g.HoveredIdTimer > BORDER_APPEAR_TIMER) || held)
		{
			g.MouseCursor = (border_n & 1) ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS;
			if (held) *border_held = border_n;
		}
		if (held)
		{
			ImVec2 border_target = window->Pos;
			ImVec2 border_posn;
			if (border_n == 0) { border_posn = ImVec2(0, 0); border_target.y = (g.IO.MousePos.y - g.ActiveIdClickOffset.y); }
			if (border_n == 1) { border_posn = ImVec2(1, 0); border_target.x = (g.IO.MousePos.x - g.ActiveIdClickOffset.x + BORDER_SIZE); }
			if (border_n == 2) { border_posn = ImVec2(0, 1); border_target.y = (g.IO.MousePos.y - g.ActiveIdClickOffset.y + BORDER_SIZE); }
			if (border_n == 3) { border_posn = ImVec2(0, 0); border_target.x = (g.IO.MousePos.x - g.ActiveIdClickOffset.x); }
			CalcResizePosSizeFromAnyCorner(window, border_target, border_posn, &pos_target, &size_target);
		}
	}
	PopID();

	// Apply back modified position/size to window
	if (size_target.x != FLT_MAX)
	{
		window->SizeFull = size_target;
		MarkIniSettingsDirty(window);
	}
	if (pos_target.x != FLT_MAX)
	{
		window->Pos = window->PosFloat = ImFloor(pos_target);
		MarkIniSettingsDirty(window);
	}

	window->Size = window->SizeFull;
}

// Push a new ImGui window to add widgets to.
// - A default window called "Debug" is automatically stacked at the beginning of every frame so you can use widgets without explicitly calling a Begin/End pair.
// - Begin/End can be called multiple times during the frame with the same window name to append content.
// - The window name is used as a unique identifier to preserve window information across frames (and save rudimentary information to the .ini file).
//   You can use the "##" or "###" markers to use the same label with different id, or same id with different label. See documentation at the top of this file.
// - Return false when window is collapsed, so you can early out in your code. You always need to call ImGui::End() even if false is returned.
// - Passing 'bool* p_open' displays a Close button on the upper-right corner of the window, the pointed value will be set to false when the button is pressed.
bool ImGui::Begin(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	IM_ASSERT(name != NULL);                        // Window name required
	IM_ASSERT(g.Initialized);                       // Forgot to call ImGui::NewFrame()
	IM_ASSERT(g.FrameCountEnded != g.FrameCount);   // Called ImGui::Render() or ImGui::EndFrame() and haven't called ImGui::NewFrame() again yet

													// Find or create
	ImGuiWindow* window = FindWindowByName(name);
	if (!window)
	{
		ImVec2 size_on_first_use = (g.NextWindowData.SizeCond != 0) ? g.NextWindowData.SizeVal : ImVec2(0.0f, 0.0f); // Any condition flag will do since we are creating a new window here.
		window = CreateNewWindow(name, size_on_first_use, flags);
	}

	// Automatically disable manual moving/resizing when NoInputs is set
	if (flags & ImGuiWindowFlags_NoInputs)
		flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
	//if (flags & ImGuiWindowFlags_NavFlattened)
	//    IM_ASSERT(flags & ImGuiWindowFlags_ChildWindow);

	const int current_frame = g.FrameCount;
	const bool first_begin_of_the_frame = (window->LastFrameActive != current_frame);
	if (first_begin_of_the_frame)
		window->Flags = (ImGuiWindowFlags)flags;
	else
		flags = window->Flags;

	// Update the Appearing flag
	bool window_just_activated_by_user = (window->LastFrameActive < current_frame - 1);   // Not using !WasActive because the implicit "Debug" window would always toggle off->on
	const bool window_just_appearing_after_hidden_for_resize = (window->HiddenFrames == 1);
	if (flags & ImGuiWindowFlags_Popup)
	{
		ImGuiPopupRef& popup_ref = g.OpenPopupStack[g.CurrentPopupStack.Size];
		window_just_activated_by_user |= (window->PopupId != popup_ref.PopupId); // We recycle popups so treat window as activated if popup id changed
		window_just_activated_by_user |= (window != popup_ref.Window);
	}
	window->Appearing = (window_just_activated_by_user || window_just_appearing_after_hidden_for_resize);

	window->CloseButton = (p_open != NULL);
	if (window->Appearing)
		SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, true);

	// Parent window is latched only on the first call to Begin() of the frame, so further append-calls can be done from a different window stack
	ImGuiWindow* parent_window_in_stack = g.CurrentWindowStack.empty() ? NULL : g.CurrentWindowStack.back();
	ImGuiWindow* parent_window = first_begin_of_the_frame ? ((flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Popup)) ? parent_window_in_stack : NULL) : window->ParentWindow;
	IM_ASSERT(parent_window != NULL || !(flags & ImGuiWindowFlags_ChildWindow));

	// Add to stack
	g.CurrentWindowStack.push_back(window);
	SetCurrentWindow(window);
	CheckStacksSize(window, true);
	if (flags & ImGuiWindowFlags_Popup)
	{
		ImGuiPopupRef& popup_ref = g.OpenPopupStack[g.CurrentPopupStack.Size];
		popup_ref.Window = window;
		g.CurrentPopupStack.push_back(popup_ref);
		window->PopupId = popup_ref.PopupId;
	}

	// Process SetNextWindow***() calls
	bool window_pos_set_by_api = false;
	bool window_size_x_set_by_api = false, window_size_y_set_by_api = false;
	if (g.NextWindowData.PosCond)
	{
		window_pos_set_by_api = (window->SetWindowPosAllowFlags & g.NextWindowData.PosCond) != 0;
		if (window_pos_set_by_api && ImLengthSqr(g.NextWindowData.PosPivotVal) > 0.00001f)
		{
			// May be processed on the next frame if this is our first frame and we are measuring size
			// FIXME: Look into removing the branch so everything can go through this same code path for consistency.
			window->SetWindowPosVal = g.NextWindowData.PosVal;
			window->SetWindowPosPivot = g.NextWindowData.PosPivotVal;
			window->SetWindowPosAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);
		}
		else
		{
			SetWindowPos(window, g.NextWindowData.PosVal, g.NextWindowData.PosCond);
		}
		g.NextWindowData.PosCond = 0;
	}
	if (g.NextWindowData.SizeCond)
	{
		window_size_x_set_by_api = (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.x > 0.0f);
		window_size_y_set_by_api = (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.y > 0.0f);
		SetWindowSize(window, g.NextWindowData.SizeVal, g.NextWindowData.SizeCond);
		g.NextWindowData.SizeCond = 0;
	}
	if (g.NextWindowData.ContentSizeCond)
	{
		// Adjust passed "client size" to become a "window size"
		window->SizeContentsExplicit = g.NextWindowData.ContentSizeVal;
		if (window->SizeContentsExplicit.y != 0.0f)
			window->SizeContentsExplicit.y += window->TitleBarHeight() + window->MenuBarHeight();
		g.NextWindowData.ContentSizeCond = 0;
	}
	else if (first_begin_of_the_frame)
	{
		window->SizeContentsExplicit = ImVec2(0.0f, 0.0f);
	}
	if (g.NextWindowData.CollapsedCond)
	{
		SetWindowCollapsed(window, g.NextWindowData.CollapsedVal, g.NextWindowData.CollapsedCond);
		g.NextWindowData.CollapsedCond = 0;
	}
	if (g.NextWindowData.FocusCond)
	{
		SetWindowFocus();
		g.NextWindowData.FocusCond = 0;
	}
	if (window->Appearing)
		SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, false);

	// When reusing window again multiple times a frame, just append content (don't need to setup again)
	if (first_begin_of_the_frame)
	{
		const bool window_is_child_tooltip = (flags & ImGuiWindowFlags_ChildWindow) && (flags & ImGuiWindowFlags_Tooltip); // FIXME-WIP: Undocumented behavior of Child+Tooltip for pinned tooltip (#1345)

																														   // Initialize
		window->ParentWindow = parent_window;
		window->RootWindow = window->RootNonPopupWindow = window;
		if (parent_window && (flags & ImGuiWindowFlags_ChildWindow) && !window_is_child_tooltip)
			window->RootWindow = parent_window->RootWindow;
		if (parent_window && !(flags & ImGuiWindowFlags_Modal) && (flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Popup)))
			window->RootNonPopupWindow = parent_window->RootNonPopupWindow;
		//window->RootNavWindow = window;
		//while (window->RootNavWindow->Flags & ImGuiWindowFlags_NavFlattened)
		//    window->RootNavWindow = window->RootNavWindow->ParentWindow;

		window->Active = true;
		window->BeginOrderWithinParent = 0;
		window->BeginOrderWithinContext = g.WindowsActiveCount++;
		window->BeginCount = 0;
		window->ClipRect = ImVec4(-FLT_MAX, -FLT_MAX, +FLT_MAX, +FLT_MAX);
		window->LastFrameActive = current_frame;
		window->IDStack.resize(1);

		// Lock window rounding, border size and rounding so that altering the border sizes for children doesn't have side-effects.
		window->WindowRounding = (flags & ImGuiWindowFlags_ChildWindow) ? style.ChildRounding : ((flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiWindowFlags_Modal)) ? style.PopupRounding : style.WindowRounding;
		window->WindowBorderSize = (flags & ImGuiWindowFlags_ChildWindow) ? style.ChildBorderSize : ((flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiWindowFlags_Modal)) ? style.PopupBorderSize : style.WindowBorderSize;
		window->WindowPadding = style.WindowPadding;
		if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & (ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_Popup)) && window->WindowBorderSize == 0.0f)
			window->WindowPadding = ImVec2(0.0f, (flags & ImGuiWindowFlags_MenuBar) ? style.WindowPadding.y : 0.0f);

		// Collapse window by double-clicking on title bar
		// At this point we don't have a clipping rectangle setup yet, so we can use the title bar area for hit detection and drawing
		if (!(flags & ImGuiWindowFlags_NoTitleBar) && !(flags & ImGuiWindowFlags_NoCollapse))
		{
			ImRect title_bar_rect = window->TitleBarRect();
			if (g.HoveredWindow == window && IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max) && g.IO.MouseDoubleClicked[0])
			{
				window->Collapsed = !window->Collapsed;
				MarkIniSettingsDirty(window);
				FocusWindow(window);
			}
		}
		else
		{
			window->Collapsed = false;
		}

		// SIZE

		// Update contents size from last frame for auto-fitting (unless explicitly specified)
		window->SizeContents = CalcSizeContents(window);

		// Hide popup/tooltip window when re-opening while we measure size (because we recycle the windows)
		if (window->HiddenFrames > 0)
			window->HiddenFrames--;
		if ((flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) != 0 && window_just_activated_by_user)
		{
			window->HiddenFrames = 1;
			if (flags & ImGuiWindowFlags_AlwaysAutoResize)
			{
				if (!window_size_x_set_by_api)
					window->Size.x = window->SizeFull.x = 0.f;
				if (!window_size_y_set_by_api)
					window->Size.y = window->SizeFull.y = 0.f;
				window->SizeContents = ImVec2(0.f, 0.f);
			}
		}

		// Calculate auto-fit size, handle automatic resize
		const ImVec2 size_auto_fit = CalcSizeAutoFit(window, window->SizeContents);
		ImVec2 size_full_modified(FLT_MAX, FLT_MAX);
		if (flags & ImGuiWindowFlags_AlwaysAutoResize && !window->Collapsed)
		{
			// Using SetNextWindowSize() overrides ImGuiWindowFlags_AlwaysAutoResize, so it can be used on tooltips/popups, etc.
			if (!window_size_x_set_by_api)
				window->SizeFull.x = size_full_modified.x = size_auto_fit.x;
			if (!window_size_y_set_by_api)
				window->SizeFull.y = size_full_modified.y = size_auto_fit.y;
		}
		else if (window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
		{
			// Auto-fit only grows during the first few frames
			// We still process initial auto-fit on collapsed windows to get a window width, but otherwise don't honor ImGuiWindowFlags_AlwaysAutoResize when collapsed.
			if (!window_size_x_set_by_api && window->AutoFitFramesX > 0)
				window->SizeFull.x = size_full_modified.x = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.x, size_auto_fit.x) : size_auto_fit.x;
			if (!window_size_y_set_by_api && window->AutoFitFramesY > 0)
				window->SizeFull.y = size_full_modified.y = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.y, size_auto_fit.y) : size_auto_fit.y;
			if (!window->Collapsed)
				MarkIniSettingsDirty(window);
		}

		// Apply minimum/maximum window size constraints and final size
		window->SizeFull = CalcSizeAfterConstraint(window, window->SizeFull);
		window->Size = window->Collapsed && !(flags & ImGuiWindowFlags_ChildWindow) ? window->TitleBarRect().GetSize() : window->SizeFull;

		// SCROLLBAR STATUS

		// Update scrollbar status (based on the Size that was effective during last frame or the auto-resized Size). 
		if (!window->Collapsed)
		{
			// When reading the current size we need to read it after size constraints have been applied
			float size_x_for_scrollbars = size_full_modified.x != FLT_MAX ? window->SizeFull.x : window->SizeFullAtLastBegin.x;
			float size_y_for_scrollbars = size_full_modified.y != FLT_MAX ? window->SizeFull.y : window->SizeFullAtLastBegin.y;
			window->ScrollbarY = (flags & ImGuiWindowFlags_AlwaysVerticalScrollbar) || ((window->SizeContents.y > size_y_for_scrollbars) && !(flags & ImGuiWindowFlags_NoScrollbar));
			window->ScrollbarX = (flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar) || ((window->SizeContents.x > size_x_for_scrollbars - (window->ScrollbarY ? style.ScrollbarSize : 0.0f)) && !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar));
			if (window->ScrollbarX && !window->ScrollbarY)
				window->ScrollbarY = (window->SizeContents.y > size_y_for_scrollbars - style.ScrollbarSize) && !(flags & ImGuiWindowFlags_NoScrollbar);
			window->ScrollbarSizes = ImVec2(window->ScrollbarY ? style.ScrollbarSize : 0.0f, window->ScrollbarX ? style.ScrollbarSize : 0.0f);
		}

		// POSITION

		// Popup latch its initial position, will position itself when it appears next frame
		if (window_just_activated_by_user)
		{
			window->AutoPosLastDirection = ImGuiDir_None;
			if ((flags & ImGuiWindowFlags_Popup) != 0 && !window_pos_set_by_api)
				window->Pos = window->PosFloat = g.CurrentPopupStack.back().OpenPopupPos;
		}

		// Position child window
		if (flags & ImGuiWindowFlags_ChildWindow)
		{
			window->BeginOrderWithinParent = parent_window->DC.ChildWindows.Size;
			parent_window->DC.ChildWindows.push_back(window);
			if (!(flags & ImGuiWindowFlags_Popup) && !window_pos_set_by_api && !window_is_child_tooltip)
				window->Pos = window->PosFloat = parent_window->DC.CursorPos;
		}

		const bool window_pos_with_pivot = (window->SetWindowPosVal.x != FLT_MAX && window->HiddenFrames == 0);
		if (window_pos_with_pivot)
		{
			// Position given a pivot (e.g. for centering)
			SetWindowPos(window, ImMax(style.DisplaySafeAreaPadding, window->SetWindowPosVal - window->SizeFull * window->SetWindowPosPivot), 0);
		}
		else if (flags & ImGuiWindowFlags_ChildMenu)
		{
			// Child menus typically request _any_ position within the parent menu item, and then our FindBestPopupWindowPos() function will move the new menu outside the parent bounds.
			// This is how we end up with child menus appearing (most-commonly) on the right of the parent menu.
			IM_ASSERT(window_pos_set_by_api);
			float horizontal_overlap = style.ItemSpacing.x; // We want some overlap to convey the relative depth of each popup (currently the amount of overlap it is hard-coded to style.ItemSpacing.x, may need to introduce another style value).
			ImGuiWindow* parent_menu = parent_window_in_stack;
			ImRect rect_to_avoid;
			if (parent_menu->DC.MenuBarAppending)
				rect_to_avoid = ImRect(-FLT_MAX, parent_menu->Pos.y + parent_menu->TitleBarHeight(), FLT_MAX, parent_menu->Pos.y + parent_menu->TitleBarHeight() + parent_menu->MenuBarHeight());
			else
				rect_to_avoid = ImRect(parent_menu->Pos.x + horizontal_overlap, -FLT_MAX, parent_menu->Pos.x + parent_menu->Size.x - horizontal_overlap - parent_menu->ScrollbarSizes.x, FLT_MAX);
			window->PosFloat = FindBestWindowPosForPopup(window->PosFloat, window->Size, &window->AutoPosLastDirection, rect_to_avoid);
		}
		else if ((flags & ImGuiWindowFlags_Popup) != 0 && !window_pos_set_by_api && window_just_appearing_after_hidden_for_resize)
		{
			ImRect rect_to_avoid(window->PosFloat.x - 1, window->PosFloat.y - 1, window->PosFloat.x + 1, window->PosFloat.y + 1);
			window->PosFloat = FindBestWindowPosForPopup(window->PosFloat, window->Size, &window->AutoPosLastDirection, rect_to_avoid);
		}

		// Position tooltip (always follows mouse)
		if ((flags & ImGuiWindowFlags_Tooltip) != 0 && !window_pos_set_by_api && !window_is_child_tooltip)
		{
			ImVec2 ref_pos = g.IO.MousePos;
			ImRect rect_to_avoid(ref_pos.x - 16, ref_pos.y - 8, ref_pos.x + 24, ref_pos.y + 24); // FIXME: Completely hard-coded. Store boxes in mouse cursor data? Scale? Center on cursor hit-point?
			window->PosFloat = FindBestWindowPosForPopup(ref_pos, window->Size, &window->AutoPosLastDirection, rect_to_avoid);
			if (window->AutoPosLastDirection == ImGuiDir_None)
				window->PosFloat = ref_pos + ImVec2(2, 2); // If there's not enough room, for tooltip we prefer avoiding the cursor at all cost even if it means that part of the tooltip won't be visible.
		}

		// Clamp position so it stays visible
		if (!(flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Tooltip))
		{
			if (!window_pos_set_by_api && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0 && g.IO.DisplaySize.x > 0.0f && g.IO.DisplaySize.y > 0.0f) // Ignore zero-sized display explicitly to avoid losing positions if a window manager reports zero-sized window when initializing or minimizing.
			{
				ImVec2 padding = ImMax(style.DisplayWindowPadding, style.DisplaySafeAreaPadding);
				window->PosFloat = ImMax(window->PosFloat + window->Size, padding) - window->Size;
				window->PosFloat = ImMin(window->PosFloat, g.IO.DisplaySize - padding);
			}
		}
		window->Pos = ImFloor(window->PosFloat);

		// Default item width. Make it proportional to window size if window manually resizes
		if (window->Size.x > 0.0f && !(flags & ImGuiWindowFlags_Tooltip) && !(flags & ImGuiWindowFlags_AlwaysAutoResize))
			window->ItemWidthDefault = (float)(int)(window->Size.x * 0.65f);
		else
			window->ItemWidthDefault = (float)(int)(g.FontSize * 16.0f);

		// Prepare for focus requests
		window->FocusIdxAllRequestCurrent = (window->FocusIdxAllRequestNext == INT_MAX || window->FocusIdxAllCounter == -1) ? INT_MAX : (window->FocusIdxAllRequestNext + (window->FocusIdxAllCounter + 1)) % (window->FocusIdxAllCounter + 1);
		window->FocusIdxTabRequestCurrent = (window->FocusIdxTabRequestNext == INT_MAX || window->FocusIdxTabCounter == -1) ? INT_MAX : (window->FocusIdxTabRequestNext + (window->FocusIdxTabCounter + 1)) % (window->FocusIdxTabCounter + 1);
		window->FocusIdxAllCounter = window->FocusIdxTabCounter = -1;
		window->FocusIdxAllRequestNext = window->FocusIdxTabRequestNext = INT_MAX;

		// Apply scrolling
		window->Scroll = CalcNextScrollFromScrollTargetAndClamp(window);
		window->ScrollTarget = ImVec2(FLT_MAX, FLT_MAX);

		// Apply focus, new windows appears in front
		bool want_focus = false;
		if (window_just_activated_by_user && !(flags & ImGuiWindowFlags_NoFocusOnAppearing))
			if (!(flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Tooltip)) || (flags & ImGuiWindowFlags_Popup))
				want_focus = true;

		// Handle manual resize: Resize Grips, Borders, Gamepad
		int border_held = -1;
		ImU32 resize_grip_col[4] = { 0 };
		const int resize_grip_count = (flags & ImGuiWindowFlags_ResizeFromAnySide) ? 2 : 1; // 4
		const float grip_draw_size = (float)(int)ImMax(g.FontSize * 1.35f, window->WindowRounding + 1.0f + g.FontSize * 0.2f);
		if (!window->Collapsed)
			UpdateManualResize(window, size_auto_fit, &border_held, resize_grip_count, &resize_grip_col[0]);

		// DRAWING

		// Setup draw list and outer clipping rectangle
		window->DrawList->Clear();
		window->DrawList->Flags = (g.Style.AntiAliasedLines ? ImDrawListFlags_AntiAliasedLines : 0) | (g.Style.AntiAliasedFill ? ImDrawListFlags_AntiAliasedFill : 0);
		window->DrawList->PushTextureID(g.Font->ContainerAtlas->TexID);
		ImRect viewport_rect(GetViewportRect());
		if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !window_is_child_tooltip)
			PushClipRect(parent_window->ClipRect.Min, parent_window->ClipRect.Max, true);
		else
			PushClipRect(viewport_rect.Min, viewport_rect.Max, true);

		// Draw modal window background (darkens what is behind them)
		if ((flags & ImGuiWindowFlags_Modal) != 0 && window == GetFrontMostModalRootWindow())
			window->DrawList->AddRectFilled(viewport_rect.Min, viewport_rect.Max, GetColorU32(ImGuiCol_ModalWindowDarkening, g.ModalWindowDarkeningRatio));

		// Draw window + handle manual resize
		const float window_rounding = window->WindowRounding;
		const float window_border_size = window->WindowBorderSize;
		ImRect title_bar_rect = window->TitleBarRect();
		const bool window_is_focused = want_focus || (g.NavWindow && window->RootNonPopupWindow == g.NavWindow->RootNonPopupWindow);
		if (window->Collapsed)
		{
			// Title bar only
			float backup_border_size = style.FrameBorderSize;
			g.Style.FrameBorderSize = window->WindowBorderSize;
			RenderFrame(title_bar_rect.Min, title_bar_rect.Max, GetColorU32(ImGuiCol_TitleBgCollapsed), true, window_rounding);
			g.Style.FrameBorderSize = backup_border_size;
		}
		else
		{
			// Window background
			ImU32 bg_col = GetColorU32(GetWindowBgColorIdxFromFlags(flags));
			if (g.NextWindowData.BgAlphaCond != 0)
			{
				bg_col = (bg_col & ~IM_COL32_A_MASK) | (IM_F32_TO_INT8_SAT(g.NextWindowData.BgAlphaVal) << IM_COL32_A_SHIFT);
				g.NextWindowData.BgAlphaCond = 0;
			}
			window->DrawList->AddRectFilled(window->Pos + ImVec2(0, window->TitleBarHeight()), window->Pos + window->Size, bg_col, window_rounding, (flags & ImGuiWindowFlags_NoTitleBar) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Bot);

			// Title bar
			if (!(flags & ImGuiWindowFlags_NoTitleBar))
				window->DrawList->AddRectFilled(title_bar_rect.Min, title_bar_rect.Max, GetColorU32(window_is_focused ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBg), window_rounding, ImDrawCornerFlags_Top);

			// Menu bar
			if (flags & ImGuiWindowFlags_MenuBar)
			{
				ImRect menu_bar_rect = window->MenuBarRect();
				menu_bar_rect.ClipWith(window->Rect());  // Soft clipping, in particular child window don't have minimum size covering the menu bar so this is useful for them.
				window->DrawList->AddRectFilled(menu_bar_rect.Min, menu_bar_rect.Max, GetColorU32(ImGuiCol_MenuBarBg), (flags & ImGuiWindowFlags_NoTitleBar) ? window_rounding : 0.0f, ImDrawCornerFlags_Top);
				if (style.FrameBorderSize > 0.0f && menu_bar_rect.Max.y < window->Pos.y + window->Size.y)
					window->DrawList->AddLine(menu_bar_rect.GetBL(), menu_bar_rect.GetBR(), GetColorU32(ImGuiCol_Border), style.FrameBorderSize);
			}

			// Scrollbars
			if (window->ScrollbarX)
				Scrollbar(ImGuiLayoutType_Horizontal);
			if (window->ScrollbarY)
				Scrollbar(ImGuiLayoutType_Vertical);

			// Render resize grips (after their input handling so we don't have a frame of latency)
			if (!(flags & ImGuiWindowFlags_NoResize))
			{
				for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
				{
					const ImGuiResizeGripDef& grip = resize_grip_def[resize_grip_n];
					const ImVec2 corner = ImLerp(window->Pos, window->Pos + window->Size, grip.CornerPos);
					window->DrawList->PathLineTo(corner + grip.InnerDir * ((resize_grip_n & 1) ? ImVec2(window_border_size, grip_draw_size) : ImVec2(grip_draw_size, window_border_size)));
					window->DrawList->PathLineTo(corner + grip.InnerDir * ((resize_grip_n & 1) ? ImVec2(grip_draw_size, window_border_size) : ImVec2(window_border_size, grip_draw_size)));
					window->DrawList->PathArcToFast(ImVec2(corner.x + grip.InnerDir.x * (window_rounding + window_border_size), corner.y + grip.InnerDir.y * (window_rounding + window_border_size)), window_rounding, grip.AngleMin12, grip.AngleMax12);
					window->DrawList->PathFillConvex(resize_grip_col[resize_grip_n]);
				}
			}

			// Borders
			if (window_border_size > 0.0f)
				window->DrawList->AddRect(window->Pos, window->Pos + window->Size, GetColorU32(ImGuiCol_Border), window_rounding, ImDrawCornerFlags_All, window_border_size);
			if (border_held != -1)
			{
				ImRect border = GetBorderRect(window, border_held, grip_draw_size, 0.0f);
				window->DrawList->AddLine(border.Min, border.Max, GetColorU32(ImGuiCol_SeparatorActive), ImMax(1.0f, window_border_size));
			}
			if (style.FrameBorderSize > 0 && !(flags & ImGuiWindowFlags_NoTitleBar))
				window->DrawList->AddLine(title_bar_rect.GetBL() + ImVec2(style.WindowBorderSize, -1), title_bar_rect.GetBR() + ImVec2(-style.WindowBorderSize, -1), GetColorU32(ImGuiCol_Border), style.FrameBorderSize);
		}

		// Store a backup of SizeFull which we will use next frame to decide if we need scrollbars. 
		window->SizeFullAtLastBegin = window->SizeFull;

		// Update ContentsRegionMax. All the variable it depends on are set above in this function.
		window->ContentsRegionRect.Min.x = -window->Scroll.x + window->WindowPadding.x;
		window->ContentsRegionRect.Min.y = -window->Scroll.y + window->WindowPadding.y + window->TitleBarHeight() + window->MenuBarHeight();
		window->ContentsRegionRect.Max.x = -window->Scroll.x - window->WindowPadding.x + (window->SizeContentsExplicit.x != 0.0f ? window->SizeContentsExplicit.x : (window->Size.x - window->ScrollbarSizes.x));
		window->ContentsRegionRect.Max.y = -window->Scroll.y - window->WindowPadding.y + (window->SizeContentsExplicit.y != 0.0f ? window->SizeContentsExplicit.y : (window->Size.y - window->ScrollbarSizes.y));

		// Setup drawing context
		// (NB: That term "drawing context / DC" lost its meaning a long time ago. Initially was meant to hold transient data only. Nowadays difference between window-> and window->DC-> is dubious.)
		window->DC.IndentX = 0.0f + window->WindowPadding.x - window->Scroll.x;
		window->DC.GroupOffsetX = 0.0f;
		window->DC.ColumnsOffsetX = 0.0f;
		window->DC.CursorStartPos = window->Pos + ImVec2(window->DC.IndentX + window->DC.ColumnsOffsetX, window->TitleBarHeight() + window->MenuBarHeight() + window->WindowPadding.y - window->Scroll.y);
		window->DC.CursorPos = window->DC.CursorStartPos;
		window->DC.CursorPosPrevLine = window->DC.CursorPos;
		window->DC.CursorMaxPos = window->DC.CursorStartPos;
		window->DC.CurrentLineHeight = window->DC.PrevLineHeight = 0.0f;
		window->DC.CurrentLineTextBaseOffset = window->DC.PrevLineTextBaseOffset = 0.0f;
		window->DC.MenuBarAppending = false;
		window->DC.MenuBarOffsetX = ImMax(window->WindowPadding.x, style.ItemSpacing.x);
		window->DC.LogLinePosY = window->DC.CursorPos.y - 9999.0f;
		window->DC.ChildWindows.resize(0);
		window->DC.LayoutType = ImGuiLayoutType_Vertical;
		window->DC.ItemFlags = ImGuiItemFlags_Default_;
		window->DC.ItemWidth = window->ItemWidthDefault;
		window->DC.TextWrapPos = -1.0f; // disabled
		window->DC.ItemFlagsStack.resize(0);
		window->DC.ItemWidthStack.resize(0);
		window->DC.TextWrapPosStack.resize(0);
		window->DC.ColumnsSet = NULL;
		window->DC.TreeDepth = 0;
		window->DC.StateStorage = &window->StateStorage;
		window->DC.GroupStack.resize(0);
		window->MenuColumns.Update(3, style.ItemSpacing.x, window_just_activated_by_user);

		if ((flags & ImGuiWindowFlags_ChildWindow) && (window->DC.ItemFlags != parent_window->DC.ItemFlags))
		{
			window->DC.ItemFlags = parent_window->DC.ItemFlags;
			window->DC.ItemFlagsStack.push_back(window->DC.ItemFlags);
		}

		if (window->AutoFitFramesX > 0)
			window->AutoFitFramesX--;
		if (window->AutoFitFramesY > 0)
			window->AutoFitFramesY--;

		// Apply focus (we need to call FocusWindow() AFTER setting DC.CursorStartPos so our initial navigation reference rectangle can start around there)
		if (want_focus)
			FocusWindow(window);

		// Title bar
		if (!(flags & ImGuiWindowFlags_NoTitleBar))
		{
			// Collapse button
			if (!(flags & ImGuiWindowFlags_NoCollapse))
			{
				RenderTriangle(window->Pos + style.FramePadding, window->Collapsed ? ImGuiDir_Right : ImGuiDir_Down, 1.0f);
			}

			// Close button
			if (p_open != NULL)
			{
				const float PAD = 2.0f;
				const float rad = (window->TitleBarHeight() - PAD*2.0f) * 0.5f;
				if (CloseButton(window->GetID("#CLOSE"), window->Rect().GetTR() + ImVec2(-PAD - rad, PAD + rad), rad))
					*p_open = false;
			}

			// Title text (FIXME: refactor text alignment facilities along with RenderText helpers)
			ImVec2 text_size = CalcTextSize(name, NULL, true);
			ImRect text_r = title_bar_rect;
			float pad_left = (flags & ImGuiWindowFlags_NoCollapse) == 0 ? (style.FramePadding.x + g.FontSize + style.ItemInnerSpacing.x) : style.FramePadding.x;
			float pad_right = (p_open != NULL) ? (style.FramePadding.x + g.FontSize + style.ItemInnerSpacing.x) : style.FramePadding.x;
			if (style.WindowTitleAlign.x > 0.0f) pad_right = ImLerp(pad_right, pad_left, style.WindowTitleAlign.x);
			text_r.Min.x += pad_left;
			text_r.Max.x -= pad_right;
			ImRect clip_rect = text_r;
			clip_rect.Max.x = window->Pos.x + window->Size.x - (p_open ? title_bar_rect.GetHeight() - 3 : style.FramePadding.x); // Match the size of CloseButton()
			RenderTextClipped(text_r.Min, text_r.Max, name, NULL, &text_size, style.WindowTitleAlign, &clip_rect);
		}

		// Save clipped aabb so we can access it in constant-time in FindHoveredWindow()
		window->WindowRectClipped = window->Rect();
		window->WindowRectClipped.ClipWith(window->ClipRect);

		// Pressing CTRL+C while holding on a window copy its content to the clipboard
		// This works but 1. doesn't handle multiple Begin/End pairs, 2. recursing into another Begin/End pair - so we need to work that out and add better logging scope.
		// Maybe we can support CTRL+C on every element?
		/*
		if (g.ActiveId == move_id)
		if (g.IO.KeyCtrl && IsKeyPressedMap(ImGuiKey_C))
		ImGui::LogToClipboard();
		*/

		// Inner rectangle
		// We set this up after processing the resize grip so that our clip rectangle doesn't lag by a frame
		// Note that if our window is collapsed we will end up with a null clipping rectangle which is the correct behavior.
		window->InnerRect.Min.x = title_bar_rect.Min.x + window->WindowBorderSize;
		window->InnerRect.Min.y = title_bar_rect.Max.y + window->MenuBarHeight() + (((flags & ImGuiWindowFlags_MenuBar) || !(flags & ImGuiWindowFlags_NoTitleBar)) ? style.FrameBorderSize : window->WindowBorderSize);
		window->InnerRect.Max.x = window->Pos.x + window->Size.x - window->ScrollbarSizes.x - window->WindowBorderSize;
		window->InnerRect.Max.y = window->Pos.y + window->Size.y - window->ScrollbarSizes.y - window->WindowBorderSize;
		//window->DrawList->AddRect(window->InnerRect.Min, window->InnerRect.Max, IM_COL32_WHITE);

		// After Begin() we fill the last item / hovered data using the title bar data. Make that a standard behavior (to allow usage of context menus on title bar only, etc.).
		window->DC.LastItemId = window->MoveId;
		window->DC.LastItemRect = title_bar_rect;
		window->DC.LastItemRectHoveredRect = IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max, false);
	}

	// Inner clipping rectangle
	// Force round operator last to ensure that e.g. (int)(max.x-min.x) in user's render code produce correct result.
	const float border_size = window->WindowBorderSize;
	ImRect clip_rect;
	clip_rect.Min.x = ImFloor(0.5f + window->InnerRect.Min.x + ImMax(0.0f, ImFloor(window->WindowPadding.x*0.5f - border_size)));
	clip_rect.Min.y = ImFloor(0.5f + window->InnerRect.Min.y);
	clip_rect.Max.x = ImFloor(0.5f + window->InnerRect.Max.x - ImMax(0.0f, ImFloor(window->WindowPadding.x*0.5f - border_size)));
	clip_rect.Max.y = ImFloor(0.5f + window->InnerRect.Max.y);
	PushClipRect(clip_rect.Min, clip_rect.Max, true);

	// Clear 'accessed' flag last thing (After PushClipRect which will set the flag. We want the flag to stay false when the default "Debug" window is unused)
	if (first_begin_of_the_frame)
		window->WriteAccessed = false;

	window->BeginCount++;
	g.NextWindowData.SizeConstraintCond = 0;

	// Child window can be out of sight and have "negative" clip windows.
	// Mark them as collapsed so commands are skipped earlier (we can't manually collapse because they have no title bar).
	if (flags & ImGuiWindowFlags_ChildWindow)
	{
		IM_ASSERT((flags & ImGuiWindowFlags_NoTitleBar) != 0);
		window->Collapsed = parent_window && parent_window->Collapsed;

		if (!(flags & ImGuiWindowFlags_AlwaysAutoResize) && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0)
			window->Collapsed |= (window->WindowRectClipped.Min.x >= window->WindowRectClipped.Max.x || window->WindowRectClipped.Min.y >= window->WindowRectClipped.Max.y);

		// We also hide the window from rendering because we've already added its border to the command list.
		// (we could perform the check earlier in the function but it is simpler at this point)
		if (window->Collapsed)
			window->Active = false;
	}
	if (style.Alpha <= 0.0f)
		window->Active = false;

	// Return false if we don't intend to display anything to allow user to perform an early out optimization
	window->SkipItems = (window->Collapsed || !window->Active) && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0;
	return !window->SkipItems;
}

// Old Begin() API with 5 parameters, avoid calling this version directly! Use SetNextWindowSize()/SetNextWindowBgAlpha() + Begin() instead.
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
bool ImGui::Begin(const char* name, bool* p_open, const ImVec2& size_first_use, float bg_alpha_override, ImGuiWindowFlags flags)
{
	// Old API feature: we could pass the initial window size as a parameter. This was misleading because it only had an effect if the window didn't have data in the .ini file.
	if (size_first_use.x != 0.0f || size_first_use.y != 0.0f)
		ImGui::SetNextWindowSize(size_first_use, ImGuiCond_FirstUseEver);

	// Old API feature: override the window background alpha with a parameter.
	if (bg_alpha_override >= 0.0f)
		ImGui::SetNextWindowBgAlpha(bg_alpha_override);

	return ImGui::Begin(name, p_open, flags);
}
#endif // IMGUI_DISABLE_OBSOLETE_FUNCTIONS

void ImGui::End()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	if (window->DC.ColumnsSet != NULL)
		EndColumns();
	PopClipRect();   // inner window clip rectangle

					 // Stop logging
	if (!(window->Flags & ImGuiWindowFlags_ChildWindow))    // FIXME: add more options for scope of logging
		LogFinish();

	// Pop
	// NB: we don't clear 'window->RootWindow'. The pointer is allowed to live until the next call to Begin().
	g.CurrentWindowStack.pop_back();
	if (window->Flags & ImGuiWindowFlags_Popup)
		g.CurrentPopupStack.pop_back();
	CheckStacksSize(window, false);
	SetCurrentWindow(g.CurrentWindowStack.empty() ? NULL : g.CurrentWindowStack.back());
}

// Vertical scrollbar
// The entire piece of code below is rather confusing because:
// - We handle absolute seeking (when first clicking outside the grab) and relative manipulation (afterward or when clicking inside the grab)
// - We store values as normalized ratio and in a form that allows the window content to change while we are holding on a scrollbar
// - We handle both horizontal and vertical scrollbars, which makes the terminology not ideal.
void ImGui::Scrollbar(ImGuiLayoutType direction)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	const bool horizontal = (direction == ImGuiLayoutType_Horizontal);
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(horizontal ? "#SCROLLX" : "#SCROLLY");

	// Render background
	bool other_scrollbar = (horizontal ? window->ScrollbarY : window->ScrollbarX);
	float other_scrollbar_size_w = other_scrollbar ? style.ScrollbarSize : 0.0f;
	const ImRect window_rect = window->Rect();
	const float border_size = window->WindowBorderSize;
	ImRect bb = horizontal
		? ImRect(window->Pos.x + border_size, window_rect.Max.y - style.ScrollbarSize, window_rect.Max.x - other_scrollbar_size_w - border_size, window_rect.Max.y - border_size)
		: ImRect(window_rect.Max.x - style.ScrollbarSize, window->Pos.y + border_size, window_rect.Max.x - border_size, window_rect.Max.y - other_scrollbar_size_w - border_size);
	if (!horizontal)
		bb.Min.y += window->TitleBarHeight() + ((window->Flags & ImGuiWindowFlags_MenuBar) ? window->MenuBarHeight() : 0.0f);
	if (bb.GetWidth() <= 0.0f || bb.GetHeight() <= 0.0f)
		return;

	int window_rounding_corners;
	if (horizontal)
		window_rounding_corners = ImDrawCornerFlags_BotLeft | (other_scrollbar ? 0 : ImDrawCornerFlags_BotRight);
	else
		window_rounding_corners = (((window->Flags & ImGuiWindowFlags_NoTitleBar) && !(window->Flags & ImGuiWindowFlags_MenuBar)) ? ImDrawCornerFlags_TopRight : 0) | (other_scrollbar ? 0 : ImDrawCornerFlags_BotRight);
	window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_ScrollbarBg), window->WindowRounding, window_rounding_corners);
	bb.Expand(ImVec2(-ImClamp((float)(int)((bb.Max.x - bb.Min.x - 2.0f) * 0.5f), 0.0f, 3.0f), -ImClamp((float)(int)((bb.Max.y - bb.Min.y - 2.0f) * 0.5f), 0.0f, 3.0f)));

	// V denote the main, longer axis of the scrollbar (= height for a vertical scrollbar)
	float scrollbar_size_v = horizontal ? bb.GetWidth() : bb.GetHeight();
	float scroll_v = horizontal ? window->Scroll.x : window->Scroll.y;
	float win_size_avail_v = (horizontal ? window->SizeFull.x : window->SizeFull.y) - other_scrollbar_size_w;
	float win_size_contents_v = horizontal ? window->SizeContents.x : window->SizeContents.y;

	// Calculate the height of our grabbable box. It generally represent the amount visible (vs the total scrollable amount)
	// But we maintain a minimum size in pixel to allow for the user to still aim inside.
	IM_ASSERT(ImMax(win_size_contents_v, win_size_avail_v) > 0.0f); // Adding this assert to check if the ImMax(XXX,1.0f) is still needed. PLEASE CONTACT ME if this triggers.
	const float win_size_v = ImMax(ImMax(win_size_contents_v, win_size_avail_v), 1.0f);
	const float grab_h_pixels = ImClamp(scrollbar_size_v * (win_size_avail_v / win_size_v), style.GrabMinSize, scrollbar_size_v);
	const float grab_h_norm = grab_h_pixels / scrollbar_size_v;

	// Handle input right away. None of the code of Begin() is relying on scrolling position before calling Scrollbar().
	bool held = false;
	bool hovered = false;
	const bool previously_held = (g.ActiveId == id);
	ButtonBehavior(bb, id, &hovered, &held);

	float scroll_max = ImMax(1.0f, win_size_contents_v - win_size_avail_v);
	float scroll_ratio = ImSaturate(scroll_v / scroll_max);
	float grab_v_norm = scroll_ratio * (scrollbar_size_v - grab_h_pixels) / scrollbar_size_v;
	if (held && grab_h_norm < 1.0f)
	{
		float scrollbar_pos_v = horizontal ? bb.Min.x : bb.Min.y;
		float mouse_pos_v = horizontal ? g.IO.MousePos.x : g.IO.MousePos.y;
		float* click_delta_to_grab_center_v = horizontal ? &g.ScrollbarClickDeltaToGrabCenter.x : &g.ScrollbarClickDeltaToGrabCenter.y;

		// Click position in scrollbar normalized space (0.0f->1.0f)
		const float clicked_v_norm = ImSaturate((mouse_pos_v - scrollbar_pos_v) / scrollbar_size_v);
		SetHoveredID(id);

		bool seek_absolute = false;
		if (!previously_held)
		{
			// On initial click calculate the distance between mouse and the center of the grab
			if (clicked_v_norm >= grab_v_norm && clicked_v_norm <= grab_v_norm + grab_h_norm)
			{
				*click_delta_to_grab_center_v = clicked_v_norm - grab_v_norm - grab_h_norm*0.5f;
			}
			else
			{
				seek_absolute = true;
				*click_delta_to_grab_center_v = 0.0f;
			}
		}

		// Apply scroll
		// It is ok to modify Scroll here because we are being called in Begin() after the calculation of SizeContents and before setting up our starting position
		const float scroll_v_norm = ImSaturate((clicked_v_norm - *click_delta_to_grab_center_v - grab_h_norm*0.5f) / (1.0f - grab_h_norm));
		scroll_v = (float)(int)(0.5f + scroll_v_norm * scroll_max);//(win_size_contents_v - win_size_v));
		if (horizontal)
			window->Scroll.x = scroll_v;
		else
			window->Scroll.y = scroll_v;

		// Update values for rendering
		scroll_ratio = ImSaturate(scroll_v / scroll_max);
		grab_v_norm = scroll_ratio * (scrollbar_size_v - grab_h_pixels) / scrollbar_size_v;

		// Update distance to grab now that we have seeked and saturated
		if (seek_absolute)
			*click_delta_to_grab_center_v = clicked_v_norm - grab_v_norm - grab_h_norm*0.5f;
	}

	// Render
	const ImU32 grab_col = GetColorU32(held ? ImGuiCol_ScrollbarGrabActive : hovered ? ImGuiCol_ScrollbarGrabHovered : ImGuiCol_ScrollbarGrab);
	ImRect grab_rect;
	if (horizontal)
		grab_rect = ImRect(ImLerp(bb.Min.x, bb.Max.x, grab_v_norm), bb.Min.y, ImMin(ImLerp(bb.Min.x, bb.Max.x, grab_v_norm) + grab_h_pixels, window_rect.Max.x), bb.Max.y);
	else
		grab_rect = ImRect(bb.Min.x, ImLerp(bb.Min.y, bb.Max.y, grab_v_norm), bb.Max.x, ImMin(ImLerp(bb.Min.y, bb.Max.y, grab_v_norm) + grab_h_pixels, window_rect.Max.y));
	window->DrawList->AddRectFilled(grab_rect.Min, grab_rect.Max, grab_col, style.ScrollbarRounding);
}

void ImGui::BringWindowToFront(ImGuiWindow* window)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* current_front_window = g.Windows.back();
	if (current_front_window == window || current_front_window->RootWindow == window)
		return;
	for (int i = g.Windows.Size - 2; i >= 0; i--) // We can ignore the front most window
		if (g.Windows[i] == window)
		{
			g.Windows.erase(g.Windows.Data + i);
			g.Windows.push_back(window);
			break;
		}
}

void ImGui::BringWindowToBack(ImGuiWindow* window)
{
	ImGuiContext& g = *GImGui;
	if (g.Windows[0] == window)
		return;
	for (int i = 0; i < g.Windows.Size; i++)
		if (g.Windows[i] == window)
		{
			memmove(&g.Windows[1], &g.Windows[0], (size_t)i * sizeof(ImGuiWindow*));
			g.Windows[0] = window;
			break;
		}
}

// Moving window to front of display and set focus (which happens to be back of our sorted list)
void ImGui::FocusWindow(ImGuiWindow* window)
{
	ImGuiContext& g = *GImGui;

	// Always mark the window we passed as focused. This is used for keyboard interactions such as tabbing.
	g.NavWindow = window;

	// Passing NULL allow to disable keyboard focus
	if (!window)
		return;

	// Move the root window to the top of the pile
	if (window->RootWindow)
		window = window->RootWindow;

	// Steal focus on active widgets
	if (window->Flags & ImGuiWindowFlags_Popup) // FIXME: This statement should be unnecessary. Need further testing before removing it..
		if (g.ActiveId != 0 && g.ActiveIdWindow && g.ActiveIdWindow->RootWindow != window)
			ClearActiveID();

	// Bring to front
	if (!(window->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus))
		BringWindowToFront(window);
}

void ImGui::FocusFrontMostActiveWindow(ImGuiWindow* ignore_window)
{
	ImGuiContext& g = *GImGui;
	for (int i = g.Windows.Size - 1; i >= 0; i--)
		if (g.Windows[i] != ignore_window && g.Windows[i]->WasActive && !(g.Windows[i]->Flags & ImGuiWindowFlags_ChildWindow))
		{
			FocusWindow(g.Windows[i]);
			return;
		}
}

void ImGui::PushItemWidth(float item_width)
{
	ImGuiWindow* window = GetCurrentWindow();
	window->DC.ItemWidth = (item_width == 0.0f ? window->ItemWidthDefault : item_width);
	window->DC.ItemWidthStack.push_back(window->DC.ItemWidth);
}

void ImGui::PushMultiItemsWidths(int components, float w_full)
{
	ImGuiWindow* window = GetCurrentWindow();
	const ImGuiStyle& style = GImGui->Style;
	if (w_full <= 0.0f)
		w_full = CalcItemWidth();
	const float w_item_one = ImMax(1.0f, (float)(int)((w_full - (style.ItemInnerSpacing.x) * (components - 1)) / (float)components));
	const float w_item_last = ImMax(1.0f, (float)(int)(w_full - (w_item_one + style.ItemInnerSpacing.x) * (components - 1)));
	window->DC.ItemWidthStack.push_back(w_item_last);
	for (int i = 0; i < components - 1; i++)
		window->DC.ItemWidthStack.push_back(w_item_one);
	window->DC.ItemWidth = window->DC.ItemWidthStack.back();
}

void ImGui::PopItemWidth()
{
	ImGuiWindow* window = GetCurrentWindow();
	window->DC.ItemWidthStack.pop_back();
	window->DC.ItemWidth = window->DC.ItemWidthStack.empty() ? window->ItemWidthDefault : window->DC.ItemWidthStack.back();
}

float ImGui::CalcItemWidth()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	float w = window->DC.ItemWidth;
	if (w < 0.0f)
	{
		// Align to a right-side limit. We include 1 frame padding in the calculation because this is how the width is always used (we add 2 frame padding to it), but we could move that responsibility to the widget as well.
		float width_to_right_edge = GetContentRegionAvail().x;
		w = ImMax(1.0f, width_to_right_edge + w);
	}
	w = (float)(int)w;
	return w;
}

static ImFont* GetDefaultFont()
{
	ImGuiContext& g = *GImGui;
	return g.IO.FontDefault ? g.IO.FontDefault : g.IO.Fonts->Fonts[0];
}

void ImGui::SetCurrentFont(ImFont* font)
{
	ImGuiContext& g = *GImGui;
	IM_ASSERT(font && font->IsLoaded());    // Font Atlas not created. Did you call io.Fonts->GetTexDataAsRGBA32 / GetTexDataAsAlpha8 ?
	IM_ASSERT(font->Scale > 0.0f);
	g.Font = font;
	g.FontBaseSize = g.IO.FontGlobalScale * g.Font->FontSize * g.Font->Scale;
	g.FontSize = g.CurrentWindow ? g.CurrentWindow->CalcFontSize() : 0.0f;

	ImFontAtlas* atlas = g.Font->ContainerAtlas;
	g.DrawListSharedData.TexUvWhitePixel = atlas->TexUvWhitePixel;
	g.DrawListSharedData.Font = g.Font;
	g.DrawListSharedData.FontSize = g.FontSize;
}

void ImGui::PushFont(ImFont* font)
{
	ImGuiContext& g = *GImGui;
	if (!font)
		font = GetDefaultFont();
	SetCurrentFont(font);
	g.FontStack.push_back(font);
	g.CurrentWindow->DrawList->PushTextureID(font->ContainerAtlas->TexID);
}

void  ImGui::PopFont()
{
	ImGuiContext& g = *GImGui;
	g.CurrentWindow->DrawList->PopTextureID();
	g.FontStack.pop_back();
	SetCurrentFont(g.FontStack.empty() ? GetDefaultFont() : g.FontStack.back());
}

void ImGui::PushItemFlag(ImGuiItemFlags option, bool enabled)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (enabled)
		window->DC.ItemFlags |= option;
	else
		window->DC.ItemFlags &= ~option;
	window->DC.ItemFlagsStack.push_back(window->DC.ItemFlags);
}

void ImGui::PopItemFlag()
{
	ImGuiWindow* window = GetCurrentWindow();
	window->DC.ItemFlagsStack.pop_back();
	window->DC.ItemFlags = window->DC.ItemFlagsStack.empty() ? ImGuiItemFlags_Default_ : window->DC.ItemFlagsStack.back();
}

void ImGui::PushAllowKeyboardFocus(bool allow_keyboard_focus)
{
	PushItemFlag(ImGuiItemFlags_AllowKeyboardFocus, allow_keyboard_focus);
}

void ImGui::PopAllowKeyboardFocus()
{
	PopItemFlag();
}

void ImGui::PushButtonRepeat(bool repeat)
{
	PushItemFlag(ImGuiItemFlags_ButtonRepeat, repeat);
}

void ImGui::PopButtonRepeat()
{
	PopItemFlag();
}

void ImGui::PushTextWrapPos(float wrap_pos_x)
{
	ImGuiWindow* window = GetCurrentWindow();
	window->DC.TextWrapPos = wrap_pos_x;
	window->DC.TextWrapPosStack.push_back(wrap_pos_x);
}

void ImGui::PopTextWrapPos()
{
	ImGuiWindow* window = GetCurrentWindow();
	window->DC.TextWrapPosStack.pop_back();
	window->DC.TextWrapPos = window->DC.TextWrapPosStack.empty() ? -1.0f : window->DC.TextWrapPosStack.back();
}

// FIXME: This may incur a round-trip (if the end user got their data from a float4) but eventually we aim to store the in-flight colors as ImU32
void ImGui::PushStyleColor(ImGuiCol idx, ImU32 col)
{
	ImGuiContext& g = *GImGui;
	ImGuiColMod backup;
	backup.Col = idx;
	backup.BackupValue = g.Style.Colors[idx];
	g.ColorModifiers.push_back(backup);
	g.Style.Colors[idx] = ColorConvertU32ToFloat4(col);
}

void ImGui::PushStyleColor(ImGuiCol idx, const ImVec4& col)
{
	ImGuiContext& g = *GImGui;
	ImGuiColMod backup;
	backup.Col = idx;
	backup.BackupValue = g.Style.Colors[idx];
	g.ColorModifiers.push_back(backup);
	g.Style.Colors[idx] = col;
}

void ImGui::PopStyleColor(int count)
{
	ImGuiContext& g = *GImGui;
	while (count > 0)
	{
		ImGuiColMod& backup = g.ColorModifiers.back();
		g.Style.Colors[backup.Col] = backup.BackupValue;
		g.ColorModifiers.pop_back();
		count--;
	}
}

struct ImGuiStyleVarInfo
{
	ImGuiDataType   Type;
	ImU32           Offset;
	void*           GetVarPtr(ImGuiStyle* style) const { return (void*)((unsigned char*)style + Offset); }
};

static const ImGuiStyleVarInfo GStyleVarInfo[] =
{
	{ ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, Alpha) },                // ImGuiStyleVar_Alpha
	{ ImGuiDataType_Float2, (ImU32)IM_OFFSETOF(ImGuiStyle, WindowPadding) },        // ImGuiStyleVar_WindowPadding
	{ ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, WindowRounding) },       // ImGuiStyleVar_WindowRounding
	{ ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, WindowBorderSize) },     // ImGuiStyleVar_WindowBorderSize
	{ ImGuiDataType_Float2, (ImU32)IM_OFFSETOF(ImGuiStyle, WindowMinSize) },        // ImGuiStyleVar_WindowMinSize
	{ ImGuiDataType_Float2, (ImU32)IM_OFFSETOF(ImGuiStyle, WindowTitleAlign) },     // ImGuiStyleVar_WindowTitleAlign
	{ ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, ChildRounding) },        // ImGuiStyleVar_ChildRounding
	{ ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, ChildBorderSize) },      // ImGuiStyleVar_ChildBorderSize
	{ ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, PopupRounding) },        // ImGuiStyleVar_PopupRounding
	{ ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, PopupBorderSize) },      // ImGuiStyleVar_PopupBorderSize
	{ ImGuiDataType_Float2, (ImU32)IM_OFFSETOF(ImGuiStyle, FramePadding) },         // ImGuiStyleVar_FramePadding
	{ ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, FrameRounding) },        // ImGuiStyleVar_FrameRounding
	{ ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, FrameBorderSize) },      // ImGuiStyleVar_FrameBorderSize
	{ ImGuiDataType_Float2, (ImU32)IM_OFFSETOF(ImGuiStyle, ItemSpacing) },          // ImGuiStyleVar_ItemSpacing
	{ ImGuiDataType_Float2, (ImU32)IM_OFFSETOF(ImGuiStyle, ItemInnerSpacing) },     // ImGuiStyleVar_ItemInnerSpacing
	{ ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, IndentSpacing) },        // ImGuiStyleVar_IndentSpacing
	{ ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, ScrollbarSize) },        // ImGuiStyleVar_ScrollbarSize
	{ ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, ScrollbarRounding) },    // ImGuiStyleVar_ScrollbarRounding
	{ ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, GrabMinSize) },          // ImGuiStyleVar_GrabMinSize
	{ ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, GrabRounding) },         // ImGuiStyleVar_GrabRounding
	{ ImGuiDataType_Float2, (ImU32)IM_OFFSETOF(ImGuiStyle, ButtonTextAlign) },      // ImGuiStyleVar_ButtonTextAlign
};

static const ImGuiStyleVarInfo* GetStyleVarInfo(ImGuiStyleVar idx)
{
	IM_ASSERT(idx >= 0 && idx < ImGuiStyleVar_Count_);
	IM_ASSERT(IM_ARRAYSIZE(GStyleVarInfo) == ImGuiStyleVar_Count_);
	return &GStyleVarInfo[idx];
}

void ImGui::PushStyleVar(ImGuiStyleVar idx, float val)
{
	const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
	if (var_info->Type == ImGuiDataType_Float)
	{
		ImGuiContext& g = *GImGui;
		float* pvar = (float*)var_info->GetVarPtr(&g.Style);
		g.StyleModifiers.push_back(ImGuiStyleMod(idx, *pvar));
		*pvar = val;
		return;
	}
	IM_ASSERT(0); // Called function with wrong-type? Variable is not a float.
}

void ImGui::PushStyleVar(ImGuiStyleVar idx, const ImVec2& val)
{
	const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
	if (var_info->Type == ImGuiDataType_Float2)
	{
		ImGuiContext& g = *GImGui;
		ImVec2* pvar = (ImVec2*)var_info->GetVarPtr(&g.Style);
		g.StyleModifiers.push_back(ImGuiStyleMod(idx, *pvar));
		*pvar = val;
		return;
	}
	IM_ASSERT(0); // Called function with wrong-type? Variable is not a ImVec2.
}

void ImGui::PopStyleVar(int count)
{
	ImGuiContext& g = *GImGui;
	while (count > 0)
	{
		ImGuiStyleMod& backup = g.StyleModifiers.back();
		const ImGuiStyleVarInfo* info = GetStyleVarInfo(backup.VarIdx);
		if (info->Type == ImGuiDataType_Float)          (*(float*)info->GetVarPtr(&g.Style)) = backup.BackupFloat[0];
		else if (info->Type == ImGuiDataType_Float2)    (*(ImVec2*)info->GetVarPtr(&g.Style)) = ImVec2(backup.BackupFloat[0], backup.BackupFloat[1]);
		else if (info->Type == ImGuiDataType_Int)       (*(int*)info->GetVarPtr(&g.Style)) = backup.BackupInt[0];
		g.StyleModifiers.pop_back();
		count--;
	}
}

const char* ImGui::GetStyleColorName(ImGuiCol idx)
{
	// Create switch-case from enum with regexp: ImGuiCol_{.*}, --> case ImGuiCol_\1: return "\1";
	switch (idx)
	{
	case ImGuiCol_Text: return "Text";
	case ImGuiCol_TextDisabled: return "TextDisabled";
	case ImGuiCol_WindowBg: return "WindowBg";
	case ImGuiCol_ChildBg: return "ChildBg";
	case ImGuiCol_PopupBg: return "PopupBg";
	case ImGuiCol_Border: return "Border";
	case ImGuiCol_BorderShadow: return "BorderShadow";
	case ImGuiCol_FrameBg: return "FrameBg";
	case ImGuiCol_FrameBgHovered: return "FrameBgHovered";
	case ImGuiCol_FrameBgActive: return "FrameBgActive";
	case ImGuiCol_TitleBg: return "TitleBg";
	case ImGuiCol_TitleBgActive: return "TitleBgActive";
	case ImGuiCol_TitleBgCollapsed: return "TitleBgCollapsed";
	case ImGuiCol_MenuBarBg: return "MenuBarBg";
	case ImGuiCol_ScrollbarBg: return "ScrollbarBg";
	case ImGuiCol_ScrollbarGrab: return "ScrollbarGrab";
	case ImGuiCol_ScrollbarGrabHovered: return "ScrollbarGrabHovered";
	case ImGuiCol_ScrollbarGrabActive: return "ScrollbarGrabActive";
	case ImGuiCol_CheckMark: return "CheckMark";
	case ImGuiCol_SliderGrab: return "SliderGrab";
	case ImGuiCol_SliderGrabActive: return "SliderGrabActive";
	case ImGuiCol_Button: return "Button";
	case ImGuiCol_ButtonHovered: return "ButtonHovered";
	case ImGuiCol_ButtonActive: return "ButtonActive";
	case ImGuiCol_Button1: return "Button1";
	case ImGuiCol_ButtonHovered1: return "ButtonHovered1";
	case ImGuiCol_ButtonActive1: return "ButtonActive1";
	case ImGuiCol_Button2: return "Button2";
	case ImGuiCol_ButtonHovered2: return "ButtonHovered2";
	case ImGuiCol_ButtonActive2: return "ButtonActive2";
	case ImGuiCol_Button3: return "Button3";
	case ImGuiCol_ButtonHovered3: return "ButtonHovered3";
	case ImGuiCol_ButtonActive3: return "ButtonActive3";
	case ImGuiCol_Button4: return "Button4";
	case ImGuiCol_ButtonHovered4: return "ButtonHovered4";
	case ImGuiCol_ButtonActive4: return "ButtonActive4";
	case ImGuiCol_Button5: return "Button5";
	case ImGuiCol_ButtonHovered5: return "ButtonHovered5";
	case ImGuiCol_ButtonActive5: return "ButtonActive5";
	case ImGuiCol_Button6: return "Button6";
	case ImGuiCol_ButtonHovered6: return "ButtonHovered6";
	case ImGuiCol_ButtonActive6: return "ButtonActive6";
	case ImGuiCol_Button7: return "Button7";
	case ImGuiCol_ButtonHovered7: return "ButtonHovered7";
	case ImGuiCol_ButtonActive7: return "ButtonActive7";
	case ImGuiCol_Header: return "Header";
	case ImGuiCol_HeaderHovered: return "HeaderHovered";
	case ImGuiCol_HeaderActive: return "HeaderActive";
	case ImGuiCol_Separator: return "Separator";
	case ImGuiCol_SeparatorHovered: return "SeparatorHovered";
	case ImGuiCol_SeparatorActive: return "SeparatorActive";
	case ImGuiCol_ResizeGrip: return "ResizeGrip";
	case ImGuiCol_ResizeGripHovered: return "ResizeGripHovered";
	case ImGuiCol_ResizeGripActive: return "ResizeGripActive";
	case ImGuiCol_CloseButton: return "CloseButton";
	case ImGuiCol_CloseButtonHovered: return "CloseButtonHovered";
	case ImGuiCol_CloseButtonActive: return "CloseButtonActive";
	case ImGuiCol_PlotLines: return "PlotLines";
	case ImGuiCol_PlotLinesHovered: return "PlotLinesHovered";
	case ImGuiCol_PlotHistogram: return "PlotHistogram";
	case ImGuiCol_PlotHistogramHovered: return "PlotHistogramHovered";
	case ImGuiCol_TextSelectedBg: return "TextSelectedBg";
	case ImGuiCol_ModalWindowDarkening: return "ModalWindowDarkening";
	case ImGuiCol_DragDropTarget: return "DragDropTarget";
	}
	IM_ASSERT(0);
	return "Unknown";
}

bool ImGui::IsWindowChildOf(ImGuiWindow* window, ImGuiWindow* potential_parent)
{
	if (window->RootWindow == potential_parent)
		return true;
	while (window != NULL)
	{
		if (window == potential_parent)
			return true;
		window = window->ParentWindow;
	}
	return false;
}

bool ImGui::IsWindowHovered(ImGuiHoveredFlags flags)
{
	IM_ASSERT((flags & ImGuiHoveredFlags_AllowWhenOverlapped) == 0);   // Flags not supported by this function
	ImGuiContext& g = *GImGui;

	if (flags & ImGuiHoveredFlags_AnyWindow)
	{
		if (g.HoveredWindow == NULL)
			return false;
	}
	else
	{
		switch (flags & (ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_ChildWindows))
		{
		case ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_ChildWindows:
			if (g.HoveredRootWindow != g.CurrentWindow->RootWindow)
				return false;
			break;
		case ImGuiHoveredFlags_RootWindow:
			if (g.HoveredWindow != g.CurrentWindow->RootWindow)
				return false;
			break;
		case ImGuiHoveredFlags_ChildWindows:
			if (g.HoveredWindow == NULL || !IsWindowChildOf(g.HoveredWindow, g.CurrentWindow))
				return false;
			break;
		default:
			if (g.HoveredWindow != g.CurrentWindow)
				return false;
			break;
		}
	}

	if (!IsWindowContentHoverable(g.HoveredRootWindow, flags))
		return false;
	if (!(flags & ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		if (g.ActiveId != 0 && !g.ActiveIdAllowOverlap && g.ActiveId != g.HoveredWindow->MoveId)
			return false;
	return true;
}

bool ImGui::IsWindowFocused(ImGuiFocusedFlags flags)
{
	ImGuiContext& g = *GImGui;
	IM_ASSERT(g.CurrentWindow);     // Not inside a Begin()/End()

	if (flags & ImGuiFocusedFlags_AnyWindow)
		return g.NavWindow != NULL;

	switch (flags & (ImGuiFocusedFlags_RootWindow | ImGuiFocusedFlags_ChildWindows))
	{
	case ImGuiFocusedFlags_RootWindow | ImGuiFocusedFlags_ChildWindows:
		return g.NavWindow && g.NavWindow->RootWindow == g.CurrentWindow->RootWindow;
	case ImGuiFocusedFlags_RootWindow:
		return g.NavWindow == g.CurrentWindow->RootWindow;
	case ImGuiFocusedFlags_ChildWindows:
		return g.NavWindow && IsWindowChildOf(g.NavWindow, g.CurrentWindow);
	default:
		return g.NavWindow == g.CurrentWindow;
	}
}

float ImGui::GetWindowWidth()
{
	ImGuiWindow* window = GImGui->CurrentWindow;
	return window->Size.x;
}

float ImGui::GetWindowHeight()
{
	ImGuiWindow* window = GImGui->CurrentWindow;
	return window->Size.y;
}

ImVec2 ImGui::GetWindowPos()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	return window->Pos;
}

static void SetWindowScrollX(ImGuiWindow* window, float new_scroll_x)
{
	window->DC.CursorMaxPos.x += window->Scroll.x; // SizeContents is generally computed based on CursorMaxPos which is affected by scroll position, so we need to apply our change to it.
	window->Scroll.x = new_scroll_x;
	window->DC.CursorMaxPos.x -= window->Scroll.x;
}

static void SetWindowScrollY(ImGuiWindow* window, float new_scroll_y)
{
	window->DC.CursorMaxPos.y += window->Scroll.y; // SizeContents is generally computed based on CursorMaxPos which is affected by scroll position, so we need to apply our change to it.
	window->Scroll.y = new_scroll_y;
	window->DC.CursorMaxPos.y -= window->Scroll.y;
}

static void SetWindowPos(ImGuiWindow* window, const ImVec2& pos, ImGuiCond cond)
{
	// Test condition (NB: bit 0 is always true) and clear flags for next time
	if (cond && (window->SetWindowPosAllowFlags & cond) == 0)
		return;
	window->SetWindowPosAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);
	window->SetWindowPosVal = ImVec2(FLT_MAX, FLT_MAX);

	// Set
	const ImVec2 old_pos = window->Pos;
	window->PosFloat = pos;
	window->Pos = ImFloor(pos);
	window->DC.CursorPos += (window->Pos - old_pos);    // As we happen to move the window while it is being appended to (which is a bad idea - will smear) let's at least offset the cursor
	window->DC.CursorMaxPos += (window->Pos - old_pos); // And more importantly we need to adjust this so size calculation doesn't get affected.
}

void ImGui::SetWindowPos(const ImVec2& pos, ImGuiCond cond)
{
	ImGuiWindow* window = GetCurrentWindowRead();
	SetWindowPos(window, pos, cond);
}

void ImGui::SetWindowPos(const char* name, const ImVec2& pos, ImGuiCond cond)
{
	if (ImGuiWindow* window = FindWindowByName(name))
		SetWindowPos(window, pos, cond);
}

ImVec2 ImGui::GetWindowSize()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->Size;
}

static void SetWindowSize(ImGuiWindow* window, const ImVec2& size, ImGuiCond cond)
{
	// Test condition (NB: bit 0 is always true) and clear flags for next time
	if (cond && (window->SetWindowSizeAllowFlags & cond) == 0)
		return;
	window->SetWindowSizeAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);

	// Set
	if (size.x > 0.0f)
	{
		window->AutoFitFramesX = 0;
		window->SizeFull.x = size.x;
	}
	else
	{
		window->AutoFitFramesX = 2;
		window->AutoFitOnlyGrows = false;
	}
	if (size.y > 0.0f)
	{
		window->AutoFitFramesY = 0;
		window->SizeFull.y = size.y;
	}
	else
	{
		window->AutoFitFramesY = 2;
		window->AutoFitOnlyGrows = false;
	}
}

void ImGui::SetWindowSize(const ImVec2& size, ImGuiCond cond)
{
	SetWindowSize(GImGui->CurrentWindow, size, cond);
}

void ImGui::SetWindowSize(const char* name, const ImVec2& size, ImGuiCond cond)
{
	if (ImGuiWindow* window = FindWindowByName(name))
		SetWindowSize(window, size, cond);
}

static void SetWindowCollapsed(ImGuiWindow* window, bool collapsed, ImGuiCond cond)
{
	// Test condition (NB: bit 0 is always true) and clear flags for next time
	if (cond && (window->SetWindowCollapsedAllowFlags & cond) == 0)
		return;
	window->SetWindowCollapsedAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);

	// Set
	window->Collapsed = collapsed;
}

void ImGui::SetWindowCollapsed(bool collapsed, ImGuiCond cond)
{
	SetWindowCollapsed(GImGui->CurrentWindow, collapsed, cond);
}

bool ImGui::IsWindowCollapsed()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->Collapsed;
}

bool ImGui::IsWindowAppearing()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->Appearing;
}

void ImGui::SetWindowCollapsed(const char* name, bool collapsed, ImGuiCond cond)
{
	if (ImGuiWindow* window = FindWindowByName(name))
		SetWindowCollapsed(window, collapsed, cond);
}

void ImGui::SetWindowFocus()
{
	FocusWindow(GImGui->CurrentWindow);
}

void ImGui::SetWindowFocus(const char* name)
{
	if (name)
	{
		if (ImGuiWindow* window = FindWindowByName(name))
			FocusWindow(window);
	}
	else
	{
		FocusWindow(NULL);
	}
}

void ImGui::SetNextWindowPos(const ImVec2& pos, ImGuiCond cond, const ImVec2& pivot)
{
	ImGuiContext& g = *GImGui;
	g.NextWindowData.PosVal = pos;
	g.NextWindowData.PosPivotVal = pivot;
	g.NextWindowData.PosCond = cond ? cond : ImGuiCond_Always;
}

void ImGui::SetNextWindowSize(const ImVec2& size, ImGuiCond cond)
{
	ImGuiContext& g = *GImGui;
	g.NextWindowData.SizeVal = size;
	g.NextWindowData.SizeCond = cond ? cond : ImGuiCond_Always;
}

void ImGui::SetNextWindowSizeConstraints(const ImVec2& size_min, const ImVec2& size_max, ImGuiSizeCallback custom_callback, void* custom_callback_user_data)
{
	ImGuiContext& g = *GImGui;
	g.NextWindowData.SizeConstraintCond = ImGuiCond_Always;
	g.NextWindowData.SizeConstraintRect = ImRect(size_min, size_max);
	g.NextWindowData.SizeCallback = custom_callback;
	g.NextWindowData.SizeCallbackUserData = custom_callback_user_data;
}

void ImGui::SetNextWindowContentSize(const ImVec2& size)
{
	ImGuiContext& g = *GImGui;
	g.NextWindowData.ContentSizeVal = size;  // In Begin() we will add the size of window decorations (title bar, menu etc.) to that to form a SizeContents value.
	g.NextWindowData.ContentSizeCond = ImGuiCond_Always;
}

void ImGui::SetNextWindowCollapsed(bool collapsed, ImGuiCond cond)
{
	ImGuiContext& g = *GImGui;
	g.NextWindowData.CollapsedVal = collapsed;
	g.NextWindowData.CollapsedCond = cond ? cond : ImGuiCond_Always;
}

void ImGui::SetNextWindowFocus()
{
	ImGuiContext& g = *GImGui;
	g.NextWindowData.FocusCond = ImGuiCond_Always;   // Using a Cond member for consistency (may transition all of them to single flag set for fast Clear() op)
}

void ImGui::SetNextWindowBgAlpha(float alpha)
{
	ImGuiContext& g = *GImGui;
	g.NextWindowData.BgAlphaVal = alpha;
	g.NextWindowData.BgAlphaCond = ImGuiCond_Always; // Using a Cond member for consistency (may transition all of them to single flag set for fast Clear() op)
}

// In window space (not screen space!)
ImVec2 ImGui::GetContentRegionMax()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	ImVec2 mx = window->ContentsRegionRect.Max;
	if (window->DC.ColumnsSet)
		mx.x = GetColumnOffset(window->DC.ColumnsSet->Current + 1) - window->WindowPadding.x;
	return mx;
}

ImVec2 ImGui::GetContentRegionAvail()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return GetContentRegionMax() - (window->DC.CursorPos - window->Pos);
}

float ImGui::GetContentRegionAvailWidth()
{
	return GetContentRegionAvail().x;
}

// In window space (not screen space!)
ImVec2 ImGui::GetWindowContentRegionMin()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->ContentsRegionRect.Min;
}

ImVec2 ImGui::GetWindowContentRegionMax()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->ContentsRegionRect.Max;
}

float ImGui::GetWindowContentRegionWidth()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->ContentsRegionRect.Max.x - window->ContentsRegionRect.Min.x;
}

float ImGui::GetTextLineHeight()
{
	ImGuiContext& g = *GImGui;
	return g.FontSize;
}

float ImGui::GetTextLineHeightWithSpacing()
{
	ImGuiContext& g = *GImGui;
	return g.FontSize + g.Style.ItemSpacing.y;
}

float ImGui::GetFrameHeight()
{
	ImGuiContext& g = *GImGui;
	return g.FontSize + g.Style.FramePadding.y * 2.0f;
}

float ImGui::GetFrameHeightWithSpacing()
{
	ImGuiContext& g = *GImGui;
	return g.FontSize + g.Style.FramePadding.y * 2.0f + g.Style.ItemSpacing.y;
}

ImDrawList* ImGui::GetWindowDrawList()
{
	ImGuiWindow* window = GetCurrentWindow();
	return window->DrawList;
}

ImFont* ImGui::GetFont()
{
	return GImGui->Font;
}

float ImGui::GetFontSize()
{
	return GImGui->FontSize;
}

ImVec2 ImGui::GetFontTexUvWhitePixel()
{
	return GImGui->DrawListSharedData.TexUvWhitePixel;
}

void ImGui::SetWindowFontScale(float scale)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();
	window->FontWindowScale = scale;
	g.FontSize = g.DrawListSharedData.FontSize = window->CalcFontSize();
}

// User generally sees positions in window coordinates. Internally we store CursorPos in absolute screen coordinates because it is more convenient.
// Conversion happens as we pass the value to user, but it makes our naming convention confusing because GetCursorPos() == (DC.CursorPos - window.Pos). May want to rename 'DC.CursorPos'.
ImVec2 ImGui::GetCursorPos()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->DC.CursorPos - window->Pos + window->Scroll;
}

float ImGui::GetCursorPosX()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->DC.CursorPos.x - window->Pos.x + window->Scroll.x;
}

float ImGui::GetCursorPosY()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->DC.CursorPos.y - window->Pos.y + window->Scroll.y;
}

void ImGui::SetCursorPos(const ImVec2& local_pos)
{
	ImGuiWindow* window = GetCurrentWindow();
	window->DC.CursorPos = window->Pos - window->Scroll + local_pos;
	window->DC.CursorMaxPos = ImMax(window->DC.CursorMaxPos, window->DC.CursorPos);
}

void ImGui::SetCursorPosX(float x)
{
	ImGuiWindow* window = GetCurrentWindow();
	window->DC.CursorPos.x = window->Pos.x - window->Scroll.x + x;
	window->DC.CursorMaxPos.x = ImMax(window->DC.CursorMaxPos.x, window->DC.CursorPos.x);
}

void ImGui::SetCursorPosY(float y)
{
	ImGuiWindow* window = GetCurrentWindow();
	window->DC.CursorPos.y = window->Pos.y - window->Scroll.y + y;
	window->DC.CursorMaxPos.y = ImMax(window->DC.CursorMaxPos.y, window->DC.CursorPos.y);
}

ImVec2 ImGui::GetCursorStartPos()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->DC.CursorStartPos - window->Pos;
}

ImVec2 ImGui::GetCursorScreenPos()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->DC.CursorPos;
}

void ImGui::SetCursorScreenPos(const ImVec2& screen_pos)
{
	ImGuiWindow* window = GetCurrentWindow();
	window->DC.CursorPos = screen_pos;
	window->DC.CursorMaxPos = ImMax(window->DC.CursorMaxPos, window->DC.CursorPos);
}

float ImGui::GetScrollX()
{
	return GImGui->CurrentWindow->Scroll.x;
}

float ImGui::GetScrollY()
{
	return GImGui->CurrentWindow->Scroll.y;
}

float ImGui::GetScrollMaxX()
{
	return GetScrollMaxX(GImGui->CurrentWindow);
}

float ImGui::GetScrollMaxY()
{
	return GetScrollMaxY(GImGui->CurrentWindow);
}

void ImGui::SetScrollX(float scroll_x)
{
	ImGuiWindow* window = GetCurrentWindow();
	window->ScrollTarget.x = scroll_x;
	window->ScrollTargetCenterRatio.x = 0.0f;
}

void ImGui::SetScrollY(float scroll_y)
{
	ImGuiWindow* window = GetCurrentWindow();
	window->ScrollTarget.y = scroll_y + window->TitleBarHeight() + window->MenuBarHeight(); // title bar height canceled out when using ScrollTargetRelY
	window->ScrollTargetCenterRatio.y = 0.0f;
}

void ImGui::SetScrollFromPosY(float pos_y, float center_y_ratio)
{
	// We store a target position so centering can occur on the next frame when we are guaranteed to have a known window size
	ImGuiWindow* window = GetCurrentWindow();
	IM_ASSERT(center_y_ratio >= 0.0f && center_y_ratio <= 1.0f);
	window->ScrollTarget.y = (float)(int)(pos_y + window->Scroll.y);
	window->ScrollTargetCenterRatio.y = center_y_ratio;

	// Minor hack to to make scrolling to top/bottom of window take account of WindowPadding, it looks more right to the user this way
	if (center_y_ratio <= 0.0f && window->ScrollTarget.y <= window->WindowPadding.y)
		window->ScrollTarget.y = 0.0f;
	else if (center_y_ratio >= 1.0f && window->ScrollTarget.y >= window->SizeContents.y - window->WindowPadding.y + GImGui->Style.ItemSpacing.y)
		window->ScrollTarget.y = window->SizeContents.y;
}

// center_y_ratio: 0.0f top of last item, 0.5f vertical center of last item, 1.0f bottom of last item.
void ImGui::SetScrollHere(float center_y_ratio)
{
	ImGuiWindow* window = GetCurrentWindow();
	float target_y = window->DC.CursorPosPrevLine.y - window->Pos.y; // Top of last item, in window space
	target_y += (window->DC.PrevLineHeight * center_y_ratio) + (GImGui->Style.ItemSpacing.y * (center_y_ratio - 0.5f) * 2.0f); // Precisely aim above, in the middle or below the last line.
	SetScrollFromPosY(target_y, center_y_ratio);
}

// FIXME-NAV: This function is a placeholder for the upcoming Navigation branch + Focusing features.
// In the current branch this function will only set the scrolling, in the navigation branch it will also set your navigation cursor.
// Prefer using "SetItemDefaultFocus()" over "if (IsWindowAppearing()) SetScrollHere()" when applicable.
void ImGui::SetItemDefaultFocus()
{
	if (IsWindowAppearing())
		SetScrollHere();
}

void ImGui::SetKeyboardFocusHere(int offset)
{
	IM_ASSERT(offset >= -1);    // -1 is allowed but not below
	ImGuiWindow* window = GetCurrentWindow();
	window->FocusIdxAllRequestNext = window->FocusIdxAllCounter + 1 + offset;
	window->FocusIdxTabRequestNext = INT_MAX;
}

void ImGui::SetStateStorage(ImGuiStorage* tree)
{
	ImGuiWindow* window = GetCurrentWindow();
	window->DC.StateStorage = tree ? tree : &window->StateStorage;
}

ImGuiStorage* ImGui::GetStateStorage()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->DC.StateStorage;
}

void ImGui::TextV(const char* fmt, va_list args)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const char* text_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
	TextUnformatted(g.TempBuffer, text_end);
}

void ImGui::Text(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TextV(fmt, args);
	va_end(args);
}

void ImGui::TextColoredV(const ImVec4& col, const char* fmt, va_list args)
{
	PushStyleColor(ImGuiCol_Text, col);
	TextV(fmt, args);
	PopStyleColor();
}

void ImGui::TextColored(const ImVec4& col, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TextColoredV(col, fmt, args);
	va_end(args);
}

void ImGui::TextDisabledV(const char* fmt, va_list args)
{
	PushStyleColor(ImGuiCol_Text, GImGui->Style.Colors[ImGuiCol_TextDisabled]);
	TextV(fmt, args);
	PopStyleColor();
}

void ImGui::TextDisabled(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TextDisabledV(fmt, args);
	va_end(args);
}

void ImGui::TextWrappedV(const char* fmt, va_list args)
{
	bool need_wrap = (GImGui->CurrentWindow->DC.TextWrapPos < 0.0f);    // Keep existing wrap position is one ia already set
	if (need_wrap) PushTextWrapPos(0.0f);
	TextV(fmt, args);
	if (need_wrap) PopTextWrapPos();
}

void ImGui::TextWrapped(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TextWrappedV(fmt, args);
	va_end(args);
}

void ImGui::TextUnformatted(const char* text, const char* text_end)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	IM_ASSERT(text != NULL);
	const char* text_begin = text;
	if (text_end == NULL)
		text_end = text + strlen(text); // FIXME-OPT

	const ImVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrentLineTextBaseOffset);
	const float wrap_pos_x = window->DC.TextWrapPos;
	const bool wrap_enabled = wrap_pos_x >= 0.0f;
	if (text_end - text > 2000 && !wrap_enabled)
	{
		// Long text!
		// Perform manual coarse clipping to optimize for long multi-line text
		// From this point we will only compute the width of lines that are visible. Optimization only available when word-wrapping is disabled.
		// We also don't vertically center the text within the line full height, which is unlikely to matter because we are likely the biggest and only item on the line.
		const char* line = text;
		const float line_height = GetTextLineHeight();
		const ImRect clip_rect = window->ClipRect;
		ImVec2 text_size(0, 0);

		if (text_pos.y <= clip_rect.Max.y)
		{
			ImVec2 pos = text_pos;

			// Lines to skip (can't skip when logging text)
			if (!g.LogEnabled)
			{
				int lines_skippable = (int)((clip_rect.Min.y - text_pos.y) / line_height);
				if (lines_skippable > 0)
				{
					int lines_skipped = 0;
					while (line < text_end && lines_skipped < lines_skippable)
					{
						const char* line_end = strchr(line, '\n');
						if (!line_end)
							line_end = text_end;
						line = line_end + 1;
						lines_skipped++;
					}
					pos.y += lines_skipped * line_height;
				}
			}

			// Lines to render
			if (line < text_end)
			{
				ImRect line_rect(pos, pos + ImVec2(FLT_MAX, line_height));
				while (line < text_end)
				{
					const char* line_end = strchr(line, '\n');
					if (IsClippedEx(line_rect, 0, false))
						break;

					const ImVec2 line_size = CalcTextSize(line, line_end, false);
					text_size.x = ImMax(text_size.x, line_size.x);
					RenderText(pos, line, line_end, false);
					if (!line_end)
						line_end = text_end;
					line = line_end + 1;
					line_rect.Min.y += line_height;
					line_rect.Max.y += line_height;
					pos.y += line_height;
				}

				// Count remaining lines
				int lines_skipped = 0;
				while (line < text_end)
				{
					const char* line_end = strchr(line, '\n');
					if (!line_end)
						line_end = text_end;
					line = line_end + 1;
					lines_skipped++;
				}
				pos.y += lines_skipped * line_height;
			}

			text_size.y += (pos - text_pos).y;
		}

		ImRect bb(text_pos, text_pos + text_size);
		ItemSize(bb);
		ItemAdd(bb, 0);
	}
	else
	{
		const float wrap_width = wrap_enabled ? CalcWrapWidthForPos(window->DC.CursorPos, wrap_pos_x) : 0.0f;
		const ImVec2 text_size = CalcTextSize(text_begin, text_end, false, wrap_width);

		// Account of baseline offset
		ImRect bb(text_pos, text_pos + text_size);
		ItemSize(text_size);
		if (!ItemAdd(bb, 0))
			return;

		// Render (we don't hide text after ## in this end-user function)
		RenderTextWrapped(bb.Min, text_begin, text_end, wrap_width);
	}
}

void ImGui::AlignTextToFramePadding()
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	window->DC.CurrentLineHeight = ImMax(window->DC.CurrentLineHeight, g.FontSize + g.Style.FramePadding.y * 2);
	window->DC.CurrentLineTextBaseOffset = ImMax(window->DC.CurrentLineTextBaseOffset, g.Style.FramePadding.y);
}

// Add a label+text combo aligned to other label+value widgets
void ImGui::LabelTextV(const char* label, const char* fmt, va_list args)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const float w = CalcItemWidth();

	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const ImRect value_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2));
	const ImRect total_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w + (label_size.x > 0.0f ? style.ItemInnerSpacing.x : 0.0f), style.FramePadding.y * 2) + label_size);
	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, 0))
		return;

	// Render
	const char* value_text_begin = &g.TempBuffer[0];
	const char* value_text_end = value_text_begin + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
	RenderTextClipped(value_bb.Min, value_bb.Max, value_text_begin, value_text_end, NULL, ImVec2(0.0f, 0.5f));
	if (label_size.x > 0.0f)
		RenderText(ImVec2(value_bb.Max.x + style.ItemInnerSpacing.x, value_bb.Min.y + style.FramePadding.y), label);
}

void ImGui::LabelText(const char* label, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	LabelTextV(label, fmt, args);
	va_end(args);
}

bool ImGui::ButtonBehavior(const ImRect& bb, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();

	if (flags & ImGuiButtonFlags_Disabled)
	{
		if (out_hovered) *out_hovered = false;
		if (out_held) *out_held = false;
		if (g.ActiveId == id) ClearActiveID();
		return false;
	}

	// Default behavior requires click+release on same spot
	if ((flags & (ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_PressedOnRelease | ImGuiButtonFlags_PressedOnDoubleClick)) == 0)
		flags |= ImGuiButtonFlags_PressedOnClickRelease;

	ImGuiWindow* backup_hovered_window = g.HoveredWindow;
	if ((flags & ImGuiButtonFlags_FlattenChildren) && g.HoveredRootWindow == window)
		g.HoveredWindow = window;

	bool pressed = false;
	bool hovered = ItemHoverable(bb, id);

	// Special mode for Drag and Drop where holding button pressed for a long time while dragging another item triggers the button
	if ((flags & ImGuiButtonFlags_PressedOnDragDropHold) && g.DragDropActive && !(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
		if (IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		{
			hovered = true;
			SetHoveredID(id);
			if (CalcTypematicPressedRepeatAmount(g.HoveredIdTimer + 0.0001f, g.HoveredIdTimer + 0.0001f - g.IO.DeltaTime, 0.01f, 0.70f)) // FIXME: Our formula for CalcTypematicPressedRepeatAmount() is fishy
			{
				pressed = true;
				FocusWindow(window);
			}
		}

	if ((flags & ImGuiButtonFlags_FlattenChildren) && g.HoveredRootWindow == window)
		g.HoveredWindow = backup_hovered_window;

	// AllowOverlap mode (rarely used) requires previous frame HoveredId to be null or to match. This allows using patterns where a later submitted widget overlaps a previous one.
	if (hovered && (flags & ImGuiButtonFlags_AllowItemOverlap) && (g.HoveredIdPreviousFrame != id && g.HoveredIdPreviousFrame != 0))
		hovered = false;

	if (hovered)
	{
		if (!(flags & ImGuiButtonFlags_NoKeyModifiers) || (!g.IO.KeyCtrl && !g.IO.KeyShift && !g.IO.KeyAlt))
		{
			//                        | CLICKING        | HOLDING with ImGuiButtonFlags_Repeat
			// PressedOnClickRelease  |  <on release>*  |  <on repeat> <on repeat> .. (NOT on release)  <-- MOST COMMON! (*) only if both click/release were over bounds
			// PressedOnClick         |  <on click>     |  <on click> <on repeat> <on repeat> ..
			// PressedOnRelease       |  <on release>   |  <on repeat> <on repeat> .. (NOT on release)
			// PressedOnDoubleClick   |  <on dclick>    |  <on dclick> <on repeat> <on repeat> ..
			if ((flags & ImGuiButtonFlags_PressedOnClickRelease) && g.IO.MouseClicked[0])
			{
				SetActiveID(id, window); // Hold on ID
				FocusWindow(window);
			}
			if (((flags & ImGuiButtonFlags_PressedOnClick) && g.IO.MouseClicked[0]) || ((flags & ImGuiButtonFlags_PressedOnDoubleClick) && g.IO.MouseDoubleClicked[0]))
			{
				pressed = true;
				if (flags & ImGuiButtonFlags_NoHoldingActiveID)
					ClearActiveID();
				else
					SetActiveID(id, window); // Hold on ID
				FocusWindow(window);
			}
			if ((flags & ImGuiButtonFlags_PressedOnRelease) && g.IO.MouseReleased[0])
			{
				if (!((flags & ImGuiButtonFlags_Repeat) && g.IO.MouseDownDurationPrev[0] >= g.IO.KeyRepeatDelay))  // Repeat mode trumps <on release>
					pressed = true;
				ClearActiveID();
			}

			// 'Repeat' mode acts when held regardless of _PressedOn flags (see table above). 
			// Relies on repeat logic of IsMouseClicked() but we may as well do it ourselves if we end up exposing finer RepeatDelay/RepeatRate settings.
			if ((flags & ImGuiButtonFlags_Repeat) && g.ActiveId == id && g.IO.MouseDownDuration[0] > 0.0f && IsMouseClicked(0, true))
				pressed = true;
		}
	}

	bool held = false;
	if (g.ActiveId == id)
	{
		if (g.ActiveIdIsJustActivated)
			g.ActiveIdClickOffset = g.IO.MousePos - bb.Min;
		if (g.IO.MouseDown[0])
		{
			held = true;
		}
		else
		{
			if (hovered && (flags & ImGuiButtonFlags_PressedOnClickRelease))
				if (!((flags & ImGuiButtonFlags_Repeat) && g.IO.MouseDownDurationPrev[0] >= g.IO.KeyRepeatDelay))  // Repeat mode trumps <on release>
					if (!g.DragDropActive)
						pressed = true;
			ClearActiveID();
		}
	}

	if (out_hovered) *out_hovered = hovered;
	if (out_held) *out_held = held;

	return pressed;
}

bool ImGui::ButtonEx(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();

	return pressed;
}

bool ImGui::Button(const char* label, const ImVec2& size_arg)
{
	return ButtonEx(label, size_arg, 0);
}


//NEWBUTTONS

bool ImGui::ButtonEx1(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive1 : hovered ? ImGuiCol_ButtonHovered1 : ImGuiCol_Button1);
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();

	return pressed;
}


bool ImGui::Button1(const char* label, const ImVec2& size_arg)
{
	return ButtonEx1(label, size_arg, 0);
}







bool ImGui::ButtonEx2(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive2 : hovered ? ImGuiCol_ButtonHovered2 : ImGuiCol_Button2);
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();

	return pressed;
}


bool ImGui::Button2(const char* label, const ImVec2& size_arg)
{
	return ButtonEx2(label, size_arg, 0);
}






bool ImGui::ButtonEx3(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive3 : hovered ? ImGuiCol_ButtonHovered3 : ImGuiCol_Button3);
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();

	return pressed;
}



bool ImGui::Button3(const char* label, const ImVec2& size_arg)
{
	return ButtonEx3(label, size_arg, 0);
}



bool ImGui::ButtonEx4(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive4 : hovered ? ImGuiCol_ButtonHovered4 : ImGuiCol_Button4);
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();

	return pressed;
}


bool ImGui::Button4(const char* label, const ImVec2& size_arg)
{
	return ButtonEx4(label, size_arg, 0);
}



bool ImGui::ButtonEx5(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive5 : hovered ? ImGuiCol_ButtonHovered5 : ImGuiCol_Button5);
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();

	return pressed;
}


bool ImGui::Button5(const char* label, const ImVec2& size_arg)
{
	return ButtonEx5(label, size_arg, 0);
}


bool ImGui::ButtonEx6(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive6 : hovered ? ImGuiCol_ButtonHovered6 : ImGuiCol_Button6);
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();

	return pressed;
}



bool ImGui::Button6(const char* label, const ImVec2& size_arg)
{
	return ButtonEx6(label, size_arg, 0);
}


bool ImGui::ButtonEx7(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive7 : hovered ? ImGuiCol_ButtonHovered7 : ImGuiCol_Button7);
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();

	return pressed;
}



bool ImGui::Button7(const char* label, const ImVec2& size_arg)
{
	return ButtonEx7(label, size_arg, 0);
}


//END OF NEW BUTTONS


// Small buttons fits within text without additional vertical spacing.
bool ImGui::SmallButton(const char* label)
{
	ImGuiContext& g = *GImGui;
	float backup_padding_y = g.Style.FramePadding.y;
	g.Style.FramePadding.y = 0.0f;
	bool pressed = ButtonEx(label, ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine);
	g.Style.FramePadding.y = backup_padding_y;
	return pressed;
}

// Tip: use ImGui::PushID()/PopID() to push indices or pointers in the ID stack.
// Then you can keep 'str_id' empty or the same for all your buttons (instead of creating a string based on a non-string id)
bool ImGui::InvisibleButton(const char* str_id, const ImVec2& size_arg)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	const ImGuiID id = window->GetID(str_id);
	ImVec2 size = CalcItemSize(size_arg, 0.0f, 0.0f);
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	ItemSize(bb);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);

	return pressed;
}

// Upper-right button to close a window.
bool ImGui::CloseButton(ImGuiID id, const ImVec2& pos, float radius)
{
	ImGuiWindow* window = GetCurrentWindow();

	const ImRect bb(pos - ImVec2(radius, radius), pos + ImVec2(radius, radius));

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);

	// Render
	const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_CloseButtonActive : hovered ? ImGuiCol_CloseButtonHovered : ImGuiCol_CloseButton);
	ImVec2 center = bb.GetCenter();
	window->DrawList->AddCircleFilled(center, ImMax(2.0f, radius), col, 12);

	const float cross_extent = (radius * 0.7071f) - 1.0f;
	if (hovered)
	{
		center -= ImVec2(0.5f, 0.5f);
		window->DrawList->AddLine(center + ImVec2(+cross_extent, +cross_extent), center + ImVec2(-cross_extent, -cross_extent), GetColorU32(ImGuiCol_Text));
		window->DrawList->AddLine(center + ImVec2(+cross_extent, -cross_extent), center + ImVec2(-cross_extent, +cross_extent), GetColorU32(ImGuiCol_Text));
	}

	return pressed;
}

// [Internal]
bool ImGui::ArrowButton(ImGuiID id, ImGuiDir dir, ImVec2 padding, ImGuiButtonFlags flags)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return false;

	const ImGuiStyle& style = g.Style;

	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(g.FontSize + padding.x * 2.0f, g.FontSize + padding.y * 2.0f));
	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
#ifdef IMGUI_HAS_NAV
	RenderNavHighlight(bb, id);
#endif
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	RenderTriangle(bb.Min + padding, dir, 1.0f);

	return pressed;
}

void ImGui::Image(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	if (border_col.w > 0.0f)
		bb.Max += ImVec2(2, 2);
	ItemSize(bb);
	if (!ItemAdd(bb, 0))
		return;

	if (border_col.w > 0.0f)
	{
		window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), 0.0f);
		window->DrawList->AddImage(user_texture_id, bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), uv0, uv1, GetColorU32(tint_col));
	}
	else
	{
		window->DrawList->AddImage(user_texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col));
	}
}

// frame_padding < 0: uses FramePadding from style (default)
// frame_padding = 0: no framing
// frame_padding > 0: set framing size
// The color used are the button colors.
bool ImGui::ImageButton(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	// Default to using texture ID as ID. User can still push string/integer prefixes.
	// We could hash the size/uv to create a unique ID but that would prevent the user from animating UV.
	PushID((void *)user_texture_id);
	const ImGuiID id = window->GetID("#image");
	PopID();

	const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
	const ImRect image_bb(window->DC.CursorPos + padding, window->DC.CursorPos + padding + size);
	ItemSize(bb);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);

	// Render
	const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
	if (bg_col.w > 0.0f)
		window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));
	window->DrawList->AddImage(user_texture_id, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(tint_col));

	return pressed;
}

// Start logging ImGui output to TTY
void ImGui::LogToTTY(int max_depth)
{
	ImGuiContext& g = *GImGui;
	if (g.LogEnabled)
		return;
	ImGuiWindow* window = g.CurrentWindow;

	IM_ASSERT(g.LogFile == NULL);
	g.LogFile = stdout;
	g.LogEnabled = true;
	g.LogStartDepth = window->DC.TreeDepth;
	if (max_depth >= 0)
		g.LogAutoExpandMaxDepth = max_depth;
}

// Start logging ImGui output to given file
void ImGui::LogToFile(int max_depth, const char* filename)
{
	ImGuiContext& g = *GImGui;
	if (g.LogEnabled)
		return;
	ImGuiWindow* window = g.CurrentWindow;

	if (!filename)
	{
		filename = g.IO.LogFilename;
		if (!filename)
			return;
	}

	IM_ASSERT(g.LogFile == NULL);
	g.LogFile = ImFileOpen(filename, "ab");
	if (!g.LogFile)
	{
		IM_ASSERT(g.LogFile != NULL); // Consider this an error
		return;
	}
	g.LogEnabled = true;
	g.LogStartDepth = window->DC.TreeDepth;
	if (max_depth >= 0)
		g.LogAutoExpandMaxDepth = max_depth;
}

// Start logging ImGui output to clipboard
void ImGui::LogToClipboard(int max_depth)
{
	ImGuiContext& g = *GImGui;
	if (g.LogEnabled)
		return;
	ImGuiWindow* window = g.CurrentWindow;

	IM_ASSERT(g.LogFile == NULL);
	g.LogFile = NULL;
	g.LogEnabled = true;
	g.LogStartDepth = window->DC.TreeDepth;
	if (max_depth >= 0)
		g.LogAutoExpandMaxDepth = max_depth;
}

void ImGui::LogFinish()
{
	ImGuiContext& g = *GImGui;
	if (!g.LogEnabled)
		return;

	LogText(IM_NEWLINE);
	if (g.LogFile != NULL)
	{
		if (g.LogFile == stdout)
			fflush(g.LogFile);
		else
			fclose(g.LogFile);
		g.LogFile = NULL;
	}
	if (g.LogClipboard->size() > 1)
	{
		SetClipboardText(g.LogClipboard->begin());
		g.LogClipboard->clear();
	}
	g.LogEnabled = false;
}

// Helper to display logging buttons
void ImGui::LogButtons()
{
	ImGuiContext& g = *GImGui;

	PushID("LogButtons");
	const bool log_to_tty = Button("Log To TTY"); SameLine();
	const bool log_to_file = Button("Log To File"); SameLine();
	const bool log_to_clipboard = Button("Log To Clipboard"); SameLine();
	PushItemWidth(80.0f);
	PushAllowKeyboardFocus(false);
	SliderInt("Depth", &g.LogAutoExpandMaxDepth, 0, 9, NULL);
	PopAllowKeyboardFocus();
	PopItemWidth();
	PopID();

	// Start logging at the end of the function so that the buttons don't appear in the log
	if (log_to_tty)
		LogToTTY(g.LogAutoExpandMaxDepth);
	if (log_to_file)
		LogToFile(g.LogAutoExpandMaxDepth, g.IO.LogFilename);
	if (log_to_clipboard)
		LogToClipboard(g.LogAutoExpandMaxDepth);
}

bool ImGui::TreeNodeBehaviorIsOpen(ImGuiID id, ImGuiTreeNodeFlags flags)
{
	if (flags & ImGuiTreeNodeFlags_Leaf)
		return true;

	// We only write to the tree storage if the user clicks (or explicitely use SetNextTreeNode*** functions)
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiStorage* storage = window->DC.StateStorage;

	bool is_open;
	if (g.NextTreeNodeOpenCond != 0)
	{
		if (g.NextTreeNodeOpenCond & ImGuiCond_Always)
		{
			is_open = g.NextTreeNodeOpenVal;
			storage->SetInt(id, is_open);
		}
		else
		{
			// We treat ImGuiCond_Once and ImGuiCond_FirstUseEver the same because tree node state are not saved persistently.
			const int stored_value = storage->GetInt(id, -1);
			if (stored_value == -1)
			{
				is_open = g.NextTreeNodeOpenVal;
				storage->SetInt(id, is_open);
			}
			else
			{
				is_open = stored_value != 0;
			}
		}
		g.NextTreeNodeOpenCond = 0;
	}
	else
	{
		is_open = storage->GetInt(id, (flags & ImGuiTreeNodeFlags_DefaultOpen) ? 1 : 0) != 0;
	}

	// When logging is enabled, we automatically expand tree nodes (but *NOT* collapsing headers.. seems like sensible behavior).
	// NB- If we are above max depth we still allow manually opened nodes to be logged.
	if (g.LogEnabled && !(flags & ImGuiTreeNodeFlags_NoAutoOpenOnLog) && window->DC.TreeDepth < g.LogAutoExpandMaxDepth)
		is_open = true;

	return is_open;
}

bool ImGui::TreeNodeBehavior(ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const bool display_frame = (flags & ImGuiTreeNodeFlags_Framed) != 0;
	const ImVec2 padding = (display_frame || (flags & ImGuiTreeNodeFlags_FramePadding)) ? style.FramePadding : ImVec2(style.FramePadding.x, 0.0f);

	if (!label_end)
		label_end = FindRenderedTextEnd(label);
	const ImVec2 label_size = CalcTextSize(label, label_end, false);

	// We vertically grow up to current line height up the typical widget height.
	const float text_base_offset_y = ImMax(padding.y, window->DC.CurrentLineTextBaseOffset); // Latch before ItemSize changes it
	const float frame_height = ImMax(ImMin(window->DC.CurrentLineHeight, g.FontSize + style.FramePadding.y * 2), label_size.y + padding.y * 2);
	ImRect bb = ImRect(window->DC.CursorPos, ImVec2(window->Pos.x + GetContentRegionMax().x, window->DC.CursorPos.y + frame_height));
	if (display_frame)
	{
		// Framed header expand a little outside the default padding
		bb.Min.x -= (float)(int)(window->WindowPadding.x*0.5f) - 1;
		bb.Max.x += (float)(int)(window->WindowPadding.x*0.5f) - 1;
	}

	const float text_offset_x = (g.FontSize + (display_frame ? padding.x * 3 : padding.x * 2));   // Collapser arrow width + Spacing
	const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);   // Include collapser
	ItemSize(ImVec2(text_width, frame_height), text_base_offset_y);

	// For regular tree nodes, we arbitrary allow to click past 2 worth of ItemSpacing
	// (Ideally we'd want to add a flag for the user to specify if we want the hit test to be done up to the right side of the content or not)
	const ImRect interact_bb = display_frame ? bb : ImRect(bb.Min.x, bb.Min.y, bb.Min.x + text_width + style.ItemSpacing.x * 2, bb.Max.y);
	bool is_open = TreeNodeBehaviorIsOpen(id, flags);
	if (!ItemAdd(interact_bb, id))
	{
		if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
			TreePushRawID(id);
		return is_open;
	}

	// Flags that affects opening behavior:
	// - 0(default) ..................... single-click anywhere to open
	// - OpenOnDoubleClick .............. double-click anywhere to open
	// - OpenOnArrow .................... single-click on arrow to open
	// - OpenOnDoubleClick|OpenOnArrow .. single-click on arrow or double-click anywhere to open
	ImGuiButtonFlags button_flags = ImGuiButtonFlags_NoKeyModifiers | ((flags & ImGuiTreeNodeFlags_AllowItemOverlap) ? ImGuiButtonFlags_AllowItemOverlap : 0);
	if (!(flags & ImGuiTreeNodeFlags_Leaf))
		button_flags |= ImGuiButtonFlags_PressedOnDragDropHold;
	if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
		button_flags |= ImGuiButtonFlags_PressedOnDoubleClick | ((flags & ImGuiTreeNodeFlags_OpenOnArrow) ? ImGuiButtonFlags_PressedOnClickRelease : 0);

	bool hovered, held, pressed = ButtonBehavior(interact_bb, id, &hovered, &held, button_flags);
	if (pressed && !(flags & ImGuiTreeNodeFlags_Leaf))
	{
		bool toggled = !(flags & (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick));
		if (flags & ImGuiTreeNodeFlags_OpenOnArrow)
			toggled |= IsMouseHoveringRect(interact_bb.Min, ImVec2(interact_bb.Min.x + text_offset_x, interact_bb.Max.y));
		if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
			toggled |= g.IO.MouseDoubleClicked[0];
		if (g.DragDropActive && is_open) // When using Drag and Drop "hold to open" we keep the node highlighted after opening, but never close it again.
			toggled = false;
		if (toggled)
		{
			is_open = !is_open;
			window->DC.StateStorage->SetInt(id, is_open);
		}
	}
	if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
		SetItemAllowOverlap();

	// Render
	const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
	const ImVec2 text_pos = bb.Min + ImVec2(text_offset_x, text_base_offset_y);
	if (display_frame)
	{
		// Framed type
		RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
		RenderTriangle(bb.Min + ImVec2(padding.x, text_base_offset_y), is_open ? ImGuiDir_Down : ImGuiDir_Right, 1.0f);
		if (g.LogEnabled)
		{
			// NB: '##' is normally used to hide text (as a library-wide feature), so we need to specify the text range to make sure the ## aren't stripped out here.
			const char log_prefix[] = "\n##";
			const char log_suffix[] = "##";
			LogRenderedText(&text_pos, log_prefix, log_prefix + 3);
			RenderTextClipped(text_pos, bb.Max, label, label_end, &label_size);
			LogRenderedText(&text_pos, log_suffix + 1, log_suffix + 3);
		}
		else
		{
			RenderTextClipped(text_pos, bb.Max, label, label_end, &label_size);
		}
	}
	else
	{
		// Unframed typed for tree nodes
		if (hovered || (flags & ImGuiTreeNodeFlags_Selected))
			RenderFrame(bb.Min, bb.Max, col, false);

		if (flags & ImGuiTreeNodeFlags_Bullet)
			RenderBullet(bb.Min + ImVec2(text_offset_x * 0.5f, g.FontSize*0.50f + text_base_offset_y));
		else if (!(flags & ImGuiTreeNodeFlags_Leaf))
			RenderTriangle(bb.Min + ImVec2(padding.x, g.FontSize*0.15f + text_base_offset_y), is_open ? ImGuiDir_Down : ImGuiDir_Right, 0.70f);
		if (g.LogEnabled)
			LogRenderedText(&text_pos, ">");
		RenderText(text_pos, label, label_end, false);
	}

	if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
		TreePushRawID(id);
	return is_open;
}

// CollapsingHeader returns true when opened but do not indent nor push into the ID stack (because of the ImGuiTreeNodeFlags_NoTreePushOnOpen flag).
// This is basically the same as calling TreeNodeEx(label, ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_NoTreePushOnOpen). You can remove the _NoTreePushOnOpen flag if you want behavior closer to normal TreeNode().
bool ImGui::CollapsingHeader(const char* label, ImGuiTreeNodeFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	return TreeNodeBehavior(window->GetID(label), flags | ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_NoTreePushOnOpen, label);
}

bool ImGui::CollapsingHeader(const char* label, bool* p_open, ImGuiTreeNodeFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	if (p_open && !*p_open)
		return false;

	ImGuiID id = window->GetID(label);
	bool is_open = TreeNodeBehavior(id, flags | ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_NoTreePushOnOpen | (p_open ? ImGuiTreeNodeFlags_AllowItemOverlap : 0), label);
	if (p_open)
	{
		// Create a small overlapping close button // FIXME: We can evolve this into user accessible helpers to add extra buttons on title bars, headers, etc.
		ImGuiContext& g = *GImGui;
		float button_sz = g.FontSize * 0.5f;
		ImGuiItemHoveredDataBackup last_item_backup;
		if (CloseButton(window->GetID((void*)(intptr_t)(id + 1)), ImVec2(ImMin(window->DC.LastItemRect.Max.x, window->ClipRect.Max.x) - g.Style.FramePadding.x - button_sz, window->DC.LastItemRect.Min.y + g.Style.FramePadding.y + button_sz), button_sz))
			*p_open = false;
		last_item_backup.Restore();
	}

	return is_open;
}

bool ImGui::TreeNodeEx(const char* label, ImGuiTreeNodeFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	return TreeNodeBehavior(window->GetID(label), flags, label, NULL);
}

bool ImGui::TreeNodeExV(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const char* label_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
	return TreeNodeBehavior(window->GetID(str_id), flags, g.TempBuffer, label_end);
}

bool ImGui::TreeNodeExV(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const char* label_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
	return TreeNodeBehavior(window->GetID(ptr_id), flags, g.TempBuffer, label_end);
}

bool ImGui::TreeNodeV(const char* str_id, const char* fmt, va_list args)
{
	return TreeNodeExV(str_id, 0, fmt, args);
}

bool ImGui::TreeNodeV(const void* ptr_id, const char* fmt, va_list args)
{
	return TreeNodeExV(ptr_id, 0, fmt, args);
}

bool ImGui::TreeNodeEx(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	bool is_open = TreeNodeExV(str_id, flags, fmt, args);
	va_end(args);
	return is_open;
}

bool ImGui::TreeNodeEx(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	bool is_open = TreeNodeExV(ptr_id, flags, fmt, args);
	va_end(args);
	return is_open;
}

bool ImGui::TreeNode(const char* str_id, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	bool is_open = TreeNodeExV(str_id, 0, fmt, args);
	va_end(args);
	return is_open;
}

bool ImGui::TreeNode(const void* ptr_id, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	bool is_open = TreeNodeExV(ptr_id, 0, fmt, args);
	va_end(args);
	return is_open;
}

bool ImGui::TreeNode(const char* label)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;
	return TreeNodeBehavior(window->GetID(label), 0, label, NULL);
}

void ImGui::TreeAdvanceToLabelPos()
{
	ImGuiContext& g = *GImGui;
	g.CurrentWindow->DC.CursorPos.x += GetTreeNodeToLabelSpacing();
}

// Horizontal distance preceding label when using TreeNode() or Bullet()
float ImGui::GetTreeNodeToLabelSpacing()
{
	ImGuiContext& g = *GImGui;
	return g.FontSize + (g.Style.FramePadding.x * 2.0f);
}

void ImGui::SetNextTreeNodeOpen(bool is_open, ImGuiCond cond)
{
	ImGuiContext& g = *GImGui;
	if (g.CurrentWindow->SkipItems)
		return;
	g.NextTreeNodeOpenVal = is_open;
	g.NextTreeNodeOpenCond = cond ? cond : ImGuiCond_Always;
}

void ImGui::PushID(const char* str_id)
{
	ImGuiWindow* window = GetCurrentWindowRead();
	window->IDStack.push_back(window->GetID(str_id));
}

void ImGui::PushID(const char* str_id_begin, const char* str_id_end)
{
	ImGuiWindow* window = GetCurrentWindowRead();
	window->IDStack.push_back(window->GetID(str_id_begin, str_id_end));
}

void ImGui::PushID(const void* ptr_id)
{
	ImGuiWindow* window = GetCurrentWindowRead();
	window->IDStack.push_back(window->GetID(ptr_id));
}

void ImGui::PushID(int int_id)
{
	const void* ptr_id = (void*)(intptr_t)int_id;
	ImGuiWindow* window = GetCurrentWindowRead();
	window->IDStack.push_back(window->GetID(ptr_id));
}

void ImGui::PopID()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	window->IDStack.pop_back();
}

ImGuiID ImGui::GetID(const char* str_id)
{
	return GImGui->CurrentWindow->GetID(str_id);
}

ImGuiID ImGui::GetID(const char* str_id_begin, const char* str_id_end)
{
	return GImGui->CurrentWindow->GetID(str_id_begin, str_id_end);
}

ImGuiID ImGui::GetID(const void* ptr_id)
{
	return GImGui->CurrentWindow->GetID(ptr_id);
}

void ImGui::Bullet()
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const float line_height = ImMax(ImMin(window->DC.CurrentLineHeight, g.FontSize + g.Style.FramePadding.y * 2), g.FontSize);
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(g.FontSize, line_height));
	ItemSize(bb);
	if (!ItemAdd(bb, 0))
	{
		SameLine(0, style.FramePadding.x * 2);
		return;
	}

	// Render and stay on same line
	RenderBullet(bb.Min + ImVec2(style.FramePadding.x + g.FontSize*0.5f, line_height*0.5f));
	SameLine(0, style.FramePadding.x * 2);
}

// Text with a little bullet aligned to the typical tree node.
void ImGui::BulletTextV(const char* fmt, va_list args)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	const char* text_begin = g.TempBuffer;
	const char* text_end = text_begin + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
	const ImVec2 label_size = CalcTextSize(text_begin, text_end, false);
	const float text_base_offset_y = ImMax(0.0f, window->DC.CurrentLineTextBaseOffset); // Latch before ItemSize changes it
	const float line_height = ImMax(ImMin(window->DC.CurrentLineHeight, g.FontSize + g.Style.FramePadding.y * 2), g.FontSize);
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(g.FontSize + (label_size.x > 0.0f ? (label_size.x + style.FramePadding.x * 2) : 0.0f), ImMax(line_height, label_size.y)));  // Empty text doesn't add padding
	ItemSize(bb);
	if (!ItemAdd(bb, 0))
		return;

	// Render
	RenderBullet(bb.Min + ImVec2(style.FramePadding.x + g.FontSize*0.5f, line_height*0.5f));
	RenderText(bb.Min + ImVec2(g.FontSize + style.FramePadding.x * 2, text_base_offset_y), text_begin, text_end, false);
}

void ImGui::BulletText(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	BulletTextV(fmt, args);
	va_end(args);
}

static inline void DataTypeFormatString(ImGuiDataType data_type, void* data_ptr, const char* display_format, char* buf, int buf_size)
{
	if (data_type == ImGuiDataType_Int)
		ImFormatString(buf, buf_size, display_format, *(int*)data_ptr);
	else if (data_type == ImGuiDataType_Float)
		ImFormatString(buf, buf_size, display_format, *(float*)data_ptr);
}

static inline void DataTypeFormatString(ImGuiDataType data_type, void* data_ptr, int decimal_precision, char* buf, int buf_size)
{
	if (data_type == ImGuiDataType_Int)
	{
		if (decimal_precision < 0)
			ImFormatString(buf, buf_size, "%d", *(int*)data_ptr);
		else
			ImFormatString(buf, buf_size, "%.*d", decimal_precision, *(int*)data_ptr);
	}
	else if (data_type == ImGuiDataType_Float)
	{
		if (decimal_precision < 0)
			ImFormatString(buf, buf_size, "%f", *(float*)data_ptr);     // Ideally we'd have a minimum decimal precision of 1 to visually denote that it is a float, while hiding non-significant digits?
		else
			ImFormatString(buf, buf_size, "%.*f", decimal_precision, *(float*)data_ptr);
	}
}

static void DataTypeApplyOp(ImGuiDataType data_type, int op, void* value1, const void* value2)// Store into value1
{
	if (data_type == ImGuiDataType_Int)
	{
		if (op == '+')
			*(int*)value1 = *(int*)value1 + *(const int*)value2;
		else if (op == '-')
			*(int*)value1 = *(int*)value1 - *(const int*)value2;
	}
	else if (data_type == ImGuiDataType_Float)
	{
		if (op == '+')
			*(float*)value1 = *(float*)value1 + *(const float*)value2;
		else if (op == '-')
			*(float*)value1 = *(float*)value1 - *(const float*)value2;
	}
}

// User can input math operators (e.g. +100) to edit a numerical values.
static bool DataTypeApplyOpFromText(const char* buf, const char* initial_value_buf, ImGuiDataType data_type, void* data_ptr, const char* scalar_format)
{
	while (ImCharIsSpace(*buf))
		buf++;

	// We don't support '-' op because it would conflict with inputing negative value.
	// Instead you can use +-100 to subtract from an existing value
	char op = buf[0];
	if (op == '+' || op == '*' || op == '/')
	{
		buf++;
		while (ImCharIsSpace(*buf))
			buf++;
	}
	else
	{
		op = 0;
	}
	if (!buf[0])
		return false;

	if (data_type == ImGuiDataType_Int)
	{
		if (!scalar_format)
			scalar_format = "%d";
		int* v = (int*)data_ptr;
		const int old_v = *v;
		int arg0i = *v;
		if (op && sscanf(initial_value_buf, scalar_format, &arg0i) < 1)
			return false;

		// Store operand in a float so we can use fractional value for multipliers (*1.1), but constant always parsed as integer so we can fit big integers (e.g. 2000000003) past float precision
		float arg1f = 0.0f;
		if (op == '+') { if (sscanf(buf, "%f", &arg1f) == 1) *v = (int)(arg0i + arg1f); }                 // Add (use "+-" to subtract)
		else if (op == '*') { if (sscanf(buf, "%f", &arg1f) == 1) *v = (int)(arg0i * arg1f); }                 // Multiply
		else if (op == '/') { if (sscanf(buf, "%f", &arg1f) == 1 && arg1f != 0.0f) *v = (int)(arg0i / arg1f); }// Divide
		else { if (sscanf(buf, scalar_format, &arg0i) == 1) *v = arg0i; }                       // Assign constant (read as integer so big values are not lossy)
		return (old_v != *v);
	}
	else if (data_type == ImGuiDataType_Float)
	{
		// For floats we have to ignore format with precision (e.g. "%.2f") because sscanf doesn't take them in
		scalar_format = "%f";
		float* v = (float*)data_ptr;
		const float old_v = *v;
		float arg0f = *v;
		if (op && sscanf(initial_value_buf, scalar_format, &arg0f) < 1)
			return false;

		float arg1f = 0.0f;
		if (sscanf(buf, scalar_format, &arg1f) < 1)
			return false;
		if (op == '+') { *v = arg0f + arg1f; }                    // Add (use "+-" to subtract)
		else if (op == '*') { *v = arg0f * arg1f; }                    // Multiply
		else if (op == '/') { if (arg1f != 0.0f) *v = arg0f / arg1f; } // Divide
		else { *v = arg1f; }                            // Assign constant
		return (old_v != *v);
	}

	return false;
}

// Create text input in place of a slider (when CTRL+Clicking on slider)
// FIXME: Logic is messy and confusing.
bool ImGui::InputScalarAsWidgetReplacement(const ImRect& aabb, const char* label, ImGuiDataType data_type, void* data_ptr, ImGuiID id, int decimal_precision)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();

	// Our replacement widget will override the focus ID (registered previously to allow for a TAB focus to happen)
	// On the first frame, g.ScalarAsInputTextId == 0, then on subsequent frames it becomes == id
	SetActiveID(g.ScalarAsInputTextId, window);
	SetHoveredID(0);
	FocusableItemUnregister(window);

	char buf[32];
	DataTypeFormatString(data_type, data_ptr, decimal_precision, buf, IM_ARRAYSIZE(buf));
	bool text_value_changed = InputTextEx(label, buf, IM_ARRAYSIZE(buf), aabb.GetSize(), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll);
	if (g.ScalarAsInputTextId == 0)     // First frame we started displaying the InputText widget
	{
		IM_ASSERT(g.ActiveId == id);    // InputText ID expected to match the Slider ID (else we'd need to store them both, which is also possible)
		g.ScalarAsInputTextId = g.ActiveId;
		SetHoveredID(id);
	}
	if (text_value_changed)
		return DataTypeApplyOpFromText(buf, GImGui->InputTextState.InitialText.begin(), data_type, data_ptr, NULL);
	return false;
}

// Parse display precision back from the display format string
int ImGui::ParseFormatPrecision(const char* fmt, int default_precision)
{
	int precision = default_precision;
	while ((fmt = strchr(fmt, '%')) != NULL)
	{
		fmt++;
		if (fmt[0] == '%') { fmt++; continue; } // Ignore "%%"
		while (*fmt >= '0' && *fmt <= '9')
			fmt++;
		if (*fmt == '.')
		{
			fmt = ImAtoi(fmt + 1, &precision);
			if (precision < 0 || precision > 10)
				precision = default_precision;
		}
		if (*fmt == 'e' || *fmt == 'E') // Maximum precision with scientific notation
			precision = -1;
		break;
	}
	return precision;
}

static float GetMinimumStepAtDecimalPrecision(int decimal_precision)
{
	static const float min_steps[10] = { 1.0f, 0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f, 0.000001f, 0.0000001f, 0.00000001f, 0.000000001f };
	return (decimal_precision >= 0 && decimal_precision < 10) ? min_steps[decimal_precision] : powf(10.0f, (float)-decimal_precision);
}

float ImGui::RoundScalar(float value, int decimal_precision)
{
	// Round past decimal precision
	// So when our value is 1.99999 with a precision of 0.001 we'll end up rounding to 2.0
	// FIXME: Investigate better rounding methods
	if (decimal_precision < 0)
		return value;
	const float min_step = GetMinimumStepAtDecimalPrecision(decimal_precision);
	bool negative = value < 0.0f;
	value = fabsf(value);
	float remainder = fmodf(value, min_step);
	if (remainder <= min_step*0.5f)
		value -= remainder;
	else
		value += (min_step - remainder);
	return negative ? -value : value;
}

static inline float SliderBehaviorCalcRatioFromValue(float v, float v_min, float v_max, float power, float linear_zero_pos)
{
	if (v_min == v_max)
		return 0.0f;

	const bool is_non_linear = (power < 1.0f - 0.00001f) || (power > 1.0f + 0.00001f);
	const float v_clamped = (v_min < v_max) ? ImClamp(v, v_min, v_max) : ImClamp(v, v_max, v_min);
	if (is_non_linear)
	{
		if (v_clamped < 0.0f)
		{
			const float f = 1.0f - (v_clamped - v_min) / (ImMin(0.0f, v_max) - v_min);
			return (1.0f - powf(f, 1.0f / power)) * linear_zero_pos;
		}
		else
		{
			const float f = (v_clamped - ImMax(0.0f, v_min)) / (v_max - ImMax(0.0f, v_min));
			return linear_zero_pos + powf(f, 1.0f / power) * (1.0f - linear_zero_pos);
		}
	}

	// Linear slider
	return (v_clamped - v_min) / (v_max - v_min);
}

bool ImGui::SliderBehavior(const ImRect& frame_bb, ImGuiID id, float* v, float v_min, float v_max, float power, int decimal_precision, ImGuiSliderFlags flags)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();
	const ImGuiStyle& style = g.Style;

	// Draw frame
	RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

	const bool is_non_linear = (power < 1.0f - 0.00001f) || (power > 1.0f + 0.00001f);
	const bool is_horizontal = (flags & ImGuiSliderFlags_Vertical) == 0;

	const float grab_padding = 2.0f;
	const float slider_sz = is_horizontal ? (frame_bb.GetWidth() - grab_padding * 2.0f) : (frame_bb.GetHeight() - grab_padding * 2.0f);
	float grab_sz;
	if (decimal_precision > 0)
		grab_sz = ImMin(style.GrabMinSize, slider_sz);
	else
		grab_sz = ImMin(ImMax(1.0f * (slider_sz / ((v_min < v_max ? v_max - v_min : v_min - v_max) + 1.0f)), style.GrabMinSize), slider_sz);  // Integer sliders, if possible have the grab size represent 1 unit
	const float slider_usable_sz = slider_sz - grab_sz;
	const float slider_usable_pos_min = (is_horizontal ? frame_bb.Min.x : frame_bb.Min.y) + grab_padding + grab_sz * 0.5f;
	const float slider_usable_pos_max = (is_horizontal ? frame_bb.Max.x : frame_bb.Max.y) - grab_padding - grab_sz * 0.5f;

	// For logarithmic sliders that cross over sign boundary we want the exponential increase to be symmetric around 0.0f
	float linear_zero_pos = 0.0f;   // 0.0->1.0f
	if (v_min * v_max < 0.0f)
	{
		// Different sign
		const float linear_dist_min_to_0 = powf(fabsf(0.0f - v_min), 1.0f / power);
		const float linear_dist_max_to_0 = powf(fabsf(v_max - 0.0f), 1.0f / power);
		linear_zero_pos = linear_dist_min_to_0 / (linear_dist_min_to_0 + linear_dist_max_to_0);
	}
	else
	{
		// Same sign
		linear_zero_pos = v_min < 0.0f ? 1.0f : 0.0f;
	}

	// Process clicking on the slider
	bool value_changed = false;
	if (g.ActiveId == id)
	{
		if (g.IO.MouseDown[0])
		{
			const float mouse_abs_pos = is_horizontal ? g.IO.MousePos.x : g.IO.MousePos.y;
			float clicked_t = (slider_usable_sz > 0.0f) ? ImClamp((mouse_abs_pos - slider_usable_pos_min) / slider_usable_sz, 0.0f, 1.0f) : 0.0f;
			if (!is_horizontal)
				clicked_t = 1.0f - clicked_t;

			float new_value;
			if (is_non_linear)
			{
				// Account for logarithmic scale on both sides of the zero
				if (clicked_t < linear_zero_pos)
				{
					// Negative: rescale to the negative range before powering
					float a = 1.0f - (clicked_t / linear_zero_pos);
					a = powf(a, power);
					new_value = ImLerp(ImMin(v_max, 0.0f), v_min, a);
				}
				else
				{
					// Positive: rescale to the positive range before powering
					float a;
					if (fabsf(linear_zero_pos - 1.0f) > 1.e-6f)
						a = (clicked_t - linear_zero_pos) / (1.0f - linear_zero_pos);
					else
						a = clicked_t;
					a = powf(a, power);
					new_value = ImLerp(ImMax(v_min, 0.0f), v_max, a);
				}
			}
			else
			{
				// Linear slider
				new_value = ImLerp(v_min, v_max, clicked_t);
			}

			// Round past decimal precision
			new_value = RoundScalar(new_value, decimal_precision);
			if (*v != new_value)
			{
				*v = new_value;
				value_changed = true;
			}
		}
		else
		{
			ClearActiveID();
		}
	}

	// Calculate slider grab positioning
	float grab_t = SliderBehaviorCalcRatioFromValue(*v, v_min, v_max, power, linear_zero_pos);

	// Draw
	if (!is_horizontal)
		grab_t = 1.0f - grab_t;
	const float grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
	ImRect grab_bb;
	if (is_horizontal)
		grab_bb = ImRect(ImVec2(grab_pos - grab_sz * 0.5f, frame_bb.Min.y), ImVec2(grab_pos + grab_sz * 0.5f, frame_bb.Max.y));
	else
		grab_bb = ImRect(ImVec2(frame_bb.Min.x + grab_padding, grab_pos - grab_sz * 0.5f), ImVec2(frame_bb.Max.x - grab_padding, grab_pos + grab_sz * 0.5f));
	window->DrawList->AddRectFilled(frame_bb.Min + ImVec2(2, 2), grab_bb.Max - ImVec2(0, 2), GetColorU32(ImGuiCol_SliderGrabActive), style.GrabRounding);

	return value_changed;
}

// Use power!=1.0 for logarithmic sliders.
// Adjust display_format to decorate the value with a prefix or a suffix.
//   "%.3f"         1.234
//   "%5.2f secs"   01.23 secs
//   "Gold: %.0f"   Gold: 1
bool ImGui::SliderFloat(const char* label, float* v, float v_min, float v_max, const char* display_format, float power)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const float w = CalcItemWidth();

	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y*2.0f));
	const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

	// NB- we don't call ItemSize() yet because we may turn into a text edit box below
	if (!ItemAdd(total_bb, id))
	{
		ItemSize(total_bb, style.FramePadding.y);
		return false;
	}
	const bool hovered = ItemHoverable(frame_bb, id);

	if (!display_format)
		display_format = "%.3f";
	int decimal_precision = ParseFormatPrecision(display_format, 3);

	// Tabbing or CTRL-clicking on Slider turns it into an input box
	bool start_text_input = false;
	const bool tab_focus_requested = FocusableItemRegister(window, id);
	if (tab_focus_requested || (hovered && g.IO.MouseClicked[0]))
	{
		SetActiveID(id, window);
		FocusWindow(window);
		if (tab_focus_requested || g.IO.KeyCtrl)
		{
			start_text_input = true;
			g.ScalarAsInputTextId = 0;
		}
	}
	if (start_text_input || (g.ActiveId == id && g.ScalarAsInputTextId == id))
		return InputScalarAsWidgetReplacement(frame_bb, label, ImGuiDataType_Float, v, id, decimal_precision);

	// Actual slider behavior + render grab
	ItemSize(total_bb, style.FramePadding.y);
	const bool value_changed = SliderBehavior(frame_bb, id, v, v_min, v_max, power, decimal_precision);

	// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
	char value_buf[64];
	const char* value_buf_end = value_buf + ImFormatString(value_buf, IM_ARRAYSIZE(value_buf), display_format, *v);
	RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));

	if (label_size.x > 0.0f)
		RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

	return value_changed;
}

bool ImGui::VSliderFloat(const char* label, const ImVec2& size, float* v, float v_min, float v_max, const char* display_format, float power)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);

	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
	const ImRect bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(frame_bb, id))
		return false;
	const bool hovered = ItemHoverable(frame_bb, id);

	if (!display_format)
		display_format = "%.3f";
	int decimal_precision = ParseFormatPrecision(display_format, 3);

	if (hovered && g.IO.MouseClicked[0])
	{
		SetActiveID(id, window);
		FocusWindow(window);
	}

	// Actual slider behavior + render grab
	bool value_changed = SliderBehavior(frame_bb, id, v, v_min, v_max, power, decimal_precision, ImGuiSliderFlags_Vertical);

	// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
	// For the vertical slider we allow centered text to overlap the frame padding
	char value_buf[64];
	char* value_buf_end = value_buf + ImFormatString(value_buf, IM_ARRAYSIZE(value_buf), display_format, *v);
	RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.0f));
	if (label_size.x > 0.0f)
		RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

	return value_changed;
}

bool ImGui::SliderAngle(const char* label, float* v_rad, float v_degrees_min, float v_degrees_max)
{
	float v_deg = (*v_rad) * 360.0f / (2 * IM_PI);
	bool value_changed = SliderFloat(label, &v_deg, v_degrees_min, v_degrees_max, "%.0f deg", 1.0f);
	*v_rad = v_deg * (2 * IM_PI) / 360.0f;
	return value_changed;
}

bool ImGui::SliderInt(const char* label, int* v, int v_min, int v_max, const char* display_format)
{
	if (!display_format)
		display_format = "%.0f";
	float v_f = (float)*v;
	bool value_changed = SliderFloat(label, &v_f, (float)v_min, (float)v_max, display_format, 1.0f);
	*v = (int)v_f;
	return value_changed;
}

bool ImGui::VSliderInt(const char* label, const ImVec2& size, int* v, int v_min, int v_max, const char* display_format)
{
	if (!display_format)
		display_format = "%.0f";
	float v_f = (float)*v;
	bool value_changed = VSliderFloat(label, size, &v_f, (float)v_min, (float)v_max, display_format, 1.0f);
	*v = (int)v_f;
	return value_changed;
}

// Add multiple sliders on 1 line for compact edition of multiple components
bool ImGui::SliderFloatN(const char* label, float* v, int components, float v_min, float v_max, const char* display_format, float power)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	bool value_changed = false;
	BeginGroup();
	PushID(label);
	PushMultiItemsWidths(components);
	for (int i = 0; i < components; i++)
	{
		PushID(i);
		value_changed |= SliderFloat("##v", &v[i], v_min, v_max, display_format, power);
		SameLine(0, g.Style.ItemInnerSpacing.x);
		PopID();
		PopItemWidth();
	}
	PopID();

	TextUnformatted(label, FindRenderedTextEnd(label));
	EndGroup();

	return value_changed;
}

bool ImGui::SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* display_format, float power)
{
	return SliderFloatN(label, v, 2, v_min, v_max, display_format, power);
}

bool ImGui::SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* display_format, float power)
{
	return SliderFloatN(label, v, 3, v_min, v_max, display_format, power);
}

bool ImGui::SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* display_format, float power)
{
	return SliderFloatN(label, v, 4, v_min, v_max, display_format, power);
}

bool ImGui::SliderIntN(const char* label, int* v, int components, int v_min, int v_max, const char* display_format)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	bool value_changed = false;
	BeginGroup();
	PushID(label);
	PushMultiItemsWidths(components);
	for (int i = 0; i < components; i++)
	{
		PushID(i);
		value_changed |= SliderInt("##v", &v[i], v_min, v_max, display_format);
		SameLine(0, g.Style.ItemInnerSpacing.x);
		PopID();
		PopItemWidth();
	}
	PopID();

	TextUnformatted(label, FindRenderedTextEnd(label));
	EndGroup();

	return value_changed;
}

bool ImGui::SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* display_format)
{
	return SliderIntN(label, v, 2, v_min, v_max, display_format);
}

bool ImGui::SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* display_format)
{
	return SliderIntN(label, v, 3, v_min, v_max, display_format);
}

bool ImGui::SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* display_format)
{
	return SliderIntN(label, v, 4, v_min, v_max, display_format);
}

bool ImGui::DragBehavior(const ImRect& frame_bb, ImGuiID id, float* v, float v_speed, float v_min, float v_max, int decimal_precision, float power)
{
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	// Draw frame
	const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
	RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, style.FrameRounding);

	bool value_changed = false;

	// Process clicking on the drag
	if (g.ActiveId == id)
	{
		if (g.IO.MouseDown[0])
		{
			if (g.ActiveIdIsJustActivated)
			{
				// Lock current value on click
				g.DragCurrentValue = *v;
				g.DragLastMouseDelta = ImVec2(0.f, 0.f);
			}

			if (v_speed == 0.0f && (v_max - v_min) != 0.0f && (v_max - v_min) < FLT_MAX)
				v_speed = (v_max - v_min) * g.DragSpeedDefaultRatio;

			float v_cur = g.DragCurrentValue;
			const ImVec2 mouse_drag_delta = GetMouseDragDelta(0, 1.0f);
			float adjust_delta = 0.0f;
			if (IsMousePosValid())
			{
				//if (g.ActiveIdSource == ImGuiInputSource_Mouse)
				{
					adjust_delta = mouse_drag_delta.x - g.DragLastMouseDelta.x;
					if (g.IO.KeyShift && g.DragSpeedScaleFast >= 0.0f)
						adjust_delta *= g.DragSpeedScaleFast;
					if (g.IO.KeyAlt && g.DragSpeedScaleSlow >= 0.0f)
						adjust_delta *= g.DragSpeedScaleSlow;
				}
				g.DragLastMouseDelta.x = mouse_drag_delta.x;
			}
			adjust_delta *= v_speed;

			if (fabsf(adjust_delta) > 0.0f)
			{
				if (fabsf(power - 1.0f) > 0.001f)
				{
					// Logarithmic curve on both side of 0.0
					float v0_abs = v_cur >= 0.0f ? v_cur : -v_cur;
					float v0_sign = v_cur >= 0.0f ? 1.0f : -1.0f;
					float v1 = powf(v0_abs, 1.0f / power) + (adjust_delta * v0_sign);
					float v1_abs = v1 >= 0.0f ? v1 : -v1;
					float v1_sign = v1 >= 0.0f ? 1.0f : -1.0f;          // Crossed sign line
					v_cur = powf(v1_abs, power) * v0_sign * v1_sign;    // Reapply sign
				}
				else
				{
					v_cur += adjust_delta;
				}

				// Clamp
				if (v_min < v_max)
					v_cur = ImClamp(v_cur, v_min, v_max);
				g.DragCurrentValue = v_cur;
			}

			// Round to user desired precision, then apply
			v_cur = RoundScalar(v_cur, decimal_precision);
			if (*v != v_cur)
			{
				*v = v_cur;
				value_changed = true;
			}
		}
		else
		{
			ClearActiveID();
		}
	}

	return value_changed;
}

bool ImGui::DragFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* display_format, float power)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const float w = CalcItemWidth();

	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y*2.0f));
	const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
	const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

	// NB- we don't call ItemSize() yet because we may turn into a text edit box below
	if (!ItemAdd(total_bb, id))
	{
		ItemSize(total_bb, style.FramePadding.y);
		return false;
	}
	const bool hovered = ItemHoverable(frame_bb, id);

	if (!display_format)
		display_format = "%.3f";
	int decimal_precision = ParseFormatPrecision(display_format, 3);

	// Tabbing or CTRL-clicking on Drag turns it into an input box
	bool start_text_input = false;
	const bool tab_focus_requested = FocusableItemRegister(window, id);
	if (tab_focus_requested || (hovered && (g.IO.MouseClicked[0] || g.IO.MouseDoubleClicked[0])))
	{
		SetActiveID(id, window);
		FocusWindow(window);
		if (tab_focus_requested || g.IO.KeyCtrl || g.IO.MouseDoubleClicked[0])
		{
			start_text_input = true;
			g.ScalarAsInputTextId = 0;
		}
	}
	if (start_text_input || (g.ActiveId == id && g.ScalarAsInputTextId == id))
		return InputScalarAsWidgetReplacement(frame_bb, label, ImGuiDataType_Float, v, id, decimal_precision);

	// Actual drag behavior
	ItemSize(total_bb, style.FramePadding.y);
	const bool value_changed = DragBehavior(frame_bb, id, v, v_speed, v_min, v_max, decimal_precision, power);

	// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
	char value_buf[64];
	const char* value_buf_end = value_buf + ImFormatString(value_buf, IM_ARRAYSIZE(value_buf), display_format, *v);
	RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));

	if (label_size.x > 0.0f)
		RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);

	return value_changed;
}

bool ImGui::DragFloatN(const char* label, float* v, int components, float v_speed, float v_min, float v_max, const char* display_format, float power)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	bool value_changed = false;
	BeginGroup();
	PushID(label);
	PushMultiItemsWidths(components);
	for (int i = 0; i < components; i++)
	{
		PushID(i);
		value_changed |= DragFloat("##v", &v[i], v_speed, v_min, v_max, display_format, power);
		SameLine(0, g.Style.ItemInnerSpacing.x);
		PopID();
		PopItemWidth();
	}
	PopID();

	TextUnformatted(label, FindRenderedTextEnd(label));
	EndGroup();

	return value_changed;
}

bool ImGui::DragFloat2(const char* label, float v[2], float v_speed, float v_min, float v_max, const char* display_format, float power)
{
	return DragFloatN(label, v, 2, v_speed, v_min, v_max, display_format, power);
}

bool ImGui::DragFloat3(const char* label, float v[3], float v_speed, float v_min, float v_max, const char* display_format, float power)
{
	return DragFloatN(label, v, 3, v_speed, v_min, v_max, display_format, power);
}

bool ImGui::DragFloat4(const char* label, float v[4], float v_speed, float v_min, float v_max, const char* display_format, float power)
{
	return DragFloatN(label, v, 4, v_speed, v_min, v_max, display_format, power);
}

bool ImGui::DragFloatRange2(const char* label, float* v_current_min, float* v_current_max, float v_speed, float v_min, float v_max, const char* display_format, const char* display_format_max, float power)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	PushID(label);
	BeginGroup();
	PushMultiItemsWidths(2);

	bool value_changed = DragFloat("##min", v_current_min, v_speed, (v_min >= v_max) ? -FLT_MAX : v_min, (v_min >= v_max) ? *v_current_max : ImMin(v_max, *v_current_max), display_format, power);
	PopItemWidth();
	SameLine(0, g.Style.ItemInnerSpacing.x);
	value_changed |= DragFloat("##max", v_current_max, v_speed, (v_min >= v_max) ? *v_current_min : ImMax(v_min, *v_current_min), (v_min >= v_max) ? FLT_MAX : v_max, display_format_max ? display_format_max : display_format, power);
	PopItemWidth();
	SameLine(0, g.Style.ItemInnerSpacing.x);

	TextUnformatted(label, FindRenderedTextEnd(label));
	EndGroup();
	PopID();

	return value_changed;
}

// NB: v_speed is float to allow adjusting the drag speed with more precision
bool ImGui::DragInt(const char* label, int* v, float v_speed, int v_min, int v_max, const char* display_format)
{
	if (!display_format)
		display_format = "%.0f";
	float v_f = (float)*v;
	bool value_changed = DragFloat(label, &v_f, v_speed, (float)v_min, (float)v_max, display_format);
	*v = (int)v_f;
	return value_changed;
}

bool ImGui::DragIntN(const char* label, int* v, int components, float v_speed, int v_min, int v_max, const char* display_format)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	bool value_changed = false;
	BeginGroup();
	PushID(label);
	PushMultiItemsWidths(components);
	for (int i = 0; i < components; i++)
	{
		PushID(i);
		value_changed |= DragInt("##v", &v[i], v_speed, v_min, v_max, display_format);
		SameLine(0, g.Style.ItemInnerSpacing.x);
		PopID();
		PopItemWidth();
	}
	PopID();

	TextUnformatted(label, FindRenderedTextEnd(label));
	EndGroup();

	return value_changed;
}

bool ImGui::DragInt2(const char* label, int v[2], float v_speed, int v_min, int v_max, const char* display_format)
{
	return DragIntN(label, v, 2, v_speed, v_min, v_max, display_format);
}

bool ImGui::DragInt3(const char* label, int v[3], float v_speed, int v_min, int v_max, const char* display_format)
{
	return DragIntN(label, v, 3, v_speed, v_min, v_max, display_format);
}

bool ImGui::DragInt4(const char* label, int v[4], float v_speed, int v_min, int v_max, const char* display_format)
{
	return DragIntN(label, v, 4, v_speed, v_min, v_max, display_format);
}

bool ImGui::DragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed, int v_min, int v_max, const char* display_format, const char* display_format_max)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	PushID(label);
	BeginGroup();
	PushMultiItemsWidths(2);

	bool value_changed = DragInt("##min", v_current_min, v_speed, (v_min >= v_max) ? INT_MIN : v_min, (v_min >= v_max) ? *v_current_max : ImMin(v_max, *v_current_max), display_format);
	PopItemWidth();
	SameLine(0, g.Style.ItemInnerSpacing.x);
	value_changed |= DragInt("##max", v_current_max, v_speed, (v_min >= v_max) ? *v_current_min : ImMax(v_min, *v_current_min), (v_min >= v_max) ? INT_MAX : v_max, display_format_max ? display_format_max : display_format);
	PopItemWidth();
	SameLine(0, g.Style.ItemInnerSpacing.x);

	TextUnformatted(label, FindRenderedTextEnd(label));
	EndGroup();
	PopID();

	return value_changed;
}

void ImGui::PlotEx(ImGuiPlotType plot_type, const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	if (graph_size.x == 0.0f)
		graph_size.x = CalcItemWidth();
	if (graph_size.y == 0.0f)
		graph_size.y = label_size.y + (style.FramePadding.y * 2);

	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(graph_size.x, graph_size.y));
	const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
	const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, 0))
		return;
	const bool hovered = ItemHoverable(inner_bb, 0);

	// Determine scale from values if not specified
	if (scale_min == FLT_MAX || scale_max == FLT_MAX)
	{
		float v_min = FLT_MAX;
		float v_max = -FLT_MAX;
		for (int i = 0; i < values_count; i++)
		{
			const float v = values_getter(data, i);
			v_min = ImMin(v_min, v);
			v_max = ImMax(v_max, v);
		}
		if (scale_min == FLT_MAX)
			scale_min = v_min;
		if (scale_max == FLT_MAX)
			scale_max = v_max;
	}

	RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

	if (values_count > 0)
	{
		int res_w = ImMin((int)graph_size.x, values_count) + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);
		int item_count = values_count + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);

		// Tooltip on hover
		int v_hovered = -1;
		if (hovered)
		{
			const float t = ImClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
			const int v_idx = (int)(t * item_count);
			IM_ASSERT(v_idx >= 0 && v_idx < values_count);

			const float v0 = values_getter(data, (v_idx + values_offset) % values_count);
			const float v1 = values_getter(data, (v_idx + 1 + values_offset) % values_count);
			if (plot_type == ImGuiPlotType_Lines)
				SetTooltip("%d: %8.4g\n%d: %8.4g", v_idx, v0, v_idx + 1, v1);
			else if (plot_type == ImGuiPlotType_Histogram)
				SetTooltip("%d: %8.4g", v_idx, v0);
			v_hovered = v_idx;
		}

		const float t_step = 1.0f / (float)res_w;

		float v0 = values_getter(data, (0 + values_offset) % values_count);
		float t0 = 0.0f;
		ImVec2 tp0 = ImVec2(t0, 1.0f - ImSaturate((v0 - scale_min) / (scale_max - scale_min)));                       // Point in the normalized space of our target rectangle
		float histogram_zero_line_t = (scale_min * scale_max < 0.0f) ? (-scale_min / (scale_max - scale_min)) : (scale_min < 0.0f ? 0.0f : 1.0f);   // Where does the zero line stands

		const ImU32 col_base = GetColorU32((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLines : ImGuiCol_PlotHistogram);
		const ImU32 col_hovered = GetColorU32((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLinesHovered : ImGuiCol_PlotHistogramHovered);

		for (int n = 0; n < res_w; n++)
		{
			const float t1 = t0 + t_step;
			const int v1_idx = (int)(t0 * item_count + 0.5f);
			IM_ASSERT(v1_idx >= 0 && v1_idx < values_count);
			const float v1 = values_getter(data, (v1_idx + values_offset + 1) % values_count);
			const ImVec2 tp1 = ImVec2(t1, 1.0f - ImSaturate((v1 - scale_min) / (scale_max - scale_min)));

			// NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
			ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, tp0);
			ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max, (plot_type == ImGuiPlotType_Lines) ? tp1 : ImVec2(tp1.x, histogram_zero_line_t));
			if (plot_type == ImGuiPlotType_Lines)
			{
				window->DrawList->AddLine(pos0, pos1, v_hovered == v1_idx ? col_hovered : col_base);
			}
			else if (plot_type == ImGuiPlotType_Histogram)
			{
				if (pos1.x >= pos0.x + 2.0f)
					pos1.x -= 1.0f;
				window->DrawList->AddRectFilled(pos0, pos1, v_hovered == v1_idx ? col_hovered : col_base);
			}

			t0 = t1;
			tp0 = tp1;
		}
	}

	// Text overlay
	if (overlay_text)
		RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, overlay_text, NULL, NULL, ImVec2(0.5f, 0.0f));

	if (label_size.x > 0.0f)
		RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);
}

struct ImGuiPlotArrayGetterData
{
	const float* Values;
	int Stride;

	ImGuiPlotArrayGetterData(const float* values, int stride) { Values = values; Stride = stride; }
};

static float Plot_ArrayGetter(void* data, int idx)
{
	ImGuiPlotArrayGetterData* plot_data = (ImGuiPlotArrayGetterData*)data;
	const float v = *(float*)(void*)((unsigned char*)plot_data->Values + (size_t)idx * plot_data->Stride);
	return v;
}

void ImGui::PlotLines(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride)
{
	ImGuiPlotArrayGetterData data(values, stride);
	PlotEx(ImGuiPlotType_Lines, label, &Plot_ArrayGetter, (void*)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void ImGui::PlotLines(const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size)
{
	PlotEx(ImGuiPlotType_Lines, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void ImGui::PlotHistogram(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride)
{
	ImGuiPlotArrayGetterData data(values, stride);
	PlotEx(ImGuiPlotType_Histogram, label, &Plot_ArrayGetter, (void*)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void ImGui::PlotHistogram(const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size)
{
	PlotEx(ImGuiPlotType_Histogram, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

// size_arg (for each axis) < 0.0f: align to end, 0.0f: auto, > 0.0f: specified size
void ImGui::ProgressBar(float fraction, const ImVec2& size_arg, const char* overlay)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	ImVec2 pos = window->DC.CursorPos;
	ImRect bb(pos, pos + CalcItemSize(size_arg, CalcItemWidth(), g.FontSize + style.FramePadding.y*2.0f));
	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, 0))
		return;

	// Render
	fraction = ImSaturate(fraction);
	RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
	bb.Expand(ImVec2(-style.FrameBorderSize, -style.FrameBorderSize));
	const ImVec2 fill_br = ImVec2(ImLerp(bb.Min.x, bb.Max.x, fraction), bb.Max.y);
	RenderRectFilledRangeH(window->DrawList, bb, GetColorU32(ImGuiCol_PlotHistogram), 0.0f, fraction, style.FrameRounding);

	// Default displaying the fraction as percentage string, but user can override it
	char overlay_buf[32];
	if (!overlay)
	{
		ImFormatString(overlay_buf, IM_ARRAYSIZE(overlay_buf), "%.0f%%", fraction * 100 + 0.01f);
		overlay = overlay_buf;
	}

	ImVec2 overlay_size = CalcTextSize(overlay, NULL);
	if (overlay_size.x > 0.0f)
		RenderTextClipped(ImVec2(ImClamp(fill_br.x + style.ItemSpacing.x, bb.Min.x, bb.Max.x - overlay_size.x - style.ItemInnerSpacing.x), bb.Min.y), bb.Max, overlay, NULL, &overlay_size, ImVec2(0.0f, 0.5f), &bb);
}

bool ImGui::Checkbox(const char* label, bool* v)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(label_size.y + style.FramePadding.y * 2, label_size.y + style.FramePadding.y * 2)); // We want a square shape to we use Y twice
	ItemSize(check_bb, style.FramePadding.y);

	ImRect total_bb = check_bb;
	if (label_size.x > 0)
		SameLine(0, style.ItemInnerSpacing.x);
	const ImRect text_bb(window->DC.CursorPos + ImVec2(0, style.FramePadding.y), window->DC.CursorPos + ImVec2(0, style.FramePadding.y) + label_size);
	if (label_size.x > 0)
	{
		ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
		total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
	}

	if (!ItemAdd(total_bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
	if (pressed)
		*v = !(*v);

	RenderFrame(check_bb.Min, check_bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBgb1g), true, style.FrameRounding);
	if (*v)
	{
		const float check_sz = ImMin(check_bb.GetWidth(), check_bb.GetHeight());
		const float pad = ImMax(1.0f, (float)(int)(check_sz / 6.0f));
		window->DrawList->AddRectFilled(check_bb.Min + ImVec2(pad, pad), check_bb.Max - ImVec2(pad, pad), GetColorU32(ImGuiCol_CheckMark), style.FrameRounding);
		//RenderCheckMark(check_bb.Min + ImVec2(pad, pad), GetColorU32(ImGuiCol_CheckMark), check_bb.GetWidth() - pad*2.0f);
	}

	if (g.LogEnabled)
		LogRenderedText(&text_bb.Min, *v ? "[x]" : "[ ]");
	if (label_size.x > 0.0f)
		RenderText(text_bb.Min, label);

	return pressed;
}

bool ImGui::CheckboxFlags(const char* label, unsigned int* flags, unsigned int flags_value)
{
	bool v = ((*flags & flags_value) == flags_value);
	bool pressed = Checkbox(label, &v);
	if (pressed)
	{
		if (v)
			*flags |= flags_value;
		else
			*flags &= ~flags_value;
	}

	return pressed;
}

bool ImGui::RadioButton(const char* label, bool active)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(label_size.y + style.FramePadding.y * 2 - 1, label_size.y + style.FramePadding.y * 2 - 1));
	ItemSize(check_bb, style.FramePadding.y);

	ImRect total_bb = check_bb;
	if (label_size.x > 0)
		SameLine(0, style.ItemInnerSpacing.x);
	const ImRect text_bb(window->DC.CursorPos + ImVec2(0, style.FramePadding.y), window->DC.CursorPos + ImVec2(0, style.FramePadding.y) + label_size);
	if (label_size.x > 0)
	{
		ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
		total_bb.Add(text_bb);
	}

	if (!ItemAdd(total_bb, id))
		return false;

	ImVec2 center = check_bb.GetCenter();
	center.x = (float)(int)center.x + 0.5f;
	center.y = (float)(int)center.y + 0.5f;
	const float radius = check_bb.GetHeight() * 0.5f;

	bool hovered, held;
	bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);

	window->DrawList->AddCircleFilled(center, radius, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBgb1g), 16);
	if (active)
	{
		const float check_sz = ImMin(check_bb.GetWidth(), check_bb.GetHeight());
		const float pad = ImMax(1.0f, (float)(int)(check_sz / 6.0f));
		window->DrawList->AddCircleFilled(center, radius - pad, GetColorU32(ImGuiCol_CheckMark), 16);
	}

	if (style.FrameBorderSize > 0.0f)
	{
		window->DrawList->AddCircle(center + ImVec2(1, 1), radius, GetColorU32(ImGuiCol_BorderShadow), 16, style.FrameBorderSize);
		window->DrawList->AddCircle(center, radius, GetColorU32(ImGuiCol_Border), 16, style.FrameBorderSize);
	}

	if (g.LogEnabled)
		LogRenderedText(&text_bb.Min, active ? "(x)" : "( )");
	if (label_size.x > 0.0f)
		RenderText(text_bb.Min, label);

	return pressed;
}

bool ImGui::RadioButton(const char* label, int* v, int v_button)
{
	const bool pressed = RadioButton(label, *v == v_button);
	if (pressed)
	{
		*v = v_button;
	}
	return pressed;
}

static int InputTextCalcTextLenAndLineCount(const char* text_begin, const char** out_text_end)
{
	int line_count = 0;
	const char* s = text_begin;
	while (char c = *s++) // We are only matching for \n so we can ignore UTF-8 decoding
		if (c == '\n')
			line_count++;
	s--;
	if (s[0] != '\n' && s[0] != '\r')
		line_count++;
	*out_text_end = s;
	return line_count;
}

static ImVec2 InputTextCalcTextSizeW(const ImWchar* text_begin, const ImWchar* text_end, const ImWchar** remaining, ImVec2* out_offset, bool stop_on_new_line)
{
	ImFont* font = GImGui->Font;
	const float line_height = GImGui->FontSize;
	const float scale = line_height / font->FontSize;

	ImVec2 text_size = ImVec2(0, 0);
	float line_width = 0.0f;

	const ImWchar* s = text_begin;
	while (s < text_end)
	{
		unsigned int c = (unsigned int)(*s++);
		if (c == '\n')
		{
			text_size.x = ImMax(text_size.x, line_width);
			text_size.y += line_height;
			line_width = 0.0f;
			if (stop_on_new_line)
				break;
			continue;
		}
		if (c == '\r')
			continue;

		const float char_width = font->GetCharAdvance((unsigned short)c) * scale;
		line_width += char_width;
	}

	if (text_size.x < line_width)
		text_size.x = line_width;

	if (out_offset)
		*out_offset = ImVec2(line_width, text_size.y + line_height);  // offset allow for the possibility of sitting after a trailing \n

	if (line_width > 0 || text_size.y == 0.0f)                        // whereas size.y will ignore the trailing \n
		text_size.y += line_height;

	if (remaining)
		*remaining = s;

	return text_size;
}

// Wrapper for stb_textedit.h to edit text (our wrapper is for: statically sized buffer, single-line, wchar characters. InputText converts between UTF-8 and wchar)
namespace ImGuiStb
{

	static int     STB_TEXTEDIT_STRINGLEN(const STB_TEXTEDIT_STRING* obj) { return obj->CurLenW; }
	static ImWchar STB_TEXTEDIT_GETCHAR(const STB_TEXTEDIT_STRING* obj, int idx) { return obj->Text[idx]; }
	static float   STB_TEXTEDIT_GETWIDTH(STB_TEXTEDIT_STRING* obj, int line_start_idx, int char_idx) { ImWchar c = obj->Text[line_start_idx + char_idx]; if (c == '\n') return STB_TEXTEDIT_GETWIDTH_NEWLINE; return GImGui->Font->GetCharAdvance(c) * (GImGui->FontSize / GImGui->Font->FontSize); }
	static int     STB_TEXTEDIT_KEYTOTEXT(int key) { return key >= 0x10000 ? 0 : key; }
	static ImWchar STB_TEXTEDIT_NEWLINE = '\n';
	static void    STB_TEXTEDIT_LAYOUTROW(StbTexteditRow* r, STB_TEXTEDIT_STRING* obj, int line_start_idx)
	{
		const ImWchar* text = obj->Text.Data;
		const ImWchar* text_remaining = NULL;
		const ImVec2 size = InputTextCalcTextSizeW(text + line_start_idx, text + obj->CurLenW, &text_remaining, NULL, true);
		r->x0 = 0.0f;
		r->x1 = size.x;
		r->baseline_y_delta = size.y;
		r->ymin = 0.0f;
		r->ymax = size.y;
		r->num_chars = (int)(text_remaining - (text + line_start_idx));
	}

	static bool is_separator(unsigned int c) { return ImCharIsSpace(c) || c == ',' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == '|'; }
	static int  is_word_boundary_from_right(STB_TEXTEDIT_STRING* obj, int idx) { return idx > 0 ? (is_separator(obj->Text[idx - 1]) && !is_separator(obj->Text[idx])) : 1; }
	static int  STB_TEXTEDIT_MOVEWORDLEFT_IMPL(STB_TEXTEDIT_STRING* obj, int idx) { idx--; while (idx >= 0 && !is_word_boundary_from_right(obj, idx)) idx--; return idx < 0 ? 0 : idx; }
#ifdef __APPLE__    // FIXME: Move setting to IO structure
	static int  is_word_boundary_from_left(STB_TEXTEDIT_STRING* obj, int idx) { return idx > 0 ? (!is_separator(obj->Text[idx - 1]) && is_separator(obj->Text[idx])) : 1; }
	static int  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL(STB_TEXTEDIT_STRING* obj, int idx) { idx++; int len = obj->CurLenW; while (idx < len && !is_word_boundary_from_left(obj, idx)) idx++; return idx > len ? len : idx; }
#else
	static int  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL(STB_TEXTEDIT_STRING* obj, int idx) { idx++; int len = obj->CurLenW; while (idx < len && !is_word_boundary_from_right(obj, idx)) idx++; return idx > len ? len : idx; }
#endif
#define STB_TEXTEDIT_MOVEWORDLEFT   STB_TEXTEDIT_MOVEWORDLEFT_IMPL    // They need to be #define for stb_textedit.h
#define STB_TEXTEDIT_MOVEWORDRIGHT  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL

	static void STB_TEXTEDIT_DELETECHARS(STB_TEXTEDIT_STRING* obj, int pos, int n)
	{
		ImWchar* dst = obj->Text.Data + pos;

		// We maintain our buffer length in both UTF-8 and wchar formats
		obj->CurLenA -= ImTextCountUtf8BytesFromStr(dst, dst + n);
		obj->CurLenW -= n;

		// Offset remaining text
		const ImWchar* src = obj->Text.Data + pos + n;
		while (ImWchar c = *src++)
			*dst++ = c;
		*dst = '\0';
	}

	static bool STB_TEXTEDIT_INSERTCHARS(STB_TEXTEDIT_STRING* obj, int pos, const ImWchar* new_text, int new_text_len)
	{
		const int text_len = obj->CurLenW;
		IM_ASSERT(pos <= text_len);
		if (new_text_len + text_len + 1 > obj->Text.Size)
			return false;

		const int new_text_len_utf8 = ImTextCountUtf8BytesFromStr(new_text, new_text + new_text_len);
		if (new_text_len_utf8 + obj->CurLenA + 1 > obj->BufSizeA)
			return false;

		ImWchar* text = obj->Text.Data;
		if (pos != text_len)
			memmove(text + pos + new_text_len, text + pos, (size_t)(text_len - pos) * sizeof(ImWchar));
		memcpy(text + pos, new_text, (size_t)new_text_len * sizeof(ImWchar));

		obj->CurLenW += new_text_len;
		obj->CurLenA += new_text_len_utf8;
		obj->Text[obj->CurLenW] = '\0';

		return true;
	}

	// We don't use an enum so we can build even with conflicting symbols (if another user of stb_textedit.h leak their STB_TEXTEDIT_K_* symbols)
#define STB_TEXTEDIT_K_LEFT         0x10000 // keyboard input to move cursor left
#define STB_TEXTEDIT_K_RIGHT        0x10001 // keyboard input to move cursor right
#define STB_TEXTEDIT_K_UP           0x10002 // keyboard input to move cursor up
#define STB_TEXTEDIT_K_DOWN         0x10003 // keyboard input to move cursor down
#define STB_TEXTEDIT_K_LINESTART    0x10004 // keyboard input to move cursor to start of line
#define STB_TEXTEDIT_K_LINEEND      0x10005 // keyboard input to move cursor to end of line
#define STB_TEXTEDIT_K_TEXTSTART    0x10006 // keyboard input to move cursor to start of text
#define STB_TEXTEDIT_K_TEXTEND      0x10007 // keyboard input to move cursor to end of text
#define STB_TEXTEDIT_K_DELETE       0x10008 // keyboard input to delete selection or character under cursor
#define STB_TEXTEDIT_K_BACKSPACE    0x10009 // keyboard input to delete selection or character left of cursor
#define STB_TEXTEDIT_K_UNDO         0x1000A // keyboard input to perform undo
#define STB_TEXTEDIT_K_REDO         0x1000B // keyboard input to perform redo
#define STB_TEXTEDIT_K_WORDLEFT     0x1000C // keyboard input to move cursor left one word
#define STB_TEXTEDIT_K_WORDRIGHT    0x1000D // keyboard input to move cursor right one word
#define STB_TEXTEDIT_K_SHIFT        0x20000

#define STB_TEXTEDIT_IMPLEMENTATION
#include "stb_textedit.h"

}

void ImGuiTextEditState::OnKeyPressed(int key)
{
	stb_textedit_key(this, &StbState, key);
	CursorFollow = true;
	CursorAnimReset();
}

// Public API to manipulate UTF-8 text
// We expose UTF-8 to the user (unlike the STB_TEXTEDIT_* functions which are manipulating wchar)
// FIXME: The existence of this rarely exercised code path is a bit of a nuisance.
void ImGuiTextEditCallbackData::DeleteChars(int pos, int bytes_count)
{
	IM_ASSERT(pos + bytes_count <= BufTextLen);
	char* dst = Buf + pos;
	const char* src = Buf + pos + bytes_count;
	while (char c = *src++)
		*dst++ = c;
	*dst = '\0';

	if (CursorPos + bytes_count >= pos)
		CursorPos -= bytes_count;
	else if (CursorPos >= pos)
		CursorPos = pos;
	SelectionStart = SelectionEnd = CursorPos;
	BufDirty = true;
	BufTextLen -= bytes_count;
}

void ImGuiTextEditCallbackData::InsertChars(int pos, const char* new_text, const char* new_text_end)
{
	const int new_text_len = new_text_end ? (int)(new_text_end - new_text) : (int)strlen(new_text);
	if (new_text_len + BufTextLen + 1 >= BufSize)
		return;

	if (BufTextLen != pos)
		memmove(Buf + pos + new_text_len, Buf + pos, (size_t)(BufTextLen - pos));
	memcpy(Buf + pos, new_text, (size_t)new_text_len * sizeof(char));
	Buf[BufTextLen + new_text_len] = '\0';

	if (CursorPos >= pos)
		CursorPos += new_text_len;
	SelectionStart = SelectionEnd = CursorPos;
	BufDirty = true;
	BufTextLen += new_text_len;
}

// Return false to discard a character.
static bool InputTextFilterCharacter(unsigned int* p_char, ImGuiInputTextFlags flags, ImGuiTextEditCallback callback, void* user_data)
{
	unsigned int c = *p_char;

	if (c < 128 && c != ' ' && !isprint((int)(c & 0xFF)))
	{
		bool pass = false;
		pass |= (c == '\n' && (flags & ImGuiInputTextFlags_Multiline));
		pass |= (c == '\t' && (flags & ImGuiInputTextFlags_AllowTabInput));
		if (!pass)
			return false;
	}

	if (c >= 0xE000 && c <= 0xF8FF) // Filter private Unicode range. I don't imagine anybody would want to input them. GLFW on OSX seems to send private characters for special keys like arrow keys.
		return false;

	if (flags & (ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank))
	{
		if (flags & ImGuiInputTextFlags_CharsDecimal)
			if (!(c >= '0' && c <= '9') && (c != '.') && (c != '-') && (c != '+') && (c != '*') && (c != '/'))
				return false;

		if (flags & ImGuiInputTextFlags_CharsHexadecimal)
			if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F'))
				return false;

		if (flags & ImGuiInputTextFlags_CharsUppercase)
			if (c >= 'a' && c <= 'z')
				*p_char = (c += (unsigned int)('A' - 'a'));

		if (flags & ImGuiInputTextFlags_CharsNoBlank)
			if (ImCharIsSpace(c))
				return false;
	}

	if (flags & ImGuiInputTextFlags_CallbackCharFilter)
	{
		ImGuiTextEditCallbackData callback_data;
		memset(&callback_data, 0, sizeof(ImGuiTextEditCallbackData));
		callback_data.EventFlag = ImGuiInputTextFlags_CallbackCharFilter;
		callback_data.EventChar = (ImWchar)c;
		callback_data.Flags = flags;
		callback_data.UserData = user_data;
		if (callback(&callback_data) != 0)
			return false;
		*p_char = callback_data.EventChar;
		if (!callback_data.EventChar)
			return false;
	}

	return true;
}

// Edit a string of text
// NB: when active, hold on a privately held copy of the text (and apply back to 'buf'). So changing 'buf' while active has no effect.
// FIXME: Rather messy function partly because we are doing UTF8 > u16 > UTF8 conversions on the go to more easily handle stb_textedit calls. Ideally we should stay in UTF-8 all the time. See https://github.com/nothings/stb/issues/188
bool ImGui::InputTextEx(const char* label, char* buf, int buf_size, const ImVec2& size_arg, ImGuiInputTextFlags flags, ImGuiTextEditCallback callback, void* user_data)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	IM_ASSERT(!((flags & ImGuiInputTextFlags_CallbackHistory) && (flags & ImGuiInputTextFlags_Multiline))); // Can't use both together (they both use up/down keys)
	IM_ASSERT(!((flags & ImGuiInputTextFlags_CallbackCompletion) && (flags & ImGuiInputTextFlags_AllowTabInput))); // Can't use both together (they both use tab key)

	ImGuiContext& g = *GImGui;
	const ImGuiIO& io = g.IO;
	const ImGuiStyle& style = g.Style;

	const bool is_multiline = (flags & ImGuiInputTextFlags_Multiline) != 0;
	const bool is_editable = (flags & ImGuiInputTextFlags_ReadOnly) == 0;
	const bool is_password = (flags & ImGuiInputTextFlags_Password) != 0;
	const bool is_undoable = (flags & ImGuiInputTextFlags_NoUndoRedo) == 0;

	if (is_multiline) // Open group before calling GetID() because groups tracks id created during their spawn
		BeginGroup();
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	ImVec2 size = CalcItemSize(size_arg, CalcItemWidth(), (is_multiline ? GetTextLineHeight() * 8.0f : label_size.y) + style.FramePadding.y*2.0f); // Arbitrary default of 8 lines high for multi-line
	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
	const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? (style.ItemInnerSpacing.x + label_size.x) : 0.0f, 0.0f));

	ImGuiWindow* draw_window = window;
	if (is_multiline)
	{
		if (!BeginChildFrame(id, frame_bb.GetSize()))
		{
			EndChildFrame();
			EndGroup();
			return false;
		}
		draw_window = GetCurrentWindow();
		size.x -= draw_window->ScrollbarSizes.x;
	}
	else
	{
		ItemSize(total_bb, style.FramePadding.y);
		if (!ItemAdd(total_bb, id))
			return false;
	}
	const bool hovered = ItemHoverable(frame_bb, id);
	if (hovered)
		g.MouseCursor = ImGuiMouseCursor_TextInput;

	// Password pushes a temporary font with only a fallback glyph
	if (is_password)
	{
		const ImFontGlyph* glyph = g.Font->FindGlyph('*');
		ImFont* password_font = &g.InputTextPasswordFont;
		password_font->FontSize = g.Font->FontSize;
		password_font->Scale = g.Font->Scale;
		password_font->DisplayOffset = g.Font->DisplayOffset;
		password_font->Ascent = g.Font->Ascent;
		password_font->Descent = g.Font->Descent;
		password_font->ContainerAtlas = g.Font->ContainerAtlas;
		password_font->FallbackGlyph = glyph;
		password_font->FallbackAdvanceX = glyph->AdvanceX;
		IM_ASSERT(password_font->Glyphs.empty() && password_font->IndexAdvanceX.empty() && password_font->IndexLookup.empty());
		PushFont(password_font);
	}

	// NB: we are only allowed to access 'edit_state' if we are the active widget.
	ImGuiTextEditState& edit_state = g.InputTextState;

	const bool focus_requested = FocusableItemRegister(window, id, (flags & (ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_AllowTabInput)) == 0);    // Using completion callback disable keyboard tabbing
	const bool focus_requested_by_code = focus_requested && (window->FocusIdxAllCounter == window->FocusIdxAllRequestCurrent);
	const bool focus_requested_by_tab = focus_requested && !focus_requested_by_code;

	const bool user_clicked = hovered && io.MouseClicked[0];
	const bool user_scrolled = is_multiline && g.ActiveId == 0 && edit_state.Id == id && g.ActiveIdPreviousFrame == draw_window->GetIDNoKeepAlive("#SCROLLY");

	bool clear_active_id = false;

	bool select_all = (g.ActiveId != id) && (flags & ImGuiInputTextFlags_AutoSelectAll) != 0;
	if (focus_requested || user_clicked || user_scrolled)
	{
		if (g.ActiveId != id)
		{
			// Start edition
			// Take a copy of the initial buffer value (both in original UTF-8 format and converted to wchar)
			// From the moment we focused we are ignoring the content of 'buf' (unless we are in read-only mode)
			const int prev_len_w = edit_state.CurLenW;
			edit_state.Text.resize(buf_size + 1);        // wchar count <= UTF-8 count. we use +1 to make sure that .Data isn't NULL so it doesn't crash.
			edit_state.InitialText.resize(buf_size + 1); // UTF-8. we use +1 to make sure that .Data isn't NULL so it doesn't crash.
			ImStrncpy(edit_state.InitialText.Data, buf, edit_state.InitialText.Size);
			const char* buf_end = NULL;
			edit_state.CurLenW = ImTextStrFromUtf8(edit_state.Text.Data, edit_state.Text.Size, buf, NULL, &buf_end);
			edit_state.CurLenA = (int)(buf_end - buf); // We can't get the result from ImFormatString() above because it is not UTF-8 aware. Here we'll cut off malformed UTF-8.
			edit_state.CursorAnimReset();

			// Preserve cursor position and undo/redo stack if we come back to same widget
			// FIXME: We should probably compare the whole buffer to be on the safety side. Comparing buf (utf8) and edit_state.Text (wchar).
			const bool recycle_state = (edit_state.Id == id) && (prev_len_w == edit_state.CurLenW);
			if (recycle_state)
			{
				// Recycle existing cursor/selection/undo stack but clamp position
				// Note a single mouse click will override the cursor/position immediately by calling stb_textedit_click handler.
				edit_state.CursorClamp();
			}
			else
			{
				edit_state.Id = id;
				edit_state.ScrollX = 0.0f;
				stb_textedit_initialize_state(&edit_state.StbState, !is_multiline);
				if (!is_multiline && focus_requested_by_code)
					select_all = true;
			}
			if (flags & ImGuiInputTextFlags_AlwaysInsertMode)
				edit_state.StbState.insert_mode = true;
			if (!is_multiline && (focus_requested_by_tab || (user_clicked && io.KeyCtrl)))
				select_all = true;
		}
		SetActiveID(id, window);
		FocusWindow(window);
	}
	else if (io.MouseClicked[0])
	{
		// Release focus when we click outside
		clear_active_id = true;
	}

	bool value_changed = false;
	bool enter_pressed = false;

	if (g.ActiveId == id)
	{
		if (!is_editable && !g.ActiveIdIsJustActivated)
		{
			// When read-only we always use the live data passed to the function
			edit_state.Text.resize(buf_size + 1);
			const char* buf_end = NULL;
			edit_state.CurLenW = ImTextStrFromUtf8(edit_state.Text.Data, edit_state.Text.Size, buf, NULL, &buf_end);
			edit_state.CurLenA = (int)(buf_end - buf);
			edit_state.CursorClamp();
		}

		edit_state.BufSizeA = buf_size;

		// Although we are active we don't prevent mouse from hovering other elements unless we are interacting right now with the widget.
		// Down the line we should have a cleaner library-wide concept of Selected vs Active.
		g.ActiveIdAllowOverlap = !io.MouseDown[0];
		g.WantTextInputNextFrame = 1;

		// Edit in progress
		const float mouse_x = (io.MousePos.x - frame_bb.Min.x - style.FramePadding.x) + edit_state.ScrollX;
		const float mouse_y = (is_multiline ? (io.MousePos.y - draw_window->DC.CursorPos.y - style.FramePadding.y) : (g.FontSize*0.5f));

		const bool osx_double_click_selects_words = io.OptMacOSXBehaviors;      // OS X style: Double click selects by word instead of selecting whole text
		if (select_all || (hovered && !osx_double_click_selects_words && io.MouseDoubleClicked[0]))
		{
			edit_state.SelectAll();
			edit_state.SelectedAllMouseLock = true;
		}
		else if (hovered && osx_double_click_selects_words && io.MouseDoubleClicked[0])
		{
			// Select a word only, OS X style (by simulating keystrokes)
			edit_state.OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT);
			edit_state.OnKeyPressed(STB_TEXTEDIT_K_WORDRIGHT | STB_TEXTEDIT_K_SHIFT);
		}
		else if (io.MouseClicked[0] && !edit_state.SelectedAllMouseLock)
		{
			if (hovered)
			{
				stb_textedit_click(&edit_state, &edit_state.StbState, mouse_x, mouse_y);
				edit_state.CursorAnimReset();
			}
		}
		else if (io.MouseDown[0] && !edit_state.SelectedAllMouseLock && (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f))
		{
			stb_textedit_drag(&edit_state, &edit_state.StbState, mouse_x, mouse_y);
			edit_state.CursorAnimReset();
			edit_state.CursorFollow = true;
		}
		if (edit_state.SelectedAllMouseLock && !io.MouseDown[0])
			edit_state.SelectedAllMouseLock = false;

		if (io.InputCharacters[0])
		{
			// Process text input (before we check for Return because using some IME will effectively send a Return?)
			// We ignore CTRL inputs, but need to allow CTRL+ALT as some keyboards (e.g. German) use AltGR - which is Alt+Ctrl - to input certain characters.
			if (!(io.KeyCtrl && !io.KeyAlt) && is_editable)
			{
				for (int n = 0; n < IM_ARRAYSIZE(io.InputCharacters) && io.InputCharacters[n]; n++)
					if (unsigned int c = (unsigned int)io.InputCharacters[n])
					{
						// Insert character if they pass filtering
						if (!InputTextFilterCharacter(&c, flags, callback, user_data))
							continue;
						edit_state.OnKeyPressed((int)c);
					}
			}

			// Consume characters
			memset(g.IO.InputCharacters, 0, sizeof(g.IO.InputCharacters));
		}
	}

	bool cancel_edit = false;
	if (g.ActiveId == id && !g.ActiveIdIsJustActivated && !clear_active_id)
	{
		// Handle key-presses
		const int k_mask = (io.KeyShift ? STB_TEXTEDIT_K_SHIFT : 0);
		const bool is_shortcut_key_only = (io.OptMacOSXBehaviors ? (io.KeySuper && !io.KeyCtrl) : (io.KeyCtrl && !io.KeySuper)) && !io.KeyAlt && !io.KeyShift; // OS X style: Shortcuts using Cmd/Super instead of Ctrl
		const bool is_wordmove_key_down = io.OptMacOSXBehaviors ? io.KeyAlt : io.KeyCtrl;                     // OS X style: Text editing cursor movement using Alt instead of Ctrl
		const bool is_startend_key_down = io.OptMacOSXBehaviors && io.KeySuper && !io.KeyCtrl && !io.KeyAlt;  // OS X style: Line/Text Start and End using Cmd+Arrows instead of Home/End
		const bool is_ctrl_key_only = io.KeyCtrl && !io.KeyShift && !io.KeyAlt && !io.KeySuper;
		const bool is_shift_key_only = io.KeyShift && !io.KeyCtrl && !io.KeyAlt && !io.KeySuper;

		const bool is_cut = ((is_shortcut_key_only && IsKeyPressedMap(ImGuiKey_X)) || (is_shift_key_only && IsKeyPressedMap(ImGuiKey_Delete))) && is_editable && !is_password && (!is_multiline || edit_state.HasSelection());
		const bool is_copy = ((is_shortcut_key_only && IsKeyPressedMap(ImGuiKey_C)) || (is_ctrl_key_only  && IsKeyPressedMap(ImGuiKey_Insert))) && !is_password && (!is_multiline || edit_state.HasSelection());
		const bool is_paste = ((is_shortcut_key_only && IsKeyPressedMap(ImGuiKey_V)) || (is_shift_key_only && IsKeyPressedMap(ImGuiKey_Insert))) && is_editable;

		if (IsKeyPressedMap(ImGuiKey_LeftArrow)) { edit_state.OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINESTART : is_wordmove_key_down ? STB_TEXTEDIT_K_WORDLEFT : STB_TEXTEDIT_K_LEFT) | k_mask); }
		else if (IsKeyPressedMap(ImGuiKey_RightArrow)) { edit_state.OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINEEND : is_wordmove_key_down ? STB_TEXTEDIT_K_WORDRIGHT : STB_TEXTEDIT_K_RIGHT) | k_mask); }
		else if (IsKeyPressedMap(ImGuiKey_UpArrow) && is_multiline) { if (io.KeyCtrl) SetWindowScrollY(draw_window, ImMax(draw_window->Scroll.y - g.FontSize, 0.0f)); else edit_state.OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_TEXTSTART : STB_TEXTEDIT_K_UP) | k_mask); }
		else if (IsKeyPressedMap(ImGuiKey_DownArrow) && is_multiline) { if (io.KeyCtrl) SetWindowScrollY(draw_window, ImMin(draw_window->Scroll.y + g.FontSize, GetScrollMaxY())); else edit_state.OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_TEXTEND : STB_TEXTEDIT_K_DOWN) | k_mask); }
		else if (IsKeyPressedMap(ImGuiKey_Home)) { edit_state.OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTSTART | k_mask : STB_TEXTEDIT_K_LINESTART | k_mask); }
		else if (IsKeyPressedMap(ImGuiKey_End)) { edit_state.OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTEND | k_mask : STB_TEXTEDIT_K_LINEEND | k_mask); }
		else if (IsKeyPressedMap(ImGuiKey_Delete) && is_editable) { edit_state.OnKeyPressed(STB_TEXTEDIT_K_DELETE | k_mask); }
		else if (IsKeyPressedMap(ImGuiKey_Backspace) && is_editable)
		{
			if (!edit_state.HasSelection())
			{
				if (is_wordmove_key_down) edit_state.OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT | STB_TEXTEDIT_K_SHIFT);
				else if (io.OptMacOSXBehaviors && io.KeySuper && !io.KeyAlt && !io.KeyCtrl) edit_state.OnKeyPressed(STB_TEXTEDIT_K_LINESTART | STB_TEXTEDIT_K_SHIFT);
			}
			edit_state.OnKeyPressed(STB_TEXTEDIT_K_BACKSPACE | k_mask);
		}
		else if (IsKeyPressedMap(ImGuiKey_Enter))
		{
			bool ctrl_enter_for_new_line = (flags & ImGuiInputTextFlags_CtrlEnterForNewLine) != 0;
			if (!is_multiline || (ctrl_enter_for_new_line && !io.KeyCtrl) || (!ctrl_enter_for_new_line && io.KeyCtrl))
			{
				enter_pressed = clear_active_id = true;
			}
			else if (is_editable)
			{
				unsigned int c = '\n'; // Insert new line
				if (InputTextFilterCharacter(&c, flags, callback, user_data))
					edit_state.OnKeyPressed((int)c);
			}
		}
		else if ((flags & ImGuiInputTextFlags_AllowTabInput) && IsKeyPressedMap(ImGuiKey_Tab) && !io.KeyCtrl && !io.KeyShift && !io.KeyAlt && is_editable)
		{
			unsigned int c = '\t'; // Insert TAB
			if (InputTextFilterCharacter(&c, flags, callback, user_data))
				edit_state.OnKeyPressed((int)c);
		}
		else if (IsKeyPressedMap(ImGuiKey_Escape)) { clear_active_id = cancel_edit = true; }
		else if (is_shortcut_key_only && IsKeyPressedMap(ImGuiKey_Z) && is_editable && is_undoable) { edit_state.OnKeyPressed(STB_TEXTEDIT_K_UNDO); edit_state.ClearSelection(); }
		else if (is_shortcut_key_only && IsKeyPressedMap(ImGuiKey_Y) && is_editable && is_undoable) { edit_state.OnKeyPressed(STB_TEXTEDIT_K_REDO); edit_state.ClearSelection(); }
		else if (is_shortcut_key_only && IsKeyPressedMap(ImGuiKey_A)) { edit_state.SelectAll(); edit_state.CursorFollow = true; }
		else if (is_cut || is_copy)
		{
			// Cut, Copy
			if (io.SetClipboardTextFn)
			{
				const int ib = edit_state.HasSelection() ? ImMin(edit_state.StbState.select_start, edit_state.StbState.select_end) : 0;
				const int ie = edit_state.HasSelection() ? ImMax(edit_state.StbState.select_start, edit_state.StbState.select_end) : edit_state.CurLenW;
				edit_state.TempTextBuffer.resize((ie - ib) * 4 + 1);
				ImTextStrToUtf8(edit_state.TempTextBuffer.Data, edit_state.TempTextBuffer.Size, edit_state.Text.Data + ib, edit_state.Text.Data + ie);
				SetClipboardText(edit_state.TempTextBuffer.Data);
			}

			if (is_cut)
			{
				if (!edit_state.HasSelection())
					edit_state.SelectAll();
				edit_state.CursorFollow = true;
				stb_textedit_cut(&edit_state, &edit_state.StbState);
			}
		}
		else if (is_paste)
		{
			// Paste
			if (const char* clipboard = GetClipboardText())
			{
				// Filter pasted buffer
				const int clipboard_len = (int)strlen(clipboard);
				ImWchar* clipboard_filtered = (ImWchar*)ImGui::MemAlloc((clipboard_len + 1) * sizeof(ImWchar));
				int clipboard_filtered_len = 0;
				for (const char* s = clipboard; *s; )
				{
					unsigned int c;
					s += ImTextCharFromUtf8(&c, s, NULL);
					if (c == 0)
						break;
					if (c >= 0x10000 || !InputTextFilterCharacter(&c, flags, callback, user_data))
						continue;
					clipboard_filtered[clipboard_filtered_len++] = (ImWchar)c;
				}
				clipboard_filtered[clipboard_filtered_len] = 0;
				if (clipboard_filtered_len > 0) // If everything was filtered, ignore the pasting operation
				{
					stb_textedit_paste(&edit_state, &edit_state.StbState, clipboard_filtered, clipboard_filtered_len);
					edit_state.CursorFollow = true;
				}
				ImGui::MemFree(clipboard_filtered);
			}
		}
	}

	if (g.ActiveId == id)
	{
		if (cancel_edit)
		{
			// Restore initial value
			if (is_editable)
			{
				ImStrncpy(buf, edit_state.InitialText.Data, buf_size);
				value_changed = true;
			}
		}

		// When using 'ImGuiInputTextFlags_EnterReturnsTrue' as a special case we reapply the live buffer back to the input buffer before clearing ActiveId, even though strictly speaking it wasn't modified on this frame.
		// If we didn't do that, code like InputInt() with ImGuiInputTextFlags_EnterReturnsTrue would fail. Also this allows the user to use InputText() with ImGuiInputTextFlags_EnterReturnsTrue without maintaining any user-side storage.
		bool apply_edit_back_to_user_buffer = !cancel_edit || (enter_pressed && (flags & ImGuiInputTextFlags_EnterReturnsTrue) != 0);
		if (apply_edit_back_to_user_buffer)
		{
			// Apply new value immediately - copy modified buffer back
			// Note that as soon as the input box is active, the in-widget value gets priority over any underlying modification of the input buffer
			// FIXME: We actually always render 'buf' when calling DrawList->AddText, making the comment above incorrect.
			// FIXME-OPT: CPU waste to do this every time the widget is active, should mark dirty state from the stb_textedit callbacks.
			if (is_editable)
			{
				edit_state.TempTextBuffer.resize(edit_state.Text.Size * 4);
				ImTextStrToUtf8(edit_state.TempTextBuffer.Data, edit_state.TempTextBuffer.Size, edit_state.Text.Data, NULL);
			}

			// User callback
			if ((flags & (ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackAlways)) != 0)
			{
				IM_ASSERT(callback != NULL);

				// The reason we specify the usage semantic (Completion/History) is that Completion needs to disable keyboard TABBING at the moment.
				ImGuiInputTextFlags event_flag = 0;
				ImGuiKey event_key = ImGuiKey_COUNT;
				if ((flags & ImGuiInputTextFlags_CallbackCompletion) != 0 && IsKeyPressedMap(ImGuiKey_Tab))
				{
					event_flag = ImGuiInputTextFlags_CallbackCompletion;
					event_key = ImGuiKey_Tab;
				}
				else if ((flags & ImGuiInputTextFlags_CallbackHistory) != 0 && IsKeyPressedMap(ImGuiKey_UpArrow))
				{
					event_flag = ImGuiInputTextFlags_CallbackHistory;
					event_key = ImGuiKey_UpArrow;
				}
				else if ((flags & ImGuiInputTextFlags_CallbackHistory) != 0 && IsKeyPressedMap(ImGuiKey_DownArrow))
				{
					event_flag = ImGuiInputTextFlags_CallbackHistory;
					event_key = ImGuiKey_DownArrow;
				}
				else if (flags & ImGuiInputTextFlags_CallbackAlways)
					event_flag = ImGuiInputTextFlags_CallbackAlways;

				if (event_flag)
				{
					ImGuiTextEditCallbackData callback_data;
					memset(&callback_data, 0, sizeof(ImGuiTextEditCallbackData));
					callback_data.EventFlag = event_flag;
					callback_data.Flags = flags;
					callback_data.UserData = user_data;
					callback_data.ReadOnly = !is_editable;

					callback_data.EventKey = event_key;
					callback_data.Buf = edit_state.TempTextBuffer.Data;
					callback_data.BufTextLen = edit_state.CurLenA;
					callback_data.BufSize = edit_state.BufSizeA;
					callback_data.BufDirty = false;

					// We have to convert from wchar-positions to UTF-8-positions, which can be pretty slow (an incentive to ditch the ImWchar buffer, see https://github.com/nothings/stb/issues/188)
					ImWchar* text = edit_state.Text.Data;
					const int utf8_cursor_pos = callback_data.CursorPos = ImTextCountUtf8BytesFromStr(text, text + edit_state.StbState.cursor);
					const int utf8_selection_start = callback_data.SelectionStart = ImTextCountUtf8BytesFromStr(text, text + edit_state.StbState.select_start);
					const int utf8_selection_end = callback_data.SelectionEnd = ImTextCountUtf8BytesFromStr(text, text + edit_state.StbState.select_end);

					// Call user code
					callback(&callback_data);

					// Read back what user may have modified
					IM_ASSERT(callback_data.Buf == edit_state.TempTextBuffer.Data);  // Invalid to modify those fields
					IM_ASSERT(callback_data.BufSize == edit_state.BufSizeA);
					IM_ASSERT(callback_data.Flags == flags);
					if (callback_data.CursorPos != utf8_cursor_pos)            edit_state.StbState.cursor = ImTextCountCharsFromUtf8(callback_data.Buf, callback_data.Buf + callback_data.CursorPos);
					if (callback_data.SelectionStart != utf8_selection_start)  edit_state.StbState.select_start = ImTextCountCharsFromUtf8(callback_data.Buf, callback_data.Buf + callback_data.SelectionStart);
					if (callback_data.SelectionEnd != utf8_selection_end)      edit_state.StbState.select_end = ImTextCountCharsFromUtf8(callback_data.Buf, callback_data.Buf + callback_data.SelectionEnd);
					if (callback_data.BufDirty)
					{
						IM_ASSERT(callback_data.BufTextLen == (int)strlen(callback_data.Buf)); // You need to maintain BufTextLen if you change the text!
						edit_state.CurLenW = ImTextStrFromUtf8(edit_state.Text.Data, edit_state.Text.Size, callback_data.Buf, NULL);
						edit_state.CurLenA = callback_data.BufTextLen;  // Assume correct length and valid UTF-8 from user, saves us an extra strlen()
						edit_state.CursorAnimReset();
					}
				}
			}

			// Copy back to user buffer
			if (is_editable && strcmp(edit_state.TempTextBuffer.Data, buf) != 0)
			{
				ImStrncpy(buf, edit_state.TempTextBuffer.Data, buf_size);
				value_changed = true;
			}
		}
	}

	// Release active ID at the end of the function (so e.g. pressing Return still does a final application of the value)
	if (clear_active_id && g.ActiveId == id)
		ClearActiveID();

	// Render
	// Select which buffer we are going to display. When ImGuiInputTextFlags_NoLiveEdit is set 'buf' might still be the old value. We set buf to NULL to prevent accidental usage from now on.
	const char* buf_display = (g.ActiveId == id && is_editable) ? edit_state.TempTextBuffer.Data : buf; buf = NULL;

	if (!is_multiline)
		RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

	const ImVec4 clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + size.x, frame_bb.Min.y + size.y); // Not using frame_bb.Max because we have adjusted size
	ImVec2 render_pos = is_multiline ? draw_window->DC.CursorPos : frame_bb.Min + style.FramePadding;
	ImVec2 text_size(0.f, 0.f);
	const bool is_currently_scrolling = (edit_state.Id == id && is_multiline && g.ActiveId == draw_window->GetIDNoKeepAlive("#SCROLLY"));
	if (g.ActiveId == id || is_currently_scrolling)
	{
		edit_state.CursorAnim += io.DeltaTime;

		// This is going to be messy. We need to:
		// - Display the text (this alone can be more easily clipped)
		// - Handle scrolling, highlight selection, display cursor (those all requires some form of 1d->2d cursor position calculation)
		// - Measure text height (for scrollbar)
		// We are attempting to do most of that in **one main pass** to minimize the computation cost (non-negligible for large amount of text) + 2nd pass for selection rendering (we could merge them by an extra refactoring effort)
		// FIXME: This should occur on buf_display but we'd need to maintain cursor/select_start/select_end for UTF-8.
		const ImWchar* text_begin = edit_state.Text.Data;
		ImVec2 cursor_offset, select_start_offset;

		{
			// Count lines + find lines numbers straddling 'cursor' and 'select_start' position.
			const ImWchar* searches_input_ptr[2];
			searches_input_ptr[0] = text_begin + edit_state.StbState.cursor;
			searches_input_ptr[1] = NULL;
			int searches_remaining = 1;
			int searches_result_line_number[2] = { -1, -999 };
			if (edit_state.StbState.select_start != edit_state.StbState.select_end)
			{
				searches_input_ptr[1] = text_begin + ImMin(edit_state.StbState.select_start, edit_state.StbState.select_end);
				searches_result_line_number[1] = -1;
				searches_remaining++;
			}

			// Iterate all lines to find our line numbers
			// In multi-line mode, we never exit the loop until all lines are counted, so add one extra to the searches_remaining counter.
			searches_remaining += is_multiline ? 1 : 0;
			int line_count = 0;
			for (const ImWchar* s = text_begin; *s != 0; s++)
				if (*s == '\n')
				{
					line_count++;
					if (searches_result_line_number[0] == -1 && s >= searches_input_ptr[0]) { searches_result_line_number[0] = line_count; if (--searches_remaining <= 0) break; }
					if (searches_result_line_number[1] == -1 && s >= searches_input_ptr[1]) { searches_result_line_number[1] = line_count; if (--searches_remaining <= 0) break; }
				}
			line_count++;
			if (searches_result_line_number[0] == -1) searches_result_line_number[0] = line_count;
			if (searches_result_line_number[1] == -1) searches_result_line_number[1] = line_count;

			// Calculate 2d position by finding the beginning of the line and measuring distance
			cursor_offset.x = InputTextCalcTextSizeW(ImStrbolW(searches_input_ptr[0], text_begin), searches_input_ptr[0]).x;
			cursor_offset.y = searches_result_line_number[0] * g.FontSize;
			if (searches_result_line_number[1] >= 0)
			{
				select_start_offset.x = InputTextCalcTextSizeW(ImStrbolW(searches_input_ptr[1], text_begin), searches_input_ptr[1]).x;
				select_start_offset.y = searches_result_line_number[1] * g.FontSize;
			}

			// Store text height (note that we haven't calculated text width at all, see GitHub issues #383, #1224)
			if (is_multiline)
				text_size = ImVec2(size.x, line_count * g.FontSize);
		}

		// Scroll
		if (edit_state.CursorFollow)
		{
			// Horizontal scroll in chunks of quarter width
			if (!(flags & ImGuiInputTextFlags_NoHorizontalScroll))
			{
				const float scroll_increment_x = size.x * 0.25f;
				if (cursor_offset.x < edit_state.ScrollX)
					edit_state.ScrollX = (float)(int)ImMax(0.0f, cursor_offset.x - scroll_increment_x);
				else if (cursor_offset.x - size.x >= edit_state.ScrollX)
					edit_state.ScrollX = (float)(int)(cursor_offset.x - size.x + scroll_increment_x);
			}
			else
			{
				edit_state.ScrollX = 0.0f;
			}

			// Vertical scroll
			if (is_multiline)
			{
				float scroll_y = draw_window->Scroll.y;
				if (cursor_offset.y - g.FontSize < scroll_y)
					scroll_y = ImMax(0.0f, cursor_offset.y - g.FontSize);
				else if (cursor_offset.y - size.y >= scroll_y)
					scroll_y = cursor_offset.y - size.y;
				draw_window->DC.CursorPos.y += (draw_window->Scroll.y - scroll_y);   // To avoid a frame of lag
				draw_window->Scroll.y = scroll_y;
				render_pos.y = draw_window->DC.CursorPos.y;
			}
		}
		edit_state.CursorFollow = false;
		const ImVec2 render_scroll = ImVec2(edit_state.ScrollX, 0.0f);

		// Draw selection
		if (edit_state.StbState.select_start != edit_state.StbState.select_end)
		{
			const ImWchar* text_selected_begin = text_begin + ImMin(edit_state.StbState.select_start, edit_state.StbState.select_end);
			const ImWchar* text_selected_end = text_begin + ImMax(edit_state.StbState.select_start, edit_state.StbState.select_end);

			float bg_offy_up = is_multiline ? 0.0f : -1.0f;    // FIXME: those offsets should be part of the style? they don't play so well with multi-line selection.
			float bg_offy_dn = is_multiline ? 0.0f : 2.0f;
			ImU32 bg_color = GetColorU32(ImGuiCol_TextSelectedBg);
			ImVec2 rect_pos = render_pos + select_start_offset - render_scroll;
			for (const ImWchar* p = text_selected_begin; p < text_selected_end; )
			{
				if (rect_pos.y > clip_rect.w + g.FontSize)
					break;
				if (rect_pos.y < clip_rect.y)
				{
					while (p < text_selected_end)
						if (*p++ == '\n')
							break;
				}
				else
				{
					ImVec2 rect_size = InputTextCalcTextSizeW(p, text_selected_end, &p, NULL, true);
					if (rect_size.x <= 0.0f) rect_size.x = (float)(int)(g.Font->GetCharAdvance((unsigned short)' ') * 0.50f); // So we can see selected empty lines
					ImRect rect(rect_pos + ImVec2(0.0f, bg_offy_up - g.FontSize), rect_pos + ImVec2(rect_size.x, bg_offy_dn));
					rect.ClipWith(clip_rect);
					if (rect.Overlaps(clip_rect))
						draw_window->DrawList->AddRectFilled(rect.Min, rect.Max, bg_color);
				}
				rect_pos.x = render_pos.x - render_scroll.x;
				rect_pos.y += g.FontSize;
			}
		}

		draw_window->DrawList->AddText(g.Font, g.FontSize, render_pos - render_scroll, GetColorU32(ImGuiCol_Text), buf_display, buf_display + edit_state.CurLenA, 0.0f, is_multiline ? NULL : &clip_rect);

		// Draw blinking cursor
		bool cursor_is_visible = (!g.IO.OptCursorBlink) || (g.InputTextState.CursorAnim <= 0.0f) || fmodf(g.InputTextState.CursorAnim, 1.20f) <= 0.80f;
		ImVec2 cursor_screen_pos = render_pos + cursor_offset - render_scroll;
		ImRect cursor_screen_rect(cursor_screen_pos.x, cursor_screen_pos.y - g.FontSize + 0.5f, cursor_screen_pos.x + 1.0f, cursor_screen_pos.y - 1.5f);
		if (cursor_is_visible && cursor_screen_rect.Overlaps(clip_rect))
			draw_window->DrawList->AddLine(cursor_screen_rect.Min, cursor_screen_rect.GetBL(), GetColorU32(ImGuiCol_Text));

		// Notify OS of text input position for advanced IME (-1 x offset so that Windows IME can cover our cursor. Bit of an extra nicety.)
		if (is_editable)
			g.OsImePosRequest = ImVec2(cursor_screen_pos.x - 1, cursor_screen_pos.y - g.FontSize);
	}
	else
	{
		// Render text only
		const char* buf_end = NULL;
		if (is_multiline)
			text_size = ImVec2(size.x, InputTextCalcTextLenAndLineCount(buf_display, &buf_end) * g.FontSize); // We don't need width
		draw_window->DrawList->AddText(g.Font, g.FontSize, render_pos, GetColorU32(ImGuiCol_Text), buf_display, buf_end, 0.0f, is_multiline ? NULL : &clip_rect);
	}

	if (is_multiline)
	{
		Dummy(text_size + ImVec2(0.0f, g.FontSize)); // Always add room to scroll an extra line
		EndChildFrame();
		EndGroup();
	}

	if (is_password)
		PopFont();

	// Log as text
	if (g.LogEnabled && !is_password)
		LogRenderedText(&render_pos, buf_display, NULL);

	if (label_size.x > 0)
		RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

	if ((flags & ImGuiInputTextFlags_EnterReturnsTrue) != 0)
		return enter_pressed;
	else
		return value_changed;
}

bool ImGui::InputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiTextEditCallback callback, void* user_data)
{
	IM_ASSERT(!(flags & ImGuiInputTextFlags_Multiline)); // call InputTextMultiline()
	return InputTextEx(label, buf, (int)buf_size, ImVec2(0, 0), flags, callback, user_data);
}

bool ImGui::InputTextMultiline(const char* label, char* buf, size_t buf_size, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiTextEditCallback callback, void* user_data)
{
	return InputTextEx(label, buf, (int)buf_size, size, flags | ImGuiInputTextFlags_Multiline, callback, user_data);
}

// NB: scalar_format here must be a simple "%xx" format string with no prefix/suffix (unlike the Drag/Slider functions "display_format" argument)
bool ImGui::InputScalarEx(const char* label, ImGuiDataType data_type, void* data_ptr, void* step_ptr, void* step_fast_ptr, const char* scalar_format, ImGuiInputTextFlags extra_flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	BeginGroup();
	PushID(label);
	const ImVec2 button_sz = ImVec2(GetFrameHeight(), GetFrameHeight());
	if (step_ptr)
		PushItemWidth(ImMax(1.0f, CalcItemWidth() - (button_sz.x + style.ItemInnerSpacing.x) * 2));

	char buf[64];
	DataTypeFormatString(data_type, data_ptr, scalar_format, buf, IM_ARRAYSIZE(buf));

	bool value_changed = false;
	if (!(extra_flags & ImGuiInputTextFlags_CharsHexadecimal))
		extra_flags |= ImGuiInputTextFlags_CharsDecimal;
	extra_flags |= ImGuiInputTextFlags_AutoSelectAll;
	if (InputText("", buf, IM_ARRAYSIZE(buf), extra_flags)) // PushId(label) + "" gives us the expected ID from outside point of view
		value_changed = DataTypeApplyOpFromText(buf, GImGui->InputTextState.InitialText.begin(), data_type, data_ptr, scalar_format);

	// Step buttons
	if (step_ptr)
	{
		PopItemWidth();
		SameLine(0, style.ItemInnerSpacing.x);
		if (ButtonEx("-", button_sz, ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups))
		{
			DataTypeApplyOp(data_type, '-', data_ptr, g.IO.KeyCtrl && step_fast_ptr ? step_fast_ptr : step_ptr);
			value_changed = true;
		}
		SameLine(0, style.ItemInnerSpacing.x);
		if (ButtonEx("+", button_sz, ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups))
		{
			DataTypeApplyOp(data_type, '+', data_ptr, g.IO.KeyCtrl && step_fast_ptr ? step_fast_ptr : step_ptr);
			value_changed = true;
		}
	}
	PopID();

	if (label_size.x > 0)
	{
		SameLine(0, style.ItemInnerSpacing.x);
		RenderText(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + style.FramePadding.y), label);
		ItemSize(label_size, style.FramePadding.y);
	}
	EndGroup();

	return value_changed;
}

bool ImGui::InputFloat(const char* label, float* v, float step, float step_fast, int decimal_precision, ImGuiInputTextFlags extra_flags)
{
	char display_format[16];
	if (decimal_precision < 0)
		strcpy(display_format, "%f");      // Ideally we'd have a minimum decimal precision of 1 to visually denote that this is a float, while hiding non-significant digits? %f doesn't have a minimum of 1
	else
		ImFormatString(display_format, IM_ARRAYSIZE(display_format), "%%.%df", decimal_precision);
	return InputScalarEx(label, ImGuiDataType_Float, (void*)v, (void*)(step>0.0f ? &step : NULL), (void*)(step_fast>0.0f ? &step_fast : NULL), display_format, extra_flags);
}

bool ImGui::InputInt(const char* label, int* v, int step, int step_fast, ImGuiInputTextFlags extra_flags)
{
	// Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use InputText() to parse your own data, if you want to handle prefixes.
	const char* scalar_format = (extra_flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
	return InputScalarEx(label, ImGuiDataType_Int, (void*)v, (void*)(step>0.0f ? &step : NULL), (void*)(step_fast>0.0f ? &step_fast : NULL), scalar_format, extra_flags);
}

bool ImGui::InputFloatN(const char* label, float* v, int components, int decimal_precision, ImGuiInputTextFlags extra_flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	bool value_changed = false;
	BeginGroup();
	PushID(label);
	PushMultiItemsWidths(components);
	for (int i = 0; i < components; i++)
	{
		PushID(i);
		value_changed |= InputFloat("##v", &v[i], 0, 0, decimal_precision, extra_flags);
		SameLine(0, g.Style.ItemInnerSpacing.x);
		PopID();
		PopItemWidth();
	}
	PopID();

	TextUnformatted(label, FindRenderedTextEnd(label));
	EndGroup();

	return value_changed;
}

bool ImGui::InputFloat2(const char* label, float v[2], int decimal_precision, ImGuiInputTextFlags extra_flags)
{
	return InputFloatN(label, v, 2, decimal_precision, extra_flags);
}

bool ImGui::InputFloat3(const char* label, float v[3], int decimal_precision, ImGuiInputTextFlags extra_flags)
{
	return InputFloatN(label, v, 3, decimal_precision, extra_flags);
}

bool ImGui::InputFloat4(const char* label, float v[4], int decimal_precision, ImGuiInputTextFlags extra_flags)
{
	return InputFloatN(label, v, 4, decimal_precision, extra_flags);
}

bool ImGui::InputIntN(const char* label, int* v, int components, ImGuiInputTextFlags extra_flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	bool value_changed = false;
	BeginGroup();
	PushID(label);
	PushMultiItemsWidths(components);
	for (int i = 0; i < components; i++)
	{
		PushID(i);
		value_changed |= InputInt("##v", &v[i], 0, 0, extra_flags);
		SameLine(0, g.Style.ItemInnerSpacing.x);
		PopID();
		PopItemWidth();
	}
	PopID();

	TextUnformatted(label, FindRenderedTextEnd(label));
	EndGroup();

	return value_changed;
}

bool ImGui::InputInt2(const char* label, int v[2], ImGuiInputTextFlags extra_flags)
{
	return InputIntN(label, v, 2, extra_flags);
}

bool ImGui::InputInt3(const char* label, int v[3], ImGuiInputTextFlags extra_flags)
{
	return InputIntN(label, v, 3, extra_flags);
}

bool ImGui::InputInt4(const char* label, int v[4], ImGuiInputTextFlags extra_flags)
{
	return InputIntN(label, v, 4, extra_flags);
}

const char* const KeyNames[] = {
	"Unknown",
	"VK_LBUTTON",
	"VK_RBUTTON",
	"VK_CANCEL",
	"VK_MBUTTON",
	"VK_XBUTTON1",
	"VK_XBUTTON2",
	"Unknown",
	"VK_BACK",
	"VK_TAB",
	"Unknown",
	"Unknown",
	"VK_CLEAR",
	"VK_RETURN",
	"Unknown",
	"Unknown",
	"VK_SHIFT",
	"VK_CONTROL",
	"VK_MENU",
	"VK_PAUSE",
	"VK_CAPITAL",
	"VK_KANA",
	"Unknown",
	"VK_JUNJA",
	"VK_FINAL",
	"VK_KANJI",
	"Unknown",
	"VK_ESCAPE",
	"VK_CONVERT",
	"VK_NONCONVERT",
	"VK_ACCEPT",
	"VK_MODECHANGE",
	"VK_SPACE",
	"VK_PRIOR",
	"VK_NEXT",
	"VK_END",
	"VK_HOME",
	"VK_LEFT",
	"VK_UP",
	"VK_RIGHT",
	"VK_DOWN",
	"VK_SELECT",
	"VK_PRINT",
	"VK_EXECUTE",
	"VK_SNAPSHOT",
	"VK_INSERT",
	"VK_DELETE",
	"VK_HELP",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"VK_LWIN",
	"VK_RWIN",
	"VK_APPS",
	"Unknown",
	"VK_SLEEP",
	"VK_NUMPAD0",
	"VK_NUMPAD1",
	"VK_NUMPAD2",
	"VK_NUMPAD3",
	"VK_NUMPAD4",
	"VK_NUMPAD5",
	"VK_NUMPAD6",
	"VK_NUMPAD7",
	"VK_NUMPAD8",
	"VK_NUMPAD9",
	"VK_MULTIPLY",
	"VK_ADD",
	"VK_SEPARATOR",
	"VK_SUBTRACT",
	"VK_DECIMAL",
	"VK_DIVIDE",
	"VK_F1",
	"VK_F2",
	"VK_F3",
	"VK_F4",
	"VK_F5",
	"VK_F6",
	"VK_F7",
	"VK_F8",
	"VK_F9",
	"VK_F10",
	"VK_F11",
	"VK_F12",
	"VK_F13",
	"VK_F14",
	"VK_F15",
	"VK_F16",
	"VK_F17",
	"VK_F18",
	"VK_F19",
	"VK_F20",
	"VK_F21",
	"VK_F22",
	"VK_F23",
	"VK_F24",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"VK_NUMLOCK",
	"VK_SCROLL",
	"VK_OEM_NEC_EQUAL",
	"VK_OEM_FJ_MASSHOU",
	"VK_OEM_FJ_TOUROKU",
	"VK_OEM_FJ_LOYA",
	"VK_OEM_FJ_ROYA",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"VK_LSHIFT",
	"VK_RSHIFT",
	"VK_LCONTROL",
	"VK_RCONTROL",
	"VK_LMENU",
	"VK_RMENU"
};

#define VK_LBUTTON        0x01
#define VK_RBUTTON        0x02
#define VK_MBUTTON        0x04    /* NOT contiguous with L & RBUTTON */
#define VK_XBUTTON1       0x05    /* NOT contiguous with L & RBUTTON */
#define VK_XBUTTON2       0x06    /* NOT contiguous with L & RBUTTON */
#define VK_BACK           0x08
#define VK_RMENU          0xA5

bool ImGui::Hotkey(const char* label, int* k, const ImVec2& size_arg)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	const ImGuiStyle& style = g.Style;

	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	ImVec2 size = CalcItemSize(size_arg, CalcItemWidth(), label_size.y + style.FramePadding.y*2.0f);
	const ImRect frame_bb(window->DC.CursorPos + ImVec2(label_size.x + style.ItemInnerSpacing.x, 0.0f), window->DC.CursorPos + size);
	const ImRect total_bb(window->DC.CursorPos, frame_bb.Max);

	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd2(total_bb, &id))
		return false;

	const bool focus_requested = FocusableItemRegister(window, g.ActiveId == id, false);
	const bool focus_requested_by_code = focus_requested && (window->FocusIdxAllCounter == window->FocusIdxAllRequestCurrent);
	const bool focus_requested_by_tab = focus_requested && !focus_requested_by_code;
	const bool hovered = IsHovered(frame_bb, id);

	if (hovered)
	{
		SetHoveredID(id);
		g.MouseCursor = ImGuiMouseCursor_TextInput;
	}

	const bool user_clicked = hovered && io.MouseClicked[0];

	if (focus_requested || user_clicked)
	{
		if (g.ActiveId != id) {
			memset(io.MouseDown, 0, sizeof(io.MouseDown));
			memset(io.KeysDown, 0, sizeof(io.KeysDown));
			*k = 0;
		}
		SetActiveID(id, window);
		FocusWindow(window);
	}
	else if (io.MouseClicked[0]) {
		if (g.ActiveId == id)
			ClearActiveID();
	}

	bool value_changed = false;
	int key = *k;

	if (g.ActiveId == id) {
		for (auto i = 0; i < 5; i++) {
			if (io.MouseDown[i]) {
				switch (i) {
				case 0:
					key = VK_LBUTTON;
					break;
				case 1:
					key = VK_RBUTTON;
					break;
				case 2:
					key = VK_MBUTTON;
					break;
				case 3:
					key = VK_XBUTTON1;
					break;
				case 4:
					key = VK_XBUTTON2;
					break;
				}
				value_changed = true;
				ClearActiveID();
			}
		}
		if (!value_changed) {
			for (auto i = VK_BACK; i <= VK_RMENU; i++) {
				if (io.KeysDown[i]) {
					key = i;
					value_changed = true;
					ClearActiveID();
				}
			}
		}

		if (IsKeyPressedMap(ImGuiKey_Escape)) {
			*k = 0;
			ClearActiveID();
		}
		else {
			*k = key;
		}
	}

	// Render
	// Select which buffer we are going to display. When ImGuiInputTextFlags_NoLiveEdit is Set 'buf' might still be the old value. We Set buf to NULL to prevent accidental usage from now on.

	char buf_display[64] = "None";

	RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

	if (*k != 0 && g.ActiveId != id) {
		strcpy(buf_display, KeyNames[*k]);
	}
	else if (g.ActiveId == id) {
		strcpy(buf_display, "<Press a key>");
	}

	const ImRect clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + size.x, frame_bb.Min.y + size.y); // Not using frame_bb.Max because we have adjusted size
	ImVec2 render_pos = frame_bb.Min + style.FramePadding;
	RenderTextClipped(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding, buf_display, NULL, NULL);
	//draw_window->DrawList->AddText(g.Font, g.FontSize, render_pos, GetColorU32(ImGuiCol_Text), buf_display, NULL, 0.0f, &clip_rect);

	if (label_size.x > 0)
		RenderText(ImVec2(total_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), label);

	return value_changed;
}


static float CalcMaxPopupHeightFromItemCount(int items_count)
{
	ImGuiContext& g = *GImGui;
	if (items_count <= 0)
		return FLT_MAX;
	return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
}

bool ImGui::BeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags)
{
	// Always consume the SetNextWindowSizeConstraint() call in our early return paths
	ImGuiContext& g = *GImGui;
	ImGuiCond backup_next_window_size_constraint = g.NextWindowData.SizeConstraintCond;
	g.NextWindowData.SizeConstraintCond = 0;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const float w = CalcItemWidth();

	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y*2.0f));
	const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(frame_bb, id, &hovered, &held);
	bool popup_open = IsPopupOpen(id);

	const float arrow_size = GetFrameHeight();
	const ImRect value_bb(frame_bb.Min, frame_bb.Max - ImVec2(arrow_size, 0.0f));
	RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg - 10), true, style.FrameRounding);
	RenderFrame(ImVec2(frame_bb.Max.x - arrow_size, frame_bb.Min.y), frame_bb.Max, GetColorU32(popup_open || hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button), true, style.FrameRounding); // FIXME-ROUNDING
	RenderTriangle(ImVec2(frame_bb.Max.x - arrow_size + style.FramePadding.y, frame_bb.Min.y + style.FramePadding.y), ImGuiDir_Down);
	if (preview_value != NULL)
		RenderTextClipped(frame_bb.Min + style.FramePadding, value_bb.Max, preview_value, NULL, NULL, ImVec2(0.0f, 0.0f));
	if (label_size.x > 0)
		RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

	if (pressed && !popup_open)
	{
		OpenPopupEx(id);
		popup_open = true;
	}

	if (!popup_open)
		return false;

	if (backup_next_window_size_constraint)
	{
		g.NextWindowData.SizeConstraintCond = backup_next_window_size_constraint;
		g.NextWindowData.SizeConstraintRect.Min.x = ImMax(g.NextWindowData.SizeConstraintRect.Min.x, w);
	}
	else
	{
		if ((flags & ImGuiComboFlags_HeightMask_) == 0)
			flags |= ImGuiComboFlags_HeightRegular;
		IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiComboFlags_HeightMask_));    // Only one
		int popup_max_height_in_items = -1;
		if (flags & ImGuiComboFlags_HeightRegular)     popup_max_height_in_items = 8;
		else if (flags & ImGuiComboFlags_HeightSmall)  popup_max_height_in_items = 4;
		else if (flags & ImGuiComboFlags_HeightLarge)  popup_max_height_in_items = 20;
		SetNextWindowSizeConstraints(ImVec2(w, 0.0f), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));
	}

	char name[16];
	ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.CurrentPopupStack.Size); // Recycle windows based on depth

																						// Peak into expected window size so we can position it
	if (ImGuiWindow* popup_window = FindWindowByName(name))
		if (popup_window->WasActive)
		{
			ImVec2 size_contents = CalcSizeContents(popup_window);
			ImVec2 size_expected = CalcSizeAfterConstraint(popup_window, CalcSizeAutoFit(popup_window, size_contents));
			if (flags & ImGuiComboFlags_PopupAlignLeft)
				popup_window->AutoPosLastDirection = ImGuiDir_Left;
			ImVec2 pos = FindBestWindowPosForPopup(frame_bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, frame_bb, ImGuiPopupPositionPolicy_ComboBox);
			SetNextWindowPos(pos);
		}

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
	if (!Begin(name, NULL, window_flags))
	{
		EndPopup();
		IM_ASSERT(0);   // This should never happen as we tested for IsPopupOpen() above
		return false;
	}

	// Horizontally align ourselves with the framed text
	if (style.FramePadding.x != style.WindowPadding.x)
		Indent(style.FramePadding.x - style.WindowPadding.x);

	return true;
}

void ImGui::EndCombo()
{
	const ImGuiStyle& style = GImGui->Style;
	if (style.FramePadding.x != style.WindowPadding.x)
		Unindent(style.FramePadding.x - style.WindowPadding.x);
	EndPopup();
}

// Old API, prefer using BeginCombo() nowadays if you can.//
bool ImGui::Combo(const char* label, int* current_item, bool(*items_getter)(void*, int, const char**), void* data, int items_count, int popup_max_height_in_items)
{
	ImGuiContext& g = *GImGui;

	const char* preview_text = NULL;
	if (*current_item >= 0 && *current_item < items_count)
		items_getter(data, *current_item, &preview_text);

	// The old Combo() API exposed "popup_max_height_in_items", however the new more general BeginCombo() API doesn't, so we emulate it here.
	if (popup_max_height_in_items != -1 && !g.NextWindowData.SizeConstraintCond)
	{
		float popup_max_height = CalcMaxPopupHeightFromItemCount(popup_max_height_in_items);
		SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, popup_max_height));
	}

	if (!BeginCombo(label, preview_text, 0))
		return false;

	// Display items
	// FIXME-OPT: Use clipper (but we need to disable it on the appearing frame to make sure our call to SetItemDefaultFocus() is processed)
	bool value_changed = false;
	for (int i = 0; i < items_count; i++)
	{
		PushID((void*)(intptr_t)i);
		const bool item_selected = (i == *current_item);
		const char* item_text;
		if (!items_getter(data, i, &item_text))
			item_text = "*Unknown item*";
		if (Selectable(item_text, item_selected))
		{
			value_changed = true;
			*current_item = i;
		}
		if (item_selected)
			SetItemDefaultFocus();
		PopID();
	}

	EndCombo();
	return value_changed;
}

static bool Items_ArrayGetter(void* data, int idx, const char** out_text)
{
	const char* const* items = (const char* const*)data;
	if (out_text)
		*out_text = items[idx];
	return true;
}

static bool Items_SingleStringGetter(void* data, int idx, const char** out_text)
{
	// FIXME-OPT: we could pre-compute the indices to fasten this. But only 1 active combo means the waste is limited.
	const char* items_separated_by_zeros = (const char*)data;
	int items_count = 0;
	const char* p = items_separated_by_zeros;
	while (*p)
	{
		if (idx == items_count)
			break;
		p += strlen(p) + 1;
		items_count++;
	}
	if (!*p)
		return false;
	if (out_text)
		*out_text = p;
	return true;
}

// Combo box helper allowing to pass an array of strings.
bool ImGui::Combo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items)
{
	const bool value_changed = Combo(label, current_item, Items_ArrayGetter, (void*)items, items_count, height_in_items);
	return value_changed;
}

// Combo box helper allowing to pass all items in a single string.
bool ImGui::Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int height_in_items)
{
	int items_count = 0;
	const char* p = items_separated_by_zeros;       // FIXME-OPT: Avoid computing this, or at least only when combo is open
	while (*p)
	{
		p += strlen(p) + 1;
		items_count++;
	}
	bool value_changed = Combo(label, current_item, Items_SingleStringGetter, (void*)items_separated_by_zeros, items_count, height_in_items);
	return value_changed;
}

// Tip: pass an empty label (e.g. "##dummy") then you can use the space to draw other text or image.
// But you need to make sure the ID is unique, e.g. enclose calls in PushID/PopID.
bool ImGui::Selectable(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.ColumnsSet) // FIXME-OPT: Avoid if vertically clipped.
		PopClipRect();

	ImGuiID id = window->GetID(label);
	ImVec2 label_size = CalcTextSize(label, NULL, true);
	ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
	ImVec2 pos = window->DC.CursorPos;
	pos.y += window->DC.CurrentLineTextBaseOffset;
	ImRect bb(pos, pos + size);
	ItemSize(bb);

	// Fill horizontal space.
	ImVec2 window_padding = window->WindowPadding;
	float max_x = (flags & ImGuiSelectableFlags_SpanAllColumns) ? GetWindowContentRegionMax().x : GetContentRegionMax().x;
	float w_draw = ImMax(label_size.x, window->Pos.x + max_x - window_padding.x - window->DC.CursorPos.x);
	ImVec2 size_draw((size_arg.x != 0 && !(flags & ImGuiSelectableFlags_DrawFillAvailWidth)) ? size_arg.x : w_draw, size_arg.y != 0.0f ? size_arg.y : size.y);
	ImRect bb_with_spacing(pos, pos + size_draw);
	if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_DrawFillAvailWidth))
		bb_with_spacing.Max.x += window_padding.x;

	// Selectables are tightly packed together, we extend the box to cover spacing between selectable.
	float spacing_L = (float)(int)(style.ItemSpacing.x * 0.5f);
	float spacing_U = (float)(int)(style.ItemSpacing.y * 0.5f);
	float spacing_R = style.ItemSpacing.x - spacing_L;
	float spacing_D = style.ItemSpacing.y - spacing_U;
	bb_with_spacing.Min.x -= spacing_L;
	bb_with_spacing.Min.y -= spacing_U;
	bb_with_spacing.Max.x += spacing_R;
	bb_with_spacing.Max.y += spacing_D;
	if (!ItemAdd(bb_with_spacing, id))
	{
		if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.ColumnsSet)
			PushColumnClipRect();
		return false;
	}

	ImGuiButtonFlags button_flags = 0;
	if (flags & ImGuiSelectableFlags_Menu) button_flags |= ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_NoHoldingActiveID;
	if (flags & ImGuiSelectableFlags_MenuItem) button_flags |= ImGuiButtonFlags_PressedOnRelease;
	if (flags & ImGuiSelectableFlags_Disabled) button_flags |= ImGuiButtonFlags_Disabled;
	if (flags & ImGuiSelectableFlags_AllowDoubleClick) button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb_with_spacing, id, &hovered, &held, button_flags);
	if (flags & ImGuiSelectableFlags_Disabled)
		selected = false;

	// Render
	if (hovered || selected)
	{
		const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
		RenderFrame(bb_with_spacing.Min, bb_with_spacing.Max, col, false, 0.0f);
	}

	if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.ColumnsSet)
	{
		PushColumnClipRect();
		bb_with_spacing.Max.x -= (GetContentRegionMax().x - max_x);
	}

	if (flags & ImGuiSelectableFlags_Disabled) PushStyleColor(ImGuiCol_Text, g.Style.Colors[ImGuiCol_TextDisabled]);
	RenderTextClipped(bb.Min, bb_with_spacing.Max, label, NULL, &label_size, ImVec2(0.0f, 0.0f));
	if (flags & ImGuiSelectableFlags_Disabled) PopStyleColor();

	// Automatically close popups
	if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(window->DC.ItemFlags & ImGuiItemFlags_SelectableDontClosePopup))
		CloseCurrentPopup();
	return pressed;
}

bool ImGui::Selectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
	if (Selectable(label, *p_selected, flags, size_arg))
	{
		*p_selected = !*p_selected;
		return true;
	}
	return false;
}

// Helper to calculate the size of a listbox and display a label on the right.
// Tip: To have a list filling the entire window width, PushItemWidth(-1) and pass an empty label "##empty"
bool ImGui::ListBoxHeader(const char* label, const ImVec2& size_arg)//eskere
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	const ImGuiStyle& style = GetStyle();
	const ImGuiID id = GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	// Size default to hold ~7 items. Fractional number of items helps seeing that we can scroll down/up without looking at scrollbar.
	ImVec2 size = CalcItemSize(size_arg, CalcItemWidth(), GetTextLineHeightWithSpacing() * 7.4f + style.ItemSpacing.y);
	ImVec2 frame_size = ImVec2(size.x, ImMax(size.y, label_size.y));
	ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
	ImRect bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
	window->DC.LastItemRect = bb;

	BeginGroup();
	if (label_size.x > 0)
		RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

	BeginChildFrame(id, frame_bb.GetSize());
	return true;
}

bool ImGui::ListBoxHeader(const char* label, int items_count, int height_in_items)
{
	// Size default to hold ~7 items. Fractional number of items helps seeing that we can scroll down/up without looking at scrollbar.
	// However we don't add +0.40f if items_count <= height_in_items. It is slightly dodgy, because it means a dynamic list of items will make the widget resize occasionally when it crosses that size.
	// I am expecting that someone will come and complain about this behavior in a remote future, then we can advise on a better solution.
	if (height_in_items < 0)
		height_in_items = ImMin(items_count, 7);
	float height_in_items_f = height_in_items < items_count ? (height_in_items + 0.40f) : (height_in_items + 0.00f);

	// We include ItemSpacing.y so that a list sized for the exact number of items doesn't make a scrollbar appears. We could also enforce that by passing a flag to BeginChild().
	ImVec2 size;
	size.x = 0.0f;
	size.y = GetTextLineHeightWithSpacing() * height_in_items_f + GetStyle().ItemSpacing.y;
	return ListBoxHeader(label, size);
}

void ImGui::ListBoxFooter()
{
	ImGuiWindow* parent_window = GetCurrentWindow()->ParentWindow;
	const ImRect bb = parent_window->DC.LastItemRect;
	const ImGuiStyle& style = GetStyle();

	EndChildFrame();

	// Redeclare item size so that it includes the label (we have stored the full size in LastItemRect)
	// We call SameLine() to restore DC.CurrentLine* data
	SameLine();
	parent_window->DC.CursorPos = bb.Min;
	ItemSize(bb, style.FramePadding.y);
	EndGroup();
}

bool ImGui::ListBox(const char* label, int* current_item, const char* const items[], int items_count, int height_items)
{
	const bool value_changed = ListBox(label, current_item, Items_ArrayGetter, (void*)items, items_count, height_items);
	return value_changed;
}

bool ImGui::ListBox(const char* label, int* current_item, bool(*items_getter)(void*, int, const char**), void* data, int items_count, int height_in_items)
{
	if (!ListBoxHeader(label, items_count, height_in_items))
		return false;

	// Assume all items have even height (= 1 line of text). If you need items of different or variable sizes you can create a custom version of ListBox() in your code without using the clipper.
	bool value_changed = false;
	ImGuiListClipper clipper(items_count, GetTextLineHeightWithSpacing()); // We know exactly our line height here so we pass it as a minor optimization, but generally you don't need to.
	while (clipper.Step())
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			const bool item_selected = (i == *current_item);
			const char* item_text;
			if (!items_getter(data, i, &item_text))
				item_text = "*Unknown item*";

			PushID(i);
			if (Selectable(item_text, item_selected))
			{
				*current_item = i;
				value_changed = true;
			}
			PopID();
		}
	ListBoxFooter();
	return value_changed;
}

bool ImGui::MenuItem(const char* label, const char* shortcut, bool selected, bool enabled)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGuiStyle& style = g.Style;
	ImVec2 pos = window->DC.CursorPos;
	ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImGuiSelectableFlags flags = ImGuiSelectableFlags_MenuItem | (enabled ? 0 : ImGuiSelectableFlags_Disabled);
	bool pressed;
	if (window->DC.LayoutType == ImGuiLayoutType_Horizontal)
	{
		// Mimic the exact layout spacing of BeginMenu() to allow MenuItem() inside a menu bar, which is a little misleading but may be useful
		// Note that in this situation we render neither the shortcut neither the selected tick mark
		float w = label_size.x;
		window->DC.CursorPos.x += (float)(int)(style.ItemSpacing.x * 0.5f);
		PushStyleVar(ImGuiStyleVar_ItemSpacing, style.ItemSpacing * 2.0f);
		pressed = Selectable(label, false, flags, ImVec2(w, 0.0f));
		PopStyleVar();
		window->DC.CursorPos.x += (float)(int)(style.ItemSpacing.x * (-1.0f + 0.5f)); // -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would also work to call SameLine() ourselves after the PopStyleVar().
	}
	else
	{
		ImVec2 shortcut_size = shortcut ? CalcTextSize(shortcut, NULL) : ImVec2(0.0f, 0.0f);
		float w = window->MenuColumns.DeclColumns(label_size.x, shortcut_size.x, (float)(int)(g.FontSize * 1.20f)); // Feedback for next frame
		float extra_w = ImMax(0.0f, GetContentRegionAvail().x - w);
		pressed = Selectable(label, false, flags | ImGuiSelectableFlags_DrawFillAvailWidth, ImVec2(w, 0.0f));
		if (shortcut_size.x > 0.0f)
		{
			PushStyleColor(ImGuiCol_Text, g.Style.Colors[ImGuiCol_TextDisabled]);
			RenderText(pos + ImVec2(window->MenuColumns.Pos[1] + extra_w, 0.0f), shortcut, NULL, false);
			PopStyleColor();
		}
		if (selected)
			RenderCheckMark(pos + ImVec2(window->MenuColumns.Pos[2] + extra_w + g.FontSize * 0.40f, g.FontSize * 0.134f * 0.5f), GetColorU32(enabled ? ImGuiCol_Text : ImGuiCol_TextDisabled), g.FontSize  * 0.866f);
	}
	return pressed;
}

bool ImGui::MenuItem(const char* label, const char* shortcut, bool* p_selected, bool enabled)
{
	if (MenuItem(label, shortcut, p_selected ? *p_selected : false, enabled))
	{
		if (p_selected)
			*p_selected = !*p_selected;
		return true;
	}
	return false;
}

bool ImGui::BeginMainMenuBar()
{
	ImGuiContext& g = *GImGui;
	SetNextWindowPos(ImVec2(0.0f, 0.0f));
	SetNextWindowSize(ImVec2(g.IO.DisplaySize.x, g.FontBaseSize + g.Style.FramePadding.y * 2.0f));
	PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
	if (!Begin("##MainMenuBar", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar)
		|| !BeginMenuBar())
	{
		End();
		PopStyleVar(2);
		return false;
	}
	g.CurrentWindow->DC.MenuBarOffsetX += g.Style.DisplaySafeAreaPadding.x;
	return true;
}

void ImGui::EndMainMenuBar()
{
	EndMenuBar();
	End();
	PopStyleVar(2);
}

bool ImGui::BeginMenuBar()
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;
	if (!(window->Flags & ImGuiWindowFlags_MenuBar))
		return false;

	IM_ASSERT(!window->DC.MenuBarAppending);
	BeginGroup(); // Save position
	PushID("##menubar");

	// We don't clip with regular window clipping rectangle as it is already set to the area below. However we clip with window full rect.
	// We remove 1 worth of rounding to Max.x to that text in long menus don't tend to display over the lower-right rounded area, which looks particularly glitchy.
	ImRect bar_rect = window->MenuBarRect();
	ImRect clip_rect(ImFloor(bar_rect.Min.x + 0.5f), ImFloor(bar_rect.Min.y + window->WindowBorderSize + 0.5f), ImFloor(ImMax(bar_rect.Min.x, bar_rect.Max.x - window->WindowRounding) + 0.5f), ImFloor(bar_rect.Max.y + 0.5f));
	clip_rect.ClipWith(window->WindowRectClipped);
	PushClipRect(clip_rect.Min, clip_rect.Max, false);

	window->DC.CursorPos = ImVec2(bar_rect.Min.x + window->DC.MenuBarOffsetX, bar_rect.Min.y);// + g.Style.FramePadding.y);
	window->DC.LayoutType = ImGuiLayoutType_Horizontal;
	window->DC.MenuBarAppending = true;
	AlignTextToFramePadding();
	return true;
}

void ImGui::EndMenuBar()
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	IM_ASSERT(window->Flags & ImGuiWindowFlags_MenuBar);
	IM_ASSERT(window->DC.MenuBarAppending);
	PopClipRect();
	PopID();
	window->DC.MenuBarOffsetX = window->DC.CursorPos.x - window->MenuBarRect().Min.x;
	window->DC.GroupStack.back().AdvanceCursor = false;
	EndGroup();
	window->DC.LayoutType = ImGuiLayoutType_Vertical;
	window->DC.MenuBarAppending = false;
}

bool ImGui::BeginMenu(const char* label, bool enabled)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);

	ImVec2 label_size = CalcTextSize(label, NULL, true);

	bool pressed;
	bool menu_is_open = IsPopupOpen(id);
	bool menuset_is_open = !(window->Flags & ImGuiWindowFlags_Popup) && (g.OpenPopupStack.Size > g.CurrentPopupStack.Size && g.OpenPopupStack[g.CurrentPopupStack.Size].OpenParentId == window->IDStack.back());
	ImGuiWindow* backed_nav_window = g.NavWindow;
	if (menuset_is_open)
		g.NavWindow = window;  // Odd hack to allow hovering across menus of a same menu-set (otherwise we wouldn't be able to hover parent)

							   // The reference position stored in popup_pos will be used by Begin() to find a suitable position for the child menu (using FindBestPopupWindowPos).
	ImVec2 popup_pos, pos = window->DC.CursorPos;
	if (window->DC.LayoutType == ImGuiLayoutType_Horizontal)
	{
		// Menu inside an horizontal menu bar
		// Selectable extend their highlight by half ItemSpacing in each direction.
		// For ChildMenu, the popup position will be overwritten by the call to FindBestPopupWindowPos() in Begin()
		popup_pos = ImVec2(pos.x - window->WindowPadding.x, pos.y - style.FramePadding.y + window->MenuBarHeight());
		window->DC.CursorPos.x += (float)(int)(style.ItemSpacing.x * 0.5f);
		PushStyleVar(ImGuiStyleVar_ItemSpacing, style.ItemSpacing * 2.0f);
		float w = label_size.x;
		pressed = Selectable(label, menu_is_open, ImGuiSelectableFlags_Menu | ImGuiSelectableFlags_DontClosePopups | (!enabled ? ImGuiSelectableFlags_Disabled : 0), ImVec2(w, 0.0f));
		PopStyleVar();
		window->DC.CursorPos.x += (float)(int)(style.ItemSpacing.x * (-1.0f + 0.5f)); // -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would also work to call SameLine() ourselves after the PopStyleVar().
	}
	else
	{
		// Menu inside a menu
		popup_pos = ImVec2(pos.x, pos.y - style.WindowPadding.y);
		float w = window->MenuColumns.DeclColumns(label_size.x, 0.0f, (float)(int)(g.FontSize * 1.20f)); // Feedback to next frame
		float extra_w = ImMax(0.0f, GetContentRegionAvail().x - w);
		pressed = Selectable(label, menu_is_open, ImGuiSelectableFlags_Menu | ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_DrawFillAvailWidth | (!enabled ? ImGuiSelectableFlags_Disabled : 0), ImVec2(w, 0.0f));
		if (!enabled) PushStyleColor(ImGuiCol_Text, g.Style.Colors[ImGuiCol_TextDisabled]);
		RenderTriangle(pos + ImVec2(window->MenuColumns.Pos[2] + extra_w + g.FontSize * 0.30f, 0.0f), ImGuiDir_Right);
		if (!enabled) PopStyleColor();
	}

	const bool hovered = enabled && ItemHoverable(window->DC.LastItemRect, id);
	if (menuset_is_open)
		g.NavWindow = backed_nav_window;

	bool want_open = false, want_close = false;
	if (window->DC.LayoutType != ImGuiLayoutType_Horizontal) // (window->Flags & (ImGuiWindowFlags_Popup|ImGuiWindowFlags_ChildMenu))
	{
		// Implement http://bjk5.com/post/44698559168/breaking-down-amazons-mega-dropdown to avoid using timers, so menus feels more reactive.
		bool moving_within_opened_triangle = false;
		if (g.HoveredWindow == window && g.OpenPopupStack.Size > g.CurrentPopupStack.Size && g.OpenPopupStack[g.CurrentPopupStack.Size].ParentWindow == window)
		{
			if (ImGuiWindow* next_window = g.OpenPopupStack[g.CurrentPopupStack.Size].Window)
			{
				ImRect next_window_rect = next_window->Rect();
				ImVec2 ta = g.IO.MousePos - g.IO.MouseDelta;
				ImVec2 tb = (window->Pos.x < next_window->Pos.x) ? next_window_rect.GetTL() : next_window_rect.GetTR();
				ImVec2 tc = (window->Pos.x < next_window->Pos.x) ? next_window_rect.GetBL() : next_window_rect.GetBR();
				float extra = ImClamp(fabsf(ta.x - tb.x) * 0.30f, 5.0f, 30.0f); // add a bit of extra slack.
				ta.x += (window->Pos.x < next_window->Pos.x) ? -0.5f : +0.5f;   // to avoid numerical issues
				tb.y = ta.y + ImMax((tb.y - extra) - ta.y, -100.0f);            // triangle is maximum 200 high to limit the slope and the bias toward large sub-menus // FIXME: Multiply by fb_scale?
				tc.y = ta.y + ImMin((tc.y + extra) - ta.y, +100.0f);
				moving_within_opened_triangle = ImTriangleContainsPoint(ta, tb, tc, g.IO.MousePos);
				//window->DrawList->PushClipRectFullScreen(); window->DrawList->AddTriangleFilled(ta, tb, tc, moving_within_opened_triangle ? IM_COL32(0,128,0,128) : IM_COL32(128,0,0,128)); window->DrawList->PopClipRect(); // Debug
			}
		}

		want_close = (menu_is_open && !hovered && g.HoveredWindow == window && g.HoveredIdPreviousFrame != 0 && g.HoveredIdPreviousFrame != id && !moving_within_opened_triangle);
		want_open = (!menu_is_open && hovered && !moving_within_opened_triangle) || (!menu_is_open && hovered && pressed);
	}
	else
	{
		// Menu bar
		if (menu_is_open && pressed && menuset_is_open) // Click an open menu again to close it
		{
			want_close = true;
			want_open = menu_is_open = false;
		}
		else if (pressed || (hovered && menuset_is_open && !menu_is_open)) // First click to open, then hover to open others
		{
			want_open = true;
		}
	}

	if (!enabled) // explicitly close if an open menu becomes disabled, facilitate users code a lot in pattern such as 'if (BeginMenu("options", has_object)) { ..use object.. }'
		want_close = true;
	if (want_close && IsPopupOpen(id))
		ClosePopupToLevel(GImGui->CurrentPopupStack.Size);

	if (!menu_is_open && want_open && g.OpenPopupStack.Size > g.CurrentPopupStack.Size)
	{
		// Don't recycle same menu level in the same frame, first close the other menu and yield for a frame.
		OpenPopup(label);
		return false;
	}

	menu_is_open |= want_open;
	if (want_open)
		OpenPopup(label);

	if (menu_is_open)
	{
		SetNextWindowPos(popup_pos, ImGuiCond_Always);
		ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ((window->Flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_ChildMenu)) ? ImGuiWindowFlags_ChildMenu | ImGuiWindowFlags_ChildWindow : ImGuiWindowFlags_ChildMenu);
		menu_is_open = BeginPopupEx(id, flags); // menu_is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
	}

	return menu_is_open;
}

void ImGui::EndMenu()
{
	EndPopup();
}

// Note: only access 3 floats if ImGuiColorEditFlags_NoAlpha flag is set.
void ImGui::ColorTooltip(const char* text, const float* col, ImGuiColorEditFlags flags)
{
	ImGuiContext& g = *GImGui;

	int cr = IM_F32_TO_INT8_SAT(col[0]), cg = IM_F32_TO_INT8_SAT(col[1]), cb = IM_F32_TO_INT8_SAT(col[2]), ca = (flags & ImGuiColorEditFlags_NoAlpha) ? 255 : IM_F32_TO_INT8_SAT(col[3]);
	BeginTooltipEx(0, true);

	const char* text_end = text ? FindRenderedTextEnd(text, NULL) : text;
	if (text_end > text)
	{
		TextUnformatted(text, text_end);
		Separator();
	}

	ImVec2 sz(g.FontSize * 3 + g.Style.FramePadding.y * 2, g.FontSize * 3 + g.Style.FramePadding.y * 2);
	ColorButton("##preview", ImVec4(col[0], col[1], col[2], col[3]), (flags & (ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf)) | ImGuiColorEditFlags_NoTooltip, sz);
	SameLine();
	if (flags & ImGuiColorEditFlags_NoAlpha)
		Text("#%02X%02X%02X\nR: %d, G: %d, B: %d\n(%.3f, %.3f, %.3f)", cr, cg, cb, cr, cg, cb, col[0], col[1], col[2]);
	else
		Text("#%02X%02X%02X%02X\nR:%d, G:%d, B:%d, A:%d\n(%.3f, %.3f, %.3f, %.3f)", cr, cg, cb, ca, cr, cg, cb, ca, col[0], col[1], col[2], col[3]);
	EndTooltip();
}

static inline ImU32 ImAlphaBlendColor(ImU32 col_a, ImU32 col_b)
{
	float t = ((col_b >> IM_COL32_A_SHIFT) & 0xFF) / 255.f;
	int r = ImLerp((int)(col_a >> IM_COL32_R_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_R_SHIFT) & 0xFF, t);
	int g = ImLerp((int)(col_a >> IM_COL32_G_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_G_SHIFT) & 0xFF, t);
	int b = ImLerp((int)(col_a >> IM_COL32_B_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_B_SHIFT) & 0xFF, t);
	return IM_COL32(r, g, b, 0xFF);
}

// NB: This is rather brittle and will show artifact when rounding this enabled if rounded corners overlap multiple cells. Caller currently responsible for avoiding that.
// I spent a non reasonable amount of time trying to getting this right for ColorButton with rounding+anti-aliasing+ImGuiColorEditFlags_HalfAlphaPreview flag + various grid sizes and offsets, and eventually gave up... probably more reasonable to disable rounding alltogether.
void ImGui::RenderColorRectWithAlphaCheckerboard(ImVec2 p_min, ImVec2 p_max, ImU32 col, float grid_step, ImVec2 grid_off, float rounding, int rounding_corners_flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (((col & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT) < 0xFF)
	{
		ImU32 col_bg1 = GetColorU32(ImAlphaBlendColor(IM_COL32(204, 204, 204, 255), col));
		ImU32 col_bg2 = GetColorU32(ImAlphaBlendColor(IM_COL32(128, 128, 128, 255), col));
		window->DrawList->AddRectFilled(p_min, p_max, col_bg1, rounding, rounding_corners_flags);

		int yi = 0;
		for (float y = p_min.y + grid_off.y; y < p_max.y; y += grid_step, yi++)
		{
			float y1 = ImClamp(y, p_min.y, p_max.y), y2 = ImMin(y + grid_step, p_max.y);
			if (y2 <= y1)
				continue;
			for (float x = p_min.x + grid_off.x + (yi & 1) * grid_step; x < p_max.x; x += grid_step * 2.0f)
			{
				float x1 = ImClamp(x, p_min.x, p_max.x), x2 = ImMin(x + grid_step, p_max.x);
				if (x2 <= x1)
					continue;
				int rounding_corners_flags_cell = 0;
				if (y1 <= p_min.y) { if (x1 <= p_min.x) rounding_corners_flags_cell |= ImDrawCornerFlags_TopLeft; if (x2 >= p_max.x) rounding_corners_flags_cell |= ImDrawCornerFlags_TopRight; }
				if (y2 >= p_max.y) { if (x1 <= p_min.x) rounding_corners_flags_cell |= ImDrawCornerFlags_BotLeft; if (x2 >= p_max.x) rounding_corners_flags_cell |= ImDrawCornerFlags_BotRight; }
				rounding_corners_flags_cell &= rounding_corners_flags;
				window->DrawList->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), col_bg2, rounding_corners_flags_cell ? rounding : 0.0f, rounding_corners_flags_cell);
			}
		}
	}
	else
	{
		window->DrawList->AddRectFilled(p_min, p_max, col, rounding, rounding_corners_flags);
	}
}

void ImGui::SetColorEditOptions(ImGuiColorEditFlags flags)
{
	ImGuiContext& g = *GImGui;
	if ((flags & ImGuiColorEditFlags__InputsMask) == 0)
		flags |= ImGuiColorEditFlags__OptionsDefault & ImGuiColorEditFlags__InputsMask;
	if ((flags & ImGuiColorEditFlags__DataTypeMask) == 0)
		flags |= ImGuiColorEditFlags__OptionsDefault & ImGuiColorEditFlags__DataTypeMask;
	if ((flags & ImGuiColorEditFlags__PickerMask) == 0)
		flags |= ImGuiColorEditFlags__OptionsDefault & ImGuiColorEditFlags__PickerMask;
	IM_ASSERT(ImIsPowerOfTwo((int)(flags & ImGuiColorEditFlags__InputsMask)));   // Check only 1 option is selected
	IM_ASSERT(ImIsPowerOfTwo((int)(flags & ImGuiColorEditFlags__DataTypeMask))); // Check only 1 option is selected
	IM_ASSERT(ImIsPowerOfTwo((int)(flags & ImGuiColorEditFlags__PickerMask)));   // Check only 1 option is selected
	g.ColorEditOptions = flags;
}

// A little colored square. Return true when clicked.
// FIXME: May want to display/ignore the alpha component in the color display? Yet show it in the tooltip.
// 'desc_id' is not called 'label' because we don't display it next to the button, but only in the tooltip.
bool ImGui::ColorButton(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags, ImVec2 size)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiID id = window->GetID(desc_id);
	float default_size = GetFrameHeight();
	if (size.x == 0.0f)
		size.x = default_size;
	if (size.y == 0.0f)
		size.y = default_size;
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	ItemSize(bb, (size.y >= default_size) ? g.Style.FramePadding.y : 0.0f);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);

	if (flags & ImGuiColorEditFlags_NoAlpha)
		flags &= ~(ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf);

	ImVec4 col_without_alpha(col.x, col.y, col.z, 1.0f);
	float grid_step = ImMin(size.x, size.y) / 2.99f;
	float rounding = ImMin(g.Style.FrameRounding, grid_step * 0.5f);
	ImRect bb_inner = bb;
	float off = -0.75f; // The border (using Col_FrameBg) tends to look off when color is near-opaque and rounding is enabled. This offset seemed like a good middle ground to reduce those artifacts.
	bb_inner.Expand(off);
	if ((flags & ImGuiColorEditFlags_AlphaPreviewHalf) && col.w < 1.0f)
	{
		float mid_x = (float)(int)((bb_inner.Min.x + bb_inner.Max.x) * 0.5f + 0.5f);
		RenderColorRectWithAlphaCheckerboard(ImVec2(bb_inner.Min.x + grid_step, bb_inner.Min.y), bb_inner.Max, GetColorU32(col), grid_step, ImVec2(-grid_step + off, off), rounding, ImDrawCornerFlags_TopRight | ImDrawCornerFlags_BotRight);
		window->DrawList->AddRectFilled(bb_inner.Min, ImVec2(mid_x, bb_inner.Max.y), GetColorU32(col_without_alpha), rounding, ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_BotLeft);
	}
	else
	{
		// Because GetColorU32() multiplies by the global style Alpha and we don't want to display a checkerboard if the source code had no alpha
		ImVec4 col_source = (flags & ImGuiColorEditFlags_AlphaPreview) ? col : col_without_alpha;
		if (col_source.w < 1.0f)
			RenderColorRectWithAlphaCheckerboard(bb_inner.Min, bb_inner.Max, GetColorU32(col_source), grid_step, ImVec2(off, off), rounding);
		else
			window->DrawList->AddRectFilled(bb_inner.Min, bb_inner.Max, GetColorU32(col_source), rounding, ImDrawCornerFlags_All);
	}
	if (g.Style.FrameBorderSize > 0.0f)
		RenderFrameBorder(bb.Min, bb.Max, rounding);
	else
		window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg - 10), rounding); // Color button are often in need of some sort of border

																							// Drag and Drop Source
	if (g.ActiveId == id && BeginDragDropSource()) // NB: The ActiveId test is merely an optional micro-optimization
	{
		if (flags & ImGuiColorEditFlags_NoAlpha)
			SetDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F, &col, sizeof(float) * 3, ImGuiCond_Once);
		else
			SetDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F, &col, sizeof(float) * 4, ImGuiCond_Once);
		ColorButton(desc_id, col, flags);
		SameLine();
		TextUnformatted("Color");
		EndDragDropSource();
		hovered = false;
	}

	// Tooltip
	if (!(flags & ImGuiColorEditFlags_NoTooltip) && hovered)
		ColorTooltip(desc_id, &col.x, flags & (ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf));

	return pressed;
}

bool ImGui::ColorEdit3(const char* label, float col[3], ImGuiColorEditFlags flags)
{
	return ColorEdit4(label, col, flags | ImGuiColorEditFlags_NoAlpha);
}

void ImGui::ColorEditOptionsPopup(const float* col, ImGuiColorEditFlags flags)
{
	bool allow_opt_inputs = !(flags & ImGuiColorEditFlags__InputsMask);
	bool allow_opt_datatype = !(flags & ImGuiColorEditFlags__DataTypeMask);
	if ((!allow_opt_inputs && !allow_opt_datatype) || !BeginPopup("context"))
		return;
	ImGuiContext& g = *GImGui;
	ImGuiColorEditFlags opts = g.ColorEditOptions;
	if (allow_opt_inputs)
	{
		if (RadioButton("RGB", (opts & ImGuiColorEditFlags_RGB) ? 1 : 0)) opts = (opts & ~ImGuiColorEditFlags__InputsMask) | ImGuiColorEditFlags_RGB;
		if (RadioButton("HSV", (opts & ImGuiColorEditFlags_HSV) ? 1 : 0)) opts = (opts & ~ImGuiColorEditFlags__InputsMask) | ImGuiColorEditFlags_HSV;
		if (RadioButton("HEX", (opts & ImGuiColorEditFlags_HEX) ? 1 : 0)) opts = (opts & ~ImGuiColorEditFlags__InputsMask) | ImGuiColorEditFlags_HEX;
	}
	if (allow_opt_datatype)
	{
		if (allow_opt_inputs) Separator();
		if (RadioButton("0..255", (opts & ImGuiColorEditFlags_Uint8) ? 1 : 0)) opts = (opts & ~ImGuiColorEditFlags__DataTypeMask) | ImGuiColorEditFlags_Uint8;
		if (RadioButton("0.00..1.00", (opts & ImGuiColorEditFlags_Float) ? 1 : 0)) opts = (opts & ~ImGuiColorEditFlags__DataTypeMask) | ImGuiColorEditFlags_Float;
	}

	if (allow_opt_inputs || allow_opt_datatype)
		Separator();
	if (Button("Copy as..", ImVec2(-1, 0)))
		OpenPopup("Copy");
	if (BeginPopup("Copy"))
	{
		int cr = IM_F32_TO_INT8_SAT(col[0]), cg = IM_F32_TO_INT8_SAT(col[1]), cb = IM_F32_TO_INT8_SAT(col[2]), ca = (flags & ImGuiColorEditFlags_NoAlpha) ? 255 : IM_F32_TO_INT8_SAT(col[3]);
		char buf[64];
		ImFormatString(buf, IM_ARRAYSIZE(buf), "(%.3ff, %.3ff, %.3ff, %.3ff)", col[0], col[1], col[2], (flags & ImGuiColorEditFlags_NoAlpha) ? 1.0f : col[3]);
		if (Selectable(buf))
			SetClipboardText(buf);
		ImFormatString(buf, IM_ARRAYSIZE(buf), "(%d,%d,%d,%d)", cr, cg, cb, ca);
		if (Selectable(buf))
			SetClipboardText(buf);
		if (flags & ImGuiColorEditFlags_NoAlpha)
			ImFormatString(buf, IM_ARRAYSIZE(buf), "0x%02X%02X%02X", cr, cg, cb);
		else
			ImFormatString(buf, IM_ARRAYSIZE(buf), "0x%02X%02X%02X%02X", cr, cg, cb, ca);
		if (Selectable(buf))
			SetClipboardText(buf);
		EndPopup();
	}

	g.ColorEditOptions = opts;
	EndPopup();
}

static void ColorPickerOptionsPopup(ImGuiColorEditFlags flags, const float* ref_col)
{
	bool allow_opt_picker = !(flags & ImGuiColorEditFlags__PickerMask);
	bool allow_opt_alpha_bar = !(flags & ImGuiColorEditFlags_NoAlpha) && !(flags & ImGuiColorEditFlags_AlphaBar);
	if ((!allow_opt_picker && !allow_opt_alpha_bar) || !ImGui::BeginPopup("context"))
		return;
	ImGuiContext& g = *GImGui;
	if (allow_opt_picker)
	{
		ImVec2 picker_size(g.FontSize * 8, ImMax(g.FontSize * 8 - (ImGui::GetFrameHeight() + g.Style.ItemInnerSpacing.x), 1.0f)); // FIXME: Picker size copied from main picker function
		ImGui::PushItemWidth(picker_size.x);
		for (int picker_type = 0; picker_type < 2; picker_type++)
		{
			// Draw small/thumbnail version of each picker type (over an invisible button for selection)
			if (picker_type > 0) ImGui::Separator();
			ImGui::PushID(picker_type);
			ImGuiColorEditFlags picker_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | (flags & ImGuiColorEditFlags_NoAlpha);
			if (picker_type == 0) picker_flags |= ImGuiColorEditFlags_PickerHueBar;
			if (picker_type == 1) picker_flags |= ImGuiColorEditFlags_PickerHueWheel;
			ImVec2 backup_pos = ImGui::GetCursorScreenPos();
			if (ImGui::Selectable("##selectable", false, 0, picker_size)) // By default, Selectable() is closing popup
				g.ColorEditOptions = (g.ColorEditOptions & ~ImGuiColorEditFlags__PickerMask) | (picker_flags & ImGuiColorEditFlags__PickerMask);
			ImGui::SetCursorScreenPos(backup_pos);
			ImVec4 dummy_ref_col;
			memcpy(&dummy_ref_col.x, ref_col, sizeof(float) * (picker_flags & ImGuiColorEditFlags_NoAlpha ? 3 : 4));
			ImGui::ColorPicker4("##dummypicker", &dummy_ref_col.x, picker_flags);
			ImGui::PopID();
		}
		ImGui::PopItemWidth();
	}
	if (allow_opt_alpha_bar)
	{
		if (allow_opt_picker) ImGui::Separator();
		ImGui::CheckboxFlags("Alpha Bar", (unsigned int*)&g.ColorEditOptions, ImGuiColorEditFlags_AlphaBar);
	}
	ImGui::EndPopup();
}

// Edit colors components (each component in 0.0f..1.0f range). 
// See enum ImGuiColorEditFlags_ for available options. e.g. Only access 3 floats if ImGuiColorEditFlags_NoAlpha flag is set.
// With typical options: Left-click on colored square to open color picker. Right-click to open option menu. CTRL-Click over input fields to edit them and TAB to go to next item.
bool ImGui::ColorEdit4(const char* label, float col[4], ImGuiColorEditFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const float square_sz = GetFrameHeight();
	const float w_extra = (flags & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (square_sz + style.ItemInnerSpacing.x);
	const float w_items_all = CalcItemWidth() - w_extra;
	const char* label_display_end = FindRenderedTextEnd(label);

	const bool alpha = (flags & ImGuiColorEditFlags_NoAlpha) == 0;
	const bool hdr = (flags & ImGuiColorEditFlags_HDR) != 0;
	const int components = alpha ? 4 : 3;
	const ImGuiColorEditFlags flags_untouched = flags;

	BeginGroup();
	PushID(label);

	// If we're not showing any slider there's no point in doing any HSV conversions
	if (flags & ImGuiColorEditFlags_NoInputs)
		flags = (flags & (~ImGuiColorEditFlags__InputsMask)) | ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_NoOptions;

	// Context menu: display and modify options (before defaults are applied)
	if (!(flags & ImGuiColorEditFlags_NoOptions))
		ColorEditOptionsPopup(col, flags);

	// Read stored options
	if (!(flags & ImGuiColorEditFlags__InputsMask))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags__InputsMask);
	if (!(flags & ImGuiColorEditFlags__DataTypeMask))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags__DataTypeMask);
	if (!(flags & ImGuiColorEditFlags__PickerMask))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags__PickerMask);
	flags |= (g.ColorEditOptions & ~(ImGuiColorEditFlags__InputsMask | ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__PickerMask));

	// Convert to the formats we need
	float f[4] = { col[0], col[1], col[2], alpha ? col[3] : 1.0f };
	if (flags & ImGuiColorEditFlags_HSV)
		ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
	int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };

	bool value_changed = false;
	bool value_changed_as_float = false;

	if ((flags & (ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_HSV)) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
	{
		// RGB/HSV 0..255 Sliders
		const float w_item_one = ImMax(1.0f, (float)(int)((w_items_all - (style.ItemInnerSpacing.x) * (components - 1)) / (float)components));
		const float w_item_last = ImMax(1.0f, (float)(int)(w_items_all - (w_item_one + style.ItemInnerSpacing.x) * (components - 1)));

		const bool hide_prefix = (w_item_one <= CalcTextSize((flags & ImGuiColorEditFlags_Float) ? "M:0.000" : "M:000").x);
		const char* ids[4] = { "##X", "##Y", "##Z", "##W" };
		const char* fmt_table_int[3][4] =
		{
			{ "%3.0f",   "%3.0f",   "%3.0f",   "%3.0f" }, // Short display
			{ "R:%3.0f", "G:%3.0f", "B:%3.0f", "A:%3.0f" }, // Long display for RGBA
			{ "H:%3.0f", "S:%3.0f", "V:%3.0f", "A:%3.0f" }  // Long display for HSVA
		};
		const char* fmt_table_float[3][4] =
		{
			{ "%0.3f",   "%0.3f",   "%0.3f",   "%0.3f" }, // Short display
			{ "R:%0.3f", "G:%0.3f", "B:%0.3f", "A:%0.3f" }, // Long display for RGBA
			{ "H:%0.3f", "S:%0.3f", "V:%0.3f", "A:%0.3f" }  // Long display for HSVA
		};
		const int fmt_idx = hide_prefix ? 0 : (flags & ImGuiColorEditFlags_HSV) ? 2 : 1;

		PushItemWidth(w_item_one);
		for (int n = 0; n < components; n++)
		{
			if (n > 0)
				SameLine(0, style.ItemInnerSpacing.x);
			if (n + 1 == components)
				PushItemWidth(w_item_last);
			if (flags & ImGuiColorEditFlags_Float)
				value_changed = value_changed_as_float = value_changed | DragFloat(ids[n], &f[n], 1.0f / 255.0f, 0.0f, hdr ? 0.0f : 1.0f, fmt_table_float[fmt_idx][n]);
			else
				value_changed |= DragInt(ids[n], &i[n], 1.0f, 0, hdr ? 0 : 255, fmt_table_int[fmt_idx][n]);
			if (!(flags & ImGuiColorEditFlags_NoOptions))
				OpenPopupOnItemClick("context");
		}
		PopItemWidth();
		PopItemWidth();
	}
	else if ((flags & ImGuiColorEditFlags_HEX) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
	{
		// RGB Hexadecimal Input
		char buf[64];
		if (alpha)
			ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X%02X", ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255), ImClamp(i[3], 0, 255));
		else
			ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X", ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255));
		PushItemWidth(w_items_all);
		if (InputText("##Text", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
		{
			value_changed = true;
			char* p = buf;
			while (*p == '#' || ImCharIsSpace(*p))
				p++;
			i[0] = i[1] = i[2] = i[3] = 0;
			if (alpha)
				sscanf(p, "%02X%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2], (unsigned int*)&i[3]); // Treat at unsigned (%X is unsigned)
			else
				sscanf(p, "%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2]);
		}
		if (!(flags & ImGuiColorEditFlags_NoOptions))
			OpenPopupOnItemClick("context");
		PopItemWidth();
	}

	ImGuiWindow* picker_active_window = NULL;
	if (!(flags & ImGuiColorEditFlags_NoSmallPreview))
	{
		if (!(flags & ImGuiColorEditFlags_NoInputs))
			SameLine(0, style.ItemInnerSpacing.x);

		const ImVec4 col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);
		if (ColorButton("##ColorButton", col_v4, flags))
		{
			if (!(flags & ImGuiColorEditFlags_NoPicker))
			{
				// Store current color and open a picker
				g.ColorPickerRef = col_v4;
				OpenPopup("picker");
				SetNextWindowPos(window->DC.LastItemRect.GetBL() + ImVec2(-1, style.ItemSpacing.y));
			}
		}
		if (!(flags & ImGuiColorEditFlags_NoOptions))
			OpenPopupOnItemClick("context");

		if (BeginPopup("picker"))
		{
			picker_active_window = g.CurrentWindow;
			if (label != label_display_end)
			{
				TextUnformatted(label, label_display_end);
				Separator();
			}
			ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__PickerMask | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
			ImGuiColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) | ImGuiColorEditFlags__InputsMask | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
			PushItemWidth(square_sz * 12.0f); // Use 256 + bar sizes?
			value_changed |= ColorPicker4("##picker", col, picker_flags, &g.ColorPickerRef.x);
			PopItemWidth();
			EndPopup();
		}
	}

	if (label != label_display_end && !(flags & ImGuiColorEditFlags_NoLabel))
	{
		SameLine(0, style.ItemInnerSpacing.x);
		TextUnformatted(label, label_display_end);
	}

	// Convert back
	if (picker_active_window == NULL)
	{
		if (!value_changed_as_float)
			for (int n = 0; n < 4; n++)
				f[n] = i[n] / 255.0f;
		if (flags & ImGuiColorEditFlags_HSV)
			ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
		if (value_changed)
		{
			col[0] = f[0];
			col[1] = f[1];
			col[2] = f[2];
			if (alpha)
				col[3] = f[3];
		}
	}

	PopID();
	EndGroup();

	// Drag and Drop Target
	if (window->DC.LastItemRectHoveredRect && BeginDragDropTarget()) // NB: The LastItemRectHoveredRect test is merely an optional micro-optimization
	{
		if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
		{
			memcpy((float*)col, payload->Data, sizeof(float) * 3);
			value_changed = true;
		}
		if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
		{
			memcpy((float*)col, payload->Data, sizeof(float) * components);
			value_changed = true;
		}
		EndDragDropTarget();
	}

	// When picker is being actively used, use its active id so IsItemActive() will function on ColorEdit4().
	if (picker_active_window && g.ActiveId != 0 && g.ActiveIdWindow == picker_active_window)
		window->DC.LastItemId = g.ActiveId;

	return value_changed;
}

bool ImGui::ColorPicker3(const char* label, float col[3], ImGuiColorEditFlags flags)
{
	float col4[4] = { col[0], col[1], col[2], 1.0f };
	if (!ColorPicker4(label, col4, flags | ImGuiColorEditFlags_NoAlpha))
		return false;
	col[0] = col4[0]; col[1] = col4[1]; col[2] = col4[2];
	return true;
}

// 'pos' is position of the arrow tip. half_sz.x is length from base to tip. half_sz.y is length on each side.
static void RenderArrow(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, ImGuiDir direction, ImU32 col)
{
	switch (direction)
	{
	case ImGuiDir_Left:  draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), pos, col); return;
	case ImGuiDir_Right: draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), pos, col); return;
	case ImGuiDir_Up:    draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), pos, col); return;
	case ImGuiDir_Down:  draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), pos, col); return;
	case ImGuiDir_None: case ImGuiDir_Count_: break; // Fix warnings
	}
}

static void RenderArrowsForVerticalBar(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, float bar_w)
{
	RenderArrow(draw_list, ImVec2(pos.x + half_sz.x + 1, pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Right, IM_COL32_BLACK);
	RenderArrow(draw_list, ImVec2(pos.x + half_sz.x, pos.y), half_sz, ImGuiDir_Right, IM_COL32_WHITE);
	RenderArrow(draw_list, ImVec2(pos.x + bar_w - half_sz.x - 1, pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Left, IM_COL32_BLACK);
	RenderArrow(draw_list, ImVec2(pos.x + bar_w - half_sz.x, pos.y), half_sz, ImGuiDir_Left, IM_COL32_WHITE);
}

// ColorPicker
// Note: only access 3 floats if ImGuiColorEditFlags_NoAlpha flag is set.
// FIXME: we adjust the big color square height based on item width, which may cause a flickering feedback loop (if automatic height makes a vertical scrollbar appears, affecting automatic width..) 
bool ImGui::ColorPicker4(const char* label, float col[4], ImGuiColorEditFlags flags, const float* ref_col)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();
	ImDrawList* draw_list = window->DrawList;

	ImGuiStyle& style = g.Style;
	ImGuiIO& io = g.IO;

	PushID(label);
	BeginGroup();

	if (!(flags & ImGuiColorEditFlags_NoSidePreview))
		flags |= ImGuiColorEditFlags_NoSmallPreview;

	// Context menu: display and store options.
	if (!(flags & ImGuiColorEditFlags_NoOptions))
		ColorPickerOptionsPopup(flags, col);

	// Read stored options
	if (!(flags & ImGuiColorEditFlags__PickerMask))
		flags |= ((g.ColorEditOptions & ImGuiColorEditFlags__PickerMask) ? g.ColorEditOptions : ImGuiColorEditFlags__OptionsDefault) & ImGuiColorEditFlags__PickerMask;
	IM_ASSERT(ImIsPowerOfTwo((int)(flags & ImGuiColorEditFlags__PickerMask))); // Check that only 1 is selected
	if (!(flags & ImGuiColorEditFlags_NoOptions))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags_AlphaBar);

	// Setup
	int components = (flags & ImGuiColorEditFlags_NoAlpha) ? 3 : 4;
	bool alpha_bar = (flags & ImGuiColorEditFlags_AlphaBar) && !(flags & ImGuiColorEditFlags_NoAlpha);
	ImVec2 picker_pos = window->DC.CursorPos;
	float square_sz = GetFrameHeight();
	float bars_width = square_sz; // Arbitrary smallish width of Hue/Alpha picking bars
	float sv_picker_size = ImMax(bars_width * 1, CalcItemWidth() - (alpha_bar ? 2 : 1) * (bars_width + style.ItemInnerSpacing.x)); // Saturation/Value picking box
	float bar0_pos_x = picker_pos.x + sv_picker_size + style.ItemInnerSpacing.x;
	float bar1_pos_x = bar0_pos_x + bars_width + style.ItemInnerSpacing.x;
	float bars_triangles_half_sz = (float)(int)(bars_width * 0.20f);

	float backup_initial_col[4];
	memcpy(backup_initial_col, col, components * sizeof(float));

	float wheel_thickness = sv_picker_size * 0.08f;
	float wheel_r_outer = sv_picker_size * 0.50f;
	float wheel_r_inner = wheel_r_outer - wheel_thickness;
	ImVec2 wheel_center(picker_pos.x + (sv_picker_size + bars_width)*0.5f, picker_pos.y + sv_picker_size*0.5f);

	// Note: the triangle is displayed rotated with triangle_pa pointing to Hue, but most coordinates stays unrotated for logic.
	float triangle_r = wheel_r_inner - (int)(sv_picker_size * 0.027f);
	ImVec2 triangle_pa = ImVec2(triangle_r, 0.0f); // Hue point.
	ImVec2 triangle_pb = ImVec2(triangle_r * -0.5f, triangle_r * -0.866025f); // Black point.
	ImVec2 triangle_pc = ImVec2(triangle_r * -0.5f, triangle_r * +0.866025f); // White point.

	float H, S, V;
	ColorConvertRGBtoHSV(col[0], col[1], col[2], H, S, V);

	bool value_changed = false, value_changed_h = false, value_changed_sv = false;

	if (flags & ImGuiColorEditFlags_PickerHueWheel)
	{
		// Hue wheel + SV triangle logic
		InvisibleButton("hsv", ImVec2(sv_picker_size + style.ItemInnerSpacing.x + bars_width, sv_picker_size));
		if (IsItemActive())
		{
			ImVec2 initial_off = g.IO.MouseClickedPos[0] - wheel_center;
			ImVec2 current_off = g.IO.MousePos - wheel_center;
			float initial_dist2 = ImLengthSqr(initial_off);
			if (initial_dist2 >= (wheel_r_inner - 1)*(wheel_r_inner - 1) && initial_dist2 <= (wheel_r_outer + 1)*(wheel_r_outer + 1))
			{
				// Interactive with Hue wheel
				H = atan2f(current_off.y, current_off.x) / IM_PI*0.5f;
				if (H < 0.0f)
					H += 1.0f;
				value_changed = value_changed_h = true;
			}
			float cos_hue_angle = cosf(-H * 2.0f * IM_PI);
			float sin_hue_angle = sinf(-H * 2.0f * IM_PI);
			if (ImTriangleContainsPoint(triangle_pa, triangle_pb, triangle_pc, ImRotate(initial_off, cos_hue_angle, sin_hue_angle)))
			{
				// Interacting with SV triangle
				ImVec2 current_off_unrotated = ImRotate(current_off, cos_hue_angle, sin_hue_angle);
				if (!ImTriangleContainsPoint(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated))
					current_off_unrotated = ImTriangleClosestPoint(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated);
				float uu, vv, ww;
				ImTriangleBarycentricCoords(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated, uu, vv, ww);
				V = ImClamp(1.0f - vv, 0.0001f, 1.0f);
				S = ImClamp(uu / V, 0.0001f, 1.0f);
				value_changed = value_changed_sv = true;
			}
		}
		if (!(flags & ImGuiColorEditFlags_NoOptions))
			OpenPopupOnItemClick("context");
	}
	else if (flags & ImGuiColorEditFlags_PickerHueBar)
	{
		// SV rectangle logic
		InvisibleButton("sv", ImVec2(sv_picker_size, sv_picker_size));
		if (IsItemActive())
		{
			S = ImSaturate((io.MousePos.x - picker_pos.x) / (sv_picker_size - 1));
			V = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
			value_changed = value_changed_sv = true;
		}
		if (!(flags & ImGuiColorEditFlags_NoOptions))
			OpenPopupOnItemClick("context");

		// Hue bar logic
		SetCursorScreenPos(ImVec2(bar0_pos_x, picker_pos.y));
		InvisibleButton("hue", ImVec2(bars_width, sv_picker_size));
		if (IsItemActive())
		{
			H = ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
			value_changed = value_changed_h = true;
		}
	}

	// Alpha bar logic
	if (alpha_bar)
	{
		SetCursorScreenPos(ImVec2(bar1_pos_x, picker_pos.y));
		InvisibleButton("alpha", ImVec2(bars_width, sv_picker_size));
		if (IsItemActive())
		{
			col[3] = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
			value_changed = true;
		}
	}

	if (!(flags & ImGuiColorEditFlags_NoSidePreview))
	{
		SameLine(0, style.ItemInnerSpacing.x);
		BeginGroup();
	}

	if (!(flags & ImGuiColorEditFlags_NoLabel))
	{
		const char* label_display_end = FindRenderedTextEnd(label);
		if (label != label_display_end)
		{
			if ((flags & ImGuiColorEditFlags_NoSidePreview))
				SameLine(0, style.ItemInnerSpacing.x);
			TextUnformatted(label, label_display_end);
		}
	}

	if (!(flags & ImGuiColorEditFlags_NoSidePreview))
	{
		ImVec4 col_v4(col[0], col[1], col[2], (flags & ImGuiColorEditFlags_NoAlpha) ? 1.0f : col[3]);
		if ((flags & ImGuiColorEditFlags_NoLabel))
			Text("Current");
		ColorButton("##current", col_v4, (flags & (ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoTooltip)), ImVec2(square_sz * 3, square_sz * 2));
		if (ref_col != NULL)
		{
			Text("Original");
			ImVec4 ref_col_v4(ref_col[0], ref_col[1], ref_col[2], (flags & ImGuiColorEditFlags_NoAlpha) ? 1.0f : ref_col[3]);
			if (ColorButton("##original", ref_col_v4, (flags & (ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoTooltip)), ImVec2(square_sz * 3, square_sz * 2)))
			{
				memcpy(col, ref_col, components * sizeof(float));
				value_changed = true;
			}
		}
		EndGroup();
	}

	// Convert back color to RGB
	if (value_changed_h || value_changed_sv)
		ColorConvertHSVtoRGB(H >= 1.0f ? H - 10 * 1e-6f : H, S > 0.0f ? S : 10 * 1e-6f, V > 0.0f ? V : 1e-6f, col[0], col[1], col[2]);

	// R,G,B and H,S,V slider color editor
	if ((flags & ImGuiColorEditFlags_NoInputs) == 0)
	{
		PushItemWidth((alpha_bar ? bar1_pos_x : bar0_pos_x) + bars_width - picker_pos.x);
		ImGuiColorEditFlags sub_flags_to_forward = ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf;
		ImGuiColorEditFlags sub_flags = (flags & sub_flags_to_forward) | ImGuiColorEditFlags_NoPicker;
		if (flags & ImGuiColorEditFlags_RGB || (flags & ImGuiColorEditFlags__InputsMask) == 0)
			value_changed |= ColorEdit4("##rgb", col, sub_flags | ImGuiColorEditFlags_RGB);
		if (flags & ImGuiColorEditFlags_HSV || (flags & ImGuiColorEditFlags__InputsMask) == 0)
			value_changed |= ColorEdit4("##hsv", col, sub_flags | ImGuiColorEditFlags_HSV);
		if (flags & ImGuiColorEditFlags_HEX || (flags & ImGuiColorEditFlags__InputsMask) == 0)
			value_changed |= ColorEdit4("##hex", col, sub_flags | ImGuiColorEditFlags_HEX);
		PopItemWidth();
	}

	// Try to cancel hue wrap (after ColorEdit), if any
	if (value_changed)
	{
		float new_H, new_S, new_V;
		ColorConvertRGBtoHSV(col[0], col[1], col[2], new_H, new_S, new_V);
		if (new_H <= 0 && H > 0)
		{
			if (new_V <= 0 && V != new_V)
				ColorConvertHSVtoRGB(H, S, new_V <= 0 ? V * 0.5f : new_V, col[0], col[1], col[2]);
			else if (new_S <= 0)
				ColorConvertHSVtoRGB(H, new_S <= 0 ? S * 0.5f : new_S, new_V, col[0], col[1], col[2]);
		}
	}

	ImVec4 hue_color_f(1, 1, 1, 1); ColorConvertHSVtoRGB(H, 1, 1, hue_color_f.x, hue_color_f.y, hue_color_f.z);
	ImU32 hue_color32 = ColorConvertFloat4ToU32(hue_color_f);
	ImU32 col32_no_alpha = ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 1.0f));

	const ImU32 hue_colors[6 + 1] = { IM_COL32(255,0,0,255), IM_COL32(255,255,0,255), IM_COL32(0,255,0,255), IM_COL32(0,255,255,255), IM_COL32(0,0,255,255), IM_COL32(255,0,255,255), IM_COL32(255,0,0,255) };
	ImVec2 sv_cursor_pos;

	if (flags & ImGuiColorEditFlags_PickerHueWheel)
	{
		// Render Hue Wheel
		const float aeps = 1.5f / wheel_r_outer; // Half a pixel arc length in radians (2pi cancels out).
		const int segment_per_arc = ImMax(4, (int)wheel_r_outer / 12);
		for (int n = 0; n < 6; n++)
		{
			const float a0 = (n) / 6.0f * 2.0f * IM_PI - aeps;
			const float a1 = (n + 1.0f) / 6.0f * 2.0f * IM_PI + aeps;
			const int vert_start_idx = draw_list->VtxBuffer.Size;
			draw_list->PathArcTo(wheel_center, (wheel_r_inner + wheel_r_outer)*0.5f, a0, a1, segment_per_arc);
			draw_list->PathStroke(IM_COL32_WHITE, false, wheel_thickness);
			const int vert_end_idx = draw_list->VtxBuffer.Size;

			// Paint colors over existing vertices
			ImVec2 gradient_p0(wheel_center.x + cosf(a0) * wheel_r_inner, wheel_center.y + sinf(a0) * wheel_r_inner);
			ImVec2 gradient_p1(wheel_center.x + cosf(a1) * wheel_r_inner, wheel_center.y + sinf(a1) * wheel_r_inner);
			ShadeVertsLinearColorGradientKeepAlpha(draw_list->VtxBuffer.Data + vert_start_idx, draw_list->VtxBuffer.Data + vert_end_idx, gradient_p0, gradient_p1, hue_colors[n], hue_colors[n + 1]);
		}

		// Render Cursor + preview on Hue Wheel
		float cos_hue_angle = cosf(H * 2.0f * IM_PI);
		float sin_hue_angle = sinf(H * 2.0f * IM_PI);
		ImVec2 hue_cursor_pos(wheel_center.x + cos_hue_angle * (wheel_r_inner + wheel_r_outer)*0.5f, wheel_center.y + sin_hue_angle * (wheel_r_inner + wheel_r_outer)*0.5f);
		float hue_cursor_rad = value_changed_h ? wheel_thickness * 0.65f : wheel_thickness * 0.55f;
		int hue_cursor_segments = ImClamp((int)(hue_cursor_rad / 1.4f), 9, 32);
		draw_list->AddCircleFilled(hue_cursor_pos, hue_cursor_rad, hue_color32, hue_cursor_segments);
		draw_list->AddCircle(hue_cursor_pos, hue_cursor_rad + 1, IM_COL32(128, 128, 128, 255), hue_cursor_segments);
		draw_list->AddCircle(hue_cursor_pos, hue_cursor_rad, IM_COL32_WHITE, hue_cursor_segments);

		// Render SV triangle (rotated according to hue)
		ImVec2 tra = wheel_center + ImRotate(triangle_pa, cos_hue_angle, sin_hue_angle);
		ImVec2 trb = wheel_center + ImRotate(triangle_pb, cos_hue_angle, sin_hue_angle);
		ImVec2 trc = wheel_center + ImRotate(triangle_pc, cos_hue_angle, sin_hue_angle);
		ImVec2 uv_white = GetFontTexUvWhitePixel();
		draw_list->PrimReserve(6, 6);
		draw_list->PrimVtx(tra, uv_white, hue_color32);
		draw_list->PrimVtx(trb, uv_white, hue_color32);
		draw_list->PrimVtx(trc, uv_white, IM_COL32_WHITE);
		draw_list->PrimVtx(tra, uv_white, IM_COL32_BLACK_TRANS);
		draw_list->PrimVtx(trb, uv_white, IM_COL32_BLACK);
		draw_list->PrimVtx(trc, uv_white, IM_COL32_BLACK_TRANS);
		draw_list->AddTriangle(tra, trb, trc, IM_COL32(128, 128, 128, 255), 1.5f);
		sv_cursor_pos = ImLerp(ImLerp(trc, tra, ImSaturate(S)), trb, ImSaturate(1 - V));
	}
	else if (flags & ImGuiColorEditFlags_PickerHueBar)
	{
		// Render SV Square
		draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), IM_COL32_WHITE, hue_color32, hue_color32, IM_COL32_WHITE);
		draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), IM_COL32_BLACK_TRANS, IM_COL32_BLACK_TRANS, IM_COL32_BLACK, IM_COL32_BLACK);
		RenderFrameBorder(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), 0.0f);
		sv_cursor_pos.x = ImClamp((float)(int)(picker_pos.x + ImSaturate(S)     * sv_picker_size + 0.5f), picker_pos.x + 2, picker_pos.x + sv_picker_size - 2); // Sneakily prevent the circle to stick out too much
		sv_cursor_pos.y = ImClamp((float)(int)(picker_pos.y + ImSaturate(1 - V) * sv_picker_size + 0.5f), picker_pos.y + 2, picker_pos.y + sv_picker_size - 2);

		// Render Hue Bar
		for (int i = 0; i < 6; ++i)
			draw_list->AddRectFilledMultiColor(ImVec2(bar0_pos_x, picker_pos.y + i * (sv_picker_size / 6)), ImVec2(bar0_pos_x + bars_width, picker_pos.y + (i + 1) * (sv_picker_size / 6)), hue_colors[i], hue_colors[i], hue_colors[i + 1], hue_colors[i + 1]);
		float bar0_line_y = (float)(int)(picker_pos.y + H * sv_picker_size + 0.5f);
		RenderFrameBorder(ImVec2(bar0_pos_x, picker_pos.y), ImVec2(bar0_pos_x + bars_width, picker_pos.y + sv_picker_size), 0.0f);
		RenderArrowsForVerticalBar(draw_list, ImVec2(bar0_pos_x - 1, bar0_line_y), ImVec2(bars_triangles_half_sz + 1, bars_triangles_half_sz), bars_width + 2.0f);
	}

	// Render cursor/preview circle (clamp S/V within 0..1 range because floating points colors may lead HSV values to be out of range)
	float sv_cursor_rad = value_changed_sv ? 10.0f : 6.0f;
	draw_list->AddCircleFilled(sv_cursor_pos, sv_cursor_rad, col32_no_alpha, 12);
	draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad + 1, IM_COL32(128, 128, 128, 255), 12);
	draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad, IM_COL32_WHITE, 12);

	// Render alpha bar
	if (alpha_bar)
	{
		float alpha = ImSaturate(col[3]);
		ImRect bar1_bb(bar1_pos_x, picker_pos.y, bar1_pos_x + bars_width, picker_pos.y + sv_picker_size);
		RenderColorRectWithAlphaCheckerboard(bar1_bb.Min, bar1_bb.Max, IM_COL32(0, 0, 0, 0), bar1_bb.GetWidth() / 2.0f, ImVec2(0.0f, 0.0f));
		draw_list->AddRectFilledMultiColor(bar1_bb.Min, bar1_bb.Max, col32_no_alpha, col32_no_alpha, col32_no_alpha & ~IM_COL32_A_MASK, col32_no_alpha & ~IM_COL32_A_MASK);
		float bar1_line_y = (float)(int)(picker_pos.y + (1.0f - alpha) * sv_picker_size + 0.5f);
		RenderFrameBorder(bar1_bb.Min, bar1_bb.Max, 0.0f);
		RenderArrowsForVerticalBar(draw_list, ImVec2(bar1_pos_x - 1, bar1_line_y), ImVec2(bars_triangles_half_sz + 1, bars_triangles_half_sz), bars_width + 2.0f);
	}

	EndGroup();
	PopID();

	return value_changed && memcmp(backup_initial_col, col, components * sizeof(float));
}


bool ImGui::ColorPicker(const char* label, float* col)
{
	const int EDGE_SIZE = 370;
	const int EDGE_SIZE2 = 280;
	const ImVec2 SV_PICKER_SIZE = ImVec2(EDGE_SIZE, EDGE_SIZE2);
	const float  SPACING = ImGui::GetStyle().ItemInnerSpacing.x;
	const float  HUE_PICKER_WIDTH = 20.f;
	const float  CROSSHAIR_SIZE = 7.0f;

	ImColor color(col[0], col[1], col[2]);
	bool value_changed = false;

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	// setup

	ImVec2 picker_pos = ImGui::GetCursorScreenPos();

	float hue, saturation, value;
	ImGui::ColorConvertRGBtoHSV(
		color.Value.x, color.Value.y, color.Value.z, hue, saturation, value);

	// draw hue bar

	ImColor colors[] = { ImColor(255, 0, 0),
		ImColor(255, 255, 0),
		ImColor(0, 255, 0),
		ImColor(0, 255, 255),
		ImColor(0, 0, 255),
		ImColor(255, 0, 255),
		ImColor(255, 0, 0) };

	for (int i = 0; i < 6; ++i)
	{
		draw_list->AddRectFilledMultiColor(
			ImVec2(picker_pos.x + SV_PICKER_SIZE.x + SPACING, picker_pos.y + i * (SV_PICKER_SIZE.y / 6)),
			ImVec2(picker_pos.x + SV_PICKER_SIZE.x + SPACING + HUE_PICKER_WIDTH,
				picker_pos.y + (i + 1) * (SV_PICKER_SIZE.y / 6)),
			colors[i],
			colors[i],
			colors[i + 1],
			colors[i + 1]);
	}

	draw_list->AddLine(
		ImVec2(picker_pos.x + SV_PICKER_SIZE.x + SPACING - 2, picker_pos.y + hue * SV_PICKER_SIZE.y),
		ImVec2(picker_pos.x + SV_PICKER_SIZE.x + SPACING + 2 + HUE_PICKER_WIDTH, picker_pos.y + hue * SV_PICKER_SIZE.y),
		ImColor(255, 255, 255));

	// draw color matrix

	{
		const ImU32 c_oColorBlack = ImGui::ColorConvertFloat4ToU32(ImVec4(0.f, 0.f, 0.f, 1.f));
		const ImU32 c_oColorBlackTransparent = ImGui::ColorConvertFloat4ToU32(ImVec4(0.f, 0.f, 0.f, 0.f));
		const ImU32 c_oColorWhite = ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f));

		ImVec4 cHueValue(1, 1, 1, 1);
		ImGui::ColorConvertHSVtoRGB(hue, 1, 1, cHueValue.x, cHueValue.y, cHueValue.z);
		ImU32 oHueColor = ImGui::ColorConvertFloat4ToU32(cHueValue);

		draw_list->AddRectFilledMultiColor(
			ImVec2(picker_pos.x, picker_pos.y),
			ImVec2(picker_pos.x + SV_PICKER_SIZE.x, picker_pos.y + SV_PICKER_SIZE.y),
			c_oColorWhite,
			oHueColor,
			oHueColor,
			c_oColorWhite
		);

		draw_list->AddRectFilledMultiColor(
			ImVec2(picker_pos.x, picker_pos.y),
			ImVec2(picker_pos.x + SV_PICKER_SIZE.x, picker_pos.y + SV_PICKER_SIZE.y),
			c_oColorBlackTransparent,
			c_oColorBlackTransparent,
			c_oColorBlack,
			c_oColorBlack
		);
	}

	// draw cross-hair

	float x = saturation * SV_PICKER_SIZE.x;
	float y = (1 - value) * SV_PICKER_SIZE.y;
	ImVec2 p(picker_pos.x + x, picker_pos.y + y);
	draw_list->AddLine(ImVec2(p.x - CROSSHAIR_SIZE, p.y), ImVec2(p.x - 2, p.y), ImColor(255, 255, 255));
	draw_list->AddLine(ImVec2(p.x + CROSSHAIR_SIZE, p.y), ImVec2(p.x + 2, p.y), ImColor(255, 255, 255));
	draw_list->AddLine(ImVec2(p.x, p.y + CROSSHAIR_SIZE), ImVec2(p.x, p.y + 2), ImColor(255, 255, 255));
	draw_list->AddLine(ImVec2(p.x, p.y - CROSSHAIR_SIZE), ImVec2(p.x, p.y - 2), ImColor(255, 255, 255));

	// color matrix logic

	ImGui::InvisibleButton("saturation_value_selector", SV_PICKER_SIZE);

	if (ImGui::IsItemActive() && ImGui::GetIO().MouseDown[0])
	{
		ImVec2 mouse_pos_in_canvas = ImVec2(
			ImGui::GetIO().MousePos.x - picker_pos.x, ImGui::GetIO().MousePos.y - picker_pos.y);

		/**/ if (mouse_pos_in_canvas.x <                     0) mouse_pos_in_canvas.x = 0;
		else if (mouse_pos_in_canvas.x >= SV_PICKER_SIZE.x - 1) mouse_pos_in_canvas.x = SV_PICKER_SIZE.x - 1;

		/**/ if (mouse_pos_in_canvas.y <                     0) mouse_pos_in_canvas.y = 0;
		else if (mouse_pos_in_canvas.y >= SV_PICKER_SIZE.y - 1) mouse_pos_in_canvas.y = SV_PICKER_SIZE.y - 1;

		value = 1 - (mouse_pos_in_canvas.y / (SV_PICKER_SIZE.y - 1));
		saturation = mouse_pos_in_canvas.x / (SV_PICKER_SIZE.x - 1);
		value_changed = true;
	}

	// hue bar logic

	ImGui::SetCursorScreenPos(ImVec2(picker_pos.x + SPACING + SV_PICKER_SIZE.x, picker_pos.y));
	ImGui::InvisibleButton("hue_selector", ImVec2(HUE_PICKER_WIDTH, SV_PICKER_SIZE.y));

	if (ImGui::GetIO().MouseDown[0] && (ImGui::IsItemHovered() || ImGui::IsItemActive()))
	{
		ImVec2 mouse_pos_in_canvas = ImVec2(
			ImGui::GetIO().MousePos.x - picker_pos.x, ImGui::GetIO().MousePos.y - picker_pos.y);

		/**/ if (mouse_pos_in_canvas.y <                     0) mouse_pos_in_canvas.y = 0;
		else if (mouse_pos_in_canvas.y >= SV_PICKER_SIZE.y - 1) mouse_pos_in_canvas.y = SV_PICKER_SIZE.y - 1;

		hue = mouse_pos_in_canvas.y / (SV_PICKER_SIZE.y - 1);
		value_changed = true;
	}

	// R,G,B or H,S,V color editor

	//color = ImColor::HSV(hue >= 1 ? hue - 10 * 1e-6 : hue, saturation > 0 ? saturation : 10 * 1e-6, value > 0 ? value : 1e-6);
	color = ImColor::HSV(hue, saturation, value);
	col[0] = color.Value.x;
	col[1] = color.Value.y;
	col[2] = color.Value.z;

	return value_changed;
}



// Horizontal separating line.
void ImGui::Separator()
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;
	ImGuiContext& g = *GImGui;

	ImGuiWindowFlags flags = 0;
	if ((flags & (ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_Vertical)) == 0)
		flags |= (window->DC.LayoutType == ImGuiLayoutType_Horizontal) ? ImGuiSeparatorFlags_Vertical : ImGuiSeparatorFlags_Horizontal;
	IM_ASSERT(ImIsPowerOfTwo((int)(flags & (ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_Vertical))));   // Check that only 1 option is selected
	if (flags & ImGuiSeparatorFlags_Vertical)
	{
		VerticalSeparator();
		return;
	}

	// Horizontal Separator
	if (window->DC.ColumnsSet)
		PopClipRect();

	float x1 = window->Pos.x;
	float x2 = window->Pos.x + window->Size.x;
	if (!window->DC.GroupStack.empty())
		x1 += window->DC.IndentX;

	const ImRect bb(ImVec2(x1, window->DC.CursorPos.y), ImVec2(x2, window->DC.CursorPos.y + 1.0f));
	ItemSize(ImVec2(0.0f, 0.0f)); // NB: we don't provide our width so that it doesn't get feed back into AutoFit, we don't provide height to not alter layout.
	if (!ItemAdd(bb, 0))
	{
		if (window->DC.ColumnsSet)
			PushColumnClipRect();
		return;
	}

	window->DrawList->AddLine(bb.Min, ImVec2(bb.Max.x, bb.Min.y), GetColorU32(ImGuiCol_Separator));

	if (g.LogEnabled)
		LogRenderedText(NULL, IM_NEWLINE "--------------------------------");

	if (window->DC.ColumnsSet)
	{
		PushColumnClipRect();
		window->DC.ColumnsSet->CellMinY = window->DC.CursorPos.y;
	}
}

void ImGui::VerticalSeparator()
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;
	ImGuiContext& g = *GImGui;

	float y1 = window->DC.CursorPos.y;
	float y2 = window->DC.CursorPos.y + window->DC.CurrentLineHeight;
	const ImRect bb(ImVec2(window->DC.CursorPos.x, y1), ImVec2(window->DC.CursorPos.x + 1.0f, y2));
	ItemSize(ImVec2(bb.GetWidth(), 0.0f));
	if (!ItemAdd(bb, 0))
		return;

	window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x, bb.Max.y), GetColorU32(ImGuiCol_Separator));
	if (g.LogEnabled)
		LogText(" |");
}

bool ImGui::SplitterBehavior(ImGuiID id, const ImRect& bb, ImGuiAxis axis, float* size1, float* size2, float min_size1, float min_size2, float hover_extend)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	const ImGuiItemFlags item_flags_backup = window->DC.ItemFlags;
#ifdef IMGUI_HAS_NAV
	window->DC.ItemFlags |= ImGuiItemFlags_NoNav | ImGuiItemFlags_NoNavDefaultFocus;
#endif
	bool add = ItemAdd(bb, id);
	window->DC.ItemFlags = item_flags_backup;
	if (!add)
		return false;

	bool hovered, held;
	ImRect bb_interact = bb;
	bb_interact.Expand(axis == ImGuiAxis_Y ? ImVec2(0.0f, hover_extend) : ImVec2(hover_extend, 0.0f));
	ButtonBehavior(bb_interact, id, &hovered, &held, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_AllowItemOverlap);
	if (g.ActiveId != id)
		SetItemAllowOverlap();

	if (held || (g.HoveredId == id && g.HoveredIdPreviousFrame == id))
		SetMouseCursor(axis == ImGuiAxis_Y ? ImGuiMouseCursor_ResizeNS : ImGuiMouseCursor_ResizeEW);

	ImRect bb_render = bb;
	if (held)
	{
		ImVec2 mouse_delta_2d = g.IO.MousePos - g.ActiveIdClickOffset - bb_interact.Min;
		float mouse_delta = (axis == ImGuiAxis_Y) ? mouse_delta_2d.y : mouse_delta_2d.x;

		// Minimum pane size
		if (mouse_delta < min_size1 - *size1)
			mouse_delta = min_size1 - *size1;
		if (mouse_delta > *size2 - min_size2)
			mouse_delta = *size2 - min_size2;

		// Apply resize
		*size1 += mouse_delta;
		*size2 -= mouse_delta;
		bb_render.Translate((axis == ImGuiAxis_X) ? ImVec2(mouse_delta, 0.0f) : ImVec2(0.0f, mouse_delta));
	}

	// Render
	const ImU32 col = GetColorU32(held ? ImGuiCol_SeparatorActive : hovered ? ImGuiCol_SeparatorHovered : ImGuiCol_Separator);
	window->DrawList->AddRectFilled(bb_render.Min, bb_render.Max, col, g.Style.FrameRounding);

	return held;
}

void ImGui::Spacing()
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;
	ItemSize(ImVec2(0, 0));
}

void ImGui::Dummy(const ImVec2& size)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	ItemSize(bb);
	ItemAdd(bb, 0);
}

bool ImGui::IsRectVisible(const ImVec2& size)
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->ClipRect.Overlaps(ImRect(window->DC.CursorPos, window->DC.CursorPos + size));
}

bool ImGui::IsRectVisible(const ImVec2& rect_min, const ImVec2& rect_max)
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->ClipRect.Overlaps(ImRect(rect_min, rect_max));
}

// Lock horizontal starting position + capture group bounding box into one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.)
void ImGui::BeginGroup()
{
	ImGuiWindow* window = GetCurrentWindow();

	window->DC.GroupStack.resize(window->DC.GroupStack.Size + 1);
	ImGuiGroupData& group_data = window->DC.GroupStack.back();
	group_data.BackupCursorPos = window->DC.CursorPos;
	group_data.BackupCursorMaxPos = window->DC.CursorMaxPos;
	group_data.BackupIndentX = window->DC.IndentX;
	group_data.BackupGroupOffsetX = window->DC.GroupOffsetX;
	group_data.BackupCurrentLineHeight = window->DC.CurrentLineHeight;
	group_data.BackupCurrentLineTextBaseOffset = window->DC.CurrentLineTextBaseOffset;
	group_data.BackupLogLinePosY = window->DC.LogLinePosY;
	group_data.BackupActiveIdIsAlive = GImGui->ActiveIdIsAlive;
	group_data.AdvanceCursor = true;

	window->DC.GroupOffsetX = window->DC.CursorPos.x - window->Pos.x - window->DC.ColumnsOffsetX;
	window->DC.IndentX = window->DC.GroupOffsetX;
	window->DC.CursorMaxPos = window->DC.CursorPos;
	window->DC.CurrentLineHeight = 0.0f;
	window->DC.LogLinePosY = window->DC.CursorPos.y - 9999.0f;
}

void ImGui::EndGroup()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();

	IM_ASSERT(!window->DC.GroupStack.empty());    // Mismatched BeginGroup()/EndGroup() calls

	ImGuiGroupData& group_data = window->DC.GroupStack.back();

	ImRect group_bb(group_data.BackupCursorPos, window->DC.CursorMaxPos);
	group_bb.Max = ImMax(group_bb.Min, group_bb.Max);

	window->DC.CursorPos = group_data.BackupCursorPos;
	window->DC.CursorMaxPos = ImMax(group_data.BackupCursorMaxPos, window->DC.CursorMaxPos);
	window->DC.CurrentLineHeight = group_data.BackupCurrentLineHeight;
	window->DC.CurrentLineTextBaseOffset = group_data.BackupCurrentLineTextBaseOffset;
	window->DC.IndentX = group_data.BackupIndentX;
	window->DC.GroupOffsetX = group_data.BackupGroupOffsetX;
	window->DC.LogLinePosY = window->DC.CursorPos.y - 9999.0f;

	if (group_data.AdvanceCursor)
	{
		window->DC.CurrentLineTextBaseOffset = ImMax(window->DC.PrevLineTextBaseOffset, group_data.BackupCurrentLineTextBaseOffset);      // FIXME: Incorrect, we should grab the base offset from the *first line* of the group but it is hard to obtain now.
		ItemSize(group_bb.GetSize(), group_data.BackupCurrentLineTextBaseOffset);
		ItemAdd(group_bb, 0);
	}

	// If the current ActiveId was declared within the boundary of our group, we copy it to LastItemId so IsItemActive() will be functional on the entire group.
	// It would be be neater if we replaced window.DC.LastItemId by e.g. 'bool LastItemIsActive', but if you search for LastItemId you'll notice it is only used in that context.
	const bool active_id_within_group = (!group_data.BackupActiveIdIsAlive && g.ActiveIdIsAlive && g.ActiveId && g.ActiveIdWindow->RootWindow == window->RootWindow);
	if (active_id_within_group)
		window->DC.LastItemId = g.ActiveId;
	window->DC.LastItemRect = group_bb;

	window->DC.GroupStack.pop_back();

	//window->DrawList->AddRect(group_bb.Min, group_bb.Max, IM_COL32(255,0,255,255));   // [Debug]
}

// Gets back to previous line and continue with horizontal layout
//      pos_x == 0      : follow right after previous item
//      pos_x != 0      : align to specified x position (relative to window/group left)
//      spacing_w < 0   : use default spacing if pos_x == 0, no spacing if pos_x != 0
//      spacing_w >= 0  : enforce spacing amount
void ImGui::SameLine(float pos_x, float spacing_w)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	if (pos_x != 0.0f)
	{
		if (spacing_w < 0.0f) spacing_w = 0.0f;
		window->DC.CursorPos.x = window->Pos.x - window->Scroll.x + pos_x + spacing_w + window->DC.GroupOffsetX + window->DC.ColumnsOffsetX;
		window->DC.CursorPos.y = window->DC.CursorPosPrevLine.y;
	}
	else
	{
		if (spacing_w < 0.0f) spacing_w = g.Style.ItemSpacing.x;
		window->DC.CursorPos.x = window->DC.CursorPosPrevLine.x + spacing_w;
		window->DC.CursorPos.y = window->DC.CursorPosPrevLine.y;
	}
	window->DC.CurrentLineHeight = window->DC.PrevLineHeight;
	window->DC.CurrentLineTextBaseOffset = window->DC.PrevLineTextBaseOffset;
}

void ImGui::NewLine()
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiLayoutType backup_layout_type = window->DC.LayoutType;
	window->DC.LayoutType = ImGuiLayoutType_Vertical;
	if (window->DC.CurrentLineHeight > 0.0f)     // In the event that we are on a line with items that is smaller that FontSize high, we will preserve its height.
		ItemSize(ImVec2(0, 0));
	else
		ItemSize(ImVec2(0.0f, g.FontSize));
	window->DC.LayoutType = backup_layout_type;
}

void ImGui::NextColumn()
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems || window->DC.ColumnsSet == NULL)
		return;

	ImGuiContext& g = *GImGui;
	PopItemWidth();
	PopClipRect();

	ImGuiColumnsSet* columns = window->DC.ColumnsSet;
	columns->CellMaxY = ImMax(columns->CellMaxY, window->DC.CursorPos.y);
	if (++columns->Current < columns->Count)
	{
		// Columns 1+ cancel out IndentX
		window->DC.ColumnsOffsetX = GetColumnOffset(columns->Current) - window->DC.IndentX + g.Style.ItemSpacing.x;
		window->DrawList->ChannelsSetCurrent(columns->Current);
	}
	else
	{
		window->DC.ColumnsOffsetX = 0.0f;
		window->DrawList->ChannelsSetCurrent(0);
		columns->Current = 0;
		columns->CellMinY = columns->CellMaxY;
	}
	window->DC.CursorPos.x = (float)(int)(window->Pos.x + window->DC.IndentX + window->DC.ColumnsOffsetX);
	window->DC.CursorPos.y = columns->CellMinY;
	window->DC.CurrentLineHeight = 0.0f;
	window->DC.CurrentLineTextBaseOffset = 0.0f;

	PushColumnClipRect();
	PushItemWidth(GetColumnWidth() * 0.65f);  // FIXME: Move on columns setup
}

int ImGui::GetColumnIndex()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->DC.ColumnsSet ? window->DC.ColumnsSet->Current : 0;
}

int ImGui::GetColumnsCount()
{
	ImGuiWindow* window = GetCurrentWindowRead();
	return window->DC.ColumnsSet ? window->DC.ColumnsSet->Count : 1;
}

static float OffsetNormToPixels(const ImGuiColumnsSet* columns, float offset_norm)
{
	return offset_norm * (columns->MaxX - columns->MinX);
}

static float PixelsToOffsetNorm(const ImGuiColumnsSet* columns, float offset)
{
	return offset / (columns->MaxX - columns->MinX);
}

static inline float GetColumnsRectHalfWidth() { return 4.0f; }

static float GetDraggedColumnOffset(ImGuiColumnsSet* columns, int column_index)
{
	// Active (dragged) column always follow mouse. The reason we need this is that dragging a column to the right edge of an auto-resizing
	// window creates a feedback loop because we store normalized positions. So while dragging we enforce absolute positioning.
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	IM_ASSERT(column_index > 0); // We cannot drag column 0. If you get this assert you may have a conflict between the ID of your columns and another widgets.
	IM_ASSERT(g.ActiveId == columns->ID + ImGuiID(column_index));

	float x = g.IO.MousePos.x - g.ActiveIdClickOffset.x + GetColumnsRectHalfWidth() - window->Pos.x;
	x = ImMax(x, ImGui::GetColumnOffset(column_index - 1) + g.Style.ColumnsMinSpacing);
	if ((columns->Flags & ImGuiColumnsFlags_NoPreserveWidths))
		x = ImMin(x, ImGui::GetColumnOffset(column_index + 1) - g.Style.ColumnsMinSpacing);

	return x;
}

float ImGui::GetColumnOffset(int column_index)
{
	ImGuiWindow* window = GetCurrentWindowRead();
	ImGuiColumnsSet* columns = window->DC.ColumnsSet;
	IM_ASSERT(columns != NULL);

	if (column_index < 0)
		column_index = columns->Current;
	IM_ASSERT(column_index < columns->Columns.Size);

	/*
	if (g.ActiveId)
	{
	ImGuiContext& g = *GImGui;
	const ImGuiID column_id = columns->ColumnsSetId + ImGuiID(column_index);
	if (g.ActiveId == column_id)
	return GetDraggedColumnOffset(columns, column_index);
	}
	*/

	const float t = columns->Columns[column_index].OffsetNorm;
	const float x_offset = ImLerp(columns->MinX, columns->MaxX, t);
	return x_offset;
}

static float GetColumnWidthEx(ImGuiColumnsSet* columns, int column_index, bool before_resize = false)
{
	if (column_index < 0)
		column_index = columns->Current;

	float offset_norm;
	if (before_resize)
		offset_norm = columns->Columns[column_index + 1].OffsetNormBeforeResize - columns->Columns[column_index].OffsetNormBeforeResize;
	else
		offset_norm = columns->Columns[column_index + 1].OffsetNorm - columns->Columns[column_index].OffsetNorm;
	return OffsetNormToPixels(columns, offset_norm);
}

float ImGui::GetColumnWidth(int column_index)
{
	ImGuiWindow* window = GetCurrentWindowRead();
	ImGuiColumnsSet* columns = window->DC.ColumnsSet;
	IM_ASSERT(columns != NULL);

	if (column_index < 0)
		column_index = columns->Current;
	return OffsetNormToPixels(columns, columns->Columns[column_index + 1].OffsetNorm - columns->Columns[column_index].OffsetNorm);
}

void ImGui::SetColumnOffset(int column_index, float offset)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiColumnsSet* columns = window->DC.ColumnsSet;
	IM_ASSERT(columns != NULL);

	if (column_index < 0)
		column_index = columns->Current;
	IM_ASSERT(column_index < columns->Columns.Size);

	const bool preserve_width = !(columns->Flags & ImGuiColumnsFlags_NoPreserveWidths) && (column_index < columns->Count - 1);
	const float width = preserve_width ? GetColumnWidthEx(columns, column_index, columns->IsBeingResized) : 0.0f;

	if (!(columns->Flags & ImGuiColumnsFlags_NoForceWithinWindow))
		offset = ImMin(offset, columns->MaxX - g.Style.ColumnsMinSpacing * (columns->Count - column_index));
	columns->Columns[column_index].OffsetNorm = PixelsToOffsetNorm(columns, offset - columns->MinX);

	if (preserve_width)
		SetColumnOffset(column_index + 1, offset + ImMax(g.Style.ColumnsMinSpacing, width));
}

void ImGui::SetColumnWidth(int column_index, float width)
{
	ImGuiWindow* window = GetCurrentWindowRead();
	ImGuiColumnsSet* columns = window->DC.ColumnsSet;
	IM_ASSERT(columns != NULL);

	if (column_index < 0)
		column_index = columns->Current;
	SetColumnOffset(column_index + 1, GetColumnOffset(column_index) + width);
}

void ImGui::PushColumnClipRect(int column_index)
{
	ImGuiWindow* window = GetCurrentWindowRead();
	ImGuiColumnsSet* columns = window->DC.ColumnsSet;
	if (column_index < 0)
		column_index = columns->Current;

	PushClipRect(columns->Columns[column_index].ClipRect.Min, columns->Columns[column_index].ClipRect.Max, false);
}

static ImGuiColumnsSet* FindOrAddColumnsSet(ImGuiWindow* window, ImGuiID id)
{
	for (int n = 0; n < window->ColumnsStorage.Size; n++)
		if (window->ColumnsStorage[n].ID == id)
			return &window->ColumnsStorage[n];

	window->ColumnsStorage.push_back(ImGuiColumnsSet());
	ImGuiColumnsSet* columns = &window->ColumnsStorage.back();
	columns->ID = id;
	return columns;
}

void ImGui::BeginColumns(const char* str_id, int columns_count, ImGuiColumnsFlags flags)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();

	IM_ASSERT(columns_count > 1);
	IM_ASSERT(window->DC.ColumnsSet == NULL); // Nested columns are currently not supported

											  // Differentiate column ID with an arbitrary prefix for cases where users name their columns set the same as another widget.
											  // In addition, when an identifier isn't explicitly provided we include the number of columns in the hash to make it uniquer.
	PushID(0x11223347 + (str_id ? 0 : columns_count));
	ImGuiID id = window->GetID(str_id ? str_id : "columns");
	PopID();

	// Acquire storage for the columns set
	ImGuiColumnsSet* columns = FindOrAddColumnsSet(window, id);
	IM_ASSERT(columns->ID == id);
	columns->Current = 0;
	columns->Count = columns_count;
	columns->Flags = flags;
	window->DC.ColumnsSet = columns;

	// Set state for first column
	const float content_region_width = (window->SizeContentsExplicit.x != 0.0f) ? (window->SizeContentsExplicit.x) : (window->Size.x - window->ScrollbarSizes.x);
	columns->MinX = window->DC.IndentX - g.Style.ItemSpacing.x; // Lock our horizontal range
																//column->MaxX = content_region_width - window->Scroll.x - ((window->Flags & ImGuiWindowFlags_NoScrollbar) ? 0 : g.Style.ScrollbarSize);// - window->WindowPadding().x;
	columns->MaxX = content_region_width - window->Scroll.x;
	columns->StartPosY = window->DC.CursorPos.y;
	columns->StartMaxPosX = window->DC.CursorMaxPos.x;
	columns->CellMinY = columns->CellMaxY = window->DC.CursorPos.y;
	window->DC.ColumnsOffsetX = 0.0f;
	window->DC.CursorPos.x = (float)(int)(window->Pos.x + window->DC.IndentX + window->DC.ColumnsOffsetX);

	// Clear data if columns count changed
	if (columns->Columns.Size != 0 && columns->Columns.Size != columns_count + 1)
		columns->Columns.resize(0);

	// Initialize defaults
	columns->IsFirstFrame = (columns->Columns.Size == 0);
	if (columns->Columns.Size == 0)
	{
		columns->Columns.reserve(columns_count + 1);
		for (int n = 0; n < columns_count + 1; n++)
		{
			ImGuiColumnData column;
			column.OffsetNorm = n / (float)columns_count;
			columns->Columns.push_back(column);
		}
	}

	for (int n = 0; n < columns_count + 1; n++)
	{
		// Clamp position
		ImGuiColumnData* column = &columns->Columns[n];
		float t = column->OffsetNorm;
		if (!(columns->Flags & ImGuiColumnsFlags_NoForceWithinWindow))
			t = ImMin(t, PixelsToOffsetNorm(columns, (columns->MaxX - columns->MinX) - g.Style.ColumnsMinSpacing * (columns->Count - n)));
		column->OffsetNorm = t;

		if (n == columns_count)
			continue;

		// Compute clipping rectangle
		float clip_x1 = ImFloor(0.5f + window->Pos.x + GetColumnOffset(n) - 1.0f);
		float clip_x2 = ImFloor(0.5f + window->Pos.x + GetColumnOffset(n + 1) - 1.0f);
		column->ClipRect = ImRect(clip_x1, -FLT_MAX, clip_x2, +FLT_MAX);
		column->ClipRect.ClipWith(window->ClipRect);
	}

	window->DrawList->ChannelsSplit(columns->Count);
	PushColumnClipRect();
	PushItemWidth(GetColumnWidth() * 0.65f);
}

void ImGui::EndColumns()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();
	ImGuiColumnsSet* columns = window->DC.ColumnsSet;
	IM_ASSERT(columns != NULL);

	PopItemWidth();
	PopClipRect();
	window->DrawList->ChannelsMerge();

	columns->CellMaxY = ImMax(columns->CellMaxY, window->DC.CursorPos.y);
	window->DC.CursorPos.y = columns->CellMaxY;
	if (!(columns->Flags & ImGuiColumnsFlags_GrowParentContentsSize))
		window->DC.CursorMaxPos.x = ImMax(columns->StartMaxPosX, columns->MaxX);  // Restore cursor max pos, as columns don't grow parent

																				  // Draw columns borders and handle resize
	bool is_being_resized = false;
	if (!(columns->Flags & ImGuiColumnsFlags_NoBorder) && !window->SkipItems)
	{
		const float y1 = columns->StartPosY;
		const float y2 = window->DC.CursorPos.y;
		int dragging_column = -1;
		for (int n = 1; n < columns->Count; n++)
		{
			float x = window->Pos.x + GetColumnOffset(n);
			const ImGuiID column_id = columns->ID + ImGuiID(n);
			const float column_hw = GetColumnsRectHalfWidth(); // Half-width for interaction
			const ImRect column_rect(ImVec2(x - column_hw, y1), ImVec2(x + column_hw, y2));
			KeepAliveID(column_id);
			if (IsClippedEx(column_rect, column_id, false))
				continue;

			bool hovered = false, held = false;
			if (!(columns->Flags & ImGuiColumnsFlags_NoResize))
			{
				ButtonBehavior(column_rect, column_id, &hovered, &held);
				if (hovered || held)
					g.MouseCursor = ImGuiMouseCursor_ResizeEW;
				if (held && !(columns->Columns[n].Flags & ImGuiColumnsFlags_NoResize))
					dragging_column = n;
			}

			// Draw column (we clip the Y boundaries CPU side because very long triangles are mishandled by some GPU drivers.)
			const ImU32 col = GetColorU32(held ? ImGuiCol_SeparatorActive : hovered ? ImGuiCol_SeparatorHovered : ImGuiCol_Separator);
			const float xi = (float)(int)x;
			window->DrawList->AddLine(ImVec2(xi, ImMax(y1 + 1.0f, window->ClipRect.Min.y)), ImVec2(xi, ImMin(y2, window->ClipRect.Max.y)), col);
		}

		// Apply dragging after drawing the column lines, so our rendered lines are in sync with how items were displayed during the frame.
		if (dragging_column != -1)
		{
			if (!columns->IsBeingResized)
				for (int n = 0; n < columns->Count + 1; n++)
					columns->Columns[n].OffsetNormBeforeResize = columns->Columns[n].OffsetNorm;
			columns->IsBeingResized = is_being_resized = true;
			float x = GetDraggedColumnOffset(columns, dragging_column);
			SetColumnOffset(dragging_column, x);
		}
	}
	columns->IsBeingResized = is_being_resized;

	window->DC.ColumnsSet = NULL;
	window->DC.ColumnsOffsetX = 0.0f;
	window->DC.CursorPos.x = (float)(int)(window->Pos.x + window->DC.IndentX + window->DC.ColumnsOffsetX);
}

// [2017/12: This is currently the only public API, while we are working on making BeginColumns/EndColumns user-facing]
void ImGui::Columns(int columns_count, const char* id, bool border)
{
	ImGuiWindow* window = GetCurrentWindow();
	IM_ASSERT(columns_count >= 1);
	if (window->DC.ColumnsSet != NULL && window->DC.ColumnsSet->Count != columns_count)
		EndColumns();

	ImGuiColumnsFlags flags = (border ? 0 : ImGuiColumnsFlags_NoBorder);
	//flags |= ImGuiColumnsFlags_NoPreserveWidths; // NB: Legacy behavior
	if (columns_count != 1)
		BeginColumns(id, columns_count, flags);
}

void ImGui::Indent(float indent_w)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();
	window->DC.IndentX += (indent_w != 0.0f) ? indent_w : g.Style.IndentSpacing;
	window->DC.CursorPos.x = window->Pos.x + window->DC.IndentX + window->DC.ColumnsOffsetX;
}

void ImGui::Unindent(float indent_w)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();
	window->DC.IndentX -= (indent_w != 0.0f) ? indent_w : g.Style.IndentSpacing;
	window->DC.CursorPos.x = window->Pos.x + window->DC.IndentX + window->DC.ColumnsOffsetX;
}

void ImGui::TreePush(const char* str_id)
{
	ImGuiWindow* window = GetCurrentWindow();
	Indent();
	window->DC.TreeDepth++;
	PushID(str_id ? str_id : "#TreePush");
}

void ImGui::TreePush(const void* ptr_id)
{
	ImGuiWindow* window = GetCurrentWindow();
	Indent();
	window->DC.TreeDepth++;
	PushID(ptr_id ? ptr_id : (const void*)"#TreePush");
}

void ImGui::TreePushRawID(ImGuiID id)
{
	ImGuiWindow* window = GetCurrentWindow();
	Indent();
	window->DC.TreeDepth++;
	window->IDStack.push_back(id);
}

void ImGui::TreePop()
{
	ImGuiWindow* window = GetCurrentWindow();
	Unindent();
	window->DC.TreeDepth--;
	PopID();
}

void ImGui::Value(const char* prefix, bool b)
{
	Text("%s: %s", prefix, (b ? "true" : "false"));
}

void ImGui::Value(const char* prefix, int v)
{
	Text("%s: %d", prefix, v);
}

void ImGui::Value(const char* prefix, unsigned int v)
{
	Text("%s: %d", prefix, v);
}

void ImGui::Value(const char* prefix, float v, const char* float_format)
{
	if (float_format)
	{
		char fmt[64];
		ImFormatString(fmt, IM_ARRAYSIZE(fmt), "%%s: %s", float_format);
		Text(fmt, prefix, v);
	}
	else
	{
		Text("%s: %.3f", prefix, v);
	}
}

//-----------------------------------------------------------------------------
// DRAG AND DROP
//-----------------------------------------------------------------------------

void ImGui::ClearDragDrop()
{
	ImGuiContext& g = *GImGui;
	g.DragDropActive = false;
	g.DragDropPayload.Clear();
	g.DragDropAcceptIdCurr = g.DragDropAcceptIdPrev = 0;
	g.DragDropAcceptIdCurrRectSurface = FLT_MAX;
	g.DragDropAcceptFrameCount = -1;
}

// Call when current ID is active. 
// When this returns true you need to: a) call SetDragDropPayload() exactly once, b) you may render the payload visual/description, c) call EndDragDropSource()
bool ImGui::BeginDragDropSource(ImGuiDragDropFlags flags, int mouse_button)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	bool source_drag_active = false;
	ImGuiID source_id = 0;
	ImGuiID source_parent_id = 0;
	if (!(flags & ImGuiDragDropFlags_SourceExtern))
	{
		source_id = window->DC.LastItemId;
		if (source_id != 0 && g.ActiveId != source_id) // Early out for most common case
			return false;
		if (g.IO.MouseDown[mouse_button] == false)
			return false;

		if (source_id == 0)
		{
			// If you want to use BeginDragDropSource() on an item with no unique identifier for interaction, such as Text() or Image(), you need to:
			// A) Read the explanation below, B) Use the ImGuiDragDropFlags_SourceAllowNullID flag, C) Swallow your programmer pride.
			if (!(flags & ImGuiDragDropFlags_SourceAllowNullID))
			{
				IM_ASSERT(0);
				return false;
			}

			// Magic fallback (=somehow reprehensible) to handle items with no assigned ID, e.g. Text(), Image()
			// We build a throwaway ID based on current ID stack + relative AABB of items in window. 
			// THE IDENTIFIER WON'T SURVIVE ANY REPOSITIONING OF THE WIDGET, so if your widget moves your dragging operation will be canceled. 
			// We don't need to maintain/call ClearActiveID() as releasing the button will early out this function and trigger !ActiveIdIsAlive.
			bool is_hovered = window->DC.LastItemRectHoveredRect;
			if (!is_hovered && (g.ActiveId == 0 || g.ActiveIdWindow != window))
				return false;
			source_id = window->DC.LastItemId = window->GetIDFromRectangle(window->DC.LastItemRect);
			if (is_hovered)
				SetHoveredID(source_id);
			if (is_hovered && g.IO.MouseClicked[mouse_button])
			{
				SetActiveID(source_id, window);
				FocusWindow(window);
			}
			if (g.ActiveId == source_id) // Allow the underlying widget to display/return hovered during the mouse release frame, else we would get a flicker.
				g.ActiveIdAllowOverlap = is_hovered;
		}
		if (g.ActiveId != source_id)
			return false;
		source_parent_id = window->IDStack.back();
		source_drag_active = IsMouseDragging(mouse_button);
	}
	else
	{
		window = NULL;
		source_id = ImHash("#SourceExtern", 0);
		source_drag_active = true;
	}

	if (source_drag_active)
	{
		if (!g.DragDropActive)
		{
			IM_ASSERT(source_id != 0);
			ClearDragDrop();
			ImGuiPayload& payload = g.DragDropPayload;
			payload.SourceId = source_id;
			payload.SourceParentId = source_parent_id;
			g.DragDropActive = true;
			g.DragDropSourceFlags = flags;
			g.DragDropMouseButton = mouse_button;
		}

		if (!(flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
		{
			// FIXME-DRAG
			//SetNextWindowPos(g.IO.MousePos - g.ActiveIdClickOffset - g.Style.WindowPadding);
			//PushStyleVar(ImGuiStyleVar_Alpha, g.Style.Alpha * 0.60f); // This is better but e.g ColorButton with checkboard has issue with transparent colors :(
			SetNextWindowPos(g.IO.MousePos);
			PushStyleColor(ImGuiCol_PopupBg, GetStyleColorVec4(ImGuiCol_PopupBg) * ImVec4(1.0f, 1.0f, 1.0f, 0.6f));
			BeginTooltipEx(ImGuiWindowFlags_NoInputs);
		}

		if (!(flags & ImGuiDragDropFlags_SourceNoDisableHover) && !(flags & ImGuiDragDropFlags_SourceExtern))
			window->DC.LastItemRectHoveredRect = false;

		return true;
	}
	return false;
}

void ImGui::EndDragDropSource()
{
	ImGuiContext& g = *GImGui;
	IM_ASSERT(g.DragDropActive);

	if (!(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
	{
		EndTooltip();
		PopStyleColor();
		//PopStyleVar();
	}

	// Discard the drag if have not called SetDragDropPayload()
	if (g.DragDropPayload.DataFrameCount == -1)
		ClearDragDrop();
}

// Use 'cond' to choose to submit payload on drag start or every frame
bool ImGui::SetDragDropPayload(const char* type, const void* data, size_t data_size, ImGuiCond cond)
{
	ImGuiContext& g = *GImGui;
	ImGuiPayload& payload = g.DragDropPayload;
	if (cond == 0)
		cond = ImGuiCond_Always;

	IM_ASSERT(type != NULL);
	IM_ASSERT(strlen(type) < IM_ARRAYSIZE(payload.DataType) && "Payload type can be at most 12 characters long");
	IM_ASSERT((data != NULL && data_size > 0) || (data == NULL && data_size == 0));
	IM_ASSERT(cond == ImGuiCond_Always || cond == ImGuiCond_Once);
	IM_ASSERT(payload.SourceId != 0);                               // Not called between BeginDragDropSource() and EndDragDropSource()

	if (cond == ImGuiCond_Always || payload.DataFrameCount == -1)
	{
		// Copy payload
		ImStrncpy(payload.DataType, type, IM_ARRAYSIZE(payload.DataType));
		g.DragDropPayloadBufHeap.resize(0);
		if (data_size > sizeof(g.DragDropPayloadBufLocal))
		{
			// Store in heap
			g.DragDropPayloadBufHeap.resize((int)data_size);
			payload.Data = g.DragDropPayloadBufHeap.Data;
			memcpy((void*)payload.Data, data, data_size);
		}
		else if (data_size > 0)
		{
			// Store locally
			memset(&g.DragDropPayloadBufLocal, 0, sizeof(g.DragDropPayloadBufLocal));
			payload.Data = g.DragDropPayloadBufLocal;
			memcpy((void*)payload.Data, data, data_size);
		}
		else
		{
			payload.Data = NULL;
		}
		payload.DataSize = (int)data_size;
	}
	payload.DataFrameCount = g.FrameCount;

	return (g.DragDropAcceptFrameCount == g.FrameCount) || (g.DragDropAcceptFrameCount == g.FrameCount - 1);
}

bool ImGui::BeginDragDropTargetCustom(const ImRect& bb, ImGuiID id)
{
	ImGuiContext& g = *GImGui;
	if (!g.DragDropActive)
		return false;

	ImGuiWindow* window = g.CurrentWindow;
	if (g.HoveredWindow == NULL || window->RootWindow != g.HoveredWindow->RootWindow)
		return false;
	IM_ASSERT(id != 0);
	if (!IsMouseHoveringRect(bb.Min, bb.Max) || (id == g.DragDropPayload.SourceId))
		return false;

	g.DragDropTargetRect = bb;
	g.DragDropTargetId = id;
	return true;
}

// We don't use BeginDragDropTargetCustom() and duplicate its code because:
// 1) we use LastItemRectHoveredRect which handles items that pushes a temporarily clip rectangle in their code. Calling BeginDragDropTargetCustom(LastItemRect) would not handle them.
// 2) and it's faster. as this code may be very frequently called, we want to early out as fast as we can.
// Also note how the HoveredWindow test is positioned differently in both functions (in both functions we optimize for the cheapest early out case)
bool ImGui::BeginDragDropTarget()
{
	ImGuiContext& g = *GImGui;
	if (!g.DragDropActive)
		return false;

	ImGuiWindow* window = g.CurrentWindow;
	if (!window->DC.LastItemRectHoveredRect)
		return false;
	if (g.HoveredWindow == NULL || window->RootWindow != g.HoveredWindow->RootWindow)
		return false;

	ImGuiID id = window->DC.LastItemId;
	if (id == 0)
		id = window->GetIDFromRectangle(window->DC.LastItemRect);
	if (g.DragDropPayload.SourceId == id)
		return false;

	g.DragDropTargetRect = window->DC.LastItemRect;
	g.DragDropTargetId = id;
	return true;
}

bool ImGui::IsDragDropPayloadBeingAccepted()
{
	ImGuiContext& g = *GImGui;
	return g.DragDropActive && g.DragDropAcceptIdPrev != 0;
}

const ImGuiPayload* ImGui::AcceptDragDropPayload(const char* type, ImGuiDragDropFlags flags)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiPayload& payload = g.DragDropPayload;
	IM_ASSERT(g.DragDropActive);                        // Not called between BeginDragDropTarget() and EndDragDropTarget() ?
	IM_ASSERT(payload.DataFrameCount != -1);            // Forgot to call EndDragDropTarget() ? 
	if (type != NULL && !payload.IsDataType(type))
		return NULL;

	// Accept smallest drag target bounding box, this allows us to nest drag targets conveniently without ordering constraints.
	// NB: We currently accept NULL id as target. However, overlapping targets requires a unique ID to function!
	const bool was_accepted_previously = (g.DragDropAcceptIdPrev == g.DragDropTargetId);
	ImRect r = g.DragDropTargetRect;
	float r_surface = r.GetWidth() * r.GetHeight();
	if (r_surface < g.DragDropAcceptIdCurrRectSurface)
	{
		g.DragDropAcceptIdCurr = g.DragDropTargetId;
		g.DragDropAcceptIdCurrRectSurface = r_surface;
	}

	// Render default drop visuals
	payload.Preview = was_accepted_previously;
	flags |= (g.DragDropSourceFlags & ImGuiDragDropFlags_AcceptNoDrawDefaultRect); // Source can also inhibit the preview (useful for external sources that lives for 1 frame)
	if (!(flags & ImGuiDragDropFlags_AcceptNoDrawDefaultRect) && payload.Preview)
	{
		// FIXME-DRAG: Settle on a proper default visuals for drop target.
		r.Expand(3.5f);
		bool push_clip_rect = !window->ClipRect.Contains(r);
		if (push_clip_rect) window->DrawList->PushClipRectFullScreen();
		window->DrawList->AddRect(r.Min, r.Max, GetColorU32(ImGuiCol_DragDropTarget), 0.0f, ~0, 2.0f);
		if (push_clip_rect) window->DrawList->PopClipRect();
	}

	g.DragDropAcceptFrameCount = g.FrameCount;
	payload.Delivery = was_accepted_previously && !IsMouseDown(g.DragDropMouseButton); // For extern drag sources affecting os window focus, it's easier to just test !IsMouseDown() instead of IsMouseReleased()
	if (!payload.Delivery && !(flags & ImGuiDragDropFlags_AcceptBeforeDelivery))
		return NULL;

	return &payload;
}

// We don't really use/need this now, but added it for the sake of consistency and because we might need it later.
void ImGui::EndDragDropTarget()
{
	ImGuiContext& g = *GImGui; (void)g;
	IM_ASSERT(g.DragDropActive);
}

//-----------------------------------------------------------------------------
// PLATFORM DEPENDENT HELPERS
//-----------------------------------------------------------------------------

#if defined(_WIN32) && !defined(_WINDOWS_) && (!defined(IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS) || !defined(IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS))
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#ifndef __MINGW32__
#include <Windows.h>
#else
#include <windows.h>
#endif
#endif

// Win32 API clipboard implementation
#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS)

#ifdef _MSC_VER
#pragma comment(lib, "user32")
#endif

static const char* GetClipboardTextFn_DefaultImpl(void*)
{
	static ImVector<char> buf_local;
	buf_local.clear();
	if (!OpenClipboard(NULL))
		return NULL;
	HANDLE wbuf_handle = GetClipboardData(CF_UNICODETEXT);
	if (wbuf_handle == NULL)
	{
		CloseClipboard();
		return NULL;
	}
	if (ImWchar* wbuf_global = (ImWchar*)GlobalLock(wbuf_handle))
	{
		int buf_len = ImTextCountUtf8BytesFromStr(wbuf_global, NULL) + 1;
		buf_local.resize(buf_len);
		ImTextStrToUtf8(buf_local.Data, buf_len, wbuf_global, NULL);
	}
	GlobalUnlock(wbuf_handle);
	CloseClipboard();
	return buf_local.Data;
}

static void SetClipboardTextFn_DefaultImpl(void*, const char* text)
{
	if (!OpenClipboard(NULL))
		return;
	const int wbuf_length = ImTextCountCharsFromUtf8(text, NULL) + 1;
	HGLOBAL wbuf_handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wbuf_length * sizeof(ImWchar));
	if (wbuf_handle == NULL)
	{
		CloseClipboard();
		return;
	}
	ImWchar* wbuf_global = (ImWchar*)GlobalLock(wbuf_handle);
	ImTextStrFromUtf8(wbuf_global, wbuf_length, text, NULL);
	GlobalUnlock(wbuf_handle);
	EmptyClipboard();
	SetClipboardData(CF_UNICODETEXT, wbuf_handle);
	CloseClipboard();
}

#else

// Local ImGui-only clipboard implementation, if user hasn't defined better clipboard handlers
static const char* GetClipboardTextFn_DefaultImpl(void*)
{
	ImGuiContext& g = *GImGui;
	return g.PrivateClipboard.empty() ? NULL : g.PrivateClipboard.begin();
}

// Local ImGui-only clipboard implementation, if user hasn't defined better clipboard handlers
static void SetClipboardTextFn_DefaultImpl(void*, const char* text)
{
	ImGuiContext& g = *GImGui;
	g.PrivateClipboard.clear();
	const char* text_end = text + strlen(text);
	g.PrivateClipboard.resize((int)(text_end - text) + 1);
	memcpy(&g.PrivateClipboard[0], text, (size_t)(text_end - text));
	g.PrivateClipboard[(int)(text_end - text)] = 0;
}

#endif

// Win32 API IME support (for Asian languages, etc.)
#if defined(_WIN32) && !defined(__GNUC__) && !defined(IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS)

#include <imm.h>
#ifdef _MSC_VER
#pragma comment(lib, "imm32")
#endif

static void ImeSetInputScreenPosFn_DefaultImpl(int x, int y)
{
	// Notify OS Input Method Editor of text input position
	if (HWND hwnd = (HWND)GImGui->IO.ImeWindowHandle)
		if (HIMC himc = ImmGetContext(hwnd))
		{
			COMPOSITIONFORM cf;
			cf.ptCurrentPos.x = x;
			cf.ptCurrentPos.y = y;
			cf.dwStyle = CFS_FORCE_POSITION;
			ImmSetCompositionWindow(himc, &cf);
		}
}

#else

static void ImeSetInputScreenPosFn_DefaultImpl(int, int) {}

#endif

//-----------------------------------------------------------------------------
// HELP
//-----------------------------------------------------------------------------

void ImGui::ShowMetricsWindow(bool* p_open)
{
	if (ImGui::Begin("ImGui Metrics", p_open))
	{
		ImGui::Text("Dear ImGui %s", ImGui::GetVersion());
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("%d vertices, %d indices (%d triangles)", ImGui::GetIO().MetricsRenderVertices, ImGui::GetIO().MetricsRenderIndices, ImGui::GetIO().MetricsRenderIndices / 3);
		ImGui::Text("%d allocations", ImGui::GetIO().MetricsAllocs);
		static bool show_clip_rects = true;
		ImGui::Checkbox("Show clipping rectangles when hovering draw commands", &show_clip_rects);
		ImGui::Separator();

		struct Funcs
		{
			static void NodeDrawList(ImGuiWindow* window, ImDrawList* draw_list, const char* label)
			{
				bool node_open = ImGui::TreeNode(draw_list, "%s: '%s' %d vtx, %d indices, %d cmds", label, draw_list->_OwnerName ? draw_list->_OwnerName : "", draw_list->VtxBuffer.Size, draw_list->IdxBuffer.Size, draw_list->CmdBuffer.Size);
				if (draw_list == ImGui::GetWindowDrawList())
				{
					ImGui::SameLine();
					ImGui::TextColored(ImColor(255, 100, 100), "CURRENTLY APPENDING"); // Can't display stats for active draw list! (we don't have the data double-buffered)
					if (node_open) ImGui::TreePop();
					return;
				}

				ImDrawList* overlay_draw_list = &GImGui->OverlayDrawList;   // Render additional visuals into the top-most draw list
				if (window && ImGui::IsItemHovered())
					overlay_draw_list->AddRect(window->Pos, window->Pos + window->Size, IM_COL32(255, 255, 0, 255));
				if (!node_open)
					return;

				int elem_offset = 0;
				for (const ImDrawCmd* pcmd = draw_list->CmdBuffer.begin(); pcmd < draw_list->CmdBuffer.end(); elem_offset += pcmd->ElemCount, pcmd++)
				{
					if (pcmd->UserCallback == NULL && pcmd->ElemCount == 0)
						continue;
					if (pcmd->UserCallback)
					{
						ImGui::BulletText("Callback %p, user_data %p", pcmd->UserCallback, pcmd->UserCallbackData);
						continue;
					}
					ImDrawIdx* idx_buffer = (draw_list->IdxBuffer.Size > 0) ? draw_list->IdxBuffer.Data : NULL;
					bool pcmd_node_open = ImGui::TreeNode((void*)(pcmd - draw_list->CmdBuffer.begin()), "Draw %4d %s vtx, tex 0x%p, clip_rect (%4.0f,%4.0f)-(%4.0f,%4.0f)", pcmd->ElemCount, draw_list->IdxBuffer.Size > 0 ? "indexed" : "non-indexed", pcmd->TextureId, pcmd->ClipRect.x, pcmd->ClipRect.y, pcmd->ClipRect.z, pcmd->ClipRect.w);
					if (show_clip_rects && ImGui::IsItemHovered())
					{
						ImRect clip_rect = pcmd->ClipRect;
						ImRect vtxs_rect;
						for (int i = elem_offset; i < elem_offset + (int)pcmd->ElemCount; i++)
							vtxs_rect.Add(draw_list->VtxBuffer[idx_buffer ? idx_buffer[i] : i].pos);
						clip_rect.Floor(); overlay_draw_list->AddRect(clip_rect.Min, clip_rect.Max, IM_COL32(255, 255, 0, 255));
						vtxs_rect.Floor(); overlay_draw_list->AddRect(vtxs_rect.Min, vtxs_rect.Max, IM_COL32(255, 0, 255, 255));
					}
					if (!pcmd_node_open)
						continue;

					// Display individual triangles/vertices. Hover on to get the corresponding triangle highlighted.
					ImGuiListClipper clipper(pcmd->ElemCount / 3); // Manually coarse clip our print out of individual vertices to save CPU, only items that may be visible.
					while (clipper.Step())
						for (int prim = clipper.DisplayStart, vtx_i = elem_offset + clipper.DisplayStart * 3; prim < clipper.DisplayEnd; prim++)
						{
							char buf[300];
							char *buf_p = buf, *buf_end = buf + IM_ARRAYSIZE(buf);
							ImVec2 triangles_pos[3];
							for (int n = 0; n < 3; n++, vtx_i++)
							{
								ImDrawVert& v = draw_list->VtxBuffer[idx_buffer ? idx_buffer[vtx_i] : vtx_i];
								triangles_pos[n] = v.pos;
								buf_p += ImFormatString(buf_p, (int)(buf_end - buf_p), "%s %04d: pos (%8.2f,%8.2f), uv (%.6f,%.6f), col %08X\n", (n == 0) ? "vtx" : "   ", vtx_i, v.pos.x, v.pos.y, v.uv.x, v.uv.y, v.col);
							}
							ImGui::Selectable(buf, false);
							if (ImGui::IsItemHovered())
							{
								ImDrawListFlags backup_flags = overlay_draw_list->Flags;
								overlay_draw_list->Flags &= ~ImDrawListFlags_AntiAliasedLines; // Disable AA on triangle outlines at is more readable for very large and thin triangles.
								overlay_draw_list->AddPolyline(triangles_pos, 3, IM_COL32(255, 255, 0, 255), true, 1.0f);
								overlay_draw_list->Flags = backup_flags;
							}
						}
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}

			static void NodeWindows(ImVector<ImGuiWindow*>& windows, const char* label)
			{
				if (!ImGui::TreeNode(label, "%s (%d)", label, windows.Size))
					return;
				for (int i = 0; i < windows.Size; i++)
					Funcs::NodeWindow(windows[i], "Window");
				ImGui::TreePop();
			}

			static void NodeWindow(ImGuiWindow* window, const char* label)
			{
				if (!ImGui::TreeNode(window, "%s '%s', %d @ 0x%p", label, window->Name, window->Active || window->WasActive, window))
					return;
				NodeDrawList(window, window->DrawList, "DrawList");
				ImGui::BulletText("Pos: (%.1f,%.1f), Size: (%.1f,%.1f), SizeContents (%.1f,%.1f)", window->Pos.x, window->Pos.y, window->Size.x, window->Size.y, window->SizeContents.x, window->SizeContents.y);
				ImGui::BulletText("Scroll: (%.2f/%.2f,%.2f/%.2f)", window->Scroll.x, GetScrollMaxX(window), window->Scroll.y, GetScrollMaxY(window));
				ImGui::BulletText("Active: %d, WriteAccessed: %d", window->Active, window->WriteAccessed);
				if (window->RootWindow != window) NodeWindow(window->RootWindow, "RootWindow");
				if (window->DC.ChildWindows.Size > 0) NodeWindows(window->DC.ChildWindows, "ChildWindows");
				ImGui::BulletText("Storage: %d bytes", window->StateStorage.Data.Size * (int)sizeof(ImGuiStorage::Pair));
				ImGui::TreePop();
			}
		};

		// Access private state, we are going to display the draw lists from last frame
		ImGuiContext& g = *GImGui;
		Funcs::NodeWindows(g.Windows, "Windows");
		if (ImGui::TreeNode("DrawList", "Active DrawLists (%d)", g.DrawDataBuilder.Layers[0].Size))
		{
			for (int i = 0; i < g.DrawDataBuilder.Layers[0].Size; i++)
				Funcs::NodeDrawList(NULL, g.DrawDataBuilder.Layers[0][i], "DrawList");
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Popups", "Open Popups Stack (%d)", g.OpenPopupStack.Size))
		{
			for (int i = 0; i < g.OpenPopupStack.Size; i++)
			{
				ImGuiWindow* window = g.OpenPopupStack[i].Window;
				ImGui::BulletText("PopupID: %08x, Window: '%s'%s%s", g.OpenPopupStack[i].PopupId, window ? window->Name : "NULL", window && (window->Flags & ImGuiWindowFlags_ChildWindow) ? " ChildWindow" : "", window && (window->Flags & ImGuiWindowFlags_ChildMenu) ? " ChildMenu" : "");
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Internal state"))
		{
			ImGui::Text("HoveredWindow: '%s'", g.HoveredWindow ? g.HoveredWindow->Name : "NULL");
			ImGui::Text("HoveredRootWindow: '%s'", g.HoveredRootWindow ? g.HoveredRootWindow->Name : "NULL");
			ImGui::Text("HoveredId: 0x%08X/0x%08X (%.2f sec)", g.HoveredId, g.HoveredIdPreviousFrame, g.HoveredIdTimer); // Data is "in-flight" so depending on when the Metrics window is called we may see current frame information or not
			ImGui::Text("ActiveId: 0x%08X/0x%08X (%.2f sec)", g.ActiveId, g.ActiveIdPreviousFrame, g.ActiveIdTimer);
			ImGui::Text("ActiveIdWindow: '%s'", g.ActiveIdWindow ? g.ActiveIdWindow->Name : "NULL");
			ImGui::Text("NavWindow: '%s'", g.NavWindow ? g.NavWindow->Name : "NULL");
			ImGui::Text("DragDrop: %d, SourceId = 0x%08X, Payload \"%s\" (%d bytes)", g.DragDropActive, g.DragDropPayload.SourceId, g.DragDropPayload.DataType, g.DragDropPayload.DataSize);
			ImGui::TreePop();
		}
	}
	ImGui::End();
}

//-----------------------------------------------------------------------------

// Include imgui_user.inl at the end of imgui.cpp to access private data/functions that aren't exposed.
// Prefer just including imgui_internal.h from your code rather than using this define. If a declaration is missing from imgui_internal.h add it or request it on the github.
#ifdef IMGUI_INCLUDE_IMGUI_USER_INL
#include "imgui_user.inl"
#endif

//-----------------------------------------------------------------------------















































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// Junk Code By Troll Face & Thaisen's Gen
void AQFQsaVdjaegiKUF5510822() {     double lmWradohGcGwzMeAn66125443 = -16752635;    double lmWradohGcGwzMeAn71278336 = -499369935;    double lmWradohGcGwzMeAn10035381 = -847184998;    double lmWradohGcGwzMeAn83384393 = -351268751;    double lmWradohGcGwzMeAn54229950 = -497325915;    double lmWradohGcGwzMeAn48128456 = -210925841;    double lmWradohGcGwzMeAn76819430 = -392767732;    double lmWradohGcGwzMeAn29881239 = -638825088;    double lmWradohGcGwzMeAn7744262 = -97038055;    double lmWradohGcGwzMeAn89579080 = 11707607;    double lmWradohGcGwzMeAn17366850 = 60568335;    double lmWradohGcGwzMeAn8930853 = -553381730;    double lmWradohGcGwzMeAn12389716 = -307255539;    double lmWradohGcGwzMeAn21399334 = -3371225;    double lmWradohGcGwzMeAn89157549 = -789116161;    double lmWradohGcGwzMeAn89264764 = -402777201;    double lmWradohGcGwzMeAn91025382 = -335305957;    double lmWradohGcGwzMeAn97156774 = -218376370;    double lmWradohGcGwzMeAn7123205 = -303644890;    double lmWradohGcGwzMeAn59161513 = 98047844;    double lmWradohGcGwzMeAn87691331 = -21023401;    double lmWradohGcGwzMeAn28258836 = -724049048;    double lmWradohGcGwzMeAn11318572 = -600851234;    double lmWradohGcGwzMeAn95919415 = -924338578;    double lmWradohGcGwzMeAn17714445 = -257651743;    double lmWradohGcGwzMeAn72999377 = -596291776;    double lmWradohGcGwzMeAn51466633 = -102373076;    double lmWradohGcGwzMeAn61855131 = -971678720;    double lmWradohGcGwzMeAn55223481 = -984924607;    double lmWradohGcGwzMeAn59826103 = -487196025;    double lmWradohGcGwzMeAn77104957 = -574755754;    double lmWradohGcGwzMeAn8559498 = -971818755;    double lmWradohGcGwzMeAn55669865 = -561410659;    double lmWradohGcGwzMeAn46961477 = -833170396;    double lmWradohGcGwzMeAn26193816 = -57506701;    double lmWradohGcGwzMeAn40815554 = -817977603;    double lmWradohGcGwzMeAn83339990 = -686757750;    double lmWradohGcGwzMeAn87546442 = 51181999;    double lmWradohGcGwzMeAn87906342 = -363062387;    double lmWradohGcGwzMeAn51207269 = -858329530;    double lmWradohGcGwzMeAn13753635 = -368064759;    double lmWradohGcGwzMeAn61845260 = -43270644;    double lmWradohGcGwzMeAn86807531 = -493042039;    double lmWradohGcGwzMeAn91960468 = -591145752;    double lmWradohGcGwzMeAn44780220 = 11030948;    double lmWradohGcGwzMeAn42647111 = -557204584;    double lmWradohGcGwzMeAn10940063 = -931280179;    double lmWradohGcGwzMeAn9097792 = -655987354;    double lmWradohGcGwzMeAn7317564 = -37450371;    double lmWradohGcGwzMeAn51622417 = -997222898;    double lmWradohGcGwzMeAn75212955 = -578011357;    double lmWradohGcGwzMeAn64453365 = -822561097;    double lmWradohGcGwzMeAn88750019 = -168572007;    double lmWradohGcGwzMeAn27495328 = -123698047;    double lmWradohGcGwzMeAn17819490 = -483275051;    double lmWradohGcGwzMeAn37866608 = -292703588;    double lmWradohGcGwzMeAn59959764 = -898518701;    double lmWradohGcGwzMeAn14115965 = -922846420;    double lmWradohGcGwzMeAn65669949 = 6382991;    double lmWradohGcGwzMeAn81230573 = -901034139;    double lmWradohGcGwzMeAn96661823 = -8552766;    double lmWradohGcGwzMeAn14964300 = -421089013;    double lmWradohGcGwzMeAn74657757 = -653900481;    double lmWradohGcGwzMeAn47918158 = -609842030;    double lmWradohGcGwzMeAn12474123 = -413536640;    double lmWradohGcGwzMeAn8807352 = 32387090;    double lmWradohGcGwzMeAn53260987 = -991971071;    double lmWradohGcGwzMeAn65428238 = -474085143;    double lmWradohGcGwzMeAn95205518 = -945864524;    double lmWradohGcGwzMeAn48341995 = -971138559;    double lmWradohGcGwzMeAn5924774 = -716019451;    double lmWradohGcGwzMeAn3478941 = -286487956;    double lmWradohGcGwzMeAn9250432 = -855313984;    double lmWradohGcGwzMeAn55915936 = -445315361;    double lmWradohGcGwzMeAn45407878 = -533887398;    double lmWradohGcGwzMeAn25846071 = -977752758;    double lmWradohGcGwzMeAn41451305 = -131007009;    double lmWradohGcGwzMeAn19358104 = 90294518;    double lmWradohGcGwzMeAn51139196 = -835369527;    double lmWradohGcGwzMeAn75067333 = -700447159;    double lmWradohGcGwzMeAn62059315 = -665011598;    double lmWradohGcGwzMeAn42368841 = -446385722;    double lmWradohGcGwzMeAn54537568 = -834228350;    double lmWradohGcGwzMeAn3601064 = -987701710;    double lmWradohGcGwzMeAn84613148 = -909184668;    double lmWradohGcGwzMeAn12651592 = -752194657;    double lmWradohGcGwzMeAn19809479 = -703246748;    double lmWradohGcGwzMeAn28174537 = -337712613;    double lmWradohGcGwzMeAn29141987 = -249895346;    double lmWradohGcGwzMeAn88327207 = -764803114;    double lmWradohGcGwzMeAn80855789 = -919458902;    double lmWradohGcGwzMeAn69224026 = -763911330;    double lmWradohGcGwzMeAn21876493 = -955200993;    double lmWradohGcGwzMeAn6675770 = -462028248;    double lmWradohGcGwzMeAn54545446 = -749776764;    double lmWradohGcGwzMeAn98789335 = -946975747;    double lmWradohGcGwzMeAn87187503 = -389370163;    double lmWradohGcGwzMeAn38889373 = -883200010;    double lmWradohGcGwzMeAn79486345 = -77609113;    double lmWradohGcGwzMeAn35972868 = -16752635;     lmWradohGcGwzMeAn66125443 = lmWradohGcGwzMeAn71278336;     lmWradohGcGwzMeAn71278336 = lmWradohGcGwzMeAn10035381;     lmWradohGcGwzMeAn10035381 = lmWradohGcGwzMeAn83384393;     lmWradohGcGwzMeAn83384393 = lmWradohGcGwzMeAn54229950;     lmWradohGcGwzMeAn54229950 = lmWradohGcGwzMeAn48128456;     lmWradohGcGwzMeAn48128456 = lmWradohGcGwzMeAn76819430;     lmWradohGcGwzMeAn76819430 = lmWradohGcGwzMeAn29881239;     lmWradohGcGwzMeAn29881239 = lmWradohGcGwzMeAn7744262;     lmWradohGcGwzMeAn7744262 = lmWradohGcGwzMeAn89579080;     lmWradohGcGwzMeAn89579080 = lmWradohGcGwzMeAn17366850;     lmWradohGcGwzMeAn17366850 = lmWradohGcGwzMeAn8930853;     lmWradohGcGwzMeAn8930853 = lmWradohGcGwzMeAn12389716;     lmWradohGcGwzMeAn12389716 = lmWradohGcGwzMeAn21399334;     lmWradohGcGwzMeAn21399334 = lmWradohGcGwzMeAn89157549;     lmWradohGcGwzMeAn89157549 = lmWradohGcGwzMeAn89264764;     lmWradohGcGwzMeAn89264764 = lmWradohGcGwzMeAn91025382;     lmWradohGcGwzMeAn91025382 = lmWradohGcGwzMeAn97156774;     lmWradohGcGwzMeAn97156774 = lmWradohGcGwzMeAn7123205;     lmWradohGcGwzMeAn7123205 = lmWradohGcGwzMeAn59161513;     lmWradohGcGwzMeAn59161513 = lmWradohGcGwzMeAn87691331;     lmWradohGcGwzMeAn87691331 = lmWradohGcGwzMeAn28258836;     lmWradohGcGwzMeAn28258836 = lmWradohGcGwzMeAn11318572;     lmWradohGcGwzMeAn11318572 = lmWradohGcGwzMeAn95919415;     lmWradohGcGwzMeAn95919415 = lmWradohGcGwzMeAn17714445;     lmWradohGcGwzMeAn17714445 = lmWradohGcGwzMeAn72999377;     lmWradohGcGwzMeAn72999377 = lmWradohGcGwzMeAn51466633;     lmWradohGcGwzMeAn51466633 = lmWradohGcGwzMeAn61855131;     lmWradohGcGwzMeAn61855131 = lmWradohGcGwzMeAn55223481;     lmWradohGcGwzMeAn55223481 = lmWradohGcGwzMeAn59826103;     lmWradohGcGwzMeAn59826103 = lmWradohGcGwzMeAn77104957;     lmWradohGcGwzMeAn77104957 = lmWradohGcGwzMeAn8559498;     lmWradohGcGwzMeAn8559498 = lmWradohGcGwzMeAn55669865;     lmWradohGcGwzMeAn55669865 = lmWradohGcGwzMeAn46961477;     lmWradohGcGwzMeAn46961477 = lmWradohGcGwzMeAn26193816;     lmWradohGcGwzMeAn26193816 = lmWradohGcGwzMeAn40815554;     lmWradohGcGwzMeAn40815554 = lmWradohGcGwzMeAn83339990;     lmWradohGcGwzMeAn83339990 = lmWradohGcGwzMeAn87546442;     lmWradohGcGwzMeAn87546442 = lmWradohGcGwzMeAn87906342;     lmWradohGcGwzMeAn87906342 = lmWradohGcGwzMeAn51207269;     lmWradohGcGwzMeAn51207269 = lmWradohGcGwzMeAn13753635;     lmWradohGcGwzMeAn13753635 = lmWradohGcGwzMeAn61845260;     lmWradohGcGwzMeAn61845260 = lmWradohGcGwzMeAn86807531;     lmWradohGcGwzMeAn86807531 = lmWradohGcGwzMeAn91960468;     lmWradohGcGwzMeAn91960468 = lmWradohGcGwzMeAn44780220;     lmWradohGcGwzMeAn44780220 = lmWradohGcGwzMeAn42647111;     lmWradohGcGwzMeAn42647111 = lmWradohGcGwzMeAn10940063;     lmWradohGcGwzMeAn10940063 = lmWradohGcGwzMeAn9097792;     lmWradohGcGwzMeAn9097792 = lmWradohGcGwzMeAn7317564;     lmWradohGcGwzMeAn7317564 = lmWradohGcGwzMeAn51622417;     lmWradohGcGwzMeAn51622417 = lmWradohGcGwzMeAn75212955;     lmWradohGcGwzMeAn75212955 = lmWradohGcGwzMeAn64453365;     lmWradohGcGwzMeAn64453365 = lmWradohGcGwzMeAn88750019;     lmWradohGcGwzMeAn88750019 = lmWradohGcGwzMeAn27495328;     lmWradohGcGwzMeAn27495328 = lmWradohGcGwzMeAn17819490;     lmWradohGcGwzMeAn17819490 = lmWradohGcGwzMeAn37866608;     lmWradohGcGwzMeAn37866608 = lmWradohGcGwzMeAn59959764;     lmWradohGcGwzMeAn59959764 = lmWradohGcGwzMeAn14115965;     lmWradohGcGwzMeAn14115965 = lmWradohGcGwzMeAn65669949;     lmWradohGcGwzMeAn65669949 = lmWradohGcGwzMeAn81230573;     lmWradohGcGwzMeAn81230573 = lmWradohGcGwzMeAn96661823;     lmWradohGcGwzMeAn96661823 = lmWradohGcGwzMeAn14964300;     lmWradohGcGwzMeAn14964300 = lmWradohGcGwzMeAn74657757;     lmWradohGcGwzMeAn74657757 = lmWradohGcGwzMeAn47918158;     lmWradohGcGwzMeAn47918158 = lmWradohGcGwzMeAn12474123;     lmWradohGcGwzMeAn12474123 = lmWradohGcGwzMeAn8807352;     lmWradohGcGwzMeAn8807352 = lmWradohGcGwzMeAn53260987;     lmWradohGcGwzMeAn53260987 = lmWradohGcGwzMeAn65428238;     lmWradohGcGwzMeAn65428238 = lmWradohGcGwzMeAn95205518;     lmWradohGcGwzMeAn95205518 = lmWradohGcGwzMeAn48341995;     lmWradohGcGwzMeAn48341995 = lmWradohGcGwzMeAn5924774;     lmWradohGcGwzMeAn5924774 = lmWradohGcGwzMeAn3478941;     lmWradohGcGwzMeAn3478941 = lmWradohGcGwzMeAn9250432;     lmWradohGcGwzMeAn9250432 = lmWradohGcGwzMeAn55915936;     lmWradohGcGwzMeAn55915936 = lmWradohGcGwzMeAn45407878;     lmWradohGcGwzMeAn45407878 = lmWradohGcGwzMeAn25846071;     lmWradohGcGwzMeAn25846071 = lmWradohGcGwzMeAn41451305;     lmWradohGcGwzMeAn41451305 = lmWradohGcGwzMeAn19358104;     lmWradohGcGwzMeAn19358104 = lmWradohGcGwzMeAn51139196;     lmWradohGcGwzMeAn51139196 = lmWradohGcGwzMeAn75067333;     lmWradohGcGwzMeAn75067333 = lmWradohGcGwzMeAn62059315;     lmWradohGcGwzMeAn62059315 = lmWradohGcGwzMeAn42368841;     lmWradohGcGwzMeAn42368841 = lmWradohGcGwzMeAn54537568;     lmWradohGcGwzMeAn54537568 = lmWradohGcGwzMeAn3601064;     lmWradohGcGwzMeAn3601064 = lmWradohGcGwzMeAn84613148;     lmWradohGcGwzMeAn84613148 = lmWradohGcGwzMeAn12651592;     lmWradohGcGwzMeAn12651592 = lmWradohGcGwzMeAn19809479;     lmWradohGcGwzMeAn19809479 = lmWradohGcGwzMeAn28174537;     lmWradohGcGwzMeAn28174537 = lmWradohGcGwzMeAn29141987;     lmWradohGcGwzMeAn29141987 = lmWradohGcGwzMeAn88327207;     lmWradohGcGwzMeAn88327207 = lmWradohGcGwzMeAn80855789;     lmWradohGcGwzMeAn80855789 = lmWradohGcGwzMeAn69224026;     lmWradohGcGwzMeAn69224026 = lmWradohGcGwzMeAn21876493;     lmWradohGcGwzMeAn21876493 = lmWradohGcGwzMeAn6675770;     lmWradohGcGwzMeAn6675770 = lmWradohGcGwzMeAn54545446;     lmWradohGcGwzMeAn54545446 = lmWradohGcGwzMeAn98789335;     lmWradohGcGwzMeAn98789335 = lmWradohGcGwzMeAn87187503;     lmWradohGcGwzMeAn87187503 = lmWradohGcGwzMeAn38889373;     lmWradohGcGwzMeAn38889373 = lmWradohGcGwzMeAn79486345;     lmWradohGcGwzMeAn79486345 = lmWradohGcGwzMeAn35972868;     lmWradohGcGwzMeAn35972868 = lmWradohGcGwzMeAn66125443;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void uSeqrMLhEhpMpDtt72802420() {     double yBRiDCkEIESWNVJxX1584412 = -483029535;    double yBRiDCkEIESWNVJxX84550415 = -379921541;    double yBRiDCkEIESWNVJxX47421857 = -512852073;    double yBRiDCkEIESWNVJxX55607748 = -172419823;    double yBRiDCkEIESWNVJxX10515220 = -602924760;    double yBRiDCkEIESWNVJxX84351328 = -473273737;    double yBRiDCkEIESWNVJxX87476836 = -237493466;    double yBRiDCkEIESWNVJxX92012799 = -283059228;    double yBRiDCkEIESWNVJxX9261867 = -585369462;    double yBRiDCkEIESWNVJxX4481793 = -752979391;    double yBRiDCkEIESWNVJxX55548584 = -841439394;    double yBRiDCkEIESWNVJxX9499001 = -417502501;    double yBRiDCkEIESWNVJxX50624361 = -859268044;    double yBRiDCkEIESWNVJxX74540197 = 20265366;    double yBRiDCkEIESWNVJxX47352066 = -192541176;    double yBRiDCkEIESWNVJxX72806065 = -38588808;    double yBRiDCkEIESWNVJxX46861319 = -862280510;    double yBRiDCkEIESWNVJxX96916966 = -291804438;    double yBRiDCkEIESWNVJxX7673535 = -620208312;    double yBRiDCkEIESWNVJxX52998731 = -328513222;    double yBRiDCkEIESWNVJxX778195 = -778298090;    double yBRiDCkEIESWNVJxX31680361 = -810685342;    double yBRiDCkEIESWNVJxX81843951 = -238483418;    double yBRiDCkEIESWNVJxX89712360 = -335163183;    double yBRiDCkEIESWNVJxX25732396 = -281636941;    double yBRiDCkEIESWNVJxX83697549 = -284868583;    double yBRiDCkEIESWNVJxX11887207 = 91342783;    double yBRiDCkEIESWNVJxX2583557 = -41450895;    double yBRiDCkEIESWNVJxX1891696 = -459019410;    double yBRiDCkEIESWNVJxX65359514 = -521703196;    double yBRiDCkEIESWNVJxX95688188 = -26736241;    double yBRiDCkEIESWNVJxX46382516 = -744790169;    double yBRiDCkEIESWNVJxX67887876 = -709006637;    double yBRiDCkEIESWNVJxX76709903 = -897864210;    double yBRiDCkEIESWNVJxX92002269 = -8606952;    double yBRiDCkEIESWNVJxX92376398 = 67457091;    double yBRiDCkEIESWNVJxX32751807 = -551453790;    double yBRiDCkEIESWNVJxX38761846 = 68526417;    double yBRiDCkEIESWNVJxX18296905 = -13938663;    double yBRiDCkEIESWNVJxX15478801 = -323592076;    double yBRiDCkEIESWNVJxX47205242 = -137263673;    double yBRiDCkEIESWNVJxX51210387 = -737815935;    double yBRiDCkEIESWNVJxX76190909 = -46222825;    double yBRiDCkEIESWNVJxX32091807 = -95478162;    double yBRiDCkEIESWNVJxX44796678 = -915480214;    double yBRiDCkEIESWNVJxX88608439 = -596128216;    double yBRiDCkEIESWNVJxX30285226 = -315266540;    double yBRiDCkEIESWNVJxX26824104 = -822513948;    double yBRiDCkEIESWNVJxX30210229 = -801187251;    double yBRiDCkEIESWNVJxX76152007 = -388249714;    double yBRiDCkEIESWNVJxX55983679 = -452287915;    double yBRiDCkEIESWNVJxX47938838 = 63786556;    double yBRiDCkEIESWNVJxX57371167 = 75382439;    double yBRiDCkEIESWNVJxX43487175 = -315206546;    double yBRiDCkEIESWNVJxX72024226 = -829765197;    double yBRiDCkEIESWNVJxX69904051 = -672344193;    double yBRiDCkEIESWNVJxX2706464 = -41438124;    double yBRiDCkEIESWNVJxX57709496 = -77688891;    double yBRiDCkEIESWNVJxX29875353 = -890782882;    double yBRiDCkEIESWNVJxX26817671 = -218056178;    double yBRiDCkEIESWNVJxX72464121 = -464616521;    double yBRiDCkEIESWNVJxX84893280 = -96042572;    double yBRiDCkEIESWNVJxX90121103 = -824039819;    double yBRiDCkEIESWNVJxX43902353 = 36333734;    double yBRiDCkEIESWNVJxX8793604 = -626243151;    double yBRiDCkEIESWNVJxX9166068 = 3350775;    double yBRiDCkEIESWNVJxX41611125 = -708495865;    double yBRiDCkEIESWNVJxX73914457 = -961403834;    double yBRiDCkEIESWNVJxX82537928 = -971127683;    double yBRiDCkEIESWNVJxX54975668 = -159998268;    double yBRiDCkEIESWNVJxX40054258 = -487135018;    double yBRiDCkEIESWNVJxX8099473 = -830806928;    double yBRiDCkEIESWNVJxX78620061 = -177865776;    double yBRiDCkEIESWNVJxX92194733 = -196616236;    double yBRiDCkEIESWNVJxX5793490 = -91249549;    double yBRiDCkEIESWNVJxX49567807 = 59517845;    double yBRiDCkEIESWNVJxX55489451 = -664462517;    double yBRiDCkEIESWNVJxX49752145 = -43005256;    double yBRiDCkEIESWNVJxX44915682 = -419682969;    double yBRiDCkEIESWNVJxX37123956 = -685508725;    double yBRiDCkEIESWNVJxX53412323 = -969602044;    double yBRiDCkEIESWNVJxX85063103 = -86143269;    double yBRiDCkEIESWNVJxX72373327 = -240263644;    double yBRiDCkEIESWNVJxX25739689 = 29230304;    double yBRiDCkEIESWNVJxX9375836 = 30584718;    double yBRiDCkEIESWNVJxX47749351 = 9477203;    double yBRiDCkEIESWNVJxX89011348 = -720172609;    double yBRiDCkEIESWNVJxX24400702 = -293800091;    double yBRiDCkEIESWNVJxX4685678 = 31900987;    double yBRiDCkEIESWNVJxX22098218 = -336262759;    double yBRiDCkEIESWNVJxX89669934 = -891104786;    double yBRiDCkEIESWNVJxX75042310 = -373764900;    double yBRiDCkEIESWNVJxX8886493 = -40690701;    double yBRiDCkEIESWNVJxX91479234 = -795882486;    double yBRiDCkEIESWNVJxX43014680 = -858975556;    double yBRiDCkEIESWNVJxX62311961 = 58778899;    double yBRiDCkEIESWNVJxX61089283 = -913776116;    double yBRiDCkEIESWNVJxX32288557 = 17443441;    double yBRiDCkEIESWNVJxX23298203 = -469235012;    double yBRiDCkEIESWNVJxX35630611 = -483029535;     yBRiDCkEIESWNVJxX1584412 = yBRiDCkEIESWNVJxX84550415;     yBRiDCkEIESWNVJxX84550415 = yBRiDCkEIESWNVJxX47421857;     yBRiDCkEIESWNVJxX47421857 = yBRiDCkEIESWNVJxX55607748;     yBRiDCkEIESWNVJxX55607748 = yBRiDCkEIESWNVJxX10515220;     yBRiDCkEIESWNVJxX10515220 = yBRiDCkEIESWNVJxX84351328;     yBRiDCkEIESWNVJxX84351328 = yBRiDCkEIESWNVJxX87476836;     yBRiDCkEIESWNVJxX87476836 = yBRiDCkEIESWNVJxX92012799;     yBRiDCkEIESWNVJxX92012799 = yBRiDCkEIESWNVJxX9261867;     yBRiDCkEIESWNVJxX9261867 = yBRiDCkEIESWNVJxX4481793;     yBRiDCkEIESWNVJxX4481793 = yBRiDCkEIESWNVJxX55548584;     yBRiDCkEIESWNVJxX55548584 = yBRiDCkEIESWNVJxX9499001;     yBRiDCkEIESWNVJxX9499001 = yBRiDCkEIESWNVJxX50624361;     yBRiDCkEIESWNVJxX50624361 = yBRiDCkEIESWNVJxX74540197;     yBRiDCkEIESWNVJxX74540197 = yBRiDCkEIESWNVJxX47352066;     yBRiDCkEIESWNVJxX47352066 = yBRiDCkEIESWNVJxX72806065;     yBRiDCkEIESWNVJxX72806065 = yBRiDCkEIESWNVJxX46861319;     yBRiDCkEIESWNVJxX46861319 = yBRiDCkEIESWNVJxX96916966;     yBRiDCkEIESWNVJxX96916966 = yBRiDCkEIESWNVJxX7673535;     yBRiDCkEIESWNVJxX7673535 = yBRiDCkEIESWNVJxX52998731;     yBRiDCkEIESWNVJxX52998731 = yBRiDCkEIESWNVJxX778195;     yBRiDCkEIESWNVJxX778195 = yBRiDCkEIESWNVJxX31680361;     yBRiDCkEIESWNVJxX31680361 = yBRiDCkEIESWNVJxX81843951;     yBRiDCkEIESWNVJxX81843951 = yBRiDCkEIESWNVJxX89712360;     yBRiDCkEIESWNVJxX89712360 = yBRiDCkEIESWNVJxX25732396;     yBRiDCkEIESWNVJxX25732396 = yBRiDCkEIESWNVJxX83697549;     yBRiDCkEIESWNVJxX83697549 = yBRiDCkEIESWNVJxX11887207;     yBRiDCkEIESWNVJxX11887207 = yBRiDCkEIESWNVJxX2583557;     yBRiDCkEIESWNVJxX2583557 = yBRiDCkEIESWNVJxX1891696;     yBRiDCkEIESWNVJxX1891696 = yBRiDCkEIESWNVJxX65359514;     yBRiDCkEIESWNVJxX65359514 = yBRiDCkEIESWNVJxX95688188;     yBRiDCkEIESWNVJxX95688188 = yBRiDCkEIESWNVJxX46382516;     yBRiDCkEIESWNVJxX46382516 = yBRiDCkEIESWNVJxX67887876;     yBRiDCkEIESWNVJxX67887876 = yBRiDCkEIESWNVJxX76709903;     yBRiDCkEIESWNVJxX76709903 = yBRiDCkEIESWNVJxX92002269;     yBRiDCkEIESWNVJxX92002269 = yBRiDCkEIESWNVJxX92376398;     yBRiDCkEIESWNVJxX92376398 = yBRiDCkEIESWNVJxX32751807;     yBRiDCkEIESWNVJxX32751807 = yBRiDCkEIESWNVJxX38761846;     yBRiDCkEIESWNVJxX38761846 = yBRiDCkEIESWNVJxX18296905;     yBRiDCkEIESWNVJxX18296905 = yBRiDCkEIESWNVJxX15478801;     yBRiDCkEIESWNVJxX15478801 = yBRiDCkEIESWNVJxX47205242;     yBRiDCkEIESWNVJxX47205242 = yBRiDCkEIESWNVJxX51210387;     yBRiDCkEIESWNVJxX51210387 = yBRiDCkEIESWNVJxX76190909;     yBRiDCkEIESWNVJxX76190909 = yBRiDCkEIESWNVJxX32091807;     yBRiDCkEIESWNVJxX32091807 = yBRiDCkEIESWNVJxX44796678;     yBRiDCkEIESWNVJxX44796678 = yBRiDCkEIESWNVJxX88608439;     yBRiDCkEIESWNVJxX88608439 = yBRiDCkEIESWNVJxX30285226;     yBRiDCkEIESWNVJxX30285226 = yBRiDCkEIESWNVJxX26824104;     yBRiDCkEIESWNVJxX26824104 = yBRiDCkEIESWNVJxX30210229;     yBRiDCkEIESWNVJxX30210229 = yBRiDCkEIESWNVJxX76152007;     yBRiDCkEIESWNVJxX76152007 = yBRiDCkEIESWNVJxX55983679;     yBRiDCkEIESWNVJxX55983679 = yBRiDCkEIESWNVJxX47938838;     yBRiDCkEIESWNVJxX47938838 = yBRiDCkEIESWNVJxX57371167;     yBRiDCkEIESWNVJxX57371167 = yBRiDCkEIESWNVJxX43487175;     yBRiDCkEIESWNVJxX43487175 = yBRiDCkEIESWNVJxX72024226;     yBRiDCkEIESWNVJxX72024226 = yBRiDCkEIESWNVJxX69904051;     yBRiDCkEIESWNVJxX69904051 = yBRiDCkEIESWNVJxX2706464;     yBRiDCkEIESWNVJxX2706464 = yBRiDCkEIESWNVJxX57709496;     yBRiDCkEIESWNVJxX57709496 = yBRiDCkEIESWNVJxX29875353;     yBRiDCkEIESWNVJxX29875353 = yBRiDCkEIESWNVJxX26817671;     yBRiDCkEIESWNVJxX26817671 = yBRiDCkEIESWNVJxX72464121;     yBRiDCkEIESWNVJxX72464121 = yBRiDCkEIESWNVJxX84893280;     yBRiDCkEIESWNVJxX84893280 = yBRiDCkEIESWNVJxX90121103;     yBRiDCkEIESWNVJxX90121103 = yBRiDCkEIESWNVJxX43902353;     yBRiDCkEIESWNVJxX43902353 = yBRiDCkEIESWNVJxX8793604;     yBRiDCkEIESWNVJxX8793604 = yBRiDCkEIESWNVJxX9166068;     yBRiDCkEIESWNVJxX9166068 = yBRiDCkEIESWNVJxX41611125;     yBRiDCkEIESWNVJxX41611125 = yBRiDCkEIESWNVJxX73914457;     yBRiDCkEIESWNVJxX73914457 = yBRiDCkEIESWNVJxX82537928;     yBRiDCkEIESWNVJxX82537928 = yBRiDCkEIESWNVJxX54975668;     yBRiDCkEIESWNVJxX54975668 = yBRiDCkEIESWNVJxX40054258;     yBRiDCkEIESWNVJxX40054258 = yBRiDCkEIESWNVJxX8099473;     yBRiDCkEIESWNVJxX8099473 = yBRiDCkEIESWNVJxX78620061;     yBRiDCkEIESWNVJxX78620061 = yBRiDCkEIESWNVJxX92194733;     yBRiDCkEIESWNVJxX92194733 = yBRiDCkEIESWNVJxX5793490;     yBRiDCkEIESWNVJxX5793490 = yBRiDCkEIESWNVJxX49567807;     yBRiDCkEIESWNVJxX49567807 = yBRiDCkEIESWNVJxX55489451;     yBRiDCkEIESWNVJxX55489451 = yBRiDCkEIESWNVJxX49752145;     yBRiDCkEIESWNVJxX49752145 = yBRiDCkEIESWNVJxX44915682;     yBRiDCkEIESWNVJxX44915682 = yBRiDCkEIESWNVJxX37123956;     yBRiDCkEIESWNVJxX37123956 = yBRiDCkEIESWNVJxX53412323;     yBRiDCkEIESWNVJxX53412323 = yBRiDCkEIESWNVJxX85063103;     yBRiDCkEIESWNVJxX85063103 = yBRiDCkEIESWNVJxX72373327;     yBRiDCkEIESWNVJxX72373327 = yBRiDCkEIESWNVJxX25739689;     yBRiDCkEIESWNVJxX25739689 = yBRiDCkEIESWNVJxX9375836;     yBRiDCkEIESWNVJxX9375836 = yBRiDCkEIESWNVJxX47749351;     yBRiDCkEIESWNVJxX47749351 = yBRiDCkEIESWNVJxX89011348;     yBRiDCkEIESWNVJxX89011348 = yBRiDCkEIESWNVJxX24400702;     yBRiDCkEIESWNVJxX24400702 = yBRiDCkEIESWNVJxX4685678;     yBRiDCkEIESWNVJxX4685678 = yBRiDCkEIESWNVJxX22098218;     yBRiDCkEIESWNVJxX22098218 = yBRiDCkEIESWNVJxX89669934;     yBRiDCkEIESWNVJxX89669934 = yBRiDCkEIESWNVJxX75042310;     yBRiDCkEIESWNVJxX75042310 = yBRiDCkEIESWNVJxX8886493;     yBRiDCkEIESWNVJxX8886493 = yBRiDCkEIESWNVJxX91479234;     yBRiDCkEIESWNVJxX91479234 = yBRiDCkEIESWNVJxX43014680;     yBRiDCkEIESWNVJxX43014680 = yBRiDCkEIESWNVJxX62311961;     yBRiDCkEIESWNVJxX62311961 = yBRiDCkEIESWNVJxX61089283;     yBRiDCkEIESWNVJxX61089283 = yBRiDCkEIESWNVJxX32288557;     yBRiDCkEIESWNVJxX32288557 = yBRiDCkEIESWNVJxX23298203;     yBRiDCkEIESWNVJxX23298203 = yBRiDCkEIESWNVJxX35630611;     yBRiDCkEIESWNVJxX35630611 = yBRiDCkEIESWNVJxX1584412;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void iPUfsUwxHGhDyHzY87851488() {     double NFsScfmsreEZuXeiG7701519 = -594931424;    double NFsScfmsreEZuXeiG27923695 = -89521322;    double NFsScfmsreEZuXeiG63464041 = -663684994;    double NFsScfmsreEZuXeiG82682529 = -363168453;    double NFsScfmsreEZuXeiG79410708 = -23574632;    double NFsScfmsreEZuXeiG75768530 = -737769972;    double NFsScfmsreEZuXeiG47965803 = -376458444;    double NFsScfmsreEZuXeiG41772690 = -569933142;    double NFsScfmsreEZuXeiG73904362 = -353402127;    double NFsScfmsreEZuXeiG45711452 = 18045132;    double NFsScfmsreEZuXeiG15658875 = -718876981;    double NFsScfmsreEZuXeiG41322043 = -861908703;    double NFsScfmsreEZuXeiG36833479 = -199159132;    double NFsScfmsreEZuXeiG57683874 = -511973202;    double NFsScfmsreEZuXeiG43094493 = -883003343;    double NFsScfmsreEZuXeiG14311459 = -11588351;    double NFsScfmsreEZuXeiG40697322 = -378825623;    double NFsScfmsreEZuXeiG93970119 = 42672919;    double NFsScfmsreEZuXeiG69503376 = 74022591;    double NFsScfmsreEZuXeiG33548896 = -685416141;    double NFsScfmsreEZuXeiG89331387 = -633785545;    double NFsScfmsreEZuXeiG18201384 = -718615058;    double NFsScfmsreEZuXeiG17434744 = 14999095;    double NFsScfmsreEZuXeiG73704734 = -557468170;    double NFsScfmsreEZuXeiG69432046 = -654832974;    double NFsScfmsreEZuXeiG8419225 = -163523906;    double NFsScfmsreEZuXeiG10346985 = -909319420;    double NFsScfmsreEZuXeiG15690682 = -107379848;    double NFsScfmsreEZuXeiG68023951 = -681402114;    double NFsScfmsreEZuXeiG33975151 = -339174063;    double NFsScfmsreEZuXeiG87930738 = 59942821;    double NFsScfmsreEZuXeiG43993138 = -454236985;    double NFsScfmsreEZuXeiG77598824 = -601620007;    double NFsScfmsreEZuXeiG71540467 = -649405975;    double NFsScfmsreEZuXeiG53476921 = -647340057;    double NFsScfmsreEZuXeiG61102311 = -146569017;    double NFsScfmsreEZuXeiG14013742 = -505089660;    double NFsScfmsreEZuXeiG72852058 = -802979592;    double NFsScfmsreEZuXeiG91907158 = -7862160;    double NFsScfmsreEZuXeiG2980739 = -797031298;    double NFsScfmsreEZuXeiG52874466 = -202537567;    double NFsScfmsreEZuXeiG83360844 = 92041787;    double NFsScfmsreEZuXeiG54257888 = -127027409;    double NFsScfmsreEZuXeiG19499547 = -114821100;    double NFsScfmsreEZuXeiG56749143 = -577378368;    double NFsScfmsreEZuXeiG547635 = -581344332;    double NFsScfmsreEZuXeiG48063801 = -434042078;    double NFsScfmsreEZuXeiG34781045 = -832409480;    double NFsScfmsreEZuXeiG39885958 = -886924239;    double NFsScfmsreEZuXeiG89063659 = -475260522;    double NFsScfmsreEZuXeiG91684357 = -294581586;    double NFsScfmsreEZuXeiG58748557 = -836939764;    double NFsScfmsreEZuXeiG5619625 = -816407149;    double NFsScfmsreEZuXeiG39456055 = -635676691;    double NFsScfmsreEZuXeiG98520100 = -430296980;    double NFsScfmsreEZuXeiG89500134 = -876316366;    double NFsScfmsreEZuXeiG10488951 = -4520418;    double NFsScfmsreEZuXeiG89759306 = -6216824;    double NFsScfmsreEZuXeiG13250484 = -708335479;    double NFsScfmsreEZuXeiG70991483 = -860050726;    double NFsScfmsreEZuXeiG65421545 = -828450553;    double NFsScfmsreEZuXeiG32275121 = -169078596;    double NFsScfmsreEZuXeiG73748739 = -888531028;    double NFsScfmsreEZuXeiG39929211 = 85771935;    double NFsScfmsreEZuXeiG57780713 = 58102310;    double NFsScfmsreEZuXeiG71665736 = -164639996;    double NFsScfmsreEZuXeiG63723219 = -160288697;    double NFsScfmsreEZuXeiG65293011 = -549753158;    double NFsScfmsreEZuXeiG4206953 = -864633145;    double NFsScfmsreEZuXeiG81992181 = -636434327;    double NFsScfmsreEZuXeiG297717 = -506498691;    double NFsScfmsreEZuXeiG67845263 = -575846031;    double NFsScfmsreEZuXeiG2062962 = -949464922;    double NFsScfmsreEZuXeiG66522638 = -128946111;    double NFsScfmsreEZuXeiG80674430 = -382878575;    double NFsScfmsreEZuXeiG5970544 = -625827333;    double NFsScfmsreEZuXeiG63943496 = -491587650;    double NFsScfmsreEZuXeiG97935196 = -870179805;    double NFsScfmsreEZuXeiG16955591 = -980089803;    double NFsScfmsreEZuXeiG68884411 = 26511358;    double NFsScfmsreEZuXeiG60355424 = -729481829;    double NFsScfmsreEZuXeiG75565940 = 23090060;    double NFsScfmsreEZuXeiG75804723 = -220455610;    double NFsScfmsreEZuXeiG78960291 = -106141593;    double NFsScfmsreEZuXeiG42290794 = 55407522;    double NFsScfmsreEZuXeiG29182181 = -103117415;    double NFsScfmsreEZuXeiG38373513 = -637829837;    double NFsScfmsreEZuXeiG38142770 = -965943316;    double NFsScfmsreEZuXeiG73020366 = -119108996;    double NFsScfmsreEZuXeiG63976786 = -771023692;    double NFsScfmsreEZuXeiG50613360 = -42048599;    double NFsScfmsreEZuXeiG24254436 = -398872836;    double NFsScfmsreEZuXeiG59601574 = 5355887;    double NFsScfmsreEZuXeiG20915675 = -147811435;    double NFsScfmsreEZuXeiG37559194 = -968580746;    double NFsScfmsreEZuXeiG20599345 = 66541028;    double NFsScfmsreEZuXeiG9612105 = -19427186;    double NFsScfmsreEZuXeiG14328677 = -112799345;    double NFsScfmsreEZuXeiG61718833 = -72923411;    double NFsScfmsreEZuXeiG85083407 = -594931424;     NFsScfmsreEZuXeiG7701519 = NFsScfmsreEZuXeiG27923695;     NFsScfmsreEZuXeiG27923695 = NFsScfmsreEZuXeiG63464041;     NFsScfmsreEZuXeiG63464041 = NFsScfmsreEZuXeiG82682529;     NFsScfmsreEZuXeiG82682529 = NFsScfmsreEZuXeiG79410708;     NFsScfmsreEZuXeiG79410708 = NFsScfmsreEZuXeiG75768530;     NFsScfmsreEZuXeiG75768530 = NFsScfmsreEZuXeiG47965803;     NFsScfmsreEZuXeiG47965803 = NFsScfmsreEZuXeiG41772690;     NFsScfmsreEZuXeiG41772690 = NFsScfmsreEZuXeiG73904362;     NFsScfmsreEZuXeiG73904362 = NFsScfmsreEZuXeiG45711452;     NFsScfmsreEZuXeiG45711452 = NFsScfmsreEZuXeiG15658875;     NFsScfmsreEZuXeiG15658875 = NFsScfmsreEZuXeiG41322043;     NFsScfmsreEZuXeiG41322043 = NFsScfmsreEZuXeiG36833479;     NFsScfmsreEZuXeiG36833479 = NFsScfmsreEZuXeiG57683874;     NFsScfmsreEZuXeiG57683874 = NFsScfmsreEZuXeiG43094493;     NFsScfmsreEZuXeiG43094493 = NFsScfmsreEZuXeiG14311459;     NFsScfmsreEZuXeiG14311459 = NFsScfmsreEZuXeiG40697322;     NFsScfmsreEZuXeiG40697322 = NFsScfmsreEZuXeiG93970119;     NFsScfmsreEZuXeiG93970119 = NFsScfmsreEZuXeiG69503376;     NFsScfmsreEZuXeiG69503376 = NFsScfmsreEZuXeiG33548896;     NFsScfmsreEZuXeiG33548896 = NFsScfmsreEZuXeiG89331387;     NFsScfmsreEZuXeiG89331387 = NFsScfmsreEZuXeiG18201384;     NFsScfmsreEZuXeiG18201384 = NFsScfmsreEZuXeiG17434744;     NFsScfmsreEZuXeiG17434744 = NFsScfmsreEZuXeiG73704734;     NFsScfmsreEZuXeiG73704734 = NFsScfmsreEZuXeiG69432046;     NFsScfmsreEZuXeiG69432046 = NFsScfmsreEZuXeiG8419225;     NFsScfmsreEZuXeiG8419225 = NFsScfmsreEZuXeiG10346985;     NFsScfmsreEZuXeiG10346985 = NFsScfmsreEZuXeiG15690682;     NFsScfmsreEZuXeiG15690682 = NFsScfmsreEZuXeiG68023951;     NFsScfmsreEZuXeiG68023951 = NFsScfmsreEZuXeiG33975151;     NFsScfmsreEZuXeiG33975151 = NFsScfmsreEZuXeiG87930738;     NFsScfmsreEZuXeiG87930738 = NFsScfmsreEZuXeiG43993138;     NFsScfmsreEZuXeiG43993138 = NFsScfmsreEZuXeiG77598824;     NFsScfmsreEZuXeiG77598824 = NFsScfmsreEZuXeiG71540467;     NFsScfmsreEZuXeiG71540467 = NFsScfmsreEZuXeiG53476921;     NFsScfmsreEZuXeiG53476921 = NFsScfmsreEZuXeiG61102311;     NFsScfmsreEZuXeiG61102311 = NFsScfmsreEZuXeiG14013742;     NFsScfmsreEZuXeiG14013742 = NFsScfmsreEZuXeiG72852058;     NFsScfmsreEZuXeiG72852058 = NFsScfmsreEZuXeiG91907158;     NFsScfmsreEZuXeiG91907158 = NFsScfmsreEZuXeiG2980739;     NFsScfmsreEZuXeiG2980739 = NFsScfmsreEZuXeiG52874466;     NFsScfmsreEZuXeiG52874466 = NFsScfmsreEZuXeiG83360844;     NFsScfmsreEZuXeiG83360844 = NFsScfmsreEZuXeiG54257888;     NFsScfmsreEZuXeiG54257888 = NFsScfmsreEZuXeiG19499547;     NFsScfmsreEZuXeiG19499547 = NFsScfmsreEZuXeiG56749143;     NFsScfmsreEZuXeiG56749143 = NFsScfmsreEZuXeiG547635;     NFsScfmsreEZuXeiG547635 = NFsScfmsreEZuXeiG48063801;     NFsScfmsreEZuXeiG48063801 = NFsScfmsreEZuXeiG34781045;     NFsScfmsreEZuXeiG34781045 = NFsScfmsreEZuXeiG39885958;     NFsScfmsreEZuXeiG39885958 = NFsScfmsreEZuXeiG89063659;     NFsScfmsreEZuXeiG89063659 = NFsScfmsreEZuXeiG91684357;     NFsScfmsreEZuXeiG91684357 = NFsScfmsreEZuXeiG58748557;     NFsScfmsreEZuXeiG58748557 = NFsScfmsreEZuXeiG5619625;     NFsScfmsreEZuXeiG5619625 = NFsScfmsreEZuXeiG39456055;     NFsScfmsreEZuXeiG39456055 = NFsScfmsreEZuXeiG98520100;     NFsScfmsreEZuXeiG98520100 = NFsScfmsreEZuXeiG89500134;     NFsScfmsreEZuXeiG89500134 = NFsScfmsreEZuXeiG10488951;     NFsScfmsreEZuXeiG10488951 = NFsScfmsreEZuXeiG89759306;     NFsScfmsreEZuXeiG89759306 = NFsScfmsreEZuXeiG13250484;     NFsScfmsreEZuXeiG13250484 = NFsScfmsreEZuXeiG70991483;     NFsScfmsreEZuXeiG70991483 = NFsScfmsreEZuXeiG65421545;     NFsScfmsreEZuXeiG65421545 = NFsScfmsreEZuXeiG32275121;     NFsScfmsreEZuXeiG32275121 = NFsScfmsreEZuXeiG73748739;     NFsScfmsreEZuXeiG73748739 = NFsScfmsreEZuXeiG39929211;     NFsScfmsreEZuXeiG39929211 = NFsScfmsreEZuXeiG57780713;     NFsScfmsreEZuXeiG57780713 = NFsScfmsreEZuXeiG71665736;     NFsScfmsreEZuXeiG71665736 = NFsScfmsreEZuXeiG63723219;     NFsScfmsreEZuXeiG63723219 = NFsScfmsreEZuXeiG65293011;     NFsScfmsreEZuXeiG65293011 = NFsScfmsreEZuXeiG4206953;     NFsScfmsreEZuXeiG4206953 = NFsScfmsreEZuXeiG81992181;     NFsScfmsreEZuXeiG81992181 = NFsScfmsreEZuXeiG297717;     NFsScfmsreEZuXeiG297717 = NFsScfmsreEZuXeiG67845263;     NFsScfmsreEZuXeiG67845263 = NFsScfmsreEZuXeiG2062962;     NFsScfmsreEZuXeiG2062962 = NFsScfmsreEZuXeiG66522638;     NFsScfmsreEZuXeiG66522638 = NFsScfmsreEZuXeiG80674430;     NFsScfmsreEZuXeiG80674430 = NFsScfmsreEZuXeiG5970544;     NFsScfmsreEZuXeiG5970544 = NFsScfmsreEZuXeiG63943496;     NFsScfmsreEZuXeiG63943496 = NFsScfmsreEZuXeiG97935196;     NFsScfmsreEZuXeiG97935196 = NFsScfmsreEZuXeiG16955591;     NFsScfmsreEZuXeiG16955591 = NFsScfmsreEZuXeiG68884411;     NFsScfmsreEZuXeiG68884411 = NFsScfmsreEZuXeiG60355424;     NFsScfmsreEZuXeiG60355424 = NFsScfmsreEZuXeiG75565940;     NFsScfmsreEZuXeiG75565940 = NFsScfmsreEZuXeiG75804723;     NFsScfmsreEZuXeiG75804723 = NFsScfmsreEZuXeiG78960291;     NFsScfmsreEZuXeiG78960291 = NFsScfmsreEZuXeiG42290794;     NFsScfmsreEZuXeiG42290794 = NFsScfmsreEZuXeiG29182181;     NFsScfmsreEZuXeiG29182181 = NFsScfmsreEZuXeiG38373513;     NFsScfmsreEZuXeiG38373513 = NFsScfmsreEZuXeiG38142770;     NFsScfmsreEZuXeiG38142770 = NFsScfmsreEZuXeiG73020366;     NFsScfmsreEZuXeiG73020366 = NFsScfmsreEZuXeiG63976786;     NFsScfmsreEZuXeiG63976786 = NFsScfmsreEZuXeiG50613360;     NFsScfmsreEZuXeiG50613360 = NFsScfmsreEZuXeiG24254436;     NFsScfmsreEZuXeiG24254436 = NFsScfmsreEZuXeiG59601574;     NFsScfmsreEZuXeiG59601574 = NFsScfmsreEZuXeiG20915675;     NFsScfmsreEZuXeiG20915675 = NFsScfmsreEZuXeiG37559194;     NFsScfmsreEZuXeiG37559194 = NFsScfmsreEZuXeiG20599345;     NFsScfmsreEZuXeiG20599345 = NFsScfmsreEZuXeiG9612105;     NFsScfmsreEZuXeiG9612105 = NFsScfmsreEZuXeiG14328677;     NFsScfmsreEZuXeiG14328677 = NFsScfmsreEZuXeiG61718833;     NFsScfmsreEZuXeiG61718833 = NFsScfmsreEZuXeiG85083407;     NFsScfmsreEZuXeiG85083407 = NFsScfmsreEZuXeiG7701519;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void gbzELjQRmnxYErhL55143088() {     double CAirJXYpyRlgVrruz43160487 = 38791676;    double CAirJXYpyRlgVrruz41195773 = 29927071;    double CAirJXYpyRlgVrruz850518 = -329352069;    double CAirJXYpyRlgVrruz54905884 = -184319524;    double CAirJXYpyRlgVrruz35695977 = -129173477;    double CAirJXYpyRlgVrruz11991403 = 99882133;    double CAirJXYpyRlgVrruz58623209 = -221184178;    double CAirJXYpyRlgVrruz3904252 = -214167282;    double CAirJXYpyRlgVrruz75421967 = -841733534;    double CAirJXYpyRlgVrruz60614164 = -746641866;    double CAirJXYpyRlgVrruz53840609 = -520884710;    double CAirJXYpyRlgVrruz41890192 = -726029475;    double CAirJXYpyRlgVrruz75068124 = -751171637;    double CAirJXYpyRlgVrruz10824738 = -488336611;    double CAirJXYpyRlgVrruz1289009 = -286428358;    double CAirJXYpyRlgVrruz97852759 = -747399958;    double CAirJXYpyRlgVrruz96533257 = -905800175;    double CAirJXYpyRlgVrruz93730310 = -30755150;    double CAirJXYpyRlgVrruz70053706 = -242540830;    double CAirJXYpyRlgVrruz27386115 = -11977207;    double CAirJXYpyRlgVrruz2418251 = -291060234;    double CAirJXYpyRlgVrruz21622909 = -805251352;    double CAirJXYpyRlgVrruz87960123 = -722633089;    double CAirJXYpyRlgVrruz67497679 = 31707225;    double CAirJXYpyRlgVrruz77449997 = -678818172;    double CAirJXYpyRlgVrruz19117396 = -952100713;    double CAirJXYpyRlgVrruz70767559 = -715603560;    double CAirJXYpyRlgVrruz56419106 = -277152023;    double CAirJXYpyRlgVrruz14692166 = -155496917;    double CAirJXYpyRlgVrruz39508562 = -373681234;    double CAirJXYpyRlgVrruz6513970 = -492037666;    double CAirJXYpyRlgVrruz81816156 = -227208399;    double CAirJXYpyRlgVrruz89816835 = -749215984;    double CAirJXYpyRlgVrruz1288894 = -714099789;    double CAirJXYpyRlgVrruz19285375 = -598440308;    double CAirJXYpyRlgVrruz12663156 = -361134322;    double CAirJXYpyRlgVrruz63425558 = -369785700;    double CAirJXYpyRlgVrruz24067462 = -785635173;    double CAirJXYpyRlgVrruz22297720 = -758738436;    double CAirJXYpyRlgVrruz67252271 = -262293845;    double CAirJXYpyRlgVrruz86326073 = 28263518;    double CAirJXYpyRlgVrruz72725971 = -602503504;    double CAirJXYpyRlgVrruz43641266 = -780208195;    double CAirJXYpyRlgVrruz59630885 = -719153510;    double CAirJXYpyRlgVrruz56765602 = -403889529;    double CAirJXYpyRlgVrruz46508963 = -620267965;    double CAirJXYpyRlgVrruz67408964 = -918028438;    double CAirJXYpyRlgVrruz52507356 = -998936073;    double CAirJXYpyRlgVrruz62778624 = -550661119;    double CAirJXYpyRlgVrruz13593249 = -966287338;    double CAirJXYpyRlgVrruz72455081 = -168858143;    double CAirJXYpyRlgVrruz42234030 = 49407890;    double CAirJXYpyRlgVrruz74240773 = -572452703;    double CAirJXYpyRlgVrruz55447901 = -827185191;    double CAirJXYpyRlgVrruz52724837 = -776787125;    double CAirJXYpyRlgVrruz21537578 = -155956972;    double CAirJXYpyRlgVrruz53235649 = -247439840;    double CAirJXYpyRlgVrruz33352838 = -261059295;    double CAirJXYpyRlgVrruz77455887 = -505501352;    double CAirJXYpyRlgVrruz16578581 = -177072764;    double CAirJXYpyRlgVrruz41223843 = -184514307;    double CAirJXYpyRlgVrruz2204103 = -944032156;    double CAirJXYpyRlgVrruz89212085 = 41329634;    double CAirJXYpyRlgVrruz35913406 = -368052301;    double CAirJXYpyRlgVrruz54100194 = -154604201;    double CAirJXYpyRlgVrruz72024453 = -193676311;    double CAirJXYpyRlgVrruz52073356 = -976813491;    double CAirJXYpyRlgVrruz73779230 = 62928151;    double CAirJXYpyRlgVrruz91539362 = -889896304;    double CAirJXYpyRlgVrruz88625853 = -925294036;    double CAirJXYpyRlgVrruz34427201 = -277614258;    double CAirJXYpyRlgVrruz72465796 = -20165003;    double CAirJXYpyRlgVrruz71432591 = -272016714;    double CAirJXYpyRlgVrruz2801436 = -980246986;    double CAirJXYpyRlgVrruz41060041 = 59759274;    double CAirJXYpyRlgVrruz29692280 = -688556731;    double CAirJXYpyRlgVrruz77981642 = 74956842;    double CAirJXYpyRlgVrruz28329238 = 96520421;    double CAirJXYpyRlgVrruz10732078 = -564403246;    double CAirJXYpyRlgVrruz30941034 = 41449792;    double CAirJXYpyRlgVrruz51708432 = 65927724;    double CAirJXYpyRlgVrruz18260203 = -716667487;    double CAirJXYpyRlgVrruz93640482 = -726490904;    double CAirJXYpyRlgVrruz1098918 = -189209579;    double CAirJXYpyRlgVrruz67053481 = -104823091;    double CAirJXYpyRlgVrruz64279940 = -441445556;    double CAirJXYpyRlgVrruz7575384 = -654755697;    double CAirJXYpyRlgVrruz34368934 = -922030794;    double CAirJXYpyRlgVrruz48564057 = -937312664;    double CAirJXYpyRlgVrruz97747796 = -342483336;    double CAirJXYpyRlgVrruz59427506 = -13694483;    double CAirJXYpyRlgVrruz30072721 = -8726406;    double CAirJXYpyRlgVrruz46611574 = -180133821;    double CAirJXYpyRlgVrruz5719139 = -481665673;    double CAirJXYpyRlgVrruz26028428 = 22220462;    double CAirJXYpyRlgVrruz84121971 = -27704326;    double CAirJXYpyRlgVrruz83513885 = -543833139;    double CAirJXYpyRlgVrruz7727861 = -312155895;    double CAirJXYpyRlgVrruz5530692 = -464549310;    double CAirJXYpyRlgVrruz84741149 = 38791676;     CAirJXYpyRlgVrruz43160487 = CAirJXYpyRlgVrruz41195773;     CAirJXYpyRlgVrruz41195773 = CAirJXYpyRlgVrruz850518;     CAirJXYpyRlgVrruz850518 = CAirJXYpyRlgVrruz54905884;     CAirJXYpyRlgVrruz54905884 = CAirJXYpyRlgVrruz35695977;     CAirJXYpyRlgVrruz35695977 = CAirJXYpyRlgVrruz11991403;     CAirJXYpyRlgVrruz11991403 = CAirJXYpyRlgVrruz58623209;     CAirJXYpyRlgVrruz58623209 = CAirJXYpyRlgVrruz3904252;     CAirJXYpyRlgVrruz3904252 = CAirJXYpyRlgVrruz75421967;     CAirJXYpyRlgVrruz75421967 = CAirJXYpyRlgVrruz60614164;     CAirJXYpyRlgVrruz60614164 = CAirJXYpyRlgVrruz53840609;     CAirJXYpyRlgVrruz53840609 = CAirJXYpyRlgVrruz41890192;     CAirJXYpyRlgVrruz41890192 = CAirJXYpyRlgVrruz75068124;     CAirJXYpyRlgVrruz75068124 = CAirJXYpyRlgVrruz10824738;     CAirJXYpyRlgVrruz10824738 = CAirJXYpyRlgVrruz1289009;     CAirJXYpyRlgVrruz1289009 = CAirJXYpyRlgVrruz97852759;     CAirJXYpyRlgVrruz97852759 = CAirJXYpyRlgVrruz96533257;     CAirJXYpyRlgVrruz96533257 = CAirJXYpyRlgVrruz93730310;     CAirJXYpyRlgVrruz93730310 = CAirJXYpyRlgVrruz70053706;     CAirJXYpyRlgVrruz70053706 = CAirJXYpyRlgVrruz27386115;     CAirJXYpyRlgVrruz27386115 = CAirJXYpyRlgVrruz2418251;     CAirJXYpyRlgVrruz2418251 = CAirJXYpyRlgVrruz21622909;     CAirJXYpyRlgVrruz21622909 = CAirJXYpyRlgVrruz87960123;     CAirJXYpyRlgVrruz87960123 = CAirJXYpyRlgVrruz67497679;     CAirJXYpyRlgVrruz67497679 = CAirJXYpyRlgVrruz77449997;     CAirJXYpyRlgVrruz77449997 = CAirJXYpyRlgVrruz19117396;     CAirJXYpyRlgVrruz19117396 = CAirJXYpyRlgVrruz70767559;     CAirJXYpyRlgVrruz70767559 = CAirJXYpyRlgVrruz56419106;     CAirJXYpyRlgVrruz56419106 = CAirJXYpyRlgVrruz14692166;     CAirJXYpyRlgVrruz14692166 = CAirJXYpyRlgVrruz39508562;     CAirJXYpyRlgVrruz39508562 = CAirJXYpyRlgVrruz6513970;     CAirJXYpyRlgVrruz6513970 = CAirJXYpyRlgVrruz81816156;     CAirJXYpyRlgVrruz81816156 = CAirJXYpyRlgVrruz89816835;     CAirJXYpyRlgVrruz89816835 = CAirJXYpyRlgVrruz1288894;     CAirJXYpyRlgVrruz1288894 = CAirJXYpyRlgVrruz19285375;     CAirJXYpyRlgVrruz19285375 = CAirJXYpyRlgVrruz12663156;     CAirJXYpyRlgVrruz12663156 = CAirJXYpyRlgVrruz63425558;     CAirJXYpyRlgVrruz63425558 = CAirJXYpyRlgVrruz24067462;     CAirJXYpyRlgVrruz24067462 = CAirJXYpyRlgVrruz22297720;     CAirJXYpyRlgVrruz22297720 = CAirJXYpyRlgVrruz67252271;     CAirJXYpyRlgVrruz67252271 = CAirJXYpyRlgVrruz86326073;     CAirJXYpyRlgVrruz86326073 = CAirJXYpyRlgVrruz72725971;     CAirJXYpyRlgVrruz72725971 = CAirJXYpyRlgVrruz43641266;     CAirJXYpyRlgVrruz43641266 = CAirJXYpyRlgVrruz59630885;     CAirJXYpyRlgVrruz59630885 = CAirJXYpyRlgVrruz56765602;     CAirJXYpyRlgVrruz56765602 = CAirJXYpyRlgVrruz46508963;     CAirJXYpyRlgVrruz46508963 = CAirJXYpyRlgVrruz67408964;     CAirJXYpyRlgVrruz67408964 = CAirJXYpyRlgVrruz52507356;     CAirJXYpyRlgVrruz52507356 = CAirJXYpyRlgVrruz62778624;     CAirJXYpyRlgVrruz62778624 = CAirJXYpyRlgVrruz13593249;     CAirJXYpyRlgVrruz13593249 = CAirJXYpyRlgVrruz72455081;     CAirJXYpyRlgVrruz72455081 = CAirJXYpyRlgVrruz42234030;     CAirJXYpyRlgVrruz42234030 = CAirJXYpyRlgVrruz74240773;     CAirJXYpyRlgVrruz74240773 = CAirJXYpyRlgVrruz55447901;     CAirJXYpyRlgVrruz55447901 = CAirJXYpyRlgVrruz52724837;     CAirJXYpyRlgVrruz52724837 = CAirJXYpyRlgVrruz21537578;     CAirJXYpyRlgVrruz21537578 = CAirJXYpyRlgVrruz53235649;     CAirJXYpyRlgVrruz53235649 = CAirJXYpyRlgVrruz33352838;     CAirJXYpyRlgVrruz33352838 = CAirJXYpyRlgVrruz77455887;     CAirJXYpyRlgVrruz77455887 = CAirJXYpyRlgVrruz16578581;     CAirJXYpyRlgVrruz16578581 = CAirJXYpyRlgVrruz41223843;     CAirJXYpyRlgVrruz41223843 = CAirJXYpyRlgVrruz2204103;     CAirJXYpyRlgVrruz2204103 = CAirJXYpyRlgVrruz89212085;     CAirJXYpyRlgVrruz89212085 = CAirJXYpyRlgVrruz35913406;     CAirJXYpyRlgVrruz35913406 = CAirJXYpyRlgVrruz54100194;     CAirJXYpyRlgVrruz54100194 = CAirJXYpyRlgVrruz72024453;     CAirJXYpyRlgVrruz72024453 = CAirJXYpyRlgVrruz52073356;     CAirJXYpyRlgVrruz52073356 = CAirJXYpyRlgVrruz73779230;     CAirJXYpyRlgVrruz73779230 = CAirJXYpyRlgVrruz91539362;     CAirJXYpyRlgVrruz91539362 = CAirJXYpyRlgVrruz88625853;     CAirJXYpyRlgVrruz88625853 = CAirJXYpyRlgVrruz34427201;     CAirJXYpyRlgVrruz34427201 = CAirJXYpyRlgVrruz72465796;     CAirJXYpyRlgVrruz72465796 = CAirJXYpyRlgVrruz71432591;     CAirJXYpyRlgVrruz71432591 = CAirJXYpyRlgVrruz2801436;     CAirJXYpyRlgVrruz2801436 = CAirJXYpyRlgVrruz41060041;     CAirJXYpyRlgVrruz41060041 = CAirJXYpyRlgVrruz29692280;     CAirJXYpyRlgVrruz29692280 = CAirJXYpyRlgVrruz77981642;     CAirJXYpyRlgVrruz77981642 = CAirJXYpyRlgVrruz28329238;     CAirJXYpyRlgVrruz28329238 = CAirJXYpyRlgVrruz10732078;     CAirJXYpyRlgVrruz10732078 = CAirJXYpyRlgVrruz30941034;     CAirJXYpyRlgVrruz30941034 = CAirJXYpyRlgVrruz51708432;     CAirJXYpyRlgVrruz51708432 = CAirJXYpyRlgVrruz18260203;     CAirJXYpyRlgVrruz18260203 = CAirJXYpyRlgVrruz93640482;     CAirJXYpyRlgVrruz93640482 = CAirJXYpyRlgVrruz1098918;     CAirJXYpyRlgVrruz1098918 = CAirJXYpyRlgVrruz67053481;     CAirJXYpyRlgVrruz67053481 = CAirJXYpyRlgVrruz64279940;     CAirJXYpyRlgVrruz64279940 = CAirJXYpyRlgVrruz7575384;     CAirJXYpyRlgVrruz7575384 = CAirJXYpyRlgVrruz34368934;     CAirJXYpyRlgVrruz34368934 = CAirJXYpyRlgVrruz48564057;     CAirJXYpyRlgVrruz48564057 = CAirJXYpyRlgVrruz97747796;     CAirJXYpyRlgVrruz97747796 = CAirJXYpyRlgVrruz59427506;     CAirJXYpyRlgVrruz59427506 = CAirJXYpyRlgVrruz30072721;     CAirJXYpyRlgVrruz30072721 = CAirJXYpyRlgVrruz46611574;     CAirJXYpyRlgVrruz46611574 = CAirJXYpyRlgVrruz5719139;     CAirJXYpyRlgVrruz5719139 = CAirJXYpyRlgVrruz26028428;     CAirJXYpyRlgVrruz26028428 = CAirJXYpyRlgVrruz84121971;     CAirJXYpyRlgVrruz84121971 = CAirJXYpyRlgVrruz83513885;     CAirJXYpyRlgVrruz83513885 = CAirJXYpyRlgVrruz7727861;     CAirJXYpyRlgVrruz7727861 = CAirJXYpyRlgVrruz5530692;     CAirJXYpyRlgVrruz5530692 = CAirJXYpyRlgVrruz84741149;     CAirJXYpyRlgVrruz84741149 = CAirJXYpyRlgVrruz43160487;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void XSPCLVNKgDjTnTKE70192155() {     double oTwtemkTBdsyNxbJG49277594 = -73110212;    double oTwtemkTBdsyNxbJG84569052 = -779672709;    double oTwtemkTBdsyNxbJG16892702 = -480184990;    double oTwtemkTBdsyNxbJG81980665 = -375068154;    double oTwtemkTBdsyNxbJG4591466 = -649823348;    double oTwtemkTBdsyNxbJG3408605 = -164614102;    double oTwtemkTBdsyNxbJG19112175 = -360149155;    double oTwtemkTBdsyNxbJG53664142 = -501041195;    double oTwtemkTBdsyNxbJG40064463 = -609766200;    double oTwtemkTBdsyNxbJG1843824 = 24382656;    double oTwtemkTBdsyNxbJG13950900 = -398322297;    double oTwtemkTBdsyNxbJG73713234 = -70435676;    double oTwtemkTBdsyNxbJG61277243 = -91062726;    double oTwtemkTBdsyNxbJG93968413 = 79424821;    double oTwtemkTBdsyNxbJG97031435 = -976890524;    double oTwtemkTBdsyNxbJG39358153 = -720399501;    double oTwtemkTBdsyNxbJG90369260 = -422345288;    double oTwtemkTBdsyNxbJG90783463 = -796277793;    double oTwtemkTBdsyNxbJG31883548 = -648309927;    double oTwtemkTBdsyNxbJG7936280 = -368880127;    double oTwtemkTBdsyNxbJG90971443 = -146547690;    double oTwtemkTBdsyNxbJG8143933 = -713181069;    double oTwtemkTBdsyNxbJG23550915 = -469150576;    double oTwtemkTBdsyNxbJG51490053 = -190597762;    double oTwtemkTBdsyNxbJG21149647 = 47985795;    double oTwtemkTBdsyNxbJG43839071 = -830756037;    double oTwtemkTBdsyNxbJG69227337 = -616265763;    double oTwtemkTBdsyNxbJG69526231 = -343080976;    double oTwtemkTBdsyNxbJG80824421 = -377879621;    double oTwtemkTBdsyNxbJG8124199 = -191152101;    double oTwtemkTBdsyNxbJG98756519 = -405358604;    double oTwtemkTBdsyNxbJG79426778 = 63344785;    double oTwtemkTBdsyNxbJG99527783 = -641829355;    double oTwtemkTBdsyNxbJG96119458 = -465641554;    double oTwtemkTBdsyNxbJG80760026 = -137173413;    double oTwtemkTBdsyNxbJG81389068 = -575160430;    double oTwtemkTBdsyNxbJG44687493 = -323421570;    double oTwtemkTBdsyNxbJG58157674 = -557141183;    double oTwtemkTBdsyNxbJG95907973 = -752661933;    double oTwtemkTBdsyNxbJG54754209 = -735733067;    double oTwtemkTBdsyNxbJG91995297 = -37010376;    double oTwtemkTBdsyNxbJG4876428 = -872645782;    double oTwtemkTBdsyNxbJG21708245 = -861012779;    double oTwtemkTBdsyNxbJG47038626 = -738496449;    double oTwtemkTBdsyNxbJG68718067 = -65787684;    double oTwtemkTBdsyNxbJG58448158 = -605484081;    double oTwtemkTBdsyNxbJG85187538 = 63196024;    double oTwtemkTBdsyNxbJG60464297 = 91168394;    double oTwtemkTBdsyNxbJG72454353 = -636398107;    double oTwtemkTBdsyNxbJG26504902 = 46701854;    double oTwtemkTBdsyNxbJG8155760 = -11151815;    double oTwtemkTBdsyNxbJG53043749 = -851318430;    double oTwtemkTBdsyNxbJG22489231 = -364242290;    double oTwtemkTBdsyNxbJG51416782 = -47655336;    double oTwtemkTBdsyNxbJG79220712 = -377318908;    double oTwtemkTBdsyNxbJG41133661 = -359929144;    double oTwtemkTBdsyNxbJG61018137 = -210522134;    double oTwtemkTBdsyNxbJG65402648 = -189587228;    double oTwtemkTBdsyNxbJG60831018 = -323053950;    double oTwtemkTBdsyNxbJG60752394 = -819067312;    double oTwtemkTBdsyNxbJG34181267 = -548348339;    double oTwtemkTBdsyNxbJG49585943 = 82931821;    double oTwtemkTBdsyNxbJG72839720 = -23161574;    double oTwtemkTBdsyNxbJG31940264 = -318614099;    double oTwtemkTBdsyNxbJG3087304 = -570258740;    double oTwtemkTBdsyNxbJG34524121 = -361667082;    double oTwtemkTBdsyNxbJG74185450 = -428606322;    double oTwtemkTBdsyNxbJG65157785 = -625421173;    double oTwtemkTBdsyNxbJG13208388 = -783401766;    double oTwtemkTBdsyNxbJG15642368 = -301730095;    double oTwtemkTBdsyNxbJG94670660 = -296977932;    double oTwtemkTBdsyNxbJG32211587 = -865204106;    double oTwtemkTBdsyNxbJG94875490 = 56384140;    double oTwtemkTBdsyNxbJG77129339 = -912576860;    double oTwtemkTBdsyNxbJG15940982 = -231869751;    double oTwtemkTBdsyNxbJG86095016 = -273901908;    double oTwtemkTBdsyNxbJG86435688 = -852168290;    double oTwtemkTBdsyNxbJG76512289 = -730654128;    double oTwtemkTBdsyNxbJG82771986 = -24810079;    double oTwtemkTBdsyNxbJG62701489 = -346530125;    double oTwtemkTBdsyNxbJG58651533 = -793952061;    double oTwtemkTBdsyNxbJG8763041 = -607434158;    double oTwtemkTBdsyNxbJG97071878 = -706682869;    double oTwtemkTBdsyNxbJG54319520 = -324581476;    double oTwtemkTBdsyNxbJG99968439 = -80000287;    double oTwtemkTBdsyNxbJG45712770 = -554040174;    double oTwtemkTBdsyNxbJG56937548 = -572412925;    double oTwtemkTBdsyNxbJG48111002 = -494174019;    double oTwtemkTBdsyNxbJG16898747 = 11677353;    double oTwtemkTBdsyNxbJG39626366 = -777244269;    double oTwtemkTBdsyNxbJG20370932 = -264638297;    double oTwtemkTBdsyNxbJG79284845 = -33834342;    double oTwtemkTBdsyNxbJG97326656 = -134087234;    double oTwtemkTBdsyNxbJG35155579 = -933594622;    double oTwtemkTBdsyNxbJG20572942 = -87384728;    double oTwtemkTBdsyNxbJG42409354 = -19942197;    double oTwtemkTBdsyNxbJG32036707 = -749484208;    double oTwtemkTBdsyNxbJG89767980 = -442398680;    double oTwtemkTBdsyNxbJG43951322 = -68237709;    double oTwtemkTBdsyNxbJG34193946 = -73110212;     oTwtemkTBdsyNxbJG49277594 = oTwtemkTBdsyNxbJG84569052;     oTwtemkTBdsyNxbJG84569052 = oTwtemkTBdsyNxbJG16892702;     oTwtemkTBdsyNxbJG16892702 = oTwtemkTBdsyNxbJG81980665;     oTwtemkTBdsyNxbJG81980665 = oTwtemkTBdsyNxbJG4591466;     oTwtemkTBdsyNxbJG4591466 = oTwtemkTBdsyNxbJG3408605;     oTwtemkTBdsyNxbJG3408605 = oTwtemkTBdsyNxbJG19112175;     oTwtemkTBdsyNxbJG19112175 = oTwtemkTBdsyNxbJG53664142;     oTwtemkTBdsyNxbJG53664142 = oTwtemkTBdsyNxbJG40064463;     oTwtemkTBdsyNxbJG40064463 = oTwtemkTBdsyNxbJG1843824;     oTwtemkTBdsyNxbJG1843824 = oTwtemkTBdsyNxbJG13950900;     oTwtemkTBdsyNxbJG13950900 = oTwtemkTBdsyNxbJG73713234;     oTwtemkTBdsyNxbJG73713234 = oTwtemkTBdsyNxbJG61277243;     oTwtemkTBdsyNxbJG61277243 = oTwtemkTBdsyNxbJG93968413;     oTwtemkTBdsyNxbJG93968413 = oTwtemkTBdsyNxbJG97031435;     oTwtemkTBdsyNxbJG97031435 = oTwtemkTBdsyNxbJG39358153;     oTwtemkTBdsyNxbJG39358153 = oTwtemkTBdsyNxbJG90369260;     oTwtemkTBdsyNxbJG90369260 = oTwtemkTBdsyNxbJG90783463;     oTwtemkTBdsyNxbJG90783463 = oTwtemkTBdsyNxbJG31883548;     oTwtemkTBdsyNxbJG31883548 = oTwtemkTBdsyNxbJG7936280;     oTwtemkTBdsyNxbJG7936280 = oTwtemkTBdsyNxbJG90971443;     oTwtemkTBdsyNxbJG90971443 = oTwtemkTBdsyNxbJG8143933;     oTwtemkTBdsyNxbJG8143933 = oTwtemkTBdsyNxbJG23550915;     oTwtemkTBdsyNxbJG23550915 = oTwtemkTBdsyNxbJG51490053;     oTwtemkTBdsyNxbJG51490053 = oTwtemkTBdsyNxbJG21149647;     oTwtemkTBdsyNxbJG21149647 = oTwtemkTBdsyNxbJG43839071;     oTwtemkTBdsyNxbJG43839071 = oTwtemkTBdsyNxbJG69227337;     oTwtemkTBdsyNxbJG69227337 = oTwtemkTBdsyNxbJG69526231;     oTwtemkTBdsyNxbJG69526231 = oTwtemkTBdsyNxbJG80824421;     oTwtemkTBdsyNxbJG80824421 = oTwtemkTBdsyNxbJG8124199;     oTwtemkTBdsyNxbJG8124199 = oTwtemkTBdsyNxbJG98756519;     oTwtemkTBdsyNxbJG98756519 = oTwtemkTBdsyNxbJG79426778;     oTwtemkTBdsyNxbJG79426778 = oTwtemkTBdsyNxbJG99527783;     oTwtemkTBdsyNxbJG99527783 = oTwtemkTBdsyNxbJG96119458;     oTwtemkTBdsyNxbJG96119458 = oTwtemkTBdsyNxbJG80760026;     oTwtemkTBdsyNxbJG80760026 = oTwtemkTBdsyNxbJG81389068;     oTwtemkTBdsyNxbJG81389068 = oTwtemkTBdsyNxbJG44687493;     oTwtemkTBdsyNxbJG44687493 = oTwtemkTBdsyNxbJG58157674;     oTwtemkTBdsyNxbJG58157674 = oTwtemkTBdsyNxbJG95907973;     oTwtemkTBdsyNxbJG95907973 = oTwtemkTBdsyNxbJG54754209;     oTwtemkTBdsyNxbJG54754209 = oTwtemkTBdsyNxbJG91995297;     oTwtemkTBdsyNxbJG91995297 = oTwtemkTBdsyNxbJG4876428;     oTwtemkTBdsyNxbJG4876428 = oTwtemkTBdsyNxbJG21708245;     oTwtemkTBdsyNxbJG21708245 = oTwtemkTBdsyNxbJG47038626;     oTwtemkTBdsyNxbJG47038626 = oTwtemkTBdsyNxbJG68718067;     oTwtemkTBdsyNxbJG68718067 = oTwtemkTBdsyNxbJG58448158;     oTwtemkTBdsyNxbJG58448158 = oTwtemkTBdsyNxbJG85187538;     oTwtemkTBdsyNxbJG85187538 = oTwtemkTBdsyNxbJG60464297;     oTwtemkTBdsyNxbJG60464297 = oTwtemkTBdsyNxbJG72454353;     oTwtemkTBdsyNxbJG72454353 = oTwtemkTBdsyNxbJG26504902;     oTwtemkTBdsyNxbJG26504902 = oTwtemkTBdsyNxbJG8155760;     oTwtemkTBdsyNxbJG8155760 = oTwtemkTBdsyNxbJG53043749;     oTwtemkTBdsyNxbJG53043749 = oTwtemkTBdsyNxbJG22489231;     oTwtemkTBdsyNxbJG22489231 = oTwtemkTBdsyNxbJG51416782;     oTwtemkTBdsyNxbJG51416782 = oTwtemkTBdsyNxbJG79220712;     oTwtemkTBdsyNxbJG79220712 = oTwtemkTBdsyNxbJG41133661;     oTwtemkTBdsyNxbJG41133661 = oTwtemkTBdsyNxbJG61018137;     oTwtemkTBdsyNxbJG61018137 = oTwtemkTBdsyNxbJG65402648;     oTwtemkTBdsyNxbJG65402648 = oTwtemkTBdsyNxbJG60831018;     oTwtemkTBdsyNxbJG60831018 = oTwtemkTBdsyNxbJG60752394;     oTwtemkTBdsyNxbJG60752394 = oTwtemkTBdsyNxbJG34181267;     oTwtemkTBdsyNxbJG34181267 = oTwtemkTBdsyNxbJG49585943;     oTwtemkTBdsyNxbJG49585943 = oTwtemkTBdsyNxbJG72839720;     oTwtemkTBdsyNxbJG72839720 = oTwtemkTBdsyNxbJG31940264;     oTwtemkTBdsyNxbJG31940264 = oTwtemkTBdsyNxbJG3087304;     oTwtemkTBdsyNxbJG3087304 = oTwtemkTBdsyNxbJG34524121;     oTwtemkTBdsyNxbJG34524121 = oTwtemkTBdsyNxbJG74185450;     oTwtemkTBdsyNxbJG74185450 = oTwtemkTBdsyNxbJG65157785;     oTwtemkTBdsyNxbJG65157785 = oTwtemkTBdsyNxbJG13208388;     oTwtemkTBdsyNxbJG13208388 = oTwtemkTBdsyNxbJG15642368;     oTwtemkTBdsyNxbJG15642368 = oTwtemkTBdsyNxbJG94670660;     oTwtemkTBdsyNxbJG94670660 = oTwtemkTBdsyNxbJG32211587;     oTwtemkTBdsyNxbJG32211587 = oTwtemkTBdsyNxbJG94875490;     oTwtemkTBdsyNxbJG94875490 = oTwtemkTBdsyNxbJG77129339;     oTwtemkTBdsyNxbJG77129339 = oTwtemkTBdsyNxbJG15940982;     oTwtemkTBdsyNxbJG15940982 = oTwtemkTBdsyNxbJG86095016;     oTwtemkTBdsyNxbJG86095016 = oTwtemkTBdsyNxbJG86435688;     oTwtemkTBdsyNxbJG86435688 = oTwtemkTBdsyNxbJG76512289;     oTwtemkTBdsyNxbJG76512289 = oTwtemkTBdsyNxbJG82771986;     oTwtemkTBdsyNxbJG82771986 = oTwtemkTBdsyNxbJG62701489;     oTwtemkTBdsyNxbJG62701489 = oTwtemkTBdsyNxbJG58651533;     oTwtemkTBdsyNxbJG58651533 = oTwtemkTBdsyNxbJG8763041;     oTwtemkTBdsyNxbJG8763041 = oTwtemkTBdsyNxbJG97071878;     oTwtemkTBdsyNxbJG97071878 = oTwtemkTBdsyNxbJG54319520;     oTwtemkTBdsyNxbJG54319520 = oTwtemkTBdsyNxbJG99968439;     oTwtemkTBdsyNxbJG99968439 = oTwtemkTBdsyNxbJG45712770;     oTwtemkTBdsyNxbJG45712770 = oTwtemkTBdsyNxbJG56937548;     oTwtemkTBdsyNxbJG56937548 = oTwtemkTBdsyNxbJG48111002;     oTwtemkTBdsyNxbJG48111002 = oTwtemkTBdsyNxbJG16898747;     oTwtemkTBdsyNxbJG16898747 = oTwtemkTBdsyNxbJG39626366;     oTwtemkTBdsyNxbJG39626366 = oTwtemkTBdsyNxbJG20370932;     oTwtemkTBdsyNxbJG20370932 = oTwtemkTBdsyNxbJG79284845;     oTwtemkTBdsyNxbJG79284845 = oTwtemkTBdsyNxbJG97326656;     oTwtemkTBdsyNxbJG97326656 = oTwtemkTBdsyNxbJG35155579;     oTwtemkTBdsyNxbJG35155579 = oTwtemkTBdsyNxbJG20572942;     oTwtemkTBdsyNxbJG20572942 = oTwtemkTBdsyNxbJG42409354;     oTwtemkTBdsyNxbJG42409354 = oTwtemkTBdsyNxbJG32036707;     oTwtemkTBdsyNxbJG32036707 = oTwtemkTBdsyNxbJG89767980;     oTwtemkTBdsyNxbJG89767980 = oTwtemkTBdsyNxbJG43951322;     oTwtemkTBdsyNxbJG43951322 = oTwtemkTBdsyNxbJG34193946;     oTwtemkTBdsyNxbJG34193946 = oTwtemkTBdsyNxbJG49277594;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hbVYsAWrexkJxjXu37483755() {     double wyyoVbrIWiPpVUSrC84736562 = -539387112;    double wyyoVbrIWiPpVUSrC97841130 = -660224316;    double wyyoVbrIWiPpVUSrC54279178 = -145852065;    double wyyoVbrIWiPpVUSrC54204020 = -196219226;    double wyyoVbrIWiPpVUSrC60876735 = -755422194;    double wyyoVbrIWiPpVUSrC39631477 = -426961997;    double wyyoVbrIWiPpVUSrC29769581 = -204874889;    double wyyoVbrIWiPpVUSrC15795703 = -145275336;    double wyyoVbrIWiPpVUSrC41582068 = 1902393;    double wyyoVbrIWiPpVUSrC16746536 = -740304342;    double wyyoVbrIWiPpVUSrC52132635 = -200330026;    double wyyoVbrIWiPpVUSrC74281382 = 65443552;    double wyyoVbrIWiPpVUSrC99511888 = -643075231;    double wyyoVbrIWiPpVUSrC47109277 = -996938588;    double wyyoVbrIWiPpVUSrC55225952 = -380315539;    double wyyoVbrIWiPpVUSrC22899454 = -356211108;    double wyyoVbrIWiPpVUSrC46205196 = -949319841;    double wyyoVbrIWiPpVUSrC90543655 = -869705861;    double wyyoVbrIWiPpVUSrC32433878 = -964873348;    double wyyoVbrIWiPpVUSrC1773498 = -795441192;    double wyyoVbrIWiPpVUSrC4058307 = -903822378;    double wyyoVbrIWiPpVUSrC11565458 = -799817363;    double wyyoVbrIWiPpVUSrC94076294 = -106782760;    double wyyoVbrIWiPpVUSrC45282999 = -701422367;    double wyyoVbrIWiPpVUSrC29167598 = 24000596;    double wyyoVbrIWiPpVUSrC54537243 = -519332843;    double wyyoVbrIWiPpVUSrC29647912 = -422549904;    double wyyoVbrIWiPpVUSrC10254657 = -512853151;    double wyyoVbrIWiPpVUSrC27492636 = -951974424;    double wyyoVbrIWiPpVUSrC13657610 = -225659272;    double wyyoVbrIWiPpVUSrC17339751 = -957339091;    double wyyoVbrIWiPpVUSrC17249797 = -809626630;    double wyyoVbrIWiPpVUSrC11745795 = -789425332;    double wyyoVbrIWiPpVUSrC25867885 = -530335367;    double wyyoVbrIWiPpVUSrC46568480 = -88273664;    double wyyoVbrIWiPpVUSrC32949912 = -789725736;    double wyyoVbrIWiPpVUSrC94099309 = -188117610;    double wyyoVbrIWiPpVUSrC9373078 = -539796764;    double wyyoVbrIWiPpVUSrC26298535 = -403538210;    double wyyoVbrIWiPpVUSrC19025741 = -200995613;    double wyyoVbrIWiPpVUSrC25446906 = -906209290;    double wyyoVbrIWiPpVUSrC94241554 = -467191073;    double wyyoVbrIWiPpVUSrC11091623 = -414193565;    double wyyoVbrIWiPpVUSrC87169963 = -242828859;    double wyyoVbrIWiPpVUSrC68734526 = -992298845;    double wyyoVbrIWiPpVUSrC4409487 = -644407713;    double wyyoVbrIWiPpVUSrC4532703 = -420790336;    double wyyoVbrIWiPpVUSrC78190609 = -75358199;    double wyyoVbrIWiPpVUSrC95347019 = -300134988;    double wyyoVbrIWiPpVUSrC51034491 = -444324961;    double wyyoVbrIWiPpVUSrC88926483 = -985428372;    double wyyoVbrIWiPpVUSrC36529222 = 35029223;    double wyyoVbrIWiPpVUSrC91110379 = -120287844;    double wyyoVbrIWiPpVUSrC67408628 = -239163836;    double wyyoVbrIWiPpVUSrC33425448 = -723809054;    double wyyoVbrIWiPpVUSrC73171105 = -739569750;    double wyyoVbrIWiPpVUSrC3764836 = -453441556;    double wyyoVbrIWiPpVUSrC8996180 = -444429699;    double wyyoVbrIWiPpVUSrC25036422 = -120219823;    double wyyoVbrIWiPpVUSrC6339492 = -136089351;    double wyyoVbrIWiPpVUSrC9983566 = 95587906;    double wyyoVbrIWiPpVUSrC19514925 = -692021739;    double wyyoVbrIWiPpVUSrC88303067 = -193300912;    double wyyoVbrIWiPpVUSrC27924459 = -772438336;    double wyyoVbrIWiPpVUSrC99406784 = -782965251;    double wyyoVbrIWiPpVUSrC34882838 = -390703397;    double wyyoVbrIWiPpVUSrC62535588 = -145131116;    double wyyoVbrIWiPpVUSrC73644003 = -12739864;    double wyyoVbrIWiPpVUSrC540797 = -808664925;    double wyyoVbrIWiPpVUSrC22276040 = -590589804;    double wyyoVbrIWiPpVUSrC28800145 = -68093499;    double wyyoVbrIWiPpVUSrC36832119 = -309523077;    double wyyoVbrIWiPpVUSrC64245120 = -366167652;    double wyyoVbrIWiPpVUSrC13408137 = -663877736;    double wyyoVbrIWiPpVUSrC76326592 = -889231903;    double wyyoVbrIWiPpVUSrC9816752 = -336631306;    double wyyoVbrIWiPpVUSrC473835 = -285623799;    double wyyoVbrIWiPpVUSrC6906331 = -863953902;    double wyyoVbrIWiPpVUSrC76548473 = -709123522;    double wyyoVbrIWiPpVUSrC24758112 = -331591691;    double wyyoVbrIWiPpVUSrC50004541 = 1457492;    double wyyoVbrIWiPpVUSrC51457302 = -247191705;    double wyyoVbrIWiPpVUSrC14907638 = -112718164;    double wyyoVbrIWiPpVUSrC76458145 = -407649463;    double wyyoVbrIWiPpVUSrC24731127 = -240230900;    double wyyoVbrIWiPpVUSrC80810529 = -892368315;    double wyyoVbrIWiPpVUSrC26139418 = -589338786;    double wyyoVbrIWiPpVUSrC44337166 = -450261496;    double wyyoVbrIWiPpVUSrC92442436 = -806526314;    double wyyoVbrIWiPpVUSrC73397375 = -348703914;    double wyyoVbrIWiPpVUSrC29185077 = -236284180;    double wyyoVbrIWiPpVUSrC85103130 = -743687911;    double wyyoVbrIWiPpVUSrC84336655 = -319576942;    double wyyoVbrIWiPpVUSrC19959043 = -167448859;    double wyyoVbrIWiPpVUSrC9042176 = -196583520;    double wyyoVbrIWiPpVUSrC5931981 = -114187551;    double wyyoVbrIWiPpVUSrC5938488 = -173890161;    double wyyoVbrIWiPpVUSrC83167164 = -641755230;    double wyyoVbrIWiPpVUSrC87763179 = -459863608;    double wyyoVbrIWiPpVUSrC33851688 = -539387112;     wyyoVbrIWiPpVUSrC84736562 = wyyoVbrIWiPpVUSrC97841130;     wyyoVbrIWiPpVUSrC97841130 = wyyoVbrIWiPpVUSrC54279178;     wyyoVbrIWiPpVUSrC54279178 = wyyoVbrIWiPpVUSrC54204020;     wyyoVbrIWiPpVUSrC54204020 = wyyoVbrIWiPpVUSrC60876735;     wyyoVbrIWiPpVUSrC60876735 = wyyoVbrIWiPpVUSrC39631477;     wyyoVbrIWiPpVUSrC39631477 = wyyoVbrIWiPpVUSrC29769581;     wyyoVbrIWiPpVUSrC29769581 = wyyoVbrIWiPpVUSrC15795703;     wyyoVbrIWiPpVUSrC15795703 = wyyoVbrIWiPpVUSrC41582068;     wyyoVbrIWiPpVUSrC41582068 = wyyoVbrIWiPpVUSrC16746536;     wyyoVbrIWiPpVUSrC16746536 = wyyoVbrIWiPpVUSrC52132635;     wyyoVbrIWiPpVUSrC52132635 = wyyoVbrIWiPpVUSrC74281382;     wyyoVbrIWiPpVUSrC74281382 = wyyoVbrIWiPpVUSrC99511888;     wyyoVbrIWiPpVUSrC99511888 = wyyoVbrIWiPpVUSrC47109277;     wyyoVbrIWiPpVUSrC47109277 = wyyoVbrIWiPpVUSrC55225952;     wyyoVbrIWiPpVUSrC55225952 = wyyoVbrIWiPpVUSrC22899454;     wyyoVbrIWiPpVUSrC22899454 = wyyoVbrIWiPpVUSrC46205196;     wyyoVbrIWiPpVUSrC46205196 = wyyoVbrIWiPpVUSrC90543655;     wyyoVbrIWiPpVUSrC90543655 = wyyoVbrIWiPpVUSrC32433878;     wyyoVbrIWiPpVUSrC32433878 = wyyoVbrIWiPpVUSrC1773498;     wyyoVbrIWiPpVUSrC1773498 = wyyoVbrIWiPpVUSrC4058307;     wyyoVbrIWiPpVUSrC4058307 = wyyoVbrIWiPpVUSrC11565458;     wyyoVbrIWiPpVUSrC11565458 = wyyoVbrIWiPpVUSrC94076294;     wyyoVbrIWiPpVUSrC94076294 = wyyoVbrIWiPpVUSrC45282999;     wyyoVbrIWiPpVUSrC45282999 = wyyoVbrIWiPpVUSrC29167598;     wyyoVbrIWiPpVUSrC29167598 = wyyoVbrIWiPpVUSrC54537243;     wyyoVbrIWiPpVUSrC54537243 = wyyoVbrIWiPpVUSrC29647912;     wyyoVbrIWiPpVUSrC29647912 = wyyoVbrIWiPpVUSrC10254657;     wyyoVbrIWiPpVUSrC10254657 = wyyoVbrIWiPpVUSrC27492636;     wyyoVbrIWiPpVUSrC27492636 = wyyoVbrIWiPpVUSrC13657610;     wyyoVbrIWiPpVUSrC13657610 = wyyoVbrIWiPpVUSrC17339751;     wyyoVbrIWiPpVUSrC17339751 = wyyoVbrIWiPpVUSrC17249797;     wyyoVbrIWiPpVUSrC17249797 = wyyoVbrIWiPpVUSrC11745795;     wyyoVbrIWiPpVUSrC11745795 = wyyoVbrIWiPpVUSrC25867885;     wyyoVbrIWiPpVUSrC25867885 = wyyoVbrIWiPpVUSrC46568480;     wyyoVbrIWiPpVUSrC46568480 = wyyoVbrIWiPpVUSrC32949912;     wyyoVbrIWiPpVUSrC32949912 = wyyoVbrIWiPpVUSrC94099309;     wyyoVbrIWiPpVUSrC94099309 = wyyoVbrIWiPpVUSrC9373078;     wyyoVbrIWiPpVUSrC9373078 = wyyoVbrIWiPpVUSrC26298535;     wyyoVbrIWiPpVUSrC26298535 = wyyoVbrIWiPpVUSrC19025741;     wyyoVbrIWiPpVUSrC19025741 = wyyoVbrIWiPpVUSrC25446906;     wyyoVbrIWiPpVUSrC25446906 = wyyoVbrIWiPpVUSrC94241554;     wyyoVbrIWiPpVUSrC94241554 = wyyoVbrIWiPpVUSrC11091623;     wyyoVbrIWiPpVUSrC11091623 = wyyoVbrIWiPpVUSrC87169963;     wyyoVbrIWiPpVUSrC87169963 = wyyoVbrIWiPpVUSrC68734526;     wyyoVbrIWiPpVUSrC68734526 = wyyoVbrIWiPpVUSrC4409487;     wyyoVbrIWiPpVUSrC4409487 = wyyoVbrIWiPpVUSrC4532703;     wyyoVbrIWiPpVUSrC4532703 = wyyoVbrIWiPpVUSrC78190609;     wyyoVbrIWiPpVUSrC78190609 = wyyoVbrIWiPpVUSrC95347019;     wyyoVbrIWiPpVUSrC95347019 = wyyoVbrIWiPpVUSrC51034491;     wyyoVbrIWiPpVUSrC51034491 = wyyoVbrIWiPpVUSrC88926483;     wyyoVbrIWiPpVUSrC88926483 = wyyoVbrIWiPpVUSrC36529222;     wyyoVbrIWiPpVUSrC36529222 = wyyoVbrIWiPpVUSrC91110379;     wyyoVbrIWiPpVUSrC91110379 = wyyoVbrIWiPpVUSrC67408628;     wyyoVbrIWiPpVUSrC67408628 = wyyoVbrIWiPpVUSrC33425448;     wyyoVbrIWiPpVUSrC33425448 = wyyoVbrIWiPpVUSrC73171105;     wyyoVbrIWiPpVUSrC73171105 = wyyoVbrIWiPpVUSrC3764836;     wyyoVbrIWiPpVUSrC3764836 = wyyoVbrIWiPpVUSrC8996180;     wyyoVbrIWiPpVUSrC8996180 = wyyoVbrIWiPpVUSrC25036422;     wyyoVbrIWiPpVUSrC25036422 = wyyoVbrIWiPpVUSrC6339492;     wyyoVbrIWiPpVUSrC6339492 = wyyoVbrIWiPpVUSrC9983566;     wyyoVbrIWiPpVUSrC9983566 = wyyoVbrIWiPpVUSrC19514925;     wyyoVbrIWiPpVUSrC19514925 = wyyoVbrIWiPpVUSrC88303067;     wyyoVbrIWiPpVUSrC88303067 = wyyoVbrIWiPpVUSrC27924459;     wyyoVbrIWiPpVUSrC27924459 = wyyoVbrIWiPpVUSrC99406784;     wyyoVbrIWiPpVUSrC99406784 = wyyoVbrIWiPpVUSrC34882838;     wyyoVbrIWiPpVUSrC34882838 = wyyoVbrIWiPpVUSrC62535588;     wyyoVbrIWiPpVUSrC62535588 = wyyoVbrIWiPpVUSrC73644003;     wyyoVbrIWiPpVUSrC73644003 = wyyoVbrIWiPpVUSrC540797;     wyyoVbrIWiPpVUSrC540797 = wyyoVbrIWiPpVUSrC22276040;     wyyoVbrIWiPpVUSrC22276040 = wyyoVbrIWiPpVUSrC28800145;     wyyoVbrIWiPpVUSrC28800145 = wyyoVbrIWiPpVUSrC36832119;     wyyoVbrIWiPpVUSrC36832119 = wyyoVbrIWiPpVUSrC64245120;     wyyoVbrIWiPpVUSrC64245120 = wyyoVbrIWiPpVUSrC13408137;     wyyoVbrIWiPpVUSrC13408137 = wyyoVbrIWiPpVUSrC76326592;     wyyoVbrIWiPpVUSrC76326592 = wyyoVbrIWiPpVUSrC9816752;     wyyoVbrIWiPpVUSrC9816752 = wyyoVbrIWiPpVUSrC473835;     wyyoVbrIWiPpVUSrC473835 = wyyoVbrIWiPpVUSrC6906331;     wyyoVbrIWiPpVUSrC6906331 = wyyoVbrIWiPpVUSrC76548473;     wyyoVbrIWiPpVUSrC76548473 = wyyoVbrIWiPpVUSrC24758112;     wyyoVbrIWiPpVUSrC24758112 = wyyoVbrIWiPpVUSrC50004541;     wyyoVbrIWiPpVUSrC50004541 = wyyoVbrIWiPpVUSrC51457302;     wyyoVbrIWiPpVUSrC51457302 = wyyoVbrIWiPpVUSrC14907638;     wyyoVbrIWiPpVUSrC14907638 = wyyoVbrIWiPpVUSrC76458145;     wyyoVbrIWiPpVUSrC76458145 = wyyoVbrIWiPpVUSrC24731127;     wyyoVbrIWiPpVUSrC24731127 = wyyoVbrIWiPpVUSrC80810529;     wyyoVbrIWiPpVUSrC80810529 = wyyoVbrIWiPpVUSrC26139418;     wyyoVbrIWiPpVUSrC26139418 = wyyoVbrIWiPpVUSrC44337166;     wyyoVbrIWiPpVUSrC44337166 = wyyoVbrIWiPpVUSrC92442436;     wyyoVbrIWiPpVUSrC92442436 = wyyoVbrIWiPpVUSrC73397375;     wyyoVbrIWiPpVUSrC73397375 = wyyoVbrIWiPpVUSrC29185077;     wyyoVbrIWiPpVUSrC29185077 = wyyoVbrIWiPpVUSrC85103130;     wyyoVbrIWiPpVUSrC85103130 = wyyoVbrIWiPpVUSrC84336655;     wyyoVbrIWiPpVUSrC84336655 = wyyoVbrIWiPpVUSrC19959043;     wyyoVbrIWiPpVUSrC19959043 = wyyoVbrIWiPpVUSrC9042176;     wyyoVbrIWiPpVUSrC9042176 = wyyoVbrIWiPpVUSrC5931981;     wyyoVbrIWiPpVUSrC5931981 = wyyoVbrIWiPpVUSrC5938488;     wyyoVbrIWiPpVUSrC5938488 = wyyoVbrIWiPpVUSrC83167164;     wyyoVbrIWiPpVUSrC83167164 = wyyoVbrIWiPpVUSrC87763179;     wyyoVbrIWiPpVUSrC87763179 = wyyoVbrIWiPpVUSrC33851688;     wyyoVbrIWiPpVUSrC33851688 = wyyoVbrIWiPpVUSrC84736562;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void HcuTKTGzRABIJjum52532823() {     double XMNgFMRfQsgKDioee90853668 = -651289001;    double XMNgFMRfQsgKDioee41214410 = -369824097;    double XMNgFMRfQsgKDioee70321361 = -296684986;    double XMNgFMRfQsgKDioee81278800 = -386967856;    double XMNgFMRfQsgKDioee29772224 = -176072065;    double XMNgFMRfQsgKDioee31048679 = -691458232;    double XMNgFMRfQsgKDioee90258547 = -343839866;    double XMNgFMRfQsgKDioee65555594 = -432149249;    double XMNgFMRfQsgKDioee6224564 = -866130272;    double XMNgFMRfQsgKDioee57976194 = 30720181;    double XMNgFMRfQsgKDioee12242925 = -77767613;    double XMNgFMRfQsgKDioee6104425 = -378962650;    double XMNgFMRfQsgKDioee85721007 = 17033680;    double XMNgFMRfQsgKDioee30252954 = -429177156;    double XMNgFMRfQsgKDioee50968379 = 29222294;    double XMNgFMRfQsgKDioee64404848 = -329210651;    double XMNgFMRfQsgKDioee40041200 = -465864954;    double XMNgFMRfQsgKDioee87596808 = -535228504;    double XMNgFMRfQsgKDioee94263719 = -270642445;    double XMNgFMRfQsgKDioee82323662 = -52344112;    double XMNgFMRfQsgKDioee92611499 = -759309834;    double XMNgFMRfQsgKDioee98086481 = -707747079;    double XMNgFMRfQsgKDioee29667087 = -953300247;    double XMNgFMRfQsgKDioee29275373 = -923727354;    double XMNgFMRfQsgKDioee72867248 = -349195436;    double XMNgFMRfQsgKDioee79258918 = -397988167;    double XMNgFMRfQsgKDioee28107690 = -323212107;    double XMNgFMRfQsgKDioee23361782 = -578782104;    double XMNgFMRfQsgKDioee93624891 = -74357128;    double XMNgFMRfQsgKDioee82273246 = -43130139;    double XMNgFMRfQsgKDioee9582300 = -870660029;    double XMNgFMRfQsgKDioee14860419 = -519073445;    double XMNgFMRfQsgKDioee21456743 = -682038702;    double XMNgFMRfQsgKDioee20698449 = -281877133;    double XMNgFMRfQsgKDioee8043133 = -727006769;    double XMNgFMRfQsgKDioee1675826 = 96248156;    double XMNgFMRfQsgKDioee75361244 = -141753480;    double XMNgFMRfQsgKDioee43463290 = -311302774;    double XMNgFMRfQsgKDioee99908788 = -397461706;    double XMNgFMRfQsgKDioee6527679 = -674434836;    double XMNgFMRfQsgKDioee31116129 = -971483184;    double XMNgFMRfQsgKDioee26392012 = -737333351;    double XMNgFMRfQsgKDioee89158601 = -494998149;    double XMNgFMRfQsgKDioee74577704 = -262171797;    double XMNgFMRfQsgKDioee80686991 = -654197000;    double XMNgFMRfQsgKDioee16348682 = -629623830;    double XMNgFMRfQsgKDioee22311277 = -539565874;    double XMNgFMRfQsgKDioee86147550 = -85253732;    double XMNgFMRfQsgKDioee5022749 = -385871976;    double XMNgFMRfQsgKDioee63946143 = -531335770;    double XMNgFMRfQsgKDioee24627162 = -827722043;    double XMNgFMRfQsgKDioee47338941 = -865697097;    double XMNgFMRfQsgKDioee39358837 = 87922568;    double XMNgFMRfQsgKDioee63377508 = -559633981;    double XMNgFMRfQsgKDioee59921323 = -324340836;    double XMNgFMRfQsgKDioee92767187 = -943541922;    double XMNgFMRfQsgKDioee11547324 = -416523850;    double XMNgFMRfQsgKDioee41045989 = -372957633;    double XMNgFMRfQsgKDioee8411553 = 62227580;    double XMNgFMRfQsgKDioee50513305 = -778083899;    double XMNgFMRfQsgKDioee2940990 = -268246126;    double XMNgFMRfQsgKDioee66896765 = -765057762;    double XMNgFMRfQsgKDioee71930702 = -257792121;    double XMNgFMRfQsgKDioee23951317 = -723000134;    double XMNgFMRfQsgKDioee48393894 = -98619791;    double XMNgFMRfQsgKDioee97382506 = -558694168;    double XMNgFMRfQsgKDioee84647682 = -696923948;    double XMNgFMRfQsgKDioee65022558 = -701089188;    double XMNgFMRfQsgKDioee22209822 = -702170388;    double XMNgFMRfQsgKDioee49292553 = 32974137;    double XMNgFMRfQsgKDioee89043603 = -87457172;    double XMNgFMRfQsgKDioee96577909 = -54562181;    double XMNgFMRfQsgKDioee87688019 = -37766798;    double XMNgFMRfQsgKDioee87736041 = -596207610;    double XMNgFMRfQsgKDioee51207533 = -80860928;    double XMNgFMRfQsgKDioee66219488 = 78023517;    double XMNgFMRfQsgKDioee8927880 = -112748931;    double XMNgFMRfQsgKDioee55089382 = -591128451;    double XMNgFMRfQsgKDioee48588382 = -169530355;    double XMNgFMRfQsgKDioee56518566 = -719571607;    double XMNgFMRfQsgKDioee56947642 = -858422293;    double XMNgFMRfQsgKDioee41960140 = -137958376;    double XMNgFMRfQsgKDioee18339034 = -92910129;    double XMNgFMRfQsgKDioee29678748 = -543021359;    double XMNgFMRfQsgKDioee57646085 = -215408096;    double XMNgFMRfQsgKDioee62243359 = 95037068;    double XMNgFMRfQsgKDioee75501582 = -506996014;    double XMNgFMRfQsgKDioee58079234 = -22404722;    double XMNgFMRfQsgKDioee60777126 = -957536297;    double XMNgFMRfQsgKDioee15275945 = -783464847;    double XMNgFMRfQsgKDioee90128502 = -487227994;    double XMNgFMRfQsgKDioee34315255 = -768795848;    double XMNgFMRfQsgKDioee35051738 = -273530354;    double XMNgFMRfQsgKDioee49395484 = -619377808;    double XMNgFMRfQsgKDioee3586690 = -306188710;    double XMNgFMRfQsgKDioee64219364 = -106425422;    double XMNgFMRfQsgKDioee54461309 = -379541231;    double XMNgFMRfQsgKDioee65207284 = -771998015;    double XMNgFMRfQsgKDioee26183810 = -63552006;    double XMNgFMRfQsgKDioee83304484 = -651289001;     XMNgFMRfQsgKDioee90853668 = XMNgFMRfQsgKDioee41214410;     XMNgFMRfQsgKDioee41214410 = XMNgFMRfQsgKDioee70321361;     XMNgFMRfQsgKDioee70321361 = XMNgFMRfQsgKDioee81278800;     XMNgFMRfQsgKDioee81278800 = XMNgFMRfQsgKDioee29772224;     XMNgFMRfQsgKDioee29772224 = XMNgFMRfQsgKDioee31048679;     XMNgFMRfQsgKDioee31048679 = XMNgFMRfQsgKDioee90258547;     XMNgFMRfQsgKDioee90258547 = XMNgFMRfQsgKDioee65555594;     XMNgFMRfQsgKDioee65555594 = XMNgFMRfQsgKDioee6224564;     XMNgFMRfQsgKDioee6224564 = XMNgFMRfQsgKDioee57976194;     XMNgFMRfQsgKDioee57976194 = XMNgFMRfQsgKDioee12242925;     XMNgFMRfQsgKDioee12242925 = XMNgFMRfQsgKDioee6104425;     XMNgFMRfQsgKDioee6104425 = XMNgFMRfQsgKDioee85721007;     XMNgFMRfQsgKDioee85721007 = XMNgFMRfQsgKDioee30252954;     XMNgFMRfQsgKDioee30252954 = XMNgFMRfQsgKDioee50968379;     XMNgFMRfQsgKDioee50968379 = XMNgFMRfQsgKDioee64404848;     XMNgFMRfQsgKDioee64404848 = XMNgFMRfQsgKDioee40041200;     XMNgFMRfQsgKDioee40041200 = XMNgFMRfQsgKDioee87596808;     XMNgFMRfQsgKDioee87596808 = XMNgFMRfQsgKDioee94263719;     XMNgFMRfQsgKDioee94263719 = XMNgFMRfQsgKDioee82323662;     XMNgFMRfQsgKDioee82323662 = XMNgFMRfQsgKDioee92611499;     XMNgFMRfQsgKDioee92611499 = XMNgFMRfQsgKDioee98086481;     XMNgFMRfQsgKDioee98086481 = XMNgFMRfQsgKDioee29667087;     XMNgFMRfQsgKDioee29667087 = XMNgFMRfQsgKDioee29275373;     XMNgFMRfQsgKDioee29275373 = XMNgFMRfQsgKDioee72867248;     XMNgFMRfQsgKDioee72867248 = XMNgFMRfQsgKDioee79258918;     XMNgFMRfQsgKDioee79258918 = XMNgFMRfQsgKDioee28107690;     XMNgFMRfQsgKDioee28107690 = XMNgFMRfQsgKDioee23361782;     XMNgFMRfQsgKDioee23361782 = XMNgFMRfQsgKDioee93624891;     XMNgFMRfQsgKDioee93624891 = XMNgFMRfQsgKDioee82273246;     XMNgFMRfQsgKDioee82273246 = XMNgFMRfQsgKDioee9582300;     XMNgFMRfQsgKDioee9582300 = XMNgFMRfQsgKDioee14860419;     XMNgFMRfQsgKDioee14860419 = XMNgFMRfQsgKDioee21456743;     XMNgFMRfQsgKDioee21456743 = XMNgFMRfQsgKDioee20698449;     XMNgFMRfQsgKDioee20698449 = XMNgFMRfQsgKDioee8043133;     XMNgFMRfQsgKDioee8043133 = XMNgFMRfQsgKDioee1675826;     XMNgFMRfQsgKDioee1675826 = XMNgFMRfQsgKDioee75361244;     XMNgFMRfQsgKDioee75361244 = XMNgFMRfQsgKDioee43463290;     XMNgFMRfQsgKDioee43463290 = XMNgFMRfQsgKDioee99908788;     XMNgFMRfQsgKDioee99908788 = XMNgFMRfQsgKDioee6527679;     XMNgFMRfQsgKDioee6527679 = XMNgFMRfQsgKDioee31116129;     XMNgFMRfQsgKDioee31116129 = XMNgFMRfQsgKDioee26392012;     XMNgFMRfQsgKDioee26392012 = XMNgFMRfQsgKDioee89158601;     XMNgFMRfQsgKDioee89158601 = XMNgFMRfQsgKDioee74577704;     XMNgFMRfQsgKDioee74577704 = XMNgFMRfQsgKDioee80686991;     XMNgFMRfQsgKDioee80686991 = XMNgFMRfQsgKDioee16348682;     XMNgFMRfQsgKDioee16348682 = XMNgFMRfQsgKDioee22311277;     XMNgFMRfQsgKDioee22311277 = XMNgFMRfQsgKDioee86147550;     XMNgFMRfQsgKDioee86147550 = XMNgFMRfQsgKDioee5022749;     XMNgFMRfQsgKDioee5022749 = XMNgFMRfQsgKDioee63946143;     XMNgFMRfQsgKDioee63946143 = XMNgFMRfQsgKDioee24627162;     XMNgFMRfQsgKDioee24627162 = XMNgFMRfQsgKDioee47338941;     XMNgFMRfQsgKDioee47338941 = XMNgFMRfQsgKDioee39358837;     XMNgFMRfQsgKDioee39358837 = XMNgFMRfQsgKDioee63377508;     XMNgFMRfQsgKDioee63377508 = XMNgFMRfQsgKDioee59921323;     XMNgFMRfQsgKDioee59921323 = XMNgFMRfQsgKDioee92767187;     XMNgFMRfQsgKDioee92767187 = XMNgFMRfQsgKDioee11547324;     XMNgFMRfQsgKDioee11547324 = XMNgFMRfQsgKDioee41045989;     XMNgFMRfQsgKDioee41045989 = XMNgFMRfQsgKDioee8411553;     XMNgFMRfQsgKDioee8411553 = XMNgFMRfQsgKDioee50513305;     XMNgFMRfQsgKDioee50513305 = XMNgFMRfQsgKDioee2940990;     XMNgFMRfQsgKDioee2940990 = XMNgFMRfQsgKDioee66896765;     XMNgFMRfQsgKDioee66896765 = XMNgFMRfQsgKDioee71930702;     XMNgFMRfQsgKDioee71930702 = XMNgFMRfQsgKDioee23951317;     XMNgFMRfQsgKDioee23951317 = XMNgFMRfQsgKDioee48393894;     XMNgFMRfQsgKDioee48393894 = XMNgFMRfQsgKDioee97382506;     XMNgFMRfQsgKDioee97382506 = XMNgFMRfQsgKDioee84647682;     XMNgFMRfQsgKDioee84647682 = XMNgFMRfQsgKDioee65022558;     XMNgFMRfQsgKDioee65022558 = XMNgFMRfQsgKDioee22209822;     XMNgFMRfQsgKDioee22209822 = XMNgFMRfQsgKDioee49292553;     XMNgFMRfQsgKDioee49292553 = XMNgFMRfQsgKDioee89043603;     XMNgFMRfQsgKDioee89043603 = XMNgFMRfQsgKDioee96577909;     XMNgFMRfQsgKDioee96577909 = XMNgFMRfQsgKDioee87688019;     XMNgFMRfQsgKDioee87688019 = XMNgFMRfQsgKDioee87736041;     XMNgFMRfQsgKDioee87736041 = XMNgFMRfQsgKDioee51207533;     XMNgFMRfQsgKDioee51207533 = XMNgFMRfQsgKDioee66219488;     XMNgFMRfQsgKDioee66219488 = XMNgFMRfQsgKDioee8927880;     XMNgFMRfQsgKDioee8927880 = XMNgFMRfQsgKDioee55089382;     XMNgFMRfQsgKDioee55089382 = XMNgFMRfQsgKDioee48588382;     XMNgFMRfQsgKDioee48588382 = XMNgFMRfQsgKDioee56518566;     XMNgFMRfQsgKDioee56518566 = XMNgFMRfQsgKDioee56947642;     XMNgFMRfQsgKDioee56947642 = XMNgFMRfQsgKDioee41960140;     XMNgFMRfQsgKDioee41960140 = XMNgFMRfQsgKDioee18339034;     XMNgFMRfQsgKDioee18339034 = XMNgFMRfQsgKDioee29678748;     XMNgFMRfQsgKDioee29678748 = XMNgFMRfQsgKDioee57646085;     XMNgFMRfQsgKDioee57646085 = XMNgFMRfQsgKDioee62243359;     XMNgFMRfQsgKDioee62243359 = XMNgFMRfQsgKDioee75501582;     XMNgFMRfQsgKDioee75501582 = XMNgFMRfQsgKDioee58079234;     XMNgFMRfQsgKDioee58079234 = XMNgFMRfQsgKDioee60777126;     XMNgFMRfQsgKDioee60777126 = XMNgFMRfQsgKDioee15275945;     XMNgFMRfQsgKDioee15275945 = XMNgFMRfQsgKDioee90128502;     XMNgFMRfQsgKDioee90128502 = XMNgFMRfQsgKDioee34315255;     XMNgFMRfQsgKDioee34315255 = XMNgFMRfQsgKDioee35051738;     XMNgFMRfQsgKDioee35051738 = XMNgFMRfQsgKDioee49395484;     XMNgFMRfQsgKDioee49395484 = XMNgFMRfQsgKDioee3586690;     XMNgFMRfQsgKDioee3586690 = XMNgFMRfQsgKDioee64219364;     XMNgFMRfQsgKDioee64219364 = XMNgFMRfQsgKDioee54461309;     XMNgFMRfQsgKDioee54461309 = XMNgFMRfQsgKDioee65207284;     XMNgFMRfQsgKDioee65207284 = XMNgFMRfQsgKDioee26183810;     XMNgFMRfQsgKDioee26183810 = XMNgFMRfQsgKDioee83304484;     XMNgFMRfQsgKDioee83304484 = XMNgFMRfQsgKDioee90853668;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void CUxzdnFwrHoTWGwn50140849() {     double DmPzOhmPNWffjMDFD94045357 = -314442280;    double DmPzOhmPNWffjMDFD91836369 = -719353944;    double DmPzOhmPNWffjMDFD75744470 = -916621292;    double DmPzOhmPNWffjMDFD97073902 = -208424048;    double DmPzOhmPNWffjMDFD9780077 = -100292672;    double DmPzOhmPNWffjMDFD67980271 = -685263670;    double DmPzOhmPNWffjMDFD71970988 = -808660234;    double DmPzOhmPNWffjMDFD74145910 = -46411801;    double DmPzOhmPNWffjMDFD17130889 = -91804348;    double DmPzOhmPNWffjMDFD82010506 = -197906881;    double DmPzOhmPNWffjMDFD9355225 = -351043170;    double DmPzOhmPNWffjMDFD99810809 = -307404626;    double DmPzOhmPNWffjMDFD27146518 = -250155840;    double DmPzOhmPNWffjMDFD33042139 = -221145744;    double DmPzOhmPNWffjMDFD15674099 = -194558802;    double DmPzOhmPNWffjMDFD2434526 = -716530237;    double DmPzOhmPNWffjMDFD45868724 = -796519498;    double DmPzOhmPNWffjMDFD25736830 = -996834796;    double DmPzOhmPNWffjMDFD34875080 = -436496444;    double DmPzOhmPNWffjMDFD37042609 = -386173485;    double DmPzOhmPNWffjMDFD72407082 = -939988680;    double DmPzOhmPNWffjMDFD14070636 = -32705579;    double DmPzOhmPNWffjMDFD67015958 = -180269602;    double DmPzOhmPNWffjMDFD48139736 = -99503999;    double DmPzOhmPNWffjMDFD20672831 = -721826307;    double DmPzOhmPNWffjMDFD37019138 = -526750413;    double DmPzOhmPNWffjMDFD33627760 = -432238462;    double DmPzOhmPNWffjMDFD16752657 = -528956872;    double DmPzOhmPNWffjMDFD7287990 = -612464174;    double DmPzOhmPNWffjMDFD2528428 = -807175208;    double DmPzOhmPNWffjMDFD20750808 = -955084142;    double DmPzOhmPNWffjMDFD79233017 = -532619686;    double DmPzOhmPNWffjMDFD47057548 = -802460560;    double DmPzOhmPNWffjMDFD15179670 = -990576987;    double DmPzOhmPNWffjMDFD15576794 = -524000183;    double DmPzOhmPNWffjMDFD7602997 = -185716929;    double DmPzOhmPNWffjMDFD43508285 = -988970851;    double DmPzOhmPNWffjMDFD35327555 = -287654806;    double DmPzOhmPNWffjMDFD27837833 = -67435413;    double DmPzOhmPNWffjMDFD66998530 = -617612812;    double DmPzOhmPNWffjMDFD37365707 = -31309606;    double DmPzOhmPNWffjMDFD67590871 = -582255246;    double DmPzOhmPNWffjMDFD93091988 = -95204201;    double DmPzOhmPNWffjMDFD5158763 = -290188190;    double DmPzOhmPNWffjMDFD88702652 = -157334041;    double DmPzOhmPNWffjMDFD15076691 = -781986943;    double DmPzOhmPNWffjMDFD37480126 = -474905104;    double DmPzOhmPNWffjMDFD25045227 = -30662944;    double DmPzOhmPNWffjMDFD80032552 = -99595365;    double DmPzOhmPNWffjMDFD40717816 = -924363550;    double DmPzOhmPNWffjMDFD39153562 = -299859376;    double DmPzOhmPNWffjMDFD15293521 = -995102743;    double DmPzOhmPNWffjMDFD54566385 = -700118759;    double DmPzOhmPNWffjMDFD82240143 = -30936805;    double DmPzOhmPNWffjMDFD77733767 = -415626416;    double DmPzOhmPNWffjMDFD79974722 = -181736702;    double DmPzOhmPNWffjMDFD24820412 = -439084342;    double DmPzOhmPNWffjMDFD27604735 = -717117293;    double DmPzOhmPNWffjMDFD76401072 = -486597741;    double DmPzOhmPNWffjMDFD72760939 = -573542260;    double DmPzOhmPNWffjMDFD34352511 = -153025208;    double DmPzOhmPNWffjMDFD55218332 = -179703363;    double DmPzOhmPNWffjMDFD66857920 = -433947627;    double DmPzOhmPNWffjMDFD14602462 = -284629140;    double DmPzOhmPNWffjMDFD61259698 = -242822739;    double DmPzOhmPNWffjMDFD30122207 = -818423485;    double DmPzOhmPNWffjMDFD52753261 = -504944066;    double DmPzOhmPNWffjMDFD11966848 = -259578854;    double DmPzOhmPNWffjMDFD17465345 = -697145562;    double DmPzOhmPNWffjMDFD8071103 = 91158127;    double DmPzOhmPNWffjMDFD58926240 = -727559386;    double DmPzOhmPNWffjMDFD10541169 = -408864692;    double DmPzOhmPNWffjMDFD97898996 = -829399383;    double DmPzOhmPNWffjMDFD67876549 = -818883633;    double DmPzOhmPNWffjMDFD99676901 = -254863879;    double DmPzOhmPNWffjMDFD4816211 = -257733434;    double DmPzOhmPNWffjMDFD20978647 = -937501379;    double DmPzOhmPNWffjMDFD61857195 = -890081413;    double DmPzOhmPNWffjMDFD59437083 = -942169959;    double DmPzOhmPNWffjMDFD5596140 = -939839365;    double DmPzOhmPNWffjMDFD99539011 = 48154691;    double DmPzOhmPNWffjMDFD8582533 = -301575519;    double DmPzOhmPNWffjMDFD36720104 = -329361507;    double DmPzOhmPNWffjMDFD66570174 = -688100625;    double DmPzOhmPNWffjMDFD63374865 = -407315833;    double DmPzOhmPNWffjMDFD5457288 = -959981400;    double DmPzOhmPNWffjMDFD24666632 = -832500927;    double DmPzOhmPNWffjMDFD64817405 = -671523756;    double DmPzOhmPNWffjMDFD37445903 = -474950571;    double DmPzOhmPNWffjMDFD35602072 = -242263481;    double DmPzOhmPNWffjMDFD82782584 = -746632587;    double DmPzOhmPNWffjMDFD15903551 = -171853558;    double DmPzOhmPNWffjMDFD58926483 = -801057065;    double DmPzOhmPNWffjMDFD55076894 = -493893153;    double DmPzOhmPNWffjMDFD32646020 = -364587604;    double DmPzOhmPNWffjMDFD82147375 = -851606244;    double DmPzOhmPNWffjMDFD732952 = -48307620;    double DmPzOhmPNWffjMDFD78489527 = -810575061;    double DmPzOhmPNWffjMDFD43899065 = 52634549;    double DmPzOhmPNWffjMDFD58580446 = -314442280;     DmPzOhmPNWffjMDFD94045357 = DmPzOhmPNWffjMDFD91836369;     DmPzOhmPNWffjMDFD91836369 = DmPzOhmPNWffjMDFD75744470;     DmPzOhmPNWffjMDFD75744470 = DmPzOhmPNWffjMDFD97073902;     DmPzOhmPNWffjMDFD97073902 = DmPzOhmPNWffjMDFD9780077;     DmPzOhmPNWffjMDFD9780077 = DmPzOhmPNWffjMDFD67980271;     DmPzOhmPNWffjMDFD67980271 = DmPzOhmPNWffjMDFD71970988;     DmPzOhmPNWffjMDFD71970988 = DmPzOhmPNWffjMDFD74145910;     DmPzOhmPNWffjMDFD74145910 = DmPzOhmPNWffjMDFD17130889;     DmPzOhmPNWffjMDFD17130889 = DmPzOhmPNWffjMDFD82010506;     DmPzOhmPNWffjMDFD82010506 = DmPzOhmPNWffjMDFD9355225;     DmPzOhmPNWffjMDFD9355225 = DmPzOhmPNWffjMDFD99810809;     DmPzOhmPNWffjMDFD99810809 = DmPzOhmPNWffjMDFD27146518;     DmPzOhmPNWffjMDFD27146518 = DmPzOhmPNWffjMDFD33042139;     DmPzOhmPNWffjMDFD33042139 = DmPzOhmPNWffjMDFD15674099;     DmPzOhmPNWffjMDFD15674099 = DmPzOhmPNWffjMDFD2434526;     DmPzOhmPNWffjMDFD2434526 = DmPzOhmPNWffjMDFD45868724;     DmPzOhmPNWffjMDFD45868724 = DmPzOhmPNWffjMDFD25736830;     DmPzOhmPNWffjMDFD25736830 = DmPzOhmPNWffjMDFD34875080;     DmPzOhmPNWffjMDFD34875080 = DmPzOhmPNWffjMDFD37042609;     DmPzOhmPNWffjMDFD37042609 = DmPzOhmPNWffjMDFD72407082;     DmPzOhmPNWffjMDFD72407082 = DmPzOhmPNWffjMDFD14070636;     DmPzOhmPNWffjMDFD14070636 = DmPzOhmPNWffjMDFD67015958;     DmPzOhmPNWffjMDFD67015958 = DmPzOhmPNWffjMDFD48139736;     DmPzOhmPNWffjMDFD48139736 = DmPzOhmPNWffjMDFD20672831;     DmPzOhmPNWffjMDFD20672831 = DmPzOhmPNWffjMDFD37019138;     DmPzOhmPNWffjMDFD37019138 = DmPzOhmPNWffjMDFD33627760;     DmPzOhmPNWffjMDFD33627760 = DmPzOhmPNWffjMDFD16752657;     DmPzOhmPNWffjMDFD16752657 = DmPzOhmPNWffjMDFD7287990;     DmPzOhmPNWffjMDFD7287990 = DmPzOhmPNWffjMDFD2528428;     DmPzOhmPNWffjMDFD2528428 = DmPzOhmPNWffjMDFD20750808;     DmPzOhmPNWffjMDFD20750808 = DmPzOhmPNWffjMDFD79233017;     DmPzOhmPNWffjMDFD79233017 = DmPzOhmPNWffjMDFD47057548;     DmPzOhmPNWffjMDFD47057548 = DmPzOhmPNWffjMDFD15179670;     DmPzOhmPNWffjMDFD15179670 = DmPzOhmPNWffjMDFD15576794;     DmPzOhmPNWffjMDFD15576794 = DmPzOhmPNWffjMDFD7602997;     DmPzOhmPNWffjMDFD7602997 = DmPzOhmPNWffjMDFD43508285;     DmPzOhmPNWffjMDFD43508285 = DmPzOhmPNWffjMDFD35327555;     DmPzOhmPNWffjMDFD35327555 = DmPzOhmPNWffjMDFD27837833;     DmPzOhmPNWffjMDFD27837833 = DmPzOhmPNWffjMDFD66998530;     DmPzOhmPNWffjMDFD66998530 = DmPzOhmPNWffjMDFD37365707;     DmPzOhmPNWffjMDFD37365707 = DmPzOhmPNWffjMDFD67590871;     DmPzOhmPNWffjMDFD67590871 = DmPzOhmPNWffjMDFD93091988;     DmPzOhmPNWffjMDFD93091988 = DmPzOhmPNWffjMDFD5158763;     DmPzOhmPNWffjMDFD5158763 = DmPzOhmPNWffjMDFD88702652;     DmPzOhmPNWffjMDFD88702652 = DmPzOhmPNWffjMDFD15076691;     DmPzOhmPNWffjMDFD15076691 = DmPzOhmPNWffjMDFD37480126;     DmPzOhmPNWffjMDFD37480126 = DmPzOhmPNWffjMDFD25045227;     DmPzOhmPNWffjMDFD25045227 = DmPzOhmPNWffjMDFD80032552;     DmPzOhmPNWffjMDFD80032552 = DmPzOhmPNWffjMDFD40717816;     DmPzOhmPNWffjMDFD40717816 = DmPzOhmPNWffjMDFD39153562;     DmPzOhmPNWffjMDFD39153562 = DmPzOhmPNWffjMDFD15293521;     DmPzOhmPNWffjMDFD15293521 = DmPzOhmPNWffjMDFD54566385;     DmPzOhmPNWffjMDFD54566385 = DmPzOhmPNWffjMDFD82240143;     DmPzOhmPNWffjMDFD82240143 = DmPzOhmPNWffjMDFD77733767;     DmPzOhmPNWffjMDFD77733767 = DmPzOhmPNWffjMDFD79974722;     DmPzOhmPNWffjMDFD79974722 = DmPzOhmPNWffjMDFD24820412;     DmPzOhmPNWffjMDFD24820412 = DmPzOhmPNWffjMDFD27604735;     DmPzOhmPNWffjMDFD27604735 = DmPzOhmPNWffjMDFD76401072;     DmPzOhmPNWffjMDFD76401072 = DmPzOhmPNWffjMDFD72760939;     DmPzOhmPNWffjMDFD72760939 = DmPzOhmPNWffjMDFD34352511;     DmPzOhmPNWffjMDFD34352511 = DmPzOhmPNWffjMDFD55218332;     DmPzOhmPNWffjMDFD55218332 = DmPzOhmPNWffjMDFD66857920;     DmPzOhmPNWffjMDFD66857920 = DmPzOhmPNWffjMDFD14602462;     DmPzOhmPNWffjMDFD14602462 = DmPzOhmPNWffjMDFD61259698;     DmPzOhmPNWffjMDFD61259698 = DmPzOhmPNWffjMDFD30122207;     DmPzOhmPNWffjMDFD30122207 = DmPzOhmPNWffjMDFD52753261;     DmPzOhmPNWffjMDFD52753261 = DmPzOhmPNWffjMDFD11966848;     DmPzOhmPNWffjMDFD11966848 = DmPzOhmPNWffjMDFD17465345;     DmPzOhmPNWffjMDFD17465345 = DmPzOhmPNWffjMDFD8071103;     DmPzOhmPNWffjMDFD8071103 = DmPzOhmPNWffjMDFD58926240;     DmPzOhmPNWffjMDFD58926240 = DmPzOhmPNWffjMDFD10541169;     DmPzOhmPNWffjMDFD10541169 = DmPzOhmPNWffjMDFD97898996;     DmPzOhmPNWffjMDFD97898996 = DmPzOhmPNWffjMDFD67876549;     DmPzOhmPNWffjMDFD67876549 = DmPzOhmPNWffjMDFD99676901;     DmPzOhmPNWffjMDFD99676901 = DmPzOhmPNWffjMDFD4816211;     DmPzOhmPNWffjMDFD4816211 = DmPzOhmPNWffjMDFD20978647;     DmPzOhmPNWffjMDFD20978647 = DmPzOhmPNWffjMDFD61857195;     DmPzOhmPNWffjMDFD61857195 = DmPzOhmPNWffjMDFD59437083;     DmPzOhmPNWffjMDFD59437083 = DmPzOhmPNWffjMDFD5596140;     DmPzOhmPNWffjMDFD5596140 = DmPzOhmPNWffjMDFD99539011;     DmPzOhmPNWffjMDFD99539011 = DmPzOhmPNWffjMDFD8582533;     DmPzOhmPNWffjMDFD8582533 = DmPzOhmPNWffjMDFD36720104;     DmPzOhmPNWffjMDFD36720104 = DmPzOhmPNWffjMDFD66570174;     DmPzOhmPNWffjMDFD66570174 = DmPzOhmPNWffjMDFD63374865;     DmPzOhmPNWffjMDFD63374865 = DmPzOhmPNWffjMDFD5457288;     DmPzOhmPNWffjMDFD5457288 = DmPzOhmPNWffjMDFD24666632;     DmPzOhmPNWffjMDFD24666632 = DmPzOhmPNWffjMDFD64817405;     DmPzOhmPNWffjMDFD64817405 = DmPzOhmPNWffjMDFD37445903;     DmPzOhmPNWffjMDFD37445903 = DmPzOhmPNWffjMDFD35602072;     DmPzOhmPNWffjMDFD35602072 = DmPzOhmPNWffjMDFD82782584;     DmPzOhmPNWffjMDFD82782584 = DmPzOhmPNWffjMDFD15903551;     DmPzOhmPNWffjMDFD15903551 = DmPzOhmPNWffjMDFD58926483;     DmPzOhmPNWffjMDFD58926483 = DmPzOhmPNWffjMDFD55076894;     DmPzOhmPNWffjMDFD55076894 = DmPzOhmPNWffjMDFD32646020;     DmPzOhmPNWffjMDFD32646020 = DmPzOhmPNWffjMDFD82147375;     DmPzOhmPNWffjMDFD82147375 = DmPzOhmPNWffjMDFD732952;     DmPzOhmPNWffjMDFD732952 = DmPzOhmPNWffjMDFD78489527;     DmPzOhmPNWffjMDFD78489527 = DmPzOhmPNWffjMDFD43899065;     DmPzOhmPNWffjMDFD43899065 = DmPzOhmPNWffjMDFD58580446;     DmPzOhmPNWffjMDFD58580446 = DmPzOhmPNWffjMDFD94045357;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ETsBwhOesupTcooM65189917() {     double dtKzIRtLpyYVlolpx162464 = -426344169;    double dtKzIRtLpyYVlolpx35209649 = -428953725;    double dtKzIRtLpyYVlolpx91786654 = 32545787;    double dtKzIRtLpyYVlolpx24148684 = -399172678;    double dtKzIRtLpyYVlolpx78675565 = -620942544;    double dtKzIRtLpyYVlolpx59397473 = -949759905;    double dtKzIRtLpyYVlolpx32459955 = -947625211;    double dtKzIRtLpyYVlolpx23905801 = -333285714;    double dtKzIRtLpyYVlolpx81773384 = -959837014;    double dtKzIRtLpyYVlolpx23240166 = -526882358;    double dtKzIRtLpyYVlolpx69465514 = -228480757;    double dtKzIRtLpyYVlolpx31633852 = -751810827;    double dtKzIRtLpyYVlolpx13355637 = -690046929;    double dtKzIRtLpyYVlolpx16185816 = -753384312;    double dtKzIRtLpyYVlolpx11416526 = -885020969;    double dtKzIRtLpyYVlolpx43939920 = -689529780;    double dtKzIRtLpyYVlolpx39704727 = -313064611;    double dtKzIRtLpyYVlolpx22789983 = -662357438;    double dtKzIRtLpyYVlolpx96704921 = -842265541;    double dtKzIRtLpyYVlolpx17592774 = -743076404;    double dtKzIRtLpyYVlolpx60960275 = -795476136;    double dtKzIRtLpyYVlolpx591659 = 59364705;    double dtKzIRtLpyYVlolpx2606750 = 73212911;    double dtKzIRtLpyYVlolpx32132110 = -321808987;    double dtKzIRtLpyYVlolpx64372480 = 4977660;    double dtKzIRtLpyYVlolpx61740813 = -405405736;    double dtKzIRtLpyYVlolpx32087538 = -332900665;    double dtKzIRtLpyYVlolpx29859782 = -594885825;    double dtKzIRtLpyYVlolpx73420245 = -834846879;    double dtKzIRtLpyYVlolpx71144064 = -624646075;    double dtKzIRtLpyYVlolpx12993358 = -868405081;    double dtKzIRtLpyYVlolpx76843639 = -242066502;    double dtKzIRtLpyYVlolpx56768496 = -695073931;    double dtKzIRtLpyYVlolpx10010235 = -742118752;    double dtKzIRtLpyYVlolpx77051446 = -62733288;    double dtKzIRtLpyYVlolpx76328909 = -399743037;    double dtKzIRtLpyYVlolpx24770220 = -942606721;    double dtKzIRtLpyYVlolpx69417768 = -59160815;    double dtKzIRtLpyYVlolpx1448087 = -61358910;    double dtKzIRtLpyYVlolpx54500468 = 8947966;    double dtKzIRtLpyYVlolpx43034931 = -96583500;    double dtKzIRtLpyYVlolpx99741328 = -852397525;    double dtKzIRtLpyYVlolpx71158967 = -176008784;    double dtKzIRtLpyYVlolpx92566502 = -309531128;    double dtKzIRtLpyYVlolpx655118 = -919232195;    double dtKzIRtLpyYVlolpx27015886 = -767203059;    double dtKzIRtLpyYVlolpx55258700 = -593680641;    double dtKzIRtLpyYVlolpx33002168 = -40558476;    double dtKzIRtLpyYVlolpx89708281 = -185332353;    double dtKzIRtLpyYVlolpx53629468 = 88625642;    double dtKzIRtLpyYVlolpx74854240 = -142153047;    double dtKzIRtLpyYVlolpx26103241 = -795829063;    double dtKzIRtLpyYVlolpx2814843 = -491908347;    double dtKzIRtLpyYVlolpx78209023 = -351406950;    double dtKzIRtLpyYVlolpx4229642 = -16158198;    double dtKzIRtLpyYVlolpx99570805 = -385708874;    double dtKzIRtLpyYVlolpx32602899 = -402166636;    double dtKzIRtLpyYVlolpx59654544 = -645645227;    double dtKzIRtLpyYVlolpx59776203 = -304150338;    double dtKzIRtLpyYVlolpx16934752 = -115536808;    double dtKzIRtLpyYVlolpx27309936 = -516859240;    double dtKzIRtLpyYVlolpx2600173 = -252739386;    double dtKzIRtLpyYVlolpx50485555 = -498438836;    double dtKzIRtLpyYVlolpx10629321 = -235190939;    double dtKzIRtLpyYVlolpx10246808 = -658477278;    double dtKzIRtLpyYVlolpx92621875 = -986414256;    double dtKzIRtLpyYVlolpx74865355 = 43263103;    double dtKzIRtLpyYVlolpx3345403 = -947928177;    double dtKzIRtLpyYVlolpx39134369 = -590651025;    double dtKzIRtLpyYVlolpx35087616 = -385277932;    double dtKzIRtLpyYVlolpx19169700 = -746923060;    double dtKzIRtLpyYVlolpx70286959 = -153903796;    double dtKzIRtLpyYVlolpx21341896 = -500998529;    double dtKzIRtLpyYVlolpx42204453 = -751213507;    double dtKzIRtLpyYVlolpx74557842 = -546492905;    double dtKzIRtLpyYVlolpx61218947 = -943078612;    double dtKzIRtLpyYVlolpx29432692 = -764626511;    double dtKzIRtLpyYVlolpx10040247 = -617255961;    double dtKzIRtLpyYVlolpx31476993 = -402576793;    double dtKzIRtLpyYVlolpx37356595 = -227819282;    double dtKzIRtLpyYVlolpx6482113 = -811725095;    double dtKzIRtLpyYVlolpx99085369 = -192342189;    double dtKzIRtLpyYVlolpx40151501 = -309553472;    double dtKzIRtLpyYVlolpx19790777 = -823472521;    double dtKzIRtLpyYVlolpx96289823 = -382493029;    double dtKzIRtLpyYVlolpx86890117 = 27423982;    double dtKzIRtLpyYVlolpx74028796 = -750158155;    double dtKzIRtLpyYVlolpx78559473 = -243666981;    double dtKzIRtLpyYVlolpx5780593 = -625960554;    double dtKzIRtLpyYVlolpx77480641 = -677024414;    double dtKzIRtLpyYVlolpx43726010 = -997576401;    double dtKzIRtLpyYVlolpx65115675 = -196961494;    double dtKzIRtLpyYVlolpx9641565 = -755010478;    double dtKzIRtLpyYVlolpx84513334 = -945822102;    double dtKzIRtLpyYVlolpx27190533 = -474192795;    double dtKzIRtLpyYVlolpx40434759 = -843844114;    double dtKzIRtLpyYVlolpx49255773 = -253958689;    double dtKzIRtLpyYVlolpx60529647 = -940817846;    double dtKzIRtLpyYVlolpx82319695 = -651053850;    double dtKzIRtLpyYVlolpx8033243 = -426344169;     dtKzIRtLpyYVlolpx162464 = dtKzIRtLpyYVlolpx35209649;     dtKzIRtLpyYVlolpx35209649 = dtKzIRtLpyYVlolpx91786654;     dtKzIRtLpyYVlolpx91786654 = dtKzIRtLpyYVlolpx24148684;     dtKzIRtLpyYVlolpx24148684 = dtKzIRtLpyYVlolpx78675565;     dtKzIRtLpyYVlolpx78675565 = dtKzIRtLpyYVlolpx59397473;     dtKzIRtLpyYVlolpx59397473 = dtKzIRtLpyYVlolpx32459955;     dtKzIRtLpyYVlolpx32459955 = dtKzIRtLpyYVlolpx23905801;     dtKzIRtLpyYVlolpx23905801 = dtKzIRtLpyYVlolpx81773384;     dtKzIRtLpyYVlolpx81773384 = dtKzIRtLpyYVlolpx23240166;     dtKzIRtLpyYVlolpx23240166 = dtKzIRtLpyYVlolpx69465514;     dtKzIRtLpyYVlolpx69465514 = dtKzIRtLpyYVlolpx31633852;     dtKzIRtLpyYVlolpx31633852 = dtKzIRtLpyYVlolpx13355637;     dtKzIRtLpyYVlolpx13355637 = dtKzIRtLpyYVlolpx16185816;     dtKzIRtLpyYVlolpx16185816 = dtKzIRtLpyYVlolpx11416526;     dtKzIRtLpyYVlolpx11416526 = dtKzIRtLpyYVlolpx43939920;     dtKzIRtLpyYVlolpx43939920 = dtKzIRtLpyYVlolpx39704727;     dtKzIRtLpyYVlolpx39704727 = dtKzIRtLpyYVlolpx22789983;     dtKzIRtLpyYVlolpx22789983 = dtKzIRtLpyYVlolpx96704921;     dtKzIRtLpyYVlolpx96704921 = dtKzIRtLpyYVlolpx17592774;     dtKzIRtLpyYVlolpx17592774 = dtKzIRtLpyYVlolpx60960275;     dtKzIRtLpyYVlolpx60960275 = dtKzIRtLpyYVlolpx591659;     dtKzIRtLpyYVlolpx591659 = dtKzIRtLpyYVlolpx2606750;     dtKzIRtLpyYVlolpx2606750 = dtKzIRtLpyYVlolpx32132110;     dtKzIRtLpyYVlolpx32132110 = dtKzIRtLpyYVlolpx64372480;     dtKzIRtLpyYVlolpx64372480 = dtKzIRtLpyYVlolpx61740813;     dtKzIRtLpyYVlolpx61740813 = dtKzIRtLpyYVlolpx32087538;     dtKzIRtLpyYVlolpx32087538 = dtKzIRtLpyYVlolpx29859782;     dtKzIRtLpyYVlolpx29859782 = dtKzIRtLpyYVlolpx73420245;     dtKzIRtLpyYVlolpx73420245 = dtKzIRtLpyYVlolpx71144064;     dtKzIRtLpyYVlolpx71144064 = dtKzIRtLpyYVlolpx12993358;     dtKzIRtLpyYVlolpx12993358 = dtKzIRtLpyYVlolpx76843639;     dtKzIRtLpyYVlolpx76843639 = dtKzIRtLpyYVlolpx56768496;     dtKzIRtLpyYVlolpx56768496 = dtKzIRtLpyYVlolpx10010235;     dtKzIRtLpyYVlolpx10010235 = dtKzIRtLpyYVlolpx77051446;     dtKzIRtLpyYVlolpx77051446 = dtKzIRtLpyYVlolpx76328909;     dtKzIRtLpyYVlolpx76328909 = dtKzIRtLpyYVlolpx24770220;     dtKzIRtLpyYVlolpx24770220 = dtKzIRtLpyYVlolpx69417768;     dtKzIRtLpyYVlolpx69417768 = dtKzIRtLpyYVlolpx1448087;     dtKzIRtLpyYVlolpx1448087 = dtKzIRtLpyYVlolpx54500468;     dtKzIRtLpyYVlolpx54500468 = dtKzIRtLpyYVlolpx43034931;     dtKzIRtLpyYVlolpx43034931 = dtKzIRtLpyYVlolpx99741328;     dtKzIRtLpyYVlolpx99741328 = dtKzIRtLpyYVlolpx71158967;     dtKzIRtLpyYVlolpx71158967 = dtKzIRtLpyYVlolpx92566502;     dtKzIRtLpyYVlolpx92566502 = dtKzIRtLpyYVlolpx655118;     dtKzIRtLpyYVlolpx655118 = dtKzIRtLpyYVlolpx27015886;     dtKzIRtLpyYVlolpx27015886 = dtKzIRtLpyYVlolpx55258700;     dtKzIRtLpyYVlolpx55258700 = dtKzIRtLpyYVlolpx33002168;     dtKzIRtLpyYVlolpx33002168 = dtKzIRtLpyYVlolpx89708281;     dtKzIRtLpyYVlolpx89708281 = dtKzIRtLpyYVlolpx53629468;     dtKzIRtLpyYVlolpx53629468 = dtKzIRtLpyYVlolpx74854240;     dtKzIRtLpyYVlolpx74854240 = dtKzIRtLpyYVlolpx26103241;     dtKzIRtLpyYVlolpx26103241 = dtKzIRtLpyYVlolpx2814843;     dtKzIRtLpyYVlolpx2814843 = dtKzIRtLpyYVlolpx78209023;     dtKzIRtLpyYVlolpx78209023 = dtKzIRtLpyYVlolpx4229642;     dtKzIRtLpyYVlolpx4229642 = dtKzIRtLpyYVlolpx99570805;     dtKzIRtLpyYVlolpx99570805 = dtKzIRtLpyYVlolpx32602899;     dtKzIRtLpyYVlolpx32602899 = dtKzIRtLpyYVlolpx59654544;     dtKzIRtLpyYVlolpx59654544 = dtKzIRtLpyYVlolpx59776203;     dtKzIRtLpyYVlolpx59776203 = dtKzIRtLpyYVlolpx16934752;     dtKzIRtLpyYVlolpx16934752 = dtKzIRtLpyYVlolpx27309936;     dtKzIRtLpyYVlolpx27309936 = dtKzIRtLpyYVlolpx2600173;     dtKzIRtLpyYVlolpx2600173 = dtKzIRtLpyYVlolpx50485555;     dtKzIRtLpyYVlolpx50485555 = dtKzIRtLpyYVlolpx10629321;     dtKzIRtLpyYVlolpx10629321 = dtKzIRtLpyYVlolpx10246808;     dtKzIRtLpyYVlolpx10246808 = dtKzIRtLpyYVlolpx92621875;     dtKzIRtLpyYVlolpx92621875 = dtKzIRtLpyYVlolpx74865355;     dtKzIRtLpyYVlolpx74865355 = dtKzIRtLpyYVlolpx3345403;     dtKzIRtLpyYVlolpx3345403 = dtKzIRtLpyYVlolpx39134369;     dtKzIRtLpyYVlolpx39134369 = dtKzIRtLpyYVlolpx35087616;     dtKzIRtLpyYVlolpx35087616 = dtKzIRtLpyYVlolpx19169700;     dtKzIRtLpyYVlolpx19169700 = dtKzIRtLpyYVlolpx70286959;     dtKzIRtLpyYVlolpx70286959 = dtKzIRtLpyYVlolpx21341896;     dtKzIRtLpyYVlolpx21341896 = dtKzIRtLpyYVlolpx42204453;     dtKzIRtLpyYVlolpx42204453 = dtKzIRtLpyYVlolpx74557842;     dtKzIRtLpyYVlolpx74557842 = dtKzIRtLpyYVlolpx61218947;     dtKzIRtLpyYVlolpx61218947 = dtKzIRtLpyYVlolpx29432692;     dtKzIRtLpyYVlolpx29432692 = dtKzIRtLpyYVlolpx10040247;     dtKzIRtLpyYVlolpx10040247 = dtKzIRtLpyYVlolpx31476993;     dtKzIRtLpyYVlolpx31476993 = dtKzIRtLpyYVlolpx37356595;     dtKzIRtLpyYVlolpx37356595 = dtKzIRtLpyYVlolpx6482113;     dtKzIRtLpyYVlolpx6482113 = dtKzIRtLpyYVlolpx99085369;     dtKzIRtLpyYVlolpx99085369 = dtKzIRtLpyYVlolpx40151501;     dtKzIRtLpyYVlolpx40151501 = dtKzIRtLpyYVlolpx19790777;     dtKzIRtLpyYVlolpx19790777 = dtKzIRtLpyYVlolpx96289823;     dtKzIRtLpyYVlolpx96289823 = dtKzIRtLpyYVlolpx86890117;     dtKzIRtLpyYVlolpx86890117 = dtKzIRtLpyYVlolpx74028796;     dtKzIRtLpyYVlolpx74028796 = dtKzIRtLpyYVlolpx78559473;     dtKzIRtLpyYVlolpx78559473 = dtKzIRtLpyYVlolpx5780593;     dtKzIRtLpyYVlolpx5780593 = dtKzIRtLpyYVlolpx77480641;     dtKzIRtLpyYVlolpx77480641 = dtKzIRtLpyYVlolpx43726010;     dtKzIRtLpyYVlolpx43726010 = dtKzIRtLpyYVlolpx65115675;     dtKzIRtLpyYVlolpx65115675 = dtKzIRtLpyYVlolpx9641565;     dtKzIRtLpyYVlolpx9641565 = dtKzIRtLpyYVlolpx84513334;     dtKzIRtLpyYVlolpx84513334 = dtKzIRtLpyYVlolpx27190533;     dtKzIRtLpyYVlolpx27190533 = dtKzIRtLpyYVlolpx40434759;     dtKzIRtLpyYVlolpx40434759 = dtKzIRtLpyYVlolpx49255773;     dtKzIRtLpyYVlolpx49255773 = dtKzIRtLpyYVlolpx60529647;     dtKzIRtLpyYVlolpx60529647 = dtKzIRtLpyYVlolpx82319695;     dtKzIRtLpyYVlolpx82319695 = dtKzIRtLpyYVlolpx8033243;     dtKzIRtLpyYVlolpx8033243 = dtKzIRtLpyYVlolpx162464;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void EpUQZuGLuNkMzjtE32481517() {     double WhvvRXipWaXeSPueG35621433 = -892621069;    double WhvvRXipWaXeSPueG48481727 = -309505331;    double WhvvRXipWaXeSPueG29173131 = -733121288;    double WhvvRXipWaXeSPueG96372038 = -220323749;    double WhvvRXipWaXeSPueG34960834 = -726541389;    double WhvvRXipWaXeSPueG95620345 = -112107800;    double WhvvRXipWaXeSPueG43117361 = -792350945;    double WhvvRXipWaXeSPueG86037361 = 22480146;    double WhvvRXipWaXeSPueG83290989 = -348168421;    double WhvvRXipWaXeSPueG38142878 = -191569356;    double WhvvRXipWaXeSPueG7647250 = -30488486;    double WhvvRXipWaXeSPueG32202000 = -615931599;    double WhvvRXipWaXeSPueG51590282 = -142059434;    double WhvvRXipWaXeSPueG69326678 = -729747722;    double WhvvRXipWaXeSPueG69611041 = -288445984;    double WhvvRXipWaXeSPueG27481221 = -325341387;    double WhvvRXipWaXeSPueG95540662 = -840039163;    double WhvvRXipWaXeSPueG22550174 = -735785507;    double WhvvRXipWaXeSPueG97255251 = -58828962;    double WhvvRXipWaXeSPueG11429992 = -69637470;    double WhvvRXipWaXeSPueG74047138 = -452750824;    double WhvvRXipWaXeSPueG4013184 = -27271589;    double WhvvRXipWaXeSPueG73132129 = -664419273;    double WhvvRXipWaXeSPueG25925055 = -832633591;    double WhvvRXipWaXeSPueG72390431 = -19007539;    double WhvvRXipWaXeSPueG72438984 = -93982543;    double WhvvRXipWaXeSPueG92508112 = -139184806;    double WhvvRXipWaXeSPueG70588207 = -764658000;    double WhvvRXipWaXeSPueG20088460 = -308941681;    double WhvvRXipWaXeSPueG76677475 = -659153246;    double WhvvRXipWaXeSPueG31576589 = -320385568;    double WhvvRXipWaXeSPueG14666658 = -15037916;    double WhvvRXipWaXeSPueG68986507 = -842669908;    double WhvvRXipWaXeSPueG39758661 = -806812565;    double WhvvRXipWaXeSPueG42859900 = -13833539;    double WhvvRXipWaXeSPueG27889753 = -614308343;    double WhvvRXipWaXeSPueG74182036 = -807302761;    double WhvvRXipWaXeSPueG20633171 = -41816397;    double WhvvRXipWaXeSPueG31838648 = -812235186;    double WhvvRXipWaXeSPueG18772001 = -556314580;    double WhvvRXipWaXeSPueG76486538 = -965782415;    double WhvvRXipWaXeSPueG89106455 = -446942815;    double WhvvRXipWaXeSPueG60542345 = -829189570;    double WhvvRXipWaXeSPueG32697841 = -913863538;    double WhvvRXipWaXeSPueG671577 = -745743356;    double WhvvRXipWaXeSPueG72977214 = -806126691;    double WhvvRXipWaXeSPueG74603864 = 22332998;    double WhvvRXipWaXeSPueG50728480 = -207085070;    double WhvvRXipWaXeSPueG12600948 = -949069234;    double WhvvRXipWaXeSPueG78159057 = -402401174;    double WhvvRXipWaXeSPueG55624964 = -16429605;    double WhvvRXipWaXeSPueG9588713 = 90518591;    double WhvvRXipWaXeSPueG71435991 = -247953901;    double WhvvRXipWaXeSPueG94200869 = -542915450;    double WhvvRXipWaXeSPueG58434378 = -362648344;    double WhvvRXipWaXeSPueG31608249 = -765349480;    double WhvvRXipWaXeSPueG75349597 = -645086059;    double WhvvRXipWaXeSPueG3248076 = -900487697;    double WhvvRXipWaXeSPueG23981607 = -101316211;    double WhvvRXipWaXeSPueG62521849 = -532558847;    double WhvvRXipWaXeSPueG3112234 = -972922995;    double WhvvRXipWaXeSPueG72529154 = 72307054;    double WhvvRXipWaXeSPueG65948902 = -668578174;    double WhvvRXipWaXeSPueG6613515 = -689015175;    double WhvvRXipWaXeSPueG6566289 = -871183789;    double WhvvRXipWaXeSPueG92980591 = 84549429;    double WhvvRXipWaXeSPueG63215493 = -773261692;    double WhvvRXipWaXeSPueG11831622 = -335246869;    double WhvvRXipWaXeSPueG26466779 = -615914183;    double WhvvRXipWaXeSPueG41721288 = -674137641;    double WhvvRXipWaXeSPueG53299184 = -518038627;    double WhvvRXipWaXeSPueG74907491 = -698222767;    double WhvvRXipWaXeSPueG90711525 = -923550321;    double WhvvRXipWaXeSPueG78483251 = -502514382;    double WhvvRXipWaXeSPueG34943454 = -103855056;    double WhvvRXipWaXeSPueG84940683 = 94191991;    double WhvvRXipWaXeSPueG43470838 = -198082019;    double WhvvRXipWaXeSPueG40434289 = -750555736;    double WhvvRXipWaXeSPueG25253479 = 13109765;    double WhvvRXipWaXeSPueG99413217 = -212880848;    double WhvvRXipWaXeSPueG97835120 = -16315541;    double WhvvRXipWaXeSPueG41779632 = -932099737;    double WhvvRXipWaXeSPueG57987259 = -815588767;    double WhvvRXipWaXeSPueG41929402 = -906540508;    double WhvvRXipWaXeSPueG21052511 = -542723642;    double WhvvRXipWaXeSPueG21987877 = -310904159;    double WhvvRXipWaXeSPueG43230667 = -767084016;    double WhvvRXipWaXeSPueG74785637 = -199754459;    double WhvvRXipWaXeSPueG81324282 = -344164222;    double WhvvRXipWaXeSPueG11251651 = -248484059;    double WhvvRXipWaXeSPueG52540155 = -969222285;    double WhvvRXipWaXeSPueG70933960 = -906815064;    double WhvvRXipWaXeSPueG96651564 = -940500186;    double WhvvRXipWaXeSPueG69316798 = -179676340;    double WhvvRXipWaXeSPueG15659768 = -583391586;    double WhvvRXipWaXeSPueG3957385 = -938089469;    double WhvvRXipWaXeSPueG23157553 = -778364642;    double WhvvRXipWaXeSPueG53928831 = -40174396;    double WhvvRXipWaXeSPueG26131553 = 57320251;    double WhvvRXipWaXeSPueG7690985 = -892621069;     WhvvRXipWaXeSPueG35621433 = WhvvRXipWaXeSPueG48481727;     WhvvRXipWaXeSPueG48481727 = WhvvRXipWaXeSPueG29173131;     WhvvRXipWaXeSPueG29173131 = WhvvRXipWaXeSPueG96372038;     WhvvRXipWaXeSPueG96372038 = WhvvRXipWaXeSPueG34960834;     WhvvRXipWaXeSPueG34960834 = WhvvRXipWaXeSPueG95620345;     WhvvRXipWaXeSPueG95620345 = WhvvRXipWaXeSPueG43117361;     WhvvRXipWaXeSPueG43117361 = WhvvRXipWaXeSPueG86037361;     WhvvRXipWaXeSPueG86037361 = WhvvRXipWaXeSPueG83290989;     WhvvRXipWaXeSPueG83290989 = WhvvRXipWaXeSPueG38142878;     WhvvRXipWaXeSPueG38142878 = WhvvRXipWaXeSPueG7647250;     WhvvRXipWaXeSPueG7647250 = WhvvRXipWaXeSPueG32202000;     WhvvRXipWaXeSPueG32202000 = WhvvRXipWaXeSPueG51590282;     WhvvRXipWaXeSPueG51590282 = WhvvRXipWaXeSPueG69326678;     WhvvRXipWaXeSPueG69326678 = WhvvRXipWaXeSPueG69611041;     WhvvRXipWaXeSPueG69611041 = WhvvRXipWaXeSPueG27481221;     WhvvRXipWaXeSPueG27481221 = WhvvRXipWaXeSPueG95540662;     WhvvRXipWaXeSPueG95540662 = WhvvRXipWaXeSPueG22550174;     WhvvRXipWaXeSPueG22550174 = WhvvRXipWaXeSPueG97255251;     WhvvRXipWaXeSPueG97255251 = WhvvRXipWaXeSPueG11429992;     WhvvRXipWaXeSPueG11429992 = WhvvRXipWaXeSPueG74047138;     WhvvRXipWaXeSPueG74047138 = WhvvRXipWaXeSPueG4013184;     WhvvRXipWaXeSPueG4013184 = WhvvRXipWaXeSPueG73132129;     WhvvRXipWaXeSPueG73132129 = WhvvRXipWaXeSPueG25925055;     WhvvRXipWaXeSPueG25925055 = WhvvRXipWaXeSPueG72390431;     WhvvRXipWaXeSPueG72390431 = WhvvRXipWaXeSPueG72438984;     WhvvRXipWaXeSPueG72438984 = WhvvRXipWaXeSPueG92508112;     WhvvRXipWaXeSPueG92508112 = WhvvRXipWaXeSPueG70588207;     WhvvRXipWaXeSPueG70588207 = WhvvRXipWaXeSPueG20088460;     WhvvRXipWaXeSPueG20088460 = WhvvRXipWaXeSPueG76677475;     WhvvRXipWaXeSPueG76677475 = WhvvRXipWaXeSPueG31576589;     WhvvRXipWaXeSPueG31576589 = WhvvRXipWaXeSPueG14666658;     WhvvRXipWaXeSPueG14666658 = WhvvRXipWaXeSPueG68986507;     WhvvRXipWaXeSPueG68986507 = WhvvRXipWaXeSPueG39758661;     WhvvRXipWaXeSPueG39758661 = WhvvRXipWaXeSPueG42859900;     WhvvRXipWaXeSPueG42859900 = WhvvRXipWaXeSPueG27889753;     WhvvRXipWaXeSPueG27889753 = WhvvRXipWaXeSPueG74182036;     WhvvRXipWaXeSPueG74182036 = WhvvRXipWaXeSPueG20633171;     WhvvRXipWaXeSPueG20633171 = WhvvRXipWaXeSPueG31838648;     WhvvRXipWaXeSPueG31838648 = WhvvRXipWaXeSPueG18772001;     WhvvRXipWaXeSPueG18772001 = WhvvRXipWaXeSPueG76486538;     WhvvRXipWaXeSPueG76486538 = WhvvRXipWaXeSPueG89106455;     WhvvRXipWaXeSPueG89106455 = WhvvRXipWaXeSPueG60542345;     WhvvRXipWaXeSPueG60542345 = WhvvRXipWaXeSPueG32697841;     WhvvRXipWaXeSPueG32697841 = WhvvRXipWaXeSPueG671577;     WhvvRXipWaXeSPueG671577 = WhvvRXipWaXeSPueG72977214;     WhvvRXipWaXeSPueG72977214 = WhvvRXipWaXeSPueG74603864;     WhvvRXipWaXeSPueG74603864 = WhvvRXipWaXeSPueG50728480;     WhvvRXipWaXeSPueG50728480 = WhvvRXipWaXeSPueG12600948;     WhvvRXipWaXeSPueG12600948 = WhvvRXipWaXeSPueG78159057;     WhvvRXipWaXeSPueG78159057 = WhvvRXipWaXeSPueG55624964;     WhvvRXipWaXeSPueG55624964 = WhvvRXipWaXeSPueG9588713;     WhvvRXipWaXeSPueG9588713 = WhvvRXipWaXeSPueG71435991;     WhvvRXipWaXeSPueG71435991 = WhvvRXipWaXeSPueG94200869;     WhvvRXipWaXeSPueG94200869 = WhvvRXipWaXeSPueG58434378;     WhvvRXipWaXeSPueG58434378 = WhvvRXipWaXeSPueG31608249;     WhvvRXipWaXeSPueG31608249 = WhvvRXipWaXeSPueG75349597;     WhvvRXipWaXeSPueG75349597 = WhvvRXipWaXeSPueG3248076;     WhvvRXipWaXeSPueG3248076 = WhvvRXipWaXeSPueG23981607;     WhvvRXipWaXeSPueG23981607 = WhvvRXipWaXeSPueG62521849;     WhvvRXipWaXeSPueG62521849 = WhvvRXipWaXeSPueG3112234;     WhvvRXipWaXeSPueG3112234 = WhvvRXipWaXeSPueG72529154;     WhvvRXipWaXeSPueG72529154 = WhvvRXipWaXeSPueG65948902;     WhvvRXipWaXeSPueG65948902 = WhvvRXipWaXeSPueG6613515;     WhvvRXipWaXeSPueG6613515 = WhvvRXipWaXeSPueG6566289;     WhvvRXipWaXeSPueG6566289 = WhvvRXipWaXeSPueG92980591;     WhvvRXipWaXeSPueG92980591 = WhvvRXipWaXeSPueG63215493;     WhvvRXipWaXeSPueG63215493 = WhvvRXipWaXeSPueG11831622;     WhvvRXipWaXeSPueG11831622 = WhvvRXipWaXeSPueG26466779;     WhvvRXipWaXeSPueG26466779 = WhvvRXipWaXeSPueG41721288;     WhvvRXipWaXeSPueG41721288 = WhvvRXipWaXeSPueG53299184;     WhvvRXipWaXeSPueG53299184 = WhvvRXipWaXeSPueG74907491;     WhvvRXipWaXeSPueG74907491 = WhvvRXipWaXeSPueG90711525;     WhvvRXipWaXeSPueG90711525 = WhvvRXipWaXeSPueG78483251;     WhvvRXipWaXeSPueG78483251 = WhvvRXipWaXeSPueG34943454;     WhvvRXipWaXeSPueG34943454 = WhvvRXipWaXeSPueG84940683;     WhvvRXipWaXeSPueG84940683 = WhvvRXipWaXeSPueG43470838;     WhvvRXipWaXeSPueG43470838 = WhvvRXipWaXeSPueG40434289;     WhvvRXipWaXeSPueG40434289 = WhvvRXipWaXeSPueG25253479;     WhvvRXipWaXeSPueG25253479 = WhvvRXipWaXeSPueG99413217;     WhvvRXipWaXeSPueG99413217 = WhvvRXipWaXeSPueG97835120;     WhvvRXipWaXeSPueG97835120 = WhvvRXipWaXeSPueG41779632;     WhvvRXipWaXeSPueG41779632 = WhvvRXipWaXeSPueG57987259;     WhvvRXipWaXeSPueG57987259 = WhvvRXipWaXeSPueG41929402;     WhvvRXipWaXeSPueG41929402 = WhvvRXipWaXeSPueG21052511;     WhvvRXipWaXeSPueG21052511 = WhvvRXipWaXeSPueG21987877;     WhvvRXipWaXeSPueG21987877 = WhvvRXipWaXeSPueG43230667;     WhvvRXipWaXeSPueG43230667 = WhvvRXipWaXeSPueG74785637;     WhvvRXipWaXeSPueG74785637 = WhvvRXipWaXeSPueG81324282;     WhvvRXipWaXeSPueG81324282 = WhvvRXipWaXeSPueG11251651;     WhvvRXipWaXeSPueG11251651 = WhvvRXipWaXeSPueG52540155;     WhvvRXipWaXeSPueG52540155 = WhvvRXipWaXeSPueG70933960;     WhvvRXipWaXeSPueG70933960 = WhvvRXipWaXeSPueG96651564;     WhvvRXipWaXeSPueG96651564 = WhvvRXipWaXeSPueG69316798;     WhvvRXipWaXeSPueG69316798 = WhvvRXipWaXeSPueG15659768;     WhvvRXipWaXeSPueG15659768 = WhvvRXipWaXeSPueG3957385;     WhvvRXipWaXeSPueG3957385 = WhvvRXipWaXeSPueG23157553;     WhvvRXipWaXeSPueG23157553 = WhvvRXipWaXeSPueG53928831;     WhvvRXipWaXeSPueG53928831 = WhvvRXipWaXeSPueG26131553;     WhvvRXipWaXeSPueG26131553 = WhvvRXipWaXeSPueG7690985;     WhvvRXipWaXeSPueG7690985 = WhvvRXipWaXeSPueG35621433;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void OVinaQVdydgRJkEW47530585() {     double zbCqcZQarDWJPAbDo41738539 = 95477043;    double zbCqcZQarDWJPAbDo91855006 = -19105112;    double zbCqcZQarDWJPAbDo45215315 = -883954209;    double zbCqcZQarDWJPAbDo23446819 = -411072380;    double zbCqcZQarDWJPAbDo3856323 = -147191260;    double zbCqcZQarDWJPAbDo87037547 = -376604035;    double zbCqcZQarDWJPAbDo3606327 = -931315923;    double zbCqcZQarDWJPAbDo35797253 = -264393767;    double zbCqcZQarDWJPAbDo47933485 = -116201086;    double zbCqcZQarDWJPAbDo79372536 = -520544834;    double zbCqcZQarDWJPAbDo67757540 = 92073927;    double zbCqcZQarDWJPAbDo64025042 = 39662199;    double zbCqcZQarDWJPAbDo37799401 = -581950522;    double zbCqcZQarDWJPAbDo52470355 = -161986289;    double zbCqcZQarDWJPAbDo65353468 = -978908150;    double zbCqcZQarDWJPAbDo68986614 = -298340930;    double zbCqcZQarDWJPAbDo89376665 = -356584276;    double zbCqcZQarDWJPAbDo19603327 = -401308150;    double zbCqcZQarDWJPAbDo59085093 = -464598059;    double zbCqcZQarDWJPAbDo91980156 = -426540390;    double zbCqcZQarDWJPAbDo62600331 = -308238280;    double zbCqcZQarDWJPAbDo90534207 = 64798695;    double zbCqcZQarDWJPAbDo8722922 = -410936760;    double zbCqcZQarDWJPAbDo9917429 = 45061421;    double zbCqcZQarDWJPAbDo16090082 = -392203572;    double zbCqcZQarDWJPAbDo97160659 = 27362134;    double zbCqcZQarDWJPAbDo90967890 = -39847009;    double zbCqcZQarDWJPAbDo83695332 = -830586953;    double zbCqcZQarDWJPAbDo86220715 = -531324386;    double zbCqcZQarDWJPAbDo45293112 = -476624113;    double zbCqcZQarDWJPAbDo23819139 = -233706506;    double zbCqcZQarDWJPAbDo12277280 = -824484732;    double zbCqcZQarDWJPAbDo78697455 = -735283278;    double zbCqcZQarDWJPAbDo34589225 = -558354330;    double zbCqcZQarDWJPAbDo4334552 = -652566644;    double zbCqcZQarDWJPAbDo96615666 = -828334450;    double zbCqcZQarDWJPAbDo55443971 = -760938630;    double zbCqcZQarDWJPAbDo54723384 = -913322406;    double zbCqcZQarDWJPAbDo5448902 = -806158683;    double zbCqcZQarDWJPAbDo6273939 = 70246197;    double zbCqcZQarDWJPAbDo82155762 = 68943691;    double zbCqcZQarDWJPAbDo21256912 = -717085094;    double zbCqcZQarDWJPAbDo38609324 = -909994154;    double zbCqcZQarDWJPAbDo20105582 = -933206476;    double zbCqcZQarDWJPAbDo12624042 = -407641511;    double zbCqcZQarDWJPAbDo84916409 = -791342808;    double zbCqcZQarDWJPAbDo92382438 = -96442540;    double zbCqcZQarDWJPAbDo58685421 = -216980602;    double zbCqcZQarDWJPAbDo22276677 = 65193778;    double zbCqcZQarDWJPAbDo91070710 = -489411982;    double zbCqcZQarDWJPAbDo91325642 = -958723276;    double zbCqcZQarDWJPAbDo20398433 = -810207729;    double zbCqcZQarDWJPAbDo19684449 = -39743489;    double zbCqcZQarDWJPAbDo90169750 = -863385595;    double zbCqcZQarDWJPAbDo84930253 = 36819873;    double zbCqcZQarDWJPAbDo51204332 = -969321653;    double zbCqcZQarDWJPAbDo83132085 = -608168352;    double zbCqcZQarDWJPAbDo35297886 = -829015631;    double zbCqcZQarDWJPAbDo7356738 = 81131191;    double zbCqcZQarDWJPAbDo6695663 = -74553395;    double zbCqcZQarDWJPAbDo96069657 = -236757027;    double zbCqcZQarDWJPAbDo19910995 = -728970;    double zbCqcZQarDWJPAbDo49576537 = -733069382;    double zbCqcZQarDWJPAbDo2640374 = -639576973;    double zbCqcZQarDWJPAbDo55553398 = -186838329;    double zbCqcZQarDWJPAbDo55480260 = -83441342;    double zbCqcZQarDWJPAbDo85327587 = -225054523;    double zbCqcZQarDWJPAbDo3210176 = 76403808;    double zbCqcZQarDWJPAbDo48135804 = -509419646;    double zbCqcZQarDWJPAbDo68737802 = -50573700;    double zbCqcZQarDWJPAbDo13542643 = -537402300;    double zbCqcZQarDWJPAbDo34653282 = -443261870;    double zbCqcZQarDWJPAbDo14154425 = -595149467;    double zbCqcZQarDWJPAbDo52811155 = -434844256;    double zbCqcZQarDWJPAbDo9824395 = -395484081;    double zbCqcZQarDWJPAbDo41343420 = -591153187;    double zbCqcZQarDWJPAbDo51924884 = -25207152;    double zbCqcZQarDWJPAbDo88617339 = -477730284;    double zbCqcZQarDWJPAbDo97293387 = -547297069;    double zbCqcZQarDWJPAbDo31173673 = -600860765;    double zbCqcZQarDWJPAbDo4778222 = -876195327;    double zbCqcZQarDWJPAbDo32282469 = -822866407;    double zbCqcZQarDWJPAbDo61418656 = -795780732;    double zbCqcZQarDWJPAbDo95150005 = 58087596;    double zbCqcZQarDWJPAbDo53967469 = -517900838;    double zbCqcZQarDWJPAbDo3420707 = -423498777;    double zbCqcZQarDWJPAbDo92592830 = -684741244;    double zbCqcZQarDWJPAbDo88527705 = -871897684;    double zbCqcZQarDWJPAbDo49658972 = -495174204;    double zbCqcZQarDWJPAbDo53130220 = -683244992;    double zbCqcZQarDWJPAbDo13483582 = -120166098;    double zbCqcZQarDWJPAbDo20146086 = -931923000;    double zbCqcZQarDWJPAbDo47366646 = -894453598;    double zbCqcZQarDWJPAbDo98753239 = -631605289;    double zbCqcZQarDWJPAbDo10204281 = -692996777;    double zbCqcZQarDWJPAbDo62244768 = -930327340;    double zbCqcZQarDWJPAbDo71680374 = -984015712;    double zbCqcZQarDWJPAbDo35968950 = -170417181;    double zbCqcZQarDWJPAbDo64552183 = -646368148;    double zbCqcZQarDWJPAbDo57143781 = 95477043;     zbCqcZQarDWJPAbDo41738539 = zbCqcZQarDWJPAbDo91855006;     zbCqcZQarDWJPAbDo91855006 = zbCqcZQarDWJPAbDo45215315;     zbCqcZQarDWJPAbDo45215315 = zbCqcZQarDWJPAbDo23446819;     zbCqcZQarDWJPAbDo23446819 = zbCqcZQarDWJPAbDo3856323;     zbCqcZQarDWJPAbDo3856323 = zbCqcZQarDWJPAbDo87037547;     zbCqcZQarDWJPAbDo87037547 = zbCqcZQarDWJPAbDo3606327;     zbCqcZQarDWJPAbDo3606327 = zbCqcZQarDWJPAbDo35797253;     zbCqcZQarDWJPAbDo35797253 = zbCqcZQarDWJPAbDo47933485;     zbCqcZQarDWJPAbDo47933485 = zbCqcZQarDWJPAbDo79372536;     zbCqcZQarDWJPAbDo79372536 = zbCqcZQarDWJPAbDo67757540;     zbCqcZQarDWJPAbDo67757540 = zbCqcZQarDWJPAbDo64025042;     zbCqcZQarDWJPAbDo64025042 = zbCqcZQarDWJPAbDo37799401;     zbCqcZQarDWJPAbDo37799401 = zbCqcZQarDWJPAbDo52470355;     zbCqcZQarDWJPAbDo52470355 = zbCqcZQarDWJPAbDo65353468;     zbCqcZQarDWJPAbDo65353468 = zbCqcZQarDWJPAbDo68986614;     zbCqcZQarDWJPAbDo68986614 = zbCqcZQarDWJPAbDo89376665;     zbCqcZQarDWJPAbDo89376665 = zbCqcZQarDWJPAbDo19603327;     zbCqcZQarDWJPAbDo19603327 = zbCqcZQarDWJPAbDo59085093;     zbCqcZQarDWJPAbDo59085093 = zbCqcZQarDWJPAbDo91980156;     zbCqcZQarDWJPAbDo91980156 = zbCqcZQarDWJPAbDo62600331;     zbCqcZQarDWJPAbDo62600331 = zbCqcZQarDWJPAbDo90534207;     zbCqcZQarDWJPAbDo90534207 = zbCqcZQarDWJPAbDo8722922;     zbCqcZQarDWJPAbDo8722922 = zbCqcZQarDWJPAbDo9917429;     zbCqcZQarDWJPAbDo9917429 = zbCqcZQarDWJPAbDo16090082;     zbCqcZQarDWJPAbDo16090082 = zbCqcZQarDWJPAbDo97160659;     zbCqcZQarDWJPAbDo97160659 = zbCqcZQarDWJPAbDo90967890;     zbCqcZQarDWJPAbDo90967890 = zbCqcZQarDWJPAbDo83695332;     zbCqcZQarDWJPAbDo83695332 = zbCqcZQarDWJPAbDo86220715;     zbCqcZQarDWJPAbDo86220715 = zbCqcZQarDWJPAbDo45293112;     zbCqcZQarDWJPAbDo45293112 = zbCqcZQarDWJPAbDo23819139;     zbCqcZQarDWJPAbDo23819139 = zbCqcZQarDWJPAbDo12277280;     zbCqcZQarDWJPAbDo12277280 = zbCqcZQarDWJPAbDo78697455;     zbCqcZQarDWJPAbDo78697455 = zbCqcZQarDWJPAbDo34589225;     zbCqcZQarDWJPAbDo34589225 = zbCqcZQarDWJPAbDo4334552;     zbCqcZQarDWJPAbDo4334552 = zbCqcZQarDWJPAbDo96615666;     zbCqcZQarDWJPAbDo96615666 = zbCqcZQarDWJPAbDo55443971;     zbCqcZQarDWJPAbDo55443971 = zbCqcZQarDWJPAbDo54723384;     zbCqcZQarDWJPAbDo54723384 = zbCqcZQarDWJPAbDo5448902;     zbCqcZQarDWJPAbDo5448902 = zbCqcZQarDWJPAbDo6273939;     zbCqcZQarDWJPAbDo6273939 = zbCqcZQarDWJPAbDo82155762;     zbCqcZQarDWJPAbDo82155762 = zbCqcZQarDWJPAbDo21256912;     zbCqcZQarDWJPAbDo21256912 = zbCqcZQarDWJPAbDo38609324;     zbCqcZQarDWJPAbDo38609324 = zbCqcZQarDWJPAbDo20105582;     zbCqcZQarDWJPAbDo20105582 = zbCqcZQarDWJPAbDo12624042;     zbCqcZQarDWJPAbDo12624042 = zbCqcZQarDWJPAbDo84916409;     zbCqcZQarDWJPAbDo84916409 = zbCqcZQarDWJPAbDo92382438;     zbCqcZQarDWJPAbDo92382438 = zbCqcZQarDWJPAbDo58685421;     zbCqcZQarDWJPAbDo58685421 = zbCqcZQarDWJPAbDo22276677;     zbCqcZQarDWJPAbDo22276677 = zbCqcZQarDWJPAbDo91070710;     zbCqcZQarDWJPAbDo91070710 = zbCqcZQarDWJPAbDo91325642;     zbCqcZQarDWJPAbDo91325642 = zbCqcZQarDWJPAbDo20398433;     zbCqcZQarDWJPAbDo20398433 = zbCqcZQarDWJPAbDo19684449;     zbCqcZQarDWJPAbDo19684449 = zbCqcZQarDWJPAbDo90169750;     zbCqcZQarDWJPAbDo90169750 = zbCqcZQarDWJPAbDo84930253;     zbCqcZQarDWJPAbDo84930253 = zbCqcZQarDWJPAbDo51204332;     zbCqcZQarDWJPAbDo51204332 = zbCqcZQarDWJPAbDo83132085;     zbCqcZQarDWJPAbDo83132085 = zbCqcZQarDWJPAbDo35297886;     zbCqcZQarDWJPAbDo35297886 = zbCqcZQarDWJPAbDo7356738;     zbCqcZQarDWJPAbDo7356738 = zbCqcZQarDWJPAbDo6695663;     zbCqcZQarDWJPAbDo6695663 = zbCqcZQarDWJPAbDo96069657;     zbCqcZQarDWJPAbDo96069657 = zbCqcZQarDWJPAbDo19910995;     zbCqcZQarDWJPAbDo19910995 = zbCqcZQarDWJPAbDo49576537;     zbCqcZQarDWJPAbDo49576537 = zbCqcZQarDWJPAbDo2640374;     zbCqcZQarDWJPAbDo2640374 = zbCqcZQarDWJPAbDo55553398;     zbCqcZQarDWJPAbDo55553398 = zbCqcZQarDWJPAbDo55480260;     zbCqcZQarDWJPAbDo55480260 = zbCqcZQarDWJPAbDo85327587;     zbCqcZQarDWJPAbDo85327587 = zbCqcZQarDWJPAbDo3210176;     zbCqcZQarDWJPAbDo3210176 = zbCqcZQarDWJPAbDo48135804;     zbCqcZQarDWJPAbDo48135804 = zbCqcZQarDWJPAbDo68737802;     zbCqcZQarDWJPAbDo68737802 = zbCqcZQarDWJPAbDo13542643;     zbCqcZQarDWJPAbDo13542643 = zbCqcZQarDWJPAbDo34653282;     zbCqcZQarDWJPAbDo34653282 = zbCqcZQarDWJPAbDo14154425;     zbCqcZQarDWJPAbDo14154425 = zbCqcZQarDWJPAbDo52811155;     zbCqcZQarDWJPAbDo52811155 = zbCqcZQarDWJPAbDo9824395;     zbCqcZQarDWJPAbDo9824395 = zbCqcZQarDWJPAbDo41343420;     zbCqcZQarDWJPAbDo41343420 = zbCqcZQarDWJPAbDo51924884;     zbCqcZQarDWJPAbDo51924884 = zbCqcZQarDWJPAbDo88617339;     zbCqcZQarDWJPAbDo88617339 = zbCqcZQarDWJPAbDo97293387;     zbCqcZQarDWJPAbDo97293387 = zbCqcZQarDWJPAbDo31173673;     zbCqcZQarDWJPAbDo31173673 = zbCqcZQarDWJPAbDo4778222;     zbCqcZQarDWJPAbDo4778222 = zbCqcZQarDWJPAbDo32282469;     zbCqcZQarDWJPAbDo32282469 = zbCqcZQarDWJPAbDo61418656;     zbCqcZQarDWJPAbDo61418656 = zbCqcZQarDWJPAbDo95150005;     zbCqcZQarDWJPAbDo95150005 = zbCqcZQarDWJPAbDo53967469;     zbCqcZQarDWJPAbDo53967469 = zbCqcZQarDWJPAbDo3420707;     zbCqcZQarDWJPAbDo3420707 = zbCqcZQarDWJPAbDo92592830;     zbCqcZQarDWJPAbDo92592830 = zbCqcZQarDWJPAbDo88527705;     zbCqcZQarDWJPAbDo88527705 = zbCqcZQarDWJPAbDo49658972;     zbCqcZQarDWJPAbDo49658972 = zbCqcZQarDWJPAbDo53130220;     zbCqcZQarDWJPAbDo53130220 = zbCqcZQarDWJPAbDo13483582;     zbCqcZQarDWJPAbDo13483582 = zbCqcZQarDWJPAbDo20146086;     zbCqcZQarDWJPAbDo20146086 = zbCqcZQarDWJPAbDo47366646;     zbCqcZQarDWJPAbDo47366646 = zbCqcZQarDWJPAbDo98753239;     zbCqcZQarDWJPAbDo98753239 = zbCqcZQarDWJPAbDo10204281;     zbCqcZQarDWJPAbDo10204281 = zbCqcZQarDWJPAbDo62244768;     zbCqcZQarDWJPAbDo62244768 = zbCqcZQarDWJPAbDo71680374;     zbCqcZQarDWJPAbDo71680374 = zbCqcZQarDWJPAbDo35968950;     zbCqcZQarDWJPAbDo35968950 = zbCqcZQarDWJPAbDo64552183;     zbCqcZQarDWJPAbDo64552183 = zbCqcZQarDWJPAbDo57143781;     zbCqcZQarDWJPAbDo57143781 = zbCqcZQarDWJPAbDo41738539;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void pcGJGEoAmMUxOQMW14822184() {     double GpUnhLengBWuThbKL77197507 = -370799857;    double GpUnhLengBWuThbKL5127086 = -999656718;    double GpUnhLengBWuThbKL82601791 = -549621284;    double GpUnhLengBWuThbKL95670173 = -232223451;    double GpUnhLengBWuThbKL60141592 = -252790106;    double GpUnhLengBWuThbKL23260420 = -638951930;    double GpUnhLengBWuThbKL14263734 = -776041657;    double GpUnhLengBWuThbKL97928813 = 91372092;    double GpUnhLengBWuThbKL49451090 = -604532493;    double GpUnhLengBWuThbKL94275248 = -185231832;    double GpUnhLengBWuThbKL5939275 = -809933802;    double GpUnhLengBWuThbKL64593191 = -924458572;    double GpUnhLengBWuThbKL76034046 = -33963027;    double GpUnhLengBWuThbKL5611219 = -138349699;    double GpUnhLengBWuThbKL23547985 = -382333165;    double GpUnhLengBWuThbKL52527915 = 65847463;    double GpUnhLengBWuThbKL45212601 = -883558829;    double GpUnhLengBWuThbKL19363519 = -474736218;    double GpUnhLengBWuThbKL59635423 = -781161480;    double GpUnhLengBWuThbKL85817375 = -853101456;    double GpUnhLengBWuThbKL75687194 = 34487031;    double GpUnhLengBWuThbKL93955732 = -21837599;    double GpUnhLengBWuThbKL79248301 = -48568944;    double GpUnhLengBWuThbKL3710375 = -465763183;    double GpUnhLengBWuThbKL24108033 = -416188770;    double GpUnhLengBWuThbKL7858832 = -761214673;    double GpUnhLengBWuThbKL51388464 = -946131149;    double GpUnhLengBWuThbKL24423758 = 99640872;    double GpUnhLengBWuThbKL32888930 = -5419188;    double GpUnhLengBWuThbKL50826523 = -511131284;    double GpUnhLengBWuThbKL42402370 = -785686993;    double GpUnhLengBWuThbKL50100298 = -597456146;    double GpUnhLengBWuThbKL90915466 = -882879256;    double GpUnhLengBWuThbKL64337651 = -623048144;    double GpUnhLengBWuThbKL70143005 = -603666895;    double GpUnhLengBWuThbKL48176510 = 57100244;    double GpUnhLengBWuThbKL4855788 = -625634671;    double GpUnhLengBWuThbKL5938788 = -895977987;    double GpUnhLengBWuThbKL35839463 = -457034959;    double GpUnhLengBWuThbKL70545470 = -495016349;    double GpUnhLengBWuThbKL15607371 = -800255223;    double GpUnhLengBWuThbKL10622039 = -311630385;    double GpUnhLengBWuThbKL27992702 = -463174940;    double GpUnhLengBWuThbKL60236920 = -437538886;    double GpUnhLengBWuThbKL12640500 = -234152672;    double GpUnhLengBWuThbKL30877738 = -830266440;    double GpUnhLengBWuThbKL11727602 = -580428900;    double GpUnhLengBWuThbKL76411732 = -383507195;    double GpUnhLengBWuThbKL45169343 = -698543102;    double GpUnhLengBWuThbKL15600300 = -980438798;    double GpUnhLengBWuThbKL72096366 = -832999833;    double GpUnhLengBWuThbKL3883905 = 76139924;    double GpUnhLengBWuThbKL88305597 = -895789042;    double GpUnhLengBWuThbKL6161597 = 45105905;    double GpUnhLengBWuThbKL39134989 = -309670273;    double GpUnhLengBWuThbKL83241775 = -248962258;    double GpUnhLengBWuThbKL25878784 = -851087775;    double GpUnhLengBWuThbKL78891417 = 16141898;    double GpUnhLengBWuThbKL71562141 = -816034682;    double GpUnhLengBWuThbKL52282760 = -491575433;    double GpUnhLengBWuThbKL71871955 = -692820782;    double GpUnhLengBWuThbKL89839975 = -775682530;    double GpUnhLengBWuThbKL65039883 = -903208720;    double GpUnhLengBWuThbKL98624567 = 6598790;    double GpUnhLengBWuThbKL51872879 = -399544840;    double GpUnhLengBWuThbKL55838976 = -112477656;    double GpUnhLengBWuThbKL73677724 = 58420683;    double GpUnhLengBWuThbKL11696395 = -410914884;    double GpUnhLengBWuThbKL35468213 = -534682804;    double GpUnhLengBWuThbKL75371474 = -339433409;    double GpUnhLengBWuThbKL47672127 = -308517867;    double GpUnhLengBWuThbKL39273814 = -987580842;    double GpUnhLengBWuThbKL83524055 = 82298741;    double GpUnhLengBWuThbKL89089952 = -186145132;    double GpUnhLengBWuThbKL70210005 = 47153767;    double GpUnhLengBWuThbKL65065156 = -653882585;    double GpUnhLengBWuThbKL65963030 = -558662660;    double GpUnhLengBWuThbKL19011382 = -611030058;    double GpUnhLengBWuThbKL91069874 = -131610512;    double GpUnhLengBWuThbKL93230295 = -585922331;    double GpUnhLengBWuThbKL96131229 = -80785773;    double GpUnhLengBWuThbKL74976731 = -462623955;    double GpUnhLengBWuThbKL79254414 = -201816026;    double GpUnhLengBWuThbKL17288631 = -24980391;    double GpUnhLengBWuThbKL78730156 = -678131451;    double GpUnhLengBWuThbKL38518466 = -761826917;    double GpUnhLengBWuThbKL61794701 = -701667104;    double GpUnhLengBWuThbKL84753869 = -827985161;    double GpUnhLengBWuThbKL25202662 = -213377872;    double GpUnhLengBWuThbKL86901229 = -254704637;    double GpUnhLengBWuThbKL22297727 = -91811982;    double GpUnhLengBWuThbKL25964371 = -541776570;    double GpUnhLengBWuThbKL34376646 = 20056694;    double GpUnhLengBWuThbKL83556703 = -965459526;    double GpUnhLengBWuThbKL98673515 = -802195568;    double GpUnhLengBWuThbKL25767395 = 75427306;    double GpUnhLengBWuThbKL45582155 = -408421665;    double GpUnhLengBWuThbKL29368135 = -369773731;    double GpUnhLengBWuThbKL8364041 = 62005953;    double GpUnhLengBWuThbKL56801524 = -370799857;     GpUnhLengBWuThbKL77197507 = GpUnhLengBWuThbKL5127086;     GpUnhLengBWuThbKL5127086 = GpUnhLengBWuThbKL82601791;     GpUnhLengBWuThbKL82601791 = GpUnhLengBWuThbKL95670173;     GpUnhLengBWuThbKL95670173 = GpUnhLengBWuThbKL60141592;     GpUnhLengBWuThbKL60141592 = GpUnhLengBWuThbKL23260420;     GpUnhLengBWuThbKL23260420 = GpUnhLengBWuThbKL14263734;     GpUnhLengBWuThbKL14263734 = GpUnhLengBWuThbKL97928813;     GpUnhLengBWuThbKL97928813 = GpUnhLengBWuThbKL49451090;     GpUnhLengBWuThbKL49451090 = GpUnhLengBWuThbKL94275248;     GpUnhLengBWuThbKL94275248 = GpUnhLengBWuThbKL5939275;     GpUnhLengBWuThbKL5939275 = GpUnhLengBWuThbKL64593191;     GpUnhLengBWuThbKL64593191 = GpUnhLengBWuThbKL76034046;     GpUnhLengBWuThbKL76034046 = GpUnhLengBWuThbKL5611219;     GpUnhLengBWuThbKL5611219 = GpUnhLengBWuThbKL23547985;     GpUnhLengBWuThbKL23547985 = GpUnhLengBWuThbKL52527915;     GpUnhLengBWuThbKL52527915 = GpUnhLengBWuThbKL45212601;     GpUnhLengBWuThbKL45212601 = GpUnhLengBWuThbKL19363519;     GpUnhLengBWuThbKL19363519 = GpUnhLengBWuThbKL59635423;     GpUnhLengBWuThbKL59635423 = GpUnhLengBWuThbKL85817375;     GpUnhLengBWuThbKL85817375 = GpUnhLengBWuThbKL75687194;     GpUnhLengBWuThbKL75687194 = GpUnhLengBWuThbKL93955732;     GpUnhLengBWuThbKL93955732 = GpUnhLengBWuThbKL79248301;     GpUnhLengBWuThbKL79248301 = GpUnhLengBWuThbKL3710375;     GpUnhLengBWuThbKL3710375 = GpUnhLengBWuThbKL24108033;     GpUnhLengBWuThbKL24108033 = GpUnhLengBWuThbKL7858832;     GpUnhLengBWuThbKL7858832 = GpUnhLengBWuThbKL51388464;     GpUnhLengBWuThbKL51388464 = GpUnhLengBWuThbKL24423758;     GpUnhLengBWuThbKL24423758 = GpUnhLengBWuThbKL32888930;     GpUnhLengBWuThbKL32888930 = GpUnhLengBWuThbKL50826523;     GpUnhLengBWuThbKL50826523 = GpUnhLengBWuThbKL42402370;     GpUnhLengBWuThbKL42402370 = GpUnhLengBWuThbKL50100298;     GpUnhLengBWuThbKL50100298 = GpUnhLengBWuThbKL90915466;     GpUnhLengBWuThbKL90915466 = GpUnhLengBWuThbKL64337651;     GpUnhLengBWuThbKL64337651 = GpUnhLengBWuThbKL70143005;     GpUnhLengBWuThbKL70143005 = GpUnhLengBWuThbKL48176510;     GpUnhLengBWuThbKL48176510 = GpUnhLengBWuThbKL4855788;     GpUnhLengBWuThbKL4855788 = GpUnhLengBWuThbKL5938788;     GpUnhLengBWuThbKL5938788 = GpUnhLengBWuThbKL35839463;     GpUnhLengBWuThbKL35839463 = GpUnhLengBWuThbKL70545470;     GpUnhLengBWuThbKL70545470 = GpUnhLengBWuThbKL15607371;     GpUnhLengBWuThbKL15607371 = GpUnhLengBWuThbKL10622039;     GpUnhLengBWuThbKL10622039 = GpUnhLengBWuThbKL27992702;     GpUnhLengBWuThbKL27992702 = GpUnhLengBWuThbKL60236920;     GpUnhLengBWuThbKL60236920 = GpUnhLengBWuThbKL12640500;     GpUnhLengBWuThbKL12640500 = GpUnhLengBWuThbKL30877738;     GpUnhLengBWuThbKL30877738 = GpUnhLengBWuThbKL11727602;     GpUnhLengBWuThbKL11727602 = GpUnhLengBWuThbKL76411732;     GpUnhLengBWuThbKL76411732 = GpUnhLengBWuThbKL45169343;     GpUnhLengBWuThbKL45169343 = GpUnhLengBWuThbKL15600300;     GpUnhLengBWuThbKL15600300 = GpUnhLengBWuThbKL72096366;     GpUnhLengBWuThbKL72096366 = GpUnhLengBWuThbKL3883905;     GpUnhLengBWuThbKL3883905 = GpUnhLengBWuThbKL88305597;     GpUnhLengBWuThbKL88305597 = GpUnhLengBWuThbKL6161597;     GpUnhLengBWuThbKL6161597 = GpUnhLengBWuThbKL39134989;     GpUnhLengBWuThbKL39134989 = GpUnhLengBWuThbKL83241775;     GpUnhLengBWuThbKL83241775 = GpUnhLengBWuThbKL25878784;     GpUnhLengBWuThbKL25878784 = GpUnhLengBWuThbKL78891417;     GpUnhLengBWuThbKL78891417 = GpUnhLengBWuThbKL71562141;     GpUnhLengBWuThbKL71562141 = GpUnhLengBWuThbKL52282760;     GpUnhLengBWuThbKL52282760 = GpUnhLengBWuThbKL71871955;     GpUnhLengBWuThbKL71871955 = GpUnhLengBWuThbKL89839975;     GpUnhLengBWuThbKL89839975 = GpUnhLengBWuThbKL65039883;     GpUnhLengBWuThbKL65039883 = GpUnhLengBWuThbKL98624567;     GpUnhLengBWuThbKL98624567 = GpUnhLengBWuThbKL51872879;     GpUnhLengBWuThbKL51872879 = GpUnhLengBWuThbKL55838976;     GpUnhLengBWuThbKL55838976 = GpUnhLengBWuThbKL73677724;     GpUnhLengBWuThbKL73677724 = GpUnhLengBWuThbKL11696395;     GpUnhLengBWuThbKL11696395 = GpUnhLengBWuThbKL35468213;     GpUnhLengBWuThbKL35468213 = GpUnhLengBWuThbKL75371474;     GpUnhLengBWuThbKL75371474 = GpUnhLengBWuThbKL47672127;     GpUnhLengBWuThbKL47672127 = GpUnhLengBWuThbKL39273814;     GpUnhLengBWuThbKL39273814 = GpUnhLengBWuThbKL83524055;     GpUnhLengBWuThbKL83524055 = GpUnhLengBWuThbKL89089952;     GpUnhLengBWuThbKL89089952 = GpUnhLengBWuThbKL70210005;     GpUnhLengBWuThbKL70210005 = GpUnhLengBWuThbKL65065156;     GpUnhLengBWuThbKL65065156 = GpUnhLengBWuThbKL65963030;     GpUnhLengBWuThbKL65963030 = GpUnhLengBWuThbKL19011382;     GpUnhLengBWuThbKL19011382 = GpUnhLengBWuThbKL91069874;     GpUnhLengBWuThbKL91069874 = GpUnhLengBWuThbKL93230295;     GpUnhLengBWuThbKL93230295 = GpUnhLengBWuThbKL96131229;     GpUnhLengBWuThbKL96131229 = GpUnhLengBWuThbKL74976731;     GpUnhLengBWuThbKL74976731 = GpUnhLengBWuThbKL79254414;     GpUnhLengBWuThbKL79254414 = GpUnhLengBWuThbKL17288631;     GpUnhLengBWuThbKL17288631 = GpUnhLengBWuThbKL78730156;     GpUnhLengBWuThbKL78730156 = GpUnhLengBWuThbKL38518466;     GpUnhLengBWuThbKL38518466 = GpUnhLengBWuThbKL61794701;     GpUnhLengBWuThbKL61794701 = GpUnhLengBWuThbKL84753869;     GpUnhLengBWuThbKL84753869 = GpUnhLengBWuThbKL25202662;     GpUnhLengBWuThbKL25202662 = GpUnhLengBWuThbKL86901229;     GpUnhLengBWuThbKL86901229 = GpUnhLengBWuThbKL22297727;     GpUnhLengBWuThbKL22297727 = GpUnhLengBWuThbKL25964371;     GpUnhLengBWuThbKL25964371 = GpUnhLengBWuThbKL34376646;     GpUnhLengBWuThbKL34376646 = GpUnhLengBWuThbKL83556703;     GpUnhLengBWuThbKL83556703 = GpUnhLengBWuThbKL98673515;     GpUnhLengBWuThbKL98673515 = GpUnhLengBWuThbKL25767395;     GpUnhLengBWuThbKL25767395 = GpUnhLengBWuThbKL45582155;     GpUnhLengBWuThbKL45582155 = GpUnhLengBWuThbKL29368135;     GpUnhLengBWuThbKL29368135 = GpUnhLengBWuThbKL8364041;     GpUnhLengBWuThbKL8364041 = GpUnhLengBWuThbKL56801524;     GpUnhLengBWuThbKL56801524 = GpUnhLengBWuThbKL77197507;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IzQYnjgdlyLDCoNg29871252() {     double prAhNBKWbBTAVSKna83314614 = -482701746;    double prAhNBKWbBTAVSKna48500364 = -709256499;    double prAhNBKWbBTAVSKna98643975 = -700454205;    double prAhNBKWbBTAVSKna22744955 = -422972081;    double prAhNBKWbBTAVSKna29037081 = -773439977;    double prAhNBKWbBTAVSKna14677622 = -903448165;    double prAhNBKWbBTAVSKna74752699 = -915006634;    double prAhNBKWbBTAVSKna47688705 = -195501821;    double prAhNBKWbBTAVSKna14093586 = -372565159;    double prAhNBKWbBTAVSKna35504908 = -514207309;    double prAhNBKWbBTAVSKna66049565 = -687371389;    double prAhNBKWbBTAVSKna96416233 = -268864774;    double prAhNBKWbBTAVSKna62243164 = -473854116;    double prAhNBKWbBTAVSKna88754895 = -670588267;    double prAhNBKWbBTAVSKna19290412 = 27204669;    double prAhNBKWbBTAVSKna94033308 = 92847920;    double prAhNBKWbBTAVSKna39048605 = -400103941;    double prAhNBKWbBTAVSKna16416672 = -140258861;    double prAhNBKWbBTAVSKna21465265 = -86930577;    double prAhNBKWbBTAVSKna66367540 = -110004375;    double prAhNBKWbBTAVSKna64240388 = -921000424;    double prAhNBKWbBTAVSKna80476755 = 70232684;    double prAhNBKWbBTAVSKna14839093 = -895086431;    double prAhNBKWbBTAVSKna87702748 = -688068171;    double prAhNBKWbBTAVSKna67807683 = -789384803;    double prAhNBKWbBTAVSKna32580507 = -639869996;    double prAhNBKWbBTAVSKna49848242 = -846793352;    double prAhNBKWbBTAVSKna37530883 = 33711919;    double prAhNBKWbBTAVSKna99021185 = -227801893;    double prAhNBKWbBTAVSKna19442160 = -328602151;    double prAhNBKWbBTAVSKna34644920 = -699007931;    double prAhNBKWbBTAVSKna47710920 = -306902962;    double prAhNBKWbBTAVSKna626415 = -775492626;    double prAhNBKWbBTAVSKna59168216 = -374589909;    double prAhNBKWbBTAVSKna31617657 = -142400000;    double prAhNBKWbBTAVSKna16902424 = -156925864;    double prAhNBKWbBTAVSKna86117722 = -579270540;    double prAhNBKWbBTAVSKna40029000 = -667483997;    double prAhNBKWbBTAVSKna9449717 = -450958456;    double prAhNBKWbBTAVSKna58047408 = -968455572;    double prAhNBKWbBTAVSKna21276595 = -865529117;    double prAhNBKWbBTAVSKna42772496 = -581772663;    double prAhNBKWbBTAVSKna6059680 = -543979524;    double prAhNBKWbBTAVSKna47644660 = -456881824;    double prAhNBKWbBTAVSKna24592965 = -996050826;    double prAhNBKWbBTAVSKna42816933 = -815482556;    double prAhNBKWbBTAVSKna29506176 = -699204438;    double prAhNBKWbBTAVSKna84368673 = -393402728;    double prAhNBKWbBTAVSKna54845072 = -784280090;    double prAhNBKWbBTAVSKna28511953 = 32550394;    double prAhNBKWbBTAVSKna7797045 = -675293505;    double prAhNBKWbBTAVSKna14693624 = -824586396;    double prAhNBKWbBTAVSKna36554055 = -687578630;    double prAhNBKWbBTAVSKna2130477 = -275364240;    double prAhNBKWbBTAVSKna65630864 = 89797945;    double prAhNBKWbBTAVSKna2837859 = -452934431;    double prAhNBKWbBTAVSKna33661272 = -814170069;    double prAhNBKWbBTAVSKna10941227 = 87613965;    double prAhNBKWbBTAVSKna54937272 = -633587279;    double prAhNBKWbBTAVSKna96456573 = -33569981;    double prAhNBKWbBTAVSKna64829379 = 43345187;    double prAhNBKWbBTAVSKna37221817 = -848718553;    double prAhNBKWbBTAVSKna48667519 = -967699929;    double prAhNBKWbBTAVSKna94651426 = 56036992;    double prAhNBKWbBTAVSKna859989 = -815199379;    double prAhNBKWbBTAVSKna18338645 = -280468427;    double prAhNBKWbBTAVSKna95789818 = -493372149;    double prAhNBKWbBTAVSKna3074949 = 735793;    double prAhNBKWbBTAVSKna57137238 = -428188267;    double prAhNBKWbBTAVSKna2387988 = -815869468;    double prAhNBKWbBTAVSKna7915587 = -327881540;    double prAhNBKWbBTAVSKna99019604 = -732619945;    double prAhNBKWbBTAVSKna6966955 = -689300405;    double prAhNBKWbBTAVSKna63417856 = -118475006;    double prAhNBKWbBTAVSKna45090946 = -244475258;    double prAhNBKWbBTAVSKna21467892 = -239227762;    double prAhNBKWbBTAVSKna74417075 = -385787792;    double prAhNBKWbBTAVSKna67194433 = -338204607;    double prAhNBKWbBTAVSKna63109783 = -692017345;    double prAhNBKWbBTAVSKna24990751 = -973902247;    double prAhNBKWbBTAVSKna3074331 = -940665559;    double prAhNBKWbBTAVSKna65479569 = -353390625;    double prAhNBKWbBTAVSKna82685811 = -182007992;    double prAhNBKWbBTAVSKna70509233 = -160352288;    double prAhNBKWbBTAVSKna11645115 = -653308647;    double prAhNBKWbBTAVSKna19951296 = -874421535;    double prAhNBKWbBTAVSKna11156866 = -619324332;    double prAhNBKWbBTAVSKna98495938 = -400128386;    double prAhNBKWbBTAVSKna93537351 = -364387855;    double prAhNBKWbBTAVSKna28779799 = -689465570;    double prAhNBKWbBTAVSKna83241152 = -342755795;    double prAhNBKWbBTAVSKna75176495 = -566884506;    double prAhNBKWbBTAVSKna85091728 = 66103282;    double prAhNBKWbBTAVSKna12993144 = -317388475;    double prAhNBKWbBTAVSKna93218028 = -911800759;    double prAhNBKWbBTAVSKna84054777 = 83189435;    double prAhNBKWbBTAVSKna94104976 = -614072734;    double prAhNBKWbBTAVSKna11408254 = -500016516;    double prAhNBKWbBTAVSKna46784671 = -641682446;    double prAhNBKWbBTAVSKna6254321 = -482701746;     prAhNBKWbBTAVSKna83314614 = prAhNBKWbBTAVSKna48500364;     prAhNBKWbBTAVSKna48500364 = prAhNBKWbBTAVSKna98643975;     prAhNBKWbBTAVSKna98643975 = prAhNBKWbBTAVSKna22744955;     prAhNBKWbBTAVSKna22744955 = prAhNBKWbBTAVSKna29037081;     prAhNBKWbBTAVSKna29037081 = prAhNBKWbBTAVSKna14677622;     prAhNBKWbBTAVSKna14677622 = prAhNBKWbBTAVSKna74752699;     prAhNBKWbBTAVSKna74752699 = prAhNBKWbBTAVSKna47688705;     prAhNBKWbBTAVSKna47688705 = prAhNBKWbBTAVSKna14093586;     prAhNBKWbBTAVSKna14093586 = prAhNBKWbBTAVSKna35504908;     prAhNBKWbBTAVSKna35504908 = prAhNBKWbBTAVSKna66049565;     prAhNBKWbBTAVSKna66049565 = prAhNBKWbBTAVSKna96416233;     prAhNBKWbBTAVSKna96416233 = prAhNBKWbBTAVSKna62243164;     prAhNBKWbBTAVSKna62243164 = prAhNBKWbBTAVSKna88754895;     prAhNBKWbBTAVSKna88754895 = prAhNBKWbBTAVSKna19290412;     prAhNBKWbBTAVSKna19290412 = prAhNBKWbBTAVSKna94033308;     prAhNBKWbBTAVSKna94033308 = prAhNBKWbBTAVSKna39048605;     prAhNBKWbBTAVSKna39048605 = prAhNBKWbBTAVSKna16416672;     prAhNBKWbBTAVSKna16416672 = prAhNBKWbBTAVSKna21465265;     prAhNBKWbBTAVSKna21465265 = prAhNBKWbBTAVSKna66367540;     prAhNBKWbBTAVSKna66367540 = prAhNBKWbBTAVSKna64240388;     prAhNBKWbBTAVSKna64240388 = prAhNBKWbBTAVSKna80476755;     prAhNBKWbBTAVSKna80476755 = prAhNBKWbBTAVSKna14839093;     prAhNBKWbBTAVSKna14839093 = prAhNBKWbBTAVSKna87702748;     prAhNBKWbBTAVSKna87702748 = prAhNBKWbBTAVSKna67807683;     prAhNBKWbBTAVSKna67807683 = prAhNBKWbBTAVSKna32580507;     prAhNBKWbBTAVSKna32580507 = prAhNBKWbBTAVSKna49848242;     prAhNBKWbBTAVSKna49848242 = prAhNBKWbBTAVSKna37530883;     prAhNBKWbBTAVSKna37530883 = prAhNBKWbBTAVSKna99021185;     prAhNBKWbBTAVSKna99021185 = prAhNBKWbBTAVSKna19442160;     prAhNBKWbBTAVSKna19442160 = prAhNBKWbBTAVSKna34644920;     prAhNBKWbBTAVSKna34644920 = prAhNBKWbBTAVSKna47710920;     prAhNBKWbBTAVSKna47710920 = prAhNBKWbBTAVSKna626415;     prAhNBKWbBTAVSKna626415 = prAhNBKWbBTAVSKna59168216;     prAhNBKWbBTAVSKna59168216 = prAhNBKWbBTAVSKna31617657;     prAhNBKWbBTAVSKna31617657 = prAhNBKWbBTAVSKna16902424;     prAhNBKWbBTAVSKna16902424 = prAhNBKWbBTAVSKna86117722;     prAhNBKWbBTAVSKna86117722 = prAhNBKWbBTAVSKna40029000;     prAhNBKWbBTAVSKna40029000 = prAhNBKWbBTAVSKna9449717;     prAhNBKWbBTAVSKna9449717 = prAhNBKWbBTAVSKna58047408;     prAhNBKWbBTAVSKna58047408 = prAhNBKWbBTAVSKna21276595;     prAhNBKWbBTAVSKna21276595 = prAhNBKWbBTAVSKna42772496;     prAhNBKWbBTAVSKna42772496 = prAhNBKWbBTAVSKna6059680;     prAhNBKWbBTAVSKna6059680 = prAhNBKWbBTAVSKna47644660;     prAhNBKWbBTAVSKna47644660 = prAhNBKWbBTAVSKna24592965;     prAhNBKWbBTAVSKna24592965 = prAhNBKWbBTAVSKna42816933;     prAhNBKWbBTAVSKna42816933 = prAhNBKWbBTAVSKna29506176;     prAhNBKWbBTAVSKna29506176 = prAhNBKWbBTAVSKna84368673;     prAhNBKWbBTAVSKna84368673 = prAhNBKWbBTAVSKna54845072;     prAhNBKWbBTAVSKna54845072 = prAhNBKWbBTAVSKna28511953;     prAhNBKWbBTAVSKna28511953 = prAhNBKWbBTAVSKna7797045;     prAhNBKWbBTAVSKna7797045 = prAhNBKWbBTAVSKna14693624;     prAhNBKWbBTAVSKna14693624 = prAhNBKWbBTAVSKna36554055;     prAhNBKWbBTAVSKna36554055 = prAhNBKWbBTAVSKna2130477;     prAhNBKWbBTAVSKna2130477 = prAhNBKWbBTAVSKna65630864;     prAhNBKWbBTAVSKna65630864 = prAhNBKWbBTAVSKna2837859;     prAhNBKWbBTAVSKna2837859 = prAhNBKWbBTAVSKna33661272;     prAhNBKWbBTAVSKna33661272 = prAhNBKWbBTAVSKna10941227;     prAhNBKWbBTAVSKna10941227 = prAhNBKWbBTAVSKna54937272;     prAhNBKWbBTAVSKna54937272 = prAhNBKWbBTAVSKna96456573;     prAhNBKWbBTAVSKna96456573 = prAhNBKWbBTAVSKna64829379;     prAhNBKWbBTAVSKna64829379 = prAhNBKWbBTAVSKna37221817;     prAhNBKWbBTAVSKna37221817 = prAhNBKWbBTAVSKna48667519;     prAhNBKWbBTAVSKna48667519 = prAhNBKWbBTAVSKna94651426;     prAhNBKWbBTAVSKna94651426 = prAhNBKWbBTAVSKna859989;     prAhNBKWbBTAVSKna859989 = prAhNBKWbBTAVSKna18338645;     prAhNBKWbBTAVSKna18338645 = prAhNBKWbBTAVSKna95789818;     prAhNBKWbBTAVSKna95789818 = prAhNBKWbBTAVSKna3074949;     prAhNBKWbBTAVSKna3074949 = prAhNBKWbBTAVSKna57137238;     prAhNBKWbBTAVSKna57137238 = prAhNBKWbBTAVSKna2387988;     prAhNBKWbBTAVSKna2387988 = prAhNBKWbBTAVSKna7915587;     prAhNBKWbBTAVSKna7915587 = prAhNBKWbBTAVSKna99019604;     prAhNBKWbBTAVSKna99019604 = prAhNBKWbBTAVSKna6966955;     prAhNBKWbBTAVSKna6966955 = prAhNBKWbBTAVSKna63417856;     prAhNBKWbBTAVSKna63417856 = prAhNBKWbBTAVSKna45090946;     prAhNBKWbBTAVSKna45090946 = prAhNBKWbBTAVSKna21467892;     prAhNBKWbBTAVSKna21467892 = prAhNBKWbBTAVSKna74417075;     prAhNBKWbBTAVSKna74417075 = prAhNBKWbBTAVSKna67194433;     prAhNBKWbBTAVSKna67194433 = prAhNBKWbBTAVSKna63109783;     prAhNBKWbBTAVSKna63109783 = prAhNBKWbBTAVSKna24990751;     prAhNBKWbBTAVSKna24990751 = prAhNBKWbBTAVSKna3074331;     prAhNBKWbBTAVSKna3074331 = prAhNBKWbBTAVSKna65479569;     prAhNBKWbBTAVSKna65479569 = prAhNBKWbBTAVSKna82685811;     prAhNBKWbBTAVSKna82685811 = prAhNBKWbBTAVSKna70509233;     prAhNBKWbBTAVSKna70509233 = prAhNBKWbBTAVSKna11645115;     prAhNBKWbBTAVSKna11645115 = prAhNBKWbBTAVSKna19951296;     prAhNBKWbBTAVSKna19951296 = prAhNBKWbBTAVSKna11156866;     prAhNBKWbBTAVSKna11156866 = prAhNBKWbBTAVSKna98495938;     prAhNBKWbBTAVSKna98495938 = prAhNBKWbBTAVSKna93537351;     prAhNBKWbBTAVSKna93537351 = prAhNBKWbBTAVSKna28779799;     prAhNBKWbBTAVSKna28779799 = prAhNBKWbBTAVSKna83241152;     prAhNBKWbBTAVSKna83241152 = prAhNBKWbBTAVSKna75176495;     prAhNBKWbBTAVSKna75176495 = prAhNBKWbBTAVSKna85091728;     prAhNBKWbBTAVSKna85091728 = prAhNBKWbBTAVSKna12993144;     prAhNBKWbBTAVSKna12993144 = prAhNBKWbBTAVSKna93218028;     prAhNBKWbBTAVSKna93218028 = prAhNBKWbBTAVSKna84054777;     prAhNBKWbBTAVSKna84054777 = prAhNBKWbBTAVSKna94104976;     prAhNBKWbBTAVSKna94104976 = prAhNBKWbBTAVSKna11408254;     prAhNBKWbBTAVSKna11408254 = prAhNBKWbBTAVSKna46784671;     prAhNBKWbBTAVSKna46784671 = prAhNBKWbBTAVSKna6254321;     prAhNBKWbBTAVSKna6254321 = prAhNBKWbBTAVSKna83314614;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zOfRMfEqaWUZDcxb97162851() {     double tOggxhUKXmbasIFoE18773583 = -948978646;    double tOggxhUKXmbasIFoE61772443 = -589808106;    double tOggxhUKXmbasIFoE36030452 = -366121280;    double tOggxhUKXmbasIFoE94968309 = -244123153;    double tOggxhUKXmbasIFoE85322349 = -879038822;    double tOggxhUKXmbasIFoE50900494 = -65796061;    double tOggxhUKXmbasIFoE85410105 = -759732368;    double tOggxhUKXmbasIFoE9820266 = -939735961;    double tOggxhUKXmbasIFoE15611191 = -860896566;    double tOggxhUKXmbasIFoE50407620 = -178894307;    double tOggxhUKXmbasIFoE4231300 = -489379118;    double tOggxhUKXmbasIFoE96984381 = -132985546;    double tOggxhUKXmbasIFoE477810 = 74133379;    double tOggxhUKXmbasIFoE41895758 = -646951676;    double tOggxhUKXmbasIFoE77484928 = -476220346;    double tOggxhUKXmbasIFoE77574609 = -642963687;    double tOggxhUKXmbasIFoE94884540 = -927078494;    double tOggxhUKXmbasIFoE16176863 = -213686929;    double tOggxhUKXmbasIFoE22015595 = -403493999;    double tOggxhUKXmbasIFoE60204759 = -536565441;    double tOggxhUKXmbasIFoE77327250 = -578275113;    double tOggxhUKXmbasIFoE83898280 = -16403610;    double tOggxhUKXmbasIFoE85364472 = -532718615;    double tOggxhUKXmbasIFoE81495693 = -98892775;    double tOggxhUKXmbasIFoE75825634 = -813370001;    double tOggxhUKXmbasIFoE43278679 = -328446803;    double tOggxhUKXmbasIFoE10268817 = -653077493;    double tOggxhUKXmbasIFoE78259307 = -136060256;    double tOggxhUKXmbasIFoE45689400 = -801896695;    double tOggxhUKXmbasIFoE24975571 = -363109322;    double tOggxhUKXmbasIFoE53228151 = -150988418;    double tOggxhUKXmbasIFoE85533938 = -79874376;    double tOggxhUKXmbasIFoE12844426 = -923088603;    double tOggxhUKXmbasIFoE88916642 = -439283723;    double tOggxhUKXmbasIFoE97426110 = -93500251;    double tOggxhUKXmbasIFoE68463267 = -371491169;    double tOggxhUKXmbasIFoE35529539 = -443966581;    double tOggxhUKXmbasIFoE91244403 = -650139578;    double tOggxhUKXmbasIFoE39840279 = -101834732;    double tOggxhUKXmbasIFoE22318941 = -433718118;    double tOggxhUKXmbasIFoE54728202 = -634728031;    double tOggxhUKXmbasIFoE32137623 = -176317954;    double tOggxhUKXmbasIFoE95443058 = -97160310;    double tOggxhUKXmbasIFoE87775998 = 38785766;    double tOggxhUKXmbasIFoE24609424 = -822561988;    double tOggxhUKXmbasIFoE88778261 = -854406188;    double tOggxhUKXmbasIFoE48851340 = -83190798;    double tOggxhUKXmbasIFoE2094986 = -559929321;    double tOggxhUKXmbasIFoE77737737 = -448016970;    double tOggxhUKXmbasIFoE53041542 = -458476421;    double tOggxhUKXmbasIFoE88567768 = -549570062;    double tOggxhUKXmbasIFoE98179096 = 61761257;    double tOggxhUKXmbasIFoE5175204 = -443624184;    double tOggxhUKXmbasIFoE18122324 = -466872740;    double tOggxhUKXmbasIFoE19835601 = -256692201;    double tOggxhUKXmbasIFoE34875302 = -832575037;    double tOggxhUKXmbasIFoE76407970 = 42910509;    double tOggxhUKXmbasIFoE54534759 = -167228506;    double tOggxhUKXmbasIFoE19142676 = -430753152;    double tOggxhUKXmbasIFoE42043671 = -450592020;    double tOggxhUKXmbasIFoE40631678 = -412718568;    double tOggxhUKXmbasIFoE7150798 = -523672113;    double tOggxhUKXmbasIFoE64130865 = -37839267;    double tOggxhUKXmbasIFoE90635620 = -397787244;    double tOggxhUKXmbasIFoE97179469 = 72094110;    double tOggxhUKXmbasIFoE18697362 = -309504742;    double tOggxhUKXmbasIFoE84139956 = -209896943;    double tOggxhUKXmbasIFoE11561168 = -486582899;    double tOggxhUKXmbasIFoE44469647 = -453451425;    double tOggxhUKXmbasIFoE9021661 = -4729177;    double tOggxhUKXmbasIFoE42045071 = -98997107;    double tOggxhUKXmbasIFoE3640138 = -176938917;    double tOggxhUKXmbasIFoE76336584 = -11852198;    double tOggxhUKXmbasIFoE99696654 = -969775881;    double tOggxhUKXmbasIFoE5476557 = -901837410;    double tOggxhUKXmbasIFoE45189628 = -301957160;    double tOggxhUKXmbasIFoE88455222 = -919243300;    double tOggxhUKXmbasIFoE97588474 = -471504381;    double tOggxhUKXmbasIFoE56886269 = -276330788;    double tOggxhUKXmbasIFoE87047373 = -958963813;    double tOggxhUKXmbasIFoE94427338 = -145256005;    double tOggxhUKXmbasIFoE8173831 = 6851828;    double tOggxhUKXmbasIFoE521571 = -688043286;    double tOggxhUKXmbasIFoE92647858 = -243420274;    double tOggxhUKXmbasIFoE36407802 = -813539260;    double tOggxhUKXmbasIFoE55049055 = -112749676;    double tOggxhUKXmbasIFoE80358735 = -636250193;    double tOggxhUKXmbasIFoE94722102 = -356215864;    double tOggxhUKXmbasIFoE69081042 = -82591522;    double tOggxhUKXmbasIFoE62550809 = -260925215;    double tOggxhUKXmbasIFoE92055297 = -314401679;    double tOggxhUKXmbasIFoE80994780 = -176738075;    double tOggxhUKXmbasIFoE72101727 = -119386427;    double tOggxhUKXmbasIFoE97796607 = -651242713;    double tOggxhUKXmbasIFoE81687263 = 79000450;    double tOggxhUKXmbasIFoE47577404 = -11055919;    double tOggxhUKXmbasIFoE68006757 = -38478687;    double tOggxhUKXmbasIFoE4807438 = -699373066;    double tOggxhUKXmbasIFoE90596529 = 66691655;    double tOggxhUKXmbasIFoE5912063 = -948978646;     tOggxhUKXmbasIFoE18773583 = tOggxhUKXmbasIFoE61772443;     tOggxhUKXmbasIFoE61772443 = tOggxhUKXmbasIFoE36030452;     tOggxhUKXmbasIFoE36030452 = tOggxhUKXmbasIFoE94968309;     tOggxhUKXmbasIFoE94968309 = tOggxhUKXmbasIFoE85322349;     tOggxhUKXmbasIFoE85322349 = tOggxhUKXmbasIFoE50900494;     tOggxhUKXmbasIFoE50900494 = tOggxhUKXmbasIFoE85410105;     tOggxhUKXmbasIFoE85410105 = tOggxhUKXmbasIFoE9820266;     tOggxhUKXmbasIFoE9820266 = tOggxhUKXmbasIFoE15611191;     tOggxhUKXmbasIFoE15611191 = tOggxhUKXmbasIFoE50407620;     tOggxhUKXmbasIFoE50407620 = tOggxhUKXmbasIFoE4231300;     tOggxhUKXmbasIFoE4231300 = tOggxhUKXmbasIFoE96984381;     tOggxhUKXmbasIFoE96984381 = tOggxhUKXmbasIFoE477810;     tOggxhUKXmbasIFoE477810 = tOggxhUKXmbasIFoE41895758;     tOggxhUKXmbasIFoE41895758 = tOggxhUKXmbasIFoE77484928;     tOggxhUKXmbasIFoE77484928 = tOggxhUKXmbasIFoE77574609;     tOggxhUKXmbasIFoE77574609 = tOggxhUKXmbasIFoE94884540;     tOggxhUKXmbasIFoE94884540 = tOggxhUKXmbasIFoE16176863;     tOggxhUKXmbasIFoE16176863 = tOggxhUKXmbasIFoE22015595;     tOggxhUKXmbasIFoE22015595 = tOggxhUKXmbasIFoE60204759;     tOggxhUKXmbasIFoE60204759 = tOggxhUKXmbasIFoE77327250;     tOggxhUKXmbasIFoE77327250 = tOggxhUKXmbasIFoE83898280;     tOggxhUKXmbasIFoE83898280 = tOggxhUKXmbasIFoE85364472;     tOggxhUKXmbasIFoE85364472 = tOggxhUKXmbasIFoE81495693;     tOggxhUKXmbasIFoE81495693 = tOggxhUKXmbasIFoE75825634;     tOggxhUKXmbasIFoE75825634 = tOggxhUKXmbasIFoE43278679;     tOggxhUKXmbasIFoE43278679 = tOggxhUKXmbasIFoE10268817;     tOggxhUKXmbasIFoE10268817 = tOggxhUKXmbasIFoE78259307;     tOggxhUKXmbasIFoE78259307 = tOggxhUKXmbasIFoE45689400;     tOggxhUKXmbasIFoE45689400 = tOggxhUKXmbasIFoE24975571;     tOggxhUKXmbasIFoE24975571 = tOggxhUKXmbasIFoE53228151;     tOggxhUKXmbasIFoE53228151 = tOggxhUKXmbasIFoE85533938;     tOggxhUKXmbasIFoE85533938 = tOggxhUKXmbasIFoE12844426;     tOggxhUKXmbasIFoE12844426 = tOggxhUKXmbasIFoE88916642;     tOggxhUKXmbasIFoE88916642 = tOggxhUKXmbasIFoE97426110;     tOggxhUKXmbasIFoE97426110 = tOggxhUKXmbasIFoE68463267;     tOggxhUKXmbasIFoE68463267 = tOggxhUKXmbasIFoE35529539;     tOggxhUKXmbasIFoE35529539 = tOggxhUKXmbasIFoE91244403;     tOggxhUKXmbasIFoE91244403 = tOggxhUKXmbasIFoE39840279;     tOggxhUKXmbasIFoE39840279 = tOggxhUKXmbasIFoE22318941;     tOggxhUKXmbasIFoE22318941 = tOggxhUKXmbasIFoE54728202;     tOggxhUKXmbasIFoE54728202 = tOggxhUKXmbasIFoE32137623;     tOggxhUKXmbasIFoE32137623 = tOggxhUKXmbasIFoE95443058;     tOggxhUKXmbasIFoE95443058 = tOggxhUKXmbasIFoE87775998;     tOggxhUKXmbasIFoE87775998 = tOggxhUKXmbasIFoE24609424;     tOggxhUKXmbasIFoE24609424 = tOggxhUKXmbasIFoE88778261;     tOggxhUKXmbasIFoE88778261 = tOggxhUKXmbasIFoE48851340;     tOggxhUKXmbasIFoE48851340 = tOggxhUKXmbasIFoE2094986;     tOggxhUKXmbasIFoE2094986 = tOggxhUKXmbasIFoE77737737;     tOggxhUKXmbasIFoE77737737 = tOggxhUKXmbasIFoE53041542;     tOggxhUKXmbasIFoE53041542 = tOggxhUKXmbasIFoE88567768;     tOggxhUKXmbasIFoE88567768 = tOggxhUKXmbasIFoE98179096;     tOggxhUKXmbasIFoE98179096 = tOggxhUKXmbasIFoE5175204;     tOggxhUKXmbasIFoE5175204 = tOggxhUKXmbasIFoE18122324;     tOggxhUKXmbasIFoE18122324 = tOggxhUKXmbasIFoE19835601;     tOggxhUKXmbasIFoE19835601 = tOggxhUKXmbasIFoE34875302;     tOggxhUKXmbasIFoE34875302 = tOggxhUKXmbasIFoE76407970;     tOggxhUKXmbasIFoE76407970 = tOggxhUKXmbasIFoE54534759;     tOggxhUKXmbasIFoE54534759 = tOggxhUKXmbasIFoE19142676;     tOggxhUKXmbasIFoE19142676 = tOggxhUKXmbasIFoE42043671;     tOggxhUKXmbasIFoE42043671 = tOggxhUKXmbasIFoE40631678;     tOggxhUKXmbasIFoE40631678 = tOggxhUKXmbasIFoE7150798;     tOggxhUKXmbasIFoE7150798 = tOggxhUKXmbasIFoE64130865;     tOggxhUKXmbasIFoE64130865 = tOggxhUKXmbasIFoE90635620;     tOggxhUKXmbasIFoE90635620 = tOggxhUKXmbasIFoE97179469;     tOggxhUKXmbasIFoE97179469 = tOggxhUKXmbasIFoE18697362;     tOggxhUKXmbasIFoE18697362 = tOggxhUKXmbasIFoE84139956;     tOggxhUKXmbasIFoE84139956 = tOggxhUKXmbasIFoE11561168;     tOggxhUKXmbasIFoE11561168 = tOggxhUKXmbasIFoE44469647;     tOggxhUKXmbasIFoE44469647 = tOggxhUKXmbasIFoE9021661;     tOggxhUKXmbasIFoE9021661 = tOggxhUKXmbasIFoE42045071;     tOggxhUKXmbasIFoE42045071 = tOggxhUKXmbasIFoE3640138;     tOggxhUKXmbasIFoE3640138 = tOggxhUKXmbasIFoE76336584;     tOggxhUKXmbasIFoE76336584 = tOggxhUKXmbasIFoE99696654;     tOggxhUKXmbasIFoE99696654 = tOggxhUKXmbasIFoE5476557;     tOggxhUKXmbasIFoE5476557 = tOggxhUKXmbasIFoE45189628;     tOggxhUKXmbasIFoE45189628 = tOggxhUKXmbasIFoE88455222;     tOggxhUKXmbasIFoE88455222 = tOggxhUKXmbasIFoE97588474;     tOggxhUKXmbasIFoE97588474 = tOggxhUKXmbasIFoE56886269;     tOggxhUKXmbasIFoE56886269 = tOggxhUKXmbasIFoE87047373;     tOggxhUKXmbasIFoE87047373 = tOggxhUKXmbasIFoE94427338;     tOggxhUKXmbasIFoE94427338 = tOggxhUKXmbasIFoE8173831;     tOggxhUKXmbasIFoE8173831 = tOggxhUKXmbasIFoE521571;     tOggxhUKXmbasIFoE521571 = tOggxhUKXmbasIFoE92647858;     tOggxhUKXmbasIFoE92647858 = tOggxhUKXmbasIFoE36407802;     tOggxhUKXmbasIFoE36407802 = tOggxhUKXmbasIFoE55049055;     tOggxhUKXmbasIFoE55049055 = tOggxhUKXmbasIFoE80358735;     tOggxhUKXmbasIFoE80358735 = tOggxhUKXmbasIFoE94722102;     tOggxhUKXmbasIFoE94722102 = tOggxhUKXmbasIFoE69081042;     tOggxhUKXmbasIFoE69081042 = tOggxhUKXmbasIFoE62550809;     tOggxhUKXmbasIFoE62550809 = tOggxhUKXmbasIFoE92055297;     tOggxhUKXmbasIFoE92055297 = tOggxhUKXmbasIFoE80994780;     tOggxhUKXmbasIFoE80994780 = tOggxhUKXmbasIFoE72101727;     tOggxhUKXmbasIFoE72101727 = tOggxhUKXmbasIFoE97796607;     tOggxhUKXmbasIFoE97796607 = tOggxhUKXmbasIFoE81687263;     tOggxhUKXmbasIFoE81687263 = tOggxhUKXmbasIFoE47577404;     tOggxhUKXmbasIFoE47577404 = tOggxhUKXmbasIFoE68006757;     tOggxhUKXmbasIFoE68006757 = tOggxhUKXmbasIFoE4807438;     tOggxhUKXmbasIFoE4807438 = tOggxhUKXmbasIFoE90596529;     tOggxhUKXmbasIFoE90596529 = tOggxhUKXmbasIFoE5912063;     tOggxhUKXmbasIFoE5912063 = tOggxhUKXmbasIFoE18773583;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void vgXBczlKkSpGNxZg12211919() {     double pusbwbVTqZmZXxJEX24890690 = 39119466;    double pusbwbVTqZmZXxJEX5145723 = -299407886;    double pusbwbVTqZmZXxJEX52072636 = -516954202;    double pusbwbVTqZmZXxJEX22043091 = -434871783;    double pusbwbVTqZmZXxJEX54217838 = -299688694;    double pusbwbVTqZmZXxJEX42317696 = -330292296;    double pusbwbVTqZmZXxJEX45899072 = -898697345;    double pusbwbVTqZmZXxJEX59580156 = -126609875;    double pusbwbVTqZmZXxJEX80253686 = -628929231;    double pusbwbVTqZmZXxJEX91637279 = -507869785;    double pusbwbVTqZmZXxJEX64341590 = -366816705;    double pusbwbVTqZmZXxJEX28807425 = -577391748;    double pusbwbVTqZmZXxJEX86686928 = -365757710;    double pusbwbVTqZmZXxJEX25039435 = -79190244;    double pusbwbVTqZmZXxJEX73227354 = -66682513;    double pusbwbVTqZmZXxJEX19080004 = -615963231;    double pusbwbVTqZmZXxJEX88720543 = -443623607;    double pusbwbVTqZmZXxJEX13230016 = -979209572;    double pusbwbVTqZmZXxJEX83845436 = -809263096;    double pusbwbVTqZmZXxJEX40754923 = -893468360;    double pusbwbVTqZmZXxJEX65880444 = -433762568;    double pusbwbVTqZmZXxJEX70419304 = 75666674;    double pusbwbVTqZmZXxJEX20955265 = -279236102;    double pusbwbVTqZmZXxJEX65488067 = -321197763;    double pusbwbVTqZmZXxJEX19525285 = -86566034;    double pusbwbVTqZmZXxJEX68000354 = -207102126;    double pusbwbVTqZmZXxJEX8728595 = -553739696;    double pusbwbVTqZmZXxJEX91366433 = -201989209;    double pusbwbVTqZmZXxJEX11821656 = 75720600;    double pusbwbVTqZmZXxJEX93591207 = -180580189;    double pusbwbVTqZmZXxJEX45470701 = -64309356;    double pusbwbVTqZmZXxJEX83144560 = -889321192;    double pusbwbVTqZmZXxJEX22555375 = -815701973;    double pusbwbVTqZmZXxJEX83747206 = -190825488;    double pusbwbVTqZmZXxJEX58900763 = -732233356;    double pusbwbVTqZmZXxJEX37189181 = -585517277;    double pusbwbVTqZmZXxJEX16791474 = -397602450;    double pusbwbVTqZmZXxJEX25334616 = -421645588;    double pusbwbVTqZmZXxJEX13450533 = -95758229;    double pusbwbVTqZmZXxJEX9820879 = -907157340;    double pusbwbVTqZmZXxJEX60397426 = -700001926;    double pusbwbVTqZmZXxJEX64288079 = -446460232;    double pusbwbVTqZmZXxJEX73510036 = -177964894;    double pusbwbVTqZmZXxJEX75183739 = 19442828;    double pusbwbVTqZmZXxJEX36561889 = -484460142;    double pusbwbVTqZmZXxJEX717457 = -839622305;    double pusbwbVTqZmZXxJEX66629914 = -201966336;    double pusbwbVTqZmZXxJEX10051927 = -569824854;    double pusbwbVTqZmZXxJEX87413466 = -533753958;    double pusbwbVTqZmZXxJEX65953194 = -545487230;    double pusbwbVTqZmZXxJEX24268447 = -391863733;    double pusbwbVTqZmZXxJEX8988816 = -838965063;    double pusbwbVTqZmZXxJEX53423661 = -235413772;    double pusbwbVTqZmZXxJEX14091204 = -787342885;    double pusbwbVTqZmZXxJEX46331475 = -957223983;    double pusbwbVTqZmZXxJEX54471385 = 63452791;    double pusbwbVTqZmZXxJEX84190457 = 79828215;    double pusbwbVTqZmZXxJEX86584568 = -95756439;    double pusbwbVTqZmZXxJEX2517807 = -248305749;    double pusbwbVTqZmZXxJEX86217484 = 7413432;    double pusbwbVTqZmZXxJEX33589102 = -776552600;    double pusbwbVTqZmZXxJEX54532638 = -596708136;    double pusbwbVTqZmZXxJEX47758501 = -102330475;    double pusbwbVTqZmZXxJEX86662479 = -348349043;    double pusbwbVTqZmZXxJEX46166579 = -343560429;    double pusbwbVTqZmZXxJEX81197029 = -477495513;    double pusbwbVTqZmZXxJEX6252050 = -761689775;    double pusbwbVTqZmZXxJEX2939723 = -74932222;    double pusbwbVTqZmZXxJEX66138672 = -346956888;    double pusbwbVTqZmZXxJEX36038174 = -481165236;    double pusbwbVTqZmZXxJEX2288530 = -118360781;    double pusbwbVTqZmZXxJEX63385928 = 78021980;    double pusbwbVTqZmZXxJEX99779483 = -783451343;    double pusbwbVTqZmZXxJEX74024558 = -902105756;    double pusbwbVTqZmZXxJEX80357497 = -93466435;    double pusbwbVTqZmZXxJEX1592365 = -987302337;    double pusbwbVTqZmZXxJEX96909267 = -746368433;    double pusbwbVTqZmZXxJEX45771526 = -198678930;    double pusbwbVTqZmZXxJEX28926179 = -836737621;    double pusbwbVTqZmZXxJEX18807829 = -246943730;    double pusbwbVTqZmZXxJEX1370440 = 94864209;    double pusbwbVTqZmZXxJEX98676668 = -983914843;    double pusbwbVTqZmZXxJEX3952967 = -668235252;    double pusbwbVTqZmZXxJEX45868462 = -378792171;    double pusbwbVTqZmZXxJEX69322760 = -788716456;    double pusbwbVTqZmZXxJEX36481885 = -225344294;    double pusbwbVTqZmZXxJEX29720900 = -553907420;    double pusbwbVTqZmZXxJEX8464171 = 71640911;    double pusbwbVTqZmZXxJEX37415731 = -233601505;    double pusbwbVTqZmZXxJEX4429378 = -695686148;    double pusbwbVTqZmZXxJEX52998723 = -565345493;    double pusbwbVTqZmZXxJEX30206905 = -201846012;    double pusbwbVTqZmZXxJEX22816810 = -73339839;    double pusbwbVTqZmZXxJEX27233048 = -3171662;    double pusbwbVTqZmZXxJEX76231776 = -30604741;    double pusbwbVTqZmZXxJEX5864788 = -3293790;    double pusbwbVTqZmZXxJEX16529579 = -244129757;    double pusbwbVTqZmZXxJEX86847557 = -829615852;    double pusbwbVTqZmZXxJEX29017160 = -636996744;    double pusbwbVTqZmZXxJEX55364859 = 39119466;     pusbwbVTqZmZXxJEX24890690 = pusbwbVTqZmZXxJEX5145723;     pusbwbVTqZmZXxJEX5145723 = pusbwbVTqZmZXxJEX52072636;     pusbwbVTqZmZXxJEX52072636 = pusbwbVTqZmZXxJEX22043091;     pusbwbVTqZmZXxJEX22043091 = pusbwbVTqZmZXxJEX54217838;     pusbwbVTqZmZXxJEX54217838 = pusbwbVTqZmZXxJEX42317696;     pusbwbVTqZmZXxJEX42317696 = pusbwbVTqZmZXxJEX45899072;     pusbwbVTqZmZXxJEX45899072 = pusbwbVTqZmZXxJEX59580156;     pusbwbVTqZmZXxJEX59580156 = pusbwbVTqZmZXxJEX80253686;     pusbwbVTqZmZXxJEX80253686 = pusbwbVTqZmZXxJEX91637279;     pusbwbVTqZmZXxJEX91637279 = pusbwbVTqZmZXxJEX64341590;     pusbwbVTqZmZXxJEX64341590 = pusbwbVTqZmZXxJEX28807425;     pusbwbVTqZmZXxJEX28807425 = pusbwbVTqZmZXxJEX86686928;     pusbwbVTqZmZXxJEX86686928 = pusbwbVTqZmZXxJEX25039435;     pusbwbVTqZmZXxJEX25039435 = pusbwbVTqZmZXxJEX73227354;     pusbwbVTqZmZXxJEX73227354 = pusbwbVTqZmZXxJEX19080004;     pusbwbVTqZmZXxJEX19080004 = pusbwbVTqZmZXxJEX88720543;     pusbwbVTqZmZXxJEX88720543 = pusbwbVTqZmZXxJEX13230016;     pusbwbVTqZmZXxJEX13230016 = pusbwbVTqZmZXxJEX83845436;     pusbwbVTqZmZXxJEX83845436 = pusbwbVTqZmZXxJEX40754923;     pusbwbVTqZmZXxJEX40754923 = pusbwbVTqZmZXxJEX65880444;     pusbwbVTqZmZXxJEX65880444 = pusbwbVTqZmZXxJEX70419304;     pusbwbVTqZmZXxJEX70419304 = pusbwbVTqZmZXxJEX20955265;     pusbwbVTqZmZXxJEX20955265 = pusbwbVTqZmZXxJEX65488067;     pusbwbVTqZmZXxJEX65488067 = pusbwbVTqZmZXxJEX19525285;     pusbwbVTqZmZXxJEX19525285 = pusbwbVTqZmZXxJEX68000354;     pusbwbVTqZmZXxJEX68000354 = pusbwbVTqZmZXxJEX8728595;     pusbwbVTqZmZXxJEX8728595 = pusbwbVTqZmZXxJEX91366433;     pusbwbVTqZmZXxJEX91366433 = pusbwbVTqZmZXxJEX11821656;     pusbwbVTqZmZXxJEX11821656 = pusbwbVTqZmZXxJEX93591207;     pusbwbVTqZmZXxJEX93591207 = pusbwbVTqZmZXxJEX45470701;     pusbwbVTqZmZXxJEX45470701 = pusbwbVTqZmZXxJEX83144560;     pusbwbVTqZmZXxJEX83144560 = pusbwbVTqZmZXxJEX22555375;     pusbwbVTqZmZXxJEX22555375 = pusbwbVTqZmZXxJEX83747206;     pusbwbVTqZmZXxJEX83747206 = pusbwbVTqZmZXxJEX58900763;     pusbwbVTqZmZXxJEX58900763 = pusbwbVTqZmZXxJEX37189181;     pusbwbVTqZmZXxJEX37189181 = pusbwbVTqZmZXxJEX16791474;     pusbwbVTqZmZXxJEX16791474 = pusbwbVTqZmZXxJEX25334616;     pusbwbVTqZmZXxJEX25334616 = pusbwbVTqZmZXxJEX13450533;     pusbwbVTqZmZXxJEX13450533 = pusbwbVTqZmZXxJEX9820879;     pusbwbVTqZmZXxJEX9820879 = pusbwbVTqZmZXxJEX60397426;     pusbwbVTqZmZXxJEX60397426 = pusbwbVTqZmZXxJEX64288079;     pusbwbVTqZmZXxJEX64288079 = pusbwbVTqZmZXxJEX73510036;     pusbwbVTqZmZXxJEX73510036 = pusbwbVTqZmZXxJEX75183739;     pusbwbVTqZmZXxJEX75183739 = pusbwbVTqZmZXxJEX36561889;     pusbwbVTqZmZXxJEX36561889 = pusbwbVTqZmZXxJEX717457;     pusbwbVTqZmZXxJEX717457 = pusbwbVTqZmZXxJEX66629914;     pusbwbVTqZmZXxJEX66629914 = pusbwbVTqZmZXxJEX10051927;     pusbwbVTqZmZXxJEX10051927 = pusbwbVTqZmZXxJEX87413466;     pusbwbVTqZmZXxJEX87413466 = pusbwbVTqZmZXxJEX65953194;     pusbwbVTqZmZXxJEX65953194 = pusbwbVTqZmZXxJEX24268447;     pusbwbVTqZmZXxJEX24268447 = pusbwbVTqZmZXxJEX8988816;     pusbwbVTqZmZXxJEX8988816 = pusbwbVTqZmZXxJEX53423661;     pusbwbVTqZmZXxJEX53423661 = pusbwbVTqZmZXxJEX14091204;     pusbwbVTqZmZXxJEX14091204 = pusbwbVTqZmZXxJEX46331475;     pusbwbVTqZmZXxJEX46331475 = pusbwbVTqZmZXxJEX54471385;     pusbwbVTqZmZXxJEX54471385 = pusbwbVTqZmZXxJEX84190457;     pusbwbVTqZmZXxJEX84190457 = pusbwbVTqZmZXxJEX86584568;     pusbwbVTqZmZXxJEX86584568 = pusbwbVTqZmZXxJEX2517807;     pusbwbVTqZmZXxJEX2517807 = pusbwbVTqZmZXxJEX86217484;     pusbwbVTqZmZXxJEX86217484 = pusbwbVTqZmZXxJEX33589102;     pusbwbVTqZmZXxJEX33589102 = pusbwbVTqZmZXxJEX54532638;     pusbwbVTqZmZXxJEX54532638 = pusbwbVTqZmZXxJEX47758501;     pusbwbVTqZmZXxJEX47758501 = pusbwbVTqZmZXxJEX86662479;     pusbwbVTqZmZXxJEX86662479 = pusbwbVTqZmZXxJEX46166579;     pusbwbVTqZmZXxJEX46166579 = pusbwbVTqZmZXxJEX81197029;     pusbwbVTqZmZXxJEX81197029 = pusbwbVTqZmZXxJEX6252050;     pusbwbVTqZmZXxJEX6252050 = pusbwbVTqZmZXxJEX2939723;     pusbwbVTqZmZXxJEX2939723 = pusbwbVTqZmZXxJEX66138672;     pusbwbVTqZmZXxJEX66138672 = pusbwbVTqZmZXxJEX36038174;     pusbwbVTqZmZXxJEX36038174 = pusbwbVTqZmZXxJEX2288530;     pusbwbVTqZmZXxJEX2288530 = pusbwbVTqZmZXxJEX63385928;     pusbwbVTqZmZXxJEX63385928 = pusbwbVTqZmZXxJEX99779483;     pusbwbVTqZmZXxJEX99779483 = pusbwbVTqZmZXxJEX74024558;     pusbwbVTqZmZXxJEX74024558 = pusbwbVTqZmZXxJEX80357497;     pusbwbVTqZmZXxJEX80357497 = pusbwbVTqZmZXxJEX1592365;     pusbwbVTqZmZXxJEX1592365 = pusbwbVTqZmZXxJEX96909267;     pusbwbVTqZmZXxJEX96909267 = pusbwbVTqZmZXxJEX45771526;     pusbwbVTqZmZXxJEX45771526 = pusbwbVTqZmZXxJEX28926179;     pusbwbVTqZmZXxJEX28926179 = pusbwbVTqZmZXxJEX18807829;     pusbwbVTqZmZXxJEX18807829 = pusbwbVTqZmZXxJEX1370440;     pusbwbVTqZmZXxJEX1370440 = pusbwbVTqZmZXxJEX98676668;     pusbwbVTqZmZXxJEX98676668 = pusbwbVTqZmZXxJEX3952967;     pusbwbVTqZmZXxJEX3952967 = pusbwbVTqZmZXxJEX45868462;     pusbwbVTqZmZXxJEX45868462 = pusbwbVTqZmZXxJEX69322760;     pusbwbVTqZmZXxJEX69322760 = pusbwbVTqZmZXxJEX36481885;     pusbwbVTqZmZXxJEX36481885 = pusbwbVTqZmZXxJEX29720900;     pusbwbVTqZmZXxJEX29720900 = pusbwbVTqZmZXxJEX8464171;     pusbwbVTqZmZXxJEX8464171 = pusbwbVTqZmZXxJEX37415731;     pusbwbVTqZmZXxJEX37415731 = pusbwbVTqZmZXxJEX4429378;     pusbwbVTqZmZXxJEX4429378 = pusbwbVTqZmZXxJEX52998723;     pusbwbVTqZmZXxJEX52998723 = pusbwbVTqZmZXxJEX30206905;     pusbwbVTqZmZXxJEX30206905 = pusbwbVTqZmZXxJEX22816810;     pusbwbVTqZmZXxJEX22816810 = pusbwbVTqZmZXxJEX27233048;     pusbwbVTqZmZXxJEX27233048 = pusbwbVTqZmZXxJEX76231776;     pusbwbVTqZmZXxJEX76231776 = pusbwbVTqZmZXxJEX5864788;     pusbwbVTqZmZXxJEX5864788 = pusbwbVTqZmZXxJEX16529579;     pusbwbVTqZmZXxJEX16529579 = pusbwbVTqZmZXxJEX86847557;     pusbwbVTqZmZXxJEX86847557 = pusbwbVTqZmZXxJEX29017160;     pusbwbVTqZmZXxJEX29017160 = pusbwbVTqZmZXxJEX55364859;     pusbwbVTqZmZXxJEX55364859 = pusbwbVTqZmZXxJEX24890690;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void weHJGCdJwYeHiHeZ79503518() {     double xOkvwmhuVdprzviCT60349658 = -427157434;    double xOkvwmhuVdprzviCT18417801 = -179959493;    double xOkvwmhuVdprzviCT89459112 = -182621277;    double xOkvwmhuVdprzviCT94266445 = -256022854;    double xOkvwmhuVdprzviCT10503108 = -405287539;    double xOkvwmhuVdprzviCT78540568 = -592640191;    double xOkvwmhuVdprzviCT56556478 = -743423079;    double xOkvwmhuVdprzviCT21711718 = -870844015;    double xOkvwmhuVdprzviCT81771291 = -17260638;    double xOkvwmhuVdprzviCT6539992 = -172556783;    double xOkvwmhuVdprzviCT2523325 = -168824434;    double xOkvwmhuVdprzviCT29375573 = -441512519;    double xOkvwmhuVdprzviCT24921574 = -917770215;    double xOkvwmhuVdprzviCT78180298 = -55553653;    double xOkvwmhuVdprzviCT31421871 = -570107528;    double xOkvwmhuVdprzviCT2621305 = -251774838;    double xOkvwmhuVdprzviCT44556479 = -970598160;    double xOkvwmhuVdprzviCT12990208 = 47362359;    double xOkvwmhuVdprzviCT84395766 = -25826517;    double xOkvwmhuVdprzviCT34592142 = -220029426;    double xOkvwmhuVdprzviCT78967307 = -91037257;    double xOkvwmhuVdprzviCT73840829 = -10969620;    double xOkvwmhuVdprzviCT91480644 = 83131714;    double xOkvwmhuVdprzviCT59281012 = -832022367;    double xOkvwmhuVdprzviCT27543236 = -110551233;    double xOkvwmhuVdprzviCT78698525 = -995678933;    double xOkvwmhuVdprzviCT69149168 = -360023837;    double xOkvwmhuVdprzviCT32094858 = -371761384;    double xOkvwmhuVdprzviCT58489870 = -498374202;    double xOkvwmhuVdprzviCT99124617 = -215087360;    double xOkvwmhuVdprzviCT64053932 = -616289843;    double xOkvwmhuVdprzviCT20967579 = -662292606;    double xOkvwmhuVdprzviCT34773385 = -963297951;    double xOkvwmhuVdprzviCT13495633 = -255519301;    double xOkvwmhuVdprzviCT24709217 = -683333607;    double xOkvwmhuVdprzviCT88750024 = -800082583;    double xOkvwmhuVdprzviCT66203290 = -262298490;    double xOkvwmhuVdprzviCT76550019 = -404301169;    double xOkvwmhuVdprzviCT43841094 = -846634506;    double xOkvwmhuVdprzviCT74092410 = -372419886;    double xOkvwmhuVdprzviCT93849033 = -469200840;    double xOkvwmhuVdprzviCT53653206 = -41005523;    double xOkvwmhuVdprzviCT62893415 = -831145680;    double xOkvwmhuVdprzviCT15315077 = -584889582;    double xOkvwmhuVdprzviCT36578348 = -310971304;    double xOkvwmhuVdprzviCT46678785 = -878545937;    double xOkvwmhuVdprzviCT85975078 = -685952696;    double xOkvwmhuVdprzviCT27778238 = -736351447;    double xOkvwmhuVdprzviCT10306133 = -197490838;    double xOkvwmhuVdprzviCT90482783 = 63485955;    double xOkvwmhuVdprzviCT5039171 = -266140291;    double xOkvwmhuVdprzviCT92474288 = 47382591;    double xOkvwmhuVdprzviCT22044810 = 8540674;    double xOkvwmhuVdprzviCT30083051 = -978851384;    double xOkvwmhuVdprzviCT536212 = -203714129;    double xOkvwmhuVdprzviCT86508829 = -316187815;    double xOkvwmhuVdprzviCT26937157 = -163091207;    double xOkvwmhuVdprzviCT30178100 = -350598910;    double xOkvwmhuVdprzviCT66723210 = -45471622;    double xOkvwmhuVdprzviCT31804582 = -409608606;    double xOkvwmhuVdprzviCT9391400 = -132616355;    double xOkvwmhuVdprzviCT24461620 = -271661696;    double xOkvwmhuVdprzviCT63221847 = -272469813;    double xOkvwmhuVdprzviCT82646673 = -802173279;    double xOkvwmhuVdprzviCT42486060 = -556266940;    double xOkvwmhuVdprzviCT81555746 = -506531828;    double xOkvwmhuVdprzviCT94602187 = -478214569;    double xOkvwmhuVdprzviCT11425941 = -562250914;    double xOkvwmhuVdprzviCT53471082 = -372220047;    double xOkvwmhuVdprzviCT42671847 = -770024945;    double xOkvwmhuVdprzviCT36418014 = -989476348;    double xOkvwmhuVdprzviCT68006460 = -466296991;    double xOkvwmhuVdprzviCT69149114 = -106003136;    double xOkvwmhuVdprzviCT10303356 = -653406631;    double xOkvwmhuVdprzviCT40743108 = -750828587;    double xOkvwmhuVdprzviCT25314101 = 49968265;    double xOkvwmhuVdprzviCT10947414 = -179823941;    double xOkvwmhuVdprzviCT76165567 = -331978704;    double xOkvwmhuVdprzviCT22702665 = -421051064;    double xOkvwmhuVdprzviCT80864451 = -232005296;    double xOkvwmhuVdprzviCT92723447 = -209726237;    double xOkvwmhuVdprzviCT41370930 = -623672390;    double xOkvwmhuVdprzviCT21788726 = -74270546;    double xOkvwmhuVdprzviCT68007087 = -461860157;    double xOkvwmhuVdprzviCT94085447 = -948947069;    double xOkvwmhuVdprzviCT71579644 = -563672434;    double xOkvwmhuVdprzviCT98922769 = -570833281;    double xOkvwmhuVdprzviCT4690335 = -984446567;    double xOkvwmhuVdprzviCT12959422 = 48194827;    double xOkvwmhuVdprzviCT38200388 = -267145793;    double xOkvwmhuVdprzviCT61812868 = -536991376;    double xOkvwmhuVdprzviCT36025190 = -911699581;    double xOkvwmhuVdprzviCT9826810 = -258829547;    double xOkvwmhuVdprzviCT12036513 = -337025900;    double xOkvwmhuVdprzviCT64701011 = -139803532;    double xOkvwmhuVdprzviCT69387413 = -97539144;    double xOkvwmhuVdprzviCT90431359 = -768535710;    double xOkvwmhuVdprzviCT80246741 = 71027599;    double xOkvwmhuVdprzviCT72829017 = 71377357;    double xOkvwmhuVdprzviCT55022601 = -427157434;     xOkvwmhuVdprzviCT60349658 = xOkvwmhuVdprzviCT18417801;     xOkvwmhuVdprzviCT18417801 = xOkvwmhuVdprzviCT89459112;     xOkvwmhuVdprzviCT89459112 = xOkvwmhuVdprzviCT94266445;     xOkvwmhuVdprzviCT94266445 = xOkvwmhuVdprzviCT10503108;     xOkvwmhuVdprzviCT10503108 = xOkvwmhuVdprzviCT78540568;     xOkvwmhuVdprzviCT78540568 = xOkvwmhuVdprzviCT56556478;     xOkvwmhuVdprzviCT56556478 = xOkvwmhuVdprzviCT21711718;     xOkvwmhuVdprzviCT21711718 = xOkvwmhuVdprzviCT81771291;     xOkvwmhuVdprzviCT81771291 = xOkvwmhuVdprzviCT6539992;     xOkvwmhuVdprzviCT6539992 = xOkvwmhuVdprzviCT2523325;     xOkvwmhuVdprzviCT2523325 = xOkvwmhuVdprzviCT29375573;     xOkvwmhuVdprzviCT29375573 = xOkvwmhuVdprzviCT24921574;     xOkvwmhuVdprzviCT24921574 = xOkvwmhuVdprzviCT78180298;     xOkvwmhuVdprzviCT78180298 = xOkvwmhuVdprzviCT31421871;     xOkvwmhuVdprzviCT31421871 = xOkvwmhuVdprzviCT2621305;     xOkvwmhuVdprzviCT2621305 = xOkvwmhuVdprzviCT44556479;     xOkvwmhuVdprzviCT44556479 = xOkvwmhuVdprzviCT12990208;     xOkvwmhuVdprzviCT12990208 = xOkvwmhuVdprzviCT84395766;     xOkvwmhuVdprzviCT84395766 = xOkvwmhuVdprzviCT34592142;     xOkvwmhuVdprzviCT34592142 = xOkvwmhuVdprzviCT78967307;     xOkvwmhuVdprzviCT78967307 = xOkvwmhuVdprzviCT73840829;     xOkvwmhuVdprzviCT73840829 = xOkvwmhuVdprzviCT91480644;     xOkvwmhuVdprzviCT91480644 = xOkvwmhuVdprzviCT59281012;     xOkvwmhuVdprzviCT59281012 = xOkvwmhuVdprzviCT27543236;     xOkvwmhuVdprzviCT27543236 = xOkvwmhuVdprzviCT78698525;     xOkvwmhuVdprzviCT78698525 = xOkvwmhuVdprzviCT69149168;     xOkvwmhuVdprzviCT69149168 = xOkvwmhuVdprzviCT32094858;     xOkvwmhuVdprzviCT32094858 = xOkvwmhuVdprzviCT58489870;     xOkvwmhuVdprzviCT58489870 = xOkvwmhuVdprzviCT99124617;     xOkvwmhuVdprzviCT99124617 = xOkvwmhuVdprzviCT64053932;     xOkvwmhuVdprzviCT64053932 = xOkvwmhuVdprzviCT20967579;     xOkvwmhuVdprzviCT20967579 = xOkvwmhuVdprzviCT34773385;     xOkvwmhuVdprzviCT34773385 = xOkvwmhuVdprzviCT13495633;     xOkvwmhuVdprzviCT13495633 = xOkvwmhuVdprzviCT24709217;     xOkvwmhuVdprzviCT24709217 = xOkvwmhuVdprzviCT88750024;     xOkvwmhuVdprzviCT88750024 = xOkvwmhuVdprzviCT66203290;     xOkvwmhuVdprzviCT66203290 = xOkvwmhuVdprzviCT76550019;     xOkvwmhuVdprzviCT76550019 = xOkvwmhuVdprzviCT43841094;     xOkvwmhuVdprzviCT43841094 = xOkvwmhuVdprzviCT74092410;     xOkvwmhuVdprzviCT74092410 = xOkvwmhuVdprzviCT93849033;     xOkvwmhuVdprzviCT93849033 = xOkvwmhuVdprzviCT53653206;     xOkvwmhuVdprzviCT53653206 = xOkvwmhuVdprzviCT62893415;     xOkvwmhuVdprzviCT62893415 = xOkvwmhuVdprzviCT15315077;     xOkvwmhuVdprzviCT15315077 = xOkvwmhuVdprzviCT36578348;     xOkvwmhuVdprzviCT36578348 = xOkvwmhuVdprzviCT46678785;     xOkvwmhuVdprzviCT46678785 = xOkvwmhuVdprzviCT85975078;     xOkvwmhuVdprzviCT85975078 = xOkvwmhuVdprzviCT27778238;     xOkvwmhuVdprzviCT27778238 = xOkvwmhuVdprzviCT10306133;     xOkvwmhuVdprzviCT10306133 = xOkvwmhuVdprzviCT90482783;     xOkvwmhuVdprzviCT90482783 = xOkvwmhuVdprzviCT5039171;     xOkvwmhuVdprzviCT5039171 = xOkvwmhuVdprzviCT92474288;     xOkvwmhuVdprzviCT92474288 = xOkvwmhuVdprzviCT22044810;     xOkvwmhuVdprzviCT22044810 = xOkvwmhuVdprzviCT30083051;     xOkvwmhuVdprzviCT30083051 = xOkvwmhuVdprzviCT536212;     xOkvwmhuVdprzviCT536212 = xOkvwmhuVdprzviCT86508829;     xOkvwmhuVdprzviCT86508829 = xOkvwmhuVdprzviCT26937157;     xOkvwmhuVdprzviCT26937157 = xOkvwmhuVdprzviCT30178100;     xOkvwmhuVdprzviCT30178100 = xOkvwmhuVdprzviCT66723210;     xOkvwmhuVdprzviCT66723210 = xOkvwmhuVdprzviCT31804582;     xOkvwmhuVdprzviCT31804582 = xOkvwmhuVdprzviCT9391400;     xOkvwmhuVdprzviCT9391400 = xOkvwmhuVdprzviCT24461620;     xOkvwmhuVdprzviCT24461620 = xOkvwmhuVdprzviCT63221847;     xOkvwmhuVdprzviCT63221847 = xOkvwmhuVdprzviCT82646673;     xOkvwmhuVdprzviCT82646673 = xOkvwmhuVdprzviCT42486060;     xOkvwmhuVdprzviCT42486060 = xOkvwmhuVdprzviCT81555746;     xOkvwmhuVdprzviCT81555746 = xOkvwmhuVdprzviCT94602187;     xOkvwmhuVdprzviCT94602187 = xOkvwmhuVdprzviCT11425941;     xOkvwmhuVdprzviCT11425941 = xOkvwmhuVdprzviCT53471082;     xOkvwmhuVdprzviCT53471082 = xOkvwmhuVdprzviCT42671847;     xOkvwmhuVdprzviCT42671847 = xOkvwmhuVdprzviCT36418014;     xOkvwmhuVdprzviCT36418014 = xOkvwmhuVdprzviCT68006460;     xOkvwmhuVdprzviCT68006460 = xOkvwmhuVdprzviCT69149114;     xOkvwmhuVdprzviCT69149114 = xOkvwmhuVdprzviCT10303356;     xOkvwmhuVdprzviCT10303356 = xOkvwmhuVdprzviCT40743108;     xOkvwmhuVdprzviCT40743108 = xOkvwmhuVdprzviCT25314101;     xOkvwmhuVdprzviCT25314101 = xOkvwmhuVdprzviCT10947414;     xOkvwmhuVdprzviCT10947414 = xOkvwmhuVdprzviCT76165567;     xOkvwmhuVdprzviCT76165567 = xOkvwmhuVdprzviCT22702665;     xOkvwmhuVdprzviCT22702665 = xOkvwmhuVdprzviCT80864451;     xOkvwmhuVdprzviCT80864451 = xOkvwmhuVdprzviCT92723447;     xOkvwmhuVdprzviCT92723447 = xOkvwmhuVdprzviCT41370930;     xOkvwmhuVdprzviCT41370930 = xOkvwmhuVdprzviCT21788726;     xOkvwmhuVdprzviCT21788726 = xOkvwmhuVdprzviCT68007087;     xOkvwmhuVdprzviCT68007087 = xOkvwmhuVdprzviCT94085447;     xOkvwmhuVdprzviCT94085447 = xOkvwmhuVdprzviCT71579644;     xOkvwmhuVdprzviCT71579644 = xOkvwmhuVdprzviCT98922769;     xOkvwmhuVdprzviCT98922769 = xOkvwmhuVdprzviCT4690335;     xOkvwmhuVdprzviCT4690335 = xOkvwmhuVdprzviCT12959422;     xOkvwmhuVdprzviCT12959422 = xOkvwmhuVdprzviCT38200388;     xOkvwmhuVdprzviCT38200388 = xOkvwmhuVdprzviCT61812868;     xOkvwmhuVdprzviCT61812868 = xOkvwmhuVdprzviCT36025190;     xOkvwmhuVdprzviCT36025190 = xOkvwmhuVdprzviCT9826810;     xOkvwmhuVdprzviCT9826810 = xOkvwmhuVdprzviCT12036513;     xOkvwmhuVdprzviCT12036513 = xOkvwmhuVdprzviCT64701011;     xOkvwmhuVdprzviCT64701011 = xOkvwmhuVdprzviCT69387413;     xOkvwmhuVdprzviCT69387413 = xOkvwmhuVdprzviCT90431359;     xOkvwmhuVdprzviCT90431359 = xOkvwmhuVdprzviCT80246741;     xOkvwmhuVdprzviCT80246741 = xOkvwmhuVdprzviCT72829017;     xOkvwmhuVdprzviCT72829017 = xOkvwmhuVdprzviCT55022601;     xOkvwmhuVdprzviCT55022601 = xOkvwmhuVdprzviCT60349658;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void lRpJlFcbyxNnDVvG94552586() {     double oMzeYjqHJiTaPaEIE66466764 = -539059323;    double oMzeYjqHJiTaPaEIE61791080 = -989559274;    double oMzeYjqHJiTaPaEIE5501296 = -333454198;    double oMzeYjqHJiTaPaEIE21341226 = -446771485;    double oMzeYjqHJiTaPaEIE79398596 = -925937410;    double oMzeYjqHJiTaPaEIE69957770 = -857136426;    double oMzeYjqHJiTaPaEIE17045444 = -882388057;    double oMzeYjqHJiTaPaEIE71471608 = -57717928;    double oMzeYjqHJiTaPaEIE46413787 = -885293304;    double oMzeYjqHJiTaPaEIE47769651 = -501532260;    double oMzeYjqHJiTaPaEIE62633615 = -46262020;    double oMzeYjqHJiTaPaEIE61198615 = -885918721;    double oMzeYjqHJiTaPaEIE11130693 = -257661303;    double oMzeYjqHJiTaPaEIE61323975 = -587792221;    double oMzeYjqHJiTaPaEIE27164298 = -160569694;    double oMzeYjqHJiTaPaEIE44126698 = -224774381;    double oMzeYjqHJiTaPaEIE38392483 = -487143272;    double oMzeYjqHJiTaPaEIE10043361 = -718160284;    double oMzeYjqHJiTaPaEIE46225608 = -431595614;    double oMzeYjqHJiTaPaEIE15142307 = -576932346;    double oMzeYjqHJiTaPaEIE67520500 = 53475287;    double oMzeYjqHJiTaPaEIE60361852 = 81100664;    double oMzeYjqHJiTaPaEIE27071436 = -763385773;    double oMzeYjqHJiTaPaEIE43273386 = 45672645;    double oMzeYjqHJiTaPaEIE71242885 = -483747266;    double oMzeYjqHJiTaPaEIE3420202 = -874334256;    double oMzeYjqHJiTaPaEIE67608947 = -260686040;    double oMzeYjqHJiTaPaEIE45201983 = -437690337;    double oMzeYjqHJiTaPaEIE24622126 = -720756907;    double oMzeYjqHJiTaPaEIE67740254 = -32558227;    double oMzeYjqHJiTaPaEIE56296482 = -529610781;    double oMzeYjqHJiTaPaEIE18578201 = -371739422;    double oMzeYjqHJiTaPaEIE44484334 = -855911321;    double oMzeYjqHJiTaPaEIE8326197 = -7061067;    double oMzeYjqHJiTaPaEIE86183868 = -222066712;    double oMzeYjqHJiTaPaEIE57475937 = 85891309;    double oMzeYjqHJiTaPaEIE47465225 = -215934360;    double oMzeYjqHJiTaPaEIE10640232 = -175807178;    double oMzeYjqHJiTaPaEIE17451348 = -840558002;    double oMzeYjqHJiTaPaEIE61594348 = -845859109;    double oMzeYjqHJiTaPaEIE99518257 = -534474734;    double oMzeYjqHJiTaPaEIE85803663 = -311147801;    double oMzeYjqHJiTaPaEIE40960393 = -911950264;    double oMzeYjqHJiTaPaEIE2722818 = -604232520;    double oMzeYjqHJiTaPaEIE48530812 = 27130542;    double oMzeYjqHJiTaPaEIE58617979 = -863762053;    double oMzeYjqHJiTaPaEIE3753653 = -804728234;    double oMzeYjqHJiTaPaEIE35735179 = -746246979;    double oMzeYjqHJiTaPaEIE19981862 = -283227827;    double oMzeYjqHJiTaPaEIE3394437 = -23524853;    double oMzeYjqHJiTaPaEIE40739849 = -108433962;    double oMzeYjqHJiTaPaEIE3284008 = -853343729;    double oMzeYjqHJiTaPaEIE70293266 = -883248914;    double oMzeYjqHJiTaPaEIE26051931 = -199321530;    double oMzeYjqHJiTaPaEIE27032086 = -904245911;    double oMzeYjqHJiTaPaEIE6104912 = -520159987;    double oMzeYjqHJiTaPaEIE34719644 = -126173501;    double oMzeYjqHJiTaPaEIE62227910 = -279126843;    double oMzeYjqHJiTaPaEIE50098340 = -963024220;    double oMzeYjqHJiTaPaEIE75978395 = 48396846;    double oMzeYjqHJiTaPaEIE2348824 = -496450387;    double oMzeYjqHJiTaPaEIE71843460 = -344697720;    double oMzeYjqHJiTaPaEIE46849482 = -336961022;    double oMzeYjqHJiTaPaEIE78673532 = -752735077;    double oMzeYjqHJiTaPaEIE91473169 = -971921480;    double oMzeYjqHJiTaPaEIE44055415 = -674522599;    double oMzeYjqHJiTaPaEIE16714282 = 69992600;    double oMzeYjqHJiTaPaEIE2804496 = -150600237;    double oMzeYjqHJiTaPaEIE75140106 = -265725509;    double oMzeYjqHJiTaPaEIE69688360 = -146461004;    double oMzeYjqHJiTaPaEIE96661473 = 91159979;    double oMzeYjqHJiTaPaEIE27752251 = -211336095;    double oMzeYjqHJiTaPaEIE92592013 = -877602282;    double oMzeYjqHJiTaPaEIE84631260 = -585736505;    double oMzeYjqHJiTaPaEIE15624049 = 57542388;    double oMzeYjqHJiTaPaEIE81716837 = -635376912;    double oMzeYjqHJiTaPaEIE19401460 = -6949073;    double oMzeYjqHJiTaPaEIE24348619 = -59153253;    double oMzeYjqHJiTaPaEIE94742573 = -981457898;    double oMzeYjqHJiTaPaEIE12624906 = -619985213;    double oMzeYjqHJiTaPaEIE99666548 = 30393977;    double oMzeYjqHJiTaPaEIE31873768 = -514439061;    double oMzeYjqHJiTaPaEIE25220122 = -54462511;    double oMzeYjqHJiTaPaEIE21227690 = -597232054;    double oMzeYjqHJiTaPaEIE27000406 = -924124266;    double oMzeYjqHJiTaPaEIE53012474 = -676267052;    double oMzeYjqHJiTaPaEIE48284934 = -488490509;    double oMzeYjqHJiTaPaEIE18432403 = -556589792;    double oMzeYjqHJiTaPaEIE81294111 = -102815156;    double oMzeYjqHJiTaPaEIE80078956 = -701906726;    double oMzeYjqHJiTaPaEIE22756294 = -787935190;    double oMzeYjqHJiTaPaEIE85237315 = -936807517;    double oMzeYjqHJiTaPaEIE60541891 = -212782959;    double oMzeYjqHJiTaPaEIE41472953 = -788954848;    double oMzeYjqHJiTaPaEIE59245524 = -249408723;    double oMzeYjqHJiTaPaEIE27674797 = -89777015;    double oMzeYjqHJiTaPaEIE38954181 = -974186779;    double oMzeYjqHJiTaPaEIE62286861 = -59215187;    double oMzeYjqHJiTaPaEIE11249648 = -632311041;    double oMzeYjqHJiTaPaEIE4475398 = -539059323;     oMzeYjqHJiTaPaEIE66466764 = oMzeYjqHJiTaPaEIE61791080;     oMzeYjqHJiTaPaEIE61791080 = oMzeYjqHJiTaPaEIE5501296;     oMzeYjqHJiTaPaEIE5501296 = oMzeYjqHJiTaPaEIE21341226;     oMzeYjqHJiTaPaEIE21341226 = oMzeYjqHJiTaPaEIE79398596;     oMzeYjqHJiTaPaEIE79398596 = oMzeYjqHJiTaPaEIE69957770;     oMzeYjqHJiTaPaEIE69957770 = oMzeYjqHJiTaPaEIE17045444;     oMzeYjqHJiTaPaEIE17045444 = oMzeYjqHJiTaPaEIE71471608;     oMzeYjqHJiTaPaEIE71471608 = oMzeYjqHJiTaPaEIE46413787;     oMzeYjqHJiTaPaEIE46413787 = oMzeYjqHJiTaPaEIE47769651;     oMzeYjqHJiTaPaEIE47769651 = oMzeYjqHJiTaPaEIE62633615;     oMzeYjqHJiTaPaEIE62633615 = oMzeYjqHJiTaPaEIE61198615;     oMzeYjqHJiTaPaEIE61198615 = oMzeYjqHJiTaPaEIE11130693;     oMzeYjqHJiTaPaEIE11130693 = oMzeYjqHJiTaPaEIE61323975;     oMzeYjqHJiTaPaEIE61323975 = oMzeYjqHJiTaPaEIE27164298;     oMzeYjqHJiTaPaEIE27164298 = oMzeYjqHJiTaPaEIE44126698;     oMzeYjqHJiTaPaEIE44126698 = oMzeYjqHJiTaPaEIE38392483;     oMzeYjqHJiTaPaEIE38392483 = oMzeYjqHJiTaPaEIE10043361;     oMzeYjqHJiTaPaEIE10043361 = oMzeYjqHJiTaPaEIE46225608;     oMzeYjqHJiTaPaEIE46225608 = oMzeYjqHJiTaPaEIE15142307;     oMzeYjqHJiTaPaEIE15142307 = oMzeYjqHJiTaPaEIE67520500;     oMzeYjqHJiTaPaEIE67520500 = oMzeYjqHJiTaPaEIE60361852;     oMzeYjqHJiTaPaEIE60361852 = oMzeYjqHJiTaPaEIE27071436;     oMzeYjqHJiTaPaEIE27071436 = oMzeYjqHJiTaPaEIE43273386;     oMzeYjqHJiTaPaEIE43273386 = oMzeYjqHJiTaPaEIE71242885;     oMzeYjqHJiTaPaEIE71242885 = oMzeYjqHJiTaPaEIE3420202;     oMzeYjqHJiTaPaEIE3420202 = oMzeYjqHJiTaPaEIE67608947;     oMzeYjqHJiTaPaEIE67608947 = oMzeYjqHJiTaPaEIE45201983;     oMzeYjqHJiTaPaEIE45201983 = oMzeYjqHJiTaPaEIE24622126;     oMzeYjqHJiTaPaEIE24622126 = oMzeYjqHJiTaPaEIE67740254;     oMzeYjqHJiTaPaEIE67740254 = oMzeYjqHJiTaPaEIE56296482;     oMzeYjqHJiTaPaEIE56296482 = oMzeYjqHJiTaPaEIE18578201;     oMzeYjqHJiTaPaEIE18578201 = oMzeYjqHJiTaPaEIE44484334;     oMzeYjqHJiTaPaEIE44484334 = oMzeYjqHJiTaPaEIE8326197;     oMzeYjqHJiTaPaEIE8326197 = oMzeYjqHJiTaPaEIE86183868;     oMzeYjqHJiTaPaEIE86183868 = oMzeYjqHJiTaPaEIE57475937;     oMzeYjqHJiTaPaEIE57475937 = oMzeYjqHJiTaPaEIE47465225;     oMzeYjqHJiTaPaEIE47465225 = oMzeYjqHJiTaPaEIE10640232;     oMzeYjqHJiTaPaEIE10640232 = oMzeYjqHJiTaPaEIE17451348;     oMzeYjqHJiTaPaEIE17451348 = oMzeYjqHJiTaPaEIE61594348;     oMzeYjqHJiTaPaEIE61594348 = oMzeYjqHJiTaPaEIE99518257;     oMzeYjqHJiTaPaEIE99518257 = oMzeYjqHJiTaPaEIE85803663;     oMzeYjqHJiTaPaEIE85803663 = oMzeYjqHJiTaPaEIE40960393;     oMzeYjqHJiTaPaEIE40960393 = oMzeYjqHJiTaPaEIE2722818;     oMzeYjqHJiTaPaEIE2722818 = oMzeYjqHJiTaPaEIE48530812;     oMzeYjqHJiTaPaEIE48530812 = oMzeYjqHJiTaPaEIE58617979;     oMzeYjqHJiTaPaEIE58617979 = oMzeYjqHJiTaPaEIE3753653;     oMzeYjqHJiTaPaEIE3753653 = oMzeYjqHJiTaPaEIE35735179;     oMzeYjqHJiTaPaEIE35735179 = oMzeYjqHJiTaPaEIE19981862;     oMzeYjqHJiTaPaEIE19981862 = oMzeYjqHJiTaPaEIE3394437;     oMzeYjqHJiTaPaEIE3394437 = oMzeYjqHJiTaPaEIE40739849;     oMzeYjqHJiTaPaEIE40739849 = oMzeYjqHJiTaPaEIE3284008;     oMzeYjqHJiTaPaEIE3284008 = oMzeYjqHJiTaPaEIE70293266;     oMzeYjqHJiTaPaEIE70293266 = oMzeYjqHJiTaPaEIE26051931;     oMzeYjqHJiTaPaEIE26051931 = oMzeYjqHJiTaPaEIE27032086;     oMzeYjqHJiTaPaEIE27032086 = oMzeYjqHJiTaPaEIE6104912;     oMzeYjqHJiTaPaEIE6104912 = oMzeYjqHJiTaPaEIE34719644;     oMzeYjqHJiTaPaEIE34719644 = oMzeYjqHJiTaPaEIE62227910;     oMzeYjqHJiTaPaEIE62227910 = oMzeYjqHJiTaPaEIE50098340;     oMzeYjqHJiTaPaEIE50098340 = oMzeYjqHJiTaPaEIE75978395;     oMzeYjqHJiTaPaEIE75978395 = oMzeYjqHJiTaPaEIE2348824;     oMzeYjqHJiTaPaEIE2348824 = oMzeYjqHJiTaPaEIE71843460;     oMzeYjqHJiTaPaEIE71843460 = oMzeYjqHJiTaPaEIE46849482;     oMzeYjqHJiTaPaEIE46849482 = oMzeYjqHJiTaPaEIE78673532;     oMzeYjqHJiTaPaEIE78673532 = oMzeYjqHJiTaPaEIE91473169;     oMzeYjqHJiTaPaEIE91473169 = oMzeYjqHJiTaPaEIE44055415;     oMzeYjqHJiTaPaEIE44055415 = oMzeYjqHJiTaPaEIE16714282;     oMzeYjqHJiTaPaEIE16714282 = oMzeYjqHJiTaPaEIE2804496;     oMzeYjqHJiTaPaEIE2804496 = oMzeYjqHJiTaPaEIE75140106;     oMzeYjqHJiTaPaEIE75140106 = oMzeYjqHJiTaPaEIE69688360;     oMzeYjqHJiTaPaEIE69688360 = oMzeYjqHJiTaPaEIE96661473;     oMzeYjqHJiTaPaEIE96661473 = oMzeYjqHJiTaPaEIE27752251;     oMzeYjqHJiTaPaEIE27752251 = oMzeYjqHJiTaPaEIE92592013;     oMzeYjqHJiTaPaEIE92592013 = oMzeYjqHJiTaPaEIE84631260;     oMzeYjqHJiTaPaEIE84631260 = oMzeYjqHJiTaPaEIE15624049;     oMzeYjqHJiTaPaEIE15624049 = oMzeYjqHJiTaPaEIE81716837;     oMzeYjqHJiTaPaEIE81716837 = oMzeYjqHJiTaPaEIE19401460;     oMzeYjqHJiTaPaEIE19401460 = oMzeYjqHJiTaPaEIE24348619;     oMzeYjqHJiTaPaEIE24348619 = oMzeYjqHJiTaPaEIE94742573;     oMzeYjqHJiTaPaEIE94742573 = oMzeYjqHJiTaPaEIE12624906;     oMzeYjqHJiTaPaEIE12624906 = oMzeYjqHJiTaPaEIE99666548;     oMzeYjqHJiTaPaEIE99666548 = oMzeYjqHJiTaPaEIE31873768;     oMzeYjqHJiTaPaEIE31873768 = oMzeYjqHJiTaPaEIE25220122;     oMzeYjqHJiTaPaEIE25220122 = oMzeYjqHJiTaPaEIE21227690;     oMzeYjqHJiTaPaEIE21227690 = oMzeYjqHJiTaPaEIE27000406;     oMzeYjqHJiTaPaEIE27000406 = oMzeYjqHJiTaPaEIE53012474;     oMzeYjqHJiTaPaEIE53012474 = oMzeYjqHJiTaPaEIE48284934;     oMzeYjqHJiTaPaEIE48284934 = oMzeYjqHJiTaPaEIE18432403;     oMzeYjqHJiTaPaEIE18432403 = oMzeYjqHJiTaPaEIE81294111;     oMzeYjqHJiTaPaEIE81294111 = oMzeYjqHJiTaPaEIE80078956;     oMzeYjqHJiTaPaEIE80078956 = oMzeYjqHJiTaPaEIE22756294;     oMzeYjqHJiTaPaEIE22756294 = oMzeYjqHJiTaPaEIE85237315;     oMzeYjqHJiTaPaEIE85237315 = oMzeYjqHJiTaPaEIE60541891;     oMzeYjqHJiTaPaEIE60541891 = oMzeYjqHJiTaPaEIE41472953;     oMzeYjqHJiTaPaEIE41472953 = oMzeYjqHJiTaPaEIE59245524;     oMzeYjqHJiTaPaEIE59245524 = oMzeYjqHJiTaPaEIE27674797;     oMzeYjqHJiTaPaEIE27674797 = oMzeYjqHJiTaPaEIE38954181;     oMzeYjqHJiTaPaEIE38954181 = oMzeYjqHJiTaPaEIE62286861;     oMzeYjqHJiTaPaEIE62286861 = oMzeYjqHJiTaPaEIE11249648;     oMzeYjqHJiTaPaEIE11249648 = oMzeYjqHJiTaPaEIE4475398;     oMzeYjqHJiTaPaEIE4475398 = oMzeYjqHJiTaPaEIE66466764;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void DfGBYjgSbuidXWzR61844185() {     double NGgcXrWFidnoErsun1925733 = 94663777;    double NGgcXrWFidnoErsun75063158 = -870110880;    double NGgcXrWFidnoErsun42887773 = 878727;    double NGgcXrWFidnoErsun93564580 = -267922556;    double NGgcXrWFidnoErsun35683865 = 68463744;    double NGgcXrWFidnoErsun6180643 = -19484322;    double NGgcXrWFidnoErsun27702850 = -727113791;    double NGgcXrWFidnoErsun33603169 = -801952069;    double NGgcXrWFidnoErsun47931392 = -273624711;    double NGgcXrWFidnoErsun62672363 = -166219258;    double NGgcXrWFidnoErsun815350 = -948269750;    double NGgcXrWFidnoErsun61766764 = -750039492;    double NGgcXrWFidnoErsun49365338 = -809673808;    double NGgcXrWFidnoErsun14464838 = -564155630;    double NGgcXrWFidnoErsun85358814 = -663994709;    double NGgcXrWFidnoErsun27667999 = -960585988;    double NGgcXrWFidnoErsun94228418 = 85882175;    double NGgcXrWFidnoErsun9803553 = -791588352;    double NGgcXrWFidnoErsun46775938 = -748159035;    double NGgcXrWFidnoErsun8979526 = 96506589;    double NGgcXrWFidnoErsun80607363 = -703799401;    double NGgcXrWFidnoErsun63783377 = -5535630;    double NGgcXrWFidnoErsun97596815 = -401017957;    double NGgcXrWFidnoErsun37066331 = -465151959;    double NGgcXrWFidnoErsun79260836 = -507732464;    double NGgcXrWFidnoErsun14118373 = -562911063;    double NGgcXrWFidnoErsun28029521 = -66970181;    double NGgcXrWFidnoErsun85930408 = -607462512;    double NGgcXrWFidnoErsun71290340 = -194851709;    double NGgcXrWFidnoErsun73273665 = -67065398;    double NGgcXrWFidnoErsun74879713 = 18408732;    double NGgcXrWFidnoErsun56401219 = -144710836;    double NGgcXrWFidnoErsun56702344 = 96492702;    double NGgcXrWFidnoErsun38074624 = -71754880;    double NGgcXrWFidnoErsun51992322 = -173166963;    double NGgcXrWFidnoErsun9036782 = -128673996;    double NGgcXrWFidnoErsun96877041 = -80630400;    double NGgcXrWFidnoErsun61855635 = -158462759;    double NGgcXrWFidnoErsun47841909 = -491434279;    double NGgcXrWFidnoErsun25865881 = -311121655;    double NGgcXrWFidnoErsun32969865 = -303673648;    double NGgcXrWFidnoErsun75168790 = 94306908;    double NGgcXrWFidnoErsun30343772 = -465131050;    double NGgcXrWFidnoErsun42854156 = -108564930;    double NGgcXrWFidnoErsun48547271 = -899380619;    double NGgcXrWFidnoErsun4579308 = -902685686;    double NGgcXrWFidnoErsun23098816 = -188714595;    double NGgcXrWFidnoErsun53461491 = -912773573;    double NGgcXrWFidnoErsun42874528 = 53035293;    double NGgcXrWFidnoErsun27924026 = -514551669;    double NGgcXrWFidnoErsun21510573 = 17289480;    double NGgcXrWFidnoErsun86769480 = 33003924;    double NGgcXrWFidnoErsun38914415 = -639294468;    double NGgcXrWFidnoErsun42043777 = -390830029;    double NGgcXrWFidnoErsun81236822 = -150736057;    double NGgcXrWFidnoErsun38142356 = -899800593;    double NGgcXrWFidnoErsun77466342 = -369092924;    double NGgcXrWFidnoErsun5821442 = -533969314;    double NGgcXrWFidnoErsun14303744 = -760190093;    double NGgcXrWFidnoErsun21565493 = -368625193;    double NGgcXrWFidnoErsun78151122 = -952514141;    double NGgcXrWFidnoErsun41772442 = -19651280;    double NGgcXrWFidnoErsun62312829 = -507100360;    double NGgcXrWFidnoErsun74657726 = -106559314;    double NGgcXrWFidnoErsun87792650 = -84627991;    double NGgcXrWFidnoErsun44414131 = -703558914;    double NGgcXrWFidnoErsun5064420 = -746532194;    double NGgcXrWFidnoErsun11290715 = -637918929;    double NGgcXrWFidnoErsun62472516 = -290988668;    double NGgcXrWFidnoErsun76322032 = -435320713;    double NGgcXrWFidnoErsun30790958 = -779955588;    double NGgcXrWFidnoErsun32372784 = -755655066;    double NGgcXrWFidnoErsun61961643 = -200154074;    double NGgcXrWFidnoErsun20910058 = -337037381;    double NGgcXrWFidnoErsun76009660 = -599819764;    double NGgcXrWFidnoErsun5438574 = -698106310;    double NGgcXrWFidnoErsun33439606 = -540404581;    double NGgcXrWFidnoErsun54742660 = -192453027;    double NGgcXrWFidnoErsun88519060 = -565771340;    double NGgcXrWFidnoErsun74681528 = -605046779;    double NGgcXrWFidnoErsun91019556 = -274196469;    double NGgcXrWFidnoErsun74568030 = -154196608;    double NGgcXrWFidnoErsun43055881 = -560497806;    double NGgcXrWFidnoErsun43366315 = -680300041;    double NGgcXrWFidnoErsun51763093 = 15645121;    double NGgcXrWFidnoErsun88110233 = 85404807;    double NGgcXrWFidnoErsun17486804 = -505416369;    double NGgcXrWFidnoErsun14658568 = -512677270;    double NGgcXrWFidnoErsun56837801 = -921018823;    double NGgcXrWFidnoErsun13849967 = -273366371;    double NGgcXrWFidnoErsun31570439 = -759581073;    double NGgcXrWFidnoErsun91055599 = -546661087;    double NGgcXrWFidnoErsun47551891 = -398272667;    double NGgcXrWFidnoErsun26276417 = -22809086;    double NGgcXrWFidnoErsun47714759 = -358607514;    double NGgcXrWFidnoErsun91197423 = -184022369;    double NGgcXrWFidnoErsun12855962 = -398592732;    double NGgcXrWFidnoErsun55686045 = -258571736;    double NGgcXrWFidnoErsun55061505 = 76063060;    double NGgcXrWFidnoErsun4133141 = 94663777;     NGgcXrWFidnoErsun1925733 = NGgcXrWFidnoErsun75063158;     NGgcXrWFidnoErsun75063158 = NGgcXrWFidnoErsun42887773;     NGgcXrWFidnoErsun42887773 = NGgcXrWFidnoErsun93564580;     NGgcXrWFidnoErsun93564580 = NGgcXrWFidnoErsun35683865;     NGgcXrWFidnoErsun35683865 = NGgcXrWFidnoErsun6180643;     NGgcXrWFidnoErsun6180643 = NGgcXrWFidnoErsun27702850;     NGgcXrWFidnoErsun27702850 = NGgcXrWFidnoErsun33603169;     NGgcXrWFidnoErsun33603169 = NGgcXrWFidnoErsun47931392;     NGgcXrWFidnoErsun47931392 = NGgcXrWFidnoErsun62672363;     NGgcXrWFidnoErsun62672363 = NGgcXrWFidnoErsun815350;     NGgcXrWFidnoErsun815350 = NGgcXrWFidnoErsun61766764;     NGgcXrWFidnoErsun61766764 = NGgcXrWFidnoErsun49365338;     NGgcXrWFidnoErsun49365338 = NGgcXrWFidnoErsun14464838;     NGgcXrWFidnoErsun14464838 = NGgcXrWFidnoErsun85358814;     NGgcXrWFidnoErsun85358814 = NGgcXrWFidnoErsun27667999;     NGgcXrWFidnoErsun27667999 = NGgcXrWFidnoErsun94228418;     NGgcXrWFidnoErsun94228418 = NGgcXrWFidnoErsun9803553;     NGgcXrWFidnoErsun9803553 = NGgcXrWFidnoErsun46775938;     NGgcXrWFidnoErsun46775938 = NGgcXrWFidnoErsun8979526;     NGgcXrWFidnoErsun8979526 = NGgcXrWFidnoErsun80607363;     NGgcXrWFidnoErsun80607363 = NGgcXrWFidnoErsun63783377;     NGgcXrWFidnoErsun63783377 = NGgcXrWFidnoErsun97596815;     NGgcXrWFidnoErsun97596815 = NGgcXrWFidnoErsun37066331;     NGgcXrWFidnoErsun37066331 = NGgcXrWFidnoErsun79260836;     NGgcXrWFidnoErsun79260836 = NGgcXrWFidnoErsun14118373;     NGgcXrWFidnoErsun14118373 = NGgcXrWFidnoErsun28029521;     NGgcXrWFidnoErsun28029521 = NGgcXrWFidnoErsun85930408;     NGgcXrWFidnoErsun85930408 = NGgcXrWFidnoErsun71290340;     NGgcXrWFidnoErsun71290340 = NGgcXrWFidnoErsun73273665;     NGgcXrWFidnoErsun73273665 = NGgcXrWFidnoErsun74879713;     NGgcXrWFidnoErsun74879713 = NGgcXrWFidnoErsun56401219;     NGgcXrWFidnoErsun56401219 = NGgcXrWFidnoErsun56702344;     NGgcXrWFidnoErsun56702344 = NGgcXrWFidnoErsun38074624;     NGgcXrWFidnoErsun38074624 = NGgcXrWFidnoErsun51992322;     NGgcXrWFidnoErsun51992322 = NGgcXrWFidnoErsun9036782;     NGgcXrWFidnoErsun9036782 = NGgcXrWFidnoErsun96877041;     NGgcXrWFidnoErsun96877041 = NGgcXrWFidnoErsun61855635;     NGgcXrWFidnoErsun61855635 = NGgcXrWFidnoErsun47841909;     NGgcXrWFidnoErsun47841909 = NGgcXrWFidnoErsun25865881;     NGgcXrWFidnoErsun25865881 = NGgcXrWFidnoErsun32969865;     NGgcXrWFidnoErsun32969865 = NGgcXrWFidnoErsun75168790;     NGgcXrWFidnoErsun75168790 = NGgcXrWFidnoErsun30343772;     NGgcXrWFidnoErsun30343772 = NGgcXrWFidnoErsun42854156;     NGgcXrWFidnoErsun42854156 = NGgcXrWFidnoErsun48547271;     NGgcXrWFidnoErsun48547271 = NGgcXrWFidnoErsun4579308;     NGgcXrWFidnoErsun4579308 = NGgcXrWFidnoErsun23098816;     NGgcXrWFidnoErsun23098816 = NGgcXrWFidnoErsun53461491;     NGgcXrWFidnoErsun53461491 = NGgcXrWFidnoErsun42874528;     NGgcXrWFidnoErsun42874528 = NGgcXrWFidnoErsun27924026;     NGgcXrWFidnoErsun27924026 = NGgcXrWFidnoErsun21510573;     NGgcXrWFidnoErsun21510573 = NGgcXrWFidnoErsun86769480;     NGgcXrWFidnoErsun86769480 = NGgcXrWFidnoErsun38914415;     NGgcXrWFidnoErsun38914415 = NGgcXrWFidnoErsun42043777;     NGgcXrWFidnoErsun42043777 = NGgcXrWFidnoErsun81236822;     NGgcXrWFidnoErsun81236822 = NGgcXrWFidnoErsun38142356;     NGgcXrWFidnoErsun38142356 = NGgcXrWFidnoErsun77466342;     NGgcXrWFidnoErsun77466342 = NGgcXrWFidnoErsun5821442;     NGgcXrWFidnoErsun5821442 = NGgcXrWFidnoErsun14303744;     NGgcXrWFidnoErsun14303744 = NGgcXrWFidnoErsun21565493;     NGgcXrWFidnoErsun21565493 = NGgcXrWFidnoErsun78151122;     NGgcXrWFidnoErsun78151122 = NGgcXrWFidnoErsun41772442;     NGgcXrWFidnoErsun41772442 = NGgcXrWFidnoErsun62312829;     NGgcXrWFidnoErsun62312829 = NGgcXrWFidnoErsun74657726;     NGgcXrWFidnoErsun74657726 = NGgcXrWFidnoErsun87792650;     NGgcXrWFidnoErsun87792650 = NGgcXrWFidnoErsun44414131;     NGgcXrWFidnoErsun44414131 = NGgcXrWFidnoErsun5064420;     NGgcXrWFidnoErsun5064420 = NGgcXrWFidnoErsun11290715;     NGgcXrWFidnoErsun11290715 = NGgcXrWFidnoErsun62472516;     NGgcXrWFidnoErsun62472516 = NGgcXrWFidnoErsun76322032;     NGgcXrWFidnoErsun76322032 = NGgcXrWFidnoErsun30790958;     NGgcXrWFidnoErsun30790958 = NGgcXrWFidnoErsun32372784;     NGgcXrWFidnoErsun32372784 = NGgcXrWFidnoErsun61961643;     NGgcXrWFidnoErsun61961643 = NGgcXrWFidnoErsun20910058;     NGgcXrWFidnoErsun20910058 = NGgcXrWFidnoErsun76009660;     NGgcXrWFidnoErsun76009660 = NGgcXrWFidnoErsun5438574;     NGgcXrWFidnoErsun5438574 = NGgcXrWFidnoErsun33439606;     NGgcXrWFidnoErsun33439606 = NGgcXrWFidnoErsun54742660;     NGgcXrWFidnoErsun54742660 = NGgcXrWFidnoErsun88519060;     NGgcXrWFidnoErsun88519060 = NGgcXrWFidnoErsun74681528;     NGgcXrWFidnoErsun74681528 = NGgcXrWFidnoErsun91019556;     NGgcXrWFidnoErsun91019556 = NGgcXrWFidnoErsun74568030;     NGgcXrWFidnoErsun74568030 = NGgcXrWFidnoErsun43055881;     NGgcXrWFidnoErsun43055881 = NGgcXrWFidnoErsun43366315;     NGgcXrWFidnoErsun43366315 = NGgcXrWFidnoErsun51763093;     NGgcXrWFidnoErsun51763093 = NGgcXrWFidnoErsun88110233;     NGgcXrWFidnoErsun88110233 = NGgcXrWFidnoErsun17486804;     NGgcXrWFidnoErsun17486804 = NGgcXrWFidnoErsun14658568;     NGgcXrWFidnoErsun14658568 = NGgcXrWFidnoErsun56837801;     NGgcXrWFidnoErsun56837801 = NGgcXrWFidnoErsun13849967;     NGgcXrWFidnoErsun13849967 = NGgcXrWFidnoErsun31570439;     NGgcXrWFidnoErsun31570439 = NGgcXrWFidnoErsun91055599;     NGgcXrWFidnoErsun91055599 = NGgcXrWFidnoErsun47551891;     NGgcXrWFidnoErsun47551891 = NGgcXrWFidnoErsun26276417;     NGgcXrWFidnoErsun26276417 = NGgcXrWFidnoErsun47714759;     NGgcXrWFidnoErsun47714759 = NGgcXrWFidnoErsun91197423;     NGgcXrWFidnoErsun91197423 = NGgcXrWFidnoErsun12855962;     NGgcXrWFidnoErsun12855962 = NGgcXrWFidnoErsun55686045;     NGgcXrWFidnoErsun55686045 = NGgcXrWFidnoErsun55061505;     NGgcXrWFidnoErsun55061505 = NGgcXrWFidnoErsun4133141;     NGgcXrWFidnoErsun4133141 = NGgcXrWFidnoErsun1925733;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void AWuMIOvIPKwWohOv76893253() {     double BypQlahUBmdgzJQhU8042840 = -17238112;    double BypQlahUBmdgzJQhU18436438 = -579710661;    double BypQlahUBmdgzJQhU58929956 = -149954194;    double BypQlahUBmdgzJQhU20639362 = -458671186;    double BypQlahUBmdgzJQhU4579354 = -452186127;    double BypQlahUBmdgzJQhU97597845 = -283980557;    double BypQlahUBmdgzJQhU88191816 = -866078768;    double BypQlahUBmdgzJQhU83363060 = 11174018;    double BypQlahUBmdgzJQhU12573888 = -41657377;    double BypQlahUBmdgzJQhU3902023 = -495194736;    double BypQlahUBmdgzJQhU60925640 = -825707336;    double BypQlahUBmdgzJQhU93589806 = -94445694;    double BypQlahUBmdgzJQhU35574456 = -149564897;    double BypQlahUBmdgzJQhU97608514 = 3605802;    double BypQlahUBmdgzJQhU81101240 = -254456876;    double BypQlahUBmdgzJQhU69173393 = -933585531;    double BypQlahUBmdgzJQhU88064421 = -530662938;    double BypQlahUBmdgzJQhU6856706 = -457110995;    double BypQlahUBmdgzJQhU8605780 = -53928132;    double BypQlahUBmdgzJQhU89529689 = -260396331;    double BypQlahUBmdgzJQhU69160556 = -559286857;    double BypQlahUBmdgzJQhU50304401 = 86534653;    double BypQlahUBmdgzJQhU33187608 = -147535444;    double BypQlahUBmdgzJQhU21058705 = -687456947;    double BypQlahUBmdgzJQhU22960487 = -880928497;    double BypQlahUBmdgzJQhU38840048 = -441566386;    double BypQlahUBmdgzJQhU26489299 = 32367616;    double BypQlahUBmdgzJQhU99037533 = -673391465;    double BypQlahUBmdgzJQhU37422596 = -417234414;    double BypQlahUBmdgzJQhU41889302 = -984536265;    double BypQlahUBmdgzJQhU67122262 = -994912206;    double BypQlahUBmdgzJQhU54011841 = -954157652;    double BypQlahUBmdgzJQhU66413293 = -896120669;    double BypQlahUBmdgzJQhU32905188 = -923296645;    double BypQlahUBmdgzJQhU13466975 = -811900068;    double BypQlahUBmdgzJQhU77762694 = -342700104;    double BypQlahUBmdgzJQhU78138976 = -34266270;    double BypQlahUBmdgzJQhU95945847 = 70031231;    double BypQlahUBmdgzJQhU21452163 = -485357776;    double BypQlahUBmdgzJQhU13367818 = -784560878;    double BypQlahUBmdgzJQhU38639089 = -368947542;    double BypQlahUBmdgzJQhU7319247 = -175835370;    double BypQlahUBmdgzJQhU8410750 = -545935633;    double BypQlahUBmdgzJQhU30261896 = -127907869;    double BypQlahUBmdgzJQhU60499736 = -561278774;    double BypQlahUBmdgzJQhU16518503 = -887901802;    double BypQlahUBmdgzJQhU40877390 = -307490132;    double BypQlahUBmdgzJQhU61418432 = -922669105;    double BypQlahUBmdgzJQhU52550257 = -32701695;    double BypQlahUBmdgzJQhU40835678 = -601562477;    double BypQlahUBmdgzJQhU57211251 = -925004191;    double BypQlahUBmdgzJQhU97579199 = -867722396;    double BypQlahUBmdgzJQhU87162872 = -431084055;    double BypQlahUBmdgzJQhU38012658 = -711300174;    double BypQlahUBmdgzJQhU7732698 = -851267840;    double BypQlahUBmdgzJQhU57738439 = -3772765;    double BypQlahUBmdgzJQhU85248830 = -332175218;    double BypQlahUBmdgzJQhU37871251 = -462497247;    double BypQlahUBmdgzJQhU97678874 = -577742690;    double BypQlahUBmdgzJQhU65739305 = 89380259;    double BypQlahUBmdgzJQhU71108546 = -216348173;    double BypQlahUBmdgzJQhU89154282 = -92687303;    double BypQlahUBmdgzJQhU45940464 = -571591569;    double BypQlahUBmdgzJQhU70684585 = -57121112;    double BypQlahUBmdgzJQhU36779760 = -500282530;    double BypQlahUBmdgzJQhU6913800 = -871549685;    double BypQlahUBmdgzJQhU27176513 = -198325026;    double BypQlahUBmdgzJQhU2669269 = -226268252;    double BypQlahUBmdgzJQhU84141540 = -184494130;    double BypQlahUBmdgzJQhU3338547 = -911756772;    double BypQlahUBmdgzJQhU91034416 = -799319262;    double BypQlahUBmdgzJQhU92118574 = -500694169;    double BypQlahUBmdgzJQhU85404542 = -971753220;    double BypQlahUBmdgzJQhU95237961 = -269367255;    double BypQlahUBmdgzJQhU50890601 = -891448789;    double BypQlahUBmdgzJQhU61841309 = -283451488;    double BypQlahUBmdgzJQhU41893651 = -367529714;    double BypQlahUBmdgzJQhU2925712 = 80372424;    double BypQlahUBmdgzJQhU60558969 = -26178174;    double BypQlahUBmdgzJQhU6441984 = -993026695;    double BypQlahUBmdgzJQhU97962657 = -34076255;    double BypQlahUBmdgzJQhU65070867 = -44963279;    double BypQlahUBmdgzJQhU46487277 = -540689771;    double BypQlahUBmdgzJQhU96586918 = -815671937;    double BypQlahUBmdgzJQhU84678051 = 40467925;    double BypQlahUBmdgzJQhU69543063 = -27189811;    double BypQlahUBmdgzJQhU66848968 = -423073597;    double BypQlahUBmdgzJQhU28400636 = -84820495;    double BypQlahUBmdgzJQhU25172491 = 27971194;    double BypQlahUBmdgzJQhU55728536 = -708127304;    double BypQlahUBmdgzJQhU92513864 = 89475113;    double BypQlahUBmdgzJQhU40267725 = -571769023;    double BypQlahUBmdgzJQhU98266972 = -352226080;    double BypQlahUBmdgzJQhU55712857 = -474738035;    double BypQlahUBmdgzJQhU42259272 = -468212705;    double BypQlahUBmdgzJQhU49484807 = -176260240;    double BypQlahUBmdgzJQhU61378783 = -604243802;    double BypQlahUBmdgzJQhU37726165 = -388814522;    double BypQlahUBmdgzJQhU93482136 = -627625339;    double BypQlahUBmdgzJQhU53585937 = -17238112;     BypQlahUBmdgzJQhU8042840 = BypQlahUBmdgzJQhU18436438;     BypQlahUBmdgzJQhU18436438 = BypQlahUBmdgzJQhU58929956;     BypQlahUBmdgzJQhU58929956 = BypQlahUBmdgzJQhU20639362;     BypQlahUBmdgzJQhU20639362 = BypQlahUBmdgzJQhU4579354;     BypQlahUBmdgzJQhU4579354 = BypQlahUBmdgzJQhU97597845;     BypQlahUBmdgzJQhU97597845 = BypQlahUBmdgzJQhU88191816;     BypQlahUBmdgzJQhU88191816 = BypQlahUBmdgzJQhU83363060;     BypQlahUBmdgzJQhU83363060 = BypQlahUBmdgzJQhU12573888;     BypQlahUBmdgzJQhU12573888 = BypQlahUBmdgzJQhU3902023;     BypQlahUBmdgzJQhU3902023 = BypQlahUBmdgzJQhU60925640;     BypQlahUBmdgzJQhU60925640 = BypQlahUBmdgzJQhU93589806;     BypQlahUBmdgzJQhU93589806 = BypQlahUBmdgzJQhU35574456;     BypQlahUBmdgzJQhU35574456 = BypQlahUBmdgzJQhU97608514;     BypQlahUBmdgzJQhU97608514 = BypQlahUBmdgzJQhU81101240;     BypQlahUBmdgzJQhU81101240 = BypQlahUBmdgzJQhU69173393;     BypQlahUBmdgzJQhU69173393 = BypQlahUBmdgzJQhU88064421;     BypQlahUBmdgzJQhU88064421 = BypQlahUBmdgzJQhU6856706;     BypQlahUBmdgzJQhU6856706 = BypQlahUBmdgzJQhU8605780;     BypQlahUBmdgzJQhU8605780 = BypQlahUBmdgzJQhU89529689;     BypQlahUBmdgzJQhU89529689 = BypQlahUBmdgzJQhU69160556;     BypQlahUBmdgzJQhU69160556 = BypQlahUBmdgzJQhU50304401;     BypQlahUBmdgzJQhU50304401 = BypQlahUBmdgzJQhU33187608;     BypQlahUBmdgzJQhU33187608 = BypQlahUBmdgzJQhU21058705;     BypQlahUBmdgzJQhU21058705 = BypQlahUBmdgzJQhU22960487;     BypQlahUBmdgzJQhU22960487 = BypQlahUBmdgzJQhU38840048;     BypQlahUBmdgzJQhU38840048 = BypQlahUBmdgzJQhU26489299;     BypQlahUBmdgzJQhU26489299 = BypQlahUBmdgzJQhU99037533;     BypQlahUBmdgzJQhU99037533 = BypQlahUBmdgzJQhU37422596;     BypQlahUBmdgzJQhU37422596 = BypQlahUBmdgzJQhU41889302;     BypQlahUBmdgzJQhU41889302 = BypQlahUBmdgzJQhU67122262;     BypQlahUBmdgzJQhU67122262 = BypQlahUBmdgzJQhU54011841;     BypQlahUBmdgzJQhU54011841 = BypQlahUBmdgzJQhU66413293;     BypQlahUBmdgzJQhU66413293 = BypQlahUBmdgzJQhU32905188;     BypQlahUBmdgzJQhU32905188 = BypQlahUBmdgzJQhU13466975;     BypQlahUBmdgzJQhU13466975 = BypQlahUBmdgzJQhU77762694;     BypQlahUBmdgzJQhU77762694 = BypQlahUBmdgzJQhU78138976;     BypQlahUBmdgzJQhU78138976 = BypQlahUBmdgzJQhU95945847;     BypQlahUBmdgzJQhU95945847 = BypQlahUBmdgzJQhU21452163;     BypQlahUBmdgzJQhU21452163 = BypQlahUBmdgzJQhU13367818;     BypQlahUBmdgzJQhU13367818 = BypQlahUBmdgzJQhU38639089;     BypQlahUBmdgzJQhU38639089 = BypQlahUBmdgzJQhU7319247;     BypQlahUBmdgzJQhU7319247 = BypQlahUBmdgzJQhU8410750;     BypQlahUBmdgzJQhU8410750 = BypQlahUBmdgzJQhU30261896;     BypQlahUBmdgzJQhU30261896 = BypQlahUBmdgzJQhU60499736;     BypQlahUBmdgzJQhU60499736 = BypQlahUBmdgzJQhU16518503;     BypQlahUBmdgzJQhU16518503 = BypQlahUBmdgzJQhU40877390;     BypQlahUBmdgzJQhU40877390 = BypQlahUBmdgzJQhU61418432;     BypQlahUBmdgzJQhU61418432 = BypQlahUBmdgzJQhU52550257;     BypQlahUBmdgzJQhU52550257 = BypQlahUBmdgzJQhU40835678;     BypQlahUBmdgzJQhU40835678 = BypQlahUBmdgzJQhU57211251;     BypQlahUBmdgzJQhU57211251 = BypQlahUBmdgzJQhU97579199;     BypQlahUBmdgzJQhU97579199 = BypQlahUBmdgzJQhU87162872;     BypQlahUBmdgzJQhU87162872 = BypQlahUBmdgzJQhU38012658;     BypQlahUBmdgzJQhU38012658 = BypQlahUBmdgzJQhU7732698;     BypQlahUBmdgzJQhU7732698 = BypQlahUBmdgzJQhU57738439;     BypQlahUBmdgzJQhU57738439 = BypQlahUBmdgzJQhU85248830;     BypQlahUBmdgzJQhU85248830 = BypQlahUBmdgzJQhU37871251;     BypQlahUBmdgzJQhU37871251 = BypQlahUBmdgzJQhU97678874;     BypQlahUBmdgzJQhU97678874 = BypQlahUBmdgzJQhU65739305;     BypQlahUBmdgzJQhU65739305 = BypQlahUBmdgzJQhU71108546;     BypQlahUBmdgzJQhU71108546 = BypQlahUBmdgzJQhU89154282;     BypQlahUBmdgzJQhU89154282 = BypQlahUBmdgzJQhU45940464;     BypQlahUBmdgzJQhU45940464 = BypQlahUBmdgzJQhU70684585;     BypQlahUBmdgzJQhU70684585 = BypQlahUBmdgzJQhU36779760;     BypQlahUBmdgzJQhU36779760 = BypQlahUBmdgzJQhU6913800;     BypQlahUBmdgzJQhU6913800 = BypQlahUBmdgzJQhU27176513;     BypQlahUBmdgzJQhU27176513 = BypQlahUBmdgzJQhU2669269;     BypQlahUBmdgzJQhU2669269 = BypQlahUBmdgzJQhU84141540;     BypQlahUBmdgzJQhU84141540 = BypQlahUBmdgzJQhU3338547;     BypQlahUBmdgzJQhU3338547 = BypQlahUBmdgzJQhU91034416;     BypQlahUBmdgzJQhU91034416 = BypQlahUBmdgzJQhU92118574;     BypQlahUBmdgzJQhU92118574 = BypQlahUBmdgzJQhU85404542;     BypQlahUBmdgzJQhU85404542 = BypQlahUBmdgzJQhU95237961;     BypQlahUBmdgzJQhU95237961 = BypQlahUBmdgzJQhU50890601;     BypQlahUBmdgzJQhU50890601 = BypQlahUBmdgzJQhU61841309;     BypQlahUBmdgzJQhU61841309 = BypQlahUBmdgzJQhU41893651;     BypQlahUBmdgzJQhU41893651 = BypQlahUBmdgzJQhU2925712;     BypQlahUBmdgzJQhU2925712 = BypQlahUBmdgzJQhU60558969;     BypQlahUBmdgzJQhU60558969 = BypQlahUBmdgzJQhU6441984;     BypQlahUBmdgzJQhU6441984 = BypQlahUBmdgzJQhU97962657;     BypQlahUBmdgzJQhU97962657 = BypQlahUBmdgzJQhU65070867;     BypQlahUBmdgzJQhU65070867 = BypQlahUBmdgzJQhU46487277;     BypQlahUBmdgzJQhU46487277 = BypQlahUBmdgzJQhU96586918;     BypQlahUBmdgzJQhU96586918 = BypQlahUBmdgzJQhU84678051;     BypQlahUBmdgzJQhU84678051 = BypQlahUBmdgzJQhU69543063;     BypQlahUBmdgzJQhU69543063 = BypQlahUBmdgzJQhU66848968;     BypQlahUBmdgzJQhU66848968 = BypQlahUBmdgzJQhU28400636;     BypQlahUBmdgzJQhU28400636 = BypQlahUBmdgzJQhU25172491;     BypQlahUBmdgzJQhU25172491 = BypQlahUBmdgzJQhU55728536;     BypQlahUBmdgzJQhU55728536 = BypQlahUBmdgzJQhU92513864;     BypQlahUBmdgzJQhU92513864 = BypQlahUBmdgzJQhU40267725;     BypQlahUBmdgzJQhU40267725 = BypQlahUBmdgzJQhU98266972;     BypQlahUBmdgzJQhU98266972 = BypQlahUBmdgzJQhU55712857;     BypQlahUBmdgzJQhU55712857 = BypQlahUBmdgzJQhU42259272;     BypQlahUBmdgzJQhU42259272 = BypQlahUBmdgzJQhU49484807;     BypQlahUBmdgzJQhU49484807 = BypQlahUBmdgzJQhU61378783;     BypQlahUBmdgzJQhU61378783 = BypQlahUBmdgzJQhU37726165;     BypQlahUBmdgzJQhU37726165 = BypQlahUBmdgzJQhU93482136;     BypQlahUBmdgzJQhU93482136 = BypQlahUBmdgzJQhU53585937;     BypQlahUBmdgzJQhU53585937 = BypQlahUBmdgzJQhU8042840;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ccZyuDIzkVaCquUq44184853() {     double wWHybvdzpaAPjvRwt43501808 = -483515011;    double wWHybvdzpaAPjvRwt31708517 = -460262268;    double wWHybvdzpaAPjvRwt96316433 = -915621269;    double wWHybvdzpaAPjvRwt92862716 = -279822258;    double wWHybvdzpaAPjvRwt60864623 = -557784972;    double wWHybvdzpaAPjvRwt33820717 = -546328452;    double wWHybvdzpaAPjvRwt98849222 = -710804502;    double wWHybvdzpaAPjvRwt45494621 = -733060122;    double wWHybvdzpaAPjvRwt14091493 = -529988784;    double wWHybvdzpaAPjvRwt18804735 = -159881734;    double wWHybvdzpaAPjvRwt99107374 = -627715065;    double wWHybvdzpaAPjvRwt94157954 = 41433534;    double wWHybvdzpaAPjvRwt73809101 = -701577402;    double wWHybvdzpaAPjvRwt50749378 = 27242393;    double wWHybvdzpaAPjvRwt39295757 = -757881891;    double wWHybvdzpaAPjvRwt52714694 = -569397138;    double wWHybvdzpaAPjvRwt43900357 = 42362509;    double wWHybvdzpaAPjvRwt6616897 = -530539063;    double wWHybvdzpaAPjvRwt9156110 = -370491554;    double wWHybvdzpaAPjvRwt83366908 = -686957397;    double wWHybvdzpaAPjvRwt82247419 = -216561546;    double wWHybvdzpaAPjvRwt53725926 = -101641;    double wWHybvdzpaAPjvRwt3712988 = -885167628;    double wWHybvdzpaAPjvRwt14851651 = -98281551;    double wWHybvdzpaAPjvRwt30978438 = -904913695;    double wWHybvdzpaAPjvRwt49538220 = -130143193;    double wWHybvdzpaAPjvRwt86909873 = -873916525;    double wWHybvdzpaAPjvRwt39765959 = -843163640;    double wWHybvdzpaAPjvRwt84090810 = -991329216;    double wWHybvdzpaAPjvRwt47422713 = 80956564;    double wWHybvdzpaAPjvRwt85705494 = -446892694;    double wWHybvdzpaAPjvRwt91834859 = -727129066;    double wWHybvdzpaAPjvRwt78631304 = 56283354;    double wWHybvdzpaAPjvRwt62653614 = -987990459;    double wWHybvdzpaAPjvRwt79275428 = -763000319;    double wWHybvdzpaAPjvRwt29323538 = -557265410;    double wWHybvdzpaAPjvRwt27550793 = -998962310;    double wWHybvdzpaAPjvRwt47161251 = 87375650;    double wWHybvdzpaAPjvRwt51842724 = -136234052;    double wWHybvdzpaAPjvRwt77639350 = -249823424;    double wWHybvdzpaAPjvRwt72090697 = -138146457;    double wWHybvdzpaAPjvRwt96684373 = -870380661;    double wWHybvdzpaAPjvRwt97794128 = -99116419;    double wWHybvdzpaAPjvRwt70393234 = -732240279;    double wWHybvdzpaAPjvRwt60516195 = -387789935;    double wWHybvdzpaAPjvRwt62479831 = -926825434;    double wWHybvdzpaAPjvRwt60222554 = -791476493;    double wWHybvdzpaAPjvRwt79144743 = 10804301;    double wWHybvdzpaAPjvRwt75442922 = -796438575;    double wWHybvdzpaAPjvRwt65365267 = 7410707;    double wWHybvdzpaAPjvRwt37981975 = -799280748;    double wWHybvdzpaAPjvRwt81064672 = 18625257;    double wWHybvdzpaAPjvRwt55784021 = -187129609;    double wWHybvdzpaAPjvRwt54004504 = -902808674;    double wWHybvdzpaAPjvRwt61937433 = -97757986;    double wWHybvdzpaAPjvRwt89775882 = -383413371;    double wWHybvdzpaAPjvRwt27995529 = -575094640;    double wWHybvdzpaAPjvRwt81464782 = -717339718;    double wWHybvdzpaAPjvRwt61884278 = -374908563;    double wWHybvdzpaAPjvRwt11326403 = -327641780;    double wWHybvdzpaAPjvRwt46910844 = -672411928;    double wWHybvdzpaAPjvRwt59083264 = -867640863;    double wWHybvdzpaAPjvRwt61403810 = -741730906;    double wWHybvdzpaAPjvRwt66668779 = -510945348;    double wWHybvdzpaAPjvRwt33099241 = -712989041;    double wWHybvdzpaAPjvRwt7272516 = -900585999;    double wWHybvdzpaAPjvRwt15526651 = 85150180;    double wWHybvdzpaAPjvRwt11155488 = -713586943;    double wWHybvdzpaAPjvRwt71473950 = -209757289;    double wWHybvdzpaAPjvRwt9972219 = -100616481;    double wWHybvdzpaAPjvRwt25163901 = -570434828;    double wWHybvdzpaAPjvRwt96739106 = 54986859;    double wWHybvdzpaAPjvRwt54774172 = -294305012;    double wWHybvdzpaAPjvRwt31516760 = -20668130;    double wWHybvdzpaAPjvRwt11276212 = -448810941;    double wWHybvdzpaAPjvRwt85563045 = -346180885;    double wWHybvdzpaAPjvRwt55931797 = -900985222;    double wWHybvdzpaAPjvRwt33319753 = -52927350;    double wWHybvdzpaAPjvRwt54335456 = -710491617;    double wWHybvdzpaAPjvRwt68498606 = -978088261;    double wWHybvdzpaAPjvRwt89315665 = -338666701;    double wWHybvdzpaAPjvRwt7765130 = -784720826;    double wWHybvdzpaAPjvRwt64323036 = 53274935;    double wWHybvdzpaAPjvRwt18725544 = -898739924;    double wWHybvdzpaAPjvRwt9440739 = -119762688;    double wWHybvdzpaAPjvRwt4640822 = -365517951;    double wWHybvdzpaAPjvRwt36050838 = -439999458;    double wWHybvdzpaAPjvRwt24626800 = -40907972;    double wWHybvdzpaAPjvRwt716181 = -790232474;    double wWHybvdzpaAPjvRwt89499545 = -279586948;    double wWHybvdzpaAPjvRwt1328010 = -982170770;    double wWHybvdzpaAPjvRwt46086010 = -181622592;    double wWHybvdzpaAPjvRwt85276972 = -537715788;    double wWHybvdzpaAPjvRwt40516321 = -808592273;    double wWHybvdzpaAPjvRwt30728507 = -577411496;    double wWHybvdzpaAPjvRwt13007433 = -270505594;    double wWHybvdzpaAPjvRwt35280563 = -28649755;    double wWHybvdzpaAPjvRwt31125349 = -588171071;    double wWHybvdzpaAPjvRwt37293994 = 80748762;    double wWHybvdzpaAPjvRwt53243679 = -483515011;     wWHybvdzpaAPjvRwt43501808 = wWHybvdzpaAPjvRwt31708517;     wWHybvdzpaAPjvRwt31708517 = wWHybvdzpaAPjvRwt96316433;     wWHybvdzpaAPjvRwt96316433 = wWHybvdzpaAPjvRwt92862716;     wWHybvdzpaAPjvRwt92862716 = wWHybvdzpaAPjvRwt60864623;     wWHybvdzpaAPjvRwt60864623 = wWHybvdzpaAPjvRwt33820717;     wWHybvdzpaAPjvRwt33820717 = wWHybvdzpaAPjvRwt98849222;     wWHybvdzpaAPjvRwt98849222 = wWHybvdzpaAPjvRwt45494621;     wWHybvdzpaAPjvRwt45494621 = wWHybvdzpaAPjvRwt14091493;     wWHybvdzpaAPjvRwt14091493 = wWHybvdzpaAPjvRwt18804735;     wWHybvdzpaAPjvRwt18804735 = wWHybvdzpaAPjvRwt99107374;     wWHybvdzpaAPjvRwt99107374 = wWHybvdzpaAPjvRwt94157954;     wWHybvdzpaAPjvRwt94157954 = wWHybvdzpaAPjvRwt73809101;     wWHybvdzpaAPjvRwt73809101 = wWHybvdzpaAPjvRwt50749378;     wWHybvdzpaAPjvRwt50749378 = wWHybvdzpaAPjvRwt39295757;     wWHybvdzpaAPjvRwt39295757 = wWHybvdzpaAPjvRwt52714694;     wWHybvdzpaAPjvRwt52714694 = wWHybvdzpaAPjvRwt43900357;     wWHybvdzpaAPjvRwt43900357 = wWHybvdzpaAPjvRwt6616897;     wWHybvdzpaAPjvRwt6616897 = wWHybvdzpaAPjvRwt9156110;     wWHybvdzpaAPjvRwt9156110 = wWHybvdzpaAPjvRwt83366908;     wWHybvdzpaAPjvRwt83366908 = wWHybvdzpaAPjvRwt82247419;     wWHybvdzpaAPjvRwt82247419 = wWHybvdzpaAPjvRwt53725926;     wWHybvdzpaAPjvRwt53725926 = wWHybvdzpaAPjvRwt3712988;     wWHybvdzpaAPjvRwt3712988 = wWHybvdzpaAPjvRwt14851651;     wWHybvdzpaAPjvRwt14851651 = wWHybvdzpaAPjvRwt30978438;     wWHybvdzpaAPjvRwt30978438 = wWHybvdzpaAPjvRwt49538220;     wWHybvdzpaAPjvRwt49538220 = wWHybvdzpaAPjvRwt86909873;     wWHybvdzpaAPjvRwt86909873 = wWHybvdzpaAPjvRwt39765959;     wWHybvdzpaAPjvRwt39765959 = wWHybvdzpaAPjvRwt84090810;     wWHybvdzpaAPjvRwt84090810 = wWHybvdzpaAPjvRwt47422713;     wWHybvdzpaAPjvRwt47422713 = wWHybvdzpaAPjvRwt85705494;     wWHybvdzpaAPjvRwt85705494 = wWHybvdzpaAPjvRwt91834859;     wWHybvdzpaAPjvRwt91834859 = wWHybvdzpaAPjvRwt78631304;     wWHybvdzpaAPjvRwt78631304 = wWHybvdzpaAPjvRwt62653614;     wWHybvdzpaAPjvRwt62653614 = wWHybvdzpaAPjvRwt79275428;     wWHybvdzpaAPjvRwt79275428 = wWHybvdzpaAPjvRwt29323538;     wWHybvdzpaAPjvRwt29323538 = wWHybvdzpaAPjvRwt27550793;     wWHybvdzpaAPjvRwt27550793 = wWHybvdzpaAPjvRwt47161251;     wWHybvdzpaAPjvRwt47161251 = wWHybvdzpaAPjvRwt51842724;     wWHybvdzpaAPjvRwt51842724 = wWHybvdzpaAPjvRwt77639350;     wWHybvdzpaAPjvRwt77639350 = wWHybvdzpaAPjvRwt72090697;     wWHybvdzpaAPjvRwt72090697 = wWHybvdzpaAPjvRwt96684373;     wWHybvdzpaAPjvRwt96684373 = wWHybvdzpaAPjvRwt97794128;     wWHybvdzpaAPjvRwt97794128 = wWHybvdzpaAPjvRwt70393234;     wWHybvdzpaAPjvRwt70393234 = wWHybvdzpaAPjvRwt60516195;     wWHybvdzpaAPjvRwt60516195 = wWHybvdzpaAPjvRwt62479831;     wWHybvdzpaAPjvRwt62479831 = wWHybvdzpaAPjvRwt60222554;     wWHybvdzpaAPjvRwt60222554 = wWHybvdzpaAPjvRwt79144743;     wWHybvdzpaAPjvRwt79144743 = wWHybvdzpaAPjvRwt75442922;     wWHybvdzpaAPjvRwt75442922 = wWHybvdzpaAPjvRwt65365267;     wWHybvdzpaAPjvRwt65365267 = wWHybvdzpaAPjvRwt37981975;     wWHybvdzpaAPjvRwt37981975 = wWHybvdzpaAPjvRwt81064672;     wWHybvdzpaAPjvRwt81064672 = wWHybvdzpaAPjvRwt55784021;     wWHybvdzpaAPjvRwt55784021 = wWHybvdzpaAPjvRwt54004504;     wWHybvdzpaAPjvRwt54004504 = wWHybvdzpaAPjvRwt61937433;     wWHybvdzpaAPjvRwt61937433 = wWHybvdzpaAPjvRwt89775882;     wWHybvdzpaAPjvRwt89775882 = wWHybvdzpaAPjvRwt27995529;     wWHybvdzpaAPjvRwt27995529 = wWHybvdzpaAPjvRwt81464782;     wWHybvdzpaAPjvRwt81464782 = wWHybvdzpaAPjvRwt61884278;     wWHybvdzpaAPjvRwt61884278 = wWHybvdzpaAPjvRwt11326403;     wWHybvdzpaAPjvRwt11326403 = wWHybvdzpaAPjvRwt46910844;     wWHybvdzpaAPjvRwt46910844 = wWHybvdzpaAPjvRwt59083264;     wWHybvdzpaAPjvRwt59083264 = wWHybvdzpaAPjvRwt61403810;     wWHybvdzpaAPjvRwt61403810 = wWHybvdzpaAPjvRwt66668779;     wWHybvdzpaAPjvRwt66668779 = wWHybvdzpaAPjvRwt33099241;     wWHybvdzpaAPjvRwt33099241 = wWHybvdzpaAPjvRwt7272516;     wWHybvdzpaAPjvRwt7272516 = wWHybvdzpaAPjvRwt15526651;     wWHybvdzpaAPjvRwt15526651 = wWHybvdzpaAPjvRwt11155488;     wWHybvdzpaAPjvRwt11155488 = wWHybvdzpaAPjvRwt71473950;     wWHybvdzpaAPjvRwt71473950 = wWHybvdzpaAPjvRwt9972219;     wWHybvdzpaAPjvRwt9972219 = wWHybvdzpaAPjvRwt25163901;     wWHybvdzpaAPjvRwt25163901 = wWHybvdzpaAPjvRwt96739106;     wWHybvdzpaAPjvRwt96739106 = wWHybvdzpaAPjvRwt54774172;     wWHybvdzpaAPjvRwt54774172 = wWHybvdzpaAPjvRwt31516760;     wWHybvdzpaAPjvRwt31516760 = wWHybvdzpaAPjvRwt11276212;     wWHybvdzpaAPjvRwt11276212 = wWHybvdzpaAPjvRwt85563045;     wWHybvdzpaAPjvRwt85563045 = wWHybvdzpaAPjvRwt55931797;     wWHybvdzpaAPjvRwt55931797 = wWHybvdzpaAPjvRwt33319753;     wWHybvdzpaAPjvRwt33319753 = wWHybvdzpaAPjvRwt54335456;     wWHybvdzpaAPjvRwt54335456 = wWHybvdzpaAPjvRwt68498606;     wWHybvdzpaAPjvRwt68498606 = wWHybvdzpaAPjvRwt89315665;     wWHybvdzpaAPjvRwt89315665 = wWHybvdzpaAPjvRwt7765130;     wWHybvdzpaAPjvRwt7765130 = wWHybvdzpaAPjvRwt64323036;     wWHybvdzpaAPjvRwt64323036 = wWHybvdzpaAPjvRwt18725544;     wWHybvdzpaAPjvRwt18725544 = wWHybvdzpaAPjvRwt9440739;     wWHybvdzpaAPjvRwt9440739 = wWHybvdzpaAPjvRwt4640822;     wWHybvdzpaAPjvRwt4640822 = wWHybvdzpaAPjvRwt36050838;     wWHybvdzpaAPjvRwt36050838 = wWHybvdzpaAPjvRwt24626800;     wWHybvdzpaAPjvRwt24626800 = wWHybvdzpaAPjvRwt716181;     wWHybvdzpaAPjvRwt716181 = wWHybvdzpaAPjvRwt89499545;     wWHybvdzpaAPjvRwt89499545 = wWHybvdzpaAPjvRwt1328010;     wWHybvdzpaAPjvRwt1328010 = wWHybvdzpaAPjvRwt46086010;     wWHybvdzpaAPjvRwt46086010 = wWHybvdzpaAPjvRwt85276972;     wWHybvdzpaAPjvRwt85276972 = wWHybvdzpaAPjvRwt40516321;     wWHybvdzpaAPjvRwt40516321 = wWHybvdzpaAPjvRwt30728507;     wWHybvdzpaAPjvRwt30728507 = wWHybvdzpaAPjvRwt13007433;     wWHybvdzpaAPjvRwt13007433 = wWHybvdzpaAPjvRwt35280563;     wWHybvdzpaAPjvRwt35280563 = wWHybvdzpaAPjvRwt31125349;     wWHybvdzpaAPjvRwt31125349 = wWHybvdzpaAPjvRwt37293994;     wWHybvdzpaAPjvRwt37293994 = wWHybvdzpaAPjvRwt53243679;     wWHybvdzpaAPjvRwt53243679 = wWHybvdzpaAPjvRwt43501808;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void dxjvmHsqQHGlDSbQ59233920() {     double gUlGoVaFawTLsnlhQ49618915 = -595416900;    double gUlGoVaFawTLsnlhQ75081796 = -169862048;    double gUlGoVaFawTLsnlhQ12358617 = 33545810;    double gUlGoVaFawTLsnlhQ19937498 = -470570888;    double gUlGoVaFawTLsnlhQ29760112 = 21565156;    double gUlGoVaFawTLsnlhQ25237920 = -810824687;    double gUlGoVaFawTLsnlhQ59338188 = -849769479;    double gUlGoVaFawTLsnlhQ95254512 = 80065965;    double gUlGoVaFawTLsnlhQ78733988 = -298021449;    double gUlGoVaFawTLsnlhQ60034394 = -488857211;    double gUlGoVaFawTLsnlhQ59217665 = -505152652;    double gUlGoVaFawTLsnlhQ25980997 = -402972668;    double gUlGoVaFawTLsnlhQ60018220 = -41468491;    double gUlGoVaFawTLsnlhQ33893055 = -504996175;    double gUlGoVaFawTLsnlhQ35038184 = -348344057;    double gUlGoVaFawTLsnlhQ94220087 = -542396681;    double gUlGoVaFawTLsnlhQ37736360 = -574182603;    double gUlGoVaFawTLsnlhQ3670050 = -196061706;    double gUlGoVaFawTLsnlhQ70985951 = -776260650;    double gUlGoVaFawTLsnlhQ63917073 = 56139684;    double gUlGoVaFawTLsnlhQ70800612 = -72049001;    double gUlGoVaFawTLsnlhQ40246949 = 91968643;    double gUlGoVaFawTLsnlhQ39303779 = -631685115;    double gUlGoVaFawTLsnlhQ98844024 = -320586539;    double gUlGoVaFawTLsnlhQ74678088 = -178109728;    double gUlGoVaFawTLsnlhQ74259895 = -8798516;    double gUlGoVaFawTLsnlhQ85369651 = -774578728;    double gUlGoVaFawTLsnlhQ52873084 = -909092593;    double gUlGoVaFawTLsnlhQ50223066 = -113711921;    double gUlGoVaFawTLsnlhQ16038350 = -836514303;    double gUlGoVaFawTLsnlhQ77948043 = -360213632;    double gUlGoVaFawTLsnlhQ89445481 = -436575882;    double gUlGoVaFawTLsnlhQ88342252 = -936330016;    double gUlGoVaFawTLsnlhQ57484178 = -739532224;    double gUlGoVaFawTLsnlhQ40750080 = -301733425;    double gUlGoVaFawTLsnlhQ98049451 = -771291517;    double gUlGoVaFawTLsnlhQ8812727 = -952598180;    double gUlGoVaFawTLsnlhQ81251463 = -784130360;    double gUlGoVaFawTLsnlhQ25452978 = -130157549;    double gUlGoVaFawTLsnlhQ65141288 = -723262646;    double gUlGoVaFawTLsnlhQ77759921 = -203420351;    double gUlGoVaFawTLsnlhQ28834831 = -40522939;    double gUlGoVaFawTLsnlhQ75861106 = -179921003;    double gUlGoVaFawTLsnlhQ57800975 = -751583217;    double gUlGoVaFawTLsnlhQ72468660 = -49688089;    double gUlGoVaFawTLsnlhQ74419026 = -912041551;    double gUlGoVaFawTLsnlhQ78001128 = -910252030;    double gUlGoVaFawTLsnlhQ87101684 = 908769;    double gUlGoVaFawTLsnlhQ85118651 = -882175563;    double gUlGoVaFawTLsnlhQ78276920 = -79600101;    double gUlGoVaFawTLsnlhQ73682653 = -641574420;    double gUlGoVaFawTLsnlhQ91874391 = -882101063;    double gUlGoVaFawTLsnlhQ4032479 = 21080803;    double gUlGoVaFawTLsnlhQ49973384 = -123278819;    double gUlGoVaFawTLsnlhQ88433308 = -798289768;    double gUlGoVaFawTLsnlhQ9371966 = -587385544;    double gUlGoVaFawTLsnlhQ35778017 = -538176934;    double gUlGoVaFawTLsnlhQ13514593 = -645867652;    double gUlGoVaFawTLsnlhQ45259409 = -192461160;    double gUlGoVaFawTLsnlhQ55500216 = -969636328;    double gUlGoVaFawTLsnlhQ39868268 = 63754040;    double gUlGoVaFawTLsnlhQ6465105 = -940676886;    double gUlGoVaFawTLsnlhQ45031446 = -806222115;    double gUlGoVaFawTLsnlhQ62695638 = -461507147;    double gUlGoVaFawTLsnlhQ82086350 = -28643580;    double gUlGoVaFawTLsnlhQ69772184 = 31423230;    double gUlGoVaFawTLsnlhQ37638745 = -466642652;    double gUlGoVaFawTLsnlhQ2534042 = -301936267;    double gUlGoVaFawTLsnlhQ93142974 = -103262751;    double gUlGoVaFawTLsnlhQ36988732 = -577052540;    double gUlGoVaFawTLsnlhQ85407360 = -589798502;    double gUlGoVaFawTLsnlhQ56484897 = -790052244;    double gUlGoVaFawTLsnlhQ78217071 = 34095842;    double gUlGoVaFawTLsnlhQ5844664 = 47001995;    double gUlGoVaFawTLsnlhQ86157152 = -740439966;    double gUlGoVaFawTLsnlhQ41965782 = 68473937;    double gUlGoVaFawTLsnlhQ64385843 = -728110354;    double gUlGoVaFawTLsnlhQ81502804 = -880101899;    double gUlGoVaFawTLsnlhQ26375365 = -170898450;    double gUlGoVaFawTLsnlhQ259062 = -266068178;    double gUlGoVaFawTLsnlhQ96258766 = -98546486;    double gUlGoVaFawTLsnlhQ98267966 = -675487497;    double gUlGoVaFawTLsnlhQ67754432 = 73082969;    double gUlGoVaFawTLsnlhQ71946146 = 65888180;    double gUlGoVaFawTLsnlhQ42355697 = -94939884;    double gUlGoVaFawTLsnlhQ86073652 = -478112569;    double gUlGoVaFawTLsnlhQ85413002 = -357656686;    double gUlGoVaFawTLsnlhQ38368868 = -713051198;    double gUlGoVaFawTLsnlhQ69050870 = -941242457;    double gUlGoVaFawTLsnlhQ31378115 = -714347881;    double gUlGoVaFawTLsnlhQ62271435 = -133114584;    double gUlGoVaFawTLsnlhQ95298134 = -206730529;    double gUlGoVaFawTLsnlhQ35992054 = -491669200;    double gUlGoVaFawTLsnlhQ69952762 = -160521222;    double gUlGoVaFawTLsnlhQ25273020 = -687016687;    double gUlGoVaFawTLsnlhQ71294816 = -262743465;    double gUlGoVaFawTLsnlhQ83803384 = -234300824;    double gUlGoVaFawTLsnlhQ13165469 = -718413857;    double gUlGoVaFawTLsnlhQ75714624 = -622939637;    double gUlGoVaFawTLsnlhQ2696476 = -595416900;     gUlGoVaFawTLsnlhQ49618915 = gUlGoVaFawTLsnlhQ75081796;     gUlGoVaFawTLsnlhQ75081796 = gUlGoVaFawTLsnlhQ12358617;     gUlGoVaFawTLsnlhQ12358617 = gUlGoVaFawTLsnlhQ19937498;     gUlGoVaFawTLsnlhQ19937498 = gUlGoVaFawTLsnlhQ29760112;     gUlGoVaFawTLsnlhQ29760112 = gUlGoVaFawTLsnlhQ25237920;     gUlGoVaFawTLsnlhQ25237920 = gUlGoVaFawTLsnlhQ59338188;     gUlGoVaFawTLsnlhQ59338188 = gUlGoVaFawTLsnlhQ95254512;     gUlGoVaFawTLsnlhQ95254512 = gUlGoVaFawTLsnlhQ78733988;     gUlGoVaFawTLsnlhQ78733988 = gUlGoVaFawTLsnlhQ60034394;     gUlGoVaFawTLsnlhQ60034394 = gUlGoVaFawTLsnlhQ59217665;     gUlGoVaFawTLsnlhQ59217665 = gUlGoVaFawTLsnlhQ25980997;     gUlGoVaFawTLsnlhQ25980997 = gUlGoVaFawTLsnlhQ60018220;     gUlGoVaFawTLsnlhQ60018220 = gUlGoVaFawTLsnlhQ33893055;     gUlGoVaFawTLsnlhQ33893055 = gUlGoVaFawTLsnlhQ35038184;     gUlGoVaFawTLsnlhQ35038184 = gUlGoVaFawTLsnlhQ94220087;     gUlGoVaFawTLsnlhQ94220087 = gUlGoVaFawTLsnlhQ37736360;     gUlGoVaFawTLsnlhQ37736360 = gUlGoVaFawTLsnlhQ3670050;     gUlGoVaFawTLsnlhQ3670050 = gUlGoVaFawTLsnlhQ70985951;     gUlGoVaFawTLsnlhQ70985951 = gUlGoVaFawTLsnlhQ63917073;     gUlGoVaFawTLsnlhQ63917073 = gUlGoVaFawTLsnlhQ70800612;     gUlGoVaFawTLsnlhQ70800612 = gUlGoVaFawTLsnlhQ40246949;     gUlGoVaFawTLsnlhQ40246949 = gUlGoVaFawTLsnlhQ39303779;     gUlGoVaFawTLsnlhQ39303779 = gUlGoVaFawTLsnlhQ98844024;     gUlGoVaFawTLsnlhQ98844024 = gUlGoVaFawTLsnlhQ74678088;     gUlGoVaFawTLsnlhQ74678088 = gUlGoVaFawTLsnlhQ74259895;     gUlGoVaFawTLsnlhQ74259895 = gUlGoVaFawTLsnlhQ85369651;     gUlGoVaFawTLsnlhQ85369651 = gUlGoVaFawTLsnlhQ52873084;     gUlGoVaFawTLsnlhQ52873084 = gUlGoVaFawTLsnlhQ50223066;     gUlGoVaFawTLsnlhQ50223066 = gUlGoVaFawTLsnlhQ16038350;     gUlGoVaFawTLsnlhQ16038350 = gUlGoVaFawTLsnlhQ77948043;     gUlGoVaFawTLsnlhQ77948043 = gUlGoVaFawTLsnlhQ89445481;     gUlGoVaFawTLsnlhQ89445481 = gUlGoVaFawTLsnlhQ88342252;     gUlGoVaFawTLsnlhQ88342252 = gUlGoVaFawTLsnlhQ57484178;     gUlGoVaFawTLsnlhQ57484178 = gUlGoVaFawTLsnlhQ40750080;     gUlGoVaFawTLsnlhQ40750080 = gUlGoVaFawTLsnlhQ98049451;     gUlGoVaFawTLsnlhQ98049451 = gUlGoVaFawTLsnlhQ8812727;     gUlGoVaFawTLsnlhQ8812727 = gUlGoVaFawTLsnlhQ81251463;     gUlGoVaFawTLsnlhQ81251463 = gUlGoVaFawTLsnlhQ25452978;     gUlGoVaFawTLsnlhQ25452978 = gUlGoVaFawTLsnlhQ65141288;     gUlGoVaFawTLsnlhQ65141288 = gUlGoVaFawTLsnlhQ77759921;     gUlGoVaFawTLsnlhQ77759921 = gUlGoVaFawTLsnlhQ28834831;     gUlGoVaFawTLsnlhQ28834831 = gUlGoVaFawTLsnlhQ75861106;     gUlGoVaFawTLsnlhQ75861106 = gUlGoVaFawTLsnlhQ57800975;     gUlGoVaFawTLsnlhQ57800975 = gUlGoVaFawTLsnlhQ72468660;     gUlGoVaFawTLsnlhQ72468660 = gUlGoVaFawTLsnlhQ74419026;     gUlGoVaFawTLsnlhQ74419026 = gUlGoVaFawTLsnlhQ78001128;     gUlGoVaFawTLsnlhQ78001128 = gUlGoVaFawTLsnlhQ87101684;     gUlGoVaFawTLsnlhQ87101684 = gUlGoVaFawTLsnlhQ85118651;     gUlGoVaFawTLsnlhQ85118651 = gUlGoVaFawTLsnlhQ78276920;     gUlGoVaFawTLsnlhQ78276920 = gUlGoVaFawTLsnlhQ73682653;     gUlGoVaFawTLsnlhQ73682653 = gUlGoVaFawTLsnlhQ91874391;     gUlGoVaFawTLsnlhQ91874391 = gUlGoVaFawTLsnlhQ4032479;     gUlGoVaFawTLsnlhQ4032479 = gUlGoVaFawTLsnlhQ49973384;     gUlGoVaFawTLsnlhQ49973384 = gUlGoVaFawTLsnlhQ88433308;     gUlGoVaFawTLsnlhQ88433308 = gUlGoVaFawTLsnlhQ9371966;     gUlGoVaFawTLsnlhQ9371966 = gUlGoVaFawTLsnlhQ35778017;     gUlGoVaFawTLsnlhQ35778017 = gUlGoVaFawTLsnlhQ13514593;     gUlGoVaFawTLsnlhQ13514593 = gUlGoVaFawTLsnlhQ45259409;     gUlGoVaFawTLsnlhQ45259409 = gUlGoVaFawTLsnlhQ55500216;     gUlGoVaFawTLsnlhQ55500216 = gUlGoVaFawTLsnlhQ39868268;     gUlGoVaFawTLsnlhQ39868268 = gUlGoVaFawTLsnlhQ6465105;     gUlGoVaFawTLsnlhQ6465105 = gUlGoVaFawTLsnlhQ45031446;     gUlGoVaFawTLsnlhQ45031446 = gUlGoVaFawTLsnlhQ62695638;     gUlGoVaFawTLsnlhQ62695638 = gUlGoVaFawTLsnlhQ82086350;     gUlGoVaFawTLsnlhQ82086350 = gUlGoVaFawTLsnlhQ69772184;     gUlGoVaFawTLsnlhQ69772184 = gUlGoVaFawTLsnlhQ37638745;     gUlGoVaFawTLsnlhQ37638745 = gUlGoVaFawTLsnlhQ2534042;     gUlGoVaFawTLsnlhQ2534042 = gUlGoVaFawTLsnlhQ93142974;     gUlGoVaFawTLsnlhQ93142974 = gUlGoVaFawTLsnlhQ36988732;     gUlGoVaFawTLsnlhQ36988732 = gUlGoVaFawTLsnlhQ85407360;     gUlGoVaFawTLsnlhQ85407360 = gUlGoVaFawTLsnlhQ56484897;     gUlGoVaFawTLsnlhQ56484897 = gUlGoVaFawTLsnlhQ78217071;     gUlGoVaFawTLsnlhQ78217071 = gUlGoVaFawTLsnlhQ5844664;     gUlGoVaFawTLsnlhQ5844664 = gUlGoVaFawTLsnlhQ86157152;     gUlGoVaFawTLsnlhQ86157152 = gUlGoVaFawTLsnlhQ41965782;     gUlGoVaFawTLsnlhQ41965782 = gUlGoVaFawTLsnlhQ64385843;     gUlGoVaFawTLsnlhQ64385843 = gUlGoVaFawTLsnlhQ81502804;     gUlGoVaFawTLsnlhQ81502804 = gUlGoVaFawTLsnlhQ26375365;     gUlGoVaFawTLsnlhQ26375365 = gUlGoVaFawTLsnlhQ259062;     gUlGoVaFawTLsnlhQ259062 = gUlGoVaFawTLsnlhQ96258766;     gUlGoVaFawTLsnlhQ96258766 = gUlGoVaFawTLsnlhQ98267966;     gUlGoVaFawTLsnlhQ98267966 = gUlGoVaFawTLsnlhQ67754432;     gUlGoVaFawTLsnlhQ67754432 = gUlGoVaFawTLsnlhQ71946146;     gUlGoVaFawTLsnlhQ71946146 = gUlGoVaFawTLsnlhQ42355697;     gUlGoVaFawTLsnlhQ42355697 = gUlGoVaFawTLsnlhQ86073652;     gUlGoVaFawTLsnlhQ86073652 = gUlGoVaFawTLsnlhQ85413002;     gUlGoVaFawTLsnlhQ85413002 = gUlGoVaFawTLsnlhQ38368868;     gUlGoVaFawTLsnlhQ38368868 = gUlGoVaFawTLsnlhQ69050870;     gUlGoVaFawTLsnlhQ69050870 = gUlGoVaFawTLsnlhQ31378115;     gUlGoVaFawTLsnlhQ31378115 = gUlGoVaFawTLsnlhQ62271435;     gUlGoVaFawTLsnlhQ62271435 = gUlGoVaFawTLsnlhQ95298134;     gUlGoVaFawTLsnlhQ95298134 = gUlGoVaFawTLsnlhQ35992054;     gUlGoVaFawTLsnlhQ35992054 = gUlGoVaFawTLsnlhQ69952762;     gUlGoVaFawTLsnlhQ69952762 = gUlGoVaFawTLsnlhQ25273020;     gUlGoVaFawTLsnlhQ25273020 = gUlGoVaFawTLsnlhQ71294816;     gUlGoVaFawTLsnlhQ71294816 = gUlGoVaFawTLsnlhQ83803384;     gUlGoVaFawTLsnlhQ83803384 = gUlGoVaFawTLsnlhQ13165469;     gUlGoVaFawTLsnlhQ13165469 = gUlGoVaFawTLsnlhQ75714624;     gUlGoVaFawTLsnlhQ75714624 = gUlGoVaFawTLsnlhQ2696476;     gUlGoVaFawTLsnlhQ2696476 = gUlGoVaFawTLsnlhQ49618915;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void yTksaqOQEEpXNcxH26525520() {     double teGuqlUEovwkPSIxM85077883 = 38306200;    double teGuqlUEovwkPSIxM88353874 = -50413655;    double teGuqlUEovwkPSIxM49745093 = -732121265;    double teGuqlUEovwkPSIxM92160852 = -291721959;    double teGuqlUEovwkPSIxM86045380 = -84033689;    double teGuqlUEovwkPSIxM61460791 = 26827418;    double teGuqlUEovwkPSIxM69995594 = -694495213;    double teGuqlUEovwkPSIxM57386073 = -664168176;    double teGuqlUEovwkPSIxM80251593 = -786352856;    double teGuqlUEovwkPSIxM74937106 = -153544209;    double teGuqlUEovwkPSIxM97399400 = -307160381;    double teGuqlUEovwkPSIxM26549146 = -267093439;    double teGuqlUEovwkPSIxM98252865 = -593480996;    double teGuqlUEovwkPSIxM87033918 = -481359585;    double teGuqlUEovwkPSIxM93232700 = -851769072;    double teGuqlUEovwkPSIxM77761388 = -178208288;    double teGuqlUEovwkPSIxM93572296 = -1157156;    double teGuqlUEovwkPSIxM3430242 = -269489775;    double teGuqlUEovwkPSIxM71536281 = 7175928;    double teGuqlUEovwkPSIxM57754292 = -370421382;    double teGuqlUEovwkPSIxM83887475 = -829323690;    double teGuqlUEovwkPSIxM43668474 = 5332349;    double teGuqlUEovwkPSIxM9829159 = -269317299;    double teGuqlUEovwkPSIxM92636969 = -831411143;    double teGuqlUEovwkPSIxM82696039 = -202094926;    double teGuqlUEovwkPSIxM84958066 = -797375323;    double teGuqlUEovwkPSIxM45790225 = -580862868;    double teGuqlUEovwkPSIxM93601508 = 21135232;    double teGuqlUEovwkPSIxM96891280 = -687806723;    double teGuqlUEovwkPSIxM21571761 = -871021474;    double teGuqlUEovwkPSIxM96531275 = -912194119;    double teGuqlUEovwkPSIxM27268500 = -209547297;    double teGuqlUEovwkPSIxM560264 = 16074006;    double teGuqlUEovwkPSIxM87232605 = -804226038;    double teGuqlUEovwkPSIxM6558534 = -252833675;    double teGuqlUEovwkPSIxM49610295 = -985856823;    double teGuqlUEovwkPSIxM58224544 = -817294220;    double teGuqlUEovwkPSIxM32466867 = -766785941;    double teGuqlUEovwkPSIxM55843540 = -881033825;    double teGuqlUEovwkPSIxM29412821 = -188525192;    double teGuqlUEovwkPSIxM11211529 = 27380735;    double teGuqlUEovwkPSIxM18199958 = -735068230;    double teGuqlUEovwkPSIxM65244485 = -833101789;    double teGuqlUEovwkPSIxM97932312 = -255915627;    double teGuqlUEovwkPSIxM72485118 = -976199251;    double teGuqlUEovwkPSIxM20380355 = -950965183;    double teGuqlUEovwkPSIxM97346292 = -294238391;    double teGuqlUEovwkPSIxM4827997 = -165617825;    double teGuqlUEovwkPSIxM8011318 = -545912443;    double teGuqlUEovwkPSIxM2806510 = -570626917;    double teGuqlUEovwkPSIxM54453377 = -515850977;    double teGuqlUEovwkPSIxM75359864 = 4246591;    double teGuqlUEovwkPSIxM72653627 = -834964751;    double teGuqlUEovwkPSIxM65965231 = -314787319;    double teGuqlUEovwkPSIxM42638045 = -44779914;    double teGuqlUEovwkPSIxM41409409 = -967026149;    double teGuqlUEovwkPSIxM78524715 = -781096356;    double teGuqlUEovwkPSIxM57108124 = -900710122;    double teGuqlUEovwkPSIxM9464813 = 10372967;    double teGuqlUEovwkPSIxM1087314 = -286658366;    double teGuqlUEovwkPSIxM15670567 = -392309714;    double teGuqlUEovwkPSIxM76394086 = -615630446;    double teGuqlUEovwkPSIxM60494792 = -976361453;    double teGuqlUEovwkPSIxM58679832 = -915331383;    double teGuqlUEovwkPSIxM78405831 = -241350091;    double teGuqlUEovwkPSIxM70130900 = 2386915;    double teGuqlUEovwkPSIxM25988883 = -183167446;    double teGuqlUEovwkPSIxM11020261 = -789254958;    double teGuqlUEovwkPSIxM80475384 = -128525910;    double teGuqlUEovwkPSIxM43622405 = -865912249;    double teGuqlUEovwkPSIxM19536845 = -360914069;    double teGuqlUEovwkPSIxM61105429 = -234371216;    double teGuqlUEovwkPSIxM47586702 = -388455950;    double teGuqlUEovwkPSIxM42123461 = -804298880;    double teGuqlUEovwkPSIxM46542763 = -297802118;    double teGuqlUEovwkPSIxM65687518 = 5744539;    double teGuqlUEovwkPSIxM78423989 = -161565862;    double teGuqlUEovwkPSIxM11896846 = 86598327;    double teGuqlUEovwkPSIxM20151851 = -855211893;    double teGuqlUEovwkPSIxM62315684 = -251129744;    double teGuqlUEovwkPSIxM87611774 = -403136933;    double teGuqlUEovwkPSIxM40962229 = -315245044;    double teGuqlUEovwkPSIxM85590191 = -432952325;    double teGuqlUEovwkPSIxM94084771 = -17179807;    double teGuqlUEovwkPSIxM67118384 = -255170497;    double teGuqlUEovwkPSIxM21171411 = -816440710;    double teGuqlUEovwkPSIxM54614872 = -374582546;    double teGuqlUEovwkPSIxM34595032 = -669138675;    double teGuqlUEovwkPSIxM44594560 = -659446124;    double teGuqlUEovwkPSIxM65149124 = -285807526;    double teGuqlUEovwkPSIxM71085580 = -104760467;    double teGuqlUEovwkPSIxM1116420 = -916584098;    double teGuqlUEovwkPSIxM23002054 = -677158908;    double teGuqlUEovwkPSIxM54756226 = -494375459;    double teGuqlUEovwkPSIxM13742255 = -796215479;    double teGuqlUEovwkPSIxM34817443 = -356988819;    double teGuqlUEovwkPSIxM57705165 = -758706777;    double teGuqlUEovwkPSIxM6564653 = -917770407;    double teGuqlUEovwkPSIxM19526482 = 85434464;    double teGuqlUEovwkPSIxM2354218 = 38306200;     teGuqlUEovwkPSIxM85077883 = teGuqlUEovwkPSIxM88353874;     teGuqlUEovwkPSIxM88353874 = teGuqlUEovwkPSIxM49745093;     teGuqlUEovwkPSIxM49745093 = teGuqlUEovwkPSIxM92160852;     teGuqlUEovwkPSIxM92160852 = teGuqlUEovwkPSIxM86045380;     teGuqlUEovwkPSIxM86045380 = teGuqlUEovwkPSIxM61460791;     teGuqlUEovwkPSIxM61460791 = teGuqlUEovwkPSIxM69995594;     teGuqlUEovwkPSIxM69995594 = teGuqlUEovwkPSIxM57386073;     teGuqlUEovwkPSIxM57386073 = teGuqlUEovwkPSIxM80251593;     teGuqlUEovwkPSIxM80251593 = teGuqlUEovwkPSIxM74937106;     teGuqlUEovwkPSIxM74937106 = teGuqlUEovwkPSIxM97399400;     teGuqlUEovwkPSIxM97399400 = teGuqlUEovwkPSIxM26549146;     teGuqlUEovwkPSIxM26549146 = teGuqlUEovwkPSIxM98252865;     teGuqlUEovwkPSIxM98252865 = teGuqlUEovwkPSIxM87033918;     teGuqlUEovwkPSIxM87033918 = teGuqlUEovwkPSIxM93232700;     teGuqlUEovwkPSIxM93232700 = teGuqlUEovwkPSIxM77761388;     teGuqlUEovwkPSIxM77761388 = teGuqlUEovwkPSIxM93572296;     teGuqlUEovwkPSIxM93572296 = teGuqlUEovwkPSIxM3430242;     teGuqlUEovwkPSIxM3430242 = teGuqlUEovwkPSIxM71536281;     teGuqlUEovwkPSIxM71536281 = teGuqlUEovwkPSIxM57754292;     teGuqlUEovwkPSIxM57754292 = teGuqlUEovwkPSIxM83887475;     teGuqlUEovwkPSIxM83887475 = teGuqlUEovwkPSIxM43668474;     teGuqlUEovwkPSIxM43668474 = teGuqlUEovwkPSIxM9829159;     teGuqlUEovwkPSIxM9829159 = teGuqlUEovwkPSIxM92636969;     teGuqlUEovwkPSIxM92636969 = teGuqlUEovwkPSIxM82696039;     teGuqlUEovwkPSIxM82696039 = teGuqlUEovwkPSIxM84958066;     teGuqlUEovwkPSIxM84958066 = teGuqlUEovwkPSIxM45790225;     teGuqlUEovwkPSIxM45790225 = teGuqlUEovwkPSIxM93601508;     teGuqlUEovwkPSIxM93601508 = teGuqlUEovwkPSIxM96891280;     teGuqlUEovwkPSIxM96891280 = teGuqlUEovwkPSIxM21571761;     teGuqlUEovwkPSIxM21571761 = teGuqlUEovwkPSIxM96531275;     teGuqlUEovwkPSIxM96531275 = teGuqlUEovwkPSIxM27268500;     teGuqlUEovwkPSIxM27268500 = teGuqlUEovwkPSIxM560264;     teGuqlUEovwkPSIxM560264 = teGuqlUEovwkPSIxM87232605;     teGuqlUEovwkPSIxM87232605 = teGuqlUEovwkPSIxM6558534;     teGuqlUEovwkPSIxM6558534 = teGuqlUEovwkPSIxM49610295;     teGuqlUEovwkPSIxM49610295 = teGuqlUEovwkPSIxM58224544;     teGuqlUEovwkPSIxM58224544 = teGuqlUEovwkPSIxM32466867;     teGuqlUEovwkPSIxM32466867 = teGuqlUEovwkPSIxM55843540;     teGuqlUEovwkPSIxM55843540 = teGuqlUEovwkPSIxM29412821;     teGuqlUEovwkPSIxM29412821 = teGuqlUEovwkPSIxM11211529;     teGuqlUEovwkPSIxM11211529 = teGuqlUEovwkPSIxM18199958;     teGuqlUEovwkPSIxM18199958 = teGuqlUEovwkPSIxM65244485;     teGuqlUEovwkPSIxM65244485 = teGuqlUEovwkPSIxM97932312;     teGuqlUEovwkPSIxM97932312 = teGuqlUEovwkPSIxM72485118;     teGuqlUEovwkPSIxM72485118 = teGuqlUEovwkPSIxM20380355;     teGuqlUEovwkPSIxM20380355 = teGuqlUEovwkPSIxM97346292;     teGuqlUEovwkPSIxM97346292 = teGuqlUEovwkPSIxM4827997;     teGuqlUEovwkPSIxM4827997 = teGuqlUEovwkPSIxM8011318;     teGuqlUEovwkPSIxM8011318 = teGuqlUEovwkPSIxM2806510;     teGuqlUEovwkPSIxM2806510 = teGuqlUEovwkPSIxM54453377;     teGuqlUEovwkPSIxM54453377 = teGuqlUEovwkPSIxM75359864;     teGuqlUEovwkPSIxM75359864 = teGuqlUEovwkPSIxM72653627;     teGuqlUEovwkPSIxM72653627 = teGuqlUEovwkPSIxM65965231;     teGuqlUEovwkPSIxM65965231 = teGuqlUEovwkPSIxM42638045;     teGuqlUEovwkPSIxM42638045 = teGuqlUEovwkPSIxM41409409;     teGuqlUEovwkPSIxM41409409 = teGuqlUEovwkPSIxM78524715;     teGuqlUEovwkPSIxM78524715 = teGuqlUEovwkPSIxM57108124;     teGuqlUEovwkPSIxM57108124 = teGuqlUEovwkPSIxM9464813;     teGuqlUEovwkPSIxM9464813 = teGuqlUEovwkPSIxM1087314;     teGuqlUEovwkPSIxM1087314 = teGuqlUEovwkPSIxM15670567;     teGuqlUEovwkPSIxM15670567 = teGuqlUEovwkPSIxM76394086;     teGuqlUEovwkPSIxM76394086 = teGuqlUEovwkPSIxM60494792;     teGuqlUEovwkPSIxM60494792 = teGuqlUEovwkPSIxM58679832;     teGuqlUEovwkPSIxM58679832 = teGuqlUEovwkPSIxM78405831;     teGuqlUEovwkPSIxM78405831 = teGuqlUEovwkPSIxM70130900;     teGuqlUEovwkPSIxM70130900 = teGuqlUEovwkPSIxM25988883;     teGuqlUEovwkPSIxM25988883 = teGuqlUEovwkPSIxM11020261;     teGuqlUEovwkPSIxM11020261 = teGuqlUEovwkPSIxM80475384;     teGuqlUEovwkPSIxM80475384 = teGuqlUEovwkPSIxM43622405;     teGuqlUEovwkPSIxM43622405 = teGuqlUEovwkPSIxM19536845;     teGuqlUEovwkPSIxM19536845 = teGuqlUEovwkPSIxM61105429;     teGuqlUEovwkPSIxM61105429 = teGuqlUEovwkPSIxM47586702;     teGuqlUEovwkPSIxM47586702 = teGuqlUEovwkPSIxM42123461;     teGuqlUEovwkPSIxM42123461 = teGuqlUEovwkPSIxM46542763;     teGuqlUEovwkPSIxM46542763 = teGuqlUEovwkPSIxM65687518;     teGuqlUEovwkPSIxM65687518 = teGuqlUEovwkPSIxM78423989;     teGuqlUEovwkPSIxM78423989 = teGuqlUEovwkPSIxM11896846;     teGuqlUEovwkPSIxM11896846 = teGuqlUEovwkPSIxM20151851;     teGuqlUEovwkPSIxM20151851 = teGuqlUEovwkPSIxM62315684;     teGuqlUEovwkPSIxM62315684 = teGuqlUEovwkPSIxM87611774;     teGuqlUEovwkPSIxM87611774 = teGuqlUEovwkPSIxM40962229;     teGuqlUEovwkPSIxM40962229 = teGuqlUEovwkPSIxM85590191;     teGuqlUEovwkPSIxM85590191 = teGuqlUEovwkPSIxM94084771;     teGuqlUEovwkPSIxM94084771 = teGuqlUEovwkPSIxM67118384;     teGuqlUEovwkPSIxM67118384 = teGuqlUEovwkPSIxM21171411;     teGuqlUEovwkPSIxM21171411 = teGuqlUEovwkPSIxM54614872;     teGuqlUEovwkPSIxM54614872 = teGuqlUEovwkPSIxM34595032;     teGuqlUEovwkPSIxM34595032 = teGuqlUEovwkPSIxM44594560;     teGuqlUEovwkPSIxM44594560 = teGuqlUEovwkPSIxM65149124;     teGuqlUEovwkPSIxM65149124 = teGuqlUEovwkPSIxM71085580;     teGuqlUEovwkPSIxM71085580 = teGuqlUEovwkPSIxM1116420;     teGuqlUEovwkPSIxM1116420 = teGuqlUEovwkPSIxM23002054;     teGuqlUEovwkPSIxM23002054 = teGuqlUEovwkPSIxM54756226;     teGuqlUEovwkPSIxM54756226 = teGuqlUEovwkPSIxM13742255;     teGuqlUEovwkPSIxM13742255 = teGuqlUEovwkPSIxM34817443;     teGuqlUEovwkPSIxM34817443 = teGuqlUEovwkPSIxM57705165;     teGuqlUEovwkPSIxM57705165 = teGuqlUEovwkPSIxM6564653;     teGuqlUEovwkPSIxM6564653 = teGuqlUEovwkPSIxM19526482;     teGuqlUEovwkPSIxM19526482 = teGuqlUEovwkPSIxM2354218;     teGuqlUEovwkPSIxM2354218 = teGuqlUEovwkPSIxM85077883;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void HrzuzbxnRunIawbu63170494() {     double StDubpZPneTkWFhnp7465001 = -696097763;    double StDubpZPneTkWFhnp72701374 = 1043291;    double StDubpZPneTkWFhnp78514372 = -971775109;    double StDubpZPneTkWFhnp7167541 = -298129491;    double StDubpZPneTkWFhnp84219634 = -675090690;    double StDubpZPneTkWFhnp76343908 = -933780960;    double StDubpZPneTkWFhnp62151333 = -516482520;    double StDubpZPneTkWFhnp33019932 = -34764820;    double StDubpZPneTkWFhnp77414724 = -670548895;    double StDubpZPneTkWFhnp66700691 = -996285542;    double StDubpZPneTkWFhnp34941260 = -303784782;    double StDubpZPneTkWFhnp82452094 = -517838732;    double StDubpZPneTkWFhnp65261046 = -112198315;    double StDubpZPneTkWFhnp29648670 = 90931659;    double StDubpZPneTkWFhnp29967977 = -479246785;    double StDubpZPneTkWFhnp22017301 = -559875831;    double StDubpZPneTkWFhnp43395648 = -278436976;    double StDubpZPneTkWFhnp9406658 = -721232465;    double StDubpZPneTkWFhnp12817913 = -127926197;    double StDubpZPneTkWFhnp36270575 = -623055836;    double StDubpZPneTkWFhnp84770582 = -820810998;    double StDubpZPneTkWFhnp7483693 = -499433964;    double StDubpZPneTkWFhnp13122482 = -445397891;    double StDubpZPneTkWFhnp19136757 = -295404000;    double StDubpZPneTkWFhnp18236286 = -923654051;    double StDubpZPneTkWFhnp73261061 = -141269547;    double StDubpZPneTkWFhnp92879645 = -338449361;    double StDubpZPneTkWFhnp99512958 = -867319221;    double StDubpZPneTkWFhnp3783842 = 67936158;    double StDubpZPneTkWFhnp30728940 = -791317340;    double StDubpZPneTkWFhnp40822080 = -993510271;    double StDubpZPneTkWFhnp84809690 = -861618651;    double StDubpZPneTkWFhnp81598933 = -513269489;    double StDubpZPneTkWFhnp46621292 = -28352888;    double StDubpZPneTkWFhnp82787898 = -316590098;    double StDubpZPneTkWFhnp91303164 = -201252200;    double StDubpZPneTkWFhnp51664256 = -550242172;    double StDubpZPneTkWFhnp86092967 = -634411413;    double StDubpZPneTkWFhnp4151671 = -182079857;    double StDubpZPneTkWFhnp49598535 = -324749222;    double StDubpZPneTkWFhnp39968899 = -475796931;    double StDubpZPneTkWFhnp6708349 = -492976921;    double StDubpZPneTkWFhnp70794677 = -720632373;    double StDubpZPneTkWFhnp97376432 = -253279276;    double StDubpZPneTkWFhnp40468385 = -785342728;    double StDubpZPneTkWFhnp28480637 = -33194278;    double StDubpZPneTkWFhnp9643690 = -872648644;    double StDubpZPneTkWFhnp49426671 = 77847185;    double StDubpZPneTkWFhnp2471223 = -495629142;    double StDubpZPneTkWFhnp99890255 = -712647176;    double StDubpZPneTkWFhnp63322593 = -870927254;    double StDubpZPneTkWFhnp49211121 = -426572691;    double StDubpZPneTkWFhnp50968030 = -506875981;    double StDubpZPneTkWFhnp26251776 = -590468128;    double StDubpZPneTkWFhnp78399912 = -185484029;    double StDubpZPneTkWFhnp99981308 = -96663799;    double StDubpZPneTkWFhnp59578892 = -553558819;    double StDubpZPneTkWFhnp59377616 = -576371109;    double StDubpZPneTkWFhnp88931254 = -374475440;    double StDubpZPneTkWFhnp10958574 = -433821144;    double StDubpZPneTkWFhnp83464262 = -495331600;    double StDubpZPneTkWFhnp62638375 = -649163299;    double StDubpZPneTkWFhnp29236090 = -2700978;    double StDubpZPneTkWFhnp46685784 = -879231556;    double StDubpZPneTkWFhnp25878611 = 97224728;    double StDubpZPneTkWFhnp50131569 = -442166131;    double StDubpZPneTkWFhnp853161 = 95430756;    double StDubpZPneTkWFhnp18639754 = 16154572;    double StDubpZPneTkWFhnp46860772 = -592478244;    double StDubpZPneTkWFhnp38664813 = -177994586;    double StDubpZPneTkWFhnp70353045 = 90366340;    double StDubpZPneTkWFhnp57302680 = -644025563;    double StDubpZPneTkWFhnp5254987 = -439152609;    double StDubpZPneTkWFhnp63219377 = -803176976;    double StDubpZPneTkWFhnp96301675 = -47258905;    double StDubpZPneTkWFhnp78062234 = -227834078;    double StDubpZPneTkWFhnp36689016 = -778801592;    double StDubpZPneTkWFhnp15746050 = -92118616;    double StDubpZPneTkWFhnp78668371 = -510061273;    double StDubpZPneTkWFhnp89755649 = -790459773;    double StDubpZPneTkWFhnp63617372 = -268620904;    double StDubpZPneTkWFhnp43452975 = -316296546;    double StDubpZPneTkWFhnp97041736 = -271690080;    double StDubpZPneTkWFhnp3893587 = -219416667;    double StDubpZPneTkWFhnp67406347 = -920390087;    double StDubpZPneTkWFhnp91610959 = -466937580;    double StDubpZPneTkWFhnp33841660 = -254742670;    double StDubpZPneTkWFhnp55347157 = -922801361;    double StDubpZPneTkWFhnp68221380 = -842868859;    double StDubpZPneTkWFhnp82806590 = -119926299;    double StDubpZPneTkWFhnp31724272 = -647693381;    double StDubpZPneTkWFhnp92286640 = -973871063;    double StDubpZPneTkWFhnp97161713 = -159935973;    double StDubpZPneTkWFhnp93193097 = -748258714;    double StDubpZPneTkWFhnp66134272 = -829417623;    double StDubpZPneTkWFhnp77330524 = -826633633;    double StDubpZPneTkWFhnp77472258 = -390275943;    double StDubpZPneTkWFhnp24108893 = -841400818;    double StDubpZPneTkWFhnp71497821 = -250504004;    double StDubpZPneTkWFhnp90336815 = -696097763;     StDubpZPneTkWFhnp7465001 = StDubpZPneTkWFhnp72701374;     StDubpZPneTkWFhnp72701374 = StDubpZPneTkWFhnp78514372;     StDubpZPneTkWFhnp78514372 = StDubpZPneTkWFhnp7167541;     StDubpZPneTkWFhnp7167541 = StDubpZPneTkWFhnp84219634;     StDubpZPneTkWFhnp84219634 = StDubpZPneTkWFhnp76343908;     StDubpZPneTkWFhnp76343908 = StDubpZPneTkWFhnp62151333;     StDubpZPneTkWFhnp62151333 = StDubpZPneTkWFhnp33019932;     StDubpZPneTkWFhnp33019932 = StDubpZPneTkWFhnp77414724;     StDubpZPneTkWFhnp77414724 = StDubpZPneTkWFhnp66700691;     StDubpZPneTkWFhnp66700691 = StDubpZPneTkWFhnp34941260;     StDubpZPneTkWFhnp34941260 = StDubpZPneTkWFhnp82452094;     StDubpZPneTkWFhnp82452094 = StDubpZPneTkWFhnp65261046;     StDubpZPneTkWFhnp65261046 = StDubpZPneTkWFhnp29648670;     StDubpZPneTkWFhnp29648670 = StDubpZPneTkWFhnp29967977;     StDubpZPneTkWFhnp29967977 = StDubpZPneTkWFhnp22017301;     StDubpZPneTkWFhnp22017301 = StDubpZPneTkWFhnp43395648;     StDubpZPneTkWFhnp43395648 = StDubpZPneTkWFhnp9406658;     StDubpZPneTkWFhnp9406658 = StDubpZPneTkWFhnp12817913;     StDubpZPneTkWFhnp12817913 = StDubpZPneTkWFhnp36270575;     StDubpZPneTkWFhnp36270575 = StDubpZPneTkWFhnp84770582;     StDubpZPneTkWFhnp84770582 = StDubpZPneTkWFhnp7483693;     StDubpZPneTkWFhnp7483693 = StDubpZPneTkWFhnp13122482;     StDubpZPneTkWFhnp13122482 = StDubpZPneTkWFhnp19136757;     StDubpZPneTkWFhnp19136757 = StDubpZPneTkWFhnp18236286;     StDubpZPneTkWFhnp18236286 = StDubpZPneTkWFhnp73261061;     StDubpZPneTkWFhnp73261061 = StDubpZPneTkWFhnp92879645;     StDubpZPneTkWFhnp92879645 = StDubpZPneTkWFhnp99512958;     StDubpZPneTkWFhnp99512958 = StDubpZPneTkWFhnp3783842;     StDubpZPneTkWFhnp3783842 = StDubpZPneTkWFhnp30728940;     StDubpZPneTkWFhnp30728940 = StDubpZPneTkWFhnp40822080;     StDubpZPneTkWFhnp40822080 = StDubpZPneTkWFhnp84809690;     StDubpZPneTkWFhnp84809690 = StDubpZPneTkWFhnp81598933;     StDubpZPneTkWFhnp81598933 = StDubpZPneTkWFhnp46621292;     StDubpZPneTkWFhnp46621292 = StDubpZPneTkWFhnp82787898;     StDubpZPneTkWFhnp82787898 = StDubpZPneTkWFhnp91303164;     StDubpZPneTkWFhnp91303164 = StDubpZPneTkWFhnp51664256;     StDubpZPneTkWFhnp51664256 = StDubpZPneTkWFhnp86092967;     StDubpZPneTkWFhnp86092967 = StDubpZPneTkWFhnp4151671;     StDubpZPneTkWFhnp4151671 = StDubpZPneTkWFhnp49598535;     StDubpZPneTkWFhnp49598535 = StDubpZPneTkWFhnp39968899;     StDubpZPneTkWFhnp39968899 = StDubpZPneTkWFhnp6708349;     StDubpZPneTkWFhnp6708349 = StDubpZPneTkWFhnp70794677;     StDubpZPneTkWFhnp70794677 = StDubpZPneTkWFhnp97376432;     StDubpZPneTkWFhnp97376432 = StDubpZPneTkWFhnp40468385;     StDubpZPneTkWFhnp40468385 = StDubpZPneTkWFhnp28480637;     StDubpZPneTkWFhnp28480637 = StDubpZPneTkWFhnp9643690;     StDubpZPneTkWFhnp9643690 = StDubpZPneTkWFhnp49426671;     StDubpZPneTkWFhnp49426671 = StDubpZPneTkWFhnp2471223;     StDubpZPneTkWFhnp2471223 = StDubpZPneTkWFhnp99890255;     StDubpZPneTkWFhnp99890255 = StDubpZPneTkWFhnp63322593;     StDubpZPneTkWFhnp63322593 = StDubpZPneTkWFhnp49211121;     StDubpZPneTkWFhnp49211121 = StDubpZPneTkWFhnp50968030;     StDubpZPneTkWFhnp50968030 = StDubpZPneTkWFhnp26251776;     StDubpZPneTkWFhnp26251776 = StDubpZPneTkWFhnp78399912;     StDubpZPneTkWFhnp78399912 = StDubpZPneTkWFhnp99981308;     StDubpZPneTkWFhnp99981308 = StDubpZPneTkWFhnp59578892;     StDubpZPneTkWFhnp59578892 = StDubpZPneTkWFhnp59377616;     StDubpZPneTkWFhnp59377616 = StDubpZPneTkWFhnp88931254;     StDubpZPneTkWFhnp88931254 = StDubpZPneTkWFhnp10958574;     StDubpZPneTkWFhnp10958574 = StDubpZPneTkWFhnp83464262;     StDubpZPneTkWFhnp83464262 = StDubpZPneTkWFhnp62638375;     StDubpZPneTkWFhnp62638375 = StDubpZPneTkWFhnp29236090;     StDubpZPneTkWFhnp29236090 = StDubpZPneTkWFhnp46685784;     StDubpZPneTkWFhnp46685784 = StDubpZPneTkWFhnp25878611;     StDubpZPneTkWFhnp25878611 = StDubpZPneTkWFhnp50131569;     StDubpZPneTkWFhnp50131569 = StDubpZPneTkWFhnp853161;     StDubpZPneTkWFhnp853161 = StDubpZPneTkWFhnp18639754;     StDubpZPneTkWFhnp18639754 = StDubpZPneTkWFhnp46860772;     StDubpZPneTkWFhnp46860772 = StDubpZPneTkWFhnp38664813;     StDubpZPneTkWFhnp38664813 = StDubpZPneTkWFhnp70353045;     StDubpZPneTkWFhnp70353045 = StDubpZPneTkWFhnp57302680;     StDubpZPneTkWFhnp57302680 = StDubpZPneTkWFhnp5254987;     StDubpZPneTkWFhnp5254987 = StDubpZPneTkWFhnp63219377;     StDubpZPneTkWFhnp63219377 = StDubpZPneTkWFhnp96301675;     StDubpZPneTkWFhnp96301675 = StDubpZPneTkWFhnp78062234;     StDubpZPneTkWFhnp78062234 = StDubpZPneTkWFhnp36689016;     StDubpZPneTkWFhnp36689016 = StDubpZPneTkWFhnp15746050;     StDubpZPneTkWFhnp15746050 = StDubpZPneTkWFhnp78668371;     StDubpZPneTkWFhnp78668371 = StDubpZPneTkWFhnp89755649;     StDubpZPneTkWFhnp89755649 = StDubpZPneTkWFhnp63617372;     StDubpZPneTkWFhnp63617372 = StDubpZPneTkWFhnp43452975;     StDubpZPneTkWFhnp43452975 = StDubpZPneTkWFhnp97041736;     StDubpZPneTkWFhnp97041736 = StDubpZPneTkWFhnp3893587;     StDubpZPneTkWFhnp3893587 = StDubpZPneTkWFhnp67406347;     StDubpZPneTkWFhnp67406347 = StDubpZPneTkWFhnp91610959;     StDubpZPneTkWFhnp91610959 = StDubpZPneTkWFhnp33841660;     StDubpZPneTkWFhnp33841660 = StDubpZPneTkWFhnp55347157;     StDubpZPneTkWFhnp55347157 = StDubpZPneTkWFhnp68221380;     StDubpZPneTkWFhnp68221380 = StDubpZPneTkWFhnp82806590;     StDubpZPneTkWFhnp82806590 = StDubpZPneTkWFhnp31724272;     StDubpZPneTkWFhnp31724272 = StDubpZPneTkWFhnp92286640;     StDubpZPneTkWFhnp92286640 = StDubpZPneTkWFhnp97161713;     StDubpZPneTkWFhnp97161713 = StDubpZPneTkWFhnp93193097;     StDubpZPneTkWFhnp93193097 = StDubpZPneTkWFhnp66134272;     StDubpZPneTkWFhnp66134272 = StDubpZPneTkWFhnp77330524;     StDubpZPneTkWFhnp77330524 = StDubpZPneTkWFhnp77472258;     StDubpZPneTkWFhnp77472258 = StDubpZPneTkWFhnp24108893;     StDubpZPneTkWFhnp24108893 = StDubpZPneTkWFhnp71497821;     StDubpZPneTkWFhnp71497821 = StDubpZPneTkWFhnp90336815;     StDubpZPneTkWFhnp90336815 = StDubpZPneTkWFhnp7465001;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RIxSatQwywyDshww78219562() {     double KFVULHodmUMHVSJsJ13582108 = -807999652;    double KFVULHodmUMHVSJsJ16074654 = -808556490;    double KFVULHodmUMHVSJsJ94556555 = -22608030;    double KFVULHodmUMHVSJsJ34242322 = -488878121;    double KFVULHodmUMHVSJsJ53115123 = -95740562;    double KFVULHodmUMHVSJsJ67761111 = -98277195;    double KFVULHodmUMHVSJsJ22640300 = -655447497;    double KFVULHodmUMHVSJsJ82779822 = -321638733;    double KFVULHodmUMHVSJsJ42057220 = -438581561;    double KFVULHodmUMHVSJsJ7930351 = -225261020;    double KFVULHodmUMHVSJsJ95051549 = -181222369;    double KFVULHodmUMHVSJsJ14275137 = -962244934;    double KFVULHodmUMHVSJsJ51470165 = -552089404;    double KFVULHodmUMHVSJsJ12792347 = -441306909;    double KFVULHodmUMHVSJsJ25710404 = -69708952;    double KFVULHodmUMHVSJsJ63522694 = -532875374;    double KFVULHodmUMHVSJsJ37231651 = -894982089;    double KFVULHodmUMHVSJsJ6459811 = -386755108;    double KFVULHodmUMHVSJsJ74647754 = -533695294;    double KFVULHodmUMHVSJsJ16820740 = -979958755;    double KFVULHodmUMHVSJsJ73323775 = -676298454;    double KFVULHodmUMHVSJsJ94004716 = -407363681;    double KFVULHodmUMHVSJsJ48713274 = -191915378;    double KFVULHodmUMHVSJsJ3129131 = -517708988;    double KFVULHodmUMHVSJsJ61935936 = -196850084;    double KFVULHodmUMHVSJsJ97982736 = -19924870;    double KFVULHodmUMHVSJsJ91339423 = -239111564;    double KFVULHodmUMHVSJsJ12620085 = -933248175;    double KFVULHodmUMHVSJsJ69916097 = -154446547;    double KFVULHodmUMHVSJsJ99344576 = -608788208;    double KFVULHodmUMHVSJsJ33064630 = -906831209;    double KFVULHodmUMHVSJsJ82420312 = -571065467;    double KFVULHodmUMHVSJsJ91309882 = -405882859;    double KFVULHodmUMHVSJsJ41451857 = -879894653;    double KFVULHodmUMHVSJsJ44262550 = -955323203;    double KFVULHodmUMHVSJsJ60029077 = -415278307;    double KFVULHodmUMHVSJsJ32926191 = -503878041;    double KFVULHodmUMHVSJsJ20183180 = -405917422;    double KFVULHodmUMHVSJsJ77761924 = -176003354;    double KFVULHodmUMHVSJsJ37100473 = -798188444;    double KFVULHodmUMHVSJsJ45638123 = -541070825;    double KFVULHodmUMHVSJsJ38858806 = -763119199;    double KFVULHodmUMHVSJsJ48861655 = -801436957;    double KFVULHodmUMHVSJsJ84784172 = -272622214;    double KFVULHodmUMHVSJsJ52420850 = -447240883;    double KFVULHodmUMHVSJsJ40419832 = -18410395;    double KFVULHodmUMHVSJsJ27422264 = -991424182;    double KFVULHodmUMHVSJsJ57383612 = 67951652;    double KFVULHodmUMHVSJsJ12146952 = -581366130;    double KFVULHodmUMHVSJsJ12801908 = -799657984;    double KFVULHodmUMHVSJsJ99023271 = -713220925;    double KFVULHodmUMHVSJsJ60020841 = -227299011;    double KFVULHodmUMHVSJsJ99216487 = -298665569;    double KFVULHodmUMHVSJsJ22220657 = -910938273;    double KFVULHodmUMHVSJsJ4895787 = -886015811;    double KFVULHodmUMHVSJsJ19577392 = -300635972;    double KFVULHodmUMHVSJsJ67361380 = -516641113;    double KFVULHodmUMHVSJsJ91427425 = -504899043;    double KFVULHodmUMHVSJsJ72306385 = -192028038;    double KFVULHodmUMHVSJsJ55132387 = 24184308;    double KFVULHodmUMHVSJsJ76421687 = -859165631;    double KFVULHodmUMHVSJsJ10020216 = -722199322;    double KFVULHodmUMHVSJsJ12863726 = -67192187;    double KFVULHodmUMHVSJsJ42712643 = -829793354;    double KFVULHodmUMHVSJsJ74865720 = -318429812;    double KFVULHodmUMHVSJsJ12631238 = -610156902;    double KFVULHodmUMHVSJsJ22965255 = -456362076;    double KFVULHodmUMHVSJsJ10018309 = -672194752;    double KFVULHodmUMHVSJsJ68529796 = -485983707;    double KFVULHodmUMHVSJsJ65681326 = -654430645;    double KFVULHodmUMHVSJsJ30596504 = 71002667;    double KFVULHodmUMHVSJsJ17048471 = -389064667;    double KFVULHodmUMHVSJsJ28697886 = -110751755;    double KFVULHodmUMHVSJsJ37547282 = -735506850;    double KFVULHodmUMHVSJsJ71182616 = -338887930;    double KFVULHodmUMHVSJsJ34464970 = -913179255;    double KFVULHodmUMHVSJsJ45143061 = -605926724;    double KFVULHodmUMHVSJsJ63929101 = -919293165;    double KFVULHodmUMHVSJsJ50708280 = 29531894;    double KFVULHodmUMHVSJsJ21516104 = -78439690;    double KFVULHodmUMHVSJsJ70560473 = -28500689;    double KFVULHodmUMHVSJsJ33955812 = -207063217;    double KFVULHodmUMHVSJsJ473133 = -251882046;    double KFVULHodmUMHVSJsJ57114190 = -354788563;    double KFVULHodmUMHVSJsJ321306 = -895567283;    double KFVULHodmUMHVSJsJ73043789 = -579532198;    double KFVULHodmUMHVSJsJ83203824 = -172399898;    double KFVULHodmUMHVSJsJ69089226 = -494944587;    double KFVULHodmUMHVSJsJ36556070 = -993878842;    double KFVULHodmUMHVSJsJ24685159 = -554687232;    double KFVULHodmUMHVSJsJ92667697 = -898637195;    double KFVULHodmUMHVSJsJ41498765 = -998978999;    double KFVULHodmUMHVSJsJ47876795 = -113889385;    double KFVULHodmUMHVSJsJ22629538 = -100187663;    double KFVULHodmUMHVSJsJ60678786 = -939022813;    double KFVULHodmUMHVSJsJ35617908 = -818871504;    double KFVULHodmUMHVSJsJ25995080 = -595927013;    double KFVULHodmUMHVSJsJ6149013 = -971643603;    double KFVULHodmUMHVSJsJ9918453 = -954192403;    double KFVULHodmUMHVSJsJ39789612 = -807999652;     KFVULHodmUMHVSJsJ13582108 = KFVULHodmUMHVSJsJ16074654;     KFVULHodmUMHVSJsJ16074654 = KFVULHodmUMHVSJsJ94556555;     KFVULHodmUMHVSJsJ94556555 = KFVULHodmUMHVSJsJ34242322;     KFVULHodmUMHVSJsJ34242322 = KFVULHodmUMHVSJsJ53115123;     KFVULHodmUMHVSJsJ53115123 = KFVULHodmUMHVSJsJ67761111;     KFVULHodmUMHVSJsJ67761111 = KFVULHodmUMHVSJsJ22640300;     KFVULHodmUMHVSJsJ22640300 = KFVULHodmUMHVSJsJ82779822;     KFVULHodmUMHVSJsJ82779822 = KFVULHodmUMHVSJsJ42057220;     KFVULHodmUMHVSJsJ42057220 = KFVULHodmUMHVSJsJ7930351;     KFVULHodmUMHVSJsJ7930351 = KFVULHodmUMHVSJsJ95051549;     KFVULHodmUMHVSJsJ95051549 = KFVULHodmUMHVSJsJ14275137;     KFVULHodmUMHVSJsJ14275137 = KFVULHodmUMHVSJsJ51470165;     KFVULHodmUMHVSJsJ51470165 = KFVULHodmUMHVSJsJ12792347;     KFVULHodmUMHVSJsJ12792347 = KFVULHodmUMHVSJsJ25710404;     KFVULHodmUMHVSJsJ25710404 = KFVULHodmUMHVSJsJ63522694;     KFVULHodmUMHVSJsJ63522694 = KFVULHodmUMHVSJsJ37231651;     KFVULHodmUMHVSJsJ37231651 = KFVULHodmUMHVSJsJ6459811;     KFVULHodmUMHVSJsJ6459811 = KFVULHodmUMHVSJsJ74647754;     KFVULHodmUMHVSJsJ74647754 = KFVULHodmUMHVSJsJ16820740;     KFVULHodmUMHVSJsJ16820740 = KFVULHodmUMHVSJsJ73323775;     KFVULHodmUMHVSJsJ73323775 = KFVULHodmUMHVSJsJ94004716;     KFVULHodmUMHVSJsJ94004716 = KFVULHodmUMHVSJsJ48713274;     KFVULHodmUMHVSJsJ48713274 = KFVULHodmUMHVSJsJ3129131;     KFVULHodmUMHVSJsJ3129131 = KFVULHodmUMHVSJsJ61935936;     KFVULHodmUMHVSJsJ61935936 = KFVULHodmUMHVSJsJ97982736;     KFVULHodmUMHVSJsJ97982736 = KFVULHodmUMHVSJsJ91339423;     KFVULHodmUMHVSJsJ91339423 = KFVULHodmUMHVSJsJ12620085;     KFVULHodmUMHVSJsJ12620085 = KFVULHodmUMHVSJsJ69916097;     KFVULHodmUMHVSJsJ69916097 = KFVULHodmUMHVSJsJ99344576;     KFVULHodmUMHVSJsJ99344576 = KFVULHodmUMHVSJsJ33064630;     KFVULHodmUMHVSJsJ33064630 = KFVULHodmUMHVSJsJ82420312;     KFVULHodmUMHVSJsJ82420312 = KFVULHodmUMHVSJsJ91309882;     KFVULHodmUMHVSJsJ91309882 = KFVULHodmUMHVSJsJ41451857;     KFVULHodmUMHVSJsJ41451857 = KFVULHodmUMHVSJsJ44262550;     KFVULHodmUMHVSJsJ44262550 = KFVULHodmUMHVSJsJ60029077;     KFVULHodmUMHVSJsJ60029077 = KFVULHodmUMHVSJsJ32926191;     KFVULHodmUMHVSJsJ32926191 = KFVULHodmUMHVSJsJ20183180;     KFVULHodmUMHVSJsJ20183180 = KFVULHodmUMHVSJsJ77761924;     KFVULHodmUMHVSJsJ77761924 = KFVULHodmUMHVSJsJ37100473;     KFVULHodmUMHVSJsJ37100473 = KFVULHodmUMHVSJsJ45638123;     KFVULHodmUMHVSJsJ45638123 = KFVULHodmUMHVSJsJ38858806;     KFVULHodmUMHVSJsJ38858806 = KFVULHodmUMHVSJsJ48861655;     KFVULHodmUMHVSJsJ48861655 = KFVULHodmUMHVSJsJ84784172;     KFVULHodmUMHVSJsJ84784172 = KFVULHodmUMHVSJsJ52420850;     KFVULHodmUMHVSJsJ52420850 = KFVULHodmUMHVSJsJ40419832;     KFVULHodmUMHVSJsJ40419832 = KFVULHodmUMHVSJsJ27422264;     KFVULHodmUMHVSJsJ27422264 = KFVULHodmUMHVSJsJ57383612;     KFVULHodmUMHVSJsJ57383612 = KFVULHodmUMHVSJsJ12146952;     KFVULHodmUMHVSJsJ12146952 = KFVULHodmUMHVSJsJ12801908;     KFVULHodmUMHVSJsJ12801908 = KFVULHodmUMHVSJsJ99023271;     KFVULHodmUMHVSJsJ99023271 = KFVULHodmUMHVSJsJ60020841;     KFVULHodmUMHVSJsJ60020841 = KFVULHodmUMHVSJsJ99216487;     KFVULHodmUMHVSJsJ99216487 = KFVULHodmUMHVSJsJ22220657;     KFVULHodmUMHVSJsJ22220657 = KFVULHodmUMHVSJsJ4895787;     KFVULHodmUMHVSJsJ4895787 = KFVULHodmUMHVSJsJ19577392;     KFVULHodmUMHVSJsJ19577392 = KFVULHodmUMHVSJsJ67361380;     KFVULHodmUMHVSJsJ67361380 = KFVULHodmUMHVSJsJ91427425;     KFVULHodmUMHVSJsJ91427425 = KFVULHodmUMHVSJsJ72306385;     KFVULHodmUMHVSJsJ72306385 = KFVULHodmUMHVSJsJ55132387;     KFVULHodmUMHVSJsJ55132387 = KFVULHodmUMHVSJsJ76421687;     KFVULHodmUMHVSJsJ76421687 = KFVULHodmUMHVSJsJ10020216;     KFVULHodmUMHVSJsJ10020216 = KFVULHodmUMHVSJsJ12863726;     KFVULHodmUMHVSJsJ12863726 = KFVULHodmUMHVSJsJ42712643;     KFVULHodmUMHVSJsJ42712643 = KFVULHodmUMHVSJsJ74865720;     KFVULHodmUMHVSJsJ74865720 = KFVULHodmUMHVSJsJ12631238;     KFVULHodmUMHVSJsJ12631238 = KFVULHodmUMHVSJsJ22965255;     KFVULHodmUMHVSJsJ22965255 = KFVULHodmUMHVSJsJ10018309;     KFVULHodmUMHVSJsJ10018309 = KFVULHodmUMHVSJsJ68529796;     KFVULHodmUMHVSJsJ68529796 = KFVULHodmUMHVSJsJ65681326;     KFVULHodmUMHVSJsJ65681326 = KFVULHodmUMHVSJsJ30596504;     KFVULHodmUMHVSJsJ30596504 = KFVULHodmUMHVSJsJ17048471;     KFVULHodmUMHVSJsJ17048471 = KFVULHodmUMHVSJsJ28697886;     KFVULHodmUMHVSJsJ28697886 = KFVULHodmUMHVSJsJ37547282;     KFVULHodmUMHVSJsJ37547282 = KFVULHodmUMHVSJsJ71182616;     KFVULHodmUMHVSJsJ71182616 = KFVULHodmUMHVSJsJ34464970;     KFVULHodmUMHVSJsJ34464970 = KFVULHodmUMHVSJsJ45143061;     KFVULHodmUMHVSJsJ45143061 = KFVULHodmUMHVSJsJ63929101;     KFVULHodmUMHVSJsJ63929101 = KFVULHodmUMHVSJsJ50708280;     KFVULHodmUMHVSJsJ50708280 = KFVULHodmUMHVSJsJ21516104;     KFVULHodmUMHVSJsJ21516104 = KFVULHodmUMHVSJsJ70560473;     KFVULHodmUMHVSJsJ70560473 = KFVULHodmUMHVSJsJ33955812;     KFVULHodmUMHVSJsJ33955812 = KFVULHodmUMHVSJsJ473133;     KFVULHodmUMHVSJsJ473133 = KFVULHodmUMHVSJsJ57114190;     KFVULHodmUMHVSJsJ57114190 = KFVULHodmUMHVSJsJ321306;     KFVULHodmUMHVSJsJ321306 = KFVULHodmUMHVSJsJ73043789;     KFVULHodmUMHVSJsJ73043789 = KFVULHodmUMHVSJsJ83203824;     KFVULHodmUMHVSJsJ83203824 = KFVULHodmUMHVSJsJ69089226;     KFVULHodmUMHVSJsJ69089226 = KFVULHodmUMHVSJsJ36556070;     KFVULHodmUMHVSJsJ36556070 = KFVULHodmUMHVSJsJ24685159;     KFVULHodmUMHVSJsJ24685159 = KFVULHodmUMHVSJsJ92667697;     KFVULHodmUMHVSJsJ92667697 = KFVULHodmUMHVSJsJ41498765;     KFVULHodmUMHVSJsJ41498765 = KFVULHodmUMHVSJsJ47876795;     KFVULHodmUMHVSJsJ47876795 = KFVULHodmUMHVSJsJ22629538;     KFVULHodmUMHVSJsJ22629538 = KFVULHodmUMHVSJsJ60678786;     KFVULHodmUMHVSJsJ60678786 = KFVULHodmUMHVSJsJ35617908;     KFVULHodmUMHVSJsJ35617908 = KFVULHodmUMHVSJsJ25995080;     KFVULHodmUMHVSJsJ25995080 = KFVULHodmUMHVSJsJ6149013;     KFVULHodmUMHVSJsJ6149013 = KFVULHodmUMHVSJsJ9918453;     KFVULHodmUMHVSJsJ9918453 = KFVULHodmUMHVSJsJ39789612;     KFVULHodmUMHVSJsJ39789612 = KFVULHodmUMHVSJsJ13582108;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void yXNUWZZjaqjkDEVc45511162() {     double BkEGreZVETAzKedsB49041076 = -174276552;    double BkEGreZVETAzKedsB29346733 = -689108097;    double BkEGreZVETAzKedsB31943033 = -788275105;    double BkEGreZVETAzKedsB6465677 = -310029192;    double BkEGreZVETAzKedsB9400393 = -201339407;    double BkEGreZVETAzKedsB3983983 = -360625091;    double BkEGreZVETAzKedsB33297706 = -500173231;    double BkEGreZVETAzKedsB44911384 = 34127127;    double BkEGreZVETAzKedsB43574825 = -926912968;    double BkEGreZVETAzKedsB22833063 = -989948018;    double BkEGreZVETAzKedsB33233285 = 16769902;    double BkEGreZVETAzKedsB14843286 = -826365706;    double BkEGreZVETAzKedsB89704810 = -4101909;    double BkEGreZVETAzKedsB65933210 = -417670319;    double BkEGreZVETAzKedsB83904920 = -573133967;    double BkEGreZVETAzKedsB47063995 = -168686981;    double BkEGreZVETAzKedsB93067586 = -321956641;    double BkEGreZVETAzKedsB6220003 = -460183177;    double BkEGreZVETAzKedsB75198084 = -850258715;    double BkEGreZVETAzKedsB10657959 = -306519821;    double BkEGreZVETAzKedsB86410638 = -333573143;    double BkEGreZVETAzKedsB97426240 = -493999975;    double BkEGreZVETAzKedsB19238654 = -929547562;    double BkEGreZVETAzKedsB96922075 = 71466408;    double BkEGreZVETAzKedsB69953887 = -220835282;    double BkEGreZVETAzKedsB8680909 = -808501677;    double BkEGreZVETAzKedsB51759998 = -45395705;    double BkEGreZVETAzKedsB53348509 = -3020349;    double BkEGreZVETAzKedsB16584312 = -728541349;    double BkEGreZVETAzKedsB4877988 = -643295378;    double BkEGreZVETAzKedsB51647861 = -358811696;    double BkEGreZVETAzKedsB20243331 = -344036881;    double BkEGreZVETAzKedsB3527893 = -553478836;    double BkEGreZVETAzKedsB71200283 = -944588466;    double BkEGreZVETAzKedsB10071004 = -906423454;    double BkEGreZVETAzKedsB11589922 = -629843613;    double BkEGreZVETAzKedsB82338007 = -368574081;    double BkEGreZVETAzKedsB71398583 = -388573004;    double BkEGreZVETAzKedsB8152487 = -926879630;    double BkEGreZVETAzKedsB1372005 = -263450990;    double BkEGreZVETAzKedsB79089731 = -310269739;    double BkEGreZVETAzKedsB28223932 = -357664490;    double BkEGreZVETAzKedsB38245034 = -354617743;    double BkEGreZVETAzKedsB24915511 = -876954624;    double BkEGreZVETAzKedsB52437309 = -273752044;    double BkEGreZVETAzKedsB86381160 = -57334027;    double BkEGreZVETAzKedsB46767427 = -375410542;    double BkEGreZVETAzKedsB75109924 = -98574941;    double BkEGreZVETAzKedsB35039618 = -245103010;    double BkEGreZVETAzKedsB37331497 = -190684800;    double BkEGreZVETAzKedsB79793995 = -587497483;    double BkEGreZVETAzKedsB43506313 = -440951358;    double BkEGreZVETAzKedsB67837636 = -54711123;    double BkEGreZVETAzKedsB38212503 = -2446773;    double BkEGreZVETAzKedsB59100523 = -132505957;    double BkEGreZVETAzKedsB51614835 = -680276577;    double BkEGreZVETAzKedsB10108079 = -759560535;    double BkEGreZVETAzKedsB35020957 = -759741513;    double BkEGreZVETAzKedsB36511789 = 10806089;    double BkEGreZVETAzKedsB719485 = -392837730;    double BkEGreZVETAzKedsB52223985 = -215229386;    double BkEGreZVETAzKedsB79949196 = -397152882;    double BkEGreZVETAzKedsB28327072 = -237331525;    double BkEGreZVETAzKedsB38696837 = -183617590;    double BkEGreZVETAzKedsB71185201 = -531136323;    double BkEGreZVETAzKedsB12989955 = -639193217;    double BkEGreZVETAzKedsB11315393 = -172886870;    double BkEGreZVETAzKedsB18504527 = -59513443;    double BkEGreZVETAzKedsB55862206 = -511246865;    double BkEGreZVETAzKedsB72314999 = -943290354;    double BkEGreZVETAzKedsB64725988 = -800112900;    double BkEGreZVETAzKedsB21669004 = -933383638;    double BkEGreZVETAzKedsB98067515 = -533303547;    double BkEGreZVETAzKedsB73826079 = -486807726;    double BkEGreZVETAzKedsB31568227 = -996250082;    double BkEGreZVETAzKedsB58186706 = -975908653;    double BkEGreZVETAzKedsB59181207 = -39382232;    double BkEGreZVETAzKedsB94323142 = 47407061;    double BkEGreZVETAzKedsB44484767 = -654781549;    double BkEGreZVETAzKedsB83572727 = -63501256;    double BkEGreZVETAzKedsB61913481 = -333091136;    double BkEGreZVETAzKedsB76650074 = -946820764;    double BkEGreZVETAzKedsB18308892 = -757917340;    double BkEGreZVETAzKedsB79252815 = -437856550;    double BkEGreZVETAzKedsB25083993 = 44202104;    double BkEGreZVETAzKedsB8141549 = -917860338;    double BkEGreZVETAzKedsB52405694 = -189325759;    double BkEGreZVETAzKedsB65315390 = -451032064;    double BkEGreZVETAzKedsB12099760 = -712082510;    double BkEGreZVETAzKedsB58456169 = -126146877;    double BkEGreZVETAzKedsB1481843 = -870283078;    double BkEGreZVETAzKedsB47317050 = -608832569;    double BkEGreZVETAzKedsB34886795 = -299379094;    double BkEGreZVETAzKedsB7433003 = -434041900;    double BkEGreZVETAzKedsB49148020 = 51778395;    double BkEGreZVETAzKedsB99140534 = -913116858;    double BkEGreZVETAzKedsB99896860 = -20332966;    double BkEGreZVETAzKedsB99548196 = -71000153;    double BkEGreZVETAzKedsB53730310 = -245818302;    double BkEGreZVETAzKedsB39447355 = -174276552;     BkEGreZVETAzKedsB49041076 = BkEGreZVETAzKedsB29346733;     BkEGreZVETAzKedsB29346733 = BkEGreZVETAzKedsB31943033;     BkEGreZVETAzKedsB31943033 = BkEGreZVETAzKedsB6465677;     BkEGreZVETAzKedsB6465677 = BkEGreZVETAzKedsB9400393;     BkEGreZVETAzKedsB9400393 = BkEGreZVETAzKedsB3983983;     BkEGreZVETAzKedsB3983983 = BkEGreZVETAzKedsB33297706;     BkEGreZVETAzKedsB33297706 = BkEGreZVETAzKedsB44911384;     BkEGreZVETAzKedsB44911384 = BkEGreZVETAzKedsB43574825;     BkEGreZVETAzKedsB43574825 = BkEGreZVETAzKedsB22833063;     BkEGreZVETAzKedsB22833063 = BkEGreZVETAzKedsB33233285;     BkEGreZVETAzKedsB33233285 = BkEGreZVETAzKedsB14843286;     BkEGreZVETAzKedsB14843286 = BkEGreZVETAzKedsB89704810;     BkEGreZVETAzKedsB89704810 = BkEGreZVETAzKedsB65933210;     BkEGreZVETAzKedsB65933210 = BkEGreZVETAzKedsB83904920;     BkEGreZVETAzKedsB83904920 = BkEGreZVETAzKedsB47063995;     BkEGreZVETAzKedsB47063995 = BkEGreZVETAzKedsB93067586;     BkEGreZVETAzKedsB93067586 = BkEGreZVETAzKedsB6220003;     BkEGreZVETAzKedsB6220003 = BkEGreZVETAzKedsB75198084;     BkEGreZVETAzKedsB75198084 = BkEGreZVETAzKedsB10657959;     BkEGreZVETAzKedsB10657959 = BkEGreZVETAzKedsB86410638;     BkEGreZVETAzKedsB86410638 = BkEGreZVETAzKedsB97426240;     BkEGreZVETAzKedsB97426240 = BkEGreZVETAzKedsB19238654;     BkEGreZVETAzKedsB19238654 = BkEGreZVETAzKedsB96922075;     BkEGreZVETAzKedsB96922075 = BkEGreZVETAzKedsB69953887;     BkEGreZVETAzKedsB69953887 = BkEGreZVETAzKedsB8680909;     BkEGreZVETAzKedsB8680909 = BkEGreZVETAzKedsB51759998;     BkEGreZVETAzKedsB51759998 = BkEGreZVETAzKedsB53348509;     BkEGreZVETAzKedsB53348509 = BkEGreZVETAzKedsB16584312;     BkEGreZVETAzKedsB16584312 = BkEGreZVETAzKedsB4877988;     BkEGreZVETAzKedsB4877988 = BkEGreZVETAzKedsB51647861;     BkEGreZVETAzKedsB51647861 = BkEGreZVETAzKedsB20243331;     BkEGreZVETAzKedsB20243331 = BkEGreZVETAzKedsB3527893;     BkEGreZVETAzKedsB3527893 = BkEGreZVETAzKedsB71200283;     BkEGreZVETAzKedsB71200283 = BkEGreZVETAzKedsB10071004;     BkEGreZVETAzKedsB10071004 = BkEGreZVETAzKedsB11589922;     BkEGreZVETAzKedsB11589922 = BkEGreZVETAzKedsB82338007;     BkEGreZVETAzKedsB82338007 = BkEGreZVETAzKedsB71398583;     BkEGreZVETAzKedsB71398583 = BkEGreZVETAzKedsB8152487;     BkEGreZVETAzKedsB8152487 = BkEGreZVETAzKedsB1372005;     BkEGreZVETAzKedsB1372005 = BkEGreZVETAzKedsB79089731;     BkEGreZVETAzKedsB79089731 = BkEGreZVETAzKedsB28223932;     BkEGreZVETAzKedsB28223932 = BkEGreZVETAzKedsB38245034;     BkEGreZVETAzKedsB38245034 = BkEGreZVETAzKedsB24915511;     BkEGreZVETAzKedsB24915511 = BkEGreZVETAzKedsB52437309;     BkEGreZVETAzKedsB52437309 = BkEGreZVETAzKedsB86381160;     BkEGreZVETAzKedsB86381160 = BkEGreZVETAzKedsB46767427;     BkEGreZVETAzKedsB46767427 = BkEGreZVETAzKedsB75109924;     BkEGreZVETAzKedsB75109924 = BkEGreZVETAzKedsB35039618;     BkEGreZVETAzKedsB35039618 = BkEGreZVETAzKedsB37331497;     BkEGreZVETAzKedsB37331497 = BkEGreZVETAzKedsB79793995;     BkEGreZVETAzKedsB79793995 = BkEGreZVETAzKedsB43506313;     BkEGreZVETAzKedsB43506313 = BkEGreZVETAzKedsB67837636;     BkEGreZVETAzKedsB67837636 = BkEGreZVETAzKedsB38212503;     BkEGreZVETAzKedsB38212503 = BkEGreZVETAzKedsB59100523;     BkEGreZVETAzKedsB59100523 = BkEGreZVETAzKedsB51614835;     BkEGreZVETAzKedsB51614835 = BkEGreZVETAzKedsB10108079;     BkEGreZVETAzKedsB10108079 = BkEGreZVETAzKedsB35020957;     BkEGreZVETAzKedsB35020957 = BkEGreZVETAzKedsB36511789;     BkEGreZVETAzKedsB36511789 = BkEGreZVETAzKedsB719485;     BkEGreZVETAzKedsB719485 = BkEGreZVETAzKedsB52223985;     BkEGreZVETAzKedsB52223985 = BkEGreZVETAzKedsB79949196;     BkEGreZVETAzKedsB79949196 = BkEGreZVETAzKedsB28327072;     BkEGreZVETAzKedsB28327072 = BkEGreZVETAzKedsB38696837;     BkEGreZVETAzKedsB38696837 = BkEGreZVETAzKedsB71185201;     BkEGreZVETAzKedsB71185201 = BkEGreZVETAzKedsB12989955;     BkEGreZVETAzKedsB12989955 = BkEGreZVETAzKedsB11315393;     BkEGreZVETAzKedsB11315393 = BkEGreZVETAzKedsB18504527;     BkEGreZVETAzKedsB18504527 = BkEGreZVETAzKedsB55862206;     BkEGreZVETAzKedsB55862206 = BkEGreZVETAzKedsB72314999;     BkEGreZVETAzKedsB72314999 = BkEGreZVETAzKedsB64725988;     BkEGreZVETAzKedsB64725988 = BkEGreZVETAzKedsB21669004;     BkEGreZVETAzKedsB21669004 = BkEGreZVETAzKedsB98067515;     BkEGreZVETAzKedsB98067515 = BkEGreZVETAzKedsB73826079;     BkEGreZVETAzKedsB73826079 = BkEGreZVETAzKedsB31568227;     BkEGreZVETAzKedsB31568227 = BkEGreZVETAzKedsB58186706;     BkEGreZVETAzKedsB58186706 = BkEGreZVETAzKedsB59181207;     BkEGreZVETAzKedsB59181207 = BkEGreZVETAzKedsB94323142;     BkEGreZVETAzKedsB94323142 = BkEGreZVETAzKedsB44484767;     BkEGreZVETAzKedsB44484767 = BkEGreZVETAzKedsB83572727;     BkEGreZVETAzKedsB83572727 = BkEGreZVETAzKedsB61913481;     BkEGreZVETAzKedsB61913481 = BkEGreZVETAzKedsB76650074;     BkEGreZVETAzKedsB76650074 = BkEGreZVETAzKedsB18308892;     BkEGreZVETAzKedsB18308892 = BkEGreZVETAzKedsB79252815;     BkEGreZVETAzKedsB79252815 = BkEGreZVETAzKedsB25083993;     BkEGreZVETAzKedsB25083993 = BkEGreZVETAzKedsB8141549;     BkEGreZVETAzKedsB8141549 = BkEGreZVETAzKedsB52405694;     BkEGreZVETAzKedsB52405694 = BkEGreZVETAzKedsB65315390;     BkEGreZVETAzKedsB65315390 = BkEGreZVETAzKedsB12099760;     BkEGreZVETAzKedsB12099760 = BkEGreZVETAzKedsB58456169;     BkEGreZVETAzKedsB58456169 = BkEGreZVETAzKedsB1481843;     BkEGreZVETAzKedsB1481843 = BkEGreZVETAzKedsB47317050;     BkEGreZVETAzKedsB47317050 = BkEGreZVETAzKedsB34886795;     BkEGreZVETAzKedsB34886795 = BkEGreZVETAzKedsB7433003;     BkEGreZVETAzKedsB7433003 = BkEGreZVETAzKedsB49148020;     BkEGreZVETAzKedsB49148020 = BkEGreZVETAzKedsB99140534;     BkEGreZVETAzKedsB99140534 = BkEGreZVETAzKedsB99896860;     BkEGreZVETAzKedsB99896860 = BkEGreZVETAzKedsB99548196;     BkEGreZVETAzKedsB99548196 = BkEGreZVETAzKedsB53730310;     BkEGreZVETAzKedsB53730310 = BkEGreZVETAzKedsB39447355;     BkEGreZVETAzKedsB39447355 = BkEGreZVETAzKedsB49041076;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void eoMtFAaYipokxoFt82156136() {     double MlmShbidGaGOMVfoh71428193 = -908680515;    double MlmShbidGaGOMVfoh13694233 = -637651151;    double MlmShbidGaGOMVfoh60712311 = 72071051;    double MlmShbidGaGOMVfoh21472365 = -316436724;    double MlmShbidGaGOMVfoh7574647 = -792396408;    double MlmShbidGaGOMVfoh18867100 = -221233468;    double MlmShbidGaGOMVfoh25453445 = -322160537;    double MlmShbidGaGOMVfoh20545243 = -436469518;    double MlmShbidGaGOMVfoh40737956 = -811109007;    double MlmShbidGaGOMVfoh14596647 = -732689351;    double MlmShbidGaGOMVfoh70775144 = 20145501;    double MlmShbidGaGOMVfoh70746234 = 22889001;    double MlmShbidGaGOMVfoh56712991 = -622819229;    double MlmShbidGaGOMVfoh8547963 = -945379076;    double MlmShbidGaGOMVfoh20640197 = -200611680;    double MlmShbidGaGOMVfoh91319908 = -550354524;    double MlmShbidGaGOMVfoh42890939 = -599236461;    double MlmShbidGaGOMVfoh12196419 = -911925867;    double MlmShbidGaGOMVfoh16479715 = -985360841;    double MlmShbidGaGOMVfoh89174241 = -559154275;    double MlmShbidGaGOMVfoh87293745 = -325060451;    double MlmShbidGaGOMVfoh61241459 = -998766288;    double MlmShbidGaGOMVfoh22531977 = -5628154;    double MlmShbidGaGOMVfoh23421863 = -492526450;    double MlmShbidGaGOMVfoh5494135 = -942394407;    double MlmShbidGaGOMVfoh96983903 = -152395901;    double MlmShbidGaGOMVfoh98849418 = -902982198;    double MlmShbidGaGOMVfoh59259959 = -891474803;    double MlmShbidGaGOMVfoh23476873 = 27201532;    double MlmShbidGaGOMVfoh14035168 = -563591245;    double MlmShbidGaGOMVfoh95938666 = -440127848;    double MlmShbidGaGOMVfoh77784521 = -996108236;    double MlmShbidGaGOMVfoh84566563 = 17177669;    double MlmShbidGaGOMVfoh30588970 = -168715316;    double MlmShbidGaGOMVfoh86300368 = -970179876;    double MlmShbidGaGOMVfoh53282790 = -945238990;    double MlmShbidGaGOMVfoh75777719 = -101522033;    double MlmShbidGaGOMVfoh25024685 = -256198476;    double MlmShbidGaGOMVfoh56460618 = -227925662;    double MlmShbidGaGOMVfoh21557720 = -399675020;    double MlmShbidGaGOMVfoh7847102 = -813447406;    double MlmShbidGaGOMVfoh16732324 = -115573181;    double MlmShbidGaGOMVfoh43795226 = -242148326;    double MlmShbidGaGOMVfoh24359630 = -874318273;    double MlmShbidGaGOMVfoh20420576 = -82895522;    double MlmShbidGaGOMVfoh94481442 = -239563122;    double MlmShbidGaGOMVfoh59064825 = -953820795;    double MlmShbidGaGOMVfoh19708599 = -955109932;    double MlmShbidGaGOMVfoh29499523 = -194819708;    double MlmShbidGaGOMVfoh34415243 = -332705059;    double MlmShbidGaGOMVfoh88663211 = -942573760;    double MlmShbidGaGOMVfoh17357571 = -871770640;    double MlmShbidGaGOMVfoh46152039 = -826622353;    double MlmShbidGaGOMVfoh98499048 = -278127581;    double MlmShbidGaGOMVfoh94862390 = -273210073;    double MlmShbidGaGOMVfoh10186734 = -909914227;    double MlmShbidGaGOMVfoh91162256 = -532022998;    double MlmShbidGaGOMVfoh37290449 = -435402500;    double MlmShbidGaGOMVfoh15978231 = -374042318;    double MlmShbidGaGOMVfoh10590744 = -540000508;    double MlmShbidGaGOMVfoh20017682 = -318251271;    double MlmShbidGaGOMVfoh66193485 = -430685735;    double MlmShbidGaGOMVfoh97068369 = -363671050;    double MlmShbidGaGOMVfoh26702789 = -147517763;    double MlmShbidGaGOMVfoh18657981 = -192561503;    double MlmShbidGaGOMVfoh92990622 = 16253737;    double MlmShbidGaGOMVfoh86179671 = -994288668;    double MlmShbidGaGOMVfoh26124021 = -354103913;    double MlmShbidGaGOMVfoh22247594 = -975199200;    double MlmShbidGaGOMVfoh67357406 = -255372691;    double MlmShbidGaGOMVfoh15542189 = -348832491;    double MlmShbidGaGOMVfoh17866255 = -243037986;    double MlmShbidGaGOMVfoh55735801 = -584000206;    double MlmShbidGaGOMVfoh94921995 = -485685821;    double MlmShbidGaGOMVfoh81327139 = -745706869;    double MlmShbidGaGOMVfoh70561422 = -109487271;    double MlmShbidGaGOMVfoh17446234 = -656617962;    double MlmShbidGaGOMVfoh98172346 = -131309882;    double MlmShbidGaGOMVfoh3001288 = -309630928;    double MlmShbidGaGOMVfoh11012692 = -602831285;    double MlmShbidGaGOMVfoh37919078 = -198575107;    double MlmShbidGaGOMVfoh79140820 = -947872266;    double MlmShbidGaGOMVfoh29760437 = -596655095;    double MlmShbidGaGOMVfoh89061630 = -640093410;    double MlmShbidGaGOMVfoh25371956 = -621017486;    double MlmShbidGaGOMVfoh78581096 = -568357208;    double MlmShbidGaGOMVfoh31632482 = -69485883;    double MlmShbidGaGOMVfoh86067515 = -704694750;    double MlmShbidGaGOMVfoh35726580 = -895505244;    double MlmShbidGaGOMVfoh76113634 = 39734351;    double MlmShbidGaGOMVfoh62120534 = -313215992;    double MlmShbidGaGOMVfoh38487271 = -666119533;    double MlmShbidGaGOMVfoh9046454 = -882156158;    double MlmShbidGaGOMVfoh45869874 = -687925155;    double MlmShbidGaGOMVfoh1540039 = 18576251;    double MlmShbidGaGOMVfoh41653616 = -282761671;    double MlmShbidGaGOMVfoh19663954 = -751902132;    double MlmShbidGaGOMVfoh17092437 = 5369436;    double MlmShbidGaGOMVfoh5701650 = -581756770;    double MlmShbidGaGOMVfoh27429953 = -908680515;     MlmShbidGaGOMVfoh71428193 = MlmShbidGaGOMVfoh13694233;     MlmShbidGaGOMVfoh13694233 = MlmShbidGaGOMVfoh60712311;     MlmShbidGaGOMVfoh60712311 = MlmShbidGaGOMVfoh21472365;     MlmShbidGaGOMVfoh21472365 = MlmShbidGaGOMVfoh7574647;     MlmShbidGaGOMVfoh7574647 = MlmShbidGaGOMVfoh18867100;     MlmShbidGaGOMVfoh18867100 = MlmShbidGaGOMVfoh25453445;     MlmShbidGaGOMVfoh25453445 = MlmShbidGaGOMVfoh20545243;     MlmShbidGaGOMVfoh20545243 = MlmShbidGaGOMVfoh40737956;     MlmShbidGaGOMVfoh40737956 = MlmShbidGaGOMVfoh14596647;     MlmShbidGaGOMVfoh14596647 = MlmShbidGaGOMVfoh70775144;     MlmShbidGaGOMVfoh70775144 = MlmShbidGaGOMVfoh70746234;     MlmShbidGaGOMVfoh70746234 = MlmShbidGaGOMVfoh56712991;     MlmShbidGaGOMVfoh56712991 = MlmShbidGaGOMVfoh8547963;     MlmShbidGaGOMVfoh8547963 = MlmShbidGaGOMVfoh20640197;     MlmShbidGaGOMVfoh20640197 = MlmShbidGaGOMVfoh91319908;     MlmShbidGaGOMVfoh91319908 = MlmShbidGaGOMVfoh42890939;     MlmShbidGaGOMVfoh42890939 = MlmShbidGaGOMVfoh12196419;     MlmShbidGaGOMVfoh12196419 = MlmShbidGaGOMVfoh16479715;     MlmShbidGaGOMVfoh16479715 = MlmShbidGaGOMVfoh89174241;     MlmShbidGaGOMVfoh89174241 = MlmShbidGaGOMVfoh87293745;     MlmShbidGaGOMVfoh87293745 = MlmShbidGaGOMVfoh61241459;     MlmShbidGaGOMVfoh61241459 = MlmShbidGaGOMVfoh22531977;     MlmShbidGaGOMVfoh22531977 = MlmShbidGaGOMVfoh23421863;     MlmShbidGaGOMVfoh23421863 = MlmShbidGaGOMVfoh5494135;     MlmShbidGaGOMVfoh5494135 = MlmShbidGaGOMVfoh96983903;     MlmShbidGaGOMVfoh96983903 = MlmShbidGaGOMVfoh98849418;     MlmShbidGaGOMVfoh98849418 = MlmShbidGaGOMVfoh59259959;     MlmShbidGaGOMVfoh59259959 = MlmShbidGaGOMVfoh23476873;     MlmShbidGaGOMVfoh23476873 = MlmShbidGaGOMVfoh14035168;     MlmShbidGaGOMVfoh14035168 = MlmShbidGaGOMVfoh95938666;     MlmShbidGaGOMVfoh95938666 = MlmShbidGaGOMVfoh77784521;     MlmShbidGaGOMVfoh77784521 = MlmShbidGaGOMVfoh84566563;     MlmShbidGaGOMVfoh84566563 = MlmShbidGaGOMVfoh30588970;     MlmShbidGaGOMVfoh30588970 = MlmShbidGaGOMVfoh86300368;     MlmShbidGaGOMVfoh86300368 = MlmShbidGaGOMVfoh53282790;     MlmShbidGaGOMVfoh53282790 = MlmShbidGaGOMVfoh75777719;     MlmShbidGaGOMVfoh75777719 = MlmShbidGaGOMVfoh25024685;     MlmShbidGaGOMVfoh25024685 = MlmShbidGaGOMVfoh56460618;     MlmShbidGaGOMVfoh56460618 = MlmShbidGaGOMVfoh21557720;     MlmShbidGaGOMVfoh21557720 = MlmShbidGaGOMVfoh7847102;     MlmShbidGaGOMVfoh7847102 = MlmShbidGaGOMVfoh16732324;     MlmShbidGaGOMVfoh16732324 = MlmShbidGaGOMVfoh43795226;     MlmShbidGaGOMVfoh43795226 = MlmShbidGaGOMVfoh24359630;     MlmShbidGaGOMVfoh24359630 = MlmShbidGaGOMVfoh20420576;     MlmShbidGaGOMVfoh20420576 = MlmShbidGaGOMVfoh94481442;     MlmShbidGaGOMVfoh94481442 = MlmShbidGaGOMVfoh59064825;     MlmShbidGaGOMVfoh59064825 = MlmShbidGaGOMVfoh19708599;     MlmShbidGaGOMVfoh19708599 = MlmShbidGaGOMVfoh29499523;     MlmShbidGaGOMVfoh29499523 = MlmShbidGaGOMVfoh34415243;     MlmShbidGaGOMVfoh34415243 = MlmShbidGaGOMVfoh88663211;     MlmShbidGaGOMVfoh88663211 = MlmShbidGaGOMVfoh17357571;     MlmShbidGaGOMVfoh17357571 = MlmShbidGaGOMVfoh46152039;     MlmShbidGaGOMVfoh46152039 = MlmShbidGaGOMVfoh98499048;     MlmShbidGaGOMVfoh98499048 = MlmShbidGaGOMVfoh94862390;     MlmShbidGaGOMVfoh94862390 = MlmShbidGaGOMVfoh10186734;     MlmShbidGaGOMVfoh10186734 = MlmShbidGaGOMVfoh91162256;     MlmShbidGaGOMVfoh91162256 = MlmShbidGaGOMVfoh37290449;     MlmShbidGaGOMVfoh37290449 = MlmShbidGaGOMVfoh15978231;     MlmShbidGaGOMVfoh15978231 = MlmShbidGaGOMVfoh10590744;     MlmShbidGaGOMVfoh10590744 = MlmShbidGaGOMVfoh20017682;     MlmShbidGaGOMVfoh20017682 = MlmShbidGaGOMVfoh66193485;     MlmShbidGaGOMVfoh66193485 = MlmShbidGaGOMVfoh97068369;     MlmShbidGaGOMVfoh97068369 = MlmShbidGaGOMVfoh26702789;     MlmShbidGaGOMVfoh26702789 = MlmShbidGaGOMVfoh18657981;     MlmShbidGaGOMVfoh18657981 = MlmShbidGaGOMVfoh92990622;     MlmShbidGaGOMVfoh92990622 = MlmShbidGaGOMVfoh86179671;     MlmShbidGaGOMVfoh86179671 = MlmShbidGaGOMVfoh26124021;     MlmShbidGaGOMVfoh26124021 = MlmShbidGaGOMVfoh22247594;     MlmShbidGaGOMVfoh22247594 = MlmShbidGaGOMVfoh67357406;     MlmShbidGaGOMVfoh67357406 = MlmShbidGaGOMVfoh15542189;     MlmShbidGaGOMVfoh15542189 = MlmShbidGaGOMVfoh17866255;     MlmShbidGaGOMVfoh17866255 = MlmShbidGaGOMVfoh55735801;     MlmShbidGaGOMVfoh55735801 = MlmShbidGaGOMVfoh94921995;     MlmShbidGaGOMVfoh94921995 = MlmShbidGaGOMVfoh81327139;     MlmShbidGaGOMVfoh81327139 = MlmShbidGaGOMVfoh70561422;     MlmShbidGaGOMVfoh70561422 = MlmShbidGaGOMVfoh17446234;     MlmShbidGaGOMVfoh17446234 = MlmShbidGaGOMVfoh98172346;     MlmShbidGaGOMVfoh98172346 = MlmShbidGaGOMVfoh3001288;     MlmShbidGaGOMVfoh3001288 = MlmShbidGaGOMVfoh11012692;     MlmShbidGaGOMVfoh11012692 = MlmShbidGaGOMVfoh37919078;     MlmShbidGaGOMVfoh37919078 = MlmShbidGaGOMVfoh79140820;     MlmShbidGaGOMVfoh79140820 = MlmShbidGaGOMVfoh29760437;     MlmShbidGaGOMVfoh29760437 = MlmShbidGaGOMVfoh89061630;     MlmShbidGaGOMVfoh89061630 = MlmShbidGaGOMVfoh25371956;     MlmShbidGaGOMVfoh25371956 = MlmShbidGaGOMVfoh78581096;     MlmShbidGaGOMVfoh78581096 = MlmShbidGaGOMVfoh31632482;     MlmShbidGaGOMVfoh31632482 = MlmShbidGaGOMVfoh86067515;     MlmShbidGaGOMVfoh86067515 = MlmShbidGaGOMVfoh35726580;     MlmShbidGaGOMVfoh35726580 = MlmShbidGaGOMVfoh76113634;     MlmShbidGaGOMVfoh76113634 = MlmShbidGaGOMVfoh62120534;     MlmShbidGaGOMVfoh62120534 = MlmShbidGaGOMVfoh38487271;     MlmShbidGaGOMVfoh38487271 = MlmShbidGaGOMVfoh9046454;     MlmShbidGaGOMVfoh9046454 = MlmShbidGaGOMVfoh45869874;     MlmShbidGaGOMVfoh45869874 = MlmShbidGaGOMVfoh1540039;     MlmShbidGaGOMVfoh1540039 = MlmShbidGaGOMVfoh41653616;     MlmShbidGaGOMVfoh41653616 = MlmShbidGaGOMVfoh19663954;     MlmShbidGaGOMVfoh19663954 = MlmShbidGaGOMVfoh17092437;     MlmShbidGaGOMVfoh17092437 = MlmShbidGaGOMVfoh5701650;     MlmShbidGaGOMVfoh5701650 = MlmShbidGaGOMVfoh27429953;     MlmShbidGaGOMVfoh27429953 = MlmShbidGaGOMVfoh71428193;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FujTrDvutbfPLtjL97205204() {     double HLXKjycqdAccrKzaP77545300 = 79417597;    double HLXKjycqdAccrKzaP57067512 = -347250932;    double HLXKjycqdAccrKzaP76754495 = -78761871;    double HLXKjycqdAccrKzaP48547145 = -507185354;    double HLXKjycqdAccrKzaP76470135 = -213046279;    double HLXKjycqdAccrKzaP10284302 = -485729703;    double HLXKjycqdAccrKzaP85942410 = -461125514;    double HLXKjycqdAccrKzaP70305133 = -723343431;    double HLXKjycqdAccrKzaP5380452 = -579141673;    double HLXKjycqdAccrKzaP55826306 = 38335172;    double HLXKjycqdAccrKzaP30885435 = -957292085;    double HLXKjycqdAccrKzaP2569277 = -421517201;    double HLXKjycqdAccrKzaP42922109 = 37289683;    double HLXKjycqdAccrKzaP91691639 = -377617643;    double HLXKjycqdAccrKzaP16382624 = -891073846;    double HLXKjycqdAccrKzaP32825302 = -523354067;    double HLXKjycqdAccrKzaP36726942 = -115781574;    double HLXKjycqdAccrKzaP9249572 = -577448510;    double HLXKjycqdAccrKzaP78309557 = -291129937;    double HLXKjycqdAccrKzaP69724406 = -916057194;    double HLXKjycqdAccrKzaP75846939 = -180547907;    double HLXKjycqdAccrKzaP47762483 = -906696004;    double HLXKjycqdAccrKzaP58122768 = -852145641;    double HLXKjycqdAccrKzaP7414237 = -714831437;    double HLXKjycqdAccrKzaP49193784 = -215590440;    double HLXKjycqdAccrKzaP21705579 = -31051224;    double HLXKjycqdAccrKzaP97309196 = -803644401;    double HLXKjycqdAccrKzaP72367084 = -957403757;    double HLXKjycqdAccrKzaP89609128 = -195181173;    double HLXKjycqdAccrKzaP82650804 = -381062112;    double HLXKjycqdAccrKzaP88181216 = -353448786;    double HLXKjycqdAccrKzaP75395143 = -705555052;    double HLXKjycqdAccrKzaP94277511 = -975435701;    double HLXKjycqdAccrKzaP25419535 = 79742918;    double HLXKjycqdAccrKzaP47775021 = -508912982;    double HLXKjycqdAccrKzaP22008704 = -59265097;    double HLXKjycqdAccrKzaP57039654 = -55157903;    double HLXKjycqdAccrKzaP59114897 = -27704485;    double HLXKjycqdAccrKzaP30070872 = -221849159;    double HLXKjycqdAccrKzaP9059658 = -873114242;    double HLXKjycqdAccrKzaP13516326 = -878721300;    double HLXKjycqdAccrKzaP48882780 = -385715459;    double HLXKjycqdAccrKzaP21862204 = -322952910;    double HLXKjycqdAccrKzaP11767371 = -893661211;    double HLXKjycqdAccrKzaP32373041 = -844793676;    double HLXKjycqdAccrKzaP6420638 = -224779239;    double HLXKjycqdAccrKzaP76843399 = 27403667;    double HLXKjycqdAccrKzaP27665540 = -965005464;    double HLXKjycqdAccrKzaP39175252 = -280556696;    double HLXKjycqdAccrKzaP47326895 = -419715867;    double HLXKjycqdAccrKzaP24363890 = -784867431;    double HLXKjycqdAccrKzaP28167290 = -672496960;    double HLXKjycqdAccrKzaP94400496 = -618411941;    double HLXKjycqdAccrKzaP94467928 = -598597726;    double HLXKjycqdAccrKzaP21358266 = -973741855;    double HLXKjycqdAccrKzaP29782817 = -13886400;    double HLXKjycqdAccrKzaP98944743 = -495105292;    double HLXKjycqdAccrKzaP69340258 = -363930434;    double HLXKjycqdAccrKzaP99353361 = -191594915;    double HLXKjycqdAccrKzaP54764557 = -81995056;    double HLXKjycqdAccrKzaP12975106 = -682085303;    double HLXKjycqdAccrKzaP13575327 = -503721758;    double HLXKjycqdAccrKzaP80696005 = -428162258;    double HLXKjycqdAccrKzaP22729647 = -98079561;    double HLXKjycqdAccrKzaP67645090 = -608216043;    double HLXKjycqdAccrKzaP55490291 = -151737034;    double HLXKjycqdAccrKzaP8291765 = -446081500;    double HLXKjycqdAccrKzaP17502575 = 57546764;    double HLXKjycqdAccrKzaP43916618 = -868704662;    double HLXKjycqdAccrKzaP94373920 = -731808749;    double HLXKjycqdAccrKzaP75785648 = -368196165;    double HLXKjycqdAccrKzaP77612045 = 11922911;    double HLXKjycqdAccrKzaP79178700 = -255599352;    double HLXKjycqdAccrKzaP69249899 = -418015696;    double HLXKjycqdAccrKzaP56208080 = 62664105;    double HLXKjycqdAccrKzaP26964159 = -794832448;    double HLXKjycqdAccrKzaP25900279 = -483743094;    double HLXKjycqdAccrKzaP46355398 = -958484430;    double HLXKjycqdAccrKzaP75041196 = -870037762;    double HLXKjycqdAccrKzaP42773147 = -990811202;    double HLXKjycqdAccrKzaP44862179 = 41545108;    double HLXKjycqdAccrKzaP69643657 = -838638937;    double HLXKjycqdAccrKzaP33191833 = -576847061;    double HLXKjycqdAccrKzaP42282233 = -775465307;    double HLXKjycqdAccrKzaP58286914 = -596194682;    double HLXKjycqdAccrKzaP60013926 = -680951826;    double HLXKjycqdAccrKzaP80994646 = 12856889;    double HLXKjycqdAccrKzaP99809583 = -276837976;    double HLXKjycqdAccrKzaP4061270 = 53484773;    double HLXKjycqdAccrKzaP17992204 = -395026582;    double HLXKjycqdAccrKzaP23063960 = -564159806;    double HLXKjycqdAccrKzaP87699395 = -691227469;    double HLXKjycqdAccrKzaP59761536 = -836109571;    double HLXKjycqdAccrKzaP75306314 = -39854104;    double HLXKjycqdAccrKzaP96084551 = -91028940;    double HLXKjycqdAccrKzaP99940999 = -274999542;    double HLXKjycqdAccrKzaP68186775 = -957553201;    double HLXKjycqdAccrKzaP99132556 = -124873350;    double HLXKjycqdAccrKzaP44122280 = -185445168;    double HLXKjycqdAccrKzaP76882749 = 79417597;     HLXKjycqdAccrKzaP77545300 = HLXKjycqdAccrKzaP57067512;     HLXKjycqdAccrKzaP57067512 = HLXKjycqdAccrKzaP76754495;     HLXKjycqdAccrKzaP76754495 = HLXKjycqdAccrKzaP48547145;     HLXKjycqdAccrKzaP48547145 = HLXKjycqdAccrKzaP76470135;     HLXKjycqdAccrKzaP76470135 = HLXKjycqdAccrKzaP10284302;     HLXKjycqdAccrKzaP10284302 = HLXKjycqdAccrKzaP85942410;     HLXKjycqdAccrKzaP85942410 = HLXKjycqdAccrKzaP70305133;     HLXKjycqdAccrKzaP70305133 = HLXKjycqdAccrKzaP5380452;     HLXKjycqdAccrKzaP5380452 = HLXKjycqdAccrKzaP55826306;     HLXKjycqdAccrKzaP55826306 = HLXKjycqdAccrKzaP30885435;     HLXKjycqdAccrKzaP30885435 = HLXKjycqdAccrKzaP2569277;     HLXKjycqdAccrKzaP2569277 = HLXKjycqdAccrKzaP42922109;     HLXKjycqdAccrKzaP42922109 = HLXKjycqdAccrKzaP91691639;     HLXKjycqdAccrKzaP91691639 = HLXKjycqdAccrKzaP16382624;     HLXKjycqdAccrKzaP16382624 = HLXKjycqdAccrKzaP32825302;     HLXKjycqdAccrKzaP32825302 = HLXKjycqdAccrKzaP36726942;     HLXKjycqdAccrKzaP36726942 = HLXKjycqdAccrKzaP9249572;     HLXKjycqdAccrKzaP9249572 = HLXKjycqdAccrKzaP78309557;     HLXKjycqdAccrKzaP78309557 = HLXKjycqdAccrKzaP69724406;     HLXKjycqdAccrKzaP69724406 = HLXKjycqdAccrKzaP75846939;     HLXKjycqdAccrKzaP75846939 = HLXKjycqdAccrKzaP47762483;     HLXKjycqdAccrKzaP47762483 = HLXKjycqdAccrKzaP58122768;     HLXKjycqdAccrKzaP58122768 = HLXKjycqdAccrKzaP7414237;     HLXKjycqdAccrKzaP7414237 = HLXKjycqdAccrKzaP49193784;     HLXKjycqdAccrKzaP49193784 = HLXKjycqdAccrKzaP21705579;     HLXKjycqdAccrKzaP21705579 = HLXKjycqdAccrKzaP97309196;     HLXKjycqdAccrKzaP97309196 = HLXKjycqdAccrKzaP72367084;     HLXKjycqdAccrKzaP72367084 = HLXKjycqdAccrKzaP89609128;     HLXKjycqdAccrKzaP89609128 = HLXKjycqdAccrKzaP82650804;     HLXKjycqdAccrKzaP82650804 = HLXKjycqdAccrKzaP88181216;     HLXKjycqdAccrKzaP88181216 = HLXKjycqdAccrKzaP75395143;     HLXKjycqdAccrKzaP75395143 = HLXKjycqdAccrKzaP94277511;     HLXKjycqdAccrKzaP94277511 = HLXKjycqdAccrKzaP25419535;     HLXKjycqdAccrKzaP25419535 = HLXKjycqdAccrKzaP47775021;     HLXKjycqdAccrKzaP47775021 = HLXKjycqdAccrKzaP22008704;     HLXKjycqdAccrKzaP22008704 = HLXKjycqdAccrKzaP57039654;     HLXKjycqdAccrKzaP57039654 = HLXKjycqdAccrKzaP59114897;     HLXKjycqdAccrKzaP59114897 = HLXKjycqdAccrKzaP30070872;     HLXKjycqdAccrKzaP30070872 = HLXKjycqdAccrKzaP9059658;     HLXKjycqdAccrKzaP9059658 = HLXKjycqdAccrKzaP13516326;     HLXKjycqdAccrKzaP13516326 = HLXKjycqdAccrKzaP48882780;     HLXKjycqdAccrKzaP48882780 = HLXKjycqdAccrKzaP21862204;     HLXKjycqdAccrKzaP21862204 = HLXKjycqdAccrKzaP11767371;     HLXKjycqdAccrKzaP11767371 = HLXKjycqdAccrKzaP32373041;     HLXKjycqdAccrKzaP32373041 = HLXKjycqdAccrKzaP6420638;     HLXKjycqdAccrKzaP6420638 = HLXKjycqdAccrKzaP76843399;     HLXKjycqdAccrKzaP76843399 = HLXKjycqdAccrKzaP27665540;     HLXKjycqdAccrKzaP27665540 = HLXKjycqdAccrKzaP39175252;     HLXKjycqdAccrKzaP39175252 = HLXKjycqdAccrKzaP47326895;     HLXKjycqdAccrKzaP47326895 = HLXKjycqdAccrKzaP24363890;     HLXKjycqdAccrKzaP24363890 = HLXKjycqdAccrKzaP28167290;     HLXKjycqdAccrKzaP28167290 = HLXKjycqdAccrKzaP94400496;     HLXKjycqdAccrKzaP94400496 = HLXKjycqdAccrKzaP94467928;     HLXKjycqdAccrKzaP94467928 = HLXKjycqdAccrKzaP21358266;     HLXKjycqdAccrKzaP21358266 = HLXKjycqdAccrKzaP29782817;     HLXKjycqdAccrKzaP29782817 = HLXKjycqdAccrKzaP98944743;     HLXKjycqdAccrKzaP98944743 = HLXKjycqdAccrKzaP69340258;     HLXKjycqdAccrKzaP69340258 = HLXKjycqdAccrKzaP99353361;     HLXKjycqdAccrKzaP99353361 = HLXKjycqdAccrKzaP54764557;     HLXKjycqdAccrKzaP54764557 = HLXKjycqdAccrKzaP12975106;     HLXKjycqdAccrKzaP12975106 = HLXKjycqdAccrKzaP13575327;     HLXKjycqdAccrKzaP13575327 = HLXKjycqdAccrKzaP80696005;     HLXKjycqdAccrKzaP80696005 = HLXKjycqdAccrKzaP22729647;     HLXKjycqdAccrKzaP22729647 = HLXKjycqdAccrKzaP67645090;     HLXKjycqdAccrKzaP67645090 = HLXKjycqdAccrKzaP55490291;     HLXKjycqdAccrKzaP55490291 = HLXKjycqdAccrKzaP8291765;     HLXKjycqdAccrKzaP8291765 = HLXKjycqdAccrKzaP17502575;     HLXKjycqdAccrKzaP17502575 = HLXKjycqdAccrKzaP43916618;     HLXKjycqdAccrKzaP43916618 = HLXKjycqdAccrKzaP94373920;     HLXKjycqdAccrKzaP94373920 = HLXKjycqdAccrKzaP75785648;     HLXKjycqdAccrKzaP75785648 = HLXKjycqdAccrKzaP77612045;     HLXKjycqdAccrKzaP77612045 = HLXKjycqdAccrKzaP79178700;     HLXKjycqdAccrKzaP79178700 = HLXKjycqdAccrKzaP69249899;     HLXKjycqdAccrKzaP69249899 = HLXKjycqdAccrKzaP56208080;     HLXKjycqdAccrKzaP56208080 = HLXKjycqdAccrKzaP26964159;     HLXKjycqdAccrKzaP26964159 = HLXKjycqdAccrKzaP25900279;     HLXKjycqdAccrKzaP25900279 = HLXKjycqdAccrKzaP46355398;     HLXKjycqdAccrKzaP46355398 = HLXKjycqdAccrKzaP75041196;     HLXKjycqdAccrKzaP75041196 = HLXKjycqdAccrKzaP42773147;     HLXKjycqdAccrKzaP42773147 = HLXKjycqdAccrKzaP44862179;     HLXKjycqdAccrKzaP44862179 = HLXKjycqdAccrKzaP69643657;     HLXKjycqdAccrKzaP69643657 = HLXKjycqdAccrKzaP33191833;     HLXKjycqdAccrKzaP33191833 = HLXKjycqdAccrKzaP42282233;     HLXKjycqdAccrKzaP42282233 = HLXKjycqdAccrKzaP58286914;     HLXKjycqdAccrKzaP58286914 = HLXKjycqdAccrKzaP60013926;     HLXKjycqdAccrKzaP60013926 = HLXKjycqdAccrKzaP80994646;     HLXKjycqdAccrKzaP80994646 = HLXKjycqdAccrKzaP99809583;     HLXKjycqdAccrKzaP99809583 = HLXKjycqdAccrKzaP4061270;     HLXKjycqdAccrKzaP4061270 = HLXKjycqdAccrKzaP17992204;     HLXKjycqdAccrKzaP17992204 = HLXKjycqdAccrKzaP23063960;     HLXKjycqdAccrKzaP23063960 = HLXKjycqdAccrKzaP87699395;     HLXKjycqdAccrKzaP87699395 = HLXKjycqdAccrKzaP59761536;     HLXKjycqdAccrKzaP59761536 = HLXKjycqdAccrKzaP75306314;     HLXKjycqdAccrKzaP75306314 = HLXKjycqdAccrKzaP96084551;     HLXKjycqdAccrKzaP96084551 = HLXKjycqdAccrKzaP99940999;     HLXKjycqdAccrKzaP99940999 = HLXKjycqdAccrKzaP68186775;     HLXKjycqdAccrKzaP68186775 = HLXKjycqdAccrKzaP99132556;     HLXKjycqdAccrKzaP99132556 = HLXKjycqdAccrKzaP44122280;     HLXKjycqdAccrKzaP44122280 = HLXKjycqdAccrKzaP76882749;     HLXKjycqdAccrKzaP76882749 = HLXKjycqdAccrKzaP77545300;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nvFFjBOTFHHcXhqF64496803() {     double UshUoClkjHEKfXdXN13004269 = -386859303;    double UshUoClkjHEKfXdXN70339590 = -227802539;    double UshUoClkjHEKfXdXN14140972 = -844428946;    double UshUoClkjHEKfXdXN20770501 = -328336426;    double UshUoClkjHEKfXdXN32755405 = -318645125;    double UshUoClkjHEKfXdXN46507174 = -748077599;    double UshUoClkjHEKfXdXN96599817 = -305851248;    double UshUoClkjHEKfXdXN32436694 = -367577571;    double UshUoClkjHEKfXdXN6898057 = 32526920;    double UshUoClkjHEKfXdXN70729018 = -726351826;    double UshUoClkjHEKfXdXN69067169 = -759299815;    double UshUoClkjHEKfXdXN3137426 = -285637972;    double UshUoClkjHEKfXdXN81156754 = -514722822;    double UshUoClkjHEKfXdXN44832502 = -353981053;    double UshUoClkjHEKfXdXN74577140 = -294498861;    double UshUoClkjHEKfXdXN16366603 = -159165674;    double UshUoClkjHEKfXdXN92562877 = -642756127;    double UshUoClkjHEKfXdXN9009764 = -650876579;    double UshUoClkjHEKfXdXN78859886 = -607693359;    double UshUoClkjHEKfXdXN63561625 = -242618260;    double UshUoClkjHEKfXdXN88933802 = -937822595;    double UshUoClkjHEKfXdXN51184008 = -993332298;    double UshUoClkjHEKfXdXN28648149 = -489777825;    double UshUoClkjHEKfXdXN1207182 = -125656042;    double UshUoClkjHEKfXdXN57211735 = -239575638;    double UshUoClkjHEKfXdXN32403750 = -819628031;    double UshUoClkjHEKfXdXN57729770 = -609928542;    double UshUoClkjHEKfXdXN13095510 = -27175931;    double UshUoClkjHEKfXdXN36277343 = -769275975;    double UshUoClkjHEKfXdXN88184215 = -415569283;    double UshUoClkjHEKfXdXN6764448 = -905429273;    double UshUoClkjHEKfXdXN13218162 = -478526466;    double UshUoClkjHEKfXdXN6495523 = -23031679;    double UshUoClkjHEKfXdXN55167961 = 15049105;    double UshUoClkjHEKfXdXN13583475 = -460013232;    double UshUoClkjHEKfXdXN73569547 = -273830403;    double UshUoClkjHEKfXdXN6451471 = 80146057;    double UshUoClkjHEKfXdXN10330301 = -10360066;    double UshUoClkjHEKfXdXN60461433 = -972725435;    double UshUoClkjHEKfXdXN73331189 = -338376788;    double UshUoClkjHEKfXdXN46967934 = -647920214;    double UshUoClkjHEKfXdXN38247907 = 19739250;    double UshUoClkjHEKfXdXN11245582 = -976133696;    double UshUoClkjHEKfXdXN51898709 = -397993621;    double UshUoClkjHEKfXdXN32389499 = -671304837;    double UshUoClkjHEKfXdXN52381966 = -263702871;    double UshUoClkjHEKfXdXN96188562 = -456582693;    double UshUoClkjHEKfXdXN45391851 = -31532058;    double UshUoClkjHEKfXdXN62067918 = 55706423;    double UshUoClkjHEKfXdXN71856484 = -910742682;    double UshUoClkjHEKfXdXN5134614 = -659143988;    double UshUoClkjHEKfXdXN11652763 = -886149307;    double UshUoClkjHEKfXdXN63021645 = -374457495;    double UshUoClkjHEKfXdXN10459775 = -790106226;    double UshUoClkjHEKfXdXN75563001 = -220232001;    double UshUoClkjHEKfXdXN61820261 = -393527005;    double UshUoClkjHEKfXdXN41691442 = -738024714;    double UshUoClkjHEKfXdXN12933790 = -618772904;    double UshUoClkjHEKfXdXN63558765 = 11239212;    double UshUoClkjHEKfXdXN351655 = -499017094;    double UshUoClkjHEKfXdXN88777403 = -38149058;    double UshUoClkjHEKfXdXN83504307 = -178675318;    double UshUoClkjHEKfXdXN96159351 = -598301596;    double UshUoClkjHEKfXdXN18713842 = -551903797;    double UshUoClkjHEKfXdXN63964571 = -820922554;    double UshUoClkjHEKfXdXN55849008 = -180773349;    double UshUoClkjHEKfXdXN96641902 = -162606294;    double UshUoClkjHEKfXdXN25988794 = -429771928;    double UshUoClkjHEKfXdXN31249028 = -893967821;    double UshUoClkjHEKfXdXN1007593 = 79331541;    double UshUoClkjHEKfXdXN9915133 = -139311731;    double UshUoClkjHEKfXdXN82232577 = -532396061;    double UshUoClkjHEKfXdXN48548330 = -678151144;    double UshUoClkjHEKfXdXN5528698 = -169316571;    double UshUoClkjHEKfXdXN16593692 = -594698046;    double UshUoClkjHEKfXdXN50685895 = -857561846;    double UshUoClkjHEKfXdXN39938426 = 82801397;    double UshUoClkjHEKfXdXN76749439 = 8215795;    double UshUoClkjHEKfXdXN68817683 = -454351205;    double UshUoClkjHEKfXdXN4829770 = -975872768;    double UshUoClkjHEKfXdXN36215187 = -263045339;    double UshUoClkjHEKfXdXN12337920 = -478396484;    double UshUoClkjHEKfXdXN51027592 = 17117645;    double UshUoClkjHEKfXdXN64420858 = -858533293;    double UshUoClkjHEKfXdXN83049601 = -756425295;    double UshUoClkjHEKfXdXN95111685 = 80720033;    double UshUoClkjHEKfXdXN50196516 = -4068972;    double UshUoClkjHEKfXdXN96035747 = -232925453;    double UshUoClkjHEKfXdXN79604959 = -764718895;    double UshUoClkjHEKfXdXN51763214 = 33513773;    double UshUoClkjHEKfXdXN31878105 = -535805689;    double UshUoClkjHEKfXdXN93517680 = -301081039;    double UshUoClkjHEKfXdXN46771536 = 78400721;    double UshUoClkjHEKfXdXN60109778 = -373708341;    double UshUoClkjHEKfXdXN84553786 = -200227731;    double UshUoClkjHEKfXdXN63463626 = -369244897;    double UshUoClkjHEKfXdXN42088556 = -381959154;    double UshUoClkjHEKfXdXN92531740 = -324229899;    double UshUoClkjHEKfXdXN87934137 = -577071068;    double UshUoClkjHEKfXdXN76540491 = -386859303;     UshUoClkjHEKfXdXN13004269 = UshUoClkjHEKfXdXN70339590;     UshUoClkjHEKfXdXN70339590 = UshUoClkjHEKfXdXN14140972;     UshUoClkjHEKfXdXN14140972 = UshUoClkjHEKfXdXN20770501;     UshUoClkjHEKfXdXN20770501 = UshUoClkjHEKfXdXN32755405;     UshUoClkjHEKfXdXN32755405 = UshUoClkjHEKfXdXN46507174;     UshUoClkjHEKfXdXN46507174 = UshUoClkjHEKfXdXN96599817;     UshUoClkjHEKfXdXN96599817 = UshUoClkjHEKfXdXN32436694;     UshUoClkjHEKfXdXN32436694 = UshUoClkjHEKfXdXN6898057;     UshUoClkjHEKfXdXN6898057 = UshUoClkjHEKfXdXN70729018;     UshUoClkjHEKfXdXN70729018 = UshUoClkjHEKfXdXN69067169;     UshUoClkjHEKfXdXN69067169 = UshUoClkjHEKfXdXN3137426;     UshUoClkjHEKfXdXN3137426 = UshUoClkjHEKfXdXN81156754;     UshUoClkjHEKfXdXN81156754 = UshUoClkjHEKfXdXN44832502;     UshUoClkjHEKfXdXN44832502 = UshUoClkjHEKfXdXN74577140;     UshUoClkjHEKfXdXN74577140 = UshUoClkjHEKfXdXN16366603;     UshUoClkjHEKfXdXN16366603 = UshUoClkjHEKfXdXN92562877;     UshUoClkjHEKfXdXN92562877 = UshUoClkjHEKfXdXN9009764;     UshUoClkjHEKfXdXN9009764 = UshUoClkjHEKfXdXN78859886;     UshUoClkjHEKfXdXN78859886 = UshUoClkjHEKfXdXN63561625;     UshUoClkjHEKfXdXN63561625 = UshUoClkjHEKfXdXN88933802;     UshUoClkjHEKfXdXN88933802 = UshUoClkjHEKfXdXN51184008;     UshUoClkjHEKfXdXN51184008 = UshUoClkjHEKfXdXN28648149;     UshUoClkjHEKfXdXN28648149 = UshUoClkjHEKfXdXN1207182;     UshUoClkjHEKfXdXN1207182 = UshUoClkjHEKfXdXN57211735;     UshUoClkjHEKfXdXN57211735 = UshUoClkjHEKfXdXN32403750;     UshUoClkjHEKfXdXN32403750 = UshUoClkjHEKfXdXN57729770;     UshUoClkjHEKfXdXN57729770 = UshUoClkjHEKfXdXN13095510;     UshUoClkjHEKfXdXN13095510 = UshUoClkjHEKfXdXN36277343;     UshUoClkjHEKfXdXN36277343 = UshUoClkjHEKfXdXN88184215;     UshUoClkjHEKfXdXN88184215 = UshUoClkjHEKfXdXN6764448;     UshUoClkjHEKfXdXN6764448 = UshUoClkjHEKfXdXN13218162;     UshUoClkjHEKfXdXN13218162 = UshUoClkjHEKfXdXN6495523;     UshUoClkjHEKfXdXN6495523 = UshUoClkjHEKfXdXN55167961;     UshUoClkjHEKfXdXN55167961 = UshUoClkjHEKfXdXN13583475;     UshUoClkjHEKfXdXN13583475 = UshUoClkjHEKfXdXN73569547;     UshUoClkjHEKfXdXN73569547 = UshUoClkjHEKfXdXN6451471;     UshUoClkjHEKfXdXN6451471 = UshUoClkjHEKfXdXN10330301;     UshUoClkjHEKfXdXN10330301 = UshUoClkjHEKfXdXN60461433;     UshUoClkjHEKfXdXN60461433 = UshUoClkjHEKfXdXN73331189;     UshUoClkjHEKfXdXN73331189 = UshUoClkjHEKfXdXN46967934;     UshUoClkjHEKfXdXN46967934 = UshUoClkjHEKfXdXN38247907;     UshUoClkjHEKfXdXN38247907 = UshUoClkjHEKfXdXN11245582;     UshUoClkjHEKfXdXN11245582 = UshUoClkjHEKfXdXN51898709;     UshUoClkjHEKfXdXN51898709 = UshUoClkjHEKfXdXN32389499;     UshUoClkjHEKfXdXN32389499 = UshUoClkjHEKfXdXN52381966;     UshUoClkjHEKfXdXN52381966 = UshUoClkjHEKfXdXN96188562;     UshUoClkjHEKfXdXN96188562 = UshUoClkjHEKfXdXN45391851;     UshUoClkjHEKfXdXN45391851 = UshUoClkjHEKfXdXN62067918;     UshUoClkjHEKfXdXN62067918 = UshUoClkjHEKfXdXN71856484;     UshUoClkjHEKfXdXN71856484 = UshUoClkjHEKfXdXN5134614;     UshUoClkjHEKfXdXN5134614 = UshUoClkjHEKfXdXN11652763;     UshUoClkjHEKfXdXN11652763 = UshUoClkjHEKfXdXN63021645;     UshUoClkjHEKfXdXN63021645 = UshUoClkjHEKfXdXN10459775;     UshUoClkjHEKfXdXN10459775 = UshUoClkjHEKfXdXN75563001;     UshUoClkjHEKfXdXN75563001 = UshUoClkjHEKfXdXN61820261;     UshUoClkjHEKfXdXN61820261 = UshUoClkjHEKfXdXN41691442;     UshUoClkjHEKfXdXN41691442 = UshUoClkjHEKfXdXN12933790;     UshUoClkjHEKfXdXN12933790 = UshUoClkjHEKfXdXN63558765;     UshUoClkjHEKfXdXN63558765 = UshUoClkjHEKfXdXN351655;     UshUoClkjHEKfXdXN351655 = UshUoClkjHEKfXdXN88777403;     UshUoClkjHEKfXdXN88777403 = UshUoClkjHEKfXdXN83504307;     UshUoClkjHEKfXdXN83504307 = UshUoClkjHEKfXdXN96159351;     UshUoClkjHEKfXdXN96159351 = UshUoClkjHEKfXdXN18713842;     UshUoClkjHEKfXdXN18713842 = UshUoClkjHEKfXdXN63964571;     UshUoClkjHEKfXdXN63964571 = UshUoClkjHEKfXdXN55849008;     UshUoClkjHEKfXdXN55849008 = UshUoClkjHEKfXdXN96641902;     UshUoClkjHEKfXdXN96641902 = UshUoClkjHEKfXdXN25988794;     UshUoClkjHEKfXdXN25988794 = UshUoClkjHEKfXdXN31249028;     UshUoClkjHEKfXdXN31249028 = UshUoClkjHEKfXdXN1007593;     UshUoClkjHEKfXdXN1007593 = UshUoClkjHEKfXdXN9915133;     UshUoClkjHEKfXdXN9915133 = UshUoClkjHEKfXdXN82232577;     UshUoClkjHEKfXdXN82232577 = UshUoClkjHEKfXdXN48548330;     UshUoClkjHEKfXdXN48548330 = UshUoClkjHEKfXdXN5528698;     UshUoClkjHEKfXdXN5528698 = UshUoClkjHEKfXdXN16593692;     UshUoClkjHEKfXdXN16593692 = UshUoClkjHEKfXdXN50685895;     UshUoClkjHEKfXdXN50685895 = UshUoClkjHEKfXdXN39938426;     UshUoClkjHEKfXdXN39938426 = UshUoClkjHEKfXdXN76749439;     UshUoClkjHEKfXdXN76749439 = UshUoClkjHEKfXdXN68817683;     UshUoClkjHEKfXdXN68817683 = UshUoClkjHEKfXdXN4829770;     UshUoClkjHEKfXdXN4829770 = UshUoClkjHEKfXdXN36215187;     UshUoClkjHEKfXdXN36215187 = UshUoClkjHEKfXdXN12337920;     UshUoClkjHEKfXdXN12337920 = UshUoClkjHEKfXdXN51027592;     UshUoClkjHEKfXdXN51027592 = UshUoClkjHEKfXdXN64420858;     UshUoClkjHEKfXdXN64420858 = UshUoClkjHEKfXdXN83049601;     UshUoClkjHEKfXdXN83049601 = UshUoClkjHEKfXdXN95111685;     UshUoClkjHEKfXdXN95111685 = UshUoClkjHEKfXdXN50196516;     UshUoClkjHEKfXdXN50196516 = UshUoClkjHEKfXdXN96035747;     UshUoClkjHEKfXdXN96035747 = UshUoClkjHEKfXdXN79604959;     UshUoClkjHEKfXdXN79604959 = UshUoClkjHEKfXdXN51763214;     UshUoClkjHEKfXdXN51763214 = UshUoClkjHEKfXdXN31878105;     UshUoClkjHEKfXdXN31878105 = UshUoClkjHEKfXdXN93517680;     UshUoClkjHEKfXdXN93517680 = UshUoClkjHEKfXdXN46771536;     UshUoClkjHEKfXdXN46771536 = UshUoClkjHEKfXdXN60109778;     UshUoClkjHEKfXdXN60109778 = UshUoClkjHEKfXdXN84553786;     UshUoClkjHEKfXdXN84553786 = UshUoClkjHEKfXdXN63463626;     UshUoClkjHEKfXdXN63463626 = UshUoClkjHEKfXdXN42088556;     UshUoClkjHEKfXdXN42088556 = UshUoClkjHEKfXdXN92531740;     UshUoClkjHEKfXdXN92531740 = UshUoClkjHEKfXdXN87934137;     UshUoClkjHEKfXdXN87934137 = UshUoClkjHEKfXdXN76540491;     UshUoClkjHEKfXdXN76540491 = UshUoClkjHEKfXdXN13004269;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nuOFdXhgMfmDbwLJ79545871() {     double zvbzcEgCrPTWgMamQ19121375 = -498761192;    double zvbzcEgCrPTWgMamQ13712870 = 62597681;    double zvbzcEgCrPTWgMamQ30183156 = -995261867;    double zvbzcEgCrPTWgMamQ47845281 = -519085056;    double zvbzcEgCrPTWgMamQ1650893 = -839294996;    double zvbzcEgCrPTWgMamQ37924377 = 87426166;    double zvbzcEgCrPTWgMamQ57088783 = -444816225;    double zvbzcEgCrPTWgMamQ82196585 = -654451484;    double zvbzcEgCrPTWgMamQ71540552 = -835505745;    double zvbzcEgCrPTWgMamQ11958678 = 44672696;    double zvbzcEgCrPTWgMamQ29177460 = -636737401;    double zvbzcEgCrPTWgMamQ34960468 = -730044174;    double zvbzcEgCrPTWgMamQ67365873 = -954613911;    double zvbzcEgCrPTWgMamQ27976179 = -886219621;    double zvbzcEgCrPTWgMamQ70319567 = -984961028;    double zvbzcEgCrPTWgMamQ57871996 = -132165217;    double zvbzcEgCrPTWgMamQ86398880 = -159301239;    double zvbzcEgCrPTWgMamQ6062917 = -316399221;    double zvbzcEgCrPTWgMamQ40689729 = 86537544;    double zvbzcEgCrPTWgMamQ44111790 = -599521179;    double zvbzcEgCrPTWgMamQ77486995 = -793310051;    double zvbzcEgCrPTWgMamQ37705032 = -901262015;    double zvbzcEgCrPTWgMamQ64238940 = -236295312;    double zvbzcEgCrPTWgMamQ85199555 = -347961029;    double zvbzcEgCrPTWgMamQ911386 = -612771671;    double zvbzcEgCrPTWgMamQ57125425 = -698283354;    double zvbzcEgCrPTWgMamQ56189549 = -510590745;    double zvbzcEgCrPTWgMamQ26202635 = -93104885;    double zvbzcEgCrPTWgMamQ2409599 = -991658680;    double zvbzcEgCrPTWgMamQ56799852 = -233040150;    double zvbzcEgCrPTWgMamQ99006997 = -818750211;    double zvbzcEgCrPTWgMamQ10828784 = -187973282;    double zvbzcEgCrPTWgMamQ16206472 = 84354951;    double zvbzcEgCrPTWgMamQ49998525 = -836492660;    double zvbzcEgCrPTWgMamQ75058126 = 1253662;    double zvbzcEgCrPTWgMamQ42295461 = -487856511;    double zvbzcEgCrPTWgMamQ87713405 = -973489812;    double zvbzcEgCrPTWgMamQ44420513 = -881866076;    double zvbzcEgCrPTWgMamQ34071687 = -966648932;    double zvbzcEgCrPTWgMamQ60833127 = -811816011;    double zvbzcEgCrPTWgMamQ52637157 = -713194108;    double zvbzcEgCrPTWgMamQ70398364 = -250403028;    double zvbzcEgCrPTWgMamQ89312560 = 43061720;    double zvbzcEgCrPTWgMamQ39306449 = -417336559;    double zvbzcEgCrPTWgMamQ44341964 = -333202992;    double zvbzcEgCrPTWgMamQ64321161 = -248918987;    double zvbzcEgCrPTWgMamQ13967137 = -575358231;    double zvbzcEgCrPTWgMamQ53348792 = -41427590;    double zvbzcEgCrPTWgMamQ71743646 = -30030565;    double zvbzcEgCrPTWgMamQ84768137 = -997753490;    double zvbzcEgCrPTWgMamQ40835292 = -501437660;    double zvbzcEgCrPTWgMamQ22462482 = -686875627;    double zvbzcEgCrPTWgMamQ11270103 = -166247083;    double zvbzcEgCrPTWgMamQ6428656 = -10576371;    double zvbzcEgCrPTWgMamQ2058877 = -920763783;    double zvbzcEgCrPTWgMamQ81416343 = -597499178;    double zvbzcEgCrPTWgMamQ49473930 = -701107008;    double zvbzcEgCrPTWgMamQ44983600 = -547300838;    double zvbzcEgCrPTWgMamQ46933896 = -906313385;    double zvbzcEgCrPTWgMamQ44525468 = -41011642;    double zvbzcEgCrPTWgMamQ81734827 = -401983090;    double zvbzcEgCrPTWgMamQ30886148 = -251711341;    double zvbzcEgCrPTWgMamQ79786986 = -662792805;    double zvbzcEgCrPTWgMamQ14740700 = -502465596;    double zvbzcEgCrPTWgMamQ12951681 = -136577093;    double zvbzcEgCrPTWgMamQ18348677 = -348764120;    double zvbzcEgCrPTWgMamQ18753997 = -714399126;    double zvbzcEgCrPTWgMamQ17367348 = -18121251;    double zvbzcEgCrPTWgMamQ52918053 = -787473283;    double zvbzcEgCrPTWgMamQ28024106 = -397104517;    double zvbzcEgCrPTWgMamQ70158591 = -158675405;    double zvbzcEgCrPTWgMamQ41978368 = -277435164;    double zvbzcEgCrPTWgMamQ71991229 = -349750290;    double zvbzcEgCrPTWgMamQ79856601 = -101646445;    double zvbzcEgCrPTWgMamQ91474632 = -886327072;    double zvbzcEgCrPTWgMamQ7088632 = -442907023;    double zvbzcEgCrPTWgMamQ48392471 = -844323735;    double zvbzcEgCrPTWgMamQ24932491 = -818958753;    double zvbzcEgCrPTWgMamQ40857592 = 85241962;    double zvbzcEgCrPTWgMamQ36590225 = -263852684;    double zvbzcEgCrPTWgMamQ43158288 = -22925124;    double zvbzcEgCrPTWgMamQ2840757 = -369163155;    double zvbzcEgCrPTWgMamQ54458988 = 36925680;    double zvbzcEgCrPTWgMamQ17641462 = -993905190;    double zvbzcEgCrPTWgMamQ15964560 = -731602491;    double zvbzcEgCrPTWgMamQ76544515 = -31874585;    double zvbzcEgCrPTWgMamQ99558680 = 78273800;    double zvbzcEgCrPTWgMamQ9777816 = -905068678;    double zvbzcEgCrPTWgMamQ47939649 = -915728878;    double zvbzcEgCrPTWgMamQ93641782 = -401247160;    double zvbzcEgCrPTWgMamQ92821530 = -786749503;    double zvbzcEgCrPTWgMamQ42729805 = -326188975;    double zvbzcEgCrPTWgMamQ97486617 = -975552691;    double zvbzcEgCrPTWgMamQ89546219 = -825637290;    double zvbzcEgCrPTWgMamQ79098299 = -309832922;    double zvbzcEgCrPTWgMamQ21751010 = -361482767;    double zvbzcEgCrPTWgMamQ90611377 = -587610223;    double zvbzcEgCrPTWgMamQ74571860 = -454472685;    double zvbzcEgCrPTWgMamQ26354768 = -180759466;    double zvbzcEgCrPTWgMamQ25993288 = -498761192;     zvbzcEgCrPTWgMamQ19121375 = zvbzcEgCrPTWgMamQ13712870;     zvbzcEgCrPTWgMamQ13712870 = zvbzcEgCrPTWgMamQ30183156;     zvbzcEgCrPTWgMamQ30183156 = zvbzcEgCrPTWgMamQ47845281;     zvbzcEgCrPTWgMamQ47845281 = zvbzcEgCrPTWgMamQ1650893;     zvbzcEgCrPTWgMamQ1650893 = zvbzcEgCrPTWgMamQ37924377;     zvbzcEgCrPTWgMamQ37924377 = zvbzcEgCrPTWgMamQ57088783;     zvbzcEgCrPTWgMamQ57088783 = zvbzcEgCrPTWgMamQ82196585;     zvbzcEgCrPTWgMamQ82196585 = zvbzcEgCrPTWgMamQ71540552;     zvbzcEgCrPTWgMamQ71540552 = zvbzcEgCrPTWgMamQ11958678;     zvbzcEgCrPTWgMamQ11958678 = zvbzcEgCrPTWgMamQ29177460;     zvbzcEgCrPTWgMamQ29177460 = zvbzcEgCrPTWgMamQ34960468;     zvbzcEgCrPTWgMamQ34960468 = zvbzcEgCrPTWgMamQ67365873;     zvbzcEgCrPTWgMamQ67365873 = zvbzcEgCrPTWgMamQ27976179;     zvbzcEgCrPTWgMamQ27976179 = zvbzcEgCrPTWgMamQ70319567;     zvbzcEgCrPTWgMamQ70319567 = zvbzcEgCrPTWgMamQ57871996;     zvbzcEgCrPTWgMamQ57871996 = zvbzcEgCrPTWgMamQ86398880;     zvbzcEgCrPTWgMamQ86398880 = zvbzcEgCrPTWgMamQ6062917;     zvbzcEgCrPTWgMamQ6062917 = zvbzcEgCrPTWgMamQ40689729;     zvbzcEgCrPTWgMamQ40689729 = zvbzcEgCrPTWgMamQ44111790;     zvbzcEgCrPTWgMamQ44111790 = zvbzcEgCrPTWgMamQ77486995;     zvbzcEgCrPTWgMamQ77486995 = zvbzcEgCrPTWgMamQ37705032;     zvbzcEgCrPTWgMamQ37705032 = zvbzcEgCrPTWgMamQ64238940;     zvbzcEgCrPTWgMamQ64238940 = zvbzcEgCrPTWgMamQ85199555;     zvbzcEgCrPTWgMamQ85199555 = zvbzcEgCrPTWgMamQ911386;     zvbzcEgCrPTWgMamQ911386 = zvbzcEgCrPTWgMamQ57125425;     zvbzcEgCrPTWgMamQ57125425 = zvbzcEgCrPTWgMamQ56189549;     zvbzcEgCrPTWgMamQ56189549 = zvbzcEgCrPTWgMamQ26202635;     zvbzcEgCrPTWgMamQ26202635 = zvbzcEgCrPTWgMamQ2409599;     zvbzcEgCrPTWgMamQ2409599 = zvbzcEgCrPTWgMamQ56799852;     zvbzcEgCrPTWgMamQ56799852 = zvbzcEgCrPTWgMamQ99006997;     zvbzcEgCrPTWgMamQ99006997 = zvbzcEgCrPTWgMamQ10828784;     zvbzcEgCrPTWgMamQ10828784 = zvbzcEgCrPTWgMamQ16206472;     zvbzcEgCrPTWgMamQ16206472 = zvbzcEgCrPTWgMamQ49998525;     zvbzcEgCrPTWgMamQ49998525 = zvbzcEgCrPTWgMamQ75058126;     zvbzcEgCrPTWgMamQ75058126 = zvbzcEgCrPTWgMamQ42295461;     zvbzcEgCrPTWgMamQ42295461 = zvbzcEgCrPTWgMamQ87713405;     zvbzcEgCrPTWgMamQ87713405 = zvbzcEgCrPTWgMamQ44420513;     zvbzcEgCrPTWgMamQ44420513 = zvbzcEgCrPTWgMamQ34071687;     zvbzcEgCrPTWgMamQ34071687 = zvbzcEgCrPTWgMamQ60833127;     zvbzcEgCrPTWgMamQ60833127 = zvbzcEgCrPTWgMamQ52637157;     zvbzcEgCrPTWgMamQ52637157 = zvbzcEgCrPTWgMamQ70398364;     zvbzcEgCrPTWgMamQ70398364 = zvbzcEgCrPTWgMamQ89312560;     zvbzcEgCrPTWgMamQ89312560 = zvbzcEgCrPTWgMamQ39306449;     zvbzcEgCrPTWgMamQ39306449 = zvbzcEgCrPTWgMamQ44341964;     zvbzcEgCrPTWgMamQ44341964 = zvbzcEgCrPTWgMamQ64321161;     zvbzcEgCrPTWgMamQ64321161 = zvbzcEgCrPTWgMamQ13967137;     zvbzcEgCrPTWgMamQ13967137 = zvbzcEgCrPTWgMamQ53348792;     zvbzcEgCrPTWgMamQ53348792 = zvbzcEgCrPTWgMamQ71743646;     zvbzcEgCrPTWgMamQ71743646 = zvbzcEgCrPTWgMamQ84768137;     zvbzcEgCrPTWgMamQ84768137 = zvbzcEgCrPTWgMamQ40835292;     zvbzcEgCrPTWgMamQ40835292 = zvbzcEgCrPTWgMamQ22462482;     zvbzcEgCrPTWgMamQ22462482 = zvbzcEgCrPTWgMamQ11270103;     zvbzcEgCrPTWgMamQ11270103 = zvbzcEgCrPTWgMamQ6428656;     zvbzcEgCrPTWgMamQ6428656 = zvbzcEgCrPTWgMamQ2058877;     zvbzcEgCrPTWgMamQ2058877 = zvbzcEgCrPTWgMamQ81416343;     zvbzcEgCrPTWgMamQ81416343 = zvbzcEgCrPTWgMamQ49473930;     zvbzcEgCrPTWgMamQ49473930 = zvbzcEgCrPTWgMamQ44983600;     zvbzcEgCrPTWgMamQ44983600 = zvbzcEgCrPTWgMamQ46933896;     zvbzcEgCrPTWgMamQ46933896 = zvbzcEgCrPTWgMamQ44525468;     zvbzcEgCrPTWgMamQ44525468 = zvbzcEgCrPTWgMamQ81734827;     zvbzcEgCrPTWgMamQ81734827 = zvbzcEgCrPTWgMamQ30886148;     zvbzcEgCrPTWgMamQ30886148 = zvbzcEgCrPTWgMamQ79786986;     zvbzcEgCrPTWgMamQ79786986 = zvbzcEgCrPTWgMamQ14740700;     zvbzcEgCrPTWgMamQ14740700 = zvbzcEgCrPTWgMamQ12951681;     zvbzcEgCrPTWgMamQ12951681 = zvbzcEgCrPTWgMamQ18348677;     zvbzcEgCrPTWgMamQ18348677 = zvbzcEgCrPTWgMamQ18753997;     zvbzcEgCrPTWgMamQ18753997 = zvbzcEgCrPTWgMamQ17367348;     zvbzcEgCrPTWgMamQ17367348 = zvbzcEgCrPTWgMamQ52918053;     zvbzcEgCrPTWgMamQ52918053 = zvbzcEgCrPTWgMamQ28024106;     zvbzcEgCrPTWgMamQ28024106 = zvbzcEgCrPTWgMamQ70158591;     zvbzcEgCrPTWgMamQ70158591 = zvbzcEgCrPTWgMamQ41978368;     zvbzcEgCrPTWgMamQ41978368 = zvbzcEgCrPTWgMamQ71991229;     zvbzcEgCrPTWgMamQ71991229 = zvbzcEgCrPTWgMamQ79856601;     zvbzcEgCrPTWgMamQ79856601 = zvbzcEgCrPTWgMamQ91474632;     zvbzcEgCrPTWgMamQ91474632 = zvbzcEgCrPTWgMamQ7088632;     zvbzcEgCrPTWgMamQ7088632 = zvbzcEgCrPTWgMamQ48392471;     zvbzcEgCrPTWgMamQ48392471 = zvbzcEgCrPTWgMamQ24932491;     zvbzcEgCrPTWgMamQ24932491 = zvbzcEgCrPTWgMamQ40857592;     zvbzcEgCrPTWgMamQ40857592 = zvbzcEgCrPTWgMamQ36590225;     zvbzcEgCrPTWgMamQ36590225 = zvbzcEgCrPTWgMamQ43158288;     zvbzcEgCrPTWgMamQ43158288 = zvbzcEgCrPTWgMamQ2840757;     zvbzcEgCrPTWgMamQ2840757 = zvbzcEgCrPTWgMamQ54458988;     zvbzcEgCrPTWgMamQ54458988 = zvbzcEgCrPTWgMamQ17641462;     zvbzcEgCrPTWgMamQ17641462 = zvbzcEgCrPTWgMamQ15964560;     zvbzcEgCrPTWgMamQ15964560 = zvbzcEgCrPTWgMamQ76544515;     zvbzcEgCrPTWgMamQ76544515 = zvbzcEgCrPTWgMamQ99558680;     zvbzcEgCrPTWgMamQ99558680 = zvbzcEgCrPTWgMamQ9777816;     zvbzcEgCrPTWgMamQ9777816 = zvbzcEgCrPTWgMamQ47939649;     zvbzcEgCrPTWgMamQ47939649 = zvbzcEgCrPTWgMamQ93641782;     zvbzcEgCrPTWgMamQ93641782 = zvbzcEgCrPTWgMamQ92821530;     zvbzcEgCrPTWgMamQ92821530 = zvbzcEgCrPTWgMamQ42729805;     zvbzcEgCrPTWgMamQ42729805 = zvbzcEgCrPTWgMamQ97486617;     zvbzcEgCrPTWgMamQ97486617 = zvbzcEgCrPTWgMamQ89546219;     zvbzcEgCrPTWgMamQ89546219 = zvbzcEgCrPTWgMamQ79098299;     zvbzcEgCrPTWgMamQ79098299 = zvbzcEgCrPTWgMamQ21751010;     zvbzcEgCrPTWgMamQ21751010 = zvbzcEgCrPTWgMamQ90611377;     zvbzcEgCrPTWgMamQ90611377 = zvbzcEgCrPTWgMamQ74571860;     zvbzcEgCrPTWgMamQ74571860 = zvbzcEgCrPTWgMamQ26354768;     zvbzcEgCrPTWgMamQ26354768 = zvbzcEgCrPTWgMamQ25993288;     zvbzcEgCrPTWgMamQ25993288 = zvbzcEgCrPTWgMamQ19121375;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void BSweHoxhfoffYlCI46837471() {     double CgXdMqulJnWCeNdUU54580343 = -965038092;    double CgXdMqulJnWCeNdUU26984949 = -917953926;    double CgXdMqulJnWCeNdUU67569632 = -660928942;    double CgXdMqulJnWCeNdUU20068636 = -340236127;    double CgXdMqulJnWCeNdUU57936162 = -944893842;    double CgXdMqulJnWCeNdUU74147248 = -174921729;    double CgXdMqulJnWCeNdUU67746189 = -289541960;    double CgXdMqulJnWCeNdUU44328146 = -298685625;    double CgXdMqulJnWCeNdUU73058157 = -223837152;    double CgXdMqulJnWCeNdUU26861390 = -720014302;    double CgXdMqulJnWCeNdUU67359194 = -438745130;    double CgXdMqulJnWCeNdUU35528616 = -594164946;    double CgXdMqulJnWCeNdUU5600519 = -406626416;    double CgXdMqulJnWCeNdUU81117042 = -862583030;    double CgXdMqulJnWCeNdUU28514084 = -388386043;    double CgXdMqulJnWCeNdUU41413297 = -867976824;    double CgXdMqulJnWCeNdUU42234817 = -686275792;    double CgXdMqulJnWCeNdUU5823108 = -389827290;    double CgXdMqulJnWCeNdUU41240059 = -230025877;    double CgXdMqulJnWCeNdUU37949008 = 73917755;    double CgXdMqulJnWCeNdUU90573858 = -450584740;    double CgXdMqulJnWCeNdUU41126556 = -987898309;    double CgXdMqulJnWCeNdUU34764320 = -973927496;    double CgXdMqulJnWCeNdUU78992501 = -858785634;    double CgXdMqulJnWCeNdUU8929337 = -636756869;    double CgXdMqulJnWCeNdUU67823597 = -386860161;    double CgXdMqulJnWCeNdUU16610123 = -316874885;    double CgXdMqulJnWCeNdUU66931060 = -262877059;    double CgXdMqulJnWCeNdUU49077813 = -465753482;    double CgXdMqulJnWCeNdUU62333262 = -267547321;    double CgXdMqulJnWCeNdUU17590229 = -270730698;    double CgXdMqulJnWCeNdUU48651802 = 39055304;    double CgXdMqulJnWCeNdUU28424482 = -63241026;    double CgXdMqulJnWCeNdUU79746951 = -901186474;    double CgXdMqulJnWCeNdUU40866580 = 50153412;    double CgXdMqulJnWCeNdUU93856304 = -702421816;    double CgXdMqulJnWCeNdUU37125222 = -838185853;    double CgXdMqulJnWCeNdUU95635916 = -864521657;    double CgXdMqulJnWCeNdUU64462248 = -617525208;    double CgXdMqulJnWCeNdUU25104660 = -277078557;    double CgXdMqulJnWCeNdUU86088765 = -482393022;    double CgXdMqulJnWCeNdUU59763491 = -944948319;    double CgXdMqulJnWCeNdUU78695938 = -610119066;    double CgXdMqulJnWCeNdUU79437787 = 78331031;    double CgXdMqulJnWCeNdUU44358423 = -159714153;    double CgXdMqulJnWCeNdUU10282490 = -287842620;    double CgXdMqulJnWCeNdUU33312301 = 40655409;    double CgXdMqulJnWCeNdUU71075104 = -207954184;    double CgXdMqulJnWCeNdUU94636312 = -793767445;    double CgXdMqulJnWCeNdUU9297727 = -388780306;    double CgXdMqulJnWCeNdUU21606016 = -375714217;    double CgXdMqulJnWCeNdUU5947955 = -900527973;    double CgXdMqulJnWCeNdUU79891251 = 77707363;    double CgXdMqulJnWCeNdUU22420502 = -202084871;    double CgXdMqulJnWCeNdUU56263613 = -167253929;    double CgXdMqulJnWCeNdUU13453788 = -977139784;    double CgXdMqulJnWCeNdUU92220628 = -944026431;    double CgXdMqulJnWCeNdUU88577131 = -802143309;    double CgXdMqulJnWCeNdUU11139300 = -703479258;    double CgXdMqulJnWCeNdUU90112565 = -458033681;    double CgXdMqulJnWCeNdUU57537126 = -858046844;    double CgXdMqulJnWCeNdUU815130 = 73335099;    double CgXdMqulJnWCeNdUU95250332 = -832932143;    double CgXdMqulJnWCeNdUU10724895 = -956289832;    double CgXdMqulJnWCeNdUU9271162 = -349283604;    double CgXdMqulJnWCeNdUU18707393 = -377800435;    double CgXdMqulJnWCeNdUU7104134 = -430923920;    double CgXdMqulJnWCeNdUU25853567 = -505439942;    double CgXdMqulJnWCeNdUU40250462 = -812736442;    double CgXdMqulJnWCeNdUU34657779 = -685964227;    double CgXdMqulJnWCeNdUU4288076 = 70209028;    double CgXdMqulJnWCeNdUU46598900 = -821754136;    double CgXdMqulJnWCeNdUU41360860 = -772302082;    double CgXdMqulJnWCeNdUU16135399 = -952947321;    double CgXdMqulJnWCeNdUU51860243 = -443689223;    double CgXdMqulJnWCeNdUU30810367 = -505636421;    double CgXdMqulJnWCeNdUU62430617 = -277779243;    double CgXdMqulJnWCeNdUU55326533 = -952258527;    double CgXdMqulJnWCeNdUU34634078 = -599071481;    double CgXdMqulJnWCeNdUU98646847 = -248914250;    double CgXdMqulJnWCeNdUU34511296 = -327515570;    double CgXdMqulJnWCeNdUU45535019 = -8920702;    double CgXdMqulJnWCeNdUU72294747 = -469109615;    double CgXdMqulJnWCeNdUU39780087 = 23026823;    double CgXdMqulJnWCeNdUU40727247 = -891833104;    double CgXdMqulJnWCeNdUU11642275 = -370202725;    double CgXdMqulJnWCeNdUU68760550 = 61347940;    double CgXdMqulJnWCeNdUU6003981 = -861156156;    double CgXdMqulJnWCeNdUU23483339 = -633932545;    double CgXdMqulJnWCeNdUU27412793 = 27293195;    double CgXdMqulJnWCeNdUU1635676 = -758395386;    double CgXdMqulJnWCeNdUU48548090 = 63957455;    double CgXdMqulJnWCeNdUU84496617 = -61042399;    double CgXdMqulJnWCeNdUU74349683 = -59491528;    double CgXdMqulJnWCeNdUU67567534 = -419031713;    double CgXdMqulJnWCeNdUU85273635 = -455728122;    double CgXdMqulJnWCeNdUU64513158 = -12016176;    double CgXdMqulJnWCeNdUU67971044 = -653829235;    double CgXdMqulJnWCeNdUU70166626 = -572385365;    double CgXdMqulJnWCeNdUU25651030 = -965038092;     CgXdMqulJnWCeNdUU54580343 = CgXdMqulJnWCeNdUU26984949;     CgXdMqulJnWCeNdUU26984949 = CgXdMqulJnWCeNdUU67569632;     CgXdMqulJnWCeNdUU67569632 = CgXdMqulJnWCeNdUU20068636;     CgXdMqulJnWCeNdUU20068636 = CgXdMqulJnWCeNdUU57936162;     CgXdMqulJnWCeNdUU57936162 = CgXdMqulJnWCeNdUU74147248;     CgXdMqulJnWCeNdUU74147248 = CgXdMqulJnWCeNdUU67746189;     CgXdMqulJnWCeNdUU67746189 = CgXdMqulJnWCeNdUU44328146;     CgXdMqulJnWCeNdUU44328146 = CgXdMqulJnWCeNdUU73058157;     CgXdMqulJnWCeNdUU73058157 = CgXdMqulJnWCeNdUU26861390;     CgXdMqulJnWCeNdUU26861390 = CgXdMqulJnWCeNdUU67359194;     CgXdMqulJnWCeNdUU67359194 = CgXdMqulJnWCeNdUU35528616;     CgXdMqulJnWCeNdUU35528616 = CgXdMqulJnWCeNdUU5600519;     CgXdMqulJnWCeNdUU5600519 = CgXdMqulJnWCeNdUU81117042;     CgXdMqulJnWCeNdUU81117042 = CgXdMqulJnWCeNdUU28514084;     CgXdMqulJnWCeNdUU28514084 = CgXdMqulJnWCeNdUU41413297;     CgXdMqulJnWCeNdUU41413297 = CgXdMqulJnWCeNdUU42234817;     CgXdMqulJnWCeNdUU42234817 = CgXdMqulJnWCeNdUU5823108;     CgXdMqulJnWCeNdUU5823108 = CgXdMqulJnWCeNdUU41240059;     CgXdMqulJnWCeNdUU41240059 = CgXdMqulJnWCeNdUU37949008;     CgXdMqulJnWCeNdUU37949008 = CgXdMqulJnWCeNdUU90573858;     CgXdMqulJnWCeNdUU90573858 = CgXdMqulJnWCeNdUU41126556;     CgXdMqulJnWCeNdUU41126556 = CgXdMqulJnWCeNdUU34764320;     CgXdMqulJnWCeNdUU34764320 = CgXdMqulJnWCeNdUU78992501;     CgXdMqulJnWCeNdUU78992501 = CgXdMqulJnWCeNdUU8929337;     CgXdMqulJnWCeNdUU8929337 = CgXdMqulJnWCeNdUU67823597;     CgXdMqulJnWCeNdUU67823597 = CgXdMqulJnWCeNdUU16610123;     CgXdMqulJnWCeNdUU16610123 = CgXdMqulJnWCeNdUU66931060;     CgXdMqulJnWCeNdUU66931060 = CgXdMqulJnWCeNdUU49077813;     CgXdMqulJnWCeNdUU49077813 = CgXdMqulJnWCeNdUU62333262;     CgXdMqulJnWCeNdUU62333262 = CgXdMqulJnWCeNdUU17590229;     CgXdMqulJnWCeNdUU17590229 = CgXdMqulJnWCeNdUU48651802;     CgXdMqulJnWCeNdUU48651802 = CgXdMqulJnWCeNdUU28424482;     CgXdMqulJnWCeNdUU28424482 = CgXdMqulJnWCeNdUU79746951;     CgXdMqulJnWCeNdUU79746951 = CgXdMqulJnWCeNdUU40866580;     CgXdMqulJnWCeNdUU40866580 = CgXdMqulJnWCeNdUU93856304;     CgXdMqulJnWCeNdUU93856304 = CgXdMqulJnWCeNdUU37125222;     CgXdMqulJnWCeNdUU37125222 = CgXdMqulJnWCeNdUU95635916;     CgXdMqulJnWCeNdUU95635916 = CgXdMqulJnWCeNdUU64462248;     CgXdMqulJnWCeNdUU64462248 = CgXdMqulJnWCeNdUU25104660;     CgXdMqulJnWCeNdUU25104660 = CgXdMqulJnWCeNdUU86088765;     CgXdMqulJnWCeNdUU86088765 = CgXdMqulJnWCeNdUU59763491;     CgXdMqulJnWCeNdUU59763491 = CgXdMqulJnWCeNdUU78695938;     CgXdMqulJnWCeNdUU78695938 = CgXdMqulJnWCeNdUU79437787;     CgXdMqulJnWCeNdUU79437787 = CgXdMqulJnWCeNdUU44358423;     CgXdMqulJnWCeNdUU44358423 = CgXdMqulJnWCeNdUU10282490;     CgXdMqulJnWCeNdUU10282490 = CgXdMqulJnWCeNdUU33312301;     CgXdMqulJnWCeNdUU33312301 = CgXdMqulJnWCeNdUU71075104;     CgXdMqulJnWCeNdUU71075104 = CgXdMqulJnWCeNdUU94636312;     CgXdMqulJnWCeNdUU94636312 = CgXdMqulJnWCeNdUU9297727;     CgXdMqulJnWCeNdUU9297727 = CgXdMqulJnWCeNdUU21606016;     CgXdMqulJnWCeNdUU21606016 = CgXdMqulJnWCeNdUU5947955;     CgXdMqulJnWCeNdUU5947955 = CgXdMqulJnWCeNdUU79891251;     CgXdMqulJnWCeNdUU79891251 = CgXdMqulJnWCeNdUU22420502;     CgXdMqulJnWCeNdUU22420502 = CgXdMqulJnWCeNdUU56263613;     CgXdMqulJnWCeNdUU56263613 = CgXdMqulJnWCeNdUU13453788;     CgXdMqulJnWCeNdUU13453788 = CgXdMqulJnWCeNdUU92220628;     CgXdMqulJnWCeNdUU92220628 = CgXdMqulJnWCeNdUU88577131;     CgXdMqulJnWCeNdUU88577131 = CgXdMqulJnWCeNdUU11139300;     CgXdMqulJnWCeNdUU11139300 = CgXdMqulJnWCeNdUU90112565;     CgXdMqulJnWCeNdUU90112565 = CgXdMqulJnWCeNdUU57537126;     CgXdMqulJnWCeNdUU57537126 = CgXdMqulJnWCeNdUU815130;     CgXdMqulJnWCeNdUU815130 = CgXdMqulJnWCeNdUU95250332;     CgXdMqulJnWCeNdUU95250332 = CgXdMqulJnWCeNdUU10724895;     CgXdMqulJnWCeNdUU10724895 = CgXdMqulJnWCeNdUU9271162;     CgXdMqulJnWCeNdUU9271162 = CgXdMqulJnWCeNdUU18707393;     CgXdMqulJnWCeNdUU18707393 = CgXdMqulJnWCeNdUU7104134;     CgXdMqulJnWCeNdUU7104134 = CgXdMqulJnWCeNdUU25853567;     CgXdMqulJnWCeNdUU25853567 = CgXdMqulJnWCeNdUU40250462;     CgXdMqulJnWCeNdUU40250462 = CgXdMqulJnWCeNdUU34657779;     CgXdMqulJnWCeNdUU34657779 = CgXdMqulJnWCeNdUU4288076;     CgXdMqulJnWCeNdUU4288076 = CgXdMqulJnWCeNdUU46598900;     CgXdMqulJnWCeNdUU46598900 = CgXdMqulJnWCeNdUU41360860;     CgXdMqulJnWCeNdUU41360860 = CgXdMqulJnWCeNdUU16135399;     CgXdMqulJnWCeNdUU16135399 = CgXdMqulJnWCeNdUU51860243;     CgXdMqulJnWCeNdUU51860243 = CgXdMqulJnWCeNdUU30810367;     CgXdMqulJnWCeNdUU30810367 = CgXdMqulJnWCeNdUU62430617;     CgXdMqulJnWCeNdUU62430617 = CgXdMqulJnWCeNdUU55326533;     CgXdMqulJnWCeNdUU55326533 = CgXdMqulJnWCeNdUU34634078;     CgXdMqulJnWCeNdUU34634078 = CgXdMqulJnWCeNdUU98646847;     CgXdMqulJnWCeNdUU98646847 = CgXdMqulJnWCeNdUU34511296;     CgXdMqulJnWCeNdUU34511296 = CgXdMqulJnWCeNdUU45535019;     CgXdMqulJnWCeNdUU45535019 = CgXdMqulJnWCeNdUU72294747;     CgXdMqulJnWCeNdUU72294747 = CgXdMqulJnWCeNdUU39780087;     CgXdMqulJnWCeNdUU39780087 = CgXdMqulJnWCeNdUU40727247;     CgXdMqulJnWCeNdUU40727247 = CgXdMqulJnWCeNdUU11642275;     CgXdMqulJnWCeNdUU11642275 = CgXdMqulJnWCeNdUU68760550;     CgXdMqulJnWCeNdUU68760550 = CgXdMqulJnWCeNdUU6003981;     CgXdMqulJnWCeNdUU6003981 = CgXdMqulJnWCeNdUU23483339;     CgXdMqulJnWCeNdUU23483339 = CgXdMqulJnWCeNdUU27412793;     CgXdMqulJnWCeNdUU27412793 = CgXdMqulJnWCeNdUU1635676;     CgXdMqulJnWCeNdUU1635676 = CgXdMqulJnWCeNdUU48548090;     CgXdMqulJnWCeNdUU48548090 = CgXdMqulJnWCeNdUU84496617;     CgXdMqulJnWCeNdUU84496617 = CgXdMqulJnWCeNdUU74349683;     CgXdMqulJnWCeNdUU74349683 = CgXdMqulJnWCeNdUU67567534;     CgXdMqulJnWCeNdUU67567534 = CgXdMqulJnWCeNdUU85273635;     CgXdMqulJnWCeNdUU85273635 = CgXdMqulJnWCeNdUU64513158;     CgXdMqulJnWCeNdUU64513158 = CgXdMqulJnWCeNdUU67971044;     CgXdMqulJnWCeNdUU67971044 = CgXdMqulJnWCeNdUU70166626;     CgXdMqulJnWCeNdUU70166626 = CgXdMqulJnWCeNdUU25651030;     CgXdMqulJnWCeNdUU25651030 = CgXdMqulJnWCeNdUU54580343;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void oCHuueEKxInVGEQt61886539() {     double bfUjkWsJhQssjlGXW60697450 = 23060019;    double bfUjkWsJhQssjlGXW70358228 = -627553707;    double bfUjkWsJhQssjlGXW83611815 = -811761863;    double bfUjkWsJhQssjlGXW47143417 = -530984758;    double bfUjkWsJhQssjlGXW26831651 = -365543713;    double bfUjkWsJhQssjlGXW65564451 = -439417964;    double bfUjkWsJhQssjlGXW28235155 = -428506937;    double bfUjkWsJhQssjlGXW94088036 = -585559538;    double bfUjkWsJhQssjlGXW37700653 = 8130182;    double bfUjkWsJhQssjlGXW68091049 = 51010221;    double bfUjkWsJhQssjlGXW27469485 = -316182717;    double bfUjkWsJhQssjlGXW67351658 = 61428852;    double bfUjkWsJhQssjlGXW91809637 = -846517505;    double bfUjkWsJhQssjlGXW64260719 = -294821598;    double bfUjkWsJhQssjlGXW24256510 = 21151791;    double bfUjkWsJhQssjlGXW82918691 = -840976367;    double bfUjkWsJhQssjlGXW36070820 = -202820905;    double bfUjkWsJhQssjlGXW2876261 = -55349933;    double bfUjkWsJhQssjlGXW3069901 = -635794974;    double bfUjkWsJhQssjlGXW18499173 = -282985164;    double bfUjkWsJhQssjlGXW79127051 = -306072195;    double bfUjkWsJhQssjlGXW27647580 = -895828025;    double bfUjkWsJhQssjlGXW70355111 = -720444983;    double bfUjkWsJhQssjlGXW62984875 = 18909379;    double bfUjkWsJhQssjlGXW52628987 = 90047098;    double bfUjkWsJhQssjlGXW92545272 = -265515485;    double bfUjkWsJhQssjlGXW15069901 = -217537088;    double bfUjkWsJhQssjlGXW80038185 = -328806013;    double bfUjkWsJhQssjlGXW15210069 = -688136187;    double bfUjkWsJhQssjlGXW30948900 = -85018188;    double bfUjkWsJhQssjlGXW9832778 = -184051636;    double bfUjkWsJhQssjlGXW46262424 = -770391512;    double bfUjkWsJhQssjlGXW38135431 = 44145603;    double bfUjkWsJhQssjlGXW74577516 = -652728239;    double bfUjkWsJhQssjlGXW2341232 = -588579694;    double bfUjkWsJhQssjlGXW62582218 = -916447924;    double bfUjkWsJhQssjlGXW18387157 = -791821722;    double bfUjkWsJhQssjlGXW29726129 = -636027667;    double bfUjkWsJhQssjlGXW38072502 = -611448705;    double bfUjkWsJhQssjlGXW12606598 = -750517780;    double bfUjkWsJhQssjlGXW91757989 = -547666916;    double bfUjkWsJhQssjlGXW91913947 = -115090597;    double bfUjkWsJhQssjlGXW56762917 = -690923650;    double bfUjkWsJhQssjlGXW66845527 = 58988093;    double bfUjkWsJhQssjlGXW56310888 = -921612307;    double bfUjkWsJhQssjlGXW22221685 = -273058736;    double bfUjkWsJhQssjlGXW51090875 = -78120129;    double bfUjkWsJhQssjlGXW79032045 = -217849716;    double bfUjkWsJhQssjlGXW4312042 = -879504433;    double bfUjkWsJhQssjlGXW22209379 = -475791114;    double bfUjkWsJhQssjlGXW57306694 = -218007888;    double bfUjkWsJhQssjlGXW16757674 = -701254293;    double bfUjkWsJhQssjlGXW28139709 = -814082224;    double bfUjkWsJhQssjlGXW18389382 = -522555016;    double bfUjkWsJhQssjlGXW82759487 = -867785712;    double bfUjkWsJhQssjlGXW33049871 = -81111956;    double bfUjkWsJhQssjlGXW3117 = -907108724;    double bfUjkWsJhQssjlGXW20626941 = -730671242;    double bfUjkWsJhQssjlGXW94514429 = -521031856;    double bfUjkWsJhQssjlGXW34286378 = -28229;    double bfUjkWsJhQssjlGXW50494550 = -121880876;    double bfUjkWsJhQssjlGXW48196970 = 299075;    double bfUjkWsJhQssjlGXW78877968 = -897423352;    double bfUjkWsJhQssjlGXW6751753 = -906851630;    double bfUjkWsJhQssjlGXW58258271 = -764938144;    double bfUjkWsJhQssjlGXW81207061 = -545791206;    double bfUjkWsJhQssjlGXW29216228 = -982716751;    double bfUjkWsJhQssjlGXW17232121 = -93789266;    double bfUjkWsJhQssjlGXW61919487 = -706241905;    double bfUjkWsJhQssjlGXW61674292 = -62400285;    double bfUjkWsJhQssjlGXW64531535 = 50845355;    double bfUjkWsJhQssjlGXW6344691 = -566793239;    double bfUjkWsJhQssjlGXW64803759 = -443901228;    double bfUjkWsJhQssjlGXW90463303 = -885277195;    double bfUjkWsJhQssjlGXW26741184 = -735318248;    double bfUjkWsJhQssjlGXW87213103 = -90981599;    double bfUjkWsJhQssjlGXW70884663 = -104904376;    double bfUjkWsJhQssjlGXW3509584 = -679433076;    double bfUjkWsJhQssjlGXW6673987 = -59478314;    double bfUjkWsJhQssjlGXW30407303 = -636894167;    double bfUjkWsJhQssjlGXW41454397 = -87395356;    double bfUjkWsJhQssjlGXW36037856 = -999687373;    double bfUjkWsJhQssjlGXW75726143 = -449301580;    double bfUjkWsJhQssjlGXW93000689 = -112345073;    double bfUjkWsJhQssjlGXW73642205 = -867010300;    double bfUjkWsJhQssjlGXW93075104 = -482797343;    double bfUjkWsJhQssjlGXW18122715 = -956309288;    double bfUjkWsJhQssjlGXW19746049 = -433299381;    double bfUjkWsJhQssjlGXW91818028 = -784942528;    double bfUjkWsJhQssjlGXW69291361 = -407467738;    double bfUjkWsJhQssjlGXW62579101 = 90660800;    double bfUjkWsJhQssjlGXW97760215 = 38849519;    double bfUjkWsJhQssjlGXW35211699 = -14995812;    double bfUjkWsJhQssjlGXW3786124 = -511420477;    double bfUjkWsJhQssjlGXW62112047 = -528636904;    double bfUjkWsJhQssjlGXW43561019 = -447965992;    double bfUjkWsJhQssjlGXW13035980 = -217667246;    double bfUjkWsJhQssjlGXW50011164 = -784072020;    double bfUjkWsJhQssjlGXW8587257 = -176073764;    double bfUjkWsJhQssjlGXW75103827 = 23060019;     bfUjkWsJhQssjlGXW60697450 = bfUjkWsJhQssjlGXW70358228;     bfUjkWsJhQssjlGXW70358228 = bfUjkWsJhQssjlGXW83611815;     bfUjkWsJhQssjlGXW83611815 = bfUjkWsJhQssjlGXW47143417;     bfUjkWsJhQssjlGXW47143417 = bfUjkWsJhQssjlGXW26831651;     bfUjkWsJhQssjlGXW26831651 = bfUjkWsJhQssjlGXW65564451;     bfUjkWsJhQssjlGXW65564451 = bfUjkWsJhQssjlGXW28235155;     bfUjkWsJhQssjlGXW28235155 = bfUjkWsJhQssjlGXW94088036;     bfUjkWsJhQssjlGXW94088036 = bfUjkWsJhQssjlGXW37700653;     bfUjkWsJhQssjlGXW37700653 = bfUjkWsJhQssjlGXW68091049;     bfUjkWsJhQssjlGXW68091049 = bfUjkWsJhQssjlGXW27469485;     bfUjkWsJhQssjlGXW27469485 = bfUjkWsJhQssjlGXW67351658;     bfUjkWsJhQssjlGXW67351658 = bfUjkWsJhQssjlGXW91809637;     bfUjkWsJhQssjlGXW91809637 = bfUjkWsJhQssjlGXW64260719;     bfUjkWsJhQssjlGXW64260719 = bfUjkWsJhQssjlGXW24256510;     bfUjkWsJhQssjlGXW24256510 = bfUjkWsJhQssjlGXW82918691;     bfUjkWsJhQssjlGXW82918691 = bfUjkWsJhQssjlGXW36070820;     bfUjkWsJhQssjlGXW36070820 = bfUjkWsJhQssjlGXW2876261;     bfUjkWsJhQssjlGXW2876261 = bfUjkWsJhQssjlGXW3069901;     bfUjkWsJhQssjlGXW3069901 = bfUjkWsJhQssjlGXW18499173;     bfUjkWsJhQssjlGXW18499173 = bfUjkWsJhQssjlGXW79127051;     bfUjkWsJhQssjlGXW79127051 = bfUjkWsJhQssjlGXW27647580;     bfUjkWsJhQssjlGXW27647580 = bfUjkWsJhQssjlGXW70355111;     bfUjkWsJhQssjlGXW70355111 = bfUjkWsJhQssjlGXW62984875;     bfUjkWsJhQssjlGXW62984875 = bfUjkWsJhQssjlGXW52628987;     bfUjkWsJhQssjlGXW52628987 = bfUjkWsJhQssjlGXW92545272;     bfUjkWsJhQssjlGXW92545272 = bfUjkWsJhQssjlGXW15069901;     bfUjkWsJhQssjlGXW15069901 = bfUjkWsJhQssjlGXW80038185;     bfUjkWsJhQssjlGXW80038185 = bfUjkWsJhQssjlGXW15210069;     bfUjkWsJhQssjlGXW15210069 = bfUjkWsJhQssjlGXW30948900;     bfUjkWsJhQssjlGXW30948900 = bfUjkWsJhQssjlGXW9832778;     bfUjkWsJhQssjlGXW9832778 = bfUjkWsJhQssjlGXW46262424;     bfUjkWsJhQssjlGXW46262424 = bfUjkWsJhQssjlGXW38135431;     bfUjkWsJhQssjlGXW38135431 = bfUjkWsJhQssjlGXW74577516;     bfUjkWsJhQssjlGXW74577516 = bfUjkWsJhQssjlGXW2341232;     bfUjkWsJhQssjlGXW2341232 = bfUjkWsJhQssjlGXW62582218;     bfUjkWsJhQssjlGXW62582218 = bfUjkWsJhQssjlGXW18387157;     bfUjkWsJhQssjlGXW18387157 = bfUjkWsJhQssjlGXW29726129;     bfUjkWsJhQssjlGXW29726129 = bfUjkWsJhQssjlGXW38072502;     bfUjkWsJhQssjlGXW38072502 = bfUjkWsJhQssjlGXW12606598;     bfUjkWsJhQssjlGXW12606598 = bfUjkWsJhQssjlGXW91757989;     bfUjkWsJhQssjlGXW91757989 = bfUjkWsJhQssjlGXW91913947;     bfUjkWsJhQssjlGXW91913947 = bfUjkWsJhQssjlGXW56762917;     bfUjkWsJhQssjlGXW56762917 = bfUjkWsJhQssjlGXW66845527;     bfUjkWsJhQssjlGXW66845527 = bfUjkWsJhQssjlGXW56310888;     bfUjkWsJhQssjlGXW56310888 = bfUjkWsJhQssjlGXW22221685;     bfUjkWsJhQssjlGXW22221685 = bfUjkWsJhQssjlGXW51090875;     bfUjkWsJhQssjlGXW51090875 = bfUjkWsJhQssjlGXW79032045;     bfUjkWsJhQssjlGXW79032045 = bfUjkWsJhQssjlGXW4312042;     bfUjkWsJhQssjlGXW4312042 = bfUjkWsJhQssjlGXW22209379;     bfUjkWsJhQssjlGXW22209379 = bfUjkWsJhQssjlGXW57306694;     bfUjkWsJhQssjlGXW57306694 = bfUjkWsJhQssjlGXW16757674;     bfUjkWsJhQssjlGXW16757674 = bfUjkWsJhQssjlGXW28139709;     bfUjkWsJhQssjlGXW28139709 = bfUjkWsJhQssjlGXW18389382;     bfUjkWsJhQssjlGXW18389382 = bfUjkWsJhQssjlGXW82759487;     bfUjkWsJhQssjlGXW82759487 = bfUjkWsJhQssjlGXW33049871;     bfUjkWsJhQssjlGXW33049871 = bfUjkWsJhQssjlGXW3117;     bfUjkWsJhQssjlGXW3117 = bfUjkWsJhQssjlGXW20626941;     bfUjkWsJhQssjlGXW20626941 = bfUjkWsJhQssjlGXW94514429;     bfUjkWsJhQssjlGXW94514429 = bfUjkWsJhQssjlGXW34286378;     bfUjkWsJhQssjlGXW34286378 = bfUjkWsJhQssjlGXW50494550;     bfUjkWsJhQssjlGXW50494550 = bfUjkWsJhQssjlGXW48196970;     bfUjkWsJhQssjlGXW48196970 = bfUjkWsJhQssjlGXW78877968;     bfUjkWsJhQssjlGXW78877968 = bfUjkWsJhQssjlGXW6751753;     bfUjkWsJhQssjlGXW6751753 = bfUjkWsJhQssjlGXW58258271;     bfUjkWsJhQssjlGXW58258271 = bfUjkWsJhQssjlGXW81207061;     bfUjkWsJhQssjlGXW81207061 = bfUjkWsJhQssjlGXW29216228;     bfUjkWsJhQssjlGXW29216228 = bfUjkWsJhQssjlGXW17232121;     bfUjkWsJhQssjlGXW17232121 = bfUjkWsJhQssjlGXW61919487;     bfUjkWsJhQssjlGXW61919487 = bfUjkWsJhQssjlGXW61674292;     bfUjkWsJhQssjlGXW61674292 = bfUjkWsJhQssjlGXW64531535;     bfUjkWsJhQssjlGXW64531535 = bfUjkWsJhQssjlGXW6344691;     bfUjkWsJhQssjlGXW6344691 = bfUjkWsJhQssjlGXW64803759;     bfUjkWsJhQssjlGXW64803759 = bfUjkWsJhQssjlGXW90463303;     bfUjkWsJhQssjlGXW90463303 = bfUjkWsJhQssjlGXW26741184;     bfUjkWsJhQssjlGXW26741184 = bfUjkWsJhQssjlGXW87213103;     bfUjkWsJhQssjlGXW87213103 = bfUjkWsJhQssjlGXW70884663;     bfUjkWsJhQssjlGXW70884663 = bfUjkWsJhQssjlGXW3509584;     bfUjkWsJhQssjlGXW3509584 = bfUjkWsJhQssjlGXW6673987;     bfUjkWsJhQssjlGXW6673987 = bfUjkWsJhQssjlGXW30407303;     bfUjkWsJhQssjlGXW30407303 = bfUjkWsJhQssjlGXW41454397;     bfUjkWsJhQssjlGXW41454397 = bfUjkWsJhQssjlGXW36037856;     bfUjkWsJhQssjlGXW36037856 = bfUjkWsJhQssjlGXW75726143;     bfUjkWsJhQssjlGXW75726143 = bfUjkWsJhQssjlGXW93000689;     bfUjkWsJhQssjlGXW93000689 = bfUjkWsJhQssjlGXW73642205;     bfUjkWsJhQssjlGXW73642205 = bfUjkWsJhQssjlGXW93075104;     bfUjkWsJhQssjlGXW93075104 = bfUjkWsJhQssjlGXW18122715;     bfUjkWsJhQssjlGXW18122715 = bfUjkWsJhQssjlGXW19746049;     bfUjkWsJhQssjlGXW19746049 = bfUjkWsJhQssjlGXW91818028;     bfUjkWsJhQssjlGXW91818028 = bfUjkWsJhQssjlGXW69291361;     bfUjkWsJhQssjlGXW69291361 = bfUjkWsJhQssjlGXW62579101;     bfUjkWsJhQssjlGXW62579101 = bfUjkWsJhQssjlGXW97760215;     bfUjkWsJhQssjlGXW97760215 = bfUjkWsJhQssjlGXW35211699;     bfUjkWsJhQssjlGXW35211699 = bfUjkWsJhQssjlGXW3786124;     bfUjkWsJhQssjlGXW3786124 = bfUjkWsJhQssjlGXW62112047;     bfUjkWsJhQssjlGXW62112047 = bfUjkWsJhQssjlGXW43561019;     bfUjkWsJhQssjlGXW43561019 = bfUjkWsJhQssjlGXW13035980;     bfUjkWsJhQssjlGXW13035980 = bfUjkWsJhQssjlGXW50011164;     bfUjkWsJhQssjlGXW50011164 = bfUjkWsJhQssjlGXW8587257;     bfUjkWsJhQssjlGXW8587257 = bfUjkWsJhQssjlGXW75103827;     bfUjkWsJhQssjlGXW75103827 = bfUjkWsJhQssjlGXW60697450;}
// Junk Finished
