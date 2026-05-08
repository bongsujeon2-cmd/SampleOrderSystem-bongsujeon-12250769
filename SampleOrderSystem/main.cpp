#include <gmock/gmock.h>
#include <string>
#include <windows.h>

static bool hasArg(int argc, char* argv[], const std::string& flag) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == flag) return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    if (hasArg(argc, argv, "--test")) {
        testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }
    // TODO: 이후 단계에서 앱 로직 구현
    return 0;
}
