#include "render_menu.hpp"
#include "../menu_background.hpp"
#include "../resource.h"
#include "../utilities/config.hpp"
#include "../utilities/skcrypt.hpp"
#include "../myfont.hpp"
#include <string>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wideParameter, LPARAM longParameter);

// Ensure the function signature matches WNDPROC exactly
LRESULT CALLBACK window_process(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
		return true;

	switch (message)
	{
	case WM_SIZE:
		if (render_menu::device && wParam != SIZE_MINIMIZED)
		{
			render_menu::destory_render_target();
			render_menu::swap_chain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			render_menu::create_render_target();
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

	return ::DefWindowProc(hwnd, message, wParam, lParam);
}


void render_menu::create_window(LPCWSTR window_name)
{
	window_class.cbSize = sizeof(WNDCLASSEX);
	window_class.style = CS_CLASSDC;
	window_class.lpfnWndProc = window_process;
	window_class.cbClsExtra = 0L;
	window_class.cbWndExtra = 0L;
	window_class.hInstance = GetModuleHandleA(NULL);
	window_class.hIcon = LoadIcon(window_class.hInstance, MAKEINTRESOURCE(IDI_ICON1));
	window_class.hCursor = NULL;
	window_class.hbrBackground = NULL;
	window_class.lpszMenuName = NULL;
	window_class.lpszClassName = skcrypt( L"class001" );
	window_class.hIconSm = NULL;

	RegisterClassEx(&window_class);

	window = CreateWindow(window_class.lpszClassName, window_name,
		WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU /*| WS_MAXIMIZEBOX*/, 100, 100, width, height, NULL, NULL,
		window_class.hInstance, NULL);

	if (!create_device())
	{
		destroy_device();
		::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
		return;
	}

	::ShowWindow(window, SW_SHOWDEFAULT);
	::UpdateWindow(window);
}

void render_menu::destroy_window()
{
	::DestroyWindow(window);
	::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
}

bool render_menu::create_device()
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = window;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &swap_chain, &device, &featureLevel, &device_context) != S_OK)
		return false;

	create_render_target();
	return true;
}

void render_menu::destroy_device()
{
	if (swap_chain) { swap_chain->Release(); swap_chain = nullptr; }
	if (device_context) { device_context->Release(); device_context = nullptr; }
	if (device) { device->Release(); device = nullptr; }
}

void render_menu::create_render_target()
{
	ID3D11Texture2D* pBackBuffer;
	swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	device->CreateRenderTargetView(pBackBuffer, NULL, &render_target_view);
	pBackBuffer->Release();
}

void render_menu::destory_render_target()
{
	if (render_target_view) { render_target_view->Release(); render_target_view = NULL; }
}

void render_menu::create_imgui( )
{
	IMGUI_CHECKVERSION( );
	ImGui::CreateContext( );
	ImGuiIO& io = ::ImGui::GetIO( );
	io.IniFilename = NULL;
	io.Fonts->AddFontDefault( );
	icon_font = io.Fonts->AddFontFromMemoryCompressedTTF( MyFont_compressed_data, MyFont_compressed_size, 18 );

	ImGuiStyle& style = ::ImGui::GetStyle( );
	style.FrameBorderSize = 1.f;
	style.GrabMinSize = 21.f;

	ImGui::StyleColorsCustom( );
	D3DX11CreateShaderResourceViewFromMemory( device, menu_background, sizeof( menu_background ), nullptr, nullptr, &background, nullptr );

	ImGui_ImplWin32_Init( window );
	ImGui_ImplDX11_Init( device, device_context );
}

void render_menu::destroy_imgui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void render_menu::begin_render()
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT)
		{
			is_running = false;
			return;
		}
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void render_menu::end_render( )
{
	ImGui::Render();
	const float clear_color_with_alpha[4] = { 0.f, 0.f, 0.f, 1.f };
	device_context->OMSetRenderTargets(1, &render_target_view, NULL);
	device_context->ClearRenderTargetView(render_target_view, clear_color_with_alpha);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	swap_chain->Present(1, 0); // Present with vsync
}

void render_menu::render()
{
	//ImGui::ShowDemoWindow( );

	ImGui::SetNextWindowPos( { 0, 0 } );
	ImGui::SetNextWindowSize( { width - 16, height - 39 } );

	static int current_tab{ 0 };
	const char* tabs[ ] = { "0", "1", "2" };

	ImGui::Begin(
		skcrypt( "##main_window" ),
		&is_running,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar
	);

	const ImVec2 content_available{ ImGui::GetContentRegionAvail() };
	ImGui::BeginChild( skcrypt( "##tab_window" ), ImVec2( content_available.x, 30 ) );

	for ( int i{ 0 }; i < ARRAYSIZE( tabs ); ++i )
	{
		ImGui::PushFont(icon_font);
		ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.f, 0.f ) );
		ImGui::SameLine( );
		if ( ImGui::Button( tabs[ i ], ImVec2( content_available.x / ARRAYSIZE( tabs ), 30 ) ) )
		{
			current_tab = i;
		}
		ImGui::PopStyleVar( );
		ImGui::PopFont();
	}

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
	ImGui::EndChild( );
	ImGui::GetWindowDrawList()->AddImage(background, ImGui::GetWindowPos(), ImGui::GetWindowSize(), ImVec2(0.7f, 0.7f));
	ImGui::BeginChild( skcrypt( "##child_frame" ), ImGui::GetContentRegionAvail( ), true );
	ImGui::PopStyleVar( );
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 2.f));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 6.f));
	ImGui::PushItemWidth( ImGui::GetContentRegionAvail().x - 1 );

	static bool is_rebinding = false; // Tracks if we are rebinding a key
	static std::string key_display = "Press a key..."; // Text for key rebinding

	switch ( current_tab )
	{
	case 0:
		ImGui::Text(skcrypt("Field of View"));
		ImGui::SliderInt(skcrypt("##aimbot_fov"), &cfg::aimbot_fov, 0, 40);
		ImGui::Text(skcrypt("Smoothing"));
		ImGui::SliderInt(skcrypt("##aimbot_smooth"), &cfg::aimbot_smooth, 1, 20);
		ImGui::Text(skcrypt("Recoil length"));
		ImGui::SliderInt(skcrypt("##recoil_length"), &cfg::recoil_length, 0, 50);

		ImGui::Text("Aimbot Keybind:");                // Label for the keybind section
		if (is_rebinding) {
			ImGui::Text(key_display.c_str());          // Display current rebinding status

			// Check for key press
			for (int key = 0; key < 256; ++key) {      // Loop through all virtual keys (0-255)
				if (ImGui::IsKeyPressed(key)) {        // Detect a key press
					cfg::aimbot_key = key;             // Update the configuration with the new key
					key_display = "Bound to key: " + std::to_string(key); // Update feedback
					is_rebinding = false;              // Exit rebinding mode
					break;
				}
			}

			// Handle mouse buttons explicitly
			if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) { // Left mouse button
				cfg::aimbot_key = VK_LBUTTON;
				key_display = "Bound to: Left Mouse Button";
				is_rebinding = false;
			}
			else if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) { // Right mouse button
				cfg::aimbot_key = VK_RBUTTON;
				key_display = "Bound to: Right Mouse Button";
				is_rebinding = false;
			}
			else if (GetAsyncKeyState(VK_MBUTTON) & 0x8000) { // Middle mouse button
				cfg::aimbot_key = VK_MBUTTON;
				key_display = "Bound to: Middle Mouse Button";
				is_rebinding = false;
			}
			else if (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) { // Mouse XButton1
				cfg::aimbot_key = VK_XBUTTON1;
				key_display = "Bound to: Mouse Button 4";
				is_rebinding = false;
			}
			else if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) { // Mouse XButton2
				cfg::aimbot_key = VK_XBUTTON2;
				key_display = "Bound to: Mouse Button 5";
				is_rebinding = false;
			}
		}
		else {
			key_display = "Current key: " + std::to_string(cfg::aimbot_key); // Show currently bound key
			ImGui::Text(key_display.c_str());         // Display the current binding
			if (ImGui::Button("Rebind Key")) {        // Button to start rebinding
				is_rebinding = true;                  // Enter rebinding mode
				key_display = "Press a key...";       // Update feedback
			}
		}
		break;

		case 1:
			ImGui::Text(skcrypt("Field of View"));
			ImGui::SliderInt(skcrypt("##magnet_fov"), &cfg::magnet_fov, 0, 40);
			ImGui::Text(skcrypt("Smoothing"));
			ImGui::SliderInt(skcrypt("##magnet_smooth"), &cfg::magnet_smooth, 1, 20);
			ImGui::Text(skcrypt("Delay between shots (ms)"));
			ImGui::SliderInt(skcrypt("##recoil_length"), &cfg::magnet_delay_between_shots, 100, 300);
		break;

		case 2:
			ImGui::Text("Perlin Noise Settings");
			ImGui::Checkbox("Enable Perlin Noise", &cfg::use_perlin_noise);
			ImGui::SliderFloat("Frequency", &cfg::perlin_frequency, 0.01f, 1.0f);
			ImGui::SliderFloat("Amplitude", &cfg::perlin_amplitude, 0.0f, 10.0f);
		break;

		default:
			ImGui::Text(skcrypt("How did you get here?"));
		break;
	}
	ImGui::EndChild();
	ImGui::End();
	ImGui::PopStyleVar();
}