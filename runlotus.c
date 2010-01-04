#include <stdio.h>
#include <unistd.h>

#define DIR "/Users/jpm/Auto/projects/on-github/jndcalx"
#define UNIXUID	501
#define UNIXGID	20

int main(int argc, char **argv)
{
	if (chdir(DIR) == -1) {
		perror(DIR);
		exit(1);
	}

	if (setgid(UNIXGID) == -1) {
		perror("setgid");
		exit(1);
	}

	if (setuid(UNIXUID) == -1) {
		perror("setuid");
		exit(1);
	}
	putenv("Notes_ExecDirectory=/Applications/Notes.app/Contents/MacOS");
	putenv("DYLD_LIBRARY_PATH=/Applications/Notes.app/Contents/MacOS");

	execlp("./jndcalx", "jndcalx", "testcal.nsf", 0);

	perror("exec");
}
