/*extern "C" {
#include "/home/giulia/darknet/include/darknet.h"

}*/

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat, Scalar)
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>  // OpenCV window I/O
#include <boost/circular_buffer.hpp>
#include <pthread.h>
#include <iostream>
#include <time.h> // Per le funzioni e le strutture temporali

using namespace cv;
using namespace std;


//Definisce e inizializza la variabile mutex (semaforo binario)
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

//Crea un buffer circolare di Mat Object della capacità di 2000 elementi
boost::circular_buffer<Mat> cb(200);


VideoCapture VideoStream("/media/video2.mp4");
//VideoCapture VideoStream(0);
const string WindowName = "Streaming video ";



//aggiunge ms millisecondi alla variabile temporale puntata da t
void time_add_ms(struct timespec *t, int ms) {
	t->tv_sec += ms/1000; 
	t->tv_nsec += (ms%1000) * 1000000;
	if( t->tv_nsec > 1000000000) {
		t->tv_nsec -= 1000000000;
		t->tv_sec += 1;
	}
}


		
void *produttore(void *arg) {

	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	time_add_ms(&t, 1000);
	
	
	while(1) {
		
		
		Mat frame;
		VideoStream >> frame;
		
		if (frame.empty()) break;
                
                pthread_mutex_lock(&mtx);
                cb.push_back(frame);
                if (!frame.empty()) 
                cout << "produttore \n\n" << endl; 
                pthread_mutex_unlock(&mtx);  
                
    
			}
			
			clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
			time_add_ms(&t, 1000);
	}




void *consumatore(void *arg) {
	struct timespec t;
	int i = *(int*)arg; //inserito per stampare in output l'indice del lettore come verifica

	clock_gettime(CLOCK_MONOTONIC, &t);
	time_add_ms(&t, 1000);   

	
	while(1) {
		
        Mat frame; 
                   
        pthread_mutex_lock(&mtx); 
        frame = cb.back(); //Recupera un puntatore all'ultimo elemento inserito, aggiornato con la primitiva push_back()                 
        pthread_mutex_unlock(&mtx); 
        
	        if (!frame.empty()) {
			
			switch(i) {
		
			case 2: {

				imshow(WindowName, frame);
				char c=(char)waitKey(33); 
				if(c==27)
				break; 
				 }		 
		
			case 3: { 
				
				int test = frame.type();
				cout << "Sto stampando il frame con tipo pari a " << test; break;
						
				}		
			}}	
			clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
			time_add_ms(&t, 1000);
			}
}


int main() {

	pthread_t  tid1, tid2, tid3;
	int id1=1; 
	int id2=2, id3=3; 
	timespec dt; dt.tv_sec = 2; dt.tv_nsec = 0; 
	assert (cb.capacity() == 200);
	

	pthread_create(&tid1, NULL, produttore, NULL);
	clock_nanosleep(CLOCK_MONOTONIC, 0, &dt, NULL); 
	pthread_create(&tid2, NULL, consumatore, &id2);
	clock_nanosleep(CLOCK_MONOTONIC, 0, &dt, NULL);
	pthread_create(&tid3, NULL, consumatore, &id3);
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	pthread_join(tid3, NULL);
	pthread_exit(NULL);

}
