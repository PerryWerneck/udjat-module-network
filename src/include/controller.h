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

 #include <config.h>
 #include <udjat/network/agents/host.h>
 #include <udjat/tools/timer.h>
 #include <udjat/tools/handler.h>
 #include <mutex>
 #include <memory>
 #include <iostream>
 #include <list>

 using namespace std;

 namespace Udjat {

 	class Network::HostAgent::Controller : private MainLoop::Timer, private MainLoop::Handler {
	public:

		#pragma pack(1)
		/// @brief ICMP payload.
		struct Payload {
			uint16_t	id;
			uint16_t	seq;
			uint64_t	time;
		};
		#pragma pack()


	private:

		static recursive_mutex guard;

		class Host {
		private:

			Network::HostAgent *agent;
			sockaddr_storage addr;

			uint16_t id;
			uint16_t packets = 0;
			time_t timeout;
			time_t next = 0;

		public:

			Host(Network::HostAgent *agent, const sockaddr_storage &addr);

			bool onTimer();
			void send() noexcept;

			inline bool operator ==(const Network::HostAgent *agent) const noexcept {
				return agent == this->agent;
			}

			bool onResponse(int icmp_type, const sockaddr_storage &addr, const Controller::Payload &payload) noexcept;

		};

		list<Host> hosts;

		Controller();

		void start();
		void stop();

		void on_timer() override;
		void handle_event(const Event event) override;


	public:

		static Controller & getInstance();

		~Controller();

		void insert(Network::HostAgent *agent, const sockaddr_storage &addr);
		void remove(Network::HostAgent *agent);

		void send(const sockaddr_storage &addr, const Payload &payload);

	};


 }
