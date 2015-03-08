#include <curl/curl.h>
#include <cstdlib>
#include <cstring>
#include "datafeed.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"
#include "vote.h"
#include "main.h"
#include "bitcoinrpc.h"
#include "wallet.h"

using namespace std;
using namespace json_spirit;

string strDataFeedError = "";

class DataFeedRequest
{
private:
    CURL *curl;
    string result;
    string error;
    string url;

    size_t ReceiveCallback(char *contents, size_t size, size_t nmemb)
    {
        size_t written = 0;

        try
        {
            size_t realsize = size * nmemb;

            int nMaxSize = GetArg("maxdatafeedsize", 10 * 1024);
            if (result.size() + realsize > nMaxSize)
                throw runtime_error((boost::format("Data feed size exceeds limit (%1% bytes)") % nMaxSize).str());

            result.append(contents, realsize);
            written = realsize;
        }
        catch (exception &e)
        {
            error = string(e.what());
        }

        return written;
    }

    static size_t WriteMemoryCallback(char *contents, size_t size, size_t nmemb, void *userp)
    {
        DataFeedRequest *request = (DataFeedRequest*)userp;
        return request->ReceiveCallback(contents, size, nmemb);
    }

public:
    DataFeedRequest(const string& sURL)
        : url(sURL)
    {
        curl = curl_easy_init();
        if (curl == NULL)
            throw runtime_error("Failed to initialize curl");

        curl_easy_setopt(curl, CURLOPT_URL, sURL.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

        if (fUseProxy)
        {
            curl_easy_setopt(curl, CURLOPT_PROXY, addrProxy.ToStringIP().c_str());
            curl_easy_setopt(curl, CURLOPT_PROXYPORT, addrProxy.GetPort());
            curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
        }
    }

    ~DataFeedRequest()
    {
        if (curl)
            curl_easy_cleanup(curl);
    }

    void Perform()
    {
        CURLcode res;
        res = curl_easy_perform(curl);

        if (res == CURLE_OK)
        {
            long http_code = 0;
            if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code) != CURLE_OK)
                throw runtime_error("Unable to get data feed response code");

            if (http_code != 200)
                throw runtime_error((boost::format("Data feed failed: Response code %ld") % http_code).str());

            printf("Received %lu bytes from data feed %s:\n%s\n", (long)result.size(), url.c_str(), result.c_str());
        }
        else
        {
            if (error == "")
            {
                error = (boost::format("Data feed failed: %s") % curl_easy_strerror(res)).str();
                throw runtime_error(error);
            }
            else
                throw runtime_error(error);
        }
    }

    string GetResult() const
    {
        return result;
    }
};

void VerifyDataFeedSignature(const string& strMessage, const string& strSign, const string& strAddress)
{
    CBitcoinAddress addr(strAddress);
    if (!addr.IsValid())
        throw runtime_error("Invalid data feed address");

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
        throw runtime_error("Data feed address does not refer to key");

    bool fInvalid = false;
    vector<unsigned char> vchSig = DecodeBase64(strSign.c_str(), &fInvalid);

    if (fInvalid)
        throw runtime_error("Malformed base64 encoding of data feed signature");

    CDataStream ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    CKey key;
    if (!key.SetCompactSignature(Hash(ss.begin(), ss.end()), vchSig))
        throw runtime_error("Unable to set key from data feed signature");

    if (key.GetPubKey().GetID() != keyID)
        throw runtime_error("Data feed signature check failed");
}

bool GetVoteFromDataFeed(const CDataFeed& dataFeed, CVote& voteRet)
{
    bool result = false;

    try
    {
        DataFeedRequest request(dataFeed.sURL);
        request.Perform();

        if (dataFeed.sSignatureAddress != "" && dataFeed.sSignatureURL != "")
        {
            DataFeedRequest signatureRequest(dataFeed.sSignatureURL);
            signatureRequest.Perform();

            VerifyDataFeedSignature(request.GetResult(), signatureRequest.GetResult(), dataFeed.sSignatureAddress);
        }

        Value valReply;
        if (!read_string(request.GetResult(), valReply))
            throw runtime_error("Data feed returned invalid data");

        voteRet = ParseVote(valReply.get_obj());
        result = true;
    }
    catch (exception &e)
    {
        printf("GetVoteFromDataFeed error: %s\n", e.what());
        throw;
    }

    return result;
}

CVote ParseVote(const Object& objVote)
{
    CVote vote;

    BOOST_FOREACH(const Pair& voteAttribute, objVote)
    {
        if (voteAttribute.name_ == "motions")
        {
            BOOST_FOREACH(const Value& motionVoteObject, voteAttribute.value_.get_array())
                vote.vMotion.push_back(uint160(motionVoteObject.get_str()));
        }
        else if (voteAttribute.name_ == "custodians")
        {
            BOOST_FOREACH(const Value& custodianVoteObject, voteAttribute.value_.get_array())
            {
                CCustodianVote custodianVote;
                BOOST_FOREACH(const Pair& custodianVoteAttribute, custodianVoteObject.get_obj())
                {
                    if (custodianVoteAttribute.name_ == "address")
                    {
                        CBitcoinAddress address(custodianVoteAttribute.value_.get_str());
                        if (!address.IsValid())
                            throw runtime_error("Invalid address\n");

                        custodianVote.SetAddress(address);
                        if (custodianVote.cUnit == 'S' || !ValidUnit(custodianVote.cUnit))
                            throw runtime_error("Invalid custodian unit\n");
                    }
                    else if (custodianVoteAttribute.name_ == "amount")
                        custodianVote.nAmount = AmountFromValue(custodianVoteAttribute.value_);
                    else
                        throw runtime_error("Invalid custodian vote object\n");
                }
                vote.vCustodianVote.push_back(custodianVote);
            }
        }
        else if (voteAttribute.name_ == "parkrates")
        {
            BOOST_FOREACH(const Value& parkRateVoteObject, voteAttribute.value_.get_array())
            {
                CParkRateVote parkRateVote;
                BOOST_FOREACH(const Pair& parkRateVoteAttribute, parkRateVoteObject.get_obj())
                {
                    if (parkRateVoteAttribute.name_ == "unit")
                    {
                        parkRateVote.cUnit = parkRateVoteAttribute.value_.get_str()[0];
                        if (parkRateVote.cUnit == 'S' || !ValidUnit(parkRateVote.cUnit))
                            throw runtime_error("Invalid park rate unit\n");
                    }
                    else if (parkRateVoteAttribute.name_ == "rates")
                    {
                        BOOST_FOREACH(const Value& parkRateObject, parkRateVoteAttribute.value_.get_array())
                        {
                            CParkRate parkRate;
                            BOOST_FOREACH(const Pair& parkRateAttribute, parkRateObject.get_obj())
                            {
                                if (parkRateAttribute.name_ == "blocks")
                                {
                                   int blocks = parkRateAttribute.value_.get_int();
                                   double compactDuration = log2(blocks);
                                   double integerPart;
                                   if (modf(compactDuration, &integerPart) != 0.0)
                                       throw runtime_error("Park duration is not a power of 2\n");
                                   if (compactDuration < 0 || compactDuration > 255)
                                       throw runtime_error("Park duration out of range\n");
                                   parkRate.nCompactDuration = compactDuration;
                                }
                                else if (parkRateAttribute.name_ == "rate")
                                {
                                    double dAmount = parkRateAttribute.value_.get_real();
                                    if (dAmount < 0.0 || dAmount > MAX_MONEY)
                                        throw runtime_error("Invalid park rate amount\n");
                                    parkRate.nRate = roundint64(dAmount * COIN_PARK_RATE);
                                    if (!MoneyRange(parkRate.nRate))
                                        throw runtime_error("Invalid park rate amount\n");
                                }
                                else
                                    throw runtime_error("Invalid park rate object\n");
                            }
                            parkRateVote.vParkRate.push_back(parkRate);
                        }
                    }
                    else
                        throw runtime_error("Invalid custodian vote object\n");
                }
                vote.vParkRateVote.push_back(parkRateVote);
            }
        }
        else
            throw runtime_error("Invalid vote object\n");
    }

    return vote;
}

void UpdateFromDataFeed()
{
    CWallet* pwallet = GetWallet('S');
    CDataFeed dataFeed;
    {
        LOCK(pwallet->cs_wallet);
        dataFeed = pwallet->GetDataFeed();
    }

    if (dataFeed.sURL == "")
        return;

    printf("Updating from data feed %s\n", dataFeed.sURL.c_str());

    CVote feedVote;
    if (GetVoteFromDataFeed(dataFeed, feedVote))
    {
        LOCK(pwallet->cs_wallet);
        CVote newVote = pwallet->vote;
        BOOST_FOREACH(const string& sPart, dataFeed.vParts)
        {
            if (sPart == "custodians")
                newVote.vCustodianVote = feedVote.vCustodianVote;
            else if (sPart == "parkrates")
                newVote.vParkRateVote = feedVote.vParkRateVote;
            else if (sPart == "motions")
                newVote.vMotion = feedVote.vMotion;
            else
                throw runtime_error("Invalid part");
        }
        pwallet->SetVote(newVote);
        printf("Vote updated from data feed\n");
    }
}

void ThreadUpdateFromDataFeed(void* parg)
{
    printf("ThreadUpdateFromDataFeed started\n");

    int delay = GetArg("-datafeeddelay", 60);
    int64 lastUpdate = GetTime();

    while (true)
    {
#ifdef TESTING
        Sleep(200);
#else
        Sleep(1000);
#endif

        if (fShutdown)
            break;

        int64 now = GetTime();

        if (now >= lastUpdate + delay)
        {
            strDataFeedError = "";
            try
            {
                UpdateFromDataFeed();
            }
            catch (exception &e)
            {
                printf("UpdateFromDataFeed failed: %s\n", e.what());
                strDataFeedError = string(e.what());
            }

            lastUpdate = now;
        }
    }

    printf("ThreadUpdateFromDataFeed exited\n");
}

void StartUpdateFromDataFeed()
{
    if (!CreateThread(ThreadUpdateFromDataFeed, NULL))
        printf("Error; CreateThread(ThreadUpdateFromDataFeed) failed\n");
}
