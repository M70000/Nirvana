#include "profilechanger.h"
#include "../helpers/proto/message.hpp"
#include "../helpers/proto/writer.hpp"
#include "../options.hpp"

#define _gc2ch MatchmakingGC2ClientHello
#define _pci PlayerCommendationInfo
#define _pri PlayerRankingInfo
#define CAST(cast, address, add) reinterpret_cast<cast>((uint32_t)address + (uint32_t)add)

std::string Profile::Changer(void* pubDest, uint32_t* pcubMsgSize)
{
	Writer msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8, 19);

	//replace commends
	auto _commendation = msg.has(_gc2ch::commendation) ? msg.get(_gc2ch::commendation).String() : std::string("");
	Writer commendation(_commendation, 4);

	if (g_Options.profile_friendly > -1)
		commendation.replace(Field(_pci::cmd_friendly, TYPE_UINT32, (int64_t)g_Options.profile_friendly));

	if (g_Options.profile_teaching > -1)
		commendation.replace(Field(_pci::cmd_teaching, TYPE_UINT32, (int64_t)g_Options.profile_teaching));

	if (g_Options.profile_leader > -1)
		commendation.replace(Field(_pci::cmd_leader, TYPE_UINT32, (int64_t)g_Options.profile_leader));

	msg.replace(Field(_gc2ch::commendation, TYPE_STRING, commendation.serialize()));

	auto _ranking = msg.has(_gc2ch::ranking) ? msg.get(_gc2ch::ranking).String() : std::string("");
	Writer ranking(_ranking, 6);

	if (g_Options.profile_rankid > 0)
		ranking.replace(Field(_pri::rank_id, TYPE_UINT32, (int64_t)g_Options.profile_rankid));

	if (g_Options.profile_wins > -1)
		ranking.replace(Field(_pri::wins, TYPE_UINT32, (int64_t)g_Options.profile_wins));

	msg.replace(Field(_gc2ch::ranking, TYPE_STRING, ranking.serialize()));

	if (g_Options.profile_level > -1)
		msg.replace(Field(_gc2ch::player_level, TYPE_INT32, (int64_t)g_Options.profile_level));

	if (g_Options.profile_exp > -1)
		msg.replace(Field(_gc2ch::player_xp, TYPE_INT32, (int64_t)g_Options.profile_exp));


	return msg.serialize();
}