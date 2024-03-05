#include "../singleton.hpp"
#include "../valve_sdk/csgostructs.hpp"


namespace movementRecorder
{
	void MovementRecord(CUserCmd* cmd);
	void MovementPlay(CUserCmd* cmd);
	void MovementDrawing();
	void saveToFile();
	void readFromFile();
}

namespace MovementRecorder2
{
	void MovementR(CUserCmd* cmd);
	bool MovementP(CUserCmd* cmd);
	void MovementD();
}

#pragma once
class BunnyHop : public Singleton<BunnyHop>
{
public:
	void OnCreateMove(CUserCmd* cmd);
	void Knifebot(CUserCmd* cmd);
	void JumpBug(CUserCmd* cmd);
	//void EdgebugDetection(CUserCmd* ucmd, float unpredicted_velocity);
	bool EdgebugDetected();
	//void EBUG(CUserCmd* cmd);
	void EdgebugRun(CUserCmd* cmd);
	void EdgebugDraw();
	void EdgebugLock(CUserCmd* cmd);
	void Autoalign(CUserCmd* cmd);
	void FastStop(CUserCmd* cmd);
//	void CopyBOTMovement(CUserCmd* cmd);
	void SlideWalk(CUserCmd* cmd);
	//void fakestuff(CUserCmd* cmd);
	void AutoStrafe(CUserCmd* cmd, QAngle va);
};