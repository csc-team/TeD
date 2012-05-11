#ifndef TRACKIMAGE_H
#define TRACKIMAGE_H

/*
***
	Находит особые точки на заданном участке изображения
***
*/
void findKeyPoints(CvRect r);
/*
 ***
 	Выделение необходимых ресурсов. Нужно вызвать перед первым запуском
 ***
 */
void initResources(IplImage* frame);
	
/*
 ***
 	Освобождение выделенных ресурсов. 
 ***
 */
void releaseResources();
		   
		   
/*
***
	Отслеживает текст на текущем изображении.
	Возвращает прямоугольник содержащий текстовую область
***
*/
CvRect TextTracking(IplImage* frame, int mode = 0);  	// если mode != 0 нужно вызвать детектор
#endif // TRACKIMAGE_H
