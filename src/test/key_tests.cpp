#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>

#include "key.h"
#include "base58.h"
#include "uint256.h"
#include "util.h"

using namespace std;

// Mixed style keys
static const string strDefaultSecret1     ("7QRR3XwvHW963ERsAQQi2JScqv2PgNDcrwMXwAcmJ3g9byEbRqN");
static const string strDefaultSecret2     ("7RexmM8ERicLWvtNVZE3aQnViZLr2gqq1dShPDcSdmgf3MjNx64");
static const string strDefaultSecret1C    ("VGNm8BbQBqchErGkQWKe4jz5Cu396kehHQudY5SrWEonyL9d9dhs");
static const string strDefaultSecret2C    ("VMpZ8jpvG6ey96qjs3EaNTyokRkbQSrWumrRF4fS9JE2uPQ8a1xR");

// NuShares keys
static const string strSSecret1     ("5zn9do6aVdjaNn2F64LjxqG6P8AENnpqp6P1gsj7YXPrstEgQxL");
static const string strSSecret2     ("621hMcGtdrCprUUkRDA5WwbyFmUgj7T3xnUB8vintFQNKHqSpda");
static const string strSSecret1C    ("P42H7QTcqP8vK7iVBKk8dS1ckebk3BGASrkA2ztst99u9Rn9aEhP");
static const string strSSecret2C    ("P9U57xh8ueBCDNHUdrf4wA1MJBKCLsTz5Dgwjz7TXCa95V2dt5c1");

// NuBits keys
static const string strBSecret1     ("62itp2YcM4QoWbbwzdxjv9rsJEmx2VfibjT7w2R1YBQFNWuRPef");
static const string strBSecret2     ("63xSXqivVGt3zJ4TKnn5UGCkAt6QNpHvkRYHP5QgsuQkov3sKB2");
static const string strBSecret1C    ("PCbugZpX1Y44HdjFjQ92Rc7715oPsbXtcJNHbTEyNcucRkyvVT4h");
static const string strBSecret2C    ("PJ3hh8435o6LBtJFBw3xjL6qYcWrBHjiEfK5JSTZ1gKrMpGGxHs2");

// The resulting NSR addresses for the above keys
static const CBitcoinAddress addrS1 ("SkYqsCFMoSkgZh5gqzses71yRWZsvBTYTA");
static const CBitcoinAddress addrS2 ("SbNy74rQ5yGkWw6LiagR8Nnch3TNyMGvd4");
static const CBitcoinAddress addrS1C("Sj6Jtef2gkNXBW5MqxS8qUmsWJeewfKuan");
static const CBitcoinAddress addrS2C("SYij48kVjZiiWawysqsnpF8FL9gaKpPNf9");

static const string strAddressBad("1HV9Lc3sNHZxwj4Zk6fB38tEmBryq2cBiF");


#ifdef KEY_TESTS_DUMPINFO
void dumpKeyInfo(uint256 privkey)
{
    CSecret secret;
    secret.resize(32);
    memcpy(&secret[0], &privkey, 32);
    vector<unsigned char> sec;
    sec.resize(32);
    memcpy(&sec[0], &secret[0], 32);
    printf("  * secret (hex): %s\n", HexStr(sec).c_str());

    for (int nCompressed=0; nCompressed<2; nCompressed++)
    {
        bool fCompressed = nCompressed == 1;
        printf("  * %s:\n", fCompressed ? "compressed" : "uncompressed");
        CBitcoinSecret bsecret;
        bsecret.SetSecret(secret, fCompressed, '?');
        printf("    * secret (base58): %s\n", bsecret.ToString().c_str());
        CKey key;
        key.SetSecret(secret, fCompressed);
        vector<unsigned char> vchPubKey = key.GetPubKey();
        printf("    * pubkey (hex): %s\n", HexStr(vchPubKey).c_str());
        printf("    * address (base58): %s\n", CBitcoinAddress(vchPubKey).ToString().c_str());
    }
}
#endif


BOOST_AUTO_TEST_SUITE(key_tests)

BOOST_AUTO_TEST_CASE(key_test1)
{
    CBitcoinSecret bsecret1, bsecret2, bsecret1C, bsecret2C, baddress1;
    BOOST_CHECK( bsecret1.SetString (strDefaultSecret1,  '?'));
    BOOST_CHECK( bsecret2.SetString (strDefaultSecret2,  '?'));
    BOOST_CHECK( bsecret1C.SetString(strDefaultSecret1C, '?'));
    BOOST_CHECK( bsecret2C.SetString(strDefaultSecret2C, '?'));
    BOOST_CHECK(!baddress1.SetString(strAddressBad,      '?'));

    bool fCompressed;
    CSecret secret1  = bsecret1.GetSecret (fCompressed);
    BOOST_CHECK(fCompressed == false);
    CSecret secret2  = bsecret2.GetSecret (fCompressed);
    BOOST_CHECK(fCompressed == false);
    CSecret secret1C = bsecret1C.GetSecret(fCompressed);
    BOOST_CHECK(fCompressed == true);
    CSecret secret2C = bsecret2C.GetSecret(fCompressed);
    BOOST_CHECK(fCompressed == true);

    BOOST_CHECK(secret1 == secret1C);
    BOOST_CHECK(secret2 == secret2C);

    CKey key1, key2, key1C, key2C;
    key1.SetSecret(secret1, false);
    key2.SetSecret(secret2, false);
    key1C.SetSecret(secret1, true);
    key2C.SetSecret(secret2, true);

    BOOST_CHECK(addrS1.Get()  == CTxDestination(key1.GetPubKey().GetID()));
    BOOST_CHECK(addrS2.Get()  == CTxDestination(key2.GetPubKey().GetID()));
    BOOST_CHECK(addrS1C.Get() == CTxDestination(key1C.GetPubKey().GetID()));
    BOOST_CHECK(addrS2C.Get() == CTxDestination(key2C.GetPubKey().GetID()));

    for (int n=0; n<16; n++)
    {
        string strMsg = strprintf("Very secret message %i: 11", n);
        uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());

        // normal signatures

        vector<unsigned char> sign1, sign2, sign1C, sign2C;

        BOOST_CHECK(key1.Sign (hashMsg, sign1));
        BOOST_CHECK(key2.Sign (hashMsg, sign2));
        BOOST_CHECK(key1C.Sign(hashMsg, sign1C));
        BOOST_CHECK(key2C.Sign(hashMsg, sign2C));

        BOOST_CHECK( key1.Verify(hashMsg, sign1));
        BOOST_CHECK(!key1.Verify(hashMsg, sign2));
        BOOST_CHECK( key1.Verify(hashMsg, sign1C));
        BOOST_CHECK(!key1.Verify(hashMsg, sign2C));

        BOOST_CHECK(!key2.Verify(hashMsg, sign1));
        BOOST_CHECK( key2.Verify(hashMsg, sign2));
        BOOST_CHECK(!key2.Verify(hashMsg, sign1C));
        BOOST_CHECK( key2.Verify(hashMsg, sign2C));

        BOOST_CHECK( key1C.Verify(hashMsg, sign1));
        BOOST_CHECK(!key1C.Verify(hashMsg, sign2));
        BOOST_CHECK( key1C.Verify(hashMsg, sign1C));
        BOOST_CHECK(!key1C.Verify(hashMsg, sign2C));

        BOOST_CHECK(!key2C.Verify(hashMsg, sign1));
        BOOST_CHECK( key2C.Verify(hashMsg, sign2));
        BOOST_CHECK(!key2C.Verify(hashMsg, sign1C));
        BOOST_CHECK( key2C.Verify(hashMsg, sign2C));

        // compact signatures (with key recovery)

        vector<unsigned char> csign1, csign2, csign1C, csign2C;

        BOOST_CHECK(key1.SignCompact (hashMsg, csign1));
        BOOST_CHECK(key2.SignCompact (hashMsg, csign2));
        BOOST_CHECK(key1C.SignCompact(hashMsg, csign1C));
        BOOST_CHECK(key2C.SignCompact(hashMsg, csign2C));

        CKey rkey1, rkey2, rkey1C, rkey2C;

        BOOST_CHECK(rkey1.SetCompactSignature (hashMsg, csign1));
        BOOST_CHECK(rkey2.SetCompactSignature (hashMsg, csign2));
        BOOST_CHECK(rkey1C.SetCompactSignature(hashMsg, csign1C));
        BOOST_CHECK(rkey2C.SetCompactSignature(hashMsg, csign2C));


        BOOST_CHECK(rkey1.GetPubKey()  == key1.GetPubKey());
        BOOST_CHECK(rkey2.GetPubKey()  == key2.GetPubKey());
        BOOST_CHECK(rkey1C.GetPubKey() == key1C.GetPubKey());
        BOOST_CHECK(rkey2C.GetPubKey() == key2C.GetPubKey());
    }
}

BOOST_AUTO_TEST_CASE(key_test2)
{
    CBitcoinSecret bsecret1, bsecret2, bsecret1C, bsecret2C;
    // New encoding for NuShares
    BOOST_CHECK( bsecret1.SetString (strSSecret1,  'S'));
    BOOST_CHECK( bsecret2.SetString (strSSecret2,  'S'));
    BOOST_CHECK( bsecret1C.SetString(strSSecret1C, 'S'));
    BOOST_CHECK( bsecret2C.SetString(strSSecret2C, 'S'));
    // Set NuShares private key with NuBits unit, should fail
    BOOST_CHECK(!bsecret1.SetString (strSSecret1,  'B'));
    BOOST_CHECK(!bsecret2.SetString (strSSecret2,  'B'));
    BOOST_CHECK(!bsecret1C.SetString(strSSecret1C, 'B'));
    BOOST_CHECK(!bsecret2C.SetString(strSSecret2C, 'B'));

    // New encoding for NuBits
    BOOST_CHECK( bsecret1.SetString (strBSecret1,  'B'));
    BOOST_CHECK( bsecret2.SetString (strBSecret2,  'B'));
    BOOST_CHECK( bsecret1C.SetString(strBSecret1C, 'B'));
    BOOST_CHECK( bsecret2C.SetString(strBSecret2C, 'B'));
    // Set NuBits private key with NuShares unit, should fail
    BOOST_CHECK(!bsecret1.SetString (strBSecret1,  'S'));
    BOOST_CHECK(!bsecret2.SetString (strBSecret2,  'S'));
    BOOST_CHECK(!bsecret1C.SetString(strBSecret1C, 'S'));
    BOOST_CHECK(!bsecret2C.SetString(strBSecret2C, 'S'));

    // Old keys encoding for NuShares
    BOOST_CHECK( bsecret1.SetString (strDefaultSecret1,  'S'));
    BOOST_CHECK( bsecret2.SetString (strDefaultSecret2,  'S'));
    BOOST_CHECK( bsecret1C.SetString(strDefaultSecret1C, 'S'));
    BOOST_CHECK( bsecret2C.SetString(strDefaultSecret2C, 'S'));

    // Old keys encoding for NuBits
    BOOST_CHECK( bsecret1.SetString (strDefaultSecret1,  'B'));
    BOOST_CHECK( bsecret2.SetString (strDefaultSecret2,  'B'));
    BOOST_CHECK( bsecret1C.SetString(strDefaultSecret1C, 'B'));
    BOOST_CHECK( bsecret2C.SetString(strDefaultSecret2C, 'B'));

    // Check the CBitcoinSecret
    bool fCompressed;
    CSecret secret = bsecret1.GetSecret(fCompressed);
    BOOST_CHECK(CBitcoinSecret(secret, fCompressed, 'S').ToString() == strSSecret1);
    BOOST_CHECK(CBitcoinSecret(secret, fCompressed, 'B').ToString() == strBSecret1);
    secret = bsecret2.GetSecret(fCompressed);
    BOOST_CHECK(CBitcoinSecret(secret, fCompressed, 'S').ToString() == strSSecret2);
    BOOST_CHECK(CBitcoinSecret(secret, fCompressed, 'B').ToString() == strBSecret2);
    secret = bsecret1C.GetSecret(fCompressed);
    BOOST_CHECK(CBitcoinSecret(secret, fCompressed, 'S').ToString() == strSSecret1C);
    BOOST_CHECK(CBitcoinSecret(secret, fCompressed, 'B').ToString() == strBSecret1C);
    secret = bsecret2C.GetSecret(fCompressed);
    BOOST_CHECK(CBitcoinSecret(secret, fCompressed, 'S').ToString() == strSSecret2C);
    BOOST_CHECK(CBitcoinSecret(secret, fCompressed, 'B').ToString() == strBSecret2C);
}

BOOST_AUTO_TEST_SUITE_END()
