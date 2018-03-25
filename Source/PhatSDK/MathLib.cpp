

#include "StdAfx.h"
#include "MathLib.h"
#include "Frame.h"

inline Vector CrossProduct(const Vector& a, const Vector& b)
{
	return Vector(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

float FindVectorZ(const Vector& p1, const Vector& p2, const Vector& p3, float x, float y)
{
	Vector v1 = p3 - p1;
	Vector v2 = p2 - p1;
	Vector normal = CrossProduct(v1, v2).normalize();

	float poo = -((normal.x * p1.x) + (normal.y * p1.y) + (normal.z * p1.z));
	float z = (-((normal.x * x) + (normal.y * y) + poo)) / normal.z;

	return z;
}

#if 0
// Checks if the sum of two 32-bit values will overflow.
void inline WillOF(DWORD v1, DWORD v2, BOOL *bResult)
{
#ifdef _WIN32
	__asm {
		mov eax, DWORD PTR[v1]
		add eax, DWORD PTR[v2]
		xor eax, eax
		adc eax, eax

		mov DWORD PTR[bResult], eax
	}
#else
	*bResult = TRUE;
#endif
}
#endif

/* Matrix courtesy of Asriel */
matrix::matrix() {
	data[0][0] = 1.0f;
	data[0][1] = 0.0f;
	data[0][2] = 0.0f;
	data[0][3] = 0.0f;
	data[1][0] = 0.0f;
	data[1][1] = 1.0f;
	data[1][2] = 0.0f;
	data[1][3] = 0.0f;
	data[2][0] = 0.0f;
	data[2][1] = 0.0f;
	data[2][2] = 1.0f;
	data[2][3] = 0.0f;
	data[3][0] = 0.0f;
	data[3][1] = 0.0f;
	data[3][2] = 0.0f;
	data[3][3] = 1.0f;
}

/* General definition */
void matrix::define(float xa, float xb, float xc, float xd, float ya, float yb, float yc, float yd,
	float za, float zb, float zc, float zd) {
	data[0][0] = xa;
	data[0][1] = xb;
	data[0][2] = xc;
	data[0][3] = xd;
	data[1][0] = ya;
	data[1][1] = yb;
	data[1][2] = yc;
	data[1][3] = yd;
	data[2][0] = za;
	data[2][1] = zb;
	data[2][2] = zc;
	data[2][3] = zd;
}

/* Definition by Quaternion */
void matrix::defineByQuaternion(float qw, float qx, float qy, float qz) {
	data[0][0] = 1 - (2 * (qy * qy)) - (2 * (qz * qz));
	data[1][0] = (2 * qx * qy) - (2 * qw * qz);
	data[2][0] = (2 * qx * qz) + (2 * qw * qy);

	data[0][1] = (2 * qx * qy) + (2 * qw * qz);
	data[1][1] = 1 - (2 * (qx * qx)) - (2 * (qz * qz));
	data[2][1] = (2 * qy * qz) - (2 * qw * qx);

	data[0][2] = (2 * qx * qz) - (2 * qw * qy);
	data[1][2] = (2 * qy * qz) + (2 * qw * qx);
	data[2][2] = 1 - (2 * (qx * qx)) - (2 * (qy * qy));
}

/* Definition by Eulerian angular rotation */

#pragma warning(disable : 4244)

void matrix::defineByRotation(float roll, float pitch, float yaw) {
	double sr, sp, sy, cr, cp, cy;
	matrix mp, my;

	sr = sin(DEG2RAD(roll));
	cr = cos(DEG2RAD(roll));
	sp = sin(DEG2RAD(pitch));
	cp = cos(DEG2RAD(pitch));
	sy = sin(DEG2RAD(yaw));
	cy = cos(DEG2RAD(yaw));

	data[0][0] = data[1][1] = cr;
	data[1][0] = -(data[0][1] = sr);

	mp.data[1][1] = mp.data[2][2] = cp;
	mp.data[1][2] = -(mp.data[2][1] = sp);

	my.data[0][0] = my.data[2][2] = cy;
	my.data[0][2] = -(my.data[2][0] = sy);

	multiply(my);
	multiply(mp);
}

void matrix::applyRotation(float roll, float pitch, float yaw) {
	matrix mat;

	mat.defineByRotation(roll, pitch, yaw);
	multiply(mat);
}

void matrix::applyTranslation(float x, float y, float z) {
	data[0][3] = x;
	data[1][3] = y;
	data[2][3] = z;
}

/* Apply this matrix to a vector */
void matrix::applyToVector(Vector &vect) {
	float xo, yo, zo, wo;
	xo = vect.x;
	yo = vect.y;
	zo = vect.z;
	wo = 1.0f;

	vect.x = xo * data[0][0] + yo * data[1][0] + zo * data[2][0] + wo * data[3][0];
	vect.y = xo * data[0][1] + yo * data[1][1] + zo * data[2][1] + wo * data[3][0];
	vect.z = xo * data[0][2] + yo * data[1][2] + zo * data[2][2] + wo * data[3][0];
}

void matrix::copy(matrix &dest) {
	dest.define(data[0][0], data[0][1], data[0][2], data[0][3],
		data[1][0], data[1][1], data[1][2], data[1][3],
		data[2][0], data[2][1], data[2][2], data[2][3]);
}

void matrix::multiply(matrix second)
{
	matrix temp;
	copy(temp);

	for (int j = 0; j < 4; j++)
		for (int i = 0; i < 4; i++)
			data[i][j] = temp.data[i][0] * second.data[0][j] +
			temp.data[i][1] * second.data[1][j] +
			temp.data[i][2] * second.data[2][j] +
			temp.data[i][3] * second.data[3][j];
}

ULONG Vec2D::pack_size()
{
	return(sizeof(float) * 2);
}

Vector cross_product(const Vector& v1, const Vector& v2)
{
	return Vector(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

BOOL Vector::is_zero() const
{
	if ((fabs(x) >= F_EPSILON) || (fabs(y) >= F_EPSILON) || (fabs(z) >= F_EPSILON))
		return FALSE;

	return TRUE;
}

float Vector::get_heading()
{
	Vector heading(x, y, 0);

	if (heading.normalize_check_small())
		return 0.0f;

	return fmod(450 - RAD2DEG(atan2(heading.y, heading.x)), 360);
}

float Vector::dot_product(const Vector& v) const
{
	return((x * v.x) + (y * v.y) + (z * v.z));
}

BOOL Vector::normalize_check_small()
{
	float nfactor = magnitude();

	if (nfactor < F_EPSILON)
		return TRUE; // Too small.

	nfactor = 1 / nfactor;

	x *= nfactor;
	y *= nfactor;
	z *= nfactor;

	return FALSE;
}

Vector& Vector::normalize()
{
	float nfactor = 1 / magnitude();

	x *= nfactor;
	y *= nfactor;
	z *= nfactor;

	return *this;
}

BOOL Vector::IsValid() const
{
	if (_isnan(x) || _isnan(y) || _isnan(z))
		return FALSE;

	return TRUE;
}

BOOL Quaternion::IsValid() const
{
	if (_isnan(w) || _isnan(x) || _isnan(y) || _isnan(z))
		return FALSE;

	float magn = (w * w) + (x * x) + (y * y) + (z * z);

	if (_isnan(magn))
		return FALSE;

	if ((F_EPSILON * 5.0f) < fabs(magn - 1.0f))
		return FALSE;

	return TRUE;
}

void Quaternion::normalize()
{
	float magn = 1 / magnitude();

	w *= magn;
	x *= magn;
	y *= magn;
	z *= magn;
}

float Quaternion::dot_product(const Quaternion& q) const
{
	return((w * q.w) + (x * q.x) + (y * q.y) + (z * q.z));
}

Plane::Plane()
{
}

Plane::Plane(Vector& Vect1, Vector& Vect2)
{
	m_normal = Vect1;
	m_dist = -(m_normal.dot_product(Vect2));
}

void Plane::snap_to_plane(Vector *offset)
{
	if (fabs(m_normal.z) > F_EPSILON)
	{
		offset->z = 0;
		offset->z = -((offset->y * m_normal.y) + (offset->x * m_normal.x) + (m_normal.z * 0.0) + m_dist) * (1.0 / m_normal.z) - 1.0 / m_normal.z * -m_dist;
	}
}

float Plane::dot_product(const Vector& point)
{
	return m_normal.dot_product(point) + m_dist;
}

ULONG Plane::pack_size()
{
	return (m_normal.pack_size() + sizeof(float));
}

void Plane::Pack(BinaryWriter *pWriter)
{
	m_normal.Pack(pWriter);
	pWriter->Write<float>(m_dist);
}

BOOL Plane::UnPack(BYTE** ppData, ULONG iSize)
{
	if (iSize < pack_size())
		return FALSE;

	// Plane Normal
	UNPACK_OBJ(m_normal);

	// Plane Distance
	UNPACK(float, m_dist);

	return TRUE;
}

bool Plane::UnPack(BinaryReader *pReader)
{
	m_normal.UnPack(pReader);
	m_dist = pReader->Read<float>();
	return true;
}

BOOL Plane::compute_time_of_intersection(const Ray& ray, float *time)
{
	float dot = m_normal.dot_product(ray.m_direction);

	if (F_EPSILON > abs(dot))
		return FALSE;

	float depth = dot_product(ray.m_origin) * (-1.0 / dot);
	*time = depth;

	if (depth < 0.0f)
		return FALSE;

	return TRUE;
}

Sidedness Plane::which_side(const Vector& point, float near_dist)
{
	float dp = dot_product(point);

	if (dp > near_dist)
		return Sidedness::POSITIVE;

	if (dp < -near_dist)
		return Sidedness::NEGATIVE;

	return Sidedness::IN_PLANE;
}

Plane Plane::localtoglobal(class Position &to, const Position &from, const Plane &local_plane)
{
	Plane result;

	Vector point = local_plane.m_normal * -local_plane.m_dist;
	Vector point2 = from.frame.localtoglobalvec(local_plane.m_normal);
	Vector point3 = to.localtoglobal(from, point);

	result.m_normal = point2;
	result.m_dist = -point2.dot_product(point3);
	return result;
}

bool is_newer_event_stamp(WORD oldStamp, WORD stamp)
{
	if (abs(stamp - oldStamp) >= 0x8000)
		return (oldStamp > stamp);
	else
		return (stamp > oldStamp);
}

Sidedness Plane::intersect_box(BBox *box)
{
	Sidedness result;
	
	float dp = dot_product(box->m_Min);
	if (dp <= F_EPSILON)
	{
		if (dp >= -F_EPSILON)
			return Sidedness::CROSSING;

		result = Sidedness::NEGATIVE;
	}
	else
	{
		result = Sidedness::POSITIVE;
	}

	Vector v;

	v.x = box->m_Max.x;
	v.y = box->m_Max.y;
	v.z = box->m_Max.z;
	if (result != which_side(v, F_EPSILON))
		return Sidedness::CROSSING;

	v.x = box->m_Max.x;
	v.y = box->m_Min.y;
	v.z = box->m_Min.z;
	if (result != which_side(v, F_EPSILON))
		return Sidedness::CROSSING;

	v.x = box->m_Min.x;
	v.y = box->m_Max.y;
	v.z = box->m_Min.z;
	if (result != which_side(v, F_EPSILON))
		return Sidedness::CROSSING;

	v.x = box->m_Min.x;
	v.y = box->m_Min.y;
	v.z = box->m_Max.z;
	if (result != which_side(v, F_EPSILON))
		return Sidedness::CROSSING;

	v.x = box->m_Max.x;
	v.y = box->m_Max.y;
	v.z = box->m_Min.z;
	if (result != which_side(v, F_EPSILON))
		return Sidedness::CROSSING;

	v.x = box->m_Max.x;
	v.y = box->m_Min.y;
	v.z = box->m_Max.z;
	if (result != which_side(v, F_EPSILON))
		return Sidedness::CROSSING;

	v.x = box->m_Min.x;
	v.y = box->m_Max.y;
	v.z = box->m_Max.z;
	if (result != which_side(v, F_EPSILON))
		return Sidedness::CROSSING;

	return result;
}
