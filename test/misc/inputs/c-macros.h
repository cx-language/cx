#define INT_ONE 1
#define INT_NEG -1
#define HEX 0x10
#define OCTAL 010
#define UNSIGNED 42U
#define LONG_VAL 100000L
#define LONGLONG_VAL 10000000000LL
#define FLOAT_VAL 1.5
#define FLOAT_F 2.5f
#define ALIAS INT_ONE
#define STR "hello"
#define CH 'A'
#define PAREN (42)
#define PAREN_FLOAT (1.5)
#define ADD (1 + 2)
#define SHIFT (1 << 4)
#define BITOR (1 | 2)
#define BITAND (6 & 3)
#define BITXOR (6 ^ 3)
#define NEG (-INT_ONE)
#define BITNOT (~0)
#define LOGNOT (!0)
#define MUL ((2 + 3) * 4)
#define DIV (7 / 2)
#define MOD (7 % 3)
#define EQ (1 == 1)
#define LT (1 < 2)
#define LAND (1 && 0)
#define LOR (1 || 0)
#define FROM_MACRO (ALIAS + 4)
#define UEXPR (1U << 31)
#define LEXPR (1LL << 40)
#define PRECEDENCE (1 + 2 * 3)
#define SHIFT_ADD (1 << 2 + 1)
#define BITAND_EQ (0 & 1 == 0)
#define OR_AND (1 || 0 && 0)
#define SUB_ASSOC (10 - 4 - 3)
#define DIV_ASSOC (100 / 10 / 2)
#define LLONG_ULONG_MIX (1LL + 1UL)
#define LONG_UINT_MIX (1L + 1U)
#define MIXED_CMP (-1 < 1U)
#define ULL_WRAP (18446744073709551615ULL + 1)
#define FDIV (7.0 / 2)
#define CHAR_EXPR ('A' + 1)
#define MY_INF (__builtin_huge_val())
#define MY_NAN (__builtin_nan(""))
#define STR_CONCAT "a" "b"
#define FUNC_LIKE(a, b) ((a) + (b))
#define EMPTY
#define CAST ((int)3)
#define DIV_ZERO (1 / 0)
#define MOD_ZERO (1 % 0)
#define MIN_DIV_NEG ((0 - 2147483647 - 1) / -1)
#define SHIFT_OVER (1 << 100)
#define SHIFT_NEG (1 << (0 - 1))
#define UNKNOWN_REF (NO_SUCH_MACRO + 1)
#define TERNARY (1 ? 2 : 3)
#define WITH_SIZEOF (sizeof(int))
#define FLOAT_MOD (1.5 % 2)
#define FLOAT_SHIFT (1.5 << 1)
#define FLOAT_BITAND (1.5 & 1)
#define STR_EXPR ("a" + 1)
#define PAREN_STR ("hi")
#define BARE_BUILTIN __builtin_huge_val
typedef struct {
    int r, g, b, a;
} Color;
#define COMPOUND (Color){ 1, 2, 3, 4 }
