#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

struct parm {
	char if1[256];
	char if2[256];
	int skip;
	int skip1;
	int skip2;
	int count;
	int width;
	int bs;
	int decimal;
};

void red_printf(uint8_t hex)
{
	printf("\033[41;37m%02x\033[0m", hex);
}

void usage()
{
	printf("\nUsage:\n\n"
			"\tif1=\n"
			"\tif2=\n"
			"\tcount=\n"
			"\tskip=\n"
			"\tskip1=\n"
			"\tskip2=\n"
			"\tbs=\n"
			"\twidth=\n"
			"\tsource code: https://github.com/handfourstone/hex_compare.git\n"
			"\n"
		);
}

bool is_a_number(const char *str, uint32_t *number)
{
	int i = 0;

	if (NULL == str) {
		return false;
	}

	for (i = 0; i < strlen(str); i++) {
		if (*(str + i) < '0' || *(str + i) > '9') {
			goto parse_unit;
		}
	}
	*number = atoi(str);
	return true;

parse_unit:
	if (0 == strcmp((str + i), "K")
			|| 0 == strcmp((str + i), "k")
			|| 0 == strcmp((str + i), "KB")
			|| 0 == strcmp((str + i), "kb")) {
		*number = atoi(str) * 1024;
		return true;
	} else if (0 == strcmp((str + i), "M")
			|| 0 == strcmp((str + i), "m")
			|| 0 == strcmp((str + i), "MB")
			|| 0 == strcmp((str + i), "mb")) {
		*number = atoi(str) * 1024 * 1024;
		return true;
	} else if (0 == strcmp((str + i), "G")
			|| 0 == strcmp((str + i), "g")
			|| 0 == strcmp((str + i), "GB")
			|| 0 == strcmp((str + i), "gb")) {
		*number = atoi(str) * 1024 * 1024 * 1024;
		return true;
	} else {
		printf("%s is a invalid number\n", str);
		return false;
	}
}

bool parse_argv(int argc, char *argv[], struct parm *parm)
{
	int i = 0, j = 0;
	uint32_t number;

	if (argc < 3) {
		usage();
		printf("if1 and if2 are required\n");
		return false;
	}

	for (i = 1; i < argc; i++) {
		if (0 == strcmp(argv[i], "help")) {
			return false;
		} else if (0 == strncmp(argv[i], "if1=", strlen("if1="))) {
			strcpy(parm->if1, (argv[i] + strlen("if1=")));
			continue;
		} else if (0 == strncmp(argv[i], "if2=", strlen("if2="))) {
			strcpy(parm->if2, (argv[i] + strlen("if2=")));
			continue;
		} else if (0 == strncmp(argv[i], "decimal=", strlen("decimal="))) {
			if (0 == strcmp(argv[i] + strlen("decimal="), "8")) {
				parm->decimal = 8;
				continue;
			} else if (0 == strcmp(argv[i] + strlen("decimal="), "10")) {
				parm->decimal = 10;
				continue;
			} else if (0 == strcmp(argv[i] + strlen("decimal="), "16")) {
				parm->decimal = 16;
				continue;
			} else {
				return false;
			}
		} else if (0 == strncmp(argv[i], "skip=", strlen("skip="))) {
			if (true == is_a_number(argv[i] + strlen("skip="), &number)) {
				parm->skip = number;
			} else {
				return false;
			}
		} else if (0 == strncmp(argv[i], "skip1=", strlen("skip1="))) {
			if (true == is_a_number(argv[i] + strlen("skip1="), &number)) {
				parm->skip1 = number;
				continue;
			} else {
				return false;
			}
		} else if (0 == strncmp(argv[i], "skip2=", strlen("skip2="))) {
			if (true == is_a_number(argv[i] + strlen("skip2="), &number)) {
				parm->skip2 = number;
				continue;
			} else {
				return false;
			}
		} else if (0 == strncmp(argv[i], "count=", strlen("count="))) {
			if (true == is_a_number(argv[i] + strlen("count="), &number)) {
				parm->count = number;
				continue;
			} else {
				return false;
			}
		} else if (0 == strncmp(argv[i], "width=", strlen("width="))) {
			if (true == is_a_number(argv[i] + strlen("width="), &number)) {
				parm->width = number;
				continue;
			} else {
				return false;
			}
		} else if (0 == strncmp(argv[i], "bs=", strlen("bs="))) {
			if (true == is_a_number(argv[i] + strlen("bs="), &number)) {
				parm->bs = number;
				parm->skip *= parm->bs;
				parm->skip1 *= parm->bs;
				parm->skip2 *= parm->bs;
				parm->count *= parm->bs;
				continue;
			} else {
				return false;
			}
		} else {
			return false;
		}
	}
	return true;
}

bool if_valid_parm(struct parm *parm)
{
	struct stat st;
	uint32_t if1_size;
	uint32_t if2_size;

	if (NULL == parm->if1) {
		printf("if1 is required\n");
	}

	if (NULL == parm->if2) {
		printf("if2 is required\n");
	}

	if (0 != access(parm->if1, F_OK)) {
		printf("can't find %s\n", parm->if1);
		return false;
	}

	if (0 != access(parm->if2, F_OK)) {
		printf("can't find %s\n", parm->if2);
		return false;
	}

	if (-1 == stat(parm->if1, &st)) {
		printf("stat %s error\n", parm->if1);
		return false;
	}
	if1_size = st.st_size;

	if (-1 == stat(parm->if2, &st)) {
		printf("stat %s error\n", parm->if2);
		return false;
	}
	if2_size = st.st_size;

	if (parm->skip + parm->skip1 + parm->count > if1_size) {
		printf("%s size(%d) less than [skip(%d) + skip1(%d) + count(%d)]\n", parm->if1, if1_size, (int)parm->skip, (int)parm->skip1, (int)parm->count);
		return false;
	}

	if (parm->skip + parm->skip2 + parm->count > if2_size) {
		printf("%s size(%d) less than [skip(%d) + skip2(%d) + count(%d)]\n", parm->if2, if2_size, (int)parm->skip, (int)parm->skip2, (int)parm->count);
		return false;
	}
	return true;
}

void printf_format(char *buf1, int count1, char *buf2, int count2, struct parm *parm)
{
	int i = 0, j = 0;
	unsigned int line_len;
	char cmd[128] = {0};
	char tmp[32] = {0};
	int min_count;
	int cmd_len;
	int end1 = 0, end2 = 0;

	min_count = count1 > count2 ? count2 : count1;

	if (0 == parm->decimal) {
		parm->decimal = 16;
	}

	if (0 == parm->width) {
		parm->width = 16;
	}

	sprintf(cmd, "%s", "printf ");

	switch (parm->decimal) {
		case 8:
			sprintf(tmp, "%o", ((min_count % parm->width == 0) ? (min_count / parm->width) : (min_count / parm->width + 1)));
			line_len = strlen(tmp);
			sprintf(cmd + strlen(cmd), " %%0%do ", line_len);
			break;
		case 10:
			sprintf(tmp, "%d", ((min_count % parm->width == 0) ? (min_count / parm->width) : (min_count / parm->width + 1)));
			line_len = strlen(tmp);
			sprintf(cmd + strlen(cmd), " %%0%dd ", line_len);
			break;
		case 16:
			sprintf(tmp, "%x", ((min_count % parm->width == 0) ? (min_count / parm->width) : (min_count / parm->width + 1)));
			line_len = strlen(tmp);
			sprintf(cmd + strlen(cmd), " %%0%dx ", line_len);
			break;
		default:
			return;
	}

	cmd_len = strlen(cmd);

	for (i = 0; i < ((min_count % parm->width == 0) ? (min_count / parm->width) : (min_count / parm->width + 1)); i++) {
		sprintf(cmd + cmd_len, " %d", i);
		printf(": ");
		system(cmd);
		for (j = 0; j < parm->width; j++) {
			if (i * parm->width + j >= min_count) {
				for (; j < parm->width; j++) {
					printf("   ");
				}
				break;
			}
			if ( *(buf1 + i * parm->width + j) != *(buf2 + i * parm->width + j)) {
				red_printf(*(buf1 + i * parm->width + j));
				printf(" ");
			} else {
				printf("%02x ", *(buf1 + i * parm->width + j));
			}
		}
		printf(" | ");
		for (j = 0; j < parm->width; j++) {
			if (i * parm->width + j >= min_count) {
				break;
			}
			if ( *(buf1 + i * parm->width + j) != *(buf2 + i * parm->width + j)) {
				red_printf(*(buf2 + i * parm->width + j));
				printf(" ");
			} else {
				printf("%02x ", *(buf2 + i * parm->width + j));
			}
		}
		printf("\n");
	}
	return;
}

void compare(struct parm *parm)
{
	FILE *fp1, *fp2;
	struct stat stat1;
	struct stat stat2;
	int off1, off2;
	int count1, count2;
	char *if1_buf, *if2_buf;
	int i;

	if (-1 == stat(parm->if1, &stat1)) {
		printf("stat %s error\n", parm->if1);
		return;
	}

	if (-1 == stat(parm->if2, &stat2)) {
		printf("stat %s error\n", parm->if2);
		return;
	}

	off1 = parm->skip + parm->skip1;
	off2 = parm->skip + parm->skip2;

	if (0 == parm->count) {
		count1 = stat1.st_size - off1;
		count2 = stat2.st_size - off2;
	} else {
		count1 = parm->count;
		count2 = parm->count;
	}

	if (NULL == (fp1 = fopen(parm->if1, "r"))) {
		printf("fopen %s error\n", parm->if1);
		return;
	}

	if (NULL == (fp2 = fopen(parm->if2, "r"))) {
		fclose(fp1);
		printf("fopen %s error\n", parm->if2);
		return;
	}

	if (NULL == (if1_buf = malloc(count1))) {
		fclose(fp1);
		fclose(fp2);
		printf("malloc for if1 error\n");
		return;
	}

	if (NULL == (if2_buf = malloc(count2))) {
		fclose(fp1);
		fclose(fp2);
		free(if1_buf);
		printf("malloc for if2 error\n");
		return;
	}

	fseek(fp1, off1, SEEK_SET);
	fseek(fp2, off2, SEEK_SET);

	fread(if1_buf, 1, count1, fp1);
	fread(if2_buf, 1, count2, fp2);

	fclose(fp1);
	fclose(fp2);

	printf_format(if1_buf, count1, if2_buf, count2, parm);

	free(if1_buf);
	free(if2_buf);
}

int main(int argc, char *argv[])
{
	int index;
	struct parm *parm;

	parm = malloc(sizeof(struct parm));
	if (NULL == parm) {
		exit(0);
	}
	memset(parm, 0, sizeof(struct parm));

	if (false == parse_argv(argc, argv, parm)) {
		exit(0);
	}

	if (false == if_valid_parm(parm)) {
		exit(0);
	}
	compare(parm);
	return 0;
}
