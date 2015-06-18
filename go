#!/bin/bash

OUTDIR=_out
DEFAULT_TARGET=default
BASEDIR=$(dirname $0)

function main {
    if [ "$1" == "" ]; then
	TARGET=$DEFAULT_TARGET
    else
	TARGET=$1
    fi

    $TARGET
}

function build {
    mkdir -p $OUTDIR
    cd $OUTDIR
    cmake ..
    make
    cd ..
}

function tests {
    cd $OUTDIR
    ctest --output-on-failure .
}

function docs {
    cd $BASEDIR
    doxygen Doxyfile
}

function default {
    build && docs && tests
}

function clean {
    rm -rf $OUTDIR
}

cd $BASEDIR
main $*
