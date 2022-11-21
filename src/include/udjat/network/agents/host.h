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

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/factory.h>
 #include <udjat/agent.h>
 #include <udjat/agent/state.h>
 #include <sys/socket.h>

 namespace Udjat {

	namespace Network {

		enum ICMPResponse : uint8_t {
			echo_reply,
			destination_unreachable,
			time_exceeded,
			timeout
		};

		/// @brief Agent to check for DNS resolution and ICMP test.
		class UDJAT_API HostAgent : public Udjat::Abstract::Agent {
		public:
			class State;

		private:

			/// @brief Internal agent state.
			std::shared_ptr<Abstract::State> selected;

			class Controller;
			friend class Controller;

			struct {
				bool check = true;		///< @brief Do ICMP check.
				time_t interval = 1;	///< @brief ICMP packet interval.
				time_t timeout = 5;		///< @brief ICMP timeout.
				uint64_t time = 0;		///< @brief Time of last response.
			} icmp;

			/// @brief Agent states.
			std::vector<std::shared_ptr<State>> states;

		protected:

			/// @brief If the agent has no states load the default ones.
			void checkStates();

			/// @brief Set address (do an ICMP check if necessary).
			void set(const sockaddr_storage &addr);

			/// @brief Do a DNS check.
			static sockaddr_storage resolv(sockaddr_storage &dnssrv, const char *hostname);

			//std::shared_ptr<Abstract::State> stateFromValue() const override;

		public:

			class State;

			class Factory : public Udjat::Factory {
			public:
				Factory();
				std::shared_ptr<Abstract::Agent> AgentFactory(const Abstract::Object &parent, const pugi::xml_node &node) const override;
			};

			HostAgent(const pugi::xml_node &node);
			virtual ~HostAgent();

			std::shared_ptr<Abstract::State> StateFactory(const pugi::xml_node &node) override;

			void set(ICMPResponse response, uint64_t time = 0);

			std::string to_string() const noexcept override;

			Udjat::Value & get(Udjat::Value &value) const;

			Value & getProperties(Value &response) const noexcept override;

 		};


	}

 }
