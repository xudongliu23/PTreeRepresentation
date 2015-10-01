/*
 * test.c
 */

#include "ptree.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, const char*argv[]) {

  if (argc < 2) {
    exit(1);
  }

  for (int i = 1; i < argc; i++) {
    printf("\n\n--- %s\n", argv[i]);

    JSON_Value *json = parson_json_parse_file(argv[i]);
    JSON_Object *obj = parson_json_value_get_object(json);

    Ptree p = {.type=ePreferencePtreeTypeRoot0, .id=0, .lst=NULL, .mst=NULL, .rst=NULL};
    ptree_read(&p, obj);
    printf("Original: \n%s\n", parson_json_serialize_to_string(ptree_toJSON(&p)));

    Ptree copy;
    ptree_copy(&p, &copy);
    printf("\nCopy: \n%s\n", parson_json_serialize_to_string(ptree_toJSON(&copy)));

		printf("\nNumber of leaves: %u\n", number_of_leaves(&p));

		ptree_free(&p);
		ptree_free(&copy);
  }

  return 0;
}
