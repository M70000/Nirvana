#include "DiscordRPC.hpp"
#include "valve_sdk/csgostructs.hpp"
#include "valve_sdk/sdk.hpp"

DiscordRPC DCRPC = DiscordRPC();

void DiscordRPC::RPCStart()
{
	DiscordEventHandlers Handler;
	memset(&Handler, 0, sizeof(Handler));
	Discord_Initialize("744217299346653265", &Handler, 1, NULL);
}

void DiscordRPC::UpdateRPC()
{
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
/*	if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected())
	{
		float kdr = g_LocalPlayer->m_plKills() / (g_LocalPlayer->m_plDeaths() + 1); // so we don't divide by 0 by mistake, lol
		discordPresence.state = "In-Game";
		discordPresence.details = std::string("KDR: " + std::to_string(kdr)).c_str();
	//	discordPresence.details = "Playing with Nirvana";
	}
	else
	{
		discordPresence.state = "Main Menu";
		discordPresence.details = "Playing with Nirvana";
	}*/
	discordPresence.state = "Playing CS:GO";
	discordPresence.details = "Using nirvanaz.life";
//	discordPresence.startTimestamp = 1507665886;
//	discordPresence.endTimestamp = 1507665886;
	discordPresence.largeImageKey = "nirvana_stuff";
	discordPresence.largeImageText = "Nirvana Software";
	discordPresence.smallImageKey = "nirvana_stuff";
	discordPresence.smallImageText = "CS:GO Cheat";
	Discord_UpdatePresence(&discordPresence);
}