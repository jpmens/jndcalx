% jndcalx - Create ICALENDAR from Lotus Notes/Domino
% Jan-Piet Mens
% __ISO_DATE__


# Motivation

I want to see tasks and calendar entries from my IBM/Lotus Notes calendar in
ICAL on my Mac. To enable this, I created JX -- a C program which runs on a
Lotus Notes platform (Mac OS X, Windows or Linux), and which creates ICALENDAR
output (`.ics`) on the fly, without having to manually export and import your
calendar. In other words, you can use JX as a XXX calendar, and have ICAL or
SUNBIRD periodically refresh their calendars from Lotus Notes or Domino.

# Platforms

I run JX on Mac OS X with a Notes 8.5 installation. I originally
started writing JX on Windows XP with Notes 6.5, and it still compiles
and runs on that as well. I haven't tried Linux, but there is no
obvious reason why it won't work on that.

## Mac OS X

Mac OS X, being UNIX, has all the good things that UNIX has, including
multi-user, an installed Web server, etc. The installation section
below shows you how to get JX running on Mac OS X.

## Windows

Windows, _not_ being UNIX, is a bitch. Look somewhere else for getting
a Web server running on Windows ([Apache](http://httpd.apache.org) or
[Cygwin]() are good places to start looking). If you don't have a
local Web server, you can still use JX from the command-line and tell
it to create an `.ics` file.

## Domino

I originally wanted JX to run directly on Lotus Domino, as a CGI
program and have it read an authenticated user's mail file to produce
calendar output. The design is still there, but I haven't as yet
completed the code. (If you are _very_ interested in that, I'll try
and hurry up; it's not just a money problem -- it's always a money
problem.)

# Bugs

Yes, lots. Maybe. There are things that work, and there are (probably) plenty
of things that don't work. I'm not an ICALENDAR specialist, nor will I ever
be. I use this program and will be fixing things that don't work for me. If you
find things that don't work, tell me: if I have the time and the knowledge,
I'll fix them. I prefer that you fix bugs, which is why I make the source code
available, and I'll then gladly merge your fixes.


# Demonstration

Let me first show you what works. (Things that I _know_ don't work are
documented in the `TODO` file, which accompanies this software.

Here are a few screen shots of a calendar I've created in a Notes 8.5
mail file. (There is no difference between 6.5 and 8.5 -- JX reads
and processes both identically.)

### Todo


### Calendar


#### Reminder
#### Aniversary
#### Recurring event







# Using jndcalx


## Prerequisites

You'll need a workstation with an installed and running version of
Lotus Notes (I've tried 6.5, 7.something, and 8.5.) Note that I say
"running": the Notes client must be launched so that the ID file is
unlocked. If Notes isn't running, all sorts of ugly things will
happen (but you don't want to know that).

Notes is great, and all that, but having an external C program,
which uses the C API, access a Notes database means that the C
program, JX in this case, will prompt you for a password. If you know
what you are doing, skip over this section, but I don't think you do,
so carry on reading.

There is currently only one really clean method, and that us to allow
C programs on your workstation to access your databases without
forcing them to authenticate. In order to allow them to do so (which
poses a horrible security risk, so don't do that) you only have to set
a small checkbox right like this:

fig(2.2, assets/notes-sec-password.png, Notes security settings)

You'll find that checkbox somewhere in File->Security. Go ahead, and
do that, but don't tell your system administrator: she'll send you a
dragon and force you to use Outlook Express.

If you don't have this set, you're on your own, and JX will _not_ work
as described below. (What will work however, is starting JX on the
command-line and have it create an `.ics` file -- JX will prompt you
for your Notes ID password.)

### Installing JX

Installing JX means copying the binary file to some clever location.
I'd like to suggest `/usr/local/bin` on your workstation. Copy the
`runlotus` program to the same place:

	sudo install -m 111 jndcalx /usr/local/bin/jndcalx
	sudo install -m 4111 -o root runlotus /usr/local/bin/runlotus

Did you see that? My God! A set-uid-bit program to root? To run JX?
Yes, that is, unfortunately necessary. Here are the reasons:

* `jndcalx`, the actual executable C program that calls the Lotus API
  has to do so as _me_ (that is, _you_, well, the user who owns the
  Notes installation on Mac OS X/Linux). It does so wonderfully, when
  you run it yourself, but you won't typically be running it yourself!
* We want our local Web server (Apache on Mac OS X) to launch
  `jndcalx` when we access a particular CGI/PHP resource. Apache will
  run the program as user `www` and group `www`, and that is not _me_,
  well _you_, you get what I mean. So we need a small program that
  becomes _you_ and executes `jndcalx` as you. The only way to do
  that, is by running a setuid program.
* If you know better and/or if you mistrust the few lines of source
  code of `runlotus.c`, you can play around with Apache's _suexec_
  module and try and get that working -- it's up to you. The important
  bit is that when `jndcalx` runs, it runs with the effective userid
  of _you_. (To determine your effective user id, open Terminal and
  type

		$ id
		uid=501(you) gid=20(staff) ....

### Command line


### Specifying Notes databases

You can specify a Lotus Notes database as being either locally
accessible on your workstation or remotely accessible on a Lotus
Domino server.

* For databases on your local workstation, specify the path to the
* database using forward slashes (`/`) (yes, also on Windows!),
* whereby the path is relative to your Lotus Notes data directory. For
* example:

		mail.nsf
		mail/username.nsf

* For remote databases on a Domino server, specify the path as
* _servername_, _bangbang_, _databasename_, taking into account that a
* connection from your workstation to the _servername_ must be
* possible, and that your user must be allowed to access
* _databasename_ on that server. For example:

		marketing/servers/o=megacorp!!mail/username.nsf

### Access to Notes databases.

Command-line program wanting to access Lotus Notes databases will
prompt the user for her password if the ACL of the database doesn't
allow default readership. The only solution at the moment is to
configure your Notes security to not have external programs prompt for
a password, but do note that this can be a security problem. (Consult
your system administrator -- if you have one -- or the Lotus Notes
documentation for more information about this.)

To disable external programs to prompt for your passowrd, select
_File_, _Security_, _User security_ from your Notes menu. In the
_Security Basics_ dialog box, ensure the setting _Don't prompt for a
password from other Notes-based programs_ is checked.




