#include <stdio.h>
#include "../fsm.h"

enum states { STATE0, STATE1, NUM_STATES };

void state_enter(void *ctx, size_t n) { (void)ctx; (void)n; printf("Entering state\n"); }
void state_tick(void *ctx, size_t n, float dt) { (void)ctx; (void)n; (void)dt; printf("Ticking state\n"); }
void state_draw(void *ctx, size_t n) { (void)ctx; (void)n; printf("Drawing state\n"); }
void state_exit(void *ctx, size_t n) { (void)ctx; (void)n; printf("Exiting state\n"); }

int main() {
  fsm_t sm;
  fsm_init(&sm, STATE0, NUM_STATES);

  fsm_state_t state0 = {state_enter, state_tick, state_draw, state_exit};
  fsm_state_t state1 = {state_enter, NULL, NULL, state_exit};

  fsm_register_state(&sm, STATE0, &state0);
  fsm_register_state(&sm, STATE1, &state1);

  if (!fsm_start(&sm)) {
    printf("Failed to start FSM\n");
    fsm_free(&sm);
    return 1;
  }

  for (unsigned i = 0; i < 6; i++) {
    printf("frame %d\n", i);
    fsm_tick_state(&sm, 0.1f);
    fsm_draw_state(&sm);

    if (i == 3) {
      fsm_change_state(&sm, STATE1);
    }
  }

  fsm_free(&sm);

  return 0;
}