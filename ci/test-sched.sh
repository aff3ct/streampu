#!/bin/bash

if [ -z "$BIN_PATH" ]
then
   bin=../build/bin/test-generic-pipeline
else
   bin=$BIN_PATH
fi

file=out.txt
res="# Solution stages {(n,r)}:"
rm -f $file

retval=0
function check_solution # param 1 => output file name, param 2 => expected solution
{
   file=$1
   expected_solution=$2
   given_solution=$(cat $file | grep "$res")

   is_right_solution=$(echo $given_solution | grep "$expected_solution")
   if [ -z "$is_right_solution" ]
   then
      echo -e "\e[31mTest NOT passed!\e[0m"
      echo -e "  - Expected solution: $expected_solution"
      echo -e "  - Given solution: $given_solution"
      retval=1
   else
      echo -e "\e[32mTest passed!\e[0m"
   fi
   rm -f $file
}

echo "# Test: one resource"
echo "[init/1.5/][relayf/15/][incrf-20-][relay/15/][fin/1.5/]"
./${bin} -t "1" -C "(init_15,relayf_150,incrementf_S_200,relay_150,fin_15)" -S "OTAC" -v -e 1 -P "none" > $file
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi # check if the previous execution succeeded
check_solution $file "{(5, 1)}"
echo " "

echo "# Test: stateless chain"
echo "[init/1.5/][relayf/15/][incrf/20/][relay/15/][fin/1.5/]"
threads=$(( $RANDOM % 10 + 1 ))
./${bin} -t "$threads" -C "(init_15,relayf_150,incrementf_200,relay_150,fin_15)" -S "OTAC" -v -e 1 -P "none" > $file
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi # check if the previous execution succeeded
check_solution $file "{(5, ${threads})}"
echo " "

echo "# Test: stateful chain"
echo "[init-1.5-][relayf-15-][incrf-20-][relay-15-][fin-1.5-]"
threads=$(( $RANDOM % 10 + 3 ))
./${bin} -t "$threads" -C "(init_S_15,relayf_S_150,incrementf_S_200,relay_S_150,fin_S_15)" -S "OTAC" -v -e 1 -P "none" > $file
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi # check if the previous execution succeeded
check_solution $file "{(2, 1)(1, 1)(2, 1)}"
echo " "

echo "# Test: one stateless task"
echo "[init-1.5-][relayf-15-][incrf/120/][relay-15-][fin-1.5-]"
./${bin} -t "6" -C "(init_S_15,relayf_S_150,incrementf_1200,relay_S_150,fin_S_15)" -S "OTAC" -v -e 1 -P "none" > $file
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi # check if the previous execution succeeded
check_solution $file "{(2, 1)(1, 4)(2, 1)}"
echo " "

echo "# Test: one stateful task"
echo "[init/1.5/][relayf/15/][incrf-60-][relay/15/][fin/1.5/]"
./${bin} -t "3" -C "(init_15,relayf_150,incrementf_S_600,relay_150,fin_15)" -S "OTAC" -v -e 1 -P "none" > $file
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi # check if the previous execution succeeded
check_solution $file "{(2, 1)(1, 1)(2, 1)}"

exit $retval
