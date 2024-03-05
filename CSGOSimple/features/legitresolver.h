#pragma once
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "../options.hpp"
#include <fstream>
#include <iomanip>
#include <sys/stat.h>
#include <iostream>
#include <vector>
#include <regex>
#include <array>
#include "../helpers/input.hpp"
#include "../dirent.h"
#include "../render.hpp"


class LegitResolver
{
public:
	void Createmove(C_BasePlayer* entity);
	void FrameStageNotify(int stage);
	//void FrameStageNotify(ClientFrameStage_t stage);
	void Resolve(C_BasePlayer* entity);
}; extern LegitResolver legitresolver;