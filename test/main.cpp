#include <RPC.h>

#include <future>
#include <hla/object.h>
#include <hla/string.h>
#include <hla/utility.h>
#include <iostream>

constexpr uint16_t port = 7357;

int main()
{
	unsigned rpc_id = 1;

	// plaintext
	{
		asio::io_context context;
		RPC<unsigned> rpc(context, port);

		rpc.functions[rpc_id] = [](auto& socket){
			write_string(socket, "hello world!");
		};

		rpc.accept();
		auto ctx_future = std::async([&context] { context.run(); });

		asio::ip::tcp::socket socket(context);
		socket.connect(asio::ip::tcp::endpoint(hla::localhost_v4, port));

		write_bytes(socket, rpc_id);
		std::cout << read_string(socket) << std::endl;

		context.stop();
	}
}