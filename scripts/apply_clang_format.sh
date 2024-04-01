#!/bin/bash

find include -iname *.h -o -iname *.hpp -o -iname *.hxx | xargs clang-format -i