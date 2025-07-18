#ifndef emergency_type_TEST
#define emergency_type_TEST

// the following is to include only the main from the first c file
#ifndef TEST_MAIN
#define TEST_MAIN
#define emergency_type_MAIN
#endif // TEST_MAIN

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "../external/cJSON.h"

#include "../model/emergency_type.h"
emergency_type_t* instantiate_emergency_type(int include_optional);



emergency_type_t* instantiate_emergency_type(int include_optional) {
  emergency_type_t* emergency_type = NULL;
  if (include_optional) {
    emergency_type = emergency_type_create(
    );
  } else {
    emergency_type = emergency_type_create(
    );
  }

  return emergency_type;
}


#ifdef emergency_type_MAIN

void test_emergency_type(int include_optional) {
    emergency_type_t* emergency_type_1 = instantiate_emergency_type(include_optional);

	cJSON* jsonemergency_type_1 = emergency_type_convertToJSON(emergency_type_1);
	printf("emergency_type :\n%s\n", cJSON_Print(jsonemergency_type_1));
	emergency_type_t* emergency_type_2 = emergency_type_parseFromJSON(jsonemergency_type_1);
	cJSON* jsonemergency_type_2 = emergency_type_convertToJSON(emergency_type_2);
	printf("repeating emergency_type:\n%s\n", cJSON_Print(jsonemergency_type_2));
}

int main() {
  test_emergency_type(1);
  test_emergency_type(0);

  printf("Hello world \n");
  return 0;
}

#endif // emergency_type_MAIN
#endif // emergency_type_TEST
