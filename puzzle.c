/*
  	puzzle.c

 	Solve 4x4 puzzle, each square tile has one of 4 connectors, and each connector is male/female


	   Top						0                  And if flipped (shiny side down) then      3

Left		  Right   		3		1                                                      0     2

	  Bottom					2                                                             1

On all tiles there are two male and two female connectors, and they are adjacent.
Sides 0 and 3 and the male connectors, and when flipping the tile over it is done so the two 
male connectors switch places.

Ignoring the fact that the pieces can be turned over, first find solutions for case where all sides are face up.


Piece order:     0  1  4  9   This is also the initial placement of tiles.  
				 3  2  5 10
				 8  7  6 11
				15 14 13 12 

Order this way to maximise speed at which mismatches are found as tiles are checked in sequence.

For adjacent tiles, conector type must match but be of opposite gender.

Rules:  connector[0][0]  N/A
		connector[0][1] == connector[1][3] ; gender[0][1] != gender[1][3]
		connector[0][2] == connector[3][0] ; gender[0][2] != gender[3][0]
		etc ... for all 24 adjoining edges

*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> /* Give us the bool type, defines bool, true=1, false=0 */

#include "puzzle.h"

/* Typedefs */
typedef enum {ArrowIn, ArrowOut, Cross, Round } Connector;
typedef enum {Female, Male} Gender ;


/* Globals */
static Connector connector[16][4] ;   	// 16 Tiles, 4 Sides each, index refers to position
static Gender    gender[16][4]    ;   	// Ditto
static bool      smooth_side_up[16] ; 	// Each time is eather smooth side up or not.
static int 		 tile_id[16] ;   		// unique id written on tile, 0-15, moves with tile
                           			  	// as tiles are rearranged.
static int tileOrder[16]; 				// Tile order - starts 0-15 ends 15-0 as order stepped



/* Functions */

// last ascending pair, return index of rightmost (last) pair of indices wth ascending values
// index is pointing to first of pair, i.e.e the left of the two.
// returns a negative number if no AP, which means we're at 15 - 0 in decending order.
// offset allow search to be pushed to left, to exclude tail. Use offset of 14 for full search.
int lastAPindex(int offset)
{
	int i = offset;
	for(; i>=0; i--) {
		if (tileOrder[i] < tileOrder[i+1]) {
			break;
		}
	}
	return i;
}

// scan from offset+1 to end of tileOrder, and find index with lowest higher value than at offset 
int smallestHVindex(int offset)
{
	int i;
	int testValue;
	int refValue = tileOrder[offset];
	int sHVindex = 16;
	int sHV = 16;
	for (i=offset+1; i<16; i++) {
		testValue = tileOrder[i];
		if (testValue > refValue && testValue < sHV) {
			sHV = testValue;
			sHVindex = i;
		}
	}
	if (sHV == 16) {
		return offset;
	} else {
		return sHVindex;
	}
}

// swap tiles at indices x and y
int swap(int x, int y)
{
	int temp;
	temp = tileOrder[y];
	tileOrder[y] = tileOrder[x];
	tileOrder[x] = temp;
	return 0;
}

// sorttail - reorder the values in ascending order from offset onwards
int sorttail(int offset) {
	for (int i=offset; i< 16; i++) {
		// find smallest value
		int sv = tileOrder[i];	// initialise at first, then scan for smaller
		int svi = i;
		for (int j = i+1; j < 16; j++) {
			int tv = tileOrder[j];
			if (tv < sv) {
				sv = tv;
				svi = j;
			}
		}
		if (svi != i) {
			swap(i, svi);
		}		
	}
	return 0;
}

// step to next iteration
int stepSequence()
{
	int lastAPix = lastAPindex(14) ;	// start at last pair, ix 14,15
	if (lastAPix >= 0) {
		int shvix = smallestHVindex(lastAPix) ;
		swap(lastAPix,shvix) ;
		sorttail(lastAPix+1) ;
		return 1 ;
	} else {
		return (-1) ; // cannot step any further
	}
}

// step to next iteration but force change at specific offset
// this can dramatically speed up the search process
// This forces the tile at index offset to change
int stepSequenceEarly(int offset)
{
	// can we find a higher value in the tail?
	int shvix = smallestHVindex(offset) ;
	if (shvix != offset) {
		swap(offset,shvix) ;
		sorttail(offset+1) ;		
		return 1;
	}

	// if not in tail, can head be stepped?
	int lastAPix = lastAPindex(offset) ;
	if (lastAPix >= 0) {
		int shvix = smallestHVindex(lastAPix) ;
		swap(lastAPix,shvix) ;
		sorttail(lastAPix+1) ;
		return 1;
	} else {
		return (-1) ;
	}

}

void rotate(int tile_index)
{
	// rotate sides of tile 90 degrees clockwise
	Connector temp_connector;
	Gender    temp_gender;

	temp_connector = connector[tile_index][0] ;
	connector[tile_index][0] = connector[tile_index][3] ;
	connector[tile_index][3] = connector[tile_index][2] ;
	connector[tile_index][2] = connector[tile_index][1] ;	
	connector[tile_index][1] = temp_connector ;

	temp_gender = gender[tile_index][0] ;
	gender[tile_index][0] = gender[tile_index][3] ;
	gender[tile_index][3] = gender[tile_index][2] ;
	gender[tile_index][2] = gender[tile_index][1] ;
	gender[tile_index][1] = temp_gender ;

}

void reset_tile(int tile_index)
{	// maybe more needed - but undo any rotations.
	initialiseTile(tile_index);
}

char *connector_name(Connector connectorval)
{
	if (connectorval == ArrowIn) return "ArrowIn";
	else if (connectorval == ArrowOut) return "ArrowOut";
	else if (connectorval == Cross) return "Cross";
	else if (connectorval == Round) return "Round";
	else return "Null";
}

char *gender_name(Gender genderval)
{
	if (genderval == Female) {
		return "Female";
	} else if (genderval == Male) {
		return "Male";
	} else {
		return "Null";
	}
}

char *smooth_up_name(bool SmoothSideUp)
{
	if (SmoothSideUp) {
		return "Up" ;
	} else {
		return "Down" ;
	}
}

void flip(int tile_index)
{
	// flip tile by swapping sides 0 with 3 and side 1 with 2 and toggling smooth_side_up
	Connector temp_connector;
	Gender    temp_gender;

	temp_connector = connector[tile_index][0];
	connector[tile_index][0] = connector[tile_index][3];
	connector[tile_index][3] = temp_connector;

	temp_connector = connector[tile_index][1];
	connector[tile_index][1] = connector[tile_index][2];
	connector[tile_index][2] = temp_connector;

	temp_gender = gender[tile_index][0];
	gender[tile_index][0] = gender[tile_index][3];
	gender[tile_index][3] = temp_gender;

	temp_gender = gender[tile_index][1];
	gender[tile_index][1] = gender[tile_index][2];
	gender[tile_index][2] = temp_gender;

	smooth_side_up[tile_index] = !smooth_side_up[tile_index] ;

}

void print_tile(int tile_index)
/* print the edge connector & gender for a specific tile */
{
	printf("\nTile %d : %-4s    ", tile_index, smooth_up_name(smooth_side_up[tile_index])) ;
	printf("Unique Tile ID: %d\n", tile_id[tile_index]) ;
	for (int i = 0; i < 4; i++) {
		printf("    %-8s %-6s\n", connector_name(connector[tile_index][i]), gender_name(gender[tile_index][i])) ;
	}
}

void initialiseAllTiles()
{
	int j;
	for (j=0; j<16; j++) {
		tileOrder[j] = j;
		initialiseTile(j);
	}
}

void initialiseTile(int tile_index)
{	/* Initialise the data based on actual pieces */

	// Just about all the variables are global, except for i 
	// which increments and is used just to simplify code copy/paste.

	int i = tile_index ;
	tile_id[i] = i ;
	smooth_side_up[i] = true ;
	
	if (i == 0) {
		connector[i][0] = Round ;		gender[i][0] = Male ;
		connector[i][1] = ArrowOut ;	gender[i][1] = Female ;
		connector[i][2] = Round ;		gender[i][2] = Female ;
		connector[i][3] = Round ;		gender[i][3] = Male ;
	} else if (i == 1) {
		connector[i][0] = ArrowIn ;		gender[i][0] = Male ;
		connector[i][1] = Cross ;		gender[i][1] = Female ;
		connector[i][2] = ArrowOut ;	gender[i][2] = Female ;
		connector[i][3] = ArrowIn ;		gender[i][3] = Male ;
	} else if (i == 2 ) {
		connector[i][0] = ArrowIn ;		gender[i][0] = Male ;
		connector[i][1] = Cross ;		gender[i][1] = Female ;
		connector[i][2] = Round ;		gender[i][2] = Female ;
		connector[i][3] = ArrowIn ;		gender[i][3] = Male ;
	} else if (i ==3) {
		connector[i][0] = Round ;		gender[i][0] = Male ;
		connector[i][1] = ArrowOut ;	gender[i][1] = Female ;
		connector[i][2] = ArrowIn ;		gender[i][2] = Female ;
		connector[i][3] = ArrowIn ;		gender[i][3] = Male ;
	} else if (i == 4) {
		connector[i][0] = Round ;		gender[i][0] = Male ;
		connector[i][1] = ArrowIn ;		gender[i][1] = Female ;
		connector[i][2] = Cross ;		gender[i][2] = Female ;
		connector[i][3] = ArrowIn ;		gender[i][3] = Male ;
	} else if (i == 5) {
		connector[i][0] = Round ;		gender[i][0] = Male ;
		connector[i][1] = Cross ;		gender[i][1] = Female ;
		connector[i][2] = Round ;		gender[i][2] = Female ;
		connector[i][3] = ArrowIn ;		gender[i][3] = Male ;
	} else if (i == 6) {
		connector[i][0] = ArrowIn ;		gender[i][0] = Male ;
		connector[i][1] = Round ;		gender[i][1] = Female ;
		connector[i][2] = ArrowOut ;	gender[i][2] = Female ;
		connector[i][3] = ArrowOut ;	gender[i][3] = Male ;
	} else if (i == 7) {
		connector[i][0] = ArrowOut ;	gender[i][0] = Male ;
		connector[i][1] = ArrowIn ;		gender[i][1] = Female ;
		connector[i][2] = Round ;		gender[i][2] = Female ;
		connector[i][3] = ArrowOut ;	gender[i][3] = Male ;
	} else if (i == 8) {
		connector[i][0] = Cross ;		gender[i][0] = Male ;
		connector[i][1] = ArrowIn ;		gender[i][1] = Female ;
		connector[i][2] = Cross ;		gender[i][2] = Female ;
		connector[i][3] = ArrowOut ;	gender[i][3] = Male ;
	} else if (i == 9) {
		connector[i][0] = Cross ;		gender[i][0] = Male ;
		connector[i][1] = Round ;		gender[i][1] = Female ;
		connector[i][2] = ArrowOut ;	gender[i][2] = Female ;
		connector[i][3] = ArrowOut ;	gender[i][3] = Male ;
	} else if (i == 10){
		connector[i][0] = ArrowIn ;		gender[i][0] = Male ;
		connector[i][1] = ArrowIn ;		gender[i][1] = Female ;
		connector[i][2] = ArrowOut ;	gender[i][2] = Female ;
		connector[i][3] = Cross ;		gender[i][3] = Male ;
	} else if (i == 11) {
		connector[i][0] = Round ;		gender[i][0] = Male ;
		connector[i][1] = ArrowOut ;	gender[i][1] = Female ;
		connector[i][2] = ArrowOut ;	gender[i][2] = Female ;
		connector[i][3] = Cross ;		gender[i][3] = Male ;
	} else if (i == 12) {	
		connector[i][0] = Round ;		gender[i][0] = Male ;
		connector[i][1] = Round ;		gender[i][1] = Female ;
		connector[i][2] = ArrowIn ;		gender[i][2] = Female ;
		connector[i][3] = Cross ;		gender[i][3] = Male ;
	} else if (i == 13) {
		connector[i][0] = Round ;		gender[i][0] = Male ;
		connector[i][1] = Round ;		gender[i][1] = Female ;
		connector[i][2] = Cross ;		gender[i][2] = Female ;
		connector[i][3] = Cross ;		gender[i][3] = Male ;
	} else if (i == 14) {
		connector[i][0] = ArrowOut ;	gender[i][0] = Male ;
		connector[i][1] = ArrowOut ;	gender[i][1] = Female ;
		connector[i][2] = Round ;		gender[i][2] = Female ;
		connector[i][3] = Round ;		gender[i][3] = Male ;
	} else if (i == 15) {
		connector[i][0] = ArrowOut ;	gender[i][0] = Male ;
		connector[i][1] = Cross ;		gender[i][1] = Female ;
		connector[i][2] = Round ;		gender[i][2] = Female ;
		connector[i][3] = Round ;		gender[i][3] = Male ;
	} else {
		printf("We should never get here: illegal index in initialiseTile()");
		exit(1);
	}
	
}

int printTileOrder()
{
	printf("\nTiles are in following order: "	);
	for (int i=0; i < 16; i++)
	{
		printf("%d ", tileOrder[i]);
	}
	printf("\n\n");
}

int checkfit(int tile_index){

	/* put stuff here to check connections for this tile to previous tiles */
	return 0;
}


/* main */

int main(int argc, char **argv) /* args will be ignored for now */
{
	int tile, rot;
	bool flipped; 

	initialiseAllTiles();


/* Need to do the 0,1,2,3 ... 15 increase thing until 15, 14, 13 .. 0  is reached 
	   And find how to jump the sequence to save time too.
*/ 

/*
	printf("Here are the tiles that will be analysed for 4x4 grid:\n\n");

	for (int i = 0; i < 16; i++) {
		print_tile(i);
	}

	printf("\n\n");
*/


/*
	printf("Here is the first tile flipped then rotated:\n\n");

	print_tile(0);
	flip(0);
	print_tile(0);
	rotate(0);
	print_tile(0);
*/	

/*
	// Test code - 
	printf("Initial tile order:\n");
	printTileOrder();
	for (long li = 1; li < 100; li++) {
		int result;
		int offset = li % 15 ;
		printf("Step %ld  offset %d",li,offset);
		result = stepSequenceEarly(offset);
		if (result < 0) {
			break;
		}
		printTileOrder();
	}
*/

} /* end main */
