#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "opencv2/highgui/highgui.hpp"
#include <cv.h>
#include <highgui.h>
#include <vector>
#include "TextDetection.h"
#include "android_log.h"
#include "ru_cs_detector_view_SampleView.h"

#include <stdio.h>
#include <stdlib.h>
#include "TextTracker.h"
using namespace std;
using namespace cv;

extern "C" {

bool isNull(const CvRect rec) {
	return rec.x == 0 && rec.y == 0 && rec.width == 0 && rec.height == 0;
}

void printProp(const IplImage &image2);
int num = 1;
JNIEXPORT void JNICALL Java_ru_cs_detector_view_SampleView_FindFeatures(
		JNIEnv* env, jobject thiz, jint width, jint height, jbyteArray in,
		jintArray out) {
	jbyte* inputData = env->GetByteArrayElements(in, 0);
	jint* outputData = env->GetIntArrayElements(out, 0);

	Mat inputMat(height + height / 2, width, CV_8UC1,
			(unsigned char *) inputData);
	Mat outputMat(height, width, CV_8UC4, (unsigned char *) outputData);
	jint *forCalcData = new jint[height * width];

	Mat inputMatForCalc(height, width, CV_8UC3, (unsigned char *) forCalcData);

	//Please make attention about BGRA byte order
	//ARGB stored in java as int array becomes BGRA at native level
	cvtColor(inputMat, outputMat, CV_YUV420sp2BGR, 4);
	cvtColor(inputMat, inputMatForCalc, CV_YUV420sp2BGR, 3); //todo
	char b[256];
	const IplImage image = inputMatForCalc;


	sprintf(b,"/sdcard/Dump/%d.jpg",num++);
	cvSaveImage(b, &image);
	delete[] forCalcData;
	env->ReleaseIntArrayElements(out, outputData, 0);
	env->ReleaseByteArrayElements(in, inputData, 0);
}

JNIEXPORT void JNICALL Java_ru_cs_detector_view_SampleView_initResources(
		JNIEnv* env, jobject thiz, jint width, jint height, jbyteArray in) {
	num = 0;
	jbyte* inputData = env->GetByteArrayElements(in, 0);
	Mat inputMat(height + height / 2, width, CV_8UC1,
			(unsigned char *) inputData);
	jint *data = new jint[height * width];
	Mat mat(height, width, CV_8UC3, data);
	cvtColor(inputMat, mat, CV_YUV420sp2BGR, 3);
	IplImage image = mat;
	initResources(&image);
	env->ReleaseByteArrayElements(in, inputData, 0);
}
int mode = 1;

JNIEXPORT void JNICALL Java_ru_cs_detector_view_SampleView_textTracking(
		JNIEnv* env, jobject thiz, jint width, jint height, jbyteArray in,
		jintArray out) {
	jbyte* inputData = env->GetByteArrayElements(in, 0);
	jint* outputData = env->GetIntArrayElements(out, 0);

	Mat inputMat(height + height / 2, width, CV_8UC1,
			(unsigned char *) inputData);
	Mat outputMat(height, width, CV_8UC4, (unsigned char *) outputData);
	jint *forCalcData = new jint[height * width];

	Mat inputMatForCalc(height, width, CV_8UC3, (unsigned char *) forCalcData);

	//Please make attention about BGRA byte order
	//ARGB stored in java as int array becomes BGRA at native level
	cvtColor(inputMat, outputMat, CV_YUV420sp2BGR, 4);
	cvtColor(inputMat, inputMatForCalc, CV_YUV420sp2BGR, 3);

	IplImage frame = inputMatForCalc;
	CvRect rec = TextTracking(&frame, mode);
	if (isNull(rec)) {
		mode = 1;
	} else {
		rectangle(outputMat, rec, Scalar(0, 0, 255, 255));
		mode = 0;
	}
	delete[] forCalcData;
	env->ReleaseIntArrayElements(out, outputData, 0);
	env->ReleaseByteArrayElements(in, inputData, 0);
}

void printProp(const IplImage &image2) {
	LOGI("------------------------------");
	char b[256];

	sprintf(b, "size %d", image2.nSize);
	LOGI(b);

	sprintf(b, "ID %d", image2.ID);
	LOGI(b);

	sprintf(b, "nChannels %d", image2.nChannels);
	LOGI(b);

	sprintf(b, "alphaChannel %d", image2.alphaChannel);
	LOGI(b);

	sprintf(b, "depth %d", image2.depth);
	LOGI(b);

	sprintf(b, "dataOrder %d", image2.dataOrder);
	LOGI(b);

	sprintf(b, "origin %d\n", image2.origin);
	LOGI(b);

	sprintf(b, "align %d", image2.align);
	LOGI(b);

	sprintf(b, "width %d", image2.width);
	LOGI(b);

	sprintf(b, "height %d", image2.height);
	LOGI(b);

	sprintf(b, "imageSize %d", image2.imageSize);
	LOGI(b);

	sprintf(b, "widthStep %d", image2.widthStep);
	LOGI(b);

	sprintf(b, "iimageDataOrigin %d", image2.imageDataOrigin);
	LOGI(b);

	LOGI("------------------------------");
}

}
