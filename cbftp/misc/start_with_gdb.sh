#!/bin/bash
pushd `dirname $0` > /dev/null
SCRIPTPATH=`pwd`
popd > /dev/null
gdb $SCRIPTPATH/cbftp --eval-command="set confirm off" --eval-command="handle SIGINT nostop print pass" --eval-command="set confirm on" --eval-command="run"
