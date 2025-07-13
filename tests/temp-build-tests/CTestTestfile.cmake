# CMake generated Testfile for 
# Source directory: /mnt/d/AATFG/API/Codigo/git/tests
# Build directory: /mnt/d/AATFG/API/Codigo/git/tests/temp-build-tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_elevator_state_manager "/mnt/d/AATFG/API/Codigo/git/tests/temp-build-tests/test_elevator_state_manager" "--automated")
set_tests_properties(test_elevator_state_manager PROPERTIES  ENVIRONMENT "CUNIT_OUT_NAME=test_elevator_state_manager" TIMEOUT "30" WORKING_DIRECTORY "/mnt/d/AATFG/API/Codigo/git/tests/temp-build-tests/test_reports" _BACKTRACE_TRIPLES "/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;61;add_test;/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;96;add_test_with_report;/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;0;")
add_test(test_can_bridge "/mnt/d/AATFG/API/Codigo/git/tests/temp-build-tests/test_can_bridge" "--automated")
set_tests_properties(test_can_bridge PROPERTIES  ENVIRONMENT "CUNIT_OUT_NAME=test_can_bridge" TIMEOUT "30" WORKING_DIRECTORY "/mnt/d/AATFG/API/Codigo/git/tests/temp-build-tests/test_reports" _BACKTRACE_TRIPLES "/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;61;add_test;/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;97;add_test_with_report;/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;0;")
add_test(test_api_handlers "/mnt/d/AATFG/API/Codigo/git/tests/temp-build-tests/test_api_handlers" "--automated")
set_tests_properties(test_api_handlers PROPERTIES  ENVIRONMENT "CUNIT_OUT_NAME=test_api_handlers" TIMEOUT "30" WORKING_DIRECTORY "/mnt/d/AATFG/API/Codigo/git/tests/temp-build-tests/test_reports" _BACKTRACE_TRIPLES "/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;61;add_test;/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;98;add_test_with_report;/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;0;")
add_test(test_servidor_central "/mnt/d/AATFG/API/Codigo/git/tests/temp-build-tests/test_servidor_central" "--automated")
set_tests_properties(test_servidor_central PROPERTIES  ENVIRONMENT "CUNIT_OUT_NAME=test_servidor_central" TIMEOUT "30" WORKING_DIRECTORY "/mnt/d/AATFG/API/Codigo/git/tests/temp-build-tests/test_reports" _BACKTRACE_TRIPLES "/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;61;add_test;/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;99;add_test_with_report;/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;0;")
add_test(test_psk_security "/mnt/d/AATFG/API/Codigo/git/tests/temp-build-tests/test_psk_security" "--automated")
set_tests_properties(test_psk_security PROPERTIES  ENVIRONMENT "CUNIT_OUT_NAME=test_psk_security" TIMEOUT "30" WORKING_DIRECTORY "/mnt/d/AATFG/API/Codigo/git/tests/temp-build-tests/test_reports" _BACKTRACE_TRIPLES "/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;61;add_test;/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;100;add_test_with_report;/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;0;")
add_test(test_can_to_coap "/mnt/d/AATFG/API/Codigo/git/tests/temp-build-tests/test_can_to_coap" "--automated")
set_tests_properties(test_can_to_coap PROPERTIES  ENVIRONMENT "CUNIT_OUT_NAME=test_can_to_coap" TIMEOUT "30" WORKING_DIRECTORY "/mnt/d/AATFG/API/Codigo/git/tests/temp-build-tests/test_reports" _BACKTRACE_TRIPLES "/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;61;add_test;/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;103;add_test_with_report;/mnt/d/AATFG/API/Codigo/git/tests/CMakeLists.txt;0;")
subdirs("mocks")
