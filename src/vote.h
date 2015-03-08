#ifndef VOTE_H
#define VOTE_H

#include "serialize.h"
#include "base58.h"

class CBlock;
class CBlockIndex;

static const int64 COIN_PARK_RATE = 100000 * COIN; // Park rate internal encoding precision. The minimum possible rate is (1.0 / COIN_PARK_RATE) coins per parked coin
static const int64 MAX_PARK_RATE = 1000000 * COIN_PARK_RATE;
static const unsigned char MAX_COMPACT_DURATION = 30; // about 2000 years

class CCustodianVote
{
public:
    unsigned char cUnit;
    bool fScript;
    uint160 hashAddress;
    int64 nAmount;

    CCustodianVote() :
        cUnit('?'),
        fScript(false),
        hashAddress(0),
        nAmount(0)
    {
    }

    bool IsValid() const;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(cUnit);
        if (nVersion >= 20200) // version 0.2.2
            READWRITE(fScript);
        else if (fRead)
            const_cast<CCustodianVote*>(this)->fScript = false;
        READWRITE(hashAddress);
        READWRITE(nAmount);
    )

    inline bool operator==(const CCustodianVote& other) const
    {
        return (cUnit == other.cUnit &&
                fScript == other.fScript &&
                hashAddress == other.hashAddress &&
                nAmount == other.nAmount);
    }
    inline bool operator!=(const CCustodianVote& other) const
    {
        return !(*this == other);
    }

    class CDestinationVisitor : public boost::static_visitor<bool>
    {
        private:
            CCustodianVote *custodianVote;
        public:
            CDestinationVisitor(CCustodianVote *custodianVote) : custodianVote(custodianVote) { }

            bool operator()(const CNoDestination &dest) const {
                custodianVote->fScript = false;
                custodianVote->hashAddress = 0;
                return false;
            }

            bool operator()(const CKeyID &keyID) const {
                custodianVote->fScript = false;
                custodianVote->hashAddress = keyID;
                return true;
            }

            bool operator()(const CScriptID &scriptID) const {
                custodianVote->fScript = true;
                custodianVote->hashAddress = scriptID;
                return true;
            }
    };

    void SetAddress(const CBitcoinAddress& address)
    {
        cUnit = address.GetUnit();
        CTxDestination destination = address.Get();
        boost::apply_visitor(CDestinationVisitor(this), destination);
    }

    CBitcoinAddress GetAddress() const
    {
        CBitcoinAddress address;
        if (fScript)
            address.Set(CScriptID(hashAddress), cUnit);
        else
            address.Set(CKeyID(hashAddress), cUnit);
        return address;
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
        if (fScript < other.fScript)
            return true;
        if (fScript > other.fScript)
            return false;
        if (hashAddress < other.hashAddress)
            return true;
        return false;
    }
};

inline bool CompactDurationRange(unsigned char nCompactDuration)
{
    return (nCompactDuration < MAX_COMPACT_DURATION);
}

inline bool ParkDurationRange(int64 nDuration)
{
    return (nDuration >= 1 && nDuration <= 1000000000); // about 1900 years
}

inline bool ParkRateRange(int64 nRate)
{
    return (nRate >= 0 && nRate <= MAX_PARK_RATE);
}

class CParkRate
{
public:
    unsigned char nCompactDuration;
    int64 nRate;

    CParkRate() :
        nCompactDuration(0),
        nRate(0)
    {
    }

    CParkRate(unsigned char nCompactDuration, int64 nRate) :
        nCompactDuration(nCompactDuration),
        nRate(nRate)
    {
    }

    bool IsValid() const;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(nCompactDuration);
        READWRITE(nRate);
    )

    int64 GetDuration() const
    {
        if (!CompactDurationRange(nCompactDuration))
            throw std::runtime_error("Park rate compact duration out of range");
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

    bool IsValid() const;

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
    std::vector<uint160> vMotion;

    int64 nCoinAgeDestroyed;

    CVote() :
        nVersion(PROTOCOL_VERSION),
        nCoinAgeDestroyed(0)
    {
    }

    CVote(const CVote &original, int nVersion) :
        nVersion(nVersion),
        vCustodianVote(original.vCustodianVote),
        vParkRateVote(original.vParkRateVote),
        vMotion(original.vMotion),
        nCoinAgeDestroyed(original.nCoinAgeDestroyed)
    {
    }

    void SetNull()
    {
        nVersion = 0;
        vCustodianVote.clear();
        vParkRateVote.clear();
        vMotion.clear();
        nCoinAgeDestroyed = 0;
    }

    template<typename Stream>
    unsigned int ReadWriteSingleMotion(Stream& s, int nType, int nVersion, CSerActionGetSerializeSize ser_action) const
    {
        uint160 hashMotion;
        return ::SerReadWrite(s, hashMotion, nType, nVersion, ser_action);
    }

    template<typename Stream>
    unsigned int ReadWriteSingleMotion(Stream& s, int nType, int nVersion, CSerActionSerialize ser_action) const
    {
        uint160 hashMotion;
        switch (vMotion.size())
        {
            case 0:
                hashMotion = 0;
                break;
            case 1:
                hashMotion = vMotion[0];
                break;
            default:
                printf("Warning: serializing multiple motion votes in a vote structure not supporting it. Only the first motion is serialized.\n");
                hashMotion = vMotion[0];
                break;
        }
        return ::SerReadWrite(s, hashMotion, nType, nVersion, ser_action);
    }

    template<typename Stream>
    unsigned int ReadWriteSingleMotion(Stream& s, int nType, int nVersion, CSerActionUnserialize ser_action)
    {
        uint160 hashMotion;
        unsigned int result = ::SerReadWrite(s, hashMotion, nType, nVersion, ser_action);
        vMotion.clear();
        if (hashMotion != 0)
            vMotion.push_back(hashMotion);
        return result;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(vCustodianVote);
        READWRITE(vParkRateVote);
        if (nVersion >= 50000)
            READWRITE(vMotion);
        else
            ReadWriteSingleMotion(s, nType, nVersion, ser_action);
    )

    CScript ToScript(int nVersion) const;
    CScript ToScript() const
    {
        return ToScript(nVersion);
    }

    bool IsValid() const;

    void Upgrade();

    inline bool operator==(const CVote& other) const
    {
        return (nVersion == other.nVersion &&
                vCustodianVote == other.vCustodianVote &&
                vParkRateVote == other.vParkRateVote &&
                vMotion == other.vMotion);
    }
    inline bool operator!=(const CVote& other) const
    {
        return !(*this == other);
    }
};

bool IsVote(const CScript& scriptPubKey);
bool ExtractVote(const CScript& scriptPubKey, CVote& voteRet);
bool ExtractVote(const CBlock& block, CVote& voteRet);
bool ExtractVotes(const CBlock &block, CBlockIndex *pindexprev, unsigned int nCount, std::vector<CVote> &vVoteResult);

bool IsParkRateResult(const CScript& scriptPubKey);
bool ExtractParkRateResult(const CScript& scriptPubKey, CParkRateVote& parkRateResultRet);
bool ExtractParkRateResults(const CBlock& block, std::vector<CParkRateVote>& vParkRateResultRet);

bool CalculateParkRateResults(const std::vector<CVote>& vVote, const std::map<unsigned char, std::vector<const CParkRateVote*> >& mapPreviousVotes, std::vector<CParkRateVote>& results);
bool CalculateParkRateResults(const CVote &vote, CBlockIndex *pindexprev, std::vector<CParkRateVote>& vParkRateResult);
int64 GetPremium(int64 nValue, int64 nDuration, unsigned char cUnit, const std::vector<CParkRateVote>& vParkRateResult);

bool GenerateCurrencyCoinBases(const std::vector<CVote> vVote, std::map<CBitcoinAddress, CBlockIndex*> mapAlreadyElected, std::vector<CTransaction>& vCurrencyCoinBaseRet);

#endif
