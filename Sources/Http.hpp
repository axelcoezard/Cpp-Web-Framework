#pragma once

#include <iostream>
#include <functional>
#include <map>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>

typedef boost::beast::http::request<boost::beast::http::string_body> HttpRequest;
typedef boost::beast::http::response<boost::beast::http::string_body> HttpResponse;
typedef boost::beast::http::status HttpStatus;
typedef boost::beast::http::field HttpField;
typedef boost::beast::http::verb HttpMethod;

typedef std::string HttpRoute;
typedef std::function<HttpStatus(HttpRequest&, HttpResponse&)> HttpCallback;

struct CorsOptions
{
	std::string origin;
	std::string methods;
	std::string headers;
	std::string maxAge;
};

class HttpServer
{
private:
	/**
	 * @brief A map of all routes. Each route can have a callback for each http method.
	 */
	std::map<HttpRoute, std::map<HttpMethod, HttpCallback>> m_routes;

	/**
	 * @brief The Boost::Asio's IO Context used to create the TCP Acceptor.
	 */
	boost::asio::io_context m_ioContext;

	/**
	 * @brief The CORS options
	 */
	CorsOptions m_corsOptions;

public:

	/**
	 * @brief Set the CORS options
	 *
	 * @param options The CORS options
	 */
	void SetCors(CorsOptions options)
	{
		m_corsOptions = options;
	}

	/**
	 * @brief Register a callback for a GET request on the indicated route
	 *
	 * @param route The route
	 * @param callback The callback. It will be called using the request and the response.
	 */
	void Get(HttpRoute route, HttpCallback callback)
	{
		AddRoute(HttpMethod::get, route, callback);
	}

	/**
	 * @brief Register a callback for a POST request on the indicated route
	 *
	 * @param route The route
	 * @param callback The callback. It will be called using the request and the response.
	 */
	void Post(HttpRoute route, HttpCallback callback)
	{
		AddRoute(HttpMethod::post, route, callback);
	}

	/**
	 * @brief Starts listening for http requests on the indicated port.
	 *
	 * @param port The port
	 */
	void Listen(uint16_t port)
	{
		// Create and bind the acceptor
		boost::asio::ip::tcp::acceptor acceptor(m_ioContext, {
			boost::asio::ip::tcp::v4(),
			port
		});

		while (true)
		{
			// Accept incoming connections
			boost::asio::ip::tcp::socket socket(m_ioContext);
			acceptor.accept(socket);

			// Read the HTTP request
			HttpRequest req;
			boost::beast::flat_buffer buffer;
			boost::beast::http::read(socket, buffer, req);

			// Prepare the HTTP response with default params
			HttpResponse res;
			res.keep_alive(req.keep_alive());
			res.version(req.version());

			// Handle CORS preflight requests
			if (req.method() == HttpMethod::options)
			{
				res.set(HttpField::access_control_allow_origin, m_corsOptions.origin);
				res.set(HttpField::access_control_allow_methods, m_corsOptions.methods);
				res.set(HttpField::access_control_allow_headers, m_corsOptions.headers);
				res.set(HttpField::access_control_max_age, m_corsOptions.maxAge);
				res.result(HttpStatus::ok);

				res.prepare_payload();
				boost::beast::http::write(socket, res);
				continue;
			}

			// Try to find the route's callback
			HttpCallback callback = FindRoute(req);
			res.result(callback != nullptr ? callback(req, res) : HttpStatus::not_found);

			// Set the origin header
			auto originHeader = req.find(HttpField::origin);
			if (originHeader != req.end())
			{
				res.set(HttpField::access_control_allow_origin, originHeader->value());
			}

			// Write the response back to the client
			res.prepare_payload();
			boost::beast::http::write(socket, res);
		}
	}

private:
	void AddRoute(HttpMethod method, HttpRoute route, HttpCallback callback)
	{
		if (m_routes.find(route) == m_routes.end())
			m_routes[route] = std::map<HttpMethod, HttpCallback>();

		m_routes[route][method] = callback;
	}

	HttpCallback FindRoute(HttpRequest& request)
	{
		auto route = m_routes.find((HttpRoute) request.target());
		if (route == m_routes.end())
			return nullptr;

		auto routeData = route->second.find(request.method());
		if (routeData == route->second.end())
			return nullptr;

		return routeData->second;
	}
};
