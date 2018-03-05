// ImGui - standalone example application for DirectX 9
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include <d3dx9.h>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <regex>
#include <string>
#include <vector>





using namespace std;
// Data
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            g_d3dpp.BackBufferWidth  = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
            if (hr == D3DERR_INVALIDCALL)
                IM_ASSERT(0);
            ImGui_ImplDX9_CreateDeviceObjects();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

struct ExampleAppConsole2					// LOG STARTS HERE, ISSUES :(
{
    char                  InputBuf[256];
    ImVector<char*>       Items;
    bool                  ScrollToBottom;
    ImVector<char*>       History;
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    ImVector<const char*> Commands;

    ExampleAppConsole2()
    {
    	
    	
		ClearLog();
        memset(InputBuf, 0, sizeof(InputBuf));
        HistoryPos = -1;
        AddLog(" ");
        AddLog("Issues putting logs into me :(");
        AddLog("Google me for source help.");
    	AddLog("Clipper needs setting up.");
    	AddLog("Could be API Issue.");
    	AddLog("Window works perfect though :)");
    	AddLog("Max Char length = 45");
    	AddLog("Lots of code rewritten..");
    	AddLog("Check source.");
    	AddLog("Playing with pictures");
    	AddLog("Check about source.");
    	AddLog("For basic UI have -");
    	AddLog("Low level pictures.");
    	AddLog("Grab W3 Icons. ");
    	AddLog("Need em loads");
    	AddLog("SEE NOTES ON ADDING PICS");
    	
	}
        

    
    ~ExampleAppConsole2()
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
    	
    	ImGui::SetNextWindowPos(ImVec2(0, 640));    
		ImGui::SetNextWindowSize(ImVec2(355,120)); 
		if (!ImGui::Begin(title, p_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
        {
            ImGui::End();
            return;
        }

        if (ImGui::Button("Clear")) { ClearLog(); } ImGui::SameLine(); 
        bool copy_to_clipboard = ImGui::Button("Copy"); ImGui::SameLine();
        if (ImGui::Button("Scroll to bottom")) ScrollToBottom = true;
        ImGui::Separator();
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
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
        
        
        ImGui::End();
    }

    
    
    
};													// LOG ENDS HERE
static LPDIRECT3DTEXTURE9       avatar = NULL;		// image idents here. here =)
static LPDIRECT3DTEXTURE9       d20 = NULL;


static void ShowExampleAppConsole2(bool* p_open)	// DRAW LOG
{
    static ExampleAppConsole2 console2;
    console2.Draw("Example: Console2", p_open);
}

int main(int, char**)
{
    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, _T("ImGui Example"), NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindow(_T("ImGui Example"), _T("ImGui DirectX9 Example"), WS_OVERLAPPEDWINDOW, 0, 0, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    LPDIRECT3D9 pD3D;
    if ((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
    {
        UnregisterClass(_T("ImGui Example"), wc.hInstance);
        return 0;
    }
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Present with vsync

    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // Present without vsync, maximum unthrottled framerate

    // Create the D3DDevice
    if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
    {
        pD3D->Release();
        UnregisterClass(_T("ImGui Example"), wc.hInstance);
        return 0;
    }

    // Setup ImGui binding
    ImGui_ImplDX9_Init(hwnd, g_pd3dDevice);

    // Setup style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them. 
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple. 
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);
    bool load_sheet_window = false;
    bool show_demo_window = false;
    bool about = false;
    bool show_another_window = false;
    bool ShowLogOutput = false;
    bool show_app_main_menu_bar = true;
    bool ShowExampleMenuFile = false;
    bool ShowCharacterScreen = false;
    bool show_app = false;
    bool show_char_port = true; /// inside char create screen leave on...
    bool test_window = false;
    bool show_ui = false;
    bool char_port = false;
    bool exit = false;
    bool load_image_about = false;
	static bool show_app_console2 = false;
	bool hermetic_cookbook = false;
	
	bool game_start_options = true;
	bool import_image_from_file = false;
 
	
	
	
    ImVec4 clear_color = ImVec4(0.073f, 0.115f, 0.177f, 1.000f); //RGBA colour for screen. place holder picture me asap. i still need music.
	D3DXCreateTextureFromFile( g_pd3dDevice, "d20.png" , &d20 );			// image file grabs here :)
	D3DXCreateTextureFromFile( g_pd3dDevice, "smile.png" , &avatar );	 
	
	
    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
    while (msg.message != WM_QUIT)
    {
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }									   ///////////////////////////////////////////
        ImGui_ImplDX9_NewFrame();             // IT ALL STARTS HERE
		
		
		
		
		if (game_start_options)
				ImGui::OpenPopup("Choose from below");
            if (ImGui::BeginPopupModal("Choose from below", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
            {
				if (ImGui::Button("NEW GAME",  ImVec2(120,0))) { ImGui::CloseCurrentPopup(); game_start_options = false; ShowCharacterScreen = true;}
                if (ImGui::Button("DEV MODE", ImVec2(120,0))) { ImGui::CloseCurrentPopup(); game_start_options = false; show_app = true; show_ui = true;
				show_app_main_menu_bar = true; show_app_console2 = true;}
               
                if (ImGui::Button("EXIT", ImVec2(120,0))) { ImGui::CloseCurrentPopup(); game_start_options = false; goto Shutdown; }
                ImGui::EndPopup();
            }
			
		
		
		
		
		
		
												
        if (show_app_main_menu_bar)
        {
        	ImGui::BeginMainMenuBar();
    	}
         if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("New Game", "CTRL+N", &ShowCharacterScreen);
            ImGui::MenuItem("Save Game", "CTRL+S"); 
            ImGui::Separator();
            ImGui::MenuItem("Load Sheet", "CTRL+W", &load_sheet_window);
            ImGui::MenuItem("Load Game", "CTRL+L");
            ImGui::MenuItem("Options", "CTRL+O");
            ImGui::MenuItem("Exit","CTRL+E",  &exit);
            if (exit)
            {
            	goto Shutdown;	
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
				{
					ImGui::MenuItem("About", "CTRL+A", &about);
					ImGui::MenuItem("Import image", "CTRL+b", &import_image_from_file);
					ImGui::MenuItem("About import image", "CTRL+c", &load_image_about);
					
					ImGui::EndMenu();
				}
            
        
        
        ImGui::EndMainMenuBar();	
		
		
		if (about)
		{	
		
			ImGui::Begin("About", &about);
            ImGui::Text("");
            ImGui::Text("Dear ImGui, %s", ImGui::GetVersion());
            ImGui::Text("");
			ImGui::Text("By Luke Hays");
            ImGui::Text("");
			ImGui::Text("Written in C++, using Dev ++ with the TDM Mingw64 compiler.");
            ImGui::Text("Useing IMGUI API, and OGRE 3D graphics engine.");
            ImGui::Text("");
            ImGui::Text("Easter eggs to add:");
            ImGui::Text("get system time and at 00:00");
            ImGui::Text("Make a refrence to the witching hour from dungeon keeper 2");
            ImGui::Text("");
            ImGui::Text("");
            //ImGui::Image();
            //ImGui::ImageButton(), 
			//ImDrawList::AddImage()
			//ExampleAppConsole2::AddLog("hello"); // still having issues, google me.
            ImGui::End();
        }
        
        
        
		if (show_app)
				{
			
			ImGui::SetNextWindowPos(ImVec2(435, 131));    
			ImGui::SetNextWindowSize(ImVec2(400,300)); 
			ImGui::Begin("Window manager.", &show_app);
            ImGui::Text("");
            ImGui::Text("Window skipper..");
            ImGui::Text("");
            ImGui::Checkbox("Character sheet", &ShowCharacterScreen);      // Edit bools storing our windows open/close state
            ImGui::Checkbox("Load character sheet", &load_sheet_window);
            ImGui::Checkbox("Alchemy set", &hermetic_cookbook);
			ImGui::Checkbox("Show UI", &show_ui);
            ImGui::Text("");
            
            ImGui::Text("");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        	
			
         							
			//ImGui::ShowMetricsWindow();
			ImGui::End();
				}

				if (hermetic_cookbook)    // no doubt i am going to have to put this in the main game loop.
				{
				ImGui::SetNextWindowPos(ImVec2(0, 131));    
				ImGui::SetNextWindowSize(ImVec2(250,400)); 
				ImGui::Begin("Cookbook Menu", &hermetic_cookbook, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            	ImGui::Text("Mouse over for details");
				ImGui::Separator();
				ImGui::Text("");
				if (ImGui::TreeNode("The Hermetic cook book"))
            	{
            		static int i = 1;
                    if (ImGui::TreeNode((void*)(intptr_t)i, "Chemical Details", i))
                    {
                    	
                    	ImGui::Text("> The colors"); 
						if (ImGui::IsItemHovered())
            			{
                		ImGui::SetNextWindowPos(ImVec2(255, 131));    
						ImGui::SetNextWindowSize(ImVec2(300,300));
						ImGui::BeginTooltip();
						ImGui::Text("Red");ImGui::SameLine(60);ImGui::Text("= Damage");
						ImGui::Text("Yellow");ImGui::SameLine(60);ImGui::Text("= Duration");
						ImGui::Text("Green");ImGui::SameLine(60);ImGui::Text("= Damage Per Second");
						ImGui::Text("Blue");ImGui::SameLine(60);ImGui::Text("= Area of Effect");
						ImGui::Text("Pink");ImGui::SameLine(60);ImGui::Text("= Infectiousness");
						ImGui::Text("");
						ImGui::Text("Experiment to discover special recipes. ");
						ImGui::Text("");
						ImGui::Text("Special recipes have better effects");
						ImGui::Text("and bigger bonuses.");
						ImGui::EndTooltip(); 
						}
                        ImGui::TreePop();
                    }
               		static int j = 6; 
                    if (ImGui::TreeNode((void*)(intptr_t)j, "Recipes", j))
                    {
                        ImGui::TextDisabled("> Recipe 1 (?)");
                        if (ImGui::IsItemHovered())
            			{
                		ImGui::SetNextWindowPos(ImVec2(255, 131));    
						ImGui::SetNextWindowSize(ImVec2(300,300));
						ImGui::BeginTooltip();
						ImGui::Text("Discover me - Discovery tip 1");
						ImGui::EndTooltip(); 
						}
                        ImGui::TextDisabled("> Recipe 2 (?)");
                        if (ImGui::IsItemHovered())
            			{
                		ImGui::SetNextWindowPos(ImVec2(255, 131));    
						ImGui::SetNextWindowSize(ImVec2(300,300));
						ImGui::BeginTooltip();
						ImGui::Text("Discover me - Discovery tip 2");
						ImGui::EndTooltip(); 
						}
                        ImGui::TextDisabled("> Recipe 3 (?)");
                        if (ImGui::IsItemHovered())
            			{
                		ImGui::SetNextWindowPos(ImVec2(255, 131));    
						ImGui::SetNextWindowSize(ImVec2(300,300));
						ImGui::BeginTooltip();
						ImGui::Text("Discover me - Discovery tip 3");
						ImGui::EndTooltip(); 
						}
						ImGui::TextDisabled("> Recipe 4 (?)");
                        if (ImGui::IsItemHovered())
            			{
                		ImGui::SetNextWindowPos(ImVec2(255, 131));    
						ImGui::SetNextWindowSize(ImVec2(300,300));
						ImGui::BeginTooltip();
						ImGui::Text("Discover me - Discovery tip 4");
						ImGui::EndTooltip(); 
						}
						ImGui::TextDisabled("> Recipe 5 (?)");
                        if (ImGui::IsItemHovered())
            			{
                		ImGui::SetNextWindowPos(ImVec2(255, 131));    
						ImGui::SetNextWindowSize(ImVec2(300,300));
						ImGui::BeginTooltip();
						ImGui::Text("Discover me - Discovery tip 5");
						ImGui::EndTooltip(); 
						}
						ImGui::TextDisabled("> Recipe 6 (?)");
						if (ImGui::IsItemHovered())
            			{
                		ImGui::SetNextWindowPos(ImVec2(255, 131));    
						ImGui::SetNextWindowSize(ImVec2(300,300));
						ImGui::BeginTooltip();
						ImGui::Text("Discover me - Discovery tip 6");
						ImGui::EndTooltip(); 
						}
						ImGui::TreePop();
                    	}
                    	static int k = 3;
                    	if (ImGui::TreeNode((void*)(intptr_t)k, "Crafting Traps & Devices", k))
                    	{
                        ImGui::Text("> Spike trap");
						if (ImGui::IsItemHovered())
            			{
                		ImGui::SetNextWindowPos(ImVec2(255, 131));    
						ImGui::SetNextWindowSize(ImVec2(300,300));
						ImGui::BeginTooltip();
						ImGui::Text("Required items");
						ImGui::Text(" ");
						ImGui::Text("1x");ImGui::SameLine(50);ImGui::Text("Box of nails");
						ImGui::Text("1x");ImGui::SameLine(50);ImGui::Text("Board");
						ImGui::Text("1x");ImGui::SameLine(50);ImGui::Text("Big spring");
						ImGui::Text("4x");ImGui::SameLine(50);ImGui::Text("string");
						ImGui::Text("");
						ImGui::Text("Can be placed on the ground.");
						ImGui::Text("Trigged when stepped on.");
						ImGui::EndTooltip(); 
						}
						ImGui::TextDisabled("> Trap ## (?)");
						if (ImGui::IsItemHovered())
            			{
                		ImGui::SetNextWindowPos(ImVec2(255, 131));    
						ImGui::SetNextWindowSize(ImVec2(300,300));
						ImGui::BeginTooltip();
						ImGui::Text("Discover me - Discovery tip 7");
						ImGui::EndTooltip(); 
						}
						ImGui::TreePop();
                    }
                ImGui::TreePop();
            	}
            	ImGui::End();
            	}
			if (hermetic_cookbook)
					{
							
			ImGui::SetNextWindowPos(ImVec2(740, 131));    
			ImGui::SetNextWindowSize(ImVec2(500,400)); 
			ImGui::Begin("Alchemy", &hermetic_cookbook, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::Text("");
            ImGui::Text("Toy idea for the Hermetic class. WIP BUGGY");
            ImGui::Text("");
            ImGui::Text("Using different color chemicals to produce a concoction,");
            ImGui::Text("to throw at an opponent or object. ");
            ImGui::Text("");
            ImGui::Text("Move the sliders to determine how much of a chemical you put in.");
            ImGui::Text("Cook book and details to left.");
            ImGui::End();
            		}
            if(hermetic_cookbook)
			{
			ImGui::SetNextWindowPos(ImVec2(500, 131));    
			//ImGui::SetNextWindowSize(ImVec2(500,400)); 
			ImGui::Begin("Alchemy kit", &hermetic_cookbook, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::Text("");
            static int damage;
            static int damage_i;
            ImGui::PushID(damage);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(7.0f, 0.5f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(7.0f, 0.6f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(7.0f, 0.7f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(7.0f, 0.9f, 0.9f));
            ImGui::VSliderInt("##damage", ImVec2(18,160), &damage_i, 0, 100," ");
            ImGui::PopStyleColor(4);
            ImGui::PopID();
            static int duration;
            static int duration_i;
            ImGui::PushID(duration);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(0.161f, 0.852f, 0.781f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(0.161f, 0.900f, 0.781f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(0.161f, 0.900f, 0.781f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(0.161f, 0.900f, 0.981f));
            ImGui::SameLine();ImGui::VSliderInt("##duration", ImVec2(18,160), &duration_i, 0, 100, " ");
            ImGui::PopStyleColor(4);
            ImGui::PopID();
			static int dps;
            static int dps_i;
            ImGui::PushID(dps);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(0.277f, 0.926f, 0.829f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(0.277f, 0.970f, 0.829f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(0.277f, 0.970f, 0.829f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(0.277f, 0.970f, 0.981f));
            ImGui::SameLine();ImGui::VSliderInt("##dps", ImVec2(18,160), &dps_i, 0, 100, " ");
            ImGui::PopStyleColor(4);
            ImGui::PopID();
			static int aoe;
            static int aoe_i;
            ImGui::PushID(aoe);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(0.645f, 0.926f, 0.829f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(0.645f, 0.970f, 0.829f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(0.645f, 0.970f, 0.829f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(0.645f, 0.970f, 0.981f));
            ImGui::SameLine();ImGui::VSliderInt("##aoe", ImVec2(18,160), &aoe_i, 0, 100, " ");
            ImGui::PopStyleColor(4);
            ImGui::PopID();
			static int sticky;
            static int sticky_i;
            ImGui::PushID(sticky);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(0.848f, 0.926f, 0.829f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(0.848f, 0.970f, 0.829f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(0.848f, 0.970f, 0.829f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(0.848f, 0.970f, 0.981f));
            ImGui::SameLine();ImGui::VSliderInt("##sticky", ImVec2(18,160), &sticky_i, 0, 100, " ");
            ImGui::PopStyleColor(4);
            ImGui::PopID();
            
            static int beaker_total = 0;
            beaker_total = damage_i + duration_i + dps_i + aoe_i + sticky_i;
            ImGui::Text("");
            if (beaker_total >= 100){beaker_total = 100;}
			ImGui::Text("");
			ImGui::Text("Capacity %d%%", beaker_total);
			ImGui::Text("");
			ImGui::Text("");ImGui::SameLine(40);ImGui::Button("Concoct");
			
			
			
			ImGui::End();
			
				}
		
		if (load_sheet_window)					// quick and easy to follow & fix, need a look up table, for class,  
												
			{									
			static int load_check = 2;			// Integer load checking .. sorry ^^
			static char first_name_load[40];				
			static char last_name_load[40];
			static int sex_load = 10;
			static int abnormalities_load = 10;
			static int Class_s_load = 10;
			static int Special_flag_load = 10;
			static int Difficulty_load = 10;
			static int Essence_load = 0;
			static int Dexterity_load = 0;
			static int Health_load = 0;
			static int Fatigue_load = 0;
			static int Insanity_res_load = 0;
			static int skill_points_spent_load = 0;
			static int skill_blah_load = 0;
			static int skill_blah_q_load = 0;
			static int skill_blah_w_load = 0;
			static int skill_blah_e_load = 0;
			static int skill_blah_r_load = 0;
			static int skill_blah_t_load = 0;
			static int skill_blah_y_load = 0;
			static int skill_blah_u_load = 0;
			static int skill_blah_i_load = 0;
			
			ImGui::SetNextWindowPos(ImVec2(308,221));    
			ImGui::SetNextWindowSize(ImVec2(664,410)); 
			ImGui::Begin("Load a character sheet.", &load_sheet_window);
            ImGui::Text("");
            ImGui::Text("Just to test loading and basic character sheet nothing more.");
            ImGui::Text("");
			ImGui::Text("Check source code for details, using regex to filter info.");
			ImGui::Text("");
			ImGui::Text("Create a character and then type the characters first name and include any capitals ");
            ImGui::Text("or load example 'Lisa'. This is a work in progress.");
            ImGui::Text(" ");
            ImGui::PushItemWidth(200);
            static char load_file[40] = "Type filename here...";    // setup text filtering, could be in latest update.
            string load_name = load_file;							
			ImGui::InputText(" - type the filename without extension and click load button.", load_file, IM_ARRAYSIZE(load_file), ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll);
            ImGui::PopItemWidth();
            if(ImGui::Button("LOAD"))
            {
            	
            	ifstream file(load_name.c_str(), ios::in|ios::ate);
            			if (file.is_open())
            				{						// Setup directories. '%DIR%\Saves\Charactername\'
							load_check = 2;			// Reminder write an actual header and cpp file save/load feature HAHAHAHA!
            				streampos size;			// regex "clean words" only produces numbers,
					  		char * memblock;		// regex "clean numbers" only produces words.
							size = file.tellg();
							memblock = new char [size];
							file.seekg (0, ios::beg);
							file.read (memblock, size);
							regex number("((\\+|-)?[[:digit:]]+)(\\.(([[:digit:]]+)?))?((e|E)((\\+|-)?)[[:digit:]]+)?");
						    regex word("[[:alpha:]]+");
						    string clean_words, clean_numbers;
						    const string format="";
						    clean_numbers=regex_replace(memblock,number,format,regex_constants::format_default);
						    clean_words=regex_replace(memblock,word,format,regex_constants::format_default);
						    stringstream load_values(clean_words);
						    stringstream load_fullname(clean_numbers);
						    char first[40];	load_fullname	>> first_name_load;		//load_fullname produces words 
						    char last[40];	load_fullname 	>> last_name_load;
							int qq = 0; 	load_values 	>> sex_load;			// load_values produces numbers
							int zz = 0;		load_values 	>> abnormalities_load;
							int yy = 0;		load_values 	>> Class_s_load;
							int ww = 0;		load_values 	>> Special_flag_load;			
							int pp = 0;		load_values 	>> Difficulty_load;
							int aa = 0;		load_values 	>> Essence_load;
							int bb = 0;		load_values 	>> Dexterity_load;
							int qn = 0;		load_values		>> Health_load;
							int cc = 0;		load_values 	>> Fatigue_load;
							int dd = 0;		load_values 	>> Insanity_res_load;
							int vv = 0;		load_values		>> skill_points_spent_load;
							int tt = 0;		load_values		>> skill_blah_load;
							int qw = 0;		load_values		>> skill_blah_q_load;
							int qe = 0;		load_values		>> skill_blah_w_load;
							int qr = 0;		load_values		>> skill_blah_e_load;
							int qy = 0;		load_values		>> skill_blah_r_load;
							int qu = 0;		load_values		>> skill_blah_t_load;
							int qi = 0;		load_values		>> skill_blah_y_load;
							int qo = 0;		load_values		>> skill_blah_u_load;
							int qp = 0;		load_values		>> skill_blah_i_load;
							file.close();
							}
							else {load_check = 1;} 
							}
			if(load_check == 1){ImGui::SameLine();ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "- FAILED! incorrect filename check directory of this .exe.");}				
			ImGui::Text("I could also call io.imgui and get the user to press enter..");
			ImGui::Text("");
            ImGui::Separator();
			ImGui::Text("");
			ImGui::Text("Character Profile.");
			ImGui::Text("");
			ImGui::Text("First name: %s", first_name_load);ImGui::SameLine(200);ImGui::Text("Last name: %s", last_name_load);
			ImGui::Text("Sex: ");
			if(sex_load == 1){ImGui::SameLine();ImGui::Text("Female");}
			if(sex_load == 0){ImGui::SameLine();ImGui::Text("Male");}
			ImGui::Text("");
			ImGui::Text("Essence: %d", Essence_load);ImGui::SameLine(120);ImGui::Text("Dexterity: %d", Dexterity_load);
			ImGui::Text("Health:  %d", Health_load);ImGui::SameLine(120);ImGui::Text("Fatigue:   %d", Fatigue_load);
			ImGui::Text("Insanity Resistence: %d%%", Insanity_res_load);
			ImGui::Text("");
			switch (Class_s_load)
			{
			
				case 0:	{ImGui::Text("School of thought:	Hermetic Philosophy");break;}
				case 1:	{ImGui::Text("School of thought:	Necromancy");break;}
				case 2:	{ImGui::Text("School of thought:	Elementalism");break;}
				case 3:	{ImGui::Text("School of thought:	Illusion");break;}
				case 4:	{ImGui::Text("School of thought:	Lycanthropy");break;}
				
			}
			switch (abnormalities_load)
			{
				case 0:{ImGui::SameLine();ImGui::Text("			Abnormality:	None");break;}
				case 1:{ImGui::SameLine();ImGui::Text("			Abnormality:	Nocturnal");break;}
				case 2:{ImGui::SameLine();ImGui::Text("			Abnormality:	Deviant Tastes");break;}
				case 3:{ImGui::SameLine();ImGui::Text("			Abnormality:	Lygophobia");break;}
				case 4:{ImGui::SameLine();ImGui::Text("			Abnormality:	Egomania");break;}
				case 5:{ImGui::SameLine();ImGui::Text("			Abnormality:	Necromania");break;}
			}
			ImGui::Text("");
			switch (Special_flag_load)
			{
				case 0:{ImGui::Text("Current effects:				");break;}
				case 1:{ImGui::Text("Current effects:	Vampirism");break;}
				case 2:{ImGui::Text("Current effects:	Possessed");break;}
				case 3:{ImGui::Text("Current effects:	Warewolf");break;}
				case 4:{ImGui::Text("Current effects:	Pyromaniac");break;}
				case 5:{ImGui::Text("Current effects:	Chemical Addiction");break;}
				case 6:{ImGui::Text("Current effects:	Stalker");break;}
				case 7:{ImGui::Text("Current effects:	Void Walker");break;}
			}
			ImGui::Text("");
			ImGui::Separator();
			ImGui::Text("");
			ImGui::Text("Skill points spent: %d",skill_points_spent_load);
			ImGui::Text("");
			ImGui::Text("Skills");
			ImGui::Text("");
			ImGui::Text("Scholarly pursuits    %d  - Skill needed for old writings.", skill_blah_load);
            ImGui::Text("Antiquarian studies   %d  - Is that item magical or not?", skill_blah_q_load);
            ImGui::Text("Astrology             %d  - Stars, signs and prophecy.", skill_blah_w_load);            
            ImGui::Text("Medium                %d  - Even the dead gossip.", skill_blah_e_load);
            ImGui::Text("Occultist knowledge   %d  - More then a good read?", skill_blah_r_load);
            ImGui::Text("Tarot reading         %d  - Ace up your sleeve?", skill_blah_t_load);
            ImGui::Text("Knowledge             %d  - Clues to a puzzle.", skill_blah_y_load);
            ImGui::Text("Learning              %d  - A candle and a good book.", skill_blah_u_load);
            ImGui::Text("Archaeology           %d  - Tomb or something sinister?", skill_blah_i_load);
            ImGui::Text("");
			ImGui::Text("");
			ImGui::End();
				
			}
		
		
			if (import_image_from_file)
				ImGui::OpenPopup("Import image:");
            if (ImGui::BeginPopupModal("Import image:", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
				static char picture_name[40] = "mynameforfile.jpg";
				ImGui::PushItemWidth(248);
				ImGui::InputText("#####dummy ", picture_name, IM_ARRAYSIZE(picture_name), ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll);
            	ImGui::PopItemWidth();
				if (ImGui::Button("LOAD",  ImVec2(120,0))) { ImGui::CloseCurrentPopup(); import_image_from_file = false; }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120,0))) { ImGui::CloseCurrentPopup(); import_image_from_file = false; }
                ImGui::EndPopup();
            }
			
			if (load_image_about)
				{
				ImGui::SetNextWindowPos(ImVec2(400, 200));
				ImGui::SetNextWindowSize(ImVec2(300,310));
				ImGui::Begin("Import an image from file:", &load_image_about);
            	ImGui::Text("");
            	ImGui::Text("1, Images must be 100x100 pixel");
            	ImGui::Text("   RGB 16 BIT, .JPEG");
				ImGui::Text("   Lower case names.");
				ImGui::Text("");
				ImGui::Text("2, Place the .jpeg into the");
				ImGui::Text("   Portraits folder.");
            	ImGui::Text("");
            	ImGui::Text("3, Image name must not contain:");
            	ImGui::Text("   Numbers in file name.");
				ImGui::Text("   Specail charecters.");
				ImGui::Text("   or spaces.");
				ImGui::Text("");
				ImGui::Text("   e.g 'mynameforfile.jpg'");
				ImGui::Text("");
				ImGui::Text("   Will import correctly.");
				ImGui::End();
				}
        // 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow(). Read its code to learn more about Dear ImGui!
        if (show_demo_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&show_demo_window);
        }
        
		if (show_ui)
				 {
				if (show_ui)
				 	
					{
			ImGui::SetNextWindowPos(ImVec2(977, 20));    
			ImGui::SetNextWindowSize(ImVec2(285,178)); 
			ImGui::Begin("MINIMAP.", &show_ui, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::Text("Minimap");
            ImGui::Text("clickable.");
			ImGui::End();
					
					}
					if(show_ui) ShowExampleAppConsole2(&show_app_console2);
					if(show_ui)
					{
					
        	ImGui::SetNextWindowPos(ImVec2(977, 200));    
			ImGui::SetNextWindowSize(ImVec2(285,560));
			ImGui::Begin("map and other ui elements.", &show_ui, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::Text("");
            ImGui::Text("Charactar loadout.");
            ImGui::Text("on hand, worn, books.");
            ImGui::Text("");
            ImGui::End();
            		}
        		if (show_ui)
            	{
            ImGui::SetNextWindowPos(ImVec2(0, 20));    
			ImGui::SetNextWindowSize(ImVec2(105,105)); 
			ImGui::Begin("Picture.", &show_ui, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::Image((void *)avatar, ImVec2(85, 85), ImVec2(0,0),ImVec2(1,1),ImVec4(255,255,255,255),ImVec4(0,0,0,0));
			ImGui::End();
				}
				{
            ImGui::SetNextWindowPos(ImVec2(0, 125));
			ImGui::SetNextWindowSize(ImVec2(105,50)); 
			ImGui::Begin("health/mana.", &show_ui, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
			ImGui::Text("Health:");
            ImGui::Text("Mana:");
			ImGui::End();
				}
				if (show_ui)
       			{
        	ImGui::SetNextWindowPos(ImVec2(360, 20));    
			ImGui::SetNextWindowSize(ImVec2(500,100)); 
			ImGui::Begin("Place holder for status effects", &show_ui, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::Text("");
            ImGui::Text("Place holder for status effects.");
            ImGui::End();
				}
				if (show_ui)
       			{
        	ImGui::SetNextWindowPos(ImVec2(357, 640));    
			ImGui::SetNextWindowSize(ImVec2(618,120)); 
			ImGui::Begin("skills,explore and combat.", &show_ui, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            static int Tools = 0;
    		ImGui::PushItemWidth(100);ImGui::Combo("####dummy",&Tools,"Explore\0Spells\0Inventory\0", 3);ImGui::PopItemWidth(); // switch and case use for class specific icons.  
			ImGui::Separator();
			if(Tools == 0)
			{
			ImGui::ImageButton((void *)avatar, ImVec2(35, 35), ImVec2(0,0),ImVec2(1,1),0,ImVec4(0,0,0,0),ImVec4(255,255,255,255));  
			ImGui::SameLine();
			ImGui::ImageButton((void *)d20, ImVec2(35, 35), ImVec2(0,0),ImVec2(1,1),0,ImVec4(0,0,0,0),ImVec4(255,255,255,255)); 
			ImGui::SameLine();
			ImGui::ImageButton((void *)avatar, ImVec2(35, 35), ImVec2(0,0),ImVec2(1,1),0,ImVec4(0,0,0,0),ImVec4(255,255,255,255)); 
			ImGui::SameLine();
			ImGui::ImageButton((void *)d20, ImVec2(35, 35), ImVec2(0,0),ImVec2(1,1),0,ImVec4(0,0,0,0),ImVec4(255,255,255,255)); 
			ImGui::SameLine();
			ImGui::ImageButton((void *)avatar, ImVec2(35, 35), ImVec2(0,0),ImVec2(1,1),0,ImVec4(0,0,0,0),ImVec4(255,255,255,255)); 
			ImGui::Text("Going to add a switch and case for each class / Special flag to acquire required icons.");
			}
			if(Tools == 1){ImGui::Text("Spells/combat");}
			if(Tools == 2){ImGui::Text("Inventory");}
			
			
            ImGui::End();
				}
				}
					
        if (ShowCharacterScreen)
		{		ImGui::OpenPopup("dummymechar");
            if (ImGui::BeginPopupModal("dummymechar", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
           
			//ImGui::SetNextWindowSize(ImVec2(750,700));
			//ImGui::SetNextWindowPos(ImVec2(280, 20));
			//ImGui::Begin("Roll me up!", &ShowCharacterScreen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
			ImGui::Text("");
			ImGui::SameLine(305);
			if (show_char_port)
            {
			
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
            ImGui::BeginChild("Picture", ImVec2(105,105), true);
            ImGui::EndChild();
            ImGui::PopStyleVar();
            }
        	ImGui::Text("");
        	ImGui::SameLine(305);
        	if (ImGui::Button("<--")){
			}
			ImGui::SameLine(381);
			if (ImGui::Button("-->")){
			}
			
			ImGui::Separator();
			ImGui::Text("");
			ImGui::Text("");
			ImGui::SameLine(262);
            ImGui::PushItemWidth(200);
            static char first_name[40] = "First name...";    // Add filter character check - and player reminder 
            string save_name = first_name;
			ImGui::InputText(" - 40 Charaters Max", first_name, IM_ARRAYSIZE(first_name), ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll);
            ImGui::PopItemWidth();
            ImGui::Text("");
			ImGui::SameLine(262);
            ImGui::PushItemWidth(200);
            static char last_name[40] = "Last name...";
            ImGui::InputText("   First/Last name. ", last_name, IM_ARRAYSIZE(last_name), ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll);
			ImGui::PopItemWidth();    // Add filter character check - and player reminder
            ImGui::Text("");
            ImGui::Separator();
			ImGui::Text("");
			ImGui::Text("Sex:");ImGui::SameLine(160);ImGui::Text("Abnormalities:");
            static int Sex = 0;
            static int abnormalities = 0;
            static int Class_s = 0;
            ImGui::RadioButton("Male", &Sex, 0);ImGui::SameLine(160);ImGui::RadioButton("None", &abnormalities, 0);ImGui::SameLine(280);ImGui::RadioButton("Deviant Tastes", &abnormalities, 2);ImGui::SameLine(430);ImGui::RadioButton("Egomania", &abnormalities, 4);
            ImGui::RadioButton("Female", &Sex, 1);ImGui::SameLine(160);ImGui::RadioButton("Nocturnal", &abnormalities, 1);ImGui::SameLine(280);ImGui::RadioButton("Lygophobia", &abnormalities, 3);ImGui::SameLine(430);ImGui::RadioButton("Necromania", &abnormalities, 5);
            ImGui::Text("");
            ImGui::Text("Abnormalities have a give and take effect to the stats of a charecter. Adds new elements of game play.");
			ImGui::Text("Example, 'Nocturnal', And 'Necromancy' school of thought will produce a Necromancer with Vampirism. ");
			ImGui::Text("This will have an impact on the characters insanity resistence and behavior.");
			ImGui::Text("");
			ImGui::Text("Not all combinations have an effect but will alter gameplay, check current effect.");
			ImGui::Text("");
			ImGui::Text("New players its suggested you choose 'None'.");
			ImGui::Text("");
			ImGui::Text("Current effect: "); //:| Look into me. Those "if`s" :P build a look up table asap. or something better.
			int Special_flag = 0;
			if (Class_s == 1 & abnormalities == 1){ImGui::SameLine(125);ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "Vampirism");Special_flag = 1;};
			if (Class_s == 3 & abnormalities == 5){ImGui::SameLine(125);ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "Possessed");Special_flag = 2;;};
			if (Class_s == 4 & abnormalities == 1){ImGui::SameLine(125);ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "Warewolf");Special_flag = 3;};
			if (Class_s == 2 & abnormalities == 4){ImGui::SameLine(125);ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "Pyromaniac");Special_flag = 4;};
			if (Class_s == 0 & abnormalities == 4){ImGui::SameLine(125);ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "Chemical Addiction");Special_flag = 5;};
			if (Class_s == 3 & abnormalities == 1){ImGui::SameLine(125);ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "Stalker");Special_flag = 6;};
			if (Class_s == 2 & abnormalities == 1){ImGui::SameLine(125);ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "Void Walker");Special_flag = 7;};
			ImGui::Text("");
			ImGui::Separator();
            ImGui::Text("");
            ImGui::Text("School of thought: ");
            ImGui::Text("");
            ImGui::RadioButton("Hermetic Philosophy", &Class_s, 0);ImGui::SameLine(200);ImGui::Text("- The All, It gets deeper.");
            ImGui::RadioButton("Necromancy", &Class_s, 1);ImGui::SameLine(200);ImGui::Text("- Mum always said 'Make' new friends.");
            ImGui::RadioButton("Elementalism", &Class_s, 2);ImGui::SameLine(200);ImGui::Text("- It never rains, but it pours.");
            ImGui::RadioButton("Illusion", &Class_s, 3);ImGui::SameLine(200);ImGui::Text("- Have you ever seen a ghost?");
			ImGui::RadioButton("Lycanthropy", &Class_s, 4);ImGui::SameLine(200);ImGui::Text("- When the moon is full and bright.");
			ImGui::Text(""); 
            ImGui::Separator();
            ImGui::Text("");
            ImGui::Text("Riddles, journals, occultist manuscripts and tarot cards are the curreny of the game.");
            ImGui::Text("All will be stored in books you carry.");
            ImGui::Text("You start with a school book that can hold a few of the above mentioned.");
			ImGui::Text("However you can learn above menitioned and gain new abilities, insight and grey hairs.");
			ImGui::Text("");
			ImGui::Text("Tarot cards can be used like magical spells, however they have side effects.");
            ImGui::Text("Side effects can be lethal when insanity is at 100%%.");
            ImGui::Text("");
            ImGui::Separator();
            static int Difficulty = 0;
            ImGui::Text("");
            ImGui::Text("Who do you choose?");
            ImGui::Text("");
            ImGui::RadioButton("Sadistic   - AI", &Difficulty, 1);ImGui::SameLine(200);ImGui::Text("- Punishment and cruelty.");ImGui::SameLine(400);ImGui::Text("- Hard");
            ImGui::RadioButton("Nihilistic - AI", &Difficulty, 0);ImGui::SameLine(200);ImGui::Text("- Why even bother?");ImGui::SameLine(400);ImGui::Text("- Easy");
            ImGui::Text("");
            ImGui::Separator();
            int Essence;
            if (Essence > 30){ Essence = 0;}
            int Dexterity;
            if (Dexterity > 30){ Dexterity = 0;}
            int Health;
            if (Health > 30){ Health = 0;}
            int Fatigue;
            if (Fatigue > 30){ Fatigue = 0;}
            int Insanity_res;
            if (Insanity_res > 50){ Insanity_res = 0;}
            if (abnormalities >= 1){Insanity_res = 10;
			if (Special_flag >= 1 & abnormalities >= 1){Insanity_res = 30;}}else {Insanity_res = 0;}
			static int skill_points = 15;
			int skill_points_spent = 0;
            static int skill_blah = 0;
            static int skill_blah_q = 0;
            static int skill_blah_w = 0;
            static int skill_blah_e = 0;
            static int skill_blah_r = 0;
            static int skill_blah_t = 0;
            static int skill_blah_y = 0;
            static int skill_blah_u = 0;
            static int skill_blah_i = 0;
            ImGui::Text("");
            ImGui::Text("You need to roll stats, (15) + 6. Try to roll above ten in all stats.");
            ImGui::Text(" ");
            static ImVec2 size(50, 50);
            static int Reroll_me;
            ImGui::PushID(Reroll_me);                 // buttons with styles need IDs 
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.2f, 2.0f, 0.6f));      // Intresting feature play with me! :)
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(4.0f, 0.3f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(4.0f, 0.3f, 0.8f));
            //ImGui::ImageButton((void *)d20, ImVec2(50, 50), ImVec2(0,0),ImVec2(1,1),0,ImVec4(0,0,0,0),ImVec4(255,255,255,255)); // Shares the same style setting as button in text.... 
			//if (ImGui::IsItemActive())
            
			if (ImGui::Button("Reroll Stats"))   // add a dice sound effect to me.
                {
                	
                	srand(static_cast<unsigned int>(time(0)));
					int randomNumber = rand();
																
	             	Essence = 0;								
                	Dexterity = 0;							// I(k) = ( a * I(k-1) + c ) % m,// Minimal standard RNG (Park and Miller 1988)
					Health = 0;									 
                	Fatigue = 0;								
                	Essence = (randomNumber % 15) + 6;
                	randomNumber = randomNumber - 1;
            		Dexterity = (randomNumber % 15) + 6;
            		randomNumber = randomNumber - 1;
            		Health = (randomNumber % 15) + 6;
            		randomNumber = randomNumber - 1;
            		Fatigue = (randomNumber % 15) + 6;
            		 
            
            		
			}
			ImGui::PopStyleColor(3);
            ImGui::PopID();
			if (Essence <= 1 & Health <= 1 & Dexterity <= 1 & Fatigue <= 1 ){ImGui::SameLine(120);ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "- Click to roll me.");}
            ImGui::Text(" ");
            ImGui::Text("Energy, Mana.");ImGui::SameLine(120);ImGui::Text("Dodge.");ImGui::SameLine(250);ImGui::Text("The Curse of youth.");ImGui::SameLine(400);ImGui::Text("Run rabbit run.");ImGui::SameLine(550);ImGui::Text("Bewarned it will happen!");
            ImGui::Text("Essence: %d" , Essence);ImGui::SameLine(120);ImGui::Text("Dexterity: %d", Dexterity);ImGui::SameLine(250);ImGui::Text("Health: %d", Health);ImGui::SameLine(400);ImGui::Text("Fatigue: %d", Fatigue);ImGui::SameLine(550);ImGui::Text("Insanity Resistence: %d%%", Insanity_res);
            ImGui::Text(" ");
            ImGui::Separator();
            ImGui::Text(" ");
            ImGui::Text("'Grey hair' Points to spend: %d", skill_points);ImGui::SameLine(300);ImGui::Text("- Grey hairs are the skills currency, the more the better!");
            skill_points_spent = skill_blah + skill_blah_q + skill_blah_w + skill_blah_e + skill_blah_r + skill_blah_t + skill_blah_y + skill_blah_u + skill_blah_i;
			ImGui::Text("'Grey hair' Points used : %d", skill_points_spent);
            if (skill_points_spent < 15){ImGui::SameLine(300);ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "- Spend me! or save me!, but you will need me.");}
			if (skill_points_spent > 15){ImGui::SameLine(300);ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "- You have spent too many!");}
			if (skill_points_spent == 15){ImGui::SameLine(300);ImGui::TextColored(ImVec4(0.5f,1.0f,0.0f,1.0f), "- Skills setup!");}
			ImGui::Text("");
			ImGui::Text("'Grey hair' Skill list");
			ImGui::Text("");
            ImGui::PushItemWidth(100);
            ImGui::Text("Scholarly pursuits    - Skill needed for old writings.");ImGui::SameLine(400);ImGui::SliderInt("+ Reading/Writing", &skill_blah, 0, 3);
            ImGui::Text("Antiquarian studies   - Is that item magical or not?");ImGui::SameLine(400);ImGui::SliderInt("+ identify/Use", &skill_blah_q, 0, 3);
            ImGui::Text("Astrology             - Stars, signs and prophecy.");ImGui::SameLine(400);ImGui::SliderInt("+ Luck", &skill_blah_w, 0, 3);            
            ImGui::Text("Medium                - Even the dead gossip.");ImGui::SameLine(400);ImGui::SliderInt("+ Etiquette", &skill_blah_e, 0, 3);
            ImGui::Text("Occultist knowledge   - More then a good read?");ImGui::SameLine(400);ImGui::SliderInt("+ Spell power/Duration", &skill_blah_r, 0, 3);
            ImGui::Text("Tarot reading         - Ace up your sleeve?");ImGui::SameLine(400);ImGui::SliderInt("+ Tarot Power/ - Side effects", &skill_blah_t, 0, 3);
            ImGui::Text("Knowledge             - Clues to a puzzle.");ImGui::SameLine(400);ImGui::SliderInt("+ Clues, plot items", &skill_blah_y, 0, 3);
            ImGui::Text("Learning              - A candle and a good book.");ImGui::SameLine(400);ImGui::SliderInt("+ Bonues grey hairs", &skill_blah_u, 0, 3);
            ImGui::Text("Archaeology           - Tomb or something sinister?");ImGui::SameLine(400);ImGui::SliderInt("+ Find secrects", &skill_blah_i, 0, 3);
            ImGui::PopItemWidth();
			ImGui::Text("");
            ImGui::Text("Each level gain will produce grey hairs and points to spend in the school of thought skills tree.");
            ImGui::Text("");
			ImGui::Separator();
            ImGui::Text("");
			ImGui::Text("Clicking accept will save the currently selected options, remaining skills points will be saved.");
            ImGui::Text("Please check them carefully.");
			ImGui::Text("");
			ImGui::Text("");ImGui::SameLine(300);
            if (ImGui::Button("Cancel"))
			{
            ShowCharacterScreen = false;
            game_start_options = true;
			
            ImGui::CloseCurrentPopup();
        	}
        	ImGui::SameLine(380);
        	if (ImGui::Button("Accept"))
        	{
        		if (skill_points_spent <= 15 & Essence > 1){
				
        		ofstream save_data(save_name.c_str());	// Rewrite me using for statement to write from a buffer maybe?? char * memblock; memblock = new char [size] of buffer. push buffer = memblock. save_data << memblock. flush buffer, repeat.   
  						if (save_data.is_open())		// Not happy but works for time being.
  							{							// Regex friendly and easy to pull. loading code in. works.
  														
    							save_data << first_name;			save_data << "\n";	save_data << last_name;		save_data << "\n";		
    							save_data << Sex;					save_data << "\n";	save_data << abnormalities;	save_data << "\n";
    							save_data << Class_s;				save_data << "\n";	save_data << Special_flag;	save_data << "\n";
    							save_data << Difficulty;			save_data << "\n";	save_data << Essence;		save_data << "\n";
    							save_data << Dexterity;				save_data << "\n";	save_data << Health;		save_data << "\n";
    							save_data << Fatigue;				save_data << "\n";	save_data << Insanity_res;	save_data << "\n";
    							save_data << skill_points_spent;	save_data << "\n";	save_data << skill_blah;	save_data << "\n";
    							save_data << skill_blah_q;			save_data << "\n";	save_data << skill_blah_w;	save_data << "\n";
    							save_data << skill_blah_e;			save_data << "\n";	save_data << skill_blah_r;	save_data << "\n";
    							save_data << skill_blah_t;			save_data << "\n";	save_data << skill_blah_y;	save_data << "\n";
    							save_data << skill_blah_u;			save_data << "\n";	save_data << skill_blah_i;	save_data << "\n";
    							save_data.close();  	// maybe once orge is init, copy data over to new file make backup.
														// create two working saves for debugging x,y,z char locations on crash.
														// loop for autosave ? every time% elapsed?. ImGui::GetTime() 
								ShowCharacterScreen = false;
								show_ui = true;
								show_app_console2 = true;  // need to setup main game loop, 
								ImGui::CloseCurrentPopup();
							}
						}
					}
			ImGui::Text("");
			ImGui::Text("");
			//ImGui::End();
			//ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
        
        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x*255.0f), (int)(clear_color.y*255.0f), (int)(clear_color.z*255.0f), (int)(clear_color.w*255.0f));
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
        {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            g_pd3dDevice->Reset(&g_d3dpp);
            ImGui_ImplDX9_CreateDeviceObjects();
        }
    }
	Shutdown:
    ImGui_ImplDX9_Shutdown();
    if (g_pd3dDevice) g_pd3dDevice->Release();
    if (pD3D) pD3D->Release();
    UnregisterClass(_T("ImGui Example"), wc.hInstance);

    return 0;
}
