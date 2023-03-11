#include <iostream>

//to brute force a delay:
//#include <chrono> 

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp> // will handle the movement of memory
#include <asio/ts/internet.hpp> // for network communication


std::vector<char> vBuffer(1 * 1024);

void GrabSomeData(asio::ip::tcp::socket& socket) {
	socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
		[&](std::error_code ec, std::size_t length) {
			if (!ec) {
				std::cout << "\n\nRead " << length << "bytes\n\n";
				for (int i = 0; i < length; i++)
					std::cout << vBuffer[i];
				GrabSomeData(socket);
			}
		}
	);
}

int main() {
	
	asio::error_code ec;

	// Create a 'context' - essentially the platform specific interface
	// The space where asio can do its work
	asio::io_context context;

	// Create some fake tasks to asio so the context doesn't finish
	// If asio doesn't have work to do, it just exits immediately
	asio::io_context::work idleWork(context);

	// Start the context
	std::thread thrContext = std::thread([&]() { context.run(); });

	//Get the address of somewhere we wish to connect to
	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);
	//127.0.0.1 is our ip
	//93.184.216.34 is the ip for example.com
	//It communicates on port 80: HTTP
	//Our endpoint is simply an address

	//Create a socket, the context will deliver the implementation
	asio::ip::tcp::socket socket(context);

	//Tell socket to try and connect
	socket.connect(endpoint, ec);

	if (!ec)
		std::cout << "Connected!" << std::endl;
	else
		std::cout << "Failed to connect to address:\n" << ec.message() << std::endl;

	if (socket.is_open()) {

		GrabSomeData(socket);

		std::string sRequest =
			"GET /index.html HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Connection: close\r\n\r\n";

		socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);
		//write_some = please try and send as much of this data as possible
		//but it TAKES TIME so we'll either have to
		//brute force the delay:
		//using namespace std::chrono_literals;
		//std::this_thread::sleep_for(200ms);
		//or we can use some asio tricks:
		socket.wait(socket.wait_read);

		//the manual, SYNCHRONOUS version of the GrabSomeData() function:

		//size_t bytes = socket.available();
		//std::cout << "Bytes available: " << bytes << std::endl;
		//if (bytes) {
		//	//i'm gonna read them from the socket
		//	std::vector<char> vBuffer(bytes);
		//	socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);
		//	for (auto c : vBuffer)
		//		std::cout << c;
		//}

		// but we want to request data from the server ASYNCHRONOUSLY:
		// GrabSomeData(socket);
		// we'll call this function at the beginning
		// so the program doesn't end before the request is made

		//Program does something else, while asio handles data transfer in background
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(2000ms);

	}

	return 0;
}