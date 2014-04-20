#ifndef VOTE_H
#define VOTE_H

#include "serialize.h"

class CBlock;
class CBlockIndex;

class CCustodianVote
{
public:
    unsigned char cUnit;
    uint160 hashAddress;
    uint64 nAmount;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(cUnit);
        READWRITE(hashAddress);
        READWRITE(nAmount);
    )
};

class CParkRate
{
public:
    unsigned char nDuration;
    unsigned int nRate;

    CParkRate()
    {
    }

    CParkRate(unsigned char nDuration, unsigned int nRate) :
        nDuration(nDuration),
        nRate(nRate)
    {
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(nDuration);
        READWRITE(nRate);
    )

    friend bool operator==(const CParkRate& a, const CParkRate& b)
    {
        return (a.nDuration == b.nDuration &&
                a.nRate     == b.nRate);
    }
};

class CParkRateVote
{
public:
    unsigned char cUnit;
    std::vector<CParkRate> vParkRate;

    void SetNull()
    {
        cUnit = 0;
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

bool IsParkRateResult(const CScript& scriptPubKey);
bool ExtractParkRateResult(const CScript& scriptPubKey, CParkRateVote& parkRateResultRet);
bool ExtractParkRateResults(const CBlock& block, std::vector<CParkRateVote>& vParkRateResultRet);

bool CalculateParkRateResults(const std::vector<CVote>& vVote, std::vector<CParkRateVote> &results);
bool CalculateParkRateResults(const CVote &vote, CBlockIndex *pindexprev, std::vector<CParkRateVote> &vParkRateResult);

bool CheckVote(const CBlock& block, CBlockIndex *pindexprev);

#endif
