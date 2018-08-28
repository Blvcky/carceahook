// dear imgui, v1.54 WIP
// (demo code)

// Message to the person tempted to delete this file when integrating ImGui into their code base:
// Don't do it! Do NOT remove this file from your project! It is useful reference code that you and other users will want to refer to.
// Everything in this file will be stripped out by the linker if you don't call ImGui::ShowDemoWindow().
// During development, you can call ImGui::ShowDemoWindow() in your code to learn about various features of ImGui. Have it wired in a debug menu!
// Removing this file from your project is hindering access to documentation for everyone in your team, likely leading you to poorer usage of the library.
// Note that you can #define IMGUI_DISABLE_DEMO_WINDOWS in imconfig.h for the same effect.
// If you want to link core ImGui in your final builds but not those demo windows, #define IMGUI_DISABLE_DEMO_WINDOWS in imconfig.h and those functions will be empty.
// In other situation, when you have ImGui available you probably want this to be available for reference and execution.
// Thank you,
// -Your beloved friend, imgui_demo.cpp (that you won't delete)

// Message to beginner C/C++ programmers. About the meaning of 'static': in this demo code, we frequently we use 'static' variables inside functions. 
// We do this as a way to gather code and data in the same place, just to make the demo code faster to read, faster to write, and use less code. 
// A static variable persist across calls, so it is essentially like a global variable but declared inside the scope of the function. 
// It also happens to be a convenient way of storing simple UI related information as long as your function doesn't need to be reentrant or used in threads.
// This might be a pattern you occasionally want to use in your code, but most of the real data you would be editing is likely to be stored outside your function.

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#include <ctype.h>          // toupper, isprint
#include <math.h>           // sqrtf, powf, cosf, sinf, floorf, ceilf
#include <stdio.h>          // vsnprintf, sscanf, printf
#include <stdlib.h>         // NULL, malloc, free, atoi
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>         // intptr_t
#else
#include <stdint.h>         // intptr_t
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#define snprintf _snprintf
#endif
#ifdef __clang__
#pragma clang diagnostic ignored "-Wold-style-cast"             // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"    // warning : 'xx' is deprecated: The POSIX name for this item.. // for strdup used in demo code (so user can copy & paste the code)
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"   // warning : cast to 'void *' from smaller integer type 'int'
#pragma clang diagnostic ignored "-Wformat-security"            // warning : warning: format string is not a string literal
#pragma clang diagnostic ignored "-Wexit-time-destructors"      // warning : declaration requires an exit-time destructor       // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. ImGui coding style welcomes static/globals.
#if __has_warning("-Wreserved-id-macro")
#pragma clang diagnostic ignored "-Wreserved-id-macro"          // warning : macro name is a reserved identifier                //
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"          // warning: cast to pointer from integer of different size
#pragma GCC diagnostic ignored "-Wformat-security"              // warning : format string is not a string literal (potentially insecure)
#pragma GCC diagnostic ignored "-Wdouble-promotion"             // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"                   // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#if (__GNUC__ >= 6)
#pragma GCC diagnostic ignored "-Wmisleading-indentation"       // warning: this 'if' clause does not guard this statement      // GCC 6.0+ only. See #883 on GitHub.
#endif
#endif

// Play it nice with Windows users. Notepad in 2017 still doesn't display text data with Unix-style \n.
#ifdef _WIN32
#define IM_NEWLINE "\r\n"
#else
#define IM_NEWLINE "\n"
#endif

#define IM_MAX(_A,_B)       (((_A) >= (_B)) ? (_A) : (_B))

//-----------------------------------------------------------------------------
// DEMO CODE
//-----------------------------------------------------------------------------

#if !defined(IMGUI_DISABLE_OBSOLETE_FUNCTIONS) && defined(IMGUI_DISABLE_TEST_WINDOWS) && !defined(IMGUI_DISABLE_DEMO_WINDOWS)   // Obsolete name since 1.53, TEST->DEMO
#define IMGUI_DISABLE_DEMO_WINDOWS
#endif

#if !defined(IMGUI_DISABLE_DEMO_WINDOWS)

static void ShowExampleAppConsole(bool* p_open);
static void ShowExampleAppLog(bool* p_open);
static void ShowExampleAppLayout(bool* p_open);
static void ShowExampleAppPropertyEditor(bool* p_open);
static void ShowExampleAppLongText(bool* p_open);
static void ShowExampleAppAutoResize(bool* p_open);
static void ShowExampleAppConstrainedResize(bool* p_open);
static void ShowExampleAppFixedOverlay(bool* p_open);
static void ShowExampleAppWindowTitles(bool* p_open);
static void ShowExampleAppCustomRendering(bool* p_open);
static void ShowExampleAppMainMenuBar();
static void ShowExampleMenuFile();

static void ShowHelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(450.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ImGui::ShowUserGuide()
{
    ImGui::BulletText("Double-click on title bar to collapse window.");
    ImGui::BulletText("Click and drag on lower right corner to resize window\n(double-click to auto fit window to its contents).");
    ImGui::BulletText("Click and drag on any empty space to move window.");
    ImGui::BulletText("TAB/SHIFT+TAB to cycle through keyboard editable fields.");
    ImGui::BulletText("CTRL+Click on a slider or drag box to input value as text.");
    if (ImGui::GetIO().FontAllowUserScaling)
        ImGui::BulletText("CTRL+Mouse Wheel to zoom window contents.");
    ImGui::BulletText("Mouse Wheel to scroll.");
    ImGui::BulletText("While editing text:\n");
    ImGui::Indent();
    ImGui::BulletText("Hold SHIFT or use mouse to select text.");
    ImGui::BulletText("CTRL+Left/Right to word jump.");
    ImGui::BulletText("CTRL+A or double-click to select all.");
    ImGui::BulletText("CTRL+X,CTRL+C,CTRL+V to use clipboard.");
    ImGui::BulletText("CTRL+Z,CTRL+Y to undo/redo.");
    ImGui::BulletText("ESCAPE to revert.");
    ImGui::BulletText("You can apply arithmetic operators +,*,/ on numerical values.\nUse +- to subtract.");
    ImGui::Unindent();
}

// Demonstrate most ImGui features (big function!)
void ImGui::ShowDemoWindow(bool* p_open)
{
    // Examples apps
    static bool show_app_main_menu_bar = false;
    static bool show_app_console = false;
    static bool show_app_log = false;
    static bool show_app_layout = false;
    static bool show_app_property_editor = false;
    static bool show_app_long_text = false;
    static bool show_app_auto_resize = false;
    static bool show_app_constrained_resize = false;
    static bool show_app_fixed_overlay = false;
    static bool show_app_window_titles = false;
    static bool show_app_custom_rendering = false;
    static bool show_app_style_editor = false;

    static bool show_app_metrics = false;
    static bool show_app_about = false;

    if (show_app_main_menu_bar)       ShowExampleAppMainMenuBar();
    if (show_app_console)             ShowExampleAppConsole(&show_app_console);
    if (show_app_log)                 ShowExampleAppLog(&show_app_log);
    if (show_app_layout)              ShowExampleAppLayout(&show_app_layout);
    if (show_app_property_editor)     ShowExampleAppPropertyEditor(&show_app_property_editor);
    if (show_app_long_text)           ShowExampleAppLongText(&show_app_long_text);
    if (show_app_auto_resize)         ShowExampleAppAutoResize(&show_app_auto_resize);
    if (show_app_constrained_resize)  ShowExampleAppConstrainedResize(&show_app_constrained_resize);
    if (show_app_fixed_overlay)       ShowExampleAppFixedOverlay(&show_app_fixed_overlay);
    if (show_app_window_titles)       ShowExampleAppWindowTitles(&show_app_window_titles);
    if (show_app_custom_rendering)    ShowExampleAppCustomRendering(&show_app_custom_rendering);

    if (show_app_metrics)             { ImGui::ShowMetricsWindow(&show_app_metrics); }
    if (show_app_style_editor)        { ImGui::Begin("Style Editor", &show_app_style_editor); ImGui::ShowStyleEditor(); ImGui::End(); }
    if (show_app_about)
    {
        ImGui::Begin("About Dear ImGui", &show_app_about, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Dear ImGui, %s", ImGui::GetVersion());
        ImGui::Separator();
        ImGui::Text("By Omar Cornut and all dear imgui contributors.");
        ImGui::Text("Dear ImGui is licensed under the MIT License, see LICENSE for more information.");
        ImGui::End();
    }

    static bool no_titlebar = false;
    static bool no_scrollbar = false;
    static bool no_menu = false;
    static bool no_move = false;
    static bool no_resize = false;
    static bool no_collapse = false;
    static bool no_close = false;

    // Demonstrate the various window flags. Typically you would just use the default.
    ImGuiWindowFlags window_flags = 0;
    if (no_titlebar)  window_flags |= ImGuiWindowFlags_NoTitleBar;
    if (no_scrollbar) window_flags |= ImGuiWindowFlags_NoScrollbar;
    if (!no_menu)     window_flags |= ImGuiWindowFlags_MenuBar;
    if (no_move)      window_flags |= ImGuiWindowFlags_NoMove;
    if (no_resize)    window_flags |= ImGuiWindowFlags_NoResize;
    if (no_collapse)  window_flags |= ImGuiWindowFlags_NoCollapse;
    if (no_close)     p_open = NULL; // Don't pass our bool* to Begin

    ImGui::SetNextWindowSize(ImVec2(550,680), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("ImGui Demo", p_open, window_flags))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    //ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);    // 2/3 of the space for widget and 1/3 for labels
    ImGui::PushItemWidth(-140);                                 // Right align, keep 140 pixels for labels

    ImGui::Text("dear imgui says hello. (%s)", IMGUI_VERSION);

    // Menu
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Examples"))
        {
            ImGui::MenuItem("Main menu bar", NULL, &show_app_main_menu_bar);
            ImGui::MenuItem("Console", NULL, &show_app_console);
            ImGui::MenuItem("Log", NULL, &show_app_log);
            ImGui::MenuItem("Simple layout", NULL, &show_app_layout);
            ImGui::MenuItem("Property editor", NULL, &show_app_property_editor);
            ImGui::MenuItem("Long text display", NULL, &show_app_long_text);
            ImGui::MenuItem("Auto-resizing window", NULL, &show_app_auto_resize);
            ImGui::MenuItem("Constrained-resizing window", NULL, &show_app_constrained_resize);
            ImGui::MenuItem("Simple overlay", NULL, &show_app_fixed_overlay);
            ImGui::MenuItem("Manipulating window titles", NULL, &show_app_window_titles);
            ImGui::MenuItem("Custom rendering", NULL, &show_app_custom_rendering);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            ImGui::MenuItem("Metrics", NULL, &show_app_metrics);
            ImGui::MenuItem("Style Editor", NULL, &show_app_style_editor);
            ImGui::MenuItem("About Dear ImGui", NULL, &show_app_about);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::Spacing();
    if (ImGui::CollapsingHeader("Help"))
    {
        ImGui::TextWrapped("This window is being created by the ShowDemoWindow() function. Please refer to the code in imgui_demo.cpp for reference.\n\n");
        ImGui::Text("USER GUIDE:");
        ImGui::ShowUserGuide();
    }

    if (ImGui::CollapsingHeader("Window options"))
    {
        ImGui::Checkbox("No titlebar", &no_titlebar); ImGui::SameLine(150);
        ImGui::Checkbox("No scrollbar", &no_scrollbar); ImGui::SameLine(300);
        ImGui::Checkbox("No menu", &no_menu);
        ImGui::Checkbox("No move", &no_move); ImGui::SameLine(150);
        ImGui::Checkbox("No resize", &no_resize); ImGui::SameLine(300);
        ImGui::Checkbox("No collapse", &no_collapse);
        ImGui::Checkbox("No close", &no_close);

        if (ImGui::TreeNode("Style"))
        {
            ImGui::ShowStyleEditor();
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Capture/Logging"))
        {
            ImGui::TextWrapped("The logging API redirects all text output so you can easily capture the content of a window or a block. Tree nodes can be automatically expanded. You can also call ImGui::LogText() to output directly to the log without a visual output.");
            ImGui::LogButtons();
            ImGui::TreePop();
        }
    }

    if (ImGui::CollapsingHeader("Widgets"))
    {
        if (ImGui::TreeNode("Basic"))
        {
            static int clicked = 0;
            if (ImGui::Button("Button")) 
                clicked++;
            if (clicked & 1)
            {
                ImGui::SameLine();
                ImGui::Text("Thanks for clicking me!");
            }

            static bool check = true;
            ImGui::Checkbox("checkbox", &check);

            static int e = 0;
            ImGui::RadioButton("radio a", &e, 0); ImGui::SameLine();
            ImGui::RadioButton("radio b", &e, 1); ImGui::SameLine();
            ImGui::RadioButton("radio c", &e, 2);

            // Color buttons, demonstrate using PushID() to add unique identifier in the ID stack, and changing style.
            for (int i = 0; i < 7; i++)
            {
                if (i > 0) ImGui::SameLine();
                ImGui::PushID(i);
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i/7.0f, 0.6f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i/7.0f, 0.7f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i/7.0f, 0.8f, 0.8f));
                ImGui::Button("Click");
                ImGui::PopStyleColor(3);
                ImGui::PopID();
            }

            ImGui::Text("Hover over me");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("I am a tooltip");

            ImGui::SameLine();
            ImGui::Text("- or me");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("I am a fancy tooltip");
                static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
                ImGui::PlotLines("Curve", arr, IM_ARRAYSIZE(arr));
                ImGui::EndTooltip();
            }

            // Testing ImGuiOnceUponAFrame helper.
            //static ImGuiOnceUponAFrame once;
            //for (int i = 0; i < 5; i++)
            //    if (once)
            //        ImGui::Text("This will be displayed only once.");

            ImGui::Separator();

            ImGui::LabelText("label", "Value");

            {
                // Simplified one-liner Combo() API, using values packed in a single constant string
                static int current_item_1 = 1;
                ImGui::Combo("combo", &current_item_1, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");
                //ImGui::Combo("combo w/ array of char*", &current_item_2_idx, items, IM_ARRAYSIZE(items));   // Combo using proper array. You can also pass a callback to retrieve array value, no need to create/copy an array just for that.

                // General BeginCombo() API, you have full control over your selection data and display type
                const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO", "PPPP", "QQQQQQQQQQ", "RRR", "SSSS" };
                static const char* current_item_2 = NULL;
                if (ImGui::BeginCombo("combo 2", current_item_2)) // The second parameter is the label previewed before opening the combo.
                {
                    for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                    {
                        bool is_selected = (current_item_2 == items[n]); // You can store your selection however you want, outside or inside your objects
                        if (ImGui::Selectable(items[n], is_selected))
                            current_item_2 = items[n];
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
                    }
                    ImGui::EndCombo();
                }
            }

            {
                static char str0[128] = "Hello, world!";
                static int i0=123;
                static float f0=0.001f;
                ImGui::InputText("input text", str0, IM_ARRAYSIZE(str0));
                ImGui::SameLine(); ShowHelpMarker("Hold SHIFT or use mouse to select text.\n" "CTRL+Left/Right to word jump.\n" "CTRL+A or double-click to select all.\n" "CTRL+X,CTRL+C,CTRL+V clipboard.\n" "CTRL+Z,CTRL+Y undo/redo.\n" "ESCAPE to revert.\n");

                ImGui::InputInt("input int", &i0);
                ImGui::SameLine(); ShowHelpMarker("You can apply arithmetic operators +,*,/ on numerical values.\n  e.g. [ 100 ], input \'*2\', result becomes [ 200 ]\nUse +- to subtract.\n");

                ImGui::InputFloat("input float", &f0, 0.01f, 1.0f);

                static float vec4a[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
                ImGui::InputFloat3("input float3", vec4a);
            }

            {
                static int i1=50, i2=42;
                ImGui::DragInt("drag int", &i1, 1);
                ImGui::SameLine(); ShowHelpMarker("Click and drag to edit value.\nHold SHIFT/ALT for faster/slower edit.\nDouble-click or CTRL+click to input value.");

                ImGui::DragInt("drag int 0..100", &i2, 1, 0, 100, "%.0f%%");

                static float f1=1.00f, f2=0.0067f;
                ImGui::DragFloat("drag float", &f1, 0.005f);
                ImGui::DragFloat("drag small float", &f2, 0.0001f, 0.0f, 0.0f, "%.06f ns");
            }

            {
                static int i1=0;
                ImGui::SliderInt("slider int", &i1, -1, 3);
                ImGui::SameLine(); ShowHelpMarker("CTRL+click to input value.");

                static float f1=0.123f, f2=0.0f;
                ImGui::SliderFloat("slider float", &f1, 0.0f, 1.0f, "ratio = %.3f");
                ImGui::SliderFloat("slider log float", &f2, -10.0f, 10.0f, "%.4f", 3.0f);
                static float angle = 0.0f;
                ImGui::SliderAngle("slider angle", &angle);
            }

            static float col1[3] = { 1.0f,0.0f,0.2f };
            static float col2[4] = { 0.4f,0.7f,0.0f,0.5f };
            ImGui::ColorEdit3("color 1", col1);
            ImGui::SameLine(); ShowHelpMarker("Click on the colored square to open a color picker.\nRight-click on the colored square to show options.\nCTRL+click on individual component to input value.\n");

            ImGui::ColorEdit4("color 2", col2);

            const char* listbox_items[] = { "Apple", "Banana", "Cherry", "Kiwi", "Mango", "Orange", "Pineapple", "Strawberry", "Watermelon" };
            static int listbox_item_current = 1;
            ImGui::ListBox("listbox\n(single select)", &listbox_item_current, listbox_items, IM_ARRAYSIZE(listbox_items), 4);

            //static int listbox_item_current2 = 2;
            //ImGui::PushItemWidth(-1);
            //ImGui::ListBox("##listbox2", &listbox_item_current2, listbox_items, IM_ARRAYSIZE(listbox_items), 4);
            //ImGui::PopItemWidth();

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Trees"))
        {
            if (ImGui::TreeNode("Basic trees"))
            {
                for (int i = 0; i < 5; i++)
                    if (ImGui::TreeNode((void*)(intptr_t)i, "Child %d", i))
                    {
                        ImGui::Text("blah blah");
                        ImGui::SameLine(); 
                        if (ImGui::SmallButton("button")) { };
                        ImGui::TreePop();
                    }
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Advanced, with Selectable nodes"))
            {
                ShowHelpMarker("This is a more standard looking tree with selectable nodes.\nClick to select, CTRL+Click to toggle, click on arrows or double-click to open.");
                static bool align_label_with_current_x_position = false;
                ImGui::Checkbox("Align label with current X position)", &align_label_with_current_x_position);
                ImGui::Text("Hello!");
                if (align_label_with_current_x_position)
                    ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());

                static int selection_mask = (1 << 2); // Dumb representation of what may be user-side selection state. You may carry selection state inside or outside your objects in whatever format you see fit.
                int node_clicked = -1;                // Temporary storage of what node we have clicked to process selection at the end of the loop. May be a pointer to your own node type, etc.
                ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize()*3); // Increase spacing to differentiate leaves from expanded contents.
                for (int i = 0; i < 6; i++)
                {
                    // Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
                    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((selection_mask & (1 << i)) ? ImGuiTreeNodeFlags_Selected : 0);
                    if (i < 3)
                    {
                        // Node
                        bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "Selectable Node %d", i);
                        if (ImGui::IsItemClicked()) 
                            node_clicked = i;
                        if (node_open)
                        {
                            ImGui::Text("Blah blah\nBlah Blah");
                            ImGui::TreePop();
                        }
                    }
                    else
                    {
                        // Leaf: The only reason we have a TreeNode at all is to allow selection of the leaf. Otherwise we can use BulletText() or TreeAdvanceToLabelPos()+Text().
                        node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
                        ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "Selectable Leaf %d", i);
                        if (ImGui::IsItemClicked()) 
                            node_clicked = i;
                    }
                }
                if (node_clicked != -1)
                {
                    // Update selection state. Process outside of tree loop to avoid visual inconsistencies during the clicking-frame.
                    if (ImGui::GetIO().KeyCtrl)
                        selection_mask ^= (1 << node_clicked);          // CTRL+click to toggle
                    else //if (!(selection_mask & (1 << node_clicked))) // Depending on selection behavior you want, this commented bit preserve selection when clicking on item that is part of the selection
                        selection_mask = (1 << node_clicked);           // Click to single-select
                }
                ImGui::PopStyleVar();
                if (align_label_with_current_x_position)
                    ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Collapsing Headers"))
        {
            static bool closable_group = true;
            ImGui::Checkbox("Enable extra group", &closable_group);
            if (ImGui::CollapsingHeader("Header"))
            {
                ImGui::Text("IsItemHovered: %d", IsItemHovered());
                for (int i = 0; i < 5; i++)
                    ImGui::Text("Some content %d", i);
            }
            if (ImGui::CollapsingHeader("Header with a close button", &closable_group))
            {
                ImGui::Text("IsItemHovered: %d", IsItemHovered());
                for (int i = 0; i < 5; i++)
                    ImGui::Text("More content %d", i);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Bullets"))
        {
            ImGui::BulletText("Bullet point 1");
            ImGui::BulletText("Bullet point 2\nOn multiple lines");
            ImGui::Bullet(); ImGui::Text("Bullet point 3 (two calls)");
            ImGui::Bullet(); ImGui::SmallButton("Button");
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Text"))
        {
            if (ImGui::TreeNode("Colored Text"))
            {
                // Using shortcut. You can use PushStyleColor()/PopStyleColor() for more flexibility.
                ImGui::TextColored(ImVec4(1.0f,0.0f,1.0f,1.0f), "Pink");
                ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "Yellow");
                ImGui::TextDisabled("Disabled");
                ImGui::SameLine(); ShowHelpMarker("The TextDisabled color is stored in ImGuiStyle.");
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Word Wrapping"))
            {
                // Using shortcut. You can use PushTextWrapPos()/PopTextWrapPos() for more flexibility.
                ImGui::TextWrapped("This text should automatically wrap on the edge of the window. The current implementation for text wrapping follows simple rules suitable for English and possibly other languages.");
                ImGui::Spacing();

                static float wrap_width = 200.0f;
                ImGui::SliderFloat("Wrap width", &wrap_width, -20, 600, "%.0f");

                ImGui::Text("Test paragraph 1:");
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + wrap_width, pos.y), ImVec2(pos.x + wrap_width + 10, pos.y + ImGui::GetTextLineHeight()), IM_COL32(255,0,255,255));
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrap_width);
                ImGui::Text("The lazy dog is a good dog. This paragraph is made to fit within %.0f pixels. Testing a 1 character word. The quick brown fox jumps over the lazy dog.", wrap_width);
                ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255,255,0,255));
                ImGui::PopTextWrapPos();

                ImGui::Text("Test paragraph 2:");
                pos = ImGui::GetCursorScreenPos();
                ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + wrap_width, pos.y), ImVec2(pos.x + wrap_width + 10, pos.y + ImGui::GetTextLineHeight()), IM_COL32(255,0,255,255));
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrap_width);
                ImGui::Text("aaaaaaaa bbbbbbbb, c cccccccc,dddddddd. d eeeeeeee   ffffffff. gggggggg!hhhhhhhh");
                ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255,255,0,255));
                ImGui::PopTextWrapPos();

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("UTF-8 Text"))
            {
                // UTF-8 test with Japanese characters
                // (needs a suitable font, try Arial Unicode or M+ fonts http://mplus-fonts.sourceforge.jp/mplus-outline-fonts/index-en.html)
                // - From C++11 you can use the u8"my text" syntax to encode literal strings as UTF-8
                // - For earlier compiler, you may be able to encode your sources as UTF-8 (e.g. Visual Studio save your file as 'UTF-8 without signature')
                // - HOWEVER, FOR THIS DEMO FILE, BECAUSE WE WANT TO SUPPORT COMPILER, WE ARE *NOT* INCLUDING RAW UTF-8 CHARACTERS IN THIS SOURCE FILE.
                //   Instead we are encoding a few string with hexadecimal constants. Don't do this in your application!
                // Note that characters values are preserved even by InputText() if the font cannot be displayed, so you can safely copy & paste garbled characters into another application.
                ImGui::TextWrapped("CJK text will only appears if the font was loaded with the appropriate CJK character ranges. Call io.Font->LoadFromFileTTF() manually to load extra character ranges.");
                ImGui::Text("Hiragana: \xe3\x81\x8b\xe3\x81\x8d\xe3\x81\x8f\xe3\x81\x91\xe3\x81\x93 (kakikukeko)");
                ImGui::Text("Kanjis: \xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e (nihongo)");
                static char buf[32] = "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e"; // "nihongo"
                ImGui::InputText("UTF-8 input", buf, IM_ARRAYSIZE(buf));
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Images"))
        {
            ImGui::TextWrapped("Below we are displaying the font texture (which is the only texture we have access to in this demo). Use the 'ImTextureID' type as storage to pass pointers or identifier to your own texture data. Hover the texture for a zoomed view!");
            ImGuiIO& io = ImGui::GetIO();

            // Here we are grabbing the font texture because that's the only one we have access to inside the demo code.
            // Remember that ImTextureID is just storage for whatever you want it to be, it is essentially a value that will be passed to the render function inside the ImDrawCmd structure.
            // If you use one of the default imgui_impl_XXXX.cpp renderer, they all have comments at the top of their file to specify what they expect to be stored in ImTextureID.
            // (for example, the imgui_impl_dx11.cpp renderer expect a 'ID3D11ShaderResourceView*' pointer. The imgui_impl_glfw_gl3.cpp renderer expect a GLuint OpenGL texture identifier etc.)
            // If you decided that ImTextureID = MyEngineTexture*, then you can pass your MyEngineTexture* pointers to ImGui::Image(), and gather width/height through your own functions, etc.
            // Using ShowMetricsWindow() as a "debugger" to inspect the draw data that are being passed to your render will help you debug issues if you are confused about this.
            // Consider using the lower-level ImDrawList::AddImage() API, via ImGui::GetWindowDrawList()->AddImage().
            ImTextureID my_tex_id = io.Fonts->TexID; 
            float my_tex_w = (float)io.Fonts->TexWidth;
            float my_tex_h = (float)io.Fonts->TexHeight;

            ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), ImVec2(0,0), ImVec2(1,1), ImColor(255,255,255,255), ImColor(255,255,255,128));
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                float focus_sz = 32.0f;
                float focus_x = io.MousePos.x - pos.x - focus_sz * 0.5f; if (focus_x < 0.0f) focus_x = 0.0f; else if (focus_x > my_tex_w - focus_sz) focus_x = my_tex_w - focus_sz;
                float focus_y = io.MousePos.y - pos.y - focus_sz * 0.5f; if (focus_y < 0.0f) focus_y = 0.0f; else if (focus_y > my_tex_h - focus_sz) focus_y = my_tex_h - focus_sz;
                ImGui::Text("Min: (%.2f, %.2f)", focus_x, focus_y);
                ImGui::Text("Max: (%.2f, %.2f)", focus_x + focus_sz, focus_y + focus_sz);
                ImVec2 uv0 = ImVec2((focus_x) / my_tex_w, (focus_y) / my_tex_h);
                ImVec2 uv1 = ImVec2((focus_x + focus_sz) / my_tex_w, (focus_y + focus_sz) / my_tex_h);
                ImGui::Image(my_tex_id, ImVec2(128,128), uv0, uv1, ImColor(255,255,255,255), ImColor(255,255,255,128));
                ImGui::EndTooltip();
            }
            ImGui::TextWrapped("And now some textured buttons..");
            static int pressed_count = 0;
            for (int i = 0; i < 8; i++)
            {
                ImGui::PushID(i);
                int frame_padding = -1 + i;     // -1 = uses default padding
                if (ImGui::ImageButton(my_tex_id, ImVec2(32,32), ImVec2(0,0), ImVec2(32.0f/my_tex_w,32/my_tex_h), frame_padding, ImColor(0,0,0,255)))
                    pressed_count += 1;
                ImGui::PopID();
                ImGui::SameLine();
            }
            ImGui::NewLine();
            ImGui::Text("Pressed %d times.", pressed_count);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Selectables"))
        {
            // Selectable() has 2 overloads:
            // - The one taking "bool selected" as a read-only selection information. When Selectable() has been clicked is returns true and you can alter selection state accordingly.
            // - The one taking "bool* p_selected" as a read-write selection information (convenient in some cases)
            // The earlier is more flexible, as in real application your selection may be stored in a different manner (in flags within objects, as an external list, etc).
            if (ImGui::TreeNode("Basic"))
            {
                static bool selection[5] = { false, true, false, false, false };
                ImGui::Selectable("1. I am selectable", &selection[0]);
                ImGui::Selectable("2. I am selectable", &selection[1]);
                ImGui::Text("3. I am not selectable");
                ImGui::Selectable("4. I am selectable", &selection[3]);
                if (ImGui::Selectable("5. I am double clickable", selection[4], ImGuiSelectableFlags_AllowDoubleClick))
                    if (ImGui::IsMouseDoubleClicked(0))
                        selection[4] = !selection[4];
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Selection State: Single Selection"))
            {
                static int selected = -1;
                for (int n = 0; n < 5; n++)
                {
                    char buf[32];
                    sprintf(buf, "Object %d", n);
                    if (ImGui::Selectable(buf, selected == n)) 
                        selected = n;
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Selection State: Multiple Selection"))
            {
                ShowHelpMarker("Hold CTRL and click to select multiple items.");
                static bool selection[5] = { false, false, false, false, false };
                for (int n = 0; n < 5; n++)
                {
                    char buf[32];
                    sprintf(buf, "Object %d", n);
                    if (ImGui::Selectable(buf, selection[n]))
                    {
                        if (!ImGui::GetIO().KeyCtrl)    // Clear selection when CTRL is not held
                            memset(selection, 0, sizeof(selection)); 
                        selection[n] ^= 1;
                    }
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Rendering more text into the same line"))
            {
                // Using the Selectable() override that takes "bool* p_selected" parameter and toggle your booleans automatically.
                static bool selected[3] = { false, false, false };
                ImGui::Selectable("main.c",    &selected[0]); ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");
                ImGui::Selectable("Hello.cpp", &selected[1]); ImGui::SameLine(300); ImGui::Text("12,345 bytes");
                ImGui::Selectable("Hello.h",   &selected[2]); ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("In columns"))
            {
                ImGui::Columns(3, NULL, false);
                static bool selected[16] = { 0 };
                for (int i = 0; i < 16; i++)
                {
                    char label[32]; sprintf(label, "Item %d", i);
                    if (ImGui::Selectable(label, &selected[i])) {}
                    ImGui::NextColumn();
                }
                ImGui::Columns(1);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Grid"))
            {
                static bool selected[16] = { true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true };
                for (int i = 0; i < 16; i++)
                {
                    ImGui::PushID(i);
                    if (ImGui::Selectable("Sailor", &selected[i], 0, ImVec2(50,50)))
                    {
                        int x = i % 4, y = i / 4;
                        if (x > 0) selected[i - 1] ^= 1;
                        if (x < 3) selected[i + 1] ^= 1;
                        if (y > 0) selected[i - 4] ^= 1;
                        if (y < 3) selected[i + 4] ^= 1;
                    }
                    if ((i % 4) < 3) ImGui::SameLine();
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Filtered Text Input"))
        {
            static char buf1[64] = ""; ImGui::InputText("default", buf1, 64);
            static char buf2[64] = ""; ImGui::InputText("decimal", buf2, 64, ImGuiInputTextFlags_CharsDecimal);
            static char buf3[64] = ""; ImGui::InputText("hexadecimal", buf3, 64, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
            static char buf4[64] = ""; ImGui::InputText("uppercase", buf4, 64, ImGuiInputTextFlags_CharsUppercase);
            static char buf5[64] = ""; ImGui::InputText("no blank", buf5, 64, ImGuiInputTextFlags_CharsNoBlank);
            struct TextFilters { static int FilterImGuiLetters(ImGuiTextEditCallbackData* data) { if (data->EventChar < 256 && strchr("imgui", (char)data->EventChar)) return 0; return 1; } };
            static char buf6[64] = ""; ImGui::InputText("\"imgui\" letters", buf6, 64, ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterImGuiLetters);

            ImGui::Text("Password input");
            static char bufpass[64] = "password123";
            ImGui::InputText("password", bufpass, 64, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_CharsNoBlank);
            ImGui::SameLine(); ShowHelpMarker("Display all characters as '*'.\nDisable clipboard cut and copy.\nDisable logging.\n");
            ImGui::InputText("password (clear)", bufpass, 64, ImGuiInputTextFlags_CharsNoBlank);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Multi-line Text Input"))
        {
            static bool read_only = false;
            static char text[1024*16] =
                "/*\n"
                " The Pentium F00F bug, shorthand for F0 0F C7 C8,\n"
                " the hexadecimal encoding of one offending instruction,\n"
                " more formally, the invalid operand with locked CMPXCHG8B\n"
                " instruction bug, is a design flaw in the majority of\n"
                " Intel Pentium, Pentium MMX, and Pentium OverDrive\n"
                " processors (all in the P5 microarchitecture).\n"
                "*/\n\n"
                "label:\n"
                "\tlock cmpxchg8b eax\n";

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
            ImGui::Checkbox("Read-only", &read_only);
            ImGui::PopStyleVar();
            ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-1.0f, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput | (read_only ? ImGuiInputTextFlags_ReadOnly : 0));
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Plots widgets"))
        {
            static bool animate = true;
            ImGui::Checkbox("Animate", &animate);

            static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
            ImGui::PlotLines("Frame Times", arr, IM_ARRAYSIZE(arr));

            // Create a dummy array of contiguous float values to plot
            // Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float and the sizeof() of your structure in the Stride parameter.
            static float values[90] = { 0 };
            static int values_offset = 0;
            static float refresh_time = 0.0f;
            if (!animate || refresh_time == 0.0f)
                refresh_time = ImGui::GetTime();
            while (refresh_time < ImGui::GetTime()) // Create dummy data at fixed 60 hz rate for the demo
            {
                static float phase = 0.0f;
                values[values_offset] = cosf(phase);
                values_offset = (values_offset+1) % IM_ARRAYSIZE(values);
                phase += 0.10f*values_offset;
                refresh_time += 1.0f/60.0f;
            }
            ImGui::PlotLines("Lines", values, IM_ARRAYSIZE(values), values_offset, "avg 0.0", -1.0f, 1.0f, ImVec2(0,80));
            ImGui::PlotHistogram("Histogram", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 1.0f, ImVec2(0,80));

            // Use functions to generate output
            // FIXME: This is rather awkward because current plot API only pass in indices. We probably want an API passing floats and user provide sample rate/count.
            struct Funcs
            {
                static float Sin(void*, int i) { return sinf(i * 0.1f); }
                static float Saw(void*, int i) { return (i & 1) ? 1.0f : -1.0f; }
            };
            static int func_type = 0, display_count = 70;
            ImGui::Separator();
            ImGui::PushItemWidth(100); ImGui::Combo("func", &func_type, "Sin\0Saw\0"); ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::SliderInt("Sample count", &display_count, 1, 400);
            float (*func)(void*, int) = (func_type == 0) ? Funcs::Sin : Funcs::Saw;
            ImGui::PlotLines("Lines", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0,80));
            ImGui::PlotHistogram("Histogram", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0,80));
            ImGui::Separator();

            // Animate a simple progress bar
            static float progress = 0.0f, progress_dir = 1.0f;
            if (animate)
            {
                progress += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
                if (progress >= +1.1f) { progress = +1.1f; progress_dir *= -1.0f; }
                if (progress <= -0.1f) { progress = -0.1f; progress_dir *= -1.0f; }
            }

            // Typically we would use ImVec2(-1.0f,0.0f) to use all available width, or ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses ItemWidth.
            ImGui::ProgressBar(progress, ImVec2(0.0f,0.0f));
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::Text("Progress Bar");

            float progress_saturated = (progress < 0.0f) ? 0.0f : (progress > 1.0f) ? 1.0f : progress;
            char buf[32];
            sprintf(buf, "%d/%d", (int)(progress_saturated*1753), 1753);
            ImGui::ProgressBar(progress, ImVec2(0.f,0.f), buf);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Color/Picker Widgets"))
        {
            static ImVec4 color = ImColor(114, 144, 154, 200);

            static bool alpha_preview = true;
            static bool alpha_half_preview = false;
            static bool options_menu = true;
            static bool hdr = false;
            ImGui::Checkbox("With Alpha Preview", &alpha_preview);
            ImGui::Checkbox("With Half Alpha Preview", &alpha_half_preview);
            ImGui::Checkbox("With Options Menu", &options_menu); ImGui::SameLine(); ShowHelpMarker("Right-click on the individual color widget to show options.");
            ImGui::Checkbox("With HDR", &hdr); ImGui::SameLine(); ShowHelpMarker("Currently all this does is to lift the 0..1 limits on dragging widgets.");
            int misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);

            ImGui::Text("Color widget:");
            ImGui::SameLine(); ShowHelpMarker("Click on the colored square to open a color picker.\nCTRL+click on individual component to input value.\n");
            ImGui::ColorEdit3("MyColor##1", (float*)&color, misc_flags);

            ImGui::Text("Color widget HSV with Alpha:");
            ImGui::ColorEdit4("MyColor##2", (float*)&color, ImGuiColorEditFlags_HSV | misc_flags);

            ImGui::Text("Color widget with Float Display:");
            ImGui::ColorEdit4("MyColor##2f", (float*)&color, ImGuiColorEditFlags_Float | misc_flags);

            ImGui::Text("Color button with Picker:");
            ImGui::SameLine(); ShowHelpMarker("With the ImGuiColorEditFlags_NoInputs flag you can hide all the slider/text inputs.\nWith the ImGuiColorEditFlags_NoLabel flag you can pass a non-empty label which will only be used for the tooltip and picker popup.");
            ImGui::ColorEdit4("MyColor##3", (float*)&color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);

            ImGui::Text("Color button with Custom Picker Popup:");

            // Generate a dummy palette
            static bool saved_palette_inited = false;
            static ImVec4 saved_palette[32];
            if (!saved_palette_inited)
                for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
                {
                    ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f, saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
                    saved_palette[n].w = 1.0f; // Alpha
                }
            saved_palette_inited = true;

            static ImVec4 backup_color;
            bool open_popup = ImGui::ColorButton("MyColor##3b", color, misc_flags);
            ImGui::SameLine();
            open_popup |= ImGui::Button("Palette");
            if (open_popup)
            {
                ImGui::OpenPopup("mypicker");
                backup_color = color;
            }
            if (ImGui::BeginPopup("mypicker"))
            {
                // FIXME: Adding a drag and drop example here would be perfect!
                ImGui::Text("MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!");
                ImGui::Separator();
                ImGui::ColorPicker4("##picker", (float*)&color, misc_flags | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
                ImGui::SameLine();
                ImGui::BeginGroup();
                ImGui::Text("Current");
                ImGui::ColorButton("##current", color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60,40));
                ImGui::Text("Previous");
                if (ImGui::ColorButton("##previous", backup_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60,40)))
                    color = backup_color;
                ImGui::Separator();
                ImGui::Text("Palette");
                for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
                {
                    ImGui::PushID(n);
                    if ((n % 8) != 0)
                        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);
                    if (ImGui::ColorButton("##palette", saved_palette[n], ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip, ImVec2(20,20)))
                        color = ImVec4(saved_palette[n].x, saved_palette[n].y, saved_palette[n].z, color.w); // Preserve alpha!

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
                            memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 3);
                        if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
                            memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 4);
                        EndDragDropTarget();
                    }

                    ImGui::PopID();
                }
                ImGui::EndGroup();
                ImGui::EndPopup();
            }

            ImGui::Text("Color button only:");
            ImGui::ColorButton("MyColor##3c", *(ImVec4*)&color, misc_flags, ImVec2(80,80));

            ImGui::Text("Color picker:");
            static bool alpha = true;
            static bool alpha_bar = true;
            static bool side_preview = true;
            static bool ref_color = false;
            static ImVec4 ref_color_v(1.0f,0.0f,1.0f,0.5f);
            static int inputs_mode = 2;
            static int picker_mode = 0;
            ImGui::Checkbox("With Alpha", &alpha);
            ImGui::Checkbox("With Alpha Bar", &alpha_bar);
            ImGui::Checkbox("With Side Preview", &side_preview);
            if (side_preview)
            {
                ImGui::SameLine();
                ImGui::Checkbox("With Ref Color", &ref_color);
                if (ref_color)
                {
                    ImGui::SameLine();
                    ImGui::ColorEdit4("##RefColor", &ref_color_v.x, ImGuiColorEditFlags_NoInputs | misc_flags);
                }
            }
            ImGui::Combo("Inputs Mode", &inputs_mode, "All Inputs\0No Inputs\0RGB Input\0HSV Input\0HEX Input\0");
            ImGui::Combo("Picker Mode", &picker_mode, "Auto/Current\0Hue bar + SV rect\0Hue wheel + SV triangle\0");
            ImGui::SameLine(); ShowHelpMarker("User can right-click the picker to change mode.");
            ImGuiColorEditFlags flags = misc_flags;
            if (!alpha) flags |= ImGuiColorEditFlags_NoAlpha; // This is by default if you call ColorPicker3() instead of ColorPicker4()
            if (alpha_bar) flags |= ImGuiColorEditFlags_AlphaBar;
            if (!side_preview) flags |= ImGuiColorEditFlags_NoSidePreview;
            if (picker_mode == 1) flags |= ImGuiColorEditFlags_PickerHueBar;
            if (picker_mode == 2) flags |= ImGuiColorEditFlags_PickerHueWheel;
            if (inputs_mode == 1) flags |= ImGuiColorEditFlags_NoInputs;
            if (inputs_mode == 2) flags |= ImGuiColorEditFlags_RGB;
            if (inputs_mode == 3) flags |= ImGuiColorEditFlags_HSV;
            if (inputs_mode == 4) flags |= ImGuiColorEditFlags_HEX;
            ImGui::ColorPicker4("MyColor##4", (float*)&color, flags, ref_color ? &ref_color_v.x : NULL);

            ImGui::Text("Programmatically set defaults/options:");
            ImGui::SameLine(); ShowHelpMarker("SetColorEditOptions() is designed to allow you to set boot-time default.\nWe don't have Push/Pop functions because you can force options on a per-widget basis if needed, and the user can change non-forced ones with the options menu.\nWe don't have a getter to avoid encouraging you to persistently save values that aren't forward-compatible.");
            if (ImGui::Button("Uint8 + HSV"))
                ImGui::SetColorEditOptions(ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_HSV);
            ImGui::SameLine();
            if (ImGui::Button("Float + HDR"))
                ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_RGB);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Range Widgets"))
        {
            static float begin = 10, end = 90;
            static int begin_i = 100, end_i = 1000;
            ImGui::DragFloatRange2("range", &begin, &end, 0.25f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
            ImGui::DragIntRange2("range int (no bounds)", &begin_i, &end_i, 5, 0, 0, "Min: %.0f units", "Max: %.0f units");
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Multi-component Widgets"))
        {
            static float vec4f[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
            static int vec4i[4] = { 1, 5, 100, 255 };

            ImGui::InputFloat2("input float2", vec4f);
            ImGui::DragFloat2("drag float2", vec4f, 0.01f, 0.0f, 1.0f);
            ImGui::SliderFloat2("slider float2", vec4f, 0.0f, 1.0f);
            ImGui::DragInt2("drag int2", vec4i, 1, 0, 255);
            ImGui::InputInt2("input int2", vec4i);
            ImGui::SliderInt2("slider int2", vec4i, 0, 255);
            ImGui::Spacing();

            ImGui::InputFloat3("input float3", vec4f);
            ImGui::DragFloat3("drag float3", vec4f, 0.01f, 0.0f, 1.0f);
            ImGui::SliderFloat3("slider float3", vec4f, 0.0f, 1.0f);
            ImGui::DragInt3("drag int3", vec4i, 1, 0, 255);
            ImGui::InputInt3("input int3", vec4i);
            ImGui::SliderInt3("slider int3", vec4i, 0, 255);
            ImGui::Spacing();

            ImGui::InputFloat4("input float4", vec4f);
            ImGui::DragFloat4("drag float4", vec4f, 0.01f, 0.0f, 1.0f);
            ImGui::SliderFloat4("slider float4", vec4f, 0.0f, 1.0f);
            ImGui::InputInt4("input int4", vec4i);
            ImGui::DragInt4("drag int4", vec4i, 1, 0, 255);
            ImGui::SliderInt4("slider int4", vec4i, 0, 255);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Vertical Sliders"))
        {
            const float spacing = 4;
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, spacing));

            static int int_value = 0;
            ImGui::VSliderInt("##int", ImVec2(18,160), &int_value, 0, 5);
            ImGui::SameLine();

            static float values[7] = { 0.0f, 0.60f, 0.35f, 0.9f, 0.70f, 0.20f, 0.0f };
            ImGui::PushID("set1");
            for (int i = 0; i < 7; i++)
            {
                if (i > 0) ImGui::SameLine();
                ImGui::PushID(i);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(i/7.0f, 0.5f, 0.5f));
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(i/7.0f, 0.6f, 0.5f));
                ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(i/7.0f, 0.7f, 0.5f));
                ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(i/7.0f, 0.9f, 0.9f));
                ImGui::VSliderFloat("##v", ImVec2(18,160), &values[i], 0.0f, 1.0f, "");
                if (ImGui::IsItemActive() || ImGui::IsItemHovered())
                    ImGui::SetTooltip("%.3f", values[i]);
                ImGui::PopStyleColor(4);
                ImGui::PopID();
            }
            ImGui::PopID();

            ImGui::SameLine();
            ImGui::PushID("set2");
            static float values2[4] = { 0.20f, 0.80f, 0.40f, 0.25f };
            const int rows = 3;
            const ImVec2 small_slider_size(18, (160.0f-(rows-1)*spacing)/rows);
            for (int nx = 0; nx < 4; nx++)
            {
                if (nx > 0) ImGui::SameLine();
                ImGui::BeginGroup();
                for (int ny = 0; ny < rows; ny++)
                {
                    ImGui::PushID(nx*rows+ny);
                    ImGui::VSliderFloat("##v", small_slider_size, &values2[nx], 0.0f, 1.0f, "");
                    if (ImGui::IsItemActive() || ImGui::IsItemHovered())
                        ImGui::SetTooltip("%.3f", values2[nx]);
                    ImGui::PopID();
                }
                ImGui::EndGroup();
            }
            ImGui::PopID();

            ImGui::SameLine();
            ImGui::PushID("set3");
            for (int i = 0; i < 4; i++)
            {
                if (i > 0) ImGui::SameLine();
                ImGui::PushID(i);
                ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
                ImGui::VSliderFloat("##v", ImVec2(40,160), &values[i], 0.0f, 1.0f, "%.2f\nsec");
                ImGui::PopStyleVar();
                ImGui::PopID();
            }
            ImGui::PopID();
            ImGui::PopStyleVar();
            ImGui::TreePop();
        }
    }

    if (ImGui::CollapsingHeader("Layout"))
    {
        if (ImGui::TreeNode("Child regions"))
        {
            static bool disable_mouse_wheel = false;
            ImGui::Checkbox("Disable Mouse Wheel", &disable_mouse_wheel);

            ImGui::Text("Without border");
            static int line = 50;
            bool goto_line = ImGui::Button("Goto");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
            goto_line |= ImGui::InputInt("##Line", &line, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::PopItemWidth();

            // Child 1: no border, enable horizontal scrollbar
            {
                ImGui::BeginChild("Child1", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 300), false, ImGuiWindowFlags_HorizontalScrollbar | (disable_mouse_wheel ? ImGuiWindowFlags_NoScrollWithMouse : 0));
                for (int i = 0; i < 100; i++)
                {
                    ImGui::Text("%04d: scrollable region", i);
                    if (goto_line && line == i)
                        ImGui::SetScrollHere();
                }
                if (goto_line && line >= 100)
                    ImGui::SetScrollHere();
                ImGui::EndChild();
            }

            ImGui::SameLine();

            // Child 2: rounded border
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                ImGui::BeginChild("Child2", ImVec2(0,300), true, (disable_mouse_wheel ? ImGuiWindowFlags_NoScrollWithMouse : 0));
                ImGui::Text("With border");
                ImGui::Columns(2);
                for (int i = 0; i < 100; i++)
                {
                    if (i == 50)
                        ImGui::NextColumn();
                    char buf[32];
                    sprintf(buf, "%08x", i*5731);
                    ImGui::Button(buf, ImVec2(-1.0f, 0.0f));
                }
                ImGui::EndChild();
                ImGui::PopStyleVar();
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Widgets Width"))
        {
            static float f = 0.0f;
            ImGui::Text("PushItemWidth(100)");
            ImGui::SameLine(); ShowHelpMarker("Fixed width.");
            ImGui::PushItemWidth(100);
            ImGui::DragFloat("float##1", &f);
            ImGui::PopItemWidth();

            ImGui::Text("PushItemWidth(GetWindowWidth() * 0.5f)");
            ImGui::SameLine(); ShowHelpMarker("Half of window width.");
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
            ImGui::DragFloat("float##2", &f);
            ImGui::PopItemWidth();

            ImGui::Text("PushItemWidth(GetContentRegionAvailWidth() * 0.5f)");
            ImGui::SameLine(); ShowHelpMarker("Half of available width.\n(~ right-cursor_pos)\n(works within a column set)");
            ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() * 0.5f);
            ImGui::DragFloat("float##3", &f);
            ImGui::PopItemWidth();

            ImGui::Text("PushItemWidth(-100)");
            ImGui::SameLine(); ShowHelpMarker("Align to right edge minus 100");
            ImGui::PushItemWidth(-100);
            ImGui::DragFloat("float##4", &f);
            ImGui::PopItemWidth();

            ImGui::Text("PushItemWidth(-1)");
            ImGui::SameLine(); ShowHelpMarker("Align to right edge");
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("float##5", &f);
            ImGui::PopItemWidth();

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Basic Horizontal Layout"))
        {
            ImGui::TextWrapped("(Use ImGui::SameLine() to keep adding items to the right of the preceding item)");

            // Text
            ImGui::Text("Two items: Hello"); ImGui::SameLine();
            ImGui::TextColored(ImVec4(1,1,0,1), "Sailor");

            // Adjust spacing
            ImGui::Text("More spacing: Hello"); ImGui::SameLine(0, 20);
            ImGui::TextColored(ImVec4(1,1,0,1), "Sailor");

            // Button
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Normal buttons"); ImGui::SameLine();
            ImGui::Button("Banana"); ImGui::SameLine();
            ImGui::Button("Apple"); ImGui::SameLine();
            ImGui::Button("Corniflower");

            // Button
            ImGui::Text("Small buttons"); ImGui::SameLine();
            ImGui::SmallButton("Like this one"); ImGui::SameLine();
            ImGui::Text("can fit within a text block.");

            // Aligned to arbitrary position. Easy/cheap column.
            ImGui::Text("Aligned");
            ImGui::SameLine(150); ImGui::Text("x=150");
            ImGui::SameLine(300); ImGui::Text("x=300");
            ImGui::Text("Aligned");
            ImGui::SameLine(150); ImGui::SmallButton("x=150");
            ImGui::SameLine(300); ImGui::SmallButton("x=300");

            // Checkbox
            static bool c1=false,c2=false,c3=false,c4=false;
            ImGui::Checkbox("My", &c1); ImGui::SameLine();
            ImGui::Checkbox("Tailor", &c2); ImGui::SameLine();
            ImGui::Checkbox("Is", &c3); ImGui::SameLine();
            ImGui::Checkbox("Rich", &c4);

            // Various
            static float f0=1.0f, f1=2.0f, f2=3.0f;
            ImGui::PushItemWidth(80);
            const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD" };
            static int item = -1;
            ImGui::Combo("Combo", &item, items, IM_ARRAYSIZE(items)); ImGui::SameLine();
            ImGui::SliderFloat("X", &f0, 0.0f,5.0f); ImGui::SameLine();
            ImGui::SliderFloat("Y", &f1, 0.0f,5.0f); ImGui::SameLine();
            ImGui::SliderFloat("Z", &f2, 0.0f,5.0f);
            ImGui::PopItemWidth();

            ImGui::PushItemWidth(80);
            ImGui::Text("Lists:");
            static int selection[4] = { 0, 1, 2, 3 };
            for (int i = 0; i < 4; i++)
            {
                if (i > 0) ImGui::SameLine();
                ImGui::PushID(i);
                ImGui::ListBox("", &selection[i], items, IM_ARRAYSIZE(items));
                ImGui::PopID();
                //if (ImGui::IsItemHovered()) ImGui::SetTooltip("ListBox %d hovered", i);
            }
            ImGui::PopItemWidth();

            // Dummy
            ImVec2 sz(30,30);
            ImGui::Button("A", sz); ImGui::SameLine();
            ImGui::Dummy(sz); ImGui::SameLine();
            ImGui::Button("B", sz);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Groups"))
        {
            ImGui::TextWrapped("(Using ImGui::BeginGroup()/EndGroup() to layout items. BeginGroup() basically locks the horizontal position. EndGroup() bundles the whole group so that you can use functions such as IsItemHovered() on it.)");
            ImGui::BeginGroup();
            {
                ImGui::BeginGroup();
                ImGui::Button("AAA");
                ImGui::SameLine();
                ImGui::Button("BBB");
                ImGui::SameLine();
                ImGui::BeginGroup();
                ImGui::Button("CCC");
                ImGui::Button("DDD");
                ImGui::EndGroup();
                ImGui::SameLine();
                ImGui::Button("EEE");
                ImGui::EndGroup();
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("First group hovered");
            }
            // Capture the group size and create widgets using the same size
            ImVec2 size = ImGui::GetItemRectSize();
            const float values[5] = { 0.5f, 0.20f, 0.80f, 0.60f, 0.25f };
            ImGui::PlotHistogram("##values", values, IM_ARRAYSIZE(values), 0, NULL, 0.0f, 1.0f, size);

            ImGui::Button("ACTION", ImVec2((size.x - ImGui::GetStyle().ItemSpacing.x)*0.5f,size.y));
            ImGui::SameLine();
            ImGui::Button("REACTION", ImVec2((size.x - ImGui::GetStyle().ItemSpacing.x)*0.5f,size.y));
            ImGui::EndGroup();
            ImGui::SameLine();

            ImGui::Button("LEVERAGE\nBUZZWORD", size);
            ImGui::SameLine();

            ImGui::ListBoxHeader("List", size);
            ImGui::Selectable("Selected", true);
            ImGui::Selectable("Not Selected", false);
            ImGui::ListBoxFooter();

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Text Baseline Alignment"))
        {
            ImGui::TextWrapped("(This is testing the vertical alignment that occurs on text to keep it at the same baseline as widgets. Lines only composed of text or \"small\" widgets fit in less vertical spaces than lines with normal widgets)");

            ImGui::Text("One\nTwo\nThree"); ImGui::SameLine();
            ImGui::Text("Hello\nWorld"); ImGui::SameLine();
            ImGui::Text("Banana");

            ImGui::Text("Banana"); ImGui::SameLine();
            ImGui::Text("Hello\nWorld"); ImGui::SameLine();
            ImGui::Text("One\nTwo\nThree");

            ImGui::Button("HOP##1"); ImGui::SameLine();
            ImGui::Text("Banana"); ImGui::SameLine();
            ImGui::Text("Hello\nWorld"); ImGui::SameLine();
            ImGui::Text("Banana");

            ImGui::Button("HOP##2"); ImGui::SameLine();
            ImGui::Text("Hello\nWorld"); ImGui::SameLine();
            ImGui::Text("Banana");

            ImGui::Button("TEST##1"); ImGui::SameLine();
            ImGui::Text("TEST"); ImGui::SameLine();
            ImGui::SmallButton("TEST##2");

            ImGui::AlignTextToFramePadding(); // If your line starts with text, call this to align it to upcoming widgets.
            ImGui::Text("Text aligned to Widget"); ImGui::SameLine();
            ImGui::Button("Widget##1"); ImGui::SameLine();
            ImGui::Text("Widget"); ImGui::SameLine();
            ImGui::SmallButton("Widget##2"); ImGui::SameLine();
            ImGui::Button("Widget##3");

            // Tree
            const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
            ImGui::Button("Button##1");
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::TreeNode("Node##1")) { for (int i = 0; i < 6; i++) ImGui::BulletText("Item %d..", i); ImGui::TreePop(); }    // Dummy tree data

            ImGui::AlignTextToFramePadding();         // Vertically align text node a bit lower so it'll be vertically centered with upcoming widget. Otherwise you can use SmallButton (smaller fit).
            bool node_open = ImGui::TreeNode("Node##2");  // Common mistake to avoid: if we want to SameLine after TreeNode we need to do it before we add child content.
            ImGui::SameLine(0.0f, spacing); ImGui::Button("Button##2");
            if (node_open) { for (int i = 0; i < 6; i++) ImGui::BulletText("Item %d..", i); ImGui::TreePop(); }   // Dummy tree data

            // Bullet
            ImGui::Button("Button##3");
            ImGui::SameLine(0.0f, spacing);
            ImGui::BulletText("Bullet text");

            ImGui::AlignTextToFramePadding();
            ImGui::BulletText("Node");
            ImGui::SameLine(0.0f, spacing); ImGui::Button("Button##4");

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Scrolling"))
        {
            ImGui::TextWrapped("(Use SetScrollHere() or SetScrollFromPosY() to scroll to a given position.)");
            static bool track = true;
            static int track_line = 50, scroll_to_px = 200;
            ImGui::Checkbox("Track", &track);
            ImGui::PushItemWidth(100);
            ImGui::SameLine(130); track |= ImGui::DragInt("##line", &track_line, 0.25f, 0, 99, "Line = %.0f");
            bool scroll_to = ImGui::Button("Scroll To Pos");
            ImGui::SameLine(130); scroll_to |= ImGui::DragInt("##pos_y", &scroll_to_px, 1.00f, 0, 9999, "Y = %.0f px");
            ImGui::PopItemWidth();
            if (scroll_to) track = false;

            for (int i = 0; i < 5; i++)
            {
                if (i > 0) ImGui::SameLine();
                ImGui::BeginGroup();
                ImGui::Text("%s", i == 0 ? "Top" : i == 1 ? "25%" : i == 2 ? "Center" : i == 3 ? "75%" : "Bottom");
                ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)i), ImVec2(ImGui::GetWindowWidth() * 0.17f, 200.0f), true);
                if (scroll_to)
                    ImGui::SetScrollFromPosY(ImGui::GetCursorStartPos().y + scroll_to_px, i * 0.25f);
                for (int line = 0; line < 100; line++)
                {
                    if (track && line == track_line)
                    {
                        ImGui::TextColored(ImColor(255,255,0), "Line %d", line);
                        ImGui::SetScrollHere(i * 0.25f); // 0.0f:top, 0.5f:center, 1.0f:bottom
                    }
                    else
                    {
                        ImGui::Text("Line %d", line);
                    }
                }
                float scroll_y = ImGui::GetScrollY(), scroll_max_y = ImGui::GetScrollMaxY();
                ImGui::EndChild();
                ImGui::Text("%.0f/%0.f", scroll_y, scroll_max_y);
                ImGui::EndGroup();
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Horizontal Scrolling"))
        {
            ImGui::Bullet(); ImGui::TextWrapped("Horizontal scrolling for a window has to be enabled explicitly via the ImGuiWindowFlags_HorizontalScrollbar flag.");
            ImGui::Bullet(); ImGui::TextWrapped("You may want to explicitly specify content width by calling SetNextWindowContentWidth() before Begin().");
            static int lines = 7;
            ImGui::SliderInt("Lines", &lines, 1, 15);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 1.0f));
            ImGui::BeginChild("scrolling", ImVec2(0, ImGui::GetFrameHeightWithSpacing()*7 + 30), true, ImGuiWindowFlags_HorizontalScrollbar);
            for (int line = 0; line < lines; line++)
            {
                // Display random stuff (for the sake of this trivial demo we are using basic Button+SameLine. If you want to create your own time line for a real application you may be better off 
                // manipulating the cursor position yourself, aka using SetCursorPos/SetCursorScreenPos to position the widgets yourself. You may also want to use the lower-level ImDrawList API)
                int num_buttons = 10 + ((line & 1) ? line * 9 : line * 3);
                for (int n = 0; n < num_buttons; n++)
                {
                    if (n > 0) ImGui::SameLine();
                    ImGui::PushID(n + line * 1000);
                    char num_buf[16];
                    sprintf(num_buf, "%d", n);
                    const char* label = (!(n%15)) ? "FizzBuzz" : (!(n%3)) ? "Fizz" : (!(n%5)) ? "Buzz" : num_buf;
                    float hue = n*0.05f;
                    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(hue, 0.6f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(hue, 0.7f, 0.7f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(hue, 0.8f, 0.8f));
                    ImGui::Button(label, ImVec2(40.0f + sinf((float)(line + n)) * 20.0f, 0.0f));
                    ImGui::PopStyleColor(3);
                    ImGui::PopID();
                }
            }
            float scroll_x = ImGui::GetScrollX(), scroll_max_x = ImGui::GetScrollMaxX();
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            float scroll_x_delta = 0.0f;
            ImGui::SmallButton("<<"); if (ImGui::IsItemActive()) scroll_x_delta = -ImGui::GetIO().DeltaTime * 1000.0f; ImGui::SameLine(); 
            ImGui::Text("Scroll from code"); ImGui::SameLine();
            ImGui::SmallButton(">>"); if (ImGui::IsItemActive()) scroll_x_delta = +ImGui::GetIO().DeltaTime * 1000.0f; ImGui::SameLine(); 
            ImGui::Text("%.0f/%.0f", scroll_x, scroll_max_x);
            if (scroll_x_delta != 0.0f)
            {
                ImGui::BeginChild("scrolling"); // Demonstrate a trick: you can use Begin to set yourself in the context of another window (here we are already out of your child window)
                ImGui::SetScrollX(ImGui::GetScrollX() + scroll_x_delta);
                ImGui::End();
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Clipping"))
        {
            static ImVec2 size(100, 100), offset(50, 20);
            ImGui::TextWrapped("On a per-widget basis we are occasionally clipping text CPU-side if it won't fit in its frame. Otherwise we are doing coarser clipping + passing a scissor rectangle to the renderer. The system is designed to try minimizing both execution and CPU/GPU rendering cost.");
            ImGui::DragFloat2("size", (float*)&size, 0.5f, 0.0f, 200.0f, "%.0f");
            ImGui::TextWrapped("(Click and drag)");
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec4 clip_rect(pos.x, pos.y, pos.x+size.x, pos.y+size.y);
            ImGui::InvisibleButton("##dummy", size);
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging()) { offset.x += ImGui::GetIO().MouseDelta.x; offset.y += ImGui::GetIO().MouseDelta.y; }
            ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x+size.x,pos.y+size.y), IM_COL32(90,90,120,255));
            ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize()*2.0f, ImVec2(pos.x+offset.x,pos.y+offset.y), IM_COL32(255,255,255,255), "Line 1 hello\nLine 2 clip me!", NULL, 0.0f, &clip_rect);
            ImGui::TreePop();
        }
    }

    if (ImGui::CollapsingHeader("Popups & Modal windows"))
    {
        if (ImGui::TreeNode("Popups"))
        {
            ImGui::TextWrapped("When a popup is active, it inhibits interacting with windows that are behind the popup. Clicking outside the popup closes it.");

            static int selected_fish = -1;
            const char* names[] = { "Bream", "Haddock", "Mackerel", "Pollock", "Tilefish" };
            static bool toggles[] = { true, false, false, false, false };

            // Simple selection popup
            // (If you want to show the current selection inside the Button itself, you may want to build a string using the "###" operator to preserve a constant ID with a variable label)
            if (ImGui::Button("Select.."))
                ImGui::OpenPopup("select");
            ImGui::SameLine();
            ImGui::TextUnformatted(selected_fish == -1 ? "<None>" : names[selected_fish]);
            if (ImGui::BeginPopup("select"))
            {
                ImGui::Text("Aquarium");
                ImGui::Separator();
                for (int i = 0; i < IM_ARRAYSIZE(names); i++)
                    if (ImGui::Selectable(names[i]))
                        selected_fish = i;
                ImGui::EndPopup();
            }

            // Showing a menu with toggles
            if (ImGui::Button("Toggle.."))
                ImGui::OpenPopup("toggle");
            if (ImGui::BeginPopup("toggle"))
            {
                for (int i = 0; i < IM_ARRAYSIZE(names); i++)
                    ImGui::MenuItem(names[i], "", &toggles[i]);
                if (ImGui::BeginMenu("Sub-menu"))
                {
                    ImGui::MenuItem("Click me");
                    ImGui::EndMenu();
                }

                ImGui::Separator();
                ImGui::Text("Tooltip here");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("I am a tooltip over a popup");

                if (ImGui::Button("Stacked Popup"))
                    ImGui::OpenPopup("another popup");
                if (ImGui::BeginPopup("another popup"))
                {
                    for (int i = 0; i < IM_ARRAYSIZE(names); i++)
                        ImGui::MenuItem(names[i], "", &toggles[i]);
                    if (ImGui::BeginMenu("Sub-menu"))
                    {
                        ImGui::MenuItem("Click me");
                        ImGui::EndMenu();
                    }
                    ImGui::EndPopup();
                }
                ImGui::EndPopup();
            }

            if (ImGui::Button("Popup Menu.."))
                ImGui::OpenPopup("FilePopup");
            if (ImGui::BeginPopup("FilePopup"))
            {
                ShowExampleMenuFile();
                ImGui::EndPopup();
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Context menus"))
        {
            // BeginPopupContextItem() is a helper to provide common/simple popup behavior of essentially doing:
            //    if (IsItemHovered() && IsMouseClicked(0))
            //       OpenPopup(id);
            //    return BeginPopup(id);
            // For more advanced uses you may want to replicate and cuztomize this code. This the comments inside BeginPopupContextItem() implementation.
            static float value = 0.5f;
            ImGui::Text("Value = %.3f (<-- right-click here)", value);
            if (ImGui::BeginPopupContextItem("item context menu"))
            {
                if (ImGui::Selectable("Set to zero")) value = 0.0f;
                if (ImGui::Selectable("Set to PI")) value = 3.1415f;
                ImGui::PushItemWidth(-1);
                ImGui::DragFloat("##Value", &value, 0.1f, 0.0f, 0.0f);
                ImGui::PopItemWidth();
                ImGui::EndPopup();
            }

            static char name[32] = "Label1";
            char buf[64]; sprintf(buf, "Button: %s###Button", name); // ### operator override ID ignoring the preceding label
            ImGui::Button(buf);
            if (ImGui::BeginPopupContextItem()) // When used after an item that has an ID (here the Button), we can skip providing an ID to BeginPopupContextItem().
            {
                ImGui::Text("Edit name:");
                ImGui::InputText("##edit", name, IM_ARRAYSIZE(name));
                if (ImGui::Button("Close"))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
            ImGui::SameLine(); ImGui::Text("(<-- right-click here)");

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Modals"))
        {
            ImGui::TextWrapped("Modal windows are like popups but the user cannot close them by clicking outside the window.");

            if (ImGui::Button("Delete.."))
                ImGui::OpenPopup("Delete?");
            if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                    ImGui::Text("All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n");
                ImGui::Separator();

                //static int dummy_i = 0;
                //ImGui::Combo("Combo", &dummy_i, "Delete\0Delete harder\0");

                static bool dont_ask_me_next_time = false;
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
                ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
                ImGui::PopStyleVar();

                if (ImGui::Button("OK", ImVec2(120,0))) { ImGui::CloseCurrentPopup(); }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120,0))) { ImGui::CloseCurrentPopup(); }
                ImGui::EndPopup();
            }

            if (ImGui::Button("Stacked modals.."))
                ImGui::OpenPopup("Stacked 1");
            if (ImGui::BeginPopupModal("Stacked 1"))
            {
                ImGui::Text("Hello from Stacked The First\nUsing style.Colors[ImGuiCol_ModalWindowDarkening] for darkening.");
                static int item = 1;
                ImGui::Combo("Combo", &item, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");
                static float color[4] = { 0.4f,0.7f,0.0f,0.5f };
                ImGui::ColorEdit4("color", color);  // This is to test behavior of stacked regular popups over a modal

                if (ImGui::Button("Add another modal.."))
                    ImGui::OpenPopup("Stacked 2");
                if (ImGui::BeginPopupModal("Stacked 2"))
                {
                    ImGui::Text("Hello from Stacked The Second!");
                    if (ImGui::Button("Close"))
                        ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }

                if (ImGui::Button("Close"))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Menus inside a regular window"))
        {
            ImGui::TextWrapped("Below we are testing adding menu items to a regular window. It's rather unusual but should work!");
            ImGui::Separator();
            // NB: As a quirk in this very specific example, we want to differentiate the parent of this menu from the parent of the various popup menus above.
            // To do so we are encloding the items in a PushID()/PopID() block to make them two different menusets. If we don't, opening any popup above and hovering our menu here
            // would open it. This is because once a menu is active, we allow to switch to a sibling menu by just hovering on it, which is the desired behavior for regular menus.
            ImGui::PushID("foo");
            ImGui::MenuItem("Menu item", "CTRL+M");
            if (ImGui::BeginMenu("Menu inside a regular window"))
            {
                ShowExampleMenuFile();
                ImGui::EndMenu();
            }
            ImGui::PopID();
            ImGui::Separator();
            ImGui::TreePop();
        }
    }

    if (ImGui::CollapsingHeader("Columns"))
    {
        ImGui::PushID("Columns");

        // Basic columns
        if (ImGui::TreeNode("Basic"))
        {
            ImGui::Text("Without border:");
            ImGui::Columns(3, "mycolumns3", false);  // 3-ways, no border
            ImGui::Separator();
            for (int n = 0; n < 14; n++)
            {
                char label[32];
                sprintf(label, "Item %d", n);
                if (ImGui::Selectable(label)) {}
                //if (ImGui::Button(label, ImVec2(-1,0))) {}
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::Separator();

            ImGui::Text("With border:");
            ImGui::Columns(4, "mycolumns"); // 4-ways, with border
            ImGui::Separator();
            ImGui::Text("ID"); ImGui::NextColumn();
            ImGui::Text("Name"); ImGui::NextColumn();
            ImGui::Text("Path"); ImGui::NextColumn();
            ImGui::Text("Hovered"); ImGui::NextColumn();
            ImGui::Separator();
            const char* names[3] = { "One", "Two", "Three" };
            const char* paths[3] = { "/path/one", "/path/two", "/path/three" };
            static int selected = -1;
            for (int i = 0; i < 3; i++)
            {
                char label[32];
                sprintf(label, "%04d", i);
                if (ImGui::Selectable(label, selected == i, ImGuiSelectableFlags_SpanAllColumns))
                    selected = i;
                bool hovered = ImGui::IsItemHovered();
                ImGui::NextColumn();
                ImGui::Text(names[i]); ImGui::NextColumn();
                ImGui::Text(paths[i]); ImGui::NextColumn();
                ImGui::Text("%d", hovered); ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::Separator();
            ImGui::TreePop();
        }

        // Create multiple items in a same cell before switching to next column
        if (ImGui::TreeNode("Mixed items"))
        {
            ImGui::Columns(3, "mixed");
            ImGui::Separator();

            ImGui::Text("Hello");
            ImGui::Button("Banana");
            ImGui::NextColumn();

            ImGui::Text("ImGui");
            ImGui::Button("Apple");
            static float foo = 1.0f;
            ImGui::InputFloat("red", &foo, 0.05f, 0, 3);
            ImGui::Text("An extra line here.");
            ImGui::NextColumn();

                ImGui::Text("Sailor");
            ImGui::Button("Corniflower");
            static float bar = 1.0f;
            ImGui::InputFloat("blue", &bar, 0.05f, 0, 3);
            ImGui::NextColumn();

            if (ImGui::CollapsingHeader("Category A")) { ImGui::Text("Blah blah blah"); } ImGui::NextColumn();
            if (ImGui::CollapsingHeader("Category B")) { ImGui::Text("Blah blah blah"); } ImGui::NextColumn();
            if (ImGui::CollapsingHeader("Category C")) { ImGui::Text("Blah blah blah"); } ImGui::NextColumn();
            ImGui::Columns(1);
            ImGui::Separator();
            ImGui::TreePop();
        }

        // Word wrapping
        if (ImGui::TreeNode("Word-wrapping"))
        {
            ImGui::Columns(2, "word-wrapping");
            ImGui::Separator();
            ImGui::TextWrapped("The quick brown fox jumps over the lazy dog.");
            ImGui::TextWrapped("Hello Left");
            ImGui::NextColumn();
            ImGui::TextWrapped("The quick brown fox jumps over the lazy dog.");
            ImGui::TextWrapped("Hello Right");
            ImGui::Columns(1);
            ImGui::Separator();
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Borders"))
        {
            // NB: Future columns API should allow automatic horizontal borders.
            static bool h_borders = true;
            static bool v_borders = true;
            ImGui::Checkbox("horizontal", &h_borders);
            ImGui::SameLine();
            ImGui::Checkbox("vertical", &v_borders);
            ImGui::Columns(4, NULL, v_borders);
            for (int i = 0; i < 4*3; i++)
            {
                if (h_borders && ImGui::GetColumnIndex() == 0)
                    ImGui::Separator();
                ImGui::Text("%c%c%c", 'a'+i, 'a'+i, 'a'+i);
                ImGui::Text("Width %.2f\nOffset %.2f", ImGui::GetColumnWidth(), ImGui::GetColumnOffset());
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
            if (h_borders)
                ImGui::Separator();
            ImGui::TreePop();
        }

        // Scrolling columns
        /*
        if (ImGui::TreeNode("Vertical Scrolling"))
        {
            ImGui::BeginChild("##header", ImVec2(0, ImGui::GetTextLineHeightWithSpacing()+ImGui::GetStyle().ItemSpacing.y));
            ImGui::Columns(3);
            ImGui::Text("ID"); ImGui::NextColumn();
            ImGui::Text("Name"); ImGui::NextColumn();
            ImGui::Text("Path"); ImGui::NextColumn();
            ImGui::Columns(1);
            ImGui::Separator();
            ImGui::EndChild();
            ImGui::BeginChild("##scrollingregion", ImVec2(0, 60));
            ImGui::Columns(3);
            for (int i = 0; i < 10; i++)
            {
                ImGui::Text("%04d", i); ImGui::NextColumn();
                ImGui::Text("Foobar"); ImGui::NextColumn();
                ImGui::Text("/path/foobar/%04d/", i); ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::EndChild();
            ImGui::TreePop();
        }
        */

        if (ImGui::TreeNode("Horizontal Scrolling"))
        {
            ImGui::SetNextWindowContentSize(ImVec2(1500.0f, 0.0f));
            ImGui::BeginChild("##ScrollingRegion", ImVec2(0, ImGui::GetFontSize() * 20), false, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::Columns(10);
            int ITEMS_COUNT = 2000;
            ImGuiListClipper clipper(ITEMS_COUNT);  // Also demonstrate using the clipper for large list
            while (clipper.Step())
            {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                    for (int j = 0; j < 10; j++)
                    {
                        ImGui::Text("Line %d Column %d...", i, j);
                        ImGui::NextColumn();
                    }
            }
            ImGui::Columns(1);
            ImGui::EndChild();
            ImGui::TreePop();
        }

        bool node_open = ImGui::TreeNode("Tree within single cell");
        ImGui::SameLine(); ShowHelpMarker("NB: Tree node must be poped before ending the cell. There's no storage of state per-cell.");
        if (node_open)
        {
            ImGui::Columns(2, "tree items");
            ImGui::Separator();
            if (ImGui::TreeNode("Hello")) { ImGui::BulletText("Sailor"); ImGui::TreePop(); } ImGui::NextColumn();
            if (ImGui::TreeNode("Bonjour")) { ImGui::BulletText("Marin"); ImGui::TreePop(); } ImGui::NextColumn();
            ImGui::Columns(1);
            ImGui::Separator();
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    if (ImGui::CollapsingHeader("Filtering"))
    {
        static ImGuiTextFilter filter;
        ImGui::Text("Filter usage:\n"
                    "  \"\"         display all lines\n"
                    "  \"xxx\"      display lines containing \"xxx\"\n"
                    "  \"xxx,yyy\"  display lines containing \"xxx\" or \"yyy\"\n"
                    "  \"-xxx\"     hide lines containing \"xxx\"");
        filter.Draw();
        const char* lines[] = { "aaa1.c", "bbb1.c", "ccc1.c", "aaa2.cpp", "bbb2.cpp", "ccc2.cpp", "abc.h", "hello, world" };
        for (int i = 0; i < IM_ARRAYSIZE(lines); i++)
            if (filter.PassFilter(lines[i]))
                ImGui::BulletText("%s", lines[i]);
    }

    if (ImGui::CollapsingHeader("Inputs & Focus"))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Checkbox("io.MouseDrawCursor", &io.MouseDrawCursor);
        ImGui::SameLine(); ShowHelpMarker("Request ImGui to render a mouse cursor for you in software. Note that a mouse cursor rendered via your application GPU rendering path will feel more laggy than hardware cursor, but will be more in sync with your other visuals.\n\nSome desktop applications may use both kinds of cursors (e.g. enable software cursor only when resizing/dragging something).");

        ImGui::Text("WantCaptureMouse: %d", io.WantCaptureMouse);
        ImGui::Text("WantCaptureKeyboard: %d", io.WantCaptureKeyboard);
        ImGui::Text("WantTextInput: %d", io.WantTextInput);
        ImGui::Text("WantMoveMouse: %d", io.WantMoveMouse);

        if (ImGui::TreeNode("Keyboard & Mouse State"))
        {
            if (ImGui::IsMousePosValid())
                ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
            else
                ImGui::Text("Mouse pos: <INVALID>");
            ImGui::Text("Mouse down:");     for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (io.MouseDownDuration[i] >= 0.0f)   { ImGui::SameLine(); ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }
            ImGui::Text("Mouse clicked:");  for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseClicked(i))          { ImGui::SameLine(); ImGui::Text("b%d", i); }
            ImGui::Text("Mouse dbl-clicked:"); for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseDoubleClicked(i)) { ImGui::SameLine(); ImGui::Text("b%d", i); }
            ImGui::Text("Mouse released:"); for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseReleased(i))         { ImGui::SameLine(); ImGui::Text("b%d", i); }
            ImGui::Text("Mouse wheel: %.1f", io.MouseWheel);

            ImGui::Text("Keys down:");      for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (io.KeysDownDuration[i] >= 0.0f)     { ImGui::SameLine(); ImGui::Text("%d (%.02f secs)", i, io.KeysDownDuration[i]); }
            ImGui::Text("Keys pressed:");   for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (ImGui::IsKeyPressed(i))             { ImGui::SameLine(); ImGui::Text("%d", i); }
            ImGui::Text("Keys release:");   for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (ImGui::IsKeyReleased(i))            { ImGui::SameLine(); ImGui::Text("%d", i); }
            ImGui::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");


            ImGui::Button("Hovering me sets the\nkeyboard capture flag");
            if (ImGui::IsItemHovered())
                ImGui::CaptureKeyboardFromApp(true);
            ImGui::SameLine();
            ImGui::Button("Holding me clears the\nthe keyboard capture flag");
            if (ImGui::IsItemActive())
                ImGui::CaptureKeyboardFromApp(false);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Tabbing"))
        {
            ImGui::Text("Use TAB/SHIFT+TAB to cycle through keyboard editable fields.");
            static char buf[32] = "dummy";
            ImGui::InputText("1", buf, IM_ARRAYSIZE(buf));
            ImGui::InputText("2", buf, IM_ARRAYSIZE(buf));
            ImGui::InputText("3", buf, IM_ARRAYSIZE(buf));
            ImGui::PushAllowKeyboardFocus(false);
            ImGui::InputText("4 (tab skip)", buf, IM_ARRAYSIZE(buf));
            //ImGui::SameLine(); ShowHelperMarker("Use ImGui::PushAllowKeyboardFocus(bool)\nto disable tabbing through certain widgets.");
            ImGui::PopAllowKeyboardFocus();
            ImGui::InputText("5", buf, IM_ARRAYSIZE(buf));
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Focus from code"))
        {
            bool focus_1 = ImGui::Button("Focus on 1"); ImGui::SameLine();
            bool focus_2 = ImGui::Button("Focus on 2"); ImGui::SameLine();
            bool focus_3 = ImGui::Button("Focus on 3");
            int has_focus = 0;
            static char buf[128] = "click on a button to set focus";

            if (focus_1) ImGui::SetKeyboardFocusHere();
            ImGui::InputText("1", buf, IM_ARRAYSIZE(buf));
            if (ImGui::IsItemActive()) has_focus = 1;

            if (focus_2) ImGui::SetKeyboardFocusHere();
            ImGui::InputText("2", buf, IM_ARRAYSIZE(buf));
            if (ImGui::IsItemActive()) has_focus = 2;

            ImGui::PushAllowKeyboardFocus(false);
            if (focus_3) ImGui::SetKeyboardFocusHere();
            ImGui::InputText("3 (tab skip)", buf, IM_ARRAYSIZE(buf));
            if (ImGui::IsItemActive()) has_focus = 3;
            ImGui::PopAllowKeyboardFocus();

            if (has_focus)
                ImGui::Text("Item with focus: %d", has_focus);
            else
                ImGui::Text("Item with focus: <none>");
            ImGui::TextWrapped("Cursor & selection are preserved when refocusing last used item in code.");
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Focused & Hovered Test"))
        {
            static bool embed_all_inside_a_child_window = false;
            ImGui::Checkbox("Embed everything inside a child window (for additional testing)", &embed_all_inside_a_child_window);
            if (embed_all_inside_a_child_window)
                ImGui::BeginChild("embeddingchild", ImVec2(0, ImGui::GetFontSize() * 25), true);

            // Testing IsWindowFocused() function with its various flags (note that the flags can be combined)
            ImGui::BulletText(
                "IsWindowFocused() = %d\n"
                "IsWindowFocused(_ChildWindows) = %d\n"
                "IsWindowFocused(_ChildWindows|_RootWindow) = %d\n"
                "IsWindowFocused(_RootWindow) = %d\n"
                "IsWindowFocused(_AnyWindow) = %d\n",
                ImGui::IsWindowFocused(),
                ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows),
                ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootWindow),
                ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow),
                ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow));

            // Testing IsWindowHovered() function with its various flags (note that the flags can be combined)
            ImGui::BulletText(
                "IsWindowHovered() = %d\n"
                "IsWindowHovered(_AllowWhenBlockedByPopup) = %d\n"
                "IsWindowHovered(_AllowWhenBlockedByActiveItem) = %d\n"
                "IsWindowHovered(_ChildWindows) = %d\n"
                "IsWindowHovered(_ChildWindows|_RootWindow) = %d\n"
                "IsWindowHovered(_RootWindow) = %d\n"
                "IsWindowHovered(_AnyWindow) = %d\n",
                ImGui::IsWindowHovered(),
                ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup),
                ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem),
                ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows),
                ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_RootWindow),
                ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow),
                ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow));

            // Testing IsItemHovered() function (because BulletText is an item itself and that would affect the output of IsItemHovered, we pass all lines in a single items to shorten the code)
            ImGui::Button("ITEM");
            ImGui::BulletText(
                "IsItemHovered() = %d\n"
                "IsItemHovered(_AllowWhenBlockedByPopup) = %d\n"
                "IsItemHovered(_AllowWhenBlockedByActiveItem) = %d\n"
                "IsItemHovered(_AllowWhenOverlapped) = %d\n"
                "IsItemhovered(_RectOnly) = %d\n",
                ImGui::IsItemHovered(),
                ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup),
                ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem),
                ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped),
                ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly));

            ImGui::BeginChild("child", ImVec2(0,50), true);
            ImGui::Text("This is another child window for testing IsWindowHovered() flags.");
            ImGui::EndChild();

            if (embed_all_inside_a_child_window)
                EndChild();

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Dragging"))
        {
            ImGui::TextWrapped("You can use ImGui::GetMouseDragDelta(0) to query for the dragged amount on any widget.");
            for (int button = 0; button < 3; button++)
                ImGui::Text("IsMouseDragging(%d):\n  w/ default threshold: %d,\n  w/ zero threshold: %d\n  w/ large threshold: %d", 
                    button, ImGui::IsMouseDragging(button), ImGui::IsMouseDragging(button, 0.0f), ImGui::IsMouseDragging(button, 20.0f));
            ImGui::Button("Drag Me");
            if (ImGui::IsItemActive())
            {
                // Draw a line between the button and the mouse cursor
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                draw_list->PushClipRectFullScreen();
                draw_list->AddLine(io.MouseClickedPos[0], io.MousePos, ImGui::GetColorU32(ImGuiCol_Button), 4.0f);
                draw_list->PopClipRect();

                // Drag operations gets "unlocked" when the mouse has moved past a certain threshold (the default threshold is stored in io.MouseDragThreshold)
                // You can request a lower or higher threshold using the second parameter of IsMouseDragging() and GetMouseDragDelta()
                ImVec2 value_raw = ImGui::GetMouseDragDelta(0, 0.0f);
                ImVec2 value_with_lock_threshold = ImGui::GetMouseDragDelta(0);
                ImVec2 mouse_delta = io.MouseDelta;
                ImGui::SameLine(); ImGui::Text("Raw (%.1f, %.1f), WithLockThresold (%.1f, %.1f), MouseDelta (%.1f, %.1f)", value_raw.x, value_raw.y, value_with_lock_threshold.x, value_with_lock_threshold.y, mouse_delta.x, mouse_delta.y);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Mouse cursors"))
        {
            const char* mouse_cursors_names[] = { "Arrow", "TextInput", "Move", "ResizeNS", "ResizeEW", "ResizeNESW", "ResizeNWSE" };
            IM_ASSERT(IM_ARRAYSIZE(mouse_cursors_names) == ImGuiMouseCursor_Count_);

            ImGui::Text("Current mouse cursor = %d: %s", ImGui::GetMouseCursor(), mouse_cursors_names[ImGui::GetMouseCursor()]);
            ImGui::Text("Hover to see mouse cursors:");
            ImGui::SameLine(); ShowHelpMarker("Your application can render a different mouse cursor based on what ImGui::GetMouseCursor() returns. If software cursor rendering (io.MouseDrawCursor) is set ImGui will draw the right cursor for you, otherwise your backend needs to handle it.");
            for (int i = 0; i < ImGuiMouseCursor_Count_; i++)
            {
                char label[32];
                sprintf(label, "Mouse cursor %d: %s", i, mouse_cursors_names[i]);
                ImGui::Bullet(); ImGui::Selectable(label, false);
                if (ImGui::IsItemHovered())
                    ImGui::SetMouseCursor(i);
            }
            ImGui::TreePop();
        }
    }

    ImGui::End();
}

// Demo helper function to select among default colors. See ShowStyleEditor() for more advanced options.
// Here we use the simplified Combo() api that packs items into a single literal string. Useful for quick combo boxes where the choices are known locally.
bool ImGui::ShowStyleSelector(const char* label)
{
    static int style_idx = 0;
    if (ImGui::Combo(label, &style_idx, "Classic\0Dark\0Light\0"))
    {
        switch (style_idx)
        {
        case 0: ImGui::StyleColorsClassic(); break;
        case 1: ImGui::StyleColorsDark(); break;
        case 2: ImGui::StyleColorsLight(); break;
        }
        return true;
    }
    return false;
}

// Demo helper function to select among loaded fonts.
// Here we use the regular BeginCombo()/EndCombo() api which is more the more flexible one.
void ImGui::ShowFontSelector(const char* label)
{
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font_current = ImGui::GetFont();
    if (ImGui::BeginCombo(label, font_current->GetDebugName()))
    {
        for (int n = 0; n < io.Fonts->Fonts.Size; n++)
            if (ImGui::Selectable(io.Fonts->Fonts[n]->GetDebugName(), io.Fonts->Fonts[n] == font_current))
                io.FontDefault = io.Fonts->Fonts[n];
        ImGui::EndCombo();
    }
    ImGui::SameLine(); 
    ShowHelpMarker(
        "- Load additional fonts with io.Fonts->AddFontFromFileTTF().\n"
        "- The font atlas is built when calling io.Fonts->GetTexDataAsXXXX() or io.Fonts->Build().\n"
        "- Read FAQ and documentation in extra_fonts/ for more details.\n"
        "- If you need to add/remove fonts at runtime (e.g. for DPI change), do it before calling NewFrame().");
}

void ImGui::ShowStyleEditor(ImGuiStyle* ref)
{
    // You can pass in a reference ImGuiStyle structure to compare to, revert to and save to (else it compares to an internally stored reference)
    ImGuiStyle& style = ImGui::GetStyle();
    static ImGuiStyle ref_saved_style;

    // Default to using internal storage as reference
    static bool init = true;
    if (init && ref == NULL)
        ref_saved_style = style;
    init = false;
    if (ref == NULL)
        ref = &ref_saved_style;

    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

    if (ImGui::ShowStyleSelector("Colors##Selector"))
        ref_saved_style = style;
    ImGui::ShowFontSelector("Fonts##Selector");

    // Simplified Settings
    if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f")) 
        style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
    { bool window_border = (style.WindowBorderSize > 0.0f); if (ImGui::Checkbox("WindowBorder", &window_border)) style.WindowBorderSize = window_border ? 1.0f : 0.0f; }
    ImGui::SameLine();
    { bool frame_border = (style.FrameBorderSize > 0.0f); if (ImGui::Checkbox("FrameBorder", &frame_border)) style.FrameBorderSize = frame_border ? 1.0f : 0.0f; }
    ImGui::SameLine();
    { bool popup_border = (style.PopupBorderSize > 0.0f); if (ImGui::Checkbox("PopupBorder", &popup_border)) style.PopupBorderSize = popup_border ? 1.0f : 0.0f; }

    // Save/Revert button
    if (ImGui::Button("Save Ref"))
        *ref = ref_saved_style = style;
    ImGui::SameLine();
    if (ImGui::Button("Revert Ref"))
        style = *ref;
    ImGui::SameLine();
    ShowHelpMarker("Save/Revert in local non-persistent storage. Default Colors definition are not affected. Use \"Export Colors\" below to save them somewhere.");

    if (ImGui::TreeNode("Rendering"))
    {
        ImGui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines); ImGui::SameLine(); ShowHelpMarker("When disabling anti-aliasing lines, you'll probably want to disable borders in your style as well.");
        ImGui::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
        ImGui::PushItemWidth(100);
        ImGui::DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, FLT_MAX, NULL, 2.0f);
        if (style.CurveTessellationTol < 0.0f) style.CurveTessellationTol = 0.10f;
        ImGui::DragFloat("Global Alpha", &style.Alpha, 0.005f, 0.20f, 1.0f, "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
        ImGui::PopItemWidth();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Settings"))
    {
        ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
        ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 16.0f, "%.0f");
        ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
        ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
        ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
        ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
        ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
        ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
        ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
        ImGui::Text("BorderSize");
        ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
        ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
        ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
        ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
        ImGui::Text("Rounding");
        ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 14.0f, "%.0f");
        ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 16.0f, "%.0f");
        ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
        ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
        ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
        ImGui::Text("Alignment");
        ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine(); ShowHelpMarker("Alignment applies when a button is larger than its text content.");
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Colors"))
    {
        static int output_dest = 0;
        static bool output_only_modified = true;
        if (ImGui::Button("Export Unsaved"))
        {
            if (output_dest == 0)
                ImGui::LogToClipboard();
            else
                ImGui::LogToTTY();
            ImGui::LogText("ImVec4* colors = ImGui::GetStyle().Colors;" IM_NEWLINE);
            for (int i = 0; i < ImGuiCol_COUNT; i++)
            {
                const ImVec4& col = style.Colors[i];
                const char* name = ImGui::GetStyleColorName(i);
                if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(ImVec4)) != 0)
                    ImGui::LogText("colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE, name, 23-(int)strlen(name), "", col.x, col.y, col.z, col.w);
            }
            ImGui::LogFinish();
        }
        ImGui::SameLine(); ImGui::PushItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0"); ImGui::PopItemWidth();
        ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

        ImGui::Text("Tip: Left-click on colored square to open color picker,\nRight-click to open edit options menu.");

        static ImGuiTextFilter filter;
        filter.Draw("Filter colors", 200);

        static ImGuiColorEditFlags alpha_flags = 0;
        ImGui::RadioButton("Opaque", &alpha_flags, 0); ImGui::SameLine(); 
        ImGui::RadioButton("Alpha", &alpha_flags, ImGuiColorEditFlags_AlphaPreview); ImGui::SameLine(); 
        ImGui::RadioButton("Both", &alpha_flags, ImGuiColorEditFlags_AlphaPreviewHalf);

        ImGui::BeginChild("#colors", ImVec2(0, 300), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
        ImGui::PushItemWidth(-160);
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName(i);
            if (!filter.PassFilter(name))
                continue;
            ImGui::PushID(i);
            ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
            if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
            {
                // Tips: in a real user application, you may want to merge and use an icon font into the main font, so instead of "Save"/"Revert" you'd use icons.
                // Read the FAQ and extra_fonts/README.txt about using icon fonts. It's really easy and super convenient!
                ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) ref->Colors[i] = style.Colors[i];
                ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) style.Colors[i] = ref->Colors[i];
            }
            ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
            ImGui::TextUnformatted(name);
            ImGui::PopID();
        }
        ImGui::PopItemWidth();
        ImGui::EndChild();

        ImGui::TreePop();
    }

    bool fonts_opened = ImGui::TreeNode("Fonts", "Fonts (%d)", ImGui::GetIO().Fonts->Fonts.Size);
    if (fonts_opened)
    {
        ImFontAtlas* atlas = ImGui::GetIO().Fonts;
        if (ImGui::TreeNode("Atlas texture", "Atlas texture (%dx%d pixels)", atlas->TexWidth, atlas->TexHeight))
        {
            ImGui::Image(atlas->TexID, ImVec2((float)atlas->TexWidth, (float)atlas->TexHeight), ImVec2(0,0), ImVec2(1,1), ImColor(255,255,255,255), ImColor(255,255,255,128));
            ImGui::TreePop();
        }
        ImGui::PushItemWidth(100);
        for (int i = 0; i < atlas->Fonts.Size; i++)
        {
            ImFont* font = atlas->Fonts[i];
            ImGui::PushID(font);
            bool font_details_opened = ImGui::TreeNode(font, "Font %d: \'%s\', %.2f px, %d glyphs", i, font->ConfigData ? font->ConfigData[0].Name : "", font->FontSize, font->Glyphs.Size);
            ImGui::SameLine(); if (ImGui::SmallButton("Set as default")) ImGui::GetIO().FontDefault = font;
            if (font_details_opened)
            {
                ImGui::PushFont(font);
                ImGui::Text("The quick brown fox jumps over the lazy dog");
                ImGui::PopFont();
                ImGui::DragFloat("Font scale", &font->Scale, 0.005f, 0.3f, 2.0f, "%.1f");   // Scale only this font
                ImGui::SameLine(); ShowHelpMarker("Note than the default embedded font is NOT meant to be scaled.\n\nFont are currently rendered into bitmaps at a given size at the time of building the atlas. You may oversample them to get some flexibility with scaling. You can also render at multiple sizes and select which one to use at runtime.\n\n(Glimmer of hope: the atlas system should hopefully be rewritten in the future to make scaling more natural and automatic.)");
                ImGui::Text("Ascent: %f, Descent: %f, Height: %f", font->Ascent, font->Descent, font->Ascent - font->Descent);
                ImGui::Text("Fallback character: '%c' (%d)", font->FallbackChar, font->FallbackChar);
                ImGui::Text("Texture surface: %d pixels (approx) ~ %dx%d", font->MetricsTotalSurface, (int)sqrtf((float)font->MetricsTotalSurface), (int)sqrtf((float)font->MetricsTotalSurface));
                for (int config_i = 0; config_i < font->ConfigDataCount; config_i++)
                {
                    ImFontConfig* cfg = &font->ConfigData[config_i];
                    ImGui::BulletText("Input %d: \'%s\', Oversample: (%d,%d), PixelSnapH: %d", config_i, cfg->Name, cfg->OversampleH, cfg->OversampleV, cfg->PixelSnapH);
                }
                if (ImGui::TreeNode("Glyphs", "Glyphs (%d)", font->Glyphs.Size))
                {
                    // Display all glyphs of the fonts in separate pages of 256 characters
                    const ImFontGlyph* glyph_fallback = font->FallbackGlyph; // Forcefully/dodgily make FindGlyph() return NULL on fallback, which isn't the default behavior.
                    font->FallbackGlyph = NULL;
                    for (int base = 0; base < 0x10000; base += 256)
                    {
                        int count = 0;
                        for (int n = 0; n < 256; n++)
                            count += font->FindGlyph((ImWchar)(base + n)) ? 1 : 0;
                        if (count > 0 && ImGui::TreeNode((void*)(intptr_t)base, "U+%04X..U+%04X (%d %s)", base, base+255, count, count > 1 ? "glyphs" : "glyph"))
                        {
                            float cell_spacing = style.ItemSpacing.y;
                            ImVec2 cell_size(font->FontSize * 1, font->FontSize * 1);
                            ImVec2 base_pos = ImGui::GetCursorScreenPos();
                            ImDrawList* draw_list = ImGui::GetWindowDrawList();
                            for (int n = 0; n < 256; n++)
                            {
                                ImVec2 cell_p1(base_pos.x + (n % 16) * (cell_size.x + cell_spacing), base_pos.y + (n / 16) * (cell_size.y + cell_spacing));
                                ImVec2 cell_p2(cell_p1.x + cell_size.x, cell_p1.y + cell_size.y);
                                const ImFontGlyph* glyph = font->FindGlyph((ImWchar)(base+n));;
                                draw_list->AddRect(cell_p1, cell_p2, glyph ? IM_COL32(255,255,255,100) : IM_COL32(255,255,255,50));
                                font->RenderChar(draw_list, cell_size.x, cell_p1, ImGui::GetColorU32(ImGuiCol_Text), (ImWchar)(base+n)); // We use ImFont::RenderChar as a shortcut because we don't have UTF-8 conversion functions available to generate a string.
                                if (glyph && ImGui::IsMouseHoveringRect(cell_p1, cell_p2))
                                {
                                    ImGui::BeginTooltip();
                                    ImGui::Text("Codepoint: U+%04X", base+n);
                                    ImGui::Separator();
                                    ImGui::Text("AdvanceX: %.1f", glyph->AdvanceX);
                                    ImGui::Text("Pos: (%.2f,%.2f)->(%.2f,%.2f)", glyph->X0, glyph->Y0, glyph->X1, glyph->Y1);
                                    ImGui::Text("UV: (%.3f,%.3f)->(%.3f,%.3f)", glyph->U0, glyph->V0, glyph->U1, glyph->V1);
                                    ImGui::EndTooltip();
                                }
                            }
                            ImGui::Dummy(ImVec2((cell_size.x + cell_spacing) * 16, (cell_size.y + cell_spacing) * 16));
                            ImGui::TreePop();
                        }
                    }
                    font->FallbackGlyph = glyph_fallback;
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        static float window_scale = 1.0f;
        ImGui::DragFloat("this window scale", &window_scale, 0.005f, 0.3f, 2.0f, "%.1f");              // scale only this window
        ImGui::DragFloat("global scale", &ImGui::GetIO().FontGlobalScale, 0.005f, 0.3f, 2.0f, "%.1f"); // scale everything
        ImGui::PopItemWidth();
        ImGui::SetWindowFontScale(window_scale);
        ImGui::TreePop();
    }

    ImGui::PopItemWidth();
}

// Demonstrate creating a fullscreen menu bar and populating it.
static void ShowExampleAppMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

static void ShowExampleMenuFile()
{
    ImGui::MenuItem("(dummy menu)", NULL, false, false);
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            if (ImGui::BeginMenu("Recurse.."))
            {
                ShowExampleMenuFile();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}
    ImGui::Separator();
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        static bool b = true;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::Checkbox("Check", &b);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Colors"))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImGui::ColorButton(name, ImGui::GetStyleColorVec4((ImGuiCol)i));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::PopStyleVar();
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}

// Demonstrate creating a window which gets auto-resized according to its content.
static void ShowExampleAppAutoResize(bool* p_open)
{
    if (!ImGui::Begin("Example: Auto-resizing window", p_open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }

    static int lines = 10;
    ImGui::Text("Window will resize every-frame to the size of its content.\nNote that you probably don't want to query the window size to\noutput your content because that would create a feedback loop.");
    ImGui::SliderInt("Number of lines", &lines, 1, 20);
    for (int i = 0; i < lines; i++)
        ImGui::Text("%*sThis is line %d", i*4, "", i); // Pad with space to extend size horizontally
    ImGui::End();
}

// Demonstrate creating a window with custom resize constraints.
static void ShowExampleAppConstrainedResize(bool* p_open)
{
    struct CustomConstraints // Helper functions to demonstrate programmatic constraints
    {
        static void Square(ImGuiSizeCallbackData* data) { data->DesiredSize = ImVec2(IM_MAX(data->DesiredSize.x, data->DesiredSize.y), IM_MAX(data->DesiredSize.x, data->DesiredSize.y)); }
        static void Step(ImGuiSizeCallbackData* data)   { float step = (float)(int)(intptr_t)data->UserData; data->DesiredSize = ImVec2((int)(data->DesiredSize.x / step + 0.5f) * step, (int)(data->DesiredSize.y / step + 0.5f) * step); }
    };

    static bool auto_resize = false;
    static int type = 0;
    static int display_lines = 10;
    if (type == 0) ImGui::SetNextWindowSizeConstraints(ImVec2(-1, 0),    ImVec2(-1, FLT_MAX));      // Vertical only
    if (type == 1) ImGui::SetNextWindowSizeConstraints(ImVec2(0, -1),    ImVec2(FLT_MAX, -1));      // Horizontal only
    if (type == 2) ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(FLT_MAX, FLT_MAX)); // Width > 100, Height > 100
    if (type == 3) ImGui::SetNextWindowSizeConstraints(ImVec2(400, -1),  ImVec2(500, -1));          // Width 400-500
    if (type == 4) ImGui::SetNextWindowSizeConstraints(ImVec2(-1, 400),  ImVec2(-1, 500));          // Height 400-500
    if (type == 5) ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0),     ImVec2(FLT_MAX, FLT_MAX), CustomConstraints::Square);          // Always Square
    if (type == 6) ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0),     ImVec2(FLT_MAX, FLT_MAX), CustomConstraints::Step, (void*)100);// Fixed Step

    ImGuiWindowFlags flags = auto_resize ? ImGuiWindowFlags_AlwaysAutoResize : 0;
    if (ImGui::Begin("Example: Constrained Resize", p_open, flags))
    {
        const char* desc[] = 
        {
            "Resize vertical only",
            "Resize horizontal only",
            "Width > 100, Height > 100",
            "Width 400-500",
            "Height 400-500",
            "Custom: Always Square",
            "Custom: Fixed Steps (100)",
        };
        if (ImGui::Button("200x200")) { ImGui::SetWindowSize(ImVec2(200, 200)); } ImGui::SameLine();
        if (ImGui::Button("500x500")) { ImGui::SetWindowSize(ImVec2(500, 500)); } ImGui::SameLine();
        if (ImGui::Button("800x200")) { ImGui::SetWindowSize(ImVec2(800, 200)); }
        ImGui::PushItemWidth(200);
        ImGui::Combo("Constraint", &type, desc, IM_ARRAYSIZE(desc));
        ImGui::DragInt("Lines", &display_lines, 0.2f, 1, 100);
        ImGui::PopItemWidth();
        ImGui::Checkbox("Auto-resize", &auto_resize);
        for (int i = 0; i < display_lines; i++)
            ImGui::Text("%*sHello, sailor! Making this line long enough for the example.", i * 4, "");
    }
    ImGui::End();
}

// Demonstrate creating a simple static window with no decoration + a context-menu to choose which corner of the screen to use.
static void ShowExampleAppFixedOverlay(bool* p_open)
{
    const float DISTANCE = 10.0f;
    static int corner = 0;
    ImVec2 window_pos = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE : DISTANCE);
    ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
    if (ImGui::Begin("Example: Fixed Overlay", p_open, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::Text("Simple overlay\nin the corner of the screen.\n(right-click to change position)");
        ImGui::Separator();
        ImGui::Text("Mouse Position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
            if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
            if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
            if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
            if (p_open && ImGui::MenuItem("Close")) *p_open = false; 
            ImGui::EndPopup();
        }
        ImGui::End();
    }
}

// Demonstrate using "##" and "###" in identifiers to manipulate ID generation.
// This apply to regular items as well. Read FAQ section "How can I have multiple widgets with the same label? Can I have widget without a label? (Yes). A primer on the purpose of labels/IDs." for details.
static void ShowExampleAppWindowTitles(bool*)
{
    // By default, Windows are uniquely identified by their title.
    // You can use the "##" and "###" markers to manipulate the display/ID.

    // Using "##" to display same title but have unique identifier.
    ImGui::SetNextWindowPos(ImVec2(100,100), ImGuiCond_FirstUseEver);
    ImGui::Begin("Same title as another window##1");
    ImGui::Text("This is window 1.\nMy title is the same as window 2, but my identifier is unique.");
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(100,200), ImGuiCond_FirstUseEver);
    ImGui::Begin("Same title as another window##2");
    ImGui::Text("This is window 2.\nMy title is the same as window 1, but my identifier is unique.");
    ImGui::End();

    // Using "###" to display a changing title but keep a static identifier "AnimatedTitle"
    char buf[128];
    sprintf(buf, "Animated title %c %d###AnimatedTitle", "|/-\\"[(int)(ImGui::GetTime()/0.25f)&3], ImGui::GetFrameCount());
    ImGui::SetNextWindowPos(ImVec2(100,300), ImGuiCond_FirstUseEver);
    ImGui::Begin(buf);
    ImGui::Text("This window has a changing title.");
    ImGui::End();
}

// Demonstrate using the low-level ImDrawList to draw custom shapes. 
static void ShowExampleAppCustomRendering(bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(350,560), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Example: Custom rendering", p_open))
    {
        ImGui::End();
        return;
    }

    // Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types and benefit of overloaded operators, etc.
    // Define IM_VEC2_CLASS_EXTRA in imconfig.h to create implicit conversions between your types and ImVec2/ImVec4.
    // ImGui defines overloaded operators but they are internal to imgui.cpp and not exposed outside (to avoid messing with your types)
    // In this example we are not using the maths operators!
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Primitives
    ImGui::Text("Primitives");
    static float sz = 36.0f;
    static ImVec4 col = ImVec4(1.0f,1.0f,0.4f,1.0f);
    ImGui::DragFloat("Size", &sz, 0.2f, 2.0f, 72.0f, "%.0f");
    ImGui::ColorEdit3("Color", &col.x);
    {
        const ImVec2 p = ImGui::GetCursorScreenPos();
        const ImU32 col32 = ImColor(col);
        float x = p.x + 4.0f, y = p.y + 4.0f, spacing = 8.0f;
        for (int n = 0; n < 2; n++)
        {
            float thickness = (n == 0) ? 1.0f : 4.0f;
            draw_list->AddCircle(ImVec2(x+sz*0.5f, y+sz*0.5f), sz*0.5f, col32, 20, thickness); x += sz+spacing;
            draw_list->AddRect(ImVec2(x, y), ImVec2(x+sz, y+sz), col32, 0.0f, ImDrawCornerFlags_All, thickness); x += sz+spacing;
            draw_list->AddRect(ImVec2(x, y), ImVec2(x+sz, y+sz), col32, 10.0f, ImDrawCornerFlags_All, thickness); x += sz+spacing;
            draw_list->AddRect(ImVec2(x, y), ImVec2(x+sz, y+sz), col32, 10.0f, ImDrawCornerFlags_TopLeft|ImDrawCornerFlags_BotRight, thickness); x += sz+spacing;
            draw_list->AddTriangle(ImVec2(x+sz*0.5f, y), ImVec2(x+sz,y+sz-0.5f), ImVec2(x,y+sz-0.5f), col32, thickness); x += sz+spacing;
            draw_list->AddLine(ImVec2(x, y), ImVec2(x+sz, y   ), col32, thickness); x += sz+spacing;
            draw_list->AddLine(ImVec2(x, y), ImVec2(x+sz, y+sz), col32, thickness); x += sz+spacing;
            draw_list->AddLine(ImVec2(x, y), ImVec2(x,    y+sz), col32, thickness); x += spacing;
            draw_list->AddBezierCurve(ImVec2(x, y), ImVec2(x+sz*1.3f,y+sz*0.3f), ImVec2(x+sz-sz*1.3f,y+sz-sz*0.3f), ImVec2(x+sz, y+sz), col32, thickness);
            x = p.x + 4;
            y += sz+spacing;
        }
        draw_list->AddCircleFilled(ImVec2(x+sz*0.5f, y+sz*0.5f), sz*0.5f, col32, 32); x += sz+spacing;
        draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x+sz, y+sz), col32); x += sz+spacing;
        draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x+sz, y+sz), col32, 10.0f); x += sz+spacing;
        draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x+sz, y+sz), col32, 10.0f, ImDrawCornerFlags_TopLeft|ImDrawCornerFlags_BotRight); x += sz+spacing;
        draw_list->AddTriangleFilled(ImVec2(x+sz*0.5f, y), ImVec2(x+sz,y+sz-0.5f), ImVec2(x,y+sz-0.5f), col32); x += sz+spacing;
        draw_list->AddRectFilledMultiColor(ImVec2(x, y), ImVec2(x+sz, y+sz), IM_COL32(0,0,0,255), IM_COL32(255,0,0,255), IM_COL32(255,255,0,255), IM_COL32(0,255,0,255));
        ImGui::Dummy(ImVec2((sz+spacing)*8, (sz+spacing)*3));
    }
    ImGui::Separator();
    {
        static ImVector<ImVec2> points;
        static bool adding_line = false;
        ImGui::Text("Canvas example");
        if (ImGui::Button("Clear")) points.clear();
        if (points.Size >= 2) { ImGui::SameLine(); if (ImGui::Button("Undo")) { points.pop_back(); points.pop_back(); } }
        ImGui::Text("Left-click and drag to add lines,\nRight-click to undo");

        // Here we are using InvisibleButton() as a convenience to 1) advance the cursor and 2) allows us to use IsItemHovered()
        // However you can draw directly and poll mouse/keyboard by yourself. You can manipulate the cursor using GetCursorPos() and SetCursorPos().
        // If you only use the ImDrawList API, you can notify the owner window of its extends by using SetCursorPos(max).
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
        if (canvas_size.x < 50.0f) canvas_size.x = 50.0f;
        if (canvas_size.y < 50.0f) canvas_size.y = 50.0f;
        draw_list->AddRectFilledMultiColor(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(50,50,50,255), IM_COL32(50,50,60,255), IM_COL32(60,60,70,255), IM_COL32(50,50,60,255));
        draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(255,255,255,255));

        bool adding_preview = false;
        ImGui::InvisibleButton("canvas", canvas_size);
        ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y);
        if (adding_line)
        {
            adding_preview = true;
            points.push_back(mouse_pos_in_canvas);
            if (!ImGui::IsMouseDown(0))
                adding_line = adding_preview = false;
        }
        if (ImGui::IsItemHovered())
        {
            if (!adding_line && ImGui::IsMouseClicked(0))
            {
                points.push_back(mouse_pos_in_canvas);
                adding_line = true;
            }
            if (ImGui::IsMouseClicked(1) && !points.empty())
            {
                adding_line = adding_preview = false;
                points.pop_back();
                points.pop_back();
            }
        }
        draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x+canvas_size.x, canvas_pos.y+canvas_size.y), true);      // clip lines within the canvas (if we resize it, etc.)
        for (int i = 0; i < points.Size - 1; i += 2)
            draw_list->AddLine(ImVec2(canvas_pos.x + points[i].x, canvas_pos.y + points[i].y), ImVec2(canvas_pos.x + points[i+1].x, canvas_pos.y + points[i+1].y), IM_COL32(255,255,0,255), 2.0f);
        draw_list->PopClipRect();
        if (adding_preview)
            points.pop_back();
    }
    ImGui::End();
}

// Demonstrating creating a simple console window, with scrolling, filtering, completion and history.
// For the console example, here we are using a more C++ like approach of declaring a class to hold the data and the functions.
struct ExampleAppConsole
{
    char                  InputBuf[256];
    ImVector<char*>       Items;
    bool                  ScrollToBottom;
    ImVector<char*>       History;
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    ImVector<const char*> Commands;

    ExampleAppConsole()
    {
        ClearLog();
        memset(InputBuf, 0, sizeof(InputBuf));
        HistoryPos = -1;
        Commands.push_back("HELP");
        Commands.push_back("HISTORY");
        Commands.push_back("CLEAR");
        Commands.push_back("CLASSIFY");  // "classify" is here to provide an example of "C"+[tab] completing to "CL" and displaying matches.
        AddLog("Welcome to ImGui!");
    }
    ~ExampleAppConsole()
    {
        ClearLog();
        for (int i = 0; i < History.Size; i++)
            free(History[i]);
    }

    // Portable helpers
    static int   Stricmp(const char* str1, const char* str2)         { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
    static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
    static char* Strdup(const char *str)                             { size_t len = strlen(str) + 1; void* buff = malloc(len); return (char*)memcpy(buff, (const void*)str, len); }

    void    ClearLog()
    {
        for (int i = 0; i < Items.Size; i++)
            free(Items[i]);
        Items.clear();
        ScrollToBottom = true;
    }

    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        // FIXME-OPT
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
        buf[IM_ARRAYSIZE(buf)-1] = 0;
        va_end(args);
        Items.push_back(Strdup(buf));
        ScrollToBottom = true;
    }

    void    Draw(const char* title, bool* p_open)
    {
        ImGui::SetNextWindowSize(ImVec2(520,600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar. So e.g. IsItemHovered() will return true when hovering the title bar.
        // Here we create a context menu only available from the title bar.
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Close"))
                *p_open = false;
            ImGui::EndPopup();
        }

        ImGui::TextWrapped("This example implements a console with basic coloring, completion and history. A more elaborate implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
        ImGui::TextWrapped("Enter 'HELP' for help, press TAB to use text completion.");

        // TODO: display items starting from the bottom

        if (ImGui::SmallButton("Add Dummy Text")) { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); } ImGui::SameLine();
        if (ImGui::SmallButton("Add Dummy Error")) { AddLog("[error] something went wrong"); } ImGui::SameLine();
        if (ImGui::SmallButton("Clear")) { ClearLog(); } ImGui::SameLine();
        bool copy_to_clipboard = ImGui::SmallButton("Copy"); ImGui::SameLine();
        if (ImGui::SmallButton("Scroll to bottom")) ScrollToBottom = true;
        //static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }

        ImGui::Separator();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
        static ImGuiTextFilter filter;
        filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
        ImGui::PopStyleVar();
        ImGui::Separator();

        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::Selectable("Clear")) ClearLog();
            ImGui::EndPopup();
        }

        // Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
        // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
        // You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
        // To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
        //     ImGuiListClipper clipper(Items.Size);
        //     while (clipper.Step())
        //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        // However take note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
        // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
        // and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
        // If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,1)); // Tighten spacing
        if (copy_to_clipboard)
            ImGui::LogToClipboard();
        ImVec4 col_default_text = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        for (int i = 0; i < Items.Size; i++)
        {
            const char* item = Items[i];
            if (!filter.PassFilter(item))
                continue;
            ImVec4 col = col_default_text;
            if (strstr(item, "[error]")) col = ImColor(1.0f,0.4f,0.4f,1.0f);
            else if (strncmp(item, "# ", 2) == 0) col = ImColor(1.0f,0.78f,0.58f,1.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, col);
            ImGui::TextUnformatted(item);
            ImGui::PopStyleColor();
        }
        if (copy_to_clipboard)
            ImGui::LogFinish();
        if (ScrollToBottom)
            ImGui::SetScrollHere();
        ScrollToBottom = false;
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Separator();

        // Command-line
        if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackCompletion|ImGuiInputTextFlags_CallbackHistory, &TextEditCallbackStub, (void*)this))
        {
            char* input_end = InputBuf+strlen(InputBuf);
            while (input_end > InputBuf && input_end[-1] == ' ') { input_end--; } *input_end = 0;
            if (InputBuf[0])
                ExecCommand(InputBuf);
            strcpy(InputBuf, "");
        }

        // Demonstrate keeping auto focus on the input box
        if (ImGui::IsItemHovered() || (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

        ImGui::End();
    }

    void    ExecCommand(const char* command_line)
    {
        AddLog("# %s\n", command_line);

        // Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
        HistoryPos = -1;
        for (int i = History.Size-1; i >= 0; i--)
            if (Stricmp(History[i], command_line) == 0)
            {
                free(History[i]);
                History.erase(History.begin() + i);
                break;
            }
        History.push_back(Strdup(command_line));

        // Process command
        if (Stricmp(command_line, "CLEAR") == 0)
        {
            ClearLog();
        }
        else if (Stricmp(command_line, "HELP") == 0)
        {
            AddLog("Commands:");
            for (int i = 0; i < Commands.Size; i++)
                AddLog("- %s", Commands[i]);
        }
        else if (Stricmp(command_line, "HISTORY") == 0)
        {
            int first = History.Size - 10;
            for (int i = first > 0 ? first : 0; i < History.Size; i++)
                AddLog("%3d: %s\n", i, History[i]);
        }
        else
        {
            AddLog("Unknown command: '%s'\n", command_line);
        }
    }

    static int TextEditCallbackStub(ImGuiTextEditCallbackData* data) // In C++11 you are better off using lambdas for this sort of forwarding callbacks
    {
        ExampleAppConsole* console = (ExampleAppConsole*)data->UserData;
        return console->TextEditCallback(data);
    }

    int     TextEditCallback(ImGuiTextEditCallbackData* data)
    {
        //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
        switch (data->EventFlag)
        {
        case ImGuiInputTextFlags_CallbackCompletion:
            {
                // Example of TEXT COMPLETION

                // Locate beginning of current word
                const char* word_end = data->Buf + data->CursorPos;
                const char* word_start = word_end;
                while (word_start > data->Buf)
                {
                    const char c = word_start[-1];
                    if (c == ' ' || c == '\t' || c == ',' || c == ';')
                        break;
                    word_start--;
                }

                // Build a list of candidates
                ImVector<const char*> candidates;
                for (int i = 0; i < Commands.Size; i++)
                    if (Strnicmp(Commands[i], word_start, (int)(word_end-word_start)) == 0)
                        candidates.push_back(Commands[i]);

                if (candidates.Size == 0)
                {
                    // No match
                    AddLog("No match for \"%.*s\"!\n", (int)(word_end-word_start), word_start);
                }
                else if (candidates.Size == 1)
                {
                    // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
                    data->DeleteChars((int)(word_start-data->Buf), (int)(word_end-word_start));
                    data->InsertChars(data->CursorPos, candidates[0]);
                    data->InsertChars(data->CursorPos, " ");
                }
                else
                {
                    // Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
                    int match_len = (int)(word_end - word_start);
                    for (;;)
                    {
                        int c = 0;
                        bool all_candidates_matches = true;
                        for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                            if (i == 0)
                                c = toupper(candidates[i][match_len]);
                            else if (c == 0 || c != toupper(candidates[i][match_len]))
                                all_candidates_matches = false;
                        if (!all_candidates_matches)
                            break;
                        match_len++;
                    }

                    if (match_len > 0)
                    {
                        data->DeleteChars((int)(word_start - data->Buf), (int)(word_end-word_start));
                        data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                    }

                    // List matches
                    AddLog("Possible matches:\n");
                    for (int i = 0; i < candidates.Size; i++)
                        AddLog("- %s\n", candidates[i]);
                }

                break;
            }
        case ImGuiInputTextFlags_CallbackHistory:
            {
                // Example of HISTORY
                const int prev_history_pos = HistoryPos;
                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    if (HistoryPos == -1)
                        HistoryPos = History.Size - 1;
                    else if (HistoryPos > 0)
                        HistoryPos--;
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    if (HistoryPos != -1)
                        if (++HistoryPos >= History.Size)
                            HistoryPos = -1;
                }

                // A better implementation would preserve the data on the current input line along with cursor position.
                if (prev_history_pos != HistoryPos)
                {
                    data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = (int)snprintf(data->Buf, (size_t)data->BufSize, "%s", (HistoryPos >= 0) ? History[HistoryPos] : "");
                    data->BufDirty = true;
                }
            }
        }
        return 0;
    }
};

static void ShowExampleAppConsole(bool* p_open)
{
    static ExampleAppConsole console;
    console.Draw("Example: Console", p_open);
}

// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
struct ExampleAppLog
{
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets;        // Index to lines offset
    bool                ScrollToBottom;

    void    Clear()     { Buf.clear(); LineOffsets.clear(); }

    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        int old_size = Buf.size();
        va_list args;
        va_start(args, fmt);
        Buf.appendfv(fmt, args);
        va_end(args);
        for (int new_size = Buf.size(); old_size < new_size; old_size++)
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size);
        ScrollToBottom = true;
    }

    void    Draw(const char* title, bool* p_open = NULL)
    {
        ImGui::SetNextWindowSize(ImVec2(500,400), ImGuiCond_FirstUseEver);
        ImGui::Begin(title, p_open);
        if (ImGui::Button("Clear")) Clear();
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -100.0f);
        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);
        if (copy) ImGui::LogToClipboard();

        if (Filter.IsActive())
        {
            const char* buf_begin = Buf.begin();
            const char* line = buf_begin;
            for (int line_no = 0; line != NULL; line_no++)
            {
                const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : NULL;
                if (Filter.PassFilter(line, line_end))
                    ImGui::TextUnformatted(line, line_end);
                line = line_end && line_end[1] ? line_end + 1 : NULL;
            }
        }
        else
        {
            ImGui::TextUnformatted(Buf.begin());
        }

        if (ScrollToBottom)
            ImGui::SetScrollHere(1.0f);
        ScrollToBottom = false;
        ImGui::EndChild();
        ImGui::End();
    }
};

// Demonstrate creating a simple log window with basic filtering.
static void ShowExampleAppLog(bool* p_open)
{
    static ExampleAppLog log;

    // Demo: add random items (unless Ctrl is held)
    static float last_time = -1.0f;
    float time = ImGui::GetTime();
    if (time - last_time >= 0.20f && !ImGui::GetIO().KeyCtrl)
    {
        const char* random_words[] = { "system", "info", "warning", "error", "fatal", "notice", "log" };
        log.AddLog("[%s] Hello, time is %.1f, frame count is %d\n", random_words[rand() % IM_ARRAYSIZE(random_words)], time, ImGui::GetFrameCount());
        last_time = time;
    }

    log.Draw("Example: Log", p_open);
}

// Demonstrate create a window with multiple child windows.
static void ShowExampleAppLayout(bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Example: Layout", p_open, ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Close")) *p_open = false;
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // left
        static int selected = 0;
        ImGui::BeginChild("left pane", ImVec2(150, 0), true);
        for (int i = 0; i < 100; i++)
        {
            char label[128];
            sprintf(label, "MyObject %d", i);
            if (ImGui::Selectable(label, selected == i))
                selected = i;
        }
        ImGui::EndChild();
        ImGui::SameLine();

        // right
        ImGui::BeginGroup();
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
                ImGui::Text("MyObject: %d", selected);
                ImGui::Separator();
                ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
            ImGui::EndChild();
            if (ImGui::Button("Revert")) {}
            ImGui::SameLine();
            if (ImGui::Button("Save")) {}
        ImGui::EndGroup();
    }
    ImGui::End();
}

// Demonstrate create a simple property editor.
static void ShowExampleAppPropertyEditor(bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(430,450), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Example: Property editor", p_open))
    {
        ImGui::End();
        return;
    }

    ShowHelpMarker("This example shows how you may implement a property editor using two columns.\nAll objects/fields data are dummies here.\nRemember that in many simple cases, you can use ImGui::SameLine(xxx) to position\nyour cursor horizontally instead of using the Columns() API.");

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2,2));
    ImGui::Columns(2);
    ImGui::Separator();

    struct funcs
    {
        static void ShowDummyObject(const char* prefix, int uid)
        {
            ImGui::PushID(uid);                      // Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
            ImGui::AlignTextToFramePadding();  // Text and Tree nodes are less high than regular widgets, here we add vertical spacing to make the tree lines equal high.
            bool node_open = ImGui::TreeNode("Object", "%s_%u", prefix, uid);
            ImGui::NextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("my sailor is rich");
            ImGui::NextColumn();
            if (node_open)
            {
                static float dummy_members[8] = { 0.0f,0.0f,1.0f,3.1416f,100.0f,999.0f };
                for (int i = 0; i < 8; i++)
                {
                    ImGui::PushID(i); // Use field index as identifier.
                    if (i < 2)
                    {
                        ShowDummyObject("Child", 424242);
                    }
                    else
                    {
                        ImGui::AlignTextToFramePadding();
                        // Here we use a Selectable (instead of Text) to highlight on hover
                        //ImGui::Text("Field_%d", i);
                        char label[32];
                        sprintf(label, "Field_%d", i);
                        ImGui::Bullet();
                        ImGui::Selectable(label);
                        ImGui::NextColumn();
                        ImGui::PushItemWidth(-1);
                        if (i >= 5)
                            ImGui::InputFloat("##value", &dummy_members[i], 1.0f);
                        else
                            ImGui::DragFloat("##value", &dummy_members[i], 0.01f);
                        ImGui::PopItemWidth();
                        ImGui::NextColumn();
                    }
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    };

    // Iterate dummy objects with dummy members (all the same data)
    for (int obj_i = 0; obj_i < 3; obj_i++)
        funcs::ShowDummyObject("Object", obj_i);

    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::PopStyleVar();
    ImGui::End();
}

// Demonstrate/test rendering huge amount of text, and the incidence of clipping.
static void ShowExampleAppLongText(bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(520,600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Example: Long text display", p_open))
    {
        ImGui::End();
        return;
    }

    static int test_type = 0;
    static ImGuiTextBuffer log;
    static int lines = 0;
    ImGui::Text("Printing unusually long amount of text.");
    ImGui::Combo("Test type", &test_type, "Single call to TextUnformatted()\0Multiple calls to Text(), clipped manually\0Multiple calls to Text(), not clipped (slow)\0");
    ImGui::Text("Buffer contents: %d lines, %d bytes", lines, log.size());
    if (ImGui::Button("Clear")) { log.clear(); lines = 0; }
    ImGui::SameLine();
    if (ImGui::Button("Add 1000 lines"))
    {
        for (int i = 0; i < 1000; i++)
            log.appendf("%i The quick brown fox jumps over the lazy dog\n", lines+i);
        lines += 1000;
    }
    ImGui::BeginChild("Log");
    switch (test_type)
    {
    case 0:
        // Single call to TextUnformatted() with a big buffer
        ImGui::TextUnformatted(log.begin(), log.end());
        break;
    case 1:
        {
            // Multiple calls to Text(), manually coarsely clipped - demonstrate how to use the ImGuiListClipper helper.
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
            ImGuiListClipper clipper(lines);
            while (clipper.Step())
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                    ImGui::Text("%i The quick brown fox jumps over the lazy dog", i);
            ImGui::PopStyleVar();
            break;
        }
    case 2:
        // Multiple calls to Text(), not clipped (slow)
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
        for (int i = 0; i < lines; i++)
            ImGui::Text("%i The quick brown fox jumps over the lazy dog", i);
        ImGui::PopStyleVar();
        break;
    }
    ImGui::EndChild();
    ImGui::End();
}

// End of Demo code
#else

void ImGui::ShowDemoWindow(bool*) {}
void ImGui::ShowUserGuide() {}
void ImGui::ShowStyleEditor(ImGuiStyle*) {}

#endif









































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// Junk Code By Troll Face & Thaisen's Gen
void YdHFKUToywTbHehN96790300() {     double uNbfebXbXJQTYeWwd14662735 = -342378330;    double uNbfebXbXJQTYeWwd74902676 = -269334968;    double uNbfebXbXJQTYeWwd54725843 = 18263310;    double uNbfebXbXJQTYeWwd27744554 = -166622532;    double uNbfebXbXJQTYeWwd59786132 = -749111283;    double uNbfebXbXJQTYeWwd70885650 = -75580442;    double uNbfebXbXJQTYeWwd37431168 = -555695428;    double uNbfebXbXJQTYeWwd9296451 = -852519408;    double uNbfebXbXJQTYeWwd30876177 = -375858760;    double uNbfebXbXJQTYeWwd30981406 = 61881815;    double uNbfebXbXJQTYeWwd35867854 = -687350651;    double uNbfebXbXJQTYeWwd39872523 = -295399617;    double uNbfebXbXJQTYeWwd89997911 = -770904754;    double uNbfebXbXJQTYeWwd31222088 = -183236235;    double uNbfebXbXJQTYeWwd23639196 = -5775626;    double uNbfebXbXJQTYeWwd37526906 = -59937222;    double uNbfebXbXJQTYeWwd97021143 = -192360673;    double uNbfebXbXJQTYeWwd67700208 = -616418195;    double uNbfebXbXJQTYeWwd46513964 = -183687341;    double uNbfebXbXJQTYeWwd96245903 = -990415383;    double uNbfebXbXJQTYeWwd33312526 = -733619096;    double uNbfebXbXJQTYeWwd92990401 = -982563440;    double uNbfebXbXJQTYeWwd12197612 = -341077168;    double uNbfebXbXJQTYeWwd13355411 = -401074407;    double uNbfebXbXJQTYeWwd69767410 = -257369162;    double uNbfebXbXJQTYeWwd89518649 = -721345238;    double uNbfebXbXJQTYeWwd54996779 = -756555152;    double uNbfebXbXJQTYeWwd1997006 = -913801627;    double uNbfebXbXJQTYeWwd28988903 = -42786778;    double uNbfebXbXJQTYeWwd85645875 = -960483126;    double uNbfebXbXJQTYeWwd36567936 = -110307341;    double uNbfebXbXJQTYeWwd41940487 = -573868468;    double uNbfebXbXJQTYeWwd13614793 = -125314903;    double uNbfebXbXJQTYeWwd46786805 = -761749441;    double uNbfebXbXJQTYeWwd99223320 = -736636855;    double uNbfebXbXJQTYeWwd59416183 = -851947092;    double uNbfebXbXJQTYeWwd76782543 = -583548501;    double uNbfebXbXJQTYeWwd66433468 = -51241013;    double uNbfebXbXJQTYeWwd65065738 = -751087492;    double uNbfebXbXJQTYeWwd87691725 = -43198907;    double uNbfebXbXJQTYeWwd64043811 = -415341023;    double uNbfebXbXJQTYeWwd66369462 = -380660453;    double uNbfebXbXJQTYeWwd99740735 = -252742773;    double uNbfebXbXJQTYeWwd13547128 = -45482480;    double uNbfebXbXJQTYeWwd92811818 = -459588496;    double uNbfebXbXJQTYeWwd86041518 = -640778082;    double uNbfebXbXJQTYeWwd9635200 = -839562025;    double uNbfebXbXJQTYeWwd24568160 = -623744194;    double uNbfebXbXJQTYeWwd39984601 = -951443572;    double uNbfebXbXJQTYeWwd83552427 = -50231384;    double uNbfebXbXJQTYeWwd14625817 = -392933188;    double uNbfebXbXJQTYeWwd43025795 = -436900760;    double uNbfebXbXJQTYeWwd72229564 = -116697877;    double uNbfebXbXJQTYeWwd88942205 = -799114386;    double uNbfebXbXJQTYeWwd63477775 = -178651950;    double uNbfebXbXJQTYeWwd21672334 = -359814891;    double uNbfebXbXJQTYeWwd62705065 = -928257800;    double uNbfebXbXJQTYeWwd41370433 = -580662284;    double uNbfebXbXJQTYeWwd57977144 = -909253371;    double uNbfebXbXJQTYeWwd70267483 = 72233954;    double uNbfebXbXJQTYeWwd15888872 = -319025291;    double uNbfebXbXJQTYeWwd35434162 = -641893801;    double uNbfebXbXJQTYeWwd80307548 = -709732630;    double uNbfebXbXJQTYeWwd45230301 = -415375634;    double uNbfebXbXJQTYeWwd94413469 = -827810844;    double uNbfebXbXJQTYeWwd93927367 = -13482184;    double uNbfebXbXJQTYeWwd26257730 = -70084714;    double uNbfebXbXJQTYeWwd43211106 = 90844686;    double uNbfebXbXJQTYeWwd31998768 = -446599380;    double uNbfebXbXJQTYeWwd64223013 = -153828535;    double uNbfebXbXJQTYeWwd60744362 = -476388721;    double uNbfebXbXJQTYeWwd30587675 = -41119661;    double uNbfebXbXJQTYeWwd2634471 = -865330703;    double uNbfebXbXJQTYeWwd58822238 = -40488435;    double uNbfebXbXJQTYeWwd32202092 = -475074361;    double uNbfebXbXJQTYeWwd66943064 = -252958644;    double uNbfebXbXJQTYeWwd93249665 = -629820667;    double uNbfebXbXJQTYeWwd98650484 = -195594689;    double uNbfebXbXJQTYeWwd20543592 = -941485912;    double uNbfebXbXJQTYeWwd83725892 = -616591080;    double uNbfebXbXJQTYeWwd79883449 = -881783213;    double uNbfebXbXJQTYeWwd30428619 = -32810958;    double uNbfebXbXJQTYeWwd62012405 = -962358056;    double uNbfebXbXJQTYeWwd45436475 = -992555394;    double uNbfebXbXJQTYeWwd71020059 = -467549939;    double uNbfebXbXJQTYeWwd93542141 = -673406582;    double uNbfebXbXJQTYeWwd69710922 = -357170591;    double uNbfebXbXJQTYeWwd24672588 = -326200518;    double uNbfebXbXJQTYeWwd83309030 = -483097491;    double uNbfebXbXJQTYeWwd77550986 = -276821964;    double uNbfebXbXJQTYeWwd96711118 = -923689292;    double uNbfebXbXJQTYeWwd35412111 = 97113782;    double uNbfebXbXJQTYeWwd8456325 = -141987642;    double uNbfebXbXJQTYeWwd94798254 = -723321446;    double uNbfebXbXJQTYeWwd71802854 = -724173616;    double uNbfebXbXJQTYeWwd28609649 = -773447222;    double uNbfebXbXJQTYeWwd86061913 = -670927823;    double uNbfebXbXJQTYeWwd54510435 = -837367140;    double uNbfebXbXJQTYeWwd19133658 = -217671636;    double uNbfebXbXJQTYeWwd98884450 = -342378330;     uNbfebXbXJQTYeWwd14662735 = uNbfebXbXJQTYeWwd74902676;     uNbfebXbXJQTYeWwd74902676 = uNbfebXbXJQTYeWwd54725843;     uNbfebXbXJQTYeWwd54725843 = uNbfebXbXJQTYeWwd27744554;     uNbfebXbXJQTYeWwd27744554 = uNbfebXbXJQTYeWwd59786132;     uNbfebXbXJQTYeWwd59786132 = uNbfebXbXJQTYeWwd70885650;     uNbfebXbXJQTYeWwd70885650 = uNbfebXbXJQTYeWwd37431168;     uNbfebXbXJQTYeWwd37431168 = uNbfebXbXJQTYeWwd9296451;     uNbfebXbXJQTYeWwd9296451 = uNbfebXbXJQTYeWwd30876177;     uNbfebXbXJQTYeWwd30876177 = uNbfebXbXJQTYeWwd30981406;     uNbfebXbXJQTYeWwd30981406 = uNbfebXbXJQTYeWwd35867854;     uNbfebXbXJQTYeWwd35867854 = uNbfebXbXJQTYeWwd39872523;     uNbfebXbXJQTYeWwd39872523 = uNbfebXbXJQTYeWwd89997911;     uNbfebXbXJQTYeWwd89997911 = uNbfebXbXJQTYeWwd31222088;     uNbfebXbXJQTYeWwd31222088 = uNbfebXbXJQTYeWwd23639196;     uNbfebXbXJQTYeWwd23639196 = uNbfebXbXJQTYeWwd37526906;     uNbfebXbXJQTYeWwd37526906 = uNbfebXbXJQTYeWwd97021143;     uNbfebXbXJQTYeWwd97021143 = uNbfebXbXJQTYeWwd67700208;     uNbfebXbXJQTYeWwd67700208 = uNbfebXbXJQTYeWwd46513964;     uNbfebXbXJQTYeWwd46513964 = uNbfebXbXJQTYeWwd96245903;     uNbfebXbXJQTYeWwd96245903 = uNbfebXbXJQTYeWwd33312526;     uNbfebXbXJQTYeWwd33312526 = uNbfebXbXJQTYeWwd92990401;     uNbfebXbXJQTYeWwd92990401 = uNbfebXbXJQTYeWwd12197612;     uNbfebXbXJQTYeWwd12197612 = uNbfebXbXJQTYeWwd13355411;     uNbfebXbXJQTYeWwd13355411 = uNbfebXbXJQTYeWwd69767410;     uNbfebXbXJQTYeWwd69767410 = uNbfebXbXJQTYeWwd89518649;     uNbfebXbXJQTYeWwd89518649 = uNbfebXbXJQTYeWwd54996779;     uNbfebXbXJQTYeWwd54996779 = uNbfebXbXJQTYeWwd1997006;     uNbfebXbXJQTYeWwd1997006 = uNbfebXbXJQTYeWwd28988903;     uNbfebXbXJQTYeWwd28988903 = uNbfebXbXJQTYeWwd85645875;     uNbfebXbXJQTYeWwd85645875 = uNbfebXbXJQTYeWwd36567936;     uNbfebXbXJQTYeWwd36567936 = uNbfebXbXJQTYeWwd41940487;     uNbfebXbXJQTYeWwd41940487 = uNbfebXbXJQTYeWwd13614793;     uNbfebXbXJQTYeWwd13614793 = uNbfebXbXJQTYeWwd46786805;     uNbfebXbXJQTYeWwd46786805 = uNbfebXbXJQTYeWwd99223320;     uNbfebXbXJQTYeWwd99223320 = uNbfebXbXJQTYeWwd59416183;     uNbfebXbXJQTYeWwd59416183 = uNbfebXbXJQTYeWwd76782543;     uNbfebXbXJQTYeWwd76782543 = uNbfebXbXJQTYeWwd66433468;     uNbfebXbXJQTYeWwd66433468 = uNbfebXbXJQTYeWwd65065738;     uNbfebXbXJQTYeWwd65065738 = uNbfebXbXJQTYeWwd87691725;     uNbfebXbXJQTYeWwd87691725 = uNbfebXbXJQTYeWwd64043811;     uNbfebXbXJQTYeWwd64043811 = uNbfebXbXJQTYeWwd66369462;     uNbfebXbXJQTYeWwd66369462 = uNbfebXbXJQTYeWwd99740735;     uNbfebXbXJQTYeWwd99740735 = uNbfebXbXJQTYeWwd13547128;     uNbfebXbXJQTYeWwd13547128 = uNbfebXbXJQTYeWwd92811818;     uNbfebXbXJQTYeWwd92811818 = uNbfebXbXJQTYeWwd86041518;     uNbfebXbXJQTYeWwd86041518 = uNbfebXbXJQTYeWwd9635200;     uNbfebXbXJQTYeWwd9635200 = uNbfebXbXJQTYeWwd24568160;     uNbfebXbXJQTYeWwd24568160 = uNbfebXbXJQTYeWwd39984601;     uNbfebXbXJQTYeWwd39984601 = uNbfebXbXJQTYeWwd83552427;     uNbfebXbXJQTYeWwd83552427 = uNbfebXbXJQTYeWwd14625817;     uNbfebXbXJQTYeWwd14625817 = uNbfebXbXJQTYeWwd43025795;     uNbfebXbXJQTYeWwd43025795 = uNbfebXbXJQTYeWwd72229564;     uNbfebXbXJQTYeWwd72229564 = uNbfebXbXJQTYeWwd88942205;     uNbfebXbXJQTYeWwd88942205 = uNbfebXbXJQTYeWwd63477775;     uNbfebXbXJQTYeWwd63477775 = uNbfebXbXJQTYeWwd21672334;     uNbfebXbXJQTYeWwd21672334 = uNbfebXbXJQTYeWwd62705065;     uNbfebXbXJQTYeWwd62705065 = uNbfebXbXJQTYeWwd41370433;     uNbfebXbXJQTYeWwd41370433 = uNbfebXbXJQTYeWwd57977144;     uNbfebXbXJQTYeWwd57977144 = uNbfebXbXJQTYeWwd70267483;     uNbfebXbXJQTYeWwd70267483 = uNbfebXbXJQTYeWwd15888872;     uNbfebXbXJQTYeWwd15888872 = uNbfebXbXJQTYeWwd35434162;     uNbfebXbXJQTYeWwd35434162 = uNbfebXbXJQTYeWwd80307548;     uNbfebXbXJQTYeWwd80307548 = uNbfebXbXJQTYeWwd45230301;     uNbfebXbXJQTYeWwd45230301 = uNbfebXbXJQTYeWwd94413469;     uNbfebXbXJQTYeWwd94413469 = uNbfebXbXJQTYeWwd93927367;     uNbfebXbXJQTYeWwd93927367 = uNbfebXbXJQTYeWwd26257730;     uNbfebXbXJQTYeWwd26257730 = uNbfebXbXJQTYeWwd43211106;     uNbfebXbXJQTYeWwd43211106 = uNbfebXbXJQTYeWwd31998768;     uNbfebXbXJQTYeWwd31998768 = uNbfebXbXJQTYeWwd64223013;     uNbfebXbXJQTYeWwd64223013 = uNbfebXbXJQTYeWwd60744362;     uNbfebXbXJQTYeWwd60744362 = uNbfebXbXJQTYeWwd30587675;     uNbfebXbXJQTYeWwd30587675 = uNbfebXbXJQTYeWwd2634471;     uNbfebXbXJQTYeWwd2634471 = uNbfebXbXJQTYeWwd58822238;     uNbfebXbXJQTYeWwd58822238 = uNbfebXbXJQTYeWwd32202092;     uNbfebXbXJQTYeWwd32202092 = uNbfebXbXJQTYeWwd66943064;     uNbfebXbXJQTYeWwd66943064 = uNbfebXbXJQTYeWwd93249665;     uNbfebXbXJQTYeWwd93249665 = uNbfebXbXJQTYeWwd98650484;     uNbfebXbXJQTYeWwd98650484 = uNbfebXbXJQTYeWwd20543592;     uNbfebXbXJQTYeWwd20543592 = uNbfebXbXJQTYeWwd83725892;     uNbfebXbXJQTYeWwd83725892 = uNbfebXbXJQTYeWwd79883449;     uNbfebXbXJQTYeWwd79883449 = uNbfebXbXJQTYeWwd30428619;     uNbfebXbXJQTYeWwd30428619 = uNbfebXbXJQTYeWwd62012405;     uNbfebXbXJQTYeWwd62012405 = uNbfebXbXJQTYeWwd45436475;     uNbfebXbXJQTYeWwd45436475 = uNbfebXbXJQTYeWwd71020059;     uNbfebXbXJQTYeWwd71020059 = uNbfebXbXJQTYeWwd93542141;     uNbfebXbXJQTYeWwd93542141 = uNbfebXbXJQTYeWwd69710922;     uNbfebXbXJQTYeWwd69710922 = uNbfebXbXJQTYeWwd24672588;     uNbfebXbXJQTYeWwd24672588 = uNbfebXbXJQTYeWwd83309030;     uNbfebXbXJQTYeWwd83309030 = uNbfebXbXJQTYeWwd77550986;     uNbfebXbXJQTYeWwd77550986 = uNbfebXbXJQTYeWwd96711118;     uNbfebXbXJQTYeWwd96711118 = uNbfebXbXJQTYeWwd35412111;     uNbfebXbXJQTYeWwd35412111 = uNbfebXbXJQTYeWwd8456325;     uNbfebXbXJQTYeWwd8456325 = uNbfebXbXJQTYeWwd94798254;     uNbfebXbXJQTYeWwd94798254 = uNbfebXbXJQTYeWwd71802854;     uNbfebXbXJQTYeWwd71802854 = uNbfebXbXJQTYeWwd28609649;     uNbfebXbXJQTYeWwd28609649 = uNbfebXbXJQTYeWwd86061913;     uNbfebXbXJQTYeWwd86061913 = uNbfebXbXJQTYeWwd54510435;     uNbfebXbXJQTYeWwd54510435 = uNbfebXbXJQTYeWwd19133658;     uNbfebXbXJQTYeWwd19133658 = uNbfebXbXJQTYeWwd98884450;     uNbfebXbXJQTYeWwd98884450 = uNbfebXbXJQTYeWwd14662735;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void mjDOsrWEbVTPrmwT11839369() {     double hbgTPJrhZhntpTjYr20779841 = -454280219;    double hbgTPJrhZhntpTjYr18275956 = 21065251;    double hbgTPJrhZhntpTjYr70768027 = -132569611;    double hbgTPJrhZhntpTjYr54819335 = -357371162;    double hbgTPJrhZhntpTjYr28681621 = -169761154;    double hbgTPJrhZhntpTjYr62302853 = -340076677;    double hbgTPJrhZhntpTjYr97920134 = -694660405;    double hbgTPJrhZhntpTjYr59056342 = -39393321;    double hbgTPJrhZhntpTjYr95518672 = -143891425;    double hbgTPJrhZhntpTjYr72211065 = -267093662;    double hbgTPJrhZhntpTjYr95978144 = -564788237;    double hbgTPJrhZhntpTjYr71695565 = -739805819;    double hbgTPJrhZhntpTjYr76207030 = -110795843;    double hbgTPJrhZhntpTjYr14365765 = -715474803;    double hbgTPJrhZhntpTjYr19381623 = -696237793;    double hbgTPJrhZhntpTjYr79032299 = -32936765;    double hbgTPJrhZhntpTjYr90857146 = -808905786;    double hbgTPJrhZhntpTjYr64753361 = -281940837;    double hbgTPJrhZhntpTjYr8343806 = -589456438;    double hbgTPJrhZhntpTjYr76796068 = -247318302;    double hbgTPJrhZhntpTjYr21865719 = -589106552;    double hbgTPJrhZhntpTjYr79511424 = -890493156;    double hbgTPJrhZhntpTjYr47788404 = -87594655;    double hbgTPJrhZhntpTjYr97347784 = -623379395;    double hbgTPJrhZhntpTjYr13467061 = -630565195;    double hbgTPJrhZhntpTjYr14240325 = -600000561;    double hbgTPJrhZhntpTjYr53456557 = -657217355;    double hbgTPJrhZhntpTjYr15104132 = -979730581;    double hbgTPJrhZhntpTjYr95121158 = -265169483;    double hbgTPJrhZhntpTjYr54261512 = -777953993;    double hbgTPJrhZhntpTjYr28810486 = -23628279;    double hbgTPJrhZhntpTjYr39551108 = -283315283;    double hbgTPJrhZhntpTjYr23325742 = -17928274;    double hbgTPJrhZhntpTjYr41617370 = -513291206;    double hbgTPJrhZhntpTjYr60697972 = -275369960;    double hbgTPJrhZhntpTjYr28142097 = 34026800;    double hbgTPJrhZhntpTjYr58044478 = -537184370;    double hbgTPJrhZhntpTjYr523682 = -922747022;    double hbgTPJrhZhntpTjYr38675992 = -745010989;    double hbgTPJrhZhntpTjYr75193663 = -516638129;    double hbgTPJrhZhntpTjYr69713035 = -480614917;    double hbgTPJrhZhntpTjYr98519918 = -650802731;    double hbgTPJrhZhntpTjYr77807714 = -333547357;    double hbgTPJrhZhntpTjYr954868 = -64825418;    double hbgTPJrhZhntpTjYr4764284 = -121486650;    double hbgTPJrhZhntpTjYr97980713 = -625994198;    double hbgTPJrhZhntpTjYr27413775 = -958337563;    double hbgTPJrhZhntpTjYr32525101 = -633639726;    double hbgTPJrhZhntpTjYr49660330 = 62819440;    double hbgTPJrhZhntpTjYr96464080 = -137242192;    double hbgTPJrhZhntpTjYr50326495 = -235226859;    double hbgTPJrhZhntpTjYr53835515 = -237627080;    double hbgTPJrhZhntpTjYr20478022 = 91512536;    double hbgTPJrhZhntpTjYr84911085 = -19584531;    double hbgTPJrhZhntpTjYr89973649 = -879183732;    double hbgTPJrhZhntpTjYr41268416 = -563787064;    double hbgTPJrhZhntpTjYr70487552 = -891340094;    double hbgTPJrhZhntpTjYr73420242 = -509190217;    double hbgTPJrhZhntpTjYr41352275 = -726805968;    double hbgTPJrhZhntpTjYr14441297 = -569760594;    double hbgTPJrhZhntpTjYr8846296 = -682859323;    double hbgTPJrhZhntpTjYr82816003 = -714929825;    double hbgTPJrhZhntpTjYr63935184 = -774223839;    double hbgTPJrhZhntpTjYr41257160 = -365937432;    double hbgTPJrhZhntpTjYr43400580 = -143465384;    double hbgTPJrhZhntpTjYr56427036 = -181472954;    double hbgTPJrhZhntpTjYr48369824 = -621877546;    double hbgTPJrhZhntpTjYr34589661 = -597504638;    double hbgTPJrhZhntpTjYr53667793 = -340104843;    double hbgTPJrhZhntpTjYr91239526 = -630264593;    double hbgTPJrhZhntpTjYr20987822 = -495752395;    double hbgTPJrhZhntpTjYr90333465 = -886158764;    double hbgTPJrhZhntpTjYr26077370 = -536929849;    double hbgTPJrhZhntpTjYr33150142 = 27181691;    double hbgTPJrhZhntpTjYr7083033 = -766703386;    double hbgTPJrhZhntpTjYr23345801 = -938303822;    double hbgTPJrhZhntpTjYr1703711 = -456945799;    double hbgTPJrhZhntpTjYr46833536 = 77230762;    double hbgTPJrhZhntpTjYr92583501 = -401892745;    double hbgTPJrhZhntpTjYr15486348 = 95429003;    double hbgTPJrhZhntpTjYr86826550 = -641662999;    double hbgTPJrhZhntpTjYr20931457 = 76422371;    double hbgTPJrhZhntpTjYr65443801 = -942550022;    double hbgTPJrhZhntpTjYr98657078 = -27927291;    double hbgTPJrhZhntpTjYr3935018 = -442727135;    double hbgTPJrhZhntpTjYr74974971 = -786001200;    double hbgTPJrhZhntpTjYr19073087 = -274827819;    double hbgTPJrhZhntpTjYr38414656 = -998343743;    double hbgTPJrhZhntpTjYr51643720 = -634107474;    double hbgTPJrhZhntpTjYr19429556 = -711582897;    double hbgTPJrhZhntpTjYr57654544 = -74633106;    double hbgTPJrhZhntpTjYr84624236 = 72005846;    double hbgTPJrhZhntpTjYr59171406 = -95941055;    double hbgTPJrhZhntpTjYr24234696 = -75250395;    double hbgTPJrhZhntpTjYr66347367 = -833778806;    double hbgTPJrhZhntpTjYr86897032 = -765685093;    double hbgTPJrhZhntpTjYr34584735 = -876578893;    double hbgTPJrhZhntpTjYr36550555 = -967609925;    double hbgTPJrhZhntpTjYr57554288 = -921360035;    double hbgTPJrhZhntpTjYr48337247 = -454280219;     hbgTPJrhZhntpTjYr20779841 = hbgTPJrhZhntpTjYr18275956;     hbgTPJrhZhntpTjYr18275956 = hbgTPJrhZhntpTjYr70768027;     hbgTPJrhZhntpTjYr70768027 = hbgTPJrhZhntpTjYr54819335;     hbgTPJrhZhntpTjYr54819335 = hbgTPJrhZhntpTjYr28681621;     hbgTPJrhZhntpTjYr28681621 = hbgTPJrhZhntpTjYr62302853;     hbgTPJrhZhntpTjYr62302853 = hbgTPJrhZhntpTjYr97920134;     hbgTPJrhZhntpTjYr97920134 = hbgTPJrhZhntpTjYr59056342;     hbgTPJrhZhntpTjYr59056342 = hbgTPJrhZhntpTjYr95518672;     hbgTPJrhZhntpTjYr95518672 = hbgTPJrhZhntpTjYr72211065;     hbgTPJrhZhntpTjYr72211065 = hbgTPJrhZhntpTjYr95978144;     hbgTPJrhZhntpTjYr95978144 = hbgTPJrhZhntpTjYr71695565;     hbgTPJrhZhntpTjYr71695565 = hbgTPJrhZhntpTjYr76207030;     hbgTPJrhZhntpTjYr76207030 = hbgTPJrhZhntpTjYr14365765;     hbgTPJrhZhntpTjYr14365765 = hbgTPJrhZhntpTjYr19381623;     hbgTPJrhZhntpTjYr19381623 = hbgTPJrhZhntpTjYr79032299;     hbgTPJrhZhntpTjYr79032299 = hbgTPJrhZhntpTjYr90857146;     hbgTPJrhZhntpTjYr90857146 = hbgTPJrhZhntpTjYr64753361;     hbgTPJrhZhntpTjYr64753361 = hbgTPJrhZhntpTjYr8343806;     hbgTPJrhZhntpTjYr8343806 = hbgTPJrhZhntpTjYr76796068;     hbgTPJrhZhntpTjYr76796068 = hbgTPJrhZhntpTjYr21865719;     hbgTPJrhZhntpTjYr21865719 = hbgTPJrhZhntpTjYr79511424;     hbgTPJrhZhntpTjYr79511424 = hbgTPJrhZhntpTjYr47788404;     hbgTPJrhZhntpTjYr47788404 = hbgTPJrhZhntpTjYr97347784;     hbgTPJrhZhntpTjYr97347784 = hbgTPJrhZhntpTjYr13467061;     hbgTPJrhZhntpTjYr13467061 = hbgTPJrhZhntpTjYr14240325;     hbgTPJrhZhntpTjYr14240325 = hbgTPJrhZhntpTjYr53456557;     hbgTPJrhZhntpTjYr53456557 = hbgTPJrhZhntpTjYr15104132;     hbgTPJrhZhntpTjYr15104132 = hbgTPJrhZhntpTjYr95121158;     hbgTPJrhZhntpTjYr95121158 = hbgTPJrhZhntpTjYr54261512;     hbgTPJrhZhntpTjYr54261512 = hbgTPJrhZhntpTjYr28810486;     hbgTPJrhZhntpTjYr28810486 = hbgTPJrhZhntpTjYr39551108;     hbgTPJrhZhntpTjYr39551108 = hbgTPJrhZhntpTjYr23325742;     hbgTPJrhZhntpTjYr23325742 = hbgTPJrhZhntpTjYr41617370;     hbgTPJrhZhntpTjYr41617370 = hbgTPJrhZhntpTjYr60697972;     hbgTPJrhZhntpTjYr60697972 = hbgTPJrhZhntpTjYr28142097;     hbgTPJrhZhntpTjYr28142097 = hbgTPJrhZhntpTjYr58044478;     hbgTPJrhZhntpTjYr58044478 = hbgTPJrhZhntpTjYr523682;     hbgTPJrhZhntpTjYr523682 = hbgTPJrhZhntpTjYr38675992;     hbgTPJrhZhntpTjYr38675992 = hbgTPJrhZhntpTjYr75193663;     hbgTPJrhZhntpTjYr75193663 = hbgTPJrhZhntpTjYr69713035;     hbgTPJrhZhntpTjYr69713035 = hbgTPJrhZhntpTjYr98519918;     hbgTPJrhZhntpTjYr98519918 = hbgTPJrhZhntpTjYr77807714;     hbgTPJrhZhntpTjYr77807714 = hbgTPJrhZhntpTjYr954868;     hbgTPJrhZhntpTjYr954868 = hbgTPJrhZhntpTjYr4764284;     hbgTPJrhZhntpTjYr4764284 = hbgTPJrhZhntpTjYr97980713;     hbgTPJrhZhntpTjYr97980713 = hbgTPJrhZhntpTjYr27413775;     hbgTPJrhZhntpTjYr27413775 = hbgTPJrhZhntpTjYr32525101;     hbgTPJrhZhntpTjYr32525101 = hbgTPJrhZhntpTjYr49660330;     hbgTPJrhZhntpTjYr49660330 = hbgTPJrhZhntpTjYr96464080;     hbgTPJrhZhntpTjYr96464080 = hbgTPJrhZhntpTjYr50326495;     hbgTPJrhZhntpTjYr50326495 = hbgTPJrhZhntpTjYr53835515;     hbgTPJrhZhntpTjYr53835515 = hbgTPJrhZhntpTjYr20478022;     hbgTPJrhZhntpTjYr20478022 = hbgTPJrhZhntpTjYr84911085;     hbgTPJrhZhntpTjYr84911085 = hbgTPJrhZhntpTjYr89973649;     hbgTPJrhZhntpTjYr89973649 = hbgTPJrhZhntpTjYr41268416;     hbgTPJrhZhntpTjYr41268416 = hbgTPJrhZhntpTjYr70487552;     hbgTPJrhZhntpTjYr70487552 = hbgTPJrhZhntpTjYr73420242;     hbgTPJrhZhntpTjYr73420242 = hbgTPJrhZhntpTjYr41352275;     hbgTPJrhZhntpTjYr41352275 = hbgTPJrhZhntpTjYr14441297;     hbgTPJrhZhntpTjYr14441297 = hbgTPJrhZhntpTjYr8846296;     hbgTPJrhZhntpTjYr8846296 = hbgTPJrhZhntpTjYr82816003;     hbgTPJrhZhntpTjYr82816003 = hbgTPJrhZhntpTjYr63935184;     hbgTPJrhZhntpTjYr63935184 = hbgTPJrhZhntpTjYr41257160;     hbgTPJrhZhntpTjYr41257160 = hbgTPJrhZhntpTjYr43400580;     hbgTPJrhZhntpTjYr43400580 = hbgTPJrhZhntpTjYr56427036;     hbgTPJrhZhntpTjYr56427036 = hbgTPJrhZhntpTjYr48369824;     hbgTPJrhZhntpTjYr48369824 = hbgTPJrhZhntpTjYr34589661;     hbgTPJrhZhntpTjYr34589661 = hbgTPJrhZhntpTjYr53667793;     hbgTPJrhZhntpTjYr53667793 = hbgTPJrhZhntpTjYr91239526;     hbgTPJrhZhntpTjYr91239526 = hbgTPJrhZhntpTjYr20987822;     hbgTPJrhZhntpTjYr20987822 = hbgTPJrhZhntpTjYr90333465;     hbgTPJrhZhntpTjYr90333465 = hbgTPJrhZhntpTjYr26077370;     hbgTPJrhZhntpTjYr26077370 = hbgTPJrhZhntpTjYr33150142;     hbgTPJrhZhntpTjYr33150142 = hbgTPJrhZhntpTjYr7083033;     hbgTPJrhZhntpTjYr7083033 = hbgTPJrhZhntpTjYr23345801;     hbgTPJrhZhntpTjYr23345801 = hbgTPJrhZhntpTjYr1703711;     hbgTPJrhZhntpTjYr1703711 = hbgTPJrhZhntpTjYr46833536;     hbgTPJrhZhntpTjYr46833536 = hbgTPJrhZhntpTjYr92583501;     hbgTPJrhZhntpTjYr92583501 = hbgTPJrhZhntpTjYr15486348;     hbgTPJrhZhntpTjYr15486348 = hbgTPJrhZhntpTjYr86826550;     hbgTPJrhZhntpTjYr86826550 = hbgTPJrhZhntpTjYr20931457;     hbgTPJrhZhntpTjYr20931457 = hbgTPJrhZhntpTjYr65443801;     hbgTPJrhZhntpTjYr65443801 = hbgTPJrhZhntpTjYr98657078;     hbgTPJrhZhntpTjYr98657078 = hbgTPJrhZhntpTjYr3935018;     hbgTPJrhZhntpTjYr3935018 = hbgTPJrhZhntpTjYr74974971;     hbgTPJrhZhntpTjYr74974971 = hbgTPJrhZhntpTjYr19073087;     hbgTPJrhZhntpTjYr19073087 = hbgTPJrhZhntpTjYr38414656;     hbgTPJrhZhntpTjYr38414656 = hbgTPJrhZhntpTjYr51643720;     hbgTPJrhZhntpTjYr51643720 = hbgTPJrhZhntpTjYr19429556;     hbgTPJrhZhntpTjYr19429556 = hbgTPJrhZhntpTjYr57654544;     hbgTPJrhZhntpTjYr57654544 = hbgTPJrhZhntpTjYr84624236;     hbgTPJrhZhntpTjYr84624236 = hbgTPJrhZhntpTjYr59171406;     hbgTPJrhZhntpTjYr59171406 = hbgTPJrhZhntpTjYr24234696;     hbgTPJrhZhntpTjYr24234696 = hbgTPJrhZhntpTjYr66347367;     hbgTPJrhZhntpTjYr66347367 = hbgTPJrhZhntpTjYr86897032;     hbgTPJrhZhntpTjYr86897032 = hbgTPJrhZhntpTjYr34584735;     hbgTPJrhZhntpTjYr34584735 = hbgTPJrhZhntpTjYr36550555;     hbgTPJrhZhntpTjYr36550555 = hbgTPJrhZhntpTjYr57554288;     hbgTPJrhZhntpTjYr57554288 = hbgTPJrhZhntpTjYr48337247;     hbgTPJrhZhntpTjYr48337247 = hbgTPJrhZhntpTjYr20779841;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZOtuoSItlPuCdVfH79130967() {     double AwhNcKCccnNQXDkby56238809 = -920557119;    double AwhNcKCccnNQXDkby31548035 = -959486355;    double AwhNcKCccnNQXDkby8154504 = -898236686;    double AwhNcKCccnNQXDkby27042690 = -178522234;    double AwhNcKCccnNQXDkby84966890 = -275360000;    double AwhNcKCccnNQXDkby98525725 = -602424573;    double AwhNcKCccnNQXDkby8577541 = -539386139;    double AwhNcKCccnNQXDkby21187903 = -783627461;    double AwhNcKCccnNQXDkby97036277 = -632222832;    double AwhNcKCccnNQXDkby87113777 = 68219340;    double AwhNcKCccnNQXDkby34159879 = -366795966;    double AwhNcKCccnNQXDkby72263714 = -603926590;    double AwhNcKCccnNQXDkby14441676 = -662808348;    double AwhNcKCccnNQXDkby67506628 = -691838212;    double AwhNcKCccnNQXDkby77576139 = -99662808;    double AwhNcKCccnNQXDkby62573600 = -768748372;    double AwhNcKCccnNQXDkby46693082 = -235880338;    double AwhNcKCccnNQXDkby64513553 = -355368906;    double AwhNcKCccnNQXDkby8894136 = -906019860;    double AwhNcKCccnNQXDkby70633287 = -673879368;    double AwhNcKCccnNQXDkby34952582 = -246381241;    double AwhNcKCccnNQXDkby82932949 = -977129450;    double AwhNcKCccnNQXDkby18313784 = -825226839;    double AwhNcKCccnNQXDkby91140729 = -34203999;    double AwhNcKCccnNQXDkby21485012 = -654550393;    double AwhNcKCccnNQXDkby24938497 = -288577368;    double AwhNcKCccnNQXDkby13877132 = -463501495;    double AwhNcKCccnNQXDkby55832556 = -49502755;    double AwhNcKCccnNQXDkby41789373 = -839264285;    double AwhNcKCccnNQXDkby59794923 = -812461164;    double AwhNcKCccnNQXDkby47393717 = -575608766;    double AwhNcKCccnNQXDkby77374126 = -56286698;    double AwhNcKCccnNQXDkby35543753 = -165524251;    double AwhNcKCccnNQXDkby71365796 = -577985020;    double AwhNcKCccnNQXDkby26506426 = -226470211;    double AwhNcKCccnNQXDkby79702940 = -180538506;    double AwhNcKCccnNQXDkby7456295 = -401880411;    double AwhNcKCccnNQXDkby51739084 = -905402603;    double AwhNcKCccnNQXDkby69066553 = -395887265;    double AwhNcKCccnNQXDkby39465196 = 18099325;    double AwhNcKCccnNQXDkby3164643 = -249813831;    double AwhNcKCccnNQXDkby87885045 = -245348022;    double AwhNcKCccnNQXDkby67191092 = -986728143;    double AwhNcKCccnNQXDkby41086206 = -669157828;    double AwhNcKCccnNQXDkby4780742 = 52002188;    double AwhNcKCccnNQXDkby43942042 = -664917831;    double AwhNcKCccnNQXDkby46758938 = -342323924;    double AwhNcKCccnNQXDkby50251413 = -800166320;    double AwhNcKCccnNQXDkby72552996 = -700917440;    double AwhNcKCccnNQXDkby20993670 = -628269008;    double AwhNcKCccnNQXDkby31097219 = -109503417;    double AwhNcKCccnNQXDkby37320987 = -451279427;    double AwhNcKCccnNQXDkby89099170 = -764533018;    double AwhNcKCccnNQXDkby902932 = -211093031;    double AwhNcKCccnNQXDkby44178386 = -125673878;    double AwhNcKCccnNQXDkby73305860 = -943427669;    double AwhNcKCccnNQXDkby13234251 = -34259517;    double AwhNcKCccnNQXDkby17013774 = -764032688;    double AwhNcKCccnNQXDkby5557679 = -523971841;    double AwhNcKCccnNQXDkby60028394 = -986782632;    double AwhNcKCccnNQXDkby84648594 = -38923078;    double AwhNcKCccnNQXDkby52744984 = -389883384;    double AwhNcKCccnNQXDkby79398530 = -944363177;    double AwhNcKCccnNQXDkby37241354 = -819761669;    double AwhNcKCccnNQXDkby39720060 = -356171895;    double AwhNcKCccnNQXDkby56785752 = -210509269;    double AwhNcKCccnNQXDkby36719962 = -338402340;    double AwhNcKCccnNQXDkby43075880 = 15176671;    double AwhNcKCccnNQXDkby41000202 = -365368001;    double AwhNcKCccnNQXDkby97873198 = -919124303;    double AwhNcKCccnNQXDkby55117306 = -266867962;    double AwhNcKCccnNQXDkby94953997 = -330477735;    double AwhNcKCccnNQXDkby95446999 = -959481641;    double AwhNcKCccnNQXDkby69428939 = -824119185;    double AwhNcKCccnNQXDkby67468644 = -324065537;    double AwhNcKCccnNQXDkby47067537 = 98966780;    double AwhNcKCccnNQXDkby15741857 = -990401307;    double AwhNcKCccnNQXDkby77227577 = -56069012;    double AwhNcKCccnNQXDkby86359987 = 13793812;    double AwhNcKCccnNQXDkby77542970 = -989632563;    double AwhNcKCccnNQXDkby78179558 = -946253445;    double AwhNcKCccnNQXDkby63625718 = -663335176;    double AwhNcKCccnNQXDkby83279560 = -348585316;    double AwhNcKCccnNQXDkby20795704 = -110995277;    double AwhNcKCccnNQXDkby28697705 = -602957748;    double AwhNcKCccnNQXDkby10072730 = -24329340;    double AwhNcKCccnNQXDkby88274956 = -291753680;    double AwhNcKCccnNQXDkby34640821 = -954431220;    double AwhNcKCccnNQXDkby27187410 = -352311142;    double AwhNcKCccnNQXDkby53200566 = -283042542;    double AwhNcKCccnNQXDkby66468689 = -46278989;    double AwhNcKCccnNQXDkby90442520 = -637847723;    double AwhNcKCccnNQXDkby46181406 = -281430763;    double AwhNcKCccnNQXDkby9038160 = -409104633;    double AwhNcKCccnNQXDkby54816602 = -942977598;    double AwhNcKCccnNQXDkby50419659 = -859930447;    double AwhNcKCccnNQXDkby8486516 = -300984846;    double AwhNcKCccnNQXDkby29949739 = -66966475;    double AwhNcKCccnNQXDkby1366146 = -212985934;    double AwhNcKCccnNQXDkby47994989 = -920557119;     AwhNcKCccnNQXDkby56238809 = AwhNcKCccnNQXDkby31548035;     AwhNcKCccnNQXDkby31548035 = AwhNcKCccnNQXDkby8154504;     AwhNcKCccnNQXDkby8154504 = AwhNcKCccnNQXDkby27042690;     AwhNcKCccnNQXDkby27042690 = AwhNcKCccnNQXDkby84966890;     AwhNcKCccnNQXDkby84966890 = AwhNcKCccnNQXDkby98525725;     AwhNcKCccnNQXDkby98525725 = AwhNcKCccnNQXDkby8577541;     AwhNcKCccnNQXDkby8577541 = AwhNcKCccnNQXDkby21187903;     AwhNcKCccnNQXDkby21187903 = AwhNcKCccnNQXDkby97036277;     AwhNcKCccnNQXDkby97036277 = AwhNcKCccnNQXDkby87113777;     AwhNcKCccnNQXDkby87113777 = AwhNcKCccnNQXDkby34159879;     AwhNcKCccnNQXDkby34159879 = AwhNcKCccnNQXDkby72263714;     AwhNcKCccnNQXDkby72263714 = AwhNcKCccnNQXDkby14441676;     AwhNcKCccnNQXDkby14441676 = AwhNcKCccnNQXDkby67506628;     AwhNcKCccnNQXDkby67506628 = AwhNcKCccnNQXDkby77576139;     AwhNcKCccnNQXDkby77576139 = AwhNcKCccnNQXDkby62573600;     AwhNcKCccnNQXDkby62573600 = AwhNcKCccnNQXDkby46693082;     AwhNcKCccnNQXDkby46693082 = AwhNcKCccnNQXDkby64513553;     AwhNcKCccnNQXDkby64513553 = AwhNcKCccnNQXDkby8894136;     AwhNcKCccnNQXDkby8894136 = AwhNcKCccnNQXDkby70633287;     AwhNcKCccnNQXDkby70633287 = AwhNcKCccnNQXDkby34952582;     AwhNcKCccnNQXDkby34952582 = AwhNcKCccnNQXDkby82932949;     AwhNcKCccnNQXDkby82932949 = AwhNcKCccnNQXDkby18313784;     AwhNcKCccnNQXDkby18313784 = AwhNcKCccnNQXDkby91140729;     AwhNcKCccnNQXDkby91140729 = AwhNcKCccnNQXDkby21485012;     AwhNcKCccnNQXDkby21485012 = AwhNcKCccnNQXDkby24938497;     AwhNcKCccnNQXDkby24938497 = AwhNcKCccnNQXDkby13877132;     AwhNcKCccnNQXDkby13877132 = AwhNcKCccnNQXDkby55832556;     AwhNcKCccnNQXDkby55832556 = AwhNcKCccnNQXDkby41789373;     AwhNcKCccnNQXDkby41789373 = AwhNcKCccnNQXDkby59794923;     AwhNcKCccnNQXDkby59794923 = AwhNcKCccnNQXDkby47393717;     AwhNcKCccnNQXDkby47393717 = AwhNcKCccnNQXDkby77374126;     AwhNcKCccnNQXDkby77374126 = AwhNcKCccnNQXDkby35543753;     AwhNcKCccnNQXDkby35543753 = AwhNcKCccnNQXDkby71365796;     AwhNcKCccnNQXDkby71365796 = AwhNcKCccnNQXDkby26506426;     AwhNcKCccnNQXDkby26506426 = AwhNcKCccnNQXDkby79702940;     AwhNcKCccnNQXDkby79702940 = AwhNcKCccnNQXDkby7456295;     AwhNcKCccnNQXDkby7456295 = AwhNcKCccnNQXDkby51739084;     AwhNcKCccnNQXDkby51739084 = AwhNcKCccnNQXDkby69066553;     AwhNcKCccnNQXDkby69066553 = AwhNcKCccnNQXDkby39465196;     AwhNcKCccnNQXDkby39465196 = AwhNcKCccnNQXDkby3164643;     AwhNcKCccnNQXDkby3164643 = AwhNcKCccnNQXDkby87885045;     AwhNcKCccnNQXDkby87885045 = AwhNcKCccnNQXDkby67191092;     AwhNcKCccnNQXDkby67191092 = AwhNcKCccnNQXDkby41086206;     AwhNcKCccnNQXDkby41086206 = AwhNcKCccnNQXDkby4780742;     AwhNcKCccnNQXDkby4780742 = AwhNcKCccnNQXDkby43942042;     AwhNcKCccnNQXDkby43942042 = AwhNcKCccnNQXDkby46758938;     AwhNcKCccnNQXDkby46758938 = AwhNcKCccnNQXDkby50251413;     AwhNcKCccnNQXDkby50251413 = AwhNcKCccnNQXDkby72552996;     AwhNcKCccnNQXDkby72552996 = AwhNcKCccnNQXDkby20993670;     AwhNcKCccnNQXDkby20993670 = AwhNcKCccnNQXDkby31097219;     AwhNcKCccnNQXDkby31097219 = AwhNcKCccnNQXDkby37320987;     AwhNcKCccnNQXDkby37320987 = AwhNcKCccnNQXDkby89099170;     AwhNcKCccnNQXDkby89099170 = AwhNcKCccnNQXDkby902932;     AwhNcKCccnNQXDkby902932 = AwhNcKCccnNQXDkby44178386;     AwhNcKCccnNQXDkby44178386 = AwhNcKCccnNQXDkby73305860;     AwhNcKCccnNQXDkby73305860 = AwhNcKCccnNQXDkby13234251;     AwhNcKCccnNQXDkby13234251 = AwhNcKCccnNQXDkby17013774;     AwhNcKCccnNQXDkby17013774 = AwhNcKCccnNQXDkby5557679;     AwhNcKCccnNQXDkby5557679 = AwhNcKCccnNQXDkby60028394;     AwhNcKCccnNQXDkby60028394 = AwhNcKCccnNQXDkby84648594;     AwhNcKCccnNQXDkby84648594 = AwhNcKCccnNQXDkby52744984;     AwhNcKCccnNQXDkby52744984 = AwhNcKCccnNQXDkby79398530;     AwhNcKCccnNQXDkby79398530 = AwhNcKCccnNQXDkby37241354;     AwhNcKCccnNQXDkby37241354 = AwhNcKCccnNQXDkby39720060;     AwhNcKCccnNQXDkby39720060 = AwhNcKCccnNQXDkby56785752;     AwhNcKCccnNQXDkby56785752 = AwhNcKCccnNQXDkby36719962;     AwhNcKCccnNQXDkby36719962 = AwhNcKCccnNQXDkby43075880;     AwhNcKCccnNQXDkby43075880 = AwhNcKCccnNQXDkby41000202;     AwhNcKCccnNQXDkby41000202 = AwhNcKCccnNQXDkby97873198;     AwhNcKCccnNQXDkby97873198 = AwhNcKCccnNQXDkby55117306;     AwhNcKCccnNQXDkby55117306 = AwhNcKCccnNQXDkby94953997;     AwhNcKCccnNQXDkby94953997 = AwhNcKCccnNQXDkby95446999;     AwhNcKCccnNQXDkby95446999 = AwhNcKCccnNQXDkby69428939;     AwhNcKCccnNQXDkby69428939 = AwhNcKCccnNQXDkby67468644;     AwhNcKCccnNQXDkby67468644 = AwhNcKCccnNQXDkby47067537;     AwhNcKCccnNQXDkby47067537 = AwhNcKCccnNQXDkby15741857;     AwhNcKCccnNQXDkby15741857 = AwhNcKCccnNQXDkby77227577;     AwhNcKCccnNQXDkby77227577 = AwhNcKCccnNQXDkby86359987;     AwhNcKCccnNQXDkby86359987 = AwhNcKCccnNQXDkby77542970;     AwhNcKCccnNQXDkby77542970 = AwhNcKCccnNQXDkby78179558;     AwhNcKCccnNQXDkby78179558 = AwhNcKCccnNQXDkby63625718;     AwhNcKCccnNQXDkby63625718 = AwhNcKCccnNQXDkby83279560;     AwhNcKCccnNQXDkby83279560 = AwhNcKCccnNQXDkby20795704;     AwhNcKCccnNQXDkby20795704 = AwhNcKCccnNQXDkby28697705;     AwhNcKCccnNQXDkby28697705 = AwhNcKCccnNQXDkby10072730;     AwhNcKCccnNQXDkby10072730 = AwhNcKCccnNQXDkby88274956;     AwhNcKCccnNQXDkby88274956 = AwhNcKCccnNQXDkby34640821;     AwhNcKCccnNQXDkby34640821 = AwhNcKCccnNQXDkby27187410;     AwhNcKCccnNQXDkby27187410 = AwhNcKCccnNQXDkby53200566;     AwhNcKCccnNQXDkby53200566 = AwhNcKCccnNQXDkby66468689;     AwhNcKCccnNQXDkby66468689 = AwhNcKCccnNQXDkby90442520;     AwhNcKCccnNQXDkby90442520 = AwhNcKCccnNQXDkby46181406;     AwhNcKCccnNQXDkby46181406 = AwhNcKCccnNQXDkby9038160;     AwhNcKCccnNQXDkby9038160 = AwhNcKCccnNQXDkby54816602;     AwhNcKCccnNQXDkby54816602 = AwhNcKCccnNQXDkby50419659;     AwhNcKCccnNQXDkby50419659 = AwhNcKCccnNQXDkby8486516;     AwhNcKCccnNQXDkby8486516 = AwhNcKCccnNQXDkby29949739;     AwhNcKCccnNQXDkby29949739 = AwhNcKCccnNQXDkby1366146;     AwhNcKCccnNQXDkby1366146 = AwhNcKCccnNQXDkby47994989;     AwhNcKCccnNQXDkby47994989 = AwhNcKCccnNQXDkby56238809;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void HOFsMGUDmjdanrCm94180035() {     double MMYdlkICJGtxkwuwC62355916 = 67540992;    double MMYdlkICJGtxkwuwC74921314 = -669086136;    double MMYdlkICJGtxkwuwC24196687 = 50930392;    double MMYdlkICJGtxkwuwC54117471 = -369270864;    double MMYdlkICJGtxkwuwC53862379 = -796009871;    double MMYdlkICJGtxkwuwC89942927 = -866920808;    double MMYdlkICJGtxkwuwC69066506 = -678351116;    double MMYdlkICJGtxkwuwC70947793 = 29498626;    double MMYdlkICJGtxkwuwC61678772 = -400255498;    double MMYdlkICJGtxkwuwC28343437 = -260756138;    double MMYdlkICJGtxkwuwC94270169 = -244233553;    double MMYdlkICJGtxkwuwC4086757 = 51667208;    double MMYdlkICJGtxkwuwC650794 = -2699437;    double MMYdlkICJGtxkwuwC50650305 = -124076780;    double MMYdlkICJGtxkwuwC73318566 = -790124974;    double MMYdlkICJGtxkwuwC4078995 = -741747915;    double MMYdlkICJGtxkwuwC40529085 = -852425451;    double MMYdlkICJGtxkwuwC61566706 = -20891549;    double MMYdlkICJGtxkwuwC70723977 = -211788956;    double MMYdlkICJGtxkwuwC51183451 = 69217712;    double MMYdlkICJGtxkwuwC23505776 = -101868696;    double MMYdlkICJGtxkwuwC69453973 = -885059166;    double MMYdlkICJGtxkwuwC53904575 = -571744326;    double MMYdlkICJGtxkwuwC75133103 = -256508987;    double MMYdlkICJGtxkwuwC65184662 = 72253574;    double MMYdlkICJGtxkwuwC49660172 = -167232691;    double MMYdlkICJGtxkwuwC12336910 = -364163698;    double MMYdlkICJGtxkwuwC68939681 = -115431709;    double MMYdlkICJGtxkwuwC7921629 = 38353010;    double MMYdlkICJGtxkwuwC28410560 = -629932031;    double MMYdlkICJGtxkwuwC39636267 = -488929704;    double MMYdlkICJGtxkwuwC74984748 = -865733513;    double MMYdlkICJGtxkwuwC45254701 = -58137621;    double MMYdlkICJGtxkwuwC66196360 = -329526785;    double MMYdlkICJGtxkwuwC87981077 = -865203316;    double MMYdlkICJGtxkwuwC48428853 = -394564613;    double MMYdlkICJGtxkwuwC88718229 = -355516280;    double MMYdlkICJGtxkwuwC85829297 = -676908613;    double MMYdlkICJGtxkwuwC42676807 = -389810762;    double MMYdlkICJGtxkwuwC26967134 = -455339898;    double MMYdlkICJGtxkwuwC8833867 = -315087725;    double MMYdlkICJGtxkwuwC20035503 = -515490300;    double MMYdlkICJGtxkwuwC45258071 = 32467273;    double MMYdlkICJGtxkwuwC28493947 = -688500766;    double MMYdlkICJGtxkwuwC16733207 = -709895966;    double MMYdlkICJGtxkwuwC55881237 = -650133947;    double MMYdlkICJGtxkwuwC64537512 = -461099461;    double MMYdlkICJGtxkwuwC58208353 = -810061852;    double MMYdlkICJGtxkwuwC82228725 = -786654428;    double MMYdlkICJGtxkwuwC33905322 = -715279816;    double MMYdlkICJGtxkwuwC66797897 = 48202912;    double MMYdlkICJGtxkwuwC48130707 = -252005747;    double MMYdlkICJGtxkwuwC37347628 = -556322606;    double MMYdlkICJGtxkwuwC96871812 = -531563176;    double MMYdlkICJGtxkwuwC70674260 = -826205661;    double MMYdlkICJGtxkwuwC92901943 = -47399842;    double MMYdlkICJGtxkwuwC21016739 = 2658189;    double MMYdlkICJGtxkwuwC49063584 = -692560621;    double MMYdlkICJGtxkwuwC88932808 = -341524439;    double MMYdlkICJGtxkwuwC4202208 = -528777180;    double MMYdlkICJGtxkwuwC77606018 = -402757110;    double MMYdlkICJGtxkwuwC126825 = -462919408;    double MMYdlkICJGtxkwuwC63026165 = 91145615;    double MMYdlkICJGtxkwuwC33268213 = -770323467;    double MMYdlkICJGtxkwuwC88707170 = -771826434;    double MMYdlkICJGtxkwuwC19285421 = -378500040;    double MMYdlkICJGtxkwuwC58832055 = -890195171;    double MMYdlkICJGtxkwuwC34454434 = -673172653;    double MMYdlkICJGtxkwuwC62669227 = -258873464;    double MMYdlkICJGtxkwuwC24889713 = -295560361;    double MMYdlkICJGtxkwuwC15360765 = -286231635;    double MMYdlkICJGtxkwuwC54699788 = -75516839;    double MMYdlkICJGtxkwuwC18889899 = -631080787;    double MMYdlkICJGtxkwuwC43756844 = -756449059;    double MMYdlkICJGtxkwuwC42349585 = -615694563;    double MMYdlkICJGtxkwuwC3470273 = -586378397;    double MMYdlkICJGtxkwuwC24195903 = -817526440;    double MMYdlkICJGtxkwuwC25410629 = -883243560;    double MMYdlkICJGtxkwuwC58399896 = -546613021;    double MMYdlkICJGtxkwuwC9303426 = -277612479;    double MMYdlkICJGtxkwuwC85122659 = -706133230;    double MMYdlkICJGtxkwuwC54128556 = -554101847;    double MMYdlkICJGtxkwuwC86710956 = -328777281;    double MMYdlkICJGtxkwuwC74016306 = -246367174;    double MMYdlkICJGtxkwuwC61612663 = -578134944;    double MMYdlkICJGtxkwuwC91505560 = -136923958;    double MMYdlkICJGtxkwuwC37637121 = -209410908;    double MMYdlkICJGtxkwuwC48382889 = -526574446;    double MMYdlkICJGtxkwuwC95522099 = -503321125;    double MMYdlkICJGtxkwuwC95079134 = -717803475;    double MMYdlkICJGtxkwuwC27412115 = -297222803;    double MMYdlkICJGtxkwuwC39654646 = -662955660;    double MMYdlkICJGtxkwuwC96896488 = -235384175;    double MMYdlkICJGtxkwuwC38474600 = -861033582;    double MMYdlkICJGtxkwuwC49361115 = 47417212;    double MMYdlkICJGtxkwuwC8707042 = -852168318;    double MMYdlkICJGtxkwuwC57009337 = -506635915;    double MMYdlkICJGtxkwuwC11989858 = -197209260;    double MMYdlkICJGtxkwuwC39786776 = -916674333;    double MMYdlkICJGtxkwuwC97447785 = 67540992;     MMYdlkICJGtxkwuwC62355916 = MMYdlkICJGtxkwuwC74921314;     MMYdlkICJGtxkwuwC74921314 = MMYdlkICJGtxkwuwC24196687;     MMYdlkICJGtxkwuwC24196687 = MMYdlkICJGtxkwuwC54117471;     MMYdlkICJGtxkwuwC54117471 = MMYdlkICJGtxkwuwC53862379;     MMYdlkICJGtxkwuwC53862379 = MMYdlkICJGtxkwuwC89942927;     MMYdlkICJGtxkwuwC89942927 = MMYdlkICJGtxkwuwC69066506;     MMYdlkICJGtxkwuwC69066506 = MMYdlkICJGtxkwuwC70947793;     MMYdlkICJGtxkwuwC70947793 = MMYdlkICJGtxkwuwC61678772;     MMYdlkICJGtxkwuwC61678772 = MMYdlkICJGtxkwuwC28343437;     MMYdlkICJGtxkwuwC28343437 = MMYdlkICJGtxkwuwC94270169;     MMYdlkICJGtxkwuwC94270169 = MMYdlkICJGtxkwuwC4086757;     MMYdlkICJGtxkwuwC4086757 = MMYdlkICJGtxkwuwC650794;     MMYdlkICJGtxkwuwC650794 = MMYdlkICJGtxkwuwC50650305;     MMYdlkICJGtxkwuwC50650305 = MMYdlkICJGtxkwuwC73318566;     MMYdlkICJGtxkwuwC73318566 = MMYdlkICJGtxkwuwC4078995;     MMYdlkICJGtxkwuwC4078995 = MMYdlkICJGtxkwuwC40529085;     MMYdlkICJGtxkwuwC40529085 = MMYdlkICJGtxkwuwC61566706;     MMYdlkICJGtxkwuwC61566706 = MMYdlkICJGtxkwuwC70723977;     MMYdlkICJGtxkwuwC70723977 = MMYdlkICJGtxkwuwC51183451;     MMYdlkICJGtxkwuwC51183451 = MMYdlkICJGtxkwuwC23505776;     MMYdlkICJGtxkwuwC23505776 = MMYdlkICJGtxkwuwC69453973;     MMYdlkICJGtxkwuwC69453973 = MMYdlkICJGtxkwuwC53904575;     MMYdlkICJGtxkwuwC53904575 = MMYdlkICJGtxkwuwC75133103;     MMYdlkICJGtxkwuwC75133103 = MMYdlkICJGtxkwuwC65184662;     MMYdlkICJGtxkwuwC65184662 = MMYdlkICJGtxkwuwC49660172;     MMYdlkICJGtxkwuwC49660172 = MMYdlkICJGtxkwuwC12336910;     MMYdlkICJGtxkwuwC12336910 = MMYdlkICJGtxkwuwC68939681;     MMYdlkICJGtxkwuwC68939681 = MMYdlkICJGtxkwuwC7921629;     MMYdlkICJGtxkwuwC7921629 = MMYdlkICJGtxkwuwC28410560;     MMYdlkICJGtxkwuwC28410560 = MMYdlkICJGtxkwuwC39636267;     MMYdlkICJGtxkwuwC39636267 = MMYdlkICJGtxkwuwC74984748;     MMYdlkICJGtxkwuwC74984748 = MMYdlkICJGtxkwuwC45254701;     MMYdlkICJGtxkwuwC45254701 = MMYdlkICJGtxkwuwC66196360;     MMYdlkICJGtxkwuwC66196360 = MMYdlkICJGtxkwuwC87981077;     MMYdlkICJGtxkwuwC87981077 = MMYdlkICJGtxkwuwC48428853;     MMYdlkICJGtxkwuwC48428853 = MMYdlkICJGtxkwuwC88718229;     MMYdlkICJGtxkwuwC88718229 = MMYdlkICJGtxkwuwC85829297;     MMYdlkICJGtxkwuwC85829297 = MMYdlkICJGtxkwuwC42676807;     MMYdlkICJGtxkwuwC42676807 = MMYdlkICJGtxkwuwC26967134;     MMYdlkICJGtxkwuwC26967134 = MMYdlkICJGtxkwuwC8833867;     MMYdlkICJGtxkwuwC8833867 = MMYdlkICJGtxkwuwC20035503;     MMYdlkICJGtxkwuwC20035503 = MMYdlkICJGtxkwuwC45258071;     MMYdlkICJGtxkwuwC45258071 = MMYdlkICJGtxkwuwC28493947;     MMYdlkICJGtxkwuwC28493947 = MMYdlkICJGtxkwuwC16733207;     MMYdlkICJGtxkwuwC16733207 = MMYdlkICJGtxkwuwC55881237;     MMYdlkICJGtxkwuwC55881237 = MMYdlkICJGtxkwuwC64537512;     MMYdlkICJGtxkwuwC64537512 = MMYdlkICJGtxkwuwC58208353;     MMYdlkICJGtxkwuwC58208353 = MMYdlkICJGtxkwuwC82228725;     MMYdlkICJGtxkwuwC82228725 = MMYdlkICJGtxkwuwC33905322;     MMYdlkICJGtxkwuwC33905322 = MMYdlkICJGtxkwuwC66797897;     MMYdlkICJGtxkwuwC66797897 = MMYdlkICJGtxkwuwC48130707;     MMYdlkICJGtxkwuwC48130707 = MMYdlkICJGtxkwuwC37347628;     MMYdlkICJGtxkwuwC37347628 = MMYdlkICJGtxkwuwC96871812;     MMYdlkICJGtxkwuwC96871812 = MMYdlkICJGtxkwuwC70674260;     MMYdlkICJGtxkwuwC70674260 = MMYdlkICJGtxkwuwC92901943;     MMYdlkICJGtxkwuwC92901943 = MMYdlkICJGtxkwuwC21016739;     MMYdlkICJGtxkwuwC21016739 = MMYdlkICJGtxkwuwC49063584;     MMYdlkICJGtxkwuwC49063584 = MMYdlkICJGtxkwuwC88932808;     MMYdlkICJGtxkwuwC88932808 = MMYdlkICJGtxkwuwC4202208;     MMYdlkICJGtxkwuwC4202208 = MMYdlkICJGtxkwuwC77606018;     MMYdlkICJGtxkwuwC77606018 = MMYdlkICJGtxkwuwC126825;     MMYdlkICJGtxkwuwC126825 = MMYdlkICJGtxkwuwC63026165;     MMYdlkICJGtxkwuwC63026165 = MMYdlkICJGtxkwuwC33268213;     MMYdlkICJGtxkwuwC33268213 = MMYdlkICJGtxkwuwC88707170;     MMYdlkICJGtxkwuwC88707170 = MMYdlkICJGtxkwuwC19285421;     MMYdlkICJGtxkwuwC19285421 = MMYdlkICJGtxkwuwC58832055;     MMYdlkICJGtxkwuwC58832055 = MMYdlkICJGtxkwuwC34454434;     MMYdlkICJGtxkwuwC34454434 = MMYdlkICJGtxkwuwC62669227;     MMYdlkICJGtxkwuwC62669227 = MMYdlkICJGtxkwuwC24889713;     MMYdlkICJGtxkwuwC24889713 = MMYdlkICJGtxkwuwC15360765;     MMYdlkICJGtxkwuwC15360765 = MMYdlkICJGtxkwuwC54699788;     MMYdlkICJGtxkwuwC54699788 = MMYdlkICJGtxkwuwC18889899;     MMYdlkICJGtxkwuwC18889899 = MMYdlkICJGtxkwuwC43756844;     MMYdlkICJGtxkwuwC43756844 = MMYdlkICJGtxkwuwC42349585;     MMYdlkICJGtxkwuwC42349585 = MMYdlkICJGtxkwuwC3470273;     MMYdlkICJGtxkwuwC3470273 = MMYdlkICJGtxkwuwC24195903;     MMYdlkICJGtxkwuwC24195903 = MMYdlkICJGtxkwuwC25410629;     MMYdlkICJGtxkwuwC25410629 = MMYdlkICJGtxkwuwC58399896;     MMYdlkICJGtxkwuwC58399896 = MMYdlkICJGtxkwuwC9303426;     MMYdlkICJGtxkwuwC9303426 = MMYdlkICJGtxkwuwC85122659;     MMYdlkICJGtxkwuwC85122659 = MMYdlkICJGtxkwuwC54128556;     MMYdlkICJGtxkwuwC54128556 = MMYdlkICJGtxkwuwC86710956;     MMYdlkICJGtxkwuwC86710956 = MMYdlkICJGtxkwuwC74016306;     MMYdlkICJGtxkwuwC74016306 = MMYdlkICJGtxkwuwC61612663;     MMYdlkICJGtxkwuwC61612663 = MMYdlkICJGtxkwuwC91505560;     MMYdlkICJGtxkwuwC91505560 = MMYdlkICJGtxkwuwC37637121;     MMYdlkICJGtxkwuwC37637121 = MMYdlkICJGtxkwuwC48382889;     MMYdlkICJGtxkwuwC48382889 = MMYdlkICJGtxkwuwC95522099;     MMYdlkICJGtxkwuwC95522099 = MMYdlkICJGtxkwuwC95079134;     MMYdlkICJGtxkwuwC95079134 = MMYdlkICJGtxkwuwC27412115;     MMYdlkICJGtxkwuwC27412115 = MMYdlkICJGtxkwuwC39654646;     MMYdlkICJGtxkwuwC39654646 = MMYdlkICJGtxkwuwC96896488;     MMYdlkICJGtxkwuwC96896488 = MMYdlkICJGtxkwuwC38474600;     MMYdlkICJGtxkwuwC38474600 = MMYdlkICJGtxkwuwC49361115;     MMYdlkICJGtxkwuwC49361115 = MMYdlkICJGtxkwuwC8707042;     MMYdlkICJGtxkwuwC8707042 = MMYdlkICJGtxkwuwC57009337;     MMYdlkICJGtxkwuwC57009337 = MMYdlkICJGtxkwuwC11989858;     MMYdlkICJGtxkwuwC11989858 = MMYdlkICJGtxkwuwC39786776;     MMYdlkICJGtxkwuwC39786776 = MMYdlkICJGtxkwuwC97447785;     MMYdlkICJGtxkwuwC97447785 = MMYdlkICJGtxkwuwC62355916;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VrpuQuitdEPzjNIC61471635() {     double YqoyEikiwBhcOxicZ97814884 = -398735908;    double YqoyEikiwBhcOxicZ88193392 = -549637743;    double YqoyEikiwBhcOxicZ61583164 = -714736682;    double YqoyEikiwBhcOxicZ26340826 = -190421935;    double YqoyEikiwBhcOxicZ10147648 = -901608716;    double YqoyEikiwBhcOxicZ26165800 = -29268703;    double YqoyEikiwBhcOxicZ79723912 = -523076850;    double YqoyEikiwBhcOxicZ33079355 = -714735515;    double YqoyEikiwBhcOxicZ63196378 = -888586905;    double YqoyEikiwBhcOxicZ43246149 = 74556864;    double YqoyEikiwBhcOxicZ32451905 = -46241282;    double YqoyEikiwBhcOxicZ4654905 = -912453563;    double YqoyEikiwBhcOxicZ38885439 = -554711942;    double YqoyEikiwBhcOxicZ3791168 = -100440189;    double YqoyEikiwBhcOxicZ31513082 = -193549989;    double YqoyEikiwBhcOxicZ87620295 = -377559522;    double YqoyEikiwBhcOxicZ96365021 = -279400004;    double YqoyEikiwBhcOxicZ61326898 = -94319617;    double YqoyEikiwBhcOxicZ71274307 = -528352378;    double YqoyEikiwBhcOxicZ45020670 = -357343353;    double YqoyEikiwBhcOxicZ36592638 = -859143385;    double YqoyEikiwBhcOxicZ72875498 = -971695460;    double YqoyEikiwBhcOxicZ24429955 = -209376510;    double YqoyEikiwBhcOxicZ68926048 = -767333591;    double YqoyEikiwBhcOxicZ73202613 = 48268376;    double YqoyEikiwBhcOxicZ60358343 = -955809498;    double YqoyEikiwBhcOxicZ72757483 = -170447839;    double YqoyEikiwBhcOxicZ9668107 = -285203883;    double YqoyEikiwBhcOxicZ54589843 = -535741792;    double YqoyEikiwBhcOxicZ33943971 = -664439202;    double YqoyEikiwBhcOxicZ58219498 = 59089808;    double YqoyEikiwBhcOxicZ12807767 = -638704928;    double YqoyEikiwBhcOxicZ57472712 = -205733598;    double YqoyEikiwBhcOxicZ95944786 = -394220598;    double YqoyEikiwBhcOxicZ53789531 = -816303567;    double YqoyEikiwBhcOxicZ99989697 = -609129919;    double YqoyEikiwBhcOxicZ38130046 = -220212321;    double YqoyEikiwBhcOxicZ37044700 = -659564194;    double YqoyEikiwBhcOxicZ73067368 = -40687038;    double YqoyEikiwBhcOxicZ91238665 = 79397556;    double YqoyEikiwBhcOxicZ42285475 = -84286640;    double YqoyEikiwBhcOxicZ9400630 = -110035591;    double YqoyEikiwBhcOxicZ34641449 = -620713513;    double YqoyEikiwBhcOxicZ68625284 = -192833176;    double YqoyEikiwBhcOxicZ16749666 = -536407127;    double YqoyEikiwBhcOxicZ1842566 = -689057579;    double YqoyEikiwBhcOxicZ83882676 = -945085822;    double YqoyEikiwBhcOxicZ75934665 = -976588446;    double YqoyEikiwBhcOxicZ5121391 = -450391308;    double YqoyEikiwBhcOxicZ58434911 = -106306632;    double YqoyEikiwBhcOxicZ47568620 = -926073645;    double YqoyEikiwBhcOxicZ31616179 = -465658093;    double YqoyEikiwBhcOxicZ5968777 = -312368160;    double YqoyEikiwBhcOxicZ12863659 = -723071676;    double YqoyEikiwBhcOxicZ24878997 = -72695807;    double YqoyEikiwBhcOxicZ24939387 = -427040448;    double YqoyEikiwBhcOxicZ63763437 = -240261233;    double YqoyEikiwBhcOxicZ92657115 = -947403092;    double YqoyEikiwBhcOxicZ53138212 = -138690312;    double YqoyEikiwBhcOxicZ49789305 = -945799219;    double YqoyEikiwBhcOxicZ53408316 = -858820865;    double YqoyEikiwBhcOxicZ70055806 = -137872968;    double YqoyEikiwBhcOxicZ78489512 = -78993723;    double YqoyEikiwBhcOxicZ29252407 = -124147703;    double YqoyEikiwBhcOxicZ85026650 = -984532945;    double YqoyEikiwBhcOxicZ19644138 = -407536355;    double YqoyEikiwBhcOxicZ47182193 = -606719965;    double YqoyEikiwBhcOxicZ42940653 = -60491344;    double YqoyEikiwBhcOxicZ50001636 = -284136623;    double YqoyEikiwBhcOxicZ31523385 = -584420071;    double YqoyEikiwBhcOxicZ49490249 = -57347202;    double YqoyEikiwBhcOxicZ59320321 = -619835810;    double YqoyEikiwBhcOxicZ88259529 = 46367420;    double YqoyEikiwBhcOxicZ80035641 = -507749934;    double YqoyEikiwBhcOxicZ2735196 = -173056714;    double YqoyEikiwBhcOxicZ27192009 = -649107795;    double YqoyEikiwBhcOxicZ38234049 = -250981948;    double YqoyEikiwBhcOxicZ55804670 = 83456665;    double YqoyEikiwBhcOxicZ52176383 = -130926464;    double YqoyEikiwBhcOxicZ71360048 = -262674045;    double YqoyEikiwBhcOxicZ76475667 = 89276323;    double YqoyEikiwBhcOxicZ96822818 = -193859394;    double YqoyEikiwBhcOxicZ4546716 = -834812575;    double YqoyEikiwBhcOxicZ96154931 = -329435161;    double YqoyEikiwBhcOxicZ86375350 = -738365557;    double YqoyEikiwBhcOxicZ26603319 = -475252099;    double YqoyEikiwBhcOxicZ6838991 = -226336768;    double YqoyEikiwBhcOxicZ44609053 = -482661923;    double YqoyEikiwBhcOxicZ71065790 = -221524792;    double YqoyEikiwBhcOxicZ28850145 = -289263120;    double YqoyEikiwBhcOxicZ36226260 = -268868687;    double YqoyEikiwBhcOxicZ45472931 = -272809229;    double YqoyEikiwBhcOxicZ83906487 = -420873883;    double YqoyEikiwBhcOxicZ23278064 = -94887820;    double YqoyEikiwBhcOxicZ37830350 = -61781580;    double YqoyEikiwBhcOxicZ72229668 = -946413673;    double YqoyEikiwBhcOxicZ30911118 = 68958132;    double YqoyEikiwBhcOxicZ5389043 = -396565810;    double YqoyEikiwBhcOxicZ83598633 = -208300232;    double YqoyEikiwBhcOxicZ97105528 = -398735908;     YqoyEikiwBhcOxicZ97814884 = YqoyEikiwBhcOxicZ88193392;     YqoyEikiwBhcOxicZ88193392 = YqoyEikiwBhcOxicZ61583164;     YqoyEikiwBhcOxicZ61583164 = YqoyEikiwBhcOxicZ26340826;     YqoyEikiwBhcOxicZ26340826 = YqoyEikiwBhcOxicZ10147648;     YqoyEikiwBhcOxicZ10147648 = YqoyEikiwBhcOxicZ26165800;     YqoyEikiwBhcOxicZ26165800 = YqoyEikiwBhcOxicZ79723912;     YqoyEikiwBhcOxicZ79723912 = YqoyEikiwBhcOxicZ33079355;     YqoyEikiwBhcOxicZ33079355 = YqoyEikiwBhcOxicZ63196378;     YqoyEikiwBhcOxicZ63196378 = YqoyEikiwBhcOxicZ43246149;     YqoyEikiwBhcOxicZ43246149 = YqoyEikiwBhcOxicZ32451905;     YqoyEikiwBhcOxicZ32451905 = YqoyEikiwBhcOxicZ4654905;     YqoyEikiwBhcOxicZ4654905 = YqoyEikiwBhcOxicZ38885439;     YqoyEikiwBhcOxicZ38885439 = YqoyEikiwBhcOxicZ3791168;     YqoyEikiwBhcOxicZ3791168 = YqoyEikiwBhcOxicZ31513082;     YqoyEikiwBhcOxicZ31513082 = YqoyEikiwBhcOxicZ87620295;     YqoyEikiwBhcOxicZ87620295 = YqoyEikiwBhcOxicZ96365021;     YqoyEikiwBhcOxicZ96365021 = YqoyEikiwBhcOxicZ61326898;     YqoyEikiwBhcOxicZ61326898 = YqoyEikiwBhcOxicZ71274307;     YqoyEikiwBhcOxicZ71274307 = YqoyEikiwBhcOxicZ45020670;     YqoyEikiwBhcOxicZ45020670 = YqoyEikiwBhcOxicZ36592638;     YqoyEikiwBhcOxicZ36592638 = YqoyEikiwBhcOxicZ72875498;     YqoyEikiwBhcOxicZ72875498 = YqoyEikiwBhcOxicZ24429955;     YqoyEikiwBhcOxicZ24429955 = YqoyEikiwBhcOxicZ68926048;     YqoyEikiwBhcOxicZ68926048 = YqoyEikiwBhcOxicZ73202613;     YqoyEikiwBhcOxicZ73202613 = YqoyEikiwBhcOxicZ60358343;     YqoyEikiwBhcOxicZ60358343 = YqoyEikiwBhcOxicZ72757483;     YqoyEikiwBhcOxicZ72757483 = YqoyEikiwBhcOxicZ9668107;     YqoyEikiwBhcOxicZ9668107 = YqoyEikiwBhcOxicZ54589843;     YqoyEikiwBhcOxicZ54589843 = YqoyEikiwBhcOxicZ33943971;     YqoyEikiwBhcOxicZ33943971 = YqoyEikiwBhcOxicZ58219498;     YqoyEikiwBhcOxicZ58219498 = YqoyEikiwBhcOxicZ12807767;     YqoyEikiwBhcOxicZ12807767 = YqoyEikiwBhcOxicZ57472712;     YqoyEikiwBhcOxicZ57472712 = YqoyEikiwBhcOxicZ95944786;     YqoyEikiwBhcOxicZ95944786 = YqoyEikiwBhcOxicZ53789531;     YqoyEikiwBhcOxicZ53789531 = YqoyEikiwBhcOxicZ99989697;     YqoyEikiwBhcOxicZ99989697 = YqoyEikiwBhcOxicZ38130046;     YqoyEikiwBhcOxicZ38130046 = YqoyEikiwBhcOxicZ37044700;     YqoyEikiwBhcOxicZ37044700 = YqoyEikiwBhcOxicZ73067368;     YqoyEikiwBhcOxicZ73067368 = YqoyEikiwBhcOxicZ91238665;     YqoyEikiwBhcOxicZ91238665 = YqoyEikiwBhcOxicZ42285475;     YqoyEikiwBhcOxicZ42285475 = YqoyEikiwBhcOxicZ9400630;     YqoyEikiwBhcOxicZ9400630 = YqoyEikiwBhcOxicZ34641449;     YqoyEikiwBhcOxicZ34641449 = YqoyEikiwBhcOxicZ68625284;     YqoyEikiwBhcOxicZ68625284 = YqoyEikiwBhcOxicZ16749666;     YqoyEikiwBhcOxicZ16749666 = YqoyEikiwBhcOxicZ1842566;     YqoyEikiwBhcOxicZ1842566 = YqoyEikiwBhcOxicZ83882676;     YqoyEikiwBhcOxicZ83882676 = YqoyEikiwBhcOxicZ75934665;     YqoyEikiwBhcOxicZ75934665 = YqoyEikiwBhcOxicZ5121391;     YqoyEikiwBhcOxicZ5121391 = YqoyEikiwBhcOxicZ58434911;     YqoyEikiwBhcOxicZ58434911 = YqoyEikiwBhcOxicZ47568620;     YqoyEikiwBhcOxicZ47568620 = YqoyEikiwBhcOxicZ31616179;     YqoyEikiwBhcOxicZ31616179 = YqoyEikiwBhcOxicZ5968777;     YqoyEikiwBhcOxicZ5968777 = YqoyEikiwBhcOxicZ12863659;     YqoyEikiwBhcOxicZ12863659 = YqoyEikiwBhcOxicZ24878997;     YqoyEikiwBhcOxicZ24878997 = YqoyEikiwBhcOxicZ24939387;     YqoyEikiwBhcOxicZ24939387 = YqoyEikiwBhcOxicZ63763437;     YqoyEikiwBhcOxicZ63763437 = YqoyEikiwBhcOxicZ92657115;     YqoyEikiwBhcOxicZ92657115 = YqoyEikiwBhcOxicZ53138212;     YqoyEikiwBhcOxicZ53138212 = YqoyEikiwBhcOxicZ49789305;     YqoyEikiwBhcOxicZ49789305 = YqoyEikiwBhcOxicZ53408316;     YqoyEikiwBhcOxicZ53408316 = YqoyEikiwBhcOxicZ70055806;     YqoyEikiwBhcOxicZ70055806 = YqoyEikiwBhcOxicZ78489512;     YqoyEikiwBhcOxicZ78489512 = YqoyEikiwBhcOxicZ29252407;     YqoyEikiwBhcOxicZ29252407 = YqoyEikiwBhcOxicZ85026650;     YqoyEikiwBhcOxicZ85026650 = YqoyEikiwBhcOxicZ19644138;     YqoyEikiwBhcOxicZ19644138 = YqoyEikiwBhcOxicZ47182193;     YqoyEikiwBhcOxicZ47182193 = YqoyEikiwBhcOxicZ42940653;     YqoyEikiwBhcOxicZ42940653 = YqoyEikiwBhcOxicZ50001636;     YqoyEikiwBhcOxicZ50001636 = YqoyEikiwBhcOxicZ31523385;     YqoyEikiwBhcOxicZ31523385 = YqoyEikiwBhcOxicZ49490249;     YqoyEikiwBhcOxicZ49490249 = YqoyEikiwBhcOxicZ59320321;     YqoyEikiwBhcOxicZ59320321 = YqoyEikiwBhcOxicZ88259529;     YqoyEikiwBhcOxicZ88259529 = YqoyEikiwBhcOxicZ80035641;     YqoyEikiwBhcOxicZ80035641 = YqoyEikiwBhcOxicZ2735196;     YqoyEikiwBhcOxicZ2735196 = YqoyEikiwBhcOxicZ27192009;     YqoyEikiwBhcOxicZ27192009 = YqoyEikiwBhcOxicZ38234049;     YqoyEikiwBhcOxicZ38234049 = YqoyEikiwBhcOxicZ55804670;     YqoyEikiwBhcOxicZ55804670 = YqoyEikiwBhcOxicZ52176383;     YqoyEikiwBhcOxicZ52176383 = YqoyEikiwBhcOxicZ71360048;     YqoyEikiwBhcOxicZ71360048 = YqoyEikiwBhcOxicZ76475667;     YqoyEikiwBhcOxicZ76475667 = YqoyEikiwBhcOxicZ96822818;     YqoyEikiwBhcOxicZ96822818 = YqoyEikiwBhcOxicZ4546716;     YqoyEikiwBhcOxicZ4546716 = YqoyEikiwBhcOxicZ96154931;     YqoyEikiwBhcOxicZ96154931 = YqoyEikiwBhcOxicZ86375350;     YqoyEikiwBhcOxicZ86375350 = YqoyEikiwBhcOxicZ26603319;     YqoyEikiwBhcOxicZ26603319 = YqoyEikiwBhcOxicZ6838991;     YqoyEikiwBhcOxicZ6838991 = YqoyEikiwBhcOxicZ44609053;     YqoyEikiwBhcOxicZ44609053 = YqoyEikiwBhcOxicZ71065790;     YqoyEikiwBhcOxicZ71065790 = YqoyEikiwBhcOxicZ28850145;     YqoyEikiwBhcOxicZ28850145 = YqoyEikiwBhcOxicZ36226260;     YqoyEikiwBhcOxicZ36226260 = YqoyEikiwBhcOxicZ45472931;     YqoyEikiwBhcOxicZ45472931 = YqoyEikiwBhcOxicZ83906487;     YqoyEikiwBhcOxicZ83906487 = YqoyEikiwBhcOxicZ23278064;     YqoyEikiwBhcOxicZ23278064 = YqoyEikiwBhcOxicZ37830350;     YqoyEikiwBhcOxicZ37830350 = YqoyEikiwBhcOxicZ72229668;     YqoyEikiwBhcOxicZ72229668 = YqoyEikiwBhcOxicZ30911118;     YqoyEikiwBhcOxicZ30911118 = YqoyEikiwBhcOxicZ5389043;     YqoyEikiwBhcOxicZ5389043 = YqoyEikiwBhcOxicZ83598633;     YqoyEikiwBhcOxicZ83598633 = YqoyEikiwBhcOxicZ97105528;     YqoyEikiwBhcOxicZ97105528 = YqoyEikiwBhcOxicZ97814884;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void TXPbMlaJWSltiiZc76520703() {     double ZZrnBcFkMBKlgVKfs3931992 = -510637796;    double ZZrnBcFkMBKlgVKfs31566672 = -259237523;    double ZZrnBcFkMBKlgVKfs77625347 = -865569604;    double ZZrnBcFkMBKlgVKfs53415606 = -381170566;    double ZZrnBcFkMBKlgVKfs79043136 = -322258588;    double ZZrnBcFkMBKlgVKfs17583002 = -293764938;    double ZZrnBcFkMBKlgVKfs40212879 = -662041827;    double ZZrnBcFkMBKlgVKfs82839245 = 98390572;    double ZZrnBcFkMBKlgVKfs27838873 = -656619570;    double ZZrnBcFkMBKlgVKfs84475808 = -254418613;    double ZZrnBcFkMBKlgVKfs92562194 = 76321131;    double ZZrnBcFkMBKlgVKfs36477947 = -256859765;    double ZZrnBcFkMBKlgVKfs25094558 = -994603031;    double ZZrnBcFkMBKlgVKfs86934844 = -632678757;    double ZZrnBcFkMBKlgVKfs27255509 = -884012156;    double ZZrnBcFkMBKlgVKfs29125689 = -350559065;    double ZZrnBcFkMBKlgVKfs90201024 = -895945117;    double ZZrnBcFkMBKlgVKfs58380051 = -859842260;    double ZZrnBcFkMBKlgVKfs33104149 = -934121475;    double ZZrnBcFkMBKlgVKfs25570835 = -714246273;    double ZZrnBcFkMBKlgVKfs25145832 = -714630840;    double ZZrnBcFkMBKlgVKfs59396521 = -879625177;    double ZZrnBcFkMBKlgVKfs60020747 = 44106003;    double ZZrnBcFkMBKlgVKfs52918422 = -989638579;    double ZZrnBcFkMBKlgVKfs16902263 = -324927657;    double ZZrnBcFkMBKlgVKfs85080018 = -834464821;    double ZZrnBcFkMBKlgVKfs71217261 = -71110042;    double ZZrnBcFkMBKlgVKfs22775232 = -351132837;    double ZZrnBcFkMBKlgVKfs20722099 = -758124497;    double ZZrnBcFkMBKlgVKfs2559608 = -481910069;    double ZZrnBcFkMBKlgVKfs50462048 = -954231130;    double ZZrnBcFkMBKlgVKfs10418389 = -348151743;    double ZZrnBcFkMBKlgVKfs67183660 = -98346969;    double ZZrnBcFkMBKlgVKfs90775350 = -145762363;    double ZZrnBcFkMBKlgVKfs15264184 = -355036673;    double ZZrnBcFkMBKlgVKfs68715610 = -823156027;    double ZZrnBcFkMBKlgVKfs19391981 = -173848190;    double ZZrnBcFkMBKlgVKfs71134913 = -431070204;    double ZZrnBcFkMBKlgVKfs46677622 = -34610535;    double ZZrnBcFkMBKlgVKfs78740603 = -394041666;    double ZZrnBcFkMBKlgVKfs47954699 = -149560534;    double ZZrnBcFkMBKlgVKfs41551086 = -380177869;    double ZZrnBcFkMBKlgVKfs12708428 = -701518097;    double ZZrnBcFkMBKlgVKfs56033025 = -212176114;    double ZZrnBcFkMBKlgVKfs28702131 = -198305282;    double ZZrnBcFkMBKlgVKfs13781761 = -674273696;    double ZZrnBcFkMBKlgVKfs1661251 = 36138641;    double ZZrnBcFkMBKlgVKfs83891606 = -986483978;    double ZZrnBcFkMBKlgVKfs14797120 = -536128296;    double ZZrnBcFkMBKlgVKfs71346564 = -193317440;    double ZZrnBcFkMBKlgVKfs83269298 = -768367317;    double ZZrnBcFkMBKlgVKfs42425899 = -266384413;    double ZZrnBcFkMBKlgVKfs54217234 = -104157748;    double ZZrnBcFkMBKlgVKfs8832539 = 56458179;    double ZZrnBcFkMBKlgVKfs51374871 = -773227589;    double ZZrnBcFkMBKlgVKfs44535470 = -631012620;    double ZZrnBcFkMBKlgVKfs71545925 = -203343527;    double ZZrnBcFkMBKlgVKfs24706926 = -875931025;    double ZZrnBcFkMBKlgVKfs36513343 = 43757091;    double ZZrnBcFkMBKlgVKfs93963117 = -487793767;    double ZZrnBcFkMBKlgVKfs46365740 = -122654897;    double ZZrnBcFkMBKlgVKfs17437647 = -210908991;    double ZZrnBcFkMBKlgVKfs62117147 = -143484932;    double ZZrnBcFkMBKlgVKfs25279266 = -74709502;    double ZZrnBcFkMBKlgVKfs34013761 = -300187484;    double ZZrnBcFkMBKlgVKfs82143806 = -575527126;    double ZZrnBcFkMBKlgVKfs69294287 = -58512797;    double ZZrnBcFkMBKlgVKfs34319207 = -748840668;    double ZZrnBcFkMBKlgVKfs71670661 = -177642085;    double ZZrnBcFkMBKlgVKfs58539898 = 39143871;    double ZZrnBcFkMBKlgVKfs9733709 = -76710876;    double ZZrnBcFkMBKlgVKfs19066112 = -364874913;    double ZZrnBcFkMBKlgVKfs11702429 = -725231725;    double ZZrnBcFkMBKlgVKfs54363545 = -440079809;    double ZZrnBcFkMBKlgVKfs77616136 = -464685740;    double ZZrnBcFkMBKlgVKfs83594745 = -234452972;    double ZZrnBcFkMBKlgVKfs46688094 = -78107080;    double ZZrnBcFkMBKlgVKfs3987722 = -743717883;    double ZZrnBcFkMBKlgVKfs24216292 = -691333298;    double ZZrnBcFkMBKlgVKfs3120503 = -650653962;    double ZZrnBcFkMBKlgVKfs83418768 = -770603462;    double ZZrnBcFkMBKlgVKfs87325655 = -84626065;    double ZZrnBcFkMBKlgVKfs7978112 = -815004541;    double ZZrnBcFkMBKlgVKfs49375534 = -464807057;    double ZZrnBcFkMBKlgVKfs19290309 = -713542753;    double ZZrnBcFkMBKlgVKfs8036149 = -587846717;    double ZZrnBcFkMBKlgVKfs56201155 = -143993996;    double ZZrnBcFkMBKlgVKfs58351121 = -54805148;    double ZZrnBcFkMBKlgVKfs39400480 = -372534775;    double ZZrnBcFkMBKlgVKfs70728713 = -724024053;    double ZZrnBcFkMBKlgVKfs97169685 = -519812500;    double ZZrnBcFkMBKlgVKfs94685055 = -297917165;    double ZZrnBcFkMBKlgVKfs34621570 = -374827295;    double ZZrnBcFkMBKlgVKfs52714504 = -546816769;    double ZZrnBcFkMBKlgVKfs32374863 = -171386770;    double ZZrnBcFkMBKlgVKfs30517052 = -938651543;    double ZZrnBcFkMBKlgVKfs79433939 = -136692938;    double ZZrnBcFkMBKlgVKfs87429161 = -526808596;    double ZZrnBcFkMBKlgVKfs22019265 = -911988630;    double ZZrnBcFkMBKlgVKfs46558325 = -510637796;     ZZrnBcFkMBKlgVKfs3931992 = ZZrnBcFkMBKlgVKfs31566672;     ZZrnBcFkMBKlgVKfs31566672 = ZZrnBcFkMBKlgVKfs77625347;     ZZrnBcFkMBKlgVKfs77625347 = ZZrnBcFkMBKlgVKfs53415606;     ZZrnBcFkMBKlgVKfs53415606 = ZZrnBcFkMBKlgVKfs79043136;     ZZrnBcFkMBKlgVKfs79043136 = ZZrnBcFkMBKlgVKfs17583002;     ZZrnBcFkMBKlgVKfs17583002 = ZZrnBcFkMBKlgVKfs40212879;     ZZrnBcFkMBKlgVKfs40212879 = ZZrnBcFkMBKlgVKfs82839245;     ZZrnBcFkMBKlgVKfs82839245 = ZZrnBcFkMBKlgVKfs27838873;     ZZrnBcFkMBKlgVKfs27838873 = ZZrnBcFkMBKlgVKfs84475808;     ZZrnBcFkMBKlgVKfs84475808 = ZZrnBcFkMBKlgVKfs92562194;     ZZrnBcFkMBKlgVKfs92562194 = ZZrnBcFkMBKlgVKfs36477947;     ZZrnBcFkMBKlgVKfs36477947 = ZZrnBcFkMBKlgVKfs25094558;     ZZrnBcFkMBKlgVKfs25094558 = ZZrnBcFkMBKlgVKfs86934844;     ZZrnBcFkMBKlgVKfs86934844 = ZZrnBcFkMBKlgVKfs27255509;     ZZrnBcFkMBKlgVKfs27255509 = ZZrnBcFkMBKlgVKfs29125689;     ZZrnBcFkMBKlgVKfs29125689 = ZZrnBcFkMBKlgVKfs90201024;     ZZrnBcFkMBKlgVKfs90201024 = ZZrnBcFkMBKlgVKfs58380051;     ZZrnBcFkMBKlgVKfs58380051 = ZZrnBcFkMBKlgVKfs33104149;     ZZrnBcFkMBKlgVKfs33104149 = ZZrnBcFkMBKlgVKfs25570835;     ZZrnBcFkMBKlgVKfs25570835 = ZZrnBcFkMBKlgVKfs25145832;     ZZrnBcFkMBKlgVKfs25145832 = ZZrnBcFkMBKlgVKfs59396521;     ZZrnBcFkMBKlgVKfs59396521 = ZZrnBcFkMBKlgVKfs60020747;     ZZrnBcFkMBKlgVKfs60020747 = ZZrnBcFkMBKlgVKfs52918422;     ZZrnBcFkMBKlgVKfs52918422 = ZZrnBcFkMBKlgVKfs16902263;     ZZrnBcFkMBKlgVKfs16902263 = ZZrnBcFkMBKlgVKfs85080018;     ZZrnBcFkMBKlgVKfs85080018 = ZZrnBcFkMBKlgVKfs71217261;     ZZrnBcFkMBKlgVKfs71217261 = ZZrnBcFkMBKlgVKfs22775232;     ZZrnBcFkMBKlgVKfs22775232 = ZZrnBcFkMBKlgVKfs20722099;     ZZrnBcFkMBKlgVKfs20722099 = ZZrnBcFkMBKlgVKfs2559608;     ZZrnBcFkMBKlgVKfs2559608 = ZZrnBcFkMBKlgVKfs50462048;     ZZrnBcFkMBKlgVKfs50462048 = ZZrnBcFkMBKlgVKfs10418389;     ZZrnBcFkMBKlgVKfs10418389 = ZZrnBcFkMBKlgVKfs67183660;     ZZrnBcFkMBKlgVKfs67183660 = ZZrnBcFkMBKlgVKfs90775350;     ZZrnBcFkMBKlgVKfs90775350 = ZZrnBcFkMBKlgVKfs15264184;     ZZrnBcFkMBKlgVKfs15264184 = ZZrnBcFkMBKlgVKfs68715610;     ZZrnBcFkMBKlgVKfs68715610 = ZZrnBcFkMBKlgVKfs19391981;     ZZrnBcFkMBKlgVKfs19391981 = ZZrnBcFkMBKlgVKfs71134913;     ZZrnBcFkMBKlgVKfs71134913 = ZZrnBcFkMBKlgVKfs46677622;     ZZrnBcFkMBKlgVKfs46677622 = ZZrnBcFkMBKlgVKfs78740603;     ZZrnBcFkMBKlgVKfs78740603 = ZZrnBcFkMBKlgVKfs47954699;     ZZrnBcFkMBKlgVKfs47954699 = ZZrnBcFkMBKlgVKfs41551086;     ZZrnBcFkMBKlgVKfs41551086 = ZZrnBcFkMBKlgVKfs12708428;     ZZrnBcFkMBKlgVKfs12708428 = ZZrnBcFkMBKlgVKfs56033025;     ZZrnBcFkMBKlgVKfs56033025 = ZZrnBcFkMBKlgVKfs28702131;     ZZrnBcFkMBKlgVKfs28702131 = ZZrnBcFkMBKlgVKfs13781761;     ZZrnBcFkMBKlgVKfs13781761 = ZZrnBcFkMBKlgVKfs1661251;     ZZrnBcFkMBKlgVKfs1661251 = ZZrnBcFkMBKlgVKfs83891606;     ZZrnBcFkMBKlgVKfs83891606 = ZZrnBcFkMBKlgVKfs14797120;     ZZrnBcFkMBKlgVKfs14797120 = ZZrnBcFkMBKlgVKfs71346564;     ZZrnBcFkMBKlgVKfs71346564 = ZZrnBcFkMBKlgVKfs83269298;     ZZrnBcFkMBKlgVKfs83269298 = ZZrnBcFkMBKlgVKfs42425899;     ZZrnBcFkMBKlgVKfs42425899 = ZZrnBcFkMBKlgVKfs54217234;     ZZrnBcFkMBKlgVKfs54217234 = ZZrnBcFkMBKlgVKfs8832539;     ZZrnBcFkMBKlgVKfs8832539 = ZZrnBcFkMBKlgVKfs51374871;     ZZrnBcFkMBKlgVKfs51374871 = ZZrnBcFkMBKlgVKfs44535470;     ZZrnBcFkMBKlgVKfs44535470 = ZZrnBcFkMBKlgVKfs71545925;     ZZrnBcFkMBKlgVKfs71545925 = ZZrnBcFkMBKlgVKfs24706926;     ZZrnBcFkMBKlgVKfs24706926 = ZZrnBcFkMBKlgVKfs36513343;     ZZrnBcFkMBKlgVKfs36513343 = ZZrnBcFkMBKlgVKfs93963117;     ZZrnBcFkMBKlgVKfs93963117 = ZZrnBcFkMBKlgVKfs46365740;     ZZrnBcFkMBKlgVKfs46365740 = ZZrnBcFkMBKlgVKfs17437647;     ZZrnBcFkMBKlgVKfs17437647 = ZZrnBcFkMBKlgVKfs62117147;     ZZrnBcFkMBKlgVKfs62117147 = ZZrnBcFkMBKlgVKfs25279266;     ZZrnBcFkMBKlgVKfs25279266 = ZZrnBcFkMBKlgVKfs34013761;     ZZrnBcFkMBKlgVKfs34013761 = ZZrnBcFkMBKlgVKfs82143806;     ZZrnBcFkMBKlgVKfs82143806 = ZZrnBcFkMBKlgVKfs69294287;     ZZrnBcFkMBKlgVKfs69294287 = ZZrnBcFkMBKlgVKfs34319207;     ZZrnBcFkMBKlgVKfs34319207 = ZZrnBcFkMBKlgVKfs71670661;     ZZrnBcFkMBKlgVKfs71670661 = ZZrnBcFkMBKlgVKfs58539898;     ZZrnBcFkMBKlgVKfs58539898 = ZZrnBcFkMBKlgVKfs9733709;     ZZrnBcFkMBKlgVKfs9733709 = ZZrnBcFkMBKlgVKfs19066112;     ZZrnBcFkMBKlgVKfs19066112 = ZZrnBcFkMBKlgVKfs11702429;     ZZrnBcFkMBKlgVKfs11702429 = ZZrnBcFkMBKlgVKfs54363545;     ZZrnBcFkMBKlgVKfs54363545 = ZZrnBcFkMBKlgVKfs77616136;     ZZrnBcFkMBKlgVKfs77616136 = ZZrnBcFkMBKlgVKfs83594745;     ZZrnBcFkMBKlgVKfs83594745 = ZZrnBcFkMBKlgVKfs46688094;     ZZrnBcFkMBKlgVKfs46688094 = ZZrnBcFkMBKlgVKfs3987722;     ZZrnBcFkMBKlgVKfs3987722 = ZZrnBcFkMBKlgVKfs24216292;     ZZrnBcFkMBKlgVKfs24216292 = ZZrnBcFkMBKlgVKfs3120503;     ZZrnBcFkMBKlgVKfs3120503 = ZZrnBcFkMBKlgVKfs83418768;     ZZrnBcFkMBKlgVKfs83418768 = ZZrnBcFkMBKlgVKfs87325655;     ZZrnBcFkMBKlgVKfs87325655 = ZZrnBcFkMBKlgVKfs7978112;     ZZrnBcFkMBKlgVKfs7978112 = ZZrnBcFkMBKlgVKfs49375534;     ZZrnBcFkMBKlgVKfs49375534 = ZZrnBcFkMBKlgVKfs19290309;     ZZrnBcFkMBKlgVKfs19290309 = ZZrnBcFkMBKlgVKfs8036149;     ZZrnBcFkMBKlgVKfs8036149 = ZZrnBcFkMBKlgVKfs56201155;     ZZrnBcFkMBKlgVKfs56201155 = ZZrnBcFkMBKlgVKfs58351121;     ZZrnBcFkMBKlgVKfs58351121 = ZZrnBcFkMBKlgVKfs39400480;     ZZrnBcFkMBKlgVKfs39400480 = ZZrnBcFkMBKlgVKfs70728713;     ZZrnBcFkMBKlgVKfs70728713 = ZZrnBcFkMBKlgVKfs97169685;     ZZrnBcFkMBKlgVKfs97169685 = ZZrnBcFkMBKlgVKfs94685055;     ZZrnBcFkMBKlgVKfs94685055 = ZZrnBcFkMBKlgVKfs34621570;     ZZrnBcFkMBKlgVKfs34621570 = ZZrnBcFkMBKlgVKfs52714504;     ZZrnBcFkMBKlgVKfs52714504 = ZZrnBcFkMBKlgVKfs32374863;     ZZrnBcFkMBKlgVKfs32374863 = ZZrnBcFkMBKlgVKfs30517052;     ZZrnBcFkMBKlgVKfs30517052 = ZZrnBcFkMBKlgVKfs79433939;     ZZrnBcFkMBKlgVKfs79433939 = ZZrnBcFkMBKlgVKfs87429161;     ZZrnBcFkMBKlgVKfs87429161 = ZZrnBcFkMBKlgVKfs22019265;     ZZrnBcFkMBKlgVKfs22019265 = ZZrnBcFkMBKlgVKfs46558325;     ZZrnBcFkMBKlgVKfs46558325 = ZZrnBcFkMBKlgVKfs3931992;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tTBCoTidOBaUPKkG43812302() {     double EXZcNhvLODSmwwiLX39390960 = -976914696;    double EXZcNhvLODSmwwiLX44838750 = -139789130;    double EXZcNhvLODSmwwiLX15011825 = -531236679;    double EXZcNhvLODSmwwiLX25638961 = -202321637;    double EXZcNhvLODSmwwiLX35328406 = -427857433;    double EXZcNhvLODSmwwiLX53805874 = -556112834;    double EXZcNhvLODSmwwiLX50870285 = -506767562;    double EXZcNhvLODSmwwiLX44970807 = -645843568;    double EXZcNhvLODSmwwiLX29356479 = -44950977;    double EXZcNhvLODSmwwiLX99378520 = 80894389;    double EXZcNhvLODSmwwiLX30743930 = -825686598;    double EXZcNhvLODSmwwiLX37046096 = -120980537;    double EXZcNhvLODSmwwiLX63329203 = -446615535;    double EXZcNhvLODSmwwiLX40075708 = -609042166;    double EXZcNhvLODSmwwiLX85450025 = -287437171;    double EXZcNhvLODSmwwiLX12666990 = 13629328;    double EXZcNhvLODSmwwiLX46036960 = -322919669;    double EXZcNhvLODSmwwiLX58140242 = -933270328;    double EXZcNhvLODSmwwiLX33654479 = -150684896;    double EXZcNhvLODSmwwiLX19408054 = -40807339;    double EXZcNhvLODSmwwiLX38232695 = -371905529;    double EXZcNhvLODSmwwiLX62818046 = -966261471;    double EXZcNhvLODSmwwiLX30546127 = -693526181;    double EXZcNhvLODSmwwiLX46711367 = -400463183;    double EXZcNhvLODSmwwiLX24920215 = -348912855;    double EXZcNhvLODSmwwiLX95778190 = -523041628;    double EXZcNhvLODSmwwiLX31637836 = -977394183;    double EXZcNhvLODSmwwiLX63503657 = -520905011;    double EXZcNhvLODSmwwiLX67390313 = -232219299;    double EXZcNhvLODSmwwiLX8093019 = -516417240;    double EXZcNhvLODSmwwiLX69045279 = -406211617;    double EXZcNhvLODSmwwiLX48241407 = -121123158;    double EXZcNhvLODSmwwiLX79401671 = -245942946;    double EXZcNhvLODSmwwiLX20523778 = -210456177;    double EXZcNhvLODSmwwiLX81072637 = -306136923;    double EXZcNhvLODSmwwiLX20276454 = 62278667;    double EXZcNhvLODSmwwiLX68803797 = -38544230;    double EXZcNhvLODSmwwiLX22350317 = -413725785;    double EXZcNhvLODSmwwiLX77068184 = -785486811;    double EXZcNhvLODSmwwiLX43012136 = -959304213;    double EXZcNhvLODSmwwiLX81406306 = 81240552;    double EXZcNhvLODSmwwiLX30916213 = 25276840;    double EXZcNhvLODSmwwiLX2091806 = -254698883;    double EXZcNhvLODSmwwiLX96164363 = -816508524;    double EXZcNhvLODSmwwiLX28718589 = -24816443;    double EXZcNhvLODSmwwiLX59743089 = -713197328;    double EXZcNhvLODSmwwiLX21006414 = -447847720;    double EXZcNhvLODSmwwiLX1617919 = -53010571;    double EXZcNhvLODSmwwiLX37689786 = -199865176;    double EXZcNhvLODSmwwiLX95876153 = -684344256;    double EXZcNhvLODSmwwiLX64040022 = -642643874;    double EXZcNhvLODSmwwiLX25911371 = -480036760;    double EXZcNhvLODSmwwiLX22838383 = -960203302;    double EXZcNhvLODSmwwiLX24824386 = -135050321;    double EXZcNhvLODSmwwiLX5579608 = -19717735;    double EXZcNhvLODSmwwiLX76572913 = 89346774;    double EXZcNhvLODSmwwiLX14292624 = -446262949;    double EXZcNhvLODSmwwiLX68300457 = -30773496;    double EXZcNhvLODSmwwiLX718747 = -853408782;    double EXZcNhvLODSmwwiLX39550215 = -904815805;    double EXZcNhvLODSmwwiLX22168038 = -578718651;    double EXZcNhvLODSmwwiLX87366628 = -985862551;    double EXZcNhvLODSmwwiLX77580493 = -313624270;    double EXZcNhvLODSmwwiLX21263460 = -528533738;    double EXZcNhvLODSmwwiLX30333241 = -512893995;    double EXZcNhvLODSmwwiLX82502522 = -604563441;    double EXZcNhvLODSmwwiLX57644425 = -875037591;    double EXZcNhvLODSmwwiLX42805426 = -136159359;    double EXZcNhvLODSmwwiLX59003071 = -202905244;    double EXZcNhvLODSmwwiLX65173571 = -249715839;    double EXZcNhvLODSmwwiLX43863193 = -947826442;    double EXZcNhvLODSmwwiLX23686644 = -909193885;    double EXZcNhvLODSmwwiLX81072058 = -47783518;    double EXZcNhvLODSmwwiLX90642343 = -191380684;    double EXZcNhvLODSmwwiLX38001747 = -22047891;    double EXZcNhvLODSmwwiLX7316482 = -297182370;    double EXZcNhvLODSmwwiLX60726240 = -611562589;    double EXZcNhvLODSmwwiLX34381763 = -877017657;    double EXZcNhvLODSmwwiLX17992779 = -275646740;    double EXZcNhvLODSmwwiLX65177125 = -635715528;    double EXZcNhvLODSmwwiLX74771776 = 24806091;    double EXZcNhvLODSmwwiLX30019918 = -824383612;    double EXZcNhvLODSmwwiLX25813871 = -221039835;    double EXZcNhvLODSmwwiLX71514159 = -547875044;    double EXZcNhvLODSmwwiLX44052996 = -873773366;    double EXZcNhvLODSmwwiLX43133908 = -926174857;    double EXZcNhvLODSmwwiLX25403025 = -160919857;    double EXZcNhvLODSmwwiLX54577285 = -10892626;    double EXZcNhvLODSmwwiLX14944170 = -90738443;    double EXZcNhvLODSmwwiLX4499724 = -295483698;    double EXZcNhvLODSmwwiLX5983831 = -491458384;    double EXZcNhvLODSmwwiLX503341 = 92229265;    double EXZcNhvLODSmwwiLX21631570 = -560317004;    double EXZcNhvLODSmwwiLX37517969 = -880671006;    double EXZcNhvLODSmwwiLX20844098 = -280585562;    double EXZcNhvLODSmwwiLX94039678 = 67103102;    double EXZcNhvLODSmwwiLX53335719 = -661098891;    double EXZcNhvLODSmwwiLX80828345 = -726165145;    double EXZcNhvLODSmwwiLX65831122 = -203614530;    double EXZcNhvLODSmwwiLX46216067 = -976914696;     EXZcNhvLODSmwwiLX39390960 = EXZcNhvLODSmwwiLX44838750;     EXZcNhvLODSmwwiLX44838750 = EXZcNhvLODSmwwiLX15011825;     EXZcNhvLODSmwwiLX15011825 = EXZcNhvLODSmwwiLX25638961;     EXZcNhvLODSmwwiLX25638961 = EXZcNhvLODSmwwiLX35328406;     EXZcNhvLODSmwwiLX35328406 = EXZcNhvLODSmwwiLX53805874;     EXZcNhvLODSmwwiLX53805874 = EXZcNhvLODSmwwiLX50870285;     EXZcNhvLODSmwwiLX50870285 = EXZcNhvLODSmwwiLX44970807;     EXZcNhvLODSmwwiLX44970807 = EXZcNhvLODSmwwiLX29356479;     EXZcNhvLODSmwwiLX29356479 = EXZcNhvLODSmwwiLX99378520;     EXZcNhvLODSmwwiLX99378520 = EXZcNhvLODSmwwiLX30743930;     EXZcNhvLODSmwwiLX30743930 = EXZcNhvLODSmwwiLX37046096;     EXZcNhvLODSmwwiLX37046096 = EXZcNhvLODSmwwiLX63329203;     EXZcNhvLODSmwwiLX63329203 = EXZcNhvLODSmwwiLX40075708;     EXZcNhvLODSmwwiLX40075708 = EXZcNhvLODSmwwiLX85450025;     EXZcNhvLODSmwwiLX85450025 = EXZcNhvLODSmwwiLX12666990;     EXZcNhvLODSmwwiLX12666990 = EXZcNhvLODSmwwiLX46036960;     EXZcNhvLODSmwwiLX46036960 = EXZcNhvLODSmwwiLX58140242;     EXZcNhvLODSmwwiLX58140242 = EXZcNhvLODSmwwiLX33654479;     EXZcNhvLODSmwwiLX33654479 = EXZcNhvLODSmwwiLX19408054;     EXZcNhvLODSmwwiLX19408054 = EXZcNhvLODSmwwiLX38232695;     EXZcNhvLODSmwwiLX38232695 = EXZcNhvLODSmwwiLX62818046;     EXZcNhvLODSmwwiLX62818046 = EXZcNhvLODSmwwiLX30546127;     EXZcNhvLODSmwwiLX30546127 = EXZcNhvLODSmwwiLX46711367;     EXZcNhvLODSmwwiLX46711367 = EXZcNhvLODSmwwiLX24920215;     EXZcNhvLODSmwwiLX24920215 = EXZcNhvLODSmwwiLX95778190;     EXZcNhvLODSmwwiLX95778190 = EXZcNhvLODSmwwiLX31637836;     EXZcNhvLODSmwwiLX31637836 = EXZcNhvLODSmwwiLX63503657;     EXZcNhvLODSmwwiLX63503657 = EXZcNhvLODSmwwiLX67390313;     EXZcNhvLODSmwwiLX67390313 = EXZcNhvLODSmwwiLX8093019;     EXZcNhvLODSmwwiLX8093019 = EXZcNhvLODSmwwiLX69045279;     EXZcNhvLODSmwwiLX69045279 = EXZcNhvLODSmwwiLX48241407;     EXZcNhvLODSmwwiLX48241407 = EXZcNhvLODSmwwiLX79401671;     EXZcNhvLODSmwwiLX79401671 = EXZcNhvLODSmwwiLX20523778;     EXZcNhvLODSmwwiLX20523778 = EXZcNhvLODSmwwiLX81072637;     EXZcNhvLODSmwwiLX81072637 = EXZcNhvLODSmwwiLX20276454;     EXZcNhvLODSmwwiLX20276454 = EXZcNhvLODSmwwiLX68803797;     EXZcNhvLODSmwwiLX68803797 = EXZcNhvLODSmwwiLX22350317;     EXZcNhvLODSmwwiLX22350317 = EXZcNhvLODSmwwiLX77068184;     EXZcNhvLODSmwwiLX77068184 = EXZcNhvLODSmwwiLX43012136;     EXZcNhvLODSmwwiLX43012136 = EXZcNhvLODSmwwiLX81406306;     EXZcNhvLODSmwwiLX81406306 = EXZcNhvLODSmwwiLX30916213;     EXZcNhvLODSmwwiLX30916213 = EXZcNhvLODSmwwiLX2091806;     EXZcNhvLODSmwwiLX2091806 = EXZcNhvLODSmwwiLX96164363;     EXZcNhvLODSmwwiLX96164363 = EXZcNhvLODSmwwiLX28718589;     EXZcNhvLODSmwwiLX28718589 = EXZcNhvLODSmwwiLX59743089;     EXZcNhvLODSmwwiLX59743089 = EXZcNhvLODSmwwiLX21006414;     EXZcNhvLODSmwwiLX21006414 = EXZcNhvLODSmwwiLX1617919;     EXZcNhvLODSmwwiLX1617919 = EXZcNhvLODSmwwiLX37689786;     EXZcNhvLODSmwwiLX37689786 = EXZcNhvLODSmwwiLX95876153;     EXZcNhvLODSmwwiLX95876153 = EXZcNhvLODSmwwiLX64040022;     EXZcNhvLODSmwwiLX64040022 = EXZcNhvLODSmwwiLX25911371;     EXZcNhvLODSmwwiLX25911371 = EXZcNhvLODSmwwiLX22838383;     EXZcNhvLODSmwwiLX22838383 = EXZcNhvLODSmwwiLX24824386;     EXZcNhvLODSmwwiLX24824386 = EXZcNhvLODSmwwiLX5579608;     EXZcNhvLODSmwwiLX5579608 = EXZcNhvLODSmwwiLX76572913;     EXZcNhvLODSmwwiLX76572913 = EXZcNhvLODSmwwiLX14292624;     EXZcNhvLODSmwwiLX14292624 = EXZcNhvLODSmwwiLX68300457;     EXZcNhvLODSmwwiLX68300457 = EXZcNhvLODSmwwiLX718747;     EXZcNhvLODSmwwiLX718747 = EXZcNhvLODSmwwiLX39550215;     EXZcNhvLODSmwwiLX39550215 = EXZcNhvLODSmwwiLX22168038;     EXZcNhvLODSmwwiLX22168038 = EXZcNhvLODSmwwiLX87366628;     EXZcNhvLODSmwwiLX87366628 = EXZcNhvLODSmwwiLX77580493;     EXZcNhvLODSmwwiLX77580493 = EXZcNhvLODSmwwiLX21263460;     EXZcNhvLODSmwwiLX21263460 = EXZcNhvLODSmwwiLX30333241;     EXZcNhvLODSmwwiLX30333241 = EXZcNhvLODSmwwiLX82502522;     EXZcNhvLODSmwwiLX82502522 = EXZcNhvLODSmwwiLX57644425;     EXZcNhvLODSmwwiLX57644425 = EXZcNhvLODSmwwiLX42805426;     EXZcNhvLODSmwwiLX42805426 = EXZcNhvLODSmwwiLX59003071;     EXZcNhvLODSmwwiLX59003071 = EXZcNhvLODSmwwiLX65173571;     EXZcNhvLODSmwwiLX65173571 = EXZcNhvLODSmwwiLX43863193;     EXZcNhvLODSmwwiLX43863193 = EXZcNhvLODSmwwiLX23686644;     EXZcNhvLODSmwwiLX23686644 = EXZcNhvLODSmwwiLX81072058;     EXZcNhvLODSmwwiLX81072058 = EXZcNhvLODSmwwiLX90642343;     EXZcNhvLODSmwwiLX90642343 = EXZcNhvLODSmwwiLX38001747;     EXZcNhvLODSmwwiLX38001747 = EXZcNhvLODSmwwiLX7316482;     EXZcNhvLODSmwwiLX7316482 = EXZcNhvLODSmwwiLX60726240;     EXZcNhvLODSmwwiLX60726240 = EXZcNhvLODSmwwiLX34381763;     EXZcNhvLODSmwwiLX34381763 = EXZcNhvLODSmwwiLX17992779;     EXZcNhvLODSmwwiLX17992779 = EXZcNhvLODSmwwiLX65177125;     EXZcNhvLODSmwwiLX65177125 = EXZcNhvLODSmwwiLX74771776;     EXZcNhvLODSmwwiLX74771776 = EXZcNhvLODSmwwiLX30019918;     EXZcNhvLODSmwwiLX30019918 = EXZcNhvLODSmwwiLX25813871;     EXZcNhvLODSmwwiLX25813871 = EXZcNhvLODSmwwiLX71514159;     EXZcNhvLODSmwwiLX71514159 = EXZcNhvLODSmwwiLX44052996;     EXZcNhvLODSmwwiLX44052996 = EXZcNhvLODSmwwiLX43133908;     EXZcNhvLODSmwwiLX43133908 = EXZcNhvLODSmwwiLX25403025;     EXZcNhvLODSmwwiLX25403025 = EXZcNhvLODSmwwiLX54577285;     EXZcNhvLODSmwwiLX54577285 = EXZcNhvLODSmwwiLX14944170;     EXZcNhvLODSmwwiLX14944170 = EXZcNhvLODSmwwiLX4499724;     EXZcNhvLODSmwwiLX4499724 = EXZcNhvLODSmwwiLX5983831;     EXZcNhvLODSmwwiLX5983831 = EXZcNhvLODSmwwiLX503341;     EXZcNhvLODSmwwiLX503341 = EXZcNhvLODSmwwiLX21631570;     EXZcNhvLODSmwwiLX21631570 = EXZcNhvLODSmwwiLX37517969;     EXZcNhvLODSmwwiLX37517969 = EXZcNhvLODSmwwiLX20844098;     EXZcNhvLODSmwwiLX20844098 = EXZcNhvLODSmwwiLX94039678;     EXZcNhvLODSmwwiLX94039678 = EXZcNhvLODSmwwiLX53335719;     EXZcNhvLODSmwwiLX53335719 = EXZcNhvLODSmwwiLX80828345;     EXZcNhvLODSmwwiLX80828345 = EXZcNhvLODSmwwiLX65831122;     EXZcNhvLODSmwwiLX65831122 = EXZcNhvLODSmwwiLX46216067;     EXZcNhvLODSmwwiLX46216067 = EXZcNhvLODSmwwiLX39390960;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void gLobNmLdrYchHujr89177797() {     double CPxvMAUiimGIZMeEi13240787 = -285692964;    double CPxvMAUiimGIZMeEi25561911 = -318367151;    double CPxvMAUiimGIZMeEi99090640 = -536338830;    double CPxvMAUiimGIZMeEi96285489 = -393375388;    double CPxvMAUiimGIZMeEi27946478 = -767129066;    double CPxvMAUiimGIZMeEi45931796 = -552066610;    double CPxvMAUiimGIZMeEi82414286 = -165827172;    double CPxvMAUiimGIZMeEi41189453 = -902745893;    double CPxvMAUiimGIZMeEi3387695 = -750326312;    double CPxvMAUiimGIZMeEi49739779 = -812021152;    double CPxvMAUiimGIZMeEi49784785 = -74392013;    double CPxvMAUiimGIZMeEi62007374 = -629707943;    double CPxvMAUiimGIZMeEi52729187 = -601683639;    double CPxvMAUiimGIZMeEi72867706 = -956885913;    double CPxvMAUiimGIZMeEi87703655 = -698255419;    double CPxvMAUiimGIZMeEi8660761 = -710878194;    double CPxvMAUiimGIZMeEi89864551 = -743144774;    double CPxvMAUiimGIZMeEi93573224 = -986971194;    double CPxvMAUiimGIZMeEi35545351 = -405744570;    double CPxvMAUiimGIZMeEi60839946 = -304978565;    double CPxvMAUiimGIZMeEi93494607 = -750797142;    double CPxvMAUiimGIZMeEi61901699 = -112513392;    double CPxvMAUiimGIZMeEi32960410 = -29380839;    double CPxvMAUiimGIZMeEi55775160 = -387720212;    double CPxvMAUiimGIZMeEi8407496 = 29245439;    double CPxvMAUiimGIZMeEi67561913 = -841882391;    double CPxvMAUiimGIZMeEi75197110 = -80798600;    double CPxvMAUiimGIZMeEi29273232 = -367236558;    double CPxvMAUiimGIZMeEi517453 = -418614247;    double CPxvMAUiimGIZMeEi91430425 = 36573994;    double CPxvMAUiimGIZMeEi53873105 = -951976181;    double CPxvMAUiimGIZMeEi72401609 = -71144800;    double CPxvMAUiimGIZMeEi2495414 = -111382197;    double CPxvMAUiimGIZMeEi80087136 = -606003983;    double CPxvMAUiimGIZMeEi84272497 = -790763192;    double CPxvMAUiimGIZMeEi43368695 = -219147220;    double CPxvMAUiimGIZMeEi68800956 = -974701431;    double CPxvMAUiimGIZMeEi97089390 = -178928246;    double CPxvMAUiimGIZMeEi48216920 = -798507738;    double CPxvMAUiimGIZMeEi26713393 = -810658865;    double CPxvMAUiimGIZMeEi59873500 = -374660850;    double CPxvMAUiimGIZMeEi14900403 = -495242042;    double CPxvMAUiimGIZMeEi94708793 = -382528732;    double CPxvMAUiimGIZMeEi74021823 = -259535446;    double CPxvMAUiimGIZMeEi48670257 = -463340477;    double CPxvMAUiimGIZMeEi24448964 = -811852925;    double CPxvMAUiimGIZMeEi34608674 = -17976127;    double CPxvMAUiimGIZMeEi30746225 = -941788723;    double CPxvMAUiimGIZMeEi99482653 = -335588674;    double CPxvMAUiimGIZMeEi61029889 = -673356029;    double CPxvMAUiimGIZMeEi33496378 = -82798320;    double CPxvMAUiimGIZMeEi21190198 = -196516379;    double CPxvMAUiimGIZMeEi17673240 = -683988662;    double CPxvMAUiimGIZMeEi23664054 = -835314790;    double CPxvMAUiimGIZMeEi95683190 = -465044951;    double CPxvMAUiimGIZMeEi51339087 = -73179572;    double CPxvMAUiimGIZMeEi92601500 = -188986313;    double CPxvMAUiimGIZMeEi43315481 = -48618619;    double CPxvMAUiimGIZMeEi87877994 = -322620827;    double CPxvMAUiimGIZMeEi60384565 = -925246676;    double CPxvMAUiimGIZMeEi70734686 = -371268011;    double CPxvMAUiimGIZMeEi53141054 = -798590615;    double CPxvMAUiimGIZMeEi40672000 = -384131646;    double CPxvMAUiimGIZMeEi11957269 = -686900306;    double CPxvMAUiimGIZMeEi95866673 = -860044972;    double CPxvMAUiimGIZMeEi77383175 = 96752786;    double CPxvMAUiimGIZMeEi59511960 = -418325746;    double CPxvMAUiimGIZMeEi72642051 = -995679657;    double CPxvMAUiimGIZMeEi88595209 = -66122722;    double CPxvMAUiimGIZMeEi44334961 = -379108199;    double CPxvMAUiimGIZMeEi39859804 = -736176763;    double CPxvMAUiimGIZMeEi92775160 = -464216528;    double CPxvMAUiimGIZMeEi45356305 = -88463457;    double CPxvMAUiimGIZMeEi8831958 = -595085706;    double CPxvMAUiimGIZMeEi966446 = -930317716;    double CPxvMAUiimGIZMeEi78594204 = -155555101;    double CPxvMAUiimGIZMeEi67192906 = -729984661;    double CPxvMAUiimGIZMeEi58938586 = -769845394;    double CPxvMAUiimGIZMeEi7104903 = -924379735;    double CPxvMAUiimGIZMeEi83958531 = -158901636;    double CPxvMAUiimGIZMeEi32953239 = -723906264;    double CPxvMAUiimGIZMeEi44450885 = -139009878;    double CPxvMAUiimGIZMeEi29790579 = 68352116;    double CPxvMAUiimGIZMeEi39487563 = -745258219;    double CPxvMAUiimGIZMeEi57934048 = -880627686;    double CPxvMAUiimGIZMeEi32682907 = -655459802;    double CPxvMAUiimGIZMeEi54728370 = -387156138;    double CPxvMAUiimGIZMeEi78831359 = -276067408;    double CPxvMAUiimGIZMeEi84403945 = -40959032;    double CPxvMAUiimGIZMeEi32933410 = -617583620;    double CPxvMAUiimGIZMeEi50767194 = 69839092;    double CPxvMAUiimGIZMeEi25485476 = -826082812;    double CPxvMAUiimGIZMeEi9211397 = -856307419;    double CPxvMAUiimGIZMeEi87832355 = -873261062;    double CPxvMAUiimGIZMeEi55978707 = -339390855;    double CPxvMAUiimGIZMeEi6732447 = -576070236;    double CPxvMAUiimGIZMeEi74228402 = -11110396;    double CPxvMAUiimGIZMeEi82751524 = -695628426;    double CPxvMAUiimGIZMeEi78155149 = -399490474;    double CPxvMAUiimGIZMeEi71287082 = -285692964;     CPxvMAUiimGIZMeEi13240787 = CPxvMAUiimGIZMeEi25561911;     CPxvMAUiimGIZMeEi25561911 = CPxvMAUiimGIZMeEi99090640;     CPxvMAUiimGIZMeEi99090640 = CPxvMAUiimGIZMeEi96285489;     CPxvMAUiimGIZMeEi96285489 = CPxvMAUiimGIZMeEi27946478;     CPxvMAUiimGIZMeEi27946478 = CPxvMAUiimGIZMeEi45931796;     CPxvMAUiimGIZMeEi45931796 = CPxvMAUiimGIZMeEi82414286;     CPxvMAUiimGIZMeEi82414286 = CPxvMAUiimGIZMeEi41189453;     CPxvMAUiimGIZMeEi41189453 = CPxvMAUiimGIZMeEi3387695;     CPxvMAUiimGIZMeEi3387695 = CPxvMAUiimGIZMeEi49739779;     CPxvMAUiimGIZMeEi49739779 = CPxvMAUiimGIZMeEi49784785;     CPxvMAUiimGIZMeEi49784785 = CPxvMAUiimGIZMeEi62007374;     CPxvMAUiimGIZMeEi62007374 = CPxvMAUiimGIZMeEi52729187;     CPxvMAUiimGIZMeEi52729187 = CPxvMAUiimGIZMeEi72867706;     CPxvMAUiimGIZMeEi72867706 = CPxvMAUiimGIZMeEi87703655;     CPxvMAUiimGIZMeEi87703655 = CPxvMAUiimGIZMeEi8660761;     CPxvMAUiimGIZMeEi8660761 = CPxvMAUiimGIZMeEi89864551;     CPxvMAUiimGIZMeEi89864551 = CPxvMAUiimGIZMeEi93573224;     CPxvMAUiimGIZMeEi93573224 = CPxvMAUiimGIZMeEi35545351;     CPxvMAUiimGIZMeEi35545351 = CPxvMAUiimGIZMeEi60839946;     CPxvMAUiimGIZMeEi60839946 = CPxvMAUiimGIZMeEi93494607;     CPxvMAUiimGIZMeEi93494607 = CPxvMAUiimGIZMeEi61901699;     CPxvMAUiimGIZMeEi61901699 = CPxvMAUiimGIZMeEi32960410;     CPxvMAUiimGIZMeEi32960410 = CPxvMAUiimGIZMeEi55775160;     CPxvMAUiimGIZMeEi55775160 = CPxvMAUiimGIZMeEi8407496;     CPxvMAUiimGIZMeEi8407496 = CPxvMAUiimGIZMeEi67561913;     CPxvMAUiimGIZMeEi67561913 = CPxvMAUiimGIZMeEi75197110;     CPxvMAUiimGIZMeEi75197110 = CPxvMAUiimGIZMeEi29273232;     CPxvMAUiimGIZMeEi29273232 = CPxvMAUiimGIZMeEi517453;     CPxvMAUiimGIZMeEi517453 = CPxvMAUiimGIZMeEi91430425;     CPxvMAUiimGIZMeEi91430425 = CPxvMAUiimGIZMeEi53873105;     CPxvMAUiimGIZMeEi53873105 = CPxvMAUiimGIZMeEi72401609;     CPxvMAUiimGIZMeEi72401609 = CPxvMAUiimGIZMeEi2495414;     CPxvMAUiimGIZMeEi2495414 = CPxvMAUiimGIZMeEi80087136;     CPxvMAUiimGIZMeEi80087136 = CPxvMAUiimGIZMeEi84272497;     CPxvMAUiimGIZMeEi84272497 = CPxvMAUiimGIZMeEi43368695;     CPxvMAUiimGIZMeEi43368695 = CPxvMAUiimGIZMeEi68800956;     CPxvMAUiimGIZMeEi68800956 = CPxvMAUiimGIZMeEi97089390;     CPxvMAUiimGIZMeEi97089390 = CPxvMAUiimGIZMeEi48216920;     CPxvMAUiimGIZMeEi48216920 = CPxvMAUiimGIZMeEi26713393;     CPxvMAUiimGIZMeEi26713393 = CPxvMAUiimGIZMeEi59873500;     CPxvMAUiimGIZMeEi59873500 = CPxvMAUiimGIZMeEi14900403;     CPxvMAUiimGIZMeEi14900403 = CPxvMAUiimGIZMeEi94708793;     CPxvMAUiimGIZMeEi94708793 = CPxvMAUiimGIZMeEi74021823;     CPxvMAUiimGIZMeEi74021823 = CPxvMAUiimGIZMeEi48670257;     CPxvMAUiimGIZMeEi48670257 = CPxvMAUiimGIZMeEi24448964;     CPxvMAUiimGIZMeEi24448964 = CPxvMAUiimGIZMeEi34608674;     CPxvMAUiimGIZMeEi34608674 = CPxvMAUiimGIZMeEi30746225;     CPxvMAUiimGIZMeEi30746225 = CPxvMAUiimGIZMeEi99482653;     CPxvMAUiimGIZMeEi99482653 = CPxvMAUiimGIZMeEi61029889;     CPxvMAUiimGIZMeEi61029889 = CPxvMAUiimGIZMeEi33496378;     CPxvMAUiimGIZMeEi33496378 = CPxvMAUiimGIZMeEi21190198;     CPxvMAUiimGIZMeEi21190198 = CPxvMAUiimGIZMeEi17673240;     CPxvMAUiimGIZMeEi17673240 = CPxvMAUiimGIZMeEi23664054;     CPxvMAUiimGIZMeEi23664054 = CPxvMAUiimGIZMeEi95683190;     CPxvMAUiimGIZMeEi95683190 = CPxvMAUiimGIZMeEi51339087;     CPxvMAUiimGIZMeEi51339087 = CPxvMAUiimGIZMeEi92601500;     CPxvMAUiimGIZMeEi92601500 = CPxvMAUiimGIZMeEi43315481;     CPxvMAUiimGIZMeEi43315481 = CPxvMAUiimGIZMeEi87877994;     CPxvMAUiimGIZMeEi87877994 = CPxvMAUiimGIZMeEi60384565;     CPxvMAUiimGIZMeEi60384565 = CPxvMAUiimGIZMeEi70734686;     CPxvMAUiimGIZMeEi70734686 = CPxvMAUiimGIZMeEi53141054;     CPxvMAUiimGIZMeEi53141054 = CPxvMAUiimGIZMeEi40672000;     CPxvMAUiimGIZMeEi40672000 = CPxvMAUiimGIZMeEi11957269;     CPxvMAUiimGIZMeEi11957269 = CPxvMAUiimGIZMeEi95866673;     CPxvMAUiimGIZMeEi95866673 = CPxvMAUiimGIZMeEi77383175;     CPxvMAUiimGIZMeEi77383175 = CPxvMAUiimGIZMeEi59511960;     CPxvMAUiimGIZMeEi59511960 = CPxvMAUiimGIZMeEi72642051;     CPxvMAUiimGIZMeEi72642051 = CPxvMAUiimGIZMeEi88595209;     CPxvMAUiimGIZMeEi88595209 = CPxvMAUiimGIZMeEi44334961;     CPxvMAUiimGIZMeEi44334961 = CPxvMAUiimGIZMeEi39859804;     CPxvMAUiimGIZMeEi39859804 = CPxvMAUiimGIZMeEi92775160;     CPxvMAUiimGIZMeEi92775160 = CPxvMAUiimGIZMeEi45356305;     CPxvMAUiimGIZMeEi45356305 = CPxvMAUiimGIZMeEi8831958;     CPxvMAUiimGIZMeEi8831958 = CPxvMAUiimGIZMeEi966446;     CPxvMAUiimGIZMeEi966446 = CPxvMAUiimGIZMeEi78594204;     CPxvMAUiimGIZMeEi78594204 = CPxvMAUiimGIZMeEi67192906;     CPxvMAUiimGIZMeEi67192906 = CPxvMAUiimGIZMeEi58938586;     CPxvMAUiimGIZMeEi58938586 = CPxvMAUiimGIZMeEi7104903;     CPxvMAUiimGIZMeEi7104903 = CPxvMAUiimGIZMeEi83958531;     CPxvMAUiimGIZMeEi83958531 = CPxvMAUiimGIZMeEi32953239;     CPxvMAUiimGIZMeEi32953239 = CPxvMAUiimGIZMeEi44450885;     CPxvMAUiimGIZMeEi44450885 = CPxvMAUiimGIZMeEi29790579;     CPxvMAUiimGIZMeEi29790579 = CPxvMAUiimGIZMeEi39487563;     CPxvMAUiimGIZMeEi39487563 = CPxvMAUiimGIZMeEi57934048;     CPxvMAUiimGIZMeEi57934048 = CPxvMAUiimGIZMeEi32682907;     CPxvMAUiimGIZMeEi32682907 = CPxvMAUiimGIZMeEi54728370;     CPxvMAUiimGIZMeEi54728370 = CPxvMAUiimGIZMeEi78831359;     CPxvMAUiimGIZMeEi78831359 = CPxvMAUiimGIZMeEi84403945;     CPxvMAUiimGIZMeEi84403945 = CPxvMAUiimGIZMeEi32933410;     CPxvMAUiimGIZMeEi32933410 = CPxvMAUiimGIZMeEi50767194;     CPxvMAUiimGIZMeEi50767194 = CPxvMAUiimGIZMeEi25485476;     CPxvMAUiimGIZMeEi25485476 = CPxvMAUiimGIZMeEi9211397;     CPxvMAUiimGIZMeEi9211397 = CPxvMAUiimGIZMeEi87832355;     CPxvMAUiimGIZMeEi87832355 = CPxvMAUiimGIZMeEi55978707;     CPxvMAUiimGIZMeEi55978707 = CPxvMAUiimGIZMeEi6732447;     CPxvMAUiimGIZMeEi6732447 = CPxvMAUiimGIZMeEi74228402;     CPxvMAUiimGIZMeEi74228402 = CPxvMAUiimGIZMeEi82751524;     CPxvMAUiimGIZMeEi82751524 = CPxvMAUiimGIZMeEi78155149;     CPxvMAUiimGIZMeEi78155149 = CPxvMAUiimGIZMeEi71287082;     CPxvMAUiimGIZMeEi71287082 = CPxvMAUiimGIZMeEi13240787;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void kcBFUIoWbecxysah56469397() {     double oKtmPMnZgMxItsEIz48699755 = -751969864;    double oKtmPMnZgMxItsEIz38833989 = -198918758;    double oKtmPMnZgMxItsEIz36477117 = -202005905;    double oKtmPMnZgMxItsEIz68508844 = -214526459;    double oKtmPMnZgMxItsEIz84231747 = -872727912;    double oKtmPMnZgMxItsEIz82154668 = -814414506;    double oKtmPMnZgMxItsEIz93071692 = -10552907;    double oKtmPMnZgMxItsEIz3321014 = -546980033;    double oKtmPMnZgMxItsEIz4905300 = -138657718;    double oKtmPMnZgMxItsEIz64642491 = -476708150;    double oKtmPMnZgMxItsEIz87966519 = -976399743;    double oKtmPMnZgMxItsEIz62575522 = -493828715;    double oKtmPMnZgMxItsEIz90963833 = -53696144;    double oKtmPMnZgMxItsEIz26008569 = -933249322;    double oKtmPMnZgMxItsEIz45898172 = -101680434;    double oKtmPMnZgMxItsEIz92202061 = -346689801;    double oKtmPMnZgMxItsEIz45700487 = -170119326;    double oKtmPMnZgMxItsEIz93333416 = 39600737;    double oKtmPMnZgMxItsEIz36095681 = -722307992;    double oKtmPMnZgMxItsEIz54677164 = -731539631;    double oKtmPMnZgMxItsEIz6581470 = -408071831;    double oKtmPMnZgMxItsEIz65323224 = -199149686;    double oKtmPMnZgMxItsEIz3485790 = -767013023;    double oKtmPMnZgMxItsEIz49568105 = -898544816;    double oKtmPMnZgMxItsEIz16425447 = 5260241;    double oKtmPMnZgMxItsEIz78260084 = -530459197;    double oKtmPMnZgMxItsEIz35617684 = -987082741;    double oKtmPMnZgMxItsEIz70001657 = -537008732;    double oKtmPMnZgMxItsEIz47185667 = -992709050;    double oKtmPMnZgMxItsEIz96963836 = 2066824;    double oKtmPMnZgMxItsEIz72456337 = -403956668;    double oKtmPMnZgMxItsEIz10224628 = -944116214;    double oKtmPMnZgMxItsEIz14713425 = -258978174;    double oKtmPMnZgMxItsEIz9835563 = -670697796;    double oKtmPMnZgMxItsEIz50080951 = -741863442;    double oKtmPMnZgMxItsEIz94929538 = -433712526;    double oKtmPMnZgMxItsEIz18212773 = -839397471;    double oKtmPMnZgMxItsEIz48304794 = -161583827;    double oKtmPMnZgMxItsEIz78607481 = -449384015;    double oKtmPMnZgMxItsEIz90984925 = -275921411;    double oKtmPMnZgMxItsEIz93325107 = -143859764;    double oKtmPMnZgMxItsEIz4265530 = -89787333;    double oKtmPMnZgMxItsEIz84092171 = 64290482;    double oKtmPMnZgMxItsEIz14153162 = -863867856;    double oKtmPMnZgMxItsEIz48686716 = -289851639;    double oKtmPMnZgMxItsEIz70410292 = -850776557;    double oKtmPMnZgMxItsEIz53953838 = -501962487;    double oKtmPMnZgMxItsEIz48472536 = -8315316;    double oKtmPMnZgMxItsEIz22375320 = 674446;    double oKtmPMnZgMxItsEIz85559478 = -64382844;    double oKtmPMnZgMxItsEIz14267102 = 42925122;    double oKtmPMnZgMxItsEIz4675671 = -410168726;    double oKtmPMnZgMxItsEIz86294388 = -440034216;    double oKtmPMnZgMxItsEIz39655900 = 73176710;    double oKtmPMnZgMxItsEIz49887927 = -811535097;    double oKtmPMnZgMxItsEIz83376530 = -452820178;    double oKtmPMnZgMxItsEIz35348199 = -431905735;    double oKtmPMnZgMxItsEIz86909012 = -303461090;    double oKtmPMnZgMxItsEIz52083398 = -119786700;    double oKtmPMnZgMxItsEIz5971663 = -242268715;    double oKtmPMnZgMxItsEIz46536984 = -827331766;    double oKtmPMnZgMxItsEIz23070036 = -473544175;    double oKtmPMnZgMxItsEIz56135347 = -554270984;    double oKtmPMnZgMxItsEIz7941463 = -40724543;    double oKtmPMnZgMxItsEIz92186154 = 27248517;    double oKtmPMnZgMxItsEIz77741891 = 67716471;    double oKtmPMnZgMxItsEIz47862098 = -134850541;    double oKtmPMnZgMxItsEIz81128270 = -382998349;    double oKtmPMnZgMxItsEIz75927618 = -91385881;    double oKtmPMnZgMxItsEIz50968634 = -667967908;    double oKtmPMnZgMxItsEIz73989288 = -507292330;    double oKtmPMnZgMxItsEIz97395692 = 91464500;    double oKtmPMnZgMxItsEIz14725935 = -511015249;    double oKtmPMnZgMxItsEIz45110755 = -346386581;    double oKtmPMnZgMxItsEIz61352056 = -487679867;    double oKtmPMnZgMxItsEIz2315941 = -218284498;    double oKtmPMnZgMxItsEIz81231052 = -163440169;    double oKtmPMnZgMxItsEIz89332627 = -903145168;    double oKtmPMnZgMxItsEIz881389 = -508693178;    double oKtmPMnZgMxItsEIz46015154 = -143963203;    double oKtmPMnZgMxItsEIz24306247 = 71503290;    double oKtmPMnZgMxItsEIz87145147 = -878767425;    double oKtmPMnZgMxItsEIz47626338 = -437683179;    double oKtmPMnZgMxItsEIz61626189 = -828326206;    double oKtmPMnZgMxItsEIz82696735 = 59141701;    double oKtmPMnZgMxItsEIz67780666 = -993787943;    double oKtmPMnZgMxItsEIz23930240 = -404081998;    double oKtmPMnZgMxItsEIz75057524 = -232154885;    double oKtmPMnZgMxItsEIz59947636 = -859162699;    double oKtmPMnZgMxItsEIz66704420 = -189043265;    double oKtmPMnZgMxItsEIz59581339 = 98193209;    double oKtmPMnZgMxItsEIz31303761 = -435936382;    double oKtmPMnZgMxItsEIz96221396 = 58202873;    double oKtmPMnZgMxItsEIz72635819 = -107115300;    double oKtmPMnZgMxItsEIz44447942 = -448589646;    double oKtmPMnZgMxItsEIz70255072 = -670315590;    double oKtmPMnZgMxItsEIz48130183 = -535516349;    double oKtmPMnZgMxItsEIz76150708 = -894984976;    double oKtmPMnZgMxItsEIz21967008 = -791116373;    double oKtmPMnZgMxItsEIz70944825 = -751969864;     oKtmPMnZgMxItsEIz48699755 = oKtmPMnZgMxItsEIz38833989;     oKtmPMnZgMxItsEIz38833989 = oKtmPMnZgMxItsEIz36477117;     oKtmPMnZgMxItsEIz36477117 = oKtmPMnZgMxItsEIz68508844;     oKtmPMnZgMxItsEIz68508844 = oKtmPMnZgMxItsEIz84231747;     oKtmPMnZgMxItsEIz84231747 = oKtmPMnZgMxItsEIz82154668;     oKtmPMnZgMxItsEIz82154668 = oKtmPMnZgMxItsEIz93071692;     oKtmPMnZgMxItsEIz93071692 = oKtmPMnZgMxItsEIz3321014;     oKtmPMnZgMxItsEIz3321014 = oKtmPMnZgMxItsEIz4905300;     oKtmPMnZgMxItsEIz4905300 = oKtmPMnZgMxItsEIz64642491;     oKtmPMnZgMxItsEIz64642491 = oKtmPMnZgMxItsEIz87966519;     oKtmPMnZgMxItsEIz87966519 = oKtmPMnZgMxItsEIz62575522;     oKtmPMnZgMxItsEIz62575522 = oKtmPMnZgMxItsEIz90963833;     oKtmPMnZgMxItsEIz90963833 = oKtmPMnZgMxItsEIz26008569;     oKtmPMnZgMxItsEIz26008569 = oKtmPMnZgMxItsEIz45898172;     oKtmPMnZgMxItsEIz45898172 = oKtmPMnZgMxItsEIz92202061;     oKtmPMnZgMxItsEIz92202061 = oKtmPMnZgMxItsEIz45700487;     oKtmPMnZgMxItsEIz45700487 = oKtmPMnZgMxItsEIz93333416;     oKtmPMnZgMxItsEIz93333416 = oKtmPMnZgMxItsEIz36095681;     oKtmPMnZgMxItsEIz36095681 = oKtmPMnZgMxItsEIz54677164;     oKtmPMnZgMxItsEIz54677164 = oKtmPMnZgMxItsEIz6581470;     oKtmPMnZgMxItsEIz6581470 = oKtmPMnZgMxItsEIz65323224;     oKtmPMnZgMxItsEIz65323224 = oKtmPMnZgMxItsEIz3485790;     oKtmPMnZgMxItsEIz3485790 = oKtmPMnZgMxItsEIz49568105;     oKtmPMnZgMxItsEIz49568105 = oKtmPMnZgMxItsEIz16425447;     oKtmPMnZgMxItsEIz16425447 = oKtmPMnZgMxItsEIz78260084;     oKtmPMnZgMxItsEIz78260084 = oKtmPMnZgMxItsEIz35617684;     oKtmPMnZgMxItsEIz35617684 = oKtmPMnZgMxItsEIz70001657;     oKtmPMnZgMxItsEIz70001657 = oKtmPMnZgMxItsEIz47185667;     oKtmPMnZgMxItsEIz47185667 = oKtmPMnZgMxItsEIz96963836;     oKtmPMnZgMxItsEIz96963836 = oKtmPMnZgMxItsEIz72456337;     oKtmPMnZgMxItsEIz72456337 = oKtmPMnZgMxItsEIz10224628;     oKtmPMnZgMxItsEIz10224628 = oKtmPMnZgMxItsEIz14713425;     oKtmPMnZgMxItsEIz14713425 = oKtmPMnZgMxItsEIz9835563;     oKtmPMnZgMxItsEIz9835563 = oKtmPMnZgMxItsEIz50080951;     oKtmPMnZgMxItsEIz50080951 = oKtmPMnZgMxItsEIz94929538;     oKtmPMnZgMxItsEIz94929538 = oKtmPMnZgMxItsEIz18212773;     oKtmPMnZgMxItsEIz18212773 = oKtmPMnZgMxItsEIz48304794;     oKtmPMnZgMxItsEIz48304794 = oKtmPMnZgMxItsEIz78607481;     oKtmPMnZgMxItsEIz78607481 = oKtmPMnZgMxItsEIz90984925;     oKtmPMnZgMxItsEIz90984925 = oKtmPMnZgMxItsEIz93325107;     oKtmPMnZgMxItsEIz93325107 = oKtmPMnZgMxItsEIz4265530;     oKtmPMnZgMxItsEIz4265530 = oKtmPMnZgMxItsEIz84092171;     oKtmPMnZgMxItsEIz84092171 = oKtmPMnZgMxItsEIz14153162;     oKtmPMnZgMxItsEIz14153162 = oKtmPMnZgMxItsEIz48686716;     oKtmPMnZgMxItsEIz48686716 = oKtmPMnZgMxItsEIz70410292;     oKtmPMnZgMxItsEIz70410292 = oKtmPMnZgMxItsEIz53953838;     oKtmPMnZgMxItsEIz53953838 = oKtmPMnZgMxItsEIz48472536;     oKtmPMnZgMxItsEIz48472536 = oKtmPMnZgMxItsEIz22375320;     oKtmPMnZgMxItsEIz22375320 = oKtmPMnZgMxItsEIz85559478;     oKtmPMnZgMxItsEIz85559478 = oKtmPMnZgMxItsEIz14267102;     oKtmPMnZgMxItsEIz14267102 = oKtmPMnZgMxItsEIz4675671;     oKtmPMnZgMxItsEIz4675671 = oKtmPMnZgMxItsEIz86294388;     oKtmPMnZgMxItsEIz86294388 = oKtmPMnZgMxItsEIz39655900;     oKtmPMnZgMxItsEIz39655900 = oKtmPMnZgMxItsEIz49887927;     oKtmPMnZgMxItsEIz49887927 = oKtmPMnZgMxItsEIz83376530;     oKtmPMnZgMxItsEIz83376530 = oKtmPMnZgMxItsEIz35348199;     oKtmPMnZgMxItsEIz35348199 = oKtmPMnZgMxItsEIz86909012;     oKtmPMnZgMxItsEIz86909012 = oKtmPMnZgMxItsEIz52083398;     oKtmPMnZgMxItsEIz52083398 = oKtmPMnZgMxItsEIz5971663;     oKtmPMnZgMxItsEIz5971663 = oKtmPMnZgMxItsEIz46536984;     oKtmPMnZgMxItsEIz46536984 = oKtmPMnZgMxItsEIz23070036;     oKtmPMnZgMxItsEIz23070036 = oKtmPMnZgMxItsEIz56135347;     oKtmPMnZgMxItsEIz56135347 = oKtmPMnZgMxItsEIz7941463;     oKtmPMnZgMxItsEIz7941463 = oKtmPMnZgMxItsEIz92186154;     oKtmPMnZgMxItsEIz92186154 = oKtmPMnZgMxItsEIz77741891;     oKtmPMnZgMxItsEIz77741891 = oKtmPMnZgMxItsEIz47862098;     oKtmPMnZgMxItsEIz47862098 = oKtmPMnZgMxItsEIz81128270;     oKtmPMnZgMxItsEIz81128270 = oKtmPMnZgMxItsEIz75927618;     oKtmPMnZgMxItsEIz75927618 = oKtmPMnZgMxItsEIz50968634;     oKtmPMnZgMxItsEIz50968634 = oKtmPMnZgMxItsEIz73989288;     oKtmPMnZgMxItsEIz73989288 = oKtmPMnZgMxItsEIz97395692;     oKtmPMnZgMxItsEIz97395692 = oKtmPMnZgMxItsEIz14725935;     oKtmPMnZgMxItsEIz14725935 = oKtmPMnZgMxItsEIz45110755;     oKtmPMnZgMxItsEIz45110755 = oKtmPMnZgMxItsEIz61352056;     oKtmPMnZgMxItsEIz61352056 = oKtmPMnZgMxItsEIz2315941;     oKtmPMnZgMxItsEIz2315941 = oKtmPMnZgMxItsEIz81231052;     oKtmPMnZgMxItsEIz81231052 = oKtmPMnZgMxItsEIz89332627;     oKtmPMnZgMxItsEIz89332627 = oKtmPMnZgMxItsEIz881389;     oKtmPMnZgMxItsEIz881389 = oKtmPMnZgMxItsEIz46015154;     oKtmPMnZgMxItsEIz46015154 = oKtmPMnZgMxItsEIz24306247;     oKtmPMnZgMxItsEIz24306247 = oKtmPMnZgMxItsEIz87145147;     oKtmPMnZgMxItsEIz87145147 = oKtmPMnZgMxItsEIz47626338;     oKtmPMnZgMxItsEIz47626338 = oKtmPMnZgMxItsEIz61626189;     oKtmPMnZgMxItsEIz61626189 = oKtmPMnZgMxItsEIz82696735;     oKtmPMnZgMxItsEIz82696735 = oKtmPMnZgMxItsEIz67780666;     oKtmPMnZgMxItsEIz67780666 = oKtmPMnZgMxItsEIz23930240;     oKtmPMnZgMxItsEIz23930240 = oKtmPMnZgMxItsEIz75057524;     oKtmPMnZgMxItsEIz75057524 = oKtmPMnZgMxItsEIz59947636;     oKtmPMnZgMxItsEIz59947636 = oKtmPMnZgMxItsEIz66704420;     oKtmPMnZgMxItsEIz66704420 = oKtmPMnZgMxItsEIz59581339;     oKtmPMnZgMxItsEIz59581339 = oKtmPMnZgMxItsEIz31303761;     oKtmPMnZgMxItsEIz31303761 = oKtmPMnZgMxItsEIz96221396;     oKtmPMnZgMxItsEIz96221396 = oKtmPMnZgMxItsEIz72635819;     oKtmPMnZgMxItsEIz72635819 = oKtmPMnZgMxItsEIz44447942;     oKtmPMnZgMxItsEIz44447942 = oKtmPMnZgMxItsEIz70255072;     oKtmPMnZgMxItsEIz70255072 = oKtmPMnZgMxItsEIz48130183;     oKtmPMnZgMxItsEIz48130183 = oKtmPMnZgMxItsEIz76150708;     oKtmPMnZgMxItsEIz76150708 = oKtmPMnZgMxItsEIz21967008;     oKtmPMnZgMxItsEIz21967008 = oKtmPMnZgMxItsEIz70944825;     oKtmPMnZgMxItsEIz70944825 = oKtmPMnZgMxItsEIz48699755;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void jhbxrvoVkqpNZate71518464() {     double bUeruSgGetzXctfTG54816861 = -863871753;    double bUeruSgGetzXctfTG82207268 = 91481461;    double bUeruSgGetzXctfTG52519301 = -352838827;    double bUeruSgGetzXctfTG95583624 = -405275089;    double bUeruSgGetzXctfTG53127235 = -293377783;    double bUeruSgGetzXctfTG73571870 = 21089259;    double bUeruSgGetzXctfTG53560658 = -149517884;    double bUeruSgGetzXctfTG53080904 = -833853946;    double bUeruSgGetzXctfTG69547795 = 93309616;    double bUeruSgGetzXctfTG5872151 = -805683628;    double bUeruSgGetzXctfTG48076810 = -853837329;    double bUeruSgGetzXctfTG94398564 = -938234916;    double bUeruSgGetzXctfTG77172951 = -493587233;    double bUeruSgGetzXctfTG9152246 = -365487890;    double bUeruSgGetzXctfTG41640599 = -792142600;    double bUeruSgGetzXctfTG33707455 = -319689344;    double bUeruSgGetzXctfTG39536490 = -786664439;    double bUeruSgGetzXctfTG90386569 = -725921906;    double bUeruSgGetzXctfTG97925522 = -28077089;    double bUeruSgGetzXctfTG35227329 = 11557449;    double bUeruSgGetzXctfTG95134663 = -263559287;    double bUeruSgGetzXctfTG51844248 = -107079403;    double bUeruSgGetzXctfTG39076581 = -513530510;    double bUeruSgGetzXctfTG33560479 = -20849804;    double bUeruSgGetzXctfTG60125096 = -367935792;    double bUeruSgGetzXctfTG2981761 = -409114521;    double bUeruSgGetzXctfTG34077462 = -887744944;    double bUeruSgGetzXctfTG83108782 = -602937686;    double bUeruSgGetzXctfTG13317923 = -115091754;    double bUeruSgGetzXctfTG65579473 = -915404044;    double bUeruSgGetzXctfTG64698886 = -317277606;    double bUeruSgGetzXctfTG7835250 = -653563030;    double bUeruSgGetzXctfTG24424373 = -151591545;    double bUeruSgGetzXctfTG4666127 = -422239561;    double bUeruSgGetzXctfTG11555603 = -280596548;    double bUeruSgGetzXctfTG63655451 = -647738633;    double bUeruSgGetzXctfTG99474707 = -793033341;    double bUeruSgGetzXctfTG82395006 = 66910164;    double bUeruSgGetzXctfTG52217735 = -443307511;    double bUeruSgGetzXctfTG78486863 = -749360634;    double bUeruSgGetzXctfTG98994331 = -209133659;    double bUeruSgGetzXctfTG36415987 = -359929611;    double bUeruSgGetzXctfTG62159150 = -16514102;    double bUeruSgGetzXctfTG1560903 = -883210794;    double bUeruSgGetzXctfTG60639181 = 48250207;    double bUeruSgGetzXctfTG82349487 = -835992674;    double bUeruSgGetzXctfTG71732412 = -620738025;    double bUeruSgGetzXctfTG56429477 = -18210848;    double bUeruSgGetzXctfTG32051048 = -85062542;    double bUeruSgGetzXctfTG98471130 = -151393652;    double bUeruSgGetzXctfTG49967780 = -899368549;    double bUeruSgGetzXctfTG15485390 = -210895046;    double bUeruSgGetzXctfTG34542846 = -231823804;    double bUeruSgGetzXctfTG35624781 = -247293435;    double bUeruSgGetzXctfTG76383801 = -412066880;    double bUeruSgGetzXctfTG2972614 = -656792350;    double bUeruSgGetzXctfTG43130687 = -394988029;    double bUeruSgGetzXctfTG18958822 = -231989024;    double bUeruSgGetzXctfTG35458528 = 62660703;    double bUeruSgGetzXctfTG50145475 = -884263263;    double bUeruSgGetzXctfTG39494408 = -91165798;    double bUeruSgGetzXctfTG70451876 = -546580198;    double bUeruSgGetzXctfTG39762982 = -618762193;    double bUeruSgGetzXctfTG3968322 = 8713659;    double bUeruSgGetzXctfTG41173264 = -388406022;    double bUeruSgGetzXctfTG40241560 = -100274300;    double bUeruSgGetzXctfTG69974192 = -686643372;    double bUeruSgGetzXctfTG72506824 = 28652328;    double bUeruSgGetzXctfTG97596643 = 15108657;    double bUeruSgGetzXctfTG77985147 = -44403967;    double bUeruSgGetzXctfTG34232748 = -526656004;    double bUeruSgGetzXctfTG57141483 = -753574603;    double bUeruSgGetzXctfTG38168834 = -182614395;    double bUeruSgGetzXctfTG19438659 = -278716455;    double bUeruSgGetzXctfTG36232997 = -779308893;    double bUeruSgGetzXctfTG58718677 = -903629676;    double bUeruSgGetzXctfTG89685098 = 9434699;    double bUeruSgGetzXctfTG37515679 = -630319717;    double bUeruSgGetzXctfTG72921297 = 30899989;    double bUeruSgGetzXctfTG77775609 = -531943119;    double bUeruSgGetzXctfTG31249348 = -788376496;    double bUeruSgGetzXctfTG77647985 = -769534096;    double bUeruSgGetzXctfTG51057734 = -417875144;    double bUeruSgGetzXctfTG14846792 = -963698102;    double bUeruSgGetzXctfTG15611694 = 83964505;    double bUeruSgGetzXctfTG49213496 = -6382561;    double bUeruSgGetzXctfTG73292404 = -321739226;    double bUeruSgGetzXctfTG88799592 = -904298111;    double bUeruSgGetzXctfTG28282326 = 89827318;    double bUeruSgGetzXctfTG8582989 = -623804198;    double bUeruSgGetzXctfTG20524765 = -152750605;    double bUeruSgGetzXctfTG80515885 = -461044318;    double bUeruSgGetzXctfTG46936478 = -995750539;    double bUeruSgGetzXctfTG2072260 = -559044249;    double bUeruSgGetzXctfTG38992455 = -558194837;    double bUeruSgGetzXctfTG28542456 = -662553461;    double bUeruSgGetzXctfTG96653004 = -741167419;    double bUeruSgGetzXctfTG58190828 = 74772238;    double bUeruSgGetzXctfTG60387638 = -394804772;    double bUeruSgGetzXctfTG20397622 = -863871753;     bUeruSgGetzXctfTG54816861 = bUeruSgGetzXctfTG82207268;     bUeruSgGetzXctfTG82207268 = bUeruSgGetzXctfTG52519301;     bUeruSgGetzXctfTG52519301 = bUeruSgGetzXctfTG95583624;     bUeruSgGetzXctfTG95583624 = bUeruSgGetzXctfTG53127235;     bUeruSgGetzXctfTG53127235 = bUeruSgGetzXctfTG73571870;     bUeruSgGetzXctfTG73571870 = bUeruSgGetzXctfTG53560658;     bUeruSgGetzXctfTG53560658 = bUeruSgGetzXctfTG53080904;     bUeruSgGetzXctfTG53080904 = bUeruSgGetzXctfTG69547795;     bUeruSgGetzXctfTG69547795 = bUeruSgGetzXctfTG5872151;     bUeruSgGetzXctfTG5872151 = bUeruSgGetzXctfTG48076810;     bUeruSgGetzXctfTG48076810 = bUeruSgGetzXctfTG94398564;     bUeruSgGetzXctfTG94398564 = bUeruSgGetzXctfTG77172951;     bUeruSgGetzXctfTG77172951 = bUeruSgGetzXctfTG9152246;     bUeruSgGetzXctfTG9152246 = bUeruSgGetzXctfTG41640599;     bUeruSgGetzXctfTG41640599 = bUeruSgGetzXctfTG33707455;     bUeruSgGetzXctfTG33707455 = bUeruSgGetzXctfTG39536490;     bUeruSgGetzXctfTG39536490 = bUeruSgGetzXctfTG90386569;     bUeruSgGetzXctfTG90386569 = bUeruSgGetzXctfTG97925522;     bUeruSgGetzXctfTG97925522 = bUeruSgGetzXctfTG35227329;     bUeruSgGetzXctfTG35227329 = bUeruSgGetzXctfTG95134663;     bUeruSgGetzXctfTG95134663 = bUeruSgGetzXctfTG51844248;     bUeruSgGetzXctfTG51844248 = bUeruSgGetzXctfTG39076581;     bUeruSgGetzXctfTG39076581 = bUeruSgGetzXctfTG33560479;     bUeruSgGetzXctfTG33560479 = bUeruSgGetzXctfTG60125096;     bUeruSgGetzXctfTG60125096 = bUeruSgGetzXctfTG2981761;     bUeruSgGetzXctfTG2981761 = bUeruSgGetzXctfTG34077462;     bUeruSgGetzXctfTG34077462 = bUeruSgGetzXctfTG83108782;     bUeruSgGetzXctfTG83108782 = bUeruSgGetzXctfTG13317923;     bUeruSgGetzXctfTG13317923 = bUeruSgGetzXctfTG65579473;     bUeruSgGetzXctfTG65579473 = bUeruSgGetzXctfTG64698886;     bUeruSgGetzXctfTG64698886 = bUeruSgGetzXctfTG7835250;     bUeruSgGetzXctfTG7835250 = bUeruSgGetzXctfTG24424373;     bUeruSgGetzXctfTG24424373 = bUeruSgGetzXctfTG4666127;     bUeruSgGetzXctfTG4666127 = bUeruSgGetzXctfTG11555603;     bUeruSgGetzXctfTG11555603 = bUeruSgGetzXctfTG63655451;     bUeruSgGetzXctfTG63655451 = bUeruSgGetzXctfTG99474707;     bUeruSgGetzXctfTG99474707 = bUeruSgGetzXctfTG82395006;     bUeruSgGetzXctfTG82395006 = bUeruSgGetzXctfTG52217735;     bUeruSgGetzXctfTG52217735 = bUeruSgGetzXctfTG78486863;     bUeruSgGetzXctfTG78486863 = bUeruSgGetzXctfTG98994331;     bUeruSgGetzXctfTG98994331 = bUeruSgGetzXctfTG36415987;     bUeruSgGetzXctfTG36415987 = bUeruSgGetzXctfTG62159150;     bUeruSgGetzXctfTG62159150 = bUeruSgGetzXctfTG1560903;     bUeruSgGetzXctfTG1560903 = bUeruSgGetzXctfTG60639181;     bUeruSgGetzXctfTG60639181 = bUeruSgGetzXctfTG82349487;     bUeruSgGetzXctfTG82349487 = bUeruSgGetzXctfTG71732412;     bUeruSgGetzXctfTG71732412 = bUeruSgGetzXctfTG56429477;     bUeruSgGetzXctfTG56429477 = bUeruSgGetzXctfTG32051048;     bUeruSgGetzXctfTG32051048 = bUeruSgGetzXctfTG98471130;     bUeruSgGetzXctfTG98471130 = bUeruSgGetzXctfTG49967780;     bUeruSgGetzXctfTG49967780 = bUeruSgGetzXctfTG15485390;     bUeruSgGetzXctfTG15485390 = bUeruSgGetzXctfTG34542846;     bUeruSgGetzXctfTG34542846 = bUeruSgGetzXctfTG35624781;     bUeruSgGetzXctfTG35624781 = bUeruSgGetzXctfTG76383801;     bUeruSgGetzXctfTG76383801 = bUeruSgGetzXctfTG2972614;     bUeruSgGetzXctfTG2972614 = bUeruSgGetzXctfTG43130687;     bUeruSgGetzXctfTG43130687 = bUeruSgGetzXctfTG18958822;     bUeruSgGetzXctfTG18958822 = bUeruSgGetzXctfTG35458528;     bUeruSgGetzXctfTG35458528 = bUeruSgGetzXctfTG50145475;     bUeruSgGetzXctfTG50145475 = bUeruSgGetzXctfTG39494408;     bUeruSgGetzXctfTG39494408 = bUeruSgGetzXctfTG70451876;     bUeruSgGetzXctfTG70451876 = bUeruSgGetzXctfTG39762982;     bUeruSgGetzXctfTG39762982 = bUeruSgGetzXctfTG3968322;     bUeruSgGetzXctfTG3968322 = bUeruSgGetzXctfTG41173264;     bUeruSgGetzXctfTG41173264 = bUeruSgGetzXctfTG40241560;     bUeruSgGetzXctfTG40241560 = bUeruSgGetzXctfTG69974192;     bUeruSgGetzXctfTG69974192 = bUeruSgGetzXctfTG72506824;     bUeruSgGetzXctfTG72506824 = bUeruSgGetzXctfTG97596643;     bUeruSgGetzXctfTG97596643 = bUeruSgGetzXctfTG77985147;     bUeruSgGetzXctfTG77985147 = bUeruSgGetzXctfTG34232748;     bUeruSgGetzXctfTG34232748 = bUeruSgGetzXctfTG57141483;     bUeruSgGetzXctfTG57141483 = bUeruSgGetzXctfTG38168834;     bUeruSgGetzXctfTG38168834 = bUeruSgGetzXctfTG19438659;     bUeruSgGetzXctfTG19438659 = bUeruSgGetzXctfTG36232997;     bUeruSgGetzXctfTG36232997 = bUeruSgGetzXctfTG58718677;     bUeruSgGetzXctfTG58718677 = bUeruSgGetzXctfTG89685098;     bUeruSgGetzXctfTG89685098 = bUeruSgGetzXctfTG37515679;     bUeruSgGetzXctfTG37515679 = bUeruSgGetzXctfTG72921297;     bUeruSgGetzXctfTG72921297 = bUeruSgGetzXctfTG77775609;     bUeruSgGetzXctfTG77775609 = bUeruSgGetzXctfTG31249348;     bUeruSgGetzXctfTG31249348 = bUeruSgGetzXctfTG77647985;     bUeruSgGetzXctfTG77647985 = bUeruSgGetzXctfTG51057734;     bUeruSgGetzXctfTG51057734 = bUeruSgGetzXctfTG14846792;     bUeruSgGetzXctfTG14846792 = bUeruSgGetzXctfTG15611694;     bUeruSgGetzXctfTG15611694 = bUeruSgGetzXctfTG49213496;     bUeruSgGetzXctfTG49213496 = bUeruSgGetzXctfTG73292404;     bUeruSgGetzXctfTG73292404 = bUeruSgGetzXctfTG88799592;     bUeruSgGetzXctfTG88799592 = bUeruSgGetzXctfTG28282326;     bUeruSgGetzXctfTG28282326 = bUeruSgGetzXctfTG8582989;     bUeruSgGetzXctfTG8582989 = bUeruSgGetzXctfTG20524765;     bUeruSgGetzXctfTG20524765 = bUeruSgGetzXctfTG80515885;     bUeruSgGetzXctfTG80515885 = bUeruSgGetzXctfTG46936478;     bUeruSgGetzXctfTG46936478 = bUeruSgGetzXctfTG2072260;     bUeruSgGetzXctfTG2072260 = bUeruSgGetzXctfTG38992455;     bUeruSgGetzXctfTG38992455 = bUeruSgGetzXctfTG28542456;     bUeruSgGetzXctfTG28542456 = bUeruSgGetzXctfTG96653004;     bUeruSgGetzXctfTG96653004 = bUeruSgGetzXctfTG58190828;     bUeruSgGetzXctfTG58190828 = bUeruSgGetzXctfTG60387638;     bUeruSgGetzXctfTG60387638 = bUeruSgGetzXctfTG20397622;     bUeruSgGetzXctfTG20397622 = bUeruSgGetzXctfTG54816861;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void WwEATcfBlmAOIKNj38810064() {     double KZfHYSoEIvXtLOxBc90275829 = -230148652;    double KZfHYSoEIvXtLOxBc95479346 = -889070145;    double KZfHYSoEIvXtLOxBc89905777 = -18505902;    double KZfHYSoEIvXtLOxBc67806979 = -226426161;    double KZfHYSoEIvXtLOxBc9412505 = -398976628;    double KZfHYSoEIvXtLOxBc9794743 = -241258636;    double KZfHYSoEIvXtLOxBc64218065 = 5756382;    double KZfHYSoEIvXtLOxBc15212466 = -478088087;    double KZfHYSoEIvXtLOxBc71065400 = -395021791;    double KZfHYSoEIvXtLOxBc20774863 = -470370626;    double KZfHYSoEIvXtLOxBc86258544 = -655845058;    double KZfHYSoEIvXtLOxBc94966713 = -802355688;    double KZfHYSoEIvXtLOxBc15407597 = 54400262;    double KZfHYSoEIvXtLOxBc62293109 = -341851300;    double KZfHYSoEIvXtLOxBc99835115 = -195567615;    double KZfHYSoEIvXtLOxBc17248756 = 44499049;    double KZfHYSoEIvXtLOxBc95372426 = -213638992;    double KZfHYSoEIvXtLOxBc90146760 = -799349974;    double KZfHYSoEIvXtLOxBc98475852 = -344640510;    double KZfHYSoEIvXtLOxBc29064548 = -415003617;    double KZfHYSoEIvXtLOxBc8221527 = 79166025;    double KZfHYSoEIvXtLOxBc55265772 = -193715697;    double KZfHYSoEIvXtLOxBc9601962 = -151162694;    double KZfHYSoEIvXtLOxBc27353424 = -531674408;    double KZfHYSoEIvXtLOxBc68143047 = -391920991;    double KZfHYSoEIvXtLOxBc13679932 = -97691327;    double KZfHYSoEIvXtLOxBc94498036 = -694029084;    double KZfHYSoEIvXtLOxBc23837208 = -772709860;    double KZfHYSoEIvXtLOxBc59986137 = -689186557;    double KZfHYSoEIvXtLOxBc71112884 = -949911214;    double KZfHYSoEIvXtLOxBc83282118 = -869258093;    double KZfHYSoEIvXtLOxBc45658268 = -426534444;    double KZfHYSoEIvXtLOxBc36642384 = -299187522;    double KZfHYSoEIvXtLOxBc34414554 = -486933375;    double KZfHYSoEIvXtLOxBc77364056 = -231696798;    double KZfHYSoEIvXtLOxBc15216296 = -862303939;    double KZfHYSoEIvXtLOxBc48886524 = -657729381;    double KZfHYSoEIvXtLOxBc33610410 = 84254583;    double KZfHYSoEIvXtLOxBc82608297 = -94183788;    double KZfHYSoEIvXtLOxBc42758395 = -214623180;    double KZfHYSoEIvXtLOxBc32445940 = 21667427;    double KZfHYSoEIvXtLOxBc25781113 = 45525098;    double KZfHYSoEIvXtLOxBc51542528 = -669694888;    double KZfHYSoEIvXtLOxBc41692240 = -387543204;    double KZfHYSoEIvXtLOxBc60655640 = -878260954;    double KZfHYSoEIvXtLOxBc28310816 = -874916306;    double KZfHYSoEIvXtLOxBc91077575 = -4724386;    double KZfHYSoEIvXtLOxBc74155789 = -184737442;    double KZfHYSoEIvXtLOxBc54943714 = -848799422;    double KZfHYSoEIvXtLOxBc23000721 = -642420468;    double KZfHYSoEIvXtLOxBc30738504 = -773645107;    double KZfHYSoEIvXtLOxBc98970862 = -424547392;    double KZfHYSoEIvXtLOxBc3163995 = 12130642;    double KZfHYSoEIvXtLOxBc51616627 = -438801934;    double KZfHYSoEIvXtLOxBc30588538 = -758557025;    double KZfHYSoEIvXtLOxBc35010058 = 63567044;    double KZfHYSoEIvXtLOxBc85877385 = -637907452;    double KZfHYSoEIvXtLOxBc62552353 = -486831494;    double KZfHYSoEIvXtLOxBc99663931 = -834505170;    double KZfHYSoEIvXtLOxBc95732572 = -201285301;    double KZfHYSoEIvXtLOxBc15296706 = -547229552;    double KZfHYSoEIvXtLOxBc40380858 = -221533758;    double KZfHYSoEIvXtLOxBc55226328 = -788901531;    double KZfHYSoEIvXtLOxBc99952515 = -445110577;    double KZfHYSoEIvXtLOxBc37492745 = -601112533;    double KZfHYSoEIvXtLOxBc40600276 = -129310615;    double KZfHYSoEIvXtLOxBc58324329 = -403168166;    double KZfHYSoEIvXtLOxBc80993043 = -458666364;    double KZfHYSoEIvXtLOxBc84929052 = -10154502;    double KZfHYSoEIvXtLOxBc84618819 = -333263676;    double KZfHYSoEIvXtLOxBc68362232 = -297771570;    double KZfHYSoEIvXtLOxBc61762016 = -197893575;    double KZfHYSoEIvXtLOxBc7538464 = -605166187;    double KZfHYSoEIvXtLOxBc55717457 = -30017331;    double KZfHYSoEIvXtLOxBc96618608 = -336671044;    double KZfHYSoEIvXtLOxBc82440413 = -966359074;    double KZfHYSoEIvXtLOxBc3723245 = -524020809;    double KZfHYSoEIvXtLOxBc67909721 = -763619491;    double KZfHYSoEIvXtLOxBc66697784 = -653413454;    double KZfHYSoEIvXtLOxBc39832232 = -517004685;    double KZfHYSoEIvXtLOxBc22602356 = 7033058;    double KZfHYSoEIvXtLOxBc20342247 = -409291643;    double KZfHYSoEIvXtLOxBc68893493 = -923910438;    double KZfHYSoEIvXtLOxBc36985417 = 53233911;    double KZfHYSoEIvXtLOxBc40374381 = -76266108;    double KZfHYSoEIvXtLOxBc84311255 = -344710702;    double KZfHYSoEIvXtLOxBc42494274 = -338665087;    double KZfHYSoEIvXtLOxBc85025756 = -860385588;    double KZfHYSoEIvXtLOxBc3826016 = -728376350;    double KZfHYSoEIvXtLOxBc42353999 = -195263843;    double KZfHYSoEIvXtLOxBc29338910 = -124396488;    double KZfHYSoEIvXtLOxBc86334170 = -70897887;    double KZfHYSoEIvXtLOxBc33946478 = -81240248;    double KZfHYSoEIvXtLOxBc86875724 = -892898487;    double KZfHYSoEIvXtLOxBc27461689 = -667393628;    double KZfHYSoEIvXtLOxBc92065082 = -756798815;    double KZfHYSoEIvXtLOxBc70554785 = -165573372;    double KZfHYSoEIvXtLOxBc51590012 = -124584311;    double KZfHYSoEIvXtLOxBc4199496 = -786430671;    double KZfHYSoEIvXtLOxBc20055364 = -230148652;     KZfHYSoEIvXtLOxBc90275829 = KZfHYSoEIvXtLOxBc95479346;     KZfHYSoEIvXtLOxBc95479346 = KZfHYSoEIvXtLOxBc89905777;     KZfHYSoEIvXtLOxBc89905777 = KZfHYSoEIvXtLOxBc67806979;     KZfHYSoEIvXtLOxBc67806979 = KZfHYSoEIvXtLOxBc9412505;     KZfHYSoEIvXtLOxBc9412505 = KZfHYSoEIvXtLOxBc9794743;     KZfHYSoEIvXtLOxBc9794743 = KZfHYSoEIvXtLOxBc64218065;     KZfHYSoEIvXtLOxBc64218065 = KZfHYSoEIvXtLOxBc15212466;     KZfHYSoEIvXtLOxBc15212466 = KZfHYSoEIvXtLOxBc71065400;     KZfHYSoEIvXtLOxBc71065400 = KZfHYSoEIvXtLOxBc20774863;     KZfHYSoEIvXtLOxBc20774863 = KZfHYSoEIvXtLOxBc86258544;     KZfHYSoEIvXtLOxBc86258544 = KZfHYSoEIvXtLOxBc94966713;     KZfHYSoEIvXtLOxBc94966713 = KZfHYSoEIvXtLOxBc15407597;     KZfHYSoEIvXtLOxBc15407597 = KZfHYSoEIvXtLOxBc62293109;     KZfHYSoEIvXtLOxBc62293109 = KZfHYSoEIvXtLOxBc99835115;     KZfHYSoEIvXtLOxBc99835115 = KZfHYSoEIvXtLOxBc17248756;     KZfHYSoEIvXtLOxBc17248756 = KZfHYSoEIvXtLOxBc95372426;     KZfHYSoEIvXtLOxBc95372426 = KZfHYSoEIvXtLOxBc90146760;     KZfHYSoEIvXtLOxBc90146760 = KZfHYSoEIvXtLOxBc98475852;     KZfHYSoEIvXtLOxBc98475852 = KZfHYSoEIvXtLOxBc29064548;     KZfHYSoEIvXtLOxBc29064548 = KZfHYSoEIvXtLOxBc8221527;     KZfHYSoEIvXtLOxBc8221527 = KZfHYSoEIvXtLOxBc55265772;     KZfHYSoEIvXtLOxBc55265772 = KZfHYSoEIvXtLOxBc9601962;     KZfHYSoEIvXtLOxBc9601962 = KZfHYSoEIvXtLOxBc27353424;     KZfHYSoEIvXtLOxBc27353424 = KZfHYSoEIvXtLOxBc68143047;     KZfHYSoEIvXtLOxBc68143047 = KZfHYSoEIvXtLOxBc13679932;     KZfHYSoEIvXtLOxBc13679932 = KZfHYSoEIvXtLOxBc94498036;     KZfHYSoEIvXtLOxBc94498036 = KZfHYSoEIvXtLOxBc23837208;     KZfHYSoEIvXtLOxBc23837208 = KZfHYSoEIvXtLOxBc59986137;     KZfHYSoEIvXtLOxBc59986137 = KZfHYSoEIvXtLOxBc71112884;     KZfHYSoEIvXtLOxBc71112884 = KZfHYSoEIvXtLOxBc83282118;     KZfHYSoEIvXtLOxBc83282118 = KZfHYSoEIvXtLOxBc45658268;     KZfHYSoEIvXtLOxBc45658268 = KZfHYSoEIvXtLOxBc36642384;     KZfHYSoEIvXtLOxBc36642384 = KZfHYSoEIvXtLOxBc34414554;     KZfHYSoEIvXtLOxBc34414554 = KZfHYSoEIvXtLOxBc77364056;     KZfHYSoEIvXtLOxBc77364056 = KZfHYSoEIvXtLOxBc15216296;     KZfHYSoEIvXtLOxBc15216296 = KZfHYSoEIvXtLOxBc48886524;     KZfHYSoEIvXtLOxBc48886524 = KZfHYSoEIvXtLOxBc33610410;     KZfHYSoEIvXtLOxBc33610410 = KZfHYSoEIvXtLOxBc82608297;     KZfHYSoEIvXtLOxBc82608297 = KZfHYSoEIvXtLOxBc42758395;     KZfHYSoEIvXtLOxBc42758395 = KZfHYSoEIvXtLOxBc32445940;     KZfHYSoEIvXtLOxBc32445940 = KZfHYSoEIvXtLOxBc25781113;     KZfHYSoEIvXtLOxBc25781113 = KZfHYSoEIvXtLOxBc51542528;     KZfHYSoEIvXtLOxBc51542528 = KZfHYSoEIvXtLOxBc41692240;     KZfHYSoEIvXtLOxBc41692240 = KZfHYSoEIvXtLOxBc60655640;     KZfHYSoEIvXtLOxBc60655640 = KZfHYSoEIvXtLOxBc28310816;     KZfHYSoEIvXtLOxBc28310816 = KZfHYSoEIvXtLOxBc91077575;     KZfHYSoEIvXtLOxBc91077575 = KZfHYSoEIvXtLOxBc74155789;     KZfHYSoEIvXtLOxBc74155789 = KZfHYSoEIvXtLOxBc54943714;     KZfHYSoEIvXtLOxBc54943714 = KZfHYSoEIvXtLOxBc23000721;     KZfHYSoEIvXtLOxBc23000721 = KZfHYSoEIvXtLOxBc30738504;     KZfHYSoEIvXtLOxBc30738504 = KZfHYSoEIvXtLOxBc98970862;     KZfHYSoEIvXtLOxBc98970862 = KZfHYSoEIvXtLOxBc3163995;     KZfHYSoEIvXtLOxBc3163995 = KZfHYSoEIvXtLOxBc51616627;     KZfHYSoEIvXtLOxBc51616627 = KZfHYSoEIvXtLOxBc30588538;     KZfHYSoEIvXtLOxBc30588538 = KZfHYSoEIvXtLOxBc35010058;     KZfHYSoEIvXtLOxBc35010058 = KZfHYSoEIvXtLOxBc85877385;     KZfHYSoEIvXtLOxBc85877385 = KZfHYSoEIvXtLOxBc62552353;     KZfHYSoEIvXtLOxBc62552353 = KZfHYSoEIvXtLOxBc99663931;     KZfHYSoEIvXtLOxBc99663931 = KZfHYSoEIvXtLOxBc95732572;     KZfHYSoEIvXtLOxBc95732572 = KZfHYSoEIvXtLOxBc15296706;     KZfHYSoEIvXtLOxBc15296706 = KZfHYSoEIvXtLOxBc40380858;     KZfHYSoEIvXtLOxBc40380858 = KZfHYSoEIvXtLOxBc55226328;     KZfHYSoEIvXtLOxBc55226328 = KZfHYSoEIvXtLOxBc99952515;     KZfHYSoEIvXtLOxBc99952515 = KZfHYSoEIvXtLOxBc37492745;     KZfHYSoEIvXtLOxBc37492745 = KZfHYSoEIvXtLOxBc40600276;     KZfHYSoEIvXtLOxBc40600276 = KZfHYSoEIvXtLOxBc58324329;     KZfHYSoEIvXtLOxBc58324329 = KZfHYSoEIvXtLOxBc80993043;     KZfHYSoEIvXtLOxBc80993043 = KZfHYSoEIvXtLOxBc84929052;     KZfHYSoEIvXtLOxBc84929052 = KZfHYSoEIvXtLOxBc84618819;     KZfHYSoEIvXtLOxBc84618819 = KZfHYSoEIvXtLOxBc68362232;     KZfHYSoEIvXtLOxBc68362232 = KZfHYSoEIvXtLOxBc61762016;     KZfHYSoEIvXtLOxBc61762016 = KZfHYSoEIvXtLOxBc7538464;     KZfHYSoEIvXtLOxBc7538464 = KZfHYSoEIvXtLOxBc55717457;     KZfHYSoEIvXtLOxBc55717457 = KZfHYSoEIvXtLOxBc96618608;     KZfHYSoEIvXtLOxBc96618608 = KZfHYSoEIvXtLOxBc82440413;     KZfHYSoEIvXtLOxBc82440413 = KZfHYSoEIvXtLOxBc3723245;     KZfHYSoEIvXtLOxBc3723245 = KZfHYSoEIvXtLOxBc67909721;     KZfHYSoEIvXtLOxBc67909721 = KZfHYSoEIvXtLOxBc66697784;     KZfHYSoEIvXtLOxBc66697784 = KZfHYSoEIvXtLOxBc39832232;     KZfHYSoEIvXtLOxBc39832232 = KZfHYSoEIvXtLOxBc22602356;     KZfHYSoEIvXtLOxBc22602356 = KZfHYSoEIvXtLOxBc20342247;     KZfHYSoEIvXtLOxBc20342247 = KZfHYSoEIvXtLOxBc68893493;     KZfHYSoEIvXtLOxBc68893493 = KZfHYSoEIvXtLOxBc36985417;     KZfHYSoEIvXtLOxBc36985417 = KZfHYSoEIvXtLOxBc40374381;     KZfHYSoEIvXtLOxBc40374381 = KZfHYSoEIvXtLOxBc84311255;     KZfHYSoEIvXtLOxBc84311255 = KZfHYSoEIvXtLOxBc42494274;     KZfHYSoEIvXtLOxBc42494274 = KZfHYSoEIvXtLOxBc85025756;     KZfHYSoEIvXtLOxBc85025756 = KZfHYSoEIvXtLOxBc3826016;     KZfHYSoEIvXtLOxBc3826016 = KZfHYSoEIvXtLOxBc42353999;     KZfHYSoEIvXtLOxBc42353999 = KZfHYSoEIvXtLOxBc29338910;     KZfHYSoEIvXtLOxBc29338910 = KZfHYSoEIvXtLOxBc86334170;     KZfHYSoEIvXtLOxBc86334170 = KZfHYSoEIvXtLOxBc33946478;     KZfHYSoEIvXtLOxBc33946478 = KZfHYSoEIvXtLOxBc86875724;     KZfHYSoEIvXtLOxBc86875724 = KZfHYSoEIvXtLOxBc27461689;     KZfHYSoEIvXtLOxBc27461689 = KZfHYSoEIvXtLOxBc92065082;     KZfHYSoEIvXtLOxBc92065082 = KZfHYSoEIvXtLOxBc70554785;     KZfHYSoEIvXtLOxBc70554785 = KZfHYSoEIvXtLOxBc51590012;     KZfHYSoEIvXtLOxBc51590012 = KZfHYSoEIvXtLOxBc4199496;     KZfHYSoEIvXtLOxBc4199496 = KZfHYSoEIvXtLOxBc20055364;     KZfHYSoEIvXtLOxBc20055364 = KZfHYSoEIvXtLOxBc90275829;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void uuJkYdvqSDzMvTuV53859132() {     double zJnAudexSdFhugEkJ96392936 = -342050541;    double zJnAudexSdFhugEkJ38852626 = -598669926;    double zJnAudexSdFhugEkJ5947962 = -169338823;    double zJnAudexSdFhugEkJ94881760 = -417174791;    double zJnAudexSdFhugEkJ78307993 = -919626500;    double zJnAudexSdFhugEkJ1211945 = -505754871;    double zJnAudexSdFhugEkJ24707031 = -133208595;    double zJnAudexSdFhugEkJ64972356 = -764962000;    double zJnAudexSdFhugEkJ35707896 = -163054457;    double zJnAudexSdFhugEkJ62004522 = -799346103;    double zJnAudexSdFhugEkJ46368835 = -533282645;    double zJnAudexSdFhugEkJ26789756 = -146761890;    double zJnAudexSdFhugEkJ1616716 = -385490827;    double zJnAudexSdFhugEkJ45436786 = -874089867;    double zJnAudexSdFhugEkJ95577541 = -886029781;    double zJnAudexSdFhugEkJ58754150 = 71499506;    double zJnAudexSdFhugEkJ89208429 = -830184104;    double zJnAudexSdFhugEkJ87199913 = -464872617;    double zJnAudexSdFhugEkJ60305694 = -750409607;    double zJnAudexSdFhugEkJ9614713 = -771906536;    double zJnAudexSdFhugEkJ96774719 = -876321431;    double zJnAudexSdFhugEkJ41786796 = -101645413;    double zJnAudexSdFhugEkJ45192753 = -997680181;    double zJnAudexSdFhugEkJ11345798 = -753979396;    double zJnAudexSdFhugEkJ11842698 = -765117024;    double zJnAudexSdFhugEkJ38401607 = 23653349;    double zJnAudexSdFhugEkJ92957814 = -594691288;    double zJnAudexSdFhugEkJ36944333 = -838638814;    double zJnAudexSdFhugEkJ26118393 = -911569261;    double zJnAudexSdFhugEkJ39728521 = -767382081;    double zJnAudexSdFhugEkJ75524667 = -782579031;    double zJnAudexSdFhugEkJ43268890 = -135981260;    double zJnAudexSdFhugEkJ46353332 = -191800892;    double zJnAudexSdFhugEkJ29245118 = -238475140;    double zJnAudexSdFhugEkJ38838708 = -870429904;    double zJnAudexSdFhugEkJ83942208 = 23669953;    double zJnAudexSdFhugEkJ30148459 = -611365251;    double zJnAudexSdFhugEkJ67700622 = -787251427;    double zJnAudexSdFhugEkJ56218551 = -88107285;    double zJnAudexSdFhugEkJ30260333 = -688062402;    double zJnAudexSdFhugEkJ38115164 = -43606467;    double zJnAudexSdFhugEkJ57931570 = -224617180;    double zJnAudexSdFhugEkJ29609507 = -750499472;    double zJnAudexSdFhugEkJ29099981 = -406886142;    double zJnAudexSdFhugEkJ72608105 = -540159109;    double zJnAudexSdFhugEkJ40250011 = -860132422;    double zJnAudexSdFhugEkJ8856150 = -123499923;    double zJnAudexSdFhugEkJ82112730 = -194632974;    double zJnAudexSdFhugEkJ64619443 = -934536411;    double zJnAudexSdFhugEkJ35912373 = -729431276;    double zJnAudexSdFhugEkJ66439182 = -615938778;    double zJnAudexSdFhugEkJ9780582 = -225273712;    double zJnAudexSdFhugEkJ51412452 = -879658946;    double zJnAudexSdFhugEkJ47585507 = -759272080;    double zJnAudexSdFhugEkJ57084412 = -359088808;    double zJnAudexSdFhugEkJ54606140 = -140405129;    double zJnAudexSdFhugEkJ93659873 = -600989745;    double zJnAudexSdFhugEkJ94602163 = -415359428;    double zJnAudexSdFhugEkJ83039062 = -652057768;    double zJnAudexSdFhugEkJ39906386 = -843279849;    double zJnAudexSdFhugEkJ8254131 = -911063584;    double zJnAudexSdFhugEkJ87762698 = -294569782;    double zJnAudexSdFhugEkJ38853964 = -853392739;    double zJnAudexSdFhugEkJ95979374 = -395672376;    double zJnAudexSdFhugEkJ86479854 = 83232928;    double zJnAudexSdFhugEkJ3099945 = -297301385;    double zJnAudexSdFhugEkJ80436423 = -954960998;    double zJnAudexSdFhugEkJ72371598 = -47015687;    double zJnAudexSdFhugEkJ6598078 = 96340036;    double zJnAudexSdFhugEkJ11635334 = -809699735;    double zJnAudexSdFhugEkJ28605691 = -317135244;    double zJnAudexSdFhugEkJ21507807 = 57067322;    double zJnAudexSdFhugEkJ30981363 = -276765333;    double zJnAudexSdFhugEkJ30045361 = 37652795;    double zJnAudexSdFhugEkJ71499549 = -628300070;    double zJnAudexSdFhugEkJ38843149 = -551704251;    double zJnAudexSdFhugEkJ12177290 = -351145942;    double zJnAudexSdFhugEkJ16092772 = -490794040;    double zJnAudexSdFhugEkJ38737693 = -113820287;    double zJnAudexSdFhugEkJ71592687 = -904984602;    double zJnAudexSdFhugEkJ29545457 = -852846728;    double zJnAudexSdFhugEkJ10845085 = -300058314;    double zJnAudexSdFhugEkJ72324889 = -904102404;    double zJnAudexSdFhugEkJ90206019 = -82137986;    double zJnAudexSdFhugEkJ73289339 = -51443304;    double zJnAudexSdFhugEkJ65744085 = -457305320;    double zJnAudexSdFhugEkJ91856438 = -256322315;    double zJnAudexSdFhugEkJ98767824 = -432528813;    double zJnAudexSdFhugEkJ72160705 = -879386333;    double zJnAudexSdFhugEkJ84232568 = -630024776;    double zJnAudexSdFhugEkJ90282335 = -375340302;    double zJnAudexSdFhugEkJ35546296 = -96005824;    double zJnAudexSdFhugEkJ84661560 = -35193660;    double zJnAudexSdFhugEkJ16312165 = -244827436;    double zJnAudexSdFhugEkJ22006203 = -776998819;    double zJnAudexSdFhugEkJ50352465 = -749036686;    double zJnAudexSdFhugEkJ19077607 = -371224441;    double zJnAudexSdFhugEkJ33630132 = -254827097;    double zJnAudexSdFhugEkJ42620126 = -390119070;    double zJnAudexSdFhugEkJ69508160 = -342050541;     zJnAudexSdFhugEkJ96392936 = zJnAudexSdFhugEkJ38852626;     zJnAudexSdFhugEkJ38852626 = zJnAudexSdFhugEkJ5947962;     zJnAudexSdFhugEkJ5947962 = zJnAudexSdFhugEkJ94881760;     zJnAudexSdFhugEkJ94881760 = zJnAudexSdFhugEkJ78307993;     zJnAudexSdFhugEkJ78307993 = zJnAudexSdFhugEkJ1211945;     zJnAudexSdFhugEkJ1211945 = zJnAudexSdFhugEkJ24707031;     zJnAudexSdFhugEkJ24707031 = zJnAudexSdFhugEkJ64972356;     zJnAudexSdFhugEkJ64972356 = zJnAudexSdFhugEkJ35707896;     zJnAudexSdFhugEkJ35707896 = zJnAudexSdFhugEkJ62004522;     zJnAudexSdFhugEkJ62004522 = zJnAudexSdFhugEkJ46368835;     zJnAudexSdFhugEkJ46368835 = zJnAudexSdFhugEkJ26789756;     zJnAudexSdFhugEkJ26789756 = zJnAudexSdFhugEkJ1616716;     zJnAudexSdFhugEkJ1616716 = zJnAudexSdFhugEkJ45436786;     zJnAudexSdFhugEkJ45436786 = zJnAudexSdFhugEkJ95577541;     zJnAudexSdFhugEkJ95577541 = zJnAudexSdFhugEkJ58754150;     zJnAudexSdFhugEkJ58754150 = zJnAudexSdFhugEkJ89208429;     zJnAudexSdFhugEkJ89208429 = zJnAudexSdFhugEkJ87199913;     zJnAudexSdFhugEkJ87199913 = zJnAudexSdFhugEkJ60305694;     zJnAudexSdFhugEkJ60305694 = zJnAudexSdFhugEkJ9614713;     zJnAudexSdFhugEkJ9614713 = zJnAudexSdFhugEkJ96774719;     zJnAudexSdFhugEkJ96774719 = zJnAudexSdFhugEkJ41786796;     zJnAudexSdFhugEkJ41786796 = zJnAudexSdFhugEkJ45192753;     zJnAudexSdFhugEkJ45192753 = zJnAudexSdFhugEkJ11345798;     zJnAudexSdFhugEkJ11345798 = zJnAudexSdFhugEkJ11842698;     zJnAudexSdFhugEkJ11842698 = zJnAudexSdFhugEkJ38401607;     zJnAudexSdFhugEkJ38401607 = zJnAudexSdFhugEkJ92957814;     zJnAudexSdFhugEkJ92957814 = zJnAudexSdFhugEkJ36944333;     zJnAudexSdFhugEkJ36944333 = zJnAudexSdFhugEkJ26118393;     zJnAudexSdFhugEkJ26118393 = zJnAudexSdFhugEkJ39728521;     zJnAudexSdFhugEkJ39728521 = zJnAudexSdFhugEkJ75524667;     zJnAudexSdFhugEkJ75524667 = zJnAudexSdFhugEkJ43268890;     zJnAudexSdFhugEkJ43268890 = zJnAudexSdFhugEkJ46353332;     zJnAudexSdFhugEkJ46353332 = zJnAudexSdFhugEkJ29245118;     zJnAudexSdFhugEkJ29245118 = zJnAudexSdFhugEkJ38838708;     zJnAudexSdFhugEkJ38838708 = zJnAudexSdFhugEkJ83942208;     zJnAudexSdFhugEkJ83942208 = zJnAudexSdFhugEkJ30148459;     zJnAudexSdFhugEkJ30148459 = zJnAudexSdFhugEkJ67700622;     zJnAudexSdFhugEkJ67700622 = zJnAudexSdFhugEkJ56218551;     zJnAudexSdFhugEkJ56218551 = zJnAudexSdFhugEkJ30260333;     zJnAudexSdFhugEkJ30260333 = zJnAudexSdFhugEkJ38115164;     zJnAudexSdFhugEkJ38115164 = zJnAudexSdFhugEkJ57931570;     zJnAudexSdFhugEkJ57931570 = zJnAudexSdFhugEkJ29609507;     zJnAudexSdFhugEkJ29609507 = zJnAudexSdFhugEkJ29099981;     zJnAudexSdFhugEkJ29099981 = zJnAudexSdFhugEkJ72608105;     zJnAudexSdFhugEkJ72608105 = zJnAudexSdFhugEkJ40250011;     zJnAudexSdFhugEkJ40250011 = zJnAudexSdFhugEkJ8856150;     zJnAudexSdFhugEkJ8856150 = zJnAudexSdFhugEkJ82112730;     zJnAudexSdFhugEkJ82112730 = zJnAudexSdFhugEkJ64619443;     zJnAudexSdFhugEkJ64619443 = zJnAudexSdFhugEkJ35912373;     zJnAudexSdFhugEkJ35912373 = zJnAudexSdFhugEkJ66439182;     zJnAudexSdFhugEkJ66439182 = zJnAudexSdFhugEkJ9780582;     zJnAudexSdFhugEkJ9780582 = zJnAudexSdFhugEkJ51412452;     zJnAudexSdFhugEkJ51412452 = zJnAudexSdFhugEkJ47585507;     zJnAudexSdFhugEkJ47585507 = zJnAudexSdFhugEkJ57084412;     zJnAudexSdFhugEkJ57084412 = zJnAudexSdFhugEkJ54606140;     zJnAudexSdFhugEkJ54606140 = zJnAudexSdFhugEkJ93659873;     zJnAudexSdFhugEkJ93659873 = zJnAudexSdFhugEkJ94602163;     zJnAudexSdFhugEkJ94602163 = zJnAudexSdFhugEkJ83039062;     zJnAudexSdFhugEkJ83039062 = zJnAudexSdFhugEkJ39906386;     zJnAudexSdFhugEkJ39906386 = zJnAudexSdFhugEkJ8254131;     zJnAudexSdFhugEkJ8254131 = zJnAudexSdFhugEkJ87762698;     zJnAudexSdFhugEkJ87762698 = zJnAudexSdFhugEkJ38853964;     zJnAudexSdFhugEkJ38853964 = zJnAudexSdFhugEkJ95979374;     zJnAudexSdFhugEkJ95979374 = zJnAudexSdFhugEkJ86479854;     zJnAudexSdFhugEkJ86479854 = zJnAudexSdFhugEkJ3099945;     zJnAudexSdFhugEkJ3099945 = zJnAudexSdFhugEkJ80436423;     zJnAudexSdFhugEkJ80436423 = zJnAudexSdFhugEkJ72371598;     zJnAudexSdFhugEkJ72371598 = zJnAudexSdFhugEkJ6598078;     zJnAudexSdFhugEkJ6598078 = zJnAudexSdFhugEkJ11635334;     zJnAudexSdFhugEkJ11635334 = zJnAudexSdFhugEkJ28605691;     zJnAudexSdFhugEkJ28605691 = zJnAudexSdFhugEkJ21507807;     zJnAudexSdFhugEkJ21507807 = zJnAudexSdFhugEkJ30981363;     zJnAudexSdFhugEkJ30981363 = zJnAudexSdFhugEkJ30045361;     zJnAudexSdFhugEkJ30045361 = zJnAudexSdFhugEkJ71499549;     zJnAudexSdFhugEkJ71499549 = zJnAudexSdFhugEkJ38843149;     zJnAudexSdFhugEkJ38843149 = zJnAudexSdFhugEkJ12177290;     zJnAudexSdFhugEkJ12177290 = zJnAudexSdFhugEkJ16092772;     zJnAudexSdFhugEkJ16092772 = zJnAudexSdFhugEkJ38737693;     zJnAudexSdFhugEkJ38737693 = zJnAudexSdFhugEkJ71592687;     zJnAudexSdFhugEkJ71592687 = zJnAudexSdFhugEkJ29545457;     zJnAudexSdFhugEkJ29545457 = zJnAudexSdFhugEkJ10845085;     zJnAudexSdFhugEkJ10845085 = zJnAudexSdFhugEkJ72324889;     zJnAudexSdFhugEkJ72324889 = zJnAudexSdFhugEkJ90206019;     zJnAudexSdFhugEkJ90206019 = zJnAudexSdFhugEkJ73289339;     zJnAudexSdFhugEkJ73289339 = zJnAudexSdFhugEkJ65744085;     zJnAudexSdFhugEkJ65744085 = zJnAudexSdFhugEkJ91856438;     zJnAudexSdFhugEkJ91856438 = zJnAudexSdFhugEkJ98767824;     zJnAudexSdFhugEkJ98767824 = zJnAudexSdFhugEkJ72160705;     zJnAudexSdFhugEkJ72160705 = zJnAudexSdFhugEkJ84232568;     zJnAudexSdFhugEkJ84232568 = zJnAudexSdFhugEkJ90282335;     zJnAudexSdFhugEkJ90282335 = zJnAudexSdFhugEkJ35546296;     zJnAudexSdFhugEkJ35546296 = zJnAudexSdFhugEkJ84661560;     zJnAudexSdFhugEkJ84661560 = zJnAudexSdFhugEkJ16312165;     zJnAudexSdFhugEkJ16312165 = zJnAudexSdFhugEkJ22006203;     zJnAudexSdFhugEkJ22006203 = zJnAudexSdFhugEkJ50352465;     zJnAudexSdFhugEkJ50352465 = zJnAudexSdFhugEkJ19077607;     zJnAudexSdFhugEkJ19077607 = zJnAudexSdFhugEkJ33630132;     zJnAudexSdFhugEkJ33630132 = zJnAudexSdFhugEkJ42620126;     zJnAudexSdFhugEkJ42620126 = zJnAudexSdFhugEkJ69508160;     zJnAudexSdFhugEkJ69508160 = zJnAudexSdFhugEkJ96392936;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hyHAhpMAzENtNLhG21150731() {     double piFLeUGptwmPgIToX31851905 = -808327441;    double piFLeUGptwmPgIToX52124704 = -479221532;    double piFLeUGptwmPgIToX43334438 = -935005898;    double piFLeUGptwmPgIToX67105115 = -238325862;    double piFLeUGptwmPgIToX34593263 = 74774655;    double piFLeUGptwmPgIToX37434817 = -768102766;    double piFLeUGptwmPgIToX35364437 = 22065671;    double piFLeUGptwmPgIToX27103917 = -409196140;    double piFLeUGptwmPgIToX37225501 = -651385864;    double piFLeUGptwmPgIToX76907234 = -464033101;    double piFLeUGptwmPgIToX84550569 = -335290374;    double piFLeUGptwmPgIToX27357904 = -10882661;    double piFLeUGptwmPgIToX39851361 = -937503332;    double piFLeUGptwmPgIToX98577649 = -850453277;    double piFLeUGptwmPgIToX53772058 = -289454797;    double piFLeUGptwmPgIToX42295451 = -664312101;    double piFLeUGptwmPgIToX45044365 = -257158657;    double piFLeUGptwmPgIToX86960105 = -538300686;    double piFLeUGptwmPgIToX60856024 = 33026972;    double piFLeUGptwmPgIToX3451931 = -98467602;    double piFLeUGptwmPgIToX9861583 = -533596120;    double piFLeUGptwmPgIToX45208321 = -188281707;    double piFLeUGptwmPgIToX15718133 = -635312365;    double piFLeUGptwmPgIToX5138743 = -164804000;    double piFLeUGptwmPgIToX19860649 = -789102222;    double piFLeUGptwmPgIToX49099779 = -764923457;    double piFLeUGptwmPgIToX53378388 = -400975428;    double piFLeUGptwmPgIToX77672757 = 91589012;    double piFLeUGptwmPgIToX72786607 = -385664064;    double piFLeUGptwmPgIToX45261932 = -801889252;    double piFLeUGptwmPgIToX94107898 = -234559519;    double piFLeUGptwmPgIToX81091908 = 91047326;    double piFLeUGptwmPgIToX58571343 = -339396870;    double piFLeUGptwmPgIToX58993544 = -303168954;    double piFLeUGptwmPgIToX4647162 = -821530154;    double piFLeUGptwmPgIToX35503052 = -190895353;    double piFLeUGptwmPgIToX79560275 = -476061291;    double piFLeUGptwmPgIToX18916026 = -769907008;    double piFLeUGptwmPgIToX86609112 = -838983561;    double piFLeUGptwmPgIToX94531865 = -153324948;    double piFLeUGptwmPgIToX71566771 = -912805381;    double piFLeUGptwmPgIToX47296697 = -919162471;    double piFLeUGptwmPgIToX18992885 = -303680258;    double piFLeUGptwmPgIToX69231319 = 88781448;    double piFLeUGptwmPgIToX72624563 = -366670270;    double piFLeUGptwmPgIToX86211339 = -899056055;    double piFLeUGptwmPgIToX28201314 = -607486284;    double piFLeUGptwmPgIToX99839041 = -361159568;    double piFLeUGptwmPgIToX87512109 = -598273291;    double piFLeUGptwmPgIToX60441962 = -120458092;    double piFLeUGptwmPgIToX47209906 = -490215335;    double piFLeUGptwmPgIToX93266054 = -438926059;    double piFLeUGptwmPgIToX20033601 = -635704500;    double piFLeUGptwmPgIToX63577354 = -950780579;    double piFLeUGptwmPgIToX11289149 = -705578954;    double piFLeUGptwmPgIToX86643584 = -520045734;    double piFLeUGptwmPgIToX36406572 = -843909168;    double piFLeUGptwmPgIToX38195695 = -670201899;    double piFLeUGptwmPgIToX47244466 = -449223641;    double piFLeUGptwmPgIToX85493483 = -160301888;    double piFLeUGptwmPgIToX84056428 = -267127339;    double piFLeUGptwmPgIToX57691679 = 30476658;    double piFLeUGptwmPgIToX54317310 = 76467923;    double piFLeUGptwmPgIToX91963569 = -849496612;    double piFLeUGptwmPgIToX82799335 = -129473583;    double piFLeUGptwmPgIToX3458661 = -326337700;    double piFLeUGptwmPgIToX68786561 = -671485792;    double piFLeUGptwmPgIToX80857816 = -534334379;    double piFLeUGptwmPgIToX93930487 = 71076877;    double piFLeUGptwmPgIToX18269006 = 1440556;    double piFLeUGptwmPgIToX62735175 = -88250811;    double piFLeUGptwmPgIToX26128339 = -487251649;    double piFLeUGptwmPgIToX350994 = -699317125;    double piFLeUGptwmPgIToX66324158 = -813648080;    double piFLeUGptwmPgIToX31885160 = -185662221;    double piFLeUGptwmPgIToX62564885 = -614433649;    double piFLeUGptwmPgIToX26215437 = -884601450;    double piFLeUGptwmPgIToX46486814 = -624093814;    double piFLeUGptwmPgIToX32514180 = -798133730;    double piFLeUGptwmPgIToX33649310 = -890046168;    double piFLeUGptwmPgIToX20898465 = -57437174;    double piFLeUGptwmPgIToX53539347 = 60184139;    double piFLeUGptwmPgIToX90160648 = -310137698;    double piFLeUGptwmPgIToX12344645 = -165205972;    double piFLeUGptwmPgIToX98052026 = -211673917;    double piFLeUGptwmPgIToX841845 = -795633460;    double piFLeUGptwmPgIToX61058308 = -273248175;    double piFLeUGptwmPgIToX94993989 = -388616291;    double piFLeUGptwmPgIToX47704395 = -597590000;    double piFLeUGptwmPgIToX18003578 = -201484421;    double piFLeUGptwmPgIToX99096480 = -346986185;    double piFLeUGptwmPgIToX41364581 = -805859393;    double piFLeUGptwmPgIToX71671559 = -220683368;    double piFLeUGptwmPgIToX1115629 = -578681673;    double piFLeUGptwmPgIToX10475437 = -886197610;    double piFLeUGptwmPgIToX13875092 = -843282040;    double piFLeUGptwmPgIToX92979387 = -895630394;    double piFLeUGptwmPgIToX27029316 = -454183646;    double piFLeUGptwmPgIToX86431983 = -781744969;    double piFLeUGptwmPgIToX69165902 = -808327441;     piFLeUGptwmPgIToX31851905 = piFLeUGptwmPgIToX52124704;     piFLeUGptwmPgIToX52124704 = piFLeUGptwmPgIToX43334438;     piFLeUGptwmPgIToX43334438 = piFLeUGptwmPgIToX67105115;     piFLeUGptwmPgIToX67105115 = piFLeUGptwmPgIToX34593263;     piFLeUGptwmPgIToX34593263 = piFLeUGptwmPgIToX37434817;     piFLeUGptwmPgIToX37434817 = piFLeUGptwmPgIToX35364437;     piFLeUGptwmPgIToX35364437 = piFLeUGptwmPgIToX27103917;     piFLeUGptwmPgIToX27103917 = piFLeUGptwmPgIToX37225501;     piFLeUGptwmPgIToX37225501 = piFLeUGptwmPgIToX76907234;     piFLeUGptwmPgIToX76907234 = piFLeUGptwmPgIToX84550569;     piFLeUGptwmPgIToX84550569 = piFLeUGptwmPgIToX27357904;     piFLeUGptwmPgIToX27357904 = piFLeUGptwmPgIToX39851361;     piFLeUGptwmPgIToX39851361 = piFLeUGptwmPgIToX98577649;     piFLeUGptwmPgIToX98577649 = piFLeUGptwmPgIToX53772058;     piFLeUGptwmPgIToX53772058 = piFLeUGptwmPgIToX42295451;     piFLeUGptwmPgIToX42295451 = piFLeUGptwmPgIToX45044365;     piFLeUGptwmPgIToX45044365 = piFLeUGptwmPgIToX86960105;     piFLeUGptwmPgIToX86960105 = piFLeUGptwmPgIToX60856024;     piFLeUGptwmPgIToX60856024 = piFLeUGptwmPgIToX3451931;     piFLeUGptwmPgIToX3451931 = piFLeUGptwmPgIToX9861583;     piFLeUGptwmPgIToX9861583 = piFLeUGptwmPgIToX45208321;     piFLeUGptwmPgIToX45208321 = piFLeUGptwmPgIToX15718133;     piFLeUGptwmPgIToX15718133 = piFLeUGptwmPgIToX5138743;     piFLeUGptwmPgIToX5138743 = piFLeUGptwmPgIToX19860649;     piFLeUGptwmPgIToX19860649 = piFLeUGptwmPgIToX49099779;     piFLeUGptwmPgIToX49099779 = piFLeUGptwmPgIToX53378388;     piFLeUGptwmPgIToX53378388 = piFLeUGptwmPgIToX77672757;     piFLeUGptwmPgIToX77672757 = piFLeUGptwmPgIToX72786607;     piFLeUGptwmPgIToX72786607 = piFLeUGptwmPgIToX45261932;     piFLeUGptwmPgIToX45261932 = piFLeUGptwmPgIToX94107898;     piFLeUGptwmPgIToX94107898 = piFLeUGptwmPgIToX81091908;     piFLeUGptwmPgIToX81091908 = piFLeUGptwmPgIToX58571343;     piFLeUGptwmPgIToX58571343 = piFLeUGptwmPgIToX58993544;     piFLeUGptwmPgIToX58993544 = piFLeUGptwmPgIToX4647162;     piFLeUGptwmPgIToX4647162 = piFLeUGptwmPgIToX35503052;     piFLeUGptwmPgIToX35503052 = piFLeUGptwmPgIToX79560275;     piFLeUGptwmPgIToX79560275 = piFLeUGptwmPgIToX18916026;     piFLeUGptwmPgIToX18916026 = piFLeUGptwmPgIToX86609112;     piFLeUGptwmPgIToX86609112 = piFLeUGptwmPgIToX94531865;     piFLeUGptwmPgIToX94531865 = piFLeUGptwmPgIToX71566771;     piFLeUGptwmPgIToX71566771 = piFLeUGptwmPgIToX47296697;     piFLeUGptwmPgIToX47296697 = piFLeUGptwmPgIToX18992885;     piFLeUGptwmPgIToX18992885 = piFLeUGptwmPgIToX69231319;     piFLeUGptwmPgIToX69231319 = piFLeUGptwmPgIToX72624563;     piFLeUGptwmPgIToX72624563 = piFLeUGptwmPgIToX86211339;     piFLeUGptwmPgIToX86211339 = piFLeUGptwmPgIToX28201314;     piFLeUGptwmPgIToX28201314 = piFLeUGptwmPgIToX99839041;     piFLeUGptwmPgIToX99839041 = piFLeUGptwmPgIToX87512109;     piFLeUGptwmPgIToX87512109 = piFLeUGptwmPgIToX60441962;     piFLeUGptwmPgIToX60441962 = piFLeUGptwmPgIToX47209906;     piFLeUGptwmPgIToX47209906 = piFLeUGptwmPgIToX93266054;     piFLeUGptwmPgIToX93266054 = piFLeUGptwmPgIToX20033601;     piFLeUGptwmPgIToX20033601 = piFLeUGptwmPgIToX63577354;     piFLeUGptwmPgIToX63577354 = piFLeUGptwmPgIToX11289149;     piFLeUGptwmPgIToX11289149 = piFLeUGptwmPgIToX86643584;     piFLeUGptwmPgIToX86643584 = piFLeUGptwmPgIToX36406572;     piFLeUGptwmPgIToX36406572 = piFLeUGptwmPgIToX38195695;     piFLeUGptwmPgIToX38195695 = piFLeUGptwmPgIToX47244466;     piFLeUGptwmPgIToX47244466 = piFLeUGptwmPgIToX85493483;     piFLeUGptwmPgIToX85493483 = piFLeUGptwmPgIToX84056428;     piFLeUGptwmPgIToX84056428 = piFLeUGptwmPgIToX57691679;     piFLeUGptwmPgIToX57691679 = piFLeUGptwmPgIToX54317310;     piFLeUGptwmPgIToX54317310 = piFLeUGptwmPgIToX91963569;     piFLeUGptwmPgIToX91963569 = piFLeUGptwmPgIToX82799335;     piFLeUGptwmPgIToX82799335 = piFLeUGptwmPgIToX3458661;     piFLeUGptwmPgIToX3458661 = piFLeUGptwmPgIToX68786561;     piFLeUGptwmPgIToX68786561 = piFLeUGptwmPgIToX80857816;     piFLeUGptwmPgIToX80857816 = piFLeUGptwmPgIToX93930487;     piFLeUGptwmPgIToX93930487 = piFLeUGptwmPgIToX18269006;     piFLeUGptwmPgIToX18269006 = piFLeUGptwmPgIToX62735175;     piFLeUGptwmPgIToX62735175 = piFLeUGptwmPgIToX26128339;     piFLeUGptwmPgIToX26128339 = piFLeUGptwmPgIToX350994;     piFLeUGptwmPgIToX350994 = piFLeUGptwmPgIToX66324158;     piFLeUGptwmPgIToX66324158 = piFLeUGptwmPgIToX31885160;     piFLeUGptwmPgIToX31885160 = piFLeUGptwmPgIToX62564885;     piFLeUGptwmPgIToX62564885 = piFLeUGptwmPgIToX26215437;     piFLeUGptwmPgIToX26215437 = piFLeUGptwmPgIToX46486814;     piFLeUGptwmPgIToX46486814 = piFLeUGptwmPgIToX32514180;     piFLeUGptwmPgIToX32514180 = piFLeUGptwmPgIToX33649310;     piFLeUGptwmPgIToX33649310 = piFLeUGptwmPgIToX20898465;     piFLeUGptwmPgIToX20898465 = piFLeUGptwmPgIToX53539347;     piFLeUGptwmPgIToX53539347 = piFLeUGptwmPgIToX90160648;     piFLeUGptwmPgIToX90160648 = piFLeUGptwmPgIToX12344645;     piFLeUGptwmPgIToX12344645 = piFLeUGptwmPgIToX98052026;     piFLeUGptwmPgIToX98052026 = piFLeUGptwmPgIToX841845;     piFLeUGptwmPgIToX841845 = piFLeUGptwmPgIToX61058308;     piFLeUGptwmPgIToX61058308 = piFLeUGptwmPgIToX94993989;     piFLeUGptwmPgIToX94993989 = piFLeUGptwmPgIToX47704395;     piFLeUGptwmPgIToX47704395 = piFLeUGptwmPgIToX18003578;     piFLeUGptwmPgIToX18003578 = piFLeUGptwmPgIToX99096480;     piFLeUGptwmPgIToX99096480 = piFLeUGptwmPgIToX41364581;     piFLeUGptwmPgIToX41364581 = piFLeUGptwmPgIToX71671559;     piFLeUGptwmPgIToX71671559 = piFLeUGptwmPgIToX1115629;     piFLeUGptwmPgIToX1115629 = piFLeUGptwmPgIToX10475437;     piFLeUGptwmPgIToX10475437 = piFLeUGptwmPgIToX13875092;     piFLeUGptwmPgIToX13875092 = piFLeUGptwmPgIToX92979387;     piFLeUGptwmPgIToX92979387 = piFLeUGptwmPgIToX27029316;     piFLeUGptwmPgIToX27029316 = piFLeUGptwmPgIToX86431983;     piFLeUGptwmPgIToX86431983 = piFLeUGptwmPgIToX69165902;     piFLeUGptwmPgIToX69165902 = piFLeUGptwmPgIToX31851905;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void CSQkgyWBEgGkJhbA36199799() {     double EkXWvFRuVuaEdyRRg37969012 = -920229330;    double EkXWvFRuVuaEdyRRg95497983 = -188821313;    double EkXWvFRuVuaEdyRRg59376621 = 14161181;    double EkXWvFRuVuaEdyRRg94179896 = -429074492;    double EkXWvFRuVuaEdyRRg3488751 = -445875216;    double EkXWvFRuVuaEdyRRg28852019 = 67400998;    double EkXWvFRuVuaEdyRRg95853403 = -116899306;    double EkXWvFRuVuaEdyRRg76863808 = -696070054;    double EkXWvFRuVuaEdyRRg1867997 = -419418529;    double EkXWvFRuVuaEdyRRg18136894 = -793008579;    double EkXWvFRuVuaEdyRRg44660860 = -212727961;    double EkXWvFRuVuaEdyRRg59180947 = -455288863;    double EkXWvFRuVuaEdyRRg26060480 = -277394420;    double EkXWvFRuVuaEdyRRg81721325 = -282691845;    double EkXWvFRuVuaEdyRRg49514485 = -979916963;    double EkXWvFRuVuaEdyRRg83800844 = -637311645;    double EkXWvFRuVuaEdyRRg38880368 = -873703770;    double EkXWvFRuVuaEdyRRg84013258 = -203823328;    double EkXWvFRuVuaEdyRRg22685866 = -372742125;    double EkXWvFRuVuaEdyRRg84002095 = -455370521;    double EkXWvFRuVuaEdyRRg98414775 = -389083575;    double EkXWvFRuVuaEdyRRg31729345 = -96211424;    double EkXWvFRuVuaEdyRRg51308924 = -381829852;    double EkXWvFRuVuaEdyRRg89131116 = -387108988;    double EkXWvFRuVuaEdyRRg63560299 = -62298255;    double EkXWvFRuVuaEdyRRg73821454 = -643578781;    double EkXWvFRuVuaEdyRRg51838167 = -301637631;    double EkXWvFRuVuaEdyRRg90779882 = 25660058;    double EkXWvFRuVuaEdyRRg38918863 = -608046768;    double EkXWvFRuVuaEdyRRg13877569 = -619360119;    double EkXWvFRuVuaEdyRRg86350448 = -147880457;    double EkXWvFRuVuaEdyRRg78702530 = -718399490;    double EkXWvFRuVuaEdyRRg68282291 = -232010240;    double EkXWvFRuVuaEdyRRg53824108 = -54710719;    double EkXWvFRuVuaEdyRRg66121814 = -360263260;    double EkXWvFRuVuaEdyRRg4228966 = -404921460;    double EkXWvFRuVuaEdyRRg60822210 = -429697161;    double EkXWvFRuVuaEdyRRg53006238 = -541413018;    double EkXWvFRuVuaEdyRRg60219366 = -832907058;    double EkXWvFRuVuaEdyRRg82033803 = -626764171;    double EkXWvFRuVuaEdyRRg77235995 = -978079275;    double EkXWvFRuVuaEdyRRg79447154 = -89304749;    double EkXWvFRuVuaEdyRRg97059862 = -384484842;    double EkXWvFRuVuaEdyRRg56639059 = 69438510;    double EkXWvFRuVuaEdyRRg84577028 = -28568424;    double EkXWvFRuVuaEdyRRg98150534 = -884272171;    double EkXWvFRuVuaEdyRRg45979888 = -726261821;    double EkXWvFRuVuaEdyRRg7795983 = -371055100;    double EkXWvFRuVuaEdyRRg97187838 = -684010279;    double EkXWvFRuVuaEdyRRg73353615 = -207468900;    double EkXWvFRuVuaEdyRRg82910584 = -332509007;    double EkXWvFRuVuaEdyRRg4075774 = -239652379;    double EkXWvFRuVuaEdyRRg68282058 = -427494088;    double EkXWvFRuVuaEdyRRg59546234 = -171250724;    double EkXWvFRuVuaEdyRRg37785024 = -306110736;    double EkXWvFRuVuaEdyRRg6239668 = -724017907;    double EkXWvFRuVuaEdyRRg44189059 = -806991462;    double EkXWvFRuVuaEdyRRg70245504 = -598729832;    double EkXWvFRuVuaEdyRRg30619597 = -266776238;    double EkXWvFRuVuaEdyRRg29667297 = -802296436;    double EkXWvFRuVuaEdyRRg77013852 = -630961371;    double EkXWvFRuVuaEdyRRg5073521 = -42559365;    double EkXWvFRuVuaEdyRRg37944946 = 11976714;    double EkXWvFRuVuaEdyRRg87990427 = -800058410;    double EkXWvFRuVuaEdyRRg31786445 = -545128123;    double EkXWvFRuVuaEdyRRg65958329 = -494328471;    double EkXWvFRuVuaEdyRRg90898655 = -123278624;    double EkXWvFRuVuaEdyRRg72236371 = -122683702;    double EkXWvFRuVuaEdyRRg15599512 = -922428585;    double EkXWvFRuVuaEdyRRg45285519 = -474995503;    double EkXWvFRuVuaEdyRRg22978635 = -107614484;    double EkXWvFRuVuaEdyRRg85874129 = -232290753;    double EkXWvFRuVuaEdyRRg23793893 = -370916271;    double EkXWvFRuVuaEdyRRg40652063 = -745977955;    double EkXWvFRuVuaEdyRRg6766101 = -477291246;    double EkXWvFRuVuaEdyRRg18967622 = -199778826;    double EkXWvFRuVuaEdyRRg34669482 = -711726582;    double EkXWvFRuVuaEdyRRg94669865 = -351268362;    double EkXWvFRuVuaEdyRRg4554089 = -258540564;    double EkXWvFRuVuaEdyRRg65409765 = -178026084;    double EkXWvFRuVuaEdyRRg27841566 = -917316960;    double EkXWvFRuVuaEdyRRg44042184 = -930582532;    double EkXWvFRuVuaEdyRRg93592044 = -290329664;    double EkXWvFRuVuaEdyRRg65565248 = -300577869;    double EkXWvFRuVuaEdyRRg30966985 = -186851113;    double EkXWvFRuVuaEdyRRg82274674 = -908228078;    double EkXWvFRuVuaEdyRRg10420473 = -190905403;    double EkXWvFRuVuaEdyRRg8736058 = 39240484;    double EkXWvFRuVuaEdyRRg16039085 = -748599983;    double EkXWvFRuVuaEdyRRg59882147 = -636245354;    double EkXWvFRuVuaEdyRRg60039906 = -597929999;    double EkXWvFRuVuaEdyRRg90576705 = -830967329;    double EkXWvFRuVuaEdyRRg22386642 = -174636780;    double EkXWvFRuVuaEdyRRg30552069 = 69389378;    double EkXWvFRuVuaEdyRRg5019951 = -995802801;    double EkXWvFRuVuaEdyRRg72162475 = -835519911;    double EkXWvFRuVuaEdyRRg41502209 = -1281464;    double EkXWvFRuVuaEdyRRg9069436 = -584426432;    double EkXWvFRuVuaEdyRRg24852614 = -385433368;    double EkXWvFRuVuaEdyRRg18618699 = -920229330;     EkXWvFRuVuaEdyRRg37969012 = EkXWvFRuVuaEdyRRg95497983;     EkXWvFRuVuaEdyRRg95497983 = EkXWvFRuVuaEdyRRg59376621;     EkXWvFRuVuaEdyRRg59376621 = EkXWvFRuVuaEdyRRg94179896;     EkXWvFRuVuaEdyRRg94179896 = EkXWvFRuVuaEdyRRg3488751;     EkXWvFRuVuaEdyRRg3488751 = EkXWvFRuVuaEdyRRg28852019;     EkXWvFRuVuaEdyRRg28852019 = EkXWvFRuVuaEdyRRg95853403;     EkXWvFRuVuaEdyRRg95853403 = EkXWvFRuVuaEdyRRg76863808;     EkXWvFRuVuaEdyRRg76863808 = EkXWvFRuVuaEdyRRg1867997;     EkXWvFRuVuaEdyRRg1867997 = EkXWvFRuVuaEdyRRg18136894;     EkXWvFRuVuaEdyRRg18136894 = EkXWvFRuVuaEdyRRg44660860;     EkXWvFRuVuaEdyRRg44660860 = EkXWvFRuVuaEdyRRg59180947;     EkXWvFRuVuaEdyRRg59180947 = EkXWvFRuVuaEdyRRg26060480;     EkXWvFRuVuaEdyRRg26060480 = EkXWvFRuVuaEdyRRg81721325;     EkXWvFRuVuaEdyRRg81721325 = EkXWvFRuVuaEdyRRg49514485;     EkXWvFRuVuaEdyRRg49514485 = EkXWvFRuVuaEdyRRg83800844;     EkXWvFRuVuaEdyRRg83800844 = EkXWvFRuVuaEdyRRg38880368;     EkXWvFRuVuaEdyRRg38880368 = EkXWvFRuVuaEdyRRg84013258;     EkXWvFRuVuaEdyRRg84013258 = EkXWvFRuVuaEdyRRg22685866;     EkXWvFRuVuaEdyRRg22685866 = EkXWvFRuVuaEdyRRg84002095;     EkXWvFRuVuaEdyRRg84002095 = EkXWvFRuVuaEdyRRg98414775;     EkXWvFRuVuaEdyRRg98414775 = EkXWvFRuVuaEdyRRg31729345;     EkXWvFRuVuaEdyRRg31729345 = EkXWvFRuVuaEdyRRg51308924;     EkXWvFRuVuaEdyRRg51308924 = EkXWvFRuVuaEdyRRg89131116;     EkXWvFRuVuaEdyRRg89131116 = EkXWvFRuVuaEdyRRg63560299;     EkXWvFRuVuaEdyRRg63560299 = EkXWvFRuVuaEdyRRg73821454;     EkXWvFRuVuaEdyRRg73821454 = EkXWvFRuVuaEdyRRg51838167;     EkXWvFRuVuaEdyRRg51838167 = EkXWvFRuVuaEdyRRg90779882;     EkXWvFRuVuaEdyRRg90779882 = EkXWvFRuVuaEdyRRg38918863;     EkXWvFRuVuaEdyRRg38918863 = EkXWvFRuVuaEdyRRg13877569;     EkXWvFRuVuaEdyRRg13877569 = EkXWvFRuVuaEdyRRg86350448;     EkXWvFRuVuaEdyRRg86350448 = EkXWvFRuVuaEdyRRg78702530;     EkXWvFRuVuaEdyRRg78702530 = EkXWvFRuVuaEdyRRg68282291;     EkXWvFRuVuaEdyRRg68282291 = EkXWvFRuVuaEdyRRg53824108;     EkXWvFRuVuaEdyRRg53824108 = EkXWvFRuVuaEdyRRg66121814;     EkXWvFRuVuaEdyRRg66121814 = EkXWvFRuVuaEdyRRg4228966;     EkXWvFRuVuaEdyRRg4228966 = EkXWvFRuVuaEdyRRg60822210;     EkXWvFRuVuaEdyRRg60822210 = EkXWvFRuVuaEdyRRg53006238;     EkXWvFRuVuaEdyRRg53006238 = EkXWvFRuVuaEdyRRg60219366;     EkXWvFRuVuaEdyRRg60219366 = EkXWvFRuVuaEdyRRg82033803;     EkXWvFRuVuaEdyRRg82033803 = EkXWvFRuVuaEdyRRg77235995;     EkXWvFRuVuaEdyRRg77235995 = EkXWvFRuVuaEdyRRg79447154;     EkXWvFRuVuaEdyRRg79447154 = EkXWvFRuVuaEdyRRg97059862;     EkXWvFRuVuaEdyRRg97059862 = EkXWvFRuVuaEdyRRg56639059;     EkXWvFRuVuaEdyRRg56639059 = EkXWvFRuVuaEdyRRg84577028;     EkXWvFRuVuaEdyRRg84577028 = EkXWvFRuVuaEdyRRg98150534;     EkXWvFRuVuaEdyRRg98150534 = EkXWvFRuVuaEdyRRg45979888;     EkXWvFRuVuaEdyRRg45979888 = EkXWvFRuVuaEdyRRg7795983;     EkXWvFRuVuaEdyRRg7795983 = EkXWvFRuVuaEdyRRg97187838;     EkXWvFRuVuaEdyRRg97187838 = EkXWvFRuVuaEdyRRg73353615;     EkXWvFRuVuaEdyRRg73353615 = EkXWvFRuVuaEdyRRg82910584;     EkXWvFRuVuaEdyRRg82910584 = EkXWvFRuVuaEdyRRg4075774;     EkXWvFRuVuaEdyRRg4075774 = EkXWvFRuVuaEdyRRg68282058;     EkXWvFRuVuaEdyRRg68282058 = EkXWvFRuVuaEdyRRg59546234;     EkXWvFRuVuaEdyRRg59546234 = EkXWvFRuVuaEdyRRg37785024;     EkXWvFRuVuaEdyRRg37785024 = EkXWvFRuVuaEdyRRg6239668;     EkXWvFRuVuaEdyRRg6239668 = EkXWvFRuVuaEdyRRg44189059;     EkXWvFRuVuaEdyRRg44189059 = EkXWvFRuVuaEdyRRg70245504;     EkXWvFRuVuaEdyRRg70245504 = EkXWvFRuVuaEdyRRg30619597;     EkXWvFRuVuaEdyRRg30619597 = EkXWvFRuVuaEdyRRg29667297;     EkXWvFRuVuaEdyRRg29667297 = EkXWvFRuVuaEdyRRg77013852;     EkXWvFRuVuaEdyRRg77013852 = EkXWvFRuVuaEdyRRg5073521;     EkXWvFRuVuaEdyRRg5073521 = EkXWvFRuVuaEdyRRg37944946;     EkXWvFRuVuaEdyRRg37944946 = EkXWvFRuVuaEdyRRg87990427;     EkXWvFRuVuaEdyRRg87990427 = EkXWvFRuVuaEdyRRg31786445;     EkXWvFRuVuaEdyRRg31786445 = EkXWvFRuVuaEdyRRg65958329;     EkXWvFRuVuaEdyRRg65958329 = EkXWvFRuVuaEdyRRg90898655;     EkXWvFRuVuaEdyRRg90898655 = EkXWvFRuVuaEdyRRg72236371;     EkXWvFRuVuaEdyRRg72236371 = EkXWvFRuVuaEdyRRg15599512;     EkXWvFRuVuaEdyRRg15599512 = EkXWvFRuVuaEdyRRg45285519;     EkXWvFRuVuaEdyRRg45285519 = EkXWvFRuVuaEdyRRg22978635;     EkXWvFRuVuaEdyRRg22978635 = EkXWvFRuVuaEdyRRg85874129;     EkXWvFRuVuaEdyRRg85874129 = EkXWvFRuVuaEdyRRg23793893;     EkXWvFRuVuaEdyRRg23793893 = EkXWvFRuVuaEdyRRg40652063;     EkXWvFRuVuaEdyRRg40652063 = EkXWvFRuVuaEdyRRg6766101;     EkXWvFRuVuaEdyRRg6766101 = EkXWvFRuVuaEdyRRg18967622;     EkXWvFRuVuaEdyRRg18967622 = EkXWvFRuVuaEdyRRg34669482;     EkXWvFRuVuaEdyRRg34669482 = EkXWvFRuVuaEdyRRg94669865;     EkXWvFRuVuaEdyRRg94669865 = EkXWvFRuVuaEdyRRg4554089;     EkXWvFRuVuaEdyRRg4554089 = EkXWvFRuVuaEdyRRg65409765;     EkXWvFRuVuaEdyRRg65409765 = EkXWvFRuVuaEdyRRg27841566;     EkXWvFRuVuaEdyRRg27841566 = EkXWvFRuVuaEdyRRg44042184;     EkXWvFRuVuaEdyRRg44042184 = EkXWvFRuVuaEdyRRg93592044;     EkXWvFRuVuaEdyRRg93592044 = EkXWvFRuVuaEdyRRg65565248;     EkXWvFRuVuaEdyRRg65565248 = EkXWvFRuVuaEdyRRg30966985;     EkXWvFRuVuaEdyRRg30966985 = EkXWvFRuVuaEdyRRg82274674;     EkXWvFRuVuaEdyRRg82274674 = EkXWvFRuVuaEdyRRg10420473;     EkXWvFRuVuaEdyRRg10420473 = EkXWvFRuVuaEdyRRg8736058;     EkXWvFRuVuaEdyRRg8736058 = EkXWvFRuVuaEdyRRg16039085;     EkXWvFRuVuaEdyRRg16039085 = EkXWvFRuVuaEdyRRg59882147;     EkXWvFRuVuaEdyRRg59882147 = EkXWvFRuVuaEdyRRg60039906;     EkXWvFRuVuaEdyRRg60039906 = EkXWvFRuVuaEdyRRg90576705;     EkXWvFRuVuaEdyRRg90576705 = EkXWvFRuVuaEdyRRg22386642;     EkXWvFRuVuaEdyRRg22386642 = EkXWvFRuVuaEdyRRg30552069;     EkXWvFRuVuaEdyRRg30552069 = EkXWvFRuVuaEdyRRg5019951;     EkXWvFRuVuaEdyRRg5019951 = EkXWvFRuVuaEdyRRg72162475;     EkXWvFRuVuaEdyRRg72162475 = EkXWvFRuVuaEdyRRg41502209;     EkXWvFRuVuaEdyRRg41502209 = EkXWvFRuVuaEdyRRg9069436;     EkXWvFRuVuaEdyRRg9069436 = EkXWvFRuVuaEdyRRg24852614;     EkXWvFRuVuaEdyRRg24852614 = EkXWvFRuVuaEdyRRg18618699;     EkXWvFRuVuaEdyRRg18618699 = EkXWvFRuVuaEdyRRg37969012;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LyNvsMegnVjBjpRy3491399() {     double nsPfHMUKEKCNLmnJl73427980 = -286506230;    double nsPfHMUKEKCNLmnJl8770063 = -69372920;    double nsPfHMUKEKCNLmnJl96763098 = -751505894;    double nsPfHMUKEKCNLmnJl66403251 = -250225564;    double nsPfHMUKEKCNLmnJl59774020 = -551474062;    double nsPfHMUKEKCNLmnJl65074891 = -194946897;    double nsPfHMUKEKCNLmnJl6510810 = 38374959;    double nsPfHMUKEKCNLmnJl38995369 = -340304194;    double nsPfHMUKEKCNLmnJl3385602 = -907749936;    double nsPfHMUKEKCNLmnJl33039606 = -457695577;    double nsPfHMUKEKCNLmnJl82842594 = -14735690;    double nsPfHMUKEKCNLmnJl59749095 = -319409635;    double nsPfHMUKEKCNLmnJl64295125 = -829406925;    double nsPfHMUKEKCNLmnJl34862189 = -259055254;    double nsPfHMUKEKCNLmnJl7709002 = -383341978;    double nsPfHMUKEKCNLmnJl67342145 = -273123252;    double nsPfHMUKEKCNLmnJl94716303 = -300678323;    double nsPfHMUKEKCNLmnJl83773450 = -277251397;    double nsPfHMUKEKCNLmnJl23236196 = -689305547;    double nsPfHMUKEKCNLmnJl77839314 = -881931587;    double nsPfHMUKEKCNLmnJl11501639 = -46358264;    double nsPfHMUKEKCNLmnJl35150869 = -182847718;    double nsPfHMUKEKCNLmnJl21834305 = -19462036;    double nsPfHMUKEKCNLmnJl82924062 = -897933592;    double nsPfHMUKEKCNLmnJl71578250 = -86283453;    double nsPfHMUKEKCNLmnJl84519626 = -332155588;    double nsPfHMUKEKCNLmnJl12258741 = -107921772;    double nsPfHMUKEKCNLmnJl31508308 = -144112116;    double nsPfHMUKEKCNLmnJl85587077 = -82141571;    double nsPfHMUKEKCNLmnJl19410980 = -653867290;    double nsPfHMUKEKCNLmnJl4933680 = -699860944;    double nsPfHMUKEKCNLmnJl16525549 = -491370904;    double nsPfHMUKEKCNLmnJl80500302 = -379606217;    double nsPfHMUKEKCNLmnJl83572535 = -119404532;    double nsPfHMUKEKCNLmnJl31930268 = -311363510;    double nsPfHMUKEKCNLmnJl55789809 = -619486766;    double nsPfHMUKEKCNLmnJl10234027 = -294393201;    double nsPfHMUKEKCNLmnJl4221642 = -524068599;    double nsPfHMUKEKCNLmnJl90609927 = -483783334;    double nsPfHMUKEKCNLmnJl46305335 = -92026717;    double nsPfHMUKEKCNLmnJl10687603 = -747278190;    double nsPfHMUKEKCNLmnJl68812281 = -783850040;    double nsPfHMUKEKCNLmnJl86443241 = 62334372;    double nsPfHMUKEKCNLmnJl96770397 = -534893900;    double nsPfHMUKEKCNLmnJl84593487 = -955079586;    double nsPfHMUKEKCNLmnJl44111863 = -923195803;    double nsPfHMUKEKCNLmnJl65325052 = -110248182;    double nsPfHMUKEKCNLmnJl25522295 = -537581693;    double nsPfHMUKEKCNLmnJl20080505 = -347747159;    double nsPfHMUKEKCNLmnJl97883204 = -698495716;    double nsPfHMUKEKCNLmnJl63681308 = -206785564;    double nsPfHMUKEKCNLmnJl87561246 = -453304726;    double nsPfHMUKEKCNLmnJl36903207 = -183539641;    double nsPfHMUKEKCNLmnJl75538081 = -362759224;    double nsPfHMUKEKCNLmnJl91989759 = -652600882;    double nsPfHMUKEKCNLmnJl38277111 = -3658513;    double nsPfHMUKEKCNLmnJl86935758 = 50089116;    double nsPfHMUKEKCNLmnJl13839037 = -853572303;    double nsPfHMUKEKCNLmnJl94825000 = -63942111;    double nsPfHMUKEKCNLmnJl75254394 = -119318475;    double nsPfHMUKEKCNLmnJl52816150 = 12974875;    double nsPfHMUKEKCNLmnJl75002501 = -817512925;    double nsPfHMUKEKCNLmnJl53408292 = -158162624;    double nsPfHMUKEKCNLmnJl83974622 = -153882647;    double nsPfHMUKEKCNLmnJl28105926 = -757834634;    double nsPfHMUKEKCNLmnJl66317046 = -523364786;    double nsPfHMUKEKCNLmnJl79248792 = -939803418;    double nsPfHMUKEKCNLmnJl80722590 = -610002394;    double nsPfHMUKEKCNLmnJl2931922 = -947691744;    double nsPfHMUKEKCNLmnJl51919192 = -763855212;    double nsPfHMUKEKCNLmnJl57108119 = -978730051;    double nsPfHMUKEKCNLmnJl90494662 = -776609724;    double nsPfHMUKEKCNLmnJl93163522 = -793468063;    double nsPfHMUKEKCNLmnJl76930860 = -497278830;    double nsPfHMUKEKCNLmnJl67151711 = -34653398;    double nsPfHMUKEKCNLmnJl42689358 = -262508224;    double nsPfHMUKEKCNLmnJl48707628 = -145182090;    double nsPfHMUKEKCNLmnJl25063907 = -484568137;    double nsPfHMUKEKCNLmnJl98330574 = -942854007;    double nsPfHMUKEKCNLmnJl27466388 = -163087651;    double nsPfHMUKEKCNLmnJl19194574 = -121907406;    double nsPfHMUKEKCNLmnJl86736446 = -570340079;    double nsPfHMUKEKCNLmnJl11427804 = -796364958;    double nsPfHMUKEKCNLmnJl87703873 = -383645855;    double nsPfHMUKEKCNLmnJl55729671 = -347081726;    double nsPfHMUKEKCNLmnJl17372434 = -146556219;    double nsPfHMUKEKCNLmnJl79622342 = -207831263;    double nsPfHMUKEKCNLmnJl4962222 = 83153006;    double nsPfHMUKEKCNLmnJl91582775 = -466803651;    double nsPfHMUKEKCNLmnJl93653156 = -207704998;    double nsPfHMUKEKCNLmnJl68854051 = -569575882;    double nsPfHMUKEKCNLmnJl96394990 = -440820899;    double nsPfHMUKEKCNLmnJl9396642 = -360126488;    double nsPfHMUKEKCNLmnJl15355533 = -264464860;    double nsPfHMUKEKCNLmnJl93489184 = -5001592;    double nsPfHMUKEKCNLmnJl35685101 = -929765265;    double nsPfHMUKEKCNLmnJl15403989 = -525687417;    double nsPfHMUKEKCNLmnJl2468620 = -783782982;    double nsPfHMUKEKCNLmnJl68664472 = -777059267;    double nsPfHMUKEKCNLmnJl18276442 = -286506230;     nsPfHMUKEKCNLmnJl73427980 = nsPfHMUKEKCNLmnJl8770063;     nsPfHMUKEKCNLmnJl8770063 = nsPfHMUKEKCNLmnJl96763098;     nsPfHMUKEKCNLmnJl96763098 = nsPfHMUKEKCNLmnJl66403251;     nsPfHMUKEKCNLmnJl66403251 = nsPfHMUKEKCNLmnJl59774020;     nsPfHMUKEKCNLmnJl59774020 = nsPfHMUKEKCNLmnJl65074891;     nsPfHMUKEKCNLmnJl65074891 = nsPfHMUKEKCNLmnJl6510810;     nsPfHMUKEKCNLmnJl6510810 = nsPfHMUKEKCNLmnJl38995369;     nsPfHMUKEKCNLmnJl38995369 = nsPfHMUKEKCNLmnJl3385602;     nsPfHMUKEKCNLmnJl3385602 = nsPfHMUKEKCNLmnJl33039606;     nsPfHMUKEKCNLmnJl33039606 = nsPfHMUKEKCNLmnJl82842594;     nsPfHMUKEKCNLmnJl82842594 = nsPfHMUKEKCNLmnJl59749095;     nsPfHMUKEKCNLmnJl59749095 = nsPfHMUKEKCNLmnJl64295125;     nsPfHMUKEKCNLmnJl64295125 = nsPfHMUKEKCNLmnJl34862189;     nsPfHMUKEKCNLmnJl34862189 = nsPfHMUKEKCNLmnJl7709002;     nsPfHMUKEKCNLmnJl7709002 = nsPfHMUKEKCNLmnJl67342145;     nsPfHMUKEKCNLmnJl67342145 = nsPfHMUKEKCNLmnJl94716303;     nsPfHMUKEKCNLmnJl94716303 = nsPfHMUKEKCNLmnJl83773450;     nsPfHMUKEKCNLmnJl83773450 = nsPfHMUKEKCNLmnJl23236196;     nsPfHMUKEKCNLmnJl23236196 = nsPfHMUKEKCNLmnJl77839314;     nsPfHMUKEKCNLmnJl77839314 = nsPfHMUKEKCNLmnJl11501639;     nsPfHMUKEKCNLmnJl11501639 = nsPfHMUKEKCNLmnJl35150869;     nsPfHMUKEKCNLmnJl35150869 = nsPfHMUKEKCNLmnJl21834305;     nsPfHMUKEKCNLmnJl21834305 = nsPfHMUKEKCNLmnJl82924062;     nsPfHMUKEKCNLmnJl82924062 = nsPfHMUKEKCNLmnJl71578250;     nsPfHMUKEKCNLmnJl71578250 = nsPfHMUKEKCNLmnJl84519626;     nsPfHMUKEKCNLmnJl84519626 = nsPfHMUKEKCNLmnJl12258741;     nsPfHMUKEKCNLmnJl12258741 = nsPfHMUKEKCNLmnJl31508308;     nsPfHMUKEKCNLmnJl31508308 = nsPfHMUKEKCNLmnJl85587077;     nsPfHMUKEKCNLmnJl85587077 = nsPfHMUKEKCNLmnJl19410980;     nsPfHMUKEKCNLmnJl19410980 = nsPfHMUKEKCNLmnJl4933680;     nsPfHMUKEKCNLmnJl4933680 = nsPfHMUKEKCNLmnJl16525549;     nsPfHMUKEKCNLmnJl16525549 = nsPfHMUKEKCNLmnJl80500302;     nsPfHMUKEKCNLmnJl80500302 = nsPfHMUKEKCNLmnJl83572535;     nsPfHMUKEKCNLmnJl83572535 = nsPfHMUKEKCNLmnJl31930268;     nsPfHMUKEKCNLmnJl31930268 = nsPfHMUKEKCNLmnJl55789809;     nsPfHMUKEKCNLmnJl55789809 = nsPfHMUKEKCNLmnJl10234027;     nsPfHMUKEKCNLmnJl10234027 = nsPfHMUKEKCNLmnJl4221642;     nsPfHMUKEKCNLmnJl4221642 = nsPfHMUKEKCNLmnJl90609927;     nsPfHMUKEKCNLmnJl90609927 = nsPfHMUKEKCNLmnJl46305335;     nsPfHMUKEKCNLmnJl46305335 = nsPfHMUKEKCNLmnJl10687603;     nsPfHMUKEKCNLmnJl10687603 = nsPfHMUKEKCNLmnJl68812281;     nsPfHMUKEKCNLmnJl68812281 = nsPfHMUKEKCNLmnJl86443241;     nsPfHMUKEKCNLmnJl86443241 = nsPfHMUKEKCNLmnJl96770397;     nsPfHMUKEKCNLmnJl96770397 = nsPfHMUKEKCNLmnJl84593487;     nsPfHMUKEKCNLmnJl84593487 = nsPfHMUKEKCNLmnJl44111863;     nsPfHMUKEKCNLmnJl44111863 = nsPfHMUKEKCNLmnJl65325052;     nsPfHMUKEKCNLmnJl65325052 = nsPfHMUKEKCNLmnJl25522295;     nsPfHMUKEKCNLmnJl25522295 = nsPfHMUKEKCNLmnJl20080505;     nsPfHMUKEKCNLmnJl20080505 = nsPfHMUKEKCNLmnJl97883204;     nsPfHMUKEKCNLmnJl97883204 = nsPfHMUKEKCNLmnJl63681308;     nsPfHMUKEKCNLmnJl63681308 = nsPfHMUKEKCNLmnJl87561246;     nsPfHMUKEKCNLmnJl87561246 = nsPfHMUKEKCNLmnJl36903207;     nsPfHMUKEKCNLmnJl36903207 = nsPfHMUKEKCNLmnJl75538081;     nsPfHMUKEKCNLmnJl75538081 = nsPfHMUKEKCNLmnJl91989759;     nsPfHMUKEKCNLmnJl91989759 = nsPfHMUKEKCNLmnJl38277111;     nsPfHMUKEKCNLmnJl38277111 = nsPfHMUKEKCNLmnJl86935758;     nsPfHMUKEKCNLmnJl86935758 = nsPfHMUKEKCNLmnJl13839037;     nsPfHMUKEKCNLmnJl13839037 = nsPfHMUKEKCNLmnJl94825000;     nsPfHMUKEKCNLmnJl94825000 = nsPfHMUKEKCNLmnJl75254394;     nsPfHMUKEKCNLmnJl75254394 = nsPfHMUKEKCNLmnJl52816150;     nsPfHMUKEKCNLmnJl52816150 = nsPfHMUKEKCNLmnJl75002501;     nsPfHMUKEKCNLmnJl75002501 = nsPfHMUKEKCNLmnJl53408292;     nsPfHMUKEKCNLmnJl53408292 = nsPfHMUKEKCNLmnJl83974622;     nsPfHMUKEKCNLmnJl83974622 = nsPfHMUKEKCNLmnJl28105926;     nsPfHMUKEKCNLmnJl28105926 = nsPfHMUKEKCNLmnJl66317046;     nsPfHMUKEKCNLmnJl66317046 = nsPfHMUKEKCNLmnJl79248792;     nsPfHMUKEKCNLmnJl79248792 = nsPfHMUKEKCNLmnJl80722590;     nsPfHMUKEKCNLmnJl80722590 = nsPfHMUKEKCNLmnJl2931922;     nsPfHMUKEKCNLmnJl2931922 = nsPfHMUKEKCNLmnJl51919192;     nsPfHMUKEKCNLmnJl51919192 = nsPfHMUKEKCNLmnJl57108119;     nsPfHMUKEKCNLmnJl57108119 = nsPfHMUKEKCNLmnJl90494662;     nsPfHMUKEKCNLmnJl90494662 = nsPfHMUKEKCNLmnJl93163522;     nsPfHMUKEKCNLmnJl93163522 = nsPfHMUKEKCNLmnJl76930860;     nsPfHMUKEKCNLmnJl76930860 = nsPfHMUKEKCNLmnJl67151711;     nsPfHMUKEKCNLmnJl67151711 = nsPfHMUKEKCNLmnJl42689358;     nsPfHMUKEKCNLmnJl42689358 = nsPfHMUKEKCNLmnJl48707628;     nsPfHMUKEKCNLmnJl48707628 = nsPfHMUKEKCNLmnJl25063907;     nsPfHMUKEKCNLmnJl25063907 = nsPfHMUKEKCNLmnJl98330574;     nsPfHMUKEKCNLmnJl98330574 = nsPfHMUKEKCNLmnJl27466388;     nsPfHMUKEKCNLmnJl27466388 = nsPfHMUKEKCNLmnJl19194574;     nsPfHMUKEKCNLmnJl19194574 = nsPfHMUKEKCNLmnJl86736446;     nsPfHMUKEKCNLmnJl86736446 = nsPfHMUKEKCNLmnJl11427804;     nsPfHMUKEKCNLmnJl11427804 = nsPfHMUKEKCNLmnJl87703873;     nsPfHMUKEKCNLmnJl87703873 = nsPfHMUKEKCNLmnJl55729671;     nsPfHMUKEKCNLmnJl55729671 = nsPfHMUKEKCNLmnJl17372434;     nsPfHMUKEKCNLmnJl17372434 = nsPfHMUKEKCNLmnJl79622342;     nsPfHMUKEKCNLmnJl79622342 = nsPfHMUKEKCNLmnJl4962222;     nsPfHMUKEKCNLmnJl4962222 = nsPfHMUKEKCNLmnJl91582775;     nsPfHMUKEKCNLmnJl91582775 = nsPfHMUKEKCNLmnJl93653156;     nsPfHMUKEKCNLmnJl93653156 = nsPfHMUKEKCNLmnJl68854051;     nsPfHMUKEKCNLmnJl68854051 = nsPfHMUKEKCNLmnJl96394990;     nsPfHMUKEKCNLmnJl96394990 = nsPfHMUKEKCNLmnJl9396642;     nsPfHMUKEKCNLmnJl9396642 = nsPfHMUKEKCNLmnJl15355533;     nsPfHMUKEKCNLmnJl15355533 = nsPfHMUKEKCNLmnJl93489184;     nsPfHMUKEKCNLmnJl93489184 = nsPfHMUKEKCNLmnJl35685101;     nsPfHMUKEKCNLmnJl35685101 = nsPfHMUKEKCNLmnJl15403989;     nsPfHMUKEKCNLmnJl15403989 = nsPfHMUKEKCNLmnJl2468620;     nsPfHMUKEKCNLmnJl2468620 = nsPfHMUKEKCNLmnJl68664472;     nsPfHMUKEKCNLmnJl68664472 = nsPfHMUKEKCNLmnJl18276442;     nsPfHMUKEKCNLmnJl18276442 = nsPfHMUKEKCNLmnJl73427980;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MGDWNuUUhBOCFjtF18540467() {     double nJqWWnJwNncMlJBpc79545087 = -398408118;    double nJqWWnJwNncMlJBpc52143342 = -878972700;    double nJqWWnJwNncMlJBpc12805282 = -902338815;    double nJqWWnJwNncMlJBpc93478031 = -440974194;    double nJqWWnJwNncMlJBpc28669509 = 27876067;    double nJqWWnJwNncMlJBpc56492093 = -459443132;    double nJqWWnJwNncMlJBpc66999775 = -100590018;    double nJqWWnJwNncMlJBpc88755260 = -627178107;    double nJqWWnJwNncMlJBpc68028096 = -675782602;    double nJqWWnJwNncMlJBpc74269265 = -786671054;    double nJqWWnJwNncMlJBpc42952885 = -992173277;    double nJqWWnJwNncMlJBpc91572137 = -763815836;    double nJqWWnJwNncMlJBpc50504243 = -169298014;    double nJqWWnJwNncMlJBpc18005866 = -791293822;    double nJqWWnJwNncMlJBpc3451428 = 26195856;    double nJqWWnJwNncMlJBpc8847540 = -246122795;    double nJqWWnJwNncMlJBpc88552307 = -917223435;    double nJqWWnJwNncMlJBpc80826603 = 57225960;    double nJqWWnJwNncMlJBpc85066037 = 4925357;    double nJqWWnJwNncMlJBpc58389479 = -138834507;    double nJqWWnJwNncMlJBpc54832 = 98154281;    double nJqWWnJwNncMlJBpc21671893 = -90777434;    double nJqWWnJwNncMlJBpc57425096 = -865979523;    double nJqWWnJwNncMlJBpc66916436 = -20238579;    double nJqWWnJwNncMlJBpc15277901 = -459479486;    double nJqWWnJwNncMlJBpc9241302 = -210810911;    double nJqWWnJwNncMlJBpc10718519 = -8583975;    double nJqWWnJwNncMlJBpc44615433 = -210041070;    double nJqWWnJwNncMlJBpc51719333 = -304524275;    double nJqWWnJwNncMlJBpc88026616 = -471338157;    double nJqWWnJwNncMlJBpc97176229 = -613181882;    double nJqWWnJwNncMlJBpc14136171 = -200817720;    double nJqWWnJwNncMlJBpc90211250 = -272219588;    double nJqWWnJwNncMlJBpc78403099 = -970946298;    double nJqWWnJwNncMlJBpc93404919 = -950096616;    double nJqWWnJwNncMlJBpc24515723 = -833512874;    double nJqWWnJwNncMlJBpc91495961 = -248029071;    double nJqWWnJwNncMlJBpc38311855 = -295574608;    double nJqWWnJwNncMlJBpc64220181 = -477706831;    double nJqWWnJwNncMlJBpc33807273 = -565465940;    double nJqWWnJwNncMlJBpc16356827 = -812552084;    double nJqWWnJwNncMlJBpc962738 = 46007682;    double nJqWWnJwNncMlJBpc64510219 = -18470212;    double nJqWWnJwNncMlJBpc84178138 = -554236838;    double nJqWWnJwNncMlJBpc96545952 = -616977740;    double nJqWWnJwNncMlJBpc56051058 = -908411920;    double nJqWWnJwNncMlJBpc83103626 = -229023720;    double nJqWWnJwNncMlJBpc33479236 = -547477226;    double nJqWWnJwNncMlJBpc29756233 = -433484147;    double nJqWWnJwNncMlJBpc10794857 = -785506524;    double nJqWWnJwNncMlJBpc99381986 = -49079235;    double nJqWWnJwNncMlJBpc98370965 = -254031046;    double nJqWWnJwNncMlJBpc85151663 = 24670771;    double nJqWWnJwNncMlJBpc71506961 = -683229369;    double nJqWWnJwNncMlJBpc18485635 = -253132664;    double nJqWWnJwNncMlJBpc57873194 = -207630685;    double nJqWWnJwNncMlJBpc94718245 = 87006822;    double nJqWWnJwNncMlJBpc45888846 = -782100236;    double nJqWWnJwNncMlJBpc78200131 = -981494708;    double nJqWWnJwNncMlJBpc19428208 = -761313023;    double nJqWWnJwNncMlJBpc45773575 = -350859157;    double nJqWWnJwNncMlJBpc22384342 = -890548948;    double nJqWWnJwNncMlJBpc37035927 = -222653833;    double nJqWWnJwNncMlJBpc80001480 = -104444445;    double nJqWWnJwNncMlJBpc77093035 = -73489173;    double nJqWWnJwNncMlJBpc28816714 = -691355557;    double nJqWWnJwNncMlJBpc1360887 = -391596249;    double nJqWWnJwNncMlJBpc72101144 = -198351717;    double nJqWWnJwNncMlJBpc24600946 = -841197206;    double nJqWWnJwNncMlJBpc78935705 = -140291271;    double nJqWWnJwNncMlJBpc17351578 = -998093725;    double nJqWWnJwNncMlJBpc50240453 = -521648827;    double nJqWWnJwNncMlJBpc16606422 = -465067209;    double nJqWWnJwNncMlJBpc51258764 = -429608704;    double nJqWWnJwNncMlJBpc42032652 = -326282423;    double nJqWWnJwNncMlJBpc99092094 = -947853401;    double nJqWWnJwNncMlJBpc57161673 = 27692777;    double nJqWWnJwNncMlJBpc73246958 = -211742685;    double nJqWWnJwNncMlJBpc70370483 = -403260840;    double nJqWWnJwNncMlJBpc59226842 = -551067567;    double nJqWWnJwNncMlJBpc26137675 = -981787192;    double nJqWWnJwNncMlJBpc77239283 = -461106750;    double nJqWWnJwNncMlJBpc14859200 = -776556923;    double nJqWWnJwNncMlJBpc40924476 = -519017752;    double nJqWWnJwNncMlJBpc88644630 = -322258923;    double nJqWWnJwNncMlJBpc98805263 = -259150837;    double nJqWWnJwNncMlJBpc28984507 = -125488491;    double nJqWWnJwNncMlJBpc18704290 = -588990219;    double nJqWWnJwNncMlJBpc59917464 = -617813634;    double nJqWWnJwNncMlJBpc35531726 = -642465931;    double nJqWWnJwNncMlJBpc29797477 = -820519696;    double nJqWWnJwNncMlJBpc45607115 = -465928835;    double nJqWWnJwNncMlJBpc60111723 = -314079901;    double nJqWWnJwNncMlJBpc44791974 = -716393809;    double nJqWWnJwNncMlJBpc88033698 = -114606783;    double nJqWWnJwNncMlJBpc93972484 = -922003136;    double nJqWWnJwNncMlJBpc63926810 = -731338486;    double nJqWWnJwNncMlJBpc84508739 = -914025767;    double nJqWWnJwNncMlJBpc7085103 = -380747665;    double nJqWWnJwNncMlJBpc67729238 = -398408118;     nJqWWnJwNncMlJBpc79545087 = nJqWWnJwNncMlJBpc52143342;     nJqWWnJwNncMlJBpc52143342 = nJqWWnJwNncMlJBpc12805282;     nJqWWnJwNncMlJBpc12805282 = nJqWWnJwNncMlJBpc93478031;     nJqWWnJwNncMlJBpc93478031 = nJqWWnJwNncMlJBpc28669509;     nJqWWnJwNncMlJBpc28669509 = nJqWWnJwNncMlJBpc56492093;     nJqWWnJwNncMlJBpc56492093 = nJqWWnJwNncMlJBpc66999775;     nJqWWnJwNncMlJBpc66999775 = nJqWWnJwNncMlJBpc88755260;     nJqWWnJwNncMlJBpc88755260 = nJqWWnJwNncMlJBpc68028096;     nJqWWnJwNncMlJBpc68028096 = nJqWWnJwNncMlJBpc74269265;     nJqWWnJwNncMlJBpc74269265 = nJqWWnJwNncMlJBpc42952885;     nJqWWnJwNncMlJBpc42952885 = nJqWWnJwNncMlJBpc91572137;     nJqWWnJwNncMlJBpc91572137 = nJqWWnJwNncMlJBpc50504243;     nJqWWnJwNncMlJBpc50504243 = nJqWWnJwNncMlJBpc18005866;     nJqWWnJwNncMlJBpc18005866 = nJqWWnJwNncMlJBpc3451428;     nJqWWnJwNncMlJBpc3451428 = nJqWWnJwNncMlJBpc8847540;     nJqWWnJwNncMlJBpc8847540 = nJqWWnJwNncMlJBpc88552307;     nJqWWnJwNncMlJBpc88552307 = nJqWWnJwNncMlJBpc80826603;     nJqWWnJwNncMlJBpc80826603 = nJqWWnJwNncMlJBpc85066037;     nJqWWnJwNncMlJBpc85066037 = nJqWWnJwNncMlJBpc58389479;     nJqWWnJwNncMlJBpc58389479 = nJqWWnJwNncMlJBpc54832;     nJqWWnJwNncMlJBpc54832 = nJqWWnJwNncMlJBpc21671893;     nJqWWnJwNncMlJBpc21671893 = nJqWWnJwNncMlJBpc57425096;     nJqWWnJwNncMlJBpc57425096 = nJqWWnJwNncMlJBpc66916436;     nJqWWnJwNncMlJBpc66916436 = nJqWWnJwNncMlJBpc15277901;     nJqWWnJwNncMlJBpc15277901 = nJqWWnJwNncMlJBpc9241302;     nJqWWnJwNncMlJBpc9241302 = nJqWWnJwNncMlJBpc10718519;     nJqWWnJwNncMlJBpc10718519 = nJqWWnJwNncMlJBpc44615433;     nJqWWnJwNncMlJBpc44615433 = nJqWWnJwNncMlJBpc51719333;     nJqWWnJwNncMlJBpc51719333 = nJqWWnJwNncMlJBpc88026616;     nJqWWnJwNncMlJBpc88026616 = nJqWWnJwNncMlJBpc97176229;     nJqWWnJwNncMlJBpc97176229 = nJqWWnJwNncMlJBpc14136171;     nJqWWnJwNncMlJBpc14136171 = nJqWWnJwNncMlJBpc90211250;     nJqWWnJwNncMlJBpc90211250 = nJqWWnJwNncMlJBpc78403099;     nJqWWnJwNncMlJBpc78403099 = nJqWWnJwNncMlJBpc93404919;     nJqWWnJwNncMlJBpc93404919 = nJqWWnJwNncMlJBpc24515723;     nJqWWnJwNncMlJBpc24515723 = nJqWWnJwNncMlJBpc91495961;     nJqWWnJwNncMlJBpc91495961 = nJqWWnJwNncMlJBpc38311855;     nJqWWnJwNncMlJBpc38311855 = nJqWWnJwNncMlJBpc64220181;     nJqWWnJwNncMlJBpc64220181 = nJqWWnJwNncMlJBpc33807273;     nJqWWnJwNncMlJBpc33807273 = nJqWWnJwNncMlJBpc16356827;     nJqWWnJwNncMlJBpc16356827 = nJqWWnJwNncMlJBpc962738;     nJqWWnJwNncMlJBpc962738 = nJqWWnJwNncMlJBpc64510219;     nJqWWnJwNncMlJBpc64510219 = nJqWWnJwNncMlJBpc84178138;     nJqWWnJwNncMlJBpc84178138 = nJqWWnJwNncMlJBpc96545952;     nJqWWnJwNncMlJBpc96545952 = nJqWWnJwNncMlJBpc56051058;     nJqWWnJwNncMlJBpc56051058 = nJqWWnJwNncMlJBpc83103626;     nJqWWnJwNncMlJBpc83103626 = nJqWWnJwNncMlJBpc33479236;     nJqWWnJwNncMlJBpc33479236 = nJqWWnJwNncMlJBpc29756233;     nJqWWnJwNncMlJBpc29756233 = nJqWWnJwNncMlJBpc10794857;     nJqWWnJwNncMlJBpc10794857 = nJqWWnJwNncMlJBpc99381986;     nJqWWnJwNncMlJBpc99381986 = nJqWWnJwNncMlJBpc98370965;     nJqWWnJwNncMlJBpc98370965 = nJqWWnJwNncMlJBpc85151663;     nJqWWnJwNncMlJBpc85151663 = nJqWWnJwNncMlJBpc71506961;     nJqWWnJwNncMlJBpc71506961 = nJqWWnJwNncMlJBpc18485635;     nJqWWnJwNncMlJBpc18485635 = nJqWWnJwNncMlJBpc57873194;     nJqWWnJwNncMlJBpc57873194 = nJqWWnJwNncMlJBpc94718245;     nJqWWnJwNncMlJBpc94718245 = nJqWWnJwNncMlJBpc45888846;     nJqWWnJwNncMlJBpc45888846 = nJqWWnJwNncMlJBpc78200131;     nJqWWnJwNncMlJBpc78200131 = nJqWWnJwNncMlJBpc19428208;     nJqWWnJwNncMlJBpc19428208 = nJqWWnJwNncMlJBpc45773575;     nJqWWnJwNncMlJBpc45773575 = nJqWWnJwNncMlJBpc22384342;     nJqWWnJwNncMlJBpc22384342 = nJqWWnJwNncMlJBpc37035927;     nJqWWnJwNncMlJBpc37035927 = nJqWWnJwNncMlJBpc80001480;     nJqWWnJwNncMlJBpc80001480 = nJqWWnJwNncMlJBpc77093035;     nJqWWnJwNncMlJBpc77093035 = nJqWWnJwNncMlJBpc28816714;     nJqWWnJwNncMlJBpc28816714 = nJqWWnJwNncMlJBpc1360887;     nJqWWnJwNncMlJBpc1360887 = nJqWWnJwNncMlJBpc72101144;     nJqWWnJwNncMlJBpc72101144 = nJqWWnJwNncMlJBpc24600946;     nJqWWnJwNncMlJBpc24600946 = nJqWWnJwNncMlJBpc78935705;     nJqWWnJwNncMlJBpc78935705 = nJqWWnJwNncMlJBpc17351578;     nJqWWnJwNncMlJBpc17351578 = nJqWWnJwNncMlJBpc50240453;     nJqWWnJwNncMlJBpc50240453 = nJqWWnJwNncMlJBpc16606422;     nJqWWnJwNncMlJBpc16606422 = nJqWWnJwNncMlJBpc51258764;     nJqWWnJwNncMlJBpc51258764 = nJqWWnJwNncMlJBpc42032652;     nJqWWnJwNncMlJBpc42032652 = nJqWWnJwNncMlJBpc99092094;     nJqWWnJwNncMlJBpc99092094 = nJqWWnJwNncMlJBpc57161673;     nJqWWnJwNncMlJBpc57161673 = nJqWWnJwNncMlJBpc73246958;     nJqWWnJwNncMlJBpc73246958 = nJqWWnJwNncMlJBpc70370483;     nJqWWnJwNncMlJBpc70370483 = nJqWWnJwNncMlJBpc59226842;     nJqWWnJwNncMlJBpc59226842 = nJqWWnJwNncMlJBpc26137675;     nJqWWnJwNncMlJBpc26137675 = nJqWWnJwNncMlJBpc77239283;     nJqWWnJwNncMlJBpc77239283 = nJqWWnJwNncMlJBpc14859200;     nJqWWnJwNncMlJBpc14859200 = nJqWWnJwNncMlJBpc40924476;     nJqWWnJwNncMlJBpc40924476 = nJqWWnJwNncMlJBpc88644630;     nJqWWnJwNncMlJBpc88644630 = nJqWWnJwNncMlJBpc98805263;     nJqWWnJwNncMlJBpc98805263 = nJqWWnJwNncMlJBpc28984507;     nJqWWnJwNncMlJBpc28984507 = nJqWWnJwNncMlJBpc18704290;     nJqWWnJwNncMlJBpc18704290 = nJqWWnJwNncMlJBpc59917464;     nJqWWnJwNncMlJBpc59917464 = nJqWWnJwNncMlJBpc35531726;     nJqWWnJwNncMlJBpc35531726 = nJqWWnJwNncMlJBpc29797477;     nJqWWnJwNncMlJBpc29797477 = nJqWWnJwNncMlJBpc45607115;     nJqWWnJwNncMlJBpc45607115 = nJqWWnJwNncMlJBpc60111723;     nJqWWnJwNncMlJBpc60111723 = nJqWWnJwNncMlJBpc44791974;     nJqWWnJwNncMlJBpc44791974 = nJqWWnJwNncMlJBpc88033698;     nJqWWnJwNncMlJBpc88033698 = nJqWWnJwNncMlJBpc93972484;     nJqWWnJwNncMlJBpc93972484 = nJqWWnJwNncMlJBpc63926810;     nJqWWnJwNncMlJBpc63926810 = nJqWWnJwNncMlJBpc84508739;     nJqWWnJwNncMlJBpc84508739 = nJqWWnJwNncMlJBpc7085103;     nJqWWnJwNncMlJBpc7085103 = nJqWWnJwNncMlJBpc67729238;     nJqWWnJwNncMlJBpc67729238 = nJqWWnJwNncMlJBpc79545087;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NpCGWCpcoGDxNKEJ85832065() {     double NuyZFOBbizDmBLXtE15004056 = -864685018;    double NuyZFOBbizDmBLXtE65415420 = -759524307;    double NuyZFOBbizDmBLXtE50191759 = -568005890;    double NuyZFOBbizDmBLXtE65701386 = -262125265;    double NuyZFOBbizDmBLXtE84954778 = -77722778;    double NuyZFOBbizDmBLXtE92714965 = -721791027;    double NuyZFOBbizDmBLXtE77657181 = 54684248;    double NuyZFOBbizDmBLXtE50886821 = -271412248;    double NuyZFOBbizDmBLXtE69545702 = -64114009;    double NuyZFOBbizDmBLXtE89171977 = -451358052;    double NuyZFOBbizDmBLXtE81134619 = -794181006;    double NuyZFOBbizDmBLXtE92140286 = -627936608;    double NuyZFOBbizDmBLXtE88738888 = -721310519;    double NuyZFOBbizDmBLXtE71146729 = -767657231;    double NuyZFOBbizDmBLXtE61645944 = -477229159;    double NuyZFOBbizDmBLXtE92388840 = -981934402;    double NuyZFOBbizDmBLXtE44388243 = -344197988;    double NuyZFOBbizDmBLXtE80586794 = -16202108;    double NuyZFOBbizDmBLXtE85616367 = -311638065;    double NuyZFOBbizDmBLXtE52226697 = -565395572;    double NuyZFOBbizDmBLXtE13141695 = -659120408;    double NuyZFOBbizDmBLXtE25093418 = -177413728;    double NuyZFOBbizDmBLXtE27950476 = -503611707;    double NuyZFOBbizDmBLXtE60709381 = -531063184;    double NuyZFOBbizDmBLXtE23295852 = -483464685;    double NuyZFOBbizDmBLXtE19939473 = -999387718;    double NuyZFOBbizDmBLXtE71139093 = -914868116;    double NuyZFOBbizDmBLXtE85343858 = -379813244;    double NuyZFOBbizDmBLXtE98387547 = -878619078;    double NuyZFOBbizDmBLXtE93560027 = -505845328;    double NuyZFOBbizDmBLXtE15759461 = -65162369;    double NuyZFOBbizDmBLXtE51959189 = 26210865;    double NuyZFOBbizDmBLXtE2429262 = -419815565;    double NuyZFOBbizDmBLXtE8151526 = 64359889;    double NuyZFOBbizDmBLXtE59213373 = -901196867;    double NuyZFOBbizDmBLXtE76076566 = 51921820;    double NuyZFOBbizDmBLXtE40907778 = -112725111;    double NuyZFOBbizDmBLXtE89527257 = -278230190;    double NuyZFOBbizDmBLXtE94610742 = -128583107;    double NuyZFOBbizDmBLXtE98078805 = -30728486;    double NuyZFOBbizDmBLXtE49808434 = -581750998;    double NuyZFOBbizDmBLXtE90327864 = -648537609;    double NuyZFOBbizDmBLXtE53893598 = -671650998;    double NuyZFOBbizDmBLXtE24309477 = -58569248;    double NuyZFOBbizDmBLXtE96562410 = -443488901;    double NuyZFOBbizDmBLXtE2012387 = -947335552;    double NuyZFOBbizDmBLXtE2448790 = -713010080;    double NuyZFOBbizDmBLXtE51205547 = -714003819;    double NuyZFOBbizDmBLXtE52648899 = -97221027;    double NuyZFOBbizDmBLXtE35324446 = -176533340;    double NuyZFOBbizDmBLXtE80152710 = 76644207;    double NuyZFOBbizDmBLXtE81856438 = -467683392;    double NuyZFOBbizDmBLXtE53772812 = -831374783;    double NuyZFOBbizDmBLXtE87498807 = -874737869;    double NuyZFOBbizDmBLXtE72690371 = -599622810;    double NuyZFOBbizDmBLXtE89910637 = -587271291;    double NuyZFOBbizDmBLXtE37464944 = -155912600;    double NuyZFOBbizDmBLXtE89482377 = 63057293;    double NuyZFOBbizDmBLXtE42405535 = -778660581;    double NuyZFOBbizDmBLXtE65015305 = -78335061;    double NuyZFOBbizDmBLXtE21575873 = -806922912;    double NuyZFOBbizDmBLXtE92313323 = -565502508;    double NuyZFOBbizDmBLXtE52499273 = -392793171;    double NuyZFOBbizDmBLXtE75985675 = -558268681;    double NuyZFOBbizDmBLXtE73412516 = -286195684;    double NuyZFOBbizDmBLXtE29175431 = -720391872;    double NuyZFOBbizDmBLXtE89711024 = -108121043;    double NuyZFOBbizDmBLXtE80587363 = -685670408;    double NuyZFOBbizDmBLXtE11933356 = -866460365;    double NuyZFOBbizDmBLXtE85569378 = -429150980;    double NuyZFOBbizDmBLXtE51481062 = -769209291;    double NuyZFOBbizDmBLXtE54860985 = 34032201;    double NuyZFOBbizDmBLXtE85976051 = -887619001;    double NuyZFOBbizDmBLXtE87537562 = -180909580;    double NuyZFOBbizDmBLXtE2418263 = -983644575;    double NuyZFOBbizDmBLXtE22813830 = 89417201;    double NuyZFOBbizDmBLXtE71199820 = -505762731;    double NuyZFOBbizDmBLXtE3641000 = -345042459;    double NuyZFOBbizDmBLXtE64146970 = 12425717;    double NuyZFOBbizDmBLXtE21283465 = -536129133;    double NuyZFOBbizDmBLXtE17490683 = -186377638;    double NuyZFOBbizDmBLXtE19933546 = -100864297;    double NuyZFOBbizDmBLXtE32694959 = -182592217;    double NuyZFOBbizDmBLXtE63063101 = -602085738;    double NuyZFOBbizDmBLXtE13407317 = -482489536;    double NuyZFOBbizDmBLXtE33903023 = -597478977;    double NuyZFOBbizDmBLXtE98186376 = -142414352;    double NuyZFOBbizDmBLXtE14930454 = -545077697;    double NuyZFOBbizDmBLXtE35461155 = -336017301;    double NuyZFOBbizDmBLXtE69302736 = -213925576;    double NuyZFOBbizDmBLXtE38611622 = -792165580;    double NuyZFOBbizDmBLXtE51425400 = -75782404;    double NuyZFOBbizDmBLXtE47121723 = -499569609;    double NuyZFOBbizDmBLXtE29595438 = 49751953;    double NuyZFOBbizDmBLXtE76502932 = -223805574;    double NuyZFOBbizDmBLXtE57495111 = 83751510;    double NuyZFOBbizDmBLXtE37828591 = -155744439;    double NuyZFOBbizDmBLXtE77907923 = -13382317;    double NuyZFOBbizDmBLXtE50896960 = -772373565;    double NuyZFOBbizDmBLXtE67386980 = -864685018;     NuyZFOBbizDmBLXtE15004056 = NuyZFOBbizDmBLXtE65415420;     NuyZFOBbizDmBLXtE65415420 = NuyZFOBbizDmBLXtE50191759;     NuyZFOBbizDmBLXtE50191759 = NuyZFOBbizDmBLXtE65701386;     NuyZFOBbizDmBLXtE65701386 = NuyZFOBbizDmBLXtE84954778;     NuyZFOBbizDmBLXtE84954778 = NuyZFOBbizDmBLXtE92714965;     NuyZFOBbizDmBLXtE92714965 = NuyZFOBbizDmBLXtE77657181;     NuyZFOBbizDmBLXtE77657181 = NuyZFOBbizDmBLXtE50886821;     NuyZFOBbizDmBLXtE50886821 = NuyZFOBbizDmBLXtE69545702;     NuyZFOBbizDmBLXtE69545702 = NuyZFOBbizDmBLXtE89171977;     NuyZFOBbizDmBLXtE89171977 = NuyZFOBbizDmBLXtE81134619;     NuyZFOBbizDmBLXtE81134619 = NuyZFOBbizDmBLXtE92140286;     NuyZFOBbizDmBLXtE92140286 = NuyZFOBbizDmBLXtE88738888;     NuyZFOBbizDmBLXtE88738888 = NuyZFOBbizDmBLXtE71146729;     NuyZFOBbizDmBLXtE71146729 = NuyZFOBbizDmBLXtE61645944;     NuyZFOBbizDmBLXtE61645944 = NuyZFOBbizDmBLXtE92388840;     NuyZFOBbizDmBLXtE92388840 = NuyZFOBbizDmBLXtE44388243;     NuyZFOBbizDmBLXtE44388243 = NuyZFOBbizDmBLXtE80586794;     NuyZFOBbizDmBLXtE80586794 = NuyZFOBbizDmBLXtE85616367;     NuyZFOBbizDmBLXtE85616367 = NuyZFOBbizDmBLXtE52226697;     NuyZFOBbizDmBLXtE52226697 = NuyZFOBbizDmBLXtE13141695;     NuyZFOBbizDmBLXtE13141695 = NuyZFOBbizDmBLXtE25093418;     NuyZFOBbizDmBLXtE25093418 = NuyZFOBbizDmBLXtE27950476;     NuyZFOBbizDmBLXtE27950476 = NuyZFOBbizDmBLXtE60709381;     NuyZFOBbizDmBLXtE60709381 = NuyZFOBbizDmBLXtE23295852;     NuyZFOBbizDmBLXtE23295852 = NuyZFOBbizDmBLXtE19939473;     NuyZFOBbizDmBLXtE19939473 = NuyZFOBbizDmBLXtE71139093;     NuyZFOBbizDmBLXtE71139093 = NuyZFOBbizDmBLXtE85343858;     NuyZFOBbizDmBLXtE85343858 = NuyZFOBbizDmBLXtE98387547;     NuyZFOBbizDmBLXtE98387547 = NuyZFOBbizDmBLXtE93560027;     NuyZFOBbizDmBLXtE93560027 = NuyZFOBbizDmBLXtE15759461;     NuyZFOBbizDmBLXtE15759461 = NuyZFOBbizDmBLXtE51959189;     NuyZFOBbizDmBLXtE51959189 = NuyZFOBbizDmBLXtE2429262;     NuyZFOBbizDmBLXtE2429262 = NuyZFOBbizDmBLXtE8151526;     NuyZFOBbizDmBLXtE8151526 = NuyZFOBbizDmBLXtE59213373;     NuyZFOBbizDmBLXtE59213373 = NuyZFOBbizDmBLXtE76076566;     NuyZFOBbizDmBLXtE76076566 = NuyZFOBbizDmBLXtE40907778;     NuyZFOBbizDmBLXtE40907778 = NuyZFOBbizDmBLXtE89527257;     NuyZFOBbizDmBLXtE89527257 = NuyZFOBbizDmBLXtE94610742;     NuyZFOBbizDmBLXtE94610742 = NuyZFOBbizDmBLXtE98078805;     NuyZFOBbizDmBLXtE98078805 = NuyZFOBbizDmBLXtE49808434;     NuyZFOBbizDmBLXtE49808434 = NuyZFOBbizDmBLXtE90327864;     NuyZFOBbizDmBLXtE90327864 = NuyZFOBbizDmBLXtE53893598;     NuyZFOBbizDmBLXtE53893598 = NuyZFOBbizDmBLXtE24309477;     NuyZFOBbizDmBLXtE24309477 = NuyZFOBbizDmBLXtE96562410;     NuyZFOBbizDmBLXtE96562410 = NuyZFOBbizDmBLXtE2012387;     NuyZFOBbizDmBLXtE2012387 = NuyZFOBbizDmBLXtE2448790;     NuyZFOBbizDmBLXtE2448790 = NuyZFOBbizDmBLXtE51205547;     NuyZFOBbizDmBLXtE51205547 = NuyZFOBbizDmBLXtE52648899;     NuyZFOBbizDmBLXtE52648899 = NuyZFOBbizDmBLXtE35324446;     NuyZFOBbizDmBLXtE35324446 = NuyZFOBbizDmBLXtE80152710;     NuyZFOBbizDmBLXtE80152710 = NuyZFOBbizDmBLXtE81856438;     NuyZFOBbizDmBLXtE81856438 = NuyZFOBbizDmBLXtE53772812;     NuyZFOBbizDmBLXtE53772812 = NuyZFOBbizDmBLXtE87498807;     NuyZFOBbizDmBLXtE87498807 = NuyZFOBbizDmBLXtE72690371;     NuyZFOBbizDmBLXtE72690371 = NuyZFOBbizDmBLXtE89910637;     NuyZFOBbizDmBLXtE89910637 = NuyZFOBbizDmBLXtE37464944;     NuyZFOBbizDmBLXtE37464944 = NuyZFOBbizDmBLXtE89482377;     NuyZFOBbizDmBLXtE89482377 = NuyZFOBbizDmBLXtE42405535;     NuyZFOBbizDmBLXtE42405535 = NuyZFOBbizDmBLXtE65015305;     NuyZFOBbizDmBLXtE65015305 = NuyZFOBbizDmBLXtE21575873;     NuyZFOBbizDmBLXtE21575873 = NuyZFOBbizDmBLXtE92313323;     NuyZFOBbizDmBLXtE92313323 = NuyZFOBbizDmBLXtE52499273;     NuyZFOBbizDmBLXtE52499273 = NuyZFOBbizDmBLXtE75985675;     NuyZFOBbizDmBLXtE75985675 = NuyZFOBbizDmBLXtE73412516;     NuyZFOBbizDmBLXtE73412516 = NuyZFOBbizDmBLXtE29175431;     NuyZFOBbizDmBLXtE29175431 = NuyZFOBbizDmBLXtE89711024;     NuyZFOBbizDmBLXtE89711024 = NuyZFOBbizDmBLXtE80587363;     NuyZFOBbizDmBLXtE80587363 = NuyZFOBbizDmBLXtE11933356;     NuyZFOBbizDmBLXtE11933356 = NuyZFOBbizDmBLXtE85569378;     NuyZFOBbizDmBLXtE85569378 = NuyZFOBbizDmBLXtE51481062;     NuyZFOBbizDmBLXtE51481062 = NuyZFOBbizDmBLXtE54860985;     NuyZFOBbizDmBLXtE54860985 = NuyZFOBbizDmBLXtE85976051;     NuyZFOBbizDmBLXtE85976051 = NuyZFOBbizDmBLXtE87537562;     NuyZFOBbizDmBLXtE87537562 = NuyZFOBbizDmBLXtE2418263;     NuyZFOBbizDmBLXtE2418263 = NuyZFOBbizDmBLXtE22813830;     NuyZFOBbizDmBLXtE22813830 = NuyZFOBbizDmBLXtE71199820;     NuyZFOBbizDmBLXtE71199820 = NuyZFOBbizDmBLXtE3641000;     NuyZFOBbizDmBLXtE3641000 = NuyZFOBbizDmBLXtE64146970;     NuyZFOBbizDmBLXtE64146970 = NuyZFOBbizDmBLXtE21283465;     NuyZFOBbizDmBLXtE21283465 = NuyZFOBbizDmBLXtE17490683;     NuyZFOBbizDmBLXtE17490683 = NuyZFOBbizDmBLXtE19933546;     NuyZFOBbizDmBLXtE19933546 = NuyZFOBbizDmBLXtE32694959;     NuyZFOBbizDmBLXtE32694959 = NuyZFOBbizDmBLXtE63063101;     NuyZFOBbizDmBLXtE63063101 = NuyZFOBbizDmBLXtE13407317;     NuyZFOBbizDmBLXtE13407317 = NuyZFOBbizDmBLXtE33903023;     NuyZFOBbizDmBLXtE33903023 = NuyZFOBbizDmBLXtE98186376;     NuyZFOBbizDmBLXtE98186376 = NuyZFOBbizDmBLXtE14930454;     NuyZFOBbizDmBLXtE14930454 = NuyZFOBbizDmBLXtE35461155;     NuyZFOBbizDmBLXtE35461155 = NuyZFOBbizDmBLXtE69302736;     NuyZFOBbizDmBLXtE69302736 = NuyZFOBbizDmBLXtE38611622;     NuyZFOBbizDmBLXtE38611622 = NuyZFOBbizDmBLXtE51425400;     NuyZFOBbizDmBLXtE51425400 = NuyZFOBbizDmBLXtE47121723;     NuyZFOBbizDmBLXtE47121723 = NuyZFOBbizDmBLXtE29595438;     NuyZFOBbizDmBLXtE29595438 = NuyZFOBbizDmBLXtE76502932;     NuyZFOBbizDmBLXtE76502932 = NuyZFOBbizDmBLXtE57495111;     NuyZFOBbizDmBLXtE57495111 = NuyZFOBbizDmBLXtE37828591;     NuyZFOBbizDmBLXtE37828591 = NuyZFOBbizDmBLXtE77907923;     NuyZFOBbizDmBLXtE77907923 = NuyZFOBbizDmBLXtE50896960;     NuyZFOBbizDmBLXtE50896960 = NuyZFOBbizDmBLXtE67386980;     NuyZFOBbizDmBLXtE67386980 = NuyZFOBbizDmBLXtE15004056;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void brfkTltQGJxoKiwe881134() {     double VxpGIjMIBSuvrOkBn21121162 = -976586907;    double VxpGIjMIBSuvrOkBn8788700 = -469124088;    double VxpGIjMIBSuvrOkBn66233942 = -718838811;    double VxpGIjMIBSuvrOkBn92776167 = -452873896;    double VxpGIjMIBSuvrOkBn53850266 = -598372650;    double VxpGIjMIBSuvrOkBn84132167 = -986287262;    double VxpGIjMIBSuvrOkBn38146148 = -84280729;    double VxpGIjMIBSuvrOkBn646712 = -558286161;    double VxpGIjMIBSuvrOkBn34188197 = -932146675;    double VxpGIjMIBSuvrOkBn30401637 = -780333530;    double VxpGIjMIBSuvrOkBn41244910 = -671618593;    double VxpGIjMIBSuvrOkBn23963329 = 27657190;    double VxpGIjMIBSuvrOkBn74948007 = -61201608;    double VxpGIjMIBSuvrOkBn54290406 = -199895799;    double VxpGIjMIBSuvrOkBn57388371 = -67691326;    double VxpGIjMIBSuvrOkBn33894234 = -954933945;    double VxpGIjMIBSuvrOkBn38224246 = -960743101;    double VxpGIjMIBSuvrOkBn77639947 = -781724751;    double VxpGIjMIBSuvrOkBn47446209 = -717407162;    double VxpGIjMIBSuvrOkBn32776862 = -922298492;    double VxpGIjMIBSuvrOkBn1694888 = -514607864;    double VxpGIjMIBSuvrOkBn11614442 = -85343444;    double VxpGIjMIBSuvrOkBn63541267 = -250129194;    double VxpGIjMIBSuvrOkBn44701755 = -753368171;    double VxpGIjMIBSuvrOkBn66995501 = -856660718;    double VxpGIjMIBSuvrOkBn44661148 = -878043041;    double VxpGIjMIBSuvrOkBn69598871 = -815530319;    double VxpGIjMIBSuvrOkBn98450983 = -445742198;    double VxpGIjMIBSuvrOkBn64519803 = -1001782;    double VxpGIjMIBSuvrOkBn62175664 = -323316195;    double VxpGIjMIBSuvrOkBn8002011 = 21516693;    double VxpGIjMIBSuvrOkBn49569811 = -783235950;    double VxpGIjMIBSuvrOkBn12140211 = -312428935;    double VxpGIjMIBSuvrOkBn2982090 = -787181876;    double VxpGIjMIBSuvrOkBn20688026 = -439929972;    double VxpGIjMIBSuvrOkBn44802480 = -162104287;    double VxpGIjMIBSuvrOkBn22169713 = -66360981;    double VxpGIjMIBSuvrOkBn23617471 = -49736199;    double VxpGIjMIBSuvrOkBn68220996 = -122506604;    double VxpGIjMIBSuvrOkBn85580743 = -504167708;    double VxpGIjMIBSuvrOkBn55477658 = -647024892;    double VxpGIjMIBSuvrOkBn22478322 = -918679887;    double VxpGIjMIBSuvrOkBn31960576 = -752455581;    double VxpGIjMIBSuvrOkBn11717217 = -77912186;    double VxpGIjMIBSuvrOkBn8514876 = -105387056;    double VxpGIjMIBSuvrOkBn13951582 = -932551668;    double VxpGIjMIBSuvrOkBn20227364 = -831785618;    double VxpGIjMIBSuvrOkBn59162488 = -723899352;    double VxpGIjMIBSuvrOkBn62324628 = -182958015;    double VxpGIjMIBSuvrOkBn48236099 = -263544148;    double VxpGIjMIBSuvrOkBn15853389 = -865649464;    double VxpGIjMIBSuvrOkBn92666157 = -268409712;    double VxpGIjMIBSuvrOkBn2021270 = -623164371;    double VxpGIjMIBSuvrOkBn83467688 = -95208014;    double VxpGIjMIBSuvrOkBn99186245 = -200154593;    double VxpGIjMIBSuvrOkBn9506721 = -791243463;    double VxpGIjMIBSuvrOkBn45247432 = -118994894;    double VxpGIjMIBSuvrOkBn21532188 = -965470640;    double VxpGIjMIBSuvrOkBn25780666 = -596213179;    double VxpGIjMIBSuvrOkBn9189119 = -720329609;    double VxpGIjMIBSuvrOkBn14533297 = -70756944;    double VxpGIjMIBSuvrOkBn39695164 = -638538532;    double VxpGIjMIBSuvrOkBn36126909 = -457284379;    double VxpGIjMIBSuvrOkBn72012533 = -508830480;    double VxpGIjMIBSuvrOkBn22399626 = -701850223;    double VxpGIjMIBSuvrOkBn91675099 = -888382643;    double VxpGIjMIBSuvrOkBn11823119 = -659913875;    double VxpGIjMIBSuvrOkBn71965917 = -274019732;    double VxpGIjMIBSuvrOkBn33602380 = -759965828;    double VxpGIjMIBSuvrOkBn12585892 = -905587039;    double VxpGIjMIBSuvrOkBn11724522 = -788572965;    double VxpGIjMIBSuvrOkBn14606776 = -811006902;    double VxpGIjMIBSuvrOkBn9418952 = -559218147;    double VxpGIjMIBSuvrOkBn61865466 = -113239454;    double VxpGIjMIBSuvrOkBn77299203 = -175273600;    double VxpGIjMIBSuvrOkBn79216566 = -595927977;    double VxpGIjMIBSuvrOkBn79653865 = -332887863;    double VxpGIjMIBSuvrOkBn51824051 = -72217008;    double VxpGIjMIBSuvrOkBn36186879 = -547981116;    double VxpGIjMIBSuvrOkBn53043920 = -924109050;    double VxpGIjMIBSuvrOkBn24433784 = 53742576;    double VxpGIjMIBSuvrOkBn10436383 = 8369032;    double VxpGIjMIBSuvrOkBn36126355 = -162784183;    double VxpGIjMIBSuvrOkBn16283705 = -737457635;    double VxpGIjMIBSuvrOkBn46322276 = -457666732;    double VxpGIjMIBSuvrOkBn15335853 = -710073595;    double VxpGIjMIBSuvrOkBn47548541 = -60071580;    double VxpGIjMIBSuvrOkBn28672522 = -117220922;    double VxpGIjMIBSuvrOkBn3795845 = -487027284;    double VxpGIjMIBSuvrOkBn11181305 = -648686509;    double VxpGIjMIBSuvrOkBn99555047 = 56890607;    double VxpGIjMIBSuvrOkBn637526 = -100890341;    double VxpGIjMIBSuvrOkBn97836804 = -453523021;    double VxpGIjMIBSuvrOkBn59031878 = -402176995;    double VxpGIjMIBSuvrOkBn71047446 = -333410765;    double VxpGIjMIBSuvrOkBn15782495 = 91513639;    double VxpGIjMIBSuvrOkBn86351412 = -361395509;    double VxpGIjMIBSuvrOkBn59948042 = -143625102;    double VxpGIjMIBSuvrOkBn89317590 = -376061963;    double VxpGIjMIBSuvrOkBn16839777 = -976586907;     VxpGIjMIBSuvrOkBn21121162 = VxpGIjMIBSuvrOkBn8788700;     VxpGIjMIBSuvrOkBn8788700 = VxpGIjMIBSuvrOkBn66233942;     VxpGIjMIBSuvrOkBn66233942 = VxpGIjMIBSuvrOkBn92776167;     VxpGIjMIBSuvrOkBn92776167 = VxpGIjMIBSuvrOkBn53850266;     VxpGIjMIBSuvrOkBn53850266 = VxpGIjMIBSuvrOkBn84132167;     VxpGIjMIBSuvrOkBn84132167 = VxpGIjMIBSuvrOkBn38146148;     VxpGIjMIBSuvrOkBn38146148 = VxpGIjMIBSuvrOkBn646712;     VxpGIjMIBSuvrOkBn646712 = VxpGIjMIBSuvrOkBn34188197;     VxpGIjMIBSuvrOkBn34188197 = VxpGIjMIBSuvrOkBn30401637;     VxpGIjMIBSuvrOkBn30401637 = VxpGIjMIBSuvrOkBn41244910;     VxpGIjMIBSuvrOkBn41244910 = VxpGIjMIBSuvrOkBn23963329;     VxpGIjMIBSuvrOkBn23963329 = VxpGIjMIBSuvrOkBn74948007;     VxpGIjMIBSuvrOkBn74948007 = VxpGIjMIBSuvrOkBn54290406;     VxpGIjMIBSuvrOkBn54290406 = VxpGIjMIBSuvrOkBn57388371;     VxpGIjMIBSuvrOkBn57388371 = VxpGIjMIBSuvrOkBn33894234;     VxpGIjMIBSuvrOkBn33894234 = VxpGIjMIBSuvrOkBn38224246;     VxpGIjMIBSuvrOkBn38224246 = VxpGIjMIBSuvrOkBn77639947;     VxpGIjMIBSuvrOkBn77639947 = VxpGIjMIBSuvrOkBn47446209;     VxpGIjMIBSuvrOkBn47446209 = VxpGIjMIBSuvrOkBn32776862;     VxpGIjMIBSuvrOkBn32776862 = VxpGIjMIBSuvrOkBn1694888;     VxpGIjMIBSuvrOkBn1694888 = VxpGIjMIBSuvrOkBn11614442;     VxpGIjMIBSuvrOkBn11614442 = VxpGIjMIBSuvrOkBn63541267;     VxpGIjMIBSuvrOkBn63541267 = VxpGIjMIBSuvrOkBn44701755;     VxpGIjMIBSuvrOkBn44701755 = VxpGIjMIBSuvrOkBn66995501;     VxpGIjMIBSuvrOkBn66995501 = VxpGIjMIBSuvrOkBn44661148;     VxpGIjMIBSuvrOkBn44661148 = VxpGIjMIBSuvrOkBn69598871;     VxpGIjMIBSuvrOkBn69598871 = VxpGIjMIBSuvrOkBn98450983;     VxpGIjMIBSuvrOkBn98450983 = VxpGIjMIBSuvrOkBn64519803;     VxpGIjMIBSuvrOkBn64519803 = VxpGIjMIBSuvrOkBn62175664;     VxpGIjMIBSuvrOkBn62175664 = VxpGIjMIBSuvrOkBn8002011;     VxpGIjMIBSuvrOkBn8002011 = VxpGIjMIBSuvrOkBn49569811;     VxpGIjMIBSuvrOkBn49569811 = VxpGIjMIBSuvrOkBn12140211;     VxpGIjMIBSuvrOkBn12140211 = VxpGIjMIBSuvrOkBn2982090;     VxpGIjMIBSuvrOkBn2982090 = VxpGIjMIBSuvrOkBn20688026;     VxpGIjMIBSuvrOkBn20688026 = VxpGIjMIBSuvrOkBn44802480;     VxpGIjMIBSuvrOkBn44802480 = VxpGIjMIBSuvrOkBn22169713;     VxpGIjMIBSuvrOkBn22169713 = VxpGIjMIBSuvrOkBn23617471;     VxpGIjMIBSuvrOkBn23617471 = VxpGIjMIBSuvrOkBn68220996;     VxpGIjMIBSuvrOkBn68220996 = VxpGIjMIBSuvrOkBn85580743;     VxpGIjMIBSuvrOkBn85580743 = VxpGIjMIBSuvrOkBn55477658;     VxpGIjMIBSuvrOkBn55477658 = VxpGIjMIBSuvrOkBn22478322;     VxpGIjMIBSuvrOkBn22478322 = VxpGIjMIBSuvrOkBn31960576;     VxpGIjMIBSuvrOkBn31960576 = VxpGIjMIBSuvrOkBn11717217;     VxpGIjMIBSuvrOkBn11717217 = VxpGIjMIBSuvrOkBn8514876;     VxpGIjMIBSuvrOkBn8514876 = VxpGIjMIBSuvrOkBn13951582;     VxpGIjMIBSuvrOkBn13951582 = VxpGIjMIBSuvrOkBn20227364;     VxpGIjMIBSuvrOkBn20227364 = VxpGIjMIBSuvrOkBn59162488;     VxpGIjMIBSuvrOkBn59162488 = VxpGIjMIBSuvrOkBn62324628;     VxpGIjMIBSuvrOkBn62324628 = VxpGIjMIBSuvrOkBn48236099;     VxpGIjMIBSuvrOkBn48236099 = VxpGIjMIBSuvrOkBn15853389;     VxpGIjMIBSuvrOkBn15853389 = VxpGIjMIBSuvrOkBn92666157;     VxpGIjMIBSuvrOkBn92666157 = VxpGIjMIBSuvrOkBn2021270;     VxpGIjMIBSuvrOkBn2021270 = VxpGIjMIBSuvrOkBn83467688;     VxpGIjMIBSuvrOkBn83467688 = VxpGIjMIBSuvrOkBn99186245;     VxpGIjMIBSuvrOkBn99186245 = VxpGIjMIBSuvrOkBn9506721;     VxpGIjMIBSuvrOkBn9506721 = VxpGIjMIBSuvrOkBn45247432;     VxpGIjMIBSuvrOkBn45247432 = VxpGIjMIBSuvrOkBn21532188;     VxpGIjMIBSuvrOkBn21532188 = VxpGIjMIBSuvrOkBn25780666;     VxpGIjMIBSuvrOkBn25780666 = VxpGIjMIBSuvrOkBn9189119;     VxpGIjMIBSuvrOkBn9189119 = VxpGIjMIBSuvrOkBn14533297;     VxpGIjMIBSuvrOkBn14533297 = VxpGIjMIBSuvrOkBn39695164;     VxpGIjMIBSuvrOkBn39695164 = VxpGIjMIBSuvrOkBn36126909;     VxpGIjMIBSuvrOkBn36126909 = VxpGIjMIBSuvrOkBn72012533;     VxpGIjMIBSuvrOkBn72012533 = VxpGIjMIBSuvrOkBn22399626;     VxpGIjMIBSuvrOkBn22399626 = VxpGIjMIBSuvrOkBn91675099;     VxpGIjMIBSuvrOkBn91675099 = VxpGIjMIBSuvrOkBn11823119;     VxpGIjMIBSuvrOkBn11823119 = VxpGIjMIBSuvrOkBn71965917;     VxpGIjMIBSuvrOkBn71965917 = VxpGIjMIBSuvrOkBn33602380;     VxpGIjMIBSuvrOkBn33602380 = VxpGIjMIBSuvrOkBn12585892;     VxpGIjMIBSuvrOkBn12585892 = VxpGIjMIBSuvrOkBn11724522;     VxpGIjMIBSuvrOkBn11724522 = VxpGIjMIBSuvrOkBn14606776;     VxpGIjMIBSuvrOkBn14606776 = VxpGIjMIBSuvrOkBn9418952;     VxpGIjMIBSuvrOkBn9418952 = VxpGIjMIBSuvrOkBn61865466;     VxpGIjMIBSuvrOkBn61865466 = VxpGIjMIBSuvrOkBn77299203;     VxpGIjMIBSuvrOkBn77299203 = VxpGIjMIBSuvrOkBn79216566;     VxpGIjMIBSuvrOkBn79216566 = VxpGIjMIBSuvrOkBn79653865;     VxpGIjMIBSuvrOkBn79653865 = VxpGIjMIBSuvrOkBn51824051;     VxpGIjMIBSuvrOkBn51824051 = VxpGIjMIBSuvrOkBn36186879;     VxpGIjMIBSuvrOkBn36186879 = VxpGIjMIBSuvrOkBn53043920;     VxpGIjMIBSuvrOkBn53043920 = VxpGIjMIBSuvrOkBn24433784;     VxpGIjMIBSuvrOkBn24433784 = VxpGIjMIBSuvrOkBn10436383;     VxpGIjMIBSuvrOkBn10436383 = VxpGIjMIBSuvrOkBn36126355;     VxpGIjMIBSuvrOkBn36126355 = VxpGIjMIBSuvrOkBn16283705;     VxpGIjMIBSuvrOkBn16283705 = VxpGIjMIBSuvrOkBn46322276;     VxpGIjMIBSuvrOkBn46322276 = VxpGIjMIBSuvrOkBn15335853;     VxpGIjMIBSuvrOkBn15335853 = VxpGIjMIBSuvrOkBn47548541;     VxpGIjMIBSuvrOkBn47548541 = VxpGIjMIBSuvrOkBn28672522;     VxpGIjMIBSuvrOkBn28672522 = VxpGIjMIBSuvrOkBn3795845;     VxpGIjMIBSuvrOkBn3795845 = VxpGIjMIBSuvrOkBn11181305;     VxpGIjMIBSuvrOkBn11181305 = VxpGIjMIBSuvrOkBn99555047;     VxpGIjMIBSuvrOkBn99555047 = VxpGIjMIBSuvrOkBn637526;     VxpGIjMIBSuvrOkBn637526 = VxpGIjMIBSuvrOkBn97836804;     VxpGIjMIBSuvrOkBn97836804 = VxpGIjMIBSuvrOkBn59031878;     VxpGIjMIBSuvrOkBn59031878 = VxpGIjMIBSuvrOkBn71047446;     VxpGIjMIBSuvrOkBn71047446 = VxpGIjMIBSuvrOkBn15782495;     VxpGIjMIBSuvrOkBn15782495 = VxpGIjMIBSuvrOkBn86351412;     VxpGIjMIBSuvrOkBn86351412 = VxpGIjMIBSuvrOkBn59948042;     VxpGIjMIBSuvrOkBn59948042 = VxpGIjMIBSuvrOkBn89317590;     VxpGIjMIBSuvrOkBn89317590 = VxpGIjMIBSuvrOkBn16839777;     VxpGIjMIBSuvrOkBn16839777 = VxpGIjMIBSuvrOkBn21121162;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void lJTDsDpRsahnhkII68172733() {     double cJzTmgaoaLWlPIFtP56580130 = -342863807;    double cJzTmgaoaLWlPIFtP22060778 = -349675694;    double cJzTmgaoaLWlPIFtP3620420 = -384505886;    double cJzTmgaoaLWlPIFtP64999522 = -274024967;    double cJzTmgaoaLWlPIFtP10135536 = -703971495;    double cJzTmgaoaLWlPIFtP20355040 = -148635158;    double cJzTmgaoaLWlPIFtP48803554 = 70993537;    double cJzTmgaoaLWlPIFtP62778273 = -202520301;    double cJzTmgaoaLWlPIFtP35705803 = -320478082;    double cJzTmgaoaLWlPIFtP45304349 = -445020528;    double cJzTmgaoaLWlPIFtP79426644 = -473626322;    double cJzTmgaoaLWlPIFtP24531477 = -936463581;    double cJzTmgaoaLWlPIFtP13182653 = -613214113;    double cJzTmgaoaLWlPIFtP7431269 = -176259208;    double cJzTmgaoaLWlPIFtP15582888 = -571116341;    double cJzTmgaoaLWlPIFtP17435535 = -590745552;    double cJzTmgaoaLWlPIFtP94060181 = -387717654;    double cJzTmgaoaLWlPIFtP77400139 = -855152819;    double cJzTmgaoaLWlPIFtP47996539 = 66029417;    double cJzTmgaoaLWlPIFtP26614081 = -248859558;    double cJzTmgaoaLWlPIFtP14781751 = -171882552;    double cJzTmgaoaLWlPIFtP15035967 = -171979738;    double cJzTmgaoaLWlPIFtP34066648 = -987761378;    double cJzTmgaoaLWlPIFtP38494700 = -164192776;    double cJzTmgaoaLWlPIFtP75013452 = -880645916;    double cJzTmgaoaLWlPIFtP55359320 = -566619848;    double cJzTmgaoaLWlPIFtP30019445 = -621814460;    double cJzTmgaoaLWlPIFtP39179408 = -615514372;    double cJzTmgaoaLWlPIFtP11188018 = -575096585;    double cJzTmgaoaLWlPIFtP67709074 = -357823366;    double cJzTmgaoaLWlPIFtP26585242 = -530463794;    double cJzTmgaoaLWlPIFtP87392829 = -556207365;    double cJzTmgaoaLWlPIFtP24358221 = -460024913;    double cJzTmgaoaLWlPIFtP32730516 = -851875690;    double cJzTmgaoaLWlPIFtP86496479 = -391030223;    double cJzTmgaoaLWlPIFtP96363323 = -376669593;    double cJzTmgaoaLWlPIFtP71581529 = 68942979;    double cJzTmgaoaLWlPIFtP74832874 = -32391780;    double cJzTmgaoaLWlPIFtP98611558 = -873382880;    double cJzTmgaoaLWlPIFtP49852275 = 30569746;    double cJzTmgaoaLWlPIFtP88929266 = -416223806;    double cJzTmgaoaLWlPIFtP11843449 = -513225178;    double cJzTmgaoaLWlPIFtP21343955 = -305636367;    double cJzTmgaoaLWlPIFtP51848555 = -682244596;    double cJzTmgaoaLWlPIFtP8531335 = 68101783;    double cJzTmgaoaLWlPIFtP59912910 = -971475300;    double cJzTmgaoaLWlPIFtP39572528 = -215771978;    double cJzTmgaoaLWlPIFtP76888800 = -890425945;    double cJzTmgaoaLWlPIFtP85217294 = -946694896;    double cJzTmgaoaLWlPIFtP72765688 = -754570963;    double cJzTmgaoaLWlPIFtP96624111 = -739926021;    double cJzTmgaoaLWlPIFtP76151630 = -482062059;    double cJzTmgaoaLWlPIFtP70642418 = -379209925;    double cJzTmgaoaLWlPIFtP99459534 = -286716514;    double cJzTmgaoaLWlPIFtP53390982 = -546644739;    double cJzTmgaoaLWlPIFtP41544164 = -70884069;    double cJzTmgaoaLWlPIFtP87994130 = -361914317;    double cJzTmgaoaLWlPIFtP65125719 = -120313111;    double cJzTmgaoaLWlPIFtP89986069 = -393379052;    double cJzTmgaoaLWlPIFtP54776216 = -37351648;    double cJzTmgaoaLWlPIFtP90335594 = -526820698;    double cJzTmgaoaLWlPIFtP9624146 = -313492091;    double cJzTmgaoaLWlPIFtP51590255 = -627423717;    double cJzTmgaoaLWlPIFtP67996728 = -962654716;    double cJzTmgaoaLWlPIFtP18719107 = -914556734;    double cJzTmgaoaLWlPIFtP92033815 = -917418958;    double cJzTmgaoaLWlPIFtP173256 = -376438669;    double cJzTmgaoaLWlPIFtP80452136 = -761338423;    double cJzTmgaoaLWlPIFtP20934790 = -785228986;    double cJzTmgaoaLWlPIFtP19219564 = -94446748;    double cJzTmgaoaLWlPIFtP45854006 = -559688532;    double cJzTmgaoaLWlPIFtP19227308 = -255325874;    double cJzTmgaoaLWlPIFtP78788581 = -981769939;    double cJzTmgaoaLWlPIFtP98144263 = -964540329;    double cJzTmgaoaLWlPIFtP37684815 = -832635752;    double cJzTmgaoaLWlPIFtP2938303 = -658657374;    double cJzTmgaoaLWlPIFtP93692011 = -866343371;    double cJzTmgaoaLWlPIFtP82218092 = -205516782;    double cJzTmgaoaLWlPIFtP29963366 = -132294559;    double cJzTmgaoaLWlPIFtP15100543 = -909170616;    double cJzTmgaoaLWlPIFtP15786793 = -250847870;    double cJzTmgaoaLWlPIFtP53130645 = -731388515;    double cJzTmgaoaLWlPIFtP53962114 = -668819477;    double cJzTmgaoaLWlPIFtP38422330 = -820525622;    double cJzTmgaoaLWlPIFtP71084962 = -617897345;    double cJzTmgaoaLWlPIFtP50433612 = 51598264;    double cJzTmgaoaLWlPIFtP16750411 = -76997440;    double cJzTmgaoaLWlPIFtP24898687 = -73308399;    double cJzTmgaoaLWlPIFtP79339534 = -205230952;    double cJzTmgaoaLWlPIFtP44952315 = -220146154;    double cJzTmgaoaLWlPIFtP8369193 = 85244723;    double cJzTmgaoaLWlPIFtP6455810 = -810743910;    double cJzTmgaoaLWlPIFtP84846804 = -639012729;    double cJzTmgaoaLWlPIFtP43835342 = -736031233;    double cJzTmgaoaLWlPIFtP59516680 = -442609556;    double cJzTmgaoaLWlPIFtP79305120 = -2731715;    double cJzTmgaoaLWlPIFtP60253193 = -885801462;    double cJzTmgaoaLWlPIFtP53347227 = -342981652;    double cJzTmgaoaLWlPIFtP33129448 = -767687862;    double cJzTmgaoaLWlPIFtP16497519 = -342863807;     cJzTmgaoaLWlPIFtP56580130 = cJzTmgaoaLWlPIFtP22060778;     cJzTmgaoaLWlPIFtP22060778 = cJzTmgaoaLWlPIFtP3620420;     cJzTmgaoaLWlPIFtP3620420 = cJzTmgaoaLWlPIFtP64999522;     cJzTmgaoaLWlPIFtP64999522 = cJzTmgaoaLWlPIFtP10135536;     cJzTmgaoaLWlPIFtP10135536 = cJzTmgaoaLWlPIFtP20355040;     cJzTmgaoaLWlPIFtP20355040 = cJzTmgaoaLWlPIFtP48803554;     cJzTmgaoaLWlPIFtP48803554 = cJzTmgaoaLWlPIFtP62778273;     cJzTmgaoaLWlPIFtP62778273 = cJzTmgaoaLWlPIFtP35705803;     cJzTmgaoaLWlPIFtP35705803 = cJzTmgaoaLWlPIFtP45304349;     cJzTmgaoaLWlPIFtP45304349 = cJzTmgaoaLWlPIFtP79426644;     cJzTmgaoaLWlPIFtP79426644 = cJzTmgaoaLWlPIFtP24531477;     cJzTmgaoaLWlPIFtP24531477 = cJzTmgaoaLWlPIFtP13182653;     cJzTmgaoaLWlPIFtP13182653 = cJzTmgaoaLWlPIFtP7431269;     cJzTmgaoaLWlPIFtP7431269 = cJzTmgaoaLWlPIFtP15582888;     cJzTmgaoaLWlPIFtP15582888 = cJzTmgaoaLWlPIFtP17435535;     cJzTmgaoaLWlPIFtP17435535 = cJzTmgaoaLWlPIFtP94060181;     cJzTmgaoaLWlPIFtP94060181 = cJzTmgaoaLWlPIFtP77400139;     cJzTmgaoaLWlPIFtP77400139 = cJzTmgaoaLWlPIFtP47996539;     cJzTmgaoaLWlPIFtP47996539 = cJzTmgaoaLWlPIFtP26614081;     cJzTmgaoaLWlPIFtP26614081 = cJzTmgaoaLWlPIFtP14781751;     cJzTmgaoaLWlPIFtP14781751 = cJzTmgaoaLWlPIFtP15035967;     cJzTmgaoaLWlPIFtP15035967 = cJzTmgaoaLWlPIFtP34066648;     cJzTmgaoaLWlPIFtP34066648 = cJzTmgaoaLWlPIFtP38494700;     cJzTmgaoaLWlPIFtP38494700 = cJzTmgaoaLWlPIFtP75013452;     cJzTmgaoaLWlPIFtP75013452 = cJzTmgaoaLWlPIFtP55359320;     cJzTmgaoaLWlPIFtP55359320 = cJzTmgaoaLWlPIFtP30019445;     cJzTmgaoaLWlPIFtP30019445 = cJzTmgaoaLWlPIFtP39179408;     cJzTmgaoaLWlPIFtP39179408 = cJzTmgaoaLWlPIFtP11188018;     cJzTmgaoaLWlPIFtP11188018 = cJzTmgaoaLWlPIFtP67709074;     cJzTmgaoaLWlPIFtP67709074 = cJzTmgaoaLWlPIFtP26585242;     cJzTmgaoaLWlPIFtP26585242 = cJzTmgaoaLWlPIFtP87392829;     cJzTmgaoaLWlPIFtP87392829 = cJzTmgaoaLWlPIFtP24358221;     cJzTmgaoaLWlPIFtP24358221 = cJzTmgaoaLWlPIFtP32730516;     cJzTmgaoaLWlPIFtP32730516 = cJzTmgaoaLWlPIFtP86496479;     cJzTmgaoaLWlPIFtP86496479 = cJzTmgaoaLWlPIFtP96363323;     cJzTmgaoaLWlPIFtP96363323 = cJzTmgaoaLWlPIFtP71581529;     cJzTmgaoaLWlPIFtP71581529 = cJzTmgaoaLWlPIFtP74832874;     cJzTmgaoaLWlPIFtP74832874 = cJzTmgaoaLWlPIFtP98611558;     cJzTmgaoaLWlPIFtP98611558 = cJzTmgaoaLWlPIFtP49852275;     cJzTmgaoaLWlPIFtP49852275 = cJzTmgaoaLWlPIFtP88929266;     cJzTmgaoaLWlPIFtP88929266 = cJzTmgaoaLWlPIFtP11843449;     cJzTmgaoaLWlPIFtP11843449 = cJzTmgaoaLWlPIFtP21343955;     cJzTmgaoaLWlPIFtP21343955 = cJzTmgaoaLWlPIFtP51848555;     cJzTmgaoaLWlPIFtP51848555 = cJzTmgaoaLWlPIFtP8531335;     cJzTmgaoaLWlPIFtP8531335 = cJzTmgaoaLWlPIFtP59912910;     cJzTmgaoaLWlPIFtP59912910 = cJzTmgaoaLWlPIFtP39572528;     cJzTmgaoaLWlPIFtP39572528 = cJzTmgaoaLWlPIFtP76888800;     cJzTmgaoaLWlPIFtP76888800 = cJzTmgaoaLWlPIFtP85217294;     cJzTmgaoaLWlPIFtP85217294 = cJzTmgaoaLWlPIFtP72765688;     cJzTmgaoaLWlPIFtP72765688 = cJzTmgaoaLWlPIFtP96624111;     cJzTmgaoaLWlPIFtP96624111 = cJzTmgaoaLWlPIFtP76151630;     cJzTmgaoaLWlPIFtP76151630 = cJzTmgaoaLWlPIFtP70642418;     cJzTmgaoaLWlPIFtP70642418 = cJzTmgaoaLWlPIFtP99459534;     cJzTmgaoaLWlPIFtP99459534 = cJzTmgaoaLWlPIFtP53390982;     cJzTmgaoaLWlPIFtP53390982 = cJzTmgaoaLWlPIFtP41544164;     cJzTmgaoaLWlPIFtP41544164 = cJzTmgaoaLWlPIFtP87994130;     cJzTmgaoaLWlPIFtP87994130 = cJzTmgaoaLWlPIFtP65125719;     cJzTmgaoaLWlPIFtP65125719 = cJzTmgaoaLWlPIFtP89986069;     cJzTmgaoaLWlPIFtP89986069 = cJzTmgaoaLWlPIFtP54776216;     cJzTmgaoaLWlPIFtP54776216 = cJzTmgaoaLWlPIFtP90335594;     cJzTmgaoaLWlPIFtP90335594 = cJzTmgaoaLWlPIFtP9624146;     cJzTmgaoaLWlPIFtP9624146 = cJzTmgaoaLWlPIFtP51590255;     cJzTmgaoaLWlPIFtP51590255 = cJzTmgaoaLWlPIFtP67996728;     cJzTmgaoaLWlPIFtP67996728 = cJzTmgaoaLWlPIFtP18719107;     cJzTmgaoaLWlPIFtP18719107 = cJzTmgaoaLWlPIFtP92033815;     cJzTmgaoaLWlPIFtP92033815 = cJzTmgaoaLWlPIFtP173256;     cJzTmgaoaLWlPIFtP173256 = cJzTmgaoaLWlPIFtP80452136;     cJzTmgaoaLWlPIFtP80452136 = cJzTmgaoaLWlPIFtP20934790;     cJzTmgaoaLWlPIFtP20934790 = cJzTmgaoaLWlPIFtP19219564;     cJzTmgaoaLWlPIFtP19219564 = cJzTmgaoaLWlPIFtP45854006;     cJzTmgaoaLWlPIFtP45854006 = cJzTmgaoaLWlPIFtP19227308;     cJzTmgaoaLWlPIFtP19227308 = cJzTmgaoaLWlPIFtP78788581;     cJzTmgaoaLWlPIFtP78788581 = cJzTmgaoaLWlPIFtP98144263;     cJzTmgaoaLWlPIFtP98144263 = cJzTmgaoaLWlPIFtP37684815;     cJzTmgaoaLWlPIFtP37684815 = cJzTmgaoaLWlPIFtP2938303;     cJzTmgaoaLWlPIFtP2938303 = cJzTmgaoaLWlPIFtP93692011;     cJzTmgaoaLWlPIFtP93692011 = cJzTmgaoaLWlPIFtP82218092;     cJzTmgaoaLWlPIFtP82218092 = cJzTmgaoaLWlPIFtP29963366;     cJzTmgaoaLWlPIFtP29963366 = cJzTmgaoaLWlPIFtP15100543;     cJzTmgaoaLWlPIFtP15100543 = cJzTmgaoaLWlPIFtP15786793;     cJzTmgaoaLWlPIFtP15786793 = cJzTmgaoaLWlPIFtP53130645;     cJzTmgaoaLWlPIFtP53130645 = cJzTmgaoaLWlPIFtP53962114;     cJzTmgaoaLWlPIFtP53962114 = cJzTmgaoaLWlPIFtP38422330;     cJzTmgaoaLWlPIFtP38422330 = cJzTmgaoaLWlPIFtP71084962;     cJzTmgaoaLWlPIFtP71084962 = cJzTmgaoaLWlPIFtP50433612;     cJzTmgaoaLWlPIFtP50433612 = cJzTmgaoaLWlPIFtP16750411;     cJzTmgaoaLWlPIFtP16750411 = cJzTmgaoaLWlPIFtP24898687;     cJzTmgaoaLWlPIFtP24898687 = cJzTmgaoaLWlPIFtP79339534;     cJzTmgaoaLWlPIFtP79339534 = cJzTmgaoaLWlPIFtP44952315;     cJzTmgaoaLWlPIFtP44952315 = cJzTmgaoaLWlPIFtP8369193;     cJzTmgaoaLWlPIFtP8369193 = cJzTmgaoaLWlPIFtP6455810;     cJzTmgaoaLWlPIFtP6455810 = cJzTmgaoaLWlPIFtP84846804;     cJzTmgaoaLWlPIFtP84846804 = cJzTmgaoaLWlPIFtP43835342;     cJzTmgaoaLWlPIFtP43835342 = cJzTmgaoaLWlPIFtP59516680;     cJzTmgaoaLWlPIFtP59516680 = cJzTmgaoaLWlPIFtP79305120;     cJzTmgaoaLWlPIFtP79305120 = cJzTmgaoaLWlPIFtP60253193;     cJzTmgaoaLWlPIFtP60253193 = cJzTmgaoaLWlPIFtP53347227;     cJzTmgaoaLWlPIFtP53347227 = cJzTmgaoaLWlPIFtP33129448;     cJzTmgaoaLWlPIFtP33129448 = cJzTmgaoaLWlPIFtP16497519;     cJzTmgaoaLWlPIFtP16497519 = cJzTmgaoaLWlPIFtP56580130;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VxFiqpuoMaIftDoU83221800() {     double gJptbFgyZITDsQoDe62697237 = -454765695;    double gJptbFgyZITDsQoDe65434057 = -59275475;    double gJptbFgyZITDsQoDe19662603 = -535338807;    double gJptbFgyZITDsQoDe92074303 = -464773597;    double gJptbFgyZITDsQoDe79031024 = -124621366;    double gJptbFgyZITDsQoDe11772242 = -413131393;    double gJptbFgyZITDsQoDe9292520 = -67971440;    double gJptbFgyZITDsQoDe12538164 = -489394214;    double gJptbFgyZITDsQoDe348298 = -88510747;    double gJptbFgyZITDsQoDe86534008 = -773996005;    double gJptbFgyZITDsQoDe39536935 = -351063908;    double gJptbFgyZITDsQoDe56354519 = -280869783;    double gJptbFgyZITDsQoDe99391771 = 46894799;    double gJptbFgyZITDsQoDe90574945 = -708497776;    double gJptbFgyZITDsQoDe11325314 = -161578507;    double gJptbFgyZITDsQoDe58940928 = -563745095;    double gJptbFgyZITDsQoDe87896185 = 95737234;    double gJptbFgyZITDsQoDe74453292 = -520675462;    double gJptbFgyZITDsQoDe9826381 = -339739680;    double gJptbFgyZITDsQoDe7164246 = -605762477;    double gJptbFgyZITDsQoDe3334944 = -27370008;    double gJptbFgyZITDsQoDe1556990 = -79909455;    double gJptbFgyZITDsQoDe69657439 = -734278865;    double gJptbFgyZITDsQoDe22487074 = -386497763;    double gJptbFgyZITDsQoDe18713103 = -153841949;    double gJptbFgyZITDsQoDe80080995 = -445275171;    double gJptbFgyZITDsQoDe28479223 = -522476663;    double gJptbFgyZITDsQoDe52286534 = -681443326;    double gJptbFgyZITDsQoDe77320273 = -797479289;    double gJptbFgyZITDsQoDe36324711 = -175294233;    double gJptbFgyZITDsQoDe18827792 = -443784732;    double gJptbFgyZITDsQoDe85003451 = -265654180;    double gJptbFgyZITDsQoDe34069170 = -352638283;    double gJptbFgyZITDsQoDe27561081 = -603417455;    double gJptbFgyZITDsQoDe47971131 = 70236672;    double gJptbFgyZITDsQoDe65089236 = -590695701;    double gJptbFgyZITDsQoDe52843464 = -984692890;    double gJptbFgyZITDsQoDe8923087 = -903897790;    double gJptbFgyZITDsQoDe72221811 = -867306377;    double gJptbFgyZITDsQoDe37354213 = -442869477;    double gJptbFgyZITDsQoDe94598490 = -481497701;    double gJptbFgyZITDsQoDe43993905 = -783367456;    double gJptbFgyZITDsQoDe99410932 = -386440951;    double gJptbFgyZITDsQoDe39256295 = -701587534;    double gJptbFgyZITDsQoDe20483800 = -693796371;    double gJptbFgyZITDsQoDe71852105 = -956691417;    double gJptbFgyZITDsQoDe57351102 = -334547516;    double gJptbFgyZITDsQoDe84845741 = -900321478;    double gJptbFgyZITDsQoDe94893023 = 67568116;    double gJptbFgyZITDsQoDe85677340 = -841581771;    double gJptbFgyZITDsQoDe32324790 = -582219693;    double gJptbFgyZITDsQoDe86961349 = -282788379;    double gJptbFgyZITDsQoDe18890876 = -170999513;    double gJptbFgyZITDsQoDe95428414 = -607186659;    double gJptbFgyZITDsQoDe79886856 = -147176521;    double gJptbFgyZITDsQoDe61140247 = -274856241;    double gJptbFgyZITDsQoDe95776618 = -324996611;    double gJptbFgyZITDsQoDe97175528 = -48841044;    double gJptbFgyZITDsQoDe73361200 = -210931649;    double gJptbFgyZITDsQoDe98950028 = -679346196;    double gJptbFgyZITDsQoDe83293019 = -890654730;    double gJptbFgyZITDsQoDe57005986 = -386528115;    double gJptbFgyZITDsQoDe35217891 = -691914926;    double gJptbFgyZITDsQoDe64023586 = -913216514;    double gJptbFgyZITDsQoDe67706216 = -230211274;    double gJptbFgyZITDsQoDe54533484 = 14590271;    double gJptbFgyZITDsQoDe22285350 = -928231501;    double gJptbFgyZITDsQoDe71830690 = -349687747;    double gJptbFgyZITDsQoDe42603814 = -678734449;    double gJptbFgyZITDsQoDe46236078 = -570882807;    double gJptbFgyZITDsQoDe6097465 = -579052205;    double gJptbFgyZITDsQoDe78973098 = -364977;    double gJptbFgyZITDsQoDe2231481 = -653369085;    double gJptbFgyZITDsQoDe72472168 = -896870203;    double gJptbFgyZITDsQoDe12565756 = -24264777;    double gJptbFgyZITDsQoDe59341039 = -244002552;    double gJptbFgyZITDsQoDe2146058 = -693468504;    double gJptbFgyZITDsQoDe30401144 = 67308669;    double gJptbFgyZITDsQoDe2003275 = -692701393;    double gJptbFgyZITDsQoDe46860998 = -197150533;    double gJptbFgyZITDsQoDe22729893 = -10727656;    double gJptbFgyZITDsQoDe43633482 = -622155186;    double gJptbFgyZITDsQoDe57393510 = -649011443;    double gJptbFgyZITDsQoDe91642932 = -955897518;    double gJptbFgyZITDsQoDe3999921 = -593074541;    double gJptbFgyZITDsQoDe31866442 = -60996354;    double gJptbFgyZITDsQoDe66112575 = 5345332;    double gJptbFgyZITDsQoDe38640755 = -745451625;    double gJptbFgyZITDsQoDe47674224 = -356240935;    double gJptbFgyZITDsQoDe86830883 = -654907087;    double gJptbFgyZITDsQoDe69312618 = -165699090;    double gJptbFgyZITDsQoDe55667935 = -835851847;    double gJptbFgyZITDsQoDe35561886 = -592966141;    double gJptbFgyZITDsQoDe73271782 = -87960182;    double gJptbFgyZITDsQoDe54061194 = -552214747;    double gJptbFgyZITDsQoDe37592504 = 5030414;    double gJptbFgyZITDsQoDe8776015 = 8547469;    double gJptbFgyZITDsQoDe35387346 = -473224437;    double gJptbFgyZITDsQoDe71550078 = -371376261;    double gJptbFgyZITDsQoDe65950316 = -454765695;     gJptbFgyZITDsQoDe62697237 = gJptbFgyZITDsQoDe65434057;     gJptbFgyZITDsQoDe65434057 = gJptbFgyZITDsQoDe19662603;     gJptbFgyZITDsQoDe19662603 = gJptbFgyZITDsQoDe92074303;     gJptbFgyZITDsQoDe92074303 = gJptbFgyZITDsQoDe79031024;     gJptbFgyZITDsQoDe79031024 = gJptbFgyZITDsQoDe11772242;     gJptbFgyZITDsQoDe11772242 = gJptbFgyZITDsQoDe9292520;     gJptbFgyZITDsQoDe9292520 = gJptbFgyZITDsQoDe12538164;     gJptbFgyZITDsQoDe12538164 = gJptbFgyZITDsQoDe348298;     gJptbFgyZITDsQoDe348298 = gJptbFgyZITDsQoDe86534008;     gJptbFgyZITDsQoDe86534008 = gJptbFgyZITDsQoDe39536935;     gJptbFgyZITDsQoDe39536935 = gJptbFgyZITDsQoDe56354519;     gJptbFgyZITDsQoDe56354519 = gJptbFgyZITDsQoDe99391771;     gJptbFgyZITDsQoDe99391771 = gJptbFgyZITDsQoDe90574945;     gJptbFgyZITDsQoDe90574945 = gJptbFgyZITDsQoDe11325314;     gJptbFgyZITDsQoDe11325314 = gJptbFgyZITDsQoDe58940928;     gJptbFgyZITDsQoDe58940928 = gJptbFgyZITDsQoDe87896185;     gJptbFgyZITDsQoDe87896185 = gJptbFgyZITDsQoDe74453292;     gJptbFgyZITDsQoDe74453292 = gJptbFgyZITDsQoDe9826381;     gJptbFgyZITDsQoDe9826381 = gJptbFgyZITDsQoDe7164246;     gJptbFgyZITDsQoDe7164246 = gJptbFgyZITDsQoDe3334944;     gJptbFgyZITDsQoDe3334944 = gJptbFgyZITDsQoDe1556990;     gJptbFgyZITDsQoDe1556990 = gJptbFgyZITDsQoDe69657439;     gJptbFgyZITDsQoDe69657439 = gJptbFgyZITDsQoDe22487074;     gJptbFgyZITDsQoDe22487074 = gJptbFgyZITDsQoDe18713103;     gJptbFgyZITDsQoDe18713103 = gJptbFgyZITDsQoDe80080995;     gJptbFgyZITDsQoDe80080995 = gJptbFgyZITDsQoDe28479223;     gJptbFgyZITDsQoDe28479223 = gJptbFgyZITDsQoDe52286534;     gJptbFgyZITDsQoDe52286534 = gJptbFgyZITDsQoDe77320273;     gJptbFgyZITDsQoDe77320273 = gJptbFgyZITDsQoDe36324711;     gJptbFgyZITDsQoDe36324711 = gJptbFgyZITDsQoDe18827792;     gJptbFgyZITDsQoDe18827792 = gJptbFgyZITDsQoDe85003451;     gJptbFgyZITDsQoDe85003451 = gJptbFgyZITDsQoDe34069170;     gJptbFgyZITDsQoDe34069170 = gJptbFgyZITDsQoDe27561081;     gJptbFgyZITDsQoDe27561081 = gJptbFgyZITDsQoDe47971131;     gJptbFgyZITDsQoDe47971131 = gJptbFgyZITDsQoDe65089236;     gJptbFgyZITDsQoDe65089236 = gJptbFgyZITDsQoDe52843464;     gJptbFgyZITDsQoDe52843464 = gJptbFgyZITDsQoDe8923087;     gJptbFgyZITDsQoDe8923087 = gJptbFgyZITDsQoDe72221811;     gJptbFgyZITDsQoDe72221811 = gJptbFgyZITDsQoDe37354213;     gJptbFgyZITDsQoDe37354213 = gJptbFgyZITDsQoDe94598490;     gJptbFgyZITDsQoDe94598490 = gJptbFgyZITDsQoDe43993905;     gJptbFgyZITDsQoDe43993905 = gJptbFgyZITDsQoDe99410932;     gJptbFgyZITDsQoDe99410932 = gJptbFgyZITDsQoDe39256295;     gJptbFgyZITDsQoDe39256295 = gJptbFgyZITDsQoDe20483800;     gJptbFgyZITDsQoDe20483800 = gJptbFgyZITDsQoDe71852105;     gJptbFgyZITDsQoDe71852105 = gJptbFgyZITDsQoDe57351102;     gJptbFgyZITDsQoDe57351102 = gJptbFgyZITDsQoDe84845741;     gJptbFgyZITDsQoDe84845741 = gJptbFgyZITDsQoDe94893023;     gJptbFgyZITDsQoDe94893023 = gJptbFgyZITDsQoDe85677340;     gJptbFgyZITDsQoDe85677340 = gJptbFgyZITDsQoDe32324790;     gJptbFgyZITDsQoDe32324790 = gJptbFgyZITDsQoDe86961349;     gJptbFgyZITDsQoDe86961349 = gJptbFgyZITDsQoDe18890876;     gJptbFgyZITDsQoDe18890876 = gJptbFgyZITDsQoDe95428414;     gJptbFgyZITDsQoDe95428414 = gJptbFgyZITDsQoDe79886856;     gJptbFgyZITDsQoDe79886856 = gJptbFgyZITDsQoDe61140247;     gJptbFgyZITDsQoDe61140247 = gJptbFgyZITDsQoDe95776618;     gJptbFgyZITDsQoDe95776618 = gJptbFgyZITDsQoDe97175528;     gJptbFgyZITDsQoDe97175528 = gJptbFgyZITDsQoDe73361200;     gJptbFgyZITDsQoDe73361200 = gJptbFgyZITDsQoDe98950028;     gJptbFgyZITDsQoDe98950028 = gJptbFgyZITDsQoDe83293019;     gJptbFgyZITDsQoDe83293019 = gJptbFgyZITDsQoDe57005986;     gJptbFgyZITDsQoDe57005986 = gJptbFgyZITDsQoDe35217891;     gJptbFgyZITDsQoDe35217891 = gJptbFgyZITDsQoDe64023586;     gJptbFgyZITDsQoDe64023586 = gJptbFgyZITDsQoDe67706216;     gJptbFgyZITDsQoDe67706216 = gJptbFgyZITDsQoDe54533484;     gJptbFgyZITDsQoDe54533484 = gJptbFgyZITDsQoDe22285350;     gJptbFgyZITDsQoDe22285350 = gJptbFgyZITDsQoDe71830690;     gJptbFgyZITDsQoDe71830690 = gJptbFgyZITDsQoDe42603814;     gJptbFgyZITDsQoDe42603814 = gJptbFgyZITDsQoDe46236078;     gJptbFgyZITDsQoDe46236078 = gJptbFgyZITDsQoDe6097465;     gJptbFgyZITDsQoDe6097465 = gJptbFgyZITDsQoDe78973098;     gJptbFgyZITDsQoDe78973098 = gJptbFgyZITDsQoDe2231481;     gJptbFgyZITDsQoDe2231481 = gJptbFgyZITDsQoDe72472168;     gJptbFgyZITDsQoDe72472168 = gJptbFgyZITDsQoDe12565756;     gJptbFgyZITDsQoDe12565756 = gJptbFgyZITDsQoDe59341039;     gJptbFgyZITDsQoDe59341039 = gJptbFgyZITDsQoDe2146058;     gJptbFgyZITDsQoDe2146058 = gJptbFgyZITDsQoDe30401144;     gJptbFgyZITDsQoDe30401144 = gJptbFgyZITDsQoDe2003275;     gJptbFgyZITDsQoDe2003275 = gJptbFgyZITDsQoDe46860998;     gJptbFgyZITDsQoDe46860998 = gJptbFgyZITDsQoDe22729893;     gJptbFgyZITDsQoDe22729893 = gJptbFgyZITDsQoDe43633482;     gJptbFgyZITDsQoDe43633482 = gJptbFgyZITDsQoDe57393510;     gJptbFgyZITDsQoDe57393510 = gJptbFgyZITDsQoDe91642932;     gJptbFgyZITDsQoDe91642932 = gJptbFgyZITDsQoDe3999921;     gJptbFgyZITDsQoDe3999921 = gJptbFgyZITDsQoDe31866442;     gJptbFgyZITDsQoDe31866442 = gJptbFgyZITDsQoDe66112575;     gJptbFgyZITDsQoDe66112575 = gJptbFgyZITDsQoDe38640755;     gJptbFgyZITDsQoDe38640755 = gJptbFgyZITDsQoDe47674224;     gJptbFgyZITDsQoDe47674224 = gJptbFgyZITDsQoDe86830883;     gJptbFgyZITDsQoDe86830883 = gJptbFgyZITDsQoDe69312618;     gJptbFgyZITDsQoDe69312618 = gJptbFgyZITDsQoDe55667935;     gJptbFgyZITDsQoDe55667935 = gJptbFgyZITDsQoDe35561886;     gJptbFgyZITDsQoDe35561886 = gJptbFgyZITDsQoDe73271782;     gJptbFgyZITDsQoDe73271782 = gJptbFgyZITDsQoDe54061194;     gJptbFgyZITDsQoDe54061194 = gJptbFgyZITDsQoDe37592504;     gJptbFgyZITDsQoDe37592504 = gJptbFgyZITDsQoDe8776015;     gJptbFgyZITDsQoDe8776015 = gJptbFgyZITDsQoDe35387346;     gJptbFgyZITDsQoDe35387346 = gJptbFgyZITDsQoDe71550078;     gJptbFgyZITDsQoDe71550078 = gJptbFgyZITDsQoDe65950316;     gJptbFgyZITDsQoDe65950316 = gJptbFgyZITDsQoDe62697237;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void KgNYxocLsadhWqRm50513400() {     double QurdutHWeUqngycVY98156205 = -921042595;    double QurdutHWeUqngycVY78706135 = 60172918;    double QurdutHWeUqngycVY57049079 = -201005882;    double QurdutHWeUqngycVY64297658 = -285924669;    double QurdutHWeUqngycVY35316294 = -230220212;    double QurdutHWeUqngycVY47995114 = -675479288;    double QurdutHWeUqngycVY19949926 = 87302825;    double QurdutHWeUqngycVY74669724 = -133628355;    double QurdutHWeUqngycVY1865903 = -576842154;    double QurdutHWeUqngycVY1436721 = -438683003;    double QurdutHWeUqngycVY77718670 = -153071638;    double QurdutHWeUqngycVY56922668 = -144990555;    double QurdutHWeUqngycVY37626417 = -505117706;    double QurdutHWeUqngycVY43715809 = -684861185;    double QurdutHWeUqngycVY69519830 = -665003522;    double QurdutHWeUqngycVY42482229 = -199556702;    double QurdutHWeUqngycVY43732121 = -431237319;    double QurdutHWeUqngycVY74213483 = -594103531;    double QurdutHWeUqngycVY10376711 = -656303101;    double QurdutHWeUqngycVY1001464 = 67676457;    double QurdutHWeUqngycVY16421807 = -784644697;    double QurdutHWeUqngycVY4978515 = -166545749;    double QurdutHWeUqngycVY40182819 = -371911049;    double QurdutHWeUqngycVY16280019 = -897322368;    double QurdutHWeUqngycVY26731054 = -177827147;    double QurdutHWeUqngycVY90779167 = -133851978;    double QurdutHWeUqngycVY88899797 = -328760803;    double QurdutHWeUqngycVY93014958 = -851215500;    double QurdutHWeUqngycVY23988488 = -271574091;    double QurdutHWeUqngycVY41858122 = -209801404;    double QurdutHWeUqngycVY37411023 = -995765219;    double QurdutHWeUqngycVY22826470 = -38625595;    double QurdutHWeUqngycVY46287180 = -500234260;    double QurdutHWeUqngycVY57309507 = -668111268;    double QurdutHWeUqngycVY13779585 = -980863579;    double QurdutHWeUqngycVY16650081 = -805261006;    double QurdutHWeUqngycVY2255281 = -849388931;    double QurdutHWeUqngycVY60138490 = -886553371;    double QurdutHWeUqngycVY2612374 = -518182654;    double QurdutHWeUqngycVY1625746 = 91867977;    double QurdutHWeUqngycVY28050098 = -250696615;    double QurdutHWeUqngycVY33359032 = -377912747;    double QurdutHWeUqngycVY88794311 = 60378263;    double QurdutHWeUqngycVY79387633 = -205919944;    double QurdutHWeUqngycVY20500259 = -520307533;    double QurdutHWeUqngycVY17813434 = -995615049;    double QurdutHWeUqngycVY76696266 = -818533877;    double QurdutHWeUqngycVY2572053 = 33151929;    double QurdutHWeUqngycVY17785690 = -696168764;    double QurdutHWeUqngycVY10206930 = -232608587;    double QurdutHWeUqngycVY13095514 = -456496250;    double QurdutHWeUqngycVY70446822 = -496440726;    double QurdutHWeUqngycVY87512024 = 72954933;    double QurdutHWeUqngycVY11420262 = -798695159;    double QurdutHWeUqngycVY34091593 = -493666667;    double QurdutHWeUqngycVY93177691 = -654496847;    double QurdutHWeUqngycVY38523317 = -567916033;    double QurdutHWeUqngycVY40769060 = -303683515;    double QurdutHWeUqngycVY37566604 = -8097522;    double QurdutHWeUqngycVY44537127 = 3631766;    double QurdutHWeUqngycVY59095317 = -246718485;    double QurdutHWeUqngycVY26934968 = -61481675;    double QurdutHWeUqngycVY50681237 = -862054264;    double QurdutHWeUqngycVY60007781 = -267040751;    double QurdutHWeUqngycVY64025697 = -442917785;    double QurdutHWeUqngycVY54892200 = -14446043;    double QurdutHWeUqngycVY10635488 = -644756295;    double QurdutHWeUqngycVY80316909 = -837006438;    double QurdutHWeUqngycVY29936224 = -703997607;    double QurdutHWeUqngycVY52869750 = -859742516;    double QurdutHWeUqngycVY40226949 = -350167772;    double QurdutHWeUqngycVY83593631 = -544683948;    double QurdutHWeUqngycVY71601110 = 24079122;    double QurdutHWeUqngycVY8750966 = -648171079;    double QurdutHWeUqngycVY72951366 = -681626929;    double QurdutHWeUqngycVY83062775 = -306731950;    double QurdutHWeUqngycVY16184204 = -126924012;    double QurdutHWeUqngycVY60795185 = -65991105;    double QurdutHWeUqngycVY95779760 = -277014835;    double QurdutHWeUqngycVY8917621 = -182212099;    double QurdutHWeUqngycVY14082902 = -315318102;    double QurdutHWeUqngycVY86327744 = -261912733;    double QurdutHWeUqngycVY75229269 = -55046737;    double QurdutHWeUqngycVY13781558 = 61034495;    double QurdutHWeUqngycVY28762608 = -753305154;    double QurdutHWeUqngycVY66964201 = -399324494;    double QurdutHWeUqngycVY35314446 = -11580529;    double QurdutHWeUqngycVY34866919 = -701539102;    double QurdutHWeUqngycVY23217914 = -74444602;    double QurdutHWeUqngycVY20601894 = -226366732;    double QurdutHWeUqngycVY78126763 = -137344974;    double QurdutHWeUqngycVY61486220 = -445705416;    double QurdutHWeUqngycVY22571886 = -778455850;    double QurdutHWeUqngycVY58075247 = -421814420;    double QurdutHWeUqngycVY42530428 = -661413539;    double QurdutHWeUqngycVY1115131 = -89214940;    double QurdutHWeUqngycVY82677795 = -515858484;    double QurdutHWeUqngycVY28786530 = -672580987;    double QurdutHWeUqngycVY15361937 = -763002160;    double QurdutHWeUqngycVY65608058 = -921042595;     QurdutHWeUqngycVY98156205 = QurdutHWeUqngycVY78706135;     QurdutHWeUqngycVY78706135 = QurdutHWeUqngycVY57049079;     QurdutHWeUqngycVY57049079 = QurdutHWeUqngycVY64297658;     QurdutHWeUqngycVY64297658 = QurdutHWeUqngycVY35316294;     QurdutHWeUqngycVY35316294 = QurdutHWeUqngycVY47995114;     QurdutHWeUqngycVY47995114 = QurdutHWeUqngycVY19949926;     QurdutHWeUqngycVY19949926 = QurdutHWeUqngycVY74669724;     QurdutHWeUqngycVY74669724 = QurdutHWeUqngycVY1865903;     QurdutHWeUqngycVY1865903 = QurdutHWeUqngycVY1436721;     QurdutHWeUqngycVY1436721 = QurdutHWeUqngycVY77718670;     QurdutHWeUqngycVY77718670 = QurdutHWeUqngycVY56922668;     QurdutHWeUqngycVY56922668 = QurdutHWeUqngycVY37626417;     QurdutHWeUqngycVY37626417 = QurdutHWeUqngycVY43715809;     QurdutHWeUqngycVY43715809 = QurdutHWeUqngycVY69519830;     QurdutHWeUqngycVY69519830 = QurdutHWeUqngycVY42482229;     QurdutHWeUqngycVY42482229 = QurdutHWeUqngycVY43732121;     QurdutHWeUqngycVY43732121 = QurdutHWeUqngycVY74213483;     QurdutHWeUqngycVY74213483 = QurdutHWeUqngycVY10376711;     QurdutHWeUqngycVY10376711 = QurdutHWeUqngycVY1001464;     QurdutHWeUqngycVY1001464 = QurdutHWeUqngycVY16421807;     QurdutHWeUqngycVY16421807 = QurdutHWeUqngycVY4978515;     QurdutHWeUqngycVY4978515 = QurdutHWeUqngycVY40182819;     QurdutHWeUqngycVY40182819 = QurdutHWeUqngycVY16280019;     QurdutHWeUqngycVY16280019 = QurdutHWeUqngycVY26731054;     QurdutHWeUqngycVY26731054 = QurdutHWeUqngycVY90779167;     QurdutHWeUqngycVY90779167 = QurdutHWeUqngycVY88899797;     QurdutHWeUqngycVY88899797 = QurdutHWeUqngycVY93014958;     QurdutHWeUqngycVY93014958 = QurdutHWeUqngycVY23988488;     QurdutHWeUqngycVY23988488 = QurdutHWeUqngycVY41858122;     QurdutHWeUqngycVY41858122 = QurdutHWeUqngycVY37411023;     QurdutHWeUqngycVY37411023 = QurdutHWeUqngycVY22826470;     QurdutHWeUqngycVY22826470 = QurdutHWeUqngycVY46287180;     QurdutHWeUqngycVY46287180 = QurdutHWeUqngycVY57309507;     QurdutHWeUqngycVY57309507 = QurdutHWeUqngycVY13779585;     QurdutHWeUqngycVY13779585 = QurdutHWeUqngycVY16650081;     QurdutHWeUqngycVY16650081 = QurdutHWeUqngycVY2255281;     QurdutHWeUqngycVY2255281 = QurdutHWeUqngycVY60138490;     QurdutHWeUqngycVY60138490 = QurdutHWeUqngycVY2612374;     QurdutHWeUqngycVY2612374 = QurdutHWeUqngycVY1625746;     QurdutHWeUqngycVY1625746 = QurdutHWeUqngycVY28050098;     QurdutHWeUqngycVY28050098 = QurdutHWeUqngycVY33359032;     QurdutHWeUqngycVY33359032 = QurdutHWeUqngycVY88794311;     QurdutHWeUqngycVY88794311 = QurdutHWeUqngycVY79387633;     QurdutHWeUqngycVY79387633 = QurdutHWeUqngycVY20500259;     QurdutHWeUqngycVY20500259 = QurdutHWeUqngycVY17813434;     QurdutHWeUqngycVY17813434 = QurdutHWeUqngycVY76696266;     QurdutHWeUqngycVY76696266 = QurdutHWeUqngycVY2572053;     QurdutHWeUqngycVY2572053 = QurdutHWeUqngycVY17785690;     QurdutHWeUqngycVY17785690 = QurdutHWeUqngycVY10206930;     QurdutHWeUqngycVY10206930 = QurdutHWeUqngycVY13095514;     QurdutHWeUqngycVY13095514 = QurdutHWeUqngycVY70446822;     QurdutHWeUqngycVY70446822 = QurdutHWeUqngycVY87512024;     QurdutHWeUqngycVY87512024 = QurdutHWeUqngycVY11420262;     QurdutHWeUqngycVY11420262 = QurdutHWeUqngycVY34091593;     QurdutHWeUqngycVY34091593 = QurdutHWeUqngycVY93177691;     QurdutHWeUqngycVY93177691 = QurdutHWeUqngycVY38523317;     QurdutHWeUqngycVY38523317 = QurdutHWeUqngycVY40769060;     QurdutHWeUqngycVY40769060 = QurdutHWeUqngycVY37566604;     QurdutHWeUqngycVY37566604 = QurdutHWeUqngycVY44537127;     QurdutHWeUqngycVY44537127 = QurdutHWeUqngycVY59095317;     QurdutHWeUqngycVY59095317 = QurdutHWeUqngycVY26934968;     QurdutHWeUqngycVY26934968 = QurdutHWeUqngycVY50681237;     QurdutHWeUqngycVY50681237 = QurdutHWeUqngycVY60007781;     QurdutHWeUqngycVY60007781 = QurdutHWeUqngycVY64025697;     QurdutHWeUqngycVY64025697 = QurdutHWeUqngycVY54892200;     QurdutHWeUqngycVY54892200 = QurdutHWeUqngycVY10635488;     QurdutHWeUqngycVY10635488 = QurdutHWeUqngycVY80316909;     QurdutHWeUqngycVY80316909 = QurdutHWeUqngycVY29936224;     QurdutHWeUqngycVY29936224 = QurdutHWeUqngycVY52869750;     QurdutHWeUqngycVY52869750 = QurdutHWeUqngycVY40226949;     QurdutHWeUqngycVY40226949 = QurdutHWeUqngycVY83593631;     QurdutHWeUqngycVY83593631 = QurdutHWeUqngycVY71601110;     QurdutHWeUqngycVY71601110 = QurdutHWeUqngycVY8750966;     QurdutHWeUqngycVY8750966 = QurdutHWeUqngycVY72951366;     QurdutHWeUqngycVY72951366 = QurdutHWeUqngycVY83062775;     QurdutHWeUqngycVY83062775 = QurdutHWeUqngycVY16184204;     QurdutHWeUqngycVY16184204 = QurdutHWeUqngycVY60795185;     QurdutHWeUqngycVY60795185 = QurdutHWeUqngycVY95779760;     QurdutHWeUqngycVY95779760 = QurdutHWeUqngycVY8917621;     QurdutHWeUqngycVY8917621 = QurdutHWeUqngycVY14082902;     QurdutHWeUqngycVY14082902 = QurdutHWeUqngycVY86327744;     QurdutHWeUqngycVY86327744 = QurdutHWeUqngycVY75229269;     QurdutHWeUqngycVY75229269 = QurdutHWeUqngycVY13781558;     QurdutHWeUqngycVY13781558 = QurdutHWeUqngycVY28762608;     QurdutHWeUqngycVY28762608 = QurdutHWeUqngycVY66964201;     QurdutHWeUqngycVY66964201 = QurdutHWeUqngycVY35314446;     QurdutHWeUqngycVY35314446 = QurdutHWeUqngycVY34866919;     QurdutHWeUqngycVY34866919 = QurdutHWeUqngycVY23217914;     QurdutHWeUqngycVY23217914 = QurdutHWeUqngycVY20601894;     QurdutHWeUqngycVY20601894 = QurdutHWeUqngycVY78126763;     QurdutHWeUqngycVY78126763 = QurdutHWeUqngycVY61486220;     QurdutHWeUqngycVY61486220 = QurdutHWeUqngycVY22571886;     QurdutHWeUqngycVY22571886 = QurdutHWeUqngycVY58075247;     QurdutHWeUqngycVY58075247 = QurdutHWeUqngycVY42530428;     QurdutHWeUqngycVY42530428 = QurdutHWeUqngycVY1115131;     QurdutHWeUqngycVY1115131 = QurdutHWeUqngycVY82677795;     QurdutHWeUqngycVY82677795 = QurdutHWeUqngycVY28786530;     QurdutHWeUqngycVY28786530 = QurdutHWeUqngycVY15361937;     QurdutHWeUqngycVY15361937 = QurdutHWeUqngycVY65608058;     QurdutHWeUqngycVY65608058 = QurdutHWeUqngycVY98156205;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void uCBgrElGDmGwrbDD65562468() {     double wYcxACMzfDetdAlvs4273313 = 67055516;    double wYcxACMzfDetdAlvs22079415 = -749426862;    double wYcxACMzfDetdAlvs73091263 = -351838803;    double wYcxACMzfDetdAlvs91372438 = -476673299;    double wYcxACMzfDetdAlvs4211782 = -750870083;    double wYcxACMzfDetdAlvs39412317 = -939975523;    double wYcxACMzfDetdAlvs80438892 = -51662152;    double wYcxACMzfDetdAlvs24429616 = -420502268;    double wYcxACMzfDetdAlvs66508398 = -344874820;    double wYcxACMzfDetdAlvs42666380 = -767658481;    double wYcxACMzfDetdAlvs37828960 = -30509224;    double wYcxACMzfDetdAlvs88745710 = -589396756;    double wYcxACMzfDetdAlvs23835535 = -945008795;    double wYcxACMzfDetdAlvs26859486 = -117099753;    double wYcxACMzfDetdAlvs65262257 = -255465689;    double wYcxACMzfDetdAlvs83987623 = -172556246;    double wYcxACMzfDetdAlvs37568124 = 52217568;    double wYcxACMzfDetdAlvs71266636 = -259626173;    double wYcxACMzfDetdAlvs72206552 = 37927802;    double wYcxACMzfDetdAlvs81551628 = -289226462;    double wYcxACMzfDetdAlvs4975001 = -640132152;    double wYcxACMzfDetdAlvs91499538 = -74475465;    double wYcxACMzfDetdAlvs75773610 = -118428536;    double wYcxACMzfDetdAlvs272393 = -19627355;    double wYcxACMzfDetdAlvs70430704 = -551023180;    double wYcxACMzfDetdAlvs15500843 = -12507301;    double wYcxACMzfDetdAlvs87359575 = -229423006;    double wYcxACMzfDetdAlvs6122084 = -917144454;    double wYcxACMzfDetdAlvs90120743 = -493956796;    double wYcxACMzfDetdAlvs10473759 = -27272271;    double wYcxACMzfDetdAlvs29653573 = -909086157;    double wYcxACMzfDetdAlvs20437092 = -848072410;    double wYcxACMzfDetdAlvs55998129 = -392847631;    double wYcxACMzfDetdAlvs52140071 = -419653034;    double wYcxACMzfDetdAlvs75254237 = -519596684;    double wYcxACMzfDetdAlvs85375993 = 80712886;    double wYcxACMzfDetdAlvs83517215 = -803024800;    double wYcxACMzfDetdAlvs94228702 = -658059381;    double wYcxACMzfDetdAlvs76222627 = -512106150;    double wYcxACMzfDetdAlvs89127683 = -381571246;    double wYcxACMzfDetdAlvs33719322 = -315970509;    double wYcxACMzfDetdAlvs65509489 = -648055026;    double wYcxACMzfDetdAlvs66861289 = -20426321;    double wYcxACMzfDetdAlvs66795374 = -225262882;    double wYcxACMzfDetdAlvs32452723 = -182205687;    double wYcxACMzfDetdAlvs29752629 = -980831165;    double wYcxACMzfDetdAlvs94474840 = -937309414;    double wYcxACMzfDetdAlvs10528994 = 23256397;    double wYcxACMzfDetdAlvs27461419 = -781905752;    double wYcxACMzfDetdAlvs23118583 = -319619395;    double wYcxACMzfDetdAlvs48796192 = -298789921;    double wYcxACMzfDetdAlvs81256541 = -297167046;    double wYcxACMzfDetdAlvs35760482 = -818834654;    double wYcxACMzfDetdAlvs7389142 = -19165304;    double wYcxACMzfDetdAlvs60587468 = -94198449;    double wYcxACMzfDetdAlvs12773774 = -858469020;    double wYcxACMzfDetdAlvs46305804 = -530998327;    double wYcxACMzfDetdAlvs72818870 = -232211449;    double wYcxACMzfDetdAlvs20941735 = -925650119;    double wYcxACMzfDetdAlvs88710939 = -638362782;    double wYcxACMzfDetdAlvs52052741 = -610552517;    double wYcxACMzfDetdAlvs74316808 = -134517698;    double wYcxACMzfDetdAlvs34308872 = -926545472;    double wYcxACMzfDetdAlvs56034639 = -217602549;    double wYcxACMzfDetdAlvs13012807 = -858572324;    double wYcxACMzfDetdAlvs17391869 = -182436814;    double wYcxACMzfDetdAlvs32747582 = -96549126;    double wYcxACMzfDetdAlvs71695464 = -425355762;    double wYcxACMzfDetdAlvs51605249 = -597503070;    double wYcxACMzfDetdAlvs79886263 = -236178575;    double wYcxACMzfDetdAlvs470409 = -369531446;    double wYcxACMzfDetdAlvs43339422 = -289723052;    double wYcxACMzfDetdAlvs95044009 = -747520023;    double wYcxACMzfDetdAlvs83078869 = -580500953;    double wYcxACMzfDetdAlvs47832307 = -973255954;    double wYcxACMzfDetdAlvs39465511 = -992077127;    double wYcxACMzfDetdAlvs24638249 = 45950856;    double wYcxACMzfDetdAlvs8978237 = -893165654;    double wYcxACMzfDetdAlvs67819670 = -837421669;    double wYcxACMzfDetdAlvs40678076 = -570192015;    double wYcxACMzfDetdAlvs21026003 = -75197887;    double wYcxACMzfDetdAlvs76830581 = -152679404;    double wYcxACMzfDetdAlvs78660665 = -35238702;    double wYcxACMzfDetdAlvs67002161 = -74337401;    double wYcxACMzfDetdAlvs61677566 = -728482350;    double wYcxACMzfDetdAlvs48397031 = -511919112;    double wYcxACMzfDetdAlvs84676609 = 70762243;    double wYcxACMzfDetdAlvs48608987 = -273682327;    double wYcxACMzfDetdAlvs91552603 = -225454585;    double wYcxACMzfDetdAlvs62480463 = -661127665;    double wYcxACMzfDetdAlvs39070189 = -388288788;    double wYcxACMzfDetdAlvs10698345 = -470813352;    double wYcxACMzfDetdAlvs73286967 = -732409262;    double wYcxACMzfDetdAlvs87511687 = -873743369;    double wYcxACMzfDetdAlvs37074942 = -771018729;    double wYcxACMzfDetdAlvs59402513 = -81452811;    double wYcxACMzfDetdAlvs31200617 = -721509554;    double wYcxACMzfDetdAlvs10826650 = -802823773;    double wYcxACMzfDetdAlvs53782567 = -366690559;    double wYcxACMzfDetdAlvs15060855 = 67055516;     wYcxACMzfDetdAlvs4273313 = wYcxACMzfDetdAlvs22079415;     wYcxACMzfDetdAlvs22079415 = wYcxACMzfDetdAlvs73091263;     wYcxACMzfDetdAlvs73091263 = wYcxACMzfDetdAlvs91372438;     wYcxACMzfDetdAlvs91372438 = wYcxACMzfDetdAlvs4211782;     wYcxACMzfDetdAlvs4211782 = wYcxACMzfDetdAlvs39412317;     wYcxACMzfDetdAlvs39412317 = wYcxACMzfDetdAlvs80438892;     wYcxACMzfDetdAlvs80438892 = wYcxACMzfDetdAlvs24429616;     wYcxACMzfDetdAlvs24429616 = wYcxACMzfDetdAlvs66508398;     wYcxACMzfDetdAlvs66508398 = wYcxACMzfDetdAlvs42666380;     wYcxACMzfDetdAlvs42666380 = wYcxACMzfDetdAlvs37828960;     wYcxACMzfDetdAlvs37828960 = wYcxACMzfDetdAlvs88745710;     wYcxACMzfDetdAlvs88745710 = wYcxACMzfDetdAlvs23835535;     wYcxACMzfDetdAlvs23835535 = wYcxACMzfDetdAlvs26859486;     wYcxACMzfDetdAlvs26859486 = wYcxACMzfDetdAlvs65262257;     wYcxACMzfDetdAlvs65262257 = wYcxACMzfDetdAlvs83987623;     wYcxACMzfDetdAlvs83987623 = wYcxACMzfDetdAlvs37568124;     wYcxACMzfDetdAlvs37568124 = wYcxACMzfDetdAlvs71266636;     wYcxACMzfDetdAlvs71266636 = wYcxACMzfDetdAlvs72206552;     wYcxACMzfDetdAlvs72206552 = wYcxACMzfDetdAlvs81551628;     wYcxACMzfDetdAlvs81551628 = wYcxACMzfDetdAlvs4975001;     wYcxACMzfDetdAlvs4975001 = wYcxACMzfDetdAlvs91499538;     wYcxACMzfDetdAlvs91499538 = wYcxACMzfDetdAlvs75773610;     wYcxACMzfDetdAlvs75773610 = wYcxACMzfDetdAlvs272393;     wYcxACMzfDetdAlvs272393 = wYcxACMzfDetdAlvs70430704;     wYcxACMzfDetdAlvs70430704 = wYcxACMzfDetdAlvs15500843;     wYcxACMzfDetdAlvs15500843 = wYcxACMzfDetdAlvs87359575;     wYcxACMzfDetdAlvs87359575 = wYcxACMzfDetdAlvs6122084;     wYcxACMzfDetdAlvs6122084 = wYcxACMzfDetdAlvs90120743;     wYcxACMzfDetdAlvs90120743 = wYcxACMzfDetdAlvs10473759;     wYcxACMzfDetdAlvs10473759 = wYcxACMzfDetdAlvs29653573;     wYcxACMzfDetdAlvs29653573 = wYcxACMzfDetdAlvs20437092;     wYcxACMzfDetdAlvs20437092 = wYcxACMzfDetdAlvs55998129;     wYcxACMzfDetdAlvs55998129 = wYcxACMzfDetdAlvs52140071;     wYcxACMzfDetdAlvs52140071 = wYcxACMzfDetdAlvs75254237;     wYcxACMzfDetdAlvs75254237 = wYcxACMzfDetdAlvs85375993;     wYcxACMzfDetdAlvs85375993 = wYcxACMzfDetdAlvs83517215;     wYcxACMzfDetdAlvs83517215 = wYcxACMzfDetdAlvs94228702;     wYcxACMzfDetdAlvs94228702 = wYcxACMzfDetdAlvs76222627;     wYcxACMzfDetdAlvs76222627 = wYcxACMzfDetdAlvs89127683;     wYcxACMzfDetdAlvs89127683 = wYcxACMzfDetdAlvs33719322;     wYcxACMzfDetdAlvs33719322 = wYcxACMzfDetdAlvs65509489;     wYcxACMzfDetdAlvs65509489 = wYcxACMzfDetdAlvs66861289;     wYcxACMzfDetdAlvs66861289 = wYcxACMzfDetdAlvs66795374;     wYcxACMzfDetdAlvs66795374 = wYcxACMzfDetdAlvs32452723;     wYcxACMzfDetdAlvs32452723 = wYcxACMzfDetdAlvs29752629;     wYcxACMzfDetdAlvs29752629 = wYcxACMzfDetdAlvs94474840;     wYcxACMzfDetdAlvs94474840 = wYcxACMzfDetdAlvs10528994;     wYcxACMzfDetdAlvs10528994 = wYcxACMzfDetdAlvs27461419;     wYcxACMzfDetdAlvs27461419 = wYcxACMzfDetdAlvs23118583;     wYcxACMzfDetdAlvs23118583 = wYcxACMzfDetdAlvs48796192;     wYcxACMzfDetdAlvs48796192 = wYcxACMzfDetdAlvs81256541;     wYcxACMzfDetdAlvs81256541 = wYcxACMzfDetdAlvs35760482;     wYcxACMzfDetdAlvs35760482 = wYcxACMzfDetdAlvs7389142;     wYcxACMzfDetdAlvs7389142 = wYcxACMzfDetdAlvs60587468;     wYcxACMzfDetdAlvs60587468 = wYcxACMzfDetdAlvs12773774;     wYcxACMzfDetdAlvs12773774 = wYcxACMzfDetdAlvs46305804;     wYcxACMzfDetdAlvs46305804 = wYcxACMzfDetdAlvs72818870;     wYcxACMzfDetdAlvs72818870 = wYcxACMzfDetdAlvs20941735;     wYcxACMzfDetdAlvs20941735 = wYcxACMzfDetdAlvs88710939;     wYcxACMzfDetdAlvs88710939 = wYcxACMzfDetdAlvs52052741;     wYcxACMzfDetdAlvs52052741 = wYcxACMzfDetdAlvs74316808;     wYcxACMzfDetdAlvs74316808 = wYcxACMzfDetdAlvs34308872;     wYcxACMzfDetdAlvs34308872 = wYcxACMzfDetdAlvs56034639;     wYcxACMzfDetdAlvs56034639 = wYcxACMzfDetdAlvs13012807;     wYcxACMzfDetdAlvs13012807 = wYcxACMzfDetdAlvs17391869;     wYcxACMzfDetdAlvs17391869 = wYcxACMzfDetdAlvs32747582;     wYcxACMzfDetdAlvs32747582 = wYcxACMzfDetdAlvs71695464;     wYcxACMzfDetdAlvs71695464 = wYcxACMzfDetdAlvs51605249;     wYcxACMzfDetdAlvs51605249 = wYcxACMzfDetdAlvs79886263;     wYcxACMzfDetdAlvs79886263 = wYcxACMzfDetdAlvs470409;     wYcxACMzfDetdAlvs470409 = wYcxACMzfDetdAlvs43339422;     wYcxACMzfDetdAlvs43339422 = wYcxACMzfDetdAlvs95044009;     wYcxACMzfDetdAlvs95044009 = wYcxACMzfDetdAlvs83078869;     wYcxACMzfDetdAlvs83078869 = wYcxACMzfDetdAlvs47832307;     wYcxACMzfDetdAlvs47832307 = wYcxACMzfDetdAlvs39465511;     wYcxACMzfDetdAlvs39465511 = wYcxACMzfDetdAlvs24638249;     wYcxACMzfDetdAlvs24638249 = wYcxACMzfDetdAlvs8978237;     wYcxACMzfDetdAlvs8978237 = wYcxACMzfDetdAlvs67819670;     wYcxACMzfDetdAlvs67819670 = wYcxACMzfDetdAlvs40678076;     wYcxACMzfDetdAlvs40678076 = wYcxACMzfDetdAlvs21026003;     wYcxACMzfDetdAlvs21026003 = wYcxACMzfDetdAlvs76830581;     wYcxACMzfDetdAlvs76830581 = wYcxACMzfDetdAlvs78660665;     wYcxACMzfDetdAlvs78660665 = wYcxACMzfDetdAlvs67002161;     wYcxACMzfDetdAlvs67002161 = wYcxACMzfDetdAlvs61677566;     wYcxACMzfDetdAlvs61677566 = wYcxACMzfDetdAlvs48397031;     wYcxACMzfDetdAlvs48397031 = wYcxACMzfDetdAlvs84676609;     wYcxACMzfDetdAlvs84676609 = wYcxACMzfDetdAlvs48608987;     wYcxACMzfDetdAlvs48608987 = wYcxACMzfDetdAlvs91552603;     wYcxACMzfDetdAlvs91552603 = wYcxACMzfDetdAlvs62480463;     wYcxACMzfDetdAlvs62480463 = wYcxACMzfDetdAlvs39070189;     wYcxACMzfDetdAlvs39070189 = wYcxACMzfDetdAlvs10698345;     wYcxACMzfDetdAlvs10698345 = wYcxACMzfDetdAlvs73286967;     wYcxACMzfDetdAlvs73286967 = wYcxACMzfDetdAlvs87511687;     wYcxACMzfDetdAlvs87511687 = wYcxACMzfDetdAlvs37074942;     wYcxACMzfDetdAlvs37074942 = wYcxACMzfDetdAlvs59402513;     wYcxACMzfDetdAlvs59402513 = wYcxACMzfDetdAlvs31200617;     wYcxACMzfDetdAlvs31200617 = wYcxACMzfDetdAlvs10826650;     wYcxACMzfDetdAlvs10826650 = wYcxACMzfDetdAlvs53782567;     wYcxACMzfDetdAlvs53782567 = wYcxACMzfDetdAlvs15060855;     wYcxACMzfDetdAlvs15060855 = wYcxACMzfDetdAlvs4273313;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void vPyUHsQJRNegBhbV2207443() {     double AKbbrrSSkHVINQLMz26660430 = -667348447;    double AKbbrrSSkHVINQLMz6426916 = -697969917;    double AKbbrrSSkHVINQLMz1860542 = -591492648;    double AKbbrrSSkHVINQLMz6379128 = -483080831;    double AKbbrrSSkHVINQLMz2386037 = -241927084;    double AKbbrrSSkHVINQLMz54295433 = -800583901;    double AKbbrrSSkHVINQLMz72594631 = -973649458;    double AKbbrrSSkHVINQLMz63475 = -891098912;    double AKbbrrSSkHVINQLMz63671529 = -229070859;    double AKbbrrSSkHVINQLMz34429964 = -510399814;    double AKbbrrSSkHVINQLMz75370820 = -27133625;    double AKbbrrSSkHVINQLMz44648659 = -840142050;    double AKbbrrSSkHVINQLMz90843715 = -463726115;    double AKbbrrSSkHVINQLMz69474237 = -644808510;    double AKbbrrSSkHVINQLMz1997535 = -982943402;    double AKbbrrSSkHVINQLMz28243536 = -554223788;    double AKbbrrSSkHVINQLMz87391475 = -225062252;    double AKbbrrSSkHVINQLMz77243053 = -711368864;    double AKbbrrSSkHVINQLMz13488184 = -97174323;    double AKbbrrSSkHVINQLMz60067912 = -541860916;    double AKbbrrSSkHVINQLMz5858108 = -631619461;    double AKbbrrSSkHVINQLMz55314757 = -579241778;    double AKbbrrSSkHVINQLMz79066934 = -294509128;    double AKbbrrSSkHVINQLMz26772180 = -583620213;    double AKbbrrSSkHVINQLMz5970951 = -172582305;    double AKbbrrSSkHVINQLMz3803837 = -456401525;    double AKbbrrSSkHVINQLMz34448996 = 12990501;    double AKbbrrSSkHVINQLMz12033534 = -705598908;    double AKbbrrSSkHVINQLMz97013304 = -838213915;    double AKbbrrSSkHVINQLMz19630939 = 52431862;    double AKbbrrSSkHVINQLMz73944377 = -990402309;    double AKbbrrSSkHVINQLMz77978282 = -400143765;    double AKbbrrSSkHVINQLMz37036799 = -922191125;    double AKbbrrSSkHVINQLMz11528759 = -743779884;    double AKbbrrSSkHVINQLMz51483601 = -583353107;    double AKbbrrSSkHVINQLMz27068863 = -234682490;    double AKbbrrSSkHVINQLMz76956927 = -535972752;    double AKbbrrSSkHVINQLMz47854803 = -525684853;    double AKbbrrSSkHVINQLMz24530759 = -913152182;    double AKbbrrSSkHVINQLMz9313398 = -517795275;    double AKbbrrSSkHVINQLMz62476692 = -819148175;    double AKbbrrSSkHVINQLMz54017880 = -405963717;    double AKbbrrSSkHVINQLMz72411481 = 92043095;    double AKbbrrSSkHVINQLMz66239493 = -222626531;    double AKbbrrSSkHVINQLMz435990 = 8650835;    double AKbbrrSSkHVINQLMz37852910 = -63060261;    double AKbbrrSSkHVINQLMz6772238 = -415719667;    double AKbbrrSSkHVINQLMz55127668 = -833278594;    double AKbbrrSSkHVINQLMz21921324 = -731622450;    double AKbbrrSSkHVINQLMz20202329 = -461639654;    double AKbbrrSSkHVINQLMz57665409 = -653866198;    double AKbbrrSSkHVINQLMz55107799 = -727986328;    double AKbbrrSSkHVINQLMz14074885 = -490745885;    double AKbbrrSSkHVINQLMz67675687 = -294846112;    double AKbbrrSSkHVINQLMz96349335 = -234902564;    double AKbbrrSSkHVINQLMz71345673 = 11893331;    double AKbbrrSSkHVINQLMz27359982 = -303460790;    double AKbbrrSSkHVINQLMz75088361 = 92127565;    double AKbbrrSSkHVINQLMz408177 = -210498526;    double AKbbrrSSkHVINQLMz98582199 = -785525560;    double AKbbrrSSkHVINQLMz19846438 = -713574402;    double AKbbrrSSkHVINQLMz60561097 = -168050551;    double AKbbrrSSkHVINQLMz3050171 = 47115003;    double AKbbrrSSkHVINQLMz44040591 = -181502722;    double AKbbrrSSkHVINQLMz60485586 = -519997505;    double AKbbrrSSkHVINQLMz97392537 = -626989861;    double AKbbrrSSkHVINQLMz7611860 = -917950925;    double AKbbrrSSkHVINQLMz79314957 = -719946232;    double AKbbrrSSkHVINQLMz17990637 = 38544596;    double AKbbrrSSkHVINQLMz74928671 = -648260912;    double AKbbrrSSkHVINQLMz51286609 = 81748963;    double AKbbrrSSkHVINQLMz39536673 = -699377399;    double AKbbrrSSkHVINQLMz52712295 = -798216682;    double AKbbrrSSkHVINQLMz4174786 = -579379049;    double AKbbrrSSkHVINQLMz97591219 = -722712742;    double AKbbrrSSkHVINQLMz51840227 = -125655744;    double AKbbrrSSkHVINQLMz82903275 = -571284874;    double AKbbrrSSkHVINQLMz12827441 = 28117403;    double AKbbrrSSkHVINQLMz26336190 = -492271048;    double AKbbrrSSkHVINQLMz68118040 = -9522044;    double AKbbrrSSkHVINQLMz97031599 = 59318142;    double AKbbrrSSkHVINQLMz79321327 = -153730906;    double AKbbrrSSkHVINQLMz90112210 = -973976458;    double AKbbrrSSkHVINQLMz76810976 = -276574261;    double AKbbrrSSkHVINQLMz61965529 = -293701940;    double AKbbrrSSkHVINQLMz18836579 = -162415982;    double AKbbrrSSkHVINQLMz63903397 = -909397881;    double AKbbrrSSkHVINQLMz69361112 = -527345013;    double AKbbrrSSkHVINQLMz15179424 = -408877320;    double AKbbrrSSkHVINQLMz80137928 = -495246438;    double AKbbrrSSkHVINQLMz99708880 = -931221701;    double AKbbrrSSkHVINQLMz1868566 = -528100317;    double AKbbrrSSkHVINQLMz47446627 = -215186327;    double AKbbrrSSkHVINQLMz25948559 = -27626623;    double AKbbrrSSkHVINQLMz89466959 = -804220873;    double AKbbrrSSkHVINQLMz1915596 = -551097625;    double AKbbrrSSkHVINQLMz50967710 = -353078720;    double AKbbrrSSkHVINQLMz28370890 = -726454184;    double AKbbrrSSkHVINQLMz5753907 = -702629027;    double AKbbrrSSkHVINQLMz3043453 = -667348447;     AKbbrrSSkHVINQLMz26660430 = AKbbrrSSkHVINQLMz6426916;     AKbbrrSSkHVINQLMz6426916 = AKbbrrSSkHVINQLMz1860542;     AKbbrrSSkHVINQLMz1860542 = AKbbrrSSkHVINQLMz6379128;     AKbbrrSSkHVINQLMz6379128 = AKbbrrSSkHVINQLMz2386037;     AKbbrrSSkHVINQLMz2386037 = AKbbrrSSkHVINQLMz54295433;     AKbbrrSSkHVINQLMz54295433 = AKbbrrSSkHVINQLMz72594631;     AKbbrrSSkHVINQLMz72594631 = AKbbrrSSkHVINQLMz63475;     AKbbrrSSkHVINQLMz63475 = AKbbrrSSkHVINQLMz63671529;     AKbbrrSSkHVINQLMz63671529 = AKbbrrSSkHVINQLMz34429964;     AKbbrrSSkHVINQLMz34429964 = AKbbrrSSkHVINQLMz75370820;     AKbbrrSSkHVINQLMz75370820 = AKbbrrSSkHVINQLMz44648659;     AKbbrrSSkHVINQLMz44648659 = AKbbrrSSkHVINQLMz90843715;     AKbbrrSSkHVINQLMz90843715 = AKbbrrSSkHVINQLMz69474237;     AKbbrrSSkHVINQLMz69474237 = AKbbrrSSkHVINQLMz1997535;     AKbbrrSSkHVINQLMz1997535 = AKbbrrSSkHVINQLMz28243536;     AKbbrrSSkHVINQLMz28243536 = AKbbrrSSkHVINQLMz87391475;     AKbbrrSSkHVINQLMz87391475 = AKbbrrSSkHVINQLMz77243053;     AKbbrrSSkHVINQLMz77243053 = AKbbrrSSkHVINQLMz13488184;     AKbbrrSSkHVINQLMz13488184 = AKbbrrSSkHVINQLMz60067912;     AKbbrrSSkHVINQLMz60067912 = AKbbrrSSkHVINQLMz5858108;     AKbbrrSSkHVINQLMz5858108 = AKbbrrSSkHVINQLMz55314757;     AKbbrrSSkHVINQLMz55314757 = AKbbrrSSkHVINQLMz79066934;     AKbbrrSSkHVINQLMz79066934 = AKbbrrSSkHVINQLMz26772180;     AKbbrrSSkHVINQLMz26772180 = AKbbrrSSkHVINQLMz5970951;     AKbbrrSSkHVINQLMz5970951 = AKbbrrSSkHVINQLMz3803837;     AKbbrrSSkHVINQLMz3803837 = AKbbrrSSkHVINQLMz34448996;     AKbbrrSSkHVINQLMz34448996 = AKbbrrSSkHVINQLMz12033534;     AKbbrrSSkHVINQLMz12033534 = AKbbrrSSkHVINQLMz97013304;     AKbbrrSSkHVINQLMz97013304 = AKbbrrSSkHVINQLMz19630939;     AKbbrrSSkHVINQLMz19630939 = AKbbrrSSkHVINQLMz73944377;     AKbbrrSSkHVINQLMz73944377 = AKbbrrSSkHVINQLMz77978282;     AKbbrrSSkHVINQLMz77978282 = AKbbrrSSkHVINQLMz37036799;     AKbbrrSSkHVINQLMz37036799 = AKbbrrSSkHVINQLMz11528759;     AKbbrrSSkHVINQLMz11528759 = AKbbrrSSkHVINQLMz51483601;     AKbbrrSSkHVINQLMz51483601 = AKbbrrSSkHVINQLMz27068863;     AKbbrrSSkHVINQLMz27068863 = AKbbrrSSkHVINQLMz76956927;     AKbbrrSSkHVINQLMz76956927 = AKbbrrSSkHVINQLMz47854803;     AKbbrrSSkHVINQLMz47854803 = AKbbrrSSkHVINQLMz24530759;     AKbbrrSSkHVINQLMz24530759 = AKbbrrSSkHVINQLMz9313398;     AKbbrrSSkHVINQLMz9313398 = AKbbrrSSkHVINQLMz62476692;     AKbbrrSSkHVINQLMz62476692 = AKbbrrSSkHVINQLMz54017880;     AKbbrrSSkHVINQLMz54017880 = AKbbrrSSkHVINQLMz72411481;     AKbbrrSSkHVINQLMz72411481 = AKbbrrSSkHVINQLMz66239493;     AKbbrrSSkHVINQLMz66239493 = AKbbrrSSkHVINQLMz435990;     AKbbrrSSkHVINQLMz435990 = AKbbrrSSkHVINQLMz37852910;     AKbbrrSSkHVINQLMz37852910 = AKbbrrSSkHVINQLMz6772238;     AKbbrrSSkHVINQLMz6772238 = AKbbrrSSkHVINQLMz55127668;     AKbbrrSSkHVINQLMz55127668 = AKbbrrSSkHVINQLMz21921324;     AKbbrrSSkHVINQLMz21921324 = AKbbrrSSkHVINQLMz20202329;     AKbbrrSSkHVINQLMz20202329 = AKbbrrSSkHVINQLMz57665409;     AKbbrrSSkHVINQLMz57665409 = AKbbrrSSkHVINQLMz55107799;     AKbbrrSSkHVINQLMz55107799 = AKbbrrSSkHVINQLMz14074885;     AKbbrrSSkHVINQLMz14074885 = AKbbrrSSkHVINQLMz67675687;     AKbbrrSSkHVINQLMz67675687 = AKbbrrSSkHVINQLMz96349335;     AKbbrrSSkHVINQLMz96349335 = AKbbrrSSkHVINQLMz71345673;     AKbbrrSSkHVINQLMz71345673 = AKbbrrSSkHVINQLMz27359982;     AKbbrrSSkHVINQLMz27359982 = AKbbrrSSkHVINQLMz75088361;     AKbbrrSSkHVINQLMz75088361 = AKbbrrSSkHVINQLMz408177;     AKbbrrSSkHVINQLMz408177 = AKbbrrSSkHVINQLMz98582199;     AKbbrrSSkHVINQLMz98582199 = AKbbrrSSkHVINQLMz19846438;     AKbbrrSSkHVINQLMz19846438 = AKbbrrSSkHVINQLMz60561097;     AKbbrrSSkHVINQLMz60561097 = AKbbrrSSkHVINQLMz3050171;     AKbbrrSSkHVINQLMz3050171 = AKbbrrSSkHVINQLMz44040591;     AKbbrrSSkHVINQLMz44040591 = AKbbrrSSkHVINQLMz60485586;     AKbbrrSSkHVINQLMz60485586 = AKbbrrSSkHVINQLMz97392537;     AKbbrrSSkHVINQLMz97392537 = AKbbrrSSkHVINQLMz7611860;     AKbbrrSSkHVINQLMz7611860 = AKbbrrSSkHVINQLMz79314957;     AKbbrrSSkHVINQLMz79314957 = AKbbrrSSkHVINQLMz17990637;     AKbbrrSSkHVINQLMz17990637 = AKbbrrSSkHVINQLMz74928671;     AKbbrrSSkHVINQLMz74928671 = AKbbrrSSkHVINQLMz51286609;     AKbbrrSSkHVINQLMz51286609 = AKbbrrSSkHVINQLMz39536673;     AKbbrrSSkHVINQLMz39536673 = AKbbrrSSkHVINQLMz52712295;     AKbbrrSSkHVINQLMz52712295 = AKbbrrSSkHVINQLMz4174786;     AKbbrrSSkHVINQLMz4174786 = AKbbrrSSkHVINQLMz97591219;     AKbbrrSSkHVINQLMz97591219 = AKbbrrSSkHVINQLMz51840227;     AKbbrrSSkHVINQLMz51840227 = AKbbrrSSkHVINQLMz82903275;     AKbbrrSSkHVINQLMz82903275 = AKbbrrSSkHVINQLMz12827441;     AKbbrrSSkHVINQLMz12827441 = AKbbrrSSkHVINQLMz26336190;     AKbbrrSSkHVINQLMz26336190 = AKbbrrSSkHVINQLMz68118040;     AKbbrrSSkHVINQLMz68118040 = AKbbrrSSkHVINQLMz97031599;     AKbbrrSSkHVINQLMz97031599 = AKbbrrSSkHVINQLMz79321327;     AKbbrrSSkHVINQLMz79321327 = AKbbrrSSkHVINQLMz90112210;     AKbbrrSSkHVINQLMz90112210 = AKbbrrSSkHVINQLMz76810976;     AKbbrrSSkHVINQLMz76810976 = AKbbrrSSkHVINQLMz61965529;     AKbbrrSSkHVINQLMz61965529 = AKbbrrSSkHVINQLMz18836579;     AKbbrrSSkHVINQLMz18836579 = AKbbrrSSkHVINQLMz63903397;     AKbbrrSSkHVINQLMz63903397 = AKbbrrSSkHVINQLMz69361112;     AKbbrrSSkHVINQLMz69361112 = AKbbrrSSkHVINQLMz15179424;     AKbbrrSSkHVINQLMz15179424 = AKbbrrSSkHVINQLMz80137928;     AKbbrrSSkHVINQLMz80137928 = AKbbrrSSkHVINQLMz99708880;     AKbbrrSSkHVINQLMz99708880 = AKbbrrSSkHVINQLMz1868566;     AKbbrrSSkHVINQLMz1868566 = AKbbrrSSkHVINQLMz47446627;     AKbbrrSSkHVINQLMz47446627 = AKbbrrSSkHVINQLMz25948559;     AKbbrrSSkHVINQLMz25948559 = AKbbrrSSkHVINQLMz89466959;     AKbbrrSSkHVINQLMz89466959 = AKbbrrSSkHVINQLMz1915596;     AKbbrrSSkHVINQLMz1915596 = AKbbrrSSkHVINQLMz50967710;     AKbbrrSSkHVINQLMz50967710 = AKbbrrSSkHVINQLMz28370890;     AKbbrrSSkHVINQLMz28370890 = AKbbrrSSkHVINQLMz5753907;     AKbbrrSSkHVINQLMz5753907 = AKbbrrSSkHVINQLMz3043453;     AKbbrrSSkHVINQLMz3043453 = AKbbrrSSkHVINQLMz26660430;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GsdfBlOwvLqANYxt69499042() {     double IKKkVtKUUgFhIafmG62119398 = -33625347;    double IKKkVtKUUgFhIafmG19698994 = -578521523;    double IKKkVtKUUgFhIafmG39247019 = -257159723;    double IKKkVtKUUgFhIafmG78602482 = -304231902;    double IKKkVtKUUgFhIafmG58671305 = -347525930;    double IKKkVtKUUgFhIafmG90518305 = 37068204;    double IKKkVtKUUgFhIafmG83252037 = -818375192;    double IKKkVtKUUgFhIafmG62195035 = -535333052;    double IKKkVtKUUgFhIafmG65189135 = -717402266;    double IKKkVtKUUgFhIafmG49332676 = -175086812;    double IKKkVtKUUgFhIafmG13552555 = -929141354;    double IKKkVtKUUgFhIafmG45216808 = -704262821;    double IKKkVtKUUgFhIafmG29078361 = 84261380;    double IKKkVtKUUgFhIafmG22615101 = -621171920;    double IKKkVtKUUgFhIafmG60192050 = -386368417;    double IKKkVtKUUgFhIafmG11784837 = -190035395;    double IKKkVtKUUgFhIafmG43227412 = -752036804;    double IKKkVtKUUgFhIafmG77003244 = -784796933;    double IKKkVtKUUgFhIafmG14038514 = -413737745;    double IKKkVtKUUgFhIafmG53905130 = -968421982;    double IKKkVtKUUgFhIafmG18944971 = -288894149;    double IKKkVtKUUgFhIafmG58736281 = -665878072;    double IKKkVtKUUgFhIafmG49592314 = 67858688;    double IKKkVtKUUgFhIafmG20565126 = 5555183;    double IKKkVtKUUgFhIafmG13988902 = -196567503;    double IKKkVtKUUgFhIafmG14502009 = -144978332;    double IKKkVtKUUgFhIafmG94869569 = -893293640;    double IKKkVtKUUgFhIafmG52761959 = -875371082;    double IKKkVtKUUgFhIafmG43681519 = -312308718;    double IKKkVtKUUgFhIafmG25164350 = 17924692;    double IKKkVtKUUgFhIafmG92527609 = -442382796;    double IKKkVtKUUgFhIafmG15801301 = -173115179;    double IKKkVtKUUgFhIafmG49254810 = 30212897;    double IKKkVtKUUgFhIafmG41277185 = -808473697;    double IKKkVtKUUgFhIafmG17292055 = -534453357;    double IKKkVtKUUgFhIafmG78629706 = -449247796;    double IKKkVtKUUgFhIafmG26368744 = -400668792;    double IKKkVtKUUgFhIafmG99070206 = -508340434;    double IKKkVtKUUgFhIafmG54921320 = -564028459;    double IKKkVtKUUgFhIafmG73584930 = 16942179;    double IKKkVtKUUgFhIafmG95928300 = -588347089;    double IKKkVtKUUgFhIafmG43383007 = -509007;    double IKKkVtKUUgFhIafmG61794860 = -561137691;    double IKKkVtKUUgFhIafmG6370832 = -826958941;    double IKKkVtKUUgFhIafmG452449 = -917860326;    double IKKkVtKUUgFhIafmG83814238 = -101983893;    double IKKkVtKUUgFhIafmG26117401 = -899706028;    double IKKkVtKUUgFhIafmG72853980 = -999805188;    double IKKkVtKUUgFhIafmG44813989 = -395359331;    double IKKkVtKUUgFhIafmG44731918 = -952666470;    double IKKkVtKUUgFhIafmG38436133 = -528142756;    double IKKkVtKUUgFhIafmG38593271 = -941638674;    double IKKkVtKUUgFhIafmG82696033 = -246791439;    double IKKkVtKUUgFhIafmG83667533 = -486354612;    double IKKkVtKUUgFhIafmG50554072 = -581392710;    double IKKkVtKUUgFhIafmG3383117 = -367747275;    double IKKkVtKUUgFhIafmG70106680 = -546380212;    double IKKkVtKUUgFhIafmG18681894 = -162714906;    double IKKkVtKUUgFhIafmG64613580 = -7664399;    double IKKkVtKUUgFhIafmG44169297 = -102547598;    double IKKkVtKUUgFhIafmG95648735 = -69638157;    double IKKkVtKUUgFhIafmG30490079 = -943004111;    double IKKkVtKUUgFhIafmG18513517 = -123024335;    double IKKkVtKUUgFhIafmG40024785 = -635326958;    double IKKkVtKUUgFhIafmG56805067 = -732704016;    double IKKkVtKUUgFhIafmG97751253 = -656026175;    double IKKkVtKUUgFhIafmG95961997 = -634475719;    double IKKkVtKUUgFhIafmG87801176 = -107264923;    double IKKkVtKUUgFhIafmG5323046 = 13281437;    double IKKkVtKUUgFhIafmG81562344 = -937120621;    double IKKkVtKUUgFhIafmG85416093 = -789366604;    double IKKkVtKUUgFhIafmG44157205 = -143696371;    double IKKkVtKUUgFhIafmG22081925 = -120768475;    double IKKkVtKUUgFhIafmG40453584 = -330679924;    double IKKkVtKUUgFhIafmG57976830 = -280074893;    double IKKkVtKUUgFhIafmG75561963 = -188385142;    double IKKkVtKUUgFhIafmG96941421 = -4740382;    double IKKkVtKUUgFhIafmG43221482 = -105182371;    double IKKkVtKUUgFhIafmG20112677 = -76584491;    double IKKkVtKUUgFhIafmG30174664 = 5416390;    double IKKkVtKUUgFhIafmG88384607 = -245272305;    double IKKkVtKUUgFhIafmG22015590 = -893488453;    double IKKkVtKUUgFhIafmG7947970 = -380011752;    double IKKkVtKUUgFhIafmG98949601 = -359642248;    double IKKkVtKUUgFhIafmG86728216 = -453932553;    double IKKkVtKUUgFhIafmG53934338 = -500744123;    double IKKkVtKUUgFhIafmG33105268 = -926323741;    double IKKkVtKUUgFhIafmG65587277 = -483432491;    double IKKkVtKUUgFhIafmG90723113 = -127080988;    double IKKkVtKUUgFhIafmG13908939 = -66706082;    double IKKkVtKUUgFhIafmG8523026 = -902867585;    double IKKkVtKUUgFhIafmG7686851 = -137953886;    double IKKkVtKUUgFhIafmG34456627 = -400676035;    double IKKkVtKUUgFhIafmG10752023 = -361480861;    double IKKkVtKUUgFhIafmG77936194 = -913419665;    double IKKkVtKUUgFhIafmG65438222 = -645342979;    double IKKkVtKUUgFhIafmG24869491 = -877484673;    double IKKkVtKUUgFhIafmG21770075 = -925810733;    double IKKkVtKUUgFhIafmG49565764 = 5745074;    double IKKkVtKUUgFhIafmG2701195 = -33625347;     IKKkVtKUUgFhIafmG62119398 = IKKkVtKUUgFhIafmG19698994;     IKKkVtKUUgFhIafmG19698994 = IKKkVtKUUgFhIafmG39247019;     IKKkVtKUUgFhIafmG39247019 = IKKkVtKUUgFhIafmG78602482;     IKKkVtKUUgFhIafmG78602482 = IKKkVtKUUgFhIafmG58671305;     IKKkVtKUUgFhIafmG58671305 = IKKkVtKUUgFhIafmG90518305;     IKKkVtKUUgFhIafmG90518305 = IKKkVtKUUgFhIafmG83252037;     IKKkVtKUUgFhIafmG83252037 = IKKkVtKUUgFhIafmG62195035;     IKKkVtKUUgFhIafmG62195035 = IKKkVtKUUgFhIafmG65189135;     IKKkVtKUUgFhIafmG65189135 = IKKkVtKUUgFhIafmG49332676;     IKKkVtKUUgFhIafmG49332676 = IKKkVtKUUgFhIafmG13552555;     IKKkVtKUUgFhIafmG13552555 = IKKkVtKUUgFhIafmG45216808;     IKKkVtKUUgFhIafmG45216808 = IKKkVtKUUgFhIafmG29078361;     IKKkVtKUUgFhIafmG29078361 = IKKkVtKUUgFhIafmG22615101;     IKKkVtKUUgFhIafmG22615101 = IKKkVtKUUgFhIafmG60192050;     IKKkVtKUUgFhIafmG60192050 = IKKkVtKUUgFhIafmG11784837;     IKKkVtKUUgFhIafmG11784837 = IKKkVtKUUgFhIafmG43227412;     IKKkVtKUUgFhIafmG43227412 = IKKkVtKUUgFhIafmG77003244;     IKKkVtKUUgFhIafmG77003244 = IKKkVtKUUgFhIafmG14038514;     IKKkVtKUUgFhIafmG14038514 = IKKkVtKUUgFhIafmG53905130;     IKKkVtKUUgFhIafmG53905130 = IKKkVtKUUgFhIafmG18944971;     IKKkVtKUUgFhIafmG18944971 = IKKkVtKUUgFhIafmG58736281;     IKKkVtKUUgFhIafmG58736281 = IKKkVtKUUgFhIafmG49592314;     IKKkVtKUUgFhIafmG49592314 = IKKkVtKUUgFhIafmG20565126;     IKKkVtKUUgFhIafmG20565126 = IKKkVtKUUgFhIafmG13988902;     IKKkVtKUUgFhIafmG13988902 = IKKkVtKUUgFhIafmG14502009;     IKKkVtKUUgFhIafmG14502009 = IKKkVtKUUgFhIafmG94869569;     IKKkVtKUUgFhIafmG94869569 = IKKkVtKUUgFhIafmG52761959;     IKKkVtKUUgFhIafmG52761959 = IKKkVtKUUgFhIafmG43681519;     IKKkVtKUUgFhIafmG43681519 = IKKkVtKUUgFhIafmG25164350;     IKKkVtKUUgFhIafmG25164350 = IKKkVtKUUgFhIafmG92527609;     IKKkVtKUUgFhIafmG92527609 = IKKkVtKUUgFhIafmG15801301;     IKKkVtKUUgFhIafmG15801301 = IKKkVtKUUgFhIafmG49254810;     IKKkVtKUUgFhIafmG49254810 = IKKkVtKUUgFhIafmG41277185;     IKKkVtKUUgFhIafmG41277185 = IKKkVtKUUgFhIafmG17292055;     IKKkVtKUUgFhIafmG17292055 = IKKkVtKUUgFhIafmG78629706;     IKKkVtKUUgFhIafmG78629706 = IKKkVtKUUgFhIafmG26368744;     IKKkVtKUUgFhIafmG26368744 = IKKkVtKUUgFhIafmG99070206;     IKKkVtKUUgFhIafmG99070206 = IKKkVtKUUgFhIafmG54921320;     IKKkVtKUUgFhIafmG54921320 = IKKkVtKUUgFhIafmG73584930;     IKKkVtKUUgFhIafmG73584930 = IKKkVtKUUgFhIafmG95928300;     IKKkVtKUUgFhIafmG95928300 = IKKkVtKUUgFhIafmG43383007;     IKKkVtKUUgFhIafmG43383007 = IKKkVtKUUgFhIafmG61794860;     IKKkVtKUUgFhIafmG61794860 = IKKkVtKUUgFhIafmG6370832;     IKKkVtKUUgFhIafmG6370832 = IKKkVtKUUgFhIafmG452449;     IKKkVtKUUgFhIafmG452449 = IKKkVtKUUgFhIafmG83814238;     IKKkVtKUUgFhIafmG83814238 = IKKkVtKUUgFhIafmG26117401;     IKKkVtKUUgFhIafmG26117401 = IKKkVtKUUgFhIafmG72853980;     IKKkVtKUUgFhIafmG72853980 = IKKkVtKUUgFhIafmG44813989;     IKKkVtKUUgFhIafmG44813989 = IKKkVtKUUgFhIafmG44731918;     IKKkVtKUUgFhIafmG44731918 = IKKkVtKUUgFhIafmG38436133;     IKKkVtKUUgFhIafmG38436133 = IKKkVtKUUgFhIafmG38593271;     IKKkVtKUUgFhIafmG38593271 = IKKkVtKUUgFhIafmG82696033;     IKKkVtKUUgFhIafmG82696033 = IKKkVtKUUgFhIafmG83667533;     IKKkVtKUUgFhIafmG83667533 = IKKkVtKUUgFhIafmG50554072;     IKKkVtKUUgFhIafmG50554072 = IKKkVtKUUgFhIafmG3383117;     IKKkVtKUUgFhIafmG3383117 = IKKkVtKUUgFhIafmG70106680;     IKKkVtKUUgFhIafmG70106680 = IKKkVtKUUgFhIafmG18681894;     IKKkVtKUUgFhIafmG18681894 = IKKkVtKUUgFhIafmG64613580;     IKKkVtKUUgFhIafmG64613580 = IKKkVtKUUgFhIafmG44169297;     IKKkVtKUUgFhIafmG44169297 = IKKkVtKUUgFhIafmG95648735;     IKKkVtKUUgFhIafmG95648735 = IKKkVtKUUgFhIafmG30490079;     IKKkVtKUUgFhIafmG30490079 = IKKkVtKUUgFhIafmG18513517;     IKKkVtKUUgFhIafmG18513517 = IKKkVtKUUgFhIafmG40024785;     IKKkVtKUUgFhIafmG40024785 = IKKkVtKUUgFhIafmG56805067;     IKKkVtKUUgFhIafmG56805067 = IKKkVtKUUgFhIafmG97751253;     IKKkVtKUUgFhIafmG97751253 = IKKkVtKUUgFhIafmG95961997;     IKKkVtKUUgFhIafmG95961997 = IKKkVtKUUgFhIafmG87801176;     IKKkVtKUUgFhIafmG87801176 = IKKkVtKUUgFhIafmG5323046;     IKKkVtKUUgFhIafmG5323046 = IKKkVtKUUgFhIafmG81562344;     IKKkVtKUUgFhIafmG81562344 = IKKkVtKUUgFhIafmG85416093;     IKKkVtKUUgFhIafmG85416093 = IKKkVtKUUgFhIafmG44157205;     IKKkVtKUUgFhIafmG44157205 = IKKkVtKUUgFhIafmG22081925;     IKKkVtKUUgFhIafmG22081925 = IKKkVtKUUgFhIafmG40453584;     IKKkVtKUUgFhIafmG40453584 = IKKkVtKUUgFhIafmG57976830;     IKKkVtKUUgFhIafmG57976830 = IKKkVtKUUgFhIafmG75561963;     IKKkVtKUUgFhIafmG75561963 = IKKkVtKUUgFhIafmG96941421;     IKKkVtKUUgFhIafmG96941421 = IKKkVtKUUgFhIafmG43221482;     IKKkVtKUUgFhIafmG43221482 = IKKkVtKUUgFhIafmG20112677;     IKKkVtKUUgFhIafmG20112677 = IKKkVtKUUgFhIafmG30174664;     IKKkVtKUUgFhIafmG30174664 = IKKkVtKUUgFhIafmG88384607;     IKKkVtKUUgFhIafmG88384607 = IKKkVtKUUgFhIafmG22015590;     IKKkVtKUUgFhIafmG22015590 = IKKkVtKUUgFhIafmG7947970;     IKKkVtKUUgFhIafmG7947970 = IKKkVtKUUgFhIafmG98949601;     IKKkVtKUUgFhIafmG98949601 = IKKkVtKUUgFhIafmG86728216;     IKKkVtKUUgFhIafmG86728216 = IKKkVtKUUgFhIafmG53934338;     IKKkVtKUUgFhIafmG53934338 = IKKkVtKUUgFhIafmG33105268;     IKKkVtKUUgFhIafmG33105268 = IKKkVtKUUgFhIafmG65587277;     IKKkVtKUUgFhIafmG65587277 = IKKkVtKUUgFhIafmG90723113;     IKKkVtKUUgFhIafmG90723113 = IKKkVtKUUgFhIafmG13908939;     IKKkVtKUUgFhIafmG13908939 = IKKkVtKUUgFhIafmG8523026;     IKKkVtKUUgFhIafmG8523026 = IKKkVtKUUgFhIafmG7686851;     IKKkVtKUUgFhIafmG7686851 = IKKkVtKUUgFhIafmG34456627;     IKKkVtKUUgFhIafmG34456627 = IKKkVtKUUgFhIafmG10752023;     IKKkVtKUUgFhIafmG10752023 = IKKkVtKUUgFhIafmG77936194;     IKKkVtKUUgFhIafmG77936194 = IKKkVtKUUgFhIafmG65438222;     IKKkVtKUUgFhIafmG65438222 = IKKkVtKUUgFhIafmG24869491;     IKKkVtKUUgFhIafmG24869491 = IKKkVtKUUgFhIafmG21770075;     IKKkVtKUUgFhIafmG21770075 = IKKkVtKUUgFhIafmG49565764;     IKKkVtKUUgFhIafmG49565764 = IKKkVtKUUgFhIafmG2701195;     IKKkVtKUUgFhIafmG2701195 = IKKkVtKUUgFhIafmG62119398;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void svtwDxgIkNCARjyx6144017() {     double MGFOZlTyrQDYZYwNr84506515 = -768029310;    double MGFOZlTyrQDYZYwNr4046495 = -527064578;    double MGFOZlTyrQDYZYwNr68016297 = -496813567;    double MGFOZlTyrQDYZYwNr93609170 = -310639433;    double MGFOZlTyrQDYZYwNr56845560 = -938582931;    double MGFOZlTyrQDYZYwNr5401423 = -923540174;    double MGFOZlTyrQDYZYwNr75407776 = -640362498;    double MGFOZlTyrQDYZYwNr37828894 = 94070303;    double MGFOZlTyrQDYZYwNr62352266 = -601598305;    double MGFOZlTyrQDYZYwNr41096261 = 82171855;    double MGFOZlTyrQDYZYwNr51094414 = -925765755;    double MGFOZlTyrQDYZYwNr1119757 = -955008115;    double MGFOZlTyrQDYZYwNr96086541 = -534455939;    double MGFOZlTyrQDYZYwNr65229853 = -48880676;    double MGFOZlTyrQDYZYwNr96927327 = -13846130;    double MGFOZlTyrQDYZYwNr56040749 = -571702938;    double MGFOZlTyrQDYZYwNr93050763 = 70683376;    double MGFOZlTyrQDYZYwNr82979661 = -136539623;    double MGFOZlTyrQDYZYwNr55320144 = -548839870;    double MGFOZlTyrQDYZYwNr32421414 = -121056436;    double MGFOZlTyrQDYZYwNr19828078 = -280381458;    double MGFOZlTyrQDYZYwNr22551500 = -70644385;    double MGFOZlTyrQDYZYwNr52885637 = -108221904;    double MGFOZlTyrQDYZYwNr47064913 = -558437674;    double MGFOZlTyrQDYZYwNr49529149 = -918126628;    double MGFOZlTyrQDYZYwNr2805004 = -588872556;    double MGFOZlTyrQDYZYwNr41958990 = -650880133;    double MGFOZlTyrQDYZYwNr58673409 = -663825535;    double MGFOZlTyrQDYZYwNr50574080 = -656565837;    double MGFOZlTyrQDYZYwNr34321529 = 97628825;    double MGFOZlTyrQDYZYwNr36818414 = -523698948;    double MGFOZlTyrQDYZYwNr73342491 = -825186534;    double MGFOZlTyrQDYZYwNr30293481 = -499130598;    double MGFOZlTyrQDYZYwNr665873 = -32600547;    double MGFOZlTyrQDYZYwNr93521419 = -598209780;    double MGFOZlTyrQDYZYwNr20322576 = -764643173;    double MGFOZlTyrQDYZYwNr19808456 = -133616744;    double MGFOZlTyrQDYZYwNr52696307 = -375965906;    double MGFOZlTyrQDYZYwNr3229452 = -965074490;    double MGFOZlTyrQDYZYwNr93770644 = -119281850;    double MGFOZlTyrQDYZYwNr24685671 = 8475245;    double MGFOZlTyrQDYZYwNr31891398 = -858417699;    double MGFOZlTyrQDYZYwNr67345052 = -448668275;    double MGFOZlTyrQDYZYwNr5814951 = -824322590;    double MGFOZlTyrQDYZYwNr68435715 = -727003804;    double MGFOZlTyrQDYZYwNr91914520 = -284212988;    double MGFOZlTyrQDYZYwNr38414799 = -378116280;    double MGFOZlTyrQDYZYwNr17452655 = -756340178;    double MGFOZlTyrQDYZYwNr39273894 = -345076029;    double MGFOZlTyrQDYZYwNr41815663 = 5313271;    double MGFOZlTyrQDYZYwNr47305349 = -883219033;    double MGFOZlTyrQDYZYwNr12444529 = -272457956;    double MGFOZlTyrQDYZYwNr61010436 = 81297331;    double MGFOZlTyrQDYZYwNr43954079 = -762035421;    double MGFOZlTyrQDYZYwNr86315939 = -722096825;    double MGFOZlTyrQDYZYwNr61955016 = -597384925;    double MGFOZlTyrQDYZYwNr51160858 = -318842675;    double MGFOZlTyrQDYZYwNr20951385 = -938375893;    double MGFOZlTyrQDYZYwNr44080021 = -392512806;    double MGFOZlTyrQDYZYwNr54040556 = -249710376;    double MGFOZlTyrQDYZYwNr63442432 = -172660042;    double MGFOZlTyrQDYZYwNr16734367 = -976536963;    double MGFOZlTyrQDYZYwNr87254814 = -249363860;    double MGFOZlTyrQDYZYwNr28030737 = -599227130;    double MGFOZlTyrQDYZYwNr4277847 = -394129197;    double MGFOZlTyrQDYZYwNr77751922 = -579222;    double MGFOZlTyrQDYZYwNr70826276 = -355877517;    double MGFOZlTyrQDYZYwNr95420669 = -401855392;    double MGFOZlTyrQDYZYwNr71708433 = -450670897;    double MGFOZlTyrQDYZYwNr76604752 = -249202958;    double MGFOZlTyrQDYZYwNr36232293 = -338086195;    double MGFOZlTyrQDYZYwNr40354456 = -553350719;    double MGFOZlTyrQDYZYwNr79750209 = -171465134;    double MGFOZlTyrQDYZYwNr61549500 = -329558020;    double MGFOZlTyrQDYZYwNr7735743 = -29531681;    double MGFOZlTyrQDYZYwNr87936679 = -421963760;    double MGFOZlTyrQDYZYwNr55206448 = -621976111;    double MGFOZlTyrQDYZYwNr47070686 = -283899314;    double MGFOZlTyrQDYZYwNr78629197 = -831433871;    double MGFOZlTyrQDYZYwNr57614628 = -533913640;    double MGFOZlTyrQDYZYwNr64390204 = -110756276;    double MGFOZlTyrQDYZYwNr24506336 = -894539955;    double MGFOZlTyrQDYZYwNr19399515 = -218749507;    double MGFOZlTyrQDYZYwNr8758417 = -561879108;    double MGFOZlTyrQDYZYwNr87016179 = -19152143;    double MGFOZlTyrQDYZYwNr24373886 = -151240993;    double MGFOZlTyrQDYZYwNr12332055 = -806483866;    double MGFOZlTyrQDYZYwNr86339402 = -737095177;    double MGFOZlTyrQDYZYwNr14349934 = -310503722;    double MGFOZlTyrQDYZYwNr31566404 = 99175145;    double MGFOZlTyrQDYZYwNr69161718 = -345800499;    double MGFOZlTyrQDYZYwNr98857071 = -195240851;    double MGFOZlTyrQDYZYwNr8616286 = -983453100;    double MGFOZlTyrQDYZYwNr49188895 = -615364115;    double MGFOZlTyrQDYZYwNr30328212 = -946621809;    double MGFOZlTyrQDYZYwNr7951304 = -14987793;    double MGFOZlTyrQDYZYwNr44636584 = -509053839;    double MGFOZlTyrQDYZYwNr39314315 = -849441145;    double MGFOZlTyrQDYZYwNr1537105 = -330193394;    double MGFOZlTyrQDYZYwNr90683792 = -768029310;     MGFOZlTyrQDYZYwNr84506515 = MGFOZlTyrQDYZYwNr4046495;     MGFOZlTyrQDYZYwNr4046495 = MGFOZlTyrQDYZYwNr68016297;     MGFOZlTyrQDYZYwNr68016297 = MGFOZlTyrQDYZYwNr93609170;     MGFOZlTyrQDYZYwNr93609170 = MGFOZlTyrQDYZYwNr56845560;     MGFOZlTyrQDYZYwNr56845560 = MGFOZlTyrQDYZYwNr5401423;     MGFOZlTyrQDYZYwNr5401423 = MGFOZlTyrQDYZYwNr75407776;     MGFOZlTyrQDYZYwNr75407776 = MGFOZlTyrQDYZYwNr37828894;     MGFOZlTyrQDYZYwNr37828894 = MGFOZlTyrQDYZYwNr62352266;     MGFOZlTyrQDYZYwNr62352266 = MGFOZlTyrQDYZYwNr41096261;     MGFOZlTyrQDYZYwNr41096261 = MGFOZlTyrQDYZYwNr51094414;     MGFOZlTyrQDYZYwNr51094414 = MGFOZlTyrQDYZYwNr1119757;     MGFOZlTyrQDYZYwNr1119757 = MGFOZlTyrQDYZYwNr96086541;     MGFOZlTyrQDYZYwNr96086541 = MGFOZlTyrQDYZYwNr65229853;     MGFOZlTyrQDYZYwNr65229853 = MGFOZlTyrQDYZYwNr96927327;     MGFOZlTyrQDYZYwNr96927327 = MGFOZlTyrQDYZYwNr56040749;     MGFOZlTyrQDYZYwNr56040749 = MGFOZlTyrQDYZYwNr93050763;     MGFOZlTyrQDYZYwNr93050763 = MGFOZlTyrQDYZYwNr82979661;     MGFOZlTyrQDYZYwNr82979661 = MGFOZlTyrQDYZYwNr55320144;     MGFOZlTyrQDYZYwNr55320144 = MGFOZlTyrQDYZYwNr32421414;     MGFOZlTyrQDYZYwNr32421414 = MGFOZlTyrQDYZYwNr19828078;     MGFOZlTyrQDYZYwNr19828078 = MGFOZlTyrQDYZYwNr22551500;     MGFOZlTyrQDYZYwNr22551500 = MGFOZlTyrQDYZYwNr52885637;     MGFOZlTyrQDYZYwNr52885637 = MGFOZlTyrQDYZYwNr47064913;     MGFOZlTyrQDYZYwNr47064913 = MGFOZlTyrQDYZYwNr49529149;     MGFOZlTyrQDYZYwNr49529149 = MGFOZlTyrQDYZYwNr2805004;     MGFOZlTyrQDYZYwNr2805004 = MGFOZlTyrQDYZYwNr41958990;     MGFOZlTyrQDYZYwNr41958990 = MGFOZlTyrQDYZYwNr58673409;     MGFOZlTyrQDYZYwNr58673409 = MGFOZlTyrQDYZYwNr50574080;     MGFOZlTyrQDYZYwNr50574080 = MGFOZlTyrQDYZYwNr34321529;     MGFOZlTyrQDYZYwNr34321529 = MGFOZlTyrQDYZYwNr36818414;     MGFOZlTyrQDYZYwNr36818414 = MGFOZlTyrQDYZYwNr73342491;     MGFOZlTyrQDYZYwNr73342491 = MGFOZlTyrQDYZYwNr30293481;     MGFOZlTyrQDYZYwNr30293481 = MGFOZlTyrQDYZYwNr665873;     MGFOZlTyrQDYZYwNr665873 = MGFOZlTyrQDYZYwNr93521419;     MGFOZlTyrQDYZYwNr93521419 = MGFOZlTyrQDYZYwNr20322576;     MGFOZlTyrQDYZYwNr20322576 = MGFOZlTyrQDYZYwNr19808456;     MGFOZlTyrQDYZYwNr19808456 = MGFOZlTyrQDYZYwNr52696307;     MGFOZlTyrQDYZYwNr52696307 = MGFOZlTyrQDYZYwNr3229452;     MGFOZlTyrQDYZYwNr3229452 = MGFOZlTyrQDYZYwNr93770644;     MGFOZlTyrQDYZYwNr93770644 = MGFOZlTyrQDYZYwNr24685671;     MGFOZlTyrQDYZYwNr24685671 = MGFOZlTyrQDYZYwNr31891398;     MGFOZlTyrQDYZYwNr31891398 = MGFOZlTyrQDYZYwNr67345052;     MGFOZlTyrQDYZYwNr67345052 = MGFOZlTyrQDYZYwNr5814951;     MGFOZlTyrQDYZYwNr5814951 = MGFOZlTyrQDYZYwNr68435715;     MGFOZlTyrQDYZYwNr68435715 = MGFOZlTyrQDYZYwNr91914520;     MGFOZlTyrQDYZYwNr91914520 = MGFOZlTyrQDYZYwNr38414799;     MGFOZlTyrQDYZYwNr38414799 = MGFOZlTyrQDYZYwNr17452655;     MGFOZlTyrQDYZYwNr17452655 = MGFOZlTyrQDYZYwNr39273894;     MGFOZlTyrQDYZYwNr39273894 = MGFOZlTyrQDYZYwNr41815663;     MGFOZlTyrQDYZYwNr41815663 = MGFOZlTyrQDYZYwNr47305349;     MGFOZlTyrQDYZYwNr47305349 = MGFOZlTyrQDYZYwNr12444529;     MGFOZlTyrQDYZYwNr12444529 = MGFOZlTyrQDYZYwNr61010436;     MGFOZlTyrQDYZYwNr61010436 = MGFOZlTyrQDYZYwNr43954079;     MGFOZlTyrQDYZYwNr43954079 = MGFOZlTyrQDYZYwNr86315939;     MGFOZlTyrQDYZYwNr86315939 = MGFOZlTyrQDYZYwNr61955016;     MGFOZlTyrQDYZYwNr61955016 = MGFOZlTyrQDYZYwNr51160858;     MGFOZlTyrQDYZYwNr51160858 = MGFOZlTyrQDYZYwNr20951385;     MGFOZlTyrQDYZYwNr20951385 = MGFOZlTyrQDYZYwNr44080021;     MGFOZlTyrQDYZYwNr44080021 = MGFOZlTyrQDYZYwNr54040556;     MGFOZlTyrQDYZYwNr54040556 = MGFOZlTyrQDYZYwNr63442432;     MGFOZlTyrQDYZYwNr63442432 = MGFOZlTyrQDYZYwNr16734367;     MGFOZlTyrQDYZYwNr16734367 = MGFOZlTyrQDYZYwNr87254814;     MGFOZlTyrQDYZYwNr87254814 = MGFOZlTyrQDYZYwNr28030737;     MGFOZlTyrQDYZYwNr28030737 = MGFOZlTyrQDYZYwNr4277847;     MGFOZlTyrQDYZYwNr4277847 = MGFOZlTyrQDYZYwNr77751922;     MGFOZlTyrQDYZYwNr77751922 = MGFOZlTyrQDYZYwNr70826276;     MGFOZlTyrQDYZYwNr70826276 = MGFOZlTyrQDYZYwNr95420669;     MGFOZlTyrQDYZYwNr95420669 = MGFOZlTyrQDYZYwNr71708433;     MGFOZlTyrQDYZYwNr71708433 = MGFOZlTyrQDYZYwNr76604752;     MGFOZlTyrQDYZYwNr76604752 = MGFOZlTyrQDYZYwNr36232293;     MGFOZlTyrQDYZYwNr36232293 = MGFOZlTyrQDYZYwNr40354456;     MGFOZlTyrQDYZYwNr40354456 = MGFOZlTyrQDYZYwNr79750209;     MGFOZlTyrQDYZYwNr79750209 = MGFOZlTyrQDYZYwNr61549500;     MGFOZlTyrQDYZYwNr61549500 = MGFOZlTyrQDYZYwNr7735743;     MGFOZlTyrQDYZYwNr7735743 = MGFOZlTyrQDYZYwNr87936679;     MGFOZlTyrQDYZYwNr87936679 = MGFOZlTyrQDYZYwNr55206448;     MGFOZlTyrQDYZYwNr55206448 = MGFOZlTyrQDYZYwNr47070686;     MGFOZlTyrQDYZYwNr47070686 = MGFOZlTyrQDYZYwNr78629197;     MGFOZlTyrQDYZYwNr78629197 = MGFOZlTyrQDYZYwNr57614628;     MGFOZlTyrQDYZYwNr57614628 = MGFOZlTyrQDYZYwNr64390204;     MGFOZlTyrQDYZYwNr64390204 = MGFOZlTyrQDYZYwNr24506336;     MGFOZlTyrQDYZYwNr24506336 = MGFOZlTyrQDYZYwNr19399515;     MGFOZlTyrQDYZYwNr19399515 = MGFOZlTyrQDYZYwNr8758417;     MGFOZlTyrQDYZYwNr8758417 = MGFOZlTyrQDYZYwNr87016179;     MGFOZlTyrQDYZYwNr87016179 = MGFOZlTyrQDYZYwNr24373886;     MGFOZlTyrQDYZYwNr24373886 = MGFOZlTyrQDYZYwNr12332055;     MGFOZlTyrQDYZYwNr12332055 = MGFOZlTyrQDYZYwNr86339402;     MGFOZlTyrQDYZYwNr86339402 = MGFOZlTyrQDYZYwNr14349934;     MGFOZlTyrQDYZYwNr14349934 = MGFOZlTyrQDYZYwNr31566404;     MGFOZlTyrQDYZYwNr31566404 = MGFOZlTyrQDYZYwNr69161718;     MGFOZlTyrQDYZYwNr69161718 = MGFOZlTyrQDYZYwNr98857071;     MGFOZlTyrQDYZYwNr98857071 = MGFOZlTyrQDYZYwNr8616286;     MGFOZlTyrQDYZYwNr8616286 = MGFOZlTyrQDYZYwNr49188895;     MGFOZlTyrQDYZYwNr49188895 = MGFOZlTyrQDYZYwNr30328212;     MGFOZlTyrQDYZYwNr30328212 = MGFOZlTyrQDYZYwNr7951304;     MGFOZlTyrQDYZYwNr7951304 = MGFOZlTyrQDYZYwNr44636584;     MGFOZlTyrQDYZYwNr44636584 = MGFOZlTyrQDYZYwNr39314315;     MGFOZlTyrQDYZYwNr39314315 = MGFOZlTyrQDYZYwNr1537105;     MGFOZlTyrQDYZYwNr1537105 = MGFOZlTyrQDYZYwNr90683792;     MGFOZlTyrQDYZYwNr90683792 = MGFOZlTyrQDYZYwNr84506515;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void iwwBstlNzjQVbvUs21193085() {     double UozUQiDFjneuLLNJA90623622 = -879931199;    double UozUQiDFjneuLLNJA47419774 = -236664359;    double UozUQiDFjneuLLNJA84058481 = -647646488;    double UozUQiDFjneuLLNJA20683951 = -501388064;    double UozUQiDFjneuLLNJA25741048 = -359232802;    double UozUQiDFjneuLLNJA96818624 = -88036409;    double UozUQiDFjneuLLNJA35896742 = -779327475;    double UozUQiDFjneuLLNJA87588784 = -192803610;    double UozUQiDFjneuLLNJA26994761 = -369630971;    double UozUQiDFjneuLLNJA82325920 = -246803622;    double UozUQiDFjneuLLNJA11204705 = -803203342;    double UozUQiDFjneuLLNJA32942799 = -299414316;    double UozUQiDFjneuLLNJA82295660 = -974347028;    double UozUQiDFjneuLLNJA48373530 = -581119244;    double UozUQiDFjneuLLNJA92669754 = -704308296;    double UozUQiDFjneuLLNJA97546142 = -544702481;    double UozUQiDFjneuLLNJA86886766 = -545861737;    double UozUQiDFjneuLLNJA80032814 = -902062266;    double UozUQiDFjneuLLNJA17149986 = -954608967;    double UozUQiDFjneuLLNJA12971579 = -477959355;    double UozUQiDFjneuLLNJA8381271 = -135868913;    double UozUQiDFjneuLLNJA9072524 = 21425898;    double UozUQiDFjneuLLNJA88476428 = -954739391;    double UozUQiDFjneuLLNJA31057287 = -780742662;    double UozUQiDFjneuLLNJA93228799 = -191322660;    double UozUQiDFjneuLLNJA27526679 = -467527879;    double UozUQiDFjneuLLNJA40418769 = -551542336;    double UozUQiDFjneuLLNJA71780534 = -729754489;    double UozUQiDFjneuLLNJA16706335 = -878948541;    double UozUQiDFjneuLLNJA2937166 = -819842042;    double UozUQiDFjneuLLNJA29060964 = -437019887;    double UozUQiDFjneuLLNJA70953113 = -534633350;    double UozUQiDFjneuLLNJA40004429 = -391743968;    double UozUQiDFjneuLLNJA95496436 = -884142313;    double UozUQiDFjneuLLNJA54996072 = -136942885;    double UozUQiDFjneuLLNJA89048488 = -978669280;    double UozUQiDFjneuLLNJA1070391 = -87252613;    double UozUQiDFjneuLLNJA86786520 = -147471915;    double UozUQiDFjneuLLNJA76839705 = -958997987;    double UozUQiDFjneuLLNJA81272582 = -592721073;    double UozUQiDFjneuLLNJA30354895 = -56798649;    double UozUQiDFjneuLLNJA64041855 = -28559977;    double UozUQiDFjneuLLNJA45412030 = -529472858;    double UozUQiDFjneuLLNJA93222691 = -843665528;    double UozUQiDFjneuLLNJA80388180 = -388901958;    double UozUQiDFjneuLLNJA3853716 = -269429105;    double UozUQiDFjneuLLNJA56193373 = -496891818;    double UozUQiDFjneuLLNJA25409596 = -766235711;    double UozUQiDFjneuLLNJA48949623 = -430813017;    double UozUQiDFjneuLLNJA54727316 = -81697537;    double UozUQiDFjneuLLNJA83006027 = -725512704;    double UozUQiDFjneuLLNJA23254248 = -73184276;    double UozUQiDFjneuLLNJA9258894 = -810492256;    double UozUQiDFjneuLLNJA39922959 = 17494434;    double UozUQiDFjneuLLNJA12811814 = -322628608;    double UozUQiDFjneuLLNJA81551098 = -801357097;    double UozUQiDFjneuLLNJA58943345 = -281924968;    double UozUQiDFjneuLLNJA53001195 = -866903826;    double UozUQiDFjneuLLNJA27455152 = -210065404;    double UozUQiDFjneuLLNJA98214369 = -891704924;    double UozUQiDFjneuLLNJA56399856 = -536494074;    double UozUQiDFjneuLLNJA64116208 = 50427013;    double UozUQiDFjneuLLNJA70882449 = -313855069;    double UozUQiDFjneuLLNJA24057596 = -549788929;    double UozUQiDFjneuLLNJA53264956 = -809783736;    double UozUQiDFjneuLLNJA40251591 = -168569992;    double UozUQiDFjneuLLNJA92938370 = -907670349;    double UozUQiDFjneuLLNJA86799223 = 9795284;    double UozUQiDFjneuLLNJA93377458 = -344176360;    double UozUQiDFjneuLLNJA3621266 = -725639016;    double UozUQiDFjneuLLNJA96475752 = -357449868;    double UozUQiDFjneuLLNJA100247 = -298389822;    double UozUQiDFjneuLLNJA3193109 = -943064279;    double UozUQiDFjneuLLNJA35877404 = -261887895;    double UozUQiDFjneuLLNJA82616683 = -321160706;    double UozUQiDFjneuLLNJA44339416 = -7308937;    double UozUQiDFjneuLLNJA63660493 = -449101244;    double UozUQiDFjneuLLNJA95253737 = -11073863;    double UozUQiDFjneuLLNJA50669106 = -291840704;    double UozUQiDFjneuLLNJA89375083 = -921893556;    double UozUQiDFjneuLLNJA71333305 = -970636061;    double UozUQiDFjneuLLNJA15009173 = -785306626;    double UozUQiDFjneuLLNJA22830911 = -198941473;    double UozUQiDFjneuLLNJA61979019 = -697251005;    double UozUQiDFjneuLLNJA19931138 = 5670661;    double UozUQiDFjneuLLNJA5806716 = -263835611;    double UozUQiDFjneuLLNJA61694219 = -724141094;    double UozUQiDFjneuLLNJA81471 = -309238402;    double UozUQiDFjneuLLNJA82684622 = -461513705;    double UozUQiDFjneuLLNJA73444973 = -335585788;    double UozUQiDFjneuLLNJA30105144 = -596744312;    double UozUQiDFjneuLLNJA48069196 = -220348787;    double UozUQiDFjneuLLNJA59331368 = -937406512;    double UozUQiDFjneuLLNJA78625335 = 32706936;    double UozUQiDFjneuLLNJA24872726 = 43773000;    double UozUQiDFjneuLLNJA66238687 = -7225663;    double UozUQiDFjneuLLNJA93159405 = -714704908;    double UozUQiDFjneuLLNJA21354435 = -979683930;    double UozUQiDFjneuLLNJA39957735 = 66118207;    double UozUQiDFjneuLLNJA40136589 = -879931199;     UozUQiDFjneuLLNJA90623622 = UozUQiDFjneuLLNJA47419774;     UozUQiDFjneuLLNJA47419774 = UozUQiDFjneuLLNJA84058481;     UozUQiDFjneuLLNJA84058481 = UozUQiDFjneuLLNJA20683951;     UozUQiDFjneuLLNJA20683951 = UozUQiDFjneuLLNJA25741048;     UozUQiDFjneuLLNJA25741048 = UozUQiDFjneuLLNJA96818624;     UozUQiDFjneuLLNJA96818624 = UozUQiDFjneuLLNJA35896742;     UozUQiDFjneuLLNJA35896742 = UozUQiDFjneuLLNJA87588784;     UozUQiDFjneuLLNJA87588784 = UozUQiDFjneuLLNJA26994761;     UozUQiDFjneuLLNJA26994761 = UozUQiDFjneuLLNJA82325920;     UozUQiDFjneuLLNJA82325920 = UozUQiDFjneuLLNJA11204705;     UozUQiDFjneuLLNJA11204705 = UozUQiDFjneuLLNJA32942799;     UozUQiDFjneuLLNJA32942799 = UozUQiDFjneuLLNJA82295660;     UozUQiDFjneuLLNJA82295660 = UozUQiDFjneuLLNJA48373530;     UozUQiDFjneuLLNJA48373530 = UozUQiDFjneuLLNJA92669754;     UozUQiDFjneuLLNJA92669754 = UozUQiDFjneuLLNJA97546142;     UozUQiDFjneuLLNJA97546142 = UozUQiDFjneuLLNJA86886766;     UozUQiDFjneuLLNJA86886766 = UozUQiDFjneuLLNJA80032814;     UozUQiDFjneuLLNJA80032814 = UozUQiDFjneuLLNJA17149986;     UozUQiDFjneuLLNJA17149986 = UozUQiDFjneuLLNJA12971579;     UozUQiDFjneuLLNJA12971579 = UozUQiDFjneuLLNJA8381271;     UozUQiDFjneuLLNJA8381271 = UozUQiDFjneuLLNJA9072524;     UozUQiDFjneuLLNJA9072524 = UozUQiDFjneuLLNJA88476428;     UozUQiDFjneuLLNJA88476428 = UozUQiDFjneuLLNJA31057287;     UozUQiDFjneuLLNJA31057287 = UozUQiDFjneuLLNJA93228799;     UozUQiDFjneuLLNJA93228799 = UozUQiDFjneuLLNJA27526679;     UozUQiDFjneuLLNJA27526679 = UozUQiDFjneuLLNJA40418769;     UozUQiDFjneuLLNJA40418769 = UozUQiDFjneuLLNJA71780534;     UozUQiDFjneuLLNJA71780534 = UozUQiDFjneuLLNJA16706335;     UozUQiDFjneuLLNJA16706335 = UozUQiDFjneuLLNJA2937166;     UozUQiDFjneuLLNJA2937166 = UozUQiDFjneuLLNJA29060964;     UozUQiDFjneuLLNJA29060964 = UozUQiDFjneuLLNJA70953113;     UozUQiDFjneuLLNJA70953113 = UozUQiDFjneuLLNJA40004429;     UozUQiDFjneuLLNJA40004429 = UozUQiDFjneuLLNJA95496436;     UozUQiDFjneuLLNJA95496436 = UozUQiDFjneuLLNJA54996072;     UozUQiDFjneuLLNJA54996072 = UozUQiDFjneuLLNJA89048488;     UozUQiDFjneuLLNJA89048488 = UozUQiDFjneuLLNJA1070391;     UozUQiDFjneuLLNJA1070391 = UozUQiDFjneuLLNJA86786520;     UozUQiDFjneuLLNJA86786520 = UozUQiDFjneuLLNJA76839705;     UozUQiDFjneuLLNJA76839705 = UozUQiDFjneuLLNJA81272582;     UozUQiDFjneuLLNJA81272582 = UozUQiDFjneuLLNJA30354895;     UozUQiDFjneuLLNJA30354895 = UozUQiDFjneuLLNJA64041855;     UozUQiDFjneuLLNJA64041855 = UozUQiDFjneuLLNJA45412030;     UozUQiDFjneuLLNJA45412030 = UozUQiDFjneuLLNJA93222691;     UozUQiDFjneuLLNJA93222691 = UozUQiDFjneuLLNJA80388180;     UozUQiDFjneuLLNJA80388180 = UozUQiDFjneuLLNJA3853716;     UozUQiDFjneuLLNJA3853716 = UozUQiDFjneuLLNJA56193373;     UozUQiDFjneuLLNJA56193373 = UozUQiDFjneuLLNJA25409596;     UozUQiDFjneuLLNJA25409596 = UozUQiDFjneuLLNJA48949623;     UozUQiDFjneuLLNJA48949623 = UozUQiDFjneuLLNJA54727316;     UozUQiDFjneuLLNJA54727316 = UozUQiDFjneuLLNJA83006027;     UozUQiDFjneuLLNJA83006027 = UozUQiDFjneuLLNJA23254248;     UozUQiDFjneuLLNJA23254248 = UozUQiDFjneuLLNJA9258894;     UozUQiDFjneuLLNJA9258894 = UozUQiDFjneuLLNJA39922959;     UozUQiDFjneuLLNJA39922959 = UozUQiDFjneuLLNJA12811814;     UozUQiDFjneuLLNJA12811814 = UozUQiDFjneuLLNJA81551098;     UozUQiDFjneuLLNJA81551098 = UozUQiDFjneuLLNJA58943345;     UozUQiDFjneuLLNJA58943345 = UozUQiDFjneuLLNJA53001195;     UozUQiDFjneuLLNJA53001195 = UozUQiDFjneuLLNJA27455152;     UozUQiDFjneuLLNJA27455152 = UozUQiDFjneuLLNJA98214369;     UozUQiDFjneuLLNJA98214369 = UozUQiDFjneuLLNJA56399856;     UozUQiDFjneuLLNJA56399856 = UozUQiDFjneuLLNJA64116208;     UozUQiDFjneuLLNJA64116208 = UozUQiDFjneuLLNJA70882449;     UozUQiDFjneuLLNJA70882449 = UozUQiDFjneuLLNJA24057596;     UozUQiDFjneuLLNJA24057596 = UozUQiDFjneuLLNJA53264956;     UozUQiDFjneuLLNJA53264956 = UozUQiDFjneuLLNJA40251591;     UozUQiDFjneuLLNJA40251591 = UozUQiDFjneuLLNJA92938370;     UozUQiDFjneuLLNJA92938370 = UozUQiDFjneuLLNJA86799223;     UozUQiDFjneuLLNJA86799223 = UozUQiDFjneuLLNJA93377458;     UozUQiDFjneuLLNJA93377458 = UozUQiDFjneuLLNJA3621266;     UozUQiDFjneuLLNJA3621266 = UozUQiDFjneuLLNJA96475752;     UozUQiDFjneuLLNJA96475752 = UozUQiDFjneuLLNJA100247;     UozUQiDFjneuLLNJA100247 = UozUQiDFjneuLLNJA3193109;     UozUQiDFjneuLLNJA3193109 = UozUQiDFjneuLLNJA35877404;     UozUQiDFjneuLLNJA35877404 = UozUQiDFjneuLLNJA82616683;     UozUQiDFjneuLLNJA82616683 = UozUQiDFjneuLLNJA44339416;     UozUQiDFjneuLLNJA44339416 = UozUQiDFjneuLLNJA63660493;     UozUQiDFjneuLLNJA63660493 = UozUQiDFjneuLLNJA95253737;     UozUQiDFjneuLLNJA95253737 = UozUQiDFjneuLLNJA50669106;     UozUQiDFjneuLLNJA50669106 = UozUQiDFjneuLLNJA89375083;     UozUQiDFjneuLLNJA89375083 = UozUQiDFjneuLLNJA71333305;     UozUQiDFjneuLLNJA71333305 = UozUQiDFjneuLLNJA15009173;     UozUQiDFjneuLLNJA15009173 = UozUQiDFjneuLLNJA22830911;     UozUQiDFjneuLLNJA22830911 = UozUQiDFjneuLLNJA61979019;     UozUQiDFjneuLLNJA61979019 = UozUQiDFjneuLLNJA19931138;     UozUQiDFjneuLLNJA19931138 = UozUQiDFjneuLLNJA5806716;     UozUQiDFjneuLLNJA5806716 = UozUQiDFjneuLLNJA61694219;     UozUQiDFjneuLLNJA61694219 = UozUQiDFjneuLLNJA81471;     UozUQiDFjneuLLNJA81471 = UozUQiDFjneuLLNJA82684622;     UozUQiDFjneuLLNJA82684622 = UozUQiDFjneuLLNJA73444973;     UozUQiDFjneuLLNJA73444973 = UozUQiDFjneuLLNJA30105144;     UozUQiDFjneuLLNJA30105144 = UozUQiDFjneuLLNJA48069196;     UozUQiDFjneuLLNJA48069196 = UozUQiDFjneuLLNJA59331368;     UozUQiDFjneuLLNJA59331368 = UozUQiDFjneuLLNJA78625335;     UozUQiDFjneuLLNJA78625335 = UozUQiDFjneuLLNJA24872726;     UozUQiDFjneuLLNJA24872726 = UozUQiDFjneuLLNJA66238687;     UozUQiDFjneuLLNJA66238687 = UozUQiDFjneuLLNJA93159405;     UozUQiDFjneuLLNJA93159405 = UozUQiDFjneuLLNJA21354435;     UozUQiDFjneuLLNJA21354435 = UozUQiDFjneuLLNJA39957735;     UozUQiDFjneuLLNJA39957735 = UozUQiDFjneuLLNJA40136589;     UozUQiDFjneuLLNJA40136589 = UozUQiDFjneuLLNJA90623622;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void mhYztZQYqsulaSqm88484683() {     double xxlooFHRspqaTDjbE26082591 = -246208099;    double xxlooFHRspqaTDjbE60691852 = -117215965;    double xxlooFHRspqaTDjbE21444958 = -313313563;    double xxlooFHRspqaTDjbE92907306 = -322539135;    double xxlooFHRspqaTDjbE82026317 = -464831648;    double xxlooFHRspqaTDjbE33041497 = -350384305;    double xxlooFHRspqaTDjbE46554148 = -624053210;    double xxlooFHRspqaTDjbE49720346 = -937037750;    double xxlooFHRspqaTDjbE28512367 = -857962378;    double xxlooFHRspqaTDjbE97228632 = 88509380;    double xxlooFHRspqaTDjbE49386439 = -605211071;    double xxlooFHRspqaTDjbE33510948 = -163535088;    double xxlooFHRspqaTDjbE20530306 = -426359533;    double xxlooFHRspqaTDjbE1514394 = -557482654;    double xxlooFHRspqaTDjbE50864271 = -107733311;    double xxlooFHRspqaTDjbE81087443 = -180514088;    double xxlooFHRspqaTDjbE42722702 = 27163710;    double xxlooFHRspqaTDjbE79793005 = -975490335;    double xxlooFHRspqaTDjbE17700316 = -171172388;    double xxlooFHRspqaTDjbE6808797 = -904520421;    double xxlooFHRspqaTDjbE21468134 = -893143602;    double xxlooFHRspqaTDjbE12494049 = -65210396;    double xxlooFHRspqaTDjbE59001808 = -592371575;    double xxlooFHRspqaTDjbE24850232 = -191567266;    double xxlooFHRspqaTDjbE1246751 = -215307859;    double xxlooFHRspqaTDjbE38224850 = -156104686;    double xxlooFHRspqaTDjbE839343 = -357826477;    double xxlooFHRspqaTDjbE12508960 = -899526663;    double xxlooFHRspqaTDjbE63374550 = -353043344;    double xxlooFHRspqaTDjbE8470577 = -854349213;    double xxlooFHRspqaTDjbE47644195 = -989000374;    double xxlooFHRspqaTDjbE8776132 = -307604764;    double xxlooFHRspqaTDjbE52222440 = -539339945;    double xxlooFHRspqaTDjbE25244863 = -948836126;    double xxlooFHRspqaTDjbE20804526 = -88043136;    double xxlooFHRspqaTDjbE40609333 = -93234586;    double xxlooFHRspqaTDjbE50482207 = 48051347;    double xxlooFHRspqaTDjbE38001923 = -130127496;    double xxlooFHRspqaTDjbE7230267 = -609874263;    double xxlooFHRspqaTDjbE45544114 = -57983619;    double xxlooFHRspqaTDjbE63806503 = -925997564;    double xxlooFHRspqaTDjbE53406982 = -723105268;    double xxlooFHRspqaTDjbE34795408 = -82653644;    double xxlooFHRspqaTDjbE33354029 = -347997938;    double xxlooFHRspqaTDjbE80404639 = -215413119;    double xxlooFHRspqaTDjbE49815044 = -308352737;    double xxlooFHRspqaTDjbE75538536 = -980878179;    double xxlooFHRspqaTDjbE43135908 = -932762304;    double xxlooFHRspqaTDjbE71842289 = -94549897;    double xxlooFHRspqaTDjbE79256905 = -572724353;    double xxlooFHRspqaTDjbE63776751 = -599789262;    double xxlooFHRspqaTDjbE6739720 = -286836623;    double xxlooFHRspqaTDjbE77880042 = -566537810;    double xxlooFHRspqaTDjbE55914805 = -174014066;    double xxlooFHRspqaTDjbE67016550 = -669118754;    double xxlooFHRspqaTDjbE13588543 = -80997703;    double xxlooFHRspqaTDjbE1690044 = -524844391;    double xxlooFHRspqaTDjbE96594726 = -21746297;    double xxlooFHRspqaTDjbE91660555 = -7231277;    double xxlooFHRspqaTDjbE43801467 = -208726962;    double xxlooFHRspqaTDjbE32202154 = -992557828;    double xxlooFHRspqaTDjbE34045189 = -724526547;    double xxlooFHRspqaTDjbE86345796 = -483994407;    double xxlooFHRspqaTDjbE20041790 = 96386835;    double xxlooFHRspqaTDjbE49584437 = 77509753;    double xxlooFHRspqaTDjbE40610308 = -197606307;    double xxlooFHRspqaTDjbE81288507 = -624195143;    double xxlooFHRspqaTDjbE95285442 = -477523407;    double xxlooFHRspqaTDjbE80709867 = -369439518;    double xxlooFHRspqaTDjbE10254938 = 85501274;    double xxlooFHRspqaTDjbE30605237 = -128565435;    double xxlooFHRspqaTDjbE4720779 = -842708794;    double xxlooFHRspqaTDjbE72562739 = -265616072;    double xxlooFHRspqaTDjbE72156201 = -13188770;    double xxlooFHRspqaTDjbE43002294 = -978522858;    double xxlooFHRspqaTDjbE68061152 = -70038335;    double xxlooFHRspqaTDjbE77698640 = -982556752;    double xxlooFHRspqaTDjbE25647779 = -144373637;    double xxlooFHRspqaTDjbE44445593 = -976154147;    double xxlooFHRspqaTDjbE51431706 = -906955122;    double xxlooFHRspqaTDjbE62686313 = -175226508;    double xxlooFHRspqaTDjbE57703435 = -425064173;    double xxlooFHRspqaTDjbE40666670 = -704976767;    double xxlooFHRspqaTDjbE84117644 = -780318991;    double xxlooFHRspqaTDjbE44693825 = -154559952;    double xxlooFHRspqaTDjbE40904475 = -602163751;    double xxlooFHRspqaTDjbE30896090 = -741066954;    double xxlooFHRspqaTDjbE96307634 = -265325880;    double xxlooFHRspqaTDjbE58228313 = -179717373;    double xxlooFHRspqaTDjbE7215983 = 92954567;    double xxlooFHRspqaTDjbE38919289 = -568390196;    double xxlooFHRspqaTDjbE53887481 = -930202357;    double xxlooFHRspqaTDjbE46341368 = -22896220;    double xxlooFHRspqaTDjbE63428799 = -301147302;    double xxlooFHRspqaTDjbE13341960 = -65425791;    double xxlooFHRspqaTDjbE29761314 = -101471018;    double xxlooFHRspqaTDjbE67061186 = -139110861;    double xxlooFHRspqaTDjbE14753619 = -79040480;    double xxlooFHRspqaTDjbE83769592 = -325507692;    double xxlooFHRspqaTDjbE39794331 = -246208099;     xxlooFHRspqaTDjbE26082591 = xxlooFHRspqaTDjbE60691852;     xxlooFHRspqaTDjbE60691852 = xxlooFHRspqaTDjbE21444958;     xxlooFHRspqaTDjbE21444958 = xxlooFHRspqaTDjbE92907306;     xxlooFHRspqaTDjbE92907306 = xxlooFHRspqaTDjbE82026317;     xxlooFHRspqaTDjbE82026317 = xxlooFHRspqaTDjbE33041497;     xxlooFHRspqaTDjbE33041497 = xxlooFHRspqaTDjbE46554148;     xxlooFHRspqaTDjbE46554148 = xxlooFHRspqaTDjbE49720346;     xxlooFHRspqaTDjbE49720346 = xxlooFHRspqaTDjbE28512367;     xxlooFHRspqaTDjbE28512367 = xxlooFHRspqaTDjbE97228632;     xxlooFHRspqaTDjbE97228632 = xxlooFHRspqaTDjbE49386439;     xxlooFHRspqaTDjbE49386439 = xxlooFHRspqaTDjbE33510948;     xxlooFHRspqaTDjbE33510948 = xxlooFHRspqaTDjbE20530306;     xxlooFHRspqaTDjbE20530306 = xxlooFHRspqaTDjbE1514394;     xxlooFHRspqaTDjbE1514394 = xxlooFHRspqaTDjbE50864271;     xxlooFHRspqaTDjbE50864271 = xxlooFHRspqaTDjbE81087443;     xxlooFHRspqaTDjbE81087443 = xxlooFHRspqaTDjbE42722702;     xxlooFHRspqaTDjbE42722702 = xxlooFHRspqaTDjbE79793005;     xxlooFHRspqaTDjbE79793005 = xxlooFHRspqaTDjbE17700316;     xxlooFHRspqaTDjbE17700316 = xxlooFHRspqaTDjbE6808797;     xxlooFHRspqaTDjbE6808797 = xxlooFHRspqaTDjbE21468134;     xxlooFHRspqaTDjbE21468134 = xxlooFHRspqaTDjbE12494049;     xxlooFHRspqaTDjbE12494049 = xxlooFHRspqaTDjbE59001808;     xxlooFHRspqaTDjbE59001808 = xxlooFHRspqaTDjbE24850232;     xxlooFHRspqaTDjbE24850232 = xxlooFHRspqaTDjbE1246751;     xxlooFHRspqaTDjbE1246751 = xxlooFHRspqaTDjbE38224850;     xxlooFHRspqaTDjbE38224850 = xxlooFHRspqaTDjbE839343;     xxlooFHRspqaTDjbE839343 = xxlooFHRspqaTDjbE12508960;     xxlooFHRspqaTDjbE12508960 = xxlooFHRspqaTDjbE63374550;     xxlooFHRspqaTDjbE63374550 = xxlooFHRspqaTDjbE8470577;     xxlooFHRspqaTDjbE8470577 = xxlooFHRspqaTDjbE47644195;     xxlooFHRspqaTDjbE47644195 = xxlooFHRspqaTDjbE8776132;     xxlooFHRspqaTDjbE8776132 = xxlooFHRspqaTDjbE52222440;     xxlooFHRspqaTDjbE52222440 = xxlooFHRspqaTDjbE25244863;     xxlooFHRspqaTDjbE25244863 = xxlooFHRspqaTDjbE20804526;     xxlooFHRspqaTDjbE20804526 = xxlooFHRspqaTDjbE40609333;     xxlooFHRspqaTDjbE40609333 = xxlooFHRspqaTDjbE50482207;     xxlooFHRspqaTDjbE50482207 = xxlooFHRspqaTDjbE38001923;     xxlooFHRspqaTDjbE38001923 = xxlooFHRspqaTDjbE7230267;     xxlooFHRspqaTDjbE7230267 = xxlooFHRspqaTDjbE45544114;     xxlooFHRspqaTDjbE45544114 = xxlooFHRspqaTDjbE63806503;     xxlooFHRspqaTDjbE63806503 = xxlooFHRspqaTDjbE53406982;     xxlooFHRspqaTDjbE53406982 = xxlooFHRspqaTDjbE34795408;     xxlooFHRspqaTDjbE34795408 = xxlooFHRspqaTDjbE33354029;     xxlooFHRspqaTDjbE33354029 = xxlooFHRspqaTDjbE80404639;     xxlooFHRspqaTDjbE80404639 = xxlooFHRspqaTDjbE49815044;     xxlooFHRspqaTDjbE49815044 = xxlooFHRspqaTDjbE75538536;     xxlooFHRspqaTDjbE75538536 = xxlooFHRspqaTDjbE43135908;     xxlooFHRspqaTDjbE43135908 = xxlooFHRspqaTDjbE71842289;     xxlooFHRspqaTDjbE71842289 = xxlooFHRspqaTDjbE79256905;     xxlooFHRspqaTDjbE79256905 = xxlooFHRspqaTDjbE63776751;     xxlooFHRspqaTDjbE63776751 = xxlooFHRspqaTDjbE6739720;     xxlooFHRspqaTDjbE6739720 = xxlooFHRspqaTDjbE77880042;     xxlooFHRspqaTDjbE77880042 = xxlooFHRspqaTDjbE55914805;     xxlooFHRspqaTDjbE55914805 = xxlooFHRspqaTDjbE67016550;     xxlooFHRspqaTDjbE67016550 = xxlooFHRspqaTDjbE13588543;     xxlooFHRspqaTDjbE13588543 = xxlooFHRspqaTDjbE1690044;     xxlooFHRspqaTDjbE1690044 = xxlooFHRspqaTDjbE96594726;     xxlooFHRspqaTDjbE96594726 = xxlooFHRspqaTDjbE91660555;     xxlooFHRspqaTDjbE91660555 = xxlooFHRspqaTDjbE43801467;     xxlooFHRspqaTDjbE43801467 = xxlooFHRspqaTDjbE32202154;     xxlooFHRspqaTDjbE32202154 = xxlooFHRspqaTDjbE34045189;     xxlooFHRspqaTDjbE34045189 = xxlooFHRspqaTDjbE86345796;     xxlooFHRspqaTDjbE86345796 = xxlooFHRspqaTDjbE20041790;     xxlooFHRspqaTDjbE20041790 = xxlooFHRspqaTDjbE49584437;     xxlooFHRspqaTDjbE49584437 = xxlooFHRspqaTDjbE40610308;     xxlooFHRspqaTDjbE40610308 = xxlooFHRspqaTDjbE81288507;     xxlooFHRspqaTDjbE81288507 = xxlooFHRspqaTDjbE95285442;     xxlooFHRspqaTDjbE95285442 = xxlooFHRspqaTDjbE80709867;     xxlooFHRspqaTDjbE80709867 = xxlooFHRspqaTDjbE10254938;     xxlooFHRspqaTDjbE10254938 = xxlooFHRspqaTDjbE30605237;     xxlooFHRspqaTDjbE30605237 = xxlooFHRspqaTDjbE4720779;     xxlooFHRspqaTDjbE4720779 = xxlooFHRspqaTDjbE72562739;     xxlooFHRspqaTDjbE72562739 = xxlooFHRspqaTDjbE72156201;     xxlooFHRspqaTDjbE72156201 = xxlooFHRspqaTDjbE43002294;     xxlooFHRspqaTDjbE43002294 = xxlooFHRspqaTDjbE68061152;     xxlooFHRspqaTDjbE68061152 = xxlooFHRspqaTDjbE77698640;     xxlooFHRspqaTDjbE77698640 = xxlooFHRspqaTDjbE25647779;     xxlooFHRspqaTDjbE25647779 = xxlooFHRspqaTDjbE44445593;     xxlooFHRspqaTDjbE44445593 = xxlooFHRspqaTDjbE51431706;     xxlooFHRspqaTDjbE51431706 = xxlooFHRspqaTDjbE62686313;     xxlooFHRspqaTDjbE62686313 = xxlooFHRspqaTDjbE57703435;     xxlooFHRspqaTDjbE57703435 = xxlooFHRspqaTDjbE40666670;     xxlooFHRspqaTDjbE40666670 = xxlooFHRspqaTDjbE84117644;     xxlooFHRspqaTDjbE84117644 = xxlooFHRspqaTDjbE44693825;     xxlooFHRspqaTDjbE44693825 = xxlooFHRspqaTDjbE40904475;     xxlooFHRspqaTDjbE40904475 = xxlooFHRspqaTDjbE30896090;     xxlooFHRspqaTDjbE30896090 = xxlooFHRspqaTDjbE96307634;     xxlooFHRspqaTDjbE96307634 = xxlooFHRspqaTDjbE58228313;     xxlooFHRspqaTDjbE58228313 = xxlooFHRspqaTDjbE7215983;     xxlooFHRspqaTDjbE7215983 = xxlooFHRspqaTDjbE38919289;     xxlooFHRspqaTDjbE38919289 = xxlooFHRspqaTDjbE53887481;     xxlooFHRspqaTDjbE53887481 = xxlooFHRspqaTDjbE46341368;     xxlooFHRspqaTDjbE46341368 = xxlooFHRspqaTDjbE63428799;     xxlooFHRspqaTDjbE63428799 = xxlooFHRspqaTDjbE13341960;     xxlooFHRspqaTDjbE13341960 = xxlooFHRspqaTDjbE29761314;     xxlooFHRspqaTDjbE29761314 = xxlooFHRspqaTDjbE67061186;     xxlooFHRspqaTDjbE67061186 = xxlooFHRspqaTDjbE14753619;     xxlooFHRspqaTDjbE14753619 = xxlooFHRspqaTDjbE83769592;     xxlooFHRspqaTDjbE83769592 = xxlooFHRspqaTDjbE39794331;     xxlooFHRspqaTDjbE39794331 = xxlooFHRspqaTDjbE26082591;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZBVNLGJnHWBuMZUw3533752() {     double BdofJelJUVcxDoidj32199698 = -358109987;    double BdofJelJUVcxDoidj4065132 = -926815746;    double BdofJelJUVcxDoidj37487141 = -464146484;    double BdofJelJUVcxDoidj19982087 = -513287765;    double BdofJelJUVcxDoidj50921806 = -985481519;    double BdofJelJUVcxDoidj24458699 = -614880540;    double BdofJelJUVcxDoidj7043115 = -763018187;    double BdofJelJUVcxDoidj99480236 = -123911663;    double BdofJelJUVcxDoidj93154861 = -625995043;    double BdofJelJUVcxDoidj38458292 = -240466098;    double BdofJelJUVcxDoidj9496730 = -482648658;    double BdofJelJUVcxDoidj65333990 = -607941290;    double BdofJelJUVcxDoidj6739424 = -866250622;    double BdofJelJUVcxDoidj84658069 = 10278778;    double BdofJelJUVcxDoidj46606697 = -798195478;    double BdofJelJUVcxDoidj22592838 = -153513631;    double BdofJelJUVcxDoidj36558705 = -589381402;    double BdofJelJUVcxDoidj76846158 = -641012977;    double BdofJelJUVcxDoidj79530157 = -576941485;    double BdofJelJUVcxDoidj87358961 = -161423340;    double BdofJelJUVcxDoidj10021327 = -748631057;    double BdofJelJUVcxDoidj99015071 = 26859888;    double BdofJelJUVcxDoidj94592600 = -338889062;    double BdofJelJUVcxDoidj8842606 = -413872254;    double BdofJelJUVcxDoidj44946400 = -588503892;    double BdofJelJUVcxDoidj62946525 = -34760009;    double BdofJelJUVcxDoidj99299120 = -258488680;    double BdofJelJUVcxDoidj25616085 = -965455617;    double BdofJelJUVcxDoidj29506805 = -575426048;    double BdofJelJUVcxDoidj77086213 = -671820080;    double BdofJelJUVcxDoidj39886745 = -902321312;    double BdofJelJUVcxDoidj6386754 = -17051580;    double BdofJelJUVcxDoidj61933388 = -431953316;    double BdofJelJUVcxDoidj20075427 = -700377891;    double BdofJelJUVcxDoidj82279177 = -726776241;    double BdofJelJUVcxDoidj9335246 = -307260694;    double BdofJelJUVcxDoidj31744142 = 94415477;    double BdofJelJUVcxDoidj72092136 = 98366494;    double BdofJelJUVcxDoidj80840520 = -603797760;    double BdofJelJUVcxDoidj33046052 = -531422842;    double BdofJelJUVcxDoidj69475726 = -991271458;    double BdofJelJUVcxDoidj85557438 = -993247546;    double BdofJelJUVcxDoidj12862387 = -163458228;    double BdofJelJUVcxDoidj20761770 = -367340877;    double BdofJelJUVcxDoidj92357103 = -977311274;    double BdofJelJUVcxDoidj61754239 = -293568853;    double BdofJelJUVcxDoidj93317110 = 346284;    double BdofJelJUVcxDoidj51092848 = -942657837;    double BdofJelJUVcxDoidj81518018 = -180286885;    double BdofJelJUVcxDoidj92168557 = -659735161;    double BdofJelJUVcxDoidj99477429 = -442082933;    double BdofJelJUVcxDoidj17549440 = -87562943;    double BdofJelJUVcxDoidj26128500 = -358327398;    double BdofJelJUVcxDoidj51883686 = -494484211;    double BdofJelJUVcxDoidj93512424 = -269650536;    double BdofJelJUVcxDoidj33184626 = -284969876;    double BdofJelJUVcxDoidj9472532 = -487926685;    double BdofJelJUVcxDoidj28644536 = 49725769;    double BdofJelJUVcxDoidj75035686 = -924783874;    double BdofJelJUVcxDoidj87975280 = -850721510;    double BdofJelJUVcxDoidj25159579 = -256391860;    double BdofJelJUVcxDoidj81427029 = -797562570;    double BdofJelJUVcxDoidj69973431 = -548485616;    double BdofJelJUVcxDoidj16068649 = -954174963;    double BdofJelJUVcxDoidj98571546 = -338144787;    double BdofJelJUVcxDoidj3109976 = -365597078;    double BdofJelJUVcxDoidj3400602 = -75987975;    double BdofJelJUVcxDoidj86663996 = -65872731;    double BdofJelJUVcxDoidj2378893 = -262944981;    double BdofJelJUVcxDoidj37271452 = -390934784;    double BdofJelJUVcxDoidj90848695 = -147929108;    double BdofJelJUVcxDoidj64466569 = -587747897;    double BdofJelJUVcxDoidj96005638 = 62784782;    double BdofJelJUVcxDoidj46484106 = 54481356;    double BdofJelJUVcxDoidj17883235 = -170151883;    double BdofJelJUVcxDoidj24463888 = -755383512;    double BdofJelJUVcxDoidj86152685 = -809681885;    double BdofJelJUVcxDoidj73830830 = -971548186;    double BdofJelJUVcxDoidj16485502 = -436560980;    double BdofJelJUVcxDoidj83192161 = -194935039;    double BdofJelJUVcxDoidj69629414 = 64893707;    double BdofJelJUVcxDoidj48206272 = -315830843;    double BdofJelJUVcxDoidj44098066 = -685168732;    double BdofJelJUVcxDoidj37338248 = -915690888;    double BdofJelJUVcxDoidj77608783 = -129737148;    double BdofJelJUVcxDoidj22337305 = -714758369;    double BdofJelJUVcxDoidj80258254 = -658724182;    double BdofJelJUVcxDoidj10049703 = -937469105;    double BdofJelJUVcxDoidj26563003 = -330727356;    double BdofJelJUVcxDoidj49094552 = -341806366;    double BdofJelJUVcxDoidj99862714 = -819334010;    double BdofJelJUVcxDoidj3099606 = -955310293;    double BdofJelJUVcxDoidj97056449 = 23150368;    double BdofJelJUVcxDoidj92865239 = -753076250;    double BdofJelJUVcxDoidj7886474 = -175030982;    double BdofJelJUVcxDoidj88048697 = -93708888;    double BdofJelJUVcxDoidj15584008 = -344761930;    double BdofJelJUVcxDoidj96793737 = -209283265;    double BdofJelJUVcxDoidj22190223 = 70803910;    double BdofJelJUVcxDoidj89247128 = -358109987;     BdofJelJUVcxDoidj32199698 = BdofJelJUVcxDoidj4065132;     BdofJelJUVcxDoidj4065132 = BdofJelJUVcxDoidj37487141;     BdofJelJUVcxDoidj37487141 = BdofJelJUVcxDoidj19982087;     BdofJelJUVcxDoidj19982087 = BdofJelJUVcxDoidj50921806;     BdofJelJUVcxDoidj50921806 = BdofJelJUVcxDoidj24458699;     BdofJelJUVcxDoidj24458699 = BdofJelJUVcxDoidj7043115;     BdofJelJUVcxDoidj7043115 = BdofJelJUVcxDoidj99480236;     BdofJelJUVcxDoidj99480236 = BdofJelJUVcxDoidj93154861;     BdofJelJUVcxDoidj93154861 = BdofJelJUVcxDoidj38458292;     BdofJelJUVcxDoidj38458292 = BdofJelJUVcxDoidj9496730;     BdofJelJUVcxDoidj9496730 = BdofJelJUVcxDoidj65333990;     BdofJelJUVcxDoidj65333990 = BdofJelJUVcxDoidj6739424;     BdofJelJUVcxDoidj6739424 = BdofJelJUVcxDoidj84658069;     BdofJelJUVcxDoidj84658069 = BdofJelJUVcxDoidj46606697;     BdofJelJUVcxDoidj46606697 = BdofJelJUVcxDoidj22592838;     BdofJelJUVcxDoidj22592838 = BdofJelJUVcxDoidj36558705;     BdofJelJUVcxDoidj36558705 = BdofJelJUVcxDoidj76846158;     BdofJelJUVcxDoidj76846158 = BdofJelJUVcxDoidj79530157;     BdofJelJUVcxDoidj79530157 = BdofJelJUVcxDoidj87358961;     BdofJelJUVcxDoidj87358961 = BdofJelJUVcxDoidj10021327;     BdofJelJUVcxDoidj10021327 = BdofJelJUVcxDoidj99015071;     BdofJelJUVcxDoidj99015071 = BdofJelJUVcxDoidj94592600;     BdofJelJUVcxDoidj94592600 = BdofJelJUVcxDoidj8842606;     BdofJelJUVcxDoidj8842606 = BdofJelJUVcxDoidj44946400;     BdofJelJUVcxDoidj44946400 = BdofJelJUVcxDoidj62946525;     BdofJelJUVcxDoidj62946525 = BdofJelJUVcxDoidj99299120;     BdofJelJUVcxDoidj99299120 = BdofJelJUVcxDoidj25616085;     BdofJelJUVcxDoidj25616085 = BdofJelJUVcxDoidj29506805;     BdofJelJUVcxDoidj29506805 = BdofJelJUVcxDoidj77086213;     BdofJelJUVcxDoidj77086213 = BdofJelJUVcxDoidj39886745;     BdofJelJUVcxDoidj39886745 = BdofJelJUVcxDoidj6386754;     BdofJelJUVcxDoidj6386754 = BdofJelJUVcxDoidj61933388;     BdofJelJUVcxDoidj61933388 = BdofJelJUVcxDoidj20075427;     BdofJelJUVcxDoidj20075427 = BdofJelJUVcxDoidj82279177;     BdofJelJUVcxDoidj82279177 = BdofJelJUVcxDoidj9335246;     BdofJelJUVcxDoidj9335246 = BdofJelJUVcxDoidj31744142;     BdofJelJUVcxDoidj31744142 = BdofJelJUVcxDoidj72092136;     BdofJelJUVcxDoidj72092136 = BdofJelJUVcxDoidj80840520;     BdofJelJUVcxDoidj80840520 = BdofJelJUVcxDoidj33046052;     BdofJelJUVcxDoidj33046052 = BdofJelJUVcxDoidj69475726;     BdofJelJUVcxDoidj69475726 = BdofJelJUVcxDoidj85557438;     BdofJelJUVcxDoidj85557438 = BdofJelJUVcxDoidj12862387;     BdofJelJUVcxDoidj12862387 = BdofJelJUVcxDoidj20761770;     BdofJelJUVcxDoidj20761770 = BdofJelJUVcxDoidj92357103;     BdofJelJUVcxDoidj92357103 = BdofJelJUVcxDoidj61754239;     BdofJelJUVcxDoidj61754239 = BdofJelJUVcxDoidj93317110;     BdofJelJUVcxDoidj93317110 = BdofJelJUVcxDoidj51092848;     BdofJelJUVcxDoidj51092848 = BdofJelJUVcxDoidj81518018;     BdofJelJUVcxDoidj81518018 = BdofJelJUVcxDoidj92168557;     BdofJelJUVcxDoidj92168557 = BdofJelJUVcxDoidj99477429;     BdofJelJUVcxDoidj99477429 = BdofJelJUVcxDoidj17549440;     BdofJelJUVcxDoidj17549440 = BdofJelJUVcxDoidj26128500;     BdofJelJUVcxDoidj26128500 = BdofJelJUVcxDoidj51883686;     BdofJelJUVcxDoidj51883686 = BdofJelJUVcxDoidj93512424;     BdofJelJUVcxDoidj93512424 = BdofJelJUVcxDoidj33184626;     BdofJelJUVcxDoidj33184626 = BdofJelJUVcxDoidj9472532;     BdofJelJUVcxDoidj9472532 = BdofJelJUVcxDoidj28644536;     BdofJelJUVcxDoidj28644536 = BdofJelJUVcxDoidj75035686;     BdofJelJUVcxDoidj75035686 = BdofJelJUVcxDoidj87975280;     BdofJelJUVcxDoidj87975280 = BdofJelJUVcxDoidj25159579;     BdofJelJUVcxDoidj25159579 = BdofJelJUVcxDoidj81427029;     BdofJelJUVcxDoidj81427029 = BdofJelJUVcxDoidj69973431;     BdofJelJUVcxDoidj69973431 = BdofJelJUVcxDoidj16068649;     BdofJelJUVcxDoidj16068649 = BdofJelJUVcxDoidj98571546;     BdofJelJUVcxDoidj98571546 = BdofJelJUVcxDoidj3109976;     BdofJelJUVcxDoidj3109976 = BdofJelJUVcxDoidj3400602;     BdofJelJUVcxDoidj3400602 = BdofJelJUVcxDoidj86663996;     BdofJelJUVcxDoidj86663996 = BdofJelJUVcxDoidj2378893;     BdofJelJUVcxDoidj2378893 = BdofJelJUVcxDoidj37271452;     BdofJelJUVcxDoidj37271452 = BdofJelJUVcxDoidj90848695;     BdofJelJUVcxDoidj90848695 = BdofJelJUVcxDoidj64466569;     BdofJelJUVcxDoidj64466569 = BdofJelJUVcxDoidj96005638;     BdofJelJUVcxDoidj96005638 = BdofJelJUVcxDoidj46484106;     BdofJelJUVcxDoidj46484106 = BdofJelJUVcxDoidj17883235;     BdofJelJUVcxDoidj17883235 = BdofJelJUVcxDoidj24463888;     BdofJelJUVcxDoidj24463888 = BdofJelJUVcxDoidj86152685;     BdofJelJUVcxDoidj86152685 = BdofJelJUVcxDoidj73830830;     BdofJelJUVcxDoidj73830830 = BdofJelJUVcxDoidj16485502;     BdofJelJUVcxDoidj16485502 = BdofJelJUVcxDoidj83192161;     BdofJelJUVcxDoidj83192161 = BdofJelJUVcxDoidj69629414;     BdofJelJUVcxDoidj69629414 = BdofJelJUVcxDoidj48206272;     BdofJelJUVcxDoidj48206272 = BdofJelJUVcxDoidj44098066;     BdofJelJUVcxDoidj44098066 = BdofJelJUVcxDoidj37338248;     BdofJelJUVcxDoidj37338248 = BdofJelJUVcxDoidj77608783;     BdofJelJUVcxDoidj77608783 = BdofJelJUVcxDoidj22337305;     BdofJelJUVcxDoidj22337305 = BdofJelJUVcxDoidj80258254;     BdofJelJUVcxDoidj80258254 = BdofJelJUVcxDoidj10049703;     BdofJelJUVcxDoidj10049703 = BdofJelJUVcxDoidj26563003;     BdofJelJUVcxDoidj26563003 = BdofJelJUVcxDoidj49094552;     BdofJelJUVcxDoidj49094552 = BdofJelJUVcxDoidj99862714;     BdofJelJUVcxDoidj99862714 = BdofJelJUVcxDoidj3099606;     BdofJelJUVcxDoidj3099606 = BdofJelJUVcxDoidj97056449;     BdofJelJUVcxDoidj97056449 = BdofJelJUVcxDoidj92865239;     BdofJelJUVcxDoidj92865239 = BdofJelJUVcxDoidj7886474;     BdofJelJUVcxDoidj7886474 = BdofJelJUVcxDoidj88048697;     BdofJelJUVcxDoidj88048697 = BdofJelJUVcxDoidj15584008;     BdofJelJUVcxDoidj15584008 = BdofJelJUVcxDoidj96793737;     BdofJelJUVcxDoidj96793737 = BdofJelJUVcxDoidj22190223;     BdofJelJUVcxDoidj22190223 = BdofJelJUVcxDoidj89247128;     BdofJelJUVcxDoidj89247128 = BdofJelJUVcxDoidj32199698;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void feZaUoahqKNPDItA70825351() {     double lQdjqeiOLOOELzzEz67658666 = -824386887;    double lQdjqeiOLOOELzzEz17337210 = -807367353;    double lQdjqeiOLOOELzzEz74873618 = -129813559;    double lQdjqeiOLOOELzzEz92205441 = -334438837;    double lQdjqeiOLOOELzzEz7207076 = 8919636;    double lQdjqeiOLOOELzzEz60681571 = -877228435;    double lQdjqeiOLOOELzzEz17700521 = -607743921;    double lQdjqeiOLOOELzzEz61611797 = -868145804;    double lQdjqeiOLOOELzzEz94672467 = -14326450;    double lQdjqeiOLOOELzzEz53361004 = 94846904;    double lQdjqeiOLOOELzzEz47678464 = -284656387;    double lQdjqeiOLOOELzzEz65902138 = -472062061;    double lQdjqeiOLOOELzzEz44974069 = -318263127;    double lQdjqeiOLOOELzzEz37798933 = 33915369;    double lQdjqeiOLOOELzzEz4801214 = -201620493;    double lQdjqeiOLOOELzzEz6134139 = -889325238;    double lQdjqeiOLOOELzzEz92394641 = -16355955;    double lQdjqeiOLOOELzzEz76606350 = -714441046;    double lQdjqeiOLOOELzzEz80080487 = -893504907;    double lQdjqeiOLOOELzzEz81196180 = -587984406;    double lQdjqeiOLOOELzzEz23108190 = -405905746;    double lQdjqeiOLOOELzzEz2436597 = -59776406;    double lQdjqeiOLOOELzzEz65117980 = 23478754;    double lQdjqeiOLOOELzzEz2635551 = -924696858;    double lQdjqeiOLOOELzzEz52964351 = -612489090;    double lQdjqeiOLOOELzzEz73644697 = -823336816;    double lQdjqeiOLOOELzzEz59719695 = -64772820;    double lQdjqeiOLOOELzzEz66344509 = -35227792;    double lQdjqeiOLOOELzzEz76175020 = -49520851;    double lQdjqeiOLOOELzzEz82619624 = -706327251;    double lQdjqeiOLOOELzzEz58469976 = -354301799;    double lQdjqeiOLOOELzzEz44209772 = -890022994;    double lQdjqeiOLOOELzzEz74151399 = -579549293;    double lQdjqeiOLOOELzzEz49823854 = -765071705;    double lQdjqeiOLOOELzzEz48087631 = -677876492;    double lQdjqeiOLOOELzzEz60896089 = -521826000;    double lQdjqeiOLOOELzzEz81155958 = -870280563;    double lQdjqeiOLOOELzzEz23307540 = -984289087;    double lQdjqeiOLOOELzzEz11231082 = -254674037;    double lQdjqeiOLOOELzzEz97317584 = 3314612;    double lQdjqeiOLOOELzzEz2927335 = -760470372;    double lQdjqeiOLOOELzzEz74922565 = -587792837;    double lQdjqeiOLOOELzzEz2245765 = -816639014;    double lQdjqeiOLOOELzzEz60893108 = -971673287;    double lQdjqeiOLOOELzzEz92373562 = -803822435;    double lQdjqeiOLOOELzzEz7715568 = -332492486;    double lQdjqeiOLOOELzzEz12662275 = -483640077;    double lQdjqeiOLOOELzzEz68819160 = -9184430;    double lQdjqeiOLOOELzzEz4410685 = -944023765;    double lQdjqeiOLOOELzzEz16698147 = -50761977;    double lQdjqeiOLOOELzzEz80248153 = -316359490;    double lQdjqeiOLOOELzzEz1034912 = -301215290;    double lQdjqeiOLOOELzzEz94749648 = -114372952;    double lQdjqeiOLOOELzzEz67875532 = -685992711;    double lQdjqeiOLOOELzzEz47717161 = -616140682;    double lQdjqeiOLOOELzzEz65222069 = -664610481;    double lQdjqeiOLOOELzzEz52219230 = -730846107;    double lQdjqeiOLOOELzzEz72238067 = -205116701;    double lQdjqeiOLOOELzzEz39241090 = -721949747;    double lQdjqeiOLOOELzzEz33562378 = -167743549;    double lQdjqeiOLOOELzzEz961877 = -712455615;    double lQdjqeiOLOOELzzEz51356011 = -472516130;    double lQdjqeiOLOOELzzEz85436777 = -718624954;    double lQdjqeiOLOOELzzEz12052843 = -307999200;    double lQdjqeiOLOOELzzEz94891027 = -550851298;    double lQdjqeiOLOOELzzEz3468693 = -394633393;    double lQdjqeiOLOOELzzEz91750739 = -892512769;    double lQdjqeiOLOOELzzEz95150215 = -553191422;    double lQdjqeiOLOOELzzEz89711302 = -288208139;    double lQdjqeiOLOOELzzEz43905124 = -679794494;    double lQdjqeiOLOOELzzEz24978180 = 80955325;    double lQdjqeiOLOOELzzEz69087102 = -32066868;    double lQdjqeiOLOOELzzEz65375268 = -359767010;    double lQdjqeiOLOOELzzEz82762903 = -796819520;    double lQdjqeiOLOOELzzEz78268846 = -827514035;    double lQdjqeiOLOOELzzEz48185624 = -818112910;    double lQdjqeiOLOOELzzEz190832 = -243137393;    double lQdjqeiOLOOELzzEz4224872 = -4847960;    double lQdjqeiOLOOELzzEz10261988 = -20874423;    double lQdjqeiOLOOELzzEz45248784 = -179996605;    double lQdjqeiOLOOELzzEz60982422 = -239696739;    double lQdjqeiOLOOELzzEz90900534 = 44411609;    double lQdjqeiOLOOELzzEz61933825 = -91204027;    double lQdjqeiOLOOELzzEz59476873 = -998758874;    double lQdjqeiOLOOELzzEz2371471 = -289967761;    double lQdjqeiOLOOELzzEz57435064 = 46913490;    double lQdjqeiOLOOELzzEz49460124 = -675650043;    double lQdjqeiOLOOELzzEz6275867 = -893556583;    double lQdjqeiOLOOELzzEz2106693 = -48931023;    double lQdjqeiOLOOELzzEz82865561 = 86733989;    double lQdjqeiOLOOELzzEz8676860 = -790979893;    double lQdjqeiOLOOELzzEz8917891 = -565163862;    double lQdjqeiOLOOELzzEz84066449 = -162339341;    double lQdjqeiOLOOELzzEz77668704 = 13069512;    double lQdjqeiOLOOELzzEz96355707 = -284229773;    double lQdjqeiOLOOELzzEz51571323 = -187954243;    double lQdjqeiOLOOELzzEz89485787 = -869167883;    double lQdjqeiOLOOELzzEz90192922 = -408639815;    double lQdjqeiOLOOELzzEz66002080 = -320821989;    double lQdjqeiOLOOELzzEz88904870 = -824386887;     lQdjqeiOLOOELzzEz67658666 = lQdjqeiOLOOELzzEz17337210;     lQdjqeiOLOOELzzEz17337210 = lQdjqeiOLOOELzzEz74873618;     lQdjqeiOLOOELzzEz74873618 = lQdjqeiOLOOELzzEz92205441;     lQdjqeiOLOOELzzEz92205441 = lQdjqeiOLOOELzzEz7207076;     lQdjqeiOLOOELzzEz7207076 = lQdjqeiOLOOELzzEz60681571;     lQdjqeiOLOOELzzEz60681571 = lQdjqeiOLOOELzzEz17700521;     lQdjqeiOLOOELzzEz17700521 = lQdjqeiOLOOELzzEz61611797;     lQdjqeiOLOOELzzEz61611797 = lQdjqeiOLOOELzzEz94672467;     lQdjqeiOLOOELzzEz94672467 = lQdjqeiOLOOELzzEz53361004;     lQdjqeiOLOOELzzEz53361004 = lQdjqeiOLOOELzzEz47678464;     lQdjqeiOLOOELzzEz47678464 = lQdjqeiOLOOELzzEz65902138;     lQdjqeiOLOOELzzEz65902138 = lQdjqeiOLOOELzzEz44974069;     lQdjqeiOLOOELzzEz44974069 = lQdjqeiOLOOELzzEz37798933;     lQdjqeiOLOOELzzEz37798933 = lQdjqeiOLOOELzzEz4801214;     lQdjqeiOLOOELzzEz4801214 = lQdjqeiOLOOELzzEz6134139;     lQdjqeiOLOOELzzEz6134139 = lQdjqeiOLOOELzzEz92394641;     lQdjqeiOLOOELzzEz92394641 = lQdjqeiOLOOELzzEz76606350;     lQdjqeiOLOOELzzEz76606350 = lQdjqeiOLOOELzzEz80080487;     lQdjqeiOLOOELzzEz80080487 = lQdjqeiOLOOELzzEz81196180;     lQdjqeiOLOOELzzEz81196180 = lQdjqeiOLOOELzzEz23108190;     lQdjqeiOLOOELzzEz23108190 = lQdjqeiOLOOELzzEz2436597;     lQdjqeiOLOOELzzEz2436597 = lQdjqeiOLOOELzzEz65117980;     lQdjqeiOLOOELzzEz65117980 = lQdjqeiOLOOELzzEz2635551;     lQdjqeiOLOOELzzEz2635551 = lQdjqeiOLOOELzzEz52964351;     lQdjqeiOLOOELzzEz52964351 = lQdjqeiOLOOELzzEz73644697;     lQdjqeiOLOOELzzEz73644697 = lQdjqeiOLOOELzzEz59719695;     lQdjqeiOLOOELzzEz59719695 = lQdjqeiOLOOELzzEz66344509;     lQdjqeiOLOOELzzEz66344509 = lQdjqeiOLOOELzzEz76175020;     lQdjqeiOLOOELzzEz76175020 = lQdjqeiOLOOELzzEz82619624;     lQdjqeiOLOOELzzEz82619624 = lQdjqeiOLOOELzzEz58469976;     lQdjqeiOLOOELzzEz58469976 = lQdjqeiOLOOELzzEz44209772;     lQdjqeiOLOOELzzEz44209772 = lQdjqeiOLOOELzzEz74151399;     lQdjqeiOLOOELzzEz74151399 = lQdjqeiOLOOELzzEz49823854;     lQdjqeiOLOOELzzEz49823854 = lQdjqeiOLOOELzzEz48087631;     lQdjqeiOLOOELzzEz48087631 = lQdjqeiOLOOELzzEz60896089;     lQdjqeiOLOOELzzEz60896089 = lQdjqeiOLOOELzzEz81155958;     lQdjqeiOLOOELzzEz81155958 = lQdjqeiOLOOELzzEz23307540;     lQdjqeiOLOOELzzEz23307540 = lQdjqeiOLOOELzzEz11231082;     lQdjqeiOLOOELzzEz11231082 = lQdjqeiOLOOELzzEz97317584;     lQdjqeiOLOOELzzEz97317584 = lQdjqeiOLOOELzzEz2927335;     lQdjqeiOLOOELzzEz2927335 = lQdjqeiOLOOELzzEz74922565;     lQdjqeiOLOOELzzEz74922565 = lQdjqeiOLOOELzzEz2245765;     lQdjqeiOLOOELzzEz2245765 = lQdjqeiOLOOELzzEz60893108;     lQdjqeiOLOOELzzEz60893108 = lQdjqeiOLOOELzzEz92373562;     lQdjqeiOLOOELzzEz92373562 = lQdjqeiOLOOELzzEz7715568;     lQdjqeiOLOOELzzEz7715568 = lQdjqeiOLOOELzzEz12662275;     lQdjqeiOLOOELzzEz12662275 = lQdjqeiOLOOELzzEz68819160;     lQdjqeiOLOOELzzEz68819160 = lQdjqeiOLOOELzzEz4410685;     lQdjqeiOLOOELzzEz4410685 = lQdjqeiOLOOELzzEz16698147;     lQdjqeiOLOOELzzEz16698147 = lQdjqeiOLOOELzzEz80248153;     lQdjqeiOLOOELzzEz80248153 = lQdjqeiOLOOELzzEz1034912;     lQdjqeiOLOOELzzEz1034912 = lQdjqeiOLOOELzzEz94749648;     lQdjqeiOLOOELzzEz94749648 = lQdjqeiOLOOELzzEz67875532;     lQdjqeiOLOOELzzEz67875532 = lQdjqeiOLOOELzzEz47717161;     lQdjqeiOLOOELzzEz47717161 = lQdjqeiOLOOELzzEz65222069;     lQdjqeiOLOOELzzEz65222069 = lQdjqeiOLOOELzzEz52219230;     lQdjqeiOLOOELzzEz52219230 = lQdjqeiOLOOELzzEz72238067;     lQdjqeiOLOOELzzEz72238067 = lQdjqeiOLOOELzzEz39241090;     lQdjqeiOLOOELzzEz39241090 = lQdjqeiOLOOELzzEz33562378;     lQdjqeiOLOOELzzEz33562378 = lQdjqeiOLOOELzzEz961877;     lQdjqeiOLOOELzzEz961877 = lQdjqeiOLOOELzzEz51356011;     lQdjqeiOLOOELzzEz51356011 = lQdjqeiOLOOELzzEz85436777;     lQdjqeiOLOOELzzEz85436777 = lQdjqeiOLOOELzzEz12052843;     lQdjqeiOLOOELzzEz12052843 = lQdjqeiOLOOELzzEz94891027;     lQdjqeiOLOOELzzEz94891027 = lQdjqeiOLOOELzzEz3468693;     lQdjqeiOLOOELzzEz3468693 = lQdjqeiOLOOELzzEz91750739;     lQdjqeiOLOOELzzEz91750739 = lQdjqeiOLOOELzzEz95150215;     lQdjqeiOLOOELzzEz95150215 = lQdjqeiOLOOELzzEz89711302;     lQdjqeiOLOOELzzEz89711302 = lQdjqeiOLOOELzzEz43905124;     lQdjqeiOLOOELzzEz43905124 = lQdjqeiOLOOELzzEz24978180;     lQdjqeiOLOOELzzEz24978180 = lQdjqeiOLOOELzzEz69087102;     lQdjqeiOLOOELzzEz69087102 = lQdjqeiOLOOELzzEz65375268;     lQdjqeiOLOOELzzEz65375268 = lQdjqeiOLOOELzzEz82762903;     lQdjqeiOLOOELzzEz82762903 = lQdjqeiOLOOELzzEz78268846;     lQdjqeiOLOOELzzEz78268846 = lQdjqeiOLOOELzzEz48185624;     lQdjqeiOLOOELzzEz48185624 = lQdjqeiOLOOELzzEz190832;     lQdjqeiOLOOELzzEz190832 = lQdjqeiOLOOELzzEz4224872;     lQdjqeiOLOOELzzEz4224872 = lQdjqeiOLOOELzzEz10261988;     lQdjqeiOLOOELzzEz10261988 = lQdjqeiOLOOELzzEz45248784;     lQdjqeiOLOOELzzEz45248784 = lQdjqeiOLOOELzzEz60982422;     lQdjqeiOLOOELzzEz60982422 = lQdjqeiOLOOELzzEz90900534;     lQdjqeiOLOOELzzEz90900534 = lQdjqeiOLOOELzzEz61933825;     lQdjqeiOLOOELzzEz61933825 = lQdjqeiOLOOELzzEz59476873;     lQdjqeiOLOOELzzEz59476873 = lQdjqeiOLOOELzzEz2371471;     lQdjqeiOLOOELzzEz2371471 = lQdjqeiOLOOELzzEz57435064;     lQdjqeiOLOOELzzEz57435064 = lQdjqeiOLOOELzzEz49460124;     lQdjqeiOLOOELzzEz49460124 = lQdjqeiOLOOELzzEz6275867;     lQdjqeiOLOOELzzEz6275867 = lQdjqeiOLOOELzzEz2106693;     lQdjqeiOLOOELzzEz2106693 = lQdjqeiOLOOELzzEz82865561;     lQdjqeiOLOOELzzEz82865561 = lQdjqeiOLOOELzzEz8676860;     lQdjqeiOLOOELzzEz8676860 = lQdjqeiOLOOELzzEz8917891;     lQdjqeiOLOOELzzEz8917891 = lQdjqeiOLOOELzzEz84066449;     lQdjqeiOLOOELzzEz84066449 = lQdjqeiOLOOELzzEz77668704;     lQdjqeiOLOOELzzEz77668704 = lQdjqeiOLOOELzzEz96355707;     lQdjqeiOLOOELzzEz96355707 = lQdjqeiOLOOELzzEz51571323;     lQdjqeiOLOOELzzEz51571323 = lQdjqeiOLOOELzzEz89485787;     lQdjqeiOLOOELzzEz89485787 = lQdjqeiOLOOELzzEz90192922;     lQdjqeiOLOOELzzEz90192922 = lQdjqeiOLOOELzzEz66002080;     lQdjqeiOLOOELzzEz66002080 = lQdjqeiOLOOELzzEz88904870;     lQdjqeiOLOOELzzEz88904870 = lQdjqeiOLOOELzzEz67658666;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void iRXpGtDvyZQTHrVY85874418() {     double YjnhukKYKVTSuCRKL73775772 = -936288776;    double YjnhukKYKVTSuCRKL60710489 = -516967133;    double YjnhukKYKVTSuCRKL90915801 = -280646480;    double YjnhukKYKVTSuCRKL19280223 = -525187467;    double YjnhukKYKVTSuCRKL76102563 = -511730235;    double YjnhukKYKVTSuCRKL52098773 = -41724670;    double YjnhukKYKVTSuCRKL78189486 = -746708898;    double YjnhukKYKVTSuCRKL11371689 = -55019717;    double YjnhukKYKVTSuCRKL59314962 = -882359116;    double YjnhukKYKVTSuCRKL94590663 = -234128573;    double YjnhukKYKVTSuCRKL7788755 = -162093973;    double YjnhukKYKVTSuCRKL97725181 = -916468263;    double YjnhukKYKVTSuCRKL31183188 = -758154215;    double YjnhukKYKVTSuCRKL20942610 = -498323199;    double YjnhukKYKVTSuCRKL543641 = -892082659;    double YjnhukKYKVTSuCRKL47639532 = -862324781;    double YjnhukKYKVTSuCRKL86230644 = -632901068;    double YjnhukKYKVTSuCRKL73659503 = -379963689;    double YjnhukKYKVTSuCRKL41910329 = -199274004;    double YjnhukKYKVTSuCRKL61746345 = -944887326;    double YjnhukKYKVTSuCRKL11661383 = -261393202;    double YjnhukKYKVTSuCRKL88957620 = 32293877;    double YjnhukKYKVTSuCRKL708772 = -823038733;    double YjnhukKYKVTSuCRKL86627924 = -47001846;    double YjnhukKYKVTSuCRKL96664001 = -985685123;    double YjnhukKYKVTSuCRKL98366372 = -701992139;    double YjnhukKYKVTSuCRKL58179473 = 34564977;    double YjnhukKYKVTSuCRKL79451635 = -101156745;    double YjnhukKYKVTSuCRKL42307275 = -271903555;    double YjnhukKYKVTSuCRKL51235261 = -523798118;    double YjnhukKYKVTSuCRKL50712526 = -267622737;    double YjnhukKYKVTSuCRKL41820394 = -599469810;    double YjnhukKYKVTSuCRKL83862347 = -472162663;    double YjnhukKYKVTSuCRKL44654418 = -516613470;    double YjnhukKYKVTSuCRKL9562283 = -216609597;    double YjnhukKYKVTSuCRKL29622003 = -735852107;    double YjnhukKYKVTSuCRKL62417893 = -823916433;    double YjnhukKYKVTSuCRKL57397752 = -755795097;    double YjnhukKYKVTSuCRKL84841335 = -248597533;    double YjnhukKYKVTSuCRKL84819522 = -470124610;    double YjnhukKYKVTSuCRKL8596559 = -825744266;    double YjnhukKYKVTSuCRKL7073023 = -857935115;    double YjnhukKYKVTSuCRKL80312743 = -897443598;    double YjnhukKYKVTSuCRKL48300848 = -991016225;    double YjnhukKYKVTSuCRKL4326028 = -465720590;    double YjnhukKYKVTSuCRKL19654763 = -317708602;    double YjnhukKYKVTSuCRKL30440849 = -602415614;    double YjnhukKYKVTSuCRKL76776101 = -19079963;    double YjnhukKYKVTSuCRKL14086414 = 70239246;    double YjnhukKYKVTSuCRKL29609800 = -137772785;    double YjnhukKYKVTSuCRKL15948832 = -158653162;    double YjnhukKYKVTSuCRKL11844632 = -101941610;    double YjnhukKYKVTSuCRKL42998106 = 93837460;    double YjnhukKYKVTSuCRKL63844412 = 93537144;    double YjnhukKYKVTSuCRKL74213036 = -216672464;    double YjnhukKYKVTSuCRKL84818152 = -868582654;    double YjnhukKYKVTSuCRKL60001718 = -693928401;    double YjnhukKYKVTSuCRKL4287878 = -133644635;    double YjnhukKYKVTSuCRKL22616221 = -539502344;    double YjnhukKYKVTSuCRKL77736191 = -809738097;    double YjnhukKYKVTSuCRKL93919300 = 23710353;    double YjnhukKYKVTSuCRKL98737851 = -545552153;    double YjnhukKYKVTSuCRKL69064413 = -783116162;    double YjnhukKYKVTSuCRKL8079702 = -258560998;    double YjnhukKYKVTSuCRKL43878137 = -966505837;    double YjnhukKYKVTSuCRKL65968361 = -562624164;    double YjnhukKYKVTSuCRKL13862834 = -344305600;    double YjnhukKYKVTSuCRKL86528770 = -141540746;    double YjnhukKYKVTSuCRKL11380327 = -181713602;    double YjnhukKYKVTSuCRKL70921637 = -56230552;    double YjnhukKYKVTSuCRKL85221639 = 61591651;    double YjnhukKYKVTSuCRKL28832893 = -877105972;    double YjnhukKYKVTSuCRKL88818167 = -31366156;    double YjnhukKYKVTSuCRKL57090807 = -729149394;    double YjnhukKYKVTSuCRKL53149787 = -19143060;    double YjnhukKYKVTSuCRKL4588361 = -403458088;    double YjnhukKYKVTSuCRKL8644878 = -70262525;    double YjnhukKYKVTSuCRKL52407923 = -832022509;    double YjnhukKYKVTSuCRKL82301897 = -581281257;    double YjnhukKYKVTSuCRKL77009239 = -567976521;    double YjnhukKYKVTSuCRKL67925523 = 423475;    double YjnhukKYKVTSuCRKL81403371 = -946355061;    double YjnhukKYKVTSuCRKL65365221 = -71395992;    double YjnhukKYKVTSuCRKL12697476 = -34130771;    double YjnhukKYKVTSuCRKL35286429 = -265144957;    double YjnhukKYKVTSuCRKL38867894 = -65681128;    double YjnhukKYKVTSuCRKL98822288 = -593307271;    double YjnhukKYKVTSuCRKL20017936 = -465699808;    double YjnhukKYKVTSuCRKL70441382 = -199941006;    double YjnhukKYKVTSuCRKL24744131 = -348026944;    double YjnhukKYKVTSuCRKL69620285 = 58076293;    double YjnhukKYKVTSuCRKL58130015 = -590271799;    double YjnhukKYKVTSuCRKL34781531 = -116292753;    double YjnhukKYKVTSuCRKL7105145 = -438859437;    double YjnhukKYKVTSuCRKL90900221 = -393834964;    double YjnhukKYKVTSuCRKL9858707 = -180192113;    double YjnhukKYKVTSuCRKL38008609 = 25181047;    double YjnhukKYKVTSuCRKL72233041 = -538882600;    double YjnhukKYKVTSuCRKL4422711 = 75489612;    double YjnhukKYKVTSuCRKL38357667 = -936288776;     YjnhukKYKVTSuCRKL73775772 = YjnhukKYKVTSuCRKL60710489;     YjnhukKYKVTSuCRKL60710489 = YjnhukKYKVTSuCRKL90915801;     YjnhukKYKVTSuCRKL90915801 = YjnhukKYKVTSuCRKL19280223;     YjnhukKYKVTSuCRKL19280223 = YjnhukKYKVTSuCRKL76102563;     YjnhukKYKVTSuCRKL76102563 = YjnhukKYKVTSuCRKL52098773;     YjnhukKYKVTSuCRKL52098773 = YjnhukKYKVTSuCRKL78189486;     YjnhukKYKVTSuCRKL78189486 = YjnhukKYKVTSuCRKL11371689;     YjnhukKYKVTSuCRKL11371689 = YjnhukKYKVTSuCRKL59314962;     YjnhukKYKVTSuCRKL59314962 = YjnhukKYKVTSuCRKL94590663;     YjnhukKYKVTSuCRKL94590663 = YjnhukKYKVTSuCRKL7788755;     YjnhukKYKVTSuCRKL7788755 = YjnhukKYKVTSuCRKL97725181;     YjnhukKYKVTSuCRKL97725181 = YjnhukKYKVTSuCRKL31183188;     YjnhukKYKVTSuCRKL31183188 = YjnhukKYKVTSuCRKL20942610;     YjnhukKYKVTSuCRKL20942610 = YjnhukKYKVTSuCRKL543641;     YjnhukKYKVTSuCRKL543641 = YjnhukKYKVTSuCRKL47639532;     YjnhukKYKVTSuCRKL47639532 = YjnhukKYKVTSuCRKL86230644;     YjnhukKYKVTSuCRKL86230644 = YjnhukKYKVTSuCRKL73659503;     YjnhukKYKVTSuCRKL73659503 = YjnhukKYKVTSuCRKL41910329;     YjnhukKYKVTSuCRKL41910329 = YjnhukKYKVTSuCRKL61746345;     YjnhukKYKVTSuCRKL61746345 = YjnhukKYKVTSuCRKL11661383;     YjnhukKYKVTSuCRKL11661383 = YjnhukKYKVTSuCRKL88957620;     YjnhukKYKVTSuCRKL88957620 = YjnhukKYKVTSuCRKL708772;     YjnhukKYKVTSuCRKL708772 = YjnhukKYKVTSuCRKL86627924;     YjnhukKYKVTSuCRKL86627924 = YjnhukKYKVTSuCRKL96664001;     YjnhukKYKVTSuCRKL96664001 = YjnhukKYKVTSuCRKL98366372;     YjnhukKYKVTSuCRKL98366372 = YjnhukKYKVTSuCRKL58179473;     YjnhukKYKVTSuCRKL58179473 = YjnhukKYKVTSuCRKL79451635;     YjnhukKYKVTSuCRKL79451635 = YjnhukKYKVTSuCRKL42307275;     YjnhukKYKVTSuCRKL42307275 = YjnhukKYKVTSuCRKL51235261;     YjnhukKYKVTSuCRKL51235261 = YjnhukKYKVTSuCRKL50712526;     YjnhukKYKVTSuCRKL50712526 = YjnhukKYKVTSuCRKL41820394;     YjnhukKYKVTSuCRKL41820394 = YjnhukKYKVTSuCRKL83862347;     YjnhukKYKVTSuCRKL83862347 = YjnhukKYKVTSuCRKL44654418;     YjnhukKYKVTSuCRKL44654418 = YjnhukKYKVTSuCRKL9562283;     YjnhukKYKVTSuCRKL9562283 = YjnhukKYKVTSuCRKL29622003;     YjnhukKYKVTSuCRKL29622003 = YjnhukKYKVTSuCRKL62417893;     YjnhukKYKVTSuCRKL62417893 = YjnhukKYKVTSuCRKL57397752;     YjnhukKYKVTSuCRKL57397752 = YjnhukKYKVTSuCRKL84841335;     YjnhukKYKVTSuCRKL84841335 = YjnhukKYKVTSuCRKL84819522;     YjnhukKYKVTSuCRKL84819522 = YjnhukKYKVTSuCRKL8596559;     YjnhukKYKVTSuCRKL8596559 = YjnhukKYKVTSuCRKL7073023;     YjnhukKYKVTSuCRKL7073023 = YjnhukKYKVTSuCRKL80312743;     YjnhukKYKVTSuCRKL80312743 = YjnhukKYKVTSuCRKL48300848;     YjnhukKYKVTSuCRKL48300848 = YjnhukKYKVTSuCRKL4326028;     YjnhukKYKVTSuCRKL4326028 = YjnhukKYKVTSuCRKL19654763;     YjnhukKYKVTSuCRKL19654763 = YjnhukKYKVTSuCRKL30440849;     YjnhukKYKVTSuCRKL30440849 = YjnhukKYKVTSuCRKL76776101;     YjnhukKYKVTSuCRKL76776101 = YjnhukKYKVTSuCRKL14086414;     YjnhukKYKVTSuCRKL14086414 = YjnhukKYKVTSuCRKL29609800;     YjnhukKYKVTSuCRKL29609800 = YjnhukKYKVTSuCRKL15948832;     YjnhukKYKVTSuCRKL15948832 = YjnhukKYKVTSuCRKL11844632;     YjnhukKYKVTSuCRKL11844632 = YjnhukKYKVTSuCRKL42998106;     YjnhukKYKVTSuCRKL42998106 = YjnhukKYKVTSuCRKL63844412;     YjnhukKYKVTSuCRKL63844412 = YjnhukKYKVTSuCRKL74213036;     YjnhukKYKVTSuCRKL74213036 = YjnhukKYKVTSuCRKL84818152;     YjnhukKYKVTSuCRKL84818152 = YjnhukKYKVTSuCRKL60001718;     YjnhukKYKVTSuCRKL60001718 = YjnhukKYKVTSuCRKL4287878;     YjnhukKYKVTSuCRKL4287878 = YjnhukKYKVTSuCRKL22616221;     YjnhukKYKVTSuCRKL22616221 = YjnhukKYKVTSuCRKL77736191;     YjnhukKYKVTSuCRKL77736191 = YjnhukKYKVTSuCRKL93919300;     YjnhukKYKVTSuCRKL93919300 = YjnhukKYKVTSuCRKL98737851;     YjnhukKYKVTSuCRKL98737851 = YjnhukKYKVTSuCRKL69064413;     YjnhukKYKVTSuCRKL69064413 = YjnhukKYKVTSuCRKL8079702;     YjnhukKYKVTSuCRKL8079702 = YjnhukKYKVTSuCRKL43878137;     YjnhukKYKVTSuCRKL43878137 = YjnhukKYKVTSuCRKL65968361;     YjnhukKYKVTSuCRKL65968361 = YjnhukKYKVTSuCRKL13862834;     YjnhukKYKVTSuCRKL13862834 = YjnhukKYKVTSuCRKL86528770;     YjnhukKYKVTSuCRKL86528770 = YjnhukKYKVTSuCRKL11380327;     YjnhukKYKVTSuCRKL11380327 = YjnhukKYKVTSuCRKL70921637;     YjnhukKYKVTSuCRKL70921637 = YjnhukKYKVTSuCRKL85221639;     YjnhukKYKVTSuCRKL85221639 = YjnhukKYKVTSuCRKL28832893;     YjnhukKYKVTSuCRKL28832893 = YjnhukKYKVTSuCRKL88818167;     YjnhukKYKVTSuCRKL88818167 = YjnhukKYKVTSuCRKL57090807;     YjnhukKYKVTSuCRKL57090807 = YjnhukKYKVTSuCRKL53149787;     YjnhukKYKVTSuCRKL53149787 = YjnhukKYKVTSuCRKL4588361;     YjnhukKYKVTSuCRKL4588361 = YjnhukKYKVTSuCRKL8644878;     YjnhukKYKVTSuCRKL8644878 = YjnhukKYKVTSuCRKL52407923;     YjnhukKYKVTSuCRKL52407923 = YjnhukKYKVTSuCRKL82301897;     YjnhukKYKVTSuCRKL82301897 = YjnhukKYKVTSuCRKL77009239;     YjnhukKYKVTSuCRKL77009239 = YjnhukKYKVTSuCRKL67925523;     YjnhukKYKVTSuCRKL67925523 = YjnhukKYKVTSuCRKL81403371;     YjnhukKYKVTSuCRKL81403371 = YjnhukKYKVTSuCRKL65365221;     YjnhukKYKVTSuCRKL65365221 = YjnhukKYKVTSuCRKL12697476;     YjnhukKYKVTSuCRKL12697476 = YjnhukKYKVTSuCRKL35286429;     YjnhukKYKVTSuCRKL35286429 = YjnhukKYKVTSuCRKL38867894;     YjnhukKYKVTSuCRKL38867894 = YjnhukKYKVTSuCRKL98822288;     YjnhukKYKVTSuCRKL98822288 = YjnhukKYKVTSuCRKL20017936;     YjnhukKYKVTSuCRKL20017936 = YjnhukKYKVTSuCRKL70441382;     YjnhukKYKVTSuCRKL70441382 = YjnhukKYKVTSuCRKL24744131;     YjnhukKYKVTSuCRKL24744131 = YjnhukKYKVTSuCRKL69620285;     YjnhukKYKVTSuCRKL69620285 = YjnhukKYKVTSuCRKL58130015;     YjnhukKYKVTSuCRKL58130015 = YjnhukKYKVTSuCRKL34781531;     YjnhukKYKVTSuCRKL34781531 = YjnhukKYKVTSuCRKL7105145;     YjnhukKYKVTSuCRKL7105145 = YjnhukKYKVTSuCRKL90900221;     YjnhukKYKVTSuCRKL90900221 = YjnhukKYKVTSuCRKL9858707;     YjnhukKYKVTSuCRKL9858707 = YjnhukKYKVTSuCRKL38008609;     YjnhukKYKVTSuCRKL38008609 = YjnhukKYKVTSuCRKL72233041;     YjnhukKYKVTSuCRKL72233041 = YjnhukKYKVTSuCRKL4422711;     YjnhukKYKVTSuCRKL4422711 = YjnhukKYKVTSuCRKL38357667;     YjnhukKYKVTSuCRKL38357667 = YjnhukKYKVTSuCRKL73775772;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void fFFccFicEqfGftTU53166018() {     double RFcAZTRJyNdmLbLCQ9234741 = -302565676;    double RFcAZTRJyNdmLbLCQ73982568 = -397518740;    double RFcAZTRJyNdmLbLCQ28302279 = 53686445;    double RFcAZTRJyNdmLbLCQ91503577 = -346338538;    double RFcAZTRJyNdmLbLCQ32387833 = -617329081;    double RFcAZTRJyNdmLbLCQ88321645 = -304072565;    double RFcAZTRJyNdmLbLCQ88846893 = -591434632;    double RFcAZTRJyNdmLbLCQ73503249 = -799253857;    double RFcAZTRJyNdmLbLCQ60832567 = -270690523;    double RFcAZTRJyNdmLbLCQ9493376 = -998815571;    double RFcAZTRJyNdmLbLCQ45970490 = 35898297;    double RFcAZTRJyNdmLbLCQ98293329 = -780589035;    double RFcAZTRJyNdmLbLCQ69417833 = -210166720;    double RFcAZTRJyNdmLbLCQ74083473 = -474686608;    double RFcAZTRJyNdmLbLCQ58738157 = -295507674;    double RFcAZTRJyNdmLbLCQ31180833 = -498136388;    double RFcAZTRJyNdmLbLCQ42066580 = -59875621;    double RFcAZTRJyNdmLbLCQ73419694 = -453391757;    double RFcAZTRJyNdmLbLCQ42460659 = -515837425;    double RFcAZTRJyNdmLbLCQ55583564 = -271448391;    double RFcAZTRJyNdmLbLCQ24748246 = 81332109;    double RFcAZTRJyNdmLbLCQ92379145 = -54342417;    double RFcAZTRJyNdmLbLCQ71234151 = -460670917;    double RFcAZTRJyNdmLbLCQ80420869 = -557826450;    double RFcAZTRJyNdmLbLCQ4681953 = 90329679;    double RFcAZTRJyNdmLbLCQ9064545 = -390568946;    double RFcAZTRJyNdmLbLCQ18600047 = -871719164;    double RFcAZTRJyNdmLbLCQ20180060 = -270928920;    double RFcAZTRJyNdmLbLCQ88975490 = -845998358;    double RFcAZTRJyNdmLbLCQ56768672 = -558305289;    double RFcAZTRJyNdmLbLCQ69295757 = -819603224;    double RFcAZTRJyNdmLbLCQ79643412 = -372441224;    double RFcAZTRJyNdmLbLCQ96080358 = -619758641;    double RFcAZTRJyNdmLbLCQ74402844 = -581307284;    double RFcAZTRJyNdmLbLCQ75370736 = -167709848;    double RFcAZTRJyNdmLbLCQ81182846 = -950417413;    double RFcAZTRJyNdmLbLCQ11829710 = -688612473;    double RFcAZTRJyNdmLbLCQ8613156 = -738450678;    double RFcAZTRJyNdmLbLCQ15231897 = -999473810;    double RFcAZTRJyNdmLbLCQ49091054 = 64612844;    double RFcAZTRJyNdmLbLCQ42048166 = -594943180;    double RFcAZTRJyNdmLbLCQ96438149 = -452480406;    double RFcAZTRJyNdmLbLCQ69696121 = -450624384;    double RFcAZTRJyNdmLbLCQ88432186 = -495348635;    double RFcAZTRJyNdmLbLCQ4342487 = -292231751;    double RFcAZTRJyNdmLbLCQ65616091 = -356632234;    double RFcAZTRJyNdmLbLCQ49786013 = 13598025;    double RFcAZTRJyNdmLbLCQ94502413 = -185606556;    double RFcAZTRJyNdmLbLCQ36979080 = -693497634;    double RFcAZTRJyNdmLbLCQ54139389 = -628799600;    double RFcAZTRJyNdmLbLCQ96719555 = -32929719;    double RFcAZTRJyNdmLbLCQ95330103 = -315593956;    double RFcAZTRJyNdmLbLCQ11619255 = -762208094;    double RFcAZTRJyNdmLbLCQ79836259 = -97971355;    double RFcAZTRJyNdmLbLCQ28417772 = -563162610;    double RFcAZTRJyNdmLbLCQ16855596 = -148223260;    double RFcAZTRJyNdmLbLCQ2748417 = -936847823;    double RFcAZTRJyNdmLbLCQ47881409 = -388487106;    double RFcAZTRJyNdmLbLCQ86821624 = -336668217;    double RFcAZTRJyNdmLbLCQ23323289 = -126760135;    double RFcAZTRJyNdmLbLCQ69721598 = -432353402;    double RFcAZTRJyNdmLbLCQ68666833 = -220505713;    double RFcAZTRJyNdmLbLCQ84527759 = -953255500;    double RFcAZTRJyNdmLbLCQ4063896 = -712385234;    double RFcAZTRJyNdmLbLCQ40197618 = -79212348;    double RFcAZTRJyNdmLbLCQ66327077 = -591660479;    double RFcAZTRJyNdmLbLCQ2212971 = -60830395;    double RFcAZTRJyNdmLbLCQ95014989 = -628859437;    double RFcAZTRJyNdmLbLCQ98712736 = -206976761;    double RFcAZTRJyNdmLbLCQ77555310 = -345090262;    double RFcAZTRJyNdmLbLCQ19351124 = -809523916;    double RFcAZTRJyNdmLbLCQ33453425 = -321424943;    double RFcAZTRJyNdmLbLCQ58187798 = -453917948;    double RFcAZTRJyNdmLbLCQ93369605 = -480450269;    double RFcAZTRJyNdmLbLCQ13535398 = -676505211;    double RFcAZTRJyNdmLbLCQ28310097 = -466187485;    double RFcAZTRJyNdmLbLCQ22683024 = -603718033;    double RFcAZTRJyNdmLbLCQ82801965 = -965322283;    double RFcAZTRJyNdmLbLCQ76078383 = -165594700;    double RFcAZTRJyNdmLbLCQ39065862 = -553038088;    double RFcAZTRJyNdmLbLCQ59278532 = -304166971;    double RFcAZTRJyNdmLbLCQ24097634 = -586112609;    double RFcAZTRJyNdmLbLCQ83200980 = -577431286;    double RFcAZTRJyNdmLbLCQ34836101 = -117198758;    double RFcAZTRJyNdmLbLCQ60049116 = -425375570;    double RFcAZTRJyNdmLbLCQ73965653 = -404009268;    double RFcAZTRJyNdmLbLCQ68024158 = -610233131;    double RFcAZTRJyNdmLbLCQ16244100 = -421787286;    double RFcAZTRJyNdmLbLCQ45985072 = 81855326;    double RFcAZTRJyNdmLbLCQ58515141 = 80513411;    double RFcAZTRJyNdmLbLCQ78434430 = 86430410;    double RFcAZTRJyNdmLbLCQ63948300 = -200125368;    double RFcAZTRJyNdmLbLCQ21791531 = -301782461;    double RFcAZTRJyNdmLbLCQ91908608 = -772713675;    double RFcAZTRJyNdmLbLCQ79369455 = -503033755;    double RFcAZTRJyNdmLbLCQ73381333 = -274437468;    double RFcAZTRJyNdmLbLCQ11910390 = -499224906;    double RFcAZTRJyNdmLbLCQ65632225 = -738239150;    double RFcAZTRJyNdmLbLCQ48234569 = -316136287;    double RFcAZTRJyNdmLbLCQ38015409 = -302565676;     RFcAZTRJyNdmLbLCQ9234741 = RFcAZTRJyNdmLbLCQ73982568;     RFcAZTRJyNdmLbLCQ73982568 = RFcAZTRJyNdmLbLCQ28302279;     RFcAZTRJyNdmLbLCQ28302279 = RFcAZTRJyNdmLbLCQ91503577;     RFcAZTRJyNdmLbLCQ91503577 = RFcAZTRJyNdmLbLCQ32387833;     RFcAZTRJyNdmLbLCQ32387833 = RFcAZTRJyNdmLbLCQ88321645;     RFcAZTRJyNdmLbLCQ88321645 = RFcAZTRJyNdmLbLCQ88846893;     RFcAZTRJyNdmLbLCQ88846893 = RFcAZTRJyNdmLbLCQ73503249;     RFcAZTRJyNdmLbLCQ73503249 = RFcAZTRJyNdmLbLCQ60832567;     RFcAZTRJyNdmLbLCQ60832567 = RFcAZTRJyNdmLbLCQ9493376;     RFcAZTRJyNdmLbLCQ9493376 = RFcAZTRJyNdmLbLCQ45970490;     RFcAZTRJyNdmLbLCQ45970490 = RFcAZTRJyNdmLbLCQ98293329;     RFcAZTRJyNdmLbLCQ98293329 = RFcAZTRJyNdmLbLCQ69417833;     RFcAZTRJyNdmLbLCQ69417833 = RFcAZTRJyNdmLbLCQ74083473;     RFcAZTRJyNdmLbLCQ74083473 = RFcAZTRJyNdmLbLCQ58738157;     RFcAZTRJyNdmLbLCQ58738157 = RFcAZTRJyNdmLbLCQ31180833;     RFcAZTRJyNdmLbLCQ31180833 = RFcAZTRJyNdmLbLCQ42066580;     RFcAZTRJyNdmLbLCQ42066580 = RFcAZTRJyNdmLbLCQ73419694;     RFcAZTRJyNdmLbLCQ73419694 = RFcAZTRJyNdmLbLCQ42460659;     RFcAZTRJyNdmLbLCQ42460659 = RFcAZTRJyNdmLbLCQ55583564;     RFcAZTRJyNdmLbLCQ55583564 = RFcAZTRJyNdmLbLCQ24748246;     RFcAZTRJyNdmLbLCQ24748246 = RFcAZTRJyNdmLbLCQ92379145;     RFcAZTRJyNdmLbLCQ92379145 = RFcAZTRJyNdmLbLCQ71234151;     RFcAZTRJyNdmLbLCQ71234151 = RFcAZTRJyNdmLbLCQ80420869;     RFcAZTRJyNdmLbLCQ80420869 = RFcAZTRJyNdmLbLCQ4681953;     RFcAZTRJyNdmLbLCQ4681953 = RFcAZTRJyNdmLbLCQ9064545;     RFcAZTRJyNdmLbLCQ9064545 = RFcAZTRJyNdmLbLCQ18600047;     RFcAZTRJyNdmLbLCQ18600047 = RFcAZTRJyNdmLbLCQ20180060;     RFcAZTRJyNdmLbLCQ20180060 = RFcAZTRJyNdmLbLCQ88975490;     RFcAZTRJyNdmLbLCQ88975490 = RFcAZTRJyNdmLbLCQ56768672;     RFcAZTRJyNdmLbLCQ56768672 = RFcAZTRJyNdmLbLCQ69295757;     RFcAZTRJyNdmLbLCQ69295757 = RFcAZTRJyNdmLbLCQ79643412;     RFcAZTRJyNdmLbLCQ79643412 = RFcAZTRJyNdmLbLCQ96080358;     RFcAZTRJyNdmLbLCQ96080358 = RFcAZTRJyNdmLbLCQ74402844;     RFcAZTRJyNdmLbLCQ74402844 = RFcAZTRJyNdmLbLCQ75370736;     RFcAZTRJyNdmLbLCQ75370736 = RFcAZTRJyNdmLbLCQ81182846;     RFcAZTRJyNdmLbLCQ81182846 = RFcAZTRJyNdmLbLCQ11829710;     RFcAZTRJyNdmLbLCQ11829710 = RFcAZTRJyNdmLbLCQ8613156;     RFcAZTRJyNdmLbLCQ8613156 = RFcAZTRJyNdmLbLCQ15231897;     RFcAZTRJyNdmLbLCQ15231897 = RFcAZTRJyNdmLbLCQ49091054;     RFcAZTRJyNdmLbLCQ49091054 = RFcAZTRJyNdmLbLCQ42048166;     RFcAZTRJyNdmLbLCQ42048166 = RFcAZTRJyNdmLbLCQ96438149;     RFcAZTRJyNdmLbLCQ96438149 = RFcAZTRJyNdmLbLCQ69696121;     RFcAZTRJyNdmLbLCQ69696121 = RFcAZTRJyNdmLbLCQ88432186;     RFcAZTRJyNdmLbLCQ88432186 = RFcAZTRJyNdmLbLCQ4342487;     RFcAZTRJyNdmLbLCQ4342487 = RFcAZTRJyNdmLbLCQ65616091;     RFcAZTRJyNdmLbLCQ65616091 = RFcAZTRJyNdmLbLCQ49786013;     RFcAZTRJyNdmLbLCQ49786013 = RFcAZTRJyNdmLbLCQ94502413;     RFcAZTRJyNdmLbLCQ94502413 = RFcAZTRJyNdmLbLCQ36979080;     RFcAZTRJyNdmLbLCQ36979080 = RFcAZTRJyNdmLbLCQ54139389;     RFcAZTRJyNdmLbLCQ54139389 = RFcAZTRJyNdmLbLCQ96719555;     RFcAZTRJyNdmLbLCQ96719555 = RFcAZTRJyNdmLbLCQ95330103;     RFcAZTRJyNdmLbLCQ95330103 = RFcAZTRJyNdmLbLCQ11619255;     RFcAZTRJyNdmLbLCQ11619255 = RFcAZTRJyNdmLbLCQ79836259;     RFcAZTRJyNdmLbLCQ79836259 = RFcAZTRJyNdmLbLCQ28417772;     RFcAZTRJyNdmLbLCQ28417772 = RFcAZTRJyNdmLbLCQ16855596;     RFcAZTRJyNdmLbLCQ16855596 = RFcAZTRJyNdmLbLCQ2748417;     RFcAZTRJyNdmLbLCQ2748417 = RFcAZTRJyNdmLbLCQ47881409;     RFcAZTRJyNdmLbLCQ47881409 = RFcAZTRJyNdmLbLCQ86821624;     RFcAZTRJyNdmLbLCQ86821624 = RFcAZTRJyNdmLbLCQ23323289;     RFcAZTRJyNdmLbLCQ23323289 = RFcAZTRJyNdmLbLCQ69721598;     RFcAZTRJyNdmLbLCQ69721598 = RFcAZTRJyNdmLbLCQ68666833;     RFcAZTRJyNdmLbLCQ68666833 = RFcAZTRJyNdmLbLCQ84527759;     RFcAZTRJyNdmLbLCQ84527759 = RFcAZTRJyNdmLbLCQ4063896;     RFcAZTRJyNdmLbLCQ4063896 = RFcAZTRJyNdmLbLCQ40197618;     RFcAZTRJyNdmLbLCQ40197618 = RFcAZTRJyNdmLbLCQ66327077;     RFcAZTRJyNdmLbLCQ66327077 = RFcAZTRJyNdmLbLCQ2212971;     RFcAZTRJyNdmLbLCQ2212971 = RFcAZTRJyNdmLbLCQ95014989;     RFcAZTRJyNdmLbLCQ95014989 = RFcAZTRJyNdmLbLCQ98712736;     RFcAZTRJyNdmLbLCQ98712736 = RFcAZTRJyNdmLbLCQ77555310;     RFcAZTRJyNdmLbLCQ77555310 = RFcAZTRJyNdmLbLCQ19351124;     RFcAZTRJyNdmLbLCQ19351124 = RFcAZTRJyNdmLbLCQ33453425;     RFcAZTRJyNdmLbLCQ33453425 = RFcAZTRJyNdmLbLCQ58187798;     RFcAZTRJyNdmLbLCQ58187798 = RFcAZTRJyNdmLbLCQ93369605;     RFcAZTRJyNdmLbLCQ93369605 = RFcAZTRJyNdmLbLCQ13535398;     RFcAZTRJyNdmLbLCQ13535398 = RFcAZTRJyNdmLbLCQ28310097;     RFcAZTRJyNdmLbLCQ28310097 = RFcAZTRJyNdmLbLCQ22683024;     RFcAZTRJyNdmLbLCQ22683024 = RFcAZTRJyNdmLbLCQ82801965;     RFcAZTRJyNdmLbLCQ82801965 = RFcAZTRJyNdmLbLCQ76078383;     RFcAZTRJyNdmLbLCQ76078383 = RFcAZTRJyNdmLbLCQ39065862;     RFcAZTRJyNdmLbLCQ39065862 = RFcAZTRJyNdmLbLCQ59278532;     RFcAZTRJyNdmLbLCQ59278532 = RFcAZTRJyNdmLbLCQ24097634;     RFcAZTRJyNdmLbLCQ24097634 = RFcAZTRJyNdmLbLCQ83200980;     RFcAZTRJyNdmLbLCQ83200980 = RFcAZTRJyNdmLbLCQ34836101;     RFcAZTRJyNdmLbLCQ34836101 = RFcAZTRJyNdmLbLCQ60049116;     RFcAZTRJyNdmLbLCQ60049116 = RFcAZTRJyNdmLbLCQ73965653;     RFcAZTRJyNdmLbLCQ73965653 = RFcAZTRJyNdmLbLCQ68024158;     RFcAZTRJyNdmLbLCQ68024158 = RFcAZTRJyNdmLbLCQ16244100;     RFcAZTRJyNdmLbLCQ16244100 = RFcAZTRJyNdmLbLCQ45985072;     RFcAZTRJyNdmLbLCQ45985072 = RFcAZTRJyNdmLbLCQ58515141;     RFcAZTRJyNdmLbLCQ58515141 = RFcAZTRJyNdmLbLCQ78434430;     RFcAZTRJyNdmLbLCQ78434430 = RFcAZTRJyNdmLbLCQ63948300;     RFcAZTRJyNdmLbLCQ63948300 = RFcAZTRJyNdmLbLCQ21791531;     RFcAZTRJyNdmLbLCQ21791531 = RFcAZTRJyNdmLbLCQ91908608;     RFcAZTRJyNdmLbLCQ91908608 = RFcAZTRJyNdmLbLCQ79369455;     RFcAZTRJyNdmLbLCQ79369455 = RFcAZTRJyNdmLbLCQ73381333;     RFcAZTRJyNdmLbLCQ73381333 = RFcAZTRJyNdmLbLCQ11910390;     RFcAZTRJyNdmLbLCQ11910390 = RFcAZTRJyNdmLbLCQ65632225;     RFcAZTRJyNdmLbLCQ65632225 = RFcAZTRJyNdmLbLCQ48234569;     RFcAZTRJyNdmLbLCQ48234569 = RFcAZTRJyNdmLbLCQ38015409;     RFcAZTRJyNdmLbLCQ38015409 = RFcAZTRJyNdmLbLCQ9234741;}
// Junk Finished
