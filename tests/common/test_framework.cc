#include "test_framework.h"
#include <iostream>
#include <stdexcept>

std::vector<std::pair<std::string, TestFunction>>& GetTestRegistry() {
    static std::vector<std::pair<std::string, TestFunction>> registry;
    return registry;
}

void RunAllTests() {
    int passed = 0, failed = 0;

    for (const auto& [name, test] : GetTestRegistry()) {
        std::cout << "Running test: " << name << " ... " << std::endl;

        try {
            test();  // Run the test function
            std::cout << "✅ PASSED" << std::endl;
            passed++;
        } catch (const std::exception& e) {
            std::cout << "❌ FAILED (" << e.what() << ")" << std::endl;
            failed++;
        } catch (...) {
            std::cout << "❌ FAILED (Unknown Error)" << std::endl;
            failed++;
        }
    }

    // Print final summary
    std::cout << "\n==============================" << std::endl;
    std::cout << "✅ PASSED: " << passed << " | ❌ FAILED: " << failed << std::endl;

    if (failed == 0) {
        std::cout << "🎉 ALL TESTS PASSED SUCCESSFULLY! 🎉" << std::endl;
    } else {
        std::cout << "⚠️ SOME TESTS FAILED. CHECK ERRORS ABOVE. ⚠️" << std::endl;
    }
}
