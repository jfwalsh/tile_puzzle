// puzzle.h
#ifndef PUZZLE_H_
#define PUZZLE_H_
int lastAPindex();
int smallestHVindex(int offset);
void swap(int x, int y);
void sorttail(int offset);
char *smooth_up_name(bool SmoothSideUp);
void print_tile(int pos);
void initialiseTile(int pos);
void initialiseTileSpec();
void reset(int pos);
void rotate(int pos);
int stepSequenceOffset(int offset);
int stepSequence();
void flip(int pos);
bool checkTile(int pos);
bool sidesMatch(int tile1, int side1, int tile2, int side2);
bool nudge(int pos);
bool nudgeable(int pos);
#endif