#include "consteval_hash.h"

int main()
{
  const char * val = "dog";
  flow::consteval_wyhash(val, 3, 0);
}