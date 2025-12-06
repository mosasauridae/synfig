/* === S Y N F I G ========================================================= */
/*!	\file waypointremove.cpp
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

#include "removeframes.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>
#include <synfig/valuenodes/valuenode_animated.h>

#include <synfigapp/localization.h>
#include <synfigapp/actions/activepointremove.h>
#include <synfigapp/actions/activepointsetsmart.h>
#include <synfigapp/actions/canvasrenddescset.h>
#include <synfigapp/actions/keyframeremove.h>
#include <synfigapp/actions/keyframeset.h>
#include <synfigapp/actions/waypointremove.h>
#include <synfigapp/actions/waypointset.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::RemoveFrames);
ACTION_SET_NAME(Action::RemoveFrames,"RemoveFrames");
ACTION_SET_LOCAL_NAME(Action::RemoveFrames,N_("Remove Frames"));
ACTION_SET_TASK(Action::RemoveFrames,"remove");
ACTION_SET_CATEGORY(Action::RemoveFrames,Action::CATEGORY_CANVAS);
ACTION_SET_PRIORITY(Action::RemoveFrames,0);
ACTION_SET_VERSION(Action::RemoveFrames,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::RemoveFrames::RemoveFrames()
    : time_(Time::begin() - 1),
      duration_(Time::begin() - 1)
{
	set_dirty(true);
}

Action::ParamVocab
Action::RemoveFrames::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

    ret.push_back(ParamDesc("time",Param::TYPE_TIME)
        .set_local_name(_("Time"))
	);

    ret.push_back(ParamDesc("duration",Param::TYPE_TIME)
        .set_local_name(_("Duration"))
	);

	return ret;
}

synfig::String
Action::RemoveFrames::get_local_name()const
{
    auto fps = get_canvas()->rend_desc().get_frame_rate();

    return strprintf(_("Remove Frames: %s at %s"),
                        duration_.get_string(fps).c_str(),
                        time_.get_string(fps).c_str());
}

bool
Action::RemoveFrames::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::RemoveFrames::set_param(const synfig::String& name, const Action::Param &param)
{
    if(name=="time" && param.get_type()==Param::TYPE_TIME)
    {
        time_=param.get_time();
        return true;
	}
    if(name=="duration" && param.get_type()==Param::TYPE_TIME)
    {
        duration_=param.get_time();
        return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::RemoveFrames::is_ready()const
{
    if (time_ == (Time::begin()-1) || duration_ == (Time::begin()-1))
    {
        return false;
    }

    if (time_ > get_canvas()->rend_desc().get_time_end())
    {
        return false;
    }

    if (time_ < Time(0) || duration_ <= Time(0))
    {
        return false;
    }

	return Action::CanvasSpecific::is_ready();
}

void RemoveFrames::prepare()
{
    Canvas::Handle canvas = get_canvas();
    etl::loose_handle<synfigapp::CanvasInterface> canvas_interface = get_canvas_interface();

    if (time_ + duration_ > canvas->rend_desc().get_time_end())
        duration_ = canvas->rend_desc().get_time_end() - time_;

    // update canvas
    synfig::RendDesc rend_desc = canvas->rend_desc();
    rend_desc.set_time_end(rend_desc.get_time_end() - duration_);

    Action::Handle action(CanvasRendDescSet::create());
    action->set_param("canvas", canvas);
    action->set_param("canvas_interface", canvas_interface);
    action->set_param("rend_desc", rend_desc);
    add_action(action);

    // update keyframes
    for (auto kf_iter = canvas->keyframe_list().begin(); kf_iter != canvas->keyframe_list().end(); kf_iter++)
    {
        const Keyframe& keyframe = *kf_iter;
        if (keyframe.get_time() >= time_ + duration_)
        {
            Keyframe newKeyframe = keyframe;
            newKeyframe.set_time(keyframe.get_time() - duration_);

            Action::Handle action(KeyframeSet::create());
            action->set_param("canvas", canvas);
            action->set_param("canvas_interface", canvas_interface);
            action->set_param("keyframe", newKeyframe);
            action->set_param("mode", KEYFRAMEMODE_NO_MOVE);
            add_action(action);
        }
        else if (keyframe.get_time() >= time_)
        {
            Action::Handle action(KeyframeRemove::create());
            action->set_param("canvas", canvas);
            action->set_param("canvas_interface", canvas_interface);
            action->set_param("keyframe", keyframe);
            action->set_param("mode", KEYFRAMEMODE_NO_MOVE);
            add_action(action);
        }
    }

    // update valuedescs
    std::vector<synfigapp::ValueDesc> vd_list;
    canvas_interface->find_important_value_descs(vd_list);

    for (auto vd_iter = vd_list.begin(); vd_iter != vd_list.end(); vd_iter++)
    {
        synfigapp::ValueDesc& value_desc = *vd_iter;
        if(value_desc.is_value_node())
        {
            ValueNode::Handle value_node(value_desc.get_value_node());

            // If we are a dynamic list, then we need to update the ActivePoints
            if(ValueNode_DynamicList::Handle::cast_dynamic(value_node))
            {
                ValueNode_DynamicList::Handle value_node_dynamic(ValueNode_DynamicList::Handle::cast_dynamic(value_node));
                for(int i=value_node_dynamic->link_count()-1; i>=0; i--)
                {
                    const auto& entry = value_node_dynamic->list[i];
                    for (auto en_iter = entry.timing_info.begin(); en_iter != entry.timing_info.end(); en_iter++)
                    {
                        if (en_iter->get_time() >= time_ + duration_)
                        {
                            Activepoint newEntry = *en_iter;
                            newEntry.set_time(newEntry.get_time() - duration_);

                            Action::Handle action(ActivepointSetSmart::create());
                            action->set_param("canvas", canvas);
                            action->set_param("canvas_interface", canvas_interface);
                            action->set_param("value_desc", ValueDesc(value_node_dynamic, i));
                            action->set_param("activepoint", newEntry);
                            add_action(action);
                        }
                        else if (en_iter->get_time() >= time_)
                        {
                            Action::Handle action(ActivepointRemove::create());
                            action->set_param("canvas", canvas);
                            action->set_param("canvas_interface", canvas_interface);
                            action->set_param("value_desc", ValueDesc(value_node_dynamic, i));
                            action->set_param("activepoint", *en_iter);
                            add_action(action);
                        }
                    }
                }
            }
            else if(ValueNode_Animated::Handle::cast_dynamic(value_node))
            {
                ValueNode_Animated::Handle value_node_animated(ValueNode_Animated::Handle::cast_dynamic(value_node));

                const auto& waypoint_list = value_node_animated->waypoint_list();
                for (auto wp_iter=waypoint_list.begin(); wp_iter != waypoint_list.end(); wp_iter++)
                {
                    if (wp_iter->get_time() >= time_ + duration_)
                    {
                        Waypoint newWaypoint = *wp_iter;
                        newWaypoint.set_time(wp_iter->get_time() - duration_);

                        Action::Handle action(WaypointSet::create());
                        action->set_param("canvas",get_canvas());
                        action->set_param("canvas_interface",get_canvas_interface());
                        action->set_param("value_node",ValueNode::Handle::cast_static(value_node));
                        action->set_param("waypoint",newWaypoint);
                        add_action(action);
                    }
                    else if (wp_iter->get_time() >= time_)
                    {
                        Action::Handle action(WaypointRemove::create());
                        action->set_param("canvas",get_canvas());
                        action->set_param("canvas_interface",get_canvas_interface());
                        action->set_param("value_node",ValueNode::Handle::cast_static(value_node));
                        action->set_param("waypoint",*wp_iter);
                        add_action(action);
                    }
                }
            }
        }

    }
}
