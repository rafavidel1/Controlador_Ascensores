#ifndef emergency_call_request_TEST
#define emergency_call_request_TEST

// the following is to include only the main from the first c file
#ifndef TEST_MAIN
#define TEST_MAIN
#define emergency_call_request_MAIN
#endif // TEST_MAIN

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "../external/cJSON.h"

#include "../model/emergency_call_request.h"
emergency_call_request_t* instantiate_emergency_call_request(int include_optional);



emergency_call_request_t* instantiate_emergency_call_request(int include_optional) {
  emergency_call_request_t* emergency_call_request = NULL;
  if (include_optional) {
    emergency_call_request = emergency_call_request_create(
      "E1",
      "E1A1",
      api_gateway_coap_para_sistema_de_ascensores_emergency_call_request__"PEOPLE_TRAPPED",
      5,
      "Personas atrapadas, puerta no abre",
      "2024-01-15T10:30Z",
      list_createList()
    );
  } else {
    emergency_call_request = emergency_call_request_create(
      "E1",
      "E1A1",
      api_gateway_coap_para_sistema_de_ascensores_emergency_call_request__"PEOPLE_TRAPPED",
      5,
      "Personas atrapadas, puerta no abre",
      "2024-01-15T10:30Z",
      list_createList()
    );
  }

  return emergency_call_request;
}


#ifdef emergency_call_request_MAIN

void test_emergency_call_request(int include_optional) {
    emergency_call_request_t* emergency_call_request_1 = instantiate_emergency_call_request(include_optional);

	cJSON* jsonemergency_call_request_1 = emergency_call_request_convertToJSON(emergency_call_request_1);
	printf("emergency_call_request :\n%s\n", cJSON_Print(jsonemergency_call_request_1));
	emergency_call_request_t* emergency_call_request_2 = emergency_call_request_parseFromJSON(jsonemergency_call_request_1);
	cJSON* jsonemergency_call_request_2 = emergency_call_request_convertToJSON(emergency_call_request_2);
	printf("repeating emergency_call_request:\n%s\n", cJSON_Print(jsonemergency_call_request_2));
}

int main() {
  test_emergency_call_request(1);
  test_emergency_call_request(0);

  printf("Hello world \n");
  return 0;
}

#endif // emergency_call_request_MAIN
#endif // emergency_call_request_TEST
