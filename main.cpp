
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <set>
#include <semaphore.h>
#include "omp.h"
#include "graph.h"


int main(void) {

    int my_rank, comm_sz;
    MPI_Comm comm;
    sem_t sem;
    sem_init(&sem, 0, 1);

    MPI_Init(NULL, NULL);
    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &comm_sz);
    MPI_Comm_rank(comm, &my_rank);
    MPI_Status status;

    int numVertices = 1000;
    int numEdegs = 1000;
    vector<int> minlength(numVertices, 999999);      //保存最短距離
    vector<vector<char>> adjList;                                          //保存圖

    if(my_rank==0){
        auto graph = getrandomgraph(numVertices, numEdegs);
        adjList = graph.adjList;

        for(int i=0; i<numVertices; i++){
            MPI_Send(&adjList[i][0], numVertices, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
        }
    }
    else{
        adjList.resize(numVertices);
        for(int i=0; i<numVertices; i++){
            adjList[i].resize(numVertices,'0');
        }
        for(int i=0; i<numVertices; i++){
            MPI_Recv(&adjList[i][0], numVertices, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);
        }
    }

    minlength[0] = 0;
    vector<int> visit = {0};
    vector<int> next_visit;
    for(int h=1; h<100; h++){
        if(visit.size()==0) break;

        int t= (visit.size()+1)/2;
        int local_a = t*my_rank;
        int local_b = local_a+t <= visit.size() ? local_a+t : visit.size();
        //printf("myrank=%d       %d   %d\n",my_rank,local_a,local_b);


        #pragma omp parallel num_threads(8)
        for(int i=local_a; i<local_b; i++){
            //printf("thread %d i=%d\n",omp_get_thread_num(),i);
            int p = visit[i];
            for(int j=0; j<numVertices; j++){
                if((adjList[p][j]=='1')&&(minlength[j]==999999)){
                    minlength[j] = h;
                    sem_wait(&sem);
                    next_visit.push_back(j);
                    sem_post(&sem);
                }
            }
        }
        /*
        printf("h = %d    my_rank = %d            next   ",h,my_rank);
        for(int k=0;k<next_visit.size();k++){
            printf("%d  ",next_visit[k]);
        }
            printf("\n");


        printf("h = %d    my_rank = %d \n",h,my_rank);
*/
        vector<int> temp;
        if(my_rank==0){
            int size = next_visit.size();
            MPI_Send(&size, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            MPI_Send(&next_visit[0], size, MPI_INT, 1, 0, MPI_COMM_WORLD);

            MPI_Recv(&size, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
            //printf("h = %d    my_rank = %d  size=%d  \n",h,my_rank,size);
            temp.resize(size);
            MPI_Recv(&temp[0], size, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
        }
        else if(my_rank==1){
            int size;
            MPI_Recv(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            temp.resize(size,0);
            MPI_Recv(&temp[0], size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

            size = next_visit.size();
            MPI_Send(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Send(&next_visit[0], size, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }

        for(int p=0; p<temp.size(); p++){
            if(minlength[temp[p]]==999999) minlength[temp[p]]=h;
        }

        visit.clear();
        set<int> s(next_visit.begin(),next_visit.end());
        s.insert(temp.begin(),temp.end());
        visit.insert(visit.end(),s.begin(),s.end());

        next_visit.clear();
    }
    if(my_rank==0){
        printf("result :    ");
        for(int k=0;k<minlength.size();k++){
            printf("%d  ",minlength[k]);
        }
    }
    MPI_Finalize();
    return 0;
}