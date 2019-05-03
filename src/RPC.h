#pragma once

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

	// accepts new connections
	hla::server server;

	// serves RPC requests
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


	// listen to incoming RPC connections
	void accept()
	{
		using asio::ip::tcp;
		server.accept([&](tcp::socket& socket)
		{
			auto& conn = connections.emplace_back(std::move(socket));
			add_connection(conn);
		});
	}

	// provide hooking point for new connection event.
	// on_accept may return false to reject the connection.
	template<typename OnAccept>
	void accept(OnAccept&& on_accept)
	{
		using asio::ip::tcp;
		server.accept([&](tcp::socket& socket)
		{
			auto& conn = connections.emplace_back(std::move(socket));

			if (!on_accept(conn)) {
				connections.pop_back();
				return;
			}

			add_connection(conn);
		});
	}


protected:
	void add_connection(connection& conn);
};