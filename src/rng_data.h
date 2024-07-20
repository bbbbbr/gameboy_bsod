
#ifndef _RNG_H
#define _RNG_H

void rng_load(void);
void rng_save(void);
void rng_mix(void);
bool rng_is_initialized(void);

#endif // _RNG_H