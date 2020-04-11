/*
 * Userspace program that communicates with the vga_ball device driver
 * through ioctls
 *
 * Stephen A. Edwards
 * Columbia University
 */

#include <stdio.h>
#include "vga_ball.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

int vga_ball_fd;

// Set the next coordinate position
// 0 is left, 1 is right
uint16_t set_next_col(uint16_t col, int dir) {
  uint16_t col_next;
  if (dir == 0) col_next = col + 1;
  else col_next = col - 1;

  return col_next;
}

// set next coordinate position
// 0 is down, 1 is right
uint16_t set_next_row(uint16_t row, int dir) {
  uint16_t row_next;
  if (dir == 0)row_next = row - 1;
  else row_next = row + 1;

  return row_next;
}

/* Read and print the background color */
void print_background_color() {
  vga_ball_arg_t vla;
  
  if (ioctl(vga_ball_fd, VGA_BALL_READ_BACKGROUND, &vla)) {
      perror("ioctl(VGA_BALL_READ_BACKGROUND) failed");
      return;
  }
  printf("%02x %02x %02x %02x %02x %02x %02x\n",
	 vla.background.red, vla.background.green, vla.background.blue,
	 vla.background.col1, vla.background.col2,
	 vla.background.row1, vla.background.row2
	 );
}

/* Set the background color */
void set_background_color(const vga_ball_color_t *c)
{
  vga_ball_arg_t vla;
  vla.background = *c;
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &vla)) {
      perror("ioctl(VGA_BALL_SET_BACKGROUND) failed");
      return;
  }
}

int main()
{
  vga_ball_arg_t vla;
  int i, n_s, e_w;
  uint16_t row, col;
  static const char filename[] = "/dev/vga_ball";

  vga_ball_color_t colors;
  
  char next_color[9][3] = {
    { 0xff, 0x00, 0x00 },/* Red */
    { 0x00, 0xff, 0x00 },/* Green */
    { 0x00, 0x00, 0xff },/* Blue */
    { 0xff, 0xff, 0x00 },/* Yellow */
    { 0x00, 0xff, 0xff },/* Cyan */
    { 0xff, 0x00, 0xff },/* Magenta */
    { 0x80, 0x80, 0x80 },/* Gray */
    { 0xff, 0x00, 0x00 },/* red2 */
    { 0x00, 0x00, 0x00 } /* Black */
  };

  #define COLORS 9
  
  if ( (vga_ball_fd = open(filename, O_RDWR)) == -1) {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }

  printf("initial state: ");
  print_background_color();

  row = 240;
  col = 320;

  e_w = 0;
  n_s = 1;
  colors.red = 0x00;
  colors.green = 0x00;
  colors.blue = 0x00;
  while (1) {
    row = set_next_row(row, n_s);
    col = set_next_col(col, e_w);
    colors.col1 = (col >> 8) & 0xff;
    colors.col2 = col & 0xff;
    colors.row1 = (row >> 8) & 0xff;
    colors.row2 = row & 0xff;
    set_background_color(&colors);
    print_background_color();
    printf("row: %d \ncol: %d\n", row, col);
    if (row == 25 || row == 455) {
      if (n_s == 0) n_s = 1;
      else n_s = 0;
      colors.red = next_color[i % COLORS][0];
      colors.green = next_color[i % COLORS][1];
      colors.blue = next_color[i % COLORS][2];
    }
    if (col == 25 || col == 1255) {
      if (e_w == 0) e_w = 1;
      else e_w = 0;
      colors.red = next_color[i % COLORS][0];
      colors.green = next_color[i % COLORS][1];
      colors.blue = next_color[i % COLORS][2];
    }
    usleep(5000);
    i++;
  }
  
  printf("VGA BALL Userspace program terminating\n");
  return 0;
}
