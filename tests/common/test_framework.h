#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <vector>
#include <functional>
#include <iostream>

// Type alias for a test function
using TestFunction = std::function<void()>;

// Declare test registry function
std::vector<std::pair<std::string, TestFunction>>& GetTestRegistry();

// Function to run all registered tests
void RunAllTests();

// Macro to register tests
#define REGISTER_TEST(name, func) \
    static void __attribute__((constructor)) register_##func() { \
        GetTestRegistry().push_back({name, func}); \
    }

#endif // TEST_FRAMEWORK_H
