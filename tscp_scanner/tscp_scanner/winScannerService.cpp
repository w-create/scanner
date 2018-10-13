#include <stdio.h>
#include <iostream>
#include <string.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include "twain.h"
#include "scannerService.h"



extern "C" {

#include "jpeglib.h"
#include "jerror.h"
}



/**
** author zhengchenglei
** createTime 2018/10/08
**/



#define VERSION 1.0;



TW_IDENTITY AppID;

StrType MANUFACTURER = "PENTUM";

StrType PRODUCT_FAMILY = "PENTUM";

StrType PRODUCT_NAME = "PANTUM-SCANNER";
unsigned char *m_pBits;

BOOL m_bSourceSelected;

StrType DRIVER_DLL = "TWAIN_32.DLL";

HMODULE hDSMDLL;

DSMENTRYPROC lpDSM_Entry;

TW_IDENTITY m_Source;

TW_MEMREF WF;

int m_nImageCount;
char name[20][50];
TW_INT16  m_returnCode;




using namespace std;





float FIX32ToFloat(TW_FIX32  fix32)
{
	float  floater;
	floater = (float)fix32.Whole + (float)fix32.Frac / 65536.0;
	return floater;
}

TW_FIX32 FloatToFix32(float floater)
{
	TW_FIX32 Fix32_value;
	TW_INT32 value = (TW_INT32)(floater * 65536.0 + 0.5);
	Fix32_value.Whole = value >> 16;
	Fix32_value.Frac = value & 0x0000ffffL;
	return (Fix32_value);
}




/**
*  CHECK SYSTEM ENVIRONMENT
**/
BOOL checkEnvironment() {


	if ((hDSMDLL = LoadLibrary(DRIVER_DLL)) == NULL) {
		cout << "aaaaaaaaa" << endl;
		return TRUE;
	}

	return FALSE;
}




BOOL CallTwainProc(pTW_IDENTITY pOrigin, pTW_IDENTITY pDest, TW_UINT32 DG, TW_UINT16 DAT, TW_UINT16 MSG, TW_MEMREF pData) {

	USHORT ret_val;
	ret_val = (*lpDSM_Entry)(pOrigin, pDest, DG, DAT, MSG, pData);
	m_returnCode = ret_val;
	cout << "callTwainProc result ";
	cout << "handler finish! "; cout << ret_val << endl;
	if (ret_val == TWRC_SUCCESS) {
		return (ret_val == TWRC_SUCCESS);
	}
	else {
		return FALSE;
	}

};



BOOL initAppId() {

	AppID.Id = 0;
	AppID.Version.MajorNum = 3;
	AppID.Version.MinorNum = 5;
	AppID.Version.Language = TWLG_ENGLISH_USA;
	AppID.Version.Country = TWCY_USA;
	lstrcpy(AppID.Version.Info, "1.0");
	AppID.ProtocolMajor = 2;
	AppID.ProtocolMinor = 2;
	AppID.SupportedGroups = DF_APP2 | DG_IMAGE | DG_CONTROL;
	lstrcpy(AppID.Manufacturer, MANUFACTURER);
	lstrcpy(AppID.ProductFamily, PRODUCT_FAMILY);
	lstrcpy(AppID.ProductName, PRODUCT_NAME);

	return TRUE;
};


void logger(const char *key, const char *val) {

	cout << *key; cout << *val << endl;
}

void logger(const char * key, int val) {

	cout << key; cout << val << endl;
}



BOOL SetCapability(TW_CAPABILITY& cap)
{

	return CallTwainProc(&AppID, &m_Source, DG_CONTROL, DAT_CAPABILITY, MSG_SET, (TW_MEMREF)&cap);

}

/*
Sets the capability of the Twain Data Source
*/
BOOL SetCapability(TW_UINT16 cap, TW_UINT16 value, BOOL sign)
{

	TW_CAPABILITY twCap;
	pTW_ONEVALUE pVal;
	BOOL ret_value = FALSE;

	twCap.Cap = cap;
	twCap.ConType = TWON_ONEVALUE;

	twCap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));
	if (twCap.hContainer)
	{
		pVal = (pTW_ONEVALUE)GlobalLock(twCap.hContainer);
		pVal->ItemType = sign ? TWTY_INT16 : TWTY_UINT16;
		pVal->Item = (TW_UINT32)value;
		GlobalUnlock(twCap.hContainer);
		ret_value = SetCapability(twCap);
		GlobalFree(twCap.hContainer);
	}
	return ret_value;
}





BOOL SetResolution(TW_UINT16 cap, TW_UINT32 value)
{

	TW_CAPABILITY twCap;
	pTW_ONEVALUE pVal;
	BOOL ret_value = FALSE;

	twCap.Cap = cap;
	twCap.ConType = TWON_ONEVALUE;

	twCap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));
	if (twCap.hContainer)
	{
		pVal = (pTW_ONEVALUE)GlobalLock(twCap.hContainer);
		pVal->ItemType = TWTY_FIX32;
		pVal->Item = (TW_UINT32)value;
		GlobalUnlock(twCap.hContainer);
		ret_value = SetCapability(twCap);
		GlobalFree(twCap.hContainer);
	}
	return ret_value;
}



BOOL SetImageCount(TW_INT16 nCount)
{
	if (SetCapability(CAP_XFERCOUNT, (TW_UINT16)nCount, TRUE))
	{
		m_nImageCount = nCount;
		return TRUE;
	}

	return FALSE;
}

BOOL optionSetting() {



	TW_CAPABILITY twCap;
	pTW_ONEVALUE pVal;
	BOOL ret_value = FALSE;
	ret_value = SetCapability(CAP_DUPLEXENABLED, 0, TRUE);
	cout << "CAP_DUPLEXENABLED "; cout << ret_value << endl;
	ret_value = SetCapability(ICAP_SUPPORTEDSIZES, 1, TRUE);
	cout << "ICAP_SUPPORTEDSIZES "; cout << ret_value << endl;
	ret_value = SetCapability(ICAP_PIXELTYPE, 2, TRUE);
	cout << "ICAP_PIXELTYPE "; cout << ret_value << endl;
	ret_value = SetResolution(ICAP_XRESOLUTION, 300);
    cout << "ICAP_XRESOLUTION "; cout << ICAP_XRESOLUTION << endl;
	ret_value = SetResolution(ICAP_YRESOLUTION, 300);
	cout << "ICAP_YRESOLUTION "; cout << ret_value << endl;
	SetImageCount(-1);










	// 查找进纸位置



	TW_CAPABILITY   twCapability;
	//	TW_INT16         rc;
	//Setup TW_CAPABILITY Structure 
	twCapability.Cap = CAP_CAMERASIDE;     //Fill in capability of interest 
//	twCapability.ConType = TWON_DONTCARE16;
	twCapability.hContainer = NULL;
	//Send the Triplet 


	ret_value = (*lpDSM_Entry)(&AppID, &m_Source, DG_IMAGE, DAT_IMAGEINFO, MSG_GET, (TW_MEMREF)&twCapability);
	
	
	if (ret_value == TWRC_SUCCESS) {
		cout << "获取参数成功 success ! " << endl;
		cout << "twCapability.ConType ! "; cout << twCapability.ConType << endl;
		//Switch on Container Type 
		switch (twCapability.ConType){
		case TWON_ENUMERATION:
		{
			pTW_ENUMERATION   pvalEnum;
			TW_UINT16         valueU16;
			TW_UINT16         index;
			pvalEnum = (pTW_ENUMERATION)GlobalLock(twCapability.hContainer);
			int NumItems = pvalEnum->NumItems;
			int CurrentIndex = pvalEnum->CurrentIndex;
			int DefaultIndex = pvalEnum->DefaultIndex;

			index = 0;
			//for (index = 0; index < pvalEnum->NumItems; index++)
			while (index < pvalEnum->NumItems)
			{
				if (pvalEnum->ItemType == TWTY_UINT16)
				{
					valueU16 = ((TW_UINT16)(pvalEnum->ItemList[index * 2]));
					cout << "caption valueU16 "; cout << valueU16 << endl;
					//Store Item Value 
				}
				else if (pvalEnum->ItemType == TWTY_BOOL)
				{
					boolean valueBool = ((TW_BOOL*)&pvalEnum->ItemList)[index];
					cout << "caption valueU16 "; cout << valueBool << endl;
				}
				index++;
			}
			GlobalUnlock(twCapability.hContainer);
		}
		break;
		case TWON_ONEVALUE:
		{
			cout << "获取一个参数成功 success !  " << endl;
			pTW_ONEVALUE pvalOneValue;
			TW_BOOL valueBool;
			pvalOneValue = (pTW_ONEVALUE)GlobalLock(twCapability.hContainer);
			cout << pvalOneValue->ItemType << endl;
			if (pvalOneValue->ItemType == TWTY_BOOL)
			{
				valueBool = (TW_BOOL)pvalOneValue->Item;
				cout << "valueBool valueU16 "; cout << valueBool << endl;
				//Store Item Value 
			}
			else if(pvalOneValue->ItemType == TWTY_INT16){
				valueBool = (TW_BOOL)pvalOneValue->Item;
				cout << "valueBool valueU16 "; cout << valueBool << endl;
			}
			GlobalUnlock(twCapability.hContainer);
		}
		break;
		//-----RANGE 
		case TWON_RANGE:
		{
			pTW_RANGE         pvalRange;
			pTW_FIX32         pTWFix32;
			float            valueF32;
			TW_UINT16         index;
			pvalRange = (pTW_RANGE)GlobalLock(twCapability.hContainer);
			if ((TW_UINT16)pvalRange->ItemType == TWTY_FIX32)
			{
				pTWFix32 = (pTW_FIX32)(pvalRange->MinValue);
				valueF32 = FIX32ToFloat(*pTWFix32);
				//Store Item Value 
				pTWFix32 = (pTW_FIX32)(pvalRange->MaxValue);
				valueF32 = FIX32ToFloat(*pTWFix32);
				//Store Item Value 
				pTWFix32 = (pTW_FIX32)(pvalRange->StepSize);
				valueF32 = FIX32ToFloat(*pTWFix32);
				//Store Item Value 
			}
			GlobalUnlock(twCapability.hContainer);
		}
		break;
		case TWON_ARRAY:
		{
			pTW_ARRAY         pvalArray;
			TW_UINT16         valueU16;
			TW_UINT16         index;

			pvalArray = (pTW_ARRAY)GlobalLock(twCapability.hContainer);
			for (index = 0; index < pvalArray->NumItems; index++)
			{
				if (pvalArray->ItemType == TWTY_UINT16)
				{
					valueU16 = ((TW_UINT16)(pvalArray->ItemList[index * 2]));
					//Store Item Value 
				}
			}
			GlobalUnlock(twCapability.hContainer);
		}
		break;
		}   //End Switch Statement 
		GlobalFree(twCapability.hContainer);
	}else {

		cout << "获取参数失败 ! " << endl;
		//Capability MSG_GET Failed check Condition Code 
	}




	//return ret_value;

	TW_USERINTERFACE userInterface;
	userInterface.ShowUI = FALSE;
	TW_UINT16 rc = (*lpDSM_Entry) (&AppID, &m_Source, DG_CONTROL, DAT_USERINTERFACE, MSG_ENABLEDS, &userInterface);
	cout << "MSG_ENABLEDS "; cout << rc << endl;
	cout << "userInterface hParent "; cout << userInterface.hParent << endl;
	cout << "userInterface ModalUI "; cout << userInterface.ModalUI << endl;
	cout << "userInterface ShowUI "; cout << userInterface.ShowUI << endl;

	if (rc != 0) {
		return FALSE;
	}


	return TRUE;

}


BOOL scannerProcess() {

	TW_EVENT twEvent;
	twEvent.pEvent = (TW_MEMREF)WF;
	twEvent.TWMessage = MSG_NULL;

	TW_UINT16 rc = (*lpDSM_Entry) (&AppID, &m_Source, DG_CONTROL, DAT_EVENT, MSG_PROCESSEVENT, &twEvent);
	if (rc != 0) {
		cout << "scan process error !" << endl;
		return FALSE;
	}
	cout << "MSG_PROCESSEVENT "; cout << rc << endl;
	cout << "twEvent pEvent "; cout << twEvent.pEvent << endl;
	cout << "twEvent TWMessage "; cout << twEvent.TWMessage << endl;

	TW_IMAGEINFO info;
	rc = (*lpDSM_Entry)(&AppID, &m_Source, DG_IMAGE, DAT_IMAGEINFO, MSG_GET, (TW_MEMREF)&info);
	cout << "MSG_GET "; cout << rc << endl;
	if (rc != 0) {
		cout << "get paramter failure !" << endl;
		return FALSE;
	}


	HANDLE hBitmap;
	rc = (*lpDSM_Entry)(&AppID, &m_Source, DG_IMAGE, DAT_IMAGENATIVEXFER, MSG_GET, &hBitmap);
	cout << "DAT_IMAGENATIVEXFER - TWRC_XFERDONE "; cout << rc << endl;

	return 0;

}


BOOL openSourceManager() {

	//cout << "aaaa" << endl;


	WF = GetModuleHandle(NULL);
	lpDSM_Entry = (DSMENTRYPROC)GetProcAddress(hDSMDLL, MAKEINTRESOURCE(1));
	cout << "aaaa" << endl;
	if (lpDSM_Entry == NULL) {
		cout << "get method failure!" << endl;
	}
	TW_UINT16 rc = (*lpDSM_Entry) (&AppID, NULL, DG_CONTROL, DAT_PARENT, MSG_OPENDSM, WF);

	cout << "MSG_OPENDSM" << endl;

	return rc;
}


BOOL openDataSource(string scannerName) {

	//cout << "choose driver is"; cout << &scannerName.c_str << endl;

	while (true) {
		TW_UINT16 rc = (*lpDSM_Entry) (&AppID, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, &m_Source);

		int result = scannerName.compare(m_Source.ProductName);
		//cout << "method compare finish "; cout << m_Source.ProductName; cout << result << endl;
		if (rc != 0 || scannerName.compare(m_Source.ProductName) == 0) {
			cout << "jump scanner name "; cout << m_Source.ProductName << endl;
			break;
		}
	}

	BOOL result = CallTwainProc(&AppID, NULL, DG_CONTROL, DAT_IDENTITY, MSG_OPENDS, &m_Source);

	if (!result) {
		cout << "open data source failure" << endl;
	}
	else {
		cout << "open data source success " << endl;
	}

	return result;

}






void openScanner() {


}




BOOL SelectDefaultSource(DSMENTRYPROC *lpDSM_Entry)
{
	cout << "开始获取默认数据源!" << endl;
	m_bSourceSelected = CallTwainProc(&AppID, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETDEFAULT, &m_Source);
	return m_bSourceSelected;
};



DRIVERNAMES getDrivers(DRIVERNAMES *driverNames) {


	TW_UINT16 rc = (*lpDSM_Entry) (&AppID, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, &m_Source);

	if (rc != 0) {
		return *driverNames;
	}

	strcpy(name[0], m_Source.ProductName);
	*driverNames->drivers = name[0];

	int i = 0;
	while (true) {
		rc = (*lpDSM_Entry) (&AppID, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, &m_Source);

		if (rc != 0) {
			break;
		}
		i++;
		driverNames->drivers++;
		string temp = m_Source.ProductName;
		int length = temp.length();
		strcpy(name[i], m_Source.ProductName);
		*driverNames->drivers = name[i];
	}

	driverNames->size = i + 1;
	driverNames->drivers = driverNames->drivers - i;
	return *driverNames;
}




void scannerHandler(DRIVER_NAME driverName, PIC_DIR_PATH path) {


}



BOOL GetImageInfo(TW_IMAGEINFO& info)
{
	return CallTwainProc(&AppID, &m_Source, DG_IMAGE, DAT_IMAGEINFO, MSG_GET, (TW_MEMREF)&info);
}


int  ShouldTransfer(TW_IMAGEINFO& info) {
	return TWCPP_DOTRANSFER;
};


BOOL EndTransfer()
{
	TW_PENDINGXFERS twPend;
	if (CallTwainProc(&AppID, &m_Source, DG_CONTROL, DAT_PENDINGXFERS, MSG_ENDXFER, (TW_MEMREF)&twPend))
	{
		if (twPend.Count == 0)
		{
			// DisableSource();
		}
		return twPend.Count != 0;
	}
	return FALSE;
}


/*
Aborts all transfers
*/
void CancelTransfer()
{
	TW_PENDINGXFERS twPend;
	CallTwainProc(&AppID, &m_Source, DG_CONTROL, DAT_PENDINGXFERS, MSG_RESET, (TW_MEMREF)&twPend);
}



void GeneJpegFile(const char* jpegFileName, unsigned char* inputData,
	int nWidth, int nHeight, int nChannel, int nQuality)
{

	/* This struct contains the JPEG compression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	* It is possible to have several such structures, representing multiple
	* compression/decompression processes, in existence at once.  We refer
	* to any one struct (and its associated working data) as a "JPEG object".
	*/
	struct jpeg_compress_struct cinfo;

	/* This struct represents a JPEG error handler.  It is declared separately
	* because applications often want to supply a specialized error handler
	* (see the second half of this file for an example).  But here we just
	* take the easy way out and use the standard error handler, which will
	* print a message on stderr and call exit() if compression fails.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct jpeg_error_mgr jerr;

	/* More stuff */
	FILE *outfile;                  /* target file */
	JSAMPROW row_pointer[1];        /* pointer to JSAMPLE row[s] */
	int     row_stride;             /* physical row width in image buffer */

									/* Step 1: allocate and initialize JPEG compression object */

									/* We have to set up the error handler first, in case the initialization
									* step fails.  (Unlikely, but it could happen if you are out of memory.)
									* This routine fills in the contents of struct jerr, and returns jerr's
									* address which we place into the link field in cinfo.
									*/
	cinfo.err = jpeg_std_error(&jerr);

	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);  /* Now we can initialize the JPEG compression object. */

								   /* Step 2: specify data destination (eg, a file) */
								   /* Note: steps 2 and 3 can be done in either order. */

								   /* Here we use the library-supplied code to send compressed data to a
								   * stdio stream.  You can also write your own code to do something else.
								   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
								   * requires it in order to write binary files.
								   */
	if ((outfile = fopen(jpegFileName, "wb")) == NULL)
	{
		fprintf(stderr, "can't open %s\n", jpegFileName);
		return;
	}
	jpeg_stdio_dest(&cinfo, outfile);

	/* Step 3: set parameters for compression */

	/* First we supply a description of the input image.
	* Four fields of the cinfo struct must be filled in:
	*/
	cinfo.image_width = nWidth;                /* image width and height, in pixels */
	cinfo.image_height = nHeight;
	cinfo.input_components = nChannel;         /* # of color components per pixel */

	if (nChannel == 1)
	{
		cinfo.in_color_space = JCS_GRAYSCALE;  /* colorspace of input image */
	}
	else if (nChannel == 3)
	{
		cinfo.in_color_space = JCS_RGB;        /* colorspace of input image */
	}

	/* Now use the library's routine to set default compression parameters.
	* (You must set at least cinfo.in_color_space before calling this,
	* since the defaults depend on the source color space.)
	*/
	jpeg_set_defaults(&cinfo);

	// Now you can set any non-default parameters you wish to.
	// Here we just illustrate the use of quality (quantization table) scaling:
	jpeg_set_quality(&cinfo, nQuality, TRUE); /* limit to baseline-JPEG values */

											  /* Step 4: Start compressor */

											  /* TRUE ensures that we will write a complete interchange-JPEG file.
											  * Pass TRUE unless you are very sure of what you're doing.
											  */
	jpeg_start_compress(&cinfo, TRUE);

	/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */

	/* Here we use the library's state variable cinfo.next_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	* To keep things simple, we pass one scanline per call; you can pass
	* more if you wish, though.
	*/
	row_stride = nWidth * nChannel; /* JSAMPLEs per row in image_buffer */

	while (cinfo.next_scanline < cinfo.image_height)
	{
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could pass
		* more than one scanline at a time if that's more convenient.
		*/
		row_pointer[0] = &inputData[cinfo.next_scanline * row_stride];

		(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	/* Step 6: Finish compression */
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	/* After finish_compress, we can close the output file. */
	fclose(outfile);

}






BOOL GetImage(TW_IMAGEINFO& info)
{
	cout << "GetImage" << endl;
	HANDLE hBitmap;
	CallTwainProc(&AppID, &m_Source, DG_IMAGE, DAT_IMAGENATIVEXFER, MSG_GET, &hBitmap);
	cout << "GetImage m_returnCode "; cout << m_returnCode << endl;
	switch (m_returnCode)
	{
	case TWRC_XFERDONE: {


		DWORD dwLen = ::GlobalSize(hBitmap);

		LPSTR hCopy;
		hCopy = (LPSTR) ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwLen);


		UCHAR *lpVoid, *pBits;
		LPBITMAPINFOHEADER pHead;
		RGBQUAD *pRgb;
		lpVoid = (UCHAR *)GlobalLock(hBitmap);
		pHead = (LPBITMAPINFOHEADER)lpVoid;
		int width = pHead->biWidth;
		int height = pHead->biHeight;
		int m_nBits = pHead->biBitCount;
		int widthBit = width * (info.BitsPerPixel / 8);
		int widthBmpBit = widthBit;
		while (widthBmpBit % 4) widthBmpBit++;
		pBits = lpVoid + sizeof(BITMAPINFOHEADER);
		m_pBits = (unsigned char *)malloc(height*widthBmpBit);
		memcpy(m_pBits, pBits, height*widthBmpBit);



		cout << "开始写入数据 " << endl;
		cout << "biBitCount "; cout << m_nBits << endl; cout << "picture width "; cout << info.ImageWidth; cout << "picture length  "; cout << info.ImageLength << endl;
		cout << "data size "; cout << dwLen; cout << "channel "; cout << info.BitsPerPixel << endl;

		int size = height * widthBmpBit;
		pBits = pBits + size;
		//			int widthBit = width*(info.BitsPerPixel/8);
		unsigned char *picdata = (unsigned char*)malloc(height*widthBit);
		int k = 1;
		while (k <= size) {
			if (k%widthBmpBit > widthBit || (k%widthBmpBit == 0)) {
		
				pBits--;
				k++;
				continue;
			}
			*picdata = *pBits;
			k++;
			pBits--;
			if (k>size) {
				break;
			}
			picdata++;
		}
		picdata = picdata - height * widthBit;

		cout << "data convert " << endl;
		unsigned char *convert;
		convert = (unsigned char*)malloc(height*widthBit);
		int v = height;
		int oo = 0;
		while (v > 0) {
			int x = 0;
			int startOff = widthBit * (v - 1);

			while (x < widthBit) {
				*convert = *(picdata + startOff + x);
				x++;
				if (x < widthBit) {
					oo++;
					convert++;
				}
			}
			if (v >0) {
				oo++;
				convert++;
			}
			v--;
		}

		cout << oo; cout << " data convert end" << endl;
		convert = convert - height * widthBit;
		

		cout << "insert file" << endl;
		GeneJpegFile("c:/aaa.jpeg", (unsigned char *)convert, info.ImageWidth, info.ImageLength, info.BitsPerPixel / 8, 50);
		GlobalUnlock(lpVoid);
	}
						break;
	case TWRC_CANCEL:
		break;
	case TWRC_FAILURE:
		CancelTransfer();
		return FALSE;

	}
	GlobalFree(hBitmap);
	return EndTransfer();
}





void messageHandler() {


	TW_IMAGEINFO info;
	BOOL bContinue = TRUE;
	while (bContinue)
	{
		cout << "messageHandler bContinue"; cout << bContinue << endl;

		if (GetImageInfo(info))
		{
			int permission;
			permission = ShouldTransfer(info);
			switch (permission)
			{
			case TWCPP_CANCELTHIS:
				bContinue = EndTransfer();
				break;
			case TWCPP_CANCELALL:
				CancelTransfer();
				bContinue = FALSE;
				break;
			case TWCPP_DOTRANSFER: {
				bContinue = GetImage(info);
				break;

			}
			}
		}
	}

}




int main() {

	char a;

	if (checkEnvironment()) {
		cout << "checkEnvironment faile" << endl;
	}
	else {
		cout << "checkEnvironment success" << endl;
	}
	initAppId();
	cout << "initAppId success" << endl;
	openSourceManager();

	DRIVERNAMES *driverNames = (DRIVERNAMES*)malloc(sizeof(DRIVERNAMES));
	driverNames->drivers = (String *)malloc(sizeof(String) * 20);
	driverNames->size = 0;

	getDrivers(driverNames);

	int size = driverNames->size;
	while (size > 0) {
		cout << "result  ";  cout << *driverNames->drivers << endl;
		if (size > 0) {
			driverNames->drivers++;
		}
		size--;
	}
	openDataSource("Pantum M6600 Series TWAIN");
	optionSetting();
	cout << "messageHandler" << endl;
	messageHandler();



	cin >> a;
	return 0;

}