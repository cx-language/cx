__int128 big;
unsigned __int128 ubig;
__int128 getbig(void);
struct WithBig {
    unsigned __int128 v[2];
    int x;
};
int takesBigStruct(struct WithBig* s);
int plain(int x);
