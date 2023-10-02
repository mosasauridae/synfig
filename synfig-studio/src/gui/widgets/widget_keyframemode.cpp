/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_interpolation.h
**	\brief Widget for interpolation selection
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2022      Rodolfo R. Gomes
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "widget_keyframemode.h"

#include <gui/localization.h>
#include <synfig/paramdesc.h>

#endif

using namespace synfig;
using namespace studio;

Widget_KeyframeMode::Widget_KeyframeMode(KeyframeMode mode)
{
	const auto opts = ParamDesc("mode")
			.set_hint("enum")
			.add_enum_value(KEYFRAMEMODE_MOVE_AND_SCALE, "move_and_scale", _("Move and Scale"))
			.add_enum_value(KEYFRAMEMODE_MOVE_ONLY, "move_only", _("Move Only"))
			.add_enum_value(KEYFRAMEMODE_NO_MOVE, "no_move", _("No Move"));

	set_param_desc(opts);
}
