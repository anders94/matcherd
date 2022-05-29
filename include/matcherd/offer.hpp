// This is very minimal to reduce memory requirements on the matcher.
// There is no "side" because buys and sells are in different maps.
// There is no price because map keys are the prices with values as
// are queues of these offer structures.

struct offer {
    uint64_t offerId;
    uint64_t volume;
};
