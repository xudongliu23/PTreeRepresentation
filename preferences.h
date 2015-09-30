/*****************************************************************
 *  Copyright (c) 2015, Palo Alto Research Center.               *
 *  All rights reserved.                                         *
 *****************************************************************/

/*
 * preferences.h:  preference logic for digihub
 */

#ifndef PREFERENCES_
#define PREFERENCES_

/* #include "hpg_types.h" */
#include "parson.h"
#define cHpgStateVectorSize 21

/* make prototypes usable from C++ */
/* #ifdef __cplusplus */
/* extern "C" { */
/* #endif */

typedef enum { false, true } bool;

typedef struct Atom {
  PreferenceAtomType type;
  unsigned int variable; /* lhs */
  unsigned int value; /* rhs */
} Atom;

typedef enum {
		ePreferencePTreeTypeRoot2 = 0,
		ePreferencePTreeTypeRoot1l,
		ePreferencePTreeTypeRoot1m,
		ePreferencePTreeTypeRoot1r,
		ePreferencePTreeTypeRoot0
} PreferencePTreeType;
    
typedef struct PTree {
  PreferencePTreeType type;
	UInt32 id;
  /* for Atoms, doh! */
  struct Atom* atom;
  /* for Ands and Ors */
  struct PTree* lst;
  struct PTree* mst;
  struct PTree* rst;
} PTree;

typedef struct ShadowedPTree {
  PTree* real;
  PTree* shadow;
} ShadowedPTree;

/* vector of state variable value we'll use to check declarative
   constraints */
typedef unsigned int stateVector[cHpgStateVectorSize];

/* typedef struct PointerArray { */
/*   PTree **array = NULL; */
/*   UInt32 size; */
/*   UInt32 count = 0; */
/* } PointerArray; */

/* --------------------------------------------------------- 
   pointer arrays, needed for each (time-bound) use of properties
*/

/* PointerArray *property_pointerArray_alloc(void); */
/* void property_pointerArray_free(void); */

/** a stack of pointers to properties; used as a pool of memory that
    needs to be freed later */
typedef struct PropertiesStack {
  PTree *property;
  struct PropertiesStack *next;
} PropertiesStack;


/* --------------------------------------------------------- 
   basic operations
*/

/* allocate and initialize a new property */
PTree* property_alloc(void);

/* make a deep copy */
void property_copy(const PTree* in, PTree* out);

/* free memory used by property */
void property_free(PTree* p);

/* --------------------------------------------------------- 
   Semantics
*/

/* bool propertyIsTrue(const PTree *property, const stateVector vector); */
int property_eval(const PTree *property, const stateVector vector);

/* progress a shadowed property based on current state; This
   progresses the shadow, and if it changed, it will create new
   properties in the ShadowedPTree and reset the old shadow. This
   way we can avoid allocating memory until there is a change, while
   still enabling the use of the same (shadowed) property in multiple
   search nodes without copying. */
bool property_progress(ShadowedPTree *property, const stateVector vector);

/* push negation down to leaf nodes */
void property_normalize(PTree *property);

/* negate the given property */
void property_negate(PTree *property);

/* check if all the eventually subformulas are satisfied in the goal state */
bool isGoodGoal(const PTree* property, const stateVector goalVector);

/* --------------------------------------------------------- 
   (De)serialize from/to JSON
*/

/* read (deserialize) a propoerty from json */
void property_read(PTree *property, JSON_Object *json);

JSON_Value* property_toJSON(PTree *property);


#endif
