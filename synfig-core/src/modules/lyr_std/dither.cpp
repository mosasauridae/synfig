/* === S Y N F I G ========================================================= */
/*!	\file dither.cpp
**	\brief Implementation of the "Dither" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011-2013 Carlos López
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

#include <random>
#include "dither.h"

#include <synfig/localization.h>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>

#include <synfig/general.h>
#include <chrono>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Dither);
SYNFIG_LAYER_SET_NAME(Layer_Dither,"dither");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Dither,N_("Dither"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Dither,N_("Filters"));
SYNFIG_LAYER_SET_VERSION(Layer_Dither,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

rendering::Task::Token TaskDither::token(
    DescAbstract<TaskDither>("Dither") );
rendering::Task::Token TaskDitherSW::token(
    DescReal<TaskDitherSW, TaskDither>("DitherSW") );


// generate a PRNG table
static std::vector<float>
make_prng_table()
{
    size_t N = 5000;
    std::vector<float> r(N);

    // for cross-platform consistency, don't use random_device and avoid the use of std::uniform_real_distribution (ie most of <random>).
    // mersenne twister with default constructor is cross-platform so that's still okay.
    std::mt19937 gen;
    for (size_t i=0; i<N; i++)
    {
        r[i] = (double)gen() / gen.max();
    }

    return r;
};

// pre-generated PRNG table
// https://xkcd.com/221/
static const std::vector<float> prng_table = make_prng_table();

static Color
apply_dithering(float depth, bool show_pattern, const Color &c, const Vector &seed1, const int seed2)
{
    static const float epsilon(0.00001);
    if (c.get_a() < epsilon)
    {
        return c;
    }

    // the PRNG we choose doesn't really need to be random, we just need a good source
    // of entropy for the dithering. We want it to be deterministic, fast, and random enough to
    // get a good-looking dither.
    //
    // the "PRNG" algorithm here is:
    // 1. Hash the input by mashing the bits together
    // 2. Use the hash as a lookup into a pre-generated random table.

    uint64_t s0Bits = seed1[0] * UINT16_MAX;
    uint64_t s1Bits = seed1[1] * UINT16_MAX;
    uint64_t s2Bits = seed2;

    size_t hash = ((s0Bits + 687080119907498221ul) * 617516190114062147ul) ^
                  ((s1Bits + 280713548311792597ul) * 377700939715608409ul) ^
                  ((s2Bits + 301676334379489441ul) * 188727652806685939ul);
    hash = hash % (prng_table.size() - 4);

    // these numbers represent the probability that the corresponding number gets rounded up
    float pr = prng_table[hash+0];
    float pg = prng_table[hash+1];
    float pb = prng_table[hash+2];
    float pa = prng_table[hash+3];

    static const float alpha_depth = 100.0f;

    int r,g,b,a;

    if (show_pattern)
    {
        // if show_pattern is on, ignore the original color and just set the color to the PRNG noise
        r = pr * depth;
        g = pg * depth;
        b = pb * depth;
        a = alpha_depth;
    }
    else
    {
        // convert from floating point (0-1) to RGB (0-255), then round either up or down probabilistically based on the decimal point.
        // ex. if c.get_r() is 62.853 in RGB, that will have a 85.3% chance of rounding up to 63 and an 14.7% chance of rounding down to 62.
        r = c.get_r() * depth + pr;
        g = c.get_g() * depth + pg;
        b = c.get_b() * depth + pb;
        a = c.get_a() * alpha_depth + pa;
    }

    return Color(r / depth, g / depth, b / depth, a / alpha_depth);
}

bool
TaskDitherSW::run(RunParams&) const
{
	RectInt r = target_rect;
	if (r.valid())
	{
		VectorInt offset = get_offset();
		RectInt ra = sub_task()->target_rect + r.get_min() + get_offset();
        const Rect& sa = sub_task()->source_rect;
		if (ra.valid())
		{
			rect_set_intersect(ra, ra, r);
			if (ra.valid())
			{
				LockWrite ldst(this);
				if (!ldst) return false;
				LockRead lsrc(sub_task());
				if (!lsrc) return false;

				const synfig::Surface &a = lsrc->get_surface();
				synfig::Surface &c = ldst->get_surface();

                Real sy = sa.miny;
                const Real dsy = sa.get_height() / ra.get_height();
                const Real dsx = sa.get_width() / ra.get_width();

                for(int y = ra.miny; y < ra.maxy; ++y, sy += dsy)
                {
					const Color *ca = &a[y - r.miny + offset[1]][ra.minx - r.minx + offset[0]];
					Color *cc = &c[y][ra.minx];

                    Real sx = sa.minx;
                    for(int x = ra.minx; x < ra.maxx; ++x, ++ca, ++cc, sx += dsx)
                        *cc = apply_dithering(depth, show_pattern, *ca, Vector(sx,sy), seed);
				}
			}
		}
	}

	return true;
}


Layer_Dither::Layer_Dither():
    param_depth(DitherDepth_8bit),
    param_seed(int(time(nullptr))),
    param_showpattern(bool(false))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
Layer_Dither::set_param(const String & param, const ValueBase &value)
{
    IMPORT_VALUE(param_depth);
    IMPORT_VALUE(param_seed);
    IMPORT_VALUE(param_showpattern);

	return false;
}

ValueBase
Layer_Dither::get_param(const String &param)const
{
    EXPORT_VALUE(param_depth);
    EXPORT_VALUE(param_seed);
    EXPORT_VALUE(param_showpattern);

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Layer::Vocab
Layer_Dither::get_param_vocab()const
{
	Layer::Vocab ret;

    ret.push_back(ParamDesc("depth")
        .set_local_name(_("Bit Depth"))
        .set_description(_("What precision of dithering to apply"))
        .set_hint("enum")
        .set_static(true)
        .add_enum_value(DitherDepth_8bit,  "8bit",  _("8-bit RGB"))
        .add_enum_value(DitherDepth_16bit, "16bit", _("16-bit High Precision"))
    );
    ret.push_back(ParamDesc("seed")
        .set_local_name(_("RandomNoise Seed"))
        .set_description(_("Change to modify the random seed of the noise"))
    );
    ret.push_back(ParamDesc("showpattern")
        .set_local_name(_("Show Noise Pattern"))
        .set_description(_("Show the noise pattern instead of applying dithering"))
    );

	return ret;
}

Color
Layer_Dither::get_color(Context context, const Point &pos)const
{
    return apply_dithering(get_depth(),
                           param_showpattern.get(bool()),
                           context.get_color(pos),
                           pos,
                           param_seed.get(int()));
}

Rect
Layer_Dither::get_full_bounding_rect(Context context)const
{
	return context.get_full_bounding_rect();
}

rendering::Task::Handle
Layer_Dither::build_rendering_task_vfunc(Context context)const
{
    TaskDither::Handle task(new TaskDither());
    task->depth = get_depth();
    task->seed = param_seed.get(int());
    task->show_pattern = param_showpattern.get(bool());
    task->sub_task() = context.build_rendering_task();
    return task;
}

float
Layer_Dither::get_depth() const
{
    return param_depth.get(DitherDepth()) == DitherDepth_8bit ? 255.0f : 65535.0f;
}
