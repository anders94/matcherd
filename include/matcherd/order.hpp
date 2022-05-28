#include <iostream>

// An order is an object we must act upon. If it doesn't get completely
// filled, it will become an offer which is a subset of an order.

struct order {
    uint64_t userId;
    std::string side;
    uint64_t price;
    uint64_t volume;

};
