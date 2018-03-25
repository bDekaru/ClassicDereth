
#pragma once

#include "LegacyPackObj.h"
#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "Packable.h"
#include "GameEnums.h"

#define PI 3.1415926535897f
#define RAD2DEG( x ) ( (x) * (180.f / PI) )
#define DEG2RAD( x ) ( (x) * (PI / 180.f) )

class matrix {
public:
	matrix();
	void define(float xa, float xb, float xc, float xd, float ya, float yb, float yc, float yd, float za, float zb, float zc, float zd);
	void defineByRotation(float roll, float pitch, float yaw);
	void defineByQuaternion(float qw, float qx, float qy, float qz);
	void applyRotation(float roll, float pitch, float yaw);
	void applyTranslation(float x, float y, float z);
	void applyToVector(class Vector &vect);
	void multiply(matrix second);
	void copy(matrix &dest);
	float data[4][4];
};

class Vec2D
{
public:
	inline Vec2D() {
	}

	inline Vec2D(float _x, float _y) {
		x = _x;
		y = _y;
	}

	inline Vec2D& operator+=(const Vec2D& quant) {
		x += quant.x;
		y += quant.y;
		return *this;
	}

	ULONG pack_size();

	inline ULONG Pack(BYTE** ppData, ULONG iSize) { // For legacy purposes
		PACK(float, x);
		PACK(float, y);
		return pack_size();
	}
	inline BOOL UnPack(BYTE** ppData, ULONG iSize) { // For legacy purposes
		if (iSize < pack_size())
			return FALSE;

		UNPACK(float, x);
		UNPACK(float, y);
		return TRUE;
	}

	inline void Pack(BinaryWriter *pWriter) {
		pWriter->Write<float>(x);
		pWriter->Write<float>(y);
	}
	inline bool UnPack(BinaryReader *pReader) {
		x = pReader->Read<float>();
		y = pReader->Read<float>();
		return true;
	}

	float x, y;
};

class Vector
{
public:
	inline Vector() {
		x = 0;
		y = 0;
		z = 0;
	}

	inline Vector(float _x, float _y, float _z) {
		x = _x;
		y = _y;
		z = _z;
	}
	inline Vector operator*(const float amount) const {
		return Vector(x * amount, y * amount, z * amount);
	}
	inline Vector operator*(const Vector& quant) const {
		return Vector(x * quant.x, y * quant.y, z * quant.z);
	}
	inline Vector& operator*=(const float amount) {
		x *= amount;
		y *= amount;
		z *= amount;
		return *this;
	}
	inline Vector& operator*=(const Vector& quant) {
		x *= quant.x;
		y *= quant.y;
		z *= quant.z;
		return *this;
	}

	inline Vector operator/(const float amount) const {
		return Vector(x / amount, y / amount, z / amount);
	}
	inline Vector& operator/=(const float amount) {
		x /= amount;
		y /= amount;
		z /= amount;
		return *this;
	}
	inline Vector& operator/=(const Vector& quant) {
		x /= quant.x;
		y /= quant.y;
		z /= quant.z;
		return *this;
	}

	inline Vector operator-(const Vector& quant) const {
		return Vector(x - quant.x, y - quant.y, z - quant.z);
	}
	inline Vector& operator-=(const Vector& quant) {
		x -= quant.x;
		y -= quant.y;
		z -= quant.z;
		return *this;
	}

	inline Vector operator+(const Vector& quant) const {
		return Vector(x + quant.x, y + quant.y, z + quant.z);
	}
	inline Vector& operator+=(const Vector& quant) {
		x += quant.x;
		y += quant.y;
		z += quant.z;
		return *this;
	}

	inline operator const float *() const {
		return &x;
	}
	inline operator float *() {
		return &x;
	}

	float magnitude() const {
		return (float)sqrt((x * x) + (y * y) + (z * z));
	}

	float mag_squared() const {
		return (float)((x * x) + (y * y) + (z * z));
	}

	float sum_of_square() const {
		return ((x * x) + (y * y) + (z * z));
	}

	inline ULONG pack_size() {
		return(sizeof(float) * 3);
	}


	inline ULONG Pack(BYTE** ppData, ULONG iSize) { // For legacy purposes
		PACK(float, x);
		PACK(float, y);
		PACK(float, z);
		return pack_size();
	}
	inline BOOL UnPack(BYTE** ppData, ULONG iSize) { // For legacy purposes
		if (iSize < pack_size())
			return FALSE;

		UNPACK(float, x);
		UNPACK(float, y);
		UNPACK(float, z);
		return TRUE;
	}

	inline void Pack(BinaryWriter *pWriter) {
		pWriter->Write<float>(x);
		pWriter->Write<float>(y);
		pWriter->Write<float>(z);
	}
	inline bool UnPack(BinaryReader *pReader) {
		x = pReader->Read<float>();
		y = pReader->Read<float>();
		z = pReader->Read<float>();
		return true;
	}

	inline void PackJson(json& writer) {
		writer["x"] = x;
		writer["y"] = y;
		writer["z"] = z;
	}
	inline bool UnPackJson(const json& reader) {
		x = reader["x"];
		y = reader["y"];
		z = reader["z"];
		return true;
	}

	BOOL IsValid() const;
	float dot_product(const Vector& v) const;
	Vector& normalize();
	BOOL normalize_check_small();
	BOOL is_zero() const;
	float get_heading();

	BOOL is_equal(const Vector& v) const
	{
		if (fabs(x - v.x) >= F_EPSILON)
			return FALSE;
		if (fabs(y - v.y) >= F_EPSILON)
			return FALSE;
		if (fabs(z - v.z) >= F_EPSILON)
			return FALSE;
		return TRUE;
	}

	bool operator==(const Vector& v) { return is_equal(v) ? true : false; }
	bool operator!=(const Vector& v) { return !(*this == v); }

	float x, y, z;
};

float FindVectorZ(const Vector& p1, const Vector& p2, const Vector& p3, float x, float y);
Vector cross_product(const Vector& v1, const Vector& v2);

// Imaginary class, either entirely inlined by Turbine or non-existant.
class Quaternion
{
public:
	inline Quaternion() {
	}

	inline Quaternion(float _w, float _x, float _y, float _z) {
		w = _w;
		x = _x;
		y = _y;
		z = _z;
	}

	inline Quaternion operator*(const float amount) const {
		return Quaternion(w * amount, x * amount, y * amount, z * amount);
	}
	inline Quaternion operator*(const Quaternion &other) const {
		return Quaternion(w * other.w, x * other.x, y * other.y, z * other.z);
	}
	inline Quaternion& operator*=(const float amount) {
		w *= amount;
		x *= amount;
		y *= amount;
		z *= amount;
		return *this;
	}
	inline Quaternion& operator*=(const Quaternion &other) {
		w *= other.w;
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this;
	}


	inline Quaternion operator+(const Quaternion &quant) const {
		return Quaternion(w + quant.w, x + quant.x, y + quant.y, z + quant.z);
	}
	inline Quaternion& operator+=(const Quaternion &quant) {
		w += quant.w;
		x += quant.x;
		y += quant.y;
		z += quant.z;
		return *this;
	}

	// Should probably be renamed. It's not the length of the quaternion!!
	inline float magnitude() const {
		return (float)sqrt((x * x) + (y * y) + (z * z) + (w * w));
	}

	inline ULONG Pack(BYTE** ppData, ULONG iSize) { // For legacy purposes
		PACK(float, w);
		PACK(float, x);
		PACK(float, y);
		PACK(float, z);
		return (sizeof(float) * 4);
	}
	inline BOOL UnPack(BYTE** ppData, ULONG iSize) { // For legacy purposes
		UNPACK(float, w);
		UNPACK(float, x);
		UNPACK(float, y);
		UNPACK(float, z);
		return TRUE;
	}

	inline void Pack(BinaryWriter *pWriter) {
		pWriter->Write<float>(w);
		pWriter->Write<float>(x);
		pWriter->Write<float>(y);
		pWriter->Write<float>(z);
	}
	inline bool UnPack(BinaryReader *pReader) {
		w = pReader->Read<float>();
		x = pReader->Read<float>();
		y = pReader->Read<float>();
		z = pReader->Read<float>();
		return true;
	}

	inline void PackJson(json& writer) {
		writer["w"] = w;
		writer["x"] = x;
		writer["y"] = y;
		writer["z"] = z;
	}
	inline bool UnPackJson(const json& reader) {
		w = reader["w"];
		x = reader["x"];
		y = reader["y"];
		z = reader["z"];
		return true;
	}

	BOOL IsValid() const;
	void normalize();
	float dot_product(const Quaternion& q) const;

	bool operator==(const Quaternion& q) { return w == q.w && x == q.x && y == q.y && z == q.z; }
	bool operator!=(const Quaternion& q) { return !(*this == q); }

	float w, x, y, z;
};

class Ray
{
public:
	Vector m_origin;
	Vector m_direction;
};

class Plane
{
public:
	// Constructors
	Plane();
	Plane(Vector& Vect1, Vector& Vect2);

	// Pack Functions
	ULONG pack_size();
	BOOL UnPack(BYTE** ppData, ULONG iSize); // For legacy purposes
	void Pack(BinaryWriter *pWriter);
	bool UnPack(BinaryReader *pReader);

	// Math Functions
	void snap_to_plane(Vector *offset);
	float dot_product(const Vector& point);
	Sidedness which_side(const Vector& point, float near_dist);
	BOOL compute_time_of_intersection(const Ray& ray, float *time);
	Sidedness intersect_box(class BBox *box);

	static Plane localtoglobal(class Position &to, const Position &from, const Plane &local_plane);

	Vector m_normal;
	float m_dist;
};

bool is_newer_event_stamp(WORD oldStamp, WORD stamp);


