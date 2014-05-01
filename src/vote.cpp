#include <numeric>

#include "script.h"
#include "vote.h"
#include "main.h"

using namespace std;

CScript CVote::ToScript() const
{
    CScript voteScript;

    voteScript << OP_RETURN;
    voteScript << OP_1;

    CDataStream voteStream(SER_NETWORK, PROTOCOL_VERSION);
    voteStream << *this;

    vector<unsigned char> vchVote(voteStream.begin(), voteStream.end());
    voteScript << vchVote;

    return voteScript;
}

bool IsVote(const CScript& scriptPubKey)
{
    return (scriptPubKey.size() > 2 && scriptPubKey[0] == OP_RETURN && scriptPubKey[1] == OP_1);
}

bool ExtractVote(const CScript& scriptPubKey, CVote& voteRet)
{
    if (!IsVote(scriptPubKey))
        return false;

    CScript voteScript(scriptPubKey.begin() + 2, scriptPubKey.end());

    vector<vector<unsigned char> > stack;
    CTransaction fakeTx;
    EvalScript(stack, voteScript, fakeTx, 0, 0);

    if (stack.size() != 1)
        return false;

    CDataStream voteStream(stack[0], SER_NETWORK, PROTOCOL_VERSION);

    voteStream >> voteRet;

    return true;
}

bool ExtractVote(const CBlock& block, CVote& voteRet)
{
    if (!block.IsProofOfStake())
        return false;

    if (block.vtx.size() < 2)
        return error("invalid transaction count in proof of stake block");

    const CTransaction& tx = block.vtx[1];
    if (!tx.IsCoinStake())
        return error("CoinStake not found in proof of stake block");

    BOOST_FOREACH (const CTxOut& txo, tx.vout)
    {
        CVote vote;
        if (ExtractVote(txo.scriptPubKey, voteRet))
            return true;
    }

    return false;
}

CScript CParkRateVote::ToParkRateResultScript() const
{
    CScript script;

    script << OP_RETURN;
    script << OP_2;

    CDataStream stream(SER_NETWORK, PROTOCOL_VERSION);
    stream << *this;

    vector<unsigned char> vch(stream.begin(), stream.end());
    script << vch;

    return script;
}

bool IsParkRateResult(const CScript& scriptPubKey)
{
    return (scriptPubKey.size() > 2 && scriptPubKey[0] == OP_RETURN && scriptPubKey[1] == OP_2);
}

bool ExtractParkRateResult(const CScript& scriptPubKey, CParkRateVote& parkRateResultRet)
{
    if (!IsParkRateResult(scriptPubKey))
        return false;

    CScript script(scriptPubKey.begin() + 2, scriptPubKey.end());

    vector<vector<unsigned char> > stack;
    CTransaction fakeTx;
    EvalScript(stack, script, fakeTx, 0, 0);

    if (stack.size() != 1)
        return false;

    CDataStream stream(stack[0], SER_NETWORK, PROTOCOL_VERSION);

    stream >> parkRateResultRet;

    return true;
}

bool ExtractParkRateResults(const CBlock& block, vector<CParkRateVote>& vParkRateResultRet)
{
    vParkRateResultRet.clear();

    if (!block.IsProofOfStake())
        return false;

    if (block.vtx.size() < 2)
        return error("invalid transaction count in proof of stake block");

    const CTransaction& tx = block.vtx[1];
    if (!tx.IsCoinStake())
        return error("CoinStake not found in proof of stake block");

    BOOST_FOREACH (const CTxOut& txo, tx.vout)
    {
        CParkRateVote result;
        if (ExtractParkRateResult(txo.scriptPubKey, result))
            vParkRateResultRet.push_back(result);
    }

    return true;
}


typedef map<uint64, uint64> RateWeightMap;
typedef RateWeightMap::value_type RateWeight;

typedef map<unsigned char, RateWeightMap> DurationRateWeightMap;
typedef DurationRateWeightMap::value_type DurationRateWeight;

static uint64 AddRateWeight(const uint64& totalWeight, const RateWeight& rateWeight)
{
    return totalWeight + rateWeight.second;
}

bool CalculateParkRateResults(const std::vector<CVote>& vVote, std::vector<CParkRateVote> &results)
{
    results.clear();

    if (vVote.empty())
        return true;

    // Only one unit is supported for now
    vector<CParkRate> result;

    DurationRateWeightMap durationRateWeights;
    uint64 totalVoteWeight = 0;

    BOOST_FOREACH(const CVote& vote, vVote)
    {
        if (!vote.IsValid())
            return false;

        if (vote.nCoinAgeDestroyed == 0)
            return error("vote with 0 coin age destroyed");

        totalVoteWeight += vote.nCoinAgeDestroyed;

        BOOST_FOREACH(const CParkRateVote& parkRateVote, vote.vParkRateVote)
        {
            BOOST_FOREACH(const CParkRate& parkRate, parkRateVote.vParkRate)
            {
                RateWeightMap &rateWeights = durationRateWeights[parkRate.nDuration];
                rateWeights[parkRate.nRate] += vote.nCoinAgeDestroyed;
            }
        }
    }

    uint64 halfWeight = totalVoteWeight / 2;

    BOOST_FOREACH(const DurationRateWeight& durationRateWeight, durationRateWeights)
    {
        unsigned char nDuration = durationRateWeight.first;
        const RateWeightMap &rateWeights = durationRateWeight.second;

        uint64 totalWeight = accumulate(rateWeights.begin(), rateWeights.end(), (uint64)0, AddRateWeight);
        uint64 sum = totalWeight;
        uint64 median = 0;

        BOOST_FOREACH(const RateWeight& rateWeight, rateWeights)
        {
            if (sum <= halfWeight)
                break;

            sum -= rateWeight.second;
            median = rateWeight.first;
        }

        if (median != 0)
        {
            CParkRate parkRate;
            parkRate.nDuration = nDuration;
            parkRate.nRate = median;
            result.push_back(parkRate);
        }
    }

    CParkRateVote parkRateVote;
    parkRateVote.cUnit = 'B';
    parkRateVote.vParkRate = result;
    results.push_back(parkRateVote);

    return true;
}

bool CalculateParkRateResults(const CVote &vote, CBlockIndex *pindexprev, std::vector<CParkRateVote> &vParkRateResult)
{
    vector<CVote> vVote;
    vVote.reserve(PARK_RATE_VOTES);
    vVote.push_back(vote);

    CBlockIndex *pindex = pindexprev;
    for (int i=0; i<PARK_RATE_VOTES-1 && pindex; i++)
    {
        if (pindex->IsProofOfStake())
            vVote.push_back(pindex->vote);
        pindex = pindex->pprev;
    }

    return CalculateParkRateResults(vVote, vParkRateResult);
}

bool CVote::IsValid() const
{
    set<unsigned char> seenParkVoteUnits;
    BOOST_FOREACH(const CParkRateVote& parkRateVote, vParkRateVote)
    {
        if (parkRateVote.cUnit == 'S')
            return false;
        if (parkRateVote.cUnit != 'B')
            return false;
        if (seenParkVoteUnits.find(parkRateVote.cUnit) != seenParkVoteUnits.end())
            return false;
        seenParkVoteUnits.insert(parkRateVote.cUnit);

        set<unsigned char> seenDurations;
        BOOST_FOREACH(const CParkRate& parkRate, parkRateVote.vParkRate)
        {
            if (seenDurations.find(parkRate.nDuration) != seenDurations.end())
                return false;
            seenDurations.insert(parkRate.nDuration);
        }
    }
    return true;
}

bool ExtractVotes(const CBlock& block, CBlockIndex *pindexprev, unsigned int nCount, std::vector<CVote> &vVoteRet)
{
    CVote vote;
    if (!ExtractVote(block, vote))
        return error("ExtractVotes(): ExtractVote failed");

    if (!vote.IsValid())
        return error("ExtractVotes(): Invalid vote");

    if (!block.GetCoinAge(vote.nCoinAgeDestroyed))
        return error("ExtractVotes(): Unable to get block coin age");

    vVoteRet.reserve(nCount);
    vVoteRet.push_back(vote);

    CBlockIndex *pindex = pindexprev;
    for (int i=0; i<nCount-1 && pindex; i++)
    {
        if (pindex->IsProofOfStake())
            vVoteRet.push_back(pindex->vote);
        pindex = pindex->pprev;
    }
    return true;
}

bool CheckVote(const CBlock& block, CBlockIndex *pindexprev)
{
    CVote vote;
    if (!ExtractVote(block, vote))
        return error("CheckVote(): ExtractVote failed");

    if (!vote.IsValid())
        return error("CheckVote(): Invalid vote");

    if (!block.GetCoinAge(vote.nCoinAgeDestroyed))
        return error("CheckVote(): Unable to get block coin age");

    vector<CParkRateVote> vParkRateResult;
    if (!ExtractParkRateResults(block, vParkRateResult))
        return error("CheckVote(): ExtractParkRateResults failed");

    vector<CParkRateVote> vExpectedParkRateResult;
    if (!CalculateParkRateResults(vote, pindexprev, vExpectedParkRateResult))
        return error("CheckVote(): CalculateParkRateResults failed");

    if (vParkRateResult != vExpectedParkRateResult)
        return error("CheckVote(): Invalid park rate result");

    return true;
}

typedef map<CCustodianVote, uint64> CustodianVoteMap;

bool GenerateCurrencyCoinBases(const std::vector<CVote> vVote, std::set<CBitcoinAddress> setAlreadyElected, std::vector<CTransaction>& vCurrencyCoinBaseRet)
{
    vCurrencyCoinBaseRet.clear();

    if (vVote.empty())
        return true;

    CustodianVoteMap custodianVoteWeights;
    CustodianVoteMap custodianVoteCounts;
    uint64 totalVoteWeight = 0;

    BOOST_FOREACH(const CVote& vote, vVote)
    {
        if (!vote.IsValid())
            return false;

        if (vote.nCoinAgeDestroyed == 0)
            return error("vote with 0 coin age destroyed");

        totalVoteWeight += vote.nCoinAgeDestroyed;

        BOOST_FOREACH(const CCustodianVote& custodianVote, vote.vCustodianVote)
        {
            if (!setAlreadyElected.count(custodianVote.GetAddress()))
            {
                custodianVoteWeights[custodianVote] += vote.nCoinAgeDestroyed;
                custodianVoteCounts[custodianVote] += 1;
            }
        }
    }

    uint64 halfWeight = totalVoteWeight / 2;
    uint64 halfCount = vVote.size() / 2;

    BOOST_FOREACH(const CustodianVoteMap::value_type& value, custodianVoteWeights)
    {
        const CCustodianVote &custodianVote = value.first;
        uint64 weight = value.second;
        uint64 count = custodianVoteCounts[custodianVote];

        if (weight > halfWeight && count > halfCount)
        {
            CTransaction tx;
            tx.cUnit = 'B';

            CScript scriptPubKey;
            scriptPubKey.SetBitcoinAddress(custodianVote.GetAddress(), 'B');

            tx.vin.push_back(CTxIn());
            tx.vout.push_back(CTxOut(custodianVote.nAmount, scriptPubKey));

            vCurrencyCoinBaseRet.push_back(tx);
        }
    }

    return true;
}
