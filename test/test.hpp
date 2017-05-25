#include <xplatform>

void _testResult(const char* file, int line, const char* fn, const char* test, bool value);

#define test(X) _testResult(__FILE__, __LINE__, STX_FUNCTION, #X, X)
