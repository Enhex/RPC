#include <RPC.h>

int main()
{
	asio::io_context context;
	RPC rpc(context, 7357);
}
