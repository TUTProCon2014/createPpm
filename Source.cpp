#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <ctime>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

typedef struct _RGB_DATA
{
	uchar r;    //R
	uchar g;    //G
	uchar b;    //B
}RGB_DATA;

/*2～16までの乱数作成(分割数)。分割数決め打ちのため現在未使用*/
int devideRand(){
	return rand() % 15 + 2;
}

RGB_DATA getRGB(IplImage *img, unsigned int x, unsigned int y)
{
	RGB_DATA data;
	data.r = data.g = data.b = 0;
	data.b = ((uchar*)(img->imageData + img->widthStep*y))[x * 3];
	data.g = ((uchar*)(img->imageData + img->widthStep*y))[x * 3 + 1];
	data.r = ((uchar*)(img->imageData + img->widthStep*y))[x * 3 + 2];
	return data;
}

bool writePpmHeader(const string& outputfile, const Mat& out_mat, const int div_x, const int div_y)
{
	ofstream ofs(outputfile, std::ios::binary);
	ofs << "P6" << endl;
	ofs << "# " << div_x << " " << div_y << endl;
	ofs << "# 10" << endl;
	ofs << "# 120 10" << endl;
	ofs << out_mat.cols << " " << out_mat.rows << endl;
	ofs << "255" << endl;
	return true;
}

bool writePpmImage(const string outputfile, const Mat& out_mat){
	IplImage src_img = out_mat;

	//ファイルオープン（バイナリ書き込みモード）
	FILE *fp;
	fopen_s(&fp, (char*)&outputfile, "ab");

	RGB_DATA rgb;
	rgb.r = rgb.g = rgb.b = 0;

	//ファイルオープンに成功したらデータを書き込む
	if (fp != NULL) {
		for (int y = 0; y < src_img.height; y++)
		{
			for (int x = 0; x < src_img.width; x++)
			{
				//RGB値取得
				rgb = getRGB(&src_img, x, y);

				//バイナリデータ書き込み
				fwrite(&rgb.r, sizeof(uchar), 1, fp);
				fwrite(&rgb.g, sizeof(uchar), 1, fp);
				fwrite(&rgb.b, sizeof(uchar), 1, fp);
			}
		}

		//ファイルクローズ
		fclose(fp);
	}
	return true;
}

void SavePpm(const string& filename, const Mat& output, const int div_x, const int div_y){
	writePpmHeader(filename, output, div_x, div_y); //ヘッダ編集
	writePpmImage(filename,output); //Matをバイナリで書き込み
}


int main(int argc, char* argv[]){
	/*画像ごとにパラメーターを変更(下４行)*/
	const auto readfile = "img.jpg"; //読み込み画像(決め打ち)
	const auto outputfile = "img1.ppm"; //書き込み画像(決め打ち)
	const int div_x = 3; //devideRand(); 横分割数(決め打ち)
	const int div_y = 3; //devideRand(); 縦分割数(決め打ち)
	/**/

	srand((unsigned int)time(NULL));
	int w, h, i, j;
	IplImage *src_img, *dst_img, *tmp_img[2];
	const int div_xy = div_x*div_y;
	CvRect roi[div_xy];
	CvRNG rng = cvRNG(time(NULL));

	// (1)画像を読み込む
	src_img = cvLoadImage(readfile, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);

	w = src_img->width - src_img->width % div_x + div_x;
	h = src_img->height - src_img->height % div_y + div_y;
	dst_img = cvCreateImage(cvSize(w, h), src_img->depth, src_img->nChannels);
	tmp_img[0] = cvCreateImage(cvSize(w / div_x, h / div_y), src_img->depth, src_img->nChannels);
	tmp_img[1] = cvCreateImage(cvSize(w / div_x, h / div_y), src_img->depth, src_img->nChannels);
	
	Mat s_img = cvarrToMat(src_img);
	Mat d_img = cvarrToMat(dst_img);

	imshow(readfile,s_img); //読み込み確認

	resize(s_img, d_img, d_img.size());
	IplImage ds_img = d_img;

	// (2)画像を分割するための矩形を設定
	for (i = 0; i < div_x; i++) {
		for (j = 0; j < div_y; j++) {
			roi[div_x * j + i].x = w / div_x * i;
			roi[div_x * j + i].y = h / div_y * j;
			roi[div_x * j + i].width = w / div_x;
			roi[div_x * j + i].height = h / div_y;
		}
	}

	// (3)ROIを利用して部分画像を入れ換える
	for (i = 0; i < div_xy * 2; i++) {
		int p1 = cvRandInt(&rng) % div_xy;
		int p2 = cvRandInt(&rng) % div_xy;
		cvSetImageROI(&ds_img, roi[p1]);
		cvCopy(&ds_img, tmp_img[0]);
		cvSetImageROI(&ds_img, roi[p2]);
		cvCopy(&ds_img, tmp_img[1]);
		cvCopy(tmp_img[0], &ds_img);
		cvSetImageROI(&ds_img, roi[p1]);
		cvCopy(tmp_img[1], &ds_img);
	}
	cvResetImageROI(&ds_img);

	Mat reDst_img = cvarrToMat(&ds_img);

	resize(reDst_img, s_img, s_img.size());

	// (4)画像の表示
	//imshow("分割確認", s_img);

	SavePpm(outputfile, s_img, div_x, div_y); //問題形式でppm保存

	IplImage *check_img = cvLoadImage(outputfile, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	cvShowImage(outputfile, check_img); //ppmファイル確認

	cvWaitKey(0); // キー入力待ち

	cvReleaseImage(&src_img);
	cvReleaseImage(&dst_img);
	cvReleaseImage(&check_img);
	return 0;
}