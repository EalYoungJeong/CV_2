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
#define ROW 512	//���α���
#define COL 512 //���α���
#define SIZE 512*512 //��ü �̹��� ũ��
#define PI 3.141592 //������ ����
void Additive_Gaussian(unsigned char* src, unsigned char* dest, int sigma)//Additive Gaussian ������ �߰��Լ�
{
	srand(time(NULL));//time���� rand �Լ� seed
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
		double temp = src[i] + rn_gaussian;//���� �̹��� pixel �� ����þ� ������ �߰�

		//0~255 level���� ������ 256�̻��� �����÷ο� ����
		if (temp < 0) dest[i] = 0;
		else if (temp > 255) dest[i] = 255;
		else dest[i] = temp;

		sum = sum + rn_gaussian;
	}
}
void SaltandPepper(unsigned char* src, unsigned char* dest, int ratio)//Salt and Pepper ������ �߰� �Լ�
{
	srand(time(NULL));
	int n = SIZE * ratio / 100;//������ŭ �ұ����� �Ѹ�
	//�̹��� �迭 ����
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

double get_PSNR(unsigned char* ori, unsigned char* res)//PSNR ��� �Լ�
{
	double MSE = 0, PSNR = 0;
	int differ;
	for (int i = 0; i < 512; i++)
	{
		for (int j = 0; j < 512; j++)
		{
			differ = ori[512 * i + j] - res[512 * i + j];//�����̹��� - ������ �̹���
			differ = differ * differ;
			MSE = MSE + differ;//���� ����
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

void Copy_Padding(unsigned char* src, unsigned char* dest, int size)//Copy padding �Լ�, 
{//size��ŭ �� �Ʒ� �� �� ���� size/2��ŭ �����ڸ� �����
	int dest_idx = 0;//dest �̹����� index ����

	//ó�� padding�Ǵ� size/2 ��
	for (int i = 0; i < size / 2; i++)
	{
		for (int j = 0; j < size / 2; j++)
			dest[dest_idx++] = src[0];

		for (int j = 0; j < COL; j++)
			dest[dest_idx++] = src[j];
		for (int j = 0; j < size / 2; j++)
			dest[dest_idx++] = src[511];
	}
	//��� ����
	for (int i = 0; i < ROW; i++)
	{
		for (int j = 0; j < size / 2; j++)
			dest[dest_idx++] = src[512 * i];
		for (int j = 0; j < COL; j++)
			dest[dest_idx++] = src[(512 * i) + j];
		for (int j = 0; j < size / 2; j++)
			dest[dest_idx++] = src[(512 * i) + 511];
	}
	//������ size/2 ��
	for (int i = 0; i < size / 2; i++)
	{
		for (int j = 0; j < size / 2; j++)
			dest[dest_idx++] = src[512 * 511];

		for (int j = 0; j < COL; j++)
			dest[dest_idx++] = src[(512 * 511) + j];
		for (int j = 0; j < size / 2; j++)
			dest[dest_idx++] = src[(512 * 512) - 1];
	}
	if (size == 15) {//padding Ȯ���� ���� �ӽ� ������ ����
		FILE* fptemp = fopen("temp.raw", "wb");
		fwrite(dest, sizeof(unsigned char), (512 + (size - 1)) * (512 + (size - 1)), fptemp);
	}
	return;
}
void Make_Gaussian_filter(int* filter, int size, int sigma)//����þ� ���� �����Լ�
{
	double C = 1 / (2 * PI * sigma * sigma);// 1/2*pi*sigma^2
	double G, R;//G = G(x,y) ����þ� ����, R �� exp�� ����
	int normal_coordi = size / 2;//���� ũ��
	int x, y;//������ ��ǥ
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
			//��ǥ ����� 0���� ����� ���� ����
			x = j - normal_coordi;
			y = i - normal_coordi;

			//���� ���
			R = (double)(x * x + y * y) / (-2 * sigma * sigma);
			G = C * exp(R);
			cout << G << " ";
			sum = sum + G;
			filter[i * size + j] = (int)(G * 10000000); //int type���� filter�� ����, int type���� ��ȯ�ϱ� ���� �ӽ÷� 10000000�� ����
			fp_mask << filter[i * size + j] << " ";
		}
		fp_mask << endl;
	}
	cout << endl << endl;
}

int Convolution(unsigned char* src, int* filter, int size)//Convolution ���� �Լ�
{
	int sum = 0;
	int temp;
	for (int i = 0; i < size * size; i++)
	{
		temp = src[i] * filter[i] / 10000000;//filter ���۰������� ������ 10000�� �ٽ� ������
		sum = sum + temp;//���� �ռ�
	}
	return sum;// �ռ��� ��ȯ
}
void get_Part(unsigned char* src, unsigned char* dest, int i, int j, int size)//Mask ���뿬���� ���� mask size��ŭ�� part ����
{
	int normal_coordi = size / 2;//���� ũ��
	int start = normal_coordi * (-1);
	int dest_idx = 0;
	for (int m = start; m < normal_coordi + 1; m++)
	{
		for (int n = start; n < normal_coordi; n++)
			dest[dest_idx++] = src[(i + m) * (512 + size - 1) + j + n];//dest�� src�� ��ǥ ����
	}
}
void Gaussian_filtering(unsigned char* src, int* filter, int size, int key, unsigned char* ori)//����þ� ���� ���� �� raw���� ����, PSNR ���
{
	unsigned char* part = new unsigned char[size * size];//filter size�� ���� �̹��� part
	unsigned char* output = new unsigned char[SIZE];//filtering �� �����
	int normal_coordi = size / 2;//���� ũ��
	for (int i = 0; i < 512; i++)
	{
		for (int j = 0; j < 512; j++)
		{
			get_Part(src, part, i + normal_coordi, j + normal_coordi, size);//���� �̹����� part ����
			output[(i * 512) + j] = Convolution(part, filter, size);//Convolution ���� �� �������
		}
	}
	FILE* fp_output;//��� raw������ file pointer

	if (key == 1) //key�� ������ ����, 1~6(AGN5, 10, 15, SPN20, 30, 40)
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
unsigned char get_Median(unsigned char* src, int size)//Median�� ��� �Լ�
{
	for (int i = 0; i < size-2; i++)//bubble sort�� ����
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
	return src[size / 2];//�߰��� ��ȯ

}
unsigned char get_alpha(unsigned char* src, int size, double alpha)//trim �� �� ��հ��� ��� �Լ�
{
	for (int i = 0; i < size - 2; i++)//bubble sort�� ����
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

	int trim = (int)(alpha * size * size);//���� ����� ����
	int sum = 0;
	for (int i = trim; i < (size * size) - trim; i++)//�翷 trim������ ������ ��ҵ��� ��
		sum = sum + src[i];
	return sum / (size * size - (2 * trim));//��հ� ��ȯ
}
void Median_filtering(unsigned char* src, int size, int key, unsigned char* ori)
{
	unsigned char* part = new unsigned char[size * size];//median ���͸� ������ �迭
	unsigned char* output = new unsigned char[SIZE];//filtering �� �����
	int normal_coordi = size / 2;//���� ũ��
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
	unsigned char* part = new unsigned char[size * size];//alpha trimmed mean ���͸� ������ �迭
	unsigned char* output = new unsigned char[SIZE];//filtering �� �����
	int normal_coordi = size / 2;//���� ũ��
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
	//���� �̹��� �ҷ�����
	FILE* fp_input_img = fopen("lena(512x512).raw", "rb");
	unsigned char* uc_input_img = new unsigned char[SIZE];
	fread(uc_input_img, sizeof(unsigned char), SIZE, fp_input_img);

	cout << RAND_MAX << endl;
	//������ �߰��� �̹��� �迭
	unsigned char* noise_img_agn5 = new unsigned char[SIZE];
	unsigned char* noise_img_agn10 = new unsigned char[SIZE];
	unsigned char* noise_img_agn15 = new unsigned char[SIZE];
	unsigned char* noise_img_spn20 = new unsigned char[SIZE];
	unsigned char* noise_img_spn30 = new unsigned char[SIZE];
	unsigned char* noise_img_spn40 = new unsigned char[SIZE];

	//Additive Gaussian ������ �߰� sigma = 5, 10, 15
	Additive_Gaussian(uc_input_img, noise_img_agn5, 5);//sigam = 5
	Additive_Gaussian(uc_input_img, noise_img_agn10, 10);//sigma = 10
	Additive_Gaussian(uc_input_img, noise_img_agn15, 15);//sigma = 15
	//Additive Gaussian ������ �߰��� raw���� ����
	FILE* fp_agn5 = fopen("AdditiveGaussian_sigma5.raw", "wb");
	fwrite(noise_img_agn5, sizeof(unsigned char), SIZE, fp_agn5);
	FILE* fp_agn10 = fopen("AdditiveGaussian_sigma10.raw", "wb");
	fwrite(noise_img_agn10, sizeof(unsigned char), SIZE, fp_agn10);
	FILE* fp_agn15 = fopen("AdditiveGaussian_sigma15.raw", "wb");
	fwrite(noise_img_agn15, sizeof(unsigned char), SIZE, fp_agn15);

	//Salt and Pepper ������ �߰�, ���� 20, 30, 40
	SaltandPepper(uc_input_img, noise_img_spn20, 20);
	SaltandPepper(uc_input_img, noise_img_spn30, 30);
	SaltandPepper(uc_input_img, noise_img_spn40, 40);
	//Salt and pepper ������ �߰��� raw ���� ����
	FILE* fp_spn20 = fopen("SaltandPepper_ratio20.raw", "wb");
	fwrite(noise_img_spn20, sizeof(unsigned char), SIZE, fp_spn20);
	FILE* fp_spn30 = fopen("SaltandPepper_ratio30.raw", "wb");
	fwrite(noise_img_spn30, sizeof(unsigned char), SIZE, fp_spn30);
	FILE* fp_spn40 = fopen("SaltandPepper_ratio40.raw", "wb");
	fwrite(noise_img_spn40, sizeof(unsigned char), SIZE, fp_spn40);

	//copy padding �� �̹��� �迭 
	//3x3 ���� ������ �迭 padding
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

	//5x5 ���� ������ �迭 padding
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

	//9x9 ���� ������ �迭 padding
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

	//15x15 ���� ������ �迭 padding
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

	//Gaussian filter ����ũ ����
	int* Gaussian_filter3_1 = new int[9];//3x3
	int* Gaussian_filter9_3 = new int[81];//9x9
	int* Gaussian_filter15_5 = new int[225];//15x15
	Make_Gaussian_filter(Gaussian_filter3_1, 3, 1);//3x3, sigma 1
	Make_Gaussian_filter(Gaussian_filter9_3, 9, 3);//9x9, sigam 3
	Make_Gaussian_filter(Gaussian_filter15_5, 15, 5);//15x15, sigma 5

	//key 1~6, ���� Additive Gaussian noise 5, 10, 15, Salt and Pepper noise 20, 30, 40

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

	//�Ҵ��� ��� �޸� ����
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
