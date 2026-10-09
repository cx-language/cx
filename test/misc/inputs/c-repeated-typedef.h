#include "c-repeated-typedef-part.h"

// Repeated identical typedefs are legal C: the same header can repeat one
// (like PFORMAT_STRING in rpcndr.h), and two headers in one import can each
// declare one (like INT in winnt.h and minwindef.h).
typedef int WidgetInt;
typedef int WidgetDirect;
typedef int WidgetDirect;

// Every C_ASSERT use redefines __C_ASSERT__ with the same type.
#define REPEATED_ASSERT typedef char RepeatedAssert[1]
REPEATED_ASSERT;
REPEATED_ASSERT;
