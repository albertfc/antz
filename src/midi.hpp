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

#include "ustl/uvector.h"
#include "ustl/ualgo.h"

#include "log.h"

struct MIDI_status
{
	ustl::vector<uint8_t> stack;
	bool     cv1_free;
	uint8_t  cv1_note;
	bool     cv2_on;
	bool     cv2_free;
	uint8_t  cv2_note;
	uint8_t  pitch_bend[2];

	MIDI_status(): stack(128)
	{
		reset();
	}

	void reset( void )
	{
		stack.resize(0);
		cv1_free = true;
		cv1_note = 0;
		cv2_on   = false;
		cv2_free = true;
		cv2_note = 0;
		pitch_bend[0] = 0;
		pitch_bend[1] = 64; // No bend
	}
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

			_status.reset();
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

		bool is_note_in_range( const uint8_t note )
		{
			return ( note >= MIDI_NOTE_MIN && note < MIDI_NOTE_MAX );
		}

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

		uint16_t compute_voltage( uint32_t note )
		{
			return get_bended_voltage( _status.pitch_bend, note );
		}

		void cv1_play_note( const uint8_t note, const uint8_t vel = 0 )
		{
			_status.cv1_note = note;
			_status.cv1_free = false;
			_Antz_view::set_note( true );
			if( !is_note_in_range( note ) )
				return;

			_Antz_view::set_cv1( compute_voltage( note ) );
			if( !_status.cv2_on )
				_Antz_view::set_vel( vel << 5 );
			_Antz_view::set_gate( true );
		}

		void cv2_play_note( const uint32_t note )
		{
			_status.cv2_note = note;
			_status.cv2_free = false;
			_Antz_view::set_note( true );
			if( !is_note_in_range( note ) )
				return;

			_Antz_view::set_cv2( compute_voltage( note ) );
			_Antz_view::set_gate( true );
		}

		void depress( const uint8_t * buffer )
		{
			DBG( "Depress data '%0x %0x'\n", buffer[0], buffer[1] );

			// Note On with velocity of 0 == Note Off
			if( buffer[1] == 0 )
				return release( buffer );

			const uint32_t note = buffer[0];

			if( !_status.cv2_on ) // monophonic mode
			{
				// Play note
				cv1_play_note( note, buffer[1] );
				// Push note only if it hasn't already been depressed (it isn't in the stack)
				if( _status.stack.end() == ustl::find( _status.stack.begin(), _status.stack.end(), note ))
					_status.stack.push_back( note );
			}
			else // paraphonic
			{
				// Handle note only if it hasn't already been depressed (it is being played or it  isn't in the stack)
				if( (!_status.cv1_free && _status.cv1_note == note)
				 || (!_status.cv2_free && _status.cv2_note == note)
				 || _status.stack.end() != ustl::find( _status.stack.begin(), _status.stack.end(), note ))
					return;

				// Play note if there is a free channel, otherwise push it.
				if( _status.cv1_free )
					cv1_play_note( note );
				else if( _status.cv2_free )
					cv2_play_note( note );
				else
					_status.stack.push_back( note );
			}
		}

		void release( const uint8_t * buffer )
		{
			DBG( "Release data '%0x %0x'\n", buffer[0], buffer[1] );

			const uint8_t note = buffer[0];

			if( !_status.cv2_on ) // monphonic mode
			{
				// Remove released note from stack
				auto iterator = ustl::find( _status.stack.begin(), _status.stack.end(), note );
				if( iterator != _status.stack.end() ) // Note must be in queue
					_status.stack.erase( iterator );

				if( _status.stack.size() ) // If there are notes to play, pop it and play.
					cv1_play_note( _status.stack.back(), buffer[1] );
				else { // No more notes to play
					_Antz_view::set_note( false );
					_Antz_view::set_gate( false );
				}
			}
			else // paraphonic mode
			{
				// Search for a free channel. If there is at least one note in the 
				// stack, pop it and play.
				if( !_status.cv1_free && _status.cv1_note == note ) {
					if( _status.stack.size() ) {
						cv1_play_note( _status.stack.back() );
						_status.stack.pop_back();
					} else {
						_status.cv1_free = true;
					}
				} else if( !_status.cv2_free && _status.cv2_note == note ) {
					if( _status.stack.size() ) {
						cv2_play_note( _status.stack.back() );
						_status.stack.pop_back();
					} else {
						_status.cv2_free = true;
					}
				}
				else { // No free channels. Remove note from stack
					auto iterator = ustl::find( _status.stack.begin(), _status.stack.end(), note );
					if( iterator != _status.stack.end() ) // Note must be in queue
						_status.stack.erase( iterator );
				}

				if( _status.cv1_free && _status.cv2_free ) // No more notes to play
				{
					_Antz_view::set_note( false );
					_Antz_view::set_gate( false );
				}
			}
		}

	public:
		static void set_cv2( bool value )
		{
			if( value == _status.cv2_on )
				return;
			_status.reset();
			_status.cv2_on = value;
			_Antz_view::set_note( false );
			_Antz_view::set_gate( false );
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

	public:
		virtual void process( const uint8_t * buffer )
		{
			// Save pitch value
			_status.pitch_bend[0] = buffer[0];
			_status.pitch_bend[1] = buffer[1];

			// Apply pitch if needed
			if( _status.cv1_note )
				_Antz_view::set_cv1( MIDI_CVM<Antz_ifaces>::get_bended_voltage( _status.pitch_bend, _status.cv1_note ) );
			if( _status.cv2_note && _status.cv2_on )
				_Antz_view::set_cv2( MIDI_CVM<Antz_ifaces>::get_bended_voltage( _status.pitch_bend, _status.cv2_note ) );
		}
};
