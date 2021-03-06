#include <GL/glew.h>
#include "SDL.h"

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <sstream>

#include "message_dialog.hpp"
#include "raster.hpp"
#include "surface_cache.hpp"
#include "texture.hpp"
#include "foreach.hpp"
#include "character.hpp"
#include "image_widget.hpp"
#include "label.hpp"
#include "frame.hpp"

namespace gui {

message_dialog::message_dialog(const game_logic::world& world,
                   const std::string& pc_portrait,
                   const std::string& npc_portrait,
			       const std::string& msg,
			       const std::vector<std::string>* options,
			       bool starts_conversation)
	: dialog(0,0,graphics::screen_width(),graphics::screen_height()),
	  msg_(msg), options_(options), world_(world),
	  pc_portrait_name_(pc_portrait), npc_portrait_name_(npc_portrait),
	  fade_in_rate_(30), starts_conversation_(starts_conversation)
{

	selected_ = -1;
	set_clear_bg(false);
	construct_interface();
}

frame_ptr message_dialog::make_option_frame(int option, widget_ptr widget, frame_manager::key_mapper_ptr keys)
{
	frame_ptr ret =
		frame_manager::make_frame(widget, option == selected_ ? "dialog-option-selected" : "dialog-option",
					  keys);
	ret->set_fg(option == selected_ ? graphics::color_white() : graphics::color_black());
	ret->set_bg(graphics::color_black());
	return ret;
}

frame_ptr message_dialog::make_option_frame(int option, widget_ptr widget)
{
	frame_ptr ret =
		frame_manager::make_frame(widget, option == selected_ ? "dialog-option-selected" : "dialog-option");
	ret->set_fg(option == selected_ ? graphics::color_white() : graphics::color_black());
	ret->set_bg(graphics::color_black());
	return ret;
}

void message_dialog::construct_interface()
{
	SDL_Color color = graphics::color_white();
	const int s_width = width();
	const int s_height = height();

	clear();
	dialog_labels_.clear();
	option_frames_.clear();

	set_padding(0);
	dialog_label_ptr msg_label_inner(new dialog_label(msg_, color, 18));
	msg_label_inner->set_fixed_width(true);
	dialog_labels_.push_back(msg_label_inner);
	widget_ptr msg_label = frame_manager::make_frame(msg_label_inner, "dialog-message");
	{
		int right_offset = 200;
		if(npc_portrait_name_.empty()) {
			right_offset /= 2;
		}
		/* with fixed width, y height is calculated */
		msg_label->set_dim(s_width - s_width/4 - right_offset, 0);

		set_cursor(s_width - s_width/8 - right_offset - msg_label->width(),
			   s_height/8+10);
		add_widget(msg_label, dialog::MOVE_RIGHT);
		set_cursor(cursor_x(), cursor_y()-10);
	}

	if(!npc_portrait_name_.empty()) {
		widget_ptr portrait(new image_widget(npc_portrait_name_, 205, 205));
		portrait = frame_manager::make_frame(portrait,"dialog-npc-portrait");
		add_widget(portrait, dialog::MOVE_RIGHT);
	}

	if(options_) {
		int left_offset = 0;

		set_cursor(s_width/8, s_height*7/8 - 205);
		if(!pc_portrait_name_.empty()) {
			pc_portrait_ = widget_ptr(new image_widget(pc_portrait_name_, 205, 205));
			frame_ptr pc_frame = frame_manager::make_frame(pc_portrait_, "dialog-pc-portrait");
			pc_portrait_ = pc_frame;
			if(starts_conversation_) {
				pc_portrait_->set_visible(false);
			}
			add_widget(pc_portrait_, dialog::MOVE_RIGHT);
			left_offset = pc_portrait_->width();
		} else {
			left_offset = 100;
		}

		set_cursor(s_width/8 + left_offset, cursor_y()+10);

		int xbox = cursor_x();
		int ybox = cursor_y();
		int wbox = 0;
		int hbox = 0;

		int option = 0;
		option_box_ = boost::shared_ptr<phantom_dialog>(new phantom_dialog());
		option_box_->set_padding(0);
		foreach(const std::string& s, *options_) {
			std::ostringstream option_text_s;
			if(option < 9) {
				option_text_s << (option+1) << ". ";
			} else {
				option_text_s << "(*) ";
			}
			option_text_s << s;
			dialog_label_ptr option_label_inner(new dialog_label(option_text_s.str(), color, 18));
			option_label_inner->set_fixed_width(true);
			dialog_labels_.push_back(option_label_inner);
			frame_ptr option_frame =
				make_option_frame(option, option_label_inner);

			option_frame->set_dim(s_width - s_width/4 - left_offset, 0);
			option_frame->set_visible(false);
			option_frames_.push_back(option_frame);
			if(option_frame->width() > wbox) {
				wbox = option_frame->width();
			}
			if(option_frame->y() + option_frame->height() > hbox) {
				hbox = option_frame->y() + option_frame->height();
			}
			option_box_->add_widget(option_frame, dialog::MOVE_DOWN);
			++option;
		}
		hbox -= ybox;

		all_option_frame_ = frame_manager::make_frame(option_box_, "dialog-all-options");
		add_widget(all_option_frame_);
		all_option_frame_->set_visible(false);

		foreach(frame_ptr option_frame, option_frames_) {
			option_frame->add_key_set("all", xbox, ybox, wbox, hbox);
		}
	}
}

void message_dialog::handle_draw() const
{
    if(world_.draw()) {
	graphics::prepare_raster();
        dialog::handle_draw_children();
        SDL_GL_SwapBuffers();
    }
}

int message_dialog::find_option(int x, int y) {
	int opt_num = 0;
	const int cx = x - option_box_->x();
	const int cy = y - option_box_->y();
	foreach(frame_ptr option, option_frames_) {
		if(cx > option->x() &&
		   cy > option->y() &&
		   cx < option->x() + option->width() &&
		   cy < option->y() + option->height()) {
			break;
		}
		++opt_num;
	}
	if(opt_num == option_frames_.size()) {
		opt_num = -1;
	}
	return opt_num;
}

void message_dialog::show_modal()
{
    world_.renderer().reset_timing();
    
    fade_in();
    if(options_ != NULL) {
        ensure_fade_over();
        if(selected_ == -1) {
            selected_ = 0;
            update_option(0);
        }
    }
    dialog::show_modal();
}

void message_dialog::ensure_fade_over() {
	foreach(dialog_label_ptr dlabel, dialog_labels_) {
		dlabel->set_progress(dlabel->get_max_progress());
	}
	if(pc_portrait_ != NULL) {
		pc_portrait_->set_visible(true);
	}
	if(all_option_frame_ != NULL) {
		all_option_frame_->set_visible(true);
	}
	foreach(frame_ptr fr, option_frames_) {
		fr->set_visible(true);
	}
}

void message_dialog::fade_in()
{
    bool done = false;
    int count = 0;

    foreach(dialog_label_ptr dlabel, dialog_labels_) {
        int prog;
        int max = dlabel->get_max_progress();
        int start = SDL_GetTicks();
        int end = start + max * fade_in_rate_;
        int now = start;
        
        if(count > 0) {
            if(count == 1) {
                if(pc_portrait_ != NULL) {
                    pc_portrait_->set_visible(true);
                }
                if(all_option_frame_ != NULL ){
                    all_option_frame_->set_visible(true);
                }
            }
            option_frames_[count-1]->set_visible(true);
        }
        
        input::pump pump;
        fade_in_listener listener(done);
        pump.register_listener(&listener);
        
        while(now < end && !done && pump.process()) {
            prog = (now - start)/fade_in_rate_;
            dlabel->set_progress(prog);
            draw();
            now = SDL_GetTicks();
        }
        dlabel->set_progress(max);
        ++count;
    }
}

void message_dialog::update_option(int option) {
	const int label_index = option + 1;
	if(option != -1) {
		frame_ptr w_old = option_frames_[option];
		frame_ptr w_new =
			make_option_frame(option, dialog_labels_[label_index], w_old->get_keys());

		option_frames_[option] = w_new;
		option_box_->replace_widget(w_old, w_new);
	}
}

bool message_dialog::handle_event(const SDL_Event& event, bool claimed) {
    if(claimed) {
        return claimed;
    }

	int old_selected = selected_;

	switch(event.type) {
	case SDL_KEYDOWN:
		if(options_) {
			switch(event.key.keysym.sym) {
			case SDLK_UP:
				if(selected_ > 0) {
					--selected_;
				} else if(selected_ < 0) {
					selected_ = 0;
				}
				break;
			case SDLK_DOWN:
				if(selected_ < options_->size()-1) {
					++selected_;
				} else if(selected_ < 0) {
					selected_ = options_->size()-1;
				}
				break;
			case SDLK_RETURN:
			case SDLK_SPACE:
				close();
				break;
			case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
			case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
				selected_ = event.key.keysym.sym - SDLK_1;
				if(selected_ > options_->size()-1) {
					selected_ = old_selected;
				} else {
					close();
				}
				break;
			case SDLK_0:
			case SDLK_ASTERISK:
				if(options_->size() > 9) {
					selected_ = 9;
					close();
				}
				break;
			}
		} else {
			close();
		}
		claimed = true;
		break;
	case SDL_MOUSEMOTION:
		if(options_) {
			selected_ = find_option(event.motion.x, event.motion.y);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		if(options_) {
			selected_ = find_option(event.button.x, event.button.y);
			if(selected_ >= 0) {
				close();
			}
		} else {
			close();
		}
		claimed = true;
		break;
	default:
		break;
	}
	if(old_selected != selected_) {
		update_option(old_selected);
		update_option(selected_);
	}
	return claimed;
}

bool message_dialog::fade_in_listener::process_event(const SDL_Event& event, bool claimed) {
    if(claimed) {
        return claimed;
    }
		
    switch(event.type) {
    case SDL_KEYDOWN:
        switch(event.key.keysym.sym) {
        case SDLK_1: case SDLK_2: case SDLK_3:
        case SDLK_4: case SDLK_5: case SDLK_6:
        case SDLK_7: case SDLK_8: case SDLK_9:
            /* ideally we'd push this back but there
               isn't an obvious way to do it for now */
            break;
        default:
            break;
        }
        done_ = true;
        claimed = true;
        break;
    case SDL_MOUSEBUTTONDOWN:
        done_ = true;
        claimed = true;
        break;
    default:
        break;
    }
    return claimed;
}


} // namespace gui
