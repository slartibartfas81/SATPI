/* Frontend.h

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
#ifndef INPUT_DVB_FRONTEND_H_INCLUDE
#define INPUT_DVB_FRONTEND_H_INCLUDE INPUT_DVB_FRONTEND_H_INCLUDE

#include <FwDecl.h>
#include <input/Device.h>
#include <input/Transformation.h>
#include <input/dvb/delivery/System.h>
#include <input/dvb/FrontendData.h>
#ifdef LIBDVBCSA
#include <input/dvb/FrontendDecryptInterface.h>
#include <decrypt/dvbapi/ClientProperties.h>
#endif

#include <vector>
#include <string>

FW_DECL_NS1(input, DeviceData);
FW_DECL_NS3(input, dvb, delivery, System);

FW_DECL_SP_NS2(decrypt, dvbapi, Client);
FW_DECL_SP_NS2(input, dvb, Frontend);

FW_DECL_VECTOR_NS0(Stream);

namespace input {
namespace dvb {

/// The class @c Frontend carries all the data/information of an frontend
/// and to tune it
class Frontend :
#ifdef LIBDVBCSA
	public input::dvb::FrontendDecryptInterface,
#endif
	public input::Device {
		// =======================================================================
		// -- Static Data members ------------------------------------------------
		// =======================================================================

	public:

		static const unsigned int DEFAULT_DVR_BUFFER_SIZE;
		static const unsigned int MAX_DVR_BUFFER_SIZE;

		// =======================================================================
		//  -- Constructors and destructor ---------------------------------------
		// =======================================================================

	public:

		Frontend(
			int streamID,
			const std::string &appDataPath,
			const std::string &fe,
			const std::string &dvr,
			const std::string &dmx);

		virtual ~Frontend();

		// =======================================================================
		//  -- Static member functions -------------------------------------------
		// =======================================================================

	public:

		static void enumerate(
			StreamVector &streamVector,
			const std::string &appDataPath,
			decrypt::dvbapi::SpClient decrypt,
			const std::string &dvbAdapterpath);

		// =======================================================================
		// -- base::XMLSupport ---------------------------------------------------
		// =======================================================================

	public:
		///
		virtual void addToXML(std::string &xml) const override;

		virtual void fromXML(const std::string &xml) override;

#ifdef LIBDVBCSA
		// =======================================================================
		// -- FrontendDecryptInterface -------------------------------------------
		// =======================================================================
	public:

		virtual int getStreamID() const override;

		virtual int getBatchCount() const override;

		virtual int getBatchParity() const override;

		virtual int getMaximumBatchSize() const override;

		virtual void decryptBatch(bool final) override;

		virtual void setBatchData(unsigned char *ptr, int len, int parity, unsigned char *originalPtr) override;

		virtual const dvbcsa_bs_key_s *getKey(int parity) const override;

		virtual void setKey(const unsigned char *cw, int parity, int index) override;

		virtual void startOSCamFilterData(int pid, int demux, int filter,
			const unsigned char *filterData, const unsigned char *filterMask) override;

		virtual void stopOSCamFilterData(int pid, int demux, int filter) override;

		virtual bool findOSCamFilterData(int streamID, int pid, const unsigned char *tsPacket, int &tableID,
			int &filter, int &demux, mpegts::TSData &filterData) override;

		virtual void stopOSCamFilters(int streamID) override;

		virtual void setECMInfo(
			int pid,
			int serviceID,
			int caID,
			int provID,
			int emcTime,
			const std::string &cardSystem,
			const std::string &readerName,
			const std::string &sourceName,
			const std::string &protocolName,
			int hops) override;

		virtual bool isMarkedAsPMT(int pid) const override;

		virtual const mpegts::PMT &getPMTData() const override;
#endif

		// =======================================================================
		//  -- input::Device------------------------------------------------------
		// =======================================================================

	public:

		virtual void addDeliverySystemCount(
			std::size_t &dvbs2,
			std::size_t &dvbt,
			std::size_t &dvbt2,
			std::size_t &dvbc,
			std::size_t &dvbc2) override;

		virtual bool isDataAvailable() override;

		virtual bool readFullTSPacket(mpegts::PacketBuffer &buffer) override;

		virtual bool capableOf(InputSystem system) const override;

		virtual bool capableToTransform(const std::string &msg, const std::string &method) const override;

		virtual void monitorSignal(bool showStatus) override;

		virtual bool hasDeviceDataChanged() const override;

		virtual void parseStreamString(const std::string &msg, const std::string &method) override;

		virtual bool update() override;

		virtual bool teardown() override;

		virtual std::string attributeDescribeString() const override;

		// =======================================================================
		//  -- Other member functions --------------------------------------------
		// =======================================================================

	protected:

		void setupFrontend();

		///
		int openFE(const std::string &path, bool readonly) const;

		///
		void closeFE();

		///
		int openDVR(const std::string &path) const;

		///
		void closeDVR();

		///
		int openDMX(const std::string &path) const;

		///
		bool setDMXFilter(int fd, uint16_t pid);

		///
		bool tune();

		///
		bool isTuned() const {
			return (_fd_dvr != -1) && _tuned;
		}

		bool updatePIDFilters();

		///
		bool setupAndTune();

		///
		void closePid(int pid);

		///
		bool openPid(int pid);

		// =======================================================================
		// -- Data members -------------------------------------------------------
		// =======================================================================

	private:
		bool _tuned;
		int _fd_fe;
		int _fd_dvr;
		std::string _path_to_fe;
		std::string _path_to_dvr;
		std::string _path_to_dmx;
		struct dvb_frontend_info _fe_info;

		input::dvb::delivery::SystemVector _deliverySystem;
		input::dvb::FrontendData _frontendData;
#ifdef LIBDVBCSA
		decrypt::dvbapi::ClientProperties _dvbapiData;
#endif
		input::dvb::FrontendData _transformFrontendData;
		input::Transformation _transform;
		std::size_t _dvbs2;
		std::size_t _dvbt;
		std::size_t _dvbt2;
		std::size_t _dvbc;
		std::size_t _dvbc2;

		unsigned long _dvrBufferSizeMB;
};

} // namespace dvb
} // namespace input

#endif // INPUT_DVB_FRONTEND_H_INCLUDE
