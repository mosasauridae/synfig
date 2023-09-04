/* === S Y N F I G ========================================================= */
/*!	\file valuedescset.h
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_APP_ACTION_VALUEDESCSET_H
#define __SYNFIG_APP_ACTION_VALUEDESCSET_H

/* === H E A D E R S ======================================================= */

#include <synfigapp/action.h>
#include <synfigapp/value_desc.h>
#include <synfig/valuenodes/valuenode_animated.h>
#include <list>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

namespace Action {

class ValueDescSet :
	public Super
{
private:

	ValueDesc value_desc;
	synfig::ValueBase value;
	synfig::Time time;
	bool recursive;
	bool animate;
	bool lock_animation;
	synfig::ValueNode_Animated::Handle value_node_animated;

	/// convenient method to create a ValueDescSet action and add it to stack
	void add_action_valuedescset(const synfig::ValueBase& value, const ValueDesc& value_desc, bool recursive = true);

	static bool Show_Edit_Animated_Parameter_Dialog;
	static bool Auto_Respond_Message_Shown;

public:

	// this is a big giant hack. the better way to do this would be to accept these
	// as input parameters and manage them at a higher level. that way the decision
	// can be managed better.
	static void resetShowEditAnimatedParametersDialogFlag()
	{
		Show_Edit_Animated_Parameter_Dialog = true;
		Auto_Respond_Message_Shown = false;
	}

	ValueDescSet();

	static ParamVocab get_param_vocab();
	static bool is_candidate(const ParamList &x);

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void prepare();

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
