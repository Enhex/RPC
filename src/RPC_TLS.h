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

	RPC_TLS(asio::io_context& context, asio::ip::tcp::endpoint const& endpoint) :
		server(context, endpoint)
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
		connection(RPC_TLS& manager, asio::ip::tcp::socket&& tcp, hla::error_code& ec) :
			manager(manager),
			socket(std::move(tcp), manager.ssl_context)
		{
			socket.handshake(ssl_socket_t::server, ec);
		}

		RPC_TLS& manager;
		ssl_socket_t socket;

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
			// TLS handshake may fail
			hla::error_code ec;
			auto& conn = connections.emplace_back(*this, std::move(socket), ec);
			if(ec) {
				connections.pop_back();
			}
			else {
				socket_to_connection.emplace(&conn.socket, --connections.end());
				conn.serve();
			}
		});
	}

	// provide hooking point for new connection event.
	// on_accept may return false to reject the connection.
	template<typename OnAccept, typename OnFail>
	void accept(OnAccept&& on_accept, OnFail&& on_fail = [](hla::error_code&){})
	{
		using asio::ip::tcp;
		server.accept([&, on_accept = std::move(on_accept), on_fail = std::move(on_fail)](tcp::socket& socket)
		{
			hla::error_code ec;
			auto& conn = connections.emplace_back(*this, std::move(socket), ec);
			if(ec) {
				connections.pop_back();
				on_fail(ec);
				return;
			}

			socket_to_connection.emplace(&conn.socket, --connections.end());

			if (!on_accept(conn.socket)) {
				remove_connection(&conn.socket);
				return;
			}

			conn.serve();
		});
	}

	static void client_close_connection(ssl_socket_t& socket)
	{
		hla::error_code ec; // ignore errors
		socket.lowest_layer().cancel(ec);
		socket.shutdown(ec);
		socket.next_layer().shutdown(asio::ip::tcp::socket::shutdown_both, ec);
		socket.next_layer().close(ec);
	}
};