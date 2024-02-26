200 LOC to get your REPL up and running.

### Capabilities

Read input.
Simple history (scroll with keyboard arrows). 
Recognizes CTRL+D (`EOF`) or ENTER (`\n`) as input delimiters. No multiline input as-is.

If you're looking for something more fully-featured, look at [linenoise-mob](github.com/rain-1/linenoise-mob)
instead, or go with `libreadline`, the OG behemoth that does it all, everywhere. 

### Example

You only need to add `tinyline.h` and `tinyline.c` to your project. An example use
could be:

```C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinyline.h"

int main(int argc, char** argv) {
  TinyLine_setup();
  char* command = NULL;

  // 0x04 = EOF
  while(command == NULL || *command != 0x04) {
    command = TinyLine_readLine();
    printf("%s\n", command);
  }

  return 0;
}
```

Save to `example.c` and compile with 
```
cc tinyline.c example.c -o example
```

### Tests

Has integration tests (`make test` or `./test`) written with `expect`.
`expect` was written in 1990 by Don Libes at NIST and still does the job perfectly. Thanks, Don!

### Contributing

If you create a PR (awesome!), please include a test case, ideally in the main `test` file.
