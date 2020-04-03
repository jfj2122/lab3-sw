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
unsigned char set_next_col(uint16_t col, int dir) {
  char col_next;
  if (dir == 0) col_next = col + 1;
  else col_next = col - 1;

  return col_next;
}

// set next coordinate position
// 0 is down, 1 is right
unsigned char set_next_row(uint16_t row, int dir) {
  char row_next;
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
  //unsigned char row1, row2, col1, col2;
  static const char filename[] = "/dev/vga_ball";

  vga_ball_color_t colors;/*[] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00}
    };*/
  
  char next_color[9][3] = {
    { 0xff, 0x00, 0x00 },//, 0x2f, 0x0f }, /* Red */
    { 0x00, 0xff, 0x00 },//, 0x3f, 0x1f }, /* Green */
    { 0x00, 0x00, 0xff },//, 0x4f, 0x2f }, /* Blue */
    { 0xff, 0xff, 0x00 },//, 0x5f, 0x3f }, /* Yellow */
    { 0x00, 0xff, 0xff },//, 0x6f, 0x4f }, /* Cyan */
    { 0xff, 0x00, 0xff },//, 0x7f, 0x5f }, /* Magenta */
    { 0x80, 0x80, 0x80 },//, 0x8f, 0x6f }, /* Gray */
    { 0x00, 0x00, 0x00 },//, 0x9f, 0x7f }, /* Black */
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

  row = 200;
  col = 200;
  //col1 = 0x00;
  //row1 = 0x00;
  
  e_w = 1;
  n_s = 0;
  for (i = 0 ; i < 2000 ; i++) {
    //row = set_next_row(row, n_s);
    col = set_next_col(col, e_w);
    //colors[i % COLORS][3] = col;
    //colors[i % COLORS][4] = row;
    /*&colors[0] = { next_color[i % COLORS][0],
		   next_color[i % COLORS][1],
		   next_color[i % COLORS][2],
		   col,
		   row };*/
    colors.red = 0x00;//next_color[i % COLORS][0];
    colors.green = 0x00;//next_color[i % COLORS][1];
    colors.blue = 0x00;//next_color[i % COLORS][2];
    colors.col1 = (col >> 8) & 0xff;
    colors.col2 = col & 0xff;
    colors.row1 = (row >> 8) & 0xff;
    colors.row2 = row & 0xff;
    set_background_color(&colors);
    print_background_color();
    printf("Cycle %d \n", i);
    if (row == 0 || row == 10000) {
      if (n_s == 0) n_s = 1;
      else n_s = 0;
    }
    if (col == 0 || col == 900) {
      if (e_w == 0) e_w = 1;
      else e_w = 0;
      }
    usleep(20000);
  }
  
  printf("VGA BALL Userspace program terminating\n");
  return 0;
}
