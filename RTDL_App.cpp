/*
non_periodic
*/
 

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>  // Gaussian Blur
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

//Apre la camera di default
VideoCapture VideoStream("/home/giulia/Scrivania/Pratica/videoprova.mp4");
//VideoCapture VideoStream(0);
const string WindowName = "Streaming video ";


/*  ____________________INIZIO Codice prof. Aldo Franco Dragoni per la periodicità del thread ____________________*/

// confronta le due variabili temporali t1 e t2
// restituisce 0  se t1=t2
// restituisce +1 se t1>t2
// restituisce -1 se t1<t2
int time_cmp(struct timespec t1, struct timespec t2) {
	if (t1.tv_sec > t2.tv_sec) return 1;
	if (t1.tv_sec < t2.tv_sec) return -1;
	if (t1.tv_nsec > t2.tv_nsec) return 1;
	if (t1.tv_nsec < t2.tv_nsec) return -1;
	return 0;
}

// copia la variabile temporale ts in quella puntata da td
void time_copy(struct timespec *td, struct timespec ts) {
	td->tv_sec = ts.tv_sec;
	td->tv_nsec = ts.tv_nsec;
}

//aggiunge ms millisecondi alla variabile temporale puntata da t
void time_add_ms(struct timespec *t, int ms) {
	t->tv_sec += ms/1000; 
	t->tv_nsec += (ms%1000) * 1000000;
	if( t->tv_nsec > 1000000000) {
		t->tv_nsec -= 1000000000;
		t->tv_sec += 1;
	}
}
/* ____________________FINE codice prof. Aldo Franco Dragoni ____________________*/



		
void *produttore(void *arg) {
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	time_add_ms(&t, 1000);
	Mat frame;
	
	while(1) {
		
			Mat image;
			VideoStream >> frame;
			//cvtColor(frame, image, COLOR_RGB2GRAY);			
			if (frame.empty()) break;
                
                frame.copyTo(image);
                
                pthread_mutex_lock(&mtx);
                cb.push_back(image);
                if (!image.empty()) 
                cout << "produttore \n\n" << endl; 
                pthread_mutex_unlock(&mtx);  
                
    
			}
			
			clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
			time_add_ms(&t, 1000);
	}





void *consumatore(void *arg) {
	struct timespec t;
	int i = *(int*)arg; //inserito per stampare in output l'indice del lettore come verifica
	/*std::string out_string;
	std::stringstream ss;
	ss << i;
	i_to_string = ss.str(); */
	//const string WindowName = "Streaming video "+i;
	clock_gettime(CLOCK_MONOTONIC, &t);
	time_add_ms(&t, 1000);   

	
	while(1) {
		
        Mat frame; 
                   
        pthread_mutex_lock(&mtx); /* ____________________Start Critical Section____________________ */ 
        frame = cb.back(); //Recupera un puntatore all'ultimo elemento inserito nel CAB - questo puntatore è aggiornato dalla push_back() nella void* produttore()                 

		//__________Creazione Oggetto per la scrittura del file e inizializzazione degli attributi__________ */
		/*VideoWriter video;
		int codec = VideoWriter::fourcc('F','M','P','4');
		double fps = 25.0;
		string filename = "./live.mov"; 
		bool isColor = (frame.type() == CV_8UC3);
        video.open(filename , codec, fps, frame.size(), isColor); 
        if(!video.isOpened()){
			cout << "Non è stato possibile aprire il il file per la scrittura del video \n";
			}*/
        
        pthread_mutex_unlock(&mtx); /* ____________________End Critical Section____________________ */
        
	        if (!frame.empty()) {
			
			switch(i) {

			//id2 thread entra in questo ramo e stampa a video finché qualcuno non preme q
			case 2: {
				imshow(WindowName, frame);
				// Press  ESC on keyboard to exit
				char c=(char)waitKey(33); 
				if(c==27)
				break; 
				 }
				 
				 
			//id2 thread entra in questo ramo e stampa su file
			case 3: { 
				//video.write(frame); break;
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
	int id1=1; 			//ID produttore
	int id2=2, id3=3; 	//IDs consumatori
	timespec dt; dt.tv_sec = 2;	dt.tv_nsec = 0; 
	assert (cb.capacity() == 200);
	

	pthread_create(&tid1, NULL, produttore, NULL);
	clock_nanosleep(CLOCK_MONOTONIC, 0, &dt, NULL); // sospende per dt. Operazione necessaria altrimenti lo scrittore non fa in tempo 
													//a produrre il frame e il lettore trova il buffer vuoto, andando in errore
	pthread_create(&tid2, NULL, consumatore, &id2);
	clock_nanosleep(CLOCK_MONOTONIC, 0, &dt, NULL); // sospende per dt. Operazione necessaria altrimenti lo scrittore non fa in tempo 
													//a produrre il frame e il lettore trova il buffer vuoto, andando in errore
	pthread_create(&tid2, NULL, consumatore, &id3);
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	pthread_join(tid3, NULL);
	pthread_exit(NULL);
}


