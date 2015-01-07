#include <universe.h>

int main(int argc, char **argv)
{
	printf("started testserver with pid %d\n", getpid());
	
	open_port(5);

	while(1)
	{
		int req_id = uread_port(5);

		printf("gotten request %d\n", req_id);
		uaccept(req_id);
		printf("accepted.\n");
	}
}
