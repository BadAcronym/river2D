#pragma once

#include "river2D_main.h"

extern void mapedit_update
(
    void
);

extern void mapedit_processControls
(
    bool               isDown,
    uint64_t           key,
    River2D_ControlMap *controls
);
