#pragma once

// Includes every map plus database, agent, proxy and the local server entry.
// This is an in-process table limit; the variable-length wire layout is unchanged.
constexpr unsigned int DRAGON_SERVER_CAPACITY = 128;
static_assert(DRAGON_SERVER_CAPACITY >= 103 + 3, "Full map catalogue must fit");
