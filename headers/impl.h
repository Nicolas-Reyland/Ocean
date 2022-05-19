//
// Created on 17/05/2022.
//

#include "parser/parser.h"

#ifndef OCEAN_IMPL_H
#define OCEAN_IMPL_H

Parser command_parser();
Parser command_tail_parser();
Parser name_parser();
Parser names_parser();
Parser cmd_sep_parser();
Parser pipe_parser();
Parser amp_parser();
Parser new_cmd_parser();
Parser literal_parser();
Parser doubleq_parser();
Parser singleq_parser();

#endif // OCEAN_IMPL_H