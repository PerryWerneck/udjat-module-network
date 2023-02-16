/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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

 #include <config.h>

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/object.h>
 #include <pugixml.hpp>
 #include <udjat/net/ip/address.h>
 #include <udjat/net/ip/state.h>
 #include <udjat/net/interface.h>

 using namespace std;

 namespace Udjat {

	std::shared_ptr<Abstract::IP::State> IP::State::Factory(const pugi::xml_node &node) {

		/*
		const char *subnet = Object::getAttribute(node,"subnet").as_string();

		// Create remote/local subnet.
		class LocalState : public IP::State {
		private:
			bool revert;

		protected:
			/// @brief Test an IPV4 address range.
			bool compare(const sockaddr_in &addr) const override {
				bool found = Network::Interface::for_each([&addr](const Network::Interface &interface){
					return interface.local(addr);
				});
				return (revert ? !found : found);
			}

			/// @brief Test an IPV6 address range.
			bool compare(const sockaddr_in6 &addr) const override {
				bool found = Network::Interface::for_each([&addr](const Network::Interface &interface){
					return interface.local(addr);
				});
				return (revert ? !found : found);
			}

		public:
			State(const pugi::xml_node &node, bool r) : IP::State{node}, revert{r} {

			}

		};

		if(!strncasecmp(subnet,"local")) {
			return make_shared<LocalState>(node,false);
		}

		if(!strncasecmp(subnet,"remote")) {
			return make_shared<LocalState>(node,true);
		}
		*/

		// Create default state.
		return make_shared<IP::State>(node);
	}

	IP::State::State(const char *subnet) : Abstract::IP::State{subnet} {
		SubNet::set(subnet);
	}

	IP::State::State(const pugi::xml_node &node) : Abstract::IP::State{node}, IP::SubNet{node} {
	}

	std::string IP::State::to_string() const noexcept {

		if(Object::properties.summary[0]) {
			return Object::properties.summary;
		}

		return IP::SubNet::to_string();
	}

	/// @brief Test an IPV4 address range.
	bool IP::State::compare(const sockaddr_in &addr) const {
		return IP::SubNet::contains(addr);
	}

	/// @brief Test an IPV6 address range.
	bool IP::State::compare(const sockaddr_in6 &addr) const {
		return IP::SubNet::contains(addr);
	}

	bool Abstract::IP::State::compare(const sockaddr_storage &addr) const {

		switch(addr.ss_family) {
		case AF_INET:
			return compare(*((const sockaddr_in *) &addr));

		case AF_INET6:
			return compare(*((const sockaddr_in6 *) &addr));

		default:
			throw runtime_error("Invalid address family");
		}


	}

 }
