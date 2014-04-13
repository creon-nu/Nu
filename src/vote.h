#ifndef VOTE_H
#define VOTE_H

#include "serialize.h"

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
};

class CParkRateVote
{
public:
    unsigned char cUnit;
    std::vector<CParkRate> vParkRate;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(cUnit);
        READWRITE(vParkRate);
    )
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

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(vCustodianVote);
        READWRITE(vParkRateVote);
        READWRITE(hashMotion);
    )

    CScript ToScript() const;
};

bool IsVote(const CScript& scriptPubKey);
bool ExtractVote(const CScript& scriptPubKey, CVote& voteRet);

#endif
