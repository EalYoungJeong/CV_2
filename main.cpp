#include<iostream>
#include<fstream>
#include<stdlib.h>
#include<time.h>
#include<iomanip>
#include<math.h>
#include<cmath>
#pragma warning(disable:4996)
#define _USE_MATH_DEFINES
using namespace std;
#define ROW 512	//세로길이
#define COL 512 //가로길이
#define SIZE 512*512 //전체 이미지 크기
#define PI 3.141592 //원주율 파이
void Additive_Gaussian(unsigned char* src, unsigned char* dest, int sigma)//Additive Gaussian 노이즈 추가함수
{
	srand(time(NULL));//time으로 rand 함수 seed
	double sum = 0;
	for (int i = 0; i < SIZE; i++)
	{
		double v1, v2, s, rn_gaussian;
		do
		{
			v1 = 2 * ((double)rand() / RAND_MAX) - 1;
			v2 = 2 * ((double)rand() / RAND_MAX) - 1;
			s = v1 * v1 + v2 * v2;
		} while (s >= 1 || s == 0);

		s = sqrt((-2 * log(s)) / s);
		rn_gaussian = sigma * v1 * s;
		double temp = src[i] + rn_gaussian;//원본 이미지 pixel 에 가우시안 노이즈 추가

		//0~255 level에서 음수나 256이상의 오버플로우 방지
		if (temp < 0) dest[i] = 0;
		else if (temp > 255) dest[i] = 255;
		else dest[i] = temp;

		sum = sum + rn_gaussian;
	}
}
void SaltandPepper(unsigned char* src, unsigned char* dest, int ratio)//Salt and Pepper 노이즈 추가 함수
{
	srand(time(NULL));
	int n = SIZE * ratio / 100;//비율만큼 소금후추 뿌림
	//이미지 배열 복사
	for (int i = 0; i < SIZE; i++)
		dest[i] = src[i];

	for (int i = 0; i < n; i++)
	{
		//random position
		int x = rand() % 512;
		int y = rand() % 512;

		int a = rand() % 2;
		if (a)//salt
			dest[(512 * y) + x] = 255;
		else//pepper
			dest[(512 * y) + x] = 0;
	}
}

double get_PSNR(unsigned char* ori, unsigned char* res)//PSNR 계산 함수
{
	double MSE = 0, PSNR = 0;
	int differ;
	for (int i = 0; i < 512; i++)
	{
		for (int j = 0; j < 512; j++)
		{
			differ = ori[512 * i + j] - res[512 * i + j];//기존이미지 - 복원한 이미지
			differ = differ * differ;
			MSE = MSE + differ;//누적 덧셈
		}
	}
	MSE = MSE / (512 * 512);
	//cout << "MSE = " << MSE << endl;
	PSNR = 10 * log10(255 * 255 / MSE);
	cout.precision(4);
	cout << fixed;
	cout << "MSE = " << MSE << "  PSNR = " << PSNR << "dB"<<endl;
	return PSNR;
}

void Copy_Padding(unsigned char* src, unsigned char* dest, int size)//Copy padding 함수, 
{//size만큼 위 아래 왼 오 각각 size/2만큼 가장자리 복사됨
	int dest_idx = 0;//dest 이미지의 index 변수

	//처음 padding되는 size/2 줄
	for (int i = 0; i < size / 2; i++)
	{
		for (int j = 0; j < size / 2; j++)
			dest[dest_idx++] = src[0];

		for (int j = 0; j < COL; j++)
			dest[dest_idx++] = src[j];
		for (int j = 0; j < size / 2; j++)
			dest[dest_idx++] = src[511];
	}
	//가운데 몸통
	for (int i = 0; i < ROW; i++)
	{
		for (int j = 0; j < size / 2; j++)
			dest[dest_idx++] = src[512 * i];
		for (int j = 0; j < COL; j++)
			dest[dest_idx++] = src[(512 * i) + j];
		for (int j = 0; j < size / 2; j++)
			dest[dest_idx++] = src[(512 * i) + 511];
	}
	//마지막 size/2 줄
	for (int i = 0; i < size / 2; i++)
	{
		for (int j = 0; j < size / 2; j++)
			dest[dest_idx++] = src[512 * 511];

		for (int j = 0; j < COL; j++)
			dest[dest_idx++] = src[(512 * 511) + j];
		for (int j = 0; j < size / 2; j++)
			dest[dest_idx++] = src[(512 * 512) - 1];
	}
	if (size == 15) {//padding 확인을 위한 임시 데이터 생성
		FILE* fptemp = fopen("temp.raw", "wb");
		fwrite(dest, sizeof(unsigned char), (512 + (size - 1)) * (512 + (size - 1)), fptemp);
	}
	return;
}
void Make_Gaussian_filter(int* filter, int size, int sigma)//가우시안 필터 생성함수
{
	double C = 1 / (2 * PI * sigma * sigma);// 1/2*pi*sigma^2
	double G, R;//G = G(x,y) 가우시안 필터, R 은 exp의 지수
	int normal_coordi = size / 2;//보정 크기
	int x, y;//필터의 좌표
	double sum = 0;
	ofstream fp_mask;
	if (size == 3)
		fp_mask.open("mask3x3.txt");
	else if(size == 9)
		fp_mask.open("mask9x9.txt");
	else if(size == 15)
		fp_mask.open("mask15x15.txt");

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			//좌표 평균을 0으로 만들기 위한 보정
			x = j - normal_coordi;
			y = i - normal_coordi;

			//공식 계산
			R = (double)(x * x + y * y) / (-2 * sigma * sigma);
			G = C * exp(R);
			cout << G << " ";
			sum = sum + G;
			filter[i * size + j] = (int)(G * 10000000); //int type으로 filter에 삽입, int type으로 변환하기 위해 임시로 10000000을 곱함
			fp_mask << filter[i * size + j] << " ";
		}
		fp_mask << endl;
	}
	cout << endl << endl;
}

int Convolution(unsigned char* src, int* filter, int size)//Convolution 연산 함수
{
	int sum = 0;
	int temp;
	for (int i = 0; i < size * size; i++)
	{
		temp = src[i] * filter[i] / 10000000;//filter 제작과정에서 곱해진 10000을 다시 나눠줌
		sum = sum + temp;//누적 합성
	}
	return sum;// 합성곱 반환
}
void get_Part(unsigned char* src, unsigned char* dest, int i, int j, int size)//Mask 적용연산을 위한 mask size만큼의 part 추출
{
	int normal_coordi = size / 2;//보정 크기
	int start = normal_coordi * (-1);
	int dest_idx = 0;
	for (int m = start; m < normal_coordi + 1; m++)
	{
		for (int n = start; n < normal_coordi; n++)
			dest[dest_idx++] = src[(i + m) * (512 + size - 1) + j + n];//dest와 src의 좌표 보정
	}
}
void Gaussian_filtering(unsigned char* src, int* filter, int size, int key, unsigned char* ori)//가우시안 필터 적용 후 raw파일 생성, PSNR 계산
{
	unsigned char* part = new unsigned char[size * size];//filter size의 원본 이미지 part
	unsigned char* output = new unsigned char[SIZE];//filtering 후 결과물
	int normal_coordi = size / 2;//보정 크기
	for (int i = 0; i < 512; i++)
	{
		for (int j = 0; j < 512; j++)
		{
			get_Part(src, part, i + normal_coordi, j + normal_coordi, size);//원본 이미지의 part 도출
			output[(i * 512) + j] = Convolution(part, filter, size);//Convolution 연산 후 결과대입
		}
	}
	FILE* fp_output;//결과 raw파일의 file pointer

	if (key == 1) //key는 노이즈 종류, 1~6(AGN5, 10, 15, SPN20, 30, 40)
	{//additive gaussian, sigma 5
		if (size == 3)
		{
			fp_output = fopen("AGN_5__G1.raw", "wb");
			cout << "Additive Gaussian Noise sigma = 5, Gaussian filter size 3x3, sigma 1" << endl;
			get_PSNR(ori, output);
		}
		else if (size == 9)
		{
			fp_output = fopen("AGN_5__G3.raw", "wb");
			cout << "Additive Gaussian Noise sigma = 5, Gaussian filter size 9x9, sigma 3" << endl;
			get_PSNR(ori, output);
		}
		else
		{
			fp_output = fopen("AGN_5__G5.raw", "wb");
			cout << "Additive Gaussian Noise sigma = 5, Gaussian filter size 15x15, sigma 5" << endl;
			get_PSNR(ori, output);
		}
	}
	else if (key == 2)
	{//additive gaussian, sigma 10
		if (size == 3)
		{
			fp_output = fopen("AGN_10__G1.raw", "wb");
			cout << "Additive Gaussian Noise sigma = 10, Gaussian filter size 3x3, sigma 1" << endl;
			get_PSNR(ori, output);
		}
		else if (size == 9)
		{
			fp_output = fopen("AGN_10__G3.raw", "wb");
			cout << "Additive Gaussian Noise sigma = 10, Gaussian filter size 9x9, sigma 3" << endl;
			get_PSNR(ori, output);
		}
		else
		{
			fp_output = fopen("AGN_10__G5.raw", "wb");
			cout << "Additive Gaussian Noise sigma = 10, Gaussian filter size 15x15, sigma 5" << endl;
			get_PSNR(ori, output);
		}
	}
	else if (key == 3)
	{//additive gaussian, sigma 15
		if (size == 3)
		{
			fp_output = fopen("AGN_15__G1.raw", "wb");
			cout << "Additive Gaussian Noise sigma = 15, Gaussian filter size 3x3, sigma 1" << endl;
			get_PSNR(ori, output);
		}
		else if (size == 9)
		{
			fp_output = fopen("AGN_15__G3.raw", "wb");
			cout << "Additive Gaussian Noise sigma = 15, Gaussian filter size 9x9, sigma 3" << endl;
			get_PSNR(ori, output);
		}
		else
		{
			fp_output = fopen("AGN_15__G5.raw", "wb");
			cout << "Additive Gaussian Noise sigma = 15, Gaussian filter size 15x15, sigma 5" << endl;
			get_PSNR(ori, output);
		}
	}
	else if (key == 4)
	{//salt and pepper, ratio 20
		if (size == 3)
		{
			fp_output = fopen("SPN_20__G1.raw", "wb");
			cout << "Salt and Pepper ratio = 20%, Gaussian filter size 3x3, sigma 1" << endl;
			get_PSNR(ori, output);
		}
		else if (size == 9)
		{
			fp_output = fopen("SPN_20__G3.raw", "wb");
			cout << "Salt and Pepper ratio = 20%, Gaussian filter size 9x9, sigma 3" << endl;
			get_PSNR(ori, output);
		}
		else
		{
			fp_output = fopen("SPN_20__G5.raw", "wb");
			cout << "Salt and Pepper ratio = 20%, Gaussian filter size 15x15, sigma 5" << endl;
			get_PSNR(ori, output);
		}
	}
	else if (key == 5)
	{//salt and pepper, ratio 30
		if (size == 3)
		{
			fp_output = fopen("SPN_30__G1.raw", "wb");
			cout << "Salt and Pepper ratio = 30%, Gaussian filter size 3x3, sigma 1" << endl;
			get_PSNR(ori, output);
		}
		else if (size == 9)
		{
			fp_output = fopen("SPN_30__G3.raw", "wb");
			cout << "Salt and Pepper ratio = 30%, Gaussian filter size 9x9, sigma 3" << endl;
			get_PSNR(ori, output);
		}
		else
		{
			fp_output = fopen("SPN_30__G5.raw", "wb");
			cout << "Salt and Pepper ratio = 30%, Gaussian filter size 15x15, sigma 5" << endl;
			get_PSNR(ori, output);
		}
	}
	else
	{//salt and pepper, ratio 40
		if (size == 3)
		{
			fp_output = fopen("SPN_40__G1.raw", "wb");
			cout << "Salt and Pepper ratio = 40%, Gaussian filter size 3x3, sigma 1" << endl;
			get_PSNR(ori, output);
		}
		else if (size == 9)
		{
			fp_output = fopen("SPN_40__G3.raw", "wb");
			cout << "Salt and Pepper ratio = 40%, Gaussian filter size 9x9, sigma 3" << endl;
			get_PSNR(ori, output);
		}
		else
		{
			fp_output = fopen("SPN_40__G5.raw", "wb");
			cout << "Salt and Pepper ratio = 40%, Gaussian filter size 15x15, sigma 5" << endl;
			get_PSNR(ori, output);
		}
	}
	cout << endl;
	fwrite(output, sizeof(unsigned char), SIZE, fp_output);
	fclose(fp_output);


}
unsigned char get_Median(unsigned char* src, int size)//Median을 얻는 함수
{
	for (int i = 0; i < size-2; i++)//bubble sort로 정렬
	{
		for (int j = i+1; j < size-1; j++)
		{
			if (src[j] > src[j+1])
			{
				int temp = src[j+1];
				src[j+1] = src[j];
				src[j+1] = temp;
			}
		}
	}
	return src[size / 2];//중간값 반환

}
unsigned char get_alpha(unsigned char* src, int size, double alpha)//trim 한 뒤 평균값을 얻는 함수
{
	for (int i = 0; i < size - 2; i++)//bubble sort로 정렬
	{
		for (int j = i + 1; j < size - 1; j++)
		{
			if (src[j] > src[j + 1])
			{
				int temp = src[j + 1];
				src[j + 1] = src[j];
				src[j + 1] = temp;
			}
		}
	}

	int trim = (int)(alpha * size * size);//버릴 요소의 개수
	int sum = 0;
	for (int i = trim; i < (size * size) - trim; i++)//양옆 trim개수를 제외한 요소들의 합
		sum = sum + src[i];
	return sum / (size * size - (2 * trim));//평균값 반환
}
void Median_filtering(unsigned char* src, int size, int key, unsigned char* ori)
{
	unsigned char* part = new unsigned char[size * size];//median 필터를 적용할 배열
	unsigned char* output = new unsigned char[SIZE];//filtering 후 결과물
	int normal_coordi = size / 2;//보정 크기
	for (int i = 0; i < 512; i++)
	{
		for (int j = 0; j < 512; j++)
		{
			get_Part(src, part, i + normal_coordi, j + normal_coordi, size);
			output[i * 512 + j] = get_Median(part, size);
		}
	}
	FILE* fp_output;
	if (key == 1)
	{
		if (size == 3)
		{
			fp_output = fopen("AGN_5__M3.raw", "wb");
			cout << "Additive Gaussian Noise sigma = 5, Median filter size 3x3" << endl;
			get_PSNR(ori, output);
		}
		else
		{
			fp_output = fopen("AGN_5__M5.raw", "wb");
			cout << "Additive Gaussian Noise sigma = 5, Median filter size 5x5" << endl;
			get_PSNR(ori, output);
		}
	}
	else if (key == 2)
	{
		if (size == 3)
		{
			fp_output = fopen("AGN_10__M3.raw", "wb");
			cout << "Additive Gaussian Noise sigma = 10, Median filter size 3x3" << endl;
			get_PSNR(ori, output);
		}
		else
		{
			fp_output = fopen("AGN_10__M5.raw", "wb");
			cout << "Additive Gaussian Noise sigma = 10, Median filter size 5x5" << endl;
			get_PSNR(ori, output);
		}
	}
	else if (key == 3)
	{
		if (size == 3)
		{
			fp_output = fopen("AGN_15__M3.raw", "wb");
			cout << "Additive Gaussian Noise sigma = 15, Median filter size 3x3" << endl;
			get_PSNR(ori, output);
		}
		else
		{
			fp_output = fopen("AGN_15__M5.raw", "wb");
			cout << "Additive Gaussian Noise sigma = 15, Median filter size 5x5" << endl;
			get_PSNR(ori, output);
		}
	}
	else if (key == 4)
	{
		if (size == 3)
		{
			fp_output = fopen("SPN_20__M3.raw", "wb");
			cout << "Salt and Pepper ratio = 20%, Median filter size 3x3" << endl;
			get_PSNR(ori, output);
		}
		else
		{
			fp_output = fopen("SPN_20__M5.raw", "wb");
			cout << "Salt and Pepper ratio = 20%, Median filter size 5x5" << endl;
			get_PSNR(ori, output);
		}
	}
	else if (key == 5)
	{
		if (size == 3)
		{
			fp_output = fopen("SPN_30__M3.raw", "wb");
			cout << "Salt and Pepper ratio = 30%, Median filter size 3x3" << endl;
			get_PSNR(ori, output);
		}
		else
		{
			fp_output = fopen("SPN_30__M5.raw", "wb");
			cout << "Salt and Pepper ratio = 30%, Median filter size 5x5" << endl;
			get_PSNR(ori, output);
		}
	}
	else
	{
		if (size == 3)
		{
			fp_output = fopen("SPN_40__M3.raw", "wb");
			cout << "Salt and Pepper ratio = 40%, Median filter size 3x3" << endl;
			get_PSNR(ori, output);
		}
		else
		{
			fp_output = fopen("SPN_40__M5.raw", "wb");
			cout << "Salt and Pepper ratio = 40%, Median filter size 5x5" << endl;
			get_PSNR(ori, output);
		}
	}
	cout << endl;
	fwrite(output, sizeof(unsigned char), SIZE, fp_output);
	fclose(fp_output);
	delete[] output;
	delete[] part;
}

void Alpha_trimmed_Mean(unsigned char* src, int size, int key, double alpha, unsigned char* ori)
{
	unsigned char* part = new unsigned char[size * size];//alpha trimmed mean 필터를 적용할 배열
	unsigned char* output = new unsigned char[SIZE];//filtering 후 결과물
	int normal_coordi = size / 2;//보정 크기
	for (int i = 0; i < 512; i++)
	{
		for (int j = 0; j < 512; j++)
		{
			get_Part(src, part, i + normal_coordi, j + normal_coordi, size);
			output[i * 512 + j] = get_alpha(part, size, alpha);
		}
	}
	FILE* fp_output;
	if (key == 1)
	{
		if (size == 3)
		{
			if (alpha == 0.2)
			{
				fp_output = fopen("AGN_5__A3_0.2.raw", "wb");
				cout << "Additive Gaussian Noise sigma = 5, Alpha-Trimmed Mean filter size 3x3, alpha 0.2" << endl;
				get_PSNR(ori, output);
			}
			else
			{
				fp_output = fopen("AGN_5__A3_0.4.raw", "wb");
				cout << "Additive Gaussian Noise sigma = 5, Alpha-Trimmed Mean filter size 3x3, alpha 0.4" << endl;
				get_PSNR(ori, output);
			}
		}
		else
		{
			if (alpha == 0.1)
			{
				fp_output = fopen("AGN_5__A5_0.1.raw", "wb");
				cout << "Additive Gaussian Noise sigma = 5, Median filter size 5x5, alpha 0.1" << endl;
				get_PSNR(ori, output);
			}
			else
			{
				fp_output = fopen("AGN_5__A5_0.4.raw", "wb");
				cout << "Additive Gaussian Noise sigma = 5, Median filter size 5x5, alpha 0.4" << endl;
				get_PSNR(ori, output);
			}
		}
	}
	else if (key == 2)
	{
		if (size == 3)
		{
			if (alpha == 0.2)
			{
				fp_output = fopen("AGN_10__A3_0.2.raw", "wb");
				cout << "Additive Gaussian Noise sigma = 10, Alpha-Trimmed Mean filter size 3x3, alpha 0.2" << endl;
				get_PSNR(ori, output);
			}
			else
			{
				fp_output = fopen("AGN_10__A3_0.4.raw", "wb");
				cout << "Additive Gaussian Noise sigma = 10, Alpha-Trimmed Mean filter size 3x3, alpha 0.4" << endl;
				get_PSNR(ori, output);
			}
		}
		else
		{
			if (alpha == 0.1)
			{
				fp_output = fopen("AGN_10__A5_0.1.raw", "wb");
				cout << "Additive Gaussian Noise sigma = 10, Median filter size 5x5, alpha 0.1" << endl;
				get_PSNR(ori, output);
			}
			else
			{
				fp_output = fopen("AGN_10__A5_0.4.raw", "wb");
				cout << "Additive Gaussian Noise sigma = 10, Median filter size 5x5, alpha 0.4" << endl;
				get_PSNR(ori, output);
			}
		}
	}
	else if (key == 3)
	{
		if (size == 3)
		{
			if (alpha == 0.2)
			{
				fp_output = fopen("AGN_15__A3_0.2.raw", "wb");
				cout << "Additive Gaussian Noise sigma = 15, Alpha-Trimmed Mean filter size 3x3, alpha 0.2" << endl;
				get_PSNR(ori, output);
			}
			else
			{
				fp_output = fopen("AGN_15__A3_0.4.raw", "wb");
				cout << "Additive Gaussian Noise sigma = 15, Alpha-Trimmed Mean filter size 3x3, alpha 0.4" << endl;
				get_PSNR(ori, output);
			}
		}
		else
		{
			if (alpha == 0.1)
			{
				fp_output = fopen("AGN_15__A5_0.1.raw", "wb");
				cout << "Additive Gaussian Noise sigma = 15, Median filter size 5x5, alpha 0.1" << endl;
				get_PSNR(ori, output);
			}
			else
			{
				fp_output = fopen("AGN_15__A5_0.4.raw", "wb");
				cout << "Additive Gaussian Noise sigma = 15, Median filter size 5x5, alpha 0.4" << endl;
				get_PSNR(ori, output);
			}
		}
	}
	else if (key == 4)
	{
		if (size == 3)
		{
			if (alpha == 0.2)
			{
				fp_output = fopen("SPN_20__A3_0.2.raw", "wb");
				cout << "Salt and Pepper ratio = 20%, Alpha-Trimmed Mean filter size 3x3, alpha 0.2" << endl;
				get_PSNR(ori, output);
			}
			else
			{
				fp_output = fopen("SPN_20__A3_0.4.raw", "wb");
				cout << "Salt and Pepper ratio = 20%, Alpha-Trimmed Mean filter size 3x3, alpha 0.4" << endl;
				get_PSNR(ori, output);
			}
		}
		else
		{
			if (alpha == 0.1)
			{
				fp_output = fopen("SPN_20__A5_0.1.raw", "wb");
				cout << "Salt and Pepper ratio = 20%, Median filter size 5x5, alpha 0.1" << endl;
				get_PSNR(ori, output);
			}
			else
			{
				fp_output = fopen("SPN_20__A5_0.4.raw", "wb");
				cout << "Salt and Pepper ratio = 20%, Median filter size 5x5, alpha 0.4" << endl;
				get_PSNR(ori, output);
			}
		}
	}
	else if (key == 5)
	{
	if (size == 3)
	{
		if (alpha == 0.2)
		{
			fp_output = fopen("SPN_30__A3_0.2.raw", "wb");
			cout << "Salt and Pepper ratio = 30%, Alpha-Trimmed Mean filter size 3x3, alpha 0.2" << endl;
			get_PSNR(ori, output);
		}
		else
		{
			fp_output = fopen("SPN_30__A3_0.4.raw", "wb");
			cout << "Salt and Pepper ratio = 30%, Alpha-Trimmed Mean filter size 3x3, alpha 0.4" << endl;
			get_PSNR(ori, output);
		}
	}
	else
	{
		if (alpha == 0.1)
		{
			fp_output = fopen("SPN_30__A5_0.1.raw", "wb");
			cout << "Salt and Pepper ratio = 30%, Median filter size 5x5, alpha 0.1" << endl;
			get_PSNR(ori, output);
		}
		else
		{
			fp_output = fopen("SPN_30__A5_0.4.raw", "wb");
			cout << "Salt and Pepper ratio = 30%, Median filter size 5x5, alpha 0.4" << endl;
			get_PSNR(ori, output);
		}
	}
	}
	else
	{
		if (size == 3)
		{
			if (alpha == 0.2)
			{
				fp_output = fopen("SPN_40__A3_0.2.raw", "wb");
				cout << "Salt and Pepper ratio = 40%, Alpha-Trimmed Mean filter size 3x3, alpha 0.2" << endl;
				get_PSNR(ori, output);
			}
			else
			{
				fp_output = fopen("SPN_40__A3_0.4.raw", "wb");
				cout << "Salt and Pepper ratio = 40%, Alpha-Trimmed Mean filter size 3x3, alpha 0.4" << endl;
				get_PSNR(ori, output);
			}
		}
		else
		{
			if (alpha == 0.1)
			{
				fp_output = fopen("SPN_40__A5_0.1.raw", "wb");
				cout << "Salt and Pepper ratio = 40%, Median filter size 5x5, alpha 0.1" << endl;
				get_PSNR(ori, output);
			}
			else
			{
				fp_output = fopen("SPN_40__A5_0.4.raw", "wb");
				cout << "Salt and Pepper ratio = 40%, Median filter size 5x5, alpha 0.4" << endl;
				get_PSNR(ori, output);
			}
		}
	}
	cout << endl;
	fwrite(output, sizeof(unsigned char), SIZE, fp_output);
	fclose(fp_output);
	delete[] output;
	delete[] part;
}

int main()
{
	//원본 이미지 불러오기
	FILE* fp_input_img = fopen("lena(512x512).raw", "rb");
	unsigned char* uc_input_img = new unsigned char[SIZE];
	fread(uc_input_img, sizeof(unsigned char), SIZE, fp_input_img);

	cout << RAND_MAX << endl;
	//노이즈 추가된 이미지 배열
	unsigned char* noise_img_agn5 = new unsigned char[SIZE];
	unsigned char* noise_img_agn10 = new unsigned char[SIZE];
	unsigned char* noise_img_agn15 = new unsigned char[SIZE];
	unsigned char* noise_img_spn20 = new unsigned char[SIZE];
	unsigned char* noise_img_spn30 = new unsigned char[SIZE];
	unsigned char* noise_img_spn40 = new unsigned char[SIZE];

	//Additive Gaussian 노이즈 추가 sigma = 5, 10, 15
	Additive_Gaussian(uc_input_img, noise_img_agn5, 5);//sigam = 5
	Additive_Gaussian(uc_input_img, noise_img_agn10, 10);//sigma = 10
	Additive_Gaussian(uc_input_img, noise_img_agn15, 15);//sigma = 15
	//Additive Gaussian 노이즈 추가된 raw파일 생성
	FILE* fp_agn5 = fopen("AdditiveGaussian_sigma5.raw", "wb");
	fwrite(noise_img_agn5, sizeof(unsigned char), SIZE, fp_agn5);
	FILE* fp_agn10 = fopen("AdditiveGaussian_sigma10.raw", "wb");
	fwrite(noise_img_agn10, sizeof(unsigned char), SIZE, fp_agn10);
	FILE* fp_agn15 = fopen("AdditiveGaussian_sigma15.raw", "wb");
	fwrite(noise_img_agn15, sizeof(unsigned char), SIZE, fp_agn15);

	//Salt and Pepper 노이즈 추가, 비율 20, 30, 40
	SaltandPepper(uc_input_img, noise_img_spn20, 20);
	SaltandPepper(uc_input_img, noise_img_spn30, 30);
	SaltandPepper(uc_input_img, noise_img_spn40, 40);
	//Salt and pepper 노이즈 추가된 raw 파일 생성
	FILE* fp_spn20 = fopen("SaltandPepper_ratio20.raw", "wb");
	fwrite(noise_img_spn20, sizeof(unsigned char), SIZE, fp_spn20);
	FILE* fp_spn30 = fopen("SaltandPepper_ratio30.raw", "wb");
	fwrite(noise_img_spn30, sizeof(unsigned char), SIZE, fp_spn30);
	FILE* fp_spn40 = fopen("SaltandPepper_ratio40.raw", "wb");
	fwrite(noise_img_spn40, sizeof(unsigned char), SIZE, fp_spn40);

	//copy padding 할 이미지 배열 
	//3x3 필터 적용할 배열 padding
	unsigned char* padded_img3_agn5 = new unsigned char[(ROW + 2) * (COL + 2)];
	unsigned char* padded_img3_agn10 = new unsigned char[(ROW + 2) * (COL + 2)];
	unsigned char* padded_img3_agn15 = new unsigned char[(ROW + 2) * (COL + 2)];

	unsigned char* padded_img3_spn20 = new unsigned char[(ROW + 2) * (COL + 2)];
	unsigned char* padded_img3_spn30 = new unsigned char[(ROW + 2) * (COL + 2)];
	unsigned char* padded_img3_spn40 = new unsigned char[(ROW + 2) * (COL + 2)];

	Copy_Padding(noise_img_agn5, padded_img3_agn5, 3);
	Copy_Padding(noise_img_agn10, padded_img3_agn10, 3);
	Copy_Padding(noise_img_agn15, padded_img3_agn15, 3);
	Copy_Padding(noise_img_spn20, padded_img3_spn20, 3);
	Copy_Padding(noise_img_spn30, padded_img3_spn30, 3);
	Copy_Padding(noise_img_spn40, padded_img3_spn40, 3);

	//5x5 필터 적용할 배열 padding
	unsigned char* padded_img5_agn5 = new unsigned char[(ROW + 4) * (COL + 4)]; // 5x5
	unsigned char* padded_img5_agn10 = new unsigned char[(ROW + 4) * (COL + 4)]; // 5x5
	unsigned char* padded_img5_agn15 = new unsigned char[(ROW + 4) * (COL + 4)]; // 5x5

	unsigned char* padded_img5_spn20 = new unsigned char[(ROW + 4) * (COL + 4)]; // 5x5
	unsigned char* padded_img5_spn30 = new unsigned char[(ROW + 4) * (COL + 4)]; // 5x5
	unsigned char* padded_img5_spn40 = new unsigned char[(ROW + 4) * (COL + 4)]; // 5x5

	Copy_Padding(noise_img_agn5, padded_img5_agn5, 5);
	Copy_Padding(noise_img_agn10, padded_img5_agn10, 5);
	Copy_Padding(noise_img_agn15, padded_img5_agn15, 5);
	Copy_Padding(noise_img_spn20, padded_img5_spn20, 5);
	Copy_Padding(noise_img_spn30, padded_img5_spn30, 5);
	Copy_Padding(noise_img_spn40, padded_img5_spn40, 5);

	//9x9 필터 적용할 배열 padding
	unsigned char* padded_img9_agn5 = new unsigned char[(ROW + 8) * (COL + 8)]; // 9x9
	unsigned char* padded_img9_agn10 = new unsigned char[(ROW + 8) * (COL + 8)]; // 9x9
	unsigned char* padded_img9_agn15 = new unsigned char[(ROW + 8) * (COL + 8)]; // 9x9

	unsigned char* padded_img9_spn20 = new unsigned char[(ROW + 8) * (COL + 8)]; // 9x9
	unsigned char* padded_img9_spn30 = new unsigned char[(ROW + 8) * (COL + 8)]; // 9x9
	unsigned char* padded_img9_spn40 = new unsigned char[(ROW + 8) * (COL + 8)]; // 9x9

	Copy_Padding(noise_img_agn5, padded_img9_agn5, 9);
	Copy_Padding(noise_img_agn10, padded_img9_agn10, 9);
	Copy_Padding(noise_img_agn15, padded_img9_agn15, 9);
	Copy_Padding(noise_img_spn20, padded_img9_spn20, 9);
	Copy_Padding(noise_img_spn30, padded_img9_spn30, 9);
	Copy_Padding(noise_img_spn40, padded_img9_spn40, 9);

	//15x15 필터 적용할 배열 padding
	unsigned char* padded_img15_agn5 = new unsigned char[(ROW + 14) * (COL + 14)]; //15x15
	unsigned char* padded_img15_agn10 = new unsigned char[(ROW + 14) * (COL + 14)]; //15x15
	unsigned char* padded_img15_agn15 = new unsigned char[(ROW + 14) * (COL + 14)]; //15x15

	unsigned char* padded_img15_spn20 = new unsigned char[(ROW + 14) * (COL + 14)]; //15x15
	unsigned char* padded_img15_spn30 = new unsigned char[(ROW + 14) * (COL + 14)]; //15x15
	unsigned char* padded_img15_spn40 = new unsigned char[(ROW + 14) * (COL + 14)]; //15x15

	Copy_Padding(noise_img_agn5, padded_img15_agn5, 15);
	Copy_Padding(noise_img_agn10, padded_img15_agn10, 15);
	Copy_Padding(noise_img_agn15, padded_img15_agn15, 15);
	Copy_Padding(noise_img_spn20, padded_img15_spn20, 15);
	Copy_Padding(noise_img_spn30, padded_img15_spn30, 15);
	Copy_Padding(noise_img_spn40, padded_img15_spn40, 15);

	//Gaussian filter 마스크 생성
	int* Gaussian_filter3_1 = new int[9];//3x3
	int* Gaussian_filter9_3 = new int[81];//9x9
	int* Gaussian_filter15_5 = new int[225];//15x15
	Make_Gaussian_filter(Gaussian_filter3_1, 3, 1);//3x3, sigma 1
	Make_Gaussian_filter(Gaussian_filter9_3, 9, 3);//9x9, sigam 3
	Make_Gaussian_filter(Gaussian_filter15_5, 15, 5);//15x15, sigma 5

	//key 1~6, 각각 Additive Gaussian noise 5, 10, 15, Salt and Pepper noise 20, 30, 40

	//Gaussian filtering mask 3x3, sigma = 1
	Gaussian_filtering(padded_img3_agn5, Gaussian_filter3_1, 3, 1, uc_input_img);
	Gaussian_filtering(padded_img3_agn10, Gaussian_filter3_1, 3, 2, uc_input_img);
	Gaussian_filtering(padded_img3_agn15, Gaussian_filter3_1, 3, 3, uc_input_img);
	Gaussian_filtering(padded_img3_spn20, Gaussian_filter3_1, 3, 4, uc_input_img);
	Gaussian_filtering(padded_img3_spn30, Gaussian_filter3_1, 3, 5, uc_input_img);
	Gaussian_filtering(padded_img3_spn40, Gaussian_filter3_1, 3, 6, uc_input_img);
	
	//Gaussian filtering mask 9x9 sigma = 3
	Gaussian_filtering(padded_img9_agn5, Gaussian_filter9_3, 9, 1, uc_input_img);
	Gaussian_filtering(padded_img9_agn10, Gaussian_filter9_3, 9, 2, uc_input_img);
	Gaussian_filtering(padded_img9_agn15, Gaussian_filter9_3, 9, 3, uc_input_img);
	Gaussian_filtering(padded_img9_spn20, Gaussian_filter9_3, 9, 4, uc_input_img);
	Gaussian_filtering(padded_img9_spn30, Gaussian_filter9_3, 9, 5, uc_input_img);
	Gaussian_filtering(padded_img9_spn40, Gaussian_filter9_3, 9, 6, uc_input_img);

	//Gaussian filtering mask 15x15, sigma = 5
	Gaussian_filtering(padded_img15_agn5, Gaussian_filter15_5, 15, 1, uc_input_img);
	Gaussian_filtering(padded_img15_agn10, Gaussian_filter15_5, 15, 2, uc_input_img);
	Gaussian_filtering(padded_img15_agn15, Gaussian_filter15_5, 15, 3, uc_input_img);
	Gaussian_filtering(padded_img15_spn20, Gaussian_filter15_5, 15, 4, uc_input_img);
	Gaussian_filtering(padded_img15_spn30, Gaussian_filter15_5, 15, 5, uc_input_img);
	Gaussian_filtering(padded_img15_spn40, Gaussian_filter15_5, 15, 6, uc_input_img);

	//Median filtering size 3x3
	Median_filtering(padded_img3_agn5, 3, 1, uc_input_img);
	Median_filtering(padded_img3_agn10, 3, 2, uc_input_img);
	Median_filtering(padded_img3_agn15, 3, 3, uc_input_img);
	Median_filtering(padded_img3_spn20, 3, 4, uc_input_img);
	Median_filtering(padded_img3_spn30, 3, 5, uc_input_img);
	Median_filtering(padded_img3_spn40, 3, 6, uc_input_img);

	//Median filtering size 5x5
	Median_filtering(padded_img5_agn5, 5, 1, uc_input_img);
	Median_filtering(padded_img5_agn10, 5, 2, uc_input_img);
	Median_filtering(padded_img5_agn15, 5, 3, uc_input_img);
	Median_filtering(padded_img5_spn20, 5, 4, uc_input_img);
	Median_filtering(padded_img5_spn30, 5, 5, uc_input_img);
	Median_filtering(padded_img5_spn40, 5, 6, uc_input_img);

	//Alpha-trimmed mean filtering size 3x3, alpha = 0.2
	Alpha_trimmed_Mean(padded_img3_agn5, 3, 1, 0.2, uc_input_img);
	Alpha_trimmed_Mean(padded_img3_agn10, 3, 2, 0.2, uc_input_img);
	Alpha_trimmed_Mean(padded_img3_agn15, 3, 3, 0.2, uc_input_img);
	Alpha_trimmed_Mean(padded_img3_spn20, 3, 4, 0.2, uc_input_img);
	Alpha_trimmed_Mean(padded_img3_spn30, 3, 5, 0.2, uc_input_img);
	Alpha_trimmed_Mean(padded_img3_spn40, 3, 6, 0.2, uc_input_img);

	//Alpha-trimmed mean filtering size 3x3, alpha = 0.4
	Alpha_trimmed_Mean(padded_img3_agn5, 3, 1, 0.4, uc_input_img);
	Alpha_trimmed_Mean(padded_img3_agn10, 3, 2, 0.4, uc_input_img);
	Alpha_trimmed_Mean(padded_img3_agn15, 3, 3, 0.4, uc_input_img);
	Alpha_trimmed_Mean(padded_img3_spn20, 3, 4, 0.4, uc_input_img);
	Alpha_trimmed_Mean(padded_img3_spn30, 3, 5, 0.4, uc_input_img);
	Alpha_trimmed_Mean(padded_img3_spn40, 3, 6, 0.4, uc_input_img);

	//Alpha-trimmed mean filtering size 5x5, alpha = 0.1
	Alpha_trimmed_Mean(padded_img5_agn5, 5, 1, 0.1, uc_input_img);
	Alpha_trimmed_Mean(padded_img5_agn10, 5, 2, 0.1, uc_input_img);
	Alpha_trimmed_Mean(padded_img5_agn15, 5, 3, 0.1, uc_input_img);
	Alpha_trimmed_Mean(padded_img5_spn20, 5, 4, 0.1, uc_input_img);
	Alpha_trimmed_Mean(padded_img5_spn30, 5, 5, 0.1, uc_input_img);
	Alpha_trimmed_Mean(padded_img5_spn40, 5, 6, 0.1, uc_input_img);

	//Alpha-trimmed mean filtering size 5x5, alpha = 0.4
	Alpha_trimmed_Mean(padded_img5_agn5, 5, 1, 0.4, uc_input_img);
	Alpha_trimmed_Mean(padded_img5_agn10, 5, 2, 0.4, uc_input_img);
	Alpha_trimmed_Mean(padded_img5_agn15, 5, 3, 0.4, uc_input_img);
	Alpha_trimmed_Mean(padded_img5_spn20, 5, 4, 0.4, uc_input_img);
	Alpha_trimmed_Mean(padded_img5_spn30, 5, 5, 0.4, uc_input_img);
	Alpha_trimmed_Mean(padded_img5_spn40, 5, 6, 0.4, uc_input_img);

	//할당한 모든 메모리 해제
	delete[] uc_input_img;
	delete[] noise_img_agn5;
	delete[] noise_img_agn10;
	delete[] noise_img_spn20;
	delete[] noise_img_spn30;
	delete[] noise_img_spn40;

	delete[] padded_img3_agn5;
	delete[] padded_img3_agn10;
	delete[] padded_img3_agn15;
	delete[] padded_img3_spn20;
	delete[] padded_img3_spn30;
	delete[] padded_img3_spn40;

	delete[] padded_img5_agn5;
	delete[] padded_img5_agn10;
	delete[] padded_img5_agn15;
	delete[] padded_img5_spn20;
	delete[] padded_img5_spn30;
	delete[] padded_img5_spn40;

	delete[] padded_img9_agn5;
	delete[] padded_img9_agn10;
	delete[] padded_img9_agn15;
	delete[] padded_img9_spn20;
	delete[] padded_img9_spn30;
	delete[] padded_img9_spn40;

	delete[] padded_img15_agn5;
	delete[] padded_img15_agn10;
	delete[] padded_img15_agn15;
	delete[] padded_img15_spn20;
	delete[] padded_img15_spn30;
	delete[] padded_img15_spn40;
}
