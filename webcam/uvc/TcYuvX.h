/*=///////////////////////////////////////////////////////////////////////=*/
/*= TcYuvX.h : Defines the YUV查表法 header for the LIB application.      =*/
/*= Made By ShanChengKun.PentiumWorkingRoom   Based Made in 2014.12.08    =*/
/*= CopyRight(C) 1996-2018 . Email: sck007@163.com . Update on 2015.09.19 =*/
/*=///////////////////////////////////////////////////////////////////////=*/

#if !defined(_SCK007_TCYUVLIB_INCLUDED_PENTIUMWORKINGROOM_ZNN008_)
#define _SCK007_TCYUVLIB_INCLUDED_PENTIUMWORKINGROOM_ZNN008_

/*=///////////////////////////////////////////////////////////////////////=*/

typedef unsigned char		BYTE;
typedef unsigned short		WORD;
typedef unsigned int		UINT;

/*=///////////////////////////////////////////////////////////////////////=*/

/* DShow的4:2:2的首选格式(YUYV)：打包-YUY2，大小(nWd * nHi * 2) */
void RgbFromPackYUY2(BYTE *pRgb, const BYTE *pYuv, int nWd, int nHi);

/* Yuv420SP(Y平面UV打包)：0=DShow的NV12，1=安卓NV21，大小(nWd * nHi * 3 / 2) */
void RgbFromYuv420SP(BYTE *pRgb, const BYTE *pYuv, int nWd, int nHi, int nFor);

/* Yuv420P(YUV均为平面)：0=标准的I420，1=H264的YV12，大小(nWd * nHi * 3 / 2) */
void RgbFromYuv420P(BYTE *pRgb, const BYTE *pYuv, int nWd, int nHi, int nFor);

/* 从RAW格式使用插值的方法返回RGB数据，(0=BG,1=RG,2=GB,3=GR) */
void RgbFromRaw(BYTE *pRgb, const BYTE *pRaw, int nWd, int nHi, int nWho);

/*=///////////////////////////////////////////////////////////////////////=*/

#endif /* _SCK007_TCYUVLIB_INCLUDED_PENTIUMWORKINGROOM_ZNN008_ */

/*=///////////////////////////////////////////////////////////////////////=*/
/*= The end of this file. =*/
