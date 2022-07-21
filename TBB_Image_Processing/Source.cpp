#include <iostream>
#include <vector>
//Thread building blocks library
#include <tbb/task_scheduler_init.h>
//Free Image library
#include <FreeImagePlus.h>

#include <thread>
#include <tbb/tbb.h>

using namespace std;
using namespace tbb;

int main()
{
	int nt = task_scheduler_init::default_num_threads();
	task_scheduler_init T(nt);

	//variables used throughout code declared here
	uint64_t width = 0;
	uint64_t height = 0;

	auto startTime = chrono::high_resolution_clock::now();
	auto endTime = chrono::high_resolution_clock::now();

	//Part 1 (Image Comparison): -----------------DO NOT REMOVE THIS COMMENT----------------------------//
	#pragma region Sequential Image Comparison
	
	cout << "Starting Sequential Image Comparison...\n";
	startTime = chrono::high_resolution_clock::now();

	//load images and convert to float images (greyscale 0.0->1.0 black->white)
	fipImage fTopImg;
	fTopImg.load("Images\\render_top_1.png");
	fTopImg.convertToFloat();

	width = fTopImg.getWidth();
	height = fTopImg.getHeight();

	fipImage sTopImg;
	sTopImg.load("Images\\render_top_2.png");
	sTopImg.convertToFloat();

	//Get the pointers to the start of the images' pixels to read their values
	const float* const fTopBuf = (float*)fTopImg.accessPixels();
	const float* const sTopBuf = (float*)sTopImg.accessPixels();

	fipImage topImg;
	topImg = fipImage(FIT_FLOAT, width, height, 32);
	float* topBuf = (float*)topImg.accessPixels();

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			//2d->1d index conversion, x + width * y, for every y level, increment by width.
			int i = x + width * y;
			//ternary statement for if both pixels in the loaded images match, the brightness of the created pixel is 0, if they do not match, the brightness is 1
			topBuf[i] = fTopBuf[i] == sTopBuf[i] ? 0.0f : 1.0f;
		}
	}

	fipImage fBotImg;
	fBotImg.load("Images\\render_bottom_1.png");
	fBotImg.convertToFloat();

	fipImage sBotImg;
	sBotImg.load("Images\\render_bottom_2.png");
	sBotImg.convertToFloat();

	const float* const fBotBuf = (float*)fBotImg.accessPixels();
	const float* const sBotBuf = (float*)sBotImg.accessPixels();

	fipImage botImg;
	botImg = fipImage(FIT_FLOAT, width, height, 32);
	float* botBuf = (float*)botImg.accessPixels();

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			int i = x + width * y;
			botBuf[i] = fBotBuf[i] == sBotBuf[i] ? 0.0f : 1.0f;
		}
	}

	fipImage combImg;
	combImg = fipImage(FIT_FLOAT, width, height, 32);
	float* combBuf = (float*)combImg.accessPixels();

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			int i = x + width * y;
			//Get half of the value of the pixels in the current index and combine the values in the new pixel (this will only ever result in 0 OR 0.5f)
			float t = topBuf[i] / 2;
			float b = botBuf[i] / 2;
			combBuf[i] = t+b;
		}
	}

	//Convert the images to 24Bit Bitmap images and save them
	topImg.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	botImg.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	topImg.convertTo24Bits();
	botImg.convertTo24Bits();
	topImg.save("stage1_top_1.png", 0);
	botImg.save("stage1_bottom_1.png");

	combImg.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	combImg.convertTo24Bits();
	combImg.save("stage1_combined_1.png");

	endTime = chrono::high_resolution_clock::now();

	cout << "Sequential Image Comparison took: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << " milliseconds\n";

	#pragma endregion

	#pragma region Multithreaded Image Comparison (c++11)
	
	cout << "Starting Multithreaded Image Comparison...\n";
	startTime = chrono::high_resolution_clock::now();

	//Lambda functions of the sequential code to allow the top and bottom images to be processed at the same time
	auto topImgProc = []() {
		fipImage fTopImg;
		fTopImg.load("Images\\render_top_1.png");
		fTopImg.convertToFloat();

		uint64_t width = fTopImg.getWidth();
		uint64_t height = fTopImg.getHeight();

		fipImage sTopImg;
		sTopImg.load("Images\\render_top_2.png");
		sTopImg.convertToFloat();

		const float* const fTopBuf = (float*)fTopImg.accessPixels();
		const float* const sTopBuf = (float*)sTopImg.accessPixels();

		fipImage topImg;
		topImg = fipImage(FIT_FLOAT, width, height, 32);
		float* topBuf = (float*)topImg.accessPixels();

		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				int i = x + width * y;
				topBuf[i] = fTopBuf[i] == sTopBuf[i] ? 0.0f : 1.0f;
			}
		}

		topImg.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
		topImg.convertTo24Bits();
		topImg.save("stage1_top_2.png", 0);
	};

	auto botImgProc = []() {
		fipImage fBotImg;
		fBotImg.load("Images\\render_bottom_1.png");
		fBotImg.convertToFloat();

		uint64_t width = fBotImg.getWidth();
		uint64_t height = fBotImg.getHeight();

		fipImage sBotImg;
		sBotImg.load("Images\\render_bottom_2.png");
		sBotImg.convertToFloat();

		const float* const fBotBuf = (float*)fBotImg.accessPixels();
		const float* const sBotBuf = (float*)sBotImg.accessPixels();

		fipImage botImg;
		botImg = fipImage(FIT_FLOAT, width, height, 32);
		float* botBuf = (float*)botImg.accessPixels();

		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				int i = x + width * y;
				botBuf[i] = fBotBuf[i] == sBotBuf[i] ? 0.0f : 1.0f;
			}
		}

		botImg.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
		botImg.convertTo24Bits();
		botImg.save("stage1_bottom_2.png");
	};

	auto combImgProc = []() {
		fipImage topImg;
		topImg.load("stage1_top_2.png");
		topImg.convertToFloat();
		float* topBuf = (float*)topImg.accessPixels();

		uint64_t width = topImg.getWidth();
		uint64_t height = topImg.getHeight();

		fipImage botImg;
		botImg.load("stage1_bottom_2.png");
		botImg.convertToFloat();
		float* botBuf = (float*)botImg.accessPixels();

		fipImage combImg;
		combImg = fipImage(FIT_FLOAT, width, height, 32);
		float* combBuf = (float*)combImg.accessPixels();

		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				int i = x + width * y;
				float t = topBuf[i] / 2;
				float b = botBuf[i] / 2;
				combBuf[i] = t + b;
			}
		}

		combImg.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
		combImg.convertTo24Bits();
		combImg.save("stage1_combined_2.png");
	};

	//Create the top&bottom image processor threads and wait for them to end and create the combined image processor thread (This lets the bottom and top images to be created before combining them)
	thread topImgThread(topImgProc);
	thread botImgThread(botImgProc);
	topImgThread.join();
	botImgThread.join();
	thread combImgThread(combImgProc);
	combImgThread.join();

	endTime = chrono::high_resolution_clock::now();

	cout << "Multithreaded Image Comparison took: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << " milliseconds\n";

	#pragma endregion
	
	//Part 2 (Blur & post-processing): -----------DO NOT REMOVE THIS COMMENT----------------------------//
	//CREATE GAUSSIAN MATRIX & PARSE WITH SIGMA TO GAUSSIAN BLUR FUNCTION
	#pragma region Sequential Blur & Post Processing
	
	cout << "Starting Sequential Blur & Post Processing...\n";
	startTime = chrono::high_resolution_clock::now();
	
	fipImage baseImg;
	baseImg.load("stage1_combined_1.png");
	baseImg.convertToFloat();
	float* baseBuf = (float*)baseImg.accessPixels();

	width = baseImg.getWidth();
	height = baseImg.getHeight();

	fipImage blurImg;
	blurImg = fipImage(FIT_FLOAT, width, height, 32);
	float* blurBuf = (float*)blurImg.accessPixels();

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			//Apply the gaussian blur to each pixel in the image, easily decypherable while the code is split up like this
			float pixelVal = 0.0f;

			if (y > 0 && x > 0) {
				pixelVal += baseBuf[(y - 1) * width + (x - 1)] * 0.102059f;
			}
			if (y > 0) {
				pixelVal += baseBuf[(y - 1) * width + x] * 0.115349f;
			}
			if (y > 0 && x < width) {
				pixelVal += baseBuf[(y - 1) * width + (x + 1)] * 0.102059f;
			}

			if (x > 0) {
				pixelVal += baseBuf[y * width + (x - 1)] * 0.115349f;
			}
			pixelVal += baseBuf[y * width + x] * 0.130371f;
			if (x < width) {
				pixelVal += baseBuf[y * width + (x + 1)] * 0.115349;
			}

			if (y < height - 1 && x > 0) {
				pixelVal += baseBuf[(y + 1) * width + (x - 1)] * 0.102059;
			}
			if (y < height - 1) {
				pixelVal += baseBuf[(y + 1) * width + x] * 0.115349;
			}
			if (y < height - 1 && x < width) {
				pixelVal += baseBuf[(y + 1) * width + (x + 1)] * 0.102059;
			}

			blurBuf[y * width + x] = pixelVal;
		}
	}

	fipImage threshImg;
	threshImg = fipImage(FIT_FLOAT, width, height, 32);
	float* threshBuf = (float*)threshImg.accessPixels();

	for (auto y = 0; y < height; ++y) {
		for (auto x = 0; x < width; ++x) {
			//If the current pixel is not black, make it fully white, else make it black.
			if (blurBuf[y * width + x] != 0.0f) {
				threshBuf[y * width + x] = 1.0f;
			}
			else {
				threshBuf[y * width + x] = 0.0f;
			}
		}
	}

	endTime = chrono::high_resolution_clock::now();

	cout << "Sequential Blur & Post Processing took: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << " milliseconds\n";

	blurImg.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	threshImg.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	blurImg.convertTo24Bits();
	threshImg.convertTo24Bits();
	blurImg.save("stage2_blurred_1.png");
	threshImg.save("stage2_threshold_1.png");

	baseImg = NULL;
	blurImg = NULL;
	threshImg = NULL;
	baseBuf = nullptr;
	blurBuf = nullptr;
	threshBuf = nullptr;

	#pragma endregion

	#pragma region TBB Blur & Post Processing
	
	cout << "Starting Multithreaded Blue & Post Processing...\n";
	startTime = chrono::high_resolution_clock::now();

	baseImg.load("stage1_combined_2.png");
	baseImg.convertToFloat();
	baseBuf = (float*)baseImg.accessPixels();

	width = baseImg.getWidth();
	height = baseImg.getHeight();

	blurImg = fipImage(FIT_FLOAT, width, height, 32);
	blurBuf = (float*)blurImg.accessPixels();

	//setup the parallel for loop using blocked ranges 1->height-1 and 1->width-1
	parallel_for(blocked_range2d<uint64_t, uint64_t>(1, height - 1, 1, width - 1), [&](const blocked_range2d<uint64_t, uint64_t>& r) {
		//set variables to be used in the for loops to process the image
		auto ybegin = r.rows().begin();
		auto yend = r.rows().end();
		auto xbegin = r.cols().begin();
		auto xend = r.cols().end();

		for (auto y = ybegin; y < yend; ++y) {
			for (auto x = xbegin; x < xend; ++x) {
				float pixelVal = 0.0f;

				//if statements to avoid out of bounds and weird blur application at points (corners & edges)
				//using a generated Gaussian Matrix, apply the blur to each pixel of the image
				if (y > 0 && x > 0) {
					pixelVal += baseBuf[(y - 1) * width + (x - 1)] * 0.102059f;
				}
				if (y > 0) {
					pixelVal += baseBuf[(y - 1) * width + x] * 0.115349f;
				}
				if (y > 0 && x < width) {
					pixelVal += baseBuf[(y - 1) * width + (x + 1)] * 0.102059f;
				}

				if (x > 0) {
					pixelVal += baseBuf[y * width + (x - 1)] * 0.115349f;
				}
				pixelVal += baseBuf[y * width + x] * 0.130371f;
				if (x < width) {
					pixelVal += baseBuf[y * width + (x + 1)] * 0.115349;
				}

				if (y < height - 1 && x > 0) {
					pixelVal += baseBuf[(y + 1) * width + (x - 1)] * 0.102059;
				}
				if (y < height - 1) {
					pixelVal += baseBuf[(y + 1) * width + x] * 0.115349;
				}
				if (y < height - 1 && x < width) {
					pixelVal += baseBuf[(y + 1) * width + (x + 1)] * 0.102059;
				}

				blurBuf[y * width + x] = pixelVal;
			}
		}
	});

	threshImg = fipImage(FIT_FLOAT, width, height, 32);
	threshBuf = (float*)threshImg.accessPixels();

	parallel_for(blocked_range2d<uint64_t, uint64_t>(1, height - 1, 1, width - 1), [&](const blocked_range2d<uint64_t, uint64_t>& r) {
		auto ybegin = r.rows().begin();
		auto yend = r.rows().end();
		auto xbegin = r.cols().begin();
		auto xend = r.cols().end();

		for (auto y = ybegin; y < yend; ++y) {
			for (auto x = xbegin; x < xend; ++x) {
				if (blurBuf[y * width + x] != 0.0f) {
					threshBuf[y * width + x] = 1.0f;
				}
				else {
					threshBuf[y * width + x] = 0.0f;
				}
			}
		}
	});

	endTime = chrono::high_resolution_clock::now();

	cout << "Multithreaded Blur & Post Processing took: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << " milliseconds\n";

	blurImg.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	threshImg.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	blurImg.convertTo24Bits();
	threshImg.convertTo24Bits();
	blurImg.save("stage2_blurred_2.png");
	threshImg.save("stage2_threshold_2.png");

	blurImg = NULL;
	threshImg = NULL;
	blurBuf = nullptr;
	threshBuf = nullptr;
	
	#pragma endregion

	//Part 3 (Image Mask): -----------------------DO NOT REMOVE THIS COMMENT----------------------------//

	#pragma region Sequential Image Mask
	
	cout << "Starting Sequential Image Mask...\n";
	startTime = chrono::high_resolution_clock::now();

	topImg.load("Images\\render_top_1.png");

	width = topImg.getWidth();
	height = topImg.getHeight();

	fipImage newTopImg;
	newTopImg = fipImage(FIT_BITMAP, width, height, topImg.getBitsPerPixel());

	threshImg.load("stage2_threshold_1.png");
	threshImg.convertToFloat();
	threshBuf = (float*)threshImg.accessPixels();

	//uint64_t used to avoid overflows on these specific images (if extremely large images are expected, other types may have to be used with other workarounds (quadrants?))
	uint64_t whitePixels = 0;
	uint64_t totalPixels = (threshImg.getWidth() * threshImg.getHeight()) / 1000; //division used to avoid overflows (I don't know if this matters)

	for (auto y = 0; y < height; y++) {
		for (auto x = 0; x < width; x++) {
			RGBQUAD orgColor;
			topImg.getPixelColor(x, y, &orgColor);
			if (threshBuf[(y * width) + x] == 1.0f) {
				whitePixels++;
				//invert each rgb value of the current pixel (255 - current pixel colour value) if the pixel in the threshold is white, else, use the current colour
				RGBQUAD newColor;
				newColor.rgbRed = 255 - orgColor.rgbRed;
				newColor.rgbGreen = 255 - orgColor.rgbGreen;
				newColor.rgbBlue = 255 - orgColor.rgbBlue;
				newTopImg.setPixelColor(x, y, &newColor);
			}
			else {
				newTopImg.setPixelColor(x, y, &orgColor);
			}
		}
	}

	//cast whitePixels to a double and divide by 1000 to match totalPixels division
	float percentWhite = (((double)whitePixels / 1000) / totalPixels) * 100;

	cout << percentWhite << "% White" << endl;

	endTime = chrono::high_resolution_clock::now();

	cout << "Sequential Image Mask took: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << " milliseconds\n";

	newTopImg.save("newtop_1.png");

	
	#pragma endregion

	#pragma region TBB Image Mask
	
	cout << "Starting Multithreaded Image Mask...\n";
	startTime = chrono::high_resolution_clock::now();

	topImg.load("Images\\render_top_1.png");

	width = topImg.getWidth();
	height = topImg.getHeight();

	newTopImg = fipImage(FIT_BITMAP, width, height, topImg.getBitsPerPixel());

	threshImg.load("stage2_threshold_2.png");
	threshImg.convertToFloat();
	threshBuf = (float*)threshImg.accessPixels();

	whitePixels = 0;
	totalPixels = (threshImg.getWidth() * threshImg.getHeight()) / 1000;

	spin_mutex counterMutex;

	parallel_for(blocked_range2d<uint64_t, uint64_t>(1, height - 1, 1, width - 1), [&](const blocked_range2d<uint64_t, uint64_t>& r) {
		auto ybegin = r.rows().begin();
		auto yend = r.rows().end();
		auto xbegin = r.cols().begin();
		auto xend = r.cols().end();

		for (auto y = ybegin; y < yend; y++) {
			for (auto x = xbegin; x < xend; x++) {
				RGBQUAD orgColor;
				topImg.getPixelColor(x, y, &orgColor);
				if (threshBuf[(y * width) + x] == 1.0f) {
					{ //mutex used to stop race conditions between other threads with access to the shared variable whitePixels
					spin_mutex::scoped_lock lock(counterMutex);
					whitePixels++;
					}
					RGBQUAD newColor;
					newColor.rgbRed = 255 - orgColor.rgbRed;
					newColor.rgbGreen = 255 - orgColor.rgbGreen;
					newColor.rgbBlue = 255 - orgColor.rgbBlue;
					newTopImg.setPixelColor(x, y, &newColor);
				} else {
					newTopImg.setPixelColor(x, y, &orgColor);
				}
			}
		}
	});

	percentWhite = (((double)whitePixels / 1000) / totalPixels) * 100;

	cout << percentWhite << "% White pixels" << endl;

	endTime = chrono::high_resolution_clock::now();

	cout << "Multithreaded Image Mask took: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << " milliseconds\n";

	newTopImg.save("newtop_2.png");

	#pragma endregion


	return 0;
}