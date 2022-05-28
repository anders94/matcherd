#include <iostream>

// This is very minimal to reduce memory requirements on the matcher.
// There is no "side" because buys and sells are in different maps.
// There are no prices because the map keys are prices and the values
// are queues of these order structures.

struct offer {
    uint64_t userId;
    uint64_t volume;
};
