/* Streamer.cpp

   Copyright (C) 2014 - 2019 Marc Postema (mpostema09 -at- gmail.com)

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
   Or, point your browser to http://www.gnu.org/copyleft/gpl.html
 */
#include <input/stream/Streamer.h>

#include <Log.h>
#include <Unused.h>
#include <Stream.h>
#include <StringConverter.h>
#include <mpegts/PacketBuffer.h>

#include <cstring>

namespace input {
namespace stream {

	Streamer::Streamer(int streamID, const std::string &bindIPAddress) :
		Device(streamID),
		_bindIPAddress(bindIPAddress),
		_uri("None"),
		_multiAddr("None"),
		_port(0),
		_udp(false) {
		_pfd[0].events  = 0;
		_pfd[0].revents = 0;
		_pfd[0].fd      = -1;
	}

	Streamer::~Streamer() {}

	// =======================================================================
	//  -- Static member functions -------------------------------------------
	// =======================================================================

	void Streamer::enumerate(
			StreamVector &streamVector,
			const std::string &bindIPAddress) {
		SI_LOG_INFO("Setting up TS Streamer");
		const StreamVector::size_type size = streamVector.size();
		const input::stream::SpStreamer streamer = std::make_shared<Streamer>(size, bindIPAddress);
		streamVector.push_back(std::make_shared<Stream>(size, streamer, nullptr));
	}

	// =======================================================================
	//  -- base::XMLSupport --------------------------------------------------
	// =======================================================================

	void Streamer::addToXML(std::string &xml) const {
		base::MutexLock lock(_mutex);
		ADD_XML_ELEMENT(xml, "frontendname", "Streamer");
		ADD_XML_ELEMENT(xml, "pathname", _uri);
	}

	void Streamer::fromXML(const std::string &UNUSED(xml)) {
		base::MutexLock lock(_mutex);
	}

	// =======================================================================
	//  -- input::Device -----------------------------------------------------
	// =======================================================================

	void Streamer::addDeliverySystemCount(
			std::size_t &dvbs2,
			std::size_t &dvbt,
			std::size_t &dvbt2,
			std::size_t &dvbc,
			std::size_t &dvbc2) {
		dvbs2 += 0;
		dvbt  += 0;
		dvbt2 += 0;
		dvbc  += 0;
		dvbc2 += 0;
	}

	bool Streamer::isDataAvailable() {
		// call poll with a timeout of 500 ms
		const int pollRet = poll(_pfd, 1, 500);
		if (pollRet > 0) {
			return _pfd[0].revents != 0;
		}
		return false;
	}

	bool Streamer::readFullTSPacket(mpegts::PacketBuffer &buffer) {
		if (_udpMultiListen.getFD() != -1) {
			// Read from stream
			char *ptr = reinterpret_cast<char *>(buffer.getWriteBufferPtr());
			const auto size = buffer.getAmountOfBytesToWrite();

			const ssize_t readSize = _udpMultiListen.recvDatafrom(ptr, size, MSG_DONTWAIT);
			if (readSize > 0) {
				buffer.addAmountOfBytesWritten(readSize);
				buffer.trySyncing();
			} else {
				PERROR("_udpMultiListen");
			}
			return buffer.full();
		}
		return false;
	}

	bool Streamer::capableOf(const input::InputSystem system) const {
		return system == input::InputSystem::STREAMER;
	}

	bool Streamer::capableToTransform(const std::string &UNUSED(msg),
			const std::string &UNUSED(method)) const {
		return false;
	}

	void Streamer::monitorSignal(const bool UNUSED(showStatus)) {}

	bool Streamer::hasDeviceDataChanged() const {
		return false;
	}

// Server side
// vlc -vvv "D:\test.ts" :sout=#udp{dst=224.0.1.3:123} :sout-all :sout-keep --loop

// Client side
// http://192.168.178.10:8875/?msys=streamer&uri=udp://224.0.1.3:1234

	void Streamer::parseStreamString(const std::string &msg, const std::string &method) {
		SI_LOG_INFO("Stream: %d, Parsing transport parameters...", _streamID);
		if (StringConverter::getURIParameter(msg, method, _uri) == true) {
			// Open stream
			_udp = _uri.find("udp") != std::string::npos;
			std::string::size_type begin = _uri.find("//");
			if (begin != std::string::npos) {
				std::string::size_type end = _uri.find_first_of(":", begin);
				if (end != std::string::npos) {
					begin += 2;
					_multiAddr = _uri.substr(begin, end - begin);
					begin = end + 1;
					end = _uri.size();
					_port = std::stoi(_uri.substr(begin, end - begin));

					//  Open mutlicast stream
					if(initMutlicastUDPSocket(_udpMultiListen, _multiAddr, _bindIPAddress, _port)) {
						SI_LOG_INFO("Stream: %d, Streamer reading from: %s:%d  fd %d", _streamID, _multiAddr.c_str(), _port, _udpMultiListen.getFD());
						// set receive buffer to 8MB
						const int bufferSize =  1024 * 1024 * 8;
						_udpMultiListen.setNetworkReceiveBufferSize(bufferSize);

						_pfd[0].events  = POLLIN | POLLHUP | POLLRDNORM | POLLERR;
						_pfd[0].revents = 0;
						_pfd[0].fd      = _udpMultiListen.getFD();

					} else {
						SI_LOG_ERROR("Stream: %d, Init UDP Multicast socket failed", _streamID);
					}
				}
			}
		}
		SI_LOG_DEBUG("Stream: %d, Parsing transport parameters (Finished)", _streamID);
	}

	bool Streamer::update() {
		return true;
	}

	bool Streamer::teardown() {
		// Close stream
		_udpMultiListen.closeFD();
		return true;
	}

	std::string Streamer::attributeDescribeString() const {
		std::string desc;
		if (_udpMultiListen.getFD() != -1) {
			// ver=1.5;tuner=<feID>;uri=<uri>
			StringConverter::addFormattedString(desc, "ver=1.5;tuner=%d;uri=%s",
					_streamID + 1, _uri.c_str());
		} else {
			desc = "";
		}
		return desc;
	}

	// =======================================================================
	//  -- Other member functions --------------------------------------------
	// =======================================================================

} // namespace stream
} // namespace input
