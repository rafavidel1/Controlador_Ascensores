#ifndef door_state_TEST
#define door_state_TEST

// the following is to include only the main from the first c file
#ifndef TEST_MAIN
#define TEST_MAIN
#define door_state_MAIN
#endif // TEST_MAIN

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "../external/cJSON.h"

#include "../model/door_state.h"
door_state_t* instantiate_door_state(int include_optional);



door_state_t* instantiate_door_state(int include_optional) {
  door_state_t* door_state = NULL;
  if (include_optional) {
    door_state = door_state_create(
    );
  } else {
    door_state = door_state_create(
    );
  }

  return door_state;
}


#ifdef door_state_MAIN

void test_door_state(int include_optional) {
    door_state_t* door_state_1 = instantiate_door_state(include_optional);

	cJSON* jsondoor_state_1 = door_state_convertToJSON(door_state_1);
	printf("door_state :\n%s\n", cJSON_Print(jsondoor_state_1));
	door_state_t* door_state_2 = door_state_parseFromJSON(jsondoor_state_1);
	cJSON* jsondoor_state_2 = door_state_convertToJSON(door_state_2);
	printf("repeating door_state:\n%s\n", cJSON_Print(jsondoor_state_2));
}

int main() {
  test_door_state(1);
  test_door_state(0);

  printf("Hello world \n");
  return 0;
}

#endif // door_state_MAIN
#endif // door_state_TEST
