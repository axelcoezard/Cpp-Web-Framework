# Module: OAuth

```cpp

OAuth::Provider provider = OAuth::Provider({
	.clientId = "<client_id>",
	.clientSecret = "<client_secret>",
	.redirectUri = "<redirect_uri>",
	.urlAuthorize = "<authorization_url>",
	.urlAccessToken = "<access_token_url>",
	.urlResourceOwnerDetails = "<resource_owner_details_url>"
});

OAuth::Token token = provider.GetAccessToken();

provider.Fetch(token, "<resource_owner_details_url>", {
	.method = "PUT",
	.body = {
		{ "key1", "value1" },
		{ "key2", "value2" }
	},
	.headers = {
		{ "key1", "value1" },
		{ "key2", "value2" }
	}
});

```
