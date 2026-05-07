/* === S Y N F I G ========================================================= */
/*!	\file dither.h
**	\brief Header file for implementation of the "Dither" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
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

#ifndef __SYNFIG_LAYER_DITHER_H
#define __SYNFIG_LAYER_DITHER_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>

#include <synfig/rendering/optimizer.h>
#include <synfig/rendering/common/task/taskpixelprocessor.h>
#include <synfig/rendering/software/task/tasksw.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace modules
{
namespace lyr_std
{

enum DitherDepth {
    DitherDepth_8bit,
    DitherDepth_16bit
};

class TaskDither: public rendering::TaskPixelProcessor
{
public:
    typedef etl::handle<TaskDither> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

    float depth = 255.0f; // 255.0ff or 65535.0f
    int seed = 0;
    bool show_pattern = false;
};


class TaskDitherSW: public TaskDither, public rendering::TaskSW
{
public:
    typedef etl::handle<TaskDitherSW> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

    virtual bool run(RunParams &params) const;
};


class Layer_Dither : public Layer
{
	SYNFIG_LAYER_MODULE_EXT

private:
    //!Parameter: (DitherDepth)
    synfig::ValueBase param_depth;
    //!Parameter: (RandomNoise)
    synfig::ValueBase param_seed;
    //!Parameter: (bool)
    synfig::ValueBase param_showpattern;

public:

    Layer_Dither();

	virtual bool set_param(const String & param, const ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual Rect get_full_bounding_rect(Context context)const;

	virtual Vocab get_param_vocab()const;

	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context)const;

private:

    float get_depth() const;

}; // END of class Layer_Dither

}; // END of namespace lyr_std
}; // END of namespace modules
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
