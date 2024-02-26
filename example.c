#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinyline.h"

int main(__unused int argc, __unused char** argv) {
  TinyLine_setup();
  char* command = NULL;

  // 0x04 = EOF
  while(command == NULL || *command != 0x04) {
    command = TinyLine_readLine();
    printf("%s\n", command);
  }

  return 0;
}
