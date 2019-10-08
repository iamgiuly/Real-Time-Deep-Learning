/* CPP API to Darknet Framework */ 

/* extern "C" {
#include "/home/giulia/darknet/include/darknet.h"} */

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


//Definisce e inizializza la variabile mutex
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

//Crea un buffer circolare di Mat Object della capacità di 200 elementi
boost::circular_buffer<Mat> cb(200);


VideoCapture VideoStream("/home/giulia/Scrivania/SODTR/video2.mp4");
//VideoCapture VideoStream(0); //Webcam

const string WindowName2 = "Streaming video - 1 ";
//const string WindowName3 = "Streaming video - 2 ";
int periodo = 33;


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
	time_add_ms(&t, periodo);


	while(1) {

		Mat frame;
		VideoStream >> frame;	

		if (frame.empty()) break;

                pthread_mutex_lock(&mtx);
                cb.push_back(frame);
                cout << "produttore \n\n" << endl; 
                pthread_mutex_unlock(&mtx);

  
	}

			
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		time_add_ms(&t, periodo);
}




void *consumatore(void *arg) {


	struct timespec t;
	int i = *(int*)arg; //inserito per stampare in output l'indice del lettore come verifica
	clock_gettime(CLOCK_MONOTONIC, &t);
	time_add_ms(&t, periodo); 
  
	
	while(1) {
	
        	Mat frame;       
        	pthread_mutex_lock(&mtx); 
        	frame = cb.back(); 					//assegna al riferimento dell'oggetto Mat il puntatore del mrb               
        	pthread_mutex_unlock(&mtx); 
        

	        if (!frame.empty()) {		
			switch(i) {
		
			case 2: {

				imshow(WindowName2, frame); 	//deve essere seguita dalla funzione waitKey
				char c=(char)waitKey(33); 	//indica i ms da attendere prima di mostrare un nuovo frame
				if(c==27)
				break;}
		 
		
			case 3: { 
				
				int test = frame.type(); 	//Identificatore del tipo di elemento salvato nella matrice Mat, compatibile con il sistema di tipo "CvMat"
				cout << "Ho letto il frame con tipo = " << test << endl; 
				break;

				/*imshow(WindowName3, frame); //deve essere seguita dalla funzione waitKey
				char c=(char)waitKey(33); //indica i ms da attendere prima di mostrare un nuovo frame
				if(c==27)
				break; */}		
			}
		}	

		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		time_add_ms(&t, periodo);

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


