#!/bin/sh

FILES=`find -iname '*Enums.xml' | perl -pe 's#.xml##g'`
for i in $FILES; do touch -d "-1 year" $i.hh $i.cpp; done
