#include "RPC.h"
#include <hla/object.h>
#include <hla/utility.h>
#include <iostream>
#include <thread>

constexpr uint16_t rpc_port = 7357;

using RPC_ID = uint32_t;

constexpr RPC_ID f_id = 1;

void f(asio::ip::tcp::socket& socket)
{
	auto x = read_bytes<int>(socket);
	std::cout << "f(" << x << ")\n";
}

int main()
{
	using asio::ip::tcp;

	asio::io_context context;

	RPC<RPC_ID> rpc(context, rpc_port);
	rpc.accept();

	// register RPC
	rpc.functions[f_id] = f;

	auto threads = hla::thread_pool_run(context);

	// call RPC via TCP
	tcp::socket rpc_client(context);
	hla::connect(rpc_client, "127.0.0.1", rpc_port);
	RPC_ID rpc_call_id = 1;

	for (int n = 5; n-- > 0;)
	{
		write_bytes(rpc_client, rpc_call_id);
		write_bytes(rpc_client, int(n));

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	rpc_client.shutdown(rpc_client.shutdown_both);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	context.stop();

	hla::join_threads(threads);
}