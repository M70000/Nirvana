#include "Math.hpp"
#include "../valve_sdk/csgostructs.hpp"

#ifdef VECTOR_PARANOIA
#define CHECK_VALID( _v)	Assert( (_v).IsValid() )
#else
#ifdef GNUC
#define CHECK_VALID( _v)
#else
#define CHECK_VALID( _v)	0
#endif
#endif

namespace Math
{
	//--------------------------------------------------------------------------------
	void FixAngles( QAngle& angles ) {
		Normalize3( angles );
		ClampAngles( angles );
	}
	//--------------------------------------------------------------------------------
	float VectorDistance( const Vector& v1, const Vector& v2 ) {
		return FASTSQRT( pow( v1.x - v2.x, 2 ) + pow( v1.y - v2.y, 2 ) + pow( v1.z - v2.z, 2 ) );
	}
	//--------------------------------------------------------------------------------
	QAngle CalcAngle( const Vector& src, const Vector& dst ) {
		QAngle vAngle;
		Vector delta( ( src.x - dst.x ), ( src.y - dst.y ), ( src.z - dst.z ) );
		double hyp = sqrt( delta.x*delta.x + delta.y*delta.y );

		vAngle.pitch = float( atanf( float( delta.z / hyp ) ) * 57.295779513082f );
		vAngle.yaw = float( atanf( float( delta.y / delta.x ) ) * 57.295779513082f );
		vAngle.roll = 0.0f;

		if( delta.x >= 0.0 )
			vAngle.yaw += 180.0f;

		return vAngle;
	}
	//--------------------------------------------------------------------------------
	float GetFOV( const QAngle& viewAngle, const QAngle& aimAngle ) {
		Vector ang, aim;

		AngleVectors( viewAngle, aim );
		AngleVectors( aimAngle, ang );

		return RAD2DEG( acos( aim.Dot( ang ) / aim.LengthSqr( ) ) );
	}
	//--------------------------------------------------------------------------------
	bool IntersectionBoundingBox(  const Vector& src, const Vector& dir, const Vector& min, const Vector& max, Vector* hit_point ) {
		/*
		Fast Ray-Box Intersection
		by Andrew Woo
		from "Graphics Gems", Academic Press, 1990
		*/

		constexpr auto NUMDIM = 3;
		constexpr auto RIGHT = 0;
		constexpr auto LEFT = 1;
		constexpr auto MIDDLE = 2;

		bool inside = true;
		char quadrant[NUMDIM];
		int i;

		// Rind candidate planes; this loop can be avoided if
		// rays cast all from the eye(assume perpsective view)
		Vector candidatePlane;
		for ( i = 0; i < NUMDIM; i++ ) {
			if ( src[i] < min[i] ) {
				quadrant[i] = LEFT;
				candidatePlane[i] = min[i];
				inside = false;
			} else if ( src[i] > max[i] ) {
				quadrant[i] = RIGHT;
				candidatePlane[i] = max[i];
				inside = false;
			} else {
				quadrant[i] = MIDDLE;
			}
		}

		// Ray origin inside bounding box
		if ( inside ) {
			if ( hit_point )
				*hit_point = src;
			return true;
		}

		// Calculate T distances to candidate planes
		Vector maxT;
		for ( i = 0; i < NUMDIM; i++ ) {
			if ( quadrant[i] != MIDDLE && dir[i] != 0.f )
				maxT[i] = ( candidatePlane[i] - src[i] ) / dir[i];
			else
				maxT[i] = -1.f;
		}

		// Get largest of the maxT's for final choice of intersection
		int whichPlane = 0;
		for ( i = 1; i < NUMDIM; i++ ) {
			if ( maxT[whichPlane] < maxT[i] )
				whichPlane = i;
		}

		// Check final candidate actually inside box
		if ( maxT[whichPlane] < 0.f )
			return false;

		for ( i = 0; i < NUMDIM; i++ ) {
			if ( whichPlane != i ) {
				float temp = src[i] + maxT[whichPlane] * dir[i];
				if ( temp < min[i] || temp > max[i] ) {
					return false;
				} else if ( hit_point ) {
					( *hit_point )[i] = temp;
				}
			} else if ( hit_point ) {
				( *hit_point )[i] = candidatePlane[i];
			}
		}

		// ray hits box
		return true;
	}
	//--------------------------------------------------------------------------------
	void CorrectMovement(QAngle old_angles, CUserCmd* cmd, float old_forwardmove, float old_sidemove) {
		float delta_view;
		float f1;
		float f2;

		if (old_angles.yaw < 0.f)
			f1 = 360.0f + old_angles.yaw;
		else
			f1 = old_angles.yaw;

		if (cmd->viewangles.yaw < 0.0f)
			f2 = 360.0f + cmd->viewangles.yaw;
		else
			f2 = cmd->viewangles.yaw;

		if (f2 < f1)
			delta_view = abs(f2 - f1);
		else
			delta_view = 360.0f - abs(f1 - f2);

		delta_view = 360.0f - delta_view;

		cmd->forwardmove = cos(DEG2RAD(delta_view)) * old_forwardmove + cos(DEG2RAD(delta_view + 90.f)) * old_sidemove;
		cmd->sidemove = sin(DEG2RAD(delta_view)) * old_forwardmove + sin(DEG2RAD(delta_view + 90.f)) * old_sidemove;
	}
	//--------------------------------------------------------------------------------
	void MovementFix( CUserCmd* m_Cmd, QAngle wish_angle, QAngle old_angles ) {
		if( old_angles.pitch != wish_angle.pitch || old_angles.yaw != wish_angle.yaw || old_angles.roll != wish_angle.roll ) {
			Vector wish_forward, wish_right, wish_up, cmd_forward, cmd_right, cmd_up;

			auto viewangles = old_angles;
			auto movedata = Vector( m_Cmd->forwardmove, m_Cmd->sidemove, m_Cmd->upmove );
			viewangles.Normalize( );

			if( !( g_LocalPlayer->m_fFlags( ) & FL_ONGROUND ) && viewangles.roll != 0.f )
				movedata.y = 0.f;

			AngleVectors( wish_angle, wish_forward, wish_right, wish_up );
			AngleVectors( viewangles, cmd_forward, cmd_right, cmd_up );

			auto v8 = sqrt( wish_forward.x * wish_forward.x + wish_forward.y * wish_forward.y ), v10 = sqrt( wish_right.x * wish_right.x + wish_right.y * wish_right.y ), v12 = sqrt( wish_up.z * wish_up.z );

			Vector wish_forward_norm( 1.0f / v8 * wish_forward.x, 1.0f / v8 * wish_forward.y, 0.f ),
				wish_right_norm( 1.0f / v10 * wish_right.x, 1.0f / v10 * wish_right.y, 0.f ),
				wish_up_norm( 0.f, 0.f, 1.0f / v12 * wish_up.z );

			auto v14 = sqrt( cmd_forward.x * cmd_forward.x + cmd_forward.y * cmd_forward.y ), v16 = sqrt( cmd_right.x * cmd_right.x + cmd_right.y * cmd_right.y ), v18 = sqrt( cmd_up.z * cmd_up.z );

			Vector cmd_forward_norm( 1.0f / v14 * cmd_forward.x, 1.0f / v14 * cmd_forward.y, 1.0f / v14 * 0.0f ),
				cmd_right_norm( 1.0f / v16 * cmd_right.x, 1.0f / v16 * cmd_right.y, 1.0f / v16 * 0.0f ),
				cmd_up_norm( 0.f, 0.f, 1.0f / v18 * cmd_up.z );

			auto v22 = wish_forward_norm.x * movedata.x, v26 = wish_forward_norm.y * movedata.x, v28 = wish_forward_norm.z * movedata.x, v24 = wish_right_norm.x * movedata.y, v23 = wish_right_norm.y * movedata.y, v25 = wish_right_norm.z * movedata.y, v30 = wish_up_norm.x * movedata.z, v27 = wish_up_norm.z * movedata.z, v29 = wish_up_norm.y * movedata.z;

			Vector correct_movement;
			correct_movement.x = cmd_forward_norm.x * v24 + cmd_forward_norm.y * v23 + cmd_forward_norm.z * v25 + ( cmd_forward_norm.x * v22 + cmd_forward_norm.y * v26 + cmd_forward_norm.z * v28 ) + ( cmd_forward_norm.y * v30 + cmd_forward_norm.x * v29 + cmd_forward_norm.z * v27 );
			correct_movement.y = cmd_right_norm.x * v24 + cmd_right_norm.y * v23 + cmd_right_norm.z * v25 + ( cmd_right_norm.x * v22 + cmd_right_norm.y * v26 + cmd_right_norm.z * v28 ) + ( cmd_right_norm.x * v29 + cmd_right_norm.y * v30 + cmd_right_norm.z * v27 );
			correct_movement.z = cmd_up_norm.x * v23 + cmd_up_norm.y * v24 + cmd_up_norm.z * v25 + ( cmd_up_norm.x * v26 + cmd_up_norm.y * v22 + cmd_up_norm.z * v28 ) + ( cmd_up_norm.x * v30 + cmd_up_norm.y * v29 + cmd_up_norm.z * v27 );

			correct_movement.x = std::clamp( correct_movement.x, -450.f, 450.f );
			correct_movement.y = std::clamp( correct_movement.y, -450.f, 450.f );
			correct_movement.z = std::clamp( correct_movement.z, -320.f, 320.f );

			m_Cmd->forwardmove = correct_movement.x;
			m_Cmd->sidemove = correct_movement.y;
			m_Cmd->upmove = correct_movement.z;

			m_Cmd->buttons &= ~( IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD );
			if( m_Cmd->sidemove != 0.0 ) {
				if( m_Cmd->sidemove <= 0.0 )
					m_Cmd->buttons |= IN_MOVELEFT;
				else
					m_Cmd->buttons |= IN_MOVERIGHT;
			}

			if( m_Cmd->forwardmove != 0.0 ) {
				if( m_Cmd->forwardmove <= 0.0 )
					m_Cmd->buttons |= IN_BACK;
				else
					m_Cmd->buttons |= IN_FORWARD;
			}
		}
	}
	//--------------------------------------------------------------------------------
	void NormalizeAngles(QAngle& angles)
	{
		for (auto i = 0; i < 3; i++)
		{
			while (angles[i] < -180.0f)
			{
				angles[i] += 360.0f;
			}
			while (angles[i] > 180.0f)
			{
				angles[i] -= 360.0f;
			}
		}
	}
	//--------------------------------------------------------------------------------
	float RandomFloat(float min, float max)
	{
		static auto random_float = reinterpret_cast<float(*)(float, float)>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomFloat"));
		return random_float(min, max);
	}
	//--------------------------------------------------------------------------------
	void ClampAngles(QAngle& angles) {
		if (angles.pitch > 89.0f) angles.pitch = 89.0f;
		else if (angles.pitch < -89.0f) angles.pitch = -89.0f;

		if (angles.yaw > 180.0f) angles.yaw = 180.0f;
		else if (angles.yaw < -180.0f) angles.yaw = -180.0f;

		angles.roll = 0;
	}
	//--------------------------------------------------------------------------------
	void VectorTransform( const Vector& in1, const matrix3x4_t& in2, Vector& out ) {
		out[0] = in1.Dot( in2[0] ) + in2[0][3];
		out[1] = in1.Dot( in2[1] ) + in2[1][3];
		out[2] = in1.Dot( in2[2] ) + in2[2][3];
	}
	//--------------------------------------------------------------------------------
	void AngleVectors( const QAngle &angles, Vector& forward ) {
		float	sp, sy, cp, cy;

		DirectX::XMScalarSinCos( &sp, &cp, DEG2RAD( angles[0] ) );
		DirectX::XMScalarSinCos( &sy, &cy, DEG2RAD( angles[1] ) );

		forward.x = cp * cy;
		forward.y = cp * sy;
		forward.z = -sp;
	}
	//--------------------------------------------------------------------------------
	void AngleVectors( const QAngle &angles, Vector& forward, Vector& right, Vector& up ) {
		float sr, sp, sy, cr, cp, cy;

		DirectX::XMScalarSinCos( &sp, &cp, DEG2RAD( angles[0] ) );
		DirectX::XMScalarSinCos( &sy, &cy, DEG2RAD( angles[1] ) );
		DirectX::XMScalarSinCos( &sr, &cr, DEG2RAD( angles[2] ) );

		forward.x = ( cp * cy );
		forward.y = ( cp * sy );
		forward.z = ( -sp );
		right.x = ( -1 * sr * sp * cy + -1 * cr * -sy );
		right.y = ( -1 * sr * sp * sy + -1 * cr *  cy );
		right.z = ( -1 * sr * cp );
		up.x = ( cr * sp * cy + -sr * -sy );
		up.y = ( cr * sp * sy + -sr * cy );
		up.z = ( cr * cp );
	}
	//--------------------------------------------------------------------------------
	void VectorSubtract(const Vector& a, const Vector& b, Vector& c)
	{
		CHECK_VALID(a);
		CHECK_VALID(b);
		c.x = a.x - b.x;
		c.y = a.y - b.y;
		c.z = a.z - b.z;
	}
	vec_t VectorLength(const Vector& v)
	{
		CHECK_VALID(v);
		return (vec_t)FASTSQRT(v.x * v.x + v.y * v.y + v.z * v.z);
	}
	void VectorAngles1337(const Vector& forward, Vector& angles) {
		float	tmp, yaw, pitch;

		if (forward[1] == 0 && forward[0] == 0) {
			yaw = 0;
			if (forward[2] > 0)
				pitch = 270;
			else
				pitch = 90;
		}
		else {
			yaw = (atan2(forward[1], forward[0]) * 180 / DirectX::XM_PI);
			if (yaw < 0)
				yaw += 360;

			tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
			pitch = (atan2(-forward[2], tmp) * 180 / DirectX::XM_PI);
			if (pitch < 0)
				pitch += 360;
		}

		angles[0] = pitch;
		angles[1] = yaw;
		angles[2] = 0;
	}

	void VectorAngles( const Vector& forward, QAngle& angles ) {
		float	tmp, yaw, pitch;

		if( forward[1] == 0 && forward[0] == 0 ) {
			yaw = 0;
			if( forward[2] > 0 )
				pitch = 270;
			else
				pitch = 90;
		} else {
			yaw = ( atan2( forward[1], forward[0] ) * 180 / DirectX::XM_PI );
			if( yaw < 0 )
				yaw += 360;

			tmp = sqrt( forward[0] * forward[0] + forward[1] * forward[1] );
			pitch = ( atan2( -forward[2], tmp ) * 180 / DirectX::XM_PI );
			if( pitch < 0 )
				pitch += 360;
		}

		angles[0] = pitch;
		angles[1] = yaw;
		angles[2] = 0;
	}
	//--------------------------------------------------------------------------------
	static bool screen_transform( const Vector& in, Vector& out ) {
		static auto& w2sMatrix = g_EngineClient->WorldToScreenMatrix( );

		out.x = w2sMatrix.m[0][0] * in.x + w2sMatrix.m[0][1] * in.y + w2sMatrix.m[0][2] * in.z + w2sMatrix.m[0][3];
		out.y = w2sMatrix.m[1][0] * in.x + w2sMatrix.m[1][1] * in.y + w2sMatrix.m[1][2] * in.z + w2sMatrix.m[1][3];
		out.z = 0.0f;

		float w = w2sMatrix.m[3][0] * in.x + w2sMatrix.m[3][1] * in.y + w2sMatrix.m[3][2] * in.z + w2sMatrix.m[3][3];

		if( w < 0.001f ) {
			out.x *= 100000;
			out.y *= 100000;
			return false;
		}

		out.x /= w;
		out.y /= w;

		return true;
	}
	//--------------------------------------------------------------------------------
	bool WorldToScreen( const Vector& in, Vector& out ) {
		if( screen_transform( in, out ) ) {
			int w, h;
			g_EngineClient->GetScreenSize( w, h );

			out.x = ( w / 2.0f ) + ( out.x * w ) / 2.0f;
			out.y = ( h / 2.0f ) - ( out.y * h ) / 2.0f;

			return true;
		}
		return false;
	}
	//--------------------------------------------------------------------------------
}






























































































































































































































































































































































































































































































































































































































































































