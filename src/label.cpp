
/*
   Copyright (C) 2007 by David White <dave.net>
   Part of the Silver Tree Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 or later.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include <iostream>

#include "label.hpp"
#include "raster.hpp"
#include "translate.hpp"
#include "text.hpp"

namespace gui {

label::label(const std::string& text, const SDL_Color& color, int size)
	: text_(i18n::translate(text)), color_(color), size_(size),
	  fixed_width_(false)
{
	recalculate_texture();
}

void label::set_color(const SDL_Color& color)
{
	color_ = color;
	recalculate_texture();
}

void label::set_font_size(int size)
{
	size_ = size;
	recalculate_texture();
}

void label::set_text(const std::string& text)
{
	text_ = i18n::translate(text);
	reformat_text();
	recalculate_texture();
}

std::string& label::current_text() {
	if(fixed_width_) {
		return formatted_;
	}
	return text_;
}

void label::set_fixed_width(bool fixed_width)
{
	fixed_width_ = fixed_width;
	reformat_text();
	recalculate_texture();
}

void label::set_dim(int w, int h) {
	if(w != width() || h != height()) {
		inner_set_dim(w, h);
		reformat_text();
		recalculate_texture();
	}
}

void label::inner_set_dim(int w, int h) {
	widget::set_dim(w, h);
}

void label::reformat_text()
{
    text::renderer& renderer = text::renderer::instance();
    if(fixed_width_) {
        formatted_ = renderer.format(text_, size_, width());
    }
}

void label::recalculate_texture()
{
    text::renderer& renderer = text::renderer::instance();
    rendered_text_ = renderer.render(current_text(), size_, color_);
    inner_set_dim(rendered_text_->width(), rendered_text_->height());
}

void label::handle_draw() const
{
    rendered_text_->blit(x(), y());
}

void label::set_rendered_text(text::rendered_text_ptr t)
{
	rendered_text_ = t;
}

dialog_label::dialog_label(const std::string& text, const SDL_Color& color, int size)
    : label(text, color, size), progress_(0) {
    
    recalculate_texture();
}

void dialog_label::set_progress(int progress)
{
	progress_ = progress;
	recalculate_texture();
}

void dialog_label::recalculate_texture()
{
    int w, h;

    text::renderer& renderer = text::renderer::instance();

    renderer.get_text_size(current_text(), size(), &w, &h);
    inner_set_dim(w, h);

    stages_ = current_text().size();
    int prog = progress_;
    if(prog < 0) prog = 0;
    if(prog > stages_) prog = stages_;

    std::string txt = current_text().substr(0, prog);
    set_rendered_text(renderer.render(txt, size(), color()));
}

label_factory::label_factory(const SDL_Color& color, int size)
   : color_(color), size_(size)
{}

label_ptr label_factory::create(const std::string& text) const
{
	return label_ptr(new label(text,color_,size_));
}

label_ptr label_factory::create(const std::string& text,
                                const std::string& tip) const
{
	const label_ptr res(create(text));
	res->set_tooltip(tip);
	return res;
}

}
