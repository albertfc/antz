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

#include <stdint.h>

#include "log.h"

struct MIDI_status
{
	bool     cv1_free = true;
	uint16_t cv1_note;
	bool     cv2_on   = false;
	bool     cv2_free = true;
	uint16_t cv2_note;
};

template <typename Antz_ifaces>
class MIDI_msg
{
	public:
		virtual void process( const uint8_t * buffer ) = 0;
};

template <typename Antz_ifaces>
class MIDI_CVM: public MIDI_msg<Antz_ifaces>  // Channel Voice Message
{
	protected:
		static MIDI_status _status;
	public:
		static void set_cv2( bool value )
		{
			if( value == _status.cv2_on )
				return;
			_status.cv2_on   = value;
			_status.cv2_note = 0;
		}
};

#define MIDI_VOLTAGE_MAX 255
#define MIDI_NOTE_MIN 36
#define MIDI_NOTE_MAX 96
#define MIDI_NOTE_TOTAL (MIDI_NOTE_MAX - MIDI_NOTE_MIN)

template <typename Antz_ifaces>
class MIDI_CVM_depress: public MIDI_CVM<Antz_ifaces>
{
	typedef decltype(Antz_ifaces::_Antz_view) _Antz_view;
	using MIDI_CVM<Antz_ifaces>::_status;

	public:
		virtual void process( const uint8_t * buffer )
		{
			const uint16_t note   = buffer[0];
			const uint8_t voltage = ((note - MIDI_NOTE_MIN) * MIDI_VOLTAGE_MAX) / MIDI_NOTE_TOTAL;
			if( _status.cv1_free )
			{
				_status.cv1_note = note;
				_status.cv1_free = false;
				if( note >= MIDI_NOTE_MIN && note < MIDI_NOTE_MAX )
				{
					_Antz_view::set_cv1( voltage );
					if( !_status.cv2_on )
						_Antz_view::set_vel( buffer[1] << 1 );
				}
			}
			else if( _status.cv2_on && _status.cv2_free )
			{
				_status.cv2_note = note;
				_status.cv2_free = false;
				if( note >= MIDI_NOTE_MIN && note < MIDI_NOTE_MAX )
					_Antz_view::set_cv2( voltage );
			}
			else // Ignore msg
				return;

			_Antz_view::set_note( true );
			if( note >= MIDI_NOTE_MIN && note < MIDI_NOTE_MAX )
				_Antz_view::set_gate( true );
			DBG( "Depress data '%0x %0x'\n", buffer[0], buffer[1] );
		}
};

template <typename Antz_ifaces>
class MIDI_CVM_release: public MIDI_CVM<Antz_ifaces>
{
	typedef decltype(Antz_ifaces::_Antz_view) _Antz_view;
	using MIDI_CVM<Antz_ifaces>::_status;

	public:
		virtual void process( const uint8_t * buffer )
		{
			const uint16_t note   = buffer[0];
			if( !_status.cv1_free && _status.cv1_note == note )
			{
				_status.cv1_free = true;
			}
			else if( !_status.cv2_free && _status.cv2_on && _status.cv2_note == note )
			{
				_status.cv2_free = true;
			}
			else // Ignore msg
				return;

			if( _status.cv1_free && _status.cv2_free )
			{
				_Antz_view::set_note( false );
				if( note >= MIDI_NOTE_MIN && note < MIDI_NOTE_MAX )
					_Antz_view::set_gate( false );
			}
			DBG( "Release data '%0x %0x'\n", buffer[0], buffer[1] );
		}
};



