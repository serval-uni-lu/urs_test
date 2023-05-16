#!/bin/bash
fn=$(echo "$1" | sed 's/.def$/.sif/')
singularity build --fakeroot "$fn" "$1"
