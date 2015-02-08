#include <numeric>

#include "script.h"
#include "vote.h"
#include "main.h"

using namespace std;

CScript CVote::ToScript(int nVersion) const
{
    CScript voteScript;

    voteScript << OP_RETURN;
    voteScript << OP_1;

    CDataStream voteStream(SER_NETWORK, PROTOCOL_VERSION);
    if (nVersion == this->nVersion)
        voteStream << *this;
    else
        voteStream << CVote(*this, nVersion);

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

string CParkRateVote::ToString() const
{
    std::stringstream stream;
    stream << "ParkRateVote unit=" << cUnit;
    BOOST_FOREACH(const CParkRate& parkRate, vParkRate)
        stream << " " << (int)parkRate.nCompactDuration << ":" << parkRate.nRate;
    return stream.str();
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

bool CalculateParkRateResults(const std::vector<CVote>& vVote, const std::map<unsigned char, std::vector<const CParkRateVote*> >& mapPreviousRates, std::vector<CParkRateVote>& results)
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
                RateWeightMap &rateWeights = durationRateWeights[parkRate.nCompactDuration];
                rateWeights[parkRate.nRate] += vote.nCoinAgeDestroyed;
            }
        }
    }

    uint64 halfWeight = totalVoteWeight / 2;

    BOOST_FOREACH(const DurationRateWeight& durationRateWeight, durationRateWeights)
    {
        unsigned char nCompactDuration = durationRateWeight.first;
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
            parkRate.nCompactDuration = nCompactDuration;
            parkRate.nRate = median;
            result.push_back(parkRate);
        }
    }

    map<unsigned char, unsigned int> minPreviousRates;

    if (mapPreviousRates.count('B'))
    {
        const std::vector<const CParkRateVote*>& previousUnitRates = mapPreviousRates.find('B')->second;
        BOOST_FOREACH(const CParkRateVote* parkRateVote, previousUnitRates)
        {
            BOOST_FOREACH(const CParkRate& parkRate, parkRateVote->vParkRate)
            {
                map<unsigned char, unsigned int>::iterator it = minPreviousRates.find(parkRate.nCompactDuration);
                if (it == minPreviousRates.end())
                    minPreviousRates[parkRate.nCompactDuration] = parkRate.nRate;
                else
                    if (parkRate.nRate < it->second)
                        it->second = parkRate.nRate;
            }
        }
    }

    BOOST_FOREACH(CParkRate& parkRate, result)
    {
        uint64 previousMin = minPreviousRates[parkRate.nCompactDuration];
        uint64 duration = parkRate.GetDuration();
        uint64 maxIncrease = (uint64)100 * COIN_PARK_RATE / COIN * duration * STAKE_TARGET_SPACING / 31557600;

        if (parkRate.nRate > (int64)previousMin + maxIncrease)
            parkRate.nRate = previousMin + maxIncrease;
    }

    CParkRateVote parkRateVote;
    parkRateVote.cUnit = 'B';
    parkRateVote.vParkRate = result;
    results.push_back(parkRateVote);

    return true;
}

bool CalculateParkRateResults(const CVote &vote, CBlockIndex *pindexprev, std::vector<CParkRateVote>& vParkRateResult)
{
    vector<CVote> vVote;
    vVote.reserve(PARK_RATE_VOTES);
    vVote.push_back(vote);

    map<unsigned char, vector<const CParkRateVote*> > mapPreviousRates;
    BOOST_FOREACH(unsigned char cUnit, sAvailableUnits)
    {
        if (cUnit != 'S')
            mapPreviousRates[cUnit].reserve(PARK_RATE_PREVIOUS_VOTES);
    }

    CBlockIndex *pindex = pindexprev;
    for (int i=0; i<PARK_RATE_VOTES-1 && pindex; i++)
    {
        if (pindex->IsProofOfStake())
            vVote.push_back(pindex->vote);

        BOOST_FOREACH(const CParkRateVote& previousRate, pindex->vParkRateResult)
        {
            vector<const CParkRateVote*>& unitPreviousRates = mapPreviousRates[previousRate.cUnit];
            if (unitPreviousRates.size() < PARK_RATE_PREVIOUS_VOTES)
                mapPreviousRates[previousRate.cUnit].push_back(&previousRate);
        }

        pindex = pindex->pprev;
    }

    return CalculateParkRateResults(vVote, mapPreviousRates, vParkRateResult);
}

bool CVote::IsValid() const
{
    set<unsigned char> seenParkVoteUnits;
    BOOST_FOREACH(const CParkRateVote& parkRateVote, vParkRateVote)
    {
        if (!IsValidCurrency(parkRateVote.cUnit))
            return false;
        if (seenParkVoteUnits.find(parkRateVote.cUnit) != seenParkVoteUnits.end())
            return false;
        seenParkVoteUnits.insert(parkRateVote.cUnit);

        set<unsigned char> seenCompactDurations;
        BOOST_FOREACH(const CParkRate& parkRate, parkRateVote.vParkRate)
        {
            if (seenCompactDurations.find(parkRate.nCompactDuration) != seenCompactDurations.end())
                return false;
            seenCompactDurations.insert(parkRate.nCompactDuration);
        }
    }

    set<CCustodianVote> seenCustodianVotes;
    BOOST_FOREACH(const CCustodianVote& custodianVote, vCustodianVote)
    {
        if (!IsValidCurrency(custodianVote.cUnit))

        if (seenCustodianVotes.count(custodianVote))
            return false;
        seenCustodianVotes.insert(custodianVote);
    }
    return true;
}

void CVote::Upgrade()
{
    if (nVersion < 50000)
        nVersion = 50000;
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

    if (!block.GetCoinStakeAge(vote.nCoinAgeDestroyed))
        return error("CheckVote(): Unable to get coin stake coin age");

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

class CCustodianVoteCounter
{
public:
    uint64 nWeight;
    int nCount;
};

typedef map<CCustodianVote, CCustodianVoteCounter> CustodianVoteCounterMap;
typedef map<CBitcoinAddress, uint64> GrantedAmountMap;
typedef map<unsigned char, GrantedAmountMap> GrantedAmountPerUnitMap;

bool GenerateCurrencyCoinBases(const std::vector<CVote> vVote, std::map<CBitcoinAddress, CBlockIndex*> mapAlreadyElected, std::vector<CTransaction>& vCurrencyCoinBaseRet)
{
    vCurrencyCoinBaseRet.clear();

    if (vVote.empty())
        return true;

    CustodianVoteCounterMap mapCustodianVoteCounter;
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
            if (!mapAlreadyElected.count(custodianVote.GetAddress()))
            {
                CCustodianVoteCounter& counter = mapCustodianVoteCounter[custodianVote];
                counter.nWeight += vote.nCoinAgeDestroyed;
                counter.nCount++;
            }
        }
    }

    uint64 halfWeight = totalVoteWeight / 2;
    uint64 halfCount = vVote.size() / 2;

    map<CBitcoinAddress, uint64> mapGrantedAddressWeight;
    GrantedAmountPerUnitMap mapGrantedCustodians;

    BOOST_FOREACH(const CustodianVoteCounterMap::value_type& value, mapCustodianVoteCounter)
    {
        const CCustodianVote &custodianVote = value.first;
        const CCustodianVoteCounter& counter = value.second;

        if (counter.nWeight > halfWeight && counter.nCount > halfCount)
        {
            CBitcoinAddress address = custodianVote.GetAddress();
            if (counter.nWeight > mapGrantedAddressWeight[address])
            {
                mapGrantedAddressWeight[address] = counter.nWeight;
                mapGrantedCustodians[custodianVote.cUnit][address] = custodianVote.nAmount;
            }
        }
    }

    BOOST_FOREACH(const GrantedAmountPerUnitMap::value_type& grantedAmountPerUnit, mapGrantedCustodians)
    {
        unsigned char cUnit = grantedAmountPerUnit.first;
        const GrantedAmountMap& mapGrantedAmount = grantedAmountPerUnit.second;

        CTransaction tx;
        tx.cUnit = cUnit;
        tx.vin.push_back(CTxIn());

        BOOST_FOREACH(const GrantedAmountMap::value_type& grantedAmount, mapGrantedAmount)
        {
            const CBitcoinAddress& address = grantedAmount.first;
            uint64 amount = grantedAmount.second;

            CScript scriptPubKey;
            scriptPubKey.SetDestination(address.Get());

            tx.vout.push_back(CTxOut(amount, scriptPubKey));
        }

        vCurrencyCoinBaseRet.push_back(tx);
    }

    return true;
}

uint64 GetPremium(uint64 nValue, uint64 nDuration, unsigned char cUnit, const std::vector<CParkRateVote>& vParkRateResult)
{
    BOOST_FOREACH(const CParkRateVote& parkRateVote, vParkRateResult)
    {
        if (parkRateVote.cUnit != cUnit)
            continue;

        vector<CParkRate> vSortedParkRate = parkRateVote.vParkRate;
        sort(vSortedParkRate.begin(), vSortedParkRate.end());

        for (unsigned int i = 0; i < vSortedParkRate.size(); i++)
        {
            const CParkRate& parkRate = vSortedParkRate[i];

            if (nDuration == parkRate.GetDuration())
                return nValue * parkRate.nRate / COIN_PARK_RATE;

            if (nDuration < parkRate.GetDuration())
            {
                if (i == 0)
                    return 0;

                const CParkRate& prevParkRate = vSortedParkRate[i-1];

                CBigNum bnRate(prevParkRate.nRate);
                CBigNum bnInterpolatedRate(nDuration);
                bnInterpolatedRate -= prevParkRate.GetDuration();
                bnInterpolatedRate *= (int64)parkRate.nRate - (int64)prevParkRate.nRate;
                bnInterpolatedRate /= (int64)parkRate.GetDuration() - (int64)prevParkRate.GetDuration();
                bnRate += bnInterpolatedRate;
                uint64 nRate = bnRate.getuint64();

                return nValue * nRate / COIN_PARK_RATE;
            }
        }
    }
    return 0;
}
