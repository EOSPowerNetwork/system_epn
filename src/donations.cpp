#include <eosio/action.hpp>
#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <string>

#include "donations.hpp"

using namespace eosio;
using std::string;
using namespace system_epn;

donations::donations(name receiver, name code, datastream<const char*> ds) : contract(receiver, code, ds)
{
    /* NOP */
}

void donations::draftdon(const name& owner, const name& contractID)
{
    DonationsTable _donations(get_self(), owner.value);
    // Todo: Check if there is already a donation with this ID made by this caller
}

//void donations::signdon(const name& to, const asset& amount, const uint32_t& frequency) {}