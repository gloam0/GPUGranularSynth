#!/bin/bash

sudo apt-get -y install libasound2-dev libasound2-plugins alsa-utils libjack-jackd2-dev
git submodule update --init --recursive

# test audio with `speaker-test -c 2 -r 48000 -D default -t sine -l 1`