OS Simulator

This program sets its own system clock, created a number of user processes, and runs them for a random set of time. Upon termination, it prints the output to the log file. It's awesome. 

Usage: ./oss -c x -f executable -t time
-c: The maximum number of processes to run
-f: The process the OS will run -c amount of times
-t: The amount of timein seconds the OS will run before quitting itself
