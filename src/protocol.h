/*
  Tibia CLient
  Copyright (C) 2019 Saiyans King

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

#ifndef __FILE_PROTOCOL_h_
#define __FILE_PROTOCOL_h_

#include "connection.h"

enum ProtocolOS
{
	PROTOCOL_OS_LINUX = 1,
	PROTOCOL_OS_WINDOWS = 2,
	PROTOCOL_OS_FLASH = 3
};

enum ChecksumMethods : Uint8
{
	CHECKSUM_METHOD_NONE,
	CHECKSUM_METHOD_ADLER32,
	CHECKSUM_METHOD_SEQUENCE,
	CHECKSUM_METHOD_CHALLENGE
};

class ProtocolGame;
class Protocol
{
	public:
		Protocol();
		virtual ~Protocol() {;}

		virtual ProtocolGame* getProtocolGame() {return NULL;}
		virtual const ProtocolGame* getProtocolGame() const {return NULL;}

		virtual void parseMessage(InputMessage& msg) = 0;
		virtual void onConnect() = 0;
		virtual void onConnectionError(ConnectionError error) = 0;
		virtual void onDisconnect() = 0;

		bool onRecv(InputMessage& msg);
		bool onSend(OutputMessage& msg);

		SDL_INLINE void setEncryption(bool encrypt) {m_encryption = encrypt;}
		SDL_INLINE void setChecksumMethod(Uint8 checksumMethod) {m_checksumMethod = checksumMethod;}
		SDL_INLINE void setEncryptionKeys(Uint32 keys[4]) {m_encryptionKeys[0] = keys[0]; m_encryptionKeys[1] = keys[1]; m_encryptionKeys[2] = keys[2]; m_encryptionKeys[3] = keys[3];}

		Uint16 getHeaderPos();

		static Uint16 getOS();
		static Uint16 getProtocolVersion();
		static Uint32 getClientVersion();

	private:
		Uint32 m_encryptionKeys[4];
		Uint32 m_clientSequence;
		Uint32 m_serverSequence;
		Uint8 m_checksumMethod;
		bool m_encryption;
};

#endif /* __FILE_PROTOCOL_h_ */
