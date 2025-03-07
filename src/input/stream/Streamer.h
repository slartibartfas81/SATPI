/* Streamer.h

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
#ifndef INPUT_STREAM_STREAMER_H_INCLUDE
#define INPUT_STREAM_STREAMER_H_INCLUDE INPUT_STREAM_STREAMER_H_INCLUDE

#include <FwDecl.h>
#include <input/Device.h>
#include <socket/SocketClient.h>
#include <socket/UdpSocket.h>

#include <vector>
#include <string>

#include <poll.h>

FW_DECL_NS1(input, DeviceData);

FW_DECL_SP_NS2(input, stream, Streamer);

FW_DECL_VECTOR_NS0(Stream);

namespace input {
namespace stream {

/// The class @c Streamer is for reading from an TS stream as input device.
/// Stream can be an Multicast UDP e.g.
/// http://ip.of.your.box:8875/?msys=streamer&uri=udp://224.0.1.3:1234
class Streamer :
	public input::Device,
	public UdpSocket {

		// =====================================================================
		//  -- Constructors and destructor -------------------------------------
		// =====================================================================

	public:

		Streamer(int streamID, const std::string &bindIPAddress);

		virtual ~Streamer();

		// =====================================================================
		//  -- Static member functions -----------------------------------------
		// =====================================================================

	public:

		static void enumerate(
			StreamVector &streamVector,
			const std::string &bindIPAddress);

		// =====================================================================
		// -- base::XMLSupport -------------------------------------------------
		// =====================================================================

	public:
		///
		virtual void addToXML(std::string &xml) const override;

		///
		virtual void fromXML(const std::string &xml) override;


		// =====================================================================
		//  -- input::Device----------------------------------------------------
		// =====================================================================

	public:

		virtual void addDeliverySystemCount(
			std::size_t &dvbs2,
			std::size_t &dvbt,
			std::size_t &dvbt2,
			std::size_t &dvbc,
			std::size_t &dvbc2) override;

		virtual bool isDataAvailable() override;

		virtual bool readFullTSPacket(mpegts::PacketBuffer &buffer) override;

		virtual bool capableOf(input::InputSystem msys) const override;

		virtual bool capableToTransform(const std::string &msg, const std::string &method) const override;

		virtual void monitorSignal(bool showStatus) override;

		virtual bool hasDeviceDataChanged() const override;

		virtual void parseStreamString(const std::string &msg, const std::string &method) override;

		virtual bool update() override;

		virtual bool teardown() override;

		virtual std::string attributeDescribeString() const override;

		// =====================================================================
		//  -- Other member functions ------------------------------------------
		// =====================================================================

	protected:

		// =====================================================================
		// -- Data members -----------------------------------------------------
		// =====================================================================

	private:
		std::string _bindIPAddress;
		std::string _uri;
		pollfd _pfd[1];
		SocketClient _udpMultiListen;
		std::string _multiAddr;
		int _port;
		bool _udp;
};

} // namespace stream
} // namespace input

#endif // INPUT_STREAM_STREAMER_H_INCLUDE
