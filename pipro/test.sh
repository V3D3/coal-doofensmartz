./build.sh

for $i in (tests/*):
    ./app tests/$i/DCache.txt tests/$i/ICache.txt tests/$i/RF.txt -o tests/$i/aout
    // cmp files in aout and out here, and report pass/fail to terminal