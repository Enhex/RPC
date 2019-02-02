#include <any>
#include <functional>
#include <hla/server.h>
#include <list>
#include <unordered_map>

struct RPC
{
	RPC(asio::io_context& context, uint16_t port);

	// RPC ID to function
	std::unordered_map<size_t, std::function<void(asio::ip::tcp::socket&)>> functions;

	hla::server server;

	struct connection : asio::ip::tcp::socket
	{
		using asio::ip::tcp::socket::socket;

		RPC* manager = nullptr;
		std::list<connection>::iterator self_iter; // used to self remove in O(1)
		void serve();

	protected:
		size_t RPC_id = 0;
	};

	std::list<connection> connections;
};