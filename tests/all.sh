#!/bin/bash

# # テストするのはとりあえずtournament-sse42のみ(avx2が無いインスタンスを引くことがあるので)
# if [ "x$1" != "xtournament-sse42" ]; then
#   echo "testing cancel(!= $1)"
#   exit 0
# fi

echo "testing started"
pwd

echo "eval data downloading.."
wget --load-cookies /tmp/cookies.txt \
  `wget --keep-session-cookies --save-cookies=/tmp/cookies.txt \
    'https://drive.google.com/uc?id=1r4o85v6_wxQhIF4K7sAuCfuLzTxIJLJ-' -q -O - \
    | perl -nle 'if($_=~/download-link.*?href="(.*?)"/i){$str=$1;$str=~s/&amp;/&/g;print "https://drive.google.com$str";}'` \
      -O qzilla7.7z > /dev/null 2>&1
if [ $? != 0 ]; then
  echo "testing failed(wget)"
  exit 1
fi

echo "eval data unzip.."
7z x qzilla7.7z
if [ $? != 0 ]; then
  echo "testing failed(7z x)"
  exit 1
fi

mv qzilla7 eval

wget https://github.com/nodchip/tnk-/releases/download/wcsc28-2018-05-05/tnk-wcsc28-2018-05-05.7z -O tnk.7z > /dev/null 2>&1
if [ $? != 0 ]; then
  echo "testing failed(wget)"
  exit 1
fi

echo "eval data unzip.."
7z x tnk.7z
if [ $? != 0 ]; then
  echo "testing failed(7z x)"
  exit 1
fi

echo "loading eval.bin.."
cat eval/*.bin > /dev/null 2>&1

./bench.sh

