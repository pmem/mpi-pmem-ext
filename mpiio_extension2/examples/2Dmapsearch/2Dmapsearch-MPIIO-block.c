/*
Copyright 2014-2016, Gdansk University of Technology

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#include "file_io_pmem.h"
#include "file_io_pmem_wrappers.h"

#define CELL_VAL_LOW_THRESHOLD 5
#define CELL_VAL_HIGH_THRESHOLD 10

#define SURROUNDING_RADIUS 4
#define HOW_MANY_TO_SEARCH_FOR_IN_SURROUNDING 4

// this version allows reading from a file using a block

// parameters for a buffer for block reading
#define BLOCK_SIZE 512
char block_buffer[BLOCK_SIZE];
long long buffer_start_offset=0;
long long buffer_end_offset=0;
int block_read=0; // 1 if a block was read at least once

//int read_counter=0;

char get_xy_cell(long long x,long long y,MPI_File file,long long mapxsize,long long mapysize) { // get the value of a cell from a file

  char temp;
  MPI_Status status;

  MPI_Offset offset=y*mapxsize+x; // this is the location in the file to read from

  // first check if the cell is in the buffer
  if ((!block_read) || (offset<buffer_start_offset) || (offset>buffer_end_offset)) { // need to read a block

    MPI_Offset last_map_offset=mapysize*mapxsize-1;
    int charstoread=(((offset+BLOCK_SIZE)>last_map_offset)?(last_map_offset-offset+1):BLOCK_SIZE);

    buffer_start_offset=offset;
    buffer_end_offset=buffer_start_offset+charstoread-1;

    //        printf("%d x=%d y=%d Reading %d bytes starting at offset %d start=%d end=%d\n",read_counter,(int)x,(int)y,charstoread,(int)offset,(int)buffer_start_offset,(int)buffer_end_offset);
    //        read_counter++;

    MPI_File_read_at(file,offset,&block_buffer,charstoread,MPI_CHAR,&status);

    block_read=1;
  }// else printf("\nin buffer");

  // now simply return the cell value from the buffer
 
  return block_buffer[offset-buffer_start_offset];
}

void set_xy_cell(long long x,long long y,MPI_File file,long long mapxsize,long long mapysize,char val) {

  MPI_Status status;
  
  MPI_Offset offset=y*mapxsize+x; // this is the location in the file to read from
  
  MPI_File_write_at(file,offset,&val,1,MPI_CHAR,&status);

}


double eval_surrounding(long long x,long long y,MPI_File file,long long mapxsize,long long mapysize) { // evaluate if there are at least a
  // HOW_MANY_TO_SEARCH_FOR_IN_SURROUNDING number of similar objects in the surrounding starting from the center

  long long minx,maxx,miny,maxy;
  double eval=0;
  double temp_eval;
  long long i,j;
  int numberofitemsfound=0;


  double center_eval=get_xy_cell(x,y,file,mapxsize,mapysize);

  // for full surrounding search uncomment the following and comment the current algorithm
  /*
  for(i=1;i<SURROUNDING_RADIUS;i++) {

    if (((x-i)<mapxsize) && ((x-i)>=0) && (y>=0) && (y<mapysize)) {
      temp_eval=get_xy_cell(x-i,y,file,mapxsize,mapysize);
      if (fabs(temp_eval-center_eval)<=10)
	numberofitemsfound++;
      
    }
    if (numberofitemsfound>=HOW_MANY_TO_SEARCH_FOR_IN_SURROUNDING) break;

    for(j=-i+1;j<i;j++) {
      if (((x+j)<mapxsize) && ((x+j)>=0) && ((y+i-llabs(j))>=0) && ((y+i-llabs(j))<mapysize)) {
	temp_eval=get_xy_cell(x+j,y+i-llabs(j),file,mapxsize,mapysize);
	if (fabs(temp_eval-center_eval)<=10)
	  numberofitemsfound++;
	
      }
      if (numberofitemsfound>=HOW_MANY_TO_SEARCH_FOR_IN_SURROUNDING) break;

      if (((x+j)<mapxsize) && ((x+j)>=0) && ((y-i+llabs(j))>=0) && ((y-i+llabs(j))<mapysize)) {
	temp_eval=get_xy_cell(x+j,y-i+llabs(j),file,mapxsize,mapysize);
	if (fabs(temp_eval-center_eval)<=10)
	  numberofitemsfound++;
	
      }
      if (numberofitemsfound>=HOW_MANY_TO_SEARCH_FOR_IN_SURROUNDING) break;
    }
    if (numberofitemsfound>=HOW_MANY_TO_SEARCH_FOR_IN_SURROUNDING) break;

    if (((x+i)<mapxsize) && ((x+i)>=0) && (y>=0) && (y<mapysize)) {
      temp_eval=get_xy_cell(x+i,y,file,mapxsize,mapysize);
      if (fabs(temp_eval-center_eval)<=10)
	numberofitemsfound++;
      
    }
    if (numberofitemsfound>=HOW_MANY_TO_SEARCH_FOR_IN_SURROUNDING) break;
    
  }
  */

  
  for(i=1;i<SURROUNDING_RADIUS;i++) {
    for(j=-i+1;j<i;j++) {
      if (((x+j)<mapxsize) && ((x+j)>=0) && ((y+i-j)>=0) && ((y+i-j)<mapysize)) {
	temp_eval=get_xy_cell(x+j,y+i-j,file,mapxsize,mapysize);
	if (fabs(temp_eval-center_eval)<=10)
	  numberofitemsfound++;
	
      }
      if (numberofitemsfound>=HOW_MANY_TO_SEARCH_FOR_IN_SURROUNDING) break;
    }
    if (numberofitemsfound>=HOW_MANY_TO_SEARCH_FOR_IN_SURROUNDING) break;
  }
  

  return numberofitemsfound;

  /*
  minx=x-SURROUNDING_WIDTH;
  if (minx<0) minx=0;
  maxx=x+SURROUNDING_WIDTH;
  if (maxx>=mapxsize) maxx=mapxsize-1;

  miny=y-SURROUNDING_WIDTH;
  if (miny<0) miny=0;
  maxy=y+SURROUNDING_WIDTH;
  if (maxy>=mapysize) maxy=mapysize-1;

  for(i=minx;i<=maxx;i++)
    for(j=miny;j<=maxy;j++) {
      temp_eval=get_xy_cell(i,j,file,mapxsize,mapysize);

      eval+=sqrt((center_eval-temp_eval)*(center_eval-temp_eval));
      
    }
  */


}

int main(int argc,char **argv) {
  
  MPI_File file;
  long long mapxsize,mapysize;
  
  long long myxmin,myxmax,myymin,myymax;
  int processes_in_x_dim,processes_in_y_dim;
  int my_proc_id_in_x_dim,my_proc_id_in_y_dim;

  long long boxxsize,boxysize; // sizes of a map fragment handled by each process

  int myrank,proccount;

  MPI_Offset filesize;

  long long x,y; // counters to go through a map fragment

  double max_similarity,my_similarity,my_temp_similarity;

  double cell_val;

  int provided_thread_support;
  MPI_Init_thread(&argc,&argv,MPI_THREAD_MULTIPLE, &provided_thread_support);
  
  // first read the file name from command line
  
  if (argc<7) {
    printf("\nSyntax: 2Dmapsearch-MPIIO <map_filename> mapxsize mapysize processes_in_x_dim processes_in_y_dim pmem_path\n");
    MPI_Finalize();
    exit(-1);
  }
  
  mapxsize=atol(argv[2]);
  mapysize=atol(argv[3]);
  
  processes_in_x_dim=atoi(argv[4]);
  processes_in_y_dim=atoi(argv[5]);

  if (mapxsize*mapysize<=0) {
    printf("\nWrong map size given.\n");
    MPI_Finalize();
    exit(-1);
  }

  // find out my rank and the number of processes 

  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&proccount);

  // now check if the number of processes matches the specified processes in dims
  if (proccount!=(processes_in_x_dim*processes_in_y_dim)) {
    printf("\nThe number of processes started does not match processes_in_x_dim*processes_in_y_dim.\n");
    MPI_Finalize();
    exit(-1);
  }

  MPI_Info info;
  MPI_Info_create(&info);
  MPI_Info_set(info,"pmem_path",argv[6]);
  MPI_Info_set(info,"pmem_io_mode","0");
  MPI_File_open(MPI_COMM_WORLD,argv[1],MPI_MODE_RDWR,info,&file) ;
  
  // now check the size of the file vs the given map size
  MPI_File_get_size(file,&filesize);
  if (filesize<mapxsize*mapysize) {
    printf("\nFile too small for the specified map size.\n");
    MPI_File_close(&file);
    MPI_Finalize();
    exit(-1);
  }


  // now each process should determine its bounding box for the map

  // length of each box will be (mapxsize/processes_in_x_dim) and similarly for the y dimension

  boxxsize=(mapxsize/processes_in_x_dim);
  boxysize=(mapysize/processes_in_y_dim);

  my_proc_id_in_x_dim=myrank%processes_in_x_dim;
  my_proc_id_in_y_dim=myrank/processes_in_x_dim;

  myxmin=my_proc_id_in_x_dim*boxxsize;
  myymin=my_proc_id_in_y_dim*boxysize;
  myxmax=myxmin+boxxsize;
  myymax=myymin+boxysize;

  // now each process should scan its fragment

  // if a certain element is detected then the application scans its immediate surroundings for elements of some other types

  my_similarity=0;

  for(y=myymin;y<myymax;y++) 
    for(x=myxmin;x<myxmax;x++) {

      
      cell_val=get_xy_cell(x,y,file,mapxsize,mapysize);
      if ((cell_val>=CELL_VAL_LOW_THRESHOLD) && (cell_val<=CELL_VAL_HIGH_THRESHOLD)) {
	my_temp_similarity=eval_surrounding(x,y,file,mapxsize,mapysize);
	//set_xy_cell(x,y,file,mapxsize,mapysize,0); // set a value to 0 to mark the location
      }      

      if (my_temp_similarity>=my_similarity)
	my_similarity=my_temp_similarity;
      
    }


  // now all processes should select the highest similarity

  MPI_Reduce(&my_similarity,&max_similarity,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

  if (!myrank) {
    printf("\nThe final similarity is %f\n",max_similarity);

  }


 MPI_File_close(&file);

 MPI_Finalize();

}
