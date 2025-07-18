#ifndef server_response_TEST
#define server_response_TEST

// the following is to include only the main from the first c file
#ifndef TEST_MAIN
#define TEST_MAIN
#define server_response_MAIN
#endif // TEST_MAIN

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "../external/cJSON.h"

#include "../model/server_response.h"
server_response_t* instantiate_server_response(int include_optional);



server_response_t* instantiate_server_response(int include_optional) {
  server_response_t* server_response = NULL;
  if (include_optional) {
    server_response = server_response_create(
      "E1A1",
      "T_1640995200123",
      7,
      45
    );
  } else {
    server_response = server_response_create(
      "E1A1",
      "T_1640995200123",
      7,
      45
    );
  }

  return server_response;
}


#ifdef server_response_MAIN

void test_server_response(int include_optional) {
    server_response_t* server_response_1 = instantiate_server_response(include_optional);

	cJSON* jsonserver_response_1 = server_response_convertToJSON(server_response_1);
	printf("server_response :\n%s\n", cJSON_Print(jsonserver_response_1));
	server_response_t* server_response_2 = server_response_parseFromJSON(jsonserver_response_1);
	cJSON* jsonserver_response_2 = server_response_convertToJSON(server_response_2);
	printf("repeating server_response:\n%s\n", cJSON_Print(jsonserver_response_2));
}

int main() {
  test_server_response(1);
  test_server_response(0);

  printf("Hello world \n");
  return 0;
}

#endif // server_response_MAIN
#endif // server_response_TEST
