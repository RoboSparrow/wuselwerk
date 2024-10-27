#!/bin/bash

# this script is called from make

GIT_TAG="$(git describe --tags --abbrev=0)"
if [ "${?}" -ne 0 ];
    then GIT_TAG="<none>";
fi

GIT_HASH="$(git rev-parse --short HEAD)"

cat > ./build <<EOF
${GIT_TAG}, ${GIT_HASH}
EOF
