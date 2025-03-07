/* InterfaceAttr.cpp

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
#include <InterfaceAttr.h>

#include "Log.h"
#include "Utils.h"
#include "StringConverter.h"

#include <netinet/in.h>
#include <string>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>

#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <unistd.h>

	// =========================================================================
	//  -- Constructors and destructor -----------------------------------------
	// =========================================================================

	InterfaceAttr::InterfaceAttr(const std::string &bindInterfaceName) {
		bindToInterfaceName(bindInterfaceName);
	}

	InterfaceAttr::~InterfaceAttr() {}

	// =========================================================================
	//  -- Static member functions ---------------------------------------------
	// =========================================================================

	int InterfaceAttr::getNetworkUDPBufferSize() {
		int fd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (fd == -1) {
			PERROR("socket");
			return 0;
		}
		int bufferSize;
		socklen_t optlen = sizeof(bufferSize);
		if (::getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &bufferSize, &optlen) == -1) {
			PERROR("getsockopt: SO_SNDBUF");
			bufferSize = 0;
		}
		CLOSE_FD(fd);
		return bufferSize / 2;
	}

	// =========================================================================
	//  -- Other member functions ----------------------------------------------
	// =========================================================================

	void InterfaceAttr::bindToInterfaceName(const std::string &ifaceName) {
		struct ifaddrs *ifaddrHead, *ifa;
		// get list of interfaces
		if (::getifaddrs(&ifaddrHead) == -1) {
			PERROR("getifaddrs");
			return;
		}
		bool foundInterface = false;
		// go through list, find requested ifaceName or first usable interface
		for (ifa = ifaddrHead; ifa != nullptr; ifa = ifa->ifa_next) {
			if (ifa->ifa_addr == nullptr) {
				// try next one
				continue;
			}
			const int family = ifa->ifa_addr->sa_family;
			// we are not looking for 'lo' interface, but check any other AF_INET* interface
			if ((strcmp(ifa->ifa_name, "lo")) != 0 && (family == AF_INET /*|| family == AF_INET6*/)) {
				char host[NI_MAXHOST];
				const int s = ::getnameinfo(ifa->ifa_addr,
					(family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
					 host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
				if (s != 0) {
					GAI_PERROR("getnameinfo()", s);
					continue;
				}
				// Get hardware address
				int fd = ::socket(AF_INET, SOCK_STREAM, 0);
				if (fd == -1) {
					PERROR("Server socket");
					continue;
				}
				struct ifreq ifr;
				ifr.ifr_addr.sa_family = family;
				std::memcpy(&ifr.ifr_name, ifa->ifa_name, IFNAMSIZ);
				if (::ioctl(fd, SIOCGIFHWADDR, &ifr) != 0) {
					PERROR("ioctl SIOCGIFHWADDR");
					CLOSE_FD(fd);
					continue;
				}
				CLOSE_FD(fd);
				// Check did we find the requested or we should get the first usable
				if (ifaceName == ifr.ifr_name || ifaceName.empty()) {
					const unsigned char* mac = reinterpret_cast<unsigned char *>(ifr.ifr_hwaddr.sa_data);
					StringConverter::addFormattedString(_macAddrDecorated, "%02x:%02x:%02x:%02x:%02x:%02x",
						   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
					StringConverter::addFormattedString(_macAddr, "%02x%02x%02x%02x%02x%02x",
						   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
					_ifaceName = ifr.ifr_name;
					_ipAddr = host;
					foundInterface = true;
					break;
				}
			}
		}
		// free linked list
		::freeifaddrs(ifaddrHead);
		if (foundInterface) {
			SI_LOG_INFO("%s: %s [%s]", _ifaceName.c_str(), _ipAddr.c_str(), _macAddrDecorated.c_str());
		} else {
			if (ifaceName.empty()) {
				SI_LOG_INFO("Bind failed: No usable network interface found");
			} else {
				SI_LOG_INFO("Bind failed: Could not find [%s] interface", ifaceName.c_str());
			}
		}
	}

	std::string InterfaceAttr::getUUID() const {
		return StringConverter::stringFormat("50c958a8-e839-4b96-b7ae-%1", _macAddr);
	}
