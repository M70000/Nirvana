#include "triggerbot.h"
#include "../options.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "backtrack.hpp"
#include <chrono>

float Triggerbot::serverTime(CUserCmd* cmd) noexcept
{
    static int tick;
    static CUserCmd* lastCmd;

    if (cmd) {
        if (g_LocalPlayer && (!lastCmd || lastCmd->hasbeenpredicted))
            tick = g_LocalPlayer->m_nTickBase();
        else
            tick++;
        lastCmd = cmd;
    }
    return tick * g_GlobalVars->interval_per_tick;
}

bool Triggerbot::IsLineGoesThroughSmoke(Vector vStartPos, Vector vEndPos) 
{
    static auto LineGoesThroughSmokeFn = (bool(*)(Vector vStartPos, Vector vEndPos))Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
    return LineGoesThroughSmokeFn(vStartPos, vEndPos);
}

void Triggerbot::run(CUserCmd* cmd) noexcept
{
        if (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || g_LocalPlayer->m_flNextAttack() > serverTime(cmd) || g_LocalPlayer->m_bIsDefusing() || g_LocalPlayer->waitForNoAttack())
            return;

    const auto activeWeapon = g_LocalPlayer->m_hActiveWeapon();
    if (!activeWeapon || !activeWeapon->m_iClip1() || activeWeapon->m_flNextPrimaryAttack() > serverTime(cmd))
        return;

    if (g_LocalPlayer->m_iShotsFired() > 0 && !activeWeapon->isFullAuto())
        return;

    if (!g_Options.trigger_enable)
        return;

    static auto lastTime = 0.0f;
    static auto lastContact = 0.0f;

    const auto now = g_GlobalVars->realtime;

    if (now - lastContact < g_Options.trigger_burst_time) {
        cmd->buttons |= IN_ATTACK;
        return;
    }
    lastContact = 0.0f;

    if (!GetAsyncKeyState(g_Options.trigger_key))
        return;

    if (now - lastTime < g_Options.trigger_delay_time / 1000.0f)
        return;

    if (!g_Options.trigger_ignoreflash && g_LocalPlayer->IsFlashed())
        return;

    if (g_Options.trigger_scopecheck && activeWeapon->IsSniper() && !g_LocalPlayer->m_bIsScoped())
        return;

    const auto weaponData = activeWeapon->GetCSWeaponData();
    if (!weaponData)
        return;

    const auto startPos = g_LocalPlayer->GetEyePos();
    const auto endPos = startPos + Vector::fromAngle(cmd->viewangles1337 + g_LocalPlayer->m_aimPunchAngle1337()) * weaponData->flRange;

    if (!g_Options.trigger_ignoresmoke && IsLineGoesThroughSmoke(startPos, endPos))
        return;

    Ray_t ray;
    ray.Init(startPos, endPos);

    CTraceFilter flt;
    flt.pSkip = g_LocalPlayer;

    trace_t trace;
    g_EngineTrace->TraceRay(ray, 0x46004009, &flt, &trace);

    lastTime = now;

    if (!trace.entity || !trace.entity->IsPlayer())
        return;

   /* C_BasePlayer* player;

    if (player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
        return;*/

    if (trace.entity->m_bGunGameImmunity())
        return;

    if (g_Options.trigger_hitgroup && trace.hitgroup != g_Options.trigger_hitgroup)
        return;

    float damage = (activeWeapon->m_iItemDefinitionIndex() != ClassId_CWeaponTaser ? HitGroup::getDamageMultiplier(trace.hitgroup) : 1.0f) * weaponData->iDamage * std::pow(weaponData->flRangeModifier, trace.fraction * weaponData->flRange / 500.0f);

    if (float armorRatio{ weaponData->flArmorRatio / 2.0f }; activeWeapon->m_iItemDefinitionIndex() != ClassId_CWeaponTaser && HitGroup::isArmored(trace.hitgroup, trace.entity->hasHelmet()))
        damage -= (trace.entity->m_ArmorValue() < damage * armorRatio / 2.0f ? trace.entity->m_ArmorValue() * 4.0f : damage) * (1.0f - armorRatio);

    if (damage >= (g_Options.trigger_killshot ? trace.entity->m_iHealth() : g_Options.trigger_minDamage)) {
        cmd->buttons |= IN_ATTACK;
        lastTime = 0.0f;
        lastContact = now;
    }
}
