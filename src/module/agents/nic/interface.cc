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

 #include <config.h>
 #include <private/agents/nic.h>
 #include <udjat/agent.h>
 #include <udjat/tools/net/nic.h>
 #include <udjat/tools/logger.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	Network::Agent::Interface::Interface(const pugi::xml_node &node) : Udjat::Agent<unsigned short>(node,NIC_STATE_UNDEFINED) {

		static const char * names[] = {
			"nic",
			"interface-name",
			"name"
		};

		for(size_t ix = 0; ix < N_ELEMENTS(names); ix++) {
			auto attr = Object::getAttribute(node,names[ix],false);
			if(attr) {
				intf = Network::Interface::get(attr.as_string("eth0"));
				break;
			}
		}

		if(!intf) {
			throw runtime_error("Required attribute 'interface-name' was not found");
		}

	}

	Network::Agent::Interface::~Interface() {
	}

	std::shared_ptr<Abstract::State> Network::Agent::Interface::StateFactory(const pugi::xml_node &node) {
		return super::StateFactory(node,NicStateFactory(node));
	}

	static unsigned short interface_state(std::shared_ptr<Udjat::Network::Interface> intf) {

		if(!intf->found()) {
			debug("Interface '",intf->name(),"' not found");
			return Network::NIC_STATE_NOT_FOUND;
		}

		if(intf->up() && Network::Agent::has_link(intf->name())) {
			debug("Interface '",intf->name(),"' is online");
			return Network::NIC_STATE_ONLINE;
		}

		debug("Interface '",intf->name(),"' is offline");
		return Network::NIC_STATE_OFFLINE;
	}

	void Network::Agent::Interface::start() {
		super::start(interface_state(intf));
	}

	bool Network::Agent::Interface::refresh() {
		return set(interface_state(intf));
	}

	std::string Network::Agent::Interface::to_string() const noexcept {

		switch(get()) {
		case NIC_STATE_NOT_FOUND:
			return "not-found";

		case NIC_STATE_ONLINE:
			return "online";

		case NIC_STATE_OFFLINE:
			return "offline";

		}

		return "";
	}


 }
