#pragma once

#include "flow/message_registry.hpp"
#include "flow/system.hpp"

namespace flow {
/**
 * Parse arguments passed in to application execute the options selected
 * @param argc The argc in main
 * @param argv The argv in main
 */
void begin(int argc, char **argv);
void begin();

}// namespace flow
