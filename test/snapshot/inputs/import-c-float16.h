_Float16 h;
__bf16 bh;
_Float16 geth(void);
struct WithHalf {
    _Float16 h;
    int x;
};
int takesHalfStruct(struct WithHalf* s);
int plain(int x);
