// Tiny test harness: CHECK / CHECK_THROWS record failures and keep going.
// Unlike assert(), these still run when NDEBUG is defined.
#pragma once

#include <exception>
#include <iostream>
#include <sstream>
#include <vector>

namespace check {

inline int failures = 0;
inline std::ostringstream details;  // messages for the test that is running

struct TestCase {
    const char* name;
    void (*fn)();
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct Register {
    Register(const char* name, void (*fn)()) { registry().push_back({name, fn}); }
};

inline void fail(const char* file, int line, const char* message) {
    details << "    " << file << ":" << line << ": " << message << "\n";
    ++failures;
}

inline int run_all() {
    int failed_tests = 0;
    for (const TestCase& test : registry()) {
        const int before = failures;
        details.str("");
        try {
            test.fn();
        } catch (const std::exception& e) {
            details << "    uncaught exception: " << e.what() << "\n";
            ++failures;
        } catch (...) {
            details << "    uncaught non-standard exception\n";
            ++failures;
        }
        const bool passed = failures == before;
        if (!passed) ++failed_tests;
        std::cout << (passed ? "PASS  " : "FAIL  ") << test.name << "\n" << details.str();
    }
    const int total = static_cast<int>(registry().size());
    std::cout << "\n" << (total - failed_tests) << "/" << total << " tests passed\n";
    return failed_tests == 0 ? 0 : 1;
}

}  // namespace check

#define TEST(name)                                        \
    static void name();                                   \
    static check::Register name##_registration(#name, name); \
    static void name()

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) check::fail(__FILE__, __LINE__, "CHECK(" #expr ")"); \
    } while (0)

#define CHECK_THROWS(expr, exception_type)                                       \
    do {                                                                         \
        bool caught_expected = false;                                            \
        try {                                                                    \
            expr;                                                                \
        } catch (const exception_type&) {                                        \
            caught_expected = true;                                              \
        } catch (...) {                                                          \
        }                                                                        \
        if (!caught_expected)                                                    \
            check::fail(__FILE__, __LINE__,                                      \
                        "CHECK_THROWS(" #expr ", " #exception_type ")");         \
    } while (0)
