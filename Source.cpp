#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <cstdio>
#include <ctime>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "../utils/include/image.hpp"
#include "../utils/include/types.hpp"


using namespace cv;
using namespace std;
using namespace procon;


template <typename T, typename S>
T readFrom(S& stream)
{
	T t;
	stream >> t;
	return t;
}


bool writePpmHeader(const string& outputfile, const Mat& out_mat, const int div_x, const int div_y)
{
	ofstream ofs(outputfile, std::ios::binary);
	ofs << "P6" << endl;
	ofs << "# " << div_x << " " << div_y << endl;
	ofs << "# 10" << endl;
	ofs << "# 120 10" << endl;
	return true;
}


bool writePpmImage(const string outputfile, const Mat& out_mat){
	//ファイルオープン（バイナリ書き込みモード）
	ofstream ofs(outputfile, std::ios::binary | std::ios::app);

	std::vector<uchar> buf;
	imencode(".ppm", out_mat, buf);

	// P6\nを無視
	ofs.write(reinterpret_cast<char *>(&(buf[3])), buf.size() * sizeof(uchar));

	return true;
}


void SavePpm(const string& filename, const Mat& output, const int div_x, const int div_y){
	writePpmHeader(filename, output, div_x, div_y); //ヘッダ編集
	writePpmImage(filename, output); //Matをバイナリで書き込み
}


int main(int argc, char* argv[]){
	const string windowName = "createPpm";

	/*画像ごとにパラメーターを変更(下４行)*/

	std::cout << "image file ----- ";
	const auto readfile = readFrom<std::string>(std::cin);
	std::cout << "output file ----- ";
	const auto outputfile = readFrom<std::string>(std::cin); //書き込み画像(決め打ち)
	std::cout << "div_x ---- ";
	const auto div_x = readFrom<size_t>(std::cin); //devideRand(); 横分割数(決め打ち)
	std::cout << "div_y ---- ";
	const auto div_y = readFrom<size_t>(std::cin); //devideRand(); 縦分割数(決め打ち)

	cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);

	auto divImg = [&](){
		// (1)画像を読み込む
		auto s_img = imread(readfile);

		// (2)画像を分割するための矩形を設定
		utils::Image img(s_img(Rect(0, 0, s_img.cols / div_x * div_x,
								 s_img.rows / div_y * div_y)));
		return utils::makeDividedImage(std::move(img), div_x, div_y);
	}();

	std::random_device seed_gen;
	std::mt19937 engine(seed_gen());
	std::uniform_int_distribution<size_t> distX(0, div_x-1),
	                                      distY(0, div_y-1);

	// (3)部分画像を入れ換える
	std::vector<std::vector<utils::ImageID>> idxs;
	idxs.reserve(div_y);

	for(size_t i = 0; i < div_y; ++i){
		idxs.emplace_back();

		auto& last = idxs[idxs.size()-1];
		last.reserve(div_x);
		for(size_t j = 0; j < div_x; ++j)
			last.emplace_back(i, j);
	}

	std::shuffle(idxs.begin(), idxs.end(), engine);
	for(auto& e: idxs)
		std::shuffle(e.begin(), e.end(), engine);

	auto swpImg = utils::SwappedImage(divImg, idxs);

	for (size_t i = 0; i < div_x * div_y * 2; i++){
		size_t p1x = distX(engine),
		       p2x = distX(engine),
		       p1y = distY(engine),
		       p2y = distY(engine);

		swpImg.swap_element(utils::makeIndex2D(p1y, p1x), utils::makeIndex2D(p2y, p2x));
	}

	auto rndMat = swpImg.cvMat();
	// (4)画像の表示
	imshow(windowName, rndMat);

	SavePpm(outputfile, rndMat, div_x, div_y); //問題形式でppm保存

	waitKey(0); // キー入力待ち

	return 0;
}
