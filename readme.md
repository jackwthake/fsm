# fsm

[![Build](https://github.com/jackwthake/fsm/actions/workflows/build.yml/badge.svg)](https://github.com/jackwthake/fsm/actions/workflows/build.yml)

A **tiny, callback-driven finite state machine** for C. One `.c` file, one `.h` file, libc only. Built for game scenes and GUI screens, and used unmodified across several of my projects.

## Table of Contents
- [Key Features](#key-features)
- [Design Notes](#design-notes)
- [Quick Start](#quick-start)
- [Used In](#used-in)
- [Building](#building)
- [API Reference](#api-reference)

# Key Features

- **Four lifecycle callbacks**: `enter`, `tick` (with delta time), `draw`, and `exit`, each optional
- **Transitions run exit then enter**: `fsm_change_state` calls `exit` on the old state and `enter` on the new one
- **Context swapping**: Replace the shared context mid-run (for example, a scene allocating its own data in `enter`)
- **Optional context cleanup**: Register a function that `fsm_free` calls on the final context
- **Small footprint**: One `calloc` at init, and every operation after that is an indexed lookup and a call
- **Portable C99**: No platform code, no threads, no dependencies beyond `<stdlib.h>` and `<stddef.h>`

# Design Notes

**Calloc'd state table**: Registering a state copies your callback struct into a zero-initialized array, so anything you leave unset is `NULL` and gets skipped. That is the whole mechanism behind "populate only what you need."

**Scenes as data**: A state is a plain `fsm_state_t`. Designated initializers read well and survive reordering of the struct:

```c
fsm_state_t menu = { .enter = menu_enter, .tick = menu_tick };
```

**Context is borrowed, not owned**: The FSM stores the pointer you give it and never frees it, apart from calling the optional cleanup function on whatever context is current when you call `fsm_free`.

**Transitions inside callbacks**: Calling `fsm_change_state` from inside `tick` works, and is the usual way to drive scenes. Treat it as the last statement of that callback: the rest of the callback still runs after the swap, against the new state's context.

# Quick Start

A complete program with two states that runs in a terminal (`test/main.c`):

```c
#include <stdio.h>
#include "fsm.h"

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
```

Output:

```
Entering state
frame 0
Ticking state
Drawing state
frame 1
Ticking state
Drawing state
frame 2
Ticking state
Drawing state
frame 3
Ticking state
Drawing state
Exiting state
Entering state
frame 4
frame 5
Exiting state
```

# Used In

- **[wired](https://github.com/jackwthake/wired)**: Manage main game state, features nested state machines
- **[shader-works](https://github.com/jackwthake/shader-works)**: Game states for several software-rendered 3D demos

# Building

It's two files. Copy `fsm.c` and `fsm.h` into your project. This module is highly portable.

Requirements: a C99 compiler.

# API Reference

## Types

```c
typedef struct {
  void (*enter)(void *ctx, size_t ctx_size);
  void (*tick)(void *ctx, size_t ctx_size, float dt);
  void (*draw)(void *ctx, size_t ctx_size);
  void (*exit)(void *ctx, size_t ctx_size);
} fsm_state_t;
```
Callbacks for one state. Any member can be `NULL`.

## Lifecycle

```c
void fsm_init(fsm_t *sm, unsigned default_state, unsigned num_states);
```
Initialize the machine and allocate a table of `num_states` zeroed entries. The current state is set to `default_state`, but nothing runs until `fsm_start`.

---
```c
int fsm_start(fsm_t *sm);
```
Enter the current state by calling its `enter` callback if it has one. Returns a negative value on failure (invalid machine or out-of-range state).

---
```c
void fsm_free(fsm_t *sm);
```
Call the context cleanup function if one is set, then free the state table. Does call `exit` on the current state.

## Configuration

```c
int fsm_register_state(fsm_t *sm, unsigned state, fsm_state_t *callbacks);
```
Copy `callbacks` into slot `state`. Returns a negative value if `state` is out of range.

---
```c
int fsm_update_ctx(fsm_t *sm, void *ctx, size_t size);
```
Set the pointer and size passed to every callback. Can be called at any time, including from inside a callback.

---
```c
void fsm_set_ctx_cleanup(fsm_t *sm, void (*cleanup)(void *, size_t));
```
Optional. Called by `fsm_free` with the context that is current at that moment. The FSM never frees `ctx` itself.

## Running

```c
int fsm_change_state(fsm_t *sm, unsigned new_state);
```
Call `exit` on the current state, switch, then call `enter` on the new one. Changing to the current state runs both. Returns a negative value if `new_state` is out of range.

---
```c
void fsm_tick_state(fsm_t *sm, float dt);
void fsm_draw_state(fsm_t *sm);
```
Call the current state's `tick` (with `dt`) or `draw` callback if set.

---
```c
int fsm_get_state(fsm_t *sm);
```
Return the current state index, or a negative value if `sm` is `NULL`.
