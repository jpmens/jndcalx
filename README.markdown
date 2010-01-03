This is jndcalx, a program which exports Lotus Notes calendar entries in
iCalendar format. It works for me and it might work for you. 

Snippets of documentation are in the doc/ directory.

You'll need to do the following:

- Edit `runlotus.c` and change the value of `DIR` to reflect the
  directory in which you're installing the executable `jndcalx`. Further
  set `UNIXUID` and `UNIXGID` to the numeric values of your uid and gid
  respectively. 
- Compile the program `make runlotus` and then make the program setuid
  to root

		---s--x--x  1 root  wheel  12708 Sep  7 19:58 runlotus

- On Mac OS X I use a CGI script such as that contained in `testcal.cgi`
  to launch the program.

If you have improvements to suggest or to make, I'd greatly appreciate
patches.

