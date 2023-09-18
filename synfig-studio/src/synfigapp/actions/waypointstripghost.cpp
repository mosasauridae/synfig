/* === S Y N F I G ========================================================= */
/*!	\file waypointstripghost.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/general.h>

#include "waypointstripghost.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::WaypointStripGhost);
ACTION_SET_NAME(Action::WaypointStripGhost,"WaypointStripGhost");
ACTION_SET_LOCAL_NAME(Action::WaypointStripGhost,N_("Strip Ghost Tag"));
ACTION_SET_TASK(Action::WaypointStripGhost,"stripghost");
ACTION_SET_CATEGORY(Action::WaypointStripGhost,Action::CATEGORY_WAYPOINT);
ACTION_SET_PRIORITY(Action::WaypointStripGhost,0);
ACTION_SET_VERSION(Action::WaypointStripGhost,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::WaypointStripGhost::WaypointStripGhost()
{
	waypoint.set_time(Time::begin()-1);
	set_dirty(true);
}

Action::ParamVocab
Action::WaypointStripGhost::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode (Animated)"))
	);

	ret.push_back(ParamDesc("waypoint",Param::TYPE_WAYPOINT)
		.set_local_name(_("Waypoint"))
		.set_desc(_("Waypoint to be modified"))
	);

	return ret;
}

bool
Action::WaypointStripGhost::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::WaypointStripGhost::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=ValueNode_Animated::Handle::cast_dynamic(param.get_value_node());

		return static_cast<bool>(value_node);
	}
	if(name=="waypoint" && param.get_type()==Param::TYPE_WAYPOINT)
	{
		waypoint=param.get_waypoint();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::WaypointStripGhost::is_ready()const
{
	if(!value_node || waypoint.get_time()==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::WaypointStripGhost::perform()
{
	WaypointList::iterator iter(value_node->find(waypoint));

	if((UniqueID)*iter!=(UniqueID)waypoint)
		throw Error(_("UniqueID mismatch, iter=%d, waypoint=%d"),iter->get_uid(),waypoint.get_uid());

	if(iter->get_time()!=waypoint.get_time())
		throw Error(_("Time mismatch iter=%s, waypoint=%s"),iter->get_time().get_string().c_str(),waypoint.get_time().get_string().c_str());

	origGhostFlag = waypoint.is_ghost();
	iter->set_ghost(false);

	value_node->changed();
}

void
Action::WaypointStripGhost::undo()
{
	WaypointList::iterator iter(value_node->find(waypoint));

	if((UniqueID)*iter!=(UniqueID)waypoint)
		throw Error(_("UniqueID mismatch, iter=%d, waypoint=%d"),iter->get_uid(),waypoint.get_uid());

	if(iter->get_time()!=waypoint.get_time())
		throw Error(_("Time mismatch iter=%s, waypoint=%s"),iter->get_time().get_string().c_str(),waypoint.get_time().get_string().c_str());

	iter->set_ghost(origGhostFlag);

	value_node->changed();
}
