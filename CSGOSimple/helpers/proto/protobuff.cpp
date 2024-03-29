#include "protobuff.hpp"

#include "../../features/profilechanger.h"
//#include "..//..//features/inventory.hpp"

#include "..//..//options.hpp"

#define CAST(cast, address, add) reinterpret_cast<cast>((uint32_t)address + (uint32_t)add)

void Protobuf::WritePacket(std::string packet, void* thisPtr, void* oldEBP, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize)
{
	if ((uint32_t)packet.size() <= cubDest - 8)
	{
		memcpy((void*)((DWORD)pubDest + 8), (void*)packet.data(), packet.size());
		*pcubMsgSize = packet.size() + 8;
	}
	else
	{
		auto memPtr = *CAST(void**, thisPtr, 0x14);
		auto memPtrSize = *CAST(uint32_t*, thisPtr, 0x18);
		auto newSize = (memPtrSize - cubDest) + packet.size() + 8;

		auto memory = g_MemAlloc->Realloc(memPtr, newSize + 4);

		*CAST(void**, thisPtr, 0x14) = memory;
		*CAST(uint32_t*, thisPtr, 0x18) = newSize;
		*CAST(void**, oldEBP, -0x14) = memory;

		memcpy(CAST(void*, memory, 24), (void*)packet.data(), packet.size());

		*pcubMsgSize = packet.size() + 8;
	}
}

void Protobuf::ReceiveMessage(void* thisPtr, void* oldEBP, uint32_t messageType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize)
{
	if (messageType == k_EMsgGCCStrike15_v2_MatchmakingGC2ClientHello && g_Options.enable_profilechanger)
	{
		auto packet = g_Profile->Changer(pubDest, pcubMsgSize);
		WritePacket(packet, thisPtr, oldEBP, pubDest, cubDest, pcubMsgSize);
	}
/*	else if (messageType == k_EMsgGCClientWelcome)
	{
		auto packet = g_Inventory->Changer(pubDest, pcubMsgSize);
		WritePacket(packet, thisPtr, oldEBP, pubDest, cubDest, pcubMsgSize);
	}*/

}

bool Protobuf::PreSendMessage(uint32_t& unMsgType, void* pubData, uint32_t& cubData)
{
	//g_Inventory->Presend(unMsgType, pubData, cubData);

	return true;
}

///////////////////////////////////
/******** Custom Messages ********/
///////////////////////////////////
bool Protobuf::SendClientHello()
{
	// message CMsgClientHello {
	// 	repeated .CMsgSOCacheHaveVersion socache_have_versions = 2;
	// 	optional uint32 client_session_need = 3;
	// 	optional uint32 partner_accountflags = 7;
	// }
	Writer msg(7);
	msg.add(Field(3, TYPE_UINT32, (int64_t)1)); //client_session_need
	auto packet = msg.serialize();

	void* ptr = malloc(packet.size() + 8);

	if (!ptr)
		return false;

	((uint32_t*)ptr)[0] = k_EMsgGCClientHello | ((DWORD)1 << 31);
	((uint32_t*)ptr)[1] = 0;

	memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
	bool result = g_SteamGameCoordinator->GCSendMessage(k_EMsgGCClientHello | ((DWORD)1 << 31), ptr, packet.size() + 8) == k_EGCResultOK;
	free(ptr);

	return result;
}

bool Protobuf::SendMatchmakingClient2GCHello()
{
	Writer msg(0);
	auto packet = msg.serialize();
	void* ptr = malloc(packet.size() + 8);

	if (!ptr)
		return false;

	((uint32_t*)ptr)[0] = k_EMsgGCCStrike15_v2_MatchmakingClient2GCHello | ((DWORD)1 << 31);
	((uint32_t*)ptr)[1] = 0;

	memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
	bool result = g_SteamGameCoordinator->GCSendMessage(k_EMsgGCCStrike15_v2_MatchmakingClient2GCHello | ((DWORD)1 << 31), ptr, packet.size() + 8) == k_EGCResultOK;
	free(ptr);

	return result;
}

bool Protobuf::EquipWeapon(int weaponid, int classid, int slotid)
{
	// message CMsgAdjustItemEquippedState {
	// 	optional uint64 item_id = 1;
	// 	optional uint32 new_class = 2;
	// 	optional uint32 new_slot = 3;
	// 	optional bool swap = 4;
	// }
	Writer msg(4);
	msg.add(Field(1, TYPE_UINT64, (int64_t)(/*START_ITEM_INDEX*/10000 + weaponid))); //item_id
	msg.add(Field(2, TYPE_UINT32, (int64_t)classid)); //new_class
	msg.add(Field(3, TYPE_UINT32, (int64_t)slotid)); //new_slot
	msg.add(Field(4, TYPE_BOOL, (int64_t)true)); //swap
	auto packet = msg.serialize();

	void* ptr = malloc(packet.size() + 8);

	if (!ptr)
		return false;

	((uint32_t*)ptr)[0] = k_EMsgGCAdjustItemEquippedState | ((DWORD)1 << 31);
	((uint32_t*)ptr)[1] = 0;

	memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
	bool result = g_SteamGameCoordinator->GCSendMessage(k_EMsgGCAdjustItemEquippedState | ((DWORD)1 << 31), ptr, packet.size() + 8) == k_EGCResultOK;
	free(ptr);

	return result;
}