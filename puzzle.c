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

#define DEBUG false 

/* Typedefs */
typedef enum {ArrowIn, ArrowOut, Cross, Round } Connector;
typedef enum {Female, Male} Gender ;

static struct Tile { 	Connector c[4];
				Gender g[4];
				bool isUp;
				int rotation;
				int id;
			} tileSpec[16], tile[16];

bool stopOnFirstSolution = false;
bool useFlippedTiles = false;

/* Globals */
// static Connector connector[16][4] ;   	// 16 Tiles, 4 Sides each, index refers to position
// static Gender    gender[16][4]    ;   	// Ditto
// static int  	 rotation[16]; 			// 0 to 3, 0 is original, steps once each time rotated. 
// static bool      smooth_side_up[16] ; 	// Each time is eather smooth side up or not.
// static int 		 tile_id[16] ;   		// unique id written on tile, 0-15, points to index
                           			  	// of tileOrder.
// tileOrder[n] -> which tile is at position n
// tile_id[n]   -> where is tile n in array tileOrder


static int 		tileOrder[16]; 			// Tile order - starts 0-15 ends 15-0 as order stepped

static int 	solutionCount = 0 ;

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

// swap tiles at positions x and y
void swap(int x, int y)
{	// https://www.geeksforgeeks.org/are-array-members-deeply-copied/ - should work
	struct Tile tempTile;
	tempTile = tile[x];
	tile[x] = tile[y];
	tile[y] = tempTile;
	// now update tileOrder as well
	int i;
	i = tileOrder[x];
	tileOrder[x] = tileOrder[y];
	tileOrder[y] = i; 
}

// sorttail - reorder the values in ascending order from offset onwards
//            also reset tiles in tail to initial state.
void sorttail(int offset) 
{
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
	// now tail is sorted, we need to reset each tile in the tail to initial state
	for (int i=offset; i< 16; i++) {
		reset(i);
	}
}

// step to next iteration
int stepSequence()
{
	int lastAPix = lastAPindex(14) ;	// start at last pair, ix 14,15
	if (lastAPix >= 0) {
		int shvix = smallestHVindex(lastAPix) ;
		swap(lastAPix,shvix) ;
		reset(lastAPix) ;
		sorttail(lastAPix+1) ;
		return 1 ;
	} else {
		return (-1) ; // cannot step any further
	}
}

// step to next iteration but force change at specific offset
// this can dramatically speed up the search process
// This forces the tile at index offset to change
int stepSequenceOffset(int offset)
{
	// can we find a higher value in the tail?
	int shvix = smallestHVindex(offset) ;
	if (DEBUG){
		printf("170 - offset is %d, shvix is %d \n", offset, shvix);
	}
	if (shvix != offset) {
		swap(offset,shvix) ;
		reset(offset) ;
		sorttail(offset+1) ;		
		return 1;
	}

	// if not in tail, can head be stepped?
	int lastAPix = lastAPindex(offset) ;
	if (lastAPix >= 0) {
		int shvix = smallestHVindex(lastAPix) ;
		swap(lastAPix,shvix) ;
		reset(lastAPix) ;
		sorttail(lastAPix+1) ;
		return 1;
	} else {
		return (-1) ;
	}
}

void rotate(int pos)
{
	// rotate sides of tile at pos 90 degrees clockwise
	Connector temp_connector;
	Gender    temp_gender;

	if (DEBUG) {
		printf("in rotate(), for tile pos %d \n",pos) ;
	}

	temp_connector = tile[pos].c[0] ;
	tile[pos].c[0] = tile[pos].c[1] ;
	tile[pos].c[1] = tile[pos].c[2] ;
	tile[pos].c[2] = tile[pos].c[3] ;
	tile[pos].c[3] = temp_connector ;

	temp_gender    = tile[pos].g[0] ;
	tile[pos].g[0] = tile[pos].g[1] ;
	tile[pos].g[1] = tile[pos].g[2] ;
	tile[pos].g[2] = tile[pos].g[3] ;
	tile[pos].g[3] = temp_gender ;

	// rotate the tile clockwise, back to 0 if past 3.
	tile[pos].rotation = (tile[pos].rotation + 1 ) % 4 ;
	if (DEBUG) {
		printf("new rotation value: %d \n", tile[pos].rotation);
	}
}

void reset(int pos)
{	// find tile id and reload tile spec - the id value should not change.
	int index = tile[pos].id ;
	tile[pos] = tileSpec[index];
}

char *connectorName(Connector connectorval)
{
	if (connectorval == ArrowIn) return "ArrowIn";
	else if (connectorval == ArrowOut) return "ArrowOut";
	else if (connectorval == Cross) return "Cross";
	else if (connectorval == Round) return "Round";
	else return "Null";
}

char *genderName(Gender genderval)
{
	if (genderval == Female) {
		return "Female";
	} else if (genderval == Male) {
		return "Male";
	} else {
		return "Null";
	}
}

char *upOrDownName(bool isUp)
{
	if (isUp) {
		return "Up" ;
	} else {
		return "Down" ;
	}
}

void flip(int pos)
{
	// flip tile by swapping sides 0 with 3 and side 1 with 2 and toggling smooth_side_up
	// assume that tile will be reset before flipping?
	Connector temp_connector;
	Gender    temp_gender;

	temp_connector = tile[pos].c[0];
	tile[pos].c[0] = tile[pos].c[3];
	tile[pos].c[3] = temp_connector;

	temp_connector = tile[pos].c[1];
	tile[pos].c[1] = tile[pos].c[2];
	tile[pos].c[2] = temp_connector;
	
	temp_gender = tile[pos].g[0];
	tile[pos].g[0] = tile[pos].g[3];
	tile[pos].g[3] = temp_gender;
	
	temp_gender = tile[pos].g[1];
	tile[pos].g[1] = tile[pos].g[2];
	tile[pos].g[2] = temp_gender;

	tile[pos].isUp = !(tile[pos].isUp) ;

}

bool nudgeable(int pos)
{
	if (tile[pos].rotation < 3) {
		return true;
	} 
	if (useFlippedTiles && tile[pos].isUp) {
		return true;
	}
	return false;
}

bool nudge(int pos)
{
	if (nudgeable(pos)) {
		if (tile[pos].rotation < 3) {
			rotate(pos);
		} else {
			reset(pos);
			flip(pos);
		}
		return true;
	} else {
		return false;
	}
}

void printTile(int pos)
/* print the edge connector & gender for a tile is a specified position */
{
	printf("\nTile position %d : %-4s    ", pos, upOrDownName(tile[pos].isUp)) ;
	printf("Unique Tile ID: %d\n", tile[pos].id ) ;
	for (int i = 0; i < 4; i++) {
		printf("    %-8s %-6s\n", connectorName(tile[pos].c[i]), genderName(tile[pos].g[i]) ) ;
	}
}

void printSolution()
{
	printf("\n\nSolution found [%d]:\n",solutionCount);
	for (int i=0; i < 16; i++) {
		printTile(i);
	}
}

void initialiseTiles()
{	// initialiase tileOrder and tile[].
	for (int i=0; i<16; i++) {
		tileOrder[i] = i;
		tile[i] = tileSpec[i];
	}
}

void initialiseTileSpec()
{	/* Initialise the data based on actual pieces */
	
	for (int i = 0; i < 16; i++) {
		
		tileSpec[i].id = i;
		tileSpec[i].rotation = 0;
		tileSpec[i].isUp = true ;
		
		if (i == 0) {
			tileSpec[i].c[0] = Round ;			tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = ArrowOut ;		tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = Round ;			tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = Round ;			tileSpec[i].g[3] = Male ;
		} else if (i == 1) {
			tileSpec[i].c[0] = ArrowIn ;		tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = Cross ;			tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = ArrowOut ;		tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = ArrowIn ;		tileSpec[i].g[3] = Male ;
		} else if (i == 2 ) {
			tileSpec[i].c[0] = ArrowIn ;		tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = Cross ;			tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = Round ;			tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = ArrowIn ;		tileSpec[i].g[3] = Male ;
		} else if (i ==3) {
			tileSpec[i].c[0] = Round ;			tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = ArrowOut ;		tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = ArrowIn ;		tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = ArrowIn ;		tileSpec[i].g[3] = Male ;
		} else if (i == 4) {
			tileSpec[i].c[0] = Round ;			tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = ArrowIn ;		tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = Cross ;			tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = ArrowIn ;		tileSpec[i].g[3] = Male ;
		} else if (i == 5) {
			tileSpec[i].c[0] = Round ;			tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = Cross ;			tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = Round ;			tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = ArrowIn ;		tileSpec[i].g[3] = Male ;
		} else if (i == 6) {
			tileSpec[i].c[0] = ArrowIn ;		tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = Round ;			tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = ArrowOut ;		tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = ArrowOut ;		tileSpec[i].g[3] = Male ;
		} else if (i == 7) {
			tileSpec[i].c[0] = ArrowOut ;		tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = ArrowIn ;		tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = Round ;			tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = ArrowOut ;		tileSpec[i].g[3] = Male ;
		} else if (i == 8) {
			tileSpec[i].c[0] = Cross ;			tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = ArrowIn ;		tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = Cross ;			tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = ArrowOut ;		tileSpec[i].g[3] = Male ;
		} else if (i == 9) {
			tileSpec[i].c[0] = Cross ;			tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = Round ;			tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = ArrowOut ;		tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = ArrowOut ;		tileSpec[i].g[3] = Male ;
		} else if (i == 10){
			tileSpec[i].c[0] = ArrowIn ;		tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = ArrowIn ;		tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = ArrowOut ;		tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = Cross ;			tileSpec[i].g[3] = Male ;
		} else if (i == 11) {
			tileSpec[i].c[0] = Round ;			tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = ArrowOut ;		tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = ArrowOut ;		tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = Cross ;			tileSpec[i].g[3] = Male ;
		} else if (i == 12) {	
			tileSpec[i].c[0] = Round ;			tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = Round ;			tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = ArrowIn ;		tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = Cross ;			tileSpec[i].g[3] = Male ;
		} else if (i == 13) {
			tileSpec[i].c[0] = Round ;			tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = Round ;			tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = Cross ;			tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = Cross ;			tileSpec[i].g[3] = Male ;
		} else if (i == 14) {
			tileSpec[i].c[0] = ArrowOut ;		tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = ArrowOut ;		tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = Round ;			tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = Round ;			tileSpec[i].g[3] = Male ;
		} else if (i == 15) {
			tileSpec[i].c[0] = ArrowOut ;		tileSpec[i].g[0] = Male ;
			tileSpec[i].c[1] = Cross ;			tileSpec[i].g[1] = Female ;
			tileSpec[i].c[2] = Round ;			tileSpec[i].g[2] = Female ;
			tileSpec[i].c[3] = Round ;			tileSpec[i].g[3] = Male ;
		} else {
			printf("We should never get here: illegal index in initialiseTile()");
			exit(1);
		}
	}
}

int printTileOrder()
{
	printf("\nTile order: "	);
	for (int i=0; i < 16; i++)
	{
		printf("%d ", tileOrder[i]);
	}
	printf("\n\n");
}


bool sidesMatch(int pos1, int side1, int pos2, int side2) 
{
		bool result;
		if ((tile[pos1].g[side1] != tile[pos2].g[side2]) && (tile[pos1].c[side1] == tile[pos2].c[side2])) {
			result = true;
		} else {
			result = false;
		}
		return result;
}

bool checkTile(int pos)
{
	// return true if tile fits with all earlier tiles. 
	
	bool result;

	switch (pos) {

		case 0:
			// first tile is always ok
			result = true;
			break;
			
		case 1:
			// check tile 1 side 3 against tile 0 side 1
			if (sidesMatch(1,3,0,1)) {
				result = true;
			} else {
				result = false;
			}
			break;
			
		case 2:
			// check tile 2 side 0 against tile 1 side 2
			if (sidesMatch(2,0,1,2)) {
				result = true;
			} else {
				result = false;
			}
			break;
			
		case 3:
			// check tile 3 side 1 against tile 2 side 3, and 
			// check tile 3 side 0 against tile 0 side 2
			if (sidesMatch(3,1,2,3) && sidesMatch(3,0,0,2)) {
				result = true;
			} else {
				result = false;
			}			 
			break;
			 
		case 4:
			// check tile 4 side 3 against tile 1 side 1 
			if  (sidesMatch(4,3,1,1)) {   
				result = true;
			} else {
				result = false;
			}			 
			break;
			
		case 5:
			// check tile 5 side 0 against tile 4 side 2 and tile 5 side 3 v tile 2 side 1
			if (sidesMatch(5,0,4,2) && sidesMatch(5,3,2,1)) {
				result = true;
			} else {
				result = false;
			}			 
			break;
			
		case 6:
			// check tile 6 side 0 against tile 5 side 2 
			if  (sidesMatch(6,0,5,2)) {   
				result = true;
			} else {
				result = false;
			}			 
			break;
			
		case 7:
			// check tile 7 side 0 against tile 2 side 2 and tile 7 side 1 v tile 6 side 3
			if (sidesMatch(7,0,2,2) && sidesMatch(7,1,6,3)) {
				result = true;
			} else {
				result = false;
			}			 
			break;
			
		case 8:
			// check tile 8 side 0 against tile 3 side 2 and tile 8 side 1 v tile 7 side 3
			if (sidesMatch(8,0,3,2) && sidesMatch(8,1,7,3)) {
				result = true;
			} else {
				result = false;
			}			 
			break;
			
		case 9:
			// check tile 9 side 3 against tile 4 side 1 
			if  (sidesMatch(9,3,4,1)) {   
				result = true;
			} else {
				result = false;
			}			 
			break;
			
		case 10:
			// check tile 10 side 0 against tile 9 side 2 and tile 10 side 3 v tile 5 side 1
			if (sidesMatch(10,0,9,2) && sidesMatch(10,3,5,1)) {
				result = true;
			} else {
				result = false;
			}			 
			break;
			
		case 11:
			// check tile 11 side 0 against tile 10 side 2 and tile 11 side 3 v tile 6 side 1
			if (sidesMatch(11,0,10,2) && sidesMatch(11,3,6,1)) {
				result = true;
			} else {
				result = false;
			}			 
			break;
			
		case 12:
			// check tile 12 side 0 against tile 11 side 2
			if (sidesMatch(12,0,11,2)) {
				result = true;
			} else {
				result = false;
			}			 
			break;
			
		case 13:
			// check tile 13 side 0 against tile 6 side 2 and tile 13 side 1 v tile 12 side 3
			if (sidesMatch(13,0,6,2) && sidesMatch(13,1,12,3)) {
				result = true;
			} else {
				result = false;
			}			 
			break;
			
		case 14:
			// check tile 14 side 0 against tile 7 side 2 and tile 14 side 1 v tile 13 side 3
			if (sidesMatch(14,0,7,2) && sidesMatch(14,1,13,3)) {
				result = true;
			} else {
				result = false;
			}			 
			break;
			
		case 15:
			// check tile 15 side 0 against tile 8 side 2 and tile 15 side 1 v tile 14 side 3
			if (sidesMatch(15,0,8,2) && sidesMatch(15,1,14,3)) {
				result = true;
			} else {
				result = false;
			}			 
			break;

		default:
			break;
	}
	return result;
}

int checkForSolution()
{
	int i;
	for (i=1; i < 16; i++) {
		if (!checkTile(i)) { // if mismatch found return first tile where this occurs
			return i;
		}
	}
	return 0; // solution found!
}

/* main */

int main(int argc, char **argv) /* args will be ignored for now */
{
	int n, result;
	initialiseTileSpec();
	initialiseTiles();

	// printTileOrder();
	
	while (true) {
		n = checkForSolution();
		if (DEBUG) {
			printf("Checking for solution: result is %d\n",n );
		}
		if (n == 0) {
			solutionCount++;
			printSolution();
			if (stopOnFirstSolution) {
				break;
			} else {
				n = 15;
			}
		} 
		if (nudge(n)) {
			continue;
		} else {
			if (n == 15) {
				result = stepSequence();
			} else { 
				result = stepSequenceOffset(n);
			}
			if (result == 1) {
				if (DEBUG) {
					printTileOrder();
				}
				continue;
			} else {
				printf("End of sequence ...\n");
				break;
			}

		}

	}


/*
	printf("Here is the current tiles layout:\n\n");

	for (int i = 0; i < 16; i++) {
		printTile(i);
	}

	printf("\n\n");
*/



/*
	printf("Here is the first tile flipped then rotated:\n\n");

	printTile(0);
	flip(0);
	printTile(0);
	rotate(0);
	printTile(0);
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
