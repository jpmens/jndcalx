This is jndcalx, a program which exports Lotus Notes calendar entries in
iCalendar format. It works for me and it might work for you. I've
[written a bit about my
motivation](http://blog.fupps.com/2009/10/23/get-icalsunbird-to-subscribe-to-your-lotus-notes-calendar/)
for creating this.

Snippets of documentation are in the doc/ directory.

You'll need to do the following to build `jndcalx` on Mac OS X:

- Edit `env.mac` and set the `LOTUS` environment variable to the
  directory containing your `notesapi` directory.

		export LOTUS="/Users/jpm/Zz"
		export Notes_ExecDirectory="/Applications/Notes.app/Contents/MacOS"
		export DYLD_LIBRARY_PATH=${Notes_ExecDirectory}

- Source `env.mac`

		source env.mac

- Make the binary

		make -f Makefile.macosx jndcalx

- Test `jndcalx` by giving it the name of a local (or remote) database.
  Ensure your copy of Lotus Notes is running or you'll be prompted for a
  password.

		./jndcalx jpm.nsf > test.ics
		Notes: Can't open database: err=0x103 [File does not exist]
		./jndcalx mail/jpm.nsf > test.ics

- The resulting file `test.ics` contains your calendar.

If, and only if, the above was successful, you can continue with the next steps.

You'll need to do the following to run `jndcalx` on Mac OS X via a CGI wrapper:

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


