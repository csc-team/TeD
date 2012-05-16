//источник - https://sites.google.com/site/roboticssaurav/strokewidthnokia
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/unordered_map.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

#include <cassert>
#include <cmath>
#include <iostream>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include <math.h>
#include <time.h>
#include <utility>
#include <algorithm>
#include <vector>
#include <limits.h>
#include <sys/time.h>
#include "TextDetection.h"
//#include "android_log.h"//todo

#define PI 3.14159265


std::vector<std::pair<CvPoint,CvPoint> > findBoundingBoxes( std::vector<std::vector<Point2d> > & components,
                                                           std::vector<Chain> & chains,
                                                           std::vector<std::pair<Point2d,Point2d> > & compBB,
                                                           IplImage * output) {
    std::vector<std::pair<CvPoint,CvPoint> > bb;
    bb.reserve(chains.size());
    for (std::vector<Chain>::iterator chainit = chains.begin(); chainit != chains.end(); chainit++) {
        int minx = output->width;
        int miny = output->height;
        int maxx = 0;
        int maxy = 0;
        for (std::vector<int>::const_iterator cit = chainit->components.begin(); cit != chainit->components.end(); cit++) {
                miny = std::min(miny,compBB[*cit].first.y);
                minx = std::min(minx,compBB[*cit].first.x);
                maxy = std::max(maxy,compBB[*cit].second.y);
                maxx = std::max(maxx,compBB[*cit].second.x);
        }
        CvPoint p0 = cvPoint(minx,miny);
        CvPoint p1 = cvPoint(maxx,maxy);
        std::pair<CvPoint,CvPoint> pair(p0,p1);
        bb.push_back(pair);
    }
    return bb;
}

std::vector<std::pair<CvPoint,CvPoint> > findBoundingBoxes( std::vector<std::vector<Point2d> > & components,
                                                           IplImage * output) {
    std::vector<std::pair<CvPoint,CvPoint> > bb;
    bb.reserve(components.size());
    for (std::vector<std::vector<Point2d> >::iterator compit = components.begin(); compit != components.end(); compit++) {
        int minx = output->width;
        int miny = output->height;
        int maxx = 0;
        int maxy = 0;
        for (std::vector<Point2d>::iterator it = compit->begin(); it != compit->end(); it++) {
                miny = std::min(miny,it->y);
                minx = std::min(minx,it->x);
                maxy = std::max(maxy,it->y);
                maxx = std::max(maxx,it->x);
        }
        CvPoint p0 = cvPoint(minx,miny);
        CvPoint p1 = cvPoint(maxx,maxy);
        std::pair<CvPoint,CvPoint> pair(p0,p1);
        bb.push_back(pair);
    }
    return bb;
}

void normalizeImage (IplImage * input, IplImage * output) {
    assert ( input->depth == IPL_DEPTH_32F );
    assert ( input->nChannels == 1 );
    assert ( output->depth == IPL_DEPTH_32F );
    assert ( output->nChannels == 1 );
    float maxVal = 0;
    float minVal = 1e100;
    for( int row = 0; row < input->height; row++ ){
        const float* ptr = (const float*)(input->imageData + row * input->widthStep);
        for ( int col = 0; col < input->width; col++ ){
            if (*ptr < 0) { }
            else {
                maxVal = std::max(*ptr, maxVal);
                minVal = std::min(*ptr, minVal);
            }
            ptr++;
        }
    }

    float difference = maxVal - minVal;
    for( int row = 0; row < input->height; row++ ){
        const float* ptrin = (const float*)(input->imageData + row * input->widthStep);\
        float* ptrout = (float*)(output->imageData + row * output->widthStep);\
        for ( int col = 0; col < input->width; col++ ){
            if (*ptrin < 0) {
                *ptrout = 1;
            } else {
                *ptrout = ((*ptrin) - minVal)/difference;
            }
            ptrout++;
            ptrin++;
        }
    }
}

void renderComponents (IplImage * SWTImage, std::vector<std::vector<Point2d> > & components, IplImage * output) {
    for (std::vector<std::vector<Point2d> >::iterator it = components.begin(); it != components.end();it++) {
        for (std::vector<Point2d>::iterator pit = it->begin(); pit != it->end(); pit++) {
            CV_IMAGE_ELEM(output, float, pit->y, pit->x) = CV_IMAGE_ELEM(SWTImage, float, pit->y, pit->x);
        }
    }
    for( int row = 0; row < output->height; row++ ){
        float* ptr = (float*)(output->imageData + row * output->widthStep);
        for ( int col = 0; col < output->width; col++ ){
            if (*ptr == 0) {
                *ptr = -1;
            }
            ptr++;
        }
    }
    float maxVal = 0;
    float minVal = 1e100;
    for( int row = 0; row < output->height; row++ ){
        const float* ptr = (const float*)(output->imageData + row * output->widthStep);
        for ( int col = 0; col < output->width; col++ ){
            if (*ptr == 0) { }
            else {
                maxVal = std::max(*ptr, maxVal);
                minVal = std::min(*ptr, minVal);
            }
            ptr++;
        }
    }
    float difference = maxVal - minVal;
    for( int row = 0; row < output->height; row++ ){
        float* ptr = (float*)(output->imageData + row * output->widthStep);\
        for ( int col = 0; col < output->width; col++ ){
            if (*ptr < 1) {
                *ptr = 1;
            } else {
                *ptr = ((*ptr) - minVal)/difference;
            }
            ptr++;
        }
    }

}

void renderComponentsWithBoxes (IplImage * SWTImage, std::vector<std::vector<Point2d> > & components,
                                std::vector<std::pair<Point2d,Point2d> > & compBB, IplImage * output) {
    IplImage * outTemp =
            cvCreateImage ( cvGetSize ( output ), IPL_DEPTH_32F, 1 );

    renderComponents(SWTImage,components,outTemp);
    std::vector<std::pair<CvPoint,CvPoint> > bb;
    bb.reserve(compBB.size());
    for (std::vector<std::pair<Point2d,Point2d> >::iterator it=compBB.begin(); it != compBB.end(); it++ ) {
        CvPoint p0 = cvPoint(it->first.x,it->first.y);
        CvPoint p1 = cvPoint(it->second.x,it->second.y);
        std::pair<CvPoint,CvPoint> pair(p0,p1);
        bb.push_back(pair);
    }

    IplImage * out =
            cvCreateImage ( cvGetSize ( output ), IPL_DEPTH_8U, 1 );
    cvConvertScale(outTemp, out, 255, 0);
    cvCvtColor (out, output, CV_GRAY2RGB);
    //cvReleaseImage ( &outTemp );
    //cvReleaseImage ( &out );

    int count = 0;
    for (std::vector<std::pair<CvPoint,CvPoint> >::iterator it= bb.begin(); it != bb.end(); it++) {
        CvScalar c;
        if (count % 3 == 0) c=cvScalar(255,0,0);
        else if (count % 3 == 1) c=cvScalar(0,255,0);
        else c=cvScalar(0,0,255);
        count++;
        cvRectangle(output,it->first,it->second,c,2);
    }
}

void renderChainsWithBoxes (IplImage * SWTImage,
                   std::vector<std::vector<Point2d> > & components,
                   std::vector<Chain> & chains,
                   std::vector<std::pair<Point2d,Point2d> > & compBB,
                   IplImage * output) {
    // keep track of included components
    std::vector<bool> included;
    included.reserve(components.size());
    for (unsigned int i = 0; i != components.size(); i++) {
        included.push_back(false);
    }
    for (std::vector<Chain>::iterator it = chains.begin(); it != chains.end();it++) {
        for (std::vector<int>::iterator cit = it->components.begin(); cit != it->components.end(); cit++) {
            included[*cit] = true;
        }
    }
    std::vector<std::vector<Point2d> > componentsRed;
    for (unsigned int i = 0; i != components.size(); i++ ) {
        if (included[i]) {
            componentsRed.push_back(components[i]);
        }
    }
    IplImage * outTemp =
            cvCreateImage ( cvGetSize ( output ), IPL_DEPTH_32F, 1 );

    //std::cout << componentsRed.size() << " components after chaining" << std::endl;
    renderComponents(SWTImage,componentsRed,outTemp);
    std::vector<std::pair<CvPoint,CvPoint> > bb;
    bb = findBoundingBoxes(components, chains, compBB, outTemp);

    IplImage * out =
            cvCreateImage ( cvGetSize ( output ), IPL_DEPTH_8U, 1 );
    cvConvertScale(outTemp, out, 255, 0);
    cvCvtColor (out, output, CV_GRAY2RGB);
    cvReleaseImage ( &out );
    cvReleaseImage ( &outTemp);

    int count = 0;
    for (std::vector<std::pair<CvPoint,CvPoint> >::iterator it= bb.begin(); it != bb.end(); it++) {
        CvScalar c;
        if (count % 3 == 0) c=cvScalar(255,0,0);
        else if (count % 3 == 1) c=cvScalar(0,255,0);
        else c=cvScalar(0,0,255);
        count++;
        cvRectangle(output,it->first,it->second,c,2);
    }
}

void renderChains (IplImage * SWTImage,
                   std::vector<std::vector<Point2d> > & components,
                   std::vector<Chain> & chains,
                   IplImage * output) {
    // keep track of included components
    std::vector<bool> included;
    included.reserve(components.size());
    for (unsigned int i = 0; i != components.size(); i++) {
        included.push_back(false);
    }
    for (std::vector<Chain>::iterator it = chains.begin(); it != chains.end();it++) {
        for (std::vector<int>::iterator cit = it->components.begin(); cit != it->components.end(); cit++) {
            included[*cit] = true;
        }
    }
    std::vector<std::vector<Point2d> > componentsRed;
    for (unsigned int i = 0; i != components.size(); i++ ) {
        if (included[i]) {
            componentsRed.push_back(components[i]);
        }
    }
    //std::cout << componentsRed.size() << " components after chaining" << std::endl;
    IplImage * outTemp =
            cvCreateImage ( cvGetSize ( output ), IPL_DEPTH_32F, 1 );
    renderComponents(SWTImage,componentsRed,outTemp);
    cvConvertScale(outTemp, output, 255, 0);

}


void renderChains (IplImage * SWTImage,
                   std::vector<std::vector<Point2d> > & components,
                   std::vector<Chain> & chains,
                   std::vector<std::pair<Point2d,Point2d> > & compBB,
                   std::vector<std::pair<CvPoint,CvPoint> > & bb,
                   IplImage * output) {
    // keep track of included components
    std::vector<bool> included;
    included.reserve(components.size());
    for (unsigned int i = 0; i != components.size(); i++) {
        included.push_back(false);
    }
    for (std::vector<Chain>::iterator it = chains.begin(); it != chains.end();it++) {
        for (std::vector<int>::iterator cit = it->components.begin(); cit != it->components.end(); cit++) {
            included[*cit] = true;
        }
    }
    std::vector<std::vector<Point2d> > componentsRed;
    for (unsigned int i = 0; i != components.size(); i++ ) {
        if (included[i]) {
            componentsRed.push_back(components[i]);
        }
    }
    //std::cout << componentsRed.size() << " components after chaining" << std::endl;
    IplImage * outTemp =
            cvCreateImage ( cvGetSize ( output ), IPL_DEPTH_32F, 1 );
    renderComponents(SWTImage,componentsRed,outTemp);

    bb = findBoundingBoxes(components, chains, compBB, outTemp);

    IplImage * out =
            cvCreateImage ( cvGetSize ( output ), IPL_DEPTH_8U, 1 );
    cvConvertScale(outTemp, out, 255, 0);
    cvCvtColor (out, output, CV_GRAY2RGB);
    cvReleaseImage ( &out );
    cvReleaseImage ( &outTemp);
}

inline int intersect (int a, int b, int c, int d) {
	if (a > b)  std::swap (a, b);
	if (c > d)  std::swap (c, d);
	return std::min(b,d) - std::max(a,c);
}

bool addRectToArea(std::pair<std::pair<CvPoint,CvPoint>, int>  &compLines0, std::pair<CvPoint,CvPoint> &compLines1) {

	CvPoint p00 = compLines0.first.first;
	CvPoint p01 = compLines0.first.second;
	CvPoint p10 = compLines1.first;
	CvPoint p11 = compLines1.second;

	int intersectx = intersect (p00.y, p01.y, p10.y, p11.y);
	int intersecty = intersect (p00.x, p01.x, p10.x, p11.x);
	
	if ( (intersectx > 0) && (intersecty > 0) && (intersectx * intersecty > 0.5 * abs(p11.x - p10.x) * abs(p11.y - p10.y)) ) {

		compLines0.second ++;

		if (p00.x > p10.x) compLines0.first.first.x = p10.x;
		if (p00.y > p10.y) compLines0.first.first.y = p10.y;
		if (p01.x < p11.x) compLines0.first.second.x = p11.x;
		if (p01.y < p11.y) compLines0.first.second.y = p11.y;
		return true;
	}

	return false;
}


CvRect getRegion (IplImage* input) {

   
    std::vector<std::pair<CvPoint,CvPoint> > lightbb;
    std::vector<std::pair<CvPoint,CvPoint> > darkbb;

    getChains(input, darkbb, 1);
    getChains(input, lightbb, 0);

    std::list< std::pair<std::pair<CvPoint,CvPoint>, int> > darkCompLines;
    std::list< std::pair<std::pair<CvPoint,CvPoint>, int> > lightCompLines;

    for (std::vector<std::pair<CvPoint,CvPoint> >::iterator bb = darkbb.begin(); bb != darkbb.end(); ++bb) { 
        int k = 0;
	for (std::list< std::pair<std::pair<CvPoint,CvPoint>, int> >::iterator it = darkCompLines.begin(); it != darkCompLines.end(); ++it) {
		if  (addRectToArea((*it), (*bb) )) k++;
	}
	if (k == 0) {
		std::pair<std::pair<CvPoint,CvPoint>, int> pair((*bb), 1);
		darkCompLines.push_back(pair);
	}
   }

    for (std::vector<std::pair<CvPoint,CvPoint> >::iterator bb = lightbb.begin(); bb != lightbb.end(); ++bb) { 
        int k = 0;
	for (std::list< std::pair<std::pair<CvPoint,CvPoint>, int> >::iterator it = lightCompLines.begin(); it != lightCompLines.end(); ++it) {
		if  (addRectToArea((*it), (*bb) )) k++;
	}
	if (k == 0) {
		std::pair<std::pair<CvPoint,CvPoint>, int> pair((*bb), 1);
		lightCompLines.push_back(pair);
	}
   }

    int maxArea = 0;
    CvRect r;
    r.x = 0;
    r.y = 0;
    r.width = 0;
    r.height = 0;

     //CvScalar c = cvScalar(0, 0, 255);

    for (std::list< std::pair<std::pair<CvPoint,CvPoint>, int> >::iterator it = lightCompLines.begin(); it != lightCompLines.end(); it++) {

        //cvRectangle(input ,cvPoint(it->first.first.x, it->first.first.y), cvPoint(it->first.second.x, it->first.second.y  ), c, 2);

        if (abs((it->first.second.x - it->first.first.x) * (it->first.second.y - it->first.first.y)) > maxArea) {
            maxArea = abs((it->first.second.x - it->first.first.x) * (it->first.second.y - it->first.first.y));
	    r.x = it->first.first.x;
            r.y = it->first.first.y;
            r.width = abs(it->first.second.x - it->first.first.x);
            r.height = abs(it->first.second.y - it->first.first.y);

	}
    }

    for (std::list< std::pair<std::pair<CvPoint,CvPoint>, int> >::iterator it = darkCompLines.begin(); it != darkCompLines.end(); it++) {

        //cvRectangle(input ,cvPoint(it->first.first.x, it->first.first.y), cvPoint(it->first.second.x, it->first.second.y  ), c, 2);

        if (abs((it->first.second.x - it->first.first.x) * (it->first.second.y - it->first.first.y)) > maxArea) {
            maxArea = abs((it->first.second.x - it->first.first.x) * (it->first.second.y - it->first.first.y));
	    r.x = it->first.first.x;
            r.y = it->first.first.y;
            r.width = abs(it->first.second.x - it->first.first.x);
            r.height = abs(it->first.second.y - it->first.first.y);

	}
    }

    return r;


}

void getChains(	IplImage* input, 
			std::vector<std::pair<CvPoint,CvPoint> > &bb,
			bool dark_on_light)
{
    assert ( input->depth == IPL_DEPTH_8U );
    assert ( input->nChannels == 3 );

    IplImage * grayImage =
            cvCreateImage( cvGetSize ( input ), IPL_DEPTH_8U, 1 );
    cvCvtColor ( input, grayImage, CV_RGB2GRAY );

    double threshold_low = 175;
    double threshold_high = 320;
    IplImage * edgeImage =
            cvCreateImage( cvGetSize (input),IPL_DEPTH_8U, 1 );
    cvCanny(grayImage, edgeImage, threshold_low, threshold_high, 3) ;

    IplImage * gaussianImage =
            cvCreateImage ( cvGetSize(input), IPL_DEPTH_32F, 1);
    cvConvertScale (grayImage, gaussianImage, 1./255., 0);
    cvSmooth( gaussianImage, gaussianImage, CV_GAUSSIAN, 5, 5);

    IplImage * gradientX =
            cvCreateImage ( cvGetSize ( input ), IPL_DEPTH_32F, 1 );

    IplImage * gradientY =
            cvCreateImage ( cvGetSize ( input ), IPL_DEPTH_32F, 1 );
 
    cvSobel(gaussianImage, gradientX , 1, 0, CV_SCHARR);
    cvSobel(gaussianImage, gradientY , 0, 1, CV_SCHARR);
    cvSmooth(gradientX, gradientX, 3, 3);
    cvSmooth(gradientY, gradientY, 3, 3);
    cvReleaseImage ( &gaussianImage );
    cvReleaseImage ( &grayImage );

    std::vector<Ray> rays;
    IplImage * SWTImage =
            cvCreateImage ( cvGetSize ( input ), IPL_DEPTH_32F, 1 );
    for( int row = 0; row < input->height; row++ ){
        float* ptr = (float*)(SWTImage->imageData + row * SWTImage->widthStep);
        for ( int col = 0; col < input->width; col++ ){
            *ptr++ = -1;
        }
    }
    strokeWidthTransform ( edgeImage, gradientX, gradientY, dark_on_light, SWTImage, rays );
    SWTMedianFilter ( SWTImage, rays );

    IplImage * output2 =
            cvCreateImage ( cvGetSize ( input ), IPL_DEPTH_32F, 1 );
    normalizeImage (SWTImage, output2);
    IplImage * saveSWT =
            cvCreateImage ( cvGetSize ( input ), IPL_DEPTH_8U, 1 );
    cvConvertScale(output2, saveSWT, 255, 0);
    cvReleaseImage ( &output2 );
    cvReleaseImage ( &saveSWT );

    std::vector<std::vector<Point2d> > components = findLegallyConnectedComponents(SWTImage, rays);

    std::vector<std::vector<Point2d> > validComponents;
    std::vector<Point2dFloat> compCenters;
    std::vector<std::pair<Point2d,Point2d> > compBB;
    std::vector<float> compMedians;
    std::vector<Point2d> compDimensions;
    filterComponents(SWTImage, components, validComponents, compCenters, compMedians, compDimensions, compBB );

    /*IplImage * output3 = cvCreateImage ( cvGetSize ( input ), 8U, 3 );
    renderComponentsWithBoxes (SWTImage, validComponents, compBB, output3);

    if (dark_on_light) cvSaveImage ( "components1.png",output3);
    else cvSaveImage ( "components0.png",output3);
    cvReleaseImage ( &output3 );*/

    std::vector< Chain > chains;
    chains = makeChains(input, validComponents, compCenters, compMedians, compDimensions, compBB);

    IplImage * output4 = cvCreateImage ( cvGetSize ( input ), IPL_DEPTH_8U, 3);
    renderChains (SWTImage, validComponents, chains, compBB, bb, output4);

    cvReleaseImage ( &output4 );
    cvReleaseImage ( &gradientX );
    cvReleaseImage ( &gradientX );
    cvReleaseImage ( &gradientY );
    cvReleaseImage ( &SWTImage );
    cvReleaseImage ( &edgeImage );
}


void strokeWidthTransform (IplImage * edgeImage,
                           IplImage * gradientX,
                           IplImage * gradientY,
                           bool dark_on_light,
                           IplImage * SWTImage,
                           std::vector<Ray> & rays) {
    // First pass
    float prec = .05;
    for( int row = 0; row < edgeImage->height; row++ ){
        const uchar* ptr = (const uchar*)(edgeImage->imageData + row * edgeImage->widthStep);
        for ( int col = 0; col < edgeImage->width; col++ ){
            if (*ptr > 0) {
                Ray r;

                Point2d p;
                p.x = col;
                p.y = row;
                r.p = p;
                std::vector<Point2d> points;
                points.push_back(p);

                float curX = (float)col + 0.5;
                float curY = (float)row + 0.5;
                int curPixX = col;
                int curPixY = row;
                float G_x = CV_IMAGE_ELEM ( gradientX, float, row, col);
                float G_y = CV_IMAGE_ELEM ( gradientY, float, row, col);
                // normalize gradient
                float mag = sqrt( (G_x * G_x) + (G_y * G_y) );
                if (dark_on_light){
                    G_x = -G_x/mag;
                    G_y = -G_y/mag;
                } else {
                    G_x = G_x/mag;
                    G_y = G_y/mag;

                }
                while (true) {
                    curX += G_x*prec;
                    curY += G_y*prec;
                    if ((int)(floor(curX)) != curPixX || (int)(floor(curY)) != curPixY) {
                        curPixX = (int)(floor(curX));
                        curPixY = (int)(floor(curY));
                        // check if pixel is outside boundary of image
                        if (curPixX < 0 || (curPixX >= SWTImage->width) || curPixY < 0 || (curPixY >= SWTImage->height)) {
                            break;
                        }
                        Point2d pnew;
                        pnew.x = curPixX;
                        pnew.y = curPixY;
                        points.push_back(pnew);

                        if (CV_IMAGE_ELEM ( edgeImage, uchar, curPixY, curPixX) > 0) {
                            r.q = pnew;
                            // dot product
                            float G_xt = CV_IMAGE_ELEM(gradientX,float,curPixY,curPixX);
                            float G_yt = CV_IMAGE_ELEM(gradientY,float,curPixY,curPixX);
                            mag = sqrt( (G_xt * G_xt) + (G_yt * G_yt) );
                            if (dark_on_light){
                                G_xt = -G_xt/mag;
                                G_yt = -G_yt/mag;
                            } else {
                                G_xt = G_xt/mag;
                                G_yt = G_yt/mag;

                            }

                            if (acos(G_x * -G_xt + G_y * -G_yt) < PI/2.0 ) {
                                float length = sqrt( ((float)r.q.x - (float)r.p.x)*((float)r.q.x - (float)r.p.x) + ((float)r.q.y - (float)r.p.y)*((float)r.q.y - (float)r.p.y));
                                for (std::vector<Point2d>::iterator pit = points.begin(); pit != points.end(); pit++) {
                                    if (CV_IMAGE_ELEM(SWTImage, float, pit->y, pit->x) < 0) {
                                        CV_IMAGE_ELEM(SWTImage, float, pit->y, pit->x) = length;
                                    } else {
                                        CV_IMAGE_ELEM(SWTImage, float, pit->y, pit->x) = std::min(length, CV_IMAGE_ELEM(SWTImage, float, pit->y, pit->x));
                                    }
                                }
                                r.points = points;
                                rays.push_back(r);
                            }
                            break;
                        }
                    }
                }
            }
            ptr++;
        }
    }

}

void SWTMedianFilter (IplImage * SWTImage,
                     std::vector<Ray> & rays) {
    for (std::vector<Ray>::iterator rit = rays.begin(); rit != rays.end(); rit++) {
        for (std::vector<Point2d>::iterator pit = rit->points.begin(); pit != rit->points.end(); pit++) {
            pit->SWT = CV_IMAGE_ELEM(SWTImage, float, pit->y, pit->x);
        }
        std::sort(rit->points.begin(), rit->points.end(), &Point2dSort);
        float median = (rit->points[rit->points.size()/2]).SWT;
        for (std::vector<Point2d>::iterator pit = rit->points.begin(); pit != rit->points.end(); pit++) {
            CV_IMAGE_ELEM(SWTImage, float, pit->y, pit->x) = std::min(pit->SWT, median);
        }
    }

}

bool Point2dSort (const Point2d &lhs, const Point2d &rhs) {
    return lhs.SWT < rhs.SWT;
}

std::vector< std::vector<Point2d> >
findLegallyConnectedComponents (IplImage * SWTImage,
                                std::vector<Ray> & rays)
{
        boost::unordered_map<int, int> map;
        boost::unordered_map<int, Point2d> revmap;

        typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> Graph;
        int num_vertices = 0;
        // Number vertices for graph.  Associate each point with number
        for( int row = 0; row < SWTImage->height; row++ ){
            float * ptr = (float*)(SWTImage->imageData + row * SWTImage->widthStep);
            for (int col = 0; col < SWTImage->width; col++ ){
                if (*ptr > 0) {
                    map[row * SWTImage->width + col] = num_vertices;
                    Point2d p;
                    p.x = col;
                    p.y = row;
                    revmap[num_vertices] = p;
                    num_vertices++;
                }
                ptr++;
            }
        }

        Graph g(num_vertices);

        for( int row = 0; row < SWTImage->height; row++ ){
            float * ptr = (float*)(SWTImage->imageData + row * SWTImage->widthStep);
            for (int col = 0; col < SWTImage->width; col++ ){
                if (*ptr > 0) {
                    // check pixel to the right, right-down, down, left-down
                    int this_pixel = map[row * SWTImage->width + col];
                    if (col+1 < SWTImage->width) {
                        float right = CV_IMAGE_ELEM(SWTImage, float, row, col+1);
                        if (right > 0 && ((*ptr)/right <= 3.0 || right/(*ptr) <= 3.0))
                            boost::add_edge(this_pixel, map.at(row * SWTImage->width + col + 1), g);
                    }
                    if (row+1 < SWTImage->height) {
                        if (col+1 < SWTImage->width) {
                            float right_down = CV_IMAGE_ELEM(SWTImage, float, row+1, col+1);
                            if (right_down > 0 && ((*ptr)/right_down <= 3.0 || right_down/(*ptr) <= 3.0))
                                boost::add_edge(this_pixel, map.at((row+1) * SWTImage->width + col + 1), g);
                        }
                        float down = CV_IMAGE_ELEM(SWTImage, float, row+1, col);
                        if (down > 0 && ((*ptr)/down <= 3.0 || down/(*ptr) <= 3.0))
                            boost::add_edge(this_pixel, map.at((row+1) * SWTImage->width + col), g);
                        if (col-1 >= 0) {
                            float left_down = CV_IMAGE_ELEM(SWTImage, float, row+1, col-1);
                            if (left_down > 0 && ((*ptr)/left_down <= 3.0 || left_down/(*ptr) <= 3.0))
                                boost::add_edge(this_pixel, map.at((row+1) * SWTImage->width + col - 1), g);
                        }
                    }
                }
                ptr++;
            }
        }

        std::vector<int> c(num_vertices);

        int num_comp = connected_components(g, &c[0]);

        std::vector<std::vector<Point2d> > components;
        components.reserve(num_comp);
        //std::cout << "Before filtering, " << num_comp << " components and " << num_vertices << " vertices" << std::endl;
        for (int j = 0; j < num_comp; j++) {
            std::vector<Point2d> tmp;
            components.push_back( tmp );
        }
        for (int j = 0; j < num_vertices; j++) {
            Point2d p = revmap[j];
            (components[c[j]]).push_back(p);
        }

        return components;
}

std::vector< std::vector<Point2d> >
findLegallyConnectedComponentsRAY (IplImage * SWTImage,
                                std::vector<Ray> & rays)
{
        boost::unordered_map<int, int> map;
        boost::unordered_map<int, Point2d> revmap;

        typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> Graph;
        int num_vertices = 0;
        // Number vertices for graph.  Associate each point with number
        for( int row = 0; row < SWTImage->height; row++ ){
            float * ptr = (float*)(SWTImage->imageData + row * SWTImage->widthStep);
            for (int col = 0; col < SWTImage->width; col++ ){
                if (*ptr > 0) {
                    map[row * SWTImage->width + col] = num_vertices;
                    Point2d p;
                    p.x = col;
                    p.y = row;
                    revmap[num_vertices] = p;
                    num_vertices++;
                }
                ptr++;
            }
        }

        Graph g(num_vertices);

        // Traverse and add edges to graph
        for (std::vector<Ray>::const_iterator it = rays.begin(); it != rays.end(); it++) {
                float lastSW = 0;
                int lastRow = 0;
                int lastCol = 0;
                for (std::vector<Point2d>::const_iterator it2 = it->points.begin(); it2 != it->points.end(); it2++) {
                        float currentSW = CV_IMAGE_ELEM(SWTImage, float, it2->y, it2->x);
                        if (lastSW == 0) {}
                        else if (lastSW/currentSW<=3.0 || currentSW/lastSW<=3.0){
                                boost::add_edge(map.at(it2->y * SWTImage->width + it2->x), map.at(lastRow * SWTImage->width + lastCol), g);
                        }
                        lastSW = currentSW;
                        lastRow = it2->y;
                        lastCol = it2->x;
                }
                lastSW = 0;
                lastRow = 0;
                lastCol = 0;
        }

        std::vector<int> c(num_vertices);

        int num_comp = connected_components(g, &c[0]);

        std::vector<std::vector<Point2d> > components;
        components.reserve(num_comp);
        //std::cout << "Before filtering, " << num_comp << " components and " << num_vertices << " vertices" << std::endl;
        for (int j = 0; j < num_comp; j++) {
            std::vector<Point2d> tmp;
            components.push_back( tmp );
        }
        for (int j = 0; j < num_vertices; j++) {
            Point2d p = revmap[j];
            (components[c[j]]).push_back(p);
        }

        return components;
}

void componentStats(IplImage * SWTImage,
                                        const std::vector<Point2d> & component,
                                        float & mean, float & variance, float & median,
                                        int & minx, int & miny, int & maxx, int & maxy)
{
        std::vector<float> temp;
        temp.reserve(component.size());
        mean = 0;
        variance = 0;
        minx = 1000000;
        miny = 1000000;
        maxx = 0;
        maxy = 0;
        for (std::vector<Point2d>::const_iterator it = component.begin(); it != component.end(); it++) {
                float t = CV_IMAGE_ELEM(SWTImage, float, it->y, it->x);
                mean += t;
                temp.push_back(t);
                miny = std::min(miny,it->y);
                minx = std::min(minx,it->x);
                maxy = std::max(maxy,it->y);
                maxx = std::max(maxx,it->x);
        }
        mean = mean / ((float)component.size());
        for (std::vector<float>::const_iterator it = temp.begin(); it != temp.end(); it++) {
            variance += (*it - mean) * (*it - mean);
        }
        variance = variance / ((float)component.size());
        std::sort(temp.begin(),temp.end());
        median = temp[temp.size()/2];
}


void filterComponents(IplImage * SWTImage,
                      std::vector<std::vector<Point2d> > & components,
                      std::vector<std::vector<Point2d> > & validComponents,
                      std::vector<Point2dFloat> & compCenters,
                      std::vector<float> & compMedians,
                      std::vector<Point2d> & compDimensions,
                      std::vector<std::pair<Point2d,Point2d> > & compBB )
{
        validComponents.reserve(components.size());
        compCenters.reserve(components.size());
        compMedians.reserve(components.size());
        compDimensions.reserve(components.size());
        // bounding boxes
        compBB.reserve(components.size());
        for (std::vector<std::vector<Point2d> >::iterator it = components.begin(); it != components.end();it++) {
            // compute the stroke width mean, variance, median
            float mean, variance, median;
            int minx, miny, maxx, maxy;
            componentStats(SWTImage, (*it), mean, variance, median, minx, miny, maxx, maxy);

            // check if variance is less than half the mean
            if (variance > 0.5 * mean) {
                 continue;
            }

            float length = (float)(maxx-minx+1);
            float width = (float)(maxy-miny+1);

            // check font height
            if (width > 300) {
                continue;
            }

            float area = length * width;
            float rminx = (float)minx;
            float rmaxx = (float)maxx;
            float rminy = (float)miny;
            float rmaxy = (float)maxy;
            // compute the rotated bounding box
            float increment = 1./36.;
            for (float theta = increment * PI; theta<PI/2.0; theta += increment * PI) {
                float xmin,xmax,ymin,ymax,xtemp,ytemp,ltemp,wtemp;
                    xmin = 1000000;
                    ymin = 1000000;
                    xmax = 0;
                    ymax = 0;
                for (unsigned int i = 0; i < (*it).size(); i++) {
                    xtemp = (*it)[i].x * cos(theta) + (*it)[i].y * -sin(theta);
                    ytemp = (*it)[i].x * sin(theta) + (*it)[i].y * cos(theta);
                    xmin = std::min(xtemp,xmin);
                    xmax = std::max(xtemp,xmax);
                    ymin = std::min(ytemp,ymin);
                    ymax = std::max(ytemp,ymax);
                }
                ltemp = xmax - xmin + 1;
                wtemp = ymax - ymin + 1;
                if (ltemp*wtemp < area) {
                    area = ltemp*wtemp;
                    length = ltemp;
                    width = wtemp;
                }
            }
            // check if the aspect ratio is between 1/10 and 10
            if (length/width < 1./10. || length/width > 10.) {
                continue;
            }

            // compute the diameter TODO finish
            // compute dense representation of component
            std::vector <std::vector<float> > denseRepr;
            denseRepr.reserve(maxx-minx+1);
            for (int i = 0; i < maxx-minx+1; i++) {
                std::vector<float> tmp;
                tmp.reserve(maxy-miny+1);
                denseRepr.push_back(tmp);
                for (int j = 0; j < maxy-miny+1; j++) {\
                    denseRepr[i].push_back(0);
                }
            }
            for (std::vector<Point2d>::iterator pit = it->begin(); pit != it->end(); pit++) {
                (denseRepr[pit->x - minx])[pit->y - miny] = 1;
            }
            // create graph representing components
            const int num_nodes = it->size();
            /*
            E edges[] = { E(0,2),
                          E(1,1), E(1,3), E(1,4),
                          E(2,1), E(2,3),
                          E(3,4),
                          E(4,0), E(4,1) };

            Graph G(edges + sizeof(edges) / sizeof(E), weights, num_nodes);
            */
            Point2dFloat center;
            center.x = ((float)(maxx+minx))/2.0;
            center.y = ((float)(maxy+miny))/2.0;

            Point2d dimensions;
            dimensions.x = maxx - minx + 1;
            dimensions.y = maxy - miny + 1;

            Point2d bb1;
            bb1.x = minx;
            bb1.y = miny;

            Point2d bb2;
            bb2.x = maxx;
            bb2.y = maxy;
            std::pair<Point2d, Point2d> pair(bb1,bb2);

            compBB.push_back(pair);
            compDimensions.push_back(dimensions);
            compMedians.push_back(median);
            compCenters.push_back(center);
            validComponents.push_back(*it);
        }
       std::vector<std::vector<Point2d > > tempComp;
       std::vector<Point2d > tempDim;
       std::vector<float > tempMed;
       std::vector<Point2dFloat > tempCenters;
       std::vector<std::pair<Point2d,Point2d> > tempBB;
       tempComp.reserve(validComponents.size());
       tempCenters.reserve(validComponents.size());
       tempDim.reserve(validComponents.size());
       tempMed.reserve(validComponents.size());
       tempBB.reserve(validComponents.size());
       for (unsigned int i = 0; i < validComponents.size(); i++) {
            int count = 0;
            for (unsigned int j = 0; j < validComponents.size(); j++) {
                if (i != j) {
                    if (compBB[i].first.x <= compCenters[j].x && compBB[i].second.x >= compCenters[j].x &&
                        compBB[i].first.y <= compCenters[j].y && compBB[i].second.y >= compCenters[j].y) {
                        count++;
                    }
                }
            }
            if (count < 2) {
                tempComp.push_back(validComponents[i]);
                tempCenters.push_back(compCenters[i]);
                tempMed.push_back(compMedians[i]);
                tempDim.push_back(compDimensions[i]);
                tempBB.push_back(compBB[i]);
            }
        }
        compDimensions.reserve(tempComp.size());
        compMedians.reserve(tempComp.size());
        compCenters.reserve(tempComp.size());
        validComponents.reserve(tempComp.size());
        compBB.reserve(tempComp.size());

        validComponents = tempComp;
        compDimensions = tempDim;
        compMedians = tempMed;
        compCenters = tempCenters;
        compBB = tempBB;



        //std::cout << "After filtering " << validComponents.size() << " components" << std::endl;
}

bool sharesOneEnd( Chain c0, Chain c1) {
    if (c0.p == c1.p || c0.p == c1.q || c0.q == c1.q || c0.q == c1.p) {
        return true;
    }
    else {
        return false;
    }
}

bool chainSortDist (const Chain &lhs, const Chain &rhs) {
    return lhs.dist < rhs.dist;
}

bool chainSortLength (const Chain &lhs, const Chain &rhs) {
    return lhs.components.size() > rhs.components.size();
}

std::vector<Chain> makeChains( IplImage * colorImage,
                 std::vector<std::vector<Point2d> > & components,
                 std::vector<Point2dFloat> & compCenters,
                 std::vector<float> & compMedians,
                 std::vector<Point2d> & compDimensions,
                 std::vector<std::pair<Point2d,Point2d> > & compBB) {
    assert (compCenters.size() == components.size());
    // make vector of color averages
    std::vector<Point3dFloat> colorAverages;
    colorAverages.reserve(components.size());
    for (std::vector<std::vector<Point2d> >::iterator it = components.begin(); it != components.end();it++) {
        Point3dFloat mean;
        mean.x = 0;
        mean.y = 0;
        mean.z = 0;
        int num_points = 0;
        for (std::vector<Point2d>::iterator pit = it->begin(); pit != it->end(); pit++) {
            mean.x += (float) CV_IMAGE_ELEM (colorImage, unsigned char, pit->y, (pit->x)*3 );
            mean.y += (float) CV_IMAGE_ELEM (colorImage, unsigned char, pit->y, (pit->x)*3+1 );
            mean.z += (float) CV_IMAGE_ELEM (colorImage, unsigned char, pit->y, (pit->x)*3+2 );
            num_points++;
        }
        mean.x = mean.x / ((float)num_points);
        mean.y = mean.y / ((float)num_points);
        mean.z = mean.z / ((float)num_points);
        colorAverages.push_back(mean);
    }

    // form all eligible pairs and calculate the direction of each
    std::vector<Chain> chains;
    for ( unsigned int i = 0; i < components.size() - 1; i++ ) {
        for ( unsigned int j = i + 1; j < components.size(); j++ ) {
            // TODO add color metric
            if ( (compMedians[i]/compMedians[j] <= 2.0 || compMedians[j]/compMedians[i] <= 2.0) &&
                 (compDimensions[i].y/compDimensions[j].y <= 2.0 || compDimensions[j].y/compDimensions[i].y <= 2.0)) {
                float dist = (compCenters[i].x - compCenters[j].x) * (compCenters[i].x - compCenters[j].x) +
                             (compCenters[i].y - compCenters[j].y) * (compCenters[i].y - compCenters[j].y);
                float colorDist = (colorAverages[i].x - colorAverages[j].x) * (colorAverages[i].x - colorAverages[j].x) +
                                  (colorAverages[i].y - colorAverages[j].y) * (colorAverages[i].y - colorAverages[j].y) +
                                  (colorAverages[i].z - colorAverages[j].z) * (colorAverages[i].z - colorAverages[j].z);
                if (dist < 9*(float)(std::max(std::min(compDimensions[i].x,compDimensions[i].y),std::min(compDimensions[j].x,compDimensions[j].y)))
                    *(float)(std::max(std::min(compDimensions[i].x,compDimensions[i].y),std::min(compDimensions[j].x,compDimensions[j].y)))
                    && colorDist < 1600) {
                    Chain c;
                    c.p = i;
                    c.q = j;
                    std::vector<int> comps;
                    comps.push_back(c.p);
                    comps.push_back(c.q);
                    c.components = comps;
                    c.dist = dist;
                    float d_x = (compCenters[i].x - compCenters[j].x);
                    float d_y = (compCenters[i].y - compCenters[j].y);
                    float mag = sqrt(d_x*d_x + d_y*d_y);
                    d_x = d_x / mag;
                    d_y = d_y / mag;
                    Point2dFloat dir;
                    dir.x = d_x;
                    dir.y = d_y;
                    c.direction = dir;
                    chains.push_back(c);

                }
            }
        }
    }
    //std::cout << chains.size() << " eligible pairs" << std::endl;
    std::sort(chains.begin(), chains.end(), &chainSortDist);

    std::cerr << std::endl;
    const float strictness = PI/6.0;
    //merge chains
    int merges = 1;
    while (merges > 0) {
        for (unsigned int i = 0; i < chains.size(); i++) {
            chains[i].merged = false;
        }
        merges = 0;
        std::vector<Chain> newchains;
        for (unsigned int i = 0; i < chains.size(); i++) {
            for (unsigned int j = 0; j < chains.size(); j++) {
                if (i != j) {
                    if (!chains[i].merged && !chains[j].merged && sharesOneEnd(chains[i],chains[j])) {
                        if (chains[i].p == chains[j].p) {
                            if (acos(chains[i].direction.x * -chains[j].direction.x + chains[i].direction.y * -chains[j].direction.y) < strictness) {

                                chains[i].p = chains[j].q;
                                for (std::vector<int>::iterator it = chains[j].components.begin(); it != chains[j].components.end(); it++) {
                                    chains[i].components.push_back(*it);
                                }
                                float d_x = (compCenters[chains[i].p].x - compCenters[chains[i].q].x);
                                float d_y = (compCenters[chains[i].p].y - compCenters[chains[i].q].y);
                                chains[i].dist = d_x * d_x + d_y * d_y;

                                float mag = sqrt(d_x*d_x + d_y*d_y);
                                d_x = d_x / mag;
                                d_y = d_y / mag;
                                Point2dFloat dir;
                                dir.x = d_x;
                                dir.y = d_y;
                                chains[i].direction = dir;
                                chains[j].merged = true;
                                merges++;
                            }
                        } else if (chains[i].p == chains[j].q) {
                            if (acos(chains[i].direction.x * chains[j].direction.x + chains[i].direction.y * chains[j].direction.y) < strictness) {

                                chains[i].p = chains[j].p;
                                for (std::vector<int>::iterator it = chains[j].components.begin(); it != chains[j].components.end(); it++) {
                                    chains[i].components.push_back(*it);
                                }
                                float d_x = (compCenters[chains[i].p].x - compCenters[chains[i].q].x);
                                float d_y = (compCenters[chains[i].p].y - compCenters[chains[i].q].y);
                                float mag = sqrt(d_x*d_x + d_y*d_y);
                                chains[i].dist = d_x * d_x + d_y * d_y;

                                d_x = d_x / mag;
                                d_y = d_y / mag;

                                Point2dFloat dir;
                                dir.x = d_x;
                                dir.y = d_y;
                                chains[i].direction = dir;
                                chains[j].merged = true;
                                merges++;
                            }
                        } else if (chains[i].q == chains[j].p) {
                            if (acos(chains[i].direction.x * chains[j].direction.x + chains[i].direction.y * chains[j].direction.y) < strictness) {

                                chains[i].q = chains[j].q;
                                for (std::vector<int>::iterator it = chains[j].components.begin(); it != chains[j].components.end(); it++) {
                                    chains[i].components.push_back(*it);
                                }
                                float d_x = (compCenters[chains[i].p].x - compCenters[chains[i].q].x);
                                float d_y = (compCenters[chains[i].p].y - compCenters[chains[i].q].y);
                                float mag = sqrt(d_x*d_x + d_y*d_y);
                                chains[i].dist = d_x * d_x + d_y * d_y;


                                d_x = d_x / mag;
                                d_y = d_y / mag;
                                Point2dFloat dir;
                                dir.x = d_x;
                                dir.y = d_y;

                                chains[i].direction = dir;
                                chains[j].merged = true;
                                merges++;
                            }
                        } else if (chains[i].q == chains[j].q) {
                            if (acos(chains[i].direction.x * -chains[j].direction.x + chains[i].direction.y * -chains[j].direction.y) < strictness) {

                                chains[i].q = chains[j].p;
                                for (std::vector<int>::iterator it = chains[j].components.begin(); it != chains[j].components.end(); it++) {
                                    chains[i].components.push_back(*it);
                                }
                                float d_x = (compCenters[chains[i].p].x - compCenters[chains[i].q].x);
                                float d_y = (compCenters[chains[i].p].y - compCenters[chains[i].q].y);
                                chains[i].dist = d_x * d_x + d_y * d_y;

                                float mag = sqrt(d_x*d_x + d_y*d_y);
                                d_x = d_x / mag;
                                d_y = d_y / mag;
                                Point2dFloat dir;
                                dir.x = d_x;
                                dir.y = d_y;
                                chains[i].direction = dir;
                                chains[j].merged = true;
                                merges++;
                            }
                        }
                    }
                }
            }
        }
        for (unsigned int i = 0; i < chains.size(); i++) {
            if (!chains[i].merged) {
                newchains.push_back(chains[i]);
            }
        }
        chains = newchains;
        std::stable_sort(chains.begin(), chains.end(), &chainSortLength);
    }

    std::vector<Chain> newchains;
    newchains.reserve(chains.size());
    for (std::vector<Chain>::iterator cit = chains.begin(); cit != chains.end(); cit++) {
        if (cit->components.size() >= 2) {
            newchains.push_back(*cit);
        }
    }
    chains = newchains;
    return chains;
}