A lightweight c/c++ LRU memory cache. Years ago, I have implemented the same function with STL, a hashmp to access the items and a list to choose which items to discard. But I fell that will use more memory. So in a new project I try to rewrite one.

**Example:**
```

#include "lruc_ss.h"
#include <stdio.h>

int main(){

    lruc_t lruc = lruc_ss_new(64, 128);

    lruc_ss_insert(lruc, "fan", "abcd");


    const char* v = lruc_ss_find(lruc, "fan");

    if(v != 0)
        printf("v = %s\n", v);

    lruc_ss_free(lruc);
}

```



---
