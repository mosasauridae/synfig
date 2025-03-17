/* === S Y N F I G ========================================================= */
/*!	\file dock_curves.cpp
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

#include "dock_curves.h"

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/localization.h>
#include <gui/trees/layerparamtreestore.h>
#include <gui/widgets/widget_curves.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dock_Curves::Dock_Curves():
	Dock_CanvasSpecific("curves",_("Graphs"),"graphs_icon"),
	table_(),
	last_widget_curves_()
{
	vscrollbar_.set_orientation(Gtk::ORIENTATION_VERTICAL);

	// Make toolbar small for space efficiency
	get_style_context()->add_class("synfigstudio-efficient-workspace");

	prop_toolbar_ = Gtk::manage(new Gtk::Toolbar);

	all_button_ = Gtk::manage(new Gtk::ToggleToolButton());
	all_button_->set_label(_("All"));
	all_button_->set_active(true);
	all_button_->signal_toggled().connect(std::bind(&Dock_Curves::all_button_toggled, this));
	prop_toolbar_->append(*all_button_);

	for (const String& type_name : Widget_Curves::get_all_channel_names())
	{
		Gtk::ToggleToolButton* type_button = Gtk::manage(new Gtk::ToggleToolButton);
		type_button->set_label(_(type_name.c_str()));
		type_button->signal_toggled().connect([=]() { this->type_button_toggled(type_name); });
		type_buttons_[type_name] = type_button;
		prop_toolbar_->append(*type_button);
	}

	set_toolbar(*prop_toolbar_);
}

Dock_Curves::~Dock_Curves()
{
	if (table_) delete table_;
	if (prop_toolbar_) delete prop_toolbar_;
}

void
Dock_Curves::curve_selection_changed(Gtk::TreeView* param_tree_view, Widget_Curves* curves)
{
	LayerParamTreeStore::Model param_model;
	Gtk::TreeIter iter;
	if(!param_tree_view->get_selection()->count_selected_rows())
	{
		curves->clear();
		curves->refresh();
		return;
	}

	std::list< std::pair<std::string, synfigapp::ValueDesc> > value_descs;

	auto path_list = param_tree_view->get_selection()->get_selected_rows();
	auto model = param_tree_view->get_model();
	for (const auto& path_it : path_list) {
		auto iter = model->get_iter(path_it);
		std::string name;

		{
			auto iter2 = iter;
			while (iter2) {
				std::string current_label = (*iter2)[param_model.label].operator Glib::ustring();
				name = current_label + ":" + name;
				iter2 = iter2->parent();
			}
			name.pop_back();
		}

		value_descs.push_back( std::pair<std::string, synfigapp::ValueDesc> (name, (*iter)[param_model.value_desc]));
	}

	all_button_->hide();
	for (auto btn : type_buttons_)
		btn.second->hide();

	std::vector<String> channels = Widget_Curves::get_channel_names(value_descs);

	all_button_->set_active(true);
	if (channels.size() > 1)
	{
		all_button_->show();

		for (const auto& d : channels)
		{
			auto iter = type_buttons_.find(d);
			if (iter != type_buttons_.end())
			{
				iter->second->show();
				iter->second->set_active(false);
			}
		}
	}

	curves->set_value_descs(get_canvas_interface(), value_descs);
}

void
Dock_Curves::init_canvas_view_vfunc(CanvasView::LooseHandle canvas_view)
{
	//! Curves is registered thru CanvasView::set_ext_widget
	//! and will be deleted during CanvasView::~CanvasView()
	//! \see CanvasView::set_ext_widget
	//! \see CanvasView::~CanvasView
	Widget_Curves* curves(new Widget_Curves());
	curves->set_time_model(canvas_view->time_model());

	Gtk::TreeView* param_tree_view(
		static_cast<Gtk::TreeView*>(canvas_view->get_ext_widget("params"))
	);

	param_tree_view->get_selection()->signal_changed().connect(
		[=]() {	this->curve_selection_changed(param_tree_view, curves); }
	);

	if (studio::LayerTree* tree_layer = dynamic_cast<studio::LayerTree*>(canvas_view->get_ext_widget("layers_cmp"))) {
		tree_layer->signal_param_tree_header_height_changed().connect(
				sigc::mem_fun(*this, &studio::Dock_Curves::on_update_header_height) );
	}

	curves->signal_waypoint_clicked().connect(sigc::mem_fun(*this, &Dock_Curves::on_curves_waypoint_clicked));

	curves->signal_waypoint_double_clicked().connect(sigc::mem_fun(*this, &Dock_Curves::on_curves_waypoint_double_clicked));

	canvas_view->set_ext_widget(get_name(),curves);
}

void
Dock_Curves::refresh_selected_param()
{
/*	Gtk::TreeView* tree_view(
		static_cast<Gtk::TreeView*>(get_canvas_view()->get_ext_widget(get_name()))
	);
	Gtk::TreeModel::iterator iter(tree_view->get_selection()->get_selected());

	if(iter)
	{
		LayerParamTreeStore::Model model;
		get_canvas_view()->work_area->set_selected_value_node(
			(synfig::ValueNode::Handle)(*iter)[model.value_node]
		);
	}
	else
	{
		get_canvas_view()->work_area->set_selected_value_node(0);
	}
*/
}

void
Dock_Curves::changed_canvas_view_vfunc(CanvasView::LooseHandle canvas_view)
{
	if(table_)
	{
		delete table_;
		table_ = 0;

		last_widget_curves_ = 0;

		hscrollbar_.unset_adjustment();
		vscrollbar_.unset_adjustment();

		widget_timeslider_.set_canvas_view( CanvasView::Handle() );

		widget_kf_list_.set_time_model( etl::handle<TimeModel>() );
		widget_kf_list_.set_canvas_interface( etl::loose_handle<synfigapp::CanvasInterface>() );
	}


	if(canvas_view)
	{
		last_widget_curves_=dynamic_cast<Widget_Curves*>( canvas_view->get_ext_widget(get_name()) );
		if (!last_widget_curves_) {
			return;
		}

		vscrollbar_.set_adjustment(last_widget_curves_->get_range_adjustment());
		hscrollbar_.set_adjustment(canvas_view->time_model()->scroll_time_adjustment());

		widget_timeslider_.set_canvas_view(canvas_view);

		widget_kf_list_.set_time_model(canvas_view->time_model());
		widget_kf_list_.set_canvas_interface(canvas_view->canvas_interface());

		table_=new Gtk::Table(3, 2);
		table_->attach(widget_kf_list_,      0, 1, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::SHRINK);
		table_->attach(widget_timeslider_,   0, 1, 1, 2, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::SHRINK);
		table_->attach(*last_widget_curves_, 0, 1, 2, 3, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND);
		table_->attach(hscrollbar_,          0, 1, 3, 4, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::SHRINK);
		table_->attach(vscrollbar_,          1, 2, 0, 3, Gtk::FILL|Gtk::SHRINK, Gtk::FILL|Gtk::EXPAND);
		table_->show_all();
		add(*table_);
	}
}

void
Dock_Curves::on_update_header_height(int height)
{
	int w = 0, h = 0;
	widget_kf_list_.get_size_request(w, h);
	int ts_height = std::max(1, height - h);

	widget_timeslider_.get_size_request(w, h);
	if (h != ts_height)
		widget_timeslider_.set_size_request(-1, ts_height);
}

void
Dock_Curves::on_curves_waypoint_clicked(synfigapp::ValueDesc value_desc, std::set<synfig::Waypoint, std::less<synfig::UniqueID> > waypoint_set, int button)
{
	if (button != 3)
		return;
	button = 2;
	CanvasView::LooseHandle canvas_view = get_canvas_view();
	if (canvas_view)
		canvas_view->on_waypoint_clicked_canvasview(value_desc, waypoint_set, button);
}

void
Dock_Curves::on_curves_waypoint_double_clicked(synfigapp::ValueDesc value_desc, std::set<synfig::Waypoint, std::less<synfig::UniqueID> > waypoint_set, int button)
{
	if (button != 1)
		return;
	button = -1;
	CanvasView::LooseHandle canvas_view = get_canvas_view();
	if (canvas_view)
		canvas_view->on_waypoint_clicked_canvasview(value_desc, waypoint_set, button);
}

bool Dock_Curves::is_any_channel_active() const
{
	if (all_button_->get_active())
		return true;

	for (const auto& kv : type_buttons_)
		if (kv.second->get_active())
			return true;

	return false;
}

void Dock_Curves::all_button_toggled()
{
	if (toggling_ || last_widget_curves_ == nullptr)
		return;

	toggling_ = true;
	if (all_button_->get_active())
	{
		for (const auto& kv : type_buttons_)
			kv.second->set_active(false);

		last_widget_curves_->set_selected_channel("");
	}
	else
	{
		if (!is_any_channel_active())
			all_button_->set_active(true);
	}
	toggling_ = false;
}

void Dock_Curves::type_button_toggled(const synfig::String& name)
{
	if (toggling_ || last_widget_curves_ == nullptr)
		return;

	toggling_ = true;
	if (type_buttons_[name]->get_active())
	{
		all_button_->set_active(false);
		for (const auto& kv : type_buttons_)
			if (kv.first != name)
				kv.second->set_active(false);

		last_widget_curves_->set_selected_channel(name);
	}
	else
	{
		if (!is_any_channel_active())
			type_buttons_[name]->set_active(true);
	}
	toggling_ = false;
}
