#!/usr/bin/env bash

# Example:
#   ./concatToOGG.sh mp3 applications/client/render-data/sounds/Duncan\ Woods/Cameleon

ffmpeg -f concat -safe 0 -i <( for f in "${2}"/*."${1}"; do echo "file '$(pwd)/$f'"; done ) "${2}"/merged.ogg
