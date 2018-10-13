
#include <stdio.h>

/**
 ** author zhengchenglei
 ** createTime 2018/10/08
 **/

//  scanner argument
#define DEFINE_COMPRESS_PERCENT 50;

#define TWCPP_ANYCOUNT		(-1)
#define TWCPP_CANCELTHIS	(1)
#define TWCPP_CANCELALL		(2)
#define TWCPP_DOTRANSFER	(0)

// return status and message infomation...
int DDL_NOFOUND_CODE = -100;
const char* DDL_NOFOUND_MSG = "DDL_NOFOUND";



typedef int SCANTYPE;  // signl page. two side page
typedef int COMPRESS_PERCENT;
typedef char* DRIVER_NAME;
typedef char* PIC_DIR_PATH;
typedef const char* StrType;
typedef char* String;




struct HANDLER_RESULT
{
	int HANDLER_CODE;
	const char *MSG;
} RESULT_STATUS;


typedef struct {

	int size;
	String *drivers;

} DRIVERNAMES;




struct PARAMTER
{
	SCANTYPE type;

	COMPRESS_PERCENT percent;

} SCAN_PARAMETER;



/**
 *  Setting scanner paramter. It's contain scanType,
    compress percent and so on ...
 * 
 **/
void setScannerParamter(int option,int val);

/**
 *
 **/
void scannerHandler(DRIVER_NAME driverName, PIC_DIR_PATH path);

/**
 *
 **/
void generatePic(const char* fileName, unsigned char* inputData,int nWidth, int nHeight, int nChannel, int nQuality);


/**
*
***/
void openScanner();


DRIVERNAMES getDrivers();
