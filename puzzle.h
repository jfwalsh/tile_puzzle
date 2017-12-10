// puzzle.h
#ifndef PUZZLE_H_
#define PUZZLE_H_
int lastAPindex();
int smallestHVindex(int offset);
int swap(int x, int y);
int sorttail(int offset);
char *smooth_up_name(bool SmoothSideUp);
void print_tile(int tile_index);
void initialiseTile(int tile_index);
void initialiseAllTiles();
void reset_tile(int tile_index);
void rotate(int tile_index);
int stepSequenceEarly(int offset);
int stepSequence();
void flip(int tile_index);
int checkfit(int tile_index);

#endif