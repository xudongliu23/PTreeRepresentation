/*****************************************************************
 *  Copyright (c) 2015, Xudong Liu.                              *
 *  All rights reserved.                                         *
 *****************************************************************/

#include "formula_label.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ALLOC(T) malloc(sizeof(T))

#define myfree(X) if (X != NULL) { free(X); X = NULL; }


/* --------------------------------------------------------- */

bool formula_atom_isTrue(Atom *atom, const outcomeVector vector);
JSON_Value* formula_atom_toJSON(Atom *atom);
void formula_atom_read(Atom *atom, JSON_Object *json);
bool formula_progress_aux(Formula *formula, const outcomeVector vector);
void formula_shallowCopy(const Formula* in, Formula* out);


/* --------------------------------------------------------- 
   memory pool
*/

/* PointerArray *formula_pointerArray_alloc() { */
/*   PointerArray *rtv = malloc(sizeof *rtv); */
/*   rtv->array = malloc(size * sizeof *(rtv->array)); */
/*   rtv->size = PROPERTY_POINTER_ARRAY_SIZE; */
/* }; */

/* void formula_pointerArray_grow(PointerArray* pointerarray) { */
/*   UInt32 newSize = pointerarray->size * 2; */
/*   Formula **newArray = malloc(newSize * sizeof *(pointerarray->array)); */
/*   memcpy(newArray, pointerarray->array, pointerarray->size * sizeof *(pointerarray->array)); */
/*   pointerarray->size = newSize; */
/* }; */

/* void formula_pointerArray_add(PointerArray *array, Formula* p) { */
/*   if (array->count >= array->size) { */
/*     // increase pointer array size */
/*     formula_pointerArray_grow(array); */
/*   } */
/*   array->array[array->count++] = p; */
/* } */

/* void formula_pointerArray_free(PointerArray *array) { */
/*   if (array != NULL && array->array != NULL) { */
/*     for (UInt32 i = 0; i < array->count; i++) { */
/*       free(array->array[i]); */
/*     } */
/*     free(array->array); */
/*     array->array = NULL; */
/*   } */
/* } */

/* --------------------------------------------------------- 
   basic operations
*/
Formula* formula_alloc() {
  Formula* rtv = malloc(sizeof *rtv);
  /* printf("alloc %p\n", rtv);  */
  rtv->atom = NULL;
  rtv->lhs = NULL;
  rtv->rhs = NULL;
  /* formula_pointerArray_add(rtv); */
  return rtv;
}

void formula_free(Formula* p) {
  /* printf(" formula_free %p -- type: %d\n", p, p->type); */
  if (p != NULL) {
    switch (p->type) {
    case eFormulaTypeAtom:
      myfree(p->atom);
      break;
    case eFormulaTypeAnd:
    case eFormulaTypeOr:
    case eFormulaTypeImplication:
    case eFormulaTypeEquivalence:
      formula_free(p->lhs);
      formula_free(p->rhs);
      myfree(p->lhs);
      myfree(p->rhs);
      break;
    case eFormulaTypeNot:
      formula_free(p->rhs);
      myfree(p->rhs);
      break;
    default:
      /* true, false: nothing to do */
      break;
    }
  }
}

void formula_copy(const Formula* in, Formula* out) {
  /* printf("formula_copy %p -> %p\n", in, out); */
  out->type = in->type;
  switch (in->type) {
  case eFormulaTypeAtom:
    out->atom = ALLOC(Atom);
    *(out->atom) = *(in->atom);
    break;
  case eFormulaTypeAnd:
  case eFormulaTypeOr:
    out->lhs = formula_alloc();
    formula_copy(in->lhs, out->lhs);
    out->rhs = formula_alloc();
    formula_copy(in->rhs, out->rhs);
    break;
  case eFormulaTypeNot:
    out->rhs = formula_alloc();
    formula_copy(in->rhs, out->rhs);
    break;
  default:
    /* true, false: nothing to do */
    break;
  }
}

void formula_shallowCopy(const Formula* in, Formula* out) {
  out->type = in->type;
  out->atom = in->atom;
  out->lhs = in->lhs;
  out->rhs = in->rhs;
}

/* --------------------------------------------------------- 
   Semantics
*/

/* Semantics of Atoms */
bool formula_atom_isTrue(Atom *atom, const outcomeVector vector) {
  return 
    ((atom->type == eFormulaAtomTypeEquals
      && vector[atom->variable] == atom->value)
     || (atom->type == eFormulaAtomTypeGT
         && vector[atom->variable] > atom->value) 
     || (atom->type == eFormulaAtomTypeLT
         && vector[atom->variable] < atom->value) );
}

/* Semantics of Formulas */

/* @return formula is 1: true, 0: false */
int formula_eval(const Formula *formula, const outcomeVector vector) {
  int l, r;
  switch (formula->type) {
  case eFormulaTypeFalse: return 0; break;
  case eFormulaTypeTrue: return 1; break;
  case eFormulaTypeAtom:
    return (formula_atom_isTrue(formula->atom, vector) ? 1 : 0);
    break;
    
  case eFormulaTypeAnd:
    l = formula_eval(formula->lhs, vector);
    if (l == 0) {
      return 0;
    } else {
      r = formula_eval(formula->rhs, vector);
      return (r == 0 ? 0 : 1); 
    }
    break;

  case eFormulaTypeOr:
    l = formula_eval(formula->lhs, vector);
    if (l == 1) {
      return 1;
    } else {
      r = formula_eval(formula->rhs, vector);
      return (r == 1 ? 1 : 0);
    }
    break;

  case eFormulaTypeImplication:
    l = formula_eval(formula->lhs, vector);
    if (l == 1) {
      return 1;
    } else {
      r = formula_eval(formula->rhs, vector);
      return (r == 1 ? 1 : 0);
    }
    break;

  case eFormulaTypeNot:
    r = formula_eval(formula->rhs, vector);
    return (r == 1 ? 0 : 1);

  default:
    break;
  }
  // this shouldn't happen
  return 0;
}

void formula_negate(Formula *formula) {
  Formula tmp;
  Formula *tmp2;
  switch (formula->type) {
  case eFormulaTypeFalse:
    formula->type = eFormulaTypeTrue;
    break;
    
  case eFormulaTypeTrue:
    formula->type = eFormulaTypeFalse;
    break;

  case eFormulaTypeAtom:
    formula->rhs = formula_alloc();
    *formula->rhs = *formula;
    formula->type = eFormulaTypeNot;
    break;

  case eFormulaTypeAnd:
    formula->type = eFormulaTypeOr;
    formula_negate(formula->lhs);
    formula_negate(formula->rhs);
    break;

  case eFormulaTypeOr:
    formula->type = eFormulaTypeAnd;
    formula_negate(formula->lhs);
    formula_negate(formula->rhs);
    break;

  case eFormulaTypeImplication:
    formula->type = eFormulaTypeAnd;
    formula_negate(formula->rhs);
    break;

  case eFormulaTypeEquivalence:
    formula_negate(formula->lhs);  // negate either one
    break;

  case eFormulaTypeNot:
    /* double negation */
    tmp2 = formula->rhs;
    tmp = *formula->rhs;
    *formula = tmp;
    myfree(tmp2);
    break;

  default:
    // shouldn't happen
    printf("(formula_negate) THIS SHOULDN'T HAPPEN\n");
    break;
  }
}

void formula_normalize(Formula *formula) {
  /* this could be made more efficient by caching whether or not the
     formula is normalized already. would then need to update that
     flag upon any changes or make sure that all changes leave it in
     normal form again afterwards, but it would us from rechecking
     that each time around */
  switch (formula->type) {
  case eFormulaTypeNot:
    if (formula->rhs->type != eFormulaTypeAtom) {
      formula_negate(formula->rhs);
      /* formula_copy(formula->rhs, formula); */
      Formula tmp = *formula->rhs;
      Formula *tmp2 = formula->rhs;
      *formula = tmp;
      myfree(tmp2);
    }
    break;

  case eFormulaTypeAnd:
  case eFormulaTypeOr:
  case eFormulaTypeImplication:
  case eFormulaTypeEquivalence:
    formula_normalize(formula->lhs);
    formula_normalize(formula->rhs);
    break;

  default:
    // nothing to do
    break;
  }
}

/* --------------------------------------------------------- 
   Deserialize from JSON
*/

void formula_atom_read(Atom *atom, JSON_Object *json) {
  const char* type_string = parson_json_object_get_string(json, "type");
  if (strcmp(type_string, "=") == 0) {
    atom->type = eFormulaAtomTypeEquals;
  } else if (strcmp(type_string, "<") == 0) {
    atom->type = eFormulaAtomTypeLT;
  } else if (strcmp(type_string, ">") == 0) {
    atom->type = eFormulaAtomTypeGT;
  } else {
    printf("(formula_atom_read) unknown type: %s\n", type_string);
  }
  atom->variable = (int)parson_json_object_get_number(json, "variable"); // TODO
  atom->value = (int)parson_json_object_get_number(json, "value");
//printf("atom's type:%u\n",atom->type);
//printf("atom's variable:%u\n",atom->variable);
//printf("atom's value:%u\n",atom->value);
}

void formula_read(Formula *formula, JSON_Object *json) {
  const char* type_string = parson_json_object_get_string(json, "type");
  if (strcmp(type_string, "not") == 0) {
    formula->type = eFormulaTypeNot;
    JSON_Object *rhs = parson_json_object_get_object(json, "value");
    formula->rhs = formula_alloc();
    formula_read(formula->rhs, rhs);

  } else if (strcmp(type_string, "and") == 0) {
    formula->type = eFormulaTypeAnd;
    JSON_Array *array = parson_json_object_get_array(json, "value");
    JSON_Object *lhs = parson_json_array_get_object(array, 0);
    JSON_Object *rhs = parson_json_array_get_object(array, 1);
    formula->lhs = formula_alloc();
    formula->rhs = formula_alloc();
    formula_read(formula->lhs, lhs);
    formula_read(formula->rhs, rhs);   

  } else if (strcmp(type_string, "or") == 0) {
    formula->type = eFormulaTypeOr;
    JSON_Array *array = parson_json_object_get_array(json, "value");
    JSON_Object *lhs = parson_json_array_get_object(array, 0);
    JSON_Object *rhs = parson_json_array_get_object(array, 1);
    formula->lhs = formula_alloc();
    formula->rhs = formula_alloc();
    formula_read(formula->lhs, lhs);
    formula_read(formula->rhs, rhs);   

  } else if (strcmp(type_string, "after") == 0) {
    formula->type = eFormulaTypeAfter;
    JSON_Array *array = parson_json_object_get_array(json, "value");
    JSON_Object *lhs = parson_json_array_get_object(array, 0);
    JSON_Object *rhs = parson_json_array_get_object(array, 1);
    formula->lhs = formula_alloc();
    formula->rhs = formula_alloc();
    formula_read(formula->lhs, lhs);
    formula_read(formula->rhs, rhs);   

  } else if (strcmp(type_string, "always") == 0) {
    formula->type = eFormulaTypeAlways;
    JSON_Object *rhs = parson_json_object_get_object(json, "value");
    formula->rhs = formula_alloc();
    formula_read(formula->rhs, rhs);

  } else if (strcmp(type_string, "eventually") == 0) {
    formula->type = eFormulaTypeEventually;
    JSON_Object *rhs = parson_json_object_get_object(json, "value");
    formula->rhs = formula_alloc();
    formula_read(formula->rhs, rhs);

  } else if (strcmp(type_string, "next") == 0) {
    formula->type = eFormulaTypeNext;
    JSON_Object *rhs = parson_json_object_get_object(json, "value");
    formula->rhs = formula_alloc();
    formula_read(formula->rhs, rhs);

  } else if (strcmp(type_string, "false") == 0) {
    formula->type = eFormulaTypeFalse;

  } else if (strcmp(type_string, "true") == 0) {
    formula->type = eFormulaTypeTrue;

  } else {
    /* default */
    formula->type = eFormulaTypeAtom;
    /* JSON_Object *atom_json = parson_json_object_get_object(json, "value"); */
    formula->atom = ALLOC(Atom);
    formula_atom_read(formula->atom, json);
  }
}

/* --------------------------------------------------------- 
   Serialize
 */

JSON_Value* formula_atom_toJSON(Atom *atom) {
  JSON_Value* json = parson_json_value_init_object();
  JSON_Object* obj = parson_json_object(json);
  switch (atom->type) {
  case eFormulaAtomTypeEquals:
    parson_json_object_set_string(obj, "type", "=");
    break;
  case eFormulaAtomTypeLT:
    parson_json_object_set_string(obj, "type", "<");
    break;
  case eFormulaAtomTypeGT:
    parson_json_object_set_string(obj, "type", ">");
    break;
  default:
    break;
  }
  parson_json_object_set_number(obj, "variable", atom->variable);
  parson_json_object_set_number(obj, "value", atom->value);
  return json;
}

JSON_Value* formula_toJSON(Formula *formula) {
  JSON_Value* json;
  JSON_Object* obj;
  JSON_Value* array_json;
  JSON_Array* array;
  switch (formula->type) {
  case eFormulaTypeFalse:
    json = parson_json_value_init_boolean(false);
    break;

  case eFormulaTypeTrue:
    json = parson_json_value_init_boolean(true);
    break;

  case eFormulaTypeAtom:
    json = formula_atom_toJSON(formula->atom);
    break;

  case eFormulaTypeAnd:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
    parson_json_object_set_string(obj, "type", "and");
    array_json = parson_json_value_init_array();
    array = parson_json_array(array_json);
    parson_json_object_set_value(obj, "value", array_json);
    parson_json_array_append_value(array, formula_toJSON(formula->lhs));
    parson_json_array_append_value(array, formula_toJSON(formula->rhs));
    break;

  case eFormulaTypeOr:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
    parson_json_object_set_string(obj, "type", "or");
    array_json = parson_json_value_init_array();
    array = parson_json_array(array_json);
    parson_json_object_set_value(obj, "value", array_json);
    parson_json_array_append_value(array, formula_toJSON(formula->lhs));
    parson_json_array_append_value(array, formula_toJSON(formula->rhs));
    break;

  case eFormulaTypeAfter:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
    parson_json_object_set_string(obj, "type", "after");
    array_json = parson_json_value_init_array();
    array = parson_json_array(array_json);
    parson_json_object_set_value(obj, "value", array_json);
    parson_json_array_append_value(array, formula_toJSON(formula->lhs));
    parson_json_array_append_value(array, formula_toJSON(formula->rhs));
    break;

  case eFormulaTypeNot:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
    parson_json_object_set_string(obj, "type", "not");
    parson_json_object_set_value(obj, "value", formula_toJSON(formula->rhs));
    break;

  case eFormulaTypeAlways:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
    parson_json_object_set_string(obj, "type", "always");
    parson_json_object_set_value(obj, "value", formula_toJSON(formula->rhs));
    break;

  case eFormulaTypeEventually:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
    parson_json_object_set_string(obj, "type", "eventually");
    parson_json_object_set_value(obj, "value", formula_toJSON(formula->rhs));
    break;

  case eFormulaTypeNext:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
    parson_json_object_set_string(obj, "type", "next");
    parson_json_object_set_value(obj, "value", formula_toJSON(formula->rhs));
    break;

  default:
    break;
  }
  return json;
}
