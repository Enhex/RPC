#include <RPC.h>
#include <RPC_TLS.h>

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

	// TLS
	{
		asio::io_context context;

		RPC_TLS<unsigned> rpc(context, 7357);
		rpc.ssl_context.set_verify_mode(asio::ssl::verify_peer);
		rpc.ssl_context.use_certificate_chain_file("cert.pem");
		rpc.ssl_context.use_private_key_file("key.pem", asio::ssl::context::pem);

		rpc.functions[rpc_id] = [](auto& socket){
			write_string(socket, "hello TLS!");
		};

		rpc.accept();
		auto ctx_future = std::async([&context] { context.run(); });

		// client side context
		asio::ssl::context ssl_context{asio::ssl::context::tls_client};
		ssl_context.set_verify_mode(asio::ssl::verify_peer);
		ssl_context.load_verify_file("cert.pem");

		using ssl_socket = asio::ssl::stream<asio::ip::tcp::socket>;
		ssl_socket socket(asio::ip::tcp::socket(context), ssl_context);
		socket.next_layer().connect(asio::ip::tcp::endpoint(hla::localhost_v4, port));

		socket.handshake(ssl_socket::client);

		write_bytes(socket, rpc_id);
		std::cout << read_string(socket) << std::endl;

		context.stop();
	}
}