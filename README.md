MicroShell SOP
==============

MatiShell
---------
A simple C programme meant to emulate a standard BASH Shell.
WARNING: It is prepared to run correclty only on Linux

Programmed as a university project.

Has a few quirks and features:
*  a `help` command which displays a simple overview of its features
*  the `cd` command which was fully implemented by me
*  an `exit` command
*  a simple `history` command (stores the history of your current session) with a buffer, which deletes old entries when it reaches its limit
*  a custom parser written by me which properly handles quotation marks and apostrophes in interpreting commands
*  when you provide it with a command that hasn't been implemented by me, it searches for that command using the `$PATH` variable and runs it with given arguments once it finds it

Usage
------
To properly configure and run this programme:
* navigate to this repo on your local machine (using a terminal)
* enter the `microshell` directory
* compile the programme with this command `$ gcc -ansi -Wall -o MatiShell main.c`
* now you can run it with `./MatiShell`