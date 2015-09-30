#include "preferences.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, const char*argv[]) {

  if (argc < 2) {
    exit(1);
  }

  stateVector vector;
  stateVector vector2;
  /* JSON_Value *json = parson_json_parse_file("test_preferences.json"); */
  for (int i = 1; i < argc; i++) {
    printf("\n\n--- %s\n", argv[i]);

    JSON_Value *json = parson_json_parse_file(argv[i]);
    JSON_Object *obj = parson_json_value_get_object(json);

    vector[0] = 2;
    vector[1] = 230;
    /*vector2[0] = 2;*/
    vector2[0] = 1;
    vector2[1] = 380;
    
    Property p = {.type=1, .atom=NULL, .lhs=NULL, .rhs=NULL};
    property_read(&p, obj);
    printf("Original: \n%s\n", parson_json_serialize_to_string(property_toJSON(&p)));
    printf("eval: %d\n", property_eval(&p, vector));
    printf("eval2: %d\n", property_eval(&p, vector2));

    vector2[0] = 1;
    vector2[1] = 3;
    vector2[2] = 30;
    vector2[20] = 1;
    Property p1 = {.type=1, .atom=NULL, .lhs=NULL, .rhs=NULL};
    property_read(&p1, obj);
    printf("Original of p1: \n%s\n", parson_json_serialize_to_string(property_toJSON(&p1)));
    printf("eval2: %d\n", property_eval(&p1, vector2));
    if(isGoodGoal(&p1, vector2)) {
      printf("Good goal.\n");
    } else {
      printf("Bad goal.\n");
    }

    Property copy;
    property_copy(&p, &copy);
    printf("\nCopy: \n%s\n", parson_json_serialize_to_string(property_toJSON(&copy)));

    property_normalize(&p);
    printf("\nNormalized: \n%s\n", parson_json_serialize_to_string(property_toJSON(&p)));

    property_negate(&copy);
    printf("\nNegated: \n%s\n", parson_json_serialize_to_string(property_toJSON(&copy)));
    printf("Freeing copy\n");
    property_free(&copy);

    ShadowedProperty sp;
    sp.real = malloc(sizeof *sp.real);
    property_copy(&p, sp.real);
    sp.shadow = malloc(sizeof *sp.shadow);
    property_copy(sp.real, sp.shadow);
    Property* tmp = sp.shadow;
    bool freeit = property_progress(&sp, vector);
    printf("\nProgressed: \n%s\n", 
           parson_json_serialize_to_string(property_toJSON(sp.real)));
    if (freeit) {
      printf("Freeing old sp\n");
      property_free(&p);
      property_free(tmp);
      free(tmp);
    }
    
    Property *tmp2 = sp.real;
    tmp = sp.shadow;
    freeit = property_progress(&sp, vector2);
    printf("\nProgressed x2: \n%s\n", 
           parson_json_serialize_to_string(property_toJSON(sp.real)));
    if (freeit) {
      printf("Freeing old sp (2)\n");
      property_free(tmp);
      property_free(tmp2);
      free(tmp);
      free(tmp2);
    }

    printf("Freeing sp\n");
    property_free(sp.real);
    property_free(sp.shadow);
    free(sp.real);
    free(sp.shadow);

  }

  //Property* ptrr = property_alloc();
  //printf("\n%p\n", ptrr);
  //printf("%d\n", ptrr->type);
  //printf("%p\n", ptrr->atom);
  //printf("%p\n", ptrr->lhs);
  //printf("%p\n", ptrr->rhs);
  //property_free(ptrr);
  //free(ptrr);
  //printf("%p\n", ptrr);
  

  return 0;
}
