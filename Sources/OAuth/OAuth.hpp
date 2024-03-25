#pragma once

#include <string>
#include <map>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace OAuth
{
	struct ProviderDetails
	{
		std::string clientId;
		std::string clientSecret;
		std::string redirectUri;
		std::string urlAuthorize;
		std::string urlAccessToken;
		std::string urlResourceOwnerDetails;
	};

	struct ProviderFetchDetails
	{
		std::string method;
		std::string body;
		std::map<std::string, std::string> headers;
	};

	struct Token
	{
		std::string accessToken;
		std::string tokenType;
		std::string refreshToken;
		std::string resourceOwnerId;
		uint32_t expires;
	};

	class ProviderException : public std::exception
	{
	public:
		ProviderException(const std::string& message) : m_message(message) {}

		virtual const char* what() const noexcept override {
			return m_message.c_str();
		}

	private:
		std::string m_message;
	};

	class Provider
	{
	protected:
		ProviderDetails m_informations;
	public:
		Provider(ProviderDetails informations): m_informations(informations)
		{
			curl_global_init(CURL_GLOBAL_ALL);
		}

		~Provider()
		{
			curl_global_cleanup();
		}

		/**
		 * @brief Get the Access Token object using the client credentials and access token URL
		 *
		 * @throws ProviderException
		 * 	<li>If the curl session failed to initialize</li>
		 * 	<li>If the fetch failed</li>
		 * 	<li>If the access token is not found in the response</li>
		 * 	<li>If the token type is not found in the response</li>
		 * 	<li>If the refresh token is not found in the response</li>
		 * 	<li>If the resource owner ID is not found in the response</li>
		 * 	<li>If the expires in is not found in the response</li>
		 *
		 * @return Token& The access token object
		 */
		Token& GetAccessToken()
		{
			CURL* curl = curl_easy_init();

			if(curl == nullptr)
				throw ProviderException("Failed to initialize curl");

			curl_easy_setopt(curl, CURLOPT_URL, m_informations.urlAccessToken.c_str());
			curl_easy_setopt(curl, CURLOPT_POST, 1);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "grant_type=client_credentials");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, "Content-Type: application/json");
			curl_easy_setopt(curl, CURLOPT_USERPWD, (m_informations.clientId + ":" + m_informations.clientSecret).c_str());

			std::string responseData;
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void *buffer, size_t size, size_t nmemb, void *userp) -> size_t {
				auto responseData = static_cast<std::string*>(userp);
				responseData->append(static_cast<char*>(buffer), size * nmemb);
				return size * nmemb;
			});
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

			CURLcode response = curl_easy_perform(curl);
			curl_easy_cleanup(curl);

			if (response != CURLE_OK)
				throw ProviderException("Failed to fetch access token");

			nlohmann::json parsedResponse = nlohmann::json::parse(responseData);

			if (parsedResponse.find("access_token") == parsedResponse.end())
				throw ProviderException("Failed to parse: 'access_token'");

			if (parsedResponse.find("token_type") == parsedResponse.end())
				throw ProviderException("Failed to parse: 'token_type'");

			if (parsedResponse.find("refresh_token") == parsedResponse.end())
				throw ProviderException("Failed to parse: 'refresh_token'");

			if (parsedResponse.find("resource_owner_id") == parsedResponse.end())
				throw ProviderException("Failed to parse: 'resource_owner_id'");

			if (parsedResponse.find("expires_in") == parsedResponse.end())
				throw ProviderException("Failed to parse: 'expires_in'");

			Token token = {
				.accessToken = parsedResponse.at("access_token").get<std::string>(),
				.refreshToken = parsedResponse.at("refresh_token").get<std::string>(),
				.resourceOwnerId = parsedResponse.at("resource_owner_id").get<std::string>(),
				.expires = parsedResponse.at("expires_in").get<uint32_t>()
			};

			return token;
		}

		/**
		 * @brief Fetch data from the provider using the access token and the URL
		 *
		 * @param token The access token (returned by GetAccessToken())
		 * @param url The URL to fetch
		 * @param details The fetch details, like the method, body and headers (optional)
		 *
		 * @throws ProviderException
		 * 	<li>If the curl session failed to initialize</li>
		 * 	<li>If the fetch failed</li>
		 *
		 * @return std::string
		 */
		std::string Fetch(const Token& token, const std::string& url, ProviderFetchDetails details = {})
		{
			CURL* curl = curl_easy_init();

			if(curl == nullptr)
				throw ProviderException("Failed to initialize curl");

			// Set the URL to fetch
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

			// Set the default headers
			struct curl_slist *headers = NULL;
			headers = curl_slist_append(headers, "Content-Type: application/json");
			headers = curl_slist_append(headers, ("Authorization: " + token.tokenType + " " + token.accessToken).c_str());

			// Set the custom headers if any
			for (auto& [key, value] : details.headers)
				headers = curl_slist_append(headers, (key + ": " + value).c_str());

			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

			// If the method is POST, set the body
			if (details.method == "POST")
			{
				curl_easy_setopt(curl, CURLOPT_POST, 1);
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, details.body.c_str());
			}

			// Retrieve the response
			std::string responseData;
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void *buffer, size_t size, size_t nmemb, void *userp) -> size_t {
				auto responseData = static_cast<std::string*>(userp);
				responseData->append(static_cast<char*>(buffer), size * nmemb);
				return size * nmemb;
			});
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

			// Perform the request and end the curl session
			CURLcode response = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			curl_slist_free_all(headers);

			if (response != CURLE_OK)
				throw ProviderException("Failed to fetch data");

			return responseData;
		}

		std::string GetClientId()
		{
			return m_informations.clientId;
		}

		std::string GetClientSecret()
		{
			return m_informations.clientSecret;
		}

		std::string GetRedirectUri()
		{
			return m_informations.redirectUri;
		}

		std::string GetAccessTokenUrl()
		{
			return m_informations.urlAccessToken;
		}

		std::string GetResourceOwnerDetailsUrl()
		{
			return m_informations.urlResourceOwnerDetails;
		}
	};
}
