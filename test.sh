#!/usr/bin/env expect -f

spawn example
expect "> "

send "quit\n"
expect eof
