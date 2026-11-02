#ifndef FSM_H
#define FSM_H

#include <stddef.h>

// Callbacks for state lifecycle and execution
typedef struct {
  void (*enter)(void *, size_t);              // Called when entering state
  void (*tick)(void *, size_t, float dt);     // Called each timestep
  void (*draw)(void *, size_t);               // Called each frame
  void (*exit)(void *, size_t);               // Called when exiting state
} fsm_state_t;

// Finite state machine
typedef struct {
  fsm_state_t *states;
  unsigned num_states;

  void *ctx;                                  // Shared state passed to callbacks
  size_t ctx_size;

  unsigned default_state, current_state;
  void (*ctx_cleanup)(void *, size_t);        // Optional cleanup function for ctx (takes ctx and ctx_size)
} fsm_t;

// Initialize state machine with default state and total number of states
void fsm_init(fsm_t *sm, unsigned default_state, unsigned num_states);

// Start the state machine by entering the current state
int fsm_start(fsm_t *sm);

// Free state machine resources
void fsm_free(fsm_t *sm);

// Register state callbacks for a specific state
int fsm_register_state(fsm_t *sm, unsigned state, fsm_state_t *interface);

// Transition to a new state (calls exit on current, enter on new)
int fsm_change_state(fsm_t *sm, unsigned new_state);

// Update the shared ctx state pointer passed to all callbacks
int fsm_update_ctx(fsm_t *sm, void *state, size_t size);

// Set the cleanup function for the shared context, not required
void fsm_set_ctx_cleanup(fsm_t *sm, void (*ctx_cleanup)(void *, size_t));

// Get the current state index
int fsm_get_state(fsm_t *sm);

// Execute tick callback for current state with delta time
void fsm_tick_state(fsm_t *sm, float dt);

// Execute draw callback for current state
void fsm_draw_state(fsm_t *sm);

#endif
