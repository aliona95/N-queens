#include <stdio.h>
#include <time.h>
#include <mpi.h>
#define true 1
#define false 0

int placeQueens(int column, int positions[], int n) {
    int varNum = 0;
    int row;
   
    for (row = 1; row <= n; row++){ //	vieno stulpelio visi langeliai
        if (correct(row, column, positions)){
            
            if (column < n){
                positions[column] = row;  // positions[column] priskiriamas tame stulpelyje uzimtos eilutes nr
		column++;
                varNum += placeQueens(column, positions, n);
                column--;
                positions[column] = 0;  // 
            }
            if (column == n){  // apeita visa lenta
                varNum++;    
            }
        }       
    }
    return varNum;
}

int correct(int row, int column, int positions[]){
    int previousColumn = column - 1;
    
    for (; previousColumn > 0; previousColumn--){
        if (positions[previousColumn] -  row == 0){ // toje pacioje eiluteje
            return false;
        }
        int rowSub = row-positions[previousColumn];  
        // tikriname istrizaines
        if (rowSub == column - previousColumn || rowSub == previousColumn - column){
            return false;
	}
    }
    return true;
}

void master(int n, int visoProc){
    MPI_Status status;
    int time1, time2;
    
    int result = 0;
    int tempRes;

    int doneRow[2]; 
    int positions[n + 1];
    
    int i;
    int j;
    time1 = clock();


    for (i = 1; i <= n; i++){      //  paskirstomi darbai

        positions[1] = i;
        for (j = 1; j <= n; j++){
		if (correct(j, 2, positions)){
                    positions[2] = j;
                    MPI_Recv(&tempRes, 1, MPI_INT, MPI_ANY_SOURCE, 201, MPI_COMM_WORLD, &status);
                    result += tempRes;

                    doneRow[0] = i;
                    doneRow[1] = j;
                    
                    //printf("POSSS111: %d   %d\n", doneRow[0], doneRow[1]);


        
                    MPI_Send(&(doneRow), 2, MPI_INT, status.MPI_SOURCE, 201, MPI_COMM_WORLD);    
                }    
         }
    }
// cia reikia keist???
    int endOfWork = -1;
    for (i = 1; i < visoProc; i++) { //  nurodome, kad baigesi darbai     
        MPI_Recv(&tempRes, 1, MPI_INT, MPI_ANY_SOURCE, 201, MPI_COMM_WORLD, &status);
        result += tempRes;
        MPI_Send(&endOfWork, 1, MPI_INT, status.MPI_SOURCE, 201, MPI_COMM_WORLD);
    }
	
    time2 = clock();
    printf("Result: %d\n", result);
    
    printf("Time: %f ms\n", ((float)time2 / CLOCKS_PER_SEC));
}

void slave(int n){
    MPI_Status status;

    double siaip[1];    

    int i = 0;
    int tempRes = 0 ;
    //int doneRow = 0;  // sekancio galimo (teisingo) eilutes stulpelio sk  
    int doneRow[2]; 
    int positions[n + 1];

      
    doneRow[0] = 0;
  
    while (doneRow[0] >= 0){    // kol ne -1, endOfWork
        MPI_Send(&tempRes, 1, MPI_INT, 0, 201, MPI_COMM_WORLD);
        tempRes = 0;

        MPI_Recv(&(doneRow), 2, MPI_INT, 0, 201, MPI_COMM_WORLD, &status);
        

       /* if (doneRow[0] > 0){      
            positions[1] = doneRow[0];
	    //for(i = 1; i <= n; i++){
		//if (correct(i, 2, positions)){
                    positions[2] = i; // buvo doneRow[1]???
		    //printf("POS: %d \n", i);
                    //tempRes += placeQueens(3, positions, n); // 2 - nuo antro stulpelio pradedame tikrinima

               // }
	   // }
            tempRes = placeQueens(3, positions, n); // 2 - nuo antro stulpelio pradedame tikrinima
        }*/


        if (doneRow[0] > 0){      
            positions[1] = doneRow[0];
	    positions[2] = doneRow[1];
            //printf("POSSS: %d   %d\n", doneRow[0], doneRow[1]);

            tempRes = placeQueens(3, positions, n); // 3 - nuo trecio stulpelio pradedame tikrinima
        }

    }
      
}

int main(int argc, char * argv[]){
    int n = atoi(argv[1]); // n - lentos n x n dydis    
    int procId;
    int procNum;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &procId);
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    
    if (procId == 0){
		master(n, procNum);    
    }
    if (procId != 0){  // procId == 0 => master
        slave(n); 
    }

    MPI_Finalize();
    return 0;
}
// 5.758801 16x16 220
// 11.792618 16x16 100
// 21.048662 16x16 50
