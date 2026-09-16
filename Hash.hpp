#pragma once
#include "Quasar.hpp"

namespace Quasar
{
    int get_random_32_number();
    void init_random_keys(State& state);
    int generate_hash_key(State& state);
}
