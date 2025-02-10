#!/bin/bash

declare output="/home/gsolomon/output.mp4"
declare temp_file="output.ogv"

printf "STATUS: recording...ctrl-C to stop\n"
recordmydesktop --x 550 --y 150 --width 1200 --height 800 -o ${temp_file}

printf "STATUS: convverting to mp4...\n"
ffmpeg -i ${temp_file} -vcodec libx264 "${output}"
rm -f ${temp_file}

printf "STATUS: saved ${output}\n"
