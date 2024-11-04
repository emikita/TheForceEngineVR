#include <TFE_Ui/ui.h>
#include <TFE_FileSystem/paths.h>
#include <TFE_FileSystem/fileutil.h>
#include <TFE_Settings/settings.h>

#include <TFE_Input/inputMapping.h>
#include <TFE_Vr/vr.h>

//#define USE_INTERNAL_IMGUI_RENDERER

#include "imGUI/imgui.h"
#include "imGUI/imgui_impl_sdl2.h"
#if defined(USE_INTERNAL_IMGUI_RENDERER)
#include "imGUI/imgui_impl_opengl3.h"
#else
#include <TFE_RenderShared/ImGuiDraw.h>
#endif
#include "portable-file-dialogs.h"
#include "markdown.h"
#include <SDL.h>

namespace TFE_Ui
{
const char* glsl_version = "#version 130";
static s32 s_uiScale = 100;

bool init(void* window, void* context, s32 uiScale)
{
	s_uiScale = uiScale;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL((SDL_Window*)window, context);
#if defined(USE_INTERNAL_IMGUI_RENDERER)
	ImGui_ImplOpenGL3_Init(glsl_version);
#else
	TFE_RenderShared::imGuiDraw_init();
#endif

	// Set the default font (13 px)
	// TODO: Allow scaled UI, so loading a different font for larger scales.
	if (s_uiScale <= 100)
	{
		io.Fonts->AddFontDefault();
	}
	else
	{
		char fp[TFE_MAX_PATH];
		sprintf(fp, "Fonts/DroidSansMono.ttf");
		TFE_Paths::mapSystemPath(fp);
		io.Fonts->AddFontFromFileTTF(fp, f32(13 * s_uiScale / 100));
	}
	
	TFE_Markdown::init(f32(16 * s_uiScale / 100));

	// Initialize file dialogs.
	if (!pfd::settings::available())
	{
		// TODO: Log error
		return false;
	}
	
	return true;
}

void shutdown()
{
	TFE_Markdown::shutdown();

#if defined(USE_INTERNAL_IMGUI_RENDERER)
	ImGui_ImplOpenGL3_Shutdown();
#else
	TFE_RenderShared::imGuiDraw_destroy();
#endif
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void setUiScale(s32 scale)
{
	s_uiScale = scale;
}

s32 getUiScale()
{
	return s_uiScale;
}

void setUiInput(const void* inputEvent)
{
	const SDL_Event* sdlEvent = (SDL_Event*)inputEvent;
	ImGui_ImplSDL2_ProcessEvent(sdlEvent);
}

void begin()
{
#if defined(USE_INTERNAL_IMGUI_RENDERER)
	ImGui_ImplOpenGL3_NewFrame();
#else
	TFE_RenderShared::imGuiDraw_createFontTexture();
#endif
	ImGui_ImplSDL2_NewFrame();

	// override some ImGui SDL2 stuff for VR
	if (TFE_Settings::getTempSettings()->vr)
	{
		ImGuiIO& io = ImGui::GetIO();
		const Vec2ui& size = vr::GetRenderTargetSize();
		io.DisplaySize = ImVec2((float)size.x, (float)size.y);
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

		s32 mouseX, mouseY;
		TFE_Input::getMousePos(&mouseX, &mouseY);
		io.AddMousePosEvent((f32)mouseX, (f32)mouseY);
	}

	ImGui::NewFrame();
}

void render()
{
	ImGui::Render();
#if defined(USE_INTERNAL_IMGUI_RENDERER)
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#else
	TFE_RenderShared::imGuiDraw_render();
#endif
}

void invalidateFontAtlas()
{
#if defined(USE_INTERNAL_IMGUI_RENDERER)
	ImGui_ImplOpenGL3_DestroyFontsTexture();
#else
	TFE_RenderShared::imGuiDraw_destroyFontTexture();
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// General TFE tries to keep paths consistently using forward slashes for readability, consistency and
// generally they work equally well on Linux, Mac and Windows.
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// *However* some specific APIs (usually of the Win32 variety) require backslashes.
// So FileUtil::convertToOSPath() must be used with initial paths to convert to the correct slash type.
// This can be a bit of a waste but I'd rather have consistent paths through most of the application
// and restrict the ugliness to as small an area as possible.
/////////////////////////////////////////////////////////////////////////////////////////////////////////
FileResult openFileDialog(const char* title, const char* initPath, std::vector<std::string> const &filters/* = { "All Files", "*" }*/, bool multiSelect/* = false*/)
{
	char initPathOS[TFE_MAX_PATH] = "";
	if (initPath)
	{
		FileUtil::convertToOSPath(initPath, initPathOS);
	}

	return pfd::open_file(title, initPathOS, filters, multiSelect ? pfd::opt::multiselect : pfd::opt::none).result();
}

FileResult directorySelectDialog(const char* title, const char* initPath, bool forceInitPath/* = false*/)
{
	char initPathOS[TFE_MAX_PATH] = "";
	if (initPath)
	{
		FileUtil::convertToOSPath(initPath, initPathOS);
	}

	FileResult result;
	std::string res = pfd::select_folder(title, initPathOS).result();
	result.push_back(res);

	return result;
}

FileResult saveFileDialog(const char* title, const char* initPath, std::vector<std::string> filters/* = { "All Files", "*" }*/, bool forceOverwrite/* = false*/)
{
	char initPathOS[TFE_MAX_PATH] = "";
	if (initPath)
	{
		FileUtil::convertToOSPath(initPath, initPathOS);
	}

	FileResult result;
	std::string res = pfd::save_file(title, initPathOS, filters, forceOverwrite ? pfd::opt::force_overwrite : pfd::opt::none).result();
	result.push_back(res);

	return result;
}

}
