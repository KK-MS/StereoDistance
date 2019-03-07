#include "pre_img.h"


void fillHole(const Mat srcBw, Mat &dstBw)
{
    Size m_Size = srcBw.size();
    Mat Temp=Mat::zeros(m_Size.height+2,m_Size.width+2,srcBw.type());//��չͼ��
    srcBw.copyTo(Temp(Range(1, m_Size.height + 1), Range(1, m_Size.width + 1)));
 
    cv::floodFill(Temp, Point(0, 0), Scalar(255));//�������
 
    Mat cutImg;//�ü���չ��ͼ��
    Temp(Range(1, m_Size.height + 1), Range(1, m_Size.width + 1)).copyTo(cutImg);
 
    dstBw = srcBw | (~cutImg);
}

bool isCircle(const Mat srcBw,Mat& mytemp)//�����Ľ���
{//�������һ���Ҷ�ͼ��
	Mat temp = Mat::zeros( srcBw.size(), CV_8UC1 );
	bool iscircle = false;
	//���srcBw��Ϣ
	int w = srcBw.cols;
	int h = srcBw.rows;
	int w1 = mytemp.cols;
	int h1 = mytemp.rows;
	//cout << w << " " << w1 << " " << h << " " << h1 << endl;
	int count1=0;//�����ֵ�ȱʧ���ؼ�����
	int count2=0;
	int count3=0;
	int count4=0;
	//��srcBwƽ���ֳ��ķ�,���з���ȱʧ�����ظ�������ռ����
	//�ȷ�������
	for(int i = 0;i < h/2;i ++)
	{
		for(int j = 0;j < w/2;j ++)
		{
			if(srcBw.at<uchar>(i,j) == 0)
			{
				temp.at<uchar>(i,j) = 255;
				mytemp.at<uchar>(i,j*mytemp.channels()+0) = 255;
				mytemp.at<uchar>(i,j*mytemp.channels()+1) = 255;
				mytemp.at<uchar>(i,j*mytemp.channels()+2) = 255;
				count1 ++;
			}
		}
	}
	//����
	for(int i = 0;i < h/2;i ++)
	{
		for(int j = w/2-1;j < w;j ++)
		{
			if(srcBw.at<uchar>(i,j) == 0)
			{
				temp.at<uchar>(i,j) = 255;				
				mytemp.at<uchar>(i,j*mytemp.channels()+0) = 255;
				mytemp.at<uchar>(i,j*mytemp.channels()+1) = 255;
				mytemp.at<uchar>(i,j*mytemp.channels()+2) = 255;
				count2 ++;
			}
		}
	}
	//����
	for(int i = h/2-1;i < h;i ++)
	{
		for(int j = 0;j < w/2;j ++)
		{
			if(srcBw.at<uchar>(i,j) == 0)
			{
				temp.at<uchar>(i,j) = 255;
				mytemp.at<uchar>(i,j*mytemp.channels()+0) = 255;
				mytemp.at<uchar>(i,j*mytemp.channels()+1) = 255;
				mytemp.at<uchar>(i,j*mytemp.channels()+2) = 255;
				count3 ++;
			}
		}
	}
	//����
	for(int i = h/2-1;i < h;i ++)
	{
		for(int j = w/2-1;j < w;j ++)
		{
			if(srcBw.at<uchar>(i,j) == 0)
			{
				temp.at<uchar>(i,j) = 255;				
				mytemp.at<uchar>(i,j*mytemp.channels()+0) = 255;
				mytemp.at<uchar>(i,j*mytemp.channels()+1) = 255;
				mytemp.at<uchar>(i,j*mytemp.channels()+2) = 255;
				count4 ++;
			}
		}
	}


	float c1 = (float)count1/(float)(w*h);//����
	float c2 = (float)count2/(float)(w*h);//����
	float c3 = (float)count3/(float)(w*h);//����
	float c4 = (float)count4/(float)(w*h);//����
	//imshow("temp",mytemp);
	cout << "result: " << c1 << "," << c2
		<< "," << c3 << "," << c4 << endl;

	//�޶�ÿ�����ʵķ�Χ
	if((c1>0.037&&c1<0.12)&&(c2>0.037&&c2<0.12)&&(c2>0.037&&c2<0.12)&&(c2>0.037&&c2<0.12))
	{
		//���Ʋ�ֵ,��ֵ�Ƚ��ݴ����ڿ�֮���ֵ�����������=����&&����=���»�����=����&&����=����
		if((abs(c1-c2)<0.04&&abs(c3-c4)<0.04)||(abs(c1-c3)<0.04&&abs(c2-c4)<0.04))
		{
			iscircle = true;
		}
	}


	return iscircle;
}