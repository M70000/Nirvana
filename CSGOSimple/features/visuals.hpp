#pragma once

#include "../singleton.hpp"

#include "../render.hpp"
#include "../helpers/math.hpp"
#include "../valve_sdk/csgostructs.hpp"

#include <map>

class bomb_planted_event : public IGameEventListener2
{
public:
	~bomb_planted_event() { g_GameEvents->RemoveListener(this); }

	virtual void FireGameEvent(IGameEvent* p_event);
};

class round_end_event : public IGameEventListener2
{
public:
	~round_end_event() { g_GameEvents->RemoveListener(this); }

	virtual void FireGameEvent(IGameEvent* p_event);
};

/*class vote_cast_event : public IGameEventListener2, public Singleton<vote_cast_event>
{
public:
	~vote_cast_event() { g_GameEvents->RemoveListener(this); }

	void FireGameEvent(IGameEvent* p_event);
};*/

struct NadeInfo_t
{
	Color color;
	std::string name;
};

struct SoundInfo_t {
	int guid;
	float soundTime;
	float alpha;
	Vector soundPos;
};

struct motionblurhistory {
	motionblurhistory() {
		lasttimeupdate = 0.0f;
		previouspitch = 0.0f;
		previousyaw = 0.0f;
		previouspositon = Vector{ 0.0f, 0.0f, 0.0f };
		nomotionbluruntil = 0.0f;
	}
	float lasttimeupdate;
	float previouspitch;
	float previousyaw;
	Vector previouspositon;
	float nomotionbluruntil;
};

class Visuals : public IGameEventListener2, public Singleton<Visuals>
{
	friend class Singleton<Visuals>;

	CRITICAL_SECTION cs;

	float flHurtTime;

	Visuals();
	~Visuals();
public:
	class Player
	{
	public:
		struct
		{
			C_BasePlayer* pl;
			bool          is_enemy;
			bool          is_visible;
			Color         clr;
			Vector        head_pos;
			Vector        feet_pos;
			RECT          bbox;
			int right_text_size = 0;
			int bottom_text_size = 0;
			player_info_t* pinfo;
			bool is_nirvana_user = false;
		} ctx;

		bool Begin(C_BasePlayer* pl);
		void RenderBox();
		void GrenadeKillAlert();
		void RenderName();
		void DrawNirvanaUser();
		void RenderWeaponName();
		void RenderHealth();
		void DrawFlags();
		void RenderArmour();
		void RenderSkeleton(C_BaseEntity* ent);
		void RenderSkeletBacktrack(C_BasePlayer* player);
		void RenderBacktrack();
		//void draw_flags();
		//void ESPFlags();
	};

	virtual void FireGameEvent(IGameEvent* event);
	virtual int  GetEventDebugID(void);
	void sniperCrosshair() noexcept;

	void DrawLine(float x1, float y1, float x2, float y2, Color color, float size = 1.f);
	void DrawBox(float x1, float y1, float x2, float y2, Color clr, float size = 1.f);
	void DrawBoxEdges(float x1, float y1, float x2, float y2, Color clr, float edge_size, float size = 1.f);

	std::map< int, std::vector< SoundInfo_t > > m_Sounds;

	void RenderSounds();
	void UpdateSounds();

	NadeInfo_t GetNadeInfo(C_BaseEntity* nade);
	void DrawThrowable(C_BaseEntity* throwable, ClientClass* client);
	void RenderWeapon(C_BaseCombatWeapon* ent);
	void C4Info(C_BaseEntity* ent);
	void BombTimer(C_BaseEntity* ent);
	void RenderDefuseKit(C_BaseEntity* ent);
	void RenderPlantedC4(C_BaseEntity* ent);
	void MolotovTimer(C_BaseEntity* entity);
	void RenderItemEsp(C_BaseEntity* ent);
	void DrawHitmarker();
	void KillCounterEvent(IGameEvent* event);
	void KillCounter();
	void EdgeBugDetector();
	void hitmarker_event(IGameEvent* event);
	void killsound_event(IGameEvent* event);
	void keystrokes() noexcept;
	void NullingIndicator() noexcept;
	void MovementRecorderInd() noexcept;
	void LegitAA_Draw();
	void MovementIndicators() noexcept;
	void VelocityDraw() noexcept;
	void VelocityGraph() noexcept;
	void FootTrails();
	void RenderSpectatorList();
	void ThirdPerson();
	void DrawFOV();
	void RCSCrosshair();
	void RenderSniperCrosshair();
	bool create_fonts();
	void motionblur_render(CViewSetup* setup);
	void motionblur_run();
public:
	void AddToDrawList();
	void Render();
	void Listener();
	void RemoveListener();
};
