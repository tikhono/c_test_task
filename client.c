#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/supplemental/util/platform.h>

#ifndef PRCS //prescision
# define PRCS 4
#endif

void
fatal(const char *func, int rv)
{
	fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
	exit(1);
}

void
process(int16_t numbers_count, int64_t *number)
{
	for (int i = 0; i < numbers_count; i++) {
		if (number[i] == INT_MIN)
		{
			fprintf(stdout, "3037000499.9760 * i");
			//Because absolute value of INT_MIN is unrepresentable without using extentions
			//and there is a need to get appropriate value for this case
			//I decided to use hardcoded value.
			//Another approach is to use INT_MAX for both INT_MAX and absolute
			//value of INT_MIN because with current precision there is the same
			//result for both of them.
			//fprintf(stdout, "%.4f * i\n",
			//		round(sqrt(INT_MAX) * pow(10, PRCS - 1)) / pow(10, PRCS - 1));
			//or
			//fprintf(stdout, "%.4f * i\n",
			//		round(sqrt(abs(number[i] + 1)) * pow(10, PRCS - 1)) / pow(10, PRCS - 1));
		} else if (number[i] < 0) {
			fprintf(stdout, "%.4f * i\n",
					round(sqrt(llabs(number[i])) * pow(10, PRCS - 1)) / pow(10, PRCS - 1));
		} else {
			fprintf(stdout, "%.4f\n",
					round(sqrt(number[i]) * pow(10, PRCS - 1)) / pow(10, PRCS - 1));
		}
	}
}

int
parse_server(const char *url)
{
	nng_socket sock;
	int        rv;
	nng_msg *  msg;
	unsigned   msec;

	if ((rv = nng_req0_open(&sock)) != 0) {
		fatal("nng_req0_open", rv);
	}
	if ((rv = nng_dial(sock, url, NULL, 0)) != 0) {
		fatal("nng_dial", rv);
	}
	if ((rv = nng_msg_alloc(&msg, 0)) != 0) {
		fatal("nng_msg_alloc", rv);
	}
	if ((rv = nng_sendmsg(sock, msg, 0)) != 0) {
		fatal("nng_send", rv);
	}
	if ((rv = nng_recvmsg(sock, &msg, 0)) != 0) {
		fatal("nng_recvmsg", rv);
	}
	uint16_t	numbers_count = 0;
	nng_msg_trim_u16(msg, &numbers_count);
	if (numbers_count < 0) {
		fprintf(stderr, "Numbers count cannot be 0 or less\n");
		exit(EXIT_FAILURE);
	}
	int64_t		number[numbers_count];
	int64_t		cur_number = 0;
	for (uint16_t i = 0; i < numbers_count; i++) {
		nng_msg_trim_u64(msg, &cur_number);
		number[i] = cur_number;
	}
	process(numbers_count, number);
	nng_msg_free(msg);
	nng_close(sock);
	return (0);
}

void
parse_file(FILE *in_file)
{
	int16_t	numbers_count = 0;
	if (fscanf(in_file, "%d", &numbers_count) != 1) {
		fprintf(stderr, "Unable to read values\n");
		fclose(in_file);
		exit(EXIT_FAILURE);
	}
	if (numbers_count < 1 || numbers_count > 20) {
		fprintf(stderr, "Numbers count must be in range [1; 20]\n");
		fclose(in_file);
		exit(EXIT_FAILURE);
	}
	int64_t		number[numbers_count];
	int64_t		cur_number = 0;
	for (uint16_t i = 0; i < numbers_count; i++) {
		if (fscanf(in_file, "%ld", &cur_number) != 1) {
			fprintf(stderr, "Not enough data\n");
			fclose(in_file);
			exit(EXIT_FAILURE);
		}
		number[i] = cur_number;
	}
	fclose(in_file);
	process(numbers_count, number);
}

int
main(int argc, char **argv)
{
	int rc;

	if (argc == 2) {
		fprintf(stdout, "Get descriptor: \"%s\"\n", argv[1]);
		FILE *in_file = fopen(argv[1], "r");
		if (in_file != NULL) {
			fprintf(stdout, "File was found\n");
			parse_file(in_file);
		} else {
			fprintf(stdout, "No file was found, proceed to connect to the server\n");
			rc = parse_server(argv[1]);
		}	//add another source of data here
	} else {
		fprintf(stderr, "Specify only one file or adress in format <host:port>\n");
		exit(EXIT_FAILURE);
	}
	exit(rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
