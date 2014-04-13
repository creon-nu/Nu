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

