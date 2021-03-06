#include "donationsIntf.hpp"

#include <eosio/check.hpp>

#include "core/errormessages.hpp"
#include "core/fixedprops.hpp"

namespace system_epn
{
    using eosio::block_timestamp;
    using eosio::check;
    using eosio::const_mem_fun;
    using eosio::current_block_time;
    using eosio::indexed_by;
    using eosio::name;
    using eosio::seconds;
    using std::back_inserter;
    using std::distance;
    using std::find_if;
    using std::string;
    using std::transform;
    using std::vector;

    using SignerMIType = eosio::multi_index<"donsigners"_n,
                                            DonationSignature,
                                            indexed_by<"bysigner"_n, const_mem_fun<DonationSignature, uint64_t, &DonationSignature::get_secondary_1>>,
                                            indexed_by<"bycontractid"_n, const_mem_fun<DonationSignature, uint64_t, &DonationSignature::get_secondary_2>>,
                                            indexed_by<"byservblock"_n, const_mem_fun<DonationSignature, uint64_t, &DonationSignature::get_secondary_3>>>;

    using DrafterMIType = eosio::multi_index<"dondrafts"_n, DonationDraft>;

    DonationsIntf::DonationsIntf(const name& drafter, const name& contractID)
        : drafter(drafter)
        , contractID(contractID) {
        check(exists(drafter, contractID), error::contractDNE.data());
        _draft = *(DrafterMIType(fixedProps::contract_account, drafter.value).find(contractID.value));

        // Get all signers
        SignerMIType allSignatures(fixedProps::contract_account, fixedProps::contract_account.value);
        auto sigsByID = allSignatures.get_index<"bycontractid"_n>();
        for_each(sigsByID.lower_bound(contractID.value), sigsByID.upper_bound(contractID.value), [&](const DonationSignature& row) {
            if (row.drafter == drafter) _signatures.push_back(row);
        });
    }

    void DonationsIntf::sign(const name& signer, const Asset& quantity, const Frequency& frequency, const Memo& signerMemo) {
        check(signer != drafter, error::invalidSigner.data());
        SignerMIType signatures(fixedProps::contract_account, fixedProps::contract_account.value);

        auto sigsBySigner = signatures.get_index<"bysigner"_n>();
        auto feq = [&](const DonationSignature& row) { return row.contractID == contractID && row.drafter == drafter; };
        auto itr = find_if(sigsBySigner.lower_bound(signer.value), sigsBySigner.upper_bound(signer.value), feq);
        check(itr == sigsBySigner.upper_bound(signer.value), error::duplicateSigner.data());

        // Calculate next block timestamp where we should service the pull transaction
        auto timestamp = current_block_time();
        auto nextTimestamp = block_timestamp(timestamp.to_time_point() + seconds(frequency.value));

        uint64_t index = static_cast<uint64_t>(distance(signatures.begin(), signatures.end()));
        auto addSigner = [&](DonationSignature& row) {
            row.index = index;
            row.signer = signer;
            row.contractID = contractID;
            row.serviceBlock = nextTimestamp;
            row.drafter = drafter;
            row.quantity = quantity;
            row.frequency = frequency;
            row.signerMemo = signerMemo;
        };

        const auto ram_payer = signer;
        signatures.emplace(ram_payer, addSigner);
    }

    Memo DonationsIntf::getMemoSuffix() const {
        return _draft.memoSuffix;
    }

    size_t DonationsIntf::getNumSigners() const {
        return _signatures.size();
    }

    DonationSignature DonationsIntf::getSignature(const name& signer) const {
        auto itr = std::find_if(_signatures.begin(), _signatures.end(), [&](const auto& signature) { return signature.signer == signer; });
        eosio::check(itr != _signatures.end(), error::invalidSigner.data());
        return *itr;
    }

    bool DonationsIntf::exists(const name& drafter, const name& contractID) {
        auto drafts = DrafterMIType(fixedProps::contract_account, drafter.value);
        auto itr = drafts.find(contractID.value);
        return (itr != drafts.end());
    }

    void DonationsIntf::draft(const name& owner, const name& contractID, const Memo& memoSuffix) {
        auto drafts = DrafterMIType(fixedProps::contract_account, owner.value);
        check(drafts.find(contractID.value) == drafts.end(), error::doubleDraft.data());

        auto configureNewDraft = [&](auto& row) {
            row.contractID = contractID;
            row.memoSuffix = memoSuffix;
        };

        auto ram_payer = owner;
        drafts.emplace(ram_payer, configureNewDraft);  //Todo change back to ramPpayer
    }

    vector<DonationSignature> DonationsIntf::getAllContractSignatures() {
        SignerMIType allSignatures(fixedProps::contract_account, fixedProps::contract_account.value);
        auto byContractID = allSignatures.get_index<"bycontractid"_n>();

        vector<DonationSignature> _signatures;
        transform(byContractID.cbegin(), byContractID.cend(), back_inserter(_signatures), [&](const DonationSignature& row) { return row; });

        return _signatures;
    }

}  // namespace system_epn
