#include <stdint.h>

void* progminer_setup_devices();
void progminer_start(void* settings, void (on_solution_found)(uint32_t nonce, int height));
void progminer_set_work(uint32_t header[32], int height, uint8_t* target);
void progminer_pause();
