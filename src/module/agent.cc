/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include "private.h"
 #include <udjat/network/agent.h>
 #include <controller.h>
 #include <udjat/tools/xml.h>
 #include <udjat/network/resolver.h>
 #include <cstring>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <ifaddrs.h>

 namespace Udjat {

	/// @brief ICMP Response state.
	class ICMPResponseState : public Network::Agent::State {
	private:
		Network::ICMPResponse id;

	public:
		ICMPResponseState(const char *name, const Level level, const char *summary, const char *body, const Network::ICMPResponse i) : Network::Agent::State(name,level,summary,body), id(i) {
		}

		ICMPResponseState(const pugi::xml_node &node, const Network::ICMPResponse i) : Network::Agent::State(node), id(i) {
		}

		bool isValid(const Network::ICMPResponse response) const noexcept override {
			return response == id;
		}

	};

	Network::Agent::Agent(const pugi::xml_node &node) {

		// Do an ICMP check?
		icmp.check = Udjat::Attribute(node,"icmp").as_bool(true);
		icmp.timeout = Udjat::Attribute(node,"icmp-timeout").as_uint(icmp.timeout);

		load(node);

	}

	Network::Agent::~Agent() {
	}

	bool Network::Agent::hasStates() const noexcept {
		return !this->states.empty();
	}

	void Network::Agent::append_state(const pugi::xml_node &node) {

		/// @brief IP Address range state.
		class Range : public Network::Range {
		private:
			sockaddr_storage addr;
			uint16_t mask;

		public:
			Range(const pugi::xml_node &node) : Network::Range(node) {
				memset(&addr,0,sizeof(addr));
				parse(Attribute(node,"range").as_string(),addr,mask);
			}

		protected:
			bool test(const sockaddr_storage &ip) const override {

				// TODO: Implement IPV6 methods.

				if(addr.ss_family == AF_INET) {
					sockaddr_in netmask;
					return inRange(*((const sockaddr_in *) &ip), *((const sockaddr_in *) &this->addr), getMask(netmask,this->mask));
				} else {
					throw runtime_error("Unsupported address family");
				}

				return false;
			}

		};

		/// @brief Local network state
		class SameNetwork : public Network::Range {
		public:
			SameNetwork(const pugi::xml_node &node) : Network::Range(node) {
				revert = !Attribute(node,"same-network").as_bool(true);
			}

		protected:
			bool test(const sockaddr_storage &ip) const override {

				bool rc = false;

				struct ifaddrs *ifaces;

				if(getifaddrs(&ifaces)) {
					throw std::system_error(errno, std::system_category(), "Cant get network interface list");
				}

				try {

					for(ifaddrs *iface = ifaces; iface; iface = iface->ifa_next) {

						if(!iface->ifa_addr || iface->ifa_addr->sa_family != AF_INET) {
							continue;
						}

						if(inRange(ip,*iface->ifa_addr,*iface->ifa_netmask)) {
#ifdef DEBUG
							cout << "Found " << std::to_string(ip) << " in " << iface->ifa_name << endl;
#endif // DEBUG
							rc = true;
							break;

						}
					}

				} catch(...) {

					freeifaddrs(ifaces);
					throw;
				}

				freeifaddrs(ifaces);

				return rc;
			}
		};

		if(node.attribute("range")) {

			states.push_back(make_shared<Range>(node));

		} else if(node.attribute("same-network")) {

			states.push_back(make_shared<SameNetwork>(node));

		} else if(node.attribute("icmp-response")) {

			if(!icmp.check) {
				throw runtime_error("Can't use 'icmp-response' states without icmp='true' attribute on the agent definition");
			}

			ICMPResponse id = (ICMPResponse) Attribute(node,"icmp-response").select("echo-reply", "destination-unreachable", "time-exceeded", "timeout", nullptr);
			states.push_back(make_shared<ICMPResponseState>(node,id));

		} else {

			super::append_state(node);

		}

	}

	void Network::Agent::set(const sockaddr_storage &addr) {

		// Check states.
		for(auto state : states) {

			State * st = dynamic_cast<State *>(state.get());

			if(st && (!selected || st->getLevel() > selected->getLevel())) {
				if(st->isValid(addr)) {
					selected = state;
				}
#ifdef DEBUG
				else {
					cout << "State " << st->getName() << " was rejected" << endl;
				}
#endif // DEBUG
			}
		}

		if(icmp.check) {
			Controller::getInstance().insert(this,addr);
		}

	}

	sockaddr_storage Network::Agent::resolv(sockaddr_storage &dnssrv, const char *hostname) {

		sockaddr_storage addr;

#ifdef DEBUG
		cout << "Checking " << hostname << endl;
#endif // DEBUG

		// Check DNS resolution.
		DNSResolver resolver{dnssrv};

		resolver.query(hostname);

		if(!resolver.size()) {
			throw runtime_error(string{"Can't resolve '"} + hostname + "'");
		}

		addr = resolver.begin()->getAddr();

		return addr;
	}

	void Udjat::Network::Agent::set(const Udjat::Network::ICMPResponse response) {

		for(auto state : states) {

			State * st = dynamic_cast<State *>(state.get());

			if(st && (!selected || st->getLevel() >= selected->getLevel())) {
				if(st->isValid(response)) {
					selected = state;
				}
			}

		}

		if(selected) {
			activate(selected);
		}

	}

	void Network::Agent::checkStates() {

		if(states.empty() && icmp.check) {

			static const struct {
				const char *name;
				const ICMPResponse id;
				const Level level;
				const char *summary;
				const char *body;
			} responses[] = {
				{
					"active",
					ICMPResponse::echo_reply,
					Level::ready,
					"${agent.name} is active",
					"Got ICMP echo reply from host."
				},
				{
					"unreachable",
					ICMPResponse::destination_unreachable,
					Level::error,
					"${agent.name} is not reachable",
					"Destination Unreachable. The gateway doesnt know how to get to the defined network."
				},
				{
					"time-exceeded",
					ICMPResponse::time_exceeded,
					Level::error,
					"${agent.name} is not acessible",
					"Time Exceeded. The ICMP request has been discarded because it was 'out of time'."
				},
				{
					"timeout",
					ICMPResponse::timeout,
					Level::error,
					"${agent.name} is not available",
					"No ICMP response from host."
				}

			};

			cout << getName() << "\tLoading standard ICMP states" << endl;

			for(size_t ix = 0; ix < (sizeof(responses)/sizeof(responses[0])); ix++) {

				states.push_back(make_shared<ICMPResponseState>(
										responses[ix].name,
										responses[ix].level,
										Quark(expand(responses[ix].summary)).c_str(),
										responses[ix].body,
										responses[ix].id
									)
								);

			}

		}

	}

 }
