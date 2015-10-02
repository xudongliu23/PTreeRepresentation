/*
 * test.c
 */

#include "ptree.h"
#include "formula_label.h"
#include "reasoning.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, const char*argv[]) {

  if (argc < 3) {
		printf("Not enough arguments!\n\n");
    exit(1);
  }

	OutcomeVector o1 = {0,1,1,1,0};
	OutcomeVector o2 = {0,1,0,1,0};

	/* Test ptree module */
  printf("\n\n--- %s:\n", argv[2]);

  JSON_Value *json = parson_json_parse_file(argv[2]);
  JSON_Object *obj = parson_json_value_get_object(json);

  Ptree p = {.type=ePreferencePtreeTypeRoot0, .id=0, .lst=NULL, .mst=NULL, .rst=NULL};
  ptree_read(&p, obj);
  printf("Ptree: \n%s\n", parson_json_serialize_to_string(ptree_toJSON(&p)));
	printf("\nNumber of leaves in ptree: %u\n", number_of_leaves(&p));

#if 0
	/* Test formula module */
  printf("\n\n--- %s\n", argv[3]);

  json = parson_json_parse_file(argv[3]);
  obj = parson_json_value_get_object(json);

	Formula f = {.type=eFormulaTypeTrue, .atom=NULL, .lhs=NULL, .rhs=NULL};
	formula_read(&f, obj);
  printf("Original formula: \n%s\n", parson_json_serialize_to_string(formula_toJSON(&f)));

	Formula cp;
	formula_copy(&f, &cp);
  printf("\nCopy formula: \n%s\n", parson_json_serialize_to_string(formula_toJSON(&cp)));

	if(formula_eval(&f, o1)) {
		printf("\nFormula satisfied.\n\n");
	} else {                          
		printf("\nFormula falsified.\n\n");
	}

	formula_free(&f);
	formula_free(&cp);
#endif

	/* Compute the rank of outcome o */
	UInt32 rank_o1 = 0;
	ptreeRepresentationComputeLeafIndex(argv[1], &p, o1, &rank_o1);
	printf("The rank of o1 is %d.\n", rank_o1);

	/* Compare outcomes */
	UInt32 rank_o2 = 0;
	ptreeRepresentationComputeLeafIndex(argv[1], &p, o2, &rank_o2);
	printf("The rank of o2 is %d.\n", rank_o2);

	if (rank_o1 == rank_o2) {
		printf("o1 is equivalent to o2.\n\n");
	} else if (rank_o1 < rank_o2) {
		printf("o1 is preferred over o2.\n\n");
	} else {
		printf("o2 is preferred over o1.\n\n");
	}

	ptree_free(&p);

  return 0;
}





