#pragma once
#include <windows.h>
#include "DiscordRichPresence--master/include/discord_register.h"
#include "DiscordRichPresence--master/include/discord_rpc.h"

class DiscordRPC
{
public:
	void RPCStart();
	void UpdateRPC();
};

extern DiscordRPC DCRPC;