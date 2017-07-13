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
#include <stdint.h>

//// MVC arch pattern

// View and iterfaces
template <typename Impl_Antz_view>
struct Iface_Antz_view
{
	static void set_note( bool value )
		{ Impl_Antz_view::set_note_impl( value ); }
	static void set_gate( bool value )
		{ Impl_Antz_view::set_gate_impl( value ); }
	static void set_cv1( uint16_t value )
		{ Impl_Antz_view::set_cv1_impl( value ); }
	static void set_cv2( uint16_t value )
		{ Impl_Antz_view::set_cv2_impl( value ); }
	static void set_vel( uint16_t value )
		{ Impl_Antz_view::set_vel_impl( value ); }

	private:
		Iface_Antz_view() {} // Pure static class. Disallow creating an instance of this object
};

template <typename Impl_Antz_view>
struct Antz_interfaces
{
	static Iface_Antz_view<Impl_Antz_view> _Antz_view;

	private:
		Antz_interfaces() {} // Pure static class. Disallow creating an instance of this object
};


// Model
#include "midi.hpp"

#define ANTZ_MODEL_STATIC_INIT( Impl_Antz_view ) \
	typedef Antz_interfaces<Impl_Antz_view>  _Antz_ifaces; \
	template<> \
	MIDI_status MIDI_msg<_Antz_ifaces>::_status = MIDI_status(); \
	template<> \
	MIDI_CMM_all_notes_off<_Antz_ifaces> Antz_model<Impl_Antz_view>::all_notes_off = MIDI_CMM_all_notes_off<_Antz_ifaces>(); \
	template<> \
	MIDI_CVM_depress<_Antz_ifaces> Antz_model<Impl_Antz_view>::depress = MIDI_CVM_depress<_Antz_ifaces>(); \
	template<> \
	MIDI_CVM_release<_Antz_ifaces> Antz_model<Impl_Antz_view>::release = MIDI_CVM_release<_Antz_ifaces>(); \
	template<> \
	MIDI_CVM_pitch_bend<_Antz_ifaces> Antz_model<Impl_Antz_view>::pitch_bend = MIDI_CVM_pitch_bend<_Antz_ifaces>(); \
	template<> \
	MIDI_msg<_Antz_ifaces> * const Antz_model<Impl_Antz_view >::strategies[]  = { \
		&Antz_model<Impl_Antz_view >::release, \
		&Antz_model<Impl_Antz_view >::depress, \
		&Antz_model<Impl_Antz_view >::all_notes_off, \
		&Antz_model<Impl_Antz_view >::pitch_bend \
	};

template <typename Impl_Antz_view>
class Antz_model
{
	private:
		Antz_model() {} // Pure static class. Disallow creating an instance of this object
		typedef Antz_interfaces<Impl_Antz_view>  _Antz_ifaces;

		static int8_t get_strategy_idx( const uint8_t status )
		{
			// Maps msg_supported -> strategies_idx       Note off  Note On   All off   P. bend
			static constexpr uint8_t msg_supported[4] = { 0b1000,   0b1001,   0b1011,   0b1110}; // Must be sorted!

			uint8_t idx = 0;
			while( status > msg_supported[idx] && idx++ < sizeof( msg_supported ) );
			return idx < sizeof( msg_supported ) && msg_supported[idx] == status ? idx : -1;
		}

	public:
		// Strategy pattern
		static MIDI_CMM_all_notes_off<_Antz_ifaces> all_notes_off;
		static MIDI_CVM_depress<_Antz_ifaces> depress;
		static MIDI_CVM_release<_Antz_ifaces> release;
		static MIDI_CVM_pitch_bend<_Antz_ifaces> pitch_bend;
		static MIDI_msg<_Antz_ifaces> * const strategies[4];

		static void process_msg( const uint8_t * msg )
		{
			// get message idx base on status msg
			const int8_t idx = get_strategy_idx( msg[0] >> 4 );
			if( idx >= 0 )
				strategies[ idx ]->process( &msg[1] );
#ifndef NDEBUG
			else
				DBG( "unknown status\n" );
#endif
		}
		static void set_cv2( bool value )
		{
			MIDI_CVM<_Antz_ifaces>::set_cv2( value );
		}
};

// Controller
template <typename Impl_Antz_view>
class Antz_controller
{
	private:
		Antz_controller() {} // Pure static class. Disallow creating an instance of this object
		typedef Antz_model<Impl_Antz_view>  _Antz_model;

	public:
		static void process_msg( const uint8_t * msg )
		{
			_Antz_model::process_msg( msg );
		}
		static void set_cv2( bool value )
		{
			_Antz_model::set_cv2( value );
		}
};
