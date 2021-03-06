// Distributed under the BSD License, see accompanying LICENSE.txt
// (C) Copyright 2010 John-John Tedro et al.
#ifndef _TEXT_H_
#define _TEXT_H_

#include <exception>

#include <boost/numeric/conversion/cast.hpp>
#include <boost/filesystem.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H 

namespace fs = boost::filesystem;

#include "image/image_base.hpp"
#include "utf8.hpp"
#include <assert.h>

#include <iostream>

namespace text {
  class text_error : public std::exception {
  private:
    const std::string error;
  public:
    text_error(const std::string error) : error(error) {};

    virtual ~text_error() throw() {};
    
    virtual const char* what() const throw()
    {
      return error.c_str();
    }
  };

  class font_face {
  private:
    const fs::path font_path;
    FT_Library library;
    FT_Face face;
    int size;
    color base;
  public:
    font_face(const fs::path font_path, int size, color base) throw(text_error) : font_path(font_path), base(base) {
      int error;
      
      error = FT_Init_FreeType( &library );

      if (error) {
        throw text_error("Failed to initialize the freetype2 library");
      }

      error = FT_New_Face(library, font_path.string().c_str(), 0, &face);

      if (error == FT_Err_Unknown_File_Format) {
        throw text_error("Could not open file - unknown file format: " + font_path.string());
      }
      else if (error) {
        throw text_error("Could not open file: " + font_path.string());
      }

      set_size(size);
    }

    void set_size(int size) {
      if (FT_Set_Pixel_Sizes(face, 0, size)) {
        throw text_error("Failed to set font resolution");
      }
      
      this->size = size;
    }
    
    void draw_bitmap(image_base& target, FT_Bitmap* bitmap, image_base::pos_t pen_x, image_base::pos_t pen_y) const {
      assert(bitmap->pixel_mode == FT_PIXEL_MODE_GRAY);
      
      uint8_t* buffer = bitmap->buffer;

      image_base::pos_t s_bitmap_rows =
        boost::numeric_cast<image_base::pos_t>(bitmap->rows);
      
      image_base::pos_t s_bitmap_width =
        boost::numeric_cast<image_base::pos_t>(bitmap->width);
      
      for (image_base::pos_t y = 0; y < s_bitmap_rows && y < target.get_height() + pen_y; y++) {
        for (image_base::pos_t x = 0; x < s_bitmap_width && x < target.get_width() + pen_x; x++) {
          color c(base);
          c.a = buffer[x + y * bitmap->width];
          target.safe_blend_pixel(pen_x + x, pen_y + y, c);
        }
      }
    }
    
    void draw(image_base& image, const std::string rawtext, int x, int y) const {
      FT_GlyphSlot slot = face->glyph;

      int error;

      int pen_x = x, pen_y = y;
      
      std::vector<uint32_t> text = utf8_decode(rawtext);
      
      for (std::vector<uint32_t>::iterator it = text.begin(); it != text.end(); it++ ) {
        uint32_t cc = *it;
        
        if (cc == '\n') {
          pen_x = x;
          pen_y += size + 2;
          continue;
        }
        
        error = FT_Load_Char( face, cc, FT_LOAD_RENDER ); 
        if ( error ) continue;
        
        draw_bitmap(image, &slot->bitmap, pen_x + slot->bitmap_left, pen_y - slot->bitmap_top);
        
        /* increment pen position */
        pen_x += slot->bitmap.width + 2;
      }
    }

    void set_color(color& c) {
      base = c;
    }
  };
}

#endif 
