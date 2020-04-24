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

#ifndef __FILE_CONNECTION_h_
#define __FILE_CONNECTION_h_

#include "inputMessage.h"
#include "outputMessage.h"

enum ReadState
{
	READ_SIZE,
	READ_BUFFER
};

enum ConnectionState
{
	CONNECTION_STATE_INIT,
	CONNECTION_STATE_CONNECTING,
	CONNECTION_STATE_CONNECTED,
	CONNECTION_STATE_SEND_ERROR,
	CONNECTION_STATE_CLOSED,
	CONNECTION_STATE_ERROR
};

enum ConnectionError
{
	CONNECTION_ERROR_RESOLVE_HOST,
	CONNECTION_ERROR_CANNOT_CONNECT,
	CONNECTION_ERROR_FAIL_CONNECT,
	CONNECTION_ERROR_REFUSED_CONNECT,
	CONNECTION_ERROR_TIMEOUT,
	CONNECTION_ERROR_RECV_FAIL,
	CONNECTION_ERROR_SEND_FAIL,
	CONNECTION_ERROR_PROTOCOL_FAIL,
	CONNECTION_ERROR_INVALID
};

class Protocol;
class Connection
{
	public:
		Connection(const char* host, Uint16 port, const char* proxy, const char* proxyAuth, Protocol* protocol);
		~Connection();

		SDL_FORCE_INLINE Protocol* getProtocol() {return m_protocol;}
		SDL_FORCE_INLINE ConnectionState getConnectionStatus() {return m_connectionState;}
		void closeConnection();
		void updateConnection();

		Sint32 internalRead(Uint16 n);
		void sendMessage(OutputMessage& msg);

	protected:
		void closeConnectionError(ConnectionError error);

	private:
		Protocol* m_protocol;
		InputMessage m_inputMessage;

		void* m_curlHandle;
		void* m_curlEasyHandle;

		char* m_host;
		Uint16 m_port;
		char* m_proxy;
		char* m_proxyAuth;

		ReadState m_readState;
		ConnectionState m_connectionState;
		Uint32 m_waiting;
		Uint16 m_messageSize;
};

#endif /* __FILE_CONNECTION_h_ */
