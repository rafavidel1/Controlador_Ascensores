#ifndef floor_call_request_TEST
#define floor_call_request_TEST

// the following is to include only the main from the first c file
#ifndef TEST_MAIN
#define TEST_MAIN
#define floor_call_request_MAIN
#endif // TEST_MAIN

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "../external/cJSON.h"

#include "../model/floor_call_request.h"
floor_call_request_t* instantiate_floor_call_request(int include_optional);



floor_call_request_t* instantiate_floor_call_request(int include_optional) {
  floor_call_request_t* floor_call_request = NULL;
  if (include_optional) {
    floor_call_request = floor_call_request_create(
      "E1",
      2,
      api_gateway_coap_para_sistema_de_ascensores_floor_call_request__"SUBIENDO",
      list_createList()
    );
  } else {
    floor_call_request = floor_call_request_create(
      "E1",
      2,
      api_gateway_coap_para_sistema_de_ascensores_floor_call_request__"SUBIENDO",
      list_createList()
    );
  }

  return floor_call_request;
}


#ifdef floor_call_request_MAIN

void test_floor_call_request(int include_optional) {
    floor_call_request_t* floor_call_request_1 = instantiate_floor_call_request(include_optional);

	cJSON* jsonfloor_call_request_1 = floor_call_request_convertToJSON(floor_call_request_1);
	printf("floor_call_request :\n%s\n", cJSON_Print(jsonfloor_call_request_1));
	floor_call_request_t* floor_call_request_2 = floor_call_request_parseFromJSON(jsonfloor_call_request_1);
	cJSON* jsonfloor_call_request_2 = floor_call_request_convertToJSON(floor_call_request_2);
	printf("repeating floor_call_request:\n%s\n", cJSON_Print(jsonfloor_call_request_2));
}

int main() {
  test_floor_call_request(1);
  test_floor_call_request(0);

  printf("Hello world \n");
  return 0;
}

#endif // floor_call_request_MAIN
#endif // floor_call_request_TEST
