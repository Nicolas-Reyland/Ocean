//
// Created by Nicolas Reyland on 8/25/22.
//

#ifndef OCEAN_EXEC_UNTIL_LOOP_H
#define OCEAN_EXEC_UNTIL_LOOP_H

#include <stddef.h>
#include "executable.h"

struct ExecUntilLoop {
    Executable * condition_commands;
    size_t num_condition_commands;
    Executable * body_commands;
    size_t num_body_commands;
};

int exec_until_loop(struct ExecUntilLoop);

#endif // OCEAN_EXEC_UNTIL_LOOP_H
