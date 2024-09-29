/* === S Y N F I G ========================================================= */
/*!    \file valuenode_atan2.cpp
**    \brief Implementation of the "aTan2" valuenode conversion.
**
**    \legal
**    Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**    Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
**
**    This file is part of Synfig.
**
**    Synfig is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 2 of the License, or
**    (at your option) any later version.
**
**    Synfig is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**    \endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#    include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#    include <config.h>
#endif

#include <tuple>
#include "valuenode_bonejoint.h"
#include "valuenode_bone.h"
#include "valuenode_const.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/vector.h>
#include <synfig/canvas.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_BoneJoint, RELEASE_VERSION_1_6_0, "bonejoint", N_("Bone Joint"))

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_BoneJoint::ValueNode_BoneJoint(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	init_children_vocab();
	if (value.get_type() == type_angle)
	{
		set_link("target",        ValueNode_Const::create(Point{}));
		set_link("bone1",         ValueNode_Const::create(String{}));
		set_link("bone2",         ValueNode_Const::create(String{}));
		set_link("isrighthanded", ValueNode_Const::create(true));
		set_link("isfirst",       ValueNode_Const::create(true));
	}
	else
	{
		throw Exception::BadType(value.get_type().description.local_name);
	}
}

LinkableValueNode*
ValueNode_BoneJoint::create_new()const
{
	return new ValueNode_BoneJoint(get_type());
}

ValueNode_BoneJoint*
ValueNode_BoneJoint::create(const ValueBase& x, etl::loose_handle<Canvas>)
{
	return new ValueNode_BoneJoint(x);
}

ValueNode_BoneJoint::~ValueNode_BoneJoint()
{
	unlink_all();
}


//private static Vector[] CircleIntersection(Vector c1, double r1, Vector c2, double r2)
std::pair<Vector, Vector> CircleIntersection(const Vector& c1, double r1, const Vector& c2, double r2)
{
	// https://math.stackexchange.com/a/1367732

	double R_sq = (c1 - c2).mag_squared();
	double r1_sq = r1 * r1;
	double r2_sq = r2 * r2;

	double det = (2 * (r1_sq + r2_sq) / R_sq) - ((r1_sq - r2_sq) * (r1_sq - r2_sq) / R_sq / R_sq) - 1;
	if (det < 0)
	{
		// too far apart, no intersection.
		// for our purposes we just care about the angle, so give the midpoint
		Vector mid = (c1 + c2) * .5;
		return {mid, mid};
	}

	Vector a = ((c1 + c2) * .5) + ((c2 - c1) * (r1_sq - r2_sq) / 2 / R_sq);
	Vector b = Vector(c2[1] - c1[1], c1[0] - c2[0]) * .5 * std::sqrt(det);

	return { a - b, a + b };
}

struct FlagLocker
{
	FlagLocker(int* a) : a_(a)
	{
		++(*a_);
	}
	~FlagLocker()
	{
		--(*a_);
	}

private:
	int* a_;
};

static const Time CACHE_INVALID = Time::begin();

ValueBase
ValueNode_BoneJoint::operator()(Time time)const
{
	if (calculatingSemaphore_ > 0) { return Angle::deg(0); }
	FlagLocker lockThis(&calculatingSemaphore_);

	if (cacheTime_ == time)
	{
		return cacheValue_;
	}

	std::string bn1Name = (*bone1_)(time).get(String{});
	std::string bn2Name = (*bone2_)(time).get(String{});

	Canvas::Handle canvas = get_root_canvas();

	if (bone1Node_.empty()) const_cast<ValueNode_BoneJoint*>(this)->find_bone1(bone1_);
	if (bone2Node_.empty()) const_cast<ValueNode_BoneJoint*>(this)->find_bone2(bone2_);

	ValueNode_Bone::Handle bn1 = bone1Node_;
	ValueNode_Bone::Handle bn2 = bone2Node_;

	if (bn1.empty() || bn2.empty())
	{
		return cacheTime_ = time, cacheValue_ = Angle::deg(0);
	}

	std::unique_ptr<FlagLocker> lockBone1, lockBone2;
	ValueNode_BoneJoint::Handle bn1Angle = ValueNode_BoneJoint::Handle::cast_dynamic(bn1->get_link("angle"));
	ValueNode_BoneJoint::Handle bn2Angle = ValueNode_BoneJoint::Handle::cast_dynamic(bn2->get_link("angle"));
	if (bn1Angle) lockBone1.reset(new FlagLocker(&bn1Angle->calculatingSemaphore_));
	if (bn2Angle) lockBone2.reset(new FlagLocker(&bn2Angle->calculatingSemaphore_));

	Bone firstBone = (*bn1)(time).get(Bone{});
	Bone secondBone = (*bn2)(time).get(Bone{});

	auto firstMat = firstBone.get_animated_matrix();
	auto secondMat = secondBone.get_animated_matrix();

	auto o1 = firstMat.get_transformed(Vector(0,0));

	auto o2 = secondMat.get_transformed(Vector(0,0));
	auto r2 = secondBone.get_length() * secondBone.get_scalelx();
	auto t = (*target_)(time).get(Vector());

	// get intersection point
	std::pair<Vector, Vector> opts = CircleIntersection(o1, (o2-o1).mag(), t, r2);

	Vector p = ((*isrighthanded_)(time).get(bool{}) ? opts.second : opts.first);

	// angle 1st bone towards the intersection point
	Matrix3 firstMatInverted = firstMat.get_inverted();
	Angle deltaAngleToSecondBone = firstMatInverted.get_transformed(p - o1, false).angle() - firstMatInverted.get_transformed(o2 - o1, false).angle();

	Angle newFirstAngle = firstBone.get_angle() + deltaAngleToSecondBone;

	if ((*isfirst_)(time).get(false))
		return cacheTime_ = time, cacheValue_ = newFirstAngle;

	// angle 2nd bone towards endpoint
	Matrix3 newFirstMatInverted = ValueNode_Bone::get_animated_matrix(
				time,
				firstBone.get_scalex(),
				1.0,
				newFirstAngle,
				firstBone.get_origin(),
				firstBone.get_parent())
			.get_inverted();

	Angle newSecondAngle = newFirstMatInverted.get_transformed(t - p, false).angle();	
	return cacheTime_ = time, cacheValue_ = newSecondAngle;
}

bool
ValueNode_BoneJoint::check_type(Type &type)
{
	return type==type_angle;
}

bool
ValueNode_BoneJoint::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(target_, type_vector);
	case 1: {
		VALUENODE_CHECK_TYPE(type_string);
		find_bone1(value);
		VALUENODE_SET_VALUE(bone1_);
	}
	case 2: {
		VALUENODE_CHECK_TYPE(type_string);
		find_bone2(value);
		VALUENODE_SET_VALUE(bone2_);
	}
	case 3: CHECK_TYPE_AND_SET_VALUE(isrighthanded_, type_bool);
	case 4: CHECK_TYPE_AND_SET_VALUE(isfirst_, type_bool);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_BoneJoint::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
	case 0: return target_;
	case 1: return bone1_;
	case 2: return bone2_;
	case 3: return isrighthanded_;
	case 4: return isfirst_;
	}
	return 0;
}

void
ValueNode_BoneJoint::set_root_canvas(etl::loose_handle<Canvas> x)
{
	LinkableValueNode::set_root_canvas(x);
	target_->set_root_canvas(x);
	bone1_->set_root_canvas(x);
	bone2_->set_root_canvas(x);
	isrighthanded_->set_root_canvas(x);
	isfirst_->set_root_canvas(x);
}

LinkableValueNode::Vocab
ValueNode_BoneJoint::get_children_vocab_vfunc()const
{
	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc("target")
		.set_local_name(_("Target"))
		.set_description(_("Target point to angle towards"))
	);

	ret.push_back(ParamDesc("bone1")
		.set_local_name(_("Bone 1"))
		.set_description(_("The first bone of the joint"))
	);

	ret.push_back(ParamDesc("bone2")
		.set_local_name(_("Bone 2"))
		.set_description(_("The second bone of the joint"))
	);

	ret.push_back(ParamDesc("isrighthanded")
		.set_local_name(_("Right handed"))
		.set_description(_("Whether this is a right-handed (checked) or left-handed (unchecked) joint"))
	);

	ret.push_back(ParamDesc("isfirst")
		.set_local_name(_("First?"))
		.set_description(_("Whether the angle being calculated is the first bone's (checked) or second bone's (unchecked)"))
	);

	return ret;
}

void ValueNode_BoneJoint::on_changed()
{
	cacheTime_ = CACHE_INVALID;
	LinkableValueNode::on_changed();
}

void ValueNode_BoneJoint::on_child_changed(const Node *x)
{
	cacheTime_ = CACHE_INVALID;
	LinkableValueNode::on_child_changed(x);
}

void ValueNode_BoneJoint::on_dependent_bone_changed()
{
	cacheTime_ = CACHE_INVALID;
}

void ValueNode_BoneJoint::find_bone1(ValueNode::Handle value)
{
	if (!bone1Node_.empty())
		bone1NodeConnection_->disconnect();

	String realName = (*value)(Time::zero()).get(String{});

	Canvas::Handle canvas = get_root_canvas();
	bone1Node_ = ValueNode_Bone::LooseHandle{};
	for (auto p : ValueNode_Bone::get_bone_map(canvas))
	{
		String name = p.second->get_bone_name(Time::zero());
		if (name == realName)
		{
			bone1Node_ = p.second;
			bone1NodeConnection_ = bone1Node_->signal_changed().connect(
						sigc::mem_fun(*this, &ValueNode_BoneJoint::on_dependent_bone_changed));
			break;
		}
	}
}

void ValueNode_BoneJoint::find_bone2(ValueNode::Handle value)
{
	if (!bone2Node_.empty())
		bone2NodeConnection_->disconnect();

	String realName = (*value)(Time::zero()).get(String{});

	Canvas::Handle canvas = get_root_canvas();
	bone2Node_ = ValueNode_Bone::LooseHandle{};
	for (auto p : ValueNode_Bone::get_bone_map(canvas))
	{
		String name = p.second->get_bone_name(Time::zero());
		if (name == realName)
		{
			bone2Node_ = p.second;
			bone2NodeConnection_ = bone2Node_->signal_changed().connect(
						sigc::mem_fun(*this, &ValueNode_BoneJoint::on_dependent_bone_changed));
			break;
		}
	}
}
