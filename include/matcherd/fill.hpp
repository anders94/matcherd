// Fills are created when an order matches an offer.

struct fill {
    uint64_t sequence;
    uint64_t offerId;
    uint64_t takerId;
    uint64_t price;
    uint64_t volume;

};

std::string stringify_fill(fill f) {
    std::string s;

    s = std::to_string(f.sequence) + "|" + std::to_string(f.offerId) + "|" +
        std::to_string(f.takerId) + "|" + std::to_string(f.price) + "|" +
        std::to_string(f.volume);

    return s;

}
