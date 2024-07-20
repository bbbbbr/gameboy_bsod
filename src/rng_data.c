#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>
#include <rand.h>

// Map a secondary stats struct to the beginning of SRAM (0xA000) when using MBC5
uint16_t __at (0xA000) sram_randseed;


// Load saved RNG seed from SRAM
// Don't really care if it's been initialized the first time
// since it's intended to be sort of stateless and rolling.
// So there isn't any checksum or other validation. OK for now.
void rng_load(void) {
    ENABLE_RAM_MBC5;
    initrand(sram_randseed);
    // Meaning: __rand_seed = sram_randseed;
    DISABLE_RAM_MBC5;
}


// Save RNG seed to SRAM
void rng_save(void) {
    ENABLE_RAM_MBC5;
    sram_randseed = __rand_seed;
    DISABLE_RAM_MBC5;
}


// Mix in user input to the RNG seed
void rng_mix(void) {
    __rand_seed ^= DIV_REG;
    rng_save();
}