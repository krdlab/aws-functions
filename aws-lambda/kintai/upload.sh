#!/bin/bash
set -eu

test -f ../kintai.zip && rm ../kintai.zip
zip -r -q ../kintai.zip ./
