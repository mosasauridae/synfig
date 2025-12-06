/* === S Y N F I G ========================================================= */
/*!	\file dialog_removeframes.cpp
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

#include "dialog_removeframes.h"

#include <gui/dialogs/canvasproperties.h>
#include <gui/localization.h>
#include <gui/widgets/widget_time.h>

#include <gtkmm/icontheme.h>
#include <gdkmm/pixbuf.h>

#endif

using namespace studio;
using namespace synfig;

/* === M A C R O S ========================================================= */

#if defined(__has_cpp_attribute)
# if __has_cpp_attribute(fallthrough)
#  define fallthrough__ [[fallthrough]]
# endif
#endif

#ifndef fallthrough__
# if __GNUC__ >= 7
#  define fallthrough__ __attribute__((fallthrough))
# elif __clang__
#  define fallthrough__ [[clang::fallthrough]]
# else
#  define fallthrough__ ((void)0)
# endif
#endif

/* === M E T H O D S ======================================================= */

Dialog_RemoveFrames::Dialog_RemoveFrames(Gtk::Window& parent,Canvas::Handle canvas)
    : Gtk::Dialog       ("Insert Frames", parent),
    canvas_(canvas)
{
    set_resizable(false);

    auto grid = manage(new Gtk::Grid());
    grid->get_style_context()->add_class("dialog-secondary-content");
    get_content_area()->pack_start(*grid);

    time_widget_=manage(new Widget_Time());
    time_widget_->set_fps(canvas->rend_desc().get_frame_rate());
    time_widget_->set_value(0);

    Gtk::Label *time_label = manage(new Gtk::Label(_("_Time"), true));
    time_label->set_mnemonic_widget(*time_widget_);
    grid->attach(*time_label,   0, 0, 1, 1);
    grid->attach(*time_widget_, 1, 0, 1, 1);

    duration_widget_=manage(new Widget_Time());
    duration_widget_->set_fps(canvas->rend_desc().get_frame_rate());
    duration_widget_->set_value(1 / canvas->rend_desc().get_frame_rate());

    Gtk::Label *duration_label = manage(new Gtk::Label(_("_Duration"), true));
    duration_label->set_mnemonic_widget(*duration_widget_);
    grid->attach(*duration_label,   0, 1, 1, 1);
    grid->attach(*duration_widget_, 1, 1, 1, 1);

    Gtk::Button *apply_button(manage(new Gtk::Button(_("_Apply"), true)));
    apply_button->show();
    add_action_widget(*apply_button,1);
    apply_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_RemoveFrames::on_apply_pressed));

    Gtk::Button *cancel_button(manage(new Gtk::Button(_("_Close"), true)));
    cancel_button->show();
    add_action_widget(*cancel_button,0);
    cancel_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_RemoveFrames::hide));

    Gtk::Button *ok_button(manage(new Gtk::Button(_("_OK"), true)));
    ok_button->show();
    add_action_widget(*ok_button,2);
    ok_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_RemoveFrames::on_ok_pressed));

    show_all_children();
}

void Dialog_RemoveFrames::set_canvas(synfig::Canvas::Handle x)
{
    canvas_ = x;
    time_widget_->set_fps(canvas_->rend_desc().get_frame_rate());
    duration_widget_->set_fps(canvas_->rend_desc().get_frame_rate());
}

void Dialog_RemoveFrames::set_time(synfig::Time time)
{
    time_widget_->set_value(time);
}

Time Dialog_RemoveFrames::get_time() const
{
    return time_widget_->get_value();
}

Time Dialog_RemoveFrames::get_duration() const
{
    return duration_widget_->get_value();
}

void Dialog_RemoveFrames::on_ok_pressed()
{
    hide();
    signal_edited_();
}

void Dialog_RemoveFrames::on_apply_pressed()
{
    signal_edited_();
}
