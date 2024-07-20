#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>
#include <rand.h>

// Map a secondary stats struct to the beginning of SRAM (0xA000) when using MBC5
uint16_t AT(0xA000) sram_randseed;
uint32_t AT(0xA002) sram_fingerprint;

const uint32_t initialized_fingerprint = 0xA5F011EEu;
bool  sram_rng_has_init = false;

// Load saved RNG seed from SRAM
// Use a minimal test for SRAM init, doesn't matter if it's corrupted
// since it gets rewritten periodically.
void rng_load(void) {
    ENABLE_RAM_MBC5;
    SWITCH_RAM(0);
    initrand(sram_randseed);
    // Meaning: __rand_seed = sram_randseed;

    sram_rng_has_init = (initialized_fingerprint == sram_fingerprint);
    DISABLE_RAM_MBC5;
}


// Save RNG seed to SRAM
void rng_save(void) {
    ENABLE_RAM_MBC5;
    sram_randseed = __rand_seed;

    sram_fingerprint = initialized_fingerprint;
    sram_rng_has_init = true;
    DISABLE_RAM_MBC5;
}


// Mix in user input to the RNG seed
void rng_mix(void) {
    __rand_seed ^= DIV_REG;
    rng_save();
}


bool rng_is_initialized(void) {
    return (sram_rng_has_init);
}