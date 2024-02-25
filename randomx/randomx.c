#include <stdbool.h>
#include <randomx.h>

#include "miner.h"

int rx_scanhash(int thr_id, randomx_vm* vm, uint32_t data[32], uint32_t target[8], uint32_t max_nonce, unsigned long* hashes_done) {
  uint32_t first_nonce = data[19];
  uint32_t n = data[19] - 1;
  uint8_t block_hash[32];
  uint32_t rx_hash[8];

  do {
    data[19] = ++n;
    sha256d(block_hash, (unsigned char*)data, 80);
    randomx_calculate_hash(vm, block_hash, 32, rx_hash);
    if (fulltest(rx_hash, target)) {
      char* hexhash = abin2hex((unsigned char*)block_hash, 32);
      char* rxhex = abin2hex((unsigned char*)rx_hash, 32);
      *hashes_done = n - first_nonce + 1;
      return 1;
    }
  } while (n < max_nonce && !work_restart[thr_id].restart);
	*hashes_done = n - first_nonce + 1;
  data[19] = n;
  return 0;
}
