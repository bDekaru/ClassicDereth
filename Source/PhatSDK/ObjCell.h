
#pragma once

#include "PartCell.h"
#include "ObjCache.h"
#include "DArray.h"
#include "Frame.h"
#include "HashData.h"
#include "Transition.h"
#include "LandDefs.h"

class CPhysicsObj;
class CLandBlock;
class LIGHTOBJ;

enum DetectionType
{
	NoChangeDetection = 0x0,
	EnteredDetection = 0x1,
	LeftDetection = 0x2,
	FORCE_DetectionType_32_BIT = 0x7FFFFFFF,
};

class DetectionInfo
{
public:
	uint32_t object_id;
	DetectionType object_status;
};

class GlobalVoyeurInfo
{
public:
	unsigned int object_iid;
	unsigned int type;
	int ref_count;
};

class CShadowObj : public LongHashData
{
public:
	CShadowObj();
	virtual ~CShadowObj();

	void set_physobj(CPhysicsObj *pObject);

	CPhysicsObj *physobj; // 0x0C
	uint32_t m_CellID; // 0x10
	class CObjCell *cell; // 0x14
};

class CObjCell : public CPartCell, public DBObj
{
public:
	CObjCell();
	virtual ~CObjCell();

	void add_object(CPhysicsObj *pObject);
	void add_shadow_object(CShadowObj *_object, unsigned int num_shadow_cells);
	void add_light(LIGHTOBJ *Light);

	void remove_object(CPhysicsObj *pObject);
	void remove_shadow_object(CShadowObj *_object);
	void remove_light(LIGHTOBJ *Light);

	void release_objects();

	CPhysicsObj *get_object(uint32_t iid);

	virtual TransitionState find_collisions(class CTransition *);
	virtual TransitionState find_env_collisions(class CTransition *transition);
	TransitionState find_obj_collisions(class CTransition *transition);
	TransitionState check_entry_restrictions(class CTransition *transition);

	virtual void find_transit_cells(const unsigned int num_parts, CPhysicsPart **parts, struct CELLARRAY *cell_array);
	virtual void find_transit_cells(Position *p, const unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array, SPHEREPATH *path);

	virtual BOOL point_in_cell(const Vector &pt);

	static void find_cell_list(Position *p, unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array, CObjCell **curr_cell, SPHEREPATH *path);
	static void find_cell_list(Position *p, unsigned int num_cylsphere, CCylSphere *cylsphere, CELLARRAY *cell_array, SPHEREPATH *path);
	static void find_cell_list(Position *p, class CSphere *sphere, CELLARRAY *cell_array, SPHEREPATH *path);
	static void find_cell_list(CELLARRAY *cell_array, CObjCell **check_cell, SPHEREPATH *path);

	static CObjCell *GetVisible(uint32_t cell_id, bool bDoPostLoad = true);

	int check_collisions(CPhysicsObj *object);
	LandDefs::WaterType get_block_water_type();
	double get_water_depth(Vector *point);

	uint32_t water_type = 0; // 0x50
	Position pos; // 0x54

	uint32_t num_objects = 0; // 0x9C
	DArray<CPhysicsObj *> object_list; // 0xA0

	// Light Information
	uint32_t num_lights = 0; // 0xB0
	DArray<const LIGHTOBJ *> light_list; // 0xB4

	uint32_t num_shadow_objects = 0;
	DArray<CShadowObj *> shadow_object_list; 

	uint32_t restriction_obj = 0;
	ClipPlaneList **clip_planes = 0;
	uint32_t num_stabs = 0;
	uint32_t *stab_list = 0;
	BOOL seen_outside = 0;
	LongNIValHash<GlobalVoyeurInfo> *voyeur_table; // 0xEC
	CLandBlock *myLandBlock_ = 0; // 0xF0

};

