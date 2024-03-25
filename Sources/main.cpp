#include <iostream>
#include <string>

#include "Http.hpp"
#include <nlohmann/json.hpp>

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

	std::string value = "Hello, World!"; // default value

	server.Get("/planning", [&value](HttpRequest& req, HttpResponse& res) {
		res.set("Content-Type", "application/json");
		res.body() = "{\"value\": \"" + value + "\"}";
		return HttpStatus::ok;
	});

	server.Post("/planning", [&value](HttpRequest& req, HttpResponse& res) {
		// Parse the json body
		nlohmann::json result;
		try {
			result = nlohmann::json::parse(req.body());
		} catch (nlohmann::json::parse_error& e) {
			return HttpStatus::bad_request;
		}

		// Check if the json body contains the key "value"
		std::string localValue = result.at("value").get<std::string>();
		if (localValue.empty())
			return HttpStatus::bad_request;

		value = std::move(localValue);

		// Send the response
		res.set("Content-Type", "application/json");
		res.body() = "{\"value\": \"" + value + "\"}";

		return HttpStatus::ok;
	});

	// Should be called last
	server.Listen(std::atoi(argv[1]));

	return EXIT_SUCCESS;
}
