#include <iostream>
#include <string>

#include "Http.hpp"

int main(int argc, char** argv)
{
	// Check if the user has provided the port
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
		return EXIT_FAILURE;
	}

	// Create an HttpServer
	HttpServer server;

	// Set the CORS options
	server.SetCors({
		.origin = "*",
		.methods = "GET, POST", // TODO: make it dynamic depending on the routes
		.headers = "Content-Type",
		.maxAge = "3600"
	});

	server.Post("/planning", [](HttpRequest& req, HttpResponse& res) {
		res.set(HttpField::content_type, "text/plain");
		res.body() = "{\"message\": \"Hello, World!\"}";
		return HttpStatus::ok;
	});

	// Should be called last
	server.Listen(std::atoi(argv[1]));

	return EXIT_SUCCESS;
}
