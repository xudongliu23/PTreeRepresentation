/*****************************************************************
 *  Copyright (c) 2015, Xudong Liu.                              *
 *  All rights reserved.                                         *
 *****************************************************************/

/*
 * reasoning.c
 */

#include "reasoning.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void
ptreeRepresentationComputeLeafIndex(
    char         			 *instancename,
    Ptree              *ptree,
    OutcomeVector       outcome,
		UInt32						 *ind) {
	Formula f;
	char form_filename[cMaxLengthFileName];
	sprintf(form_filename, "instances/label%d_%s.json", ptree->id, instancename);
	JSON_Value *json = parson_json_parse_file(form_filename);
  JSON_Object *obj = parson_json_value_get_object(json);
	formula_read(&f, obj);

	if (ptree == NULL) {
		return;
	}

	switch (ptree->type) {
		case ePreferencePtreeTypeRoot0:
			if(formula_eval(&f, outcome)) {
				return;
			} else {
				*ind += 1;
				return;
			}
			break;
    case ePreferencePtreeTypeRoot2:                               
			if(formula_eval(&f, outcome)) {
				ptreeRepresentationComputeLeafIndex(instancename, ptree->lst, outcome, ind);
			} else {
				*ind += number_of_leaves(ptree->lst);
				ptreeRepresentationComputeLeafIndex(instancename, ptree->rst, outcome, ind);
			}
      break;                                                      
    case ePreferencePtreeTypeRoot1l:
			if(formula_eval(&f, outcome)) {
				ptreeRepresentationComputeLeafIndex(instancename, ptree->lst, outcome, ind);
			} else {
				*ind += number_of_leaves(ptree->lst);
				return;
			}
      break;
    case ePreferencePtreeTypeRoot1m:                              
			if(formula_eval(&f, outcome)) {
				ptreeRepresentationComputeLeafIndex(instancename, ptree->mst, outcome, ind);
			} else {
				*ind += number_of_leaves(ptree->mst);
				ptreeRepresentationComputeLeafIndex(instancename, ptree->mst, outcome, ind);
			}
      break;
    case ePreferencePtreeTypeRoot1r:                              
			if(formula_eval(&f, outcome)) {
				return;
			} else {
				*ind += 1;
				ptreeRepresentationComputeLeafIndex(instancename, ptree->rst, outcome, ind);
			}
      break;
    default:                                                      
      /* true, false: nothing to do */
      break;                                                      
	}
}

