#ifndef cabin_request_TEST
#define cabin_request_TEST

// the following is to include only the main from the first c file
#ifndef TEST_MAIN
#define TEST_MAIN
#define cabin_request_MAIN
#endif // TEST_MAIN

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "../external/cJSON.h"

#include "../model/cabin_request.h"
cabin_request_t* instantiate_cabin_request(int include_optional);



cabin_request_t* instantiate_cabin_request(int include_optional) {
  cabin_request_t* cabin_request = NULL;
  if (include_optional) {
    cabin_request = cabin_request_create(
      "E1",
      "E1A1",
      7,
      list_createList()
    );
  } else {
    cabin_request = cabin_request_create(
      "E1",
      "E1A1",
      7,
      list_createList()
    );
  }

  return cabin_request;
}


#ifdef cabin_request_MAIN

void test_cabin_request(int include_optional) {
    cabin_request_t* cabin_request_1 = instantiate_cabin_request(include_optional);

	cJSON* jsoncabin_request_1 = cabin_request_convertToJSON(cabin_request_1);
	printf("cabin_request :\n%s\n", cJSON_Print(jsoncabin_request_1));
	cabin_request_t* cabin_request_2 = cabin_request_parseFromJSON(jsoncabin_request_1);
	cJSON* jsoncabin_request_2 = cabin_request_convertToJSON(cabin_request_2);
	printf("repeating cabin_request:\n%s\n", cJSON_Print(jsoncabin_request_2));
}

int main() {
  test_cabin_request(1);
  test_cabin_request(0);

  printf("Hello world \n");
  return 0;
}

#endif // cabin_request_MAIN
#endif // cabin_request_TEST
