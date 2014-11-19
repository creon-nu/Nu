#ifndef DATA_FEED_H
#define DATA_FEED_H

#include <string>
#include "json/json_spirit_value.h"

class CVote;

bool GetVoteFromDataFeed(std::string sURL, CVote& voteRet);
CVote ParseVote(const json_spirit::Object& objVote);
void StartUpdateFromDataFeed();
void UpdateFromDataFeed();

extern std::string strDataFeedError;

#endif
