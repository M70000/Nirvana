#pragma once
#include "../valve_sdk/interfaces/IGameEventmanager.hpp"
#include "../singleton.hpp"
#include "../valve_sdk/math/Vector.hpp"
#include "IViewRenderBeams.h"
#include <vector>

struct grenade
{
	int type;
	Vector location;
};

class GrenadeDetonateEvent : public IGameEventListener2, public Singleton<GrenadeDetonateEvent>
{
public:

	void FireGameEvent(IGameEvent* event);
	int  GetEventDebugID(void);

	void RegisterSelf();
	void UnregisterSelf();

	void Paint();

private:

	std::vector<grenade> v_grenade_info;
};