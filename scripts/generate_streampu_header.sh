#!/bin/bash

SRC_DIR=".\/include"
if [ -z "$1" ]; then
  STREAMPU_HEADER=.\/include\/streampu.hpp
else
  STREAMPU_HEADER="$1"
fi

# create the StreamPU header file with the C++ header
echo "/*!"                                                                      >  $STREAMPU_HEADER
echo " * \\file"                                                                >> $STREAMPU_HEADER
echo " * \\brief StreamPU main header file, include all the other headers."     >> $STREAMPU_HEADER
echo " *"                                                                       >> $STREAMPU_HEADER
echo " * \\section LICENSE"                                                     >> $STREAMPU_HEADER
echo " * This file is under MIT license (https://opensource.org/licenses/MIT)." >> $STREAMPU_HEADER
echo " */"                                                                      >> $STREAMPU_HEADER
echo ""                                                                         >> $STREAMPU_HEADER
echo "#ifndef STREAMPU_HPP"                                                     >> $STREAMPU_HEADER
echo "#define STREAMPU_HPP"                                                     >> $STREAMPU_HEADER
echo ""                                                                         >> $STREAMPU_HEADER

if [[ $OSTYPE == 'darwin'* ]]; then
  INCLUDE_PATH=.\/include
else
  INCLUDE_PATH=.\/include\/
fi

headersh=$(find $INCLUDE_PATH -type f -follow -print | # find all source files
           grep "[.]h$" |                              # take all .h files
           sed 's/[[:blank:]]\+/\n/g')                 # change end lines with '\n'

headershpp=$(find $INCLUDE_PATH -type f -follow -print | # find all source files
             grep "[.]hpp$" |                            # take all .hpp files
             grep -v streampu.hpp |                      # remove streampu.hpp from the file list
             sed 's/[[:blank:]]\+/\n/g')                 # change end lines with '\n'

headers="$headersh $headershpp"
# sort the header by alphabetical order
headers=$(echo $headers | tr " " "\n" | sort -f -d | tr "\n" " " ; echo)

for f in $headers; do
  guard=$(grep -m1 "#ifndef" < $f)
  if [ "$guard" != "#ifndef DOXYGEN_SHOULD_SKIP_THIS" ]; then
    echo $guard >> $STREAMPU_HEADER # add a guard to the header file to accelerate compilation
    rc=$?; if [[ $rc != 0 ]]; then echo "No #ifndef in file '$f'."; exit 1; fi

    echo $f | sed "s/$SRC_DIR\//#include </" | # change "./src" with "#include <"
    sed 's|$|>|g' >> $STREAMPU_HEADER # add at the end of each line the ">" to close the 'include'
    echo "#endif" >> $STREAMPU_HEADER # close the guard
  fi
done

echo -ne "\n#endif" >> $STREAMPU_HEADER # add some empty lines in the StreamPU header file and close the 'define' header
