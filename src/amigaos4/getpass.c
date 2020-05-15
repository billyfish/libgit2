/* Getpass.c */

#define	PASSWD_LEN	127


#include <stdio.h>
#include <signal.h>


static void echo(FILE *fp, int on)
{
    static int f_echo = 1;

    if(fp != stdin)
    {
			fprintf(stderr,"echo: Internal error, echo not called on stdin\n");
    }
    else
    {
			if(on)					/* always do on */
				{
					f_echo = 1;
				}
			else if(f_echo && !on)
				{
					f_echo = 0;
				}
    }
}

char *getpass(char *promptstr)
{
	register int ch;
	register char *p;
	FILE *fp, *outfp;
	void (*oldintr)(int);
	static char buf[PASSWD_LEN + 1];
	/*
	 * read and write to /dev/tty if possible; else read from
	 * stdin and write to stderr.
	 */
		outfp = stderr;
	fp = stdin;

	oldintr = signal(SIGINT, SIG_IGN);
	echo(fp, 0);				/* Turn echoing off. */
	(void) fputs(promptstr, outfp);
	(void) rewind(outfp);		/* implied flush */
	for (p = buf; (ch = getc(fp)) != EOF && ch != '\n' && ch != '\r';)
	{
#if 0
	    printf("ch = 0x%02lx\n", (int) ch); 
#endif
		if (p < buf + PASSWD_LEN)
			*p++ = ch;
	}
	*p = '\0';
	fputs ("\n", outfp);
	echo(fp, 1);
	(void) signal(SIGINT, oldintr);
	if (fp != stdin)
		(void)fclose(fp);
	return(buf);
}								/* Getpass */


/* eof Getpass.c */
