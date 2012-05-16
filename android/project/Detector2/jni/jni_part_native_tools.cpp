#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <cv.h>
#include <highgui.h>
#include <vector>
#include "TextDetection.h"
#include "android_log.h"
#include "ru_cs_detector_tools_NativeTools.h"
#include "TextTracker.h"
#include <stdio.h>

extern "C" {

jboolean isNull(const CvRect rec) {
	return rec.x == 0 && rec.y == 0 && rec.width == 0 && rec.height == 0;
}
jboolean doTreck(JNIEnv* env, jint width, jint height, jbyteArray in,
		jintArray out, int mode);
JNIEXPORT jobjectArray JNICALL Java_ru_cs_detector_tools_NativeTools_getComp(
		JNIEnv *env, jclass clazz, jint width, jint height, jbyteArray in) {
	jbyte* inputData = env->GetByteArrayElements(in, 0);
	cv::Mat inputMatForCalc(height, width, CV_8UC1,
			(unsigned char *) inputData);
	IplImage image = inputMatForCalc;
	CvRect* regions;
	image.nChannels = 3;
	int kol = 0; //getComp(&image, &regions);
	jclass myCvRectClass = env->FindClass("ru/cs/detector/tools/MyCvRect");
	jobjectArray res = env->NewObjectArray(kol, myCvRectClass, 0);
	jmethodID idConstructor = env->GetMethodID(myCvRectClass, "<init>",
			"(IIII)V");
	for (int i = 0; i < kol; ++i) {
		CvRect tmp = regions[i];
		jobject myCvRect = env->NewObject(myCvRectClass, idConstructor, tmp.x,
				tmp.y, tmp.width, tmp.height);
		env->SetObjectArrayElement(res, i, myCvRect);
	}
}

//=================================

JNIEXPORT void JNICALL Java_ru_cs_detector_tools_NativeTools_releaseResources(
		JNIEnv* env, jclass clazz) {
	releaseResources();
}

JNIEXPORT void JNICALL Java_ru_cs_detector_tools_NativeTools_toRGBA(JNIEnv* env,
		jclass clazz, jint width, jint height, jbyteArray in, jintArray out) {
	jbyte* inputData = env->GetByteArrayElements(in, 0);
	jint* outputData = env->GetIntArrayElements(out, 0);
	cv::Mat inputMat(height + height / 2, width, CV_8UC1,
			(unsigned char *) inputData);
	cv::Mat outputMat(height, width, CV_8UC4, (unsigned char *) outputData);
	cvtColor(inputMat, outputMat, CV_YUV420sp2BGR, 4);
	env->ReleaseIntArrayElements(out, outputData, 0);
	env->ReleaseByteArrayElements(in, inputData, 0);
}
int numCapture;
int numTreck;
JNIEXPORT void JNICALL Java_ru_cs_detector_tools_NativeTools_initResources__II_3B_3I(
		JNIEnv* env, jclass clazz, jint width, jint height, jbyteArray in,
		jintArray out) {
	jbyte* inputData = env->GetByteArrayElements(in, 0);
	jint* outputData = env->GetIntArrayElements(out, 0);
	cv::Mat inputMat(height + height / 2, width, CV_8UC1,
			(unsigned char *) inputData);
	cv::Mat outputMat(height, width, CV_8UC4, (unsigned char *) outputData);

	cvtColor(inputMat, outputMat, CV_YUV420sp2BGR, 4);
	IplImage image = outputMat;
	initResources(&image);
	numCapture = 0;
	numTreck = 0;
	env->ReleaseIntArrayElements(out, outputData, 0);
	env->ReleaseByteArrayElements(in, inputData, 0);
}

JNIEXPORT jboolean JNICALL Java_ru_cs_detector_tools_NativeTools_captureText__II_3B_3I(
		JNIEnv* env, jclass clazz, jint width, jint height, jbyteArray in,
		jintArray out) {
	LOGI("CALL CAPTURE TEXT");
	return doTreck(env, width, height, in, out, 1);
}
JNIEXPORT jboolean JNICALL Java_ru_cs_detector_tools_NativeTools_trackText__II_3B_3I(
		JNIEnv* env, jclass clazz, jint width, jint height, jbyteArray in,
		jintArray out) {
	LOGI("CALL TRECKTEXT");
	return doTreck(env, width, height, in, out, 0);
}

jboolean doTreck(JNIEnv* env, jint width, jint height, jbyteArray in,
		jintArray out, int mode) {

	jbyte* inputData = env->GetByteArrayElements(in, 0);
	jint* outputData = env->GetIntArrayElements(out, 0);
	cv::Mat inputMat(height + height / 2, width, CV_8UC1,
			(unsigned char *) inputData);
	cv::Mat outputMat(height, width, CV_8UC4, (unsigned char *) outputData);
	cvtColor(inputMat, outputMat, CV_YUV420sp2BGR, 4);
	jint *forCalcData = new jint[height * width];
	cv::Mat inputMatForCalc(height, width, CV_8UC3,
			(unsigned char *) forCalcData);
	cvtColor(inputMat, inputMatForCalc, CV_YUV420sp2BGR, 3);
	IplImage frame = inputMatForCalc;
	CvRect rec = TextTracking(&frame, mode);
	rectangle(outputMat, rec, cv::Scalar(0, 0, 255, 255),5);
	env->ReleaseIntArrayElements(out, outputData, 0);
	env->ReleaseByteArrayElements(in, inputData, 0);
	delete[] forCalcData;
	return !isNull(rec);
}

}

