#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/supplemental/util/platform.h>

#ifndef PARALLEL
#define PARALLEL 128
#endif

#ifndef SLEEP_TIME
#define SLEEP_TIME 500
#endif

struct work {
	enum {INIT, RECV, WAIT, SEND}	state;
	nng_aio				*aio;
	nng_msg				*msg;
	nng_ctx				ctx;
};

int64_t
random_int(void)
{
	return rand() - (rand() / 2);
}	// to get full range of int range [min int; max int]

void
fatal(const char *func, int rv)
{
	fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
	exit(1);
}

void
server_cb(void *arg)
{
	struct work	*work = arg;
	nng_msg		*msg;
	int		rv;

	switch (work->state) {
	case INIT:
		work->state = RECV;
		nng_ctx_recv(work->ctx, work->aio);
		break;
	case RECV:
		if ((rv = nng_aio_result(work->aio)) != 0) {
			fatal("nng_ctx_recv", rv);
		}
		msg = nng_aio_get_msg(work->aio);
		if(rv = nng_msg_alloc(&work->msg, 0)) {
			fatal("nng_msg_alloc", rv);
		}
		uint16_t numbers_count = rand() % 20 + 1;
		fprintf(stdout, "numbers_count is:  %d\n", numbers_count);
		nng_msg_append_u16(work->msg, (numbers_count));
		//u8 is unsupported in this library, used u16 instead
		for (uint16_t i = 0; i < numbers_count; i++) {
			int64_t cur_number = random_int();
			nng_msg_append_u64(work->msg, cur_number);
			fprintf(stdout, "wrote %d\n", cur_number);
		}
		work->state = WAIT;
		nng_sleep_aio(SLEEP_TIME, work->aio);
		break;
	case WAIT:
		nng_aio_set_msg(work->aio, work->msg);
		work->state = SEND;
		nng_ctx_send(work->ctx, work->aio);
		break;
	case SEND:
		if ((rv = nng_aio_result(work->aio)) != 0) {
			nng_msg_free(work->msg);
			fatal("nng_ctx_send", rv);
		}
		work->state = RECV;
		nng_ctx_recv(work->ctx, work->aio);
		break;
	default:
		fatal("bad state!", NNG_ESTATE);
		break;
	}
}

struct work *
alloc_work(nng_socket sock)
{
	struct work	*w;
	int		rv;

	if ((w = nng_alloc(sizeof(*w))) == NULL) {
		fatal("nng_alloc", NNG_ENOMEM);
	}
	if ((rv = nng_aio_alloc(&w->aio, server_cb, w)) != 0) {
		fatal("nng_aio_alloc", rv);
	}
	if ((rv = nng_ctx_open(&w->ctx, sock)) != 0) {
		fatal("nng_ctx_open", rv);
	}
	w->state = INIT;
	return (w);
}

int
server(const char *url)
{
	nng_socket	sock;
	struct work	*works[PARALLEL];
	int		rv;
	int		i;

	rv = nng_rep0_open(&sock);
	if (rv != 0) {
		fatal("nng_rep0_open", rv);
	}
	for (i = 0; i < PARALLEL; i++) {
		works[i] = alloc_work(sock);
	}
	if ((rv = nng_listen(sock, url, NULL, 0)) != 0) {
		fatal("nng_listen", rv);
	}
	for (i = 0; i < PARALLEL; i++) {
		server_cb(works[i]);
	}
	for (;;) {
		nng_msleep(3600000);//neither pause() nor sleep() portable
	}
}

int
main(int argc, char **argv)
{
	srand(time(0));
	int rc;

	if (argc == 2) {
		fprintf(stdout, "Get adress: \"%s\"", argv[1]);
		rc = server(argv[1]);
	} else {
		fprintf(stderr, "Specify only one adress in format <host:port>");
		exit(EXIT_FAILURE);
	}
	exit(rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
