#include "RPC.h"

size_t test = 0;

RPC::RPC(asio::io_context & context, uint16_t port) :
	server(context, port)
{
	// listen to incoming RPCs
	using asio::ip::tcp;
	server.accept([&](tcp::socket& socket)
	{
		auto& conn = connections.emplace_back(std::move(socket));
		conn.self_iter = --connections.end();
		conn.manager = this;
		conn.serve();
	});
}


void RPC::connection::serve()
{
	async_receive(asio::buffer(&RPC_id, sizeof(RPC_id)), [&](const hla::error_code& error, std::size_t bytes_transferred)
	{
		if (error == asio::error::eof ||
			error == asio::error::connection_reset) {
			manager->connections.erase(self_iter);
			return;
		}

		// try to find a matching function
		auto function_iter = manager->functions.find(RPC_id);
		if (function_iter != manager->functions.end()) {
			function_iter->second(*this);
		}

		// after done reading the RPC data, wait for the next RPC
		serve();
	});
}