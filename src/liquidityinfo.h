#ifndef LIQUIDITYINFO_H
#define LIQUIDITYINFO_H

#include "serialize.h"
#include "uint256.h"
#include "base58.h"
#include "net.h"
#include "util.h"

class CUnsignedLiquidityInfo
{
public:
    int nVersion;
    unsigned char cUnit;
    int64 nTime;
    int64 nBuyAmount;
    int64 nSellAmount;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(nTime);
        READWRITE(cUnit);
        READWRITE(nBuyAmount);
        READWRITE(nSellAmount);
    )

    void SetNull()
    {
        nVersion = 1;
        cUnit = '?';
        nTime = 0;
        nBuyAmount = 0;
        nSellAmount = 0;
    }

    virtual std::string GetCustodianString() const
    {
        return "unknown";
    }

    std::string ToString() const
    {
        return strprintf(
                "CLiquidityInfo(\n"
                "    nVersion     = %d\n"
                "    cUnit        = %c\n"
                "    custodian    = %s\n"
                "    nTime        = %"PRI64d"\n"
                "    nBuyAmount   = %s\n"
                "    nSellAmount  = %s\n"
                ")\n",
            nVersion,
            cUnit,
            GetCustodianString().c_str(),
            nTime,
            FormatMoney(nBuyAmount).c_str(),
            FormatMoney(nSellAmount).c_str());
    }

    void print() const
    {
        printf("%s", ToString().c_str());
    }
};

class CLiquidityInfo : public CUnsignedLiquidityInfo
{
public:
    unsigned char cCustodianUnit;
    std::vector<unsigned char> vchCustodianPubKey;
    std::vector<unsigned char> vchMsg;
    std::vector<unsigned char> vchSig;

    CLiquidityInfo()
    {
        SetNull();
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(cCustodianUnit);
        READWRITE(vchCustodianPubKey);
        READWRITE(vchMsg);
        READWRITE(vchSig);
    )

    void SetNull()
    {
        CUnsignedLiquidityInfo::SetNull();
        cCustodianUnit = '?';
        vchCustodianPubKey.clear();
        vchMsg.clear();
        vchSig.clear();
    }

    bool IsNull() const
    {
        return (cCustodianUnit == '?');
    }

    CBitcoinAddress GetCustodianAddress() const
    {
        return CBitcoinAddress(Hash160(vchCustodianPubKey), cCustodianUnit);
    }

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }

    bool RelayTo(CNode* pnode) const
    {
        // returns true if wasn't already contained in the set
        if (pnode->setKnown.insert(GetHash()).second)
        {
            pnode->PushMessage("liquidity", *this);
            return true;
        }
        return false;
    }

    bool CheckSignature()
    {
        CKey key;
        if (!key.SetPubKey(vchCustodianPubKey))
            return error("CLiquidityInfo::CheckSignature() : SetPubKey failed");
        if (!key.Verify(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
            return error("CLiquidityInfo::CheckSignature() : verify signature failed");

        // Now unserialize the data
        CDataStream sMsg(vchMsg, SER_NETWORK, PROTOCOL_VERSION);
        sMsg >> *(CUnsignedLiquidityInfo*)this;
        return true;
    }

    bool ProcessLiquidityInfo();

    virtual std::string GetCustodianString() const
    {
        return GetCustodianAddress().ToString();
    }
};

extern std::map<const CBitcoinAddress, CLiquidityInfo> mapLiquidityInfo;
extern CCriticalSection cs_mapLiquidityInfo;

#endif
