/* === S Y N F I G ========================================================= */
/*!	\file timepointsmove.cpp
**	\brief Move the Time Points File
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
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

#include "timepointsmove.h"
#include <synfig/layers/layer_pastecanvas.h>
#include <synfigapp/canvasinterface.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>
#include <synfig/valuenodes/valuenode_animated.h>

#include "activepointremove.h"
#include "waypointremove.h"
#include "activepointset.h"
#include "waypointset.h"
#include "timepointsdelete.h"
#include <synfigapp/timegather.h>

#include <typeinfo>

#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::TimepointsMove);
ACTION_SET_NAME(Action::TimepointsMove,"TimepointsMove");
ACTION_SET_LOCAL_NAME(Action::TimepointsMove,N_("Move Time Points"));
ACTION_SET_TASK(Action::TimepointsMove,"move");
ACTION_SET_CATEGORY(Action::TimepointsMove,Action::CATEGORY_WAYPOINT|Action::CATEGORY_ACTIVEPOINT);
ACTION_SET_PRIORITY(Action::TimepointsMove,0);
ACTION_SET_VERSION(Action::TimepointsMove,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::TimepointsMove::TimepointsMove()
	: timemove(0)
{
	set_dirty(false);
}

Action::ParamVocab
Action::TimepointsMove::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("addlayer",Param::TYPE_VALUE)
		.set_local_name(_("New Selected Layer"))
		.set_desc(_("A layer to add to our selected list"))
		.set_supports_multiple()
		.set_optional()
	);

	ret.push_back(ParamDesc("addcanvas",Param::TYPE_CANVAS)
		.set_local_name(_("New Selected Canvas"))
		.set_desc(_("A canvas to add to our selected list"))
		.set_supports_multiple()
		.set_optional()
	);

	ret.push_back(ParamDesc("addvaluedesc",Param::TYPE_VALUEDESC)
		.set_local_name(_("New Selected ValueBase"))
		.set_desc(_("A valuenode's description to add to our selected list"))
		.set_supports_multiple()
		.set_optional()
	);

	ret.push_back(ParamDesc("addtime",Param::TYPE_TIME)
		.set_local_name(_("New Selected Time Point"))
		.set_desc(_("A time point to add to our selected list"))
		.set_supports_multiple()
	);

	ret.push_back(ParamDesc("deltatime",Param::TYPE_TIME)
		.set_local_name(_("Time adjustment"))
		.set_desc(_("The amount of time to adjust all the selected points"))
	);

	return ret;
}

bool
Action::TimepointsMove::is_candidate(const ParamList &x)
{
	if(!candidate_check(get_param_vocab(),x))
		return false;

	if(	x.find("addlayer") == x.end() &&
		x.find("addcanvas") == x.end() &&
		x.find("addvaluedesc") == x.end())
		return false;
	return true;
}

bool
Action::TimepointsMove::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="addlayer" && param.get_type()==Param::TYPE_LAYER)
	{
		//add a layer to the list
		sel_layers.push_back(param.get_layer());
		//synfig::info("action got layer");

		return true;
	}

	if(name=="addcanvas" && param.get_type()==Param::TYPE_CANVAS)
	{
		//add a layer to the list
		sel_canvases.push_back(param.get_canvas());
		//synfig::info("action got canvas");

		return true;
	}

	if(name=="addvaluedesc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		//add a layer to the list
		sel_values.push_back(param.get_value_desc());
		//synfig::info("action got valuedesc");

		return true;
	}

	if(name=="addtime" && param.get_type()==Param::TYPE_TIME)
	{
		//add a layer to the list
		sel_times.insert(param.get_time());
		//synfig::info("action got time");

		return true;
	}

	if(name=="deltatime" && param.get_type()==Param::TYPE_TIME)
	{
		timemove = param.get_time();
		//synfig::info("action got time to move");

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::TimepointsMove::is_ready()const
{
	if((sel_layers.empty() && sel_canvases.empty() && sel_values.empty()) || sel_times.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::TimepointsMove::prepare()
{
	clear();

	//synfig::info("Preparing TimepointsMove by %f secs",(float)timemove);

	if(sel_times.empty()) return;

	//all our lists should be set correctly...

	/*{
		std::set<synfig::Time>::iterator i = sel_times.begin(), end = sel_times.end();

		for(; i != end; ++i)
		{
			synfig::info("Time %f", (float)*i);
		}
	}*/

	//build our sub-action list
	// 	and yes we do need to store it temporarily so we don't duplicate
	//		an operation on a specific valuenode, etc....
	timepoints_ref	match;
	timepoints_ref	match_del;

	Time fps = get_canvas()->rend_desc().get_frame_rate();

	// need to delete waypoints at the destination
	std::set<synfig::Time> del_times;
	for (const synfig::Time& sel_time : sel_times)
	{
		del_times.insert((sel_time + timemove).round(fps));
	}

	//std::vector<synfig::Layer::Handle>
	//synfig::info("Layers %d", sel_layers.size());
	{
		std::vector<synfig::Layer::Handle>::iterator i = sel_layers.begin(),
													end = sel_layers.end();

		for(; i != end; ++i)
		{
			//synfig::info("Recurse through a layer");
			recurse_layer(*i,sel_times,match);
			recurse_layer(*i,del_times,match_del);
		}
	}

	//std::vector<synfig::Canvas::Handle>	sel_canvases;
	//synfig::info("Canvases %d", sel_canvases.size());
	{
		std::vector<synfig::Canvas::Handle>::iterator 	i = sel_canvases.begin(),
														end = sel_canvases.end();

		for(; i != end; ++i)
		{
			//synfig::info("Recurse through a canvas");
			recurse_canvas(*i,sel_times,match);
			recurse_canvas(*i,del_times,match_del);
		}
	}

	//std::vector<synfigapp::ValueDesc>
	//synfig::info("ValueBasedescs %d", sel_values.size());
	{
		std::vector<synfigapp::ValueDesc>::iterator 	i = sel_values.begin(),
													end = sel_values.end();

		for(; i != end; ++i)
		{
			//synfig::info("Recurse through a valuedesc");
			recurse_valuedesc(*i,sel_times,match);
			recurse_valuedesc(*i,del_times,match_del);
		}
	}              
        
	// filter down the delete list to those waypoints that would actually be overwritten,
	// and be careful to not delete a moving waypoint
	for (const ValueBaseTimeInfo& deleting : match_del.waypointbiglist)
	{
		for (auto del_wpt_iter = deleting.waypoints.begin(); del_wpt_iter != deleting.waypoints.end(); )
		{
			const synfig::Waypoint& del_wpt = *del_wpt_iter;

			bool is_moving_waypoint = false;
			bool any_moving_to_time = false;
			for (const ValueBaseTimeInfo& moving : match.waypointbiglist)
			{
				for (const synfig::Waypoint& mov_wpt : moving.waypoints)
				{
					// don't delete waypoints that are moving
					if (del_wpt.get_guid() == mov_wpt.get_guid())
					{
						is_moving_waypoint = true;
						break;
					}
					else if (del_wpt.get_parent_value_node() == mov_wpt.get_parent_value_node() &&
							 del_wpt.get_time() == (mov_wpt.get_time() + timemove).round(fps))
					{
						any_moving_to_time = true;
					}
				}
			}

			if (is_moving_waypoint || !any_moving_to_time)
			{
				del_wpt_iter = deleting.waypoints.erase(del_wpt_iter);
			}
			else
			{
				++del_wpt_iter;
			}

		}
	}
	for (const ActiveTimeInfo& i : match.actpointbiglist)
	{
		for (const auto& w : i.activepoints)
		{
			for (const ActiveTimeInfo& j : match_del.actpointbiglist)
			{
				j.activepoints.erase(w);
			}
		}
	}

	//synfig::info("built list of waypoints/activepoints to modify");
	//synfig::info("\t There are %d waypoint sets and %d activepointsets",
	//				match.waypointbiglist.size(), match.actpointbiglist.size());
	//process them...
	{
		//must build from both lists
		timepoints_ref::waytracker::const_iterator 	i = match.waypointbiglist.begin(),
													iend = match.waypointbiglist.end();
		for(; i != iend; ++i)
		{
			Action::Handle action(WaypointSet::create());

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("value_node",ValueNode::Handle(i->val));

			synfig::Time dilated_timemove(timemove * i->time_dilation);

			//iterate through each waypoint for this specific valuenode
			std::set<synfig::Waypoint>::const_iterator 	j = i->waypoints.begin(),
														jend = i->waypoints.end();
			for(; j != jend; ++j)
			{
				//synfig::info("add waypoint mod...");
				//NOTE: We may want to store the old time for undoing the action...
				Waypoint w = *j;
				w.set_time((w.get_time() + dilated_timemove).round(fps));
				action->set_param("waypoint",w);
			}

			//run the action now that we've added everything
			assert(action->is_ready());
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action_front(action);
		}
	}
	{
		//must build from both lists
		timepoints_ref::acttracker::const_iterator 	i = match.actpointbiglist.begin(),
													end = match.actpointbiglist.end();
		for(; i != end; ++i)
		{
			Action::Handle action(ActivepointSet::create());

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("value_desc",i->val);

			synfig::Time dilated_timemove(timemove * i->time_dilation);

			//iterate through each activepoint for this specific valuenode
			std::set<synfig::Activepoint>::const_iterator 	j = i->activepoints.begin(),
															jend = i->activepoints.end();
			for(; j != jend; ++j)
			{
				//synfig::info("add activepoint mod...");

				//NOTE: We may want to store the old time for undoing the action...
				Activepoint a = *j;
				a.set_time((a.get_time() + dilated_timemove).round(fps));
				action->set_param("activepoint",a);
			}

			assert(action->is_ready());
			if(!action->is_ready())
			{
				throw Error(Error::TYPE_NOTREADY);
			}

			add_action_front(action);
		}
	}

	// process delete
	for (const Action::Handle& action : TimepointsDelete::makeActions(match_del, get_canvas(), get_canvas_interface()))
	{
		add_action_front(action);
	}
}

void
Action::TimepointsMove::perform()
{
	Action::Super::perform();
}
