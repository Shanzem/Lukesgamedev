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
struct ExampleAppLog    // CODE FOR LOG, MAKE ME NEATER.
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

        ImGui::Begin(title, p_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
        if (ImGui::Button("Clear")) Clear();
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -100.0f);
        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0,0), true, ImGuiWindowFlags_HorizontalScrollbar);
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
static void ShowExampleAppLog(bool* p_open)
{
    static ExampleAppLog log;

    // Demo: add random items (unless Ctrl is held)
    static float last_time = -1.0f;
    float time = ImGui::GetTime();
    if (time - last_time >= 0.80f && !ImGui::GetIO().KeyCtrl)
    {
        const char* random_words[] = { "Hits", "Attack", "Experiance gain", "Miss target", "Ouch" };
        log.AddLog("[%s] Hello,\n", random_words[rand() % IM_ARRAYSIZE(random_words)], time, ImGui::GetFrameCount());
        last_time = time;
    }
		
    	//ImGui::SetNextWindowPos(ImVec2(5, 635));
    	//ImGui::SetNextWindowSize(ImVec2(503,120));
		log.Draw("Reove title bar.", p_open);
		
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

    bool show_demo_window = false;
    bool show_another_window = false;
    bool ShowLogOutput = false;
    bool show_app_main_menu_bar = true;
    bool ShowExampleMenuFile = false;
    bool ShowCharacterScreen = false;
    bool show_app = true;
    bool test_window = false;
    bool show_ui = true;
    bool char_port = false;
    bool exit = false;
	static bool show_app_log = false;
    
    ImVec4 clear_color = ImVec4(0.00f, 0.00f, 0.00f, 1.00f); // colour for screen. place holder picture me asap.
	
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
											 /////////////////////////////////////////////
        if (show_app_main_menu_bar)
        {
        	ImGui::BeginMainMenuBar();
    	}
         if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("New", "CTRL+N", &ShowCharacterScreen);
            ImGui::MenuItem("Save as", "CTRL+S");
            ImGui::Separator();
            ImGui::MenuItem("Load", "CTRL+L");
            ImGui::MenuItem("Options", "CTRL+O");
            ImGui::MenuItem("Exit", "CTRL+E", &exit);
            if (exit)
            {
            	goto Shutdown;	
			}
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();	

        // 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name your windows.
        
		if (show_app)
				{
			ImGui::SetNextWindowPos(ImVec2(435, 131));    
			ImGui::SetNextWindowSize(ImVec2(400,300)); 
			ImGui::Begin("Window manager.", &show_app);
            ImGui::Text("");
            ImGui::Text("Window skipper..");
            ImGui::Checkbox("Character sheet", &ShowCharacterScreen);      // Edit bools storing our windows open/close state
            ImGui::Checkbox("Show example dice roll log", &show_app_log);
            ImGui::Checkbox("Show UI", &show_ui);
				if (show_app_log)                 ShowExampleAppLog(&show_app_log);
            if (ImGui::Button("Close Me"))
                show_another_window = false;
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
				ImGui::SetNextWindowSize(ImVec2(400,178)); 
				ImGui::Begin("MINIMAP.", &show_ui, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            	ImGui::Text("Minimap");
            	ImGui::Text("clickable.");
				ImGui::End();
					
					}
        	ImGui::SetNextWindowPos(ImVec2(977, 200));    
			ImGui::SetNextWindowSize(ImVec2(400,600));
			ImGui::Begin("map and other ui elements.", &show_ui, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::Text("");
            ImGui::Text("Place holder for clickable elements.");
            ImGui::Text("Inventory, other stuff.");
            ImGui::Text("");
        		if (show_ui)
            	{
            	ImGui::SetNextWindowPos(ImVec2(0, 20));    
				ImGui::SetNextWindowSize(ImVec2(100,100)); 
				ImGui::Begin("Picture.", &show_ui, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            	ImGui::Text("Add Picture");
            	ImGui::Text("clickable.");
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
        	ImGui::SetNextWindowPos(ImVec2(5, 640));    
			ImGui::SetNextWindowSize(ImVec2(350,120)); 
			ImGui::Begin("Place holder for log window.", &show_ui, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::Text("");
            ImGui::Text("Place holder for dice log window");
            ImGui::End();
				}
				if (show_ui)
       			 {
        	ImGui::SetNextWindowPos(ImVec2(357, 640));    
			ImGui::SetNextWindowSize(ImVec2(618,120)); 
			ImGui::Begin("Place holder for skills.", &show_ui, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::Text("");
            ImGui::Text("Skill and Explorer items here..");
            ImGui::End();
				}
            ImGui::End();
				}
					
        if (ShowCharacterScreen)
		{
			ImGui::SetNextWindowPos(ImVec2(315, 20), ImGuiCond_FirstUseEver);
			ImGui::Begin("Character birth certificate", &ShowCharacterScreen);
            ImGui::Text("Birth certifiacte and Schooling");
            ImGui::Text("");
            ImGui::Text("");
            ImGui::Text("Fullname:");
            static char str0[128] = "First name...";
            ImGui::InputText(" ", str0, IM_ARRAYSIZE(str0));
            static char str1[128] = "Last name...";
            ImGui::InputText("  ", str1, IM_ARRAYSIZE(str1));
            ImGui::Text("");
            ImGui::Separator();
			ImGui::Text("");
			ImGui::Text("Sex:");ImGui::SameLine(160);ImGui::Text("Abnormalities:");
            static int Sex = 0;
            static int abnormalities = 0;
            ImGui::RadioButton("Male", &Sex, 0);ImGui::SameLine(160);ImGui::RadioButton("None", &abnormalities, 0);ImGui::SameLine(280);ImGui::RadioButton("Deviant Tastes", &abnormalities, 2);ImGui::SameLine(430);ImGui::RadioButton("Egomania", &abnormalities, 4);
            ImGui::RadioButton("Female", &Sex, 1);ImGui::SameLine(160);ImGui::RadioButton("Nocturnal", &abnormalities, 1);ImGui::SameLine(280);ImGui::RadioButton("Lygophobia", &abnormalities, 3);ImGui::SameLine(430);ImGui::RadioButton("Necromania", &abnormalities, 5);
            ImGui::Text("");
            ImGui::Text("Abnormalities have a give and take effect to the stats of a charecter. Adds new elements of game play.");
			ImGui::Text("Example, 'Nocturnal', And 'Necromancy' school of thought will produce a Necromancer with Vampirism. ");
			ImGui::Text("This will have an impact on the characters insanity resistence and behavior.");
			ImGui::Text("");
			ImGui::Text("New players its suggested you choose 'None'.");
			ImGui::Text("");
			ImGui::Text("Current effect:");
			ImGui::Text("");
			ImGui::Separator();
            ImGui::Text("");
            ImGui::Text("School of thought:");
            static int Class = 0;
            ImGui::RadioButton("Hermetic Philosophy", &Class, 0);ImGui::SameLine(200);ImGui::Text("- The All, It gets deeper.");
            ImGui::RadioButton("Necromancy", &Class, 1);ImGui::SameLine(200);ImGui::Text("- Mum always said 'Make' new friends.");
            ImGui::RadioButton("Elementalism", &Class, 2);ImGui::SameLine(200);ImGui::Text("- It never rains, but it pours.");
            ImGui::RadioButton("Illusion", &Class, 3);ImGui::SameLine(200);ImGui::Text("- Have you ever seen a ghost?");
			ImGui::RadioButton("Lycanthropy", &Class, 4);ImGui::SameLine(200);ImGui::Text("- When the moon is full and bright.");
			ImGui::Text(""); 
            ImGui::Separator();
            ImGui::Text("");
            ImGui::Text("Riddles, journals, occultist manuscripts and tarot cards are the curreny of the game.");
            ImGui::Text("All will be stored in books you carry. You start with a school book that can hold 10 of the above mentioned.");
            ImGui::Text("However you can learn above menitioned and gain new abilities, insight and grey hairs.");
			ImGui::Text("");
			ImGui::Text("Tarot cards can be used like magical items !BUT! have side effects and can be lethal when insanity is at 100%%. ");
            ImGui::Text("");
            ImGui::Separator();
            static int Difficulty = 0;
            ImGui::Text("");
            ImGui::Text("Who do you choose?");
            ImGui::Text("");
            ImGui::RadioButton("Sadistic   AI", &Difficulty, 1);ImGui::SameLine(200);ImGui::Text("- Punishment and cruelty.");ImGui::SameLine(400);ImGui::Text("- Hard");
            ImGui::RadioButton("Nihilistic AI", &Difficulty, 0);ImGui::SameLine(200);ImGui::Text("- Why even bother?");ImGui::SameLine(400);ImGui::Text("- Easy");
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
            if (Insanity_res > 30){ Insanity_res = 0;}
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
            ImGui::Text("You need to roll stats, (15) + 7 !");
            ImGui::Text("");
            if (ImGui::Button("Reroll Stats"))
                {
                	srand(static_cast<unsigned int>(time(0)));
					int randomNumber = rand();
			
                	Essence = 0;
                	Dexterity = 0;
                	Health = 0;
                	Fatigue = 0;
                	Insanity_res = 0;
                	Essence = (randomNumber % 15) + 7;
            		Dexterity = (randomNumber % 10) + 5;
            		Health = (randomNumber % 15) + 6;
            		Fatigue = (randomNumber % 15) + 8;
            
				}
			ImGui::Text(" ");
            ImGui::Text("Energy, Mana. ");ImGui::SameLine(120);ImGui::Text("Dodge");ImGui::SameLine(220);ImGui::Text("The Curse of youth.");ImGui::SameLine(400);ImGui::Text("Run rabbit run.");ImGui::SameLine(550);ImGui::Text("Bewarned it will happen!");
            ImGui::Text("Essence: %d", Essence);ImGui::SameLine(120);ImGui::Text("Dexterity: %d", Dexterity);ImGui::SameLine(220);ImGui::Text("Health: %d", Health);ImGui::SameLine(400);ImGui::Text("Fatigue: %d", Fatigue);ImGui::SameLine(550);ImGui::Text("Insanity Resistence: %d%%", Insanity_res);
            ImGui::Text(" ");
			ImGui::Text("'Grey hair' Points to spend: %d", skill_points);ImGui::SameLine(300);ImGui::Text("- Grey hairs are the skills currency, the more the better!");
            skill_points_spent = skill_blah + skill_blah_q + skill_blah_w + skill_blah_e + skill_blah_r + skill_blah_t + skill_blah_y + skill_blah_u + skill_blah_i;
			ImGui::Text("'Grey hair' Points used : %d", skill_points_spent);
            ImGui::Text("");
			ImGui::Separator();
			ImGui::Text("");
			ImGui::Text("'Grey hair' Skill list");
			ImGui::Text("");
            ImGui::PushItemWidth(100);
            ImGui::Text("Scholarly pursuits    - Skill needed for old writings.");ImGui::SameLine(400);ImGui::SliderInt("+ Reading/Writing", &skill_blah, 0, 3);
            ImGui::Text("Antiquarian studies   - Is that item magical or not?");ImGui::SameLine(400);ImGui::SliderInt("+ identify/Use", &skill_blah_q, 0, 3);
            ImGui::Text("Astrology             - Stars, signs and prophecy.");ImGui::SameLine(400);ImGui::SliderInt("+ Luck", &skill_blah_w, 0, 3);            
            ImGui::Text("Medium                - Even the dead gossip.");ImGui::SameLine(400);ImGui::SliderInt("+ Etiquette", &skill_blah_e, 0, 3);
            ImGui::Text("Occultist knowledge   - More then a good read?");ImGui::SameLine(400);ImGui::SliderInt("+ Spell power/Duration", &skill_blah_r, 0, 3);
            ImGui::Text("Tarot reading         - Ace up your sleeve?");ImGui::SameLine(400);ImGui::SliderInt("+ Tarot Power", &skill_blah_t, 0, 3);
            ImGui::Text("Knowledge             - Clues to a puzzle.");ImGui::SameLine(400);ImGui::SliderInt("+ Clues, plot items", &skill_blah_y, 0, 3);
            ImGui::Text("Learning              - A candle and a good book.");ImGui::SameLine(400);ImGui::SliderInt("+ Bonues grey hairs", &skill_blah_u, 0, 3);
            ImGui::Text("Archaeology           - Tomb or something sinister?");ImGui::SameLine(400);ImGui::SliderInt("+ Find secrects", &skill_blah_i, 0, 3);
            ImGui::Text("");
            ImGui::Text("Each level gain will produce a number grey hairs and points to spend in the school of thought skills tree.");
            ImGui::Text("");
			ImGui::Separator();
            ImGui::Text("");
            ImGui::Text("");ImGui::SameLine(300);
            if (ImGui::Button("Cancel"))
			{
            
			ShowCharacterScreen = false;
            
        	}
        	ImGui::SameLine(380);
        	if (ImGui::Button("Accept"))
        	{
        	
			ShowCharacterScreen = false;
				
			}
			ImGui::Text("");
			ImGui::Text("");
			ImGui::End();
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
