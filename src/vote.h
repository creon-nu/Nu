#ifndef VOTE_H
#define VOTE_H

#include "serialize.h"
#include "base58.h"

class CBlock;
class CBlockIndex;

class CCustodianVote
{
public:
    unsigned char cUnit;
    uint160 hashAddress;
    uint64 nAmount;

    CCustodianVote() :
        cUnit('?'),
        hashAddress(0),
        nAmount(0)
    {
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(cUnit);
        READWRITE(hashAddress);
        READWRITE(nAmount);
    )

    CBitcoinAddress GetAddress() const
    {
        return CBitcoinAddress(hashAddress, cUnit);
    }

    bool operator< (const CCustodianVote& other) const
    {
        if (cUnit < other.cUnit)
            return true;
        if (cUnit > other.cUnit)
            return false;
        if (nAmount < other.nAmount)
            return true;
        if (nAmount > other.nAmount)
            return false;
        if (hashAddress < other.hashAddress)
            return true;
        return false;
    }
};

class CParkRate
{
public:
    unsigned char nCompactDuration;
    unsigned int nRate;

    CParkRate() :
        nCompactDuration(0),
        nRate(0)
    {
    }

    CParkRate(unsigned char nCompactDuration, unsigned int nRate) :
        nCompactDuration(nCompactDuration),
        nRate(nRate)
    {
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(nCompactDuration);
        READWRITE(nRate);
    )

    uint64 GetDuration() const
    {
        return 1 << nCompactDuration;
    }

    friend bool operator==(const CParkRate& a, const CParkRate& b)
    {
        return (a.nCompactDuration == b.nCompactDuration &&
                a.nRate == b.nRate);
    }

    bool operator<(const CParkRate& other) const
    {
        if (nCompactDuration < other.nCompactDuration)
            return true;
        if (nCompactDuration > other.nCompactDuration)
            return false;
        return nRate < other.nRate;
    }
};

class CParkRateVote
{
public:
    unsigned char cUnit;
    std::vector<CParkRate> vParkRate;

    CParkRateVote() :
        cUnit('?')
    {
    }

    void SetNull()
    {
        cUnit = '?';
        vParkRate.clear();
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(cUnit);
        READWRITE(vParkRate);
    )

    CScript ToParkRateResultScript() const;

    friend bool operator==(const CParkRateVote& a, const CParkRateVote& b)
    {
        return (a.cUnit     == b.cUnit &&
                a.vParkRate == b.vParkRate);
    }

    std::string ToString() const;
};

class CVote
{
public:
    int nVersion;
    std::vector<CCustodianVote> vCustodianVote;
    std::vector<CParkRateVote> vParkRateVote;
    uint160 hashMotion;

    CVote() :
        nVersion(1),
        hashMotion()
    {
    }

    void SetNull()
    {
        nVersion = 0;
        vCustodianVote.clear();
        vParkRateVote.clear();
        hashMotion = 0;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(vCustodianVote);
        READWRITE(vParkRateVote);
        READWRITE(hashMotion);
    )

    CScript ToScript() const;

    uint64 nCoinAgeDestroyed;

    bool IsValid() const;
};

bool IsVote(const CScript& scriptPubKey);
bool ExtractVote(const CScript& scriptPubKey, CVote& voteRet);
bool ExtractVote(const CBlock& block, CVote& voteRet);
bool ExtractVotes(const CBlock &block, CBlockIndex *pindexprev, unsigned int nCount, std::vector<CVote> &vVoteResult);

bool IsParkRateResult(const CScript& scriptPubKey);
bool ExtractParkRateResult(const CScript& scriptPubKey, CParkRateVote& parkRateResultRet);
bool ExtractParkRateResults(const CBlock& block, std::vector<CParkRateVote>& vParkRateResultRet);

bool CalculateParkRateResults(const std::vector<CVote>& vVote, std::vector<CParkRateVote>& results);
bool CalculateParkRateResults(const CVote &vote, CBlockIndex *pindexprev, std::vector<CParkRateVote>& vParkRateResult);
uint64 GetPremium(uint64 nValue, uint64 nDuration, unsigned char cUnit, const std::vector<CParkRateVote>& vParkRateResult);

bool CheckVote(const CBlock& block, CBlockIndex *pindexprev);

bool GenerateCurrencyCoinBases(const std::vector<CVote> vVote, std::map<CBitcoinAddress, CBlockIndex*> mapAlreadyElected, std::vector<CTransaction>& vCurrencyCoinBaseRet);

#endif
