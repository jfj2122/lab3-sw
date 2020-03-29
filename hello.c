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
#include <unistd.h>

int vga_ball_fd;

// Set the next coordinate position
// 0 is down, 1 is up
unsigned char set_next_col(unsigned char col, int dir) {
  char col_next;
  if (dir == 0) col_next = col - 0x01;
  else col_next = col + 0x01;

  return col_next;
}

// set next coordinate position
// 0 is right, 1 is left
unsigned char set_next_row(unsigned char row, int dir) {
  char row_next;
  if (dir == 0)row_next = row + 0x01;
  else row_next = row - 0x01;

  return row_next;
}

/* Read and print the background color */
void print_background_color() {
  vga_ball_arg_t vla;
  
  if (ioctl(vga_ball_fd, VGA_BALL_READ_BACKGROUND, &vla)) {
      perror("ioctl(VGA_BALL_READ_BACKGROUND) failed");
      return;
  }
  printf("%02x %02x %02x %02x %02x\n",
	 vla.background.red, vla.background.green, vla.background.blue,
	 vla.background.col, vla.background.row
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
  unsigned char row, col;
  static const char filename[] = "/dev/vga_ball";

  static const vga_ball_color_t colors[];
  char[] next_color[] = {
    { 0xff, 0x00, 0x00 }//, 0x2f, 0x0f }, /* Red */
    { 0x00, 0xff, 0x00 }//, 0x3f, 0x1f }, /* Green */
    { 0x00, 0x00, 0xff }//, 0x4f, 0x2f }, /* Blue */
    { 0xff, 0xff, 0x00 }//, 0x5f, 0x3f }, /* Yellow */
    { 0x00, 0xff, 0xff }//, 0x6f, 0x4f }, /* Cyan */
    { 0xff, 0x00, 0xff }//, 0x7f, 0x5f }, /* Magenta */
    { 0x80, 0x80, 0x80 }//, 0x8f, 0x6f }, /* Gray */
    { 0x00, 0x00, 0x00 }//, 0x9f, 0x7f }, /* Black */
    { 0xff, 0xff, 0xff }//, 0xaf, 0x8f }  /* White */
  };

# define COLORS 9

  printf("VGA ball Userspace program started\n");

  if ( (vga_ball_fd = open(filename, O_RDWR)) == -1) {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }

  printf("initial state: ");
  print_background_color();
  
  col = 0x2f;
  row = 0x2f;
  e_w = 1;
  n_s = 0;
  for (i = 0 ; i < 300 ; i++) {
    row = set_next_row(row, n_s);
    col = set_next_col(col, e_w);
    //colors[i % COLORS][3] = col;
    //colors[i % COLORS][4] = row;
    colors = {{next_color[i % COLORS][0],
	      next_color[i % COLORS][1],
	      next_color[i % COLORS][2],
	      col,
	      row }};
    set_background_color(&colors[i]);
    print_background_color();
    if (row == 0 || row == 360) {
      if (n_s == 0) n_s = 1;
      else n_s = 0;
    }
    if (col == 0 || col == 360) {
      if (e_w == 0) e_w = 1;
      else e_w = 0;
    }
    usleep(400000);
  }
  
  printf("VGA BALL Userspace program terminating\n");
  return 0;
}
