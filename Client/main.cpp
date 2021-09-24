#include "client.hpp"
#include <thread>

using namespace std;

int main(int argc, const char* argv[]) {
    try {
        //if (argc != 3) {
        //    logger() << "Usage: client <host> <port>\n";
        //    return 1;
        //}

        asio::io_context context;

        tcp::resolver resolver(context);
//        auto endpoints = resolver.resolve(argv[1], argv[2]);
        auto endpoints = resolver.resolve("127.0.0.1", "1337");
        Client c(context, endpoints);

        thread t([&context] { context.run(); });

        string uname;
        do {
            cout << "Username (up to 31 characters): ";
            cin >> uname;
        } while (uname.length() > MAX_USERNAME_SIZE);

        c.write(build::auth(uname));

        char buffer[MAX_MSG_SIZE];
        while (cin.getline(buffer, MAX_MSG_SIZE)) {
            size_t len = strlen(buffer);
            if (len != 0)
                c.write(build::msg(buffer, len));
        }

        c.close();
        t.join();

    } catch (const std::exception &ex) {
        logger<LT::ERR>() << "Exception: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
