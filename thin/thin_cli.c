#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include "payload.h"

static void usage(char *);

int
main(int argc, char *argv[]) {
	struct payload message;
	struct thin_conn_handle *ch;
	int arg;
	int opt_idx = 0, flag = 1;
	int ret;

	const struct option longopts[] = {
		{ "add", required_argument, NULL, 0 },
		{ "del", required_argument, NULL, 0 },
		{ 0, 0, 0, 0 }
	};

	init_payload(&message);
	message.type = PAYLOAD_CLI;

	/* We expect at least one valid option and, if more, the others
	   are discarded
	*/
	while ((arg = getopt_long(argc, argv, "h",
				  longopts, &opt_idx)) != -1 && flag) {
		switch(arg) {
		case 0:
			/* master: it is fine to have a string with trailing spaces */
			ret = snprintf(message.path, PAYLOAD_MAX_PATH_LENGTH,
				       "%s %s", longopts[opt_idx].name, optarg);
			if (ret >= PAYLOAD_MAX_PATH_LENGTH) {
				fprintf(stderr, "input too long\n");
				return 2;
			}
			flag = 0;
			break;
		case 's':
			ret = snprintf(message.path, IP_MAX_LEN, "%s %s",
				       longopts[opt_idx].name, optarg);
			if (ret >= IP_MAX_LEN) {
				fprintf(stderr, "input too long\n");
				return 2;
			}
			flag = 0;
			break;
		case 'h':
			usage(argv[0]);
			return 0;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	/* there must be at least one valid option */
	if(flag) {
		usage(argv[0]);
		return 1;
	}

	ch = thin_connection_create();
	if (ch == NULL) {
		fprintf(stderr, "connection initialization failed");
		return 1;
	}	
	ret = thin_sync_send_and_receive(ch, &message);
	if(ret) {
		fprintf(stderr, "socket error (%d)\n", ret);
		return 1;
	}
	if(message.err_code == THIN_ERR_CODE_SUCCESS)
		printf("message: ok\n");
	else
		printf("message: fail\n"); 

	thin_connection_destroy(ch);
	return 0;
}

static void
usage(char *prog_name)
{
	printf("usage: %s -h\n", prog_name);
	printf("usage: %s --add <volume group name>\n", prog_name);
	printf("usage: %s --del <volume group name>\n", prog_name);
}
