/*****************************************************************
 *  Copyright (c) 2015, Xudong Liu.                              *
 *  All rights reserved.                                         *
 *****************************************************************/

/*
 * ptree.c
 */

#include "ptree.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ALLOC(T) malloc(sizeof(T))

#define myfree(X) if (X != NULL) { free(X); X = NULL; }
// Use for debugging:
/* #define myfree(X) if (X != NULL) { printf("free %p\n", X); free(X); X = NULL; } */

/* #define myfree(X) // #DEBUG */



/* --------------------------------------------------------- */

void ptree_shallowCopy(const Ptree* in, Ptree* out);

/* --------------------------------------------------------- 
   basic operations
*/
Ptree* ptree_alloc() {
  Ptree* rtv = malloc(sizeof *rtv);
  /* printf("alloc %p\n", rtv);  */
  rtv->lst = NULL;
  rtv->mst = NULL;
  rtv->rst = NULL;
  /* ptree_pointerArray_add(rtv); */
  return rtv;
}

void ptree_free(Ptree* p) {
  /* printf(" ptree_free %p -- type: %d\n", p, p->type); */
  if (p != NULL) {
    switch (p->type) {
    case ePreferencePtreeTypeRoot0:
      break;
    case ePreferencePtreeTypeRoot2:
      ptree_free(p->lst);
      ptree_free(p->rst);
      myfree(p->lst);
      myfree(p->rst);
      break;
    case ePreferencePtreeTypeRoot1l:
      ptree_free(p->lst);
      myfree(p->lst);
      break;
    case ePreferencePtreeTypeRoot1m:
      ptree_free(p->mst);
      myfree(p->mst);
      break;
    case ePreferencePtreeTypeRoot1r:
      ptree_free(p->rst);
      myfree(p->rst);
      break;
    default:
      /* true, false: nothing to do */
      break;
    }
  }
}

void ptree_copy(const Ptree* in, Ptree* out) {
  /* printf("ptree_copy %p -> %p\n", in, out); */
  out->type = in->type;
  switch (in->type) {
  case ePreferencePtreeTypeRoot0:
		out->id = in->id;
    break;
  case ePreferencePtreeTypeRoot2:
		out->id = in->id;
    out->lst = ptree_alloc();
    ptree_copy(in->lst, out->lst);
    out->rst = ptree_alloc();
    ptree_copy(in->rst, out->rst);
    break;
  case ePreferencePtreeTypeRoot1l:
		out->id = in->id;
    out->lst = ptree_alloc();
    ptree_copy(in->lst, out->lst);
    break;
  case ePreferencePtreeTypeRoot1m:
		out->id = in->id;
    out->mst = ptree_alloc();
    ptree_copy(in->mst, out->mst);
    break;
  case ePreferencePtreeTypeRoot1r:
		out->id = in->id;
    out->rst = ptree_alloc();
    ptree_copy(in->rst, out->rst);
    break;
  default:
    /* true, false: nothing to do */
    break;
  }
}

void ptree_shallowCopy(const Ptree* in, Ptree* out) {
  out->type = in->type;
  out->id = in->id;
  out->lst = in->lst;
  out->mst = in->mst;
  out->rst = in->rst;
}

/* --------------------------------------------------------- 
   Deserialize from JSON
*/

void ptree_read(Ptree *ptree, JSON_Object *json) {
  const char* type_string = parson_json_object_get_string(json, "type");
	if (strcmp(type_string, "root2") == 0) {
    ptree->type = ePreferencePtreeTypeRoot2;
    ptree->id = (UInt32)parson_json_object_get_number(json, "id");
    JSON_Array *array = parson_json_object_get_array(json, "value");
    JSON_Object *lst = parson_json_array_get_object(array, 0);
    JSON_Object *rst = parson_json_array_get_object(array, 1);
    ptree->lst = ptree_alloc();
    ptree->rst = ptree_alloc();
    ptree_read(ptree->lst, lst);
    ptree_read(ptree->rst, rst);   
	} else if (strcmp(type_string, "root1l") == 0) {
    ptree->type = ePreferencePtreeTypeRoot1l;
    ptree->id = (UInt32)parson_json_object_get_number(json, "id");
		JSON_Object *lst = parson_json_object_get_object(json, "value");
    ptree->lst = ptree_alloc();
    ptree_read(ptree->lst, lst);
	}	else if (strcmp(type_string, "root1m") == 0) {
    ptree->type = ePreferencePtreeTypeRoot1m;
    ptree->id = (UInt32)parson_json_object_get_number(json, "id");
		JSON_Object *mst = parson_json_object_get_object(json, "value");
    ptree->mst = ptree_alloc();
    ptree_read(ptree->mst, mst);
	}	else if (strcmp(type_string, "root1r") == 0) {
    ptree->type = ePreferencePtreeTypeRoot1r;
    ptree->id = (UInt32)parson_json_object_get_number(json, "id");
		JSON_Object *rst = parson_json_object_get_object(json, "value");
    ptree->rst = ptree_alloc();
    ptree_read(ptree->rst, rst);
	} else {
    /* default */
    ptree->type = ePreferencePtreeTypeRoot0;
    ptree->id = (UInt32)parson_json_object_get_number(json, "id");
  }
}

/* --------------------------------------------------------- 
   Serialize
 */

JSON_Value* ptree_toJSON(Ptree *ptree) {
  JSON_Value* json;
  JSON_Object* obj;
  JSON_Value* array_json;
  JSON_Array* array;
  switch (ptree->type) {
  case ePreferencePtreeTypeRoot0:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
  	parson_json_object_set_number(obj, "id", ptree->id);
    break;

  case ePreferencePtreeTypeRoot2:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
  	parson_json_object_set_number(obj, "id", ptree->id);
    parson_json_object_set_string(obj, "type", "root2");
    array_json = parson_json_value_init_array();
    array = parson_json_array(array_json);
    parson_json_object_set_value(obj, "value", array_json);
    parson_json_array_append_value(array, ptree_toJSON(ptree->lst));
    parson_json_array_append_value(array, ptree_toJSON(ptree->rst));
    break;

  case ePreferencePtreeTypeRoot1l:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
  	parson_json_object_set_number(obj, "id", ptree->id);
    parson_json_object_set_string(obj, "type", "root1l");
    parson_json_object_set_value(obj, "value", ptree_toJSON(ptree->lst));
    break;

  case ePreferencePtreeTypeRoot1m:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
  	parson_json_object_set_number(obj, "id", ptree->id);
    parson_json_object_set_string(obj, "type", "root1m");
    parson_json_object_set_value(obj, "value", ptree_toJSON(ptree->mst));
    break;

  case ePreferencePtreeTypeRoot1r:
    json = parson_json_value_init_object();
    obj = parson_json_object(json);
  	parson_json_object_set_number(obj, "id", ptree->id);
    parson_json_object_set_string(obj, "type", "root1r");
    parson_json_object_set_value(obj, "value", ptree_toJSON(ptree->rst));
    break;

  default:
    break;
  }
  return json;
}

UInt32 number_of_leaves(Ptree *ptree) {
  switch (ptree->type) {
  case ePreferencePtreeTypeRoot0:
		return 2;
    break;

  case ePreferencePtreeTypeRoot2:
		return number_of_leaves(ptree->lst) + number_of_leaves(ptree->rst);
    break;

  case ePreferencePtreeTypeRoot1l:
		return number_of_leaves(ptree->lst) + 1;
    break;

  case ePreferencePtreeTypeRoot1m:
		return number_of_leaves(ptree->mst) * 2;
    break;

  case ePreferencePtreeTypeRoot1r:
		return number_of_leaves(ptree->rst) + 1;
    break;

  default:
		return 0;
    break;
  }
}




