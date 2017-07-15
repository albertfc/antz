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
	uint8_t  cv1_note = 0;
	bool     cv2_on   = false;
	bool     cv2_free = true;
	uint8_t  cv2_note = 0;
};

template <typename Antz_ifaces>
class MIDI_msg
{
	protected:
		static MIDI_status _status;

	public:
		virtual void process( const uint8_t * buffer ) = 0;
};

template <typename Antz_ifaces>
class MIDI_CMM: public MIDI_msg<Antz_ifaces>  // Channel Mode Message
{};

template <typename Antz_ifaces>
class MIDI_CMM_all_notes_off: public MIDI_CMM<Antz_ifaces>
{
	typedef decltype(Antz_ifaces::_Antz_view) _Antz_view;
	using MIDI_msg<Antz_ifaces>::_status;

	public:
		virtual void process( const uint8_t * buffer )
		{
			if( buffer[0] != 123 || buffer[1] != 0 )
				return;

			_status.cv1_free = true;
			_status.cv2_free = true;
			_Antz_view::set_note( false );
			_Antz_view::set_gate( false );
		}
};

static constexpr uint32_t MIDI_VOLTAGE_MAX = 4095L;
static constexpr uint32_t MIDI_NOTE_MIN    = 36UL;
static constexpr uint32_t MIDI_NOTE_MAX    = 96UL;
static constexpr uint32_t MIDI_NOTE_TOTAL  = (MIDI_NOTE_MAX - MIDI_NOTE_MIN);

template <typename Antz_ifaces>
class MIDI_CVM: public MIDI_msg<Antz_ifaces>  // Channel Voice Message
{
	typedef decltype(Antz_ifaces::_Antz_view) _Antz_view;

	protected:
		using MIDI_msg<Antz_ifaces>::_status;

		void depress( const uint8_t * buffer )
		{
			// Note On with velocity of 0 == Note Off
			if( buffer[1] == 0 )
				return release( buffer );

			const uint32_t note    = buffer[0];
			const uint16_t voltage = ((note - MIDI_NOTE_MIN) * MIDI_VOLTAGE_MAX) / MIDI_NOTE_TOTAL;
			if( _status.cv1_free )
			{
				_status.cv1_note = note;
				_status.cv1_free = false;
				if( note >= MIDI_NOTE_MIN && note < MIDI_NOTE_MAX )
				{
					_Antz_view::set_cv1( voltage );
					if( !_status.cv2_on )
						_Antz_view::set_vel( buffer[1] << 5 );
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

		void release( const uint8_t * buffer )
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

	public:
		static void set_cv2( bool value )
		{
			if( value == _status.cv2_on )
				return;
			_status.cv2_on   = value;
			_status.cv2_note = 0;
		}
};

template <typename Antz_ifaces>
class MIDI_CVM_depress: public MIDI_CVM<Antz_ifaces>
{
	public:
		virtual void process( const uint8_t * buffer )
		{
			return MIDI_CVM<Antz_ifaces>::depress( buffer );
		}
};

template <typename Antz_ifaces>
class MIDI_CVM_release: public MIDI_CVM<Antz_ifaces>
{
	public:
		virtual void process( const uint8_t * buffer )
		{
			return MIDI_CVM<Antz_ifaces>::release( buffer );
		}
};

template <typename Antz_ifaces>
class MIDI_CVM_pitch_bend: public MIDI_CVM<Antz_ifaces>
{
	typedef decltype(Antz_ifaces::_Antz_view) _Antz_view;

	protected:
		using MIDI_CVM<Antz_ifaces>::_status;

		uint16_t get_bended_voltage( const uint8_t * data_bytes, uint32_t note )
		{
			note = note <  MIDI_NOTE_MIN ? MIDI_NOTE_MIN
			     : note >= MIDI_NOTE_MAX ? MIDI_NOTE_MAX-1
			     : note;
			// transform pitch data to a signed int centered in 0 with +/- (1<<13 -1)
			const int16_t pitch = (((uint16_t)data_bytes[1] << 7) + data_bytes[0]) - 0x1fff - 1;
			// Compute voltage using fixed point and apply a pitch bend equivalent to 2 semitones
			int32_t voltage  = ((((note - MIDI_NOTE_MIN) << 13) + 2*pitch) * MIDI_VOLTAGE_MAX);
			        voltage /= (int32_t)MIDI_NOTE_TOTAL<<13; // CAUTION: division between signed and unsigned produces garbage!
			// Protect voltage from out of bounds values
			return voltage > (int32_t)MIDI_VOLTAGE_MAX ? MIDI_VOLTAGE_MAX
			     : voltage <                         0 ?                0
			     :                                                voltage;
		}

	public:
		virtual void process( const uint8_t * buffer )
		{
			if( _status.cv1_note )
				_Antz_view::set_cv1( get_bended_voltage( buffer, _status.cv1_note ) );
			if( _status.cv2_note && _status.cv2_on )
				_Antz_view::set_cv2( get_bended_voltage( buffer, _status.cv2_note ) );
		}
};
