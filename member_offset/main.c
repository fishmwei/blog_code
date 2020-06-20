
#include <stdio.h>
#include <assert.h>

#define memberOffsetof(TYPE, MEMBER) ((size_t)(&((TYPE *)0)->MEMBER))


struct regionA {
  int a;
  int b;
  // int c;
  char globalData[30]; // 各个阶段共用的，不希望被覆盖的
};

struct regionB {
  char name[8];
  char globalData[30];
};

struct regionC {
  char c;
  char resv[7];
  char globalData[30];
};

union sharedRegion {
  struct regionA areaA;
  struct regionB areaB;
  struct regionC areaC;
};

int main(void) {


assert(memberOffsetof(struct regionA, globalData) == memberOffsetof(struct regionB, globalData));
assert(memberOffsetof(struct regionA, globalData) == memberOffsetof(struct regionC, globalData));

    return 0;
}
