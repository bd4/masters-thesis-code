#!/bin/bash

for f in cryptosystems/speedtest*.elg; do
    elgamaltime -m32 -c10000 $f >> speedtest.log
    elgamaltime -m64 -c10000 $f >> speedtest.log
done
