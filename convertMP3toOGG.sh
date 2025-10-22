#!/usr/bin/env bash

# Example:
#   ./convertMP3toOGG.sh applications/client/render-data/sounds/Duncan\ Woods/Cameleon

function convert() {
  # arg $1: path to audio file

  ffmpeg -hide_banner -loglevel error \
    -i "${1}" "${1}".ogg
}

function main() {
  # arg $1: Path to directory containing *.mp3 files

  for file in "${1}/"*.mp3 ; do
    convert "${file}" || return $?
  done
}

main "$@"
