#!/bin/bash
BUILD_DIR="./build_release"
cppcheck --force --xml --enable=all --inconclusive --language=c++ --std=c++14 --platform=unix64 -I "./$BUILD_DIR/boost_install/include/" -I "./$BUILD_DIR/folly_install/include/" ./src  2> result.xml &&\
cppcheck-htmlreport --source-encoding="utf8" --title="PEPM19-poster" --source-dir=. --report-dir=cppcheck_report --file=result.xml && rm result.xml