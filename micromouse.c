/*
 * micromouse.c
 *
 * Program to find shortest path in maze using modified flood-fill
 * algorithm.
 *
 * Contributors:
 * Piyush Zambad
 * Ganesh Divekar
 *
 * Aug 2009
 */

#include <stdio.h>
#include <stdlib.h>
/*
 * CELLSIZE
 * Size of the maze that micromouse has to slove.
 */
#define CELLSIZE 16

int k,l;

/*
 * lcr
 * Input to the micromouse about the walls present to the Left, Center and
 * Right of it. On a physical robot, these inputs would come from 3 infrared
 * sensors - one in each direction.
 *
 * Possible Values:
 * Decimal | Binary | Explaination
 * 0 	   | 000    | Walls on no side
 * 1       | 001    | Wall to the right/east side
 * 2       | 010    | Wall to the straight/north side
 * 3       | 011    | Walls to north and east sides
 * 4       | 100    | Wall to the left/west side
 * 5       | 101    | Walls to west and east sides
 * 6       | 110    | Walls to west and north sides
 * 7       | 111    | Walls on all the 3 sides (west/north/east)
 */
int lcr;


/*
 * Path-lengths of tracing/retracing to center of maze/back to starting point
 * by the micromouse.
 */
unsigned char plength1,plength2,plength=0;

unsigned char choice;
unsigned char a[CELLSIZE][CELLSIZE],b[CELLSIZE][CELLSIZE],i,j;

/*
 * flag[][]
 * 2D array representing the presence of walls across each cell.
 * Each index utilizes first 4 bits, representing the walls present
 * in the 4 directions. The value is saved as (n e s w), where MSB
 * represents whether wall is present (1) or not (0) in the (n)orth direction,
 * LSB represents in (w)est direction and so on.
 *
 *         n(msb)
 *
 *  w    [i,j]    e
 *(lsb)
 *         s
 *
 * The seventh bit is set to 1 when the micromouse visits that cell the first
 * time.
 */
unsigned char flag[CELLSIZE][CELLSIZE];

/*
 * turn:
 * Current direction of the head of micromouse relative to the original
 * direction at the start (assumed to be straight/north(0)).
 *
 * 0 -> Straight/North (^)
 * 1 -> Right/East (>)
 * 2 -> Reverse/South (\/)
 * 3 -> Left/West (<)
 */
unsigned char turn = 0;

unsigned char count = 0;

/*
 * trflag
 *
 * This variable represents how many times micromouse has been to the center of
 * the maze and back to the starting point.
 * For eg. - 1 represents micormouse has reached center of the maze for 1st time,
 * 2 represents micromouse has reached back to the starting point and so on.
 */
unsigned char trflag = 0;

/*
 * STRAIGHT
 * Value/cost of the cell, immediately north/straight of the current cell
 * relative to the current direction of the micromouse. 
 */
#define STRAIGHT ( (turn==0) * a[i+1][j] +\
		   (turn==1) * a[i][j+1] +\
		   (turn==2) * a[i-1][j] +\
  		   (turn==3) * a[i][j-1] )

/*
 * RIGHT
 * Value/cost of the cell, immediately east/right of the current cell 
 * relative to the current direction of the micromouse. 
 */
#define RIGHT ( (turn==3) * a[i+1][j] +\
		(turn==0) * a[i][j+1] +\
		(turn==1) * a[i-1][j] +\
 		(turn==2) * a[i][j-1] )

/*
 * REVERSE
 * Value/cost of the cell immediately south/reverse of the current cell 
 * relative to the current direction of the micromouse. 
 */
#define REVERSE ( (turn==2) * a[i+1][j] +\
		  (turn==3) * a[i][j+1] +\
		  (turn==0) * a[i-1][j] +\
		  (turn==1) * a[i][j-1] )

/*
 * LEFT
 * Value/cost of the cell, immediately west/left of the current cell 
 * relative to the current direction of the micromouse. 
 */
#define LEFT ( (turn==1)*a[i+1][j] + (turn==2)*a[i][j+1] + (turn==3)*a[i-1][j] + (turn==0)*a[i][j-1] )

/*
 * PRESENT
 * Value/cost of the cell, currently the micromouse is at.
 */
#define PRESENT a[i][j]


/*
 * printmaze()
 *
 * Function to print the cell-values of all the cells in the entire maze
 * of size [CELLSIZE x CELLSIZE] in an user-understandable format.
 */
void
printmaze()
{
	for(k=CELLSIZE-1;k>=0;k--)
	{
		for(l=0;l<=CELLSIZE-1;l++)
		if(k!=i||l!=j) 
			printf("\t%d",a[k][l]);
		else
			printf("\t%d*%d",a[k][l],turn);
		printf("\n\n\n");
	}
	printf("\n\n\n\n\n\n\n\n");
}


/*
 * printflag()
 *
 * Function to print the flag-values of all the cells in the entire maze
 * of size [CELLSIZE x CELLSIZE] in an user-understandable format.
 */
void printflag()
{    
     	printf("\n\n\n\tFLAGS\n");
	

	for(k=CELLSIZE-1; k>=0; k--)
	{
		for(l=0; l<=CELLSIZE-1; l++)
		{
			printf("\t%d", flag[k][l]);
		}
		printf("\n");
	}  
}

void printchoice()
{
   		printf("Do u want to continue ?");
		scanf("%s",&choice);
}

void printtrace()
{
     	printf("\t\t\tTRACE\n");
	for(k=CELLSIZE-1;k>=0;k--)
	{
		for(l=0;l<=CELLSIZE-1;l++)
		if(flag[k][l]&64) 
			printf("\t**");
		else
			printf("\t%d",a[k][l]);
		printf("\n\n\n");
	}
	printf("\nPathlength of tracing is %d and Pathlength of retracing is %d\n\n\n\n\n",plength1,plength2);
	
}

/*
 * filtermaze()
 *
 * After the maze has been solved for the first time, filter out the untoched
 * cells.
 */
void
filtermaze()
{     
	if((trflag%2) == 0) {
		for(k=0; k<=(CELLSIZE/2)-1; k++)
		{
			for(l=0; l<=(CELLSIZE/2)-1; l++)
			{
				/*
				 * Set the cells that have not been visited by
				 * micromouse to 200
				 */
				if (a[CELLSIZE/2+k][CELLSIZE/2-1-l] == k+l)
					a[CELLSIZE/2+k][CELLSIZE/2-1-l] = 200;
				if(a[CELLSIZE/2-1-k][CELLSIZE/2-1-l]==k+l)
					a[CELLSIZE/2-1-k][CELLSIZE/2-1-l] = 200;
				if(a[CELLSIZE/2+k][CELLSIZE/2+l]==k+l)
					a[CELLSIZE/2+k][CELLSIZE/2+l] = 200;
				if(a[CELLSIZE/2-1-k][CELLSIZE/2+l]==k+l)
					a[CELLSIZE/2-1-k][CELLSIZE/2+l] = 200;
			}
		}
		a[CELLSIZE/2][CELLSIZE/2-1] = a[CELLSIZE/2-1][CELLSIZE/2-1] =\
			a[CELLSIZE/2][CELLSIZE/2] =a[CELLSIZE/2-1][CELLSIZE/2] = 0;
	} else { 
		for(k=0;k<=(CELLSIZE)-1;k++)
			for(l=0;l<=(CELLSIZE)-1;l++)
				if(a[k][l]==k+l)
					a[k][l] = 200;
		a[0][0]=0;
	}
	printflag();
}

/*
 * reback ()
 *
 * Adjust the cell value.
 */
void
reback ()
{
	k = i - (turn==0)*1 - (turn==2)*(-1);
	l = j - (turn==1)*1 - (turn==3)*(-1);

	if(!((flag[i][j]&128)>>7)) {
		if (trflag == 0) {
			if (((k >= CELLSIZE/2) && (l >= CELLSIZE/2) && (a[k][l] >= (k+l+6-CELLSIZE))) ||
				((k >= CELLSIZE/2) && (l <= (CELLSIZE/2-1)) && (a[k][l] >= (k-l+5))) ||
				((k <= CELLSIZE/2-1) && (l <= (CELLSIZE/2-1)) && (a[k][l] >= (CELLSIZE-k-l+4))) ||
				((k <= CELLSIZE/2-1) && (l >= (CELLSIZE/2)) && (a[k][l] >= (-k+l+5))))
				a[k][l] -= 3;
		} else {
			if(a[k][l]>=(k+l+6))
				a[k][l] -= 3;
		}
	}
}

/*
 * modifymaze()
 * 
 * This function updates/modifies the values for flag and cell-value based on
 * the input.
 */
void
modifymaze ()
{
    
	switch(lcr)
	{          
		case 1 :
			flag[i][j] |= (1*(turn==2) + 2*(turn==1) + 4*(turn==0) + 8*(turn==3));
			break;
		case 2 :
			flag[i][j] |= (2*(turn==2) + 4*(turn==1) + 8*(turn==0) + 1*(turn==3));
			break;
		case 4:
			flag[i][j] |= (4*(turn==2) + 8*(turn==1) + 1*(turn==0) + 2*(turn==3)); 
			break;
		case 3 :
			flag[i][j] |= (3*(turn==2) + 6*(turn==1) + 12*(turn==0) + 9*(turn==3)); 
			break;
		case 5 :
			flag[i][j] |= (5*(turn==2) + 10*(turn==1) + 5*(turn==0) + 10*(turn==3));
			break;
		case 6 :
			flag[i][j] |= (6*(turn==2) + 12*(turn==1) + 9*(turn==0) + 3*(turn==3));
			break;
		case 7 :
			if(i!=0 && j!=0) 
				/* Trap!! Don't visit again. Mark all sides as blocked */
				flag[i][j] |= 15;
			else 
				flag[i][j] |= (7*(turn==2) + 14*(turn==1) + 13*(turn==0) + 11*(turn==3));
			/*
			 * For dead-end, increment cell-value by 12.
			 */
			a[i][j] += 12;
			break;
	}
  	/* Increment cell-value by 3 */ 
	a[i][j] += 3;
	reback();

	/* Indicate we have been here */
	flag[i][j] |= (1<<7);
}


/*
 * getturn (int modifier)
 *
 * Function that helps micromouse take the next step based on the current
 * position. The fn modifies the current cell-value and updates the location
 * (i, j). The update of the cell-value is part of the algorithm required for
 * micromouse to solve the maze.
 */
void
getturn (int modifier)
{
	switch(modifier)
	{
		case 0:
			if (PRESENT <= STRAIGHT)
				/*
				 * Modify current cell-value
				 */
				a[i][j] = 1 + STRAIGHT;

			if (trflag < 2)
				modifymaze();
			else
				flag[i][j] |= (1<<6);

			i += (turn==0)*1 + (turn==2)*(-1);
			j += (turn==1)*1 + (turn==3)*(-1);
			break;
		case 1:
			if (PRESENT <= RIGHT)
				a[i][j] = 1 + RIGHT;

			if (trflag < 2)
				modifymaze();
			else
				flag[i][j] |= (1<<6);

			j += (turn==0)*1 + (turn==2)*(-1);
			i += (turn==1)*(-1) + (turn==3)*1; 
			break;
		case 2:
			if (PRESENT <= REVERSE)
				a[i][j] = 1 + REVERSE;

			if (trflag < 2)
				modifymaze();
			else
				flag[i][j] |= (1<<6);

			i += (turn==0)*(-1) + (turn==2)*1;
			j += (turn==1)*(-1) + (turn==3)*1;
			break;
		case 3:
			if (PRESENT <= LEFT)
				a[i][j] = 1 + LEFT;

			if (trflag < 2)
				modifymaze();
			else
				flag[i][j] |= (1<<6);

			j += (turn==0)*(-1) + (turn==2)*1;
			i += (turn==1)*1 + (turn==3)*(-1);
			break;
	}
	/* Path-length has increased by 1 */
	plength++;

	/* Update direction of micromouse (turn) */
	count += modifier;
	turn = count % 4;
}


/*
 * getlcr()
 * Function to print the maze in user-understandable format and get the user
 * input for walls 
 */
void getlcr()
{
	printmaze();
	printf("\nEnter values of ir input ");
	scanf("%d",&lcr);
}


/*
 * trace()
 *
 * This function traces the path to the center of the maze either by 1. taking 
 * the maze-walls as inputs during the first run or 2. using the cell values in
 * the subsequent runs. The cell values used in the subsequent runs are updated
 * in the first run.
 */
unsigned char
trace ()
{
	plength = 0;
	
	while( a[i][j] != 0 ) {
		/*
		 * First get input 
		 */
		if( trflag < 2 ) {
                	getlcr();				
		} else {
			lcr = (turn==0) * (((flag[i][j]&12)>>2) + ((flag[i][j]&1)<<2)) +
			      (turn==1) * ((flag[i][j]&14)>>1) + (turn==2)*(flag[i][j]&7) +
			      (turn==3) * (((flag[i][j]&3)<<1) + ((flag[i][j]&8)>>3));
		}
	
		switch (lcr) {
			case 0:
				/* Left/Straight/Right are open */
				if ((STRAIGHT <= RIGHT) && (STRAIGHT <= LEFT)) {
					/* Go straight (preferred) */
					getturn(0);
				} else {
					if(RIGHT>LEFT)
						/* Turn Left */
						getturn(3);
					else    
						/* Turn Right */
						getturn(1);
				}
				break;
			case 1:
				/* Left/Straight are open. Right is blocked. */
				if (STRAIGHT <= LEFT) {
					/* Go straight (preferred) */
					getturn(0);
				} else {
					/* Turn Left */
					getturn(3);
				}
				break;
			case 4:
				/* Right/Straight are open. Left is blocked. */
				if (STRAIGHT <= RIGHT) {
					/*
					 * Go straight (preferred over right/left
					 * over same cell-value)
					 */
					getturn(0);
				} else {
					/* Turn Right */
					getturn(1);
				}
				break;
			case 5:
				/* Right/Left are blocked. Go straight */
				getturn(0);
				break;
			case 2:
				if (RIGHT <= LEFT) {
					/*
					 * Turn Right (preferred over left for
					 * same cell-value)
					 */
					getturn(1);
				} else {
					/* Turn Left */
					getturn(3);
				}
				break;
			case 3:
				/* Right/Straight are blocked. Turn left  */
				getturn(3);
				break;
			case 6:
				/* Left/Straight are blocked. Turn right  */
				getturn(1);
				break;
			case 7:
				/* Left/Straight/Right are blocked. Take a
				 * U-Turn!!
				 */
				getturn(2);
				break;
			default:
				printf("\nIncorrect option entered. Exiting.");
				exit(1);
		}
	}
	return (plength);
}

/*
 * swap()
 *
 * Swap 'a' and 'b' vectors. This is to save the cell-value data for the previous
 * run in 'b' which can be used to determine the shortest path.
 */
void
swap()
{
	unsigned char temp;
	for(k=0;k<=(CELLSIZE)-1;k++)
	{
		for(l=0; l<=(CELLSIZE)-1; l++)
		{
			temp=a[k][l];
			a[k][l]=b[k][l];
			b[k][l]=temp;
		}
	}
}


/*
 * virtualpath()
 *
 * Optimize cell values and flags to get the shortest possible path.
 * This is called only after the micromouse has been once to the center of
 * the maze and back (trflag == 2).
 */
void
virtualpath()
{

	swap();
	count += (4-turn);
	turn = count%4;
	plength1 = trace();
	printtrace();
	
	for(k=CELLSIZE-1; k>=0; k--)
		for(l=0; l<=CELLSIZE-1; l++)
			flag[k][l]&=191;
	
	swap();
	plength2=trace()-4;
	printtrace();	
	
	if(plength1<=plength2) {
		count+=(4-turn);
		turn=count%4;
	} else {
		for(k=CELLSIZE-1;k>=0;k--)
			for(l=0;l<=CELLSIZE-1;l++)
				if(flag[k][l]&64)
					 a[k][l]=100-a[k][l]; 
				else
					 a[k][l]=200;
		a[CELLSIZE/2][CELLSIZE/2-1] = a[CELLSIZE/2-1][CELLSIZE/2-1] =\
						a[CELLSIZE/2][CELLSIZE/2] =\
						a[CELLSIZE/2-1][CELLSIZE/2] = 0;
		swap();
		count+=2;
		turn=count%4;
	}
}

int main()
{   
	/*
	 * Initialze the variables
 	 */
	i=j=turn=count=0;

	for(k=0;k<=(CELLSIZE)-1;k++)
	{
		for(l=0;l<=(CELLSIZE)-1;l++)
		{
			flag[k][l] = 0;
			if(l==0)
				flag[k][l]|=1;
			if(k==0)
				flag[k][l]|=2;
			if(l==CELLSIZE-1)
				flag[k][l]|=4;
			if(k==CELLSIZE-1)
				flag[k][l]|=8;
		 }
	}

	/*
	 * In action now!
	 */	
	do
	{
		if (trflag==2)
			virtualpath();
		if((trflag%2)==0) {
			swap();
			if (trflag==0)
			{
				for(k=0;k<=(CELLSIZE/2)-1;k++)
					for(l=0;l<=(CELLSIZE/2)-1;l++)
						 a[CELLSIZE/2+k][CELLSIZE/2-1-l] =\
							a[CELLSIZE/2-1-k][CELLSIZE/2-1-l] =\
							a[CELLSIZE/2+k][CELLSIZE/2+l] =\
							a[CELLSIZE/2-1-k][CELLSIZE/2+l] = k+l;;
			}
		} else {
			swap();
			if (trflag==1) {
				/*
				 * The micromouse is at the centre of the maze. Let it traverse back to
				 * starting point (0,0). Updating cell-values accordingly.
				 */
				for(k=0; k <= (CELLSIZE)-1; k++)
					for(l=0; l <= (CELLSIZE)-1; l++)
						a[k][l]=k+l;;
			}     
		}
		trace();
		filtermaze();
		printmaze();
		trflag++;
		printchoice();
	} while(choice=='y');
	return 0;
}

