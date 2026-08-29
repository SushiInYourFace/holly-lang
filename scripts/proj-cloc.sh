#!/bin/zsh
PROJECT_DIR="/Users/Noelle/Programming/holly-lang"
HIST_FILE="${PROJECT_DIR}/scripts/cloc_results"


EXCLUDE_DIRS=".cache,holly.dSYM,uthash,uthash-2.4.0"
EXCLUDE_LANGS="XML,JSON"

TIMESTAMP=$(date "+%Y-%m-%d %H:%M:%S")

CLOC_OUT=$(cloc --exclude-dir=$EXCLUDE_DIRS --quiet --exclude-lang=$EXCLUDE_LANGS $PROJECT_DIR)

echo "$CLOC_OUT"
CLOC_SUM=$(echo "$CLOC_OUT" | grep "SUM")

echo "${TIMESTAMP}: $CLOC_SUM" >> $HIST_FILE