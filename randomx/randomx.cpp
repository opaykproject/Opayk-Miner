#include <cstring>
#include <iostream>
#include <thread>
#include <mutex>
#include <randomx.h>

static const size_t HASH_SIZE = 32;

static randomx_flags flags;
static bool flags_set = false;

static std::mutex seedhash_lock;
static char current_seedhash[HASH_SIZE];
static int current_seedhash_set = 0;
static randomx_dataset *dataset = nullptr;

static bool is_same_seed(const char* seedhash) { return current_seedhash_set && (memcmp(current_seedhash, seedhash, HASH_SIZE) == 0); }

#define SEEDHASH_EPOCH_BLOCKS	2048
#define SEEDHASH_EPOCH_LAG		64

static bool alloc_cache(randomx_cache** cache)
{
  if (*cache) {
    return true;
  }

  if (flags_set) {
    *cache = randomx_alloc_cache(flags);
    if (!*cache) return false;
  } else {
    flags = randomx_get_flags();
    flags |= RANDOMX_FLAG_FULL_MEM;
    *cache = randomx_alloc_cache(flags | RANDOMX_FLAG_LARGE_PAGES);
    if (*cache) {
      flags |= RANDOMX_FLAG_LARGE_PAGES;
      flags_set = true;
    } else {
      std::cerr << "Large pages unavailable" << std::endl;
      flags_set = true;
      *cache = randomx_alloc_cache(flags);
      if (!*cache) return false;
    }
  }
  return true;
}

extern "C" {

uint64_t randomx_seedheight(const uint64_t height) {
  return (height <= SEEDHASH_EPOCH_BLOCKS+SEEDHASH_EPOCH_LAG) ? 0 : (height - SEEDHASH_EPOCH_LAG - 1) & ~(SEEDHASH_EPOCH_BLOCKS-1);
}

void* randomx_init_worker() {
  return randomx_create_vm(flags, nullptr, dataset);
}

int randomx_set_seedhash(const char *seedhash) {
  std::unique_lock<std::mutex> lock(seedhash_lock);
  if (is_same_seed(seedhash)) {
    return 1;
  }
  memcpy(current_seedhash, seedhash, HASH_SIZE);
  current_seedhash_set = 1;

  randomx_cache* cache = nullptr;
  if (!alloc_cache(&cache)) return 0;
  randomx_init_cache(cache, current_seedhash, HASH_SIZE);

  dataset = randomx_alloc_dataset(flags);
  if (dataset == nullptr) {
    std::cerr << "RandomX dataset allocation failed" << std::endl;
    exit(1);
  }
  auto datasetItemCount = randomx_dataset_item_count();
  std::thread t1(&randomx_init_dataset, dataset, cache, 0, datasetItemCount / 2);
  std::thread t2(&randomx_init_dataset, dataset, cache, datasetItemCount / 2, datasetItemCount - datasetItemCount / 2);
  t1.join();
  t2.join();
  randomx_release_cache(cache);

  return 1;
}

extern "C" int rx_scanhash(int thr_id, randomx_vm* rx_vm, uint32_t data[32], uint32_t target[8], uint32_t max_nonce, unsigned long* hashes_done);

int randomx_scanhash(int thr_id, void* rx_vm, uint32_t data[32], uint32_t target[8], uint32_t max_nonce, unsigned long* hashes_done) {
  randomx_vm* vm = (randomx_vm*)rx_vm;
  randomx_vm_set_dataset(vm, dataset);

  return rx_scanhash(thr_id, vm, data, target, max_nonce, hashes_done);
}

}
