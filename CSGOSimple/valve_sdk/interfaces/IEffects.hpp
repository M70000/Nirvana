#pragma once

struct te_tf_particle_effects_control_point_t {
    int m_eParticleAttachment;
    Vector m_vecOffset{};
};

struct te_tf_particle_effects_colors_t {
    Vector m_vecColor1;
    Vector m_vecColor2;
};

class CEffectData {
public:
    Vector m_vOrigin;
    Vector m_vStart;
    Vector m_vNormal;
    Vector m_vAngles;
    int m_fFlags;
    int m_nEntIndex;
    float m_flScale;
    float m_flMagnitude;
    float m_flRadius;
    int m_nAttachmentIndex;
    short m_nSurfaceProp;

    int m_nMaterial;
    int m_nDamageType;
    int m_nHitBox;

    unsigned char m_nColor;

    bool m_bCustomColors;
    te_tf_particle_effects_colors_t m_CustomColors;

    bool m_bControlPoint1;
    te_tf_particle_effects_control_point_t m_ControlPoint1;

public:
    CEffectData() {
        m_vOrigin = { 0.f, 0.f, 0.f };
        m_vStart = { 0.f, 0.f, 0.f };
        m_vNormal = { 0.f, 0.f, 0.f };
        m_vAngles = { 0.f, 0.f, 0.f };

        m_fFlags = 0;
        m_nEntIndex = 0;
        m_flScale = 1.f;
        m_nAttachmentIndex = 0;
        m_nSurfaceProp = 0;

        m_flMagnitude = 0.0f;
        m_flRadius = 0.0f;

        m_nMaterial = 0;
        m_nDamageType = 0;
        m_nHitBox = 0;

        m_nColor = 0;

        m_bCustomColors = false;
        m_CustomColors.m_vecColor1 = { 1.f, 1.f, 1.f };
        m_CustomColors.m_vecColor2 = { 1.f, 1.f, 1.f };

        m_bControlPoint1 = false;
        m_ControlPoint1.m_eParticleAttachment = 0;
    }

    int GetEffectNameIndex() {
        return m_iEffectName;
    }

private:
    int m_iEffectName;
};

class IEffects
{
public:
    virtual ~IEffects() {};
    virtual void Beam(const Vector& Start, const Vector& End, int nModelIndex,
        int nHaloIndex, unsigned char frameStart, unsigned char frameRate,
        float flLife, unsigned char width, unsigned char endWidth, unsigned char fadeLength,
        unsigned char noise, unsigned char red, unsigned char green,
        unsigned char blue, unsigned char brightness, unsigned char speed) = 0;

    virtual void Smoke(const Vector& origin, int modelIndex, float scale, float framerate) = 0;
    virtual void Sparks(const Vector& position, int nMagnitude = 1, int nTrailLength = 1, const Vector* pvecDir = NULL) = 0;
    virtual void Dust(const Vector& pos, const Vector& dir, float size, float speed) = 0;
    virtual void MuzzleFlash(const Vector& vecOrigin, const Vector& vecAngles, float flScale, int iType) = 0;
    virtual void MetalSparks(const Vector& position, const Vector& direction) = 0;
    virtual void EnergySplash(const Vector& position, const Vector& direction, bool bExplosive = false) = 0;
    virtual void Ricochet(const Vector& position, const Vector& direction) = 0;
    virtual float Time() = 0;
    virtual bool IsServer() = 0;
    virtual void SuppressEffectsSounds(bool bSuppress) = 0;

   /* void DispatchEffect(const CEffectData& data)
    {
        using type_fn = bool(__cdecl*)(const CEffectData&);
        static auto fn = reinterpret_cast<type_fn>(Utils::PatternScan("client.dll", "55 8B EC 8B 4D 08 F3 0F 10 51 ? 8D"));
        fn(data);
    }*/
};
