/*
 * test.c
 */

#include "ptree.h"
#include "formula_label.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, const char*argv[]) {

  if (argc < 3) {
		printf("Not enough arguments!\n\n");
    exit(1);
  }

	/* Test ptree module */
  printf("\n\n--- %s\n", argv[1]);

  JSON_Value *json = parson_json_parse_file(argv[1]);
  JSON_Object *obj = parson_json_value_get_object(json);

  Ptree p = {.type=ePreferencePtreeTypeRoot0, .id=0, .lst=NULL, .mst=NULL, .rst=NULL};
  ptree_read(&p, obj);
  printf("Original ptree: \n%s\n", parson_json_serialize_to_string(ptree_toJSON(&p)));

  Ptree copy;
  ptree_copy(&p, &copy);
  printf("\nCopy ptree: \n%s\n", parson_json_serialize_to_string(ptree_toJSON(&copy)));

	printf("\nNumber of leaves in ptree: %u\n", number_of_leaves(&p));

	ptree_free(&p);
	ptree_free(&copy);


	/* Test formula module */
  printf("\n\n--- %s\n", argv[2]);

  json = parson_json_parse_file(argv[2]);
  obj = parson_json_value_get_object(json);

	Formula f = {.type=eFormulaTypeTrue, .atom=NULL, .lhs=NULL, .rhs=NULL};
	formula_read(&f, obj);
  printf("Original formula: \n%s\n", parson_json_serialize_to_string(formula_toJSON(&f)));

	Formula cp;
	formula_copy(&f, &cp);
  printf("\nCopy formula: \n%s\n", parson_json_serialize_to_string(formula_toJSON(&cp)));

	OutcomeVector o = {0,0,0,1,0};
	if(formula_eval(&f, o)) {
		printf("\nFormula satisfied.\n");
	} else {
		printf("\nFormula falsified.\n");
	}

	formula_free(&f);
	formula_free(&cp);

  return 0;
}





