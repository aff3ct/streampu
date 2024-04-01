#!/bin/bash

find include -iname *.h -o -iname *.hpp -o -iname *.hxx | xargs clang-format -i
find src -iname *.cpp -o -iname *.h -o -iname *.hpp -o -iname *.hxx | xargs clang-format -i
find tests -iname *.cpp -o -iname *.h -o -iname *.hpp -o -iname *.hxx | xargs clang-format -i
find signal_tracer -iname *.cpp -o -iname *.h -o -iname *.hpp -o -iname *.hxx | xargs clang-format -i
