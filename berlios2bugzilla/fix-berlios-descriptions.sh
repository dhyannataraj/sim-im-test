#/bin/sh

# remove all invalid XML chars
sed -r -e 's/[^]a-zA-Z0-9,.<>()[\\/ `~!?@#$%^&*{}_+:;\"|'"'"'\=-]//g' $1
