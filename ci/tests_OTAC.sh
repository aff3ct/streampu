#!/bin/bash

bin=../build/bin/test-generic-pipeline
file=out.txt
res="# Solution stages {(n,r)}:"
rm -f $file

echo "# Test: one resource"
echo "[init/1.5/][relayf/15/][incrf-20-][relay/15/][fin/1.5/]"
./${bin} -t "1" -C "(init,relayf_15,incrementf_S_20,relay_15,fin)" -e 100 | grep "$res {(5, 1)}" > $file
if [ -e $file ]
then
   if [ -s $file ]
   then
      echo -e "\e[32mTest passed!\e[0m \n" 
   else
      echo -e "\e[31mTest NOT passed!\e[0m \n"
   fi
else
   echo -e "\e[31mTest NOT passed!\e[0m \n" 
fi
rm -f $file

echo "# Test: stateless chain"
echo "[init/1.5/][relayf/15/][incrf/20/][relay/15/][fin/1.5/]"
threads=$(( $RANDOM % 10 + 1 ))
./${bin} -t "$threads" -C "(init,relayf_15,incrementf_20,relay_15,fin)" -e 100 | grep "$res {(5, ${threads})}" > $file
if [ -e $file ]
then
   if [ -s $file ]
   then
      echo -e "\e[32mTest passed!\e[0m \n" 
   else
      echo -e "\e[31mTest NOT passed!\e[0m \n"
   fi
else
   echo -e "\e[31mTest NOT passed!\e[0m \n" 
fi
rm -f $file

echo "# Test: stateless chain"
echo "[init-1.5-][relayf-15-][incrf-20-][relay-15-][fin-1.5-]"
threads=$(( $RANDOM % 10 + 3 ))
./${bin} -t "$threads" -C "(init_S,relayf_S_15,incrementf_S_20,relay_S_15,fin_S)" -e 100 | grep "$res {(2, 1)(1, 1)(2, 1)}" > $file
if [ -e $file ]
then
   if [ -s $file ]
   then
      echo -e "\e[32mTest passed!\e[0m \n" 
   else
      echo -e "\e[31mTest NOT passed!\e[0m \n"
   fi
else
   echo -e "\e[31mTest NOT passed!\e[0m \n" 
fi
rm -f $file

echo "# Test: one stateless task"
echo "[init-1.5-][relayf-15-][incrf/120/][relay-15-][fin-1.5-]"
./${bin} -t "6" -C "(init_S,relayf_S_15,incrementf_120,relay_S_15,fin_S)" -e 100 | grep "$res {(2, 1)(1, 4)(2, 1)}" > $file
if [ -e $file ]
then
   if [ -s $file ]
   then
      echo -e "\e[32mTest passed!\e[0m \n" 
   else
      echo -e "\e[31mTest NOT passed!\e[0m \n"
   fi
else
   echo -e "\e[31mTest NOT passed!\e[0m \n" 
fi
rm -f $file

echo "# Test: one stateful task"
echo "[init/1.5/][relayf/15/][incrf-60-][relay/15/][fin/1.5/]"
./${bin} -t "3" -C "(init,relayf_15,incrementf_S_60,relay_15,fin)" -e 100 | grep "$res {(2, 1)(1, 1)(2, 1)}" > $file
if [ -e $file ]
then
   if [ -s $file ]
   then
      echo -e "\e[32mTest passed!\e[0m \n" 
   else
      echo -e "\e[31mTest NOT passed!\e[0m \n"
   fi
else
   echo -e "\e[31mTest NOT passed!\e[0m \n" 
fi
rm -f $file