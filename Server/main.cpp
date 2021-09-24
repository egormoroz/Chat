#include "server.hpp"
#include "logger.hpp"

int main() {
	try {
		asio::io_context io_context;
		tcp::endpoint ep(tcp::v4(), 1337);
		Server server(io_context, ep);

		io_context.run();
	} catch (const std::exception &ex) {
		logger<LT::ERR>() << "Exception: " << ex.what() << "\n";
	}
}
