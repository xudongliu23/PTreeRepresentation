
#include "preferences.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* #define PROPERTY_POINTER_ARRAY_SIZE 1000 */

#define ALLOC(T) malloc(sizeof(T))

#define myfree(X) if (X != NULL) { free(X); X = NULL; }
// Use for debugging:
/* #define myfree(X) if (X != NULL) { printf("free %p\n", X); free(X); X = NULL; } */

/* #define myfree(X) // #DEBUG */



/* --------------------------------------------------------- */

bool property_atom_isTrue(Atom *atom, const stateVector vector);
JSON_Value* property_atom_toJSON(Atom *atom);
void property_atom_read(Atom *atom, JSON_Object *json);
bool property_progress_aux(Property *property, const stateVector vector);
void property_shallowCopy(const Property* in, Property* out);


/* --------------------------------------------------------- 
   memory pool
*/

/* PointerArray *property_pointerArray_alloc() { */
/*   PointerArray *rtv = malloc(sizeof *rtv); */
/*   rtv->array = malloc(size * sizeof *(rtv->array)); */
/*   rtv->size = PROPERTY_POINTER_ARRAY_SIZE; */
/* }; */

/* void property_pointerArray_grow(PointerArray* pointerarray) { */
/*   UInt32 newSize = pointerarray->size * 2; */
/*   Property **newArray = malloc(newSize * sizeof *(pointerarray->array)); */
/*   memcpy(newArray, pointerarray->array, pointerarray->size * sizeof *(pointerarray->array)); */
/*   pointerarray->size = newSize; */
/* }; */

/* void property_pointerArray_add(PointerArray *array, Property* p) { */
/*   if (array->count >= array->size) { */
/*     // increase pointer array size */
/*     property_pointerArray_grow(array); */
/*   } */
/*   array->array[array->count++] = p; */
/* } */

/* void property_pointerArray_free(PointerArray *array) { */
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
Property* property_alloc() {
  Property* rtv = malloc(sizeof *rtv);
  /* printf("alloc %p\n", rtv);  */
  rtv->atom = NULL;
  rtv->lhs = NULL;
  rtv->rhs = NULL;
  /* property_pointerArray_add(rtv); */
  return rtv;
}

void property_free(Property* p) {
  /* printf(" property_free %p -- type: %d\n", p, p->type); */
  if (p != NULL) {
    switch (p->type) {
    case ePreferencePropertyTypeAtom:
      myfree(p->atom);
      break;
    case ePreferencePropertyTypeAnd:
    case ePreferencePropertyTypeOr:
    case ePreferencePropertyTypeAfter:
      property_free(p->lhs);
      property_free(p->rhs);
      myfree(p->lhs);
      myfree(p->rhs);
      break;
    case ePreferencePropertyTypeNot:
    case ePreferencePropertyTypeAlways:
    case ePreferencePropertyTypeEventually:
    case ePreferencePropertyTypeNext:
      property_free(p->rhs);
      myfree(p->rhs);
      break;
    default:
      /* true, false: nothing to do */
      break;
    }
  }
}

void property_copy(const Property* in, Property* out) {
  /* printf("property_copy %p -> %p\n", in, out); */
  out->type = in->type;
  switch (in->type) {
  case ePreferencePropertyTypeAtom:
    out->atom = ALLOC(Atom);
    *(out->atom) = *(in->atom);
    break;
  case ePreferencePropertyTypeAnd:
  case ePreferencePropertyTypeOr:
  case ePreferencePropertyTypeAfter:
    out->lhs = property_alloc();
    property_copy(in->lhs, out->lhs);
    out->rhs = property_alloc();
    property_copy(in->rhs, out->rhs);
    break;
  case ePreferencePropertyTypeNot:
  case ePreferencePropertyTypeAlways:
  case ePreferencePropertyTypeEventually:
  case ePreferencePropertyTypeNext:
    out->rhs = property_alloc();
    property_copy(in->rhs, out->rhs);
    break;
  default:
    /* true, false: nothing to do */
    break;
  }
}

void property_shallowCopy(const Property* in, Property* out) {
  out->type = in->type;
  out->atom = in->atom;
  out->lhs = in->lhs;
  out->rhs = in->rhs;
}

/* --------------------------------------------------------- 
   Semantics
*/

/* Semantics of Atoms */
bool property_atom_isTrue(Atom *atom, const stateVector vector) {
  return 
    ((atom->type == ePreferenceAtomTypeEquals
      && vector[atom->variable] == atom->value)
     || (atom->type == ePreferenceAtomTypeGT
         && vector[atom->variable] > atom->value) 
     || (atom->type == ePreferenceAtomTypeLT
         && vector[atom->variable] < atom->value) );
}

/* Semantics of Properties */

/* @return property is 1: true, -1: false, 0: not yet decided
   (temporal). */
int property_eval(const Property *property, const stateVector vector) {
  ////// IN
  // In the beginning state or the state where going to any next node falsifies the property,
  // this function just return true.
  if(vector[0] == 0) {
    return 1;
  }
  ////// IN

  int l, r;
  switch (property->type) {
  case ePreferencePropertyTypeAlways:
    if (property_eval(property->rhs, vector) == -1) {
      return -1;
    } else return 0;
    break;

  case ePreferencePropertyTypeEventually:
    if (property_eval(property->rhs, vector) == 1) {
      return 1;
    } else return 0;
    break;

  case ePreferencePropertyTypeNext: return 0; break;
  case ePreferencePropertyTypeFalse: return -1; break;
  case ePreferencePropertyTypeTrue: return 1; break;
  case ePreferencePropertyTypeAfter: return 0; break;
  case ePreferencePropertyTypeAtom:
    return (property_atom_isTrue(property->atom, vector) ? 1 : -1);
    break;
    
  case ePreferencePropertyTypeAnd:
    l = property_eval(property->lhs, vector);
    if (l == -1) {
      return -1;
    } else {
      r = property_eval(property->rhs, vector);
      return (r == -1 ? -1 // surely false
              : (l == 1 && r == 1 ? 1 // surely true
                 : 0 )); // don't know yet
    }
    break;

  case ePreferencePropertyTypeOr:
    l = property_eval(property->lhs, vector);
    if (l == 1) {
      return 1;
    } else {
      r = property_eval(property->rhs, vector);
      return (r == 1 ? 1 // surely true
              : (l == -1 && r == -1 ? -1 // surely false
                 : 0 )); // don't know yet
    }
    break;

  case ePreferencePropertyTypeNot:
    r = property_eval(property->rhs, vector);
    return r * -1;

  default:
    break;
  }
  // this shouldn't happen
  return 0;
}

/* bool propertyIsTrue(const Property *property, const stateVector vector) { */
  /* return property_eval(property, vector) == 1; */
/* } */
bool property_progress(ShadowedProperty *property, const stateVector vector) {
//printf("\nBeginning of property_progress\n");
//printf("shadow=%p\n", property->shadow);
//printf("real=%p\n", property->real);

  ////// IN
  // In the beginning state or the state where going to any next node falsifies the property,
  // this function just return false.
  if(vector[0] == 0) {
    return false;
  }
  ////// IN

  if (property_progress_aux(property->shadow, vector)) {
//printf("changed shadow->type:%u\n", property->shadow->type);
    property_normalize(property->shadow);
    Property *newReal = property_alloc();
    Property *newShadow = property_alloc();
    property_copy(property->shadow, newReal);
    property_copy(property->shadow, newShadow);
    property_copy(property->real, property->shadow); // reset shadow
    property->real = newReal;
    property->shadow = newShadow;
    return true;
  } else {
    return false;
  }
}

/* bool property_progress(Property *property, const stateVector vector,  */
/*                       Property *progressed) { */
/*   // make sure property is normalized, i.e., all negation is at the */
/*   // leaf nodes */
/*   Property tmp; */
/*   /\* printf("progressing %p\n", property); *\/ */
/*   if (property_progress_aux(property, vector, &tmp)) { */
/*     property_copy(&tmp, progressed); */
/*     property_normalize(progressed); */
/*     return true; */
/*   } else { */
/*     property_shallowCopy(property, progressed); */
/*     return false; */
/*   } */
/* } */

/* progress a property based on current state; Note: property *must*
   be in normal form already. Return true if changed.
*/
bool property_progress_aux(Property *property, const stateVector vector) {

  Property tmp;
  int l, r;

//printf("property=%p\n", property);
//printf("vector=%u\n", vector[0]);
//printf("property->type=%u\n", property->type);
//printf("atom=%p\n", property->atom);
//printf("atom->type=%u\n", property->atom->type);

  switch (property->type) {
  case ePreferencePropertyTypeAlways:
    // switch on whether sub-property was progressed
    r = property_eval(property->rhs, vector);
    if (r == -1) {
      // aready false
      property->type = ePreferencePropertyTypeFalse;
      property_free(property->rhs);
      myfree(property->rhs);
//printf("type=%u\n", property->type);
//printf("atom=%p\n", property->atom);
//printf("atom->type=%u\n", property->atom->type);
//printf("atom->variable=%u\n", property->atom->variable);
//printf("atom->value=%u\n", property->atom->value);
//printf("lhs=%p\n", property->lhs);
//printf("lhs->type=%u\n", property->lhs->type);
//printf("rhs=%p\n", property->rhs);
      return true;
    } else if (r == 1) {
      return false;
    } else {
      /* tmp AND property */
      ////// IN
      property->type = ePreferencePropertyTypeAnd;
      property->lhs = property_alloc();
      property_copy(property->rhs, property->lhs);
      property_progress_aux(property->lhs, vector);
      Property* tmp1 = property_alloc();
      tmp1->type = ePreferencePropertyTypeAlways;
      tmp1->rhs = property->rhs;
      property->rhs = tmp1;
      ////// IN
      
      ////// OUT
      //property->type = ePreferencePropertyTypeAnd;
      //property->lhs = property_alloc();
      //property_copy(property->rhs, property->lhs);
      //property_progress_aux(property->lhs, vector);
      ////// OUT
      return true;
    }
    break;

  case ePreferencePropertyTypeEventually:
    r = property_eval(property->rhs, vector);
    if (r == 1) {
      // already true
      property->type = ePreferencePropertyTypeTrue;
      property_free(property->rhs);
      myfree(property->rhs);
      return true;
    } else if (r == -1) {
      return false;
    } else {
      /* tmp OR property; this is necessary because property->rhs
         might be temporal, e.g., property = eventually[next X] */
      ////// IN
      property->type = ePreferencePropertyTypeOr;
      property->lhs = property_alloc();
      property_copy(property->rhs, property->lhs);
      property_progress_aux(property->lhs, vector);
      Property* tmp1 = property_alloc();
      tmp1->type = ePreferencePropertyTypeEventually;
      tmp1->rhs = property->rhs;
      property->rhs = tmp1;
      ////// IN
      
      ////// OUT
      //property->type = ePreferencePropertyTypeOr;
      //property->lhs = property_alloc();
      //property_copy(property->rhs, property->lhs);
      //property_progress_aux(property->lhs, vector);
      ////// OUT
      return true;
    }
    break;

  case ePreferencePropertyTypeNext:
    tmp.rhs = property->rhs;
    property_shallowCopy(property->rhs, property);
    myfree(tmp.rhs);
    return true;
    break;

  case ePreferencePropertyTypeAfter:
    l = property_eval(property->lhs, vector);
    if (l == 1) {
      tmp = *property;
      property_shallowCopy(property->rhs, property);
      myfree(tmp.rhs);
      property_free(tmp.lhs);
      myfree(tmp.lhs);
      return true;
    } else {
      return false;
    }
    break;

  case ePreferencePropertyTypeAnd:
    l = property_progress_aux(property->lhs, vector);
    r = property_progress_aux(property->rhs, vector);

    // simplify if possible
    if (l || r) {
      if (property->lhs->type == ePreferencePropertyTypeFalse
          || property->rhs->type == ePreferencePropertyTypeFalse) {
        property->type = ePreferencePropertyTypeFalse;
        property_free(property->lhs);
        myfree(property->lhs);
        property_free(property->rhs);
        myfree(property->rhs);
      } else if (property->lhs->type == ePreferencePropertyTypeTrue) {
        tmp = *property;
        property_shallowCopy(property->rhs, property);
        property_free(tmp.lhs);
        myfree(tmp.lhs);
        myfree(tmp.rhs);
      } else if (property->rhs->type == ePreferencePropertyTypeTrue) {
        tmp = *property;
        property_shallowCopy(property->lhs, property);
        property_free(tmp.rhs);
        myfree(tmp.rhs);
        myfree(tmp.lhs);
      }
      return true;
    }
    return false;
    break;

  case ePreferencePropertyTypeOr:
    l = property_progress_aux(property->lhs, vector);
    r = property_progress_aux(property->rhs, vector);

    // simplify if possible
    if (l || r) {
      if (property->lhs->type == ePreferencePropertyTypeTrue
          || property->rhs->type == ePreferencePropertyTypeTrue) {
        property->type = ePreferencePropertyTypeTrue;
        property_free(property->lhs);
        myfree(property->lhs);
        property_free(property->rhs);
        myfree(property->rhs);
      } else if (property->lhs->type == ePreferencePropertyTypeFalse) {
        tmp = *property;
        property_shallowCopy(property->rhs, property);
        property_free(tmp.lhs);
        myfree(tmp.lhs);
        myfree(tmp.rhs);
      } else if (property->rhs->type == ePreferencePropertyTypeFalse) {
        tmp = *property;
        property_shallowCopy(property->lhs, property);
        property_free(tmp.rhs);
        myfree(tmp.rhs);
        myfree(tmp.lhs);
      }
      return true;
    }
    return false;
    break;

  case ePreferencePropertyTypeNot:
    // since property is normalized, this can only be a negated atom
    r = property_progress_aux(property->rhs, vector);
    if (property->rhs->type == ePreferencePropertyTypeFalse) {
      property->type = ePreferencePropertyTypeTrue;
      property_free(property->rhs);
      myfree(property->rhs);
    } else if (property->rhs->type == ePreferencePropertyTypeTrue) {
      property->type = ePreferencePropertyTypeFalse;
      property_free(property->rhs);
      myfree(property->rhs);
    }
    return r;
    break;

  case ePreferencePropertyTypeAtom:
    if (property_eval(property, vector) == 1) {
      property->type = ePreferencePropertyTypeTrue;
    } else {
      property->type = ePreferencePropertyTypeFalse;
    }
    myfree(property->atom);
    return true;
    break;

  default:
    /* true or false, just copy */
    return false;
    break;
  }
}

void property_negate(Property *property) {
  Property tmp;
  Property *tmp2;
  switch (property->type) {
  case ePreferencePropertyTypeFalse:
    property->type = ePreferencePropertyTypeTrue;
    break;
    
  case ePreferencePropertyTypeTrue:
    property->type = ePreferencePropertyTypeFalse;
    break;

  case ePreferencePropertyTypeAtom:
    property->rhs = property_alloc();
    *property->rhs = *property;
    property->type = ePreferencePropertyTypeNot;
    break;

  case ePreferencePropertyTypeAnd:
    property->type = ePreferencePropertyTypeOr;
    property_negate(property->lhs);
    property_negate(property->rhs);
    break;

  case ePreferencePropertyTypeOr:
    property->type = ePreferencePropertyTypeAnd;
    property_negate(property->lhs);
    property_negate(property->rhs);
    break;

  case ePreferencePropertyTypeNot:
    /* double negation */
    tmp2 = property->rhs;
    tmp = *property->rhs;
    *property = tmp;
    myfree(tmp2);
    break;

  case ePreferencePropertyTypeAlways:
    property->type = ePreferencePropertyTypeEventually;
    property_negate(property->rhs);
    break;

  case ePreferencePropertyTypeEventually:
    property->type = ePreferencePropertyTypeAlways;
    property_negate(property->rhs);
    break;

  case ePreferencePropertyTypeNext:
  case ePreferencePropertyTypeAfter:
    property_negate(property->rhs);
    break;

  default:
    // shouldn't happen
    printf("(property_negate) THIS SHOULDN'T HAPPEN\n");
    break;
  }
}

void property_normalize(Property *property) {
  /* this could be made more efficient by caching whether or not the
     property is normalized already. would then need to update that
     flag upon any changes or make sure that all changes leave it in
     normal form again afterwards, but it would us from rechecking
     that each time around */
  switch (property->type) {
  case ePreferencePropertyTypeNot:
    if (property->rhs->type != ePreferencePropertyTypeAtom) {
      property_negate(property->rhs);
      /* property_copy(property->rhs, property); */
      Property tmp = *property->rhs;
      Property *tmp2 = property->rhs;
      *property = tmp;
      myfree(tmp2);
    }
    break;

  case ePreferencePropertyTypeAnd:
  case ePreferencePropertyTypeOr:
  case ePreferencePropertyTypeAfter:
    property_normalize(property->lhs);
    property_normalize(property->rhs);
    break;

  case ePreferencePropertyTypeAlways:
  case ePreferencePropertyTypeEventually:
  case ePreferencePropertyTypeNext:
    property_normalize(property->rhs);
    break;

  default:
    // nothing to do
    break;
  }
}

bool isGoodGoal(const Property* property, const stateVector goalVector) {
  if(property == NULL) {
    return true;
  }

  bool r = isGoodGoal(property->rhs, goalVector);
  bool l = isGoodGoal(property->lhs, goalVector);

  switch (property->type) {
  case ePreferencePropertyTypeFalse:
    return false;
    break;
    
  case ePreferencePropertyTypeTrue:
    return true;
    break;

  case ePreferencePropertyTypeAtom:
    return property_eval(property, goalVector);
    break;

  case ePreferencePropertyTypeAnd:
    return r && l;
    break;

  case ePreferencePropertyTypeOr:
    return r || l;
    break;

  case ePreferencePropertyTypeNot:
    return !r;
    break;

  case ePreferencePropertyTypeAlways:
    return true;
    break;

  case ePreferencePropertyTypeEventually:
    return (property_eval(property->rhs, goalVector) == 1);
    break;

  case ePreferencePropertyTypeNext:
    return false; 
  case ePreferencePropertyTypeAfter:
    return true; 
    break;

  default:
    // shouldn't happen
    printf("THIS SHOULDN'T HAPPEN\n");
    return false;
    break;
  }
}

/* --------------------------------------------------------- 
   Deserialize from JSON
*/

void property_atom_read(Atom *atom, JSON_Object *json) {
  const char* type_string = parson_json_object_get_string(json, "type");
  if (strcmp(type_string, "=") == 0) {
    atom->type = ePreferenceAtomTypeEquals;
  } else if (strcmp(type_string, "<") == 0) {
    atom->type = ePreferenceAtomTypeLT;
  } else if (strcmp(type_string, ">") == 0) {
    atom->type = ePreferenceAtomTypeGT;
  } else {
    printf("(property_atom_read) unknown type: %s\n", type_string);
  }
  atom->variable = (int)parson_json_object_get_number(json, "variable"); // TODO
  atom->value = (int)parson_json_object_get_number(json, "value");
//printf("atom's type:%u\n",atom->type);
//printf("atom's variable:%u\n",atom->variable);
//printf("atom's value:%u\n",atom->value);
}

void property_read(Property *property, JSON_Object *json) {
  const char* type_string = parson_json_object_get_string(json, "type");
  if (strcmp(type_string, "not") == 0) {
    property->type = ePreferencePropertyTypeNot;
    JSON_Object *rhs = parson_json_object_get_object(json, "value");
    property->rhs = property_alloc();
    property_read(property->rhs, rhs);

  } else if (strcmp(type_string, "and") == 0) {
    property->type = ePreferencePropertyTypeAnd;
    JSON_Array *array = parson_json_object_get_array(json, "value");
    JSON_Object *lhs = parson_json_array_get_object(array, 0);
    JSON_Object *rhs = parson_json_array_get_object(array, 1);
    property->lhs = property_alloc();
    property->rhs = property_alloc();
    property_read(property->lhs, lhs);
    property_read(property->rhs, rhs);   

  } else if (strcmp(type_string, "or") == 0) {
    property->type = ePreferencePropertyTypeOr;
    JSON_Array *array = parson_json_object_get_array(json, "value");
    JSON_Object *lhs = parson_json_array_get_object(array, 0);
    JSON_Object *rhs = parson_json_array_get_object(array, 1);
    property->lhs = property_alloc();
    property->rhs = property_alloc();
    property_read(property->lhs, lhs);
    property_read(property->rhs, rhs);   

  } else if (strcmp(type_string, "after") == 0) {
    property->type = ePreferencePropertyTypeAfter;
    JSON_Array *array = parson_json_object_get_array(json, "value");
    JSON_Object *lhs = parson_json_array_get_object(array, 0);
    JSON_Object *rhs = parson_json_array_get_object(array, 1);
    property->lhs = property_alloc();
    property->rhs = property_alloc();
    property_read(property->lhs, lhs);
    property_read(property->rhs, rhs);   

  } else if (strcmp(type_string, "always") == 0) {
    property->type = ePreferencePropertyTypeAlways;
    JSON_Object *rhs = parson_json_object_get_object(json, "value");
    property->rhs = property_alloc();
    property_read(property->rhs, rhs);

  } else if (strcmp(type_string, "eventually") == 0) {
    property->type = ePreferencePropertyTypeEventually;
    JSON_Object *rhs = parson_json_object_get_object(json, "value");
    property->rhs = property_alloc();
    property_read(property->rhs, rhs);

  } else if (strcmp(type_string, "next") == 0) {
    property->type = ePreferencePropertyTypeNext;
    JSON_Object *rhs = parson_json_object_get_object(json, "value");
    property->rhs = property_alloc();
    property_read(property->rhs, rhs);

  } else if (strcmp(type_string, "false") == 0) {
    property->type = ePreferencePropertyTypeFalse;

  } else if (strcmp(type_string, "true") == 0) {
    property->type = ePreferencePropertyTypeTrue;

  } else {
    /* default */
    property->type = ePreferencePropertyTypeAtom;
    /* JSON_Object *atom_json = parson_json_object_get_object(json, "value"); */
    property->atom = ALLOC(Atom);
    property_atom_read(property->atom, json);
  }
}

/* --------------------------------------------------------- 
   Serialize
 */

JSON_Value* property_atom_toJSON(Atom *atom) {
  JSON_Value* json = parson_json_value_init_object();
  JSON_Object* obj = parson_json_object(json);
  switch (atom->type) {
  case ePreferenceAtomTypeEquals:
    parson_json_object_set_string(obj, "type", "=");
    break;
  case ePreferenceAtomTypeLT:
    parson_json_object_set_string(obj, "type", "<");
    break;
  case ePreferenceAtomTypeGT:
    parson_json_object_set_string(obj, "type", ">");
    break;
  default:
    break;
  }
  parson_json_object_set_number(obj, "variable", atom->variable);
  parson_json_object_set_number(obj, "value", atom->value);
  return json;
}

JSON_Value* property_toJSON(Property *property) {
  JSON_Value* json;
  JSON_Object* obj;
  JSON_Value* array_json;
  JSON_Array* array;
  switch (property->type) {
  case ePreferencePropertyTypeFalse:
    json = parson_json_value_init_boolean(false);
    break;

  case ePreferencePropertyTypeTrue:
    json = parson_json_value_init_boolean(true);
    break;

  case ePreferencePropertyTypeAtom:
    json = property_atom_toJSON(property->atom);
    break;

  case ePreferencePropertyTypeAnd:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
    parson_json_object_set_string(obj, "type", "and");
    array_json = parson_json_value_init_array();
    array = parson_json_array(array_json);
    parson_json_object_set_value(obj, "value", array_json);
    parson_json_array_append_value(array, property_toJSON(property->lhs));
    parson_json_array_append_value(array, property_toJSON(property->rhs));
    break;

  case ePreferencePropertyTypeOr:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
    parson_json_object_set_string(obj, "type", "or");
    array_json = parson_json_value_init_array();
    array = parson_json_array(array_json);
    parson_json_object_set_value(obj, "value", array_json);
    parson_json_array_append_value(array, property_toJSON(property->lhs));
    parson_json_array_append_value(array, property_toJSON(property->rhs));
    break;

  case ePreferencePropertyTypeAfter:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
    parson_json_object_set_string(obj, "type", "after");
    array_json = parson_json_value_init_array();
    array = parson_json_array(array_json);
    parson_json_object_set_value(obj, "value", array_json);
    parson_json_array_append_value(array, property_toJSON(property->lhs));
    parson_json_array_append_value(array, property_toJSON(property->rhs));
    break;

  case ePreferencePropertyTypeNot:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
    parson_json_object_set_string(obj, "type", "not");
    parson_json_object_set_value(obj, "value", property_toJSON(property->rhs));
    break;

  case ePreferencePropertyTypeAlways:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
    parson_json_object_set_string(obj, "type", "always");
    parson_json_object_set_value(obj, "value", property_toJSON(property->rhs));
    break;

  case ePreferencePropertyTypeEventually:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
    parson_json_object_set_string(obj, "type", "eventually");
    parson_json_object_set_value(obj, "value", property_toJSON(property->rhs));
    break;

  case ePreferencePropertyTypeNext:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
    parson_json_object_set_string(obj, "type", "next");
    parson_json_object_set_value(obj, "value", property_toJSON(property->rhs));
    break;

  default:
    break;
  }
  return json;
}
