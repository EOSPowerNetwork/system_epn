#pragma once

#include <eosio/eosio.hpp>
#include <eosio/name.hpp>

#include <string>
#include <vector>

namespace system_epn
{
    using eosio::name;
    using std::string;
    using std::vector;

    struct Donations
    {
        name to;
        uint32_t lastPayout;
        string memoSuffix;

        uint64_t primary_key() const { return to.value; }
    };
    EOSIO_REFLECT(Donations, to, lastPayout, memoSuffix);
}  // namespace system_epn