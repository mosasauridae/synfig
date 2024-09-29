/* === S Y N F I G ========================================================= */
/*!	\file valuenode_bonejoint.h
**	\brief Header file for implementation of the "Bone Joint" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**  Copyright (c) 2011 Carlos López
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

#ifndef __SYNFIG_VALUENODE_BONEJOINT_H
#define __SYNFIG_VALUENODE_BONEJOINT_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>
#include "valuenode_bone.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_BoneJoint : public LinkableValueNode
{
	mutable int calculatingSemaphore_ = 0;

	mutable Time cacheTime_ = Time::begin();
	mutable Angle cacheValue_;

	ValueNode::RHandle target_;
	ValueNode::RHandle isrighthanded_;
	ValueNode::RHandle bone1_;
	ValueNode::RHandle bone2_;
	ValueNode::RHandle isfirst_;

	ValueNode_Bone::LooseHandle bone1Node_;
	sigc::signal0<void>::iterator bone1NodeConnection_;

	ValueNode_Bone::LooseHandle bone2Node_;
	sigc::signal0<void>::iterator bone2NodeConnection_;

	ValueNode_BoneJoint(const ValueBase &value);

public:
	typedef etl::handle<ValueNode_BoneJoint> Handle;
	typedef etl::handle<const ValueNode_BoneJoint> ConstHandle;

	static ValueNode_BoneJoint* create(const ValueBase& value=ValueBase(), etl::loose_handle<Canvas> canvas=nullptr);
	virtual ~ValueNode_BoneJoint();

	virtual ValueBase operator()(Time t) const override;

	virtual String get_name() const override;
	virtual String get_local_name() const override;
	static bool check_type(Type &type);

	virtual void set_root_canvas(etl::loose_handle<Canvas> canvas) override;

protected:
	virtual LinkableValueNode* create_new() const override;

	virtual bool set_link_vfunc(int i,ValueNode::Handle x) override;
	virtual ValueNode::LooseHandle get_link_vfunc(int i) const override;

	virtual Vocab get_children_vocab_vfunc() const override;

	virtual void on_changed() override;
	virtual void on_child_changed(const Node *x) override;

	void on_dependent_bone_changed();

	void find_bone1(ValueNode::Handle value);
	void find_bone2(ValueNode::Handle value);

}; // END of class ValueNode_BoneJoint

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
