
#pragma once

#include "MathLib.h"

class AFrame : public PackObj
{
public:
	AFrame();

	BOOL UnPack(BYTE **ppData, ULONG iSize); // For legacy purposes

	virtual void Pack(BinaryWriter *pWriter) override;
	virtual bool UnPack(BinaryReader *pReader) override;
	BOOL IsValid();

	Vector m_origin;
	Quaternion m_angles;
};

// wait a second, these aren't really inherited.. fix this.
class Frame : public AFrame, public PackableJson
{
public:
	Frame();

	ULONG pack_size() { return ((sizeof(float) * 3) + (sizeof(float) * 4)); } // For legacy purposes.
	ULONG Pack(BYTE **ppData, ULONG iSize); // For legacy purposes.
	BOOL UnPack(BYTE **ppData, ULONG iSize); // For legacy purposes.

	virtual void Pack(BinaryWriter *pWriter) override;
	virtual bool UnPack(BinaryReader *pReader) override;

	virtual void PackJson(json &writer) override;
	virtual bool UnPackJson(const json &reader) override;

	BOOL IsValid();
	BOOL IsValidExceptForHeading();

	BOOL is_vector_equal(const Frame& F) const;
	BOOL is_quaternion_equal(const Frame& F) const;

	void cache();
	void cache_quaternion();

	void combine(const Frame* a, const AFrame* b);
	void combine(const Frame* a, const AFrame* b, const Vector* scale);
	void rotate(const Vector& angles);
	void grotate(Vector& angles);
	void set_rotate(const Quaternion& angles);
	void euler_set_rotate(Vector Angles, int order);
	Vector globaltolocal(const Vector& point) const;
	Vector globaltolocalvec(const Vector& point) const;
	Vector localtoglobal(const Vector& point) const;
	Vector localtoglobalvec(const Vector& point) const;
	Vector localtolocal(const Frame& frame, const Vector& point) const;
	void subtract1(const Frame* a, const AFrame* b);
	void subtract2(Frame *_f1, Frame *_f2);

	void rotate_around_axis_to_vector(int axis, const Vector& target);
	void set_heading(float DegreeHeading);
	Vector get_vector_heading();
	void set_vector_heading(const Vector& Heading);

	float get_heading(void) const;

	void interpolate_rotation(const Frame &from, const Frame &to, float t);

	// Inherited:
	// Vector m_origin;
	// Quaternion m_angles;

	union {
		float m[3][3];

		struct
		{
			float m00, m01, m02; // 0x1C, 0x20, 0x24 0, 1, 2 | 0, 1, 2 | 0, 4, 8
			float m10, m11, m12; // 0x28, 0x2C, 0x30 3, 4, 5 | 4, 5, 6 | 1, 5, 9
			float m20, m21, m22; // 0x34, 0x38, 0x3C 6, 7, 8 | 8, 9, 10 | 2, 6, 10
		};

		struct
		{
			float _11, _12, _13;
			float _21, _22, _23;
			float _31, _32, _33;
		};
	};
};

class Position : public PackObj, public PackableJson
{
public:
	Position(uint32_t landcell, const Vector &origin = Vector(0, 0, 0), const Quaternion &angles = Quaternion(1, 0, 0, 0));
	Position(const Position& Pos);
	Position(const char *str); // custom
	Position();
	~Position();

	virtual ULONG pack_size() {
		// For legacy purposes.
		return (sizeof(uint32_t) + frame.pack_size());
	}
	
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	void PackOrigin(BinaryWriter *pWriter);
	bool UnPackOrigin(BinaryReader *pReader);

	Position& operator=(const Position& Pos);
	BOOL operator==(const Position& Pos) const;

	Vector localtolocal(const Position& Pos, const Vector& Offset) const;
	
	Vector localtoglobalvec(const Vector &v) const;
	Vector globaltolocalvec(const Vector& point) const;

	Vector localtoglobal(const Vector& point) const;
	Vector localtoglobal(const Position& Pos, const Vector& point) const;

	Vector *get_origin();

	Vector get_offset(const Position& pos) const;
	Vector get_offset(const Position& pos, const Vector& rotation) const;
	float distance(const Position& pos) const;
	float distance_squared(const Position& pos) const; // custom
	float heading(const Position &p);
	float heading_diff(const Position &p);
	uint32_t determine_quadrant(float height, const Position& p) const;

	void adjust_to_outside();

	Position add_offset(const Vector& v) const; // custom

	Frame subtract2(Position *p2);

	static float cylinder_distance(float r1, float h1, const Position &p1, float r2, float h2, const Position &p2);
	static float cylinder_distance_no_z(float r1, const Position &p1, float r2, const Position &p2);

	uint32_t objcell_id;
	Frame frame;
};

inline std::ostream& operator<<(std::ostream& out, const Position &pos)
{
	auto flags = out.flags();
	auto prec = out.precision();
	char fill = out.fill();

	out << "0x" << std::setfill('0') << std::setw(8) << std::hex << pos.objcell_id;
	out << std::setfill(fill) << std::setw(0) << std::dec << std::fixed << std::setprecision(4);
	out << " [" << pos.frame.m_origin << "] " << pos.frame.m_angles;

	out.precision(prec);
	out.flags(flags);

	return out;
}

inline bool operator !(const Position &position) {
	return !position.objcell_id ? true : false;
}

class PositionPack : public PackObj // used on network
{
public:
	PositionPack() { }
	virtual ~PositionPack() { }

	virtual void Pack(BinaryWriter *pWriter) override;
	virtual bool UnPack(BinaryReader *pReader) override;

	short instance_timestamp = 0; // 0x4
	short position_timestamp = 0; // 0x6
	short teleport_timestamp = 0; // 0x8
	short force_position_timestamp = 0; // 0xA
	Position position; // 0xC
	Vector velocity; // 0x54
	uint32_t placement_id = 0; // 0x60
	int has_contact = 0; // 0x64
};

