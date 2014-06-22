
#include "liquidityinfo.h"
#include "main.h"
#include "ui_interface.h"

using namespace std;

map<const CBitcoinAddress, CLiquidityInfo> mapLiquidityInfo;
CCriticalSection cs_mapLiquidityInfo;

bool CLiquidityInfo::ProcessLiquidityInfo()
{
    if (!ValidUnit(cCustodianUnit) || cCustodianUnit == 'S')
        return false;

    CBitcoinAddress address(GetCustodianAddress());

    {
        LOCK(cs_mapElectedCustodian);

        if (!mapElectedCustodian.count(address))
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
            if (it->second.nTime > nTime)
                return false;
        }

        mapLiquidityInfo[address] = *this;
    }

    printf("accepted liquidity info from %s\n", address.ToString().c_str());
    MainFrameRepaint();
    return true;
}
