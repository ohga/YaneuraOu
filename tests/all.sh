#!/bin/bash

# # テストするのはとりあえずtournament-sse42のみ(avx2が無いインスタンスを引くことがあるので)
# if [ "x$1" != "xtournament-sse42" ]; then
#   echo "testing cancel(!= $1)"
#   exit 0
# fi

echo "testing started"
pwd

echo "eval data downloading.."
#wget http://garnet-alice.net/shogiengine/godwhale/client/download/GodwhaleClient_1_1_1_0_full.zip -O GodwhaleClient_1_1_1_0_full.zip > /dev/null 2>&1
wget --load-cookies /tmp/cookies.txt \
    `wget --keep-session-cookies --save-cookies=/tmp/cookies.txt \
        'https://drive.google.com/uc?id=1FV6BX6tnDyJdCJt31jmCjRDLrqkuT8tH' -q -O - \
            | perl -nle 'if($_=~/download-link.*?href="(.*?)"/i){$str=$1;$str=~s/&amp;/&/g;print "https://drive.google.com$str";}'` \
                  -O GodwhaleClient_1_1_1_0_full.zip > /dev/null 2>&1
if [ $? != 0 ]; then
  echo "testing failed(wget)"
  exit 1
fi

echo "eval data unzip.."
unzip -o GodwhaleClient_1_1_1_0_full.zip > /dev/null 2>&1
if [ $? != 0 ]; then
  echo "testing failed(unzip)"
  exit 1
fi

mv ./Engine/eval ./

echo "loading eval.bin.."
cat eval/*.bin > /dev/null 2>&1

./bench.sh

