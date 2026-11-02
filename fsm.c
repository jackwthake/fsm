#include "fsm.h"

#include <stdlib.h>

static int fsm_valid(const fsm_t *sm) {
  return sm && sm->states && sm->current_state < sm->num_states;
}

// Initialize FSM and allocate state interfaces
void fsm_init(fsm_t *sm, unsigned default_state, unsigned num_states) {
  if (!sm) return;

  sm->default_state = default_state;
  sm->current_state = default_state;

  sm->states = calloc(num_states, sizeof(fsm_state_t));
  sm->num_states = num_states;
  sm->ctx_cleanup = NULL;

  sm->ctx = NULL;
  sm->ctx_size = 0;
}

// Enter the current state to begin FSM execution
int fsm_start(fsm_t *sm) {
  if (!fsm_valid(sm)) return -1;

  if (fsm_valid(sm) && sm->states[sm->current_state].enter)
    sm->states[sm->current_state].enter(sm->ctx, sm->ctx_size);
  
  return 1;
}

// Free allocated state interface array
void fsm_free(fsm_t *sm) {
  if (!sm) return;

  if (sm->states) {
    if (fsm_valid(sm) && sm->states[sm->current_state].exit) {
      sm->states[sm->current_state].exit(sm->ctx, sm->ctx_size);
    }

    free(sm->states);
    sm->states = NULL;
    sm->num_states = 0;
  }

  if (sm->ctx_cleanup) {
    sm->ctx_cleanup(sm->ctx, sm->ctx_size);
  }

  sm->ctx = NULL;
  sm->ctx_size = 0;
}

// Register callbacks for a specific state
int fsm_register_state(fsm_t *sm, unsigned state, fsm_state_t *interface) {
  if (!sm || !interface) return -1;
  if (state >= sm->num_states) return -1;

  sm->states[state] = *interface;

  return 1;
}

// Update the shared state pointer passed to all callbacks
int fsm_update_ctx(fsm_t *sm, void *state, size_t size) {
  if (!sm || !state) return -1;

  sm->ctx = state;
  sm->ctx_size = size;
  return 1;
}

// Set the cleanup function for the shared context, not required
void fsm_set_ctx_cleanup(fsm_t *sm, void (*ctx_cleanup)(void *, size_t)) {
  if (!sm) return;
  sm->ctx_cleanup = ctx_cleanup;
}

// Transition between states, calling exit and enter callbacks
int fsm_change_state(fsm_t *sm, unsigned new_state) {
  if (!sm || !sm->states) return -1;
  if (new_state >= sm->num_states) return -1;

  if (fsm_valid(sm) && sm->states[sm->current_state].exit)
    sm->states[sm->current_state].exit(sm->ctx, sm->ctx_size);

  sm->current_state = new_state;

  if (fsm_valid(sm) && sm->states[sm->current_state].enter)
    sm->states[sm->current_state].enter(sm->ctx, sm->ctx_size);

  return 1;
}

// Return the current state index
int fsm_get_state(fsm_t *sm) {
  if (!sm) return -1;

  return (int)sm->current_state;
}

// Execute tick callback with delta time for fixed timestep updates
void fsm_tick_state(fsm_t *sm, float dt) {
  if (!sm) return;

  if (fsm_valid(sm) && sm->states[sm->current_state].tick)
    sm->states[sm->current_state].tick(sm->ctx, sm->ctx_size, dt);
}

// Execute draw callback for the current state
void fsm_draw_state(fsm_t *sm) {
  if (!sm) return;

  if (fsm_valid(sm) && sm->states[sm->current_state].draw)
    sm->states[sm->current_state].draw(sm->ctx, sm->ctx_size);
}
