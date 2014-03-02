#include <boost/test/unit_test.hpp>

#include "main.h"
#include "wallet.h"

using namespace std;

BOOST_AUTO_TEST_SUITE(unit_tests)

//
// Helper: create two dummy transactions, each with
// two outputs.  The first has 11 and 50 CENT outputs
// paid to a TX_PUBKEY, the second 21 and 22 CENT outputs
// paid to a TX_PUBKEYHASH.
// Both are shares.
//
static std::vector<CTransaction>
SetupDummyInputs(CBasicKeyStore& keystoreRet, MapPrevTx& inputsRet)
{
    std::vector<CTransaction> dummyTransactions;
    dummyTransactions.resize(2);

    // Add some keys to the keystore:
    CKey key[4];
    for (int i = 0; i < 4; i++)
    {
        key[i].MakeNewKey(i % 2);
        keystoreRet.AddKey(key[i]);
    }

    // Create some dummy input transactions
    dummyTransactions[0].cUnit = 'S';
    dummyTransactions[0].vout.resize(2);
    dummyTransactions[0].vout[0].nValue = 11*CENT;
    dummyTransactions[0].vout[0].scriptPubKey << key[0].GetPubKey() << OP_CHECKSIG;
    dummyTransactions[0].vout[1].nValue = 50*CENT;
    dummyTransactions[0].vout[1].scriptPubKey << key[1].GetPubKey() << OP_CHECKSIG;
    inputsRet[dummyTransactions[0].GetHash()] = make_pair(CTxIndex(), dummyTransactions[0]);

    dummyTransactions[1].cUnit = 'S';
    dummyTransactions[1].vout.resize(2);
    dummyTransactions[1].vout[0].nValue = 21*CENT;
    dummyTransactions[1].vout[0].scriptPubKey.SetBitcoinAddress(key[2].GetPubKey());
    dummyTransactions[1].vout[1].nValue = 22*CENT;
    dummyTransactions[1].vout[1].scriptPubKey.SetBitcoinAddress(key[3].GetPubKey());
    inputsRet[dummyTransactions[1].GetHash()] = make_pair(CTxIndex(), dummyTransactions[1]);

    return dummyTransactions;
}

BOOST_AUTO_TEST_CASE(test_same_unit_in_transaction)
{
    CBasicKeyStore keystore;
    MapPrevTx dummyInputs;
    std::vector<CTransaction> dummyTransactions = SetupDummyInputs(keystore, dummyInputs);

    CTransaction t1;
    t1.vin.resize(3);
    t1.vin[0].prevout.hash = dummyTransactions[0].GetHash();
    t1.vin[0].prevout.n = 1;
    t1.vin[0].scriptSig << std::vector<unsigned char>(65, 0);
    t1.vin[1].prevout.hash = dummyTransactions[1].GetHash();
    t1.vin[1].prevout.n = 0;
    t1.vin[1].scriptSig << std::vector<unsigned char>(65, 0) << std::vector<unsigned char>(33, 4);
    t1.vin[2].prevout.hash = dummyTransactions[1].GetHash();
    t1.vin[2].prevout.n = 1;
    t1.vin[2].scriptSig << std::vector<unsigned char>(65, 0) << std::vector<unsigned char>(33, 4);
    t1.vout.resize(1);
    t1.vout[0].nValue = 90*CENT;
    t1.vout[0].scriptPubKey << OP_1;

    // No unit defined
    BOOST_CHECK(!t1.CheckTransaction());

    // Same unit as input
    t1.cUnit = 'S';
    BOOST_CHECK(t1.CheckTransaction());
    BOOST_CHECK(t1.AreInputsSameUnit(dummyInputs));

    // Different unit
    t1.cUnit = 'U';
    BOOST_CHECK(!t1.AreInputsSameUnit(dummyInputs));

    dummyInputs[dummyTransactions[0].GetHash()].second.cUnit = 'U';
    BOOST_CHECK(!t1.AreInputsSameUnit(dummyInputs));

    // Same unit again
    dummyInputs[dummyTransactions[1].GetHash()].second.cUnit = 'U';
    BOOST_CHECK(t1.AreInputsSameUnit(dummyInputs));
}

BOOST_AUTO_TEST_CASE(test_changing_unit_invalidates_signature)
{
    CBasicKeyStore keystore;
    MapPrevTx dummyInputs;
    std::vector<CTransaction> dummyTransactions = SetupDummyInputs(keystore, dummyInputs);

    CTransaction t1;
    t1.vin.resize(1);
    t1.vin[0].prevout.hash = dummyTransactions[0].GetHash();
    t1.vin[0].prevout.n = 1;
    t1.vin[0].scriptSig << std::vector<unsigned char>(65, 0);
    t1.vout.resize(1);
    t1.vout[0].nValue = 11*CENT;
    t1.vout[0].scriptPubKey << OP_1;
    t1.cUnit = 'S';

    BOOST_CHECK(SignSignature(keystore, dummyTransactions[0], t1, 0));

    BOOST_CHECK(t1.CheckTransaction());
    BOOST_CHECK(VerifySignature(dummyTransactions[0], t1, 0, true, 0));

    t1.cUnit = 'U';
    BOOST_CHECK(!VerifySignature(dummyTransactions[0], t1, 0, true, 0));
}

BOOST_AUTO_TEST_SUITE_END()

