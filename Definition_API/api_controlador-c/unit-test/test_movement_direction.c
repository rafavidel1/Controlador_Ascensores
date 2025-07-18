#ifndef movement_direction_TEST
#define movement_direction_TEST

// the following is to include only the main from the first c file
#ifndef TEST_MAIN
#define TEST_MAIN
#define movement_direction_MAIN
#endif // TEST_MAIN

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "../external/cJSON.h"

#include "../model/movement_direction.h"
movement_direction_t* instantiate_movement_direction(int include_optional);



movement_direction_t* instantiate_movement_direction(int include_optional) {
  movement_direction_t* movement_direction = NULL;
  if (include_optional) {
    movement_direction = movement_direction_create(
    );
  } else {
    movement_direction = movement_direction_create(
    );
  }

  return movement_direction;
}


#ifdef movement_direction_MAIN

void test_movement_direction(int include_optional) {
    movement_direction_t* movement_direction_1 = instantiate_movement_direction(include_optional);

	cJSON* jsonmovement_direction_1 = movement_direction_convertToJSON(movement_direction_1);
	printf("movement_direction :\n%s\n", cJSON_Print(jsonmovement_direction_1));
	movement_direction_t* movement_direction_2 = movement_direction_parseFromJSON(jsonmovement_direction_1);
	cJSON* jsonmovement_direction_2 = movement_direction_convertToJSON(movement_direction_2);
	printf("repeating movement_direction:\n%s\n", cJSON_Print(jsonmovement_direction_2));
}

int main() {
  test_movement_direction(1);
  test_movement_direction(0);

  printf("Hello world \n");
  return 0;
}

#endif // movement_direction_MAIN
#endif // movement_direction_TEST
