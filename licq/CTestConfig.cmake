## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.

set(CTEST_PROJECT_NAME "Licq")
set(CTEST_NIGHTLY_START_TIME "03:00:00 EST")

set(CTEST_UPDATE_TYPE "svn")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "my.cdash.org")
set(CTEST_DROP_LOCATION "/submit.php?project=Licq")
set(CTEST_DROP_SITE_CDASH TRUE)