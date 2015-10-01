/*****************************************************************
 *  Copyright (c) 2015, Xudong Liu.                              *
 *  All rights reserved.                                         *
 *****************************************************************/

/*
 * formula_label.h
 */

#ifndef FORMULA_LABEL_
#define FORMULA_LABEL_

#include "parson.h"

#define cOutcomeVectorSize 20

typedef enum { false, true } bool;

typedef enum {
  eFormulaAtomTypeEquals = 0,
  eFormulaAtomTypeGT,
  eFormulaAtomTypeLT,
} FormulaAtomType;

typedef struct Atom {
  FormulaAtomType type;
  unsigned int variable; /* lhs */
  unsigned int value; /* rhs */
} Atom;
/* Example: 'x_4'
   type==eFormulaAtomTypeEquals, 
   variable==4,
   value==1
*/

typedef enum {
  eFormulaTypeFalse = 0,
  eFormulaTypeTrue,
  eFormulaTypeAtom, /* 2 */
  eFormulaTypeAnd,
  eFormulaTypeOr,
  eFormulaTypeNot,
  eFormulaTypeImplication,
  eFormulaTypeEquivalence
} FormulaType;
    
typedef struct Formula {
  FormulaType type;
  struct Atom* atom;
  struct Formula* lhs; /* Note: unary operators like "not" have *no* lhs */
  struct Formula* rhs;
} Formula;

/* vector of values representing an outcome */
typedef unsigned int outcomeVector[cOutcomeVectorSize];

/** a stack of pointers to formulas; used as a pool of memory that
    needs to be freed later */
typedef struct FormulasStack {
  Formula *formula;
  struct FormulasStack *next;
} FormulasStack;


/* --------------------------------------------------------- 
   basic operations
*/

/* allocate and initialize a new formula */
Formula* formula_alloc(void);

/* make a deep copy */
void formula_copy(const Formula* in, Formula* out);

/* free memory used by formula */
void formula_free(Formula* p);

/* --------------------------------------------------------- 
   Semantics
*/

int formula_eval(const Formula *formula, const outcomeVector vector);

/* push negation down to leaf nodes */
void formula_normalize(Formula *formula);

/* negate the given formula */
void formula_negate(Formula *formula);

/* --------------------------------------------------------- 
   (De)serialize from/to JSON
*/

/* read (deserialize) a propoerty from json */
void formula_read(Formula *formula, JSON_Object *json);

JSON_Value* formula_toJSON(Formula *formula);


#endif
