#!/bin/bash

#set -ev


rm tmpcoverage_*
rm zerocov_raw.txt

find ../../ -type f -name "*.gcda" > lst
for i in `cat lst`; do 
    gcov -a -p -b $i # -> creates src#foo.cpp.gcov
    find ../../ -type f -name "*.gcov" > lstcov
    while read i ; do
       egrep -e "function .* called [0-9]+" $i  >> zerocov_raw.txt
       rm $i
    done < lstcov
done

#get a summary of functions never called:
python coverage_summary.py zerocov_raw.txt | egrep -v -i -e "^__pyx" \
    | grep -v pthread | grep -v gthread | grep -v gnu_cxx | grep -v bswap | grep "PTools::"


echo done
