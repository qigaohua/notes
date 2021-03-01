#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <syslog.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


// 1
#define swap(a, b) (a ^= b, b ^= a, a ^= b)

// 2
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)


// 3
char *conf_get_by_key(const char *fname, const char *key)
{
    int len = 0;
    int key_len = 0;
    char buf[4096];
    FILE *fp = NULL;
    char *p = NULL;

    if (fname == NULL || key == NULL) {
        return NULL;
    }
    key_len = strlen(key);
    if (key_len < 1) {
        return NULL;
    }
    fp = fopen(fname, "rb");
    if (fp == NULL) {
        return NULL;
    }
    while (fgets(buf, sizeof(buf), fp)) {
        if (buf[0] == '#') {
            continue;
        }
        len = strlen(buf);
        if (len < 2) {
            continue;
        }
        /* Remove the EOL */
        /* EOL == CR */
        if (buf[len - 1] == '\r') {
            buf[len - 1] = 0;
            len -= 1;
        } else if (buf[len - 1] == '\n') {
            /* EOL = CR+LF */
            if (buf[len - 2] == '\r') {
                buf[len - 2] = 0;
                len -= 2;
            } else {
                /* EOL = LF */
                buf[len - 1] = 0;
                len -= 1;
            }
        }

        /* Skip the empty line */
        if (len == 0) {
            continue;
        }
        if (strncasecmp(buf, key, key_len) == 0 && buf[key_len] == '=') {
            p = malloc(len - key_len);
            if (p == NULL) {
                syslog(LOG_INFO, "Malloc memory error.\n");
                fclose(fp);
                return NULL;
            }
            strcpy(p, buf + key_len + 1);
        }
    }

    fclose(fp);

    return p;
}



// 4
#define LOCK_FILE "/var/run/jfind_ircs.pid"
#define LOCK_MODE (S_IRUSR|S_IWUSR|S_IRGRP)
int is_already_running(const char *lock_file, mode_t lock_mode) {
    int ret, fd;
    char buf[32];
    struct flock fl;

    fd = open(lock_file, O_RDWR|O_CREAT, lock_mode);
    if (fd < 0) {
#ifdef DEBUG
        syslog(LOG_INFO, "open lock file[%s] error.\n", lock_file);
#endif
        exit(1);
    }
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    ret = fcntl(fd, F_SETLK, &fl);
    if (ret) {
        /* already running or some error. */
        close(fd);
        return 1;
    }
    /* O.K. write the pid */
    ret = ftruncate(fd,0);
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)getpid());
    ret = write(fd, buf, strlen(buf) + 1);

    return 0;
}



// 5
#include <signal.h>
static void daemon_init() {
    int ret, i;
    pid_t pid;
    struct sigaction sa;

    ret = umask(0);
    pid = fork();
    if (pid < 0) {
        exit(1);
    } else if (pid != 0) {
        exit(0);
    }

    ret = setsid();
    if (ret == ((pid_t) -1)) {
        exit(1);
    }

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    ret = sigaction(SIGHUP, &sa, NULL);
    if (ret < 0) {
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        exit(1);
    } else if (pid != 0) {
        exit(0);
    }

    ret = chdir("/");
    if (ret) {
        exit(1);
    }
    return;
}


// 6
#include <stdlib.h>
#include <time.h>
char* genRandomString(char *string, int length)
{
    int flag, i;
    // char* string;
    if ( !string || length <= 0 )
        return NULL;
    srand((unsigned) time(NULL ));

    for (i = 0; i < length; i++)
    {
        flag = rand() % 3;
        switch (flag)
        {
            case 0:
                string[i] = 'A' + rand() % 26;
                break;
            case 1:
                string[i] = 'a' + rand() % 26;
                break;
            case 2:
                string[i] = '0' + rand() % 10;
                break;
            default:
                string[i] = 'x';
                break;
        }

    }
    string[length] = '\0';
    printf("random : %s", string);
    return string;
}



// 7
#include <ctype.h>
#include <stdint.h>
static void dump_line(FILE * out, const char *addr, const long len)
{
	int i;
	fprintf(out, "|");
	for (i = 0; i < 16; i++) {
		if (i < len) {
			fprintf(out, " %02X", ((uint8_t *) addr)[i]);
		} else {
			fprintf(out, " ..");
		}
		if (!((i + 1) % 8)) {
			fprintf(out, " |");
		}
	}
	fprintf(out, "\t");
	for (i = 0; i < 16; i++) {
		char c = 0x7f & ((uint8_t *) addr)[i];
		if (i < len && isprint(c)) {
			fprintf(out, "%c", c);
		} else {
			fprintf(out, ".");
		}
	}
}

void dump(FILE * out, const char *addr, const long len)
{
	int i;
	for (i = 0; i < len; i += 16) {
		dump_line(out, addr + i, (len - i) < 16 ? (len - i) : 16);
		fprintf(out, "\n");
	}
	fflush(out);
}











