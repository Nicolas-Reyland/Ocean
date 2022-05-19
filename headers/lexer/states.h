//
// Created by Nicolas Reyland on 5/10/22.
//

#ifndef OCEAN_STATES_H
#define OCEAN_STATES_H

typedef enum {
    STATE_GENERAL = 0x100,
    STATE_LITERAL = 0x200,
    STATE_AMPERSAND = 0x300,
    STATE_PIPE = 0x400,
    STATE_SINGLE_QUOTES = 0x500,
    STATE_DOUBLE_QUOTES = 0x600,
    STATE_RIGHT_ANGLE_BRACKET = 0x700,
    STATE_LEFT_ANGLE_BRACKET = 0x800,
    STATE_SINGLE_CHAR = 0x900,
    STATE_ESCAPE = 0xe00,
    STATE_EOF = 0xf00,
} STATE;

char * STATE_STRING(STATE);

#endif // OCEAN_STATES_H