// Copyright (c) 2014-2015 The Nu developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "liquidityinfo.h"
#include "main.h"
#include "ui_interface.h"

#define LIQUIDITY_INFO_MAX_HEIGHT 260000 // Liquidity info from custodians who were elected more than LIQUIDITY_INFO_MAX_HEIGHT blocks ago are ignored

using namespace std;

map<const CBitcoinAddress, CLiquidityInfo> mapLiquidityInfo;
CCriticalSection cs_mapLiquidityInfo;
int64 nLastLiquidityUpdate = 0;

bool CLiquidityInfo::ProcessLiquidityInfo()
{
    if (!IsValidCurrency(cCustodianUnit))
        return false;

    CBitcoinAddress address(GetCustodianAddress());

    {
        LOCK2(cs_main, cs_mapElectedCustodian);

        map<const CBitcoinAddress, CBlockIndex*>::iterator it;
        it = mapElectedCustodian.find(address);
        if (it == mapElectedCustodian.end())
            return false;

        CBlockIndex *pindex = it->second;
        if (!pindexBest || pindexBest->nHeight - pindex->nHeight > LIQUIDITY_INFO_MAX_HEIGHT)
            return false;
    }

    if (!CheckSignature())
        return false;

    {
        LOCK(cs_mapLiquidityInfo);

        map<const CBitcoinAddress, CLiquidityInfo>::iterator it;
        it = mapLiquidityInfo.find(address);
        if (it != mapLiquidityInfo.end())
        {
            if (it->second.nTime >= nTime)
                return false;
        }

        mapLiquidityInfo[address] = *this;
        nLastLiquidityUpdate = GetAdjustedTime();
    }

    printf("accepted liquidity info from %s\n", address.ToString().c_str());
    MainFrameRepaint();
    return true;
}

void RemoveExpiredLiquidityInfo(int nCurrentHeight)
{
    bool fAnyRemoved = false;
    {
        LOCK(cs_mapLiquidityInfo);
        LOCK2(cs_main, cs_mapElectedCustodian);

        map<const CBitcoinAddress, CLiquidityInfo>::iterator it;
        it = mapLiquidityInfo.begin();
        while (it != mapLiquidityInfo.end())
        {
            const CBitcoinAddress& address = it->first;
            CBlockIndex *pindex = mapElectedCustodian[address];

            if (nCurrentHeight - pindex->nHeight > LIQUIDITY_INFO_MAX_HEIGHT)
            {
                mapLiquidityInfo.erase(it++);
                fAnyRemoved = true;
                nLastLiquidityUpdate = GetAdjustedTime();
            }
            else
                it++;
        }
    }

    if (fAnyRemoved)
        MainFrameRepaint();
}
