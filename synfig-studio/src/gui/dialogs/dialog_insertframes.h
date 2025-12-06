/* === S Y N F I G ========================================================= */
/*!	\file dialog_insertframes.h
**	\brief Template Header
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

#ifndef __SYNFIG_GTKMM_DIALOG_INSERTFRAMES_H
#define __SYNFIG_GTKMM_DIALOG_INSERTFRAMES_H

#include <ETL/handle>
#include <synfig/renddesc.h>
#include <synfigapp/canvasinterface.h>

#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/dialog.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/comboboxtext.h>

namespace studio
{

class CanvasProperties;
class Widget_ValueBase;
class Widget_Waypoint;
class Widget_Time;

class Dialog_InsertFrames : public Gtk::Dialog
{
    synfig::Canvas::Handle canvas_;

    Widget_Time *time_widget_;
    Widget_Time *duration_widget_;

    sigc::signal<void> signal_edited_;

    void on_ok_pressed();
    void on_apply_pressed();

    void refresh();

public:

    sigc::signal<void>& signal_edited() { return signal_edited_; }

    Dialog_InsertFrames(Gtk::Window &parent, synfig::Canvas::Handle canvas);

    void set_canvas(synfig::Canvas::Handle x);
    void set_time(synfig::Time x);
    synfig::Time get_time()const;
    synfig::Time get_duration()const;

}; // End of class InsertFrames

}; // End of namespace studio

#endif
