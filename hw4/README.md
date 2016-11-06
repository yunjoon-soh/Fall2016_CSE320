# Homework 4 - SFISH Shell

## Yun Joon (Daniel) Soh (108256259)
* Successfully implement all part 5.
* No extra credit implemented

## Notes.
* Piped processes are each forked separately. e.g., yes | grep n
  * In such case, ctrl-z will stop the yes process only and move on to grep
  * Since grep's stdin pipe is not closed by former process, "yes", it waits
  * So, to stop it, you need to ctrl-z twice.
* ctrl-p only prints CURRENTLY RUNNING PROCESSES
  * In other words, it will be different from what "jobs" command returns

## Return values
* Builtin commands return SF\_SUCCESS, which is 0, on success, SF\_FAIL, which is 1, on failure.

## Tested with criterion libraries

Criterion - unit testing library
The MIT License (MIT)
Copyright © 2015-2016 Franklin "Snaipe" Mathieu <http://snai.pe/>
