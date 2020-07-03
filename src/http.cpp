/*
  The Forgotten Client
  Copyright (C) 2020 Saiyans King

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

//It is simple class designed to handle Store images nothing more advanced

#include "http.h"

#include <curl/curl.h>

Http g_http;

size_t write_data(void* ptr, size_t size, size_t nmemb, SDL_RWops* stream)
{
	return SDL_RWwrite(stream, ptr, size, nmemb);
}

void deleteRequest(Uint32 internalId, Sint32)
{
	g_http.removeRequest(internalId, false);
}

Http::Http()
{
	m_remainingHandles = 0;
	m_curlHandle = SDL_reinterpret_cast(void*, curl_multi_init());

	m_curlJsonHeader = NULL;
	m_curlJsonHeader = SDL_reinterpret_cast(void*, curl_slist_append(SDL_reinterpret_cast(curl_slist*, m_curlJsonHeader), "Accept: application/json"));
	m_curlJsonHeader = SDL_reinterpret_cast(void*, curl_slist_append(SDL_reinterpret_cast(curl_slist*, m_curlJsonHeader), "Content-Type: application/json"));
	m_curlJsonHeader = SDL_reinterpret_cast(void*, curl_slist_append(SDL_reinterpret_cast(curl_slist*, m_curlJsonHeader), "charset: utf-8"));
}

Http::~Http()
{
	for(std::vector<HttpRequest>::iterator it = m_httpRequests.begin(), end = m_httpRequests.end(); it != end; ++it)
	{
		HttpRequest& currentRequest = (*it);
		SDL_RWclose(currentRequest.file_handle);
		curl_multi_remove_handle(SDL_reinterpret_cast(CURLM*, m_curlHandle), SDL_reinterpret_cast(CURL*, currentRequest.easy_handle));
		curl_easy_cleanup(SDL_reinterpret_cast(CURL*, currentRequest.easy_handle));
		if(currentRequest.json_data)
			SDL_free(currentRequest.json_data);
	}
	curl_multi_cleanup(SDL_reinterpret_cast(CURLM*, m_curlHandle));
}

Uint32 Http::addRequest(const std::string& urlRequest, const std::string& fileName, const std::string& jsonData, void (*eventHandlerFunction)(Uint32,Sint32), Uint32 mEvent)
{
	static Uint32 httpInternalId = 0;
	SDL_RWops* fp = SDL_RWFromFile(fileName.c_str(), "wb");
	if(!fp)
	{
		HttpRequest newRequest;
		newRequest.eventHandlerFunction = eventHandlerFunction;
		newRequest.easy_handle = NULL;
		newRequest.file_handle = NULL;
		newRequest.json_data = NULL;
		newRequest.result = HTTP_RESULT_FAILED;
		newRequest.internalId = httpInternalId;
		newRequest.evtParam = mEvent;
		m_httpRequests.push_back(newRequest);
		if(eventHandlerFunction)
			UTIL_SafeEventHandler(eventHandlerFunction, mEvent, SDL_static_cast(Sint32, httpInternalId));

		UTIL_SafeEventHandler(deleteRequest, httpInternalId, 0);
		return httpInternalId++;
	}

	char* json_data = NULL;
	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, urlRequest.c_str());
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
	if(!jsonData.empty())
	{
		json_data = SDL_strdup(jsonData.c_str());
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, SDL_reinterpret_cast(curl_slist*, m_curlJsonHeader));
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
	}
	curl_multi_add_handle(SDL_reinterpret_cast(CURLM*, m_curlHandle), curl);

	HttpRequest newRequest;
	newRequest.eventHandlerFunction = eventHandlerFunction;
	newRequest.easy_handle = SDL_reinterpret_cast(void*, curl);
	newRequest.file_handle = fp;
	newRequest.json_data = json_data;
	newRequest.result = HTTP_RESULT_SUCCEEDED;
	newRequest.internalId = httpInternalId;
	newRequest.evtParam = mEvent;
	m_httpRequests.push_back(newRequest);

	++m_remainingHandles;
	return httpInternalId++;
}

HttpRequest* Http::getRequest(Uint32 internalId)
{
	for(std::vector<HttpRequest>::iterator it = m_httpRequests.begin(), end = m_httpRequests.end(); it != end; ++it)
	{
		HttpRequest& currentRequest = (*it);
		if(internalId == currentRequest.internalId)
			return &currentRequest;
	}
	return NULL;
}

void Http::removeRequest(Uint32 internalId, bool cleanup)
{
	for(std::vector<HttpRequest>::iterator it = m_httpRequests.begin(), end = m_httpRequests.end(); it != end; ++it)
	{
		HttpRequest& currentRequest = (*it);
		if(internalId == currentRequest.internalId)
		{
			if(cleanup)
			{
				SDL_RWclose(currentRequest.file_handle);
				curl_multi_remove_handle(SDL_reinterpret_cast(CURLM*, m_curlHandle), SDL_reinterpret_cast(CURL*, currentRequest.easy_handle));
				curl_easy_cleanup(SDL_reinterpret_cast(CURL*, currentRequest.easy_handle));
			}
			if(currentRequest.json_data)
				SDL_free(currentRequest.json_data);
			m_httpRequests.erase(it);
			return;
		}
	}
}

void Http::handleResult(void* handle, Sint32 result)
{
	for(std::vector<HttpRequest>::iterator it = m_httpRequests.begin(), end = m_httpRequests.end(); it != end; ++it)
	{
		HttpRequest& currentRequest = (*it);
		if(currentRequest.easy_handle == handle)
		{
			SDL_RWclose(currentRequest.file_handle);
			if (SDL_static_cast(CURLcode, result) == CURLE_OK)
				currentRequest.result = HTTP_RESULT_SUCCEEDED;
			else
				currentRequest.result = HTTP_RESULT_FAILED;

			if(currentRequest.eventHandlerFunction)
				UTIL_SafeEventHandler(currentRequest.eventHandlerFunction, currentRequest.evtParam, SDL_static_cast(Sint32, currentRequest.internalId));

			UTIL_SafeEventHandler(deleteRequest, currentRequest.internalId, 0);
			return;
		}
	}
}

void Http::updateHttp()
{
	if(!m_remainingHandles)
		return;

	Sint32 remainingHandles = m_remainingHandles;
	curl_multi_perform(SDL_reinterpret_cast(CURLM*, m_curlHandle), &m_remainingHandles);
	if(remainingHandles != m_remainingHandles)
	{
		Sint32 msgs_left;
		CURLMsg* msg;
		while((msg = curl_multi_info_read(SDL_reinterpret_cast(CURLM*, m_curlHandle), &msgs_left)) != NULL && (msg->msg == CURLMSG_DONE))
		{
			handleResult(SDL_reinterpret_cast(void*, msg->easy_handle), SDL_static_cast(Sint32, msg->data.result));
			curl_multi_remove_handle(SDL_reinterpret_cast(CURLM*, m_curlHandle), msg->easy_handle);
			curl_easy_cleanup(msg->easy_handle);
		}
	}
}
