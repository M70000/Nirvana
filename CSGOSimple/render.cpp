#include "render.hpp"

#include <mutex>

#include "features/visuals.hpp"
#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "fonts/fonts.hpp"
#include "images.hpp"
#include "helpers/math.hpp"

ImFont* g_pDefaultFont;
ImFont* g_pSecondFont;
ImFont* g_pVelocityFont;
ImFont* g_pAstriumFont;
ImFont* g_pSmallestPixelFont;
ImFont* g_pTahomaFont;
IDirect3DTexture9 *brand_img = nullptr;

ImDrawListSharedData _data;

std::mutex render_mutex;

void Render::Initialize()
{
	ImGui::CreateContext();

	if (brand_img == nullptr)
		D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, &images::brand_bytes, sizeof(images::brand_bytes), 142, 40, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &brand_img);

	ImGui_ImplWin32_Init(InputSys::Get().GetMainWindow());
	ImGui_ImplDX9_Init(g_D3DDevice9);

	draw_list = new ImDrawList(ImGui::GetDrawListSharedData());
	draw_list_act = new ImDrawList(ImGui::GetDrawListSharedData());
	draw_list_rendering = new ImDrawList(ImGui::GetDrawListSharedData());

	GetFonts();
}

void Render::GetFonts() {

	// menu font
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		Fonts::Droid_compressed_data,
		Fonts::Droid_compressed_size,
		14.f,
		nullptr,
		ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	
	// esp font
	g_pDefaultFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		Fonts::Droid_compressed_data,
		Fonts::Droid_compressed_size,
		18.f,
		nullptr,
		ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	

	// font for watermark; just example
	g_pSecondFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		Fonts::Cousine_compressed_data,
		Fonts::Cousine_compressed_size,
		18.f,
		nullptr,
		ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	// Velocity Font
	g_pVelocityFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		Fonts::Verdana_compressed_data,
		Fonts::Verdana_compressed_size,
		34.f,
		nullptr,
		ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	// Weapon Icons Font
	g_pAstriumFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		Fonts::AstriumWep_compressed_data,
		Fonts::AstriumWep_compressed_size,
		18.f,
		nullptr,
		ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	// Smallest Pixel Font
	g_pSmallestPixelFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		Fonts::Smallest_compressed_data,
		Fonts::Smallest_compressed_size,
		20.f,
		nullptr,
		ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	// Tahoma Font
	g_pTahomaFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		Fonts::Tahoma_compressed_data,
		Fonts::Tahoma_compressed_size,
		20.f,
		nullptr,
		ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
}

void Render::ClearDrawList() {
	render_mutex.lock();
	draw_list_act->Clear();
	render_mutex.unlock();
}

float frameRate = 0.f;
void Render::BeginScene() {
	draw_list->Clear();
	draw_list->PushClipRectFullScreen();


	static float alpha;

	if (Menu::Get().IsVisible() && alpha < 0.5)			alpha += 0.025;
	else if (!Menu::Get().IsVisible() && alpha > 0)		alpha -= 0.025;

	int screenWidth, screenHeight;
	g_EngineClient->GetScreenSize(screenWidth, screenHeight);

	draw_list->AddRectFilled(ImVec2(0, 0), ImVec2(screenWidth, screenHeight), ImGui::GetColorU32(ImVec4(0, 0, 0, alpha)));


	if (g_Options.misc_watermark)
	{
		g_Options.rainbow += 0.0005f;
		if (g_Options.rainbow > 1.f)
			g_Options.rainbow = 0.f;

		frameRate = 0.9f * frameRate + (1.0 - 0.9f) * g_GlobalVars->absoluteframetime;
		static float rainbow;
		rainbow += 0.005f;
		if (rainbow > 1.f)
			rainbow = 0.f;

		static std::string name = "PAID USER";
		if (g_LocalPlayer) {
			player_info_t player_info;
			g_EngineClient->GetPlayerInfo(g_EngineClient->GetLocalPlayer(), &player_info);
			name = player_info.szName;
		}
		auto net_channel = g_EngineClient->GetNetChannelInfo();
		auto latency = (net_channel && g_EngineClient->IsInGame() && !g_EngineClient->IsPlayingDemo()) ? net_channel->GetAvgLatency(FLOW_OUTGOING) : 0.0f;
		auto ping = std::to_string((int)(std::fmax(0.0f, latency) * 1000.0f));
		std::stringstream ss;
		ss << "NIRVANA | FPS: " << static_cast<int>(1.f / frameRate) << " | PING: " << ping.c_str() << " | " << name.c_str();
		Render::Get().RenderBoxFilled(228, 38, 5, 5, Color(0.10f, 0.09f, 0.12f, 1.00f));
		/*	if (!g_Options.misc.chroma_menu)
				Render::get().RenderLine(380, 10, 5, 10, Color(menu::color), 3.f);
			else*/
		Render::Get().RenderLine(228, 10, 5, 10, Color::FromHSB(g_Options.rainbow, 1.f, 1.f), 10.f);
		Render::Get().RenderText(ss.str(), ImVec2(15, 20), 12.f, Color(255, 255, 255, 255), false, false, g_pDefaultFont);
	}

	if (g_EngineClient->IsInGame() && g_LocalPlayer)
		Visuals::Get().AddToDrawList();

	if (g_EngineClient->IsInGame() && g_LocalPlayer && g_Options.other_drawfov)
		Visuals::Get().DrawFOV();

	extern float side;		

	render_mutex.lock();
	*draw_list_act = *draw_list;
	render_mutex.unlock();
}

ImDrawList* Render::RenderScene() {

	if (render_mutex.try_lock()) {
		*draw_list_rendering = *draw_list_act;
		render_mutex.unlock();
	}

	return draw_list_rendering;
}


float Render::RenderText(const std::string& text, ImVec2 pos, float size, Color color, bool center, bool outline, ImFont* pFont)
{
	ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());
	if (!pFont->ContainerAtlas) return 0.f;
	draw_list->PushTextureID(pFont->ContainerAtlas->TexID);

	if (center)
		pos.x -= textSize.x / 2.0f;

	if (outline) {
		draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y + 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x - 1, pos.y - 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y - 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x - 1, pos.y + 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
	}

	draw_list->AddText(pFont, size, pos, GetU32(color), text.c_str());

	draw_list->PopTextureID();

	return pos.y + textSize.y;
}

void Render::RenderCircle3D(Vector position, float points, float radius, Color color)
{
	float step = (float)M_PI * 2.0f / points;

	for (float a = 0; a < (M_PI * 2.0f); a += step)
	{
		Vector start(radius * cosf(a) + position.x, radius * sinf(a) + position.y, position.z);
		Vector end(radius * cosf(a + step) + position.x, radius * sinf(a + step) + position.y, position.z);

		Vector start2d, end2d;
		if (g_DebugOverlay->ScreenPosition(start, start2d) || g_DebugOverlay->ScreenPosition(end, end2d))
			return;

		RenderLine(start2d.x, start2d.y, end2d.x, end2d.y, color);
	}
}

#define ZERO Vector(0.0f, 0.0f, 0.0f)

void Render::DrawBeamCircle(Color clr, int width, Vector pos) {
	BeamInfo_t beam_info;
	beam_info.m_nType = TE_BEAMRINGPOINT;
	beam_info.m_pszModelName = ("sprites/purplelaser1.vmt");
	beam_info.m_nModelIndex = g_MdlInfo->GetModelIndex("sprites/purplelaser1.vmt");
	beam_info.m_flHaloScale = 0;
	beam_info.m_flLife = 0.50f;
	beam_info.m_flWidth = width;
	beam_info.m_flFadeLength = 0;
	beam_info.m_flAmplitude = 0;
	beam_info.m_flRed = clr.r();
	beam_info.m_flGreen = clr.g();
	beam_info.m_flBlue = clr.b();
	beam_info.m_flBrightness = clr.a();
	beam_info.m_flSpeed = 0;
	beam_info.m_nStartFrame = 0;
	beam_info.m_flFrameRate = 0;
	beam_info.m_nSegments = -1;
	beam_info.m_nFlags = FBEAM_SHADEIN;
	beam_info.m_vecCenter = pos;
	beam_info.m_flStartRadius = 50;
	beam_info.m_flEndRadius = 500;

	auto beam = g_RenderBeams->CreateBeamRingPoint(beam_info);
	if (beam) g_RenderBeams->DrawBeam(beam);
}

void Render::Draw3DFilledCircle(const Vector& origin, float radius, Color color)
{
	static auto prevScreenPos = ZERO; //-V656
	static auto step = M_PI * 2.0f / 72.0f;

	auto screenPos = ZERO;
	auto screen = ZERO;

	if (!Math::WorldToScreen(origin, screen))
		return;

	for (auto rotation = 0.0f; rotation <= M_PI * 2.0f; rotation += step) //-V1034
	{
		Vector pos(radius * cos(rotation) + origin.x, radius * sin(rotation) + origin.y, origin.z);

		if (Math::WorldToScreen(pos, screenPos))
		{
			if (!prevScreenPos.IsZero() && prevScreenPos.IsValid() && screenPos.IsValid() && prevScreenPos != screenPos)
			{
				RenderLine(prevScreenPos.x, prevScreenPos.y, screenPos.x, screenPos.y, color);
				RenderTriangle(screen.x, screen.y, screenPos.x, screenPos.y, prevScreenPos.x, prevScreenPos.y, Color(color.r(), color.g(), color.b(), color.a() / 2));
			}

			prevScreenPos = screenPos;
		}
	}
}



































































































































































































































































































































































































































































































































































































































































































