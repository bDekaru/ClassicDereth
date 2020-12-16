
#include <StdAfx.h>
#include "Frame.h"
#include "LandDefs.h"
#include "LegacyPackObj.h"

AFrame::AFrame()
{
	m_origin = Vector(0, 0, 0);
	m_angles = Quaternion(1.0, 0, 0, 0);
}

BOOL AFrame::UnPack(BYTE **ppData, ULONG iSize)
{
	UNPACK_OBJ(m_origin);
	UNPACK_OBJ(m_angles);

	if (!IsValid())
		return FALSE;

	return TRUE;
}

void AFrame::Pack(BinaryWriter *pWriter)
{
	m_origin.Pack(pWriter);
	m_angles.Pack(pWriter);
}

bool AFrame::UnPack(BinaryReader *pReader)
{
	m_origin.UnPack(pReader);
	m_angles.UnPack(pReader);

	if (pReader->GetLastError() || !IsValid())
		return false;

	return true;
}

BOOL AFrame::IsValid()
{
	if (!m_origin.IsValid())
		return FALSE;

	if (!m_angles.IsValid())
		return FALSE;

	return TRUE;
}

Frame::Frame()
{
	m_origin = Vector(0, 0, 0);
	m_angles = Quaternion(1.0, 0, 0, 0);

	cache();
}

ULONG Frame::Pack(BYTE **ppData, ULONG iSize) // For legacy purposes only
{
	UNFINISHED_LEGACY("Frame::Pack");
	return 0;
}

BOOL Frame::UnPack(BYTE **ppData, ULONG iSize) // For legacy purposes only
{
	if (iSize < pack_size())
		return FALSE;

	UNPACK_OBJ(m_origin);
	UNPACK_OBJ(m_angles);

	if (!IsValid())
		return FALSE;

	cache();

	return TRUE;
}

void Frame::PackJson(json &writer)
{
	m_origin.PackJson(writer["origin"]);
	m_angles.PackJson(writer["angles"]);
}

bool Frame::UnPackJson(const json &reader)
{
	m_origin.UnPackJson(reader["origin"]);
	m_angles.UnPackJson(reader["angles"]);
	return true;
}

BOOL Frame::IsValid()
{
	if (!m_origin.IsValid())
		return FALSE;

	if (!m_angles.IsValid())
		return FALSE;

	return TRUE;
}

BOOL Frame::IsValidExceptForHeading()
{
	if (!m_origin.IsValid())
		return FALSE;

	if (m_angles.IsValid())
		return FALSE;

	return TRUE;
}

DEFINE_PACK(Frame)
{
	m_origin.Pack(pWriter);
	m_angles.Pack(pWriter);
}

DEFINE_UNPACK(Frame)
{
	m_origin.UnPack(pReader);
	m_angles.UnPack(pReader);

	if (!IsValid())
		return false;

	cache();
	return true;
}

void EulGetOrd(int ord, int& i, int& j, int& k, int& h, int& n, int& s, int& f)
{
#define EulFrmS 0
#define EulFrmR 1
#define EulFrm(ord) ((unsigned)(ord)&1)
#define EulRepNo 0
#define EulRepYes 1
#define EulRep(ord) (((unsigned)(ord)>>1)&1)
#define EulParEven 0
#define EulParOdd 1
#define EulSafe "\000\001\002\000"
#define EulNext "\001\002\000\001"

	int o = ord;

	f = o & 1; o >>= 1;
	s = o & 1; o >>= 1;
	n = o & 1; o >>= 1;
	i = EulSafe[o & 3];
	j = EulNext[i + n];
	k = EulNext[i + 1 - n];
	h = s ? k : i;
}

void Frame::euler_set_rotate(Vector Angles, int order)
{
	Quaternion qu;

	float a[3];
	float ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
	int i, j, k, h, n, s, f;
	EulGetOrd(order, i, j, k, h, n, s, f);

	if (f == EulFrmR)
	{
		// Rotate
		float t = Angles.x;
		Angles.x = Angles.z;
		Angles.z = t;
	}

	if (n == EulParOdd)
		Angles.y = -Angles.y;

	ti = Angles.x * 0.5f; tj = Angles.y * 0.5f; th = Angles.z * 0.5f;
	ci = cos(ti);  cj = cos(tj);  ch = cos(th);
	si = sin(ti);  sj = sin(tj);  sh = sin(th);
	cc = ci * ch; cs = ci * sh; sc = si * ch; ss = si * sh;

	if (s == EulRepYes)
	{
		a[i] = cj * (cs + sc);    /* Could speed up with */
		a[j] = sj * (cc + ss);    /* trig identities. */
		a[k] = sj * (cs - sc);
		qu.w = cj * (cc - ss);
	}
	else
	{
		a[i] = cj * sc - sj * cs;
		a[j] = cj * ss + sj * cc;
		a[k] = cj * cs - sj * sc;
		qu.w = cj * cc + sj * ss;
	}

	if (n == EulParOdd)
		a[j] = -a[j];

	qu.x = a[0];
	qu.y = a[1];
	qu.z = a[2];

	set_rotate(qu);
}

void Frame::set_rotate(const Quaternion& angles)
{
	m_angles = angles;
	m_angles.normalize();

	cache();
}

void Frame::rotate_around_axis_to_vector(int axis, const Vector& target)
{
	Vector axe = *((Vector *)(&m00 + (axis * 3)));
	Vector dir = target - (axe * axe.dot_product(target));

	int next_ord[3] = { 2, 0, 1 };

	if (!dir.normalize_check_small())
		dir = *((Vector *)(&m00 + (next_ord[axis] * 3)));

	if ((F_EPSILON * 5.0f) < abs(dir.dot_product(axe)))
		return;

	int axe_x, axe_y;

	axe_x = next_ord[axis];
	*((Vector *)(&m00 + axe_x)) = dir;

	axe_y = next_ord[axe_x];
	*((Vector *)(&m00 + axe_y)) = cross_product(dir, axe);

	// Finish me.
	cache_quaternion();
}

void Frame::set_heading(float DegreeHeading)
{
	float RadianHeading = DEG2RAD(DegreeHeading);

	Vector vechead(sin(RadianHeading), cos(RadianHeading), m12 + (m02 + m22 * 0.0f));

	set_vector_heading(vechead);
}

Vector Frame::get_vector_heading()
{
	Vector heading;
	heading.x = m10;
	heading.y = m11;
	heading.z = m12;
	return heading;
}

void Frame::set_vector_heading(const Vector& Heading)
{
	Vector NormalHeading = Heading;

	if (NormalHeading.normalize_check_small())
		return;

	double zrotate = DEG2RAD(-fmod(450 - RAD2DEG(atan2(NormalHeading.y, NormalHeading.x)), 360));
	double xrotate = asin(NormalHeading.z);

	euler_set_rotate(Vector((float)xrotate, 0, (float)zrotate), 0);
}

void Frame::subtract1(const Frame* a, const AFrame* b)
{
	Quaternion TempQuat;
	TempQuat.w = (((a->m_angles.w * b->m_angles.w) + (a->m_angles.x * b->m_angles.x)) + (a->m_angles.y * b->m_angles.y)) + (a->m_angles.z * b->m_angles.z);
	TempQuat.x = ((a->m_angles.x * b->m_angles.w) - (a->m_angles.w * b->m_angles.x) - (a->m_angles.y * b->m_angles.z)) + (a->m_angles.z * b->m_angles.y);
	TempQuat.y = ((a->m_angles.x * b->m_angles.z) - (a->m_angles.w * b->m_angles.y)) + (a->m_angles.y * b->m_angles.w) - (a->m_angles.z * b->m_angles.x);
	TempQuat.z = (((a->m_angles.x * b->m_angles.y) + (a->m_angles.w * b->m_angles.z)) - (a->m_angles.y * b->m_angles.x)) + (a->m_angles.z * b->m_angles.w);

	set_rotate(TempQuat);

	Vector TempVec;
	TempVec.x = m_origin.x + (-(b->m_origin.x) * m00) + (-(b->m_origin.y) * m10) + (-(b->m_origin.z) * m20);
	TempVec.y = m_origin.y + (-(b->m_origin.x) * m01) + (-(b->m_origin.y) * m11) + (-(b->m_origin.z) * m21);
	TempVec.z = m_origin.z + (-(b->m_origin.x) * m02) + (-(b->m_origin.y) * m12) + (-(b->m_origin.z) * m22);

	m_origin = TempVec;
}

void Frame::combine(const Frame* a, const AFrame* b)
{
	Vector TempVec;
	TempVec.x = a->m_origin.x + (b->m_origin.x * a->m00) + (b->m_origin.y * a->m10) + (b->m_origin.z * a->m20);
	TempVec.y = a->m_origin.y + (b->m_origin.x * a->m01) + (b->m_origin.y * a->m11) + (b->m_origin.z * a->m21);
	TempVec.z = a->m_origin.z + (b->m_origin.x * a->m02) + (b->m_origin.y * a->m12) + (b->m_origin.z * a->m22);

	m_origin = TempVec;

	Quaternion TempQuat;
	TempQuat.w = (((a->m_angles.w * b->m_angles.w) - (a->m_angles.x * b->m_angles.x)) - (a->m_angles.y * b->m_angles.y)) - (a->m_angles.z * b->m_angles.z);
	TempQuat.x = ((a->m_angles.x * b->m_angles.w) + (a->m_angles.w * b->m_angles.x) + (a->m_angles.y * b->m_angles.z)) - (a->m_angles.z * b->m_angles.y);
	TempQuat.y = ((a->m_angles.w * b->m_angles.y) - (a->m_angles.x * b->m_angles.z)) + (a->m_angles.y * b->m_angles.w) + (a->m_angles.z * b->m_angles.x);
	TempQuat.z = (((a->m_angles.x * b->m_angles.y) + (a->m_angles.w * b->m_angles.z)) - (a->m_angles.y * b->m_angles.x)) + (a->m_angles.z * b->m_angles.w);

	set_rotate(TempQuat);
}

void Frame::combine(const Frame* a, const AFrame* b, const Vector* scale)
{
	Vector ScaledVec;
	ScaledVec.x = b->m_origin.x * scale->x;
	ScaledVec.y = b->m_origin.y * scale->y;
	ScaledVec.z = b->m_origin.z * scale->z;

	Vector TempVec;
	TempVec.x = a->m_origin.x + (ScaledVec.x * a->m00) + (ScaledVec.y * a->m10) + (ScaledVec.z * a->m20);
	TempVec.y = a->m_origin.y + (ScaledVec.x * a->m01) + (ScaledVec.y * a->m11) + (ScaledVec.z * a->m21);
	TempVec.z = a->m_origin.z + (ScaledVec.x * a->m02) + (ScaledVec.y * a->m12) + (ScaledVec.z * a->m22);

	m_origin = TempVec;

	Quaternion TempQuat;
	TempQuat.w = (((a->m_angles.w * b->m_angles.w) - (a->m_angles.x * b->m_angles.x)) - (a->m_angles.y * b->m_angles.y)) - (a->m_angles.z * b->m_angles.z);
	TempQuat.x = ((a->m_angles.x * b->m_angles.w) + (a->m_angles.w * b->m_angles.x) + (a->m_angles.y * b->m_angles.z)) - (a->m_angles.z * b->m_angles.y);
	TempQuat.y = ((a->m_angles.w * b->m_angles.y) - (a->m_angles.x * b->m_angles.z)) + (a->m_angles.y * b->m_angles.w) + (a->m_angles.z * b->m_angles.x);
	TempQuat.z = (((a->m_angles.x * b->m_angles.y) + (a->m_angles.w * b->m_angles.z)) - (a->m_angles.y * b->m_angles.x)) + (a->m_angles.z * b->m_angles.w);

	set_rotate(TempQuat);
}

void Frame::rotate(const Vector& angles)
{
	Vector Rotation;

	Rotation.x = (m00 * angles.x) + (m10 * angles.y) + (m20 * angles.z);
	Rotation.y = (m01 * angles.x) + (m11 * angles.y) + (m21 * angles.z);
	Rotation.z = (m02 * angles.x) + (m12 * angles.y) + (m22 * angles.z);

	grotate(Rotation);
}

void Frame::grotate(Vector& angles)
{
	float magn = angles.magnitude();

	if (magn < F_EPSILON)
		return;

	angles.x *= 1 / magn;
	angles.y *= 1 / magn;
	angles.z *= 1 / magn;

	Quaternion qu;

	qu.w = cos(magn * 0.5f);
	qu.x = angles.x * sin(magn * 0.5f);
	qu.y = angles.y * sin(magn * 0.5f);
	qu.z = angles.z * sin(magn * 0.5f);


	Quaternion rot;
	rot.w = (((qu.w * m_angles.w) - (qu.x * m_angles.x)) - (qu.y * m_angles.y)) - (qu.z * m_angles.z);
	rot.x = ((qu.x * m_angles.w) + (qu.w * m_angles.x) + (qu.y * m_angles.z)) - (qu.z * m_angles.y);
	rot.y = ((qu.w * m_angles.y) - (qu.x * m_angles.z)) + (qu.y * m_angles.w) + (qu.z * m_angles.x);
	rot.z = (((qu.x * m_angles.y) + (qu.w * m_angles.z)) - (qu.y * m_angles.x)) + (qu.z * m_angles.w);

	// Normalize Rotation and Cache
	set_rotate(rot);
}

Vector Frame::globaltolocal(const Vector& point) const
{
	Vector point_offset = point - m_origin;

	float x = (point_offset.x * m00) + (point_offset.y * m01) + (point_offset.z * m02);
	float y = (point_offset.x * m10) + (point_offset.y * m11) + (point_offset.z * m12);
	float z = (point_offset.x * m20) + (point_offset.y * m21) + (point_offset.z * m22);

	return Vector(x, y, z);
}

Vector Frame::globaltolocalvec(const Vector& point) const
{
	float x = (point.x * m00) + (point.y * m01) + (point.z * m02);
	float y = (point.x * m10) + (point.y * m11) + (point.z * m12);
	float z = (point.x * m20) + (point.y * m21) + (point.z * m22);

	return Vector(x, y, z);
}

Vector Frame::localtoglobal(const Vector& point) const
{
	float x = m_origin.x + (point.x * m00) + (point.y * m10) + (point.z * m20);
	float y = m_origin.y + (point.x * m01) + (point.y * m11) + (point.z * m21);
	float z = m_origin.z + (point.x * m02) + (point.y * m12) + (point.z * m22);

	return Vector(x, y, z);
}

Vector Frame::localtoglobalvec(const Vector& point) const
{
	float x = (point.x * m00) + (point.y * m10) + (point.z * m20);
	float y = (point.x * m01) + (point.y * m11) + (point.z * m21);
	float z = (point.x * m02) + (point.y * m12) + (point.z * m22);

	return Vector(x, y, z);
}

Vector Frame::localtolocal(const Frame& frame, const Vector& point) const
{
	Vector global_point = frame.localtoglobal(point);
	return globaltolocal(global_point);
}

void Frame::cache_quaternion()
{
	float W, X, Y, Z, S, T;
	T = m00 + m11 + m22 + 1.0f;

	if (T > F_EPSILON)
	{
		S = sqrt(T) * 2;
		X = (m12 - m21) / S;
		Y = (m20 - m02) / S;
		Z = (m01 - m10) / S;
		W = 0.25f * S;
	}
	else
	{
		// I'd rather not reverse this.
		// Ripped from http://skal.planet-d.net/demo/matrixfaq.htm#Q55
		// This matrix is a flipped 3x3 from the documentation's 4x4 :P

		if (m00 > m11 && m00 > m22)
		{ // Column 0: 
			S = sqrt(1.0f + m00 - m11 - m22) * 2;
			X = 0.25f * S;
			Y = (m01 + m10) / S;
			Z = (m20 + m02) / S;
			W = (m12 - m21) / S;
		}
		else if (m11 > m22)
		{ // Column 1: 
			S = sqrt(1.0f + m11 - m00 - m22) * 2;
			X = (m01 + m10) / S;
			Y = 0.25f * S;
			Z = (m12 + m21) / S;
			W = (m20 - m02) / S;
		}
		else
		{ // Column 2:
			S = sqrt(1.0f + m22 - m00 - m11) * 2;
			X = (m20 + m02) / S;
			Y = (m12 + m21) / S;
			Z = 0.25f * S;
			W = (m01 - m10) / S;
		}
	}

	set_rotate(Quaternion(W, X, Y, Z));
}

void Frame::cache()
{
	m00 = (1 - (m_angles.y * (m_angles.y * 2))) - (m_angles.z * (m_angles.z * 2));
	m01 = (m_angles.x * (m_angles.y * 2)) + (m_angles.w * (m_angles.z * 2));
	m02 = (m_angles.x * (m_angles.z * 2)) - (m_angles.w * (m_angles.y * 2));
	m10 = (m_angles.x * (m_angles.y * 2)) - (m_angles.w * (m_angles.z * 2));
	m11 = (1 - (m_angles.x * (m_angles.x * 2))) - (m_angles.z * (m_angles.z * 2));
	m12 = (m_angles.y * (m_angles.z * 2)) + (m_angles.w * (m_angles.x * 2));
	m20 = (m_angles.x * (m_angles.z * 2)) + (m_angles.w * (m_angles.y * 2));
	m21 = (m_angles.y * (m_angles.z * 2)) - (m_angles.w * (m_angles.x * 2));
	m22 = (1 - (m_angles.x * (m_angles.x * 2))) - (m_angles.y * (m_angles.y * 2));
}

BOOL Frame::is_vector_equal(const Frame& F) const
{
	if (fabs(m_origin.x - F.m_origin.x) >= F_EPSILON)
		return FALSE;
	if (fabs(m_origin.y - F.m_origin.y) >= F_EPSILON)
		return FALSE;
	if (fabs(m_origin.z - F.m_origin.z) >= F_EPSILON)
		return FALSE;

	return TRUE;
}

BOOL Frame::is_quaternion_equal(const Frame& F) const
{
	if (fabs(m_angles.w - F.m_angles.w) >= F_EPSILON)
		return FALSE;
	if (fabs(m_angles.x - F.m_angles.x) >= F_EPSILON)
		return FALSE;
	if (fabs(m_angles.y - F.m_angles.y) >= F_EPSILON)
		return FALSE;
	if (fabs(m_angles.z - F.m_angles.z) >= F_EPSILON)
		return FALSE;

	return TRUE;
}

float Frame::get_heading(void) const
{
	// Untested?
	Vector Heading;
	Heading.x = ((m00 + m20) * 0.0f) + m10;
	Heading.y = ((m01 + m21) * 0.0f) + m11;

	return (float) fmod(450 - RAD2DEG(atan2(Heading.y, Heading.x)), 360);
}

void Frame::interpolate_rotation(const Frame &from, const Frame &to, float t)
{
	float cosom = from.m_angles.dot_product(to.m_angles);

	Quaternion q;

	if (cosom < 0)
	{
		cosom = -cosom;
		q = Quaternion(-to.m_angles.w, -to.m_angles.x, -to.m_angles.y, -to.m_angles.z);
	}
	else
	{
		q = to.m_angles;
	}

	float v11;
	float v12;
	if ((1.0 - cosom) <= F_EPSILON)
	{
		v11 = 1.0 - t;
		v12 = t;
	}
	else
	{
		float v8 = acos(cosom);
		float omega = v8;
		float cosoma = 1.0 - t;
		float v10 = 1.0 / sin(v8);
		v11 = v10 * sin(v8 * cosoma);
		v12 = sin(t * omega) * v10;
		if (v11 < 0.0 || v11 > 1.0 || v12 < 0.0 || v12 > 1.0)
		{
			v11 = cosoma;
			v12 = t;
		}
	}

	Quaternion new_q;
	new_q = q*v12 + from.m_angles*v11;
	set_rotate(new_q);
}

void Frame::subtract2(Frame *_f1, Frame *_f2)
{
	float new_qz;
	float new_qy;
	float new_qx; 
	float new_qw;

	m_origin = _f2->globaltolocal(_f1->m_origin);
	new_qz = _f1->m_angles.z * _f2->m_angles.w - _f1->m_angles.y * _f2->m_angles.x + _f1->m_angles.x * _f2->m_angles.y - _f1->m_angles.w * _f2->m_angles.z;
	new_qy = _f1->m_angles.z * _f2->m_angles.x + _f2->m_angles.w * _f1->m_angles.y - _f2->m_angles.y * _f1->m_angles.w - _f1->m_angles.x * _f2->m_angles.z;
	new_qx = _f1->m_angles.x * _f2->m_angles.w - _f1->m_angles.w * _f2->m_angles.x - _f2->m_angles.y * _f1->m_angles.z + _f1->m_angles.y * _f2->m_angles.z;
	new_qw = _f1->m_angles.w * _f2->m_angles.w + _f2->m_angles.y * _f1->m_angles.y + _f1->m_angles.x * _f2->m_angles.x + _f1->m_angles.z * _f2->m_angles.z;
	set_rotate(Quaternion(new_qw, new_qx, new_qy, new_qz));
}

Position::Position(uint32_t landcell, const Vector &origin, const Quaternion &angles)
{
	objcell_id = landcell;
	frame.m_origin = origin;
	frame.m_angles = angles;
	frame.cache();
}

Position::Position(const Position& Pos)
{
	*this = Pos;
}

Position::Position()
{
	objcell_id = 0;
}

Position::Position(const char *str)
{
	uint32_t cell_id = 0;
	float x = 0, y = 0, z = 0;
	float qw = 1, qx = 0, qy = 0, qz = 0;
	int fields = sscanf(str, "%X %f %f %f %f %f %f %f", &cell_id, &x, &y, &z, &qw, &qx, &qy, &qz);

	if (fields == 4 || fields == 8)
	{
		objcell_id = cell_id;
		frame.m_origin = Vector(x, y, z);
		frame.m_angles = Quaternion(qw, qx, qy, qz);
	}
	else
	{
		objcell_id = 0;
	}
}

Position::~Position()
{
}

Position& Position::operator=(const Position& Pos)
{
	objcell_id = Pos.objcell_id;
	frame = Pos.frame;

	return *this;
}

DEFINE_PACK(Position)
{
	pWriter->Write<uint32_t>(objcell_id);
	frame.Pack(pWriter);
}

DEFINE_UNPACK(Position)
{
	objcell_id = pReader->Read<uint32_t>();
	frame.UnPack(pReader);
	return true;
}

DEFINE_PACK_JSON(Position)
{
	writer["objcell_id"] = objcell_id;
	frame.PackJson(writer["frame"]);
}

DEFINE_UNPACK_JSON(Position)
{
	objcell_id = reader["objcell_id"];
	frame.UnPackJson(reader["frame"]);
	return true;
}

void Position::PackOrigin(BinaryWriter *pWriter)
{
	pWriter->Write<uint32_t>(objcell_id);
	frame.m_origin.Pack(pWriter);
}

bool Position::UnPackOrigin(BinaryReader *pReader)
{
	objcell_id = pReader->Read<uint32_t>();
	frame.m_origin.UnPack(pReader);
	return true;
}

BOOL Position::operator==(const Position& Pos) const
{
	if (objcell_id != Pos.objcell_id)
		return FALSE;

	if (!frame.is_vector_equal(Pos.frame))
		return FALSE;

	if (!frame.is_quaternion_equal(Pos.frame))
		return FALSE;

	return TRUE;
}

Vector Position::localtolocal(const Position& Pos, const Vector& Offset) const
{
	Vector cell_offset = Pos.frame.localtoglobal(Offset);

	Vector block_offset = LandDefs::get_block_offset(objcell_id, Pos.objcell_id);

	return frame.globaltolocal(block_offset + cell_offset);
}

Vector Position::localtoglobalvec(const Vector &v) const
{
	return frame.localtoglobalvec(v);
}

Vector Position::globaltolocalvec(const Vector& point) const
{
	return frame.globaltolocalvec(point);
}

Vector Position::localtoglobal(const Position& Pos, const Vector& point) const
{
	Vector cell_offset = Pos.frame.localtoglobal(point);

	Vector block_offset = LandDefs::get_block_offset(objcell_id, Pos.objcell_id);

	return(block_offset + cell_offset);
}

Vector Position::localtoglobal(const Vector& point) const
{
	// DOES NOT factor in block offsets
	return frame.localtoglobal(point);
}

Vector *Position::get_origin()
{
	return &frame.m_origin;
}

// Duplicate function name? sub_42A3C0
Vector Position::get_offset(const Position& pos) const
{
	Vector block_offset = LandDefs::get_block_offset(objcell_id, pos.objcell_id);

	Vector global_offset = block_offset + pos.frame.m_origin - frame.m_origin;

	return global_offset;
}

Vector Position::get_offset(const Position& pos, const Vector& rotation) const
{
	Vector block_offset = LandDefs::get_block_offset(objcell_id, pos.objcell_id);

	Vector rotated_origin = pos.localtoglobal(rotation);

	return(block_offset + rotated_origin - frame.m_origin);
}

float Position::distance(const Position& pos) const
{
	return get_offset(pos).magnitude();
}

float Position::distance_squared(const Position& pos) const
{
	return get_offset(pos).mag_squared();
}

void Position::adjust_to_outside()
{
	LandDefs::adjust_to_outside(&objcell_id, &frame.m_origin);
}

Position Position::add_offset(const Vector& v) const
{
	Position translated = *this;
	translated.frame.m_origin += v;
	return translated;
}


float Position::cylinder_distance(float r1, float h1, const Position &p1, float r2, float h2, const Position &p2)
{
	Vector offset = p1.get_offset(p2);
	float magReach = offset.magnitude() - (r1 + r2);
	
	float v8;
	float v10;

	if (p1.frame.m_origin.z <= p2.frame.m_origin.z)
	{
		v8 = p2.frame.m_origin.z;
		v10 = p1.frame.m_origin.z + h1;
	}
	else
	{
		v8 = p1.frame.m_origin.z;
		v10 = p2.frame.m_origin.z + h2;
	}

	float v11 = v8 - v10;

	// i hope this is right... not 100% sure
	if (v11 > 0)
	{
		if (magReach > 0)
			return sqrt((v11*v11)+(magReach*magReach));

		return v11;
	}
	else
	{
		if (magReach > 0)
			return magReach;

		return -sqrt((v11*v11) + (magReach*magReach));
	}
}

float Position::cylinder_distance_no_z(float r1, const Position &p1, float r2, const Position &p2)
{
	Vector offset = p1.get_offset(p2);
	return offset.magnitude() - (r1 + r2);
}

float Position::heading(const Position &p)
{
	Vector direction = get_offset(p);
	direction.z = 0.0f;

	if (direction.normalize_check_small())
		return 0.0f;

	return (float) fmod(450.0 - RAD2DEG(atan2(direction.y, direction.x)), 360.0);
}

float Position::heading_diff(const Position &p)
{
	return heading(p) - frame.get_heading();
}

uint32_t Position::determine_quadrant(float height, const Position& p) const
{
	Vector hitpoint = localtolocal(p, Vector(0, 0, 0));
	
	uint32_t quadrant;

	if (hitpoint.x < 0.0)
		quadrant = 0x8;
	else
		quadrant = 0x10;

	if (hitpoint.y >= 0.0)
		quadrant |= 0x20;
	else
		quadrant |= 0x40;

	if ((height * 0.333333333) <= hitpoint.z)
	{
		if ((height * 0.66666667) <= hitpoint.z)
			quadrant |= 1; // high
		else
			quadrant |= 2; // medium
	}
	else
	{
		quadrant |= 4; // low
	}

	return quadrant;
}

Frame Position::subtract2(Position *p2)
{
	Frame result;

	result.subtract2(&frame, &p2->frame);
	result.m_origin = p2->localtolocal(*this, Vector(0, 0, 0));
	return result;
}

void PositionPack::Pack(BinaryWriter *pWriter)
{
	BinaryWriter content;
	uint32_t flags = 0;

	if (!position.frame.m_angles.w)
		flags |= 8;
	else
		content.Write<float>(position.frame.m_angles.w);

	if (!position.frame.m_angles.x)
		flags |= 0x10;
	else
		content.Write<float>(position.frame.m_angles.x);

	if (!position.frame.m_angles.y)
		flags |= 0x20;
	else
		content.Write<float>(position.frame.m_angles.y);

	if (!position.frame.m_angles.z)
		flags |= 0x40;
	else
		content.Write<float>(position.frame.m_angles.z);

	if (!velocity.is_zero())
	{
		flags |= 1;
		velocity.Pack(pWriter);
	}

	if (placement_id)
	{
		flags |= 2;
		content.Write<uint32_t>(placement_id);
	}

	if (has_contact)
	{
		flags |= 4;
	}

	pWriter->Write<uint32_t>(flags);
	position.PackOrigin(pWriter);
	pWriter->Write(&content);
	pWriter->Write<WORD>(instance_timestamp); // 0x174
	pWriter->Write<WORD>(position_timestamp); // 0x16e
	pWriter->Write<WORD>(teleport_timestamp); // 0x16c
	pWriter->Write<WORD>(force_position_timestamp); // 0x170
}

bool PositionPack::UnPack(BinaryReader *pReader)
{
	uint32_t flags = pReader->Read<uint32_t>();
	position.UnPackOrigin(pReader);	

	if (flags & 8)
		position.frame.m_angles.w = 0;
	else
		position.frame.m_angles.w = pReader->Read<float>();

	if (flags & 0x10)
		position.frame.m_angles.x = 0;
	else
		position.frame.m_angles.x = pReader->Read<float>();

	if (flags & 0x20)
		position.frame.m_angles.y = 0;
	else
		position.frame.m_angles.y = pReader->Read<float>();

	if (flags & 0x40)
		position.frame.m_angles.z = 0;
	else
		position.frame.m_angles.z = pReader->Read<float>();

	if (flags & 1)
		velocity.UnPack(pReader);
	else
		velocity = Vector(0, 0, 0);

	if (flags & 2)
		placement_id = pReader->Read<uint32_t>();
	else
		placement_id = 0;

	has_contact = (flags & 4) ? TRUE : FALSE;

	instance_timestamp = pReader->Read<WORD>();
	position_timestamp = pReader->Read<WORD>();
	teleport_timestamp = pReader->Read<WORD>();
	force_position_timestamp = pReader->Read<WORD>();

	return true;
}
