#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>

// Recebe uma imagem de placa e devolve uma nova imagem binaria preprocessada,
// mais facilde segmentar
cv::Mat preprocess(cv::Mat src);

// Recebe uma imagem preprocessada e retorna um vetor de Retangulos correspondentes
// a cada digito. Os digitos estao ordenados por coordenada x
std::vector<cv::Rect> segment(cv::Mat preproc);

// Recebe um vetor de Retangulos e uma imagem e retorna os um vetor de Mat
// correspondente as submatrizes da imagem nas posicoes dos retangulos
std::vector<cv::Mat> get_digit_vector(std::vector<cv::Rect> & chars, cv::Mat preproc);

// Recebe um vetor de Retangulos, uma imagem e uma string e salva as imagens
// correspondentes as submatrizes da imagem nas posicoes dos retangulos
// os aqrquivos sao jpg, tem prefixo igual a string passada e sao numerados de 0 a 7
void write_digits_to_file(std::vector<cv::Rect> & chars, cv::Mat preproc, char name[]);

// Mostra na tela o resultado da segmentacao
void show_result(cv::Mat orig, std::vector<cv::Rect> digits);

