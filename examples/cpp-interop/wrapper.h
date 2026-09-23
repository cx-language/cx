// C ABI wrappers around mathlib.h so cx can call C++ through imported
// C declarations. C++ classes cross the boundary as opaque handles;
// construction/destruction is explicit (create/destroy) since cx has
// no C++ constructor/destructor integration.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CalculatorHandle CalculatorHandle;

CalculatorHandle* calculator_create(double initial);
void calculator_add(CalculatorHandle* calc, double value);
void calculator_multiply(CalculatorHandle* calc, double value);
double calculator_result(const CalculatorHandle* calc);
void calculator_destroy(CalculatorHandle* calc);

double math_factorial(int n);

#ifdef __cplusplus
}
#endif
