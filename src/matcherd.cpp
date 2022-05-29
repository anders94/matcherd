#include <iostream>
#include <deque>
#include <map>

#include <sw/redis++/redis++.h>

#include "matcherd/order.hpp"
#include "matcherd/offer.hpp"
#include "matcherd/fill.hpp"

// single threaded matcher

void print_queue(std::deque<offer> q) {
    for(offer o : q) {
      std::cout << o.volume << ' ';
    }
    std::cout << '\n';
}

void print_map(const std::map<uint64_t, std::deque<offer>>& m) {
    for (const auto& [k, q] : m) {
        std::cout << '[' << k << "] ";
        print_queue(q);
    }
    std::cout << '\n';
}

void parse_order(std::string *s, order *o) {
    std::string delimiter = "|";
    size_t pos = 0;

    try {
        if ((pos = s->find(delimiter)) != std::string::npos) {
            o->orderId = std::stoull(s->substr(0, pos));
            s->erase(0, pos + delimiter.length());
            if ((pos = s->find(delimiter)) != std::string::npos) {
                o->side = s->substr(0, pos);
                s->erase(0, pos + delimiter.length());
                if ((pos = s->find(delimiter)) != std::string::npos) {
                    o->price = std::stoull(s->substr(0, pos));
                    s->erase(0, pos + delimiter.length());
                    if ((pos = s->find(delimiter)) == std::string::npos) {
                        o->volume = std::stoull(*s);

                    }

                }

            }

        }

    }
    catch (std::invalid_argument const& ex) {
        std::cout << "Error Parsing: " << *s << std::endl;

    }

}

fill build_fill(offer off, order ord) {
    fill f;
    f.offerId = off.offerId;
    f.price = ord.price;
    f.takerId = ord.orderId;
    f.volume = off.volume;

    return f;
}

int main(void) {
    auto redis = sw::redis::Redis("tcp://127.0.0.1:6379/0");

    std::map<uint64_t, std::deque<offer>> buys { };
    std::map<uint64_t, std::deque<offer>> sells { };

    bool chatty = false;
    int count = 0;

    while (count < 1000000) {
        std::deque<fill> fills; // the fills this order generates

        // await the next order
        auto value = redis.blpop("btc-usd");

        if (value) {
            order ord;
            parse_order(&value->second, &ord);

            if (chatty)
                std::cout << "order: " << ord.side << ' ' << ord.volume << " @ " << ord.price << std::endl;

            std::map<uint64_t, std::deque<offer>>::iterator mit;
            if (ord.side.compare("B") == 0) { // handle buy
                // see if we have a cheaper sell
                if (!sells.empty()) {
                    mit = sells.begin();
                    if (mit->first < ord.price) {
                        // adjust the price to the top of the book
                        ord.price = mit->first;

                    }

                }

                // see if we have a sell at that price
                mit = sells.find(ord.price);
                if (mit != sells.end()) {
                    // if so, consume as much as possible
                    while (ord.volume > 0 && !mit->second.empty()) {
                        if (ord.volume < mit->second.at(0).volume) {
                            if (chatty)
                                std::cout << "FILL " << ord.volume << " @ " << ord.price << '\n';

                            fill f = build_fill(mit->second.at(0), ord);
                            fills.push_back(f);

                            mit->second.at(0).volume -= ord.volume;
                            ord.volume = 0;
                        }
                        else { // volume >= mit->second.at(0).volume
                            if (chatty)
                                std::cout << "FILL " << mit->second.at(0).volume << " @ " << ord.price << '\n';

                            fill f = build_fill(mit->second.at(0), ord);
                            fills.push_back(f);

                            ord.volume -= mit->second.at(0).volume;
                            // remove offer from queue
                            mit->second.pop_front();

                            // if queue is empty, remove queue from map
                            if (mit->second.empty())
                                sells.erase(ord.price);

                        }

                    }
                    if (chatty)
                        std::cout << '\n';

                }

                // if left over, add to buys
                if (ord.volume > 0) {
                    offer off;
                    off.offerId = ord.orderId;
                    off.volume = ord.volume;
                    buys[ord.price].push_back(off);

                }

            }
            else if (ord.side.compare("S") == 0) { // handle sell
                // see if we have a more expensive buy
                if (!buys.empty()) {
                    mit = --buys.end(); // go to the end because thats the highest price
                    if (mit->first > ord.price) {
                        // adjust the price to the top of the book
                        ord.price = mit->first;

                    }

                }

                // see if we have a buy at that price
                mit = buys.find(ord.price);
                if (mit != buys.end()) {
                    // if so, consume as much as possible
                    while (ord.volume > 0 && !mit->second.empty()) {
                        if (ord.volume < mit->second.at(0).volume) {
                            if (chatty)
                                std::cout << "FILL " << ord.volume << " @ " << ord.price << '\n';

                            fill f = build_fill(mit->second.at(0), ord);
                            fills.push_back(f);

                            mit->second.at(0).volume -= ord.volume;
                            ord.volume = 0;
                        }
                        else { // volume >= mit->second.at(0).volume
                            if (chatty)
                                std::cout << "FILL " << mit->second.at(0).volume << " @ " << ord.price << '\n';

                            fill f = build_fill(mit->second.at(0), ord);
                            fills.push_back(f);

                            ord.volume -= mit->second.at(0).volume;
                            // remove offer from queue
                            mit->second.pop_front();
                            // TODO: publish fill

                            // if queue is empty, remove queue from map
                            if (mit->second.empty())
                                buys.erase(ord.price);

                        }

                    }
                    if (chatty)
                        std::cout << '\n';

                }

                if (ord.volume > 0) {
                    offer off;
                    off.offerId = ord.orderId;
                    off.volume = ord.volume;
                    sells[ord.price].push_back(off);
                    redis.publish("offers", stringify_offer(off));

                }

            }
            else
                std::cout << "Side must be B or S\n";

        }

        for (int f = 0; f < fills.size(); f++) {
            redis.publish("fills", stringify_fill(fills[f]));
        }

        if (chatty) {
            std::cout << "buys:\n";
            print_map(buys);
            std::cout << "sells:\n";
            print_map(sells);
            std::cout << "-----------------------------------------\n\n";
        }

        // throw some artificial delay in there so we can see things working at human speed
        //std::this_thread::sleep_for (std::chrono::milliseconds(500));

        count++;

    }

}
