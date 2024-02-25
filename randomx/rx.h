#include <stdint.h>
#include <stddef.h>

uint64_t randomx_seedheight(const uint64_t height);
void* randomx_init_worker();
int randomx_set_seedhash(const char *seedhash);
int randomx_scanhash(int thr_id, void* rx_vm, uint32_t data[32], uint32_t target[8], uint32_t max_nonce, unsigned long* hashes_done);
