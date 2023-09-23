/* === S Y N F I G ========================================================= */
/*!	\file canvas.h
**	\brief Canvas Class Implementation
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIGCANVASVIEWPARAMS_H
#define __SYNFIGCANVASVIEWPARAMS_H

/* === H E A D E R S ======================================================= */

#include <synfig/time.h>
#include <synfig/vector.h>
#include <synfigapp/editmode.h>

namespace studio {

/*!	\class CanvasViewParams
**	\brief Collection of parameters that represents the state of the canvas view.
* This includes properties like like current frame and canvas zoom.
*
* This collection only stores raw values, and nothing that would reference a
* handle in the canvas.
*
*/
class CanvasViewParams
{
private:

	//! Current playback time
	synfig::Time::value_type current_time_;

public:
	synfig::Time::value_type play_bounds_lower_;
	synfig::Time::value_type play_bounds_upper_;
	bool play_bounds_enable_;
	bool play_repeat_;

	float work_area_zoom_;
	synfig::Point work_area_focus_;

	synfigapp::EditMode mode_;

public:

	//! Gets the current time
	synfig::Time::value_type get_current_time() const { return current_time_; }

	//! Sets the current time
	void set_current_time(synfig::Time::value_type x) { current_time_ = x; }

}; // END of class CanvasViewParams

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
