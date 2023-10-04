/* === S Y N F I G ========================================================= */
/*!	\file WaypointAddMultiple.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "waypointaddmultiple.h"
#include "valuenodelinkconnect.h"
#include "valuenodereplace.h"

#include "waypointset.h"
#include "waypointadd.h"

#include "valuedescconnect.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/exception.h>
#include <synfigapp/main.h>

#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::WaypointAddMultiple);
ACTION_SET_NAME(Action::WaypointAddMultiple,"WaypointAddMultiple");
ACTION_SET_LOCAL_NAME(Action::WaypointAddMultiple,N_("Add Multiple Waypoints"));
ACTION_SET_TASK(Action::WaypointAddMultiple,"add_multiple");
ACTION_SET_CATEGORY(Action::WaypointAddMultiple,Action::CATEGORY_WAYPOINT|Action::CATEGORY_VALUEDESC|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::WaypointAddMultiple,0);
ACTION_SET_VERSION(Action::WaypointAddMultiple,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::WaypointAddMultiple::WaypointAddMultiple()
{
	set_dirty(true);
}

Action::ParamVocab
Action::WaypointAddMultiple::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
		.set_supports_multiple()
		.set_optional()
	);

	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("Destination ValueNode (Animated)"))
		.set_supports_multiple()
		.set_optional()
	);

	ret.push_back(ParamDesc("waypoint",Param::TYPE_WAYPOINT)
		.set_local_name(_("New Waypoint"))
		.set_desc(_("Waypoint to be added"))
		.set_optional()
	);

	ret.push_back(ParamDesc("waypoint_model",Param::TYPE_WAYPOINTMODEL)
		.set_local_name(_("Waypoint Model"))
		.set_optional()
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_desc(_("Time where waypoint is to be added"))
		.set_optional()
	);

	return ret;
}

bool
Action::WaypointAddMultiple::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(),x))
		return false;

	auto value_desc_range = x.equal_range("value_desc");
	auto value_node_range = x.equal_range("value_node");

	if (std::distance(value_desc_range.first, value_desc_range.second) + std::distance(value_node_range.first, value_node_range.second) < 4)
		return false;

	for (auto it = value_desc_range.first; it != value_desc_range.second; ++it)
	{
		ValueDesc value_desc(it->second.get_value_desc());
		bool can_add_waypoint =
				// Parameter has to be animatable
				!(value_desc).get_static() &&
				//no expandable parameters
				!(LinkableValueNode::Handle::cast_dynamic(value_desc.get_value_node()));

		if (!can_add_waypoint)
			return false;
	}

	return true;
}

bool
Action::WaypointAddMultiple::set_param(const synfig::String& name, const Action::Param &param)
{
	if (name == "value_desc" || name == "value_node")
	{
		GUID guid;
		if (name == "value_desc" && param.get_type() == Param::TYPE_VALUEDESC)
			guid = param.get_value_desc().get_value_node()->get_guid();
		else if (name == "value_node" && param.get_type() == Param::TYPE_VALUENODE)
			guid = param.get_value_node()->get_guid();
		else
			return false;

		auto it = subactions.find(guid);
		if (it != subactions.end())
		{
			it->second->set_param(name, param);
		}
		else
		{
			synfigapp::Action::Handle action(synfigapp::Action::create("WaypointSetSmart"));
			action->set_param_list(common_params);
			action->set_param(name, param);
			subactions[guid] = action;
		}
	}
	else
	{
		common_params.add(name, param);

		for (const auto& p : subactions)
		{
			if (!p.second->set_param(name, param))
				return false;
		}
	}

	Action::CanvasSpecific::set_param(name,param);
	return true;
}

bool
Action::WaypointAddMultiple::is_ready()const
{
	for (const auto& p : subactions)
	{
		if (!p.second->is_ready())
			return false;
	}
	return true;
}

void
Action::WaypointAddMultiple::prepare()
{
	clear();

	for (const auto& p : subactions)
	{
		add_action_front(p.second);
	}
}

