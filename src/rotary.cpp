#include "rotary.hpp"

uint8_t Rotary::num_encoders = 0;

PIO Rotary::pio[MAX_ENCODERS] = {NULL, NULL};

int Rotary::pin_clk[MAX_ENCODERS] = {-1, -1};
int Rotary::pin_dt[MAX_ENCODERS] = {-1, -1};
int Rotary::pin_sw[MAX_ENCODERS] = {-1, -1};

uint8_t Rotary::sw_state[MAX_ENCODERS] = {0, 0};

rotary_callback_t Rotary::cb[MAX_ENCODERS] = {NULL, NULL};
