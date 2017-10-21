#include <gmock/gmock.h>
#include <glog/logging.h>

int main(int argc, char** argv) {
    // The following line must be executed to initialize Google Mock
    // (and Google Test) before running the tests.
    // Initialize Google's logging library.
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
