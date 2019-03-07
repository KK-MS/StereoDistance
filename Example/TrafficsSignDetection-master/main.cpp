#include "color_cvt.h"
#include "pre_img.h"

RNG rng(12345);
string labelname[500] = {"stop","20t","car forbiden","stop2","5","no noisy","30"};

int main()
{
	char path[50];
	//�������������������ֱ��ѵ��
	CvSVM classifier;
	classifier.load("model180.txt");
	for(int k = 0;k<100;k++)
	{
		sprintf(path,"raw/%d.jpg",k+1);
		cout << path << endl;
		//��ȡͼƬ
		Mat src = imread(path);
		Mat copy;
		src.copyTo(copy);
		//�õ�ͼ�����
		int width = src.cols;   //ͼ����
		int height = src.rows;   //ͼ��߶�
		//ɫ�ʷָ�
		double B=0.0,G=0.0,R=0.0,H=0.0,S=0.0,V=0.0;
		Mat Mat_rgb = Mat::zeros( src.size(), CV_8UC1 );
		Mat Mat_rgb_copy;//һ���ݴ浥Ԫ
		//Mat copy;
		//src.copyTo(copy);
		int x,y,px,py; //ѭ��
		for (y=0; y<height; y++)
		{
			for ( x=0; x<width; x++)
			{
				// ��ȡBGRֵ
				B = src.at<Vec3b>(y,x)[0];
				G = src.at<Vec3b>(y,x)[1];
				R = src.at<Vec3b>(y,x)[2];
				RGB2HSV(R,G,B,H,S,V); 
				//��ɫ��337-360
				if((H>=337 && H<=360||H>=0&&H<=10)&&S>=12&&S<=100&&V>20&&V<99)
				{
					Mat_rgb.at<uchar>(y,x) = 255;  //R     
				}
			}
		}
		//imshow("hsi",Mat_rgb);
		medianBlur(Mat_rgb,Mat_rgb,3);
		Mat element = getStructuringElement(MORPH_ELLIPSE,
										   Size(2*1 + 1, 2*1 + 1),
										   Point(1,1));
		Mat element1 = getStructuringElement(MORPH_ELLIPSE,
										   Size(2*3 + 1, 2*3 + 1),
										   Point(3,3));
		//��̬ѧ����ͼ�񣬿������ͱղ�����������������ͨ��Ч����
		erode(Mat_rgb,Mat_rgb,element);
		dilate(Mat_rgb,Mat_rgb,element1);
		fillHole(Mat_rgb,Mat_rgb);
		Mat_rgb.copyTo(Mat_rgb_copy);
		/// ����αƽ����� + ��ȡ���κ�Բ�α߽��
		//������
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		findContours( Mat_rgb, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
		/// ����αƽ����� + ��ȡ���κ�Բ�α߽��
		vector<vector<Point> > contours_poly( contours.size() );
		vector<Rect> boundRect( contours.size() );
		vector<Point2f>center( contours.size() );
		vector<float>radius( contours.size() );
  
		for( int i = 0; i < contours.size(); i++ )
		{ 
			approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
			boundRect[i] = boundingRect( Mat(contours_poly[i]) );
			minEnclosingCircle( contours_poly[i], center[i], radius[i] );
		}
		/// ����������� + ��Χ�ľ��ο� + Բ�ο�
		Mat drawing = Mat::zeros( Mat_rgb.size(), CV_8UC3 );
		int count1 = 0;
		for( int i = 0; i< contours.size(); i++ )
		{
			Rect rect = boundRect[i];
			//���Ƚ���һ�������ƣ�ɸѡ������
			//�߿������
			float ratio = (float)rect.width / (float)rect.height;
			//�������     
			float Area = (float)rect.width * (float)rect.height;
			float dConArea = (float)contourArea(contours[i]);
			float dConLen = (float)arcLength(contours[i],1);
			if(dConArea <400)
				continue;
			if(ratio>2||ratio<0.5)
				continue;
		
			//����Բɸѡ��ͨ���Ŀ��ȱʧ���رȽ�
			Mat roiImage;  
			Mat_rgb_copy(rect).copyTo(roiImage);
			//imshow("test",roiImage);
			Mat temp;
			copy(rect).copyTo(temp);
			//imshow("test2",temp);
			bool iscircle = isCircle(roiImage,temp);
			cout << "circle:" << iscircle << endl;
			if(!iscircle)
				continue;
			//����������״���ƣ�������Բ�α�־��
			float C = (4*PI*dConArea)/(dConLen*dConLen);
			if(C < 0.4)//����Բ�ȳ�������״����ɸѡ
				continue;
			//cout << C << "," << R << "," << E << "," << E << endl;
			copy(rect).copyTo(roiImage);
			//imshow("roi",roiImage);
			//count1++;
			//sprintf(path,"newresult1/%d_%d.jpg",k,count1);
			//imwrite(path, temp);
			//sprintf(path,"newresult2/%d.jpg",count1);
			//imwrite(path, roiImage);
			//*********svm*********
			//imshow("a",temp);
			Mat temp2 = Mat::zeros( temp.size(), CV_8UC1 );
			cvtColor(temp,temp2,CV_BGR2GRAY);
			resize(temp2,temp2,Size(45,45));
			//threshold(temp2,temp2,120,255,THRESH_BINARY_INV);
			temp2 = temp2.reshape(0,1);
			temp2.convertTo(temp2, CV_32F);
			int result = (int)classifier.predict(temp2)-1;//svmԤ��
			//*********************
			Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
			drawContours( drawing, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
			rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
			rectangle( src, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
			putText(src, labelname[result], cvPoint(boundRect[i].x, boundRect[i].y-10),1,1,CV_RGB(255, 0, 0),2);//��ɫ����ע��
			//circle( drawing, center[i], (int)radius[i], color, 2, 8, 0 );
			count1++;
			sprintf(path,"result/%d_%d.jpg",k,count1);
			imwrite(path, src);//�������յļ��ʶ����
		}
	 
		//�����ʾ
		//imshow("src",src);
		//imshow("drawing",drawing);
		//imshow("filter", Mat_rgb);
		//waitKey(0);
	}
	waitKey(0);
	return 0;
}