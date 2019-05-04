#include <RPC.h>

int main()
{
	asio::io_context context;
	RPC<int> rpc(context, 7357);
}
