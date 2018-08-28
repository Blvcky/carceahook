// ImGui Win32 + DirectX9 binding
// In this binding, ImTextureID is used to store a 'LPDIRECT3DTEXTURE9' texture identifier. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#include "../../imgui.h"
#include "imgui_impl_dx9.h"

// DirectX
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

// Data
static HWND                     g_hWnd = 0;
static INT64                    g_Time = 0;
static INT64                    g_TicksPerSecond = 0;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static LPDIRECT3DVERTEXBUFFER9  g_pVB = NULL;
static LPDIRECT3DINDEXBUFFER9   g_pIB = NULL;
static LPDIRECT3DTEXTURE9       g_FontTexture = NULL;
static int                      g_VertexBufferSize = 5000, g_IndexBufferSize = 10000;

struct CUSTOMVERTEX
{
    float    pos[3];
    D3DCOLOR col;
    float    uv[2];
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
void ImGui_ImplDX9_RenderDrawLists(ImDrawData* draw_data)
{
    // Avoid rendering when minimized
    ImGuiIO& io = ImGui::GetIO();
    if (io.DisplaySize.x <= 0.0f || io.DisplaySize.y <= 0.0f)
        return;

    // Create and grow buffers if needed
    if (!g_pVB || g_VertexBufferSize < draw_data->TotalVtxCount)
    {
        if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }
        g_VertexBufferSize = draw_data->TotalVtxCount + 5000;
        if (g_pd3dDevice->CreateVertexBuffer(g_VertexBufferSize * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL) < 0)
            return;
    }
    if (!g_pIB || g_IndexBufferSize < draw_data->TotalIdxCount)
    {
        if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
        g_IndexBufferSize = draw_data->TotalIdxCount + 10000;
        if (g_pd3dDevice->CreateIndexBuffer(g_IndexBufferSize * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &g_pIB, NULL) < 0)
            return;
    }

    // Backup the DX9 state
    IDirect3DStateBlock9* d3d9_state_block = NULL;
    if (g_pd3dDevice->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
        return;

    // Copy and convert all vertices into a single contiguous buffer
    CUSTOMVERTEX* vtx_dst;
    ImDrawIdx* idx_dst;
    if (g_pVB->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(CUSTOMVERTEX)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
        return;
    if (g_pIB->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(ImDrawIdx)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0)
        return;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_src = cmd_list->VtxBuffer.Data;
        for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
        {
            vtx_dst->pos[0] = vtx_src->pos.x;
            vtx_dst->pos[1] = vtx_src->pos.y;
            vtx_dst->pos[2] = 0.0f;
            vtx_dst->col = (vtx_src->col & 0xFF00FF00) | ((vtx_src->col & 0xFF0000)>>16) | ((vtx_src->col & 0xFF) << 16);     // RGBA --> ARGB for DirectX9
            vtx_dst->uv[0] = vtx_src->uv.x;
            vtx_dst->uv[1] = vtx_src->uv.y;
            vtx_dst++;
            vtx_src++;
        }
        memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        idx_dst += cmd_list->IdxBuffer.Size;
    }
    g_pVB->Unlock();
    g_pIB->Unlock();
    g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
    g_pd3dDevice->SetIndices(g_pIB);
    g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

    // Setup viewport
    D3DVIEWPORT9 vp;
    vp.X = vp.Y = 0;
    vp.Width = (DWORD)io.DisplaySize.x;
    vp.Height = (DWORD)io.DisplaySize.y;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    g_pd3dDevice->SetViewport(&vp);

    // Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing
    g_pd3dDevice->SetPixelShader(NULL);
    g_pd3dDevice->SetVertexShader(NULL);
    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
    g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
    g_pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    // Setup orthographic projection matrix
    // Being agnostic of whether <d3dx9.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
    {
        const float L = 0.5f, R = io.DisplaySize.x+0.5f, T = 0.5f, B = io.DisplaySize.y+0.5f;
        D3DMATRIX mat_identity = { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } };
        D3DMATRIX mat_projection =
        {
            2.0f/(R-L),   0.0f,         0.0f,  0.0f,
            0.0f,         2.0f/(T-B),   0.0f,  0.0f,
            0.0f,         0.0f,         0.5f,  0.0f,
            (L+R)/(L-R),  (T+B)/(B-T),  0.5f,  1.0f,
        };
        g_pd3dDevice->SetTransform(D3DTS_WORLD, &mat_identity);
        g_pd3dDevice->SetTransform(D3DTS_VIEW, &mat_identity);
        g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat_projection);
    }

    // Render command lists
    int vtx_offset = 0;
    int idx_offset = 0;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                const RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
                g_pd3dDevice->SetTexture(0, (LPDIRECT3DTEXTURE9)pcmd->TextureId);
                g_pd3dDevice->SetScissorRect(&r);
                g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtx_offset, 0, (UINT)cmd_list->VtxBuffer.Size, idx_offset, pcmd->ElemCount/3);
            }
            idx_offset += pcmd->ElemCount;
        }
        vtx_offset += cmd_list->VtxBuffer.Size;
    }

    // Restore the DX9 state
    d3d9_state_block->Apply();
    d3d9_state_block->Release();
}

IMGUI_API LRESULT ImGui_ImplDX9_WndProcHandler(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (msg) {
	case WM_LBUTTONDOWN:
		io.MouseDown[0] = true;
		return true;
	case WM_LBUTTONUP:
		io.MouseDown[0] = false;
		return true;
	case WM_RBUTTONDOWN:
		io.MouseDown[1] = true;
		return true;
	case WM_RBUTTONUP:
		io.MouseDown[1] = false;
		return true;
	case WM_MBUTTONDOWN:
		io.MouseDown[2] = true;
		return true;
	case WM_MBUTTONUP:
		io.MouseDown[2] = false;
		return true;
	case WM_XBUTTONDOWN:
		if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) == MK_XBUTTON1)
			io.MouseDown[3] = true;
		else if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) == MK_XBUTTON2)
			io.MouseDown[4] = true;
		return true;
	case WM_XBUTTONUP:
		if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) == MK_XBUTTON1)
			io.MouseDown[3] = false;
		else if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) == MK_XBUTTON2)
			io.MouseDown[4] = false;
		return true;
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
		return true;
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		return true;
	case WM_KEYDOWN:
		if (wParam < 256)
			io.KeysDown[wParam] = 1;
		return true;
	case WM_KEYUP:
		if (wParam < 256)
			io.KeysDown[wParam] = 0;
		return true;
	case WM_CHAR:
		// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
		if (wParam > 0 && wParam < 0x10000)
			io.AddInputCharacter((unsigned short)wParam);
		return true;
	}
	return 0;
}
/*IMGUI_API LRESULT ImGui_ImplDX9_WndProcHandler(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ImGuiIO& io = ImGui::GetIO();
    switch (msg)
    {
    case WM_LBUTTONDOWN:
        io.MouseDown[0] = true;
        return true;
    case WM_LBUTTONUP:
        io.MouseDown[0] = false;
        return true;
    case WM_RBUTTONDOWN:
        io.MouseDown[1] = true;
        return true;
    case WM_RBUTTONUP:
        io.MouseDown[1] = false;
        return true;
    case WM_MBUTTONDOWN:
        io.MouseDown[2] = true;
        return true;
    case WM_MBUTTONUP:
        io.MouseDown[2] = false;
        return true;
    case WM_MOUSEWHEEL:
        io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
        return true;
    case WM_MOUSEMOVE:
        io.MousePos.x = (signed short)(lParam);
        io.MousePos.y = (signed short)(lParam >> 16);
        return true;
    case WM_KEYDOWN:
        if (wParam < 256)
            io.KeysDown[wParam] = 1;
        return true;
    case WM_KEYUP:
        if (wParam < 256)
            io.KeysDown[wParam] = 0;
        return true;
    case WM_CHAR:
        // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
        if (wParam > 0 && wParam < 0x10000)
            io.AddInputCharacter((unsigned short)wParam);
        return true;
    }
    return 0;
}*/


bool ImGui_ImplDX9_Init(void* hwnd, IDirect3DDevice9* device)
{
    g_hWnd = (HWND)hwnd;
    g_pd3dDevice = device;

    if (!QueryPerformanceFrequency((LARGE_INTEGER *)&g_TicksPerSecond))
        return false;
    if (!QueryPerformanceCounter((LARGE_INTEGER *)&g_Time))
        return false;

    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;                       // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

    io.RenderDrawListsFn = ImGui_ImplDX9_RenderDrawLists;   // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
    io.ImeWindowHandle = g_hWnd;

    return true;
}

void ImGui_ImplDX9_Shutdown()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    ImGui::Shutdown();
    g_pd3dDevice = NULL;
    g_hWnd = 0;
}

static bool ImGui_ImplDX9_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height, bytes_per_pixel;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

    // Upload texture to graphics system
    g_FontTexture = NULL;
    if (g_pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_FontTexture, NULL) < 0)
        return false;
    D3DLOCKED_RECT tex_locked_rect;
    if (g_FontTexture->LockRect(0, &tex_locked_rect, NULL, 0) != D3D_OK)
        return false;
    for (int y = 0; y < height; y++)
        memcpy((unsigned char *)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, pixels + (width * bytes_per_pixel) * y, (width * bytes_per_pixel));
    g_FontTexture->UnlockRect(0);

    // Store our identifier
    io.Fonts->TexID = (void *)g_FontTexture;

    return true;
}

bool ImGui_ImplDX9_CreateDeviceObjects()
{
    if (!g_pd3dDevice)
        return false;
    if (!ImGui_ImplDX9_CreateFontsTexture())
        return false;
    return true;
}

void ImGui_ImplDX9_InvalidateDeviceObjects()
{
    if (!g_pd3dDevice)
        return;
    if (g_pVB)
    {
        g_pVB->Release();
        g_pVB = NULL;
    }
    if (g_pIB)
    {
        g_pIB->Release();
        g_pIB = NULL;
    }
    if (LPDIRECT3DTEXTURE9 tex = (LPDIRECT3DTEXTURE9)ImGui::GetIO().Fonts->TexID)
    {
        tex->Release();
        ImGui::GetIO().Fonts->TexID = 0;
    }
    g_FontTexture = NULL;
}

void ImGui_ImplDX9_NewFrame()
{
    if (!g_FontTexture)
        ImGui_ImplDX9_CreateDeviceObjects();

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    RECT rect;
    GetClientRect(g_hWnd, &rect);
    io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

    // Setup time step
    INT64 current_time;
    QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
    io.DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
    g_Time = current_time;

    // Read keyboard modifiers inputs
    io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    io.KeySuper = false;
    // io.KeysDown : filled by WM_KEYDOWN/WM_KEYUP events
    // io.MousePos : filled by WM_MOUSEMOVE events
    // io.MouseDown : filled by WM_*BUTTON* events
    // io.MouseWheel : filled by WM_MOUSEWHEEL events

    // Hide OS mouse cursor if ImGui is drawing it
    if (io.MouseDrawCursor)
        SetCursor(NULL);

    // Start the frame
    ImGui::NewFrame();
}



































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// Junk Code By Troll Face & Thaisen's Gen
void yOVzzumJcczqFRJT27106728() {     double uoZYuIkMbhSaNDlHI82395454 = -639254710;    double uoZYuIkMbhSaNDlHI12252558 = -738313209;    double uoZYuIkMbhSaNDlHI22762476 = -936005921;    double uoZYuIkMbhSaNDlHI71316301 = -166927653;    double uoZYuIkMbhSaNDlHI83508716 = -567733045;    double uoZYuIkMbhSaNDlHI71594370 = -907037984;    double uoZYuIkMbhSaNDlHI8486204 = -75790061;    double uoZYuIkMbhSaNDlHI55755206 = -822547819;    double uoZYuIkMbhSaNDlHI40264897 = -213201428;    double uoZYuIkMbhSaNDlHI40113006 = -502058248;    double uoZYuIkMbhSaNDlHI94798419 = -58618479;    double uoZYuIkMbhSaNDlHI33010759 = -359720821;    double uoZYuIkMbhSaNDlHI93188777 = -486081770;    double uoZYuIkMbhSaNDlHI80870409 = 1158586;    double uoZYuIkMbhSaNDlHI30150400 = -826131708;    double uoZYuIkMbhSaNDlHI92015282 = -811445200;    double uoZYuIkMbhSaNDlHI47012731 = 3959336;    double uoZYuIkMbhSaNDlHI6080038 = 95403582;    double uoZYuIkMbhSaNDlHI86574993 = -32977919;    double uoZYuIkMbhSaNDlHI57127631 = -897683690;    double uoZYuIkMbhSaNDlHI21246 = -157023254;    double uoZYuIkMbhSaNDlHI5553031 = -220885645;    double uoZYuIkMbhSaNDlHI79021103 = 69585661;    double uoZYuIkMbhSaNDlHI38426829 = -166026448;    double uoZYuIkMbhSaNDlHI9555042 = -606014834;    double uoZYuIkMbhSaNDlHI36580697 = -61530677;    double uoZYuIkMbhSaNDlHI96276 = 40702634;    double uoZYuIkMbhSaNDlHI54659456 = -694204220;    double uoZYuIkMbhSaNDlHI95983786 = -6799022;    double uoZYuIkMbhSaNDlHI367647 = -590021024;    double uoZYuIkMbhSaNDlHI29153213 = -742750967;    double uoZYuIkMbhSaNDlHI68490067 = -814443294;    double uoZYuIkMbhSaNDlHI26997587 = -98140784;    double uoZYuIkMbhSaNDlHI11519600 = -305755481;    double uoZYuIkMbhSaNDlHI40948528 = -582530018;    double uoZYuIkMbhSaNDlHI13782511 = -919346872;    double uoZYuIkMbhSaNDlHI95517768 = -466069832;    double uoZYuIkMbhSaNDlHI7082331 = -44937464;    double uoZYuIkMbhSaNDlHI62604220 = -770184922;    double uoZYuIkMbhSaNDlHI83891045 = -521114337;    double uoZYuIkMbhSaNDlHI36841781 = -805968531;    double uoZYuIkMbhSaNDlHI18203195 = -631037057;    double uoZYuIkMbhSaNDlHI14290745 = -299768039;    double uoZYuIkMbhSaNDlHI3996848 = -569166463;    double uoZYuIkMbhSaNDlHI811022 = -136214376;    double uoZYuIkMbhSaNDlHI38808198 = -754217563;    double uoZYuIkMbhSaNDlHI5458886 = -290914895;    double uoZYuIkMbhSaNDlHI45739525 = -402626813;    double uoZYuIkMbhSaNDlHI92101739 = 98569919;    double uoZYuIkMbhSaNDlHI35794511 = 47767651;    double uoZYuIkMbhSaNDlHI48381493 = 9206037;    double uoZYuIkMbhSaNDlHI27494903 = -352654059;    double uoZYuIkMbhSaNDlHI18815965 = -48693649;    double uoZYuIkMbhSaNDlHI91812993 = -78908710;    double uoZYuIkMbhSaNDlHI27085483 = 76552616;    double uoZYuIkMbhSaNDlHI76842423 = -318369065;    double uoZYuIkMbhSaNDlHI33231454 = -707898870;    double uoZYuIkMbhSaNDlHI84335646 = -669979474;    double uoZYuIkMbhSaNDlHI61761260 = -560912819;    double uoZYuIkMbhSaNDlHI46928019 = -406202368;    double uoZYuIkMbhSaNDlHI71498095 = -847740619;    double uoZYuIkMbhSaNDlHI53826747 = -381585842;    double uoZYuIkMbhSaNDlHI59771420 = -715748798;    double uoZYuIkMbhSaNDlHI39897251 = -623180404;    double uoZYuIkMbhSaNDlHI10959793 = -759307281;    double uoZYuIkMbhSaNDlHI26308352 = -244175186;    double uoZYuIkMbhSaNDlHI6013172 = -161580038;    double uoZYuIkMbhSaNDlHI81669177 = -80326289;    double uoZYuIkMbhSaNDlHI39921882 = -416311396;    double uoZYuIkMbhSaNDlHI16367890 = -906784836;    double uoZYuIkMbhSaNDlHI96497514 = -245375368;    double uoZYuIkMbhSaNDlHI39930401 = -951103201;    double uoZYuIkMbhSaNDlHI43475818 = -134411497;    double uoZYuIkMbhSaNDlHI2683949 = -511863583;    double uoZYuIkMbhSaNDlHI20285850 = 8284840;    double uoZYuIkMbhSaNDlHI81818050 = -525986198;    double uoZYuIkMbhSaNDlHI91262285 = -921117606;    double uoZYuIkMbhSaNDlHI75024256 = -361247877;    double uoZYuIkMbhSaNDlHI37615808 = 70187927;    double uoZYuIkMbhSaNDlHI70746843 = -851797272;    double uoZYuIkMbhSaNDlHI31121811 = -770615783;    double uoZYuIkMbhSaNDlHI54356750 = -556670553;    double uoZYuIkMbhSaNDlHI62557717 = -692774140;    double uoZYuIkMbhSaNDlHI60189276 = 45433327;    double uoZYuIkMbhSaNDlHI51986153 = -499227062;    double uoZYuIkMbhSaNDlHI1658310 = -290096909;    double uoZYuIkMbhSaNDlHI49674102 = -665749645;    double uoZYuIkMbhSaNDlHI35184594 = 80767926;    double uoZYuIkMbhSaNDlHI84434117 = -282308098;    double uoZYuIkMbhSaNDlHI64106104 = -164160953;    double uoZYuIkMbhSaNDlHI80551056 = -111448002;    double uoZYuIkMbhSaNDlHI11182122 = -796090359;    double uoZYuIkMbhSaNDlHI45321070 = -484024645;    double uoZYuIkMbhSaNDlHI15676201 = -263982554;    double uoZYuIkMbhSaNDlHI12392951 = -673373718;    double uoZYuIkMbhSaNDlHI83015034 = -324382690;    double uoZYuIkMbhSaNDlHI58431775 = -915288260;    double uoZYuIkMbhSaNDlHI74393494 = -676587635;    double uoZYuIkMbhSaNDlHI93037054 = -809859182;    double uoZYuIkMbhSaNDlHI74502669 = -639254710;     uoZYuIkMbhSaNDlHI82395454 = uoZYuIkMbhSaNDlHI12252558;     uoZYuIkMbhSaNDlHI12252558 = uoZYuIkMbhSaNDlHI22762476;     uoZYuIkMbhSaNDlHI22762476 = uoZYuIkMbhSaNDlHI71316301;     uoZYuIkMbhSaNDlHI71316301 = uoZYuIkMbhSaNDlHI83508716;     uoZYuIkMbhSaNDlHI83508716 = uoZYuIkMbhSaNDlHI71594370;     uoZYuIkMbhSaNDlHI71594370 = uoZYuIkMbhSaNDlHI8486204;     uoZYuIkMbhSaNDlHI8486204 = uoZYuIkMbhSaNDlHI55755206;     uoZYuIkMbhSaNDlHI55755206 = uoZYuIkMbhSaNDlHI40264897;     uoZYuIkMbhSaNDlHI40264897 = uoZYuIkMbhSaNDlHI40113006;     uoZYuIkMbhSaNDlHI40113006 = uoZYuIkMbhSaNDlHI94798419;     uoZYuIkMbhSaNDlHI94798419 = uoZYuIkMbhSaNDlHI33010759;     uoZYuIkMbhSaNDlHI33010759 = uoZYuIkMbhSaNDlHI93188777;     uoZYuIkMbhSaNDlHI93188777 = uoZYuIkMbhSaNDlHI80870409;     uoZYuIkMbhSaNDlHI80870409 = uoZYuIkMbhSaNDlHI30150400;     uoZYuIkMbhSaNDlHI30150400 = uoZYuIkMbhSaNDlHI92015282;     uoZYuIkMbhSaNDlHI92015282 = uoZYuIkMbhSaNDlHI47012731;     uoZYuIkMbhSaNDlHI47012731 = uoZYuIkMbhSaNDlHI6080038;     uoZYuIkMbhSaNDlHI6080038 = uoZYuIkMbhSaNDlHI86574993;     uoZYuIkMbhSaNDlHI86574993 = uoZYuIkMbhSaNDlHI57127631;     uoZYuIkMbhSaNDlHI57127631 = uoZYuIkMbhSaNDlHI21246;     uoZYuIkMbhSaNDlHI21246 = uoZYuIkMbhSaNDlHI5553031;     uoZYuIkMbhSaNDlHI5553031 = uoZYuIkMbhSaNDlHI79021103;     uoZYuIkMbhSaNDlHI79021103 = uoZYuIkMbhSaNDlHI38426829;     uoZYuIkMbhSaNDlHI38426829 = uoZYuIkMbhSaNDlHI9555042;     uoZYuIkMbhSaNDlHI9555042 = uoZYuIkMbhSaNDlHI36580697;     uoZYuIkMbhSaNDlHI36580697 = uoZYuIkMbhSaNDlHI96276;     uoZYuIkMbhSaNDlHI96276 = uoZYuIkMbhSaNDlHI54659456;     uoZYuIkMbhSaNDlHI54659456 = uoZYuIkMbhSaNDlHI95983786;     uoZYuIkMbhSaNDlHI95983786 = uoZYuIkMbhSaNDlHI367647;     uoZYuIkMbhSaNDlHI367647 = uoZYuIkMbhSaNDlHI29153213;     uoZYuIkMbhSaNDlHI29153213 = uoZYuIkMbhSaNDlHI68490067;     uoZYuIkMbhSaNDlHI68490067 = uoZYuIkMbhSaNDlHI26997587;     uoZYuIkMbhSaNDlHI26997587 = uoZYuIkMbhSaNDlHI11519600;     uoZYuIkMbhSaNDlHI11519600 = uoZYuIkMbhSaNDlHI40948528;     uoZYuIkMbhSaNDlHI40948528 = uoZYuIkMbhSaNDlHI13782511;     uoZYuIkMbhSaNDlHI13782511 = uoZYuIkMbhSaNDlHI95517768;     uoZYuIkMbhSaNDlHI95517768 = uoZYuIkMbhSaNDlHI7082331;     uoZYuIkMbhSaNDlHI7082331 = uoZYuIkMbhSaNDlHI62604220;     uoZYuIkMbhSaNDlHI62604220 = uoZYuIkMbhSaNDlHI83891045;     uoZYuIkMbhSaNDlHI83891045 = uoZYuIkMbhSaNDlHI36841781;     uoZYuIkMbhSaNDlHI36841781 = uoZYuIkMbhSaNDlHI18203195;     uoZYuIkMbhSaNDlHI18203195 = uoZYuIkMbhSaNDlHI14290745;     uoZYuIkMbhSaNDlHI14290745 = uoZYuIkMbhSaNDlHI3996848;     uoZYuIkMbhSaNDlHI3996848 = uoZYuIkMbhSaNDlHI811022;     uoZYuIkMbhSaNDlHI811022 = uoZYuIkMbhSaNDlHI38808198;     uoZYuIkMbhSaNDlHI38808198 = uoZYuIkMbhSaNDlHI5458886;     uoZYuIkMbhSaNDlHI5458886 = uoZYuIkMbhSaNDlHI45739525;     uoZYuIkMbhSaNDlHI45739525 = uoZYuIkMbhSaNDlHI92101739;     uoZYuIkMbhSaNDlHI92101739 = uoZYuIkMbhSaNDlHI35794511;     uoZYuIkMbhSaNDlHI35794511 = uoZYuIkMbhSaNDlHI48381493;     uoZYuIkMbhSaNDlHI48381493 = uoZYuIkMbhSaNDlHI27494903;     uoZYuIkMbhSaNDlHI27494903 = uoZYuIkMbhSaNDlHI18815965;     uoZYuIkMbhSaNDlHI18815965 = uoZYuIkMbhSaNDlHI91812993;     uoZYuIkMbhSaNDlHI91812993 = uoZYuIkMbhSaNDlHI27085483;     uoZYuIkMbhSaNDlHI27085483 = uoZYuIkMbhSaNDlHI76842423;     uoZYuIkMbhSaNDlHI76842423 = uoZYuIkMbhSaNDlHI33231454;     uoZYuIkMbhSaNDlHI33231454 = uoZYuIkMbhSaNDlHI84335646;     uoZYuIkMbhSaNDlHI84335646 = uoZYuIkMbhSaNDlHI61761260;     uoZYuIkMbhSaNDlHI61761260 = uoZYuIkMbhSaNDlHI46928019;     uoZYuIkMbhSaNDlHI46928019 = uoZYuIkMbhSaNDlHI71498095;     uoZYuIkMbhSaNDlHI71498095 = uoZYuIkMbhSaNDlHI53826747;     uoZYuIkMbhSaNDlHI53826747 = uoZYuIkMbhSaNDlHI59771420;     uoZYuIkMbhSaNDlHI59771420 = uoZYuIkMbhSaNDlHI39897251;     uoZYuIkMbhSaNDlHI39897251 = uoZYuIkMbhSaNDlHI10959793;     uoZYuIkMbhSaNDlHI10959793 = uoZYuIkMbhSaNDlHI26308352;     uoZYuIkMbhSaNDlHI26308352 = uoZYuIkMbhSaNDlHI6013172;     uoZYuIkMbhSaNDlHI6013172 = uoZYuIkMbhSaNDlHI81669177;     uoZYuIkMbhSaNDlHI81669177 = uoZYuIkMbhSaNDlHI39921882;     uoZYuIkMbhSaNDlHI39921882 = uoZYuIkMbhSaNDlHI16367890;     uoZYuIkMbhSaNDlHI16367890 = uoZYuIkMbhSaNDlHI96497514;     uoZYuIkMbhSaNDlHI96497514 = uoZYuIkMbhSaNDlHI39930401;     uoZYuIkMbhSaNDlHI39930401 = uoZYuIkMbhSaNDlHI43475818;     uoZYuIkMbhSaNDlHI43475818 = uoZYuIkMbhSaNDlHI2683949;     uoZYuIkMbhSaNDlHI2683949 = uoZYuIkMbhSaNDlHI20285850;     uoZYuIkMbhSaNDlHI20285850 = uoZYuIkMbhSaNDlHI81818050;     uoZYuIkMbhSaNDlHI81818050 = uoZYuIkMbhSaNDlHI91262285;     uoZYuIkMbhSaNDlHI91262285 = uoZYuIkMbhSaNDlHI75024256;     uoZYuIkMbhSaNDlHI75024256 = uoZYuIkMbhSaNDlHI37615808;     uoZYuIkMbhSaNDlHI37615808 = uoZYuIkMbhSaNDlHI70746843;     uoZYuIkMbhSaNDlHI70746843 = uoZYuIkMbhSaNDlHI31121811;     uoZYuIkMbhSaNDlHI31121811 = uoZYuIkMbhSaNDlHI54356750;     uoZYuIkMbhSaNDlHI54356750 = uoZYuIkMbhSaNDlHI62557717;     uoZYuIkMbhSaNDlHI62557717 = uoZYuIkMbhSaNDlHI60189276;     uoZYuIkMbhSaNDlHI60189276 = uoZYuIkMbhSaNDlHI51986153;     uoZYuIkMbhSaNDlHI51986153 = uoZYuIkMbhSaNDlHI1658310;     uoZYuIkMbhSaNDlHI1658310 = uoZYuIkMbhSaNDlHI49674102;     uoZYuIkMbhSaNDlHI49674102 = uoZYuIkMbhSaNDlHI35184594;     uoZYuIkMbhSaNDlHI35184594 = uoZYuIkMbhSaNDlHI84434117;     uoZYuIkMbhSaNDlHI84434117 = uoZYuIkMbhSaNDlHI64106104;     uoZYuIkMbhSaNDlHI64106104 = uoZYuIkMbhSaNDlHI80551056;     uoZYuIkMbhSaNDlHI80551056 = uoZYuIkMbhSaNDlHI11182122;     uoZYuIkMbhSaNDlHI11182122 = uoZYuIkMbhSaNDlHI45321070;     uoZYuIkMbhSaNDlHI45321070 = uoZYuIkMbhSaNDlHI15676201;     uoZYuIkMbhSaNDlHI15676201 = uoZYuIkMbhSaNDlHI12392951;     uoZYuIkMbhSaNDlHI12392951 = uoZYuIkMbhSaNDlHI83015034;     uoZYuIkMbhSaNDlHI83015034 = uoZYuIkMbhSaNDlHI58431775;     uoZYuIkMbhSaNDlHI58431775 = uoZYuIkMbhSaNDlHI74393494;     uoZYuIkMbhSaNDlHI74393494 = uoZYuIkMbhSaNDlHI93037054;     uoZYuIkMbhSaNDlHI93037054 = uoZYuIkMbhSaNDlHI74502669;     uoZYuIkMbhSaNDlHI74502669 = uoZYuIkMbhSaNDlHI82395454;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void XDkcdBWWYlhKQlQV42155796() {     double XmnpvNgZbTruFRPUu88512560 = -751156598;    double XmnpvNgZbTruFRPUu55625837 = -447912990;    double XmnpvNgZbTruFRPUu38804659 = 13161158;    double XmnpvNgZbTruFRPUu98391082 = -357676283;    double XmnpvNgZbTruFRPUu52404204 = 11617084;    double XmnpvNgZbTruFRPUu63011573 = -71534219;    double XmnpvNgZbTruFRPUu68975169 = -214755038;    double XmnpvNgZbTruFRPUu5515097 = -9421732;    double XmnpvNgZbTruFRPUu4907393 = 18765906;    double XmnpvNgZbTruFRPUu81342664 = -831033726;    double XmnpvNgZbTruFRPUu54908709 = 63943934;    double XmnpvNgZbTruFRPUu64833801 = -804127023;    double XmnpvNgZbTruFRPUu79397895 = -925972858;    double XmnpvNgZbTruFRPUu64014086 = -531079982;    double XmnpvNgZbTruFRPUu25892827 = -416593874;    double XmnpvNgZbTruFRPUu33520677 = -784444743;    double XmnpvNgZbTruFRPUu40848735 = -612585777;    double XmnpvNgZbTruFRPUu3133191 = -670119061;    double XmnpvNgZbTruFRPUu48404836 = -438747015;    double XmnpvNgZbTruFRPUu37677796 = -154586610;    double XmnpvNgZbTruFRPUu88574438 = -12510709;    double XmnpvNgZbTruFRPUu92074054 = -128815361;    double XmnpvNgZbTruFRPUu14611896 = -776931826;    double XmnpvNgZbTruFRPUu22419203 = -388331436;    double XmnpvNgZbTruFRPUu53254691 = -979210867;    double XmnpvNgZbTruFRPUu61302372 = 59814000;    double XmnpvNgZbTruFRPUu98556053 = -959959569;    double XmnpvNgZbTruFRPUu67766581 = -760133174;    double XmnpvNgZbTruFRPUu62116042 = -229181726;    double XmnpvNgZbTruFRPUu68983283 = -407491892;    double XmnpvNgZbTruFRPUu21395762 = -656071906;    double XmnpvNgZbTruFRPUu66100689 = -523890110;    double XmnpvNgZbTruFRPUu36708536 = 9245846;    double XmnpvNgZbTruFRPUu6350165 = -57297246;    double XmnpvNgZbTruFRPUu2423180 = -121263123;    double XmnpvNgZbTruFRPUu82508423 = -33372980;    double XmnpvNgZbTruFRPUu76779703 = -419705701;    double XmnpvNgZbTruFRPUu41172543 = -916443473;    double XmnpvNgZbTruFRPUu36214474 = -764108419;    double XmnpvNgZbTruFRPUu71392983 = -994553559;    double XmnpvNgZbTruFRPUu42511005 = -871242425;    double XmnpvNgZbTruFRPUu50353652 = -901179335;    double XmnpvNgZbTruFRPUu92357723 = -380572623;    double XmnpvNgZbTruFRPUu91404587 = -588509401;    double XmnpvNgZbTruFRPUu12763487 = -898112530;    double XmnpvNgZbTruFRPUu50747393 = -739433679;    double XmnpvNgZbTruFRPUu23237460 = -409690432;    double XmnpvNgZbTruFRPUu53696466 = -412522345;    double XmnpvNgZbTruFRPUu1777469 = 12832931;    double XmnpvNgZbTruFRPUu48706163 = -39243157;    double XmnpvNgZbTruFRPUu84082171 = -933087634;    double XmnpvNgZbTruFRPUu38304623 = -153380379;    double XmnpvNgZbTruFRPUu67064422 = -940483237;    double XmnpvNgZbTruFRPUu87781873 = -399378855;    double XmnpvNgZbTruFRPUu53581357 = -623979167;    double XmnpvNgZbTruFRPUu96438506 = -522341237;    double XmnpvNgZbTruFRPUu41013942 = -670981164;    double XmnpvNgZbTruFRPUu16385457 = -598507407;    double XmnpvNgZbTruFRPUu45136391 = -378465416;    double XmnpvNgZbTruFRPUu91101832 = 51803084;    double XmnpvNgZbTruFRPUu64455519 = -111574651;    double XmnpvNgZbTruFRPUu1208589 = -454621865;    double XmnpvNgZbTruFRPUu43399055 = -780240006;    double XmnpvNgZbTruFRPUu35924110 = -573742202;    double XmnpvNgZbTruFRPUu59946902 = -74961821;    double XmnpvNgZbTruFRPUu88808020 = -412165957;    double XmnpvNgZbTruFRPUu28125266 = -713372869;    double XmnpvNgZbTruFRPUu73047731 = -768675612;    double XmnpvNgZbTruFRPUu61590906 = -309816859;    double XmnpvNgZbTruFRPUu43384403 = -283220895;    double XmnpvNgZbTruFRPUu56740974 = -264739042;    double XmnpvNgZbTruFRPUu99676191 = -696142304;    double XmnpvNgZbTruFRPUu66918717 = -906010642;    double XmnpvNgZbTruFRPUu77011852 = -444193457;    double XmnpvNgZbTruFRPUu95166790 = -283344185;    double XmnpvNgZbTruFRPUu38220787 = -111331375;    double XmnpvNgZbTruFRPUu99716330 = -748242739;    double XmnpvNgZbTruFRPUu23207308 = -88422425;    double XmnpvNgZbTruFRPUu9655717 = -490218906;    double XmnpvNgZbTruFRPUu2507299 = -139777188;    double XmnpvNgZbTruFRPUu38064912 = -530495569;    double XmnpvNgZbTruFRPUu44859587 = -447437224;    double XmnpvNgZbTruFRPUu65989113 = -672966105;    double XmnpvNgZbTruFRPUu13409879 = -89938570;    double XmnpvNgZbTruFRPUu84901111 = -474404258;    double XmnpvNgZbTruFRPUu83091139 = -402691527;    double XmnpvNgZbTruFRPUu99036266 = -583406873;    double XmnpvNgZbTruFRPUu48926662 = -591375299;    double XmnpvNgZbTruFRPUu52768807 = -433318080;    double XmnpvNgZbTruFRPUu5984674 = -598921886;    double XmnpvNgZbTruFRPUu41494482 = -362391816;    double XmnpvNgZbTruFRPUu60394246 = -821198295;    double XmnpvNgZbTruFRPUu96036152 = -437978058;    double XmnpvNgZbTruFRPUu45112642 = -715911503;    double XmnpvNgZbTruFRPUu6937464 = -782978908;    double XmnpvNgZbTruFRPUu41302417 = -316620560;    double XmnpvNgZbTruFRPUu6954597 = -20939329;    double XmnpvNgZbTruFRPUu56433613 = -806830421;    double XmnpvNgZbTruFRPUu31457685 = -413547581;    double XmnpvNgZbTruFRPUu23955466 = -751156598;     XmnpvNgZbTruFRPUu88512560 = XmnpvNgZbTruFRPUu55625837;     XmnpvNgZbTruFRPUu55625837 = XmnpvNgZbTruFRPUu38804659;     XmnpvNgZbTruFRPUu38804659 = XmnpvNgZbTruFRPUu98391082;     XmnpvNgZbTruFRPUu98391082 = XmnpvNgZbTruFRPUu52404204;     XmnpvNgZbTruFRPUu52404204 = XmnpvNgZbTruFRPUu63011573;     XmnpvNgZbTruFRPUu63011573 = XmnpvNgZbTruFRPUu68975169;     XmnpvNgZbTruFRPUu68975169 = XmnpvNgZbTruFRPUu5515097;     XmnpvNgZbTruFRPUu5515097 = XmnpvNgZbTruFRPUu4907393;     XmnpvNgZbTruFRPUu4907393 = XmnpvNgZbTruFRPUu81342664;     XmnpvNgZbTruFRPUu81342664 = XmnpvNgZbTruFRPUu54908709;     XmnpvNgZbTruFRPUu54908709 = XmnpvNgZbTruFRPUu64833801;     XmnpvNgZbTruFRPUu64833801 = XmnpvNgZbTruFRPUu79397895;     XmnpvNgZbTruFRPUu79397895 = XmnpvNgZbTruFRPUu64014086;     XmnpvNgZbTruFRPUu64014086 = XmnpvNgZbTruFRPUu25892827;     XmnpvNgZbTruFRPUu25892827 = XmnpvNgZbTruFRPUu33520677;     XmnpvNgZbTruFRPUu33520677 = XmnpvNgZbTruFRPUu40848735;     XmnpvNgZbTruFRPUu40848735 = XmnpvNgZbTruFRPUu3133191;     XmnpvNgZbTruFRPUu3133191 = XmnpvNgZbTruFRPUu48404836;     XmnpvNgZbTruFRPUu48404836 = XmnpvNgZbTruFRPUu37677796;     XmnpvNgZbTruFRPUu37677796 = XmnpvNgZbTruFRPUu88574438;     XmnpvNgZbTruFRPUu88574438 = XmnpvNgZbTruFRPUu92074054;     XmnpvNgZbTruFRPUu92074054 = XmnpvNgZbTruFRPUu14611896;     XmnpvNgZbTruFRPUu14611896 = XmnpvNgZbTruFRPUu22419203;     XmnpvNgZbTruFRPUu22419203 = XmnpvNgZbTruFRPUu53254691;     XmnpvNgZbTruFRPUu53254691 = XmnpvNgZbTruFRPUu61302372;     XmnpvNgZbTruFRPUu61302372 = XmnpvNgZbTruFRPUu98556053;     XmnpvNgZbTruFRPUu98556053 = XmnpvNgZbTruFRPUu67766581;     XmnpvNgZbTruFRPUu67766581 = XmnpvNgZbTruFRPUu62116042;     XmnpvNgZbTruFRPUu62116042 = XmnpvNgZbTruFRPUu68983283;     XmnpvNgZbTruFRPUu68983283 = XmnpvNgZbTruFRPUu21395762;     XmnpvNgZbTruFRPUu21395762 = XmnpvNgZbTruFRPUu66100689;     XmnpvNgZbTruFRPUu66100689 = XmnpvNgZbTruFRPUu36708536;     XmnpvNgZbTruFRPUu36708536 = XmnpvNgZbTruFRPUu6350165;     XmnpvNgZbTruFRPUu6350165 = XmnpvNgZbTruFRPUu2423180;     XmnpvNgZbTruFRPUu2423180 = XmnpvNgZbTruFRPUu82508423;     XmnpvNgZbTruFRPUu82508423 = XmnpvNgZbTruFRPUu76779703;     XmnpvNgZbTruFRPUu76779703 = XmnpvNgZbTruFRPUu41172543;     XmnpvNgZbTruFRPUu41172543 = XmnpvNgZbTruFRPUu36214474;     XmnpvNgZbTruFRPUu36214474 = XmnpvNgZbTruFRPUu71392983;     XmnpvNgZbTruFRPUu71392983 = XmnpvNgZbTruFRPUu42511005;     XmnpvNgZbTruFRPUu42511005 = XmnpvNgZbTruFRPUu50353652;     XmnpvNgZbTruFRPUu50353652 = XmnpvNgZbTruFRPUu92357723;     XmnpvNgZbTruFRPUu92357723 = XmnpvNgZbTruFRPUu91404587;     XmnpvNgZbTruFRPUu91404587 = XmnpvNgZbTruFRPUu12763487;     XmnpvNgZbTruFRPUu12763487 = XmnpvNgZbTruFRPUu50747393;     XmnpvNgZbTruFRPUu50747393 = XmnpvNgZbTruFRPUu23237460;     XmnpvNgZbTruFRPUu23237460 = XmnpvNgZbTruFRPUu53696466;     XmnpvNgZbTruFRPUu53696466 = XmnpvNgZbTruFRPUu1777469;     XmnpvNgZbTruFRPUu1777469 = XmnpvNgZbTruFRPUu48706163;     XmnpvNgZbTruFRPUu48706163 = XmnpvNgZbTruFRPUu84082171;     XmnpvNgZbTruFRPUu84082171 = XmnpvNgZbTruFRPUu38304623;     XmnpvNgZbTruFRPUu38304623 = XmnpvNgZbTruFRPUu67064422;     XmnpvNgZbTruFRPUu67064422 = XmnpvNgZbTruFRPUu87781873;     XmnpvNgZbTruFRPUu87781873 = XmnpvNgZbTruFRPUu53581357;     XmnpvNgZbTruFRPUu53581357 = XmnpvNgZbTruFRPUu96438506;     XmnpvNgZbTruFRPUu96438506 = XmnpvNgZbTruFRPUu41013942;     XmnpvNgZbTruFRPUu41013942 = XmnpvNgZbTruFRPUu16385457;     XmnpvNgZbTruFRPUu16385457 = XmnpvNgZbTruFRPUu45136391;     XmnpvNgZbTruFRPUu45136391 = XmnpvNgZbTruFRPUu91101832;     XmnpvNgZbTruFRPUu91101832 = XmnpvNgZbTruFRPUu64455519;     XmnpvNgZbTruFRPUu64455519 = XmnpvNgZbTruFRPUu1208589;     XmnpvNgZbTruFRPUu1208589 = XmnpvNgZbTruFRPUu43399055;     XmnpvNgZbTruFRPUu43399055 = XmnpvNgZbTruFRPUu35924110;     XmnpvNgZbTruFRPUu35924110 = XmnpvNgZbTruFRPUu59946902;     XmnpvNgZbTruFRPUu59946902 = XmnpvNgZbTruFRPUu88808020;     XmnpvNgZbTruFRPUu88808020 = XmnpvNgZbTruFRPUu28125266;     XmnpvNgZbTruFRPUu28125266 = XmnpvNgZbTruFRPUu73047731;     XmnpvNgZbTruFRPUu73047731 = XmnpvNgZbTruFRPUu61590906;     XmnpvNgZbTruFRPUu61590906 = XmnpvNgZbTruFRPUu43384403;     XmnpvNgZbTruFRPUu43384403 = XmnpvNgZbTruFRPUu56740974;     XmnpvNgZbTruFRPUu56740974 = XmnpvNgZbTruFRPUu99676191;     XmnpvNgZbTruFRPUu99676191 = XmnpvNgZbTruFRPUu66918717;     XmnpvNgZbTruFRPUu66918717 = XmnpvNgZbTruFRPUu77011852;     XmnpvNgZbTruFRPUu77011852 = XmnpvNgZbTruFRPUu95166790;     XmnpvNgZbTruFRPUu95166790 = XmnpvNgZbTruFRPUu38220787;     XmnpvNgZbTruFRPUu38220787 = XmnpvNgZbTruFRPUu99716330;     XmnpvNgZbTruFRPUu99716330 = XmnpvNgZbTruFRPUu23207308;     XmnpvNgZbTruFRPUu23207308 = XmnpvNgZbTruFRPUu9655717;     XmnpvNgZbTruFRPUu9655717 = XmnpvNgZbTruFRPUu2507299;     XmnpvNgZbTruFRPUu2507299 = XmnpvNgZbTruFRPUu38064912;     XmnpvNgZbTruFRPUu38064912 = XmnpvNgZbTruFRPUu44859587;     XmnpvNgZbTruFRPUu44859587 = XmnpvNgZbTruFRPUu65989113;     XmnpvNgZbTruFRPUu65989113 = XmnpvNgZbTruFRPUu13409879;     XmnpvNgZbTruFRPUu13409879 = XmnpvNgZbTruFRPUu84901111;     XmnpvNgZbTruFRPUu84901111 = XmnpvNgZbTruFRPUu83091139;     XmnpvNgZbTruFRPUu83091139 = XmnpvNgZbTruFRPUu99036266;     XmnpvNgZbTruFRPUu99036266 = XmnpvNgZbTruFRPUu48926662;     XmnpvNgZbTruFRPUu48926662 = XmnpvNgZbTruFRPUu52768807;     XmnpvNgZbTruFRPUu52768807 = XmnpvNgZbTruFRPUu5984674;     XmnpvNgZbTruFRPUu5984674 = XmnpvNgZbTruFRPUu41494482;     XmnpvNgZbTruFRPUu41494482 = XmnpvNgZbTruFRPUu60394246;     XmnpvNgZbTruFRPUu60394246 = XmnpvNgZbTruFRPUu96036152;     XmnpvNgZbTruFRPUu96036152 = XmnpvNgZbTruFRPUu45112642;     XmnpvNgZbTruFRPUu45112642 = XmnpvNgZbTruFRPUu6937464;     XmnpvNgZbTruFRPUu6937464 = XmnpvNgZbTruFRPUu41302417;     XmnpvNgZbTruFRPUu41302417 = XmnpvNgZbTruFRPUu6954597;     XmnpvNgZbTruFRPUu6954597 = XmnpvNgZbTruFRPUu56433613;     XmnpvNgZbTruFRPUu56433613 = XmnpvNgZbTruFRPUu31457685;     XmnpvNgZbTruFRPUu31457685 = XmnpvNgZbTruFRPUu23955466;     XmnpvNgZbTruFRPUu23955466 = XmnpvNgZbTruFRPUu88512560;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void EgwnBWXudDTmlnZE9447396() {     double rFJJedTKfpLXhrDIl23971530 = -117433498;    double rFJJedTKfpLXhrDIl68897915 = -328464596;    double rFJJedTKfpLXhrDIl76191136 = -752505917;    double rFJJedTKfpLXhrDIl70614437 = -178827354;    double rFJJedTKfpLXhrDIl8689474 = -93981762;    double rFJJedTKfpLXhrDIl99234444 = -333882115;    double rFJJedTKfpLXhrDIl79632575 = -59480773;    double rFJJedTKfpLXhrDIl67646658 = -753655873;    double rFJJedTKfpLXhrDIl6424998 = -469565501;    double rFJJedTKfpLXhrDIl96245376 = -495720724;    double rFJJedTKfpLXhrDIl93090444 = -838063795;    double rFJJedTKfpLXhrDIl65401949 = -668247795;    double rFJJedTKfpLXhrDIl17632541 = -377985363;    double rFJJedTKfpLXhrDIl17154950 = -507443391;    double rFJJedTKfpLXhrDIl84087342 = -920018889;    double rFJJedTKfpLXhrDIl17061978 = -420256350;    double rFJJedTKfpLXhrDIl96684670 = -39560330;    double rFJJedTKfpLXhrDIl2893383 = -743547129;    double rFJJedTKfpLXhrDIl48955166 = -755310437;    double rFJJedTKfpLXhrDIl31515015 = -581147676;    double rFJJedTKfpLXhrDIl1661302 = -769785398;    double rFJJedTKfpLXhrDIl95495578 = -215451655;    double rFJJedTKfpLXhrDIl85137275 = -414564010;    double rFJJedTKfpLXhrDIl16212148 = -899156040;    double rFJJedTKfpLXhrDIl61272642 = 96803935;    double rFJJedTKfpLXhrDIl72000543 = -728762807;    double rFJJedTKfpLXhrDIl58976627 = -766243709;    double rFJJedTKfpLXhrDIl8495007 = -929905348;    double rFJJedTKfpLXhrDIl8784257 = -803276529;    double rFJJedTKfpLXhrDIl74516693 = -441999062;    double rFJJedTKfpLXhrDIl39978994 = -108052393;    double rFJJedTKfpLXhrDIl3923708 = -296861524;    double rFJJedTKfpLXhrDIl48926546 = -138350132;    double rFJJedTKfpLXhrDIl36098591 = -121991060;    double rFJJedTKfpLXhrDIl68231633 = -72363374;    double rFJJedTKfpLXhrDIl34069267 = -247938285;    double rFJJedTKfpLXhrDIl26191520 = -284401742;    double rFJJedTKfpLXhrDIl92387946 = -899099055;    double rFJJedTKfpLXhrDIl66605036 = -414984695;    double rFJJedTKfpLXhrDIl35664516 = -459816105;    double rFJJedTKfpLXhrDIl75962613 = -640441339;    double rFJJedTKfpLXhrDIl39718778 = -495724626;    double rFJJedTKfpLXhrDIl81741101 = 66246591;    double rFJJedTKfpLXhrDIl31535926 = -92841811;    double rFJJedTKfpLXhrDIl12779945 = -724623691;    double rFJJedTKfpLXhrDIl96708721 = -778357311;    double rFJJedTKfpLXhrDIl42582624 = -893676793;    double rFJJedTKfpLXhrDIl71422778 = -579048938;    double rFJJedTKfpLXhrDIl24670134 = -750903949;    double rFJJedTKfpLXhrDIl73235752 = -530269973;    double rFJJedTKfpLXhrDIl64852895 = -807364192;    double rFJJedTKfpLXhrDIl21790095 = -367032726;    double rFJJedTKfpLXhrDIl35685571 = -696528791;    double rFJJedTKfpLXhrDIl3773720 = -590887355;    double rFJJedTKfpLXhrDIl7786094 = -970469312;    double rFJJedTKfpLXhrDIl28475951 = -901981843;    double rFJJedTKfpLXhrDIl83760640 = -913900586;    double rFJJedTKfpLXhrDIl59978988 = -853349878;    double rFJJedTKfpLXhrDIl9341795 = -175631289;    double rFJJedTKfpLXhrDIl36688930 = -365218955;    double rFJJedTKfpLXhrDIl40257818 = -567638406;    double rFJJedTKfpLXhrDIl71137569 = -129575425;    double rFJJedTKfpLXhrDIl58862401 = -950379344;    double rFJJedTKfpLXhrDIl31908304 = 72433561;    double rFJJedTKfpLXhrDIl56266383 = -287668332;    double rFJJedTKfpLXhrDIl89166736 = -441202272;    double rFJJedTKfpLXhrDIl16475404 = -429897663;    double rFJJedTKfpLXhrDIl81533950 = -155994304;    double rFJJedTKfpLXhrDIl48923316 = -335080017;    double rFJJedTKfpLXhrDIl50018075 = -572080604;    double rFJJedTKfpLXhrDIl90870458 = -35854609;    double rFJJedTKfpLXhrDIl4296724 = -140461276;    double rFJJedTKfpLXhrDIl36288347 = -228562435;    double rFJJedTKfpLXhrDIl13290650 = -195494332;    double rFJJedTKfpLXhrDIl55552402 = -940706337;    double rFJJedTKfpLXhrDIl61942523 = -174060773;    double rFJJedTKfpLXhrDIl13754478 = -181698247;    double rFJJedTKfpLXhrDIl53601349 = -221722199;    double rFJJedTKfpLXhrDIl3432203 = -74532349;    double rFJJedTKfpLXhrDIl64563921 = -124838755;    double rFJJedTKfpLXhrDIl29417920 = -835086015;    double rFJJedTKfpLXhrDIl87553849 = -87194771;    double rFJJedTKfpLXhrDIl83824872 = -79001399;    double rFJJedTKfpLXhrDIl35548504 = -173006556;    double rFJJedTKfpLXhrDIl9663799 = -634634871;    double rFJJedTKfpLXhrDIl18188899 = -741019667;    double rFJJedTKfpLXhrDIl68238136 = -600332733;    double rFJJedTKfpLXhrDIl45152827 = -547462777;    double rFJJedTKfpLXhrDIl28312497 = -151521748;    double rFJJedTKfpLXhrDIl39755683 = -170381531;    double rFJJedTKfpLXhrDIl50308627 = -334037700;    double rFJJedTKfpLXhrDIl66212531 = -431051864;    double rFJJedTKfpLXhrDIl83046152 = -623467766;    double rFJJedTKfpLXhrDIl29916106 = 50234260;    double rFJJedTKfpLXhrDIl95406698 = -892177700;    double rFJJedTKfpLXhrDIl4825044 = -410865915;    double rFJJedTKfpLXhrDIl80856377 = -545345282;    double rFJJedTKfpLXhrDIl49832798 = 93813029;    double rFJJedTKfpLXhrDIl75269543 = -805173480;    double rFJJedTKfpLXhrDIl23613208 = -117433498;     rFJJedTKfpLXhrDIl23971530 = rFJJedTKfpLXhrDIl68897915;     rFJJedTKfpLXhrDIl68897915 = rFJJedTKfpLXhrDIl76191136;     rFJJedTKfpLXhrDIl76191136 = rFJJedTKfpLXhrDIl70614437;     rFJJedTKfpLXhrDIl70614437 = rFJJedTKfpLXhrDIl8689474;     rFJJedTKfpLXhrDIl8689474 = rFJJedTKfpLXhrDIl99234444;     rFJJedTKfpLXhrDIl99234444 = rFJJedTKfpLXhrDIl79632575;     rFJJedTKfpLXhrDIl79632575 = rFJJedTKfpLXhrDIl67646658;     rFJJedTKfpLXhrDIl67646658 = rFJJedTKfpLXhrDIl6424998;     rFJJedTKfpLXhrDIl6424998 = rFJJedTKfpLXhrDIl96245376;     rFJJedTKfpLXhrDIl96245376 = rFJJedTKfpLXhrDIl93090444;     rFJJedTKfpLXhrDIl93090444 = rFJJedTKfpLXhrDIl65401949;     rFJJedTKfpLXhrDIl65401949 = rFJJedTKfpLXhrDIl17632541;     rFJJedTKfpLXhrDIl17632541 = rFJJedTKfpLXhrDIl17154950;     rFJJedTKfpLXhrDIl17154950 = rFJJedTKfpLXhrDIl84087342;     rFJJedTKfpLXhrDIl84087342 = rFJJedTKfpLXhrDIl17061978;     rFJJedTKfpLXhrDIl17061978 = rFJJedTKfpLXhrDIl96684670;     rFJJedTKfpLXhrDIl96684670 = rFJJedTKfpLXhrDIl2893383;     rFJJedTKfpLXhrDIl2893383 = rFJJedTKfpLXhrDIl48955166;     rFJJedTKfpLXhrDIl48955166 = rFJJedTKfpLXhrDIl31515015;     rFJJedTKfpLXhrDIl31515015 = rFJJedTKfpLXhrDIl1661302;     rFJJedTKfpLXhrDIl1661302 = rFJJedTKfpLXhrDIl95495578;     rFJJedTKfpLXhrDIl95495578 = rFJJedTKfpLXhrDIl85137275;     rFJJedTKfpLXhrDIl85137275 = rFJJedTKfpLXhrDIl16212148;     rFJJedTKfpLXhrDIl16212148 = rFJJedTKfpLXhrDIl61272642;     rFJJedTKfpLXhrDIl61272642 = rFJJedTKfpLXhrDIl72000543;     rFJJedTKfpLXhrDIl72000543 = rFJJedTKfpLXhrDIl58976627;     rFJJedTKfpLXhrDIl58976627 = rFJJedTKfpLXhrDIl8495007;     rFJJedTKfpLXhrDIl8495007 = rFJJedTKfpLXhrDIl8784257;     rFJJedTKfpLXhrDIl8784257 = rFJJedTKfpLXhrDIl74516693;     rFJJedTKfpLXhrDIl74516693 = rFJJedTKfpLXhrDIl39978994;     rFJJedTKfpLXhrDIl39978994 = rFJJedTKfpLXhrDIl3923708;     rFJJedTKfpLXhrDIl3923708 = rFJJedTKfpLXhrDIl48926546;     rFJJedTKfpLXhrDIl48926546 = rFJJedTKfpLXhrDIl36098591;     rFJJedTKfpLXhrDIl36098591 = rFJJedTKfpLXhrDIl68231633;     rFJJedTKfpLXhrDIl68231633 = rFJJedTKfpLXhrDIl34069267;     rFJJedTKfpLXhrDIl34069267 = rFJJedTKfpLXhrDIl26191520;     rFJJedTKfpLXhrDIl26191520 = rFJJedTKfpLXhrDIl92387946;     rFJJedTKfpLXhrDIl92387946 = rFJJedTKfpLXhrDIl66605036;     rFJJedTKfpLXhrDIl66605036 = rFJJedTKfpLXhrDIl35664516;     rFJJedTKfpLXhrDIl35664516 = rFJJedTKfpLXhrDIl75962613;     rFJJedTKfpLXhrDIl75962613 = rFJJedTKfpLXhrDIl39718778;     rFJJedTKfpLXhrDIl39718778 = rFJJedTKfpLXhrDIl81741101;     rFJJedTKfpLXhrDIl81741101 = rFJJedTKfpLXhrDIl31535926;     rFJJedTKfpLXhrDIl31535926 = rFJJedTKfpLXhrDIl12779945;     rFJJedTKfpLXhrDIl12779945 = rFJJedTKfpLXhrDIl96708721;     rFJJedTKfpLXhrDIl96708721 = rFJJedTKfpLXhrDIl42582624;     rFJJedTKfpLXhrDIl42582624 = rFJJedTKfpLXhrDIl71422778;     rFJJedTKfpLXhrDIl71422778 = rFJJedTKfpLXhrDIl24670134;     rFJJedTKfpLXhrDIl24670134 = rFJJedTKfpLXhrDIl73235752;     rFJJedTKfpLXhrDIl73235752 = rFJJedTKfpLXhrDIl64852895;     rFJJedTKfpLXhrDIl64852895 = rFJJedTKfpLXhrDIl21790095;     rFJJedTKfpLXhrDIl21790095 = rFJJedTKfpLXhrDIl35685571;     rFJJedTKfpLXhrDIl35685571 = rFJJedTKfpLXhrDIl3773720;     rFJJedTKfpLXhrDIl3773720 = rFJJedTKfpLXhrDIl7786094;     rFJJedTKfpLXhrDIl7786094 = rFJJedTKfpLXhrDIl28475951;     rFJJedTKfpLXhrDIl28475951 = rFJJedTKfpLXhrDIl83760640;     rFJJedTKfpLXhrDIl83760640 = rFJJedTKfpLXhrDIl59978988;     rFJJedTKfpLXhrDIl59978988 = rFJJedTKfpLXhrDIl9341795;     rFJJedTKfpLXhrDIl9341795 = rFJJedTKfpLXhrDIl36688930;     rFJJedTKfpLXhrDIl36688930 = rFJJedTKfpLXhrDIl40257818;     rFJJedTKfpLXhrDIl40257818 = rFJJedTKfpLXhrDIl71137569;     rFJJedTKfpLXhrDIl71137569 = rFJJedTKfpLXhrDIl58862401;     rFJJedTKfpLXhrDIl58862401 = rFJJedTKfpLXhrDIl31908304;     rFJJedTKfpLXhrDIl31908304 = rFJJedTKfpLXhrDIl56266383;     rFJJedTKfpLXhrDIl56266383 = rFJJedTKfpLXhrDIl89166736;     rFJJedTKfpLXhrDIl89166736 = rFJJedTKfpLXhrDIl16475404;     rFJJedTKfpLXhrDIl16475404 = rFJJedTKfpLXhrDIl81533950;     rFJJedTKfpLXhrDIl81533950 = rFJJedTKfpLXhrDIl48923316;     rFJJedTKfpLXhrDIl48923316 = rFJJedTKfpLXhrDIl50018075;     rFJJedTKfpLXhrDIl50018075 = rFJJedTKfpLXhrDIl90870458;     rFJJedTKfpLXhrDIl90870458 = rFJJedTKfpLXhrDIl4296724;     rFJJedTKfpLXhrDIl4296724 = rFJJedTKfpLXhrDIl36288347;     rFJJedTKfpLXhrDIl36288347 = rFJJedTKfpLXhrDIl13290650;     rFJJedTKfpLXhrDIl13290650 = rFJJedTKfpLXhrDIl55552402;     rFJJedTKfpLXhrDIl55552402 = rFJJedTKfpLXhrDIl61942523;     rFJJedTKfpLXhrDIl61942523 = rFJJedTKfpLXhrDIl13754478;     rFJJedTKfpLXhrDIl13754478 = rFJJedTKfpLXhrDIl53601349;     rFJJedTKfpLXhrDIl53601349 = rFJJedTKfpLXhrDIl3432203;     rFJJedTKfpLXhrDIl3432203 = rFJJedTKfpLXhrDIl64563921;     rFJJedTKfpLXhrDIl64563921 = rFJJedTKfpLXhrDIl29417920;     rFJJedTKfpLXhrDIl29417920 = rFJJedTKfpLXhrDIl87553849;     rFJJedTKfpLXhrDIl87553849 = rFJJedTKfpLXhrDIl83824872;     rFJJedTKfpLXhrDIl83824872 = rFJJedTKfpLXhrDIl35548504;     rFJJedTKfpLXhrDIl35548504 = rFJJedTKfpLXhrDIl9663799;     rFJJedTKfpLXhrDIl9663799 = rFJJedTKfpLXhrDIl18188899;     rFJJedTKfpLXhrDIl18188899 = rFJJedTKfpLXhrDIl68238136;     rFJJedTKfpLXhrDIl68238136 = rFJJedTKfpLXhrDIl45152827;     rFJJedTKfpLXhrDIl45152827 = rFJJedTKfpLXhrDIl28312497;     rFJJedTKfpLXhrDIl28312497 = rFJJedTKfpLXhrDIl39755683;     rFJJedTKfpLXhrDIl39755683 = rFJJedTKfpLXhrDIl50308627;     rFJJedTKfpLXhrDIl50308627 = rFJJedTKfpLXhrDIl66212531;     rFJJedTKfpLXhrDIl66212531 = rFJJedTKfpLXhrDIl83046152;     rFJJedTKfpLXhrDIl83046152 = rFJJedTKfpLXhrDIl29916106;     rFJJedTKfpLXhrDIl29916106 = rFJJedTKfpLXhrDIl95406698;     rFJJedTKfpLXhrDIl95406698 = rFJJedTKfpLXhrDIl4825044;     rFJJedTKfpLXhrDIl4825044 = rFJJedTKfpLXhrDIl80856377;     rFJJedTKfpLXhrDIl80856377 = rFJJedTKfpLXhrDIl49832798;     rFJJedTKfpLXhrDIl49832798 = rFJJedTKfpLXhrDIl75269543;     rFJJedTKfpLXhrDIl75269543 = rFJJedTKfpLXhrDIl23613208;     rFJJedTKfpLXhrDIl23613208 = rFJJedTKfpLXhrDIl23971530;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void XOuaJtWdPxNVHuvS24496463() {     double NpnPclLaWZjihqUgz30088636 = -229335387;    double NpnPclLaWZjihqUgz12271195 = -38064377;    double NpnPclLaWZjihqUgz92233319 = -903338838;    double NpnPclLaWZjihqUgz97689217 = -369575984;    double NpnPclLaWZjihqUgz77584962 = -614631633;    double NpnPclLaWZjihqUgz90651647 = -598378350;    double NpnPclLaWZjihqUgz40121542 = -198445750;    double NpnPclLaWZjihqUgz17406549 = 59470214;    double NpnPclLaWZjihqUgz71067493 = -237598166;    double NpnPclLaWZjihqUgz37475036 = -824696201;    double NpnPclLaWZjihqUgz53200734 = -715501382;    double NpnPclLaWZjihqUgz97224992 = -12653996;    double NpnPclLaWZjihqUgz3841660 = -817876452;    double NpnPclLaWZjihqUgz298627 = 60318041;    double NpnPclLaWZjihqUgz79829769 = -510481056;    double NpnPclLaWZjihqUgz58567371 = -393255893;    double NpnPclLaWZjihqUgz90520673 = -656105443;    double NpnPclLaWZjihqUgz99946535 = -409069772;    double NpnPclLaWZjihqUgz10785008 = -61079534;    double NpnPclLaWZjihqUgz12065180 = -938050595;    double NpnPclLaWZjihqUgz90214494 = -625272854;    double NpnPclLaWZjihqUgz82016602 = -123381372;    double NpnPclLaWZjihqUgz20728067 = -161081497;    double NpnPclLaWZjihqUgz204522 = -21461028;    double NpnPclLaWZjihqUgz4972293 = -276392098;    double NpnPclLaWZjihqUgz96722218 = -607418130;    double NpnPclLaWZjihqUgz57436405 = -666905912;    double NpnPclLaWZjihqUgz21602132 = -995834302;    double NpnPclLaWZjihqUgz74916512 = 74340767;    double NpnPclLaWZjihqUgz43132330 = -259469930;    double NpnPclLaWZjihqUgz32221543 = -21373331;    double NpnPclLaWZjihqUgz1534330 = -6308340;    double NpnPclLaWZjihqUgz58637495 = -30963502;    double NpnPclLaWZjihqUgz30929155 = -973532825;    double NpnPclLaWZjihqUgz29706286 = -711096479;    double NpnPclLaWZjihqUgz2795181 = -461964393;    double NpnPclLaWZjihqUgz7453454 = -238037611;    double NpnPclLaWZjihqUgz26478159 = -670605064;    double NpnPclLaWZjihqUgz40215290 = -408908192;    double NpnPclLaWZjihqUgz23166454 = -933255328;    double NpnPclLaWZjihqUgz81631837 = -705715233;    double NpnPclLaWZjihqUgz71869235 = -765866904;    double NpnPclLaWZjihqUgz59808080 = -14557993;    double NpnPclLaWZjihqUgz18943667 = -112184749;    double NpnPclLaWZjihqUgz24732410 = -386521846;    double NpnPclLaWZjihqUgz8647917 = -763573428;    double NpnPclLaWZjihqUgz60361198 = 87547670;    double NpnPclLaWZjihqUgz79379719 = -588944471;    double NpnPclLaWZjihqUgz34345863 = -836640937;    double NpnPclLaWZjihqUgz86147405 = -617280781;    double NpnPclLaWZjihqUgz553574 = -649657863;    double NpnPclLaWZjihqUgz32599814 = -167759046;    double NpnPclLaWZjihqUgz83934028 = -488318379;    double NpnPclLaWZjihqUgz99742600 = -911357500;    double NpnPclLaWZjihqUgz34281969 = -571001095;    double NpnPclLaWZjihqUgz48072033 = -5954016;    double NpnPclLaWZjihqUgz91543128 = -876982880;    double NpnPclLaWZjihqUgz92028797 = -781877811;    double NpnPclLaWZjihqUgz92716925 = 6816113;    double NpnPclLaWZjihqUgz80862743 = 92786497;    double NpnPclLaWZjihqUgz33215242 = -931472438;    double NpnPclLaWZjihqUgz18519410 = -202611448;    double NpnPclLaWZjihqUgz42490037 = 85129447;    double NpnPclLaWZjihqUgz27935163 = -978128237;    double NpnPclLaWZjihqUgz5253493 = -703322871;    double NpnPclLaWZjihqUgz51666405 = -609193042;    double NpnPclLaWZjihqUgz38587497 = -981690495;    double NpnPclLaWZjihqUgz72912505 = -844343627;    double NpnPclLaWZjihqUgz70592340 = -228585480;    double NpnPclLaWZjihqUgz77034589 = 51483337;    double NpnPclLaWZjihqUgz51113917 = -55218282;    double NpnPclLaWZjihqUgz64042514 = -985500379;    double NpnPclLaWZjihqUgz59731246 = 99838419;    double NpnPclLaWZjihqUgz87618554 = -127824206;    double NpnPclLaWZjihqUgz30433343 = -132335362;    double NpnPclLaWZjihqUgz18345260 = -859405950;    double NpnPclLaWZjihqUgz22208523 = -8823379;    double NpnPclLaWZjihqUgz1784401 = 51103252;    double NpnPclLaWZjihqUgz75472111 = -634939182;    double NpnPclLaWZjihqUgz96324375 = -512818671;    double NpnPclLaWZjihqUgz36361021 = -594965800;    double NpnPclLaWZjihqUgz78056686 = 22038558;    double NpnPclLaWZjihqUgz87256268 = -59193365;    double NpnPclLaWZjihqUgz88769107 = -308378453;    double NpnPclLaWZjihqUgz42578757 = -609812067;    double NpnPclLaWZjihqUgz99621728 = -853614285;    double NpnPclLaWZjihqUgz17600301 = -517989961;    double NpnPclLaWZjihqUgz58894895 = -119606002;    double NpnPclLaWZjihqUgz96647186 = -302531731;    double NpnPclLaWZjihqUgz81634252 = -605142464;    double NpnPclLaWZjihqUgz11252053 = -584981513;    double NpnPclLaWZjihqUgz15424657 = -456159801;    double NpnPclLaWZjihqUgz33761234 = -577421178;    double NpnPclLaWZjihqUgz59352546 = -401694689;    double NpnPclLaWZjihqUgz89951211 = 98217110;    double NpnPclLaWZjihqUgz63112427 = -403103785;    double NpnPclLaWZjihqUgz29379199 = -750996352;    double NpnPclLaWZjihqUgz31872917 = -36429756;    double NpnPclLaWZjihqUgz13690174 = -408861879;    double NpnPclLaWZjihqUgz73066005 = -229335387;     NpnPclLaWZjihqUgz30088636 = NpnPclLaWZjihqUgz12271195;     NpnPclLaWZjihqUgz12271195 = NpnPclLaWZjihqUgz92233319;     NpnPclLaWZjihqUgz92233319 = NpnPclLaWZjihqUgz97689217;     NpnPclLaWZjihqUgz97689217 = NpnPclLaWZjihqUgz77584962;     NpnPclLaWZjihqUgz77584962 = NpnPclLaWZjihqUgz90651647;     NpnPclLaWZjihqUgz90651647 = NpnPclLaWZjihqUgz40121542;     NpnPclLaWZjihqUgz40121542 = NpnPclLaWZjihqUgz17406549;     NpnPclLaWZjihqUgz17406549 = NpnPclLaWZjihqUgz71067493;     NpnPclLaWZjihqUgz71067493 = NpnPclLaWZjihqUgz37475036;     NpnPclLaWZjihqUgz37475036 = NpnPclLaWZjihqUgz53200734;     NpnPclLaWZjihqUgz53200734 = NpnPclLaWZjihqUgz97224992;     NpnPclLaWZjihqUgz97224992 = NpnPclLaWZjihqUgz3841660;     NpnPclLaWZjihqUgz3841660 = NpnPclLaWZjihqUgz298627;     NpnPclLaWZjihqUgz298627 = NpnPclLaWZjihqUgz79829769;     NpnPclLaWZjihqUgz79829769 = NpnPclLaWZjihqUgz58567371;     NpnPclLaWZjihqUgz58567371 = NpnPclLaWZjihqUgz90520673;     NpnPclLaWZjihqUgz90520673 = NpnPclLaWZjihqUgz99946535;     NpnPclLaWZjihqUgz99946535 = NpnPclLaWZjihqUgz10785008;     NpnPclLaWZjihqUgz10785008 = NpnPclLaWZjihqUgz12065180;     NpnPclLaWZjihqUgz12065180 = NpnPclLaWZjihqUgz90214494;     NpnPclLaWZjihqUgz90214494 = NpnPclLaWZjihqUgz82016602;     NpnPclLaWZjihqUgz82016602 = NpnPclLaWZjihqUgz20728067;     NpnPclLaWZjihqUgz20728067 = NpnPclLaWZjihqUgz204522;     NpnPclLaWZjihqUgz204522 = NpnPclLaWZjihqUgz4972293;     NpnPclLaWZjihqUgz4972293 = NpnPclLaWZjihqUgz96722218;     NpnPclLaWZjihqUgz96722218 = NpnPclLaWZjihqUgz57436405;     NpnPclLaWZjihqUgz57436405 = NpnPclLaWZjihqUgz21602132;     NpnPclLaWZjihqUgz21602132 = NpnPclLaWZjihqUgz74916512;     NpnPclLaWZjihqUgz74916512 = NpnPclLaWZjihqUgz43132330;     NpnPclLaWZjihqUgz43132330 = NpnPclLaWZjihqUgz32221543;     NpnPclLaWZjihqUgz32221543 = NpnPclLaWZjihqUgz1534330;     NpnPclLaWZjihqUgz1534330 = NpnPclLaWZjihqUgz58637495;     NpnPclLaWZjihqUgz58637495 = NpnPclLaWZjihqUgz30929155;     NpnPclLaWZjihqUgz30929155 = NpnPclLaWZjihqUgz29706286;     NpnPclLaWZjihqUgz29706286 = NpnPclLaWZjihqUgz2795181;     NpnPclLaWZjihqUgz2795181 = NpnPclLaWZjihqUgz7453454;     NpnPclLaWZjihqUgz7453454 = NpnPclLaWZjihqUgz26478159;     NpnPclLaWZjihqUgz26478159 = NpnPclLaWZjihqUgz40215290;     NpnPclLaWZjihqUgz40215290 = NpnPclLaWZjihqUgz23166454;     NpnPclLaWZjihqUgz23166454 = NpnPclLaWZjihqUgz81631837;     NpnPclLaWZjihqUgz81631837 = NpnPclLaWZjihqUgz71869235;     NpnPclLaWZjihqUgz71869235 = NpnPclLaWZjihqUgz59808080;     NpnPclLaWZjihqUgz59808080 = NpnPclLaWZjihqUgz18943667;     NpnPclLaWZjihqUgz18943667 = NpnPclLaWZjihqUgz24732410;     NpnPclLaWZjihqUgz24732410 = NpnPclLaWZjihqUgz8647917;     NpnPclLaWZjihqUgz8647917 = NpnPclLaWZjihqUgz60361198;     NpnPclLaWZjihqUgz60361198 = NpnPclLaWZjihqUgz79379719;     NpnPclLaWZjihqUgz79379719 = NpnPclLaWZjihqUgz34345863;     NpnPclLaWZjihqUgz34345863 = NpnPclLaWZjihqUgz86147405;     NpnPclLaWZjihqUgz86147405 = NpnPclLaWZjihqUgz553574;     NpnPclLaWZjihqUgz553574 = NpnPclLaWZjihqUgz32599814;     NpnPclLaWZjihqUgz32599814 = NpnPclLaWZjihqUgz83934028;     NpnPclLaWZjihqUgz83934028 = NpnPclLaWZjihqUgz99742600;     NpnPclLaWZjihqUgz99742600 = NpnPclLaWZjihqUgz34281969;     NpnPclLaWZjihqUgz34281969 = NpnPclLaWZjihqUgz48072033;     NpnPclLaWZjihqUgz48072033 = NpnPclLaWZjihqUgz91543128;     NpnPclLaWZjihqUgz91543128 = NpnPclLaWZjihqUgz92028797;     NpnPclLaWZjihqUgz92028797 = NpnPclLaWZjihqUgz92716925;     NpnPclLaWZjihqUgz92716925 = NpnPclLaWZjihqUgz80862743;     NpnPclLaWZjihqUgz80862743 = NpnPclLaWZjihqUgz33215242;     NpnPclLaWZjihqUgz33215242 = NpnPclLaWZjihqUgz18519410;     NpnPclLaWZjihqUgz18519410 = NpnPclLaWZjihqUgz42490037;     NpnPclLaWZjihqUgz42490037 = NpnPclLaWZjihqUgz27935163;     NpnPclLaWZjihqUgz27935163 = NpnPclLaWZjihqUgz5253493;     NpnPclLaWZjihqUgz5253493 = NpnPclLaWZjihqUgz51666405;     NpnPclLaWZjihqUgz51666405 = NpnPclLaWZjihqUgz38587497;     NpnPclLaWZjihqUgz38587497 = NpnPclLaWZjihqUgz72912505;     NpnPclLaWZjihqUgz72912505 = NpnPclLaWZjihqUgz70592340;     NpnPclLaWZjihqUgz70592340 = NpnPclLaWZjihqUgz77034589;     NpnPclLaWZjihqUgz77034589 = NpnPclLaWZjihqUgz51113917;     NpnPclLaWZjihqUgz51113917 = NpnPclLaWZjihqUgz64042514;     NpnPclLaWZjihqUgz64042514 = NpnPclLaWZjihqUgz59731246;     NpnPclLaWZjihqUgz59731246 = NpnPclLaWZjihqUgz87618554;     NpnPclLaWZjihqUgz87618554 = NpnPclLaWZjihqUgz30433343;     NpnPclLaWZjihqUgz30433343 = NpnPclLaWZjihqUgz18345260;     NpnPclLaWZjihqUgz18345260 = NpnPclLaWZjihqUgz22208523;     NpnPclLaWZjihqUgz22208523 = NpnPclLaWZjihqUgz1784401;     NpnPclLaWZjihqUgz1784401 = NpnPclLaWZjihqUgz75472111;     NpnPclLaWZjihqUgz75472111 = NpnPclLaWZjihqUgz96324375;     NpnPclLaWZjihqUgz96324375 = NpnPclLaWZjihqUgz36361021;     NpnPclLaWZjihqUgz36361021 = NpnPclLaWZjihqUgz78056686;     NpnPclLaWZjihqUgz78056686 = NpnPclLaWZjihqUgz87256268;     NpnPclLaWZjihqUgz87256268 = NpnPclLaWZjihqUgz88769107;     NpnPclLaWZjihqUgz88769107 = NpnPclLaWZjihqUgz42578757;     NpnPclLaWZjihqUgz42578757 = NpnPclLaWZjihqUgz99621728;     NpnPclLaWZjihqUgz99621728 = NpnPclLaWZjihqUgz17600301;     NpnPclLaWZjihqUgz17600301 = NpnPclLaWZjihqUgz58894895;     NpnPclLaWZjihqUgz58894895 = NpnPclLaWZjihqUgz96647186;     NpnPclLaWZjihqUgz96647186 = NpnPclLaWZjihqUgz81634252;     NpnPclLaWZjihqUgz81634252 = NpnPclLaWZjihqUgz11252053;     NpnPclLaWZjihqUgz11252053 = NpnPclLaWZjihqUgz15424657;     NpnPclLaWZjihqUgz15424657 = NpnPclLaWZjihqUgz33761234;     NpnPclLaWZjihqUgz33761234 = NpnPclLaWZjihqUgz59352546;     NpnPclLaWZjihqUgz59352546 = NpnPclLaWZjihqUgz89951211;     NpnPclLaWZjihqUgz89951211 = NpnPclLaWZjihqUgz63112427;     NpnPclLaWZjihqUgz63112427 = NpnPclLaWZjihqUgz29379199;     NpnPclLaWZjihqUgz29379199 = NpnPclLaWZjihqUgz31872917;     NpnPclLaWZjihqUgz31872917 = NpnPclLaWZjihqUgz13690174;     NpnPclLaWZjihqUgz13690174 = NpnPclLaWZjihqUgz73066005;     NpnPclLaWZjihqUgz73066005 = NpnPclLaWZjihqUgz30088636;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZWuhhORPkEhDNcjQ91788062() {     double tSIaoOKfVNMJLIVVL65547604 = -695612287;    double tSIaoOKfVNMJLIVVL25543273 = 81384017;    double tSIaoOKfVNMJLIVVL29619796 = -569005913;    double tSIaoOKfVNMJLIVVL69912572 = -190727056;    double tSIaoOKfVNMJLIVVL33870232 = -720230478;    double tSIaoOKfVNMJLIVVL26874519 = -860726245;    double tSIaoOKfVNMJLIVVL50778948 = -43171484;    double tSIaoOKfVNMJLIVVL79538110 = -684763926;    double tSIaoOKfVNMJLIVVL72585098 = -725929573;    double tSIaoOKfVNMJLIVVL52377748 = -489383199;    double tSIaoOKfVNMJLIVVL91382469 = -517509111;    double tSIaoOKfVNMJLIVVL97793140 = -976774768;    double tSIaoOKfVNMJLIVVL42076305 = -269888957;    double tSIaoOKfVNMJLIVVL53439489 = 83954632;    double tSIaoOKfVNMJLIVVL38024286 = 86093929;    double tSIaoOKfVNMJLIVVL42108672 = -29067500;    double tSIaoOKfVNMJLIVVL46356609 = -83079995;    double tSIaoOKfVNMJLIVVL99706727 = -482497840;    double tSIaoOKfVNMJLIVVL11335338 = -377642955;    double tSIaoOKfVNMJLIVVL5902398 = -264611661;    double tSIaoOKfVNMJLIVVL3301358 = -282547542;    double tSIaoOKfVNMJLIVVL85438127 = -210017666;    double tSIaoOKfVNMJLIVVL91253446 = -898713681;    double tSIaoOKfVNMJLIVVL93997466 = -532285632;    double tSIaoOKfVNMJLIVVL12990244 = -300377297;    double tSIaoOKfVNMJLIVVL7420391 = -295994937;    double tSIaoOKfVNMJLIVVL17856980 = -473190053;    double tSIaoOKfVNMJLIVVL62330556 = -65606476;    double tSIaoOKfVNMJLIVVL21584727 = -499754036;    double tSIaoOKfVNMJLIVVL48665741 = -293977100;    double tSIaoOKfVNMJLIVVL50804775 = -573353818;    double tSIaoOKfVNMJLIVVL39357348 = -879279754;    double tSIaoOKfVNMJLIVVL70855505 = -178559479;    double tSIaoOKfVNMJLIVVL60677581 = 61773361;    double tSIaoOKfVNMJLIVVL95514739 = -662196730;    double tSIaoOKfVNMJLIVVL54356024 = -676529699;    double tSIaoOKfVNMJLIVVL56865270 = -102733652;    double tSIaoOKfVNMJLIVVL77693562 = -653260645;    double tSIaoOKfVNMJLIVVL70605851 = -59784468;    double tSIaoOKfVNMJLIVVL87437985 = -398517874;    double tSIaoOKfVNMJLIVVL15083445 = -474914148;    double tSIaoOKfVNMJLIVVL61234362 = -360412195;    double tSIaoOKfVNMJLIVVL49191458 = -667738779;    double tSIaoOKfVNMJLIVVL59075004 = -716517159;    double tSIaoOKfVNMJLIVVL24748869 = -213033007;    double tSIaoOKfVNMJLIVVL54609245 = -802497060;    double tSIaoOKfVNMJLIVVL79706361 = -396438691;    double tSIaoOKfVNMJLIVVL97106030 = -755471064;    double tSIaoOKfVNMJLIVVL57238529 = -500377818;    double tSIaoOKfVNMJLIVVL10676995 = -8307597;    double tSIaoOKfVNMJLIVVL81324297 = -523934420;    double tSIaoOKfVNMJLIVVL16085287 = -381411392;    double tSIaoOKfVNMJLIVVL52555177 = -244363933;    double tSIaoOKfVNMJLIVVL15734447 = -2866000;    double tSIaoOKfVNMJLIVVL88486704 = -917491241;    double tSIaoOKfVNMJLIVVL80109477 = -385594621;    double tSIaoOKfVNMJLIVVL34289827 = -19902303;    double tSIaoOKfVNMJLIVVL35622330 = 63279718;    double tSIaoOKfVNMJLIVVL56922329 = -890349759;    double tSIaoOKfVNMJLIVVL26449841 = -324235542;    double tSIaoOKfVNMJLIVVL9017540 = -287536192;    double tSIaoOKfVNMJLIVVL88448391 = -977565008;    double tSIaoOKfVNMJLIVVL57953383 = -85009891;    double tSIaoOKfVNMJLIVVL23919357 = -331952474;    double tSIaoOKfVNMJLIVVL1572974 = -916029382;    double tSIaoOKfVNMJLIVVL52025122 = -638229357;    double tSIaoOKfVNMJLIVVL26937635 = -698215289;    double tSIaoOKfVNMJLIVVL81398723 = -231662319;    double tSIaoOKfVNMJLIVVL57924750 = -253848638;    double tSIaoOKfVNMJLIVVL83668261 = -237376372;    double tSIaoOKfVNMJLIVVL85243401 = -926333849;    double tSIaoOKfVNMJLIVVL68663047 = -429819351;    double tSIaoOKfVNMJLIVVL29100876 = -322713373;    double tSIaoOKfVNMJLIVVL23897352 = -979125082;    double tSIaoOKfVNMJLIVVL90818953 = -789697514;    double tSIaoOKfVNMJLIVVL42066996 = -922135348;    double tSIaoOKfVNMJLIVVL36246669 = -542278887;    double tSIaoOKfVNMJLIVVL32178442 = -82196522;    double tSIaoOKfVNMJLIVVL69248598 = -219252625;    double tSIaoOKfVNMJLIVVL58380998 = -497880237;    double tSIaoOKfVNMJLIVVL27714029 = -899556247;    double tSIaoOKfVNMJLIVVL20750949 = -717718989;    double tSIaoOKfVNMJLIVVL5092028 = -565228659;    double tSIaoOKfVNMJLIVVL10907733 = -391446440;    double tSIaoOKfVNMJLIVVL67341444 = -770042680;    double tSIaoOKfVNMJLIVVL34719488 = -91942426;    double tSIaoOKfVNMJLIVVL86802171 = -534915822;    double tSIaoOKfVNMJLIVVL55121059 = -75693480;    double tSIaoOKfVNMJLIVVL72190876 = -20735399;    double tSIaoOKfVNMJLIVVL15405262 = -176602109;    double tSIaoOKfVNMJLIVVL20066198 = -556627397;    double tSIaoOKfVNMJLIVVL21242941 = -66013370;    double tSIaoOKfVNMJLIVVL20771234 = -762910886;    double tSIaoOKfVNMJLIVVL44156010 = -735548927;    double tSIaoOKfVNMJLIVVL78420446 = -10981682;    double tSIaoOKfVNMJLIVVL26635053 = -497349140;    double tSIaoOKfVNMJLIVVL3280979 = -175402305;    double tSIaoOKfVNMJLIVVL25272101 = -235786306;    double tSIaoOKfVNMJLIVVL57502031 = -800487778;    double tSIaoOKfVNMJLIVVL72723747 = -695612287;     tSIaoOKfVNMJLIVVL65547604 = tSIaoOKfVNMJLIVVL25543273;     tSIaoOKfVNMJLIVVL25543273 = tSIaoOKfVNMJLIVVL29619796;     tSIaoOKfVNMJLIVVL29619796 = tSIaoOKfVNMJLIVVL69912572;     tSIaoOKfVNMJLIVVL69912572 = tSIaoOKfVNMJLIVVL33870232;     tSIaoOKfVNMJLIVVL33870232 = tSIaoOKfVNMJLIVVL26874519;     tSIaoOKfVNMJLIVVL26874519 = tSIaoOKfVNMJLIVVL50778948;     tSIaoOKfVNMJLIVVL50778948 = tSIaoOKfVNMJLIVVL79538110;     tSIaoOKfVNMJLIVVL79538110 = tSIaoOKfVNMJLIVVL72585098;     tSIaoOKfVNMJLIVVL72585098 = tSIaoOKfVNMJLIVVL52377748;     tSIaoOKfVNMJLIVVL52377748 = tSIaoOKfVNMJLIVVL91382469;     tSIaoOKfVNMJLIVVL91382469 = tSIaoOKfVNMJLIVVL97793140;     tSIaoOKfVNMJLIVVL97793140 = tSIaoOKfVNMJLIVVL42076305;     tSIaoOKfVNMJLIVVL42076305 = tSIaoOKfVNMJLIVVL53439489;     tSIaoOKfVNMJLIVVL53439489 = tSIaoOKfVNMJLIVVL38024286;     tSIaoOKfVNMJLIVVL38024286 = tSIaoOKfVNMJLIVVL42108672;     tSIaoOKfVNMJLIVVL42108672 = tSIaoOKfVNMJLIVVL46356609;     tSIaoOKfVNMJLIVVL46356609 = tSIaoOKfVNMJLIVVL99706727;     tSIaoOKfVNMJLIVVL99706727 = tSIaoOKfVNMJLIVVL11335338;     tSIaoOKfVNMJLIVVL11335338 = tSIaoOKfVNMJLIVVL5902398;     tSIaoOKfVNMJLIVVL5902398 = tSIaoOKfVNMJLIVVL3301358;     tSIaoOKfVNMJLIVVL3301358 = tSIaoOKfVNMJLIVVL85438127;     tSIaoOKfVNMJLIVVL85438127 = tSIaoOKfVNMJLIVVL91253446;     tSIaoOKfVNMJLIVVL91253446 = tSIaoOKfVNMJLIVVL93997466;     tSIaoOKfVNMJLIVVL93997466 = tSIaoOKfVNMJLIVVL12990244;     tSIaoOKfVNMJLIVVL12990244 = tSIaoOKfVNMJLIVVL7420391;     tSIaoOKfVNMJLIVVL7420391 = tSIaoOKfVNMJLIVVL17856980;     tSIaoOKfVNMJLIVVL17856980 = tSIaoOKfVNMJLIVVL62330556;     tSIaoOKfVNMJLIVVL62330556 = tSIaoOKfVNMJLIVVL21584727;     tSIaoOKfVNMJLIVVL21584727 = tSIaoOKfVNMJLIVVL48665741;     tSIaoOKfVNMJLIVVL48665741 = tSIaoOKfVNMJLIVVL50804775;     tSIaoOKfVNMJLIVVL50804775 = tSIaoOKfVNMJLIVVL39357348;     tSIaoOKfVNMJLIVVL39357348 = tSIaoOKfVNMJLIVVL70855505;     tSIaoOKfVNMJLIVVL70855505 = tSIaoOKfVNMJLIVVL60677581;     tSIaoOKfVNMJLIVVL60677581 = tSIaoOKfVNMJLIVVL95514739;     tSIaoOKfVNMJLIVVL95514739 = tSIaoOKfVNMJLIVVL54356024;     tSIaoOKfVNMJLIVVL54356024 = tSIaoOKfVNMJLIVVL56865270;     tSIaoOKfVNMJLIVVL56865270 = tSIaoOKfVNMJLIVVL77693562;     tSIaoOKfVNMJLIVVL77693562 = tSIaoOKfVNMJLIVVL70605851;     tSIaoOKfVNMJLIVVL70605851 = tSIaoOKfVNMJLIVVL87437985;     tSIaoOKfVNMJLIVVL87437985 = tSIaoOKfVNMJLIVVL15083445;     tSIaoOKfVNMJLIVVL15083445 = tSIaoOKfVNMJLIVVL61234362;     tSIaoOKfVNMJLIVVL61234362 = tSIaoOKfVNMJLIVVL49191458;     tSIaoOKfVNMJLIVVL49191458 = tSIaoOKfVNMJLIVVL59075004;     tSIaoOKfVNMJLIVVL59075004 = tSIaoOKfVNMJLIVVL24748869;     tSIaoOKfVNMJLIVVL24748869 = tSIaoOKfVNMJLIVVL54609245;     tSIaoOKfVNMJLIVVL54609245 = tSIaoOKfVNMJLIVVL79706361;     tSIaoOKfVNMJLIVVL79706361 = tSIaoOKfVNMJLIVVL97106030;     tSIaoOKfVNMJLIVVL97106030 = tSIaoOKfVNMJLIVVL57238529;     tSIaoOKfVNMJLIVVL57238529 = tSIaoOKfVNMJLIVVL10676995;     tSIaoOKfVNMJLIVVL10676995 = tSIaoOKfVNMJLIVVL81324297;     tSIaoOKfVNMJLIVVL81324297 = tSIaoOKfVNMJLIVVL16085287;     tSIaoOKfVNMJLIVVL16085287 = tSIaoOKfVNMJLIVVL52555177;     tSIaoOKfVNMJLIVVL52555177 = tSIaoOKfVNMJLIVVL15734447;     tSIaoOKfVNMJLIVVL15734447 = tSIaoOKfVNMJLIVVL88486704;     tSIaoOKfVNMJLIVVL88486704 = tSIaoOKfVNMJLIVVL80109477;     tSIaoOKfVNMJLIVVL80109477 = tSIaoOKfVNMJLIVVL34289827;     tSIaoOKfVNMJLIVVL34289827 = tSIaoOKfVNMJLIVVL35622330;     tSIaoOKfVNMJLIVVL35622330 = tSIaoOKfVNMJLIVVL56922329;     tSIaoOKfVNMJLIVVL56922329 = tSIaoOKfVNMJLIVVL26449841;     tSIaoOKfVNMJLIVVL26449841 = tSIaoOKfVNMJLIVVL9017540;     tSIaoOKfVNMJLIVVL9017540 = tSIaoOKfVNMJLIVVL88448391;     tSIaoOKfVNMJLIVVL88448391 = tSIaoOKfVNMJLIVVL57953383;     tSIaoOKfVNMJLIVVL57953383 = tSIaoOKfVNMJLIVVL23919357;     tSIaoOKfVNMJLIVVL23919357 = tSIaoOKfVNMJLIVVL1572974;     tSIaoOKfVNMJLIVVL1572974 = tSIaoOKfVNMJLIVVL52025122;     tSIaoOKfVNMJLIVVL52025122 = tSIaoOKfVNMJLIVVL26937635;     tSIaoOKfVNMJLIVVL26937635 = tSIaoOKfVNMJLIVVL81398723;     tSIaoOKfVNMJLIVVL81398723 = tSIaoOKfVNMJLIVVL57924750;     tSIaoOKfVNMJLIVVL57924750 = tSIaoOKfVNMJLIVVL83668261;     tSIaoOKfVNMJLIVVL83668261 = tSIaoOKfVNMJLIVVL85243401;     tSIaoOKfVNMJLIVVL85243401 = tSIaoOKfVNMJLIVVL68663047;     tSIaoOKfVNMJLIVVL68663047 = tSIaoOKfVNMJLIVVL29100876;     tSIaoOKfVNMJLIVVL29100876 = tSIaoOKfVNMJLIVVL23897352;     tSIaoOKfVNMJLIVVL23897352 = tSIaoOKfVNMJLIVVL90818953;     tSIaoOKfVNMJLIVVL90818953 = tSIaoOKfVNMJLIVVL42066996;     tSIaoOKfVNMJLIVVL42066996 = tSIaoOKfVNMJLIVVL36246669;     tSIaoOKfVNMJLIVVL36246669 = tSIaoOKfVNMJLIVVL32178442;     tSIaoOKfVNMJLIVVL32178442 = tSIaoOKfVNMJLIVVL69248598;     tSIaoOKfVNMJLIVVL69248598 = tSIaoOKfVNMJLIVVL58380998;     tSIaoOKfVNMJLIVVL58380998 = tSIaoOKfVNMJLIVVL27714029;     tSIaoOKfVNMJLIVVL27714029 = tSIaoOKfVNMJLIVVL20750949;     tSIaoOKfVNMJLIVVL20750949 = tSIaoOKfVNMJLIVVL5092028;     tSIaoOKfVNMJLIVVL5092028 = tSIaoOKfVNMJLIVVL10907733;     tSIaoOKfVNMJLIVVL10907733 = tSIaoOKfVNMJLIVVL67341444;     tSIaoOKfVNMJLIVVL67341444 = tSIaoOKfVNMJLIVVL34719488;     tSIaoOKfVNMJLIVVL34719488 = tSIaoOKfVNMJLIVVL86802171;     tSIaoOKfVNMJLIVVL86802171 = tSIaoOKfVNMJLIVVL55121059;     tSIaoOKfVNMJLIVVL55121059 = tSIaoOKfVNMJLIVVL72190876;     tSIaoOKfVNMJLIVVL72190876 = tSIaoOKfVNMJLIVVL15405262;     tSIaoOKfVNMJLIVVL15405262 = tSIaoOKfVNMJLIVVL20066198;     tSIaoOKfVNMJLIVVL20066198 = tSIaoOKfVNMJLIVVL21242941;     tSIaoOKfVNMJLIVVL21242941 = tSIaoOKfVNMJLIVVL20771234;     tSIaoOKfVNMJLIVVL20771234 = tSIaoOKfVNMJLIVVL44156010;     tSIaoOKfVNMJLIVVL44156010 = tSIaoOKfVNMJLIVVL78420446;     tSIaoOKfVNMJLIVVL78420446 = tSIaoOKfVNMJLIVVL26635053;     tSIaoOKfVNMJLIVVL26635053 = tSIaoOKfVNMJLIVVL3280979;     tSIaoOKfVNMJLIVVL3280979 = tSIaoOKfVNMJLIVVL25272101;     tSIaoOKfVNMJLIVVL25272101 = tSIaoOKfVNMJLIVVL57502031;     tSIaoOKfVNMJLIVVL57502031 = tSIaoOKfVNMJLIVVL72723747;     tSIaoOKfVNMJLIVVL72723747 = tSIaoOKfVNMJLIVVL65547604;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void AbDNUHpfPWdvewNK6837131() {     double uTYOEpFnucCZkjDwB71664711 = -807514175;    double uTYOEpFnucCZkjDwB68916552 = -728215764;    double uTYOEpFnucCZkjDwB45661980 = -719838834;    double uTYOEpFnucCZkjDwB96987353 = -381475686;    double uTYOEpFnucCZkjDwB2765720 = -140880350;    double uTYOEpFnucCZkjDwB18291722 = -25222480;    double uTYOEpFnucCZkjDwB11267914 = -182136461;    double uTYOEpFnucCZkjDwB29298001 = -971637839;    double uTYOEpFnucCZkjDwB37227594 = -493962239;    double uTYOEpFnucCZkjDwB93607407 = -818358677;    double uTYOEpFnucCZkjDwB51492759 = -394946698;    double uTYOEpFnucCZkjDwB29616183 = -321180970;    double uTYOEpFnucCZkjDwB28285424 = -709780046;    double uTYOEpFnucCZkjDwB36583166 = -448283936;    double uTYOEpFnucCZkjDwB33766713 = -604368237;    double uTYOEpFnucCZkjDwB83614066 = -2067043;    double uTYOEpFnucCZkjDwB40192612 = -699625108;    double uTYOEpFnucCZkjDwB96759880 = -148020483;    double uTYOEpFnucCZkjDwB73165179 = -783412052;    double uTYOEpFnucCZkjDwB86452562 = -621514580;    double uTYOEpFnucCZkjDwB91854550 = -138034998;    double uTYOEpFnucCZkjDwB71959151 = -117947382;    double uTYOEpFnucCZkjDwB26844239 = -645231168;    double uTYOEpFnucCZkjDwB77989840 = -754590620;    double uTYOEpFnucCZkjDwB56689894 = -673573330;    double uTYOEpFnucCZkjDwB32142066 = -174650260;    double uTYOEpFnucCZkjDwB16316758 = -373852256;    double uTYOEpFnucCZkjDwB75437682 = -131535430;    double uTYOEpFnucCZkjDwB87716982 = -722136740;    double uTYOEpFnucCZkjDwB17281378 = -111447968;    double uTYOEpFnucCZkjDwB43047324 = -486674756;    double uTYOEpFnucCZkjDwB36967969 = -588726570;    double uTYOEpFnucCZkjDwB80566454 = -71172849;    double uTYOEpFnucCZkjDwB55508145 = -789768404;    double uTYOEpFnucCZkjDwB56989391 = -200929836;    double uTYOEpFnucCZkjDwB23081938 = -890555807;    double uTYOEpFnucCZkjDwB38127205 = -56369521;    double uTYOEpFnucCZkjDwB11783775 = -424766655;    double uTYOEpFnucCZkjDwB44216105 = -53707965;    double uTYOEpFnucCZkjDwB74939923 = -871957096;    double uTYOEpFnucCZkjDwB20752669 = -540188042;    double uTYOEpFnucCZkjDwB93384819 = -630554473;    double uTYOEpFnucCZkjDwB27258437 = -748543363;    double uTYOEpFnucCZkjDwB46482745 = -735860098;    double uTYOEpFnucCZkjDwB36701334 = -974931162;    double uTYOEpFnucCZkjDwB66548440 = -787713176;    double uTYOEpFnucCZkjDwB97484935 = -515214229;    double uTYOEpFnucCZkjDwB5062972 = -765366597;    double uTYOEpFnucCZkjDwB66914258 = -586114806;    double uTYOEpFnucCZkjDwB23588647 = -95318405;    double uTYOEpFnucCZkjDwB17024976 = -366228092;    double uTYOEpFnucCZkjDwB26895006 = -182137712;    double uTYOEpFnucCZkjDwB803635 = -36153521;    double uTYOEpFnucCZkjDwB11703327 = -323336145;    double uTYOEpFnucCZkjDwB14982580 = -518023023;    double uTYOEpFnucCZkjDwB99705560 = -589566794;    double uTYOEpFnucCZkjDwB42072314 = 17015403;    double uTYOEpFnucCZkjDwB67672139 = -965248215;    double uTYOEpFnucCZkjDwB40297460 = -707902357;    double uTYOEpFnucCZkjDwB70623654 = -966230090;    double uTYOEpFnucCZkjDwB1974964 = -651370224;    double uTYOEpFnucCZkjDwB35830232 = 49398968;    double uTYOEpFnucCZkjDwB41581019 = -149501100;    double uTYOEpFnucCZkjDwB19946216 = -282514272;    double uTYOEpFnucCZkjDwB50560083 = -231683921;    double uTYOEpFnucCZkjDwB14524790 = -806220128;    double uTYOEpFnucCZkjDwB49049729 = -150008121;    double uTYOEpFnucCZkjDwB72777278 = -920011642;    double uTYOEpFnucCZkjDwB79593775 = -147354101;    double uTYOEpFnucCZkjDwB10684775 = -713812431;    double uTYOEpFnucCZkjDwB45486861 = -945697523;    double uTYOEpFnucCZkjDwB28408838 = -174858454;    double uTYOEpFnucCZkjDwB52543775 = 5687481;    double uTYOEpFnucCZkjDwB98225255 = -911454956;    double uTYOEpFnucCZkjDwB65699894 = 18673461;    double uTYOEpFnucCZkjDwB98469731 = -507480525;    double uTYOEpFnucCZkjDwB44700715 = -369404020;    double uTYOEpFnucCZkjDwB80361493 = -909371071;    double uTYOEpFnucCZkjDwB41288507 = -779659459;    double uTYOEpFnucCZkjDwB90141453 = -885860154;    double uTYOEpFnucCZkjDwB34657130 = -659436032;    double uTYOEpFnucCZkjDwB11253786 = -608485660;    double uTYOEpFnucCZkjDwB8523424 = -545420625;    double uTYOEpFnucCZkjDwB64128335 = -526818336;    double uTYOEpFnucCZkjDwB256403 = -745219876;    double uTYOEpFnucCZkjDwB16152318 = -204537044;    double uTYOEpFnucCZkjDwB36164335 = -452573050;    double uTYOEpFnucCZkjDwB68863127 = -747836705;    double uTYOEpFnucCZkjDwB40525566 = -171745381;    double uTYOEpFnucCZkjDwB57283831 = -611363042;    double uTYOEpFnucCZkjDwB81009623 = -807571210;    double uTYOEpFnucCZkjDwB70455066 = -91121306;    double uTYOEpFnucCZkjDwB71486315 = -716864298;    double uTYOEpFnucCZkjDwB73592451 = -87477876;    double uTYOEpFnucCZkjDwB72964959 = -120586873;    double uTYOEpFnucCZkjDwB84922436 = -489587011;    double uTYOEpFnucCZkjDwB51803800 = -381053374;    double uTYOEpFnucCZkjDwB7312221 = -366029091;    double uTYOEpFnucCZkjDwB95922661 = -404176177;    double uTYOEpFnucCZkjDwB22176544 = -807514175;     uTYOEpFnucCZkjDwB71664711 = uTYOEpFnucCZkjDwB68916552;     uTYOEpFnucCZkjDwB68916552 = uTYOEpFnucCZkjDwB45661980;     uTYOEpFnucCZkjDwB45661980 = uTYOEpFnucCZkjDwB96987353;     uTYOEpFnucCZkjDwB96987353 = uTYOEpFnucCZkjDwB2765720;     uTYOEpFnucCZkjDwB2765720 = uTYOEpFnucCZkjDwB18291722;     uTYOEpFnucCZkjDwB18291722 = uTYOEpFnucCZkjDwB11267914;     uTYOEpFnucCZkjDwB11267914 = uTYOEpFnucCZkjDwB29298001;     uTYOEpFnucCZkjDwB29298001 = uTYOEpFnucCZkjDwB37227594;     uTYOEpFnucCZkjDwB37227594 = uTYOEpFnucCZkjDwB93607407;     uTYOEpFnucCZkjDwB93607407 = uTYOEpFnucCZkjDwB51492759;     uTYOEpFnucCZkjDwB51492759 = uTYOEpFnucCZkjDwB29616183;     uTYOEpFnucCZkjDwB29616183 = uTYOEpFnucCZkjDwB28285424;     uTYOEpFnucCZkjDwB28285424 = uTYOEpFnucCZkjDwB36583166;     uTYOEpFnucCZkjDwB36583166 = uTYOEpFnucCZkjDwB33766713;     uTYOEpFnucCZkjDwB33766713 = uTYOEpFnucCZkjDwB83614066;     uTYOEpFnucCZkjDwB83614066 = uTYOEpFnucCZkjDwB40192612;     uTYOEpFnucCZkjDwB40192612 = uTYOEpFnucCZkjDwB96759880;     uTYOEpFnucCZkjDwB96759880 = uTYOEpFnucCZkjDwB73165179;     uTYOEpFnucCZkjDwB73165179 = uTYOEpFnucCZkjDwB86452562;     uTYOEpFnucCZkjDwB86452562 = uTYOEpFnucCZkjDwB91854550;     uTYOEpFnucCZkjDwB91854550 = uTYOEpFnucCZkjDwB71959151;     uTYOEpFnucCZkjDwB71959151 = uTYOEpFnucCZkjDwB26844239;     uTYOEpFnucCZkjDwB26844239 = uTYOEpFnucCZkjDwB77989840;     uTYOEpFnucCZkjDwB77989840 = uTYOEpFnucCZkjDwB56689894;     uTYOEpFnucCZkjDwB56689894 = uTYOEpFnucCZkjDwB32142066;     uTYOEpFnucCZkjDwB32142066 = uTYOEpFnucCZkjDwB16316758;     uTYOEpFnucCZkjDwB16316758 = uTYOEpFnucCZkjDwB75437682;     uTYOEpFnucCZkjDwB75437682 = uTYOEpFnucCZkjDwB87716982;     uTYOEpFnucCZkjDwB87716982 = uTYOEpFnucCZkjDwB17281378;     uTYOEpFnucCZkjDwB17281378 = uTYOEpFnucCZkjDwB43047324;     uTYOEpFnucCZkjDwB43047324 = uTYOEpFnucCZkjDwB36967969;     uTYOEpFnucCZkjDwB36967969 = uTYOEpFnucCZkjDwB80566454;     uTYOEpFnucCZkjDwB80566454 = uTYOEpFnucCZkjDwB55508145;     uTYOEpFnucCZkjDwB55508145 = uTYOEpFnucCZkjDwB56989391;     uTYOEpFnucCZkjDwB56989391 = uTYOEpFnucCZkjDwB23081938;     uTYOEpFnucCZkjDwB23081938 = uTYOEpFnucCZkjDwB38127205;     uTYOEpFnucCZkjDwB38127205 = uTYOEpFnucCZkjDwB11783775;     uTYOEpFnucCZkjDwB11783775 = uTYOEpFnucCZkjDwB44216105;     uTYOEpFnucCZkjDwB44216105 = uTYOEpFnucCZkjDwB74939923;     uTYOEpFnucCZkjDwB74939923 = uTYOEpFnucCZkjDwB20752669;     uTYOEpFnucCZkjDwB20752669 = uTYOEpFnucCZkjDwB93384819;     uTYOEpFnucCZkjDwB93384819 = uTYOEpFnucCZkjDwB27258437;     uTYOEpFnucCZkjDwB27258437 = uTYOEpFnucCZkjDwB46482745;     uTYOEpFnucCZkjDwB46482745 = uTYOEpFnucCZkjDwB36701334;     uTYOEpFnucCZkjDwB36701334 = uTYOEpFnucCZkjDwB66548440;     uTYOEpFnucCZkjDwB66548440 = uTYOEpFnucCZkjDwB97484935;     uTYOEpFnucCZkjDwB97484935 = uTYOEpFnucCZkjDwB5062972;     uTYOEpFnucCZkjDwB5062972 = uTYOEpFnucCZkjDwB66914258;     uTYOEpFnucCZkjDwB66914258 = uTYOEpFnucCZkjDwB23588647;     uTYOEpFnucCZkjDwB23588647 = uTYOEpFnucCZkjDwB17024976;     uTYOEpFnucCZkjDwB17024976 = uTYOEpFnucCZkjDwB26895006;     uTYOEpFnucCZkjDwB26895006 = uTYOEpFnucCZkjDwB803635;     uTYOEpFnucCZkjDwB803635 = uTYOEpFnucCZkjDwB11703327;     uTYOEpFnucCZkjDwB11703327 = uTYOEpFnucCZkjDwB14982580;     uTYOEpFnucCZkjDwB14982580 = uTYOEpFnucCZkjDwB99705560;     uTYOEpFnucCZkjDwB99705560 = uTYOEpFnucCZkjDwB42072314;     uTYOEpFnucCZkjDwB42072314 = uTYOEpFnucCZkjDwB67672139;     uTYOEpFnucCZkjDwB67672139 = uTYOEpFnucCZkjDwB40297460;     uTYOEpFnucCZkjDwB40297460 = uTYOEpFnucCZkjDwB70623654;     uTYOEpFnucCZkjDwB70623654 = uTYOEpFnucCZkjDwB1974964;     uTYOEpFnucCZkjDwB1974964 = uTYOEpFnucCZkjDwB35830232;     uTYOEpFnucCZkjDwB35830232 = uTYOEpFnucCZkjDwB41581019;     uTYOEpFnucCZkjDwB41581019 = uTYOEpFnucCZkjDwB19946216;     uTYOEpFnucCZkjDwB19946216 = uTYOEpFnucCZkjDwB50560083;     uTYOEpFnucCZkjDwB50560083 = uTYOEpFnucCZkjDwB14524790;     uTYOEpFnucCZkjDwB14524790 = uTYOEpFnucCZkjDwB49049729;     uTYOEpFnucCZkjDwB49049729 = uTYOEpFnucCZkjDwB72777278;     uTYOEpFnucCZkjDwB72777278 = uTYOEpFnucCZkjDwB79593775;     uTYOEpFnucCZkjDwB79593775 = uTYOEpFnucCZkjDwB10684775;     uTYOEpFnucCZkjDwB10684775 = uTYOEpFnucCZkjDwB45486861;     uTYOEpFnucCZkjDwB45486861 = uTYOEpFnucCZkjDwB28408838;     uTYOEpFnucCZkjDwB28408838 = uTYOEpFnucCZkjDwB52543775;     uTYOEpFnucCZkjDwB52543775 = uTYOEpFnucCZkjDwB98225255;     uTYOEpFnucCZkjDwB98225255 = uTYOEpFnucCZkjDwB65699894;     uTYOEpFnucCZkjDwB65699894 = uTYOEpFnucCZkjDwB98469731;     uTYOEpFnucCZkjDwB98469731 = uTYOEpFnucCZkjDwB44700715;     uTYOEpFnucCZkjDwB44700715 = uTYOEpFnucCZkjDwB80361493;     uTYOEpFnucCZkjDwB80361493 = uTYOEpFnucCZkjDwB41288507;     uTYOEpFnucCZkjDwB41288507 = uTYOEpFnucCZkjDwB90141453;     uTYOEpFnucCZkjDwB90141453 = uTYOEpFnucCZkjDwB34657130;     uTYOEpFnucCZkjDwB34657130 = uTYOEpFnucCZkjDwB11253786;     uTYOEpFnucCZkjDwB11253786 = uTYOEpFnucCZkjDwB8523424;     uTYOEpFnucCZkjDwB8523424 = uTYOEpFnucCZkjDwB64128335;     uTYOEpFnucCZkjDwB64128335 = uTYOEpFnucCZkjDwB256403;     uTYOEpFnucCZkjDwB256403 = uTYOEpFnucCZkjDwB16152318;     uTYOEpFnucCZkjDwB16152318 = uTYOEpFnucCZkjDwB36164335;     uTYOEpFnucCZkjDwB36164335 = uTYOEpFnucCZkjDwB68863127;     uTYOEpFnucCZkjDwB68863127 = uTYOEpFnucCZkjDwB40525566;     uTYOEpFnucCZkjDwB40525566 = uTYOEpFnucCZkjDwB57283831;     uTYOEpFnucCZkjDwB57283831 = uTYOEpFnucCZkjDwB81009623;     uTYOEpFnucCZkjDwB81009623 = uTYOEpFnucCZkjDwB70455066;     uTYOEpFnucCZkjDwB70455066 = uTYOEpFnucCZkjDwB71486315;     uTYOEpFnucCZkjDwB71486315 = uTYOEpFnucCZkjDwB73592451;     uTYOEpFnucCZkjDwB73592451 = uTYOEpFnucCZkjDwB72964959;     uTYOEpFnucCZkjDwB72964959 = uTYOEpFnucCZkjDwB84922436;     uTYOEpFnucCZkjDwB84922436 = uTYOEpFnucCZkjDwB51803800;     uTYOEpFnucCZkjDwB51803800 = uTYOEpFnucCZkjDwB7312221;     uTYOEpFnucCZkjDwB7312221 = uTYOEpFnucCZkjDwB95922661;     uTYOEpFnucCZkjDwB95922661 = uTYOEpFnucCZkjDwB22176544;     uTYOEpFnucCZkjDwB22176544 = uTYOEpFnucCZkjDwB71664711;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void xkuTMVohxJpkUDCk74128729() {     double qtyXwVNEsEwxqiXar7123680 = -173791075;    double qtyXwVNEsEwxqiXar82188631 = -608767371;    double qtyXwVNEsEwxqiXar83048456 = -385505909;    double qtyXwVNEsEwxqiXar69210708 = -202626757;    double qtyXwVNEsEwxqiXar59050989 = -246479195;    double qtyXwVNEsEwxqiXar54514594 = -287570375;    double qtyXwVNEsEwxqiXar21925320 = -26862195;    double qtyXwVNEsEwxqiXar91429561 = -615871980;    double qtyXwVNEsEwxqiXar38745199 = -982293646;    double qtyXwVNEsEwxqiXar8510120 = -483045675;    double qtyXwVNEsEwxqiXar89674494 = -196954427;    double qtyXwVNEsEwxqiXar30184332 = -185301741;    double qtyXwVNEsEwxqiXar66520069 = -161792551;    double qtyXwVNEsEwxqiXar89724029 = -424647345;    double qtyXwVNEsEwxqiXar91961228 = -7793252;    double qtyXwVNEsEwxqiXar67155367 = -737878651;    double qtyXwVNEsEwxqiXar96028548 = -126599661;    double qtyXwVNEsEwxqiXar96520071 = -221448552;    double qtyXwVNEsEwxqiXar73715509 = 24527;    double qtyXwVNEsEwxqiXar80289781 = 51924354;    double qtyXwVNEsEwxqiXar4941414 = -895309687;    double qtyXwVNEsEwxqiXar75380675 = -204583676;    double qtyXwVNEsEwxqiXar97369618 = -282863352;    double qtyXwVNEsEwxqiXar71782786 = -165415224;    double qtyXwVNEsEwxqiXar64707845 = -697558528;    double qtyXwVNEsEwxqiXar42840238 = -963227067;    double qtyXwVNEsEwxqiXar76737332 = -180136397;    double qtyXwVNEsEwxqiXar16166107 = -301307604;    double qtyXwVNEsEwxqiXar34385197 = -196231543;    double qtyXwVNEsEwxqiXar22814789 = -145955138;    double qtyXwVNEsEwxqiXar61630556 = 61344757;    double qtyXwVNEsEwxqiXar74790988 = -361697984;    double qtyXwVNEsEwxqiXar92784465 = -218768827;    double qtyXwVNEsEwxqiXar85256572 = -854462217;    double qtyXwVNEsEwxqiXar22797845 = -152030086;    double qtyXwVNEsEwxqiXar74642781 = -5121112;    double qtyXwVNEsEwxqiXar87539021 = 78934439;    double qtyXwVNEsEwxqiXar62999178 = -407422236;    double qtyXwVNEsEwxqiXar74606666 = -804584241;    double qtyXwVNEsEwxqiXar39211456 = -337219642;    double qtyXwVNEsEwxqiXar54204276 = -309386956;    double qtyXwVNEsEwxqiXar82749945 = -225099764;    double qtyXwVNEsEwxqiXar16641815 = -301724149;    double qtyXwVNEsEwxqiXar86614083 = -240192507;    double qtyXwVNEsEwxqiXar36717793 = -801442323;    double qtyXwVNEsEwxqiXar12509769 = -826636809;    double qtyXwVNEsEwxqiXar16830100 = -999200589;    double qtyXwVNEsEwxqiXar22789284 = -931893190;    double qtyXwVNEsEwxqiXar89806924 = -249851686;    double qtyXwVNEsEwxqiXar48118236 = -586345220;    double qtyXwVNEsEwxqiXar97795699 = -240504649;    double qtyXwVNEsEwxqiXar10380479 = -395790059;    double qtyXwVNEsEwxqiXar69424782 = -892199075;    double qtyXwVNEsEwxqiXar27695174 = -514844645;    double qtyXwVNEsEwxqiXar69187316 = -864513169;    double qtyXwVNEsEwxqiXar31743004 = -969207400;    double qtyXwVNEsEwxqiXar84819013 = -225904019;    double qtyXwVNEsEwxqiXar11265671 = -120090686;    double qtyXwVNEsEwxqiXar4502864 = -505068230;    double qtyXwVNEsEwxqiXar16210752 = -283252128;    double qtyXwVNEsEwxqiXar77777262 = -7433979;    double qtyXwVNEsEwxqiXar5759214 = -725554592;    double qtyXwVNEsEwxqiXar57044365 = -319640438;    double qtyXwVNEsEwxqiXar15930410 = -736338508;    double qtyXwVNEsEwxqiXar46879564 = -444390432;    double qtyXwVNEsEwxqiXar14883507 = -835256443;    double qtyXwVNEsEwxqiXar37399867 = -966532915;    double qtyXwVNEsEwxqiXar81263497 = -307330334;    double qtyXwVNEsEwxqiXar66926184 = -172617260;    double qtyXwVNEsEwxqiXar17318448 = 97327860;    double qtyXwVNEsEwxqiXar79616345 = -716813090;    double qtyXwVNEsEwxqiXar33029370 = -719177425;    double qtyXwVNEsEwxqiXar21913406 = -416864311;    double qtyXwVNEsEwxqiXar34504054 = -662755831;    double qtyXwVNEsEwxqiXar26085505 = -638688691;    double qtyXwVNEsEwxqiXar22191468 = -570209923;    double qtyXwVNEsEwxqiXar58738861 = -902859528;    double qtyXwVNEsEwxqiXar10755535 = 57329155;    double qtyXwVNEsEwxqiXar35064994 = -363972901;    double qtyXwVNEsEwxqiXar52198076 = -870921720;    double qtyXwVNEsEwxqiXar26010138 = -964026479;    double qtyXwVNEsEwxqiXar53948048 = -248243207;    double qtyXwVNEsEwxqiXar26359183 = 48544081;    double qtyXwVNEsEwxqiXar86266960 = -609886323;    double qtyXwVNEsEwxqiXar25019090 = -905450490;    double qtyXwVNEsEwxqiXar51250077 = -542865185;    double qtyXwVNEsEwxqiXar5366206 = -469498910;    double qtyXwVNEsEwxqiXar65089291 = -703924183;    double qtyXwVNEsEwxqiXar16069257 = -989949049;    double qtyXwVNEsEwxqiXar91054841 = -182822687;    double qtyXwVNEsEwxqiXar89823768 = -779217094;    double qtyXwVNEsEwxqiXar76273351 = -800974876;    double qtyXwVNEsEwxqiXar58496315 = -902354007;    double qtyXwVNEsEwxqiXar58395915 = -421332114;    double qtyXwVNEsEwxqiXar61434194 = -229785664;    double qtyXwVNEsEwxqiXar48445063 = -583832365;    double qtyXwVNEsEwxqiXar25705581 = -905459327;    double qtyXwVNEsEwxqiXar711405 = -565385641;    double qtyXwVNEsEwxqiXar39734519 = -795802076;    double qtyXwVNEsEwxqiXar21834286 = -173791075;     qtyXwVNEsEwxqiXar7123680 = qtyXwVNEsEwxqiXar82188631;     qtyXwVNEsEwxqiXar82188631 = qtyXwVNEsEwxqiXar83048456;     qtyXwVNEsEwxqiXar83048456 = qtyXwVNEsEwxqiXar69210708;     qtyXwVNEsEwxqiXar69210708 = qtyXwVNEsEwxqiXar59050989;     qtyXwVNEsEwxqiXar59050989 = qtyXwVNEsEwxqiXar54514594;     qtyXwVNEsEwxqiXar54514594 = qtyXwVNEsEwxqiXar21925320;     qtyXwVNEsEwxqiXar21925320 = qtyXwVNEsEwxqiXar91429561;     qtyXwVNEsEwxqiXar91429561 = qtyXwVNEsEwxqiXar38745199;     qtyXwVNEsEwxqiXar38745199 = qtyXwVNEsEwxqiXar8510120;     qtyXwVNEsEwxqiXar8510120 = qtyXwVNEsEwxqiXar89674494;     qtyXwVNEsEwxqiXar89674494 = qtyXwVNEsEwxqiXar30184332;     qtyXwVNEsEwxqiXar30184332 = qtyXwVNEsEwxqiXar66520069;     qtyXwVNEsEwxqiXar66520069 = qtyXwVNEsEwxqiXar89724029;     qtyXwVNEsEwxqiXar89724029 = qtyXwVNEsEwxqiXar91961228;     qtyXwVNEsEwxqiXar91961228 = qtyXwVNEsEwxqiXar67155367;     qtyXwVNEsEwxqiXar67155367 = qtyXwVNEsEwxqiXar96028548;     qtyXwVNEsEwxqiXar96028548 = qtyXwVNEsEwxqiXar96520071;     qtyXwVNEsEwxqiXar96520071 = qtyXwVNEsEwxqiXar73715509;     qtyXwVNEsEwxqiXar73715509 = qtyXwVNEsEwxqiXar80289781;     qtyXwVNEsEwxqiXar80289781 = qtyXwVNEsEwxqiXar4941414;     qtyXwVNEsEwxqiXar4941414 = qtyXwVNEsEwxqiXar75380675;     qtyXwVNEsEwxqiXar75380675 = qtyXwVNEsEwxqiXar97369618;     qtyXwVNEsEwxqiXar97369618 = qtyXwVNEsEwxqiXar71782786;     qtyXwVNEsEwxqiXar71782786 = qtyXwVNEsEwxqiXar64707845;     qtyXwVNEsEwxqiXar64707845 = qtyXwVNEsEwxqiXar42840238;     qtyXwVNEsEwxqiXar42840238 = qtyXwVNEsEwxqiXar76737332;     qtyXwVNEsEwxqiXar76737332 = qtyXwVNEsEwxqiXar16166107;     qtyXwVNEsEwxqiXar16166107 = qtyXwVNEsEwxqiXar34385197;     qtyXwVNEsEwxqiXar34385197 = qtyXwVNEsEwxqiXar22814789;     qtyXwVNEsEwxqiXar22814789 = qtyXwVNEsEwxqiXar61630556;     qtyXwVNEsEwxqiXar61630556 = qtyXwVNEsEwxqiXar74790988;     qtyXwVNEsEwxqiXar74790988 = qtyXwVNEsEwxqiXar92784465;     qtyXwVNEsEwxqiXar92784465 = qtyXwVNEsEwxqiXar85256572;     qtyXwVNEsEwxqiXar85256572 = qtyXwVNEsEwxqiXar22797845;     qtyXwVNEsEwxqiXar22797845 = qtyXwVNEsEwxqiXar74642781;     qtyXwVNEsEwxqiXar74642781 = qtyXwVNEsEwxqiXar87539021;     qtyXwVNEsEwxqiXar87539021 = qtyXwVNEsEwxqiXar62999178;     qtyXwVNEsEwxqiXar62999178 = qtyXwVNEsEwxqiXar74606666;     qtyXwVNEsEwxqiXar74606666 = qtyXwVNEsEwxqiXar39211456;     qtyXwVNEsEwxqiXar39211456 = qtyXwVNEsEwxqiXar54204276;     qtyXwVNEsEwxqiXar54204276 = qtyXwVNEsEwxqiXar82749945;     qtyXwVNEsEwxqiXar82749945 = qtyXwVNEsEwxqiXar16641815;     qtyXwVNEsEwxqiXar16641815 = qtyXwVNEsEwxqiXar86614083;     qtyXwVNEsEwxqiXar86614083 = qtyXwVNEsEwxqiXar36717793;     qtyXwVNEsEwxqiXar36717793 = qtyXwVNEsEwxqiXar12509769;     qtyXwVNEsEwxqiXar12509769 = qtyXwVNEsEwxqiXar16830100;     qtyXwVNEsEwxqiXar16830100 = qtyXwVNEsEwxqiXar22789284;     qtyXwVNEsEwxqiXar22789284 = qtyXwVNEsEwxqiXar89806924;     qtyXwVNEsEwxqiXar89806924 = qtyXwVNEsEwxqiXar48118236;     qtyXwVNEsEwxqiXar48118236 = qtyXwVNEsEwxqiXar97795699;     qtyXwVNEsEwxqiXar97795699 = qtyXwVNEsEwxqiXar10380479;     qtyXwVNEsEwxqiXar10380479 = qtyXwVNEsEwxqiXar69424782;     qtyXwVNEsEwxqiXar69424782 = qtyXwVNEsEwxqiXar27695174;     qtyXwVNEsEwxqiXar27695174 = qtyXwVNEsEwxqiXar69187316;     qtyXwVNEsEwxqiXar69187316 = qtyXwVNEsEwxqiXar31743004;     qtyXwVNEsEwxqiXar31743004 = qtyXwVNEsEwxqiXar84819013;     qtyXwVNEsEwxqiXar84819013 = qtyXwVNEsEwxqiXar11265671;     qtyXwVNEsEwxqiXar11265671 = qtyXwVNEsEwxqiXar4502864;     qtyXwVNEsEwxqiXar4502864 = qtyXwVNEsEwxqiXar16210752;     qtyXwVNEsEwxqiXar16210752 = qtyXwVNEsEwxqiXar77777262;     qtyXwVNEsEwxqiXar77777262 = qtyXwVNEsEwxqiXar5759214;     qtyXwVNEsEwxqiXar5759214 = qtyXwVNEsEwxqiXar57044365;     qtyXwVNEsEwxqiXar57044365 = qtyXwVNEsEwxqiXar15930410;     qtyXwVNEsEwxqiXar15930410 = qtyXwVNEsEwxqiXar46879564;     qtyXwVNEsEwxqiXar46879564 = qtyXwVNEsEwxqiXar14883507;     qtyXwVNEsEwxqiXar14883507 = qtyXwVNEsEwxqiXar37399867;     qtyXwVNEsEwxqiXar37399867 = qtyXwVNEsEwxqiXar81263497;     qtyXwVNEsEwxqiXar81263497 = qtyXwVNEsEwxqiXar66926184;     qtyXwVNEsEwxqiXar66926184 = qtyXwVNEsEwxqiXar17318448;     qtyXwVNEsEwxqiXar17318448 = qtyXwVNEsEwxqiXar79616345;     qtyXwVNEsEwxqiXar79616345 = qtyXwVNEsEwxqiXar33029370;     qtyXwVNEsEwxqiXar33029370 = qtyXwVNEsEwxqiXar21913406;     qtyXwVNEsEwxqiXar21913406 = qtyXwVNEsEwxqiXar34504054;     qtyXwVNEsEwxqiXar34504054 = qtyXwVNEsEwxqiXar26085505;     qtyXwVNEsEwxqiXar26085505 = qtyXwVNEsEwxqiXar22191468;     qtyXwVNEsEwxqiXar22191468 = qtyXwVNEsEwxqiXar58738861;     qtyXwVNEsEwxqiXar58738861 = qtyXwVNEsEwxqiXar10755535;     qtyXwVNEsEwxqiXar10755535 = qtyXwVNEsEwxqiXar35064994;     qtyXwVNEsEwxqiXar35064994 = qtyXwVNEsEwxqiXar52198076;     qtyXwVNEsEwxqiXar52198076 = qtyXwVNEsEwxqiXar26010138;     qtyXwVNEsEwxqiXar26010138 = qtyXwVNEsEwxqiXar53948048;     qtyXwVNEsEwxqiXar53948048 = qtyXwVNEsEwxqiXar26359183;     qtyXwVNEsEwxqiXar26359183 = qtyXwVNEsEwxqiXar86266960;     qtyXwVNEsEwxqiXar86266960 = qtyXwVNEsEwxqiXar25019090;     qtyXwVNEsEwxqiXar25019090 = qtyXwVNEsEwxqiXar51250077;     qtyXwVNEsEwxqiXar51250077 = qtyXwVNEsEwxqiXar5366206;     qtyXwVNEsEwxqiXar5366206 = qtyXwVNEsEwxqiXar65089291;     qtyXwVNEsEwxqiXar65089291 = qtyXwVNEsEwxqiXar16069257;     qtyXwVNEsEwxqiXar16069257 = qtyXwVNEsEwxqiXar91054841;     qtyXwVNEsEwxqiXar91054841 = qtyXwVNEsEwxqiXar89823768;     qtyXwVNEsEwxqiXar89823768 = qtyXwVNEsEwxqiXar76273351;     qtyXwVNEsEwxqiXar76273351 = qtyXwVNEsEwxqiXar58496315;     qtyXwVNEsEwxqiXar58496315 = qtyXwVNEsEwxqiXar58395915;     qtyXwVNEsEwxqiXar58395915 = qtyXwVNEsEwxqiXar61434194;     qtyXwVNEsEwxqiXar61434194 = qtyXwVNEsEwxqiXar48445063;     qtyXwVNEsEwxqiXar48445063 = qtyXwVNEsEwxqiXar25705581;     qtyXwVNEsEwxqiXar25705581 = qtyXwVNEsEwxqiXar711405;     qtyXwVNEsEwxqiXar711405 = qtyXwVNEsEwxqiXar39734519;     qtyXwVNEsEwxqiXar39734519 = qtyXwVNEsEwxqiXar21834286;     qtyXwVNEsEwxqiXar21834286 = qtyXwVNEsEwxqiXar7123680;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zrDojpZxrreLYAhe19494225() {     double MlfhzXZKDBTPVARrP80973506 = -582569343;    double MlfhzXZKDBTPVARrP62911791 = -787345392;    double MlfhzXZKDBTPVARrP67127272 = -390608061;    double MlfhzXZKDBTPVARrP39857236 = -393680508;    double MlfhzXZKDBTPVARrP51669061 = -585750828;    double MlfhzXZKDBTPVARrP46640516 = -283524152;    double MlfhzXZKDBTPVARrP53469321 = -785921806;    double MlfhzXZKDBTPVARrP87648207 = -872774305;    double MlfhzXZKDBTPVARrP12776415 = -587668980;    double MlfhzXZKDBTPVARrP58871378 = -275961216;    double MlfhzXZKDBTPVARrP8715350 = -545659842;    double MlfhzXZKDBTPVARrP55145610 = -694029147;    double MlfhzXZKDBTPVARrP55920053 = -316860655;    double MlfhzXZKDBTPVARrP22516028 = -772491092;    double MlfhzXZKDBTPVARrP94214859 = -418611500;    double MlfhzXZKDBTPVARrP63149137 = -362386172;    double MlfhzXZKDBTPVARrP39856140 = -546824765;    double MlfhzXZKDBTPVARrP31953054 = -275149418;    double MlfhzXZKDBTPVARrP75606380 = -255035148;    double MlfhzXZKDBTPVARrP21721674 = -212246873;    double MlfhzXZKDBTPVARrP60203326 = -174201300;    double MlfhzXZKDBTPVARrP74464329 = -450835598;    double MlfhzXZKDBTPVARrP99783901 = -718718010;    double MlfhzXZKDBTPVARrP80846578 = -152672252;    double MlfhzXZKDBTPVARrP48195126 = -319400234;    double MlfhzXZKDBTPVARrP14623961 = -182067830;    double MlfhzXZKDBTPVARrP20296606 = -383540814;    double MlfhzXZKDBTPVARrP81935682 = -147639151;    double MlfhzXZKDBTPVARrP67512336 = -382626491;    double MlfhzXZKDBTPVARrP6152196 = -692963904;    double MlfhzXZKDBTPVARrP46458382 = -484419807;    double MlfhzXZKDBTPVARrP98951190 = -311719626;    double MlfhzXZKDBTPVARrP15878208 = -84208078;    double MlfhzXZKDBTPVARrP44819931 = -150010023;    double MlfhzXZKDBTPVARrP25997705 = -636656355;    double MlfhzXZKDBTPVARrP97735021 = -286547000;    double MlfhzXZKDBTPVARrP87536180 = -857222762;    double MlfhzXZKDBTPVARrP37738253 = -172624697;    double MlfhzXZKDBTPVARrP45755402 = -817605168;    double MlfhzXZKDBTPVARrP22912713 = -188574295;    double MlfhzXZKDBTPVARrP32671470 = -765288358;    double MlfhzXZKDBTPVARrP66734135 = -745618647;    double MlfhzXZKDBTPVARrP9258803 = -429553998;    double MlfhzXZKDBTPVARrP64471543 = -783219429;    double MlfhzXZKDBTPVARrP56669461 = -139966357;    double MlfhzXZKDBTPVARrP77215644 = -925292406;    double MlfhzXZKDBTPVARrP30432360 = -569328996;    double MlfhzXZKDBTPVARrP51917590 = -720671341;    double MlfhzXZKDBTPVARrP51599792 = -385575183;    double MlfhzXZKDBTPVARrP13271972 = -575356993;    double MlfhzXZKDBTPVARrP67252055 = -780659096;    double MlfhzXZKDBTPVARrP5659306 = -112269678;    double MlfhzXZKDBTPVARrP64259640 = -615984435;    double MlfhzXZKDBTPVARrP26534842 = -115109114;    double MlfhzXZKDBTPVARrP59290898 = -209840385;    double MlfhzXZKDBTPVARrP6509178 = -31733746;    double MlfhzXZKDBTPVARrP63127890 = 31372617;    double MlfhzXZKDBTPVARrP86280694 = -137935809;    double MlfhzXZKDBTPVARrP91662110 = 25719725;    double MlfhzXZKDBTPVARrP37045101 = -303682999;    double MlfhzXZKDBTPVARrP26343910 = -899983339;    double MlfhzXZKDBTPVARrP71533639 = -538282656;    double MlfhzXZKDBTPVARrP20135872 = -390147814;    double MlfhzXZKDBTPVARrP6624219 = -894705077;    double MlfhzXZKDBTPVARrP12412997 = -791541409;    double MlfhzXZKDBTPVARrP9764160 = -133940216;    double MlfhzXZKDBTPVARrP39267402 = -509821070;    double MlfhzXZKDBTPVARrP11100123 = -66850632;    double MlfhzXZKDBTPVARrP96518322 = -35834738;    double MlfhzXZKDBTPVARrP96479837 = -32064501;    double MlfhzXZKDBTPVARrP75612956 = -505163410;    double MlfhzXZKDBTPVARrP2117887 = -274200069;    double MlfhzXZKDBTPVARrP86197651 = -457544250;    double MlfhzXZKDBTPVARrP52693668 = 33539147;    double MlfhzXZKDBTPVARrP89050203 = -446958515;    double MlfhzXZKDBTPVARrP93469190 = -428582654;    double MlfhzXZKDBTPVARrP65205526 = 78718400;    double MlfhzXZKDBTPVARrP35312358 = -935498582;    double MlfhzXZKDBTPVARrP24177118 = 87294104;    double MlfhzXZKDBTPVARrP70979482 = -394107828;    double MlfhzXZKDBTPVARrP84191600 = -612738834;    double MlfhzXZKDBTPVARrP68379016 = -662869473;    double MlfhzXZKDBTPVARrP30335891 = -762063968;    double MlfhzXZKDBTPVARrP54240364 = -807269498;    double MlfhzXZKDBTPVARrP38900141 = -912304809;    double MlfhzXZKDBTPVARrP40799076 = -272150130;    double MlfhzXZKDBTPVARrP34691550 = -695735192;    double MlfhzXZKDBTPVARrP89343365 = -969098964;    double MlfhzXZKDBTPVARrP85529032 = -940169638;    double MlfhzXZKDBTPVARrP19488528 = -504922609;    double MlfhzXZKDBTPVARrP34607132 = -217919618;    double MlfhzXZKDBTPVARrP1255487 = -619286953;    double MlfhzXZKDBTPVARrP46076143 = -98344422;    double MlfhzXZKDBTPVARrP8710302 = -413922170;    double MlfhzXZKDBTPVARrP96568803 = -288590957;    double MlfhzXZKDBTPVARrP61137831 = -127005703;    double MlfhzXZKDBTPVARrP46598264 = -255470833;    double MlfhzXZKDBTPVARrP2634584 = -534848922;    double MlfhzXZKDBTPVARrP52058547 = -991678020;    double MlfhzXZKDBTPVARrP46905301 = -582569343;     MlfhzXZKDBTPVARrP80973506 = MlfhzXZKDBTPVARrP62911791;     MlfhzXZKDBTPVARrP62911791 = MlfhzXZKDBTPVARrP67127272;     MlfhzXZKDBTPVARrP67127272 = MlfhzXZKDBTPVARrP39857236;     MlfhzXZKDBTPVARrP39857236 = MlfhzXZKDBTPVARrP51669061;     MlfhzXZKDBTPVARrP51669061 = MlfhzXZKDBTPVARrP46640516;     MlfhzXZKDBTPVARrP46640516 = MlfhzXZKDBTPVARrP53469321;     MlfhzXZKDBTPVARrP53469321 = MlfhzXZKDBTPVARrP87648207;     MlfhzXZKDBTPVARrP87648207 = MlfhzXZKDBTPVARrP12776415;     MlfhzXZKDBTPVARrP12776415 = MlfhzXZKDBTPVARrP58871378;     MlfhzXZKDBTPVARrP58871378 = MlfhzXZKDBTPVARrP8715350;     MlfhzXZKDBTPVARrP8715350 = MlfhzXZKDBTPVARrP55145610;     MlfhzXZKDBTPVARrP55145610 = MlfhzXZKDBTPVARrP55920053;     MlfhzXZKDBTPVARrP55920053 = MlfhzXZKDBTPVARrP22516028;     MlfhzXZKDBTPVARrP22516028 = MlfhzXZKDBTPVARrP94214859;     MlfhzXZKDBTPVARrP94214859 = MlfhzXZKDBTPVARrP63149137;     MlfhzXZKDBTPVARrP63149137 = MlfhzXZKDBTPVARrP39856140;     MlfhzXZKDBTPVARrP39856140 = MlfhzXZKDBTPVARrP31953054;     MlfhzXZKDBTPVARrP31953054 = MlfhzXZKDBTPVARrP75606380;     MlfhzXZKDBTPVARrP75606380 = MlfhzXZKDBTPVARrP21721674;     MlfhzXZKDBTPVARrP21721674 = MlfhzXZKDBTPVARrP60203326;     MlfhzXZKDBTPVARrP60203326 = MlfhzXZKDBTPVARrP74464329;     MlfhzXZKDBTPVARrP74464329 = MlfhzXZKDBTPVARrP99783901;     MlfhzXZKDBTPVARrP99783901 = MlfhzXZKDBTPVARrP80846578;     MlfhzXZKDBTPVARrP80846578 = MlfhzXZKDBTPVARrP48195126;     MlfhzXZKDBTPVARrP48195126 = MlfhzXZKDBTPVARrP14623961;     MlfhzXZKDBTPVARrP14623961 = MlfhzXZKDBTPVARrP20296606;     MlfhzXZKDBTPVARrP20296606 = MlfhzXZKDBTPVARrP81935682;     MlfhzXZKDBTPVARrP81935682 = MlfhzXZKDBTPVARrP67512336;     MlfhzXZKDBTPVARrP67512336 = MlfhzXZKDBTPVARrP6152196;     MlfhzXZKDBTPVARrP6152196 = MlfhzXZKDBTPVARrP46458382;     MlfhzXZKDBTPVARrP46458382 = MlfhzXZKDBTPVARrP98951190;     MlfhzXZKDBTPVARrP98951190 = MlfhzXZKDBTPVARrP15878208;     MlfhzXZKDBTPVARrP15878208 = MlfhzXZKDBTPVARrP44819931;     MlfhzXZKDBTPVARrP44819931 = MlfhzXZKDBTPVARrP25997705;     MlfhzXZKDBTPVARrP25997705 = MlfhzXZKDBTPVARrP97735021;     MlfhzXZKDBTPVARrP97735021 = MlfhzXZKDBTPVARrP87536180;     MlfhzXZKDBTPVARrP87536180 = MlfhzXZKDBTPVARrP37738253;     MlfhzXZKDBTPVARrP37738253 = MlfhzXZKDBTPVARrP45755402;     MlfhzXZKDBTPVARrP45755402 = MlfhzXZKDBTPVARrP22912713;     MlfhzXZKDBTPVARrP22912713 = MlfhzXZKDBTPVARrP32671470;     MlfhzXZKDBTPVARrP32671470 = MlfhzXZKDBTPVARrP66734135;     MlfhzXZKDBTPVARrP66734135 = MlfhzXZKDBTPVARrP9258803;     MlfhzXZKDBTPVARrP9258803 = MlfhzXZKDBTPVARrP64471543;     MlfhzXZKDBTPVARrP64471543 = MlfhzXZKDBTPVARrP56669461;     MlfhzXZKDBTPVARrP56669461 = MlfhzXZKDBTPVARrP77215644;     MlfhzXZKDBTPVARrP77215644 = MlfhzXZKDBTPVARrP30432360;     MlfhzXZKDBTPVARrP30432360 = MlfhzXZKDBTPVARrP51917590;     MlfhzXZKDBTPVARrP51917590 = MlfhzXZKDBTPVARrP51599792;     MlfhzXZKDBTPVARrP51599792 = MlfhzXZKDBTPVARrP13271972;     MlfhzXZKDBTPVARrP13271972 = MlfhzXZKDBTPVARrP67252055;     MlfhzXZKDBTPVARrP67252055 = MlfhzXZKDBTPVARrP5659306;     MlfhzXZKDBTPVARrP5659306 = MlfhzXZKDBTPVARrP64259640;     MlfhzXZKDBTPVARrP64259640 = MlfhzXZKDBTPVARrP26534842;     MlfhzXZKDBTPVARrP26534842 = MlfhzXZKDBTPVARrP59290898;     MlfhzXZKDBTPVARrP59290898 = MlfhzXZKDBTPVARrP6509178;     MlfhzXZKDBTPVARrP6509178 = MlfhzXZKDBTPVARrP63127890;     MlfhzXZKDBTPVARrP63127890 = MlfhzXZKDBTPVARrP86280694;     MlfhzXZKDBTPVARrP86280694 = MlfhzXZKDBTPVARrP91662110;     MlfhzXZKDBTPVARrP91662110 = MlfhzXZKDBTPVARrP37045101;     MlfhzXZKDBTPVARrP37045101 = MlfhzXZKDBTPVARrP26343910;     MlfhzXZKDBTPVARrP26343910 = MlfhzXZKDBTPVARrP71533639;     MlfhzXZKDBTPVARrP71533639 = MlfhzXZKDBTPVARrP20135872;     MlfhzXZKDBTPVARrP20135872 = MlfhzXZKDBTPVARrP6624219;     MlfhzXZKDBTPVARrP6624219 = MlfhzXZKDBTPVARrP12412997;     MlfhzXZKDBTPVARrP12412997 = MlfhzXZKDBTPVARrP9764160;     MlfhzXZKDBTPVARrP9764160 = MlfhzXZKDBTPVARrP39267402;     MlfhzXZKDBTPVARrP39267402 = MlfhzXZKDBTPVARrP11100123;     MlfhzXZKDBTPVARrP11100123 = MlfhzXZKDBTPVARrP96518322;     MlfhzXZKDBTPVARrP96518322 = MlfhzXZKDBTPVARrP96479837;     MlfhzXZKDBTPVARrP96479837 = MlfhzXZKDBTPVARrP75612956;     MlfhzXZKDBTPVARrP75612956 = MlfhzXZKDBTPVARrP2117887;     MlfhzXZKDBTPVARrP2117887 = MlfhzXZKDBTPVARrP86197651;     MlfhzXZKDBTPVARrP86197651 = MlfhzXZKDBTPVARrP52693668;     MlfhzXZKDBTPVARrP52693668 = MlfhzXZKDBTPVARrP89050203;     MlfhzXZKDBTPVARrP89050203 = MlfhzXZKDBTPVARrP93469190;     MlfhzXZKDBTPVARrP93469190 = MlfhzXZKDBTPVARrP65205526;     MlfhzXZKDBTPVARrP65205526 = MlfhzXZKDBTPVARrP35312358;     MlfhzXZKDBTPVARrP35312358 = MlfhzXZKDBTPVARrP24177118;     MlfhzXZKDBTPVARrP24177118 = MlfhzXZKDBTPVARrP70979482;     MlfhzXZKDBTPVARrP70979482 = MlfhzXZKDBTPVARrP84191600;     MlfhzXZKDBTPVARrP84191600 = MlfhzXZKDBTPVARrP68379016;     MlfhzXZKDBTPVARrP68379016 = MlfhzXZKDBTPVARrP30335891;     MlfhzXZKDBTPVARrP30335891 = MlfhzXZKDBTPVARrP54240364;     MlfhzXZKDBTPVARrP54240364 = MlfhzXZKDBTPVARrP38900141;     MlfhzXZKDBTPVARrP38900141 = MlfhzXZKDBTPVARrP40799076;     MlfhzXZKDBTPVARrP40799076 = MlfhzXZKDBTPVARrP34691550;     MlfhzXZKDBTPVARrP34691550 = MlfhzXZKDBTPVARrP89343365;     MlfhzXZKDBTPVARrP89343365 = MlfhzXZKDBTPVARrP85529032;     MlfhzXZKDBTPVARrP85529032 = MlfhzXZKDBTPVARrP19488528;     MlfhzXZKDBTPVARrP19488528 = MlfhzXZKDBTPVARrP34607132;     MlfhzXZKDBTPVARrP34607132 = MlfhzXZKDBTPVARrP1255487;     MlfhzXZKDBTPVARrP1255487 = MlfhzXZKDBTPVARrP46076143;     MlfhzXZKDBTPVARrP46076143 = MlfhzXZKDBTPVARrP8710302;     MlfhzXZKDBTPVARrP8710302 = MlfhzXZKDBTPVARrP96568803;     MlfhzXZKDBTPVARrP96568803 = MlfhzXZKDBTPVARrP61137831;     MlfhzXZKDBTPVARrP61137831 = MlfhzXZKDBTPVARrP46598264;     MlfhzXZKDBTPVARrP46598264 = MlfhzXZKDBTPVARrP2634584;     MlfhzXZKDBTPVARrP2634584 = MlfhzXZKDBTPVARrP52058547;     MlfhzXZKDBTPVARrP52058547 = MlfhzXZKDBTPVARrP46905301;     MlfhzXZKDBTPVARrP46905301 = MlfhzXZKDBTPVARrP80973506;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void xbBOuiAIQmeAISUr86785824() {     double LFwaQwoSWKhBapzzN16432475 = 51153757;    double LFwaQwoSWKhBapzzN76183870 = -667896999;    double LFwaQwoSWKhBapzzN4513750 = -56275136;    double LFwaQwoSWKhBapzzN12080591 = -214831580;    double LFwaQwoSWKhBapzzN7954331 = -691349674;    double LFwaQwoSWKhBapzzN82863388 = -545872048;    double LFwaQwoSWKhBapzzN64126727 = -630647540;    double LFwaQwoSWKhBapzzN49779769 = -517008445;    double LFwaQwoSWKhBapzzN14294020 = 23999613;    double LFwaQwoSWKhBapzzN73774090 = 59351786;    double LFwaQwoSWKhBapzzN46897084 = -347667571;    double LFwaQwoSWKhBapzzN55713758 = -558149919;    double LFwaQwoSWKhBapzzN94154698 = -868873160;    double LFwaQwoSWKhBapzzN75656891 = -748854501;    double LFwaQwoSWKhBapzzN52409376 = -922036515;    double LFwaQwoSWKhBapzzN46690438 = 1802221;    double LFwaQwoSWKhBapzzN95692075 = 26200682;    double LFwaQwoSWKhBapzzN31713246 = -348577486;    double LFwaQwoSWKhBapzzN76156710 = -571598569;    double LFwaQwoSWKhBapzzN15558892 = -638807939;    double LFwaQwoSWKhBapzzN73290189 = -931475989;    double LFwaQwoSWKhBapzzN77885853 = -537471892;    double LFwaQwoSWKhBapzzN70309281 = -356350194;    double LFwaQwoSWKhBapzzN74639523 = -663496857;    double LFwaQwoSWKhBapzzN56213077 = -343385432;    double LFwaQwoSWKhBapzzN25322132 = -970644637;    double LFwaQwoSWKhBapzzN80717180 = -189824955;    double LFwaQwoSWKhBapzzN22664107 = -317411325;    double LFwaQwoSWKhBapzzN14180551 = -956721293;    double LFwaQwoSWKhBapzzN11685607 = -727471075;    double LFwaQwoSWKhBapzzN65041613 = 63599706;    double LFwaQwoSWKhBapzzN36774209 = -84691041;    double LFwaQwoSWKhBapzzN28096218 = -231804055;    double LFwaQwoSWKhBapzzN74568357 = -214703837;    double LFwaQwoSWKhBapzzN91806158 = -587756605;    double LFwaQwoSWKhBapzzN49295865 = -501112306;    double LFwaQwoSWKhBapzzN36947997 = -721918802;    double LFwaQwoSWKhBapzzN88953656 = -155280278;    double LFwaQwoSWKhBapzzN76145964 = -468481445;    double LFwaQwoSWKhBapzzN87184245 = -753836841;    double LFwaQwoSWKhBapzzN66123078 = -534487272;    double LFwaQwoSWKhBapzzN56099262 = -340163937;    double LFwaQwoSWKhBapzzN98642180 = 17265216;    double LFwaQwoSWKhBapzzN4602882 = -287551839;    double LFwaQwoSWKhBapzzN56685919 = 33522482;    double LFwaQwoSWKhBapzzN23176973 = -964216038;    double LFwaQwoSWKhBapzzN49777523 = 46684643;    double LFwaQwoSWKhBapzzN69643902 = -887197935;    double LFwaQwoSWKhBapzzN74492457 = -49312064;    double LFwaQwoSWKhBapzzN37801562 = 33616191;    double LFwaQwoSWKhBapzzN48022779 = -654935653;    double LFwaQwoSWKhBapzzN89144778 = -325922025;    double LFwaQwoSWKhBapzzN32880789 = -372029989;    double LFwaQwoSWKhBapzzN42526688 = -306617614;    double LFwaQwoSWKhBapzzN13495635 = -556330531;    double LFwaQwoSWKhBapzzN38546621 = -411374352;    double LFwaQwoSWKhBapzzN5874589 = -211546805;    double LFwaQwoSWKhBapzzN29874226 = -392778280;    double LFwaQwoSWKhBapzzN55867514 = -871446148;    double LFwaQwoSWKhBapzzN82632198 = -720705038;    double LFwaQwoSWKhBapzzN2146208 = -256047093;    double LFwaQwoSWKhBapzzN41462621 = -213236215;    double LFwaQwoSWKhBapzzN35599218 = -560287152;    double LFwaQwoSWKhBapzzN2608414 = -248529313;    double LFwaQwoSWKhBapzzN8732478 = 95752080;    double LFwaQwoSWKhBapzzN10122876 = -162976531;    double LFwaQwoSWKhBapzzN27617540 = -226345864;    double LFwaQwoSWKhBapzzN19586342 = -554169323;    double LFwaQwoSWKhBapzzN83850732 = -61097897;    double LFwaQwoSWKhBapzzN3113511 = -320924210;    double LFwaQwoSWKhBapzzN9742441 = -276278977;    double LFwaQwoSWKhBapzzN6738420 = -818519040;    double LFwaQwoSWKhBapzzN55567282 = -880096042;    double LFwaQwoSWKhBapzzN88972465 = -817761729;    double LFwaQwoSWKhBapzzN49435814 = -4320667;    double LFwaQwoSWKhBapzzN17190927 = -491312052;    double LFwaQwoSWKhBapzzN79243673 = -454737108;    double LFwaQwoSWKhBapzzN65706399 = 31201644;    double LFwaQwoSWKhBapzzN17953604 = -597019339;    double LFwaQwoSWKhBapzzN33036105 = -379169394;    double LFwaQwoSWKhBapzzN75544609 = -917329280;    double LFwaQwoSWKhBapzzN11073279 = -302627021;    double LFwaQwoSWKhBapzzN48171649 = -168099262;    double LFwaQwoSWKhBapzzN76378989 = -890337485;    double LFwaQwoSWKhBapzzN63662828 = 27464578;    double LFwaQwoSWKhBapzzN75896835 = -610478270;    double LFwaQwoSWKhBapzzN3893420 = -712661052;    double LFwaQwoSWKhBapzzN85569530 = -925186442;    double LFwaQwoSWKhBapzzN61072722 = -658373306;    double LFwaQwoSWKhBapzzN53259537 = -76382254;    double LFwaQwoSWKhBapzzN43421277 = -189565501;    double LFwaQwoSWKhBapzzN7073772 = -229140523;    double LFwaQwoSWKhBapzzN33086142 = -283834130;    double LFwaQwoSWKhBapzzN93513765 = -747776408;    double LFwaQwoSWKhBapzzN85038037 = -397789748;    double LFwaQwoSWKhBapzzN24660458 = -221251057;    double LFwaQwoSWKhBapzzN20500045 = -779876786;    double LFwaQwoSWKhBapzzN96033767 = -734205472;    double LFwaQwoSWKhBapzzN95870404 = -283303920;    double LFwaQwoSWKhBapzzN46563044 = 51153757;     LFwaQwoSWKhBapzzN16432475 = LFwaQwoSWKhBapzzN76183870;     LFwaQwoSWKhBapzzN76183870 = LFwaQwoSWKhBapzzN4513750;     LFwaQwoSWKhBapzzN4513750 = LFwaQwoSWKhBapzzN12080591;     LFwaQwoSWKhBapzzN12080591 = LFwaQwoSWKhBapzzN7954331;     LFwaQwoSWKhBapzzN7954331 = LFwaQwoSWKhBapzzN82863388;     LFwaQwoSWKhBapzzN82863388 = LFwaQwoSWKhBapzzN64126727;     LFwaQwoSWKhBapzzN64126727 = LFwaQwoSWKhBapzzN49779769;     LFwaQwoSWKhBapzzN49779769 = LFwaQwoSWKhBapzzN14294020;     LFwaQwoSWKhBapzzN14294020 = LFwaQwoSWKhBapzzN73774090;     LFwaQwoSWKhBapzzN73774090 = LFwaQwoSWKhBapzzN46897084;     LFwaQwoSWKhBapzzN46897084 = LFwaQwoSWKhBapzzN55713758;     LFwaQwoSWKhBapzzN55713758 = LFwaQwoSWKhBapzzN94154698;     LFwaQwoSWKhBapzzN94154698 = LFwaQwoSWKhBapzzN75656891;     LFwaQwoSWKhBapzzN75656891 = LFwaQwoSWKhBapzzN52409376;     LFwaQwoSWKhBapzzN52409376 = LFwaQwoSWKhBapzzN46690438;     LFwaQwoSWKhBapzzN46690438 = LFwaQwoSWKhBapzzN95692075;     LFwaQwoSWKhBapzzN95692075 = LFwaQwoSWKhBapzzN31713246;     LFwaQwoSWKhBapzzN31713246 = LFwaQwoSWKhBapzzN76156710;     LFwaQwoSWKhBapzzN76156710 = LFwaQwoSWKhBapzzN15558892;     LFwaQwoSWKhBapzzN15558892 = LFwaQwoSWKhBapzzN73290189;     LFwaQwoSWKhBapzzN73290189 = LFwaQwoSWKhBapzzN77885853;     LFwaQwoSWKhBapzzN77885853 = LFwaQwoSWKhBapzzN70309281;     LFwaQwoSWKhBapzzN70309281 = LFwaQwoSWKhBapzzN74639523;     LFwaQwoSWKhBapzzN74639523 = LFwaQwoSWKhBapzzN56213077;     LFwaQwoSWKhBapzzN56213077 = LFwaQwoSWKhBapzzN25322132;     LFwaQwoSWKhBapzzN25322132 = LFwaQwoSWKhBapzzN80717180;     LFwaQwoSWKhBapzzN80717180 = LFwaQwoSWKhBapzzN22664107;     LFwaQwoSWKhBapzzN22664107 = LFwaQwoSWKhBapzzN14180551;     LFwaQwoSWKhBapzzN14180551 = LFwaQwoSWKhBapzzN11685607;     LFwaQwoSWKhBapzzN11685607 = LFwaQwoSWKhBapzzN65041613;     LFwaQwoSWKhBapzzN65041613 = LFwaQwoSWKhBapzzN36774209;     LFwaQwoSWKhBapzzN36774209 = LFwaQwoSWKhBapzzN28096218;     LFwaQwoSWKhBapzzN28096218 = LFwaQwoSWKhBapzzN74568357;     LFwaQwoSWKhBapzzN74568357 = LFwaQwoSWKhBapzzN91806158;     LFwaQwoSWKhBapzzN91806158 = LFwaQwoSWKhBapzzN49295865;     LFwaQwoSWKhBapzzN49295865 = LFwaQwoSWKhBapzzN36947997;     LFwaQwoSWKhBapzzN36947997 = LFwaQwoSWKhBapzzN88953656;     LFwaQwoSWKhBapzzN88953656 = LFwaQwoSWKhBapzzN76145964;     LFwaQwoSWKhBapzzN76145964 = LFwaQwoSWKhBapzzN87184245;     LFwaQwoSWKhBapzzN87184245 = LFwaQwoSWKhBapzzN66123078;     LFwaQwoSWKhBapzzN66123078 = LFwaQwoSWKhBapzzN56099262;     LFwaQwoSWKhBapzzN56099262 = LFwaQwoSWKhBapzzN98642180;     LFwaQwoSWKhBapzzN98642180 = LFwaQwoSWKhBapzzN4602882;     LFwaQwoSWKhBapzzN4602882 = LFwaQwoSWKhBapzzN56685919;     LFwaQwoSWKhBapzzN56685919 = LFwaQwoSWKhBapzzN23176973;     LFwaQwoSWKhBapzzN23176973 = LFwaQwoSWKhBapzzN49777523;     LFwaQwoSWKhBapzzN49777523 = LFwaQwoSWKhBapzzN69643902;     LFwaQwoSWKhBapzzN69643902 = LFwaQwoSWKhBapzzN74492457;     LFwaQwoSWKhBapzzN74492457 = LFwaQwoSWKhBapzzN37801562;     LFwaQwoSWKhBapzzN37801562 = LFwaQwoSWKhBapzzN48022779;     LFwaQwoSWKhBapzzN48022779 = LFwaQwoSWKhBapzzN89144778;     LFwaQwoSWKhBapzzN89144778 = LFwaQwoSWKhBapzzN32880789;     LFwaQwoSWKhBapzzN32880789 = LFwaQwoSWKhBapzzN42526688;     LFwaQwoSWKhBapzzN42526688 = LFwaQwoSWKhBapzzN13495635;     LFwaQwoSWKhBapzzN13495635 = LFwaQwoSWKhBapzzN38546621;     LFwaQwoSWKhBapzzN38546621 = LFwaQwoSWKhBapzzN5874589;     LFwaQwoSWKhBapzzN5874589 = LFwaQwoSWKhBapzzN29874226;     LFwaQwoSWKhBapzzN29874226 = LFwaQwoSWKhBapzzN55867514;     LFwaQwoSWKhBapzzN55867514 = LFwaQwoSWKhBapzzN82632198;     LFwaQwoSWKhBapzzN82632198 = LFwaQwoSWKhBapzzN2146208;     LFwaQwoSWKhBapzzN2146208 = LFwaQwoSWKhBapzzN41462621;     LFwaQwoSWKhBapzzN41462621 = LFwaQwoSWKhBapzzN35599218;     LFwaQwoSWKhBapzzN35599218 = LFwaQwoSWKhBapzzN2608414;     LFwaQwoSWKhBapzzN2608414 = LFwaQwoSWKhBapzzN8732478;     LFwaQwoSWKhBapzzN8732478 = LFwaQwoSWKhBapzzN10122876;     LFwaQwoSWKhBapzzN10122876 = LFwaQwoSWKhBapzzN27617540;     LFwaQwoSWKhBapzzN27617540 = LFwaQwoSWKhBapzzN19586342;     LFwaQwoSWKhBapzzN19586342 = LFwaQwoSWKhBapzzN83850732;     LFwaQwoSWKhBapzzN83850732 = LFwaQwoSWKhBapzzN3113511;     LFwaQwoSWKhBapzzN3113511 = LFwaQwoSWKhBapzzN9742441;     LFwaQwoSWKhBapzzN9742441 = LFwaQwoSWKhBapzzN6738420;     LFwaQwoSWKhBapzzN6738420 = LFwaQwoSWKhBapzzN55567282;     LFwaQwoSWKhBapzzN55567282 = LFwaQwoSWKhBapzzN88972465;     LFwaQwoSWKhBapzzN88972465 = LFwaQwoSWKhBapzzN49435814;     LFwaQwoSWKhBapzzN49435814 = LFwaQwoSWKhBapzzN17190927;     LFwaQwoSWKhBapzzN17190927 = LFwaQwoSWKhBapzzN79243673;     LFwaQwoSWKhBapzzN79243673 = LFwaQwoSWKhBapzzN65706399;     LFwaQwoSWKhBapzzN65706399 = LFwaQwoSWKhBapzzN17953604;     LFwaQwoSWKhBapzzN17953604 = LFwaQwoSWKhBapzzN33036105;     LFwaQwoSWKhBapzzN33036105 = LFwaQwoSWKhBapzzN75544609;     LFwaQwoSWKhBapzzN75544609 = LFwaQwoSWKhBapzzN11073279;     LFwaQwoSWKhBapzzN11073279 = LFwaQwoSWKhBapzzN48171649;     LFwaQwoSWKhBapzzN48171649 = LFwaQwoSWKhBapzzN76378989;     LFwaQwoSWKhBapzzN76378989 = LFwaQwoSWKhBapzzN63662828;     LFwaQwoSWKhBapzzN63662828 = LFwaQwoSWKhBapzzN75896835;     LFwaQwoSWKhBapzzN75896835 = LFwaQwoSWKhBapzzN3893420;     LFwaQwoSWKhBapzzN3893420 = LFwaQwoSWKhBapzzN85569530;     LFwaQwoSWKhBapzzN85569530 = LFwaQwoSWKhBapzzN61072722;     LFwaQwoSWKhBapzzN61072722 = LFwaQwoSWKhBapzzN53259537;     LFwaQwoSWKhBapzzN53259537 = LFwaQwoSWKhBapzzN43421277;     LFwaQwoSWKhBapzzN43421277 = LFwaQwoSWKhBapzzN7073772;     LFwaQwoSWKhBapzzN7073772 = LFwaQwoSWKhBapzzN33086142;     LFwaQwoSWKhBapzzN33086142 = LFwaQwoSWKhBapzzN93513765;     LFwaQwoSWKhBapzzN93513765 = LFwaQwoSWKhBapzzN85038037;     LFwaQwoSWKhBapzzN85038037 = LFwaQwoSWKhBapzzN24660458;     LFwaQwoSWKhBapzzN24660458 = LFwaQwoSWKhBapzzN20500045;     LFwaQwoSWKhBapzzN20500045 = LFwaQwoSWKhBapzzN96033767;     LFwaQwoSWKhBapzzN96033767 = LFwaQwoSWKhBapzzN95870404;     LFwaQwoSWKhBapzzN95870404 = LFwaQwoSWKhBapzzN46563044;     LFwaQwoSWKhBapzzN46563044 = LFwaQwoSWKhBapzzN16432475;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void HTKrMUoskUlzNDwA1834892() {     double sbBjCUtyCxoipqhpW22549582 = -60748132;    double sbBjCUtyCxoipqhpW19557150 = -377496779;    double sbBjCUtyCxoipqhpW20555933 = -207108057;    double sbBjCUtyCxoipqhpW39155372 = -405580210;    double sbBjCUtyCxoipqhpW76849819 = -111999545;    double sbBjCUtyCxoipqhpW74280590 = -810368283;    double sbBjCUtyCxoipqhpW24615694 = -769612517;    double sbBjCUtyCxoipqhpW99539659 = -803882358;    double sbBjCUtyCxoipqhpW78936515 = -844033053;    double sbBjCUtyCxoipqhpW15003750 = -269623691;    double sbBjCUtyCxoipqhpW7007375 = -225105158;    double sbBjCUtyCxoipqhpW87536800 = 97443879;    double sbBjCUtyCxoipqhpW80363817 = -208764248;    double sbBjCUtyCxoipqhpW58800567 = -181093069;    double sbBjCUtyCxoipqhpW48151802 = -512498682;    double sbBjCUtyCxoipqhpW88195832 = 28802678;    double sbBjCUtyCxoipqhpW89528078 = -590344430;    double sbBjCUtyCxoipqhpW28766399 = -14100129;    double sbBjCUtyCxoipqhpW37986552 = -977367666;    double sbBjCUtyCxoipqhpW96109056 = -995710858;    double sbBjCUtyCxoipqhpW61843382 = -786963444;    double sbBjCUtyCxoipqhpW64406877 = -445401608;    double sbBjCUtyCxoipqhpW5900073 = -102867681;    double sbBjCUtyCxoipqhpW58631897 = -885801844;    double sbBjCUtyCxoipqhpW99912727 = -716581465;    double sbBjCUtyCxoipqhpW50043807 = -849299960;    double sbBjCUtyCxoipqhpW79176958 = -90487158;    double sbBjCUtyCxoipqhpW35771232 = -383340279;    double sbBjCUtyCxoipqhpW80312806 = -79103998;    double sbBjCUtyCxoipqhpW80301243 = -544941942;    double sbBjCUtyCxoipqhpW57284163 = -949721233;    double sbBjCUtyCxoipqhpW34384831 = -894137856;    double sbBjCUtyCxoipqhpW37807167 = -124417425;    double sbBjCUtyCxoipqhpW69398921 = 33754398;    double sbBjCUtyCxoipqhpW53280810 = -126489711;    double sbBjCUtyCxoipqhpW18021779 = -715138413;    double sbBjCUtyCxoipqhpW18209932 = -675554672;    double sbBjCUtyCxoipqhpW23043869 = 73213713;    double sbBjCUtyCxoipqhpW49756218 = -462404941;    double sbBjCUtyCxoipqhpW74686183 = -127276064;    double sbBjCUtyCxoipqhpW71792302 = -599761166;    double sbBjCUtyCxoipqhpW88249719 = -610306216;    double sbBjCUtyCxoipqhpW76709159 = -63539368;    double sbBjCUtyCxoipqhpW92010622 = -306894777;    double sbBjCUtyCxoipqhpW68638384 = -728375673;    double sbBjCUtyCxoipqhpW35116168 = -949432154;    double sbBjCUtyCxoipqhpW67556097 = -72090894;    double sbBjCUtyCxoipqhpW77600842 = -897093467;    double sbBjCUtyCxoipqhpW84168186 = -135049052;    double sbBjCUtyCxoipqhpW50713214 = -53394617;    double sbBjCUtyCxoipqhpW83723457 = -497229324;    double sbBjCUtyCxoipqhpW99954497 = -126648345;    double sbBjCUtyCxoipqhpW81129246 = -163819577;    double sbBjCUtyCxoipqhpW38495569 = -627087759;    double sbBjCUtyCxoipqhpW39991510 = -156862314;    double sbBjCUtyCxoipqhpW58142704 = -615346524;    double sbBjCUtyCxoipqhpW13657077 = -174629099;    double sbBjCUtyCxoipqhpW61924036 = -321306213;    double sbBjCUtyCxoipqhpW39242645 = -688998745;    double sbBjCUtyCxoipqhpW26806012 = -262699586;    double sbBjCUtyCxoipqhpW95103631 = -619881125;    double sbBjCUtyCxoipqhpW88844461 = -286272239;    double sbBjCUtyCxoipqhpW19226854 = -624778361;    double sbBjCUtyCxoipqhpW98635271 = -199091111;    double sbBjCUtyCxoipqhpW57719587 = -319902459;    double sbBjCUtyCxoipqhpW72622544 = -330967302;    double sbBjCUtyCxoipqhpW49729634 = -778138696;    double sbBjCUtyCxoipqhpW10964896 = -142518647;    double sbBjCUtyCxoipqhpW5519757 = 45396641;    double sbBjCUtyCxoipqhpW30130024 = -797360269;    double sbBjCUtyCxoipqhpW69985900 = -295642651;    double sbBjCUtyCxoipqhpW66484210 = -563558144;    double sbBjCUtyCxoipqhpW79010181 = -551695188;    double sbBjCUtyCxoipqhpW63300369 = -750091603;    double sbBjCUtyCxoipqhpW24316755 = -295949692;    double sbBjCUtyCxoipqhpW73593663 = -76657229;    double sbBjCUtyCxoipqhpW87697718 = -281862241;    double sbBjCUtyCxoipqhpW13889451 = -795972905;    double sbBjCUtyCxoipqhpW89993513 = -57426172;    double sbBjCUtyCxoipqhpW64796560 = -767149311;    double sbBjCUtyCxoipqhpW82487710 = -677209066;    double sbBjCUtyCxoipqhpW1576116 = -193393691;    double sbBjCUtyCxoipqhpW51603046 = -148291228;    double sbBjCUtyCxoipqhpW29599592 = 74290619;    double sbBjCUtyCxoipqhpW96577786 = 52287382;    double sbBjCUtyCxoipqhpW57329665 = -723072888;    double sbBjCUtyCxoipqhpW53255584 = -630318280;    double sbBjCUtyCxoipqhpW99311598 = -497329667;    double sbBjCUtyCxoipqhpW29407412 = -809383289;    double sbBjCUtyCxoipqhpW95138106 = -511143187;    double sbBjCUtyCxoipqhpW4364703 = -440509315;    double sbBjCUtyCxoipqhpW56285896 = -254248459;    double sbBjCUtyCxoipqhpW83801224 = -237787542;    double sbBjCUtyCxoipqhpW22950206 = -99705356;    double sbBjCUtyCxoipqhpW79582551 = -507394939;    double sbBjCUtyCxoipqhpW82947840 = -213488928;    double sbBjCUtyCxoipqhpW69022866 = -985527855;    double sbBjCUtyCxoipqhpW78073887 = -864448257;    double sbBjCUtyCxoipqhpW34291035 = -986992318;    double sbBjCUtyCxoipqhpW96015840 = -60748132;     sbBjCUtyCxoipqhpW22549582 = sbBjCUtyCxoipqhpW19557150;     sbBjCUtyCxoipqhpW19557150 = sbBjCUtyCxoipqhpW20555933;     sbBjCUtyCxoipqhpW20555933 = sbBjCUtyCxoipqhpW39155372;     sbBjCUtyCxoipqhpW39155372 = sbBjCUtyCxoipqhpW76849819;     sbBjCUtyCxoipqhpW76849819 = sbBjCUtyCxoipqhpW74280590;     sbBjCUtyCxoipqhpW74280590 = sbBjCUtyCxoipqhpW24615694;     sbBjCUtyCxoipqhpW24615694 = sbBjCUtyCxoipqhpW99539659;     sbBjCUtyCxoipqhpW99539659 = sbBjCUtyCxoipqhpW78936515;     sbBjCUtyCxoipqhpW78936515 = sbBjCUtyCxoipqhpW15003750;     sbBjCUtyCxoipqhpW15003750 = sbBjCUtyCxoipqhpW7007375;     sbBjCUtyCxoipqhpW7007375 = sbBjCUtyCxoipqhpW87536800;     sbBjCUtyCxoipqhpW87536800 = sbBjCUtyCxoipqhpW80363817;     sbBjCUtyCxoipqhpW80363817 = sbBjCUtyCxoipqhpW58800567;     sbBjCUtyCxoipqhpW58800567 = sbBjCUtyCxoipqhpW48151802;     sbBjCUtyCxoipqhpW48151802 = sbBjCUtyCxoipqhpW88195832;     sbBjCUtyCxoipqhpW88195832 = sbBjCUtyCxoipqhpW89528078;     sbBjCUtyCxoipqhpW89528078 = sbBjCUtyCxoipqhpW28766399;     sbBjCUtyCxoipqhpW28766399 = sbBjCUtyCxoipqhpW37986552;     sbBjCUtyCxoipqhpW37986552 = sbBjCUtyCxoipqhpW96109056;     sbBjCUtyCxoipqhpW96109056 = sbBjCUtyCxoipqhpW61843382;     sbBjCUtyCxoipqhpW61843382 = sbBjCUtyCxoipqhpW64406877;     sbBjCUtyCxoipqhpW64406877 = sbBjCUtyCxoipqhpW5900073;     sbBjCUtyCxoipqhpW5900073 = sbBjCUtyCxoipqhpW58631897;     sbBjCUtyCxoipqhpW58631897 = sbBjCUtyCxoipqhpW99912727;     sbBjCUtyCxoipqhpW99912727 = sbBjCUtyCxoipqhpW50043807;     sbBjCUtyCxoipqhpW50043807 = sbBjCUtyCxoipqhpW79176958;     sbBjCUtyCxoipqhpW79176958 = sbBjCUtyCxoipqhpW35771232;     sbBjCUtyCxoipqhpW35771232 = sbBjCUtyCxoipqhpW80312806;     sbBjCUtyCxoipqhpW80312806 = sbBjCUtyCxoipqhpW80301243;     sbBjCUtyCxoipqhpW80301243 = sbBjCUtyCxoipqhpW57284163;     sbBjCUtyCxoipqhpW57284163 = sbBjCUtyCxoipqhpW34384831;     sbBjCUtyCxoipqhpW34384831 = sbBjCUtyCxoipqhpW37807167;     sbBjCUtyCxoipqhpW37807167 = sbBjCUtyCxoipqhpW69398921;     sbBjCUtyCxoipqhpW69398921 = sbBjCUtyCxoipqhpW53280810;     sbBjCUtyCxoipqhpW53280810 = sbBjCUtyCxoipqhpW18021779;     sbBjCUtyCxoipqhpW18021779 = sbBjCUtyCxoipqhpW18209932;     sbBjCUtyCxoipqhpW18209932 = sbBjCUtyCxoipqhpW23043869;     sbBjCUtyCxoipqhpW23043869 = sbBjCUtyCxoipqhpW49756218;     sbBjCUtyCxoipqhpW49756218 = sbBjCUtyCxoipqhpW74686183;     sbBjCUtyCxoipqhpW74686183 = sbBjCUtyCxoipqhpW71792302;     sbBjCUtyCxoipqhpW71792302 = sbBjCUtyCxoipqhpW88249719;     sbBjCUtyCxoipqhpW88249719 = sbBjCUtyCxoipqhpW76709159;     sbBjCUtyCxoipqhpW76709159 = sbBjCUtyCxoipqhpW92010622;     sbBjCUtyCxoipqhpW92010622 = sbBjCUtyCxoipqhpW68638384;     sbBjCUtyCxoipqhpW68638384 = sbBjCUtyCxoipqhpW35116168;     sbBjCUtyCxoipqhpW35116168 = sbBjCUtyCxoipqhpW67556097;     sbBjCUtyCxoipqhpW67556097 = sbBjCUtyCxoipqhpW77600842;     sbBjCUtyCxoipqhpW77600842 = sbBjCUtyCxoipqhpW84168186;     sbBjCUtyCxoipqhpW84168186 = sbBjCUtyCxoipqhpW50713214;     sbBjCUtyCxoipqhpW50713214 = sbBjCUtyCxoipqhpW83723457;     sbBjCUtyCxoipqhpW83723457 = sbBjCUtyCxoipqhpW99954497;     sbBjCUtyCxoipqhpW99954497 = sbBjCUtyCxoipqhpW81129246;     sbBjCUtyCxoipqhpW81129246 = sbBjCUtyCxoipqhpW38495569;     sbBjCUtyCxoipqhpW38495569 = sbBjCUtyCxoipqhpW39991510;     sbBjCUtyCxoipqhpW39991510 = sbBjCUtyCxoipqhpW58142704;     sbBjCUtyCxoipqhpW58142704 = sbBjCUtyCxoipqhpW13657077;     sbBjCUtyCxoipqhpW13657077 = sbBjCUtyCxoipqhpW61924036;     sbBjCUtyCxoipqhpW61924036 = sbBjCUtyCxoipqhpW39242645;     sbBjCUtyCxoipqhpW39242645 = sbBjCUtyCxoipqhpW26806012;     sbBjCUtyCxoipqhpW26806012 = sbBjCUtyCxoipqhpW95103631;     sbBjCUtyCxoipqhpW95103631 = sbBjCUtyCxoipqhpW88844461;     sbBjCUtyCxoipqhpW88844461 = sbBjCUtyCxoipqhpW19226854;     sbBjCUtyCxoipqhpW19226854 = sbBjCUtyCxoipqhpW98635271;     sbBjCUtyCxoipqhpW98635271 = sbBjCUtyCxoipqhpW57719587;     sbBjCUtyCxoipqhpW57719587 = sbBjCUtyCxoipqhpW72622544;     sbBjCUtyCxoipqhpW72622544 = sbBjCUtyCxoipqhpW49729634;     sbBjCUtyCxoipqhpW49729634 = sbBjCUtyCxoipqhpW10964896;     sbBjCUtyCxoipqhpW10964896 = sbBjCUtyCxoipqhpW5519757;     sbBjCUtyCxoipqhpW5519757 = sbBjCUtyCxoipqhpW30130024;     sbBjCUtyCxoipqhpW30130024 = sbBjCUtyCxoipqhpW69985900;     sbBjCUtyCxoipqhpW69985900 = sbBjCUtyCxoipqhpW66484210;     sbBjCUtyCxoipqhpW66484210 = sbBjCUtyCxoipqhpW79010181;     sbBjCUtyCxoipqhpW79010181 = sbBjCUtyCxoipqhpW63300369;     sbBjCUtyCxoipqhpW63300369 = sbBjCUtyCxoipqhpW24316755;     sbBjCUtyCxoipqhpW24316755 = sbBjCUtyCxoipqhpW73593663;     sbBjCUtyCxoipqhpW73593663 = sbBjCUtyCxoipqhpW87697718;     sbBjCUtyCxoipqhpW87697718 = sbBjCUtyCxoipqhpW13889451;     sbBjCUtyCxoipqhpW13889451 = sbBjCUtyCxoipqhpW89993513;     sbBjCUtyCxoipqhpW89993513 = sbBjCUtyCxoipqhpW64796560;     sbBjCUtyCxoipqhpW64796560 = sbBjCUtyCxoipqhpW82487710;     sbBjCUtyCxoipqhpW82487710 = sbBjCUtyCxoipqhpW1576116;     sbBjCUtyCxoipqhpW1576116 = sbBjCUtyCxoipqhpW51603046;     sbBjCUtyCxoipqhpW51603046 = sbBjCUtyCxoipqhpW29599592;     sbBjCUtyCxoipqhpW29599592 = sbBjCUtyCxoipqhpW96577786;     sbBjCUtyCxoipqhpW96577786 = sbBjCUtyCxoipqhpW57329665;     sbBjCUtyCxoipqhpW57329665 = sbBjCUtyCxoipqhpW53255584;     sbBjCUtyCxoipqhpW53255584 = sbBjCUtyCxoipqhpW99311598;     sbBjCUtyCxoipqhpW99311598 = sbBjCUtyCxoipqhpW29407412;     sbBjCUtyCxoipqhpW29407412 = sbBjCUtyCxoipqhpW95138106;     sbBjCUtyCxoipqhpW95138106 = sbBjCUtyCxoipqhpW4364703;     sbBjCUtyCxoipqhpW4364703 = sbBjCUtyCxoipqhpW56285896;     sbBjCUtyCxoipqhpW56285896 = sbBjCUtyCxoipqhpW83801224;     sbBjCUtyCxoipqhpW83801224 = sbBjCUtyCxoipqhpW22950206;     sbBjCUtyCxoipqhpW22950206 = sbBjCUtyCxoipqhpW79582551;     sbBjCUtyCxoipqhpW79582551 = sbBjCUtyCxoipqhpW82947840;     sbBjCUtyCxoipqhpW82947840 = sbBjCUtyCxoipqhpW69022866;     sbBjCUtyCxoipqhpW69022866 = sbBjCUtyCxoipqhpW78073887;     sbBjCUtyCxoipqhpW78073887 = sbBjCUtyCxoipqhpW34291035;     sbBjCUtyCxoipqhpW34291035 = sbBjCUtyCxoipqhpW96015840;     sbBjCUtyCxoipqhpW96015840 = sbBjCUtyCxoipqhpW22549582;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void leXajougjjaBpwNd69126491() {     double klJXEgNifzdItmyUy58008550 = -527025032;    double klJXEgNifzdItmyUy32829228 = -258048386;    double klJXEgNifzdItmyUy57942410 = -972775132;    double klJXEgNifzdItmyUy11378727 = -226731281;    double klJXEgNifzdItmyUy33135088 = -217598390;    double klJXEgNifzdItmyUy10503463 = 27283822;    double klJXEgNifzdItmyUy35273100 = -614338252;    double klJXEgNifzdItmyUy61671220 = -448116499;    double klJXEgNifzdItmyUy80454120 = -232364460;    double klJXEgNifzdItmyUy29906462 = 65689311;    double klJXEgNifzdItmyUy45189109 = -27112887;    double klJXEgNifzdItmyUy88104949 = -866676892;    double klJXEgNifzdItmyUy18598463 = -760776753;    double klJXEgNifzdItmyUy11941431 = -157456478;    double klJXEgNifzdItmyUy6346319 = 84076303;    double klJXEgNifzdItmyUy71737133 = -707008929;    double klJXEgNifzdItmyUy45364014 = -17318983;    double klJXEgNifzdItmyUy28526590 = -87528198;    double klJXEgNifzdItmyUy38536882 = -193931087;    double klJXEgNifzdItmyUy89946275 = -322271924;    double klJXEgNifzdItmyUy74930245 = -444238133;    double klJXEgNifzdItmyUy67828402 = -532037902;    double klJXEgNifzdItmyUy76425452 = -840499865;    double klJXEgNifzdItmyUy52424842 = -296626449;    double klJXEgNifzdItmyUy7930679 = -740566663;    double klJXEgNifzdItmyUy60741979 = -537876767;    double klJXEgNifzdItmyUy39597533 = -996771298;    double klJXEgNifzdItmyUy76499657 = -553112453;    double klJXEgNifzdItmyUy26981021 = -653198800;    double klJXEgNifzdItmyUy85834654 = -579449113;    double klJXEgNifzdItmyUy75867394 = -401701720;    double klJXEgNifzdItmyUy72207849 = -667109271;    double klJXEgNifzdItmyUy50025177 = -272013403;    double klJXEgNifzdItmyUy99147348 = -30939415;    double klJXEgNifzdItmyUy19089264 = -77589961;    double klJXEgNifzdItmyUy69582622 = -929703719;    double klJXEgNifzdItmyUy67621748 = -540250712;    double klJXEgNifzdItmyUy74259272 = 90558132;    double klJXEgNifzdItmyUy80146779 = -113281218;    double klJXEgNifzdItmyUy38957715 = -692538610;    double klJXEgNifzdItmyUy5243910 = -368960081;    double klJXEgNifzdItmyUy77614846 = -204851507;    double klJXEgNifzdItmyUy66092537 = -716720154;    double klJXEgNifzdItmyUy32141961 = -911227187;    double klJXEgNifzdItmyUy68654843 = -554886834;    double klJXEgNifzdItmyUy81077496 = -988355787;    double klJXEgNifzdItmyUy86901261 = -556077255;    double klJXEgNifzdItmyUy95327154 = 36379940;    double klJXEgNifzdItmyUy7060853 = -898785932;    double klJXEgNifzdItmyUy75242803 = -544421433;    double klJXEgNifzdItmyUy64494180 = -371505882;    double klJXEgNifzdItmyUy83439970 = -340300691;    double klJXEgNifzdItmyUy49750395 = 80134869;    double klJXEgNifzdItmyUy54487415 = -818596259;    double klJXEgNifzdItmyUy94196245 = -503352460;    double klJXEgNifzdItmyUy90180147 = -994987130;    double klJXEgNifzdItmyUy56403775 = -417548521;    double klJXEgNifzdItmyUy5517568 = -576148684;    double klJXEgNifzdItmyUy3448049 = -486164618;    double klJXEgNifzdItmyUy72393109 = -679721624;    double klJXEgNifzdItmyUy70905930 = 24055120;    double klJXEgNifzdItmyUy58773443 = 38774201;    double klJXEgNifzdItmyUy34690200 = -794917699;    double klJXEgNifzdItmyUy94619466 = -652915348;    double klJXEgNifzdItmyUy54039068 = -532608970;    double klJXEgNifzdItmyUy72981260 = -360003617;    double klJXEgNifzdItmyUy38079772 = -494663490;    double klJXEgNifzdItmyUy19451115 = -629837338;    double klJXEgNifzdItmyUy92852166 = 20133482;    double klJXEgNifzdItmyUy36763696 = 13780022;    double klJXEgNifzdItmyUy4115385 = -66758217;    double klJXEgNifzdItmyUy71104742 = -7877115;    double klJXEgNifzdItmyUy48379811 = -974246980;    double klJXEgNifzdItmyUy99579167 = -501392478;    double klJXEgNifzdItmyUy84702365 = -953311844;    double klJXEgNifzdItmyUy97315399 = -139386627;    double klJXEgNifzdItmyUy1735865 = -815317749;    double klJXEgNifzdItmyUy44283492 = -929272679;    double klJXEgNifzdItmyUy83769999 = -741739615;    double klJXEgNifzdItmyUy26853183 = -752210877;    double klJXEgNifzdItmyUy73840718 = -981799512;    double klJXEgNifzdItmyUy44270378 = -933151239;    double klJXEgNifzdItmyUy69438804 = -654326522;    double klJXEgNifzdItmyUy51738218 = -8777368;    double klJXEgNifzdItmyUy21340474 = -107943231;    double klJXEgNifzdItmyUy92427424 = 38598971;    double klJXEgNifzdItmyUy22457454 = -647244140;    double klJXEgNifzdItmyUy95537762 = -453417145;    double klJXEgNifzdItmyUy4951103 = -527586956;    double klJXEgNifzdItmyUy28909117 = -82602832;    double klJXEgNifzdItmyUy13178848 = -412155198;    double klJXEgNifzdItmyUy62104181 = -964102029;    double klJXEgNifzdItmyUy70811224 = -423277251;    double klJXEgNifzdItmyUy7753671 = -433559594;    double klJXEgNifzdItmyUy68051785 = -616593730;    double klJXEgNifzdItmyUy46470467 = -307734282;    double klJXEgNifzdItmyUy42924647 = -409933808;    double klJXEgNifzdItmyUy71473071 = 36195193;    double klJXEgNifzdItmyUy78102892 = -278618217;    double klJXEgNifzdItmyUy95673582 = -527025032;     klJXEgNifzdItmyUy58008550 = klJXEgNifzdItmyUy32829228;     klJXEgNifzdItmyUy32829228 = klJXEgNifzdItmyUy57942410;     klJXEgNifzdItmyUy57942410 = klJXEgNifzdItmyUy11378727;     klJXEgNifzdItmyUy11378727 = klJXEgNifzdItmyUy33135088;     klJXEgNifzdItmyUy33135088 = klJXEgNifzdItmyUy10503463;     klJXEgNifzdItmyUy10503463 = klJXEgNifzdItmyUy35273100;     klJXEgNifzdItmyUy35273100 = klJXEgNifzdItmyUy61671220;     klJXEgNifzdItmyUy61671220 = klJXEgNifzdItmyUy80454120;     klJXEgNifzdItmyUy80454120 = klJXEgNifzdItmyUy29906462;     klJXEgNifzdItmyUy29906462 = klJXEgNifzdItmyUy45189109;     klJXEgNifzdItmyUy45189109 = klJXEgNifzdItmyUy88104949;     klJXEgNifzdItmyUy88104949 = klJXEgNifzdItmyUy18598463;     klJXEgNifzdItmyUy18598463 = klJXEgNifzdItmyUy11941431;     klJXEgNifzdItmyUy11941431 = klJXEgNifzdItmyUy6346319;     klJXEgNifzdItmyUy6346319 = klJXEgNifzdItmyUy71737133;     klJXEgNifzdItmyUy71737133 = klJXEgNifzdItmyUy45364014;     klJXEgNifzdItmyUy45364014 = klJXEgNifzdItmyUy28526590;     klJXEgNifzdItmyUy28526590 = klJXEgNifzdItmyUy38536882;     klJXEgNifzdItmyUy38536882 = klJXEgNifzdItmyUy89946275;     klJXEgNifzdItmyUy89946275 = klJXEgNifzdItmyUy74930245;     klJXEgNifzdItmyUy74930245 = klJXEgNifzdItmyUy67828402;     klJXEgNifzdItmyUy67828402 = klJXEgNifzdItmyUy76425452;     klJXEgNifzdItmyUy76425452 = klJXEgNifzdItmyUy52424842;     klJXEgNifzdItmyUy52424842 = klJXEgNifzdItmyUy7930679;     klJXEgNifzdItmyUy7930679 = klJXEgNifzdItmyUy60741979;     klJXEgNifzdItmyUy60741979 = klJXEgNifzdItmyUy39597533;     klJXEgNifzdItmyUy39597533 = klJXEgNifzdItmyUy76499657;     klJXEgNifzdItmyUy76499657 = klJXEgNifzdItmyUy26981021;     klJXEgNifzdItmyUy26981021 = klJXEgNifzdItmyUy85834654;     klJXEgNifzdItmyUy85834654 = klJXEgNifzdItmyUy75867394;     klJXEgNifzdItmyUy75867394 = klJXEgNifzdItmyUy72207849;     klJXEgNifzdItmyUy72207849 = klJXEgNifzdItmyUy50025177;     klJXEgNifzdItmyUy50025177 = klJXEgNifzdItmyUy99147348;     klJXEgNifzdItmyUy99147348 = klJXEgNifzdItmyUy19089264;     klJXEgNifzdItmyUy19089264 = klJXEgNifzdItmyUy69582622;     klJXEgNifzdItmyUy69582622 = klJXEgNifzdItmyUy67621748;     klJXEgNifzdItmyUy67621748 = klJXEgNifzdItmyUy74259272;     klJXEgNifzdItmyUy74259272 = klJXEgNifzdItmyUy80146779;     klJXEgNifzdItmyUy80146779 = klJXEgNifzdItmyUy38957715;     klJXEgNifzdItmyUy38957715 = klJXEgNifzdItmyUy5243910;     klJXEgNifzdItmyUy5243910 = klJXEgNifzdItmyUy77614846;     klJXEgNifzdItmyUy77614846 = klJXEgNifzdItmyUy66092537;     klJXEgNifzdItmyUy66092537 = klJXEgNifzdItmyUy32141961;     klJXEgNifzdItmyUy32141961 = klJXEgNifzdItmyUy68654843;     klJXEgNifzdItmyUy68654843 = klJXEgNifzdItmyUy81077496;     klJXEgNifzdItmyUy81077496 = klJXEgNifzdItmyUy86901261;     klJXEgNifzdItmyUy86901261 = klJXEgNifzdItmyUy95327154;     klJXEgNifzdItmyUy95327154 = klJXEgNifzdItmyUy7060853;     klJXEgNifzdItmyUy7060853 = klJXEgNifzdItmyUy75242803;     klJXEgNifzdItmyUy75242803 = klJXEgNifzdItmyUy64494180;     klJXEgNifzdItmyUy64494180 = klJXEgNifzdItmyUy83439970;     klJXEgNifzdItmyUy83439970 = klJXEgNifzdItmyUy49750395;     klJXEgNifzdItmyUy49750395 = klJXEgNifzdItmyUy54487415;     klJXEgNifzdItmyUy54487415 = klJXEgNifzdItmyUy94196245;     klJXEgNifzdItmyUy94196245 = klJXEgNifzdItmyUy90180147;     klJXEgNifzdItmyUy90180147 = klJXEgNifzdItmyUy56403775;     klJXEgNifzdItmyUy56403775 = klJXEgNifzdItmyUy5517568;     klJXEgNifzdItmyUy5517568 = klJXEgNifzdItmyUy3448049;     klJXEgNifzdItmyUy3448049 = klJXEgNifzdItmyUy72393109;     klJXEgNifzdItmyUy72393109 = klJXEgNifzdItmyUy70905930;     klJXEgNifzdItmyUy70905930 = klJXEgNifzdItmyUy58773443;     klJXEgNifzdItmyUy58773443 = klJXEgNifzdItmyUy34690200;     klJXEgNifzdItmyUy34690200 = klJXEgNifzdItmyUy94619466;     klJXEgNifzdItmyUy94619466 = klJXEgNifzdItmyUy54039068;     klJXEgNifzdItmyUy54039068 = klJXEgNifzdItmyUy72981260;     klJXEgNifzdItmyUy72981260 = klJXEgNifzdItmyUy38079772;     klJXEgNifzdItmyUy38079772 = klJXEgNifzdItmyUy19451115;     klJXEgNifzdItmyUy19451115 = klJXEgNifzdItmyUy92852166;     klJXEgNifzdItmyUy92852166 = klJXEgNifzdItmyUy36763696;     klJXEgNifzdItmyUy36763696 = klJXEgNifzdItmyUy4115385;     klJXEgNifzdItmyUy4115385 = klJXEgNifzdItmyUy71104742;     klJXEgNifzdItmyUy71104742 = klJXEgNifzdItmyUy48379811;     klJXEgNifzdItmyUy48379811 = klJXEgNifzdItmyUy99579167;     klJXEgNifzdItmyUy99579167 = klJXEgNifzdItmyUy84702365;     klJXEgNifzdItmyUy84702365 = klJXEgNifzdItmyUy97315399;     klJXEgNifzdItmyUy97315399 = klJXEgNifzdItmyUy1735865;     klJXEgNifzdItmyUy1735865 = klJXEgNifzdItmyUy44283492;     klJXEgNifzdItmyUy44283492 = klJXEgNifzdItmyUy83769999;     klJXEgNifzdItmyUy83769999 = klJXEgNifzdItmyUy26853183;     klJXEgNifzdItmyUy26853183 = klJXEgNifzdItmyUy73840718;     klJXEgNifzdItmyUy73840718 = klJXEgNifzdItmyUy44270378;     klJXEgNifzdItmyUy44270378 = klJXEgNifzdItmyUy69438804;     klJXEgNifzdItmyUy69438804 = klJXEgNifzdItmyUy51738218;     klJXEgNifzdItmyUy51738218 = klJXEgNifzdItmyUy21340474;     klJXEgNifzdItmyUy21340474 = klJXEgNifzdItmyUy92427424;     klJXEgNifzdItmyUy92427424 = klJXEgNifzdItmyUy22457454;     klJXEgNifzdItmyUy22457454 = klJXEgNifzdItmyUy95537762;     klJXEgNifzdItmyUy95537762 = klJXEgNifzdItmyUy4951103;     klJXEgNifzdItmyUy4951103 = klJXEgNifzdItmyUy28909117;     klJXEgNifzdItmyUy28909117 = klJXEgNifzdItmyUy13178848;     klJXEgNifzdItmyUy13178848 = klJXEgNifzdItmyUy62104181;     klJXEgNifzdItmyUy62104181 = klJXEgNifzdItmyUy70811224;     klJXEgNifzdItmyUy70811224 = klJXEgNifzdItmyUy7753671;     klJXEgNifzdItmyUy7753671 = klJXEgNifzdItmyUy68051785;     klJXEgNifzdItmyUy68051785 = klJXEgNifzdItmyUy46470467;     klJXEgNifzdItmyUy46470467 = klJXEgNifzdItmyUy42924647;     klJXEgNifzdItmyUy42924647 = klJXEgNifzdItmyUy71473071;     klJXEgNifzdItmyUy71473071 = klJXEgNifzdItmyUy78102892;     klJXEgNifzdItmyUy78102892 = klJXEgNifzdItmyUy95673582;     klJXEgNifzdItmyUy95673582 = klJXEgNifzdItmyUy58008550;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void CJzYSMbbcxPWSqsX84175559() {     double FUMgwzWBFqTmoKBcJ64125656 = -638926920;    double FUMgwzWBFqTmoKBcJ76202507 = 32351833;    double FUMgwzWBFqTmoKBcJ73984593 = -23608053;    double FUMgwzWBFqTmoKBcJ38453508 = -417479911;    double FUMgwzWBFqTmoKBcJ2030577 = -738248262;    double FUMgwzWBFqTmoKBcJ1920665 = -237212413;    double FUMgwzWBFqTmoKBcJ95762065 = -753303229;    double FUMgwzWBFqTmoKBcJ11431112 = -734990412;    double FUMgwzWBFqTmoKBcJ45096616 = -397125;    double FUMgwzWBFqTmoKBcJ71136121 = -263286167;    double FUMgwzWBFqTmoKBcJ5299400 = 95449526;    double FUMgwzWBFqTmoKBcJ19927992 = -211083094;    double FUMgwzWBFqTmoKBcJ4807582 = -100667842;    double FUMgwzWBFqTmoKBcJ95085107 = -689695046;    double FUMgwzWBFqTmoKBcJ2088746 = -606385863;    double FUMgwzWBFqTmoKBcJ13242527 = -680008472;    double FUMgwzWBFqTmoKBcJ39200017 = -633864096;    double FUMgwzWBFqTmoKBcJ25579743 = -853050840;    double FUMgwzWBFqTmoKBcJ366725 = -599700184;    double FUMgwzWBFqTmoKBcJ70496440 = -679174843;    double FUMgwzWBFqTmoKBcJ63483439 = -299725588;    double FUMgwzWBFqTmoKBcJ54349426 = -439967619;    double FUMgwzWBFqTmoKBcJ12016245 = -587017352;    double FUMgwzWBFqTmoKBcJ36417216 = -518931436;    double FUMgwzWBFqTmoKBcJ51630329 = -13762696;    double FUMgwzWBFqTmoKBcJ85463654 = -416532090;    double FUMgwzWBFqTmoKBcJ38057311 = -897433501;    double FUMgwzWBFqTmoKBcJ89606782 = -619041407;    double FUMgwzWBFqTmoKBcJ93113276 = -875581505;    double FUMgwzWBFqTmoKBcJ54450291 = -396919980;    double FUMgwzWBFqTmoKBcJ68109944 = -315022658;    double FUMgwzWBFqTmoKBcJ69818470 = -376556086;    double FUMgwzWBFqTmoKBcJ59736126 = -164626773;    double FUMgwzWBFqTmoKBcJ93977912 = -882481181;    double FUMgwzWBFqTmoKBcJ80563916 = -716323067;    double FUMgwzWBFqTmoKBcJ38308536 = -43729827;    double FUMgwzWBFqTmoKBcJ48883683 = -493886582;    double FUMgwzWBFqTmoKBcJ8349485 = -780947878;    double FUMgwzWBFqTmoKBcJ53757033 = -107204715;    double FUMgwzWBFqTmoKBcJ26459653 = -65977832;    double FUMgwzWBFqTmoKBcJ10913134 = -434233975;    double FUMgwzWBFqTmoKBcJ9765303 = -474993785;    double FUMgwzWBFqTmoKBcJ44159516 = -797524738;    double FUMgwzWBFqTmoKBcJ19549701 = -930570125;    double FUMgwzWBFqTmoKBcJ80607308 = -216784988;    double FUMgwzWBFqTmoKBcJ93016690 = -973571903;    double FUMgwzWBFqTmoKBcJ4679836 = -674852792;    double FUMgwzWBFqTmoKBcJ3284096 = 26484407;    double FUMgwzWBFqTmoKBcJ16736582 = -984522920;    double FUMgwzWBFqTmoKBcJ88154456 = -631432241;    double FUMgwzWBFqTmoKBcJ194859 = -213799553;    double FUMgwzWBFqTmoKBcJ94249689 = -141027012;    double FUMgwzWBFqTmoKBcJ97998852 = -811654719;    double FUMgwzWBFqTmoKBcJ50456295 = -39066404;    double FUMgwzWBFqTmoKBcJ20692121 = -103884242;    double FUMgwzWBFqTmoKBcJ9776231 = -98959302;    double FUMgwzWBFqTmoKBcJ64186262 = -380630815;    double FUMgwzWBFqTmoKBcJ37567377 = -504676618;    double FUMgwzWBFqTmoKBcJ86823179 = -303717216;    double FUMgwzWBFqTmoKBcJ16566923 = -221716172;    double FUMgwzWBFqTmoKBcJ63863354 = -339778912;    double FUMgwzWBFqTmoKBcJ6155284 = -34261822;    double FUMgwzWBFqTmoKBcJ18317835 = -859408907;    double FUMgwzWBFqTmoKBcJ90646324 = -603477146;    double FUMgwzWBFqTmoKBcJ3026178 = -948263510;    double FUMgwzWBFqTmoKBcJ35480929 = -527994388;    double FUMgwzWBFqTmoKBcJ60191865 = 53543678;    double FUMgwzWBFqTmoKBcJ10829669 = -218186662;    double FUMgwzWBFqTmoKBcJ14521192 = -973371980;    double FUMgwzWBFqTmoKBcJ63780210 = -462656037;    double FUMgwzWBFqTmoKBcJ64358843 = -86121891;    double FUMgwzWBFqTmoKBcJ30850533 = -852916218;    double FUMgwzWBFqTmoKBcJ71822710 = -645846126;    double FUMgwzWBFqTmoKBcJ73907071 = -433722352;    double FUMgwzWBFqTmoKBcJ59583307 = -144940869;    double FUMgwzWBFqTmoKBcJ53718136 = -824731804;    double FUMgwzWBFqTmoKBcJ10189911 = -642442881;    double FUMgwzWBFqTmoKBcJ92466543 = -656447227;    double FUMgwzWBFqTmoKBcJ55809908 = -202146448;    double FUMgwzWBFqTmoKBcJ58613638 = -40190794;    double FUMgwzWBFqTmoKBcJ80783819 = -741679298;    double FUMgwzWBFqTmoKBcJ34773215 = -823917909;    double FUMgwzWBFqTmoKBcJ72870201 = -634518487;    double FUMgwzWBFqTmoKBcJ4958821 = -144149265;    double FUMgwzWBFqTmoKBcJ54255432 = -83120427;    double FUMgwzWBFqTmoKBcJ73860254 = -73995647;    double FUMgwzWBFqTmoKBcJ71819618 = -564901368;    double FUMgwzWBFqTmoKBcJ9279831 = -25560370;    double FUMgwzWBFqTmoKBcJ73285791 = -678596939;    double FUMgwzWBFqTmoKBcJ70787685 = -517363765;    double FUMgwzWBFqTmoKBcJ74122273 = -663099012;    double FUMgwzWBFqTmoKBcJ11316306 = -989209965;    double FUMgwzWBFqTmoKBcJ21526306 = -377230663;    double FUMgwzWBFqTmoKBcJ37190111 = -885488543;    double FUMgwzWBFqTmoKBcJ62596299 = -726198921;    double FUMgwzWBFqTmoKBcJ4757851 = -299972153;    double FUMgwzWBFqTmoKBcJ91447468 = -615584878;    double FUMgwzWBFqTmoKBcJ53513191 = -94047593;    double FUMgwzWBFqTmoKBcJ16523523 = -982306616;    double FUMgwzWBFqTmoKBcJ45126379 = -638926920;     FUMgwzWBFqTmoKBcJ64125656 = FUMgwzWBFqTmoKBcJ76202507;     FUMgwzWBFqTmoKBcJ76202507 = FUMgwzWBFqTmoKBcJ73984593;     FUMgwzWBFqTmoKBcJ73984593 = FUMgwzWBFqTmoKBcJ38453508;     FUMgwzWBFqTmoKBcJ38453508 = FUMgwzWBFqTmoKBcJ2030577;     FUMgwzWBFqTmoKBcJ2030577 = FUMgwzWBFqTmoKBcJ1920665;     FUMgwzWBFqTmoKBcJ1920665 = FUMgwzWBFqTmoKBcJ95762065;     FUMgwzWBFqTmoKBcJ95762065 = FUMgwzWBFqTmoKBcJ11431112;     FUMgwzWBFqTmoKBcJ11431112 = FUMgwzWBFqTmoKBcJ45096616;     FUMgwzWBFqTmoKBcJ45096616 = FUMgwzWBFqTmoKBcJ71136121;     FUMgwzWBFqTmoKBcJ71136121 = FUMgwzWBFqTmoKBcJ5299400;     FUMgwzWBFqTmoKBcJ5299400 = FUMgwzWBFqTmoKBcJ19927992;     FUMgwzWBFqTmoKBcJ19927992 = FUMgwzWBFqTmoKBcJ4807582;     FUMgwzWBFqTmoKBcJ4807582 = FUMgwzWBFqTmoKBcJ95085107;     FUMgwzWBFqTmoKBcJ95085107 = FUMgwzWBFqTmoKBcJ2088746;     FUMgwzWBFqTmoKBcJ2088746 = FUMgwzWBFqTmoKBcJ13242527;     FUMgwzWBFqTmoKBcJ13242527 = FUMgwzWBFqTmoKBcJ39200017;     FUMgwzWBFqTmoKBcJ39200017 = FUMgwzWBFqTmoKBcJ25579743;     FUMgwzWBFqTmoKBcJ25579743 = FUMgwzWBFqTmoKBcJ366725;     FUMgwzWBFqTmoKBcJ366725 = FUMgwzWBFqTmoKBcJ70496440;     FUMgwzWBFqTmoKBcJ70496440 = FUMgwzWBFqTmoKBcJ63483439;     FUMgwzWBFqTmoKBcJ63483439 = FUMgwzWBFqTmoKBcJ54349426;     FUMgwzWBFqTmoKBcJ54349426 = FUMgwzWBFqTmoKBcJ12016245;     FUMgwzWBFqTmoKBcJ12016245 = FUMgwzWBFqTmoKBcJ36417216;     FUMgwzWBFqTmoKBcJ36417216 = FUMgwzWBFqTmoKBcJ51630329;     FUMgwzWBFqTmoKBcJ51630329 = FUMgwzWBFqTmoKBcJ85463654;     FUMgwzWBFqTmoKBcJ85463654 = FUMgwzWBFqTmoKBcJ38057311;     FUMgwzWBFqTmoKBcJ38057311 = FUMgwzWBFqTmoKBcJ89606782;     FUMgwzWBFqTmoKBcJ89606782 = FUMgwzWBFqTmoKBcJ93113276;     FUMgwzWBFqTmoKBcJ93113276 = FUMgwzWBFqTmoKBcJ54450291;     FUMgwzWBFqTmoKBcJ54450291 = FUMgwzWBFqTmoKBcJ68109944;     FUMgwzWBFqTmoKBcJ68109944 = FUMgwzWBFqTmoKBcJ69818470;     FUMgwzWBFqTmoKBcJ69818470 = FUMgwzWBFqTmoKBcJ59736126;     FUMgwzWBFqTmoKBcJ59736126 = FUMgwzWBFqTmoKBcJ93977912;     FUMgwzWBFqTmoKBcJ93977912 = FUMgwzWBFqTmoKBcJ80563916;     FUMgwzWBFqTmoKBcJ80563916 = FUMgwzWBFqTmoKBcJ38308536;     FUMgwzWBFqTmoKBcJ38308536 = FUMgwzWBFqTmoKBcJ48883683;     FUMgwzWBFqTmoKBcJ48883683 = FUMgwzWBFqTmoKBcJ8349485;     FUMgwzWBFqTmoKBcJ8349485 = FUMgwzWBFqTmoKBcJ53757033;     FUMgwzWBFqTmoKBcJ53757033 = FUMgwzWBFqTmoKBcJ26459653;     FUMgwzWBFqTmoKBcJ26459653 = FUMgwzWBFqTmoKBcJ10913134;     FUMgwzWBFqTmoKBcJ10913134 = FUMgwzWBFqTmoKBcJ9765303;     FUMgwzWBFqTmoKBcJ9765303 = FUMgwzWBFqTmoKBcJ44159516;     FUMgwzWBFqTmoKBcJ44159516 = FUMgwzWBFqTmoKBcJ19549701;     FUMgwzWBFqTmoKBcJ19549701 = FUMgwzWBFqTmoKBcJ80607308;     FUMgwzWBFqTmoKBcJ80607308 = FUMgwzWBFqTmoKBcJ93016690;     FUMgwzWBFqTmoKBcJ93016690 = FUMgwzWBFqTmoKBcJ4679836;     FUMgwzWBFqTmoKBcJ4679836 = FUMgwzWBFqTmoKBcJ3284096;     FUMgwzWBFqTmoKBcJ3284096 = FUMgwzWBFqTmoKBcJ16736582;     FUMgwzWBFqTmoKBcJ16736582 = FUMgwzWBFqTmoKBcJ88154456;     FUMgwzWBFqTmoKBcJ88154456 = FUMgwzWBFqTmoKBcJ194859;     FUMgwzWBFqTmoKBcJ194859 = FUMgwzWBFqTmoKBcJ94249689;     FUMgwzWBFqTmoKBcJ94249689 = FUMgwzWBFqTmoKBcJ97998852;     FUMgwzWBFqTmoKBcJ97998852 = FUMgwzWBFqTmoKBcJ50456295;     FUMgwzWBFqTmoKBcJ50456295 = FUMgwzWBFqTmoKBcJ20692121;     FUMgwzWBFqTmoKBcJ20692121 = FUMgwzWBFqTmoKBcJ9776231;     FUMgwzWBFqTmoKBcJ9776231 = FUMgwzWBFqTmoKBcJ64186262;     FUMgwzWBFqTmoKBcJ64186262 = FUMgwzWBFqTmoKBcJ37567377;     FUMgwzWBFqTmoKBcJ37567377 = FUMgwzWBFqTmoKBcJ86823179;     FUMgwzWBFqTmoKBcJ86823179 = FUMgwzWBFqTmoKBcJ16566923;     FUMgwzWBFqTmoKBcJ16566923 = FUMgwzWBFqTmoKBcJ63863354;     FUMgwzWBFqTmoKBcJ63863354 = FUMgwzWBFqTmoKBcJ6155284;     FUMgwzWBFqTmoKBcJ6155284 = FUMgwzWBFqTmoKBcJ18317835;     FUMgwzWBFqTmoKBcJ18317835 = FUMgwzWBFqTmoKBcJ90646324;     FUMgwzWBFqTmoKBcJ90646324 = FUMgwzWBFqTmoKBcJ3026178;     FUMgwzWBFqTmoKBcJ3026178 = FUMgwzWBFqTmoKBcJ35480929;     FUMgwzWBFqTmoKBcJ35480929 = FUMgwzWBFqTmoKBcJ60191865;     FUMgwzWBFqTmoKBcJ60191865 = FUMgwzWBFqTmoKBcJ10829669;     FUMgwzWBFqTmoKBcJ10829669 = FUMgwzWBFqTmoKBcJ14521192;     FUMgwzWBFqTmoKBcJ14521192 = FUMgwzWBFqTmoKBcJ63780210;     FUMgwzWBFqTmoKBcJ63780210 = FUMgwzWBFqTmoKBcJ64358843;     FUMgwzWBFqTmoKBcJ64358843 = FUMgwzWBFqTmoKBcJ30850533;     FUMgwzWBFqTmoKBcJ30850533 = FUMgwzWBFqTmoKBcJ71822710;     FUMgwzWBFqTmoKBcJ71822710 = FUMgwzWBFqTmoKBcJ73907071;     FUMgwzWBFqTmoKBcJ73907071 = FUMgwzWBFqTmoKBcJ59583307;     FUMgwzWBFqTmoKBcJ59583307 = FUMgwzWBFqTmoKBcJ53718136;     FUMgwzWBFqTmoKBcJ53718136 = FUMgwzWBFqTmoKBcJ10189911;     FUMgwzWBFqTmoKBcJ10189911 = FUMgwzWBFqTmoKBcJ92466543;     FUMgwzWBFqTmoKBcJ92466543 = FUMgwzWBFqTmoKBcJ55809908;     FUMgwzWBFqTmoKBcJ55809908 = FUMgwzWBFqTmoKBcJ58613638;     FUMgwzWBFqTmoKBcJ58613638 = FUMgwzWBFqTmoKBcJ80783819;     FUMgwzWBFqTmoKBcJ80783819 = FUMgwzWBFqTmoKBcJ34773215;     FUMgwzWBFqTmoKBcJ34773215 = FUMgwzWBFqTmoKBcJ72870201;     FUMgwzWBFqTmoKBcJ72870201 = FUMgwzWBFqTmoKBcJ4958821;     FUMgwzWBFqTmoKBcJ4958821 = FUMgwzWBFqTmoKBcJ54255432;     FUMgwzWBFqTmoKBcJ54255432 = FUMgwzWBFqTmoKBcJ73860254;     FUMgwzWBFqTmoKBcJ73860254 = FUMgwzWBFqTmoKBcJ71819618;     FUMgwzWBFqTmoKBcJ71819618 = FUMgwzWBFqTmoKBcJ9279831;     FUMgwzWBFqTmoKBcJ9279831 = FUMgwzWBFqTmoKBcJ73285791;     FUMgwzWBFqTmoKBcJ73285791 = FUMgwzWBFqTmoKBcJ70787685;     FUMgwzWBFqTmoKBcJ70787685 = FUMgwzWBFqTmoKBcJ74122273;     FUMgwzWBFqTmoKBcJ74122273 = FUMgwzWBFqTmoKBcJ11316306;     FUMgwzWBFqTmoKBcJ11316306 = FUMgwzWBFqTmoKBcJ21526306;     FUMgwzWBFqTmoKBcJ21526306 = FUMgwzWBFqTmoKBcJ37190111;     FUMgwzWBFqTmoKBcJ37190111 = FUMgwzWBFqTmoKBcJ62596299;     FUMgwzWBFqTmoKBcJ62596299 = FUMgwzWBFqTmoKBcJ4757851;     FUMgwzWBFqTmoKBcJ4757851 = FUMgwzWBFqTmoKBcJ91447468;     FUMgwzWBFqTmoKBcJ91447468 = FUMgwzWBFqTmoKBcJ53513191;     FUMgwzWBFqTmoKBcJ53513191 = FUMgwzWBFqTmoKBcJ16523523;     FUMgwzWBFqTmoKBcJ16523523 = FUMgwzWBFqTmoKBcJ45126379;     FUMgwzWBFqTmoKBcJ45126379 = FUMgwzWBFqTmoKBcJ64125656;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void WkqrzJbCXxHIsQiw51467159() {     double dmbjjvgIxoYVArMVM99584624 = -5203820;    double dmbjjvgIxoYVArMVM89474585 = -948199773;    double dmbjjvgIxoYVArMVM11371071 = -789275128;    double dmbjjvgIxoYVArMVM10676863 = -238630983;    double dmbjjvgIxoYVArMVM58315846 = -843847107;    double dmbjjvgIxoYVArMVM38143537 = -499560308;    double dmbjjvgIxoYVArMVM6419473 = -598028963;    double dmbjjvgIxoYVArMVM73562672 = -379224552;    double dmbjjvgIxoYVArMVM46614221 = -488728532;    double dmbjjvgIxoYVArMVM86038833 = 72026835;    double dmbjjvgIxoYVArMVM43481134 = -806558203;    double dmbjjvgIxoYVArMVM20496140 = -75203866;    double dmbjjvgIxoYVArMVM43042227 = -652680347;    double dmbjjvgIxoYVArMVM48225971 = -666058456;    double dmbjjvgIxoYVArMVM60283262 = -9810878;    double dmbjjvgIxoYVArMVM96783827 = -315820080;    double dmbjjvgIxoYVArMVM95035953 = -60838649;    double dmbjjvgIxoYVArMVM25339935 = -926478909;    double dmbjjvgIxoYVArMVM917055 = -916263606;    double dmbjjvgIxoYVArMVM64333659 = -5735909;    double dmbjjvgIxoYVArMVM76570301 = 42999723;    double dmbjjvgIxoYVArMVM57770950 = -526603913;    double dmbjjvgIxoYVArMVM82541624 = -224649536;    double dmbjjvgIxoYVArMVM30210162 = 70243959;    double dmbjjvgIxoYVArMVM59648280 = -37747895;    double dmbjjvgIxoYVArMVM96161826 = -105108897;    double dmbjjvgIxoYVArMVM98477884 = -703717642;    double dmbjjvgIxoYVArMVM30335208 = -788813581;    double dmbjjvgIxoYVArMVM39781491 = -349676307;    double dmbjjvgIxoYVArMVM59983702 = -431427150;    double dmbjjvgIxoYVArMVM86693175 = -867003145;    double dmbjjvgIxoYVArMVM7641489 = -149527501;    double dmbjjvgIxoYVArMVM71954137 = -312222750;    double dmbjjvgIxoYVArMVM23726339 = -947174994;    double dmbjjvgIxoYVArMVM46372370 = -667423317;    double dmbjjvgIxoYVArMVM89869379 = -258295133;    double dmbjjvgIxoYVArMVM98295499 = -358582622;    double dmbjjvgIxoYVArMVM59564888 = -763603459;    double dmbjjvgIxoYVArMVM84147594 = -858080991;    double dmbjjvgIxoYVArMVM90731185 = -631240378;    double dmbjjvgIxoYVArMVM44364741 = -203432889;    double dmbjjvgIxoYVArMVM99130429 = -69539076;    double dmbjjvgIxoYVArMVM33542894 = -350705524;    double dmbjjvgIxoYVArMVM59681039 = -434902535;    double dmbjjvgIxoYVArMVM80623766 = -43296150;    double dmbjjvgIxoYVArMVM38978019 = 87504465;    double dmbjjvgIxoYVArMVM24025000 = -58839153;    double dmbjjvgIxoYVArMVM21010408 = -140042186;    double dmbjjvgIxoYVArMVM39629248 = -648259800;    double dmbjjvgIxoYVArMVM12684046 = -22459057;    double dmbjjvgIxoYVArMVM80965582 = -88076110;    double dmbjjvgIxoYVArMVM77735161 = -354679358;    double dmbjjvgIxoYVArMVM66620000 = -567700273;    double dmbjjvgIxoYVArMVM66448142 = -230574903;    double dmbjjvgIxoYVArMVM74896857 = -450374388;    double dmbjjvgIxoYVArMVM41813675 = -478599908;    double dmbjjvgIxoYVArMVM6932962 = -623550238;    double dmbjjvgIxoYVArMVM81160908 = -759519088;    double dmbjjvgIxoYVArMVM51028583 = -100883089;    double dmbjjvgIxoYVArMVM62154020 = -638738211;    double dmbjjvgIxoYVArMVM39665652 = -795842667;    double dmbjjvgIxoYVArMVM76084264 = -809215382;    double dmbjjvgIxoYVArMVM33781182 = 70451755;    double dmbjjvgIxoYVArMVM86630519 = 42698618;    double dmbjjvgIxoYVArMVM99345658 = -60970021;    double dmbjjvgIxoYVArMVM35839645 = -557030703;    double dmbjjvgIxoYVArMVM48542003 = -762981116;    double dmbjjvgIxoYVArMVM19315888 = -705505353;    double dmbjjvgIxoYVArMVM1853601 = -998635139;    double dmbjjvgIxoYVArMVM70413882 = -751515746;    double dmbjjvgIxoYVArMVM98488327 = -957237458;    double dmbjjvgIxoYVArMVM35471065 = -297235190;    double dmbjjvgIxoYVArMVM41192340 = 31602082;    double dmbjjvgIxoYVArMVM10185869 = -185023228;    double dmbjjvgIxoYVArMVM19968918 = -802303021;    double dmbjjvgIxoYVArMVM77439872 = -887461202;    double dmbjjvgIxoYVArMVM24228057 = -75898389;    double dmbjjvgIxoYVArMVM22860586 = -789747002;    double dmbjjvgIxoYVArMVM49586395 = -886459891;    double dmbjjvgIxoYVArMVM20670261 = -25252360;    double dmbjjvgIxoYVArMVM72136827 = 53730256;    double dmbjjvgIxoYVArMVM77467477 = -463675457;    double dmbjjvgIxoYVArMVM90705960 = -40553782;    double dmbjjvgIxoYVArMVM27097446 = -227217251;    double dmbjjvgIxoYVArMVM79018119 = -243351041;    double dmbjjvgIxoYVArMVM8958014 = -412323787;    double dmbjjvgIxoYVArMVM41021489 = -581827229;    double dmbjjvgIxoYVArMVM5505995 = 18352153;    double dmbjjvgIxoYVArMVM48829482 = -396800607;    double dmbjjvgIxoYVArMVM4558696 = -88823410;    double dmbjjvgIxoYVArMVM82936418 = -634744895;    double dmbjjvgIxoYVArMVM17134591 = -599063534;    double dmbjjvgIxoYVArMVM8536306 = -562720371;    double dmbjjvgIxoYVArMVM21993575 = -119342781;    double dmbjjvgIxoYVArMVM51065533 = -835397712;    double dmbjjvgIxoYVArMVM68280476 = -394217507;    double dmbjjvgIxoYVArMVM65349248 = -39990831;    double dmbjjvgIxoYVArMVM46912375 = -293404142;    double dmbjjvgIxoYVArMVM60335381 = -273932515;    double dmbjjvgIxoYVArMVM44784122 = -5203820;     dmbjjvgIxoYVArMVM99584624 = dmbjjvgIxoYVArMVM89474585;     dmbjjvgIxoYVArMVM89474585 = dmbjjvgIxoYVArMVM11371071;     dmbjjvgIxoYVArMVM11371071 = dmbjjvgIxoYVArMVM10676863;     dmbjjvgIxoYVArMVM10676863 = dmbjjvgIxoYVArMVM58315846;     dmbjjvgIxoYVArMVM58315846 = dmbjjvgIxoYVArMVM38143537;     dmbjjvgIxoYVArMVM38143537 = dmbjjvgIxoYVArMVM6419473;     dmbjjvgIxoYVArMVM6419473 = dmbjjvgIxoYVArMVM73562672;     dmbjjvgIxoYVArMVM73562672 = dmbjjvgIxoYVArMVM46614221;     dmbjjvgIxoYVArMVM46614221 = dmbjjvgIxoYVArMVM86038833;     dmbjjvgIxoYVArMVM86038833 = dmbjjvgIxoYVArMVM43481134;     dmbjjvgIxoYVArMVM43481134 = dmbjjvgIxoYVArMVM20496140;     dmbjjvgIxoYVArMVM20496140 = dmbjjvgIxoYVArMVM43042227;     dmbjjvgIxoYVArMVM43042227 = dmbjjvgIxoYVArMVM48225971;     dmbjjvgIxoYVArMVM48225971 = dmbjjvgIxoYVArMVM60283262;     dmbjjvgIxoYVArMVM60283262 = dmbjjvgIxoYVArMVM96783827;     dmbjjvgIxoYVArMVM96783827 = dmbjjvgIxoYVArMVM95035953;     dmbjjvgIxoYVArMVM95035953 = dmbjjvgIxoYVArMVM25339935;     dmbjjvgIxoYVArMVM25339935 = dmbjjvgIxoYVArMVM917055;     dmbjjvgIxoYVArMVM917055 = dmbjjvgIxoYVArMVM64333659;     dmbjjvgIxoYVArMVM64333659 = dmbjjvgIxoYVArMVM76570301;     dmbjjvgIxoYVArMVM76570301 = dmbjjvgIxoYVArMVM57770950;     dmbjjvgIxoYVArMVM57770950 = dmbjjvgIxoYVArMVM82541624;     dmbjjvgIxoYVArMVM82541624 = dmbjjvgIxoYVArMVM30210162;     dmbjjvgIxoYVArMVM30210162 = dmbjjvgIxoYVArMVM59648280;     dmbjjvgIxoYVArMVM59648280 = dmbjjvgIxoYVArMVM96161826;     dmbjjvgIxoYVArMVM96161826 = dmbjjvgIxoYVArMVM98477884;     dmbjjvgIxoYVArMVM98477884 = dmbjjvgIxoYVArMVM30335208;     dmbjjvgIxoYVArMVM30335208 = dmbjjvgIxoYVArMVM39781491;     dmbjjvgIxoYVArMVM39781491 = dmbjjvgIxoYVArMVM59983702;     dmbjjvgIxoYVArMVM59983702 = dmbjjvgIxoYVArMVM86693175;     dmbjjvgIxoYVArMVM86693175 = dmbjjvgIxoYVArMVM7641489;     dmbjjvgIxoYVArMVM7641489 = dmbjjvgIxoYVArMVM71954137;     dmbjjvgIxoYVArMVM71954137 = dmbjjvgIxoYVArMVM23726339;     dmbjjvgIxoYVArMVM23726339 = dmbjjvgIxoYVArMVM46372370;     dmbjjvgIxoYVArMVM46372370 = dmbjjvgIxoYVArMVM89869379;     dmbjjvgIxoYVArMVM89869379 = dmbjjvgIxoYVArMVM98295499;     dmbjjvgIxoYVArMVM98295499 = dmbjjvgIxoYVArMVM59564888;     dmbjjvgIxoYVArMVM59564888 = dmbjjvgIxoYVArMVM84147594;     dmbjjvgIxoYVArMVM84147594 = dmbjjvgIxoYVArMVM90731185;     dmbjjvgIxoYVArMVM90731185 = dmbjjvgIxoYVArMVM44364741;     dmbjjvgIxoYVArMVM44364741 = dmbjjvgIxoYVArMVM99130429;     dmbjjvgIxoYVArMVM99130429 = dmbjjvgIxoYVArMVM33542894;     dmbjjvgIxoYVArMVM33542894 = dmbjjvgIxoYVArMVM59681039;     dmbjjvgIxoYVArMVM59681039 = dmbjjvgIxoYVArMVM80623766;     dmbjjvgIxoYVArMVM80623766 = dmbjjvgIxoYVArMVM38978019;     dmbjjvgIxoYVArMVM38978019 = dmbjjvgIxoYVArMVM24025000;     dmbjjvgIxoYVArMVM24025000 = dmbjjvgIxoYVArMVM21010408;     dmbjjvgIxoYVArMVM21010408 = dmbjjvgIxoYVArMVM39629248;     dmbjjvgIxoYVArMVM39629248 = dmbjjvgIxoYVArMVM12684046;     dmbjjvgIxoYVArMVM12684046 = dmbjjvgIxoYVArMVM80965582;     dmbjjvgIxoYVArMVM80965582 = dmbjjvgIxoYVArMVM77735161;     dmbjjvgIxoYVArMVM77735161 = dmbjjvgIxoYVArMVM66620000;     dmbjjvgIxoYVArMVM66620000 = dmbjjvgIxoYVArMVM66448142;     dmbjjvgIxoYVArMVM66448142 = dmbjjvgIxoYVArMVM74896857;     dmbjjvgIxoYVArMVM74896857 = dmbjjvgIxoYVArMVM41813675;     dmbjjvgIxoYVArMVM41813675 = dmbjjvgIxoYVArMVM6932962;     dmbjjvgIxoYVArMVM6932962 = dmbjjvgIxoYVArMVM81160908;     dmbjjvgIxoYVArMVM81160908 = dmbjjvgIxoYVArMVM51028583;     dmbjjvgIxoYVArMVM51028583 = dmbjjvgIxoYVArMVM62154020;     dmbjjvgIxoYVArMVM62154020 = dmbjjvgIxoYVArMVM39665652;     dmbjjvgIxoYVArMVM39665652 = dmbjjvgIxoYVArMVM76084264;     dmbjjvgIxoYVArMVM76084264 = dmbjjvgIxoYVArMVM33781182;     dmbjjvgIxoYVArMVM33781182 = dmbjjvgIxoYVArMVM86630519;     dmbjjvgIxoYVArMVM86630519 = dmbjjvgIxoYVArMVM99345658;     dmbjjvgIxoYVArMVM99345658 = dmbjjvgIxoYVArMVM35839645;     dmbjjvgIxoYVArMVM35839645 = dmbjjvgIxoYVArMVM48542003;     dmbjjvgIxoYVArMVM48542003 = dmbjjvgIxoYVArMVM19315888;     dmbjjvgIxoYVArMVM19315888 = dmbjjvgIxoYVArMVM1853601;     dmbjjvgIxoYVArMVM1853601 = dmbjjvgIxoYVArMVM70413882;     dmbjjvgIxoYVArMVM70413882 = dmbjjvgIxoYVArMVM98488327;     dmbjjvgIxoYVArMVM98488327 = dmbjjvgIxoYVArMVM35471065;     dmbjjvgIxoYVArMVM35471065 = dmbjjvgIxoYVArMVM41192340;     dmbjjvgIxoYVArMVM41192340 = dmbjjvgIxoYVArMVM10185869;     dmbjjvgIxoYVArMVM10185869 = dmbjjvgIxoYVArMVM19968918;     dmbjjvgIxoYVArMVM19968918 = dmbjjvgIxoYVArMVM77439872;     dmbjjvgIxoYVArMVM77439872 = dmbjjvgIxoYVArMVM24228057;     dmbjjvgIxoYVArMVM24228057 = dmbjjvgIxoYVArMVM22860586;     dmbjjvgIxoYVArMVM22860586 = dmbjjvgIxoYVArMVM49586395;     dmbjjvgIxoYVArMVM49586395 = dmbjjvgIxoYVArMVM20670261;     dmbjjvgIxoYVArMVM20670261 = dmbjjvgIxoYVArMVM72136827;     dmbjjvgIxoYVArMVM72136827 = dmbjjvgIxoYVArMVM77467477;     dmbjjvgIxoYVArMVM77467477 = dmbjjvgIxoYVArMVM90705960;     dmbjjvgIxoYVArMVM90705960 = dmbjjvgIxoYVArMVM27097446;     dmbjjvgIxoYVArMVM27097446 = dmbjjvgIxoYVArMVM79018119;     dmbjjvgIxoYVArMVM79018119 = dmbjjvgIxoYVArMVM8958014;     dmbjjvgIxoYVArMVM8958014 = dmbjjvgIxoYVArMVM41021489;     dmbjjvgIxoYVArMVM41021489 = dmbjjvgIxoYVArMVM5505995;     dmbjjvgIxoYVArMVM5505995 = dmbjjvgIxoYVArMVM48829482;     dmbjjvgIxoYVArMVM48829482 = dmbjjvgIxoYVArMVM4558696;     dmbjjvgIxoYVArMVM4558696 = dmbjjvgIxoYVArMVM82936418;     dmbjjvgIxoYVArMVM82936418 = dmbjjvgIxoYVArMVM17134591;     dmbjjvgIxoYVArMVM17134591 = dmbjjvgIxoYVArMVM8536306;     dmbjjvgIxoYVArMVM8536306 = dmbjjvgIxoYVArMVM21993575;     dmbjjvgIxoYVArMVM21993575 = dmbjjvgIxoYVArMVM51065533;     dmbjjvgIxoYVArMVM51065533 = dmbjjvgIxoYVArMVM68280476;     dmbjjvgIxoYVArMVM68280476 = dmbjjvgIxoYVArMVM65349248;     dmbjjvgIxoYVArMVM65349248 = dmbjjvgIxoYVArMVM46912375;     dmbjjvgIxoYVArMVM46912375 = dmbjjvgIxoYVArMVM60335381;     dmbjjvgIxoYVArMVM60335381 = dmbjjvgIxoYVArMVM44784122;     dmbjjvgIxoYVArMVM44784122 = dmbjjvgIxoYVArMVM99584624;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void DogRTNXHMQNQgvgU66516226() {     double ECKtjPMDJLbarsHlm5701732 = -117105709;    double ECKtjPMDJLbarsHlm32847865 = -657799554;    double ECKtjPMDJLbarsHlm27413254 = -940108050;    double ECKtjPMDJLbarsHlm37751643 = -429379613;    double ECKtjPMDJLbarsHlm27211335 = -264496978;    double ECKtjPMDJLbarsHlm29560739 = -764056543;    double ECKtjPMDJLbarsHlm66908438 = -736993940;    double ECKtjPMDJLbarsHlm23322564 = -666098465;    double ECKtjPMDJLbarsHlm11256717 = -256761198;    double ECKtjPMDJLbarsHlm27268493 = -256948642;    double ECKtjPMDJLbarsHlm3591425 = -683995790;    double ECKtjPMDJLbarsHlm52319182 = -519610068;    double ECKtjPMDJLbarsHlm29251345 = 7428564;    double ECKtjPMDJLbarsHlm31369647 = -98297024;    double ECKtjPMDJLbarsHlm56025688 = -700273044;    double ECKtjPMDJLbarsHlm38289221 = -288819623;    double ECKtjPMDJLbarsHlm88871956 = -677383761;    double ECKtjPMDJLbarsHlm22393088 = -592001552;    double ECKtjPMDJLbarsHlm62746896 = -222032703;    double ECKtjPMDJLbarsHlm44883823 = -362638829;    double ECKtjPMDJLbarsHlm65123495 = -912487733;    double ECKtjPMDJLbarsHlm44291974 = -434533629;    double ECKtjPMDJLbarsHlm18132416 = 28832977;    double ECKtjPMDJLbarsHlm14202536 = -152061028;    double ECKtjPMDJLbarsHlm3347930 = -410943928;    double ECKtjPMDJLbarsHlm20883502 = 16235780;    double ECKtjPMDJLbarsHlm96937662 = -604379845;    double ECKtjPMDJLbarsHlm43442333 = -854742535;    double ECKtjPMDJLbarsHlm5913747 = -572059012;    double ECKtjPMDJLbarsHlm28599339 = -248898018;    double ECKtjPMDJLbarsHlm78935724 = -780324083;    double ECKtjPMDJLbarsHlm5252111 = -958974317;    double ECKtjPMDJLbarsHlm81665085 = -204836121;    double ECKtjPMDJLbarsHlm18556903 = -698716759;    double ECKtjPMDJLbarsHlm7847022 = -206156423;    double ECKtjPMDJLbarsHlm58595292 = -472321240;    double ECKtjPMDJLbarsHlm79557434 = -312218492;    double ECKtjPMDJLbarsHlm93655100 = -535109469;    double ECKtjPMDJLbarsHlm57757848 = -852004488;    double ECKtjPMDJLbarsHlm78233123 = -4679601;    double ECKtjPMDJLbarsHlm50033965 = -268706783;    double ECKtjPMDJLbarsHlm31280887 = -339681354;    double ECKtjPMDJLbarsHlm11609872 = -431510108;    double ECKtjPMDJLbarsHlm47088779 = -454245473;    double ECKtjPMDJLbarsHlm92576231 = -805194304;    double ECKtjPMDJLbarsHlm50917214 = -997711652;    double ECKtjPMDJLbarsHlm41803574 = -177614691;    double ECKtjPMDJLbarsHlm28967348 = -149937719;    double ECKtjPMDJLbarsHlm49304977 = -733996788;    double ECKtjPMDJLbarsHlm25595698 = -109469865;    double ECKtjPMDJLbarsHlm16666261 = 69630218;    double ECKtjPMDJLbarsHlm88544881 = -155405678;    double ECKtjPMDJLbarsHlm14868458 = -359489860;    double ECKtjPMDJLbarsHlm62417022 = -551045049;    double ECKtjPMDJLbarsHlm1392732 = -50906170;    double ECKtjPMDJLbarsHlm61409757 = -682572081;    double ECKtjPMDJLbarsHlm14715449 = -586632531;    double ECKtjPMDJLbarsHlm13210719 = -688047022;    double ECKtjPMDJLbarsHlm34403713 = 81564314;    double ECKtjPMDJLbarsHlm6327833 = -180732759;    double ECKtjPMDJLbarsHlm32623076 = -59676699;    double ECKtjPMDJLbarsHlm23466106 = -882251406;    double ECKtjPMDJLbarsHlm17408817 = 5960546;    double ECKtjPMDJLbarsHlm82657377 = 92136819;    double ECKtjPMDJLbarsHlm48332768 = -476624560;    double ECKtjPMDJLbarsHlm98339313 = -725021473;    double ECKtjPMDJLbarsHlm70654097 = -214773947;    double ECKtjPMDJLbarsHlm10694442 = -293854677;    double ECKtjPMDJLbarsHlm23522626 = -892140601;    double ECKtjPMDJLbarsHlm97430395 = -127951805;    double ECKtjPMDJLbarsHlm58731787 = -976601131;    double ECKtjPMDJLbarsHlm95216855 = -42274293;    double ECKtjPMDJLbarsHlm64635239 = -739997064;    double ECKtjPMDJLbarsHlm84513773 = -117353102;    double ECKtjPMDJLbarsHlm94849858 = 6067954;    double ECKtjPMDJLbarsHlm33842608 = -472806379;    double ECKtjPMDJLbarsHlm32682102 = 96976478;    double ECKtjPMDJLbarsHlm71043636 = -516921550;    double ECKtjPMDJLbarsHlm21626304 = -346866725;    double ECKtjPMDJLbarsHlm52430715 = -413232276;    double ECKtjPMDJLbarsHlm79079928 = -806149530;    double ECKtjPMDJLbarsHlm67970314 = -354442127;    double ECKtjPMDJLbarsHlm94137356 = -20745747;    double ECKtjPMDJLbarsHlm80318048 = -362589148;    double ECKtjPMDJLbarsHlm11933078 = -218528237;    double ECKtjPMDJLbarsHlm90390843 = -524918405;    double ECKtjPMDJLbarsHlm90383653 = -499484457;    double ECKtjPMDJLbarsHlm19248064 = -653791073;    double ECKtjPMDJLbarsHlm17164172 = -547810590;    double ECKtjPMDJLbarsHlm46437264 = -523584343;    double ECKtjPMDJLbarsHlm43879844 = -885688709;    double ECKtjPMDJLbarsHlm66346716 = -624171471;    double ECKtjPMDJLbarsHlm59251387 = -516673783;    double ECKtjPMDJLbarsHlm51430015 = -571271730;    double ECKtjPMDJLbarsHlm45610047 = -945002903;    double ECKtjPMDJLbarsHlm26567860 = -386455378;    double ECKtjPMDJLbarsHlm13872070 = -245641900;    double ECKtjPMDJLbarsHlm28952495 = -423646928;    double ECKtjPMDJLbarsHlm98756011 = -977620914;    double ECKtjPMDJLbarsHlm94236918 = -117105709;     ECKtjPMDJLbarsHlm5701732 = ECKtjPMDJLbarsHlm32847865;     ECKtjPMDJLbarsHlm32847865 = ECKtjPMDJLbarsHlm27413254;     ECKtjPMDJLbarsHlm27413254 = ECKtjPMDJLbarsHlm37751643;     ECKtjPMDJLbarsHlm37751643 = ECKtjPMDJLbarsHlm27211335;     ECKtjPMDJLbarsHlm27211335 = ECKtjPMDJLbarsHlm29560739;     ECKtjPMDJLbarsHlm29560739 = ECKtjPMDJLbarsHlm66908438;     ECKtjPMDJLbarsHlm66908438 = ECKtjPMDJLbarsHlm23322564;     ECKtjPMDJLbarsHlm23322564 = ECKtjPMDJLbarsHlm11256717;     ECKtjPMDJLbarsHlm11256717 = ECKtjPMDJLbarsHlm27268493;     ECKtjPMDJLbarsHlm27268493 = ECKtjPMDJLbarsHlm3591425;     ECKtjPMDJLbarsHlm3591425 = ECKtjPMDJLbarsHlm52319182;     ECKtjPMDJLbarsHlm52319182 = ECKtjPMDJLbarsHlm29251345;     ECKtjPMDJLbarsHlm29251345 = ECKtjPMDJLbarsHlm31369647;     ECKtjPMDJLbarsHlm31369647 = ECKtjPMDJLbarsHlm56025688;     ECKtjPMDJLbarsHlm56025688 = ECKtjPMDJLbarsHlm38289221;     ECKtjPMDJLbarsHlm38289221 = ECKtjPMDJLbarsHlm88871956;     ECKtjPMDJLbarsHlm88871956 = ECKtjPMDJLbarsHlm22393088;     ECKtjPMDJLbarsHlm22393088 = ECKtjPMDJLbarsHlm62746896;     ECKtjPMDJLbarsHlm62746896 = ECKtjPMDJLbarsHlm44883823;     ECKtjPMDJLbarsHlm44883823 = ECKtjPMDJLbarsHlm65123495;     ECKtjPMDJLbarsHlm65123495 = ECKtjPMDJLbarsHlm44291974;     ECKtjPMDJLbarsHlm44291974 = ECKtjPMDJLbarsHlm18132416;     ECKtjPMDJLbarsHlm18132416 = ECKtjPMDJLbarsHlm14202536;     ECKtjPMDJLbarsHlm14202536 = ECKtjPMDJLbarsHlm3347930;     ECKtjPMDJLbarsHlm3347930 = ECKtjPMDJLbarsHlm20883502;     ECKtjPMDJLbarsHlm20883502 = ECKtjPMDJLbarsHlm96937662;     ECKtjPMDJLbarsHlm96937662 = ECKtjPMDJLbarsHlm43442333;     ECKtjPMDJLbarsHlm43442333 = ECKtjPMDJLbarsHlm5913747;     ECKtjPMDJLbarsHlm5913747 = ECKtjPMDJLbarsHlm28599339;     ECKtjPMDJLbarsHlm28599339 = ECKtjPMDJLbarsHlm78935724;     ECKtjPMDJLbarsHlm78935724 = ECKtjPMDJLbarsHlm5252111;     ECKtjPMDJLbarsHlm5252111 = ECKtjPMDJLbarsHlm81665085;     ECKtjPMDJLbarsHlm81665085 = ECKtjPMDJLbarsHlm18556903;     ECKtjPMDJLbarsHlm18556903 = ECKtjPMDJLbarsHlm7847022;     ECKtjPMDJLbarsHlm7847022 = ECKtjPMDJLbarsHlm58595292;     ECKtjPMDJLbarsHlm58595292 = ECKtjPMDJLbarsHlm79557434;     ECKtjPMDJLbarsHlm79557434 = ECKtjPMDJLbarsHlm93655100;     ECKtjPMDJLbarsHlm93655100 = ECKtjPMDJLbarsHlm57757848;     ECKtjPMDJLbarsHlm57757848 = ECKtjPMDJLbarsHlm78233123;     ECKtjPMDJLbarsHlm78233123 = ECKtjPMDJLbarsHlm50033965;     ECKtjPMDJLbarsHlm50033965 = ECKtjPMDJLbarsHlm31280887;     ECKtjPMDJLbarsHlm31280887 = ECKtjPMDJLbarsHlm11609872;     ECKtjPMDJLbarsHlm11609872 = ECKtjPMDJLbarsHlm47088779;     ECKtjPMDJLbarsHlm47088779 = ECKtjPMDJLbarsHlm92576231;     ECKtjPMDJLbarsHlm92576231 = ECKtjPMDJLbarsHlm50917214;     ECKtjPMDJLbarsHlm50917214 = ECKtjPMDJLbarsHlm41803574;     ECKtjPMDJLbarsHlm41803574 = ECKtjPMDJLbarsHlm28967348;     ECKtjPMDJLbarsHlm28967348 = ECKtjPMDJLbarsHlm49304977;     ECKtjPMDJLbarsHlm49304977 = ECKtjPMDJLbarsHlm25595698;     ECKtjPMDJLbarsHlm25595698 = ECKtjPMDJLbarsHlm16666261;     ECKtjPMDJLbarsHlm16666261 = ECKtjPMDJLbarsHlm88544881;     ECKtjPMDJLbarsHlm88544881 = ECKtjPMDJLbarsHlm14868458;     ECKtjPMDJLbarsHlm14868458 = ECKtjPMDJLbarsHlm62417022;     ECKtjPMDJLbarsHlm62417022 = ECKtjPMDJLbarsHlm1392732;     ECKtjPMDJLbarsHlm1392732 = ECKtjPMDJLbarsHlm61409757;     ECKtjPMDJLbarsHlm61409757 = ECKtjPMDJLbarsHlm14715449;     ECKtjPMDJLbarsHlm14715449 = ECKtjPMDJLbarsHlm13210719;     ECKtjPMDJLbarsHlm13210719 = ECKtjPMDJLbarsHlm34403713;     ECKtjPMDJLbarsHlm34403713 = ECKtjPMDJLbarsHlm6327833;     ECKtjPMDJLbarsHlm6327833 = ECKtjPMDJLbarsHlm32623076;     ECKtjPMDJLbarsHlm32623076 = ECKtjPMDJLbarsHlm23466106;     ECKtjPMDJLbarsHlm23466106 = ECKtjPMDJLbarsHlm17408817;     ECKtjPMDJLbarsHlm17408817 = ECKtjPMDJLbarsHlm82657377;     ECKtjPMDJLbarsHlm82657377 = ECKtjPMDJLbarsHlm48332768;     ECKtjPMDJLbarsHlm48332768 = ECKtjPMDJLbarsHlm98339313;     ECKtjPMDJLbarsHlm98339313 = ECKtjPMDJLbarsHlm70654097;     ECKtjPMDJLbarsHlm70654097 = ECKtjPMDJLbarsHlm10694442;     ECKtjPMDJLbarsHlm10694442 = ECKtjPMDJLbarsHlm23522626;     ECKtjPMDJLbarsHlm23522626 = ECKtjPMDJLbarsHlm97430395;     ECKtjPMDJLbarsHlm97430395 = ECKtjPMDJLbarsHlm58731787;     ECKtjPMDJLbarsHlm58731787 = ECKtjPMDJLbarsHlm95216855;     ECKtjPMDJLbarsHlm95216855 = ECKtjPMDJLbarsHlm64635239;     ECKtjPMDJLbarsHlm64635239 = ECKtjPMDJLbarsHlm84513773;     ECKtjPMDJLbarsHlm84513773 = ECKtjPMDJLbarsHlm94849858;     ECKtjPMDJLbarsHlm94849858 = ECKtjPMDJLbarsHlm33842608;     ECKtjPMDJLbarsHlm33842608 = ECKtjPMDJLbarsHlm32682102;     ECKtjPMDJLbarsHlm32682102 = ECKtjPMDJLbarsHlm71043636;     ECKtjPMDJLbarsHlm71043636 = ECKtjPMDJLbarsHlm21626304;     ECKtjPMDJLbarsHlm21626304 = ECKtjPMDJLbarsHlm52430715;     ECKtjPMDJLbarsHlm52430715 = ECKtjPMDJLbarsHlm79079928;     ECKtjPMDJLbarsHlm79079928 = ECKtjPMDJLbarsHlm67970314;     ECKtjPMDJLbarsHlm67970314 = ECKtjPMDJLbarsHlm94137356;     ECKtjPMDJLbarsHlm94137356 = ECKtjPMDJLbarsHlm80318048;     ECKtjPMDJLbarsHlm80318048 = ECKtjPMDJLbarsHlm11933078;     ECKtjPMDJLbarsHlm11933078 = ECKtjPMDJLbarsHlm90390843;     ECKtjPMDJLbarsHlm90390843 = ECKtjPMDJLbarsHlm90383653;     ECKtjPMDJLbarsHlm90383653 = ECKtjPMDJLbarsHlm19248064;     ECKtjPMDJLbarsHlm19248064 = ECKtjPMDJLbarsHlm17164172;     ECKtjPMDJLbarsHlm17164172 = ECKtjPMDJLbarsHlm46437264;     ECKtjPMDJLbarsHlm46437264 = ECKtjPMDJLbarsHlm43879844;     ECKtjPMDJLbarsHlm43879844 = ECKtjPMDJLbarsHlm66346716;     ECKtjPMDJLbarsHlm66346716 = ECKtjPMDJLbarsHlm59251387;     ECKtjPMDJLbarsHlm59251387 = ECKtjPMDJLbarsHlm51430015;     ECKtjPMDJLbarsHlm51430015 = ECKtjPMDJLbarsHlm45610047;     ECKtjPMDJLbarsHlm45610047 = ECKtjPMDJLbarsHlm26567860;     ECKtjPMDJLbarsHlm26567860 = ECKtjPMDJLbarsHlm13872070;     ECKtjPMDJLbarsHlm13872070 = ECKtjPMDJLbarsHlm28952495;     ECKtjPMDJLbarsHlm28952495 = ECKtjPMDJLbarsHlm98756011;     ECKtjPMDJLbarsHlm98756011 = ECKtjPMDJLbarsHlm94236918;     ECKtjPMDJLbarsHlm94236918 = ECKtjPMDJLbarsHlm5701732;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void QBoTlCuhRMDOXmDS33807826() {     double tkDiPMSCVKtQYxZHX41160700 = -583382609;    double tkDiPMSCVKtQYxZHX46119943 = -538351160;    double tkDiPMSCVKtQYxZHX64799730 = -605775125;    double tkDiPMSCVKtQYxZHX9974998 = -250530684;    double tkDiPMSCVKtQYxZHX83496603 = -370095824;    double tkDiPMSCVKtQYxZHX65783611 = 73595561;    double tkDiPMSCVKtQYxZHX77565844 = -581719674;    double tkDiPMSCVKtQYxZHX85454124 = -310332606;    double tkDiPMSCVKtQYxZHX12774322 = -745092605;    double tkDiPMSCVKtQYxZHX42171205 = 78364360;    double tkDiPMSCVKtQYxZHX41773159 = -486003519;    double tkDiPMSCVKtQYxZHX52887331 = -383730839;    double tkDiPMSCVKtQYxZHX67485990 = -544583941;    double tkDiPMSCVKtQYxZHX84510510 = -74660433;    double tkDiPMSCVKtQYxZHX14220205 = -103698060;    double tkDiPMSCVKtQYxZHX21830522 = 75368770;    double tkDiPMSCVKtQYxZHX44707892 = -104358314;    double tkDiPMSCVKtQYxZHX22153280 = -665429620;    double tkDiPMSCVKtQYxZHX63297226 = -538596124;    double tkDiPMSCVKtQYxZHX38721042 = -789199895;    double tkDiPMSCVKtQYxZHX78210358 = -569762421;    double tkDiPMSCVKtQYxZHX47713499 = -521169923;    double tkDiPMSCVKtQYxZHX88657795 = -708799207;    double tkDiPMSCVKtQYxZHX7995481 = -662885632;    double tkDiPMSCVKtQYxZHX11365881 = -434929126;    double tkDiPMSCVKtQYxZHX31581673 = -772341027;    double tkDiPMSCVKtQYxZHX57358237 = -410663986;    double tkDiPMSCVKtQYxZHX84170757 = 75485291;    double tkDiPMSCVKtQYxZHX52581961 = -46153814;    double tkDiPMSCVKtQYxZHX34132750 = -283405188;    double tkDiPMSCVKtQYxZHX97518956 = -232304570;    double tkDiPMSCVKtQYxZHX43075129 = -731945731;    double tkDiPMSCVKtQYxZHX93883096 = -352432098;    double tkDiPMSCVKtQYxZHX48305329 = -763410573;    double tkDiPMSCVKtQYxZHX73655475 = -157256673;    double tkDiPMSCVKtQYxZHX10156137 = -686886546;    double tkDiPMSCVKtQYxZHX28969251 = -176914532;    double tkDiPMSCVKtQYxZHX44870504 = -517765050;    double tkDiPMSCVKtQYxZHX88148409 = -502880764;    double tkDiPMSCVKtQYxZHX42504655 = -569942147;    double tkDiPMSCVKtQYxZHX83485572 = -37905697;    double tkDiPMSCVKtQYxZHX20646014 = 65773355;    double tkDiPMSCVKtQYxZHX993251 = 15309106;    double tkDiPMSCVKtQYxZHX87220117 = 41422117;    double tkDiPMSCVKtQYxZHX92592690 = -631705466;    double tkDiPMSCVKtQYxZHX96878542 = 63364716;    double tkDiPMSCVKtQYxZHX61148737 = -661601051;    double tkDiPMSCVKtQYxZHX46693660 = -316464312;    double tkDiPMSCVKtQYxZHX72197642 = -397733669;    double tkDiPMSCVKtQYxZHX50125287 = -600496680;    double tkDiPMSCVKtQYxZHX97436984 = -904646339;    double tkDiPMSCVKtQYxZHX72030353 = -369058025;    double tkDiPMSCVKtQYxZHX83489606 = -115535414;    double tkDiPMSCVKtQYxZHX78408868 = -742553548;    double tkDiPMSCVKtQYxZHX55597468 = -397396316;    double tkDiPMSCVKtQYxZHX93447201 = 37787314;    double tkDiPMSCVKtQYxZHX57462147 = -829551954;    double tkDiPMSCVKtQYxZHX56804250 = -942889493;    double tkDiPMSCVKtQYxZHX98609116 = -815601559;    double tkDiPMSCVKtQYxZHX51914930 = -597754797;    double tkDiPMSCVKtQYxZHX8425375 = -515740453;    double tkDiPMSCVKtQYxZHX93395086 = -557204965;    double tkDiPMSCVKtQYxZHX32872163 = -164178792;    double tkDiPMSCVKtQYxZHX78641572 = -361687417;    double tkDiPMSCVKtQYxZHX44652249 = -689331071;    double tkDiPMSCVKtQYxZHX98698030 = -754057788;    double tkDiPMSCVKtQYxZHX59004234 = 68701259;    double tkDiPMSCVKtQYxZHX19180661 = -781173368;    double tkDiPMSCVKtQYxZHX10855035 = -917403760;    double tkDiPMSCVKtQYxZHX4064069 = -416811514;    double tkDiPMSCVKtQYxZHX92861271 = -747716698;    double tkDiPMSCVKtQYxZHX99837388 = -586593265;    double tkDiPMSCVKtQYxZHX34004870 = -62548857;    double tkDiPMSCVKtQYxZHX20792571 = -968653977;    double tkDiPMSCVKtQYxZHX55235469 = -651294198;    double tkDiPMSCVKtQYxZHX57564344 = -535535777;    double tkDiPMSCVKtQYxZHX46720248 = -436479030;    double tkDiPMSCVKtQYxZHX1437679 = -650221324;    double tkDiPMSCVKtQYxZHX15402790 = 68819833;    double tkDiPMSCVKtQYxZHX14487338 = -398293842;    double tkDiPMSCVKtQYxZHX70432936 = -10739976;    double tkDiPMSCVKtQYxZHX10664577 = 5800326;    double tkDiPMSCVKtQYxZHX11973116 = -526781041;    double tkDiPMSCVKtQYxZHX2456674 = -445657134;    double tkDiPMSCVKtQYxZHX36695765 = -378758850;    double tkDiPMSCVKtQYxZHX25488603 = -863246546;    double tkDiPMSCVKtQYxZHX59585523 = -516410317;    double tkDiPMSCVKtQYxZHX15474228 = -609878550;    double tkDiPMSCVKtQYxZHX92707861 = -266014257;    double tkDiPMSCVKtQYxZHX80208274 = -95043988;    double tkDiPMSCVKtQYxZHX52693989 = -857334593;    double tkDiPMSCVKtQYxZHX72165001 = -234025040;    double tkDiPMSCVKtQYxZHX46261387 = -702163491;    double tkDiPMSCVKtQYxZHX36233479 = -905125967;    double tkDiPMSCVKtQYxZHX34079281 = 45798306;    double tkDiPMSCVKtQYxZHX90090486 = -480700733;    double tkDiPMSCVKtQYxZHX87773850 = -770047853;    double tkDiPMSCVKtQYxZHX22351679 = -623003477;    double tkDiPMSCVKtQYxZHX42567869 = -269246813;    double tkDiPMSCVKtQYxZHX93894660 = -583382609;     tkDiPMSCVKtQYxZHX41160700 = tkDiPMSCVKtQYxZHX46119943;     tkDiPMSCVKtQYxZHX46119943 = tkDiPMSCVKtQYxZHX64799730;     tkDiPMSCVKtQYxZHX64799730 = tkDiPMSCVKtQYxZHX9974998;     tkDiPMSCVKtQYxZHX9974998 = tkDiPMSCVKtQYxZHX83496603;     tkDiPMSCVKtQYxZHX83496603 = tkDiPMSCVKtQYxZHX65783611;     tkDiPMSCVKtQYxZHX65783611 = tkDiPMSCVKtQYxZHX77565844;     tkDiPMSCVKtQYxZHX77565844 = tkDiPMSCVKtQYxZHX85454124;     tkDiPMSCVKtQYxZHX85454124 = tkDiPMSCVKtQYxZHX12774322;     tkDiPMSCVKtQYxZHX12774322 = tkDiPMSCVKtQYxZHX42171205;     tkDiPMSCVKtQYxZHX42171205 = tkDiPMSCVKtQYxZHX41773159;     tkDiPMSCVKtQYxZHX41773159 = tkDiPMSCVKtQYxZHX52887331;     tkDiPMSCVKtQYxZHX52887331 = tkDiPMSCVKtQYxZHX67485990;     tkDiPMSCVKtQYxZHX67485990 = tkDiPMSCVKtQYxZHX84510510;     tkDiPMSCVKtQYxZHX84510510 = tkDiPMSCVKtQYxZHX14220205;     tkDiPMSCVKtQYxZHX14220205 = tkDiPMSCVKtQYxZHX21830522;     tkDiPMSCVKtQYxZHX21830522 = tkDiPMSCVKtQYxZHX44707892;     tkDiPMSCVKtQYxZHX44707892 = tkDiPMSCVKtQYxZHX22153280;     tkDiPMSCVKtQYxZHX22153280 = tkDiPMSCVKtQYxZHX63297226;     tkDiPMSCVKtQYxZHX63297226 = tkDiPMSCVKtQYxZHX38721042;     tkDiPMSCVKtQYxZHX38721042 = tkDiPMSCVKtQYxZHX78210358;     tkDiPMSCVKtQYxZHX78210358 = tkDiPMSCVKtQYxZHX47713499;     tkDiPMSCVKtQYxZHX47713499 = tkDiPMSCVKtQYxZHX88657795;     tkDiPMSCVKtQYxZHX88657795 = tkDiPMSCVKtQYxZHX7995481;     tkDiPMSCVKtQYxZHX7995481 = tkDiPMSCVKtQYxZHX11365881;     tkDiPMSCVKtQYxZHX11365881 = tkDiPMSCVKtQYxZHX31581673;     tkDiPMSCVKtQYxZHX31581673 = tkDiPMSCVKtQYxZHX57358237;     tkDiPMSCVKtQYxZHX57358237 = tkDiPMSCVKtQYxZHX84170757;     tkDiPMSCVKtQYxZHX84170757 = tkDiPMSCVKtQYxZHX52581961;     tkDiPMSCVKtQYxZHX52581961 = tkDiPMSCVKtQYxZHX34132750;     tkDiPMSCVKtQYxZHX34132750 = tkDiPMSCVKtQYxZHX97518956;     tkDiPMSCVKtQYxZHX97518956 = tkDiPMSCVKtQYxZHX43075129;     tkDiPMSCVKtQYxZHX43075129 = tkDiPMSCVKtQYxZHX93883096;     tkDiPMSCVKtQYxZHX93883096 = tkDiPMSCVKtQYxZHX48305329;     tkDiPMSCVKtQYxZHX48305329 = tkDiPMSCVKtQYxZHX73655475;     tkDiPMSCVKtQYxZHX73655475 = tkDiPMSCVKtQYxZHX10156137;     tkDiPMSCVKtQYxZHX10156137 = tkDiPMSCVKtQYxZHX28969251;     tkDiPMSCVKtQYxZHX28969251 = tkDiPMSCVKtQYxZHX44870504;     tkDiPMSCVKtQYxZHX44870504 = tkDiPMSCVKtQYxZHX88148409;     tkDiPMSCVKtQYxZHX88148409 = tkDiPMSCVKtQYxZHX42504655;     tkDiPMSCVKtQYxZHX42504655 = tkDiPMSCVKtQYxZHX83485572;     tkDiPMSCVKtQYxZHX83485572 = tkDiPMSCVKtQYxZHX20646014;     tkDiPMSCVKtQYxZHX20646014 = tkDiPMSCVKtQYxZHX993251;     tkDiPMSCVKtQYxZHX993251 = tkDiPMSCVKtQYxZHX87220117;     tkDiPMSCVKtQYxZHX87220117 = tkDiPMSCVKtQYxZHX92592690;     tkDiPMSCVKtQYxZHX92592690 = tkDiPMSCVKtQYxZHX96878542;     tkDiPMSCVKtQYxZHX96878542 = tkDiPMSCVKtQYxZHX61148737;     tkDiPMSCVKtQYxZHX61148737 = tkDiPMSCVKtQYxZHX46693660;     tkDiPMSCVKtQYxZHX46693660 = tkDiPMSCVKtQYxZHX72197642;     tkDiPMSCVKtQYxZHX72197642 = tkDiPMSCVKtQYxZHX50125287;     tkDiPMSCVKtQYxZHX50125287 = tkDiPMSCVKtQYxZHX97436984;     tkDiPMSCVKtQYxZHX97436984 = tkDiPMSCVKtQYxZHX72030353;     tkDiPMSCVKtQYxZHX72030353 = tkDiPMSCVKtQYxZHX83489606;     tkDiPMSCVKtQYxZHX83489606 = tkDiPMSCVKtQYxZHX78408868;     tkDiPMSCVKtQYxZHX78408868 = tkDiPMSCVKtQYxZHX55597468;     tkDiPMSCVKtQYxZHX55597468 = tkDiPMSCVKtQYxZHX93447201;     tkDiPMSCVKtQYxZHX93447201 = tkDiPMSCVKtQYxZHX57462147;     tkDiPMSCVKtQYxZHX57462147 = tkDiPMSCVKtQYxZHX56804250;     tkDiPMSCVKtQYxZHX56804250 = tkDiPMSCVKtQYxZHX98609116;     tkDiPMSCVKtQYxZHX98609116 = tkDiPMSCVKtQYxZHX51914930;     tkDiPMSCVKtQYxZHX51914930 = tkDiPMSCVKtQYxZHX8425375;     tkDiPMSCVKtQYxZHX8425375 = tkDiPMSCVKtQYxZHX93395086;     tkDiPMSCVKtQYxZHX93395086 = tkDiPMSCVKtQYxZHX32872163;     tkDiPMSCVKtQYxZHX32872163 = tkDiPMSCVKtQYxZHX78641572;     tkDiPMSCVKtQYxZHX78641572 = tkDiPMSCVKtQYxZHX44652249;     tkDiPMSCVKtQYxZHX44652249 = tkDiPMSCVKtQYxZHX98698030;     tkDiPMSCVKtQYxZHX98698030 = tkDiPMSCVKtQYxZHX59004234;     tkDiPMSCVKtQYxZHX59004234 = tkDiPMSCVKtQYxZHX19180661;     tkDiPMSCVKtQYxZHX19180661 = tkDiPMSCVKtQYxZHX10855035;     tkDiPMSCVKtQYxZHX10855035 = tkDiPMSCVKtQYxZHX4064069;     tkDiPMSCVKtQYxZHX4064069 = tkDiPMSCVKtQYxZHX92861271;     tkDiPMSCVKtQYxZHX92861271 = tkDiPMSCVKtQYxZHX99837388;     tkDiPMSCVKtQYxZHX99837388 = tkDiPMSCVKtQYxZHX34004870;     tkDiPMSCVKtQYxZHX34004870 = tkDiPMSCVKtQYxZHX20792571;     tkDiPMSCVKtQYxZHX20792571 = tkDiPMSCVKtQYxZHX55235469;     tkDiPMSCVKtQYxZHX55235469 = tkDiPMSCVKtQYxZHX57564344;     tkDiPMSCVKtQYxZHX57564344 = tkDiPMSCVKtQYxZHX46720248;     tkDiPMSCVKtQYxZHX46720248 = tkDiPMSCVKtQYxZHX1437679;     tkDiPMSCVKtQYxZHX1437679 = tkDiPMSCVKtQYxZHX15402790;     tkDiPMSCVKtQYxZHX15402790 = tkDiPMSCVKtQYxZHX14487338;     tkDiPMSCVKtQYxZHX14487338 = tkDiPMSCVKtQYxZHX70432936;     tkDiPMSCVKtQYxZHX70432936 = tkDiPMSCVKtQYxZHX10664577;     tkDiPMSCVKtQYxZHX10664577 = tkDiPMSCVKtQYxZHX11973116;     tkDiPMSCVKtQYxZHX11973116 = tkDiPMSCVKtQYxZHX2456674;     tkDiPMSCVKtQYxZHX2456674 = tkDiPMSCVKtQYxZHX36695765;     tkDiPMSCVKtQYxZHX36695765 = tkDiPMSCVKtQYxZHX25488603;     tkDiPMSCVKtQYxZHX25488603 = tkDiPMSCVKtQYxZHX59585523;     tkDiPMSCVKtQYxZHX59585523 = tkDiPMSCVKtQYxZHX15474228;     tkDiPMSCVKtQYxZHX15474228 = tkDiPMSCVKtQYxZHX92707861;     tkDiPMSCVKtQYxZHX92707861 = tkDiPMSCVKtQYxZHX80208274;     tkDiPMSCVKtQYxZHX80208274 = tkDiPMSCVKtQYxZHX52693989;     tkDiPMSCVKtQYxZHX52693989 = tkDiPMSCVKtQYxZHX72165001;     tkDiPMSCVKtQYxZHX72165001 = tkDiPMSCVKtQYxZHX46261387;     tkDiPMSCVKtQYxZHX46261387 = tkDiPMSCVKtQYxZHX36233479;     tkDiPMSCVKtQYxZHX36233479 = tkDiPMSCVKtQYxZHX34079281;     tkDiPMSCVKtQYxZHX34079281 = tkDiPMSCVKtQYxZHX90090486;     tkDiPMSCVKtQYxZHX90090486 = tkDiPMSCVKtQYxZHX87773850;     tkDiPMSCVKtQYxZHX87773850 = tkDiPMSCVKtQYxZHX22351679;     tkDiPMSCVKtQYxZHX22351679 = tkDiPMSCVKtQYxZHX42567869;     tkDiPMSCVKtQYxZHX42567869 = tkDiPMSCVKtQYxZHX93894660;     tkDiPMSCVKtQYxZHX93894660 = tkDiPMSCVKtQYxZHX41160700;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZRNselZWGvYYcVXi48856894() {     double RzyPxwpxDTuKBIssk47277807 = -695284498;    double RzyPxwpxDTuKBIssk89493222 = -247950941;    double RzyPxwpxDTuKBIssk80841914 = -756608046;    double RzyPxwpxDTuKBIssk37049779 = -441279315;    double RzyPxwpxDTuKBIssk52392092 = -890745695;    double RzyPxwpxDTuKBIssk57200813 = -190900674;    double RzyPxwpxDTuKBIssk38054811 = -720684651;    double RzyPxwpxDTuKBIssk35214015 = -597206519;    double RzyPxwpxDTuKBIssk77416817 = -513125270;    double RzyPxwpxDTuKBIssk83400864 = -250611118;    double RzyPxwpxDTuKBIssk1883450 = -363441105;    double RzyPxwpxDTuKBIssk84710373 = -828137041;    double RzyPxwpxDTuKBIssk53695109 = -984475029;    double RzyPxwpxDTuKBIssk67654187 = -606899001;    double RzyPxwpxDTuKBIssk9962632 = -794160226;    double RzyPxwpxDTuKBIssk63335916 = -997630773;    double RzyPxwpxDTuKBIssk38543895 = -720903427;    double RzyPxwpxDTuKBIssk19206433 = -330952263;    double RzyPxwpxDTuKBIssk25127068 = -944365221;    double RzyPxwpxDTuKBIssk19271207 = -46102814;    double RzyPxwpxDTuKBIssk66763551 = -425249877;    double RzyPxwpxDTuKBIssk34234523 = -429099639;    double RzyPxwpxDTuKBIssk24248588 = -455316694;    double RzyPxwpxDTuKBIssk91987854 = -885190620;    double RzyPxwpxDTuKBIssk55065531 = -808125159;    double RzyPxwpxDTuKBIssk56303348 = -650996350;    double RzyPxwpxDTuKBIssk55818015 = -311326189;    double RzyPxwpxDTuKBIssk97277883 = 9556337;    double RzyPxwpxDTuKBIssk18714217 = -268536519;    double RzyPxwpxDTuKBIssk2748387 = -100876056;    double RzyPxwpxDTuKBIssk89761505 = -145625508;    double RzyPxwpxDTuKBIssk40685751 = -441392547;    double RzyPxwpxDTuKBIssk3594045 = -245045468;    double RzyPxwpxDTuKBIssk43135894 = -514952338;    double RzyPxwpxDTuKBIssk35130128 = -795989779;    double RzyPxwpxDTuKBIssk78882049 = -900912654;    double RzyPxwpxDTuKBIssk10231186 = -130550402;    double RzyPxwpxDTuKBIssk78960716 = -289271060;    double RzyPxwpxDTuKBIssk61758663 = -496804261;    double RzyPxwpxDTuKBIssk30006593 = 56618630;    double RzyPxwpxDTuKBIssk89154796 = -103179592;    double RzyPxwpxDTuKBIssk52796471 = -204368923;    double RzyPxwpxDTuKBIssk79060228 = -65495478;    double RzyPxwpxDTuKBIssk74627858 = 22079179;    double RzyPxwpxDTuKBIssk4545156 = -293603620;    double RzyPxwpxDTuKBIssk8817738 = 78148600;    double RzyPxwpxDTuKBIssk78927311 = -780376589;    double RzyPxwpxDTuKBIssk54650601 = -326359844;    double RzyPxwpxDTuKBIssk81873371 = -483470657;    double RzyPxwpxDTuKBIssk63036940 = -687507489;    double RzyPxwpxDTuKBIssk33137663 = -746940010;    double RzyPxwpxDTuKBIssk82840073 = -169784345;    double RzyPxwpxDTuKBIssk31738064 = 92674998;    double RzyPxwpxDTuKBIssk74377749 = 36976307;    double RzyPxwpxDTuKBIssk82093342 = 2071902;    double RzyPxwpxDTuKBIssk13043285 = -166184859;    double RzyPxwpxDTuKBIssk65244635 = -792634248;    double RzyPxwpxDTuKBIssk88854060 = -871417426;    double RzyPxwpxDTuKBIssk81984247 = -633154156;    double RzyPxwpxDTuKBIssk96088743 = -139749345;    double RzyPxwpxDTuKBIssk1382799 = -879574485;    double RzyPxwpxDTuKBIssk40776927 = -630240989;    double RzyPxwpxDTuKBIssk16499799 = -228670000;    double RzyPxwpxDTuKBIssk74668430 = -312249215;    double RzyPxwpxDTuKBIssk93639358 = -4985610;    double RzyPxwpxDTuKBIssk61197698 = -922048559;    double RzyPxwpxDTuKBIssk81116328 = -483091573;    double RzyPxwpxDTuKBIssk10559216 = -369522692;    double RzyPxwpxDTuKBIssk32524060 = -810909222;    double RzyPxwpxDTuKBIssk31080582 = -893247573;    double RzyPxwpxDTuKBIssk53104730 = -767080372;    double RzyPxwpxDTuKBIssk59583179 = -331632368;    double RzyPxwpxDTuKBIssk57447769 = -834148002;    double RzyPxwpxDTuKBIssk95120474 = -900983852;    double RzyPxwpxDTuKBIssk30116410 = -942923223;    double RzyPxwpxDTuKBIssk13967081 = -120880955;    double RzyPxwpxDTuKBIssk55174294 = -263604162;    double RzyPxwpxDTuKBIssk49620730 = -377395873;    double RzyPxwpxDTuKBIssk87442699 = -491587001;    double RzyPxwpxDTuKBIssk46247793 = -786273759;    double RzyPxwpxDTuKBIssk77376037 = -870619762;    double RzyPxwpxDTuKBIssk1167414 = -984966345;    double RzyPxwpxDTuKBIssk15404512 = -506973007;    double RzyPxwpxDTuKBIssk55677277 = -581029031;    double RzyPxwpxDTuKBIssk69610723 = -353936046;    double RzyPxwpxDTuKBIssk6921433 = -975841164;    double RzyPxwpxDTuKBIssk8947688 = -434067545;    double RzyPxwpxDTuKBIssk29216296 = -182021775;    double RzyPxwpxDTuKBIssk61042551 = -417024240;    double RzyPxwpxDTuKBIssk22086844 = -529804921;    double RzyPxwpxDTuKBIssk13637415 = -8278406;    double RzyPxwpxDTuKBIssk21377126 = -259132976;    double RzyPxwpxDTuKBIssk96976468 = -656116904;    double RzyPxwpxDTuKBIssk65669920 = -257054916;    double RzyPxwpxDTuKBIssk28623795 = -63806885;    double RzyPxwpxDTuKBIssk48377869 = -472938603;    double RzyPxwpxDTuKBIssk36296672 = -975698923;    double RzyPxwpxDTuKBIssk4391798 = -753246263;    double RzyPxwpxDTuKBIssk80988499 = -972935212;    double RzyPxwpxDTuKBIssk43347457 = -695284498;     RzyPxwpxDTuKBIssk47277807 = RzyPxwpxDTuKBIssk89493222;     RzyPxwpxDTuKBIssk89493222 = RzyPxwpxDTuKBIssk80841914;     RzyPxwpxDTuKBIssk80841914 = RzyPxwpxDTuKBIssk37049779;     RzyPxwpxDTuKBIssk37049779 = RzyPxwpxDTuKBIssk52392092;     RzyPxwpxDTuKBIssk52392092 = RzyPxwpxDTuKBIssk57200813;     RzyPxwpxDTuKBIssk57200813 = RzyPxwpxDTuKBIssk38054811;     RzyPxwpxDTuKBIssk38054811 = RzyPxwpxDTuKBIssk35214015;     RzyPxwpxDTuKBIssk35214015 = RzyPxwpxDTuKBIssk77416817;     RzyPxwpxDTuKBIssk77416817 = RzyPxwpxDTuKBIssk83400864;     RzyPxwpxDTuKBIssk83400864 = RzyPxwpxDTuKBIssk1883450;     RzyPxwpxDTuKBIssk1883450 = RzyPxwpxDTuKBIssk84710373;     RzyPxwpxDTuKBIssk84710373 = RzyPxwpxDTuKBIssk53695109;     RzyPxwpxDTuKBIssk53695109 = RzyPxwpxDTuKBIssk67654187;     RzyPxwpxDTuKBIssk67654187 = RzyPxwpxDTuKBIssk9962632;     RzyPxwpxDTuKBIssk9962632 = RzyPxwpxDTuKBIssk63335916;     RzyPxwpxDTuKBIssk63335916 = RzyPxwpxDTuKBIssk38543895;     RzyPxwpxDTuKBIssk38543895 = RzyPxwpxDTuKBIssk19206433;     RzyPxwpxDTuKBIssk19206433 = RzyPxwpxDTuKBIssk25127068;     RzyPxwpxDTuKBIssk25127068 = RzyPxwpxDTuKBIssk19271207;     RzyPxwpxDTuKBIssk19271207 = RzyPxwpxDTuKBIssk66763551;     RzyPxwpxDTuKBIssk66763551 = RzyPxwpxDTuKBIssk34234523;     RzyPxwpxDTuKBIssk34234523 = RzyPxwpxDTuKBIssk24248588;     RzyPxwpxDTuKBIssk24248588 = RzyPxwpxDTuKBIssk91987854;     RzyPxwpxDTuKBIssk91987854 = RzyPxwpxDTuKBIssk55065531;     RzyPxwpxDTuKBIssk55065531 = RzyPxwpxDTuKBIssk56303348;     RzyPxwpxDTuKBIssk56303348 = RzyPxwpxDTuKBIssk55818015;     RzyPxwpxDTuKBIssk55818015 = RzyPxwpxDTuKBIssk97277883;     RzyPxwpxDTuKBIssk97277883 = RzyPxwpxDTuKBIssk18714217;     RzyPxwpxDTuKBIssk18714217 = RzyPxwpxDTuKBIssk2748387;     RzyPxwpxDTuKBIssk2748387 = RzyPxwpxDTuKBIssk89761505;     RzyPxwpxDTuKBIssk89761505 = RzyPxwpxDTuKBIssk40685751;     RzyPxwpxDTuKBIssk40685751 = RzyPxwpxDTuKBIssk3594045;     RzyPxwpxDTuKBIssk3594045 = RzyPxwpxDTuKBIssk43135894;     RzyPxwpxDTuKBIssk43135894 = RzyPxwpxDTuKBIssk35130128;     RzyPxwpxDTuKBIssk35130128 = RzyPxwpxDTuKBIssk78882049;     RzyPxwpxDTuKBIssk78882049 = RzyPxwpxDTuKBIssk10231186;     RzyPxwpxDTuKBIssk10231186 = RzyPxwpxDTuKBIssk78960716;     RzyPxwpxDTuKBIssk78960716 = RzyPxwpxDTuKBIssk61758663;     RzyPxwpxDTuKBIssk61758663 = RzyPxwpxDTuKBIssk30006593;     RzyPxwpxDTuKBIssk30006593 = RzyPxwpxDTuKBIssk89154796;     RzyPxwpxDTuKBIssk89154796 = RzyPxwpxDTuKBIssk52796471;     RzyPxwpxDTuKBIssk52796471 = RzyPxwpxDTuKBIssk79060228;     RzyPxwpxDTuKBIssk79060228 = RzyPxwpxDTuKBIssk74627858;     RzyPxwpxDTuKBIssk74627858 = RzyPxwpxDTuKBIssk4545156;     RzyPxwpxDTuKBIssk4545156 = RzyPxwpxDTuKBIssk8817738;     RzyPxwpxDTuKBIssk8817738 = RzyPxwpxDTuKBIssk78927311;     RzyPxwpxDTuKBIssk78927311 = RzyPxwpxDTuKBIssk54650601;     RzyPxwpxDTuKBIssk54650601 = RzyPxwpxDTuKBIssk81873371;     RzyPxwpxDTuKBIssk81873371 = RzyPxwpxDTuKBIssk63036940;     RzyPxwpxDTuKBIssk63036940 = RzyPxwpxDTuKBIssk33137663;     RzyPxwpxDTuKBIssk33137663 = RzyPxwpxDTuKBIssk82840073;     RzyPxwpxDTuKBIssk82840073 = RzyPxwpxDTuKBIssk31738064;     RzyPxwpxDTuKBIssk31738064 = RzyPxwpxDTuKBIssk74377749;     RzyPxwpxDTuKBIssk74377749 = RzyPxwpxDTuKBIssk82093342;     RzyPxwpxDTuKBIssk82093342 = RzyPxwpxDTuKBIssk13043285;     RzyPxwpxDTuKBIssk13043285 = RzyPxwpxDTuKBIssk65244635;     RzyPxwpxDTuKBIssk65244635 = RzyPxwpxDTuKBIssk88854060;     RzyPxwpxDTuKBIssk88854060 = RzyPxwpxDTuKBIssk81984247;     RzyPxwpxDTuKBIssk81984247 = RzyPxwpxDTuKBIssk96088743;     RzyPxwpxDTuKBIssk96088743 = RzyPxwpxDTuKBIssk1382799;     RzyPxwpxDTuKBIssk1382799 = RzyPxwpxDTuKBIssk40776927;     RzyPxwpxDTuKBIssk40776927 = RzyPxwpxDTuKBIssk16499799;     RzyPxwpxDTuKBIssk16499799 = RzyPxwpxDTuKBIssk74668430;     RzyPxwpxDTuKBIssk74668430 = RzyPxwpxDTuKBIssk93639358;     RzyPxwpxDTuKBIssk93639358 = RzyPxwpxDTuKBIssk61197698;     RzyPxwpxDTuKBIssk61197698 = RzyPxwpxDTuKBIssk81116328;     RzyPxwpxDTuKBIssk81116328 = RzyPxwpxDTuKBIssk10559216;     RzyPxwpxDTuKBIssk10559216 = RzyPxwpxDTuKBIssk32524060;     RzyPxwpxDTuKBIssk32524060 = RzyPxwpxDTuKBIssk31080582;     RzyPxwpxDTuKBIssk31080582 = RzyPxwpxDTuKBIssk53104730;     RzyPxwpxDTuKBIssk53104730 = RzyPxwpxDTuKBIssk59583179;     RzyPxwpxDTuKBIssk59583179 = RzyPxwpxDTuKBIssk57447769;     RzyPxwpxDTuKBIssk57447769 = RzyPxwpxDTuKBIssk95120474;     RzyPxwpxDTuKBIssk95120474 = RzyPxwpxDTuKBIssk30116410;     RzyPxwpxDTuKBIssk30116410 = RzyPxwpxDTuKBIssk13967081;     RzyPxwpxDTuKBIssk13967081 = RzyPxwpxDTuKBIssk55174294;     RzyPxwpxDTuKBIssk55174294 = RzyPxwpxDTuKBIssk49620730;     RzyPxwpxDTuKBIssk49620730 = RzyPxwpxDTuKBIssk87442699;     RzyPxwpxDTuKBIssk87442699 = RzyPxwpxDTuKBIssk46247793;     RzyPxwpxDTuKBIssk46247793 = RzyPxwpxDTuKBIssk77376037;     RzyPxwpxDTuKBIssk77376037 = RzyPxwpxDTuKBIssk1167414;     RzyPxwpxDTuKBIssk1167414 = RzyPxwpxDTuKBIssk15404512;     RzyPxwpxDTuKBIssk15404512 = RzyPxwpxDTuKBIssk55677277;     RzyPxwpxDTuKBIssk55677277 = RzyPxwpxDTuKBIssk69610723;     RzyPxwpxDTuKBIssk69610723 = RzyPxwpxDTuKBIssk6921433;     RzyPxwpxDTuKBIssk6921433 = RzyPxwpxDTuKBIssk8947688;     RzyPxwpxDTuKBIssk8947688 = RzyPxwpxDTuKBIssk29216296;     RzyPxwpxDTuKBIssk29216296 = RzyPxwpxDTuKBIssk61042551;     RzyPxwpxDTuKBIssk61042551 = RzyPxwpxDTuKBIssk22086844;     RzyPxwpxDTuKBIssk22086844 = RzyPxwpxDTuKBIssk13637415;     RzyPxwpxDTuKBIssk13637415 = RzyPxwpxDTuKBIssk21377126;     RzyPxwpxDTuKBIssk21377126 = RzyPxwpxDTuKBIssk96976468;     RzyPxwpxDTuKBIssk96976468 = RzyPxwpxDTuKBIssk65669920;     RzyPxwpxDTuKBIssk65669920 = RzyPxwpxDTuKBIssk28623795;     RzyPxwpxDTuKBIssk28623795 = RzyPxwpxDTuKBIssk48377869;     RzyPxwpxDTuKBIssk48377869 = RzyPxwpxDTuKBIssk36296672;     RzyPxwpxDTuKBIssk36296672 = RzyPxwpxDTuKBIssk4391798;     RzyPxwpxDTuKBIssk4391798 = RzyPxwpxDTuKBIssk80988499;     RzyPxwpxDTuKBIssk80988499 = RzyPxwpxDTuKBIssk43347457;     RzyPxwpxDTuKBIssk43347457 = RzyPxwpxDTuKBIssk47277807;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void sfUYQNcnQCfYnYDf16148493() {     double XkKsnWjssRcvKGcGu82736775 = -61561397;    double XkKsnWjssRcvKGcGu2765302 = -128502548;    double XkKsnWjssRcvKGcGu18228391 = -422275121;    double XkKsnWjssRcvKGcGu9273134 = -262430386;    double XkKsnWjssRcvKGcGu8677362 = -996344540;    double XkKsnWjssRcvKGcGu93423685 = -453248569;    double XkKsnWjssRcvKGcGu48712217 = -565410386;    double XkKsnWjssRcvKGcGu97345576 = -241440659;    double XkKsnWjssRcvKGcGu78934422 = 98543323;    double XkKsnWjssRcvKGcGu98303576 = 84701884;    double XkKsnWjssRcvKGcGu40065184 = -165448835;    double XkKsnWjssRcvKGcGu85278521 = -692257812;    double XkKsnWjssRcvKGcGu91929754 = -436487534;    double XkKsnWjssRcvKGcGu20795051 = -583262410;    double XkKsnWjssRcvKGcGu68157148 = -197585241;    double XkKsnWjssRcvKGcGu46877217 = -633442380;    double XkKsnWjssRcvKGcGu94379831 = -147877979;    double XkKsnWjssRcvKGcGu18966624 = -404380331;    double XkKsnWjssRcvKGcGu25677398 = -160928642;    double XkKsnWjssRcvKGcGu13108426 = -472663880;    double XkKsnWjssRcvKGcGu79850414 = -82524566;    double XkKsnWjssRcvKGcGu37656047 = -515735933;    double XkKsnWjssRcvKGcGu94773967 = -92948878;    double XkKsnWjssRcvKGcGu85780799 = -296015224;    double XkKsnWjssRcvKGcGu63083482 = -832110357;    double XkKsnWjssRcvKGcGu67001520 = -339573157;    double XkKsnWjssRcvKGcGu16238589 = -117610330;    double XkKsnWjssRcvKGcGu38006308 = -160215837;    double XkKsnWjssRcvKGcGu65382431 = -842631321;    double XkKsnWjssRcvKGcGu8281798 = -135383226;    double XkKsnWjssRcvKGcGu8344738 = -697605995;    double XkKsnWjssRcvKGcGu78508769 = -214363961;    double XkKsnWjssRcvKGcGu15812056 = -392641446;    double XkKsnWjssRcvKGcGu72884320 = -579646152;    double XkKsnWjssRcvKGcGu938582 = -747090029;    double XkKsnWjssRcvKGcGu30442894 = -15477959;    double XkKsnWjssRcvKGcGu59643002 = 4753558;    double XkKsnWjssRcvKGcGu30176120 = -271926641;    double XkKsnWjssRcvKGcGu92149225 = -147680537;    double XkKsnWjssRcvKGcGu94278125 = -508643916;    double XkKsnWjssRcvKGcGu22606405 = -972378506;    double XkKsnWjssRcvKGcGu42161597 = -898914214;    double XkKsnWjssRcvKGcGu68443607 = -718676264;    double XkKsnWjssRcvKGcGu14759197 = -582253231;    double XkKsnWjssRcvKGcGu4561614 = -120114781;    double XkKsnWjssRcvKGcGu54779066 = 39224967;    double XkKsnWjssRcvKGcGu98272475 = -164362949;    double XkKsnWjssRcvKGcGu72376913 = -492886438;    double XkKsnWjssRcvKGcGu4766038 = -147207537;    double XkKsnWjssRcvKGcGu87566529 = -78534304;    double XkKsnWjssRcvKGcGu13908387 = -621216568;    double XkKsnWjssRcvKGcGu66325545 = -383436691;    double XkKsnWjssRcvKGcGu359213 = -763370556;    double XkKsnWjssRcvKGcGu90369595 = -154532193;    double XkKsnWjssRcvKGcGu36298079 = -344418244;    double XkKsnWjssRcvKGcGu45080728 = -545825465;    double XkKsnWjssRcvKGcGu7991334 = 64446330;    double XkKsnWjssRcvKGcGu32447592 = -26259897;    double XkKsnWjssRcvKGcGu46189651 = -430320029;    double XkKsnWjssRcvKGcGu41675841 = -556771384;    double XkKsnWjssRcvKGcGu77185096 = -235638240;    double XkKsnWjssRcvKGcGu10705909 = -305194549;    double XkKsnWjssRcvKGcGu31963145 = -398809338;    double XkKsnWjssRcvKGcGu70652625 = -766073452;    double XkKsnWjssRcvKGcGu89958839 = -217692121;    double XkKsnWjssRcvKGcGu61556415 = -951084874;    double XkKsnWjssRcvKGcGu69466466 = -199616367;    double XkKsnWjssRcvKGcGu19045435 = -856841383;    double XkKsnWjssRcvKGcGu19856470 = -836172381;    double XkKsnWjssRcvKGcGu37714255 = -82107282;    double XkKsnWjssRcvKGcGu87234214 = -538195939;    double XkKsnWjssRcvKGcGu64203711 = -875951339;    double XkKsnWjssRcvKGcGu26817399 = -156699795;    double XkKsnWjssRcvKGcGu31399273 = -652284727;    double XkKsnWjssRcvKGcGu90502020 = -500285374;    double XkKsnWjssRcvKGcGu37688817 = -183610352;    double XkKsnWjssRcvKGcGu69212440 = -797059670;    double XkKsnWjssRcvKGcGu80014771 = -510695647;    double XkKsnWjssRcvKGcGu81219185 = -75900444;    double XkKsnWjssRcvKGcGu8304416 = -771335325;    double XkKsnWjssRcvKGcGu68729045 = -75210208;    double XkKsnWjssRcvKGcGu43861676 = -624723892;    double XkKsnWjssRcvKGcGu33240271 = 86991699;    double XkKsnWjssRcvKGcGu77815902 = -664097018;    double XkKsnWjssRcvKGcGu94373410 = -514166659;    double XkKsnWjssRcvKGcGu42019192 = -214169304;    double XkKsnWjssRcvKGcGu78149557 = -450993405;    double XkKsnWjssRcvKGcGu25442460 = -138109253;    double XkKsnWjssRcvKGcGu36586241 = -135227908;    double XkKsnWjssRcvKGcGu55857853 = -101264565;    double XkKsnWjssRcvKGcGu22451560 = 20075710;    double XkKsnWjssRcvKGcGu27195411 = -968986546;    double XkKsnWjssRcvKGcGu83986468 = -841606612;    double XkKsnWjssRcvKGcGu50473384 = -590909154;    double XkKsnWjssRcvKGcGu17093029 = -173005676;    double XkKsnWjssRcvKGcGu11900496 = -567183958;    double XkKsnWjssRcvKGcGu10198453 = -400104876;    double XkKsnWjssRcvKGcGu97790982 = -952602812;    double XkKsnWjssRcvKGcGu24800357 = -264561111;    double XkKsnWjssRcvKGcGu43005199 = -61561397;     XkKsnWjssRcvKGcGu82736775 = XkKsnWjssRcvKGcGu2765302;     XkKsnWjssRcvKGcGu2765302 = XkKsnWjssRcvKGcGu18228391;     XkKsnWjssRcvKGcGu18228391 = XkKsnWjssRcvKGcGu9273134;     XkKsnWjssRcvKGcGu9273134 = XkKsnWjssRcvKGcGu8677362;     XkKsnWjssRcvKGcGu8677362 = XkKsnWjssRcvKGcGu93423685;     XkKsnWjssRcvKGcGu93423685 = XkKsnWjssRcvKGcGu48712217;     XkKsnWjssRcvKGcGu48712217 = XkKsnWjssRcvKGcGu97345576;     XkKsnWjssRcvKGcGu97345576 = XkKsnWjssRcvKGcGu78934422;     XkKsnWjssRcvKGcGu78934422 = XkKsnWjssRcvKGcGu98303576;     XkKsnWjssRcvKGcGu98303576 = XkKsnWjssRcvKGcGu40065184;     XkKsnWjssRcvKGcGu40065184 = XkKsnWjssRcvKGcGu85278521;     XkKsnWjssRcvKGcGu85278521 = XkKsnWjssRcvKGcGu91929754;     XkKsnWjssRcvKGcGu91929754 = XkKsnWjssRcvKGcGu20795051;     XkKsnWjssRcvKGcGu20795051 = XkKsnWjssRcvKGcGu68157148;     XkKsnWjssRcvKGcGu68157148 = XkKsnWjssRcvKGcGu46877217;     XkKsnWjssRcvKGcGu46877217 = XkKsnWjssRcvKGcGu94379831;     XkKsnWjssRcvKGcGu94379831 = XkKsnWjssRcvKGcGu18966624;     XkKsnWjssRcvKGcGu18966624 = XkKsnWjssRcvKGcGu25677398;     XkKsnWjssRcvKGcGu25677398 = XkKsnWjssRcvKGcGu13108426;     XkKsnWjssRcvKGcGu13108426 = XkKsnWjssRcvKGcGu79850414;     XkKsnWjssRcvKGcGu79850414 = XkKsnWjssRcvKGcGu37656047;     XkKsnWjssRcvKGcGu37656047 = XkKsnWjssRcvKGcGu94773967;     XkKsnWjssRcvKGcGu94773967 = XkKsnWjssRcvKGcGu85780799;     XkKsnWjssRcvKGcGu85780799 = XkKsnWjssRcvKGcGu63083482;     XkKsnWjssRcvKGcGu63083482 = XkKsnWjssRcvKGcGu67001520;     XkKsnWjssRcvKGcGu67001520 = XkKsnWjssRcvKGcGu16238589;     XkKsnWjssRcvKGcGu16238589 = XkKsnWjssRcvKGcGu38006308;     XkKsnWjssRcvKGcGu38006308 = XkKsnWjssRcvKGcGu65382431;     XkKsnWjssRcvKGcGu65382431 = XkKsnWjssRcvKGcGu8281798;     XkKsnWjssRcvKGcGu8281798 = XkKsnWjssRcvKGcGu8344738;     XkKsnWjssRcvKGcGu8344738 = XkKsnWjssRcvKGcGu78508769;     XkKsnWjssRcvKGcGu78508769 = XkKsnWjssRcvKGcGu15812056;     XkKsnWjssRcvKGcGu15812056 = XkKsnWjssRcvKGcGu72884320;     XkKsnWjssRcvKGcGu72884320 = XkKsnWjssRcvKGcGu938582;     XkKsnWjssRcvKGcGu938582 = XkKsnWjssRcvKGcGu30442894;     XkKsnWjssRcvKGcGu30442894 = XkKsnWjssRcvKGcGu59643002;     XkKsnWjssRcvKGcGu59643002 = XkKsnWjssRcvKGcGu30176120;     XkKsnWjssRcvKGcGu30176120 = XkKsnWjssRcvKGcGu92149225;     XkKsnWjssRcvKGcGu92149225 = XkKsnWjssRcvKGcGu94278125;     XkKsnWjssRcvKGcGu94278125 = XkKsnWjssRcvKGcGu22606405;     XkKsnWjssRcvKGcGu22606405 = XkKsnWjssRcvKGcGu42161597;     XkKsnWjssRcvKGcGu42161597 = XkKsnWjssRcvKGcGu68443607;     XkKsnWjssRcvKGcGu68443607 = XkKsnWjssRcvKGcGu14759197;     XkKsnWjssRcvKGcGu14759197 = XkKsnWjssRcvKGcGu4561614;     XkKsnWjssRcvKGcGu4561614 = XkKsnWjssRcvKGcGu54779066;     XkKsnWjssRcvKGcGu54779066 = XkKsnWjssRcvKGcGu98272475;     XkKsnWjssRcvKGcGu98272475 = XkKsnWjssRcvKGcGu72376913;     XkKsnWjssRcvKGcGu72376913 = XkKsnWjssRcvKGcGu4766038;     XkKsnWjssRcvKGcGu4766038 = XkKsnWjssRcvKGcGu87566529;     XkKsnWjssRcvKGcGu87566529 = XkKsnWjssRcvKGcGu13908387;     XkKsnWjssRcvKGcGu13908387 = XkKsnWjssRcvKGcGu66325545;     XkKsnWjssRcvKGcGu66325545 = XkKsnWjssRcvKGcGu359213;     XkKsnWjssRcvKGcGu359213 = XkKsnWjssRcvKGcGu90369595;     XkKsnWjssRcvKGcGu90369595 = XkKsnWjssRcvKGcGu36298079;     XkKsnWjssRcvKGcGu36298079 = XkKsnWjssRcvKGcGu45080728;     XkKsnWjssRcvKGcGu45080728 = XkKsnWjssRcvKGcGu7991334;     XkKsnWjssRcvKGcGu7991334 = XkKsnWjssRcvKGcGu32447592;     XkKsnWjssRcvKGcGu32447592 = XkKsnWjssRcvKGcGu46189651;     XkKsnWjssRcvKGcGu46189651 = XkKsnWjssRcvKGcGu41675841;     XkKsnWjssRcvKGcGu41675841 = XkKsnWjssRcvKGcGu77185096;     XkKsnWjssRcvKGcGu77185096 = XkKsnWjssRcvKGcGu10705909;     XkKsnWjssRcvKGcGu10705909 = XkKsnWjssRcvKGcGu31963145;     XkKsnWjssRcvKGcGu31963145 = XkKsnWjssRcvKGcGu70652625;     XkKsnWjssRcvKGcGu70652625 = XkKsnWjssRcvKGcGu89958839;     XkKsnWjssRcvKGcGu89958839 = XkKsnWjssRcvKGcGu61556415;     XkKsnWjssRcvKGcGu61556415 = XkKsnWjssRcvKGcGu69466466;     XkKsnWjssRcvKGcGu69466466 = XkKsnWjssRcvKGcGu19045435;     XkKsnWjssRcvKGcGu19045435 = XkKsnWjssRcvKGcGu19856470;     XkKsnWjssRcvKGcGu19856470 = XkKsnWjssRcvKGcGu37714255;     XkKsnWjssRcvKGcGu37714255 = XkKsnWjssRcvKGcGu87234214;     XkKsnWjssRcvKGcGu87234214 = XkKsnWjssRcvKGcGu64203711;     XkKsnWjssRcvKGcGu64203711 = XkKsnWjssRcvKGcGu26817399;     XkKsnWjssRcvKGcGu26817399 = XkKsnWjssRcvKGcGu31399273;     XkKsnWjssRcvKGcGu31399273 = XkKsnWjssRcvKGcGu90502020;     XkKsnWjssRcvKGcGu90502020 = XkKsnWjssRcvKGcGu37688817;     XkKsnWjssRcvKGcGu37688817 = XkKsnWjssRcvKGcGu69212440;     XkKsnWjssRcvKGcGu69212440 = XkKsnWjssRcvKGcGu80014771;     XkKsnWjssRcvKGcGu80014771 = XkKsnWjssRcvKGcGu81219185;     XkKsnWjssRcvKGcGu81219185 = XkKsnWjssRcvKGcGu8304416;     XkKsnWjssRcvKGcGu8304416 = XkKsnWjssRcvKGcGu68729045;     XkKsnWjssRcvKGcGu68729045 = XkKsnWjssRcvKGcGu43861676;     XkKsnWjssRcvKGcGu43861676 = XkKsnWjssRcvKGcGu33240271;     XkKsnWjssRcvKGcGu33240271 = XkKsnWjssRcvKGcGu77815902;     XkKsnWjssRcvKGcGu77815902 = XkKsnWjssRcvKGcGu94373410;     XkKsnWjssRcvKGcGu94373410 = XkKsnWjssRcvKGcGu42019192;     XkKsnWjssRcvKGcGu42019192 = XkKsnWjssRcvKGcGu78149557;     XkKsnWjssRcvKGcGu78149557 = XkKsnWjssRcvKGcGu25442460;     XkKsnWjssRcvKGcGu25442460 = XkKsnWjssRcvKGcGu36586241;     XkKsnWjssRcvKGcGu36586241 = XkKsnWjssRcvKGcGu55857853;     XkKsnWjssRcvKGcGu55857853 = XkKsnWjssRcvKGcGu22451560;     XkKsnWjssRcvKGcGu22451560 = XkKsnWjssRcvKGcGu27195411;     XkKsnWjssRcvKGcGu27195411 = XkKsnWjssRcvKGcGu83986468;     XkKsnWjssRcvKGcGu83986468 = XkKsnWjssRcvKGcGu50473384;     XkKsnWjssRcvKGcGu50473384 = XkKsnWjssRcvKGcGu17093029;     XkKsnWjssRcvKGcGu17093029 = XkKsnWjssRcvKGcGu11900496;     XkKsnWjssRcvKGcGu11900496 = XkKsnWjssRcvKGcGu10198453;     XkKsnWjssRcvKGcGu10198453 = XkKsnWjssRcvKGcGu97790982;     XkKsnWjssRcvKGcGu97790982 = XkKsnWjssRcvKGcGu24800357;     XkKsnWjssRcvKGcGu24800357 = XkKsnWjssRcvKGcGu43005199;     XkKsnWjssRcvKGcGu43005199 = XkKsnWjssRcvKGcGu82736775;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void AUmQKoDUTwTejktA31197561() {     double DgfqyzKEAxytepmXJ88853882 = -173463286;    double DgfqyzKEAxytepmXJ46138581 = -938102328;    double DgfqyzKEAxytepmXJ34270575 = -573108042;    double DgfqyzKEAxytepmXJ36347915 = -453179016;    double DgfqyzKEAxytepmXJ77572850 = -416994412;    double DgfqyzKEAxytepmXJ84840887 = -717744804;    double DgfqyzKEAxytepmXJ9201183 = -704375363;    double DgfqyzKEAxytepmXJ47105467 = -528314572;    double DgfqyzKEAxytepmXJ43576918 = -769489343;    double DgfqyzKEAxytepmXJ39533236 = -244273593;    double DgfqyzKEAxytepmXJ175475 = -42886421;    double DgfqyzKEAxytepmXJ17101565 = -36664014;    double DgfqyzKEAxytepmXJ78138873 = -876378623;    double DgfqyzKEAxytepmXJ3938728 = -15500978;    double DgfqyzKEAxytepmXJ63899574 = -888047407;    double DgfqyzKEAxytepmXJ88382610 = -606441923;    double DgfqyzKEAxytepmXJ88215834 = -764423092;    double DgfqyzKEAxytepmXJ16019777 = -69902974;    double DgfqyzKEAxytepmXJ87507239 = -566697739;    double DgfqyzKEAxytepmXJ93658589 = -829566799;    double DgfqyzKEAxytepmXJ68403607 = 61987979;    double DgfqyzKEAxytepmXJ24177071 = -423665650;    double DgfqyzKEAxytepmXJ30364759 = -939466365;    double DgfqyzKEAxytepmXJ69773173 = -518320212;    double DgfqyzKEAxytepmXJ6783133 = -105306390;    double DgfqyzKEAxytepmXJ91723195 = -218228480;    double DgfqyzKEAxytepmXJ14698368 = -18272533;    double DgfqyzKEAxytepmXJ51113433 = -226144791;    double DgfqyzKEAxytepmXJ31514687 = 34985974;    double DgfqyzKEAxytepmXJ76897434 = 47145906;    double DgfqyzKEAxytepmXJ587287 = -610926933;    double DgfqyzKEAxytepmXJ76119391 = 76189223;    double DgfqyzKEAxytepmXJ25523004 = -285254816;    double DgfqyzKEAxytepmXJ67714884 = -331187917;    double DgfqyzKEAxytepmXJ62413233 = -285823135;    double DgfqyzKEAxytepmXJ99168806 = -229504067;    double DgfqyzKEAxytepmXJ40904937 = 51117688;    double DgfqyzKEAxytepmXJ64266332 = -43432650;    double DgfqyzKEAxytepmXJ65759479 = -141604034;    double DgfqyzKEAxytepmXJ81780062 = -982083138;    double DgfqyzKEAxytepmXJ28275629 = 62347600;    double DgfqyzKEAxytepmXJ74312054 = -69056492;    double DgfqyzKEAxytepmXJ46510585 = -799480847;    double DgfqyzKEAxytepmXJ2166937 = -601596169;    double DgfqyzKEAxytepmXJ16514079 = -882012936;    double DgfqyzKEAxytepmXJ66718261 = 54008851;    double DgfqyzKEAxytepmXJ16051050 = -283138487;    double DgfqyzKEAxytepmXJ80333853 = -502781970;    double DgfqyzKEAxytepmXJ14441767 = -232944525;    double DgfqyzKEAxytepmXJ478182 = -165545112;    double DgfqyzKEAxytepmXJ49609065 = -463510239;    double DgfqyzKEAxytepmXJ77135265 = -184163011;    double DgfqyzKEAxytepmXJ48607670 = -555160144;    double DgfqyzKEAxytepmXJ86338475 = -475002338;    double DgfqyzKEAxytepmXJ62793953 = 55049973;    double DgfqyzKEAxytepmXJ64676811 = -749797637;    double DgfqyzKEAxytepmXJ15773822 = -998635964;    double DgfqyzKEAxytepmXJ64497401 = 45212170;    double DgfqyzKEAxytepmXJ29564782 = -247872627;    double DgfqyzKEAxytepmXJ85849654 = -98765932;    double DgfqyzKEAxytepmXJ70142520 = -599472272;    double DgfqyzKEAxytepmXJ58087749 = -378230572;    double DgfqyzKEAxytepmXJ15590780 = -463300547;    double DgfqyzKEAxytepmXJ66679483 = -716635250;    double DgfqyzKEAxytepmXJ38945949 = -633346661;    double DgfqyzKEAxytepmXJ24056084 = -19075645;    double DgfqyzKEAxytepmXJ91578560 = -751409199;    double DgfqyzKEAxytepmXJ10423989 = -445190707;    double DgfqyzKEAxytepmXJ41525494 = -729677843;    double DgfqyzKEAxytepmXJ64730768 = -558543341;    double DgfqyzKEAxytepmXJ47477674 = -557559612;    double DgfqyzKEAxytepmXJ23949502 = -620990442;    double DgfqyzKEAxytepmXJ50260298 = -928298941;    double DgfqyzKEAxytepmXJ5727177 = -584614601;    double DgfqyzKEAxytepmXJ65382961 = -791914400;    double DgfqyzKEAxytepmXJ94091553 = -868955530;    double DgfqyzKEAxytepmXJ77666485 = -624184803;    double DgfqyzKEAxytepmXJ28197823 = -237870196;    double DgfqyzKEAxytepmXJ53259094 = -636307277;    double DgfqyzKEAxytepmXJ40064871 = -59315242;    double DgfqyzKEAxytepmXJ75672146 = -935089994;    double DgfqyzKEAxytepmXJ34364514 = -515490563;    double DgfqyzKEAxytepmXJ36671667 = -993200267;    double DgfqyzKEAxytepmXJ31036505 = -799468914;    double DgfqyzKEAxytepmXJ27288369 = -489343855;    double DgfqyzKEAxytepmXJ23452022 = -326763922;    double DgfqyzKEAxytepmXJ27511722 = -368650633;    double DgfqyzKEAxytepmXJ39184528 = -810252478;    double DgfqyzKEAxytepmXJ4920931 = -286237891;    double DgfqyzKEAxytepmXJ97736422 = -536025498;    double DgfqyzKEAxytepmXJ83394985 = -230868103;    double DgfqyzKEAxytepmXJ76407535 = -994094482;    double DgfqyzKEAxytepmXJ34701550 = -795560024;    double DgfqyzKEAxytepmXJ79909824 = 57161897;    double DgfqyzKEAxytepmXJ11637543 = -282610867;    double DgfqyzKEAxytepmXJ70187879 = -559421828;    double DgfqyzKEAxytepmXJ58721274 = -605755945;    double DgfqyzKEAxytepmXJ79831101 = 17154402;    double DgfqyzKEAxytepmXJ63220987 = -968249509;    double DgfqyzKEAxytepmXJ92457995 = -173463286;     DgfqyzKEAxytepmXJ88853882 = DgfqyzKEAxytepmXJ46138581;     DgfqyzKEAxytepmXJ46138581 = DgfqyzKEAxytepmXJ34270575;     DgfqyzKEAxytepmXJ34270575 = DgfqyzKEAxytepmXJ36347915;     DgfqyzKEAxytepmXJ36347915 = DgfqyzKEAxytepmXJ77572850;     DgfqyzKEAxytepmXJ77572850 = DgfqyzKEAxytepmXJ84840887;     DgfqyzKEAxytepmXJ84840887 = DgfqyzKEAxytepmXJ9201183;     DgfqyzKEAxytepmXJ9201183 = DgfqyzKEAxytepmXJ47105467;     DgfqyzKEAxytepmXJ47105467 = DgfqyzKEAxytepmXJ43576918;     DgfqyzKEAxytepmXJ43576918 = DgfqyzKEAxytepmXJ39533236;     DgfqyzKEAxytepmXJ39533236 = DgfqyzKEAxytepmXJ175475;     DgfqyzKEAxytepmXJ175475 = DgfqyzKEAxytepmXJ17101565;     DgfqyzKEAxytepmXJ17101565 = DgfqyzKEAxytepmXJ78138873;     DgfqyzKEAxytepmXJ78138873 = DgfqyzKEAxytepmXJ3938728;     DgfqyzKEAxytepmXJ3938728 = DgfqyzKEAxytepmXJ63899574;     DgfqyzKEAxytepmXJ63899574 = DgfqyzKEAxytepmXJ88382610;     DgfqyzKEAxytepmXJ88382610 = DgfqyzKEAxytepmXJ88215834;     DgfqyzKEAxytepmXJ88215834 = DgfqyzKEAxytepmXJ16019777;     DgfqyzKEAxytepmXJ16019777 = DgfqyzKEAxytepmXJ87507239;     DgfqyzKEAxytepmXJ87507239 = DgfqyzKEAxytepmXJ93658589;     DgfqyzKEAxytepmXJ93658589 = DgfqyzKEAxytepmXJ68403607;     DgfqyzKEAxytepmXJ68403607 = DgfqyzKEAxytepmXJ24177071;     DgfqyzKEAxytepmXJ24177071 = DgfqyzKEAxytepmXJ30364759;     DgfqyzKEAxytepmXJ30364759 = DgfqyzKEAxytepmXJ69773173;     DgfqyzKEAxytepmXJ69773173 = DgfqyzKEAxytepmXJ6783133;     DgfqyzKEAxytepmXJ6783133 = DgfqyzKEAxytepmXJ91723195;     DgfqyzKEAxytepmXJ91723195 = DgfqyzKEAxytepmXJ14698368;     DgfqyzKEAxytepmXJ14698368 = DgfqyzKEAxytepmXJ51113433;     DgfqyzKEAxytepmXJ51113433 = DgfqyzKEAxytepmXJ31514687;     DgfqyzKEAxytepmXJ31514687 = DgfqyzKEAxytepmXJ76897434;     DgfqyzKEAxytepmXJ76897434 = DgfqyzKEAxytepmXJ587287;     DgfqyzKEAxytepmXJ587287 = DgfqyzKEAxytepmXJ76119391;     DgfqyzKEAxytepmXJ76119391 = DgfqyzKEAxytepmXJ25523004;     DgfqyzKEAxytepmXJ25523004 = DgfqyzKEAxytepmXJ67714884;     DgfqyzKEAxytepmXJ67714884 = DgfqyzKEAxytepmXJ62413233;     DgfqyzKEAxytepmXJ62413233 = DgfqyzKEAxytepmXJ99168806;     DgfqyzKEAxytepmXJ99168806 = DgfqyzKEAxytepmXJ40904937;     DgfqyzKEAxytepmXJ40904937 = DgfqyzKEAxytepmXJ64266332;     DgfqyzKEAxytepmXJ64266332 = DgfqyzKEAxytepmXJ65759479;     DgfqyzKEAxytepmXJ65759479 = DgfqyzKEAxytepmXJ81780062;     DgfqyzKEAxytepmXJ81780062 = DgfqyzKEAxytepmXJ28275629;     DgfqyzKEAxytepmXJ28275629 = DgfqyzKEAxytepmXJ74312054;     DgfqyzKEAxytepmXJ74312054 = DgfqyzKEAxytepmXJ46510585;     DgfqyzKEAxytepmXJ46510585 = DgfqyzKEAxytepmXJ2166937;     DgfqyzKEAxytepmXJ2166937 = DgfqyzKEAxytepmXJ16514079;     DgfqyzKEAxytepmXJ16514079 = DgfqyzKEAxytepmXJ66718261;     DgfqyzKEAxytepmXJ66718261 = DgfqyzKEAxytepmXJ16051050;     DgfqyzKEAxytepmXJ16051050 = DgfqyzKEAxytepmXJ80333853;     DgfqyzKEAxytepmXJ80333853 = DgfqyzKEAxytepmXJ14441767;     DgfqyzKEAxytepmXJ14441767 = DgfqyzKEAxytepmXJ478182;     DgfqyzKEAxytepmXJ478182 = DgfqyzKEAxytepmXJ49609065;     DgfqyzKEAxytepmXJ49609065 = DgfqyzKEAxytepmXJ77135265;     DgfqyzKEAxytepmXJ77135265 = DgfqyzKEAxytepmXJ48607670;     DgfqyzKEAxytepmXJ48607670 = DgfqyzKEAxytepmXJ86338475;     DgfqyzKEAxytepmXJ86338475 = DgfqyzKEAxytepmXJ62793953;     DgfqyzKEAxytepmXJ62793953 = DgfqyzKEAxytepmXJ64676811;     DgfqyzKEAxytepmXJ64676811 = DgfqyzKEAxytepmXJ15773822;     DgfqyzKEAxytepmXJ15773822 = DgfqyzKEAxytepmXJ64497401;     DgfqyzKEAxytepmXJ64497401 = DgfqyzKEAxytepmXJ29564782;     DgfqyzKEAxytepmXJ29564782 = DgfqyzKEAxytepmXJ85849654;     DgfqyzKEAxytepmXJ85849654 = DgfqyzKEAxytepmXJ70142520;     DgfqyzKEAxytepmXJ70142520 = DgfqyzKEAxytepmXJ58087749;     DgfqyzKEAxytepmXJ58087749 = DgfqyzKEAxytepmXJ15590780;     DgfqyzKEAxytepmXJ15590780 = DgfqyzKEAxytepmXJ66679483;     DgfqyzKEAxytepmXJ66679483 = DgfqyzKEAxytepmXJ38945949;     DgfqyzKEAxytepmXJ38945949 = DgfqyzKEAxytepmXJ24056084;     DgfqyzKEAxytepmXJ24056084 = DgfqyzKEAxytepmXJ91578560;     DgfqyzKEAxytepmXJ91578560 = DgfqyzKEAxytepmXJ10423989;     DgfqyzKEAxytepmXJ10423989 = DgfqyzKEAxytepmXJ41525494;     DgfqyzKEAxytepmXJ41525494 = DgfqyzKEAxytepmXJ64730768;     DgfqyzKEAxytepmXJ64730768 = DgfqyzKEAxytepmXJ47477674;     DgfqyzKEAxytepmXJ47477674 = DgfqyzKEAxytepmXJ23949502;     DgfqyzKEAxytepmXJ23949502 = DgfqyzKEAxytepmXJ50260298;     DgfqyzKEAxytepmXJ50260298 = DgfqyzKEAxytepmXJ5727177;     DgfqyzKEAxytepmXJ5727177 = DgfqyzKEAxytepmXJ65382961;     DgfqyzKEAxytepmXJ65382961 = DgfqyzKEAxytepmXJ94091553;     DgfqyzKEAxytepmXJ94091553 = DgfqyzKEAxytepmXJ77666485;     DgfqyzKEAxytepmXJ77666485 = DgfqyzKEAxytepmXJ28197823;     DgfqyzKEAxytepmXJ28197823 = DgfqyzKEAxytepmXJ53259094;     DgfqyzKEAxytepmXJ53259094 = DgfqyzKEAxytepmXJ40064871;     DgfqyzKEAxytepmXJ40064871 = DgfqyzKEAxytepmXJ75672146;     DgfqyzKEAxytepmXJ75672146 = DgfqyzKEAxytepmXJ34364514;     DgfqyzKEAxytepmXJ34364514 = DgfqyzKEAxytepmXJ36671667;     DgfqyzKEAxytepmXJ36671667 = DgfqyzKEAxytepmXJ31036505;     DgfqyzKEAxytepmXJ31036505 = DgfqyzKEAxytepmXJ27288369;     DgfqyzKEAxytepmXJ27288369 = DgfqyzKEAxytepmXJ23452022;     DgfqyzKEAxytepmXJ23452022 = DgfqyzKEAxytepmXJ27511722;     DgfqyzKEAxytepmXJ27511722 = DgfqyzKEAxytepmXJ39184528;     DgfqyzKEAxytepmXJ39184528 = DgfqyzKEAxytepmXJ4920931;     DgfqyzKEAxytepmXJ4920931 = DgfqyzKEAxytepmXJ97736422;     DgfqyzKEAxytepmXJ97736422 = DgfqyzKEAxytepmXJ83394985;     DgfqyzKEAxytepmXJ83394985 = DgfqyzKEAxytepmXJ76407535;     DgfqyzKEAxytepmXJ76407535 = DgfqyzKEAxytepmXJ34701550;     DgfqyzKEAxytepmXJ34701550 = DgfqyzKEAxytepmXJ79909824;     DgfqyzKEAxytepmXJ79909824 = DgfqyzKEAxytepmXJ11637543;     DgfqyzKEAxytepmXJ11637543 = DgfqyzKEAxytepmXJ70187879;     DgfqyzKEAxytepmXJ70187879 = DgfqyzKEAxytepmXJ58721274;     DgfqyzKEAxytepmXJ58721274 = DgfqyzKEAxytepmXJ79831101;     DgfqyzKEAxytepmXJ79831101 = DgfqyzKEAxytepmXJ63220987;     DgfqyzKEAxytepmXJ63220987 = DgfqyzKEAxytepmXJ92457995;     DgfqyzKEAxytepmXJ92457995 = DgfqyzKEAxytepmXJ88853882;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void YghkOkzDTnnbotsU98489160() {     double QslFbBTyoQPdPxwJd24312851 = -639740186;    double QslFbBTyoQPdPxwJd59410659 = -818653935;    double QslFbBTyoQPdPxwJd71657051 = -238775117;    double QslFbBTyoQPdPxwJd8571270 = -274330088;    double QslFbBTyoQPdPxwJd33858119 = -522593257;    double QslFbBTyoQPdPxwJd21063760 = -980092699;    double QslFbBTyoQPdPxwJd19858589 = -549101097;    double QslFbBTyoQPdPxwJd9237028 = -172548713;    double QslFbBTyoQPdPxwJd45094523 = -157820750;    double QslFbBTyoQPdPxwJd54435948 = 91039409;    double QslFbBTyoQPdPxwJd38357210 = -944894150;    double QslFbBTyoQPdPxwJd17669713 = 99215214;    double QslFbBTyoQPdPxwJd16373519 = -328391128;    double QslFbBTyoQPdPxwJd57079590 = 8135613;    double QslFbBTyoQPdPxwJd22094091 = -291472422;    double QslFbBTyoQPdPxwJd71923911 = -242253530;    double QslFbBTyoQPdPxwJd44051770 = -191397645;    double QslFbBTyoQPdPxwJd15779969 = -143331043;    double QslFbBTyoQPdPxwJd88057569 = -883261161;    double QslFbBTyoQPdPxwJd87495808 = -156127865;    double QslFbBTyoQPdPxwJd81490470 = -695286710;    double QslFbBTyoQPdPxwJd27598596 = -510301944;    double QslFbBTyoQPdPxwJd890139 = -577098549;    double QslFbBTyoQPdPxwJd63566118 = 70855184;    double QslFbBTyoQPdPxwJd14801084 = -129291588;    double QslFbBTyoQPdPxwJd2421368 = 93194713;    double QslFbBTyoQPdPxwJd75118941 = -924556674;    double QslFbBTyoQPdPxwJd91841858 = -395916965;    double QslFbBTyoQPdPxwJd78182901 = -539108828;    double QslFbBTyoQPdPxwJd82430845 = 12638736;    double QslFbBTyoQPdPxwJd19170519 = -62907420;    double QslFbBTyoQPdPxwJd13942410 = -796782191;    double QslFbBTyoQPdPxwJd37741015 = -432850793;    double QslFbBTyoQPdPxwJd97463310 = -395881730;    double QslFbBTyoQPdPxwJd28221687 = -236923386;    double QslFbBTyoQPdPxwJd50729650 = -444069373;    double QslFbBTyoQPdPxwJd90316753 = -913578352;    double QslFbBTyoQPdPxwJd15481736 = -26088231;    double QslFbBTyoQPdPxwJd96150040 = -892480311;    double QslFbBTyoQPdPxwJd46051595 = -447345684;    double QslFbBTyoQPdPxwJd61727236 = -806851314;    double QslFbBTyoQPdPxwJd63677181 = -763601783;    double QslFbBTyoQPdPxwJd35893964 = -352661633;    double QslFbBTyoQPdPxwJd42298275 = -105928579;    double QslFbBTyoQPdPxwJd16530538 = -708524097;    double QslFbBTyoQPdPxwJd12679590 = 15085219;    double QslFbBTyoQPdPxwJd35396214 = -767124848;    double QslFbBTyoQPdPxwJd98060165 = -669308564;    double QslFbBTyoQPdPxwJd37334433 = -996681405;    double QslFbBTyoQPdPxwJd25007771 = -656571928;    double QslFbBTyoQPdPxwJd30379789 = -337786797;    double QslFbBTyoQPdPxwJd60620737 = -397815358;    double QslFbBTyoQPdPxwJd17228819 = -311205698;    double QslFbBTyoQPdPxwJd2330323 = -666510838;    double QslFbBTyoQPdPxwJd16998690 = -291440173;    double QslFbBTyoQPdPxwJd96714254 = -29438243;    double QslFbBTyoQPdPxwJd58520520 = -141555386;    double QslFbBTyoQPdPxwJd8090933 = -209630301;    double QslFbBTyoQPdPxwJd93770185 = -45038500;    double QslFbBTyoQPdPxwJd31436752 = -515787970;    double QslFbBTyoQPdPxwJd45944818 = 44463974;    double QslFbBTyoQPdPxwJd28016731 = -53184132;    double QslFbBTyoQPdPxwJd31054127 = -633439885;    double QslFbBTyoQPdPxwJd62663678 = -70459486;    double QslFbBTyoQPdPxwJd35265430 = -846053172;    double QslFbBTyoQPdPxwJd24414800 = -48111960;    double QslFbBTyoQPdPxwJd79928697 = -467933993;    double QslFbBTyoQPdPxwJd18910208 = -932509398;    double QslFbBTyoQPdPxwJd28857904 = -754941002;    double QslFbBTyoQPdPxwJd71364440 = -847403050;    double QslFbBTyoQPdPxwJd81607158 = -328675179;    double QslFbBTyoQPdPxwJd28570034 = -65309414;    double QslFbBTyoQPdPxwJd19629928 = -250850733;    double QslFbBTyoQPdPxwJd42005974 = -335915477;    double QslFbBTyoQPdPxwJd25768573 = -349276551;    double QslFbBTyoQPdPxwJd17813289 = -931684928;    double QslFbBTyoQPdPxwJd91704632 = -57640311;    double QslFbBTyoQPdPxwJd58591864 = -371169970;    double QslFbBTyoQPdPxwJd47035581 = -220620720;    double QslFbBTyoQPdPxwJd2121494 = -44376808;    double QslFbBTyoQPdPxwJd67025154 = -139680440;    double QslFbBTyoQPdPxwJd77058775 = -155248110;    double QslFbBTyoQPdPxwJd54507426 = -399235561;    double QslFbBTyoQPdPxwJd53175130 = -882536901;    double QslFbBTyoQPdPxwJd52051056 = -649574468;    double QslFbBTyoQPdPxwJd58549781 = -665092063;    double QslFbBTyoQPdPxwJd96713591 = -385576494;    double QslFbBTyoQPdPxwJd35410693 = -766339956;    double QslFbBTyoQPdPxwJd80464621 = -4441558;    double QslFbBTyoQPdPxwJd31507432 = -107485143;    double QslFbBTyoQPdPxwJd92209130 = -202513987;    double QslFbBTyoQPdPxwJd82225820 = -603948051;    double QslFbBTyoQPdPxwJd21711550 = -981049732;    double QslFbBTyoQPdPxwJd64713288 = -276692341;    double QslFbBTyoQPdPxwJd106777 = -391809659;    double QslFbBTyoQPdPxwJd33710506 = -653667183;    double QslFbBTyoQPdPxwJd32623055 = -30161898;    double QslFbBTyoQPdPxwJd73230285 = -182202148;    double QslFbBTyoQPdPxwJd7032846 = -259875408;    double QslFbBTyoQPdPxwJd92115738 = -639740186;     QslFbBTyoQPdPxwJd24312851 = QslFbBTyoQPdPxwJd59410659;     QslFbBTyoQPdPxwJd59410659 = QslFbBTyoQPdPxwJd71657051;     QslFbBTyoQPdPxwJd71657051 = QslFbBTyoQPdPxwJd8571270;     QslFbBTyoQPdPxwJd8571270 = QslFbBTyoQPdPxwJd33858119;     QslFbBTyoQPdPxwJd33858119 = QslFbBTyoQPdPxwJd21063760;     QslFbBTyoQPdPxwJd21063760 = QslFbBTyoQPdPxwJd19858589;     QslFbBTyoQPdPxwJd19858589 = QslFbBTyoQPdPxwJd9237028;     QslFbBTyoQPdPxwJd9237028 = QslFbBTyoQPdPxwJd45094523;     QslFbBTyoQPdPxwJd45094523 = QslFbBTyoQPdPxwJd54435948;     QslFbBTyoQPdPxwJd54435948 = QslFbBTyoQPdPxwJd38357210;     QslFbBTyoQPdPxwJd38357210 = QslFbBTyoQPdPxwJd17669713;     QslFbBTyoQPdPxwJd17669713 = QslFbBTyoQPdPxwJd16373519;     QslFbBTyoQPdPxwJd16373519 = QslFbBTyoQPdPxwJd57079590;     QslFbBTyoQPdPxwJd57079590 = QslFbBTyoQPdPxwJd22094091;     QslFbBTyoQPdPxwJd22094091 = QslFbBTyoQPdPxwJd71923911;     QslFbBTyoQPdPxwJd71923911 = QslFbBTyoQPdPxwJd44051770;     QslFbBTyoQPdPxwJd44051770 = QslFbBTyoQPdPxwJd15779969;     QslFbBTyoQPdPxwJd15779969 = QslFbBTyoQPdPxwJd88057569;     QslFbBTyoQPdPxwJd88057569 = QslFbBTyoQPdPxwJd87495808;     QslFbBTyoQPdPxwJd87495808 = QslFbBTyoQPdPxwJd81490470;     QslFbBTyoQPdPxwJd81490470 = QslFbBTyoQPdPxwJd27598596;     QslFbBTyoQPdPxwJd27598596 = QslFbBTyoQPdPxwJd890139;     QslFbBTyoQPdPxwJd890139 = QslFbBTyoQPdPxwJd63566118;     QslFbBTyoQPdPxwJd63566118 = QslFbBTyoQPdPxwJd14801084;     QslFbBTyoQPdPxwJd14801084 = QslFbBTyoQPdPxwJd2421368;     QslFbBTyoQPdPxwJd2421368 = QslFbBTyoQPdPxwJd75118941;     QslFbBTyoQPdPxwJd75118941 = QslFbBTyoQPdPxwJd91841858;     QslFbBTyoQPdPxwJd91841858 = QslFbBTyoQPdPxwJd78182901;     QslFbBTyoQPdPxwJd78182901 = QslFbBTyoQPdPxwJd82430845;     QslFbBTyoQPdPxwJd82430845 = QslFbBTyoQPdPxwJd19170519;     QslFbBTyoQPdPxwJd19170519 = QslFbBTyoQPdPxwJd13942410;     QslFbBTyoQPdPxwJd13942410 = QslFbBTyoQPdPxwJd37741015;     QslFbBTyoQPdPxwJd37741015 = QslFbBTyoQPdPxwJd97463310;     QslFbBTyoQPdPxwJd97463310 = QslFbBTyoQPdPxwJd28221687;     QslFbBTyoQPdPxwJd28221687 = QslFbBTyoQPdPxwJd50729650;     QslFbBTyoQPdPxwJd50729650 = QslFbBTyoQPdPxwJd90316753;     QslFbBTyoQPdPxwJd90316753 = QslFbBTyoQPdPxwJd15481736;     QslFbBTyoQPdPxwJd15481736 = QslFbBTyoQPdPxwJd96150040;     QslFbBTyoQPdPxwJd96150040 = QslFbBTyoQPdPxwJd46051595;     QslFbBTyoQPdPxwJd46051595 = QslFbBTyoQPdPxwJd61727236;     QslFbBTyoQPdPxwJd61727236 = QslFbBTyoQPdPxwJd63677181;     QslFbBTyoQPdPxwJd63677181 = QslFbBTyoQPdPxwJd35893964;     QslFbBTyoQPdPxwJd35893964 = QslFbBTyoQPdPxwJd42298275;     QslFbBTyoQPdPxwJd42298275 = QslFbBTyoQPdPxwJd16530538;     QslFbBTyoQPdPxwJd16530538 = QslFbBTyoQPdPxwJd12679590;     QslFbBTyoQPdPxwJd12679590 = QslFbBTyoQPdPxwJd35396214;     QslFbBTyoQPdPxwJd35396214 = QslFbBTyoQPdPxwJd98060165;     QslFbBTyoQPdPxwJd98060165 = QslFbBTyoQPdPxwJd37334433;     QslFbBTyoQPdPxwJd37334433 = QslFbBTyoQPdPxwJd25007771;     QslFbBTyoQPdPxwJd25007771 = QslFbBTyoQPdPxwJd30379789;     QslFbBTyoQPdPxwJd30379789 = QslFbBTyoQPdPxwJd60620737;     QslFbBTyoQPdPxwJd60620737 = QslFbBTyoQPdPxwJd17228819;     QslFbBTyoQPdPxwJd17228819 = QslFbBTyoQPdPxwJd2330323;     QslFbBTyoQPdPxwJd2330323 = QslFbBTyoQPdPxwJd16998690;     QslFbBTyoQPdPxwJd16998690 = QslFbBTyoQPdPxwJd96714254;     QslFbBTyoQPdPxwJd96714254 = QslFbBTyoQPdPxwJd58520520;     QslFbBTyoQPdPxwJd58520520 = QslFbBTyoQPdPxwJd8090933;     QslFbBTyoQPdPxwJd8090933 = QslFbBTyoQPdPxwJd93770185;     QslFbBTyoQPdPxwJd93770185 = QslFbBTyoQPdPxwJd31436752;     QslFbBTyoQPdPxwJd31436752 = QslFbBTyoQPdPxwJd45944818;     QslFbBTyoQPdPxwJd45944818 = QslFbBTyoQPdPxwJd28016731;     QslFbBTyoQPdPxwJd28016731 = QslFbBTyoQPdPxwJd31054127;     QslFbBTyoQPdPxwJd31054127 = QslFbBTyoQPdPxwJd62663678;     QslFbBTyoQPdPxwJd62663678 = QslFbBTyoQPdPxwJd35265430;     QslFbBTyoQPdPxwJd35265430 = QslFbBTyoQPdPxwJd24414800;     QslFbBTyoQPdPxwJd24414800 = QslFbBTyoQPdPxwJd79928697;     QslFbBTyoQPdPxwJd79928697 = QslFbBTyoQPdPxwJd18910208;     QslFbBTyoQPdPxwJd18910208 = QslFbBTyoQPdPxwJd28857904;     QslFbBTyoQPdPxwJd28857904 = QslFbBTyoQPdPxwJd71364440;     QslFbBTyoQPdPxwJd71364440 = QslFbBTyoQPdPxwJd81607158;     QslFbBTyoQPdPxwJd81607158 = QslFbBTyoQPdPxwJd28570034;     QslFbBTyoQPdPxwJd28570034 = QslFbBTyoQPdPxwJd19629928;     QslFbBTyoQPdPxwJd19629928 = QslFbBTyoQPdPxwJd42005974;     QslFbBTyoQPdPxwJd42005974 = QslFbBTyoQPdPxwJd25768573;     QslFbBTyoQPdPxwJd25768573 = QslFbBTyoQPdPxwJd17813289;     QslFbBTyoQPdPxwJd17813289 = QslFbBTyoQPdPxwJd91704632;     QslFbBTyoQPdPxwJd91704632 = QslFbBTyoQPdPxwJd58591864;     QslFbBTyoQPdPxwJd58591864 = QslFbBTyoQPdPxwJd47035581;     QslFbBTyoQPdPxwJd47035581 = QslFbBTyoQPdPxwJd2121494;     QslFbBTyoQPdPxwJd2121494 = QslFbBTyoQPdPxwJd67025154;     QslFbBTyoQPdPxwJd67025154 = QslFbBTyoQPdPxwJd77058775;     QslFbBTyoQPdPxwJd77058775 = QslFbBTyoQPdPxwJd54507426;     QslFbBTyoQPdPxwJd54507426 = QslFbBTyoQPdPxwJd53175130;     QslFbBTyoQPdPxwJd53175130 = QslFbBTyoQPdPxwJd52051056;     QslFbBTyoQPdPxwJd52051056 = QslFbBTyoQPdPxwJd58549781;     QslFbBTyoQPdPxwJd58549781 = QslFbBTyoQPdPxwJd96713591;     QslFbBTyoQPdPxwJd96713591 = QslFbBTyoQPdPxwJd35410693;     QslFbBTyoQPdPxwJd35410693 = QslFbBTyoQPdPxwJd80464621;     QslFbBTyoQPdPxwJd80464621 = QslFbBTyoQPdPxwJd31507432;     QslFbBTyoQPdPxwJd31507432 = QslFbBTyoQPdPxwJd92209130;     QslFbBTyoQPdPxwJd92209130 = QslFbBTyoQPdPxwJd82225820;     QslFbBTyoQPdPxwJd82225820 = QslFbBTyoQPdPxwJd21711550;     QslFbBTyoQPdPxwJd21711550 = QslFbBTyoQPdPxwJd64713288;     QslFbBTyoQPdPxwJd64713288 = QslFbBTyoQPdPxwJd106777;     QslFbBTyoQPdPxwJd106777 = QslFbBTyoQPdPxwJd33710506;     QslFbBTyoQPdPxwJd33710506 = QslFbBTyoQPdPxwJd32623055;     QslFbBTyoQPdPxwJd32623055 = QslFbBTyoQPdPxwJd73230285;     QslFbBTyoQPdPxwJd73230285 = QslFbBTyoQPdPxwJd7032846;     QslFbBTyoQPdPxwJd7032846 = QslFbBTyoQPdPxwJd92115738;     QslFbBTyoQPdPxwJd92115738 = QslFbBTyoQPdPxwJd24312851;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void AkhrEYDJgRqKdFMw13538228() {     double rwNCbcqFgIyYKWNSj30429957 = -751642075;    double rwNCbcqFgIyYKWNSj2783939 = -528253716;    double rwNCbcqFgIyYKWNSj87699235 = -389608038;    double rwNCbcqFgIyYKWNSj35646050 = -465078718;    double rwNCbcqFgIyYKWNSj2753608 = 56756872;    double rwNCbcqFgIyYKWNSj12480962 = -144588934;    double rwNCbcqFgIyYKWNSj80347555 = -688066074;    double rwNCbcqFgIyYKWNSj58996919 = -459422626;    double rwNCbcqFgIyYKWNSj9737019 = 74146584;    double rwNCbcqFgIyYKWNSj95665607 = -237936069;    double rwNCbcqFgIyYKWNSj98467499 = -822331737;    double rwNCbcqFgIyYKWNSj49492755 = -345190988;    double rwNCbcqFgIyYKWNSj2582637 = -768282217;    double rwNCbcqFgIyYKWNSj40223267 = -524102955;    double rwNCbcqFgIyYKWNSj17836518 = -981934589;    double rwNCbcqFgIyYKWNSj13429306 = -215253074;    double rwNCbcqFgIyYKWNSj37887773 = -807942758;    double rwNCbcqFgIyYKWNSj12833122 = -908853685;    double rwNCbcqFgIyYKWNSj49887411 = -189030257;    double rwNCbcqFgIyYKWNSj68045973 = -513030784;    double rwNCbcqFgIyYKWNSj70043663 = -550774165;    double rwNCbcqFgIyYKWNSj14119620 = -418231660;    double rwNCbcqFgIyYKWNSj36480931 = -323616036;    double rwNCbcqFgIyYKWNSj47558492 = -151449804;    double rwNCbcqFgIyYKWNSj58500734 = -502487621;    double rwNCbcqFgIyYKWNSj27143043 = -885460610;    double rwNCbcqFgIyYKWNSj73578719 = -825218877;    double rwNCbcqFgIyYKWNSj4948984 = -461845919;    double rwNCbcqFgIyYKWNSj44315157 = -761491533;    double rwNCbcqFgIyYKWNSj51046482 = -904832132;    double rwNCbcqFgIyYKWNSj11413068 = 23771642;    double rwNCbcqFgIyYKWNSj11553032 = -506229007;    double rwNCbcqFgIyYKWNSj47451963 = -325464164;    double rwNCbcqFgIyYKWNSj92293875 = -147423495;    double rwNCbcqFgIyYKWNSj89696339 = -875656491;    double rwNCbcqFgIyYKWNSj19455564 = -658095480;    double rwNCbcqFgIyYKWNSj71578688 = -867214221;    double rwNCbcqFgIyYKWNSj49571948 = -897594241;    double rwNCbcqFgIyYKWNSj69760294 = -886403807;    double rwNCbcqFgIyYKWNSj33553533 = -920784907;    double rwNCbcqFgIyYKWNSj67396460 = -872125208;    double rwNCbcqFgIyYKWNSj95827638 = 66255939;    double rwNCbcqFgIyYKWNSj13960942 = -433466217;    double rwNCbcqFgIyYKWNSj29706015 = -125271518;    double rwNCbcqFgIyYKWNSj28483003 = -370422251;    double rwNCbcqFgIyYKWNSj24618785 = 29869102;    double rwNCbcqFgIyYKWNSj53174788 = -885900385;    double rwNCbcqFgIyYKWNSj6017107 = -679204096;    double rwNCbcqFgIyYKWNSj47010162 = 17581607;    double rwNCbcqFgIyYKWNSj37919424 = -743582736;    double rwNCbcqFgIyYKWNSj66080467 = -180080468;    double rwNCbcqFgIyYKWNSj71430457 = -198541678;    double rwNCbcqFgIyYKWNSj65477276 = -102995286;    double rwNCbcqFgIyYKWNSj98299202 = -986980983;    double rwNCbcqFgIyYKWNSj43494565 = -991971955;    double rwNCbcqFgIyYKWNSj16310338 = -233410415;    double rwNCbcqFgIyYKWNSj66303007 = -104637680;    double rwNCbcqFgIyYKWNSj40140743 = -138158234;    double rwNCbcqFgIyYKWNSj77145316 = -962591097;    double rwNCbcqFgIyYKWNSj75610565 = -57782518;    double rwNCbcqFgIyYKWNSj38902243 = -319370058;    double rwNCbcqFgIyYKWNSj75398571 = -126220156;    double rwNCbcqFgIyYKWNSj14681762 = -697931094;    double rwNCbcqFgIyYKWNSj58690537 = -21021284;    double rwNCbcqFgIyYKWNSj84252539 = -161707711;    double rwNCbcqFgIyYKWNSj86914468 = -216102731;    double rwNCbcqFgIyYKWNSj2040792 = 80273176;    double rwNCbcqFgIyYKWNSj10288762 = -520858722;    double rwNCbcqFgIyYKWNSj50526928 = -648446465;    double rwNCbcqFgIyYKWNSj98380954 = -223839109;    double rwNCbcqFgIyYKWNSj41850617 = -348038853;    double rwNCbcqFgIyYKWNSj88315824 = -910348517;    double rwNCbcqFgIyYKWNSj43072827 = 77550121;    double rwNCbcqFgIyYKWNSj16333878 = -268245351;    double rwNCbcqFgIyYKWNSj649514 = -640905577;    double rwNCbcqFgIyYKWNSj74216025 = -517030105;    double rwNCbcqFgIyYKWNSj158678 = -984765443;    double rwNCbcqFgIyYKWNSj6774916 = -98344519;    double rwNCbcqFgIyYKWNSj19075490 = -781027553;    double rwNCbcqFgIyYKWNSj33881949 = -432356724;    double rwNCbcqFgIyYKWNSj73968255 = -999560226;    double rwNCbcqFgIyYKWNSj67561613 = -46014781;    double rwNCbcqFgIyYKWNSj57938822 = -379427526;    double rwNCbcqFgIyYKWNSj6395734 = 82091203;    double rwNCbcqFgIyYKWNSj84966014 = -624751664;    double rwNCbcqFgIyYKWNSj39982611 = -777686681;    double rwNCbcqFgIyYKWNSj46075756 = -303233722;    double rwNCbcqFgIyYKWNSj49152761 = -338483181;    double rwNCbcqFgIyYKWNSj48799310 = -155451541;    double rwNCbcqFgIyYKWNSj73386001 = -542246076;    double rwNCbcqFgIyYKWNSj53152556 = -453457801;    double rwNCbcqFgIyYKWNSj31437946 = -629055988;    double rwNCbcqFgIyYKWNSj72426632 = -935003145;    double rwNCbcqFgIyYKWNSj94149729 = -728621289;    double rwNCbcqFgIyYKWNSj94651290 = -501414849;    double rwNCbcqFgIyYKWNSj91997888 = -645905053;    double rwNCbcqFgIyYKWNSj81145876 = -235812968;    double rwNCbcqFgIyYKWNSj55270405 = -312444933;    double rwNCbcqFgIyYKWNSj45453476 = -963563807;    double rwNCbcqFgIyYKWNSj41568535 = -751642075;     rwNCbcqFgIyYKWNSj30429957 = rwNCbcqFgIyYKWNSj2783939;     rwNCbcqFgIyYKWNSj2783939 = rwNCbcqFgIyYKWNSj87699235;     rwNCbcqFgIyYKWNSj87699235 = rwNCbcqFgIyYKWNSj35646050;     rwNCbcqFgIyYKWNSj35646050 = rwNCbcqFgIyYKWNSj2753608;     rwNCbcqFgIyYKWNSj2753608 = rwNCbcqFgIyYKWNSj12480962;     rwNCbcqFgIyYKWNSj12480962 = rwNCbcqFgIyYKWNSj80347555;     rwNCbcqFgIyYKWNSj80347555 = rwNCbcqFgIyYKWNSj58996919;     rwNCbcqFgIyYKWNSj58996919 = rwNCbcqFgIyYKWNSj9737019;     rwNCbcqFgIyYKWNSj9737019 = rwNCbcqFgIyYKWNSj95665607;     rwNCbcqFgIyYKWNSj95665607 = rwNCbcqFgIyYKWNSj98467499;     rwNCbcqFgIyYKWNSj98467499 = rwNCbcqFgIyYKWNSj49492755;     rwNCbcqFgIyYKWNSj49492755 = rwNCbcqFgIyYKWNSj2582637;     rwNCbcqFgIyYKWNSj2582637 = rwNCbcqFgIyYKWNSj40223267;     rwNCbcqFgIyYKWNSj40223267 = rwNCbcqFgIyYKWNSj17836518;     rwNCbcqFgIyYKWNSj17836518 = rwNCbcqFgIyYKWNSj13429306;     rwNCbcqFgIyYKWNSj13429306 = rwNCbcqFgIyYKWNSj37887773;     rwNCbcqFgIyYKWNSj37887773 = rwNCbcqFgIyYKWNSj12833122;     rwNCbcqFgIyYKWNSj12833122 = rwNCbcqFgIyYKWNSj49887411;     rwNCbcqFgIyYKWNSj49887411 = rwNCbcqFgIyYKWNSj68045973;     rwNCbcqFgIyYKWNSj68045973 = rwNCbcqFgIyYKWNSj70043663;     rwNCbcqFgIyYKWNSj70043663 = rwNCbcqFgIyYKWNSj14119620;     rwNCbcqFgIyYKWNSj14119620 = rwNCbcqFgIyYKWNSj36480931;     rwNCbcqFgIyYKWNSj36480931 = rwNCbcqFgIyYKWNSj47558492;     rwNCbcqFgIyYKWNSj47558492 = rwNCbcqFgIyYKWNSj58500734;     rwNCbcqFgIyYKWNSj58500734 = rwNCbcqFgIyYKWNSj27143043;     rwNCbcqFgIyYKWNSj27143043 = rwNCbcqFgIyYKWNSj73578719;     rwNCbcqFgIyYKWNSj73578719 = rwNCbcqFgIyYKWNSj4948984;     rwNCbcqFgIyYKWNSj4948984 = rwNCbcqFgIyYKWNSj44315157;     rwNCbcqFgIyYKWNSj44315157 = rwNCbcqFgIyYKWNSj51046482;     rwNCbcqFgIyYKWNSj51046482 = rwNCbcqFgIyYKWNSj11413068;     rwNCbcqFgIyYKWNSj11413068 = rwNCbcqFgIyYKWNSj11553032;     rwNCbcqFgIyYKWNSj11553032 = rwNCbcqFgIyYKWNSj47451963;     rwNCbcqFgIyYKWNSj47451963 = rwNCbcqFgIyYKWNSj92293875;     rwNCbcqFgIyYKWNSj92293875 = rwNCbcqFgIyYKWNSj89696339;     rwNCbcqFgIyYKWNSj89696339 = rwNCbcqFgIyYKWNSj19455564;     rwNCbcqFgIyYKWNSj19455564 = rwNCbcqFgIyYKWNSj71578688;     rwNCbcqFgIyYKWNSj71578688 = rwNCbcqFgIyYKWNSj49571948;     rwNCbcqFgIyYKWNSj49571948 = rwNCbcqFgIyYKWNSj69760294;     rwNCbcqFgIyYKWNSj69760294 = rwNCbcqFgIyYKWNSj33553533;     rwNCbcqFgIyYKWNSj33553533 = rwNCbcqFgIyYKWNSj67396460;     rwNCbcqFgIyYKWNSj67396460 = rwNCbcqFgIyYKWNSj95827638;     rwNCbcqFgIyYKWNSj95827638 = rwNCbcqFgIyYKWNSj13960942;     rwNCbcqFgIyYKWNSj13960942 = rwNCbcqFgIyYKWNSj29706015;     rwNCbcqFgIyYKWNSj29706015 = rwNCbcqFgIyYKWNSj28483003;     rwNCbcqFgIyYKWNSj28483003 = rwNCbcqFgIyYKWNSj24618785;     rwNCbcqFgIyYKWNSj24618785 = rwNCbcqFgIyYKWNSj53174788;     rwNCbcqFgIyYKWNSj53174788 = rwNCbcqFgIyYKWNSj6017107;     rwNCbcqFgIyYKWNSj6017107 = rwNCbcqFgIyYKWNSj47010162;     rwNCbcqFgIyYKWNSj47010162 = rwNCbcqFgIyYKWNSj37919424;     rwNCbcqFgIyYKWNSj37919424 = rwNCbcqFgIyYKWNSj66080467;     rwNCbcqFgIyYKWNSj66080467 = rwNCbcqFgIyYKWNSj71430457;     rwNCbcqFgIyYKWNSj71430457 = rwNCbcqFgIyYKWNSj65477276;     rwNCbcqFgIyYKWNSj65477276 = rwNCbcqFgIyYKWNSj98299202;     rwNCbcqFgIyYKWNSj98299202 = rwNCbcqFgIyYKWNSj43494565;     rwNCbcqFgIyYKWNSj43494565 = rwNCbcqFgIyYKWNSj16310338;     rwNCbcqFgIyYKWNSj16310338 = rwNCbcqFgIyYKWNSj66303007;     rwNCbcqFgIyYKWNSj66303007 = rwNCbcqFgIyYKWNSj40140743;     rwNCbcqFgIyYKWNSj40140743 = rwNCbcqFgIyYKWNSj77145316;     rwNCbcqFgIyYKWNSj77145316 = rwNCbcqFgIyYKWNSj75610565;     rwNCbcqFgIyYKWNSj75610565 = rwNCbcqFgIyYKWNSj38902243;     rwNCbcqFgIyYKWNSj38902243 = rwNCbcqFgIyYKWNSj75398571;     rwNCbcqFgIyYKWNSj75398571 = rwNCbcqFgIyYKWNSj14681762;     rwNCbcqFgIyYKWNSj14681762 = rwNCbcqFgIyYKWNSj58690537;     rwNCbcqFgIyYKWNSj58690537 = rwNCbcqFgIyYKWNSj84252539;     rwNCbcqFgIyYKWNSj84252539 = rwNCbcqFgIyYKWNSj86914468;     rwNCbcqFgIyYKWNSj86914468 = rwNCbcqFgIyYKWNSj2040792;     rwNCbcqFgIyYKWNSj2040792 = rwNCbcqFgIyYKWNSj10288762;     rwNCbcqFgIyYKWNSj10288762 = rwNCbcqFgIyYKWNSj50526928;     rwNCbcqFgIyYKWNSj50526928 = rwNCbcqFgIyYKWNSj98380954;     rwNCbcqFgIyYKWNSj98380954 = rwNCbcqFgIyYKWNSj41850617;     rwNCbcqFgIyYKWNSj41850617 = rwNCbcqFgIyYKWNSj88315824;     rwNCbcqFgIyYKWNSj88315824 = rwNCbcqFgIyYKWNSj43072827;     rwNCbcqFgIyYKWNSj43072827 = rwNCbcqFgIyYKWNSj16333878;     rwNCbcqFgIyYKWNSj16333878 = rwNCbcqFgIyYKWNSj649514;     rwNCbcqFgIyYKWNSj649514 = rwNCbcqFgIyYKWNSj74216025;     rwNCbcqFgIyYKWNSj74216025 = rwNCbcqFgIyYKWNSj158678;     rwNCbcqFgIyYKWNSj158678 = rwNCbcqFgIyYKWNSj6774916;     rwNCbcqFgIyYKWNSj6774916 = rwNCbcqFgIyYKWNSj19075490;     rwNCbcqFgIyYKWNSj19075490 = rwNCbcqFgIyYKWNSj33881949;     rwNCbcqFgIyYKWNSj33881949 = rwNCbcqFgIyYKWNSj73968255;     rwNCbcqFgIyYKWNSj73968255 = rwNCbcqFgIyYKWNSj67561613;     rwNCbcqFgIyYKWNSj67561613 = rwNCbcqFgIyYKWNSj57938822;     rwNCbcqFgIyYKWNSj57938822 = rwNCbcqFgIyYKWNSj6395734;     rwNCbcqFgIyYKWNSj6395734 = rwNCbcqFgIyYKWNSj84966014;     rwNCbcqFgIyYKWNSj84966014 = rwNCbcqFgIyYKWNSj39982611;     rwNCbcqFgIyYKWNSj39982611 = rwNCbcqFgIyYKWNSj46075756;     rwNCbcqFgIyYKWNSj46075756 = rwNCbcqFgIyYKWNSj49152761;     rwNCbcqFgIyYKWNSj49152761 = rwNCbcqFgIyYKWNSj48799310;     rwNCbcqFgIyYKWNSj48799310 = rwNCbcqFgIyYKWNSj73386001;     rwNCbcqFgIyYKWNSj73386001 = rwNCbcqFgIyYKWNSj53152556;     rwNCbcqFgIyYKWNSj53152556 = rwNCbcqFgIyYKWNSj31437946;     rwNCbcqFgIyYKWNSj31437946 = rwNCbcqFgIyYKWNSj72426632;     rwNCbcqFgIyYKWNSj72426632 = rwNCbcqFgIyYKWNSj94149729;     rwNCbcqFgIyYKWNSj94149729 = rwNCbcqFgIyYKWNSj94651290;     rwNCbcqFgIyYKWNSj94651290 = rwNCbcqFgIyYKWNSj91997888;     rwNCbcqFgIyYKWNSj91997888 = rwNCbcqFgIyYKWNSj81145876;     rwNCbcqFgIyYKWNSj81145876 = rwNCbcqFgIyYKWNSj55270405;     rwNCbcqFgIyYKWNSj55270405 = rwNCbcqFgIyYKWNSj45453476;     rwNCbcqFgIyYKWNSj45453476 = rwNCbcqFgIyYKWNSj41568535;     rwNCbcqFgIyYKWNSj41568535 = rwNCbcqFgIyYKWNSj30429957;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IBwfZXKKVzxMZGRk80829827() {     double lHWjaUycNWyHhTPCx65888925 = -117918974;    double lHWjaUycNWyHhTPCx16056017 = -408805322;    double lHWjaUycNWyHhTPCx25085712 = -55275113;    double lHWjaUycNWyHhTPCx7869405 = -286229789;    double lHWjaUycNWyHhTPCx59038877 = -48841974;    double lHWjaUycNWyHhTPCx48703834 = -406936830;    double lHWjaUycNWyHhTPCx91004961 = -532791808;    double lHWjaUycNWyHhTPCx21128480 = -103656766;    double lHWjaUycNWyHhTPCx11254624 = -414184823;    double lHWjaUycNWyHhTPCx10568320 = 97376933;    double lHWjaUycNWyHhTPCx36649235 = -624339466;    double lHWjaUycNWyHhTPCx50060903 = -209311759;    double lHWjaUycNWyHhTPCx40817282 = -220294722;    double lHWjaUycNWyHhTPCx93364130 = -500466364;    double lHWjaUycNWyHhTPCx76031034 = -385359604;    double lHWjaUycNWyHhTPCx96970606 = -951064681;    double lHWjaUycNWyHhTPCx93723709 = -234917310;    double lHWjaUycNWyHhTPCx12593313 = -982281754;    double lHWjaUycNWyHhTPCx50437741 = -505593679;    double lHWjaUycNWyHhTPCx61883192 = -939591850;    double lHWjaUycNWyHhTPCx83130526 = -208048854;    double lHWjaUycNWyHhTPCx17541144 = -504867954;    double lHWjaUycNWyHhTPCx7006311 = 38751780;    double lHWjaUycNWyHhTPCx41351438 = -662274408;    double lHWjaUycNWyHhTPCx66518685 = -526472820;    double lHWjaUycNWyHhTPCx37841214 = -574037417;    double lHWjaUycNWyHhTPCx33999294 = -631503017;    double lHWjaUycNWyHhTPCx45677409 = -631618093;    double lHWjaUycNWyHhTPCx90983371 = -235586335;    double lHWjaUycNWyHhTPCx56579893 = -939339302;    double lHWjaUycNWyHhTPCx29996300 = -528208846;    double lHWjaUycNWyHhTPCx49376050 = -279200421;    double lHWjaUycNWyHhTPCx59669974 = -473060141;    double lHWjaUycNWyHhTPCx22042302 = -212117309;    double lHWjaUycNWyHhTPCx55504793 = -826756742;    double lHWjaUycNWyHhTPCx71016407 = -872660786;    double lHWjaUycNWyHhTPCx20990505 = -731910262;    double lHWjaUycNWyHhTPCx787352 = -880249822;    double lHWjaUycNWyHhTPCx150856 = -537280084;    double lHWjaUycNWyHhTPCx97825064 = -386047453;    double lHWjaUycNWyHhTPCx848068 = -641324123;    double lHWjaUycNWyHhTPCx85192764 = -628289352;    double lHWjaUycNWyHhTPCx3344321 = 13352997;    double lHWjaUycNWyHhTPCx69837353 = -729603928;    double lHWjaUycNWyHhTPCx28499462 = -196933413;    double lHWjaUycNWyHhTPCx70580113 = -9054530;    double lHWjaUycNWyHhTPCx72519951 = -269886746;    double lHWjaUycNWyHhTPCx23743419 = -845730690;    double lHWjaUycNWyHhTPCx69902827 = -746155273;    double lHWjaUycNWyHhTPCx62449013 = -134609552;    double lHWjaUycNWyHhTPCx46851191 = -54357025;    double lHWjaUycNWyHhTPCx54915929 = -412194025;    double lHWjaUycNWyHhTPCx34098425 = -959040840;    double lHWjaUycNWyHhTPCx14291050 = -78489483;    double lHWjaUycNWyHhTPCx97699301 = -238462101;    double lHWjaUycNWyHhTPCx48347781 = -613051021;    double lHWjaUycNWyHhTPCx9049707 = -347557103;    double lHWjaUycNWyHhTPCx83734274 = -393000705;    double lHWjaUycNWyHhTPCx41350720 = -759756970;    double lHWjaUycNWyHhTPCx21197663 = -474804557;    double lHWjaUycNWyHhTPCx14704541 = -775433813;    double lHWjaUycNWyHhTPCx45327553 = -901173715;    double lHWjaUycNWyHhTPCx30145108 = -868070432;    double lHWjaUycNWyHhTPCx54674731 = -474845521;    double lHWjaUycNWyHhTPCx80572020 = -374414222;    double lHWjaUycNWyHhTPCx87273184 = -245139046;    double lHWjaUycNWyHhTPCx90390929 = -736251619;    double lHWjaUycNWyHhTPCx18774981 = 91822587;    double lHWjaUycNWyHhTPCx37859338 = -673709623;    double lHWjaUycNWyHhTPCx5014627 = -512698818;    double lHWjaUycNWyHhTPCx75980101 = -119154419;    double lHWjaUycNWyHhTPCx92936357 = -354667489;    double lHWjaUycNWyHhTPCx12442458 = -345001671;    double lHWjaUycNWyHhTPCx52612676 = -19546226;    double lHWjaUycNWyHhTPCx61035124 = -198267728;    double lHWjaUycNWyHhTPCx97937761 = -579759503;    double lHWjaUycNWyHhTPCx14196824 = -418220951;    double lHWjaUycNWyHhTPCx37168957 = -231644293;    double lHWjaUycNWyHhTPCx12851977 = -365340996;    double lHWjaUycNWyHhTPCx95938571 = -417418290;    double lHWjaUycNWyHhTPCx65321263 = -204150672;    double lHWjaUycNWyHhTPCx10255876 = -785772328;    double lHWjaUycNWyHhTPCx75774581 = -885462821;    double lHWjaUycNWyHhTPCx28534359 = -976784;    double lHWjaUycNWyHhTPCx9728702 = -784982277;    double lHWjaUycNWyHhTPCx75080370 = -16014821;    double lHWjaUycNWyHhTPCx15277626 = -320159582;    double lHWjaUycNWyHhTPCx45378925 = -294570659;    double lHWjaUycNWyHhTPCx24343001 = -973655209;    double lHWjaUycNWyHhTPCx7157012 = -113705721;    double lHWjaUycNWyHhTPCx61966701 = -425103684;    double lHWjaUycNWyHhTPCx37256230 = -238909557;    double lHWjaUycNWyHhTPCx59436631 = -20492853;    double lHWjaUycNWyHhTPCx78953193 = 37524473;    double lHWjaUycNWyHhTPCx83120524 = -610613641;    double lHWjaUycNWyHhTPCx55520515 = -740150408;    double lHWjaUycNWyHhTPCx55047657 = -760218921;    double lHWjaUycNWyHhTPCx48669589 = -511801483;    double lHWjaUycNWyHhTPCx89265333 = -255189706;    double lHWjaUycNWyHhTPCx41226277 = -117918974;     lHWjaUycNWyHhTPCx65888925 = lHWjaUycNWyHhTPCx16056017;     lHWjaUycNWyHhTPCx16056017 = lHWjaUycNWyHhTPCx25085712;     lHWjaUycNWyHhTPCx25085712 = lHWjaUycNWyHhTPCx7869405;     lHWjaUycNWyHhTPCx7869405 = lHWjaUycNWyHhTPCx59038877;     lHWjaUycNWyHhTPCx59038877 = lHWjaUycNWyHhTPCx48703834;     lHWjaUycNWyHhTPCx48703834 = lHWjaUycNWyHhTPCx91004961;     lHWjaUycNWyHhTPCx91004961 = lHWjaUycNWyHhTPCx21128480;     lHWjaUycNWyHhTPCx21128480 = lHWjaUycNWyHhTPCx11254624;     lHWjaUycNWyHhTPCx11254624 = lHWjaUycNWyHhTPCx10568320;     lHWjaUycNWyHhTPCx10568320 = lHWjaUycNWyHhTPCx36649235;     lHWjaUycNWyHhTPCx36649235 = lHWjaUycNWyHhTPCx50060903;     lHWjaUycNWyHhTPCx50060903 = lHWjaUycNWyHhTPCx40817282;     lHWjaUycNWyHhTPCx40817282 = lHWjaUycNWyHhTPCx93364130;     lHWjaUycNWyHhTPCx93364130 = lHWjaUycNWyHhTPCx76031034;     lHWjaUycNWyHhTPCx76031034 = lHWjaUycNWyHhTPCx96970606;     lHWjaUycNWyHhTPCx96970606 = lHWjaUycNWyHhTPCx93723709;     lHWjaUycNWyHhTPCx93723709 = lHWjaUycNWyHhTPCx12593313;     lHWjaUycNWyHhTPCx12593313 = lHWjaUycNWyHhTPCx50437741;     lHWjaUycNWyHhTPCx50437741 = lHWjaUycNWyHhTPCx61883192;     lHWjaUycNWyHhTPCx61883192 = lHWjaUycNWyHhTPCx83130526;     lHWjaUycNWyHhTPCx83130526 = lHWjaUycNWyHhTPCx17541144;     lHWjaUycNWyHhTPCx17541144 = lHWjaUycNWyHhTPCx7006311;     lHWjaUycNWyHhTPCx7006311 = lHWjaUycNWyHhTPCx41351438;     lHWjaUycNWyHhTPCx41351438 = lHWjaUycNWyHhTPCx66518685;     lHWjaUycNWyHhTPCx66518685 = lHWjaUycNWyHhTPCx37841214;     lHWjaUycNWyHhTPCx37841214 = lHWjaUycNWyHhTPCx33999294;     lHWjaUycNWyHhTPCx33999294 = lHWjaUycNWyHhTPCx45677409;     lHWjaUycNWyHhTPCx45677409 = lHWjaUycNWyHhTPCx90983371;     lHWjaUycNWyHhTPCx90983371 = lHWjaUycNWyHhTPCx56579893;     lHWjaUycNWyHhTPCx56579893 = lHWjaUycNWyHhTPCx29996300;     lHWjaUycNWyHhTPCx29996300 = lHWjaUycNWyHhTPCx49376050;     lHWjaUycNWyHhTPCx49376050 = lHWjaUycNWyHhTPCx59669974;     lHWjaUycNWyHhTPCx59669974 = lHWjaUycNWyHhTPCx22042302;     lHWjaUycNWyHhTPCx22042302 = lHWjaUycNWyHhTPCx55504793;     lHWjaUycNWyHhTPCx55504793 = lHWjaUycNWyHhTPCx71016407;     lHWjaUycNWyHhTPCx71016407 = lHWjaUycNWyHhTPCx20990505;     lHWjaUycNWyHhTPCx20990505 = lHWjaUycNWyHhTPCx787352;     lHWjaUycNWyHhTPCx787352 = lHWjaUycNWyHhTPCx150856;     lHWjaUycNWyHhTPCx150856 = lHWjaUycNWyHhTPCx97825064;     lHWjaUycNWyHhTPCx97825064 = lHWjaUycNWyHhTPCx848068;     lHWjaUycNWyHhTPCx848068 = lHWjaUycNWyHhTPCx85192764;     lHWjaUycNWyHhTPCx85192764 = lHWjaUycNWyHhTPCx3344321;     lHWjaUycNWyHhTPCx3344321 = lHWjaUycNWyHhTPCx69837353;     lHWjaUycNWyHhTPCx69837353 = lHWjaUycNWyHhTPCx28499462;     lHWjaUycNWyHhTPCx28499462 = lHWjaUycNWyHhTPCx70580113;     lHWjaUycNWyHhTPCx70580113 = lHWjaUycNWyHhTPCx72519951;     lHWjaUycNWyHhTPCx72519951 = lHWjaUycNWyHhTPCx23743419;     lHWjaUycNWyHhTPCx23743419 = lHWjaUycNWyHhTPCx69902827;     lHWjaUycNWyHhTPCx69902827 = lHWjaUycNWyHhTPCx62449013;     lHWjaUycNWyHhTPCx62449013 = lHWjaUycNWyHhTPCx46851191;     lHWjaUycNWyHhTPCx46851191 = lHWjaUycNWyHhTPCx54915929;     lHWjaUycNWyHhTPCx54915929 = lHWjaUycNWyHhTPCx34098425;     lHWjaUycNWyHhTPCx34098425 = lHWjaUycNWyHhTPCx14291050;     lHWjaUycNWyHhTPCx14291050 = lHWjaUycNWyHhTPCx97699301;     lHWjaUycNWyHhTPCx97699301 = lHWjaUycNWyHhTPCx48347781;     lHWjaUycNWyHhTPCx48347781 = lHWjaUycNWyHhTPCx9049707;     lHWjaUycNWyHhTPCx9049707 = lHWjaUycNWyHhTPCx83734274;     lHWjaUycNWyHhTPCx83734274 = lHWjaUycNWyHhTPCx41350720;     lHWjaUycNWyHhTPCx41350720 = lHWjaUycNWyHhTPCx21197663;     lHWjaUycNWyHhTPCx21197663 = lHWjaUycNWyHhTPCx14704541;     lHWjaUycNWyHhTPCx14704541 = lHWjaUycNWyHhTPCx45327553;     lHWjaUycNWyHhTPCx45327553 = lHWjaUycNWyHhTPCx30145108;     lHWjaUycNWyHhTPCx30145108 = lHWjaUycNWyHhTPCx54674731;     lHWjaUycNWyHhTPCx54674731 = lHWjaUycNWyHhTPCx80572020;     lHWjaUycNWyHhTPCx80572020 = lHWjaUycNWyHhTPCx87273184;     lHWjaUycNWyHhTPCx87273184 = lHWjaUycNWyHhTPCx90390929;     lHWjaUycNWyHhTPCx90390929 = lHWjaUycNWyHhTPCx18774981;     lHWjaUycNWyHhTPCx18774981 = lHWjaUycNWyHhTPCx37859338;     lHWjaUycNWyHhTPCx37859338 = lHWjaUycNWyHhTPCx5014627;     lHWjaUycNWyHhTPCx5014627 = lHWjaUycNWyHhTPCx75980101;     lHWjaUycNWyHhTPCx75980101 = lHWjaUycNWyHhTPCx92936357;     lHWjaUycNWyHhTPCx92936357 = lHWjaUycNWyHhTPCx12442458;     lHWjaUycNWyHhTPCx12442458 = lHWjaUycNWyHhTPCx52612676;     lHWjaUycNWyHhTPCx52612676 = lHWjaUycNWyHhTPCx61035124;     lHWjaUycNWyHhTPCx61035124 = lHWjaUycNWyHhTPCx97937761;     lHWjaUycNWyHhTPCx97937761 = lHWjaUycNWyHhTPCx14196824;     lHWjaUycNWyHhTPCx14196824 = lHWjaUycNWyHhTPCx37168957;     lHWjaUycNWyHhTPCx37168957 = lHWjaUycNWyHhTPCx12851977;     lHWjaUycNWyHhTPCx12851977 = lHWjaUycNWyHhTPCx95938571;     lHWjaUycNWyHhTPCx95938571 = lHWjaUycNWyHhTPCx65321263;     lHWjaUycNWyHhTPCx65321263 = lHWjaUycNWyHhTPCx10255876;     lHWjaUycNWyHhTPCx10255876 = lHWjaUycNWyHhTPCx75774581;     lHWjaUycNWyHhTPCx75774581 = lHWjaUycNWyHhTPCx28534359;     lHWjaUycNWyHhTPCx28534359 = lHWjaUycNWyHhTPCx9728702;     lHWjaUycNWyHhTPCx9728702 = lHWjaUycNWyHhTPCx75080370;     lHWjaUycNWyHhTPCx75080370 = lHWjaUycNWyHhTPCx15277626;     lHWjaUycNWyHhTPCx15277626 = lHWjaUycNWyHhTPCx45378925;     lHWjaUycNWyHhTPCx45378925 = lHWjaUycNWyHhTPCx24343001;     lHWjaUycNWyHhTPCx24343001 = lHWjaUycNWyHhTPCx7157012;     lHWjaUycNWyHhTPCx7157012 = lHWjaUycNWyHhTPCx61966701;     lHWjaUycNWyHhTPCx61966701 = lHWjaUycNWyHhTPCx37256230;     lHWjaUycNWyHhTPCx37256230 = lHWjaUycNWyHhTPCx59436631;     lHWjaUycNWyHhTPCx59436631 = lHWjaUycNWyHhTPCx78953193;     lHWjaUycNWyHhTPCx78953193 = lHWjaUycNWyHhTPCx83120524;     lHWjaUycNWyHhTPCx83120524 = lHWjaUycNWyHhTPCx55520515;     lHWjaUycNWyHhTPCx55520515 = lHWjaUycNWyHhTPCx55047657;     lHWjaUycNWyHhTPCx55047657 = lHWjaUycNWyHhTPCx48669589;     lHWjaUycNWyHhTPCx48669589 = lHWjaUycNWyHhTPCx89265333;     lHWjaUycNWyHhTPCx89265333 = lHWjaUycNWyHhTPCx41226277;     lHWjaUycNWyHhTPCx41226277 = lHWjaUycNWyHhTPCx65888925;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void DfJeHVWCcMPOlWGK95878895() {     double BwgrPszZktaOAdLjx72006032 = -229820863;    double BwgrPszZktaOAdLjx59429296 = -118405103;    double BwgrPszZktaOAdLjx41127896 = -206108034;    double BwgrPszZktaOAdLjx34944186 = -476978419;    double BwgrPszZktaOAdLjx27934366 = -569491845;    double BwgrPszZktaOAdLjx40121036 = -671433065;    double BwgrPszZktaOAdLjx51493927 = -671756785;    double BwgrPszZktaOAdLjx70888371 = -390530679;    double BwgrPszZktaOAdLjx75897119 = -182217488;    double BwgrPszZktaOAdLjx51797979 = -231598544;    double BwgrPszZktaOAdLjx96759524 = -501777053;    double BwgrPszZktaOAdLjx81883946 = -653717961;    double BwgrPszZktaOAdLjx27026401 = -660185810;    double BwgrPszZktaOAdLjx76507807 = 67295068;    double BwgrPszZktaOAdLjx71773461 = 24178230;    double BwgrPszZktaOAdLjx38476000 = -924064224;    double BwgrPszZktaOAdLjx87559712 = -851462423;    double BwgrPszZktaOAdLjx9646466 = -647804397;    double BwgrPszZktaOAdLjx12267583 = -911362776;    double BwgrPszZktaOAdLjx42433356 = -196494770;    double BwgrPszZktaOAdLjx71683719 = -63536310;    double BwgrPszZktaOAdLjx4062168 = -412797670;    double BwgrPszZktaOAdLjx42597102 = -807765707;    double BwgrPszZktaOAdLjx25343812 = -884579396;    double BwgrPszZktaOAdLjx10218335 = -899668853;    double BwgrPszZktaOAdLjx62562890 = -452692740;    double BwgrPszZktaOAdLjx32459072 = -532165220;    double BwgrPszZktaOAdLjx58784534 = -697547047;    double BwgrPszZktaOAdLjx57115627 = -457969040;    double BwgrPszZktaOAdLjx25195530 = -756810170;    double BwgrPszZktaOAdLjx22238849 = -441529784;    double BwgrPszZktaOAdLjx46986672 = 11352763;    double BwgrPszZktaOAdLjx69380923 = -365673511;    double BwgrPszZktaOAdLjx16872866 = 36340926;    double BwgrPszZktaOAdLjx16979445 = -365489847;    double BwgrPszZktaOAdLjx39742321 = 13313106;    double BwgrPszZktaOAdLjx2252440 = -685546131;    double BwgrPszZktaOAdLjx34877564 = -651755832;    double BwgrPszZktaOAdLjx73761109 = -531203581;    double BwgrPszZktaOAdLjx85327002 = -859486676;    double BwgrPszZktaOAdLjx6517292 = -706598017;    double BwgrPszZktaOAdLjx17343222 = -898431630;    double BwgrPszZktaOAdLjx81411298 = -67451587;    double BwgrPszZktaOAdLjx57245094 = -748946866;    double BwgrPszZktaOAdLjx40451927 = -958831567;    double BwgrPszZktaOAdLjx82519308 = 5729354;    double BwgrPszZktaOAdLjx90298525 = -388662283;    double BwgrPszZktaOAdLjx31700359 = -855626222;    double BwgrPszZktaOAdLjx79578556 = -831892261;    double BwgrPszZktaOAdLjx75360665 = -221620360;    double BwgrPszZktaOAdLjx82551869 = -996650697;    double BwgrPszZktaOAdLjx65725649 = -212920345;    double BwgrPszZktaOAdLjx82346881 = -750830427;    double BwgrPszZktaOAdLjx10259930 = -398959628;    double BwgrPszZktaOAdLjx24195176 = -938993883;    double BwgrPszZktaOAdLjx67943864 = -817023193;    double BwgrPszZktaOAdLjx16832194 = -310639397;    double BwgrPszZktaOAdLjx15784084 = -321528638;    double BwgrPszZktaOAdLjx24725851 = -577309567;    double BwgrPszZktaOAdLjx65371476 = -16799105;    double BwgrPszZktaOAdLjx7661965 = -39267845;    double BwgrPszZktaOAdLjx92709393 = -974209739;    double BwgrPszZktaOAdLjx13772744 = -932561640;    double BwgrPszZktaOAdLjx50701590 = -425407319;    double BwgrPszZktaOAdLjx29559130 = -790068761;    double BwgrPszZktaOAdLjx49772853 = -413129817;    double BwgrPszZktaOAdLjx12503024 = -188044450;    double BwgrPszZktaOAdLjx10153535 = -596526737;    double BwgrPszZktaOAdLjx59528362 = -567215086;    double BwgrPszZktaOAdLjx32031140 = -989134877;    double BwgrPszZktaOAdLjx36223561 = -138518093;    double BwgrPszZktaOAdLjx52682148 = -99706592;    double BwgrPszZktaOAdLjx35885357 = -16600817;    double BwgrPszZktaOAdLjx26940580 = 48123900;    double BwgrPszZktaOAdLjx35916065 = -489896753;    double BwgrPszZktaOAdLjx54340498 = -165104680;    double BwgrPszZktaOAdLjx22650870 = -245346084;    double BwgrPszZktaOAdLjx85352008 = 41181158;    double BwgrPszZktaOAdLjx84891885 = -925747830;    double BwgrPszZktaOAdLjx27699027 = -805398207;    double BwgrPszZktaOAdLjx72264364 = 35969543;    double BwgrPszZktaOAdLjx758713 = -676538999;    double BwgrPszZktaOAdLjx79205977 = -865654786;    double BwgrPszZktaOAdLjx81754961 = -136348680;    double BwgrPszZktaOAdLjx42643660 = -760159474;    double BwgrPszZktaOAdLjx56513200 = -128609439;    double BwgrPszZktaOAdLjx64639790 = -237816810;    double BwgrPszZktaOAdLjx59120993 = -966713884;    double BwgrPszZktaOAdLjx92677690 = -24665191;    double BwgrPszZktaOAdLjx49035580 = -548466654;    double BwgrPszZktaOAdLjx22910127 = -676047498;    double BwgrPszZktaOAdLjx86468355 = -264017493;    double BwgrPszZktaOAdLjx10151714 = 25553735;    double BwgrPszZktaOAdLjx8389634 = -414404476;    double BwgrPszZktaOAdLjx77665038 = -720218831;    double BwgrPszZktaOAdLjx13807899 = -732388278;    double BwgrPszZktaOAdLjx3570479 = -965869990;    double BwgrPszZktaOAdLjx30709709 = -642044268;    double BwgrPszZktaOAdLjx27685964 = -958878105;    double BwgrPszZktaOAdLjx90679073 = -229820863;     BwgrPszZktaOAdLjx72006032 = BwgrPszZktaOAdLjx59429296;     BwgrPszZktaOAdLjx59429296 = BwgrPszZktaOAdLjx41127896;     BwgrPszZktaOAdLjx41127896 = BwgrPszZktaOAdLjx34944186;     BwgrPszZktaOAdLjx34944186 = BwgrPszZktaOAdLjx27934366;     BwgrPszZktaOAdLjx27934366 = BwgrPszZktaOAdLjx40121036;     BwgrPszZktaOAdLjx40121036 = BwgrPszZktaOAdLjx51493927;     BwgrPszZktaOAdLjx51493927 = BwgrPszZktaOAdLjx70888371;     BwgrPszZktaOAdLjx70888371 = BwgrPszZktaOAdLjx75897119;     BwgrPszZktaOAdLjx75897119 = BwgrPszZktaOAdLjx51797979;     BwgrPszZktaOAdLjx51797979 = BwgrPszZktaOAdLjx96759524;     BwgrPszZktaOAdLjx96759524 = BwgrPszZktaOAdLjx81883946;     BwgrPszZktaOAdLjx81883946 = BwgrPszZktaOAdLjx27026401;     BwgrPszZktaOAdLjx27026401 = BwgrPszZktaOAdLjx76507807;     BwgrPszZktaOAdLjx76507807 = BwgrPszZktaOAdLjx71773461;     BwgrPszZktaOAdLjx71773461 = BwgrPszZktaOAdLjx38476000;     BwgrPszZktaOAdLjx38476000 = BwgrPszZktaOAdLjx87559712;     BwgrPszZktaOAdLjx87559712 = BwgrPszZktaOAdLjx9646466;     BwgrPszZktaOAdLjx9646466 = BwgrPszZktaOAdLjx12267583;     BwgrPszZktaOAdLjx12267583 = BwgrPszZktaOAdLjx42433356;     BwgrPszZktaOAdLjx42433356 = BwgrPszZktaOAdLjx71683719;     BwgrPszZktaOAdLjx71683719 = BwgrPszZktaOAdLjx4062168;     BwgrPszZktaOAdLjx4062168 = BwgrPszZktaOAdLjx42597102;     BwgrPszZktaOAdLjx42597102 = BwgrPszZktaOAdLjx25343812;     BwgrPszZktaOAdLjx25343812 = BwgrPszZktaOAdLjx10218335;     BwgrPszZktaOAdLjx10218335 = BwgrPszZktaOAdLjx62562890;     BwgrPszZktaOAdLjx62562890 = BwgrPszZktaOAdLjx32459072;     BwgrPszZktaOAdLjx32459072 = BwgrPszZktaOAdLjx58784534;     BwgrPszZktaOAdLjx58784534 = BwgrPszZktaOAdLjx57115627;     BwgrPszZktaOAdLjx57115627 = BwgrPszZktaOAdLjx25195530;     BwgrPszZktaOAdLjx25195530 = BwgrPszZktaOAdLjx22238849;     BwgrPszZktaOAdLjx22238849 = BwgrPszZktaOAdLjx46986672;     BwgrPszZktaOAdLjx46986672 = BwgrPszZktaOAdLjx69380923;     BwgrPszZktaOAdLjx69380923 = BwgrPszZktaOAdLjx16872866;     BwgrPszZktaOAdLjx16872866 = BwgrPszZktaOAdLjx16979445;     BwgrPszZktaOAdLjx16979445 = BwgrPszZktaOAdLjx39742321;     BwgrPszZktaOAdLjx39742321 = BwgrPszZktaOAdLjx2252440;     BwgrPszZktaOAdLjx2252440 = BwgrPszZktaOAdLjx34877564;     BwgrPszZktaOAdLjx34877564 = BwgrPszZktaOAdLjx73761109;     BwgrPszZktaOAdLjx73761109 = BwgrPszZktaOAdLjx85327002;     BwgrPszZktaOAdLjx85327002 = BwgrPszZktaOAdLjx6517292;     BwgrPszZktaOAdLjx6517292 = BwgrPszZktaOAdLjx17343222;     BwgrPszZktaOAdLjx17343222 = BwgrPszZktaOAdLjx81411298;     BwgrPszZktaOAdLjx81411298 = BwgrPszZktaOAdLjx57245094;     BwgrPszZktaOAdLjx57245094 = BwgrPszZktaOAdLjx40451927;     BwgrPszZktaOAdLjx40451927 = BwgrPszZktaOAdLjx82519308;     BwgrPszZktaOAdLjx82519308 = BwgrPszZktaOAdLjx90298525;     BwgrPszZktaOAdLjx90298525 = BwgrPszZktaOAdLjx31700359;     BwgrPszZktaOAdLjx31700359 = BwgrPszZktaOAdLjx79578556;     BwgrPszZktaOAdLjx79578556 = BwgrPszZktaOAdLjx75360665;     BwgrPszZktaOAdLjx75360665 = BwgrPszZktaOAdLjx82551869;     BwgrPszZktaOAdLjx82551869 = BwgrPszZktaOAdLjx65725649;     BwgrPszZktaOAdLjx65725649 = BwgrPszZktaOAdLjx82346881;     BwgrPszZktaOAdLjx82346881 = BwgrPszZktaOAdLjx10259930;     BwgrPszZktaOAdLjx10259930 = BwgrPszZktaOAdLjx24195176;     BwgrPszZktaOAdLjx24195176 = BwgrPszZktaOAdLjx67943864;     BwgrPszZktaOAdLjx67943864 = BwgrPszZktaOAdLjx16832194;     BwgrPszZktaOAdLjx16832194 = BwgrPszZktaOAdLjx15784084;     BwgrPszZktaOAdLjx15784084 = BwgrPszZktaOAdLjx24725851;     BwgrPszZktaOAdLjx24725851 = BwgrPszZktaOAdLjx65371476;     BwgrPszZktaOAdLjx65371476 = BwgrPszZktaOAdLjx7661965;     BwgrPszZktaOAdLjx7661965 = BwgrPszZktaOAdLjx92709393;     BwgrPszZktaOAdLjx92709393 = BwgrPszZktaOAdLjx13772744;     BwgrPszZktaOAdLjx13772744 = BwgrPszZktaOAdLjx50701590;     BwgrPszZktaOAdLjx50701590 = BwgrPszZktaOAdLjx29559130;     BwgrPszZktaOAdLjx29559130 = BwgrPszZktaOAdLjx49772853;     BwgrPszZktaOAdLjx49772853 = BwgrPszZktaOAdLjx12503024;     BwgrPszZktaOAdLjx12503024 = BwgrPszZktaOAdLjx10153535;     BwgrPszZktaOAdLjx10153535 = BwgrPszZktaOAdLjx59528362;     BwgrPszZktaOAdLjx59528362 = BwgrPszZktaOAdLjx32031140;     BwgrPszZktaOAdLjx32031140 = BwgrPszZktaOAdLjx36223561;     BwgrPszZktaOAdLjx36223561 = BwgrPszZktaOAdLjx52682148;     BwgrPszZktaOAdLjx52682148 = BwgrPszZktaOAdLjx35885357;     BwgrPszZktaOAdLjx35885357 = BwgrPszZktaOAdLjx26940580;     BwgrPszZktaOAdLjx26940580 = BwgrPszZktaOAdLjx35916065;     BwgrPszZktaOAdLjx35916065 = BwgrPszZktaOAdLjx54340498;     BwgrPszZktaOAdLjx54340498 = BwgrPszZktaOAdLjx22650870;     BwgrPszZktaOAdLjx22650870 = BwgrPszZktaOAdLjx85352008;     BwgrPszZktaOAdLjx85352008 = BwgrPszZktaOAdLjx84891885;     BwgrPszZktaOAdLjx84891885 = BwgrPszZktaOAdLjx27699027;     BwgrPszZktaOAdLjx27699027 = BwgrPszZktaOAdLjx72264364;     BwgrPszZktaOAdLjx72264364 = BwgrPszZktaOAdLjx758713;     BwgrPszZktaOAdLjx758713 = BwgrPszZktaOAdLjx79205977;     BwgrPszZktaOAdLjx79205977 = BwgrPszZktaOAdLjx81754961;     BwgrPszZktaOAdLjx81754961 = BwgrPszZktaOAdLjx42643660;     BwgrPszZktaOAdLjx42643660 = BwgrPszZktaOAdLjx56513200;     BwgrPszZktaOAdLjx56513200 = BwgrPszZktaOAdLjx64639790;     BwgrPszZktaOAdLjx64639790 = BwgrPszZktaOAdLjx59120993;     BwgrPszZktaOAdLjx59120993 = BwgrPszZktaOAdLjx92677690;     BwgrPszZktaOAdLjx92677690 = BwgrPszZktaOAdLjx49035580;     BwgrPszZktaOAdLjx49035580 = BwgrPszZktaOAdLjx22910127;     BwgrPszZktaOAdLjx22910127 = BwgrPszZktaOAdLjx86468355;     BwgrPszZktaOAdLjx86468355 = BwgrPszZktaOAdLjx10151714;     BwgrPszZktaOAdLjx10151714 = BwgrPszZktaOAdLjx8389634;     BwgrPszZktaOAdLjx8389634 = BwgrPszZktaOAdLjx77665038;     BwgrPszZktaOAdLjx77665038 = BwgrPszZktaOAdLjx13807899;     BwgrPszZktaOAdLjx13807899 = BwgrPszZktaOAdLjx3570479;     BwgrPszZktaOAdLjx3570479 = BwgrPszZktaOAdLjx30709709;     BwgrPszZktaOAdLjx30709709 = BwgrPszZktaOAdLjx27685964;     BwgrPszZktaOAdLjx27685964 = BwgrPszZktaOAdLjx90679073;     BwgrPszZktaOAdLjx90679073 = BwgrPszZktaOAdLjx72006032;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hiCusaAanGiqXCVY32523870() {     double PyuQGnUajxqKdfoIZ94393149 = -964224826;    double PyuQGnUajxqKdfoIZ43776797 = -66948158;    double PyuQGnUajxqKdfoIZ69897174 = -445761878;    double PyuQGnUajxqKdfoIZ49950874 = -483385951;    double PyuQGnUajxqKdfoIZ26108620 = -60548846;    double PyuQGnUajxqKdfoIZ55004153 = -532041443;    double PyuQGnUajxqKdfoIZ43649666 = -493744091;    double PyuQGnUajxqKdfoIZ46522229 = -861127324;    double PyuQGnUajxqKdfoIZ73060250 = -66413527;    double PyuQGnUajxqKdfoIZ43561563 = 25660123;    double PyuQGnUajxqKdfoIZ34301385 = -498401454;    double PyuQGnUajxqKdfoIZ37786895 = -904463254;    double PyuQGnUajxqKdfoIZ94034581 = -178903130;    double PyuQGnUajxqKdfoIZ19122559 = -460413689;    double PyuQGnUajxqKdfoIZ8508738 = -703299483;    double PyuQGnUajxqKdfoIZ82731912 = -205731766;    double PyuQGnUajxqKdfoIZ37383064 = -28742243;    double PyuQGnUajxqKdfoIZ15622883 = 452913;    double PyuQGnUajxqKdfoIZ53549213 = 53535099;    double PyuQGnUajxqKdfoIZ20949640 = -449129223;    double PyuQGnUajxqKdfoIZ72566826 = -55023618;    double PyuQGnUajxqKdfoIZ67877386 = -917563984;    double PyuQGnUajxqKdfoIZ45890425 = -983846299;    double PyuQGnUajxqKdfoIZ51843599 = -348572253;    double PyuQGnUajxqKdfoIZ45758582 = -521227977;    double PyuQGnUajxqKdfoIZ50865884 = -896586964;    double PyuQGnUajxqKdfoIZ79548492 = -289751713;    double PyuQGnUajxqKdfoIZ64695984 = -486001501;    double PyuQGnUajxqKdfoIZ64008188 = -802226159;    double PyuQGnUajxqKdfoIZ34352709 = -677106036;    double PyuQGnUajxqKdfoIZ66529654 = -522845936;    double PyuQGnUajxqKdfoIZ4527863 = -640718591;    double PyuQGnUajxqKdfoIZ50419593 = -895017006;    double PyuQGnUajxqKdfoIZ76261553 = -287785924;    double PyuQGnUajxqKdfoIZ93208809 = -429246269;    double PyuQGnUajxqKdfoIZ81435189 = -302082270;    double PyuQGnUajxqKdfoIZ95692151 = -418494083;    double PyuQGnUajxqKdfoIZ88503665 = -519381304;    double PyuQGnUajxqKdfoIZ22069241 = -932249612;    double PyuQGnUajxqKdfoIZ5512718 = -995710705;    double PyuQGnUajxqKdfoIZ35274663 = -109775683;    double PyuQGnUajxqKdfoIZ5851613 = -656340321;    double PyuQGnUajxqKdfoIZ86961490 = 45017829;    double PyuQGnUajxqKdfoIZ56689213 = -746310515;    double PyuQGnUajxqKdfoIZ8435193 = -767975045;    double PyuQGnUajxqKdfoIZ90619590 = -176499742;    double PyuQGnUajxqKdfoIZ2595924 = -967072536;    double PyuQGnUajxqKdfoIZ76299034 = -612161213;    double PyuQGnUajxqKdfoIZ74038461 = -781608960;    double PyuQGnUajxqKdfoIZ72444411 = -363640619;    double PyuQGnUajxqKdfoIZ91421085 = -251726974;    double PyuQGnUajxqKdfoIZ39576906 = -643739627;    double PyuQGnUajxqKdfoIZ60661285 = -422741657;    double PyuQGnUajxqKdfoIZ70546475 = -674640437;    double PyuQGnUajxqKdfoIZ59957043 = 20302002;    double PyuQGnUajxqKdfoIZ26515764 = 53339157;    double PyuQGnUajxqKdfoIZ97886371 = -83101859;    double PyuQGnUajxqKdfoIZ18053576 = 2810375;    double PyuQGnUajxqKdfoIZ4192293 = -962157974;    double PyuQGnUajxqKdfoIZ75242735 = -163961882;    double PyuQGnUajxqKdfoIZ75455661 = -142289730;    double PyuQGnUajxqKdfoIZ78953682 = 92257409;    double PyuQGnUajxqKdfoIZ82514041 = 41098835;    double PyuQGnUajxqKdfoIZ38707541 = -389307492;    double PyuQGnUajxqKdfoIZ77031909 = -451493942;    double PyuQGnUajxqKdfoIZ29773522 = -857682863;    double PyuQGnUajxqKdfoIZ87367301 = 90553751;    double PyuQGnUajxqKdfoIZ17773029 = -891117206;    double PyuQGnUajxqKdfoIZ25913750 = 68832580;    double PyuQGnUajxqKdfoIZ27073548 = -301217213;    double PyuQGnUajxqKdfoIZ87039761 = -787237684;    double PyuQGnUajxqKdfoIZ48879399 = -509360940;    double PyuQGnUajxqKdfoIZ93553641 = -67297476;    double PyuQGnUajxqKdfoIZ48036496 = 49245804;    double PyuQGnUajxqKdfoIZ85674977 = -239353541;    double PyuQGnUajxqKdfoIZ66715214 = -398683298;    double PyuQGnUajxqKdfoIZ80915895 = -862581813;    double PyuQGnUajxqKdfoIZ89201212 = -137535785;    double PyuQGnUajxqKdfoIZ43408406 = -580597209;    double PyuQGnUajxqKdfoIZ55138991 = -244728236;    double PyuQGnUajxqKdfoIZ48269961 = -929514429;    double PyuQGnUajxqKdfoIZ3249459 = -677590501;    double PyuQGnUajxqKdfoIZ90657522 = -704392541;    double PyuQGnUajxqKdfoIZ91563776 = -338585540;    double PyuQGnUajxqKdfoIZ42931623 = -325379063;    double PyuQGnUajxqKdfoIZ26952748 = -879106309;    double PyuQGnUajxqKdfoIZ43866578 = -117976935;    double PyuQGnUajxqKdfoIZ79873118 = -120376570;    double PyuQGnUajxqKdfoIZ16304510 = -208087926;    double PyuQGnUajxqKdfoIZ66693046 = -382585427;    double PyuQGnUajxqKdfoIZ83548818 = -118980412;    double PyuQGnUajxqKdfoIZ77638576 = -321304458;    double PyuQGnUajxqKdfoIZ84311372 = -557223330;    double PyuQGnUajxqKdfoIZ46826505 = -668287730;    double PyuQGnUajxqKdfoIZ30057056 = -753420975;    double PyuQGnUajxqKdfoIZ56320980 = -102033092;    double PyuQGnUajxqKdfoIZ23337572 = -597439156;    double PyuQGnUajxqKdfoIZ48253949 = -565674680;    double PyuQGnUajxqKdfoIZ79657303 = -194816573;    double PyuQGnUajxqKdfoIZ78661671 = -964224826;     PyuQGnUajxqKdfoIZ94393149 = PyuQGnUajxqKdfoIZ43776797;     PyuQGnUajxqKdfoIZ43776797 = PyuQGnUajxqKdfoIZ69897174;     PyuQGnUajxqKdfoIZ69897174 = PyuQGnUajxqKdfoIZ49950874;     PyuQGnUajxqKdfoIZ49950874 = PyuQGnUajxqKdfoIZ26108620;     PyuQGnUajxqKdfoIZ26108620 = PyuQGnUajxqKdfoIZ55004153;     PyuQGnUajxqKdfoIZ55004153 = PyuQGnUajxqKdfoIZ43649666;     PyuQGnUajxqKdfoIZ43649666 = PyuQGnUajxqKdfoIZ46522229;     PyuQGnUajxqKdfoIZ46522229 = PyuQGnUajxqKdfoIZ73060250;     PyuQGnUajxqKdfoIZ73060250 = PyuQGnUajxqKdfoIZ43561563;     PyuQGnUajxqKdfoIZ43561563 = PyuQGnUajxqKdfoIZ34301385;     PyuQGnUajxqKdfoIZ34301385 = PyuQGnUajxqKdfoIZ37786895;     PyuQGnUajxqKdfoIZ37786895 = PyuQGnUajxqKdfoIZ94034581;     PyuQGnUajxqKdfoIZ94034581 = PyuQGnUajxqKdfoIZ19122559;     PyuQGnUajxqKdfoIZ19122559 = PyuQGnUajxqKdfoIZ8508738;     PyuQGnUajxqKdfoIZ8508738 = PyuQGnUajxqKdfoIZ82731912;     PyuQGnUajxqKdfoIZ82731912 = PyuQGnUajxqKdfoIZ37383064;     PyuQGnUajxqKdfoIZ37383064 = PyuQGnUajxqKdfoIZ15622883;     PyuQGnUajxqKdfoIZ15622883 = PyuQGnUajxqKdfoIZ53549213;     PyuQGnUajxqKdfoIZ53549213 = PyuQGnUajxqKdfoIZ20949640;     PyuQGnUajxqKdfoIZ20949640 = PyuQGnUajxqKdfoIZ72566826;     PyuQGnUajxqKdfoIZ72566826 = PyuQGnUajxqKdfoIZ67877386;     PyuQGnUajxqKdfoIZ67877386 = PyuQGnUajxqKdfoIZ45890425;     PyuQGnUajxqKdfoIZ45890425 = PyuQGnUajxqKdfoIZ51843599;     PyuQGnUajxqKdfoIZ51843599 = PyuQGnUajxqKdfoIZ45758582;     PyuQGnUajxqKdfoIZ45758582 = PyuQGnUajxqKdfoIZ50865884;     PyuQGnUajxqKdfoIZ50865884 = PyuQGnUajxqKdfoIZ79548492;     PyuQGnUajxqKdfoIZ79548492 = PyuQGnUajxqKdfoIZ64695984;     PyuQGnUajxqKdfoIZ64695984 = PyuQGnUajxqKdfoIZ64008188;     PyuQGnUajxqKdfoIZ64008188 = PyuQGnUajxqKdfoIZ34352709;     PyuQGnUajxqKdfoIZ34352709 = PyuQGnUajxqKdfoIZ66529654;     PyuQGnUajxqKdfoIZ66529654 = PyuQGnUajxqKdfoIZ4527863;     PyuQGnUajxqKdfoIZ4527863 = PyuQGnUajxqKdfoIZ50419593;     PyuQGnUajxqKdfoIZ50419593 = PyuQGnUajxqKdfoIZ76261553;     PyuQGnUajxqKdfoIZ76261553 = PyuQGnUajxqKdfoIZ93208809;     PyuQGnUajxqKdfoIZ93208809 = PyuQGnUajxqKdfoIZ81435189;     PyuQGnUajxqKdfoIZ81435189 = PyuQGnUajxqKdfoIZ95692151;     PyuQGnUajxqKdfoIZ95692151 = PyuQGnUajxqKdfoIZ88503665;     PyuQGnUajxqKdfoIZ88503665 = PyuQGnUajxqKdfoIZ22069241;     PyuQGnUajxqKdfoIZ22069241 = PyuQGnUajxqKdfoIZ5512718;     PyuQGnUajxqKdfoIZ5512718 = PyuQGnUajxqKdfoIZ35274663;     PyuQGnUajxqKdfoIZ35274663 = PyuQGnUajxqKdfoIZ5851613;     PyuQGnUajxqKdfoIZ5851613 = PyuQGnUajxqKdfoIZ86961490;     PyuQGnUajxqKdfoIZ86961490 = PyuQGnUajxqKdfoIZ56689213;     PyuQGnUajxqKdfoIZ56689213 = PyuQGnUajxqKdfoIZ8435193;     PyuQGnUajxqKdfoIZ8435193 = PyuQGnUajxqKdfoIZ90619590;     PyuQGnUajxqKdfoIZ90619590 = PyuQGnUajxqKdfoIZ2595924;     PyuQGnUajxqKdfoIZ2595924 = PyuQGnUajxqKdfoIZ76299034;     PyuQGnUajxqKdfoIZ76299034 = PyuQGnUajxqKdfoIZ74038461;     PyuQGnUajxqKdfoIZ74038461 = PyuQGnUajxqKdfoIZ72444411;     PyuQGnUajxqKdfoIZ72444411 = PyuQGnUajxqKdfoIZ91421085;     PyuQGnUajxqKdfoIZ91421085 = PyuQGnUajxqKdfoIZ39576906;     PyuQGnUajxqKdfoIZ39576906 = PyuQGnUajxqKdfoIZ60661285;     PyuQGnUajxqKdfoIZ60661285 = PyuQGnUajxqKdfoIZ70546475;     PyuQGnUajxqKdfoIZ70546475 = PyuQGnUajxqKdfoIZ59957043;     PyuQGnUajxqKdfoIZ59957043 = PyuQGnUajxqKdfoIZ26515764;     PyuQGnUajxqKdfoIZ26515764 = PyuQGnUajxqKdfoIZ97886371;     PyuQGnUajxqKdfoIZ97886371 = PyuQGnUajxqKdfoIZ18053576;     PyuQGnUajxqKdfoIZ18053576 = PyuQGnUajxqKdfoIZ4192293;     PyuQGnUajxqKdfoIZ4192293 = PyuQGnUajxqKdfoIZ75242735;     PyuQGnUajxqKdfoIZ75242735 = PyuQGnUajxqKdfoIZ75455661;     PyuQGnUajxqKdfoIZ75455661 = PyuQGnUajxqKdfoIZ78953682;     PyuQGnUajxqKdfoIZ78953682 = PyuQGnUajxqKdfoIZ82514041;     PyuQGnUajxqKdfoIZ82514041 = PyuQGnUajxqKdfoIZ38707541;     PyuQGnUajxqKdfoIZ38707541 = PyuQGnUajxqKdfoIZ77031909;     PyuQGnUajxqKdfoIZ77031909 = PyuQGnUajxqKdfoIZ29773522;     PyuQGnUajxqKdfoIZ29773522 = PyuQGnUajxqKdfoIZ87367301;     PyuQGnUajxqKdfoIZ87367301 = PyuQGnUajxqKdfoIZ17773029;     PyuQGnUajxqKdfoIZ17773029 = PyuQGnUajxqKdfoIZ25913750;     PyuQGnUajxqKdfoIZ25913750 = PyuQGnUajxqKdfoIZ27073548;     PyuQGnUajxqKdfoIZ27073548 = PyuQGnUajxqKdfoIZ87039761;     PyuQGnUajxqKdfoIZ87039761 = PyuQGnUajxqKdfoIZ48879399;     PyuQGnUajxqKdfoIZ48879399 = PyuQGnUajxqKdfoIZ93553641;     PyuQGnUajxqKdfoIZ93553641 = PyuQGnUajxqKdfoIZ48036496;     PyuQGnUajxqKdfoIZ48036496 = PyuQGnUajxqKdfoIZ85674977;     PyuQGnUajxqKdfoIZ85674977 = PyuQGnUajxqKdfoIZ66715214;     PyuQGnUajxqKdfoIZ66715214 = PyuQGnUajxqKdfoIZ80915895;     PyuQGnUajxqKdfoIZ80915895 = PyuQGnUajxqKdfoIZ89201212;     PyuQGnUajxqKdfoIZ89201212 = PyuQGnUajxqKdfoIZ43408406;     PyuQGnUajxqKdfoIZ43408406 = PyuQGnUajxqKdfoIZ55138991;     PyuQGnUajxqKdfoIZ55138991 = PyuQGnUajxqKdfoIZ48269961;     PyuQGnUajxqKdfoIZ48269961 = PyuQGnUajxqKdfoIZ3249459;     PyuQGnUajxqKdfoIZ3249459 = PyuQGnUajxqKdfoIZ90657522;     PyuQGnUajxqKdfoIZ90657522 = PyuQGnUajxqKdfoIZ91563776;     PyuQGnUajxqKdfoIZ91563776 = PyuQGnUajxqKdfoIZ42931623;     PyuQGnUajxqKdfoIZ42931623 = PyuQGnUajxqKdfoIZ26952748;     PyuQGnUajxqKdfoIZ26952748 = PyuQGnUajxqKdfoIZ43866578;     PyuQGnUajxqKdfoIZ43866578 = PyuQGnUajxqKdfoIZ79873118;     PyuQGnUajxqKdfoIZ79873118 = PyuQGnUajxqKdfoIZ16304510;     PyuQGnUajxqKdfoIZ16304510 = PyuQGnUajxqKdfoIZ66693046;     PyuQGnUajxqKdfoIZ66693046 = PyuQGnUajxqKdfoIZ83548818;     PyuQGnUajxqKdfoIZ83548818 = PyuQGnUajxqKdfoIZ77638576;     PyuQGnUajxqKdfoIZ77638576 = PyuQGnUajxqKdfoIZ84311372;     PyuQGnUajxqKdfoIZ84311372 = PyuQGnUajxqKdfoIZ46826505;     PyuQGnUajxqKdfoIZ46826505 = PyuQGnUajxqKdfoIZ30057056;     PyuQGnUajxqKdfoIZ30057056 = PyuQGnUajxqKdfoIZ56320980;     PyuQGnUajxqKdfoIZ56320980 = PyuQGnUajxqKdfoIZ23337572;     PyuQGnUajxqKdfoIZ23337572 = PyuQGnUajxqKdfoIZ48253949;     PyuQGnUajxqKdfoIZ48253949 = PyuQGnUajxqKdfoIZ79657303;     PyuQGnUajxqKdfoIZ79657303 = PyuQGnUajxqKdfoIZ78661671;     PyuQGnUajxqKdfoIZ78661671 = PyuQGnUajxqKdfoIZ94393149;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void uTmIarBpdydOWsHJ99815469() {     double TtNXnyGZHJGopbiuQ29852118 = -330501726;    double TtNXnyGZHJGopbiuQ57048875 = 52500236;    double TtNXnyGZHJGopbiuQ7283651 = -111428953;    double TtNXnyGZHJGopbiuQ22174229 = -304537022;    double TtNXnyGZHJGopbiuQ82393889 = -166147692;    double TtNXnyGZHJGopbiuQ91227025 = -794389338;    double TtNXnyGZHJGopbiuQ54307072 = -338469826;    double TtNXnyGZHJGopbiuQ8653791 = -505361464;    double TtNXnyGZHJGopbiuQ74577855 = -554744934;    double TtNXnyGZHJGopbiuQ58464275 = -739026875;    double TtNXnyGZHJGopbiuQ72483119 = -300409183;    double TtNXnyGZHJGopbiuQ38355043 = -768584026;    double TtNXnyGZHJGopbiuQ32269227 = -730915635;    double TtNXnyGZHJGopbiuQ72263422 = -436777098;    double TtNXnyGZHJGopbiuQ66703254 = -106724498;    double TtNXnyGZHJGopbiuQ66273213 = -941543373;    double TtNXnyGZHJGopbiuQ93218999 = -555716796;    double TtNXnyGZHJGopbiuQ15383074 = -72975156;    double TtNXnyGZHJGopbiuQ54099543 = -263028322;    double TtNXnyGZHJGopbiuQ14786859 = -875690289;    double TtNXnyGZHJGopbiuQ85653689 = -812298307;    double TtNXnyGZHJGopbiuQ71298911 = 95799722;    double TtNXnyGZHJGopbiuQ16415806 = -621478483;    double TtNXnyGZHJGopbiuQ45636544 = -859396858;    double TtNXnyGZHJGopbiuQ53776533 = -545213176;    double TtNXnyGZHJGopbiuQ61564056 = -585163771;    double TtNXnyGZHJGopbiuQ39969066 = -96035854;    double TtNXnyGZHJGopbiuQ5424409 = -655773675;    double TtNXnyGZHJGopbiuQ10676403 = -276320961;    double TtNXnyGZHJGopbiuQ39886120 = -711613207;    double TtNXnyGZHJGopbiuQ85112885 = 25173577;    double TtNXnyGZHJGopbiuQ42350881 = -413690006;    double TtNXnyGZHJGopbiuQ62637604 = 57387017;    double TtNXnyGZHJGopbiuQ6009980 = -352479738;    double TtNXnyGZHJGopbiuQ59017263 = -380346520;    double TtNXnyGZHJGopbiuQ32996034 = -516647576;    double TtNXnyGZHJGopbiuQ45103968 = -283190123;    double TtNXnyGZHJGopbiuQ39719069 = -502036885;    double TtNXnyGZHJGopbiuQ52459802 = -583125889;    double TtNXnyGZHJGopbiuQ69784249 = -460973251;    double TtNXnyGZHJGopbiuQ68726270 = -978974597;    double TtNXnyGZHJGopbiuQ95216739 = -250885612;    double TtNXnyGZHJGopbiuQ76344869 = -608162957;    double TtNXnyGZHJGopbiuQ96820551 = -250642925;    double TtNXnyGZHJGopbiuQ8451652 = -594486206;    double TtNXnyGZHJGopbiuQ36580919 = -215423374;    double TtNXnyGZHJGopbiuQ21941087 = -351058897;    double TtNXnyGZHJGopbiuQ94025345 = -778687806;    double TtNXnyGZHJGopbiuQ96931127 = -445345840;    double TtNXnyGZHJGopbiuQ96974000 = -854667435;    double TtNXnyGZHJGopbiuQ72191809 = -126003531;    double TtNXnyGZHJGopbiuQ23062379 = -857391973;    double TtNXnyGZHJGopbiuQ29282434 = -178787211;    double TtNXnyGZHJGopbiuQ86538321 = -866148936;    double TtNXnyGZHJGopbiuQ14161780 = -326188144;    double TtNXnyGZHJGopbiuQ58553207 = -326301449;    double TtNXnyGZHJGopbiuQ40633070 = -326021282;    double TtNXnyGZHJGopbiuQ61647107 = -252032096;    double TtNXnyGZHJGopbiuQ68397696 = -759323847;    double TtNXnyGZHJGopbiuQ20829833 = -580983921;    double TtNXnyGZHJGopbiuQ51257959 = -598353485;    double TtNXnyGZHJGopbiuQ48882664 = -682696151;    double TtNXnyGZHJGopbiuQ97977387 = -129040503;    double TtNXnyGZHJGopbiuQ34691736 = -843131728;    double TtNXnyGZHJGopbiuQ73351390 = -664200453;    double TtNXnyGZHJGopbiuQ30132238 = -886719178;    double TtNXnyGZHJGopbiuQ75717439 = -725971043;    double TtNXnyGZHJGopbiuQ26259247 = -278435898;    double TtNXnyGZHJGopbiuQ13246160 = 43569421;    double TtNXnyGZHJGopbiuQ33707221 = -590076923;    double TtNXnyGZHJGopbiuQ21169246 = -558353251;    double TtNXnyGZHJGopbiuQ53499931 = 46320089;    double TtNXnyGZHJGopbiuQ62923272 = -489849268;    double TtNXnyGZHJGopbiuQ84315293 = -802055072;    double TtNXnyGZHJGopbiuQ46060588 = -896715693;    double TtNXnyGZHJGopbiuQ90436950 = -461412695;    double TtNXnyGZHJGopbiuQ94954042 = -296037321;    double TtNXnyGZHJGopbiuQ19595254 = -270835559;    double TtNXnyGZHJGopbiuQ37184892 = -164910652;    double TtNXnyGZHJGopbiuQ17195614 = -229789802;    double TtNXnyGZHJGopbiuQ39622969 = -134104875;    double TtNXnyGZHJGopbiuQ45943720 = -317348048;    double TtNXnyGZHJGopbiuQ8493282 = -110427835;    double TtNXnyGZHJGopbiuQ13702402 = -421653527;    double TtNXnyGZHJGopbiuQ67694310 = -485609676;    double TtNXnyGZHJGopbiuQ62050507 = -117434450;    double TtNXnyGZHJGopbiuQ13068448 = -134902795;    double TtNXnyGZHJGopbiuQ76099282 = -76464048;    double TtNXnyGZHJGopbiuQ91848200 = 73708406;    double TtNXnyGZHJGopbiuQ464056 = 45954928;    double TtNXnyGZHJGopbiuQ92362963 = -90626295;    double TtNXnyGZHJGopbiuQ83456861 = 68841973;    double TtNXnyGZHJGopbiuQ71321372 = -742713038;    double TtNXnyGZHJGopbiuQ31629970 = 97858032;    double TtNXnyGZHJGopbiuQ18526291 = -862619767;    double TtNXnyGZHJGopbiuQ19843607 = -196278446;    double TtNXnyGZHJGopbiuQ97239351 = -21845109;    double TtNXnyGZHJGopbiuQ41653133 = -765031229;    double TtNXnyGZHJGopbiuQ23469162 = -586442472;    double TtNXnyGZHJGopbiuQ78319413 = -330501726;     TtNXnyGZHJGopbiuQ29852118 = TtNXnyGZHJGopbiuQ57048875;     TtNXnyGZHJGopbiuQ57048875 = TtNXnyGZHJGopbiuQ7283651;     TtNXnyGZHJGopbiuQ7283651 = TtNXnyGZHJGopbiuQ22174229;     TtNXnyGZHJGopbiuQ22174229 = TtNXnyGZHJGopbiuQ82393889;     TtNXnyGZHJGopbiuQ82393889 = TtNXnyGZHJGopbiuQ91227025;     TtNXnyGZHJGopbiuQ91227025 = TtNXnyGZHJGopbiuQ54307072;     TtNXnyGZHJGopbiuQ54307072 = TtNXnyGZHJGopbiuQ8653791;     TtNXnyGZHJGopbiuQ8653791 = TtNXnyGZHJGopbiuQ74577855;     TtNXnyGZHJGopbiuQ74577855 = TtNXnyGZHJGopbiuQ58464275;     TtNXnyGZHJGopbiuQ58464275 = TtNXnyGZHJGopbiuQ72483119;     TtNXnyGZHJGopbiuQ72483119 = TtNXnyGZHJGopbiuQ38355043;     TtNXnyGZHJGopbiuQ38355043 = TtNXnyGZHJGopbiuQ32269227;     TtNXnyGZHJGopbiuQ32269227 = TtNXnyGZHJGopbiuQ72263422;     TtNXnyGZHJGopbiuQ72263422 = TtNXnyGZHJGopbiuQ66703254;     TtNXnyGZHJGopbiuQ66703254 = TtNXnyGZHJGopbiuQ66273213;     TtNXnyGZHJGopbiuQ66273213 = TtNXnyGZHJGopbiuQ93218999;     TtNXnyGZHJGopbiuQ93218999 = TtNXnyGZHJGopbiuQ15383074;     TtNXnyGZHJGopbiuQ15383074 = TtNXnyGZHJGopbiuQ54099543;     TtNXnyGZHJGopbiuQ54099543 = TtNXnyGZHJGopbiuQ14786859;     TtNXnyGZHJGopbiuQ14786859 = TtNXnyGZHJGopbiuQ85653689;     TtNXnyGZHJGopbiuQ85653689 = TtNXnyGZHJGopbiuQ71298911;     TtNXnyGZHJGopbiuQ71298911 = TtNXnyGZHJGopbiuQ16415806;     TtNXnyGZHJGopbiuQ16415806 = TtNXnyGZHJGopbiuQ45636544;     TtNXnyGZHJGopbiuQ45636544 = TtNXnyGZHJGopbiuQ53776533;     TtNXnyGZHJGopbiuQ53776533 = TtNXnyGZHJGopbiuQ61564056;     TtNXnyGZHJGopbiuQ61564056 = TtNXnyGZHJGopbiuQ39969066;     TtNXnyGZHJGopbiuQ39969066 = TtNXnyGZHJGopbiuQ5424409;     TtNXnyGZHJGopbiuQ5424409 = TtNXnyGZHJGopbiuQ10676403;     TtNXnyGZHJGopbiuQ10676403 = TtNXnyGZHJGopbiuQ39886120;     TtNXnyGZHJGopbiuQ39886120 = TtNXnyGZHJGopbiuQ85112885;     TtNXnyGZHJGopbiuQ85112885 = TtNXnyGZHJGopbiuQ42350881;     TtNXnyGZHJGopbiuQ42350881 = TtNXnyGZHJGopbiuQ62637604;     TtNXnyGZHJGopbiuQ62637604 = TtNXnyGZHJGopbiuQ6009980;     TtNXnyGZHJGopbiuQ6009980 = TtNXnyGZHJGopbiuQ59017263;     TtNXnyGZHJGopbiuQ59017263 = TtNXnyGZHJGopbiuQ32996034;     TtNXnyGZHJGopbiuQ32996034 = TtNXnyGZHJGopbiuQ45103968;     TtNXnyGZHJGopbiuQ45103968 = TtNXnyGZHJGopbiuQ39719069;     TtNXnyGZHJGopbiuQ39719069 = TtNXnyGZHJGopbiuQ52459802;     TtNXnyGZHJGopbiuQ52459802 = TtNXnyGZHJGopbiuQ69784249;     TtNXnyGZHJGopbiuQ69784249 = TtNXnyGZHJGopbiuQ68726270;     TtNXnyGZHJGopbiuQ68726270 = TtNXnyGZHJGopbiuQ95216739;     TtNXnyGZHJGopbiuQ95216739 = TtNXnyGZHJGopbiuQ76344869;     TtNXnyGZHJGopbiuQ76344869 = TtNXnyGZHJGopbiuQ96820551;     TtNXnyGZHJGopbiuQ96820551 = TtNXnyGZHJGopbiuQ8451652;     TtNXnyGZHJGopbiuQ8451652 = TtNXnyGZHJGopbiuQ36580919;     TtNXnyGZHJGopbiuQ36580919 = TtNXnyGZHJGopbiuQ21941087;     TtNXnyGZHJGopbiuQ21941087 = TtNXnyGZHJGopbiuQ94025345;     TtNXnyGZHJGopbiuQ94025345 = TtNXnyGZHJGopbiuQ96931127;     TtNXnyGZHJGopbiuQ96931127 = TtNXnyGZHJGopbiuQ96974000;     TtNXnyGZHJGopbiuQ96974000 = TtNXnyGZHJGopbiuQ72191809;     TtNXnyGZHJGopbiuQ72191809 = TtNXnyGZHJGopbiuQ23062379;     TtNXnyGZHJGopbiuQ23062379 = TtNXnyGZHJGopbiuQ29282434;     TtNXnyGZHJGopbiuQ29282434 = TtNXnyGZHJGopbiuQ86538321;     TtNXnyGZHJGopbiuQ86538321 = TtNXnyGZHJGopbiuQ14161780;     TtNXnyGZHJGopbiuQ14161780 = TtNXnyGZHJGopbiuQ58553207;     TtNXnyGZHJGopbiuQ58553207 = TtNXnyGZHJGopbiuQ40633070;     TtNXnyGZHJGopbiuQ40633070 = TtNXnyGZHJGopbiuQ61647107;     TtNXnyGZHJGopbiuQ61647107 = TtNXnyGZHJGopbiuQ68397696;     TtNXnyGZHJGopbiuQ68397696 = TtNXnyGZHJGopbiuQ20829833;     TtNXnyGZHJGopbiuQ20829833 = TtNXnyGZHJGopbiuQ51257959;     TtNXnyGZHJGopbiuQ51257959 = TtNXnyGZHJGopbiuQ48882664;     TtNXnyGZHJGopbiuQ48882664 = TtNXnyGZHJGopbiuQ97977387;     TtNXnyGZHJGopbiuQ97977387 = TtNXnyGZHJGopbiuQ34691736;     TtNXnyGZHJGopbiuQ34691736 = TtNXnyGZHJGopbiuQ73351390;     TtNXnyGZHJGopbiuQ73351390 = TtNXnyGZHJGopbiuQ30132238;     TtNXnyGZHJGopbiuQ30132238 = TtNXnyGZHJGopbiuQ75717439;     TtNXnyGZHJGopbiuQ75717439 = TtNXnyGZHJGopbiuQ26259247;     TtNXnyGZHJGopbiuQ26259247 = TtNXnyGZHJGopbiuQ13246160;     TtNXnyGZHJGopbiuQ13246160 = TtNXnyGZHJGopbiuQ33707221;     TtNXnyGZHJGopbiuQ33707221 = TtNXnyGZHJGopbiuQ21169246;     TtNXnyGZHJGopbiuQ21169246 = TtNXnyGZHJGopbiuQ53499931;     TtNXnyGZHJGopbiuQ53499931 = TtNXnyGZHJGopbiuQ62923272;     TtNXnyGZHJGopbiuQ62923272 = TtNXnyGZHJGopbiuQ84315293;     TtNXnyGZHJGopbiuQ84315293 = TtNXnyGZHJGopbiuQ46060588;     TtNXnyGZHJGopbiuQ46060588 = TtNXnyGZHJGopbiuQ90436950;     TtNXnyGZHJGopbiuQ90436950 = TtNXnyGZHJGopbiuQ94954042;     TtNXnyGZHJGopbiuQ94954042 = TtNXnyGZHJGopbiuQ19595254;     TtNXnyGZHJGopbiuQ19595254 = TtNXnyGZHJGopbiuQ37184892;     TtNXnyGZHJGopbiuQ37184892 = TtNXnyGZHJGopbiuQ17195614;     TtNXnyGZHJGopbiuQ17195614 = TtNXnyGZHJGopbiuQ39622969;     TtNXnyGZHJGopbiuQ39622969 = TtNXnyGZHJGopbiuQ45943720;     TtNXnyGZHJGopbiuQ45943720 = TtNXnyGZHJGopbiuQ8493282;     TtNXnyGZHJGopbiuQ8493282 = TtNXnyGZHJGopbiuQ13702402;     TtNXnyGZHJGopbiuQ13702402 = TtNXnyGZHJGopbiuQ67694310;     TtNXnyGZHJGopbiuQ67694310 = TtNXnyGZHJGopbiuQ62050507;     TtNXnyGZHJGopbiuQ62050507 = TtNXnyGZHJGopbiuQ13068448;     TtNXnyGZHJGopbiuQ13068448 = TtNXnyGZHJGopbiuQ76099282;     TtNXnyGZHJGopbiuQ76099282 = TtNXnyGZHJGopbiuQ91848200;     TtNXnyGZHJGopbiuQ91848200 = TtNXnyGZHJGopbiuQ464056;     TtNXnyGZHJGopbiuQ464056 = TtNXnyGZHJGopbiuQ92362963;     TtNXnyGZHJGopbiuQ92362963 = TtNXnyGZHJGopbiuQ83456861;     TtNXnyGZHJGopbiuQ83456861 = TtNXnyGZHJGopbiuQ71321372;     TtNXnyGZHJGopbiuQ71321372 = TtNXnyGZHJGopbiuQ31629970;     TtNXnyGZHJGopbiuQ31629970 = TtNXnyGZHJGopbiuQ18526291;     TtNXnyGZHJGopbiuQ18526291 = TtNXnyGZHJGopbiuQ19843607;     TtNXnyGZHJGopbiuQ19843607 = TtNXnyGZHJGopbiuQ97239351;     TtNXnyGZHJGopbiuQ97239351 = TtNXnyGZHJGopbiuQ41653133;     TtNXnyGZHJGopbiuQ41653133 = TtNXnyGZHJGopbiuQ23469162;     TtNXnyGZHJGopbiuQ23469162 = TtNXnyGZHJGopbiuQ78319413;     TtNXnyGZHJGopbiuQ78319413 = TtNXnyGZHJGopbiuQ29852118;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IiZpVhJTqbeBbWdT36460444() {     double WXbYpNHDhgaJsQPvA52239235 = 35094311;    double WXbYpNHDhgaJsQPvA41396375 = -996042819;    double WXbYpNHDhgaJsQPvA36052930 = -351082797;    double WXbYpNHDhgaJsQPvA37180918 = -310944554;    double WXbYpNHDhgaJsQPvA80568143 = -757204693;    double WXbYpNHDhgaJsQPvA6110143 = -654997716;    double WXbYpNHDhgaJsQPvA46462811 = -160457132;    double WXbYpNHDhgaJsQPvA84287649 = -975958108;    double WXbYpNHDhgaJsQPvA71740986 = -438940973;    double WXbYpNHDhgaJsQPvA50227860 = -481768208;    double WXbYpNHDhgaJsQPvA10024979 = -297033584;    double WXbYpNHDhgaJsQPvA94257992 = 80670681;    double WXbYpNHDhgaJsQPvA99277407 = -249632955;    double WXbYpNHDhgaJsQPvA14878175 = -964485855;    double WXbYpNHDhgaJsQPvA3438532 = -834202211;    double WXbYpNHDhgaJsQPvA10529126 = -223210916;    double WXbYpNHDhgaJsQPvA43042351 = -832996616;    double WXbYpNHDhgaJsQPvA21359491 = -524717847;    double WXbYpNHDhgaJsQPvA95381174 = -398130448;    double WXbYpNHDhgaJsQPvA93303141 = -28324743;    double WXbYpNHDhgaJsQPvA86536796 = -803785615;    double WXbYpNHDhgaJsQPvA35114129 = -408966591;    double WXbYpNHDhgaJsQPvA19709129 = -797559075;    double WXbYpNHDhgaJsQPvA72136331 = -323389715;    double WXbYpNHDhgaJsQPvA89316779 = -166772300;    double WXbYpNHDhgaJsQPvA49867051 = 70942005;    double WXbYpNHDhgaJsQPvA87058486 = -953622347;    double WXbYpNHDhgaJsQPvA11335859 = -444228128;    double WXbYpNHDhgaJsQPvA17568964 = -620578080;    double WXbYpNHDhgaJsQPvA49043299 = -631909073;    double WXbYpNHDhgaJsQPvA29403691 = -56142575;    double WXbYpNHDhgaJsQPvA99892072 = 34238640;    double WXbYpNHDhgaJsQPvA43676274 = -471956478;    double WXbYpNHDhgaJsQPvA65398667 = -676606588;    double WXbYpNHDhgaJsQPvA35246628 = -444102943;    double WXbYpNHDhgaJsQPvA74688902 = -832042953;    double WXbYpNHDhgaJsQPvA38543680 = -16138075;    double WXbYpNHDhgaJsQPvA93345169 = -369662357;    double WXbYpNHDhgaJsQPvA767934 = -984171920;    double WXbYpNHDhgaJsQPvA89969964 = -597197280;    double WXbYpNHDhgaJsQPvA97483641 = -382152263;    double WXbYpNHDhgaJsQPvA83725131 = -8794303;    double WXbYpNHDhgaJsQPvA81895061 = -495693540;    double WXbYpNHDhgaJsQPvA96264670 = -248006574;    double WXbYpNHDhgaJsQPvA76434918 = -403629684;    double WXbYpNHDhgaJsQPvA44681201 = -397652469;    double WXbYpNHDhgaJsQPvA34238484 = -929469150;    double WXbYpNHDhgaJsQPvA38624020 = -535222797;    double WXbYpNHDhgaJsQPvA91391032 = -395062538;    double WXbYpNHDhgaJsQPvA94057746 = -996687694;    double WXbYpNHDhgaJsQPvA81061026 = -481079808;    double WXbYpNHDhgaJsQPvA96913635 = -188211255;    double WXbYpNHDhgaJsQPvA7596837 = -950698442;    double WXbYpNHDhgaJsQPvA46824867 = -41829745;    double WXbYpNHDhgaJsQPvA49923647 = -466892260;    double WXbYpNHDhgaJsQPvA17125107 = -555939099;    double WXbYpNHDhgaJsQPvA21687247 = -98483744;    double WXbYpNHDhgaJsQPvA63916598 = 72306917;    double WXbYpNHDhgaJsQPvA47864138 = -44172254;    double WXbYpNHDhgaJsQPvA30701093 = -728146698;    double WXbYpNHDhgaJsQPvA19051656 = -701375370;    double WXbYpNHDhgaJsQPvA35126952 = -716229004;    double WXbYpNHDhgaJsQPvA66718685 = -255380028;    double WXbYpNHDhgaJsQPvA22697687 = -807031901;    double WXbYpNHDhgaJsQPvA20824170 = -325625634;    double WXbYpNHDhgaJsQPvA10132907 = -231272224;    double WXbYpNHDhgaJsQPvA50581718 = -447372841;    double WXbYpNHDhgaJsQPvA33878741 = -573026367;    double WXbYpNHDhgaJsQPvA79631547 = -420382913;    double WXbYpNHDhgaJsQPvA28749629 = 97840741;    double WXbYpNHDhgaJsQPvA71985445 = -107072842;    double WXbYpNHDhgaJsQPvA49697182 = -363334259;    double WXbYpNHDhgaJsQPvA20591557 = -540545927;    double WXbYpNHDhgaJsQPvA5411211 = -800933168;    double WXbYpNHDhgaJsQPvA95819500 = -646172480;    double WXbYpNHDhgaJsQPvA2811666 = -694991313;    double WXbYpNHDhgaJsQPvA53219068 = -913273051;    double WXbYpNHDhgaJsQPvA23444458 = -449552502;    double WXbYpNHDhgaJsQPvA95701412 = -919760032;    double WXbYpNHDhgaJsQPvA44635579 = -769119831;    double WXbYpNHDhgaJsQPvA15628567 = 411154;    double WXbYpNHDhgaJsQPvA48434466 = -318399550;    double WXbYpNHDhgaJsQPvA19944827 = 50834409;    double WXbYpNHDhgaJsQPvA23511217 = -623890387;    double WXbYpNHDhgaJsQPvA67982273 = -50829266;    double WXbYpNHDhgaJsQPvA32490055 = -867931320;    double WXbYpNHDhgaJsQPvA92295235 = -15062919;    double WXbYpNHDhgaJsQPvA96851407 = -330126734;    double WXbYpNHDhgaJsQPvA15475020 = -109714329;    double WXbYpNHDhgaJsQPvA18121522 = -888163844;    double WXbYpNHDhgaJsQPvA53001656 = -633559209;    double WXbYpNHDhgaJsQPvA74627081 = 11555008;    double WXbYpNHDhgaJsQPvA45481032 = -225490103;    double WXbYpNHDhgaJsQPvA70066841 = -156025222;    double WXbYpNHDhgaJsQPvA70918308 = -895821911;    double WXbYpNHDhgaJsQPvA62356689 = -665923260;    double WXbYpNHDhgaJsQPvA17006446 = -753414275;    double WXbYpNHDhgaJsQPvA59197374 = -688661640;    double WXbYpNHDhgaJsQPvA75440501 = -922380940;    double WXbYpNHDhgaJsQPvA66302011 = 35094311;     WXbYpNHDhgaJsQPvA52239235 = WXbYpNHDhgaJsQPvA41396375;     WXbYpNHDhgaJsQPvA41396375 = WXbYpNHDhgaJsQPvA36052930;     WXbYpNHDhgaJsQPvA36052930 = WXbYpNHDhgaJsQPvA37180918;     WXbYpNHDhgaJsQPvA37180918 = WXbYpNHDhgaJsQPvA80568143;     WXbYpNHDhgaJsQPvA80568143 = WXbYpNHDhgaJsQPvA6110143;     WXbYpNHDhgaJsQPvA6110143 = WXbYpNHDhgaJsQPvA46462811;     WXbYpNHDhgaJsQPvA46462811 = WXbYpNHDhgaJsQPvA84287649;     WXbYpNHDhgaJsQPvA84287649 = WXbYpNHDhgaJsQPvA71740986;     WXbYpNHDhgaJsQPvA71740986 = WXbYpNHDhgaJsQPvA50227860;     WXbYpNHDhgaJsQPvA50227860 = WXbYpNHDhgaJsQPvA10024979;     WXbYpNHDhgaJsQPvA10024979 = WXbYpNHDhgaJsQPvA94257992;     WXbYpNHDhgaJsQPvA94257992 = WXbYpNHDhgaJsQPvA99277407;     WXbYpNHDhgaJsQPvA99277407 = WXbYpNHDhgaJsQPvA14878175;     WXbYpNHDhgaJsQPvA14878175 = WXbYpNHDhgaJsQPvA3438532;     WXbYpNHDhgaJsQPvA3438532 = WXbYpNHDhgaJsQPvA10529126;     WXbYpNHDhgaJsQPvA10529126 = WXbYpNHDhgaJsQPvA43042351;     WXbYpNHDhgaJsQPvA43042351 = WXbYpNHDhgaJsQPvA21359491;     WXbYpNHDhgaJsQPvA21359491 = WXbYpNHDhgaJsQPvA95381174;     WXbYpNHDhgaJsQPvA95381174 = WXbYpNHDhgaJsQPvA93303141;     WXbYpNHDhgaJsQPvA93303141 = WXbYpNHDhgaJsQPvA86536796;     WXbYpNHDhgaJsQPvA86536796 = WXbYpNHDhgaJsQPvA35114129;     WXbYpNHDhgaJsQPvA35114129 = WXbYpNHDhgaJsQPvA19709129;     WXbYpNHDhgaJsQPvA19709129 = WXbYpNHDhgaJsQPvA72136331;     WXbYpNHDhgaJsQPvA72136331 = WXbYpNHDhgaJsQPvA89316779;     WXbYpNHDhgaJsQPvA89316779 = WXbYpNHDhgaJsQPvA49867051;     WXbYpNHDhgaJsQPvA49867051 = WXbYpNHDhgaJsQPvA87058486;     WXbYpNHDhgaJsQPvA87058486 = WXbYpNHDhgaJsQPvA11335859;     WXbYpNHDhgaJsQPvA11335859 = WXbYpNHDhgaJsQPvA17568964;     WXbYpNHDhgaJsQPvA17568964 = WXbYpNHDhgaJsQPvA49043299;     WXbYpNHDhgaJsQPvA49043299 = WXbYpNHDhgaJsQPvA29403691;     WXbYpNHDhgaJsQPvA29403691 = WXbYpNHDhgaJsQPvA99892072;     WXbYpNHDhgaJsQPvA99892072 = WXbYpNHDhgaJsQPvA43676274;     WXbYpNHDhgaJsQPvA43676274 = WXbYpNHDhgaJsQPvA65398667;     WXbYpNHDhgaJsQPvA65398667 = WXbYpNHDhgaJsQPvA35246628;     WXbYpNHDhgaJsQPvA35246628 = WXbYpNHDhgaJsQPvA74688902;     WXbYpNHDhgaJsQPvA74688902 = WXbYpNHDhgaJsQPvA38543680;     WXbYpNHDhgaJsQPvA38543680 = WXbYpNHDhgaJsQPvA93345169;     WXbYpNHDhgaJsQPvA93345169 = WXbYpNHDhgaJsQPvA767934;     WXbYpNHDhgaJsQPvA767934 = WXbYpNHDhgaJsQPvA89969964;     WXbYpNHDhgaJsQPvA89969964 = WXbYpNHDhgaJsQPvA97483641;     WXbYpNHDhgaJsQPvA97483641 = WXbYpNHDhgaJsQPvA83725131;     WXbYpNHDhgaJsQPvA83725131 = WXbYpNHDhgaJsQPvA81895061;     WXbYpNHDhgaJsQPvA81895061 = WXbYpNHDhgaJsQPvA96264670;     WXbYpNHDhgaJsQPvA96264670 = WXbYpNHDhgaJsQPvA76434918;     WXbYpNHDhgaJsQPvA76434918 = WXbYpNHDhgaJsQPvA44681201;     WXbYpNHDhgaJsQPvA44681201 = WXbYpNHDhgaJsQPvA34238484;     WXbYpNHDhgaJsQPvA34238484 = WXbYpNHDhgaJsQPvA38624020;     WXbYpNHDhgaJsQPvA38624020 = WXbYpNHDhgaJsQPvA91391032;     WXbYpNHDhgaJsQPvA91391032 = WXbYpNHDhgaJsQPvA94057746;     WXbYpNHDhgaJsQPvA94057746 = WXbYpNHDhgaJsQPvA81061026;     WXbYpNHDhgaJsQPvA81061026 = WXbYpNHDhgaJsQPvA96913635;     WXbYpNHDhgaJsQPvA96913635 = WXbYpNHDhgaJsQPvA7596837;     WXbYpNHDhgaJsQPvA7596837 = WXbYpNHDhgaJsQPvA46824867;     WXbYpNHDhgaJsQPvA46824867 = WXbYpNHDhgaJsQPvA49923647;     WXbYpNHDhgaJsQPvA49923647 = WXbYpNHDhgaJsQPvA17125107;     WXbYpNHDhgaJsQPvA17125107 = WXbYpNHDhgaJsQPvA21687247;     WXbYpNHDhgaJsQPvA21687247 = WXbYpNHDhgaJsQPvA63916598;     WXbYpNHDhgaJsQPvA63916598 = WXbYpNHDhgaJsQPvA47864138;     WXbYpNHDhgaJsQPvA47864138 = WXbYpNHDhgaJsQPvA30701093;     WXbYpNHDhgaJsQPvA30701093 = WXbYpNHDhgaJsQPvA19051656;     WXbYpNHDhgaJsQPvA19051656 = WXbYpNHDhgaJsQPvA35126952;     WXbYpNHDhgaJsQPvA35126952 = WXbYpNHDhgaJsQPvA66718685;     WXbYpNHDhgaJsQPvA66718685 = WXbYpNHDhgaJsQPvA22697687;     WXbYpNHDhgaJsQPvA22697687 = WXbYpNHDhgaJsQPvA20824170;     WXbYpNHDhgaJsQPvA20824170 = WXbYpNHDhgaJsQPvA10132907;     WXbYpNHDhgaJsQPvA10132907 = WXbYpNHDhgaJsQPvA50581718;     WXbYpNHDhgaJsQPvA50581718 = WXbYpNHDhgaJsQPvA33878741;     WXbYpNHDhgaJsQPvA33878741 = WXbYpNHDhgaJsQPvA79631547;     WXbYpNHDhgaJsQPvA79631547 = WXbYpNHDhgaJsQPvA28749629;     WXbYpNHDhgaJsQPvA28749629 = WXbYpNHDhgaJsQPvA71985445;     WXbYpNHDhgaJsQPvA71985445 = WXbYpNHDhgaJsQPvA49697182;     WXbYpNHDhgaJsQPvA49697182 = WXbYpNHDhgaJsQPvA20591557;     WXbYpNHDhgaJsQPvA20591557 = WXbYpNHDhgaJsQPvA5411211;     WXbYpNHDhgaJsQPvA5411211 = WXbYpNHDhgaJsQPvA95819500;     WXbYpNHDhgaJsQPvA95819500 = WXbYpNHDhgaJsQPvA2811666;     WXbYpNHDhgaJsQPvA2811666 = WXbYpNHDhgaJsQPvA53219068;     WXbYpNHDhgaJsQPvA53219068 = WXbYpNHDhgaJsQPvA23444458;     WXbYpNHDhgaJsQPvA23444458 = WXbYpNHDhgaJsQPvA95701412;     WXbYpNHDhgaJsQPvA95701412 = WXbYpNHDhgaJsQPvA44635579;     WXbYpNHDhgaJsQPvA44635579 = WXbYpNHDhgaJsQPvA15628567;     WXbYpNHDhgaJsQPvA15628567 = WXbYpNHDhgaJsQPvA48434466;     WXbYpNHDhgaJsQPvA48434466 = WXbYpNHDhgaJsQPvA19944827;     WXbYpNHDhgaJsQPvA19944827 = WXbYpNHDhgaJsQPvA23511217;     WXbYpNHDhgaJsQPvA23511217 = WXbYpNHDhgaJsQPvA67982273;     WXbYpNHDhgaJsQPvA67982273 = WXbYpNHDhgaJsQPvA32490055;     WXbYpNHDhgaJsQPvA32490055 = WXbYpNHDhgaJsQPvA92295235;     WXbYpNHDhgaJsQPvA92295235 = WXbYpNHDhgaJsQPvA96851407;     WXbYpNHDhgaJsQPvA96851407 = WXbYpNHDhgaJsQPvA15475020;     WXbYpNHDhgaJsQPvA15475020 = WXbYpNHDhgaJsQPvA18121522;     WXbYpNHDhgaJsQPvA18121522 = WXbYpNHDhgaJsQPvA53001656;     WXbYpNHDhgaJsQPvA53001656 = WXbYpNHDhgaJsQPvA74627081;     WXbYpNHDhgaJsQPvA74627081 = WXbYpNHDhgaJsQPvA45481032;     WXbYpNHDhgaJsQPvA45481032 = WXbYpNHDhgaJsQPvA70066841;     WXbYpNHDhgaJsQPvA70066841 = WXbYpNHDhgaJsQPvA70918308;     WXbYpNHDhgaJsQPvA70918308 = WXbYpNHDhgaJsQPvA62356689;     WXbYpNHDhgaJsQPvA62356689 = WXbYpNHDhgaJsQPvA17006446;     WXbYpNHDhgaJsQPvA17006446 = WXbYpNHDhgaJsQPvA59197374;     WXbYpNHDhgaJsQPvA59197374 = WXbYpNHDhgaJsQPvA75440501;     WXbYpNHDhgaJsQPvA75440501 = WXbYpNHDhgaJsQPvA66302011;     WXbYpNHDhgaJsQPvA66302011 = WXbYpNHDhgaJsQPvA52239235;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void UYhcicUHSLALnbIz51509512() {     double IoSfdhkNyTZOdKEtW58356342 = -76807578;    double IoSfdhkNyTZOdKEtW84769654 = -705642600;    double IoSfdhkNyTZOdKEtW52095113 = -501915718;    double IoSfdhkNyTZOdKEtW64255698 = -501693184;    double IoSfdhkNyTZOdKEtW49463632 = -177854564;    double IoSfdhkNyTZOdKEtW97527344 = -919493951;    double IoSfdhkNyTZOdKEtW6951778 = -299422109;    double IoSfdhkNyTZOdKEtW34047540 = -162832021;    double IoSfdhkNyTZOdKEtW36383482 = -206973639;    double IoSfdhkNyTZOdKEtW91457519 = -810743686;    double IoSfdhkNyTZOdKEtW70135269 = -174471170;    double IoSfdhkNyTZOdKEtW26081035 = -363735521;    double IoSfdhkNyTZOdKEtW85486525 = -689524043;    double IoSfdhkNyTZOdKEtW98021851 = -396724423;    double IoSfdhkNyTZOdKEtW99180957 = -424664378;    double IoSfdhkNyTZOdKEtW52034520 = -196210459;    double IoSfdhkNyTZOdKEtW36878355 = -349541728;    double IoSfdhkNyTZOdKEtW18412644 = -190240489;    double IoSfdhkNyTZOdKEtW57211016 = -803899544;    double IoSfdhkNyTZOdKEtW73853306 = -385227662;    double IoSfdhkNyTZOdKEtW75089990 = -659273071;    double IoSfdhkNyTZOdKEtW21635153 = -316896307;    double IoSfdhkNyTZOdKEtW55299920 = -544076562;    double IoSfdhkNyTZOdKEtW56128705 = -545694703;    double IoSfdhkNyTZOdKEtW33016430 = -539968333;    double IoSfdhkNyTZOdKEtW74588726 = -907713318;    double IoSfdhkNyTZOdKEtW85518264 = -854284550;    double IoSfdhkNyTZOdKEtW24442985 = -510157082;    double IoSfdhkNyTZOdKEtW83701219 = -842960785;    double IoSfdhkNyTZOdKEtW17658936 = -449379941;    double IoSfdhkNyTZOdKEtW21646240 = 30536487;    double IoSfdhkNyTZOdKEtW97502693 = -775208176;    double IoSfdhkNyTZOdKEtW53387223 = -364569849;    double IoSfdhkNyTZOdKEtW60229231 = -428148353;    double IoSfdhkNyTZOdKEtW96721279 = 17163952;    double IoSfdhkNyTZOdKEtW43414816 = 53930940;    double IoSfdhkNyTZOdKEtW19805615 = 30226056;    double IoSfdhkNyTZOdKEtW27435382 = -141168366;    double IoSfdhkNyTZOdKEtW74378187 = -978095417;    double IoSfdhkNyTZOdKEtW77471902 = 29363497;    double IoSfdhkNyTZOdKEtW3152866 = -447426157;    double IoSfdhkNyTZOdKEtW15875588 = -278936581;    double IoSfdhkNyTZOdKEtW59962039 = -576498124;    double IoSfdhkNyTZOdKEtW83672411 = -267349512;    double IoSfdhkNyTZOdKEtW88387383 = -65527838;    double IoSfdhkNyTZOdKEtW56620396 = -382868586;    double IoSfdhkNyTZOdKEtW52017058 = 51755313;    double IoSfdhkNyTZOdKEtW46580961 = -545118329;    double IoSfdhkNyTZOdKEtW1066762 = -480799526;    double IoSfdhkNyTZOdKEtW6969399 = 16301498;    double IoSfdhkNyTZOdKEtW16761705 = -323373479;    double IoSfdhkNyTZOdKEtW7723356 = 11062424;    double IoSfdhkNyTZOdKEtW55845294 = -742488029;    double IoSfdhkNyTZOdKEtW42793747 = -362299890;    double IoSfdhkNyTZOdKEtW76419521 = -67424042;    double IoSfdhkNyTZOdKEtW36721189 = -759911271;    double IoSfdhkNyTZOdKEtW29469735 = -61566038;    double IoSfdhkNyTZOdKEtW95966408 = -956221016;    double IoSfdhkNyTZOdKEtW31239269 = -961724852;    double IoSfdhkNyTZOdKEtW74874906 = -270141246;    double IoSfdhkNyTZOdKEtW12009080 = 34790598;    double IoSfdhkNyTZOdKEtW82508793 = -789265027;    double IoSfdhkNyTZOdKEtW50346321 = -319871237;    double IoSfdhkNyTZOdKEtW18724546 = -757593699;    double IoSfdhkNyTZOdKEtW69811279 = -741280174;    double IoSfdhkNyTZOdKEtW72632575 = -399262995;    double IoSfdhkNyTZOdKEtW72693812 = -999165673;    double IoSfdhkNyTZOdKEtW25257295 = -161375691;    double IoSfdhkNyTZOdKEtW1300572 = -313888376;    double IoSfdhkNyTZOdKEtW55766142 = -378595318;    double IoSfdhkNyTZOdKEtW32228905 = -126436515;    double IoSfdhkNyTZOdKEtW9442973 = -108373363;    double IoSfdhkNyTZOdKEtW44034456 = -212145073;    double IoSfdhkNyTZOdKEtW79739114 = -733263042;    double IoSfdhkNyTZOdKEtW70700441 = -937801505;    double IoSfdhkNyTZOdKEtW59214402 = -280336490;    double IoSfdhkNyTZOdKEtW61673114 = -740398183;    double IoSfdhkNyTZOdKEtW71627509 = -176727051;    double IoSfdhkNyTZOdKEtW67741321 = -380166865;    double IoSfdhkNyTZOdKEtW76396034 = -57099748;    double IoSfdhkNyTZOdKEtW22571668 = -859468631;    double IoSfdhkNyTZOdKEtW38937304 = -209166221;    double IoSfdhkNyTZOdKEtW23376223 = 70642444;    double IoSfdhkNyTZOdKEtW76731820 = -759262284;    double IoSfdhkNyTZOdKEtW897232 = -26006462;    double IoSfdhkNyTZOdKEtW13922885 = -980525938;    double IoSfdhkNyTZOdKEtW41657400 = 67279853;    double IoSfdhkNyTZOdKEtW10593477 = 97730041;    double IoSfdhkNyTZOdKEtW83809709 = -260724312;    double IoSfdhkNyTZOdKEtW60000090 = -222924777;    double IoSfdhkNyTZOdKEtW13945082 = -884503023;    double IoSfdhkNyTZOdKEtW23839207 = -13552928;    double IoSfdhkNyTZOdKEtW96196113 = -179443515;    double IoSfdhkNyTZOdKEtW99503281 = -607954171;    double IoSfdhkNyTZOdKEtW65462822 = 94572898;    double IoSfdhkNyTZOdKEtW20644072 = -658161131;    double IoSfdhkNyTZOdKEtW65529267 = -959065344;    double IoSfdhkNyTZOdKEtW41237493 = -818904426;    double IoSfdhkNyTZOdKEtW13861132 = -526069339;    double IoSfdhkNyTZOdKEtW15754808 = -76807578;     IoSfdhkNyTZOdKEtW58356342 = IoSfdhkNyTZOdKEtW84769654;     IoSfdhkNyTZOdKEtW84769654 = IoSfdhkNyTZOdKEtW52095113;     IoSfdhkNyTZOdKEtW52095113 = IoSfdhkNyTZOdKEtW64255698;     IoSfdhkNyTZOdKEtW64255698 = IoSfdhkNyTZOdKEtW49463632;     IoSfdhkNyTZOdKEtW49463632 = IoSfdhkNyTZOdKEtW97527344;     IoSfdhkNyTZOdKEtW97527344 = IoSfdhkNyTZOdKEtW6951778;     IoSfdhkNyTZOdKEtW6951778 = IoSfdhkNyTZOdKEtW34047540;     IoSfdhkNyTZOdKEtW34047540 = IoSfdhkNyTZOdKEtW36383482;     IoSfdhkNyTZOdKEtW36383482 = IoSfdhkNyTZOdKEtW91457519;     IoSfdhkNyTZOdKEtW91457519 = IoSfdhkNyTZOdKEtW70135269;     IoSfdhkNyTZOdKEtW70135269 = IoSfdhkNyTZOdKEtW26081035;     IoSfdhkNyTZOdKEtW26081035 = IoSfdhkNyTZOdKEtW85486525;     IoSfdhkNyTZOdKEtW85486525 = IoSfdhkNyTZOdKEtW98021851;     IoSfdhkNyTZOdKEtW98021851 = IoSfdhkNyTZOdKEtW99180957;     IoSfdhkNyTZOdKEtW99180957 = IoSfdhkNyTZOdKEtW52034520;     IoSfdhkNyTZOdKEtW52034520 = IoSfdhkNyTZOdKEtW36878355;     IoSfdhkNyTZOdKEtW36878355 = IoSfdhkNyTZOdKEtW18412644;     IoSfdhkNyTZOdKEtW18412644 = IoSfdhkNyTZOdKEtW57211016;     IoSfdhkNyTZOdKEtW57211016 = IoSfdhkNyTZOdKEtW73853306;     IoSfdhkNyTZOdKEtW73853306 = IoSfdhkNyTZOdKEtW75089990;     IoSfdhkNyTZOdKEtW75089990 = IoSfdhkNyTZOdKEtW21635153;     IoSfdhkNyTZOdKEtW21635153 = IoSfdhkNyTZOdKEtW55299920;     IoSfdhkNyTZOdKEtW55299920 = IoSfdhkNyTZOdKEtW56128705;     IoSfdhkNyTZOdKEtW56128705 = IoSfdhkNyTZOdKEtW33016430;     IoSfdhkNyTZOdKEtW33016430 = IoSfdhkNyTZOdKEtW74588726;     IoSfdhkNyTZOdKEtW74588726 = IoSfdhkNyTZOdKEtW85518264;     IoSfdhkNyTZOdKEtW85518264 = IoSfdhkNyTZOdKEtW24442985;     IoSfdhkNyTZOdKEtW24442985 = IoSfdhkNyTZOdKEtW83701219;     IoSfdhkNyTZOdKEtW83701219 = IoSfdhkNyTZOdKEtW17658936;     IoSfdhkNyTZOdKEtW17658936 = IoSfdhkNyTZOdKEtW21646240;     IoSfdhkNyTZOdKEtW21646240 = IoSfdhkNyTZOdKEtW97502693;     IoSfdhkNyTZOdKEtW97502693 = IoSfdhkNyTZOdKEtW53387223;     IoSfdhkNyTZOdKEtW53387223 = IoSfdhkNyTZOdKEtW60229231;     IoSfdhkNyTZOdKEtW60229231 = IoSfdhkNyTZOdKEtW96721279;     IoSfdhkNyTZOdKEtW96721279 = IoSfdhkNyTZOdKEtW43414816;     IoSfdhkNyTZOdKEtW43414816 = IoSfdhkNyTZOdKEtW19805615;     IoSfdhkNyTZOdKEtW19805615 = IoSfdhkNyTZOdKEtW27435382;     IoSfdhkNyTZOdKEtW27435382 = IoSfdhkNyTZOdKEtW74378187;     IoSfdhkNyTZOdKEtW74378187 = IoSfdhkNyTZOdKEtW77471902;     IoSfdhkNyTZOdKEtW77471902 = IoSfdhkNyTZOdKEtW3152866;     IoSfdhkNyTZOdKEtW3152866 = IoSfdhkNyTZOdKEtW15875588;     IoSfdhkNyTZOdKEtW15875588 = IoSfdhkNyTZOdKEtW59962039;     IoSfdhkNyTZOdKEtW59962039 = IoSfdhkNyTZOdKEtW83672411;     IoSfdhkNyTZOdKEtW83672411 = IoSfdhkNyTZOdKEtW88387383;     IoSfdhkNyTZOdKEtW88387383 = IoSfdhkNyTZOdKEtW56620396;     IoSfdhkNyTZOdKEtW56620396 = IoSfdhkNyTZOdKEtW52017058;     IoSfdhkNyTZOdKEtW52017058 = IoSfdhkNyTZOdKEtW46580961;     IoSfdhkNyTZOdKEtW46580961 = IoSfdhkNyTZOdKEtW1066762;     IoSfdhkNyTZOdKEtW1066762 = IoSfdhkNyTZOdKEtW6969399;     IoSfdhkNyTZOdKEtW6969399 = IoSfdhkNyTZOdKEtW16761705;     IoSfdhkNyTZOdKEtW16761705 = IoSfdhkNyTZOdKEtW7723356;     IoSfdhkNyTZOdKEtW7723356 = IoSfdhkNyTZOdKEtW55845294;     IoSfdhkNyTZOdKEtW55845294 = IoSfdhkNyTZOdKEtW42793747;     IoSfdhkNyTZOdKEtW42793747 = IoSfdhkNyTZOdKEtW76419521;     IoSfdhkNyTZOdKEtW76419521 = IoSfdhkNyTZOdKEtW36721189;     IoSfdhkNyTZOdKEtW36721189 = IoSfdhkNyTZOdKEtW29469735;     IoSfdhkNyTZOdKEtW29469735 = IoSfdhkNyTZOdKEtW95966408;     IoSfdhkNyTZOdKEtW95966408 = IoSfdhkNyTZOdKEtW31239269;     IoSfdhkNyTZOdKEtW31239269 = IoSfdhkNyTZOdKEtW74874906;     IoSfdhkNyTZOdKEtW74874906 = IoSfdhkNyTZOdKEtW12009080;     IoSfdhkNyTZOdKEtW12009080 = IoSfdhkNyTZOdKEtW82508793;     IoSfdhkNyTZOdKEtW82508793 = IoSfdhkNyTZOdKEtW50346321;     IoSfdhkNyTZOdKEtW50346321 = IoSfdhkNyTZOdKEtW18724546;     IoSfdhkNyTZOdKEtW18724546 = IoSfdhkNyTZOdKEtW69811279;     IoSfdhkNyTZOdKEtW69811279 = IoSfdhkNyTZOdKEtW72632575;     IoSfdhkNyTZOdKEtW72632575 = IoSfdhkNyTZOdKEtW72693812;     IoSfdhkNyTZOdKEtW72693812 = IoSfdhkNyTZOdKEtW25257295;     IoSfdhkNyTZOdKEtW25257295 = IoSfdhkNyTZOdKEtW1300572;     IoSfdhkNyTZOdKEtW1300572 = IoSfdhkNyTZOdKEtW55766142;     IoSfdhkNyTZOdKEtW55766142 = IoSfdhkNyTZOdKEtW32228905;     IoSfdhkNyTZOdKEtW32228905 = IoSfdhkNyTZOdKEtW9442973;     IoSfdhkNyTZOdKEtW9442973 = IoSfdhkNyTZOdKEtW44034456;     IoSfdhkNyTZOdKEtW44034456 = IoSfdhkNyTZOdKEtW79739114;     IoSfdhkNyTZOdKEtW79739114 = IoSfdhkNyTZOdKEtW70700441;     IoSfdhkNyTZOdKEtW70700441 = IoSfdhkNyTZOdKEtW59214402;     IoSfdhkNyTZOdKEtW59214402 = IoSfdhkNyTZOdKEtW61673114;     IoSfdhkNyTZOdKEtW61673114 = IoSfdhkNyTZOdKEtW71627509;     IoSfdhkNyTZOdKEtW71627509 = IoSfdhkNyTZOdKEtW67741321;     IoSfdhkNyTZOdKEtW67741321 = IoSfdhkNyTZOdKEtW76396034;     IoSfdhkNyTZOdKEtW76396034 = IoSfdhkNyTZOdKEtW22571668;     IoSfdhkNyTZOdKEtW22571668 = IoSfdhkNyTZOdKEtW38937304;     IoSfdhkNyTZOdKEtW38937304 = IoSfdhkNyTZOdKEtW23376223;     IoSfdhkNyTZOdKEtW23376223 = IoSfdhkNyTZOdKEtW76731820;     IoSfdhkNyTZOdKEtW76731820 = IoSfdhkNyTZOdKEtW897232;     IoSfdhkNyTZOdKEtW897232 = IoSfdhkNyTZOdKEtW13922885;     IoSfdhkNyTZOdKEtW13922885 = IoSfdhkNyTZOdKEtW41657400;     IoSfdhkNyTZOdKEtW41657400 = IoSfdhkNyTZOdKEtW10593477;     IoSfdhkNyTZOdKEtW10593477 = IoSfdhkNyTZOdKEtW83809709;     IoSfdhkNyTZOdKEtW83809709 = IoSfdhkNyTZOdKEtW60000090;     IoSfdhkNyTZOdKEtW60000090 = IoSfdhkNyTZOdKEtW13945082;     IoSfdhkNyTZOdKEtW13945082 = IoSfdhkNyTZOdKEtW23839207;     IoSfdhkNyTZOdKEtW23839207 = IoSfdhkNyTZOdKEtW96196113;     IoSfdhkNyTZOdKEtW96196113 = IoSfdhkNyTZOdKEtW99503281;     IoSfdhkNyTZOdKEtW99503281 = IoSfdhkNyTZOdKEtW65462822;     IoSfdhkNyTZOdKEtW65462822 = IoSfdhkNyTZOdKEtW20644072;     IoSfdhkNyTZOdKEtW20644072 = IoSfdhkNyTZOdKEtW65529267;     IoSfdhkNyTZOdKEtW65529267 = IoSfdhkNyTZOdKEtW41237493;     IoSfdhkNyTZOdKEtW41237493 = IoSfdhkNyTZOdKEtW13861132;     IoSfdhkNyTZOdKEtW13861132 = IoSfdhkNyTZOdKEtW15754808;     IoSfdhkNyTZOdKEtW15754808 = IoSfdhkNyTZOdKEtW58356342;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hatfblkzgDMBjnlI18801111() {     double odIXMRYrkEEJDRfGw93815310 = -543084478;    double odIXMRYrkEEJDRfGw98041733 = -586194206;    double odIXMRYrkEEJDRfGw89481590 = -167582793;    double odIXMRYrkEEJDRfGw36479053 = -322844256;    double odIXMRYrkEEJDRfGw5748901 = -283453409;    double odIXMRYrkEEJDRfGw33750217 = -81841846;    double odIXMRYrkEEJDRfGw17609184 = -144147843;    double odIXMRYrkEEJDRfGw96179100 = -907066162;    double odIXMRYrkEEJDRfGw37901087 = -695305046;    double odIXMRYrkEEJDRfGw6360232 = -475430684;    double odIXMRYrkEEJDRfGw8317004 = 23521100;    double odIXMRYrkEEJDRfGw26649183 = -227856292;    double odIXMRYrkEEJDRfGw23721171 = -141536548;    double odIXMRYrkEEJDRfGw51162715 = -373087833;    double odIXMRYrkEEJDRfGw57375474 = -928089393;    double odIXMRYrkEEJDRfGw35575821 = -932022066;    double odIXMRYrkEEJDRfGw92714290 = -876516281;    double odIXMRYrkEEJDRfGw18172835 = -263668558;    double odIXMRYrkEEJDRfGw57761346 = -20462966;    double odIXMRYrkEEJDRfGw67690525 = -811788728;    double odIXMRYrkEEJDRfGw88176853 = -316547760;    double odIXMRYrkEEJDRfGw25056678 = -403532601;    double odIXMRYrkEEJDRfGw25825300 = -181708746;    double odIXMRYrkEEJDRfGw49921650 = 43480693;    double odIXMRYrkEEJDRfGw41034381 = -563953531;    double odIXMRYrkEEJDRfGw85286897 = -596290125;    double odIXMRYrkEEJDRfGw45938839 = -660568691;    double odIXMRYrkEEJDRfGw65171409 = -679929257;    double odIXMRYrkEEJDRfGw30369434 = -317055587;    double odIXMRYrkEEJDRfGw23192347 = -483887111;    double odIXMRYrkEEJDRfGw40229472 = -521444000;    double odIXMRYrkEEJDRfGw35325713 = -548179591;    double odIXMRYrkEEJDRfGw65605234 = -512165826;    double odIXMRYrkEEJDRfGw89977657 = -492842167;    double odIXMRYrkEEJDRfGw62529733 = 66063701;    double odIXMRYrkEEJDRfGw94975659 = -160634366;    double odIXMRYrkEEJDRfGw69217431 = -934469984;    double odIXMRYrkEEJDRfGw78650785 = -123823948;    double odIXMRYrkEEJDRfGw4768749 = -628971693;    double odIXMRYrkEEJDRfGw41743434 = -535899049;    double odIXMRYrkEEJDRfGw36604473 = -216625072;    double odIXMRYrkEEJDRfGw5240715 = -973481872;    double odIXMRYrkEEJDRfGw49345417 = -129678910;    double odIXMRYrkEEJDRfGw23803749 = -871681922;    double odIXMRYrkEEJDRfGw88403842 = -992038999;    double odIXMRYrkEEJDRfGw2581725 = -421792218;    double odIXMRYrkEEJDRfGw71362222 = -432231048;    double odIXMRYrkEEJDRfGw64307273 = -711644923;    double odIXMRYrkEEJDRfGw23959428 = -144536407;    double odIXMRYrkEEJDRfGw31498988 = -474725318;    double odIXMRYrkEEJDRfGw97532428 = -197650037;    double odIXMRYrkEEJDRfGw91208827 = -202589922;    double odIXMRYrkEEJDRfGw24466443 = -498533583;    double odIXMRYrkEEJDRfGw58785593 = -553808390;    double odIXMRYrkEEJDRfGw30624258 = -413914188;    double odIXMRYrkEEJDRfGw68758633 = -39551877;    double odIXMRYrkEEJDRfGw72216433 = -304485461;    double odIXMRYrkEEJDRfGw39559940 = -111063487;    double odIXMRYrkEEJDRfGw95444672 = -758890725;    double odIXMRYrkEEJDRfGw20462004 = -687163285;    double odIXMRYrkEEJDRfGw87811378 = -421273156;    double odIXMRYrkEEJDRfGw52437774 = -464218587;    double odIXMRYrkEEJDRfGw65809667 = -490010575;    double odIXMRYrkEEJDRfGw14708740 = -111417935;    double odIXMRYrkEEJDRfGw66130760 = -953986684;    double odIXMRYrkEEJDRfGw72991291 = -428299310;    double odIXMRYrkEEJDRfGw61043949 = -715690467;    double odIXMRYrkEEJDRfGw33743514 = -648694382;    double odIXMRYrkEEJDRfGw88632981 = -339151534;    double odIXMRYrkEEJDRfGw62399814 = -667455027;    double odIXMRYrkEEJDRfGw66358389 = -997552082;    double odIXMRYrkEEJDRfGw14063505 = -652692334;    double odIXMRYrkEEJDRfGw13404086 = -634696865;    double odIXMRYrkEEJDRfGw16017912 = -484563917;    double odIXMRYrkEEJDRfGw31086052 = -495163657;    double odIXMRYrkEEJDRfGw82936138 = -343065888;    double odIXMRYrkEEJDRfGw75711260 = -173853692;    double odIXMRYrkEEJDRfGw2021551 = -310026825;    double odIXMRYrkEEJDRfGw61517808 = 35519692;    double odIXMRYrkEEJDRfGw38452657 = -42161314;    double odIXMRYrkEEJDRfGw13924676 = -64059078;    double odIXMRYrkEEJDRfGw81631565 = -948923768;    double odIXMRYrkEEJDRfGw41211982 = -435392850;    double odIXMRYrkEEJDRfGw98870445 = -842330270;    double odIXMRYrkEEJDRfGw25659919 = -186237075;    double odIXMRYrkEEJDRfGw49020644 = -218854078;    double odIXMRYrkEEJDRfGw10859270 = 50353992;    double odIXMRYrkEEJDRfGw6819641 = -958357436;    double odIXMRYrkEEJDRfGw59353399 = 21072021;    double odIXMRYrkEEJDRfGw93771100 = -894384422;    double odIXMRYrkEEJDRfGw22759227 = -856148906;    double odIXMRYrkEEJDRfGw29657492 = -723406498;    double odIXMRYrkEEJDRfGw83206113 = -364933223;    double odIXMRYrkEEJDRfGw84306745 = -941808409;    double odIXMRYrkEEJDRfGw53932056 = -14625893;    double odIXMRYrkEEJDRfGw84166698 = -752406485;    double odIXMRYrkEEJDRfGw39431047 = -383471297;    double odIXMRYrkEEJDRfGw34636678 = 81739024;    double odIXMRYrkEEJDRfGw57672989 = -917695238;    double odIXMRYrkEEJDRfGw15412551 = -543084478;     odIXMRYrkEEJDRfGw93815310 = odIXMRYrkEEJDRfGw98041733;     odIXMRYrkEEJDRfGw98041733 = odIXMRYrkEEJDRfGw89481590;     odIXMRYrkEEJDRfGw89481590 = odIXMRYrkEEJDRfGw36479053;     odIXMRYrkEEJDRfGw36479053 = odIXMRYrkEEJDRfGw5748901;     odIXMRYrkEEJDRfGw5748901 = odIXMRYrkEEJDRfGw33750217;     odIXMRYrkEEJDRfGw33750217 = odIXMRYrkEEJDRfGw17609184;     odIXMRYrkEEJDRfGw17609184 = odIXMRYrkEEJDRfGw96179100;     odIXMRYrkEEJDRfGw96179100 = odIXMRYrkEEJDRfGw37901087;     odIXMRYrkEEJDRfGw37901087 = odIXMRYrkEEJDRfGw6360232;     odIXMRYrkEEJDRfGw6360232 = odIXMRYrkEEJDRfGw8317004;     odIXMRYrkEEJDRfGw8317004 = odIXMRYrkEEJDRfGw26649183;     odIXMRYrkEEJDRfGw26649183 = odIXMRYrkEEJDRfGw23721171;     odIXMRYrkEEJDRfGw23721171 = odIXMRYrkEEJDRfGw51162715;     odIXMRYrkEEJDRfGw51162715 = odIXMRYrkEEJDRfGw57375474;     odIXMRYrkEEJDRfGw57375474 = odIXMRYrkEEJDRfGw35575821;     odIXMRYrkEEJDRfGw35575821 = odIXMRYrkEEJDRfGw92714290;     odIXMRYrkEEJDRfGw92714290 = odIXMRYrkEEJDRfGw18172835;     odIXMRYrkEEJDRfGw18172835 = odIXMRYrkEEJDRfGw57761346;     odIXMRYrkEEJDRfGw57761346 = odIXMRYrkEEJDRfGw67690525;     odIXMRYrkEEJDRfGw67690525 = odIXMRYrkEEJDRfGw88176853;     odIXMRYrkEEJDRfGw88176853 = odIXMRYrkEEJDRfGw25056678;     odIXMRYrkEEJDRfGw25056678 = odIXMRYrkEEJDRfGw25825300;     odIXMRYrkEEJDRfGw25825300 = odIXMRYrkEEJDRfGw49921650;     odIXMRYrkEEJDRfGw49921650 = odIXMRYrkEEJDRfGw41034381;     odIXMRYrkEEJDRfGw41034381 = odIXMRYrkEEJDRfGw85286897;     odIXMRYrkEEJDRfGw85286897 = odIXMRYrkEEJDRfGw45938839;     odIXMRYrkEEJDRfGw45938839 = odIXMRYrkEEJDRfGw65171409;     odIXMRYrkEEJDRfGw65171409 = odIXMRYrkEEJDRfGw30369434;     odIXMRYrkEEJDRfGw30369434 = odIXMRYrkEEJDRfGw23192347;     odIXMRYrkEEJDRfGw23192347 = odIXMRYrkEEJDRfGw40229472;     odIXMRYrkEEJDRfGw40229472 = odIXMRYrkEEJDRfGw35325713;     odIXMRYrkEEJDRfGw35325713 = odIXMRYrkEEJDRfGw65605234;     odIXMRYrkEEJDRfGw65605234 = odIXMRYrkEEJDRfGw89977657;     odIXMRYrkEEJDRfGw89977657 = odIXMRYrkEEJDRfGw62529733;     odIXMRYrkEEJDRfGw62529733 = odIXMRYrkEEJDRfGw94975659;     odIXMRYrkEEJDRfGw94975659 = odIXMRYrkEEJDRfGw69217431;     odIXMRYrkEEJDRfGw69217431 = odIXMRYrkEEJDRfGw78650785;     odIXMRYrkEEJDRfGw78650785 = odIXMRYrkEEJDRfGw4768749;     odIXMRYrkEEJDRfGw4768749 = odIXMRYrkEEJDRfGw41743434;     odIXMRYrkEEJDRfGw41743434 = odIXMRYrkEEJDRfGw36604473;     odIXMRYrkEEJDRfGw36604473 = odIXMRYrkEEJDRfGw5240715;     odIXMRYrkEEJDRfGw5240715 = odIXMRYrkEEJDRfGw49345417;     odIXMRYrkEEJDRfGw49345417 = odIXMRYrkEEJDRfGw23803749;     odIXMRYrkEEJDRfGw23803749 = odIXMRYrkEEJDRfGw88403842;     odIXMRYrkEEJDRfGw88403842 = odIXMRYrkEEJDRfGw2581725;     odIXMRYrkEEJDRfGw2581725 = odIXMRYrkEEJDRfGw71362222;     odIXMRYrkEEJDRfGw71362222 = odIXMRYrkEEJDRfGw64307273;     odIXMRYrkEEJDRfGw64307273 = odIXMRYrkEEJDRfGw23959428;     odIXMRYrkEEJDRfGw23959428 = odIXMRYrkEEJDRfGw31498988;     odIXMRYrkEEJDRfGw31498988 = odIXMRYrkEEJDRfGw97532428;     odIXMRYrkEEJDRfGw97532428 = odIXMRYrkEEJDRfGw91208827;     odIXMRYrkEEJDRfGw91208827 = odIXMRYrkEEJDRfGw24466443;     odIXMRYrkEEJDRfGw24466443 = odIXMRYrkEEJDRfGw58785593;     odIXMRYrkEEJDRfGw58785593 = odIXMRYrkEEJDRfGw30624258;     odIXMRYrkEEJDRfGw30624258 = odIXMRYrkEEJDRfGw68758633;     odIXMRYrkEEJDRfGw68758633 = odIXMRYrkEEJDRfGw72216433;     odIXMRYrkEEJDRfGw72216433 = odIXMRYrkEEJDRfGw39559940;     odIXMRYrkEEJDRfGw39559940 = odIXMRYrkEEJDRfGw95444672;     odIXMRYrkEEJDRfGw95444672 = odIXMRYrkEEJDRfGw20462004;     odIXMRYrkEEJDRfGw20462004 = odIXMRYrkEEJDRfGw87811378;     odIXMRYrkEEJDRfGw87811378 = odIXMRYrkEEJDRfGw52437774;     odIXMRYrkEEJDRfGw52437774 = odIXMRYrkEEJDRfGw65809667;     odIXMRYrkEEJDRfGw65809667 = odIXMRYrkEEJDRfGw14708740;     odIXMRYrkEEJDRfGw14708740 = odIXMRYrkEEJDRfGw66130760;     odIXMRYrkEEJDRfGw66130760 = odIXMRYrkEEJDRfGw72991291;     odIXMRYrkEEJDRfGw72991291 = odIXMRYrkEEJDRfGw61043949;     odIXMRYrkEEJDRfGw61043949 = odIXMRYrkEEJDRfGw33743514;     odIXMRYrkEEJDRfGw33743514 = odIXMRYrkEEJDRfGw88632981;     odIXMRYrkEEJDRfGw88632981 = odIXMRYrkEEJDRfGw62399814;     odIXMRYrkEEJDRfGw62399814 = odIXMRYrkEEJDRfGw66358389;     odIXMRYrkEEJDRfGw66358389 = odIXMRYrkEEJDRfGw14063505;     odIXMRYrkEEJDRfGw14063505 = odIXMRYrkEEJDRfGw13404086;     odIXMRYrkEEJDRfGw13404086 = odIXMRYrkEEJDRfGw16017912;     odIXMRYrkEEJDRfGw16017912 = odIXMRYrkEEJDRfGw31086052;     odIXMRYrkEEJDRfGw31086052 = odIXMRYrkEEJDRfGw82936138;     odIXMRYrkEEJDRfGw82936138 = odIXMRYrkEEJDRfGw75711260;     odIXMRYrkEEJDRfGw75711260 = odIXMRYrkEEJDRfGw2021551;     odIXMRYrkEEJDRfGw2021551 = odIXMRYrkEEJDRfGw61517808;     odIXMRYrkEEJDRfGw61517808 = odIXMRYrkEEJDRfGw38452657;     odIXMRYrkEEJDRfGw38452657 = odIXMRYrkEEJDRfGw13924676;     odIXMRYrkEEJDRfGw13924676 = odIXMRYrkEEJDRfGw81631565;     odIXMRYrkEEJDRfGw81631565 = odIXMRYrkEEJDRfGw41211982;     odIXMRYrkEEJDRfGw41211982 = odIXMRYrkEEJDRfGw98870445;     odIXMRYrkEEJDRfGw98870445 = odIXMRYrkEEJDRfGw25659919;     odIXMRYrkEEJDRfGw25659919 = odIXMRYrkEEJDRfGw49020644;     odIXMRYrkEEJDRfGw49020644 = odIXMRYrkEEJDRfGw10859270;     odIXMRYrkEEJDRfGw10859270 = odIXMRYrkEEJDRfGw6819641;     odIXMRYrkEEJDRfGw6819641 = odIXMRYrkEEJDRfGw59353399;     odIXMRYrkEEJDRfGw59353399 = odIXMRYrkEEJDRfGw93771100;     odIXMRYrkEEJDRfGw93771100 = odIXMRYrkEEJDRfGw22759227;     odIXMRYrkEEJDRfGw22759227 = odIXMRYrkEEJDRfGw29657492;     odIXMRYrkEEJDRfGw29657492 = odIXMRYrkEEJDRfGw83206113;     odIXMRYrkEEJDRfGw83206113 = odIXMRYrkEEJDRfGw84306745;     odIXMRYrkEEJDRfGw84306745 = odIXMRYrkEEJDRfGw53932056;     odIXMRYrkEEJDRfGw53932056 = odIXMRYrkEEJDRfGw84166698;     odIXMRYrkEEJDRfGw84166698 = odIXMRYrkEEJDRfGw39431047;     odIXMRYrkEEJDRfGw39431047 = odIXMRYrkEEJDRfGw34636678;     odIXMRYrkEEJDRfGw34636678 = odIXMRYrkEEJDRfGw57672989;     odIXMRYrkEEJDRfGw57672989 = odIXMRYrkEEJDRfGw15412551;     odIXMRYrkEEJDRfGw15412551 = odIXMRYrkEEJDRfGw93815310;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void KxbocJUDGbyFZfzu33850179() {     double OpKpnrLCPIzTGMxNr99932417 = -654986367;    double OpKpnrLCPIzTGMxNr41415013 = -295793987;    double OpKpnrLCPIzTGMxNr5523774 = -318415715;    double OpKpnrLCPIzTGMxNr63553834 = -513592886;    double OpKpnrLCPIzTGMxNr74644389 = -804103281;    double OpKpnrLCPIzTGMxNr25167419 = -346338081;    double OpKpnrLCPIzTGMxNr78098149 = -283112820;    double OpKpnrLCPIzTGMxNr45938992 = -93940075;    double OpKpnrLCPIzTGMxNr2543583 = -463337712;    double OpKpnrLCPIzTGMxNr47589891 = -804406161;    double OpKpnrLCPIzTGMxNr68427294 = -953916486;    double OpKpnrLCPIzTGMxNr58472226 = -672262494;    double OpKpnrLCPIzTGMxNr9930290 = -581427637;    double OpKpnrLCPIzTGMxNr34306391 = -905326400;    double OpKpnrLCPIzTGMxNr53117901 = -518551559;    double OpKpnrLCPIzTGMxNr77081214 = -905021609;    double OpKpnrLCPIzTGMxNr86550293 = -393061394;    double OpKpnrLCPIzTGMxNr15225988 = 70808799;    double OpKpnrLCPIzTGMxNr19591188 = -426232063;    double OpKpnrLCPIzTGMxNr48240690 = -68691648;    double OpKpnrLCPIzTGMxNr76730046 = -172035215;    double OpKpnrLCPIzTGMxNr11577702 = -311462318;    double OpKpnrLCPIzTGMxNr61416092 = 71773767;    double OpKpnrLCPIzTGMxNr33914024 = -178824295;    double OpKpnrLCPIzTGMxNr84734031 = -937149564;    double OpKpnrLCPIzTGMxNr10008573 = -474945448;    double OpKpnrLCPIzTGMxNr44398617 = -561230894;    double OpKpnrLCPIzTGMxNr78278534 = -745858210;    double OpKpnrLCPIzTGMxNr96501689 = -539438292;    double OpKpnrLCPIzTGMxNr91807983 = -301357979;    double OpKpnrLCPIzTGMxNr32472021 = -434764938;    double OpKpnrLCPIzTGMxNr32936334 = -257626406;    double OpKpnrLCPIzTGMxNr75316182 = -404779196;    double OpKpnrLCPIzTGMxNr84808221 = -244383932;    double OpKpnrLCPIzTGMxNr24004385 = -572669404;    double OpKpnrLCPIzTGMxNr63701573 = -374660474;    double OpKpnrLCPIzTGMxNr50479366 = -888105854;    double OpKpnrLCPIzTGMxNr12740998 = -995329957;    double OpKpnrLCPIzTGMxNr78379002 = -622895190;    double OpKpnrLCPIzTGMxNr29245372 = 90661728;    double OpKpnrLCPIzTGMxNr42273697 = -281898966;    double OpKpnrLCPIzTGMxNr37391172 = -143624150;    double OpKpnrLCPIzTGMxNr27412396 = -210483494;    double OpKpnrLCPIzTGMxNr11211490 = -891024860;    double OpKpnrLCPIzTGMxNr356308 = -653937154;    double OpKpnrLCPIzTGMxNr14520920 = -407008334;    double OpKpnrLCPIzTGMxNr89140796 = -551006585;    double OpKpnrLCPIzTGMxNr72264214 = -721540455;    double OpKpnrLCPIzTGMxNr33635157 = -230273395;    double OpKpnrLCPIzTGMxNr44410641 = -561736126;    double OpKpnrLCPIzTGMxNr33233107 = -39943708;    double OpKpnrLCPIzTGMxNr2018548 = -3316242;    double OpKpnrLCPIzTGMxNr72714900 = -290323171;    double OpKpnrLCPIzTGMxNr54754474 = -874278535;    double OpKpnrLCPIzTGMxNr57120133 = -14445970;    double OpKpnrLCPIzTGMxNr88354716 = -243524049;    double OpKpnrLCPIzTGMxNr79998921 = -267567754;    double OpKpnrLCPIzTGMxNr71609750 = -39591420;    double OpKpnrLCPIzTGMxNr78819802 = -576443322;    double OpKpnrLCPIzTGMxNr64635816 = -229157833;    double OpKpnrLCPIzTGMxNr80768802 = -785107188;    double OpKpnrLCPIzTGMxNr99819614 = -537254611;    double OpKpnrLCPIzTGMxNr49437303 = -554501784;    double OpKpnrLCPIzTGMxNr10735599 = -61979733;    double OpKpnrLCPIzTGMxNr15117870 = -269641224;    double OpKpnrLCPIzTGMxNr35490960 = -596290080;    double OpKpnrLCPIzTGMxNr83156043 = -167483298;    double OpKpnrLCPIzTGMxNr25122068 = -237043706;    double OpKpnrLCPIzTGMxNr10302006 = -232656997;    double OpKpnrLCPIzTGMxNr89416328 = -43891086;    double OpKpnrLCPIzTGMxNr26601848 = 83084244;    double OpKpnrLCPIzTGMxNr73809295 = -397731437;    double OpKpnrLCPIzTGMxNr36846985 = -306296011;    double OpKpnrLCPIzTGMxNr90345815 = -416893792;    double OpKpnrLCPIzTGMxNr5966993 = -786792682;    double OpKpnrLCPIzTGMxNr39338875 = 71588934;    double OpKpnrLCPIzTGMxNr84165305 = -978824;    double OpKpnrLCPIzTGMxNr50204602 = -37201374;    double OpKpnrLCPIzTGMxNr33557717 = -524887141;    double OpKpnrLCPIzTGMxNr70213112 = -430141231;    double OpKpnrLCPIzTGMxNr20867777 = -923938863;    double OpKpnrLCPIzTGMxNr72134403 = -839690439;    double OpKpnrLCPIzTGMxNr44643378 = -415584816;    double OpKpnrLCPIzTGMxNr52091048 = -977702167;    double OpKpnrLCPIzTGMxNr58574877 = -161414271;    double OpKpnrLCPIzTGMxNr30453474 = -331448696;    double OpKpnrLCPIzTGMxNr60221434 = -967303236;    double OpKpnrLCPIzTGMxNr20561709 = -530500662;    double OpKpnrLCPIzTGMxNr27688089 = -129937962;    double OpKpnrLCPIzTGMxNr35649669 = -229145355;    double OpKpnrLCPIzTGMxNr83702652 = -7092720;    double OpKpnrLCPIzTGMxNr78869616 = -748514434;    double OpKpnrLCPIzTGMxNr33921195 = -318886635;    double OpKpnrLCPIzTGMxNr13743186 = -293737358;    double OpKpnrLCPIzTGMxNr48476570 = -124231084;    double OpKpnrLCPIzTGMxNr42454082 = -744644356;    double OpKpnrLCPIzTGMxNr87953869 = -589122367;    double OpKpnrLCPIzTGMxNr16676797 = -48503761;    double OpKpnrLCPIzTGMxNr96093619 = -521383636;    double OpKpnrLCPIzTGMxNr64865347 = -654986367;     OpKpnrLCPIzTGMxNr99932417 = OpKpnrLCPIzTGMxNr41415013;     OpKpnrLCPIzTGMxNr41415013 = OpKpnrLCPIzTGMxNr5523774;     OpKpnrLCPIzTGMxNr5523774 = OpKpnrLCPIzTGMxNr63553834;     OpKpnrLCPIzTGMxNr63553834 = OpKpnrLCPIzTGMxNr74644389;     OpKpnrLCPIzTGMxNr74644389 = OpKpnrLCPIzTGMxNr25167419;     OpKpnrLCPIzTGMxNr25167419 = OpKpnrLCPIzTGMxNr78098149;     OpKpnrLCPIzTGMxNr78098149 = OpKpnrLCPIzTGMxNr45938992;     OpKpnrLCPIzTGMxNr45938992 = OpKpnrLCPIzTGMxNr2543583;     OpKpnrLCPIzTGMxNr2543583 = OpKpnrLCPIzTGMxNr47589891;     OpKpnrLCPIzTGMxNr47589891 = OpKpnrLCPIzTGMxNr68427294;     OpKpnrLCPIzTGMxNr68427294 = OpKpnrLCPIzTGMxNr58472226;     OpKpnrLCPIzTGMxNr58472226 = OpKpnrLCPIzTGMxNr9930290;     OpKpnrLCPIzTGMxNr9930290 = OpKpnrLCPIzTGMxNr34306391;     OpKpnrLCPIzTGMxNr34306391 = OpKpnrLCPIzTGMxNr53117901;     OpKpnrLCPIzTGMxNr53117901 = OpKpnrLCPIzTGMxNr77081214;     OpKpnrLCPIzTGMxNr77081214 = OpKpnrLCPIzTGMxNr86550293;     OpKpnrLCPIzTGMxNr86550293 = OpKpnrLCPIzTGMxNr15225988;     OpKpnrLCPIzTGMxNr15225988 = OpKpnrLCPIzTGMxNr19591188;     OpKpnrLCPIzTGMxNr19591188 = OpKpnrLCPIzTGMxNr48240690;     OpKpnrLCPIzTGMxNr48240690 = OpKpnrLCPIzTGMxNr76730046;     OpKpnrLCPIzTGMxNr76730046 = OpKpnrLCPIzTGMxNr11577702;     OpKpnrLCPIzTGMxNr11577702 = OpKpnrLCPIzTGMxNr61416092;     OpKpnrLCPIzTGMxNr61416092 = OpKpnrLCPIzTGMxNr33914024;     OpKpnrLCPIzTGMxNr33914024 = OpKpnrLCPIzTGMxNr84734031;     OpKpnrLCPIzTGMxNr84734031 = OpKpnrLCPIzTGMxNr10008573;     OpKpnrLCPIzTGMxNr10008573 = OpKpnrLCPIzTGMxNr44398617;     OpKpnrLCPIzTGMxNr44398617 = OpKpnrLCPIzTGMxNr78278534;     OpKpnrLCPIzTGMxNr78278534 = OpKpnrLCPIzTGMxNr96501689;     OpKpnrLCPIzTGMxNr96501689 = OpKpnrLCPIzTGMxNr91807983;     OpKpnrLCPIzTGMxNr91807983 = OpKpnrLCPIzTGMxNr32472021;     OpKpnrLCPIzTGMxNr32472021 = OpKpnrLCPIzTGMxNr32936334;     OpKpnrLCPIzTGMxNr32936334 = OpKpnrLCPIzTGMxNr75316182;     OpKpnrLCPIzTGMxNr75316182 = OpKpnrLCPIzTGMxNr84808221;     OpKpnrLCPIzTGMxNr84808221 = OpKpnrLCPIzTGMxNr24004385;     OpKpnrLCPIzTGMxNr24004385 = OpKpnrLCPIzTGMxNr63701573;     OpKpnrLCPIzTGMxNr63701573 = OpKpnrLCPIzTGMxNr50479366;     OpKpnrLCPIzTGMxNr50479366 = OpKpnrLCPIzTGMxNr12740998;     OpKpnrLCPIzTGMxNr12740998 = OpKpnrLCPIzTGMxNr78379002;     OpKpnrLCPIzTGMxNr78379002 = OpKpnrLCPIzTGMxNr29245372;     OpKpnrLCPIzTGMxNr29245372 = OpKpnrLCPIzTGMxNr42273697;     OpKpnrLCPIzTGMxNr42273697 = OpKpnrLCPIzTGMxNr37391172;     OpKpnrLCPIzTGMxNr37391172 = OpKpnrLCPIzTGMxNr27412396;     OpKpnrLCPIzTGMxNr27412396 = OpKpnrLCPIzTGMxNr11211490;     OpKpnrLCPIzTGMxNr11211490 = OpKpnrLCPIzTGMxNr356308;     OpKpnrLCPIzTGMxNr356308 = OpKpnrLCPIzTGMxNr14520920;     OpKpnrLCPIzTGMxNr14520920 = OpKpnrLCPIzTGMxNr89140796;     OpKpnrLCPIzTGMxNr89140796 = OpKpnrLCPIzTGMxNr72264214;     OpKpnrLCPIzTGMxNr72264214 = OpKpnrLCPIzTGMxNr33635157;     OpKpnrLCPIzTGMxNr33635157 = OpKpnrLCPIzTGMxNr44410641;     OpKpnrLCPIzTGMxNr44410641 = OpKpnrLCPIzTGMxNr33233107;     OpKpnrLCPIzTGMxNr33233107 = OpKpnrLCPIzTGMxNr2018548;     OpKpnrLCPIzTGMxNr2018548 = OpKpnrLCPIzTGMxNr72714900;     OpKpnrLCPIzTGMxNr72714900 = OpKpnrLCPIzTGMxNr54754474;     OpKpnrLCPIzTGMxNr54754474 = OpKpnrLCPIzTGMxNr57120133;     OpKpnrLCPIzTGMxNr57120133 = OpKpnrLCPIzTGMxNr88354716;     OpKpnrLCPIzTGMxNr88354716 = OpKpnrLCPIzTGMxNr79998921;     OpKpnrLCPIzTGMxNr79998921 = OpKpnrLCPIzTGMxNr71609750;     OpKpnrLCPIzTGMxNr71609750 = OpKpnrLCPIzTGMxNr78819802;     OpKpnrLCPIzTGMxNr78819802 = OpKpnrLCPIzTGMxNr64635816;     OpKpnrLCPIzTGMxNr64635816 = OpKpnrLCPIzTGMxNr80768802;     OpKpnrLCPIzTGMxNr80768802 = OpKpnrLCPIzTGMxNr99819614;     OpKpnrLCPIzTGMxNr99819614 = OpKpnrLCPIzTGMxNr49437303;     OpKpnrLCPIzTGMxNr49437303 = OpKpnrLCPIzTGMxNr10735599;     OpKpnrLCPIzTGMxNr10735599 = OpKpnrLCPIzTGMxNr15117870;     OpKpnrLCPIzTGMxNr15117870 = OpKpnrLCPIzTGMxNr35490960;     OpKpnrLCPIzTGMxNr35490960 = OpKpnrLCPIzTGMxNr83156043;     OpKpnrLCPIzTGMxNr83156043 = OpKpnrLCPIzTGMxNr25122068;     OpKpnrLCPIzTGMxNr25122068 = OpKpnrLCPIzTGMxNr10302006;     OpKpnrLCPIzTGMxNr10302006 = OpKpnrLCPIzTGMxNr89416328;     OpKpnrLCPIzTGMxNr89416328 = OpKpnrLCPIzTGMxNr26601848;     OpKpnrLCPIzTGMxNr26601848 = OpKpnrLCPIzTGMxNr73809295;     OpKpnrLCPIzTGMxNr73809295 = OpKpnrLCPIzTGMxNr36846985;     OpKpnrLCPIzTGMxNr36846985 = OpKpnrLCPIzTGMxNr90345815;     OpKpnrLCPIzTGMxNr90345815 = OpKpnrLCPIzTGMxNr5966993;     OpKpnrLCPIzTGMxNr5966993 = OpKpnrLCPIzTGMxNr39338875;     OpKpnrLCPIzTGMxNr39338875 = OpKpnrLCPIzTGMxNr84165305;     OpKpnrLCPIzTGMxNr84165305 = OpKpnrLCPIzTGMxNr50204602;     OpKpnrLCPIzTGMxNr50204602 = OpKpnrLCPIzTGMxNr33557717;     OpKpnrLCPIzTGMxNr33557717 = OpKpnrLCPIzTGMxNr70213112;     OpKpnrLCPIzTGMxNr70213112 = OpKpnrLCPIzTGMxNr20867777;     OpKpnrLCPIzTGMxNr20867777 = OpKpnrLCPIzTGMxNr72134403;     OpKpnrLCPIzTGMxNr72134403 = OpKpnrLCPIzTGMxNr44643378;     OpKpnrLCPIzTGMxNr44643378 = OpKpnrLCPIzTGMxNr52091048;     OpKpnrLCPIzTGMxNr52091048 = OpKpnrLCPIzTGMxNr58574877;     OpKpnrLCPIzTGMxNr58574877 = OpKpnrLCPIzTGMxNr30453474;     OpKpnrLCPIzTGMxNr30453474 = OpKpnrLCPIzTGMxNr60221434;     OpKpnrLCPIzTGMxNr60221434 = OpKpnrLCPIzTGMxNr20561709;     OpKpnrLCPIzTGMxNr20561709 = OpKpnrLCPIzTGMxNr27688089;     OpKpnrLCPIzTGMxNr27688089 = OpKpnrLCPIzTGMxNr35649669;     OpKpnrLCPIzTGMxNr35649669 = OpKpnrLCPIzTGMxNr83702652;     OpKpnrLCPIzTGMxNr83702652 = OpKpnrLCPIzTGMxNr78869616;     OpKpnrLCPIzTGMxNr78869616 = OpKpnrLCPIzTGMxNr33921195;     OpKpnrLCPIzTGMxNr33921195 = OpKpnrLCPIzTGMxNr13743186;     OpKpnrLCPIzTGMxNr13743186 = OpKpnrLCPIzTGMxNr48476570;     OpKpnrLCPIzTGMxNr48476570 = OpKpnrLCPIzTGMxNr42454082;     OpKpnrLCPIzTGMxNr42454082 = OpKpnrLCPIzTGMxNr87953869;     OpKpnrLCPIzTGMxNr87953869 = OpKpnrLCPIzTGMxNr16676797;     OpKpnrLCPIzTGMxNr16676797 = OpKpnrLCPIzTGMxNr96093619;     OpKpnrLCPIzTGMxNr96093619 = OpKpnrLCPIzTGMxNr64865347;     OpKpnrLCPIzTGMxNr64865347 = OpKpnrLCPIzTGMxNr99932417;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void PQLbOClWhYsovYaH1141779() {     double eqEDHwNwWZUZgewvj35391386 = -21263266;    double eqEDHwNwWZUZgewvj54687091 = -176345593;    double eqEDHwNwWZUZgewvj42910250 = 15917210;    double eqEDHwNwWZUZgewvj35777189 = -334743957;    double eqEDHwNwWZUZgewvj30929659 = -909702126;    double eqEDHwNwWZUZgewvj61390291 = -608685977;    double eqEDHwNwWZUZgewvj88755556 = -127838554;    double eqEDHwNwWZUZgewvj8070553 = -838174215;    double eqEDHwNwWZUZgewvj4061188 = -951669119;    double eqEDHwNwWZUZgewvj62492603 = -469093159;    double eqEDHwNwWZUZgewvj6609030 = -755924215;    double eqEDHwNwWZUZgewvj59040374 = -536383266;    double eqEDHwNwWZUZgewvj48164935 = -33440142;    double eqEDHwNwWZUZgewvj87447254 = -881689810;    double eqEDHwNwWZUZgewvj11312418 = 78023426;    double eqEDHwNwWZUZgewvj60622515 = -540833216;    double eqEDHwNwWZUZgewvj42386229 = -920035947;    double eqEDHwNwWZUZgewvj14986180 = -2619269;    double eqEDHwNwWZUZgewvj20141518 = -742795484;    double eqEDHwNwWZUZgewvj42077908 = -495252713;    double eqEDHwNwWZUZgewvj89816909 = -929309904;    double eqEDHwNwWZUZgewvj14999226 = -398098612;    double eqEDHwNwWZUZgewvj31941472 = -665858417;    double eqEDHwNwWZUZgewvj27706969 = -689648899;    double eqEDHwNwWZUZgewvj92751982 = -961134763;    double eqEDHwNwWZUZgewvj20706745 = -163522255;    double eqEDHwNwWZUZgewvj4819191 = -367515034;    double eqEDHwNwWZUZgewvj19006960 = -915630385;    double eqEDHwNwWZUZgewvj43169904 = -13533094;    double eqEDHwNwWZUZgewvj97341394 = -335865149;    double eqEDHwNwWZUZgewvj51055253 = -986745425;    double eqEDHwNwWZUZgewvj70759352 = -30597821;    double eqEDHwNwWZUZgewvj87534193 = -552375174;    double eqEDHwNwWZUZgewvj14556649 = -309077745;    double eqEDHwNwWZUZgewvj89812838 = -523769655;    double eqEDHwNwWZUZgewvj15262417 = -589225780;    double eqEDHwNwWZUZgewvj99891182 = -752801894;    double eqEDHwNwWZUZgewvj63956401 = -977985538;    double eqEDHwNwWZUZgewvj8769565 = -273771467;    double eqEDHwNwWZUZgewvj93516904 = -474600818;    double eqEDHwNwWZUZgewvj75725304 = -51097880;    double eqEDHwNwWZUZgewvj26756299 = -838169441;    double eqEDHwNwWZUZgewvj16795774 = -863664280;    double eqEDHwNwWZUZgewvj51342828 = -395357270;    double eqEDHwNwWZUZgewvj372766 = -480448315;    double eqEDHwNwWZUZgewvj60482248 = -445931966;    double eqEDHwNwWZUZgewvj8485961 = 65007054;    double eqEDHwNwWZUZgewvj89990525 = -888067049;    double eqEDHwNwWZUZgewvj56527823 = -994010275;    double eqEDHwNwWZUZgewvj68940230 = 47237059;    double eqEDHwNwWZUZgewvj14003831 = 85779735;    double eqEDHwNwWZUZgewvj85504019 = -216968589;    double eqEDHwNwWZUZgewvj41336049 = -46368725;    double eqEDHwNwWZUZgewvj70746320 = 34212965;    double eqEDHwNwWZUZgewvj11324869 = -360936116;    double eqEDHwNwWZUZgewvj20392160 = -623164655;    double eqEDHwNwWZUZgewvj22745620 = -510487177;    double eqEDHwNwWZUZgewvj15203282 = -294433891;    double eqEDHwNwWZUZgewvj43025206 = -373609195;    double eqEDHwNwWZUZgewvj10222914 = -646179872;    double eqEDHwNwWZUZgewvj56571100 = -141170943;    double eqEDHwNwWZUZgewvj69748596 = -212208170;    double eqEDHwNwWZUZgewvj64900649 = -724641122;    double eqEDHwNwWZUZgewvj6719793 = -515803970;    double eqEDHwNwWZUZgewvj11437351 = -482347735;    double eqEDHwNwWZUZgewvj35849677 = -625326395;    double eqEDHwNwWZUZgewvj71506181 = -984008093;    double eqEDHwNwWZUZgewvj33608287 = -724362397;    double eqEDHwNwWZUZgewvj97634415 = -257920155;    double eqEDHwNwWZUZgewvj96050000 = -332750795;    double eqEDHwNwWZUZgewvj60731332 = -788031322;    double eqEDHwNwWZUZgewvj78429828 = -942050409;    double eqEDHwNwWZUZgewvj6216616 = -728847803;    double eqEDHwNwWZUZgewvj26624614 = -168194667;    double eqEDHwNwWZUZgewvj66352603 = -344154834;    double eqEDHwNwWZUZgewvj63060611 = 8859537;    double eqEDHwNwWZUZgewvj98203451 = -534434332;    double eqEDHwNwWZUZgewvj80598643 = -170501148;    double eqEDHwNwWZUZgewvj27334204 = -109200584;    double eqEDHwNwWZUZgewvj32269735 = -415202797;    double eqEDHwNwWZUZgewvj12220785 = -128529310;    double eqEDHwNwWZUZgewvj14828666 = -479447986;    double eqEDHwNwWZUZgewvj62479137 = -921620110;    double eqEDHwNwWZUZgewvj74229673 = 39229846;    double eqEDHwNwWZUZgewvj83337564 = -321644884;    double eqEDHwNwWZUZgewvj65551233 = -669776837;    double eqEDHwNwWZUZgewvj29423304 = -984229096;    double eqEDHwNwWZUZgewvj16787873 = -486588139;    double eqEDHwNwWZUZgewvj3231780 = -948141630;    double eqEDHwNwWZUZgewvj69420679 = -900605000;    double eqEDHwNwWZUZgewvj92516797 = 21261397;    double eqEDHwNwWZUZgewvj84687901 = -358368003;    double eqEDHwNwWZUZgewvj20931195 = -504376344;    double eqEDHwNwWZUZgewvj98546650 = -627591596;    double eqEDHwNwWZUZgewvj36945804 = -233429875;    double eqEDHwNwWZUZgewvj5976709 = -838889710;    double eqEDHwNwWZUZgewvj61855649 = -13528320;    double eqEDHwNwWZUZgewvj10075981 = -247860311;    double eqEDHwNwWZUZgewvj39905478 = -913009536;    double eqEDHwNwWZUZgewvj64523089 = -21263266;     eqEDHwNwWZUZgewvj35391386 = eqEDHwNwWZUZgewvj54687091;     eqEDHwNwWZUZgewvj54687091 = eqEDHwNwWZUZgewvj42910250;     eqEDHwNwWZUZgewvj42910250 = eqEDHwNwWZUZgewvj35777189;     eqEDHwNwWZUZgewvj35777189 = eqEDHwNwWZUZgewvj30929659;     eqEDHwNwWZUZgewvj30929659 = eqEDHwNwWZUZgewvj61390291;     eqEDHwNwWZUZgewvj61390291 = eqEDHwNwWZUZgewvj88755556;     eqEDHwNwWZUZgewvj88755556 = eqEDHwNwWZUZgewvj8070553;     eqEDHwNwWZUZgewvj8070553 = eqEDHwNwWZUZgewvj4061188;     eqEDHwNwWZUZgewvj4061188 = eqEDHwNwWZUZgewvj62492603;     eqEDHwNwWZUZgewvj62492603 = eqEDHwNwWZUZgewvj6609030;     eqEDHwNwWZUZgewvj6609030 = eqEDHwNwWZUZgewvj59040374;     eqEDHwNwWZUZgewvj59040374 = eqEDHwNwWZUZgewvj48164935;     eqEDHwNwWZUZgewvj48164935 = eqEDHwNwWZUZgewvj87447254;     eqEDHwNwWZUZgewvj87447254 = eqEDHwNwWZUZgewvj11312418;     eqEDHwNwWZUZgewvj11312418 = eqEDHwNwWZUZgewvj60622515;     eqEDHwNwWZUZgewvj60622515 = eqEDHwNwWZUZgewvj42386229;     eqEDHwNwWZUZgewvj42386229 = eqEDHwNwWZUZgewvj14986180;     eqEDHwNwWZUZgewvj14986180 = eqEDHwNwWZUZgewvj20141518;     eqEDHwNwWZUZgewvj20141518 = eqEDHwNwWZUZgewvj42077908;     eqEDHwNwWZUZgewvj42077908 = eqEDHwNwWZUZgewvj89816909;     eqEDHwNwWZUZgewvj89816909 = eqEDHwNwWZUZgewvj14999226;     eqEDHwNwWZUZgewvj14999226 = eqEDHwNwWZUZgewvj31941472;     eqEDHwNwWZUZgewvj31941472 = eqEDHwNwWZUZgewvj27706969;     eqEDHwNwWZUZgewvj27706969 = eqEDHwNwWZUZgewvj92751982;     eqEDHwNwWZUZgewvj92751982 = eqEDHwNwWZUZgewvj20706745;     eqEDHwNwWZUZgewvj20706745 = eqEDHwNwWZUZgewvj4819191;     eqEDHwNwWZUZgewvj4819191 = eqEDHwNwWZUZgewvj19006960;     eqEDHwNwWZUZgewvj19006960 = eqEDHwNwWZUZgewvj43169904;     eqEDHwNwWZUZgewvj43169904 = eqEDHwNwWZUZgewvj97341394;     eqEDHwNwWZUZgewvj97341394 = eqEDHwNwWZUZgewvj51055253;     eqEDHwNwWZUZgewvj51055253 = eqEDHwNwWZUZgewvj70759352;     eqEDHwNwWZUZgewvj70759352 = eqEDHwNwWZUZgewvj87534193;     eqEDHwNwWZUZgewvj87534193 = eqEDHwNwWZUZgewvj14556649;     eqEDHwNwWZUZgewvj14556649 = eqEDHwNwWZUZgewvj89812838;     eqEDHwNwWZUZgewvj89812838 = eqEDHwNwWZUZgewvj15262417;     eqEDHwNwWZUZgewvj15262417 = eqEDHwNwWZUZgewvj99891182;     eqEDHwNwWZUZgewvj99891182 = eqEDHwNwWZUZgewvj63956401;     eqEDHwNwWZUZgewvj63956401 = eqEDHwNwWZUZgewvj8769565;     eqEDHwNwWZUZgewvj8769565 = eqEDHwNwWZUZgewvj93516904;     eqEDHwNwWZUZgewvj93516904 = eqEDHwNwWZUZgewvj75725304;     eqEDHwNwWZUZgewvj75725304 = eqEDHwNwWZUZgewvj26756299;     eqEDHwNwWZUZgewvj26756299 = eqEDHwNwWZUZgewvj16795774;     eqEDHwNwWZUZgewvj16795774 = eqEDHwNwWZUZgewvj51342828;     eqEDHwNwWZUZgewvj51342828 = eqEDHwNwWZUZgewvj372766;     eqEDHwNwWZUZgewvj372766 = eqEDHwNwWZUZgewvj60482248;     eqEDHwNwWZUZgewvj60482248 = eqEDHwNwWZUZgewvj8485961;     eqEDHwNwWZUZgewvj8485961 = eqEDHwNwWZUZgewvj89990525;     eqEDHwNwWZUZgewvj89990525 = eqEDHwNwWZUZgewvj56527823;     eqEDHwNwWZUZgewvj56527823 = eqEDHwNwWZUZgewvj68940230;     eqEDHwNwWZUZgewvj68940230 = eqEDHwNwWZUZgewvj14003831;     eqEDHwNwWZUZgewvj14003831 = eqEDHwNwWZUZgewvj85504019;     eqEDHwNwWZUZgewvj85504019 = eqEDHwNwWZUZgewvj41336049;     eqEDHwNwWZUZgewvj41336049 = eqEDHwNwWZUZgewvj70746320;     eqEDHwNwWZUZgewvj70746320 = eqEDHwNwWZUZgewvj11324869;     eqEDHwNwWZUZgewvj11324869 = eqEDHwNwWZUZgewvj20392160;     eqEDHwNwWZUZgewvj20392160 = eqEDHwNwWZUZgewvj22745620;     eqEDHwNwWZUZgewvj22745620 = eqEDHwNwWZUZgewvj15203282;     eqEDHwNwWZUZgewvj15203282 = eqEDHwNwWZUZgewvj43025206;     eqEDHwNwWZUZgewvj43025206 = eqEDHwNwWZUZgewvj10222914;     eqEDHwNwWZUZgewvj10222914 = eqEDHwNwWZUZgewvj56571100;     eqEDHwNwWZUZgewvj56571100 = eqEDHwNwWZUZgewvj69748596;     eqEDHwNwWZUZgewvj69748596 = eqEDHwNwWZUZgewvj64900649;     eqEDHwNwWZUZgewvj64900649 = eqEDHwNwWZUZgewvj6719793;     eqEDHwNwWZUZgewvj6719793 = eqEDHwNwWZUZgewvj11437351;     eqEDHwNwWZUZgewvj11437351 = eqEDHwNwWZUZgewvj35849677;     eqEDHwNwWZUZgewvj35849677 = eqEDHwNwWZUZgewvj71506181;     eqEDHwNwWZUZgewvj71506181 = eqEDHwNwWZUZgewvj33608287;     eqEDHwNwWZUZgewvj33608287 = eqEDHwNwWZUZgewvj97634415;     eqEDHwNwWZUZgewvj97634415 = eqEDHwNwWZUZgewvj96050000;     eqEDHwNwWZUZgewvj96050000 = eqEDHwNwWZUZgewvj60731332;     eqEDHwNwWZUZgewvj60731332 = eqEDHwNwWZUZgewvj78429828;     eqEDHwNwWZUZgewvj78429828 = eqEDHwNwWZUZgewvj6216616;     eqEDHwNwWZUZgewvj6216616 = eqEDHwNwWZUZgewvj26624614;     eqEDHwNwWZUZgewvj26624614 = eqEDHwNwWZUZgewvj66352603;     eqEDHwNwWZUZgewvj66352603 = eqEDHwNwWZUZgewvj63060611;     eqEDHwNwWZUZgewvj63060611 = eqEDHwNwWZUZgewvj98203451;     eqEDHwNwWZUZgewvj98203451 = eqEDHwNwWZUZgewvj80598643;     eqEDHwNwWZUZgewvj80598643 = eqEDHwNwWZUZgewvj27334204;     eqEDHwNwWZUZgewvj27334204 = eqEDHwNwWZUZgewvj32269735;     eqEDHwNwWZUZgewvj32269735 = eqEDHwNwWZUZgewvj12220785;     eqEDHwNwWZUZgewvj12220785 = eqEDHwNwWZUZgewvj14828666;     eqEDHwNwWZUZgewvj14828666 = eqEDHwNwWZUZgewvj62479137;     eqEDHwNwWZUZgewvj62479137 = eqEDHwNwWZUZgewvj74229673;     eqEDHwNwWZUZgewvj74229673 = eqEDHwNwWZUZgewvj83337564;     eqEDHwNwWZUZgewvj83337564 = eqEDHwNwWZUZgewvj65551233;     eqEDHwNwWZUZgewvj65551233 = eqEDHwNwWZUZgewvj29423304;     eqEDHwNwWZUZgewvj29423304 = eqEDHwNwWZUZgewvj16787873;     eqEDHwNwWZUZgewvj16787873 = eqEDHwNwWZUZgewvj3231780;     eqEDHwNwWZUZgewvj3231780 = eqEDHwNwWZUZgewvj69420679;     eqEDHwNwWZUZgewvj69420679 = eqEDHwNwWZUZgewvj92516797;     eqEDHwNwWZUZgewvj92516797 = eqEDHwNwWZUZgewvj84687901;     eqEDHwNwWZUZgewvj84687901 = eqEDHwNwWZUZgewvj20931195;     eqEDHwNwWZUZgewvj20931195 = eqEDHwNwWZUZgewvj98546650;     eqEDHwNwWZUZgewvj98546650 = eqEDHwNwWZUZgewvj36945804;     eqEDHwNwWZUZgewvj36945804 = eqEDHwNwWZUZgewvj5976709;     eqEDHwNwWZUZgewvj5976709 = eqEDHwNwWZUZgewvj61855649;     eqEDHwNwWZUZgewvj61855649 = eqEDHwNwWZUZgewvj10075981;     eqEDHwNwWZUZgewvj10075981 = eqEDHwNwWZUZgewvj39905478;     eqEDHwNwWZUZgewvj39905478 = eqEDHwNwWZUZgewvj64523089;     eqEDHwNwWZUZgewvj64523089 = eqEDHwNwWZUZgewvj35391386;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void HwBAuFoBNMXsurTX16190846() {     double RQxoUYmsNeoKMjhRJ41508493 = -133165155;    double RQxoUYmsNeoKMjhRJ98060370 = -985945374;    double RQxoUYmsNeoKMjhRJ58952434 = -134915711;    double RQxoUYmsNeoKMjhRJ62851969 = -525492588;    double RQxoUYmsNeoKMjhRJ99825147 = -330351997;    double RQxoUYmsNeoKMjhRJ52807493 = -873182212;    double RQxoUYmsNeoKMjhRJ49244522 = -266803532;    double RQxoUYmsNeoKMjhRJ57830444 = -25048129;    double RQxoUYmsNeoKMjhRJ68703683 = -719701784;    double RQxoUYmsNeoKMjhRJ3722263 = -798068637;    double RQxoUYmsNeoKMjhRJ66719319 = -633361802;    double RQxoUYmsNeoKMjhRJ90863416 = -980789468;    double RQxoUYmsNeoKMjhRJ34374054 = -473331231;    double RQxoUYmsNeoKMjhRJ70590931 = -313928378;    double RQxoUYmsNeoKMjhRJ7054844 = -612438741;    double RQxoUYmsNeoKMjhRJ2127909 = -513832759;    double RQxoUYmsNeoKMjhRJ36222233 = -436581059;    double RQxoUYmsNeoKMjhRJ12039333 = -768141912;    double RQxoUYmsNeoKMjhRJ81971359 = -48564581;    double RQxoUYmsNeoKMjhRJ22628073 = -852155633;    double RQxoUYmsNeoKMjhRJ78370102 = -784797359;    double RQxoUYmsNeoKMjhRJ1520250 = -306028328;    double RQxoUYmsNeoKMjhRJ67532263 = -412375904;    double RQxoUYmsNeoKMjhRJ11699343 = -911953887;    double RQxoUYmsNeoKMjhRJ36451633 = -234330796;    double RQxoUYmsNeoKMjhRJ45428420 = -42177578;    double RQxoUYmsNeoKMjhRJ3278970 = -268177237;    double RQxoUYmsNeoKMjhRJ32114085 = -981559338;    double RQxoUYmsNeoKMjhRJ9302160 = -235915799;    double RQxoUYmsNeoKMjhRJ65957031 = -153336017;    double RQxoUYmsNeoKMjhRJ43297802 = -900066363;    double RQxoUYmsNeoKMjhRJ68369974 = -840044636;    double RQxoUYmsNeoKMjhRJ97245141 = -444988544;    double RQxoUYmsNeoKMjhRJ9387213 = -60619510;    double RQxoUYmsNeoKMjhRJ51287491 = -62502760;    double RQxoUYmsNeoKMjhRJ83988330 = -803251887;    double RQxoUYmsNeoKMjhRJ81153117 = -706437764;    double RQxoUYmsNeoKMjhRJ98046613 = -749491548;    double RQxoUYmsNeoKMjhRJ82379818 = -267694964;    double RQxoUYmsNeoKMjhRJ81018841 = -948040040;    double RQxoUYmsNeoKMjhRJ81394528 = -116371774;    double RQxoUYmsNeoKMjhRJ58906755 = -8311719;    double RQxoUYmsNeoKMjhRJ94862752 = -944468864;    double RQxoUYmsNeoKMjhRJ38750568 = -414700208;    double RQxoUYmsNeoKMjhRJ12325231 = -142346469;    double RQxoUYmsNeoKMjhRJ72421443 = -431148083;    double RQxoUYmsNeoKMjhRJ26264535 = -53768484;    double RQxoUYmsNeoKMjhRJ97947466 = -897962581;    double RQxoUYmsNeoKMjhRJ66203552 = 20252737;    double RQxoUYmsNeoKMjhRJ81851883 = -39773749;    double RQxoUYmsNeoKMjhRJ49704509 = -856513937;    double RQxoUYmsNeoKMjhRJ96313738 = -17694909;    double RQxoUYmsNeoKMjhRJ89584506 = -938158313;    double RQxoUYmsNeoKMjhRJ66715200 = -286257180;    double RQxoUYmsNeoKMjhRJ37820744 = 38532102;    double RQxoUYmsNeoKMjhRJ39988243 = -827136828;    double RQxoUYmsNeoKMjhRJ30528107 = -473569471;    double RQxoUYmsNeoKMjhRJ47253091 = -222961825;    double RQxoUYmsNeoKMjhRJ26400337 = -191161792;    double RQxoUYmsNeoKMjhRJ54396727 = -188174420;    double RQxoUYmsNeoKMjhRJ49528524 = -505004975;    double RQxoUYmsNeoKMjhRJ17130437 = -285244194;    double RQxoUYmsNeoKMjhRJ48528284 = -789132330;    double RQxoUYmsNeoKMjhRJ2746652 = -466365768;    double RQxoUYmsNeoKMjhRJ60424460 = -898002274;    double RQxoUYmsNeoKMjhRJ98349344 = -793317166;    double RQxoUYmsNeoKMjhRJ93618275 = -435800924;    double RQxoUYmsNeoKMjhRJ24986841 = -312711721;    double RQxoUYmsNeoKMjhRJ19303441 = -151425618;    double RQxoUYmsNeoKMjhRJ23066514 = -809186854;    double RQxoUYmsNeoKMjhRJ20974792 = -807394996;    double RQxoUYmsNeoKMjhRJ38175619 = -687089512;    double RQxoUYmsNeoKMjhRJ29659515 = -400446949;    double RQxoUYmsNeoKMjhRJ952518 = -100524541;    double RQxoUYmsNeoKMjhRJ41233545 = -635783859;    double RQxoUYmsNeoKMjhRJ19463347 = -676485641;    double RQxoUYmsNeoKMjhRJ6657498 = -361559465;    double RQxoUYmsNeoKMjhRJ28781695 = -997675696;    double RQxoUYmsNeoKMjhRJ99374112 = -669607418;    double RQxoUYmsNeoKMjhRJ64030189 = -803182713;    double RQxoUYmsNeoKMjhRJ19163886 = -988409095;    double RQxoUYmsNeoKMjhRJ5331503 = -370214657;    double RQxoUYmsNeoKMjhRJ65910533 = -901812076;    double RQxoUYmsNeoKMjhRJ27450277 = -96142050;    double RQxoUYmsNeoKMjhRJ16252523 = -296822080;    double RQxoUYmsNeoKMjhRJ46984063 = -782371455;    double RQxoUYmsNeoKMjhRJ78785468 = -901886324;    double RQxoUYmsNeoKMjhRJ30529941 = -58731364;    double RQxoUYmsNeoKMjhRJ71566469 = 848387;    double RQxoUYmsNeoKMjhRJ11299249 = -235365933;    double RQxoUYmsNeoKMjhRJ53460223 = -229682417;    double RQxoUYmsNeoKMjhRJ33900026 = -383475940;    double RQxoUYmsNeoKMjhRJ71646276 = -458329756;    double RQxoUYmsNeoKMjhRJ27983091 = 20479456;    double RQxoUYmsNeoKMjhRJ31490318 = -343035066;    double RQxoUYmsNeoKMjhRJ64264091 = -831127581;    double RQxoUYmsNeoKMjhRJ10378471 = -219179389;    double RQxoUYmsNeoKMjhRJ92116100 = -378103096;    double RQxoUYmsNeoKMjhRJ78326108 = -516697934;    double RQxoUYmsNeoKMjhRJ13975886 = -133165155;     RQxoUYmsNeoKMjhRJ41508493 = RQxoUYmsNeoKMjhRJ98060370;     RQxoUYmsNeoKMjhRJ98060370 = RQxoUYmsNeoKMjhRJ58952434;     RQxoUYmsNeoKMjhRJ58952434 = RQxoUYmsNeoKMjhRJ62851969;     RQxoUYmsNeoKMjhRJ62851969 = RQxoUYmsNeoKMjhRJ99825147;     RQxoUYmsNeoKMjhRJ99825147 = RQxoUYmsNeoKMjhRJ52807493;     RQxoUYmsNeoKMjhRJ52807493 = RQxoUYmsNeoKMjhRJ49244522;     RQxoUYmsNeoKMjhRJ49244522 = RQxoUYmsNeoKMjhRJ57830444;     RQxoUYmsNeoKMjhRJ57830444 = RQxoUYmsNeoKMjhRJ68703683;     RQxoUYmsNeoKMjhRJ68703683 = RQxoUYmsNeoKMjhRJ3722263;     RQxoUYmsNeoKMjhRJ3722263 = RQxoUYmsNeoKMjhRJ66719319;     RQxoUYmsNeoKMjhRJ66719319 = RQxoUYmsNeoKMjhRJ90863416;     RQxoUYmsNeoKMjhRJ90863416 = RQxoUYmsNeoKMjhRJ34374054;     RQxoUYmsNeoKMjhRJ34374054 = RQxoUYmsNeoKMjhRJ70590931;     RQxoUYmsNeoKMjhRJ70590931 = RQxoUYmsNeoKMjhRJ7054844;     RQxoUYmsNeoKMjhRJ7054844 = RQxoUYmsNeoKMjhRJ2127909;     RQxoUYmsNeoKMjhRJ2127909 = RQxoUYmsNeoKMjhRJ36222233;     RQxoUYmsNeoKMjhRJ36222233 = RQxoUYmsNeoKMjhRJ12039333;     RQxoUYmsNeoKMjhRJ12039333 = RQxoUYmsNeoKMjhRJ81971359;     RQxoUYmsNeoKMjhRJ81971359 = RQxoUYmsNeoKMjhRJ22628073;     RQxoUYmsNeoKMjhRJ22628073 = RQxoUYmsNeoKMjhRJ78370102;     RQxoUYmsNeoKMjhRJ78370102 = RQxoUYmsNeoKMjhRJ1520250;     RQxoUYmsNeoKMjhRJ1520250 = RQxoUYmsNeoKMjhRJ67532263;     RQxoUYmsNeoKMjhRJ67532263 = RQxoUYmsNeoKMjhRJ11699343;     RQxoUYmsNeoKMjhRJ11699343 = RQxoUYmsNeoKMjhRJ36451633;     RQxoUYmsNeoKMjhRJ36451633 = RQxoUYmsNeoKMjhRJ45428420;     RQxoUYmsNeoKMjhRJ45428420 = RQxoUYmsNeoKMjhRJ3278970;     RQxoUYmsNeoKMjhRJ3278970 = RQxoUYmsNeoKMjhRJ32114085;     RQxoUYmsNeoKMjhRJ32114085 = RQxoUYmsNeoKMjhRJ9302160;     RQxoUYmsNeoKMjhRJ9302160 = RQxoUYmsNeoKMjhRJ65957031;     RQxoUYmsNeoKMjhRJ65957031 = RQxoUYmsNeoKMjhRJ43297802;     RQxoUYmsNeoKMjhRJ43297802 = RQxoUYmsNeoKMjhRJ68369974;     RQxoUYmsNeoKMjhRJ68369974 = RQxoUYmsNeoKMjhRJ97245141;     RQxoUYmsNeoKMjhRJ97245141 = RQxoUYmsNeoKMjhRJ9387213;     RQxoUYmsNeoKMjhRJ9387213 = RQxoUYmsNeoKMjhRJ51287491;     RQxoUYmsNeoKMjhRJ51287491 = RQxoUYmsNeoKMjhRJ83988330;     RQxoUYmsNeoKMjhRJ83988330 = RQxoUYmsNeoKMjhRJ81153117;     RQxoUYmsNeoKMjhRJ81153117 = RQxoUYmsNeoKMjhRJ98046613;     RQxoUYmsNeoKMjhRJ98046613 = RQxoUYmsNeoKMjhRJ82379818;     RQxoUYmsNeoKMjhRJ82379818 = RQxoUYmsNeoKMjhRJ81018841;     RQxoUYmsNeoKMjhRJ81018841 = RQxoUYmsNeoKMjhRJ81394528;     RQxoUYmsNeoKMjhRJ81394528 = RQxoUYmsNeoKMjhRJ58906755;     RQxoUYmsNeoKMjhRJ58906755 = RQxoUYmsNeoKMjhRJ94862752;     RQxoUYmsNeoKMjhRJ94862752 = RQxoUYmsNeoKMjhRJ38750568;     RQxoUYmsNeoKMjhRJ38750568 = RQxoUYmsNeoKMjhRJ12325231;     RQxoUYmsNeoKMjhRJ12325231 = RQxoUYmsNeoKMjhRJ72421443;     RQxoUYmsNeoKMjhRJ72421443 = RQxoUYmsNeoKMjhRJ26264535;     RQxoUYmsNeoKMjhRJ26264535 = RQxoUYmsNeoKMjhRJ97947466;     RQxoUYmsNeoKMjhRJ97947466 = RQxoUYmsNeoKMjhRJ66203552;     RQxoUYmsNeoKMjhRJ66203552 = RQxoUYmsNeoKMjhRJ81851883;     RQxoUYmsNeoKMjhRJ81851883 = RQxoUYmsNeoKMjhRJ49704509;     RQxoUYmsNeoKMjhRJ49704509 = RQxoUYmsNeoKMjhRJ96313738;     RQxoUYmsNeoKMjhRJ96313738 = RQxoUYmsNeoKMjhRJ89584506;     RQxoUYmsNeoKMjhRJ89584506 = RQxoUYmsNeoKMjhRJ66715200;     RQxoUYmsNeoKMjhRJ66715200 = RQxoUYmsNeoKMjhRJ37820744;     RQxoUYmsNeoKMjhRJ37820744 = RQxoUYmsNeoKMjhRJ39988243;     RQxoUYmsNeoKMjhRJ39988243 = RQxoUYmsNeoKMjhRJ30528107;     RQxoUYmsNeoKMjhRJ30528107 = RQxoUYmsNeoKMjhRJ47253091;     RQxoUYmsNeoKMjhRJ47253091 = RQxoUYmsNeoKMjhRJ26400337;     RQxoUYmsNeoKMjhRJ26400337 = RQxoUYmsNeoKMjhRJ54396727;     RQxoUYmsNeoKMjhRJ54396727 = RQxoUYmsNeoKMjhRJ49528524;     RQxoUYmsNeoKMjhRJ49528524 = RQxoUYmsNeoKMjhRJ17130437;     RQxoUYmsNeoKMjhRJ17130437 = RQxoUYmsNeoKMjhRJ48528284;     RQxoUYmsNeoKMjhRJ48528284 = RQxoUYmsNeoKMjhRJ2746652;     RQxoUYmsNeoKMjhRJ2746652 = RQxoUYmsNeoKMjhRJ60424460;     RQxoUYmsNeoKMjhRJ60424460 = RQxoUYmsNeoKMjhRJ98349344;     RQxoUYmsNeoKMjhRJ98349344 = RQxoUYmsNeoKMjhRJ93618275;     RQxoUYmsNeoKMjhRJ93618275 = RQxoUYmsNeoKMjhRJ24986841;     RQxoUYmsNeoKMjhRJ24986841 = RQxoUYmsNeoKMjhRJ19303441;     RQxoUYmsNeoKMjhRJ19303441 = RQxoUYmsNeoKMjhRJ23066514;     RQxoUYmsNeoKMjhRJ23066514 = RQxoUYmsNeoKMjhRJ20974792;     RQxoUYmsNeoKMjhRJ20974792 = RQxoUYmsNeoKMjhRJ38175619;     RQxoUYmsNeoKMjhRJ38175619 = RQxoUYmsNeoKMjhRJ29659515;     RQxoUYmsNeoKMjhRJ29659515 = RQxoUYmsNeoKMjhRJ952518;     RQxoUYmsNeoKMjhRJ952518 = RQxoUYmsNeoKMjhRJ41233545;     RQxoUYmsNeoKMjhRJ41233545 = RQxoUYmsNeoKMjhRJ19463347;     RQxoUYmsNeoKMjhRJ19463347 = RQxoUYmsNeoKMjhRJ6657498;     RQxoUYmsNeoKMjhRJ6657498 = RQxoUYmsNeoKMjhRJ28781695;     RQxoUYmsNeoKMjhRJ28781695 = RQxoUYmsNeoKMjhRJ99374112;     RQxoUYmsNeoKMjhRJ99374112 = RQxoUYmsNeoKMjhRJ64030189;     RQxoUYmsNeoKMjhRJ64030189 = RQxoUYmsNeoKMjhRJ19163886;     RQxoUYmsNeoKMjhRJ19163886 = RQxoUYmsNeoKMjhRJ5331503;     RQxoUYmsNeoKMjhRJ5331503 = RQxoUYmsNeoKMjhRJ65910533;     RQxoUYmsNeoKMjhRJ65910533 = RQxoUYmsNeoKMjhRJ27450277;     RQxoUYmsNeoKMjhRJ27450277 = RQxoUYmsNeoKMjhRJ16252523;     RQxoUYmsNeoKMjhRJ16252523 = RQxoUYmsNeoKMjhRJ46984063;     RQxoUYmsNeoKMjhRJ46984063 = RQxoUYmsNeoKMjhRJ78785468;     RQxoUYmsNeoKMjhRJ78785468 = RQxoUYmsNeoKMjhRJ30529941;     RQxoUYmsNeoKMjhRJ30529941 = RQxoUYmsNeoKMjhRJ71566469;     RQxoUYmsNeoKMjhRJ71566469 = RQxoUYmsNeoKMjhRJ11299249;     RQxoUYmsNeoKMjhRJ11299249 = RQxoUYmsNeoKMjhRJ53460223;     RQxoUYmsNeoKMjhRJ53460223 = RQxoUYmsNeoKMjhRJ33900026;     RQxoUYmsNeoKMjhRJ33900026 = RQxoUYmsNeoKMjhRJ71646276;     RQxoUYmsNeoKMjhRJ71646276 = RQxoUYmsNeoKMjhRJ27983091;     RQxoUYmsNeoKMjhRJ27983091 = RQxoUYmsNeoKMjhRJ31490318;     RQxoUYmsNeoKMjhRJ31490318 = RQxoUYmsNeoKMjhRJ64264091;     RQxoUYmsNeoKMjhRJ64264091 = RQxoUYmsNeoKMjhRJ10378471;     RQxoUYmsNeoKMjhRJ10378471 = RQxoUYmsNeoKMjhRJ92116100;     RQxoUYmsNeoKMjhRJ92116100 = RQxoUYmsNeoKMjhRJ78326108;     RQxoUYmsNeoKMjhRJ78326108 = RQxoUYmsNeoKMjhRJ13975886;     RQxoUYmsNeoKMjhRJ13975886 = RQxoUYmsNeoKMjhRJ41508493;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void qfTVJAGCOTnlwvLG83482445() {     double NoJeZtCOKPnixppDB76967461 = -599442055;    double NoJeZtCOKPnixppDB11332449 = -866496981;    double NoJeZtCOKPnixppDB96338910 = -900582786;    double NoJeZtCOKPnixppDB35075325 = -346643659;    double NoJeZtCOKPnixppDB56110416 = -435950843;    double NoJeZtCOKPnixppDB89030365 = -35530107;    double NoJeZtCOKPnixppDB59901928 = -111529266;    double NoJeZtCOKPnixppDB19962005 = -769282269;    double NoJeZtCOKPnixppDB70221288 = -108033191;    double NoJeZtCOKPnixppDB18624975 = -462755635;    double NoJeZtCOKPnixppDB4901055 = -435369531;    double NoJeZtCOKPnixppDB91431565 = -844910239;    double NoJeZtCOKPnixppDB72608699 = 74656264;    double NoJeZtCOKPnixppDB23731795 = -290291787;    double NoJeZtCOKPnixppDB65249360 = -15863756;    double NoJeZtCOKPnixppDB85669209 = -149644367;    double NoJeZtCOKPnixppDB92058168 = -963555612;    double NoJeZtCOKPnixppDB11799524 = -841569980;    double NoJeZtCOKPnixppDB82521689 = -365128002;    double NoJeZtCOKPnixppDB16465292 = -178716699;    double NoJeZtCOKPnixppDB91456965 = -442072048;    double NoJeZtCOKPnixppDB4941775 = -392664622;    double NoJeZtCOKPnixppDB38057643 = -50008088;    double NoJeZtCOKPnixppDB5492288 = -322778491;    double NoJeZtCOKPnixppDB44469584 = -258315994;    double NoJeZtCOKPnixppDB56126592 = -830754385;    double NoJeZtCOKPnixppDB63699543 = -74461378;    double NoJeZtCOKPnixppDB72842510 = -51331513;    double NoJeZtCOKPnixppDB55970374 = -810010601;    double NoJeZtCOKPnixppDB71490442 = -187843187;    double NoJeZtCOKPnixppDB61881034 = -352046850;    double NoJeZtCOKPnixppDB6192993 = -613016051;    double NoJeZtCOKPnixppDB9463153 = -592584521;    double NoJeZtCOKPnixppDB39135639 = -125313324;    double NoJeZtCOKPnixppDB17095945 = -13603011;    double NoJeZtCOKPnixppDB35549174 = 82182807;    double NoJeZtCOKPnixppDB30564934 = -571133804;    double NoJeZtCOKPnixppDB49262017 = -732147129;    double NoJeZtCOKPnixppDB12770380 = 81428760;    double NoJeZtCOKPnixppDB45290374 = -413302586;    double NoJeZtCOKPnixppDB14846136 = -985570688;    double NoJeZtCOKPnixppDB48271882 = -702857010;    double NoJeZtCOKPnixppDB84246130 = -497649650;    double NoJeZtCOKPnixppDB78881906 = 80967382;    double NoJeZtCOKPnixppDB12341690 = 31142369;    double NoJeZtCOKPnixppDB18382772 = -470071715;    double NoJeZtCOKPnixppDB45609698 = -537754844;    double NoJeZtCOKPnixppDB15673779 = 35510825;    double NoJeZtCOKPnixppDB89096217 = -743484143;    double NoJeZtCOKPnixppDB6381473 = -530800565;    double NoJeZtCOKPnixppDB30475232 = -730790494;    double NoJeZtCOKPnixppDB79799211 = -231347255;    double NoJeZtCOKPnixppDB58205654 = -694203867;    double NoJeZtCOKPnixppDB82707047 = -477765680;    double NoJeZtCOKPnixppDB92025480 = -307958044;    double NoJeZtCOKPnixppDB72025686 = -106777433;    double NoJeZtCOKPnixppDB73274806 = -716488893;    double NoJeZtCOKPnixppDB90846622 = -477804296;    double NoJeZtCOKPnixppDB90605740 = 11672335;    double NoJeZtCOKPnixppDB99983824 = -605196458;    double NoJeZtCOKPnixppDB25330823 = -961068729;    double NoJeZtCOKPnixppDB87059418 = 39802246;    double NoJeZtCOKPnixppDB63991631 = -959271668;    double NoJeZtCOKPnixppDB98730845 = -920190005;    double NoJeZtCOKPnixppDB56743941 = -10708785;    double NoJeZtCOKPnixppDB98708061 = -822353481;    double NoJeZtCOKPnixppDB81968412 = -152325718;    double NoJeZtCOKPnixppDB33473060 = -800030412;    double NoJeZtCOKPnixppDB6635850 = -176688777;    double NoJeZtCOKPnixppDB29700187 = 1953437;    double NoJeZtCOKPnixppDB55104276 = -578510563;    double NoJeZtCOKPnixppDB42796151 = -131408483;    double NoJeZtCOKPnixppDB99029144 = -822998741;    double NoJeZtCOKPnixppDB37231315 = -951825417;    double NoJeZtCOKPnixppDB1619156 = -193146011;    double NoJeZtCOKPnixppDB43185083 = -739215039;    double NoJeZtCOKPnixppDB20695644 = -895014973;    double NoJeZtCOKPnixppDB59175736 = -30975471;    double NoJeZtCOKPnixppDB93150598 = -253920860;    double NoJeZtCOKPnixppDB26086813 = -788244279;    double NoJeZtCOKPnixppDB10516894 = -192999541;    double NoJeZtCOKPnixppDB48025765 = -9972204;    double NoJeZtCOKPnixppDB83746292 = -307847370;    double NoJeZtCOKPnixppDB49588902 = -179210037;    double NoJeZtCOKPnixppDB41015210 = -457052694;    double NoJeZtCOKPnixppDB82081822 = -20699595;    double NoJeZtCOKPnixppDB47987338 = -918812184;    double NoJeZtCOKPnixppDB26756106 = -14818842;    double NoJeZtCOKPnixppDB47110159 = -817355280;    double NoJeZtCOKPnixppDB45070258 = -906825578;    double NoJeZtCOKPnixppDB62274368 = -201328300;    double NoJeZtCOKPnixppDB39718311 = 6670491;    double NoJeZtCOKPnixppDB58656276 = -643819464;    double NoJeZtCOKPnixppDB12786555 = -313374782;    double NoJeZtCOKPnixppDB19959552 = -452233857;    double NoJeZtCOKPnixppDB27786718 = -925372935;    double NoJeZtCOKPnixppDB84280251 = -743585342;    double NoJeZtCOKPnixppDB85515284 = -577459646;    double NoJeZtCOKPnixppDB22137966 = -908323833;    double NoJeZtCOKPnixppDB13633628 = -599442055;     NoJeZtCOKPnixppDB76967461 = NoJeZtCOKPnixppDB11332449;     NoJeZtCOKPnixppDB11332449 = NoJeZtCOKPnixppDB96338910;     NoJeZtCOKPnixppDB96338910 = NoJeZtCOKPnixppDB35075325;     NoJeZtCOKPnixppDB35075325 = NoJeZtCOKPnixppDB56110416;     NoJeZtCOKPnixppDB56110416 = NoJeZtCOKPnixppDB89030365;     NoJeZtCOKPnixppDB89030365 = NoJeZtCOKPnixppDB59901928;     NoJeZtCOKPnixppDB59901928 = NoJeZtCOKPnixppDB19962005;     NoJeZtCOKPnixppDB19962005 = NoJeZtCOKPnixppDB70221288;     NoJeZtCOKPnixppDB70221288 = NoJeZtCOKPnixppDB18624975;     NoJeZtCOKPnixppDB18624975 = NoJeZtCOKPnixppDB4901055;     NoJeZtCOKPnixppDB4901055 = NoJeZtCOKPnixppDB91431565;     NoJeZtCOKPnixppDB91431565 = NoJeZtCOKPnixppDB72608699;     NoJeZtCOKPnixppDB72608699 = NoJeZtCOKPnixppDB23731795;     NoJeZtCOKPnixppDB23731795 = NoJeZtCOKPnixppDB65249360;     NoJeZtCOKPnixppDB65249360 = NoJeZtCOKPnixppDB85669209;     NoJeZtCOKPnixppDB85669209 = NoJeZtCOKPnixppDB92058168;     NoJeZtCOKPnixppDB92058168 = NoJeZtCOKPnixppDB11799524;     NoJeZtCOKPnixppDB11799524 = NoJeZtCOKPnixppDB82521689;     NoJeZtCOKPnixppDB82521689 = NoJeZtCOKPnixppDB16465292;     NoJeZtCOKPnixppDB16465292 = NoJeZtCOKPnixppDB91456965;     NoJeZtCOKPnixppDB91456965 = NoJeZtCOKPnixppDB4941775;     NoJeZtCOKPnixppDB4941775 = NoJeZtCOKPnixppDB38057643;     NoJeZtCOKPnixppDB38057643 = NoJeZtCOKPnixppDB5492288;     NoJeZtCOKPnixppDB5492288 = NoJeZtCOKPnixppDB44469584;     NoJeZtCOKPnixppDB44469584 = NoJeZtCOKPnixppDB56126592;     NoJeZtCOKPnixppDB56126592 = NoJeZtCOKPnixppDB63699543;     NoJeZtCOKPnixppDB63699543 = NoJeZtCOKPnixppDB72842510;     NoJeZtCOKPnixppDB72842510 = NoJeZtCOKPnixppDB55970374;     NoJeZtCOKPnixppDB55970374 = NoJeZtCOKPnixppDB71490442;     NoJeZtCOKPnixppDB71490442 = NoJeZtCOKPnixppDB61881034;     NoJeZtCOKPnixppDB61881034 = NoJeZtCOKPnixppDB6192993;     NoJeZtCOKPnixppDB6192993 = NoJeZtCOKPnixppDB9463153;     NoJeZtCOKPnixppDB9463153 = NoJeZtCOKPnixppDB39135639;     NoJeZtCOKPnixppDB39135639 = NoJeZtCOKPnixppDB17095945;     NoJeZtCOKPnixppDB17095945 = NoJeZtCOKPnixppDB35549174;     NoJeZtCOKPnixppDB35549174 = NoJeZtCOKPnixppDB30564934;     NoJeZtCOKPnixppDB30564934 = NoJeZtCOKPnixppDB49262017;     NoJeZtCOKPnixppDB49262017 = NoJeZtCOKPnixppDB12770380;     NoJeZtCOKPnixppDB12770380 = NoJeZtCOKPnixppDB45290374;     NoJeZtCOKPnixppDB45290374 = NoJeZtCOKPnixppDB14846136;     NoJeZtCOKPnixppDB14846136 = NoJeZtCOKPnixppDB48271882;     NoJeZtCOKPnixppDB48271882 = NoJeZtCOKPnixppDB84246130;     NoJeZtCOKPnixppDB84246130 = NoJeZtCOKPnixppDB78881906;     NoJeZtCOKPnixppDB78881906 = NoJeZtCOKPnixppDB12341690;     NoJeZtCOKPnixppDB12341690 = NoJeZtCOKPnixppDB18382772;     NoJeZtCOKPnixppDB18382772 = NoJeZtCOKPnixppDB45609698;     NoJeZtCOKPnixppDB45609698 = NoJeZtCOKPnixppDB15673779;     NoJeZtCOKPnixppDB15673779 = NoJeZtCOKPnixppDB89096217;     NoJeZtCOKPnixppDB89096217 = NoJeZtCOKPnixppDB6381473;     NoJeZtCOKPnixppDB6381473 = NoJeZtCOKPnixppDB30475232;     NoJeZtCOKPnixppDB30475232 = NoJeZtCOKPnixppDB79799211;     NoJeZtCOKPnixppDB79799211 = NoJeZtCOKPnixppDB58205654;     NoJeZtCOKPnixppDB58205654 = NoJeZtCOKPnixppDB82707047;     NoJeZtCOKPnixppDB82707047 = NoJeZtCOKPnixppDB92025480;     NoJeZtCOKPnixppDB92025480 = NoJeZtCOKPnixppDB72025686;     NoJeZtCOKPnixppDB72025686 = NoJeZtCOKPnixppDB73274806;     NoJeZtCOKPnixppDB73274806 = NoJeZtCOKPnixppDB90846622;     NoJeZtCOKPnixppDB90846622 = NoJeZtCOKPnixppDB90605740;     NoJeZtCOKPnixppDB90605740 = NoJeZtCOKPnixppDB99983824;     NoJeZtCOKPnixppDB99983824 = NoJeZtCOKPnixppDB25330823;     NoJeZtCOKPnixppDB25330823 = NoJeZtCOKPnixppDB87059418;     NoJeZtCOKPnixppDB87059418 = NoJeZtCOKPnixppDB63991631;     NoJeZtCOKPnixppDB63991631 = NoJeZtCOKPnixppDB98730845;     NoJeZtCOKPnixppDB98730845 = NoJeZtCOKPnixppDB56743941;     NoJeZtCOKPnixppDB56743941 = NoJeZtCOKPnixppDB98708061;     NoJeZtCOKPnixppDB98708061 = NoJeZtCOKPnixppDB81968412;     NoJeZtCOKPnixppDB81968412 = NoJeZtCOKPnixppDB33473060;     NoJeZtCOKPnixppDB33473060 = NoJeZtCOKPnixppDB6635850;     NoJeZtCOKPnixppDB6635850 = NoJeZtCOKPnixppDB29700187;     NoJeZtCOKPnixppDB29700187 = NoJeZtCOKPnixppDB55104276;     NoJeZtCOKPnixppDB55104276 = NoJeZtCOKPnixppDB42796151;     NoJeZtCOKPnixppDB42796151 = NoJeZtCOKPnixppDB99029144;     NoJeZtCOKPnixppDB99029144 = NoJeZtCOKPnixppDB37231315;     NoJeZtCOKPnixppDB37231315 = NoJeZtCOKPnixppDB1619156;     NoJeZtCOKPnixppDB1619156 = NoJeZtCOKPnixppDB43185083;     NoJeZtCOKPnixppDB43185083 = NoJeZtCOKPnixppDB20695644;     NoJeZtCOKPnixppDB20695644 = NoJeZtCOKPnixppDB59175736;     NoJeZtCOKPnixppDB59175736 = NoJeZtCOKPnixppDB93150598;     NoJeZtCOKPnixppDB93150598 = NoJeZtCOKPnixppDB26086813;     NoJeZtCOKPnixppDB26086813 = NoJeZtCOKPnixppDB10516894;     NoJeZtCOKPnixppDB10516894 = NoJeZtCOKPnixppDB48025765;     NoJeZtCOKPnixppDB48025765 = NoJeZtCOKPnixppDB83746292;     NoJeZtCOKPnixppDB83746292 = NoJeZtCOKPnixppDB49588902;     NoJeZtCOKPnixppDB49588902 = NoJeZtCOKPnixppDB41015210;     NoJeZtCOKPnixppDB41015210 = NoJeZtCOKPnixppDB82081822;     NoJeZtCOKPnixppDB82081822 = NoJeZtCOKPnixppDB47987338;     NoJeZtCOKPnixppDB47987338 = NoJeZtCOKPnixppDB26756106;     NoJeZtCOKPnixppDB26756106 = NoJeZtCOKPnixppDB47110159;     NoJeZtCOKPnixppDB47110159 = NoJeZtCOKPnixppDB45070258;     NoJeZtCOKPnixppDB45070258 = NoJeZtCOKPnixppDB62274368;     NoJeZtCOKPnixppDB62274368 = NoJeZtCOKPnixppDB39718311;     NoJeZtCOKPnixppDB39718311 = NoJeZtCOKPnixppDB58656276;     NoJeZtCOKPnixppDB58656276 = NoJeZtCOKPnixppDB12786555;     NoJeZtCOKPnixppDB12786555 = NoJeZtCOKPnixppDB19959552;     NoJeZtCOKPnixppDB19959552 = NoJeZtCOKPnixppDB27786718;     NoJeZtCOKPnixppDB27786718 = NoJeZtCOKPnixppDB84280251;     NoJeZtCOKPnixppDB84280251 = NoJeZtCOKPnixppDB85515284;     NoJeZtCOKPnixppDB85515284 = NoJeZtCOKPnixppDB22137966;     NoJeZtCOKPnixppDB22137966 = NoJeZtCOKPnixppDB13633628;     NoJeZtCOKPnixppDB13633628 = NoJeZtCOKPnixppDB76967461;}
// Junk Finished
