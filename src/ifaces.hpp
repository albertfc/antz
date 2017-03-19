/*
 * This file is part of Antz.
 * Copyright (C) 2017  Albert Farres

 * Antz is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Antz is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Antz.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

template <typename Impl_MIDI_packet_parser>
struct Iface_MIDI_packet_parser
{
	static void get_packet( uint8_t (&buffer)[3] )
		{ Impl_MIDI_packet_parser::get_packet_impl( buffer ); }

	private:
		Iface_MIDI_packet_parser() {} // Pure static class. Disallow creating an instance of this object
};

