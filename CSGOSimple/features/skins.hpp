#pragma once
#include "../valve_sdk/csgostructs.hpp"
#include "../options.hpp"
namespace Skins {
	void OnFrameStageNotify(bool frame_end);
	void AgentChanger(ClientFrameStage_t stage) noexcept;
};

namespace Preview
{
	std::string FindUrl(std::string paintName, std::string weapon);
	std::string DownloadBytes(const char* const szUrl);
}