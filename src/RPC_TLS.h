#pragma once

#include <any>
#include <functional>
#include <hla/include_ssl.h>
#include <hla/server.h>
#include <list>
#include <unordered_map>

template<typename ID_t>
struct RPC_TLS
{
	using ssl_socket_t = asio::ssl::stream<asio::ip::tcp::socket>;

	RPC_TLS(asio::io_context& context, uint16_t port) :
		server(context, port)
	{
	}

	asio::ssl::context ssl_context{asio::ssl::context::tls_server};

	// RPC ID to function
	std::unordered_map<ID_t, std::function<void(ssl_socket_t&)>> functions;

	// accepts new connections
	hla::server server;

	// serves RPC requests
	struct connection
	{
		connection(RPC_TLS& manager, asio::ip::tcp::socket&& tcp) : manager(manager), socket(std::move(tcp), manager.ssl_context)
		{
			socket.handshake(ssl_socket_t::server);
		}

		ssl_socket_t socket;
		RPC_TLS& manager;

		bool stop = false;

		void serve()
		{
			RPC_id = 0;
			socket.async_read_some(asio::buffer(&RPC_id, sizeof(RPC_id)), [&](const hla::error_code& error, std::size_t bytes_transferred)
			{
				if (error == asio::error::eof ||
					error == asio::error::connection_reset) {
					manager.remove_connection(&socket);
					return;
				}

				// try to find a matching function
				auto function_iter = manager.functions.find(RPC_id);
				if (function_iter != manager.functions.end()) {
					function_iter->second(socket);
				}

				//TODO removing the connection needs to break this loop
				if (stop) {
					manager.remove_connection(&socket);
					return;
				}

				// after done reading the RPC data, wait for the next RPC
				serve();
			});
		}

	protected:
		ID_t RPC_id = 0;
	};

	std::list<connection> connections;
	std::unordered_map<ssl_socket_t*, typename decltype(connections)::iterator> socket_to_connection;


	void close_connection(ssl_socket_t* socket)
	{
		socket_to_connection.at(socket)->stop = true;
	}

	void remove_connection(ssl_socket_t* socket)
	{
		connections.erase(socket_to_connection.at(socket));
		socket_to_connection.erase(socket);
	}

	// listen to incoming RPC connections
	void accept()
	{
		using asio::ip::tcp;
		server.accept([&](tcp::socket& socket)
		{
			auto& conn = connections.emplace_back(*this, std::move(socket));
			socket_to_connection.emplace(&conn.socket, --connections.end());
			conn.serve();
		});
	}

	// provide hooking point for new connection event.
	// on_accept may return false to reject the connection.
	template<typename OnAccept>
	void accept(OnAccept&& on_accept)
	{
		using asio::ip::tcp;
		server.accept([&, on_accept = std::move(on_accept)](tcp::socket& socket)
		{
			auto& conn = connections.emplace_back(*this, std::move(socket));
			socket_to_connection.emplace(&conn.socket, --connections.end());

			if (!on_accept(conn.socket)) {
				remove_connection(&conn.socket);
				return;
			}

			conn.serve();
		});
	}
};