#ifndef emergency_response_TEST
#define emergency_response_TEST

// the following is to include only the main from the first c file
#ifndef TEST_MAIN
#define TEST_MAIN
#define emergency_response_MAIN
#endif // TEST_MAIN

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "../external/cJSON.h"

#include "../model/emergency_response.h"
emergency_response_t* instantiate_emergency_response(int include_optional);



emergency_response_t* instantiate_emergency_response(int include_optional) {
  emergency_response_t* emergency_response = NULL;
  if (include_optional) {
    emergency_response = emergency_response_create(
      "EMG_1640995200123",
      api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_"RESCUE_PROTOCOL",
      15,
      ["BOMBEROS","MANTENIMIENTO","SEGURIDAD"],
      ["E1A2","E1A3"]
    );
  } else {
    emergency_response = emergency_response_create(
      "EMG_1640995200123",
      api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_"RESCUE_PROTOCOL",
      15,
      ["BOMBEROS","MANTENIMIENTO","SEGURIDAD"],
      ["E1A2","E1A3"]
    );
  }

  return emergency_response;
}


#ifdef emergency_response_MAIN

void test_emergency_response(int include_optional) {
    emergency_response_t* emergency_response_1 = instantiate_emergency_response(include_optional);

	cJSON* jsonemergency_response_1 = emergency_response_convertToJSON(emergency_response_1);
	printf("emergency_response :\n%s\n", cJSON_Print(jsonemergency_response_1));
	emergency_response_t* emergency_response_2 = emergency_response_parseFromJSON(jsonemergency_response_1);
	cJSON* jsonemergency_response_2 = emergency_response_convertToJSON(emergency_response_2);
	printf("repeating emergency_response:\n%s\n", cJSON_Print(jsonemergency_response_2));
}

int main() {
  test_emergency_response(1);
  test_emergency_response(0);

  printf("Hello world \n");
  return 0;
}

#endif // emergency_response_MAIN
#endif // emergency_response_TEST
