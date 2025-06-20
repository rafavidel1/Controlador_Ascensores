#ifndef elevator_state_TEST
#define elevator_state_TEST

// the following is to include only the main from the first c file
#ifndef TEST_MAIN
#define TEST_MAIN
#define elevator_state_MAIN
#endif // TEST_MAIN

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "../external/cJSON.h"

#include "../model/elevator_state.h"
elevator_state_t* instantiate_elevator_state(int include_optional);



elevator_state_t* instantiate_elevator_state(int include_optional) {
  elevator_state_t* elevator_state = NULL;
  if (include_optional) {
    elevator_state = elevator_state_create(
      "E1A1",
      0,
      api_gateway_coap_para_sistema_de_ascensores_elevator_state__"CERRADA",
      true,
      "T_123456",
      -1
    );
  } else {
    elevator_state = elevator_state_create(
      "E1A1",
      0,
      api_gateway_coap_para_sistema_de_ascensores_elevator_state__"CERRADA",
      true,
      "T_123456",
      -1
    );
  }

  return elevator_state;
}


#ifdef elevator_state_MAIN

void test_elevator_state(int include_optional) {
    elevator_state_t* elevator_state_1 = instantiate_elevator_state(include_optional);

	cJSON* jsonelevator_state_1 = elevator_state_convertToJSON(elevator_state_1);
	printf("elevator_state :\n%s\n", cJSON_Print(jsonelevator_state_1));
	elevator_state_t* elevator_state_2 = elevator_state_parseFromJSON(jsonelevator_state_1);
	cJSON* jsonelevator_state_2 = elevator_state_convertToJSON(elevator_state_2);
	printf("repeating elevator_state:\n%s\n", cJSON_Print(jsonelevator_state_2));
}

int main() {
  test_elevator_state(1);
  test_elevator_state(0);

  printf("Hello world \n");
  return 0;
}

#endif // elevator_state_MAIN
#endif // elevator_state_TEST
