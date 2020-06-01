/* 
Autores: Thomas Palmeira Ferraz e Carolina Teng
Data: 15/Maio/2018
Para a disciplina PSI-3471 Fundamentos de Sistemas Eletrônicos Inteligentes
*/

#include <cekeikon.h>
#include <string>
#include <sstream>

using namespace cv;
using namespace std;


/* Esta funcao remove os pixels distantes da cor vermelha,
  singulariza o tom de vermelho, trata os ruidos da imagem
  resultante e borra a imagem. */
Mat RedOnly(Mat entrada, int B, int G, int R, int dist){

	// imagem com pixels vermelhos
	Mat_<COR> red_only;
	red_only = entrada.clone();

	// definicao escolhida para a cor vermelha
	COR vermelho(B, G, R);
	
	// mantem pixels vermelhos e singulariza o tom de vermelho
	for (int l=0; l<entrada.rows; l++)
		for(int c=0; c<entrada.cols; c++){
			if(distancia(vermelho,red_only(l,c))>dist)
			//(red_only(l,c)[2]>50.0 && ((red_only(l,c)[1]+red_only(l,c)[0])<70.0))
				red_only(l,c)=COR(255,255,255);
			else
				red_only(l,c)= vermelho;
		}
	
	// elimina ruidos de um pixel
	for (int l=0; l<entrada.rows; l++)
		for(int c=0; c<entrada.cols; c++)
			if(red_only(l,c-1)==COR(255,255,255)&& red_only(l,c+1)==COR(255,255,255))
				red_only(l,c) = COR(255,255,255);
	
	// borra a imagem para reduzir ruidos
	double gaussDev = 2.5;
	GaussianBlur(red_only,red_only,Size(0,0),gaussDev,gaussDev);
	
	return red_only;
}

/* O programa deve receber o nome do arquivo a ser
 processado e o nome do arquivo a ser gerado nesta
 ordem. */
int main(int argc, char** argv){

	//verificacao de parametros
	if (argc != 3){
		erro (" ep1: Detecta placa \"proibido virar\" \n sintaxe: ep1 ent.ext sai.ext \n Erro: Numero de argumentos invalido\n");
		return 0;
	}

	String arquivo_entrada = argv[1];
	String arquivo_saida = argv[2];
	
	// le imagem original
	Mat_<COR> original;
	le(original, arquivo_entrada);
		
	// separa apenas pixels vermelhos
	Mat_<COR> red_only;
	red_only = RedOnly(original,35,35,165,80);
		
	// converte para gray scale
	Mat_<GRY> gray(original.size());
	for (unsigned i=0; i<original.total(); i++)
		gray(i) = round(0.299*red_only(i)[2] + 0.587*red_only(i)[1] + 0.114*red_only(i)[0]);
	
	// detecta circulos pelo Metodo de Hough
	vector<Vec3f> circles;
	HoughCircles(gray,circles,CV_HOUGH_GRADIENT,2,original.cols,100,120,0,original.rows/2);
	
	if(circles.size()!=1){
	
		// converte de BGR para YCrCb
		Mat_<COR> YCrCb;
		cvtColor(original,YCrCb,CV_BGR2YCrCb);
		
		// separa os canais Y, Cr e Cb
		vector<Mat> channels;
		split(YCrCb, channels);
		
		//Aplica-se a equaliza��o no primeiro canal Y (intensidade luminosa)
		equalizeHist(channels[0], channels[0]);
		merge(channels, YCrCb);
		Mat equal;
		cvtColor(YCrCb, equal, CV_YCrCb2BGR);
		
		// separa apenas pixels vermelhhos
		red_only = RedOnly(equal,0,0,225,180);//180
		
		// converte para gray scale
		Mat_<GRY> gray(original.size());
		for (unsigned i=0; i<original.total(); i++)
			gray(i) = round(0.299*red_only(i)[2] + 0.587*red_only(i)[1] + 0.114*red_only(i)[0]);
		
		// detecta circulos pelo Metodo de Hough
		HoughCircles(gray,circles,CV_HOUGH_GRADIENT,2,original.cols,200,100,0,original.rows/2);
		
	}
		
	// desenha o centro e a circunferencia dos circulos detectados
	for (unsigned i = 0; i<circles.size(); i++) {
		Point center(round(circles[i][0]), round(circles[i][1]));
		int radius = round(circles[i][2]);
		circle( original, center, 5, Scalar(0,255,255), -1, 8, 0 ); 
		circle( original, center, radius, Scalar(0,255,255), 3, 8, 0 );
	}
	
	// gera um novo arquivo com indicacao para a placa
	imp(original, arquivo_saida);
	
	return 0;
	
}
