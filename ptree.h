/*****************************************************************
 *  Copyright (c) 2015, Palo Alto Research Center.               *
 *  All rights reserved.                                         *
 *****************************************************************/

/*
 * ptree.h
 */


#ifndef PREFERENCES_
#define PREFERENCES_

/* #include "hpg_types.h" */
#include "parson.h"

typedef unsigned int UInt32;

typedef enum { false, true } bool;

typedef enum {
		ePreferencePtreeTypeRoot2 = 0,
		ePreferencePtreeTypeRoot1l,
		ePreferencePtreeTypeRoot1m,
		ePreferencePtreeTypeRoot1r,
		ePreferencePtreeTypeRoot0
} PreferencePtreeType;
    
typedef struct Ptree {
  PreferencePtreeType type;
	UInt32 id;
  struct Ptree* lst;
  struct Ptree* mst;
  struct Ptree* rst;
} Ptree;

typedef struct PtreesStack {
  Ptree *ptree;
  struct PtreesStack *next;
} PtreesStack;


/* --------------------------------------------------------- 
   basic operations
*/

/* allocate and initialize a new ptree */
Ptree* ptree_alloc(void);

/* make a deep copy */
void ptree_copy(const Ptree* in, Ptree* out);

/* free memory used by ptree */
void ptree_free(Ptree* p);

/* --------------------------------------------------------- 
   (De)serialize from/to JSON
*/

/* read (deserialize) a propoerty from json */
void ptree_read(Ptree *ptree, JSON_Object *json);

JSON_Value* ptree_toJSON(Ptree *ptree);

UInt32 number_of_leaves(Ptree *ptree);


#endif
