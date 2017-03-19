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

#define REG_PASTER( _x, _y )     _x ## _y
#define REG_EVALUATOR( _x, _y )  REG_PASTER( _x, _y )
#define SET_PORT( _letter )      REG_EVALUATOR( PORT, _letter )
#define SET_DDR( _letter )       REG_EVALUATOR( DDR, _letter )
#define SET_DD( _letter, _num )  REG_EVALUATOR( DD, REG_EVALUATOR( _letter, _num ) )
#define SET_P( _letter, _num )   REG_EVALUATOR( P, REG_EVALUATOR( _letter, _num ) )
#define SET_PIN( _letter, _num ) REG_EVALUATOR( PIN, _letter )

#define PORT( _a ) SET_PORT( _a ## _REG )
#define DDR( _a )  SET_DDR( _a ## _REG )
#define DD( _a )   SET_DD( _a ## _REG, _a ## _PIN )
#define PIN( _a )  SET_PIN( _a ## _REG, _a ## _PIN )
#define P( _a )    SET_P( _a ## _REG, _a ## _PIN )

#define READ_PIN( _a ) (PIN( _a ) & _BV( P( _a ) ))
#define WRT0_PIN( _a ) PORT( _a ) &=~_BV( P( _a ) )
#define WRT1_PIN( _a ) PORT( _a ) |= _BV( P( _a ) )

