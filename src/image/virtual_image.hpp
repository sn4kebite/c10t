#ifndef VIRTUAL_IMAGE
#define VIRTUAL_IMAGE

#include <string.h>

class virtual_image : public image_base {
private:
  image_base* base;
  size_t x, y;
public:
  virtual_image(size_t w, size_t h, image_base* base, size_t x, size_t y) : image_base(w, h), base(base), x(x), y(y)
  {
  }
  
  void blend_pixel(size_t x, size_t y, color &c) {
    base->blend_pixel(this->x + x, this->y + y, c);
  }
  
  void set_pixel(size_t x, size_t y, color& c) {
    base->set_pixel(this->x + x, this->y + y, c);
  }
  
  void get_pixel(size_t x, size_t y, color& c) {
    base->get_pixel(this->x + x, this->y + y, c);
  }
  
  void get_line(size_t y, size_t x, size_t width, color* c) {
    size_t o_x = this->x + x;
    size_t o_y = this->y + y;
    size_t p_width = 0;
    
    if (o_x > base->get_width()) { goto exit_zero; }
    if (o_y > base->get_height()) { goto exit_zero; }
    
    p_width = std::min(base->get_width() - o_x, width);
    
    if (p_width == width) {
      base->get_line(o_y, o_x, width, c);
    }
    else {
      base->get_line(o_y, o_x, p_width, c);
      // fill up the rest with zeroes
      memset(c + p_width, 0x0, sizeof(color) * (width - p_width));
    }

    return;

exit_zero:
    memset(c, 0x0, sizeof(color) * width);
  }
};

#endif /* VIRTUAL_IMAGE */
