#!/bin/bash

NUD=/path/to/nud
ARGS="-testnet"
VOTE_PATH=/path/to/output/vote.json
SIGNATURE_PATH=/path/to/output/vote.json.signature
ADDRESS=S....

$NUD $ARGS getvote >$VOTE_PATH
$NUD $ARGS signmessage $ADDRESS <$VOTE_PATH >$SIGNATURE_PATH
