#!/bin/bash

OUTDIR=_out
DEFAULT_TARGET=build
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
}

function clean {
    rm -rf $OUTDIR
}

cd $BASEDIR
main $*
