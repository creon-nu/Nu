#include <boost/test/unit_test.hpp>

#include "main.h"
#include "vote.h"

using namespace std;

BOOST_AUTO_TEST_SUITE(vote_tests)

BOOST_AUTO_TEST_CASE(reload_vote_from_script_tests)
{
    CVote vote;

    CCustodianVote custodianVote;
    custodianVote.cUnit = 'B';
    custodianVote.hashAddress = uint160(123465);
    custodianVote.nAmount = 100 * COIN;
    vote.vCustodianVote.push_back(custodianVote);

    CCustodianVote custodianVote2;
    custodianVote2.cUnit = 'B';
    custodianVote2.hashAddress = uint160(555555555);
    custodianVote2.nAmount = 5.5 * COIN;
    vote.vCustodianVote.push_back(custodianVote2);

    CParkRateVote parkRateVote;
    parkRateVote.cUnit = 'B';
    parkRateVote.vParkRate.push_back(CParkRate(13, 3));
    parkRateVote.vParkRate.push_back(CParkRate(14, 6));
    parkRateVote.vParkRate.push_back(CParkRate(15, 13));
    vote.vParkRateVote.push_back(parkRateVote);

    vote.hashMotion = uint160(123456);

    CScript script = vote.ToScript();

    CVote voteResult;
    BOOST_CHECK(ExtractVote(script, voteResult));

#undef CHECK_VOTE_EQUAL
#define CHECK_VOTE_EQUAL(value) BOOST_CHECK(voteResult.value == vote.value);
    CHECK_VOTE_EQUAL(vCustodianVote.size());
    for (int i=0; i<vote.vCustodianVote.size(); i++)
    {
        CHECK_VOTE_EQUAL(vCustodianVote[i].cUnit);
        CHECK_VOTE_EQUAL(vCustodianVote[i].hashAddress);
        CHECK_VOTE_EQUAL(vCustodianVote[i].nAmount);
    }

    CHECK_VOTE_EQUAL(vParkRateVote.size());
    for (int i=0; i<vote.vParkRateVote.size(); i++)
    {
        CHECK_VOTE_EQUAL(vParkRateVote[i].cUnit);
        CHECK_VOTE_EQUAL(vParkRateVote[i].vParkRate.size());
        for (int j=0; j<vote.vParkRateVote[i].vParkRate.size(); j++)
        {
            CHECK_VOTE_EQUAL(vParkRateVote[i].vParkRate[j].nDuration);
            CHECK_VOTE_EQUAL(vParkRateVote[i].vParkRate[j].nRate);
        }
    }

    CHECK_VOTE_EQUAL(hashMotion);
#undef CHECK_VOTE_EQUAL
}

BOOST_AUTO_TEST_CASE(reload_park_rates_from_script_tests)
{
    CParkRateVote parkRateVote;
    parkRateVote.cUnit = 'B';
    parkRateVote.vParkRate.push_back(CParkRate(13, 3));
    parkRateVote.vParkRate.push_back(CParkRate(14, 6));
    parkRateVote.vParkRate.push_back(CParkRate(15, 13));

    CScript script = parkRateVote.ToParkRateResultScript();
    BOOST_CHECK(IsParkRateResult(script));

    CParkRateVote parkRateVoteResult;
    BOOST_CHECK(ExtractParkRateResult(script, parkRateVoteResult));

#undef CHECK_PARK_RATE_EQUAL
#define CHECK_PARK_RATE_EQUAL(value) BOOST_CHECK(parkRateVoteResult.value == parkRateVote.value);
    CHECK_PARK_RATE_EQUAL(cUnit);
    CHECK_PARK_RATE_EQUAL(vParkRate.size());
    for (int i=0; i<parkRateVote.vParkRate.size(); i++)
    {
        CHECK_PARK_RATE_EQUAL(vParkRate[i].nDuration);
        CHECK_PARK_RATE_EQUAL(vParkRate[i].nRate);
    }
#undef CHECK_PARK_RATE_EQUAL
}

BOOST_AUTO_TEST_SUITE_END()
