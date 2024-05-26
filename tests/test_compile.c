/*

This file tests two bug patterns reported by @DavidKorczynski in https://github.com/kokke/tiny-regex-c/issues/44

*/

#include <assert.h>
#include <stdlib.h> /* for NULL */
#include "re.h"

void hexdump(const unsigned char *data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        printf("\\x%02x", data[i]);
    }
    printf("\n");
}

int hex_to_int(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else {
        return -1;
    }
}

// Function to convert a hex string to a byte array
unsigned char *hex_to_bytes(const char *hex, size_t *length) {
    size_t len = strlen(hex);
    if (len % 2 != 0) {
        return NULL;  // Invalid hex string
    }
    *length = len / 2;
    unsigned char *bytes = malloc(*length);
    for (size_t i = 0; i < *length; i++) {
        int high = hex_to_int(hex[2 * i]);
        int low = hex_to_int(hex[2 * i + 1]);
        if (high == -1 || low == -1) {
            free(bytes);
            return NULL;  // Invalid hex character
        }
        bytes[i] = (high << 4) | low;
    }
    return bytes;
}

int main(int argc, char** argv)
{
  int length;
  if (argc == 3)
  {
    size_t pattern_len;
    re_t *compiled_pattern = NULL;
    if(argv[2] != NULL){
      compiled_pattern = hex_to_bytes(argv[2], &pattern_len);
    }
    //hexdump(compiled_pattern, pattern_len);
    //hexdump(re_compile(argv[1]), pattern_len);
    assert(0 == memcmp(compiled_pattern, re_compile(argv[1]), pattern_len));
  }
  else
  {
    printf("\nUsage: %s <PATTERN> <HEX_COMPILED_PATTERN> \n", argv[0]);
  }

  return 0;
}

